/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_metrics.c
 * @brief Relay metrics exposed through the MetricsPort
 **/

#define RELAY_METRICS_ENTRY_PRIVATE

#include "orconfig.h"

#include "core/or/or.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_vegas.h"
#include "core/or/congestion_control_flow.h"
#include "core/or/circuitlist.h"
#include "core/or/dos.h"
#include "core/or/relay.h"

#include "app/config/config.h"

#include "lib/container/smartlist.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/math/fp.h"
#include "lib/metrics/metrics_store.h"

#include "feature/hs/hs_dos.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/torcert.h"
#include "feature/relay/relay_metrics.h"
#include "feature/relay/router.h"
#include "feature/relay/routerkeys.h"
#include "feature/stats/rephist.h"

#include <event2/dns.h>

/** Declarations of each fill function for metrics defined in base_metrics. */
static void fill_cc_counters_values(void);
static void fill_cc_gauges_values(void);
static void fill_circuits_values(void);
static void fill_conn_counter_values(void);
static void fill_conn_gauge_values(void);
static void fill_dns_error_values(void);
static void fill_dns_query_values(void);
static void fill_dos_values(void);
static void fill_global_bw_limit_values(void);
static void fill_socket_values(void);
static void fill_onionskins_values(void);
static void fill_oom_values(void);
static void fill_streams_values(void);
static void fill_relay_flags(void);
static void fill_tcp_exhaustion_values(void);
static void fill_traffic_values(void);
static void fill_signing_cert_expiry(void);

static void fill_est_intro_cells(void);
static void fill_est_rend_cells(void);
static void fill_intro1_cells(void);
static void fill_rend1_cells(void);

/** The base metrics that is a static array of metrics added to the metrics
 * store.
 *
 * The key member MUST be also the index of the entry in the array. */
