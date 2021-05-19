/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_cache.c
 * \brief Test hidden service caches.
 */

#define CONNECTION_PRIVATE
#define DIRCACHE_PRIVATE
#define DIRCLIENT_PRIVATE
#define HS_CACHE_PRIVATE
#define CHANNEL_OBJECT_PRIVATE

#include "trunnel/ed25519_cert.h"
#include "feature/hs/hs_cache.h"
#include "feature/rend/rendcache.h"
#include "feature/dircache/dircache.h"
#include "feature/dirclient/dirclient.h"
#include "feature/nodelist/networkstatus.h"
#include "core/mainloop/connection.h"
#include "core/proto/proto_http.h"
#include "core/or/circuitlist.h"
#include "core/or/channel.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_rand.h"

#include "core/or/edge_connection_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/or_connection_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "feature/nodelist/networkstatus_st.h"

#include "test/hs_test_helpers.h"
#include "test/test_helpers.h"
#include "test/test.h"

/* Static variable used to encoded the HSDir query. */
static char query_b64[256];

/* Build an HSDir query using a ed25519 public key. */
static const char *
helper_get_hsdir_query(const hs_descriptor_t *desc)
{
  ed25519_public_to_base64(query_b64, &desc->plaintext_data.blinded_pubkey);
  return query_b64;
}

static void
init_test(void)
{
  /* Always needed. Initialize the subsystem. */
  hs_cache_init();
  /* We need the v2 cache since our OOM and cache cleanup does poke at it. */
  rend_cache_init();
}

