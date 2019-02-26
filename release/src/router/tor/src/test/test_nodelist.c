/* Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_nodelist.c
 * \brief Unit tests for nodelist related functions.
 **/

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/torcert.h"

#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

/** Test the case when node_get_by_id() returns NULL,
 * node_get_verbose_nickname_by_id should return the base 16 encoding
 * of the id.
 */
static void
test_nodelist_node_get_verbose_nickname_by_id_null_node(void *arg)
{
  char vname[MAX_VERBOSE_NICKNAME_LEN+1];
  const char ID[] = "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA"
                    "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA";
  (void) arg;

  /* make sure node_get_by_id returns NULL */
  tt_assert(!node_get_by_id(ID));
  node_get_verbose_nickname_by_id(ID, vname);
  tt_str_op(vname,OP_EQ, "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
 done:
  return;
}

/** For routers without named flag, get_verbose_nickname should return
 * "Fingerprint~Nickname"
 */
static void
test_nodelist_node_get_verbose_nickname_not_named(void *arg)
{
  node_t mock_node;
  routerstatus_t mock_rs;

  char vname[MAX_VERBOSE_NICKNAME_LEN+1];

  (void) arg;

  memset(&mock_node, 0, sizeof(node_t));
  memset(&mock_rs, 0, sizeof(routerstatus_t));

  /* verbose nickname should use ~ instead of = for unnamed routers */
  strlcpy(mock_rs.nickname, "TestOR", sizeof(mock_rs.nickname));
  mock_node.rs = &mock_rs;
  memcpy(mock_node.identity,
          "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA"
          "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA",
          DIGEST_LEN);
  node_get_verbose_nickname(&mock_node, vname);
  tt_str_op(vname,OP_EQ, "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA~TestOR");

 done:
  return;
}

/** A node should be considered a directory server if it has an open dirport
 * of it accepts tunnelled directory requests.
 */
static void
test_nodelist_node_is_dir(void *arg)
{
  (void)arg;

  routerstatus_t rs;
  routerinfo_t ri;
  node_t node;
  memset(&node, 0, sizeof(node_t));
  memset(&rs, 0, sizeof(routerstatus_t));
  memset(&ri, 0, sizeof(routerinfo_t));

  tt_assert(!node_is_dir(&node));

  node.rs = &rs;
  tt_assert(!node_is_dir(&node));

  rs.is_v2_dir = 1;
  tt_assert(node_is_dir(&node));

  rs.is_v2_dir = 0;
  rs.dir_port = 1;
  tt_assert(! node_is_dir(&node));

  node.rs = NULL;
  tt_assert(!node_is_dir(&node));
  node.ri = &ri;
  ri.supports_tunnelled_dir_requests = 1;
  tt_assert(node_is_dir(&node));
  ri.supports_tunnelled_dir_requests = 0;
  ri.dir_port = 1;
  tt_assert(! node_is_dir(&node));

 done:
  return;
}

static networkstatus_t *dummy_ns = NULL;
static networkstatus_t *
mock_networkstatus_get_latest_consensus(void)
{
  return dummy_ns;
}
static networkstatus_t *
mock_networkstatus_get_latest_consensus_by_flavor(consensus_flavor_t f)
{
  tor_assert(f == FLAV_MICRODESC);
  return dummy_ns;
}

static void
test_nodelist_ed_id(void *arg)
{
#define N_NODES 5
  routerstatus_t *rs[N_NODES];
  microdesc_t *md[N_NODES];
  routerinfo_t *ri[N_NODES];
  networkstatus_t *ns;
  int i;
  (void)arg;

  ns = tor_malloc_zero(sizeof(networkstatus_t));
  ns->flavor = FLAV_MICRODESC;
  ns->routerstatus_list = smartlist_new();
  dummy_ns = ns;
  MOCK(networkstatus_get_latest_consensus,
       mock_networkstatus_get_latest_consensus);
  MOCK(networkstatus_get_latest_consensus_by_flavor,
       mock_networkstatus_get_latest_consensus_by_flavor);

  /* Make a bunch of dummy objects that we can play around with.  Only set the
     necessary fields */

  for (i = 0; i < N_NODES; ++i) {
    rs[i] = tor_malloc_zero(sizeof(*rs[i]));
    md[i] = tor_malloc_zero(sizeof(*md[i]));
    ri[i] = tor_malloc_zero(sizeof(*ri[i]));

    crypto_rand(md[i]->digest, sizeof(md[i]->digest));
    md[i]->ed25519_identity_pkey = tor_malloc(sizeof(ed25519_public_key_t));
    crypto_rand((char*)md[i]->ed25519_identity_pkey,
                sizeof(ed25519_public_key_t));
    crypto_rand(rs[i]->identity_digest, sizeof(rs[i]->identity_digest));
    memcpy(ri[i]->cache_info.identity_digest, rs[i]->identity_digest,
           DIGEST_LEN);
    memcpy(rs[i]->descriptor_digest, md[i]->digest, DIGEST256_LEN);
    ri[i]->cache_info.signing_key_cert = tor_malloc_zero(sizeof(tor_cert_t));
    memcpy(&ri[i]->cache_info.signing_key_cert->signing_key,
           md[i]->ed25519_identity_pkey, sizeof(ed25519_public_key_t));

    if (i < 3)
      smartlist_add(ns->routerstatus_list, rs[i]);
  }

  tt_int_op(0, OP_EQ, smartlist_len(nodelist_get_list()));

  nodelist_set_consensus(ns);

  tt_int_op(3, OP_EQ, smartlist_len(nodelist_get_list()));

  /* No Ed25519 info yet, so nothing has an ED id. */
  tt_ptr_op(NULL, OP_EQ, node_get_by_ed25519_id(md[0]->ed25519_identity_pkey));

  /* Register the first one by md, then look it up. */
  node_t *n = nodelist_add_microdesc(md[0]);
  tt_ptr_op(n, OP_EQ, node_get_by_ed25519_id(md[0]->ed25519_identity_pkey));

  /* Register the second by ri, then look it up. */
  routerinfo_t *ri_old = NULL;
  n = nodelist_set_routerinfo(ri[1], &ri_old);
  tt_ptr_op(n, OP_EQ, node_get_by_ed25519_id(md[1]->ed25519_identity_pkey));
  tt_ptr_op(ri_old, OP_EQ, NULL);

  /* Register it by md too. */
  node_t *n2 = nodelist_add_microdesc(md[1]);
  tt_ptr_op(n2, OP_EQ, n);
  tt_ptr_op(n, OP_EQ, node_get_by_ed25519_id(md[1]->ed25519_identity_pkey));

  /* Register the 4th by ri only -- we never put it into the networkstatus,
   * so it has to be independent */
  node_t *n3 = nodelist_set_routerinfo(ri[3], &ri_old);
  tt_ptr_op(n3, OP_EQ, node_get_by_ed25519_id(md[3]->ed25519_identity_pkey));
  tt_ptr_op(ri_old, OP_EQ, NULL);
  tt_int_op(4, OP_EQ, smartlist_len(nodelist_get_list()));

  /* Register the 5th by ri only, and rewrite its ed25519 pubkey to be
   * the same as the 4th, to test the duplicate ed25519 key logging in
   * nodelist.c */
  memcpy(md[4]->ed25519_identity_pkey, md[3]->ed25519_identity_pkey,
         sizeof(ed25519_public_key_t));
  memcpy(&ri[4]->cache_info.signing_key_cert->signing_key,
         md[3]->ed25519_identity_pkey, sizeof(ed25519_public_key_t));

  setup_capture_of_logs(LOG_NOTICE);
  node_t *n4 = nodelist_set_routerinfo(ri[4], &ri_old);
  tt_ptr_op(ri_old, OP_EQ, NULL);
  tt_int_op(5, OP_EQ, smartlist_len(nodelist_get_list()));
  tt_ptr_op(n4, OP_NE, node_get_by_ed25519_id(md[3]->ed25519_identity_pkey));
  tt_ptr_op(n3, OP_EQ, node_get_by_ed25519_id(md[3]->ed25519_identity_pkey));
  expect_log_msg_containing("Reused ed25519_id");

 done:
  teardown_capture_of_logs();
  for (i = 0; i < N_NODES; ++i) {
    tor_free(rs[i]);
    tor_free(md[i]->ed25519_identity_pkey);
    tor_free(md[i]);
    tor_free(ri[i]->cache_info.signing_key_cert);
    tor_free(ri[i]);
  }
  smartlist_clear(ns->routerstatus_list);
  networkstatus_vote_free(ns);
  UNMOCK(networkstatus_get_latest_consensus);
  UNMOCK(networkstatus_get_latest_consensus_by_flavor);
#undef N_NODES
}

#define NODE(name, flags) \
  { #name, test_nodelist_##name, (flags), NULL, NULL }

struct testcase_t nodelist_tests[] = {
  NODE(node_get_verbose_nickname_by_id_null_node, TT_FORK),
  NODE(node_get_verbose_nickname_not_named, TT_FORK),
  NODE(node_is_dir, TT_FORK),
  NODE(ed_id, TT_FORK),
  END_OF_TESTCASES
};

