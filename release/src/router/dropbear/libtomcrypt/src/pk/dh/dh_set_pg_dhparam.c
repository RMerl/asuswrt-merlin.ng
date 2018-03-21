/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_MDH

/**
  Import DH key parts p and g from dhparam

      dhparam data: openssl dhparam -outform DER -out dhparam.der 2048

  @param dhparam    The DH param DER encoded data
  @param dhparamlen The length of dhparam data
  @param key        [out] Where the newly created DH key will be stored
  @return CRYPT_OK if successful, note: on error all allocated memory will be freed automatically.
*/
int dh_set_pg_dhparam(const unsigned char *dhparam, unsigned long dhparamlen, dh_key *key)
{
   int err;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);
   LTC_ARGCHK(dhparam     != NULL);
   LTC_ARGCHK(dhparamlen  > 0);

   if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
      return err;
   }
   if ((err = der_decode_sequence_multi(dhparam, dhparamlen,
                                        LTC_ASN1_INTEGER, 1UL, key->prime,
                                        LTC_ASN1_INTEGER, 1UL, key->base,
                                        LTC_ASN1_EOL,     0UL, NULL)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   return CRYPT_OK;

LBL_ERR:
   dh_free(key);
   return err;
}

#endif /* LTC_MDH */

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
