/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_stats.c
 * \brief Unit tests for the statistics (reputation history) module.
 **/

#include "orconfig.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "app/config/or_state_st.h"
#include "test/rng_test_helpers.h"

#include <stdio.h>

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#else
#include <dirent.h>
#endif /* defined(_WIN32) */

#include <math.h>

/* These macros pull in declarations for some functions and structures that
 * are typically file-private. */
#define CIRCUITSTATS_PRIVATE
#define CIRCUITLIST_PRIVATE
#define MAINLOOP_PRIVATE
#define STATEFILE_PRIVATE
#define BWHIST_PRIVATE
#define ROUTER_PRIVATE

#include "core/or/or.h"
#include "lib/err/backtrace.h"
#include "lib/buf/buffers.h"
#include "core/or/circuitstats.h"
#include "app/config/config.h"
#include "test/test.h"
#include "core/mainloop/mainloop.h"
#include "lib/memarea/memarea.h"
#include "feature/stats/connstats.h"
#include "feature/stats/rephist.h"
#include "app/config/statefile.h"
#include "feature/stats/bwhist.h"
#include "feature/stats/bw_array_st.h"
#include "feature/relay/router.h"

/** Run unit tests for some stats code. */
static void
test_stats(void *arg)
{
  time_t now = 1281533250; /* 2010-08-11 13:27:30 UTC */
  char *s = NULL;
  int i;

  /* Start with testing exit port statistics; we shouldn't collect exit
   * stats without initializing them. */
  (void)arg;
  rep_hist_note_exit_stream_opened(80);
  rep_hist_note_exit_bytes(80, 100, 10000);
  s = rep_hist_format_exit_stats(now + 86400);
  tt_ptr_op(s, OP_EQ, NULL);

  /* Initialize stats, note some streams and bytes, and generate history
   * string. */
  rep_hist_exit_stats_init(now);
  rep_hist_note_exit_stream_opened(80);
  rep_hist_note_exit_bytes(80, 100, 10000);
  rep_hist_note_exit_stream_opened(443);
  rep_hist_note_exit_bytes(443, 100, 10000);
  rep_hist_note_exit_bytes(443, 100, 10000);
  s = rep_hist_format_exit_stats(now + 86400);
  tt_str_op("exit-stats-end 2010-08-12 13:27:30 (86400 s)\n"
             "exit-kibibytes-written 80=1,443=1,other=0\n"
             "exit-kibibytes-read 80=10,443=20,other=0\n"
             "exit-streams-opened 80=4,443=4,other=0\n",OP_EQ, s);
  tor_free(s);

  /* Add a few bytes on 10 more ports and ensure that only the top 10
   * ports are contained in the history string. */
  for (i = 50; i < 60; i++) {
    rep_hist_note_exit_bytes(i, i, i);
    rep_hist_note_exit_stream_opened(i);
  }
  s = rep_hist_format_exit_stats(now + 86400);
  tt_str_op("exit-stats-end 2010-08-12 13:27:30 (86400 s)\n"
             "exit-kibibytes-written 52=1,53=1,54=1,55=1,56=1,57=1,58=1,"
             "59=1,80=1,443=1,other=1\n"
             "exit-kibibytes-read 52=1,53=1,54=1,55=1,56=1,57=1,58=1,"
             "59=1,80=10,443=20,other=1\n"
             "exit-streams-opened 52=4,53=4,54=4,55=4,56=4,57=4,58=4,"
             "59=4,80=4,443=4,other=4\n",OP_EQ, s);
  tor_free(s);

  /* Stop collecting stats, add some bytes, and ensure we don't generate
   * a history string. */
  rep_hist_exit_stats_term();
  rep_hist_note_exit_bytes(80, 100, 10000);
  s = rep_hist_format_exit_stats(now + 86400);
  tt_ptr_op(s, OP_EQ, NULL);

  /* Re-start stats, add some bytes, reset stats, and see what history we
   * get when observing no streams or bytes at all. */
  rep_hist_exit_stats_init(now);
  rep_hist_note_exit_stream_opened(80);
  rep_hist_note_exit_bytes(80, 100, 10000);
  rep_hist_reset_exit_stats(now);
  s = rep_hist_format_exit_stats(now + 86400);
  tt_str_op("exit-stats-end 2010-08-12 13:27:30 (86400 s)\n"
             "exit-kibibytes-written other=0\n"
             "exit-kibibytes-read other=0\n"
             "exit-streams-opened other=0\n",OP_EQ, s);
  tor_free(s);

  /* Continue with testing connection statistics; we shouldn't collect
   * conn stats without initializing them. */
  conn_stats_note_or_conn_bytes(1, 20, 400, now, false);
  s = conn_stats_format(now + 86400);
  tt_ptr_op(s, OP_EQ, NULL);

  /* Initialize stats, note bytes, and generate history string. */
  conn_stats_init(now);
  conn_stats_note_or_conn_bytes(1, 30000, 400000, now, false);
  conn_stats_note_or_conn_bytes(1, 30000, 400000, now + 5, false);
  conn_stats_note_or_conn_bytes(2, 400000, 30000, now + 10, true);
  conn_stats_note_or_conn_bytes(2, 400000, 30000, now + 15, true);
  s = conn_stats_format(now + 86400);
  tt_str_op("conn-bi-direct 2010-08-12 13:27:30 (86400 s) 0,0,1,0\n"
            "ipv6-conn-bi-direct 2010-08-12 13:27:30 (86400 s) 0,0,0,0\n",
            OP_EQ, s);
  tor_free(s);

  /* Stop collecting stats, add some bytes, and ensure we don't generate
   * a history string. */
  conn_stats_terminate();
  conn_stats_note_or_conn_bytes(2, 400000, 30000, now + 15, true);
  s = conn_stats_format(now + 86400);
  tt_ptr_op(s, OP_EQ, NULL);

  /* Re-start stats, add some bytes, reset stats, and see what history we
   * get when observing no bytes at all. */
  conn_stats_init(now);
  conn_stats_note_or_conn_bytes(1, 30000, 400000, now, false);
  conn_stats_note_or_conn_bytes(1, 30000, 400000, now + 5, false);
  conn_stats_note_or_conn_bytes(2, 400000, 30000, now + 10, true);
  conn_stats_note_or_conn_bytes(2, 400000, 30000, now + 15, true);
  conn_stats_reset(now);
  s = conn_stats_format(now + 86400);
  tt_str_op("conn-bi-direct 2010-08-12 13:27:30 (86400 s) 0,0,0,0\n"
            "ipv6-conn-bi-direct 2010-08-12 13:27:30 (86400 s) 0,0,0,0\n",
            OP_EQ, s);
  tor_free(s);

  /* Continue with testing buffer statistics; we shouldn't collect buffer
   * stats without initializing them. */
  rep_hist_add_buffer_stats(2.0, 2.0, 20);
  s = rep_hist_format_buffer_stats(now + 86400);
  tt_ptr_op(s, OP_EQ, NULL);

  /* Initialize stats, add statistics for a single circuit, and generate
   * the history string. */
  rep_hist_buffer_stats_init(now);
  rep_hist_add_buffer_stats(2.0, 2.0, 20);
  s = rep_hist_format_buffer_stats(now + 86400);
  tt_str_op("cell-stats-end 2010-08-12 13:27:30 (86400 s)\n"
             "cell-processed-cells 20,0,0,0,0,0,0,0,0,0\n"
             "cell-queued-cells 2.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,"
                               "0.00,0.00\n"
             "cell-time-in-queue 2,0,0,0,0,0,0,0,0,0\n"
             "cell-circuits-per-decile 1\n",OP_EQ, s);
  tor_free(s);

  /* Add nineteen more circuit statistics to the one that's already in the
   * history to see that the math works correctly. */
  for (i = 21; i < 30; i++)
    rep_hist_add_buffer_stats(2.0, 2.0, i);
  for (i = 20; i < 30; i++)
    rep_hist_add_buffer_stats(3.5, 3.5, i);
  s = rep_hist_format_buffer_stats(now + 86400);
  tt_str_op("cell-stats-end 2010-08-12 13:27:30 (86400 s)\n"
             "cell-processed-cells 29,28,27,26,25,24,23,22,21,20\n"
             "cell-queued-cells 2.75,2.75,2.75,2.75,2.75,2.75,2.75,2.75,"
                               "2.75,2.75\n"
             "cell-time-in-queue 3,3,3,3,3,3,3,3,3,3\n"
             "cell-circuits-per-decile 2\n",OP_EQ, s);
  tor_free(s);

  /* Stop collecting stats, add statistics for one circuit, and ensure we
   * don't generate a history string. */
  rep_hist_buffer_stats_term();
  rep_hist_add_buffer_stats(2.0, 2.0, 20);
  s = rep_hist_format_buffer_stats(now + 86400);
  tt_ptr_op(s, OP_EQ, NULL);

  /* Re-start stats, add statistics for one circuit, reset stats, and make
   * sure that the history has all zeros. */
  rep_hist_buffer_stats_init(now);
  rep_hist_add_buffer_stats(2.0, 2.0, 20);
  rep_hist_reset_buffer_stats(now);
  s = rep_hist_format_buffer_stats(now + 86400);
  tt_str_op("cell-stats-end 2010-08-12 13:27:30 (86400 s)\n"
             "cell-processed-cells 0,0,0,0,0,0,0,0,0,0\n"
             "cell-queued-cells 0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,"
                               "0.00,0.00\n"
             "cell-time-in-queue 0,0,0,0,0,0,0,0,0,0\n"
             "cell-circuits-per-decile 0\n",OP_EQ, s);

 done:
  tor_free(s);
}

