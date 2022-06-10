/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file resolve_addr.c
 * \brief Implement resolving address functions
 **/

#define RESOLVE_ADDR_PRIVATE

#include "app/config/config.h"
#include "app/config/resolve_addr.h"

#include "core/mainloop/mainloop.h"

#include "feature/control/control_events.h"
#include "feature/dirauth/authmode.h"

#include "lib/encoding/confline.h"
#include "lib/net/gethostname.h"
#include "lib/net/resolve.h"

/** Maximum "Address" statement allowed in our configuration. */
#define MAX_CONFIG_ADDRESS 2

/** Ease our life. Arrays containing state per address family. These are to
 * add semantic to the code so we know what is accessed. */
#define IDX_NULL 0 /* Index to zeroed address object. */
#define IDX_IPV4 1 /* Index to AF_INET. */
#define IDX_IPV6 2 /* Index to AF_INET6. */
#define IDX_SIZE 3 /* How many indexes do we have. */

/** Function in our address function table return one of these code. */
typedef enum {
  /* The address has been found. */
  FN_RET_OK   = 0,
  /* The failure requirements were not met and thus it is recommended that the
   * caller stops the search. */
  FN_RET_BAIL = 1,
  /* The address was not found or failure is transient so the caller should go
   * to the next method. */
  FN_RET_NEXT = 2,
} fn_address_ret_t;

/** Last resolved addresses. */
static tor_addr_t last_resolved_addrs[] =
  { TOR_ADDR_NULL, TOR_ADDR_NULL, TOR_ADDR_NULL };
CTASSERT(ARRAY_LENGTH(last_resolved_addrs) == IDX_SIZE);

/** Last suggested addresses.
 *
 * These addresses come from a NETINFO cell from a trusted relay (currently
 * only authorities). We only use those in last resort. */
static tor_addr_t last_suggested_addrs[] =
  { TOR_ADDR_NULL, TOR_ADDR_NULL, TOR_ADDR_NULL };
CTASSERT(ARRAY_LENGTH(last_suggested_addrs) == IDX_SIZE);

/** True iff the address was found to be configured that is from the
 * configuration file either using Address or ORPort. */
static bool last_addrs_configured[] = { false, false, false };
CTASSERT(ARRAY_LENGTH(last_addrs_configured) == IDX_SIZE);

static inline int
af_to_idx(const int family)
{
  switch (family) {
  case AF_INET:
    return IDX_IPV4;
  case AF_INET6:
    return IDX_IPV6;
  default:
    /* It wouldn't be safe to just die here with an assert but we can heavily
     * scream with a bug. Return the index of the NULL address. */
    tor_assert_nonfatal_unreached();
    return IDX_NULL;
  }
}

/** Return string representation of the given method. */
const char *
resolved_addr_method_to_str(const resolved_addr_method_t method)
{
  switch (method) {
  case RESOLVED_ADDR_NONE:
    return "NONE";
  case RESOLVED_ADDR_CONFIGURED:
    return "CONFIGURED";
  case RESOLVED_ADDR_CONFIGURED_ORPORT:
    return "CONFIGURED_ORPORT";
  case RESOLVED_ADDR_GETHOSTNAME:
    return "GETHOSTNAME";
  case RESOLVED_ADDR_INTERFACE:
    return "INTERFACE";
  case RESOLVED_ADDR_RESOLVED:
    return "RESOLVED";
  default:
    tor_assert_nonfatal_unreached();
    return "???";
  }
}

/** Return true if the last address of family was configured or not. An
 * address is considered configured if it was found in the Address or ORPort
 * statement.
 *
 * This applies to the address returned by the function
 * resolved_addr_get_last() which is the cache of discovered addresses. */
bool
resolved_addr_is_configured(int family)
{
  return last_addrs_configured[af_to_idx(family)];
}

/** Copy the last suggested address of family into addr_out.
 *
 * If no last suggested address exists, the addr_out is a null address (use
 * tor_addr_is_null() to confirm). */
void
resolved_addr_get_suggested(int family, tor_addr_t *addr_out)
{
  tor_addr_copy(addr_out, &last_suggested_addrs[af_to_idx(family)]);
}

/** Set the last suggested address into our cache. This is called when we get
 * a new NETINFO cell from a trusted source. */
