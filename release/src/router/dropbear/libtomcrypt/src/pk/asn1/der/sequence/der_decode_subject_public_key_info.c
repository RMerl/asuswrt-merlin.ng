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
  @file der_decode_subject_public_key_info.c
  ASN.1 DER, encode a Subject Public Key structure --nmav
*/

#ifdef LTC_DER

/* AlgorithmIdentifier := SEQUENCE {
 *    algorithm OBJECT IDENTIFIER,
 *    parameters ANY DEFINED BY algorithm
 * }
 *
 * SubjectPublicKeyInfo := SEQUENCE {
 *    algorithm AlgorithmIdentifier,
 *    subjectPublicKey BIT STRING
 * }
 */
/**
  Decode a subject public key info
   @param in      The input buffer
   @param inlen   The length of the input buffer
   @param algorithm             One out of the enum #public_key_algorithms
   @param public_key            The buffer for the public key
   @param public_key_len        [in/out] The length of the public key buffer and the written length
   @param parameters_type       The parameters' type out of the enum ltc_asn1_type
   @param parameters            The parameters to include
   @param parameters_len        The number of parameters to include
   @return CRYPT_OK on success
*/
int der_decode_subject_public_key_info(const unsigned char *in, unsigned long inlen,
        unsigned int algorithm, void* public_key, unsigned long* public_key_len,
        unsigned long parameters_type, ltc_asn1_list* parameters, unsigned long parameters_len)
{
   int err;
   unsigned long len;
   oid_st oid;
   unsigned char *tmpbuf;
   unsigned long  tmpoid[16];
   ltc_asn1_list alg_id[2];
   ltc_asn1_list subject_pubkey[2];

   LTC_ARGCHK(in    != NULL);
   LTC_ARGCHK(inlen != 0);
   LTC_ARGCHK(public_key_len != NULL);

   err = pk_get_oid(algorithm, &oid);
   if (err != CRYPT_OK) {
        return err;
   }

   /* see if the OpenSSL DER format RSA public key will work */
   tmpbuf = XCALLOC(1, inlen);
   if (tmpbuf == NULL) {
       err = CRYPT_MEM;
       goto LBL_ERR;
   }

   /* this includes the internal hash ID and optional params (NULL in this case) */
   LTC_SET_ASN1(alg_id, 0, LTC_ASN1_OBJECT_IDENTIFIER, tmpoid, sizeof(tmpoid)/sizeof(tmpoid[0]));
   LTC_SET_ASN1(alg_id, 1, (ltc_asn1_type)parameters_type, parameters, parameters_len);

   /* the actual format of the SSL DER key is odd, it stores a RSAPublicKey
    * in a **BIT** string ... so we have to extract it then proceed to convert bit to octet
    */
   LTC_SET_ASN1(subject_pubkey, 0, LTC_ASN1_SEQUENCE, alg_id, 2);
   LTC_SET_ASN1(subject_pubkey, 1, LTC_ASN1_RAW_BIT_STRING, tmpbuf, inlen*8U);

   err=der_decode_sequence(in, inlen, subject_pubkey, 2UL);
   if (err != CRYPT_OK) {
           goto LBL_ERR;
   }

   if ((alg_id[0].size != oid.OIDlen) ||
        XMEMCMP(oid.OID, alg_id[0].data, oid.OIDlen * sizeof(oid.OID[0])) != 0) {
        /* OID mismatch */
        err = CRYPT_PK_INVALID_TYPE;
        goto LBL_ERR;
   }

   len = subject_pubkey[1].size/8;
   if (*public_key_len > len) {
       XMEMCPY(public_key, subject_pubkey[1].data, len);
       *public_key_len = len;
    } else {
        *public_key_len = len;
        err = CRYPT_BUFFER_OVERFLOW;
        goto LBL_ERR;
    }

    err = CRYPT_OK;

LBL_ERR:

    XFREE(tmpbuf);

    return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
