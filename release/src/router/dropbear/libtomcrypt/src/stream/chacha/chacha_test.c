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

int chacha_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   unsigned long len;
   unsigned char out[1000];
   /* https://tools.ietf.org/html/rfc7539#section-2.4.2 */
   unsigned char k[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                          0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
   unsigned char n[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00 };
   unsigned char ct[] = { 0x6E, 0x2E, 0x35, 0x9A, 0x25, 0x68, 0xF9, 0x80, 0x41, 0xBA, 0x07, 0x28, 0xDD, 0x0D, 0x69, 0x81,
                          0xE9, 0x7E, 0x7A, 0xEC, 0x1D, 0x43, 0x60, 0xC2, 0x0A, 0x27, 0xAF, 0xCC, 0xFD, 0x9F, 0xAE, 0x0B,
                          0xF9, 0x1B, 0x65, 0xC5, 0x52, 0x47, 0x33, 0xAB, 0x8F, 0x59, 0x3D, 0xAB, 0xCD, 0x62, 0xB3, 0x57,
                          0x16, 0x39, 0xD6, 0x24, 0xE6, 0x51, 0x52, 0xAB, 0x8F, 0x53, 0x0C, 0x35, 0x9F, 0x08, 0x61, 0xD8,
                          0x07, 0xCA, 0x0D, 0xBF, 0x50, 0x0D, 0x6A, 0x61, 0x56, 0xA3, 0x8E, 0x08, 0x8A, 0x22, 0xB6, 0x5E,
                          0x52, 0xBC, 0x51, 0x4D, 0x16, 0xCC, 0xF8, 0x06, 0x81, 0x8C, 0xE9, 0x1A, 0xB7, 0x79, 0x37, 0x36,
                          0x5A, 0xF9, 0x0B, 0xBF, 0x74, 0xA3, 0x5B, 0xE6, 0xB4, 0x0B, 0x8E, 0xED, 0xF2, 0x78, 0x5E, 0x42,
                          0x87, 0x4D };
   char pt[] = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
   chacha_state st;
   int err;

   len = strlen(pt);
   /* crypt piece by piece */
   if ((err = chacha_setup(&st, k, sizeof(k), 20)) != CRYPT_OK)                            return err;
   if ((err = chacha_ivctr32(&st, n, sizeof(n), 1)) != CRYPT_OK)                           return err;
   if ((err = chacha_crypt(&st, (unsigned char*)pt,      35,       out)) != CRYPT_OK)      return err;
   if ((err = chacha_crypt(&st, (unsigned char*)pt + 35, 35,       out + 35)) != CRYPT_OK) return err;
   if ((err = chacha_crypt(&st, (unsigned char*)pt + 70,  5,       out + 70)) != CRYPT_OK) return err;
   if ((err = chacha_crypt(&st, (unsigned char*)pt + 75,  5,       out + 75)) != CRYPT_OK) return err;
   if ((err = chacha_crypt(&st, (unsigned char*)pt + 80, len - 80, out + 80)) != CRYPT_OK) return err;
   if (compare_testvector(out, len, ct, sizeof(ct), "CHACHA-TV1", 1))                      return CRYPT_FAIL_TESTVECTOR;
   /* crypt in one go */
   if ((err = chacha_setup(&st, k, sizeof(k), 20)) != CRYPT_OK)                            return err;
   if ((err = chacha_ivctr32(&st, n, sizeof(n), 1)) != CRYPT_OK)                           return err;
   if ((err = chacha_crypt(&st, (unsigned char*)pt, len, out)) != CRYPT_OK)                return err;
   if (compare_testvector(out, len, ct, sizeof(ct), "CHACHA-TV2", 1))                      return CRYPT_FAIL_TESTVECTOR;
   /* crypt in one go - using chacha_ivctr64() */
   if ((err = chacha_setup(&st, k, sizeof(k), 20)) != CRYPT_OK)                            return err;
   if ((err = chacha_ivctr64(&st, n + 4, sizeof(n) - 4, 1)) != CRYPT_OK)                   return err;
   if ((err = chacha_crypt(&st, (unsigned char*)pt, len, out)) != CRYPT_OK)                return err;
   if (compare_testvector(out, len, ct, sizeof(ct), "CHACHA-TV3", 1))                      return CRYPT_FAIL_TESTVECTOR;

   return CRYPT_OK;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