static void
test_directory(void *arg)
{
  int ret;
  size_t oom_size;
  char *desc1_str = NULL;
  const char *desc_out;
  ed25519_keypair_t signing_kp1;
  hs_descriptor_t *desc1 = NULL;

  (void) arg;

  init_test();
  /* Generate a valid descriptor with normal values. */
  ret = ed25519_keypair_generate(&signing_kp1, 0);
  tt_int_op(ret, OP_EQ, 0);
  desc1 = hs_helper_build_hs_desc_with_ip(&signing_kp1);
  tt_assert(desc1);
  ret = hs_desc_encode_descriptor(desc1, &signing_kp1, NULL, &desc1_str);
  tt_int_op(ret, OP_EQ, 0);

  /* Very first basic test, should be able to be stored, survive a
   * clean, found with a lookup and then cleaned by our OOM. */
  {
    ret = hs_cache_store_as_dir(desc1_str);
    tt_int_op(ret, OP_EQ, 0);
    /* Re-add, it should fail since we already have it. */
    ret = hs_cache_store_as_dir(desc1_str);
    tt_int_op(ret, OP_EQ, -1);
    /* Try to clean now which should be fine, there is at worst few seconds
     * between the store and this call. */
    hs_cache_clean_as_dir(time(NULL));
    /* We should find it in our cache. */
    ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), &desc_out);
    tt_int_op(ret, OP_EQ, 1);
    tt_str_op(desc_out, OP_EQ, desc1_str);
    /* Tell our OOM to run and to at least remove a byte which will result in
     * removing the descriptor from our cache. */
    oom_size = hs_cache_handle_oom(time(NULL), 1);
    tt_int_op(oom_size, OP_GE, 1);
    ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), NULL);
    tt_int_op(ret, OP_EQ, 0);
  }

  /* Store two descriptors and remove the expiring one only. */
  {
    ed25519_keypair_t signing_kp_zero;
    ret = ed25519_keypair_generate(&signing_kp_zero, 0);
    tt_int_op(ret, OP_EQ, 0);
    hs_descriptor_t *desc_zero_lifetime;
    desc_zero_lifetime = hs_helper_build_hs_desc_with_ip(&signing_kp_zero);
    tt_assert(desc_zero_lifetime);
    desc_zero_lifetime->plaintext_data.revision_counter = 1;
    desc_zero_lifetime->plaintext_data.lifetime_sec = 0;
    char *desc_zero_lifetime_str;
    ret = hs_desc_encode_descriptor(desc_zero_lifetime, &signing_kp_zero,
                                    NULL, &desc_zero_lifetime_str);
    tt_int_op(ret, OP_EQ, 0);

    ret = hs_cache_store_as_dir(desc1_str);
    tt_int_op(ret, OP_EQ, 0);
    ret = hs_cache_store_as_dir(desc_zero_lifetime_str);
    tt_int_op(ret, OP_EQ, 0);
    /* This one should clear out our zero lifetime desc. */
    hs_cache_clean_as_dir(time(NULL));
    /* We should find desc1 in our cache. */
    ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), &desc_out);
    tt_int_op(ret, OP_EQ, 1);
    tt_str_op(desc_out, OP_EQ, desc1_str);
    /* We should NOT find our zero lifetime desc in our cache. */
    ret = hs_cache_lookup_as_dir(3,
                                 helper_get_hsdir_query(desc_zero_lifetime),
                                 NULL);
    tt_int_op(ret, OP_EQ, 0);
    /* Cleanup our entire cache. */
    oom_size = hs_cache_handle_oom(time(NULL), 1);
    tt_int_op(oom_size, OP_GE, 1);
    hs_descriptor_free(desc_zero_lifetime);
    tor_free(desc_zero_lifetime_str);
  }

  /* Throw junk at it. */
  {
    ret = hs_cache_store_as_dir("blah");
    tt_int_op(ret, OP_EQ, -1);
    /* Poor attempt at tricking the decoding. */
    ret = hs_cache_store_as_dir("hs-descriptor 3\nJUNK");
    tt_int_op(ret, OP_EQ, -1);
    /* Undecodable base64 query. */
    ret = hs_cache_lookup_as_dir(3, "blah", NULL);
    tt_int_op(ret, OP_EQ, -1);
    /* Decodable base64 query but wrong ed25519 size. */
    ret = hs_cache_lookup_as_dir(3, "dW5pY29ybg==", NULL);
    tt_int_op(ret, OP_EQ, -1);
  }

  /* Test descriptor replacement with revision counter. */
  {
    char *new_desc_str;

    /* Add a descriptor. */
    ret = hs_cache_store_as_dir(desc1_str);
    tt_int_op(ret, OP_EQ, 0);
    ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), &desc_out);
    tt_int_op(ret, OP_EQ, 1);
    /* Bump revision counter. */
    desc1->plaintext_data.revision_counter++;
    ret = hs_desc_encode_descriptor(desc1, &signing_kp1, NULL, &new_desc_str);
    tt_int_op(ret, OP_EQ, 0);
    ret = hs_cache_store_as_dir(new_desc_str);
    tt_int_op(ret, OP_EQ, 0);
    /* Look it up, it should have been replaced. */
    ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), &desc_out);
    tt_int_op(ret, OP_EQ, 1);
    tt_str_op(desc_out, OP_EQ, new_desc_str);
    tor_free(new_desc_str);
  }

 done:
  hs_descriptor_free(desc1);
  tor_free(desc1_str);
}

