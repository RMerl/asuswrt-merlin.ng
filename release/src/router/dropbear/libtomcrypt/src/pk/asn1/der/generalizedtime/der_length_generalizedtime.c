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
  @file der_length_utctime.c
  ASN.1 DER, get length of GeneralizedTime, Steffen Jaeckel
  Based on der_length_utctime.c
*/

#ifdef LTC_DER

/**
  Gets length of DER encoding of GeneralizedTime
  @param gtime        The GeneralizedTime structure to get the size of
  @param outlen [out] The length of the DER encoding
  @return CRYPT_OK if successful
*/
int der_length_generalizedtime(ltc_generalizedtime *gtime, unsigned long *outlen)
{
   LTC_ARGCHK(outlen  != NULL);
   LTC_ARGCHK(gtime != NULL);

   if (gtime->fs == 0) {
      /* we encode as YYYYMMDDhhmmssZ */
      *outlen = 2 + 14 + 1;
   } else {
      unsigned long len = 2 + 14 + 1;
      unsigned fs = gtime->fs;
      do {
         fs /= 10;
         len++;
      } while(fs != 0);
      if (gtime->off_hh == 0 && gtime->off_mm == 0) {
         /* we encode as YYYYMMDDhhmmss.fsZ */
         len += 1;
      }
      else {
         /* we encode as YYYYMMDDhhmmss.fs{+|-}hh'mm' */
         len += 5;
      }
      *outlen = len;
   }

   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
