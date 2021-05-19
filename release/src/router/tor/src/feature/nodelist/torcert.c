/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file torcert.c
 *
 * \brief Implementation for ed25519-signed certificates as used in the Tor
 * protocol.
 *
 * This certificate format is designed to be simple and compact; it's
 * documented in tor-spec.txt in the torspec.git repository.  All of the
 * certificates in this format are signed with an Ed25519 key; the
 * contents themselves may be another Ed25519 key, a digest of a
 * RSA key, or some other material.
 *
 * In this module there is also support for a crooss-certification of
 * Ed25519 identities using (older) RSA1024 identities.
 *
 * Tor uses other types of certificate too, beyond those described in this
 * module. Notably, our use of TLS requires us to touch X.509 certificates,
 * even though sensible people would stay away from those. Our X.509
 * certificates are represented with tor_x509_cert_t, and implemented in
 * tortls.c.  We also have a separate certificate type that authorities
 * use to authenticate their RSA signing keys with their RSA identity keys:
 * that one is authority_cert_t, and it's mostly handled in routerlist.c.
 */

#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/nodelist/torcert.h"
#include "trunnel/ed25519_cert.h"
#include "lib/log/log.h"
#include "trunnel/link_handshake.h"
#include "lib/tls/tortls.h"
#include "lib/tls/x509.h"

#include "core/or/or_handshake_certs_st.h"

/** As tor_cert_create(), but accept an arbitrary signed_key_type as the
 * subject key -- not just an ed25519 key.
 */
tor_cert_t *
tor_cert_create_raw(const ed25519_keypair_t *signing_key,
                      uint8_t cert_type,
                      uint8_t signed_key_type,
                      const uint8_t signed_key_info[32],
                      time_t now, time_t lifetime,
                      uint32_t flags)
{
  tor_cert_t *torcert = NULL;

  ed25519_cert_t *cert = ed25519_cert_new();
  tor_assert(cert); // Unlike Tor's, Trunnel's "new" functions can return NULL.
  cert->cert_type = cert_type;
  cert->exp_field = (uint32_t) CEIL_DIV(now + lifetime, 3600);
  cert->cert_key_type = signed_key_type;
  memcpy(cert->certified_key, signed_key_info, 32);

  if (flags & CERT_FLAG_INCLUDE_SIGNING_KEY) {
    ed25519_cert_extension_t *ext = ed25519_cert_extension_new();
    ext->ext_type = CERTEXT_SIGNED_WITH_KEY;
    memcpy(ext->un_signing_key, signing_key->pubkey.pubkey, 32);
    ed25519_cert_add_ext(cert, ext);
    ++cert->n_extensions;
  }

  const ssize_t alloc_len = ed25519_cert_encoded_len(cert);
  tor_assert(alloc_len > 0);
  uint8_t *encoded = tor_malloc(alloc_len);
  const ssize_t real_len = ed25519_cert_encode(encoded, alloc_len, cert);
  if (real_len < 0)
    goto err;
  tor_assert(real_len == alloc_len);
  tor_assert(real_len > ED25519_SIG_LEN);
  uint8_t *sig = encoded + (real_len - ED25519_SIG_LEN);
  tor_assert(fast_mem_is_zero((char*)sig, ED25519_SIG_LEN));

  ed25519_signature_t signature;
  if (ed25519_sign(&signature, encoded,
                   real_len-ED25519_SIG_LEN, signing_key)<0) {
    /* LCOV_EXCL_START */
    log_warn(LD_BUG, "Can't sign certificate");
    goto err;
    /* LCOV_EXCL_STOP */
  }
  memcpy(sig, signature.sig, ED25519_SIG_LEN);

  torcert = tor_cert_parse(encoded, real_len);
  if (! torcert) {
    /* LCOV_EXCL_START */
    log_warn(LD_BUG, "Generated a certificate we cannot parse");
    goto err;
    /* LCOV_EXCL_STOP */
  }

  if (tor_cert_checksig(torcert, &signing_key->pubkey, now) < 0) {
    /* LCOV_EXCL_START */
    log_warn(LD_BUG, "Generated a certificate whose signature we can't "
             "check: %s", tor_cert_describe_signature_status(torcert));
    goto err;
    /* LCOV_EXCL_STOP */
  }

  tor_free(encoded);

  goto done;

 /* LCOV_EXCL_START */
 err:
  tor_cert_free(torcert);
  torcert = NULL;
 /* LCOV_EXCL_STOP */

 done:
  ed25519_cert_free(cert);
  tor_free(encoded);
  return torcert;
}

