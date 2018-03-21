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
   @file dsa_verify_hash.c
   DSA implementation, verify a signature, Tom St Denis
*/


#ifdef LTC_MDSA

/**
  Verify a DSA signature
  @param r        DSA "r" parameter
  @param s        DSA "s" parameter
  @param hash     The hash that was signed
  @param hashlen  The length of the hash that was signed
  @param stat     [out] The result of the signature verification, 1==valid, 0==invalid
  @param key      The corresponding public DSA key
  @return CRYPT_OK if successful (even if the signature is invalid)
*/
int dsa_verify_hash_raw(         void   *r,          void   *s,
                    const unsigned char *hash, unsigned long hashlen,
                                    int *stat,      dsa_key *key)
{
   void          *w, *v, *u1, *u2;
   int           err;

   LTC_ARGCHK(r    != NULL);
   LTC_ARGCHK(s    != NULL);
   LTC_ARGCHK(stat != NULL);
   LTC_ARGCHK(key  != NULL);

   /* default to invalid signature */
   *stat = 0;

   /* init our variables */
   if ((err = mp_init_multi(&w, &v, &u1, &u2, NULL)) != CRYPT_OK) {
      return err;
   }

   /* neither r or s can be null or >q*/
   if (mp_cmp_d(r, 0) != LTC_MP_GT || mp_cmp_d(s, 0) != LTC_MP_GT || mp_cmp(r, key->q) != LTC_MP_LT || mp_cmp(s, key->q) != LTC_MP_LT) {
      err = CRYPT_INVALID_PACKET;
      goto error;
   }

   /* FIPS 186-4 4.7: use leftmost min(bitlen(q), bitlen(hash)) bits of 'hash' */
   hashlen = MIN(hashlen, (unsigned long)(key->qord));

   /* w = 1/s mod q */
   if ((err = mp_invmod(s, key->q, w)) != CRYPT_OK)                                       { goto error; }

   /* u1 = m * w mod q */
   if ((err = mp_read_unsigned_bin(u1, (unsigned char *)hash, hashlen)) != CRYPT_OK)      { goto error; }
   if ((err = mp_mulmod(u1, w, key->q, u1)) != CRYPT_OK)                                  { goto error; }

   /* u2 = r*w mod q */
   if ((err = mp_mulmod(r, w, key->q, u2)) != CRYPT_OK)                                   { goto error; }

   /* v = g^u1 * y^u2 mod p mod q */
   if ((err = mp_exptmod(key->g, u1, key->p, u1)) != CRYPT_OK)                            { goto error; }
   if ((err = mp_exptmod(key->y, u2, key->p, u2)) != CRYPT_OK)                            { goto error; }
   if ((err = mp_mulmod(u1, u2, key->p, v)) != CRYPT_OK)                                  { goto error; }
   if ((err = mp_mod(v, key->q, v)) != CRYPT_OK)                                          { goto error; }

   /* if r = v then we're set */
   if (mp_cmp(r, v) == LTC_MP_EQ) {
      *stat = 1;
   }

   err = CRYPT_OK;
error:
   mp_clear_multi(w, v, u1, u2, NULL);
   return err;
}

/**
  Verify a DSA signature
  @param sig      The signature
  @param siglen   The length of the signature (octets)
  @param hash     The hash that was signed
  @param hashlen  The length of the hash that was signed
  @param stat     [out] The result of the signature verification, 1==valid, 0==invalid
  @param key      The corresponding public DSA key
  @return CRYPT_OK if successful (even if the signature is invalid)
*/
int dsa_verify_hash(const unsigned char *sig, unsigned long siglen,
                    const unsigned char *hash, unsigned long hashlen,
                    int *stat, dsa_key *key)
{
   int    err;
   void   *r, *s;
   ltc_asn1_list sig_seq[2];
   unsigned long reallen = 0;

   LTC_ARGCHK(stat != NULL);
   *stat = 0; /* must be set before the first return */

   if ((err = mp_init_multi(&r, &s, NULL)) != CRYPT_OK) {
      return err;
   }

   LTC_SET_ASN1(sig_seq, 0, LTC_ASN1_INTEGER, r, 1UL);
   LTC_SET_ASN1(sig_seq, 1, LTC_ASN1_INTEGER, s, 1UL);

   err = der_decode_sequence(sig, siglen, sig_seq, 2);
   if (err != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = der_length_sequence(sig_seq, 2, &reallen);
   if (err != CRYPT_OK || reallen != siglen) {
      goto LBL_ERR;
   }

   /* do the op */
   err = dsa_verify_hash_raw(r, s, hash, hashlen, stat, key);

LBL_ERR:
   mp_clear_multi(r, s, NULL);
   return err;
}

#endif


/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
