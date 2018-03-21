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
  @file rsa_get_size.c
  Retrieve the size of an RSA key, Steffen Jaeckel.
*/

#ifdef LTC_MRSA

/**
  Retrieve the size in bytes of an RSA key.
  @param key      The RSA key
  @return The size in bytes of the RSA key or INT_MAX on error.
*/
int rsa_get_size(rsa_key *key)
{
  int ret = INT_MAX;
  LTC_ARGCHK(key != NULL);

  if (key)
  {
    ret = mp_unsigned_bin_size(key->N);
  } /* if */

  return ret;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
