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
  Reset a CCM state to as if you just called ccm_init().  This saves the initialization time.
  @param ccm   The CCM state to reset
  @return CRYPT_OK on success
*/
int ccm_reset(ccm_state *ccm)
{
   LTC_ARGCHK(ccm != NULL);
   zeromem(ccm->PAD, sizeof(ccm->PAD));
   zeromem(ccm->ctr, sizeof(ccm->ctr));
   zeromem(ccm->CTRPAD, sizeof(ccm->CTRPAD));
   ccm->CTRlen = 0;
   ccm->current_ptlen = 0;
   ccm->current_aadlen = 0;

   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
