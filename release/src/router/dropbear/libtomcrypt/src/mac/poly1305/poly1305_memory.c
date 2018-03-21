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
   POLY1305 a block of memory to produce the authentication tag
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param in        The data to POLY1305
   @param inlen     The length of the data to POLY1305 (octets)
   @param mac       [out] Destination of the authentication tag
   @param maclen    [in/out] Max size and resulting size of authentication tag
   @return CRYPT_OK if successful
*/
int poly1305_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen)
{
   poly1305_state st;
   int err;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);

   if ((err = poly1305_init(&st, key, keylen))  != CRYPT_OK) { goto LBL_ERR; }
   if ((err = poly1305_process(&st, in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
   err = poly1305_done(&st, mac, maclen);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&st, sizeof(poly1305_state));
#endif
   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
