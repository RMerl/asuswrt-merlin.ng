/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file voteflags.c
 * \brief Authority code for deciding the performance thresholds for flags,
 *   and assigning flags to routers.
 **/

#define VOTEFLAGS_PRIVATE
#include "core/or/or.h"
#include "feature/dirauth/voteflags.h"

#include "app/config/config.h"
#include "core/mainloop/mainloop.h"
#include "core/or/policies.h"
#include "feature/dirauth/bwauth.h"
#include "feature/dirauth/reachability.h"
#include "feature/dirauth/dirauth_sys.h"
#include "feature/hibernate/hibernate.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/relay/router.h"
#include "feature/stats/rephist.h"

#include "feature/dirauth/dirauth_options_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist_st.h"
#include "feature/nodelist/vote_routerstatus_st.h"

#include "lib/container/order.h"

/* Thresholds for server performance: set by
 * dirserv_compute_performance_thresholds, and used by
 * generate_v2_networkstatus */

/** Any router with an uptime of at least this value is stable. */
static uint32_t stable_uptime = 0; /* start at a safe value */
/** Any router with an mtbf of at least this value is stable. */
static double stable_mtbf = 0.0;
/** If true, we have measured enough mtbf info to look at stable_mtbf rather
 * than stable_uptime. */
static int enough_mtbf_info = 0;
/** Any router with a weighted fractional uptime of at least this much might
 * be good as a guard. */
static double guard_wfu = 0.0;
/** Don't call a router a guard unless we've known about it for at least this
 * many seconds. */
static long guard_tk = 0;
/** Any router with a bandwidth at least this high is "Fast" */
static uint32_t fast_bandwidth_kb = 0;
/** If exits can be guards, then all guards must have a bandwidth this
 * high. */
static uint32_t guard_bandwidth_including_exits_kb = 0;
/** If exits can't be guards, then all guards must have a bandwidth this
 * high. */
static uint32_t guard_bandwidth_excluding_exits_kb = 0;

/** Helper: estimate the uptime of a router given its stated uptime and the
 * amount of time since it last stated its stated uptime. */
static inline long
real_uptime(const routerinfo_t *router, time_t now)
{
  if (now < router->cache_info.published_on)
    return router->uptime;
  else
    return router->uptime + (now - router->cache_info.published_on);
}

/** Return 1 if <b>router</b> is not suitable for these parameters, else 0.
 * If <b>need_uptime</b> is non-zero, we require a minimum uptime.
 * If <b>need_capacity</b> is non-zero, we require a minimum advertised
 * bandwidth.
 */
static int
dirserv_thinks_router_is_unreliable(time_t now,
                                    const routerinfo_t *router,
                                    int need_uptime, int need_capacity)
{
  if (need_uptime) {
    if (!enough_mtbf_info) {
      /* XXXX We should change the rule from
       * "use uptime if we don't have mtbf data" to "don't advertise Stable on
       * v3 if we don't have enough mtbf data."  Or maybe not, since if we ever
       * hit a point where we need to reset a lot of authorities at once,
       * none of them would be in a position to declare Stable.
       */
      long uptime = real_uptime(router, now);
      if ((unsigned)uptime < stable_uptime &&
          uptime < dirauth_get_options()->AuthDirVoteStableGuaranteeMinUptime)
        return 1;
    } else {
      double mtbf =
        rep_hist_get_stability(router->cache_info.identity_digest, now);
      if (mtbf < stable_mtbf &&
          mtbf < dirauth_get_options()->AuthDirVoteStableGuaranteeMTBF)
        return 1;
    }
  }
  if (need_capacity) {
    uint32_t bw_kb = dirserv_get_credible_bandwidth_kb(router);
    if (bw_kb < fast_bandwidth_kb)
      return 1;
  }
  return 0;
}

/** Return 1 if <b>ri</b>'s descriptor is "active" -- running, valid,
 * not hibernating, having observed bw greater 0, and not too old. Else
 * return 0.
 */
