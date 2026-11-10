/* Copyright (c) 2015-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define CONNECTION_PRIVATE
#define MAINLOOP_PRIVATE
#define CONNECTION_OR_PRIVATE

#include "core/or/or.h"
#include "test/test.h"

#include "app/config/config.h"
#include "app/config/or_options_st.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_edge.h"
#include "feature/hs/hs_common.h"
#include "core/mainloop/mainloop.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/dircommon/directory.h"
#include "core/or/connection_or.h"
#include "lib/net/resolve.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/evloop/token_bucket.h"

#include "test/test_connection.h"
#include "test/test_helpers.h"

#include "feature/dircommon/dir_connection_st.h"
#include "core/or/entry_connection_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/or_connection_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "core/or/socks_request_st.h"

static void * test_conn_get_basic_setup(const struct testcase_t *tc);
static int test_conn_get_basic_teardown(const struct testcase_t *tc,
                                        void *arg);

static void * test_conn_get_rsrc_setup(const struct testcase_t *tc);
static int test_conn_get_rsrc_teardown(const struct testcase_t *tc,
                                        void *arg);

/* Arbitrary choice - IPv4 Directory Connection to localhost */
#define TEST_CONN_TYPE          (CONN_TYPE_DIR)
/* We assume every machine has IPv4 localhost, is that ok? */
#define TEST_CONN_ADDRESS       "127.0.0.1"
#define TEST_CONN_PORT          (12345)
#define TEST_CONN_ADDRESS_PORT  "127.0.0.1:12345"
#define TEST_CONN_FAMILY        (AF_INET)
#define TEST_CONN_STATE         (DIR_CONN_STATE_MIN_)
#define TEST_CONN_ADDRESS_2     "127.0.0.2"

#define TEST_CONN_BASIC_PURPOSE (DIR_PURPOSE_MIN_)

#define TEST_CONN_REND_ADDR     "cfs3rltphxxvabci"
#define TEST_CONN_REND_PURPOSE  (DIR_PURPOSE_FETCH_RENDDESC_V2)
#define TEST_CONN_REND_PURPOSE_SUCCESSFUL (DIR_PURPOSE_HAS_FETCHED_RENDDESC_V2)
#define TEST_CONN_REND_TYPE_2   (CONN_TYPE_AP)
#define TEST_CONN_REND_ADDR_2   "icbavxxhptlr3sfc"

#define TEST_CONN_RSRC          (networkstatus_get_flavor_name(FLAV_MICRODESC))
#define TEST_CONN_RSRC_PURPOSE  (DIR_PURPOSE_FETCH_CONSENSUS)
#define TEST_CONN_RSRC_STATE_SUCCESSFUL (DIR_CONN_STATE_CLIENT_FINISHED)
#define TEST_CONN_RSRC_2        (networkstatus_get_flavor_name(FLAV_NS))

#define TEST_CONN_DL_STATE      (DIR_CONN_STATE_CLIENT_READING)

/* see AP_CONN_STATE_IS_UNATTACHED() */
#define TEST_CONN_UNATTACHED_STATE (AP_CONN_STATE_CIRCUIT_WAIT)
#define TEST_CONN_ATTACHED_STATE   (AP_CONN_STATE_CONNECT_WAIT)

void
test_conn_lookup_addr_helper(const char *address, int family, tor_addr_t *addr)
{
  int rv = 0;

  tt_assert(addr);

  rv = tor_addr_lookup(address, family, addr);
  /* XXXX - should we retry on transient failure? */
  tt_int_op(rv, OP_EQ, 0);
  tt_assert(tor_addr_is_loopback(addr));
  tt_assert(tor_addr_is_v4(addr));

  return;

 done:
  tor_addr_make_null(addr, TEST_CONN_FAMILY);
}

static void *
test_conn_get_basic_setup(const struct testcase_t *tc)
{
  (void)tc;
  return test_conn_get_connection(TEST_CONN_STATE, TEST_CONN_TYPE,
                                  TEST_CONN_BASIC_PURPOSE);
}

static int
test_conn_get_basic_teardown(const struct testcase_t *tc, void *arg)
{
  (void)tc;
  connection_t *conn = arg;

  tt_assert(conn);
  assert_connection_ok(conn, time(NULL));

  /* teardown the connection as fast as possible */
  if (conn->linked_conn) {
    assert_connection_ok(conn->linked_conn, time(NULL));

    /* We didn't call tor_libevent_initialize(), so event_base was NULL,
     * so we can't rely on connection_unregister_events() use of event_del().
     */
    tor_event_free(conn->linked_conn->read_event);
    tor_event_free(conn->linked_conn->write_event);

    if (!conn->linked_conn->marked_for_close) {
      connection_close_immediate(conn->linked_conn);
      if (CONN_IS_EDGE(conn->linked_conn)) {
        /* Suppress warnings about all the stuff we didn't do */
        TO_EDGE_CONN(conn->linked_conn)->edge_has_sent_end = 1;
        TO_EDGE_CONN(conn->linked_conn)->end_reason =
          END_STREAM_REASON_INTERNAL;
        if (conn->linked_conn->type == CONN_TYPE_AP) {
          TO_ENTRY_CONN(conn->linked_conn)->socks_request->has_finished = 1;
        }
      }
      connection_mark_for_close(conn->linked_conn);
    }

    close_closeable_connections();
  }

  /* We didn't set the events up properly, so we can't use event_del() in
   * close_closeable_connections() > connection_free()
   * > connection_unregister_events() */
  tor_event_free(conn->read_event);
  tor_event_free(conn->write_event);

  if (!conn->marked_for_close) {
    connection_close_immediate(conn);
    if (CONN_IS_EDGE(conn)) {
      /* Suppress warnings about all the stuff we didn't do */
      TO_EDGE_CONN(conn)->edge_has_sent_end = 1;
      TO_EDGE_CONN(conn)->end_reason = END_STREAM_REASON_INTERNAL;
      if (conn->type == CONN_TYPE_AP) {
        TO_ENTRY_CONN(conn)->socks_request->has_finished = 1;
      }
    }
    connection_mark_for_close(conn);
  }

  close_closeable_connections();

  /* The unit test will fail if we return 0 */
  return 1;

  /* When conn == NULL, we can't cleanup anything */
 done:
  return 0;
}

