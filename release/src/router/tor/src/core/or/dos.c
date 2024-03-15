/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/*
 * \file dos.c
 * \brief Implement Denial of Service mitigation subsystem.
 */

#define DOS_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/channel.h"
#include "core/or/connection_or.h"
#include "core/or/relay.h"
#include "feature/hs/hs_dos.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/relay/routermode.h"
#include "feature/stats/geoip_stats.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/time/compat_time.h"

#include "core/or/dos.h"
#include "core/or/dos_sys.h"

#include "core/or/dos_options_st.h"
#include "core/or/or_connection_st.h"

/*
 * Circuit creation denial of service mitigation.
 *
 * Namespace used for this mitigation framework is "dos_cc_" where "cc" is for
 * Circuit Creation.
 */

/* Is the circuit creation DoS mitigation enabled? */
static unsigned int dos_cc_enabled = 0;

/* Consensus parameters. They can be changed when a new consensus arrives.
 * They are initialized with the hardcoded default values. */
static uint32_t dos_cc_min_concurrent_conn;
static uint32_t dos_cc_circuit_rate;
static uint32_t dos_cc_circuit_burst;
static dos_cc_defense_type_t dos_cc_defense_type;
static int32_t dos_cc_defense_time_period;

/* Keep some stats for the heartbeat so we can report out. */
static uint64_t cc_num_rejected_cells;
static uint32_t cc_num_marked_addrs;
static uint32_t cc_num_marked_addrs_max_queue;

/*
 * Concurrent connection denial of service mitigation.
 *
 * Namespace used for this mitigation framework is "dos_conn_".
 */

/* Is the connection DoS mitigation enabled? */
static unsigned int dos_conn_enabled = 0;

/* Consensus parameters. They can be changed when a new consensus arrives.
 * They are initialized with the hardcoded default values. */
static uint32_t dos_conn_max_concurrent_count;
static dos_conn_defense_type_t dos_conn_defense_type;
static uint32_t dos_conn_connect_rate = DOS_CONN_CONNECT_RATE_DEFAULT;
static uint32_t dos_conn_connect_burst = DOS_CONN_CONNECT_BURST_DEFAULT;
static int32_t dos_conn_connect_defense_time_period =
  DOS_CONN_CONNECT_DEFENSE_TIME_PERIOD_DEFAULT;

/* Keep some stats for the heartbeat so we can report out. */
static uint64_t conn_num_addr_rejected;
static uint64_t conn_num_addr_connect_rejected;

/** Consensus parameter: How many times a client IP is allowed to hit the
 * circ_max_cell_queue_size_out limit before being marked. */
static uint32_t dos_num_circ_max_outq;

/*
 * General interface of the denial of service mitigation subsystem.
 */

/* Keep stats for the heartbeat. */
static uint64_t num_single_hop_client_refused;

/** Return the consensus parameter for the outbound circ_max_cell_queue_size
 * limit. */
static uint32_t
get_param_dos_num_circ_max_outq(const networkstatus_t *ns)
{
#define DOS_NUM_CIRC_MAX_OUTQ_DEFAULT 3
#define DOS_NUM_CIRC_MAX_OUTQ_MIN 0
#define DOS_NUM_CIRC_MAX_OUTQ_MAX INT32_MAX

  /* Update the circuit max cell queue size from the consensus. */
  return networkstatus_get_param(ns, "dos_num_circ_max_outq",
                                 DOS_NUM_CIRC_MAX_OUTQ_DEFAULT,
                                 DOS_NUM_CIRC_MAX_OUTQ_MIN,
                                 DOS_NUM_CIRC_MAX_OUTQ_MAX);
}

/* Return true iff the circuit creation mitigation is enabled. We look at the
 * consensus for this else a default value is returned. */
MOCK_IMPL(STATIC unsigned int,
get_param_cc_enabled, (const networkstatus_t *ns))
{
  if (dos_get_options()->DoSCircuitCreationEnabled != -1) {
    return dos_get_options()->DoSCircuitCreationEnabled;
  }

  return !!networkstatus_get_param(ns, "DoSCircuitCreationEnabled",
                                   DOS_CC_ENABLED_DEFAULT, 0, 1);
}

/* Return the parameter for the minimum concurrent connection at which we'll
 * start counting circuit for a specific client address. */
STATIC uint32_t
get_param_cc_min_concurrent_connection(const networkstatus_t *ns)
{
  if (dos_get_options()->DoSCircuitCreationMinConnections) {
    return dos_get_options()->DoSCircuitCreationMinConnections;
  }
  return networkstatus_get_param(ns, "DoSCircuitCreationMinConnections",
                                 DOS_CC_MIN_CONCURRENT_CONN_DEFAULT,
                                 1, INT32_MAX);
}

