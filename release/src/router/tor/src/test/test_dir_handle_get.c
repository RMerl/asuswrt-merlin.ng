/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define RENDCOMMON_PRIVATE
#define GEOIP_PRIVATE
#define CONNECTION_PRIVATE
#define CONFIG_PRIVATE
#define RENDCACHE_PRIVATE
#define DIRCACHE_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "feature/dircache/consdiffmgr.h"
#include "feature/dircommon/directory.h"
#include "feature/dircache/dircache.h"
#include "test/test.h"
#include "lib/compress/compress.h"
#include "feature/rend/rendcommon.h"
#include "feature/rend/rendcache.h"
#include "feature/relay/relay_config.h"
#include "feature/relay/router.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/routerlist.h"
#include "test/rend_test_helpers.h"
#include "feature/nodelist/microdesc.h"
#include "test/test_helpers.h"
#include "feature/nodelist/nodelist.h"
#include "feature/client/entrynodes.h"
#include "feature/dirparse/authcert_parse.h"
#include "feature/dirparse/sigcommon.h"
#include "feature/nodelist/networkstatus.h"
#include "core/proto/proto_http.h"
#include "lib/geoip/geoip.h"
#include "feature/stats/geoip_stats.h"
#include "feature/dircache/dirserv.h"
#include "feature/dirauth/dirvote.h"
#include "test/log_test_helpers.h"
#include "feature/dirauth/voting_schedule.h"

#include "feature/dircommon/dir_connection_st.h"
#include "feature/dirclient/dir_server_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/rend/rend_encoded_v2_service_descriptor_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist_st.h"

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#else
#include <dirent.h>
#endif /* defined(_WIN32) */

#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
DISABLE_GCC_WARNING("-Woverlength-strings")
/* We allow huge string constants in the unit tests, but not in the code
 * at large. */
#endif
#include "vote_descriptors.inc"
#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
ENABLE_GCC_WARNING("-Woverlength-strings")
#endif

#define NOT_FOUND "HTTP/1.0 404 Not found\r\n\r\n"
#define BAD_REQUEST "HTTP/1.0 400 Bad request\r\n\r\n"
#define SERVER_BUSY "HTTP/1.0 503 Directory busy, try again later\r\n\r\n"
#define TOO_OLD "HTTP/1.0 404 Consensus is too old\r\n\r\n"
#define NOT_ENOUGH_CONSENSUS_SIGNATURES "HTTP/1.0 404 " \
  "Consensus not signed by sufficient number of requested authorities\r\n\r\n"

#define consdiffmgr_add_consensus consdiffmgr_add_consensus_nulterm

static int
mock_ignore_signature_token(const char *digest,
                            ssize_t digest_len,
                            struct directory_token_t *tok,
                            crypto_pk_t *pkey,
                            int flags,
                            const char *doctype)
{
  (void)digest;
  (void)digest_len;
  (void)tok;
  (void)pkey;
  (void)flags;
  (void)doctype;
  return 0;
}

static dir_connection_t *
new_dir_conn(void)
{
  dir_connection_t *conn = dir_connection_new(AF_INET);
  tor_addr_from_ipv4h(&conn->base_.addr, 0x7f000001);
  TO_CONN(conn)->address = tor_strdup("127.0.0.1");
  return conn;
}

