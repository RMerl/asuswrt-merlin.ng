/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_control.c
 * \brief Unit tests for hidden service control port event and command.
 **/

#define CONTROL_EVENTS_PRIVATE
#define HS_CLIENT_PRIVATE

#include "core/or/or.h"
#include "test/test.h"
#include "test/test_helpers.h"
#include "core/mainloop/connection.h"
#include "feature/control/control.h"
#include "feature/control/control_events.h"
#include "feature/control/control_cmd.h"
#include "feature/control/control_fmt.h"
#include "feature/control/control_connection_st.h"
#include "app/config/config.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_control.h"
#include "feature/nodelist/nodelist.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "lib/crypt_ops/crypto_format.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#else
#include <dirent.h>
#endif /* defined(_WIN32) */

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

/** Test that we can correctly add, remove and view client auth credentials
 *  using the control port. */
static void
test_hs_control_good_onion_client_auth_add(void *arg)
{
  (void) arg;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  int retval;
  ed25519_public_key_t service_identity_pk_2fv, service_identity_pk_jt4,
                       service_identity_pk_jam;
  control_connection_t conn;
  char *args = NULL;
  char *cp1 = NULL;
  size_t sz;

  hs_init();

  { /* Setup the control conn */
    memset(&conn, 0, sizeof(control_connection_t));
    TO_CONN(&conn)->outbuf = buf_new();
    conn.current_cmd = tor_strdup("ONION_CLIENT_AUTH_ADD");
  }

  { /* Setup the services */
    retval = hs_parse_address(
                 "2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd",
                 &service_identity_pk_2fv,
                 NULL, NULL);
    tt_int_op(retval, OP_EQ, 0);

    retval = hs_parse_address(
                 "jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd",
                 &service_identity_pk_jt4,
                 NULL, NULL);
    tt_int_op(retval, OP_EQ, 0);

    retval = hs_parse_address(
                 "jamie3vkiwibfiwucd6vxijskbhpjdyajmzeor4mc4i7yopvpo4p7cyd",
                 &service_identity_pk_jam,
                 NULL, NULL);
    tt_int_op(retval, OP_EQ, 0);
  }

  digest256map_t *client_auths = get_hs_client_auths_map();
  tt_assert(!client_auths);

  /* Register first service */
  args = tor_strdup("2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd "
                    "x25519:iJ1tjKCrMAbiFT2bVrCjhbfMDnE1fpaRbIS5ZHKUvEQ= ");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check contents */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");

  tor_free(cp1);
  tor_free(args);

  /* Register second service (even with an unrecognized argument) */
  args = tor_strdup("jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd "
           "x25519:eIIdIGoSZwI2Q/lSzpf92akGki5I+PZIDz37MA5BhlA= DropSound=No");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check contents */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");
  tor_free(cp1);
  tor_free(args);

  /* Register second service (even with an unrecognized argument) */
  args = tor_strdup("jamie3vkiwibfiwucd6vxijskbhpjdyajmzeor4mc4i7yopvpo4p7cyd "
           "x25519:FCV0c0ELDKKDpSFgVIB8Yow8Evj5iD+GoiTtK878NkQ= "
           "ClientName=MeganNicole ");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check contents */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");
  tor_free(cp1);

  client_auths = get_hs_client_auths_map();
  tt_assert(client_auths);
  tt_uint_op(digest256map_size(client_auths), OP_EQ, 3);

  hs_client_service_authorization_t *client_2fv =
    digest256map_get(client_auths, service_identity_pk_2fv.pubkey);
  tt_assert(client_2fv);
  tt_int_op(client_2fv->flags, OP_EQ, 0);

  hs_client_service_authorization_t *client_jt4 =
    digest256map_get(client_auths, service_identity_pk_jt4.pubkey);
  tt_assert(client_jt4);
  tt_int_op(client_jt4->flags, OP_EQ, 0);

  hs_client_service_authorization_t *client_jam =
    digest256map_get(client_auths, service_identity_pk_jam.pubkey);
  tt_assert(client_jam);
  tt_int_op(client_jam->flags, OP_EQ, 0);

  /* Now let's VIEW the auth credentials */
  tor_free(conn.current_cmd);
  conn.current_cmd = tor_strdup("ONION_CLIENT_AUTH_VIEW");

  /* First go with no arguments, so that we view all the credentials */
  tor_free(args);
  args = tor_strdup("");

#define VIEW_CORRECT_REPLY_NO_ADDR "250-ONION_CLIENT_AUTH_VIEW\r\n"   \
  "250-CLIENT 2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd "  \
    "x25519:iJ1tjKCrMAbiFT2bVrCjhbfMDnE1fpaRbIS5ZHKUvEQ=\r\n"   \
  "250-CLIENT jamie3vkiwibfiwucd6vxijskbhpjdyajmzeor4mc4i7yopvpo4p7cyd " \
    "x25519:FCV0c0ELDKKDpSFgVIB8Yow8Evj5iD+GoiTtK878NkQ= " \
    "ClientName=MeganNicole\r\n" \
  "250-CLIENT jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd " \
    "x25519:eIIdIGoSZwI2Q/lSzpf92akGki5I+PZIDz37MA5BhlA=\r\n"             \
  "250 OK\r\n"

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, VIEW_CORRECT_REPLY_NO_ADDR);
  tor_free(cp1);

  /* Now specify an HS addr, and see that we only view those creds */
  tor_free(args);
  args =
    tor_strdup("jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd");

