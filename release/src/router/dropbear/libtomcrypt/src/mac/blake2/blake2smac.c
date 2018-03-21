/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_BLAKE2SMAC

/**
   Initialize an BLAKE2S MAC context.
   @param st       The BLAKE2S MAC state
   @param outlen   The size of the MAC output (octets)
   @param key      The secret key
   @param keylen   The length of the secret key (octets)
   @return CRYPT_OK if successful
*/
int blake2smac_init(blake2smac_state *st, unsigned long outlen, const unsigned char *key, unsigned long keylen)
{
   LTC_ARGCHK(st  != NULL);
   LTC_ARGCHK(key != NULL);
   return blake2s_init(st, outlen, key, keylen);
}

/**
  Process data through BLAKE2S MAC
  @param st      The BLAKE2S MAC state
  @param in      The data to send through HMAC
  @param inlen   The length of the data to HMAC (octets)
  @return CRYPT_OK if successful
*/
int blake2smac_process(blake2smac_state *st, const unsigned char *in, unsigned long inlen)
{
   if (inlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(st != NULL);
   LTC_ARGCHK(in != NULL);
   return blake2s_process(st, in, inlen);
}

/**
   Terminate a BLAKE2S MAC session
   @param st      The BLAKE2S MAC state
   @param mac     [out] The destination of the BLAKE2S MAC authentication tag
   @param maclen  [in/out]  The max size and resulting size of the BLAKE2S MAC authentication tag
   @return CRYPT_OK if successful
*/
int blake2smac_done(blake2smac_state *st, unsigned char *mac, unsigned long *maclen)
{
   LTC_ARGCHK(st     != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);
   LTC_ARGCHK(*maclen >= st->blake2s.outlen);

   *maclen = st->blake2s.outlen;
   return blake2s_done(st, mac);
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
