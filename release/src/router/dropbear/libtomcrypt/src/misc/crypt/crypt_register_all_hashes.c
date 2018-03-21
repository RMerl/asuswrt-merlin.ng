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
  @file crypt_register_all_hashes.c

  Steffen Jaeckel
*/

#define REGISTER_HASH(h) do {\
   LTC_ARGCHK(register_hash(h) != -1); \
} while(0)

int register_all_hashes(void)
{
#ifdef LTC_TIGER
   REGISTER_HASH(&tiger_desc);
#endif
#ifdef LTC_MD2
   REGISTER_HASH(&md2_desc);
#endif
#ifdef LTC_MD4
   REGISTER_HASH(&md4_desc);
#endif
#ifdef LTC_MD5
   REGISTER_HASH(&md5_desc);
#endif
#ifdef LTC_SHA1
   REGISTER_HASH(&sha1_desc);
#endif
#ifdef LTC_SHA224
   REGISTER_HASH(&sha224_desc);
#endif
#ifdef LTC_SHA256
   REGISTER_HASH(&sha256_desc);
#endif
#ifdef LTC_SHA384
   REGISTER_HASH(&sha384_desc);
#endif
#ifdef LTC_SHA512
   REGISTER_HASH(&sha512_desc);
#endif
#ifdef LTC_SHA512_224
   REGISTER_HASH(&sha512_224_desc);
#endif
#ifdef LTC_SHA512_256
   REGISTER_HASH(&sha512_256_desc);
#endif
#ifdef LTC_SHA3
   REGISTER_HASH(&sha3_224_desc);
   REGISTER_HASH(&sha3_256_desc);
   REGISTER_HASH(&sha3_384_desc);
   REGISTER_HASH(&sha3_512_desc);
#endif
#ifdef LTC_RIPEMD128
   REGISTER_HASH(&rmd128_desc);
#endif
#ifdef LTC_RIPEMD160
   REGISTER_HASH(&rmd160_desc);
#endif
#ifdef LTC_RIPEMD256
   REGISTER_HASH(&rmd256_desc);
#endif
#ifdef LTC_RIPEMD320
   REGISTER_HASH(&rmd320_desc);
#endif
#ifdef LTC_WHIRLPOOL
   REGISTER_HASH(&whirlpool_desc);
#endif
#ifdef LTC_BLAKE2S
   REGISTER_HASH(&blake2s_128_desc);
   REGISTER_HASH(&blake2s_160_desc);
   REGISTER_HASH(&blake2s_224_desc);
   REGISTER_HASH(&blake2s_256_desc);
#endif
#ifdef LTC_BLAKE2S
   REGISTER_HASH(&blake2b_160_desc);
   REGISTER_HASH(&blake2b_256_desc);
   REGISTER_HASH(&blake2b_384_desc);
   REGISTER_HASH(&blake2b_512_desc);
#endif
#ifdef LTC_CHC_HASH
   REGISTER_HASH(&chc_desc);
   LTC_ARGCHK(chc_register(find_cipher_any("aes", 8, 16)) == CRYPT_OK);
#endif
   return CRYPT_OK;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