static int
router_is_active(const routerinfo_t *ri, const node_t *node, time_t now)
{
  time_t cutoff = now - ROUTER_MAX_AGE_TO_PUBLISH;
  if (ri->cache_info.published_on < cutoff) {
    return 0;
  }
  if (!node->is_running || !node->is_valid || ri->is_hibernating) {
    return 0;
  }
  /* Only require bandwidth capacity in non-test networks, or
   * if TestingTorNetwork, and TestingMinExitFlagThreshold is non-zero */
  if (!ri->bandwidthcapacity) {
    if (get_options()->TestingTorNetwork) {
      if (dirauth_get_options()->TestingMinExitFlagThreshold > 0) {
        /* If we're in a TestingTorNetwork, and TestingMinExitFlagThreshold is,
         * then require bandwidthcapacity */
        return 0;
      }
    } else {
      /* If we're not in a TestingTorNetwork, then require bandwidthcapacity */
      return 0;
    }
  }
  return 1;
}

/** Return true iff <b>router</b> should be assigned the "HSDir" flag.
 *
 * Right now this means it advertises support for it, it has a high uptime,
 * it's a directory cache, it has the Stable and Fast flags, and it's currently
 * considered Running.
 *
 * This function needs to be called after router-\>is_running has
 * been set.
 */
static int
dirserv_thinks_router_is_hs_dir(const routerinfo_t *router,
                                const node_t *node, time_t now)
{

  long uptime;

  /* If we haven't been running for at least
   * MinUptimeHidServDirectoryV2 seconds, we can't
   * have accurate data telling us a relay has been up for at least
   * that long. We also want to allow a bit of slack: Reachability
   * tests aren't instant. If we haven't been running long enough,
   * trust the relay. */

  if (get_uptime() >
      dirauth_get_options()->MinUptimeHidServDirectoryV2 * 1.1)
    uptime = MIN(rep_hist_get_uptime(router->cache_info.identity_digest, now),
                 real_uptime(router, now));
  else
    uptime = real_uptime(router, now);

  return (router->wants_to_be_hs_dir &&
          router->supports_tunnelled_dir_requests &&
          node->is_stable && node->is_fast &&
          uptime >= dirauth_get_options()->MinUptimeHidServDirectoryV2 &&
          router_is_active(router, node, now));
}

/** Don't consider routers with less bandwidth than this when computing
 * thresholds. */
#define ABSOLUTE_MIN_BW_VALUE_TO_CONSIDER_KB 4

/** Helper for dirserv_compute_performance_thresholds(): Decide whether to
 * include a router in our calculations, and return true iff we should; the
 * require_mbw parameter is passed in by
 * dirserv_compute_performance_thresholds() and controls whether we ever
 * count routers with only advertised bandwidths */
static int
router_counts_toward_thresholds(const node_t *node, time_t now,
                                const digestmap_t *omit_as_sybil,
                                int require_mbw)
{
  /* Have measured bw? */
  int have_mbw =
    dirserv_has_measured_bw(node->identity);
  uint64_t min_bw_kb = ABSOLUTE_MIN_BW_VALUE_TO_CONSIDER_KB;
  const or_options_t *options = get_options();
  const dirauth_options_t *dirauth_options = dirauth_get_options();

  if (options->TestingTorNetwork) {
    min_bw_kb = (int64_t)dirauth_options->TestingMinExitFlagThreshold / 1000;
  }

  return node->ri && router_is_active(node->ri, node, now) &&
    !digestmap_get(omit_as_sybil, node->identity) &&
    (dirserv_get_credible_bandwidth_kb(node->ri) >= min_bw_kb) &&
    (have_mbw || !require_mbw);
}

/** Look through the routerlist, the Mean Time Between Failure history, and
 * the Weighted Fractional Uptime history, and use them to set thresholds for
 * the Stable, Fast, and Guard flags.  Update the fields stable_uptime,
 * stable_mtbf, enough_mtbf_info, guard_wfu, guard_tk, fast_bandwidth,
 * guard_bandwidth_including_exits, and guard_bandwidth_excluding_exits.
 *
 * Also, set the is_exit flag of each router appropriately. */
