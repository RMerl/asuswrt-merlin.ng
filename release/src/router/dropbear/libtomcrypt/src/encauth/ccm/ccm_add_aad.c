/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

#ifdef LTC_CCM_MODE

/**
  Add AAD to the CCM state
  @param ccm       The CCM state
  @param adata     The additional authentication data to add to the CCM state
  @param adatalen  The length of the AAD data.
  @return CRYPT_OK on success
 */
int ccm_add_aad(ccm_state *ccm,
                const unsigned char *adata,  unsigned long adatalen)
{
   unsigned long y;
   int            err;

   LTC_ARGCHK(ccm   != NULL);
   LTC_ARGCHK(adata != NULL);

   if (ccm->aadlen < ccm->current_aadlen + adatalen) {
      return CRYPT_INVALID_ARG;
   }
   ccm->current_aadlen += adatalen;

   /* now add the data */
   for (y = 0; y < adatalen; y++) {
      if (ccm->x == 16) {
         /* full block so let's encrypt it */
         if ((err = cipher_descriptor[ccm->cipher].ecb_encrypt(ccm->PAD, ccm->PAD, &ccm->K)) != CRYPT_OK) {
            return err;
         }
         ccm->x = 0;
      }
      ccm->PAD[ccm->x++] ^= adata[y];
   }

   /* remainder? */
   if (ccm->aadlen == ccm->current_aadlen) {
      if (ccm->x != 0) {
         if ((err = cipher_descriptor[ccm->cipher].ecb_encrypt(ccm->PAD, ccm->PAD, &ccm->K)) != CRYPT_OK) {
            return err;
         }
      }
      ccm->x = 0;
   }

   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
