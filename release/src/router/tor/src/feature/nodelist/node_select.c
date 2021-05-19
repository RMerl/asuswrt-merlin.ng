/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file node_select.c
 * \brief Code to choose nodes randomly based on restrictions and
 *   weighted probabilities.
 **/

#define NODE_SELECT_PRIVATE
#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/policies.h"
#include "core/or/reasons.h"
#include "feature/client/entrynodes.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirclient/dirclient_modes.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/node_select.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "lib/container/bitarray.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/math/fp.h"

#include "feature/dirclient/dir_server_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

static int compute_weighted_bandwidths(const smartlist_t *sl,
                                       bandwidth_weight_rule_t rule,
                                       double **bandwidths_out,
                                       double *total_bandwidth_out);
static const routerstatus_t *router_pick_trusteddirserver_impl(
                const smartlist_t *sourcelist, dirinfo_type_t auth,
                int flags, int *n_busy_out);
static const routerstatus_t *router_pick_dirserver_generic(
                              smartlist_t *sourcelist,
                              dirinfo_type_t type, int flags);

/** Try to find a running dirserver that supports operations of <b>type</b>.
 *
 * If there are no running dirservers in our routerlist and the
 * <b>PDS_RETRY_IF_NO_SERVERS</b> flag is set, set all the fallback ones
 * (including authorities) as running again, and pick one.
 *
 * If the <b>PDS_IGNORE_FASCISTFIREWALL</b> flag is set, then include
 * dirservers that we can't reach.
 *
 * If the <b>PDS_ALLOW_SELF</b> flag is not set, then don't include ourself
 * (if we're a dirserver).
 *
 * Don't pick a fallback directory mirror if any non-fallback is viable;
 * (the fallback directory mirrors include the authorities)
 * try to avoid using servers that have returned 503 recently.
 */
const routerstatus_t *
router_pick_directory_server(dirinfo_type_t type, int flags)
{
  int busy = 0;
  const routerstatus_t *choice;

  choice = router_pick_directory_server_impl(type, flags, &busy);
  if (choice || !(flags & PDS_RETRY_IF_NO_SERVERS))
    return choice;

  if (busy) {
    /* If the reason that we got no server is that servers are "busy",
     * we must be excluding good servers because we already have serverdesc
     * fetches with them.  Do not mark down servers up because of this. */
    tor_assert((flags & (PDS_NO_EXISTING_SERVERDESC_FETCH|
                         PDS_NO_EXISTING_MICRODESC_FETCH)));
    return NULL;
  }

  log_info(LD_DIR,
           "No reachable router entries for dirservers. "
           "Trying them all again.");
  /* mark all fallback directory mirrors as up again */
  router_reset_status_download_failures();
  /* try again */
  choice = router_pick_directory_server_impl(type, flags, NULL);
  return choice;
}

/** Try to find a running fallback directory. Flags are as for
 * router_pick_directory_server.
 */
const routerstatus_t *
router_pick_dirserver_generic(smartlist_t *sourcelist,
                              dirinfo_type_t type, int flags)
{
  const routerstatus_t *choice;
  int busy = 0;

  if (smartlist_len(sourcelist) == 1) {
    /* If there's only one choice, then we should disable the logic that
     * would otherwise prevent us from choosing ourself. */
    flags |= PDS_ALLOW_SELF;
  }

  choice = router_pick_trusteddirserver_impl(sourcelist, type, flags, &busy);
  if (choice || !(flags & PDS_RETRY_IF_NO_SERVERS))
    return choice;
  if (busy) {
    /* If the reason that we got no server is that servers are "busy",
     * we must be excluding good servers because we already have serverdesc
     * fetches with them.  Do not mark down servers up because of this. */
    tor_assert((flags & (PDS_NO_EXISTING_SERVERDESC_FETCH|
                         PDS_NO_EXISTING_MICRODESC_FETCH)));
    return NULL;
  }

  log_info(LD_DIR,
           "No dirservers are reachable. Trying them all again.");
  mark_all_dirservers_up(sourcelist);
  return router_pick_trusteddirserver_impl(sourcelist, type, flags, NULL);
}

/* Common retry code for router_pick_directory_server_impl and
 * router_pick_trusteddirserver_impl. Retry with the non-preferred IP version.
 * Must be called before RETRY_WITHOUT_EXCLUDE().
 *
 * If we got no result, and we are applying IP preferences, and we are a
 * client that could use an alternate IP version, try again with the
 * opposite preferences. */
#define RETRY_ALTERNATE_IP_VERSION(retry_label)                               \
  STMT_BEGIN                                                                  \
    if (result == NULL && try_ip_pref && options->ClientUseIPv4               \
        && reachable_addr_use_ipv6(options) && !server_mode(options)        \
        && !n_busy) {                                                         \
      n_excluded = 0;                                                         \
      n_busy = 0;                                                             \
      try_ip_pref = 0;                                                        \
      goto retry_label;                                                       \
    }                                                                         \
  STMT_END

/* Common retry code for router_pick_directory_server_impl and
 * router_pick_trusteddirserver_impl. Retry without excluding nodes, but with
 * the preferred IP version. Must be called after RETRY_ALTERNATE_IP_VERSION().
 *
 * If we got no result, and we are excluding nodes, and StrictNodes is
 * not set, try again without excluding nodes. */
#define RETRY_WITHOUT_EXCLUDE(retry_label)                                    \
  STMT_BEGIN                                                                  \
    if (result == NULL && try_excluding && !options->StrictNodes              \
        && n_excluded && !n_busy) {                                           \
      try_excluding = 0;                                                      \
      n_excluded = 0;                                                         \
      n_busy = 0;                                                             \
      try_ip_pref = 1;                                                        \
      goto retry_label;                                                       \
    }                                                                         \
  STMT_END

/* Common code used in the loop within router_pick_directory_server_impl and
 * router_pick_trusteddirserver_impl.
 *
 * Check if the given <b>identity</b> supports extrainfo. If not, skip further
 * checks.
 */