/* Return the parameter for the time rate that is how many circuits over this
 * time span. */
static uint32_t
get_param_cc_circuit_rate(const networkstatus_t *ns)
{
  /* This is in seconds. */
  if (dos_get_options()->DoSCircuitCreationRate) {
    return dos_get_options()->DoSCircuitCreationRate;
  }
  return networkstatus_get_param(ns, "DoSCircuitCreationRate",
                                 DOS_CC_CIRCUIT_RATE_DEFAULT,
                                 1, INT32_MAX);
}

/* Return the parameter for the maximum circuit count for the circuit time
 * rate. */
STATIC uint32_t
get_param_cc_circuit_burst(const networkstatus_t *ns)
{
  if (dos_get_options()->DoSCircuitCreationBurst) {
    return dos_get_options()->DoSCircuitCreationBurst;
  }
  return networkstatus_get_param(ns, "DoSCircuitCreationBurst",
                                 DOS_CC_CIRCUIT_BURST_DEFAULT,
                                 1, INT32_MAX);
}

/* Return the consensus parameter of the circuit creation defense type. */
static uint32_t
get_param_cc_defense_type(const networkstatus_t *ns)
{
  if (dos_get_options()->DoSCircuitCreationDefenseType) {
    return dos_get_options()->DoSCircuitCreationDefenseType;
  }
  return networkstatus_get_param(ns, "DoSCircuitCreationDefenseType",
                                 DOS_CC_DEFENSE_TYPE_DEFAULT,
                                 DOS_CC_DEFENSE_NONE, DOS_CC_DEFENSE_MAX);
}

/* Return the consensus parameter of the defense time period which is how much
 * time should we defend against a malicious client address. */
static int32_t
get_param_cc_defense_time_period(const networkstatus_t *ns)
{
  /* Time in seconds. */
  if (dos_get_options()->DoSCircuitCreationDefenseTimePeriod) {
    return dos_get_options()->DoSCircuitCreationDefenseTimePeriod;
  }
  return networkstatus_get_param(ns, "DoSCircuitCreationDefenseTimePeriod",
                                 DOS_CC_DEFENSE_TIME_PERIOD_DEFAULT,
                                 0, INT32_MAX);
}

/* Return true iff connection mitigation is enabled. We look at the consensus
 * for this else a default value is returned. */
MOCK_IMPL(STATIC unsigned int,
get_param_conn_enabled, (const networkstatus_t *ns))
{
  if (dos_get_options()->DoSConnectionEnabled != -1) {
    return dos_get_options()->DoSConnectionEnabled;
  }
  return !!networkstatus_get_param(ns, "DoSConnectionEnabled",
                                   DOS_CONN_ENABLED_DEFAULT, 0, 1);
}

/* Return the consensus parameter for the maximum concurrent connection
 * allowed. */
STATIC uint32_t
get_param_conn_max_concurrent_count(const networkstatus_t *ns)
{
  if (dos_get_options()->DoSConnectionMaxConcurrentCount) {
    return dos_get_options()->DoSConnectionMaxConcurrentCount;
  }
  return networkstatus_get_param(ns, "DoSConnectionMaxConcurrentCount",
                                 DOS_CONN_MAX_CONCURRENT_COUNT_DEFAULT,
                                 1, INT32_MAX);
}

/* Return the consensus parameter of the connection defense type. */
static uint32_t
get_param_conn_defense_type(const networkstatus_t *ns)
{
  if (dos_get_options()->DoSConnectionDefenseType) {
    return dos_get_options()->DoSConnectionDefenseType;
  }
  return networkstatus_get_param(ns, "DoSConnectionDefenseType",
                                 DOS_CONN_DEFENSE_TYPE_DEFAULT,
                                 DOS_CONN_DEFENSE_NONE, DOS_CONN_DEFENSE_MAX);
}

/* Return the connection connect rate parameters either from the configuration
 * file or, if not found, consensus parameter. */
static uint32_t
get_param_conn_connect_rate(const networkstatus_t *ns)
{
  if (dos_get_options()->DoSConnectionConnectRate) {
    return dos_get_options()->DoSConnectionConnectRate;
  }
  return networkstatus_get_param(ns, "DoSConnectionConnectRate",
                                 DOS_CONN_CONNECT_RATE_DEFAULT,
                                 1, INT32_MAX);
}

/* Return the connection connect burst parameters either from the
 * configuration file or, if not found, consensus parameter. */
STATIC uint32_t
get_param_conn_connect_burst(const networkstatus_t *ns)
{
  if (dos_get_options()->DoSConnectionConnectBurst) {
    return dos_get_options()->DoSConnectionConnectBurst;
  }
  return networkstatus_get_param(ns, "DoSConnectionConnectBurst",
                                 DOS_CONN_CONNECT_BURST_DEFAULT,
                                 1, INT32_MAX);
}

