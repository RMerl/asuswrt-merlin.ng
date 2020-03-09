/* Copyright (c) 2017-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_control.c
 * \brief Unit tests for hidden service control port event and command.
 **/

#define CONTROL_EVENTS_PRIVATE

#include "core/or/or.h"
#include "test/test.h"
#include "feature/control/control.h"
#include "feature/control/control_events.h"
#include "feature/control/control_fmt.h"
#include "app/config/config.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_control.h"
#include "feature/nodelist/nodelist.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "lib/crypt_ops/crypto_format.h"

#include "test/test_helpers.h"

/* mock ID digest and longname for node that's in nodelist */
#define HSDIR_EXIST_ID \
  "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA" \
  "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA"
#define STR_HSDIR_EXIST_LONGNAME \
  "$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=TestDir"
#define STR_HSDIR_NONE_EXIST_LONGNAME \
  "$BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"

/* Helper global variable for hidden service descriptor event test.
 * It's used as a pointer to dynamically created message buffer in
 * send_control_event_string_replacement function, which mocks
 * send_control_event_string function.
 *
 * Always free it after use! */
static char *received_msg = NULL;

/** Mock function for send_control_event_string
 */
static void
queue_control_event_string_replacement(uint16_t event, char *msg)
{
  (void) event;
  tor_free(received_msg);
  received_msg = msg;
}

/** Mock function for node_describe_longname_by_id, it returns either
 * STR_HSDIR_EXIST_LONGNAME or STR_HSDIR_NONE_EXIST_LONGNAME
 */
static const char *
node_describe_longname_by_id_replacement(const char *id_digest)
{
  if (!strcmp(id_digest, HSDIR_EXIST_ID)) {
    return STR_HSDIR_EXIST_LONGNAME;
  } else {
    return STR_HSDIR_NONE_EXIST_LONGNAME;
  }
}

/* HSDir fetch index is a series of 'D' */
#define HSDIR_INDEX_FETCH_HEX \
  "4343434343434343434343434343434343434343434343434343434343434343"
#define HSDIR_INDEX_STORE_HEX \
  "4444444444444444444444444444444444444444444444444444444444444444"

static const node_t *
mock_node_get_by_id(const char *digest)
{
  static node_t node;
  memcpy(node.identity, digest, DIGEST_LEN);
  memset(node.hsdir_index.fetch, 'C', DIGEST256_LEN);
  memset(node.hsdir_index.store_first, 'D', DIGEST256_LEN);
  return &node;
}

