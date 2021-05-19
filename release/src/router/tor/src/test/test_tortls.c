/* Copyright (c) 2010-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define TORTLS_PRIVATE
#define TOR_X509_PRIVATE
#define LOG_PRIVATE
#include "orconfig.h"

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <math.h>
#include <stddef.h>

#include "lib/cc/compat_compiler.h"

#include "core/or/or.h"
#include "lib/log/log.h"
#include "app/config/config.h"
#include "lib/crypt_ops/compat_openssl.h"
#include "lib/tls/x509.h"
#include "lib/tls/x509_internal.h"
#include "lib/tls/tortls.h"
#include "lib/tls/tortls_st.h"
#include "lib/tls/tortls_internal.h"
#include "lib/encoding/pem.h"
#include "app/config/or_state_st.h"

#include "test/test.h"
#include "test/log_test_helpers.h"
#include "test/test_tortls.h"

#include "tinytest.h"

const char* notCompletelyValidCertString =
  "-----BEGIN CERTIFICATE-----\n"
  "MIICVjCCAb8CAg37MA0GCSqGSIb3DQEBBQUAMIGbMQswCQYDVQQGEwJKUDEOMAwG\n"
  "A1UECBMFVG9reW8xEDAOBgNVBAcTB0NodW8ta3UxETAPBgNVBAoTCEZyYW5rNERE\n"
  "MRgwFgYDVQQLEw9XZWJDZXJ0IFN1cHBvcnQxGDAWBgNVBAMTD0ZyYW5rNEREIFdl\n"
  "YiBDQTEjMCEGCSqGSIb3DQEJARYUc3VwcG9ydEBmcmFuazRkZC5jb20wHhcNMTIw\n"
  "ODIyMDUyNzIzWhcNMTcwODIxMDUyNzIzWjBKMQswCQYDVQQGEwJKUDEOMAwGA1UE\n"
  "CAwFVG9reW8xETAPBgNVBAoMCEZyYW5rNEREMRgwFgYDVQQDDA93d3cuZXhhbXBs\n"
  "ZS5jb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAMYBBrx5PlP0WNI/ZdzD\n"
  "+6Pktmurn+F2kQYbtc7XQh8/LTBvCo+P6iZoLEmUA9e7EXLRxgU1CVqeAi7QcAn9\n"
  "MwBlc8ksFJHB0rtf9pmf8Oza9E0Bynlq/4/Kb1x+d+AyhL7oK9tQwB24uHOueHi1\n"
  "C/iVv8CSWKiYe6hzN1txYe8rAgMBAAEwDQYJKoZIhvcNAQEFBQADgYEAASPdjigJ\n"
  "kXCqKWpnZ/Oc75EUcMi6HztaW8abUMlYXPIgkV2F7YanHOB7K4f7OOLjiz8DTPFf\n"
  "jC9UeuErhaA/zzWi8ewMTFZW/WshOrm3fNvcMrMLKtH534JKvcdMg6qIdjTFINIr\n"
  "evnAhf0cwULaebn+lMs8Pdl7y37+sfluVok=\n"
  "-----END CERTIFICATE-----\n";

const char* validCertString = "-----BEGIN CERTIFICATE-----\n"
  "MIIDpTCCAY0CAg3+MA0GCSqGSIb3DQEBBQUAMF4xCzAJBgNVBAYTAlVTMREwDwYD\n"
  "VQQIDAhJbGxpbm9pczEQMA4GA1UEBwwHQ2hpY2FnbzEUMBIGA1UECgwLVG9yIFRl\n"
  "c3RpbmcxFDASBgNVBAMMC1RvciBUZXN0aW5nMB4XDTE1MDkwNjEzMzk1OVoXDTQz\n"
  "MDEyMjEzMzk1OVowVjELMAkGA1UEBhMCVVMxEDAOBgNVBAcMB0NoaWNhZ28xFDAS\n"
  "BgNVBAoMC1RvciBUZXN0aW5nMR8wHQYDVQQDDBZ0ZXN0aW5nLnRvcnByb2plY3Qu\n"
  "b3JnMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDoT6uyVVhWyOF3wkHjjYbd\n"
  "nKaykyRv4JVtKQdZ4OpEErmX1zw4MmyzpQNV6iR4bQnWiyLfzyVJMZDIC/WILBfX\n"
  "w2Pza/yuLgUvDc3twMuhOACzOQVO8PrEF/aVv2+hbCCy2udXvKhnYn+CCXl3ozc8\n"
  "XcKYvujTXDyvGWY3xwAjlQIDAQABMA0GCSqGSIb3DQEBBQUAA4ICAQCUvnhzQWuQ\n"
  "MrN+pERkE+zcTI/9dGS90rUMMLgu8VDNqTa0TUQh8uO0EQ6uDvI8Js6e8tgwS0BR\n"
  "UBahqb7ZHv+rejGCBr5OudqD+x4STiiuPNJVs86JTLN8SpM9CHjIBH5WCCN2KOy3\n"
  "mevNoRcRRyYJzSFULCunIK6FGulszigMYGscrO4oiTkZiHPh9KvWT40IMiHfL+Lw\n"
  "EtEWiLex6064LcA2YQ1AMuSZyCexks63lcfaFmQbkYOKqXa1oLkIRuDsOaSVjTfe\n"
  "vec+X6jvf12cFTKS5WIeqkKF2Irt+dJoiHEGTe5RscUMN/f+gqHPzfFz5dR23sxo\n"
  "g+HC6MZHlFkLAOx3wW6epPS8A/m1mw3zMPoTnb2U2YYt8T0dJMMlUn/7Y1sEAa+a\n"
  "dSTMaeUf6VnJ//11m454EZl1to9Z7oJOgqmFffSrdD4BGIWe8f7hhW6L1Enmqe/J\n"
  "BKL3wbzZh80O1W0bndAwhnEEhlzneFY84cbBo9pmVxpODHkUcStpr5Z7pBDrcL21\n"
  "Ss/aB/1YrsVXhdvJdOGxl3Mnl9dUY57CympLGlT8f0pPS6GAKOelECOhFMHmJd8L\n"
  "dj3XQSmKtYHevZ6IvuMXSlB/fJvSjSlkCuLo5+kJoaqPuRu+i/S1qxeRy3CBwmnE\n"
  "LdSNdcX4N79GQJ996PA8+mUCQG7YRtK+WA==\n"
  "-----END CERTIFICATE-----\n";

const char* caCertString = "-----BEGIN CERTIFICATE-----\n"
  "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\n"
  "-----END CERTIFICATE-----\n";

static tor_x509_cert_t *fixed_x509_cert = NULL;
static tor_x509_cert_t *
get_peer_cert_mock_return_fixed(tor_tls_t *tls)
{
  (void)tls;
  if (fixed_x509_cert)
    return tor_x509_cert_dup(fixed_x509_cert);
  else
    return NULL;
}

tor_x509_cert_impl_t *
read_cert_from(const char *str)
{
  size_t len = strlen(str);
  uint8_t *raw_cert = tor_malloc(len);
  ssize_t true_len = pem_decode(raw_cert, len, str, len, "CERTIFICATE");
  if (true_len < 0) {
    tor_free(raw_cert);
    return NULL;
  }
  tor_x509_cert_t *cert = tor_x509_cert_decode(raw_cert, true_len);
  tor_free(raw_cert);
  if (! cert) {
    return NULL;
  }
  tor_x509_cert_impl_t *res = tor_x509_cert_impl_dup_(cert->cert);
  tor_x509_cert_free(cert);
  return res;
}

static tor_x509_cert_impl_t *
  fixed_try_to_extract_certs_from_tls_cert_out_result = NULL;
static tor_x509_cert_impl_t *
  fixed_try_to_extract_certs_from_tls_id_cert_out_result = NULL;

static void
fixed_try_to_extract_certs_from_tls(int severity, tor_tls_t *tls,
                                    tor_x509_cert_impl_t **cert_out,
                                    tor_x509_cert_impl_t **id_cert_out)
{
  (void) severity;
  (void) tls;
  *cert_out = tor_x509_cert_impl_dup_(
                      fixed_try_to_extract_certs_from_tls_cert_out_result);
  *id_cert_out =  tor_x509_cert_impl_dup_(
                      fixed_try_to_extract_certs_from_tls_id_cert_out_result);
}

static void
test_tortls_errno_to_tls_error(void *data)
{
  (void) data;
  tt_int_op(tor_errno_to_tls_error(SOCK_ERRNO(ECONNRESET)),OP_EQ,
            TOR_TLS_ERROR_CONNRESET);
  tt_int_op(tor_errno_to_tls_error(SOCK_ERRNO(ETIMEDOUT)),OP_EQ,
            TOR_TLS_ERROR_TIMEOUT);
  tt_int_op(tor_errno_to_tls_error(SOCK_ERRNO(EHOSTUNREACH)),OP_EQ,
            TOR_TLS_ERROR_NO_ROUTE);
  tt_int_op(tor_errno_to_tls_error(SOCK_ERRNO(ENETUNREACH)),OP_EQ,
            TOR_TLS_ERROR_NO_ROUTE);
  tt_int_op(tor_errno_to_tls_error(SOCK_ERRNO(ECONNREFUSED)),OP_EQ,
            TOR_TLS_ERROR_CONNREFUSED);
  tt_int_op(tor_errno_to_tls_error(0),OP_EQ,TOR_TLS_ERROR_MISC);
 done:
  (void)1;
}

static void
test_tortls_err_to_string(void *data)
{
  (void) data;
  tt_str_op(tor_tls_err_to_string(1),OP_EQ,"[Not an error.]");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_ERROR_MISC),OP_EQ,"misc error");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_ERROR_IO),OP_EQ,"unexpected close");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_ERROR_CONNREFUSED),OP_EQ,
            "connection refused");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_ERROR_CONNRESET),OP_EQ,
            "connection reset");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_ERROR_NO_ROUTE),OP_EQ,
            "host unreachable");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_ERROR_TIMEOUT),OP_EQ,
            "connection timed out");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_CLOSE),OP_EQ,"closed");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_WANTREAD),OP_EQ,"want to read");
  tt_str_op(tor_tls_err_to_string(TOR_TLS_WANTWRITE),OP_EQ,"want to write");
  tt_str_op(tor_tls_err_to_string(-100),OP_EQ,"(unknown error code)");
 done:
  (void)1;
}

#ifdef ENABLE_OPENSSL
static int
mock_tls_cert_matches_key(const tor_tls_t *tls, const tor_x509_cert_t *cert)
{
  (void) tls;
  (void) cert; // XXXX look at this.
  return 1;
}

static void
test_tortls_tor_tls_get_error(void *data)
{
  (void) data;
  MOCK(tor_tls_cert_matches_key, mock_tls_cert_matches_key);
  crypto_pk_t *key1 = NULL, *key2 = NULL;
  key1 = pk_generate(2);
  key2 = pk_generate(3);

  tor_tls_t *tls = NULL;
  tt_int_op(tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                                 key1, key2, 86400), OP_EQ, 0);
  tls = tor_tls_new(-1, 0);
  setup_capture_of_logs(LOG_WARN);
  tor_tls_get_error(tls, 0, 0,
                    (const char *)"in unit test", LOG_WARN, LD_GENERAL);
  expect_single_log_msg_containing("unexpected close while in unit test");

 done:
  UNMOCK(tor_tls_cert_matches_key);
  UNMOCK(logv);
  crypto_pk_free(key1);
  crypto_pk_free(key2);
  tor_tls_free(tls);
}
#endif /* defined(ENABLE_OPENSSL) */

