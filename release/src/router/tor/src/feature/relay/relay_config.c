/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_config.c
 * @brief Code to interpret the user's configuration of Tor's relay module.
 **/

#include "orconfig.h"
#define RELAY_CONFIG_PRIVATE
#include "feature/relay/relay_config.h"

#include "lib/encoding/confline.h"
#include "lib/confmgt/confmgt.h"

#include "lib/container/smartlist.h"
#include "lib/geoip/geoip.h"
#include "lib/meminfo/meminfo.h"
#include "lib/osinfo/uname.h"
#include "lib/process/setuid.h"

/* Required for dirinfo_type_t in or_options_t */
#include "core/or/or.h"
#include "app/config/config.h"

#include "core/mainloop/connection.h"
#include "core/mainloop/cpuworker.h"
#include "core/mainloop/mainloop.h"
#include "core/or/connection_or.h"
#include "core/or/port_cfg_st.h"

#include "feature/hibernate/hibernate.h"
#include "feature/nodelist/nickname.h"
#include "feature/stats/geoip_stats.h"
#include "feature/stats/predict_ports.h"
#include "feature/stats/connstats.h"
#include "feature/stats/rephist.h"

#include "feature/dirauth/authmode.h"

#include "feature/dircache/consdiffmgr.h"
#include "feature/relay/dns.h"
#include "feature/relay/routermode.h"
#include "feature/relay/selftest.h"

/** Contents of most recently read DirPortFrontPage file. */
static char *global_dirfrontpagecontents = NULL;

/* Copied from config.c, we will refactor later in 29211. */
#define REJECT(arg) \
  STMT_BEGIN *msg = tor_strdup(arg); return -1; STMT_END
#if defined(__GNUC__) && __GNUC__ <= 3
#define COMPLAIN(args...) \
  STMT_BEGIN log_warn(LD_CONFIG, args); STMT_END