#define SKIP_MISSING_TRUSTED_EXTRAINFO(type, identity)                        \
  STMT_BEGIN                                                                  \
    int is_trusted_extrainfo = router_digest_is_trusted_dir_type(             \
                               (identity), EXTRAINFO_DIRINFO);                \
    if (((type) & EXTRAINFO_DIRINFO) &&                                       \
        !router_supports_extrainfo((identity), is_trusted_extrainfo))         \
      continue;                                                               \
  STMT_END

#ifndef LOG_FALSE_POSITIVES_DURING_BOOTSTRAP
#define LOG_FALSE_POSITIVES_DURING_BOOTSTRAP 0
#endif

/* Log a message if rs is not found or not a preferred address */
static void
router_picked_poor_directory_log(const routerstatus_t *rs)
{
  const networkstatus_t *usable_consensus;
  usable_consensus = networkstatus_get_reasonably_live_consensus(time(NULL),
                                                 usable_consensus_flavor());

#if !LOG_FALSE_POSITIVES_DURING_BOOTSTRAP
  /* Don't log early in the bootstrap process, it's normal to pick from a
   * small pool of nodes. Of course, this won't help if we're trying to
   * diagnose bootstrap issues. */
  if (!smartlist_len(nodelist_get_list()) || !usable_consensus
      || !router_have_minimum_dir_info()) {
    return;
  }
#endif /* !LOG_FALSE_POSITIVES_DURING_BOOTSTRAP */

  /* We couldn't find a node, or the one we have doesn't fit our preferences.
   * Sometimes this is normal, sometimes it can be a reachability issue. */
  if (!rs) {
    /* This happens a lot, so it's at debug level */
    log_debug(LD_DIR, "Wanted to make an outgoing directory connection, but "
              "we couldn't find a directory that fit our criteria. "
              "Perhaps we will succeed next time with less strict criteria.");
  } else if (!reachable_addr_allows_rs(rs, FIREWALL_OR_CONNECTION, 1)
             && !reachable_addr_allows_rs(rs, FIREWALL_DIR_CONNECTION, 1)
             ) {
    /* This is rare, and might be interesting to users trying to diagnose
     * connection issues on dual-stack machines. */
    char *ipv4_str = tor_addr_to_str_dup(&rs->ipv4_addr);
    log_info(LD_DIR, "Selected a directory %s with non-preferred OR and Dir "
             "addresses for launching an outgoing connection: "
             "IPv4 %s OR %d Dir %d IPv6 %s OR %d Dir %d",
             routerstatus_describe(rs),
             ipv4_str, rs->ipv4_orport,
             rs->ipv4_dirport, fmt_addr(&rs->ipv6_addr),
             rs->ipv6_orport, rs->ipv4_dirport);
    tor_free(ipv4_str);
  }
}

#undef LOG_FALSE_POSITIVES_DURING_BOOTSTRAP

/* Check if we already have a directory fetch from ap, for serverdesc
 * (including extrainfo) or microdesc documents.
 * If so, return 1, if not, return 0.
 * Also returns 0 if addr is NULL, tor_addr_is_null(addr), or dir_port is 0.
 */
STATIC int
router_is_already_dir_fetching(const tor_addr_port_t *ap, int serverdesc,
                               int microdesc)
{
  if (!ap || tor_addr_is_null(&ap->addr) || !ap->port) {
    return 0;
  }

  /* XX/teor - we're not checking tunnel connections here, see #17848
   */
  if (serverdesc && (
     connection_get_by_type_addr_port_purpose(
       CONN_TYPE_DIR, &ap->addr, ap->port, DIR_PURPOSE_FETCH_SERVERDESC)
  || connection_get_by_type_addr_port_purpose(
       CONN_TYPE_DIR, &ap->addr, ap->port, DIR_PURPOSE_FETCH_EXTRAINFO))) {
    return 1;
  }

  if (microdesc && (
     connection_get_by_type_addr_port_purpose(
       CONN_TYPE_DIR, &ap->addr, ap->port, DIR_PURPOSE_FETCH_MICRODESC))) {
    return 1;
  }

  return 0;
}

/* Check if we already have a directory fetch from the ipv4 or ipv6
 * router, for serverdesc (including extrainfo) or microdesc documents.
 * If so, return 1, if not, return 0.
 */
static int
router_is_already_dir_fetching_(const tor_addr_t *ipv4_addr,
                                const tor_addr_t *ipv6_addr,
                                uint16_t dir_port,
                                int serverdesc,
                                int microdesc)
{
  tor_addr_port_t ipv4_dir_ap, ipv6_dir_ap;

  /* Assume IPv6 DirPort is the same as IPv4 DirPort */
  tor_addr_copy(&ipv4_dir_ap.addr, ipv4_addr);
  ipv4_dir_ap.port = dir_port;
  tor_addr_copy(&ipv6_dir_ap.addr, ipv6_addr);
  ipv6_dir_ap.port = dir_port;

  return (router_is_already_dir_fetching(&ipv4_dir_ap, serverdesc, microdesc)
       || router_is_already_dir_fetching(&ipv6_dir_ap, serverdesc, microdesc));
}

/** Pick a random running valid directory server/mirror from our
 * routerlist.  Arguments are as for router_pick_directory_server(), except:
 *
 * If <b>n_busy_out</b> is provided, set *<b>n_busy_out</b> to the number of
 * directories that we excluded for no other reason than
 * PDS_NO_EXISTING_SERVERDESC_FETCH or PDS_NO_EXISTING_MICRODESC_FETCH.
 */