static void
test_tortls_x509_cert_get_id_digests(void *ignored)
{
  (void)ignored;
  tor_x509_cert_t *cert;
  common_digests_t *d;
  const common_digests_t *res;
  cert = tor_malloc_zero(sizeof(tor_x509_cert_t));
  d = tor_malloc_zero(sizeof(common_digests_t));
  d->d[0][0] = 42;

  res = tor_x509_cert_get_id_digests(cert);
  tt_assert(!res);

  cert->pkey_digests_set = 1;
  cert->pkey_digests = *d;
  res = tor_x509_cert_get_id_digests(cert);
  tt_assert(res);
  tt_int_op(res->d[0][0], OP_EQ, 42);

 done:
  tor_free(cert);
  tor_free(d);
}

static void
test_tortls_get_my_certs(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_context_t *ctx;
  const tor_x509_cert_t *link_cert_out = NULL;
  const tor_x509_cert_t *id_cert_out = NULL;

  ctx = tor_malloc_zero(sizeof(tor_tls_context_t));

  client_tls_context = NULL;
  ret = tor_tls_get_my_certs(0, NULL, NULL);
  tt_int_op(ret, OP_EQ, -1);

  server_tls_context = NULL;
  ret = tor_tls_get_my_certs(1, NULL, NULL);
  tt_int_op(ret, OP_EQ, -1);

  client_tls_context = ctx;
  ret = tor_tls_get_my_certs(0, NULL, NULL);
  tt_int_op(ret, OP_EQ, 0);

  client_tls_context = ctx;
  ret = tor_tls_get_my_certs(0, &link_cert_out, &id_cert_out);
  tt_int_op(ret, OP_EQ, 0);

  server_tls_context = ctx;
  ret = tor_tls_get_my_certs(1, &link_cert_out, &id_cert_out);
  tt_int_op(ret, OP_EQ, 0);

 done:
  (void)1;
}

