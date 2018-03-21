/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_BLAKE2BMAC

/**
   BLAKE2B MAC a block of memory to produce the authentication tag
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param in        The data to BLAKE2B MAC
   @param inlen     The length of the data to BLAKE2B MAC (octets)
   @param mac       [out] Destination of the authentication tag
   @param maclen    [in/out] Max size and resulting size of authentication tag
   @return CRYPT_OK if successful
*/
int blake2bmac_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen)
{
   blake2bmac_state st;
   int err;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);

   if ((err = blake2bmac_init(&st, *maclen, key, keylen))  != CRYPT_OK) { goto LBL_ERR; }
   if ((err = blake2bmac_process(&st, in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
   err = blake2bmac_done(&st, mac, maclen);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&st, sizeof(blake2bmac_state));
#endif
   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
