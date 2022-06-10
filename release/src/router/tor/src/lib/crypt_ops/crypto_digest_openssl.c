/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_digest_openssl.c
 * \brief Block of functions related with digest and xof utilities and
 * operations (OpenSSL specific implementations).
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

#include "lib/crypt_ops/crypto_openssl_mgt.h"

DISABLE_GCC_WARNING("-Wredundant-decls")

#include <openssl/hmac.h>
#include <openssl/sha.h>

ENABLE_GCC_WARNING("-Wredundant-decls")

/* Crypto digest functions */

/** Compute the SHA1 digest of the <b>len</b> bytes on data stored in
 * <b>m</b>.  Write the DIGEST_LEN byte result into <b>digest</b>.
 * Return 0 on success, -1 on failure.
 */
MOCK_IMPL(int,
crypto_digest,(char *digest, const char *m, size_t len))
{
  tor_assert(m);
  tor_assert(digest);
  if (SHA1((const unsigned char*)m,len,(unsigned char*)digest) == NULL) {
    return -1;
  }
  return 0;
}

/** Compute a 256-bit digest of <b>len</b> bytes in data stored in <b>m</b>,
 * using the algorithm <b>algorithm</b>.  Write the DIGEST_LEN256-byte result
 * into <b>digest</b>.  Return 0 on success, -1 on failure. */
int
crypto_digest256(char *digest, const char *m, size_t len,
                 digest_algorithm_t algorithm)
{
  tor_assert(m);
  tor_assert(digest);
  tor_assert(algorithm == DIGEST_SHA256 || algorithm == DIGEST_SHA3_256);

  int ret = 0;
  if (algorithm == DIGEST_SHA256) {
    ret = (SHA256((const uint8_t*)m,len,(uint8_t*)digest) != NULL);
  } else {
#ifdef OPENSSL_HAS_SHA3
    unsigned int dlen = DIGEST256_LEN;
    ret = EVP_Digest(m, len, (uint8_t*)digest, &dlen, EVP_sha3_256(), NULL);
#else
    ret = (sha3_256((uint8_t *)digest, DIGEST256_LEN,(const uint8_t *)m, len)
           > -1);
#endif /* defined(OPENSSL_HAS_SHA3) */
  }

  if (!ret)
    return -1;
  return 0;
}

/** Compute a 512-bit digest of <b>len</b> bytes in data stored in <b>m</b>,
 * using the algorithm <b>algorithm</b>.  Write the DIGEST_LEN512-byte result
 * into <b>digest</b>.  Return 0 on success, -1 on failure. */
int
crypto_digest512(char *digest, const char *m, size_t len,
                 digest_algorithm_t algorithm)
{
  tor_assert(m);
  tor_assert(digest);
  tor_assert(algorithm == DIGEST_SHA512 || algorithm == DIGEST_SHA3_512);

  int ret = 0;
  if (algorithm == DIGEST_SHA512) {
    ret = (SHA512((const unsigned char*)m,len,(unsigned char*)digest)
           != NULL);
  } else {
#ifdef OPENSSL_HAS_SHA3
    unsigned int dlen = DIGEST512_LEN;
    ret = EVP_Digest(m, len, (uint8_t*)digest, &dlen, EVP_sha3_512(), NULL);
#else
    ret = (sha3_512((uint8_t*)digest, DIGEST512_LEN, (const uint8_t*)m, len)
           > -1);
#endif /* defined(OPENSSL_HAS_SHA3) */
  }

  if (!ret)
    return -1;
  return 0;
}

/** Intermediate information about the digest of a stream of data. */
struct crypto_digest_t {
  digest_algorithm_t algorithm; /**< Which algorithm is in use? */
   /** State for the digest we're using.  Only one member of the
    * union is usable, depending on the value of <b>algorithm</b>. Note also
    * that space for other members might not even be allocated!
    */
  union {
    SHA_CTX sha1; /**< state for SHA1 */
    SHA256_CTX sha2; /**< state for SHA256 */
    SHA512_CTX sha512; /**< state for SHA512 */
#ifdef OPENSSL_HAS_SHA3
    EVP_MD_CTX *md;
#else
    keccak_state sha3; /**< state for SHA3-[256,512] */
#endif
  } d;
};