static void
test_dir_handle_get_bad_request(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(directory_handle_command_get(conn, "", NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(header, OP_EQ, BAD_REQUEST);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_v1_command_not_found(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  // no frontpage configured
  tt_ptr_op(relay_get_dirportfrontpage(), OP_EQ, NULL);

  /* V1 path */
  tt_int_op(directory_handle_command_get(conn, GET("/tor/"), NULL, 0),
            OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static const char*
mock_get_dirportfrontpage(void)
{
  return "HELLO FROM FRONTPAGE";
}

static void
test_dir_handle_get_v1_command(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0, body_len = 0;
  const char *exp_body = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  MOCK(relay_get_dirportfrontpage, mock_get_dirportfrontpage);

  exp_body = relay_get_dirportfrontpage();
  body_len = strlen(exp_body);

  conn = new_dir_conn();
  tt_int_op(directory_handle_command_get(conn, GET("/tor/"), NULL, 0),
            OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, body_len+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/html\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 20\r\n"));

  tt_int_op(body_used, OP_EQ, strlen(body));
  tt_str_op(body, OP_EQ, exp_body);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(relay_get_dirportfrontpage);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
}

static void
test_dir_handle_get_not_found(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  /* Unrecognized path */
  tt_int_op(directory_handle_command_get(conn, GET("/anything"), NULL, 0),
            OP_EQ, 0);
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_robots_txt(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  tt_int_op(directory_handle_command_get(conn, GET("/tor/robots.txt"),
                                         NULL, 0), OP_EQ, 0);
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, 29, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 28\r\n"));

  tt_int_op(body_used, OP_EQ, strlen(body));
  tt_str_op(body, OP_EQ, "User-agent: *\r\nDisallow: /\r\n");

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
}

#define RENDEZVOUS2_GET(descid) GET("/tor/rendezvous2/" descid)
static void
test_dir_handle_get_rendezvous2_not_found_if_not_encrypted(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  // connection is not encrypted
  tt_assert(!connection_dir_is_encrypted(conn));

  tt_int_op(directory_handle_command_get(conn, RENDEZVOUS2_GET(), NULL, 0),
            OP_EQ, 0);
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_rendezvous2_on_encrypted_conn_with_invalid_desc_id(
  void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  conn = new_dir_conn();

  // connection is encrypted
  TO_CONN(conn)->linked = 1;
  tt_assert(connection_dir_is_encrypted(conn));

  tt_int_op(directory_handle_command_get(conn,
            RENDEZVOUS2_GET("invalid-desc-id"), NULL, 0), OP_EQ, 0);
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(header, OP_EQ, BAD_REQUEST);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_rendezvous2_on_encrypted_conn_not_well_formed(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  conn = new_dir_conn();

  // connection is encrypted
  TO_CONN(conn)->linked = 1;
  tt_assert(connection_dir_is_encrypted(conn));

  //TODO: this can't be reached because rend_valid_descriptor_id() prevents
  //this case to happen. This test is the same as
  //test_dir_handle_get_rendezvous2_on_encrypted_conn_with_invalid_desc_id We
  //should refactor to remove the case from the switch.

  const char *req = RENDEZVOUS2_GET("1bababababababababababababababab");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(header, OP_EQ, BAD_REQUEST);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_rendezvous2_not_found(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  conn = new_dir_conn();

  rend_cache_init();

  // connection is encrypted
  TO_CONN(conn)->linked = 1;
  tt_assert(connection_dir_is_encrypted(conn));

  const char *req = RENDEZVOUS2_GET("3xqunszqnaolrrfmtzgaki7mxelgvkje");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    rend_cache_free_all();
}

static const routerinfo_t * dhg_tests_router_get_my_routerinfo(void);
ATTR_UNUSED static int dhg_tests_router_get_my_routerinfo_called = 0;

static routerinfo_t *mock_routerinfo;

static const routerinfo_t *
dhg_tests_router_get_my_routerinfo(void)
{
  if (!mock_routerinfo) {
    mock_routerinfo = tor_malloc_zero(sizeof(routerinfo_t));
  }

  return mock_routerinfo;
}

static void
test_dir_handle_get_rendezvous2_on_encrypted_conn_success(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  char buff[30];
  char req[70];
  rend_encoded_v2_service_descriptor_t *desc_holder = NULL;
  char *service_id = NULL;
  char desc_id_base32[REND_DESC_ID_V2_LEN_BASE32 + 1];
  size_t body_len = 0;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  MOCK(router_get_my_routerinfo,
       dhg_tests_router_get_my_routerinfo);

  rend_cache_init();

  /* create a valid rend service descriptor */
  #define RECENT_TIME -10
  generate_desc(RECENT_TIME, &desc_holder, &service_id, 3);

  tt_int_op(rend_cache_store_v2_desc_as_dir(desc_holder->desc_str),
            OP_EQ, 0);

  base32_encode(desc_id_base32, sizeof(desc_id_base32), desc_holder->desc_id,
                DIGEST_LEN);

  conn = new_dir_conn();

  // connection is encrypted
  TO_CONN(conn)->linked = 1;
  tt_assert(connection_dir_is_encrypted(conn));

  tor_snprintf(req, sizeof(req), RENDEZVOUS2_GET("%s"), desc_id_base32);

  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  body_len = strlen(desc_holder->desc_str);
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, body_len+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Pragma: no-cache\r\n"));
  tor_snprintf(buff, sizeof(buff), "Content-Length: %ld\r\n", (long) body_len);
  tt_assert(strstr(header, buff));

  tt_int_op(body_used, OP_EQ, strlen(body));
  tt_str_op(body, OP_EQ, desc_holder->desc_str);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(router_get_my_routerinfo);

    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    rend_encoded_v2_service_descriptor_free(desc_holder);
    tor_free(service_id);
    rend_cache_free_all();
}

#define MICRODESC_GET(digest) GET("/tor/micro/d/" digest)
static void
test_dir_handle_get_micro_d_not_found(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  #define B64_256_1 "8/Pz8/u7vz8/Pz+7vz8/Pz+7u/Pz8/P7u/Pz8/P7u78"
  #define B64_256_2 "zMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMw"
  conn = new_dir_conn();

  const char *req = MICRODESC_GET(B64_256_1 "-" B64_256_2);
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);

    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static or_options_t *mock_options = NULL;
static void
init_mock_options(void)
{
  mock_options = options_new();
  mock_options->TestingTorNetwork = 1;
  mock_options->DataDirectory = tor_strdup(get_fname_rnd("datadir_tmp"));
  mock_options->CacheDirectory = tor_strdup(mock_options->DataDirectory);
  check_private_dir(mock_options->DataDirectory, CPD_CREATE, NULL);
}

static const or_options_t *
mock_get_options(void)
{
  tor_assert(mock_options);
  return mock_options;
}

static const char microdesc[] =
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMjlHH/daN43cSVRaHBwgUfnszzAhg98EvivJ9Qxfv51mvQUxPjQ07es\n"
  "gV/3n8fyh3Kqr/ehi9jxkdgSRfSnmF7giaHL1SLZ29kA7KtST+pBvmTpDtHa3ykX\n"
  "Xorc7hJvIyTZoc1HU+5XSynj3gsBE5IGK1ZRzrNS688LnuZMVp1tAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key QlrOXAa8j3LD31LESsPm/lIKFBwevk2oXdqJcd9SEUc=\n";

static void
test_dir_handle_get_micro_d(void *data)
{
  dir_connection_t *conn = NULL;
  microdesc_cache_t *mc = NULL ;
  smartlist_t *list = NULL;
  char digest[DIGEST256_LEN];
  char digest_base64[128];
  char path[80];
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  (void) data;

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* SETUP */
  init_mock_options();

  /* Add microdesc to cache */
  crypto_digest256(digest, microdesc, strlen(microdesc), DIGEST_SHA256);
  base64_encode_nopad(digest_base64, sizeof(digest_base64),
                      (uint8_t *) digest, DIGEST256_LEN);

  mc = get_microdesc_cache();
  list = microdescs_add_to_cache(mc, microdesc, NULL, SAVED_NOWHERE, 0,
                                  time(NULL), NULL);
  tt_int_op(1, OP_EQ, smartlist_len(list));

  /* Make the request */
  conn = new_dir_conn();

  tor_snprintf(path, sizeof(path), MICRODESC_GET("%s"), digest_base64);
  tt_int_op(directory_handle_command_get(conn, path, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(microdesc)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));

  tt_int_op(body_used, OP_EQ, strlen(body));
  tt_str_op(body, OP_EQ, microdesc);

  done:
    UNMOCK(get_options);
    UNMOCK(connection_write_to_buf_impl_);

    or_options_free(mock_options); mock_options = NULL;
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    smartlist_free(list);
    microdesc_free_all();
}

static void
test_dir_handle_get_micro_d_server_busy(void *data)
{
  dir_connection_t *conn = NULL;
  microdesc_cache_t *mc = NULL ;
  smartlist_t *list = NULL;
  char digest[DIGEST256_LEN];
  char digest_base64[128];
  char path[80];
  char *header = NULL;
  (void) data;

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* SETUP */
  init_mock_options();

  /* Add microdesc to cache */
  crypto_digest256(digest, microdesc, strlen(microdesc), DIGEST_SHA256);
  base64_encode_nopad(digest_base64, sizeof(digest_base64),
                      (uint8_t *) digest, DIGEST256_LEN);

  mc = get_microdesc_cache();
  list = microdescs_add_to_cache(mc, microdesc, NULL, SAVED_NOWHERE, 0,
                                  time(NULL), NULL);
  tt_int_op(1, OP_EQ, smartlist_len(list));

  //Make it busy
  mock_options->CountPrivateBandwidth = 1;

  /* Make the request */
  conn = new_dir_conn();

  tor_snprintf(path, sizeof(path), MICRODESC_GET("%s"), digest_base64);
  tt_int_op(directory_handle_command_get(conn, path, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(SERVER_BUSY, OP_EQ, header);

  done:
    UNMOCK(get_options);
    UNMOCK(connection_write_to_buf_impl_);

    or_options_free(mock_options); mock_options = NULL;
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    smartlist_free(list);
    microdesc_free_all();
}

#define BRIDGES_PATH "/tor/networkstatus-bridges"
static void
test_dir_handle_get_networkstatus_bridges_not_found_without_auth(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* SETUP */
  init_mock_options();
  mock_options->BridgeAuthoritativeDir = 1;
  mock_options->BridgePassword_AuthDigest_ = tor_strdup("digest");

  conn = new_dir_conn();
  TO_CONN(conn)->linked = 1;

  const char *req = GET(BRIDGES_PATH);
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(get_options);
    UNMOCK(connection_write_to_buf_impl_);
    or_options_free(mock_options); mock_options = NULL;
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_networkstatus_bridges(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* SETUP */
  init_mock_options();
  mock_options->BridgeAuthoritativeDir = 1;
  mock_options->BridgePassword_AuthDigest_ = tor_malloc(DIGEST256_LEN);
  crypto_digest256(mock_options->BridgePassword_AuthDigest_,
                     "abcdefghijklm12345", 18, DIGEST_SHA256);

  conn = new_dir_conn();
  TO_CONN(conn)->linked = 1;

  const char *req = "GET " BRIDGES_PATH " HTTP/1.0\r\n"
                    "Authorization: Basic abcdefghijklm12345\r\n\r\n";
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 0\r\n"));

  done:
    UNMOCK(get_options);
    UNMOCK(connection_write_to_buf_impl_);
    or_options_free(mock_options); mock_options = NULL;
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_networkstatus_bridges_not_found_wrong_auth(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* SETUP */
  init_mock_options();
  mock_options->BridgeAuthoritativeDir = 1;
  mock_options->BridgePassword_AuthDigest_ = tor_malloc(DIGEST256_LEN);
  crypto_digest256(mock_options->BridgePassword_AuthDigest_,
                     "abcdefghijklm12345", 18, DIGEST_SHA256);

  conn = new_dir_conn();
  TO_CONN(conn)->linked = 1;

  const char *req = "GET " BRIDGES_PATH " HTTP/1.0\r\n"
                           "Authorization: Basic NOTSAMEDIGEST\r\n\r\n";
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(get_options);
    UNMOCK(connection_write_to_buf_impl_);
    or_options_free(mock_options); mock_options = NULL;
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

#define SERVER_DESC_GET(id) GET("/tor/server/" id)
static void
test_dir_handle_get_server_descriptors_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = SERVER_DESC_GET("invalid");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_str_op(NOT_FOUND, OP_EQ, header);
  tt_ptr_op(conn->spool, OP_EQ, NULL);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    or_options_free(mock_options); mock_options = NULL;
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_server_descriptors_all(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  (void) data;

  /* Setup fake routerlist. */
  helper_setup_fake_routerlist();

  //TODO: change to router_get_my_extrainfo when testing "extra" path
  MOCK(router_get_my_routerinfo,
       dhg_tests_router_get_my_routerinfo);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  // We are one of the routers
  routerlist_t *our_routerlist = router_get_routerlist();
  tt_int_op(smartlist_len(our_routerlist->routers), OP_GE, 1);
  mock_routerinfo = smartlist_get(our_routerlist->routers, 0);
  set_server_identity_key(mock_routerinfo->identity_pkey);
  mock_routerinfo->cache_info.published_on = time(NULL);

  /* Treat "all" requests as if they were unencrypted */
  mock_routerinfo->cache_info.send_unencrypted = 1;

  conn = new_dir_conn();

  const char *req = SERVER_DESC_GET("all");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  //TODO: Is this a BUG?
  //It requires strlen(signed_descriptor_len)+1 as body_len but returns a body
  //which is smaller than that by annotation_len bytes
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used,
                      1024*1024, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));

  //TODO: Is this a BUG?
  //This is what should be expected: tt_int_op(body_used, OP_EQ, strlen(body));
  tt_int_op(body_used, OP_EQ,
            mock_routerinfo->cache_info.signed_descriptor_len);

  tt_str_op(body, OP_EQ, mock_routerinfo->cache_info.signed_descriptor_body +
                         mock_routerinfo->cache_info.annotations_len);
  tt_ptr_op(conn->spool, OP_EQ, NULL);

  done:
    UNMOCK(router_get_my_routerinfo);
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);

    routerlist_free_all();
    nodelist_free_all();
    entry_guards_free_all();
}

static char
TEST_DESCRIPTOR[] =
"@uploaded-at 2014-06-08 19:20:11\n"
"@source \"127.0.0.1\"\n"
"router test000a 127.0.0.1 5000 0 7000\n"
"platform Tor 0.2.5.3-alpha-dev on Linux\n"
"protocols Link 1 2 Circuit 1\n"
"published 2014-06-08 19:20:11\n"
"fingerprint C7E7 CCB8 179F 8CC3 7F5C 8A04 2B3A 180B 934B 14BA\n"
"uptime 0\n"
"bandwidth 1073741824 1073741824 0\n"
"extra-info-digest 67A152A4C7686FB07664F872620635F194D76D95\n"
"caches-extra-info\n"
"onion-key\n"
"-----BEGIN RSA PUBLIC KEY-----\n"
"MIGJAoGBAOuBUIEBARMkkka/TGyaQNgUEDLP0KG7sy6KNQTNOlZHUresPr/vlVjo\n"
"HPpLMfu9M2z18c51YX/muWwY9x4MyQooD56wI4+AqXQcJRwQfQlPn3Ay82uZViA9\n"
"DpBajRieLlKKkl145KjArpD7F5BVsqccvjErgFYXvhhjSrx7BVLnAgMBAAE=\n"
"-----END RSA PUBLIC KEY-----\n"
"signing-key\n"
"-----BEGIN RSA PUBLIC KEY-----\n"
"MIGJAoGBAN6NLnSxWQnFXxqZi5D3b0BMgV6y9NJLGjYQVP+eWtPZWgqyv4zeYsqv\n"
"O9y6c5lvxyUxmNHfoAbe/s8f2Vf3/YaC17asAVSln4ktrr3e9iY74a9RMWHv1Gzk\n"
"3042nMcqj3PEhRN0PoLkcOZNjjmNbaqki6qy9bWWZDNTdo+uI44dAgMBAAE=\n"
"-----END RSA PUBLIC KEY-----\n"
"hidden-service-dir\n"
"contact auth0@test.test\n"
"ntor-onion-key pK4bs08ERYN591jj7ca17Rn9Q02TIEfhnjR6hSq+fhU=\n"
"reject *:*\n"
"router-signature\n"
"-----BEGIN SIGNATURE-----\n"
"rx88DuM3Y7tODlHNDDEVzKpwh3csaG1or+T4l2Xs1oq3iHHyPEtB6QTLYrC60trG\n"
"aAPsj3DEowGfjga1b248g2dtic8Ab+0exfjMm1RHXfDam5TXXZU3A0wMyoHjqHuf\n"
"eChGPgFNUvEc+5YtD27qEDcUjcinYztTs7/dzxBT4PE=\n"
"-----END SIGNATURE-----\n";

static void
test_dir_handle_get_server_descriptors_authority(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  crypto_pk_t *identity_pkey = pk_generate(0);
  (void) data;

  MOCK(router_get_my_routerinfo,
       dhg_tests_router_get_my_routerinfo);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* init mock */
  router_get_my_routerinfo();
  crypto_pk_get_digest(identity_pkey,
                       mock_routerinfo->cache_info.identity_digest);

  // the digest is mine (the channel is unnecrypted, so we must allow sending)
  set_server_identity_key(identity_pkey);
  mock_routerinfo->cache_info.send_unencrypted = 1;

  /* Setup descriptor */
  long annotation_len = strstr(TEST_DESCRIPTOR, "router ") - TEST_DESCRIPTOR;
  mock_routerinfo->cache_info.signed_descriptor_body =
    tor_strdup(TEST_DESCRIPTOR);
  mock_routerinfo->cache_info.signed_descriptor_len =
    strlen(TEST_DESCRIPTOR) - annotation_len;
  mock_routerinfo->cache_info.annotations_len = annotation_len;
  mock_routerinfo->cache_info.published_on = time(NULL);

  conn = new_dir_conn();

  const char *req = SERVER_DESC_GET("authority");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  //TODO: Is this a BUG?
  //It requires strlen(TEST_DESCRIPTOR)+1 as body_len but returns a body which
  //is smaller than that by annotation_len bytes
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(TEST_DESCRIPTOR)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));

  tt_int_op(body_used, OP_EQ, strlen(body));

  tt_str_op(body, OP_EQ, TEST_DESCRIPTOR + annotation_len);
  tt_ptr_op(conn->spool, OP_EQ, NULL);

  done:
    UNMOCK(router_get_my_routerinfo);
    UNMOCK(connection_write_to_buf_impl_);
    tor_free(mock_routerinfo->cache_info.signed_descriptor_body);
    tor_free(mock_routerinfo);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    crypto_pk_free(identity_pkey);
}

static void
test_dir_handle_get_server_descriptors_fp(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  crypto_pk_t *identity_pkey = pk_generate(0);
  (void) data;

  MOCK(router_get_my_routerinfo,
       dhg_tests_router_get_my_routerinfo);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* init mock */
  router_get_my_routerinfo();
  crypto_pk_get_digest(identity_pkey,
                       mock_routerinfo->cache_info.identity_digest);

  // the digest is mine (the channel is unnecrypted, so we must allow sending)
  set_server_identity_key(identity_pkey);
  mock_routerinfo->cache_info.send_unencrypted = 1;

  /* Setup descriptor */
  long annotation_len = strstr(TEST_DESCRIPTOR, "router ") - TEST_DESCRIPTOR;
  mock_routerinfo->cache_info.signed_descriptor_body =
    tor_strdup(TEST_DESCRIPTOR);
  mock_routerinfo->cache_info.signed_descriptor_len =
    strlen(TEST_DESCRIPTOR) - annotation_len;
  mock_routerinfo->cache_info.annotations_len = annotation_len;
  mock_routerinfo->cache_info.published_on = time(NULL);

  conn = new_dir_conn();

  #define HEX1 "Fe0daff89127389bc67558691231234551193EEE"
  #define HEX2 "Deadbeef99999991111119999911111111f00ba4"
  const char *hex_digest = hex_str(mock_routerinfo->cache_info.identity_digest,
                                   DIGEST_LEN);

  char req[155];
  tor_snprintf(req, sizeof(req), SERVER_DESC_GET("fp/%s+" HEX1 "+" HEX2),
               hex_digest);
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  //TODO: Is this a BUG?
  //It requires strlen(TEST_DESCRIPTOR)+1 as body_len but returns a body which
  //is smaller than that by annotation_len bytes
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(TEST_DESCRIPTOR)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));

  tt_int_op(body_used, OP_EQ, strlen(body));

  tt_str_op(body, OP_EQ, TEST_DESCRIPTOR + annotation_len);
  tt_ptr_op(conn->spool, OP_EQ, NULL);

  done:
    UNMOCK(router_get_my_routerinfo);
    UNMOCK(connection_write_to_buf_impl_);
    tor_free(mock_routerinfo->cache_info.signed_descriptor_body);
    tor_free(mock_routerinfo);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    crypto_pk_free(identity_pkey);
}

#define HEX1 "Fe0daff89127389bc67558691231234551193EEE"
#define HEX2 "Deadbeef99999991111119999911111111f00ba4"

static void
test_dir_handle_get_server_descriptors_d(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  crypto_pk_t *identity_pkey = pk_generate(0);
  (void) data;

  /* Setup fake routerlist. */
  helper_setup_fake_routerlist();

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* Get one router's signed_descriptor_digest */
  routerlist_t *our_routerlist = router_get_routerlist();
  tt_int_op(smartlist_len(our_routerlist->routers), OP_GE, 1);
  routerinfo_t *router = smartlist_get(our_routerlist->routers, 0);
  const char *hex_digest = hex_str(router->cache_info.signed_descriptor_digest,
                                   DIGEST_LEN);

  conn = new_dir_conn();

  char req_header[155]; /* XXX Why 155? What kind of number is that?? */
  tor_snprintf(req_header, sizeof(req_header),
               SERVER_DESC_GET("d/%s+" HEX1 "+" HEX2), hex_digest);
  tt_int_op(directory_handle_command_get(conn, req_header, NULL, 0), OP_EQ, 0);

  //TODO: Is this a BUG?
  //It requires strlen(signed_descriptor_len)+1 as body_len but returns a body
  //which is smaller than that by annotation_len bytes
  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used,
                      router->cache_info.signed_descriptor_len+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));

  //TODO: Is this a BUG?
  //This is what should be expected:
  //tt_int_op(body_used, OP_EQ, strlen(body));
  tt_int_op(body_used, OP_EQ, router->cache_info.signed_descriptor_len);

  tt_str_op(body, OP_EQ, router->cache_info.signed_descriptor_body +
                         router->cache_info.annotations_len);
  tt_ptr_op(conn->spool, OP_EQ, NULL);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    tor_free(mock_routerinfo);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    crypto_pk_free(identity_pkey);

    routerlist_free_all();
    nodelist_free_all();
    entry_guards_free_all();
}

static void
test_dir_handle_get_server_descriptors_busy(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  crypto_pk_t *identity_pkey = pk_generate(0);
  (void) data;

  /* Setup fake routerlist. */
  helper_setup_fake_routerlist();

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  //Make it busy
  MOCK(get_options, mock_get_options);
  init_mock_options();
  mock_options->CountPrivateBandwidth = 1;

  /* Get one router's signed_descriptor_digest */
  routerlist_t *our_routerlist = router_get_routerlist();
  tt_int_op(smartlist_len(our_routerlist->routers), OP_GE, 1);
  routerinfo_t *router = smartlist_get(our_routerlist->routers, 0);
  const char *hex_digest = hex_str(router->cache_info.signed_descriptor_digest,
                                   DIGEST_LEN);

  conn = new_dir_conn();

  #define HEX1 "Fe0daff89127389bc67558691231234551193EEE"
  #define HEX2 "Deadbeef99999991111119999911111111f00ba4"
  char req_header[155]; /* XXX 155? Why 155? */
  tor_snprintf(req_header, sizeof(req_header),
               SERVER_DESC_GET("d/%s+" HEX1 "+" HEX2), hex_digest);
  tt_int_op(directory_handle_command_get(conn, req_header, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(SERVER_BUSY, OP_EQ, header);

  tt_ptr_op(conn->spool, OP_EQ, NULL);

  done:
    UNMOCK(get_options);
    UNMOCK(connection_write_to_buf_impl_);
    tor_free(mock_routerinfo);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    crypto_pk_free(identity_pkey);

    routerlist_free_all();
    nodelist_free_all();
    entry_guards_free_all();
}

static void
test_dir_handle_get_server_keys_bad_req(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(BAD_REQUEST, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_server_keys_all_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/all");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

#define TEST_CERTIFICATE AUTHORITY_CERT_3
#define TEST_SIGNING_KEY AUTHORITY_SIGNKEY_A_DIGEST

static const char TEST_CERT_IDENT_KEY[] =
  "D867ACF56A9D229B35C25F0090BC9867E906BE69";

static void
test_dir_handle_get_server_keys_all(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  const char digest[DIGEST_LEN] = "";

  dir_server_t *ds = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  clear_dir_servers();
  routerlist_free_all();

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);
  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/all");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(TEST_CERTIFICATE)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 1883\r\n"));

  tt_str_op(TEST_CERTIFICATE, OP_EQ, body);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);

    clear_dir_servers();
    routerlist_free_all();
}

static void
test_dir_handle_get_server_keys_authority_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/authority");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static authority_cert_t * mock_cert = NULL;

static authority_cert_t *
get_my_v3_authority_cert_m(void)
{
  tor_assert(mock_cert);
  return mock_cert;
}

static void
test_dir_handle_get_server_keys_authority(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  (void) data;

  mock_cert = authority_cert_parse_from_string(TEST_CERTIFICATE,
                                               strlen(TEST_CERTIFICATE),
                                               NULL);

  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/authority");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(TEST_CERTIFICATE)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 1883\r\n"));

  tt_str_op(TEST_CERTIFICATE, OP_EQ, body);

  done:
    UNMOCK(get_my_v3_authority_cert);
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    authority_cert_free(mock_cert); mock_cert = NULL;
}

static void
test_dir_handle_get_server_keys_fp_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/fp/somehex");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_server_keys_fp(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  dir_server_t *ds = NULL;
  const char digest[DIGEST_LEN] = "";
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  clear_dir_servers();
  routerlist_free_all();

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);

  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  conn = new_dir_conn();
  char req[71];
  tor_snprintf(req, sizeof(req),
                     GET("/tor/keys/fp/%s"), TEST_CERT_IDENT_KEY);
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(TEST_CERTIFICATE)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 1883\r\n"));

  tt_str_op(TEST_CERTIFICATE, OP_EQ, body);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    clear_dir_servers();
    routerlist_free_all();
}

static void
test_dir_handle_get_server_keys_sk_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/sk/somehex");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_server_keys_sk(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  (void) data;

  mock_cert = authority_cert_parse_from_string(TEST_CERTIFICATE,
                                               strlen(TEST_CERTIFICATE),
                                               NULL);
  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  clear_dir_servers();
  routerlist_free_all();

  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  conn = new_dir_conn();
  char req[71];
  tor_snprintf(req, sizeof(req),
               GET("/tor/keys/sk/%s"), TEST_SIGNING_KEY);
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(TEST_CERTIFICATE)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 1883\r\n"));

  tt_str_op(TEST_CERTIFICATE, OP_EQ, body);

  done:
    UNMOCK(get_my_v3_authority_cert);
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    authority_cert_free(mock_cert); mock_cert = NULL;
    tor_free(header);
    tor_free(body);
}

static void
test_dir_handle_get_server_keys_fpsk_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();

  const char *req = GET("/tor/keys/fp-sk/somehex");
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_server_keys_fpsk(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  dir_server_t *ds = NULL;
  const char digest[DIGEST_LEN] = "";
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  clear_dir_servers();
  routerlist_free_all();

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);
  dir_server_add(ds);

  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  conn = new_dir_conn();

  char req[115];
  tor_snprintf(req, sizeof(req),
               GET("/tor/keys/fp-sk/%s-%s"),
               TEST_CERT_IDENT_KEY, TEST_SIGNING_KEY);

  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(TEST_CERTIFICATE)+1, 0);

  tt_assert(header);
  tt_assert(body);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 1883\r\n"));

  tt_str_op(TEST_CERTIFICATE, OP_EQ, body);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);

    clear_dir_servers();
    routerlist_free_all();
}

