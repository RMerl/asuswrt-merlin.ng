/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file connstats.c
 * @brief Count bidirectional vs one-way connections.
 *
 * Connection statistics, use to track one-way and bidirectional connections.
 *
 * Note that this code counts concurrent connections in each
 * BIDI_INTERVAL-second interval, not total connections.  It can tell you what
 * fraction of connections are bidirectional at each time, not necessarily
 * what number are bidirectional.
 **/

#include "orconfig.h"
#include "core/or/or.h"
#include "feature/stats/connstats.h"
#include "app/config/config.h"

/** Start of the current connection stats interval or 0 if we're not
 * collecting connection statistics. */
static time_t start_of_conn_stats_interval;

/** Initialize connection stats. */
void
conn_stats_init(time_t now)
{
  start_of_conn_stats_interval = now;
}

/** Count connections on which we read and wrote less than this many bytes
 * as "below threshold." */
#define BIDI_THRESHOLD 20480

/** Count connections that we read or wrote at least this factor as many
 * bytes from/to than we wrote or read to/from as mostly reading or
 * writing. */
#define BIDI_FACTOR 10

/** Interval length in seconds for considering read and written bytes for
 * connection stats. */
#define BIDI_INTERVAL 10

/** Start of next BIDI_INTERVAL second interval. */
static time_t bidi_next_interval = 0;

/** A single grouped set of connection type counts. */
typedef struct conn_counts_t {
  /** Number of connections that we read and wrote less than BIDI_THRESHOLD
   * bytes from/to in BIDI_INTERVAL seconds. */
  uint32_t below_threshold;

  /** Number of connections that we read at least BIDI_FACTOR times more
   * bytes from than we wrote to in BIDI_INTERVAL seconds. */
  uint32_t mostly_read;

  /** Number of connections that we wrote at least BIDI_FACTOR times more
   * bytes to than we read from in BIDI_INTERVAL seconds. */
  uint32_t mostly_written;

  /** Number of connections that we read and wrote at least BIDI_THRESHOLD
   * bytes from/to, but not BIDI_FACTOR times more in either direction in
   * BIDI_INTERVAL seconds. */
  uint32_t both_read_and_written;
} conn_counts_t ;

/** A collection of connection counts, over all OR connections. */
static conn_counts_t counts;
/** A collection of connection counts, over IPv6 OR connections only. */
static conn_counts_t counts_ipv6;

/** Entry in a map from connection ID to the number of read and written
 * bytes on this connection in a BIDI_INTERVAL second interval. */
typedef struct bidi_map_entry_t {
  HT_ENTRY(bidi_map_entry_t) node;
  uint64_t conn_id; /**< Connection ID */
  size_t read; /**< Number of read bytes */
  size_t written; /**< Number of written bytes */
  bool is_ipv6; /**< True if this is an IPv6 connection */
} bidi_map_entry_t;

/** Map of OR connections together with the number of read and written
 * bytes in the current BIDI_INTERVAL second interval. */
static HT_HEAD(bidimap, bidi_map_entry_t) bidi_map =
     HT_INITIALIZER();

/** Hashtable helper: return true if @a a and @a b have the same key. */
static int
bidi_map_ent_eq(const bidi_map_entry_t *a, const bidi_map_entry_t *b)
{
  return a->conn_id == b->conn_id;
}

/** Hashtable helper: compute a digest for the key of @a entry. */
static unsigned
bidi_map_ent_hash(const bidi_map_entry_t *entry)
{
  return (unsigned) entry->conn_id;
}

HT_PROTOTYPE(bidimap, bidi_map_entry_t, node, bidi_map_ent_hash,
             bidi_map_ent_eq);
HT_GENERATE2(bidimap, bidi_map_entry_t, node, bidi_map_ent_hash,
             bidi_map_ent_eq, 0.6, tor_reallocarray_, tor_free_);

/** Release all storage held in connstats.c */
void
conn_stats_free_all(void)
{
  bidi_map_entry_t **ptr, **next, *ent;
  for (ptr = HT_START(bidimap, &bidi_map); ptr; ptr = next) {
    ent = *ptr;
    next = HT_NEXT_RMV(bidimap, &bidi_map, ptr);
    tor_free(ent);
  }
  HT_CLEAR(bidimap, &bidi_map);
}

/** Reset counters for conn statistics. */
void
conn_stats_reset(time_t now)
{
  start_of_conn_stats_interval = now;
  memset(&counts, 0, sizeof(counts));
  memset(&counts_ipv6, 0, sizeof(counts_ipv6));
  conn_stats_free_all();
}

/** Stop collecting connection stats in a way that we can re-start doing
 * so in conn_stats_init(). */
void
conn_stats_terminate(void)
{
  conn_stats_reset(0);
}

/**
 * Record a single entry @a ent in the counts structure @a cnt.
 */