#ifdef ENABLE_OPENSSL
static void
test_tortls_get_forced_write_size(void *ignored)
{
  (void)ignored;
  long ret;
  tor_tls_t *tls;

  tls = tor_malloc_zero(sizeof(tor_tls_t));

  tls->wantwrite_n = 43;
  ret = tor_tls_get_forced_write_size(tls);
  tt_int_op(ret, OP_EQ, 43);

 done:
  tor_free(tls);
}

static void
test_tortls_used_v1_handshake(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  tls = tor_malloc_zero(sizeof(tor_tls_t));

  // These tests assume both V2 handshake server and client are enabled
  tls->wasV2Handshake = 0;
  ret = tor_tls_used_v1_handshake(tls);
  tt_int_op(ret, OP_EQ, 1);

  tls->wasV2Handshake = 1;
  ret = tor_tls_used_v1_handshake(tls);
  tt_int_op(ret, OP_EQ, 0);

 done:
  tor_free(tls);
}

static void
test_tortls_server_got_renegotiate(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;

  tls = tor_malloc_zero(sizeof(tor_tls_t));

  tls->got_renegotiate = 1;
  ret = tor_tls_server_got_renegotiate(tls);
  tt_int_op(ret, OP_EQ, 1);

 done:
  tor_free(tls);
}
#endif /* defined(ENABLE_OPENSSL) */