/* Return the connection connect defense time period from the configuration
 * file or, if not found, the consensus parameter. */
static int32_t
get_param_conn_connect_defense_time_period(const networkstatus_t *ns)
{
  /* Time in seconds. */
  if (dos_get_options()->DoSConnectionConnectDefenseTimePeriod) {
    return dos_get_options()->DoSConnectionConnectDefenseTimePeriod;
  }
  return networkstatus_get_param(ns, "DoSConnectionConnectDefenseTimePeriod",
                                 DOS_CONN_CONNECT_DEFENSE_TIME_PERIOD_DEFAULT,
                                 DOS_CONN_CONNECT_DEFENSE_TIME_PERIOD_MIN,
                                 INT32_MAX);
}

/* Set circuit creation parameters located in the consensus or their default
 * if none are present. Called at initialization or when the consensus
 * changes. */
static void
set_dos_parameters(const networkstatus_t *ns)
{
  /* Get the default consensus param values. */
  dos_cc_enabled = get_param_cc_enabled(ns);
  dos_cc_min_concurrent_conn = get_param_cc_min_concurrent_connection(ns);
  dos_cc_circuit_rate = get_param_cc_circuit_rate(ns);
  dos_cc_circuit_burst = get_param_cc_circuit_burst(ns);
  dos_cc_defense_time_period = get_param_cc_defense_time_period(ns);
  dos_cc_defense_type = get_param_cc_defense_type(ns);

  /* Connection detection. */
  dos_conn_enabled = get_param_conn_enabled(ns);
  dos_conn_max_concurrent_count = get_param_conn_max_concurrent_count(ns);
  dos_conn_defense_type = get_param_conn_defense_type(ns);
  dos_conn_connect_rate = get_param_conn_connect_rate(ns);
  dos_conn_connect_burst = get_param_conn_connect_burst(ns);
  dos_conn_connect_defense_time_period =
    get_param_conn_connect_defense_time_period(ns);

  /* Circuit. */
  dos_num_circ_max_outq = get_param_dos_num_circ_max_outq(ns);
}

/* Free everything for the circuit creation DoS mitigation subsystem. */
static void
cc_free_all(void)
{
  /* If everything is freed, the circuit creation subsystem is not enabled. */
  dos_cc_enabled = 0;
}

/* Called when the consensus has changed. Do appropriate actions for the
 * circuit creation subsystem. */
static void
cc_consensus_has_changed(const networkstatus_t *ns)
{
  /* Looking at the consensus, is the circuit creation subsystem enabled? If
   * not and it was enabled before, clean it up. */
  if (dos_cc_enabled && !get_param_cc_enabled(ns)) {
    cc_free_all();
  }
}

/** Return the number of circuits we allow per second under the current
 *  configuration. */
STATIC uint64_t
get_circuit_rate_per_second(void)
{
  return dos_cc_circuit_rate;
}

/* Given the circuit creation client statistics object, refill the circuit
 * bucket if needed. This also works if the bucket was never filled in the
 * first place. The addr is only used for logging purposes. */
