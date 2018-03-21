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
  Export a DH key to a binary packet
  @param out    [out] The destination for the key
  @param outlen [in/out] The max size and resulting size of the DH key
  @param type   Which type of key (PK_PRIVATE or PK_PUBLIC)
  @param key    The key you wish to export
  @return CRYPT_OK if successful
*/
int dh_export(unsigned char *out, unsigned long *outlen, int type, dh_key *key)
{
   unsigned char flags[1];
   int err;
   unsigned long version = 0;

   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);
   LTC_ARGCHK(key    != NULL);

   if (type == PK_PRIVATE) {
      /* export x - private key */
      flags[0] = 1;
      err = der_encode_sequence_multi(out, outlen,
                                LTC_ASN1_SHORT_INTEGER, 1UL, &version,
                                LTC_ASN1_BIT_STRING,    1UL, flags,
                                LTC_ASN1_INTEGER,       1UL, key->prime,
                                LTC_ASN1_INTEGER,       1UL, key->base,
                                LTC_ASN1_INTEGER,       1UL, key->x,
                                LTC_ASN1_EOL,           0UL, NULL);
   }
   else {
      /* export y - public key */
      flags[0] = 0;
      err = der_encode_sequence_multi(out, outlen,
                                LTC_ASN1_SHORT_INTEGER, 1UL, &version,
                                LTC_ASN1_BIT_STRING,    1UL, flags,
                                LTC_ASN1_INTEGER,       1UL, key->prime,
                                LTC_ASN1_INTEGER,       1UL, key->base,
                                LTC_ASN1_INTEGER,       1UL, key->y,
                                LTC_ASN1_EOL,           0UL, NULL);
   }

   return err;
}

#endif /* LTC_MDH */

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
