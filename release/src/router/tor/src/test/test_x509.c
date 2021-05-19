/* Copyright (c) 2010-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define TOR_X509_PRIVATE
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
#include "lib/tls/x509.h"
#include "lib/tls/x509_internal.h"
#include "app/config/or_state_st.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

#include "tinytest.h"

/* A mock replacement for crypto_digest that always fails. */
static int
mock_failing_digest(char *digest, const char *m, size_t len)
{
  (void)digest;
  (void)m;
  (void)len;
  return -1;
}

static void
test_x509_cert_new_failing_digest(void *arg)
{
  (void)arg;
  crypto_pk_t *pk1=NULL, *pk2=NULL;
  tor_x509_cert_impl_t *impl = NULL;
  tor_x509_cert_t *cert = NULL;
  pk1 = pk_generate(0);
  pk2 = pk_generate(1);

  impl = tor_tls_create_certificate(pk1, pk2, "hello", "world", 86400*100);
  tt_assert(impl);
  MOCK(crypto_digest, mock_failing_digest);

  setup_full_capture_of_logs(LOG_WARN);
  cert = tor_x509_cert_new(impl);
  tt_assert(!cert);
  expect_log_msg_containing("Couldn't wrap encoded X509 certificate");
  expect_log_msg_containing("unable to compute digests of certificate key");

 done:
  crypto_pk_free(pk1);
  crypto_pk_free(pk2);
  UNMOCK(crypto_digest);
  teardown_capture_of_logs();
}

static tor_x509_cert_t *
cert_from_der64(const char *der64)
{
  size_t der64len = strlen(der64);
  unsigned char *der = tor_malloc_zero(der64len);
  int derlen;
  tor_x509_cert_t *cert = NULL;

  derlen = base64_decode((char*)der, der64len,
                         der64, der64len);
  if (derlen >= 0)
    cert = tor_x509_cert_decode(der, derlen);
  tor_free(der);
  return cert;
}

static void
test_x509_consume_ec_cert(void *arg)
{
  (void)arg;
  /* This is a small self-signed EC certificate. */
  const char certificate[] =
    "MIIBEzCBugIJAIdl5svgOZ0OMAoGCCqGSM49BAMCMBIxEDAOBgNVBAMMB1Rlc3Rp\n"
    "bmcwHhcNMTgwODIzMTcyMzI1WhcNMTkwODIzMTcyMzI1WjASMRAwDgYDVQQDDAdU\n"
    "ZXN0aW5nMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAExMDpnRc0Btic3tIyCKNE\n"
    "iNY4j4gzcaYzS2sTYRoVK3RAukG29Qg6/c8e8XcnsSquU4fItYxDRbi/3nhYk4CP\n"
    "GDAKBggqhkjOPQQDAgNIADBFAiA0h1q03C2xlONUgAOonJLrlV1SUtMeKDxNsxsU\n"
    "+FSPvQIhAM7kY9Tlt0ELmyMnORPp1VJieXn/qhL5VoxGxSedTbny\n";
  const time_t now = 1535045321; /* when I'm writing this test. */
  tor_x509_cert_t *cert = cert_from_der64(certificate);
  crypto_pk_t *key = NULL;
  tt_assert(cert);

  key = tor_tls_cert_get_key(cert);
  tt_ptr_op(NULL, OP_EQ, key); // Can't get an RSA key out of an EC cert.

  /* It's a self-signed cert -- make sure it signed itself. */
  tt_assert(tor_tls_cert_is_valid(LOG_ERR, cert, cert, now, 0));

  /* Make sure we detect its key as non-RSA1024 */
  setup_capture_of_logs(LOG_INFO);
  tt_assert(! tor_tls_cert_is_valid(LOG_INFO, cert, cert, now, 1));
  expect_log_msg_containing("Key is not RSA1024");

 done:
  tor_x509_cert_free(cert);
  crypto_pk_free(key);
  teardown_capture_of_logs();
}