static const relay_metrics_entry_t base_metrics[] =
{
  {
    .key = RELAY_METRICS_NUM_OOM_BYTES,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_load_oom_bytes_total),
    .help = "Total number of bytes the OOM has freed by subsystem",
    .fill_fn = fill_oom_values,
  },
  {
    .key = RELAY_METRICS_NUM_ONIONSKINS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_load_onionskins_total),
    .help = "Total number of onionskins handled",
    .fill_fn = fill_onionskins_values,
  },
  {
    .key = RELAY_METRICS_NUM_SOCKETS,
    .type = METRICS_TYPE_GAUGE,
    .name = METRICS_NAME(relay_load_socket_total),
    .help = "Total number of sockets",
    .fill_fn = fill_socket_values,
  },
  {
    .key = RELAY_METRICS_NUM_GLOBAL_RW_LIMIT,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_load_global_rate_limit_reached_total),
    .help = "Total number of global connection bucket limit reached",
    .fill_fn = fill_global_bw_limit_values,
  },
  {
    .key = RELAY_METRICS_NUM_DNS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_exit_dns_query_total),
    .help = "Total number of DNS queries done by this relay",
    .fill_fn = fill_dns_query_values,
  },
  {
    .key = RELAY_METRICS_NUM_DNS_ERRORS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_exit_dns_error_total),
    .help = "Total number of DNS errors encountered by this relay",
    .fill_fn = fill_dns_error_values,
  },
  {
    .key = RELAY_METRICS_NUM_TCP_EXHAUSTION,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_load_tcp_exhaustion_total),
    .help = "Total number of times we ran out of TCP ports",
    .fill_fn = fill_tcp_exhaustion_values,
  },
  {
    .key = RELAY_METRICS_CONN_COUNTERS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_connections_total),
    .help = "Total number of created/rejected connections",
    .fill_fn = fill_conn_counter_values,
  },
  {
    .key = RELAY_METRICS_CONN_GAUGES,
    .type = METRICS_TYPE_GAUGE,
    .name = METRICS_NAME(relay_connections),
    .help = "Total number of opened connections",
    .fill_fn = fill_conn_gauge_values,
  },
  {
    .key = RELAY_METRICS_NUM_STREAMS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_streams_total),
    .help = "Total number of streams",
    .fill_fn = fill_streams_values,
  },
  {
    .key = RELAY_METRICS_CC_COUNTERS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_congestion_control_total),
    .help = "Congestion control related counters",
    .fill_fn = fill_cc_counters_values,
  },
  {
    .key = RELAY_METRICS_CC_GAUGES,
    .type = METRICS_TYPE_GAUGE,
    .name = METRICS_NAME(relay_congestion_control),
    .help = "Congestion control related gauges",
    .fill_fn = fill_cc_gauges_values,
  },
  {
    .key = RELAY_METRICS_NUM_DOS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_dos_total),
    .help = "Denial of Service defenses related counters",
    .fill_fn = fill_dos_values,
  },
  {
    .key = RELAY_METRICS_NUM_TRAFFIC,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_traffic_bytes),
    .help = "Traffic related counters",
    .fill_fn = fill_traffic_values,
  },
  {
    .key = RELAY_METRICS_RELAY_FLAGS,
    .type = METRICS_TYPE_GAUGE,
    .name = METRICS_NAME(relay_flag),
    .help = "Relay flags from consensus",
    .fill_fn = fill_relay_flags,
  },
  {
    .key = RELAY_METRICS_NUM_CIRCUITS,
    .type = METRICS_TYPE_GAUGE,
    .name = METRICS_NAME(relay_circuits_total),
    .help = "Total number of circuits",
    .fill_fn = fill_circuits_values,
  },
  {
    .key = RELAY_METRICS_SIGNING_CERT_EXPIRY,
    .type = METRICS_TYPE_GAUGE,
    .name = METRICS_NAME(relay_signing_cert_expiry_timestamp),
    .help = "Timestamp at which the current online keys will expire",
    .fill_fn = fill_signing_cert_expiry,
  },
  {
    .key = RELAY_METRICS_NUM_EST_REND,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_est_rend_total),
    .help = "Total number of EST_REND cells we received",
    .fill_fn = fill_est_rend_cells,
  },
  {
    .key = RELAY_METRICS_NUM_EST_INTRO,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_est_intro_total),
    .help = "Total number of EST_INTRO cells we received",
    .fill_fn = fill_est_intro_cells,
  },
  {
    .key = RELAY_METRICS_NUM_INTRO1_CELLS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_intro1_total),
    .help = "Total number of INTRO1 cells we received",
    .fill_fn = fill_intro1_cells,
  },
  {
    .key = RELAY_METRICS_NUM_REND1_CELLS,
    .type = METRICS_TYPE_COUNTER,
    .name = METRICS_NAME(relay_rend1_total),
    .help = "Total number of REND1 cells we received",
    .fill_fn = fill_rend1_cells,
  },
};
static const size_t num_base_metrics = ARRAY_LENGTH(base_metrics);

/** The only and single store of all the relay metrics. */
static metrics_store_t *the_store;

/** Helper function to convert an handshake type into a string. */
static inline const char *
handshake_type_to_str(const uint16_t type)
{
  switch (type) {
    case ONION_HANDSHAKE_TYPE_TAP:
      return "tap";
    case ONION_HANDSHAKE_TYPE_FAST:
      return "fast";
    case ONION_HANDSHAKE_TYPE_NTOR:
      return "ntor";
    case ONION_HANDSHAKE_TYPE_NTOR_V3:
      return "ntor_v3";
    default:
      // LCOV_EXCL_START
      tor_assert_unreached();
      // LCOV_EXCL_STOP
  }
}

/** Helper function to convert a socket family type into a string. */
static inline const char *
af_to_string(const int af)
{
  switch (af) {
  case AF_INET:
    return "ipv4";
  case AF_INET6:
    return "ipv6";
  case AF_UNIX:
    return "unix";
  default:
    return "<unknown>";
  }
}

/** Fill function for the RELAY_METRICS_NUM_CIRCUITS metric. */
static void
fill_circuits_values(void)
{
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_CIRCUITS];
  metrics_store_entry_t *sentry = metrics_store_add(
      the_store, rentry->type, rentry->name, rentry->help, 0, NULL);

  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "opened"));
  metrics_store_entry_update(sentry,
                             smartlist_len(circuit_get_global_list()));
}

