/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include <math.h>
#include <time.h>

#define CONNECTION_PRIVATE
#define DIRCLIENT_PRIVATE
#define DIRVOTE_PRIVATE
#define ENTRYNODES_PRIVATE
#define HIBERNATE_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define ROUTERLIST_PRIVATE
#define NODE_SELECT_PRIVATE
#define TOR_UNIT_TESTING
#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "feature/control/control.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "feature/dircommon/directory.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirauth/dirvote.h"
#include "feature/client/entrynodes.h"
#include "feature/hibernate/hibernate.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "core/or/policies.h"
#include "feature/relay/router.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/node_select.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/dirparse/authcert_parse.h"
#include "feature/dirparse/ns_parse.h"
#include "feature/dirauth/shared_random.h"
#include "app/config/statefile.h"

#include "feature/nodelist/authority_cert_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "app/config/or_state_st.h"
#include "feature/nodelist/routerstatus_st.h"

#include "lib/encoding/confline.h"
#include "lib/buf/buffers.h"

#include "test/test.h"
#include "test/test_dir_common.h"
#include "test/log_test_helpers.h"

static authority_cert_t *mock_cert;

static authority_cert_t *
get_my_v3_authority_cert_m(void)
{
  tor_assert(mock_cert);
  return mock_cert;
}

/* 4 digests + 3 sep + pre + post + NULL */
static char output[4*BASE64_DIGEST256_LEN+3+2+2+1];

static void
mock_get_from_dirserver(uint8_t dir_purpose, uint8_t router_purpose,
                        const char *resource, int pds_flags,
                        download_want_authority_t want_authority)
{
  (void)dir_purpose;
  (void)router_purpose;
  (void)pds_flags;
  (void)want_authority;
  tt_assert(resource);
  strlcpy(output, resource, sizeof(output));
 done:
  ;
}

static void
test_routerlist_initiate_descriptor_downloads(void *arg)
{
  const char *prose = "unhurried and wise, we perceive.";
  smartlist_t *digests = smartlist_new();
  (void)arg;

  for (int i = 0; i < 20; i++) {
    smartlist_add(digests, (char*)prose);
  }

  MOCK(directory_get_from_dirserver, mock_get_from_dirserver);
  initiate_descriptor_downloads(NULL, DIR_PURPOSE_FETCH_MICRODESC,
                                digests, 3, 7, 0);
  UNMOCK(directory_get_from_dirserver);

  tt_str_op(output, OP_EQ, "d/"
            "dW5odXJyaWVkIGFuZCB3aXNlLCB3ZSBwZXJjZWl2ZS4-"
            "dW5odXJyaWVkIGFuZCB3aXNlLCB3ZSBwZXJjZWl2ZS4-"
            "dW5odXJyaWVkIGFuZCB3aXNlLCB3ZSBwZXJjZWl2ZS4-"
            "dW5odXJyaWVkIGFuZCB3aXNlLCB3ZSBwZXJjZWl2ZS4"
            ".z");

 done:
  smartlist_free(digests);
}

static int count = 0;

static void
mock_initiate_descriptor_downloads(const routerstatus_t *source,
                                   int purpose, smartlist_t *digests,
                                   int lo, int hi, int pds_flags)
{
  (void)source;
  (void)purpose;
  (void)digests;
  (void)pds_flags;
  (void)hi;
  (void)lo;
  count += 1;
}

static void
test_routerlist_launch_descriptor_downloads(void *arg)
{
  smartlist_t *downloadable = smartlist_new();
  time_t now = time(NULL);
  char *cp;
  (void)arg;

  for (int i = 0; i < 100; i++) {
    cp = tor_malloc(DIGEST256_LEN);
    tt_assert(cp);
    crypto_rand(cp, DIGEST256_LEN);
    smartlist_add(downloadable, cp);
  }

  MOCK(initiate_descriptor_downloads, mock_initiate_descriptor_downloads);
  launch_descriptor_downloads(DIR_PURPOSE_FETCH_MICRODESC, downloadable,
                              NULL, now);
  tt_int_op(3, OP_EQ, count);
  UNMOCK(initiate_descriptor_downloads);

 done:
  SMARTLIST_FOREACH(downloadable, char *, cp1, tor_free(cp1));
  smartlist_free(downloadable);
}

