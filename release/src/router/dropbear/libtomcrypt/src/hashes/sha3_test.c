/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* based on https://github.com/brainhub/SHA3IUF (public domain) */

#include "tomcrypt.h"

#ifdef LTC_SHA3

int sha3_224_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   unsigned char buf[200], hash[224 / 8];
   int i;
   hash_state c;
   const unsigned char c1 = 0xa3;

   const unsigned char sha3_224_empty[224 / 8] = {
      0x6b, 0x4e, 0x03, 0x42, 0x36, 0x67, 0xdb, 0xb7,
      0x3b, 0x6e, 0x15, 0x45, 0x4f, 0x0e, 0xb1, 0xab,
      0xd4, 0x59, 0x7f, 0x9a, 0x1b, 0x07, 0x8e, 0x3f,
      0x5b, 0x5a, 0x6b, 0xc7
   };

   const unsigned char sha3_224_0xa3_200_times[224 / 8] = {
      0x93, 0x76, 0x81, 0x6a, 0xba, 0x50, 0x3f, 0x72,
      0xf9, 0x6c, 0xe7, 0xeb, 0x65, 0xac, 0x09, 0x5d,
      0xee, 0xe3, 0xbe, 0x4b, 0xf9, 0xbb, 0xc2, 0xa1,
      0xcb, 0x7e, 0x11, 0xe0
   };

   XMEMSET(buf, c1, sizeof(buf));

   /* SHA3-224 on an empty buffer */
   sha3_224_init(&c);
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_224_empty, sizeof(sha3_224_empty), "SHA3-224", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-224 in two steps. [FIPS 202] */
   sha3_224_init(&c);
   sha3_process(&c, buf, sizeof(buf) / 2);
   sha3_process(&c, buf + sizeof(buf) / 2, sizeof(buf) / 2);
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_224_0xa3_200_times, sizeof(sha3_224_0xa3_200_times), "SHA3-224", 1)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-224 byte-by-byte: 200 steps. [FIPS 202] */
   i = 200;
   sha3_224_init(&c);
   while (i--) {
       sha3_process(&c, &c1, 1);
   }
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_224_0xa3_200_times, sizeof(sha3_224_0xa3_200_times), "SHA3-224", 2)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   return CRYPT_OK;
#endif
}

