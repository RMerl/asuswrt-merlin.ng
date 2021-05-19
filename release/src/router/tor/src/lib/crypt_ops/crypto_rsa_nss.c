/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_rsa.c
 * \brief NSS implementations of our RSA code.
 **/

#include "lib/crypt_ops/crypto_rsa.h"

#include "lib/crypt_ops/crypto_nss_mgt.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/ctime/di_ops.h"
#include "lib/encoding/binascii.h"
#include "lib/fs/files.h"
#include "lib/intmath/cmp.h"
#include "lib/intmath/muldiv.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"

#include <string.h>

#include <keyhi.h>
#include <pk11pub.h>
#include <secder.h>

#ifdef ENABLE_OPENSSL
#include <openssl/rsa.h>
#include <openssl/evp.h>
#endif

/** Declaration for crypto_pk_t structure. */
struct crypto_pk_t
{
  SECKEYPrivateKey *seckey;
  SECKEYPublicKey *pubkey;
};

/** Return true iff <b>key</b> contains the private-key portion of the RSA
 * key. */
int
crypto_pk_key_is_private(const crypto_pk_t *key)
{
  return key && key->seckey;
}

/** used by tortls.c: wrap a SecKEYPublicKey in a crypto_pk_t. Take ownership
 * of the RSA object. */
crypto_pk_t *
crypto_pk_new_from_nss_pubkey(struct SECKEYPublicKeyStr *pub)
{
  crypto_pk_t *result = tor_malloc_zero(sizeof(crypto_pk_t));
  result->pubkey = pub;
  return result;
}

/** Return the SECKEYPublicKey for the provided crypto_pk_t. */
const SECKEYPublicKey *
crypto_pk_get_nss_pubkey(const crypto_pk_t *key)
{
  tor_assert(key);
  return key->pubkey;
}

/** Return the SECKEYPrivateKey for the provided crypto_pk_t, or NULL if it
 * does not exist. */
const SECKEYPrivateKey *
crypto_pk_get_nss_privkey(const crypto_pk_t *key)
{
  tor_assert(key);
  return key->seckey;
}

#ifdef ENABLE_OPENSSL
/** used by tortls.c: wrap an RSA* in a crypto_pk_t. Take ownership of the
 * RSA object. */
crypto_pk_t *
crypto_new_pk_from_openssl_rsa_(RSA *rsa)
{
  crypto_pk_t *pk = NULL;
  unsigned char *buf = NULL;
  int len = i2d_RSAPublicKey(rsa, &buf);
  RSA_free(rsa);

  if (len < 0 || buf == NULL)
    goto end;

  pk = crypto_pk_asn1_decode((const char *)buf, len);

 end:
  if (buf)
    OPENSSL_free(buf);
  return pk;
}

/** Helper, used by tor-gencert.c.  Return the RSA from a
 * crypto_pk_t. */
struct rsa_st *
crypto_pk_get_openssl_rsa_(crypto_pk_t *pk)
{
  size_t buflen = crypto_pk_keysize(pk)*16;
  unsigned char *buf = tor_malloc_zero(buflen);
  const unsigned char *cp = buf;
  RSA *rsa = NULL;

  int used = crypto_pk_asn1_encode_private(pk, (char*)buf, buflen);
  if (used < 0)
    goto end;
  rsa = d2i_RSAPrivateKey(NULL, &cp, used);

 end:
  memwipe(buf, 0, buflen);
  tor_free(buf);
  return rsa;
}

/** used by tortls.c: get an equivalent EVP_PKEY* for a crypto_pk_t.  Iff
 * private is set, include the private-key portion of the key. Return a valid
 * pointer on success, and NULL on failure. */
