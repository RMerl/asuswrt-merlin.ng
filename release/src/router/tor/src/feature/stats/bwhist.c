/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file bwhist.c
 * @brief Tracking for relay bandwidth history
 *
 * This module handles bandwidth usage history, used by relays to
 * self-report how much bandwidth they've used for different
 * purposes over last day or so, in order to generate the
 * {dirreq-,}{read,write}-history lines in that they publish.
 **/

#define BWHIST_PRIVATE
#include "orconfig.h"
#include "core/or/or.h"
#include "feature/stats/bwhist.h"

#include "app/config/config.h"
#include "app/config/statefile.h"
#include "feature/relay/routermode.h"

#include "feature/stats/bw_array_st.h"
#include "app/config/or_state_st.h"
#include "app/config/or_options_st.h"

/** Shift the current period of b forward by one. */
STATIC void
commit_max(bw_array_t *b)
{
  /* Store total from current period. */
  b->totals[b->next_max_idx] = b->total_in_period;
  /* Store maximum from current period. */
  b->maxima[b->next_max_idx++] = b->max_total;
  /* Advance next_period and next_max_idx */
  b->next_period += NUM_SECS_BW_SUM_INTERVAL;
  if (b->next_max_idx == NUM_TOTALS)
    b->next_max_idx = 0;
  if (b->num_maxes_set < NUM_TOTALS)
    ++b->num_maxes_set;
  /* Reset max_total. */
  b->max_total = 0;
  /* Reset total_in_period. */
  b->total_in_period = 0;
}

/** Shift the current observation time of <b>b</b> forward by one second. */
STATIC void
advance_obs(bw_array_t *b)
{
  int nextidx;
  uint64_t total;

  /* Calculate the total bandwidth for the last NUM_SECS_ROLLING_MEASURE
   * seconds; adjust max_total as needed.*/
  total = b->total_obs + b->obs[b->cur_obs_idx];
  if (total > b->max_total)
    b->max_total = total;

  nextidx = b->cur_obs_idx+1;
  if (nextidx == NUM_SECS_ROLLING_MEASURE)
    nextidx = 0;

  b->total_obs = total - b->obs[nextidx];
  b->obs[nextidx]=0;
  b->cur_obs_idx = nextidx;

  if (++b->cur_obs_time >= b->next_period)
    commit_max(b);
}

/** Add <b>n</b> bytes to the number of bytes in <b>b</b> for second
 * <b>when</b>. */
STATIC void
add_obs(bw_array_t *b, time_t when, uint64_t n)
{
  if (when < b->cur_obs_time)
    return; /* Don't record data in the past. */

  /* If we're currently adding observations for an earlier second than
   * 'when', advance b->cur_obs_time and b->cur_obs_idx by an
   * appropriate number of seconds, and do all the other housekeeping. */
  while (when > b->cur_obs_time) {
    /* Doing this one second at a time is potentially inefficient, if we start
       with a state file that is very old.  Fortunately, it doesn't seem to
       show up in profiles, so we can just ignore it for now. */
    advance_obs(b);
  }

  b->obs[b->cur_obs_idx] += n;
  b->total_in_period += n;
}

/** Allocate, initialize, and return a new bw_array. */
STATIC bw_array_t *
bw_array_new(void)
{
  bw_array_t *b;
  time_t start;
  b = tor_malloc_zero(sizeof(bw_array_t));
  start = time(NULL);
  b->cur_obs_time = start;
  b->next_period = start + NUM_SECS_BW_SUM_INTERVAL;
  return b;
}

/** Free storage held by bandwidth array <b>b</b>. */
STATIC void
bw_array_free_(bw_array_t *b)
{
  if (!b) {
    return;
  }

  tor_free(b);
}