void
resolved_addr_set_suggested(const tor_addr_t *addr)
{
  if (BUG(tor_addr_family(addr) != AF_INET &&
          tor_addr_family(addr) != AF_INET6)) {
    return;
  }

  /* In case we don't have a configured address, log that we will be using the
   * one discovered from the dirauth. */
  const int idx = af_to_idx(tor_addr_family(addr));
  if (tor_addr_is_null(&last_resolved_addrs[idx]) &&
      !tor_addr_eq(&last_suggested_addrs[idx], addr)) {
    log_notice(LD_CONFIG, "External address seen and suggested by a "
                          "directory authority: %s", fmt_addr(addr));
  }
  tor_addr_copy(&last_suggested_addrs[idx], addr);
}

/** Copy the last resolved address of family into addr_out.
 *
 * If not last resolved address existed, the addr_out is a null address (use
 * tor_addr_is_null()). */
void
resolved_addr_get_last(int family, tor_addr_t *addr_out)
{
  tor_addr_copy(addr_out, &last_resolved_addrs[af_to_idx(family)]);
}

/** Reset the last resolved address of family.
 *
 * This makes it null address. */
void
resolved_addr_reset_last(int family)
{
  tor_addr_make_null(&last_resolved_addrs[af_to_idx(family)], family);
}

/** Errors returned by address_can_be_used() in order for the caller to know
 * why the address is denied or not. */
#define ERR_DEFAULT_DIRAUTH     -1 /* Using default authorities. */
#define ERR_ADDRESS_IS_INTERNAL -2 /* IP is internal. */

/** @brief Return true iff the given IP address can be used as a valid
 *         external resolved address.
 *
 * Two tests are done in this function:
 *    1) If the address if NOT internal, it can be used.
 *    2) If the address is internal and we have custom directory authorities
 *       configured then it can they be used. Important for testing networks.
 *
 * @param addr The IP address to validate.
 * @param options Global configuration options.
 * @param warn_severity Log level that should be used on error.
 * @param explicit_ip Was the IP address explicitly given.
 *
 * @return Return 0 if it can be used. Return error code ERR_* found at the
 *         top of the file.
 */
static int
address_can_be_used(const tor_addr_t *addr, const or_options_t *options,
                    int warn_severity, const bool explicit_ip)
{
  tor_assert(addr);

  /* Public address, this is fine. */
  if (!tor_addr_is_internal(addr, 0)) {
    goto allow;
  }

  /* We allow internal addresses to be used if the PublishServerDescriptor is
   * unset and AssumeReachable (or for IPv6) is set.
   *
   * This is to cover the case where a relay/bridge might be run behind a
   * firewall on a local network to users can reach the network through it
   * using Tor Browser for instance. */
  if (options->PublishServerDescriptor_ == NO_DIRINFO &&
      (options->AssumeReachable ||
       (tor_addr_family(addr) == AF_INET6 && options->AssumeReachableIPv6))) {
    goto allow;
  }

  /* We have a private IP address. This is also allowed if we set custom
   * directory authorities. */
  if (using_default_dir_authorities(options)) {
    log_fn(warn_severity, LD_CONFIG,
           "Address '%s' is a private IP address. Tor relays that use "
           "the default DirAuthorities must have public IP addresses.",
           fmt_addr(addr));
    return ERR_DEFAULT_DIRAUTH;
  }

  if (!explicit_ip) {
    /* Even with custom directory authorities, only an explicit internal
     * address is accepted. */
    log_fn(warn_severity, LD_CONFIG,
           "Address %s was resolved and thus not explicitly "
           "set. Even if DirAuthorities are custom, this is "
           "not allowed.", fmt_addr(addr));
    return ERR_ADDRESS_IS_INTERNAL;
  }

 allow:
  return 0;
}

/** @brief Get IP address from the given config line and for a specific address
 *         family.
 *
 * This can fail is more than two Address statement are found for the same
 * address family. It also fails if no statement is found.
 *
 * @param options Global configuration options.
 * @param warn_severity Log level that should be used on error.
 * @param family IP address family. Only AF_INET and AF_INET6 are supported.
 * @param method_out OUT: Method denoting how the address was found.
 *                   This is described in the control-spec.txt as
 *                   actions for "STATUS_SERVER".
 * @param hostname_out OUT: String containing the hostname gotten from the
 *                     Address value if any.
 * @param addr_out OUT: Tor address of the address found in the cline or
 *                 resolved from the cline.
 *
 * @return Return 0 on success that is an address has been found or resolved
 *         successfully. Return error code ERR_* found at the top of the file.
 */