/** Run unit tests the mtbf stats code. */
static void
test_rephist_mtbf(void *arg)
{
  (void)arg;

  time_t now = 1572500000; /* 2010-10-31 05:33:20 UTC */
  time_t far_future = MAX(now, time(NULL)) + 365*24*60*60;
  int r;

  /* Make a temporary datadir for these tests */
  char *ddir_fname = tor_strdup(get_fname_rnd("datadir_mtbf"));
  tor_free(get_options_mutable()->DataDirectory);
  get_options_mutable()->DataDirectory = tor_strdup(ddir_fname);
  check_private_dir(ddir_fname, CPD_CREATE, NULL);

  rep_history_clean(far_future);

  /* No data */

  r = rep_hist_load_mtbf_data(now);
  tt_int_op(r, OP_EQ, -1);
  rep_history_clean(far_future);

  /* Blank data */

  r = rep_hist_record_mtbf_data(now, 0);
  tt_int_op(r, OP_EQ, 0);
  r = rep_hist_load_mtbf_data(now);
  tt_int_op(r, OP_EQ, 0);
  rep_history_clean(far_future);

  r = rep_hist_record_mtbf_data(now, 1);
  tt_int_op(r, OP_EQ, 0);
  r = rep_hist_load_mtbf_data(now);
  tt_int_op(r, OP_EQ, 0);
  rep_history_clean(far_future);

 done:
  rep_history_clean(far_future);
  tor_free(ddir_fname);
}