/**
 * Create and return a new new certificate of type <b>cert_type</b> to
 * authenticate <b>signed_key</b> using the key <b>signing_key</b>.  The
 * certificate should remain valid for at least <b>lifetime</b> seconds after
 * <b>now</b>.
 *
 * If CERT_FLAG_INCLUDE_SIGNING_KEY is set in <b>flags</b>, embed
 * the public part of <b>signing_key</b> in the certificate.
 */
tor_cert_t *
tor_cert_create_ed25519(const ed25519_keypair_t *signing_key,
                uint8_t cert_type,
                const ed25519_public_key_t *signed_key,
                time_t now, time_t lifetime,
                uint32_t flags)
{
  return tor_cert_create_raw(signing_key, cert_type,
                            SIGNED_KEY_TYPE_ED25519, signed_key->pubkey,
                            now, lifetime, flags);
}

/** Release all storage held for <b>cert</b>. */
void
tor_cert_free_(tor_cert_t *cert)
{
  if (! cert)
    return;

  if (cert->encoded)
    memwipe(cert->encoded, 0, cert->encoded_len);
  tor_free(cert->encoded);

  memwipe(cert, 0, sizeof(tor_cert_t));
  tor_free(cert);
}

/** Parse a certificate encoded with <b>len</b> bytes in <b>encoded</b>. */
tor_cert_t *
tor_cert_parse(const uint8_t *encoded, const size_t len)
{
  tor_cert_t *cert = NULL;
  ed25519_cert_t *parsed = NULL;
  ssize_t got_len = ed25519_cert_parse(&parsed, encoded, len);
  if (got_len < 0 || (size_t) got_len != len)
    goto err;

  cert = tor_malloc_zero(sizeof(tor_cert_t));
  cert->encoded = tor_memdup(encoded, len);
  cert->encoded_len = len;

  memcpy(cert->signed_key.pubkey, parsed->certified_key, 32);
  int64_t valid_until_64 = ((int64_t)parsed->exp_field) * 3600;
#if SIZEOF_TIME_T < 8
  if (valid_until_64 > TIME_MAX)
    valid_until_64 = TIME_MAX - 1;
#endif
  cert->valid_until = (time_t) valid_until_64;
  cert->cert_type = parsed->cert_type;

  for (unsigned i = 0; i < ed25519_cert_getlen_ext(parsed); ++i) {
    ed25519_cert_extension_t *ext = ed25519_cert_get_ext(parsed, i);
    if (ext->ext_type == CERTEXT_SIGNED_WITH_KEY) {
      if (cert->signing_key_included)
        goto err;

      cert->signing_key_included = 1;
      memcpy(cert->signing_key.pubkey, ext->un_signing_key, 32);
    } else if (ext->ext_flags & CERTEXT_FLAG_AFFECTS_VALIDATION) {
      /* Unrecognized extension with affects_validation set */
      goto err;
    }
  }

  goto done;
 err:
  tor_cert_free(cert);
  cert = NULL;
 done:
  ed25519_cert_free(parsed);
  return cert;
}

/** Fill in <b>checkable_out</b> with the information needed to check
 * the signature on <b>cert</b> with <b>pubkey</b>.
 *
 * On success, if <b>expiration_out</b> is provided, and it is some time
 * _after_ the expiration time of this certificate, set it to the
 * expiration time of this certificate.
 */
int
tor_cert_get_checkable_sig(ed25519_checkable_t *checkable_out,
                           const tor_cert_t *cert,
                           const ed25519_public_key_t *pubkey,
                           time_t *expiration_out)
{
  if (! pubkey) {
    if (cert->signing_key_included)
      pubkey = &cert->signing_key;
    else
      return -1;
  }

  checkable_out->msg = cert->encoded;
  checkable_out->pubkey = pubkey;
  tor_assert(cert->encoded_len > ED25519_SIG_LEN);
  const size_t signed_len = cert->encoded_len - ED25519_SIG_LEN;
  checkable_out->len = signed_len;
  memcpy(checkable_out->signature.sig,
         cert->encoded + signed_len, ED25519_SIG_LEN);

  if (expiration_out) {
    *expiration_out = MIN(*expiration_out, cert->valid_until);
  }

  return 0;
}