static dir_connection_t *
test_conn_download_status_add_a_connection(const char *resource)
{
  dir_connection_t *conn = DOWNCAST(dir_connection_t,
                                    test_conn_get_connection(
                                                      TEST_CONN_STATE,
                                                      TEST_CONN_TYPE,
                                                      TEST_CONN_RSRC_PURPOSE));

  tt_assert(conn);
  assert_connection_ok(&conn->base_, time(NULL));

  /* Replace the existing resource with the one we want */
  if (resource) {
    if (conn->requested_resource) {
      tor_free(conn->requested_resource);
    }
    conn->requested_resource = tor_strdup(resource);
    assert_connection_ok(&conn->base_, time(NULL));
  }

  return conn;

 done:
  test_conn_get_rsrc_teardown(NULL, conn);
  return NULL;
}

static void *
test_conn_get_rsrc_setup(const struct testcase_t *tc)
{
  (void)tc;
  return test_conn_download_status_add_a_connection(TEST_CONN_RSRC);
}

static int
test_conn_get_rsrc_teardown(const struct testcase_t *tc, void *arg)
{
  int rv = 0;

  connection_t *conn = (connection_t *)arg;
  tt_assert(conn);
  assert_connection_ok(conn, time(NULL));

  if (conn->type == CONN_TYPE_DIR) {
    dir_connection_t *dir_conn = DOWNCAST(dir_connection_t, arg);

    tt_assert(dir_conn);
    assert_connection_ok(&dir_conn->base_, time(NULL));

    /* avoid a last-ditch attempt to refetch the consensus */
    dir_conn->base_.state = TEST_CONN_RSRC_STATE_SUCCESSFUL;
    assert_connection_ok(&dir_conn->base_, time(NULL));
  }

  /* connection_free_() cleans up requested_resource */
  rv = test_conn_get_basic_teardown(tc, conn);

 done:
  return rv;
}

static void *
test_conn_download_status_setup(const struct testcase_t *tc)
{
  return (void*)tc;
}

static int
test_conn_download_status_teardown(const struct testcase_t *tc, void *arg)
{
  (void)arg;
  int rv = 0;

  /* Ignore arg, and just loop through the connection array */
  SMARTLIST_FOREACH_BEGIN(get_connection_array(), connection_t *, conn) {
    if (conn) {
      assert_connection_ok(conn, time(NULL));

      /* connection_free_() cleans up requested_resource */
      rv = test_conn_get_rsrc_teardown(tc, conn);
      tt_int_op(rv, OP_EQ, 1);
    }
  } SMARTLIST_FOREACH_END(conn);

 done:
  return rv;
}

static void *
test_conn_proxy_connect_setup(const struct testcase_t *tc)
{
  return test_conn_get_proxy_or_connection(*(unsigned int *)tc->setup_data);
}

static int
test_conn_proxy_connect_teardown(const struct testcase_t *tc, void *arg)
{
  (void)tc;
  or_connection_t *conn = arg;

  tt_assert(conn);
  assert_connection_ok(&conn->base_, time(NULL));

 done:
  return 1;
}

/* Like connection_ap_make_link(), but does much less */
static connection_t *
test_conn_get_linked_connection(connection_t *l_conn, uint8_t state)
{
  tt_assert(l_conn);
  assert_connection_ok(l_conn, time(NULL));

  /* AP connections don't seem to have purposes */
  connection_t *conn = test_conn_get_connection(state, CONN_TYPE_AP,
                                                       0);

  tt_assert(conn);
  assert_connection_ok(conn, time(NULL));

  conn->linked = 1;
  l_conn->linked = 1;
  conn->linked_conn = l_conn;
  l_conn->linked_conn = conn;
  /* we never opened a real socket, so we can just overwrite it */
  conn->s = TOR_INVALID_SOCKET;
  l_conn->s = TOR_INVALID_SOCKET;

  assert_connection_ok(conn, time(NULL));
  assert_connection_ok(l_conn, time(NULL));

  return conn;

 done:
  test_conn_download_status_teardown(NULL, NULL);
  return NULL;
}

static struct testcase_setup_t test_conn_get_basic_st = {
  test_conn_get_basic_setup, test_conn_get_basic_teardown
};

static struct testcase_setup_t test_conn_get_rsrc_st = {
  test_conn_get_rsrc_setup, test_conn_get_rsrc_teardown
};

static struct testcase_setup_t test_conn_download_status_st = {
  test_conn_download_status_setup, test_conn_download_status_teardown
};

static struct testcase_setup_t test_conn_proxy_connect_st = {
  test_conn_proxy_connect_setup, test_conn_proxy_connect_teardown
};

