/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
/* test the ciphers and hashes using their built-in self-tests */

#include <tomcrypt_test.h>

int cipher_hash_test(void)
{
   int           x;

   /* test ciphers */
   for (x = 0; cipher_descriptor[x].name != NULL; x++) {
      DOX(cipher_descriptor[x].test(), cipher_descriptor[x].name);
   }

   /* stream ciphers */
#ifdef LTC_CHACHA
   DO(chacha_test());
#endif
#ifdef LTC_RC4_STREAM
   DO(rc4_stream_test());
#endif
#ifdef LTC_SOBER128_STREAM
   DO(sober128_stream_test());
#endif

   /* test hashes */
   for (x = 0; hash_descriptor[x].name != NULL; x++) {
      DOX(hash_descriptor[x].test(), hash_descriptor[x].name);
   }

   /* SHAKE128 + SHAKE256 tests are a bit special */
   DOX(sha3_shake_test(), "sha3_shake");

   return 0;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
