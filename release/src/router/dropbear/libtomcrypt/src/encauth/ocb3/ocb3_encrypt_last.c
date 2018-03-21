/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb3_encrypt_last.c
   OCB implementation, internal helper, by Karel Miko
*/
#include "tomcrypt.h"

#ifdef LTC_OCB3_MODE

/**
   Finish an OCB (encryption) stream
   @param ocb    The OCB state
   @param pt     The remaining plaintext
   @param ptlen  The length of the plaintext (octets)
   @param ct     [out] The output buffer
   @return CRYPT_OK if successful
*/
int ocb3_encrypt_last(ocb3_state *ocb, const unsigned char *pt, unsigned long ptlen, unsigned char *ct)
{
   unsigned char iOffset_star[MAXBLOCKSIZE];
   unsigned char iPad[MAXBLOCKSIZE];
   int err, x, full_blocks, full_blocks_len, last_block_len;

   LTC_ARGCHK(ocb != NULL);
   if (pt == NULL) LTC_ARGCHK(ptlen == 0);
   if (ptlen != 0) {
      LTC_ARGCHK(pt    != NULL);
      LTC_ARGCHK(ct    != NULL);
   }

   if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   full_blocks = ptlen/ocb->block_len;
   full_blocks_len = full_blocks * ocb->block_len;
   last_block_len = ptlen - full_blocks_len;

   /* process full blocks first */
   if (full_blocks>0) {
     if ((err = ocb3_encrypt(ocb, pt, full_blocks_len, ct)) != CRYPT_OK) {
       goto LBL_ERR;
     }
   }

   /* at this point: m = ocb->block_index (last block index), Offset_m = ocb->Offset_current */

   if (last_block_len>0) {
     /* Offset_* = Offset_m xor L_* */
     ocb3_int_xor_blocks(iOffset_star, ocb->Offset_current, ocb->L_star, ocb->block_len);

     /* Pad = ENCIPHER(K, Offset_*) */
     if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(iOffset_star, iPad, &ocb->key)) != CRYPT_OK) {
       goto LBL_ERR;
     }

     /* C_* = P_* xor Pad[1..bitlen(P_*)] */
     ocb3_int_xor_blocks(ct+full_blocks_len, pt+full_blocks_len, iPad, last_block_len);

     /* Checksum_* = Checksum_m xor (P_* || 1 || zeros(127-bitlen(P_*))) */
     ocb3_int_xor_blocks(ocb->checksum, ocb->checksum, pt+full_blocks_len, last_block_len);
     for(x=last_block_len; x<ocb->block_len; x++) {
       if (x == last_block_len)
         ocb->checksum[x] ^= 0x80;
       else
         ocb->checksum[x] ^= 0x00;
     }

     /* Tag = ENCIPHER(K, Checksum_* xor Offset_* xor L_$) xor HASH(K,A) */
     /* at this point we calculate only: Tag_part = ENCIPHER(K, Checksum_* xor Offset_* xor L_$) */
     for(x=0; x<ocb->block_len; x++) {
       ocb->tag_part[x] = (ocb->checksum[x] ^ iOffset_star[x]) ^ ocb->L_dollar[x];
     }
     if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(ocb->tag_part, ocb->tag_part, &ocb->key)) != CRYPT_OK) {
       goto LBL_ERR;
     }
   }
   else {
     /* Tag = ENCIPHER(K, Checksum_m xor Offset_m xor L_$) xor HASH(K,A) */
     /* at this point we calculate only: Tag_part = ENCIPHER(K, Checksum_m xor Offset_m xor L_$) */
     for(x=0; x<ocb->block_len; x++) {
       ocb->tag_part[x] = (ocb->checksum[x] ^ ocb->Offset_current[x]) ^ ocb->L_dollar[x];
     }
     if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(ocb->tag_part, ocb->tag_part, &ocb->key)) != CRYPT_OK) {
       goto LBL_ERR;
     }
   }

   err = CRYPT_OK;

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(iOffset_star, MAXBLOCKSIZE);
   zeromem(iPad, MAXBLOCKSIZE);
#endif

   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
