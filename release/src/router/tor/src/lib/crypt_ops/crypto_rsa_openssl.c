/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_rsa.c
 * \brief OpenSSL implementations of our RSA code.
 **/

#include "lib/crypt_ops/compat_openssl.h"
#include "lib/crypt_ops/crypto_rsa.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/ctime/di_ops.h"
#include "lib/log/util_bug.h"
#include "lib/fs/files.h"

DISABLE_GCC_WARNING("-Wredundant-decls")

#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/conf.h>

ENABLE_GCC_WARNING("-Wredundant-decls")

#include "lib/log/log.h"
#include "lib/encoding/binascii.h"

#include <string.h>
#include <stdbool.h>

/** Declaration for crypto_pk_t structure. */
struct crypto_pk_t
{
  int refs; /**< reference count, so we don't have to copy keys */
  RSA *key; /**< The key itself */
};

/** Return true iff <b>key</b> contains the private-key portion of the RSA
 * key. */
int
crypto_pk_key_is_private(const crypto_pk_t *k)
{
#ifdef OPENSSL_1_1_API
  if (!k || !k->key)
    return 0;

  const BIGNUM *p, *q;
  RSA_get0_factors(k->key, &p, &q);
  return p != NULL; /* XXX/yawning: Should we check q? */
#else /* !defined(OPENSSL_1_1_API) */
  return k && k->key && k->key->p;
#endif /* defined(OPENSSL_1_1_API) */
}

/** used by tortls.c: wrap an RSA* in a crypto_pk_t. Takes ownership of
 * its argument. */
crypto_pk_t *
crypto_new_pk_from_openssl_rsa_(RSA *rsa)
{
  crypto_pk_t *env;
  tor_assert(rsa);
  env = tor_malloc(sizeof(crypto_pk_t));
  env->refs = 1;
  env->key = rsa;
  return env;
}

/** Helper, used by tor-gencert.c.  Return a copy of the private RSA from a
 * crypto_pk_t. */
RSA *
crypto_pk_get_openssl_rsa_(crypto_pk_t *env)
{
  return RSAPrivateKey_dup(env->key);
}

/** used by tortls.c: get an equivalent EVP_PKEY* for a crypto_pk_t.  Iff
 * private is set, include the private-key portion of the key. Return a valid
 * pointer on success, and NULL on failure. */
MOCK_IMPL(EVP_PKEY *,
crypto_pk_get_openssl_evp_pkey_,(crypto_pk_t *env, int private))
{
  RSA *key = NULL;
  EVP_PKEY *pkey = NULL;
  tor_assert(env->key);
  if (private) {
    if (!(key = RSAPrivateKey_dup(env->key)))
      goto error;
  } else {
    if (!(key = RSAPublicKey_dup(env->key)))
      goto error;
  }
  if (!(pkey = EVP_PKEY_new()))
    goto error;
  if (!(EVP_PKEY_assign_RSA(pkey, key)))
    goto error;
  return pkey;
 error:
  if (pkey)
    EVP_PKEY_free(pkey);
  if (key)
    RSA_free(key);
  return NULL;
}

/** Allocate and return storage for a public key.  The key itself will not yet
 * be set.
 */
MOCK_IMPL(crypto_pk_t *,
crypto_pk_new,(void))
{
  RSA *rsa;

  rsa = RSA_new();
  tor_assert(rsa);
  return crypto_new_pk_from_openssl_rsa_(rsa);
}

/** Release a reference to an asymmetric key; when all the references
 * are released, free the key.
 */
void
crypto_pk_free_(crypto_pk_t *env)
{
  if (!env)
    return;

  if (--env->refs > 0)
    return;
  tor_assert(env->refs == 0);

  if (env->key)
    RSA_free(env->key);

  tor_free(env);
}

/** Generate a <b>bits</b>-bit new public/private keypair in <b>env</b>.
 * Return 0 on success, -1 on failure.
 */