static void
test_dir_handle_get_server_keys_busy(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  dir_server_t *ds = NULL;
  const char digest[DIGEST_LEN] = "";
  (void) data;

  clear_dir_servers();
  routerlist_free_all();

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);
  dir_server_add(ds);

  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* setup busy server */
  init_mock_options();
  mock_options->CountPrivateBandwidth = 1;

  conn = new_dir_conn();
  char req[71];
  tor_snprintf(req, sizeof(req), GET("/tor/keys/fp/%s"), TEST_CERT_IDENT_KEY);
  tt_int_op(directory_handle_command_get(conn, req, NULL, 0), OP_EQ, 0);

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(SERVER_BUSY, OP_EQ, header);

  done:
    UNMOCK(get_options);
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    or_options_free(mock_options); mock_options = NULL;

    clear_dir_servers();
    routerlist_free_all();
}

static networkstatus_t *mock_ns_val = NULL;
static networkstatus_t *
mock_ns_get_by_flavor(consensus_flavor_t f)
{
  (void)f;
  return mock_ns_val;
}

static void
test_dir_handle_get_status_vote_current_consensus_ns_not_enough_sigs(void* d)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *stats = NULL;
  (void) d;

  /* init mock */
  mock_ns_val = tor_malloc_zero(sizeof(networkstatus_t));
  mock_ns_val->flavor = FLAV_NS;
  mock_ns_val->type = NS_TYPE_CONSENSUS;
  mock_ns_val->voters = smartlist_new();
  mock_ns_val->valid_after = time(NULL) - 1800;
  mock_ns_val->valid_until = time(NULL) - 60;

  #define NETWORK_STATUS "some network status string"
  consdiffmgr_add_consensus(NETWORK_STATUS, mock_ns_val);

  /* init mock */
  init_mock_options();

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  MOCK(networkstatus_get_latest_consensus_by_flavor, mock_ns_get_by_flavor);

  /* start gathering stats */
  mock_options->DirReqStatistics = 1;
  geoip_dirreq_stats_init(time(NULL));

  conn = new_dir_conn();

  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/consensus-ns/" HEX1 "+" HEX2), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);

  tt_assert(header);
  tt_str_op(NOT_ENOUGH_CONSENSUS_SIGNATURES, OP_EQ, header);

  stats = geoip_format_dirreq_stats(time(NULL));
  tt_assert(stats);
  tt_assert(strstr(stats, "not-enough-sigs=8"));

  done:
    UNMOCK(networkstatus_get_latest_consensus_by_flavor);
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(get_options);

    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(stats);
    smartlist_free(mock_ns_val->voters);
    tor_free(mock_ns_val);
    or_options_free(mock_options); mock_options = NULL;
}

