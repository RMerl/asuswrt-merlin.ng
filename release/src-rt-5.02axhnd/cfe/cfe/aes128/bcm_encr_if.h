/*
<:label-BRCM:2012:NONE:standard

:>
*/

/**********************************************************************
 *  
 *  bcm_encr_if.h       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2011
 *  Broadcom Corporation. All rights reserved.
 *  
 *  This software is furnished under license and may be used and 
 *  copied only in accordance with the following terms and 
 *  conditions.  Subject to these conditions, you may download, 
 *  copy, install, use, modify and distribute modified or unmodified 
 *  copies of this software in source and/or binary form.  No title 
 *  or ownership is transferred hereby.
 *  
 *  1) Any source code used, modified or distributed must reproduce 
 *     and retain this copyright notice and list of conditions 
 *     as they appear in the source file.
 *  
 *  2) No right is granted to use any trade name, trademark, or 
 *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
 *     name may not be used to endorse or promote products derived 
 *     from this software without the prior written permission of 
 *     Broadcom Corporation.
 *  
 *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
 *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
 *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
 *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
 *     THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************* 
 */
#ifndef BCM_ENCR_IF_H_
#define BCM_ENCR_IF_H_

#include "lib_types.h"
#include "bcm63xx_aes.h"

typedef struct
{
    unsigned char     bek[CIPHER_KEY_LEN];
    unsigned char     iek[CIPHER_KEY_LEN];
    unsigned char     biv[CIPHER_IV_LEN];
    unsigned char     iiv[CIPHER_IV_LEN];
} Booter1EncrArgs;

extern void ek_iv_cleanup(Booter1EncrArgs *pEncrArgs);
extern void decryptWithEk(unsigned char *pDest, unsigned char *pSrc, unsigned char *pKey, uint32_t len, unsigned char *pIv);

#endif /* BCM_ENCR_IF_H_ */