int sha3_256_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   unsigned char buf[200], hash[256 / 8];
   int i;
   hash_state c;
   const unsigned char c1 = 0xa3;

   const unsigned char sha3_256_empty[256 / 8] = {
      0xa7, 0xff, 0xc6, 0xf8, 0xbf, 0x1e, 0xd7, 0x66,
      0x51, 0xc1, 0x47, 0x56, 0xa0, 0x61, 0xd6, 0x62,
      0xf5, 0x80, 0xff, 0x4d, 0xe4, 0x3b, 0x49, 0xfa,
      0x82, 0xd8, 0x0a, 0x4b, 0x80, 0xf8, 0x43, 0x4a
   };
   const unsigned char sha3_256_0xa3_200_times[256 / 8] = {
      0x79, 0xf3, 0x8a, 0xde, 0xc5, 0xc2, 0x03, 0x07,
      0xa9, 0x8e, 0xf7, 0x6e, 0x83, 0x24, 0xaf, 0xbf,
      0xd4, 0x6c, 0xfd, 0x81, 0xb2, 0x2e, 0x39, 0x73,
      0xc6, 0x5f, 0xa1, 0xbd, 0x9d, 0xe3, 0x17, 0x87
   };

   XMEMSET(buf, c1, sizeof(buf));

   /* SHA3-256 on an empty buffer */
   sha3_256_init(&c);
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_256_empty, sizeof(sha3_256_empty), "SHA3-256", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-256 as a single buffer. [FIPS 202] */
   sha3_256_init(&c);
   sha3_process(&c, buf, sizeof(buf));
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_256_0xa3_200_times, sizeof(sha3_256_0xa3_200_times), "SHA3-256", 1)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-256 in two steps. [FIPS 202] */
   sha3_256_init(&c);
   sha3_process(&c, buf, sizeof(buf) / 2);
   sha3_process(&c, buf + sizeof(buf) / 2, sizeof(buf) / 2);
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_256_0xa3_200_times, sizeof(sha3_256_0xa3_200_times), "SHA3-256", 2)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-256 byte-by-byte: 200 steps. [FIPS 202] */
   i = 200;
   sha3_256_init(&c);
   while (i--) {
       sha3_process(&c, &c1, 1);
   }
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_256_0xa3_200_times, sizeof(sha3_256_0xa3_200_times), "SHA3-256", 3)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-256 byte-by-byte: 135 bytes. Input from [Keccak]. Output
    * matched with sha3sum. */
   sha3_256_init(&c);
   sha3_process(&c, (unsigned char*)
           "\xb7\x71\xd5\xce\xf5\xd1\xa4\x1a"
           "\x93\xd1\x56\x43\xd7\x18\x1d\x2a"
           "\x2e\xf0\xa8\xe8\x4d\x91\x81\x2f"
           "\x20\xed\x21\xf1\x47\xbe\xf7\x32"
           "\xbf\x3a\x60\xef\x40\x67\xc3\x73"
           "\x4b\x85\xbc\x8c\xd4\x71\x78\x0f"
           "\x10\xdc\x9e\x82\x91\xb5\x83\x39"
           "\xa6\x77\xb9\x60\x21\x8f\x71\xe7"
           "\x93\xf2\x79\x7a\xea\x34\x94\x06"
           "\x51\x28\x29\x06\x5d\x37\xbb\x55"
           "\xea\x79\x6f\xa4\xf5\x6f\xd8\x89"
           "\x6b\x49\xb2\xcd\x19\xb4\x32\x15"
           "\xad\x96\x7c\x71\x2b\x24\xe5\x03"
           "\x2d\x06\x52\x32\xe0\x2c\x12\x74"
           "\x09\xd2\xed\x41\x46\xb9\xd7\x5d"
           "\x76\x3d\x52\xdb\x98\xd9\x49\xd3"
           "\xb0\xfe\xd6\xa8\x05\x2f\xbb", 1080 / 8);
   sha3_done(&c, hash);
   if(compare_testvector(hash, sizeof(hash),
           "\xa1\x9e\xee\x92\xbb\x20\x97\xb6"
           "\x4e\x82\x3d\x59\x77\x98\xaa\x18"
           "\xbe\x9b\x7c\x73\x6b\x80\x59\xab"
           "\xfd\x67\x79\xac\x35\xac\x81\xb5", 256 / 8, "SHA3-256", 4)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   return CRYPT_OK;
#endif
}

int sha3_384_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   unsigned char buf[200], hash[384 / 8];
   int i;
   hash_state c;
   const unsigned char c1 = 0xa3;

   const unsigned char sha3_384_0xa3_200_times[384 / 8] = {
      0x18, 0x81, 0xde, 0x2c, 0xa7, 0xe4, 0x1e, 0xf9,
      0x5d, 0xc4, 0x73, 0x2b, 0x8f, 0x5f, 0x00, 0x2b,
      0x18, 0x9c, 0xc1, 0xe4, 0x2b, 0x74, 0x16, 0x8e,
      0xd1, 0x73, 0x26, 0x49, 0xce, 0x1d, 0xbc, 0xdd,
      0x76, 0x19, 0x7a, 0x31, 0xfd, 0x55, 0xee, 0x98,
      0x9f, 0x2d, 0x70, 0x50, 0xdd, 0x47, 0x3e, 0x8f
   };

   XMEMSET(buf, c1, sizeof(buf));

   /* SHA3-384 as a single buffer. [FIPS 202] */
   sha3_384_init(&c);
   sha3_process(&c, buf, sizeof(buf));
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_384_0xa3_200_times, sizeof(sha3_384_0xa3_200_times), "SHA3-384", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-384 in two steps. [FIPS 202] */
   sha3_384_init(&c);
   sha3_process(&c, buf, sizeof(buf) / 2);
   sha3_process(&c, buf + sizeof(buf) / 2, sizeof(buf) / 2);
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_384_0xa3_200_times, sizeof(sha3_384_0xa3_200_times), "SHA3-384", 1)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-384 byte-by-byte: 200 steps. [FIPS 202] */
   i = 200;
   sha3_384_init(&c);
   while (i--) {
       sha3_process(&c, &c1, 1);
   }
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_384_0xa3_200_times, sizeof(sha3_384_0xa3_200_times), "SHA3-384", 2)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   return CRYPT_OK;
#endif
}

