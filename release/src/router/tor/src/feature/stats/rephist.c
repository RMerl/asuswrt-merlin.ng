/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rephist.c
 * \brief Basic history and performance-tracking functionality.
 *
 * Basic history and performance-tracking functionality to remember
 *    which servers have worked in the past, how much bandwidth we've
 *    been using, which ports we tend to want, and so on; further,
 *    exit port statistics, cell statistics, and connection statistics.
 *
 * The history and information tracked in this module could sensibly be
 * divided into several categories:
 *
 * <ul><li>Statistics used by authorities to remember the uptime and
 * stability information about various relays, including "uptime",
 * "weighted fractional uptime" and "mean time between failures".
 *
 * <li>Predicted ports, used by clients to remember how long it's been
 * since they opened an exit connection to each given target
 * port. Clients use this information in order to try to keep circuits
 * open to exit nodes that can connect to the ports that they care
 * about.  (The predicted ports mechanism also handles predicted circuit
 * usage that _isn't_ port-specific, such as resolves, internal circuits,
 * and so on.)
 *
 * <li>Public key operation counters, for tracking how many times we've
 * done each public key operation.  (This is unmaintained and we should
 * remove it.)
 *
 * <li>Exit statistics by port, used by exits to keep track of the
 * number of streams and bytes they've served at each exit port, so they
 * can generate their exit-kibibytes-{read,written} and
 * exit-streams-opened statistics.
 *
 * <li>Circuit stats, used by relays instances to tract circuit
 * queue fullness and delay over time, and generate cell-processed-cells,
 * cell-queued-cells, cell-time-in-queue, and cell-circuits-per-decile
 * statistics.
 *
 * <li>Descriptor serving statistics, used by directory caches to track
 * how many descriptors they've served.
 *
 * <li>Onion handshake statistics, used by relays to count how many
 * TAP and ntor handshakes they've handled.
 *
 * <li>Hidden service statistics, used by relays to count rendezvous
 * traffic and HSDir-stored descriptors.
 *
 * <li>Link protocol statistics, used by relays to count how many times
 * each link protocol has been used.
 *
 * </ul>
 *
 * The entry points for this module are scattered throughout the
 * codebase.  Sending data, receiving data, connecting to a relay,
 * losing a connection to a relay, and so on can all trigger a change in
 * our current stats.  Relays also invoke this module in order to
 * extract their statistics when building routerinfo and extrainfo
 * objects in router.c.
 *
 * TODO: This module should be broken up.
 *
 * (The "rephist" name originally stood for "reputation and history". )
 **/

#define REPHIST_PRIVATE
#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/circuitlist.h"
#include "core/or/connection_or.h"
#include "feature/dirauth/authmode.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/stats/predict_ports.h"
#include "feature/stats/connstats.h"
#include "feature/stats/rephist.h"
#include "lib/container/order.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/math/laplace.h"

#include "feature/nodelist/networkstatus_st.h"
#include "core/or/or_circuit_st.h"

#include <event2/dns.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

/** Total number of bytes currently allocated in fields used by rephist.c. */
uint64_t rephist_total_alloc=0;
/** Number of or_history_t objects currently allocated. */
uint32_t rephist_total_num=0;

/** If the total weighted run count of all runs for a router ever falls
 * below this amount, the router can be treated as having 0 MTBF. */
#define STABILITY_EPSILON   0.0001
/** Value by which to discount all old intervals for MTBF purposes.  This
 * is compounded every STABILITY_INTERVAL. */
#define STABILITY_ALPHA     0.95
/** Interval at which to discount all old intervals for MTBF purposes. */
#define STABILITY_INTERVAL  (12*60*60)
/* (This combination of ALPHA, INTERVAL, and EPSILON makes it so that an
 * interval that just ended counts twice as much as one that ended a week ago,
 * 20X as much as one that ended a month ago, and routers that have had no
 * uptime data for about half a year will get forgotten.) */

/** History of an OR. */
typedef struct or_history_t {
  /** When did we start tracking this OR? */
  time_t since;
  /** When did we most recently note a change to this OR? */
  time_t changed;

  /** The address at which we most recently connected to this OR
   * successfully. */
  tor_addr_t last_reached_addr;

  /** The port at which we most recently connected to this OR successfully */
  uint16_t last_reached_port;

  /* === For MTBF tracking: */
  /** Weighted sum total of all times that this router has been online.
   */
  unsigned long weighted_run_length;
  /** If the router is now online (according to stability-checking rules),
   * when did it come online? */
  time_t start_of_run;
  /** Sum of weights for runs in weighted_run_length. */
  double total_run_weights;
  /* === For fractional uptime tracking: */
  time_t start_of_downtime;
  unsigned long weighted_uptime;
  unsigned long total_weighted_time;
} or_history_t;

/**
 * This structure holds accounting needed to calculate the padding overhead.
 */
typedef struct padding_counts_t {
  /** Total number of cells we have received, including padding */
  uint64_t read_cell_count;
  /** Total number of cells we have sent, including padding */
  uint64_t write_cell_count;
  /** Total number of CELL_PADDING cells we have received */
  uint64_t read_pad_cell_count;
  /** Total number of CELL_PADDING cells we have sent */
  uint64_t write_pad_cell_count;
  /** Total number of read cells on padding-enabled conns */
  uint64_t enabled_read_cell_count;
  /** Total number of sent cells on padding-enabled conns */
  uint64_t enabled_write_cell_count;
  /** Total number of read CELL_PADDING cells on padding-enabled cons */
  uint64_t enabled_read_pad_cell_count;
  /** Total number of sent CELL_PADDING cells on padding-enabled cons */
  uint64_t enabled_write_pad_cell_count;
  /** Total number of RELAY_DROP cells we have received */
  uint64_t read_drop_cell_count;
  /** Total number of RELAY_DROP cells we have sent */
  uint64_t write_drop_cell_count;
  /** The maximum number of padding timers we've seen in 24 hours */
  uint64_t maximum_chanpad_timers;
  /** When did we first copy padding_current into padding_published? */
  char first_published_at[ISO_TIME_LEN+1];
} padding_counts_t;

/** Holds the current values of our padding statistics.
 * It is not published until it is transferred to padding_published. */
static padding_counts_t padding_current;

/** Remains fixed for a 24 hour period, and then is replaced
 * by a redacted copy of padding_current */
static padding_counts_t padding_published;

/** When did we last multiply all routers' weighted_run_length and
 * total_run_weights by STABILITY_ALPHA? */
static time_t stability_last_downrated = 0;

/**  */
static time_t started_tracking_stability = 0;

/** Map from hex OR identity digest to or_history_t. */
static digestmap_t *history_map = NULL;

/** Represents a state of overload stats.
 *
 *  All the timestamps in this structure have already been rounded down to the
 *  nearest hour. */
typedef struct {
  /* When did we last experience a general overload? */
  time_t overload_general_time;

  /* When did we last experience a bandwidth-related overload? */
  time_t overload_ratelimits_time;
  /* How many times have we gone off the our read limits? */
  uint64_t overload_read_count;
  /* How many times have we gone off the our write limits? */
  uint64_t overload_write_count;

  /* When did we last experience a file descriptor exhaustion? */
  time_t overload_fd_exhausted_time;
  /* How many times have we experienced a file descriptor exhaustion? */
  uint64_t overload_fd_exhausted;
} overload_stats_t;

/** Current state of overload stats */
static overload_stats_t overload_stats;

/** Counters to count the number of times we've reached an overload for the
 * global connection read/write limit. Reported on the MetricsPort. */
static uint64_t stats_n_read_limit_reached = 0;
static uint64_t stats_n_write_limit_reached = 0;

/** Total number of times we've reached TCP port exhaustion. */
static uint64_t stats_n_tcp_exhaustion = 0;

/***** DNS statistics *****/

/** Overload DNS statistics. The information in this object is used to assess
 * if, due to DNS errors, we should emit a general overload signal or not.
 *
 * NOTE: This structure is _not_ per DNS query type like the statistics below
 * because of a libevent bug
 * (https://github.com/libevent/libevent/issues/1219), on error, the type is
 * not propagated up back to the user and so we need to keep our own stats for
 * the overload signal. */
typedef struct {
  /** Total number of DNS request seen at an Exit. They might not all end
   * successfully or might even be lost by tor. This counter is incremented
   * right before the DNS request is initiated. */
  uint64_t stats_n_request;

  /** When is the next assessment time of the general overload for DNS errors.
   * Once this time is reached, all stats are reset and this time is set to the
   * next assessment time. */
  time_t next_assessment_time;
} overload_dns_stats_t;

/** Keep track of the DNS requests for the general overload state. */
static overload_dns_stats_t overload_dns_stats;

/** Represents the statistics of DNS queries seen if it is an Exit. */
typedef struct {
  /* Total number of DNS errors found in RFC 1035 (from 0 to 5 code). */
  uint64_t stats_n_error_none;          /* 0 */
  uint64_t stats_n_error_format;        /* 1 */
  uint64_t stats_n_error_serverfailed;  /* 2 */
  uint64_t stats_n_error_notexist;      /* 3 */
  uint64_t stats_n_error_notimpl;       /* 4 */
  uint64_t stats_n_error_refused;       /* 5 */

  /* Total number of DNS errors specific to libevent. */
  uint64_t stats_n_error_truncated; /* 65 */
  uint64_t stats_n_error_unknown;   /* 66 */
  uint64_t stats_n_error_tor_timeout;   /* 67 */
  uint64_t stats_n_error_shutdown;  /* 68 */
  uint64_t stats_n_error_cancel;    /* 69 */
  uint64_t stats_n_error_nodata;    /* 70 */

  /* Total number of DNS request seen at an Exit. They might not all end
   * successfully or might even be lost by tor. This counter is incremented
   * right before the DNS request is initiated. */
  uint64_t stats_n_request;
} dns_stats_t;

/* This is disabled because of the libevent bug where on error we don't get the
 * DNS query type back. Once it is fixed, we can re-enable this. */
#if 0
/** DNS statistics store for each DNS record type for which tor supports only
 * three at the moment: A, PTR and AAAA. */
static dns_stats_t dns_A_stats;
static dns_stats_t dns_PTR_stats;
static dns_stats_t dns_AAAA_stats;
#endif

/** DNS query statistics store. It covers all type of queries. */
static dns_stats_t dns_all_stats;

/** Return the point to the DNS statistics store. Ignore the type for now
 * because of a libevent problem. */
static inline dns_stats_t *
get_dns_stats_by_type(const int type)
{
  (void) type;
  return &dns_all_stats;
}

#if 0
/** From a libevent record type, return a pointer to the corresponding DNS
 * statistics store. NULL is returned if the type is unhandled. */
static inline dns_stats_t *
get_dns_stats_by_type(const int type)
{
  switch (type) {
  case DNS_IPv4_A:
    return &dns_A_stats;
  case DNS_PTR:
    return &dns_PTR_stats;
  case DNS_IPv6_AAAA:
    return &dns_AAAA_stats;
  default:
    return NULL;
  }
}
#endif

/** Return the DNS error count for the given libevent DNS type and error code.
 * The possible types are: DNS_IPv4_A, DNS_PTR, DNS_IPv6_AAAA. */
uint64_t
rep_hist_get_n_dns_error(int type, uint8_t error)
{
  dns_stats_t *dns_stats = get_dns_stats_by_type(type);
  if (BUG(!dns_stats)) {
    return 0;
  }

  switch (error) {
  case DNS_ERR_NONE:
    return dns_stats->stats_n_error_none;
  case DNS_ERR_FORMAT:
    return dns_stats->stats_n_error_format;
  case DNS_ERR_SERVERFAILED:
    return dns_stats->stats_n_error_serverfailed;
  case DNS_ERR_NOTEXIST:
    return dns_stats->stats_n_error_notexist;
  case DNS_ERR_NOTIMPL:
    return dns_stats->stats_n_error_notimpl;
  case DNS_ERR_REFUSED:
    return dns_stats->stats_n_error_refused;
  case DNS_ERR_TRUNCATED:
    return dns_stats->stats_n_error_truncated;
  case DNS_ERR_UNKNOWN:
    return dns_stats->stats_n_error_unknown;
  case DNS_ERR_TIMEOUT:
    return dns_stats->stats_n_error_tor_timeout;
  case DNS_ERR_SHUTDOWN:
    return dns_stats->stats_n_error_shutdown;
  case DNS_ERR_CANCEL:
    return dns_stats->stats_n_error_cancel;
  case DNS_ERR_NODATA:
    return dns_stats->stats_n_error_nodata;
  default:
    /* Unhandled code sent back by libevent. */
    return 0;
  }
}

/** Return the total number of DNS request seen for the given libevent DNS
 * record type. Possible types are: DNS_IPv4_A, DNS_PTR, DNS_IPv6_AAAA. */
uint64_t
rep_hist_get_n_dns_request(int type)
{
  dns_stats_t *dns_stats = get_dns_stats_by_type(type);
  if (BUG(!dns_stats)) {
    return 0;
  }
  return dns_stats->stats_n_request;
}

/** Note a DNS error for the given given libevent DNS record type and error
 * code. Possible types are: DNS_IPv4_A, DNS_PTR, DNS_IPv6_AAAA.
 *
 * NOTE: Libevent is _not_ returning the type in case of an error and so if
 * error is anything but DNS_ERR_NONE, the type is not usable and set to 0.
 *
 * See: https://gitlab.torproject.org/tpo/core/tor/-/issues/40490 */
