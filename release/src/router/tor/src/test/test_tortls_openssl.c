/* Copyright (c) 2010-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define TORTLS_PRIVATE
#define TORTLS_OPENSSL_PRIVATE
#define TOR_X509_PRIVATE
#define LOG_PRIVATE
#include "orconfig.h"

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <math.h>

#include "lib/cc/compat_compiler.h"

/* Some versions of OpenSSL declare SSL_get_selected_srtp_profile twice in
 * srtp.h. Suppress the GCC warning so we can build with -Wredundant-decl. */
DISABLE_GCC_WARNING("-Wredundant-decls")

#include <openssl/opensslv.h>

#include <openssl/ssl.h>
#include <openssl/ssl3.h>
#include <openssl/err.h>
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bn.h>

ENABLE_GCC_WARNING("-Wredundant-decls")

#include "core/or/or.h"
#include "lib/log/log.h"
#include "app/config/config.h"
#include "lib/crypt_ops/compat_openssl.h"
#include "lib/tls/x509.h"
#include "lib/tls/x509_internal.h"
#include "lib/tls/tortls.h"
#include "lib/tls/tortls_st.h"
#include "lib/tls/tortls_internal.h"
#include "app/config/or_state_st.h"

#include "test/test.h"
#include "test/log_test_helpers.h"
#include "test/test_tortls.h"

#define SSL_STATE_STR "before SSL initialization"

/* Every version and fork of OpenSSL we support now qualifies as "opaque",
 * in that it hides the members of important structures.
 *
 * That's a good thing, but it means we can't run a number of older tests
 * that require the ability to poke at OpenSSL's internals.
 *
 * We're retaining these tests here, rather than removing them,
 * in case anybody wants to port them to modern OpenSSL.
 * (Some of them are probably not worth saving, though.)
 */
#define OPENSSL_OPAQUE

#ifndef OPENSSL_OPAQUE
static SSL_METHOD *
give_me_a_test_method(void)
{
  SSL_METHOD *method = tor_malloc_zero(sizeof(SSL_METHOD));
  memcpy(method, TLSv1_method(), sizeof(SSL_METHOD));
  return method;
}

static int
fake_num_ciphers(void)
{
  return 0;
}
#endif /* !defined(OPENSSL_OPAQUE) */

static int
mock_tls_cert_matches_key(const tor_tls_t *tls, const tor_x509_cert_t *cert)
{
  (void) tls;
  (void) cert; // XXXX look at this.
  return 1;
}

static void
test_tortls_tor_tls_new(void *data)
{
  (void) data;
  MOCK(tor_tls_cert_matches_key, mock_tls_cert_matches_key);
  crypto_pk_t *key1 = NULL, *key2 = NULL;
  SSL_METHOD *method = NULL;

  key1 = pk_generate(2);
  key2 = pk_generate(3);

  tor_tls_t *tls = NULL;
  tt_int_op(tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                                 key1, key2, 86400), OP_EQ, 0);
  tls = tor_tls_new(-1, 0);
  tt_want(tls);
  tor_tls_free(tls); tls = NULL;

  SSL_CTX_free(client_tls_context->ctx);
  client_tls_context->ctx = NULL;
  tls = tor_tls_new(-1, 0);
  tt_ptr_op(tls, OP_EQ, NULL);

#ifndef OPENSSL_OPAQUE
  method = give_me_a_test_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  method->num_ciphers = fake_num_ciphers;
  client_tls_context->ctx = ctx;
  tls = tor_tls_new(-1, 0);
  tt_ptr_op(tls, OP_EQ, NULL);
#endif /* !defined(OPENSSL_OPAQUE) */

 done:
  UNMOCK(tor_tls_cert_matches_key);
  crypto_pk_free(key1);
  crypto_pk_free(key2);
  tor_tls_free(tls);
  tor_free(method);
  tor_tls_free_all();
}

static void
library_init(void)
{
  OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
}

static void
test_tortls_get_state_description(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  char *buf;
  SSL_CTX *ctx;

  library_init();
  ctx = SSL_CTX_new(SSLv23_method());

  buf = tor_malloc_zero(1000);
  tls = tor_malloc_zero(sizeof(tor_tls_t));

  tor_tls_get_state_description(NULL, buf, 20);
  tt_str_op(buf, OP_EQ, "(No SSL object)");

  SSL_free(tls->ssl);
  tls->ssl = NULL;
  tor_tls_get_state_description(tls, buf, 20);
  tt_str_op(buf, OP_EQ, "(No SSL object)");

  tls->ssl = SSL_new(ctx);
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR " in HANDSHAKE");

  tls->state = TOR_TLS_ST_OPEN;
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR " in OPEN");

  tls->state = TOR_TLS_ST_GOTCLOSE;
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR " in GOTCLOSE");

  tls->state = TOR_TLS_ST_SENTCLOSE;
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR " in SENTCLOSE");

  tls->state = TOR_TLS_ST_CLOSED;
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR " in CLOSED");

  tls->state = TOR_TLS_ST_RENEGOTIATE;
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR " in RENEGOTIATE");

  tls->state = TOR_TLS_ST_BUFFEREVENT;
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR);

  tls->state = 7;
  tor_tls_get_state_description(tls, buf, 200);
  tt_str_op(buf, OP_EQ, SSL_STATE_STR " in unknown TLS state");

 done:
  SSL_CTX_free(ctx);
  SSL_free(tls->ssl);
  tor_free(buf);
  tor_free(tls);
}

static void
test_tortls_get_by_ssl(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  tor_tls_t *res;
  SSL_CTX *ctx;
  SSL *ssl;

  library_init();
  tor_tls_allocate_tor_tls_object_ex_data_index();

  ctx = SSL_CTX_new(SSLv23_method());
  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->magic = TOR_TLS_MAGIC;

  ssl = SSL_new(ctx);

  res = tor_tls_get_by_ssl(ssl);
  tt_assert(!res);

  SSL_set_ex_data(ssl, tor_tls_object_ex_data_index, tls);

  res = tor_tls_get_by_ssl(ssl);
  tt_assert(res == tls);

 done:
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  tor_free(tls);
}

static void
test_tortls_allocate_tor_tls_object_ex_data_index(void *ignored)
{
  (void)ignored;
  int first;

  tor_tls_allocate_tor_tls_object_ex_data_index();

  first = tor_tls_object_ex_data_index;
  tor_tls_allocate_tor_tls_object_ex_data_index();
  tt_int_op(first, OP_EQ, tor_tls_object_ex_data_index);

 done:
  (void)0;
}

