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
  @file rsa_import.c
  Import an RSA key from a X.509 certificate, Steffen Jaeckel
*/

#ifdef LTC_MRSA

/**
  Import an RSA key from a X.509 certificate
  @param in      The packet to import from
  @param inlen   It's length (octets)
  @param key     [out] Destination for newly imported key
  @return CRYPT_OK if successful, upon error allocated memory is freed
*/
int rsa_import_x509(const unsigned char *in, unsigned long inlen, rsa_key *key)
{
   int           err;
   unsigned char *tmpbuf;
   unsigned long tmpbuf_len, tmp_inlen;
   ltc_asn1_list *decoded_list = NULL, *l;

   LTC_ARGCHK(in          != NULL);
   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* init key */
   if ((err = mp_init_multi(&key->e, &key->d, &key->N, &key->dQ,
                            &key->dP, &key->qP, &key->p, &key->q, NULL)) != CRYPT_OK) {
      return err;
   }

   tmpbuf_len = inlen;
   tmpbuf = XCALLOC(1, tmpbuf_len);
   if (tmpbuf == NULL) {
       err = CRYPT_MEM;
       goto LBL_ERR;
   }

   tmp_inlen = inlen;
   if ((err = der_decode_sequence_flexi(in, &tmp_inlen, &decoded_list)) == CRYPT_OK) {
      l = decoded_list;
      /* Move 2 levels up in the tree
         SEQUENCE
             SEQUENCE
                 ...
       */
      if (l->type == LTC_ASN1_SEQUENCE && l->child) {
         l = l->child;
         if (l->type == LTC_ASN1_SEQUENCE && l->child) {
            l = l->child;

            err = CRYPT_ERROR;

            /* Move forward in the tree until we find this combination
                 ...
                 SEQUENCE
                     SEQUENCE
                         OBJECT IDENTIFIER 1.2.840.113549.1.1.1
                         NULL
                     BIT STRING
             */
            do {
               /* The additional check for l->data is there to make sure
                * we won't try to decode a list that has been 'shrunk'
                */
               if (l->type == LTC_ASN1_SEQUENCE && l->data && l->child &&
                     l->child->type == LTC_ASN1_SEQUENCE && l->child->child &&
                     l->child->child->type == LTC_ASN1_OBJECT_IDENTIFIER && l->child->next &&
                     l->child->next->type == LTC_ASN1_BIT_STRING) {
                  err = der_decode_subject_public_key_info(l->data, l->size,
                       PKA_RSA, tmpbuf, &tmpbuf_len,
                       LTC_ASN1_NULL, NULL, 0);
                  if (err == CRYPT_OK) {
                     /* now it should be SEQUENCE { INTEGER, INTEGER } */
                     if ((err = der_decode_sequence_multi(tmpbuf, tmpbuf_len,
                                                          LTC_ASN1_INTEGER, 1UL, key->N,
                                                          LTC_ASN1_INTEGER, 1UL, key->e,
                                                          LTC_ASN1_EOL,     0UL, NULL)) != CRYPT_OK) {
                        goto LBL_ERR;
                     }
                     key->type = PK_PUBLIC;
                     err = CRYPT_OK;
                     goto LBL_FREE;
                  }
               }
               l = l->next;
            } while(l);
         }
      }
   }


LBL_ERR:
   rsa_free(key);

LBL_FREE:
   if (decoded_list) der_free_sequence_flexi(decoded_list);
   if (tmpbuf != NULL) XFREE(tmpbuf);

   return err;
}

#endif /* LTC_MRSA */


/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
