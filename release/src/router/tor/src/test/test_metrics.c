/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_metrics.c
 * \brief Test lib/metrics and feature/metrics functionalities
 */

#define CONFIG_PRIVATE
#define CONNECTION_PRIVATE
#define MAINLOOP_PRIVATE
#define METRICS_STORE_ENTRY_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"

#include "app/config/config.h"

#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/connection_st.h"
#include "core/or/policies.h"
#include "core/or/port_cfg_st.h"

#include "feature/metrics/metrics.h"

#include "lib/encoding/confline.h"
#include "lib/metrics/metrics_store.h"

#include <limits.h>

#define TEST_METRICS_ENTRY_NAME    "entryA"
#define TEST_METRICS_ENTRY_HELP    "Description of entryA"
#define TEST_METRICS_ENTRY_LABEL_1 "label=\"farfadet\""
#define TEST_METRICS_ENTRY_LABEL_2 "label=\"ponki\""

#define TEST_METRICS_HIST_ENTRY_NAME    "test_hist_entry"
#define TEST_METRICS_HIST_ENTRY_HELP    "Description of test_hist_entry"

static void
set_metrics_port(or_options_t *options)
{
  const char *port = "MetricsPort 9035"; /* Default to 127.0.0.1 */
  const char *policy = "MetricsPortPolicy accept 1.2.3.4";

  config_get_lines(port, &options->MetricsPort_lines, 0);
  config_get_lines(policy, &options->MetricsPortPolicy, 0);

  /* Parse and validate policy. */
  policies_parse_from_options(options);
}

