/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* The implementation is based on:
 * Public Domain poly1305 from Andrew Moon
 * https://github.com/floodyberry/poly1305-donna
 */

#include "tomcrypt.h"

#ifdef LTC_POLY1305

/**
  POLY1305 a file
  @param fname    The name of the file you wish to POLY1305
  @param key      The secret key
  @param keylen   The length of the secret key
  @param mac      [out] The POLY1305 authentication tag
  @param maclen   [in/out]  The max size and resulting size of the authentication tag
  @return CRYPT_OK if successful, CRYPT_NOP if file support has been disabled
*/
int poly1305_file(const char *fname, const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen)
{
#ifdef LTC_NO_FILE
   return CRYPT_NOP;
#else
   poly1305_state st;
   FILE *in;
   unsigned char *buf;
   size_t x;
   int err;

   LTC_ARGCHK(fname  != NULL);
   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);

   if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
      return CRYPT_MEM;
   }

   if ((err = poly1305_init(&st, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   in = fopen(fname, "rb");
   if (in == NULL) {
      err = CRYPT_FILE_NOTFOUND;
      goto LBL_ERR;
   }

   do {
      x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
      if ((err = poly1305_process(&st, buf, (unsigned long)x)) != CRYPT_OK) {
         fclose(in);
         goto LBL_CLEANBUF;
      }
   } while (x == LTC_FILE_READ_BUFSIZE);

   if (fclose(in) != 0) {
      err = CRYPT_ERROR;
      goto LBL_CLEANBUF;
   }

   err = poly1305_done(&st, mac, maclen);

LBL_CLEANBUF:
   zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&st, sizeof(poly1305_state));
#endif
   XFREE(buf);
   return err;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