/** Fill function for the RELAY_METRICS_RELAY_FLAGS metric. */
static void
fill_relay_flags(void)
{
  uint8_t is_fast = 0, is_exit = 0, is_authority = 0, is_stable = 0;
  uint8_t is_running = 0, is_v2_dir = 0, is_guard = 0, is_sybil = 0;
  uint8_t is_hs_dir = 0;

  const node_t *me =
    node_get_by_id((const char *) router_get_my_id_digest());
  if (me && me->rs) {
    is_fast = me->rs->is_fast;
    is_exit = me->rs->is_exit;
    is_authority = me->rs->is_authority;
    is_stable = me->rs->is_stable;
    is_running = me->rs->is_flagged_running;
    is_v2_dir = me->rs->is_v2_dir;
    is_guard = me->rs->is_possible_guard;
    is_sybil = me->rs->is_sybil;
    is_hs_dir = me->rs->is_hs_dir;
  }

  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_RELAY_FLAGS];
  metrics_store_entry_t *sentry = metrics_store_add(
      the_store, rentry->type, rentry->name, rentry->help, 0, NULL);

  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "Fast"));
  metrics_store_entry_update(sentry, is_fast);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "Exit"));
  metrics_store_entry_update(sentry, is_exit);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "Authority"));
  metrics_store_entry_update(sentry, is_authority);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "Stable"));
  metrics_store_entry_update(sentry, is_stable);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "HSDir"));
  metrics_store_entry_update(sentry, is_hs_dir);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "Running"));
  metrics_store_entry_update(sentry, is_running);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "V2Dir"));
  metrics_store_entry_update(sentry, is_v2_dir);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "Sybil"));
  metrics_store_entry_update(sentry, is_sybil);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "Guard"));
  metrics_store_entry_update(sentry, is_guard);
}

/** Fill function for the RELAY_METRICS_NUM_TRAFFIC metric. */
static void
fill_traffic_values(void)
{
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_TRAFFIC];
  metrics_store_entry_t *sentry = metrics_store_add(
      the_store, rentry->type, rentry->name, rentry->help, 0, NULL);

  metrics_store_entry_add_label(sentry,
          metrics_format_label("direction", "read"));
  metrics_store_entry_update(sentry, get_bytes_read());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("direction", "written"));
  metrics_store_entry_update(sentry, get_bytes_written());
}

/** Fill function for the RELAY_METRICS_NUM_DOS metric. */
static void
fill_dos_values(void)
{
  const relay_metrics_entry_t *rentry = &base_metrics[RELAY_METRICS_NUM_DOS];
  metrics_store_entry_t *sentry = metrics_store_add(
      the_store, rentry->type, rentry->name, rentry->help, 0, NULL);

  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "circuit_rejected"));
  metrics_store_entry_update(sentry, dos_get_num_cc_rejected());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "circuit_killed_max_cell"));
  metrics_store_entry_update(sentry, stats_n_circ_max_cell_reached);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "circuit_killed_max_cell_outq"));
  metrics_store_entry_update(sentry, stats_n_circ_max_cell_outq_reached);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "marked_address"));
  metrics_store_entry_update(sentry, dos_get_num_cc_marked_addr());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "marked_address_maxq"));
  metrics_store_entry_update(sentry, dos_get_num_cc_marked_addr_maxq());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "conn_rejected"));
  metrics_store_entry_update(sentry, dos_get_num_conn_addr_connect_rejected());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "concurrent_conn_rejected"));
  metrics_store_entry_update(sentry, dos_get_num_conn_addr_rejected());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "single_hop_refused"));
  metrics_store_entry_update(sentry, dos_get_num_single_hop_refused());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", "introduce2_rejected"));
  metrics_store_entry_update(sentry, hs_dos_get_intro2_rejected_count());
}

