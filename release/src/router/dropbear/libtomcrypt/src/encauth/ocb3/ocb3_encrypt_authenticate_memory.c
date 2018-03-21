/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
  @file ocb3_encrypt_authenticate_memory.c
  OCB implementation, encrypt block of memory, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_OCB3_MODE

/**
   Encrypt and generate an authentication code for a buffer of memory
   @param cipher     The index of the cipher desired
   @param key        The secret key
   @param keylen     The length of the secret key (octets)
   @param nonce      The session nonce (length of the block ciphers block size)
   @param noncelen   The length of the nonce (octets)
   @param adata      The AAD - additional associated data
   @param adatalen   The length of AAD (octets)
   @param pt         The plaintext
   @param ptlen      The length of the plaintext (octets)
   @param ct         [out] The ciphertext
   @param tag        [out] The authentication tag
   @param taglen     [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful
*/
int ocb3_encrypt_authenticate_memory(int cipher,
    const unsigned char *key,    unsigned long keylen,
    const unsigned char *nonce,  unsigned long noncelen,
    const unsigned char *adata,  unsigned long adatalen,
    const unsigned char *pt,     unsigned long ptlen,
          unsigned char *ct,
          unsigned char *tag,    unsigned long *taglen)
{
   int err;
   ocb3_state *ocb;

   LTC_ARGCHK(taglen != NULL);

   /* allocate memory */
   ocb = XMALLOC(sizeof(ocb3_state));
   if (ocb == NULL) {
      return CRYPT_MEM;
   }

   if ((err = ocb3_init(ocb, cipher, key, keylen, nonce, noncelen, *taglen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   if (adata != NULL || adatalen != 0) {
      if ((err = ocb3_add_aad(ocb, adata, adatalen)) != CRYPT_OK) {
         goto LBL_ERR;
      }
   }

   if ((err = ocb3_encrypt_last(ocb, pt, ptlen, ct)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = ocb3_done(ocb, tag, taglen);

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(ocb, sizeof(ocb3_state));
#endif

   XFREE(ocb);
   return err;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