static void
test_conn_get_basic(void *arg)
{
  connection_t *conn = (connection_t*)arg;
  tor_addr_t addr, addr2;

  tt_assert(conn);
  assert_connection_ok(conn, time(NULL));

  test_conn_lookup_addr_helper(TEST_CONN_ADDRESS, TEST_CONN_FAMILY, &addr);
  tt_assert(!tor_addr_is_null(&addr));
  test_conn_lookup_addr_helper(TEST_CONN_ADDRESS_2, TEST_CONN_FAMILY, &addr2);
  tt_assert(!tor_addr_is_null(&addr2));

  /* Check that we get this connection back when we search for it by
   * its attributes, but get NULL when we supply a different value. */

  tt_assert(connection_get_by_global_id(conn->global_identifier) == conn);
  tt_ptr_op(connection_get_by_global_id(!conn->global_identifier), OP_EQ,
            NULL);

  tt_assert(connection_get_by_type(conn->type) == conn);
  tt_assert(connection_get_by_type(TEST_CONN_TYPE) == conn);
  tt_ptr_op(connection_get_by_type(!conn->type), OP_EQ, NULL);
  tt_ptr_op(connection_get_by_type(!TEST_CONN_TYPE), OP_EQ, NULL);

  tt_assert(connection_get_by_type_state(conn->type, conn->state)
            == conn);
  tt_assert(connection_get_by_type_state(TEST_CONN_TYPE, TEST_CONN_STATE)
            == conn);
  tt_assert(connection_get_by_type_state(!conn->type, !conn->state)
            == NULL);
  tt_assert(connection_get_by_type_state(!TEST_CONN_TYPE, !TEST_CONN_STATE)
            == NULL);

  /* Match on the connection fields themselves */
  tt_assert(connection_get_by_type_addr_port_purpose(conn->type,
                                                     &conn->addr,
                                                     conn->port,
                                                     conn->purpose)
            == conn);
  /* Match on the original inputs to the connection */
  tt_assert(connection_get_by_type_addr_port_purpose(TEST_CONN_TYPE,
                                                     &conn->addr,
                                                     conn->port,
                                                     conn->purpose)
            == conn);
  tt_assert(connection_get_by_type_addr_port_purpose(conn->type,
                                                     &addr,
                                                     conn->port,
                                                     conn->purpose)
            == conn);
  tt_assert(connection_get_by_type_addr_port_purpose(conn->type,
                                                     &conn->addr,
                                                     TEST_CONN_PORT,
                                                     conn->purpose)
            == conn);
  tt_assert(connection_get_by_type_addr_port_purpose(conn->type,
                                                     &conn->addr,
                                                     conn->port,
                                                     TEST_CONN_BASIC_PURPOSE)
            == conn);
  tt_assert(connection_get_by_type_addr_port_purpose(TEST_CONN_TYPE,
                                                     &addr,
                                                     TEST_CONN_PORT,
                                                     TEST_CONN_BASIC_PURPOSE)
            == conn);
  /* Then try each of the not-matching combinations */
  tt_assert(connection_get_by_type_addr_port_purpose(!conn->type,
                                                     &conn->addr,
                                                     conn->port,
                                                     conn->purpose)
            == NULL);
  tt_assert(connection_get_by_type_addr_port_purpose(conn->type,
                                                     &addr2,
                                                     conn->port,
                                                     conn->purpose)
            == NULL);
  tt_assert(connection_get_by_type_addr_port_purpose(conn->type,
                                                     &conn->addr,
                                                     !conn->port,
                                                     conn->purpose)
            == NULL);
  tt_assert(connection_get_by_type_addr_port_purpose(conn->type,
                                                     &conn->addr,
                                                     conn->port,
                                                     !conn->purpose)
            == NULL);
  /* Then try everything not-matching */
  tt_assert(connection_get_by_type_addr_port_purpose(!conn->type,
                                                     &addr2,
                                                     !conn->port,
                                                     !conn->purpose)
            == NULL);
  tt_assert(connection_get_by_type_addr_port_purpose(!TEST_CONN_TYPE,
                                                     &addr2,
                                                     !TEST_CONN_PORT,
                                                     !TEST_CONN_BASIC_PURPOSE)
            == NULL);

 done:
  ;
}

#define sl_is_conn_assert(sl_input, conn) \
  do {                                               \
    the_sl = (sl_input);                             \
    tt_int_op(smartlist_len((the_sl)), OP_EQ, 1);         \
    tt_assert(smartlist_get((the_sl), 0) == (conn)); \
    smartlist_free(the_sl); the_sl = NULL;           \
  } while (0)

#define sl_no_conn_assert(sl_input)          \
  do {                                       \
    the_sl = (sl_input);                     \
    tt_int_op(smartlist_len((the_sl)), OP_EQ, 0); \
    smartlist_free(the_sl); the_sl = NULL;   \
  } while (0)

static void
test_conn_get_rsrc(void *arg)
{
  dir_connection_t *conn = DOWNCAST(dir_connection_t, arg);
  smartlist_t *the_sl = NULL;
  tt_assert(conn);
  assert_connection_ok(&conn->base_, time(NULL));

  sl_is_conn_assert(connection_dir_list_by_purpose_and_resource(
                                                    conn->base_.purpose,
                                                    conn->requested_resource),
                    conn);
  sl_is_conn_assert(connection_dir_list_by_purpose_and_resource(
                                                    TEST_CONN_RSRC_PURPOSE,
                                                    TEST_CONN_RSRC),
                    conn);
  sl_no_conn_assert(connection_dir_list_by_purpose_and_resource(
                                                    !conn->base_.purpose,
                                                    ""));
  sl_no_conn_assert(connection_dir_list_by_purpose_and_resource(
                                                    !TEST_CONN_RSRC_PURPOSE,
                                                    TEST_CONN_RSRC_2));

  sl_is_conn_assert(connection_dir_list_by_purpose_resource_and_state(
                                                    conn->base_.purpose,
                                                    conn->requested_resource,
                                                    conn->base_.state),
                    conn);
  sl_is_conn_assert(connection_dir_list_by_purpose_resource_and_state(
                                                    TEST_CONN_RSRC_PURPOSE,
                                                    TEST_CONN_RSRC,
                                                    TEST_CONN_STATE),
                    conn);
  sl_no_conn_assert(connection_dir_list_by_purpose_resource_and_state(
                                                    !conn->base_.purpose,
                                                    "",
                                                    !conn->base_.state));
  sl_no_conn_assert(connection_dir_list_by_purpose_resource_and_state(
                                                    !TEST_CONN_RSRC_PURPOSE,
                                                    TEST_CONN_RSRC_2,
                                                    !TEST_CONN_STATE));

  tt_int_op(connection_dir_count_by_purpose_and_resource(
                 conn->base_.purpose, conn->requested_resource),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                 TEST_CONN_RSRC_PURPOSE, TEST_CONN_RSRC),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                 !conn->base_.purpose, ""),
            OP_EQ, 0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                 !TEST_CONN_RSRC_PURPOSE, TEST_CONN_RSRC_2),
            OP_EQ, 0);

  tt_int_op(connection_dir_count_by_purpose_resource_and_state(
                 conn->base_.purpose, conn->requested_resource,
                 conn->base_.state),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_resource_and_state(
                 TEST_CONN_RSRC_PURPOSE, TEST_CONN_RSRC, TEST_CONN_STATE),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_resource_and_state(
                 !conn->base_.purpose, "", !conn->base_.state),
            OP_EQ, 0);
  tt_int_op(connection_dir_count_by_purpose_resource_and_state(
                 !TEST_CONN_RSRC_PURPOSE, TEST_CONN_RSRC_2, !TEST_CONN_STATE),
            OP_EQ, 0);

 done:
  smartlist_free(the_sl);
}