int sha3_512_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   unsigned char buf[200], hash[512 / 8];
   int i;
   hash_state c;
   const unsigned char c1 = 0xa3;

   const unsigned char sha3_512_0xa3_200_times[512 / 8] = {
      0xe7, 0x6d, 0xfa, 0xd2, 0x20, 0x84, 0xa8, 0xb1,
      0x46, 0x7f, 0xcf, 0x2f, 0xfa, 0x58, 0x36, 0x1b,
      0xec, 0x76, 0x28, 0xed, 0xf5, 0xf3, 0xfd, 0xc0,
      0xe4, 0x80, 0x5d, 0xc4, 0x8c, 0xae, 0xec, 0xa8,
      0x1b, 0x7c, 0x13, 0xc3, 0x0a, 0xdf, 0x52, 0xa3,
      0x65, 0x95, 0x84, 0x73, 0x9a, 0x2d, 0xf4, 0x6b,
      0xe5, 0x89, 0xc5, 0x1c, 0xa1, 0xa4, 0xa8, 0x41,
      0x6d, 0xf6, 0x54, 0x5a, 0x1c, 0xe8, 0xba, 0x00
   };

   XMEMSET(buf, c1, sizeof(buf));

   /* SHA3-512 as a single buffer. [FIPS 202] */
   sha3_512_init(&c);
   sha3_process(&c, buf, sizeof(buf));
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_512_0xa3_200_times, sizeof(sha3_512_0xa3_200_times), "SHA3-512", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-512 in two steps. [FIPS 202] */
   sha3_512_init(&c);
   sha3_process(&c, buf, sizeof(buf) / 2);
   sha3_process(&c, buf + sizeof(buf) / 2, sizeof(buf) / 2);
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_512_0xa3_200_times, sizeof(sha3_512_0xa3_200_times), "SHA3-512", 1)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHA3-512 byte-by-byte: 200 steps. [FIPS 202] */
   i = 200;
   sha3_512_init(&c);
   while (i--) {
       sha3_process(&c, &c1, 1);
   }
   sha3_done(&c, hash);
   if (compare_testvector(hash, sizeof(hash), sha3_512_0xa3_200_times, sizeof(sha3_512_0xa3_200_times), "SHA3-512", 2)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   return CRYPT_OK;
#endif
}