void
dirserv_compute_performance_thresholds(digestmap_t *omit_as_sybil)
{
  int n_active, n_active_nonexit, n_familiar;
  uint32_t *uptimes, *bandwidths_kb, *bandwidths_excluding_exits_kb;
  long *tks;
  double *mtbfs, *wfus;
  const smartlist_t *nodelist;
  time_t now = time(NULL);
  const or_options_t *options = get_options();
  const dirauth_options_t *dirauth_options = dirauth_get_options();

  /* Require mbw? */
  int require_mbw =
    (dirserv_get_last_n_measured_bws() >
     dirauth_options->MinMeasuredBWsForAuthToIgnoreAdvertised) ? 1 : 0;

  /* initialize these all here, in case there are no routers */
  stable_uptime = 0;
  stable_mtbf = 0;
  fast_bandwidth_kb = 0;
  guard_bandwidth_including_exits_kb = 0;
  guard_bandwidth_excluding_exits_kb = 0;
  guard_tk = 0;
  guard_wfu = 0;

  nodelist_assert_ok();
  nodelist = nodelist_get_list();

  /* Initialize arrays that will hold values for each router.  We'll
   * sort them and use that to compute thresholds. */
  n_active = n_active_nonexit = 0;
  /* Uptime for every active router. */
  uptimes = tor_calloc(smartlist_len(nodelist), sizeof(uint32_t));
  /* Bandwidth for every active router. */
  bandwidths_kb = tor_calloc(smartlist_len(nodelist), sizeof(uint32_t));
  /* Bandwidth for every active non-exit router. */
  bandwidths_excluding_exits_kb =
    tor_calloc(smartlist_len(nodelist), sizeof(uint32_t));
  /* Weighted mean time between failure for each active router. */
  mtbfs = tor_calloc(smartlist_len(nodelist), sizeof(double));
  /* Time-known for each active router. */
  tks = tor_calloc(smartlist_len(nodelist), sizeof(long));
  /* Weighted fractional uptime for each active router. */
  wfus = tor_calloc(smartlist_len(nodelist), sizeof(double));

  /* Now, fill in the arrays. */
  SMARTLIST_FOREACH_BEGIN(nodelist, node_t *, node) {
    if (options->BridgeAuthoritativeDir &&
        node->ri &&
        node->ri->purpose != ROUTER_PURPOSE_BRIDGE)
      continue;

    routerinfo_t *ri = node->ri;
    if (ri) {
      node->is_exit = (!router_exit_policy_rejects_all(ri) &&
                       exit_policy_is_general_exit(ri->exit_policy));
    }

    if (router_counts_toward_thresholds(node, now, omit_as_sybil,
                                        require_mbw)) {
      const char *id = node->identity;
      uint32_t bw_kb;

      /* resolve spurious clang shallow analysis null pointer errors */
      tor_assert(ri);

      uptimes[n_active] = (uint32_t)real_uptime(ri, now);
      mtbfs[n_active] = rep_hist_get_stability(id, now);
      tks  [n_active] = rep_hist_get_weighted_time_known(id, now);
      bandwidths_kb[n_active] = bw_kb = dirserv_get_credible_bandwidth_kb(ri);
      if (!node->is_exit || node->is_bad_exit) {
        bandwidths_excluding_exits_kb[n_active_nonexit] = bw_kb;
        ++n_active_nonexit;
      }
      ++n_active;
    }
  } SMARTLIST_FOREACH_END(node);

  /* Now, compute thresholds. */
  if (n_active) {
    /* The median uptime is stable. */
    stable_uptime = median_uint32(uptimes, n_active);
    /* The median mtbf is stable, if we have enough mtbf info */
    stable_mtbf = median_double(mtbfs, n_active);
    /* The 12.5th percentile bandwidth is fast. */
    fast_bandwidth_kb = find_nth_uint32(bandwidths_kb, n_active, n_active/8);
    /* (Now bandwidths is sorted.) */
    if (fast_bandwidth_kb < RELAY_REQUIRED_MIN_BANDWIDTH/(2 * 1000))
      fast_bandwidth_kb = bandwidths_kb[n_active/4];
    int nth = (int)(n_active *
                    dirauth_options->AuthDirVoteGuardBwThresholdFraction);
    guard_bandwidth_including_exits_kb =
      find_nth_uint32(bandwidths_kb, n_active, nth);
    guard_tk = find_nth_long(tks, n_active, n_active/8);
  }

  if (guard_tk > dirauth_options->AuthDirVoteGuardGuaranteeTimeKnown)
    guard_tk = dirauth_options->AuthDirVoteGuardGuaranteeTimeKnown;

  {
    /* We can vote on a parameter for the minimum and maximum. */
#define ABSOLUTE_MIN_VALUE_FOR_FAST_FLAG 4
    int32_t min_fast_kb, max_fast_kb, min_fast, max_fast;
    min_fast = networkstatus_get_param(NULL, "FastFlagMinThreshold",
      ABSOLUTE_MIN_VALUE_FOR_FAST_FLAG,
      ABSOLUTE_MIN_VALUE_FOR_FAST_FLAG,
      INT32_MAX);
    if (options->TestingTorNetwork) {
      min_fast = (int32_t)dirauth_options->TestingMinFastFlagThreshold;
    }
    max_fast = networkstatus_get_param(NULL, "FastFlagMaxThreshold",
                                       INT32_MAX, min_fast, INT32_MAX);
    min_fast_kb = min_fast / 1000;
    max_fast_kb = max_fast / 1000;

    if (fast_bandwidth_kb < (uint32_t)min_fast_kb)
      fast_bandwidth_kb = min_fast_kb;
    if (fast_bandwidth_kb > (uint32_t)max_fast_kb)
      fast_bandwidth_kb = max_fast_kb;
  }
  /* Protect sufficiently fast nodes from being pushed out of the set
   * of Fast nodes. */
  {
    const uint64_t fast_opt = dirauth_get_options()->AuthDirFastGuarantee;
    if (fast_opt && fast_bandwidth_kb > fast_opt / 1000)
      fast_bandwidth_kb = (uint32_t)(fast_opt / 1000);
  }

  /* Now that we have a time-known that 7/8 routers are known longer than,
   * fill wfus with the wfu of every such "familiar" router. */
  n_familiar = 0;

  SMARTLIST_FOREACH_BEGIN(nodelist, node_t *, node) {
      if (router_counts_toward_thresholds(node, now,
                                          omit_as_sybil, require_mbw)) {
        routerinfo_t *ri = node->ri;
        const char *id = ri->cache_info.identity_digest;
        long tk = rep_hist_get_weighted_time_known(id, now);
        if (tk < guard_tk)
          continue;
        wfus[n_familiar++] = rep_hist_get_weighted_fractional_uptime(id, now);
      }
  } SMARTLIST_FOREACH_END(node);
  if (n_familiar)
    guard_wfu = median_double(wfus, n_familiar);
  if (guard_wfu > dirauth_options->AuthDirVoteGuardGuaranteeWFU)
    guard_wfu = dirauth_options->AuthDirVoteGuardGuaranteeWFU;

  enough_mtbf_info = rep_hist_have_measured_enough_stability();

  if (n_active_nonexit) {
    int nth = (int)(n_active_nonexit *
                    dirauth_options->AuthDirVoteGuardBwThresholdFraction);
    guard_bandwidth_excluding_exits_kb =
      find_nth_uint32(bandwidths_excluding_exits_kb, n_active_nonexit, nth);
  }

  log_info(LD_DIRSERV,
      "Cutoffs: For Stable, %lu sec uptime, %lu sec MTBF. "
      "For Fast: %lu kilobytes/sec. "
      "For Guard: WFU %.03f%%, time-known %lu sec, "
      "and bandwidth %lu or %lu kilobytes/sec. "
      "We%s have enough stability data.",
      (unsigned long)stable_uptime,
      (unsigned long)stable_mtbf,
      (unsigned long)fast_bandwidth_kb,
      guard_wfu*100,
      (unsigned long)guard_tk,
      (unsigned long)guard_bandwidth_including_exits_kb,
      (unsigned long)guard_bandwidth_excluding_exits_kb,
      enough_mtbf_info ? "" : " don't");

  tor_free(uptimes);
  tor_free(mtbfs);
  tor_free(bandwidths_kb);
  tor_free(bandwidths_excluding_exits_kb);
  tor_free(tks);
  tor_free(wfus);
}