static fn_address_ret_t
get_address_from_config(const or_options_t *options, int warn_severity,
                        int family, resolved_addr_method_t *method_out,
                        char **hostname_out, tor_addr_t *addr_out)
{
  int ret;
  bool explicit_ip = false, resolve_failure = false;
  int num_valid_addr = 0;

  tor_assert(options);
  tor_assert(addr_out);
  tor_assert(method_out);
  tor_assert(hostname_out);

  /* Set them to NULL for safety reasons. */
  *hostname_out = NULL;
  *method_out = RESOLVED_ADDR_NONE;

  log_debug(LD_CONFIG, "Attempting to get address from configuration");

  if (!options->Address) {
    log_info(LD_CONFIG, "No Address option found in configuration.");
    /* No Address statement, inform caller to try next method. */
    return FN_RET_NEXT;
  }

  for (const config_line_t *cfg = options->Address; cfg != NULL;
       cfg = cfg->next) {
    int af;
    tor_addr_t addr;

    af = tor_addr_parse(&addr, cfg->value);
    if (af == family) {
      tor_addr_copy(addr_out, &addr);
      *method_out = RESOLVED_ADDR_CONFIGURED;
      explicit_ip = true;
      num_valid_addr++;
      continue;
    } else if (af != -1) {
      /* Parsable address but just not the one from the family we want. Skip
       * it so we don't attempt a resolve. */
      continue;
    }

    /* Not an IP address. Considering this value a hostname and attempting to
     * do a DNS lookup. */
    if (!tor_addr_lookup(cfg->value, family, &addr)) {
      tor_addr_copy(addr_out, &addr);
      *method_out = RESOLVED_ADDR_RESOLVED;
      if (*hostname_out) {
        tor_free(*hostname_out);
      }
      *hostname_out = tor_strdup(cfg->value);
      explicit_ip = false;
      num_valid_addr++;
      continue;
    } else {
      /* Hostname that can't be resolved, this is a fatal error. */
      resolve_failure = true;
      log_fn(warn_severity, LD_CONFIG,
             "Could not resolve local Address '%s'. Failing.", cfg->value);
      continue;
    }
  }

  if (!num_valid_addr) {
    if (resolve_failure) {
      /* We found no address but we got a resolution failure. This means we
       * can know if the hostname given was v4 or v6 so we can't continue. */
      return FN_RET_BAIL;
    }
    log_info(LD_CONFIG,
             "No Address option found for family %s in configuration.",
             fmt_af_family(family));
    /* No Address statement for family so move on to try next method. */
    return FN_RET_NEXT;
  }

  if (num_valid_addr >= MAX_CONFIG_ADDRESS) {
    /* Too many Address for same family. This is a fatal error. */
    log_fn(warn_severity, LD_CONFIG,
           "Found %d Address statement of address family %s. "
           "Only one is allowed.", num_valid_addr, fmt_af_family(family));
    tor_free(*hostname_out);
    return FN_RET_BAIL;
  }

  /* Great, we found an address. */
  ret = address_can_be_used(addr_out, options, warn_severity, explicit_ip);
  if (ret != 0) {
    /* One of the requirement of this interface is if an internal Address is
     * used, custom authorities must be defined else it is a fatal error.
     * Furthermore, if the Address was resolved to an internal interface, we
     * stop immediately. */
    if (ret == ERR_ADDRESS_IS_INTERNAL) {
      static bool logged_once = false;
      if (!logged_once) {
        log_warn(LD_CONFIG, "Address set with an internal address. Tor will "
                            "not work unless custom directory authorities "
                            "are defined (AlternateDirAuthority). It is also "
                            "possible to use an internal address if "
                            "PublishServerDescriptor is set to 0 and "
                            "AssumeReachable(IPv6) to 1.");
        logged_once = true;
      }
    }
    tor_free(*hostname_out);
    return FN_RET_BAIL;
  }

  /* Address can be used. We are done. */
  log_info(LD_CONFIG, "Address found in configuration: %s",
           fmt_addr(addr_out));
  return FN_RET_OK;
}

