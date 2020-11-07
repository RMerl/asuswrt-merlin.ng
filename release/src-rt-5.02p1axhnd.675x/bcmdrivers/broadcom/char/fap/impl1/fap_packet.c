/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
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
//**************************************************************************
// File Name  : fap4ke_packet.c
//
// Description: This file contains the host-side implementation of the FAP
//              Packet layer.
//               
//**************************************************************************

#include <linux/module.h>
#include <linux/bcm_log.h>
#include "fap_hw.h"
#include "fap4ke_memory.h"
#include "fap_local.h"
#include "fap4ke_msg.h"
#include "fap_packet.h"
#include "fap_dqmHost.h"
#include "fap_dqm.h"
#include "fap.h"
#include "fap4ke_iopDma.h"
#include "fap4ke_msg.h"
#include "bcmPktDma.h"
#include "fap_dynmem_host.h"
#include "fap_mcast.h"
#include "fap_protocol.h"

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
extern spinlock_t fapProto_lock_g;
#endif

#if(FAP4KE_PKT_MAX_FLOWS > 1024)
#error "FAP4KE_PKT_MAX_FLOWS is too large for fapPkt_flowHandle_t"
#endif

#if(NUM_FAPS > 2)
#error "NUM_FAPS is too large for fapPkt_flowHandle_t"
#endif

static fap4kePkt_flowId_t gNextFlowId[NUM_FAPS] = {0};

static inline fap4kePkt_flowStats_t * 
getHostStats(uint32 fapIdx, fap4kePkt_flowId_t flowId)
{
    return fapDm_getStatsFromFlowId(fapIdx,flowId);
}

static inline uint8 * 
getHostCmdList(uint32 fapIdx, fap4kePkt_flowId_t flowId)
{
    return fapDm_getCmdListFromFlowId(fapIdx,flowId);
}

#if defined(CC_FAP_ENET_STATS)
typedef struct {
    uint32_t contextFull;
    uint32_t dqmRxFull;
    uint32_t rxPackets;
    uint32_t txPackets;
    uint32_t contextCount;
    uint32_t interrupts;
} fapEnet_stats_t;

static fapEnet_stats_t fapEnet_stats_g;
#endif

uint32_t txHighWm = 0;

fapPkt_flowAlloc_t flowAlloc_g[NUM_FAPS][FAP4KE_PKT_MAX_FLOWS];

void dumpMemHost(uint8 *mem_p, uint32 length);

static fapPkt_flowHandle_t allocFlow(uint32 fapIdx, bool isHp)
{
    fap4kePkt_flowId_t flowId;
    fapPkt_flowAlloc_t *flowAlloc_p;
    fapPkt_flowHandle_t flowHandle;
    fap4kePkt_flowId_t startFlowId;

    flowHandle.u16 = FAP4KE_PKT_INVALID_FLOWHANDLE;

    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid fapIdx <%d>", fapIdx);

        goto out;
    }

    startFlowId = gNextFlowId[fapIdx];
    
    if (startFlowId > FAP4KE_PKT_MAX_FLOWS - FAP_DM_RSVD_HP_FLOW_CNT)
        startFlowId = 0;
    
    for (flowId = startFlowId; flowId < FAP4KE_PKT_MAX_FLOWS - FAP_DM_RSVD_HP_FLOW_CNT; flowId++)
    {
        flowAlloc_p = &flowAlloc_g[fapIdx][flowId];
        if(!flowAlloc_p->isAlloc)
            goto found;
    }

    for (flowId = 0; flowId < startFlowId; flowId++)
    {
        flowAlloc_p = &flowAlloc_g[fapIdx][flowId];
        if(!flowAlloc_p->isAlloc)
            goto found;
    }

    if (isHp)
    {
        for (flowId = FAP4KE_PKT_MAX_FLOWS - FAP_DM_RSVD_HP_FLOW_CNT; flowId < FAP4KE_PKT_MAX_FLOWS; flowId++)
        {
            flowAlloc_p = &flowAlloc_g[fapIdx][flowId];
            if(!flowAlloc_p->isAlloc)
                goto found;
        }        
    }
    
    goto out;

found:
    flowAlloc_p->isAlloc = 1;
    
    flowHandle.u16 = 0;
    flowHandle.fapIdx = fapIdx;
    flowHandle.flowId = flowId;
    gNextFlowId[fapIdx] = (flowId == FAP4KE_PKT_MAX_FLOWS-1) ? 0 : flowId + 1;
out:
    return flowHandle;
}

static fapRet freeFlow(fapPkt_flowHandle_t flowHandle)
{
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;
    fapPkt_flowAlloc_t *flowAlloc_p;

    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid fapIdx <%d>", fapIdx);

        return FAP_ERROR;
    }

    if(flowId >= FAP4KE_PKT_MAX_FLOWS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flowId <%d>", flowId);

        return FAP_ERROR;
    }

    flowAlloc_p = &flowAlloc_g[fapIdx][flowId];

    if(!flowAlloc_p->isAlloc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "flowId <%d> is not allocated", flowId);

        return FAP_ERROR;
    }

    flowAlloc_p->isAlloc = 0;

    gNextFlowId[fapIdx] = flowId;

    return FAP_SUCCESS;
}

fapPkt_flowHandle_t fapPkt_mcastActivate(uint32 fapIdx,
                                    fap4kePkt_flowInfo_t *flowInfo, 
                                    uint8 *checksum1, 
                                    uint8 *checksum2,
                                    uint32 mcCfglogIdx)
{
    fapPkt_flowHandle_t flowHandle;
    fap4kePkt_flowId_t flowId;
    xmit2FapMsg_t fapMsg;
    
    uint16          flowSize;
    fapDm_BlockId   flowBid;

    flowHandle = allocFlow(fapIdx, TRUE);
    if(flowHandle.u16 == FAP4KE_PKT_INVALID_FLOWHANDLE)
    {
        goto out;
    }

    if (flowInfo->fapMtu < MIN_FAP_MTU || flowInfo->fapMtu > MAX_FAP_MTU)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "ERROR ACTIVATING FLOW -- BAD MTU (fapIdx=%d, mtu=%d)\n", fapIdx, flowInfo->fapMtu);
        flowInfo->fapMtu = MAX_FAP_MTU;
    }

    flowId = flowHandle.flowId;

    if (flowInfo->type == FAP4KE_PKT_FT_IPV6)
        flowSize = fap4kePkt_flowSizeIpv6;
    else
        flowSize = fap4kePkt_flowSizeIpv4;


    flowBid = fapDm_alloc(fapIdx, flowSize, FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP);
    if (flowBid == FAP_DM_INVALID_BLOCK_ID)
    {
        freeFlow(flowHandle);
        flowHandle.u16 = FAP4KE_PKT_INVALID_FLOWHANDLE;
        goto out;
    }

    fapDm_setFlowBlockId( fapIdx, flowId,  flowBid);
    
    pHostFlowInfoPool(fapIdx)[flowId] = *flowInfo;
    
    fapMsg.flowCfg.flowId = ((mcCfglogIdx & 0xFFFF) << 16) | (flowId & 0xFFFF);
    flowAlloc_g[fapIdx][flowId].isHostValid = 1;
    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_FLW_ACTIVATE, &fapMsg);
out:
    return flowHandle;
}

fapRet fapPkt_mcastDeactivate(fapPkt_flowHandle_t flowHandle,uint32 mcCfglogIdx)
{
    xmit2FapMsg_t fapMsg;

    flowAlloc_g[flowHandle.fapIdx][flowHandle.flowId].isHostValid = 0;
    fapMsg.flowCfg.flowId = ((mcCfglogIdx & 0xFFFF) << 16) | (flowHandle.flowId & 0xFFFF);
    fapDrv_Xmit2Fap(flowHandle.fapIdx, FAP_MSG_FLW_DEACTIVATE, &fapMsg);

    /* NOTE: we cannot just free the memory here...
       wait for message from the FAP indicating deactivation has completed */

    return FAP_SUCCESS;
}

