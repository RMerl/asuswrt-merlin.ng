/*
<:label-BRCM:2012:NONE:standard

:>
*/

/**********************************************************************
 *  
 *  bcm63xx_sotp.h       
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
#ifndef H__BCM63XX_SOTP
#define H__BCM63XX_SOTP

#include "lib_types.h"

#define    SOTP_MAX_CNTR     		1000

#define    SOTP_BYTES_PER_ROW  		4

#define    SOTP_FIRST_FUSELOCK_ROW 	8
#define    SOTP_NUM_REG_IN_FUSELOCK_ROW	21

#define    SOTP_FIRST_KEYSLOT_ROW 	28
#define    SOTP_ROWS_IN_KEYSLOT 	12
#define    SOTP_ROWS_IN_REGION          4

#define    SOTP_FIRST_KEYSLOT_REGION    7
#define    SOTP_REGIONS_IN_KEYSLOT      3
#define    SOTP_REGIONS_MASK_IN_KEYSLOT 7

#define    SOTP_MIN_KEYSLOT 		7
#define    SOTP_MAX_KEYSLOT 		13

#define    SOTP_MAX_KEYLEN 		8

typedef enum
{
   SOTP_S_MODE_MFG,        // The SOTP block has not been personalized yet and is in manufacuring mode
   SOTP_S_MODE_FIELD,      // The SOTP block has been personalized and is ready for deployment
   SOTP_E_MODE_TIMEOUT,	   // The function experienced unexpected FSM timeout from the SOTP block 
   SOTP_E_MODE_ERROR       // The function experienced unexpected register return value from the SOTP block 
} SotpMode;

typedef enum
{
   SOTP_S_ROW_SUCCESS,	   // The function has completed successfully
   SOTP_S_ROW_ECC_COR, 	   // The function corrected 1 bad bit within the row (row is still usable)
   SOTP_E_ROW_ECC_DET,	   // The function detected 2 or more uncorrectable bits within the row (row is bad)
   SOTP_E_ROW_FAIL_SET,	   // The function detected that 1 or both of the fail bits are set (row is bad)
   SOTP_E_ROW_READ_LOCK,   // The function detected that the row is locked from reading
   SOTP_E_ROW_FUSE_LOCK,   // The function detected that the row is locked from further fusing
   SOTP_E_ROW_TIMEOUT,	   // The function experienced unexpected FSM timeout from the SOTP block 
   SOTP_E_ROW_ERROR	   // The function experienced unexpected register return value from the SOTP block 
} SotpRowStatus;


typedef enum
{
   SOTP_S_KEY_SUCCESS,	   // The function has completed successfully, and a credential exists
   SOTP_S_KEY_EMPTY,	   // The function has completed successfully, but the section is empty
   SOTP_E_KEY_BADPARAM,	   // The function received a bad input parameter 
   SOTP_E_KEY_OVERRUN,	   // The function ran out of rows within the section before the entire key and crc was fused
   SOTP_E_KEY_UNDERRUN,	   // The function ran out of valid section rows before reading the entire key and crc
   SOTP_E_KEY_CRC_MIS,	   // The function detected a CRC mismatch during the key read (key is bad or empty)
   SOTP_E_KEY_ERROR	   // The function experienced an unexpected return value  
} SotpKeyStatus;


SotpKeyStatus sotpWriteSecKey(uint32_t section, uint32_t *pSrc, uint32_t len);
SotpKeyStatus sotpReadSecKey(uint32_t  section, uint32_t *pDst, uint32_t len);
SotpKeyStatus sotpLockSecKeyRead(uint32_t section);
SotpMode      sotpSetMode(SotpMode mode);
SotpMode      sotpGetMode(void);

#endif