static void
test_tortls_evaluate_ecgroup_for_tls(void *ignored)
{
  (void)ignored;
  int ret;

  ret = evaluate_ecgroup_for_tls(NULL);
  tt_int_op(ret, OP_EQ, 1);

  ret = evaluate_ecgroup_for_tls("foobar");
  tt_int_op(ret, OP_EQ, 0);

  ret = evaluate_ecgroup_for_tls("P256");
  tt_int_op(ret, OP_EQ, 1);

  ret = evaluate_ecgroup_for_tls("P224");
  //  tt_int_op(ret, OP_EQ, 1); This varies between machines
  tt_assert(ret == 0 || ret == 1);

 done:
  (void)0;
}

static void
test_tortls_double_init(void *arg)
{
  (void) arg;
  /* If we call tor_tls_context_init() a second time, nothing should go
   * wrong.
   */
  crypto_pk_t *pk1 = NULL, *pk2 = NULL;
  pk1 = pk_generate(2);
  pk2 = pk_generate(0);

  int r = tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                               pk1, pk2, 86400);
  tt_int_op(r, OP_EQ, 0);

  r = tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                               pk2, pk1, 86400);
  tt_int_op(r, OP_EQ, 0);
  /* For a public server context, these are the same */
  tt_ptr_op(tor_tls_context_get(0), OP_EQ, tor_tls_context_get(1));

 done:
  crypto_pk_free(pk1);
  crypto_pk_free(pk2);
}