/* Use dirserv_compute_performance_thresholds() to compute the thresholds
 * for the status flags, specifically for bridges.
 *
 * This is only called by a Bridge Authority from
 * networkstatus_getinfo_by_purpose().
 */
void
dirserv_compute_bridge_flag_thresholds(void)
{
  digestmap_t *omit_as_sybil = digestmap_new();
  dirserv_compute_performance_thresholds(omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);
}

/** Give a statement of our current performance thresholds for inclusion
 * in a vote document. */
char *
dirserv_get_flag_thresholds_line(void)
{
  char *result=NULL;
  const int measured_threshold =
    dirauth_get_options()->MinMeasuredBWsForAuthToIgnoreAdvertised;
  const int enough_measured_bw =
    dirserv_get_last_n_measured_bws() > measured_threshold;

  tor_asprintf(&result,
      "stable-uptime=%lu stable-mtbf=%lu "
      "fast-speed=%lu "
      "guard-wfu=%.03f%% guard-tk=%lu "
      "guard-bw-inc-exits=%lu guard-bw-exc-exits=%lu "
      "enough-mtbf=%d ignoring-advertised-bws=%d",
      (unsigned long)stable_uptime,
      (unsigned long)stable_mtbf,
      (unsigned long)fast_bandwidth_kb*1000,
      guard_wfu*100,
      (unsigned long)guard_tk,
      (unsigned long)guard_bandwidth_including_exits_kb*1000,
      (unsigned long)guard_bandwidth_excluding_exits_kb*1000,
      enough_mtbf_info ? 1 : 0,
      enough_measured_bw ? 1 : 0);

  return result;
}