STATIC const routerstatus_t *
router_pick_directory_server_impl(dirinfo_type_t type, int flags,
                                  int *n_busy_out)
{
  const or_options_t *options = get_options();
  const node_t *result;
  smartlist_t *direct, *tunnel;
  smartlist_t *trusted_direct, *trusted_tunnel;
  smartlist_t *overloaded_direct, *overloaded_tunnel;
  time_t now = time(NULL);
  const networkstatus_t *consensus = networkstatus_get_latest_consensus();
  const int requireother = ! (flags & PDS_ALLOW_SELF);
  const int fascistfirewall = ! (flags & PDS_IGNORE_FASCISTFIREWALL);
  const int no_serverdesc_fetching =(flags & PDS_NO_EXISTING_SERVERDESC_FETCH);
  const int no_microdesc_fetching = (flags & PDS_NO_EXISTING_MICRODESC_FETCH);
  int try_excluding = 1, n_excluded = 0, n_busy = 0;
  int try_ip_pref = 1;

  if (!consensus)
    return NULL;

 retry_search:

  direct = smartlist_new();
  tunnel = smartlist_new();
  trusted_direct = smartlist_new();
  trusted_tunnel = smartlist_new();
  overloaded_direct = smartlist_new();
  overloaded_tunnel = smartlist_new();

  const int skip_or_fw = router_or_conn_should_skip_reachable_address_check(
                                                            options,
                                                            try_ip_pref);
  const int skip_dir_fw = router_dir_conn_should_skip_reachable_address_check(
                                                            options,
                                                            try_ip_pref);
  const int must_have_or = dirclient_must_use_begindir(options);

  /* Find all the running dirservers we know about. */
  SMARTLIST_FOREACH_BEGIN(nodelist_get_list(), const node_t *, node) {
    int is_trusted;
    int is_overloaded;
    const routerstatus_t *status = node->rs;
    const country_t country = node->country;
    if (!status)
      continue;

    if (!node->is_running || !node_is_dir(node) || !node->is_valid)
      continue;
    if (requireother && router_digest_is_me(node->identity))
      continue;

    SKIP_MISSING_TRUSTED_EXTRAINFO(type, node->identity);

    if (try_excluding &&
        routerset_contains_routerstatus(options->ExcludeNodes, status,
                                        country)) {
      ++n_excluded;
      continue;
    }

    if (router_is_already_dir_fetching_(&status->ipv4_addr,
                                        &status->ipv6_addr,
                                        status->ipv4_dirport,
                                        no_serverdesc_fetching,
                                        no_microdesc_fetching)) {
      ++n_busy;
      continue;
    }

    is_overloaded = status->last_dir_503_at + DIR_503_TIMEOUT > now;
    is_trusted = router_digest_is_trusted_dir(node->identity);

    /* Clients use IPv6 addresses if the server has one and the client
     * prefers IPv6.
     * Add the router if its preferred address and port are reachable.
     * If we don't get any routers, we'll try again with the non-preferred
     * address for each router (if any). (To ensure correct load-balancing
     * we try routers that only have one address both times.)
     */
    if (!fascistfirewall || skip_or_fw ||
        reachable_addr_allows_node(node, FIREWALL_OR_CONNECTION,
                                     try_ip_pref))
      smartlist_add(is_trusted ? trusted_tunnel :
                    is_overloaded ? overloaded_tunnel : tunnel, (void*)node);
    else if (!must_have_or && (skip_dir_fw ||
             reachable_addr_allows_node(node, FIREWALL_DIR_CONNECTION,
                                          try_ip_pref)))
      smartlist_add(is_trusted ? trusted_direct :
                    is_overloaded ? overloaded_direct : direct, (void*)node);
  } SMARTLIST_FOREACH_END(node);

  if (smartlist_len(tunnel)) {
    result = node_sl_choose_by_bandwidth(tunnel, WEIGHT_FOR_DIR);
  } else if (smartlist_len(overloaded_tunnel)) {
    result = node_sl_choose_by_bandwidth(overloaded_tunnel,
                                                 WEIGHT_FOR_DIR);
  } else if (smartlist_len(trusted_tunnel)) {
    /* FFFF We don't distinguish between trusteds and overloaded trusteds
     * yet. Maybe one day we should. */
    /* FFFF We also don't load balance over authorities yet. I think this
     * is a feature, but it could easily be a bug. -RD */
    result = smartlist_choose(trusted_tunnel);
  } else if (smartlist_len(direct)) {
    result = node_sl_choose_by_bandwidth(direct, WEIGHT_FOR_DIR);
  } else if (smartlist_len(overloaded_direct)) {
    result = node_sl_choose_by_bandwidth(overloaded_direct,
                                         WEIGHT_FOR_DIR);
  } else {
    result = smartlist_choose(trusted_direct);
  }
  smartlist_free(direct);
  smartlist_free(tunnel);
  smartlist_free(trusted_direct);
  smartlist_free(trusted_tunnel);
  smartlist_free(overloaded_direct);
  smartlist_free(overloaded_tunnel);

  RETRY_ALTERNATE_IP_VERSION(retry_search);

  RETRY_WITHOUT_EXCLUDE(retry_search);

  if (n_busy_out)
    *n_busy_out = n_busy;

  router_picked_poor_directory_log(result ? result->rs : NULL);

  return result ? result->rs : NULL;
}

/** Given an array of double/uint64_t unions that are currently being used as
 * doubles, convert them to uint64_t, and try to scale them linearly so as to
 * much of the range of uint64_t. If <b>total_out</b> is provided, set it to
 * the sum of all elements in the array _before_ scaling. */
STATIC void
scale_array_elements_to_u64(uint64_t *entries_out, const double *entries_in,
                            int n_entries,
                            uint64_t *total_out)
{
  double total = 0.0;
  double scale_factor = 0.0;
  int i;

  for (i = 0; i < n_entries; ++i)
    total += entries_in[i];

  if (total > 0.0) {
    scale_factor = ((double)INT64_MAX) / total;
    scale_factor /= 4.0; /* make sure we're very far away from overflowing */
  }

  for (i = 0; i < n_entries; ++i)
    entries_out[i] = tor_llround(entries_in[i] * scale_factor);

  if (total_out)
    *total_out = (uint64_t) total;
}

