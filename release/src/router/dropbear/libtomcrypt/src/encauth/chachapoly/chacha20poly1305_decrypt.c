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
   Decrypt bytes of ciphertext with ChaCha20Poly1305
   @param st      The ChaCha20Poly1305 state
   @param in      The ciphertext
   @param inlen   The length of the input (octets)
   @param out     [out] The plaintext (length inlen)
   @return CRYPT_OK if successful
*/
int chacha20poly1305_decrypt(chacha20poly1305_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out)
{
   unsigned char padzero[16] = { 0 };
   unsigned long padlen;
   int err;

   if (inlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(st != NULL);

   if (st->aadflg) {
      padlen = 16 - (unsigned long)(st->aadlen % 16);
      if (padlen < 16) {
        if ((err = poly1305_process(&st->poly, padzero, padlen)) != CRYPT_OK) return err;
      }
      st->aadflg = 0; /* no more AAD */
   }
   if (st->aadflg) st->aadflg = 0; /* no more AAD */
   if ((err = poly1305_process(&st->poly, in, inlen)) != CRYPT_OK)         return err;
   if ((err = chacha_crypt(&st->chacha, in, inlen, out)) != CRYPT_OK)      return err;
   st->ctlen += (ulong64)inlen;
   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
