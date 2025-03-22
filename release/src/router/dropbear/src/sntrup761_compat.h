/* To be included only by sntrup761.c, provides
 * random and sha512 implementation from Dropbear.
 * Partially based on OpenSSH crypto_api.h */
/*
 * Assembled from generated headers and source files by Markus Friedl.
 * Placed in the public domain.
 */

#include "includes.h"
#include "dbrandom.h"
#include "sntrup761.h"

#if DROPBEAR_SNTRUP761

typedef int8_t crypto_int8;
typedef uint8_t crypto_uint8;
typedef int16_t crypto_int16;
typedef uint16_t crypto_uint16;
typedef int32_t crypto_int32;
typedef uint32_t crypto_uint32;
typedef int64_t crypto_int64;
typedef uint64_t crypto_uint64;

static inline void randombytes(unsigned char* buf, unsigned int len) {
    genrandom(buf, len);
}

static inline uint32_t small_random32(void) {
    uint32_t v;
    genrandom((unsigned char*)&v, sizeof(v));
    return v;
}

static int crypto_hash_sha512(uint8_t *out, const uint8_t *m,
    unsigned long long n)
{
  hash_state hs;

  sha512_init(&hs);
  sha512_process(&hs, m, n);
  return sha512_done(&hs, out);
}

#endif /* DROPBEAR_SNTRUP761 */