/** @brief Get IP address from the local hostname by calling gethostbyname()
 *         and doing a DNS resolution on the hostname.
 *
 * @param options Global configuration options.
 * @param warn_severity Log level that should be used on error.
 * @param family IP address family. Only AF_INET and AF_INET6 are supported.
 * @param method_out OUT: Method denoting how the address was found.
 *                   This is described in the control-spec.txt as
 *                   actions for "STATUS_SERVER".
 * @param hostname_out OUT: String containing the local hostname.
 * @param addr_out OUT: Tor address resolved from the local hostname.
 *
 * @return Return 0 on success that is an address has been found and resolved
 *         successfully. Return error code ERR_* found at the top of the file.
 */
static fn_address_ret_t
get_address_from_hostname(const or_options_t *options, int warn_severity,
                          int family, resolved_addr_method_t *method_out,
                          char **hostname_out, tor_addr_t *addr_out)
{
  int ret;
  char hostname[256];

  tor_assert(addr_out);
  tor_assert(method_out);

  /* Set them to NULL for safety reasons. */
  *hostname_out = NULL;
  *method_out = RESOLVED_ADDR_NONE;

  log_debug(LD_CONFIG, "Attempting to get address from local hostname");

  if (tor_gethostname(hostname, sizeof(hostname)) < 0) {
    log_fn(warn_severity, LD_NET, "Error obtaining local hostname");
    /* Unable to obtain the local hostname is a fatal error. */
    return FN_RET_BAIL;
  }
  if (tor_addr_lookup(hostname, family, addr_out)) {
    log_fn(warn_severity, LD_NET,
           "Could not resolve local hostname '%s'. Failing.", hostname);
    /* Unable to resolve, inform caller to try next method. */
    return FN_RET_NEXT;
  }

  ret = address_can_be_used(addr_out, options, warn_severity, false);
  if (ret == ERR_DEFAULT_DIRAUTH) {
    /* Non custom authorities, inform caller to try next method. */
    return FN_RET_NEXT;
  } else if (ret == ERR_ADDRESS_IS_INTERNAL) {
    /* Internal address is a fatal error. */
    return FN_RET_BAIL;
  }

  /* addr_out contains the address of the local hostname. */
  *method_out = RESOLVED_ADDR_GETHOSTNAME;
  *hostname_out = tor_strdup(hostname);

  /* Found it! */
  log_info(LD_CONFIG, "Address found from local hostname: %s",
           fmt_addr(addr_out));
  return FN_RET_OK;
}

/** @brief Get IP address from a network interface.
 *
 * @param options Global configuration options.
 * @param warn_severity Log level that should be used on error.
 * @param family IP address family. Only AF_INET and AF_INET6 are supported.
 * @param method_out OUT: Always RESOLVED_ADDR_INTERFACE on success which
 *                   is detailed in the control-spec.txt as actions
 *                   for "STATUS_SERVER".
 * @param hostname_out OUT: String containing the local hostname. For this
 *                     function, it is always set to NULL.
 * @param addr_out OUT: Tor address found attached to the interface.
 *
 * @return Return 0 on success that is an address has been found. Return
 *         error code ERR_* found at the top of the file.
 */
static fn_address_ret_t
get_address_from_interface(const or_options_t *options, int warn_severity,
                           int family, resolved_addr_method_t *method_out,
                           char **hostname_out, tor_addr_t *addr_out)
{
  int ret;

  tor_assert(method_out);
  tor_assert(hostname_out);
  tor_assert(addr_out);

  /* Set them to NULL for safety reasons. */
  *method_out = RESOLVED_ADDR_NONE;
  *hostname_out = NULL;

  log_debug(LD_CONFIG, "Attempting to get address from network interface");

  if (get_interface_address6(warn_severity, family, addr_out) < 0) {
    log_fn(warn_severity, LD_CONFIG,
           "Could not get local interface IP address.");
    /* Unable to get IP from interface. Inform caller to try next method. */
    return FN_RET_NEXT;
  }

  ret = address_can_be_used(addr_out, options, warn_severity, false);
  if (ret < 0) {
    /* Unable to use address. Inform caller to try next method. */
    return FN_RET_NEXT;
  }

  *method_out = RESOLVED_ADDR_INTERFACE;

  /* Found it! */
  log_info(LD_CONFIG, "Address found from interface: %s", fmt_addr(addr_out));
  return FN_RET_OK;
}

/** @brief Get IP address from the ORPort (if any).
 *
 * @param options Global configuration options.
 * @param warn_severity Log level that should be used on error.
 * @param family IP address family. Only AF_INET and AF_INET6 are supported.
 * @param method_out OUT: Always RESOLVED_ADDR_CONFIGURED_ORPORT on success
 *                   which is detailed in the control-spec.txt as actions
 *                   for "STATUS_SERVER".
 * @param hostname_out OUT: String containing the ORPort hostname if any.
 * @param addr_out OUT: Tor address found if any.
 *
 * @return Return 0 on success that is an address has been found. Return
 *         error code ERR_* found at the top of the file.
 */