/** Pick a random element of <b>n_entries</b>-element array <b>entries</b>,
 * choosing each element with a probability proportional to its (uint64_t)
 * value, and return the index of that element.  If all elements are 0, choose
 * an index at random. Return -1 on error.
 */
STATIC int
choose_array_element_by_weight(const uint64_t *entries, int n_entries)
{
  int i;
  uint64_t rand_val;
  uint64_t total = 0;

  for (i = 0; i < n_entries; ++i)
    total += entries[i];

  if (n_entries < 1)
    return -1;

  if (total == 0)
    return crypto_rand_int(n_entries);

  tor_assert(total < INT64_MAX);

  rand_val = crypto_rand_uint64(total);

  return select_array_member_cumulative_timei(
                           entries, n_entries, total, rand_val);
}

/** Return bw*1000, unless bw*1000 would overflow, in which case return
 * INT32_MAX. */
static inline int32_t
kb_to_bytes(uint32_t bw)
{
  return (bw > (INT32_MAX/1000)) ? INT32_MAX : bw*1000;
}

/** Helper function:
 * choose a random element of smartlist <b>sl</b> of nodes, weighted by
 * the advertised bandwidth of each element using the consensus
 * bandwidth weights.
 *
 * If <b>rule</b>==WEIGHT_FOR_EXIT. we're picking an exit node: consider all
 * nodes' bandwidth equally regardless of their Exit status, since there may
 * be some in the list because they exit to obscure ports. If
 * <b>rule</b>==NO_WEIGHTING, we're picking a non-exit node: weight
 * exit-node's bandwidth less depending on the smallness of the fraction of
 * Exit-to-total bandwidth.  If <b>rule</b>==WEIGHT_FOR_GUARD, we're picking a
 * guard node: consider all guard's bandwidth equally. Otherwise, weight
 * guards proportionally less.
 */
static const node_t *
smartlist_choose_node_by_bandwidth_weights(const smartlist_t *sl,
                                           bandwidth_weight_rule_t rule)
{
  double *bandwidths_dbl=NULL;
  uint64_t *bandwidths_u64=NULL;

  if (compute_weighted_bandwidths(sl, rule, &bandwidths_dbl, NULL) < 0)
    return NULL;

  bandwidths_u64 = tor_calloc(smartlist_len(sl), sizeof(uint64_t));
  scale_array_elements_to_u64(bandwidths_u64, bandwidths_dbl,
                              smartlist_len(sl), NULL);

  {
    int idx = choose_array_element_by_weight(bandwidths_u64,
                                             smartlist_len(sl));
    tor_free(bandwidths_dbl);
    tor_free(bandwidths_u64);
    return idx < 0 ? NULL : smartlist_get(sl, idx);
  }
}

/** When weighting bridges, enforce these values as lower and upper
 * bound for believable bandwidth, because there is no way for us
 * to verify a bridge's bandwidth currently. */
#define BRIDGE_MIN_BELIEVABLE_BANDWIDTH 20000  /* 20 kB/sec */
#define BRIDGE_MAX_BELIEVABLE_BANDWIDTH 100000 /* 100 kB/sec */

/** Return the smaller of the router's configured BandwidthRate
 * and its advertised capacity, making sure to stay within the
 * interval between bridge-min-believe-bw and
 * bridge-max-believe-bw. */
static uint32_t
bridge_get_advertised_bandwidth_bounded(routerinfo_t *router)
{
  uint32_t result = router->bandwidthcapacity;
  if (result > router->bandwidthrate)
    result = router->bandwidthrate;
  if (result > BRIDGE_MAX_BELIEVABLE_BANDWIDTH)
    result = BRIDGE_MAX_BELIEVABLE_BANDWIDTH;
  else if (result < BRIDGE_MIN_BELIEVABLE_BANDWIDTH)
    result = BRIDGE_MIN_BELIEVABLE_BANDWIDTH;
  return result;
}

/**
 * We have found an instance of bug 32868: log our best guess about where the
 * routerstatus was found.
 **/
static void
log_buggy_rs_source(const routerstatus_t *rs)
{
  static ratelim_t buggy_rs_ratelim = RATELIM_INIT(1200);
  char *m;
  if ((m = rate_limit_log(&buggy_rs_ratelim, approx_time()))) {
    log_warn(LD_BUG,
             "Found a routerstatus %p with has_guardfraction=%u "
             " and guardfraction_percentage=%u, but is_possible_guard=%u.%s",
             rs,
             rs->has_guardfraction,
             rs->guardfraction_percentage,
             rs->is_possible_guard,
             m);
    tor_free(m);
    networkstatus_t *ns;
    int in_ns_count = 0;
    if ((ns = networkstatus_get_latest_consensus_by_flavor(FLAV_NS))) {
      int pos = smartlist_pos(ns->routerstatus_list, rs);
      if (pos >= 0) {
        ++in_ns_count;
        log_warn(LD_BUG, "Found the routerstatus at position %d of the "
                 "NS consensus.", pos);
      }
    }
    if ((ns = networkstatus_get_latest_consensus_by_flavor(FLAV_MICRODESC))) {
      int pos = smartlist_pos(ns->routerstatus_list, rs);
      if (pos >= 0) {
        ++in_ns_count;
        log_warn(LD_BUG, "Found the routerstatus at position %d of the "
                 "MD consensus.", pos);
      }
    }
    if (in_ns_count == 0) {
      log_warn(LD_BUG, "Could not find the routerstatus in any "
               "latest consensus.");
    }
    tor_assert_nonfatal_unreached();
  }
}

/** Given a list of routers and a weighting rule as in
 * smartlist_choose_node_by_bandwidth_weights, compute weighted bandwidth
 * values for each node and store them in a freshly allocated
 * *<b>bandwidths_out</b> of the same length as <b>sl</b>, and holding results
 * as doubles. If <b>total_bandwidth_out</b> is non-NULL, set it to the total
 * of all the bandwidths.
 * Return 0 on success, -1 on failure. */