#else
#define COMPLAIN(args, ...)                                     \
  STMT_BEGIN log_warn(LD_CONFIG, args, ##__VA_ARGS__); STMT_END
#endif /* defined(__GNUC__) && __GNUC__ <= 3 */

/* Used in the various options_transition_affects* functions. */
#define YES_IF_CHANGED_BOOL(opt) \
  if (!CFG_EQ_BOOL(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_INT(opt) \
  if (!CFG_EQ_INT(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_STRING(opt) \
  if (!CFG_EQ_STRING(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_LINELIST(opt) \
  if (!CFG_EQ_LINELIST(old_options, new_options, opt)) return 1;

/** Return the contents of our frontpage string, or NULL if not configured. */
MOCK_IMPL(const char*,
relay_get_dirportfrontpage, (void))
{
  return global_dirfrontpagecontents;
}

/** Release all memory and resources held by global relay configuration
 * structures.
 */
void
relay_config_free_all(void)
{
  tor_free(global_dirfrontpagecontents);
}

/** Return the bandwidthrate that we are going to report to the authorities
 * based on the config options. */
uint32_t
relay_get_effective_bwrate(const or_options_t *options)
{
  uint64_t bw = options->BandwidthRate;
  if (bw > options->MaxAdvertisedBandwidth)
    bw = options->MaxAdvertisedBandwidth;
  if (options->RelayBandwidthRate > 0 && bw > options->RelayBandwidthRate)
    bw = options->RelayBandwidthRate;
  /* config_ensure_bandwidth_cap() makes sure that this cast can't overflow. */
  return (uint32_t)bw;
}

/** Return the bandwidthburst that we are going to report to the authorities
 * based on the config options. */
uint32_t
relay_get_effective_bwburst(const or_options_t *options)
{
  uint64_t bw = options->BandwidthBurst;
  if (options->RelayBandwidthBurst > 0 && bw > options->RelayBandwidthBurst)
    bw = options->RelayBandwidthBurst;
  /* config_ensure_bandwidth_cap() makes sure that this cast can't overflow. */
  return (uint32_t)bw;
}

/** Warn for every Extended ORPort port in <b>ports</b> that is on a
 *  publicly routable address. */
void
port_warn_nonlocal_ext_orports(const smartlist_t *ports, const char *portname)
{
  SMARTLIST_FOREACH_BEGIN(ports, const port_cfg_t *, port) {
    if (port->type != CONN_TYPE_EXT_OR_LISTENER)
      continue;
    if (port->is_unix_addr)
      continue;
    /* XXX maybe warn even if address is RFC1918? */
    if (!tor_addr_is_internal(&port->addr, 1)) {
      log_warn(LD_CONFIG, "You specified a public address '%s' for %sPort. "
               "This is not advised; this address is supposed to only be "
               "exposed on localhost so that your pluggable transport "
               "proxies can connect to it.",
               fmt_addrport(&port->addr, port->port), portname);
    }
  } SMARTLIST_FOREACH_END(port);
}

/**
 * Return a static buffer describing the port number in @a port, which may
 * CFG_AUTO_PORT.
 **/
static const char *
describe_portnum(int port)
{
  static char buf[16];
  if (port == CFG_AUTO_PORT) {
    return "auto";
  } else {
    tor_snprintf(buf, sizeof(buf), "%d", port);
    return buf;
  }
}

/** Return a static buffer containing the human readable logging string that
 * describes the given port object. */
STATIC const char *
describe_relay_port(const port_cfg_t *port)
{
  IF_BUG_ONCE(!port) {
    return "<null port>";
  }

  static char buf[256];
  const char *type, *addr;

  switch (port->type) {
  case CONN_TYPE_OR_LISTENER:
    type = "OR";
    break;
  case CONN_TYPE_DIR_LISTENER:
    type = "Dir";
    break;
  case CONN_TYPE_EXT_OR_LISTENER:
    type = "ExtOR";
    break;
  default:
    type = "";
    break;
  }

  if (port->explicit_addr) {
    addr = fmt_and_decorate_addr(&port->addr);
  } else {
    addr = "";
  }

  tor_snprintf(buf, sizeof(buf), "%sPort %s%s%s",
               type, addr, (strlen(addr) > 0) ? ":" : "",
               describe_portnum(port->port));
  return buf;
}

/** Return true iff port p1 is equal to p2.
 *
 * This does a field by field comparaison. */
static bool
port_cfg_eq(const port_cfg_t *p1, const port_cfg_t *p2)
{
  bool ret = true;

  tor_assert(p1);
  tor_assert(p2);

  /* Address, port and type. */
  ret &= tor_addr_eq(&p1->addr, &p2->addr);
  ret &= (p1->port == p2->port);
  ret &= (p1->type == p2->type);

  /* Mode. */
  ret &= (p1->is_unix_addr == p2->is_unix_addr);
  ret &= (p1->is_group_writable == p2->is_group_writable);
  ret &= (p1->is_world_writable == p2->is_world_writable);
  ret &= (p1->relax_dirmode_check == p2->relax_dirmode_check);
  ret &= (p1->explicit_addr == p2->explicit_addr);

  /* Entry config flags. */
  ret &= tor_memeq(&p1->entry_cfg, &p2->entry_cfg,
                    sizeof(entry_port_cfg_t));
  /* Server config flags. */
  ret &= tor_memeq(&p1->server_cfg, &p2->server_cfg,
                    sizeof(server_port_cfg_t));
  /* Unix address path if any. */
  ret &= !strcmp(p1->unix_addr, p2->unix_addr);

  return ret;
}

/** Attempt to find duplicate ORPort that would be superseded by another and
 * remove them from the given ports list. This is possible if we have for
 * instance:
 *
 *    ORPort 9050
 *    ORPort [4242::1]:9050
 *
 * First one binds to both v4 and v6 address but second one is specific to an
 * address superseding the global bind one.
 *
 * Another example is this one:
 *
 *    ORPort 9001
 *    ORPort [4242::1]:9002
 *    ORPort [4242::2]:9003
 *
 * In this case, all IPv4 and IPv6 are kept since we do allow multiple ORPorts
 * but the published port will be the first explicit one if any to be
 * published or else the implicit.
 *
 * The following is O(n^2) but it is done at bootstrap or config reload and
 * the list is not very long usually. */
STATIC void
remove_duplicate_orports(smartlist_t *ports)
{
  /* First we'll decide what to remove, then we'll remove it. */
  bool *removing = tor_calloc(smartlist_len(ports), sizeof(bool));

  for (int i = 0; i < smartlist_len(ports); ++i) {
    const port_cfg_t *current = smartlist_get(ports, i);
    if (removing[i]) {
      continue;
    }

    /* Skip non ORPorts. */
    if (current->type != CONN_TYPE_OR_LISTENER) {
      continue;
    }

    for (int j = 0; j < smartlist_len(ports); ++j) {
      const port_cfg_t *next = smartlist_get(ports, j);

      /* Avoid comparing the same object. */
      if (current == next) {
        continue;
      }
      if (removing[j]) {
        continue;
      }
      /* Skip non ORPorts. */
      if (next->type != CONN_TYPE_OR_LISTENER) {
        continue;
      }
      /* Remove duplicates. */
      if (port_cfg_eq(current, next)) {
        removing[j] = true;
        continue;
      }
      /* Don't compare addresses of different family. */
      if (tor_addr_family(&current->addr) != tor_addr_family(&next->addr)) {
        continue;
      }
      /* At this point, we have a port of the same type and same address
       * family. Now, we want to avoid comparing addresses that are different
       * but are both explicit. As an example, these are not duplicates:
       *
       *    ORPort 127.0.0.:9001 NoAdvertise
       *    ORPort 1.2.3.4:9001 NoListen
       *
       * Any implicit address must be considered for removal since an explicit
       * one will always supersedes it. */
      if (!tor_addr_eq(&current->addr, &next->addr) &&
          current->explicit_addr && next->explicit_addr) {
        continue;
      }

      /* Port value is the same so we either have a duplicate or a port that
       * supersedes another. */
      if (current->port == next->port) {
        /* Do not remove the explicit address. As stated before above, we keep
         * explicit addresses which supersedes implicit ones. */
        if (!current->explicit_addr && next->explicit_addr) {
          continue;
        }
        removing[j] = true;
        char *next_str = tor_strdup(describe_relay_port(next));
        log_warn(LD_CONFIG, "Configuration port %s superseded by %s",
                 next_str, describe_relay_port(current));
        tor_free(next_str);
      }
    }
  }

  /* Iterate over array in reverse order to keep indices valid. */
  for (int i = smartlist_len(ports)-1; i >= 0; --i) {
    tor_assert(i < smartlist_len(ports));
    if (removing[i]) {
      port_cfg_t *current = smartlist_get(ports, i);
      smartlist_del_keeporder(ports, i);
      port_cfg_free(current);
    }
  }

  tor_free(removing);
}

/** Given a list of <b>port_cfg_t</b> in <b>ports</b>, check them for internal
 * consistency and warn as appropriate.  On Unix-based OSes, set
 * *<b>n_low_ports_out</b> to the number of sub-1024 ports we will be
 * binding, and warn if we may be unable to re-bind after hibernation. */
static int
check_and_prune_server_ports(smartlist_t *ports,
                   const or_options_t *options,
                   int *n_low_ports_out)
{
  if (BUG(!ports))
    return -1;

  if (BUG(!options))
    return -1;

  if (BUG(!n_low_ports_out))
    return -1;

  int n_orport_advertised = 0;
  int n_orport_advertised_ipv4 = 0;
  int n_orport_listeners = 0;
  int n_dirport_advertised = 0;
  int n_dirport_listeners = 0;
  int n_low_port = 0;
  int r = 0;

  /* Remove possible duplicate ORPorts before inspecting the list. */
  remove_duplicate_orports(ports);

  SMARTLIST_FOREACH_BEGIN(ports, const port_cfg_t *, port) {
    if (port->type == CONN_TYPE_DIR_LISTENER) {
      if (! port->server_cfg.no_advertise)
        ++n_dirport_advertised;
      if (! port->server_cfg.no_listen)
        ++n_dirport_listeners;
    } else if (port->type == CONN_TYPE_OR_LISTENER) {
      if (! port->server_cfg.no_advertise) {
        ++n_orport_advertised;
        if (port_binds_ipv4(port))
          ++n_orport_advertised_ipv4;
      }
      if (! port->server_cfg.no_listen)
        ++n_orport_listeners;
    } else {
      continue;
    }
#ifndef _WIN32
    if (!port->server_cfg.no_listen && port->port < 1024)
      ++n_low_port;
#endif
  } SMARTLIST_FOREACH_END(port);

  if (n_orport_advertised && !n_orport_listeners) {
    log_warn(LD_CONFIG, "We are advertising an ORPort, but not actually "
             "listening on one.");
    r = -1;
  }
  if (n_orport_listeners && !n_orport_advertised) {
    log_warn(LD_CONFIG, "We are listening on an ORPort, but not advertising "
             "any ORPorts. This will keep us from building a %s "
             "descriptor, and make us impossible to use.",
             options->BridgeRelay ? "bridge" : "router");
    r = -1;
  }
  if (n_dirport_advertised && !n_dirport_listeners) {
    log_warn(LD_CONFIG, "We are advertising a DirPort, but not actually "
             "listening on one.");
    r = -1;
  }
  if (n_dirport_advertised > 1) {
    log_warn(LD_CONFIG, "Can't advertise more than one DirPort.");
    r = -1;
  }
  if (n_orport_advertised && !n_orport_advertised_ipv4 &&
      !options->BridgeRelay) {
    log_warn(LD_CONFIG, "Configured public relay to listen only on an IPv6 "
             "address. Tor needs to listen on an IPv4 address too.");
    r = -1;
  }

  if (n_low_port && options->AccountingMax &&
      (!have_capability_support() || options->KeepBindCapabilities == 0)) {
    const char *extra = "";
    if (options->KeepBindCapabilities == 0 && have_capability_support())
      extra = ", and you have disabled KeepBindCapabilities.";
    log_warn(LD_CONFIG,
          "You have set AccountingMax to use hibernation. You have also "
          "chosen a low DirPort or OrPort%s."
          "This combination can make Tor stop "
          "working when it tries to re-attach the port after a period of "
          "hibernation. Please choose a different port or turn off "
          "hibernation unless you know this combination will work on your "
          "platform.", extra);
  }

  if (n_low_ports_out)
    *n_low_ports_out = n_low_port;

  return r;
}

/** Parse all relay ports from <b>options</b>. On success, add parsed ports to
 * <b>ports</b>, and return 0.  On failure, set *<b>msg</b> to a newly
 * allocated string describing the problem, and return -1.
 **/
int
port_parse_ports_relay(or_options_t *options,
                  char **msg,
                  smartlist_t *ports_out,
                  int *have_low_ports_out)
{
  int retval = -1;
  smartlist_t *ports = smartlist_new();
  int n_low_ports = 0;

  if (BUG(!options))
    goto err;

  if (BUG(!msg))
    goto err;

  if (BUG(!ports_out))
    goto err;

  if (BUG(!have_low_ports_out))
    goto err;

  if (options->ClientOnly) {
    retval = 0;
    goto err;
  }

  if (port_parse_config(ports,
                        options->ORPort_lines,
                        "OR", CONN_TYPE_OR_LISTENER,
                        "0.0.0.0", 0,
                        CL_PORT_SERVER_OPTIONS) < 0) {
    *msg = tor_strdup("Invalid ORPort configuration");
    goto err;
  }
  if (port_parse_config(ports,
                        options->ORPort_lines,
                        "OR", CONN_TYPE_OR_LISTENER,
                        "[::]", 0,
                        CL_PORT_SERVER_OPTIONS) < 0) {
    *msg = tor_strdup("Invalid ORPort configuration");
    goto err;
  }
  if (port_parse_config(ports,
                        options->ExtORPort_lines,
                        "ExtOR", CONN_TYPE_EXT_OR_LISTENER,
                        "127.0.0.1", 0,
                        CL_PORT_SERVER_OPTIONS|CL_PORT_WARN_NONLOCAL) < 0) {
    *msg = tor_strdup("Invalid ExtORPort configuration");
    goto err;
  }
  if (port_parse_config(ports,
                        options->DirPort_lines,
                        "Dir", CONN_TYPE_DIR_LISTENER,
                        "0.0.0.0", 0,
                        CL_PORT_SERVER_OPTIONS) < 0) {
    *msg = tor_strdup("Invalid DirPort configuration");
    goto err;
  }

  if (check_and_prune_server_ports(ports, options, &n_low_ports) < 0) {
    *msg = tor_strdup("Misconfigured server ports");
    goto err;
  }

  smartlist_add_all(ports_out, ports);
  smartlist_free(ports);
  ports = NULL;
  retval = 0;

 err:
  if (*have_low_ports_out < 0)
    *have_low_ports_out = (n_low_ports > 0);
  if (ports) {
    SMARTLIST_FOREACH(ports, port_cfg_t *, p, port_cfg_free(p));
    smartlist_free(ports);
  }
  return retval;
}

/** Update the relay *Port_set values in <b>options</b> from <b>ports</b>. */
void
port_update_port_set_relay(or_options_t *options,
                      const smartlist_t *ports)
{
  if (BUG(!options))
    return;

  if (BUG(!ports))
    return;

  if (options->ClientOnly)
    return;

  /* Update the relay *Port_set options.  The !! here is to force a boolean
   * out of an integer. */
  options->ORPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_OR_LISTENER, 0);
  options->DirPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_DIR_LISTENER, 0);
  options->ExtORPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_EXT_OR_LISTENER, 0);
}

/**
 * Legacy validation function, which checks that the current OS is usable in
 * relay mode, if options is set to a relay mode.
 *
 * Warns about OSes with potential issues. Does not set *<b>msg</b>.
 * Always returns 0.
 */
int
options_validate_relay_os(const or_options_t *old_options,
                          or_options_t *options,
                          char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  if (!server_mode(options))
    return 0;

  const char *uname = get_uname();

  if (!strcmpstart(uname, "Windows 95") ||
      !strcmpstart(uname, "Windows 98") ||
      !strcmpstart(uname, "Windows Me")) {
    log_warn(LD_CONFIG, "Tor is running as a server, but you are "
        "running %s; this probably won't work. See "
        "https://www.torproject.org/docs/faq.html#BestOSForRelay "
        "for details.", uname);
  }

  return 0;
}

/**
 * Legacy validation/normalization function for the relay info options.
 * Uses old_options as the previous options.
 *
 * Returns 0 on success, returns -1 and sets *msg to a newly allocated string
 * on error.
 */
int
options_validate_relay_info(const or_options_t *old_options,
                            or_options_t *options,
                            char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  if (options->Nickname == NULL) {
    if (server_mode(options)) {
      options->Nickname = tor_strdup(UNNAMED_ROUTER_NICKNAME);
    }
  } else {
    if (!is_legal_nickname(options->Nickname)) {
      tor_asprintf(msg,
          "Nickname '%s', nicknames must be between 1 and 19 characters "
          "inclusive, and must contain only the characters [a-zA-Z0-9].",
          options->Nickname);
      return -1;
    }
  }

  if (server_mode(options) && !options->ContactInfo) {
    log_warn(LD_CONFIG,
             "Your ContactInfo config option is not set. Please strongly "
             "consider setting it, so we can contact you if your relay is "
             "misconfigured, end-of-life, or something else goes wrong. "
             "It is also possible that your relay might get rejected from "
             "the network due to a missing valid contact address.");
  }

  const char *ContactInfo = options->ContactInfo;
  if (ContactInfo && !string_is_utf8(ContactInfo, strlen(ContactInfo)))
    REJECT("ContactInfo config option must be UTF-8.");

  return 0;
}

/** Parse an authority type from <b>options</b>-\>PublishServerDescriptor
 * and write it to <b>options</b>-\>PublishServerDescriptor_. Treat "1"
 * as "v3" unless BridgeRelay is 1, in which case treat it as "bridge".
 * Treat "0" as "".
 * Return 0 on success or -1 if not a recognized authority type (in which
 * case the value of PublishServerDescriptor_ is undefined). */
static int
compute_publishserverdescriptor(or_options_t *options)
{
  smartlist_t *list = options->PublishServerDescriptor;
  dirinfo_type_t *auth = &options->PublishServerDescriptor_;
  *auth = NO_DIRINFO;
  if (!list) /* empty list, answer is none */
    return 0;
  SMARTLIST_FOREACH_BEGIN(list, const char *, string) {
    if (!strcasecmp(string, "v1"))
      log_warn(LD_CONFIG, "PublishServerDescriptor v1 has no effect, because "
                          "there are no v1 directory authorities anymore.");
    else if (!strcmp(string, "1"))
      if (options->BridgeRelay)
        *auth |= BRIDGE_DIRINFO;
      else
        *auth |= V3_DIRINFO;
    else if (!strcasecmp(string, "v2"))
      log_warn(LD_CONFIG, "PublishServerDescriptor v2 has no effect, because "
                          "there are no v2 directory authorities anymore.");
    else if (!strcasecmp(string, "v3"))
      *auth |= V3_DIRINFO;
    else if (!strcasecmp(string, "bridge"))
      *auth |= BRIDGE_DIRINFO;
    else if (!strcasecmp(string, "hidserv"))
      log_warn(LD_CONFIG,
               "PublishServerDescriptor hidserv is invalid. See "
               "PublishHidServDescriptors.");
    else if (!strcasecmp(string, "") || !strcmp(string, "0"))
      /* no authority */;
    else
      return -1;
  } SMARTLIST_FOREACH_END(string);
  return 0;
}

/**
 * Validate the configured bridge distribution method from a BridgeDistribution
 * config line.
 *
 * The input <b>bd</b>, is a string taken from the BridgeDistribution config
 * line (if present).  If the option wasn't set, return 0 immediately.  The
 * BridgeDistribution option is then validated.  Currently valid, recognised
 * options are:
 *
 * - "none"
 * - "any"
 * - "https"
 * - "email"
 * - "moat"
 *
 * If the option string is unrecognised, a warning will be logged and 0 is
 * returned.  If the option string contains an invalid character, -1 is
 * returned.
 **/
STATIC int
check_bridge_distribution_setting(const char *bd)
{
  if (bd == NULL)
    return 0;

  const char *RECOGNIZED[] = {
    "none", "any", "https", "email", "moat"
  };
  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(RECOGNIZED); ++i) {
    if (!strcasecmp(bd, RECOGNIZED[i]))
      return 0;
  }

  const char *cp = bd;
  //  Method = (KeywordChar | "_") +
  while (TOR_ISALNUM(*cp) || *cp == '-' || *cp == '_')
    ++cp;

  if (*cp == 0) {
    log_warn(LD_CONFIG, "Unrecognized BridgeDistribution value %s. I'll "
           "assume you know what you are doing...", escaped(bd));
    return 0; // we reached the end of the string; all is well
  } else {
    return -1; // we found a bad character in the string.
  }
}

/**
 * Legacy validation/normalization function for the bridge relay options.
 * Uses old_options as the previous options.
 *
 * Returns 0 on success, returns -1 and sets *msg to a newly allocated string
 * on error.
 */
int
options_validate_publish_server(const or_options_t *old_options,
                                or_options_t *options,
                                char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  if (compute_publishserverdescriptor(options) < 0) {
    tor_asprintf(msg, "Unrecognized value in PublishServerDescriptor");
    return -1;
  }

  if ((options->BridgeRelay
        || options->PublishServerDescriptor_ & BRIDGE_DIRINFO)
      && (options->PublishServerDescriptor_ & V3_DIRINFO)) {
    REJECT("Bridges are not supposed to publish router descriptors to the "
           "directory authorities. Please correct your "
           "PublishServerDescriptor line.");
  }

  if (options->BridgeDistribution) {
    if (!options->BridgeRelay) {
      REJECT("You set BridgeDistribution, but you didn't set BridgeRelay!");
    }
    if (check_bridge_distribution_setting(options->BridgeDistribution) < 0) {
      REJECT("Invalid BridgeDistribution value.");
    }
  }

  if (options->PublishServerDescriptor)
    SMARTLIST_FOREACH(options->PublishServerDescriptor, const char *, pubdes, {
      if (!strcmp(pubdes, "1") || !strcmp(pubdes, "0"))
        if (smartlist_len(options->PublishServerDescriptor) > 1) {
          COMPLAIN("You have passed a list of multiple arguments to the "
                   "PublishServerDescriptor option that includes 0 or 1. "
                   "0 or 1 should only be used as the sole argument. "
                   "This configuration will be rejected in a future release.");
          break;
        }
    });

  return 0;
}

/**
 * Legacy validation/normalization function for the relay padding options.
 * Uses old_options as the previous options.
 *
 * Returns 0 on success, returns -1 and sets *msg to a newly allocated string
 * on error.
 */
int
options_validate_relay_padding(const or_options_t *old_options,
                               or_options_t *options,
                               char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  if (!server_mode(options))
    return 0;

  if (options->ConnectionPadding != -1) {
    REJECT("Relays must use 'auto' for the ConnectionPadding setting.");
  }

  if (options->ReducedConnectionPadding != 0) {
    REJECT("Relays cannot set ReducedConnectionPadding. ");
  }

  if (options->CircuitPadding == 0) {
    REJECT("Relays cannot set CircuitPadding to 0. ");
  }

  if (options->ReducedCircuitPadding == 1) {
    REJECT("Relays cannot set ReducedCircuitPadding. ");
  }

  return 0;
}

/**
 * Legacy validation/normalization function for the relay bandwidth options.
 * Uses old_options as the previous options.
 *
 * Returns 0 on success, returns -1 and sets *msg to a newly allocated string
 * on error.
 */
int
options_validate_relay_bandwidth(const or_options_t *old_options,
                                 or_options_t *options,
                                 char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  /* 31851: the tests expect us to validate bandwidths, even when we are not
  * in relay mode. */
  if (config_ensure_bandwidth_cap(&options->MaxAdvertisedBandwidth,
                           "MaxAdvertisedBandwidth", msg) < 0)
    return -1;
  if (config_ensure_bandwidth_cap(&options->RelayBandwidthRate,
                           "RelayBandwidthRate", msg) < 0)
    return -1;
  if (config_ensure_bandwidth_cap(&options->RelayBandwidthBurst,
                           "RelayBandwidthBurst", msg) < 0)
    return -1;
  if (config_ensure_bandwidth_cap(&options->PerConnBWRate,
                           "PerConnBWRate", msg) < 0)
    return -1;
  if (config_ensure_bandwidth_cap(&options->PerConnBWBurst,
                           "PerConnBWBurst", msg) < 0)
    return -1;

  if (options->RelayBandwidthRate && !options->RelayBandwidthBurst)
    options->RelayBandwidthBurst = options->RelayBandwidthRate;
  if (options->RelayBandwidthBurst && !options->RelayBandwidthRate)
    options->RelayBandwidthRate = options->RelayBandwidthBurst;

  if (server_mode(options)) {
    const unsigned required_min_bw =
      public_server_mode(options) ?
       RELAY_REQUIRED_MIN_BANDWIDTH : BRIDGE_REQUIRED_MIN_BANDWIDTH;
    const char * const optbridge =
      public_server_mode(options) ? "" : "bridge ";
    if (options->BandwidthRate < required_min_bw) {
      tor_asprintf(msg,
                       "BandwidthRate is set to %d bytes/second. "
                       "For %sservers, it must be at least %u.",
                       (int)options->BandwidthRate, optbridge,
                       required_min_bw);
      return -1;
    } else if (options->MaxAdvertisedBandwidth <
               required_min_bw/2) {
      tor_asprintf(msg,
                       "MaxAdvertisedBandwidth is set to %d bytes/second. "
                       "For %sservers, it must be at least %u.",
                       (int)options->MaxAdvertisedBandwidth, optbridge,
                       required_min_bw/2);
      return -1;
    }
    if (options->RelayBandwidthRate &&
      options->RelayBandwidthRate < required_min_bw) {
      tor_asprintf(msg,
                       "RelayBandwidthRate is set to %d bytes/second. "
                       "For %sservers, it must be at least %u.",
                       (int)options->RelayBandwidthRate, optbridge,
                       required_min_bw);
      return -1;
    }
  }

  /* 31851: the tests expect us to validate bandwidths, even when we are not
   * in relay mode. */
  if (options->RelayBandwidthRate > options->RelayBandwidthBurst)
    REJECT("RelayBandwidthBurst must be at least equal "
           "to RelayBandwidthRate.");

  /* if they set relaybandwidth* really high but left bandwidth*
   * at the default, raise the defaults. */
  if (options->RelayBandwidthRate > options->BandwidthRate)
    options->BandwidthRate = options->RelayBandwidthRate;
  if (options->RelayBandwidthBurst > options->BandwidthBurst)
    options->BandwidthBurst = options->RelayBandwidthBurst;

  return 0;
}

/**
 * Legacy validation/normalization function for the relay bandwidth accounting
 * options. Uses old_options as the previous options.
 *
 * Returns 0 on success, returns -1 and sets *msg to a newly allocated string
 * on error.
 */
int
options_validate_relay_accounting(const or_options_t *old_options,
                                  or_options_t *options,
                                  char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  /* 31851: the tests expect us to validate accounting, even when we are not
   * in relay mode. */
  if (accounting_parse_options(options, 1)<0)
    REJECT("Failed to parse accounting options. See logs for details.");

  if (options->AccountingMax) {
    if (options->RendConfigLines && server_mode(options)) {
      log_warn(LD_CONFIG, "Using accounting with a hidden service and an "
               "ORPort is risky: your hidden service(s) and your public "
               "address will all turn off at the same time, which may alert "
               "observers that they are being run by the same party.");
    } else if (config_count_key(options->RendConfigLines,
                                "HiddenServiceDir") > 1) {
      log_warn(LD_CONFIG, "Using accounting with multiple hidden services is "
               "risky: they will all turn off at the same time, which may "
               "alert observers that they are being run by the same party.");
    }
  }

  options->AccountingRule = ACCT_MAX;
  if (options->AccountingRule_option) {
    if (!strcmp(options->AccountingRule_option, "sum"))
      options->AccountingRule = ACCT_SUM;
    else if (!strcmp(options->AccountingRule_option, "max"))
      options->AccountingRule = ACCT_MAX;
    else if (!strcmp(options->AccountingRule_option, "in"))
      options->AccountingRule = ACCT_IN;
    else if (!strcmp(options->AccountingRule_option, "out"))
      options->AccountingRule = ACCT_OUT;
    else
      REJECT("AccountingRule must be 'sum', 'max', 'in', or 'out'");
  }

  return 0;
}

/** Verify whether lst is a list of strings containing valid-looking
 * comma-separated nicknames, or NULL. Will normalise <b>lst</b> to prefix '$'
 * to any nickname or fingerprint that needs it. Also splits comma-separated
 * list elements into multiple elements. Return 0 on success.
 * Warn and return -1 on failure.
 */
static int
normalize_nickname_list(config_line_t **normalized_out,
                        const config_line_t *lst, const char *name,
                        char **msg)
{
  if (!lst)
    return 0;

  config_line_t *new_nicknames = NULL;
  config_line_t **new_nicknames_next = &new_nicknames;

  const config_line_t *cl;
  for (cl = lst; cl; cl = cl->next) {
    const char *line = cl->value;
    if (!line)
      continue;

    int valid_line = 1;
    smartlist_t *sl = smartlist_new();
    smartlist_split_string(sl, line, ",",
      SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK|SPLIT_STRIP_SPACE, 0);
    SMARTLIST_FOREACH_BEGIN(sl, char *, s)
    {
      char *normalized = NULL;
      if (!is_legal_nickname_or_hexdigest(s)) {
        // check if first char is dollar
        if (s[0] != '$') {
          // Try again but with a dollar symbol prepended
          char *prepended;
          tor_asprintf(&prepended, "$%s", s);

          if (is_legal_nickname_or_hexdigest(prepended)) {
            // The nickname is valid when it's prepended, set it as the
            // normalized version
            normalized = prepended;
          } else {
            // Still not valid, free and fallback to error message
            tor_free(prepended);
          }
        }

        if (!normalized) {
          tor_asprintf(msg, "Invalid nickname '%s' in %s line", s, name);
          valid_line = 0;
          break;
        }
      } else {
        normalized = tor_strdup(s);
      }

      config_line_t *next = tor_malloc_zero(sizeof(*next));
      next->key = tor_strdup(cl->key);
      next->value = normalized;
      next->next = NULL;

      *new_nicknames_next = next;
      new_nicknames_next = &next->next;
    } SMARTLIST_FOREACH_END(s);

    SMARTLIST_FOREACH(sl, char *, s, tor_free(s));
    smartlist_free(sl);

    if (!valid_line) {
      config_free_lines(new_nicknames);
      return -1;
    }
  }

  *normalized_out = new_nicknames;

  return 0;
}

#define ONE_MEGABYTE (UINT64_C(1) << 20)

/* If we have less than 300 MB suggest disabling dircache */
#define DIRCACHE_MIN_MEM_MB 300
#define DIRCACHE_MIN_MEM_BYTES (DIRCACHE_MIN_MEM_MB*ONE_MEGABYTE)
#define STRINGIFY(val) #val

/** Create a warning message for emitting if we are a dircache but may not have
 * enough system memory, or if we are not a dircache but probably should be.
 * Return -1 when a message is returned in *msg*, else return 0. */
STATIC int
have_enough_mem_for_dircache(const or_options_t *options, size_t total_mem,
                             char **msg)
{
  *msg = NULL;
  /* XXX We should possibly be looking at MaxMemInQueues here
   * unconditionally.  Or we should believe total_mem unconditionally. */
  if (total_mem == 0) {
    if (get_total_system_memory(&total_mem) < 0) {
      total_mem = options->MaxMemInQueues >= SIZE_MAX ?
        SIZE_MAX : (size_t)options->MaxMemInQueues;
    }
  }
  if (options->DirCache) {
    if (total_mem < DIRCACHE_MIN_MEM_BYTES) {
      if (options->BridgeRelay) {
        tor_asprintf(msg, "Running a Bridge with less than %d MB of memory "
                       "is not recommended.", DIRCACHE_MIN_MEM_MB);
      } else {
        tor_asprintf(msg, "Being a directory cache (default) with less than "
                       "%d MB of memory is not recommended and may consume "
                       "most of the available resources. Consider disabling "
                       "this functionality by setting the DirCache option "
                       "to 0.", DIRCACHE_MIN_MEM_MB);
      }
    }
  } else {
    if (total_mem >= DIRCACHE_MIN_MEM_BYTES) {
      *msg = tor_strdup("DirCache is disabled and we are configured as a "
               "relay. We will not become a Guard.");
    }
  }
  return *msg == NULL ? 0 : -1;
}
#undef STRINGIFY

/**
 * Legacy validation/normalization function for the relay mode options.
 * Uses old_options as the previous options.
 *
 * Returns 0 on success, returns -1 and sets *msg to a newly allocated string
 * on error.
 */
int
options_validate_relay_mode(const or_options_t *old_options,
                            or_options_t *options,
                            char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  if (server_mode(options) && options->RendConfigLines)
    log_warn(LD_CONFIG,
        "Tor is currently configured as a relay and a hidden service. "
        "That's not very secure: you should probably run your hidden service "
        "in a separate Tor process, at least -- see "
        "https://bugs.torproject.org/tpo/core/tor/8742.");

  if (options->BridgeRelay && options->DirPort_set) {
    log_warn(LD_CONFIG, "Can't set a DirPort on a bridge relay; disabling "
             "DirPort");
    config_free_lines(options->DirPort_lines);
    options->DirPort_lines = NULL;
    options->DirPort_set = 0;
  }

  if (options->DirPort_set && !options->DirCache) {
    REJECT("DirPort configured but DirCache disabled. DirPort requires "
           "DirCache.");
  }

  if (options->BridgeRelay && !options->DirCache) {
    REJECT("We're a bridge but DirCache is disabled. BridgeRelay requires "
           "DirCache.");
  }

  if (options->BridgeRelay == 1 && ! options->ORPort_set)
    REJECT("BridgeRelay is 1, ORPort is not set. This is an invalid "
           "combination.");

  if (server_mode(options)) {
    char *dircache_msg = NULL;
    if (have_enough_mem_for_dircache(options, 0, &dircache_msg)) {
      log_warn(LD_CONFIG, "%s", dircache_msg);
      tor_free(dircache_msg);
    }
  }

  if (options->MyFamily_lines && options->BridgeRelay) {
    log_warn(LD_CONFIG, "Listing a family for a bridge relay is not "
             "supported: it can reveal bridge fingerprints to censors. "
             "You should also make sure you aren't listing this bridge's "
             "fingerprint in any other MyFamily.");
  }
  if (options->MyFamily_lines && !options->ContactInfo) {
    log_warn(LD_CONFIG, "MyFamily is set but ContactInfo is not configured. "
             "ContactInfo should always be set when MyFamily option is too.");
  }
  if (normalize_nickname_list(&options->MyFamily,
                              options->MyFamily_lines, "MyFamily", msg))
    return -1;

  if (options->ConstrainedSockets) {
    if (options->DirPort_set) {
      /* Providing cached directory entries while system TCP buffers are scarce
       * will exacerbate the socket errors.  Suggest that this be disabled. */
      COMPLAIN("You have requested constrained socket buffers while also "
               "serving directory entries via DirPort.  It is strongly "
               "suggested that you disable serving directory requests when "
               "system TCP buffer resources are scarce.");
    }
  }

  return 0;
}

/**
 * Legacy validation/normalization function for the relay testing options
 * in options. Uses old_options as the previous options.
 *
 * Returns 0 on success, returns -1 and sets *msg to a newly allocated string
 * on error.
 */
int
options_validate_relay_testing(const or_options_t *old_options,
                               or_options_t *options,
                               char **msg)
{
  (void)old_options;

  if (BUG(!options))
    return -1;

  if (BUG(!msg))
    return -1;

  if (options->SigningKeyLifetime < options->TestingSigningKeySlop*2)
    REJECT("SigningKeyLifetime is too short.");
  if (options->TestingLinkCertLifetime < options->TestingAuthKeySlop*2)
    REJECT("LinkCertLifetime is too short.");
  if (options->TestingAuthKeyLifetime < options->TestingLinkKeySlop*2)
    REJECT("TestingAuthKeyLifetime is too short.");

  return 0;
}

/** Return 1 if any change from <b>old_options</b> to <b>new_options</b>
 * will require us to rotate the CPU and DNS workers; else return 0. */
static int
options_transition_affects_workers(const or_options_t *old_options,
                                   const or_options_t *new_options)
{
  YES_IF_CHANGED_STRING(DataDirectory);
  YES_IF_CHANGED_INT(NumCPUs);
  YES_IF_CHANGED_LINELIST(ORPort_lines);
  YES_IF_CHANGED_BOOL(ServerDNSSearchDomains);
  YES_IF_CHANGED_BOOL(SafeLogging_);
  YES_IF_CHANGED_BOOL(ClientOnly);
  YES_IF_CHANGED_BOOL(LogMessageDomains);
  YES_IF_CHANGED_LINELIST(Logs);

  if (server_mode(old_options) != server_mode(new_options) ||
      public_server_mode(old_options) != public_server_mode(new_options) ||
      dir_server_mode(old_options) != dir_server_mode(new_options))
    return 1;

  /* Nothing that changed matters. */
  return 0;
}

/** Return 1 if any change from <b>old_options</b> to <b>new_options</b>
 * will require us to generate a new descriptor; else return 0. */
static int
options_transition_affects_descriptor(const or_options_t *old_options,
                                      const or_options_t *new_options)
{
  /* XXX We can be smarter here. If your DirPort isn't being
   * published and you just turned it off, no need to republish. Etc. */

  YES_IF_CHANGED_STRING(DataDirectory);
  YES_IF_CHANGED_STRING(Nickname);
  YES_IF_CHANGED_LINELIST(Address);
  YES_IF_CHANGED_LINELIST(ExitPolicy);
  YES_IF_CHANGED_BOOL(ExitRelay);
  YES_IF_CHANGED_BOOL(ExitPolicyRejectPrivate);
  YES_IF_CHANGED_BOOL(ExitPolicyRejectLocalInterfaces);
  YES_IF_CHANGED_BOOL(IPv6Exit);
  YES_IF_CHANGED_LINELIST(ORPort_lines);
  YES_IF_CHANGED_LINELIST(DirPort_lines);
  YES_IF_CHANGED_LINELIST(DirPort_lines);
  YES_IF_CHANGED_BOOL(ClientOnly);
  YES_IF_CHANGED_BOOL(DisableNetwork);
  YES_IF_CHANGED_BOOL(PublishServerDescriptor_);
  YES_IF_CHANGED_STRING(ContactInfo);
  YES_IF_CHANGED_STRING(BridgeDistribution);
  YES_IF_CHANGED_LINELIST(MyFamily);
  YES_IF_CHANGED_STRING(AccountingStart);
  YES_IF_CHANGED_INT(AccountingMax);
  YES_IF_CHANGED_INT(AccountingRule);
  YES_IF_CHANGED_BOOL(DirCache);
  YES_IF_CHANGED_BOOL(AssumeReachable);

  if (relay_get_effective_bwrate(old_options) !=
        relay_get_effective_bwrate(new_options) ||
      relay_get_effective_bwburst(old_options) !=
        relay_get_effective_bwburst(new_options) ||
      public_server_mode(old_options) != public_server_mode(new_options))
    return 1;

  return 0;
}

/** Fetch the active option list, and take relay actions based on it. All of
 * the things we do should survive being done repeatedly.  If present,
 * <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_relay(const or_options_t *old_options)
{
  const or_options_t *options = get_options();

  const int transition_affects_workers =
    old_options && options_transition_affects_workers(old_options, options);

  /* We want to reinit keys as needed before we do much of anything else:
     keys are important, and other things can depend on them. */
  if (transition_affects_workers ||
      (authdir_mode_v3(options) && (!old_options ||
                                    !authdir_mode_v3(old_options)))) {
    if (init_keys() < 0) {
      log_warn(LD_BUG,"Error initializing keys; exiting");
      return -1;
    }
  }

  if (server_mode(options)) {
    static int cdm_initialized = 0;
    if (cdm_initialized == 0) {
      cdm_initialized = 1;
      consdiffmgr_configure(NULL);
      consdiffmgr_validate();
    }
  }

  /* Check for transitions that need action. */
  if (old_options) {
    if (transition_affects_workers) {
      log_info(LD_GENERAL,
               "Worker-related options changed. Rotating workers.");
      const int server_mode_turned_on =
        server_mode(options) && !server_mode(old_options);
      const int dir_server_mode_turned_on =
        dir_server_mode(options) && !dir_server_mode(old_options);

      if (server_mode_turned_on || dir_server_mode_turned_on) {
        cpu_init();
      }

      if (server_mode_turned_on) {
        ip_address_changed(0);
      }
      cpuworkers_rotate_keyinfo();
    }
  }

  return 0;
}

/** Fetch the active option list, and take relay accounting actions based on
 * it. All of the things we do should survive being done repeatedly. If
 * present, <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_relay_accounting(const or_options_t *old_options)
{
  (void)old_options;

  const or_options_t *options = get_options();

  /* Set up accounting */
  if (accounting_parse_options(options, 0)<0) {
    // LCOV_EXCL_START
    log_warn(LD_BUG,"Error in previously validated accounting options");
    return -1;
    // LCOV_EXCL_STOP
  }
  if (accounting_is_enabled(options))
    configure_accounting(time(NULL));

  return 0;
}

/** Fetch the active option list, and take relay bandwidth actions based on
 * it. All of the things we do should survive being done repeatedly. If
 * present, <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_relay_bandwidth(const or_options_t *old_options)
{
  const or_options_t *options = get_options();

  /* Check for transitions that need action. */
  if (old_options) {
    if (options->PerConnBWRate != old_options->PerConnBWRate ||
        options->PerConnBWBurst != old_options->PerConnBWBurst)
      connection_or_update_token_buckets(get_connection_array(), options);

    if (options->RelayBandwidthRate != old_options->RelayBandwidthRate ||
        options->RelayBandwidthBurst != old_options->RelayBandwidthBurst)
      connection_bucket_adjust(options);
  }

  return 0;
}