static void
test_clean_as_dir(void *arg)
{
  size_t ret;
  char *desc1_str = NULL;
  time_t now = time(NULL);
  hs_descriptor_t *desc1 = NULL;
  ed25519_keypair_t signing_kp1;

  (void) arg;

  init_test();

  /* Generate a valid descriptor with values. */
  ret = ed25519_keypair_generate(&signing_kp1, 0);
  tt_int_op(ret, OP_EQ, 0);
  desc1 = hs_helper_build_hs_desc_with_ip(&signing_kp1);
  tt_assert(desc1);
  ret = hs_desc_encode_descriptor(desc1, &signing_kp1, NULL, &desc1_str);
  tt_int_op(ret, OP_EQ, 0);
  ret = hs_cache_store_as_dir(desc1_str);
  tt_int_op(ret, OP_EQ, 0);

  /* With the lifetime being 3 hours, a cleanup shouldn't remove it. */
  ret = cache_clean_v3_as_dir(now, 0);
  tt_int_op(ret, OP_EQ, 0);
  /* Should be present after clean up. */
  ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), NULL);
  tt_int_op(ret, OP_EQ, 1);
  /* Set a cutoff 100 seconds in the past. It should not remove the entry
   * since the entry is still recent enough. */
  ret = cache_clean_v3_as_dir(now, now - 100);
  tt_int_op(ret, OP_EQ, 0);
  /* Should be present after clean up. */
  ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), NULL);
  tt_int_op(ret, OP_EQ, 1);
  /* Set a cutoff of 100 seconds in the future. It should remove the entry
   * that we've just added since it's not too old for the cutoff. */
  ret = cache_clean_v3_as_dir(now, now + 100);
  tt_int_op(ret, OP_GT, 0);
  /* Shouldn't be present after clean up. */
  ret = hs_cache_lookup_as_dir(3, helper_get_hsdir_query(desc1), NULL);
  tt_int_op(ret, OP_EQ, 0);

 done:
  hs_descriptor_free(desc1);
  tor_free(desc1_str);
}

/* Test helper: Fetch an HS descriptor from an HSDir (for the hidden service
   with <b>blinded_key</b>. Return the received descriptor string. */
static char *
helper_fetch_desc_from_hsdir(const ed25519_public_key_t *blinded_key)
{
  int retval;

  char *received_desc = NULL;
  char *hsdir_query_str = NULL;

  /* The dir conn we are going to simulate */
  dir_connection_t *conn = NULL;
  edge_connection_t *edge_conn = NULL;
  or_circuit_t *or_circ = NULL;

  /* First extract the blinded public key that we are going to use in our
     query, and then build the actual query string. */
  {
    char hsdir_cache_key[ED25519_BASE64_LEN+1];

    ed25519_public_to_base64(hsdir_cache_key, blinded_key);
    tor_asprintf(&hsdir_query_str, GET("/tor/hs/3/%s"), hsdir_cache_key);
  }

  /* Simulate an HTTP GET request to the HSDir */
  conn = dir_connection_new(AF_INET);
  tt_assert(conn);
  TO_CONN(conn)->linked = 1; /* Signal that it is encrypted. */
  tor_addr_from_ipv4h(&conn->base_.addr, 0x7f000001);

  /* Pretend this conn is anonymous. */
  edge_conn = edge_connection_new(CONN_TYPE_EXIT, AF_INET);
  TO_CONN(conn)->linked_conn = TO_CONN(edge_conn);
  or_circ = or_circuit_new(0, NULL);
  or_circ->p_chan = tor_malloc_zero(sizeof(channel_t));
  edge_conn->on_circuit = TO_CIRCUIT(or_circ);

  retval = directory_handle_command_get(conn, hsdir_query_str,
                                        NULL, 0);
  tt_int_op(retval, OP_EQ, 0);

  /* Read the descriptor that the HSDir just served us */
  {
    char *headers = NULL;
    size_t body_used = 0;

    fetch_from_buf_http(TO_CONN(conn)->outbuf, &headers, MAX_HEADERS_SIZE,
                        &received_desc, &body_used, HS_DESC_MAX_LEN, 0);
    tor_free(headers);
  }

 done:
  tor_free(hsdir_query_str);
  if (conn) {
    tor_free(or_circ->p_chan);
    connection_free_minimal(TO_CONN(conn)->linked_conn);
    connection_free_minimal(TO_CONN(conn));
  }

  return received_desc;
}

/* Publish a descriptor to the HSDir, then fetch it. Check that the received
   descriptor matches the published one. */
