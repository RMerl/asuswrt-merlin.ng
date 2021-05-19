/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_digest.c
 * \brief Block of functions related with digest and xof utilities and
 * operations.
 **/

#include "lib/container/smartlist.h"
#include "lib/crypt_ops/crypto_digest.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"

#include "keccak-tiny/keccak-tiny.h"

#include <stdlib.h>
#include <string.h>

#include "lib/arch/bytes.h"

/** Set the common_digests_t in <b>ds_out</b> to contain every digest on the
 * <b>len</b> bytes in <b>m</b> that we know how to compute.  Return 0 on
 * success, -1 on failure. */
int
crypto_common_digests(common_digests_t *ds_out, const char *m, size_t len)
{
  tor_assert(ds_out);
  memset(ds_out, 0, sizeof(*ds_out));
  if (crypto_digest(ds_out->d[DIGEST_SHA1], m, len) < 0)
    return -1;
  if (crypto_digest256(ds_out->d[DIGEST_SHA256], m, len, DIGEST_SHA256) < 0)
    return -1;

  return 0;
}

/** Return the name of an algorithm, as used in directory documents. */
const char *
crypto_digest_algorithm_get_name(digest_algorithm_t alg)
{
  switch (alg) {
    case DIGEST_SHA1:
      return "sha1";
    case DIGEST_SHA256:
      return "sha256";
    case DIGEST_SHA512:
      return "sha512";
    case DIGEST_SHA3_256:
      return "sha3-256";
    case DIGEST_SHA3_512:
      return "sha3-512";
      // LCOV_EXCL_START
    default:
      tor_fragile_assert();
      return "??unknown_digest??";
      // LCOV_EXCL_STOP
  }
}

/** Given the name of a digest algorithm, return its integer value, or -1 if
 * the name is not recognized. */
int
crypto_digest_algorithm_parse_name(const char *name)
{
  if (!strcmp(name, "sha1"))
    return DIGEST_SHA1;
  else if (!strcmp(name, "sha256"))
    return DIGEST_SHA256;
  else if (!strcmp(name, "sha512"))
    return DIGEST_SHA512;
  else if (!strcmp(name, "sha3-256"))
    return DIGEST_SHA3_256;
  else if (!strcmp(name, "sha3-512"))
    return DIGEST_SHA3_512;
  else
    return -1;
}

/** Given an algorithm, return the digest length in bytes. */
size_t
crypto_digest_algorithm_get_length(digest_algorithm_t alg)
{
  switch (alg) {
    case DIGEST_SHA1:
      return DIGEST_LEN;
    case DIGEST_SHA256:
      return DIGEST256_LEN;
    case DIGEST_SHA512:
      return DIGEST512_LEN;
    case DIGEST_SHA3_256:
      return DIGEST256_LEN;
    case DIGEST_SHA3_512:
      return DIGEST512_LEN;
    default:
      tor_assert(0);              // LCOV_EXCL_LINE
      return 0; /* Unreachable */ // LCOV_EXCL_LINE
  }
}

/** Compute a MAC using SHA3-256 of <b>msg_len</b> bytes in <b>msg</b> using a
 * <b>key</b> of length <b>key_len</b> and a <b>salt</b> of length
 * <b>salt_len</b>. Store the result of <b>len_out</b> bytes in in
 * <b>mac_out</b>. This function can't fail. */
void
crypto_mac_sha3_256(uint8_t *mac_out, size_t len_out,
                    const uint8_t *key, size_t key_len,
                    const uint8_t *msg, size_t msg_len)
{
  crypto_digest_t *digest;

  const uint64_t key_len_netorder = tor_htonll(key_len);

  tor_assert(mac_out);
  tor_assert(key);
  tor_assert(msg);

  digest = crypto_digest256_new(DIGEST_SHA3_256);

  /* Order matters here that is any subsystem using this function should
   * expect this very precise ordering in the MAC construction. */
  crypto_digest_add_bytes(digest, (const char *) &key_len_netorder,
                          sizeof(key_len_netorder));
  crypto_digest_add_bytes(digest, (const char *) key, key_len);
  crypto_digest_add_bytes(digest, (const char *) msg, msg_len);
  crypto_digest_get_digest(digest, (char *) mac_out, len_out);
  crypto_digest_free(digest);
}

/* xof functions  */