/** Recent history of bandwidth observations for (all) read operations. */
static bw_array_t *read_array = NULL;
/** Recent history of bandwidth observations for IPv6 read operations. */
static bw_array_t *read_array_ipv6 = NULL;
/** Recent history of bandwidth observations for (all) write operations. */
STATIC bw_array_t *write_array = NULL;
/** Recent history of bandwidth observations for IPv6 write operations. */
static bw_array_t *write_array_ipv6 = NULL;
/** Recent history of bandwidth observations for read operations for the
    directory protocol. */
static bw_array_t *dir_read_array = NULL;
/** Recent history of bandwidth observations for write operations for the
    directory protocol. */
static bw_array_t *dir_write_array = NULL;

/** Set up structures for bandwidth history, clearing them if they already
 * exist. */
void
bwhist_init(void)
{
  bw_array_free(read_array);
  bw_array_free(read_array_ipv6);
  bw_array_free(write_array);
  bw_array_free(write_array_ipv6);
  bw_array_free(dir_read_array);
  bw_array_free(dir_write_array);

  read_array = bw_array_new();
  read_array_ipv6 = bw_array_new();
  write_array = bw_array_new();
  write_array_ipv6 = bw_array_new();
  dir_read_array = bw_array_new();
  dir_write_array = bw_array_new();
}

/** Remember that we read <b>num_bytes</b> bytes in second <b>when</b>.
 *
 * Add num_bytes to the current running total for <b>when</b>.
 *
 * <b>when</b> can go back to time, but it's safe to ignore calls
 * earlier than the latest <b>when</b> you've heard of.
 */
void
bwhist_note_bytes_written(uint64_t num_bytes, time_t when, bool ipv6)
{
/* Maybe a circular array for recent seconds, and step to a new point
 * every time a new second shows up. Or simpler is to just to have
 * a normal array and push down each item every second; it's short.
 */
/* When a new second has rolled over, compute the sum of the bytes we've
 * seen over when-1 to when-1-NUM_SECS_ROLLING_MEASURE, and stick it
 * somewhere. See bwhist_bandwidth_assess() below.
 */
  add_obs(write_array, when, num_bytes);
  if (ipv6)
    add_obs(write_array_ipv6, when, num_bytes);
}

/** Remember that we wrote <b>num_bytes</b> bytes in second <b>when</b>.
 * (like bwhist_note_bytes_written() above)
 */
void
bwhist_note_bytes_read(uint64_t num_bytes, time_t when, bool ipv6)
{
/* if we're smart, we can make this func and the one above share code */
  add_obs(read_array, when, num_bytes);
  if (ipv6)
    add_obs(read_array_ipv6, when, num_bytes);
}

/** Remember that we wrote <b>num_bytes</b> directory bytes in second
 * <b>when</b>. (like bwhist_note_bytes_written() above)
 */
void
bwhist_note_dir_bytes_written(uint64_t num_bytes, time_t when)
{
  add_obs(dir_write_array, when, num_bytes);
}

/** Remember that we read <b>num_bytes</b> directory bytes in second
 * <b>when</b>. (like bwhist_note_bytes_written() above)
 */
void
bwhist_note_dir_bytes_read(uint64_t num_bytes, time_t when)
{
  add_obs(dir_read_array, when, num_bytes);
}

/** Helper: Return the largest value in b->maxima.  (This is equal to the
 * most bandwidth used in any NUM_SECS_ROLLING_MEASURE period for the last
 * NUM_SECS_BW_SUM_IS_VALID seconds.)
 */
STATIC uint64_t
find_largest_max(bw_array_t *b)
{
  int i;
  uint64_t max;
  max=0;
  for (i=0; i<NUM_TOTALS; ++i) {
    if (b->maxima[i]>max)
      max = b->maxima[i];
  }
  return max;
}

/** Find the largest sums in the past NUM_SECS_BW_SUM_IS_VALID (roughly)
 * seconds. Find one sum for reading and one for writing. They don't have
 * to be at the same time.
 *
 * Return the smaller of these sums, divided by NUM_SECS_ROLLING_MEASURE.
 */
