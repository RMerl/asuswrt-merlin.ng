/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_CHACHA20POLY1305_MODE

/**
  Add AAD to the ChaCha20Poly1305 state
  @param st     The ChaCha20Poly1305 state
  @param in     The additional authentication data to add to the ChaCha20Poly1305 state
  @param inlen  The length of the ChaCha20Poly1305 data.
  @return CRYPT_OK on success
 */
int chacha20poly1305_add_aad(chacha20poly1305_state *st, const unsigned char *in, unsigned long inlen)
{
   int err;

   if (inlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(st != NULL);

   if (st->aadflg == 0) return CRYPT_ERROR;
   if ((err = poly1305_process(&st->poly, in, inlen)) != CRYPT_OK) return err;
   st->aadlen += (ulong64)inlen;
   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
