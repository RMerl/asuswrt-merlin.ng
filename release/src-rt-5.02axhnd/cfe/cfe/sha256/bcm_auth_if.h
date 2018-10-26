/*
* <:label-BRCM:2012:NONE:standard
* 
* :>
*/

/**********************************************************************
 *  
 *  bcm_auth_if.h       
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
#ifndef BCM_AUTH_IF_H_
#define BCM_AUTH_IF_H_

#include "bcm_sec.h"

extern int sec_verify_signature(uint8_t const* obj, uint32_t size, uint8_t const *sig, uint8_t const* key);
extern uint8_t const* authenticate(uint8_t const* signedObject, int signedSize, uint8_t const* key);
extern void die(void)  __attribute__ ((noinline));

typedef struct
{
    uint8_t     manu[SEC_S_MODULUS] __attribute__ ((aligned (4)));
    uint8_t     oper[SEC_S_MODULUS] __attribute__ ((aligned (4)));
} Booter1AuthArgs;

#endif /* BCM_AUTH_IF_H_ */