static void
test_tortls_log_one_error(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  SSL_CTX *ctx;
  SSL *ssl = NULL;

  library_init();

  ctx = SSL_CTX_new(SSLv23_method());
  tls = tor_malloc_zero(sizeof(tor_tls_t));
  setup_capture_of_logs(LOG_INFO);

  tor_tls_log_one_error(NULL, 0, LOG_WARN, 0, "something");
  expect_log_msg("TLS error while something: "
            "(null) (in (null):(null):---)\n");

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, 0, LOG_WARN, 0, NULL);
  expect_log_msg("TLS error: (null) "
            "(in (null):(null):---)\n");

  mock_clean_saved_logs();
  tls->address = tor_strdup("127.hello");
  tor_tls_log_one_error(tls, 0, LOG_WARN, 0, NULL);
  expect_log_msg("TLS error with 127.hello: "
            "(null) (in (null):(null):---)\n");
  tor_free(tls->address);

  mock_clean_saved_logs();
  tls->address = tor_strdup("127.hello");
  tor_tls_log_one_error(tls, 0, LOG_WARN, 0, "blarg");
  expect_log_msg("TLS error while blarg with "
            "127.hello: (null) (in (null):(null):---)\n");

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, ERR_PACK(1, 2, 3), LOG_WARN, 0, NULL);
  expect_log_msg_containing("TLS error with 127.hello");

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, ERR_PACK(1, 2, SSL_R_HTTP_REQUEST),
                        LOG_WARN, 0, NULL);
  expect_log_severity(LOG_INFO);

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, ERR_PACK(1, 2, SSL_R_HTTPS_PROXY_REQUEST),
                        LOG_WARN, 0, NULL);
  expect_log_severity(LOG_INFO);

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, ERR_PACK(1, 2, SSL_R_RECORD_LENGTH_MISMATCH),
                        LOG_WARN, 0, NULL);
  expect_log_severity(LOG_INFO);

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, ERR_PACK(1, 2, SSL_R_UNKNOWN_PROTOCOL),
                        LOG_WARN, 0, NULL);
  expect_log_severity(LOG_INFO);

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, ERR_PACK(1, 2, SSL_R_UNSUPPORTED_PROTOCOL),
                        LOG_WARN, 0, NULL);
  expect_log_severity(LOG_INFO);

  tls->ssl = SSL_new(ctx);

  mock_clean_saved_logs();
  tor_tls_log_one_error(tls, 0, LOG_WARN, 0, NULL);
  expect_log_msg("TLS error with 127.hello: (null)"
            " (in (null):(null):" SSL_STATE_STR ")\n");

 done:
  teardown_capture_of_logs();
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  if (tls && tls->ssl)
    SSL_free(tls->ssl);
  if (tls)
    tor_free(tls->address);
  tor_free(tls);
}

