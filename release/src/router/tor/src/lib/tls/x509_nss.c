/* Copyright (c) 2003, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file x509_nss.c
 * \brief Wrapper functions to present a consistent interface to
 * X.509 functions from NSS.
 **/

#define TOR_X509_PRIVATE
#include "lib/tls/x509.h"
#include "lib/tls/x509_internal.h"
#include "lib/tls/tortls.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/crypt_ops/crypto_nss_mgt.h"
#include "lib/log/util_bug.h"
#include "lib/encoding/time_fmt.h"
#include "lib/string/printf.h"

#include <pk11pub.h>
#include <cryptohi.h>
#include <cert.h>
#include <keyhi.h>
#include <time.h>

/* Units of PRTime per second.
 *
 * (PRTime is based in microseconds since the Unix
 * epoch.) */
#define PRTIME_PER_SEC (1000*1000)

static tor_x509_cert_impl_t *tor_x509_cert_decode_internal(
                      const uint8_t *certificate, int certificate_len);

static tor_x509_cert_impl_t *
tor_tls_create_certificate_internal(crypto_pk_t *rsa,
                                    crypto_pk_t *rsa_sign,
                                    CERTName *subject_dn,
                                    CERTName *issuer_dn,
                                    time_t start_time,
                                    time_t end_time)
{
  if (! crypto_pk_key_is_private(rsa_sign)) {
    return NULL;
  }

  const SECKEYPublicKey *subject_key = crypto_pk_get_nss_pubkey(rsa);
  const SECKEYPrivateKey *signing_key = crypto_pk_get_nss_privkey(rsa_sign);
  SECStatus s;

  CERTSubjectPublicKeyInfo *subject_spki = NULL;
  CERTCertificateRequest *request = NULL;
  CERTValidity *validity = NULL;
  CERTCertificate *cert = NULL;
  SECItem der = { .data = NULL, .len = 0 };
  SECItem signed_der = { .data = NULL, .len = 0 };

  CERTCertificate *result_cert = NULL;

  validity = CERT_CreateValidity(((PRTime)start_time) * PRTIME_PER_SEC,
                                 ((PRTime)end_time) * PRTIME_PER_SEC);
  if (BUG(! validity)) {
    /* LCOV_EXCL_START */
    crypto_nss_log_errors(LOG_WARN, "creating a validity object");
    goto err;
    /* LCOV_EXCL_STOP */
  }

  unsigned long serial_number;
  crypto_rand((char*)&serial_number, sizeof(serial_number));

  subject_spki = SECKEY_CreateSubjectPublicKeyInfo(subject_key);
  if (!subject_spki)
    goto err;

  /* Make a CSR ... */
  // XXX do we need to set any attributes?
  request = CERT_CreateCertificateRequest(subject_dn,
                                          subject_spki,
                                          NULL /* attributes */);
  if (!request)
    goto err;

  /* Put it into a certificate ... */
  cert = CERT_CreateCertificate(serial_number,
                                issuer_dn,
                                validity,
                                request);
  if (!cert)
    goto err;

  /* version 3 cert */
  *cert->version.data = 2; /* 2 means version 3. */
  cert->version.len = 1;

  // XXX do we need to set anything else on the cert?

  /* Sign it. */
  KeyType privkey_type = SECKEY_GetPrivateKeyType(signing_key);
  SECOidTag oid_tag = SEC_GetSignatureAlgorithmOidTag(privkey_type,
                                                      SEC_OID_SHA256);
  if (oid_tag == SEC_OID_UNKNOWN)
    goto err;
  s = SECOID_SetAlgorithmID(cert->arena, &cert->signature, oid_tag, NULL);
  if (s != SECSuccess)
    goto err;

  void *tmp;
  tmp = SEC_ASN1EncodeItem(cert->arena, &der, cert,
                           SEC_ASN1_GET(CERT_CertificateTemplate));
  if (!tmp)
    goto err;

#if 0
  s = SEC_DerSignDataWithAlgorithmID(cert->arena,
                                     &signed_der,
                                     der.data, der.len,
                                     (SECKEYPrivateKey *)signing_key,//const
                                     &cert->signature);
#else /* !(0) */
  s = SEC_DerSignData(cert->arena,
                      &signed_der,
                      der.data, der.len,
                      (SECKEYPrivateKey *)signing_key,//const
                      SEC_OID_PKCS1_SHA256_WITH_RSA_ENCRYPTION);
#endif /* 0 */

  if (s != SECSuccess)
    goto err;

  /* Re-parse it, to make sure all the certificates we actually use
   * appear via being decoded. */
  result_cert = tor_x509_cert_decode_internal(signed_der.data, signed_der.len);

#if 1
  {
    // Can we check the cert we just signed?
    tor_assert(result_cert);
    SECKEYPublicKey *issuer_pk = (SECKEYPublicKey *)
      crypto_pk_get_nss_pubkey(rsa_sign);
    SECStatus cert_ok = CERT_VerifySignedDataWithPublicKey(
                               &result_cert->signatureWrap, issuer_pk, NULL);
    tor_assert(cert_ok == SECSuccess);
  }
#endif /* 1 */

 err:
  if (subject_spki)
    SECKEY_DestroySubjectPublicKeyInfo(subject_spki);
  if (request)
    CERT_DestroyCertificateRequest(request);
  if (validity)
    CERT_DestroyValidity(validity);

  // unnecessary, since these are allocated in the cert's arena.
  //SECITEM_FreeItem(&der, PR_FALSE);
  //SECITEM_FreeItem(&signed_der, PR_FALSE);
  if (cert)
    CERT_DestroyCertificate(cert);

  return result_cert;
}