MOCK_IMPL(int,
bwhist_bandwidth_assess,(void))
{
  uint64_t w,r;
  r = find_largest_max(read_array);
  w = find_largest_max(write_array);
  if (r>w)
    return (int)(((double)w)/NUM_SECS_ROLLING_MEASURE);
  else
    return (int)(((double)r)/NUM_SECS_ROLLING_MEASURE);
}

/** Print the bandwidth history of b (either [dir-]read_array or
 * [dir-]write_array) into the buffer pointed to by buf.  The format is
 * simply comma separated numbers, from oldest to newest.
 *
 * It returns the number of bytes written.
 */
STATIC size_t
bwhist_fill_bandwidth_history(char *buf, size_t len, const bw_array_t *b)
{
  char *cp = buf;
  int i, n;
  const or_options_t *options = get_options();
  uint64_t cutoff;

  if (b->num_maxes_set <= b->next_max_idx) {
    /* We haven't been through the circular array yet; time starts at i=0.*/
    i = 0;
  } else {
    /* We've been around the array at least once.  The next i to be
       overwritten is the oldest. */
    i = b->next_max_idx;
  }

  if (options->RelayBandwidthRate) {
    /* We don't want to report that we used more bandwidth than the max we're
     * willing to relay; otherwise everybody will know how much traffic
     * we used ourself. */
    cutoff = options->RelayBandwidthRate * NUM_SECS_BW_SUM_INTERVAL;
  } else {
    cutoff = UINT64_MAX;
  }

  for (n=0; n<b->num_maxes_set; ++n,++i) {
    uint64_t total;
    if (i >= NUM_TOTALS)
      i -= NUM_TOTALS;
    tor_assert(i < NUM_TOTALS);
    /* Round the bandwidth used down to the nearest 1k. */
    total = b->totals[i] & ~0x3ff;
    if (total > cutoff)
      total = cutoff;

    if (n==(b->num_maxes_set-1))
      tor_snprintf(cp, len-(cp-buf), "%"PRIu64, (total));
    else
      tor_snprintf(cp, len-(cp-buf), "%"PRIu64",", (total));
    cp += strlen(cp);
  }
  return cp-buf;
}

/** Encode a single bandwidth history line into <b>buf</b>. */
static void
bwhist_get_one_bandwidth_line(buf_t *buf, const char *desc,
                              const bw_array_t *b)
{
  /* [dirreq-](read|write)-history yyyy-mm-dd HH:MM:SS (n s) n,n,n... */
  /* The n,n,n part above. Largest representation of a uint64_t is 20 chars
   * long, plus the comma. */
#define MAX_HIST_VALUE_LEN (21*NUM_TOTALS)

  char tmp[MAX_HIST_VALUE_LEN];
  char end[ISO_TIME_LEN+1];

  size_t slen = bwhist_fill_bandwidth_history(tmp, MAX_HIST_VALUE_LEN, b);
  /* If we don't have anything to write, skip to the next entry. */
  if (slen == 0)
    return;

  format_iso_time(end, b->next_period-NUM_SECS_BW_SUM_INTERVAL);
  buf_add_printf(buf, "%s %s (%d s) %s\n",
                 desc, end, NUM_SECS_BW_SUM_INTERVAL, tmp);
}

/** Allocate and return lines for representing this server's bandwidth
 * history in its descriptor. We publish these lines in our extra-info
 * descriptor.
 */
char *
bwhist_get_bandwidth_lines(void)
{
  buf_t *buf = buf_new();

  bwhist_get_one_bandwidth_line(buf, "write-history", write_array);
  bwhist_get_one_bandwidth_line(buf, "read-history", read_array);
  bwhist_get_one_bandwidth_line(buf, "ipv6-write-history", write_array_ipv6);
  bwhist_get_one_bandwidth_line(buf, "ipv6-read-history", read_array_ipv6);
  bwhist_get_one_bandwidth_line(buf, "dirreq-write-history", dir_write_array);
  bwhist_get_one_bandwidth_line(buf, "dirreq-read-history", dir_read_array);

  char *result = buf_extract(buf, NULL);
  buf_free(buf);
  return result;
}

