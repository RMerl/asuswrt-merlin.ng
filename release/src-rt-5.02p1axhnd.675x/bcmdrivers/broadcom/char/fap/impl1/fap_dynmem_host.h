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
 * File Name  : fap_dynmem_host
 *
 * Description: This file contains the host-specific interface into the 
 *              fap dynamic memory
 *******************************************************************************
 */

#ifndef _FAP_DYNMEM_HOST_H_INCLUDED_
#define _FAP_DYNMEM_HOST_H_INCLUDED_

#if !defined(DYN_MEM_TEST_APP)
#include "fap_hw.h"
#include "fap4ke_memory.h"
#include "fap4ke_dynmem_shared.h"
#endif
#include "fap_dynmem.h"

#ifdef FAP_4KE
#error "FILE SHOULD NOT BE INCLUDED FROM FAP 4KE"
#endif

typedef enum
{
    FAP_DM_DBG_DUMP_TYPE_NO_PRINT = 0,
    FAP_DM_DBG_DUMP_TYPE_SHORT,
    FAP_DM_DBG_DUMP_TYPE_LONG        
} fapDm_DebugDumpType;

typedef enum 
{
    FAP_DM_DBG_DUMP_ALL_REGIONS = 0,
    FAP_DM_DBG_DUMP_REGION,
    FAP_DM_DBG_DUMP_REGION_SHORT_MASK = 0x8
} fapDm_DebugType;


/* Interface  : note, implementation in fap_slob.c */
void  fapDm_init( void );

fapDm_BlockId  fapDm_alloc(  uint32                 fapIdx,
                             uint32                 size, 
                             fapDm_RegionOrder      order
                                       );

void  fapDm_free(   uint32            fapIdx,
                    fapDm_BlockId     blockId);

void  fapDm_shrink(   uint32            fapIdx,
                      fapDm_BlockId     blockId,
                      uint32            newSize );

void  fapDm_debug( uint32           fapIdx,
                   fapDm_DebugType  type,
                   uint32           val );
        

#if !defined(DYN_MEM_TEST_APP)
static inline void * fapDm_getHostAddrFromBlockId(  uint32 fapIdx, fapDm_BlockId blockId );
static inline fapDm_RegionIdx fapDm_getRegionFromBlockId( uint32 fapIdx, fapDm_BlockId blockId );
static inline uint32 fapDm_getOffsetFromBlockId( uint32 fapIdx, fapDm_BlockId blockId );

static inline fapDm_BlockId fapDm_getFlowBlockId(uint32 fapIdx, fap4kePkt_flowId_t flowId);
static inline void fapDm_setFlowBlockId(uint32 fapIdx, fap4kePkt_flowId_t flowId, fapDm_BlockId blockId);

static inline fapDm_RegionIdx fapDm_getFlowRegionFromFlowId(uint32 fapIdx, fap4kePkt_flowId_t flowId);
static inline uint8 * fapDm_getCmdListFromFlowId( uint32 fapIdx, fap4kePkt_flowId_t flowId );
#endif



/************************ Implementations (inlined) ************************/

#if !defined(DYN_MEM_TEST_APP)

static __always_inline void * fapDm_getHostBaseAddrFromRegion( uint32 fapIdx, fapDm_RegionIdx regionIdx )
{
    switch(regionIdx)
    {
        case FAP_DM_REGION_PSM:
            return (void *)(((uint32)pHostPsmGbl(fapIdx) + sizeof(fap4kePsm_global_t) + 3) & ~0x3);
        case FAP_DM_REGION_QSM:
            return (void *)(((uint32)pHostQsmGbl(fapIdx) + sizeof(fap4keQsm_global_t) + FAP_TOTAL_DQM_SIZE + 3) & ~0x3);
        case FAP_DM_REGION_HP:
            /* Inline recursion doesn't work as well as I'd like, so duplicating formulas here: */
            if (IS_HP_IN_PSM)
                return (void *)(((uint32)pHostPsmGbl(fapIdx) + sizeof(fap4kePsm_global_t) + 3 + FAP_DM_PSM_SIZE) & ~0x3);
            else
                return (void *)(((uint32)pHostQsmGbl(fapIdx) + sizeof(fap4keQsm_global_t) + FAP_TOTAL_DQM_SIZE + 3 + FAP_DM_QSM_SIZE) & ~0x3);
        default:
            return NULL;
    }
}


static inline void * fapDm_getHostAddrFromBlockId( uint32 fapIdx, fapDm_BlockId blockId )
{
    fapDm_BlockInfo blockInfo;
    void * regionBase;

    blockInfo.id = blockId;
    regionBase = fapDm_getHostBaseAddrFromRegion(fapIdx, blockInfo.regionIdx);

    if (regionBase)
    {
        return (void *)((uint32)regionBase + blockInfo.offset);
    }
    else
    {
        //printk("[%s.%d]: fapDm_getHostAddrFromBlockId(%d,%08lx) \n", __func__, __LINE__, fapIdx, blockId);    
        return NULL;
    } 
}

