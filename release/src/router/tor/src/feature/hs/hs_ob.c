/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_ob.c
 * \brief Implement Onion Balance specific code.
 **/

#define HS_OB_PRIVATE

#include "feature/hs/hs_service.h"

#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/networkstatus_st.h"

#include "lib/confmgt/confmgt.h"
#include "lib/encoding/confline.h"

#include "feature/hs/hs_ob.h"

/* Options config magic number. */
#define OB_OPTIONS_MAGIC 0x631DE7EA

/* Helper macros. */
#define VAR(varname, conftype, member, initvalue)                          \
  CONFIG_VAR_ETYPE(ob_options_t, varname, conftype, member, 0, initvalue)
#define V(member,conftype,initvalue)        \
  VAR(#member, conftype, member, initvalue)

/* Dummy instance of ob_options_t, used for type-checking its members with
 * CONF_CHECK_VAR_TYPE. */
DUMMY_TYPECHECK_INSTANCE(ob_options_t);

/* Array of variables for the config file options. */
static const config_var_t config_vars[] = {
  V(MasterOnionAddress, LINELIST, NULL),

  END_OF_CONFIG_VARS
};

/* "Extra" variable in the state that receives lines we can't parse. This
 * lets us preserve options from versions of Tor newer than us. */
static const struct_member_t config_extra_vars = {
  .name = "__extra",
  .type = CONFIG_TYPE_LINELIST,
  .offset = offsetof(ob_options_t, ExtraLines),
};

/* Configuration format of ob_options_t. */
static const config_format_t config_format = {
  .size = sizeof(ob_options_t),
  .magic = {
     "ob_options_t",
     OB_OPTIONS_MAGIC,
     offsetof(ob_options_t, magic_),
  },
  .vars = config_vars,
  .extra = &config_extra_vars,
};

/* Global configuration manager for the config file. */
static config_mgr_t *config_options_mgr = NULL;

/* Return the configuration manager for the config file. */
static const config_mgr_t *
get_config_options_mgr(void)
{
  if (PREDICT_UNLIKELY(config_options_mgr == NULL)) {
    config_options_mgr = config_mgr_new(&config_format);
    config_mgr_freeze(config_options_mgr);
  }
  return config_options_mgr;
}

#define ob_option_free(val) \
  FREE_AND_NULL(ob_options_t, ob_option_free_, (val))

/** Helper: Free a config options object. */
static void
ob_option_free_(ob_options_t *opts)
{
  if (opts == NULL) {
    return;
  }
  config_free(get_config_options_mgr(), opts);
}

/** Return an allocated config options object. */
static ob_options_t *
ob_option_new(void)
{
  ob_options_t *opts = config_new(get_config_options_mgr());
  config_init(get_config_options_mgr(), opts);
  return opts;
}

/** Helper function: From the configuration line value which is an onion
 * address with the ".onion" extension, find the public key and put it in
 * pkey_out.
 *
 * On success, true is returned. Else, false and pkey is untouched. */
static bool
get_onion_public_key(const char *value, ed25519_public_key_t *pkey_out)
{
  char address[HS_SERVICE_ADDR_LEN_BASE32 + 1];

  tor_assert(value);
  tor_assert(pkey_out);

  if (strcmpend(value, ".onion")) {
    /* Not a .onion extension, bad format. */
    return false;
  }

  /* Length validation. The -1 is because sizeof() counts the NUL byte. */
  if (strlen(value) >
      (HS_SERVICE_ADDR_LEN_BASE32 + sizeof(".onion") - 1)) {
    /* Too long, bad format. */
    return false;
  }

  /* We don't want the .onion so we add 2 because size - 1 is copied with
   * strlcpy() in order to accommodate the NUL byte and sizeof() counts the NUL
   * byte so we need to remove them from the equation. */
  strlcpy(address, value, strlen(value) - sizeof(".onion") + 2);

  if (hs_parse_address_no_log(address, pkey_out, NULL, NULL, NULL) < 0) {
    return false;
  }

  /* Success. */
  return true;
}

/** Parse the given ob options in opts and set the service config object
 * accordingly.
 *
 * Return 1 on success else 0. */
static int
ob_option_parse(hs_service_config_t *config, const ob_options_t *opts)
{
  int ret = 0;
  config_line_t *line;

  tor_assert(config);
  tor_assert(opts);

  for (line = opts->MasterOnionAddress; line; line = line->next) {
    /* Allocate config list if need be. */
    if (!config->ob_master_pubkeys) {
      config->ob_master_pubkeys = smartlist_new();
    }
    ed25519_public_key_t *pubkey = tor_malloc_zero(sizeof(*pubkey));

    if (!get_onion_public_key(line->value, pubkey)) {
      log_warn(LD_REND, "OnionBalance: MasterOnionAddress %s is invalid",
               line->value);
      tor_free(pubkey);
      goto end;
    }
    smartlist_add(config->ob_master_pubkeys, pubkey);
    log_notice(LD_REND, "OnionBalance: MasterOnionAddress %s registered",
               line->value);
  }
  /* Success. */
  ret = 1;

 end:
  /* No keys added, we free the list since no list means no onion balance
   * support for this tor instance. */
  if (smartlist_len(config->ob_master_pubkeys) == 0) {
    smartlist_free(config->ob_master_pubkeys);
  }
  return ret;
}

/** For the given master public key and time period, compute the subcredential
 * and put them into subcredential. The subcredential parameter needs to be at
 * least DIGEST256_LEN in size. */
static void
build_subcredential(const ed25519_public_key_t *pkey, uint64_t tp,
                    hs_subcredential_t *subcredential)
{
  ed25519_public_key_t blinded_pubkey;

  tor_assert(pkey);
  tor_assert(subcredential);

  hs_build_blinded_pubkey(pkey, NULL, 0, tp, &blinded_pubkey);
  hs_get_subcredential(pkey, &blinded_pubkey, subcredential);
}

/*
 * Public API.
 */

/** Return true iff the given service is configured as an onion balance
 * instance. To satisfy that condition, there must at least be one master
 * ed25519 public key configured. */
bool
hs_ob_service_is_instance(const hs_service_t *service)
{
  if (BUG(service == NULL)) {
    return false;
  }

  /* No list, we are not an instance. */
  if (!service->config.ob_master_pubkeys) {
    return false;
  }

  return smartlist_len(service->config.ob_master_pubkeys) > 0;
}

/** Read and parse the config file at fname on disk. The service config object
 * is populated with the options if any.
 *
 * Return 1 on success else 0. This is to follow the "ok" convention in
 * hs_config.c. */
int
hs_ob_parse_config_file(hs_service_config_t *config)
{
  static const char *fname = "ob_config";
  int ret = 0;
  char *content = NULL, *errmsg = NULL, *config_file_path = NULL;
  ob_options_t *options = NULL;
  config_line_t *lines = NULL;

  tor_assert(config);

  /* Read file from disk. */
  config_file_path = hs_path_from_filename(config->directory_path, fname);
  content = read_file_to_str(config_file_path, 0, NULL);
  if (!content) {
    log_warn(LD_FS, "OnionBalance: Unable to read config file %s",
             escaped(config_file_path));
    goto end;
  }

  /* Parse lines. */
  if (config_get_lines(content, &lines, 0) < 0) {
    goto end;
  }

  options = ob_option_new();
  config_assign(get_config_options_mgr(), options, lines, 0, &errmsg);
  if (errmsg) {
    log_warn(LD_REND, "OnionBalance: Unable to parse config file: %s",
             errmsg);
    tor_free(errmsg);
    goto end;
  }

  /* Parse the options and set the service config object with the details. */
  ret = ob_option_parse(config, options);

 end:
  config_free_lines(lines);
  ob_option_free(options);
  tor_free(content);
  tor_free(config_file_path);
  return ret;
}

/** Compute all possible subcredentials for every onion master key in the given
 * service config object. subcredentials_out is allocated and set as an
 * continuous array containing all possible values.
 *
 * On success, return the number of subcredential put in the array which will
 * correspond to an array of size: n * DIGEST256_LEN where DIGEST256_LEN is the
 * length of a single subcredential.
 *
 * If the given configuration object has no OB master keys configured, 0 is
 * returned and subcredentials_out is set to NULL.
 *
 * Otherwise, this can't fail. */
STATIC size_t
compute_subcredentials(const hs_service_t *service,
                       hs_subcredential_t **subcredentials_out)
{
  unsigned int num_pkeys, idx = 0;
  hs_subcredential_t *subcreds = NULL;
  const int steps[3] = {0, -1, 1};
  const unsigned int num_steps = ARRAY_LENGTH(steps);
  const uint64_t tp = hs_get_time_period_num(0);

  tor_assert(service);
  tor_assert(subcredentials_out);
  /* Our caller has checked these too */
  tor_assert(service->desc_current);
  tor_assert(service->desc_next);

  /* Make sure we are an OB instance, or bail out. */
  num_pkeys = smartlist_len(service->config.ob_master_pubkeys);
  if (!num_pkeys) {
    *subcredentials_out = NULL;
    return 0;
  }

  /* Time to build all the subcredentials for each time period: two for each
   * instance descriptor plus three for the onionbalance frontend service: the
   * previous one (-1), the current one (0) and the next one (1) for each
   * configured key in order to accommodate client and service consensus skew.
   *
   * If the client consensus after_time is at 23:00 but the service one is at
   * 01:00, the client will be using the previous time period where the
   * service will think it is the client next time period. Thus why we have
   * to try them all.
   *
   * The normal use case works because the service gets the descriptor object
   * that corresponds to the intro point's request, and because each
   * descriptor corresponds to a specific subcredential, we get the right
   * subcredential out of it, and use that to do the decryption.
   *
   * As a slight optimization, statistically, the current time period (0) will
   * be the one to work first so we'll put them first in the array to maximize
   * our chance of success. */

  /* We use a flat array, not a smartlist_t, in order to minimize memory
   * allocation.
   *
   * Size of array is: length of a single subcredential multiplied by the
   * number of time period we need to compute and finally multiplied by the
   * total number of keys we are about to process. In other words, for each
   * key, we allocate 3 subcredential slots. Then in the end we also add two
   * subcredentials for this instance's active descriptors. */
  subcreds =
    tor_calloc((num_steps * num_pkeys) + 2, sizeof(hs_subcredential_t));

  /* For each master pubkey we add 3 subcredentials: */
  for (unsigned int i = 0; i < num_steps; i++) {
    SMARTLIST_FOREACH_BEGIN(service->config.ob_master_pubkeys,
                            const ed25519_public_key_t *, pkey) {
      build_subcredential(pkey, tp + steps[i], &subcreds[idx]);
      idx++;
    } SMARTLIST_FOREACH_END(pkey);
  }

  /* And then in the end we add the two subcredentials of the current active
   * instance descriptors */
  memcpy(&subcreds[idx++], &service->desc_current->desc->subcredential,
         sizeof(hs_subcredential_t));
  memcpy(&subcreds[idx++], &service->desc_next->desc->subcredential,
         sizeof(hs_subcredential_t));

  log_info(LD_REND, "Refreshing %u onionbalance keys (TP #%d).",
             idx, (int)tp);

  *subcredentials_out = subcreds;
  return idx;
}

/**
 *  If we are an Onionbalance instance, refresh our keys.
 *
 *  If we are not an Onionbalance instance or we are not ready to do so, this
 *  is a NOP.
 *
 *  This function is called every time we build a new descriptor. That's
 *  because we want our Onionbalance keys to always use up-to-date
 *  subcredentials both for the instance (ourselves) and for the onionbalance
 *  frontend.
 */
void
hs_ob_refresh_keys(hs_service_t *service)
{
  hs_subcredential_t *ob_subcreds = NULL;
  size_t num_subcreds;

  tor_assert(service);

  /* Don't do any of this if we are not configured as an OB instance */
  if (!hs_ob_service_is_instance(service)) {
    return;
  }

  /* We need both service descriptors created to make onionbalance keys.
   *
   * That's because we fetch our own (the instance's) subcredentials from our
   * own descriptors which should always include the latest subcredentials that
   * clients would use.
   *
   * This function is called with each descriptor build, so we will be
   * eventually be called when both descriptors are created. */
  if (!service->desc_current || !service->desc_next) {
    return;
  }

  /* Get a new set of subcreds */
  num_subcreds = compute_subcredentials(service, &ob_subcreds);
  if (BUG(!num_subcreds)) {
    return;
  }

  /* Delete old subcredentials if any */
  if (service->state.ob_subcreds) {
    tor_free(service->state.ob_subcreds);
  }

  service->state.ob_subcreds = ob_subcreds;
  service->state.n_ob_subcreds = num_subcreds;
}

/** Free any memory allocated by the onionblance subsystem. */
void
hs_ob_free_all(void)
{
  config_mgr_free(config_options_mgr);
}
