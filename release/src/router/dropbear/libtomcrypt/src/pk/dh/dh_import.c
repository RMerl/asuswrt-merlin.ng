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
  Import a DH key from a binary packet
  @param in     The packet to read
  @param inlen  The length of the input packet
  @param key    [out] Where to import the key to
  @return CRYPT_OK if successful, on error all allocated memory is freed automatically
*/
int dh_import(const unsigned char *in, unsigned long inlen, dh_key *key)
{
   unsigned char flags[1];
   int err;
   unsigned long version;

   LTC_ARGCHK(in  != NULL);
   LTC_ARGCHK(key != NULL);

   /* init */
   if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
      return err;
   }

   /* find out what type of key it is */
   err = der_decode_sequence_multi(in, inlen,
                                   LTC_ASN1_SHORT_INTEGER, 1UL, &version,
                                   LTC_ASN1_BIT_STRING, 1UL, &flags,
                                   LTC_ASN1_EOL, 0UL, NULL);
   if (err != CRYPT_OK && err != CRYPT_INPUT_TOO_LONG) {
      goto error;
   }

   if (version == 0) {
      if (flags[0] == 1) {
         key->type = PK_PRIVATE;
         if ((err = der_decode_sequence_multi(in, inlen,
                                              LTC_ASN1_SHORT_INTEGER, 1UL, &version,
                                              LTC_ASN1_BIT_STRING,    1UL, flags,
                                              LTC_ASN1_INTEGER,       1UL, key->prime,
                                              LTC_ASN1_INTEGER,       1UL, key->base,
                                              LTC_ASN1_INTEGER,       1UL, key->x,
                                              LTC_ASN1_EOL,           0UL, NULL)) != CRYPT_OK) {
            goto error;
         }
         /* compute public key: y = (base ^ x) mod prime */
         if ((err = mp_exptmod(key->base, key->x, key->prime, key->y)) != CRYPT_OK) {
            goto error;
         }
      }
      else if (flags[0] == 0) {
         key->type = PK_PUBLIC;
         if ((err = der_decode_sequence_multi(in, inlen,
                                              LTC_ASN1_SHORT_INTEGER, 1UL, &version,
                                              LTC_ASN1_BIT_STRING,    1UL, flags,
                                              LTC_ASN1_INTEGER,       1UL, key->prime,
                                              LTC_ASN1_INTEGER,       1UL, key->base,
                                              LTC_ASN1_INTEGER,       1UL, key->y,
                                              LTC_ASN1_EOL,           0UL, NULL)) != CRYPT_OK) {
            goto error;
         }
      }
      else {
         err = CRYPT_INVALID_PACKET;
         goto error;
      }
   }
   else {
      err = CRYPT_INVALID_PACKET;
      goto error;
   }

   /* check public key */
   if ((err = dh_check_pubkey(key)) != CRYPT_OK) {
      goto error;
   }

   return CRYPT_OK;

error:
   dh_free(key);
   return err;
}

#endif /* LTC_MDH */

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