static int
compute_weighted_bandwidths(const smartlist_t *sl,
                            bandwidth_weight_rule_t rule,
                            double **bandwidths_out,
                            double *total_bandwidth_out)
{
  int64_t weight_scale;
  double Wg = -1, Wm = -1, We = -1, Wd = -1;
  double Wgb = -1, Wmb = -1, Web = -1, Wdb = -1;
  guardfraction_bandwidth_t guardfraction_bw;
  double *bandwidths = NULL;
  double total_bandwidth = 0.0;

  tor_assert(sl);
  tor_assert(bandwidths_out);

  /* Can't choose exit and guard at same time */
  tor_assert(rule == NO_WEIGHTING ||
             rule == WEIGHT_FOR_EXIT ||
             rule == WEIGHT_FOR_GUARD ||
             rule == WEIGHT_FOR_MID ||
             rule == WEIGHT_FOR_DIR);

  *bandwidths_out = NULL;

  if (total_bandwidth_out) {
    *total_bandwidth_out = 0.0;
  }

  if (smartlist_len(sl) == 0) {
    log_info(LD_CIRC,
             "Empty routerlist passed in to consensus weight node "
             "selection for rule %s",
             bandwidth_weight_rule_to_string(rule));
    return -1;
  }

  weight_scale = networkstatus_get_weight_scale_param(NULL);
  tor_assert(weight_scale >= 1);

  if (rule == WEIGHT_FOR_GUARD) {
    Wg = networkstatus_get_bw_weight(NULL, "Wgg", -1);
    Wm = networkstatus_get_bw_weight(NULL, "Wgm", -1); /* Bridges */
    We = 0;
    Wd = networkstatus_get_bw_weight(NULL, "Wgd", -1);

    Wgb = networkstatus_get_bw_weight(NULL, "Wgb", -1);
    Wmb = networkstatus_get_bw_weight(NULL, "Wmb", -1);
    Web = networkstatus_get_bw_weight(NULL, "Web", -1);
    Wdb = networkstatus_get_bw_weight(NULL, "Wdb", -1);
  } else if (rule == WEIGHT_FOR_MID) {
    Wg = networkstatus_get_bw_weight(NULL, "Wmg", -1);
    Wm = networkstatus_get_bw_weight(NULL, "Wmm", -1);
    We = networkstatus_get_bw_weight(NULL, "Wme", -1);
    Wd = networkstatus_get_bw_weight(NULL, "Wmd", -1);

    Wgb = networkstatus_get_bw_weight(NULL, "Wgb", -1);
    Wmb = networkstatus_get_bw_weight(NULL, "Wmb", -1);
    Web = networkstatus_get_bw_weight(NULL, "Web", -1);
    Wdb = networkstatus_get_bw_weight(NULL, "Wdb", -1);
  } else if (rule == WEIGHT_FOR_EXIT) {
    // Guards CAN be exits if they have weird exit policies
    // They are d then I guess...
    We = networkstatus_get_bw_weight(NULL, "Wee", -1);
    Wm = networkstatus_get_bw_weight(NULL, "Wem", -1); /* Odd exit policies */
    Wd = networkstatus_get_bw_weight(NULL, "Wed", -1);
    Wg = networkstatus_get_bw_weight(NULL, "Weg", -1); /* Odd exit policies */

    Wgb = networkstatus_get_bw_weight(NULL, "Wgb", -1);
    Wmb = networkstatus_get_bw_weight(NULL, "Wmb", -1);
    Web = networkstatus_get_bw_weight(NULL, "Web", -1);
    Wdb = networkstatus_get_bw_weight(NULL, "Wdb", -1);
  } else if (rule == WEIGHT_FOR_DIR) {
    We = networkstatus_get_bw_weight(NULL, "Wbe", -1);
    Wm = networkstatus_get_bw_weight(NULL, "Wbm", -1);
    Wd = networkstatus_get_bw_weight(NULL, "Wbd", -1);
    Wg = networkstatus_get_bw_weight(NULL, "Wbg", -1);

    Wgb = Wmb = Web = Wdb = weight_scale;
  } else if (rule == NO_WEIGHTING) {
    Wg = Wm = We = Wd = weight_scale;
    Wgb = Wmb = Web = Wdb = weight_scale;
  }

  if (Wg < 0 || Wm < 0 || We < 0 || Wd < 0 || Wgb < 0 || Wmb < 0 || Wdb < 0
      || Web < 0) {
    log_debug(LD_CIRC,
              "Got negative bandwidth weights. Defaulting to naive selection"
              " algorithm.");
    Wg = Wm = We = Wd = weight_scale;
    Wgb = Wmb = Web = Wdb = weight_scale;
  }

  Wg /= weight_scale;
  Wm /= weight_scale;
  We /= weight_scale;
  Wd /= weight_scale;

  Wgb /= weight_scale;
  Wmb /= weight_scale;
  Web /= weight_scale;
  Wdb /= weight_scale;

  bandwidths = tor_calloc(smartlist_len(sl), sizeof(double));

  // Cycle through smartlist and total the bandwidth.
  static int warned_missing_bw = 0;
  SMARTLIST_FOREACH_BEGIN(sl, const node_t *, node) {
    int is_exit = 0, is_guard = 0, is_dir = 0, this_bw = 0;
    double weight = 1;
    double weight_without_guard_flag = 0; /* Used for guardfraction */
    double final_weight = 0;
    is_exit = node->is_exit && ! node->is_bad_exit;
    is_guard = node->is_possible_guard;
    is_dir = node_is_dir(node);
    if (node->rs) {
      if (!node->rs->has_bandwidth) {
        /* This should never happen, unless all the authorities downgrade
         * to 0.2.0 or rogue routerstatuses get inserted into our consensus. */
        if (! warned_missing_bw) {
          log_warn(LD_BUG,
                 "Consensus is missing some bandwidths. Using a naive "
                 "router selection algorithm");
          warned_missing_bw = 1;
        }
        this_bw = 30000; /* Chosen arbitrarily */
      } else {
        this_bw = kb_to_bytes(node->rs->bandwidth_kb);
      }
    } else if (node->ri) {
      /* bridge or other descriptor not in our consensus */
      this_bw = bridge_get_advertised_bandwidth_bounded(node->ri);
    } else {
      /* We can't use this one. */
      continue;
    }

    if (is_guard && is_exit) {
      weight = (is_dir ? Wdb*Wd : Wd);
      weight_without_guard_flag = (is_dir ? Web*We : We);
    } else if (is_guard) {
      weight = (is_dir ? Wgb*Wg : Wg);
      weight_without_guard_flag = (is_dir ? Wmb*Wm : Wm);
    } else if (is_exit) {
      weight = (is_dir ? Web*We : We);
    } else { // middle
      weight = (is_dir ? Wmb*Wm : Wm);
    }
    /* These should be impossible; but overflows here would be bad, so let's
     * make sure. */
    if (this_bw < 0)
      this_bw = 0;
    if (weight < 0.0)
      weight = 0.0;
    if (weight_without_guard_flag < 0.0)
      weight_without_guard_flag = 0.0;

    /* If guardfraction information is available in the consensus, we
     * want to calculate this router's bandwidth according to its
     * guardfraction. Quoting from proposal236:
     *
     *    Let Wpf denote the weight from the 'bandwidth-weights' line a
     *    client would apply to N for position p if it had the guard
     *    flag, Wpn the weight if it did not have the guard flag, and B the
     *    measured bandwidth of N in the consensus.  Then instead of choosing
     *    N for position p proportionally to Wpf*B or Wpn*B, clients should
     *    choose N proportionally to F*Wpf*B + (1-F)*Wpn*B.
     */
    if (node->rs && node->rs->has_guardfraction && rule != WEIGHT_FOR_GUARD) {
      /* We should only have guardfraction set if the node has the Guard
         flag. */
      if (! node->rs->is_possible_guard) {
        log_buggy_rs_source(node->rs);
      }

      guard_get_guardfraction_bandwidth(&guardfraction_bw,
                                        this_bw,
                                        node->rs->guardfraction_percentage);

      /* Calculate final_weight = F*Wpf*B + (1-F)*Wpn*B */
      final_weight =
        guardfraction_bw.guard_bw * weight +
        guardfraction_bw.non_guard_bw * weight_without_guard_flag;

      log_debug(LD_GENERAL, "%s: Guardfraction weight %f instead of %f (%s)",
                node->rs->nickname, final_weight, weight*this_bw,
                bandwidth_weight_rule_to_string(rule));
    } else { /* no guardfraction information. calculate the weight normally. */
      final_weight = weight*this_bw;
    }

    bandwidths[node_sl_idx] = final_weight;
    total_bandwidth += final_weight;
  } SMARTLIST_FOREACH_END(node);

  log_debug(LD_CIRC, "Generated weighted bandwidths for rule %s based "
            "on weights "
            "Wg=%f Wm=%f We=%f Wd=%f with total bw %f",
            bandwidth_weight_rule_to_string(rule),
            Wg, Wm, We, Wd, total_bandwidth);

  *bandwidths_out = bandwidths;

  if (total_bandwidth_out) {
    *total_bandwidth_out = total_bandwidth;
  }

  return 0;
}