#define VIEW_CORRECT_REPLY_JT4 "250-ONION_CLIENT_AUTH_VIEW " \
    "jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd\r\n"   \
  "250-CLIENT jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd " \
    "x25519:eIIdIGoSZwI2Q/lSzpf92akGki5I+PZIDz37MA5BhlA=\r\n" \
  "250 OK\r\n"

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, VIEW_CORRECT_REPLY_JT4);
  tor_free(cp1);

  /* Now try to REMOVE the auth credentials */
  tor_free(conn.current_cmd);
  conn.current_cmd = tor_strdup("ONION_CLIENT_AUTH_REMOVE");

  /* First try with a wrong addr */
  tor_free(args);
  args = tor_strdup("thatsok");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "512 Invalid v3 address \"thatsok\"\r\n");
  tor_free(cp1);

  client_jt4 = digest256map_get(client_auths, service_identity_pk_jt4.pubkey);
  tt_assert(client_jt4);

  /* Now actually remove them. */
  tor_free(args);
  args =tor_strdup("jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");
  tor_free(cp1);

  client_jt4 = digest256map_get(client_auths, service_identity_pk_jt4.pubkey);
  tt_assert(!client_jt4);

  /* Now try another time (we should get 'already removed' msg) */
  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "251 No credentials for "
           "\"jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd\"\r\n");
  tor_free(cp1);

  client_jt4 = digest256map_get(client_auths, service_identity_pk_jt4.pubkey);
  tt_assert(!client_jt4);

  /* Now also remove the other one */
  tor_free(args);
  args =
    tor_strdup("2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");
  tor_free(cp1);

  /* Now also remove the other one */
  tor_free(args);
  args =
    tor_strdup("jamie3vkiwibfiwucd6vxijskbhpjdyajmzeor4mc4i7yopvpo4p7cyd");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");
  tor_free(cp1);

  /* Finally, do another VIEW and see that we get nothing. */
  tor_free(conn.current_cmd);
  conn.current_cmd = tor_strdup("ONION_CLIENT_AUTH_VIEW");
  tor_free(args);
  args = tor_strdup("");

#define VIEW_CORRECT_REPLY_NOTHING "250-ONION_CLIENT_AUTH_VIEW\r\n250 OK\r\n"

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, VIEW_CORRECT_REPLY_NOTHING);
  tor_free(cp1);

  /* And a final VIEW with a wrong HS addr */
  tor_free(args);
  args = tor_strdup("house");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "512 Invalid v3 address \"house\"\r\n");

 done:
  tor_free(args);
  tor_free(cp1);
  buf_free(TO_CONN(&conn)->outbuf);
  tor_free(conn.current_cmd);
  hs_client_free_all();
}

