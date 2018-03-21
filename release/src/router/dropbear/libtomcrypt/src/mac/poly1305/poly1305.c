/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* The implementation is based on:
 * Public Domain poly1305 from Andrew Moon
 * https://github.com/floodyberry/poly1305-donna
 */

#include "tomcrypt.h"

#ifdef LTC_POLY1305

/* internal only */
static void _poly1305_block(poly1305_state *st, const unsigned char *in, unsigned long inlen)
{
   const unsigned long hibit = (st->final) ? 0 : (1UL << 24); /* 1 << 128 */
   ulong32 r0,r1,r2,r3,r4;
   ulong32 s1,s2,s3,s4;
   ulong32 h0,h1,h2,h3,h4;
   ulong32 tmp;
   ulong64 d0,d1,d2,d3,d4;
   ulong32 c;

   r0 = st->r[0];
   r1 = st->r[1];
   r2 = st->r[2];
   r3 = st->r[3];
   r4 = st->r[4];

   s1 = r1 * 5;
   s2 = r2 * 5;
   s3 = r3 * 5;
   s4 = r4 * 5;

   h0 = st->h[0];
   h1 = st->h[1];
   h2 = st->h[2];
   h3 = st->h[3];
   h4 = st->h[4];

   while (inlen >= 16) {
      /* h += in[i] */
      LOAD32L(tmp, in+ 0); h0 += (tmp     ) & 0x3ffffff;
      LOAD32L(tmp, in+ 3); h1 += (tmp >> 2) & 0x3ffffff;
      LOAD32L(tmp, in+ 6); h2 += (tmp >> 4) & 0x3ffffff;
      LOAD32L(tmp, in+ 9); h3 += (tmp >> 6) & 0x3ffffff;
      LOAD32L(tmp, in+12); h4 += (tmp >> 8) | hibit;

      /* h *= r */
      d0 = ((ulong64)h0 * r0) + ((ulong64)h1 * s4) + ((ulong64)h2 * s3) + ((ulong64)h3 * s2) + ((ulong64)h4 * s1);
      d1 = ((ulong64)h0 * r1) + ((ulong64)h1 * r0) + ((ulong64)h2 * s4) + ((ulong64)h3 * s3) + ((ulong64)h4 * s2);
      d2 = ((ulong64)h0 * r2) + ((ulong64)h1 * r1) + ((ulong64)h2 * r0) + ((ulong64)h3 * s4) + ((ulong64)h4 * s3);
      d3 = ((ulong64)h0 * r3) + ((ulong64)h1 * r2) + ((ulong64)h2 * r1) + ((ulong64)h3 * r0) + ((ulong64)h4 * s4);
      d4 = ((ulong64)h0 * r4) + ((ulong64)h1 * r3) + ((ulong64)h2 * r2) + ((ulong64)h3 * r1) + ((ulong64)h4 * r0);

      /* (partial) h %= p */
                    c = (ulong32)(d0 >> 26); h0 = (ulong32)d0 & 0x3ffffff;
      d1 += c;      c = (ulong32)(d1 >> 26); h1 = (ulong32)d1 & 0x3ffffff;
      d2 += c;      c = (ulong32)(d2 >> 26); h2 = (ulong32)d2 & 0x3ffffff;
      d3 += c;      c = (ulong32)(d3 >> 26); h3 = (ulong32)d3 & 0x3ffffff;
      d4 += c;      c = (ulong32)(d4 >> 26); h4 = (ulong32)d4 & 0x3ffffff;
      h0 += c * 5;  c =          (h0 >> 26); h0 =          h0 & 0x3ffffff;
      h1 += c;

      in += 16;
      inlen -= 16;
   }

   st->h[0] = h0;
   st->h[1] = h1;
   st->h[2] = h2;
   st->h[3] = h3;
   st->h[4] = h4;
}

/**
   Initialize an POLY1305 context.
   @param st       The POLY1305 state
   @param key      The secret key
   @param keylen   The length of the secret key (octets)
   @return CRYPT_OK if successful
*/
int poly1305_init(poly1305_state *st, const unsigned char *key, unsigned long keylen)
{
   LTC_ARGCHK(st  != NULL);
   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(keylen == 32);

   /* r &= 0xffffffc0ffffffc0ffffffc0fffffff */
   LOAD32L(st->r[0], key +  0); st->r[0] = (st->r[0]     ) & 0x3ffffff;
   LOAD32L(st->r[1], key +  3); st->r[1] = (st->r[1] >> 2) & 0x3ffff03;
   LOAD32L(st->r[2], key +  6); st->r[2] = (st->r[2] >> 4) & 0x3ffc0ff;
   LOAD32L(st->r[3], key +  9); st->r[3] = (st->r[3] >> 6) & 0x3f03fff;
   LOAD32L(st->r[4], key + 12); st->r[4] = (st->r[4] >> 8) & 0x00fffff;

   /* h = 0 */
   st->h[0] = 0;
   st->h[1] = 0;
   st->h[2] = 0;
   st->h[3] = 0;
   st->h[4] = 0;

   /* save pad for later */
   LOAD32L(st->pad[0], key + 16);
   LOAD32L(st->pad[1], key + 20);
   LOAD32L(st->pad[2], key + 24);
   LOAD32L(st->pad[3], key + 28);

   st->leftover = 0;
   st->final = 0;
   return CRYPT_OK;
}