MOCK_IMPL(int,
crypto_pk_generate_key_with_bits,(crypto_pk_t *env, int bits))
{
  tor_assert(env);

  if (env->key) {
    RSA_free(env->key);
    env->key = NULL;
  }

  {
    BIGNUM *e = BN_new();
    RSA *r = NULL;
    if (!e)
      goto done;
    if (! BN_set_word(e, TOR_RSA_EXPONENT))
      goto done;
    r = RSA_new();
    if (!r)
      goto done;
    if (RSA_generate_key_ex(r, bits, e, NULL) == -1)
      goto done;

    env->key = r;
    r = NULL;
  done:
    if (e)
      BN_clear_free(e);
    if (r)
      RSA_free(r);
  }

  if (!env->key) {
    crypto_openssl_log_errors(LOG_WARN, "generating RSA key");
    return -1;
  }

  return 0;
}

/** Return true if <b>env</b> has a valid key; false otherwise.
 */
int
crypto_pk_is_valid_private_key(const crypto_pk_t *env)
{
  int r;
  tor_assert(env);

  r = RSA_check_key(env->key);
  if (r <= 0) {
    crypto_openssl_log_errors(LOG_WARN,"checking RSA key");
    return 0;
  } else {
    return 1;
  }
}

/** Return true iff <b>env</b> contains a public key whose public exponent
 * equals TOR_RSA_EXPONENT.
 */
int
crypto_pk_public_exponent_ok(const crypto_pk_t *env)
{
  tor_assert(env);
  tor_assert(env->key);

  const BIGNUM *e;

#ifdef OPENSSL_1_1_API
  const BIGNUM *n, *d;
  RSA_get0_key(env->key, &n, &e, &d);
#else
  e = env->key->e;
#endif /* defined(OPENSSL_1_1_API) */
  return BN_is_word(e, TOR_RSA_EXPONENT);
}

/** Compare the public-key components of a and b.  Return less than 0
 * if a\<b, 0 if a==b, and greater than 0 if a\>b.  A NULL key is
 * considered to be less than all non-NULL keys, and equal to itself.
 *
 * Note that this may leak information about the keys through timing.
 */
int
crypto_pk_cmp_keys(const crypto_pk_t *a, const crypto_pk_t *b)
{
  int result;
  char a_is_non_null = (a != NULL) && (a->key != NULL);
  char b_is_non_null = (b != NULL) && (b->key != NULL);
  char an_argument_is_null = !a_is_non_null | !b_is_non_null;

  result = tor_memcmp(&a_is_non_null, &b_is_non_null, sizeof(a_is_non_null));
  if (an_argument_is_null)
    return result;

  const BIGNUM *a_n, *a_e;
  const BIGNUM *b_n, *b_e;

#ifdef OPENSSL_1_1_API
  const BIGNUM *a_d, *b_d;
  RSA_get0_key(a->key, &a_n, &a_e, &a_d);
  RSA_get0_key(b->key, &b_n, &b_e, &b_d);
#else
  a_n = a->key->n;
  a_e = a->key->e;
  b_n = b->key->n;
  b_e = b->key->e;
#endif /* defined(OPENSSL_1_1_API) */

  tor_assert(a_n != NULL && a_e != NULL);
  tor_assert(b_n != NULL && b_e != NULL);

  result = BN_cmp(a_n, b_n);
  if (result)
    return result;
  return BN_cmp(a_e, b_e);
}

/** Return the size of the public key modulus in <b>env</b>, in bytes. */
size_t
crypto_pk_keysize(const crypto_pk_t *env)
{
  tor_assert(env);
  tor_assert(env->key);

  return (size_t) RSA_size((RSA*)env->key);
}

/** Return the size of the public key modulus of <b>env</b>, in bits. */
int
crypto_pk_num_bits(crypto_pk_t *env)
{
  tor_assert(env);
  tor_assert(env->key);

#ifdef OPENSSL_1_1_API
  /* It's so stupid that there's no other way to check that n is valid
   * before calling RSA_bits().
   */
  const BIGNUM *n, *e, *d;
  RSA_get0_key(env->key, &n, &e, &d);
  tor_assert(n != NULL);

  return RSA_bits(env->key);
#else /* !defined(OPENSSL_1_1_API) */
  tor_assert(env->key->n);
  return BN_num_bits(env->key->n);
#endif /* defined(OPENSSL_1_1_API) */
}

/** Increase the reference count of <b>env</b>, and return it.
 */