static void
test_conn_download_status(void *arg)
{
  dir_connection_t *conn = NULL;
  dir_connection_t *conn2 = NULL;
  dir_connection_t *conn4 = NULL;
  connection_t *ap_conn = NULL;

  const struct testcase_t *tc = arg;
  consensus_flavor_t usable_flavor =
    networkstatus_parse_flavor_name((const char*) tc->setup_data);

  /* The "other flavor" trick only works if there are two flavors */
  tor_assert(N_CONSENSUS_FLAVORS == 2);
  consensus_flavor_t other_flavor = ((usable_flavor == FLAV_NS)
                                     ? FLAV_MICRODESC
                                     : FLAV_NS);
  const char *res = networkstatus_get_flavor_name(usable_flavor);
  const char *other_res = networkstatus_get_flavor_name(other_flavor);

  /* no connections */
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 0);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* one connection, not downloading */
  conn = test_conn_download_status_add_a_connection(res);
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 0);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* one connection, downloading but not linked (not possible on a client,
   * but possible on a relay) */
  conn->base_.state = TEST_CONN_DL_STATE;
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 0);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* one connection, downloading and linked, but not yet attached */
  ap_conn = test_conn_get_linked_connection(TO_CONN(conn),
                                            TEST_CONN_UNATTACHED_STATE);
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 0);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

    /* one connection, downloading and linked and attached */
  ap_conn->state = TEST_CONN_ATTACHED_STATE;
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 1);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* one connection, linked and attached but not downloading */
  conn->base_.state = TEST_CONN_STATE;
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 0);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* two connections, both not downloading */
  conn2 = test_conn_download_status_add_a_connection(res);
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 0);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 2);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* two connections, one downloading */
  conn->base_.state = TEST_CONN_DL_STATE;
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 1);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 2);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);
  conn->base_.state = TEST_CONN_STATE;

  /* more connections, all not downloading */
  /* ignore the return value, it's free'd using the connection list */
  (void)test_conn_download_status_add_a_connection(res);
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 0);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 3);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* more connections, one downloading */
  conn->base_.state = TEST_CONN_DL_STATE;
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 1);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 3);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* more connections, two downloading (should never happen, but needs
   * to be tested for completeness) */
  conn2->base_.state = TEST_CONN_DL_STATE;
  /* ignore the return value, it's free'd using the connection list */
  (void)test_conn_get_linked_connection(TO_CONN(conn2),
                                        TEST_CONN_ATTACHED_STATE);
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 1);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 3);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);
  conn->base_.state = TEST_CONN_STATE;

  /* more connections, a different one downloading */
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 1);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 3);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                           TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 0);

  /* a connection for the other flavor (could happen if a client is set to
   * cache directory documents), one preferred flavor downloading
   */
  conn4 = test_conn_download_status_add_a_connection(other_res);
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 1);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            0);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                   TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 3);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                   TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 1);

  /* a connection for the other flavor (could happen if a client is set to
   * cache directory documents), both flavors downloading
   */
  conn4->base_.state = TEST_CONN_DL_STATE;
  /* ignore the return value, it's free'd using the connection list */
  (void)test_conn_get_linked_connection(TO_CONN(conn4),
                                        TEST_CONN_ATTACHED_STATE);
  tt_int_op(networkstatus_consensus_is_already_downloading(res), OP_EQ, 1);
  tt_int_op(networkstatus_consensus_is_already_downloading(other_res), OP_EQ,
            1);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                   TEST_CONN_RSRC_PURPOSE, res),
            OP_EQ, 3);
  tt_int_op(connection_dir_count_by_purpose_and_resource(
                                   TEST_CONN_RSRC_PURPOSE, other_res),
            OP_EQ, 1);

 done:
  /* the teardown function removes all the connections in the global list*/;
}

static void
test_conn_https_proxy_connect(void *arg)
{
  size_t sz;
  char *buf = NULL;
  or_connection_t *conn = arg;

  MOCK(connection_or_change_state, mock_connection_or_change_state);

  tt_int_op(conn->base_.proxy_state, OP_EQ, PROXY_HTTPS_WANT_CONNECT_OK);

  buf = buf_get_contents(conn->base_.outbuf, &sz);
  tt_str_op(buf, OP_EQ, "CONNECT 127.0.0.1:12345 HTTP/1.0\r\n\r\n");

 done:
  UNMOCK(connection_or_change_state);
  tor_free(buf);
}

static int handshake_start_called = 0;