void
rep_hist_note_dns_error(int type, uint8_t error)
{
  overload_dns_stats.stats_n_request++;

  /* Again, the libevent bug (see function comment), for an error that is
   * anything but DNS_ERR_NONE, the type is always 0 which means that we don't
   * have a DNS stat object for it so this code will do nothing until libevent
   * is fixed. */
  dns_stats_t *dns_stats = get_dns_stats_by_type(type);
  /* Unsupported DNS query type. */
  if (!dns_stats) {
    return;
  }

  switch (error) {
  case DNS_ERR_NONE:
    dns_stats->stats_n_error_none++;
    break;
  case DNS_ERR_FORMAT:
    dns_stats->stats_n_error_format++;
    break;
  case DNS_ERR_SERVERFAILED:
    dns_stats->stats_n_error_serverfailed++;
    break;
  case DNS_ERR_NOTEXIST:
    dns_stats->stats_n_error_notexist++;
    break;
  case DNS_ERR_NOTIMPL:
    dns_stats->stats_n_error_notimpl++;
    break;
  case DNS_ERR_REFUSED:
    dns_stats->stats_n_error_refused++;
    break;
  case DNS_ERR_TRUNCATED:
    dns_stats->stats_n_error_truncated++;
    break;
  case DNS_ERR_UNKNOWN:
    dns_stats->stats_n_error_unknown++;
    break;
  case DNS_ERR_TIMEOUT:
    dns_stats->stats_n_error_tor_timeout++;
    break;
  case DNS_ERR_SHUTDOWN:
    dns_stats->stats_n_error_shutdown++;
    break;
  case DNS_ERR_CANCEL:
    dns_stats->stats_n_error_cancel++;
    break;
  case DNS_ERR_NODATA:
    dns_stats->stats_n_error_nodata++;
    break;
  default:
    /* Unhandled code sent back by libevent. */
    break;
  }
}

/** Note a DNS request for the given given libevent DNS record type. */
void
rep_hist_note_dns_request(int type)
{
  dns_stats_t *dns_stats = get_dns_stats_by_type(type);
  if (BUG(!dns_stats)) {
    return;
  }
  dns_stats->stats_n_request++;
}

/***** END of DNS statistics *****/

/** Return true if this overload happened within the last `n_hours`. */
static bool
overload_happened_recently(time_t overload_time, int n_hours)
{
  /* An overload is relevant if it happened in the last 72 hours */
  if (overload_time > approx_time() - 3600 * n_hours) {
    return true;
  }
  return false;
}

/* The current version of the overload stats version */
#define OVERLOAD_STATS_VERSION 1

/** Return the stats_n_read_limit_reached counter. */
uint64_t
rep_hist_get_n_read_limit_reached(void)
{
  return stats_n_read_limit_reached;
}

/** Return the stats_n_write_limit_reached counter. */
uint64_t
rep_hist_get_n_write_limit_reached(void)
{
  return stats_n_write_limit_reached;
}

/** Returns an allocated string for server descriptor for publising information
 * on whether we are overloaded or not. */
char *
rep_hist_get_overload_general_line(void)
{
  char *result = NULL;
  char tbuf[ISO_TIME_LEN+1];

  /* Encode the general overload */
  if (overload_happened_recently(overload_stats.overload_general_time, 72)) {
    format_iso_time(tbuf, overload_stats.overload_general_time);
    tor_asprintf(&result, "overload-general %d %s\n",
                 OVERLOAD_STATS_VERSION, tbuf);
  }

  return result;
}

/** Returns an allocated string for extra-info documents for publishing
 *  overload statistics. */
char *
rep_hist_get_overload_stats_lines(void)
{
  char *result = NULL;
  smartlist_t *chunks = smartlist_new();
  char tbuf[ISO_TIME_LEN+1];

  /* Add bandwidth-related overloads */
  if (overload_happened_recently(overload_stats.overload_ratelimits_time,24)) {
    const or_options_t *options = get_options();
    format_iso_time(tbuf, overload_stats.overload_ratelimits_time);
    smartlist_add_asprintf(chunks,
                           "overload-ratelimits %d %s %" PRIu64 " %" PRIu64
                           " %" PRIu64 " %" PRIu64 "\n",
                           OVERLOAD_STATS_VERSION, tbuf,
                           options->BandwidthRate, options->BandwidthBurst,
                           overload_stats.overload_read_count,
                           overload_stats.overload_write_count);
  }

  /* Finally file descriptor overloads */
  if (overload_happened_recently(
                              overload_stats.overload_fd_exhausted_time, 72)) {
    format_iso_time(tbuf, overload_stats.overload_fd_exhausted_time);
    smartlist_add_asprintf(chunks, "overload-fd-exhausted %d %s\n",
                           OVERLOAD_STATS_VERSION, tbuf);
  }

  /* Bail early if we had nothing to write */
  if (smartlist_len(chunks) == 0) {
    goto done;
  }

  result = smartlist_join_strings(chunks, "", 0, NULL);

 done:
  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_free(chunks);
  return result;
}

/** Round down the time in `a` to the beginning of the current hour */
#define SET_TO_START_OF_HOUR(a) STMT_BEGIN \
  (a) = approx_time() - (approx_time() % 3600); \
STMT_END

/** Note down an overload event of type `overload`. */
void
rep_hist_note_overload(overload_type_t overload)
{
  static time_t last_read_counted = 0;
  static time_t last_write_counted = 0;

  switch (overload) {
  case OVERLOAD_GENERAL:
    SET_TO_START_OF_HOUR(overload_stats.overload_general_time);
    break;
  case OVERLOAD_READ: {
    stats_n_read_limit_reached++;
    SET_TO_START_OF_HOUR(overload_stats.overload_ratelimits_time);
    if (approx_time() >= last_read_counted + 60) { /* Count once a minute */
        overload_stats.overload_read_count++;
        last_read_counted = approx_time();
    }
    break;
  }
  case OVERLOAD_WRITE: {
    stats_n_write_limit_reached++;
    SET_TO_START_OF_HOUR(overload_stats.overload_ratelimits_time);
    if (approx_time() >= last_write_counted + 60) { /* Count once a minute */
      overload_stats.overload_write_count++;
      last_write_counted = approx_time();
    }
    break;
  }
  case OVERLOAD_FD_EXHAUSTED:
    SET_TO_START_OF_HOUR(overload_stats.overload_fd_exhausted_time);
    overload_stats.overload_fd_exhausted++;
    break;
  }
}

/** Note down that we've reached a TCP port exhaustion. This triggers an
 * overload general event. */
void
rep_hist_note_tcp_exhaustion(void)
{
  stats_n_tcp_exhaustion++;
  rep_hist_note_overload(OVERLOAD_GENERAL);
}

/** Return the total number of TCP exhaustion times we've reached. */
uint64_t
rep_hist_get_n_tcp_exhaustion(void)
{
  return stats_n_tcp_exhaustion;
}

/** Return the or_history_t for the OR with identity digest <b>id</b>,
 * creating it if necessary. */
static or_history_t *
get_or_history(const char* id)
{
  or_history_t *hist;

  if (tor_digest_is_zero(id))
    return NULL;

  hist = digestmap_get(history_map, id);
  if (!hist) {
    hist = tor_malloc_zero(sizeof(or_history_t));
    rephist_total_alloc += sizeof(or_history_t);
    rephist_total_num++;
    hist->since = hist->changed = time(NULL);
    tor_addr_make_unspec(&hist->last_reached_addr);
    digestmap_set(history_map, id, hist);
  }
  return hist;
}

/** Helper: free storage held by a single OR history entry. */
static void
free_or_history(void *_hist)
{
  or_history_t *hist = _hist;
  rephist_total_alloc -= sizeof(or_history_t);
  rephist_total_num--;
  tor_free(hist);
}

/** Initialize the static data structures for tracking history. */
void
rep_hist_init(void)
{
  history_map = digestmap_new();
}

/** We have just decided that this router with identity digest <b>id</b> is
 * reachable, meaning we will give it a "Running" flag for the next while. */
void
rep_hist_note_router_reachable(const char *id, const tor_addr_t *at_addr,
                               const uint16_t at_port, time_t when)
{
  or_history_t *hist = get_or_history(id);
  int was_in_run = 1;
  char tbuf[ISO_TIME_LEN+1];
  int addr_changed, port_changed;

  tor_assert(hist);
  tor_assert((!at_addr && !at_port) || (at_addr && at_port));

  addr_changed = at_addr && !tor_addr_is_null(&hist->last_reached_addr) &&
    tor_addr_compare(at_addr, &hist->last_reached_addr, CMP_EXACT) != 0;
  port_changed = at_port && hist->last_reached_port &&
                 at_port != hist->last_reached_port;

  if (!started_tracking_stability)
    started_tracking_stability = time(NULL);
  if (!hist->start_of_run) {
    hist->start_of_run = when;
    was_in_run = 0;
  }
  if (hist->start_of_downtime) {
    long down_length;

    format_local_iso_time(tbuf, hist->start_of_downtime);
    log_info(LD_HIST, "Router %s is now Running; it had been down since %s.",
             hex_str(id, DIGEST_LEN), tbuf);
    if (was_in_run)
      log_info(LD_HIST, "  (Paradoxically, it was already Running too.)");

    down_length = when - hist->start_of_downtime;
    hist->total_weighted_time += down_length;
    hist->start_of_downtime = 0;
  } else if (addr_changed || port_changed) {
    /* If we're reachable, but the address changed, treat this as some
     * downtime. */
    int penalty = get_options()->TestingTorNetwork ? 240 : 3600;
    networkstatus_t *ns;

    if ((ns = networkstatus_get_latest_consensus())) {
      int fresh_interval = (int)(ns->fresh_until - ns->valid_after);
      int live_interval = (int)(ns->valid_until - ns->valid_after);
      /* on average, a descriptor addr change takes .5 intervals to make it
       * into a consensus, and half a liveness period to make it to
       * clients. */
      penalty = (int)(fresh_interval + live_interval) / 2;
    }
    format_local_iso_time(tbuf, hist->start_of_run);
    log_info(LD_HIST,"Router %s still seems Running, but its address appears "
             "to have changed since the last time it was reachable.  I'm "
             "going to treat it as having been down for %d seconds",
             hex_str(id, DIGEST_LEN), penalty);
    rep_hist_note_router_unreachable(id, when-penalty);
    rep_hist_note_router_reachable(id, NULL, 0, when);
  } else {
    format_local_iso_time(tbuf, hist->start_of_run);
    if (was_in_run)
      log_debug(LD_HIST, "Router %s is still Running; it has been Running "
                "since %s", hex_str(id, DIGEST_LEN), tbuf);
    else
      log_info(LD_HIST,"Router %s is now Running; it was previously untracked",
               hex_str(id, DIGEST_LEN));
  }
  if (at_addr)
    tor_addr_copy(&hist->last_reached_addr, at_addr);
  if (at_port)
    hist->last_reached_port = at_port;
}

/** We have just decided that this router is unreachable, meaning
 * we are taking away its "Running" flag. */
void
rep_hist_note_router_unreachable(const char *id, time_t when)
{
  or_history_t *hist = get_or_history(id);
  char tbuf[ISO_TIME_LEN+1];
  int was_running = 0;
  if (!started_tracking_stability)
    started_tracking_stability = time(NULL);

  tor_assert(hist);
  if (hist->start_of_run) {
    /*XXXX We could treat failed connections differently from failed
     * connect attempts. */
    long run_length = when - hist->start_of_run;
    format_local_iso_time(tbuf, hist->start_of_run);

    hist->total_run_weights += 1.0;
    hist->start_of_run = 0;
    if (run_length < 0) {
      unsigned long penalty = -run_length;
#define SUBTRACT_CLAMPED(var, penalty) \
      do { (var) = (var) < (penalty) ? 0 : (var) - (penalty); } while (0)

      SUBTRACT_CLAMPED(hist->weighted_run_length, penalty);
      SUBTRACT_CLAMPED(hist->weighted_uptime, penalty);
    } else {
      hist->weighted_run_length += run_length;
      hist->weighted_uptime += run_length;
      hist->total_weighted_time += run_length;
    }
    was_running = 1;
    log_info(LD_HIST, "Router %s is now non-Running: it had previously been "
             "Running since %s.  Its total weighted uptime is %lu/%lu.",
             hex_str(id, DIGEST_LEN), tbuf, hist->weighted_uptime,
             hist->total_weighted_time);
  }
  if (!hist->start_of_downtime) {
    hist->start_of_downtime = when;

    if (!was_running)
      log_info(LD_HIST, "Router %s is now non-Running; it was previously "
               "untracked.", hex_str(id, DIGEST_LEN));
  } else {
    if (!was_running) {
      format_local_iso_time(tbuf, hist->start_of_downtime);

      log_info(LD_HIST, "Router %s is still non-Running; it has been "
               "non-Running since %s.", hex_str(id, DIGEST_LEN), tbuf);
    }
  }
}

/** Mark a router with ID <b>id</b> as non-Running, and retroactively declare
 * that it has never been running: give it no stability and no WFU. */
void
rep_hist_make_router_pessimal(const char *id, time_t when)
{
  or_history_t *hist = get_or_history(id);
  tor_assert(hist);

  rep_hist_note_router_unreachable(id, when);

  hist->weighted_run_length = 0;
  hist->weighted_uptime = 0;
}

/** Helper: Discount all old MTBF data, if it is time to do so.  Return
 * the time at which we should next discount MTBF data. */
