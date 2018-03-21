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
   @file dsa_import.c
   DSA implementation, import a DSA key, Tom St Denis
*/

#ifdef LTC_MDSA

/**
   Import a DSA key
   @param in       The binary packet to import from
   @param inlen    The length of the binary packet
   @param key      [out] Where to store the imported key
   @return CRYPT_OK if successful, upon error this function will free all allocated memory
*/
int dsa_import(const unsigned char *in, unsigned long inlen, dsa_key *key)
{
   int           err, stat;
   unsigned long zero = 0;
   unsigned char* tmpbuf = NULL;
   unsigned char flags[1];

   LTC_ARGCHK(in  != NULL);
   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* init key */
   if (mp_init_multi(&key->p, &key->g, &key->q, &key->x, &key->y, NULL) != CRYPT_OK) {
      return CRYPT_MEM;
   }

   /* try to match the old libtomcrypt format */
   err = der_decode_sequence_multi(in, inlen, LTC_ASN1_BIT_STRING, 1UL, flags,
                                              LTC_ASN1_EOL,        0UL, NULL);

   if (err == CRYPT_OK || err == CRYPT_INPUT_TOO_LONG) {
       /* private key */
       if (flags[0] == 1) {
           if ((err = der_decode_sequence_multi(in, inlen,
                                  LTC_ASN1_BIT_STRING,   1UL, flags,
                                  LTC_ASN1_INTEGER,      1UL, key->g,
                                  LTC_ASN1_INTEGER,      1UL, key->p,
                                  LTC_ASN1_INTEGER,      1UL, key->q,
                                  LTC_ASN1_INTEGER,      1UL, key->y,
                                  LTC_ASN1_INTEGER,      1UL, key->x,
                                  LTC_ASN1_EOL,          0UL, NULL)) != CRYPT_OK) {
               goto LBL_ERR;
           }
           key->type = PK_PRIVATE;
           goto LBL_OK;
       }
       /* public key */
       else if (flags[0] == 0) {
           if ((err = der_decode_sequence_multi(in, inlen,
                                      LTC_ASN1_BIT_STRING,   1UL, flags,
                                      LTC_ASN1_INTEGER,      1UL, key->g,
                                      LTC_ASN1_INTEGER,      1UL, key->p,
                                      LTC_ASN1_INTEGER,      1UL, key->q,
                                      LTC_ASN1_INTEGER,      1UL, key->y,
                                      LTC_ASN1_EOL,          0UL, NULL)) != CRYPT_OK) {
               goto LBL_ERR;
           }
           key->type = PK_PUBLIC;
           goto LBL_OK;
       }
       else {
          err = CRYPT_INVALID_PACKET;
          goto LBL_ERR;
       }
   }
   /* get key type */
   if ((err = der_decode_sequence_multi(in, inlen,
                          LTC_ASN1_SHORT_INTEGER, 1UL, &zero,
                          LTC_ASN1_INTEGER,      1UL, key->p,
                          LTC_ASN1_INTEGER,      1UL, key->q,
                          LTC_ASN1_INTEGER,      1UL, key->g,
                          LTC_ASN1_INTEGER,      1UL, key->y,
                          LTC_ASN1_INTEGER,      1UL, key->x,
                          LTC_ASN1_EOL,          0UL, NULL)) == CRYPT_OK) {

       key->type = PK_PRIVATE;
   } else { /* public */
      ltc_asn1_list params[3];
      unsigned long tmpbuf_len = inlen;

      LTC_SET_ASN1(params, 0, LTC_ASN1_INTEGER, key->p, 1UL);
      LTC_SET_ASN1(params, 1, LTC_ASN1_INTEGER, key->q, 1UL);
      LTC_SET_ASN1(params, 2, LTC_ASN1_INTEGER, key->g, 1UL);

      tmpbuf = XCALLOC(1, tmpbuf_len);
      if (tmpbuf == NULL) {
         err = CRYPT_MEM;
         goto LBL_ERR;
      }

      err = der_decode_subject_public_key_info(in, inlen, PKA_DSA,
                                               tmpbuf, &tmpbuf_len,
                                               LTC_ASN1_SEQUENCE, params, 3);
      if (err != CRYPT_OK) {
         XFREE(tmpbuf);
         goto LBL_ERR;
      }

      if ((err=der_decode_integer(tmpbuf, tmpbuf_len, key->y)) != CRYPT_OK) {
         XFREE(tmpbuf);
         goto LBL_ERR;
      }

      XFREE(tmpbuf);
      key->type = PK_PUBLIC;
   }

LBL_OK:
   key->qord = mp_unsigned_bin_size(key->q);

   /* quick p, q, g validation, without primality testing */
   if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if (stat == 0) {
      err = CRYPT_INVALID_PACKET;
      goto LBL_ERR;
   }
   /* validate x, y */
   if ((err = dsa_int_validate_xy(key, &stat)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if (stat == 0) {
      err = CRYPT_INVALID_PACKET;
      goto LBL_ERR;
   }

  return CRYPT_OK;
LBL_ERR:
   dsa_free(key);
   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