static void
test_commit_max(void *arg)
{
  (void) arg;
  bw_array_t *b = bw_array_new();
  time_t now = b->cur_obs_time;

  commit_max(b);
  tt_int_op(b->next_period, OP_EQ, now + 2*86400);

  b->total_in_period = 100;
  b->max_total = 10;
  commit_max(b);
  tor_assert(b->total_in_period == 0);
  tor_assert(b->max_total == 0);
  tt_int_op(b->totals[1], OP_EQ, 100);
  tt_int_op(b->maxima[1], OP_EQ, 10);
  tt_int_op(b->next_period, OP_EQ, now + 3*86400);

  commit_max(b);
  tt_int_op(b->next_period, OP_EQ, now + 4*86400);

  commit_max(b);
  tt_int_op(b->next_period, OP_EQ, now + 5*86400);

  b->total_in_period = 100;
  b->max_total = 10;
  commit_max(b);
  tor_assert(!b->next_max_idx);
  tt_int_op(b->cur_obs_idx, OP_EQ, 0);
  tt_int_op(b->totals[4], OP_EQ, 100);
  tt_int_op(b->maxima[4], OP_EQ, 10);
  tt_int_op(b->next_period, OP_EQ, now + 6*86400);
 done:
  bw_array_free(b);
}

#define test_obs(b, idx, time, tot, max) STMT_BEGIN \
    tt_int_op(b->cur_obs_idx, OP_EQ, idx); \
    tt_int_op(b->cur_obs_time, OP_EQ, time); \
    tt_int_op(b->total_obs, OP_EQ, tot); \
    tt_int_op(b->max_total, OP_EQ, max); \
  STMT_END;

