/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_config.c
 * \brief Implement hidden service configuration subsystem.
 *
 * \details
 *
 * This file has basically one main entry point: hs_config_service_all(). It
 * takes the torrc options and configure hidden service from it. In validate
 * mode, nothing is added to the global service list or keys are not generated
 * nor loaded.
 *
 * A service is configured in two steps. It is first created using the tor
 * options and then put in a staging list. It will stay there until
 * hs_service_load_all_keys() is called. That function is responsible to
 * load/generate the keys for the service in the staging list and if
 * successful, transferred the service to the main global service list where
 * at that point it is ready to be used.
 *
 * Configuration functions are per-version and there is a main generic one for
 * every option that is common to all version (config_generic_service).
 **/

#include "feature/hs/hs_common.h"
#include "feature/hs/hs_config.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_ob.h"
#include "feature/hs/hs_service.h"
#include "feature/rend/rendclient.h"
#include "feature/rend/rendservice.h"
#include "lib/encoding/confline.h"
#include "lib/conf/confdecl.h"
#include "lib/confmgt/confmgt.h"

#include "feature/hs/hs_opts_st.h"
#include "app/config/or_options_st.h"

/* Declare the table mapping hs options to hs_opts_t */
#define CONF_CONTEXT TABLE
#include "feature/hs/hs_options.inc"
#undef CONF_CONTEXT

/** Magic number for hs_opts_t. */
#define HS_OPTS_MAGIC 0x6f6e796e

static const config_format_t hs_opts_fmt = {
  .size = sizeof(hs_opts_t),
  .magic = { "hs_opts_t",
             HS_OPTS_MAGIC,
             offsetof(hs_opts_t, magic) },
  .vars = hs_opts_t_vars,
};

/** Global configuration manager to handle HS sections*/
static config_mgr_t *hs_opts_mgr = NULL;

/**
 * Return a configuration manager for the hs_opts_t configuration type.
 **/
static const config_mgr_t *
get_hs_opts_mgr(void)
{
  if (PREDICT_UNLIKELY(hs_opts_mgr == NULL)) {
    hs_opts_mgr = config_mgr_new(&hs_opts_fmt);
    config_mgr_freeze(hs_opts_mgr);
  }
  return hs_opts_mgr;
}

/**
 * Allocate, initialize, and return a new hs_opts_t.
 **/
static hs_opts_t *
hs_opts_new(void)
{
  const config_mgr_t *mgr = get_hs_opts_mgr();
  hs_opts_t *r = config_new(mgr);
  tor_assert(r);
  config_init(mgr, r);
  return r;
}

/**
 * Free an hs_opts_t.
 **/
#define hs_opts_free(opts) \
  config_free(get_hs_opts_mgr(), (opts))

/** Using the given list of services, stage them into our global state. Every
 * service version are handled. This function can remove entries in the given
 * service_list.
 *
 * Staging a service means that we take all services in service_list and we
 * put them in the staging list (global) which acts as a temporary list that
 * is used by the service loading key process. In other words, staging a
 * service puts it in a list to be considered when loading the keys and then
 * moved to the main global list. */
static void
stage_services(smartlist_t *service_list)
{
  tor_assert(service_list);

  /* This is v2 specific. Trigger service pruning which will make sure the
   * just configured services end up in the main global list. It should only
   * be done in non validation mode because v2 subsystem handles service
   * object differently. */
  rend_service_prune_list();

  /* Cleanup v2 service from the list, we don't need those object anymore
   * because we validated them all against the others and we want to stage
   * only >= v3 service. And remember, v2 has a different object type which is
   * shadow copied from an hs_service_t type. */
  SMARTLIST_FOREACH_BEGIN(service_list, hs_service_t *, s) {
    if (s->config.version == HS_VERSION_TWO) {
      SMARTLIST_DEL_CURRENT(service_list, s);
      hs_service_free(s);
    }
  } SMARTLIST_FOREACH_END(s);

  /* This is >= v3 specific. Using the newly configured service list, stage
   * them into our global state. Every object ownership is lost after. */
  hs_service_stage_services(service_list);
}