time_t
rep_hist_downrate_old_runs(time_t now)
{
  digestmap_iter_t *orhist_it;
  const char *digest1;
  or_history_t *hist;
  void *hist_p;
  double alpha = 1.0;

  if (!history_map)
    history_map = digestmap_new();
  if (!stability_last_downrated)
    stability_last_downrated = now;
  if (stability_last_downrated + STABILITY_INTERVAL > now)
    return stability_last_downrated + STABILITY_INTERVAL;

  /* Okay, we should downrate the data.  By how much? */
  while (stability_last_downrated + STABILITY_INTERVAL <= now) {
    stability_last_downrated += STABILITY_INTERVAL;
    alpha *= STABILITY_ALPHA;
  }

  log_info(LD_HIST, "Discounting all old stability info by a factor of %f",
           alpha);

  /* Multiply every w_r_l, t_r_w pair by alpha. */
  for (orhist_it = digestmap_iter_init(history_map);
       !digestmap_iter_done(orhist_it);
       orhist_it = digestmap_iter_next(history_map,orhist_it)) {
    digestmap_iter_get(orhist_it, &digest1, &hist_p);
    hist = hist_p;

    hist->weighted_run_length =
      (unsigned long)(hist->weighted_run_length * alpha);
    hist->total_run_weights *= alpha;

    hist->weighted_uptime = (unsigned long)(hist->weighted_uptime * alpha);
    hist->total_weighted_time = (unsigned long)
      (hist->total_weighted_time * alpha);
  }

  return stability_last_downrated + STABILITY_INTERVAL;
}

/** Helper: Return the weighted MTBF of the router with history <b>hist</b>. */
static double
get_stability(or_history_t *hist, time_t when)
{
  long total = hist->weighted_run_length;
  double total_weights = hist->total_run_weights;

  if (hist->start_of_run) {
    /* We're currently in a run.  Let total and total_weights hold the values
     * they would hold if the current run were to end now. */
    total += (when-hist->start_of_run);
    total_weights += 1.0;
  }
  if (total_weights < STABILITY_EPSILON) {
    /* Round down to zero, and avoid divide-by-zero. */
    return 0.0;
  }

  return total / total_weights;
}

/** Return the total amount of time we've been observing, with each run of
 * time downrated by the appropriate factor. */
static long
get_total_weighted_time(or_history_t *hist, time_t when)
{
  long total = hist->total_weighted_time;
  if (hist->start_of_run) {
    total += (when - hist->start_of_run);
  } else if (hist->start_of_downtime) {
    total += (when - hist->start_of_downtime);
  }
  return total;
}

/** Helper: Return the weighted percent-of-time-online of the router with
 * history <b>hist</b>. */
static double
get_weighted_fractional_uptime(or_history_t *hist, time_t when)
{
  long total = hist->total_weighted_time;
  long up = hist->weighted_uptime;

  if (hist->start_of_run) {
    long run_length = (when - hist->start_of_run);
    up += run_length;
    total += run_length;
  } else if (hist->start_of_downtime) {
    total += (when - hist->start_of_downtime);
  }

  if (!total) {
    /* Avoid calling anybody's uptime infinity (which should be impossible if
     * the code is working), or NaN (which can happen for any router we haven't
     * observed up or down yet). */
    return 0.0;
  }

  return ((double) up) / total;
}

/** Return how long the router whose identity digest is <b>id</b> has
 *  been reachable. Return 0 if the router is unknown or currently deemed
 *  unreachable. */
long
rep_hist_get_uptime(const char *id, time_t when)
{
  or_history_t *hist = get_or_history(id);
  if (!hist)
    return 0;
  if (!hist->start_of_run || when < hist->start_of_run)
    return 0;
  return when - hist->start_of_run;
}

/** Return an estimated MTBF for the router whose identity digest is
 * <b>id</b>. Return 0 if the router is unknown. */
double
rep_hist_get_stability(const char *id, time_t when)
{
  or_history_t *hist = get_or_history(id);
  if (!hist)
    return 0.0;

  return get_stability(hist, when);
}

/** Return an estimated percent-of-time-online for the router whose identity
 * digest is <b>id</b>. Return 0 if the router is unknown. */
double
rep_hist_get_weighted_fractional_uptime(const char *id, time_t when)
{
  or_history_t *hist = get_or_history(id);
  if (!hist)
    return 0.0;

  return get_weighted_fractional_uptime(hist, when);
}

/** Return a number representing how long we've known about the router whose
 * digest is <b>id</b>. Return 0 if the router is unknown.
 *
 * Be careful: this measure increases monotonically as we know the router for
 * longer and longer, but it doesn't increase linearly.
 */
long
rep_hist_get_weighted_time_known(const char *id, time_t when)
{
  or_history_t *hist = get_or_history(id);
  if (!hist)
    return 0;

  return get_total_weighted_time(hist, when);
}

/** Return true if we've been measuring MTBFs for long enough to
 * pronounce on Stability. */
int
rep_hist_have_measured_enough_stability(void)
{
  /* XXXX++ This doesn't do so well when we change our opinion
   * as to whether we're tracking router stability. */
  return started_tracking_stability < time(NULL) - 4*60*60;
}

/** Log all the reliability data we have remembered, with the chosen
 * severity.
 */
void
rep_hist_dump_stats(time_t now, int severity)
{
  digestmap_iter_t *orhist_it;
  const char *name1, *digest1;
  char hexdigest1[HEX_DIGEST_LEN+1];
  or_history_t *or_history;
  void *or_history_p;
  const node_t *node;

  rep_history_clean(now - get_options()->RephistTrackTime);

  tor_log(severity, LD_HIST, "--------------- Dumping history information:");

  for (orhist_it = digestmap_iter_init(history_map);
       !digestmap_iter_done(orhist_it);
       orhist_it = digestmap_iter_next(history_map,orhist_it)) {
    double s;
    long stability;
    digestmap_iter_get(orhist_it, &digest1, &or_history_p);
    or_history = (or_history_t*) or_history_p;

    if ((node = node_get_by_id(digest1)) && node_get_nickname(node))
      name1 = node_get_nickname(node);
    else
      name1 = "(unknown)";
    base16_encode(hexdigest1, sizeof(hexdigest1), digest1, DIGEST_LEN);
    s = get_stability(or_history, now);
    stability = (long)s;
    tor_log(severity, LD_HIST,
        "OR %s [%s]: wmtbf %lu:%02lu:%02lu",
        name1, hexdigest1,
        stability/3600, (stability/60)%60, stability%60);
  }
}

/** Remove history info for routers/links that haven't changed since
 * <b>before</b>.
 */
void
rep_history_clean(time_t before)
{
  int authority = authdir_mode(get_options());
  or_history_t *or_history;
  void *or_history_p;
  digestmap_iter_t *orhist_it;
  const char *d1;

  orhist_it = digestmap_iter_init(history_map);
  while (!digestmap_iter_done(orhist_it)) {
    int should_remove;
    digestmap_iter_get(orhist_it, &d1, &or_history_p);
    or_history = or_history_p;

    should_remove = authority ?
                       (or_history->total_run_weights < STABILITY_EPSILON &&
                          !or_history->start_of_run)
                       : (or_history->changed < before);
    if (should_remove) {
      orhist_it = digestmap_iter_next_rmv(history_map, orhist_it);
      free_or_history(or_history);
      continue;
    }
    orhist_it = digestmap_iter_next(history_map, orhist_it);
  }
}

/** Write MTBF data to disk. Return 0 on success, negative on failure.
 *
 * If <b>missing_means_down</b>, then if we're about to write an entry
 * that is still considered up but isn't in our routerlist, consider it
 * to be down. */
int
rep_hist_record_mtbf_data(time_t now, int missing_means_down)
{
  char time_buf[ISO_TIME_LEN+1];

  digestmap_iter_t *orhist_it;
  const char *digest;
  void *or_history_p;
  or_history_t *hist;
  open_file_t *open_file = NULL;
  FILE *f;

  {
    char *filename = get_datadir_fname("router-stability");
    f = start_writing_to_stdio_file(filename, OPEN_FLAGS_REPLACE|O_TEXT, 0600,
                                    &open_file);
    tor_free(filename);
    if (!f)
      return -1;
  }

  /* File format is:
   *   FormatLine *KeywordLine Data
   *
   *   FormatLine = "format 1" NL
   *   KeywordLine = Keyword SP Arguments NL
   *   Data = "data" NL *RouterMTBFLine "." NL
   *   RouterMTBFLine = Fingerprint SP WeightedRunLen SP
   *           TotalRunWeights [SP S=StartRunTime] NL
   */
#define PUT(s) STMT_BEGIN if (fputs((s),f)<0) goto err; STMT_END
#define PRINTF(args) STMT_BEGIN if (fprintf args <0) goto err; STMT_END

  PUT("format 2\n");

  format_iso_time(time_buf, time(NULL));
  PRINTF((f, "stored-at %s\n", time_buf));

  if (started_tracking_stability) {
    format_iso_time(time_buf, started_tracking_stability);
    PRINTF((f, "tracked-since %s\n", time_buf));
  }
  if (stability_last_downrated) {
    format_iso_time(time_buf, stability_last_downrated);
    PRINTF((f, "last-downrated %s\n", time_buf));
  }

  PUT("data\n");

  /* XXX Nick: now bridge auths record this for all routers too.
   * Should we make them record it only for bridge routers? -RD
   * Not for 0.2.0. -NM */
  for (orhist_it = digestmap_iter_init(history_map);
       !digestmap_iter_done(orhist_it);
       orhist_it = digestmap_iter_next(history_map,orhist_it)) {
    char dbuf[HEX_DIGEST_LEN+1];
    const char *t = NULL;
    digestmap_iter_get(orhist_it, &digest, &or_history_p);
    hist = (or_history_t*) or_history_p;

    base16_encode(dbuf, sizeof(dbuf), digest, DIGEST_LEN);

    if (missing_means_down && hist->start_of_run &&
        !connection_or_digest_is_known_relay(digest)) {
      /* We think this relay is running, but it's not listed in our
       * consensus. Somehow it fell out without telling us it went
       * down. Complain and also correct it. */
      log_info(LD_HIST,
               "Relay '%s' is listed as up in rephist, but it's not in "
               "our routerlist. Correcting.", dbuf);
      rep_hist_note_router_unreachable(digest, now);
    }

    PRINTF((f, "R %s\n", dbuf));
    if (hist->start_of_run > 0) {
      format_iso_time(time_buf, hist->start_of_run);
      t = time_buf;
    }
    PRINTF((f, "+MTBF %lu %.5f%s%s\n",
            hist->weighted_run_length, hist->total_run_weights,
            t ? " S=" : "", t ? t : ""));
    t = NULL;
    if (hist->start_of_downtime > 0) {
      format_iso_time(time_buf, hist->start_of_downtime);
      t = time_buf;
    }
    PRINTF((f, "+WFU %lu %lu%s%s\n",
            hist->weighted_uptime, hist->total_weighted_time,
            t ? " S=" : "", t ? t : ""));
  }

  PUT(".\n");

#undef PUT
#undef PRINTF

  return finish_writing_to_file(open_file);
 err:
  abort_writing_to_file(open_file);
  return -1;
}

/** Helper: return the first j >= i such that !strcmpstart(sl[j], prefix) and
 * such that no line sl[k] with i <= k < j starts with "R ".  Return -1 if no
 * such line exists. */
static int
find_next_with(smartlist_t *sl, int i, const char *prefix)
{
  for ( ; i < smartlist_len(sl); ++i) {
    const char *line = smartlist_get(sl, i);
    if (!strcmpstart(line, prefix))
      return i;
    if (!strcmpstart(line, "R "))
      return -1;
  }
  return -1;
}

/** How many bad times has parse_possibly_bad_iso_time() parsed? */
static int n_bogus_times = 0;
/** Parse the ISO-formatted time in <b>s</b> into *<b>time_out</b>, but
 * round any pre-1970 date to Jan 1, 1970. */
static int
parse_possibly_bad_iso_time(const char *s, time_t *time_out)
{
  int year;
  char b[5];
  strlcpy(b, s, sizeof(b));
  b[4] = '\0';
  year = (int)tor_parse_long(b, 10, 0, INT_MAX, NULL, NULL);
  if (year < 1970) {
    *time_out = 0;
    ++n_bogus_times;
    return 0;
  } else
    return parse_iso_time(s, time_out);
}

/** We've read a time <b>t</b> from a file stored at <b>stored_at</b>, which
 * says we started measuring at <b>started_measuring</b>.  Return a new number
 * that's about as much before <b>now</b> as <b>t</b> was before
 * <b>stored_at</b>.
 */
static inline time_t
correct_time(time_t t, time_t now, time_t stored_at, time_t started_measuring)
{
  if (t < started_measuring - 24*60*60*365)
    return 0;
  else if (t < started_measuring)
    return started_measuring;
  else if (t > stored_at)
    return 0;
  else {
    long run_length = stored_at - t;
    t = (time_t)(now - run_length);
    if (t < started_measuring)
      t = started_measuring;
    return t;
  }
}

/** Load MTBF data from disk.  Returns 0 on success or recoverable error, -1
 * on failure. */