/** Validates the signature on <b>cert</b> with <b>pubkey</b> relative to the
 * current time <b>now</b>.  (If <b>now</b> is 0, do not check the expiration
 * time.) Return 0 on success, -1 on failure.  Sets flags in <b>cert</b> as
 * appropriate.
 */
int
tor_cert_checksig(tor_cert_t *cert,
                  const ed25519_public_key_t *pubkey, time_t now)
{
  ed25519_checkable_t checkable;
  int okay;
  time_t expires = TIME_MAX;

  if (tor_cert_get_checkable_sig(&checkable, cert, pubkey, &expires) < 0)
    return -1;

  if (now && now > expires) {
    cert->cert_expired = 1;
    return -1;
  }

  if (ed25519_checksig_batch(&okay, &checkable, 1) < 0) {
    cert->sig_bad = 1;
    return -1;
  } else {
    cert->sig_ok = 1;
    /* Only copy the checkable public key when it is different from the signing
     * key of the certificate to avoid undefined behavior. */
    if (cert->signing_key.pubkey != checkable.pubkey->pubkey) {
      memcpy(cert->signing_key.pubkey, checkable.pubkey->pubkey, 32);
    }
    cert->cert_valid = 1;
    return 0;
  }
}

/** Return a string describing the status of the signature on <b>cert</b>
 *
 * Will always be "unchecked" unless tor_cert_checksig has been called.
 */
const char *
tor_cert_describe_signature_status(const tor_cert_t *cert)
{
  if (cert->cert_expired) {
    return "expired";
  } else if (cert->sig_bad) {
    return "mis-signed";
  } else if (cert->sig_ok) {
    return "okay";
  } else {
    return "unchecked";
  }
}

/** Return a new copy of <b>cert</b> */
MOCK_IMPL(tor_cert_t *,
tor_cert_dup,(const tor_cert_t *cert))
{
  tor_cert_t *newcert = tor_memdup(cert, sizeof(tor_cert_t));
  if (cert->encoded)
    newcert->encoded = tor_memdup(cert->encoded, cert->encoded_len);
  return newcert;
}

/** Return true iff cert1 and cert2 are the same cert. */
int
tor_cert_eq(const tor_cert_t *cert1, const tor_cert_t *cert2)
{
  tor_assert(cert1);
  tor_assert(cert2);
  return cert1->encoded_len == cert2->encoded_len &&
    tor_memeq(cert1->encoded, cert2->encoded, cert1->encoded_len);
}

/** Return true iff cert1 and cert2 are the same cert, or if they are both
 * NULL. */
int
tor_cert_opt_eq(const tor_cert_t *cert1, const tor_cert_t *cert2)
{
  if (cert1 == NULL && cert2 == NULL)
    return 1;
  if (!cert1 || !cert2)
    return 0;
  return tor_cert_eq(cert1, cert2);
}

#define RSA_ED_CROSSCERT_PREFIX "Tor TLS RSA/Ed25519 cross-certificate"

/** Create new cross-certification object to certify <b>ed_key</b> as the
 * master ed25519 identity key for the RSA identity key <b>rsa_key</b>.
 * Allocates and stores the encoded certificate in *<b>cert</b>, and returns
 * the number of bytes stored. Returns negative on error.*/