#ifndef OPENSSL_OPAQUE
static void
test_tortls_get_error(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  int ret;
  SSL_CTX *ctx;

  library_init();

  ctx = SSL_CTX_new(SSLv23_method());
  setup_capture_of_logs(LOG_INFO);
  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = SSL_new(ctx);
  SSL_set_bio(tls->ssl, BIO_new(BIO_s_mem()), NULL);

  ret = tor_tls_get_error(tls, 0, 0, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, TOR_TLS_ERROR_IO);
  expect_log_msg("TLS error: unexpected close while"
            " something (before/accept initialization)\n");

  mock_clean_saved_logs();
  ret = tor_tls_get_error(tls, 2, 0, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_entry();

  mock_clean_saved_logs();
  ret = tor_tls_get_error(tls, 0, 1, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, -11);
  expect_no_log_entry();

  mock_clean_saved_logs();
  ERR_clear_error();
  ERR_put_error(ERR_LIB_BN, 2, -1, "somewhere.c", 99);
  ret = tor_tls_get_error(tls, 0, 0, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, TOR_TLS_ERROR_MISC);
  expect_log_msg("TLS error while something: (null)"
            " (in bignum routines:(null):before/accept initialization)\n");

  mock_clean_saved_logs();
  ERR_clear_error();
  tls->ssl->rwstate = SSL_READING;
  SSL_get_rbio(tls->ssl)->flags = BIO_FLAGS_READ;
  ret = tor_tls_get_error(tls, -1, 0, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, TOR_TLS_WANTREAD);
  expect_no_log_entry();

  mock_clean_saved_logs();
  ERR_clear_error();
  tls->ssl->rwstate = SSL_READING;
  SSL_get_rbio(tls->ssl)->flags = BIO_FLAGS_WRITE;
  ret = tor_tls_get_error(tls, -1, 0, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, TOR_TLS_WANTWRITE);
  expect_no_log_entry();

  mock_clean_saved_logs();
  ERR_clear_error();
  tls->ssl->rwstate = 0;
  tls->ssl->shutdown = SSL_RECEIVED_SHUTDOWN;
  tls->ssl->s3->warn_alert =SSL_AD_CLOSE_NOTIFY;
  ret = tor_tls_get_error(tls, 0, 0, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, TOR_TLS_CLOSE);
  expect_log_entry();

  mock_clean_saved_logs();
  ret = tor_tls_get_error(tls, 0, 2, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, -10);
  expect_no_log_entry();

  mock_clean_saved_logs();
  ERR_put_error(ERR_LIB_SYS, 2, -1, "somewhere.c", 99);
  ret = tor_tls_get_error(tls, -1, 0, "something", LOG_WARN, 0);
  tt_int_op(ret, OP_EQ, -9);
  expect_log_msg("TLS error while something: (null) (in system library:"
            "connect:before/accept initialization)\n");

 done:
  teardown_capture_of_logs();
  SSL_free(tls->ssl);
  tor_free(tls);
  SSL_CTX_free(ctx);
}
#endif /* !defined(OPENSSL_OPAQUE) */

static void
test_tortls_always_accept_verify_cb(void *ignored)
{
  (void)ignored;
  int ret;

  ret = always_accept_verify_cb(0, NULL);
  tt_int_op(ret, OP_EQ, 1);

 done:
  (void)0;
}

#ifndef OPENSSL_OPAQUE
static void
test_tortls_x509_cert_free(void *ignored)
{
  (void)ignored;
  tor_x509_cert_t *cert;

  cert = tor_malloc_zero(sizeof(tor_x509_cert_t));
  tor_x509_cert_free(cert);

  cert = tor_malloc_zero(sizeof(tor_x509_cert_t));
  cert->cert = X509_new();
  cert->encoded = tor_malloc_zero(1);
  tor_x509_cert_free(cert);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
/*
 * Use only for the matching fake_x509_free() call
 */
static X509 *
fake_x509_malloc(void)
{
  return tor_malloc_zero(sizeof(X509));
}

static void
fake_x509_free(X509 *cert)
{
  if (cert) {
    if (cert->cert_info) {
      if (cert->cert_info->key) {
        if (cert->cert_info->key->pkey) {
          tor_free(cert->cert_info->key->pkey);
        }
        tor_free(cert->cert_info->key);
      }
      tor_free(cert->cert_info);
    }
    tor_free(cert);
  }
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static void
test_tortls_cert_get_key(void *ignored)
{
  (void)ignored;
  tor_x509_cert_t *cert = NULL;
  crypto_pk_t *res = NULL;
  cert = tor_malloc_zero(sizeof(tor_x509_cert_t));
  X509 *key = NULL;
  key = fake_x509_malloc();
  key->references = 1;

  res = tor_tls_cert_get_key(cert);
  tt_assert(!res);

  cert->cert = key;
  key->cert_info = tor_malloc_zero(sizeof(X509_CINF));
  key->cert_info->key = tor_malloc_zero(sizeof(X509_PUBKEY));
  key->cert_info->key->pkey = tor_malloc_zero(sizeof(EVP_PKEY));
  key->cert_info->key->pkey->references = 1;
  key->cert_info->key->pkey->type = 2;
  res = tor_tls_cert_get_key(cert);
  tt_assert(!res);

 done:
  fake_x509_free(key);
  tor_free(cert);
  crypto_pk_free(res);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static int fixed_ssl_pending_result = 0;

static int
fixed_ssl_pending(const SSL *ignored)
{
  (void)ignored;
  return fixed_ssl_pending_result;
}

static void
test_tortls_get_pending_bytes(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  SSL_METHOD *method;

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  method = tor_malloc_zero(sizeof(SSL_METHOD));
  method->ssl_pending = fixed_ssl_pending;
  tls->ssl->method = method;

  fixed_ssl_pending_result = 42;
  ret = tor_tls_get_pending_bytes(tls);
  tt_int_op(ret, OP_EQ, 42);

 done:
  tor_free(method);
  tor_free(tls->ssl);
  tor_free(tls);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static void
test_tortls_get_buffer_sizes(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  size_t rbuf_c=-1, rbuf_b=-1, wbuf_c=-1, wbuf_b=-1;

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  tls->ssl->s3 = tor_malloc_zero(sizeof(SSL3_STATE));

  tls->ssl->s3->rbuf.buf = NULL;
  tls->ssl->s3->rbuf.len = 1;
  tls->ssl->s3->rbuf.offset = 0;
  tls->ssl->s3->rbuf.left = 42;

  tls->ssl->s3->wbuf.buf = NULL;
  tls->ssl->s3->wbuf.len = 2;
  tls->ssl->s3->wbuf.offset = 0;
  tls->ssl->s3->wbuf.left = 43;

  ret = tor_tls_get_buffer_sizes(tls, &rbuf_c, &rbuf_b, &wbuf_c, &wbuf_b);
  tt_int_op(ret, OP_EQ, -1);

 done:
  tor_free(tls->ssl->s3->rbuf.buf);
  tor_free(tls->ssl->s3->wbuf.buf);
  tor_free(tls->ssl->s3);
  tor_free(tls->ssl);
  tor_free(tls);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static void
test_tortls_get_peer_cert(void *ignored)
{
  (void)ignored;
  tor_x509_cert_t *ret;
  tor_tls_t *tls;
  X509 *cert = NULL;

  cert = read_cert_from(validCertString);

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  tls->ssl->session = tor_malloc_zero(sizeof(SSL_SESSION));

  ret = tor_tls_get_peer_cert(tls);
  tt_assert(!ret);

  tls->ssl->session->peer = cert;
  ret = tor_tls_get_peer_cert(tls);
  tt_assert(ret);
  tt_assert(ret->cert == cert);

 done:
  tor_x509_cert_free(ret);
  tor_free(tls->ssl->session);
  tor_free(tls->ssl);
  tor_free(tls);
  X509_free(cert);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static void
test_tortls_peer_has_cert(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  X509 *cert = NULL;

  cert = read_cert_from(validCertString);

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  tls->ssl->session = tor_malloc_zero(sizeof(SSL_SESSION));

  ret = tor_tls_peer_has_cert(tls);
  tt_assert(!ret);

  tls->ssl->session->peer = cert;
  ret = tor_tls_peer_has_cert(tls);
  tt_assert(ret);

 done:
  tor_free(tls->ssl->session);
  tor_free(tls->ssl);
  tor_free(tls);
  X509_free(cert);
}
#endif /* !defined(OPENSSL_OPAQUE) */

static void
test_tortls_get_write_overhead_ratio(void *ignored)
{
  (void)ignored;
  double ret;

  total_bytes_written_over_tls = 0;
  ret = tls_get_write_overhead_ratio();
  tt_double_op(fabs(ret - 1.0), OP_LT, 1E-12);

  total_bytes_written_by_tls = 10;
  total_bytes_written_over_tls = 1;
  ret = tls_get_write_overhead_ratio();
  tt_double_op(fabs(ret - 10.0), OP_LT, 1E-12);

  total_bytes_written_by_tls = 10;
  total_bytes_written_over_tls = 2;
  ret = tls_get_write_overhead_ratio();
  tt_double_op(fabs(ret - 5.0), OP_LT, 1E-12);

 done:
  (void)0;
}

static void
test_tortls_is_server(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  int ret;

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->isServer = 1;
  ret = tor_tls_is_server(tls);
  tt_int_op(ret, OP_EQ, 1);

 done:
  tor_free(tls);
}

#ifndef OPENSSL_OPAQUE
static void
test_tortls_session_secret_cb(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  SSL_CTX *ctx;
  STACK_OF(SSL_CIPHER) *ciphers = NULL;
  SSL_CIPHER *one;

  library_init();

  tor_tls_allocate_tor_tls_object_ex_data_index();

  tls = tor_malloc_zero(sizeof(tor_tls_t));

  tls->magic = TOR_TLS_MAGIC;

  ctx = SSL_CTX_new(TLSv1_method());
  tls->ssl = SSL_new(ctx);
  SSL_set_ex_data(tls->ssl, tor_tls_object_ex_data_index, tls);

  SSL_set_session_secret_cb(tls->ssl, tor_tls_session_secret_cb, NULL);

  tor_tls_session_secret_cb(tls->ssl, NULL, NULL, NULL, NULL, NULL);
  tt_assert(!tls->ssl->tls_session_secret_cb);

  one = get_cipher_by_name("ECDHE-RSA-AES256-GCM-SHA384");
  one->id = 0x00ff;
  ciphers = sk_SSL_CIPHER_new_null();
  sk_SSL_CIPHER_push(ciphers, one);

  tls->client_cipher_list_type = 0;
  tor_tls_session_secret_cb(tls->ssl, NULL, NULL, ciphers, NULL, NULL);
  tt_assert(!tls->ssl->tls_session_secret_cb);

 done:
  sk_SSL_CIPHER_free(ciphers);
  SSL_free(tls->ssl);
  SSL_CTX_free(ctx);
  tor_free(tls);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
/* TODO: It seems block_renegotiation and unblock_renegotiation and
 * using different blags. This might not be correct */
static void
test_tortls_block_renegotiation(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  tls->ssl->s3 = tor_malloc_zero(sizeof(SSL3_STATE));
#ifndef SUPPORT_UNSAFE_RENEGOTIATION_FLAG
#define SSL3_FLAGS_ALLOW_UNSAFE_LEGACY_RENEGOTIATION 0
#endif

  tls->ssl->s3->flags = SSL3_FLAGS_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;

  tor_tls_block_renegotiation(tls);

 done:
  tor_free(tls->ssl->s3);
  tor_free(tls->ssl);
  tor_free(tls);
}

static void
test_tortls_unblock_renegotiation(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  tor_tls_unblock_renegotiation(tls);

  tt_uint_op(SSL_get_options(tls->ssl) &
             SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION, OP_EQ,
             SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION);

 done:
  tor_free(tls->ssl);
  tor_free(tls);
}
#endif /* !defined(OPENSSL_OPAQUE) */

static void
test_tortls_set_logged_address(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;

  tls = tor_malloc_zero(sizeof(tor_tls_t));

  tor_tls_set_logged_address(tls, "foo bar");

  tt_str_op(tls->address, OP_EQ, "foo bar");

  tor_tls_set_logged_address(tls, "foo bar 2");
  tt_str_op(tls->address, OP_EQ, "foo bar 2");

 done:
  tor_free(tls->address);
  tor_free(tls);
}

#ifndef OPENSSL_OPAQUE
static void
example_cb(tor_tls_t *t, void *arg)
{
  (void)t;
  (void)arg;
}

static void
test_tortls_set_renegotiate_callback(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  const char *arg = "hello";

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));

  tor_tls_set_renegotiate_callback(tls, example_cb, (void*)arg);
  tt_assert(tls->negotiated_callback == example_cb);
  tt_assert(tls->callback_arg == arg);
  tt_assert(!tls->got_renegotiate);

  /* Assumes V2_HANDSHAKE_SERVER */
  tt_assert(tls->ssl->info_callback == tor_tls_server_info_callback);

  tor_tls_set_renegotiate_callback(tls, NULL, (void*)arg);
  tt_assert(tls->ssl->info_callback == tor_tls_debug_state_callback);

 done:
  tor_free(tls->ssl);
  tor_free(tls);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static void
test_tortls_debug_state_callback(void *ignored)
{
  (void)ignored;
  SSL *ssl;
  char *buf = tor_malloc_zero(1000);
  int n;

  setup_capture_of_logs(LOG_DEBUG);

  ssl = tor_malloc_zero(sizeof(SSL));

  tor_tls_debug_state_callback(ssl, 32, 45);

  n = tor_snprintf(buf, 1000, "SSL %p is now in state unknown"
               " state [type=32,val=45].\n", ssl);
  /* tor's snprintf returns -1 on error */
  tt_int_op(n, OP_NE, -1);
  expect_log_msg(buf);

 done:
  teardown_capture_of_logs();
  tor_free(buf);
  tor_free(ssl);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static void
test_tortls_server_info_callback(void *ignored)
{
  (void)ignored;
  tor_tls_t *tls;
  SSL_CTX *ctx;
  SSL *ssl;

  library_init();

  ctx = SSL_CTX_new(TLSv1_method());
  ssl = SSL_new(ctx);

  tor_tls_allocate_tor_tls_object_ex_data_index();

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->magic = TOR_TLS_MAGIC;
  tls->ssl = ssl;

  setup_full_capture_of_logs(LOG_WARN);
  SSL_set_state(ssl, SSL3_ST_SW_SRVR_HELLO_A);
  mock_clean_saved_logs();
  tor_tls_server_info_callback(ssl, SSL_CB_ACCEPT_LOOP, 0);
  expect_single_log_msg("Couldn't look up the tls for an SSL*. How odd!\n");

  SSL_set_state(ssl, SSL3_ST_SW_SRVR_HELLO_B);
  mock_clean_saved_logs();
  tor_tls_server_info_callback(ssl, SSL_CB_ACCEPT_LOOP, 0);
  expect_single_log_msg("Couldn't look up the tls for an SSL*. How odd!\n");

  SSL_set_state(ssl, 99);
  mock_clean_saved_logs();
  tor_tls_server_info_callback(ssl, SSL_CB_ACCEPT_LOOP, 0);
  expect_no_log_entry();
  teardown_capture_of_logs();

  SSL_set_ex_data(tls->ssl, tor_tls_object_ex_data_index, tls);
  SSL_set_state(ssl, SSL3_ST_SW_SRVR_HELLO_B);
  tls->negotiated_callback = 0;
  //tls->server_handshake_count = 120;
  tor_tls_server_info_callback(ssl, SSL_CB_ACCEPT_LOOP, 0);
  //tt_int_op(tls->server_handshake_count, OP_EQ, 121);

  //tls->server_handshake_count = 127;
  tls->negotiated_callback = (void *)1;
  tor_tls_server_info_callback(ssl, SSL_CB_ACCEPT_LOOP, 0);
  //tt_int_op(tls->server_handshake_count, OP_EQ, 127);
  tt_int_op(tls->got_renegotiate, OP_EQ, 1);

  tls->ssl->session = SSL_SESSION_new();
  tls->wasV2Handshake = 0;
  tor_tls_server_info_callback(ssl, SSL_CB_ACCEPT_LOOP, 0);
  tt_int_op(tls->wasV2Handshake, OP_EQ, 0);

 done:
  teardown_capture_of_logs();
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  tor_free(tls);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static int fixed_ssl_read_result_index;
static int fixed_ssl_read_result[5];

static int
fixed_ssl_read(SSL *s, void *buf, int len)
{
  (void)s;
  (void)buf;
  (void)len;
  return fixed_ssl_read_result[fixed_ssl_read_result_index++];
}

static int
dummy_handshake_func(SSL *s)
{
  (void)s;
  return 1;
}

static int negotiated_callback_called;

static void
negotiated_callback_setter(tor_tls_t *t, void *arg)
{
  (void)t;
  (void)arg;
  negotiated_callback_called++;
}

static void
test_tortls_read(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  char buf[100];
  SSL_METHOD *method = give_me_a_test_method();
  setup_capture_of_logs(LOG_WARN);

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  tls->state = TOR_TLS_ST_OPEN;

  ret = tor_tls_read(tls, buf, 10);
  tt_int_op(ret, OP_EQ, -9);

  /* These tests assume that V2_HANDSHAKE_SERVER is set */
  tls->ssl->handshake_func = dummy_handshake_func;
  tls->ssl->method = method;
  method->ssl_read = fixed_ssl_read;
  fixed_ssl_read_result_index = 0;
  fixed_ssl_read_result[0] = 42;
  tls->state = TOR_TLS_ST_OPEN;
  ERR_clear_error();
  ret = tor_tls_read(tls, buf, 10);
  tt_int_op(ret, OP_EQ, 42);

  tls->state = TOR_TLS_ST_OPEN;
  tls->got_renegotiate = 1;
  fixed_ssl_read_result_index = 0;
  ERR_clear_error();
  ret = tor_tls_read(tls, buf, 10);
  tt_int_op(tls->got_renegotiate, OP_EQ, 0);

  tls->state = TOR_TLS_ST_OPEN;
  tls->got_renegotiate = 1;
  negotiated_callback_called = 0;
  tls->negotiated_callback = negotiated_callback_setter;
  fixed_ssl_read_result_index = 0;
  ERR_clear_error();
  ret = tor_tls_read(tls, buf, 10);
  tt_int_op(negotiated_callback_called, OP_EQ, 1);

#ifndef LIBRESSL_VERSION_NUMBER
  fixed_ssl_read_result_index = 0;
  fixed_ssl_read_result[0] = 0;
  tls->ssl->version = SSL2_VERSION;
  ERR_clear_error();
  ret = tor_tls_read(tls, buf, 10);
  tt_int_op(ret, OP_EQ, TOR_TLS_CLOSE);
  tt_int_op(tls->state, OP_EQ, TOR_TLS_ST_CLOSED);
#endif /* !defined(LIBRESSL_VERSION_NUMBER) */
  // TODO: fill up

 done:
  teardown_capture_of_logs();
  tor_free(tls->ssl);
  tor_free(tls);
  tor_free(method);
}

static int fixed_ssl_write_result;

static int
fixed_ssl_write(SSL *s, const void *buf, int len)
{
  (void)s;
  (void)buf;
  (void)len;
  return fixed_ssl_write_result;
}

static void
test_tortls_write(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  SSL_METHOD *method = give_me_a_test_method();
  char buf[100];
  setup_capture_of_logs(LOG_WARN);

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = tor_malloc_zero(sizeof(SSL));
  tls->state = TOR_TLS_ST_OPEN;

  ret = tor_tls_write(tls, buf, 0);
  tt_int_op(ret, OP_EQ, 0);

  ret = tor_tls_write(tls, buf, 10);
  tt_int_op(ret, OP_EQ, -9);

  tls->ssl->method = method;
  tls->wantwrite_n = 1;
  ret = tor_tls_write(tls, buf, 10);
  tt_int_op(tls->wantwrite_n, OP_EQ, 0);

  method->ssl_write = fixed_ssl_write;
  tls->ssl->handshake_func = dummy_handshake_func;
  fixed_ssl_write_result = 1;
  ERR_clear_error();
  ret = tor_tls_write(tls, buf, 10);
  tt_int_op(ret, OP_EQ, 1);

  fixed_ssl_write_result = -1;
  ERR_clear_error();
  tls->ssl->rwstate = SSL_READING;
  SSL_set_bio(tls->ssl, BIO_new(BIO_s_mem()), NULL);
  SSL_get_rbio(tls->ssl)->flags = BIO_FLAGS_READ;
  ret = tor_tls_write(tls, buf, 10);
  tt_int_op(ret, OP_EQ, TOR_TLS_WANTREAD);

  ERR_clear_error();
  tls->ssl->rwstate = SSL_READING;
  SSL_set_bio(tls->ssl, BIO_new(BIO_s_mem()), NULL);
  SSL_get_rbio(tls->ssl)->flags = BIO_FLAGS_WRITE;
  ret = tor_tls_write(tls, buf, 10);
  tt_int_op(ret, OP_EQ, TOR_TLS_WANTWRITE);

 done:
  teardown_capture_of_logs();
  BIO_free(tls->ssl->rbio);
  tor_free(tls->ssl);
  tor_free(tls);
  tor_free(method);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static int fixed_ssl_accept_result;
static int fixed_ssl_connect_result;

static int
setting_error_ssl_accept(SSL *ssl)
{
  (void)ssl;
  ERR_put_error(ERR_LIB_BN, 2, -1, "somewhere.c", 99);
  ERR_put_error(ERR_LIB_SYS, 2, -1, "somewhere.c", 99);
  return fixed_ssl_accept_result;
}

static int
setting_error_ssl_connect(SSL *ssl)
{
  (void)ssl;
  ERR_put_error(ERR_LIB_BN, 2, -1, "somewhere.c", 99);
  ERR_put_error(ERR_LIB_SYS, 2, -1, "somewhere.c", 99);
  return fixed_ssl_connect_result;
}

static int
fixed_ssl_accept(SSL *ssl)
{
  (void) ssl;
  return fixed_ssl_accept_result;
}

static void
test_tortls_handshake(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  SSL_CTX *ctx;
  SSL_METHOD *method = give_me_a_test_method();
  setup_capture_of_logs(LOG_INFO);

  SSL_library_init();
  SSL_load_error_strings();

  ctx = SSL_CTX_new(TLSv1_method());

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = SSL_new(ctx);
  tls->state = TOR_TLS_ST_HANDSHAKE;

  ret = tor_tls_handshake(tls);
  tt_int_op(ret, OP_EQ, -9);

  tls->isServer = 1;
  tls->state = TOR_TLS_ST_HANDSHAKE;
  ret = tor_tls_handshake(tls);
  tt_int_op(ret, OP_EQ, -9);

  tls->ssl->method = method;
  method->ssl_accept = fixed_ssl_accept;
  fixed_ssl_accept_result = 2;
  ERR_clear_error();
  tls->state = TOR_TLS_ST_HANDSHAKE;
  ret = tor_tls_handshake(tls);
  tt_int_op(tls->state, OP_EQ, TOR_TLS_ST_OPEN);

  method->ssl_accept = setting_error_ssl_accept;
  fixed_ssl_accept_result = 1;
  ERR_clear_error();
  mock_clean_saved_logs();
  tls->state = TOR_TLS_ST_HANDSHAKE;
  ret = tor_tls_handshake(tls);
  tt_int_op(ret, OP_EQ, TOR_TLS_ERROR_MISC);
  expect_log_entry();
  /* This fails on jessie.  Investigate why! */
#if 0
  expect_log_msg("TLS error while handshaking: (null) (in bignum routines:"
            "(null):SSLv3 write client hello B)\n");
  expect_log_msg("TLS error while handshaking: (null) (in system library:"
            "connect:SSLv3 write client hello B)\n");
#endif /* 0 */
  expect_log_severity(LOG_INFO);

  tls->isServer = 0;
  method->ssl_connect = setting_error_ssl_connect;
  fixed_ssl_connect_result = 1;
  ERR_clear_error();
  mock_clean_saved_logs();
  tls->state = TOR_TLS_ST_HANDSHAKE;
  ret = tor_tls_handshake(tls);
  tt_int_op(ret, OP_EQ, TOR_TLS_ERROR_MISC);
  expect_log_entry();
#if 0
  /* See above */
  expect_log_msg("TLS error while handshaking: "
            "(null) (in bignum routines:(null):SSLv3 write client hello B)\n");
  expect_log_msg("TLS error while handshaking: "
            "(null) (in system library:connect:SSLv3 write client hello B)\n");
#endif /* 0 */
  expect_log_severity(LOG_WARN);

 done:
  teardown_capture_of_logs();
  SSL_free(tls->ssl);
  SSL_CTX_free(ctx);
  tor_free(tls);
  tor_free(method);
}
#endif /* !defined(OPENSSL_OPAQUE) */

#ifndef OPENSSL_OPAQUE
static void
test_tortls_finish_handshake(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  SSL_CTX *ctx;
  SSL_METHOD *method = give_me_a_test_method();
  SSL_library_init();
  SSL_load_error_strings();

  X509 *c1 = read_cert_from(validCertString);
  SESS_CERT_local *sess = NULL;

  ctx = SSL_CTX_new(method);

  tls = tor_malloc_zero(sizeof(tor_tls_t));
  tls->ssl = SSL_new(ctx);
  tls->state = TOR_TLS_ST_OPEN;

  ret = tor_tls_finish_handshake(tls);
  tt_int_op(ret, OP_EQ, 0);

  tls->isServer = 1;
  tls->wasV2Handshake = 0;
  setup_full_capture_of_logs(LOG_WARN);
  ret = tor_tls_finish_handshake(tls);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tls->wasV2Handshake, OP_EQ, 1);
  expect_single_log_msg_containing("For some reason, wasV2Handshake didn't "
                                   "get set.");
  teardown_capture_of_logs();

  tls->wasV2Handshake = 1;
  ret = tor_tls_finish_handshake(tls);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tls->wasV2Handshake, OP_EQ, 1);

  tls->wasV2Handshake = 1;
  tls->ssl->session = SSL_SESSION_new();
  ret = tor_tls_finish_handshake(tls);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tls->wasV2Handshake, OP_EQ, 0);

  tls->isServer = 0;

  sess = tor_malloc_zero(sizeof(SESS_CERT_local));
  tls->ssl->session->sess_cert = (void *)sess;
  sess->cert_chain = sk_X509_new_null();
  sk_X509_push(sess->cert_chain, c1);
  tls->ssl->session->peer = c1;
  tls->wasV2Handshake = 0;
  ret = tor_tls_finish_handshake(tls);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tls->wasV2Handshake, OP_EQ, 1);

  method->num_ciphers = fake_num_ciphers;
  ret = tor_tls_finish_handshake(tls);
  tt_int_op(ret, OP_EQ, -9);

 done:
  if (sess)
    sk_X509_free(sess->cert_chain);
  if (tls->ssl && tls->ssl->session) {
    tor_free(tls->ssl->session->sess_cert);
  }
  SSL_free(tls->ssl);
  tor_free(tls);
  SSL_CTX_free(ctx);
  tor_free(method);
  teardown_capture_of_logs();
}
#endif /* !defined(OPENSSL_OPAQUE) */

static int fixed_crypto_pk_new_result_index;
static crypto_pk_t *fixed_crypto_pk_new_result[5];

static crypto_pk_t *
fixed_crypto_pk_new(void)
{
  return fixed_crypto_pk_new_result[fixed_crypto_pk_new_result_index++];
}

#ifndef OPENSSL_OPAQUE
static int fixed_crypto_pk_generate_key_with_bits_result_index;
static int fixed_crypto_pk_generate_key_with_bits_result[5];
static int fixed_tor_tls_create_certificate_result_index;
static X509 *fixed_tor_tls_create_certificate_result[5];
static int fixed_tor_x509_cert_new_result_index;
static tor_x509_cert_t *fixed_tor_x509_cert_new_result[5];

static int
fixed_crypto_pk_generate_key_with_bits(crypto_pk_t *env, int bits)
{
  (void)env;
  (void)bits;
  return fixed_crypto_pk_generate_key_with_bits_result[
                    fixed_crypto_pk_generate_key_with_bits_result_index++];
}

static X509 *
fixed_tor_tls_create_certificate(crypto_pk_t *rsa,
                                 crypto_pk_t *rsa_sign,
                                 const char *cname,
                                 const char *cname_sign,
                                 unsigned int cert_lifetime)
{
  (void)rsa;
  (void)rsa_sign;
  (void)cname;
  (void)cname_sign;
  (void)cert_lifetime;
  X509 *result = fixed_tor_tls_create_certificate_result[
                             fixed_tor_tls_create_certificate_result_index++];
  if (result)
    return X509_dup(result);
  else
    return NULL;
}

static void
fixed_tor_tls_create_certificate_results_free(void)
{
  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(fixed_tor_tls_create_certificate_result); ++i) {
    X509 *cert = fixed_tor_tls_create_certificate_result[i];
    if (cert)
      X509_free(cert);
    fixed_tor_tls_create_certificate_result[i] = NULL;
  }
}

static void
fixed_tor_x509_cert_new_results_free(void)
{
  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(fixed_tor_x509_cert_new_result); ++i) {
    tor_x509_cert_free(fixed_tor_x509_cert_new_result[i]);
  }
}

static tor_x509_cert_t *
fixed_tor_x509_cert_new(tor_x509_cert_impl_t *x509_cert)
{
  (void) x509_cert;
  tor_x509_cert_t **certp =
    &fixed_tor_x509_cert_new_result[fixed_tor_x509_cert_new_result_index++];
  tor_x509_cert_t *cert = *certp;
  *certp = NULL;
  return cert;
}

static void
test_tortls_context_new(void *ignored)
{
  (void)ignored;
  tor_tls_context_t *ret;
  crypto_pk_t *pk1, *pk2, *pk3, *pk4, *pk5, *pk6, *pk7, *pk8, *pk9, *pk10,
    *pk11, *pk12, *pk13, *pk14, *pk15, *pk16, *pk17, *pk18;

  pk1 = crypto_pk_new();
  pk2 = crypto_pk_new();
  pk3 = crypto_pk_new();
  pk4 = crypto_pk_new();
  pk5 = crypto_pk_new();
  pk6 = crypto_pk_new();
  pk7 = crypto_pk_new();
  pk8 = crypto_pk_new();
  pk9 = crypto_pk_new();
  pk10 = crypto_pk_new();
  pk11 = crypto_pk_new();
  pk12 = crypto_pk_new();
  pk13 = crypto_pk_new();
  pk14 = crypto_pk_new();
  pk15 = crypto_pk_new();
  pk16 = crypto_pk_new();
  pk17 = crypto_pk_new();
  pk18 = crypto_pk_new();

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = NULL;
  MOCK(crypto_pk_new, fixed_crypto_pk_new);
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);

  /* note: we already override this in testing_common.c, so we
   * run this unit test in a subprocess. */
  MOCK(crypto_pk_generate_key_with_bits,
       fixed_crypto_pk_generate_key_with_bits);
  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk1;
  fixed_crypto_pk_new_result[1] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result[0] = -1;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk2;
  fixed_crypto_pk_new_result[1] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result[0] = 0;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk3;
  fixed_crypto_pk_new_result[1] = pk4;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result[0] = 0;
  fixed_crypto_pk_generate_key_with_bits_result[1] = -1;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);

  MOCK(tor_tls_create_certificate, fixed_tor_tls_create_certificate);

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk5;
  fixed_crypto_pk_new_result[1] = pk6;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  fixed_crypto_pk_generate_key_with_bits_result[1] = 0;
  fixed_tor_tls_create_certificate_result_index = 0;
  fixed_tor_tls_create_certificate_result[0] = NULL;
  fixed_tor_tls_create_certificate_result[1] = X509_new();
  fixed_tor_tls_create_certificate_result[2] = X509_new();
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);
  fixed_tor_tls_create_certificate_results_free();

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk7;
  fixed_crypto_pk_new_result[1] = pk8;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  fixed_tor_tls_create_certificate_result_index = 0;
  fixed_tor_tls_create_certificate_result[0] = X509_new();
  fixed_tor_tls_create_certificate_result[1] = NULL;
  fixed_tor_tls_create_certificate_result[2] = X509_new();
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);
  fixed_tor_tls_create_certificate_results_free();

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk9;
  fixed_crypto_pk_new_result[1] = pk10;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  fixed_tor_tls_create_certificate_result_index = 0;
  fixed_tor_tls_create_certificate_result[0] = X509_new();
  fixed_tor_tls_create_certificate_result[1] = X509_new();
  fixed_tor_tls_create_certificate_result[2] = NULL;
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);
  fixed_tor_tls_create_certificate_results_free();

  MOCK(tor_x509_cert_new, fixed_tor_x509_cert_new);
  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk11;
  fixed_crypto_pk_new_result[1] = pk12;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  fixed_tor_tls_create_certificate_result_index = 0;
  fixed_tor_tls_create_certificate_result[0] = X509_new();
  fixed_tor_tls_create_certificate_result[1] = X509_new();
  fixed_tor_tls_create_certificate_result[2] = X509_new();
  fixed_tor_x509_cert_new_result_index = 0;
  fixed_tor_x509_cert_new_result[0] = NULL;
  fixed_tor_x509_cert_new_result[1] = NULL;
  fixed_tor_x509_cert_new_result[2] = NULL;
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);
  fixed_tor_tls_create_certificate_results_free();

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk13;
  fixed_crypto_pk_new_result[1] = pk14;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  fixed_tor_tls_create_certificate_result_index = 0;
  fixed_tor_tls_create_certificate_result[0] = X509_new();
  fixed_tor_tls_create_certificate_result[1] = X509_new();
  fixed_tor_tls_create_certificate_result[2] = X509_new();
  fixed_tor_x509_cert_new_result_index = 0;
  fixed_tor_x509_cert_new_result[0] = tor_malloc_zero(sizeof(tor_x509_cert_t));
  fixed_tor_x509_cert_new_result[1] = NULL;
  fixed_tor_x509_cert_new_result[2] = NULL;
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);
  fixed_tor_tls_create_certificate_results_free();
  fixed_tor_x509_cert_new_results_free();

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk15;
  fixed_crypto_pk_new_result[1] = pk16;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  fixed_tor_tls_create_certificate_result_index = 0;
  fixed_tor_tls_create_certificate_result[0] = X509_new();
  fixed_tor_tls_create_certificate_result[1] = X509_new();
  fixed_tor_tls_create_certificate_result[2] = X509_new();
  fixed_tor_x509_cert_new_result_index = 0;
  fixed_tor_x509_cert_new_result[0] = tor_malloc_zero(sizeof(tor_x509_cert_t));
  fixed_tor_x509_cert_new_result[1] = tor_malloc_zero(sizeof(tor_x509_cert_t));
  fixed_tor_x509_cert_new_result[2] = NULL;
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);
  fixed_tor_tls_create_certificate_results_free();
  fixed_tor_x509_cert_new_results_free();

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = pk17;
  fixed_crypto_pk_new_result[1] = pk18;
  fixed_crypto_pk_new_result[2] = NULL;
  fixed_crypto_pk_generate_key_with_bits_result_index = 0;
  fixed_tor_tls_create_certificate_result_index = 0;
  fixed_tor_tls_create_certificate_result[0] = X509_new();
  fixed_tor_tls_create_certificate_result[1] = X509_new();
  fixed_tor_tls_create_certificate_result[2] = X509_new();
  fixed_tor_x509_cert_new_result_index = 0;
  fixed_tor_x509_cert_new_result[0] = tor_malloc_zero(sizeof(tor_x509_cert_t));
  fixed_tor_x509_cert_new_result[1] = tor_malloc_zero(sizeof(tor_x509_cert_t));
  fixed_tor_x509_cert_new_result[2] = tor_malloc_zero(sizeof(tor_x509_cert_t));
  ret = tor_tls_context_new(NULL, 0, 0, 0);
  tt_assert(!ret);

 done:
  fixed_tor_tls_create_certificate_results_free();
  fixed_tor_x509_cert_new_results_free();
  UNMOCK(tor_x509_cert_new);
  UNMOCK(tor_tls_create_certificate);
  UNMOCK(crypto_pk_generate_key_with_bits);
  UNMOCK(crypto_pk_new);
}
#endif /* !defined(OPENSSL_OPAQUE) */