int
rep_hist_load_mtbf_data(time_t now)
{
  /* XXXX won't handle being called while history is already populated. */
  smartlist_t *lines;
  const char *line = NULL;
  int r=0, i;
  time_t last_downrated = 0, stored_at = 0, tracked_since = 0;
  time_t latest_possible_start = now;
  long format = -1;

  {
    char *filename = get_datadir_fname("router-stability");
    char *d = read_file_to_str(filename, RFTS_IGNORE_MISSING, NULL);
    tor_free(filename);
    if (!d)
      return -1;
    lines = smartlist_new();
    smartlist_split_string(lines, d, "\n", SPLIT_SKIP_SPACE, 0);
    tor_free(d);
  }

  {
    const char *firstline;
    if (smartlist_len(lines)>4) {
      firstline = smartlist_get(lines, 0);
      if (!strcmpstart(firstline, "format "))
        format = tor_parse_long(firstline+strlen("format "),
                                10, -1, LONG_MAX, NULL, NULL);
    }
  }
  if (format != 1 && format != 2) {
    log_warn(LD_HIST,
             "Unrecognized format in mtbf history file. Skipping.");
    goto err;
  }
  for (i = 1; i < smartlist_len(lines); ++i) {
    line = smartlist_get(lines, i);
    if (!strcmp(line, "data"))
      break;
    if (!strcmpstart(line, "last-downrated ")) {
      if (parse_iso_time(line+strlen("last-downrated "), &last_downrated)<0)
        log_warn(LD_HIST,"Couldn't parse downrate time in mtbf "
                 "history file.");
    }
    if (!strcmpstart(line, "stored-at ")) {
      if (parse_iso_time(line+strlen("stored-at "), &stored_at)<0)
        log_warn(LD_HIST,"Couldn't parse stored time in mtbf "
                 "history file.");
    }
    if (!strcmpstart(line, "tracked-since ")) {
      if (parse_iso_time(line+strlen("tracked-since "), &tracked_since)<0)
        log_warn(LD_HIST,"Couldn't parse started-tracking time in mtbf "
                 "history file.");
    }
  }
  if (last_downrated > now)
    last_downrated = now;
  if (tracked_since > now)
    tracked_since = now;

  if (!stored_at) {
    log_warn(LD_HIST, "No stored time recorded.");
    goto err;
  }

  if (line && !strcmp(line, "data"))
    ++i;

  n_bogus_times = 0;

  for (; i < smartlist_len(lines); ++i) {
    char digest[DIGEST_LEN];
    char hexbuf[HEX_DIGEST_LEN+1];
    char mtbf_timebuf[ISO_TIME_LEN+1];
    char wfu_timebuf[ISO_TIME_LEN+1];
    time_t start_of_run = 0;
    time_t start_of_downtime = 0;
    int have_mtbf = 0, have_wfu = 0;
    long wrl = 0;
    double trw = 0;
    long wt_uptime = 0, total_wt_time = 0;
    int n;
    or_history_t *hist;
    line = smartlist_get(lines, i);
    if (!strcmp(line, "."))
      break;

    mtbf_timebuf[0] = '\0';
    wfu_timebuf[0] = '\0';

    if (format == 1) {
      n = tor_sscanf(line, "%40s %ld %lf S=%10s %8s",
                 hexbuf, &wrl, &trw, mtbf_timebuf, mtbf_timebuf+11);
      if (n != 3 && n != 5) {
        log_warn(LD_HIST, "Couldn't scan line %s", escaped(line));
        continue;
      }
      have_mtbf = 1;
    } else {
      // format == 2.
      int mtbf_idx, wfu_idx;
      if (strcmpstart(line, "R ") || strlen(line) < 2+HEX_DIGEST_LEN)
        continue;
      strlcpy(hexbuf, line+2, sizeof(hexbuf));
      mtbf_idx = find_next_with(lines, i+1, "+MTBF ");
      wfu_idx = find_next_with(lines, i+1, "+WFU ");
      if (mtbf_idx >= 0) {
        const char *mtbfline = smartlist_get(lines, mtbf_idx);
        n = tor_sscanf(mtbfline, "+MTBF %lu %lf S=%10s %8s",
                   &wrl, &trw, mtbf_timebuf, mtbf_timebuf+11);
        if (n == 2 || n == 4) {
          have_mtbf = 1;
        } else {
          log_warn(LD_HIST, "Couldn't scan +MTBF line %s",
                   escaped(mtbfline));
        }
      }
      if (wfu_idx >= 0) {
        const char *wfuline = smartlist_get(lines, wfu_idx);
        n = tor_sscanf(wfuline, "+WFU %lu %lu S=%10s %8s",
                   &wt_uptime, &total_wt_time,
                   wfu_timebuf, wfu_timebuf+11);
        if (n == 2 || n == 4) {
          have_wfu = 1;
        } else {
          log_warn(LD_HIST, "Couldn't scan +WFU line %s", escaped(wfuline));
        }
      }
      if (wfu_idx > i)
        i = wfu_idx;
      if (mtbf_idx > i)
        i = mtbf_idx;
    }
    if (base16_decode(digest, DIGEST_LEN,
                      hexbuf, HEX_DIGEST_LEN) != DIGEST_LEN) {
      log_warn(LD_HIST, "Couldn't hex string %s", escaped(hexbuf));
      continue;
    }
    hist = get_or_history(digest);
    if (!hist)
      continue;

    if (have_mtbf) {
      if (mtbf_timebuf[0]) {
        mtbf_timebuf[10] = ' ';
        if (parse_possibly_bad_iso_time(mtbf_timebuf, &start_of_run)<0)
          log_warn(LD_HIST, "Couldn't parse time %s",
                   escaped(mtbf_timebuf));
      }
      hist->start_of_run = correct_time(start_of_run, now, stored_at,
                                        tracked_since);
      if (hist->start_of_run < latest_possible_start + wrl)
        latest_possible_start = (time_t)(hist->start_of_run - wrl);

      hist->weighted_run_length = wrl;
      hist->total_run_weights = trw;
    }
    if (have_wfu) {
      if (wfu_timebuf[0]) {
        wfu_timebuf[10] = ' ';
        if (parse_possibly_bad_iso_time(wfu_timebuf, &start_of_downtime)<0)
          log_warn(LD_HIST, "Couldn't parse time %s", escaped(wfu_timebuf));
      }
    }
    hist->start_of_downtime = correct_time(start_of_downtime, now, stored_at,
                                           tracked_since);
    hist->weighted_uptime = wt_uptime;
    hist->total_weighted_time = total_wt_time;
  }
  if (strcmp(line, "."))
    log_warn(LD_HIST, "Truncated MTBF file.");

  if (tracked_since < 86400*365) /* Recover from insanely early value. */
    tracked_since = latest_possible_start;

  stability_last_downrated = last_downrated;
  started_tracking_stability = tracked_since;

  goto done;
 err:
  r = -1;
 done:
  SMARTLIST_FOREACH(lines, char *, cp, tor_free(cp));
  smartlist_free(lines);
  return r;
}

/*** Exit port statistics ***/

/* Some constants */
/** To what multiple should byte numbers be rounded up? */
#define EXIT_STATS_ROUND_UP_BYTES 1024
/** To what multiple should stream counts be rounded up? */
#define EXIT_STATS_ROUND_UP_STREAMS 4
/** Number of TCP ports */
#define EXIT_STATS_NUM_PORTS 65536
/** Top n ports that will be included in exit stats. */
#define EXIT_STATS_TOP_N_PORTS 10

/* The following data structures are arrays and no fancy smartlists or maps,
 * so that all write operations can be done in constant time. This comes at
 * the price of some memory (1.25 MB) and linear complexity when writing
 * stats for measuring relays. */
/** Number of bytes read in current period by exit port */
static uint64_t *exit_bytes_read = NULL;
/** Number of bytes written in current period by exit port */
static uint64_t *exit_bytes_written = NULL;
/** Number of streams opened in current period by exit port */
static uint32_t *exit_streams = NULL;

/** Start time of exit stats or 0 if we're not collecting exit stats. */
static time_t start_of_exit_stats_interval;

/** Initialize exit port stats. */
void
rep_hist_exit_stats_init(time_t now)
{
  start_of_exit_stats_interval = now;
  exit_bytes_read = tor_calloc(EXIT_STATS_NUM_PORTS, sizeof(uint64_t));
  exit_bytes_written = tor_calloc(EXIT_STATS_NUM_PORTS, sizeof(uint64_t));
  exit_streams = tor_calloc(EXIT_STATS_NUM_PORTS, sizeof(uint32_t));
}

/** Reset counters for exit port statistics. */
void
rep_hist_reset_exit_stats(time_t now)
{
  start_of_exit_stats_interval = now;
  memset(exit_bytes_read, 0, EXIT_STATS_NUM_PORTS * sizeof(uint64_t));
  memset(exit_bytes_written, 0, EXIT_STATS_NUM_PORTS * sizeof(uint64_t));
  memset(exit_streams, 0, EXIT_STATS_NUM_PORTS * sizeof(uint32_t));
}

/** Stop collecting exit port stats in a way that we can re-start doing
 * so in rep_hist_exit_stats_init(). */
void
rep_hist_exit_stats_term(void)
{
  start_of_exit_stats_interval = 0;
  tor_free(exit_bytes_read);
  tor_free(exit_bytes_written);
  tor_free(exit_streams);
}

/** Helper for qsort: compare two ints.  Does not handle overflow properly,
 * but works fine for sorting an array of port numbers, which is what we use
 * it for. */
static int
compare_int_(const void *x, const void *y)
{
  return (*(int*)x - *(int*)y);
}

/** Return a newly allocated string containing the exit port statistics
 * until <b>now</b>, or NULL if we're not collecting exit stats. Caller
 * must ensure start_of_exit_stats_interval is in the past. */
char *
rep_hist_format_exit_stats(time_t now)
{
  int i, j, top_elements = 0, cur_min_idx = 0, cur_port;
  uint64_t top_bytes[EXIT_STATS_TOP_N_PORTS];
  int top_ports[EXIT_STATS_TOP_N_PORTS];
  uint64_t cur_bytes = 0, other_read = 0, other_written = 0,
           total_read = 0, total_written = 0;
  uint32_t total_streams = 0, other_streams = 0;
  smartlist_t *written_strings, *read_strings, *streams_strings;
  char *written_string, *read_string, *streams_string;
  char t[ISO_TIME_LEN+1];
  char *result;

  if (!start_of_exit_stats_interval)
    return NULL; /* Not initialized. */

  tor_assert(now >= start_of_exit_stats_interval);

  /* Go through all ports to find the n ports that saw most written and
   * read bytes.
   *
   * Invariant: at the end of the loop for iteration i,
   *    total_read is the sum of all exit_bytes_read[0..i]
   *    total_written is the sum of all exit_bytes_written[0..i]
   *    total_stream is the sum of all exit_streams[0..i]
   *
   *    top_elements = MAX(EXIT_STATS_TOP_N_PORTS,
   *                  #{j | 0 <= j <= i && volume(i) > 0})
   *
   *    For all 0 <= j < top_elements,
   *        top_bytes[j] > 0
   *        0 <= top_ports[j] <= 65535
   *        top_bytes[j] = volume(top_ports[j])
   *
   *    There is no j in 0..i and k in 0..top_elements such that:
   *        volume(j) > top_bytes[k] AND j is not in top_ports[0..top_elements]
   *
   *    There is no j!=cur_min_idx in 0..top_elements such that:
   *        top_bytes[j] < top_bytes[cur_min_idx]
   *
   * where volume(x) == exit_bytes_read[x]+exit_bytes_written[x]
   *
   * Worst case: O(EXIT_STATS_NUM_PORTS * EXIT_STATS_TOP_N_PORTS)
   */
  for (i = 1; i < EXIT_STATS_NUM_PORTS; i++) {
    total_read += exit_bytes_read[i];
    total_written += exit_bytes_written[i];
    total_streams += exit_streams[i];
    cur_bytes = exit_bytes_read[i] + exit_bytes_written[i];
    if (cur_bytes == 0) {
      continue;
    }
    if (top_elements < EXIT_STATS_TOP_N_PORTS) {
      top_bytes[top_elements] = cur_bytes;
      top_ports[top_elements++] = i;
    } else if (cur_bytes > top_bytes[cur_min_idx]) {
      top_bytes[cur_min_idx] = cur_bytes;
      top_ports[cur_min_idx] = i;
    } else {
      continue;
    }
    cur_min_idx = 0;
    for (j = 1; j < top_elements; j++) {
      if (top_bytes[j] < top_bytes[cur_min_idx]) {
        cur_min_idx = j;
      }
    }
  }

  /* Add observations of top ports to smartlists. */
  written_strings = smartlist_new();
  read_strings = smartlist_new();
  streams_strings = smartlist_new();
  other_read = total_read;
  other_written = total_written;
  other_streams = total_streams;
  /* Sort the ports; this puts them out of sync with top_bytes, but we
   * won't be using top_bytes again anyway */
  qsort(top_ports, top_elements, sizeof(int), compare_int_);
  for (j = 0; j < top_elements; j++) {
    cur_port = top_ports[j];
    if (exit_bytes_written[cur_port] > 0) {
      uint64_t num = round_uint64_to_next_multiple_of(
                     exit_bytes_written[cur_port],
                     EXIT_STATS_ROUND_UP_BYTES);
      num /= 1024;
      smartlist_add_asprintf(written_strings, "%d=%"PRIu64,
                             cur_port, (num));
      other_written -= exit_bytes_written[cur_port];
    }
    if (exit_bytes_read[cur_port] > 0) {
      uint64_t num = round_uint64_to_next_multiple_of(
                     exit_bytes_read[cur_port],
                     EXIT_STATS_ROUND_UP_BYTES);
      num /= 1024;
      smartlist_add_asprintf(read_strings, "%d=%"PRIu64,
                             cur_port, (num));
      other_read -= exit_bytes_read[cur_port];
    }
    if (exit_streams[cur_port] > 0) {
      uint32_t num = round_uint32_to_next_multiple_of(
                     exit_streams[cur_port],
                     EXIT_STATS_ROUND_UP_STREAMS);
      smartlist_add_asprintf(streams_strings, "%d=%u", cur_port, num);
      other_streams -= exit_streams[cur_port];
    }
  }

  /* Add observations of other ports in a single element. */
  other_written = round_uint64_to_next_multiple_of(other_written,
                  EXIT_STATS_ROUND_UP_BYTES);
  other_written /= 1024;
  smartlist_add_asprintf(written_strings, "other=%"PRIu64,
                         (other_written));
  other_read = round_uint64_to_next_multiple_of(other_read,
               EXIT_STATS_ROUND_UP_BYTES);
  other_read /= 1024;
  smartlist_add_asprintf(read_strings, "other=%"PRIu64,
                         (other_read));
  other_streams = round_uint32_to_next_multiple_of(other_streams,
                  EXIT_STATS_ROUND_UP_STREAMS);
  smartlist_add_asprintf(streams_strings, "other=%u", other_streams);

  /* Join all observations in single strings. */
  written_string = smartlist_join_strings(written_strings, ",", 0, NULL);
  read_string = smartlist_join_strings(read_strings, ",", 0, NULL);
  streams_string = smartlist_join_strings(streams_strings, ",", 0, NULL);
  SMARTLIST_FOREACH(written_strings, char *, cp, tor_free(cp));
  SMARTLIST_FOREACH(read_strings, char *, cp, tor_free(cp));
  SMARTLIST_FOREACH(streams_strings, char *, cp, tor_free(cp));
  smartlist_free(written_strings);
  smartlist_free(read_strings);
  smartlist_free(streams_strings);

  /* Put everything together. */
  format_iso_time(t, now);
  tor_asprintf(&result, "exit-stats-end %s (%d s)\n"
               "exit-kibibytes-written %s\n"
               "exit-kibibytes-read %s\n"
               "exit-streams-opened %s\n",
               t, (unsigned) (now - start_of_exit_stats_interval),
               written_string,
               read_string,
               streams_string);
  tor_free(written_string);
  tor_free(read_string);
  tor_free(streams_string);
  return result;
}