ssize_t
tor_make_rsa_ed25519_crosscert(const ed25519_public_key_t *ed_key,
                               const crypto_pk_t *rsa_key,
                               time_t expires,
                               uint8_t **cert)
{
  // It is later than 1985, since otherwise there would be no C89
  // compilers. (Try to diagnose #22466.)
  tor_assert_nonfatal(expires >= 15 * 365 * 86400);

  uint8_t *res;

  rsa_ed_crosscert_t *cc = rsa_ed_crosscert_new();
  memcpy(cc->ed_key, ed_key->pubkey, ED25519_PUBKEY_LEN);
  cc->expiration = (uint32_t) CEIL_DIV(expires, 3600);
  cc->sig_len = crypto_pk_keysize(rsa_key);
  rsa_ed_crosscert_setlen_sig(cc, crypto_pk_keysize(rsa_key));

  ssize_t alloc_sz = rsa_ed_crosscert_encoded_len(cc);
  tor_assert(alloc_sz > 0);
  res = tor_malloc_zero(alloc_sz);
  ssize_t sz = rsa_ed_crosscert_encode(res, alloc_sz, cc);
  tor_assert(sz > 0 && sz <= alloc_sz);

  crypto_digest_t *d = crypto_digest256_new(DIGEST_SHA256);
  crypto_digest_add_bytes(d, RSA_ED_CROSSCERT_PREFIX,
                          strlen(RSA_ED_CROSSCERT_PREFIX));

  const int signed_part_len = 32 + 4;
  crypto_digest_add_bytes(d, (char*)res, signed_part_len);

  uint8_t digest[DIGEST256_LEN];
  crypto_digest_get_digest(d, (char*)digest, sizeof(digest));
  crypto_digest_free(d);

  int siglen = crypto_pk_private_sign(rsa_key,
                                      (char*)rsa_ed_crosscert_getarray_sig(cc),
                                      rsa_ed_crosscert_getlen_sig(cc),
                                      (char*)digest, sizeof(digest));
  tor_assert(siglen > 0 && siglen <= (int)crypto_pk_keysize(rsa_key));
  tor_assert(siglen <= UINT8_MAX);
  cc->sig_len = siglen;
  rsa_ed_crosscert_setlen_sig(cc, siglen);

  sz = rsa_ed_crosscert_encode(res, alloc_sz, cc);
  rsa_ed_crosscert_free(cc);
  *cert = res;
  return sz;
}

/**
 * Check whether the <b>crosscert_len</b> byte certificate in <b>crosscert</b>
 * is in fact a correct cross-certification of <b>master_key</b> using
 * the RSA key <b>rsa_id_key</b>.
 *
 * Also reject the certificate if it expired before
 * <b>reject_if_expired_before</b>.
 *
 * Return 0 on success, negative on failure.
 */
MOCK_IMPL(int,
rsa_ed25519_crosscert_check, (const uint8_t *crosscert,
                              const size_t crosscert_len,
                              const crypto_pk_t *rsa_id_key,
                              const ed25519_public_key_t *master_key,
                              const time_t reject_if_expired_before))
{
  rsa_ed_crosscert_t *cc = NULL;
  int rv;

#define ERR(code, s)                                            \
  do {                                                          \
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,                      \
           "Received a bad RSA->Ed25519 crosscert: %s",         \
           (s));                                                \
    rv = (code);                                                \
    goto err;                                                   \
  } while (0)

  if (BUG(crypto_pk_keysize(rsa_id_key) > PK_BYTES))
    return -1;

  if (BUG(!crosscert))
    return -1;

  ssize_t parsed_len = rsa_ed_crosscert_parse(&cc, crosscert, crosscert_len);
  if (parsed_len < 0 || crosscert_len != (size_t)parsed_len) {
    ERR(-2, "Unparseable or overlong crosscert");
  }

  if (tor_memneq(rsa_ed_crosscert_getarray_ed_key(cc),
                 master_key->pubkey,
                 ED25519_PUBKEY_LEN)) {
    ERR(-3, "Crosscert did not match Ed25519 key");
  }

  const uint32_t expiration_date = rsa_ed_crosscert_get_expiration(cc);
  const uint64_t expiration_time = ((uint64_t)expiration_date) * 3600;

  if (reject_if_expired_before < 0 ||
      expiration_time < (uint64_t)reject_if_expired_before) {
    ERR(-4, "Crosscert is expired");
  }

  const uint8_t *eos = rsa_ed_crosscert_get_end_of_signed(cc);
  const uint8_t *sig = rsa_ed_crosscert_getarray_sig(cc);
  const uint8_t siglen = rsa_ed_crosscert_get_sig_len(cc);
  tor_assert(eos >= crosscert);
  tor_assert((size_t)(eos - crosscert) <= crosscert_len);
  tor_assert(siglen == rsa_ed_crosscert_getlen_sig(cc));

  /* Compute the digest */
  uint8_t digest[DIGEST256_LEN];
  crypto_digest_t *d = crypto_digest256_new(DIGEST_SHA256);
  crypto_digest_add_bytes(d, RSA_ED_CROSSCERT_PREFIX,
                          strlen(RSA_ED_CROSSCERT_PREFIX));
  crypto_digest_add_bytes(d, (char*)crosscert, eos-crosscert);
  crypto_digest_get_digest(d, (char*)digest, sizeof(digest));
  crypto_digest_free(d);

  /* Now check the signature */
  uint8_t signed_[PK_BYTES];
  int signed_len = crypto_pk_public_checksig(rsa_id_key,
                                          (char*)signed_, sizeof(signed_),
                                          (char*)sig, siglen);
  if (signed_len < DIGEST256_LEN) {
    ERR(-5, "Bad signature, or length of signed data not as expected");
  }

  if (tor_memneq(digest, signed_, DIGEST256_LEN)) {
    ERR(-6, "The signature was good, but it didn't match the data");
  }

  rv = 0;
 err:
  rsa_ed_crosscert_free(cc);
  return rv;
}