/** Fill function for the RELAY_METRICS_CC_COUNTERS metric. */
static void
fill_cc_counters_values(void)
{
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_CC_COUNTERS];

  metrics_store_entry_t *sentry = metrics_store_add(
      the_store, rentry->type, rentry->name, rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "starvation"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "rtt_reset"));
  metrics_store_entry_update(sentry, congestion_control_get_num_rtt_reset());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "clock_stalls"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "rtt_skipped"));
  metrics_store_entry_update(sentry,
                             congestion_control_get_num_clock_stalls());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "flow_control"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "xoff_num_sent"));
  metrics_store_entry_update(sentry,
                             cc_stats_flow_num_xoff_sent);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "flow_control"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "xon_num_sent"));
  metrics_store_entry_update(sentry,
                             cc_stats_flow_num_xon_sent);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_limits"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "above_delta"));
  metrics_store_entry_update(sentry, cc_stats_vegas_above_delta);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_limits"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "above_ss_cwnd_max"));
  metrics_store_entry_update(sentry, cc_stats_vegas_above_ss_cwnd_max);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_limits"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "below_ss_inc_floor"));
  metrics_store_entry_update(sentry, cc_stats_vegas_below_ss_inc_floor);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_circuits"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "circs_created"));
  metrics_store_entry_update(sentry, cc_stats_circs_created);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_circuits"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "circs_closed"));
  metrics_store_entry_update(sentry, cc_stats_circs_closed);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_circuits"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "circs_exited_ss"));
  metrics_store_entry_update(sentry, cc_stats_vegas_circ_exited_ss);
}

/** Fill function for the RELAY_METRICS_CC_GAUGES metric. */
static void
fill_cc_gauges_values(void)
{
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_CC_GAUGES];

  metrics_store_entry_t *sentry = metrics_store_add(
      the_store, rentry->type, rentry->name, rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "slow_start_exit"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "cwnd"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_exit_ss_cwnd_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "slow_start_exit"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "bdp"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_exit_ss_bdp_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "slow_start_exit"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "inc"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_exit_ss_inc_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "on_circ_close"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "cwnd"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_circ_close_cwnd_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "on_circ_close"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "ss_cwnd"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_circ_close_ss_cwnd_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "buffers"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "xon_outbuf"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_flow_xon_outbuf_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "buffers"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "xoff_outbuf"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_flow_xoff_outbuf_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_backoff"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "chan_blocked_pct"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_csig_blocked_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_backoff"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "gamma_drop"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_gamma_drop_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_backoff"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "delta_drop"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_delta_drop_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_backoff"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "ss_chan_blocked_pct"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_ss_csig_blocked_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_cwnd_update"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "alpha_pct"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_csig_alpha_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_cwnd_update"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "beta_pct"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_csig_beta_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_cwnd_update"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "delta_pct"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_csig_delta_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_estimates"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "ss_queue"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_ss_queue_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_estimates"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "queue"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_queue_ma));

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", "cc_estimates"));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("action", "bdp"));
  metrics_store_entry_update(sentry,
                             tor_llround(cc_stats_vegas_bdp_ma));
}

/** Helper: Fill in single stream metrics output. */
static void
fill_single_stream_value(metrics_store_entry_t *sentry, uint8_t cmd)
{
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", relay_command_to_string(cmd)));
  metrics_store_entry_update(sentry, rep_hist_get_exit_stream_seen(cmd));
}

/** Fill function for the RELAY_METRICS_NUM_STREAMS metric. */
static void
fill_streams_values(void)
{
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_STREAMS];
  metrics_store_entry_t *sentry = metrics_store_add(
      the_store, rentry->type, rentry->name, rentry->help, 0, NULL);
  fill_single_stream_value(sentry, RELAY_COMMAND_BEGIN);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  fill_single_stream_value(sentry, RELAY_COMMAND_BEGIN_DIR);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  fill_single_stream_value(sentry, RELAY_COMMAND_RESOLVE);
}

/** Helper: Fill in single connection metrics output. */
static void
fill_single_connection_value(metrics_store_entry_t *sentry,
                             unsigned int conn_type,
                             const char* direction,
                             const char* state,
                             int socket_family,
                             uint64_t value)
{
  metrics_store_entry_add_label(sentry,
          metrics_format_label("type", conn_type_to_string(conn_type)));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("direction", direction));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("state", state));
  metrics_store_entry_add_label(sentry,
          metrics_format_label("family", af_to_string(socket_family)));
  metrics_store_entry_update(sentry, value);
}

