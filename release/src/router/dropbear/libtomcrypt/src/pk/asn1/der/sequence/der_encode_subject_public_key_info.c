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
  @file der_encode_subject_public_key_info.c
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
  Encode a subject public key info
   @param out           The output buffer
   @param outlen        [in/out] Length of buffer and resulting length of output
   @param algorithm             One out of the enum #public_key_algorithms
   @param public_key            The buffer for the public key
   @param public_key_len        The length of the public key buffer
   @param parameters_type       The parameters' type out of the enum ltc_asn1_type
   @param parameters            The parameters to include
   @param parameters_len        The number of parameters to include
   @return CRYPT_OK on success
*/
int der_encode_subject_public_key_info(unsigned char *out, unsigned long *outlen,
        unsigned int algorithm, void* public_key, unsigned long public_key_len,
        unsigned long parameters_type, void* parameters, unsigned long parameters_len)
{
   int           err;
   ltc_asn1_list alg_id[2];
   oid_st oid;

   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   err = pk_get_oid(algorithm, &oid);
   if (err != CRYPT_OK) {
        return err;
   }

   LTC_SET_ASN1(alg_id, 0, LTC_ASN1_OBJECT_IDENTIFIER, oid.OID,    oid.OIDlen);
   LTC_SET_ASN1(alg_id, 1, (ltc_asn1_type)parameters_type,            parameters, parameters_len);

   return der_encode_sequence_multi(out, outlen,
        LTC_ASN1_SEQUENCE, (unsigned long)sizeof(alg_id)/sizeof(alg_id[0]), alg_id,
        LTC_ASN1_RAW_BIT_STRING, public_key_len*8U, public_key,
        LTC_ASN1_EOL,     0UL, NULL);

}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */

