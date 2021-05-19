/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_helpers.c
 * \brief Some helper functions to avoid code duplication in unit tests.
 */

#define ROUTERLIST_PRIVATE
#define CONFIG_PRIVATE
#define CONNECTION_PRIVATE
#define CONNECTION_OR_PRIVATE
#define MAINLOOP_PRIVATE

#include "orconfig.h"
#include "core/or/or.h"

#include "lib/buf/buffers.h"
#include "lib/confmgt/confmgt.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_naming.h"
#include "lib/encoding/confline.h"
#include "lib/net/resolve.h"
#include "lib/pubsub/pubsub_build.h"
#include "lib/pubsub/pubsub_connect.h"

#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/connection_or.h"
#include "core/or/crypt_path.h"
#include "core/or/relay.h"

#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"

#include "app/config/config.h"
#include "app/main/subsysmgr.h"

#include "core/or/cell_st.h"
#include "core/or/connection_st.h"
#include "core/or/cpath_build_state_st.h"
#include "core/or/crypt_path_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/or_connection_st.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerlist_st.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#else
#include <dirent.h>
#endif /* defined(_WIN32) */

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/test_connection.h"

#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
DISABLE_GCC_WARNING("-Woverlength-strings")
/* We allow huge string constants in the unit tests, but not in the code
 * at large. */
#endif
#include "test_descriptors.inc"
#include "core/or/circuitlist.h"
#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
ENABLE_GCC_WARNING("-Woverlength-strings")
#endif

/* Return a statically allocated string representing yesterday's date
 * in ISO format. We use it so that state file items are not found to
 * be outdated. */
const char *
get_yesterday_date_str(void)
{
  static char buf[ISO_TIME_LEN+1];

  time_t yesterday = time(NULL) - 24*60*60;
  format_iso_time(buf, yesterday);
  return buf;
}

/* NOP replacement for router_descriptor_is_older_than() */
static int
router_descriptor_is_older_than_replacement(const routerinfo_t *router,
                                            int seconds)
{
  (void) router;
  (void) seconds;
  return 0;
}

/** Parse a file containing router descriptors and load them to our
    routerlist. This function is used to setup an artificial network
    so that we can conduct tests on it. */
void
helper_setup_fake_routerlist(void)
{
  int retval;
  routerlist_t *our_routerlist = NULL;
  const smartlist_t *our_nodelist = NULL;

  /* Read the file that contains our test descriptors. */

  /* We need to mock this function otherwise the descriptors will not
     accepted as they are too old. */
  MOCK(router_descriptor_is_older_than,
       router_descriptor_is_older_than_replacement);

  // Pick a time when these descriptors' certificates were valid.
  update_approx_time(1603981036);

  /* Load all the test descriptors to the routerlist. */
  retval = router_load_routers_from_string(TEST_DESCRIPTORS,
                                           NULL, SAVED_IN_JOURNAL,
                                           NULL, 0, NULL);
  tt_int_op(retval, OP_EQ, HELPER_NUMBER_OF_DESCRIPTORS);

  update_approx_time(0); // this restores the regular approx_time behavior

  /* Sanity checking of routerlist and nodelist. */
  our_routerlist = router_get_routerlist();
  tt_int_op(smartlist_len(our_routerlist->routers), OP_EQ,
              HELPER_NUMBER_OF_DESCRIPTORS);
  routerlist_assert_ok(our_routerlist);

  our_nodelist = nodelist_get_list();
  tt_int_op(smartlist_len(our_nodelist), OP_EQ, HELPER_NUMBER_OF_DESCRIPTORS);

  /* Mark all routers as non-guards but up and running! */
  SMARTLIST_FOREACH_BEGIN(our_nodelist, node_t *, node) {
    node->is_running = 1;
    node->is_valid = 1;
    node->is_possible_guard = 0;
  } SMARTLIST_FOREACH_END(node);

 done:
  UNMOCK(router_descriptor_is_older_than);
}

void
connection_write_to_buf_mock(const char *string, size_t len,
                             connection_t *conn, int compressed)
{
  (void) compressed;

  tor_assert(string);
  tor_assert(conn);

  buf_add(conn->outbuf, string, len);
}