fapRet fapPkt_mcastUpdate(fapPkt_flowHandle_t flowHandle, uint32 msgType,
                          fap4kePkt_flowInfo_t *flowInfo, uint32 mcCfglogIdx)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;

    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Bad fapIdx <%d> for flowId <%d>", fapIdx, flowId);
        return FAP_ERROR;
    }

    if(flowId >= FAP4KE_PKT_MAX_FLOWS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flowId <%d>", flowId);

        return FAP_ERROR;
    }

    if(!flowAlloc_g[fapIdx][flowId].isAlloc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Inactive flowId <%d>", flowId);

        return FAP_ERROR;
    }

    /* update flow info and notify the FAP */
    pHostFlowInfoPool(fapIdx)[flowId] = *flowInfo;

    fapMsg.flowCfg.flowId = ((mcCfglogIdx & 0xFFFF) << 16) | (flowId & 0xFFFF);
    fapDrv_Xmit2Fap(fapIdx, msgType, &fapMsg);

    return FAP_SUCCESS;
}

void fapPkt_mcastSetMissBehavior(int dropMcastMiss)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.generic.word[0] = dropMcastMiss;

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_MCAST_SET_MISS_BEHAVIOR, &fapMsg);

        mdelay(500);
    }
}

fapPkt_flowHandle_t fapPkt_activate(uint32 fapIdx,
                                    fap4kePkt_flowInfo_t *flowInfo, 
                                    uint8 *cmdList,
                                    uint16 cmdListSize,
                                    uint8 *checksum1, 
                                    uint8 *checksum2,
                                    fap4kePkt_learnAction_t *learnAction_p)
{
    fapPkt_flowHandle_t flowHandle;
    fap4kePkt_flowId_t flowId;
    xmit2FapMsg_t fapMsg;
    
    uint16          sharedSize;
    fapDm_BlockId   sharedBid;
    fapDm_BlockId   flowBid;

    flowHandle = allocFlow(fapIdx, FALSE);
    if(flowHandle.u16 == FAP4KE_PKT_INVALID_FLOWHANDLE)
    {
        goto out;
    }

    if(flowInfo->type != FAP4KE_PKT_FT_L2)
    {
        if (flowInfo->fapMtu < MIN_FAP_MTU || flowInfo->fapMtu > MAX_FAP_MTU)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "ERROR ACTIVATING FLOW -- BAD MTU (fapIdx=%d, mtu=%d)\n",
                          fapIdx, flowInfo->fapMtu);

            flowInfo->fapMtu = MAX_FAP_MTU;
        }
    }

    flowId = flowHandle.flowId;

    if (cmdListSize >= FAP4KE_PKT_CMD_LIST_SIZE)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "ERROR: Invalid cmd List size! (fapIdx = %d, flowId=%d, cmdListSize=%d)\n", fapIdx, flowId, cmdListSize);
        goto out;
    }

    cmdListSize = (cmdListSize + 3) & (~3);
    sharedSize = ( cmdListSize +  (checksum1 ? FAP4KE_PKT_CSUM_CMD_LIST_SIZE : 0) + (checksum2 ? FAP4KE_PKT_CSUM_CMD_LIST_SIZE : 0) );

    if (sharedSize)
    {
        sharedBid = fapDm_alloc(fapIdx, sharedSize, FAP_DM_REGION_ORDER_QSM_PSM);
        if (sharedBid == FAP_DM_INVALID_BLOCK_ID)
        {
            freeFlow(flowHandle);        
            flowHandle.u16 = FAP4KE_PKT_INVALID_FLOWHANDLE;
            //BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "ERROR: Could not allocate cmdlist (fapIdx = %ld, flowId=%d, cmdListSize=%d)\n", fapIdx, flowId, cmdListSize);
            goto out;
        }
    }
    else
    {
        sharedBid = FAP_DM_INVALID_BLOCK_ID;
    }
 

    flowBid = fapDm_alloc(fapIdx, sizeof(fap4kePkt_flow_t), FAP_DM_REGION_ORDER_DSP_PSM_QSM);
    if (flowBid == FAP_DM_INVALID_BLOCK_ID)
    {
        freeFlow(flowHandle);
        flowHandle.u16 = FAP4KE_PKT_INVALID_FLOWHANDLE;
        fapDm_free(fapIdx, sharedBid);
        BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "ERROR: Could not allocate flow (fapIdx = %d, flowId=%d, cmdListSize=%d)\n", fapIdx, flowId, cmdListSize);
        goto out;
    }

    fapDm_setFlowBlockId( fapIdx, flowId,  flowBid);
    fapDm_setCmdListBlockId( fapIdx, flowId,  sharedBid);
    
    if (cmdListSize)
        memcpy( getHostCmdList(fapIdx, flowId), cmdList, cmdListSize);

#if defined(CC_FAP4KE_PKT_HW_ICSUM)
    {
        uint8 * pBlockCheckSum1 = NULL;
        uint8 * pBlockCheckSum2 = NULL;
        
        /* TBD: this is error prone -- double and tripple check no mistakes were made!!! */
        if ( checksum1 != NULL )
        {
            BCM_ASSERT( cmdListSize != 0 );
            pBlockCheckSum1 = getHostCmdList(fapIdx, flowId) + cmdListSize;
            memcpy( pBlockCheckSum1, checksum1, 
                    FAP4KE_PKT_CSUM_CMD_LIST_SIZE);

            iopDma_CmdListAddChecksum1(getHostCmdList(fapIdx, flowId),
                                       pBlockCheckSum1);
        }

        if(checksum2 != NULL)
        {
            uint8 *cmdList_p;

            if(pBlockCheckSum1 != NULL)
            {
                BCM_ASSERT( cmdListSize >= 8 );
                pBlockCheckSum2 = pBlockCheckSum1 + FAP4KE_PKT_CSUM_CMD_LIST_SIZE;
                cmdList_p = getHostCmdList(fapIdx, flowId) + 4;
            }
            else
            {
                BCM_ASSERT( cmdListSize >= 4 );
                pBlockCheckSum2 = getHostCmdList(fapIdx, flowId) + cmdListSize;
                cmdList_p = getHostCmdList(fapIdx, flowId);
            }

            memcpy(pBlockCheckSum2, checksum2,
                   FAP4KE_PKT_CSUM_CMD_LIST_SIZE);

            iopDma_CmdListAddChecksum2(cmdList_p, pBlockCheckSum2);
        }
        
#if 0
        printk("Command List %d\n", flowId);
        dumpMemHost(pktTablesHost.shared[flowId].cmdList, FAP4KE_PKT_CMD_LIST_SIZE);
        printk("checksum1 : 0x%08lX\n", (uint32)pktTablesHost.shared[flowId].checksum1);
        dumpMemHost(pktTablesHost.shared[flowId].checksum1, FAP4KE_PKT_CSUM_CMD_LIST_SIZE);
        printk("checksum2 : 0x%08lX\n", (uint32)pktTablesHost.shared[flowId].checksum2);
        dumpMemHost(pktTablesHost.shared[flowId].checksum2, FAP4KE_PKT_CSUM_CMD_LIST_SIZE);
#endif
	}