static int
handshake_start(or_connection_t *conn, int receiving)
{
  (void)receiving;

  tor_assert(conn);

  handshake_start_called = 1;
  return 0;
}

static void
test_conn_haproxy_proxy_connect(void *arg)
{
  size_t sz;
  char *buf = NULL;
  or_connection_t *conn = arg;

  MOCK(connection_or_change_state, mock_connection_or_change_state);
  MOCK(connection_tls_start_handshake, handshake_start);

  tt_int_op(conn->base_.proxy_state, OP_EQ, PROXY_HAPROXY_WAIT_FOR_FLUSH);

  buf = buf_get_contents(conn->base_.outbuf, &sz);
  tt_str_op(buf, OP_EQ, "PROXY TCP4 0.0.0.0 127.0.0.1 0 12345\r\n");

  connection_or_finished_flushing(conn);

  tt_int_op(conn->base_.proxy_state, OP_EQ, PROXY_CONNECTED);
  tt_int_op(handshake_start_called, OP_EQ, 1);

 done:
  UNMOCK(connection_or_change_state);
  UNMOCK(connection_tls_start_handshake);
  tor_free(buf);
}

static node_t test_node;

static node_t *
mock_node_get_mutable_by_id(const char *digest)
{
  (void) digest;
  static routerinfo_t node_ri;
  memset(&node_ri, 0, sizeof(node_ri));

  test_node.ri = &node_ri;
  memset(test_node.identity, 'c', sizeof(test_node.identity));

  tor_addr_parse(&node_ri.ipv4_addr, "18.0.0.1");
  node_ri.ipv4_orport = 1;

  return &test_node;
}

static const node_t *
mock_node_get_by_id(const char *digest)
{
  (void) digest;
  memset(test_node.identity, 'c', sizeof(test_node.identity));
  return &test_node;
}

/* Test whether we correctly track failed connections between relays. */
static void
test_failed_orconn_tracker(void *arg)
{
  (void) arg;

  int can_connect;
  time_t now = 1281533250; /* 2010-08-11 13:27:30 UTC */
  (void) now;

  update_approx_time(now);

  /* Prepare the OR connection that will be used in this test */
  or_connection_t or_conn;
  memset(&or_conn, 0, sizeof(or_conn));
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&or_conn.canonical_orport.addr,
                                          "18.0.0.1"));
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&or_conn.base_.addr, "18.0.0.1"));
  or_conn.base_.port = 1;
  memset(or_conn.identity_digest, 'c', sizeof(or_conn.identity_digest));

  /* Check whether we can connect with an empty failure cache:
   * this should succeed */
  can_connect = should_connect_to_relay(&or_conn);
  tt_int_op(can_connect, OP_EQ, 1);

  /* Now add the destination to the failure cache */
  note_or_connect_failed(&or_conn);

  /* Check again: now it shouldn't connect */
  can_connect = should_connect_to_relay(&or_conn);
  tt_int_op(can_connect, OP_EQ, 0);

  /* Move time forward and check again: the cache should have been cleared and
   * now it should connect */
  now += 3600;
  update_approx_time(now);
  can_connect = should_connect_to_relay(&or_conn);
  tt_int_op(can_connect, OP_EQ, 1);

  /* Now mock the node_get_*by_id() functions to start using the node subsystem
   * optimization. */
  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(node_get_mutable_by_id, mock_node_get_mutable_by_id);

  /* Since we just started using the node subsystem it will allow connections
   * now */
  can_connect = should_connect_to_relay(&or_conn);
  tt_int_op(can_connect, OP_EQ, 1);

  /* Mark it as failed */
  note_or_connect_failed(&or_conn);

  /* Check that it shouldn't connect now */
  can_connect = should_connect_to_relay(&or_conn);
  tt_int_op(can_connect, OP_EQ, 0);

  /* Move time forward and check again: now it should connect  */
  now += 3600;
  update_approx_time(now);
  can_connect = should_connect_to_relay(&or_conn);
  tt_int_op(can_connect, OP_EQ, 1);

 done:
  ;
}

