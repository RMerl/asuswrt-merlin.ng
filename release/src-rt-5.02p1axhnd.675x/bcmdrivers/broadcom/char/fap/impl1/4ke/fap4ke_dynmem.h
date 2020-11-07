/*
<:copyright-BRCM:2012:proprietary:standard

   Copyright (c) 2012 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : fap4ke_dynmem
 *
 * Description: This file contains the 4ke-specific interface into the 
 *              fap dynamic memory
 *******************************************************************************
 */

#ifndef _FAP4KE_DYNMEM_H_INCLUDED_
#define _FAP4KE_DYNMEM_H_INCLUDED_

#include "fap4ke_memory.h"
#include "fap4ke_dynmem_shared.h"

//TBD: move this somewhere better
#define __always_inline		inline __attribute__((always_inline))
#define unlikely(x) __builtin_expect((x),0)

static __always_inline void * fap4ke_getRegionStartAddr( fapDm_RegionIdx regionIdx )
{
    void * addrs[FAP_DM_REGION_MAX] = { 
        [FAP_DM_REGION_DSP] = (void *)(((uint32)p4keDspramGbl + sizeof(fap4keDspram_global_t) + 3) & ~0x3),
        [FAP_DM_REGION_PSM] = (void *)(((uint32)p4kePsmGbl + sizeof(fap4kePsm_global_t) + 3) & ~0x3),
        [FAP_DM_REGION_QSM] = (void *)(((uint32)p4keQsmGbl + sizeof(fap4keQsm_global_t) + FAP_TOTAL_DQM_SIZE + 3) & ~0x3 ),        
        [FAP_DM_REGION_HP] = IS_HP_IN_PSM 
                             ? (void *)(((uint32)p4kePsmGbl + sizeof(fap4kePsm_global_t) + 3 + FAP_DM_PSM_SIZE) & ~0x3)
                             : (void *)(((uint32)p4keQsmGbl + sizeof(fap4keQsm_global_t) + FAP_TOTAL_DQM_SIZE + 3 + FAP_DM_QSM_SIZE) & ~0x3 ) };
    return addrs[regionIdx];
}


static __always_inline void * fap4ke_getAddrFromBlockId( fapDm_BlockId blockId )
{
    fapDm_BlockInfo blockInfo;
    blockInfo.id = blockId;

    if ( unlikely (blockId == FAP_DM_INVALID_BLOCK_ID))
    {
        return NULL;
    }

    return (void *)((uint32)fap4ke_getRegionStartAddr(blockInfo.regionIdx) + blockInfo.offset);
}

static __always_inline fapDm_BlockId      fap4ke_getFlowBlockIdFromFlowId( fap4kePkt_flowId_t flowId )
{
    return p4keSdram->dynMem.flows[flowId];
}

static __always_inline fap4kePkt_flow_t * fap4ke_getFlowPtrFromFlowId( fap4kePkt_flowId_t flowId )
{
    fapDm_BlockId blockId;
    blockId = fap4ke_getFlowBlockIdFromFlowId(flowId);
    return (fap4kePkt_flow_t *) fap4ke_getAddrFromBlockId( blockId );
}

#endif /* _FAP4KE_DYNMEM_H_INCLUDED_ */