static fn_address_ret_t
get_address_from_orport(const or_options_t *options, int warn_severity,
                        int family, resolved_addr_method_t *method_out,
                        char **hostname_out, tor_addr_t *addr_out)
{
  int ret;
  const tor_addr_t *addr;

  tor_assert(method_out);
  tor_assert(hostname_out);
  tor_assert(addr_out);

  /* Set them to NULL for safety reasons. */
  *method_out = RESOLVED_ADDR_NONE;
  *hostname_out = NULL;

  log_debug(LD_CONFIG, "Attempting to get address from ORPort");

  if (!options->ORPort_set) {
    log_info(LD_CONFIG, "No ORPort found in configuration.");
    /* No ORPort statement, inform caller to try next method. */
    return FN_RET_NEXT;
  }

  /* Get ORPort for requested family. */
  addr = get_orport_addr(family);
  if (!addr) {
    /* No address configured for the ORPort. Ignore. */
    return FN_RET_NEXT;
  }

  /* We found the ORPort address. Just make sure it can be used. */
  ret = address_can_be_used(addr, options, warn_severity, true);
  if (ret < 0) {
    /* Unable to use address. Inform caller to try next method. */
    return FN_RET_NEXT;
  }

  /* Found it! */
  *method_out = RESOLVED_ADDR_CONFIGURED_ORPORT;
  tor_addr_copy(addr_out, addr);

  log_fn(warn_severity, LD_CONFIG, "Address found from ORPort: %s",
         fmt_addr(addr_out));
  return FN_RET_OK;
}

/** @brief Set the last resolved address cache using the given address.
 *
 * A log notice is emitted if the given address has changed from before. Not
 * emitted on first resolve.
 *
 * Control port event "STATUS_SERVER" is emitted with the new information if
 * it has changed.
 *
 * Finally, tor is notified that the IP address has changed.
 *
 * @param addr IP address to update the cache with.
 * @param method_used By which method did we resolved it (for logging and
 *                    control port).
 * @param hostname_used Which hostname was used. If none were used, it is
 *                      NULL. (for logging and control port).
 */
void
resolved_addr_set_last(const tor_addr_t *addr,
                       const resolved_addr_method_t method_used,
                       const char *hostname_used)
{
  /** Have we done a first resolve. This is used to control logging. */
  static bool have_resolved_once[] = { false, false, false };
  CTASSERT(ARRAY_LENGTH(have_resolved_once) == IDX_SIZE);

  bool *done_one_resolve;
  bool have_hostname = false;
  tor_addr_t *last_resolved;

  tor_assert(addr);

  /* Do we have an hostname. */
  have_hostname = (hostname_used != NULL);

  int idx = af_to_idx(tor_addr_family(addr));
  if (idx == IDX_NULL) {
    /* Not suppose to happen and if it does, af_to_idx() screams loudly. */
    return;
  }

  /* Get values from cache. */
  done_one_resolve = &have_resolved_once[idx];
  last_resolved = &last_resolved_addrs[idx];

  /* Same address last resolved. Ignore. */
  if (tor_addr_eq(last_resolved, addr)) {
    return;
  }

  /* Don't log notice if this is the first resolve we do. */
  if (*done_one_resolve) {
    /* Leave this as a notice, regardless of the requested severity,
     * at least until dynamic IP address support becomes bulletproof. */
    log_notice(LD_NET,
               "Your IP address seems to have changed to %s "
               "(METHOD=%s%s%s). Updating.",
               fmt_addr(addr),
               resolved_addr_method_to_str(method_used),
               have_hostname ? " HOSTNAME=" : "",
               have_hostname ? hostname_used : "");
    ip_address_changed(0);
  }

  /* Notify control port. */
  control_event_server_status(LOG_NOTICE,
                              "EXTERNAL_ADDRESS ADDRESS=%s METHOD=%s%s%s",
                              fmt_addr(addr),
                              resolved_addr_method_to_str(method_used),
                              have_hostname ? " HOSTNAME=" : "",
                              have_hostname ? hostname_used : "");
  /* Copy address to cache. */
  tor_addr_copy(last_resolved, addr);
  *done_one_resolve = true;

  /* Flag true if the address was configured. Else, indicate it was not. */
  last_addrs_configured[idx] = false;
  if (method_used == RESOLVED_ADDR_CONFIGURED ||
      method_used == RESOLVED_ADDR_CONFIGURED_ORPORT) {
    last_addrs_configured[idx] = true;
  }
}