#ifdef TOR_UNIT_TESTS

digest_algorithm_t
crypto_digest_get_algorithm(crypto_digest_t *digest)
{
  tor_assert(digest);

  return digest->algorithm;
}

#endif /* defined(TOR_UNIT_TESTS) */

/**
 * Return the number of bytes we need to malloc in order to get a
 * crypto_digest_t for <b>alg</b>, or the number of bytes we need to wipe
 * when we free one.
 */
static size_t
crypto_digest_alloc_bytes(digest_algorithm_t alg)
{
  /** Helper: returns the number of bytes in the 'f' field of 'st' */
#define STRUCT_FIELD_SIZE(st, f) (sizeof( ((st*)0)->f ))
  /** Gives the length of crypto_digest_t through the end of the field 'd' */
#define END_OF_FIELD(f) (offsetof(crypto_digest_t, f) + \
                         STRUCT_FIELD_SIZE(crypto_digest_t, f))
  switch (alg) {
    case DIGEST_SHA1:
      return END_OF_FIELD(d.sha1);
    case DIGEST_SHA256:
      return END_OF_FIELD(d.sha2);
    case DIGEST_SHA512:
      return END_OF_FIELD(d.sha512);
#ifdef OPENSSL_HAS_SHA3
    case DIGEST_SHA3_256: FALLTHROUGH;
    case DIGEST_SHA3_512:
      return END_OF_FIELD(d.md);
#else
    case DIGEST_SHA3_256: FALLTHROUGH;
    case DIGEST_SHA3_512:
      return END_OF_FIELD(d.sha3);
#endif /* defined(OPENSSL_HAS_SHA3) */
    default:
      tor_assert(0); // LCOV_EXCL_LINE
      return 0;      // LCOV_EXCL_LINE
  }
#undef END_OF_FIELD
#undef STRUCT_FIELD_SIZE
}

/**
 * Internal function: create and return a new digest object for 'algorithm'.
 * Does not typecheck the algorithm.
 */
static crypto_digest_t *
crypto_digest_new_internal(digest_algorithm_t algorithm)
{
  crypto_digest_t *r = tor_malloc(crypto_digest_alloc_bytes(algorithm));
  r->algorithm = algorithm;

  switch (algorithm)
    {
    case DIGEST_SHA1:
      SHA1_Init(&r->d.sha1);
      break;
    case DIGEST_SHA256:
      SHA256_Init(&r->d.sha2);
      break;
    case DIGEST_SHA512:
      SHA512_Init(&r->d.sha512);
      break;
#ifdef OPENSSL_HAS_SHA3
    case DIGEST_SHA3_256:
      r->d.md = EVP_MD_CTX_new();
      if (!EVP_DigestInit(r->d.md, EVP_sha3_256())) {
        crypto_digest_free(r);
        return NULL;
      }
      break;
    case DIGEST_SHA3_512:
      r->d.md = EVP_MD_CTX_new();
      if (!EVP_DigestInit(r->d.md, EVP_sha3_512())) {
        crypto_digest_free(r);
        return NULL;
      }
      break;
#else /* !defined(OPENSSL_HAS_SHA3) */
    case DIGEST_SHA3_256:
      keccak_digest_init(&r->d.sha3, 256);
      break;
    case DIGEST_SHA3_512:
      keccak_digest_init(&r->d.sha3, 512);
      break;
#endif /* defined(OPENSSL_HAS_SHA3) */
    default:
      tor_assert_unreached();
    }

  return r;
}

/** Allocate and return a new digest object to compute SHA1 digests.
 */