static void
construct_consensus(char **consensus_text_md, time_t now)
{
  networkstatus_t *vote = NULL;
  networkstatus_t *v1 = NULL, *v2 = NULL, *v3 = NULL;
  networkstatus_voter_info_t *voter = NULL;
  authority_cert_t *cert1=NULL, *cert2=NULL, *cert3=NULL;
  crypto_pk_t *sign_skey_1=NULL, *sign_skey_2=NULL, *sign_skey_3=NULL;
  crypto_pk_t *sign_skey_leg=NULL;
  smartlist_t *votes = NULL;
  int n_vrs;

  tt_assert(!dir_common_authority_pk_init(&cert1, &cert2, &cert3,
                                          &sign_skey_1, &sign_skey_2,
                                          &sign_skey_3));
  sign_skey_leg = pk_generate(4);

  dir_common_construct_vote_1(&vote, cert1, sign_skey_1,
                              &dir_common_gen_routerstatus_for_v3ns,
                              &v1, &n_vrs, now, 1);
  networkstatus_vote_free(vote);
  tt_assert(v1);
  tt_int_op(n_vrs, OP_EQ, 4);
  tt_int_op(smartlist_len(v1->routerstatus_list), OP_EQ, 4);

  dir_common_construct_vote_2(&vote, cert2, sign_skey_2,
                              &dir_common_gen_routerstatus_for_v3ns,
                              &v2, &n_vrs, now, 1);
  networkstatus_vote_free(vote);
  tt_assert(v2);
  tt_int_op(n_vrs, OP_EQ, 4);
  tt_int_op(smartlist_len(v2->routerstatus_list), OP_EQ, 4);

  dir_common_construct_vote_3(&vote, cert3, sign_skey_3,
                              &dir_common_gen_routerstatus_for_v3ns,
                              &v3, &n_vrs, now, 1);

  tt_assert(v3);
  tt_int_op(n_vrs, OP_EQ, 4);
  tt_int_op(smartlist_len(v3->routerstatus_list), OP_EQ, 4);
  networkstatus_vote_free(vote);
  votes = smartlist_new();
  smartlist_add(votes, v1);
  smartlist_add(votes, v2);
  smartlist_add(votes, v3);

  *consensus_text_md = networkstatus_compute_consensus(votes, 3,
                                                   cert1->identity_key,
                                                   sign_skey_1,
                                                   "AAAAAAAAAAAAAAAAAAAA",
                                                   sign_skey_leg,
                                                   FLAV_MICRODESC);

  tt_assert(*consensus_text_md);

 done:
  tor_free(voter);
  networkstatus_vote_free(v1);
  networkstatus_vote_free(v2);
  networkstatus_vote_free(v3);
  smartlist_free(votes);
  authority_cert_free(cert1);
  authority_cert_free(cert2);
  authority_cert_free(cert3);
  crypto_pk_free(sign_skey_1);
  crypto_pk_free(sign_skey_2);
  crypto_pk_free(sign_skey_3);
  crypto_pk_free(sign_skey_leg);
}

static int mock_usable_consensus_flavor_value = FLAV_NS;

static int
mock_usable_consensus_flavor(void)
{
  return mock_usable_consensus_flavor_value;
}