static int fixed_crypto_pk_get_evp_pkey_result_index = 0;
static EVP_PKEY *fixed_crypto_pk_get_evp_pkey_result[5];

static EVP_PKEY *
fixed_crypto_pk_get_evp_pkey_(crypto_pk_t *env, int private)
{
  (void) env;
  (void) private;
  return fixed_crypto_pk_get_evp_pkey_result[
                               fixed_crypto_pk_get_evp_pkey_result_index++];
}

static void
test_tortls_create_certificate(void *ignored)
{
  (void)ignored;
  X509 *ret;
  crypto_pk_t *pk1, *pk2;

  pk1 = crypto_pk_new();
  pk2 = crypto_pk_new();

  MOCK(crypto_pk_get_openssl_evp_pkey_, fixed_crypto_pk_get_evp_pkey_);
  fixed_crypto_pk_get_evp_pkey_result_index = 0;
  fixed_crypto_pk_get_evp_pkey_result[0] = NULL;
  ret = tor_tls_create_certificate(pk1, pk2, "hello", "hello2", 1);
  tt_assert(!ret);

  fixed_crypto_pk_get_evp_pkey_result_index = 0;
  fixed_crypto_pk_get_evp_pkey_result[0] = EVP_PKEY_new();
  fixed_crypto_pk_get_evp_pkey_result[1] = NULL;
  ret = tor_tls_create_certificate(pk1, pk2, "hello", "hello2", 1);
  tt_assert(!ret);

  fixed_crypto_pk_get_evp_pkey_result_index = 0;
  fixed_crypto_pk_get_evp_pkey_result[0] = EVP_PKEY_new();
  fixed_crypto_pk_get_evp_pkey_result[1] = EVP_PKEY_new();
  ret = tor_tls_create_certificate(pk1, pk2, "hello", "hello2", 1);
  tt_assert(!ret);

 done:
  UNMOCK(crypto_pk_get_openssl_evp_pkey_);
  crypto_pk_free(pk1);
  crypto_pk_free(pk2);
}