char *
buf_get_contents(buf_t *buf, size_t *sz_out)
{
  tor_assert(buf);
  tor_assert(sz_out);

  char *out;
  *sz_out = buf_datalen(buf);
  if (*sz_out >= ULONG_MAX)
    return NULL; /* C'mon, really? */
  out = tor_malloc(*sz_out + 1);
  if (buf_get_bytes(buf, out, (unsigned long)*sz_out) != 0) {
    tor_free(out);
    return NULL;
  }
  out[*sz_out] = '\0'; /* Hopefully gratuitous. */
  return out;
}

/* Set up a fake origin circuit with the specified number of cells,
 * Return a pointer to the newly-created dummy circuit */
circuit_t *
dummy_origin_circuit_new(int n_cells)
{
  origin_circuit_t *circ = origin_circuit_new();
  int i;
  cell_t cell;

  for (i=0; i < n_cells; ++i) {
    crypto_rand((void*)&cell, sizeof(cell));
    cell_queue_append_packed_copy(TO_CIRCUIT(circ),
                                  &TO_CIRCUIT(circ)->n_chan_cells,
                                  1, &cell, 1, 0);
  }

  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_C_GENERAL;
  return TO_CIRCUIT(circ);
}

/** Mock-replacement. As tor_addr_lookup, but always fails on any
 * address containing a !.  This is necessary for running the unit tests
 * on networks where DNS hijackers think it's helpful to give answers
 * for things like 1.2.3.4.5 or "invalidstuff!!"
 */
int
mock_tor_addr_lookup__fail_on_bad_addrs(const char *name,
                                        uint16_t family, tor_addr_t *out)
{
  if (name && strchr(name, '!')) {
    return -1;
  }
  return tor_addr_lookup__real(name, family, out);
}

static char *
create_directory(const char *parent_dir, const char *name)
{
  char *dir = NULL;
  tor_asprintf(&dir, "%s"PATH_SEPARATOR"%s", parent_dir, name);
#ifdef _WIN32
  tt_int_op(mkdir(dir), OP_EQ, 0);
#else
  tt_int_op(mkdir(dir, 0700), OP_EQ, 0);
#endif
  return dir;

 done:
  tor_free(dir);
  return NULL;
}

static char *
create_file(const char *parent_dir, const char *name, const char *contents)
{
  char *path = NULL;
  tor_asprintf(&path, "%s"PATH_SEPARATOR"%s", parent_dir, name);
  contents = contents == NULL ? "" : contents;
  tt_int_op(write_str_to_file(path, contents, 0), OP_EQ, 0);
  return path;

 done:
  tor_free(path);
  return NULL;
}

int
create_test_directory_structure(const char *parent_dir)
{
  int ret = -1;
  char *dir1 = NULL;
  char *dir2 = NULL;
  char *file1 = NULL;
  char *file2 = NULL;
  char *dot = NULL;
  char *empty = NULL;
  char *forbidden = NULL;

  dir1 = create_directory(parent_dir, "dir1");
  tt_assert(dir1);
  dir2 = create_directory(parent_dir, "dir2");
  tt_assert(dir2);
  file1 = create_file(parent_dir, "file1", "Test 1");
  tt_assert(file1);
  file2 = create_file(parent_dir, "file2", "Test 2");
  tt_assert(file2);
  dot = create_file(parent_dir, ".test-hidden", "Test .");
  tt_assert(dot);
  empty = create_file(parent_dir, "empty", NULL);
  tt_assert(empty);
  forbidden = create_directory(parent_dir, "forbidden");
  tt_assert(forbidden);
#ifndef _WIN32
  tt_int_op(chmod(forbidden, 0), OP_EQ, 0);
#endif
  ret = 0;
 done:
  tor_free(dir1);
  tor_free(dir2);
  tor_free(file1);
  tor_free(file2);
  tor_free(dot);
  tor_free(empty);
  tor_free(forbidden);
  return ret;
}

/*********** Helper funcs for making new connections/streams *****************/

/* Helper for test_conn_get_connection() */
static int
fake_close_socket(tor_socket_t sock)
{
  (void)sock;
  return 0;
}

/* Helper for test_conn_get_proxy_or_connection() */
void
mock_connection_or_change_state(or_connection_t *conn, uint8_t state)
{
  tor_assert(conn);
  conn->base_.state = state;
}

static int mock_connection_connect_sockaddr_called = 0;
static int fake_socket_number = TEST_CONN_FD_INIT;