// Hmmm... don't like the host having knowledge of a 4ke addresses...  Sharp stick if I
// ever saw one.  But it might save a couple of cycles in the data path
static __always_inline void * fapDm_get4keBaseAddrFromRegion( uint32 fapIdx, fapDm_RegionIdx regionIdx )
{
    switch(regionIdx)
    {
        case FAP_DM_REGION_DSP:
            return (void *)(((uint32)p4keDspramGbl + sizeof(fap4keDspram_global_t) + 3) & ~0x3);
        case FAP_DM_REGION_PSM:
            return (void *)(((uint32)p4kePsmGbl + sizeof(fap4kePsm_global_t) + 3) & ~0x3);
        case FAP_DM_REGION_QSM:
            return (void *)(((uint32)p4keQsmGbl + sizeof(fap4keQsm_global_t)+ FAP_TOTAL_DQM_SIZE + 3) & ~0x3);
        case FAP_DM_REGION_HP:
            if (IS_HP_IN_PSM)
                return (void *)(((uint32)p4kePsmGbl + sizeof(fap4kePsm_global_t) + 3 + FAP_DM_PSM_SIZE) & ~0x3);
            else
                return (void *)(((uint32)p4keQsmGbl + sizeof(fap4keQsm_global_t)+ FAP_TOTAL_DQM_SIZE + 3 + FAP_DM_QSM_SIZE) & ~0x3);
        default:
            return NULL;
    }
}

static __always_inline void * fapDm_get4keAddrFromBlockId( uint32 fapIdx, fapDm_BlockId blockId )
{
    fapDm_BlockInfo blockInfo;
    void * regionBase;

    if (blockId == FAP_DM_INVALID_BLOCK_ID)
        return NULL;

    blockInfo.id = blockId;
    regionBase = fapDm_get4keBaseAddrFromRegion(fapIdx, blockInfo.regionIdx);

    if (regionBase)
    {
        return (void *)((uint32)regionBase + blockInfo.offset);
    }
    else
    {
        return NULL;
    } 
}




static __always_inline fapDm_RegionIdx fapDm_getRegionFromBlockId( uint32 fapIdx, fapDm_BlockId blockId )
{
    fapDm_BlockInfo blockInfo;    
    blockInfo.id = blockId;
    return blockInfo.regionIdx;
}

static __always_inline uint32 fapDm_getOffsetFromBlockId( uint32 fapIdx, fapDm_BlockId blockId )
{
    fapDm_BlockInfo blockInfo;    
    blockInfo.id = blockId;
    return blockInfo.offset;
}

static __always_inline void fapDm_setFlowBlockId(uint32 fapIdx, fap4kePkt_flowId_t flowId, fapDm_BlockId blockId)
{
    pHostFapSdram(fapIdx)->dynMem.flows[flowId] = blockId;
}

static __always_inline fapDm_BlockId fapDm_getFlowBlockId(uint32 fapIdx, fap4kePkt_flowId_t flowId)
{
    return pHostFapSdram(fapIdx)->dynMem.flows[flowId];
}

static __always_inline void fapDm_setCmdListBlockId(uint32 fapIdx, fap4kePkt_flowId_t flowId, fapDm_BlockId blockId)
{
    pHostFapSdram(fapIdx)->dynMem.cmdListBids[flowId] = blockId;
}

static __always_inline fapDm_BlockId fapDm_getCmdListBlockId(uint32 fapIdx, fap4kePkt_flowId_t flowId)
{
    return pHostFapSdram(fapIdx)->dynMem.cmdListBids[flowId];
}

static __always_inline fapDm_RegionIdx fapDm_getFlowRegionFromFlowId(uint32 fapIdx, fap4kePkt_flowId_t flowId)
{
    return fapDm_getRegionFromBlockId(fapIdx, fapDm_getFlowBlockId(fapIdx, flowId));
}

static __always_inline fap4kePkt_flowStats_t * fapDm_getStatsFromFlowId( uint32 fapIdx, fap4kePkt_flowId_t flowId )
{
    return &(pHostFapSdram(fapIdx)->packet.stats[flowId]);
}

static __always_inline uint8 * fapDm_getCmdListFromFlowId( uint32 fapIdx, fap4kePkt_flowId_t flowId )
{
    return (  fapDm_getHostAddrFromBlockId(fapIdx, fapDm_getCmdListBlockId(fapIdx, flowId) ) );
}

#else

static inline void * fapDm_getHostBaseAddrFromRegion( uint32 fapIdx, fapDm_RegionIdx regionIdx )
{
    switch(regionIdx)
    {
        case FAP_DM_REGION_PSM:
            return (void *)(psm);
        case FAP_DM_REGION_QSM:
            return (void *)(qsm);
        default:
            return NULL;
    }
}

static inline fapDm_RegionIdx fapDm_getRegionFromBlockId( uint32 fapIdx, fapDm_BlockId blockId )
{
    fapDm_BlockInfo blockInfo;    
    blockInfo.id = blockId;
    return blockInfo.regionIdx;
}

static inline uint32 fapDm_getOffsetFromBlockId( uint32 fapIdx, fapDm_BlockId blockId )
{
    fapDm_BlockInfo blockInfo;    
    blockInfo.id = blockId;
    return blockInfo.offset;
}


#endif


#endif