static void
test_tortls_cert_new(void *ignored)
{
  (void)ignored;
  tor_x509_cert_t *ret;
  X509 *cert = read_cert_from(validCertString);

  ret = tor_x509_cert_new(NULL);
  tt_assert(!ret);

  ret = tor_x509_cert_new(cert);
  tt_assert(ret);
  tor_x509_cert_free(ret);
  ret = NULL;

#if 0
  cert = read_cert_from(validCertString);
  /* XXX this doesn't do what you think: it alters a copy of the pubkey. */
  X509_get_pubkey(cert)->type = EVP_PKEY_DSA;
  ret = tor_x509_cert_new(cert);
  tt_assert(ret);
#endif /* 0 */

#ifndef OPENSSL_OPAQUE
  cert = read_cert_from(validCertString);
  X509_CINF_free(cert->cert_info);
  cert->cert_info = NULL;
  ret = tor_x509_cert_new(cert);
  tt_assert(ret);
#endif /* !defined(OPENSSL_OPAQUE) */

 done:
  tor_x509_cert_free(ret);
}

static void
test_tortls_cert_is_valid(void *ignored)
{
  (void)ignored;
  int ret;
  tor_x509_cert_t *cert = NULL, *scert = NULL;
  time_t now = cert_strings_valid_at;

  scert = tor_malloc_zero(sizeof(tor_x509_cert_t));
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 0);
  tt_int_op(ret, OP_EQ, 0);

  cert = tor_malloc_zero(sizeof(tor_x509_cert_t));
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 0);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(scert);
  tor_free(cert);

  cert = tor_x509_cert_new(read_cert_from(validCertString));
  scert = tor_x509_cert_new(read_cert_from(caCertString));
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 0);
  tt_int_op(ret, OP_EQ, 1);