/** Construct and return a new empty or_handshake_certs object */
or_handshake_certs_t *
or_handshake_certs_new(void)
{
  return tor_malloc_zero(sizeof(or_handshake_certs_t));
}

/** Release all storage held in <b>certs</b> */
void
or_handshake_certs_free_(or_handshake_certs_t *certs)
{
  if (!certs)
    return;

  tor_x509_cert_free(certs->auth_cert);
  tor_x509_cert_free(certs->link_cert);
  tor_x509_cert_free(certs->id_cert);

  tor_cert_free(certs->ed_id_sign);
  tor_cert_free(certs->ed_sign_link);
  tor_cert_free(certs->ed_sign_auth);
  tor_free(certs->ed_rsa_crosscert);

  memwipe(certs, 0xBD, sizeof(*certs));
  tor_free(certs);
}

#undef ERR
#define ERR(s)                                                  \
  do {                                                          \
    log_fn(severity, LD_PROTOCOL,                               \
           "Received a bad CERTS cell: %s",                     \
           (s));                                                \
    return 0;                                                   \
  } while (0)

int
or_handshake_certs_rsa_ok(int severity,
                          or_handshake_certs_t *certs,
                          tor_tls_t *tls,
                          time_t now)
{
  tor_x509_cert_t *link_cert = certs->link_cert;
  tor_x509_cert_t *auth_cert = certs->auth_cert;
  tor_x509_cert_t *id_cert = certs->id_cert;

  if (certs->started_here) {
    if (! (id_cert && link_cert))
      ERR("The certs we wanted (ID, Link) were missing");
    if (! tor_tls_cert_matches_key(tls, link_cert))
      ERR("The link certificate didn't match the TLS public key");
    if (! tor_tls_cert_is_valid(severity, link_cert, id_cert, now, 0))
      ERR("The link certificate was not valid");
    if (! tor_tls_cert_is_valid(severity, id_cert, id_cert, now, 1))
      ERR("The ID certificate was not valid");
  } else {
    if (! (id_cert && auth_cert))
      ERR("The certs we wanted (ID, Auth) were missing");
    if (! tor_tls_cert_is_valid(LOG_PROTOCOL_WARN, auth_cert, id_cert, now, 1))
      ERR("The authentication certificate was not valid");
    if (! tor_tls_cert_is_valid(LOG_PROTOCOL_WARN, id_cert, id_cert, now, 1))
      ERR("The ID certificate was not valid");
  }

  return 1;
}

/** Check all the ed25519 certificates in <b>certs</b> against each other, and
 * against the peer certificate in <b>tls</b> if appropriate.  On success,
 * return 0; on failure, return a negative value and warn at level
 * <b>severity</b> */