crypto_pk_t *
crypto_pk_dup_key(crypto_pk_t *env)
{
  tor_assert(env);
  tor_assert(env->key);

  env->refs++;
  return env;
}

/** Replace dest with src (private key only).  (Dest must have a refcount
 * of 1)
 */
void
crypto_pk_assign_private(crypto_pk_t *dest, const crypto_pk_t *src)
{
  tor_assert(dest);
  tor_assert(dest->refs == 1);
  tor_assert(src);
  RSA_free(dest->key);
  dest->key = RSAPrivateKey_dup(src->key);
}

/** Replace dest with src (public key only).  (Dest must have a refcount
 * of 1)
 */
void
crypto_pk_assign_public(crypto_pk_t *dest, const crypto_pk_t *src)
{
  tor_assert(dest);
  tor_assert(dest->refs == 1);
  tor_assert(src);
  RSA_free(dest->key);
  dest->key = RSAPublicKey_dup(src->key);
}

/** Make a real honest-to-goodness copy of <b>env</b>, and return it.
 * Returns NULL on failure. */
crypto_pk_t *
crypto_pk_copy_full(crypto_pk_t *env)
{
  RSA *new_key;
  int privatekey = 0;
  tor_assert(env);
  tor_assert(env->key);

  if (crypto_pk_key_is_private(env)) {
    new_key = RSAPrivateKey_dup(env->key);
    privatekey = 1;
  } else {
    new_key = RSAPublicKey_dup(env->key);
  }
  if (!new_key) {
    /* LCOV_EXCL_START
     *
     * We can't cause RSA*Key_dup() to fail, so we can't really test this.
     */
    log_err(LD_CRYPTO, "Unable to duplicate a %s key: openssl failed.",
            privatekey?"private":"public");
    crypto_openssl_log_errors(LOG_ERR,
                      privatekey ? "Duplicating a private key" :
                      "Duplicating a public key");
    tor_fragile_assert();
    return NULL;
    /* LCOV_EXCL_STOP */
  }

  return crypto_new_pk_from_openssl_rsa_(new_key);
}

/** Encrypt <b>fromlen</b> bytes from <b>from</b> with the public key
 * in <b>env</b>, using the padding method <b>padding</b>.  On success,
 * write the result to <b>to</b>, and return the number of bytes
 * written.  On failure, return -1.
 *
 * <b>tolen</b> is the number of writable bytes in <b>to</b>, and must be
 * at least the length of the modulus of <b>env</b>.
 */
int
crypto_pk_public_encrypt(crypto_pk_t *env, char *to, size_t tolen,
                         const char *from, size_t fromlen, int padding)
{
  int r;
  tor_assert(env);
  tor_assert(from);
  tor_assert(to);
  tor_assert(fromlen<INT_MAX);
  tor_assert(tolen >= crypto_pk_keysize(env));

  r = RSA_public_encrypt((int)fromlen,
                         (unsigned char*)from, (unsigned char*)to,
                         env->key, crypto_get_rsa_padding(padding));
  if (r<0) {
    crypto_openssl_log_errors(LOG_WARN, "performing RSA encryption");
    return -1;
  }
  return r;
}

/** Decrypt <b>fromlen</b> bytes from <b>from</b> with the private key
 * in <b>env</b>, using the padding method <b>padding</b>.  On success,
 * write the result to <b>to</b>, and return the number of bytes
 * written.  On failure, return -1.
 *
 * <b>tolen</b> is the number of writable bytes in <b>to</b>, and must be
 * at least the length of the modulus of <b>env</b>.
 */
int
crypto_pk_private_decrypt(crypto_pk_t *env, char *to,
                          size_t tolen,
                          const char *from, size_t fromlen,
                          int padding, int warnOnFailure)
{
  int r;
  tor_assert(env);
  tor_assert(from);
  tor_assert(to);
  tor_assert(env->key);
  tor_assert(fromlen<INT_MAX);
  tor_assert(tolen >= crypto_pk_keysize(env));
  if (!crypto_pk_key_is_private(env))
    /* Not a private key */
    return -1;

  r = RSA_private_decrypt((int)fromlen,
                          (unsigned char*)from, (unsigned char*)to,
                          env->key, crypto_get_rsa_padding(padding));

  if (r<0) {
    crypto_openssl_log_errors(warnOnFailure?LOG_WARN:LOG_DEBUG,
                      "performing RSA decryption");
    return -1;
  }
  return r;
}