#ifndef OPENSSL_OPAQUE
  tor_x509_cert_free(cert);
  tor_x509_cert_free(scert);
  cert = tor_x509_cert_new(read_cert_from(validCertString));
  scert = tor_x509_cert_new(read_cert_from(caCertString));
  ASN1_TIME_free(cert->cert->cert_info->validity->notAfter);
  cert->cert->cert_info->validity->notAfter =
    ASN1_TIME_set(NULL, time(NULL)-1000000);
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 0);
  tt_int_op(ret, OP_EQ, 0);

  tor_x509_cert_free(cert);
  tor_x509_cert_free(scert);
  cert = tor_x509_cert_new(read_cert_from(validCertString));
  scert = tor_x509_cert_new(read_cert_from(caCertString));
  X509_PUBKEY_free(cert->cert->cert_info->key);
  cert->cert->cert_info->key = NULL;
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 1);
  tt_int_op(ret, OP_EQ, 0);
#endif /* !defined(OPENSSL_OPAQUE) */

#if 0
  tor_x509_cert_free(cert);
  tor_x509_cert_free(scert);
  cert = tor_x509_cert_new(read_cert_from(validCertString));
  scert = tor_x509_cert_new(read_cert_from(caCertString));
  /* This doesn't actually change the key in the cert. XXXXXX */
  BN_one(EVP_PKEY_get1_RSA(X509_get_pubkey(cert->cert))->n);
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 1);
  tt_int_op(ret, OP_EQ, 0);

  tor_x509_cert_free(cert);
  tor_x509_cert_free(scert);
  cert = tor_x509_cert_new(read_cert_from(validCertString));
  scert = tor_x509_cert_new(read_cert_from(caCertString));
  /* This doesn't actually change the key in the cert. XXXXXX */
  X509_get_pubkey(cert->cert)->type = EVP_PKEY_EC;
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 1);
  tt_int_op(ret, OP_EQ, 0);

  tor_x509_cert_free(cert);
  tor_x509_cert_free(scert);
  cert = tor_x509_cert_new(read_cert_from(validCertString));
  scert = tor_x509_cert_new(read_cert_from(caCertString));
  /* This doesn't actually change the key in the cert. XXXXXX */
  X509_get_pubkey(cert->cert)->type = EVP_PKEY_EC;
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 0);
  tt_int_op(ret, OP_EQ, 1);

  tor_x509_cert_free(cert);
  tor_x509_cert_free(scert);
  cert = tor_x509_cert_new(read_cert_from(validCertString));
  scert = tor_x509_cert_new(read_cert_from(caCertString));
  /* This doesn't actually change the key in the cert. XXXXXX */
  X509_get_pubkey(cert->cert)->type = EVP_PKEY_EC;
  X509_get_pubkey(cert->cert)->ameth = NULL;
  ret = tor_tls_cert_is_valid(LOG_WARN, cert, scert, now, 0);
  tt_int_op(ret, OP_EQ, 0);