static void
test_router_pick_directory_server_impl(void *arg)
{
  (void)arg;

  networkstatus_t *con_md = NULL;
  char *consensus_text_md = NULL;
  int flags = PDS_IGNORE_FASCISTFIREWALL|PDS_RETRY_IF_NO_SERVERS;
  or_options_t *options = get_options_mutable();
  const routerstatus_t *rs = NULL;
  options->UseMicrodescriptors = 1;
  char *router1_id = NULL, *router2_id = NULL, *router3_id = NULL;
  node_t *node_router1 = NULL, *node_router2 = NULL, *node_router3 = NULL;
  config_line_t *policy_line = NULL;
  time_t now = time(NULL);
  int tmp_dirport1, tmp_dirport3;

  (void)arg;

  MOCK(usable_consensus_flavor, mock_usable_consensus_flavor);

  /* With no consensus, we must be bootstrapping, regardless of time or flavor
   */
  mock_usable_consensus_flavor_value = FLAV_NS;
  tt_assert(networkstatus_consensus_is_bootstrapping(now));
  tt_assert(networkstatus_consensus_is_bootstrapping(now + 2000));
  tt_assert(networkstatus_consensus_is_bootstrapping(now + 2*24*60*60));
  tt_assert(networkstatus_consensus_is_bootstrapping(now - 2*24*60*60));

  mock_usable_consensus_flavor_value = FLAV_MICRODESC;
  tt_assert(networkstatus_consensus_is_bootstrapping(now));
  tt_assert(networkstatus_consensus_is_bootstrapping(now + 2000));
  tt_assert(networkstatus_consensus_is_bootstrapping(now + 2*24*60*60));
  tt_assert(networkstatus_consensus_is_bootstrapping(now - 2*24*60*60));

  /* Init SR subsystem. */
  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  mock_cert = authority_cert_parse_from_string(AUTHORITY_CERT_1,
                                               strlen(AUTHORITY_CERT_1),
                                               NULL);
  sr_init(0);
  UNMOCK(get_my_v3_authority_cert);

  /* No consensus available, fail early */
  rs = router_pick_directory_server_impl(V3_DIRINFO, (const int) 0, NULL);
  tt_ptr_op(rs, OP_EQ, NULL);

  construct_consensus(&consensus_text_md, now);
  tt_assert(consensus_text_md);
  con_md = networkstatus_parse_vote_from_string(consensus_text_md,
                                                strlen(consensus_text_md),
                                                NULL,
                                                NS_TYPE_CONSENSUS);
  tt_assert(con_md);
  tt_int_op(con_md->flavor,OP_EQ, FLAV_MICRODESC);
  tt_assert(con_md->routerstatus_list);
  tt_int_op(smartlist_len(con_md->routerstatus_list), OP_EQ, 3);
  tt_assert(!networkstatus_set_current_consensus_from_ns(con_md,
                                                 "microdesc"));

  /* If the consensus time or flavor doesn't match, we are still
   * bootstrapping */
  mock_usable_consensus_flavor_value = FLAV_NS;
  tt_assert(networkstatus_consensus_is_bootstrapping(now));
  tt_assert(networkstatus_consensus_is_bootstrapping(now + 2000));
  tt_assert(networkstatus_consensus_is_bootstrapping(now + 2*24*60*60));
  tt_assert(networkstatus_consensus_is_bootstrapping(now - 2*24*60*60));

  /* With a valid consensus for the current time and flavor, we stop
   * bootstrapping, even if we have no certificates */
  mock_usable_consensus_flavor_value = FLAV_MICRODESC;
  tt_assert(!networkstatus_consensus_is_bootstrapping(now + 2000));
  tt_assert(!networkstatus_consensus_is_bootstrapping(con_md->valid_after));
  tt_assert(!networkstatus_consensus_is_bootstrapping(con_md->valid_until));
  tt_assert(!networkstatus_consensus_is_bootstrapping(con_md->valid_until
                                                      + 24*60*60));
  /* These times are outside the test validity period */
  tt_assert(networkstatus_consensus_is_bootstrapping(now + 2*24*60*60));
  tt_assert(networkstatus_consensus_is_bootstrapping(now - 2*24*60*60));

  nodelist_set_consensus(con_md);
  nodelist_assert_ok();

  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  /* We should not fail now we have a consensus and routerstatus_list
   * and nodelist are populated. */
  tt_ptr_op(rs, OP_NE, NULL);

  /* Manipulate the nodes so we get the dir server we expect */
  router1_id = tor_malloc(DIGEST_LEN);
  memset(router1_id, TEST_DIR_ROUTER_ID_1, DIGEST_LEN);
  router2_id = tor_malloc(DIGEST_LEN);
  memset(router2_id, TEST_DIR_ROUTER_ID_2, DIGEST_LEN);
  router3_id = tor_malloc(DIGEST_LEN);
  memset(router3_id, TEST_DIR_ROUTER_ID_3, DIGEST_LEN);

  node_router1 = node_get_mutable_by_id(router1_id);
  node_router2 = node_get_mutable_by_id(router2_id);
  node_router3 = node_get_mutable_by_id(router3_id);

  node_router1->is_possible_guard = 1;

  node_router1->is_running = 0;
  node_router3->is_running = 0;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router2_id, DIGEST_LEN));
  rs = NULL;
  node_router1->is_running = 1;
  node_router3->is_running = 1;

  node_router1->rs->is_v2_dir = 0;
  node_router3->rs->is_v2_dir = 0;
  tmp_dirport1 = node_router1->rs->ipv4_dirport;
  tmp_dirport3 = node_router3->rs->ipv4_dirport;
  node_router1->rs->ipv4_dirport = 0;
  node_router3->rs->ipv4_dirport = 0;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router2_id, DIGEST_LEN));
  rs = NULL;
  node_router1->rs->is_v2_dir = 1;
  node_router3->rs->is_v2_dir = 1;
  node_router1->rs->ipv4_dirport = tmp_dirport1;
  node_router3->rs->ipv4_dirport = tmp_dirport3;

  node_router1->is_valid = 0;
  node_router3->is_valid = 0;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router2_id, DIGEST_LEN));
  rs = NULL;
  node_router1->is_valid = 1;
  node_router3->is_valid = 1;

  /* Manipulate overloaded */

  node_router2->rs->last_dir_503_at = now;
  node_router3->rs->last_dir_503_at = now;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router1_id, DIGEST_LEN));
  node_router2->rs->last_dir_503_at = 0;
  node_router3->rs->last_dir_503_at = 0;

  /* Set a Fascist firewall */
  flags &= ~ PDS_IGNORE_FASCISTFIREWALL;
  policy_line = tor_malloc_zero(sizeof(config_line_t));
  policy_line->key = tor_strdup("ReachableORAddresses");
  policy_line->value = tor_strdup("accept *:442, reject *:*");
  options->ReachableORAddresses = policy_line;
  policies_parse_from_options(options);

  node_router1->rs->ipv4_orport = 444;
  node_router2->rs->ipv4_orport = 443;
  node_router3->rs->ipv4_orport = 442;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router3_id, DIGEST_LEN));
  node_router1->rs->ipv4_orport = 442;
  node_router2->rs->ipv4_orport = 443;
  node_router3->rs->ipv4_orport = 444;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router1_id, DIGEST_LEN));

  /* Fascist firewall and overloaded */
  node_router1->rs->ipv4_orport = 442;
  node_router2->rs->ipv4_orport = 443;
  node_router3->rs->ipv4_orport = 442;
  node_router3->rs->last_dir_503_at = now;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router1_id, DIGEST_LEN));
  node_router3->rs->last_dir_503_at = 0;

  /* Fascists against OR and Dir */
  policy_line = tor_malloc_zero(sizeof(config_line_t));
  policy_line->key = tor_strdup("ReachableAddresses");
  policy_line->value = tor_strdup("accept *:80, reject *:*");
  options->ReachableDirAddresses = policy_line;
  policies_parse_from_options(options);
  node_router1->rs->ipv4_orport = 442;
  node_router2->rs->ipv4_orport = 441;
  node_router3->rs->ipv4_orport = 443;
  node_router1->rs->ipv4_dirport = 80;
  node_router2->rs->ipv4_dirport = 80;
  node_router3->rs->ipv4_dirport = 81;
  node_router1->rs->last_dir_503_at = now;
  rs = router_pick_directory_server_impl(V3_DIRINFO, flags, NULL);
  tt_ptr_op(rs, OP_NE, NULL);
  tt_assert(tor_memeq(rs->identity_digest, router1_id, DIGEST_LEN));
  node_router1->rs->last_dir_503_at = 0;

 done:
  UNMOCK(usable_consensus_flavor);

  if (router1_id)
    tor_free(router1_id);
  if (router2_id)
    tor_free(router2_id);
  if (router3_id)
    tor_free(router3_id);
  if (options->ReachableORAddresses ||
      options->ReachableDirAddresses)
    policies_free_all();
  tor_free(consensus_text_md);
  networkstatus_vote_free(con_md);
}

