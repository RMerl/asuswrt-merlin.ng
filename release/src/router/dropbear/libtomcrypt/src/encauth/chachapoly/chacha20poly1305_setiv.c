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
  Set IV + counter data to the ChaCha20Poly1305 state and reset the context
  @param st     The ChaCha20Poly1305 state
  @param iv     The IV data to add
  @param ivlen  The length of the IV (must be 12 or 8)
  @return CRYPT_OK on success
 */
int chacha20poly1305_setiv(chacha20poly1305_state *st, const unsigned char *iv, unsigned long ivlen)
{
   chacha_state tmp_st;
   int i, err;
   unsigned char polykey[32];

   LTC_ARGCHK(st != NULL);
   LTC_ARGCHK(iv != NULL);
   LTC_ARGCHK(ivlen == 12 || ivlen == 8);

   /* set IV for chacha20 */
   if (ivlen == 12) {
      /* IV 96bit */
      if ((err = chacha_ivctr32(&st->chacha, iv, ivlen, 1)) != CRYPT_OK) return err;
   }
   else {
      /* IV 64bit */
      if ((err = chacha_ivctr64(&st->chacha, iv, ivlen, 1)) != CRYPT_OK) return err;
   }

   /* copy chacha20 key to temporary state */
   for(i = 0; i < 12; i++) tmp_st.input[i] = st->chacha.input[i];
   tmp_st.rounds = 20;
   /* set IV */
   if (ivlen == 12) {
      /* IV 32bit */
      if ((err = chacha_ivctr32(&tmp_st, iv, ivlen, 0)) != CRYPT_OK) return err;
   }
   else {
      /* IV 64bit */
      if ((err = chacha_ivctr64(&tmp_st, iv, ivlen, 0)) != CRYPT_OK) return err;
   }
   /* (re)generate new poly1305 key */
   if ((err = chacha_keystream(&tmp_st, polykey, 32)) != CRYPT_OK) return err;
   /* (re)initialise poly1305 */
   if ((err = poly1305_init(&st->poly, polykey, 32)) != CRYPT_OK) return err;
   st->ctlen  = 0;
   st->aadlen = 0;
   st->aadflg = 1;

   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
