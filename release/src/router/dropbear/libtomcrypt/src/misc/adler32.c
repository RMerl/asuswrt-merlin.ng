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
   @file adler32.c
   Adler-32 checksum algorithm
   Written and placed in the public domain by Wei Dai
   Adapted for libtomcrypt by Steffen Jaeckel
*/
#ifdef LTC_ADLER32

static const unsigned long _adler32_base = 65521;

void adler32_init(adler32_state *ctx)
{
   LTC_ARGCHKVD(ctx != NULL);
   ctx->s[0] = 1;
   ctx->s[1] = 0;
}

void adler32_update(adler32_state *ctx, const unsigned char *input, unsigned long length)
{
   unsigned long s1, s2;

   LTC_ARGCHKVD(ctx != NULL);
   LTC_ARGCHKVD(input != NULL);
   s1 = ctx->s[0];
   s2 = ctx->s[1];

   if (length % 8 != 0) {
      do {
         s1 += *input++;
         s2 += s1;
         length--;
      } while (length % 8 != 0);

      if (s1 >= _adler32_base)
         s1 -= _adler32_base;
      s2 %= _adler32_base;
   }

   while (length > 0) {
      s1 += input[0];
      s2 += s1;
      s1 += input[1];
      s2 += s1;
      s1 += input[2];
      s2 += s1;
      s1 += input[3];
      s2 += s1;
      s1 += input[4];
      s2 += s1;
      s1 += input[5];
      s2 += s1;
      s1 += input[6];
      s2 += s1;
      s1 += input[7];
      s2 += s1;

      length -= 8;
      input += 8;

      if (s1 >= _adler32_base)
         s1 -= _adler32_base;
      s2 %= _adler32_base;
   }

   LTC_ARGCHKVD(s1 < _adler32_base);
   LTC_ARGCHKVD(s2 < _adler32_base);

   ctx->s[0] = (unsigned short)s1;
   ctx->s[1] = (unsigned short)s2;
}

void adler32_finish(adler32_state *ctx, void *hash, unsigned long size)
{
   unsigned char* h;

   LTC_ARGCHKVD(ctx != NULL);
   LTC_ARGCHKVD(hash != NULL);

   h = hash;

   switch (size) {
      default:
         h[3] = ctx->s[0] & 0x0ff;
         /* FALLTHROUGH */
      case 3:
         h[2] = (ctx->s[0] >> 8) & 0x0ff;
         /* FALLTHROUGH */
      case 2:
         h[1] = ctx->s[1] & 0x0ff;
         /* FALLTHROUGH */
      case 1:
         h[0] = (ctx->s[1] >> 8) & 0x0ff;
         /* FALLTHROUGH */
      case 0:
         ;
   }
}

int adler32_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   const void* in = "libtomcrypt";
   const unsigned char adler32[] = { 0x1b, 0xe8, 0x04, 0xba };
   unsigned char out[4];
   adler32_state ctx;
   adler32_init(&ctx);
   adler32_update(&ctx, in, strlen(in));
   adler32_finish(&ctx, out, 4);
   if (compare_testvector(adler32, 4, out, 4, "adler32", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }
   return CRYPT_OK;
#endif
}
#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