static void
test_upload_and_download_hs_desc(void *arg)
{
  int retval;
  hs_descriptor_t *published_desc = NULL;

  char *published_desc_str = NULL;
  char *received_desc_str = NULL;

  (void) arg;

  /* Initialize HSDir cache subsystem */
  init_test();

  /* Test a descriptor not found in the directory cache. */
  {
    ed25519_public_key_t blinded_key;
    memset(&blinded_key.pubkey, 'A', sizeof(blinded_key.pubkey));
    received_desc_str = helper_fetch_desc_from_hsdir(&blinded_key);
    tt_int_op(strlen(received_desc_str), OP_EQ, 0);
    tor_free(received_desc_str);
  }

  /* Generate a valid descriptor with normal values. */
  {
    ed25519_keypair_t signing_kp;
    retval = ed25519_keypair_generate(&signing_kp, 0);
    tt_int_op(retval, OP_EQ, 0);
    published_desc = hs_helper_build_hs_desc_with_ip(&signing_kp);
    tt_assert(published_desc);
    retval = hs_desc_encode_descriptor(published_desc, &signing_kp,
                                       NULL, &published_desc_str);
    tt_int_op(retval, OP_EQ, 0);
  }

  /* Publish descriptor to the HSDir */
  {
    retval = handle_post_hs_descriptor("/tor/hs/3/publish",published_desc_str);
    tt_int_op(retval, OP_EQ, 200);
  }

  /* Simulate a fetch of the previously published descriptor */
  {
    const ed25519_public_key_t *blinded_key;
    blinded_key = &published_desc->plaintext_data.blinded_pubkey;
    received_desc_str = helper_fetch_desc_from_hsdir(blinded_key);
  }

  /* Verify we received the exact same descriptor we published earlier */
  tt_str_op(received_desc_str, OP_EQ, published_desc_str);
  tor_free(received_desc_str);

  /* With a valid descriptor in the directory cache, try again an invalid. */
  {
    ed25519_public_key_t blinded_key;
    memset(&blinded_key.pubkey, 'A', sizeof(blinded_key.pubkey));
    received_desc_str = helper_fetch_desc_from_hsdir(&blinded_key);
    tt_int_op(strlen(received_desc_str), OP_EQ, 0);
  }

 done:
  tor_free(received_desc_str);
  tor_free(published_desc_str);
  hs_descriptor_free(published_desc);
}

/* Test that HSDirs reject outdated descriptors based on their revision
 * counter. Also test that HSDirs correctly replace old descriptors with newer
 * descriptors. */