/** If 24 hours have passed since the beginning of the current exit port
 * stats period, write exit stats to $DATADIR/stats/exit-stats (possibly
 * overwriting an existing file) and reset counters.  Return when we would
 * next want to write exit stats or 0 if we never want to write. */
time_t
rep_hist_exit_stats_write(time_t now)
{
  char *str = NULL;

  if (!start_of_exit_stats_interval)
    return 0; /* Not initialized. */
  if (start_of_exit_stats_interval + WRITE_STATS_INTERVAL > now)
    goto done; /* Not ready to write. */

  log_info(LD_HIST, "Writing exit port statistics to disk.");

  /* Generate history string. */
  str = rep_hist_format_exit_stats(now);

  /* Reset counters. */
  rep_hist_reset_exit_stats(now);

  /* Try to write to disk. */
  if (!check_or_create_data_subdir("stats")) {
    write_to_data_subdir("stats", "exit-stats", str, "exit port statistics");
  }

 done:
  tor_free(str);
  return start_of_exit_stats_interval + WRITE_STATS_INTERVAL;
}

/** Note that we wrote <b>num_written</b> bytes and read <b>num_read</b>
 * bytes to/from an exit connection to <b>port</b>. */
void
rep_hist_note_exit_bytes(uint16_t port, size_t num_written,
                         size_t num_read)
{
  if (!start_of_exit_stats_interval)
    return; /* Not initialized. */
  exit_bytes_written[port] += num_written;
  exit_bytes_read[port] += num_read;
  log_debug(LD_HIST, "Written %lu bytes and read %lu bytes to/from an "
            "exit connection to port %d.",
            (unsigned long)num_written, (unsigned long)num_read, port);
}

/** Note that we opened an exit stream to <b>port</b>. */
void
rep_hist_note_exit_stream_opened(uint16_t port)
{
  if (!start_of_exit_stats_interval)
    return; /* Not initialized. */
  exit_streams[port]++;
  log_debug(LD_HIST, "Opened exit stream to port %d", port);
}

/*** Exit streams statistics ***/

/** Number of BEGIN streams seen. */
static uint64_t streams_begin_seen;
/** Number of BEGIN_DIR streams seen. */
static uint64_t streams_begindir_seen;
/** Number of RESOLVE streams seen. */
static uint64_t streams_resolve_seen;

/** Note a stream as seen for the given relay command. */
void
rep_hist_note_exit_stream(unsigned int cmd)
{
  switch (cmd) {
  case RELAY_COMMAND_BEGIN:
    streams_begin_seen++;
    break;
  case RELAY_COMMAND_BEGIN_DIR:
    streams_begindir_seen++;
    break;
  case RELAY_COMMAND_RESOLVE:
    streams_resolve_seen++;
    break;
  default:
    tor_assert_nonfatal_unreached_once();
    break;
  }
}

/** Return number of stream seen for the given command. */
uint64_t
rep_hist_get_exit_stream_seen(unsigned int cmd)
{
  switch (cmd) {
  case RELAY_COMMAND_BEGIN:
    return streams_begin_seen;
  case RELAY_COMMAND_BEGIN_DIR:
    return streams_begindir_seen;
  case RELAY_COMMAND_RESOLVE:
    return streams_resolve_seen;
  default:
    return 0;
  }
}

/******* Connections statistics *******/

#define CONN_DIRECTION_INITIATED 0
#define CONN_DIRECTION_RECEIVED  1

#define CONN_DIRECTION(from_listener) \
  (from_listener) ? CONN_DIRECTION_RECEIVED : CONN_DIRECTION_INITIATED

/** Number of connections created as in seen per direction per type. */
static uint64_t conn_num_created_v4[2][CONN_TYPE_MAX_];
static uint64_t conn_num_created_v6[2][CONN_TYPE_MAX_];
/** Number of connections opened per direction per type. */
static uint64_t conn_num_opened_v4[2][CONN_TYPE_MAX_];
static uint64_t conn_num_opened_v6[2][CONN_TYPE_MAX_];
/** Number of connections rejected per type. Always inbound. */
static uint64_t conn_num_rejected_v4[CONN_TYPE_MAX_];
static uint64_t conn_num_rejected_v6[CONN_TYPE_MAX_];

/** Note that a connection has opened of the given type. */
void
rep_hist_note_conn_opened(bool from_listener, unsigned int type, int af)
{
  tor_assert(type <= CONN_TYPE_MAX_);

  unsigned int dir = CONN_DIRECTION(from_listener);

  switch (af) {
  case AF_INET:
    conn_num_created_v4[dir][type]++;
    conn_num_opened_v4[dir][type]++;
    break;
  case AF_INET6:
    conn_num_created_v6[dir][type]++;
    conn_num_opened_v6[dir][type]++;
    break;
  default:
    /* Ignore non IP connections at this point in time. */
    break;
  }
}

/** Note that a connection has closed of the given type. */
void
rep_hist_note_conn_closed(bool from_listener, unsigned int type, int af)
{
  tor_assert(type <= CONN_TYPE_MAX_);

  unsigned int dir = CONN_DIRECTION(from_listener);

  switch (af) {
  case AF_INET:
    if (conn_num_opened_v4[dir][type] > 0) {
      conn_num_opened_v4[dir][type]--;
    }
    break;
  case AF_INET6:
    if (conn_num_opened_v6[dir][type] > 0) {
      conn_num_opened_v6[dir][type]--;
    }
    break;
  default:
    /* Ignore non IP connections at this point in time. */
    break;
  }
}

/** Note that a connection has rejected of the given type. */
void
rep_hist_note_conn_rejected(unsigned int type, int af)
{
  tor_assert(type <= CONN_TYPE_MAX_);

  switch (af) {
  case AF_INET:
    conn_num_rejected_v4[type]++;
    break;
  case AF_INET6:
    conn_num_rejected_v6[type]++;
    break;
  default:
    /* Ignore non IP connections at this point in time. */
    break;
  }
}

/** Return number of created connections of the given type. */
uint64_t
rep_hist_get_conn_created(bool from_listener, unsigned int type, int af)
{
  tor_assert(type <= CONN_TYPE_MAX_);
  unsigned int dir = CONN_DIRECTION(from_listener);
  switch (af) {
  case AF_INET:
    return conn_num_created_v4[dir][type];
  case AF_INET6:
    return conn_num_created_v6[dir][type];
  default:
    return 0;
  }
}

/** Return number of opened connections of the given type. */
uint64_t
rep_hist_get_conn_opened(bool from_listener, unsigned int type, int af)
{
  tor_assert(type <= CONN_TYPE_MAX_);
  unsigned int dir = CONN_DIRECTION(from_listener);
  switch (af) {
  case AF_INET:
    return conn_num_opened_v4[dir][type];
  case AF_INET6:
    return conn_num_opened_v6[dir][type];
  default:
    return 0;
  }
}

/** Return number of opened connections of the given type. */
uint64_t
rep_hist_get_conn_rejected(unsigned int type, int af)
{
  tor_assert(type <= CONN_TYPE_MAX_);
  switch (af) {
  case AF_INET:
    return conn_num_rejected_v4[type];
  case AF_INET6:
    return conn_num_rejected_v6[type];
  default:
    return 0;
  }
}

/*** cell statistics ***/

/** Start of the current buffer stats interval or 0 if we're not
 * collecting buffer statistics. */
static time_t start_of_buffer_stats_interval;

/** Initialize buffer stats. */
void
rep_hist_buffer_stats_init(time_t now)
{
  start_of_buffer_stats_interval = now;
}

/** Statistics from a single circuit.  Collected when the circuit closes, or
 * when we flush statistics to disk. */
typedef struct circ_buffer_stats_t {
  /** Average number of cells in the circuit's queue */
  double mean_num_cells_in_queue;
  /** Average time a cell waits in the queue. */
  double mean_time_cells_in_queue;
  /** Total number of cells sent over this circuit */
  uint32_t processed_cells;
} circ_buffer_stats_t;

/** List of circ_buffer_stats_t. */
static smartlist_t *circuits_for_buffer_stats = NULL;

/** Remember cell statistics <b>mean_num_cells_in_queue</b>,
 * <b>mean_time_cells_in_queue</b>, and <b>processed_cells</b> of a
 * circuit. */
void
rep_hist_add_buffer_stats(double mean_num_cells_in_queue,
    double mean_time_cells_in_queue, uint32_t processed_cells)
{
  circ_buffer_stats_t *stats;
  if (!start_of_buffer_stats_interval)
    return; /* Not initialized. */
  stats = tor_malloc_zero(sizeof(circ_buffer_stats_t));
  stats->mean_num_cells_in_queue = mean_num_cells_in_queue;
  stats->mean_time_cells_in_queue = mean_time_cells_in_queue;
  stats->processed_cells = processed_cells;
  if (!circuits_for_buffer_stats)
    circuits_for_buffer_stats = smartlist_new();
  smartlist_add(circuits_for_buffer_stats, stats);
}

/** Remember cell statistics for circuit <b>circ</b> at time
 * <b>end_of_interval</b> and reset cell counters in case the circuit
 * remains open in the next measurement interval. */
void
rep_hist_buffer_stats_add_circ(circuit_t *circ, time_t end_of_interval)
{
  time_t start_of_interval;
  int interval_length;
  or_circuit_t *orcirc;
  double mean_num_cells_in_queue, mean_time_cells_in_queue;
  uint32_t processed_cells;
  if (CIRCUIT_IS_ORIGIN(circ))
    return;
  orcirc = TO_OR_CIRCUIT(circ);
  if (!orcirc->processed_cells)
    return;
  start_of_interval = (circ->timestamp_created.tv_sec >
                       start_of_buffer_stats_interval) ?
        (time_t)circ->timestamp_created.tv_sec :
        start_of_buffer_stats_interval;
  interval_length = (int) (end_of_interval - start_of_interval);
  if (interval_length <= 0)
    return;
  processed_cells = orcirc->processed_cells;
  /* 1000.0 for s -> ms; 2.0 because of app-ward and exit-ward queues */
  mean_num_cells_in_queue = (double) orcirc->total_cell_waiting_time /
      (double) interval_length / 1000.0 / 2.0;
  mean_time_cells_in_queue =
      (double) orcirc->total_cell_waiting_time /
      (double) orcirc->processed_cells;
  orcirc->total_cell_waiting_time = 0;
  orcirc->processed_cells = 0;
  rep_hist_add_buffer_stats(mean_num_cells_in_queue,
                            mean_time_cells_in_queue,
                            processed_cells);
}

/** Sorting helper: return -1, 1, or 0 based on comparison of two
 * circ_buffer_stats_t */
static int
buffer_stats_compare_entries_(const void **_a, const void **_b)
{
  const circ_buffer_stats_t *a = *_a, *b = *_b;
  if (a->processed_cells < b->processed_cells)
    return 1;
  else if (a->processed_cells > b->processed_cells)
    return -1;
  else
    return 0;
}

/** Stop collecting cell stats in a way that we can re-start doing so in
 * rep_hist_buffer_stats_init(). */
void
rep_hist_buffer_stats_term(void)
{
  rep_hist_reset_buffer_stats(0);
}

/** Clear history of circuit statistics and set the measurement interval
 * start to <b>now</b>. */
void
rep_hist_reset_buffer_stats(time_t now)
{
  if (!circuits_for_buffer_stats)
    circuits_for_buffer_stats = smartlist_new();
  SMARTLIST_FOREACH(circuits_for_buffer_stats, circ_buffer_stats_t *,
      stats, tor_free(stats));
  smartlist_clear(circuits_for_buffer_stats);
  start_of_buffer_stats_interval = now;
}

/** Return a newly allocated string containing the buffer statistics until
 * <b>now</b>, or NULL if we're not collecting buffer stats. Caller must
 * ensure start_of_buffer_stats_interval is in the past. */