/** Ease our lives. Typedef to the address discovery function signature. */
typedef fn_address_ret_t
  (*fn_address_t)(
     const or_options_t *options, int warn_severity, int family,
     resolved_addr_method_t *method_out, char **hostname_out,
     tor_addr_t *addr_out);

/** Address discovery function table. The order matters as in the first one is
 * executed first and so on. */
static const fn_address_t fn_address_table[] =
{
  /* These functions are in order for our find address algorithm. */
  get_address_from_config,
  get_address_from_orport,
  get_address_from_interface,
  get_address_from_hostname,
};
/** Length of address table as in how many functions. */
static const size_t fn_address_table_len =
  ARRAY_LENGTH(fn_address_table);

/* Address discover function table for authorities (bridge or directory).
 *
 * They only discover their address from either the configuration file or the
 * ORPort. They do not query the interface nor do any DNS resolution for
 * security reasons. */
static const fn_address_t fn_address_table_auth[] =
{
  /* These functions are in order for our find address algorithm. */
  get_address_from_config,
  get_address_from_orport,
};
/** Length of address table as in how many functions. */
static const size_t fn_address_table_auth_len =
  ARRAY_LENGTH(fn_address_table_auth);

/** @brief Attempt to find our IP address that can be used as our external
 *         reachable address.
 *
 *  The following describe the algorithm to find an address. Each have
 *  specific conditions so read carefully.
 *
 *  On success, true is returned and depending on how the address was found,
 *  the out parameters can have different values.
 *
 *  On error, false is returned and out parameters are set to NULL.
 *
 *  1. Look at the configuration Address option.

 *     If Address is a public address, True is returned and addr_out is set
 *     with it, the method_out is set to RESOLVED_ADDR_CONFIGURED and
 *     hostname_out is set to NULL.
 *
 *     If Address is an internal address but NO custom authorities are used,
 *     an error is returned.
 *
 *     If Address is a hostname, that is it can't be converted to an address,
 *     it is resolved. On success, addr_out is set with the address,
 *     method_out is set to RESOLVED_ADDR_RESOLVED and hostname_out is set
 *     to the resolved hostname. On failure to resolve, an error is returned.
 *
 *     If no given Address, fallback to the network interface (see section 2).
 *
 *  2. Look at the network interface.
 *
 *     Attempt to find the first public usable address from the list of
 *     network interfaces returned by the OS.
 *
 *     On failure, we attempt to look at the local hostname (3).
 *
 *     On success, addr_out is set with it, method_out is set to
 *     RESOLVED_ADDR_INTERFACE and hostname_out is set to NULL.
 *
 *  3. Look at the local hostname.
 *
 *     If the local hostname resolves to a non internal address, addr_out is
 *     set with it, method_out is set to RESOLVED_ADDR_GETHOSTNAME and
 *     hostname_out is set to the resolved hostname.
 *
 *     If a local hostname can NOT be found, an error is returned.
 *
 *     If the local hostname resolves to an internal address, an error is
 *     returned.
 *
 *     If the local hostname can NOT be resolved, an error is returned.
 *
 * @param options Global configuration options.
 * @param family IP address family. Only AF_INET and AF_INET6 are supported.
 * @param warn_severity Logging level.
 * @param addr_out OUT: Set with the IP address found if any.
 * @param method_out OUT: (optional) Method denoting how the address wa
 *                   found. This is described in the control-spec.txt as
 *                   actions for "STATUS_SERVER".
 * @param hostname_out OUT: String containing the hostname if any was used.
 *                     Only be set for RESOLVED and GETHOSTNAME methods.
 *                     Else it is set to NULL.
 *
 * @return True if the address was found for the given family. False if not or
 *         on errors.
 */