/** Directory authorities should avoid expressing an opinion on the
 * Running flag if their own uptime is too low for the opinion to be
 * accurate. They implement this step by not listing Running on the
 * "known-flags" line in their vote.
 *
 * The default threshold is 30 minutes, because authorities do a full
 * reachability sweep of the ID space every 10*128=1280 seconds
 * (see REACHABILITY_TEST_CYCLE_PERIOD).
 *
 * For v3 dir auths, as long as some authorities express an opinion about
 * Running, it's fine if a few authorities don't. There's an explicit
 * check, when making the consensus, to abort if *no* authorities list
 * Running as a known-flag.
 *
 * For the bridge authority, if it doesn't vote about Running, the
 * resulting networkstatus file simply won't list any bridges as Running.
 * That means the supporting tools, like bridgedb/rdsys and onionoo, need
 * to be able to handle getting a bridge networkstatus document with no
 * Running flags. For more details, see
 * https://bugs.torproject.org/tpo/anti-censorship/rdsys/102 */
int
running_long_enough_to_decide_unreachable(void)
{
  const dirauth_options_t *opts = dirauth_get_options();
  return time_of_process_start +
    opts->TestingAuthDirTimeToLearnReachability < approx_time();
}

/** Each server needs to have passed a reachability test no more
 * than this number of seconds ago, or it is listed as down in
 * the directory. */
#define REACHABLE_TIMEOUT (45*60)

/** If we tested a router and found it reachable _at least this long_ after it
 * declared itself hibernating, it is probably done hibernating and we just
 * missed a descriptor from it. */
#define HIBERNATION_PUBLICATION_SKEW (60*60)

/** Treat a router as alive if
 *    - It's me, and I'm not hibernating.
 * or - We've found it reachable recently. */
