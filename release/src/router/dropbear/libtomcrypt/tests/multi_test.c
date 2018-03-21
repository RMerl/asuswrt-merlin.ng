/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
/* test the multi helpers... */
#include <tomcrypt_test.h>

int multi_test(void)
{
   unsigned char key[32] = { 0 };
   unsigned char buf[2][MAXBLOCKSIZE];
   unsigned long len, len2;

/* register algos */
   register_hash(&sha256_desc);
   register_cipher(&aes_desc);

/* HASH testing */
   len = sizeof(buf[0]);
   hash_memory(find_hash("sha256"), (unsigned char*)"hello", 5, buf[0], &len);
   len2 = sizeof(buf[0]);
   hash_memory_multi(find_hash("sha256"), buf[1], &len2, (unsigned char*)"hello", 5, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   hash_memory_multi(find_hash("sha256"), buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL, 0);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   hash_memory_multi(find_hash("sha256"), buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }

#ifdef LTC_HMAC
   len = sizeof(buf[0]);
   hmac_memory(find_hash("sha256"), key, 16, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = sizeof(buf[0]);
   hmac_memory_multi(find_hash("sha256"), key, 16, buf[1], &len2, (unsigned char*)"hello", 5UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   hmac_memory_multi(find_hash("sha256"), key, 16, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   hmac_memory_multi(find_hash("sha256"), key, 16, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

#ifdef LTC_OMAC
   len = sizeof(buf[0]);
   omac_memory(find_cipher("aes"), key, 16, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = sizeof(buf[0]);
   omac_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"hello", 5UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   omac_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   omac_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

#ifdef LTC_PMAC
   len = sizeof(buf[0]);
   pmac_memory(find_cipher("aes"), key, 16, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = sizeof(buf[0]);
   pmac_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"hello", 5, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   pmac_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   pmac_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

#ifdef LTC_XCBC
   len = sizeof(buf[0]);
   xcbc_memory(find_cipher("aes"), key, 16, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = sizeof(buf[0]);
   xcbc_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"hello", 5, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   xcbc_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   xcbc_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

#ifdef LTC_F9
   len = sizeof(buf[0]);
   f9_memory(find_cipher("aes"), key, 16, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = sizeof(buf[0]);
   f9_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"hello", 5, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   f9_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   f9_memory_multi(find_cipher("aes"), key, 16, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

#ifdef LTC_PELICAN
   /* TODO: there is no pelican_memory_multi(..) */
#endif

#ifdef LTC_POLY1305
   len = sizeof(buf[0]);
   poly1305_memory(key, 32, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = sizeof(buf[0]);
   poly1305_memory_multi(key, 32, buf[1], &len2, (unsigned char*)"hello", 5, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   poly1305_memory_multi(key, 32, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = sizeof(buf[0]);
   poly1305_memory_multi(key, 32, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

#ifdef LTC_BLAKE2SMAC
   len = 32;
   blake2smac_memory(key, 16, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = 32;
   blake2smac_memory_multi(key, 16, buf[1], &len2, (unsigned char*)"hello", 5, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = 32;
   blake2smac_memory_multi(key, 16, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = 32;
   blake2smac_memory_multi(key, 16, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

#ifdef LTC_BLAKE2BMAC
   len = 64;
   blake2bmac_memory(key, 16, (unsigned char*)"hello", 5, buf[0], &len);
   len2 = 64;
   blake2bmac_memory_multi(key, 16, buf[1], &len2, (unsigned char*)"hello", 5, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = 64;
   blake2bmac_memory_multi(key, 16, buf[1], &len2, (unsigned char*)"he", 2UL, "llo", 3UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
   len2 = 64;
   blake2bmac_memory_multi(key, 16, buf[1], &len2, (unsigned char*)"h", 1UL, "e", 1UL, "l", 1UL, "l", 1UL, "o", 1UL, NULL);
   if (len != len2 || memcmp(buf[0], buf[1], len)) {
      printf("Failed: %d %lu %lu\n", __LINE__, len, len2);
      return CRYPT_FAIL_TESTVECTOR;
   }
#endif

   return CRYPT_OK;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