bool
find_my_address(const or_options_t *options, int family, int warn_severity,
                tor_addr_t *addr_out, resolved_addr_method_t *method_out,
                char **hostname_out)
{
  resolved_addr_method_t method_used = RESOLVED_ADDR_NONE;
  char *hostname_used = NULL;
  tor_addr_t my_addr;
  const fn_address_t *table = fn_address_table;
  size_t table_len = fn_address_table_len;

  tor_assert(options);
  tor_assert(addr_out);

  /* Set them to NULL for safety reasons. */
  tor_addr_make_unspec(addr_out);
  if (method_out) *method_out = RESOLVED_ADDR_NONE;
  if (hostname_out) *hostname_out = NULL;

  /* If an IPv6 is requested, check if IPv6 address discovery is disabled and
   * if so we always return a failure. It is done here so we don't populate
   * the resolve cache or do any DNS resolution. */
  if (family == AF_INET6 && options->AddressDisableIPv6) {
    return false;
  }

  /* For authorities (bridge and directory), we use a different table. */
  if (authdir_mode(options)) {
    table = fn_address_table_auth;
    table_len = fn_address_table_auth_len;
  }

  /*
   * Step 1: Discover address by calling methods from the function table.
   */

  /* Go over the function table. They are in order. */
  for (size_t idx = 0; idx < table_len; idx++) {
    fn_address_ret_t ret = table[idx](options, warn_severity, family,
                                      &method_used, &hostname_used, &my_addr);
    if (ret == FN_RET_BAIL) {
      return false;
    } else if (ret == FN_RET_OK) {
      goto found;
    }
    tor_assert(ret == FN_RET_NEXT);
  }

  /* We've exhausted our attempts. Failure. */
  log_fn(warn_severity, LD_CONFIG, "Unable to find our IP address.");
  return false;

 found:
  /*
   * Step 2: Update last resolved address cache and inform the control port.
   */
  resolved_addr_set_last(&my_addr, method_used, hostname_used);

  if (method_out) {
    *method_out = method_used;
  }
  if (hostname_out) {
    *hostname_out = hostname_used;
  } else {
    tor_free(hostname_used);
  }

  tor_addr_copy(addr_out, &my_addr);
  return true;
}

/** @brief: Return true iff the given addr is judged to be local to our
 * resolved address.
 *
 * This function is used to tell whether another address is 'remote' enough
 * that we can trust it when it tells us that we are reachable, or that we
 * have a certain address.
 *
 * The criterion to learn if the address is local are the following:
 *
 *    1. Internal address.
 *    2. If EnforceDistinctSubnets is set then it is never local.
 *    3. Network mask is compared. IPv4: /24 and IPv6 /48. This is different
 *       from the path selection that looks at /16 and /32 because we only
 *       want to learn here if the address is considered to come from the
 *       Internet basically.
 *
 * @param addr The address to test if local and also test against our resovled
 *             address.
 *
 * @return True iff address is considered local or else False.
 */
MOCK_IMPL(bool,
is_local_to_resolve_addr, (const tor_addr_t *addr))
{
  const int family = tor_addr_family(addr);
  const tor_addr_t *last_resolved_addr =
    &last_resolved_addrs[af_to_idx(family)];

  /* Internal address is always local. */
  if (tor_addr_is_internal(addr, 0)) {
    return true;
  }

  /* Address is not local if we don't enforce subnet distinction. */
  if (get_options()->EnforceDistinctSubnets == 0) {
    return false;
  }

  switch (family) {
  case AF_INET:
    /* It's possible that this next check will hit before the first time
     * find_my_address actually succeeds. For clients, it is likely that
     * find_my_address will never be called at all. In those cases,
     * last_resolved_addr_v4 will be 0, and so checking to see whether ip is
     * on the same /24 as last_resolved_addrs[AF_INET] will be the same as
     * checking whether it was on net 0, which is already done by
     * tor_addr_is_internal. */
    return tor_addr_compare_masked(addr, last_resolved_addr, 24,
                                   CMP_SEMANTIC) == 0;
  case AF_INET6:
    /* Look at /48 because it is typically the smallest network in the global
     * IPv6 routing tables, and it was previously the recommended per-customer
     * network block. (See [RFC 6177: IPv6 End Site Address Assignment].) */
    return tor_addr_compare_masked(addr, last_resolved_addr, 48,
                                   CMP_SEMANTIC) == 0;
    break;
  default:
    /* Unknown address type so not local. */
    return false;
  }
}

#ifdef TOR_UNIT_TESTS

void
resolve_addr_reset_suggested(int family)
{
  tor_addr_make_unspec(&last_suggested_addrs[af_to_idx(family)]);
}

#endif /* defined(TOR_UNIT_TESTS) */
