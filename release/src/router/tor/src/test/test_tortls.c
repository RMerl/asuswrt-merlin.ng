/* Copyright (c) 2010-2021, The Tor Project, Inc. */
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

// Tor www.torproject.org certificate.
// Fetched March 6 2025, ~1320 UTC.
const char* validCertString =
  "-----BEGIN CERTIFICATE-----\n"
  "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\n"
  "-----END CERTIFICATE-----\n";

// Let's encrypt certificate, used to sign validCertString.
// Also fetched March 6 2025, ~1320 UTC.
const char* caCertString =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIFBTCCAu2gAwIBAgIQS6hSk/eaL6JzBkuoBI110DANBgkqhkiG9w0BAQsFADBP\n"
  "MQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFy\n"
  "Y2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMTAeFw0yNDAzMTMwMDAwMDBa\n"
  "Fw0yNzAzMTIyMzU5NTlaMDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBF\n"
  "bmNyeXB0MQwwCgYDVQQDEwNSMTAwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
  "AoIBAQDPV+XmxFQS7bRH/sknWHZGUCiMHT6I3wWd1bUYKb3dtVq/+vbOo76vACFL\n"
  "YlpaPAEvxVgD9on/jhFD68G14BQHlo9vH9fnuoE5CXVlt8KvGFs3Jijno/QHK20a\n"
  "/6tYvJWuQP/py1fEtVt/eA0YYbwX51TGu0mRzW4Y0YCF7qZlNrx06rxQTOr8IfM4\n"
  "FpOUurDTazgGzRYSespSdcitdrLCnF2YRVxvYXvGLe48E1KGAdlX5jgc3421H5KR\n"
  "mudKHMxFqHJV8LDmowfs/acbZp4/SItxhHFYyTr6717yW0QrPHTnj7JHwQdqzZq3\n"
  "DZb3EoEmUVQK7GH29/Xi8orIlQ2NAgMBAAGjgfgwgfUwDgYDVR0PAQH/BAQDAgGG\n"
  "MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/\n"
  "AgEAMB0GA1UdDgQWBBS7vMNHpeS8qcbDpHIMEI2iNeHI6DAfBgNVHSMEGDAWgBR5\n"
  "tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKG\n"
  "Fmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYD\n"
  "VR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0B\n"
  "AQsFAAOCAgEAkrHnQTfreZ2B5s3iJeE6IOmQRJWjgVzPw139vaBw1bGWKCIL0vIo\n"
  "zwzn1OZDjCQiHcFCktEJr59L9MhwTyAWsVrdAfYf+B9haxQnsHKNY67u4s5Lzzfd\n"
  "u6PUzeetUK29v+PsPmI2cJkxp+iN3epi4hKu9ZzUPSwMqtCceb7qPVxEbpYxY1p9\n"
  "1n5PJKBLBX9eb9LU6l8zSxPWV7bK3lG4XaMJgnT9x3ies7msFtpKK5bDtotij/l0\n"
  "GaKeA97pb5uwD9KgWvaFXMIEt8jVTjLEvwRdvCn294GPDF08U8lAkIv7tghluaQh\n"
  "1QnlE4SEN4LOECj8dsIGJXpGUk3aU3KkJz9icKy+aUgA+2cP21uh6NcDIS3XyfaZ\n"
  "QjmDQ993ChII8SXWupQZVBiIpcWO4RqZk3lr7Bz5MUCwzDIA359e57SSq5CCkY0N\n"
  "4B6Vulk7LktfwrdGNVI5BsC9qqxSwSKgRJeZ9wygIaehbHFHFhcBaMDKpiZlBHyz\n"
  "rsnnlFXCb5s8HKn5LsUgGvB24L7sGNZP2CX7dhHov+YhD+jozLW2p9W4959Bz2Ei\n"
  "RmqDtmiXLnzqTpXbI+suyCsohKRg6Un0RC47+cpiVwHiXZAW+cn8eiNIjqbVgXLx\n"
  "KPpdzvvtTnOPlC7SQZSYmdunr3Bf9b77AiC/ZidstK36dRILKz7OA54=\n"
  "-----END CERTIFICATE-----\n";

// A time at which the certs above are valid.
const time_t cert_strings_valid_at = 1741267580;

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
#endif /* defined(ENABLE_OPENSSL) */
  LOCAL_TEST_CASE(evaluate_ecgroup_for_tls, 0),
  LOCAL_TEST_CASE(double_init, TT_FORK),
  LOCAL_TEST_CASE(address, TT_FORK),
  LOCAL_TEST_CASE(is_server, 0),
  LOCAL_TEST_CASE(bridge_init, TT_FORK),
  LOCAL_TEST_CASE(cert_matches_key, 0),
  END_OF_TESTCASES
};
