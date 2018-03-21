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
  @file xcbc_file.c
  XCBC support, process a file, Tom St Denis
*/

#ifdef LTC_XCBC

/**
   XCBC a file
   @param cipher   The index of the cipher desired
   @param key      The secret key
   @param keylen   The length of the secret key (octets)
   @param filename The name of the file you wish to XCBC
   @param out      [out] Where the authentication tag is to be stored
   @param outlen   [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful, CRYPT_NOP if file support has been disabled
*/
int xcbc_file(int cipher,
              const unsigned char *key, unsigned long keylen,
              const char *filename,
                    unsigned char *out, unsigned long *outlen)
{
#ifdef LTC_NO_FILE
   return CRYPT_NOP;
#else
   size_t x;
   int err;
   xcbc_state xcbc;
   FILE *in;
   unsigned char *buf;

   LTC_ARGCHK(key      != NULL);
   LTC_ARGCHK(filename != NULL);
   LTC_ARGCHK(out      != NULL);
   LTC_ARGCHK(outlen   != NULL);

   if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
      return CRYPT_MEM;
   }

   if ((err = xcbc_init(&xcbc, cipher, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   in = fopen(filename, "rb");
   if (in == NULL) {
      err = CRYPT_FILE_NOTFOUND;
      goto LBL_ERR;
   }

   do {
      x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
      if ((err = xcbc_process(&xcbc, buf, (unsigned long)x)) != CRYPT_OK) {
         fclose(in);
         goto LBL_CLEANBUF;
      }
   } while (x == LTC_FILE_READ_BUFSIZE);

   if (fclose(in) != 0) {
      err = CRYPT_ERROR;
      goto LBL_CLEANBUF;
   }

   err = xcbc_done(&xcbc, out, outlen);

LBL_CLEANBUF:
   zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&xcbc, sizeof(xcbc_state));
#endif
   XFREE(buf);
   return err;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
