/*  *********************************************************************
    *  
    *  bcm63xx_encr_if.c       
    *
    *  Author:  Brian Nay (brian.nay@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2011
    *  Broadcom Corporation. All rights reserved.
    *  
    * <:label-BRCM:2012:proprietary:standard
    * 
    *  This program is the proprietary software of Broadcom and/or its
    *  licensors, and may only be used, duplicated, modified or distributed pursuant
    *  to the terms and conditions of a separate, written license agreement executed
    *  between you and Broadcom (an "Authorized License").  Except as set forth in
    *  an Authorized License, Broadcom grants no license (express or implied), right
    *  to use, or waiver of any kind with respect to the Software, and Broadcom
    *  expressly reserves all rights in and to the Software and all intellectual
    *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
    *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
    *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    * 
    *  Except as expressly set forth in the Authorized License,
    * 
    *  1. This program, including its structure, sequence and organization,
    *     constitutes the valuable trade secrets of Broadcom, and you shall use
    *     all reasonable efforts to protect the confidentiality thereof, and to
    *     use this information only in connection with your use of Broadcom
    *     integrated circuit products.
    * 
    *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    *     PERFORMANCE OF THE SOFTWARE.
    * 
    *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    *     LIMITED REMEDY.
    * :>
    *
    ********************************************************************* */

#include "bcm_encr_if.h"
#include "bcm_auth_if.h"
#include "lib_string.h"
#include "bcm_map.h"

#define LMEM_UNCACHED_OFFSET            0x20000000

/*  *********************************************************************
    *  ek_iv_cleanup(void)
    *  
    *  Return value:
    *  	   none
    *
    ********************************************************************* */
#if (CFE_RAMAPP)
void ek_iv_cleanup(Booter1EncrArgs *pEncrArgs) {}
#else
void ek_iv_cleanup(Booter1EncrArgs *pEncrArgs)
{
   void *pBek = &pEncrArgs->bek[0];
   void *pIek = &pEncrArgs->iek[0];
   void *pBiv = &pEncrArgs->biv[0];
   void *pIiv = &pEncrArgs->iiv[0];
   Booter1EncrArgs *pIntMemCredEncrArgs = (Booter1EncrArgs *)(BTRM_INT_MEM_CREDENTIALS_ADDR + sizeof(Booter1AuthArgs));

   memset(pBek, 0, CIPHER_KEY_LEN);
   memset(pIek, 0, CIPHER_KEY_LEN);
   memset(pBiv, 0, CIPHER_IV_LEN);
   memset(pIiv, 0, CIPHER_IV_LEN);

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64) && !defined(_BCM96848_)
   pBek += LMEM_UNCACHED_OFFSET;
   pIek += LMEM_UNCACHED_OFFSET;
   pBiv += LMEM_UNCACHED_OFFSET;
   pIiv += LMEM_UNCACHED_OFFSET;

   memset(pBek, 0, CIPHER_KEY_LEN);
   memset(pIek, 0, CIPHER_KEY_LEN);
   memset(pBiv, 0, CIPHER_IV_LEN);
   memset(pIiv, 0, CIPHER_IV_LEN);
#endif

   /* Clean up the INT_MEM_CREDENTIALS location as well */
   pBek = &pIntMemCredEncrArgs->bek[0];
   pIek = &pIntMemCredEncrArgs->iek[0];
   pBiv = &pIntMemCredEncrArgs->biv[0];
   pIiv = &pIntMemCredEncrArgs->iiv[0];

   memset(pBek, 0, CIPHER_KEY_LEN);
   memset(pIek, 0, CIPHER_KEY_LEN);
   memset(pBiv, 0, CIPHER_IV_LEN);
   memset(pIiv, 0, CIPHER_IV_LEN);

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64) && !defined(_BCM96848_)
   pBek += LMEM_UNCACHED_OFFSET;
   pIek += LMEM_UNCACHED_OFFSET;
   pBiv += LMEM_UNCACHED_OFFSET;
   pIiv += LMEM_UNCACHED_OFFSET;

   memset(pBek, 0, CIPHER_KEY_LEN);
   memset(pIek, 0, CIPHER_KEY_LEN);
   memset(pBiv, 0, CIPHER_IV_LEN);
   memset(pIiv, 0, CIPHER_IV_LEN);
#endif

}
#endif

/*  *********************************************************************
    *  decryptWithEk(void)
    *  
    *  Input parameters: 
    *  	   pDest - Pointer to where the decrypt content should be placed
    *  	   pSrc  - Pointer to where the encrypted content can be found
    *  	   pKey  - Pointer to where the 16 byte key can be found
    *  	           (CIPHER_KEY_LEN in length)
    *  	   len   - Length of encrypted content
    *  	   pIv   - Pointer to where the initialization vector is stored
    *  	           (CIPHER_BLK_LEN in length)
    *  	   
    *  Return value:
    *  	   none
    *
    ********************************************************************* */
#if (defined(_BCM96838_) && (INC_NAND_FLASH_DRIVER==0))
void decryptWithEk(unsigned char *pDest, unsigned char *pSrc, unsigned char *pKey, uint32_t len, unsigned char *pIv) {}
#else
void decryptWithEk(unsigned char *pDest, unsigned char *pSrc, unsigned char *pKey, uint32_t len, unsigned char *pIv)
{
   int			offset, i;
   uint32_t 	        rk[RK_LEN];
   unsigned char        temp[CIPHER_BLK_LEN];

   /* Populate rk structure */
   aesInitDecrypt(rk, pKey);

   for( offset = 0; offset < len; offset += CIPHER_BLK_LEN )
   {
      memcpy((void *)temp, (void *)(pSrc+offset), CIPHER_BLK_LEN );
      aesCompleteDecrypt(rk, temp, pDest);

      for( i = 0; i < CIPHER_BLK_LEN; i++ )
         pDest[i] = (unsigned char)( pDest[i] ^ pIv[i] );

      memcpy( pIv, pSrc+offset, CIPHER_BLK_LEN );
      pDest += CIPHER_BLK_LEN;
   }

   memset((void *)rk, 0 , RK_LEN);
}
#endif