static or_state_t *dummy_state = NULL;
static or_state_t *
get_or_state_replacement(void)
{
  return dummy_state;
}

static void
mock_directory_initiate_request(directory_request_t *req)
{
  (void)req;
  return;
}

static circuit_guard_state_t *
mock_circuit_guard_state_new(entry_guard_t *guard, unsigned state,
                        entry_guard_restriction_t *rst)
{
  (void) guard;
  (void) state;
  (void) rst;
  return NULL;
}

/** Test that we will use our directory guards to fetch mds even if we don't
 *  have any dirinfo (tests bug #23862). */
static void
test_directory_guard_fetch_with_no_dirinfo(void *arg)
{
  int retval;
  char *consensus_text_md = NULL;
  or_options_t *options = get_options_mutable();
  time_t now = time(NULL);

  (void) arg;

  hibernate_set_state_for_testing_(HIBERNATE_STATE_LIVE);

  /* Initialize the SRV subsystem */
  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  mock_cert = authority_cert_parse_from_string(AUTHORITY_CERT_1,
                                               strlen(AUTHORITY_CERT_1),
                                               NULL);
  sr_init(0);
  UNMOCK(get_my_v3_authority_cert);

  /* Initialize the entry node configuration from the ticket */
  options->UseEntryGuards = 1;
  options->StrictNodes = 1;
  get_options_mutable()->EntryNodes = routerset_new();
  routerset_parse(get_options_mutable()->EntryNodes,
                  "2121212121212121212121212121212121212121", "foo");

  /* Mock some functions */
  dummy_state = tor_malloc_zero(sizeof(or_state_t));
  MOCK(get_or_state, get_or_state_replacement);
  MOCK(directory_initiate_request, mock_directory_initiate_request);
  /* we need to mock this one to avoid memleaks */
  MOCK(circuit_guard_state_new, mock_circuit_guard_state_new);

  /* Call guards_update_all() to simulate loading our state file (see
   * entry_guards_load_guards_from_state() and ticket #23989). */
  guards_update_all();

  /* Test logic: Simulate the arrival of a new consensus when we have no
   * dirinfo at all. Tor will need to fetch the mds from the consensus. Make
   * sure that Tor will use the specified entry guard instead of relying on the
   * fallback directories. */

  /* Fixup the dirconn that will deliver the consensus */
  dir_connection_t *conn = dir_connection_new(AF_INET);
  tor_addr_from_ipv4h(&conn->base_.addr, 0x7f000001);
  conn->base_.port = 8800;
  TO_CONN(conn)->address = tor_strdup("127.0.0.1");
  conn->base_.purpose = DIR_PURPOSE_FETCH_CONSENSUS;
  conn->requested_resource = tor_strdup("ns");

  /* Construct a consensus */
  construct_consensus(&consensus_text_md, now);
  tt_assert(consensus_text_md);

  /* Place the consensus in the dirconn */
  response_handler_args_t args;
  memset(&args, 0, sizeof(response_handler_args_t));
  args.status_code = 200;
  args.body = consensus_text_md;
  args.body_len = strlen(consensus_text_md);

  /* Update approx time so that the consensus is considered live */
  update_approx_time(now+1010);

  setup_capture_of_logs(LOG_DEBUG);

  /* Now handle the consensus */
  retval = handle_response_fetch_consensus(conn, &args);
  tt_int_op(retval, OP_EQ, 0);

  /* Make sure that our primary guard was chosen */
  expect_log_msg_containing("Selected primary guard router3");

 done:
  tor_free(consensus_text_md);
  tor_free(dummy_state);
  connection_free_minimal(TO_CONN(conn));
  entry_guards_free_all();
  teardown_capture_of_logs();
}