/** Check the signature in <b>from</b> (<b>fromlen</b> bytes long) with the
 * public key in <b>env</b>, using PKCS1 padding.  On success, write the
 * signed data to <b>to</b>, and return the number of bytes written.
 * On failure, return -1.
 *
 * <b>tolen</b> is the number of writable bytes in <b>to</b>, and must be
 * at least the length of the modulus of <b>env</b>.
 */
MOCK_IMPL(int,
crypto_pk_public_checksig,(const crypto_pk_t *env, char *to,
                           size_t tolen,
                           const char *from, size_t fromlen))
{
  int r;
  tor_assert(env);
  tor_assert(from);
  tor_assert(to);
  tor_assert(fromlen < INT_MAX);
  tor_assert(tolen >= crypto_pk_keysize(env));
  r = RSA_public_decrypt((int)fromlen,
                         (unsigned char*)from, (unsigned char*)to,
                         env->key, RSA_PKCS1_PADDING);

  if (r<0) {
    crypto_openssl_log_errors(LOG_INFO, "checking RSA signature");
    return -1;
  }
  return r;
}

/** Sign <b>fromlen</b> bytes of data from <b>from</b> with the private key in
 * <b>env</b>, using PKCS1 padding.  On success, write the signature to
 * <b>to</b>, and return the number of bytes written.  On failure, return
 * -1.
 *
 * <b>tolen</b> is the number of writable bytes in <b>to</b>, and must be
 * at least the length of the modulus of <b>env</b>.
 */
int
crypto_pk_private_sign(const crypto_pk_t *env, char *to, size_t tolen,
                       const char *from, size_t fromlen)
{
  int r;
  tor_assert(env);
  tor_assert(from);
  tor_assert(to);
  tor_assert(fromlen < INT_MAX);
  tor_assert(tolen >= crypto_pk_keysize(env));
  if (!crypto_pk_key_is_private(env))
    /* Not a private key */
    return -1;

  r = RSA_private_encrypt((int)fromlen,
                          (unsigned char*)from, (unsigned char*)to,
                          (RSA*)env->key, RSA_PKCS1_PADDING);
  if (r<0) {
    crypto_openssl_log_errors(LOG_WARN, "generating RSA signature");
    return -1;
  }
  return r;
}

/** ASN.1-encode the public portion of <b>pk</b> into <b>dest</b>.
 * Return -1 on error, or the number of characters used on success.
 */
int
crypto_pk_asn1_encode(const crypto_pk_t *pk, char *dest, size_t dest_len)
{
  int len;
  unsigned char *buf = NULL;

  len = i2d_RSAPublicKey(pk->key, &buf);
  if (len < 0 || buf == NULL)
    return -1;

  if ((size_t)len > dest_len || dest_len > SIZE_T_CEILING) {
    OPENSSL_free(buf);
    return -1;
  }
  /* We don't encode directly into 'dest', because that would be illegal
   * type-punning.  (C99 is smarter than me, C99 is smarter than me...)
   */
  memcpy(dest,buf,len);
  OPENSSL_free(buf);
  return len;
}

/** Decode an ASN.1-encoded public key from <b>str</b>; return the result on
 * success and NULL on failure.
 */
crypto_pk_t *
crypto_pk_asn1_decode(const char *str, size_t len)
{
  RSA *rsa;
  unsigned char *buf;
  const unsigned char *cp;
  cp = buf = tor_malloc(len);
  memcpy(buf,str,len);
  rsa = d2i_RSAPublicKey(NULL, &cp, len);
  tor_free(buf);
  if (!rsa) {
    crypto_openssl_log_errors(LOG_WARN,"decoding public key");
    return NULL;
  }
  return crypto_new_pk_from_openssl_rsa_(rsa);
}

/** ASN.1-encode the private portion of <b>pk</b> into <b>dest</b>.
 * Return -1 on error, or the number of characters used on success.
 */