static void
test_tortls_bridge_init(void *arg)
{
  (void)arg;
  crypto_pk_t *pk1 = NULL, *pk2 = NULL;
  pk1 = pk_generate(2);
  pk2 = pk_generate(0);

  /* If we pass in a server identity key but not the
     TOR_TLS_CTX_IS_PUBLIC_SERVER flag, we should get a bridge-style
     configuration, with two distinct contexts. */
  int r = tor_tls_context_init(0 /* flags */, pk1, pk2, 86400);

  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(tor_tls_context_get(0), OP_NE, tor_tls_context_get(1));
 done:
  crypto_pk_free(pk1);
  crypto_pk_free(pk2);
}

static void
test_tortls_address(void *arg)
{
  (void)arg;
  tor_tls_t *tls = NULL;
  crypto_pk_t *pk1=NULL, *pk2=NULL;
  pk1 = pk_generate(2);
  pk2 = pk_generate(0);

  int r = tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                               pk1, pk2, 86400);
  tt_int_op(r, OP_EQ, 0);

  tls = tor_tls_new(-1, 0);
  tls->state = TOR_TLS_ST_OPEN;
  tor_tls_set_logged_address(tls, "zombo.com");

  /* This write should fail, since the fd is -1. */
  setup_capture_of_logs(LOG_INFO);
  int n = tor_tls_write(tls, "welcome", 7);
  tt_int_op(n, OP_LT, 0);
  expect_log_msg_containing("with zombo.com");

 done:
  teardown_capture_of_logs();
  tor_tls_free(tls);
  crypto_pk_free(pk1);
  crypto_pk_free(pk2);
}

static void
test_tortls_is_server(void *arg)
{
  (void)arg;
  crypto_pk_t *pk1=NULL, *pk2=NULL;
  tor_tls_t *tls1=NULL, *tls2=NULL;
  pk1 = pk_generate(2);
  pk2 = pk_generate(0);

  int r = tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                               pk1, pk2, 86400);
  tt_int_op(r, OP_EQ, 0);
  tls1 = tor_tls_new(-1, 0);
  tls2 = tor_tls_new(-1, 1);

  tt_assert(! tor_tls_is_server(tls1));
  tt_assert(tor_tls_is_server(tls2));

 done:
  tor_tls_free(tls1);
  tor_tls_free(tls2);
  crypto_pk_free(pk1);
  crypto_pk_free(pk2);
}

static void
test_tortls_verify(void *ignored)
{
  (void)ignored;
  int ret;
  tor_tls_t *tls;
  crypto_pk_t *k = NULL;
  tor_x509_cert_impl_t *cert1 = NULL, *cert2 = NULL, *invalidCert = NULL,
    *validCert = NULL, *caCert = NULL;

  validCert = read_cert_from(validCertString);
  caCert = read_cert_from(caCertString);
  invalidCert = read_cert_from(notCompletelyValidCertString);

  tls = tor_malloc_zero(sizeof(tor_tls_t));

  MOCK(try_to_extract_certs_from_tls, fixed_try_to_extract_certs_from_tls);

  fixed_try_to_extract_certs_from_tls_cert_out_result = cert1;
  ret = tor_tls_verify(LOG_WARN, tls, &k);
  tt_int_op(ret, OP_EQ, -1);

  fixed_try_to_extract_certs_from_tls_id_cert_out_result = cert2;
  ret = tor_tls_verify(LOG_WARN, tls, &k);
  tt_int_op(ret, OP_EQ, -1);

  fixed_try_to_extract_certs_from_tls_cert_out_result = invalidCert;
  fixed_try_to_extract_certs_from_tls_id_cert_out_result = invalidCert;

  ret = tor_tls_verify(LOG_WARN, tls, &k);
  tt_int_op(ret, OP_EQ, -1);

  fixed_try_to_extract_certs_from_tls_cert_out_result = validCert;
  fixed_try_to_extract_certs_from_tls_id_cert_out_result = caCert;

  ret = tor_tls_verify(LOG_WARN, tls, &k);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(k);

 done:
  UNMOCK(try_to_extract_certs_from_tls);
  tor_x509_cert_impl_free(cert1);
  tor_x509_cert_impl_free(cert2);
  tor_x509_cert_impl_free(validCert);
  tor_x509_cert_impl_free(invalidCert);
  tor_x509_cert_impl_free(caCert);

  tor_free(tls);
  crypto_pk_free(k);
}

