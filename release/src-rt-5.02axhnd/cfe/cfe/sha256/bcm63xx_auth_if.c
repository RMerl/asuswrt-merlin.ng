/**********************************************************************
 *  
 *  bcm63xx_auth_if.c       
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
 ********************************************************************* 
 */
#include "bcm_sec.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_auth_if.h"

#if defined(_BCM963268_) || defined(_BCM963138_) || \
    defined(_BCM963148_) || defined(_BCM94908_)  || \
    defined(_BCM96858_) || defined(_BCM963158_)  || \
    defined(_BCM96846_) || defined(_BCM96856_)

#include "bcm_map.h"

#endif

#if (CFG_RAMAPP==1) && \
    ((defined(_BCM963138_) || defined(_BCM963148_) || \
     defined(_BCM94908_) || defined(_BCM96858_) || \
     defined(_BCM963158_)) || \
     defined(_BCM96846_) || defined(_BCM96856_))

#define board_setleds(...) {}
#else
extern void board_setleds(unsigned long);
#endif

extern void cfe_launch(unsigned long);

/* Halt the booting process when an error is detected
 */
void die(void) 
{
    // There's been a problem booting. Do anything necessary to make things
    // secure and spin until reset

    // Print out "FAIL" for failed authentication
    board_setleds(0x4641494c);

#if defined(_BCM963138_) || defined(_BCM963148_) || \
    defined(_BCM94908_) || defined(_BCM96858_) || \
    defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_)
    // disable SOTP and PKA blocks
    *(uint32_t *)(SOTP_BASE + SOTP_PERM) = 0;
#if (! defined(_BCM963158_)) && (!defined(_BCM96846_) && !defined(_BCM96856_))
    *(uint32_t *)(PKA_BASE + PKA_PERM) = 0;
#endif
#endif

#if (INC_BTRM_BUILD==1) && \
    (defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_))
    uint32_t binLen = 0;
    memcpy((void *)(BTRM_INT_MEM_SBI_LINK_ADDR_VIRT-sizeof(binLen)), (void *)&binLen, sizeof(binLen));
    uint32_t openJtag = 1;
    memcpy((void *)(BTRM_INT_MEM_SBI_LINK_ADDR_VIRT-sizeof(binLen)-sizeof(openJtag)), (void *)&openJtag, sizeof(openJtag));
#endif

    // Clean up cache, physical internal memory, enable JTAG if it is supposed to be enabled,
    // and then loop forever
    cfe_launch(0);
}


/** Authenticate a signed object
 * 
 * Given a signed object (that is, an object preceded by a 2048-bit signature),
 * retrieve the object and verify that it was indeed signed by the private key
 * with the public key modulus provided. Return the address of the start of the object 
 * itself.
 * 
 * If the signature cannot be verified, we will not return.
 * 
 * @param signedObject  address of the start of the signed object
 * @param signedSize    size of the signature and appended object in bytes
 * @param key           modulus of the public key that is paired with the private key used to sign the object 
 * @return              the address of the start of the object (no signature included)
 */
#if (! defined(_BCM96838_)) && (! defined(_BCM94908_)) && (! defined(_BCM96858_)) && \
    (! defined(_BCM96856_)) && (! defined(_BCM963158_)) && (! defined(_BCM96846_)) && (INC_BTRM_BOOT==0)
uint8_t const* authenticate(uint8_t const* signedObject, int signedSize, uint8_t const* key) {return NULL;}
int sec_verify_signature(uint8_t const* obj, uint32_t size, uint8_t const *sig, uint8_t const* key) {return -1;}
#else
int sec_verify_signature(uint8_t const* obj, uint32_t size, uint8_t const *sig, uint8_t const* key)
{
    // We know the size of the signature that preceded the object so locate the
    // object itself
    
    // Verify that the signature was made from this object
    return  Sec_verify((uint32_t*)sig, SEC_S_MODULUS,  // The signature we are checking
                                    (uint32_t*)key, SEC_S_MODULUS,  // The public key we are using to validate the authentication
                                    obj,  size);    // The object that was signed (without the concatenated signature)  
}

uint8_t const* authenticate(uint8_t const* signedObject, int signedSize, uint8_t const* key)
{
    uint8_t const* const object = signedObject + SEC_S_MODULUS;
    switch(sec_verify_signature(object, signedSize - SEC_S_MODULUS, signedObject, key)) {
          case SEC_S_SUCCESS:
               goto authd;
          case SEC_E_BADMESLEN:
             //The expected message is larger than we can sign using SHA-256
             board_setleds(0x45525241);
             break;
          case SEC_E_BADSIGLEN:
             // The signature block provided is the wrong size
             board_setleds(0x45525242);
             break;
          case SEC_E_BADDIGLEN:
             // The SHA-256 digest we were passed was the wrong size
             board_setleds(0x45525243);
             break;
          case SEC_E_BADPUBLEN:
             // The public key modulus provided is the wrong size
             board_setleds(0x45525244);
             break;
          case SEC_E_BADEMLEN:
             // The encoded message is too small to hold the required information
             board_setleds(0x45525245);
             break;
          case SEC_E_BADSEEDLEN:
             // The seed length is not the length of a SHA-256 digest
             board_setleds(0x45525246);
             break;
          case SEC_E_DIVFAIL:
             // An MP division operation has failed
             board_setleds(0x45525247);
             break;

          case SEC_E_BADMASKLEN:
             // We cannot generate a mask of the requested length
             board_setleds(0x45525248);
             break;
          case SEC_E_TSHORT:
             // The message representative is too short to hold the required information
             board_setleds(0x45525249);
             break;
          case SEC_E_TENDNOTBC:
             // The message representative did not end with the required byte 0xbc
             board_setleds(0x45525250);
             break;
          case SEC_E_BADLEADER:
             // The recovered message MB' is not null (but should have been)
             board_setleds(0x45525251);
             break;
          case SEC_E_MISMATCH:
             // The computed hash does not match the recovered hash
             board_setleds(0x45525252);
             break;
          default:
             // Unknown value
             board_setleds(0x45525253);
             break;
       }
       // ... and die
       die();
authd:  
      return object;
}
#endif /* #if (! defined(_BCM96838_)) && (INC_BTRM_BOOT==0) */