static void
test_dir_handle_get_status_vote_current_consensus_ns_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  char *stats = NULL;
  (void) data;

  init_mock_options();

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  /* start gathering stats */
  mock_options->DirReqStatistics = 1;
  geoip_dirreq_stats_init(time(NULL));

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/consensus-ns"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  stats = geoip_format_dirreq_stats(time(NULL));
  tt_assert(stats);
  tt_assert(strstr(stats, "not-found=8"));

  done:
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(get_options);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(stats);
    or_options_free(mock_options); mock_options = NULL;
}

static void
test_dir_handle_get_status_vote_current_consensus_too_old(void *data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void)data;

  mock_ns_val = tor_malloc_zero(sizeof(networkstatus_t));
  mock_ns_val->type = NS_TYPE_CONSENSUS;
  mock_ns_val->flavor = FLAV_MICRODESC;
  mock_ns_val->valid_after = time(NULL) - (24 * 60 * 60 + 1800);
  mock_ns_val->fresh_until = time(NULL) - (24 * 60 * 60 + 900);
  mock_ns_val->valid_until = time(NULL) - (24 * 60 * 60 + 20);

  #define NETWORK_STATUS "some network status string"
  consdiffmgr_add_consensus(NETWORK_STATUS, mock_ns_val);

  init_mock_options();

  MOCK(get_options, mock_get_options);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  MOCK(networkstatus_get_latest_consensus_by_flavor, mock_ns_get_by_flavor);

  conn = new_dir_conn();

  setup_capture_of_logs(LOG_WARN);

  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/consensus-microdesc"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(TOO_OLD, OP_EQ, header);

  expect_log_msg_containing("too old");

  tor_free(header);
  teardown_capture_of_logs();
  tor_free(mock_ns_val);

  mock_ns_val = tor_malloc_zero(sizeof(networkstatus_t));
  mock_ns_val->type = NS_TYPE_CONSENSUS;
  mock_ns_val->flavor = FLAV_NS;
  mock_ns_val->valid_after = time(NULL) - (24 * 60 * 60 + 1800);
  mock_ns_val->fresh_until = time(NULL) - (24 * 60 * 60 + 900);
  mock_ns_val->valid_until = time(NULL) - (24 * 60 * 60 + 20);

  #define NETWORK_STATUS "some network status string"
  consdiffmgr_add_consensus(NETWORK_STATUS, mock_ns_val);

  setup_capture_of_logs(LOG_WARN);

  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/consensus"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(TOO_OLD, OP_EQ, header);

  expect_no_log_entry();

  done:
    teardown_capture_of_logs();
    UNMOCK(networkstatus_get_latest_consensus_by_flavor);
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(get_options);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(mock_ns_val);
    or_options_free(mock_options); mock_options = NULL;
}

