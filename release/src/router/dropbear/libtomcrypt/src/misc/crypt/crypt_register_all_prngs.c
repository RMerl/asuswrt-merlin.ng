/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

/**
  @file crypt_register_all_prngs.c

  Steffen Jaeckel
*/

#define REGISTER_PRNG(h) do {\
   LTC_ARGCHK(register_prng(h) != -1); \
} while(0)

int register_all_prngs(void)
{
#ifdef LTC_YARROW
   REGISTER_PRNG(&yarrow_desc);
#endif
#ifdef LTC_FORTUNA
   REGISTER_PRNG(&fortuna_desc);
#endif
#ifdef LTC_RC4
   REGISTER_PRNG(&rc4_desc);
#endif
#ifdef LTC_CHACHA20_PRNG
   REGISTER_PRNG(&chacha20_prng_desc);
#endif
#ifdef LTC_SOBER128
   REGISTER_PRNG(&sober128_desc);
#endif
#ifdef LTC_SPRNG
   REGISTER_PRNG(&sprng_desc);
#endif

   return CRYPT_OK;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
