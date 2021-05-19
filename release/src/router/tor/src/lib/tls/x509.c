/* Copyright (c) 2003, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file x509_openssl.c
 * \brief Wrapper functions to present a consistent interface to
 * X.509 functions.
 **/

#define TOR_X509_PRIVATE
#include "lib/tls/x509.h"
#include "lib/tls/x509_internal.h"
#include "lib/log/util_bug.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"

/** Choose the start and end times for a certificate */
void
tor_tls_pick_certificate_lifetime(time_t now,
                                  unsigned int cert_lifetime,
                                  time_t *start_time_out,
                                  time_t *end_time_out)
{
  tor_assert(cert_lifetime < INT_MAX);
  time_t start_time, end_time;
  /* Make sure we're part-way through the certificate lifetime, rather
   * than having it start right now. Don't choose quite uniformly, since
   * then we might pick a time where we're about to expire. Lastly, be
   * sure to start on a day boundary. */
  /* Our certificate lifetime will be cert_lifetime no matter what, but if we
   * start cert_lifetime in the past, we'll have 0 real lifetime.  instead we
   * start up to (cert_lifetime - min_real_lifetime - start_granularity) in
   * the past. */
  const time_t min_real_lifetime = 24*3600;
  const time_t start_granularity = 24*3600;
  time_t earliest_start_time;
  /* Don't actually start in the future! */
  if ((int)cert_lifetime <= min_real_lifetime + start_granularity) {
    earliest_start_time = now - 1;
  } else {
    earliest_start_time = now + min_real_lifetime + start_granularity
      - cert_lifetime;
  }
  start_time = crypto_rand_time_range(earliest_start_time, now);
  /* Round the start time back to the start of a day. */
  start_time -= start_time % start_granularity;

  end_time = start_time + cert_lifetime;

  *start_time_out = start_time;
  *end_time_out = end_time;
}

/** Return a set of digests for the public key in <b>cert</b>, or NULL if this
 * cert's public key is not one we know how to take the digest of. */
const common_digests_t *
tor_x509_cert_get_id_digests(const tor_x509_cert_t *cert)
{
  if (cert->pkey_digests_set)
    return &cert->pkey_digests;
  else
    return NULL;
}

/** Return a set of digests for the public key in <b>cert</b>. */
const common_digests_t *
tor_x509_cert_get_cert_digests(const tor_x509_cert_t *cert)
{
  return &cert->cert_digests;
}

/** Free all storage held in <b>cert</b> */
void
tor_x509_cert_free_(tor_x509_cert_t *cert)
{
  if (! cert)
    return;
  tor_x509_cert_impl_free(cert->cert);
#ifdef ENABLE_OPENSSL
  tor_free(cert->encoded);
#endif
  memwipe(cert, 0x03, sizeof(*cert));
  /* LCOV_EXCL_BR_START since cert will never be NULL here */
  tor_free(cert);
  /* LCOV_EXCL_BR_STOP */
}

/**
 * Allocate a new tor_x509_cert_t to hold the certificate "x509_cert".
 *
 * Steals a reference to x509_cert.
 */
MOCK_IMPL(tor_x509_cert_t *,
tor_x509_cert_new,(tor_x509_cert_impl_t *x509_cert))
{
  tor_x509_cert_t *cert;

  if (!x509_cert)
    return NULL;

  cert = tor_malloc_zero(sizeof(tor_x509_cert_t));
  cert->cert = x509_cert;

  if (tor_x509_cert_set_cached_der_encoding(cert) < 0)
    goto err;

  {
    const uint8_t *encoded=NULL;
    size_t encoded_len=0;
    tor_x509_cert_get_der(cert, &encoded, &encoded_len);
    tor_assert(encoded);
    crypto_common_digests(&cert->cert_digests, (char *)encoded, encoded_len);
  }

  {
    crypto_pk_t *pk = tor_tls_cert_get_key(cert);
    if (pk) {
      if (crypto_pk_get_common_digests(pk, &cert->pkey_digests) < 0) {
        log_warn(LD_CRYPTO, "unable to compute digests of certificate key");
        crypto_pk_free(pk);
        goto err;
      }
    }
    cert->pkey_digests_set = 1;
    crypto_pk_free(pk);
  }

  return cert;
 err:
  log_err(LD_CRYPTO, "Couldn't wrap encoded X509 certificate.");
  tor_x509_cert_free(cert);
  return NULL;
}

/** Return a new copy of <b>cert</b>. */
tor_x509_cert_t *
tor_x509_cert_dup(const tor_x509_cert_t *cert)
{
  tor_assert(cert);
  tor_assert(cert->cert);
  return tor_x509_cert_new(tor_x509_cert_impl_dup_(cert->cert));
}
