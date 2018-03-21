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
   @file dsa_verify_key.c
   DSA implementation, verify a key, Tom St Denis
*/

#ifdef LTC_MDSA

/**
   Validate a DSA key

     Yeah, this function should've been called dsa_validate_key()
     in the first place and for compat-reasons we keep it
     as it was (for now).

   @param key   The key to validate
   @param stat  [out]  Result of test, 1==valid, 0==invalid
   @return CRYPT_OK if successful
*/
int dsa_verify_key(dsa_key *key, int *stat)
{
   int err;

   err = dsa_int_validate_primes(key, stat);
   if (err != CRYPT_OK || *stat == 0) return err;

   err = dsa_int_validate_pqg(key, stat);
   if (err != CRYPT_OK || *stat == 0) return err;

   return dsa_int_validate_xy(key, stat);
}

/**
   Non-complex part (no primality testing) of the validation
   of DSA params (p, q, g)

   @param key   The key to validate
   @param stat  [out]  Result of test, 1==valid, 0==invalid
   @return CRYPT_OK if successful
*/
int dsa_int_validate_pqg(dsa_key *key, int *stat)
{
   void *tmp1, *tmp2;
   int  err;

   LTC_ARGCHK(key  != NULL);
   LTC_ARGCHK(stat != NULL);
   *stat = 0;

   /* check q-order */
   if ( key->qord >= LTC_MDSA_MAX_GROUP || key->qord <= 15 ||
        (unsigned long)key->qord >= mp_unsigned_bin_size(key->p) ||
        (mp_unsigned_bin_size(key->p) - key->qord) >= LTC_MDSA_DELTA ) {
      return CRYPT_OK;
   }

   /* FIPS 186-4 chapter 4.1: 1 < g < p */
   if (mp_cmp_d(key->g, 1) != LTC_MP_GT || mp_cmp(key->g, key->p) != LTC_MP_LT) {
      return CRYPT_OK;
   }

   if ((err = mp_init_multi(&tmp1, &tmp2, NULL)) != CRYPT_OK)        { return err; }

   /* FIPS 186-4 chapter 4.1: q is a divisor of (p - 1) */
   if ((err = mp_sub_d(key->p, 1, tmp1)) != CRYPT_OK)                { goto error; }
   if ((err = mp_div(tmp1, key->q, tmp1, tmp2)) != CRYPT_OK)         { goto error; }
   if (mp_iszero(tmp2) != LTC_MP_YES) {
      err = CRYPT_OK;
      goto error;
   }

   /* FIPS 186-4 chapter 4.1: g is a generator of a subgroup of order q in
    * the multiplicative group of GF(p) - so we make sure that g^q mod p = 1
    */
   if ((err = mp_exptmod(key->g, key->q, key->p, tmp1)) != CRYPT_OK) { goto error; }
   if (mp_cmp_d(tmp1, 1) != LTC_MP_EQ) {
      err = CRYPT_OK;
      goto error;
   }

   err   = CRYPT_OK;
   *stat = 1;
error:
   mp_clear_multi(tmp2, tmp1, NULL);
   return err;
}

/**
   Primality testing of DSA params p and q

   @param key   The key to validate
   @param stat  [out]  Result of test, 1==valid, 0==invalid
   @return CRYPT_OK if successful
*/
int dsa_int_validate_primes(dsa_key *key, int *stat)
{
   int err, res;

   *stat = 0;
   LTC_ARGCHK(key  != NULL);
   LTC_ARGCHK(stat != NULL);

   /* key->q prime? */
   if ((err = mp_prime_is_prime(key->q, LTC_MILLER_RABIN_REPS, &res)) != CRYPT_OK) {
      return err;
   }
   if (res == LTC_MP_NO) {
      return CRYPT_OK;
   }

   /* key->p prime? */
   if ((err = mp_prime_is_prime(key->p, LTC_MILLER_RABIN_REPS, &res)) != CRYPT_OK) {
      return err;
   }
   if (res == LTC_MP_NO) {
      return CRYPT_OK;
   }

   *stat = 1;
   return CRYPT_OK;
}

/**
   Validation of a DSA key (x and y values)

   @param key   The key to validate
   @param stat  [out]  Result of test, 1==valid, 0==invalid
   @return CRYPT_OK if successful
*/
int dsa_int_validate_xy(dsa_key *key, int *stat)
{
   void *tmp;
   int  err;

   *stat = 0;
   LTC_ARGCHK(key  != NULL);
   LTC_ARGCHK(stat != NULL);

   /* 1 < y < p-1 */
   if ((err = mp_init(&tmp)) != CRYPT_OK) {
      return err;
   }
   if ((err = mp_sub_d(key->p, 1, tmp)) != CRYPT_OK) {
      goto error;
   }
   if (mp_cmp_d(key->y, 1) != LTC_MP_GT || mp_cmp(key->y, tmp) != LTC_MP_LT) {
      err = CRYPT_OK;
      goto error;
   }

   if (key->type == PK_PRIVATE) {
      /* FIPS 186-4 chapter 4.1: 0 < x < q */
      if (mp_cmp_d(key->x, 0) != LTC_MP_GT || mp_cmp(key->x, key->q) != LTC_MP_LT) {
         err = CRYPT_OK;
         goto error;
      }
      /* FIPS 186-4 chapter 4.1: y = g^x mod p */
      if ((err = mp_exptmod(key->g, key->x, key->p, tmp)) != CRYPT_OK) {
         goto error;
      }
      if (mp_cmp(tmp, key->y) != LTC_MP_EQ) {
         err = CRYPT_OK;
         goto error;
      }
   }
   else {
      /* with just a public key we cannot test y = g^x mod p therefore we
       * only test that y^q mod p = 1, which makes sure y is in g^x mod p
       */
      if ((err = mp_exptmod(key->y, key->q, key->p, tmp)) != CRYPT_OK) {
         goto error;
      }
      if (mp_cmp_d(tmp, 1) != LTC_MP_EQ) {
         err = CRYPT_OK;
         goto error;
      }
   }

   err   = CRYPT_OK;
   *stat = 1;
error:
   mp_clear(tmp);
   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