/** Write a single bw_array_t into the Values, Ends, Interval, and Maximum
 * entries of an or_state_t. Done before writing out a new state file. */
static void
bwhist_update_bwhist_state_section(or_state_t *state,
                                     const bw_array_t *b,
                                     smartlist_t **s_values,
                                     smartlist_t **s_maxima,
                                     time_t *s_begins,
                                     int *s_interval)
{
  int i,j;
  uint64_t maxval;

  if (*s_values) {
    SMARTLIST_FOREACH(*s_values, char *, val, tor_free(val));
    smartlist_free(*s_values);
  }
  if (*s_maxima) {
    SMARTLIST_FOREACH(*s_maxima, char *, val, tor_free(val));
    smartlist_free(*s_maxima);
  }
  if (! server_mode(get_options())) {
    /* Clients don't need to store bandwidth history persistently;
     * force these values to the defaults. */
    /* FFFF we should pull the default out of config.c's state table,
     * so we don't have two defaults. */
    if (*s_begins != 0 || *s_interval != 900) {
      time_t now = time(NULL);
      time_t save_at = get_options()->AvoidDiskWrites ? now+3600 : now+600;
      or_state_mark_dirty(state, save_at);
    }
    *s_begins = 0;
    *s_interval = 900;
    *s_values = smartlist_new();
    *s_maxima = smartlist_new();
    return;
  }
  *s_begins = b->next_period;
  *s_interval = NUM_SECS_BW_SUM_INTERVAL;

  *s_values = smartlist_new();
  *s_maxima = smartlist_new();
  /* Set i to first position in circular array */
  i = (b->num_maxes_set <= b->next_max_idx) ? 0 : b->next_max_idx;
  for (j=0; j < b->num_maxes_set; ++j,++i) {
    if (i >= NUM_TOTALS)
      i = 0;
    smartlist_add_asprintf(*s_values, "%"PRIu64,
                           (b->totals[i] & ~0x3ff));
    maxval = b->maxima[i] / NUM_SECS_ROLLING_MEASURE;
    smartlist_add_asprintf(*s_maxima, "%"PRIu64,
                           (maxval & ~0x3ff));
  }
  smartlist_add_asprintf(*s_values, "%"PRIu64,
                         (b->total_in_period & ~0x3ff));
  maxval = b->max_total / NUM_SECS_ROLLING_MEASURE;
  smartlist_add_asprintf(*s_maxima, "%"PRIu64,
                         (maxval & ~0x3ff));
}

/** Update <b>state</b> with the newest bandwidth history. Done before
 * writing out a new state file. */