char *
rep_hist_format_buffer_stats(time_t now)
{
#define SHARES 10
  uint64_t processed_cells[SHARES];
  uint32_t circs_in_share[SHARES];
  int number_of_circuits, i;
  double queued_cells[SHARES], time_in_queue[SHARES];
  smartlist_t *processed_cells_strings, *queued_cells_strings,
              *time_in_queue_strings;
  char *processed_cells_string, *queued_cells_string,
       *time_in_queue_string;
  char t[ISO_TIME_LEN+1];
  char *result;

  if (!start_of_buffer_stats_interval)
    return NULL; /* Not initialized. */

  tor_assert(now >= start_of_buffer_stats_interval);

  /* Calculate deciles if we saw at least one circuit. */
  memset(processed_cells, 0, SHARES * sizeof(uint64_t));
  memset(circs_in_share, 0, SHARES * sizeof(uint32_t));
  memset(queued_cells, 0, SHARES * sizeof(double));
  memset(time_in_queue, 0, SHARES * sizeof(double));
  if (!circuits_for_buffer_stats)
    circuits_for_buffer_stats = smartlist_new();
  number_of_circuits = smartlist_len(circuits_for_buffer_stats);
  if (number_of_circuits > 0) {
    smartlist_sort(circuits_for_buffer_stats,
                   buffer_stats_compare_entries_);
    i = 0;
    SMARTLIST_FOREACH_BEGIN(circuits_for_buffer_stats,
                            circ_buffer_stats_t *, stats)
    {
      int share = i++ * SHARES / number_of_circuits;
      processed_cells[share] += stats->processed_cells;
      queued_cells[share] += stats->mean_num_cells_in_queue;
      time_in_queue[share] += stats->mean_time_cells_in_queue;
      circs_in_share[share]++;
    }
    SMARTLIST_FOREACH_END(stats);
  }

  /* Write deciles to strings. */
  processed_cells_strings = smartlist_new();
  queued_cells_strings = smartlist_new();
  time_in_queue_strings = smartlist_new();
  for (i = 0; i < SHARES; i++) {
    smartlist_add_asprintf(processed_cells_strings,
                           "%"PRIu64, !circs_in_share[i] ? 0 :
                           (processed_cells[i] /
                           circs_in_share[i]));
  }
  for (i = 0; i < SHARES; i++) {
    smartlist_add_asprintf(queued_cells_strings, "%.2f",
                           circs_in_share[i] == 0 ? 0.0 :
                             queued_cells[i] / (double) circs_in_share[i]);
  }
  for (i = 0; i < SHARES; i++) {
    smartlist_add_asprintf(time_in_queue_strings, "%.0f",
                           circs_in_share[i] == 0 ? 0.0 :
                             time_in_queue[i] / (double) circs_in_share[i]);
  }

  /* Join all observations in single strings. */
  processed_cells_string = smartlist_join_strings(processed_cells_strings,
                                                  ",", 0, NULL);
  queued_cells_string = smartlist_join_strings(queued_cells_strings,
                                               ",", 0, NULL);
  time_in_queue_string = smartlist_join_strings(time_in_queue_strings,
                                                ",", 0, NULL);
  SMARTLIST_FOREACH(processed_cells_strings, char *, cp, tor_free(cp));
  SMARTLIST_FOREACH(queued_cells_strings, char *, cp, tor_free(cp));
  SMARTLIST_FOREACH(time_in_queue_strings, char *, cp, tor_free(cp));
  smartlist_free(processed_cells_strings);
  smartlist_free(queued_cells_strings);
  smartlist_free(time_in_queue_strings);

  /* Put everything together. */
  format_iso_time(t, now);
  tor_asprintf(&result, "cell-stats-end %s (%d s)\n"
               "cell-processed-cells %s\n"
               "cell-queued-cells %s\n"
               "cell-time-in-queue %s\n"
               "cell-circuits-per-decile %d\n",
               t, (unsigned) (now - start_of_buffer_stats_interval),
               processed_cells_string,
               queued_cells_string,
               time_in_queue_string,
               CEIL_DIV(number_of_circuits, SHARES));
  tor_free(processed_cells_string);
  tor_free(queued_cells_string);
  tor_free(time_in_queue_string);
  return result;
#undef SHARES
}

/** If 24 hours have passed since the beginning of the current buffer
 * stats period, write buffer stats to $DATADIR/stats/buffer-stats
 * (possibly overwriting an existing file) and reset counters.  Return
 * when we would next want to write buffer stats or 0 if we never want to
 * write. */
time_t
rep_hist_buffer_stats_write(time_t now)
{
  char *str = NULL;

  if (!start_of_buffer_stats_interval)
    return 0; /* Not initialized. */
  if (start_of_buffer_stats_interval + WRITE_STATS_INTERVAL > now)
    goto done; /* Not ready to write */

  /* Add open circuits to the history. */
  SMARTLIST_FOREACH_BEGIN(circuit_get_global_list(), circuit_t *, circ) {
    rep_hist_buffer_stats_add_circ(circ, now);
  }
  SMARTLIST_FOREACH_END(circ);

  /* Generate history string. */
  str = rep_hist_format_buffer_stats(now);

  /* Reset both buffer history and counters of open circuits. */
  rep_hist_reset_buffer_stats(now);

  /* Try to write to disk. */
  if (!check_or_create_data_subdir("stats")) {
    write_to_data_subdir("stats", "buffer-stats", str, "buffer statistics");
  }

 done:
  tor_free(str);
  return start_of_buffer_stats_interval + WRITE_STATS_INTERVAL;
}

/*** Descriptor serving statistics ***/

/** Digestmap to track which descriptors were downloaded this stats
 *  collection interval. It maps descriptor digest to pointers to 1,
 *  effectively turning this into a list. */
static digestmap_t *served_descs = NULL;

/** Number of how many descriptors were downloaded in total during this
 * interval. */
static unsigned long total_descriptor_downloads;

/** Start time of served descs stats or 0 if we're not collecting those. */
static time_t start_of_served_descs_stats_interval;

/** Initialize descriptor stats. */
void
rep_hist_desc_stats_init(time_t now)
{
  if (served_descs) {
    log_warn(LD_BUG, "Called rep_hist_desc_stats_init() when desc stats were "
             "already initialized. This is probably harmless.");
    return; // Already initialized
  }
  served_descs = digestmap_new();
  total_descriptor_downloads = 0;
  start_of_served_descs_stats_interval = now;
}

/** Reset served descs stats to empty, starting a new interval <b>now</b>. */
static void
rep_hist_reset_desc_stats(time_t now)
{
  rep_hist_desc_stats_term();
  rep_hist_desc_stats_init(now);
}

/** Stop collecting served descs stats, so that rep_hist_desc_stats_init() is
 * safe to be called again. */
void
rep_hist_desc_stats_term(void)
{
  digestmap_free(served_descs, NULL);
  served_descs = NULL;
  start_of_served_descs_stats_interval = 0;
  total_descriptor_downloads = 0;
}

/** Helper for rep_hist_desc_stats_write(). Return a newly allocated string
 * containing the served desc statistics until now, or NULL if we're not
 * collecting served desc stats. Caller must ensure that now is not before
 * start_of_served_descs_stats_interval. */
static char *
rep_hist_format_desc_stats(time_t now)
{
  char t[ISO_TIME_LEN+1];
  char *result;

  digestmap_iter_t *iter;
  const char *key;
  void *val;
  unsigned size;
  int *vals, max = 0, q3 = 0, md = 0, q1 = 0, min = 0;
  int n = 0;

  if (!start_of_served_descs_stats_interval)
    return NULL;

  size = digestmap_size(served_descs);
  if (size > 0) {
    vals = tor_calloc(size, sizeof(int));
    for (iter = digestmap_iter_init(served_descs);
         !digestmap_iter_done(iter);
         iter = digestmap_iter_next(served_descs, iter)) {
      uintptr_t count;
      digestmap_iter_get(iter, &key, &val);
      count = (uintptr_t)val;
      vals[n++] = (int)count;
      (void)key;
    }
    max = find_nth_int(vals, size, size-1);
    q3 = find_nth_int(vals, size, (3*size-1)/4);
    md = find_nth_int(vals, size, (size-1)/2);
    q1 = find_nth_int(vals, size, (size-1)/4);
    min = find_nth_int(vals, size, 0);
    tor_free(vals);
  }

  format_iso_time(t, now);

  tor_asprintf(&result,
               "served-descs-stats-end %s (%d s) total=%lu unique=%u "
               "max=%d q3=%d md=%d q1=%d min=%d\n",
               t,
               (unsigned) (now - start_of_served_descs_stats_interval),
               total_descriptor_downloads,
               size, max, q3, md, q1, min);

  return result;
}

/** If WRITE_STATS_INTERVAL seconds have passed since the beginning of
 * the current served desc stats interval, write the stats to
 * $DATADIR/stats/served-desc-stats (possibly appending to an existing file)
 * and reset the state for the next interval. Return when we would next want
 * to write served desc stats or 0 if we won't want to write. */
time_t
rep_hist_desc_stats_write(time_t now)
{
  char *filename = NULL, *str = NULL;

  if (!start_of_served_descs_stats_interval)
    return 0; /* We're not collecting stats. */
  if (start_of_served_descs_stats_interval + WRITE_STATS_INTERVAL > now)
    return start_of_served_descs_stats_interval + WRITE_STATS_INTERVAL;

  str = rep_hist_format_desc_stats(now);
  tor_assert(str != NULL);

  if (check_or_create_data_subdir("stats") < 0) {
    goto done;
  }
  filename = get_datadir_fname2("stats", "served-desc-stats");
  if (append_bytes_to_file(filename, str, strlen(str), 0) < 0)
    log_warn(LD_HIST, "Unable to write served descs statistics to disk!");

  rep_hist_reset_desc_stats(now);

 done:
  tor_free(filename);
  tor_free(str);
  return start_of_served_descs_stats_interval + WRITE_STATS_INTERVAL;
}

/** Called to note that we've served a given descriptor (by
 * digest). Increments the count of descriptors served, and the number
 * of times we've served this descriptor. */
void
rep_hist_note_desc_served(const char * desc)
{
  void *val;
  uintptr_t count;
  if (!served_descs)
    return; // We're not collecting stats
  val = digestmap_get(served_descs, desc);
  count = (uintptr_t)val;
  if (count != INT_MAX)
    ++count;
  digestmap_set(served_descs, desc, (void*)count);
  total_descriptor_downloads++;
}

/*** Connection statistics ***/

/** Internal statistics to track how many requests of each type of
 * handshake we've received, and how many we've assigned to cpuworkers.
 * Useful for seeing trends in cpu load.
 *
 * They are reset at every heartbeat.
 * @{ */
STATIC int onion_handshakes_requested[MAX_ONION_STAT_TYPE+1] = {0};
STATIC int onion_handshakes_assigned[MAX_ONION_STAT_TYPE+1] = {0};
/**@}*/

/** Counters keeping the same stats as above but for the entire duration of the
 * process (not reset). */
static uint64_t stats_n_onionskin_assigned[MAX_ONION_STAT_TYPE+1] = {0};
static uint64_t stats_n_onionskin_dropped[MAX_ONION_STAT_TYPE+1] = {0};

/* We use a scale here so we can represent percentages with decimal points by
 * scaling the value by this factor and so 0.5% becomes a value of 500.
 * Default is 1% and thus min and max range is 0 to 100%. */
#define OVERLOAD_ONIONSKIN_NTOR_PERCENT_SCALE 1000.0
#define OVERLOAD_ONIONSKIN_NTOR_PERCENT_DEFAULT 1000
#define OVERLOAD_ONIONSKIN_NTOR_PERCENT_MIN 0
#define OVERLOAD_ONIONSKIN_NTOR_PERCENT_MAX 100000

/** Consensus parameter: indicate what fraction of ntor onionskin drop over the
 * total number of requests must be reached before we trigger a general
 * overload signal.*/
static double overload_onionskin_ntor_fraction =
   OVERLOAD_ONIONSKIN_NTOR_PERCENT_DEFAULT /
   OVERLOAD_ONIONSKIN_NTOR_PERCENT_SCALE / 100.0;

/* Number of seconds for the assessment period. Default is 6 hours (21600) and
 * the min max range is within a 32bit value. We align this period to the
 * Heartbeat so the logs would match this period more or less. */
#define OVERLOAD_ONIONSKIN_NTOR_PERIOD_SECS_DEFAULT (60 * 60 * 6)
#define OVERLOAD_ONIONSKIN_NTOR_PERIOD_SECS_MIN 0
#define OVERLOAD_ONIONSKIN_NTOR_PERIOD_SECS_MAX INT32_MAX

/** Consensus parameter: Period, in seconds, over which we count the number of
 * ntor onionskins requests and how many were dropped. After that period, we
 * assess if we trigger an overload or not. */
static int32_t overload_onionskin_ntor_period_secs =
  OVERLOAD_ONIONSKIN_NTOR_PERIOD_SECS_DEFAULT;

/** Structure containing information for an assessment period of the onionskin
 * drop overload general signal.
 *
 * It is used to track, within a time period, how many requests we've gotten
 * and how many were dropped. The overload general signal is decided from these
 * depending on some consensus parameters. */
typedef struct {
  /** Total number of ntor onionskin requested for an assessment period. */
  uint64_t n_ntor_requested;

  /** Total number of dropped ntor onionskins for an assessment period. */
  uint64_t n_ntor_dropped;

  /** When is the next assessment time of the general overload for ntor
   * onionskin drop. Once this time is reached, all stats are reset and this
   * time is set to the next assessment time. */
  time_t next_assessment_time;
} overload_onionskin_assessment_t;

/** Keep track of the onionskin requests for an assessment period. */
static overload_onionskin_assessment_t overload_onionskin_assessment;

/**
 * We combine ntorv3 and ntor into the same stat, so we must
 * use this function to covert the cell type to a stat index.
 */
static inline uint16_t
onionskin_type_to_stat(uint16_t type)
{
  if (type == ONION_HANDSHAKE_TYPE_NTOR_V3) {
    return ONION_HANDSHAKE_TYPE_NTOR;
  }

  if (BUG(type > MAX_ONION_STAT_TYPE)) {
    return MAX_ONION_STAT_TYPE; // use ntor if out of range
  }

  return type;
}

/** Assess our ntor handshake statistics and decide if we need to emit a
 * general overload signal.
 *
 * Regardless of overloaded or not, if the assessment time period has passed,
 * the stats are reset back to 0 and the assessment time period updated.
 *
 * This is called when a ntor handshake is _requested_ because we want to avoid
 * to have an assymetric situation where requested counter is reset to 0 but
 * then a drop happens leading to the drop counter being incremented while the
 * requested counter is 0. */
