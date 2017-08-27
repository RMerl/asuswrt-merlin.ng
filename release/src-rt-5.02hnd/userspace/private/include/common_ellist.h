/***********************************************************************
 *
 *  Copyright (c) 2015  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2015:proprietary:standard

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
 *
 ************************************************************************/

/*****************************************************************************
*    Description:
*
*      Extended common link list header file.
*
*****************************************************************************/

#ifndef _COMMON_ELLIST_H
#define _COMMON_ELLIST_H

/* ---- Include Files ----------------------------------------------------- */

#include "bcm_common_llist.h"
#include "os_defs.h"


/* ---- Constants and Types ----------------------------------------------- */

#define BCM_COMMON_DECLARE_ELL(_name) COMMON_ELL _name

typedef struct
{
    BCM_COMMON_LLIST list;
    UINT32 keyLen;
    UINT32 count;
} COMMON_ELL;

typedef struct
{
   BCM_COMMON_DECLARE_LL_ENTRY();
   UINT8 data[0];
} COMMON_ELL_ENTRY;

typedef void (*LLIST_CB)(COMMON_ELL_ENTRY *entryP, void *usrDataP);

#define ELL_GET_COUNT(ell) (ell.count)

/* ---- Variable Externs -------------------------------------------------- */

/* ---- Function Prototypes ----------------------------------------------- */

void ellInit(COMMON_ELL *ellP, UINT32 keyLen);
COMMON_ELL_ENTRY *ellAddEntry(COMMON_ELL *ellP, COMMON_ELL_ENTRY *entryP);
COMMON_ELL_ENTRY *ellRemoveEntry(COMMON_ELL *ellP, void *key);
COMMON_ELL_ENTRY *ellFindEntry(COMMON_ELL *ellP, void *key);
void ellIterateList(COMMON_ELL *ellP, LLIST_CB cb, void *usrDataP,
  UBOOL8 removeB);

#endif /* _COMMON_ELLIST_H */
