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
   Create a DH shared secret.
   @param private_key     The private DH key in the pair
   @param public_key      The public DH key in the pair
   @param out             [out] The destination of the shared data
   @param outlen          [in/out] The max size and resulting size of the shared data.
   @return CRYPT_OK if successful
*/
int dh_shared_secret(dh_key *private_key, dh_key *public_key,
                     unsigned char *out, unsigned long *outlen)
{
   void *tmp;
   unsigned long x;
   int err;

   LTC_ARGCHK(private_key != NULL);
   LTC_ARGCHK(public_key  != NULL);
   LTC_ARGCHK(out         != NULL);
   LTC_ARGCHK(outlen      != NULL);

   /* types valid? */
   if (private_key->type != PK_PRIVATE) {
      return CRYPT_PK_NOT_PRIVATE;
   }

   /* same DH group? */
   if (mp_cmp(private_key->prime, public_key->prime) != LTC_MP_EQ) { return CRYPT_PK_TYPE_MISMATCH; }
   if (mp_cmp(private_key->base, public_key->base) != LTC_MP_EQ)   { return CRYPT_PK_TYPE_MISMATCH; }

   /* init big numbers */
   if ((err = mp_init(&tmp)) != CRYPT_OK) {
      return err;
   }

   /* check public key */
   if ((err = dh_check_pubkey(public_key)) != CRYPT_OK) {
      goto error;
   }

   /* compute tmp = y^x mod p */
   if ((err = mp_exptmod(public_key->y, private_key->x, private_key->prime, tmp)) != CRYPT_OK)  {
      goto error;
   }

   /* enough space for output? */
   x = (unsigned long)mp_unsigned_bin_size(tmp);
   if (*outlen < x) {
      *outlen = x;
      err = CRYPT_BUFFER_OVERFLOW;
      goto error;
   }
   if ((err = mp_to_unsigned_bin(tmp, out)) != CRYPT_OK) {
      goto error;
   }
   *outlen = x;
   err = CRYPT_OK;

error:
   mp_clear(tmp);
   return err;
}

#endif /* LTC_MDH */

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