MOCK_IMPL(tor_x509_cert_impl_t *,
tor_tls_create_certificate,(crypto_pk_t *rsa,
                            crypto_pk_t *rsa_sign,
                            const char *cname,
                            const char *cname_sign,
                            unsigned int cert_lifetime))
{
  tor_assert(rsa);
  tor_assert(rsa_sign);
  tor_assert(cname);
  tor_assert(cname_sign);

  char *cname_rfc_1485 = NULL, *cname_sign_rfc_1485 = NULL;
  CERTName *subject_dn = NULL, *issuer_dn = NULL;
  time_t start_time;
  time_t end_time;
  CERTCertificate *result = NULL;

  tor_asprintf(&cname_rfc_1485, "CN=%s", cname);
  tor_asprintf(&cname_sign_rfc_1485, "CN=%s", cname_sign);

  subject_dn = CERT_AsciiToName(cname_rfc_1485);
  issuer_dn = CERT_AsciiToName(cname_sign_rfc_1485);
  if (!subject_dn || !issuer_dn)
    goto err;

  tor_tls_pick_certificate_lifetime(time(NULL), cert_lifetime,
                                    &start_time, &end_time);

  result = tor_tls_create_certificate_internal(rsa,
                                               rsa_sign,
                                               subject_dn,
                                               issuer_dn,
                                               start_time,
                                               end_time);
 err:
  tor_free(cname_rfc_1485);
  tor_free(cname_sign_rfc_1485);
  if (subject_dn)
    CERT_DestroyName(subject_dn);
  if (issuer_dn)
    CERT_DestroyName(issuer_dn);

  return result;
}

/** Set *<b>encoded_out</b> and *<b>size_out</b> to <b>cert</b>'s encoded DER
 * representation and length, respectively. */
void
tor_x509_cert_get_der(const tor_x509_cert_t *cert,
                 const uint8_t **encoded_out, size_t *size_out)
{
  tor_assert(cert);
  tor_assert(cert->cert);
  tor_assert(encoded_out);
  tor_assert(size_out);

  const SECItem *item = &cert->cert->derCert;
  *encoded_out = item->data;
  *size_out = (size_t)item->len;
}

void
tor_x509_cert_impl_free_(tor_x509_cert_impl_t *cert)
{
  if (cert)
    CERT_DestroyCertificate(cert);
}

tor_x509_cert_impl_t *
tor_x509_cert_impl_dup_(tor_x509_cert_impl_t *cert)
{
  if (cert)
    return CERT_DupCertificate(cert);
  else
    return NULL;
}

/**
 * As tor_x509_cert_decode, but return the NSS certificate type
*/
static tor_x509_cert_impl_t *
tor_x509_cert_decode_internal(const uint8_t *certificate,
                              int certificate_len)
{
  tor_assert(certificate);
  if (certificate_len > INT_MAX)
    return NULL;

  SECItem der = { .type = siBuffer,
                  .data = (unsigned char *)certificate,
                  .len = certificate_len };
  CERTCertDBHandle *certdb = CERT_GetDefaultCertDB();
  tor_assert(certdb);
  return CERT_NewTempCertificate(certdb,
                                 &der,
                                 NULL /* nickname */,
                                 PR_FALSE, /* isPerm */
                                 PR_TRUE /* CopyDER */);
}

tor_x509_cert_t *
tor_x509_cert_decode(const uint8_t *certificate,
                     size_t certificate_len)
{
  CERTCertificate *cert = tor_x509_cert_decode_internal(certificate,
                                                        (int)certificate_len);
  if (! cert) {
    crypto_nss_log_errors(LOG_INFO, "decoding certificate");
    return NULL;
  }

  tor_x509_cert_t *newcert = tor_x509_cert_new(cert);

  return newcert;
}

crypto_pk_t *
tor_tls_cert_get_key(tor_x509_cert_t *cert)
{
  tor_assert(cert);
  CERTSubjectPublicKeyInfo *spki = &cert->cert->subjectPublicKeyInfo;
  SECKEYPublicKey *pub = SECKEY_ExtractPublicKey(spki); // we own this pointer
  if (pub == NULL)
    return NULL;

  if (SECKEY_GetPublicKeyType(pub) != rsaKey) {
    SECKEY_DestroyPublicKey(pub);
    return NULL;
  }

  return crypto_pk_new_from_nss_pubkey(pub);
}