int sha3_shake_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   unsigned char buf[200], hash[512];
   int i;
   hash_state c;
   const unsigned char c1 = 0xa3;
   unsigned long len;

   const unsigned char shake256_empty[32] = {
      0xab, 0x0b, 0xae, 0x31, 0x63, 0x39, 0x89, 0x43,
      0x04, 0xe3, 0x58, 0x77, 0xb0, 0xc2, 0x8a, 0x9b,
      0x1f, 0xd1, 0x66, 0xc7, 0x96, 0xb9, 0xcc, 0x25,
      0x8a, 0x06, 0x4a, 0x8f, 0x57, 0xe2, 0x7f, 0x2a
   };
   const unsigned char shake256_0xa3_200_times[32] = {
      0x6a, 0x1a, 0x9d, 0x78, 0x46, 0x43, 0x6e, 0x4d,
      0xca, 0x57, 0x28, 0xb6, 0xf7, 0x60, 0xee, 0xf0,
      0xca, 0x92, 0xbf, 0x0b, 0xe5, 0x61, 0x5e, 0x96,
      0x95, 0x9d, 0x76, 0x71, 0x97, 0xa0, 0xbe, 0xeb
   };
   const unsigned char shake128_empty[32] = {
      0x43, 0xe4, 0x1b, 0x45, 0xa6, 0x53, 0xf2, 0xa5,
      0xc4, 0x49, 0x2c, 0x1a, 0xdd, 0x54, 0x45, 0x12,
      0xdd, 0xa2, 0x52, 0x98, 0x33, 0x46, 0x2b, 0x71,
      0xa4, 0x1a, 0x45, 0xbe, 0x97, 0x29, 0x0b, 0x6f
   };
   const unsigned char shake128_0xa3_200_times[32] = {
      0x44, 0xc9, 0xfb, 0x35, 0x9f, 0xd5, 0x6a, 0xc0,
      0xa9, 0xa7, 0x5a, 0x74, 0x3c, 0xff, 0x68, 0x62,
      0xf1, 0x7d, 0x72, 0x59, 0xab, 0x07, 0x52, 0x16,
      0xc0, 0x69, 0x95, 0x11, 0x64, 0x3b, 0x64, 0x39
   };

   XMEMSET(buf, c1, sizeof(buf));

   /* SHAKE256 on an empty buffer */
   sha3_shake_init(&c, 256);
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake256_empty), shake256_empty, sizeof(shake256_empty), "SHAKE256", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE256 via sha3_shake_memory [FIPS 202] */
   len = 512;
   sha3_shake_memory(256, buf, sizeof(buf), hash, &len);
   if (compare_testvector(hash + 480, sizeof(shake256_0xa3_200_times), shake256_0xa3_200_times, sizeof(shake256_0xa3_200_times), "SHAKE256", 1)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE256 as a single buffer. [FIPS 202] */
   sha3_shake_init(&c, 256);
   sha3_shake_process(&c, buf, sizeof(buf));
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake256_0xa3_200_times), shake256_0xa3_200_times, sizeof(shake256_0xa3_200_times), "SHAKE256", 2)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE256 in two steps. [FIPS 202] */
   sha3_shake_init(&c, 256);
   sha3_shake_process(&c, buf, sizeof(buf) / 2);
   sha3_shake_process(&c, buf + sizeof(buf) / 2, sizeof(buf) / 2);
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake256_0xa3_200_times), shake256_0xa3_200_times, sizeof(shake256_0xa3_200_times), "SHAKE256", 3)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE256 byte-by-byte: 200 steps. [FIPS 202] */
   i = 200;
   sha3_shake_init(&c, 256);
   while (i--) sha3_shake_process(&c, &c1, 1);
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake256_0xa3_200_times), shake256_0xa3_200_times, sizeof(shake256_0xa3_200_times), "SHAKE256", 4)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE128 on an empty buffer */
   sha3_shake_init(&c, 128);
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake128_empty), shake128_empty, sizeof(shake128_empty), "SHAKE128", 0)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE128 via sha3_shake_memory [FIPS 202] */
   len = 512;
   sha3_shake_memory(128, buf, sizeof(buf), hash, &len);
   if (compare_testvector(hash + 480, sizeof(shake128_0xa3_200_times), shake128_0xa3_200_times, sizeof(shake128_0xa3_200_times), "SHAKE128", 1)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE128 as a single buffer. [FIPS 202] */
   sha3_shake_init(&c, 128);
   sha3_shake_process(&c, buf, sizeof(buf));
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake128_0xa3_200_times), shake128_0xa3_200_times, sizeof(shake128_0xa3_200_times), "SHAKE128", 2)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE128 in two steps. [FIPS 202] */
   sha3_shake_init(&c, 128);
   sha3_shake_process(&c, buf, sizeof(buf) / 2);
   sha3_shake_process(&c, buf + sizeof(buf) / 2, sizeof(buf) / 2);
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake128_0xa3_200_times), shake128_0xa3_200_times, sizeof(shake128_0xa3_200_times), "SHAKE128", 3)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   /* SHAKE128 byte-by-byte: 200 steps. [FIPS 202] */
   i = 200;
   sha3_shake_init(&c, 128);
   while (i--) sha3_shake_process(&c, &c1, 1);
   for (i = 0; i < 16; i++) sha3_shake_done(&c, hash, 32); /* get 512 bytes, keep in hash the last 32 */
   if (compare_testvector(hash, sizeof(shake128_0xa3_200_times), shake128_0xa3_200_times, sizeof(shake128_0xa3_200_times), "SHAKE128", 4)) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   return CRYPT_OK;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