/** Fill function for the RELAY_METRICS_CONN_COUNTERS metric. */
static void
fill_conn_counter_values(void)
{
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_CONN_COUNTERS];

  for (unsigned int i = CONN_TYPE_MIN_; i < CONN_TYPE_MAX_ ; i++) {
    /* Type is unused. Ugly but else we clobber the output. */
    if (i == 10) {
      continue;
    }
    metrics_store_entry_t *sentry = metrics_store_add(
        the_store, rentry->type, rentry->name, rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "initiated", "created", AF_INET,
                                 rep_hist_get_conn_created(false, i, AF_INET));
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "initiated", "created", AF_INET6,
                                 rep_hist_get_conn_created(false, i,
                                                           AF_INET6));

    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "received", "created", AF_INET,
                                 rep_hist_get_conn_created(true, i, AF_INET));
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "received", "created", AF_INET6,
                                 rep_hist_get_conn_created(true, i, AF_INET6));

    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "received", "rejected", AF_INET,
                                 rep_hist_get_conn_rejected(i, AF_INET));
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "received", "rejected", AF_INET6,
                                 rep_hist_get_conn_rejected(i, AF_INET6));

    /* No counter for "initiated" + "rejected" connections exists. */
  }
}

/** Fill function for the RELAY_METRICS_CONN_GAUGES metric. */
static void
fill_conn_gauge_values(void)
{
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_CONN_GAUGES];

  for (unsigned int i = CONN_TYPE_MIN_; i < CONN_TYPE_MAX_ ; i++) {
    /* Type is unused. Ugly but else we clobber the output. */
    if (i == 10) {
      continue;
    }
    metrics_store_entry_t *sentry = metrics_store_add(
        the_store, rentry->type, rentry->name, rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "initiated", "opened", AF_INET,
                                 rep_hist_get_conn_opened(false, i, AF_INET));
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "initiated", "opened", AF_INET6,
                                 rep_hist_get_conn_opened(false, i, AF_INET6));

    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "received", "opened", AF_INET,
                                 rep_hist_get_conn_opened(true, i, AF_INET));
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    fill_single_connection_value(sentry, i, "received", "opened", AF_INET6,
                                 rep_hist_get_conn_opened(true, i, AF_INET6));
  }
}

/** Fill function for the RELAY_METRICS_NUM_DNS metrics. */
static void
fill_tcp_exhaustion_values(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_TCP_EXHAUSTION];

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_update(sentry, rep_hist_get_n_tcp_exhaustion());
}

/* NOTE: Disable the record type label until libevent is fixed. */
#if 0
/** Helper array containing mapping for the name of the different DNS records
 * and their corresponding libevent values. */
static struct dns_type {
  const char *name;
  uint8_t type;
} dns_types[] = {
  { .name = "A",    .type = DNS_IPv4_A     },
  { .name = "PTR",  .type = DNS_PTR        },
  { .name = "AAAA", .type = DNS_IPv6_AAAA  },
};
static const size_t num_dns_types = ARRAY_LENGTH(dns_types);
#endif

/** Fill function for the RELAY_METRICS_NUM_DNS_ERRORS metrics. */
static void
fill_dns_error_values(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_DNS_ERRORS];

  /* Helper array to map libeven DNS errors to their names and so we can
   * iterate over this array to add all metrics. */
  static struct dns_error {
    const char *name;
    uint8_t key;
  } errors[] = {
    { .name = "success",      .key = DNS_ERR_NONE         },
    { .name = "format",       .key = DNS_ERR_FORMAT       },
    { .name = "serverfailed", .key = DNS_ERR_SERVERFAILED },
    { .name = "notexist",     .key = DNS_ERR_NOTEXIST     },
    { .name = "notimpl",      .key = DNS_ERR_NOTIMPL      },
    { .name = "refused",      .key = DNS_ERR_REFUSED      },
    { .name = "truncated",    .key = DNS_ERR_TRUNCATED    },
    { .name = "unknown",      .key = DNS_ERR_UNKNOWN      },
    { .name = "tor_timeout",  .key = DNS_ERR_TIMEOUT      },
    { .name = "shutdown",     .key = DNS_ERR_SHUTDOWN     },
    { .name = "cancel",       .key = DNS_ERR_CANCEL       },
    { .name = "nodata",       .key = DNS_ERR_NODATA       },
  };
  static const size_t num_errors = ARRAY_LENGTH(errors);

  /* NOTE: Disable the record type label until libevent is fixed. */