static void
test_x509_reject_tiny_keys(void *arg)
{
  (void)arg;
  const char *certificates[] = {
    /* Self-signed RSA512 */
   "MIIBXDCCAQYCCQDKikjJYZI5uDANBgkqhkiG9w0BAQsFADA1MRUwEwYDVQQHDAxE\n"
   "ZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29tcGFueSBMdGQwHhcNMTgw\n"
   "ODIzMTczNjQ4WhcNMTkwODIzMTczNjQ4WjA1MRUwEwYDVQQHDAxEZWZhdWx0IENp\n"
   "dHkxHDAaBgNVBAoME0RlZmF1bHQgQ29tcGFueSBMdGQwXDANBgkqhkiG9w0BAQEF\n"
   "AANLADBIAkEAqOvVKzrSpmKOTNqDzBG/iZrUdhCrMRsymFXyIScJcdsyn7jB8RMy\n"
   "fbHqG8EqB8HHLU/eqt/+zhh2w08Lx3+5QwIDAQABMA0GCSqGSIb3DQEBCwUAA0EA\n"
   "RSCq0sNbD9uWfcBqF0U4MtfFjU5x+RQQCeBVtAzwC9bggSILKZfB9XUvtGh6vqig\n",
   /* Self-signed secp112r2 */
   "MIIBLTCB+QIJAI0LtN9uWxy3MAoGCCqGSM49BAMCMEUxCzAJBgNVBAYTAkFVMRMw\n"
   "EQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0\n"
   "eSBMdGQwHhcNMTgwODIzMTc0MTQ4WhcNMTkwODIzMTc0MTQ4WjBFMQswCQYDVQQG\n"
   "EwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lk\n"
   "Z2l0cyBQdHkgTHRkMDIwEAYHKoZIzj0CAQYFK4EEAAcDHgAEf7dFHo7xhCtIcgyo\n"
   "Px+IDcUUlntZCtar6V4O0zAKBggqhkjOPQQDAgMjADAgAg4yhBJMEmpkNbZU95Zf\n"
   "uwIOJAan4J1ETxUII1RrGmw=\n"
  };
  const time_t now = 1535046182;
  tor_x509_cert_t *cert = NULL;

  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(certificates); ++i) {
    cert = cert_from_der64(certificates[i]);
    /* It might parse okay, depending on our version of NSS or OpenSSL. */
    if (cert == NULL)
      continue;
    /* But it should not validate. */
    tt_assert(! tor_tls_cert_is_valid(LOG_INFO, cert, cert, now, 0));
    tor_x509_cert_free(cert);
  }

 done:
  tor_x509_cert_free(cert);
}

static void
test_x509_expiration(void *arg)
{
  (void)arg;
  /* a 365-day RSA2048 cert, created between 0 and 60 minutes before "now" */
  const char certificate[] =
    "MIICzjCCAbYCCQDxIONWIQ9OGDANBgkqhkiG9w0BAQsFADApMQswCQYDVQQGEwJV\n"
    "UzEaMBgGA1UEAwwRSW50ZXJlc3RpbmcgdGltZXMwHhcNMTgwODIzMTc1NTE4WhcN\n"
    "MTkwODIzMTc1NTE4WjApMQswCQYDVQQGEwJVUzEaMBgGA1UEAwwRSW50ZXJlc3Rp\n"
    "bmcgdGltZXMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQD0Blz1fBii\n"
    "OffpFlzMrmfPah/vkPcNrwoyx5YiosbHErYUpqdCtfNb7rbBM5xcac1LmF9kjnOQ\n"
    "uAw1jsCNE82QHwWMlXOqaZCEJsnttNo0Y7yaSR/ChbGJ54XCp+Lx2acyTeH9cBWU\n"
    "de8/sKAQ4NqpbEP01pBH4+1mPu2MYWjVWVicUxmw0mJ3cfkJCWUzt0nC4ls8+Itk\n"
    "7XliKb216Z9uQXu/zD/JGkxAljnFs1jXCX4NyWz46xnJFzXbYCeyQnBz0tUbAvgg\n"
    "uRdryYtHzD46hd8LTXH6oK2gV64ILAhDnRb1aBjnCXxbex24XoW3hjSrKGTdNsXA\n"
    "RMWU/8QZaoiBAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAFIYDBcbit2kOMrHECZK\n"
    "ctem40A3s+0ZifzZ2KLhW8dTr/2Zb6DnlqVm2iUOV4cG/o1RAn/HzkQQuWEq+oBG\n"
    "yOPVHudvCyGs+2ZQWudgAv9xq8N7KtZwJhnn42c2YSoreqRXDQgJqGFatyr+XdR7\n"
    "gdQapLI4BFbZToeXp49Nl+q9330hKaSmIYmWEZ7R/33R64PU2el7X9/apYEcuZQT\n"
    "+FjEqcO1lJ8/dTwM/2C1BJZqUeFTAu+ac1M+4//qyJRUUc6xSJLhiens8atWaxwL\n"
    "eBCT8fCY8oPOwA1eImc/yWWmWXpv8bBWVe8OeLCMKM/OZoIdFqQpqSdcyGoh/kIW\n"
    "Dws=\n";
  const time_t now = 1535046996;

  tor_x509_cert_t *cert = cert_from_der64(certificate);
  tt_assert(cert);

  tt_assert(tor_tls_cert_is_valid(LOG_ERR, cert, cert, now, 0));

  tt_assert(tor_tls_cert_is_valid(LOG_ERR, cert, cert,
                                 now-TOR_X509_FUTURE_SLOP, 0));
  tt_assert(tor_tls_cert_is_valid(LOG_ERR, cert, cert,
                                  now+365*86400+TOR_X509_PAST_SLOP - 3600, 0));

  tt_assert(! tor_tls_cert_is_valid(LOG_INFO, cert, cert,
                                    now-TOR_X509_FUTURE_SLOP - 3600, 0));
  tt_assert(! tor_tls_cert_is_valid(LOG_INFO, cert, cert,
                                    now+365*86400+TOR_X509_FUTURE_SLOP, 0));

 done:
  tor_x509_cert_free(cert);
}

#define TEST(name) { #name, test_x509_ ## name, TT_FORK, 0, NULL }

struct testcase_t x509_tests[] = {
  TEST(cert_new_failing_digest),
  TEST(consume_ec_cert),
  TEST(reject_tiny_keys),
  TEST(expiration),
  END_OF_TESTCASES
};