/** For all nodes in <b>sl</b>, return the fraction of those nodes, weighted
 * by their weighted bandwidths with rule <b>rule</b>, for which we have
 * descriptors.
 *
 * If <b>for_direct_connect</b> is true, we intend to connect to the node
 * directly, as the first hop of a circuit; otherwise, we intend to connect
 * to it indirectly, or use it as if we were connecting to it indirectly. */
double
frac_nodes_with_descriptors(const smartlist_t *sl,
                            bandwidth_weight_rule_t rule,
                            int for_direct_conn)
{
  double *bandwidths = NULL;
  double total, present;

  if (smartlist_len(sl) == 0)
    return 0.0;

  if (compute_weighted_bandwidths(sl, rule, &bandwidths, &total) < 0 ||
      total <= 0.0) {
    int n_with_descs = 0;
    SMARTLIST_FOREACH(sl, const node_t *, node, {
      if (node_has_preferred_descriptor(node, for_direct_conn))
        n_with_descs++;
    });
    tor_free(bandwidths);
    return ((double)n_with_descs) / smartlist_len(sl);
  }

  present = 0.0;
  SMARTLIST_FOREACH_BEGIN(sl, const node_t *, node) {
    if (node_has_preferred_descriptor(node, for_direct_conn))
      present += bandwidths[node_sl_idx];
  } SMARTLIST_FOREACH_END(node);

  tor_free(bandwidths);

  return present / total;
}

/** Choose a random element of status list <b>sl</b>, weighted by
 * the advertised bandwidth of each node */
const node_t *
node_sl_choose_by_bandwidth(const smartlist_t *sl,
                            bandwidth_weight_rule_t rule)
{ /*XXXX MOVE */
  return smartlist_choose_node_by_bandwidth_weights(sl, rule);
}

/** Given a <b>router</b>, add every node_t in its family (including the
 * node itself!) to <b>sl</b>.
 *
 * Note the type mismatch: This function takes a routerinfo, but adds nodes
 * to the smartlist!
 */
static void
routerlist_add_node_and_family(smartlist_t *sl, const routerinfo_t *router)
{
  /* XXXX MOVE ? */
  node_t fake_node;
  const node_t *node = node_get_by_id(router->cache_info.identity_digest);
  if (node == NULL) {
    memset(&fake_node, 0, sizeof(fake_node));
    fake_node.ri = (routerinfo_t *)router;
    memcpy(fake_node.identity, router->cache_info.identity_digest, DIGEST_LEN);
    node = &fake_node;
  }
  nodelist_add_node_and_family(sl, node);
}

