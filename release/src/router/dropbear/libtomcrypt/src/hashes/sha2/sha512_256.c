/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
/**
   @param sha512_256.c
   SHA512/256 hash included in sha512.c
*/

#include "tomcrypt.h"

#if defined(LTC_SHA512_256) && defined(LTC_SHA512)

const struct ltc_hash_descriptor sha512_256_desc =
{
    "sha512-256",
    16,
    32,
    128,

    /* OID */
   { 2, 16, 840, 1, 101, 3, 4, 2, 6,  },
   9,

    &sha512_256_init,
    &sha512_process,
    &sha512_256_done,
    &sha512_256_test,
    NULL
};

/**
   Initialize the hash state
   @param md   The hash state you wish to initialize
   @return CRYPT_OK if successful
*/
int sha512_256_init(hash_state * md)
{
    LTC_ARGCHK(md != NULL);

    md->sha512.curlen = 0;
    md->sha512.length = 0;
    md->sha512.state[0] = CONST64(0x22312194FC2BF72C);
    md->sha512.state[1] = CONST64(0x9F555FA3C84C64C2);
    md->sha512.state[2] = CONST64(0x2393B86B6F53B151);
    md->sha512.state[3] = CONST64(0x963877195940EABD);
    md->sha512.state[4] = CONST64(0x96283EE2A88EFFE3);
    md->sha512.state[5] = CONST64(0xBE5E1E2553863992);
    md->sha512.state[6] = CONST64(0x2B0199FC2C85B8AA);
    md->sha512.state[7] = CONST64(0x0EB72DDC81C52CA2);
    return CRYPT_OK;
}

/**
   Terminate the hash to get the digest
   @param md  The hash state
   @param out [out] The destination of the hash (48 bytes)
   @return CRYPT_OK if successful
*/
int sha512_256_done(hash_state * md, unsigned char *out)
{
   unsigned char buf[64];

   LTC_ARGCHK(md  != NULL);
   LTC_ARGCHK(out != NULL);

    if (md->sha512.curlen >= sizeof(md->sha512.buf)) {
       return CRYPT_INVALID_ARG;
    }

   sha512_done(md, buf);
   XMEMCPY(out, buf, 32);
#ifdef LTC_CLEAN_STACK
   zeromem(buf, sizeof(buf));
#endif
   return CRYPT_OK;
}

/**
  Self-test the hash
  @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
*/
int  sha512_256_test(void)
{
 #ifndef LTC_TEST
    return CRYPT_NOP;
 #else
  static const struct {
      const char *msg;
      unsigned char hash[32];
  } tests[] = {
    { "abc",
      { 0x53, 0x04, 0x8E, 0x26, 0x81, 0x94, 0x1E, 0xF9,
        0x9B, 0x2E, 0x29, 0xB7, 0x6B, 0x4C, 0x7D, 0xAB,
        0xE4, 0xC2, 0xD0, 0xC6, 0x34, 0xFC, 0x6D, 0x46,
        0xE0, 0xE2, 0xF1, 0x31, 0x07, 0xE7, 0xAF, 0x23 }
    },
    { "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
      { 0x39, 0x28, 0xE1, 0x84, 0xFB, 0x86, 0x90, 0xF8,
        0x40, 0xDA, 0x39, 0x88, 0x12, 0x1D, 0x31, 0xBE,
        0x65, 0xCB, 0x9D, 0x3E, 0xF8, 0x3E, 0xE6, 0x14,
        0x6F, 0xEA, 0xC8, 0x61, 0xE1, 0x9B, 0x56, 0x3A }
    },
  };

  int i;
  unsigned char tmp[32];
  hash_state md;

  for (i = 0; i < (int)(sizeof(tests) / sizeof(tests[0])); i++) {
      sha512_256_init(&md);
      sha512_256_process(&md, (unsigned char*)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      sha512_256_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "SHA512-265", i)) {
         return CRYPT_FAIL_TESTVECTOR;
      }
  }
  return CRYPT_OK;
 #endif
}

#endif /* defined(LTC_SHA384) && defined(LTC_SHA512) */

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