#else /* CC_FAP4KE_PKT_HW_ICSUM */
	/* 
	 * For T6in4 Tunnel, we need to do checksum for the inserted IPv4 Header.
	 * We save the last FAP4KE_PKT_CSUM_CMD_LIST_SIZE bytes in cmdlist 
	 * to use for checksum.
	 */
	 
	if ( checksum1 != NULL )
	{
		uint8 * pBlockCheckSum1 = NULL;
		
	    BCM_ASSERT( cmdListSize != 0 );
	    pBlockCheckSum1 = getHostCmdList(fapIdx, flowId) + cmdListSize;
	    memcpy( pBlockCheckSum1, checksum1, 
	            FAP4KE_PKT_CSUM_CMD_LIST_SIZE);

	    iopDma_CmdListAddChecksum1(getHostCmdList(fapIdx, flowId),
	                               pBlockCheckSum1);
	}
#endif /* CC_FAP4KE_PKT_HW_ICSUM */

    if(flowInfo->type != FAP4KE_PKT_FT_L2)
    {
        if (sharedBid != FAP_DM_INVALID_BLOCK_ID)
        {
            flowInfo->ipv4.cmdList_4keAddr = fapDm_get4keAddrFromBlockId(fapIdx, sharedBid);
        }
        else
        {
            flowInfo->ipv4.cmdList_4keAddr = NULL;
        }
    }

    pHostFlowInfoPool(fapIdx)[flowId] = *flowInfo;

    BCM_LOG_INFO(BCM_LOG_ID_FAP, "extSwTagLen/wlMetadata %d (fapIdx = %d, flowId=%d, cmdListSize=%d)\n", (unsigned int)flowInfo->word, fapIdx, flowId, cmdListSize);
    if(learnAction_p != NULL)
    {
        pHostFapSdram(fapIdx)->packet.learn.action[flowId] = *learnAction_p;
    }

    fapMsg.flowCfg.flowId = flowId;
    flowAlloc_g[fapIdx][flowId].isHostValid = 1;
    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_FLW_ACTIVATE, &fapMsg);

out:
    return flowHandle;
}

fapRet fapPkt_deactivate(fapPkt_flowHandle_t flowHandle)
{
    fapRet ret = FAP_SUCCESS;
    xmit2FapMsg_t fapMsg;

    flowAlloc_g[flowHandle.fapIdx][flowHandle.flowId].isHostValid = 0;
    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "deactivate flow flowHandle=%08x, fapIdx <%d> flowId <%d>", 
        flowHandle.u16, flowHandle.fapIdx, flowHandle.flowId);

    if(ret == FAP_SUCCESS)
    {
        fapMsg.flowCfg.flowId = flowHandle.flowId;
        fapDrv_Xmit2Fap(flowHandle.fapIdx, FAP_MSG_FLW_DEACTIVATE, &fapMsg);
    }
    
        /* NOTE: we cannot just free the memory here...    It is possible that
           fapPkt_activate is called before the FAP has processed the
           FAP_MSG_FLW_DEACTIVATE message.  This is fine for flows, but, it is
           possible that the host overwrites the command list while it is being
           used.  We have to wait for a new message back from the 4ke */

    return ret;
}

fapRet fapPkt_resetStats(fapPkt_flowHandle_t flowHandle)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;

    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Bad fapIdx <%d> for flowId <%d>", fapIdx, flowId);
        return FAP_ERROR;
    }

    if(flowId >= FAP4KE_PKT_MAX_FLOWS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flowId <%d>", flowId);

        return FAP_ERROR;
    }

    if(!flowAlloc_g[fapIdx][flowId].isAlloc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Inactive flowId <%d>", flowId);

        return FAP_ERROR;
    }

    fapMsg.flowCfg.flowId = flowId;
    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_FLW_RESET_STATS, &fapMsg);

    return FAP_SUCCESS;
}

typedef struct {
    uint16              fapIdx;
    fap4kePkt_flowId_t  flowId;
} FapDynMemFreeInfo;

#define freeInfoQSize (FAP4KE_PKT_MAX_FLOWS * NUM_FAPS)
FapDynMemFreeInfo freeInfoQ[freeInfoQSize] = {{0}};
uint32  freeInfoQHead = 0;
uint32  freeInfoQTail = 0;

static inline void pushFreeInfo(uint32 fapIdx, fap4kePkt_flowId_t flowId)
{
    freeInfoQ[freeInfoQHead].fapIdx = (uint32)fapIdx;
    freeInfoQ[freeInfoQHead].flowId = flowId;
    freeInfoQHead = (freeInfoQHead+1) % freeInfoQSize;
    ASSERT(freeInfoQHead != freeInfoQTail);
}

static inline int popFreeInfo(uint32 *pFapIdx, fap4kePkt_flowId_t *pFlowId)
{
    if (freeInfoQHead == freeInfoQTail)
        return 0;
    *pFapIdx = (uint32)freeInfoQ[freeInfoQTail].fapIdx;
    *pFlowId = freeInfoQ[freeInfoQTail].flowId;    
    freeInfoQTail = (freeInfoQTail+1) % freeInfoQSize;
    return 1;
}

void fapDynMemDoTasklet(unsigned long unused);
DECLARE_TASKLET(fapDynMemTasklet, fapDynMemDoTasklet, 0);

fapRet fapPkt_dynMemFreeIsr(uint32 fapIdx, DQMQueueDataReg_S msg)
{
    fap4kePkt_flowId_t flowId;
    
    /* TBD: there's got to be a better way to syncronize message fromats
       between the host and the 4ke...  'word2' seems meaningless here... */
    flowId = msg.word2;
    // schedule a bottom half to handle the message

    pushFreeInfo(fapIdx, flowId);
    tasklet_schedule(&fapDynMemTasklet);
    return FAP_SUCCESS;
}

void fapDynMemDoTasklet(unsigned long unused)
{
    uint32 fapIdx = 0;
    fap4kePkt_flowId_t flowId = 0;
    fapDm_BlockId flowBid;
    fapDm_BlockId cmdListBid;

    FAP_PROTO_LOCK();
    while(popFreeInfo(&fapIdx, &flowId))
    {              
        BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "fapPkt_dynMemFreeHandle: flowId <%d>\n",flowId);
        
        if (flowId >= FAP4KE_PKT_MAX_FLOWS)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flow id - fapIdx <%d>, flowId <%d>", fapIdx, flowId);
            continue;        
        }
    
        flowBid = fapDm_getFlowBlockId(fapIdx, flowId);
        if (flowBid == FAP_DM_INVALID_BLOCK_ID)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flow blockId - fapIdx <%d>, flowId <%d>", fapIdx, flowId);
            continue;        
            
        }
        
        cmdListBid = fapDm_getCmdListBlockId(fapIdx, flowId);
    
        fapDm_setFlowBlockId(fapIdx, flowId, FAP_DM_INVALID_BLOCK_ID);
        fapDm_setCmdListBlockId(fapIdx, flowId, FAP_DM_INVALID_BLOCK_ID);
        
        flowAlloc_g[fapIdx][flowId].isAlloc = 0;
        fapDm_free(fapIdx, flowBid);
        if (cmdListBid != FAP_DM_INVALID_BLOCK_ID)
        {
            fapDm_free(fapIdx, cmdListBid);
        }
    }
    FAP_PROTO_UNLOCK();
}


fapRet fapPkt_getFlowInfo(fapPkt_flowHandle_t flowHandle,
                          fap4kePkt_flowInfo_t *flowInfo)
{
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;
    
    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid fapIdx <%d>", fapIdx);

        return FAP_ERROR;
    }

    if(flowId >= FAP4KE_PKT_MAX_FLOWS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flowId <%d>", flowId);

        return FAP_ERROR;
    }

    if(!flowAlloc_g[fapIdx][flowId].isAlloc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Inactive flowId <%d>", flowId);

        return FAP_ERROR;
    }

    /* update flow information in the FAP */
    {
        *flowInfo = pHostFlowInfoPool(fapIdx)[flowId];
    }

    return FAP_SUCCESS;
}