static connection_t *mocked_connection = NULL;

/* Mock connection_get_by_type_addr_port_purpose by returning
 * mocked_connection. */
static connection_t *
mock_connection_get_by_type_addr_port_purpose(int type,
                                              const tor_addr_t *addr,
                                              uint16_t port, int purpose)
{
  (void)type;
  (void)addr;
  (void)port;
  (void)purpose;

  return mocked_connection;
}

#define TEST_ADDR_STR "127.0.0.1"
#define TEST_DIR_PORT 12345

static void
test_routerlist_router_is_already_dir_fetching(void *arg)
{
  (void)arg;
  tor_addr_port_t test_ap, null_addr_ap, zero_port_ap;

  /* Setup */
  tor_addr_parse(&test_ap.addr, TEST_ADDR_STR);
  test_ap.port = TEST_DIR_PORT;
  tor_addr_make_null(&null_addr_ap.addr, AF_INET6);
  null_addr_ap.port = TEST_DIR_PORT;
  tor_addr_parse(&zero_port_ap.addr, TEST_ADDR_STR);
  zero_port_ap.port = 0;
  MOCK(connection_get_by_type_addr_port_purpose,
       mock_connection_get_by_type_addr_port_purpose);

  /* Test that we never get 1 from a NULL connection */
  mocked_connection = NULL;
  tt_int_op(router_is_already_dir_fetching(&test_ap, 1, 1), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(&test_ap, 1, 0), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(&test_ap, 0, 1), OP_EQ, 0);
  /* We always expect 0 in these cases */
  tt_int_op(router_is_already_dir_fetching(&test_ap, 0, 0), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(NULL, 1, 1), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(&null_addr_ap, 1, 1), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(&zero_port_ap, 1, 1), OP_EQ, 0);

  /* Test that we get 1 with a connection in the appropriate circumstances */
  mocked_connection = connection_new(CONN_TYPE_DIR, AF_INET);
  tt_int_op(router_is_already_dir_fetching(&test_ap, 1, 1), OP_EQ, 1);
  tt_int_op(router_is_already_dir_fetching(&test_ap, 1, 0), OP_EQ, 1);
  tt_int_op(router_is_already_dir_fetching(&test_ap, 0, 1), OP_EQ, 1);

  /* Test that we get 0 even with a connection in the appropriate
   * circumstances */
  tt_int_op(router_is_already_dir_fetching(&test_ap, 0, 0), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(NULL, 1, 1), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(&null_addr_ap, 1, 1), OP_EQ, 0);
  tt_int_op(router_is_already_dir_fetching(&zero_port_ap, 1, 1), OP_EQ, 0);

 done:
  /* If a connection is never set up, connection_free chokes on it. */
  if (mocked_connection) {
    buf_free(mocked_connection->inbuf);
    buf_free(mocked_connection->outbuf);
  }
  tor_free(mocked_connection);
  UNMOCK(connection_get_by_type_addr_port_purpose);
}

