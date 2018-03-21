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
  @file der_sequence_shrink.c
  Free memory allocated for CONSTRUCTED, SET or SEQUENCE elements by der_decode_sequence_flexi(), Steffen Jaeckel
*/

#ifdef LTC_DER

/**
  Free memory allocated for CONSTRUCTED,
  SET or SEQUENCE elements by der_decode_sequence_flexi()
  @param in     The list to shrink
*/
void der_sequence_shrink(ltc_asn1_list *in)
{
   if (!in) return;

   /* now walk the list and free stuff */
   while (in != NULL) {
      /* is there a child? */
      if (in->child) {
         der_sequence_shrink(in->child);
      }

      switch (in->type) {
         case LTC_ASN1_CONSTRUCTED:
         case LTC_ASN1_SET:
         case LTC_ASN1_SEQUENCE : if (in->data != NULL) { XFREE(in->data); in->data = NULL; } break;
         default: break;
      }

      /* move to next and free current */
      in = in->next;
   }
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
