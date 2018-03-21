/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb3_done.c
   OCB implementation, INTERNAL ONLY helper, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_OCB3_MODE

/**
   Finish OCB processing and compute the tag
   @param ocb     The OCB state
   @param tag     [out] The destination for the authentication tag
   @param taglen  [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful
*/
int ocb3_done(ocb3_state *ocb, unsigned char *tag, unsigned long *taglen)
{
   unsigned char tmp[MAXBLOCKSIZE];
   int err, x;

   LTC_ARGCHK(ocb    != NULL);
   LTC_ARGCHK(tag    != NULL);
   LTC_ARGCHK(taglen != NULL);
   if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   /* check taglen */
   if ((int)*taglen < ocb->tag_len) {
      *taglen = (unsigned long)ocb->tag_len;
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* finalize AAD processing */

   if (ocb->adata_buffer_bytes>0) {
     /* Offset_* = Offset_m xor L_* */
     ocb3_int_xor_blocks(ocb->aOffset_current, ocb->aOffset_current, ocb->L_star, ocb->block_len);

     /* CipherInput = (A_* || 1 || zeros(127-bitlen(A_*))) xor Offset_* */
     ocb3_int_xor_blocks(tmp, ocb->adata_buffer, ocb->aOffset_current, ocb->adata_buffer_bytes);
     for(x=ocb->adata_buffer_bytes; x<ocb->block_len; x++) {
       if (x == ocb->adata_buffer_bytes) {
         tmp[x] = 0x80 ^ ocb->aOffset_current[x];
       }
       else {
         tmp[x] = 0x00 ^ ocb->aOffset_current[x];
       }
     }

     /* Sum = Sum_m xor ENCIPHER(K, CipherInput) */
     if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
       goto LBL_ERR;
     }
     ocb3_int_xor_blocks(ocb->aSum_current, ocb->aSum_current, tmp, ocb->block_len);
   }

   /* finalize TAG computing */

   /* at this point ocb->aSum_current = HASH(K, A) */
   /* tag = tag ^ HASH(K, A) */
   ocb3_int_xor_blocks(tmp, ocb->tag_part, ocb->aSum_current, ocb->block_len);

   /* copy tag bytes */
   for(x = 0; x < ocb->tag_len; x++) tag[x] = tmp[x];
   *taglen = (unsigned long)ocb->tag_len;

   err = CRYPT_OK;

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(tmp, MAXBLOCKSIZE);
   zeromem(ocb, sizeof(*ocb));
#endif

   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