void
dirserv_set_router_is_running(routerinfo_t *router, time_t now)
{
  /*XXXX This function is a mess.  Separate out the part that calculates
    whether it's reachable and the part that tells rephist that the router was
    unreachable.
   */
  int answer;
  const dirauth_options_t *dirauth_options = dirauth_get_options();
  node_t *node = node_get_mutable_by_id(router->cache_info.identity_digest);
  tor_assert(node);

  if (router_is_me(router)) {
    /* We always know if we are shutting down or hibernating ourselves. */
    answer = ! we_are_hibernating();
  } else if (router->is_hibernating &&
             (router->cache_info.published_on +
              HIBERNATION_PUBLICATION_SKEW) > node->last_reachable) {
    /* A hibernating router is down unless we (somehow) had contact with it
     * since it declared itself to be hibernating. */
    answer = 0;
  } else if (! dirauth_options->AuthDirTestReachability) {
    /* If we aren't testing reachability, then everybody is up unless they say
     * they are down. */
    answer = 1;
  } else {
    /* Otherwise, a router counts as up if we found all announced OR
       ports reachable in the last REACHABLE_TIMEOUT seconds.

       XXX prop186 For now there's always one IPv4 and at most one
       IPv6 OR port.

       If we're not on IPv6, don't consider reachability of potential
       IPv6 OR port since that'd kill all dual stack relays until a
       majority of the dir auths have IPv6 connectivity. */
    answer = (now < node->last_reachable + REACHABLE_TIMEOUT &&
              (dirauth_options->AuthDirHasIPv6Connectivity != 1 ||
               tor_addr_is_null(&router->ipv6_addr) ||
               now < node->last_reachable6 + REACHABLE_TIMEOUT));
  }

  if (!answer && running_long_enough_to_decide_unreachable()) {
    /* Not considered reachable. tell rephist about that.

       Because we launch a reachability test for each router every
       REACHABILITY_TEST_CYCLE_PERIOD seconds, then the router has probably
       been down since at least that time after we last successfully reached
       it.

       XXX ipv6
     */
    time_t when = now;
    if (node->last_reachable &&
        node->last_reachable + REACHABILITY_TEST_CYCLE_PERIOD < now)
      when = node->last_reachable + REACHABILITY_TEST_CYCLE_PERIOD;
    rep_hist_note_router_unreachable(router->cache_info.identity_digest, when);
  }

  node->is_running = answer;
}

/* Check <b>node</b> and <b>ri</b> on whether or not we should publish a
 * relay's IPv6 addresses. */
static int
should_publish_node_ipv6(const node_t *node, const routerinfo_t *ri,
                         time_t now)
{
  const dirauth_options_t *options = dirauth_get_options();

  return options->AuthDirHasIPv6Connectivity == 1 &&
    !tor_addr_is_null(&ri->ipv6_addr) &&
    ((node->last_reachable6 >= now - REACHABLE_TIMEOUT) ||
     router_is_me(ri));
}

/** Set routerstatus flags based on the authority options. Same as the testing
 * function but for the main network. */
static void
dirserv_set_routerstatus_flags(routerstatus_t *rs)
{
  const dirauth_options_t *options = dirauth_get_options();

  tor_assert(rs);

  /* Assign Guard flag to relays that can get it unconditionnaly. */
  if (routerset_contains_routerstatus(options->AuthDirVoteGuard, rs, 0)) {
    rs->is_possible_guard = 1;
  }
}

/**
 * Extract status information from <b>ri</b> and from other authority
 * functions and store it in <b>rs</b>, as per
 * <b>set_routerstatus_from_routerinfo</b>.  Additionally, sets information
 * in from the authority subsystem.
 */