fapRet fapPkt_setFlowInfo(fapPkt_flowHandle_t flowHandle,
                          fap4kePkt_flowInfo_t *flowInfo)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;

    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Bad fapIdx <%d> for flowId <%d>", fapIdx, flowId);
        return FAP_ERROR;
    }

    if(flowId >= FAP4KE_PKT_MAX_FLOWS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flowId <%d>", flowId);

        return FAP_ERROR;
    }

    if(!flowAlloc_g[fapIdx][flowId].isAlloc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Inactive flowId <%d>", flowId);

        return FAP_ERROR;
    }

    /* update flow info and notify the FAP */
    pHostFlowInfoPool(fapIdx)[flowId] = *flowInfo;

    fapMsg.flowCfg.flowId = flowId;
    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_FLW_UPDATE, &fapMsg);

    return FAP_SUCCESS;
}

const char * getFlowMemTypeString(uint32 fapIdx, fap4kePkt_flowId_t flowId)
{
    switch (fapDm_getFlowRegionFromFlowId(fapIdx, flowId))
    {
        case FAP_DM_REGION_DSP:
            return "DSP"; 
            break;
        case FAP_DM_REGION_PSM:
            return "PSM"; 
            break;
        case FAP_DM_REGION_QSM:
            return "QSM"; 
            break;
        default:
            return "<ERR: invalid flow ID>"; 
            break;
    }  
}

static uint8 hashIxBins[FAP4KE_PKT_HASH_TABLE_SIZE];