crypto_digest_t *
crypto_digest_new(void)
{
  return crypto_digest_new_internal(DIGEST_SHA1);
}

/** Allocate and return a new digest object to compute 256-bit digests
 * using <b>algorithm</b>.
 *
 * C_RUST_COUPLED: `external::crypto_digest::crypto_digest256_new`
 * C_RUST_COUPLED: `crypto::digest::Sha256::default`
 */
crypto_digest_t *
crypto_digest256_new(digest_algorithm_t algorithm)
{
  tor_assert(algorithm == DIGEST_SHA256 || algorithm == DIGEST_SHA3_256);
  return crypto_digest_new_internal(algorithm);
}

/** Allocate and return a new digest object to compute 512-bit digests
 * using <b>algorithm</b>. */
crypto_digest_t *
crypto_digest512_new(digest_algorithm_t algorithm)
{
  tor_assert(algorithm == DIGEST_SHA512 || algorithm == DIGEST_SHA3_512);
  return crypto_digest_new_internal(algorithm);
}

/** Deallocate a digest object.
 */
void
crypto_digest_free_(crypto_digest_t *digest)
{
  if (!digest)
    return;
#ifdef OPENSSL_HAS_SHA3
  if (digest->algorithm == DIGEST_SHA3_256 ||
      digest->algorithm == DIGEST_SHA3_512) {
    if (digest->d.md) {
      EVP_MD_CTX_free(digest->d.md);
    }
  }
#endif /* defined(OPENSSL_HAS_SHA3) */
  size_t bytes = crypto_digest_alloc_bytes(digest->algorithm);
  memwipe(digest, 0, bytes);
  tor_free(digest);
}

/** Add <b>len</b> bytes from <b>data</b> to the digest object.
 *
 * C_RUST_COUPLED: `external::crypto_digest::crypto_digest_add_bytess`
 * C_RUST_COUPLED: `crypto::digest::Sha256::process`
 */
void
crypto_digest_add_bytes(crypto_digest_t *digest, const char *data,
                        size_t len)
{
  tor_assert(digest);
  tor_assert(data);
  /* Using the SHA*_*() calls directly means we don't support doing
   * SHA in hardware. But so far the delay of getting the question
   * to the hardware, and hearing the answer, is likely higher than
   * just doing it ourselves. Hashes are fast.
   */
  switch (digest->algorithm) {
    case DIGEST_SHA1:
      SHA1_Update(&digest->d.sha1, (void*)data, len);
      break;
    case DIGEST_SHA256:
      SHA256_Update(&digest->d.sha2, (void*)data, len);
      break;
    case DIGEST_SHA512:
      SHA512_Update(&digest->d.sha512, (void*)data, len);
      break;
#ifdef OPENSSL_HAS_SHA3
    case DIGEST_SHA3_256: FALLTHROUGH;
    case DIGEST_SHA3_512: {
      int r = EVP_DigestUpdate(digest->d.md, data, len);
      tor_assert(r);
  }
      break;
#else /* !defined(OPENSSL_HAS_SHA3) */
    case DIGEST_SHA3_256: FALLTHROUGH;
    case DIGEST_SHA3_512:
      keccak_digest_update(&digest->d.sha3, (const uint8_t *)data, len);
      break;
#endif /* defined(OPENSSL_HAS_SHA3) */
    default:
      /* LCOV_EXCL_START */
      tor_fragile_assert();
      break;
      /* LCOV_EXCL_STOP */
  }
}

/** Compute the hash of the data that has been passed to the digest
 * object; write the first out_len bytes of the result to <b>out</b>.
 * <b>out_len</b> must be \<= DIGEST512_LEN.
 *
 * C_RUST_COUPLED: `external::crypto_digest::crypto_digest_get_digest`
 * C_RUST_COUPLED: `impl digest::FixedOutput for Sha256`
 */
