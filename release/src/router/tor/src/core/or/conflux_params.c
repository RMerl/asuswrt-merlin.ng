/* Copyright (c) 2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file conflux_params.h
 * \brief Header file for conflux_params.c.
 **/

#include "core/or/or.h"

#include "app/config/config.h"

#include "core/or/conflux_params.h"
#include "core/or/congestion_control_common.h"
#include "core/or/circuitlist.h"

#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/relay/routermode.h"

#include "core/or/origin_circuit_st.h"

/**
 * Consensus parameters defaults, minimums and maximums.
 */

/* For "cfx_enabled". */
#define CONFLUX_ENABLED_MIN (0)
#define CONFLUX_ENABLED_MAX (1)
#define CONFLUX_ENABLED_DEFAULT (1)

/* For "cfx_low_exit_threshold". This is a percentage scaled to 10000 so we can
 * support two decimal points. For example, 65.78% would be 6578. */
#define LOW_EXIT_THRESHOLD_MIN (0)
#define LOW_EXIT_THRESHOLD_MAX (10000)
#define LOW_EXIT_THRESHOLD_DEFAULT (6000)

/* For "cfx_max_linked_set". */
#define MAX_LINKED_SET_MIN (0)
#define MAX_LINKED_SET_MAX (UINT8_MAX)
#define MAX_LINKED_SET_DEFAULT (10)

/* For "cfx_max_prebuilt_set". */
#define MAX_PREBUILT_SET_MIN (0)
#define MAX_PREBUILT_SET_MAX (UINT8_MAX)
#define MAX_PREBUILT_SET_DEFAULT (3)

/* For "cfx_max_leg_retry". */
#define MAX_UNLINKED_LEG_RETRY_DEFAULT (3)
#define MAX_UNLINKED_LEG_RETRY_MIN (0)
#define MAX_UNLINKED_LEG_RETRY_MAX (UINT8_MAX)

/* For "cfx_num_legs_set". */
#define NUM_LEGS_SET_MIN (0)
#define NUM_LEGS_SET_MAX (UINT8_MAX)
#define NUM_LEGS_SET_DEFAULT (2)

/* For "cfx_max_legs_set" */
#define MAX_LEGS_SET_MIN (3)
#define MAX_LEGS_SET_MAX (UINT8_MAX)
#define MAX_LEGS_SET_DEFAULT (8)

/* For "cfx_send_pct". */
#define CFX_SEND_PCT_MIN (0)
#define CFX_SEND_PCT_MAX (255)
#define CFX_SEND_PCT_DFLT 100

/* For "cfx_drain_pct". */
#define CFX_DRAIN_PCT_MIN (0)
#define CFX_DRAIN_PCT_MAX (255)
#define CFX_DRAIN_PCT_DFLT 0

/*
 * Cached consensus parameters.
 */

/* Indicate if conflux is enabled or disabled. */
static bool conflux_enabled = CONFLUX_ENABLED_DEFAULT;
/* Maximum number of linked set we are allowed to have (even if in use). */
static uint8_t max_linked_set = MAX_LINKED_SET_DEFAULT;
/* Maximum number of pre built set. */
static uint8_t max_prebuilt_set = MAX_PREBUILT_SET_DEFAULT;
/* Maximum number of unlinked leg retry that is how many times are we allowed
 * to retry a leg until it successfully links. */
STATIC uint32_t max_unlinked_leg_retry = MAX_UNLINKED_LEG_RETRY_DEFAULT;
/* Number of legs per set. */
static uint8_t num_legs_set = NUM_LEGS_SET_DEFAULT;
/* Maximum number of legs per set allowed at exits */
static uint8_t max_legs_set = MAX_LEGS_SET_DEFAULT;
/* The low Exit relay threshold, as a ratio between 0 and 1, used as a limit to
 * decide the amount of pre-built set we build depending on how many Exit relay
 * supports conflux in our current consensus. */