#endif /* 0 */

 done:
  tor_x509_cert_free(cert);
  tor_x509_cert_free(scert);
}

static void
test_tortls_context_init_one(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_context_t *old = NULL;

  MOCK(crypto_pk_new, fixed_crypto_pk_new);

  fixed_crypto_pk_new_result_index = 0;
  fixed_crypto_pk_new_result[0] = NULL;
  ret = tor_tls_context_init_one(&old, NULL, 0, 0, 0);
  tt_int_op(ret, OP_EQ, -1);

 done:
  UNMOCK(crypto_pk_new);
}

#define LOCAL_TEST_CASE(name, flags)                    \
  { #name, test_tortls_##name, (flags|TT_FORK), NULL, NULL }

#ifdef OPENSSL_OPAQUE
#define INTRUSIVE_TEST_CASE(name, flags)        \
  { #name, NULL, TT_SKIP, NULL, NULL }
#else
#define INTRUSIVE_TEST_CASE(name, flags) LOCAL_TEST_CASE(name, flags)
#endif /* defined(OPENSSL_OPAQUE) */

struct testcase_t tortls_openssl_tests[] = {
  LOCAL_TEST_CASE(tor_tls_new, TT_FORK),
  LOCAL_TEST_CASE(get_state_description, TT_FORK),
  LOCAL_TEST_CASE(get_by_ssl, TT_FORK),
  LOCAL_TEST_CASE(allocate_tor_tls_object_ex_data_index, TT_FORK),
  LOCAL_TEST_CASE(log_one_error, TT_FORK),
  INTRUSIVE_TEST_CASE(get_error, TT_FORK),
  LOCAL_TEST_CASE(always_accept_verify_cb, 0),
  INTRUSIVE_TEST_CASE(x509_cert_free, 0),
  INTRUSIVE_TEST_CASE(cert_get_key, 0),
  INTRUSIVE_TEST_CASE(get_ciphersuite_name, 0),
  INTRUSIVE_TEST_CASE(classify_client_ciphers, 0),
  INTRUSIVE_TEST_CASE(get_pending_bytes, 0),
  INTRUSIVE_TEST_CASE(get_buffer_sizes, 0),
  INTRUSIVE_TEST_CASE(get_peer_cert, 0),
  INTRUSIVE_TEST_CASE(peer_has_cert, 0),
  INTRUSIVE_TEST_CASE(finish_handshake, 0),
  INTRUSIVE_TEST_CASE(handshake, 0),
  INTRUSIVE_TEST_CASE(write, 0),
  INTRUSIVE_TEST_CASE(read, 0),
  INTRUSIVE_TEST_CASE(server_info_callback, 0),
  LOCAL_TEST_CASE(get_write_overhead_ratio, TT_FORK),
  LOCAL_TEST_CASE(is_server, 0),
  INTRUSIVE_TEST_CASE(block_renegotiation, 0),
  INTRUSIVE_TEST_CASE(unblock_renegotiation, 0),
  INTRUSIVE_TEST_CASE(set_renegotiate_callback, 0),
  LOCAL_TEST_CASE(set_logged_address, 0),
  INTRUSIVE_TEST_CASE(session_secret_cb, 0),
  INTRUSIVE_TEST_CASE(debug_state_callback, 0),
  INTRUSIVE_TEST_CASE(context_new, TT_FORK /* redundant */),
  LOCAL_TEST_CASE(create_certificate, 0),
  LOCAL_TEST_CASE(cert_new, 0),
  LOCAL_TEST_CASE(cert_is_valid, 0),
  LOCAL_TEST_CASE(context_init_one, 0),
  END_OF_TESTCASES
};