void
crypto_digest_get_digest(crypto_digest_t *digest,
                         char *out, size_t out_len)
{
  unsigned char r[DIGEST512_LEN];
  tor_assert(digest);
  tor_assert(out);
  tor_assert(out_len <= crypto_digest_algorithm_get_length(digest->algorithm));

  /* The SHA-3 code handles copying into a temporary ctx, and also can handle
   * short output buffers by truncating appropriately. */
  if (digest->algorithm == DIGEST_SHA3_256 ||
      digest->algorithm == DIGEST_SHA3_512) {
#ifdef OPENSSL_HAS_SHA3
    unsigned dlen = (unsigned)
      crypto_digest_algorithm_get_length(digest->algorithm);
    EVP_MD_CTX *tmp = EVP_MD_CTX_new();
    EVP_MD_CTX_copy(tmp, digest->d.md);
    memset(r, 0xff, sizeof(r));
    int res = EVP_DigestFinal(tmp, r, &dlen);
    EVP_MD_CTX_free(tmp);
    tor_assert(res == 1);
    goto done;
#else /* !defined(OPENSSL_HAS_SHA3) */
    /* Tiny-Keccak handles copying into a temporary ctx, and also can handle
     * short output buffers by truncating appropriately. */
    keccak_digest_sum(&digest->d.sha3, (uint8_t *)out, out_len);
    return;
#endif /* defined(OPENSSL_HAS_SHA3) */
  }

  const size_t alloc_bytes = crypto_digest_alloc_bytes(digest->algorithm);
  crypto_digest_t tmpenv;
  /* memcpy into a temporary ctx, since SHA*_Final clears the context */
  memcpy(&tmpenv, digest, alloc_bytes);
  switch (digest->algorithm) {
    case DIGEST_SHA1:
      SHA1_Final(r, &tmpenv.d.sha1);
      break;
    case DIGEST_SHA256:
      SHA256_Final(r, &tmpenv.d.sha2);
      break;
    case DIGEST_SHA512:
      SHA512_Final(r, &tmpenv.d.sha512);
      break;
//LCOV_EXCL_START
    case DIGEST_SHA3_256: FALLTHROUGH;
    case DIGEST_SHA3_512:
    default:
      log_warn(LD_BUG, "Handling unexpected algorithm %d", digest->algorithm);
      /* This is fatal, because it should never happen. */
      tor_assert_unreached();
      break;
//LCOV_EXCL_STOP
  }
#ifdef OPENSSL_HAS_SHA3
 done:
#endif
  memcpy(out, r, out_len);
  memwipe(r, 0, sizeof(r));
}

/** Allocate and return a new digest object with the same state as
 * <b>digest</b>
 *
 * C_RUST_COUPLED: `external::crypto_digest::crypto_digest_dup`
 * C_RUST_COUPLED: `impl Clone for crypto::digest::Sha256`
 */
crypto_digest_t *
crypto_digest_dup(const crypto_digest_t *digest)
{
  tor_assert(digest);
  const size_t alloc_bytes = crypto_digest_alloc_bytes(digest->algorithm);
  crypto_digest_t *result = tor_memdup(digest, alloc_bytes);

#ifdef OPENSSL_HAS_SHA3
  if (digest->algorithm == DIGEST_SHA3_256 ||
      digest->algorithm == DIGEST_SHA3_512) {
    result->d.md = EVP_MD_CTX_new();
    EVP_MD_CTX_copy(result->d.md, digest->d.md);
  }
#endif /* defined(OPENSSL_HAS_SHA3) */
  return result;
}

/** Temporarily save the state of <b>digest</b> in <b>checkpoint</b>.
 * Asserts that <b>digest</b> is a SHA1 digest object.
 */
void
crypto_digest_checkpoint(crypto_digest_checkpoint_t *checkpoint,
                         const crypto_digest_t *digest)
{
  const size_t bytes = crypto_digest_alloc_bytes(digest->algorithm);
  tor_assert(bytes <= sizeof(checkpoint->mem));
  memcpy(checkpoint->mem, digest, bytes);
}