static void
overload_general_onionskin_assessment(void)
{
  /* Initialize the time. Should be done once. */
  if (overload_onionskin_assessment.next_assessment_time == 0) {
    goto reset;
  }

  /* Not the time yet. */
  if (overload_onionskin_assessment.next_assessment_time > approx_time()) {
    goto done;
  }

  /* Make sure we have enough requests to be able to make a proper assessment.
   * We want to avoid 1 single request/drop to trigger an overload as we want
   * at least the number of requests to be above the scale of our fraction. */
  if (overload_onionskin_assessment.n_ntor_requested <
      OVERLOAD_ONIONSKIN_NTOR_PERCENT_SCALE) {
    goto done;
  }

  /* Lets see if we can signal a general overload. */
  double fraction = (double) overload_onionskin_assessment.n_ntor_dropped /
                    (double) overload_onionskin_assessment.n_ntor_requested;
  if (fraction >= overload_onionskin_ntor_fraction) {
    log_notice(LD_HIST, "General overload -> Ntor dropped (%" PRIu64 ") "
               "fraction %.4f%% is above threshold of %.4f%%",
               overload_onionskin_assessment.n_ntor_dropped,
               fraction * 100.0,
               overload_onionskin_ntor_fraction * 100.0);
    rep_hist_note_overload(OVERLOAD_GENERAL);
  }

 reset:
  /* Reset counters for the next period. */
  overload_onionskin_assessment.n_ntor_dropped = 0;
  overload_onionskin_assessment.n_ntor_requested = 0;
  overload_onionskin_assessment.next_assessment_time =
    approx_time() + overload_onionskin_ntor_period_secs;

 done:
  return;
}

/** A new onionskin (using the <b>type</b> handshake) has arrived. */
void
rep_hist_note_circuit_handshake_requested(uint16_t type)
{
  uint16_t stat = onionskin_type_to_stat(type);

  onion_handshakes_requested[stat]++;

  /* Only relays get to record requested onionskins. */
  if (stat == ONION_HANDSHAKE_TYPE_NTOR) {
    /* Assess if we've reached the overload general signal. */
    overload_general_onionskin_assessment();

    overload_onionskin_assessment.n_ntor_requested++;
  }
}

/** We've sent an onionskin (using the <b>type</b> handshake) to a
 * cpuworker. */
void
rep_hist_note_circuit_handshake_assigned(uint16_t type)
{
  onion_handshakes_assigned[onionskin_type_to_stat(type)]++;
  stats_n_onionskin_assigned[onionskin_type_to_stat(type)]++;
}

/** We've just drop an onionskin (using the <b>type</b> handshake) due to being
 * overloaded. */
void
rep_hist_note_circuit_handshake_dropped(uint16_t type)
{
  uint16_t stat = onionskin_type_to_stat(type);

  stats_n_onionskin_dropped[stat]++;

  /* Only relays get to record requested onionskins. */
  if (stat == ONION_HANDSHAKE_TYPE_NTOR) {
    /* Note the dropped ntor in the overload assessment object. */
    overload_onionskin_assessment.n_ntor_dropped++;
  }
}

/** Get the circuit handshake value that is requested. */
MOCK_IMPL(int,
rep_hist_get_circuit_handshake_requested, (uint16_t type))
{
  return onion_handshakes_requested[onionskin_type_to_stat(type)];
}

/** Get the circuit handshake value that is assigned. */
MOCK_IMPL(int,
rep_hist_get_circuit_handshake_assigned, (uint16_t type))
{
  return onion_handshakes_assigned[onionskin_type_to_stat(type)];
}

/** Get the total number of circuit handshake value that is assigned. */
MOCK_IMPL(uint64_t,
rep_hist_get_circuit_n_handshake_assigned, (uint16_t type))
{
  return stats_n_onionskin_assigned[onionskin_type_to_stat(type)];
}

/** Get the total number of circuit handshake value that is dropped. */
MOCK_IMPL(uint64_t,
rep_hist_get_circuit_n_handshake_dropped, (uint16_t type))
{
  return stats_n_onionskin_dropped[onionskin_type_to_stat(type)];
}

/** Log our onionskin statistics since the last time we were called. */
void
rep_hist_log_circuit_handshake_stats(time_t now)
{
  (void)now;
  log_notice(LD_HEARTBEAT, "Circuit handshake stats since last time: "
             "%d/%d TAP, %d/%d NTor.",
             onion_handshakes_assigned[ONION_HANDSHAKE_TYPE_TAP],
             onion_handshakes_requested[ONION_HANDSHAKE_TYPE_TAP],
             onion_handshakes_assigned[ONION_HANDSHAKE_TYPE_NTOR],
             onion_handshakes_requested[ONION_HANDSHAKE_TYPE_NTOR]);
  memset(onion_handshakes_assigned, 0, sizeof(onion_handshakes_assigned));
  memset(onion_handshakes_requested, 0, sizeof(onion_handshakes_requested));
}

/* Hidden service statistics section */

/** Start of the current hidden service stats interval or 0 if we're
 * not collecting hidden service statistics. */
static time_t start_of_hs_v2_stats_interval;

/** Our v2 statistics structure singleton. */
static hs_v2_stats_t *hs_v2_stats = NULL;

/** HSv2 stats */

/** Allocate, initialize and return an hs_v2_stats_t structure. */
static hs_v2_stats_t *
hs_v2_stats_new(void)
{
  hs_v2_stats_t *new_hs_v2_stats = tor_malloc_zero(sizeof(hs_v2_stats_t));

  return new_hs_v2_stats;
}

#define hs_v2_stats_free(val) \
  FREE_AND_NULL(hs_v2_stats_t, hs_v2_stats_free_, (val))

/** Free an hs_v2_stats_t structure. */
static void
hs_v2_stats_free_(hs_v2_stats_t *victim_hs_v2_stats)
{
  if (!victim_hs_v2_stats) {
    return;
  }
  tor_free(victim_hs_v2_stats);
}

/** Clear history of hidden service statistics and set the measurement
 * interval start to <b>now</b>. */
static void
rep_hist_reset_hs_v2_stats(time_t now)
{
  if (!hs_v2_stats) {
    hs_v2_stats = hs_v2_stats_new();
  }

  hs_v2_stats->rp_v2_relay_cells_seen = 0;

  start_of_hs_v2_stats_interval = now;
}

/*** HSv3 stats ******/

/** Start of the current hidden service stats interval or 0 if we're not
 *  collecting hidden service statistics.
 *
 *  This is particularly important for v3 statistics since this variable
 *  controls the start time of initial v3 stats collection. It's initialized by
 *  rep_hist_hs_stats_init() to the next time period start (i.e. 12:00UTC), and
 *  should_collect_v3_stats() ensures that functions that collect v3 stats do
 *  not do so sooner than that.
 *
 *  Collecting stats from 12:00UTC to 12:00UTC is extremely important for v3
 *  stats because rep_hist_hsdir_stored_maybe_new_v3_onion() uses the blinded
 *  key of each onion service as its double-counting index. Onion services
 *  rotate their descriptor at around 00:00UTC which means that their blinded
 *  key also changes around that time. However the precise time that onion
 *  services rotate their descriptors is actually when they fetch a new
 *  00:00UTC consensus and that happens at a random time (e.g. it can even
 *  happen at 02:00UTC). This means that if we started keeping v3 stats at
 *  around 00:00UTC we wouldn't be able to tell when onion services change
 *  their blinded key and hence we would double count an unpredictable amount
 *  of them (for example, if an onion service fetches the 00:00UTC consensus at
 *  01:00UTC it would upload to its old HSDir at 00:45UTC, and then to a
 *  different HSDir at 01:50UTC).
 *
 *  For this reason, we start collecting statistics at 12:00UTC. This way we
 *  know that by the time we stop collecting statistics for that time period 24
 *  hours later, all the onion services have switched to their new blinded
 *  key. This way we can predict much better how much double counting has been
 *  performed.
 */
static time_t start_of_hs_v3_stats_interval;

/** Our v3 statistics structure singleton. */
static hs_v3_stats_t *hs_v3_stats = NULL;

/** Allocate, initialize and return an hs_v3_stats_t structure. */
static hs_v3_stats_t *
hs_v3_stats_new(void)
{
  hs_v3_stats_t *new_hs_v3_stats = tor_malloc_zero(sizeof(hs_v3_stats_t));
  new_hs_v3_stats->v3_onions_seen_this_period = digest256map_new();

  return new_hs_v3_stats;
}

#define hs_v3_stats_free(val) \
  FREE_AND_NULL(hs_v3_stats_t, hs_v3_stats_free_, (val))

/** Free an hs_v3_stats_t structure. */
static void
hs_v3_stats_free_(hs_v3_stats_t *victim_hs_v3_stats)
{
  if (!victim_hs_v3_stats) {
    return;
  }

  digest256map_free(victim_hs_v3_stats->v3_onions_seen_this_period, NULL);
  tor_free(victim_hs_v3_stats);
}

/** Clear history of hidden service statistics and set the measurement
 * interval start to <b>now</b>. */
static void
rep_hist_reset_hs_v3_stats(time_t now)
{
  if (!hs_v3_stats) {
    hs_v3_stats = hs_v3_stats_new();
  }

  digest256map_free(hs_v3_stats->v3_onions_seen_this_period, NULL);
  hs_v3_stats->v3_onions_seen_this_period = digest256map_new();

  hs_v3_stats->rp_v3_relay_cells_seen = 0;

  start_of_hs_v3_stats_interval = now;
}

/** Return true if it's a good time to collect v3 stats.
 *
 *  v3 stats have a strict stats collection period (from 12:00UTC to 12:00UTC
 *  on the real network). We don't want to collect statistics if (for example)
 *  we just booted and it's 03:00UTC; we will wait until 12:00UTC before we
 *  start collecting statistics to make sure that the final result represents
 *  the whole collection period. This behavior is controlled by
 *  rep_hist_hs_stats_init().
 */
MOCK_IMPL(STATIC bool,
should_collect_v3_stats,(void))
{
  return start_of_hs_v3_stats_interval <= approx_time();
}

/** We just received a new descriptor with <b>blinded_key</b>. See if we've
 * seen this blinded key before, and if not add it to the stats.  */
void
rep_hist_hsdir_stored_maybe_new_v3_onion(const uint8_t *blinded_key)
{
  /* Return early if we don't collect HSv3 stats, or if it's not yet the time
   * to collect them. */
  if (!hs_v3_stats || !should_collect_v3_stats()) {
    return;
  }

  bool seen_before =
    !!digest256map_get(hs_v3_stats->v3_onions_seen_this_period,
                       blinded_key);

  log_info(LD_GENERAL, "Considering v3 descriptor with %s (%sseen before)",
           safe_str(hex_str((char*)blinded_key, 32)),
           seen_before ? "" : "not ");

  /* Count it if we haven't seen it before. */
  if (!seen_before) {
    digest256map_set(hs_v3_stats->v3_onions_seen_this_period,
                  blinded_key, (void*)(uintptr_t)1);
  }
}

/** We saw a new HS relay cell: count it!
 *  If <b>is_v2</b> is set then it's a v2 RP cell, otherwise it's a v3. */
void
rep_hist_seen_new_rp_cell(bool is_v2)
{
  log_debug(LD_GENERAL, "New RP cell (%d)", is_v2);

  if (is_v2 && hs_v2_stats) {
    hs_v2_stats->rp_v2_relay_cells_seen++;
  } else if (!is_v2 && hs_v3_stats && should_collect_v3_stats()) {
    hs_v3_stats->rp_v3_relay_cells_seen++;
  }
}

/** Generic HS stats code */

/** Initialize v2 and v3 hidden service statistics. */
void
rep_hist_hs_stats_init(time_t now)
{
  if (!hs_v2_stats) {
    hs_v2_stats = hs_v2_stats_new();
  }

  /* Start collecting v2 stats straight away */
  start_of_hs_v2_stats_interval = now;

  if (!hs_v3_stats) {
    hs_v3_stats = hs_v3_stats_new();
  }

  /* Start collecting v3 stats at the next 12:00 UTC */
  start_of_hs_v3_stats_interval = hs_get_start_time_of_next_time_period(now);
}

/** Stop collecting hidden service stats in a way that we can re-start
 * doing so in rep_hist_buffer_stats_init(). */
void
rep_hist_hs_stats_term(void)
{
  rep_hist_reset_hs_v2_stats(0);
  rep_hist_reset_hs_v3_stats(0);
}

/** Stats reporting code */

/* The number of cells that are supposed to be hidden from the adversary
 * by adding noise from the Laplace distribution.  This value, divided by
 * EPSILON, is Laplace parameter b. It must be greater than 0. */
#define REND_CELLS_DELTA_F 2048
/* Security parameter for obfuscating number of cells with a value between
 * ]0.0, 1.0]. Smaller values obfuscate observations more, but at the same
 * time make statistics less usable. */
#define REND_CELLS_EPSILON 0.3
/* The number of cells that are supposed to be hidden from the adversary
 * by rounding up to the next multiple of this number. */
#define REND_CELLS_BIN_SIZE 1024
/* The number of service identities that are supposed to be hidden from the
 * adversary by adding noise from the Laplace distribution. This value,
 * divided by EPSILON, is Laplace parameter b. It must be greater than 0. */
#define ONIONS_SEEN_DELTA_F 8
/* Security parameter for obfuscating number of service identities with a
 * value between ]0.0, 1.0]. Smaller values obfuscate observations more, but
 * at the same time make statistics less usable. */
#define ONIONS_SEEN_EPSILON 0.3
/* The number of service identities that are supposed to be hidden from
 * the adversary by rounding up to the next multiple of this number. */