#undef TEST_ADDR_STR
#undef TEST_DIR_PORT

static long mock_apparent_skew = 0;

/** Store apparent_skew and assert that the other arguments are as
 * expected. */
static void
mock_clock_skew_warning(const connection_t *conn, long apparent_skew,
                        int trusted, log_domain_mask_t domain,
                        const char *received, const char *source)
{
  (void)conn;
  mock_apparent_skew = apparent_skew;
  tt_int_op(trusted, OP_EQ, 1);
  tt_i64_op(domain, OP_EQ, LD_GENERAL);
  tt_str_op(received, OP_EQ, "microdesc flavor consensus");
  tt_str_op(source, OP_EQ, "CONSENSUS");
 done:
  ;
}

/** Do common setup for test_timely_consensus() and
 * test_early_consensus().  Call networkstatus_set_current_consensus()
 * on a constructed consensus and with an appropriately-modified
 * approx_time.  Callers expect presence or absence of appropriate log
 * messages and control events. */
static int
test_skew_common(void *arg, time_t now, unsigned long *offset)
{
  char *consensus = NULL;
  int retval = 0;

  *offset = strtoul(arg, NULL, 10);

  /* Initialize the SRV subsystem */
  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  mock_cert = authority_cert_parse_from_string(AUTHORITY_CERT_1,
                                               strlen(AUTHORITY_CERT_1),
                                               NULL);
  sr_init(0);
  UNMOCK(get_my_v3_authority_cert);

  construct_consensus(&consensus, now);
  tt_assert(consensus);

  update_approx_time(now + *offset);

  mock_apparent_skew = 0;
  /* Caller will call UNMOCK() */
  MOCK(clock_skew_warning, mock_clock_skew_warning);
  /* Caller will call teardown_capture_of_logs() */
  setup_capture_of_logs(LOG_WARN);
  retval = networkstatus_set_current_consensus(consensus, strlen(consensus),
                                               "microdesc", 0,
                                               NULL);

 done:
  tor_free(consensus);
  return retval;
}

/** Test non-early consensus */
static void
test_timely_consensus(void *arg)
{
  time_t now = time(NULL);
  unsigned long offset = 0;
  int retval = 0;

  retval = test_skew_common(arg, now, &offset);
  (void)offset;
  expect_no_log_msg_containing("behind the time published in the consensus");
  tt_int_op(retval, OP_EQ, 0);
  tt_int_op(mock_apparent_skew, OP_EQ, 0);
 done:
  teardown_capture_of_logs();
  UNMOCK(clock_skew_warning);
}