static void
test_conn_describe(void *arg)
{
  (void)arg;
  or_options_t *options = get_options_mutable();
  options->SafeLogging_ = SAFELOG_SCRUB_ALL;

  // Let's start with a listener connection since they're simple.
  connection_t *conn = connection_new(CONN_TYPE_OR_LISTENER, AF_INET);
  tor_addr_parse(&conn->addr, "44.22.11.11");
  conn->port = 80;
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR listener connection (ready) on 44.22.11.11:80");
  // If the address is unspec, we should still work.
  tor_addr_make_unspec(&conn->addr);
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR listener connection (ready) on <unset>:80");
  // Try making the address null.
  tor_addr_make_null(&conn->addr, AF_INET);
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR listener connection (ready) on 0.0.0.0:80");
  // What if the address is uninitialized? (This can happen if we log about the
  // connection before we set the address.)
  memset(&conn->addr, 0, sizeof(conn->addr));
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR listener connection (ready) on <unset>:80");
  connection_free_minimal(conn);

  // Try a unix socket.
  conn = connection_new(CONN_TYPE_CONTROL_LISTENER, AF_UNIX);
  conn->address = tor_strdup("/a/path/that/could/exist");
  tt_str_op(connection_describe(conn), OP_EQ,
            "Control listener connection (ready) on /a/path/that/could/exist");
  connection_free_minimal(conn);

  // Try an IPv6 address.
  conn = connection_new(CONN_TYPE_AP_LISTENER, AF_INET6);
  tor_addr_parse(&conn->addr, "ff00::3");
  conn->port = 9050;
  tt_str_op(connection_describe(conn), OP_EQ,
            "Socks listener connection (ready) on [ff00::3]:9050");
  connection_free_minimal(conn);

  // Now let's mess with exit connections.  They have some special issues.
  options->SafeLogging_ = SAFELOG_SCRUB_NONE;
  conn = connection_new(CONN_TYPE_EXIT, AF_INET);
  // If address and state are unset, we should say SOMETHING.
  tt_str_op(connection_describe(conn), OP_EQ,
            "Exit connection (uninitialized) to <unset> (DNS lookup pending)");
  // Now suppose that the address is set but we haven't resolved the hostname.
  conn->port = 443;
  conn->address = tor_strdup("www.torproject.org");
  conn->state = EXIT_CONN_STATE_RESOLVING;
  tt_str_op(connection_describe(conn), OP_EQ,
            "Exit connection (waiting for dest info) to "
            "www.torproject.org:443 (DNS lookup pending)");
  //  Now give it a hostname!
  tor_addr_parse(&conn->addr, "192.168.8.8");
  conn->state = EXIT_CONN_STATE_OPEN;
  tt_str_op(connection_describe(conn), OP_EQ,
            "Exit connection (open) to 192.168.8.8:443");
  // But what if safelogging is on?
  options->SafeLogging_ = SAFELOG_SCRUB_RELAY;
  tt_str_op(connection_describe(conn), OP_EQ,
            "Exit connection (open) to [scrubbed]");
  connection_free_minimal(conn);

  // Now at last we look at OR addresses, which are complicated.
  conn = connection_new(CONN_TYPE_OR, AF_INET6);
  conn->state = OR_CONN_STATE_OPEN;
  conn->port = 8080;
  tor_addr_parse(&conn->addr, "[ffff:3333:1111::2]");
  // This should get scrubbed, since the lack of a set ID means we might be
  // talking to a client.
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR connection (open) with [scrubbed]");
  // But now suppose we aren't safelogging? We'll get the address then.
  options->SafeLogging_ = SAFELOG_SCRUB_NONE;
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR connection (open) with [ffff:3333:1111::2]:8080");
  // Suppose we have an ID, so we know it isn't a client.
  TO_OR_CONN(conn)->identity_digest[3] = 7;
  options->SafeLogging_ = SAFELOG_SCRUB_RELAY; // back to safelogging.
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR connection (open) with [ffff:3333:1111::2]:8080 "
            "ID=<none> RSA_ID=0000000700000000000000000000000000000000");
  // Add a 'canonical address' that is the same as the one we have.
  tor_addr_parse(&TO_OR_CONN(conn)->canonical_orport.addr,
                 "[ffff:3333:1111::2]");
  TO_OR_CONN(conn)->canonical_orport.port = 8080;
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR connection (open) with [ffff:3333:1111::2]:8080 "
            "ID=<none> RSA_ID=0000000700000000000000000000000000000000");
  // Add a different 'canonical address'
  tor_addr_parse(&TO_OR_CONN(conn)->canonical_orport.addr,
                 "[ffff:3333:1111::8]");
  tt_str_op(connection_describe(conn), OP_EQ,
            "OR connection (open) with [ffff:3333:1111::2]:8080 "
            "ID=<none> RSA_ID=0000000700000000000000000000000000000000 "
            "canonical_addr=[ffff:3333:1111::8]:8080");

  // Clear identity_digest so that free_minimal won't complain.
  memset(TO_OR_CONN(conn)->identity_digest, 0, DIGEST_LEN);

 done:
  connection_free_minimal(conn);
}

/* Keep closed connections available so tests can inspect cleared indices. */
static void
mock_keep_closed_connection(connection_t *conn)
{
  (void)conn;
}

static connection_t *
new_list_test_connection(int linked)
{
  connection_t *conn = TO_CONN(dir_connection_new(AF_INET));
  conn->state = DIR_CONN_STATE_SERVER_WRITING;
  conn->purpose = DIR_PURPOSE_SERVER;
  conn->linked = linked;
  return conn;
}

/* Called while connection_free_ is mocked to retain the closed objects. */
static void
free_list_test_connections(connection_t **conns, unsigned n)
{
  for (unsigned i = 0; i < n; ++i) {
    if (!conns[i])
      continue;
    conns[i]->hold_open_until_flushed = 0;
    buf_clear(conns[i]->outbuf);
    if (!conns[i]->marked_for_close)
      connection_mark_for_close(conns[i]);
  }
  close_closeable_connections();
  UNMOCK(connection_free_);
  for (unsigned i = 0; i < n; ++i)
    connection_free(conns[i]);
}

static void
test_conn_list_indices(void *arg)
{
  (void)arg;
  connection_t *conns[3] = { NULL, NULL, NULL };
  tor_init_connection_lists();
  MOCK(connection_free_, mock_keep_closed_connection);

  for (unsigned i = 0; i < ARRAY_LENGTH(conns); ++i) {
    conns[i] = new_list_test_connection(1);
    tt_int_op(conns[i]->conn_array_index, OP_EQ, -1);
    tt_int_op(conns[i]->closeable_list_index, OP_EQ, -1);
    tt_int_op(conns[i]->active_linked_list_index, OP_EQ, -1);
    tt_assert(!connection_in_array(conns[i]));
    tt_assert(!connection_is_on_closeable_list(conns[i]));
    tt_int_op(connection_add(conns[i]), OP_EQ, 0);
    connection_start_reading(conns[i]);
    tt_int_op(conns[i]->active_linked_list_index, OP_EQ, i);
  }

  /* Removing the middle entry moves the last entry into its slot. Repeated
   * starts and stops must neither duplicate entries nor remove them twice. */
  connection_stop_reading(conns[1]);
  connection_stop_reading(conns[1]);
  tt_int_op(conns[1]->active_linked_list_index, OP_EQ, -1);
  tt_int_op(conns[2]->active_linked_list_index, OP_EQ, 1);
  connection_start_reading(conns[2]);
  connection_start_reading(conns[1]);
  connection_start_reading(conns[1]);
  tt_int_op(conns[1]->active_linked_list_index, OP_EQ, 2);
  connection_stop_reading(conns[1]);
  tt_int_op(conns[1]->active_linked_list_index, OP_EQ, -1);

  tt_int_op(connection_remove(conns[1]), OP_EQ, 0);
  tt_int_op(conns[1]->conn_array_index, OP_EQ, -1);
  tt_assert(!connection_in_array(conns[1]));
  tt_int_op(conns[2]->conn_array_index, OP_EQ, 1);
  tt_assert(connection_in_array(conns[2]));
  tt_int_op(connection_add(conns[1]), OP_EQ, 0);
  tt_int_op(conns[1]->conn_array_index, OP_EQ, 2);
  tt_int_op(connection_remove(conns[1]), OP_EQ, 0);
  tt_int_op(conns[1]->conn_array_index, OP_EQ, -1);

 done:
  free_list_test_connections(conns, ARRAY_LENGTH(conns));
}