static int dhg_tests_geoip_get_country_by_addr(const tor_addr_t *addr);
ATTR_UNUSED static int dhg_tests_geoip_get_country_by_addr_called = 0;

int
dhg_tests_geoip_get_country_by_addr(const tor_addr_t *addr)
{
  (void)addr;
  dhg_tests_geoip_get_country_by_addr_called++;
  return 1;
}

static void
status_vote_current_consensus_ns_test(char **header, char **body,
                                      size_t *body_len)
{
  dir_connection_t *conn = NULL;

  #define NETWORK_STATUS "some network status string"
#if 0
  common_digests_t digests;
  uint8_t sha3[DIGEST256_LEN];
  memset(&digests, 0x60, sizeof(digests));
  memset(sha3, 0x06, sizeof(sha3));
  dirserv_set_cached_consensus_networkstatus(NETWORK_STATUS, "ns", &digests,
                                             sha3,
                                             time(NULL));
#endif /* 0 */
  networkstatus_t *ns = tor_malloc_zero(sizeof(networkstatus_t));
  ns->type = NS_TYPE_CONSENSUS;
  ns->flavor = FLAV_NS;
  ns->valid_after = time(NULL) - 1800;
  ns->fresh_until = time(NULL) - 900;
  ns->valid_until = time(NULL) - 60;
  consdiffmgr_add_consensus(NETWORK_STATUS, ns);
  networkstatus_vote_free(ns);

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  tt_assert(mock_options);
  mock_options->DirReqStatistics = 1;
  geoip_dirreq_stats_init(time(NULL));

  /* init geoip database */
  geoip_parse_entry("10,50,AB", AF_INET);
  tt_str_op("ab", OP_EQ, geoip_get_country_name(1));

  conn = new_dir_conn();

  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/consensus-ns"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, header, MAX_HEADERS_SIZE,
                      body, body_len, strlen(NETWORK_STATUS)+7, 0);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
}

static void
test_dir_handle_get_status_vote_current_consensus_ns(void* data)
{
  char *header = NULL;
  char *body = NULL, *comp_body = NULL;
  size_t body_used = 0, comp_body_used = 0;
  char *stats = NULL, *hist = NULL;
  (void) data;

  dirserv_free_all();
  clear_geoip_db();

  MOCK(geoip_get_country_by_addr,
       dhg_tests_geoip_get_country_by_addr);
  MOCK(get_options, mock_get_options);

  init_mock_options();

  status_vote_current_consensus_ns_test(&header, &comp_body, &comp_body_used);
  tt_assert(header);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Pragma: no-cache\r\n"));

  compress_method_t compression = detect_compression_method(comp_body,
                                                            comp_body_used);
  tt_int_op(ZLIB_METHOD, OP_EQ, compression);

  tor_uncompress(&body, &body_used, comp_body, comp_body_used,
                 compression, 0, LOG_PROTOCOL_WARN);

  tt_str_op(NETWORK_STATUS, OP_EQ, body);
  tt_int_op(strlen(NETWORK_STATUS), OP_EQ, body_used);

  stats = geoip_format_dirreq_stats(time(NULL));
  tt_assert(stats);

  tt_assert(strstr(stats, "ok=8"));
  tt_assert(strstr(stats, "dirreq-v3-ips ab=8"));
  tt_assert(strstr(stats, "dirreq-v3-reqs ab=8"));
  tt_assert(strstr(stats, "dirreq-v3-direct-dl"
                          " complete=0,timeout=0,running=4"));

  hist = geoip_get_request_history();
  tt_assert(hist);
  tt_str_op("ab=8", OP_EQ, hist);

  done:
    UNMOCK(geoip_get_country_by_addr);
    UNMOCK(get_options);
    tor_free(header);
    tor_free(comp_body);
    tor_free(body);
    tor_free(stats);
    tor_free(hist);
    or_options_free(mock_options); mock_options = NULL;

    dirserv_free_all();
    clear_geoip_db();
}

static void
test_dir_handle_get_status_vote_current_consensus_ns_busy(void* data)
{
  char *header = NULL;
  char *body = NULL;
  size_t body_used = 0;
  char *stats = NULL;
  (void) data;

  dirserv_free_all();
  clear_geoip_db();

  MOCK(get_options, mock_get_options);

  // Make it busy
  init_mock_options();
  mock_options->CountPrivateBandwidth = 1;

  status_vote_current_consensus_ns_test(&header, &body, &body_used);
  tt_assert(header);

  tt_str_op(SERVER_BUSY, OP_EQ, header);

  stats = geoip_format_dirreq_stats(time(NULL));
  tt_assert(stats);
  tt_assert(strstr(stats, "busy=8"));

  done:
    UNMOCK(get_options);
    tor_free(header);
    tor_free(body);
    or_options_free(mock_options); mock_options = NULL;

    tor_free(stats);
    dirserv_free_all();
    clear_geoip_db();
}

static void
test_dir_handle_get_status_vote_current_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/" HEX1), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

/* What vote do we ask for, to get the vote in vote_descriptors.inc ? */
#define VOTE_DIGEST "78400095d8e834d87135cfc46235c909f0e99911"

static void
status_vote_current_d_test(char **header, char **body, size_t *body_l)
{
  dir_connection_t *conn = NULL;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/d/" VOTE_DIGEST), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, header, MAX_HEADERS_SIZE,
                      body, body_l, strlen(VOTE_BODY_V3)+1, 0);
  tt_assert(header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
}

static void
status_vote_next_d_test(char **header, char **body, size_t *body_l)
{
  dir_connection_t *conn = NULL;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/d/" VOTE_DIGEST), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, header, MAX_HEADERS_SIZE,
                      body, body_l, strlen(VOTE_BODY_V3)+1, 0);
  tt_assert(header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
}

static void
test_dir_handle_get_status_vote_current_d_not_found(void* data)
{
  char *header = NULL;
  (void) data;

  status_vote_current_d_test(&header, NULL, NULL);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    tor_free(header);
}

static void
test_dir_handle_get_status_vote_next_d_not_found(void* data)
{
  char *header = NULL;
  (void) data;

  status_vote_next_d_test(&header, NULL, NULL);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    tor_free(header);
}