#if 0
  for (size_t i = 0; i < num_dns_types; i++) {
    /* Dup the label because metrics_format_label() returns a pointer to a
     * string on the stack and we need that label for all metrics. */
    char *record_label =
      tor_strdup(metrics_format_label("record", dns_types[i].name));

    for (size_t j = 0; j < num_errors; j++) {
      sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                                 rentry->help, 0, NULL);
      metrics_store_entry_add_label(sentry, record_label);
      metrics_store_entry_add_label(sentry,
              metrics_format_label("reason", errors[j].name));
      metrics_store_entry_update(sentry,
              rep_hist_get_n_dns_error(dns_types[i].type, errors[j].key));
    }
    tor_free(record_label);
  }
#endif

  /* Put in the DNS errors, unfortunately not per-type for now. */
  for (size_t j = 0; j < num_errors; j++) {
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(sentry,
            metrics_format_label("reason", errors[j].name));
    metrics_store_entry_update(sentry,
            rep_hist_get_n_dns_error(0, errors[j].key));
  }
}

/** Fill function for the RELAY_METRICS_NUM_DNS metrics. */
static void
fill_dns_query_values(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_DNS];

    /* NOTE: Disable the record type label until libevent is fixed (#40490). */
#if 0
  for (size_t i = 0; i < num_dns_types; i++) {
    /* Dup the label because metrics_format_label() returns a pointer to a
     * string on the stack and we need that label for all metrics. */
    char *record_label =
      tor_strdup(metrics_format_label("record", dns_types[i].name));
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(sentry, record_label);
    metrics_store_entry_update(sentry,
                               rep_hist_get_n_dns_request(dns_types[i].type));
    tor_free(record_label);
  }
#endif

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_update(sentry, rep_hist_get_n_dns_request(0));
}

/** Fill function for the RELAY_METRICS_NUM_GLOBAL_RW_LIMIT metrics. */
static void
fill_global_bw_limit_values(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_GLOBAL_RW_LIMIT];

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
                                metrics_format_label("side", "read"));
  metrics_store_entry_update(sentry, rep_hist_get_n_read_limit_reached());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
                                metrics_format_label("side", "write"));
  metrics_store_entry_update(sentry, rep_hist_get_n_write_limit_reached());
}

/** Fill function for the RELAY_METRICS_NUM_SOCKETS metrics. */
static void
fill_socket_values(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_SOCKETS];

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
                                metrics_format_label("state", "opened"));
  metrics_store_entry_update(sentry, get_n_open_sockets());

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_update(sentry, get_max_sockets());
}

/** Fill function for the RELAY_METRICS_NUM_ONIONSKINS metrics. */
static void
fill_onionskins_values(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_ONIONSKINS];

  for (uint16_t t = 0; t <= MAX_ONION_HANDSHAKE_TYPE; t++) {
    /* Dup the label because metrics_format_label() returns a pointer to a
     * string on the stack and we need that label for all metrics. */
    char *type_label =
      tor_strdup(metrics_format_label("type", handshake_type_to_str(t)));
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(sentry, type_label);
    metrics_store_entry_add_label(sentry,
                        metrics_format_label("action", "processed"));
    metrics_store_entry_update(sentry,
                               rep_hist_get_circuit_n_handshake_assigned(t));

    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(sentry, type_label);
    metrics_store_entry_add_label(sentry,
                        metrics_format_label("action", "dropped"));
    metrics_store_entry_update(sentry,
                               rep_hist_get_circuit_n_handshake_dropped(t));
    tor_free(type_label);
  }
}