static void
test_advance_obs(void *arg)
{
  (void) arg;
  int iter, tot = 0;
  bw_array_t *b = bw_array_new();
  time_t now = b->cur_obs_time;

  for (iter = 0; iter < 10; ++iter) {
    b->obs[b->cur_obs_idx] += 10;
    tot += 10;
    advance_obs(b);
    if (iter == 9) {
      /* The current value under cur_obs_idx was zeroed in last iterN. */
      test_obs(b, 0, now+iter+1, tot - 10, tot);
      break;
    }
    test_obs(b, iter+1, now+iter+1, tot, tot);
  }

  b->total_in_period = 100;
  b->cur_obs_time = now + NUM_SECS_BW_SUM_INTERVAL - 1;
  advance_obs(b);
  test_obs(b, 1, now+NUM_SECS_BW_SUM_INTERVAL, 80, 0);
  tt_int_op(b->maxima[0], OP_EQ, 100);
  tt_int_op(b->totals[0], OP_EQ, 100);
  tt_int_op(b->num_maxes_set, OP_EQ, 1);
 done:
  bw_array_free(b);
}

#define test_add_obs_(b, now, checknow, bw, tot) STMT_BEGIN \
    tot += bw; \
    add_obs(b, now, bw); \
    tt_int_op(b->cur_obs_time, OP_EQ, checknow); \
    tt_int_op(b->obs[b->cur_obs_idx], OP_EQ, bw); \
    tt_int_op(b->total_in_period, OP_EQ, tot); \
  STMT_END;

static void
test_add_obs(void *arg)
{
  (void) arg;
  bw_array_t *b = bw_array_new();
  time_t now = b->cur_obs_time;
  uint64_t bw = 0, tot = 0;
  /* Requests for the past should not be entertained. */
  test_add_obs_(b, now-1, now, bw, tot);
  /* Test the expected functionalities for random values. */
  now += 53;
  bw = 97;
  test_add_obs_(b, now, now, bw, tot);

  now += 60*60;
  bw = 90;
  test_add_obs_(b, now, now, bw, tot);

  now += 24*60*60;
  bw = 100;
  tot = 0;
  test_add_obs_(b, now, now, bw, tot);
 done:
  bw_array_free(b);
}

static or_options_t mock_options;

static const or_options_t *
mock_get_options(void)
{
  return &mock_options;
}

#define MAX_HIST_VALUE_LEN 21*NUM_TOTALS

#define set_test_case(b, max, idx, a1, a2, a3, a4, a5) STMT_BEGIN \
    b->num_maxes_set = max; \
    b->next_max_idx = idx; \
    b->totals[0] = a1; \
    b->totals[1] = a2; \
    b->totals[2] = a3; \
    b->totals[3] = a4; \
    b->totals[4] = a5; \
  STMT_END;

#define test_fill_bw(b, buf, rv, str, checkrv) STMT_BEGIN \
    buf = tor_malloc_zero(MAX_HIST_VALUE_LEN); \
    rv = bwhist_fill_bandwidth_history(buf, MAX_HIST_VALUE_LEN, b); \
    tt_str_op(buf, OP_EQ, str); \
    tt_int_op(rv, OP_EQ, checkrv); \
    tor_free(buf); \
  STMT_END;