/** Fetch the active option list, and take bridge statistics actions based on
 * it. All of the things we do should survive being done repeatedly. If
 * present, <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_bridge_stats(const or_options_t *old_options)
{
  const or_options_t *options = get_options();

/* How long should we delay counting bridge stats after becoming a bridge?
 * We use this so we don't count clients who used our bridge thinking it is
 * a relay. If you change this, don't forget to change the log message
 * below. It's 4 hours (the time it takes to stop being used by clients)
 * plus some extra time for clock skew. */
#define RELAY_BRIDGE_STATS_DELAY (6 * 60 * 60)

  /* Check for transitions that need action. */
  if (old_options) {
    if (! bool_eq(options->BridgeRelay, old_options->BridgeRelay)) {
      int was_relay = 0;
      if (options->BridgeRelay) {
        time_t int_start = time(NULL);
        if (config_lines_eq(old_options->ORPort_lines,options->ORPort_lines)) {
          int_start += RELAY_BRIDGE_STATS_DELAY;
          was_relay = 1;
        }
        geoip_bridge_stats_init(int_start);
        log_info(LD_CONFIG, "We are acting as a bridge now.  Starting new "
                 "GeoIP stats interval%s.", was_relay ? " in 6 "
                 "hours from now" : "");
      } else {
        geoip_bridge_stats_term();
        log_info(LD_GENERAL, "We are no longer acting as a bridge.  "
                 "Forgetting GeoIP stats.");
      }
    }
  }

  return 0;
}