MOCK_IMPL(struct evp_pkey_st *,
crypto_pk_get_openssl_evp_pkey_,(crypto_pk_t *pk, int private))
{
  size_t buflen = crypto_pk_keysize(pk)*16;
  unsigned char *buf = tor_malloc_zero(buflen);
  const unsigned char *cp = buf;
  RSA *rsa = NULL;
  EVP_PKEY *result = NULL;

  if (private) {
    int len = crypto_pk_asn1_encode_private(pk, (char*)buf, buflen);
    if (len < 0)
      goto end;
    rsa = d2i_RSAPrivateKey(NULL, &cp, len);
  } else {
    int len = crypto_pk_asn1_encode(pk, (char*)buf, buflen);
    if (len < 0)
      goto end;
    rsa = d2i_RSAPublicKey(NULL, &cp, len);
  }
  if (!rsa)
    goto end;

  if (!(result = EVP_PKEY_new()))
    goto end;
  if (!(EVP_PKEY_assign_RSA(result, rsa))) {
    EVP_PKEY_free(result);
    RSA_free(rsa);
    result = NULL;
  }

 end:
  memwipe(buf, 0, buflen);
  tor_free(buf);
  return result;
}
#endif /* defined(ENABLE_OPENSSL) */

/** Allocate and return storage for a public key.  The key itself will not yet
 * be set.
 */
MOCK_IMPL(crypto_pk_t *,
crypto_pk_new,(void))
{
  crypto_pk_t *result = tor_malloc_zero(sizeof(crypto_pk_t));
  return result;
}

/** Release the NSS objects held in <b>key</b> */
static void
crypto_pk_clear(crypto_pk_t *key)
{
  if (key->pubkey)
    SECKEY_DestroyPublicKey(key->pubkey);
  if (key->seckey)
    SECKEY_DestroyPrivateKey(key->seckey);
  memset(key, 0, sizeof(crypto_pk_t));
}

/** Release a reference to an asymmetric key; when all the references
 * are released, free the key.
 */
void
crypto_pk_free_(crypto_pk_t *key)
{
  if (!key)
    return;

  crypto_pk_clear(key);

  tor_free(key);
}

/** Generate a <b>bits</b>-bit new public/private keypair in <b>env</b>.
 * Return 0 on success, -1 on failure.
 */
MOCK_IMPL(int,
crypto_pk_generate_key_with_bits,(crypto_pk_t *key, int bits))
{
  tor_assert(key);

  PK11RSAGenParams params = {
    .keySizeInBits = bits,
    .pe = TOR_RSA_EXPONENT
  };

  int result = -1;
  PK11SlotInfo *slot = PK11_GetBestSlot(CKM_RSA_PKCS_KEY_PAIR_GEN, NULL);
  SECKEYPrivateKey *seckey = NULL;
  SECKEYPublicKey *pubkey = NULL;

  if (!slot) {
    crypto_nss_log_errors(LOG_WARN, "getting slot for RSA keygen");
    goto done;
  }

  seckey = PK11_GenerateKeyPair(slot, CKM_RSA_PKCS_KEY_PAIR_GEN, &params,
                                &pubkey,
                                PR_FALSE /*isPerm */,
                                PR_FALSE /*isSensitive*/,
                                NULL);
  if (seckey == NULL || pubkey == NULL) {
    crypto_nss_log_errors(LOG_WARN, "generating an RSA key");
    goto done;
  }

  crypto_pk_clear(key);
  key->seckey = seckey;
  key->pubkey = pubkey;
  seckey = NULL;
  pubkey = NULL;

  result = 0;
 done:
  if (slot)
    PK11_FreeSlot(slot);
  if (pubkey)
    SECKEY_DestroyPublicKey(pubkey);
  if (seckey)
    SECKEY_DestroyPrivateKey(seckey);

  return result;
}

/** Return true iff <b>env</b> is a valid private key.
 */
int
crypto_pk_is_valid_private_key(const crypto_pk_t *key)
{
  /* We don't need to do validation here, since unlike OpenSSL, NSS won't let
   * us load private keys without validating them. */
  return key && key->seckey;
}