static void
test_fill_bandwidth_history(void *arg)
{
  (void) arg;
  bw_array_t *b = bw_array_new();
  char *buf;
  size_t rv;
  /* Remember bandwidth is rounded down to the nearest 1K. */
  /* Day 1. */
  set_test_case(b, 0, 0, 0, 0, 0, 0, 0);
  buf = tor_malloc_zero(MAX_HIST_VALUE_LEN);
  rv = bwhist_fill_bandwidth_history(buf, MAX_HIST_VALUE_LEN, b);
  tt_int_op(rv, OP_EQ, 0);
  tor_free(buf);
  /* Day 2. */
  set_test_case(b, 1, 1, 1000, 0, 0, 0, 0);
  test_fill_bw(b, buf, rv, "0", 1);
  /* Day 3. */
  set_test_case(b, 2, 2, 1000, 1500, 0, 0, 0);
  test_fill_bw(b, buf, rv, "0,1024", 6);
  /* Day 4. */
  set_test_case(b, 3, 3, 1000, 1500, 3500, 0, 0);
  test_fill_bw(b, buf, rv, "0,1024,3072", 11);
  /* Day 5. */
  set_test_case(b, 4, 4, 1000, 1500, 3500, 8000, 0);
  test_fill_bw(b, buf, rv, "0,1024,3072,7168", 16);
  /* Day 6. */
  set_test_case(b, 5, 0, 1000, 1500, 3500, 8000, 6000);
  test_fill_bw(b, buf, rv, "0,1024,3072,7168,5120", 21);
  /* Day 7. */
  /* Remember oldest entry first. */
  set_test_case(b, 5, 1, 10000, 1500, 3500, 8000, 6000);
  test_fill_bw(b, buf, rv, "1024,3072,7168,5120,9216", 24);
  /* Mocking get_options to manipulate RelayBandwidthRate. */
  MOCK(get_options, mock_get_options);
  /* Limits bandwidth to 1 KBps. */
  /* Cutoff is set to 88473600. */
  mock_options.RelayBandwidthRate = 1024;
  set_test_case(b, 5, 2, 88573600, 88473600, 10000, 8000, 6000);
  test_fill_bw(b, buf, rv, "9216,7168,5120,88473600,88473600", 32);
 done:
  UNMOCK(get_options);
  bw_array_free(b);
}

#define set_test_bw_lines(r, w, dr, dw, when) STMT_BEGIN \
    bwhist_note_bytes_read(r, when, false); \
    bwhist_note_bytes_written(w, when, false); \
    bwhist_note_dir_bytes_read(dr, when); \
    bwhist_note_dir_bytes_written(dw, when); \
  STMT_END;

#define test_get_bw_lines(str, checkstr) STMT_BEGIN \
    str = bwhist_get_bandwidth_lines(); \
    tt_str_op(str, OP_EQ, checkstr); \
    tor_free(str); \
  STMT_END;

static void
test_get_bandwidth_lines(void *arg)
{
  (void) arg;
  char *str = NULL, *checkstr = NULL;
  char t[ISO_TIME_LEN+1];
  int len = (67+MAX_HIST_VALUE_LEN)*4;
  checkstr = tor_malloc_zero(len);
  time_t now = time(NULL);
  bwhist_init();

  /* Day 1. */
  now += 86400;
  set_test_bw_lines(5000, 5500, 3000, 3500, now - 6*60*60);
  /* Day 2. */
  now += 86400;
  set_test_bw_lines(50000, 55000, 30000, 35000, now - 6*60*60);
  /* Day 3. */
  now += 86400;
  set_test_bw_lines(25000, 27500, 15000, 17500, now - 6*60*60);
  /* Day 4. */
  now += 86400;
  set_test_bw_lines(90000, 76000, 60000, 45000, now - 6*60*60);
  /* Day 5. */
  now += 86400;
  set_test_bw_lines(500, 55000, 30000, 35000, now - 6*60*60);
  set_test_bw_lines(0, 0, 0, 0, now);
  format_iso_time(t, now);
  tor_snprintf(checkstr, len, "write-history %s (86400 s) "
                    "5120,54272,26624,75776,54272\n"
                    "read-history %s (86400 s) "
                    "4096,49152,24576,89088,0\n"
                    "dirreq-write-history %s (86400 s) "
                    "3072,34816,17408,44032,34816\n"
                    "dirreq-read-history %s (86400 s) "
                    "2048,29696,14336,59392,29696\n",
                    t, t, t, t);
  test_get_bw_lines(str, checkstr);

 done:
  tor_free(str);
  tor_free(checkstr);
  bwhist_free_all();
}

