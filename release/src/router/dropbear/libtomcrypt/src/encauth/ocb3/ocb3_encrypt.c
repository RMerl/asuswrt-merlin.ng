/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb3_encrypt.c
   OCB implementation, encrypt data, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_OCB3_MODE

/**
   Encrypt blocks of data with OCB
   @param ocb     The OCB state
   @param pt      The plaintext (length multiple of the block size of the block cipher)
   @param ptlen   The length of the input (octets)
   @param ct      [out] The ciphertext (same size as the pt)
   @return CRYPT_OK if successful
*/
int ocb3_encrypt(ocb3_state *ocb, const unsigned char *pt, unsigned long ptlen, unsigned char *ct)
{
   unsigned char tmp[MAXBLOCKSIZE];
   int err, i, full_blocks;
   unsigned char *pt_b, *ct_b;

   LTC_ARGCHK(ocb != NULL);
   if (ptlen == 0) return CRYPT_OK; /* no data, nothing to do */
   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);

   if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
      return err;
   }
   if (ocb->block_len != cipher_descriptor[ocb->cipher].block_length) {
      return CRYPT_INVALID_ARG;
   }

   if (ptlen % ocb->block_len) { /* ptlen has to bu multiple of block_len */
      return CRYPT_INVALID_ARG;
   }

   full_blocks = ptlen/ocb->block_len;
   for(i=0; i<full_blocks; i++) {
     pt_b = (unsigned char *)pt+i*ocb->block_len;
     ct_b = (unsigned char *)ct+i*ocb->block_len;

     /* ocb->Offset_current[] = ocb->Offset_current[] ^ Offset_{ntz(block_index)} */
     ocb3_int_xor_blocks(ocb->Offset_current, ocb->Offset_current, ocb->L_[ocb3_int_ntz(ocb->block_index)], ocb->block_len);

     /* tmp[] = pt[] XOR ocb->Offset_current[] */
     ocb3_int_xor_blocks(tmp, pt_b, ocb->Offset_current, ocb->block_len);

     /* encrypt */
     if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
        goto LBL_ERR;
     }

     /* ct[] = tmp[] XOR ocb->Offset_current[] */
     ocb3_int_xor_blocks(ct_b, tmp, ocb->Offset_current, ocb->block_len);

     /* ocb->checksum[] = ocb->checksum[] XOR pt[] */
     ocb3_int_xor_blocks(ocb->checksum, ocb->checksum, pt_b, ocb->block_len);

     ocb->block_index++;
   }

   err = CRYPT_OK;

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(tmp, sizeof(tmp));
#endif
   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