/* Helper for test_conn_get_connection() */
static int
mock_connection_connect_sockaddr(connection_t *conn,
                                 const struct sockaddr *sa,
                                 socklen_t sa_len,
                                 const struct sockaddr *bindaddr,
                                 socklen_t bindaddr_len,
                                 int *socket_error)
{
  (void)sa_len;
  (void)bindaddr;
  (void)bindaddr_len;

  tor_assert(conn);
  tor_assert(sa);
  tor_assert(socket_error);

  mock_connection_connect_sockaddr_called++;

  conn->s = fake_socket_number++;
  tt_assert(SOCKET_OK(conn->s));
  /* We really should call tor_libevent_initialize() here. Because we don't,
   * we are relying on other parts of the code not checking if the_event_base
   * (and therefore event->ev_base) is NULL.  */
  tt_int_op(connection_add_connecting(conn), OP_EQ, 0);

 done:
  /* Fake "connected" status */
  return 1;
}

or_connection_t *
test_conn_get_proxy_or_connection(unsigned int proxy_type)
{
  or_connection_t *conn = NULL;
  tor_addr_t dst_addr;
  tor_addr_t proxy_addr;
  int socket_err = 0;
  int in_progress = 0;

  MOCK(connection_connect_sockaddr,
       mock_connection_connect_sockaddr);
  MOCK(connection_write_to_buf_impl_,
       connection_write_to_buf_mock);
  MOCK(connection_or_change_state,
       mock_connection_or_change_state);
  MOCK(tor_close_socket, fake_close_socket);

  tor_init_connection_lists();

  conn = or_connection_new(CONN_TYPE_OR, TEST_CONN_FAMILY);
  tt_assert(conn);

  /* Set up a destination address. */
  test_conn_lookup_addr_helper(TEST_CONN_ADDRESS, TEST_CONN_FAMILY,
                               &dst_addr);
  tt_assert(!tor_addr_is_null(&dst_addr));

  conn->proxy_type = proxy_type;
  conn->base_.proxy_state = PROXY_INFANT;

  tor_addr_copy_tight(&conn->base_.addr, &dst_addr);
  conn->base_.address = tor_addr_to_str_dup(&dst_addr);
  conn->base_.port = TEST_CONN_PORT;

  /* Set up a proxy address. */
  test_conn_lookup_addr_helper(TEST_CONN_ADDRESS_2, TEST_CONN_FAMILY,
                               &proxy_addr);
  tt_assert(!tor_addr_is_null(&proxy_addr));

  conn->base_.state = OR_CONN_STATE_CONNECTING;

  mock_connection_connect_sockaddr_called = 0;
  in_progress = connection_connect(TO_CONN(conn), TEST_CONN_ADDRESS_PORT,
                                   &proxy_addr, TEST_CONN_PORT, &socket_err);
  tt_int_op(mock_connection_connect_sockaddr_called, OP_EQ, 1);
  tt_assert(!socket_err);
  tt_assert(in_progress == 0 || in_progress == 1);

  assert_connection_ok(TO_CONN(conn), time(NULL));

  in_progress = connection_or_finished_connecting(conn);
  tt_int_op(in_progress, OP_EQ, 0);

  assert_connection_ok(TO_CONN(conn), time(NULL));

  UNMOCK(connection_connect_sockaddr);
  UNMOCK(connection_write_to_buf_impl_);
  UNMOCK(connection_or_change_state);
  UNMOCK(tor_close_socket);
  return conn;

  /* On failure */
 done:
  UNMOCK(connection_connect_sockaddr);
  UNMOCK(connection_write_to_buf_impl_);
  UNMOCK(connection_or_change_state);
  UNMOCK(tor_close_socket);
  connection_free_(TO_CONN(conn));
  return NULL;
}

/** Create and return a new connection/stream */
connection_t *
test_conn_get_connection(uint8_t state, uint8_t type, uint8_t purpose)
{
  connection_t *conn = NULL;
  tor_addr_t addr;
  int socket_err = 0;
  int in_progress = 0;

  MOCK(connection_connect_sockaddr,
       mock_connection_connect_sockaddr);
  MOCK(tor_close_socket, fake_close_socket);

  tor_init_connection_lists();

  conn = connection_new(type, TEST_CONN_FAMILY);
  tt_assert(conn);

  test_conn_lookup_addr_helper(TEST_CONN_ADDRESS, TEST_CONN_FAMILY, &addr);
  tt_assert(!tor_addr_is_null(&addr));

  tor_addr_copy_tight(&conn->addr, &addr);
  conn->port = TEST_CONN_PORT;
  mock_connection_connect_sockaddr_called = 0;
  in_progress = connection_connect(conn, TEST_CONN_ADDRESS_PORT, &addr,
                                   TEST_CONN_PORT, &socket_err);
  tt_int_op(mock_connection_connect_sockaddr_called, OP_EQ, 1);
  tt_assert(!socket_err);
  tt_assert(in_progress == 0 || in_progress == 1);

  /* fake some of the attributes so the connection looks OK */
  conn->state = state;
  conn->purpose = purpose;
  assert_connection_ok(conn, time(NULL));

  UNMOCK(connection_connect_sockaddr);
  UNMOCK(tor_close_socket);
  return conn;

  /* On failure */
 done:
  UNMOCK(connection_connect_sockaddr);
  UNMOCK(tor_close_socket);
  return NULL;
}

