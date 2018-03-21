/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb3_decrypt_last.c
   OCB implementation, internal helper, by Karel Miko
*/
#include "tomcrypt.h"

#ifdef LTC_OCB3_MODE

/**
   Finish an OCB (decryption) stream
   @param ocb    The OCB state
   @param ct     The remaining ciphertext
   @param ctlen  The length of the ciphertext (octets)
   @param pt     [out] The output buffer
   @return CRYPT_OK if successful
*/
int ocb3_decrypt_last(ocb3_state *ocb, const unsigned char *ct, unsigned long ctlen, unsigned char *pt)
{
   unsigned char iOffset_star[MAXBLOCKSIZE];
   unsigned char iPad[MAXBLOCKSIZE];
   int err, x, full_blocks, full_blocks_len, last_block_len;

   LTC_ARGCHK(ocb != NULL);
   if (ct == NULL) LTC_ARGCHK(ctlen == 0);
   if (ctlen != 0) {
      LTC_ARGCHK(ct    != NULL);
      LTC_ARGCHK(pt    != NULL);
   }

   if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   full_blocks = ctlen/ocb->block_len;
   full_blocks_len = full_blocks * ocb->block_len;
   last_block_len = ctlen - full_blocks_len;

   /* process full blocks first */
   if (full_blocks>0) {
     if ((err = ocb3_decrypt(ocb, ct, full_blocks_len, pt)) != CRYPT_OK) {
       goto LBL_ERR;
     }
   }

   if (last_block_len>0) {
     /* Offset_* = Offset_m xor L_* */
     ocb3_int_xor_blocks(iOffset_star, ocb->Offset_current, ocb->L_star, ocb->block_len);

     /* Pad = ENCIPHER(K, Offset_*) */
     if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(iOffset_star, iPad, &ocb->key)) != CRYPT_OK) {
       goto LBL_ERR;
     }

     /* P_* = C_* xor Pad[1..bitlen(C_*)] */
     ocb3_int_xor_blocks(pt+full_blocks_len, (unsigned char *)ct+full_blocks_len, iPad, last_block_len);

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
