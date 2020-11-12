#include "tommath_private.h"
#ifdef BN_MP_RAND_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

/* Dropbear sets this separately, avoid platform code */
mp_err(*s_mp_rand_source)(void *out, size_t size) = NULL;

void mp_rand_source(mp_err(*source)(void *out, size_t size))
{
   /* Dropbear, don't reset to platform if source==NULL */
   s_mp_rand_source = source;
}

mp_err mp_rand(mp_int *a, int digits)
{
   int i;
   mp_err err;

   mp_zero(a);

   if (digits <= 0) {
      return MP_OKAY;
   }

   if ((err = mp_grow(a, digits)) != MP_OKAY) {
      return err;
   }

   if ((err = s_mp_rand_source(a->dp, (size_t)digits * sizeof(mp_digit))) != MP_OKAY) {
      return err;
   }

   /* TODO: We ensure that the highest digit is nonzero. Should this be removed? */
   while ((a->dp[digits - 1] & MP_MASK) == 0u) {
      if ((err = s_mp_rand_source(a->dp + digits - 1, sizeof(mp_digit))) != MP_OKAY) {
         return err;
      }
   }

   a->used = digits;
   for (i = 0; i < digits; ++i) {
      a->dp[i] &= MP_MASK;
   }

   return MP_OKAY;
}
#endif