static double low_exit_threshold_ratio =
  LOW_EXIT_THRESHOLD_DEFAULT / (double)LOW_EXIT_THRESHOLD_MAX;

static uint8_t cfx_drain_pct = CFX_DRAIN_PCT_DFLT;
static uint8_t cfx_send_pct = CFX_SEND_PCT_DFLT;

/* Ratio of Exit relays in our consensus supporting conflux. This is computed
 * at every consensus and it is between 0 and 1. */
static double exit_conflux_ratio = 0.0;

/** Sets num_conflux_exit with the latest count of Exits in the given consensus
 * that supports Conflux. */
static void
count_exit_with_conflux_support(const networkstatus_t *ns)
{
  double supported = 0.0;
  int total_exits = 0;

  if (!ns || smartlist_len(ns->routerstatus_list) == 0) {
    return;
  }

  SMARTLIST_FOREACH_BEGIN(ns->routerstatus_list, const routerstatus_t *, rs) {
    if (!rs->is_exit || rs->is_bad_exit) {
      continue;
    }
    if (rs->pv.supports_conflux) {
      supported++;
    }
    total_exits++;
  } SMARTLIST_FOREACH_END(rs);

  if (total_exits > 0) {
    exit_conflux_ratio =
      supported / total_exits;
  } else {
    exit_conflux_ratio = 0.0;
  }

  log_info(LD_GENERAL, "Consensus has %.2f %% Exit relays supporting Conflux",
           exit_conflux_ratio * 100.0);
}

/**
 * Return true iff conflux feature is enabled and usable for a given circuit.
 *
 * Circ may be NULL, in which case we only check the consensus and torrc. */
bool
conflux_is_enabled(const circuit_t *circ)
{
  const or_options_t *opts = get_options();

  /* Conflux CAN NOT operate properly without congestion control and so
   * automatically disabled conflux if we don't have CC enabled. */
  if (!congestion_control_enabled()) {
    return false;
  }

  if (circ) {
    /* If circuit is non-null, we need to check to see if congestion
     * control was successfully negotiated. Conflux depends upon congestion
     * control, and consensus checks are not enough because there can be a
     * race between those checks and the consensus update to enable
     * congestion control. This happens in Shadow, and at relay restart. */
    if (CIRCUIT_IS_ORIGIN(circ)) {
      tor_assert(CONST_TO_ORIGIN_CIRCUIT(circ)->cpath);
      tor_assert(CONST_TO_ORIGIN_CIRCUIT(circ)->cpath->prev);
      if (!CONST_TO_ORIGIN_CIRCUIT(circ)->cpath->prev->ccontrol)
        return false;
    } else {
      if (!circ->ccontrol)
        return false;
    }
  }

  /* For clients, this is mostly for sbws. For relays, this is an emergency
   * emergency override, in case a bug is discovered by a relay operator
   * and we can't set a consensus param fast enough. Basically gives them
   * an option other than downgrading. */
  if (opts->ConfluxEnabled != -1) {
    if (server_mode(opts)) {
      char *msg;
      static ratelim_t rlimit = RATELIM_INIT(60 * 60); /* Hourly */
      if ((msg = rate_limit_log(&rlimit, time(NULL)))) {
        log_warn(LD_GENERAL,
                 "This tor is a relay and ConfluxEnabled is set to 0. "
                 "We would ask you to please write to us on "
                 "tor-relay@lists.torproject.org or file a bug explaining "
                 "why you have disabled this option. Without news from you, "
                 "we might end up marking your relay as a BadExit.");
        tor_free(msg);
      }
    }
    return opts->ConfluxEnabled;
  }

  return conflux_enabled;
}

/** Return the maximum number of linked set we are allowed to have. */
uint8_t
conflux_params_get_max_linked_set(void)
{
  return max_linked_set;
}