#if 1
static void fapPkt_dumpFlowHost(fapPkt_flowHandle_t flowHandle)
{
    fap4kePkt_flowInfo_t *flowInfo_p;
    fap4kePkt_flowStats_t *stats_p;
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;
    char *srcPhyType;
    char *destPhyType;
    char *destChannelType;

    flowInfo_p = &pHostFlowInfoPool(fapIdx)[flowId];
    stats_p = getHostStats(fapIdx, flowId);

    if(flowInfo_p->source.phy == FAP4KE_PKT_PHY_ENET)
    {
        srcPhyType = "ENET";
    }
    else if(flowInfo_p->source.phy == FAP4KE_PKT_PHY_GPON)
    {
        srcPhyType = "GPON";
    }
    else
    {
        srcPhyType = "XTM";
    }

    if(flowInfo_p->dest.phy == FAP4KE_PKT_PHY_ENET)
    {
        destPhyType = "ENET";
        destChannelType = "dchannelMask";
    }
    if(flowInfo_p->dest.phy == FAP4KE_PKT_PHY_GPON)
    {
        destPhyType = "GPON";
        destChannelType = "dchannelMask";
    }
    else if (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_WLAN)
    {
        destPhyType = "WLAN";
        destChannelType = "wlTxChainIdx";
    }	
    else
    {
        destPhyType = "XTM";
        destChannelType = "dchannel";
    }

#if defined(CONFIG_BCM_FAP_LAYER2)
    if(flowInfo_p->type == FAP4KE_PKT_FT_L2)
    {
        fap4kePkt_l2TupleFilters_t *filters_p = &flowInfo_p->l2.tuple.filters;
        fap4kePkt_l2TupleActions_t *actions_p = &flowInfo_p->l2.tuple.actions;
        char filters[256];
        char actions[256];
        int index;
        uint32 hashIx;

        index = 0;
        if(flowInfo_p->l2.tuple.filters.ctrl.v0_pbits)
        {
            index += snprintf(&filters[index], 256, "v0_pbits ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.v0_dei)
        {
            index += snprintf(&filters[index], 256, "v0_dei ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.v0_vid)
        {
            index += snprintf(&filters[index], 256, "v0_vid ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.v0_etherType)
        {
            index += snprintf(&filters[index], 256, "v0_etherType ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.v1_pbits)
        {
            index += snprintf(&filters[index], 256, "v1_pbits ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.v1_dei)
        {
            index += snprintf(&filters[index], 256, "v1_dei ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.v1_vid)
        {
            index += snprintf(&filters[index], 256, "v1_vid ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.v1_etherType)
        {
            index += snprintf(&filters[index], 256, "v1_etherType ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.etherType)
        {
            index += snprintf(&filters[index], 256, "etherType ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.ipProtocol)
        {
            index += snprintf(&filters[index], 256, "ipProtocol ");
        }
        if(flowInfo_p->l2.tuple.filters.ctrl.tos)
        {
            index += snprintf(&filters[index], 256, "tos ");
        }
        filters[index ? index-1 : 0] = '\0';

        index = 0;
        if(actions_p->ctrl.v0_tag)
        {
            index += snprintf(&actions[index], 256, "v0_tag ");
        }
        if(actions_p->ctrl.v0_tpid)
        {
            index += snprintf(&actions[index], 256, "v0_tpid ");
        }
        if(actions_p->ctrl.v0_tci)
        {
            index += snprintf(&actions[index], 256, "v0_tci ");
        }
        if(actions_p->ctrl.v1_tag)
        {
            index += snprintf(&actions[index], 256, "v1_tag ");
        }
        if(actions_p->ctrl.v1_tpid)
        {
            index += snprintf(&actions[index], 256, "v1_tpid ");
        }
        if(actions_p->ctrl.v1_tci)
        {
            index += snprintf(&actions[index], 256, "v1_tci ");
        }
        if(actions_p->ctrl.tos)
        {
            index += snprintf(&actions[index], 256, "tos ");
        }
        if(actions_p->ctrl.revIvl)
        {
            index += snprintf(&actions[index], 256, "revIvl ");
        }
        if(actions_p->ovrdLearningVid)
        {
            index += snprintf(&actions[index], 256, "ovrdLearningVid <%u> ", actions_p->learnVlanId);
        }
        actions[index ? index-1 : 0] = '\0';

        hashIx = tupleHashL2(flowInfo_p->source, flowInfo_p->dest);
        hashIxBins[hashIx]++;

        printk("FLOW <%02u> : FAP%u, Layer2, hashIx <%u>, nbrOfTags <%u>, drop <%u>\n"
               "            sphy <%s>, schannel <%u>, dphy <%s>, %s <0x%02X>, queue <%u>, mem <%s>\n"
               "            Filters: control <0x%04X> [%s]\n"
               "                     v0_tci <0x%04X>, v0_etherType <0x%04X>, "
               "v1_tci <0x%04X>, v1_etherType <0x%04X>\n"
               "                     etherType <0x%04X>, ipProto <0x%02X>, tos <0x%02X>\n"
               "            Actions: control <0x%02X> [%s], txAdjust <%d>, tos <0x%02X>\n"
               "                     v0_tpid    <0x%04X>, v1_tpid    <0x%04X>\n"
               "                     v0_tciMask <0x%04X>, v1_tciMask <0x%04X>\n"
               "                     v0_tci     <0x%04X>, v1_tci     <0x%04X>\n"
               "            hits <%u>, bytes <%u>, iq_prio <%u> ack_prio<%u>\n",
               flowId, fapIdx, hashIx, flowInfo_p->source.nbrOfTags, flowInfo_p->dest.drop,
               srcPhyType, flowInfo_p->source.channel, destPhyType, destChannelType,
               flowInfo_p->dest.channel, flowInfo_p->dest.queue, getFlowMemTypeString(fapIdx, flowId),
               flowInfo_p->l2.tuple.filters.ctrl.u16, filters,
               filters_p->vlan[0].tci.u16, filters_p->vlan[0].etherType,
               filters_p->vlan[1].tci.u16, filters_p->vlan[1].etherType,
               filters_p->misc.etherType, filters_p->misc.ipProtocol, filters_p->misc.tos,
               actions_p->ctrl.u8, actions, (int)flowInfo_p->txAdjust, actions_p->tos,
               actions_p->vlan[0].tag.tpid, actions_p->vlan[1].tag.tpid,
               actions_p->vlan[0].tciMask.u16, actions_p->vlan[1].tciMask.u16,
               actions_p->vlan[0].tag.tci.u16, actions_p->vlan[1].tag.tci.u16,
               stats_p->hits, stats_p->bytes,
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
               flowInfo_p->iq.prio,
#else
               (long unsigned)0, 
#endif
               flowInfo_p->source.tcp_pure_ack
            );
    }
    else /* IP flow */
#endif /* CONFIG_BCM_FAP_LAYER2 */
    {

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        if(flowInfo_p->type == FAP4KE_PKT_FT_IPV6)
        {
            char *type;
            fap4kePkt_ipv6Tuple_t *ipTuple_p = &flowInfo_p->ipv6.tuple;

            if(FAP4KE_PKT_IS_MCAST_IPV6(flowInfo_p->ipv6.tuple.ipDa6.u8[0]))
            {
                if (flowInfo_p->isSsm)
                {
                    type = "Mcast/ssm";
                }
                else
                {
                    type = "Mcast/asm";
                }
                printk("FLOW <%02u> : FAP%u, %s, isRouted <%u>, isIPv6 <%u>, drop <%u>, learn <%u>\n"
                       "            sphy <%s> schannel <%u> dphy <%s> %s <0x%02X>, mem <%s>\n"
                       "            nvlanTags <%u>, outerVlanId <0x%04X>, innerVlanId <0x%04X>\n"
                       "            src" IP6HEX "\n"
                       "            dst" IP6HEX "\n"
                       "            protocol <0x%02X>, mtu <%d>\n"
                       "            hits <%u>, bytes <%u>, iq_prio <%u>\n",
                       flowId, fapIdx, type, ipTuple_p->flags.isRouted, flowInfo_p->type == FAP4KE_PKT_FT_IPV6,
                       ipTuple_p->flags.drop, ipTuple_p->flags.learn,
                       (flowInfo_p->source.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->source.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : "ENET"),
                       flowInfo_p->source.channel,
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : "ENET"),
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "dchannel" : "dchannelMask",
                       fapMcast_getDestPortMask(flowHandle),
                       getFlowMemTypeString(fapIdx, flowId),
                       flowInfo_p->source.nbrOfTags, ipTuple_p->vlanId.outer, ipTuple_p->vlanId.inner,
                       IP6(ipTuple_p->ipSa6), 
                       IP6(ipTuple_p->ipDa6),
                       flowInfo_p->source.protocol,
                       flowInfo_p->fapMtu,
                       stats_p->hits, stats_p->bytes, 
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
                       flowInfo_p->iq.prio
#else
                       (long unsigned)0
#endif
                    );
            }
            else
            {
                type = "Unicast";

                printk("FLOW <%02u> : FAP%u, %s, isRouted <%u>, isIPv6 <%u>, drop <%u>, learn <%u>\n"
                       "            sphy <%s> schannel <%u> dphy <%s> %s <0x%02X>, mem <%s>\n"
                       "            src" IP6PHEX "\n"
                       "            dst" IP6PHEX "\n"
                       "            protocol <0x%02X>, mtu <%d>, inheritTos <%u>\n"
                       "            hits <%u>, bytes <%u>, iq_prio <%u>, ack_prio<%u>, destq <0x%02X>\n",
                       flowId, fapIdx, type, ipTuple_p->flags.isRouted, flowInfo_p->type == FAP4KE_PKT_FT_IPV6,
                       ipTuple_p->flags.drop, ipTuple_p->flags.learn,
                       (flowInfo_p->source.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->source.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : "ENET"),
                       flowInfo_p->source.channel,
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_WLAN) ? "WLAN" : "ENET")),
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "dchannel" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_WLAN) ? "wlMetadata" : "dchannelMask"),
                       ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_WLAN) ? (unsigned int)flowInfo_p->word : flowInfo_p->dest.channel),
                       getFlowMemTypeString(fapIdx, flowId),
                       IP6(ipTuple_p->ipSa6), ipTuple_p->sPort,
                       IP6(ipTuple_p->ipDa6), ipTuple_p->dPort,
                       flowInfo_p->source.protocol,
                       flowInfo_p->fapMtu,
                       flowInfo_p->inheritTos,
                       stats_p->hits, stats_p->bytes, 
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
                       flowInfo_p->iq.prio,
#else
                       (long unsigned)0,
#endif
                       flowInfo_p->source.tcp_pure_ack,
                       flowInfo_p->dest.queue);
            }
        }
        else
#endif
        {
            fap4kePkt_ipv4Tuple_t *ipTuple_p = &flowInfo_p->ipv4.tuple;

            if(FAP4KE_PKT_IS_MCAST_IPV4(ipTuple_p->ipDa4))
            {
                printk("FLOW <%02u> : FAP%u, %s, isRouted <%u>, isIPv6 <%u>, drop <%u>, learn <%u>\n"
                       "            sphy <%s> schannel <%u> dphy <%s> %s <0x%02X> mem <%s>\n"
                       "            nvlanTags <%u>, outerVlanId <0x%04X>, innerVlanId <0x%04X>\n"
                       "			src" IP4DDN ", dst" IP4DDN ", excludePort <%d> mtu %d\n"
                       "            hits <%u>, bytes <%u>, iq_prio <%u>\n",
                       flowId, fapIdx,
                       flowInfo_p->isSsm ? "SSM Multicast" : "ASM Multicast",
                       ipTuple_p->flags.isRouted, flowInfo_p->type == FAP4KE_PKT_FT_IPV6,
                       ipTuple_p->flags.drop, ipTuple_p->flags.learn,
                       (flowInfo_p->source.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->source.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : "ENET"),
                       flowInfo_p->source.channel,
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : "ENET"),
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "dchannel" : "dchannelMask",
                       fapMcast_getDestPortMask(flowHandle),
                       getFlowMemTypeString(fapIdx, flowId),
                       flowInfo_p->source.nbrOfTags, ipTuple_p->vlanId.outer, ipTuple_p->vlanId.inner,
                       IP4(ipTuple_p->ipSa4), IP4(ipTuple_p->ipDa4), ipTuple_p->excludeDestPort,
                       flowInfo_p->fapMtu,
                       stats_p->hits, stats_p->bytes, 
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
                       flowInfo_p->iq.prio
#else
                       (long unsigned)0
#endif
                     );
            }
            else
            {
                printk("FLOW <%02u> : FAP%u, Unicast, isRouted <%u>, isIPv6 <%u>\n"
                       "            sphy <%s> schannel <%u> dphy <%s> %s <0x%02X>, mem <%s>\n"
                       "            src" IP4PDDN ", dst" IP4PDDN ", protocol <0x%02X>, mtu <%d>, inheritTos <%u>\n"
                       "            hits <%u>, bytes <%u>, iq_prio <%u>, ack_prio<%u>, destq <0x%02X>\n",
                       flowId, fapIdx, ipTuple_p->flags.isRouted, flowInfo_p->type == FAP4KE_PKT_FT_IPV6,
                       (flowInfo_p->source.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->source.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : "ENET"),
                       flowInfo_p->source.channel,
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "XTM" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_GPON) ? "GPON" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_WLAN) ? "WLAN" : "ENET")),
                       (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ? "dchannel" : ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_WLAN) ? "wlMetadata" : "dchannelMask"),
                       ((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_WLAN) ? (unsigned int)flowInfo_p->word : flowInfo_p->dest.channel),
                       getFlowMemTypeString(fapIdx, flowId),
                       IP4(ipTuple_p->ipSa4), ipTuple_p->sPort,
                       IP4(ipTuple_p->ipDa4), ipTuple_p->dPort,
                       flowInfo_p->source.protocol,
                       flowInfo_p->fapMtu, 
                       flowInfo_p->inheritTos,
                       stats_p->hits, stats_p->bytes,
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
                       flowInfo_p->iq.prio,
#else
                       (long unsigned)0,
#endif
                       flowInfo_p->source.tcp_pure_ack,
                       flowInfo_p->dest.queue);
            }
        }
    }

    printk("\n");
}
#endif

fapRet fapPkt_printFlow(fapPkt_flowHandle_t flowHandle)
{
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;

    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Bad fapIdx <%d> for flowId <%d>", fapIdx, flowId);

        return FAP_ERROR;
    }

    if(flowId >= FAP4KE_PKT_MAX_FLOWS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flowId <%d>", flowId);

        return FAP_ERROR;
    }

#if 1
    fapPkt_dumpFlowHost(flowHandle);
#else
    {
        xmit2FapMsg_t fapMsg;

        fapMsg.flowCfg.flowId = flowId;
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_DBG_PRINT_FLOW, &fapMsg);
    }
#endif

    return FAP_SUCCESS;
}

void fapPkt_printAllFlows(int16 sourceChannel, int16 destChannel)
{
    uint32 fapIdx;
    fap4kePkt_flowId_t flowId;
    fapPkt_flowHandle_t flowHandle = {.u16 = 0};
    uint32 cummHits;
    uint32 maxHash;
    uint32 maxHashIx;
    uint32 hashIx;
    int flowCount;

    printk("Filters: sourceChannel <%d>, destChannel <%d>\n\n",
           sourceChannel, destChannel);

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        cummHits = 0;
        flowCount = 0;
        memset(hashIxBins, 0, FAP4KE_PKT_HASH_TABLE_SIZE);

        flowHandle.fapIdx = fapIdx;

        printk("==================== FAP%u Flows ====================\n\n", fapIdx);

        for(flowId=0; flowId<FAP4KE_PKT_MAX_FLOWS; ++flowId)
        {
            FAP_PROTO_LOCK();
            if(flowAlloc_g[fapIdx][flowId].isHostValid)
            {
                fap4kePkt_flowInfo_t *flowInfo_p = &pHostFlowInfoPool(fapIdx)[flowId];
                uint8 destChannelComp = (flowInfo_p->dest.phy == FAP4KE_PKT_PHY_XTM) ?
                    destChannel : (1 << destChannel);

                if((sourceChannel != -1 && flowInfo_p->source.channel != sourceChannel) ||
                   (destChannel != -1 && flowInfo_p->dest.channel != destChannelComp))
                {
                    FAP_PROTO_UNLOCK();
                    continue;
                }

                flowHandle.flowId = flowId;

                fapPkt_printFlow(flowHandle);

                flowCount++;
                cummHits += getHostStats(fapIdx, flowId)->hits;
            }
            FAP_PROTO_UNLOCK();
            schedule();
        }

        maxHashIx = 0;
        maxHash = 0;
        for(hashIx=0; hashIx<FAP4KE_PKT_HASH_TABLE_SIZE; ++hashIx)
        {
            if(hashIxBins[hashIx] > maxHash)
            {
                maxHashIx = hashIx;
                maxHash = hashIxBins[hashIx];
            }
        }

        printk(FAP_IDX_FMT "Total Flows         : <%u>\n", fapIdx, flowCount);
        printk(FAP_IDX_FMT "Cumulative Packets  : <%u>\n", fapIdx, cummHits);
#if defined(CONFIG_BCM_FAP_LAYER2)
        printk(FAP_IDX_FMT "Hash Collisions     : max <%u>, hashIx <%u>\n",
               fapIdx, maxHash, maxHashIx);
#endif
        printk("\n");
    }

    printk("==================== Done ====================\n\n");
}

int fapPkt_deactivateAll(void)
{
    uint32 fapIdx;
    fap4kePkt_flowId_t flowId;
    fapPkt_flowHandle_t flowHandle;
    int flowCount = 0;

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        flowHandle.fapIdx = fapIdx;

        for(flowId=0; flowId<FAP4KE_PKT_MAX_FLOWS; ++flowId)
        {
            if(flowAlloc_g[fapIdx][flowId].isAlloc)
            {
                flowHandle.flowId = flowId;

                /* Deactivate this flow */
                if(fapPkt_deactivate(flowHandle) == FAP_ERROR)
                {
                    return FAP_ERROR;
                }

                update_Fapdeactivates();
                flowCount++;
            }
        }
    }

    return flowCount;
}