#define ONIONS_SEEN_BIN_SIZE 8

/** Allocate and return a string containing hidden service stats that
 *  are meant to be placed in the extra-info descriptor.
 *
 *  Function works for both v2 and v3 stats depending on <b>is_v3</b>. */
STATIC char *
rep_hist_format_hs_stats(time_t now, bool is_v3)
{
  char t[ISO_TIME_LEN+1];
  char *hs_stats_string;
  int64_t obfuscated_onions_seen, obfuscated_cells_seen;

  uint64_t rp_cells_seen = is_v3 ?
    hs_v3_stats->rp_v3_relay_cells_seen : hs_v2_stats->rp_v2_relay_cells_seen;
  size_t onions_seen = is_v3 ?
    digest256map_size(hs_v3_stats->v3_onions_seen_this_period) : 0;
  time_t start_of_hs_stats_interval = is_v3 ?
    start_of_hs_v3_stats_interval : start_of_hs_v2_stats_interval;

  uint64_t rounded_cells_seen
    = round_uint64_to_next_multiple_of(rp_cells_seen, REND_CELLS_BIN_SIZE);
  rounded_cells_seen = MIN(rounded_cells_seen, INT64_MAX);
  obfuscated_cells_seen = add_laplace_noise((int64_t)rounded_cells_seen,
                          crypto_rand_double(),
                          REND_CELLS_DELTA_F, REND_CELLS_EPSILON);

  uint64_t rounded_onions_seen =
    round_uint64_to_next_multiple_of(onions_seen, ONIONS_SEEN_BIN_SIZE);
  rounded_onions_seen = MIN(rounded_onions_seen, INT64_MAX);
  obfuscated_onions_seen = add_laplace_noise((int64_t)rounded_onions_seen,
                           crypto_rand_double(), ONIONS_SEEN_DELTA_F,
                           ONIONS_SEEN_EPSILON);

  format_iso_time(t, now);
  tor_asprintf(&hs_stats_string, "%s %s (%u s)\n"
               "%s %"PRId64" delta_f=%d epsilon=%.2f bin_size=%d\n"
               "%s %"PRId64" delta_f=%d epsilon=%.2f bin_size=%d\n",
               is_v3 ? "hidserv-v3-stats-end" : "hidserv-stats-end",
               t, (unsigned) (now - start_of_hs_stats_interval),
               is_v3 ?
                "hidserv-rend-v3-relayed-cells" : "hidserv-rend-relayed-cells",
               obfuscated_cells_seen, REND_CELLS_DELTA_F,
               REND_CELLS_EPSILON, REND_CELLS_BIN_SIZE,
               is_v3 ? "hidserv-dir-v3-onions-seen" :"hidserv-dir-onions-seen",
               obfuscated_onions_seen, ONIONS_SEEN_DELTA_F,
               ONIONS_SEEN_EPSILON, ONIONS_SEEN_BIN_SIZE);

  return hs_stats_string;
}

/** If 24 hours have passed since the beginning of the current HS
 * stats period, write buffer stats to $DATADIR/stats/hidserv-v3-stats
 * (possibly overwriting an existing file) and reset counters.  Return
 * when we would next want to write buffer stats or 0 if we never want to
 * write. Function works for both v2 and v3 stats depending on <b>is_v3</b>.
 */
time_t
rep_hist_hs_stats_write(time_t now, bool is_v3)
{
  char *str = NULL;

  time_t start_of_hs_stats_interval = is_v3 ?
    start_of_hs_v3_stats_interval : start_of_hs_v2_stats_interval;

  if (!start_of_hs_stats_interval) {
    return 0; /* Not initialized. */
  }

  if (start_of_hs_stats_interval + WRITE_STATS_INTERVAL > now) {
    goto done; /* Not ready to write */
  }

  /* Generate history string. */
  str = rep_hist_format_hs_stats(now, is_v3);

  /* Reset HS history. */
  if (is_v3) {
    rep_hist_reset_hs_v3_stats(now);
  } else {
    rep_hist_reset_hs_v2_stats(now);
  }

  /* Try to write to disk. */
  if (!check_or_create_data_subdir("stats")) {
    write_to_data_subdir("stats",
                         is_v3 ? "hidserv-v3-stats" : "hidserv-stats",
                         str, "hidden service stats");
  }

 done:
  tor_free(str);
  return start_of_hs_stats_interval + WRITE_STATS_INTERVAL;
}

static uint64_t link_proto_count[MAX_LINK_PROTO+1][2];

/** Note that we negotiated link protocol version <b>link_proto</b>, on
 * a connection that started here iff <b>started_here</b> is true.
 */
void
rep_hist_note_negotiated_link_proto(unsigned link_proto, int started_here)
{
  started_here = !!started_here; /* force to 0 or 1 */
  if (link_proto > MAX_LINK_PROTO) {
    log_warn(LD_BUG, "Can't log link protocol %u", link_proto);
    return;
  }

  link_proto_count[link_proto][started_here]++;
}

/**
 * Update the maximum count of total pending channel padding timers
 * in this period.
 */
void
rep_hist_padding_count_timers(uint64_t num_timers)
{
  if (num_timers > padding_current.maximum_chanpad_timers) {
    padding_current.maximum_chanpad_timers = num_timers;
  }
}

/**
 * Count a cell that we sent for padding overhead statistics.
 *
 * RELAY_COMMAND_DROP and CELL_PADDING are accounted separately. Both should be
 * counted for PADDING_TYPE_TOTAL.
 */
void
rep_hist_padding_count_write(padding_type_t type)
{
  switch (type) {
    case PADDING_TYPE_DROP:
      padding_current.write_drop_cell_count++;
      break;
    case PADDING_TYPE_CELL:
      padding_current.write_pad_cell_count++;
      break;
    case PADDING_TYPE_TOTAL:
      padding_current.write_cell_count++;
      break;
    case PADDING_TYPE_ENABLED_TOTAL:
      padding_current.enabled_write_cell_count++;
      break;
    case PADDING_TYPE_ENABLED_CELL:
      padding_current.enabled_write_pad_cell_count++;
      break;
  }
}

/**
 * Count a cell that we've received for padding overhead statistics.
 *
 * RELAY_COMMAND_DROP and CELL_PADDING are accounted separately. Both should be
 * counted for PADDING_TYPE_TOTAL.
 */
void
rep_hist_padding_count_read(padding_type_t type)
{
  switch (type) {
    case PADDING_TYPE_DROP:
      padding_current.read_drop_cell_count++;
      break;
    case PADDING_TYPE_CELL:
      padding_current.read_pad_cell_count++;
      break;
    case PADDING_TYPE_TOTAL:
      padding_current.read_cell_count++;
      break;
    case PADDING_TYPE_ENABLED_TOTAL:
      padding_current.enabled_read_cell_count++;
      break;
    case PADDING_TYPE_ENABLED_CELL:
      padding_current.enabled_read_pad_cell_count++;
      break;
  }
}

/**
 * Reset our current padding statistics. Called once every 24 hours.
 */
void
rep_hist_reset_padding_counts(void)
{
  memset(&padding_current, 0, sizeof(padding_current));
}

/**
 * Copy our current cell counts into a structure for listing in our
 * extra-info descriptor. Also perform appropriate rounding and redaction.
 *
 * This function is called once every 24 hours.
 */
#define MIN_CELL_COUNTS_TO_PUBLISH 1
#define ROUND_CELL_COUNTS_TO 10000
void
rep_hist_prep_published_padding_counts(time_t now)
{
  memcpy(&padding_published, &padding_current, sizeof(padding_published));

  if (padding_published.read_cell_count < MIN_CELL_COUNTS_TO_PUBLISH ||
      padding_published.write_cell_count < MIN_CELL_COUNTS_TO_PUBLISH) {
    memset(&padding_published, 0, sizeof(padding_published));
    return;
  }

  format_iso_time(padding_published.first_published_at, now);
#define ROUND_AND_SET_COUNT(x) (x) = round_uint64_to_next_multiple_of((x), \
                                      ROUND_CELL_COUNTS_TO)
  ROUND_AND_SET_COUNT(padding_published.read_pad_cell_count);
  ROUND_AND_SET_COUNT(padding_published.write_pad_cell_count);
  ROUND_AND_SET_COUNT(padding_published.read_drop_cell_count);
  ROUND_AND_SET_COUNT(padding_published.write_drop_cell_count);
  ROUND_AND_SET_COUNT(padding_published.write_cell_count);
  ROUND_AND_SET_COUNT(padding_published.read_cell_count);
  ROUND_AND_SET_COUNT(padding_published.enabled_read_cell_count);
  ROUND_AND_SET_COUNT(padding_published.enabled_read_pad_cell_count);
  ROUND_AND_SET_COUNT(padding_published.enabled_write_cell_count);
  ROUND_AND_SET_COUNT(padding_published.enabled_write_pad_cell_count);
#undef ROUND_AND_SET_COUNT
}

/**
 * Returns an allocated string for extra-info documents for publishing
 * padding statistics from the last 24 hour interval.
 */
char *
rep_hist_get_padding_count_lines(void)
{
  char *result = NULL;

  if (!padding_published.read_cell_count ||
          !padding_published.write_cell_count) {
    return NULL;
  }

  tor_asprintf(&result, "padding-counts %s (%d s)"
                        " bin-size=%"PRIu64
                        " write-drop=%"PRIu64
                        " write-pad=%"PRIu64
                        " write-total=%"PRIu64
                        " read-drop=%"PRIu64
                        " read-pad=%"PRIu64
                        " read-total=%"PRIu64
                        " enabled-read-pad=%"PRIu64
                        " enabled-read-total=%"PRIu64
                        " enabled-write-pad=%"PRIu64
                        " enabled-write-total=%"PRIu64
                        " max-chanpad-timers=%"PRIu64
                        "\n",
               padding_published.first_published_at,
               REPHIST_CELL_PADDING_COUNTS_INTERVAL,
               (uint64_t)ROUND_CELL_COUNTS_TO,
               (padding_published.write_drop_cell_count),
               (padding_published.write_pad_cell_count),
               (padding_published.write_cell_count),
               (padding_published.read_drop_cell_count),
               (padding_published.read_pad_cell_count),
               (padding_published.read_cell_count),
               (padding_published.enabled_read_pad_cell_count),
               (padding_published.enabled_read_cell_count),
               (padding_published.enabled_write_pad_cell_count),
               (padding_published.enabled_write_cell_count),
               (padding_published.maximum_chanpad_timers)
               );

  return result;
}

/** Log a heartbeat message explaining how many connections of each link
 * protocol version we have used.
 */
void
rep_hist_log_link_protocol_counts(void)
{
  smartlist_t *lines = smartlist_new();

  for (int i = 1; i <= MAX_LINK_PROTO; i++) {
     char *line = NULL;
     tor_asprintf(&line, "initiated %"PRIu64" and received "
                  "%"PRIu64" v%d connections", link_proto_count[i][1],
                  link_proto_count[i][0], i);
     smartlist_add(lines, line);
  }

  char *log_line = smartlist_join_strings(lines, "; ", 0, NULL);

  log_notice(LD_HEARTBEAT, "Since startup we %s.", log_line);

  SMARTLIST_FOREACH(lines, char *, s, tor_free(s));
  smartlist_free(lines);
  tor_free(log_line);
}

/** Free all storage held by the OR/link history caches, by the
 * bandwidth history arrays, by the port history, or by statistics . */
void
rep_hist_free_all(void)
{
  hs_v2_stats_free(hs_v2_stats);
  hs_v3_stats_free(hs_v3_stats);
  digestmap_free(history_map, free_or_history);

  tor_free(exit_bytes_read);
  tor_free(exit_bytes_written);
  tor_free(exit_streams);
  predicted_ports_free_all();
  conn_stats_free_all();

  if (circuits_for_buffer_stats) {
    SMARTLIST_FOREACH(circuits_for_buffer_stats, circ_buffer_stats_t *, s,
                      tor_free(s));
    smartlist_free(circuits_for_buffer_stats);
    circuits_for_buffer_stats = NULL;
  }
  rep_hist_desc_stats_term();
  total_descriptor_downloads = 0;

  tor_assert_nonfatal(rephist_total_alloc == 0);
  tor_assert_nonfatal_once(rephist_total_num == 0);
}

/** Called just before the consensus will be replaced. Update the consensus
 * parameters in case they changed. */
void
rep_hist_consensus_has_changed(const networkstatus_t *ns)
{
  overload_onionskin_ntor_fraction =
    networkstatus_get_param(ns, "overload_onionskin_ntor_scale_percent",
                            OVERLOAD_ONIONSKIN_NTOR_PERCENT_DEFAULT,
                            OVERLOAD_ONIONSKIN_NTOR_PERCENT_MIN,
                            OVERLOAD_ONIONSKIN_NTOR_PERCENT_MAX) /
    OVERLOAD_ONIONSKIN_NTOR_PERCENT_SCALE / 100.0;

  overload_onionskin_ntor_period_secs =
    networkstatus_get_param(ns, "overload_onionskin_ntor_period_secs",
                            OVERLOAD_ONIONSKIN_NTOR_PERIOD_SECS_DEFAULT,
                            OVERLOAD_ONIONSKIN_NTOR_PERIOD_SECS_MIN,
                            OVERLOAD_ONIONSKIN_NTOR_PERIOD_SECS_MAX);
}

#ifdef TOR_UNIT_TESTS
/* only exists for unit tests: get HSv2 stats object */
const hs_v2_stats_t *
rep_hist_get_hs_v2_stats(void)
{
  return hs_v2_stats;
}

/* only exists for unit tests: get HSv2 stats object */
const hs_v3_stats_t *
rep_hist_get_hs_v3_stats(void)
{
  return hs_v3_stats;
}
#endif /* defined(TOR_UNIT_TESTS) */