/** Return true iff <b>env</b> contains a public key whose public exponent
 * equals 65537.
 */
int
crypto_pk_public_exponent_ok(const crypto_pk_t *key)
{
  return key &&
    key->pubkey &&
    key->pubkey->keyType == rsaKey &&
    DER_GetUInteger(&key->pubkey->u.rsa.publicExponent) == TOR_RSA_EXPONENT;
}

/** Compare two big-endian integers stored in a and b; return a tristate.
 */
STATIC int
secitem_uint_cmp(const SECItem *a, const SECItem *b)
{
  const unsigned abits = SECKEY_BigIntegerBitLength(a);
  const unsigned bbits = SECKEY_BigIntegerBitLength(b);

  if (abits < bbits)
    return -1;
  else if (abits > bbits)
    return 1;

  /* okay, they have the same number of bits set. Get a pair of aligned
   * pointers to their bytes that are set... */
  const unsigned nbytes = CEIL_DIV(abits, 8);
  tor_assert(nbytes <= a->len);
  tor_assert(nbytes <= b->len);

  const unsigned char *aptr = a->data + (a->len - nbytes);
  const unsigned char *bptr = b->data + (b->len - nbytes);

  /* And compare them. */
  return fast_memcmp(aptr, bptr, nbytes);
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
  char a_is_non_null = (a != NULL) && (a->pubkey != NULL);
  char b_is_non_null = (b != NULL) && (b->pubkey != NULL);
  char an_argument_is_null = !a_is_non_null | !b_is_non_null;

  result = tor_memcmp(&a_is_non_null, &b_is_non_null, sizeof(a_is_non_null));
  if (an_argument_is_null)
    return result;

  // This is all Tor uses with this structure.
  tor_assert(a->pubkey->keyType == rsaKey);
  tor_assert(b->pubkey->keyType == rsaKey);

  const SECItem *a_n, *a_e, *b_n, *b_e;
  a_n = &a->pubkey->u.rsa.modulus;
  b_n = &b->pubkey->u.rsa.modulus;
  a_e = &a->pubkey->u.rsa.publicExponent;
  b_e = &b->pubkey->u.rsa.publicExponent;

  result = secitem_uint_cmp(a_n, b_n);
  if (result)
    return result;
  return secitem_uint_cmp(a_e, b_e);
}

/** Return the size of the public key modulus in <b>env</b>, in bytes. */
size_t
crypto_pk_keysize(const crypto_pk_t *key)
{
  tor_assert(key);
  tor_assert(key->pubkey);
  return SECKEY_PublicKeyStrength(key->pubkey);
}

/** Return the size of the public key modulus of <b>env</b>, in bits. */
int
crypto_pk_num_bits(crypto_pk_t *key)
{
  tor_assert(key);
  tor_assert(key->pubkey);
  return SECKEY_PublicKeyStrengthInBits(key->pubkey);
}

/**
 * Make a copy of <b>key</b> and return it.
 */
crypto_pk_t *
crypto_pk_dup_key(crypto_pk_t *key)
{
  crypto_pk_t *result = crypto_pk_new();
  if (key->pubkey)
    result->pubkey = SECKEY_CopyPublicKey(key->pubkey);
  if (key->seckey)
    result->seckey = SECKEY_CopyPrivateKey(key->seckey);
  return result;
}

/** For testing: replace dest with src.  (Dest must have a refcount
 * of 1) */
void
crypto_pk_assign_public(crypto_pk_t *dest, const crypto_pk_t *src)
{
  crypto_pk_clear(dest);
  if (src->pubkey)
    dest->pubkey = SECKEY_CopyPublicKey(src->pubkey);
}

/** For testing: replace dest with src.  (Dest must have a refcount
 * of 1) */