static void
test_dir_handle_get_status_vote_d(void* data)
{
  char *header = NULL, *body = NULL;
  size_t body_used = 0;
  dir_server_t *ds = NULL;
  const char digest[DIGEST_LEN] = "";
  (void) data;

  MOCK(check_signature_token, mock_ignore_signature_token);
  clear_dir_servers();
  dirvote_free_all();

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);

  init_mock_options();
  mock_options->AuthoritativeDir = 1;
  mock_options->V3AuthoritativeDir = 1;
  mock_options->TestingV3AuthVotingStartOffset = 0;
  mock_options->TestingV3AuthInitialVotingInterval = 1;
  mock_options->TestingV3AuthInitialVoteDelay = 1;
  mock_options->TestingV3AuthInitialDistDelay = 1;

  time_t now = 1441223455 -1;
  dirauth_sched_recalculate_timing(mock_options, now);

  const char *msg_out = NULL;
  int status_out = 0;
  struct pending_vote_t *pv = dirvote_add_vote(VOTE_BODY_V3, 0, "foo",
                                               &msg_out, &status_out);
  tt_assert(pv);

  status_vote_current_d_test(&header, &body, &body_used);

  tt_assert(header);
  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 4403\r\n"));

  tt_str_op(VOTE_BODY_V3, OP_EQ, body);

  tor_free(header);
  tor_free(body);

  status_vote_next_d_test(&header, &body, &body_used);

  tt_assert(header);
  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 4403\r\n"));

  tt_str_op(VOTE_BODY_V3, OP_EQ, body);

  done:
    UNMOCK(check_signature_token);
    tor_free(header);
    tor_free(body);
    or_options_free(mock_options); mock_options = NULL;

    clear_dir_servers();
    dirvote_free_all();
    routerlist_free_all();
}

static void
test_dir_handle_get_status_vote_next_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/" HEX1), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
status_vote_next_consensus_test(char **header, char **body, size_t *body_used)
{
  dir_connection_t *conn = NULL;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/consensus"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, header, MAX_HEADERS_SIZE,
                      body, body_used, 18, 0);
  done:
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
}

static void
test_dir_handle_get_status_vote_next_consensus_not_found(void* data)
{
  char *header = NULL, *body = NULL;
  size_t body_used;
  (void) data;

  status_vote_next_consensus_test(&header, &body, &body_used);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    tor_free(header);
    tor_free(body);
}

static void
test_dir_handle_get_status_vote_current_authority_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  MOCK(check_signature_token, mock_ignore_signature_token);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/authority"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(check_signature_token);
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_status_vote_next_authority_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  MOCK(check_signature_token, mock_ignore_signature_token);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/authority"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(check_signature_token);
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static void
test_dir_handle_get_status_vote_next_bandwidth_not_found(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL;
  (void) data;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);
  MOCK(check_signature_token, mock_ignore_signature_token);
  conn = new_dir_conn();

  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/bandwdith"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      NULL, NULL, 1, 0);
  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    UNMOCK(check_signature_token);
    UNMOCK(connection_write_to_buf_impl_);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
}

static const char* dhg_tests_dirvote_get_pending_consensus(
                                           consensus_flavor_t flav);

const char*
dhg_tests_dirvote_get_pending_consensus(consensus_flavor_t flav)
{
  (void)flav;
  return "pending consensus";
}

static void
test_dir_handle_get_status_vote_next_consensus(void* data)
{
  char *header = NULL, *body = NULL;
  size_t body_used = 0;
  (void) data;

  MOCK(dirvote_get_pending_consensus,
       dhg_tests_dirvote_get_pending_consensus);

  status_vote_next_consensus_test(&header, &body, &body_used);
  tt_assert(header);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 17\r\n"));

  tt_str_op("pending consensus", OP_EQ, body);

  done:
    UNMOCK(dirvote_get_pending_consensus);
    tor_free(header);
    tor_free(body);
}

static void
test_dir_handle_get_status_vote_next_consensus_busy(void* data)
{
  char *header = NULL, *body = NULL;
  size_t body_used = 0;
  (void) data;

  MOCK(get_options, mock_get_options);
  MOCK(dirvote_get_pending_consensus,
       dhg_tests_dirvote_get_pending_consensus);

  //Make it busy
  init_mock_options();
  mock_options->CountPrivateBandwidth = 1;

  status_vote_next_consensus_test(&header, &body, &body_used);

  tt_assert(header);
  tt_str_op(SERVER_BUSY, OP_EQ, header);

  done:
    UNMOCK(dirvote_get_pending_consensus);
    UNMOCK(get_options);
    tor_free(header);
    tor_free(body);
    or_options_free(mock_options); mock_options = NULL;
}

static void
status_vote_next_consensus_signatures_test(char **header, char **body,
                                           size_t *body_used)
{
  dir_connection_t *conn = NULL;

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/consensus-signatures"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, header, MAX_HEADERS_SIZE,
                      body, body_used, 22, 0);

  done:
    connection_free_minimal(TO_CONN(conn));
    UNMOCK(connection_write_to_buf_impl_);
}

static void
test_dir_handle_get_status_vote_next_consensus_signatures_not_found(void* data)
{
  char *header = NULL, *body = NULL;
  size_t body_used;
  (void) data;

  status_vote_next_consensus_signatures_test(&header, &body, &body_used);

  tt_assert(header);
  tt_str_op(NOT_FOUND, OP_EQ, header);

  done:
    tor_free(header);
    tor_free(body);
}

static const char* dhg_tests_dirvote_get_pending_detached_signatures(void);

const char*
dhg_tests_dirvote_get_pending_detached_signatures(void)
{
  return "pending detached sigs";
}

static void
test_dir_handle_get_status_vote_next_consensus_signatures(void* data)
{
  char *header = NULL, *body = NULL;
  size_t body_used = 0;
  (void) data;

  MOCK(dirvote_get_pending_detached_signatures,
       dhg_tests_dirvote_get_pending_detached_signatures);

  status_vote_next_consensus_signatures_test(&header, &body, &body_used);
  tt_assert(header);

  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 21\r\n"));

  tt_str_op("pending detached sigs", OP_EQ, body);

  done:
    UNMOCK(dirvote_get_pending_detached_signatures);
    tor_free(header);
    tor_free(body);
}

static void
test_dir_handle_get_status_vote_next_consensus_signatures_busy(void* data)
{
  char *header = NULL, *body = NULL;
  size_t body_used;
  (void) data;

  MOCK(dirvote_get_pending_detached_signatures,
       dhg_tests_dirvote_get_pending_detached_signatures);
  MOCK(get_options, mock_get_options);

  //Make it busy
  init_mock_options();
  mock_options->CountPrivateBandwidth = 1;

  status_vote_next_consensus_signatures_test(&header, &body, &body_used);

  tt_assert(header);
  tt_str_op(SERVER_BUSY, OP_EQ, header);

  done:
    UNMOCK(get_options);
    UNMOCK(dirvote_get_pending_detached_signatures);
    tor_free(header);
    tor_free(body);
    or_options_free(mock_options); mock_options = NULL;
}

static void
test_dir_handle_get_status_vote_next_authority(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL, *body = NULL;
  const char *msg_out = NULL;
  int status_out = 0;
  size_t body_used = 0;
  dir_server_t *ds = NULL;
  const char digest[DIGEST_LEN] = "";
  (void) data;

  MOCK(check_signature_token, mock_ignore_signature_token);
  clear_dir_servers();
  routerlist_free_all();
  dirvote_free_all();

  mock_cert = authority_cert_parse_from_string(TEST_CERTIFICATE,
                                               strlen(TEST_CERTIFICATE),
                                               NULL);

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);
  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  init_mock_options();
  mock_options->AuthoritativeDir = 1;
  mock_options->V3AuthoritativeDir = 1;
  mock_options->TestingV3AuthVotingStartOffset = 0;
  mock_options->TestingV3AuthInitialVotingInterval = 1;
  mock_options->TestingV3AuthInitialVoteDelay = 1;
  mock_options->TestingV3AuthInitialDistDelay = 1;

  time_t now = 1441223455 -1;
  dirauth_sched_recalculate_timing(mock_options, now);

  struct pending_vote_t *vote = dirvote_add_vote(VOTE_BODY_V3, 0, "foo",
                                                 &msg_out, &status_out);
  tt_assert(vote);

  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/authority"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(VOTE_BODY_V3)+1, 0);

  tt_assert(header);
  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 4403\r\n"));

  tt_str_op(VOTE_BODY_V3, OP_EQ, body);

  done:
    UNMOCK(check_signature_token);
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(get_my_v3_authority_cert);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    authority_cert_free(mock_cert); mock_cert = NULL;
    or_options_free(mock_options); mock_options = NULL;

    clear_dir_servers();
    routerlist_free_all();
    dirvote_free_all();
}