/** Validate the given service against all service in the given list. If the
 * service is ephemeral, this function ignores it. Services with the same
 * directory path aren't allowed and will return an error. If a duplicate is
 * found, 1 is returned else 0 if none found. */
static int
service_is_duplicate_in_list(const smartlist_t *service_list,
                             const hs_service_t *service)
{
  int ret = 0;

  tor_assert(service_list);
  tor_assert(service);

  /* Ephemeral service don't have a directory configured so no need to check
   * for a service in the list having the same path. */
  if (service->config.is_ephemeral) {
    goto end;
  }

  /* XXX: Validate if we have any service that has the given service dir path.
   * This has two problems:
   *
   * a) It's O(n^2), but the same comment from the bottom of
   *    rend_config_services() should apply.
   *
   * b) We only compare directory paths as strings, so we can't
   *    detect two distinct paths that specify the same directory
   *    (which can arise from symlinks, case-insensitivity, bind
   *    mounts, etc.).
   *
   * It also can't detect that two separate Tor instances are trying
   * to use the same HiddenServiceDir; for that, we would need a
   * lock file.  But this is enough to detect a simple mistake that
   * at least one person has actually made. */
  SMARTLIST_FOREACH_BEGIN(service_list, const hs_service_t *, s) {
    if (!strcmp(s->config.directory_path, service->config.directory_path)) {
      log_warn(LD_REND, "Another hidden service is already configured "
                        "for directory %s",
               escaped(service->config.directory_path));
      ret = 1;
      goto end;
    }
  } SMARTLIST_FOREACH_END(s);

 end:
  return ret;
}

/** Check whether an integer <b>i</b> is out of bounds (not between <b>low</b>
 * and <b>high</b> incusive).  If it is, then log a warning about the option
 * <b>name</b>, and return true. Otherwise return false. */
static bool
check_value_oob(int i, const char *name, int low, int high)
{
  if (i < low || i > high) {
    log_warn(LD_CONFIG, "%s must be between %d and %d, not %d.",
             name, low, high, i);
    return true;
  }
  return false;
}

/**
 * Helper: check whether the integer value called <b>name</b> in <b>opts</b>
 * is out-of-bounds.
 **/