/** Test some error cases of ONION_CLIENT_AUTH_ADD */
static void
test_hs_control_bad_onion_client_auth_add(void *arg)
{
  (void) arg;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  int retval;
  control_connection_t conn;
  char *cp1 = NULL;
  size_t sz;
  char *args = NULL;

  hs_init();

  { /* Setup the control conn */
    memset(&conn, 0, sizeof(control_connection_t));
    TO_CONN(&conn)->outbuf = buf_new();
    conn.current_cmd = tor_strdup("ONION_CLIENT_AUTH_ADD");
  }

  digest256map_t *client_auths = get_hs_client_auths_map();
  tt_assert(!client_auths);

  /* Register first service */
  args = tor_strdup(
                "badaddr x25519:iJ1tjKCrMAbiFT2bVrCjhbfMDnE1fpaRbIS5ZHKUvEQ=");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check contents */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "512 Invalid v3 address \"badaddr\"\r\n");

  tor_free(cp1);
  tor_free(args);

  /* Register second service (even with an unrecognized argument) */
  args = tor_strdup("jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd "
                    "love:eIIdIGoSZwI2Q/lSzpf92akGki5I+PZIDz37MA5BhlA=");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check contents */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "552 Unrecognized key type \"love\"\r\n");

  tor_free(cp1);
  tor_free(args);

  /* Register second service (even with an unrecognized argument) */
  args = tor_strdup("jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd "
                    "x25519:QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUEK");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check contents */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "512 Failed to decode x25519 private key\r\n");

  tor_free(cp1);
  tor_free(args);

  /* Register with an all zero client key */
  args = tor_strdup("jt4grrjwzyz3pjkylwfau5xnjaj23vxmhskqaeyfhrfylelw4hvxcuyd "
                    "x25519:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=");
  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check contents */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "553 Invalid private key \"AAAAAAAAAAAAAAAAAAAA"
                        "AAAAAAAAAAAAAAAAAAAAAAA=\"\r\n");

  client_auths = get_hs_client_auths_map();
  tt_assert(!client_auths);

 done:
  tor_free(args);
  tor_free(cp1);
  buf_free(TO_CONN(&conn)->outbuf);
  tor_free(conn.current_cmd);
  hs_client_free_all();
}

/** Test that we can correctly add permanent client auth credentials using the
 *  control port. */
static void
test_hs_control_store_permanent_creds(void *arg)
{
  (void) arg;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  int retval;
  ed25519_public_key_t service_identity_pk_2fv;
  control_connection_t conn;
  char *args = NULL;
  char *cp1 = NULL;
  char *creds_file_str = NULL;
  char *creds_fname = NULL;

  size_t sz;

  hs_init();

  { /* Setup the control conn */
    memset(&conn, 0, sizeof(control_connection_t));
    TO_CONN(&conn)->outbuf = buf_new();
    conn.current_cmd = tor_strdup("ONION_CLIENT_AUTH_ADD");
  }

  { /* Setup the services */
    retval = hs_parse_address(
                 "2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd",
                 &service_identity_pk_2fv,
                 NULL, NULL);
    tt_int_op(retval, OP_EQ, 0);
  }

  digest256map_t *client_auths = get_hs_client_auths_map();
  tt_assert(!client_auths);

  /* Try registering first service with no ClientOnionAuthDir set */
  args = tor_strdup("2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd "
                    "x25519:iJ1tjKCrMAbiFT2bVrCjhbfMDnE1fpaRbIS5ZHKUvEQ= "
                    "Flags=Permanent");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check control port response. This one should fail. */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "553 Unable to store creds for "
        "\"2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd\"\r\n");

  { /* Setup ClientOnionAuthDir */
    int ret;
    char *perm_creds_dir = tor_strdup(get_fname("permanent_credentials"));
    get_options_mutable()->ClientOnionAuthDir = perm_creds_dir;

    #ifdef _WIN32
      ret = mkdir(perm_creds_dir);
    #else
      ret = mkdir(perm_creds_dir, 0700);
    #endif
    tt_int_op(ret, OP_EQ, 0);
  }

  tor_free(args);
  tor_free(cp1);

  /* Try the control port command again. This time it should work! */
  args = tor_strdup("2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd "
                    "x25519:iJ1tjKCrMAbiFT2bVrCjhbfMDnE1fpaRbIS5ZHKUvEQ= "
                    "Flags=Permanent");
  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check control port response */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");

  /* Check file contents! */
  creds_fname = tor_strdup(get_fname("permanent_credentials/"
     "2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd.auth_private"));
  creds_file_str = read_file_to_str(creds_fname, RFTS_BIN, NULL);

  tt_assert(creds_file_str);
  tt_str_op(creds_file_str, OP_EQ,
         "2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd:descriptor:"
         /* base32 representation of the base64 iJ1t... key above */
         "x25519:rcow3dfavmyanyqvhwnvnmfdqw34ydtrgv7jnelmqs4wi4uuxrca");

  tor_free(args);
  tor_free(cp1);

  /* Overwrite the credentials and check that they got overwrited. */
  args = tor_strdup("2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd "
                    "x25519:UDRvZLvcJo0QRLvDfkpgbtsqbkhIUQZyeo2FNBrgS18= "
                    "Flags=Permanent");
  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check control port response: we replaced! */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "251 Client for onion existed and replaced\r\n");

  tor_free(creds_file_str);

  /* Check creds file contents again. See that the key got updated */
  creds_file_str = read_file_to_str(creds_fname, RFTS_BIN, NULL);
  tt_assert(creds_file_str);
  tt_str_op(creds_file_str, OP_EQ,
         "2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd:descriptor:"
         /* base32 representation of the base64 UDRv... key above */
         "x25519:ka2g6zf33qti2ecexpbx4stan3nsu3sijbiqm4t2rwctigxajnpq");

  /* Now for our next act!!! Actually get the HS client subsystem to parse the
   * whole directory and make sure that it extracted the right credential! */
  hs_config_client_authorization(get_options(), 0);

  client_auths = get_hs_client_auths_map();
  tt_assert(client_auths);
  tt_uint_op(digest256map_size(client_auths), OP_EQ, 1);

  hs_client_service_authorization_t *client_2fv =
    digest256map_get(client_auths, service_identity_pk_2fv.pubkey);
  tt_assert(client_2fv);
  tt_int_op(client_2fv->flags, OP_EQ, CLIENT_AUTH_FLAG_IS_PERMANENT);
  tt_str_op(hex_str((char*)client_2fv->enc_seckey.secret_key, 32), OP_EQ,
           "50346F64BBDC268D1044BBC37E4A606EDB2A6E48485106727A8D85341AE04B5F");

  /* And now for the final act! Use the REMOVE control port command to remove
     the credential, and ensure that the file has also been removed! */
  tor_free(conn.current_cmd);
  tor_free(cp1);
  tor_free(args);

  /* Ensure that the creds file exists */
  tt_int_op(file_status(creds_fname), OP_EQ, FN_FILE);

  /* Do the REMOVE */
  conn.current_cmd = tor_strdup("ONION_CLIENT_AUTH_REMOVE");
  args =tor_strdup("2fvhjskjet3n5syd6yfg5lhvwcs62bojmthr35ko5bllr3iqdb4ctdyd");
  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "250 OK\r\n");

  /* Ensure that the file has been removed and the map is empty */
  tt_int_op(file_status(creds_fname), OP_EQ, FN_NOENT);
  tt_uint_op(digest256map_size(client_auths), OP_EQ, 0);

 done:
  tor_free(get_options_mutable()->ClientOnionAuthDir);
  tor_free(args);
  tor_free(cp1);
  buf_free(TO_CONN(&conn)->outbuf);
  tor_free(conn.current_cmd);
  tor_free(creds_fname);
  tor_free(creds_file_str);
  hs_client_free_all();
}