STATIC void
cc_stats_refill_bucket(cc_client_stats_t *stats, const tor_addr_t *addr)
{
  uint32_t new_circuit_bucket_count;
  uint64_t num_token, elapsed_time_last_refill = 0, circuit_rate = 0;
  time_t now;
  int64_t last_refill_ts;

  tor_assert(stats);
  tor_assert(addr);

  now = approx_time();
  last_refill_ts = (int64_t)stats->last_circ_bucket_refill_ts;

  /* If less than a second has elapsed, don't add any tokens.
   * Note: If a relay's clock is ever 0, any new clients won't get a refill
   * until the next second. But a relay that thinks it is 1970 will never
   * validate the public consensus. */
  if ((int64_t)now == last_refill_ts) {
    goto done;
  }

  /* At this point, we know we might need to add token to the bucket. We'll
   * first get the circuit rate that is how many circuit are we allowed to do
   * per second. */
  circuit_rate = get_circuit_rate_per_second();

  /* We've never filled the bucket so fill it with the maximum being the burst
   * and we are done.
   * Note: If a relay's clock is ever 0, all clients that were last refilled
   * in that zero second will get a full refill here. */
  if (last_refill_ts == 0) {
    num_token = dos_cc_circuit_burst;
    goto end;
  }

  /* Our clock jumped backward so fill it up to the maximum. Not filling it
   * could trigger a detection for a valid client. Also, if the clock jumped
   * negative but we didn't notice until the elapsed time became positive
   * again, then we potentially spent many seconds not refilling the bucket
   * when we should have been refilling it. But the fact that we didn't notice
   * until now means that no circuit creation requests came in during that
   * time, so the client doesn't end up punished that much from this hopefully
   * rare situation.*/
  if ((int64_t)now < last_refill_ts) {
    /* Use the maximum allowed value of token. */
    num_token = dos_cc_circuit_burst;
    goto end;
  }

  /* How many seconds have elapsed between now and the last refill?
   * This subtraction can't underflow, because now >= last_refill_ts.
   * And it can't overflow, because INT64_MAX - (-INT64_MIN) == UINT64_MAX. */
  elapsed_time_last_refill = (uint64_t)now - last_refill_ts;

  /* If the elapsed time is very large, it means our clock jumped forward.
   * If the multiplication would overflow, use the maximum allowed value. */
  if (elapsed_time_last_refill > UINT32_MAX) {
    num_token = dos_cc_circuit_burst;
    goto end;
  }

  /* Compute how many circuits we are allowed in that time frame which we'll
   * add to the bucket. This can't overflow, because both multiplicands
   * are less than or equal to UINT32_MAX, and num_token is uint64_t. */
  num_token = elapsed_time_last_refill * circuit_rate;

 end:
  /* If the sum would overflow, use the maximum allowed value. */
  if (num_token > UINT32_MAX - stats->circuit_bucket) {
    new_circuit_bucket_count = dos_cc_circuit_burst;
  } else {
    /* We cap the bucket to the burst value else this could overflow uint32_t
     * over time. */
    new_circuit_bucket_count = MIN(stats->circuit_bucket + (uint32_t)num_token,
                                   dos_cc_circuit_burst);
  }

  /* This function is not allowed to make the bucket count larger than the
   * burst value */
  tor_assert_nonfatal(new_circuit_bucket_count <= dos_cc_circuit_burst);
  /* This function is not allowed to make the bucket count smaller, unless it
   * is decreasing it to a newly configured, lower burst value. We allow the
   * bucket to stay the same size, in case the circuit rate is zero. */
  tor_assert_nonfatal(new_circuit_bucket_count >= stats->circuit_bucket ||
                      new_circuit_bucket_count == dos_cc_circuit_burst);

  log_debug(LD_DOS, "DoS address %s has its circuit bucket value: %" PRIu32
                    ". Filling it to %" PRIu32 ". Circuit rate is %" PRIu64
                    ". Elapsed time is %" PRIi64,
            fmt_addr(addr), stats->circuit_bucket, new_circuit_bucket_count,
            circuit_rate, (int64_t)elapsed_time_last_refill);

  stats->circuit_bucket = new_circuit_bucket_count;
  stats->last_circ_bucket_refill_ts = now;

 done:
  return;
}

/* Return true iff the circuit bucket is down to 0 and the number of
 * concurrent connections is greater or equal the minimum threshold set the
 * consensus parameter. */
static int
cc_has_exhausted_circuits(const dos_client_stats_t *stats)
{
  tor_assert(stats);
  return stats->cc_stats.circuit_bucket == 0 &&
         stats->conn_stats.concurrent_count >= dos_cc_min_concurrent_conn;
}

/* Mark client address by setting a timestamp in the stats object which tells
 * us until when it is marked as positively detected. */
static void
cc_mark_client(cc_client_stats_t *stats)
{
  tor_assert(stats);
  /* We add a random offset of a maximum of half the defense time so it is
   * less predictable. */
  stats->marked_until_ts =
    approx_time() + dos_cc_defense_time_period +
    crypto_rand_int_range(1, dos_cc_defense_time_period / 2);
}

/* Return true iff the given channel address is marked as malicious. This is
 * called a lot and part of the fast path of handling cells. It has to remain
 * as fast as we can. */
static int
cc_channel_addr_is_marked(channel_t *chan)
{
  time_t now;
  tor_addr_t addr;
  clientmap_entry_t *entry;
  cc_client_stats_t *stats = NULL;

  if (chan == NULL) {
    goto end;
  }
  /* Must be a client connection else we ignore. */
  if (!channel_is_client(chan)) {
    goto end;
  }
  /* Without an IP address, nothing can work. */
  if (!channel_get_addr_if_possible(chan, &addr)) {
    goto end;
  }

  /* We are only interested in client connection from the geoip cache. */
  entry = geoip_lookup_client(&addr, NULL, GEOIP_CLIENT_CONNECT);
  if (entry == NULL) {
    /* We can have a connection creating circuits but not tracked by the geoip
     * cache. Once this DoS subsystem is enabled, we can end up here with no
     * entry for the channel. */
    goto end;
  }
  now = approx_time();
  stats = &entry->dos_stats.cc_stats;

 end:
  return stats && stats->marked_until_ts >= now;
}

/* Concurrent connection private API. */

/* Mark client connection stats by setting a timestamp which tells us until
 * when it is marked as positively detected. */
