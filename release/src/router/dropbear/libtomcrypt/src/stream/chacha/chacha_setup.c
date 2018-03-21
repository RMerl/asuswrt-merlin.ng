/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* The implementation is based on:
 * chacha-ref.c version 20080118
 * Public domain from D. J. Bernstein
 */

#include "tomcrypt.h"

#ifdef LTC_CHACHA

static const char * const sigma = "expand 32-byte k";
static const char * const tau   = "expand 16-byte k";

/**
   Initialize an ChaCha context (only the key)
   @param st        [out] The destination of the ChaCha state
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param rounds    Number of rounds (e.g. 20 for ChaCha20)
   @return CRYPT_OK if successful
*/
int chacha_setup(chacha_state *st, const unsigned char *key, unsigned long keylen, int rounds)
{
   const char *constants;

   LTC_ARGCHK(st  != NULL);
   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(keylen == 32 || keylen == 16);

   if (rounds == 0) rounds = 20;

   LOAD32L(st->input[4], key + 0);
   LOAD32L(st->input[5], key + 4);
   LOAD32L(st->input[6], key + 8);
   LOAD32L(st->input[7], key + 12);
   if (keylen == 32) { /* 256bit */
      key += 16;
      constants = sigma;
   } else { /* 128bit */
      constants = tau;
   }
   LOAD32L(st->input[8],  key + 0);
   LOAD32L(st->input[9],  key + 4);
   LOAD32L(st->input[10], key + 8);
   LOAD32L(st->input[11], key + 12);
   LOAD32L(st->input[0],  constants + 0);
   LOAD32L(st->input[1],  constants + 4);
   LOAD32L(st->input[2],  constants + 8);
   LOAD32L(st->input[3],  constants + 12);
   st->rounds = rounds; /* e.g. 20 for chacha20 */
   st->ivlen = 0; /* will be set later by chacha_ivctr(32|64) */
   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
