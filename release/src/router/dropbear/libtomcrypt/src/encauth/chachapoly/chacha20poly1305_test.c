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

int chacha20poly1305_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   chacha20poly1305_state st1, st2;
   unsigned char k[]   = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f };
   unsigned char i12[] = { 0x07, 0x00, 0x00, 0x00, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };
   unsigned char i8[]  = { 0x07, 0x00, 0x00, 0x00, 0x40, 0x41, 0x42, 0x43 };
   unsigned char aad[] = { 0x50, 0x51, 0x52, 0x53, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7 };
   unsigned char enc[] = { 0xD3, 0x1A, 0x8D, 0x34, 0x64, 0x8E, 0x60, 0xDB, 0x7B, 0x86, 0xAF, 0xBC, 0x53, 0xEF, 0x7E, 0xC2,
                           0xA4, 0xAD, 0xED, 0x51, 0x29, 0x6E, 0x08, 0xFE, 0xA9, 0xE2, 0xB5, 0xA7, 0x36, 0xEE, 0x62, 0xD6,
                           0x3D, 0xBE, 0xA4, 0x5E, 0x8C, 0xA9, 0x67, 0x12, 0x82, 0xFA, 0xFB, 0x69, 0xDA, 0x92, 0x72, 0x8B,
                           0x1A, 0x71, 0xDE, 0x0A, 0x9E, 0x06, 0x0B, 0x29, 0x05, 0xD6, 0xA5, 0xB6, 0x7E, 0xCD, 0x3B, 0x36,
                           0x92, 0xDD, 0xBD, 0x7F, 0x2D, 0x77, 0x8B, 0x8C, 0x98, 0x03, 0xAE, 0xE3, 0x28, 0x09, 0x1B, 0x58,
                           0xFA, 0xB3, 0x24, 0xE4, 0xFA, 0xD6, 0x75, 0x94, 0x55, 0x85, 0x80, 0x8B, 0x48, 0x31, 0xD7, 0xBC,
                           0x3F, 0xF4, 0xDE, 0xF0, 0x8E, 0x4B, 0x7A, 0x9D, 0xE5, 0x76, 0xD2, 0x65, 0x86, 0xCE, 0xC6, 0x4B,
                           0x61, 0x16 };
   unsigned char tag[] = { 0x1A, 0xE1, 0x0B, 0x59, 0x4F, 0x09, 0xE2, 0x6A, 0x7E, 0x90, 0x2E, 0xCB, 0xD0, 0x60, 0x06, 0x91 };
   char m[] = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
   unsigned long mlen = strlen(m);
   unsigned long len;
   unsigned char rfc7905_pt[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
   unsigned char rfc7905_enc[] = { 0xE4, 0x62, 0x85, 0xB4, 0x29, 0x95, 0x34, 0x96, 0xAB, 0xFB, 0x67, 0xCD, 0xAE, 0xAC, 0x94, 0x1E };
   unsigned char rfc7905_tag[] = { 0x16, 0x2C, 0x92, 0x48, 0x2A, 0xDB, 0xD3, 0x5D, 0x48, 0xBE, 0xC6, 0xFF, 0x10, 0x9C, 0xBA, 0xE4 };
   unsigned char ct[1000], pt[1000], emac[16], dmac[16];
   int err;

   /* encrypt IV 96bit */
   if ((err = chacha20poly1305_init(&st1, k, sizeof(k))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_setiv(&st1, i12, sizeof(i12))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_add_aad(&st1, aad, sizeof(aad))) != CRYPT_OK) return err;
   /* encrypt piece by piece */
   if ((err = chacha20poly1305_encrypt(&st1, (unsigned char *)m,      25,        ct)) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_encrypt(&st1, (unsigned char *)m + 25, 10,        ct + 25)) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_encrypt(&st1, (unsigned char *)m + 35, 35,        ct + 35)) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_encrypt(&st1, (unsigned char *)m + 70, 5,         ct + 70)) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_encrypt(&st1, (unsigned char *)m + 75, 5,         ct + 75)) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_encrypt(&st1, (unsigned char *)m + 80, mlen - 80, ct + 80)) != CRYPT_OK) return err;
   len = sizeof(emac);
   if ((err = chacha20poly1305_done(&st1, emac, &len)) != CRYPT_OK) return err;

   if (compare_testvector(ct, mlen, enc, sizeof(enc), "ENC-CT", 1) != 0) return CRYPT_FAIL_TESTVECTOR;
   if (compare_testvector(emac, len, tag, sizeof(tag), "ENC-TAG", 2) != 0) return CRYPT_FAIL_TESTVECTOR;

   /* decrypt IV 96bit */
   if ((err = chacha20poly1305_init(&st2, k, sizeof(k))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_setiv(&st2, i12, sizeof(i12))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_add_aad(&st2, aad, sizeof(aad))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_decrypt(&st2, ct,      21,        pt)) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_decrypt(&st2, ct + 21, mlen - 21, pt + 21)) != CRYPT_OK) return err;
   len = sizeof(dmac);
   if ((err = chacha20poly1305_done(&st2, dmac, &len)) != CRYPT_OK) return err;

   if (compare_testvector(pt, mlen, m, mlen, "DEC-PT", 3) != 0) return CRYPT_FAIL_TESTVECTOR;
   if (compare_testvector(dmac, len, tag, sizeof(tag), "DEC-TAG", 4) != 0) return CRYPT_FAIL_TESTVECTOR;

   /* chacha20poly1305_memory - encrypt */
   len = sizeof(emac);
   if ((err = chacha20poly1305_memory(k, sizeof(k), i12, sizeof(i12), aad, sizeof(aad), (unsigned char *)m,
                                      mlen, ct, emac, &len, CHACHA20POLY1305_ENCRYPT)) != CRYPT_OK) return err;
   if (compare_testvector(ct, mlen, enc, sizeof(enc), "ENC-CT2", 1) != 0) return CRYPT_FAIL_TESTVECTOR;
   if (compare_testvector(emac, len, tag, sizeof(tag), "ENC-TAG2", 2) != 0) return CRYPT_FAIL_TESTVECTOR;

   /* chacha20poly1305_memory - decrypt */
   len = sizeof(dmac);
   if ((err = chacha20poly1305_memory(k, sizeof(k), i12, sizeof(i12), aad, sizeof(aad),
                                      ct, mlen, pt, dmac, &len, CHACHA20POLY1305_DECRYPT)) != CRYPT_OK) return err;
   if (compare_testvector(pt, mlen, m, mlen, "DEC-PT2", 3) != 0) return CRYPT_FAIL_TESTVECTOR;
   if (compare_testvector(dmac, len, tag, sizeof(tag), "DEC-TAG2", 4) != 0) return CRYPT_FAIL_TESTVECTOR;

   /* encrypt - rfc7905 */
   if ((err = chacha20poly1305_init(&st1, k, sizeof(k))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_setiv_rfc7905(&st1, i12, sizeof(i12), CONST64(0x1122334455667788))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_add_aad(&st1, aad, sizeof(aad))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_encrypt(&st1, rfc7905_pt, 16, ct)) != CRYPT_OK) return err;
   len = sizeof(emac);
   if ((err = chacha20poly1305_done(&st1, emac, &len)) != CRYPT_OK) return err;

   if (compare_testvector(ct, 16, rfc7905_enc, 16, "ENC-CT3", 1) != 0) return CRYPT_FAIL_TESTVECTOR;
   if (compare_testvector(emac, len, rfc7905_tag, 16, "ENC-TAG3", 2) != 0) return CRYPT_FAIL_TESTVECTOR;

   /* decrypt - rfc7905 */
   if ((err = chacha20poly1305_init(&st1, k, sizeof(k))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_setiv_rfc7905(&st1, i12, sizeof(i12), CONST64(0x1122334455667788))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_add_aad(&st1, aad, sizeof(aad))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_decrypt(&st1, ct, 16, pt)) != CRYPT_OK) return err;
   len = sizeof(dmac);
   if ((err = chacha20poly1305_done(&st1, dmac, &len)) != CRYPT_OK) return err;

   if (compare_testvector(pt, 16, rfc7905_pt, 16, "DEC-CT3", 1) != 0) return CRYPT_FAIL_TESTVECTOR;
   if (compare_testvector(dmac, len, rfc7905_tag, 16, "DEC-TAG3", 2) != 0) return CRYPT_FAIL_TESTVECTOR;

   /* encrypt IV 64bit */
   if ((err = chacha20poly1305_init(&st1, k, sizeof(k))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_setiv(&st1, i8, sizeof(i8))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_add_aad(&st1, aad, sizeof(aad))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_encrypt(&st1, (unsigned char *)m, mlen, ct)) != CRYPT_OK) return err;
   len = sizeof(emac);
   if ((err = chacha20poly1305_done(&st1, emac, &len)) != CRYPT_OK) return err;

   /* decrypt IV 64bit */
   if ((err = chacha20poly1305_init(&st2, k, sizeof(k))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_setiv(&st2, i8, sizeof(i8))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_add_aad(&st2, aad, sizeof(aad))) != CRYPT_OK) return err;
   if ((err = chacha20poly1305_decrypt(&st2, ct, mlen, pt)) != CRYPT_OK) return err;
   len = sizeof(dmac);
   if ((err = chacha20poly1305_done(&st2, dmac, &len)) != CRYPT_OK) return err;

   if (compare_testvector(pt, mlen, m, mlen, "DEC-PT4", 1) != 0) return CRYPT_FAIL_TESTVECTOR;
   if (compare_testvector(dmac, len, emac, len, "DEC-TAG4", 2) != 0) return CRYPT_FAIL_TESTVECTOR;

   return CRYPT_OK;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