fap4kePkt_flowStats_t *fapPkt_getFlowStats(fapPkt_flowHandle_t flowHandle)
{
    uint32 fapIdx = flowHandle.fapIdx;
    fap4kePkt_flowId_t flowId = flowHandle.flowId;

    if(!isValidFapIdx(fapIdx))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Bad fapIdx <%d> for flowId <%d>", fapIdx, flowId);

        return NULL;
    }
    
    if(flowId >= FAP4KE_PKT_MAX_FLOWS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid flowId <%d>", flowId);

        return NULL;
    }

    if(!flowAlloc_g[fapIdx][flowId].isAlloc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Inactive flowId <%d>", flowId);

        return NULL;
    }

    return getHostStats(fapIdx, flowId);
}

#if defined(CC_FAP_ENET_STATS)
void fapEnetStats_interrupts(void)
{
    fapEnet_stats_g.interrupts++;
}

void fapEnetStats_dump(void)
{
    uint32 fapIdx;
    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        printk("\nFAP%u:\n", fapIdx);
        printk("contextCount <%d> contextFull <%d> dqmRxFull <%d>\n",
               fapEnet_stats_g.contextCount,
               fapEnet_stats_g.contextFull,
               fapEnet_stats_g.dqmRxFull);

        printk("rxPackets <%d> rxHighWm <%d> txPackets <%d> txHighWm <%d> interrupts <%d>\n",
               fapEnet_stats_g.rxPackets,
               pHostPsmGbl(fapIdx)->stats.rxHighWm,
               fapEnet_stats_g.txPackets,
               txHighWm,
               fapEnet_stats_g.interrupts);

        printk("4keRx rxTotal <%u> rxCount <%u> rxHighW <%u> rxDropped <%u> rxAssignedMin <%u> txFreeMin <%u>\n",
               pHostPsmGbl(fapIdx)->stats.rxTotal,
               pHostPsmGbl(fapIdx)->stats.rxCount,
               pHostPsmGbl(fapIdx)->stats.rxHighWm,
               pHostPsmGbl(fapIdx)->stats.rxDropped,
               pHostPsmGbl(fapIdx)->stats.rxAssignedBdsMin,
               pHostPsmGbl(fapIdx)->stats.txFreeBdsMin);
        printk("HostRx rxTotal <%u> rxCount <%u> rxHighW <%u> budget <%u>\n",
               pHostPsmGbl(fapIdx)->stats.Q7rxTotal,
               pHostPsmGbl(fapIdx)->stats.Q7rxCount,
               pHostPsmGbl(fapIdx)->stats.Q7rxHighWm,
               pHostPsmGbl(fapIdx)->stats.Q7budget);
        printk("4keRxFree rxTotal <%u> rxCount <%u> rxHighW <%u> budget <%d>\n",
               pHostPsmGbl(fapIdx)->stats.Q12rxTotal,
               pHostPsmGbl(fapIdx)->stats.Q12rxCount,
               pHostPsmGbl(fapIdx)->stats.Q12rxHighWm,
               DQM_HOST2FAP_ETH_FREE_RXBUF_BUDGET);
        printk("4keTx txTotal <%u> txCount <%u> txHighW <%u> budget <%d>\n",
               pHostPsmGbl(fapIdx)->stats.Q11txTotal,
               pHostPsmGbl(fapIdx)->stats.Q11txCount,
               pHostPsmGbl(fapIdx)->stats.Q11txHighWm,
               DQM_HOST2FAP_ETH_XMIT_BUDGET);
        printk("HostTxFree txTotal <%u> txCount <%u> txHighW <%u> budget <%d>\n",
               pHostPsmGbl(fapIdx)->stats.Q13txTotal,
               pHostPsmGbl(fapIdx)->stats.Q13txCount,
               pHostPsmGbl(fapIdx)->stats.Q13txHighWm,
               DQM_FAP2HOST_ETH_FREE_TXBUF_BUDGET);
    }
}