void
dirauth_set_routerstatus_from_routerinfo(routerstatus_t *rs,
                                         node_t *node,
                                         const routerinfo_t *ri,
                                         time_t now,
                                         int listbadexits,
                                         int listmiddleonly)
{
  const or_options_t *options = get_options();
  uint32_t routerbw_kb = dirserv_get_credible_bandwidth_kb(ri);

  /* Set these flags so that set_routerstatus_from_routerinfo can copy them.
   */
  node->is_stable = !dirserv_thinks_router_is_unreliable(now, ri, 1, 0);
  node->is_fast = !dirserv_thinks_router_is_unreliable(now, ri, 0, 1);
  node->is_hs_dir = dirserv_thinks_router_is_hs_dir(ri, node, now);

  set_routerstatus_from_routerinfo(rs, node, ri);

  /* Override rs->is_possible_guard. */
  const uint64_t bw_opt = dirauth_get_options()->AuthDirGuardBWGuarantee;
  if (node->is_fast && node->is_stable &&
      ri->supports_tunnelled_dir_requests &&
      ((bw_opt && routerbw_kb >= bw_opt / 1000) ||
       routerbw_kb >= MIN(guard_bandwidth_including_exits_kb,
                          guard_bandwidth_excluding_exits_kb))) {
    long tk = rep_hist_get_weighted_time_known(
                                      node->identity, now);
    double wfu = rep_hist_get_weighted_fractional_uptime(
                                      node->identity, now);
    rs->is_possible_guard = (wfu >= guard_wfu && tk >= guard_tk) ? 1 : 0;
  } else {
    rs->is_possible_guard = 0;
  }

  /* Override rs->is_bad_exit */
  rs->is_bad_exit = listbadexits && node->is_bad_exit;

  /* Override rs->is_middle_only and related flags. */
  rs->is_middle_only = listmiddleonly && node->is_middle_only;
  if (rs->is_middle_only) {
    if (listbadexits)
      rs->is_bad_exit = 1;
    rs->is_exit = rs->is_possible_guard = rs->is_hs_dir = rs->is_v2_dir = 0;
  }

  /* Set rs->is_staledesc. */
  rs->is_staledesc =
    (ri->cache_info.published_on + DESC_IS_STALE_INTERVAL) < now;

  if (! should_publish_node_ipv6(node, ri, now)) {
    /* We're not configured as having IPv6 connectivity or the node isn't:
     * zero its IPv6 information. */
    tor_addr_make_null(&rs->ipv6_addr, AF_INET6);
    rs->ipv6_orport = 0;
  }

  if (options->TestingTorNetwork) {
    dirserv_set_routerstatus_testing(rs);
  } else {
    dirserv_set_routerstatus_flags(rs);
  }
}

/** Use TestingDirAuthVoteExit, TestingDirAuthVoteGuard, and
 * TestingDirAuthVoteHSDir to give out the Exit, Guard, and HSDir flags,
 * respectively. But don't set the corresponding node flags.
 * Should only be called if TestingTorNetwork is set. */
STATIC void
dirserv_set_routerstatus_testing(routerstatus_t *rs)
{
  const dirauth_options_t *options = dirauth_get_options();

  tor_assert(get_options()->TestingTorNetwork);

  if (routerset_contains_routerstatus(options->TestingDirAuthVoteExit,
                                      rs, 0)) {
    rs->is_exit = 1;
  } else if (options->TestingDirAuthVoteExitIsStrict) {
    rs->is_exit = 0;
  }

  if (routerset_contains_routerstatus(options->TestingDirAuthVoteGuard,
                                      rs, 0)) {
    rs->is_possible_guard = 1;
  } else if (options->TestingDirAuthVoteGuardIsStrict) {
    rs->is_possible_guard = 0;
  }

  if (routerset_contains_routerstatus(options->TestingDirAuthVoteHSDir,
                                      rs, 0)) {
    rs->is_hs_dir = 1;
  } else if (options->TestingDirAuthVoteHSDirIsStrict) {
    rs->is_hs_dir = 0;
  }
}

/** Use dirserv_set_router_is_running() to set bridges as running if they're
 * reachable.
 *
 * This function is called from set_bridge_running_callback() when running as
 * a bridge authority.
 */
void
dirserv_set_bridges_running(time_t now)
{
  routerlist_t *rl = router_get_routerlist();

  SMARTLIST_FOREACH_BEGIN(rl->routers, routerinfo_t *, ri) {
    if (ri->purpose == ROUTER_PURPOSE_BRIDGE)
      dirserv_set_router_is_running(ri, now);
  } SMARTLIST_FOREACH_END(ri);
}
