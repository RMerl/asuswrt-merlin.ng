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
#include <stdarg.h>

#ifdef LTC_POLY1305

/**
   POLY1305 multiple blocks of memory to produce the authentication tag
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param mac       [out] Destination of the authentication tag
   @param maclen    [in/out] Max size and resulting size of authentication tag
   @param in        The data to POLY1305
   @param inlen     The length of the data to POLY1305 (octets)
   @param ...       tuples of (data,len) pairs to POLY1305, terminated with a (NULL,x) (x=don't care)
   @return CRYPT_OK if successful
*/
int poly1305_memory_multi(const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen, const unsigned char *in,  unsigned long inlen, ...)
{
   poly1305_state st;
   int err;
   va_list args;
   const unsigned char *curptr;
   unsigned long curlen;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);

   va_start(args, inlen);
   curptr = in;
   curlen = inlen;
   if ((err = poly1305_init(&st, key, keylen)) != CRYPT_OK)          { goto LBL_ERR; }
   for (;;) {
      if ((err = poly1305_process(&st, curptr, curlen)) != CRYPT_OK) { goto LBL_ERR; }
      curptr = va_arg(args, const unsigned char*);
      if (curptr == NULL) break;
      curlen = va_arg(args, unsigned long);
   }
   err = poly1305_done(&st, mac, maclen);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&st, sizeof(poly1305_state));
#endif
   va_end(args);
   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