EXPORT_SYMBOL(fapEnetStats_interrupts);
EXPORT_SYMBOL(fapEnetStats_dump);
#endif

fapRet __init fapPkt_construct(void)
{
    memset(flowAlloc_g, 0, sizeof(fapPkt_flowAlloc_t));

#if defined(CC_FAP_ENET_STATS)
    memset(&fapEnet_stats_g, 0, sizeof(fapEnet_stats_t));
#endif

    return FAP_SUCCESS;
}

#if defined(CONFIG_BCM_FAP_LAYER2)

/*****************************************************
 * L2 APIs
 *****************************************************/

void fapPkt_setFloodingMask(uint8 channel, uint8 mask, int drop)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    fapMsg.floodingMask.u32 = 0;
    fapMsg.floodingMask.channel = channel;
    fapMsg.floodingMask.mask = mask;
    fapMsg.floodingMask.drop = (drop) ? 1 : 0;

    for(fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_SET_FLOODING_MASK, &fapMsg);
    }
}

void fapPkt_arlPrint(void)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_ARL_PRINT, &fapMsg);

        mdelay(500);
    }

#if defined(CC_FAP_MANAGE_HW_ARL)
    fapPkt_hwArlDump();
#endif
}

void fapPkt_arlFlush(uint8 channelMask)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.arlEntry.info.channelMask = channelMask;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_ARL_FLUSH, &fapMsg);
        mdelay(100);
    }
}

#if defined(CC_FAP_MANAGE_HW_ARL)

//#define CC_FAP_PKT_ARL_DEBUG

/* Switch ARL access */

typedef struct{
    int enable;
    uint8 learnPortMask;
} fapPkt_hwArlConfig_t;

static fapPkt_hwArlConfig_t fapPkt_hwArlConfig_g = { .enable=0,
                                                     .learnPortMask=0xFF };

uint16 fapPkt_hwArlRead(fap4keArl_tableEntryKey_t *arlKey_p,
                        fap4keArl_tableEntryKey_t *rsltArlKey_p)
{
#if defined(CC_FAP_PKT_ARL_DEBUG)
    uint32 retryCount = 1000000;
#endif

    ETHSW_AVTABLE_REG->macAddrIndexHigh = arlKey_p->macAddrHigh;
    ETHSW_AVTABLE_REG->macAddrIndexLow = arlKey_p->macAddrLow;
    ETHSW_AVTABLE_REG->vlanIdIndex = arlKey_p->vlanId & ARL_VID_INDEX_MASK;
    ETHSW_AVTABLE_REG->arlTableControl = ARL_CTRL_START_DONE | ARL_CTRL_READ;

    /* wait until ARL read has completed */
#if defined(CC_FAP_PKT_ARL_DEBUG)
    while((ETHSW_AVTABLE_REG->arlTableControl & ARL_CTRL_START_DONE) && --retryCount);
    if(retryCount == 0)
    {
        printk("*** hwArlRead Timeout! ***\n");
    }
    printk("Retries : %u\n", 1000000 - retryCount);
#else
    while(ETHSW_AVTABLE_REG->arlTableControl & ARL_CTRL_START_DONE);
#endif

    if(rsltArlKey_p != NULL)
    {
        uint32 arlMacVidEntryLow = ETHSW_AVTABLE_REG->arlMacVidEntryLow;
        uint32 arlMacVidEntryHigh = ETHSW_AVTABLE_REG->arlMacVidEntryHigh;

        rsltArlKey_p->macAddrHigh0 = arlMacVidEntryHigh & ARL_MAC_VID_ENTRY_HIGH_MAC_MASK;
        rsltArlKey_p->macAddrHigh1 = arlMacVidEntryLow >> 16;
        rsltArlKey_p->macAddrLow   = arlMacVidEntryLow & 0x0000FFFF;
        rsltArlKey_p->vlanId = ((arlMacVidEntryHigh & ARL_MAC_VID_ENTRY_HIGH_VID_MASK)
                                >> ARL_MAC_VID_ENTRY_HIGH_VID_SHFT);
    }

    return ETHSW_AVTABLE_REG->arlDataEntry;
}

/*
 * Return Value:
 * Overwrite mode Enabled
 *    0: N/A
 *    1: Write operation succeeded
 * Overwrite mode Disabled
 *    0: Write operation failed (collision detected)
 *    1: Write operation succeeded (no collision)
 */
int fapPkt_hwArlWrite(fap4keArl_tableEntryKey_t *arlKey_p, uint32 port, int overwrite)
{
#if defined(CC_FAP_PKT_ARL_DEBUG)
    uint32 retryCount = 1000000;
#endif
    fap4keArl_tableEntryKey_t rsltArlKey;
    uint16 arlDataEntry;

    arlDataEntry = fapPkt_hwArlRead(arlKey_p, &rsltArlKey);

    if(arlDataEntry & ARL_DATA_ENTRY_VALID)
    {
        if(!overwrite)
        {
            if(arlKey_p->macAddrHigh != rsltArlKey.macAddrHigh ||
               arlKey_p->macAddrLowVlanId != rsltArlKey.macAddrLowVlanId)
            {
                return 0;
            }
        }
    }

    arlDataEntry = (ARL_DATA_ENTRY_VALID | ARL_DATA_ENTRY_STATIC |
                    (port & ARL_DATA_ENTRY_PORT_MASK));

    ETHSW_AVTABLE_REG->arlDataEntry = arlDataEntry;

    ETHSW_AVTABLE_REG->arlMacVidEntryLow = ((uint32)(arlKey_p->macAddrHigh1 << 16) |
                                            (uint32)(arlKey_p->macAddrLow));
    ETHSW_AVTABLE_REG->arlMacVidEntryHigh = ((uint32)(arlKey_p->vlanId << ARL_MAC_VID_ENTRY_HIGH_VID_SHFT) |
                                             (uint32)(arlKey_p->macAddrHigh0));

    ETHSW_AVTABLE_REG->arlTableControl = ARL_CTRL_START_DONE | ARL_CTRL_WRITE;

    /* wait until ARL read has completed */
#if defined(CC_FAP_PKT_ARL_DEBUG)
    while((ETHSW_AVTABLE_REG->arlTableControl & ARL_CTRL_START_DONE) && --retryCount);
    if(retryCount == 0)
    {
        printk("*** hwArlWrite Timeout! ***\n");
    }
    printk("Retries : %u\n", 1000000 - retryCount);
#else
    while(ETHSW_AVTABLE_REG->arlTableControl & ARL_CTRL_START_DONE);
#endif

    /* Write operation succeeded */
    return 1;
}

