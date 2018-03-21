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

#define QUARTERROUND(a,b,c,d) \
  x[a] += x[b]; x[d] = ROL(x[d] ^ x[a], 16); \
  x[c] += x[d]; x[b] = ROL(x[b] ^ x[c], 12); \
  x[a] += x[b]; x[d] = ROL(x[d] ^ x[a],  8); \
  x[c] += x[d]; x[b] = ROL(x[b] ^ x[c],  7);

static void _chacha_block(unsigned char *output, const ulong32 *input, int rounds)
{
   ulong32 x[16];
   int i;
   XMEMCPY(x, input, sizeof(x));
   for (i = rounds; i > 0; i -= 2) {
      QUARTERROUND(0, 4, 8,12)
      QUARTERROUND(1, 5, 9,13)
      QUARTERROUND(2, 6,10,14)
      QUARTERROUND(3, 7,11,15)
      QUARTERROUND(0, 5,10,15)
      QUARTERROUND(1, 6,11,12)
      QUARTERROUND(2, 7, 8,13)
      QUARTERROUND(3, 4, 9,14)
   }
   for (i = 0; i < 16; ++i) {
     x[i] += input[i];
     STORE32L(x[i], output + 4 * i);
   }
}

/**
   Encrypt (or decrypt) bytes of ciphertext (or plaintext) with ChaCha
   @param st      The ChaCha state
   @param in      The plaintext (or ciphertext)
   @param inlen   The length of the input (octets)
   @param out     [out] The ciphertext (or plaintext), length inlen
   @return CRYPT_OK if successful
*/
int chacha_crypt(chacha_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out)
{
   unsigned char buf[64];
   unsigned long i, j;

   if (inlen == 0) return CRYPT_OK; /* nothing to do */

   LTC_ARGCHK(st        != NULL);
   LTC_ARGCHK(in        != NULL);
   LTC_ARGCHK(out       != NULL);
   LTC_ARGCHK(st->ivlen != 0);

   if (st->ksleft > 0) {
      j = MIN(st->ksleft, inlen);
      for (i = 0; i < j; ++i, st->ksleft--) out[i] = in[i] ^ st->kstream[64 - st->ksleft];
      inlen -= j;
      if (inlen == 0) return CRYPT_OK;
      out += j;
      in  += j;
   }
   for (;;) {
     _chacha_block(buf, st->input, st->rounds);
     if (st->ivlen == 8) {
       /* IV-64bit, increment 64bit counter */
       if (0 == ++st->input[12] && 0 == ++st->input[13]) return CRYPT_OVERFLOW;
     }
     else {
       /* IV-96bit, increment 32bit counter */
       if (0 == ++st->input[12]) return CRYPT_OVERFLOW;
     }
     if (inlen <= 64) {
       for (i = 0; i < inlen; ++i) out[i] = in[i] ^ buf[i];
       st->ksleft = 64 - inlen;
       for (i = inlen; i < 64; ++i) st->kstream[i] = buf[i];
       return CRYPT_OK;
     }
     for (i = 0; i < 64; ++i) out[i] = in[i] ^ buf[i];
     inlen -= 64;
     out += 64;
     in  += 64;
   }
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