int
or_handshake_certs_ed25519_ok(int severity,
                              or_handshake_certs_t *certs,
                              tor_tls_t *tls,
                              time_t now)
{
  ed25519_checkable_t check[10];
  unsigned n_checkable = 0;
  time_t expiration = TIME_MAX;

#define ADDCERT(cert, pk)                                               \
  do {                                                                  \
    tor_assert(n_checkable < ARRAY_LENGTH(check));                      \
    if (tor_cert_get_checkable_sig(&check[n_checkable++], cert, pk,     \
                                   &expiration) < 0)                    \
      ERR("Could not get checkable cert.");                             \
  } while (0)

  if (! certs->ed_id_sign || !certs->ed_id_sign->signing_key_included) {
    ERR("No Ed25519 signing key");
  }
  ADDCERT(certs->ed_id_sign, NULL);

  if (certs->started_here) {
    if (! certs->ed_sign_link)
      ERR("No Ed25519 link key");
    {
      /* check for a match with the TLS cert. */
      tor_x509_cert_t *peer_cert = tor_tls_get_peer_cert(tls);
      if (BUG(!peer_cert)) {
        /* This is a bug, because if we got to this point, we are a connection
         * that was initiated here, and we completed a TLS handshake. The
         * other side *must* have given us a certificate! */
        ERR("No x509 peer cert"); // LCOV_EXCL_LINE
      }
      const common_digests_t *peer_cert_digests =
        tor_x509_cert_get_cert_digests(peer_cert);
      int okay = tor_memeq(peer_cert_digests->d[DIGEST_SHA256],
                           certs->ed_sign_link->signed_key.pubkey,
                           DIGEST256_LEN);
      tor_x509_cert_free(peer_cert);
      if (!okay)
        ERR("Link certificate does not match TLS certificate");
    }

    ADDCERT(certs->ed_sign_link, &certs->ed_id_sign->signed_key);

  } else {
    if (! certs->ed_sign_auth)
      ERR("No Ed25519 link authentication key");
    ADDCERT(certs->ed_sign_auth, &certs->ed_id_sign->signed_key);
  }

  if (expiration < now) {
    ERR("At least one certificate expired.");
  }

  /* Okay, we've gotten ready to check all the Ed25519 certificates.
   * Now, we are going to check the RSA certificate's cross-certification
   * with the ED certificates.
   *
   * FFFF In the future, we might want to make this optional.
   */

  tor_x509_cert_t *rsa_id_cert = certs->id_cert;
  if (!rsa_id_cert) {
    ERR("Missing legacy RSA ID certificate");
  }
  if (! tor_tls_cert_is_valid(severity, rsa_id_cert, rsa_id_cert, now, 1)) {
    ERR("The legacy RSA ID certificate was not valid");
  }
  if (! certs->ed_rsa_crosscert) {
    ERR("Missing RSA->Ed25519 crosscert");
  }
  crypto_pk_t *rsa_id_key = tor_tls_cert_get_key(rsa_id_cert);
  if (!rsa_id_key) {
    ERR("RSA ID cert had no RSA key");
  }

  if (rsa_ed25519_crosscert_check(certs->ed_rsa_crosscert,
                                  certs->ed_rsa_crosscert_len,
                                  rsa_id_key,
                                  &certs->ed_id_sign->signing_key,
                                  now) < 0) {
    crypto_pk_free(rsa_id_key);
    ERR("Invalid RSA->Ed25519 crosscert");
  }
  crypto_pk_free(rsa_id_key);
  rsa_id_key = NULL;

  /* FFFF We could save a little time in the client case by queueing
   * this batch to check it later, along with the signature from the
   * AUTHENTICATE cell. That will change our data flow a bit, though,
   * so I say "postpone". */

  if (ed25519_checksig_batch(NULL, check, n_checkable) < 0) {
    ERR("At least one Ed25519 certificate was badly signed");
  }

  return 1;
}

/** Check whether an RSA-TAP cross-certification is correct. Return 0 if it
 * is, -1 if it isn't. */