static void
test_config(void *arg)
{
  char *err_msg = NULL;
  tor_addr_t addr;
  smartlist_t *ports = smartlist_new();
  or_options_t *options = get_options_mutable();

  (void) arg;

  set_metrics_port(options);

  int ret = metrics_parse_ports(options, ports, &err_msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(smartlist_len(ports), OP_EQ, 1);

  /* Validate the configured port. */
  const port_cfg_t *cfg = smartlist_get(ports, 0);
  tt_assert(tor_addr_eq_ipv4h(&cfg->addr, 0x7f000001));
  tt_int_op(cfg->port, OP_EQ, 9035);
  tt_int_op(cfg->type, OP_EQ, CONN_TYPE_METRICS_LISTENER);

  /* Address of the policy should be permitted. */
  tor_addr_from_ipv4h(&addr, 0x01020304); /* 1.2.3.4 */
  ret = metrics_policy_permits_address(&addr);
  tt_int_op(ret, OP_EQ, true);

  /* Anything else, should not. */
  tor_addr_from_ipv4h(&addr, 0x01020305); /* 1.2.3.5 */
  ret = metrics_policy_permits_address(&addr);
  tt_int_op(ret, OP_EQ, false);

 done:
  SMARTLIST_FOREACH(ports, port_cfg_t *, c, port_cfg_free(c));
  smartlist_free(ports);
  or_options_free(options);
  tor_free(err_msg);
}

static char _c_buf[256];
#define CONTAINS(conn, msg) \
  do { \
    tt_int_op(buf_datalen(conn->outbuf), OP_EQ, (strlen(msg))); \
    memset(_c_buf, 0, sizeof(_c_buf)); \
    buf_get_bytes(conn->outbuf, _c_buf, (strlen(msg))); \
    tt_str_op(_c_buf, OP_EQ, (msg)); \
    tt_int_op(buf_datalen(conn->outbuf), OP_EQ, 0); \
  } while (0)

#define WRITE(conn, msg) \
  buf_add(conn->inbuf, (msg), (strlen(msg)));

/* Free the previous conn object if any and allocate a new connection. In
 * order to be allowed, set its address to 1.2.3.4 as per the policy. */
#define NEW_ALLOWED_CONN()                              \
  do {                                                  \
    close_closeable_connections();                      \
    conn = connection_new(CONN_TYPE_METRICS, AF_INET);  \
    tor_addr_from_ipv4h(&conn->addr, 0x01020304);       \
  } while (0)

static void
test_connection(void *arg)
{
  int ret;
  connection_t *conn = NULL;
  or_options_t *options = get_options_mutable();

  (void) arg;

  /* Notice that in this test, we will allocate a new connection at every test
   * case. This is because the metrics_connection_process_inbuf() marks for
   * close the connection in case of an error and thus we can't call again an
   * inbuf process function on a marked for close connection. */

  tor_init_connection_lists();

  /* Setup policy. */
  set_metrics_port(options);

  /* Set 1.2.3.5 IP, we should get rejected. */
  NEW_ALLOWED_CONN();
  tor_addr_from_ipv4h(&conn->addr, 0x01020305);
  ret = metrics_connection_process_inbuf(conn);
  tt_int_op(ret, OP_EQ, -1);

  /* No HTTP request yet. */
  NEW_ALLOWED_CONN();
  ret = metrics_connection_process_inbuf(conn);
  tt_int_op(ret, OP_EQ, 0);
  connection_free_minimal(conn);

  /* Bad request. */
  NEW_ALLOWED_CONN();
  WRITE(conn, "HTTP 4.7\r\n\r\n");
  ret = metrics_connection_process_inbuf(conn);
  tt_int_op(ret, OP_EQ, -1);
  CONTAINS(conn, "HTTP/1.0 400 Bad Request\r\n\r\n");

  /* Path not found. */
  NEW_ALLOWED_CONN();
  WRITE(conn, "GET /badpath HTTP/1.0\r\n\r\n");
  ret = metrics_connection_process_inbuf(conn);
  tt_int_op(ret, OP_EQ, -1);
  CONTAINS(conn, "HTTP/1.0 404 Not Found\r\n\r\n");

  /* Method not allowed. */
  NEW_ALLOWED_CONN();
  WRITE(conn, "POST /something HTTP/1.0\r\n\r\n");
  ret = metrics_connection_process_inbuf(conn);
  tt_int_op(ret, OP_EQ, -1);
  CONTAINS(conn, "HTTP/1.0 405 Method Not Allowed\r\n\r\n");

  /* Ask for metrics. The content should be above 0. We don't test the
   * validity of the returned content but it is certainly not an error. */
  NEW_ALLOWED_CONN();
  WRITE(conn, "GET /metrics HTTP/1.0\r\n\r\n");
  ret = metrics_connection_process_inbuf(conn);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(buf_datalen(conn->outbuf), OP_GT, 0);

 done:
  or_options_free(options);
  connection_free_minimal(conn);
}

static void
test_prometheus(void *arg)
{
  metrics_store_t *store = NULL;
  metrics_store_entry_t *entry = NULL;
  buf_t *buf = buf_new();
  char *output = NULL;

  (void) arg;

  /* Fresh new store. No entries. */
  store = metrics_store_new();
  tt_assert(store);

  /* Add entry and validate its content. */
  entry = metrics_store_add(store, METRICS_TYPE_COUNTER,
                            TEST_METRICS_ENTRY_NAME,
                            TEST_METRICS_ENTRY_HELP,
                            0, NULL);
  tt_assert(entry);
  metrics_store_entry_add_label(entry, TEST_METRICS_ENTRY_LABEL_1);

  static const char *expected =
    "# HELP " TEST_METRICS_ENTRY_NAME " " TEST_METRICS_ENTRY_HELP "\n"
    "# TYPE " TEST_METRICS_ENTRY_NAME " counter\n"
    TEST_METRICS_ENTRY_NAME "{" TEST_METRICS_ENTRY_LABEL_1 "} 0\n";

  metrics_store_get_output(METRICS_FORMAT_PROMETHEUS, store, buf);
  output = buf_extract(buf, NULL);
  tt_str_op(expected, OP_EQ, output);

 done:
  buf_free(buf);
  tor_free(output);
  metrics_store_free(store);
}

static void
test_prometheus_histogram(void *arg)
{
  metrics_store_t *store = NULL;
  metrics_store_entry_t *entry = NULL;
  buf_t *buf = buf_new();
  char *output = NULL;
  const int64_t buckets[] = { 10, 20, 3000 };

  (void) arg;

  /* Fresh new store. No entries. */
  store = metrics_store_new();
  tt_assert(store);

  /* Add a histogram entry and validate its content. */
  entry = metrics_store_add(store, METRICS_TYPE_HISTOGRAM,
                            TEST_METRICS_HIST_ENTRY_NAME,
                            TEST_METRICS_HIST_ENTRY_HELP,
                            ARRAY_LENGTH(buckets), buckets);
  tt_assert(entry);
  metrics_store_entry_add_label(entry, TEST_METRICS_ENTRY_LABEL_1);

  static const char *expected =
    "# HELP " TEST_METRICS_HIST_ENTRY_NAME " "
        TEST_METRICS_HIST_ENTRY_HELP "\n"
    "# TYPE " TEST_METRICS_HIST_ENTRY_NAME " histogram\n"
    TEST_METRICS_HIST_ENTRY_NAME "_bucket{"
        TEST_METRICS_ENTRY_LABEL_1 ",le=\"10.00\"} 0\n"
    TEST_METRICS_HIST_ENTRY_NAME "_bucket{"
        TEST_METRICS_ENTRY_LABEL_1 ",le=\"20.00\"} 0\n"
    TEST_METRICS_HIST_ENTRY_NAME "_bucket{"
        TEST_METRICS_ENTRY_LABEL_1 ",le=\"3000.00\"} 0\n"
    TEST_METRICS_HIST_ENTRY_NAME "_bucket{"
        TEST_METRICS_ENTRY_LABEL_1 ",le=\"+Inf\"} 0\n"
    TEST_METRICS_HIST_ENTRY_NAME "_sum{" TEST_METRICS_ENTRY_LABEL_1 "} 0\n"
    TEST_METRICS_HIST_ENTRY_NAME "_count{" TEST_METRICS_ENTRY_LABEL_1 "} 0\n";

  metrics_store_get_output(METRICS_FORMAT_PROMETHEUS, store, buf);
  output = buf_extract(buf, NULL);
  tt_str_op(expected, OP_EQ, output);

 done:
  buf_free(buf);
  tor_free(output);
  metrics_store_free(store);
}

static void
test_store(void *arg)
{
  metrics_store_t *store = NULL;
  metrics_store_entry_t *entry = NULL;
  const int64_t buckets[] = { 10, 20, 3000 };
  const size_t bucket_count = ARRAY_LENGTH(buckets);

  (void) arg;

  /* Fresh new store. No entries. */
  store = metrics_store_new();
  tt_assert(store);
  tt_assert(!metrics_store_get_all(store, TEST_METRICS_ENTRY_NAME));

  /* Add entry and validate its content. */
  entry = metrics_store_add(store, METRICS_TYPE_COUNTER,
                            TEST_METRICS_ENTRY_NAME,
                            TEST_METRICS_ENTRY_HELP, 0, NULL);
  tt_assert(entry);
  tt_int_op(entry->type, OP_EQ, METRICS_TYPE_COUNTER);
  tt_str_op(entry->name, OP_EQ, TEST_METRICS_ENTRY_NAME);
  tt_str_op(entry->help, OP_EQ, TEST_METRICS_ENTRY_HELP);
  tt_uint_op(entry->u.counter.value, OP_EQ, 0);

  /* Access the entry. */
  tt_assert(metrics_store_get_all(store, TEST_METRICS_ENTRY_NAME));

  /* Add a label to the entry to make it unique. */
  metrics_store_entry_add_label(entry, TEST_METRICS_ENTRY_LABEL_1);
  tt_int_op(metrics_store_entry_has_label(entry, TEST_METRICS_ENTRY_LABEL_1),
            OP_EQ, true);

  /* Update entry's value. */
  metrics_store_entry_update(entry, 42);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 42);
  metrics_store_entry_update(entry, 42);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 84);
  metrics_store_entry_reset(entry);
  tt_int_op(metrics_store_entry_get_value(entry), OP_EQ, 0);

  /* Add a new entry of same name but different label. */
  /* Add entry and validate its content. */
  entry = metrics_store_add(store, METRICS_TYPE_COUNTER,
                            TEST_METRICS_ENTRY_NAME,
                            TEST_METRICS_ENTRY_HELP, 0, NULL);
  tt_assert(entry);
  metrics_store_entry_add_label(entry, TEST_METRICS_ENTRY_LABEL_2);

  /* Make sure _both_ entries are there. */
  const smartlist_t *entries =
    metrics_store_get_all(store, TEST_METRICS_ENTRY_NAME);
  tt_assert(entries);
  tt_int_op(smartlist_len(entries), OP_EQ, 2);

  /* Add a histogram entry and validate its content. */
  entry = metrics_store_add(store, METRICS_TYPE_HISTOGRAM,
                            TEST_METRICS_HIST_ENTRY_NAME,
                            TEST_METRICS_HIST_ENTRY_HELP,
                            bucket_count, buckets);

  tt_assert(entry);
  tt_int_op(entry->type, OP_EQ, METRICS_TYPE_HISTOGRAM);
  tt_str_op(entry->name, OP_EQ, TEST_METRICS_HIST_ENTRY_NAME);
  tt_str_op(entry->help, OP_EQ, TEST_METRICS_HIST_ENTRY_HELP);
  tt_uint_op(entry->u.histogram.bucket_count, OP_EQ, bucket_count);

  for (size_t i = 0; i < bucket_count; ++i) {
    tt_uint_op(entry->u.histogram.buckets[i].bucket, OP_EQ, buckets[i]);
    tt_uint_op(entry->u.histogram.buckets[i].value, OP_EQ, 0);
  }

  /* Access the entry. */
  tt_assert(metrics_store_get_all(store, TEST_METRICS_HIST_ENTRY_NAME));

  /* Record various observations. */
  metrics_store_hist_entry_update(entry, 3, 11);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 10), OP_EQ, 0);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 20), OP_EQ, 3);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 3);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 3);
  tt_int_op(metrics_store_hist_entry_get_count(entry), OP_EQ, 3);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, 11);

  metrics_store_hist_entry_update(entry, 1, 42);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 10), OP_EQ, 0);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 20), OP_EQ, 3);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 4);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 4);
  tt_int_op(metrics_store_hist_entry_get_count(entry), OP_EQ, 4);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, 53);

  /* Ensure this resets all buckets back to 0. */
  metrics_store_entry_reset(entry);
  for (size_t i = 0; i < bucket_count; ++i) {
    tt_uint_op(entry->u.histogram.buckets[i].bucket, OP_EQ, buckets[i]);
    tt_uint_op(entry->u.histogram.buckets[i].value, OP_EQ, 0);
  }

  /* tt_int_op assigns the third argument to a variable of type long, which
   * overflows on some platforms (e.g. on some 32-bit systems). We disable
   * these checks for those platforms. */