static void
test_tortls_cert_matches_key(void *ignored)
{
  (void)ignored;

  tor_x509_cert_impl_t *cert1 = NULL,
                       *cert2 = NULL,
                       *cert3 = NULL,
                       *cert4 = NULL;
  tor_x509_cert_t *c1 = NULL, *c2 = NULL, *c3 = NULL, *c4 = NULL;
  crypto_pk_t *k1 = NULL, *k2 = NULL, *k3 = NULL;

  k1 = pk_generate(1);
  k2 = pk_generate(2);
  k3 = pk_generate(3);

  cert1 = tor_tls_create_certificate(k1, k2, "A", "B", 1000);
  cert2 = tor_tls_create_certificate(k1, k3, "C", "D", 1000);
  cert3 = tor_tls_create_certificate(k2, k3, "C", "D", 1000);
  cert4 = tor_tls_create_certificate(k3, k2, "E", "F", 1000);

  tt_assert(cert1 && cert2 && cert3 && cert4);

  c1 = tor_x509_cert_new(cert1); cert1 = NULL;
  c2 = tor_x509_cert_new(cert2); cert2 = NULL;
  c3 = tor_x509_cert_new(cert3); cert3 = NULL;
  c4 = tor_x509_cert_new(cert4); cert4 = NULL;

  tt_assert(c1 && c2 && c3 && c4);

  MOCK(tor_tls_get_peer_cert, get_peer_cert_mock_return_fixed);

  fixed_x509_cert = NULL;
  /* If the peer has no certificate, it shouldn't match anything. */
  tt_assert(! tor_tls_cert_matches_key(NULL, c1));
  tt_assert(! tor_tls_cert_matches_key(NULL, c2));
  tt_assert(! tor_tls_cert_matches_key(NULL, c3));
  tt_assert(! tor_tls_cert_matches_key(NULL, c4));
  fixed_x509_cert = c1;
  /* If the peer has a certificate, it should match every cert with the same
   * subject key. */
  tt_assert(tor_tls_cert_matches_key(NULL, c1));
  tt_assert(tor_tls_cert_matches_key(NULL, c2));
  tt_assert(! tor_tls_cert_matches_key(NULL, c3));
  tt_assert(! tor_tls_cert_matches_key(NULL, c4));

 done:
  tor_x509_cert_free(c1);
  tor_x509_cert_free(c2);
  tor_x509_cert_free(c3);
  tor_x509_cert_free(c4);
  if (cert1) tor_x509_cert_impl_free(cert1);
  if (cert2) tor_x509_cert_impl_free(cert2);
  if (cert3) tor_x509_cert_impl_free(cert3);
  if (cert4) tor_x509_cert_impl_free(cert4);
  crypto_pk_free(k1);
  crypto_pk_free(k2);
  crypto_pk_free(k3);
  UNMOCK(tor_tls_get_peer_cert);
}

#define LOCAL_TEST_CASE(name, flags)                            \
  { #name, test_tortls_##name, (flags|TT_FORK), NULL, NULL }

struct testcase_t tortls_tests[] = {
  LOCAL_TEST_CASE(errno_to_tls_error, 0),
  LOCAL_TEST_CASE(err_to_string, 0),
  LOCAL_TEST_CASE(x509_cert_get_id_digests, 0),
  LOCAL_TEST_CASE(get_my_certs, TT_FORK),
#ifdef ENABLE_OPENSSL
  LOCAL_TEST_CASE(tor_tls_get_error, 0),
  LOCAL_TEST_CASE(get_forced_write_size, 0),
  LOCAL_TEST_CASE(used_v1_handshake, TT_FORK),
  LOCAL_TEST_CASE(server_got_renegotiate, 0),
#endif /* defined(ENABLE_OPENSSL) */
  LOCAL_TEST_CASE(evaluate_ecgroup_for_tls, 0),
  LOCAL_TEST_CASE(double_init, TT_FORK),
  LOCAL_TEST_CASE(address, TT_FORK),
  LOCAL_TEST_CASE(is_server, 0),
  LOCAL_TEST_CASE(bridge_init, TT_FORK),
  LOCAL_TEST_CASE(verify, TT_FORK),
  LOCAL_TEST_CASE(cert_matches_key, 0),
  END_OF_TESTCASES
};