static void
test_dir_handle_get_status_vote_next_bandwidth(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL, *body = NULL;
  size_t body_used = 0;
  (void) data;

  const char *content =
    "1541171221\n"
    "node_id=$68A483E05A2ABDCA6DA5A3EF8DB5177638A27F80 "
    "master_key_ed25519=YaqV4vbvPYKucElk297eVdNArDz9HtIwUoIeo0+cVIpQ "
    "bw=760 nick=Test time=2018-05-08T16:13:26\n";

  init_mock_options();
  MOCK(get_options, mock_get_options);
  mock_options->V3BandwidthsFile = tor_strdup(
    get_fname_rnd("V3BandwidthsFile")
  );

  write_str_to_file(mock_options->V3BandwidthsFile, content, 0);

  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/bandwidth"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(content)+1, 0);

  tt_assert(header);
  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 167\r\n"));

  /* Check cache lifetime */
  char expbuf[RFC1123_TIME_LEN+1];
  time_t now = approx_time();
  /* BANDWIDTH_CACHE_LIFETIME is defined in dircache.c. */
  format_rfc1123_time(expbuf, (time_t)(now + 30*60));
  char *expires = NULL;
  /* Change to 'Cache-control: max-age=%d' if using http/1.1. */
  tor_asprintf(&expires, "Expires: %s\r\n", expbuf);
  tt_assert(strstr(header, expires));

  tt_int_op(body_used, OP_EQ, strlen(body));
  tt_str_op(content, OP_EQ, body);

  tor_free(header);
  tor_free(body);

  /* Request the file using compression, the result should be the same. */
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/next/bandwidth.z"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(content)+1, 0);

  tt_assert(header);
  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Encoding: deflate\r\n"));

  /* Since using connection_write_to_buf_mock instead of mocking
   * connection_buf_add_compress, the content is not actually compressed.
   * If it would, the size and content would be different than the original.
  */

 done:
  UNMOCK(get_options);
  UNMOCK(connection_write_to_buf_impl_);
  connection_free_minimal(TO_CONN(conn));
  tor_free(header);
  tor_free(body);
  tor_free(expires);
  or_options_free(mock_options);
}

static void
test_dir_handle_get_status_vote_current_authority(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL, *body = NULL;
  const char *msg_out = NULL;
  int status_out = 0;
  size_t body_used = 0;
  const char digest[DIGEST_LEN] = "";

  dir_server_t *ds = NULL;
  (void) data;

  MOCK(check_signature_token, mock_ignore_signature_token);
  clear_dir_servers();
  routerlist_free_all();
  dirvote_free_all();

  mock_cert = authority_cert_parse_from_string(TEST_CERTIFICATE,
                                               strlen(TEST_CERTIFICATE),
                                               NULL);

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);

  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  init_mock_options();
  mock_options->AuthoritativeDir = 1;
  mock_options->V3AuthoritativeDir = 1;
  mock_options->TestingV3AuthVotingStartOffset = 0;
  mock_options->TestingV3AuthInitialVotingInterval = 1;
  mock_options->TestingV3AuthInitialVoteDelay = 1;
  mock_options->TestingV3AuthInitialDistDelay = 1;

  time_t now = 1441223455;
  dirauth_sched_recalculate_timing(mock_options, now-1);

  struct pending_vote_t *vote = dirvote_add_vote(VOTE_BODY_V3, 0, "foo",
                                                 &msg_out, &status_out);
  tt_assert(vote);

  // move the pending vote to previous vote
  dirvote_act(mock_options, now+1);

  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/authority"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(VOTE_BODY_V3)+1, 0);

  tt_assert(header);
  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 4403\r\n"));

  tt_str_op(VOTE_BODY_V3, OP_EQ, body);

  done:
    UNMOCK(check_signature_token);
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(get_my_v3_authority_cert);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    authority_cert_free(mock_cert); mock_cert = NULL;
    or_options_free(mock_options); mock_options = NULL;

    clear_dir_servers();
    routerlist_free_all();
    dirvote_free_all();
}