int
crypto_pk_asn1_encode_private(const crypto_pk_t *pk, char *dest,
                              size_t dest_len)
{
  int len;
  unsigned char *buf = NULL;

  len = i2d_RSAPrivateKey(pk->key, &buf);
  if (len < 0 || buf == NULL)
    return -1;

  if ((size_t)len > dest_len || dest_len > SIZE_T_CEILING) {
    OPENSSL_free(buf);
    return -1;
  }
  /* We don't encode directly into 'dest', because that would be illegal
   * type-punning.  (C99 is smarter than me, C99 is smarter than me...)
   */
  memcpy(dest,buf,len);
  OPENSSL_free(buf);
  return len;
}

/** Check whether any component of a private key is too large in a way that
 * seems likely to make verification too expensive. Return true if it's too
 * long, and false otherwise. */
static bool
rsa_private_key_too_long(RSA *rsa, int max_bits)
{
  const BIGNUM *n, *e, *p, *q, *d, *dmp1, *dmq1, *iqmp;
#ifdef OPENSSL_1_1_API

#if OPENSSL_VERSION_NUMBER >= OPENSSL_V_SERIES(1,1,1)
  n = RSA_get0_n(rsa);
  e = RSA_get0_e(rsa);
  p = RSA_get0_p(rsa);
  q = RSA_get0_q(rsa);
  d = RSA_get0_d(rsa);
  dmp1 = RSA_get0_dmp1(rsa);
  dmq1 = RSA_get0_dmq1(rsa);
  iqmp = RSA_get0_iqmp(rsa);
#else /* !(OPENSSL_VERSION_NUMBER >= OPENSSL_V_SERIES(1,1,1)) */
  /* The accessors above did not exist in openssl 1.1.0. */
  p = q = dmp1 = dmq1 = iqmp = NULL;
  RSA_get0_key(rsa, &n, &e, &d);
#endif /* OPENSSL_VERSION_NUMBER >= OPENSSL_V_SERIES(1,1,1) */

  if (RSA_bits(rsa) > max_bits)
    return true;
#else /* !defined(OPENSSL_1_1_API) */
  n = rsa->n;
  e = rsa->e;
  p = rsa->p;
  q = rsa->q;
  d = rsa->d;
  dmp1 = rsa->dmp1;
  dmq1 = rsa->dmq1;
  iqmp = rsa->iqmp;
#endif /* defined(OPENSSL_1_1_API) */

  if (n && BN_num_bits(n) > max_bits)
    return true;
  if (e && BN_num_bits(e) > max_bits)
    return true;
  if (p && BN_num_bits(p) > max_bits)
    return true;
  if (q && BN_num_bits(q) > max_bits)
    return true;
  if (d && BN_num_bits(d) > max_bits)
    return true;
  if (dmp1 && BN_num_bits(dmp1) > max_bits)
    return true;
  if (dmq1 && BN_num_bits(dmq1) > max_bits)
    return true;
  if (iqmp && BN_num_bits(iqmp) > max_bits)
    return true;

  return false;
}

/** Decode an ASN.1-encoded private key from <b>str</b>; return the result on
 * success and NULL on failure.
 *
 * If <b>max_bits</b> is nonnegative, reject any key longer than max_bits
 * without performing any expensive validation on it.
 */
crypto_pk_t *
crypto_pk_asn1_decode_private(const char *str, size_t len, int max_bits)
{
  RSA *rsa;
  unsigned char *buf;
  const unsigned char *cp;
  cp = buf = tor_malloc(len);
  memcpy(buf,str,len);
  rsa = d2i_RSAPrivateKey(NULL, &cp, len);
  tor_free(buf);
  if (!rsa) {
    crypto_openssl_log_errors(LOG_WARN,"decoding private key");
    return NULL;
  }
  if (max_bits >= 0 && rsa_private_key_too_long(rsa, max_bits)) {
    log_info(LD_CRYPTO, "Private key longer than expected.");
    RSA_free(rsa);
    return NULL;
  }
  crypto_pk_t *result = crypto_new_pk_from_openssl_rsa_(rsa);
  if (! crypto_pk_is_valid_private_key(result)) {
    crypto_pk_free(result);
    return NULL;
  }
  return result;
}