static void
test_hsdir_revision_counter_check(void *arg)
{
  int retval;

  ed25519_keypair_t signing_kp;

  hs_descriptor_t *published_desc = NULL;
  char *published_desc_str = NULL;

  hs_subcredential_t subcredential;
  char *received_desc_str = NULL;
  hs_descriptor_t *received_desc = NULL;

  (void) arg;

  /* Initialize HSDir cache subsystem */
  init_test();

  /* Generate a valid descriptor with normal values. */
  {
    retval = ed25519_keypair_generate(&signing_kp, 0);
    tt_int_op(retval, OP_EQ, 0);
    published_desc = hs_helper_build_hs_desc_with_ip(&signing_kp);
    tt_assert(published_desc);
    retval = hs_desc_encode_descriptor(published_desc, &signing_kp,
                                       NULL, &published_desc_str);
    tt_int_op(retval, OP_EQ, 0);
  }

  /* Publish descriptor to the HSDir */
  {
    retval = handle_post_hs_descriptor("/tor/hs/3/publish",published_desc_str);
    tt_int_op(retval, OP_EQ, 200);
  }

  /* Try publishing again with the same revision counter: Should fail. */
  {
    retval = handle_post_hs_descriptor("/tor/hs/3/publish",published_desc_str);
    tt_int_op(retval, OP_EQ, 400);
  }

  /* Fetch the published descriptor and validate the revision counter. */
  {
    const ed25519_public_key_t *blinded_key;

    blinded_key = &published_desc->plaintext_data.blinded_pubkey;
    hs_get_subcredential(&signing_kp.pubkey, blinded_key, &subcredential);
    received_desc_str = helper_fetch_desc_from_hsdir(blinded_key);

    retval = hs_desc_decode_descriptor(received_desc_str,
                                       &subcredential, NULL, &received_desc);
    tt_int_op(retval, OP_EQ, HS_DESC_DECODE_OK);
    tt_assert(received_desc);

    /* Check that the revision counter is correct */
    tt_u64_op(received_desc->plaintext_data.revision_counter, OP_EQ, 42);

    hs_descriptor_free(received_desc);
    received_desc = NULL;
    tor_free(received_desc_str);
  }

  /* Increment the revision counter and try again. Should work. */
  {
    published_desc->plaintext_data.revision_counter = 1313;
    tor_free(published_desc_str);
    retval = hs_desc_encode_descriptor(published_desc, &signing_kp,
                                       NULL, &published_desc_str);
    tt_int_op(retval, OP_EQ, 0);

    retval = handle_post_hs_descriptor("/tor/hs/3/publish",published_desc_str);
    tt_int_op(retval, OP_EQ, 200);
  }

  /* Again, fetch the published descriptor and perform the revision counter
     validation. The revision counter must have changed. */
  {
    const ed25519_public_key_t *blinded_key;

    blinded_key = &published_desc->plaintext_data.blinded_pubkey;
    received_desc_str = helper_fetch_desc_from_hsdir(blinded_key);

    retval = hs_desc_decode_descriptor(received_desc_str,
                                       &subcredential, NULL, &received_desc);
    tt_int_op(retval, OP_EQ, HS_DESC_DECODE_OK);
    tt_assert(received_desc);

    /* Check that the revision counter is the latest */
    tt_u64_op(received_desc->plaintext_data.revision_counter, OP_EQ, 1313);
  }

 done:
  hs_descriptor_free(published_desc);
  hs_descriptor_free(received_desc);
  tor_free(received_desc_str);
  tor_free(published_desc_str);
}

static networkstatus_t mock_ns;

static networkstatus_t *
mock_networkstatus_get_reasonably_live_consensus(time_t now, int flavor)
{
  (void) now;
  (void) flavor;
  return &mock_ns;
}