/* Test that a late vote is rejected, but an on-time vote is accepted. */
static void
test_dir_handle_get_status_vote_too_late(void* data)
{
  dir_connection_t *conn = NULL;
  char *header = NULL, *body = NULL;
  const char *msg_out = NULL;
  int status_out = 0;
  size_t body_used = 0;
  const char digest[DIGEST_LEN] = "";

  dir_server_t *ds = NULL;
  const char* mode = (const char *)data;

  MOCK(check_signature_token, mock_ignore_signature_token);
  clear_dir_servers();
  routerlist_free_all();
  dirvote_free_all();

  mock_cert = authority_cert_parse_from_string(TEST_CERTIFICATE,
                                               strlen(TEST_CERTIFICATE),
                                               NULL);

  /* create a trusted ds */
  ds = trusted_dir_server_new("ds", "127.0.0.1", 9059, 9060, NULL, digest,
                              NULL, V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  /* ds v3_identity_digest is the certificate's identity_key */
  base16_decode(ds->v3_identity_digest, DIGEST_LEN,
                TEST_CERT_IDENT_KEY, HEX_DIGEST_LEN);

  tt_int_op(0, OP_EQ, trusted_dirs_load_certs_from_string(TEST_CERTIFICATE,
    TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST, 1, NULL));

  init_mock_options();
  mock_options->AuthoritativeDir = 1;
  mock_options->V3AuthoritativeDir = 1;

  int base_delay = 0;
  int vote_interval = 0;
  int start_offset = 0;

  tt_assert(mode);
  /* Set the required timings, see below for details */
  if (strcmp(mode, "min") == 0) {
    /* The minimum valid test network timing */
    base_delay = 2;
    vote_interval = 10;
    start_offset = vote_interval - 5;
  } else if (strcmp(mode, "chutney") == 0) {
    /* The test network timing used by chutney */
    base_delay = 4;
    vote_interval = 20;
    start_offset = vote_interval - 5;
  } else if (strcmp(mode, "half-public") == 0) {
    /* The short consensus failure timing used in the public network */
    base_delay = 5*60;
    vote_interval = 30*60;
    start_offset = vote_interval - 9*60 - 5;
  } else if (strcmp(mode, "public") == 0) {
    /* The standard timing used in the public network */
    base_delay = 5*60;
    vote_interval = 60*60;
    start_offset = vote_interval - 9*60 - 5;
  }

  tt_assert(base_delay > 0);
  tt_assert(vote_interval > 0);
  tt_assert(start_offset > 0);

  /* Skew the time to fit the fixed time in the vote */
  mock_options->TestingV3AuthVotingStartOffset = start_offset;
  /* Calculate the rest of the timings */
  mock_options->TestingV3AuthInitialVotingInterval = vote_interval;
  mock_options->TestingV3AuthInitialVoteDelay = base_delay;
  mock_options->TestingV3AuthInitialDistDelay = base_delay;

  time_t now = 1441223455;
  dirauth_sched_recalculate_timing(mock_options, now-1);
  const time_t voting_starts = voting_schedule.voting_starts;
  const time_t fetch_missing = voting_schedule.fetch_missing_votes;

  struct pending_vote_t *vote = NULL;

  /* Next voting interval */
  vote = dirvote_add_vote(VOTE_BODY_V3,
                          fetch_missing + vote_interval, "foo",
                          &msg_out, &status_out);
  tt_assert(!vote);
  tt_int_op(status_out, OP_EQ, 400);
  tt_str_op(msg_out, OP_EQ,
            "Posted vote received too late, would be dangerous to count it");

  /* Just after fetch missing */
  vote = dirvote_add_vote(VOTE_BODY_V3,
                          fetch_missing + 1, "foo",
                          &msg_out, &status_out);
  tt_assert(!vote);
  tt_int_op(status_out, OP_EQ, 400);
  tt_str_op(msg_out, OP_EQ,
            "Posted vote received too late, would be dangerous to count it");

  /* On fetch missing */
  vote = dirvote_add_vote(VOTE_BODY_V3,
                          fetch_missing, "foo",
                          &msg_out, &status_out);
  tt_assert(vote);

  /* Move the pending vote to previous vote */
  dirvote_act(mock_options, now+1);
  /* And reset the timing */
  dirauth_sched_recalculate_timing(mock_options, now-1);

  /* Between voting starts and fetch missing */
  vote = dirvote_add_vote(VOTE_BODY_V3,
                          voting_starts + 1, "foo",
                          &msg_out, &status_out);
  tt_assert(vote);

  /* Move the pending vote to previous vote */
  dirvote_act(mock_options, now+1);
  /* And reset the timing */
  dirauth_sched_recalculate_timing(mock_options, now-1);

  /* On voting starts */
  vote = dirvote_add_vote(VOTE_BODY_V3,
                          voting_starts, "foo",
                          &msg_out, &status_out);
  tt_assert(vote);

  /* Move the pending vote to previous vote */
  dirvote_act(mock_options, now+1);
  /* And reset the timing */
  dirauth_sched_recalculate_timing(mock_options, now-1);

  /* Just before voting starts */
  vote = dirvote_add_vote(VOTE_BODY_V3,
                          voting_starts - 1, "foo",
                          &msg_out, &status_out);
  tt_assert(vote);

  /* Move the pending vote to previous vote */
  dirvote_act(mock_options, now+1);

  MOCK(get_my_v3_authority_cert, get_my_v3_authority_cert_m);
  MOCK(connection_write_to_buf_impl_, connection_write_to_buf_mock);

  conn = new_dir_conn();
  tt_int_op(0, OP_EQ, directory_handle_command_get(conn,
    GET("/tor/status-vote/current/authority"), NULL, 0));

  fetch_from_buf_http(TO_CONN(conn)->outbuf, &header, MAX_HEADERS_SIZE,
                      &body, &body_used, strlen(VOTE_BODY_V3)+1, 0);

  tt_assert(header);
  tt_ptr_op(strstr(header, "HTTP/1.0 200 OK\r\n"), OP_EQ, header);
  tt_assert(strstr(header, "Content-Type: text/plain\r\n"));
  tt_assert(strstr(header, "Content-Encoding: identity\r\n"));
  tt_assert(strstr(header, "Content-Length: 4403\r\n"));

  tt_str_op(VOTE_BODY_V3, OP_EQ, body);

  done:
    UNMOCK(check_signature_token);
    UNMOCK(connection_write_to_buf_impl_);
    UNMOCK(get_my_v3_authority_cert);
    connection_free_minimal(TO_CONN(conn));
    tor_free(header);
    tor_free(body);
    authority_cert_free(mock_cert); mock_cert = NULL;
    or_options_free(mock_options); mock_options = NULL;

    clear_dir_servers();
    routerlist_free_all();
    dirvote_free_all();
}

static void
test_dir_handle_get_parse_accept_encoding(void *arg)
{
  (void)arg;
  const unsigned B_NONE = 1u << NO_METHOD;
  const unsigned B_ZLIB = 1u << ZLIB_METHOD;
  const unsigned B_GZIP = 1u << GZIP_METHOD;
  const unsigned B_LZMA = 1u << LZMA_METHOD;
  const unsigned B_ZSTD = 1u << ZSTD_METHOD;

  unsigned encodings;

  encodings = parse_accept_encoding_header("");
  tt_uint_op(B_NONE, OP_EQ, encodings);

  encodings = parse_accept_encoding_header("  ");
  tt_uint_op(B_NONE, OP_EQ, encodings);

  encodings = parse_accept_encoding_header("dewey, cheatham, and howe ");
  tt_uint_op(B_NONE, OP_EQ, encodings);

  encodings = parse_accept_encoding_header("dewey, cheatham, and gzip");
  tt_uint_op(B_NONE, OP_EQ, encodings);

  encodings = parse_accept_encoding_header("dewey, cheatham, and, gzip");
  tt_uint_op(B_NONE|B_GZIP, OP_EQ, encodings);

  encodings = parse_accept_encoding_header(" gzip");
  tt_uint_op(B_NONE|B_GZIP, OP_EQ, encodings);

  encodings = parse_accept_encoding_header("gzip");
  tt_uint_op(B_NONE|B_GZIP, OP_EQ, encodings);

  encodings = parse_accept_encoding_header("x-zstd, deflate, x-tor-lzma");
  tt_uint_op(B_NONE|B_ZLIB|B_ZSTD|B_LZMA, OP_EQ, encodings);

  encodings = parse_accept_encoding_header(
                                        "x-zstd, deflate, x-tor-lzma, gzip");
  tt_uint_op(B_NONE|B_ZLIB|B_ZSTD|B_LZMA|B_GZIP, OP_EQ, encodings);

  encodings = parse_accept_encoding_header("x-zstd,deflate,x-tor-lzma,gzip");
  tt_uint_op(B_NONE|B_ZLIB|B_ZSTD|B_LZMA|B_GZIP, OP_EQ, encodings);

 done:
  ;
}

#define DIR_HANDLE_CMD(name,flags) \
  { #name, test_dir_handle_get_##name, (flags), NULL, NULL }

#ifdef COCCI
/* Coccinelle doesn't like the stringification in this macro */
#define DIR_HANDLE_CMD_ARG(name,flags,arg) \
  DIR_HANDLE_CMD(name,flags)
#else
#define DIR_HANDLE_CMD_ARG(name,flags,arg) \
  { #name "/" arg, test_dir_handle_get_##name, (flags), \
    &passthrough_setup, (void *)(arg) }
#endif /* defined(COCCI) */

struct testcase_t dir_handle_get_tests[] = {
  DIR_HANDLE_CMD(not_found, 0),
  DIR_HANDLE_CMD(bad_request, 0),
  DIR_HANDLE_CMD(v1_command_not_found, 0),
  DIR_HANDLE_CMD(v1_command, 0),
  DIR_HANDLE_CMD(robots_txt, 0),
  DIR_HANDLE_CMD(rendezvous2_not_found_if_not_encrypted, 0),
  DIR_HANDLE_CMD(rendezvous2_not_found, 0),
  DIR_HANDLE_CMD(rendezvous2_on_encrypted_conn_with_invalid_desc_id, 0),
  DIR_HANDLE_CMD(rendezvous2_on_encrypted_conn_not_well_formed, 0),
  DIR_HANDLE_CMD(rendezvous2_on_encrypted_conn_success, 0),
  DIR_HANDLE_CMD(micro_d_not_found, 0),
  DIR_HANDLE_CMD(micro_d_server_busy, 0),
  DIR_HANDLE_CMD(micro_d, 0),
  DIR_HANDLE_CMD(networkstatus_bridges_not_found_without_auth, 0),
  DIR_HANDLE_CMD(networkstatus_bridges_not_found_wrong_auth, 0),
  DIR_HANDLE_CMD(networkstatus_bridges, 0),
  DIR_HANDLE_CMD(server_descriptors_not_found, 0),
  DIR_HANDLE_CMD(server_descriptors_busy, TT_FORK),
  DIR_HANDLE_CMD(server_descriptors_all, TT_FORK),
  DIR_HANDLE_CMD(server_descriptors_authority, TT_FORK),
  DIR_HANDLE_CMD(server_descriptors_fp, TT_FORK),
  DIR_HANDLE_CMD(server_descriptors_d, TT_FORK),
  DIR_HANDLE_CMD(server_keys_bad_req, 0),
  DIR_HANDLE_CMD(server_keys_busy, 0),
  DIR_HANDLE_CMD(server_keys_all_not_found, 0),
  DIR_HANDLE_CMD(server_keys_all, 0),
  DIR_HANDLE_CMD(server_keys_authority_not_found, 0),
  DIR_HANDLE_CMD(server_keys_authority, 0),
  DIR_HANDLE_CMD(server_keys_fp_not_found, 0),
  DIR_HANDLE_CMD(server_keys_fp, 0),
  DIR_HANDLE_CMD(server_keys_sk_not_found, 0),
  DIR_HANDLE_CMD(server_keys_sk, 0),
  DIR_HANDLE_CMD(server_keys_fpsk_not_found, 0),
  DIR_HANDLE_CMD(server_keys_fpsk, 0),
  DIR_HANDLE_CMD(status_vote_current_not_found, 0),
  DIR_HANDLE_CMD(status_vote_next_not_found, 0),
  DIR_HANDLE_CMD(status_vote_current_authority_not_found, 0),
  DIR_HANDLE_CMD(status_vote_current_authority, 0),
  DIR_HANDLE_CMD_ARG(status_vote_too_late, 0, "min"),
  DIR_HANDLE_CMD_ARG(status_vote_too_late, 0, "chutney"),
  DIR_HANDLE_CMD_ARG(status_vote_too_late, 0, "half-public"),
  DIR_HANDLE_CMD_ARG(status_vote_too_late, 0, "public"),
  DIR_HANDLE_CMD(status_vote_next_authority_not_found, 0),
  DIR_HANDLE_CMD(status_vote_next_authority, 0),
  DIR_HANDLE_CMD(status_vote_next_bandwidth_not_found, 0),
  DIR_HANDLE_CMD(status_vote_next_bandwidth, 0),
  DIR_HANDLE_CMD(status_vote_current_consensus_ns_not_enough_sigs, TT_FORK),
  DIR_HANDLE_CMD(status_vote_current_consensus_ns_not_found, TT_FORK),
  DIR_HANDLE_CMD(status_vote_current_consensus_too_old, TT_FORK),
  DIR_HANDLE_CMD(status_vote_current_consensus_ns_busy, TT_FORK),
  DIR_HANDLE_CMD(status_vote_current_consensus_ns, TT_FORK),
  DIR_HANDLE_CMD(status_vote_current_d_not_found, 0),
  DIR_HANDLE_CMD(status_vote_next_d_not_found, 0),
  DIR_HANDLE_CMD(status_vote_d, 0),
  DIR_HANDLE_CMD(status_vote_next_consensus_not_found, 0),
  DIR_HANDLE_CMD(status_vote_next_consensus_busy, 0),
  DIR_HANDLE_CMD(status_vote_next_consensus, 0),
  DIR_HANDLE_CMD(status_vote_next_consensus_signatures_not_found, 0),
  DIR_HANDLE_CMD(status_vote_next_consensus_signatures_busy, 0),
  DIR_HANDLE_CMD(status_vote_next_consensus_signatures, 0),
  DIR_HANDLE_CMD(parse_accept_encoding, 0),
  END_OF_TESTCASES
};
