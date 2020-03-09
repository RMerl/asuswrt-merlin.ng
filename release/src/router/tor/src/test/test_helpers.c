/* Copyright (c) 2014-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_helpers.c
 * \brief Some helper functions to avoid code duplication in unit tests.
 */

#define ROUTERLIST_PRIVATE
#define CONFIG_PRIVATE
#define CONNECTION_PRIVATE
#define MAINLOOP_PRIVATE

#include "orconfig.h"
#include "core/or/or.h"

#include "lib/buf/buffers.h"
#include "app/config/config.h"
#include "lib/confmgt/confparse.h"
#include "app/main/subsysmgr.h"
#include "core/mainloop/connection.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/mainloop/mainloop.h"
#include "feature/nodelist/nodelist.h"
#include "core/or/relay.h"
#include "feature/nodelist/routerlist.h"
#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_naming.h"
#include "lib/pubsub/pubsub_build.h"
#include "lib/pubsub/pubsub_connect.h"
#include "lib/encoding/confline.h"
#include "lib/net/resolve.h"

#include "core/or/cell_st.h"
#include "core/or/connection_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/origin_circuit_st.h"
#include "feature/nodelist/routerlist_st.h"

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/test_connection.h"

#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
DISABLE_GCC_WARNING(overlength-strings)
/* We allow huge string constants in the unit tests, but not in the code
 * at large. */
#endif
#include "test_descriptors.inc"
#include "core/or/circuitlist.h"
#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
ENABLE_GCC_WARNING(overlength-strings)
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

  /* Load all the test descriptors to the routerlist. */
  retval = router_load_routers_from_string(TEST_DESCRIPTORS,
                                           NULL, SAVED_IN_JOURNAL,
                                           NULL, 0, NULL);
  tt_int_op(retval, OP_EQ, HELPER_NUMBER_OF_DESCRIPTORS);

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

/*********** Helper funcs for making new connections/streams *****************/

/* Helper for test_conn_get_connection() */
static int
fake_close_socket(tor_socket_t sock)
{
  (void)sock;
  return 0;
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
