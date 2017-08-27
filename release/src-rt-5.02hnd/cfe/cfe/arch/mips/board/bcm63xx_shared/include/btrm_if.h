/**********************************************************************
 *  
 *
 *     <:copyright-BRCM:2012:proprietary:standard
 *     
 *        Copyright (c) 2012 Broadcom 
 *        All Rights Reserved
 *     
 *      This program is the proprietary software of Broadcom and/or its
 *      licensors, and may only be used, duplicated, modified or distributed pursuant
 *      to the terms and conditions of a separate, written license agreement executed
 *      between you and Broadcom (an "Authorized License").  Except as set forth in
 *      an Authorized License, Broadcom grants no license (express or implied), right
 *      to use, or waiver of any kind with respect to the Software, and Broadcom
 *      expressly reserves all rights in and to the Software and all intellectual
 *      property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *      NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *      BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *     
 *      Except as expressly set forth in the Authorized License,
 *     
 *      1. This program, including its structure, sequence and organization,
 *         constitutes the valuable trade secrets of Broadcom, and you shall use
 *         all reasonable efforts to protect the confidentiality thereof, and to
 *         use this information only in connection with your use of Broadcom
 *         integrated circuit products.
 *     
 *      2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *         AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *         WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *         RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *         ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *         FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *         COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *         TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *         PERFORMANCE OF THE SOFTWARE.
 *     
 *      3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *         ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *         INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *         WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *         IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *         OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *         SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *         SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *         LIMITED REMEDY.
 *     :>
 *
 ********************************************************************* 
 */

/**********************************************************************
 *  
 *  btrm_if.h       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2011
 *  Broadcom Corporation. All rights reserved.
 *
 ********************************************************************* 
 */
#ifndef BTRM_IF_H_
#define BTRM_IF_H_

#include "bcm_auth_if.h"
#include "bcm_encr_if.h"

#define BTRM_SBI_UNAUTH_MGC_NUM_1       112233        
#define BTRM_SBI_UNAUTH_MGC_NUM_2       445566

#define LMEM_UNCACHED_OFFSET   		0x20000000

extern int otp_is_btrm_enabled(void);
extern int otp_is_boot_secure(void);

typedef struct
{
   Booter1AuthArgs	authArgs __attribute__ ((aligned (4)));
   Booter1EncrArgs	encrArgs; 
   uint8_t		otpOperCvInUse;
   uint8_t		otpDieFallThru;
} Booter1Args;

#endif /* BTRM_IF_H_ */
