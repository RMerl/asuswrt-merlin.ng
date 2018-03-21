/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
/* test file related functions */

#include <tomcrypt_test.h>

int file_test(void)
{
#ifdef LTC_NO_FILE
   return CRYPT_NOP;
#else
   unsigned char key[32] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
   unsigned char buf[200];
   unsigned long len;
   const char *fname = "tests/test.key";
   FILE *in;
   int err, isha256, iaes;

   /* expected results */
   unsigned char exp_sha256[32]     = { 0x76, 0xEC, 0x7F, 0xAE, 0xBD, 0xC4, 0x2A, 0x4D, 0xE3, 0x5C, 0xA7, 0x00, 0x24, 0xC2, 0xD2, 0x73,
                                        0xE9, 0xF7, 0x85, 0x6C, 0xA6, 0x16, 0x12, 0xE8, 0x9F, 0x5F, 0x66, 0x35, 0x0B, 0xA8, 0xCF, 0x5F };
   isha256 = find_hash("sha256");
   iaes = find_cipher("aes");

   len = sizeof(buf);
   if ((in = fopen(fname, "rb")) == NULL)                                       return CRYPT_FILE_NOTFOUND;
   err = hash_filehandle(isha256, in, buf, &len);
   fclose(in);
   if (err != CRYPT_OK)                                                         return err;
   if (compare_testvector(buf, len, exp_sha256, 32, "hash_filehandle", 1))      return 1;

   len = sizeof(buf);
   if ((err = hash_file(isha256, fname, buf, &len)) != CRYPT_OK)                return err;
   if (compare_testvector(buf, len, exp_sha256, 32, "hash_file", 1))            return 1;

#ifdef LTC_HMAC
   {
      unsigned char exp_hmacsha256[32] = { 0xE4, 0x07, 0x74, 0x95, 0xF1, 0xF8, 0x5B, 0xB5, 0xF1, 0x4F, 0x7D, 0x4F, 0x59, 0x8E, 0x4B, 0xBC,
                                           0x8F, 0x68, 0xCF, 0xBA, 0x2E, 0xAD, 0xC4, 0x63, 0x9D, 0x7F, 0x02, 0x99, 0x8C, 0x08, 0xAC, 0xC0 };
      len = sizeof(buf);
      if ((err = hmac_file(isha256, fname, key, 32, buf, &len)) != CRYPT_OK)    return err;
      if (compare_testvector(buf, len, exp_hmacsha256, 32, "hmac_file", 1))     return 1;
   }
#endif
#ifdef LTC_OMAC
   {
      unsigned char exp_omacaes[16]    = { 0x50, 0xB4, 0x6C, 0x62, 0xE9, 0xCA, 0x48, 0xFC, 0x38, 0x8D, 0xF4, 0xA2, 0x7D, 0x6A, 0x1E, 0xD8 };
      len = sizeof(buf);
      if ((err = omac_file(iaes, key, 32, fname, buf, &len)) != CRYPT_OK)       return err;
      if (compare_testvector(buf, len, exp_omacaes, 16, "omac_file", 1))        return 1;
   }
#endif
#ifdef LTC_PMAC
   {
      unsigned char exp_pmacaes[16]    = { 0x7D, 0x65, 0xF0, 0x75, 0x4F, 0x8D, 0xE2, 0xB0, 0xE4, 0xFA, 0x54, 0x4E, 0x45, 0x01, 0x36, 0x1B };
      len = sizeof(buf);
      if ((err = pmac_file(iaes, key, 32, fname, buf, &len)) != CRYPT_OK)       return err;
      if (compare_testvector(buf, len, exp_pmacaes, 16, "pmac_file", 1))        return 1;
   }
#endif
#ifdef LTC_XCBC
   {
      unsigned char exp_xcbcaes[16]    = { 0x9C, 0x73, 0xA2, 0xD7, 0x90, 0xA5, 0x86, 0x25, 0x4D, 0x3C, 0x8A, 0x6A, 0x24, 0x6D, 0xD1, 0xAB };
      len = sizeof(buf);
      if ((err = xcbc_file(iaes, key, 32, fname, buf, &len)) != CRYPT_OK)       return err;
      if (compare_testvector(buf, len, exp_xcbcaes, 16, "xcbc_file", 1))        return 1;
   }
#endif
#ifdef LTC_F9_MODE
   {
      unsigned char exp_f9aes[16]      = { 0x6B, 0x6A, 0x18, 0x34, 0x13, 0x8E, 0x01, 0xEF, 0x33, 0x8E, 0x7A, 0x3F, 0x5B, 0x9A, 0xA6, 0x7A };
      len = sizeof(buf);
      if ((err = f9_file(iaes, key, 32, fname, buf, &len)) != CRYPT_OK)         return err;
      if (compare_testvector(buf, len, exp_f9aes, 16, "f9_file", 1))            return 1;
   }
#endif
#ifdef LTC_POLY1305
   {
      unsigned char exp_poly1305[16]   = { 0xD0, 0xC7, 0xFB, 0x13, 0xA8, 0x87, 0x84, 0x23, 0x21, 0xCC, 0xA9, 0x43, 0x81, 0x18, 0x75, 0xBE };
      len = sizeof(buf);
      if ((err = poly1305_file(fname, key, 32, buf, &len)) != CRYPT_OK)         return err;
      if (compare_testvector(buf, len, exp_poly1305, 16, "poly1305_file", 1))   return 1;
   }
#endif
#ifdef LTC_BLAKE2SMAC
   {
      unsigned char exp_blake2smac[16]   = { 0x4f, 0x94, 0x45, 0x15, 0xcd, 0xd1, 0xca, 0x02, 0x1a, 0x0c, 0x7a, 0xe4, 0x6d, 0x2f, 0xe8, 0xb3 };
      len = 16;
      if ((err = blake2smac_file(fname, key, 32, buf, &len)) != CRYPT_OK)             return err;
      if (compare_testvector(buf, len, exp_blake2smac, 16, "exp_blake2smac_file", 1)) return 1;
   }
#endif
#ifdef LTC_BLAKE2BMAC
   {
      unsigned char exp_blake2bmac[16]   = { 0xdf, 0x0e, 0x7a, 0xab, 0x96, 0x6b, 0x75, 0x4e, 0x52, 0x6a, 0x43, 0x96, 0xbd, 0xef, 0xab, 0x44 };
      len = 16;
      if ((err = blake2bmac_file(fname, key, 32, buf, &len)) != CRYPT_OK)             return err;
      if (compare_testvector(buf, len, exp_blake2bmac, 16, "exp_blake2bmac_file", 1)) return 1;
   }
#endif

   return CRYPT_OK;
#endif
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