int fapPkt_hwArlDelete(fap4keArl_tableEntryKey_t *arlKey_p)
{
#if defined(CC_FAP_PKT_ARL_DEBUG)
    uint32 retryCount = 1000000;
#endif
    fap4keArl_tableEntryKey_t rsltArlKey;
    uint16 arlDataEntry;

    arlDataEntry = fapPkt_hwArlRead(arlKey_p, &rsltArlKey);

    if(arlDataEntry & ARL_DATA_ENTRY_VALID)
    {
        if(arlKey_p->macAddrHigh != rsltArlKey.macAddrHigh ||
           arlKey_p->macAddrLowVlanId != rsltArlKey.macAddrLowVlanId)
        {
            return 0;
        }

        ETHSW_AVTABLE_REG->arlDataEntry = 0;
        ETHSW_AVTABLE_REG->arlMacVidEntryLow = 0;
        ETHSW_AVTABLE_REG->arlMacVidEntryHigh = 0;

        ETHSW_AVTABLE_REG->arlTableControl = ARL_CTRL_START_DONE | ARL_CTRL_WRITE;

        /* wait until ARL read has completed */
#if defined(CC_FAP_PKT_ARL_DEBUG)
        while((ETHSW_AVTABLE_REG->arlTableControl & ARL_CTRL_START_DONE) && --retryCount);
        if(retryCount == 0)
        {
            printk("*** hwArlWrite Timeout! ***\n");
        }
        printk("Retries : %u\n", 1000000 - retryCount);
#else
        while(ETHSW_AVTABLE_REG->arlTableControl & ARL_CTRL_START_DONE);
#endif
    }

    /* Write operation succeeded */
    return 1;
}

void fapPkt_hwArlDump(void)
{
    int isValid;
    uint32 macVidResultLow;
    uint32 macVidResultHigh;
    uint16 dataResult;
    int count = 0;

    printk("HW ARL\n");

    printk("Learning Enable Port Mask: 0x%02X\n", fapPkt_hwArlConfig_g.learnPortMask);

    /* start search */
    ETHSW_AVTABLE_REG->arlSearchControl = ARL_SEARCH_CTRL_START_DONE;

    while(1)
    {
        /* wait until search completes */
        while(ETHSW_AVTABLE_REG->arlSearchControl == ARL_SEARCH_CTRL_START_DONE);

        /* check search result */
        isValid = ETHSW_AVTABLE_REG->arlSearchControl & ARL_SEARCH_CTRL_VALID;

        if(isValid)
        {
            count++;

            macVidResultLow = ETHSW_AVTABLE_REG->arlSearchMacVidResultLow;
            macVidResultHigh = ETHSW_AVTABLE_REG->arlSearchMacVidResultHigh;
            /* reading this register causes the serach to continue */
            dataResult = ETHSW_AVTABLE_REG->arlSearchDataResult;

            printk("MAC <%04lX%08lX>, vlanId <%u>, data <0x%04X> "
                   ": <%s>, <%s>, priority <%u>, port <%u/0x%03X>\n",
                   (macVidResultHigh & ARL_SEARCH_MAC_VID_RESULT_MAC_MASK),
                   macVidResultLow,
                   ((macVidResultHigh & ARL_SEARCH_MAC_VID_RESULT_VID_MASK) >>
                    ARL_SEARCH_MAC_VID_RESULT_VID_SHFT),
                   dataResult,
                   (dataResult & ARL_SEARCH_DATA_RESULT_STATIC) ? "Static" : "Learned",
                   (dataResult & ARL_SEARCH_DATA_RESULT_AGE) ? "Aged" : "Not Aged",
                   (dataResult & ARL_SEARCH_DATA_RESULT_PRIO_MASK) >> ARL_SEARCH_DATA_RESULT_PRIO_SHFT,
                   (dataResult & ARL_SEARCH_DATA_RESULT_PORT_MASK) >> ARL_SEARCH_DATA_RESULT_PORT_SHFT,
                   (dataResult & ARL_SEARCH_DATA_RESULT_PORT_MASK) >> ARL_SEARCH_DATA_RESULT_PORT_SHFT);
        }
        else
        {
            break;
        }
    }

    printk("found <%u> entries.\n\n", count);

#if 0
    /* Test code ! */
    {
        fap4keArl_tableEntryKey_t arlKey;
        fap4keArl_tableEntryKey_t rsltArlKey;
        uint16 arlDataEntry;

        arlKey.macAddrHigh = 0x000ACD1C;
        arlKey.macAddrLow = 0x9A90;
        arlKey.vlanId = 0;
        arlDataEntry = hwArlRead(&arlKey, &rsltArlKey);

        printk("hwArlRead Test: arlDataEntry 0x%04X, MAC %08X%04X, VID %u\n\n",
                        arlDataEntry, rsltArlKey.macAddrHigh, rsltArlKey.macAddrLow, rsltArlKey.vlanId);

        arlKey.macAddrHigh = 0x02101888;
        arlKey.macAddrLow = 0x8401;
        arlKey.vlanId = 0;
        arlDataEntry = hwArlRead(&arlKey, &rsltArlKey);

        printk("hwArlRead Test: arlDataEntry 0x%04X, MAC %08X%04X, VID %u\n\n",
               arlDataEntry, rsltArlKey.macAddrHigh, rsltArlKey.macAddrLow, rsltArlKey.vlanId);

        arlKey.macAddrHigh = 0x12345678;
        arlKey.macAddrLow = 0x9ABC;
        arlKey.vlanId = 0;

        hwArlWrite(&arlKey, 2);
    }
#endif
}

int fapPkt_hwArlUpdate(hostMsgGroups_t msgType, fapMsg_arlEntry_t *arlEntry_p)
{
    fap4keArl_tableEntryKey_t arlKey;

    if(!fapPkt_hwArlConfig_g.enable)
    {
        return 1;
    }

    /* Check if HW learning is enabled for the port */

    if(!(arlEntry_p->info.channelMask & fapPkt_hwArlConfig_g.learnPortMask))
    {
        BCM_LOG_INFO(BCM_LOG_ID_FAP, "HW ARL Learning on port mask 0x%02X is disabled",
                     arlEntry_p->info.channelMask);
        return 0;
    }

    arlKey.macAddrHigh = arlEntry_p->key.macAddrHigh;
    arlKey.macAddrLow = arlEntry_p->key.macAddrLow;

    /* In the Switch ARL we need to use the original vlanId, not the reverse */
    if(arlEntry_p->info.revIvl)
    {
        arlKey.vlanId = arlEntry_p->info.vlanId;
    }
    else
    {
        arlKey.vlanId = arlEntry_p->key.vlanId;
    }

    if(msgType == HOST_MSG_ARL_ADD)
    {
        /* New FAP ARL Entry */
        uint32 port;

        for(port=0; port<FAP4KE_PKT_MAX_SRC_PORTS; ++port)
        {
            if(arlEntry_p->info.channelMask & (1 << port))
            {
                break;
            }
        }

        /* Try to add entry to the Switch ARL */

        if(!fapPkt_hwArlWrite(&arlKey, port, 0))
        {
            BCM_LOG_INFO(BCM_LOG_ID_FAP, "Could not fapPkt_hwArlWrite");

            return 0;
        }

        BCM_LOG_INFO(BCM_LOG_ID_FAP, "Added HW ARL Entry");
    }
    else /* HOST_MSG_ARL_REMOVE */
    {
        if(!fapPkt_hwArlDelete(&arlKey))
        {
            BCM_LOG_INFO(BCM_LOG_ID_FAP, "Could not fapPkt_hwArlDelete");

            return 0;
        }

        BCM_LOG_INFO(BCM_LOG_ID_FAP, "Removed HW ARL Entry");
    }

    return 1;
}

void fapPkt_hwArlConfig(int enable, uint8 learnPortMask)
{
    fapPkt_hwArlConfig_g.enable = enable;
    fapPkt_hwArlConfig_g.learnPortMask = learnPortMask;
}

#endif /* CC_FAP_MANAGE_HW_ARL */

#endif /* CONFIG_BCM_FAP_LAYER2 */