/** Fetch the active option list, and take relay statistics actions based on
 * it. All of the things we do should survive being done repeatedly. If
 * present, <b>old_options</b> contains the previous value of the options.
 *
 * Sets <b>*print_notice_out</b> if we enabled stats, and need to print
 * a stats log using options_act_relay_stats_msg().
 *
 * If loading the GeoIP file failed, sets DirReqStatistics and
 * EntryStatistics to 0. This breaks the normalization/act ordering
 * introduced in 29211.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_relay_stats(const or_options_t *old_options,
                        bool *print_notice_out)
{
  if (BUG(!print_notice_out))
    return -1;

  or_options_t *options = get_options_mutable();

  if (options->CellStatistics || options->DirReqStatistics ||
      options->EntryStatistics || options->ExitPortStatistics ||
      options->ConnDirectionStatistics ||
      options->HiddenServiceStatistics) {
    time_t now = time(NULL);
    int print_notice = 0;

    if ((!old_options || !old_options->CellStatistics) &&
        options->CellStatistics) {
      rep_hist_buffer_stats_init(now);
      print_notice = 1;
    }
    if ((!old_options || !old_options->DirReqStatistics) &&
        options->DirReqStatistics) {
      if (geoip_is_loaded(AF_INET)) {
        geoip_dirreq_stats_init(now);
        print_notice = 1;
      } else {
        /* disable statistics collection since we have no geoip file */
        /* 29211: refactor to avoid the normalisation/act inversion */
        options->DirReqStatistics = 0;
        if (options->ORPort_set)
          log_notice(LD_CONFIG, "Configured to measure directory request "
                                "statistics, but no GeoIP database found. "
                                "Please specify a GeoIP database using the "
                                "GeoIPFile option.");
      }
    }
    if ((!old_options || !old_options->EntryStatistics) &&
        options->EntryStatistics && !should_record_bridge_info(options)) {
      /* If we get here, we've started recording bridge info when we didn't
       * do so before.  Note that "should_record_bridge_info()" will
       * always be false at this point, because of the earlier block
       * that cleared EntryStatistics when public_server_mode() was false.
       * We're leaving it in as defensive programming. */
      if (geoip_is_loaded(AF_INET) || geoip_is_loaded(AF_INET6)) {
        geoip_entry_stats_init(now);
        print_notice = 1;
      } else {
        options->EntryStatistics = 0;
        log_notice(LD_CONFIG, "Configured to measure entry node "
                              "statistics, but no GeoIP database found. "
                              "Please specify a GeoIP database using the "
                              "GeoIPFile option.");
      }
    }
    if ((!old_options || !old_options->ExitPortStatistics) &&
        options->ExitPortStatistics) {
      rep_hist_exit_stats_init(now);
      print_notice = 1;
    }
    if ((!old_options || !old_options->ConnDirectionStatistics) &&
        options->ConnDirectionStatistics) {
      conn_stats_init(now);
    }
    if ((!old_options || !old_options->HiddenServiceStatistics) &&
        options->HiddenServiceStatistics) {
      log_info(LD_CONFIG, "Configured to measure hidden service statistics.");
      rep_hist_hs_stats_init(now);
    }
    if (print_notice)
      *print_notice_out = 1;
  }

  /* If we used to have statistics enabled but we just disabled them,
     stop gathering them.  */
  if (old_options && old_options->CellStatistics &&
      !options->CellStatistics)
    rep_hist_buffer_stats_term();
  if (old_options && old_options->DirReqStatistics &&
      !options->DirReqStatistics)
    geoip_dirreq_stats_term();
  if (old_options && old_options->EntryStatistics &&
      !options->EntryStatistics)
    geoip_entry_stats_term();
  if (old_options && old_options->HiddenServiceStatistics &&
      !options->HiddenServiceStatistics)
    rep_hist_hs_stats_term();
  if (old_options && old_options->ExitPortStatistics &&
      !options->ExitPortStatistics)
    rep_hist_exit_stats_term();
  if (old_options && old_options->ConnDirectionStatistics &&
      !options->ConnDirectionStatistics)
    conn_stats_terminate();

  return 0;
}