#ifndef ALL_BUGS_ARE_FATAL
/* Exercise recovery and object lifetime, rather than diagnostic wording. */
static void
test_conn_list_index_recovery(void *arg)
{
  (void)arg;
  connection_t *conns[4] = { NULL, NULL, NULL, NULL };
  tor_init_connection_lists();
  MOCK(connection_free_, mock_keep_closed_connection);
  tor_capture_bugs_(16);
  for (unsigned i = 0; i < 3; ++i) {
    conns[i] = new_list_test_connection(1);
    tt_int_op(connection_add(conns[i]), OP_EQ, 0);
    connection_start_reading(conns[i]);
  }

  /* An index pointing at another connection must not remove that entry. */
  conns[1]->active_linked_list_index = 0;
  connection_stop_reading(conns[1]);
  tt_int_op(conns[0]->active_linked_list_index, OP_EQ, 0);
  tt_int_op(conns[2]->active_linked_list_index, OP_EQ, 1);
  conns[0]->active_on_link = 0;
  connection_start_reading(conns[0]);
  connection_start_reading(conns[1]);
  tt_int_op(conns[1]->active_linked_list_index, OP_EQ, 2);

  /* Recover a lost index, and an active flag with no list entry. */
  conns[2]->active_linked_list_index = -1;
  connection_stop_reading(conns[2]);
  tt_int_op(conns[1]->active_linked_list_index, OP_EQ, 1);
  conns[2]->active_on_link = 1;
  connection_stop_reading(conns[2]);
  tt_assert(!conns[2]->active_on_link);

  conns[1]->conn_array_index = 0;
  tt_int_op(connection_remove(conns[1]), OP_EQ, 0);
  tt_assert(connection_in_array(conns[0]));
  tt_int_op(conns[2]->conn_array_index, OP_EQ, 1);
  conns[1]->conn_array_index = 100;
  tt_assert(!connection_in_array(conns[1]));
  tt_int_op(conns[1]->conn_array_index, OP_EQ, -1);
  conns[2]->conn_array_index = -1;
  tt_int_op(connection_remove(conns[2]), OP_EQ, 0);

  for (unsigned i = 0; i < 3; ++i)
    connection_mark_for_close(conns[i]);
  conns[0]->closeable_list_index = 100;
  tt_assert(connection_is_on_closeable_list(conns[0]));
  tt_int_op(conns[0]->closeable_list_index, OP_EQ, 0);
  add_connection_to_closeable_list(conns[0]); /* must not duplicate it */
  conns[0]->closeable_list_index = -1;
  close_closeable_connections();
  for (unsigned i = 0; i < 3; ++i) {
    tt_int_op(conns[i]->conn_array_index, OP_EQ, -1);
    tt_int_op(conns[i]->closeable_list_index, OP_EQ, -1);
    tt_int_op(conns[i]->active_linked_list_index, OP_EQ, -1);
  }

  /* Direct free must remove unexpected active membership before freeing. */
  conns[3] = new_list_test_connection(1);
  tt_int_op(connection_add(conns[3]), OP_EQ, 0);
  connection_start_reading(conns[3]);
  tt_int_op(conns[3]->active_linked_list_index, OP_EQ, 0);
  tt_int_op(connection_remove(conns[3]), OP_EQ, 0);
  connection_free___real(conns[3]);
  conns[3] = NULL;

 done:
  free_list_test_connections(conns, ARRAY_LENGTH(conns));
  tor_end_capture_bugs_();
}
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

static connection_t *close_on_linked_wakeup;

static void
mock_read_and_queue_close(connection_t *conn)
{
  connection_start_reading__real(conn);
  if (close_on_linked_wakeup) {
    connection_mark_for_close(close_on_linked_wakeup);
    close_on_linked_wakeup = NULL;
  }
}

static void
test_conn_linked_list_teardown(void *arg)
{
  (void)arg;
  connection_t *conns[3] = { NULL, NULL, NULL };
  tor_init_connection_lists();
  MOCK(connection_free_, mock_keep_closed_connection);
  for (unsigned i = 0; i < ARRAY_LENGTH(conns); ++i) {
    conns[i] = new_list_test_connection(1);
    tt_int_op(connection_add(conns[i]), OP_EQ, 0);
  }
  connection_link_connections(conns[0], conns[1]);
  connection_start_reading(conns[1]);
  connection_start_reading(conns[2]);

  /* Unlinking the first connection wakes its peer. Queue another closure in
   * that callback, before the first connection leaves the closeable list. */
  close_on_linked_wakeup = conns[2];
  MOCK(connection_start_reading, mock_read_and_queue_close);
  connection_mark_for_close(conns[0]);
  tt_int_op(conns[0]->closeable_list_index, OP_EQ, 0);
  close_closeable_connections();
  tt_ptr_op(close_on_linked_wakeup, OP_EQ, NULL);
  tt_ptr_op(conns[1]->linked_conn, OP_EQ, NULL);
  tt_assert(conns[1]->active_on_link);
  tt_int_op(conns[1]->active_linked_list_index, OP_EQ, 0);
  tt_int_op(conns[1]->conn_array_index, OP_EQ, 0);
  for (unsigned i = 0; i < ARRAY_LENGTH(conns); i += 2) {
    tt_int_op(conns[i]->conn_array_index, OP_EQ, -1);
    tt_int_op(conns[i]->closeable_list_index, OP_EQ, -1);
    tt_int_op(conns[i]->active_linked_list_index, OP_EQ, -1);
    tt_assert(!conns[i]->active_on_link);
  }
  connection_mark_for_close(conns[1]);
  close_closeable_connections();
  tt_int_op(conns[1]->active_linked_list_index, OP_EQ, -1);
  tt_int_op(smartlist_len(get_connection_array()), OP_EQ, 0);

 done:
  close_on_linked_wakeup = NULL;
  UNMOCK(connection_start_reading);
  free_list_test_connections(conns, ARRAY_LENGTH(conns));
}