/** Restore the state of  <b>digest</b> from <b>checkpoint</b>.
 * Asserts that <b>digest</b> is a SHA1 digest object. Requires that the
 * state was previously stored with crypto_digest_checkpoint() */
void
crypto_digest_restore(crypto_digest_t *digest,
                      const crypto_digest_checkpoint_t *checkpoint)
{
  const size_t bytes = crypto_digest_alloc_bytes(digest->algorithm);
  memcpy(digest, checkpoint->mem, bytes);
}

/** Replace the state of the digest object <b>into</b> with the state
 * of the digest object <b>from</b>.  Requires that 'into' and 'from'
 * have the same digest type.
 */
void
crypto_digest_assign(crypto_digest_t *into,
                     const crypto_digest_t *from)
{
  tor_assert(into);
  tor_assert(from);
  tor_assert(into->algorithm == from->algorithm);
  const size_t alloc_bytes = crypto_digest_alloc_bytes(from->algorithm);

#ifdef OPENSSL_HAS_SHA3
  if (from->algorithm == DIGEST_SHA3_256 ||
      from->algorithm == DIGEST_SHA3_512) {
    EVP_MD_CTX_copy(into->d.md, from->d.md);
    return;
  }
#endif /* defined(OPENSSL_HAS_SHA3) */

  memcpy(into,from,alloc_bytes);
}

/** Given a list of strings in <b>lst</b>, set the <b>len_out</b>-byte digest
 * at <b>digest_out</b> to the hash of the concatenation of those strings,
 * plus the optional string <b>append</b>, computed with the algorithm
 * <b>alg</b>.
 * <b>out_len</b> must be \<= DIGEST512_LEN. */
void
crypto_digest_smartlist(char *digest_out, size_t len_out,
                        const smartlist_t *lst,
                        const char *append,
                        digest_algorithm_t alg)
{
  crypto_digest_smartlist_prefix(digest_out, len_out, NULL, lst, append, alg);
}

/** Given a list of strings in <b>lst</b>, set the <b>len_out</b>-byte digest
 * at <b>digest_out</b> to the hash of the concatenation of: the
 * optional string <b>prepend</b>, those strings,
 * and the optional string <b>append</b>, computed with the algorithm
 * <b>alg</b>.
 * <b>len_out</b> must be \<= DIGEST512_LEN. */
void
crypto_digest_smartlist_prefix(char *digest_out, size_t len_out,
                        const char *prepend,
                        const smartlist_t *lst,
                        const char *append,
                        digest_algorithm_t alg)
{
  crypto_digest_t *d = crypto_digest_new_internal(alg);
  if (prepend)
    crypto_digest_add_bytes(d, prepend, strlen(prepend));
  SMARTLIST_FOREACH(lst, const char *, cp,
                    crypto_digest_add_bytes(d, cp, strlen(cp)));
  if (append)
    crypto_digest_add_bytes(d, append, strlen(append));
  crypto_digest_get_digest(d, digest_out, len_out);
  crypto_digest_free(d);
}

/** Compute the HMAC-SHA-256 of the <b>msg_len</b> bytes in <b>msg</b>, using
 * the <b>key</b> of length <b>key_len</b>.  Store the DIGEST256_LEN-byte
 * result in <b>hmac_out</b>. Asserts on failure.
 */
void
crypto_hmac_sha256(char *hmac_out,
                   const char *key, size_t key_len,
                   const char *msg, size_t msg_len)
{
  /* If we've got OpenSSL >=0.9.8 we can use its hmac implementation. */
  tor_assert(key_len < INT_MAX);
  tor_assert(msg_len < INT_MAX);
  tor_assert(hmac_out);
  unsigned char *rv = NULL;
  rv = HMAC(EVP_sha256(), key, (int)key_len, (unsigned char*)msg, (int)msg_len,
            (unsigned char*)hmac_out, NULL);
  tor_assert(rv);
}