static void
conn_mark_client(conn_client_stats_t *stats)
{
  tor_assert(stats);

  /* We add a random offset of a maximum of half the defense time so it is
   * less predictable and thus more difficult to game. */
  stats->marked_until_ts =
    approx_time() + dos_conn_connect_defense_time_period +
    crypto_rand_int_range(1, dos_conn_connect_defense_time_period / 2);
}

/* Free everything for the connection DoS mitigation subsystem. */
static void
conn_free_all(void)
{
  dos_conn_enabled = 0;
}

/* Called when the consensus has changed. Do appropriate actions for the
 * connection mitigation subsystem. */
static void
conn_consensus_has_changed(const networkstatus_t *ns)
{
  /* Looking at the consensus, is the connection mitigation subsystem enabled?
   * If not and it was enabled before, clean it up. */
  if (dos_conn_enabled && !get_param_conn_enabled(ns)) {
    conn_free_all();
  }
}

/** Called when a new client connection has arrived. The following will update
 * the client connection statistics.
 *
 * The addr is used for logging purposes only.
 *
 * If the connect counter reaches its limit, it is marked. */
static void
conn_update_on_connect(conn_client_stats_t *stats, const tor_addr_t *addr)
{
  tor_assert(stats);
  tor_assert(addr);

  /* Update concurrent count for this new connect. */
  stats->concurrent_count++;

  /* Refill connect connection count. */
  token_bucket_ctr_refill(&stats->connect_count,
                          (uint32_t) monotime_coarse_absolute_sec());

  /* Decrement counter for this new connection. */
  if (token_bucket_ctr_get(&stats->connect_count) > 0) {
    token_bucket_ctr_dec(&stats->connect_count, 1);
  }

  /* Assess connect counter. Mark it if counter is down to 0 and we haven't
   * marked it before or it was reset. This is to avoid to re-mark it over and
   * over again extending continuously the blocked time. */
  if (token_bucket_ctr_get(&stats->connect_count) == 0 &&
      stats->marked_until_ts == 0) {
    conn_mark_client(stats);
  }

  log_debug(LD_DOS, "Client address %s has now %u concurrent connections. "
                    "Remaining %" TOR_PRIuSZ "/sec connections are allowed.",
            fmt_addr(addr), stats->concurrent_count,
            token_bucket_ctr_get(&stats->connect_count));
}

/** Called when a client connection is closed. The following will update
 * the client connection statistics.
 *
 * The addr is used for logging purposes only. */
static void
conn_update_on_close(conn_client_stats_t *stats, const tor_addr_t *addr)
{
  /* Extra super duper safety. Going below 0 means an underflow which could
   * lead to most likely a false positive. In theory, this should never happen
   * but let's be extra safe. */
  if (BUG(stats->concurrent_count == 0)) {
    return;
  }

  stats->concurrent_count--;
  log_debug(LD_DOS, "Client address %s has lost a connection. Concurrent "
                    "connections are now at %u",
            fmt_addr(addr), stats->concurrent_count);
}

/* General private API */

/* Return true iff we have at least one DoS detection enabled. This is used to
 * decide if we need to allocate any kind of high level DoS object. */
static inline int
dos_is_enabled(void)
{
  return (dos_cc_enabled || dos_conn_enabled);
}

/* Circuit creation public API. */

/** Return the number of rejected circuits. */
uint64_t
dos_get_num_cc_rejected(void)
{
  return cc_num_rejected_cells;
}

/** Return the number of marked addresses. */
uint32_t
dos_get_num_cc_marked_addr(void)
{
  return cc_num_marked_addrs;
}

/** Return the number of marked addresses due to max queue limit reached. */
uint32_t
dos_get_num_cc_marked_addr_maxq(void)
{
  return cc_num_marked_addrs_max_queue;
}

/** Return number of concurrent connections rejected. */
uint64_t
dos_get_num_conn_addr_rejected(void)
{
  return conn_num_addr_rejected;
}

/** Return the number of connection rejected. */
uint64_t
dos_get_num_conn_addr_connect_rejected(void)
{
  return conn_num_addr_connect_rejected;
}

/** Return the number of single hop refused. */
uint64_t
dos_get_num_single_hop_refused(void)
{
  return num_single_hop_client_refused;
}