/** Internal state for a eXtendable-Output Function (XOF). */
struct crypto_xof_t {
#ifdef OPENSSL_HAS_SHAKE3_EVP
  /* XXXX We can't enable this yet, because OpenSSL's
   * DigestFinalXOF function can't be called repeatedly on the same
   * XOF.
   *
   * We could in theory use the undocumented SHA3_absorb and SHA3_squeeze
   * functions, but let's not mess with undocumented OpenSSL internals any
   * more than we have to.
   *
   * We could also revise our XOF code so that it only allows a single
   * squeeze operation; we don't require streaming squeeze operations
   * outside the tests yet.
   */
  EVP_MD_CTX *ctx;
#else /* !defined(OPENSSL_HAS_SHAKE3_EVP) */
  /**
   * State of the Keccak sponge for the SHAKE-256 computation.
   **/
  keccak_state s;
#endif /* defined(OPENSSL_HAS_SHAKE3_EVP) */
};

/** Allocate a new XOF object backed by SHAKE-256.  The security level
 * provided is a function of the length of the output used.  Read and
 * understand FIPS-202 A.2 "Additional Consideration for Extendable-Output
 * Functions" before using this construct.
 */
crypto_xof_t *
crypto_xof_new(void)
{
  crypto_xof_t *xof;
  xof = tor_malloc(sizeof(crypto_xof_t));
#ifdef OPENSSL_HAS_SHAKE256
  xof->ctx = EVP_MD_CTX_new();
  tor_assert(xof->ctx);
  int r = EVP_DigestInit(xof->ctx, EVP_shake256());
  tor_assert(r == 1);
#else /* !defined(OPENSSL_HAS_SHAKE256) */
  keccak_xof_init(&xof->s, 256);
#endif /* defined(OPENSSL_HAS_SHAKE256) */
  return xof;
}

/** Absorb bytes into a XOF object.  Must not be called after a call to
 * crypto_xof_squeeze_bytes() for the same instance, and will assert
 * if attempted.
 */
void
crypto_xof_add_bytes(crypto_xof_t *xof, const uint8_t *data, size_t len)
{
#ifdef OPENSSL_HAS_SHAKE256
  int r = EVP_DigestUpdate(xof->ctx, data, len);
  tor_assert(r == 1);
#else
  int i = keccak_xof_absorb(&xof->s, data, len);
  tor_assert(i == 0);
#endif /* defined(OPENSSL_HAS_SHAKE256) */
}

/** Squeeze bytes out of a XOF object.  Calling this routine will render
 * the XOF instance ineligible to absorb further data.
 */
void
crypto_xof_squeeze_bytes(crypto_xof_t *xof, uint8_t *out, size_t len)
{
#ifdef OPENSSL_HAS_SHAKE256
  int r = EVP_DigestFinalXOF(xof->ctx, out, len);
  tor_assert(r == 1);
#else
  int i = keccak_xof_squeeze(&xof->s, out, len);
  tor_assert(i == 0);
#endif /* defined(OPENSSL_HAS_SHAKE256) */
}

/** Cleanse and deallocate a XOF object. */
void
crypto_xof_free_(crypto_xof_t *xof)
{
  if (!xof)
    return;
#ifdef OPENSSL_HAS_SHAKE256
  if (xof->ctx)
    EVP_MD_CTX_free(xof->ctx);
#endif
  memwipe(xof, 0, sizeof(crypto_xof_t));
  tor_free(xof);
}

/** Compute the XOF (SHAKE256) of a <b>input_len</b> bytes at <b>input</b>,
 * putting <b>output_len</b> bytes at <b>output</b>. */
void
crypto_xof(uint8_t *output, size_t output_len,
           const uint8_t *input, size_t input_len)
{
#ifdef OPENSSL_HAS_SHA3
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  tor_assert(ctx);
  int r = EVP_DigestInit(ctx, EVP_shake256());
  tor_assert(r == 1);
  r = EVP_DigestUpdate(ctx, input, input_len);
  tor_assert(r == 1);
  r = EVP_DigestFinalXOF(ctx, output, output_len);
  tor_assert(r == 1);
  EVP_MD_CTX_free(ctx);
#else /* !defined(OPENSSL_HAS_SHA3) */
  crypto_xof_t *xof = crypto_xof_new();
  crypto_xof_add_bytes(xof, input, input_len);
  crypto_xof_squeeze_bytes(xof, output, output_len);
  crypto_xof_free(xof);
#endif /* defined(OPENSSL_HAS_SHA3) */
}