int
tor_tls_cert_is_valid(int severity,
                      const tor_x509_cert_t *cert,
                      const tor_x509_cert_t *signing_cert,
                      time_t now,
                      int check_rsa_1024)
{
  int result = 0;

  tor_assert(cert);
  tor_assert(signing_cert);

  SECKEYPublicKey *pk = CERT_ExtractPublicKey(signing_cert->cert);
  if (pk == NULL) {
    log_fn(severity, LD_CRYPTO,
           "Invalid certificate: could not extract issuer key");
    goto fail;
  }

  SECStatus s = CERT_VerifySignedDataWithPublicKey(&cert->cert->signatureWrap,
                                                   pk, NULL);
  if (s != SECSuccess) {
    log_fn(severity, LD_CRYPTO,
           "Invalid certificate: could not validate signature.");
    goto fail;
  }

  if (tor_x509_check_cert_lifetime_internal(severity,
                                            cert->cert,
                                            now,
                                            TOR_X509_PAST_SLOP,
                                            TOR_X509_FUTURE_SLOP) < 0)
    goto fail;

  if (check_rsa_1024) {
    /* We require that this is a 1024-bit RSA key, for legacy reasons .:p */
    if (SECKEY_GetPublicKeyType(pk) != rsaKey ||
        SECKEY_PublicKeyStrengthInBits(pk) != 1024) {
      log_fn(severity, LD_CRYPTO, "Invalid certificate: Key is not RSA1024.");
      goto fail;
    }
  } else {
    /* We require that this key is at least minimally strong. */
    unsigned min_bits = (SECKEY_GetPublicKeyType(pk) == ecKey) ? 128: 1024;
    if (SECKEY_PublicKeyStrengthInBits(pk) < min_bits) {
      log_fn(severity, LD_CRYPTO, "Invalid certificate: Key is too weak.");
      goto fail;
    }
  }

  /* The certificate is valid. */
  result = 1;

 fail:
  if (pk)
    SECKEY_DestroyPublicKey(pk);
  return result;
}

static void
log_cert_lifetime(int severity,
                  const char *status,
                  time_t now,
                  PRTime notBefore,
                  PRTime notAfter)
{
  log_fn(severity, LD_GENERAL,
         "Certificate %s. Either their clock is set wrong, or your clock "
         "is incorrect.", status);

  char nowbuf[ISO_TIME_LEN+1];
  char nbbuf[ISO_TIME_LEN+1];
  char nabuf[ISO_TIME_LEN+1];

  format_iso_time(nowbuf, now);
  format_iso_time(nbbuf, notBefore / PRTIME_PER_SEC);
  format_iso_time(nabuf, notAfter / PRTIME_PER_SEC);

  log_fn(severity, LD_GENERAL,
         "(The certificate is valid from %s until %s. Your time is %s.)",
         nbbuf, nabuf, nowbuf);
}

int
tor_x509_check_cert_lifetime_internal(int severity,
                                      const tor_x509_cert_impl_t *cert,
                                      time_t now,
                                      int past_tolerance,
                                      int future_tolerance)
{
  tor_assert(cert);

  PRTime notBefore=0, notAfter=0;
  int64_t t;
  SECStatus r = CERT_GetCertTimes(cert, &notBefore, &notAfter);
  if (r != SECSuccess) {
    log_fn(severity, LD_CRYPTO,
           "Couldn't get validity times from certificate");
    return -1;
  }

  t = ((int64_t)now) + future_tolerance;
  t *= PRTIME_PER_SEC;
  if (notBefore > t) {
    log_cert_lifetime(severity, "not yet valid", now,
                      notBefore, notAfter);
    return -1;
  }

  t = ((int64_t)now) - past_tolerance;
  t *= PRTIME_PER_SEC;
  if (notAfter < t) {
    log_cert_lifetime(severity, "already expired", now,
                      notBefore, notAfter);
    return -1;
  }

  return 0;
}

#ifdef TOR_UNIT_TESTS
tor_x509_cert_t *
tor_x509_cert_replace_expiration(const tor_x509_cert_t *inp,
                                 time_t new_expiration_time,
                                 crypto_pk_t *signing_key)
{
  tor_assert(inp);
  tor_assert(signing_key);

  PRTime notBefore=0, notAfter=0;
  SECStatus r = CERT_GetCertTimes(inp->cert, &notBefore, &notAfter);
  if (r != SECSuccess)
    return NULL;

  time_t start_time = notBefore / PRTIME_PER_SEC;
  if (new_expiration_time < start_time) {
    /* This prevents an NSS error. */
    start_time = new_expiration_time - 10;
  }

  crypto_pk_t *subject_key = tor_tls_cert_get_key((tor_x509_cert_t *)inp);
  if (!subject_key)
    return NULL;

  CERTCertificate *newcert;

  newcert = tor_tls_create_certificate_internal(subject_key,
                                                signing_key,
                                                &inp->cert->subject,
                                                &inp->cert->issuer,
                                                start_time,
                                                new_expiration_time);

  crypto_pk_free(subject_key);

  return newcert ? tor_x509_cert_new(newcert) : NULL;
}
#endif /* defined(TOR_UNIT_TESTS) */
