/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file crypto_ope.c
 * @brief A rudimentary order-preserving encryption scheme.
 *
 * To compute the encryption of N, this scheme uses an AES-CTR stream to
 * generate M-byte values, and adds the first N of them together. (+1 each to
 * insure that the ciphertexts are strictly decreasing.)
 *
 * We use this for generating onion service revision counters based on the
 * current time, without leaking the amount of skew in our view of the current
 * time.  MUCH more analysis would be needed before using it for anything
 * else!
 */

#include "orconfig.h"

#define CRYPTO_OPE_PRIVATE
#include "lib/crypt_ops/crypto_ope.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/arch/bytes.h"

#include <string.h>

/**
 * How infrequent should the precomputed values be for this encryption?
 * The choice of this value creates a space/time tradeoff.
 *
 * Note that this value must be a multiple of 16; see
 * ope_get_cipher()
 */
#define SAMPLE_INTERVAL 1024
/** Number of precomputed samples to make for each OPE key. */
#define N_SAMPLES (OPE_INPUT_MAX / SAMPLE_INTERVAL)

struct crypto_ope_t {
  /** An AES key for use with this object. */
  uint8_t key[OPE_KEY_LEN];
  /** Cached intermediate encryption values at SAMPLE_INTERVAL,
   * SAMPLE_INTERVAL*2,...SAMPLE_INTERVAL*N_SAMPLES */
  uint64_t samples[N_SAMPLES];
};

/** The type to add up in order to produce our OPE ciphertexts */
typedef uint16_t ope_val_t;

#ifdef WORDS_BIGENDIAN
/** Convert an OPE value from little-endian. */
static inline ope_val_t
ope_val_from_le(ope_val_t x)
{
  return
    ((x) >> 8) |
    (((x)&0xff) << 8);
}
#else /* !defined(WORDS_BIGENDIAN) */
#define ope_val_from_le(x) (x)
#endif /* defined(WORDS_BIGENDIAN) */

/**
 * Return a new AES256-CTR stream cipher object for <b>ope</b>, ready to yield
 * bytes from the stream at position <b>initial_idx</b>.
 *
 * Note that because the index is converted directly to an IV, it must be a
 * multiple of the AES block size (16).
 */
STATIC crypto_cipher_t *
ope_get_cipher(const crypto_ope_t *ope, uint32_t initial_idx)
{
  uint8_t iv[CIPHER_IV_LEN];
  tor_assert((initial_idx & 0xf) == 0);
  uint32_t n = tor_htonl(initial_idx >> 4);
  memset(iv, 0, sizeof(iv));
  memcpy(iv + CIPHER_IV_LEN - sizeof(n), &n, sizeof(n));

  return crypto_cipher_new_with_iv_and_bits(ope->key,
                                            iv,
                                            OPE_KEY_LEN * 8);
}

/**
 * Retrieve and add the next <b>n</b> values from the stream cipher <b>c</b>,
 * and return their sum.
 *
 * Note that values are taken in little-endian order (for performance on
 * prevalent hardware), and are mapped from range 0..2^n-1 to range 1..2^n (so
 * that each input encrypts to a different output).
 *
 * NOTE: this function is not constant-time.
 */
STATIC uint64_t
sum_values_from_cipher(crypto_cipher_t *c, size_t n)
{
#define BUFSZ 256
  ope_val_t buf[BUFSZ];
  uint64_t total = 0;
  unsigned i;
  while (n >= BUFSZ) {
    memset(buf, 0, sizeof(buf));
    crypto_cipher_crypt_inplace(c, (char*)buf, BUFSZ*sizeof(ope_val_t));

    for (i = 0; i < BUFSZ; ++i) {
      total += ope_val_from_le(buf[i]);
      total += 1;
    }
    n -= BUFSZ;
  }

  memset(buf, 0, n*sizeof(ope_val_t));
  crypto_cipher_crypt_inplace(c, (char*)buf, n*sizeof(ope_val_t));
  for (i = 0; i < n; ++i) {
    total += ope_val_from_le(buf[i]);
    total += 1;
  }

  memset(buf, 0, sizeof(buf));
  return total;
}

/**
 * Return a new crypto_ope_t object, using the provided 256-bit key.
 */
crypto_ope_t *
crypto_ope_new(const uint8_t *key)
{
  crypto_ope_t *ope = tor_malloc_zero(sizeof(crypto_ope_t));
  memcpy(ope->key, key, OPE_KEY_LEN);

  crypto_cipher_t *cipher = ope_get_cipher(ope, 0);

  uint64_t v = 0;
  int i;
  for (i = 0; i < N_SAMPLES; ++i) {
    v += sum_values_from_cipher(cipher, SAMPLE_INTERVAL);
    ope->samples[i] = v;
  }

  crypto_cipher_free(cipher);
  return ope;
}

/** Free all storage held in <b>ope</b>. */
void
crypto_ope_free_(crypto_ope_t *ope)
{
  if (!ope)
    return;
  memwipe(ope, 0, sizeof(*ope));
  tor_free(ope);
}

/**
 * Return the encrypted value corresponding to <b>input</b>.  The input value
 * must be in range 1..OPE_INPUT_MAX.  Returns CRYPTO_OPE_ERROR on an invalid
 * input.
 *
 * NOTE: this function is not constant-time.
 */
uint64_t
crypto_ope_encrypt(const crypto_ope_t *ope, int plaintext)
{
  if (plaintext <= 0 || plaintext > OPE_INPUT_MAX)
    return CRYPTO_OPE_ERROR;

  const int sample_idx = (plaintext / SAMPLE_INTERVAL);
  const int starting_iv = sample_idx * SAMPLE_INTERVAL;
  const int remaining_values =  plaintext - starting_iv;
  uint64_t v;
  if (sample_idx == 0) {
    v = 0;
  } else {
    v = ope->samples[sample_idx - 1];
  }
  crypto_cipher_t *cipher = ope_get_cipher(ope, starting_iv*sizeof(ope_val_t));

  v += sum_values_from_cipher(cipher, remaining_values);

  crypto_cipher_free(cipher);

  return v;
}