static void
test_load_stats_file(void *arg)
{
  int ret;
  char *content = NULL, *read_file_content = NULL, *fname = NULL;

  (void) arg;

  /* Load conn-stats. */
  fname = get_datadir_fname("conn-stats");
  tt_assert(fname);
  read_file_content = tor_strdup(
    "conn-bi-direct 2020-12-13 15:48:53 (86400 s) 12,34,56,78\n"
    "ipv6-conn-bi-direct 2020-12-14 15:48:53 (86400 s) 21,43,65,87\n");
  write_str_to_file(fname, read_file_content, 0);
  ret = load_stats_file("conn-stats", "conn-bi-direct", 1607874000, &content);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(read_file_content, OP_EQ, content);

  /* Load hidserv-stats. */
  tor_free(fname);
  fname = get_datadir_fname("hidserv-stats");
  tt_assert(fname);
  tor_free(read_file_content);
  read_file_content = tor_strdup(
    "hidserv-stats-end 2020-12-13 15:48:53 (86400 s)\n"
    "hidserv-rend-relayed-cells 48754891 delta_f=2048 epsilon=0.30 "
      "bin_size=1024\n"
    "hidserv-dir-onions-seen 53 delta_f=8 epsilon=0.30 bin_size=8\n");
  write_str_to_file(fname, read_file_content, 0);
  tor_free(content);
  ret = load_stats_file("hidserv-stats", "hidserv-stats-end", 1607874000,
                        &content);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(read_file_content, OP_EQ, content);

  /* Load dirreq-stats. */
  tor_free(fname);
  fname = get_datadir_fname("dirreq-stats");
  tt_assert(fname);
  tor_free(read_file_content);
  read_file_content = tor_strdup(
    "dirreq-stats-end 2020-12-13 15:48:53 (86400 s)\n"
    "dirreq-v3-ips ru=1728,us=1144,de=696,ir=432,gb=328,fr=304,in=296,ua=232\n"
    "dirreq-v3-reqs ru=3616,us=3576,de=1896,fr=800,gb=632,ir=616\n"
    "dirreq-v3-resp ok=18472,not-enough-sigs=0,unavailable=0,not-found=0,"
      "not-modified=3136,busy=0\n"
    "dirreq-v3-direct-dl complete=0,timeout=0,running=0\n"
    "dirreq-v3-tunneled-dl complete=18124,timeout=348,running=4,min=257,"
      "d1=133653,d2=221050,q1=261242,d3=300622,d4=399758,md=539051,d6=721322,"
      "d7=959866,q3=1103363,d8=1302035,d9=2046125,max=113404000\n");
  write_str_to_file(fname, read_file_content, 0);
  tor_free(content);
  ret = load_stats_file("dirreq-stats", "dirreq-stats-end", 1607874000,
                        &content);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(read_file_content, OP_EQ, content);

  /* Attempt to load future-stats file not starting with timestamp tag. */
  tor_free(fname);
  fname = get_datadir_fname("future-stats");
  tt_assert(fname);
  tor_free(read_file_content);
  read_file_content = tor_strdup(
    "future-stuff-at-file-start\n"
    "future-stats 2020-12-13 15:48:53 (86400 s)\n");
  write_str_to_file(fname, read_file_content, 0);
  tor_free(content);
  ret = load_stats_file("future-stats", "future-stats", 1607874000, &content);
  tt_int_op(ret, OP_EQ, 1);
  tt_str_op(read_file_content, OP_EQ, content);

 done:
  tor_free(fname);
  tor_free(read_file_content);
  tor_free(content);
}

#define ENT(name)                                                       \
  { #name, test_ ## name , 0, NULL, NULL }
#define FORK(name)                                                      \
  { #name, test_ ## name , TT_FORK, NULL, NULL }

struct testcase_t stats_tests[] = {
  FORK(stats),
  ENT(rephist_mtbf),
  FORK(commit_max),
  FORK(advance_obs),
  FORK(add_obs),
  FORK(fill_bandwidth_history),
  FORK(get_bandwidth_lines),
  FORK(load_stats_file),

  END_OF_TESTCASES
};