MOCK_IMPL(int,
check_tap_onion_key_crosscert,(const uint8_t *crosscert,
                               int crosscert_len,
                               const crypto_pk_t *onion_pkey,
                               const ed25519_public_key_t *master_id_pkey,
                               const uint8_t *rsa_id_digest))
{
  uint8_t *cc = tor_malloc(crypto_pk_keysize(onion_pkey));
  int cc_len =
    crypto_pk_public_checksig(onion_pkey,
                              (char*)cc,
                              crypto_pk_keysize(onion_pkey),
                              (const char*)crosscert,
                              crosscert_len);
  if (cc_len < 0) {
    goto err;
  }
  if (cc_len < DIGEST_LEN + ED25519_PUBKEY_LEN) {
    log_warn(LD_DIR, "Short signature on cross-certification with TAP key");
    goto err;
  }
  if (tor_memneq(cc, rsa_id_digest, DIGEST_LEN) ||
      tor_memneq(cc + DIGEST_LEN, master_id_pkey->pubkey,
                 ED25519_PUBKEY_LEN)) {
    log_warn(LD_DIR, "Incorrect cross-certification with TAP key");
    goto err;
  }

  tor_free(cc);
  return 0;
 err:
  tor_free(cc);
  return -1;
}

/**
 * Check the Ed certificates and/or the RSA certificates, as appropriate.  If
 * we obtained an Ed25519 identity, set *ed_id_out. If we obtained an RSA
 * identity, set *rs_id_out. Otherwise, set them both to NULL.
 */
void
or_handshake_certs_check_both(int severity,
                              or_handshake_certs_t *certs,
                              tor_tls_t *tls,
                              time_t now,
                              const ed25519_public_key_t **ed_id_out,
                              const common_digests_t **rsa_id_out)
{
  tor_assert(ed_id_out);
  tor_assert(rsa_id_out);

  *ed_id_out = NULL;
  *rsa_id_out = NULL;

  if (certs->ed_id_sign) {
    if (or_handshake_certs_ed25519_ok(severity, certs, tls, now)) {
      tor_assert(certs->ed_id_sign);
      tor_assert(certs->id_cert);

      *ed_id_out = &certs->ed_id_sign->signing_key;
      *rsa_id_out = tor_x509_cert_get_id_digests(certs->id_cert);

      /* If we reached this point, we did not look at any of the
       * subsidiary RSA certificates, so we'd better just remove them.
       */
      tor_x509_cert_free(certs->link_cert);
      tor_x509_cert_free(certs->auth_cert);
      certs->link_cert = certs->auth_cert = NULL;
    }
    /* We do _not_ fall through here.  If you provided us Ed25519
     * certificates, we expect to verify them! */
  } else {
    /* No ed25519 keys given in the CERTS cell */
    if (or_handshake_certs_rsa_ok(severity, certs, tls, now)) {
      *rsa_id_out = tor_x509_cert_get_id_digests(certs->id_cert);
    }
  }
}

/* === ENCODING === */

/* Encode the ed25519 certificate <b>cert</b> and put the newly allocated
 * string in <b>cert_str_out</b>. Return 0 on success else a negative value. */
int
tor_cert_encode_ed22519(const tor_cert_t *cert, char **cert_str_out)
{
  int ret = -1;
  char *ed_cert_b64 = NULL;
  size_t ed_cert_b64_len;

  tor_assert(cert);
  tor_assert(cert_str_out);

  /* Get the encoded size and add the NUL byte. */
  ed_cert_b64_len = base64_encode_size(cert->encoded_len,
                                       BASE64_ENCODE_MULTILINE) + 1;
  ed_cert_b64 = tor_malloc_zero(ed_cert_b64_len);

  /* Base64 encode the encoded certificate. */
  if (base64_encode(ed_cert_b64, ed_cert_b64_len,
                    (const char *) cert->encoded, cert->encoded_len,
                    BASE64_ENCODE_MULTILINE) < 0) {
    /* LCOV_EXCL_START */
    log_err(LD_BUG, "Couldn't base64-encode ed22519 cert!");
    goto err;
    /* LCOV_EXCL_STOP */
  }

  /* Put everything together in a NUL terminated string. */
  tor_asprintf(cert_str_out,
               "-----BEGIN ED25519 CERT-----\n"
               "%s"
               "-----END ED25519 CERT-----",
               ed_cert_b64);
  /* Success! */
  ret = 0;

 err:
  tor_free(ed_cert_b64);
  return ret;
}