/**
 * Remove every node_t that appears in <b>excluded</b> from <b>sl</b>.
 *
 * Behaves like smartlist_subtract, but uses nodelist_idx values to deliver
 * linear performance when smartlist_subtract would be quadratic.
 **/
static void
nodelist_subtract(smartlist_t *sl, const smartlist_t *excluded)
{
  const smartlist_t *nodelist = nodelist_get_list();
  const int nodelist_len = smartlist_len(nodelist);
  bitarray_t *excluded_idx = bitarray_init_zero(nodelist_len);

  /* We haven't used nodelist_idx in this way previously, so I'm going to be
   * paranoid in this code, and check that nodelist_idx is correct for every
   * node before we use it.  If we fail, we fall back to smartlist_subtract().
   */

  /* Set the excluded_idx bit corresponding to every excluded node...
   */
  SMARTLIST_FOREACH_BEGIN(excluded, const node_t *, node) {
    const int idx = node->nodelist_idx;
    if (BUG(idx < 0) || BUG(idx >= nodelist_len) ||
        BUG(node != smartlist_get(nodelist, idx))) {
      goto internal_error;
    }
    bitarray_set(excluded_idx, idx);
  } SMARTLIST_FOREACH_END(node);

  /* Then remove them from sl.
   */
  SMARTLIST_FOREACH_BEGIN(sl, const node_t *, node) {
    const int idx = node->nodelist_idx;
    if (BUG(idx < 0) || BUG(idx >= nodelist_len) ||
        BUG(node != smartlist_get(nodelist, idx))) {
      goto internal_error;
    }
    if (bitarray_is_set(excluded_idx, idx)) {
      SMARTLIST_DEL_CURRENT(sl, node);
    }
  } SMARTLIST_FOREACH_END(node);

  bitarray_free(excluded_idx);
  return;

 internal_error:
  log_warn(LD_BUG, "Internal error prevented us from using the fast method "
           "for subtracting nodelists. Falling back to the quadratic way.");
  smartlist_subtract(sl, excluded);
  bitarray_free(excluded_idx);
}

/* Node selection helper for router_choose_random_node().
 *
 * Populates a node list based on <b>flags</b>, ignoring nodes in
 * <b>excludednodes</b> and <b>excludedset</b>. Chooses the node based on
 * <b>rule</b>. */
static const node_t *
router_choose_random_node_helper(smartlist_t *excludednodes,
                                 routerset_t *excludedset,
                                 router_crn_flags_t flags,
                                 bandwidth_weight_rule_t rule)
{
  smartlist_t *sl=smartlist_new();
  const node_t *choice = NULL;

  router_add_running_nodes_to_smartlist(sl, flags);
  log_debug(LD_CIRC,
           "We found %d running nodes.",
            smartlist_len(sl));

  nodelist_subtract(sl, excludednodes);

  if (excludedset) {
    routerset_subtract_nodes(sl,excludedset);
    log_debug(LD_CIRC,
              "We removed excludedset, leaving %d nodes.",
              smartlist_len(sl));
  }

  // Always weight by bandwidth
  choice = node_sl_choose_by_bandwidth(sl, rule);

  smartlist_free(sl);

  return choice;
}

/** Return a random running node from the nodelist. Never pick a node that is
 * in <b>excludedsmartlist</b>, or which matches <b>excludedset</b>, even if
 * they are the only nodes available.
 *
 * <b>flags</b> is a set of CRN_* flags, see
 * router_add_running_nodes_to_smartlist() for details.
 */
const node_t *
router_choose_random_node(smartlist_t *excludedsmartlist,
                          routerset_t *excludedset,
                          router_crn_flags_t flags)
{
  /* A limited set of flags, used for fallback node selection.
   */
  const bool need_uptime = (flags & CRN_NEED_UPTIME) != 0;
  const bool need_capacity = (flags & CRN_NEED_CAPACITY) != 0;
  const bool need_guard = (flags & CRN_NEED_GUARD) != 0;
  const bool pref_addr = (flags & CRN_PREF_ADDR) != 0;

  smartlist_t *excludednodes=smartlist_new();
  const node_t *choice = NULL;
  const routerinfo_t *r;
  bandwidth_weight_rule_t rule;

  rule = (need_guard ? WEIGHT_FOR_GUARD : WEIGHT_FOR_MID);

  /* If the node_t is not found we won't be to exclude ourself but we
   * won't be able to pick ourself in router_choose_random_node() so
   * this is fine to at least try with our routerinfo_t object. */
  if ((r = router_get_my_routerinfo()))
    routerlist_add_node_and_family(excludednodes, r);

  if (excludedsmartlist) {
    smartlist_add_all(excludednodes, excludedsmartlist);
  }

  choice = router_choose_random_node_helper(excludednodes,
                                            excludedset,
                                            flags,
                                            rule);

  if (!choice && (need_uptime || need_capacity || need_guard || pref_addr)) {
    /* try once more, with fewer restrictions. */
    log_info(LD_CIRC,
             "We couldn't find any live%s%s%s%s routers; falling back "
             "to list of all routers.",
             need_capacity?", fast":"",
             need_uptime?", stable":"",
             need_guard?", guard":"",
             pref_addr?", preferred address":"");
    flags &= ~ (CRN_NEED_UPTIME|CRN_NEED_CAPACITY|CRN_NEED_GUARD|
                CRN_PREF_ADDR);
    choice = router_choose_random_node_helper(excludednodes,
                                              excludedset,
                                              flags,
                                              rule);
  }
  smartlist_free(excludednodes);
  if (!choice) {
    log_warn(LD_CIRC,
             "No available nodes when trying to choose node. Failing.");
  }
  return choice;
}

/** Try to find a running directory authority. Flags are as for
 * router_pick_directory_server.
 */
const routerstatus_t *
router_pick_trusteddirserver(dirinfo_type_t type, int flags)
{
  return router_pick_dirserver_generic(
                                  router_get_trusted_dir_servers_mutable(),
                                  type, flags);
}