static void
test_conn_closeable_list_delayed(void *arg)
{
  (void)arg;
  connection_t *conns[3] = { NULL, NULL, NULL };
  tor_socket_t sockets[2] = { TOR_INVALID_SOCKET, TOR_INVALID_SOCKET };
  tor_init_connection_lists();
  MOCK(connection_free_, mock_keep_closed_connection);
  connection_bucket_init();
  token_bucket_rw_dec_write(&global_bucket,
                            token_bucket_rw_get_write(&global_bucket));
  tt_int_op(tor_socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), OP_EQ, 0);
  for (unsigned i = 0; i < ARRAY_LENGTH(conns); ++i) {
    conns[i] = new_list_test_connection(i != 0);
    if (i == 0) {
      conns[i]->s = sockets[0];
      sockets[0] = TOR_INVALID_SOCKET;
      conns[i]->always_rate_limit_as_remote = 1;
    }
    tt_int_op(connection_add(conns[i]), OP_EQ, 0);
    connection_mark_for_close(conns[i]);
    tt_int_op(conns[i]->closeable_list_index, OP_EQ, i);
  }
  conns[0]->hold_open_until_flushed = 1;
  buf_add(conns[0]->outbuf, "x", 1);

  /* Keep the first entry pending while removing the middle and last entries.
   * The connection moved into the middle slot must still be closed. */
  close_closeable_connections();
  tt_assert(connection_in_array(conns[0]));
  tt_assert(connection_is_on_closeable_list(conns[0]));
  tt_int_op(conns[0]->closeable_list_index, OP_EQ, 0);
  for (unsigned i = 1; i < ARRAY_LENGTH(conns); ++i) {
    tt_int_op(conns[i]->conn_array_index, OP_EQ, -1);
    tt_int_op(conns[i]->closeable_list_index, OP_EQ, -1);
  }
  buf_clear(conns[0]->outbuf);
  close_closeable_connections();
  tt_int_op(conns[0]->closeable_list_index, OP_EQ, -1);
  tt_int_op(smartlist_len(get_connection_array()), OP_EQ, 0);

 done:
  free_list_test_connections(conns, ARRAY_LENGTH(conns));
  for (unsigned i = 0; i < ARRAY_LENGTH(sockets); ++i) {
    if (SOCKET_OK(sockets[i]))
      tor_close_socket(sockets[i]);
  }
}

#ifndef COCCI
#define CONNECTION_TESTCASE(name, fork, setup)                           \
  { #name, test_conn_##name, fork, &setup, NULL }

#define STR(x) #x
/* where arg is an expression (constant, variable, compound expression) */
#define CONNECTION_TESTCASE_ARG(name, extra, fork, setup, arg)          \
  { STR(name)"/"extra,                                                  \
      test_conn_##name,                                                 \
      (fork),                                                           \
      &(setup),                                                         \
      (void *)(arg) }
#endif /* !defined(COCCI) */

static const unsigned int PROXY_CONNECT_ARG = PROXY_CONNECT;
static const unsigned int PROXY_HAPROXY_ARG = PROXY_HAPROXY;

struct testcase_t connection_tests[] = {
  { "list_indices", test_conn_list_indices, TT_FORK, NULL, NULL },
#ifndef ALL_BUGS_ARE_FATAL
  { "list_index_recovery", test_conn_list_index_recovery,
    TT_FORK, NULL, NULL },
#endif /* !defined(ALL_BUGS_ARE_FATAL) */
  { "linked_list_teardown", test_conn_linked_list_teardown,
    TT_FORK, NULL, NULL },
  { "closeable_list_delayed", test_conn_closeable_list_delayed,
    TT_FORK, NULL, NULL },
  CONNECTION_TESTCASE(get_basic, TT_FORK, test_conn_get_basic_st),
  CONNECTION_TESTCASE(get_rsrc,  TT_FORK, test_conn_get_rsrc_st),

  CONNECTION_TESTCASE_ARG(download_status, "microdesc", TT_FORK,
                          test_conn_download_status_st, "microdesc"),
  CONNECTION_TESTCASE_ARG(download_status, "ns", TT_FORK,
                          test_conn_download_status_st, "ns"),

  CONNECTION_TESTCASE_ARG(https_proxy_connect, "https", TT_FORK,
                          test_conn_proxy_connect_st, &PROXY_CONNECT_ARG),
  CONNECTION_TESTCASE_ARG(haproxy_proxy_connect, "haproxy", TT_FORK,
                          test_conn_proxy_connect_st, &PROXY_HAPROXY_ARG),

  //CONNECTION_TESTCASE(func_suffix, TT_FORK, setup_func_pair),
  { "failed_orconn_tracker", test_failed_orconn_tracker, TT_FORK, NULL, NULL },
  { "describe", test_conn_describe, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
