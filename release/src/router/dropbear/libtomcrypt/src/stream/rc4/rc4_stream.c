/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_RC4_STREAM

/**
   Initialize an RC4 context (only the key)
   @param st        [out] The destination of the RC4 state
   @param key       The secret key
   @param keylen    The length of the secret key (8 - 256 bytes)
   @return CRYPT_OK if successful
*/
int rc4_stream_setup(rc4_state *st, const unsigned char *key, unsigned long keylen)
{
   unsigned char tmp, *s;
   int x, y;
   unsigned long j;

   LTC_ARGCHK(st  != NULL);
   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(keylen >= 5); /* 40-2048 bits */

   s = st->buf;
   for (x = 0; x < 256; x++) {
      s[x] = x;
   }

   for (j = x = y = 0; x < 256; x++) {
      y = (y + s[x] + key[j++]) & 255;
      if (j == keylen) {
         j = 0;
      }
      tmp = s[x]; s[x] = s[y]; s[y] = tmp;
   }
   st->x = 0;
   st->y = 0;

   return CRYPT_OK;
}

/**
   Encrypt (or decrypt) bytes of ciphertext (or plaintext) with RC4
   @param st      The RC4 state
   @param in      The plaintext (or ciphertext)
   @param inlen   The length of the input (octets)
   @param out     [out] The ciphertext (or plaintext), length inlen
   @return CRYPT_OK if successful
*/
int rc4_stream_crypt(rc4_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out)
{
   unsigned char x, y, *s, tmp;

   LTC_ARGCHK(st  != NULL);
   LTC_ARGCHK(in  != NULL);
   LTC_ARGCHK(out != NULL);

   x = st->x;
   y = st->y;
   s = st->buf;
   while (inlen--) {
      x = (x + 1) & 255;
      y = (y + s[x]) & 255;
      tmp = s[x]; s[x] = s[y]; s[y] = tmp;
      tmp = (s[x] + s[y]) & 255;
      *out++ = *in++ ^ s[tmp];
   }
   st->x = x;
   st->y = y;
   return CRYPT_OK;
}

/**
  Generate a stream of random bytes via RC4
  @param st      The RC420 state
  @param out     [out] The output buffer
  @param outlen  The output length
  @return CRYPT_OK on success
 */
int rc4_stream_keystream(rc4_state *st, unsigned char *out, unsigned long outlen)
{
   if (outlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(out != NULL);
   XMEMSET(out, 0, outlen);
   return rc4_stream_crypt(st, out, outlen, out);
}

/**
  Terminate and clear RC4 state
  @param st      The RC4 state
  @return CRYPT_OK on success
*/
int rc4_stream_done(rc4_state *st)
{
   LTC_ARGCHK(st != NULL);
   XMEMSET(st, 0, sizeof(rc4_state));
   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