/** Test that we can store HS descriptors in the client HS cache. */
static void
test_client_cache(void *arg)
{
  int retval;
  ed25519_keypair_t signing_kp;
  hs_descriptor_t *published_desc = NULL;
  char *published_desc_str = NULL;
  hs_subcredential_t wanted_subcredential;
  response_handler_args_t *args = NULL;
  dir_connection_t *conn = NULL;

  (void) arg;

  /* Initialize HSDir cache subsystem */
  init_test();

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  /* Set consensus time */
  parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                           &mock_ns.valid_after);
  parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                           &mock_ns.fresh_until);
  parse_rfc1123_time("Sat, 26 Oct 1985 16:00:00 UTC",
                           &mock_ns.valid_until);

  /* Generate a valid descriptor with normal values. */
  {
    retval = ed25519_keypair_generate(&signing_kp, 0);
    tt_int_op(retval, OP_EQ, 0);
    published_desc = hs_helper_build_hs_desc_with_ip(&signing_kp);
    tt_assert(published_desc);
    retval = hs_desc_encode_descriptor(published_desc, &signing_kp,
                                       NULL, &published_desc_str);
    tt_int_op(retval, OP_EQ, 0);
    memcpy(&wanted_subcredential, &published_desc->subcredential,
           sizeof(hs_subcredential_t));
    tt_assert(!fast_mem_is_zero((char*)wanted_subcredential.subcred,
                                DIGEST256_LEN));
  }

  /* Test handle_response_fetch_hsdesc_v3() */
  {
    args = tor_malloc_zero(sizeof(response_handler_args_t));
    args->status_code = 200;
    args->reason = NULL;
    args->body = published_desc_str;
    args->body_len = strlen(published_desc_str);

    conn = tor_malloc_zero(sizeof(dir_connection_t));
    conn->hs_ident = tor_malloc_zero(sizeof(hs_ident_dir_conn_t));
    ed25519_pubkey_copy(&conn->hs_ident->identity_pk, &signing_kp.pubkey);
  }

  /* store the descriptor! */
  retval = handle_response_fetch_hsdesc_v3(conn, args);
  tt_int_op(retval, == , 0);

  /* Progress time a bit and attempt to clean cache: our desc should not be
   * cleaned since we still in the same TP. */
  {
    parse_rfc1123_time("Sat, 27 Oct 1985 02:00:00 UTC",
                       &mock_ns.valid_after);
    parse_rfc1123_time("Sat, 27 Oct 1985 03:00:00 UTC",
                       &mock_ns.fresh_until);
    parse_rfc1123_time("Sat, 27 Oct 1985 05:00:00 UTC",
                       &mock_ns.valid_until);

    /* fetch the descriptor and make sure it's there */
    const hs_descriptor_t *cached_desc = NULL;
    cached_desc = hs_cache_lookup_as_client(&signing_kp.pubkey);
    tt_assert(cached_desc);
    tt_mem_op(cached_desc->subcredential.subcred,
              OP_EQ, wanted_subcredential.subcred,
              SUBCRED_LEN);
  }

  /* Progress time to next TP and check that desc was cleaned */
  {
    parse_rfc1123_time("Sat, 27 Oct 1985 12:00:00 UTC",
                       &mock_ns.valid_after);
    parse_rfc1123_time("Sat, 27 Oct 1985 13:00:00 UTC",
                       &mock_ns.fresh_until);
    parse_rfc1123_time("Sat, 27 Oct 1985 15:00:00 UTC",
                       &mock_ns.valid_until);

    const hs_descriptor_t *cached_desc = NULL;
    cached_desc = hs_cache_lookup_as_client(&signing_kp.pubkey);
    tt_assert(!cached_desc);
  }

 done:
  tor_free(args);
  hs_descriptor_free(published_desc);
  tor_free(published_desc_str);
  if (conn) {
    tor_free(conn->hs_ident);
    tor_free(conn);
  }
}

/** Test that we can store HS descriptors in the client HS cache. */
static void
test_client_cache_decrypt(void *arg)
{
  int ret;
  char *desc_encoded = NULL;
  uint8_t descriptor_cookie[HS_DESC_DESCRIPTOR_COOKIE_LEN];
  curve25519_keypair_t client_kp;
  ed25519_keypair_t service_kp;
  hs_descriptor_t *desc = NULL;
  const hs_descriptor_t *search_desc;
  const char *search_desc_encoded;

  (void) arg;

  /* Initialize HSDir cache subsystem */
  hs_init();

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  /* Set consensus time */
  parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                     &mock_ns.valid_after);
  parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                     &mock_ns.fresh_until);
  parse_rfc1123_time("Sat, 26 Oct 1985 16:00:00 UTC",
                     &mock_ns.valid_until);

  /* Generate a valid descriptor with normal values. */
  {
    ret = ed25519_keypair_generate(&service_kp, 0);
    tt_int_op(ret, OP_EQ, 0);
    ret = curve25519_keypair_generate(&client_kp, 0);
    tt_int_op(ret, OP_EQ, 0);
    crypto_rand((char *) descriptor_cookie, sizeof(descriptor_cookie));

    desc = hs_helper_build_hs_desc_with_client_auth(descriptor_cookie,
                                                    &client_kp.pubkey,
                                                    &service_kp);
    tt_assert(desc);
    ret = hs_desc_encode_descriptor(desc, &service_kp, descriptor_cookie,
                                    &desc_encoded);
    tt_int_op(ret, OP_EQ, 0);
  }

  /* Put it in the cache. Should not be decrypted since the client
   * authorization creds were not added to the global map. */
  ret = hs_cache_store_as_client(desc_encoded, &service_kp.pubkey);
  tt_int_op(ret, OP_EQ, HS_DESC_DECODE_NEED_CLIENT_AUTH);

  /* We should not be able to decrypt anything. */
  ret = hs_cache_client_new_auth_parse(&service_kp.pubkey);
  tt_int_op(ret, OP_EQ, false);

  /* Add client auth to global map. */
  hs_helper_add_client_auth(&service_kp.pubkey, &client_kp.seckey);

  /* We should not be able to decrypt anything. */
  ret = hs_cache_client_new_auth_parse(&service_kp.pubkey);
  tt_int_op(ret, OP_EQ, true);

  /* Lookup the cache to make sure it is usable and there. */
  search_desc = hs_cache_lookup_as_client(&service_kp.pubkey);
  tt_assert(search_desc);
  search_desc_encoded = hs_cache_lookup_encoded_as_client(&service_kp.pubkey);
  tt_mem_op(search_desc_encoded, OP_EQ, desc_encoded, strlen(desc_encoded));

 done:
  hs_descriptor_free(desc);
  tor_free(desc_encoded);

  hs_free_all();

  UNMOCK(networkstatus_get_reasonably_live_consensus);
}