/* Helper function to parse a set of torrc options in a text format and return
 * a newly allocated or_options_t object containing the configuration. On
 * error, NULL is returned indicating that the conf couldn't be parsed
 * properly. */
or_options_t *
helper_parse_options(const char *conf)
{
  int ret = 0;
  char *msg = NULL;
  or_options_t *opt = NULL;
  config_line_t *line = NULL;

  /* Kind of pointless to call this with a NULL value. */
  tt_assert(conf);

  opt = options_new();
  tt_assert(opt);
  ret = config_get_lines(conf, &line, 1);
  if (ret != 0) {
    goto done;
  }
  ret = config_assign(get_options_mgr(), opt, line, 0, &msg);
  if (ret != 0) {
    goto done;
  }

 done:
  config_free_lines(line);
  if (ret != 0) {
    or_options_free(opt);
    opt = NULL;
  }
  return opt;
}

/**
 * Dispatch alertfn callback: flush all messages right now. Implements
 * DELIV_IMMEDIATE.
 **/
static void
alertfn_immediate(dispatch_t *d, channel_id_t chan, void *arg)
{
  (void) arg;
  dispatch_flush(d, chan, INT_MAX);
}

/**
 * Setup helper for tests that need pubsub active
 *
 * Does not hook up mainloop events.  Does set immediate delivery for
 * all channels.
 */
void *
helper_setup_pubsub(const struct testcase_t *testcase)
{
  dispatch_t *dispatcher = NULL;
  pubsub_builder_t *builder = pubsub_builder_new();
  channel_id_t chan = get_channel_id("orconn");

  (void)testcase;
  (void)subsystems_add_pubsub(builder);
  dispatcher = pubsub_builder_finalize(builder, NULL);
  tor_assert(dispatcher);
  dispatch_set_alert_fn(dispatcher, chan, alertfn_immediate, NULL);
  chan = get_channel_id("ocirc");
  dispatch_set_alert_fn(dispatcher, chan, alertfn_immediate, NULL);
  return dispatcher;
}

/**
 * Cleanup helper for tests that need pubsub active
 */
int
helper_cleanup_pubsub(const struct testcase_t *testcase, void *dispatcher_)
{
  dispatch_t *dispatcher = dispatcher_;

  (void)testcase;
  dispatch_free(dispatcher);
  return 1;
}

const struct testcase_setup_t helper_pubsub_setup = {
  helper_setup_pubsub, helper_cleanup_pubsub
};

origin_circuit_t *
new_test_origin_circuit(bool has_opened,
                        struct timeval circ_start_time,
                        int path_len,
                        extend_info_t **ei_list)
{
  origin_circuit_t *origin_circ = origin_circuit_new();

  TO_CIRCUIT(origin_circ)->purpose = CIRCUIT_PURPOSE_C_GENERAL;

  origin_circ->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  origin_circ->build_state->desired_path_len = path_len;

  if (ei_list) {
    for (int i = 0; i < path_len; i++) {
      extend_info_t *ei = ei_list[i];
      cpath_append_hop(&origin_circ->cpath, ei);
    }
  }

  if (has_opened) {
    origin_circ->has_opened = 1;
    TO_CIRCUIT(origin_circ)->state = CIRCUIT_STATE_OPEN;
    origin_circ->cpath->state = CPATH_STATE_OPEN;
  } else {
    TO_CIRCUIT(origin_circ)->timestamp_began = circ_start_time;
    TO_CIRCUIT(origin_circ)->timestamp_created = circ_start_time;
    origin_circ->cpath->state = CPATH_STATE_CLOSED;
  }

  return origin_circ;
}