void
bwhist_update_state(or_state_t *state)
{
#define UPDATE(arrname,st) \
  bwhist_update_bwhist_state_section(state,\
                                       (arrname),\
                                       &state->BWHistory ## st ## Values, \
                                       &state->BWHistory ## st ## Maxima, \
                                       &state->BWHistory ## st ## Ends, \
                                       &state->BWHistory ## st ## Interval)

  UPDATE(write_array, Write);
  UPDATE(read_array, Read);
  UPDATE(write_array_ipv6, IPv6Write);
  UPDATE(read_array_ipv6, IPv6Read);
  UPDATE(dir_write_array, DirWrite);
  UPDATE(dir_read_array, DirRead);

  if (server_mode(get_options())) {
    or_state_mark_dirty(state, time(NULL)+(2*3600));
  }
#undef UPDATE
}

/** Load a single bw_array_t from its Values, Ends, Maxima, and Interval
 * entries in an or_state_t. Done while reading the state file. */
static int
bwhist_load_bwhist_state_section(bw_array_t *b,
                                   const smartlist_t *s_values,
                                   const smartlist_t *s_maxima,
                                   const time_t s_begins,
                                   const int s_interval)
{
  time_t now = time(NULL);
  int retval = 0;
  time_t start;

  uint64_t v, mv;
  int i,ok,ok_m = 0;
  int have_maxima = s_maxima && s_values &&
    (smartlist_len(s_values) == smartlist_len(s_maxima));

  if (s_values && s_begins >= now - NUM_SECS_BW_SUM_INTERVAL*NUM_TOTALS) {
    start = s_begins - s_interval*(smartlist_len(s_values));
    if (start > now)
      return 0;
    b->cur_obs_time = start;
    b->next_period = start + NUM_SECS_BW_SUM_INTERVAL;
    SMARTLIST_FOREACH_BEGIN(s_values, const char *, cp) {
        const char *maxstr = NULL;
        v = tor_parse_uint64(cp, 10, 0, UINT64_MAX, &ok, NULL);
        if (have_maxima) {
          maxstr = smartlist_get(s_maxima, cp_sl_idx);
          mv = tor_parse_uint64(maxstr, 10, 0, UINT64_MAX, &ok_m, NULL);
          mv *= NUM_SECS_ROLLING_MEASURE;
        } else {
          /* No maxima known; guess average rate to be conservative. */
          mv = (v / s_interval) * NUM_SECS_ROLLING_MEASURE;
        }
        if (!ok) {
          retval = -1;
          log_notice(LD_HIST, "Could not parse value '%s' into a number.'",cp);
        }
        if (maxstr && !ok_m) {
          retval = -1;
          log_notice(LD_HIST, "Could not parse maximum '%s' into a number.'",
                     maxstr);
        }

        if (start < now) {
          time_t cur_start = start;
          time_t actual_interval_len = s_interval;
          uint64_t cur_val = 0;
          /* Calculate the average per second. This is the best we can do
           * because our state file doesn't have per-second resolution. */
          if (start + s_interval > now)
            actual_interval_len = now - start;
          cur_val = v / actual_interval_len;
          /* This is potentially inefficient, but since we don't do it very
           * often it should be ok. */
          while (cur_start < start + actual_interval_len) {
            add_obs(b, cur_start, cur_val);
            ++cur_start;
          }
          b->max_total = mv;
          /* This will result in some fairly choppy history if s_interval
           * is not the same as NUM_SECS_BW_SUM_INTERVAL. XXXX */
          start += actual_interval_len;
        }
    } SMARTLIST_FOREACH_END(cp);
  }

  /* Clean up maxima and observed */
  for (i=0; i<NUM_SECS_ROLLING_MEASURE; ++i) {
    b->obs[i] = 0;
  }
  b->total_obs = 0;

  return retval;
}

/** Set bandwidth history from the state file we just loaded. */
int
bwhist_load_state(or_state_t *state, char **err)
{
  int all_ok = 1;

  /* Assert they already have been malloced */
  tor_assert(read_array && write_array);
  tor_assert(read_array_ipv6 && write_array_ipv6);
  tor_assert(dir_read_array && dir_write_array);

#define LOAD(arrname,st)                                                \
  if (bwhist_load_bwhist_state_section(                               \
                                (arrname),                              \
                                state->BWHistory ## st ## Values,       \
                                state->BWHistory ## st ## Maxima,       \
                                state->BWHistory ## st ## Ends,         \
                                state->BWHistory ## st ## Interval)<0)  \
    all_ok = 0

  LOAD(write_array, Write);
  LOAD(read_array, Read);
  LOAD(write_array_ipv6, IPv6Write);
  LOAD(read_array_ipv6, IPv6Read);
  LOAD(dir_write_array, DirWrite);
  LOAD(dir_read_array, DirRead);

#undef LOAD
  if (!all_ok) {
    *err = tor_strdup("Parsing of bandwidth history values failed");
    /* and create fresh arrays */
    bwhist_init();
    return -1;
  }
  return 0;
}

void
bwhist_free_all(void)
{
  bw_array_free(read_array);
  bw_array_free(read_array_ipv6);
  bw_array_free(write_array);
  bw_array_free(write_array_ipv6);
  bw_array_free(dir_read_array);
  bw_array_free(dir_write_array);
}
