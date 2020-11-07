/*
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

#ifndef SOTP_IOCTL_H_DEFINED
#define SOTP_IOCTL_H_DEFINED

#include <bcmtypes.h>

#define SOTP_IOCTL_MAGIC       'S'
#define SOTP_DRV_MAJOR          341

#define SOTP_NUM_WORDS_IN_KEYSLOT 8
#define SOTP_NUM_ROWS_IN_KEYSLOT 8
#define SOTP_NUM_DATABITS_IN_KEYSLOT (SOTP_NUM_WORDS_IN_KEYSLOT*4*8)
typedef enum
{
    SOTP_ROW,
    SOTP_REGION_READLOCK,
    SOTP_REGION_FUSELOCK,
    SOTP_KEYSLOT,
    SOTP_KEYSLOT_READLOCK,
    SOTP_MAP,
} SOTP_ELEMENT;

typedef struct sotpIoctlParms
{
    SOTP_ELEMENT element;
    uint32_t row_addr;              /* To be used when accessing rows */ 
    uint32_t region_num;            /* To be used when locking regions */
    uint32_t keyslot_section_num;   /* To be used when accessing keyslots 
                                     * See sotp_base_defs.h for section numbers
                                     * for keyslots */
                                
    BCM_IOC_PTR(void *, inout_data);
    uint32_t data_len;
    uint32_t raw_access;
    uint32_t result;
} SOTP_IOCTL_PARMS;


#define SOTP_IOCTL_GET             _IOWR(SOTP_IOCTL_MAGIC, 0, SOTP_IOCTL_PARMS)
#define SOTP_IOCTL_SET             _IOWR(SOTP_IOCTL_MAGIC, 1, SOTP_IOCTL_PARMS)

#endif