/**
  Process data through POLY1305
  @param st      The POLY1305 state
  @param in      The data to send through HMAC
  @param inlen   The length of the data to HMAC (octets)
  @return CRYPT_OK if successful
*/
int poly1305_process(poly1305_state *st, const unsigned char *in, unsigned long inlen)
{
   unsigned long i;

   if (inlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(st != NULL);
   LTC_ARGCHK(in != NULL);

   /* handle leftover */
   if (st->leftover) {
      unsigned long want = (16 - st->leftover);
      if (want > inlen) want = inlen;
      for (i = 0; i < want; i++) st->buffer[st->leftover + i] = in[i];
      inlen -= want;
      in += want;
      st->leftover += want;
      if (st->leftover < 16) return CRYPT_OK;
      _poly1305_block(st, st->buffer, 16);
      st->leftover = 0;
   }

   /* process full blocks */
   if (inlen >= 16) {
      unsigned long want = (inlen & ~(16 - 1));
      _poly1305_block(st, in, want);
      in += want;
      inlen -= want;
   }

   /* store leftover */
   if (inlen) {
      for (i = 0; i < inlen; i++) st->buffer[st->leftover + i] = in[i];
      st->leftover += inlen;
   }
   return CRYPT_OK;
}

/**
   Terminate a POLY1305 session
   @param st      The POLY1305 state
   @param mac     [out] The destination of the POLY1305 authentication tag
   @param maclen  [in/out]  The max size and resulting size of the POLY1305 authentication tag
   @return CRYPT_OK if successful
*/
int poly1305_done(poly1305_state *st, unsigned char *mac, unsigned long *maclen)
{
   ulong32 h0,h1,h2,h3,h4,c;
   ulong32 g0,g1,g2,g3,g4;
   ulong64 f;
   ulong32 mask;

   LTC_ARGCHK(st     != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);
   LTC_ARGCHK(*maclen >= 16);

   /* process the remaining block */
   if (st->leftover) {
      unsigned long i = st->leftover;
      st->buffer[i++] = 1;
      for (; i < 16; i++) st->buffer[i] = 0;
      st->final = 1;
      _poly1305_block(st, st->buffer, 16);
   }

   /* fully carry h */
   h0 = st->h[0];
   h1 = st->h[1];
   h2 = st->h[2];
   h3 = st->h[3];
   h4 = st->h[4];

                c = h1 >> 26; h1 = h1 & 0x3ffffff;
   h2 +=     c; c = h2 >> 26; h2 = h2 & 0x3ffffff;
   h3 +=     c; c = h3 >> 26; h3 = h3 & 0x3ffffff;
   h4 +=     c; c = h4 >> 26; h4 = h4 & 0x3ffffff;
   h0 += c * 5; c = h0 >> 26; h0 = h0 & 0x3ffffff;
   h1 +=     c;

   /* compute h + -p */
   g0 = h0 + 5; c = g0 >> 26; g0 &= 0x3ffffff;
   g1 = h1 + c; c = g1 >> 26; g1 &= 0x3ffffff;
   g2 = h2 + c; c = g2 >> 26; g2 &= 0x3ffffff;
   g3 = h3 + c; c = g3 >> 26; g3 &= 0x3ffffff;
   g4 = h4 + c - (1UL << 26);

   /* select h if h < p, or h + -p if h >= p */
   mask = (g4 >> 31) - 1;
   g0 &= mask;
   g1 &= mask;
   g2 &= mask;
   g3 &= mask;
   g4 &= mask;
   mask = ~mask;
   h0 = (h0 & mask) | g0;
   h1 = (h1 & mask) | g1;
   h2 = (h2 & mask) | g2;
   h3 = (h3 & mask) | g3;
   h4 = (h4 & mask) | g4;

   /* h = h % (2^128) */
   h0 = ((h0      ) | (h1 << 26)) & 0xffffffff;
   h1 = ((h1 >>  6) | (h2 << 20)) & 0xffffffff;
   h2 = ((h2 >> 12) | (h3 << 14)) & 0xffffffff;
   h3 = ((h3 >> 18) | (h4 <<  8)) & 0xffffffff;

   /* mac = (h + pad) % (2^128) */
   f = (ulong64)h0 + st->pad[0]            ; h0 = (ulong32)f;
   f = (ulong64)h1 + st->pad[1] + (f >> 32); h1 = (ulong32)f;
   f = (ulong64)h2 + st->pad[2] + (f >> 32); h2 = (ulong32)f;
   f = (ulong64)h3 + st->pad[3] + (f >> 32); h3 = (ulong32)f;

   STORE32L(h0, mac +  0);
   STORE32L(h1, mac +  4);
   STORE32L(h2, mac +  8);
   STORE32L(h3, mac + 12);

   /* zero out the state */
   st->h[0] = 0;
   st->h[1] = 0;
   st->h[2] = 0;
   st->h[3] = 0;
   st->h[4] = 0;
   st->r[0] = 0;
   st->r[1] = 0;
   st->r[2] = 0;
   st->r[3] = 0;
   st->r[4] = 0;
   st->pad[0] = 0;
   st->pad[1] = 0;
   st->pad[2] = 0;
   st->pad[3] = 0;

   *maclen = 16;
   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