/* Called when a CREATE cell is received from the given channel. */
void
dos_cc_new_create_cell(channel_t *chan)
{
  tor_addr_t addr;
  clientmap_entry_t *entry;

  tor_assert(chan);

  /* Skip everything if not enabled. */
  if (!dos_cc_enabled) {
    goto end;
  }

  /* Must be a client connection else we ignore. */
  if (!channel_is_client(chan)) {
    goto end;
  }
  /* Without an IP address, nothing can work. */
  if (!channel_get_addr_if_possible(chan, &addr)) {
    goto end;
  }

  /* We are only interested in client connection from the geoip cache. */
  entry = geoip_lookup_client(&addr, NULL, GEOIP_CLIENT_CONNECT);
  if (entry == NULL) {
    /* We can have a connection creating circuits but not tracked by the geoip
     * cache. Once this DoS subsystem is enabled, we can end up here with no
     * entry for the channel. */
    goto end;
  }

  /* General comment. Even though the client can already be marked as
   * malicious, we continue to track statistics. If it keeps going above
   * threshold while marked, the defense period time will grow longer. There
   * is really no point at unmarking a client that keeps DoSing us. */

  /* First of all, we'll try to refill the circuit bucket opportunistically
   * before we assess. */
  cc_stats_refill_bucket(&entry->dos_stats.cc_stats, &addr);

  /* Take a token out of the circuit bucket if we are above 0 so we don't
   * underflow the bucket. */
  if (entry->dos_stats.cc_stats.circuit_bucket > 0) {
    entry->dos_stats.cc_stats.circuit_bucket--;
  }

  /* This is the detection. Assess at every CREATE cell if the client should
   * get marked as malicious. This should be kept as fast as possible. */
  if (cc_has_exhausted_circuits(&entry->dos_stats)) {
    /* If this is the first time we mark this entry, log it.
     * Under heavy DDoS, logging each time we mark would results in lots and
     * lots of logs. */
    if (entry->dos_stats.cc_stats.marked_until_ts == 0) {
      log_debug(LD_DOS, "Detected circuit creation DoS by address: %s",
                fmt_addr(&addr));
      cc_num_marked_addrs++;
    }
    cc_mark_client(&entry->dos_stats.cc_stats);
  }

 end:
  return;
}

/* Return the defense type that should be used for this circuit.
 *
 * This is part of the fast path and called a lot. */
dos_cc_defense_type_t
dos_cc_get_defense_type(channel_t *chan)
{
  tor_assert(chan);

  /* Skip everything if not enabled. */
  if (!dos_cc_enabled) {
    goto end;
  }

  /* On an OR circuit, we'll check if the previous channel is a marked client
   * connection detected by our DoS circuit creation mitigation subsystem. */
  if (cc_channel_addr_is_marked(chan)) {
    /* We've just assess that this circuit should trigger a defense for the
     * cell it just seen. Note it down. */
    cc_num_rejected_cells++;
    return dos_cc_defense_type;
  }

 end:
  return DOS_CC_DEFENSE_NONE;
}

/* Concurrent connection detection public API. */

/* Return true iff the given address is permitted to open another connection.
 * A defense value is returned for the caller to take appropriate actions. */
dos_conn_defense_type_t
dos_conn_addr_get_defense_type(const tor_addr_t *addr)
{
  clientmap_entry_t *entry;

  tor_assert(addr);

  /* Skip everything if not enabled. */
  if (!dos_conn_enabled) {
    goto end;
  }

  /* We are only interested in client connection from the geoip cache. */
  entry = geoip_lookup_client(addr, NULL, GEOIP_CLIENT_CONNECT);
  if (entry == NULL) {
    goto end;
  }

  /* Is this address marked as making too many client connections? */
  if (entry->dos_stats.conn_stats.marked_until_ts >= approx_time()) {
    conn_num_addr_connect_rejected++;
    return dos_conn_defense_type;
  }
  /* Reset it to 0 here so that if the marked timestamp has expired that is
   * we've gone beyond it, we have to reset it so the detection can mark it
   * again in the future. */
  entry->dos_stats.conn_stats.marked_until_ts = 0;

  /* Need to be above the maximum concurrent connection count to trigger a
   * defense. */
  if (entry->dos_stats.conn_stats.concurrent_count >
      dos_conn_max_concurrent_count) {
    conn_num_addr_rejected++;
    return dos_conn_defense_type;
  }

 end:
  return DOS_CONN_DEFENSE_NONE;
}

/* General API */

/* Take any appropriate actions for the given geoip entry that is about to get
 * freed. This is called for every entry that is being freed.
 *
 * This function will clear out the connection tracked flag if the concurrent
 * count of the entry is above 0 so if those connections end up being seen by
 * this subsystem, we won't try to decrement the counter for a new geoip entry
 * that might have been added after this call for the same address. */