static void
test_client_cache_remove(void *arg)
{
  int ret;
  ed25519_keypair_t service_kp;
  hs_descriptor_t *desc1 = NULL;

  (void) arg;

  hs_init();

  MOCK(networkstatus_get_reasonably_live_consensus,
       mock_networkstatus_get_reasonably_live_consensus);

  /* Set consensus time. Lookup will not return the entry if it has expired
   * and it is checked against the consensus valid_after time. */
  parse_rfc1123_time("Sat, 26 Oct 1985 13:00:00 UTC",
                     &mock_ns.valid_after);
  parse_rfc1123_time("Sat, 26 Oct 1985 14:00:00 UTC",
                     &mock_ns.fresh_until);
  parse_rfc1123_time("Sat, 26 Oct 1985 16:00:00 UTC",
                     &mock_ns.valid_until);

  /* Generate service keypair */
  tt_int_op(0, OP_EQ, ed25519_keypair_generate(&service_kp, 0));

  /* Build a descriptor and cache it. */
  {
    char *encoded;
    desc1 = hs_helper_build_hs_desc_with_ip(&service_kp);
    tt_assert(desc1);
    ret = hs_desc_encode_descriptor(desc1, &service_kp, NULL, &encoded);
    tt_int_op(ret, OP_EQ, 0);
    tt_assert(encoded);

    /* Store it */
    ret = hs_cache_store_as_client(encoded, &service_kp.pubkey);
    tt_int_op(ret, OP_EQ, HS_DESC_DECODE_OK);
    tor_free(encoded);
    tt_assert(hs_cache_lookup_as_client(&service_kp.pubkey));
  }

  /* Remove the cached entry. */
  hs_cache_remove_as_client(&service_kp.pubkey);
  tt_assert(!hs_cache_lookup_as_client(&service_kp.pubkey));

 done:
  hs_descriptor_free(desc1);
  hs_free_all();

  UNMOCK(networkstatus_get_reasonably_live_consensus);
}

struct testcase_t hs_cache[] = {
  /* Encoding tests. */
  { "directory", test_directory, TT_FORK,
    NULL, NULL },
  { "clean_as_dir", test_clean_as_dir, TT_FORK,
    NULL, NULL },
  { "hsdir_revision_counter_check", test_hsdir_revision_counter_check, TT_FORK,
    NULL, NULL },
  { "upload_and_download_hs_desc", test_upload_and_download_hs_desc, TT_FORK,
    NULL, NULL },
  { "client_cache", test_client_cache, TT_FORK,
    NULL, NULL },
  { "client_cache_decrypt", test_client_cache_decrypt, TT_FORK,
    NULL, NULL },
  { "client_cache_remove", test_client_cache_remove, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