/** Print a notice about relay/dirauth stats being enabled. */
void
options_act_relay_stats_msg(void)
{
  log_notice(LD_CONFIG, "Configured to measure statistics. Look for "
             "the *-stats files that will first be written to the "
             "data directory in 24 hours from now.");
}

/** Fetch the active option list, and take relay descriptor actions based on
 * it. All of the things we do should survive being done repeatedly. If
 * present, <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_relay_desc(const or_options_t *old_options)
{
  const or_options_t *options = get_options();

  /* Since our options changed, we might need to regenerate and upload our
   * server descriptor.
   */
  if (!old_options ||
      options_transition_affects_descriptor(old_options, options))
    mark_my_descriptor_dirty("config change");

  return 0;
}

/** Fetch the active option list, and take relay DoS actions based on
 * it. All of the things we do should survive being done repeatedly. If
 * present, <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_relay_dos(const or_options_t *old_options)
{
  const or_options_t *options = get_options();

  /* DoS mitigation subsystem only applies to public relay. */
  if (public_server_mode(options)) {
    /* If we are configured as a relay, initialize the subsystem. Even on HUP,
     * this is safe to call as it will load data from the current options
     * or/and the consensus. */
    dos_init();
  } else if (old_options && public_server_mode(old_options)) {
    /* Going from relay to non relay, clean it up. */
    dos_free_all();
  }

  return 0;
}

/** Fetch the active option list, and take dirport actions based on
 * it. All of the things we do should survive being done repeatedly. If
 * present, <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * into the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
int
options_act_relay_dir(const or_options_t *old_options)
{
  (void)old_options;

  const or_options_t *options = get_options();

  if (!public_server_mode(options))
    return 0;

  /* Load the webpage we're going to serve every time someone asks for '/' on
     our DirPort. */
  tor_free(global_dirfrontpagecontents);
  if (options->DirPortFrontPage) {
    global_dirfrontpagecontents =
      read_file_to_str(options->DirPortFrontPage, 0, NULL);
    if (!global_dirfrontpagecontents) {
      log_warn(LD_CONFIG,
               "DirPortFrontPage file '%s' not found. Continuing anyway.",
               options->DirPortFrontPage);
    }
  }

  return 0;
}