#define CHECK_OOB(opts, name, low, high)      \
  check_value_oob((opts)->name, #name, (low), (high))

/** Helper function: Given a configuration option and its value, parse the
 * value as a hs_circuit_id_protocol_t. On success, ok is set to 1 and ret is
 * the parse value. On error, ok is set to 0 and the "none"
 * hs_circuit_id_protocol_t is returned. This function logs on error. */
static hs_circuit_id_protocol_t
helper_parse_circuit_id_protocol(const char *key, const char *value, int *ok)
{
  tor_assert(value);
  tor_assert(ok);

  hs_circuit_id_protocol_t ret = HS_CIRCUIT_ID_PROTOCOL_NONE;
  *ok = 0;

  if (! strcasecmp(value, "haproxy")) {
    *ok = 1;
    ret = HS_CIRCUIT_ID_PROTOCOL_HAPROXY;
  } else if (! strcasecmp(value, "none")) {
    *ok = 1;
    ret = HS_CIRCUIT_ID_PROTOCOL_NONE;
  } else {
    log_warn(LD_CONFIG, "%s must be 'haproxy' or 'none'.", key);
    goto err;
  }

 err:
  return ret;
}

/** Return the service version by trying to learn it from the key on disk if
 * any. If nothing is found, the current service configured version is
 * returned. */
static int
config_learn_service_version(hs_service_t *service)
{
  int version;

  tor_assert(service);

  version = hs_service_get_version_from_key(service);
  if (version < 0) {
    version = service->config.version;
  }

  return version;
}

/**
 * Header key indicating the start of a new hidden service configuration
 * block.
 **/
static const char SECTION_HEADER[] = "HiddenServiceDir";

/** Return true iff the given options starting at line_ for a hidden service
 * contains at least one invalid option. Each hidden service option don't
 * apply to all versions so this function can find out. The line_ MUST start
 * right after the HiddenServiceDir line of this service.
 *
 * This is mainly for usability so we can inform the user of any invalid
 * option for the hidden service version instead of silently ignoring. */
static int
config_has_invalid_options(const config_line_t *line_,
                           const hs_service_t *service)
{
  int ret = 0;
  const char **optlist;
  const config_line_t *line;

  tor_assert(service);
  tor_assert(service->config.version <= HS_VERSION_MAX);

  /* List of options that a v3 service doesn't support thus must exclude from
   * its configuration. */
  const char *opts_exclude_v3[] = {
    "HiddenServiceAuthorizeClient",
    NULL /* End marker. */
  };

  const char *opts_exclude_v2[] = {
    "HiddenServiceExportCircuitID",
    "HiddenServiceEnableIntroDoSDefense",
    "HiddenServiceEnableIntroDoSRatePerSec",
    "HiddenServiceEnableIntroDoSBurstPerSec",
    "HiddenServiceOnionBalanceInstance",
    NULL /* End marker. */
  };

  /* Defining the size explicitly allows us to take advantage of the compiler
   * which warns us if we ever bump the max version but forget to grow this
   * array. The plus one is because we have a version 0 :). */
  struct {
    const char **list;
  } exclude_lists[HS_VERSION_MAX + 1] = {
    { NULL }, /* v0. */
    { NULL }, /* v1. */
    { opts_exclude_v2 }, /* v2 */
    { opts_exclude_v3 }, /* v3. */
  };

  optlist = exclude_lists[service->config.version].list;
  if (optlist == NULL) {
    /* No exclude options to look at for this version. */
    goto end;
  }
  for (int i = 0; optlist[i]; i++) {
    const char *opt = optlist[i];
    for (line = line_; line; line = line->next) {
      if (!strcasecmp(line->key, SECTION_HEADER)) {
        /* We just hit the next hidden service, stop right now.
         * (This shouldn't be possible, now that we have partitioned the list
         * into sections.) */
        tor_assert_nonfatal_unreached();
        goto end;
      }
      if (!strcasecmp(line->key, opt)) {
        log_warn(LD_CONFIG, "Hidden service option %s is incompatible with "
                            "version %" PRIu32 " of service in %s",
                 opt, service->config.version,
                 service->config.directory_path);

        if (!strcasecmp(line->key, "HiddenServiceAuthorizeClient")) {
          /* Special case this v2 option so that we can offer alternatives.
           * If more such special cases appear, it would be good to
           * generalize the exception mechanism here. */
          log_warn(LD_CONFIG, "For v3 onion service client authorization, "
                   "please read the 'CLIENT AUTHORIZATION' section in the "
                   "manual.");
        }

        ret = 1;
        /* Continue the loop so we can find all possible options. */
        continue;
      }
    }
  }
 end:
  return ret;
}

/** Validate service configuration. This is used when loading the configuration
 * and once we've setup a service object, it's config object is passed to this
 * function for further validation. This does not validate service key
 * material. Return 0 if valid else -1 if invalid. */
static int
config_validate_service(const hs_service_config_t *config)
{
  tor_assert(config);

  /* Amount of ports validation. */
  if (!config->ports || smartlist_len(config->ports) == 0) {
    log_warn(LD_CONFIG, "Hidden service (%s) with no ports configured.",
             escaped(config->directory_path));
    goto invalid;
  }

  /* DoS validation values. */
  if (config->has_dos_defense_enabled &&
      (config->intro_dos_burst_per_sec < config->intro_dos_rate_per_sec)) {
    log_warn(LD_CONFIG, "Hidden service DoS defenses burst (%" PRIu32 ") can "
                        "not be smaller than the rate value (%" PRIu32 ").",
             config->intro_dos_burst_per_sec, config->intro_dos_rate_per_sec);
    goto invalid;
  }

  /* Valid. */
  return 0;
 invalid:
  return -1;
}

/** Configuration function for a version 3 service. The given service
 * object must be already allocated and passed through
 * config_generic_service() prior to calling this function.
 *
 * Return 0 on success else a negative value. */
static int
config_service_v3(const hs_opts_t *hs_opts,
                  hs_service_config_t *config)
{
  tor_assert(config);
  tor_assert(hs_opts);

  /* Number of introduction points. */
  if (CHECK_OOB(hs_opts, HiddenServiceNumIntroductionPoints,
                NUM_INTRO_POINTS_DEFAULT,
                HS_CONFIG_V3_MAX_INTRO_POINTS)) {
    goto err;
  }
  config->num_intro_points = hs_opts->HiddenServiceNumIntroductionPoints;

  /* Circuit ID export setting. */
  if (hs_opts->HiddenServiceExportCircuitID) {
    int ok;
    config->circuit_id_protocol =
      helper_parse_circuit_id_protocol("HiddenServcieExportCircuitID",
                                       hs_opts->HiddenServiceExportCircuitID,
                                       &ok);
    if (!ok) {
      goto err;
    }
  }

  /* Is the DoS defense enabled? */
  config->has_dos_defense_enabled =
    hs_opts->HiddenServiceEnableIntroDoSDefense;

  /* Rate for DoS defense */
  if (CHECK_OOB(hs_opts, HiddenServiceEnableIntroDoSRatePerSec,
                 HS_CONFIG_V3_DOS_DEFENSE_RATE_PER_SEC_MIN,
                 HS_CONFIG_V3_DOS_DEFENSE_RATE_PER_SEC_MAX)) {
    goto err;
  }
  config->intro_dos_rate_per_sec =
    hs_opts->HiddenServiceEnableIntroDoSRatePerSec;
  log_info(LD_REND, "Service INTRO2 DoS defenses rate set to: %" PRIu32,
           config->intro_dos_rate_per_sec);

  if (CHECK_OOB(hs_opts, HiddenServiceEnableIntroDoSBurstPerSec,
                HS_CONFIG_V3_DOS_DEFENSE_BURST_PER_SEC_MIN,
                HS_CONFIG_V3_DOS_DEFENSE_BURST_PER_SEC_MAX)) {
    goto err;
  }
  config->intro_dos_burst_per_sec =
    hs_opts->HiddenServiceEnableIntroDoSBurstPerSec;
  log_info(LD_REND, "Service INTRO2 DoS defenses burst set to: %" PRIu32,
           config->intro_dos_burst_per_sec);

  /* Is this an onionbalance instance? */
  if (hs_opts->HiddenServiceOnionBalanceInstance) {
    /* Option is enabled, parse config file. */
    if (! hs_ob_parse_config_file(config)) {
      goto err;
    }
  }

  /* We do not load the key material for the service at this stage. This is
   * done later once tor can confirm that it is in a running state. */

  /* We are about to return a fully configured service so do one last pass of
   * validation at it. */
  if (config_validate_service(config) < 0) {
    goto err;
  }

  return 0;
 err:
  return -1;
}

/** Configure a service using the given options in hs_opts and options. This is
 * called for any service regardless of its version which means that all
 * directives in this function are generic to any service version. This
 * function will also check the validity of the service directory path.
 *
 * The line_ must be pointing to the directive directly after a
 * HiddenServiceDir. That way, when hitting the next HiddenServiceDir line or
 * reaching the end of the list of lines, we know that we have to stop looking
 * for more options.
 *
 * Return 0 on success else -1. */
static int
config_generic_service(const hs_opts_t *hs_opts,
                       const or_options_t *options,
                       hs_service_t *service)
{
  hs_service_config_t *config;

  tor_assert(hs_opts);
  tor_assert(options);
  tor_assert(service);

  /* Makes thing easier. */
  config = &service->config;

  /* Directory where the service's keys are stored. */
  tor_assert(hs_opts->HiddenServiceDir);
  config->directory_path = tor_strdup(hs_opts->HiddenServiceDir);
  log_info(LD_CONFIG, "%s=%s. Configuring...",
           SECTION_HEADER, escaped(config->directory_path));

  /* Protocol version for the service. */
  if (hs_opts->HiddenServiceVersion == -1) {
    /* No value was set; stay with the default. */
  } else if (CHECK_OOB(hs_opts, HiddenServiceVersion,
                       HS_VERSION_MIN, HS_VERSION_MAX)) {
    goto err;
  } else {
    config->hs_version_explicitly_set = 1;
    config->version = hs_opts->HiddenServiceVersion;
  }

  /* Virtual port. */
  for (const config_line_t *portline = hs_opts->HiddenServicePort;
       portline; portline = portline->next) {
    char *err_msg = NULL;
    /* XXX: Can we rename this? */
    rend_service_port_config_t *portcfg =
      rend_service_parse_port_config(portline->value, " ", &err_msg);
    if (!portcfg) {
      if (err_msg) {
        log_warn(LD_CONFIG, "%s", err_msg);
      }
      tor_free(err_msg);
      goto err;
    }
    tor_assert(!err_msg);
    smartlist_add(config->ports, portcfg);
    log_info(LD_CONFIG, "HiddenServicePort=%s for %s",
             portline->value, escaped(config->directory_path));
  }

  /* Do we allow unknown ports? */
  config->allow_unknown_ports = hs_opts->HiddenServiceAllowUnknownPorts;

  /* Directory group readable. */
  config->dir_group_readable = hs_opts->HiddenServiceDirGroupReadable;

  /* Maximum streams per circuit. */
  if (CHECK_OOB(hs_opts, HiddenServiceMaxStreams,
                0, HS_CONFIG_MAX_STREAMS_PER_RDV_CIRCUIT)) {
    goto err;
  }
  config->max_streams_per_rdv_circuit = hs_opts->HiddenServiceMaxStreams;

  /* Maximum amount of streams before we close the circuit. */
  config->max_streams_close_circuit =
    hs_opts->HiddenServiceMaxStreamsCloseCircuit;

  /* Check if we are configured in non anonymous mode meaning every service
   * becomes a single onion service. */
  if (rend_service_non_anonymous_mode_enabled(options)) {
    config->is_single_onion = 1;
  }

  /* Success */
  return 0;
 err:
  return -1;
}

/** Configure a service using the given line and options. This function will
 * call the corresponding configuration function for a specific service
 * version and validate the service against the other ones. On success, add
 * the service to the given list and return 0. On error, nothing is added to
 * the list and a negative value is returned. */
static int
config_service(config_line_t *line, const or_options_t *options,
               smartlist_t *service_list)
{
  int ret;
  hs_service_t *service = NULL;
  hs_opts_t *hs_opts = NULL;
  char *msg = NULL;

  tor_assert(line);
  tor_assert(options);
  tor_assert(service_list);

  /* We have a new hidden service. */
  service = hs_service_new(options);

  /* Try to validate and parse the configuration lines into 'hs_opts' */
  hs_opts = hs_opts_new();
  ret = config_assign(get_hs_opts_mgr(), hs_opts, line, 0, &msg);
  if (ret < 0) {
    log_warn(LD_REND, "Can't parse configuration for onion service: %s", msg);
    goto err;
  }
  tor_assert_nonfatal(msg == NULL);
  validation_status_t vs = config_validate(get_hs_opts_mgr(), NULL,
                                           hs_opts, &msg);
  if (vs < 0) {
    log_warn(LD_REND, "Bad configuration for onion service: %s", msg);
    goto err;
  }
  tor_assert_nonfatal(msg == NULL);

  /* We'll configure that service as a generic one and then pass it to a
   * specific function according to the configured version number. */
  if (config_generic_service(hs_opts, options, service) < 0) {
    goto err;
  }

  tor_assert(service->config.version <= HS_VERSION_MAX);

  /* Check permission on service directory that was just parsed. And this must
   * be done regardless of the service version. Do not ask for the directory
   * to be created, this is done when the keys are loaded because we could be
   * in validation mode right now. */
  if (hs_check_service_private_dir(options->User,
                                   service->config.directory_path,
                                   service->config.dir_group_readable,
                                   0) < 0) {
    goto err;
  }

  /* We'll try to learn the service version here by loading the key(s) if
   * present and we did not set HiddenServiceVersion. Depending on the key
   * format, we can figure out the service version. */
  if (!service->config.hs_version_explicitly_set) {
    service->config.version = config_learn_service_version(service);
  }

  /* We make sure that this set of options for a service are valid that is for
   * instance an option only for v2 is not used for v3. */
  if (config_has_invalid_options(line->next, service)) {
    goto err;
  }

  /* Different functions are in charge of specific options for a version. We
   * start just after the service directory line so once we hit another
   * directory line, the function knows that it has to stop parsing. */
  switch (service->config.version) {
  case HS_VERSION_TWO:
    ret = rend_config_service(hs_opts, options, &service->config);
    break;
  case HS_VERSION_THREE:
    ret = config_service_v3(hs_opts, &service->config);
    break;
  default:
    /* We do validate before if we support the parsed version. */
    tor_assert_nonfatal_unreached();
    goto err;
  }
  if (ret < 0) {
    goto err;
  }

  /* We'll check if this service can be kept depending on the others
   * configured previously. */
  if (service_is_duplicate_in_list(service_list, service)) {
    goto err;
  }

  /* Passes, add it to the given list. */
  smartlist_add(service_list, service);
  hs_opts_free(hs_opts);

  return 0;

 err:
  hs_service_free(service);
  hs_opts_free(hs_opts);
  tor_free(msg);
  return -1;
}

/** From a set of <b>options</b>, setup every hidden service found. Return 0 on
 * success or -1 on failure. If <b>validate_only</b> is set, parse, warn and
 * return as normal, but don't actually change the configured services. */
int
hs_config_service_all(const or_options_t *options, int validate_only)
{
  int ret = -1;
  config_line_t *remaining = NULL;
  smartlist_t *new_service_list = NULL;

  tor_assert(options);

  /* Newly configured service are put in that list which is then used for
   * validation and staging for >= v3. */
  new_service_list = smartlist_new();

  /* We need to start with a HiddenServiceDir line */
  if (options->RendConfigLines &&
      strcasecmp(options->RendConfigLines->key, SECTION_HEADER)) {
    log_warn(LD_CONFIG, "%s with no preceding %s directive",
             options->RendConfigLines->key, SECTION_HEADER);
    goto err;
  }

  remaining = config_lines_dup(options->RendConfigLines);
  while (remaining) {
    config_line_t *section = remaining;
    remaining = config_lines_partition(section, SECTION_HEADER);

    /* Try to configure this service now. On success, it will be added to the
     * list and validated against the service in that same list. */
    int rv = config_service(section, options, new_service_list);
    config_free_lines(section);
    if (rv < 0) {
      goto err;
    }
  }

  /* In non validation mode, we'll stage those services we just successfully
   * configured. Service ownership is transferred from the list to the global
   * state. If any service is invalid, it will be removed from the list and
   * freed. All versions are handled in that function. */
  if (!validate_only) {
    stage_services(new_service_list);
  } else {
    /* We've just validated that we were able to build a clean working list of
     * services. We don't need those objects anymore. */
    SMARTLIST_FOREACH(new_service_list, hs_service_t *, s,
                      hs_service_free(s));
    /* For the v2 subsystem, the configuration function adds the service
     * object to the staging list and it is transferred in the main list
     * through the prunning process. In validation mode, we thus have to purge
     * the staging list so it's not kept in memory as valid service. */
    rend_service_free_staging_list();
  }

  /* Success. Note that the service list has no ownership of its content. */
  ret = 0;
  goto end;

 err:
  SMARTLIST_FOREACH(new_service_list, hs_service_t *, s, hs_service_free(s));

 end:
  smartlist_free(new_service_list);
  /* Tor main should call the free all function on error. */
  return ret;
}

/** From a set of <b>options</b>, setup every client authorization found.
 * Return 0 on success or -1 on failure. If <b>validate_only</b> is set,
 * parse, warn and return as normal, but don't actually change the
 * configured state. */
int
hs_config_client_auth_all(const or_options_t *options, int validate_only)
{
  int ret = -1;

  /* Configure v2 authorization. */
  if (rend_parse_service_authorization(options, validate_only) < 0) {
    goto done;
  }

  /* Configure v3 authorization. */
  if (hs_config_client_authorization(options, validate_only) < 0) {
    goto done;
  }

  /* Success. */
  ret = 0;
 done:
  return ret;
}

/**
 * Free all resources held by the hs_config.c module.
 **/
void
hs_config_free_all(void)
{
  config_mgr_free(hs_opts_mgr);
}
