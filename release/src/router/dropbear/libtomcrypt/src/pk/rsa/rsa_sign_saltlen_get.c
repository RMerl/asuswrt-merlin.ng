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
  @file rsa_sign_saltlen_get.c
  Retrieve the maximum size of the salt, Steffen Jaeckel.
*/

#ifdef LTC_MRSA

/**
  Retrieve the maximum possible size of the salt when creating a PKCS#1 PSS signature.
  @param padding    Type of padding (LTC_PKCS_1_PSS only)
  @param hash_idx   The index of the desired hash
  @param key        The RSA key
  @return The maximum salt length in bytes or INT_MAX on error.
*/
int rsa_sign_saltlen_get_max_ex(int padding, int hash_idx, rsa_key *key)
{
  int ret = INT_MAX;
  LTC_ARGCHK(key != NULL);

  if ((hash_is_valid(hash_idx) == CRYPT_OK) &&
      (padding == LTC_PKCS_1_PSS))
  {
    ret = rsa_get_size(key);
    if (ret < INT_MAX)
    {
      ret -= (hash_descriptor[hash_idx].hashsize + 2);
    } /* if */
  } /* if */

  return ret;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