void
dos_geoip_entry_about_to_free(const clientmap_entry_t *geoip_ent)
{
  tor_assert(geoip_ent);

  /* The count is down to 0 meaning no connections right now, we can safely
   * clear the geoip entry from the cache. */
  if (geoip_ent->dos_stats.conn_stats.concurrent_count == 0) {
    goto end;
  }

  /* For each connection matching the geoip entry address, we'll clear the
   * tracked flag because the entry is about to get removed from the geoip
   * cache. We do not try to decrement if the flag is not set. */
  SMARTLIST_FOREACH_BEGIN(get_connection_array(), connection_t *, conn) {
    if (conn->type == CONN_TYPE_OR) {
      or_connection_t *or_conn = TO_OR_CONN(conn);
      if (!tor_addr_compare(&geoip_ent->addr, &TO_CONN(or_conn)->addr,
                            CMP_EXACT)) {
        or_conn->tracked_for_dos_mitigation = 0;
      }
    }
  } SMARTLIST_FOREACH_END(conn);

 end:
  return;
}

/** A new geoip client entry has been allocated, initialize its DoS object. */
void
dos_geoip_entry_init(clientmap_entry_t *geoip_ent)
{
  tor_assert(geoip_ent);

  /* Initialize the connection count counter with the rate and burst
   * parameters taken either from configuration or consensus.
   *
   * We do this even if the DoS connection detection is not enabled because it
   * can be enabled at runtime and these counters need to be valid. */
  token_bucket_ctr_init(&geoip_ent->dos_stats.conn_stats.connect_count,
                        dos_conn_connect_rate, dos_conn_connect_burst,
                        (uint32_t) monotime_coarse_absolute_sec());
}

/** Note that the given channel has sent outbound the maximum amount of cell
 * allowed on the next channel. */
void
dos_note_circ_max_outq(const channel_t *chan)
{
  tor_addr_t addr;
  clientmap_entry_t *entry;

  tor_assert(chan);

  /* Skip everything if circuit creation defense is disabled. */
  if (!dos_cc_enabled) {
    goto end;
  }

  /* Must be a client connection else we ignore. */
  if (!channel_is_client(chan)) {
    goto end;
  }
  /* Without an IP address, nothing can work. */
  if (!channel_get_addr_if_possible(chan, &addr)) {
    goto end;
  }

  /* We are only interested in client connection from the geoip cache. */
  entry = geoip_lookup_client(&addr, NULL, GEOIP_CLIENT_CONNECT);
  if (entry == NULL) {
    goto end;
  }

  /* Is the client marked? If yes, just ignore. */
  if (entry->dos_stats.cc_stats.marked_until_ts >= approx_time()) {
    goto end;
  }

  /* If max outq parameter is 0, it means disabled, just ignore. */
  if (dos_num_circ_max_outq == 0) {
    goto end;
  }

  entry->dos_stats.num_circ_max_cell_queue_size++;

  /* This is the detection. If we have reached the maximum amount of times a
   * client IP is allowed to reach this limit, mark client. */
  if (entry->dos_stats.num_circ_max_cell_queue_size >=
      dos_num_circ_max_outq) {
    /* Only account for this marked address if this is the first time we block
     * it else our counter is inflated with non unique entries. */
    if (entry->dos_stats.cc_stats.marked_until_ts == 0) {
      cc_num_marked_addrs_max_queue++;
    }
    log_info(LD_DOS, "Detected outbound max circuit queue from addr: %s",
             fmt_addr(&addr));
    cc_mark_client(&entry->dos_stats.cc_stats);

    /* Reset after being marked so once unmarked, we start back clean. */
    entry->dos_stats.num_circ_max_cell_queue_size = 0;
  }

 end:
  return;
}

/* Note down that we've just refused a single hop client. This increments a
 * counter later used for the heartbeat. */
void
dos_note_refuse_single_hop_client(void)
{
  num_single_hop_client_refused++;
}

/* Return true iff single hop client connection (ESTABLISH_RENDEZVOUS) should
 * be refused. */
int
dos_should_refuse_single_hop_client(void)
{
  /* If we aren't a public relay, this shouldn't apply to anything. */
  if (!public_server_mode(get_options())) {
    return 0;
  }

  if (dos_get_options()->DoSRefuseSingleHopClientRendezvous != -1) {
    return dos_get_options()->DoSRefuseSingleHopClientRendezvous;
  }

  return (int) networkstatus_get_param(NULL,
                                       "DoSRefuseSingleHopClientRendezvous",
                                       0 /* default */, 0, 1);
}