/** Return the number of maximum pre built sets that is allowed to have. */
uint8_t
conflux_params_get_max_prebuilt(void)
{
  /* Without any Exit supporting conflux, we won't be able to build a set. The
   * float problem here is minimal because exit_conflux_ratio is either a flat
   * 0 or else it means we do have at least an exit. */
  if (exit_conflux_ratio <= 0.0) {
    return 0;
  }

  /* Allow only 1 pre built set if we are lower than the low exit threshold
   * parameter from the consensus. */
  if (exit_conflux_ratio < low_exit_threshold_ratio) {
    return 1;
  }
  return max_prebuilt_set;
}

/** Return the maximum number of retry we can do until a leg links. */
uint8_t
conflux_params_get_max_unlinked_leg_retry(void)
{
  return max_unlinked_leg_retry;
}

/** Return the number of legs per set. */
uint8_t
conflux_params_get_num_legs_set(void)
{
  return num_legs_set;
}

/** Return the maximum number of legs per set. */
uint8_t
conflux_params_get_max_legs_set(void)
{
  return max_legs_set;
}

/** Return the drain percent we must hit before switching */
uint8_t
conflux_params_get_drain_pct(void)
{
  return cfx_drain_pct;
}

/** Return the percent of the congestion window to send before switching. */
uint8_t
conflux_params_get_send_pct(void)
{
  return cfx_send_pct;
}

/** Update global conflux related consensus parameter values, every consensus
 * update. */
void
conflux_params_new_consensus(const networkstatus_t *ns)
{
  /* Params used by conflux_pool.c */
  conflux_enabled =
    networkstatus_get_param(ns, "cfx_enabled",
                            CONFLUX_ENABLED_DEFAULT,
                            CONFLUX_ENABLED_MIN, CONFLUX_ENABLED_MAX);

  low_exit_threshold_ratio =
    networkstatus_get_param(ns, "cfx_low_exit_threshold",
                            LOW_EXIT_THRESHOLD_DEFAULT,
                            LOW_EXIT_THRESHOLD_MIN, LOW_EXIT_THRESHOLD_MAX) /
    (double)LOW_EXIT_THRESHOLD_MAX;

  max_linked_set =
    networkstatus_get_param(ns, "cfx_max_linked_set",
                            MAX_LINKED_SET_DEFAULT,
                            MAX_LINKED_SET_MIN, MAX_LINKED_SET_MAX);

  max_prebuilt_set =
    networkstatus_get_param(ns, "cfx_max_prebuilt_set",
                            MAX_PREBUILT_SET_DEFAULT,
                            MAX_PREBUILT_SET_MIN, MAX_PREBUILT_SET_MAX);

  max_unlinked_leg_retry =
    networkstatus_get_param(ns, "cfx_max_unlinked_leg_retry",
                            MAX_UNLINKED_LEG_RETRY_DEFAULT,
                            MAX_UNLINKED_LEG_RETRY_MIN,
                            MAX_UNLINKED_LEG_RETRY_MAX);

  num_legs_set =
    networkstatus_get_param(ns, "cfx_num_legs_set",
                            NUM_LEGS_SET_DEFAULT,
                            NUM_LEGS_SET_MIN, NUM_LEGS_SET_MAX);

  max_legs_set =
    networkstatus_get_param(ns, "cfx_max_legs_set",
                            MAX_LEGS_SET_DEFAULT,
                            MAX_LEGS_SET_MIN, MAX_LEGS_SET_MAX);

  /* Params used by conflux.c */
  cfx_send_pct = networkstatus_get_param(ns, "cfx_send_pct",
      CFX_SEND_PCT_DFLT,
      CFX_SEND_PCT_MIN,
      CFX_SEND_PCT_MAX);

  cfx_drain_pct = networkstatus_get_param(ns, "cfx_drain_pct",
      CFX_DRAIN_PCT_DFLT,
      CFX_DRAIN_PCT_MIN,
      CFX_DRAIN_PCT_MAX);

  count_exit_with_conflux_support(ns);
}