#if LONG_MAX >= INT64_MAX
  metrics_store_hist_entry_update(entry, 1, INT64_MAX - 13);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, INT64_MAX - 13);
  metrics_store_hist_entry_update(entry, 1, 13);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, INT64_MAX);
  /* Uh-oh, the sum of all observations is now greater than INT64_MAX. Make
   * sure we reset the entry instead of overflowing the sum. */
  metrics_store_hist_entry_update(entry, 1, 1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 10), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 20), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_count(entry), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, 1);
#endif

#if LONG_MIN <= INT64_MIN
  metrics_store_entry_reset(entry);
  /* In practice, we're not going to have negative observations (as we only use
   * histograms for timings, which are always positive), but technically
   * prometheus _does_ support negative observations. */
  metrics_store_hist_entry_update(entry, 1, INT64_MIN + 13);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, INT64_MIN + 13);
  metrics_store_hist_entry_update(entry, 1, -13);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, INT64_MIN);
  /* Uh-oh, the sum of all observations is now less than INT64_MIN. Make
   * sure we reset the entry instead of underflowing the sum. */
  metrics_store_hist_entry_update(entry, 1, -1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 10), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 20), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_value(entry, 3000), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_count(entry), OP_EQ, 1);
  tt_int_op(metrics_store_hist_entry_get_sum(entry), OP_EQ, -1);
#endif

 done:
  metrics_store_free(store);
}

struct testcase_t metrics_tests[] = {

  { "config", test_config, TT_FORK, NULL, NULL },
  { "connection", test_connection, TT_FORK, NULL, NULL },
  { "prometheus", test_prometheus, TT_FORK, NULL, NULL },
  { "prometheus_histogram", test_prometheus_histogram, TT_FORK, NULL, NULL },
  { "store", test_store, TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};