/** Test early consensus  */
static void
test_early_consensus(void *arg)
{
  time_t now = time(NULL);
  unsigned long offset = 0;
  int retval = 0;

  retval = test_skew_common(arg, now, &offset);
  /* Can't use expect_single_log_msg() because of unrecognized authorities */
  expect_log_msg_containing("behind the time published in the consensus");
  tt_int_op(retval, OP_EQ, 0);
  /* This depends on construct_consensus() setting valid_after=now+1000 */
  tt_int_op(mock_apparent_skew, OP_EQ, offset - 1000);
 done:
  teardown_capture_of_logs();
  UNMOCK(clock_skew_warning);
}

/** Test warn_early_consensus(), expecting no warning  */
static void
test_warn_early_consensus_no(const networkstatus_t *c, time_t now,
                             long offset)
{
  mock_apparent_skew = 0;
  setup_capture_of_logs(LOG_WARN);
  warn_early_consensus(c, "microdesc", now + offset);
  expect_no_log_msg_containing("behind the time published in the consensus");
  tt_int_op(mock_apparent_skew, OP_EQ, 0);
 done:
  teardown_capture_of_logs();
}

/** Test warn_early_consensus(), expecting a warning */
static void
test_warn_early_consensus_yes(const networkstatus_t *c, time_t now,
                              long offset)
{
  mock_apparent_skew = 0;
  setup_capture_of_logs(LOG_WARN);
  warn_early_consensus(c, "microdesc", now + offset);
  /* Can't use expect_single_log_msg() because of unrecognized authorities */
  expect_log_msg_containing("behind the time published in the consensus");
  tt_int_op(mock_apparent_skew, OP_EQ, offset);
 done:
  teardown_capture_of_logs();
}

/**
 * Test warn_early_consensus() directly, checking both the non-warning
 * case (consensus is not early) and the warning case (consensus is
 * early).  Depends on EARLY_CONSENSUS_NOTICE_SKEW=60.
 */
static void
test_warn_early_consensus(void *arg)
{
  networkstatus_t *c = NULL;
  time_t now = time(NULL);

  (void)arg;
  c = tor_malloc_zero(sizeof *c);
  c->valid_after = now;
  c->dist_seconds = 300;
  mock_apparent_skew = 0;
  MOCK(clock_skew_warning, mock_clock_skew_warning);
  test_warn_early_consensus_no(c, now, 60);
  test_warn_early_consensus_no(c, now, 0);
  test_warn_early_consensus_no(c, now, -60);
  test_warn_early_consensus_no(c, now, -360);
  test_warn_early_consensus_yes(c, now, -361);
  test_warn_early_consensus_yes(c, now, -600);
  UNMOCK(clock_skew_warning);
  tor_free(c);
}

#define NODE(name, flags) \
  { #name, test_routerlist_##name, (flags), NULL, NULL }
#define ROUTER(name,flags) \
  { #name, test_router_##name, (flags), NULL, NULL }

#define TIMELY(name, arg)                                     \
  { name, test_timely_consensus, TT_FORK, &passthrough_setup, \
    (char *)(arg) }
#define EARLY(name, arg) \
  { name, test_early_consensus, TT_FORK, &passthrough_setup, \
    (char *)(arg) }

struct testcase_t routerlist_tests[] = {
  NODE(initiate_descriptor_downloads, 0),
  NODE(launch_descriptor_downloads, 0),
  NODE(router_is_already_dir_fetching, TT_FORK),
  ROUTER(pick_directory_server_impl, TT_FORK),
  { "directory_guard_fetch_with_no_dirinfo",
    test_directory_guard_fetch_with_no_dirinfo, TT_FORK, NULL, NULL },
  /* These depend on construct_consensus() setting
   * valid_after=now+1000 and dist_seconds=250 */
  TIMELY("timely_consensus1", "1010"),
  TIMELY("timely_consensus2", "1000"),
  TIMELY("timely_consensus3", "690"),
  EARLY("early_consensus1", "689"),
  { "warn_early_consensus", test_warn_early_consensus, 0, NULL, NULL },
  END_OF_TESTCASES
};
