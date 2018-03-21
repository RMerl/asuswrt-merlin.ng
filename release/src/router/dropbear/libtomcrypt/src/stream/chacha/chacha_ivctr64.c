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

/**
  Set IV + counter data to the ChaCha state
  @param st      The ChaCha20 state
  @param iv      The IV data to add
  @param ivlen   The length of the IV (must be 8)
  @param counter 64bit (unsigned) initial counter value
  @return CRYPT_OK on success
 */
int chacha_ivctr64(chacha_state *st, const unsigned char *iv, unsigned long ivlen, ulong64 counter)
{
   LTC_ARGCHK(st != NULL);
   LTC_ARGCHK(iv != NULL);
   /* 64bit IV + 64bit counter */
   LTC_ARGCHK(ivlen == 8);

   st->input[12] = (ulong32)(counter & 0xFFFFFFFF);
   st->input[13] = (ulong32)(counter >> 32);
   LOAD32L(st->input[14], iv + 0);
   LOAD32L(st->input[15], iv + 4);
   st->ksleft = 0;
   st->ivlen = ivlen;
   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