/** Test that ADD_ONION properly handles an attacker passing it a bad private
 *  key. */
static void
test_hs_control_add_onion_with_bad_pubkey(void *arg)
{
  (void) arg;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  int retval;
  control_connection_t conn;
  char *args = NULL;
  char *cp1 = NULL;
  size_t sz;

  hs_init();

  { /* Setup the control conn */
    memset(&conn, 0, sizeof(control_connection_t));
    TO_CONN(&conn)->outbuf = buf_new();
    conn.current_cmd = tor_strdup("ADD_ONION");
  }

  args = tor_strdup("ED25519-V3:AAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA "
                    "Port=9735,127.0.0.1 Flags=DiscardPK");

  retval = handle_control_command(&conn, (uint32_t) strlen(args), args);
  tt_int_op(retval, OP_EQ, 0);

  /* Check control port response */
  cp1 = buf_get_contents(TO_CONN(&conn)->outbuf, &sz);
  tt_str_op(cp1, OP_EQ, "551 Failed to generate onion address\r\n");

 done:
  tor_free(args);
  tor_free(cp1);
  buf_free(TO_CONN(&conn)->outbuf);
  tor_free(conn.current_cmd);
}

struct testcase_t hs_control_tests[] = {
  { "hs_desc_event", test_hs_desc_event, TT_FORK,
    NULL, NULL },
  { "hs_control_good_onion_client_auth_add",
    test_hs_control_good_onion_client_auth_add, TT_FORK,
    NULL, NULL },
  { "hs_control_bad_onion_client_auth_add",
    test_hs_control_bad_onion_client_auth_add, TT_FORK,
    NULL, NULL },
  { "hs_control_store_permanent_creds",
    test_hs_control_store_permanent_creds, TT_FORK, NULL, NULL },
  { "hs_control_add_onion_with_bad_pubkey",
    test_hs_control_add_onion_with_bad_pubkey, TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};