/** Try to find a running fallback directory. Flags are as for
 * router_pick_directory_server.
 */
const routerstatus_t *
router_pick_fallback_dirserver(dirinfo_type_t type, int flags)
{
  return router_pick_dirserver_generic(
                                  router_get_fallback_dir_servers_mutable(),
                                  type, flags);
}

/** Pick a random element from a list of dir_server_t, weighting by their
 * <b>weight</b> field. */
static const dir_server_t *
dirserver_choose_by_weight(const smartlist_t *servers, double authority_weight)
{
  int n = smartlist_len(servers);
  int i;
  double *weights_dbl;
  uint64_t *weights_u64;
  const dir_server_t *ds;

  weights_dbl = tor_calloc(n, sizeof(double));
  weights_u64 = tor_calloc(n, sizeof(uint64_t));
  for (i = 0; i < n; ++i) {
    ds = smartlist_get(servers, i);
    weights_dbl[i] = ds->weight;
    if (ds->is_authority)
      weights_dbl[i] *= authority_weight;
  }

  scale_array_elements_to_u64(weights_u64, weights_dbl, n, NULL);
  i = choose_array_element_by_weight(weights_u64, n);
  tor_free(weights_dbl);
  tor_free(weights_u64);
  return (i < 0) ? NULL : smartlist_get(servers, i);
}

/** Choose randomly from among the dir_server_ts in sourcelist that
 * are up. Flags are as for router_pick_directory_server_impl().
 */
static const routerstatus_t *
router_pick_trusteddirserver_impl(const smartlist_t *sourcelist,
                                  dirinfo_type_t type, int flags,
                                  int *n_busy_out)
{
  const or_options_t *options = get_options();
  smartlist_t *direct, *tunnel;
  smartlist_t *overloaded_direct, *overloaded_tunnel;
  const routerinfo_t *me = router_get_my_routerinfo();
  const routerstatus_t *result = NULL;
  time_t now = time(NULL);
  const int requireother = ! (flags & PDS_ALLOW_SELF);
  const int fascistfirewall = ! (flags & PDS_IGNORE_FASCISTFIREWALL);
  const int no_serverdesc_fetching =(flags & PDS_NO_EXISTING_SERVERDESC_FETCH);
  const int no_microdesc_fetching =(flags & PDS_NO_EXISTING_MICRODESC_FETCH);
  const double auth_weight =
    (sourcelist == router_get_fallback_dir_servers()) ?
    options->DirAuthorityFallbackRate : 1.0;
  smartlist_t *pick_from;
  int n_busy = 0;
  int try_excluding = 1, n_excluded = 0;
  int try_ip_pref = 1;

  if (!sourcelist)
    return NULL;

 retry_search:

  direct = smartlist_new();
  tunnel = smartlist_new();
  overloaded_direct = smartlist_new();
  overloaded_tunnel = smartlist_new();

  const int skip_or_fw = router_or_conn_should_skip_reachable_address_check(
                                                            options,
                                                            try_ip_pref);
  const int skip_dir_fw = router_dir_conn_should_skip_reachable_address_check(
                                                            options,
                                                            try_ip_pref);
  const int must_have_or = dirclient_must_use_begindir(options);

  SMARTLIST_FOREACH_BEGIN(sourcelist, const dir_server_t *, d)
    {
      int is_overloaded =
          d->fake_status.last_dir_503_at + DIR_503_TIMEOUT > now;
      if (!d->is_running) continue;
      if ((type & d->type) == 0)
        continue;

      SKIP_MISSING_TRUSTED_EXTRAINFO(type, d->digest);

      if (requireother && me && router_digest_is_me(d->digest))
        continue;
      if (try_excluding &&
          routerset_contains_routerstatus(options->ExcludeNodes,
                                          &d->fake_status, -1)) {
        ++n_excluded;
        continue;
      }

      if (router_is_already_dir_fetching_(&d->ipv4_addr,
                                          &d->ipv6_addr,
                                          d->ipv4_dirport,
                                          no_serverdesc_fetching,
                                          no_microdesc_fetching)) {
        ++n_busy;
        continue;
      }

      /* Clients use IPv6 addresses if the server has one and the client
       * prefers IPv6.
       * Add the router if its preferred address and port are reachable.
       * If we don't get any routers, we'll try again with the non-preferred
       * address for each router (if any). (To ensure correct load-balancing
       * we try routers that only have one address both times.)
       */
      if (!fascistfirewall || skip_or_fw ||
          reachable_addr_allows_dir_server(d, FIREWALL_OR_CONNECTION,
                                             try_ip_pref))
        smartlist_add(is_overloaded ? overloaded_tunnel : tunnel, (void*)d);
      else if (!must_have_or && (skip_dir_fw ||
               reachable_addr_allows_dir_server(d, FIREWALL_DIR_CONNECTION,
                                                  try_ip_pref)))
        smartlist_add(is_overloaded ? overloaded_direct : direct, (void*)d);
    }
  SMARTLIST_FOREACH_END(d);

  if (smartlist_len(tunnel)) {
    pick_from = tunnel;
  } else if (smartlist_len(overloaded_tunnel)) {
    pick_from = overloaded_tunnel;
  } else if (smartlist_len(direct)) {
    pick_from = direct;
  } else {
    pick_from = overloaded_direct;
  }

  {
    const dir_server_t *selection =
      dirserver_choose_by_weight(pick_from, auth_weight);

    if (selection)
      result = &selection->fake_status;
  }

  smartlist_free(direct);
  smartlist_free(tunnel);
  smartlist_free(overloaded_direct);
  smartlist_free(overloaded_tunnel);

  RETRY_ALTERNATE_IP_VERSION(retry_search);

  RETRY_WITHOUT_EXCLUDE(retry_search);

  router_picked_poor_directory_log(result);

  if (n_busy_out)
    *n_busy_out = n_busy;
  return result;
}