void
crypto_pk_assign_private(crypto_pk_t *dest, const crypto_pk_t *src)
{
  crypto_pk_clear(dest);
  if (src->pubkey)
    dest->pubkey = SECKEY_CopyPublicKey(src->pubkey);
  if (src->seckey)
    dest->seckey = SECKEY_CopyPrivateKey(src->seckey);
}

/** Make a real honest-to-goodness copy of <b>env</b>, and return it.
 * Returns NULL on failure. */
crypto_pk_t *
crypto_pk_copy_full(crypto_pk_t *key)
{
  // These aren't reference-counted is nss, so it's fine to just
  // use the same function.
  return crypto_pk_dup_key(key);
}

static const CK_RSA_PKCS_OAEP_PARAMS oaep_params = {
            .hashAlg = CKM_SHA_1,
            .mgf = CKG_MGF1_SHA1,
            .source = CKZ_DATA_SPECIFIED,
            .pSourceData = NULL,
            .ulSourceDataLen = 0
};
static const SECItem oaep_item = {
            .type = siBuffer,
            .data = (unsigned char *) &oaep_params,
            .len = sizeof(oaep_params)
};

/** Return the mechanism code and parameters for a given padding method when
 * used with RSA */
static CK_MECHANISM_TYPE
padding_to_mechanism(int padding, SECItem **item_out)
{
  switch (padding) {
    case PK_PKCS1_OAEP_PADDING:
      *item_out = (SECItem *)&oaep_item;
      return CKM_RSA_PKCS_OAEP;
    default:
      tor_assert_unreached();
      *item_out = NULL;
      return CKM_INVALID_MECHANISM;
  }
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
  tor_assert(env);
  tor_assert(to);
  tor_assert(from);
  tor_assert(tolen < INT_MAX);
  tor_assert(fromlen < INT_MAX);

  if (BUG(! env->pubkey))
    return -1;

  unsigned int result_len = 0;
  SECItem *item = NULL;
  CK_MECHANISM_TYPE m = padding_to_mechanism(padding, &item);

  SECStatus s = PK11_PubEncrypt(env->pubkey, m, item,
                                (unsigned char *)to, &result_len,
                                (unsigned int)tolen,
                                (const unsigned char *)from,
                                (unsigned int)fromlen,
                                NULL);
  if (s != SECSuccess) {
    crypto_nss_log_errors(LOG_WARN, "encrypting to an RSA key");
    return -1;
  }

  return (int)result_len;
}

/** Decrypt <b>fromlen</b> bytes from <b>from</b> with the private key
 * in <b>env</b>, using the padding method <b>padding</b>.  On success,
 * write the result to <b>to</b>, and return the number of bytes
 * written.  On failure, return -1.
 *
 * <b>tolen</b> is the number of writable bytes in <b>to</b>, and must be
 * at least the length of the modulus of <b>key</b>.
 */
int
crypto_pk_private_decrypt(crypto_pk_t *key, char *to,
                          size_t tolen,
                          const char *from, size_t fromlen,
                          int padding, int warnOnFailure)
{
  tor_assert(key);
  tor_assert(to);
  tor_assert(from);
  tor_assert(tolen < INT_MAX);
  tor_assert(fromlen < INT_MAX);

  if (!crypto_pk_key_is_private(key))
    return -1; /* Not a private key. */

  unsigned int result_len = 0;
  SECItem *item = NULL;
  CK_MECHANISM_TYPE m = padding_to_mechanism(padding, &item);
  SECStatus s = PK11_PrivDecrypt(key->seckey, m, item,
                                 (unsigned char *)to, &result_len,
                                 (unsigned int)tolen,
                                 (const unsigned char *)from,
                                 (unsigned int)fromlen);

  if (s != SECSuccess) {
    const int severity = warnOnFailure ? LOG_WARN : LOG_INFO;
    crypto_nss_log_errors(severity, "decrypting with an RSA key");
    return -1;
  }

  return (int)result_len;
}