/** Fill function for the RELAY_METRICS_NUM_OOM_BYTES metrics. */
static void
fill_oom_values(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_NUM_OOM_BYTES];

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
                                metrics_format_label("subsys", "cell"));
  metrics_store_entry_update(sentry, oom_stats_n_bytes_removed_cell);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
                                metrics_format_label("subsys", "dns"));
  metrics_store_entry_update(sentry, oom_stats_n_bytes_removed_dns);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
                                metrics_format_label("subsys", "geoip"));
  metrics_store_entry_update(sentry, oom_stats_n_bytes_removed_geoip);

  sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                             rentry->help, 0, NULL);
  metrics_store_entry_add_label(sentry,
                                metrics_format_label("subsys", "hsdir"));
  metrics_store_entry_update(sentry, oom_stats_n_bytes_removed_hsdir);
}

/** Fill function for the RELAY_METRICS_SIGNING_CERT_EXPIRY metrics. */
static void
fill_signing_cert_expiry(void)
{
  metrics_store_entry_t *sentry;
  const tor_cert_t *signing_key;
  const relay_metrics_entry_t *rentry =
    &base_metrics[RELAY_METRICS_SIGNING_CERT_EXPIRY];

  if (get_options()->OfflineMasterKey) {
    signing_key = get_master_signing_key_cert();
    if (signing_key) {
      sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                                 rentry->help, 0, NULL);
      metrics_store_entry_update(sentry, signing_key->valid_until);
    }
  }
}

static uint64_t est_intro_actions[EST_INTRO_ACTION_COUNT] = {0};

void
relay_increment_est_intro_action(est_intro_action_t action)
{
  est_intro_actions[action]++;
}

static void
fill_est_intro_cells(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
      &base_metrics[RELAY_METRICS_NUM_EST_INTRO];

  static struct {
    const char *name;
    est_intro_action_t key;
  } actions[] = {
      {.name = "success", .key = EST_INTRO_SUCCESS},
      {.name = "malformed", .key = EST_INTRO_MALFORMED},
      {.name = "unsuitable_circuit", .key = EST_INTRO_UNSUITABLE_CIRCUIT},
      {.name = "circuit_dead", .key = EST_INTRO_CIRCUIT_DEAD},
  };
  static const size_t num_actions = ARRAY_LENGTH(actions);

  for (size_t i = 0; i < num_actions; ++i) {
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(
        sentry, metrics_format_label("action", actions[i].name));
    metrics_store_entry_update(sentry,
                               (long)est_intro_actions[actions[i].key]);
  }
}

static uint64_t est_rend_actions[EST_REND_ACTION_COUNT] = {0};

void
relay_increment_est_rend_action(est_rend_action_t action)
{
  est_rend_actions[action]++;
}

static void
fill_est_rend_cells(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
      &base_metrics[RELAY_METRICS_NUM_EST_REND];

  static struct {
    const char *name;
    est_rend_action_t key;
  } actions[] = {
      {.name = "success", .key = EST_REND_SUCCESS},
      {.name = "unsuitable_circuit", .key = EST_REND_UNSUITABLE_CIRCUIT},
      {.name = "single_hop", .key = EST_REND_SINGLE_HOP},
      {.name = "malformed", .key = EST_REND_MALFORMED},
      {.name = "duplicate_cookie", .key = EST_REND_DUPLICATE_COOKIE},
      {.name = "circuit_dead", .key = EST_REND_CIRCUIT_DEAD},
  };
  static const size_t num_actions = ARRAY_LENGTH(actions);

  for (size_t i = 0; i < num_actions; ++i) {
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(
        sentry, metrics_format_label("action", actions[i].name));
    metrics_store_entry_update(sentry, (long)est_rend_actions[actions[i].key]);
  }
}

static uint64_t intro1_actions[INTRO1_ACTION_COUNT] = {0};

void
relay_increment_intro1_action(intro1_action_t action)
{
  intro1_actions[action]++;
}

