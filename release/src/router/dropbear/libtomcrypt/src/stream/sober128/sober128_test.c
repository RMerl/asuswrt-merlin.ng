/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_SOBER128

int sober128_stream_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   unsigned char key[16] = { 0x74, 0x65, 0x73, 0x74, 0x20, 0x6b, 0x65, 0x79,
                             0x20, 0x31, 0x32, 0x38, 0x62, 0x69, 0x74, 0x73 };
   unsigned char iv[4]   = { 0x00, 0x00, 0x00, 0x00 };
   unsigned char out[20] = { 0x43, 0x50, 0x0c, 0xcf, 0x89, 0x91, 0x9f, 0x1d,
                             0xaa, 0x37, 0x74, 0x95, 0xf4, 0xb4, 0x58, 0xc2,
                             0x40, 0x37, 0x8b, 0xbb };
   int err, len = 20;
   unsigned char  src[20], dst[20];
   sober128_state st;

   XMEMSET(src, 0, len); /* input */
   if ((err = sober128_stream_setup(&st, key, sizeof(key))) != CRYPT_OK) return err;
   if ((err = sober128_stream_setiv(&st, iv, sizeof(iv))) != CRYPT_OK)   return err;
   if ((err = sober128_stream_crypt(&st, src, len, dst)) != CRYPT_OK)    return err;
   if ((err = sober128_stream_done(&st)) != CRYPT_OK)                    return err;
   if (compare_testvector(dst, len, out, len, "SOBER-128", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }
   return CRYPT_OK;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