/** Check the signature in <b>from</b> (<b>fromlen</b> bytes long) with the
 * public key in <b>key</b>, using PKCS1 padding.  On success, write the
 * signed data to <b>to</b>, and return the number of bytes written.
 * On failure, return -1.
 *
 * <b>tolen</b> is the number of writable bytes in <b>to</b>, and must be
 * at least the length of the modulus of <b>key</b>.
 */
MOCK_IMPL(int,
crypto_pk_public_checksig,(const crypto_pk_t *key, char *to,
                           size_t tolen,
                           const char *from, size_t fromlen))
{
  tor_assert(key);
  tor_assert(to);
  tor_assert(from);
  tor_assert(tolen < INT_MAX);
  tor_assert(fromlen < INT_MAX);
  tor_assert(key->pubkey);

  SECItem sig = {
                 .type = siBuffer,
                 .data = (unsigned char *) from,
                 .len = (unsigned int) fromlen,
  };
  SECItem dsig = {
                  .type = siBuffer,
                  .data = (unsigned char *) to,
                  .len = (unsigned int) tolen
  };
  SECStatus s;
  s = PK11_VerifyRecover(key->pubkey, &sig, &dsig, NULL);
  if (s != SECSuccess)
    return -1;

  return (int)dsig.len;
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
crypto_pk_private_sign(const crypto_pk_t *key, char *to, size_t tolen,
                       const char *from, size_t fromlen)
{
  tor_assert(key);
  tor_assert(to);
  tor_assert(from);
  tor_assert(tolen < INT_MAX);
  tor_assert(fromlen < INT_MAX);

  if (BUG(!crypto_pk_key_is_private(key)))
    return -1;

  SECItem sig = {
                 .type = siBuffer,
                 .data = (unsigned char *)to,
                 .len = (unsigned int) tolen
  };
  SECItem hash = {
                 .type = siBuffer,
                 .data = (unsigned char *)from,
                 .len = (unsigned int) fromlen
  };
  CK_MECHANISM_TYPE m = CKM_RSA_PKCS;
  SECStatus s = PK11_SignWithMechanism(key->seckey, m, NULL,
                                       &sig, &hash);

  if (s != SECSuccess) {
    crypto_nss_log_errors(LOG_WARN, "signing with an RSA key");
    return -1;
  }

  return (int)sig.len;
}

/* "This has lead to people trading hard-to-find object identifiers and ASN.1
 * definitions like baseball cards" - Peter Gutmann, "X.509 Style Guide". */
static const unsigned char RSA_OID[] = {
  /* RSADSI */ 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  /* PKCS1 */ 0x01, 0x01,
  /* RSA */ 0x01
};

/** ASN.1-encode the public portion of <b>pk</b> into <b>dest</b>.
 * Return -1 on error, or the number of characters used on success.
 */
int
crypto_pk_asn1_encode(const crypto_pk_t *pk, char *dest, size_t dest_len)
{
  tor_assert(pk);
  if (pk->pubkey == NULL)
    return -1;

  CERTSubjectPublicKeyInfo *info;
  info = SECKEY_CreateSubjectPublicKeyInfo(pk->pubkey);
  if (! info)
    return -1;

  const SECItem *item = &info->subjectPublicKey;
  size_t actual_len = (item->len) >> 3; /* bits to bytes */
  size_t n_used = MIN(actual_len, dest_len);
  memcpy(dest, item->data, n_used);

  SECKEY_DestroySubjectPublicKeyInfo(info);
  return (int) n_used;
}

/** Decode an ASN.1-encoded public key from <b>str</b>; return the result on
 * success and NULL on failure.
 */
crypto_pk_t *
crypto_pk_asn1_decode(const char *str, size_t len)
{
  tor_assert(str);
  if (len >= INT_MAX)
    return NULL;
  CERTSubjectPublicKeyInfo info = {
             .algorithm = {
                           .algorithm = {
                                         .type = siDEROID,
                                         .data = (unsigned char *)RSA_OID,
                                         .len = sizeof(RSA_OID)
                                           }
                           },
             .subjectPublicKey = {
                   .type = siBuffer,
                   .data = (unsigned char *)str,
                   .len = (unsigned int)(len << 3) /* bytes to bits */
                                  }
  };

  SECKEYPublicKey *pub = SECKEY_ExtractPublicKey(&info);
  if (pub == NULL)
    return NULL;

  crypto_pk_t *result = crypto_pk_new();
  result->pubkey = pub;
  return result;
}

DISABLE_GCC_WARNING("-Wunused-parameter")

/** Given a crypto_pk_t <b>pk</b>, allocate a new buffer containing the Base64
 * encoding of the DER representation of the private key into the
 * <b>dest_len</b>-byte buffer in <b>dest</b>.
 * Return the number of bytes written on success, -1 on failure.
 */
int
crypto_pk_asn1_encode_private(const crypto_pk_t *pk,
                              char *dest, size_t destlen)
{
  tor_assert(destlen <= INT_MAX);
  if (!crypto_pk_key_is_private(pk))
    return -1;

  SECKEYPrivateKeyInfo *info = PK11_ExportPrivKeyInfo(pk->seckey, NULL);
  if (!info)
    return -1;
  SECItem *item = &info->privateKey;

  if (destlen < item->len) {
    SECKEY_DestroyPrivateKeyInfo(info, PR_TRUE);
    return -1;
  }
  int result = (int)item->len;
  memcpy(dest, item->data, item->len);
  SECKEY_DestroyPrivateKeyInfo(info, PR_TRUE);

  return result;
}

/** Given a buffer containing the DER representation of the
 * private key <b>str</b>, decode and return the result on success, or NULL
 * on failure.
 *
 * If <b>max_bits</b> is nonnegative, reject any key longer than max_bits
 * without performing any expensive validation on it.
 */
crypto_pk_t *
crypto_pk_asn1_decode_private(const char *str, size_t len, int max_bits)
{
  tor_assert(str);
  tor_assert(len < INT_MAX);
  PK11SlotInfo *slot = PK11_GetBestSlot(CKM_RSA_PKCS, NULL);
  if (!slot)
    return NULL;

  SECKEYPrivateKeyInfo info = {
             .algorithm = {
                           .algorithm = {
                                         .type = siBuffer,
                                         .data = (unsigned char *)RSA_OID,
                                         .len = sizeof(RSA_OID)
                                           }
                           },
             .privateKey = {
                            .type = siBuffer,
                            .data = (unsigned char *)str,
                            .len = (int)len,
                            }
  };

  SECStatus s;
  SECKEYPrivateKey *seckey = NULL;

  s = PK11_ImportPrivateKeyInfoAndReturnKey(slot, &info,
                                            NULL /* nickname */,
                                            NULL /* publicValue */,
                                            PR_FALSE /* isPerm */,
                                            PR_FALSE /* isPrivate */,
                                            KU_ALL /* keyUsage */,
                                            &seckey, NULL);

  crypto_pk_t *output = NULL;

  if (s == SECSuccess && seckey) {
    output = crypto_pk_new();
    output->seckey = seckey;
    output->pubkey = SECKEY_ConvertToPublicKey(seckey);
    tor_assert(output->pubkey);
  } else {
    crypto_nss_log_errors(LOG_WARN, "decoding an RSA private key");
  }

  if (! crypto_pk_is_valid_private_key(output)) {
    crypto_pk_free(output);
    output = NULL;
  }

  if (output) {
    const int bits = SECKEY_PublicKeyStrengthInBits(output->pubkey);
    if (max_bits >= 0 && bits > max_bits) {
      log_info(LD_CRYPTO, "Private key longer than expected.");
      crypto_pk_free(output);
      output = NULL;
    }
  }

  if (slot)
    PK11_FreeSlot(slot);

  return output;
}