static void
fill_intro1_cells(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
      &base_metrics[RELAY_METRICS_NUM_INTRO1_CELLS];

  static struct {
    const char *name;
    intro1_action_t key;
  } actions[] = {
      {.name = "success", .key = INTRO1_SUCCESS},
      {.name = "circuit_dead", .key = INTRO1_CIRCUIT_DEAD},
      {.name = "malformed", .key = INTRO1_MALFORMED},
      {.name = "unknown_service", .key = INTRO1_UNKNOWN_SERVICE},
      {.name = "rate_limited", .key = INTRO1_RATE_LIMITED},
      {.name = "circuit_reused", .key = INTRO1_CIRCUIT_REUSED},
      {.name = "single_hop", .key = INTRO1_SINGLE_HOP},
  };
  static const size_t num_actions = ARRAY_LENGTH(actions);

  for (size_t i = 0; i < num_actions; ++i) {
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(
        sentry, metrics_format_label("action", actions[i].name));
    metrics_store_entry_update(sentry, (long)intro1_actions[actions[i].key]);
  }
}

static uint64_t rend1_actions[REND1_ACTION_COUNT] = {0};

void
relay_increment_rend1_action(rend1_action_t action)
{
  rend1_actions[action]++;
}

static void
fill_rend1_cells(void)
{
  metrics_store_entry_t *sentry;
  const relay_metrics_entry_t *rentry =
      &base_metrics[RELAY_METRICS_NUM_REND1_CELLS];

  static struct {
    const char *name;
    rend1_action_t key;
  } actions[] = {
      {.name = "success", .key = REND1_SUCCESS},
      {.name = "unsuitable_circuit", .key = REND1_UNSUITABLE_CIRCUIT},
      {.name = "malformed", .key = REND1_MALFORMED},
      {.name = "unknown_cookie", .key = REND1_UNKNOWN_COOKIE},
      {.name = "circuit_dead", .key = REND1_CIRCUIT_DEAD},
  };
  static const size_t num_actions = ARRAY_LENGTH(actions);

  for (size_t i = 0; i < num_actions; ++i) {
    sentry = metrics_store_add(the_store, rentry->type, rentry->name,
                               rentry->help, 0, NULL);
    metrics_store_entry_add_label(
        sentry, metrics_format_label("action", actions[i].name));
    metrics_store_entry_update(sentry, (long)rend1_actions[actions[i].key]);
  }
}

/** Reset the global store and fill it with all the metrics from base_metrics
 * and their associated values.
 *
 * To pull this off, every metrics has a "fill" function that is called and in
 * charge of adding the metrics to the store, appropriate labels and finally
 * updating the value to report. */
static void
fill_store(void)
{
  /* Reset the current store, we are about to fill it with all the things. */
  metrics_store_reset(the_store);

  /* Call the fill function for each metrics. */
  for (size_t i = 0; i < num_base_metrics; i++) {
    if (BUG(!base_metrics[i].fill_fn)) {
      continue;
    }
    base_metrics[i].fill_fn();
  }
}

/** Return a list of all the relay metrics stores. This is the
 * function attached to the .get_metrics() member of the subsys_t. */
const smartlist_t *
relay_metrics_get_stores(void)
{
  /* We can't have the caller to free the returned list so keep it static,
   * simply update it. */
  static smartlist_t *stores_list = NULL;

  /* We dynamically fill the store with all the metrics upon a request. The
   * reason for this is because the exposed metrics of a relay are often
   * internal counters in the fast path and thus we fetch the value when a
   * metrics port request arrives instead of keeping a local metrics store of
   * those values. */
  fill_store();

  if (!stores_list) {
    stores_list = smartlist_new();
    smartlist_add(stores_list, the_store);
  }

  return stores_list;
}

/** Initialize the relay metrics. */
void
relay_metrics_init(void)
{
  if (BUG(the_store)) {
    return;
  }
  the_store = metrics_store_new();
}

/** Free the relay metrics. */
void
relay_metrics_free(void)
{
  if (!the_store) {
    return;
  }
  /* NULL is set with this call. */
  metrics_store_free(the_store);
}
