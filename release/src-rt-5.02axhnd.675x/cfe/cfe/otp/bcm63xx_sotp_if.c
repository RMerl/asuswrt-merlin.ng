/*
<:label-BRCM:2012:NONE:standard

:>
*/

/*  *********************************************************************
    *  
    *  bcm63xx_sotp_if.c       
    *
    *  Author:  Brian Nay (brian.nay@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2011
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This program is the proprietary software of Broadcom Corporation and/or its
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

#include "bcm63xx_sotp.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "lib_crc.h"
#include "bcm_map.h"

#if (CFG_RAMAPP)

#define info(... ) { printf( __VA_ARGS__ ); }
#define print(... ) { }

#else

#define info(... ) { }
#if (INC_BTRM_BUILD==1)
extern int btrm_print(int);
#define print(... ) {btrm_print( __VA_ARGS__); }
#else
#define print(... ) { }
#endif

#endif

SotpKeyStatus sotpInit(void * sotpBasePtr)
{
    int res = sotp_init(sotpBasePtr);
    return ( (res == 0)?SOTP_S_KEY_SUCCESS:SOTP_E_KEY_ERROR ) ;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpReadSecKey(uint32_t section, uint32_t *pDst, uint32_t len)
    *  
    *  Description - Used by the bootrom or bootloader to read a key from SOTP
    *
    *  Input parameters: 
    *  	   section - Section that the key info should be read from
    *  	             (7 =< section =< 13)
    *      pDst    - pointer to the buffer containing the key (chopped 
    *                up into 32 bit words)
    *      len     - The number of 32 bit words in the key to read in (len =< 8)
    *  	   
    *  Return value:
    *  	   SOTP_S_KEY_SUCCESS - The function has completed successfully, and
    *  	                        pDst holds valid data
    *  	   SOTP_S_KEY_EMPTY   - The function has completed successfully, but 
    *  	                        no credential is currently stored in this section
    *      SOTP_E_KEY_CRC_MIS - The function detected a CRC mismatch during the key read 
    *  	   SOTP_E_KEY_UNDERRUN- The function ran out of rows within a section
    *  	                        before reading the entire key and crc
    *  	   SOTP_E_KEY_ERROR   - The function experienced an error in one of the
    *  	                        rows of the section
    *
    ********************************************************************* */
SotpKeyStatus sotpReadSecKey(uint32_t section, uint32_t *pDst, uint32_t len)
{
    SotpKeyStatus result;
    sotp_get_keyslot_data( section, (char*)pDst, len * sizeof(uint32_t), (int*)&result );
    return result;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpLockSecKeyRead(uint32_t section)
    *  
    *  Description - After the bootrom or bootloader has read a key from
    *                SOTP, this function can be called so that no 
    *                other sw can read this key at a later time.
    *
    *  Input parameters: 
    *  	   section - There are 7 locations called sections that can hold 
    *  	             up to 256 bits of key information. (7 =< section =< 13)
    *  	             This is the location that is to be locked down from
    *  	             reading until the next power-on-reset.
    *  	   
    *  Return value:
    *  	   SOTP_S_KEY_SUCCESS     - The function has completed successfully
    *  	   SOTP_E_KEY_BADPARAM    - The function recieved a bad input parameter
    *
    ********************************************************************* */
SotpKeyStatus sotpLockSecKeyRead(uint32_t section)
{
    SotpKeyStatus result;
    sotp_set_keyslot_readlock(section, (int*)&result);
    return result;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpWriteSecKey(uint32_t section, uint32_t *pSrc, uint32_t len)
    *  
    *  Description - Used during manufacturing to fuse a security credential
    *                into SOTP
    *
    *  Input parameters: 
    *  	   section - There are 7 locations called sections that can hold 
    *  	             up to 256 bits of key information. (7 =< section =< 13)
    *      pSrc    - pointer to the buffer containing the key (chopped 
    *                up into 32 bit words)
    *      len     - The number of 32 bit words in the key (len =< 8)
    *  	   
    *  Return value:
    *  	   SOTP_S_KEY_SUCCESS - The function has completed successfully
    *  	   SOTP_E_KEY_BADPARAM- The function recieved a bad input parameter
    *  	   SOTP_E_KEY_OVERRUN - The function ran out of rows within a section
    *  	                        before the entire key and crc was fused
    *  	   SOTP_E_KEY_TIMEOUT - The function experienced unexpected FSM 
    *  	                        timeout from the SOTP block 
    *  	   SOTP_E_KEY_ERROR   - The function experienced unexpected register 
    *  	                        ret value from SOTP block 
    *
    ********************************************************************* */
SotpKeyStatus sotpWriteSecKey(uint32_t section, uint32_t *pSrc, uint32_t len)
{
    SotpKeyStatus result;
    sotp_set_keyslot_data( section, (char*)pSrc, len * sizeof(uint32_t), (int*)&result );
    return result;
}

/*  *********************************************************************
    *  extern SotpMode sotpSetMode(void)
    *  
    *  Description - After the manufacuring personalization is complete,
    *                this routine is called. Upon the next POR, the bootrom
    *                will know that the SOTP block is in the field mode
    *                rather than manufacturing mode.
    *
    *  Input parameters: 
    *  	   SOTP_S_MODE_FIELD  - The SOTP block has been personalized and
    *  	                        is ready for deployment
    *  	   
    *  Return value:
    *  	   SOTP_S_MODE_FIELD  - The function has completed successfully
    *  	   SOTP_E_MODE_TIMEOUT- The function experienced unexpected FSM 
    *  	                        timeout from the SOTP block 
    *  	   SOTP_E_MODE_ERROR  - The function experienced unexpected register 
    *  	                        ret value from SOTP block 
    *
    ********************************************************************* */
SotpMode sotpSetMode(SotpMode mode)
{
   SotpMode rval = SOTP_E_MODE_ERROR;

   return rval;
}

/*  *********************************************************************
    *  extern SotpMode sotpGetMode(void)
    *  
    *  Description - Returns what mode the SOTP block is in.
    *
    *  Input parameters: 
    *  	   none 
    *  	   
    *  Return value:
    *  	   SOTP_S_MODE_MFG    - The SOTP block has not been personalized and
    *  	                        is in the manufacturing mode
    *  	   SOTP_S_MODE_FIELD  - The SOTP block has been personalized and
    *  	                        is ready for deployment
    *  	   SOTP_E_MODE_TIMEOUT- The function experienced unexpected FSM 
    *  	                        timeout from the SOTP block 
    *  	   SOTP_E_MODE_ERROR  - The function experienced unexpected register 
    *  	                        ret value from SOTP block 
    *
    ********************************************************************* */
SotpMode sotpGetMode(void)
{
   SotpMode rval = SOTP_E_MODE_ERROR;

   return rval;
}