static void
test_hs_desc_event(void *arg)
{
  int ret;
  char *expected_msg = NULL;
  char onion_address[HS_SERVICE_ADDR_LEN_BASE32 + 1];
  ed25519_keypair_t identity_kp;
  ed25519_public_key_t blinded_pk;
  char base64_blinded_pk[ED25519_BASE64_LEN + 1];
  routerstatus_t hsdir_rs;
  hs_ident_dir_conn_t ident;

  (void) arg;
  MOCK(queue_control_event_string,
       queue_control_event_string_replacement);
  MOCK(node_describe_longname_by_id,
       node_describe_longname_by_id_replacement);
  MOCK(node_get_by_id, mock_node_get_by_id);

  /* Setup what we need for this test. */
  ed25519_keypair_generate(&identity_kp, 0);
  hs_build_address(&identity_kp.pubkey, HS_VERSION_THREE, onion_address);
  ret = hs_address_is_valid(onion_address);
  tt_int_op(ret, OP_EQ, 1);
  memset(&blinded_pk, 'B', sizeof(blinded_pk));
  memset(&hsdir_rs, 0, sizeof(hsdir_rs));
  memcpy(hsdir_rs.identity_digest, HSDIR_EXIST_ID, DIGEST_LEN);
  ed25519_public_to_base64(base64_blinded_pk, &blinded_pk);
  memcpy(&ident.identity_pk, &identity_kp.pubkey,
         sizeof(ed25519_public_key_t));
  memcpy(&ident.blinded_pk, &blinded_pk, sizeof(blinded_pk));

  /* HS_DESC REQUESTED ... */
  hs_control_desc_event_requested(&identity_kp.pubkey, base64_blinded_pk,
                                  &hsdir_rs);
  tor_asprintf(&expected_msg, "650 HS_DESC REQUESTED %s NO_AUTH "
               STR_HSDIR_EXIST_LONGNAME " %s HSDIR_INDEX="
               HSDIR_INDEX_FETCH_HEX "\r\n",
               onion_address, base64_blinded_pk);
  tt_assert(received_msg);
  tt_str_op(received_msg, OP_EQ, expected_msg);
  tor_free(received_msg);
  tor_free(expected_msg);

  /* HS_DESC CREATED... */
  hs_control_desc_event_created(onion_address, &blinded_pk);
  tor_asprintf(&expected_msg, "650 HS_DESC CREATED %s UNKNOWN "
                              "UNKNOWN %s\r\n",
               onion_address, base64_blinded_pk);
  tt_assert(received_msg);
  tt_str_op(received_msg, OP_EQ, expected_msg);
  tor_free(received_msg);
  tor_free(expected_msg);

  /* HS_DESC UPLOAD... */
  uint8_t hsdir_index_store[DIGEST256_LEN];
  memset(hsdir_index_store, 'D', sizeof(hsdir_index_store));
  hs_control_desc_event_upload(onion_address, HSDIR_EXIST_ID,
                               &blinded_pk, hsdir_index_store);
  tor_asprintf(&expected_msg, "650 HS_DESC UPLOAD %s UNKNOWN "
                              STR_HSDIR_EXIST_LONGNAME " %s "
                              "HSDIR_INDEX=" HSDIR_INDEX_STORE_HEX "\r\n",
               onion_address, base64_blinded_pk);
  tt_assert(received_msg);
  tt_str_op(received_msg, OP_EQ, expected_msg);
  tor_free(received_msg);
  tor_free(expected_msg);

  /* HS_DESC FAILED... */
  hs_control_desc_event_failed(&ident, HSDIR_EXIST_ID, "BAD_DESC");
  tor_asprintf(&expected_msg, "650 HS_DESC FAILED %s NO_AUTH "
                              STR_HSDIR_EXIST_LONGNAME " %s "
                              "REASON=BAD_DESC\r\n",
               onion_address, base64_blinded_pk);
  tt_assert(received_msg);
  tt_str_op(received_msg, OP_EQ, expected_msg);
  tor_free(received_msg);
  tor_free(expected_msg);

  /* HS_DESC RECEIVED... */
  hs_control_desc_event_received(&ident, HSDIR_EXIST_ID);
  tor_asprintf(&expected_msg, "650 HS_DESC RECEIVED %s NO_AUTH "
                              STR_HSDIR_EXIST_LONGNAME " %s\r\n",
               onion_address, base64_blinded_pk);
  tt_assert(received_msg);
  tt_str_op(received_msg, OP_EQ, expected_msg);
  tor_free(received_msg);
  tor_free(expected_msg);

  /* HS_DESC UPLOADED... */
  hs_control_desc_event_uploaded(&ident, HSDIR_EXIST_ID);
  tor_asprintf(&expected_msg, "650 HS_DESC UPLOADED %s UNKNOWN "
                              STR_HSDIR_EXIST_LONGNAME "\r\n",
               onion_address);
  tt_assert(received_msg);
  tt_str_op(received_msg, OP_EQ, expected_msg);
  tor_free(received_msg);
  tor_free(expected_msg);

 done:
  UNMOCK(queue_control_event_string);
  UNMOCK(node_describe_longname_by_id);
  UNMOCK(node_get_by_id);
  tor_free(received_msg);
  tor_free(expected_msg);
}

struct testcase_t hs_control_tests[] = {
  { "hs_desc_event", test_hs_desc_event, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