static void
add_entry_to_count(conn_counts_t *cnt, const bidi_map_entry_t *ent)
{
  if (ent->read + ent->written < BIDI_THRESHOLD)
    cnt->below_threshold++;
  else if (ent->read >= ent->written * BIDI_FACTOR)
    cnt->mostly_read++;
  else if (ent->written >= ent->read * BIDI_FACTOR)
    cnt->mostly_written++;
  else
    cnt->both_read_and_written++;
}

/**
 * Count all the connection information we've received during the current
 * period in 'bidimap', and store that information in the appropriate count
 * structures.
 **/
static void
collect_period_statistics(void)
{
  bidi_map_entry_t **ptr, **next, *ent;
  for (ptr = HT_START(bidimap, &bidi_map); ptr; ptr = next) {
    ent = *ptr;
    add_entry_to_count(&counts, ent);
    if (ent->is_ipv6)
      add_entry_to_count(&counts_ipv6, ent);
    next = HT_NEXT_RMV(bidimap, &bidi_map, ptr);
    tor_free(ent);
  }
  log_info(LD_GENERAL, "%d below threshold, %d mostly read, "
           "%d mostly written, %d both read and written.",
           counts.below_threshold, counts.mostly_read, counts.mostly_written,
           counts.both_read_and_written);
}

/** We read <b>num_read</b> bytes and wrote <b>num_written</b> from/to OR
 * connection <b>conn_id</b> in second <b>when</b>. If this is the first
 * observation in a new interval, sum up the last observations. Add bytes
 * for this connection. */
void
conn_stats_note_or_conn_bytes(uint64_t conn_id, size_t num_read,
                              size_t num_written, time_t when,
                              bool is_ipv6)
{
  if (!start_of_conn_stats_interval)
    return;
  /* Initialize */
  if (bidi_next_interval == 0)
    bidi_next_interval = when + BIDI_INTERVAL;
  /* Sum up last period's statistics */
  if (when >= bidi_next_interval) {
    collect_period_statistics();
    while (when >= bidi_next_interval)
      bidi_next_interval += BIDI_INTERVAL;
  }
  /* Add this connection's bytes. */
  if (num_read > 0 || num_written > 0) {
    bidi_map_entry_t *entry, lookup;
    lookup.conn_id = conn_id;
    entry = HT_FIND(bidimap, &bidi_map, &lookup);
    if (entry) {
      entry->written += num_written;
      entry->read += num_read;
      entry->is_ipv6 |= is_ipv6;
    } else {
      entry = tor_malloc_zero(sizeof(bidi_map_entry_t));
      entry->conn_id = conn_id;
      entry->written = num_written;
      entry->read = num_read;
      entry->is_ipv6 = is_ipv6;
      HT_INSERT(bidimap, &bidi_map, entry);
    }
  }
}

/** Return a newly allocated string containing the connection statistics
 * until <b>now</b>, or NULL if we're not collecting conn stats. Caller must
 * ensure start_of_conn_stats_interval is in the past. */
char *
conn_stats_format(time_t now)
{
  char *result, written_at[ISO_TIME_LEN+1];

  if (!start_of_conn_stats_interval)
    return NULL; /* Not initialized. */

  tor_assert(now >= start_of_conn_stats_interval);

  format_iso_time(written_at, now);
  tor_asprintf(&result,
               "conn-bi-direct %s (%d s) "
                    "%"PRIu32",%"PRIu32",%"PRIu32",%"PRIu32"\n"
               "ipv6-conn-bi-direct %s (%d s) "
                    "%"PRIu32",%"PRIu32",%"PRIu32",%"PRIu32"\n",
               written_at,
               (unsigned) (now - start_of_conn_stats_interval),
               counts.below_threshold,
               counts.mostly_read,
               counts.mostly_written,
               counts.both_read_and_written,
               written_at,
               (unsigned) (now - start_of_conn_stats_interval),
               counts_ipv6.below_threshold,
               counts_ipv6.mostly_read,
               counts_ipv6.mostly_written,
               counts_ipv6.both_read_and_written);

  return result;
}

/** If 24 hours have passed since the beginning of the current conn stats
 * period, write conn stats to $DATADIR/stats/conn-stats (possibly
 * overwriting an existing file) and reset counters.  Return when we would
 * next want to write conn stats or 0 if we never want to write. */
time_t
conn_stats_save(time_t now)
{
  char *str = NULL;

  if (!start_of_conn_stats_interval)
    return 0; /* Not initialized. */
  if (start_of_conn_stats_interval + WRITE_STATS_INTERVAL > now)
    goto done; /* Not ready to write */

  /* Generate history string. */
  str = conn_stats_format(now);

  /* Reset counters. */
  conn_stats_reset(now);

  /* Try to write to disk. */
  if (!check_or_create_data_subdir("stats")) {
    write_to_data_subdir("stats", "conn-stats", str, "connection statistics");
  }

 done:
  tor_free(str);
  return start_of_conn_stats_interval + WRITE_STATS_INTERVAL;
}