/* Log a heartbeat message with some statistics. */
void
dos_log_heartbeat(void)
{
  smartlist_t *elems = smartlist_new();

  /* Stats number coming from relay.c append_cell_to_circuit_queue(). */
  smartlist_add_asprintf(elems,
                         "%" PRIu64 " circuits killed with too many cells",
                         stats_n_circ_max_cell_reached);

  if (dos_cc_enabled) {
    smartlist_add_asprintf(elems,
                           "%" PRIu64 " circuits rejected, "
                           "%" PRIu32 " marked addresses, "
                           "%" PRIu32 " marked addresses for max queue",
                           cc_num_rejected_cells, cc_num_marked_addrs,
                           cc_num_marked_addrs_max_queue);
  } else {
    smartlist_add_asprintf(elems, "[DoSCircuitCreationEnabled disabled]");
  }

  if (dos_conn_enabled) {
    smartlist_add_asprintf(elems,
                           "%" PRIu64 " same address concurrent "
                           "connections rejected", conn_num_addr_rejected);
    smartlist_add_asprintf(elems,
                           "%" PRIu64 " connections rejected",
                           conn_num_addr_connect_rejected);
  } else {
    smartlist_add_asprintf(elems, "[DoSConnectionEnabled disabled]");
  }

  if (dos_should_refuse_single_hop_client()) {
    smartlist_add_asprintf(elems,
                           "%" PRIu64 " single hop clients refused",
                           num_single_hop_client_refused);
  } else {
    smartlist_add_asprintf(elems,
                           "[DoSRefuseSingleHopClientRendezvous disabled]");
  }

  /* HS DoS stats. */
  smartlist_add_asprintf(elems,
                         "%" PRIu64 " INTRODUCE2 rejected",
                         hs_dos_get_intro2_rejected_count());

  char *msg = smartlist_join_strings(elems, ", ", 0, NULL);

  log_notice(LD_HEARTBEAT,
             "Heartbeat: DoS mitigation since startup: %s.", msg);

  tor_free(msg);
  SMARTLIST_FOREACH(elems, char *, e, tor_free(e));
  smartlist_free(elems);
}

/* Called when a new client connection has been established on the given
 * address. */
void
dos_new_client_conn(or_connection_t *or_conn, const char *transport_name)
{
  clientmap_entry_t *entry;

  tor_assert(or_conn);
  tor_assert_nonfatal(!or_conn->tracked_for_dos_mitigation);

  /* Past that point, we know we have at least one DoS detection subsystem
   * enabled so we'll start allocating stuff. */
  if (!dos_is_enabled()) {
    goto end;
  }

  /* We are only interested in client connection from the geoip cache. */
  entry = geoip_lookup_client(&TO_CONN(or_conn)->addr, transport_name,
                              GEOIP_CLIENT_CONNECT);
  if (BUG(entry == NULL)) {
    /* Should never happen because we note down the address in the geoip
     * cache before this is called. */
    goto end;
  }

  /* Update stats from this new connect. */
  conn_update_on_connect(&entry->dos_stats.conn_stats,
                         &TO_CONN(or_conn)->addr);

  or_conn->tracked_for_dos_mitigation = 1;

 end:
  return;
}

/* Called when a client connection for the given IP address has been closed. */
void
dos_close_client_conn(const or_connection_t *or_conn)
{
  clientmap_entry_t *entry;

  tor_assert(or_conn);

  /* We have to decrement the count on tracked connection only even if the
   * subsystem has been disabled at runtime because it might be re-enabled
   * after and we need to keep a synchronized counter at all time. */
  if (!or_conn->tracked_for_dos_mitigation) {
    goto end;
  }

  /* We are only interested in client connection from the geoip cache. */
  entry = geoip_lookup_client(&TO_CONN(or_conn)->addr, NULL,
                              GEOIP_CLIENT_CONNECT);
  if (entry == NULL) {
    /* This can happen because we can close a connection before the channel
     * got to be noted down in the geoip cache. */
    goto end;
  }

  /* Update stats from this new close. */
  conn_update_on_close(&entry->dos_stats.conn_stats, &TO_CONN(or_conn)->addr);

 end:
  return;
}

/* Called when the consensus has changed. We might have new consensus
 * parameters to look at. */
void
dos_consensus_has_changed(const networkstatus_t *ns)
{
  /* There are two ways to configure this subsystem, one at startup through
   * dos_init() which is called when the options are parsed. And this one
   * through the consensus. We don't want to enable any DoS mitigation if we
   * aren't a public relay. */
  if (!public_server_mode(get_options())) {
    return;
  }

  cc_consensus_has_changed(ns);
  conn_consensus_has_changed(ns);

  /* We were already enabled or we just became enabled but either way, set the
   * consensus parameters for all subsystems. */
  set_dos_parameters(ns);
}

/* Return true iff the DoS mitigation subsystem is enabled. */
int
dos_enabled(void)
{
  return dos_is_enabled();
}

/* Free everything from the Denial of Service subsystem. */
void
dos_free_all(void)
{
  /* Free the circuit creation mitigation subsystem. It is safe to do this
   * even if it wasn't initialized. */
  cc_free_all();

  /* Free the connection mitigation subsystem. It is safe to do this even if
   * it wasn't initialized. */
  conn_free_all();
}

/* Initialize the Denial of Service subsystem. */
void
dos_init(void)
{
  /* To initialize, we only need to get the parameters. */
  set_dos_parameters(NULL);
}
