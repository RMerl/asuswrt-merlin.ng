
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

/*
 *******************************************************************************
 * File Name  : fap_tm.c
 *
 * Description: This file contains the FAP Traffic Manager configuration API.
 *
 * The FAP Driver maintains two sets of TM configuration parameters: One for the
 * AUTO mode, and another for the MANUAL mode. The AUTO mode settings are
 * managed by the Ethernet driver based on the auto-negotiated PHY rates.
 * The MANUAL settings should be used by the user to configure the FAP TM.
 *
 * The mode can be set dynamically. Changing the mode does not apply the
 * corresponding settings into the FAP, it simply selects the current mode.
 * The settings corresponding to the current mode will take effect only when
 * explicitly applied to the FAP(s). This allows the caller to have a complete
 * configuration before activating it.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/bcm_log.h>
#include "fap4ke_memory.h"
#include "fap4ke_msg.h"
#include "fap.h"
#include "fap_tm.h"
#include "bcmenet.h"

#if defined(CC_FAP4KE_TM)

//#define CC_FAP_TM_DEBUG

#if defined(CC_FAP_TM_DEBUG)
#define fapTm_debug(fmt, arg...) printk(CLRm "%s.%u: " fmt CLRnl, __FUNCTION__, __LINE__, ##arg)
#else
#define fapTm_debug(fmt, arg...)
#endif

#define __trace(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_FAP, fmt, ##arg)

typedef struct {
    int valid;
    int kbps;
    int mbs;
    uint32 tokens;
    uint32 bucketSize;
} fapTm_shaper_t;

typedef struct {
    int dropProb;
    int minThreshold;
    int maxThreshold;
} fapTm_queueProfile_t;

typedef struct {
    uint32 alg;
    int queueProfileIdLo;
    int queueProfileIdHi;
    uint32_t priorityMask[2];
} fapTm_queueDropAlg_t;

#define FAP_TM_QUEUE_MAX FAP4KE_TM_WAN_QUEUE_MAX

#define FAP_TM_SCHEDULER_INDEX_INVALID -1

#define FAP_TM_PORT_IS_ENABLED(_port_p)                                 \
    ( (_port_p)->enable[(_port_p)->mode] &&                             \
      (_port_p)->schedulerIndex != FAP_TM_SCHEDULER_INDEX_INVALID )

#define FAP_TM_QUEUE_PROFILE_MASK_NUM ((FAP4KE_TM_QUEUE_PROFILE_MAX & 0x1f) ? \
    ((FAP4KE_TM_QUEUE_PROFILE_MAX >> 5) + 1) : (FAP4KE_TM_QUEUE_PROFILE_MAX >> 5))

typedef struct {
    int weight;
    int configured;
    fapTm_shaper_t shaper[FAP4KE_TM_SHAPER_TYPE_TOTAL];
} fapTm_queue_t;

typedef struct {
    fapTm_shaper_t shaper;
    fapTm_shapingType_t shapingType;
    fapTm_queue_t queue[FAP_TM_QUEUE_MAX];
    fap4keTm_arbiterType_t arbiterType;
    int arbiterArg;
} fapTm_scheduler_t;

typedef struct {
    int enable[FAP_TM_MODE_MAX];
    fapTm_mode_t mode;
    fapTm_portType_t type;
    int pauseEnable;
    int schedulerIndex;
    fapTm_scheduler_t scheduler[FAP_TM_MODE_MAX];
    fapTm_queueDropAlg_t queueDropAlg[FAP_TM_QUEUE_MAX];
} fapTm_port_t;

typedef struct {
    int masterEnable;
    uint32 schedulerAllocMask;
    fapTm_port_t port[FAP4KE_TM_MAX_PORTS];
    fapTm_queueProfile_t queueProfile[FAP4KE_TM_QUEUE_PROFILE_MAX];
    uint32 queueProfileMask[FAP_TM_QUEUE_PROFILE_MASK_NUM];
    fapTm_queueDropAlg_t xtmChnlDropAlg[XTM_TX_CHANNELS_MAX];
} fapTm_ctrl_t;

static fapTm_ctrl_t fapTmCtrl_g;

/*******************************************************************************
 *
 * Private Functions
 *
 *******************************************************************************/

static inline char *__modeToString(fapTm_mode_t mode)
{
    return (mode == FAP_TM_MODE_AUTO) ? "AUTO" : "MANUAL";
}

static int __schedulerAllocEntry(uint8 port, int schedulerIndex)
{
    uint32 schedulerIndexMask = (1 << schedulerIndex);

    if(!(fapTmCtrl_g.schedulerAllocMask & schedulerIndexMask))
    {
        fapTmCtrl_g.schedulerAllocMask |= schedulerIndexMask;

        /* Configure the FAP(s) */
        {
            xmit2FapMsg_t fapMsg;
            uint32 fapIdx;

            memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

            fapMsg.tm.port = port;
            fapMsg.tm.schedulerIndex = schedulerIndex;

            for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
            {
                fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_MAP_PORT_TO_SCHED, &fapMsg);
            }
        }

        /* Save state */
        fapTmCtrl_g.port[port].schedulerIndex = schedulerIndex;

        return 1;
    }

    return 0;
}

static int __schedulerAlloc(uint8 port, fapTm_portType_t portType)
{
    if(portType == FAP_TM_PORT_TYPE_WAN)
    {
        if(__schedulerAllocEntry(port, FAP4KE_TM_WAN_SCHEDULER_IDX))
        {
            return FAP4KE_TM_WAN_SCHEDULER_IDX;
        }
    }
    else /* LAN */
    {
        int schedulerIndex;

        for(schedulerIndex=0;
            schedulerIndex<FAP4KE_TM_LAN_SCHEDULER_MAX;
            ++schedulerIndex)
        {
            if(__schedulerAllocEntry(port, schedulerIndex))
            {
                return schedulerIndex;
            }
        }
    }

    return FAP_ERROR;
}

static void __schedulerFree(uint8 port)
{
    fapTmCtrl_g.schedulerAllocMask &= ~(1 << fapTmCtrl_g.port[port].schedulerIndex);

    fapTmCtrl_g.port[port].schedulerIndex = FAP_TM_SCHEDULER_INDEX_INVALID;
}

void fapTm_setFlowInfo(fap4kePkt_flowInfo_t *flowInfo_p, uint32 virtDestPortMask)
{
    int virtDestPort = fap4keTm_firstBitSet(virtDestPortMask);

#if defined(CONFIG_BCM_EXT_SWITCH)
#if defined(CONFIG_BCM963268)
    if(IsExternalSwitchPort(virtDestPort))
#endif
    {
        /* External Switch Port */
        flowInfo_p->extSwTagLen = sizeof(uint32_t);
    }
#endif /* CONFIG_BCM_EXT_SWITCH */

    flowInfo_p->virtDestPort = virtDestPort;
    flowInfo_p->virtDestPortMask = virtDestPortMask;

//    printk("TM: virtDestPort %d, virtDestPortMask 0x%08X, extSwTagLen %d\n",
//           flowInfo_p->virtDestPort, flowInfo_p->virtDestPortMask, flowInfo_p->extSwTagLen);
}

static void __queueApply(uint8 port, uint8 queue, uint8 shaperType,
                         uint16 tokens, uint16 bucketSize, uint8 weight)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.tm.port = port;
    fapMsg.tm.queue = queue;
    fapMsg.tm.shaperType = shaperType;
    fapMsg.tm.tokens = tokens;
    fapMsg.tm.bucketSize = bucketSize;
    fapMsg.tm.weight = weight;

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_QUEUE_CONFIG, &fapMsg);
    }
}

static int __applyQueueRatios(uint8 port)
{
    fapTm_port_t *port_p = &fapTmCtrl_g.port[port];
    int nbrOfQueues = (port_p->schedulerIndex == FAP4KE_TM_WAN_SCHEDULER_IDX) ?
        FAP4KE_TM_WAN_QUEUE_MAX : FAP4KE_TM_LAN_QUEUE_MAX;
    fapTm_scheduler_t *scheduler_p = &port_p->scheduler[port_p->mode];
    int queueIndex;
    int totalWeight = 0;

    for(queueIndex=0; queueIndex<nbrOfQueues; ++queueIndex)
    {
        fapTm_queue_t *queue_p = &scheduler_p->queue[queueIndex];

        totalWeight += queue_p->weight;
    }

    if(totalWeight == 0)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP,
                      "Port[%u] ratios have not been configured", port);

        return FAP_ERROR;
    }

    for(queueIndex=0; queueIndex<nbrOfQueues; ++queueIndex)
    {
        fapTm_queue_t *queue_p = &scheduler_p->queue[queueIndex];
        fapTm_shaper_t *shaper_p = &queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MIN];
        int shaperType;

        /* Min Shaper */

        shaper_p->valid = 1;
        shaper_p->kbps = (scheduler_p->shaper.kbps * queue_p->weight) / totalWeight;
        shaper_p->mbs = scheduler_p->shaper.mbs;
        shaper_p->tokens = fap4keTm_kbpsToTokens(shaper_p->kbps);
        shaper_p->bucketSize = fap4keTm_mbsToBucketSize(shaper_p->tokens + shaper_p->mbs);

        if(shaper_p->bucketSize > FAP_TM_BUCKET_SIZE_MAX)
        {
            shaper_p->bucketSize = FAP_TM_BUCKET_SIZE_MAX;
            shaper_p->mbs = FAP_TM_BUCKET_SIZE_MAX - shaper_p->tokens;
        }

        /* Max Shaper */

        shaper_p = &queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MAX];

        /* Use the port's parameters for the max shaper */

        memcpy(shaper_p, &scheduler_p->shaper, sizeof(fapTm_shaper_t));

        /* Apply settings to the FAPs */

        for(shaperType=0; shaperType<FAP4KE_TM_SHAPER_TYPE_TOTAL; ++shaperType)
        {
            fapTm_shaper_t *shaper_p = &queue_p->shaper[shaperType];

            __queueApply(port, queueIndex, shaperType,
                         shaper_p->tokens, shaper_p->bucketSize,
                         queue_p->weight);
        }
    }

    return FAP_SUCCESS;
}

static int __applyQueueRates(uint8 port)
{
    fapTm_port_t *port_p = &fapTmCtrl_g.port[port];
    int nbrOfQueues = (port_p->schedulerIndex == FAP4KE_TM_WAN_SCHEDULER_IDX) ?
        FAP4KE_TM_WAN_QUEUE_MAX : FAP4KE_TM_LAN_QUEUE_MAX;
    fapTm_scheduler_t *scheduler_p = &port_p->scheduler[port_p->mode];
    int queueIndex;

    for(queueIndex=0; queueIndex<nbrOfQueues; ++queueIndex)
    {
        fapTm_queue_t *queue_p = &scheduler_p->queue[queueIndex];
        fapTm_shaper_t *minShaper_p = &queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MIN];
        fapTm_shaper_t *maxShaper_p = &queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MAX];
        int shaperType;

        if(scheduler_p->arbiterType == FAP4KE_TM_ARBITER_TYPE_WRR &&
           queue_p->weight == 0)
        {
            /* Weight must be bigger than zero */

            queue_p->weight = 1;
        }

        if(scheduler_p->arbiterType == FAP4KE_TM_ARBITER_TYPE_SP_WRR &&
           queueIndex < scheduler_p->arbiterArg)
        {
            /* This is a WRR queue in the SP+WRR combo, so disable the min shaper,
               and use the port's parameters for the max shaper */

            memset(minShaper_p, 0, sizeof(fapTm_shaper_t));

            minShaper_p->valid = 1;

            memcpy(maxShaper_p, &scheduler_p->shaper, sizeof(fapTm_shaper_t));

            /* Weight must be bigger than zero */

            if(queue_p->weight == 0)
            {
                queue_p->weight = 1;
            }
        }
        else
        {
            uint32 maxShaperTokens;

            if(!minShaper_p->valid || !maxShaper_p->valid)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_FAP,
                              "Port[%u].Queue[%u] shapers have not been configured",
                              port, queueIndex);
                continue;
            }

            /* Adjust Max Rate Shaper */

            maxShaperTokens = fap4keTm_kbpsToTokens(maxShaper_p->kbps);

            if(maxShaperTokens > minShaper_p->tokens)
            {
                maxShaper_p->tokens = maxShaperTokens - minShaper_p->tokens;
            }
            else
            {
                maxShaper_p->tokens = 0;
            }

            if(maxShaper_p->tokens)
            {
                maxShaper_p->bucketSize = fap4keTm_mbsToBucketSize(maxShaper_p->tokens + maxShaper_p->mbs);
            }
            else
            {
                maxShaper_p->bucketSize = 0;
            }
        }

        /* Apply settings to the FAPs */

        for(shaperType=0; shaperType<FAP4KE_TM_SHAPER_TYPE_TOTAL; ++shaperType)
        {
            fapTm_shaper_t *shaper_p = &queue_p->shaper[shaperType];

            __queueApply(port, queueIndex, shaperType,
                         shaper_p->tokens, shaper_p->bucketSize,
                         queue_p->weight);
        }
    }

    return FAP_SUCCESS;
}

static char *shapingTypeStr(fapTm_shapingType_t shapingType)
{
    switch(shapingType)
    {
        case FAP_TM_SHAPING_TYPE_DISABLED:
            return "DISABLED";

        case FAP_TM_SHAPING_TYPE_RATE:
            return "RATE";

        case FAP_TM_SHAPING_TYPE_RATIO:
            return "RATIO";

        default:
            return "ERROR";
    }
}

static int __applyQueueProfile(int queueProfileId)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.tm.queueProfileId = queueProfileId;
    fapMsg.tm.dropProb = (uint8)fapTmCtrl_g.queueProfile[queueProfileId].dropProb;
    fapMsg.tm.minThreshold = (uint16)fapTmCtrl_g.queueProfile[queueProfileId].minThreshold;
    fapMsg.tm.maxThreshold = (uint16)fapTmCtrl_g.queueProfile[queueProfileId].maxThreshold;

    for(fapIdx = 0; fapIdx < NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_QUEUE_PROFILE_CONFIG, &fapMsg);
    }

    return FAP_SUCCESS;
}

static int __applyQueueDropAlg(uint8 port)
{
    fapTm_port_t *port_p = &fapTmCtrl_g.port[port];
    int nbrOfQueues = (port_p->type == FAP_TM_PORT_TYPE_WAN) ?
        FAP4KE_TM_WAN_QUEUE_MAX : FAP4KE_TM_LAN_QUEUE_MAX;
    int queueIndex;

    for(queueIndex=0; queueIndex<nbrOfQueues; ++queueIndex)
    {
        fapTm_queueDropAlg_t *queue_p = &port_p->queueDropAlg[queueIndex];
        xmit2FapMsg_t fapMsg;
        uint32 fapIdx;

        memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

        fapMsg.tm.port = port;
        fapMsg.tm.queue = queueIndex;
        fapMsg.tm.dropAlg = (uint8)queue_p->alg;
        fapMsg.tm.queueProfileIdLo = (uint16)queue_p->queueProfileIdLo;
        fapMsg.tm.queueProfileIdHi = (uint16)queue_p->queueProfileIdHi;

        for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
        {
            fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_QUEUE_DROPALG_CONFIG, &fapMsg);
        }
    }

    return FAP_SUCCESS;
}

static char *dropAlgTypeStr(fapTm_dropAlg_t dropAlgType)
{
    switch(dropAlgType)
    {
        case FAP_TM_DROP_ALG_DT:
            return "DROPTAIL";

        case FAP_TM_DROP_ALG_RED:
            return "RED";

        case FAP_TM_DROP_ALG_WRED:
            return "WRED";

        default:
            return "ERROR";
    }
}

/*******************************************************************************
 *
 * User API
 *
 *******************************************************************************/

/*******************************************************************************
 *
 * Function: fapTm_masterConfig
 *
 * Globally enables or disables the FAP TM function. Disabling the FAP TM does
 * not cause the existing settings to be lost.
 *
 *******************************************************************************/
void fapTm_masterConfig(int enable)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    fapTm_debug("enable %d", enable);

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.tm.masterEnable = enable;

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_ENABLE, &fapMsg);
    }

    fapTmCtrl_g.masterEnable = enable;
}

/*******************************************************************************
 *
 * Function: fapTm_portConfig
 *
 * Configures the shaper bit rate (kbps) and Maximum Burst Size (mbs) of the
 * specified port/mode. The mbs setting specifies the amount of bytes that are
 * allowed to be transmitted above the shaper bit rate when the input bit rate
 * exceeds the shaper bit rate. Once the mbs is achieved, no bursts will be
 * allowed until the input bit rate becomes lower than the shaper bit rate.
 *
 * This API does not apply the specified settings into the FAP. This can only
 * be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_portConfig(uint8 port, fapTm_mode_t mode, int kbps, int mbs,
                     fapTm_shapingType_t shapingType)
{
    fapTm_debug("port %u, mode %u, kbps %d, mbs %d, shapingType %d",
                port, mode, kbps, mbs, shapingType);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    if(kbps > FAP_TM_KBPS_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Kbps (req %d, max %d)", kbps, FAP_TM_KBPS_MAX);

        return FAP_ERROR;
    }

    if(shapingType >= FAP_TM_SHAPING_TYPE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid shapingType (%d)", shapingType);

        return FAP_ERROR;
    }

    {
        fapTm_port_t *port_p = &fapTmCtrl_g.port[port];
        fapTm_scheduler_t *scheduler_p = &port_p->scheduler[mode];
        fapTm_shaper_t *shaper_p;
        int queueIndex;

        scheduler_p->shapingType = shapingType;

        /* Configure port shaper */

        shaper_p = &scheduler_p->shaper;

        shaper_p->valid = 1;
        shaper_p->kbps = kbps;
        shaper_p->mbs = mbs;
        shaper_p->tokens = fap4keTm_kbpsToTokens(kbps);
        shaper_p->bucketSize = fap4keTm_mbsToBucketSize(shaper_p->tokens + mbs);

        if(shaper_p->bucketSize > FAP_TM_BUCKET_SIZE_MAX)
        {
            shaper_p->bucketSize = FAP_TM_BUCKET_SIZE_MAX;
            shaper_p->mbs = FAP_TM_BUCKET_SIZE_MAX - shaper_p->tokens;

            printk("Max MBS for %uKbps is %u (requested %u)\n", kbps, shaper_p->mbs, mbs);
        }

        /* Configure queue shapers to the default values, if not configured yet */

        {
            for(queueIndex=0; queueIndex<FAP_TM_QUEUE_MAX; ++queueIndex)
            {
                shaper_p = &scheduler_p->queue[queueIndex].shaper[FAP4KE_TM_SHAPER_TYPE_MAX];

                if(!shaper_p->valid || shapingType == FAP_TM_SHAPING_TYPE_DISABLED)
                {
                    shaper_p->valid = 1;
                    shaper_p->kbps = scheduler_p->shaper.kbps;
                    shaper_p->mbs = scheduler_p->shaper.mbs;
                    shaper_p->tokens = scheduler_p->shaper.tokens;
                    shaper_p->bucketSize = scheduler_p->shaper.bucketSize;
                }

                shaper_p = &scheduler_p->queue[queueIndex].shaper[FAP4KE_TM_SHAPER_TYPE_MIN];

                if(!shaper_p->valid || shapingType == FAP_TM_SHAPING_TYPE_DISABLED)
                {
                    shaper_p->valid = 1;
                    shaper_p->kbps = 0;
                    shaper_p->mbs = 0;
                    shaper_p->tokens = 0;
                    shaper_p->bucketSize = 0;
                }
            }
        }
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_getPortConfig
 *
 * Returns the shaper bit rate (kbps), Maximum Burst Size (mbs), and Shaping
 * Typeof the specified port/mode.
 *
 *******************************************************************************/
int fapTm_getPortConfig(uint8 port, fapTm_mode_t mode, int *kbps_p, int *mbs_p,
                        fapTm_shapingType_t *shapingType_p)
{
    fapTm_debug("port %u, mode %u", port, mode);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    {
        fapTm_port_t *port_p = &fapTmCtrl_g.port[port];
        fapTm_scheduler_t *scheduler_p = &port_p->scheduler[mode];

        *kbps_p = scheduler_p->shaper.kbps;
        *mbs_p = scheduler_p->shaper.mbs;
        *shapingType_p = scheduler_p->shapingType;
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_getPortCapability
 *
 * Returns the scheduling type, max queues, max strict priority queues, 
 * port shaper, and queue shaper support of the specified port.
 *
 *******************************************************************************/
int fapTm_getPortCapability(uint8 port, uint32_t *schedType_p, int *maxQueues_p,
                            int *maxSpQueues_p, uint8_t *portShaper_p, uint8_t *queueShaper_p)                        
{
    fapTm_port_t *port_p = NULL;
    
    fapTm_debug("port %u", port);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);
        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    if(port_p->type == FAP_TM_PORT_TYPE_WAN)
    {
        *maxQueues_p = FAP_TM_WAN_QUEUE_MAX;
        *maxSpQueues_p = FAP_TM_WAN_QUEUE_MAX;
    }
    else
    {
        *maxQueues_p = FAP_TM_LAN_QUEUE_MAX;
        *maxSpQueues_p = FAP_TM_LAN_QUEUE_MAX;
    }

    *schedType_p = 0;
    *schedType_p |= FAP_TM_SP_CAPABLE;
    *schedType_p |= FAP_TM_WRR_CAPABLE;
    *schedType_p |= FAP_TM_WFQ_CAPABLE;
    *schedType_p |= FAP_TM_SP_WRR_CAPABLE;
   
    *portShaper_p = TRUE;
    *queueShaper_p = TRUE;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_queueConfig
 *
 * Configures the shaper bit rate (kbps) and Maximum Burst Size (mbs) of the
 * specified port/queue/mode/shaperType. The shaper type can be either the
 * minimum rate or maximum rate, as defined in fap4keTm_shaperType_t.
 *
 * Each queue supports a minimum rate shaper and a maximum rate shaper.
 * The minimum queue rates have precedence over the maximum queue rates among
 * queues of a given port. The minimum queue rates are guaranteed as long as the
 * sum of the minimum queue rates is smaller than or equal to the port's rate.
 * When the sum of the minimum queue rates is smaller than the port's rate, the
 * remaining bandwith (after satisfying the minimum rate of each queue) is
 * distributed amongst the queues based on the arbitration scheme in use (SP,
 * WRR, etc), and is capped at the maximum rate of each queue.
 *
 * The mbs setting specifies the amount of bytes that are allowed to be
 * transmitted above the shaper bit rate when the input bit rate
 * exceeds the shaper bit rate. Once the mbs is achieved, no bursts will be
 * allowed until the input bit rate becomes lower than the shaper bit rate.
 *
 * This API does not apply the specified settings into the FAP. This can only
 * be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_queueConfig(uint8 port, fapTm_mode_t mode, uint8 queue,
                      uint8 shaperType, int kbps, int mbs)
{
    fapTm_debug("port %u, mode %u, queue %u, shaperType %u, kbps %d, mbs %d",
                port, mode, queue, shaperType, kbps, mbs);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    if(shaperType >= FAP4KE_TM_SHAPER_TYPE_TOTAL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM shaperType <%d>", shaperType);

        return FAP_ERROR;
    }

    if(kbps > FAP_TM_KBPS_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Kbps (req %d, max %d)", kbps, FAP_TM_KBPS_MAX);

        return FAP_ERROR;
    }

    {
        fapTm_scheduler_t *scheduler_p = &fapTmCtrl_g.port[port].scheduler[mode];

        if(scheduler_p->shapingType == FAP_TM_SHAPING_TYPE_RATE)
        {
            fapTm_shaper_t *shaper_p = &scheduler_p->queue[queue].shaper[shaperType];

            shaper_p->valid = 1;
            shaper_p->kbps = kbps;
            shaper_p->mbs = mbs;
            shaper_p->tokens = fap4keTm_kbpsToTokens(kbps);
            shaper_p->bucketSize = fap4keTm_mbsToBucketSize(shaper_p->tokens + mbs);

            if(shaper_p->bucketSize > FAP_TM_BUCKET_SIZE_MAX)
            {
                shaper_p->bucketSize = FAP_TM_BUCKET_SIZE_MAX;
                shaper_p->mbs = FAP_TM_BUCKET_SIZE_MAX - shaper_p->tokens;

                printk("Max MBS for %uKbps is %u (requested %u)\n", kbps, shaper_p->mbs, mbs);
            }
        }
        else
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Queue Shaping is disabled: port %u, mode %u",
                          port, mode);

            return FAP_ERROR;
        }
        /* Indicate this queue is configured, clear when unconfigured. */
        scheduler_p->queue[queue].configured = 1;
    }
    

    return FAP_SUCCESS;
}


/*******************************************************************************
 *
 * Function: fapTm_queueUnconfig
 *
 * Clear the configured bit of the specified queue.
 * This API does not need to apply. The configured bit is used to detect if a 
 * queue is configured by FAP TM API or not.
 *
 *******************************************************************************/
int fapTm_queueUnconfig(uint8 port, fapTm_mode_t mode, uint8 queue)
{
    fapTm_debug("port %u, mode %u, queue %u", port, mode, queue);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    {
        fapTm_scheduler_t *scheduler_p = &fapTmCtrl_g.port[port].scheduler[mode];

        /* Indicate this queue is un-configured. */
        scheduler_p->queue[queue].configured = 0;
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_getQueueConfig
 *
 * Returns the max/min shaping bit rate (kbps), Maximum Burst Size (mbs),
 * weight, and queue size of the specified port, mode, and queue. 
 *
 *******************************************************************************/
int fapTm_getQueueConfig(uint8 port, fapTm_mode_t mode, uint8 queue,
                         int *kbps_p, int *minKbps_p, int *mbs_p, int *weight_p, int *qsize_p)
{
    fapTm_debug("port %u, mode %u, queue %u",port, mode, queue);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    {
        fapTm_scheduler_t *scheduler_p = &fapTmCtrl_g.port[port].scheduler[mode];
        fapTm_queue_t *queue_p = &scheduler_p->queue[queue];
        fapTm_shaper_t *minShaper_p = &queue_p->shaper[FAP_IOCTL_TM_SHAPER_TYPE_MIN];
        fapTm_shaper_t *maxShaper_p = &queue_p->shaper[FAP_IOCTL_TM_SHAPER_TYPE_MAX];

        *weight_p = queue_p->weight;
        *kbps_p = maxShaper_p->kbps;
        *mbs_p = maxShaper_p->mbs;
        *minKbps_p = minShaper_p->kbps;

        if (queue_p->configured)
        {
            /* queue is configured, report real queue size */
            /* TODO: Should qsize be retrieved from bpm thresh? */
            *qsize_p = 512;
        }
        else
        {
            /* queue is not configured, report qsize is zero */
            *qsize_p = 0;
        }
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_allocQueueProfileId
 *
 * Obtain a free queue profile index.  We try to reserve Queue Profile Id#0
 *
 *******************************************************************************/
int fapTm_allocQueueProfileId(int *queueProfileId_p)
{
    int i;

    for (i = 1; i < FAP4KE_TM_QUEUE_PROFILE_MAX; i++) {
        if ((fapTmCtrl_g.queueProfileMask[i >> 5] & (0x1 << (i & 0x1f))) == 0) {
            *queueProfileId_p = i;
            fapTmCtrl_g.queueProfileMask[i >> 5] |= 0x1 << (i & 0x1f);
            return FAP_SUCCESS;
        }
    }
    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Run out of Queue Profile");
    return FAP_ERROR;
}

/*******************************************************************************
 *
 * Function: fapTm_freeQueueProfileId
 *
 * free a queue profile index
 *
 *******************************************************************************/
int fapTm_freeQueueProfileId(int queueProfileId)
{
    if((queueProfileId >= FAP4KE_TM_QUEUE_PROFILE_MAX) || (queueProfileId == 0))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Queue Profile ID %u", queueProfileId);

        return FAP_ERROR;
    }

    fapTmCtrl_g.queueProfileMask[queueProfileId >> 5] &=
            ~(0x1 << (queueProfileId & 0x1f));
    return FAP_SUCCESS;
}
/*******************************************************************************
 *
 * Function: fapTm_findQueueProfileId
 * Find a queue profile ID with specified criterion.
 * If the profile cannot be found, it will create a new one.
 *
 *******************************************************************************/

int fapTm_findQueueProfileId(int dropProbability,
                             int minThreshold,
                             int maxThreshold,
                             int* queueProfileId_p)
{
    BOOL found = FALSE;
    int redDropProbability, redMinThreshold, redMaxThreshold, i;
 
    /* When profile ID is not allocated before, find and re-use if profile setting is the same. */
    if(!*queueProfileId_p)
    {
        for (i = 0; i < FAP4KE_TM_QUEUE_PROFILE_MAX; i++)
        {
            if (fapTm_getQueueProfileConfig(i, &redDropProbability, &redMinThreshold, &redMaxThreshold))
            {
                BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_getQueueProfileConfig ERROR! queueProfileId=%d", i);
                return FAP_ERROR;
            }
            if((dropProbability == redDropProbability) &&
               (minThreshold == redMinThreshold) &&
               (maxThreshold == redMaxThreshold))
            {
                found = TRUE;
                *queueProfileId_p = i;
                break;
            }
        }
        /* If profile is not found, allocate a new one. */
        if(!found)   
        {
            if (fapTm_allocQueueProfileId(queueProfileId_p))
            {
                BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_allocQueueProfileId ERROR! Run out of Queue Profile.");
                return FAP_ERROR;
            }
        }     
    }
 
    /* Set queue drop profile */
    if (fapTm_queueProfileConfig(*queueProfileId_p, dropProbability, minThreshold, maxThreshold))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_queueProfileConfig ERROR! queueProfileId=%d", *queueProfileId_p);
        return FAP_ERROR;
    }
   
    return FAP_SUCCESS;

}  /* End of fapTm_findQueueProfileId() */

/*******************************************************************************
 *
 * Function: fapTm_queueProfileConfig
 *
 * Apply queue profile attribute to the given queue profile index.
 * If queueProfileId given is 0, it will automatically allocate a free Queue
 * Profile Index. (Also, queueProfileId#0 is reserved for default queue profile)
 *
 *******************************************************************************/
int fapTm_queueProfileConfig(int queueProfileId, int dropProbability, int minThreshold,
                             int maxThreshold)
{
    int ret;

    fapTm_debug("queueProfileId %d, dropProbability %d, minThreshold %d, "
                "maxThreshold %d\n", queueProfileId, dropProbability,
                minThreshold, maxThreshold);

    if(queueProfileId >= FAP4KE_TM_QUEUE_PROFILE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Queue Profile ID %u", queueProfileId);

        return FAP_ERROR;
    }

    if(queueProfileId == 0) {
        ret = fapTm_allocQueueProfileId(&queueProfileId);
        if (ret == FAP_ERROR)
            return ret;
    }

    if((dropProbability < 0) || (dropProbability > 100) || (minThreshold < 0) ||
       (maxThreshold < 0) || (maxThreshold <= minThreshold))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue Profile Parameter");
        return FAP_ERROR;
    }

    fapTmCtrl_g.queueProfile[queueProfileId].dropProb = dropProbability;
    fapTmCtrl_g.queueProfile[queueProfileId].minThreshold = minThreshold;
    fapTmCtrl_g.queueProfile[queueProfileId].maxThreshold = maxThreshold;
    fapTmCtrl_g.queueProfileMask[queueProfileId >> 5] |= 0x1 << (queueProfileId & 0x1f);

    return __applyQueueProfile(queueProfileId);
}

/*******************************************************************************
 *
 * Function: fapTm_getQueueProfileConfig
 *
 * Returns the queue profile attribute to the given queue profile index.
 * Assume those pointers are not NULL.
 *
 *******************************************************************************/
int fapTm_getQueueProfileConfig(int queueProfileId, int *dropProbability_p,
                             int *minThreshold_p, int *maxThreshold_p)
{
    fapTm_debug("queueProfileId %d\n", queueProfileId);

    if(queueProfileId >= FAP4KE_TM_QUEUE_PROFILE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Queue Profile ID %u", queueProfileId);

        return FAP_ERROR;
    }

    *dropProbability_p = fapTmCtrl_g.queueProfile[queueProfileId].dropProb;
    *minThreshold_p = fapTmCtrl_g.queueProfile[queueProfileId].minThreshold;
    *maxThreshold_p = fapTmCtrl_g.queueProfile[queueProfileId].maxThreshold;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_queueDropAlgConfig
 *
 * Configure the queue dropping algorithm to the given queue.
 * When dropAlgorithm is FAP_TM_DROP_ALG_DT, it ignores the values from both
 *     queue profile indices
 * When dropAlgorithm is FAP_TM_DROP_ALG_RED, it will apply the queueProfileIdLo
 * When dropAlgorithm is FAP_TM_DROP_ALG_WRED, it will use both queue profile
 *     indices
 *
 *******************************************************************************/
int fapTm_queueDropAlgConfig(uint8 port, uint8 queue, fapTm_dropAlg_t dropAlgorithm,
                            int queueProfileIdLo, int queueProfileIdHi,
                            uint32_t priorityMask0, uint32_t priorityMask1)
{
    fapTm_queueDropAlg_t *queueDropAlg_p = &fapTmCtrl_g.port[port].queueDropAlg[queue];

    fapTm_debug("port %u, queue %u, dropAlgorithm %u, queueProfileIdLo %d, "
                "queueProfileIdHi %d, priorityMask0 0x%x, priorityMask1 0x%x\n",
                port, queue, dropAlgorithm, queueProfileIdLo, queueProfileIdHi,
                priorityMask0, priorityMask1);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    if(dropAlgorithm >= FAP_TM_DROP_ALG_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Drop Algorithm <%u>", dropAlgorithm);

        return FAP_ERROR;
    }

    if(((dropAlgorithm == FAP_TM_DROP_ALG_RED) || (dropAlgorithm == FAP_TM_DROP_ALG_WRED)) &&
       (queueProfileIdLo >= FAP4KE_TM_QUEUE_PROFILE_MAX))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue Profile ID <%u>",
                      queueProfileIdLo);

        return FAP_ERROR;
    }

    if((dropAlgorithm == FAP_TM_DROP_ALG_WRED) &&
       (queueProfileIdHi >= FAP4KE_TM_QUEUE_PROFILE_MAX))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue Profile ID <%u>",
                      queueProfileIdHi);

        return FAP_ERROR;
    }

    queueDropAlg_p->alg = dropAlgorithm;
    queueDropAlg_p->queueProfileIdLo = queueProfileIdLo;
    queueDropAlg_p->queueProfileIdHi = queueProfileIdHi;
    queueDropAlg_p->priorityMask[0] = priorityMask0;
    queueDropAlg_p->priorityMask[1] = priorityMask1;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_queueDropAlgConfigExt
 *
 * Configure the queue dropping algorithm to the given queue in another way.
 * When dropAlgorithm is FAP_TM_DROP_ALG_DT, it ignores the values from Lo and Hi drop settings.
 * When dropAlgorithm is FAP_TM_DROP_ALG_RED, it will use only Lo drop settings.
 * When dropAlgorithm is FAP_TM_DROP_ALG_WRED, it will use both Lo and Hi drop settings.
 *
 *******************************************************************************/
int fapTm_queueDropAlgConfigExt(uint8 port, uint8 queue, fapTm_dropAlg_t dropAlgorithm,
                                int dropProbabilityLo, int minThresholdLo, int maxThresholdLo,
                                int dropProbabilityHi, int minThresholdHi, int maxThresholdHi,
                                uint32_t priorityMask0, uint32_t priorityMask1)
{
    fapTm_dropAlg_t oldDropAlgorithm;
    fapTm_queueDropAlg_t *queueDropAlg_p = &fapTmCtrl_g.port[port].queueDropAlg[queue];
    int queueProfileIdLo, queueProfileIdHi;
    uint32_t old_priorityMask0, old_priorityMask1;

    fapTm_debug("port %u, queue %u, dropAlgorithm %u, "
                "dropProbabilityLo %d, minThresholdLo %d, maxThresholdLo %d, "
                "dropProbabilityHi %d, minThresholdHi %d, maxThresholdHi %d.\n",
                port, queue, dropAlgorithm,
                dropProbabilityLo, minThresholdLo, maxThresholdLo,
                dropProbabilityHi, minThresholdHi, maxThresholdHi);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    if(dropAlgorithm >= FAP_TM_DROP_ALG_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Drop Algorithm <%u>", dropAlgorithm);

        return FAP_ERROR;
    }

    if (fapTm_getQueueDropAlgConfig(port, queue, &oldDropAlgorithm,
                                    &queueProfileIdLo, &queueProfileIdHi,
                                    &old_priorityMask0, &old_priorityMask1))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_getQueueDropAlgConfig ERROR! port=%d queue=%d", port, queue);
        return FAP_ERROR;
    }

    /* Find a matched queue profile or create one. */
    if((dropAlgorithm == FAP_TM_DROP_ALG_RED) || (dropAlgorithm == FAP_TM_DROP_ALG_WRED))
    {
        if(fapTm_findQueueProfileId(dropProbabilityLo, minThresholdLo, maxThresholdLo, &queueProfileIdLo))
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_findQueueProfileId ERROR! dropProbabilityLo=%d minThresholdLo=%d maxThresholdLo=%d",
                          dropProbabilityLo, minThresholdLo, maxThresholdLo);
            return FAP_ERROR;
        }
    }
    if(dropAlgorithm == FAP_TM_DROP_ALG_WRED)
    {
        if(fapTm_findQueueProfileId(dropProbabilityHi, minThresholdHi, maxThresholdHi, &queueProfileIdHi))
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_findQueueProfileId ERROR! dropProbabilityHi=%d minThresholdHi=%d maxThresholdHi=%d",
                          dropProbabilityHi, minThresholdHi, maxThresholdHi);
            return FAP_ERROR;
        }
    }

    /* Set queue drop algorithm. */
    queueDropAlg_p->alg = dropAlgorithm;
    queueDropAlg_p->queueProfileIdLo = queueProfileIdLo;
    queueDropAlg_p->queueProfileIdHi = queueProfileIdHi;
    queueDropAlg_p->priorityMask[0] = priorityMask0;
    queueDropAlg_p->priorityMask[1] = priorityMask1;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_getQueueDropAlgConfig
 *
 * Return the queue drop algorithm applied to the given queue.
 * When dropAlgorithm is FAP_TM_DROP_ALG_DT, it ignores the values from both
 *     queue profile indices
 * When dropAlgorithm is FAP_TM_DROP_ALG_RED, it will apply the queueProfileIdLo
 * When dropAlgorithm is FAP_TM_DROP_ALG_WRED, it will use both queue profile
 *     indices
 *
 *******************************************************************************/
int fapTm_getQueueDropAlgConfig(uint8 port, uint8 queue, fapTm_dropAlg_t *dropAlgorithm_p,
                            int *queueProfileIdLo_p, int *queueProfileIdHi_p,
                            uint32_t *priorityMask0_p, uint32_t *priorityMask1_p)
{
    fapTm_debug("port %u, queue %u", port, queue);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    {
        fapTm_queueDropAlg_t *queueDropAlg_p = &fapTmCtrl_g.port[port].queueDropAlg[queue];

        *dropAlgorithm_p = queueDropAlg_p->alg;
        *queueProfileIdLo_p = queueDropAlg_p->queueProfileIdLo;
        *queueProfileIdHi_p = queueDropAlg_p->queueProfileIdHi;
        *priorityMask0_p = queueDropAlg_p->priorityMask[0];
        *priorityMask1_p = queueDropAlg_p->priorityMask[1];
    }

    return FAP_SUCCESS;
}

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
static int __applyXtmQueueDropAlgConfig(int chnl)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;
    fapTm_queueDropAlg_t *xtmChnlDropAlg_p = &fapTmCtrl_g.xtmChnlDropAlg[chnl];

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.xtmQueueDropAlg.channel = chnl;
    fapMsg.xtmQueueDropAlg.dropAlg = xtmChnlDropAlg_p->alg;
    fapMsg.xtmQueueDropAlg.queueProfileIdLo = xtmChnlDropAlg_p->queueProfileIdLo;
    fapMsg.xtmQueueDropAlg.queueProfileIdHi = xtmChnlDropAlg_p->queueProfileIdHi;

    for(fapIdx = 0; fapIdx < NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_XTM_QUEUE_DROPALG_CONFIG, &fapMsg);
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_XtmQueueDropAlgConfig
 *
 * Configure the queue dropping algorithm to the given XTM channel.
 * When dropAlgorithm is FAP_TM_DROP_ALG_DT, it ignores the values from both
 *     queue profile indices
 * When dropAlgorithm is FAP_TM_DROP_ALG_RED, it will apply the queueProfileIdLo
 * When dropAlgorithm is FAP_TM_DROP_ALG_WRED, it will use both queue profile
 *     indices
 *
 *******************************************************************************/
int fapTm_XtmQueueDropAlgConfig(uint8_t chnl, fapTm_dropAlg_t dropAlgorithm,
                                int queueProfileIdLo, int queueProfileIdHi,
                                uint32_t priorityMask0, uint32_t priorityMask1)
{
    fapTm_debug("channel %u, dropAlgorithm %u, queueProfileIdLo %d, "
                "queueProfileIdHi %d, priorityMask0 0x%x, priorityMask1 0x%x\n",
                channel, dropAlgorithm, queueProfileIdLo, queueProfileIdHi,
                priorityMask0, priorityMask1);

    if(chnl >= XTM_TX_CHANNELS_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid channel %u", chnl);

        return FAP_ERROR;
    }

    if(dropAlgorithm >= FAP_TM_DROP_ALG_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Drop Algorithm <%u>", dropAlgorithm);

        return FAP_ERROR;
    }

    if(((dropAlgorithm == FAP_TM_DROP_ALG_RED) || (dropAlgorithm == FAP_TM_DROP_ALG_WRED)) &&
       (queueProfileIdLo >= FAP4KE_TM_QUEUE_PROFILE_MAX))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue Profile ID <%u>",
                      queueProfileIdLo);

        return FAP_ERROR;
    }

    if((dropAlgorithm == FAP_TM_DROP_ALG_WRED) &&
       (queueProfileIdHi >= FAP4KE_TM_QUEUE_PROFILE_MAX))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue Profile ID <%u>",
                      queueProfileIdHi);

        return FAP_ERROR;
    }

    {
        fapTm_queueDropAlg_t *xtmChnlDropAlg_p = &fapTmCtrl_g.xtmChnlDropAlg[chnl];

        xtmChnlDropAlg_p->alg = dropAlgorithm;
        xtmChnlDropAlg_p->queueProfileIdLo = queueProfileIdLo;
        xtmChnlDropAlg_p->queueProfileIdHi = queueProfileIdHi;
        xtmChnlDropAlg_p->priorityMask[0] = priorityMask0;
        xtmChnlDropAlg_p->priorityMask[1] = priorityMask1;
    }

    return __applyXtmQueueDropAlgConfig(chnl);
}

/*******************************************************************************
 *
 * Function: fapTm_getXtmQueueDropAlgConfig
 *
 * Return the queue drop algorithm applied to the given XTM channel.
 * When dropAlgorithm is FAP_TM_DROP_ALG_DT, it ignores the values from both
 *     queue profile indices
 * When dropAlgorithm is FAP_TM_DROP_ALG_RED, it will apply the queueProfileIdLo
 * When dropAlgorithm is FAP_TM_DROP_ALG_WRED, it will use both queue profile
 *     indices
 *
 *******************************************************************************/
int fapTm_getXtmQueueDropAlgConfig(uint8_t chnl, fapTm_dropAlg_t *dropAlgorithm_p,
                            int *queueProfileIdLo_p, int *queueProfileIdHi_p,
                            uint32_t *priorityMask0_p, uint32_t *priorityMask1_p)
{
    fapTm_debug("channel %u", chnl);

    if(chnl >= XTM_TX_CHANNELS_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid channel %u", chnl);

        return FAP_ERROR;
    }

    {
        fapTm_queueDropAlg_t *xtmChnlDropAlg_p = &fapTmCtrl_g.xtmChnlDropAlg[chnl];

        *dropAlgorithm_p = xtmChnlDropAlg_p->alg;
        *queueProfileIdLo_p = xtmChnlDropAlg_p->queueProfileIdLo;
        *queueProfileIdHi_p = xtmChnlDropAlg_p->queueProfileIdHi;
        *priorityMask0_p = xtmChnlDropAlg_p->priorityMask[0];
        *priorityMask1_p = xtmChnlDropAlg_p->priorityMask[1];
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_XtmQueueDropAlgConfigExt
 *
 * Configure the queue dropping algorithm to the given XTM channel in another way.
 * When dropAlgorithm is FAP_TM_DROP_ALG_DT, it ignores the values from Lo and Hi drop settings.
 * When dropAlgorithm is FAP_TM_DROP_ALG_RED, it will use only Lo drop settings.
 * When dropAlgorithm is FAP_TM_DROP_ALG_WRED, it will use both Lo and Hi drop settings.
 *
 *******************************************************************************/
int fapTm_XtmQueueDropAlgConfigExt(uint8_t chnl, fapTm_dropAlg_t dropAlgorithm,
                                   int dropProbabilityLo, int minThresholdLo, int maxThresholdLo,
                                   int dropProbabilityHi, int minThresholdHi, int maxThresholdHi)
{
    fapTm_dropAlg_t oldDropAlgorithm;
    int queueProfileIdLo, queueProfileIdHi;
    uint32_t priorityMask0, priorityMask1;

    fapTm_debug("channel %u, dropAlgorithm %u, "
                "dropProbabilityLo %d, minThresholdLo %d, maxThresholdLo %d, "
                "dropProbabilityHi %d, minThresholdHi %d, maxThresholdHi %d.\n",
                chnl, dropAlgorithm,
                dropProbabilityLo, minThresholdLo, maxThresholdLo,
                dropProbabilityHi, minThresholdHi, maxThresholdHi);

    if(chnl >= XTM_TX_CHANNELS_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid channel %u", chnl);

        return FAP_ERROR;
    }

    if(dropAlgorithm >= FAP_TM_DROP_ALG_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Drop Algorithm <%u>", dropAlgorithm);

        return FAP_ERROR;
    }

    /* Read old config of queue and keep priorityMask0 and priorityMask1 unchanged. */
    if (fapTm_getXtmQueueDropAlgConfig(chnl, &oldDropAlgorithm,
                                       &queueProfileIdLo, &queueProfileIdHi,
                                       &priorityMask0, &priorityMask1))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_getXtmQueueDropAlgConfig ERROR! chnl=%d", chnl);
        return FAP_ERROR;
    }

    /* Find a matched queue profile or create one. */
    if((dropAlgorithm == FAP_TM_DROP_ALG_RED) || (dropAlgorithm == FAP_TM_DROP_ALG_WRED))
    {
        if(fapTm_findQueueProfileId(dropProbabilityLo, minThresholdLo, maxThresholdLo, &queueProfileIdLo))
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_findQueueProfileId ERROR! dropProbabilityLo=%d minThresholdLo=%d maxThresholdLo=%d",
                          dropProbabilityLo, minThresholdLo, maxThresholdLo);
            return FAP_ERROR;
        }
    }
    if(dropAlgorithm == FAP_TM_DROP_ALG_WRED)
    {
        if(fapTm_findQueueProfileId(dropProbabilityHi, minThresholdHi, maxThresholdHi, &queueProfileIdHi))
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "fapTm_findQueueProfileId ERROR! dropProbabilityHi=%d minThresholdHi=%d maxThresholdHi=%d",
                          dropProbabilityHi, minThresholdHi, maxThresholdHi);
            return FAP_ERROR;
        }
    }

    /* Set queue drop algorithm. */
    {
        fapTm_queueDropAlg_t *xtmChnlDropAlg_p = &fapTmCtrl_g.xtmChnlDropAlg[chnl];

        xtmChnlDropAlg_p->alg = dropAlgorithm;
        xtmChnlDropAlg_p->queueProfileIdLo = queueProfileIdLo;
        xtmChnlDropAlg_p->queueProfileIdHi = queueProfileIdHi;
        xtmChnlDropAlg_p->priorityMask[0] = priorityMask0;
        xtmChnlDropAlg_p->priorityMask[1] = priorityMask1;
    }

    return __applyXtmQueueDropAlgConfig(chnl);
}

#endif

/*******************************************************************************
 *
 * Function: fapTm_checkHighPrio
 *
 * Check the given TC value (0 to 63) is set to use high priority RED profile in
 * WRED scheme, if so, return 1.  If not, return 0
 *
 *******************************************************************************/
int fapTm_checkHighPrio(uint8 phy, uint8 port, uint8 queue, uint8 chnl, uint32 tc)
{
    fapTm_queueDropAlg_t *queueDropAlg_p;

    fapTm_debug("phy %u, port %u, queue %u, chnl %u", phy, port, queue, chnl);

    // TODO! are we going to support FAP4KE_PKT_PHY_WLAN?
    if((phy != FAP4KE_PKT_PHY_XTM) && (phy != FAP4KE_PKT_PHY_ENET) &&
       (phy != FAP4KE_PKT_PHY_ENET_EXT))
    {
        /* don't print the error log, it is confusing.  */
        //BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid phy %u, support only XTM, ENET, ENET_EXT", phy);

        return FAP_ERROR;
    }

    if((phy == FAP4KE_PKT_PHY_ENET) || (phy == FAP4KE_PKT_PHY_ENET_EXT))
    {
        if(port >= FAP4KE_TM_MAX_PORTS)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

            return FAP_ERROR;
        }

        if(queue >= FAP_TM_QUEUE_MAX)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

            return FAP_ERROR;
        }
        queueDropAlg_p = &fapTmCtrl_g.port[port].queueDropAlg[queue];
    }
    else /* if (phy == FAP4KE_PKT_PHY_XTM) */
    {
        if(chnl >= XTM_TX_CHANNELS_MAX)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid channel %u", chnl);

            return FAP_ERROR;
	}
        queueDropAlg_p = &fapTmCtrl_g.xtmChnlDropAlg[chnl];
    }

    if(tc >= FAP4KE_TM_TC_MAX)
    {
        return FAP_ERROR;
    }

    if(tc < 32)
    {
        if(queueDropAlg_p->priorityMask[0] & (0x1 << tc))
            return 1;
    }
    else /* if(tc < 63) */
    {
        if(queueDropAlg_p->priorityMask[1] & (0x1 << (tc - 32)))
            return 1;
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_checkSetHighPrio
 *
 * Check the given TC value (0 to 63) is set to use high priority RED profile in
 * WRED scheme, if so, reflect the high priority info in destQueue_p.
 *
 *******************************************************************************/
int fapTm_checkSetHighPrio(uint8 port, uint8 queue, uint32 tc, uint32 *destQueue_p)
{
    fapTm_debug("port %u, queue %u", port, queue);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    if(unlikely(destQueue_p == NULL))
    {
        return FAP_ERROR;
    }

    if(tc >= FAP4KE_TM_TC_MAX)
    {
        return FAP_ERROR;
    }

    {
        fapTm_queueDropAlg_t *queueDropAlg_p = &fapTmCtrl_g.port[port].queueDropAlg[queue];
        if(tc < 32)
        {
            if(queueDropAlg_p->priorityMask[0] & (0x1 << tc))
                *destQueue_p |= FAP4KE_TM_HIGH_PRIO_MASK;
        }
        else /* if(tc < 63) */
        {
            if(queueDropAlg_p->priorityMask[1] & (0x1 << (tc - 32)))
                *destQueue_p |= FAP4KE_TM_HIGH_PRIO_MASK;
        }
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_xtmCheckHighPrio
 *
 * Check the given TC value (0 to 63) is set to use high priority RED profile in
 * WRED scheme, if so, return 1.  If not, return 0
 *
 *******************************************************************************/
int fapTm_xtmCheckHighPrio(uint8 chnl, uint32 tc)
{
    return fapTm_checkHighPrio(FAP4KE_PKT_PHY_XTM, 0, 0, chnl, tc);
}

/*******************************************************************************
 *
 * Function: fapTm_getQueueStats
 *
 * Returns the transmitted packets, transmitted bytes, dropped packets,
 * and dropped bytes of the specified port, mode, and queue.
 *
 *******************************************************************************/
int fapTm_getQueueStats(uint8 port, fapTm_mode_t mode, uint8 queue,
                        uint32_t *txPackets_p, uint32_t *txBytes_p, uint32_t *droppedPackets_p, uint32_t *droppedBytes_p)
{
    fapTm_debug("port %u, mode %u, queue %u", port, mode, queue);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }
    
    /* TODO: Per queue stats is supported on 4ke, not on host FAP driver. */
    *txPackets_p = 0;
    *txBytes_p = 0;
    *droppedPackets_p = 0;
    *droppedBytes_p = 0;

    return FAP_SUCCESS;
}


/*******************************************************************************
 *
 * Function: fapTm_setQueueWeight
 *
 * Configures the given Queue's weight.
 *
 * This API does not apply the specified settings into the FAP. This can only
 * be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_setQueueWeight(uint8 port, fapTm_mode_t mode, uint8 queue, uint8 weight)
{
    fapTm_debug("port %u, mode %u, queue %u, weight %d",
                port, mode, queue, weight);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    if(queue >= FAP_TM_QUEUE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

        return FAP_ERROR;
    }

    if(weight == 0 || weight > FAP4KE_TM_QUEUE_WEIGHT_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Weight %d", weight);

        return FAP_ERROR;
    }

    {
        fapTm_scheduler_t *scheduler_p = &fapTmCtrl_g.port[port].scheduler[mode];
        fapTm_queue_t *queue_p = &scheduler_p->queue[queue];

        queue_p->weight = weight;
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_arbiterConfig
 *
 * Configures the Arbiter type of the given port to SP, WRR, SP+WRR, or WFQ.
 * arbiterArg is a generic argument that is passed to arbiters. Currently it is
 * only used in SP+WRR, where it indicates the lowest priority queue in the
 * SP Tier.
 *
 * This API does not apply the corresponding mode settings into the FAP. This
 * can only be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_arbiterConfig(uint8 port, fapTm_mode_t mode, uint8 arbiterType, uint8 arbiterArg)
{
    fapTm_debug("port %u, arbiterType %u, arbiterArg %u",
                port, arbiterType, arbiterArg);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    if(arbiterType >= FAP4KE_TM_ARBITER_TYPE_TOTAL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid arbiterType %u", arbiterType);

        return FAP_ERROR;
    }

    {
        fapTm_scheduler_t *scheduler_p = &fapTmCtrl_g.port[port].scheduler[mode];

        if(arbiterType == FAP4KE_TM_ARBITER_TYPE_WRR &&
           scheduler_p->shapingType != FAP_TM_SHAPING_TYPE_DISABLED)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP,
                          "Port %d: In WRR, queue shaping must be disabled", port);

            return FAP_ERROR;
        }

        scheduler_p->arbiterType = arbiterType;
        scheduler_p->arbiterArg = arbiterArg;
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_getArbiterConfig
 *
 * Returns the Arbiter type of the given port and mode.
 * arbiterArg is a generic argument that is passed to arbiters.
 * Currently it is only used in SP+WRR, where it indicates the lowest 
 * priority queue in the SP Tier.
 *
 *******************************************************************************/
int fapTm_getArbiterConfig(uint8 port, fapTm_mode_t mode, fap4keTm_arbiterType_t *arbiterType_p, int *arbiterArg_p)
{
    fapTm_debug("port %u, mode %u", port, mode);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    {
        fapTm_scheduler_t *scheduler_p = &fapTmCtrl_g.port[port].scheduler[mode];
        *arbiterType_p = scheduler_p->arbiterType;
        *arbiterArg_p = scheduler_p->arbiterArg;
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_setPortMode
 *
 * Sets the mode of the given port to AUTO or MANUAL.
 *
 * This API does not apply the corresponding mode settings into the FAP. This
 * can only be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_setPortMode(uint8 port, fapTm_mode_t mode)
{
    fapTm_port_t *port_p;

    fapTm_debug("port %u, mode %u", port, mode);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    port_p->mode = mode;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_getPortMode
 *
 * Returns the mode of the given port.
 *
 *******************************************************************************/
fapTm_mode_t fapTm_getPortMode(uint8 port)
{
    fapTm_port_t *port_p;

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    return port_p->mode;
}

/*******************************************************************************
 *
 * Function: fapTm_modeReset
 *
 * Resets the current configuration of the given port and mode.
 *
 * This API does not apply the corresponding mode settings into the FAP. This
 * can only be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_modeReset(uint8 port, fapTm_mode_t mode)
{
    fapTm_port_t *port_p;
    fapTm_scheduler_t *scheduler_p;

    fapTm_debug("port %u, mode %u", port, mode);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    scheduler_p = &port_p->scheduler[mode];

    memset(scheduler_p, 0, sizeof(fapTm_scheduler_t));

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_portType
 *
 * Sets the type of the given port to LAN or WAN.
 *
 * This API does not apply the corresponding mode settings into the FAP. This
 * can only be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_portType(uint8 port, fapTm_portType_t portType)
{
    fapTm_port_t *port_p;

    fapTm_debug("port %u, portType %u", port, portType);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(portType >= FAP_TM_PORT_TYPE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid portType %u", portType);

        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    port_p->type = portType;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_portEnable
 *
 * Enables or Disables the given port and mode.
 *
 * This API does not apply the corresponding mode settings into the FAP. This
 * can only be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_portEnable(uint8 port, fapTm_mode_t mode, int enable)
{
    fapTm_port_t *port_p;

    fapTm_debug("port %u, mode %u, enable %u", port, mode, enable);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    if(mode >= FAP_TM_MODE_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Mode <%d>", mode);

        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    port_p->enable[mode] = enable;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_pauseEnable
 *
 * Enables or Disables pause on a given port.
 *
 * This API does not apply the corresponding mode settings into the FAP. This
 * can only be done via the fapTm_apply API.
 *
 *******************************************************************************/
int fapTm_pauseEnable(uint8 port, int enable)
{
    fapTm_port_t *port_p;

    fapTm_debug("port %u, enable %d", port, enable);
    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);
        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];
    port_p->pauseEnable = enable;

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_apply
 *
 * Applies the settings corresponding to the current mode of the specified port
 * into the FAP.
 *
 * This API also allows enabling or disabling the specified port.
 *
 *******************************************************************************/
int fapTm_apply(uint8 port)
{
    fapTm_port_t *port_p;
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;
    int enable;

    fapTm_debug("port %u", port);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    enable = port_p->enable[port_p->mode];

    if(enable)
    {
        fapTm_scheduler_t *scheduler_p = &port_p->scheduler[port_p->mode];
        fapTm_shaper_t *shaper_p = &scheduler_p->shaper;

        if(!shaper_p->valid)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP,
                          "Cannot enable port %u, %s mode is not configured",
                          port, __modeToString(port_p->mode));

            return FAP_ERROR;
        }

        if(port_p->schedulerIndex != FAP_TM_SCHEDULER_INDEX_INVALID)
        {
            /* Free existing Scheduler */

            __schedulerFree(port);
        }

        /* Allocate a new scheduler */
        {
            int schedulerIndex = __schedulerAlloc(port, port_p->type);

            if(schedulerIndex == FAP_ERROR)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Could not __schedulerAlloc, port %u", port);

                return FAP_ERROR;
            }
        }

        fapMsg.tm.tokens = shaper_p->tokens;
        fapMsg.tm.bucketSize = shaper_p->bucketSize;
    }
    else /* disable */
    {
        if(port_p->schedulerIndex != FAP_TM_SCHEDULER_INDEX_INVALID)
        {
            /* Scheduler has been allocated, so free it */

            __schedulerFree(port);
        }
    }

    /* Configure the FAP(s) */

    fapMsg.tm.port = port;
    fapMsg.tm.enable = enable;

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_PORT_CONFIG, &fapMsg);
    }

    fapMsg.tm.port = port;
    fapMsg.tm.enable = port_p->pauseEnable;
    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_PAUSE_EN, &fapMsg);
    }

    if(enable)
    {
        fapTm_scheduler_t *scheduler_p = &port_p->scheduler[port_p->mode];

        if(scheduler_p->shapingType == FAP_TM_SHAPING_TYPE_RATIO)
        {
            if(__applyQueueRatios(port) == FAP_ERROR)
            {
                return FAP_ERROR;
            }
        }
        else /* Rate or Disabled */
        {
            if(__applyQueueRates(port) == FAP_ERROR)
            {
                return FAP_ERROR;
            }
        }

        if(__applyQueueDropAlg(port) == FAP_ERROR)
        {
            return FAP_ERROR;
        }

        /* Apply the arbiter configuration */
        memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

        fapMsg.tm.port = port;
        fapMsg.tm.arbiterType = scheduler_p->arbiterType;
        fapMsg.tm.arbiterArg = scheduler_p->arbiterArg;
        fapMsg.tm.qShaping = (scheduler_p->shapingType ==
                              FAP_TM_SHAPING_TYPE_DISABLED) ? 0 : 1;

        for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
        {
            fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_ARBITER_CONFIG, &fapMsg);
        }
    }

    return FAP_SUCCESS;
}

/*******************************************************************************
 *
 * Function: fapTm_mapTmQueueToSwQueue
 *
 * Maps a FAP TM queue to a HW Switch Queue, on a per-port basis.
 *
 * This API applies the configuration immediately to the FAP(s).
 *
 *******************************************************************************/
int fapTm_mapTmQueueToSwQueue(uint8 port, uint8 queue, uint8 swQueue)
{
    fapTm_port_t *port_p;

    fapTm_debug("port %u, queue %u, swQueue %u", port, queue, swQueue);

    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    port_p = &fapTmCtrl_g.port[port];

    if(FAP_TM_PORT_IS_ENABLED(port_p))
    {
        /* Port is enabled and scheduler has been allocated */

        int nbrOfQueues = (port_p->schedulerIndex == FAP4KE_TM_WAN_SCHEDULER_IDX) ?
            FAP4KE_TM_WAN_QUEUE_MAX : FAP4KE_TM_LAN_QUEUE_MAX;
        xmit2FapMsg_t fapMsg;
        uint32 fapIdx;

        if(queue >= nbrOfQueues)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid TM Queue <%d>", queue);

            return FAP_ERROR;
        }

        memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

        fapMsg.tm.port = port;
        fapMsg.tm.queue = queue;
        fapMsg.tm.swQueue = swQueue;

        for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
        {
            fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_MAP_TMQUEUE_TO_SWQUEUE, &fapMsg);
        }

        return FAP_SUCCESS;
    }
    else
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Port %u is disabled", port);

        return FAP_ERROR;
    }
}

/*******************************************************************************
 *
 * Function: fapTm_status
 *
 * Prints the status of all ENABLED ports.
 *
 *******************************************************************************/
void fapTm_status(void)
{
    int port;

    printk("\tFAP TM Status: %s\n\n", fapTmCtrl_g.masterEnable ? "ON" : "OFF");

    for(port=0; port<FAP4KE_TM_MAX_PORTS; ++port)
    {
        fapTm_port_t *port_p = &fapTmCtrl_g.port[port];

        if(port_p->schedulerIndex != FAP_TM_SCHEDULER_INDEX_INVALID)
        {
            fapTm_mode_t mode;
            int nbrOfQueues = (port_p->schedulerIndex == FAP4KE_TM_WAN_SCHEDULER_IDX) ?
                FAP4KE_TM_WAN_QUEUE_MAX : FAP4KE_TM_LAN_QUEUE_MAX;
            int queueIndex;

            printk("\tPort[%02u]: %s, %s, (schedulerIndex %d)\n",
                   port, __modeToString(port_p->mode),
                   (port_p->type == FAP_TM_PORT_TYPE_LAN) ? "LAN" : "WAN",
                   port_p->schedulerIndex);

            for(mode=0; mode<FAP_TM_MODE_MAX; ++mode)
            {
                fapTm_scheduler_t *scheduler_p = &port_p->scheduler[mode];
                fapTm_shaper_t *shaper_p = &scheduler_p->shaper;

                printk("\t\t  %6s: %s, %s (%d), Kbps %u (tokens %u), MBS %u (bucketSize %u), "
                       "qShaping %s\n",
                       __modeToString(mode), port_p->enable[mode] ? "ENABLED" : "DISABLED",
                       arbiterTypeName(scheduler_p->arbiterType), scheduler_p->arbiterArg,
                       shaper_p->kbps, shaper_p->tokens, shaper_p->mbs, shaper_p->bucketSize,
                       shapingTypeStr(scheduler_p->shapingType));

                for(queueIndex=0; queueIndex<nbrOfQueues; ++queueIndex)
                {
                    fapTm_queue_t *queue_p = &scheduler_p->queue[queueIndex];
                    int bps = (port_p->mode == mode) ? fapTm_getQueueRate(port, queueIndex) : 0;

                    printk("\t\t          Queue[%u]: MIN: Kbps %u (tokens %u), MBS %u (size %u), Weight: %u\n"
                           "\t\t                    MAX: Kbps %u (tokens %u), MBS %u (size %u)\n"
                           "\t\t                    Rate: %d Kbps (%d bps)\n",
                           queueIndex,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MIN].kbps,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MIN].tokens,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MIN].mbs,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MIN].bucketSize,
                           queue_p->weight,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MAX].kbps,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MAX].tokens,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MAX].mbs,
                           queue_p->shaper[FAP4KE_TM_SHAPER_TYPE_MAX].bucketSize,
                           bps/1000, bps);
                }
            }

            printk("\t\tQueue Drop Algorithm:\n");
            for(queueIndex=0; queueIndex<nbrOfQueues; ++queueIndex)
            {
                fapTm_queueDropAlg_t *queueDropAlg_p = &port_p->queueDropAlg[queueIndex];

                printk("\t\t          Queue[%u]: DropAlg", queueIndex);

                if(queueDropAlg_p->alg == FAP_TM_DROP_ALG_DT)
                {
                    printk("(%s)\n", dropAlgTypeStr(queueDropAlg_p->alg));
                }
                else if(queueDropAlg_p->alg == FAP_TM_DROP_ALG_RED)
                {
                    printk("(%s): queueProfileId %d\n",
                           dropAlgTypeStr(queueDropAlg_p->alg),
                           queueDropAlg_p->queueProfileIdLo);
                }
                else if(queueDropAlg_p->alg == FAP_TM_DROP_ALG_WRED)
                {
                    printk("(%s): queueProfileIdLo %d, queueProfileIdHi %d, "
                           "priorityMask0 0x%x, priorityMask1 0x%x\n",
                           dropAlgTypeStr(queueDropAlg_p->alg),
                           queueDropAlg_p->queueProfileIdLo,
                           queueDropAlg_p->queueProfileIdHi,
                           queueDropAlg_p->priorityMask[0],
                           queueDropAlg_p->priorityMask[1]);
                }
                else
                {
                    printk(" Unknown\n");
                }
            }

            printk("\n");
        }
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    {
        int chnl;

        printk("\tXTM Queue Drop Algorithm Info:\n");
        for(chnl = 0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
        {
            fapTm_queueDropAlg_t *queueDropAlg_p = &fapTmCtrl_g.xtmChnlDropAlg[chnl];

            printk("\t\tchannel[%u]: DropAlg", chnl);
            if(queueDropAlg_p->alg == FAP_TM_DROP_ALG_DT)
            {
                printk("(%s)\n", dropAlgTypeStr(queueDropAlg_p->alg));
            }
            else if(queueDropAlg_p->alg == FAP_TM_DROP_ALG_RED)
            {
                printk("(%s): queueProfileId %d\n",
                       dropAlgTypeStr(queueDropAlg_p->alg),
                       queueDropAlg_p->queueProfileIdLo);
            }
            else if(queueDropAlg_p->alg == FAP_TM_DROP_ALG_WRED)
            {
                printk("(%s): queueProfileIdLo %d, queueProfileIdHi %d, "
                       "priorityMask0 0x%x, priorityMask1 0x%x\n",
                       dropAlgTypeStr(queueDropAlg_p->alg),
                       queueDropAlg_p->queueProfileIdLo,
                       queueDropAlg_p->queueProfileIdHi,
                       queueDropAlg_p->priorityMask[0],
                       queueDropAlg_p->priorityMask[1]);
            }
            else
            {
                printk(" Unknown\n");
            }
        }
        printk("\n");
    }
#endif

    {
        int queueProfileId;

        printk("\tQueue Profile Info:\n");
        for(queueProfileId = 0; queueProfileId < FAP4KE_TM_QUEUE_PROFILE_MAX; queueProfileId++)
        {
            if(fapTmCtrl_g.queueProfileMask[queueProfileId >> 5] & (0x1 << (queueProfileId & 0x1f)))
            {
                printk("\t\tQueueProfileID#%d is enabled:\n\t\t\tdropProb %d, minThreshold %d, "
                       "maxThreshold %d\n", queueProfileId,
                       fapTmCtrl_g.queueProfile[queueProfileId].dropProb,
                       fapTmCtrl_g.queueProfile[queueProfileId].minThreshold,
                       fapTmCtrl_g.queueProfile[queueProfileId].maxThreshold);
            }
        }
    }
}

/*******************************************************************************
 *
 * Function: fapTm_stats
 *
 * Prints the statistics counters of all ENABLED ports.
 *
 *******************************************************************************/
void fapTm_stats(int port)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.tm.port = port;

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_STATS, &fapMsg);

        mdelay(200);
    }
}

/*******************************************************************************
 *
 * Function: fapTm_dumpMaps
 *
 * Prints the Port to Scheduler and Priority to Queue mappings.
 *
 *******************************************************************************/
void fapTm_dumpMaps(void)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_TM_MAP_DUMP, &fapMsg);

        mdelay(200);
    }
}

/*******************************************************************************
 *
 * Function: fapTm_getQueueRate
 *
 * Returns the current bandwidth of the specified queue in bits/s. In case of
 * error, FAP_ERROR is returned.
 *
 *******************************************************************************/
int fapTm_getQueueRate(uint8 port, uint8 queue)
{
    if(port >= FAP4KE_TM_MAX_PORTS)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid port %u", port);

        return FAP_ERROR;
    }

    {
        fapTm_port_t *port_p = &fapTmCtrl_g.port[port];
        int nbrOfQueues = (port_p->schedulerIndex == FAP4KE_TM_WAN_SCHEDULER_IDX) ?
            FAP4KE_TM_WAN_QUEUE_MAX : FAP4KE_TM_LAN_QUEUE_MAX;
        uint32 fapIdx;
        int bps = 0;

//        if(!FAP_TM_PORT_IS_ENABLED(port_p))
        if(port_p->schedulerIndex == FAP_TM_SCHEDULER_INDEX_INVALID)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Port %u is disabled", port);

            return FAP_ERROR;
        }

        if(queue >= nbrOfQueues)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Queue %d (nbrOfQueues %d)",
                          queue, nbrOfQueues);

            return FAP_ERROR;
        }

        for(fapIdx=0; fapIdx<NUM_FAPS; fapIdx++)
        {
            fap4keTm_rateStatsQueue_t *rateStatsQueue_p;

            if(port_p->schedulerIndex == FAP4KE_TM_WAN_SCHEDULER_IDX)
            {
                rateStatsQueue_p = &pHostFapSdram(fapIdx)->tm.rateStats.wan.queue[queue];
            }
            else
            {
                rateStatsQueue_p = &pHostFapSdram(fapIdx)->tm.rateStats.lan[port_p->schedulerIndex].queue[queue];
            }

            bps += rateStatsQueue_p->bps;
        }

        return (bps << 3);
    }
}

/*******************************************************************************
 *
 * Function: fapTm_ioctl
 *
 * IOCTL interface to the FAP TM API.
 *
 *******************************************************************************/
int fapTm_ioctl(unsigned long arg)
{
    fapIoctl_tm_t *userTm_p = (fapIoctl_tm_t *)arg;
    fapIoctl_tm_t tm;
    int ret = FAP_SUCCESS;

    copy_from_user(&tm, userTm_p, sizeof(fapIoctl_tm_t));

    switch(tm.cmd)
    {
        case FAP_IOCTL_TM_CMD_MASTER_CONFIG:
            __trace("MASTER_CONFIG: enable <%u>", tm.enable);
            fapTm_masterConfig(tm.enable);
            break;

        case FAP_IOCTL_TM_CMD_PORT_CONFIG:
            __trace("PORT_CONFIG: port <%u>, mode <%u>, kbps <%u>, mbs <%u>, shapingType <%u>",
                    tm.port, tm.mode, tm.kbps, tm.mbs, tm.shapingType);
            ret = fapTm_portConfig(tm.port, tm.mode, tm.kbps, tm.mbs, tm.shapingType);
            break;

        case FAP_IOCTL_TM_CMD_GET_PORT_CONFIG:
            ret = fapTm_getPortConfig(tm.port, tm.mode, &tm.kbps, &tm.mbs,
                                      (fapTm_shapingType_t *)&tm.shapingType);
            __trace("GET_PORT_CONFIG: port <%u>, mode <%u>, kbps <%u>, mbs <%u>, shapingType <%u>",
                    tm.port, tm.mode, tm.kbps, tm.mbs, tm.shapingType);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;

        case FAP_IOCTL_TM_CMD_GET_PORT_CAPABILITY:
            ret = fapTm_getPortCapability(tm.port,
                                          &tm.portCapability.schedType,
                                          &tm.portCapability.maxQueues, 
                                          &tm.portCapability.maxSpQueues,
                                          &tm.portCapability.portShaper,
                                          &tm.portCapability.queueShaper);           
            __trace("GET_PORT_CONFIG: port <%u>, schedType <%u>, maxQueues <%u>, maxSpQueues <%u>, portShaper <%u>, queueShaper <%u>",
                    tm.port, 
                    tm.portCapability.schedType,
                    tm.portCapability.maxQueues, 
                    tm.portCapability.maxSpQueues,
                    tm.portCapability.portShaper,
                    tm.portCapability.queueShaper); 
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;

        case FAP_IOCTL_TM_CMD_PORT_MODE:
            __trace("PORT_MODE: port <%u>, mode <%u>", tm.port, tm.mode);
            ret = fapTm_setPortMode(tm.port, tm.mode);
            break;

        case FAP_IOCTL_TM_CMD_MODE_RESET:
            __trace("MODE_RESET: port <%u>, mode <%u>", tm.port, tm.mode);
            ret = fapTm_modeReset(tm.port, tm.mode);
            break;

        case FAP_IOCTL_TM_CMD_PORT_TYPE:
            __trace("PORT_TYPE: port <%u>, portType <%u>", tm.port, tm.portType);
            ret = fapTm_portType(tm.port, tm.portType);
            break;

        case FAP_IOCTL_TM_CMD_PORT_ENABLE:
            __trace("PORT_ENABLE: port <%u>, mode <%u>, enable <%u>", tm.port, tm.mode, tm.enable);
            ret = fapTm_portEnable(tm.port, tm.mode, tm.enable);
            break;

        case FAP_IOCTL_TM_CMD_PORT_APPLY:
            __trace("PORT_APPLY: port <%u>", tm.port);
            ret = fapTm_apply(tm.port);
            break;

        case FAP_IOCTL_TM_CMD_QUEUE_CONFIG:
            __trace("QUEUE_CONFIG: port <%u>, mode <%u>, queue <%u>, shaperType <%u>, kbps <%u>, mbs <%u>",
                    tm.port, tm.mode, tm.queue, tm.shaperType, tm.kbps, tm.mbs);
            ret = fapTm_queueConfig(tm.port, tm.mode, tm.queue, tm.shaperType, tm.kbps, tm.mbs);
            break;

        case FAP_IOCTL_TM_CMD_QUEUE_UNCONFIG:
            __trace("QUEUE_UNCONFIG: port <%u>, mode <%u>, queue <%u>",
                    tm.port, tm.mode, tm.queue);
            ret = fapTm_queueUnconfig(tm.port, tm.mode, tm.queue);
            break;

        case FAP_IOCTL_TM_CMD_GET_QUEUE_CONFIG:
            ret = fapTm_getQueueConfig(tm.port, tm.mode, tm.queue,
                                       &tm.kbps, &tm.minKbps, &tm.mbs, &tm.weight, &tm.qsize);
            __trace("GET_QUEUE_CONFIG: port <%u>, mode <%u>, queue <%u> "
                    "kbps <%u>, minKbps <%u>, mbs <%u>, weight <%u>, qsize <%u>",
                    tm.port, tm.mode, tm.queue,
                    tm.kbps, tm.minKbps, tm.mbs, tm.weight, tm.qsize);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;

        case FAP_IOCTL_TM_CMD_ALLOC_QUEUE_PROFILE_ID:
            ret = fapTm_allocQueueProfileId(&tm.queueProfileId);

            __trace("GET_PROFILE_ID: queueProfileId <%u>", tm.queueProfileId);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;

        case FAP_IOCTL_TM_CMD_FREE_QUEUE_PROFILE_ID:
            __trace("FREE_PROFILE_ID: queueProfileId <%u>", tm.queueProfileId);
            ret = fapTm_freeQueueProfileId(tm.queueProfileId);

            break;

        case FAP_IOCTL_TM_CMD_QUEUE_PROFILE_CONFIG:
            __trace("QUEUE_PROFILE_CONFIG: queueProfileId <%u>, dropProbability <%d>, "
                    "minThreshold <%d>, maxThreshold <%d>", tm.queueProfileId,
                    tm.dropProbability, tm.minThreshold, tm.maxThreshold);
            ret = fapTm_queueProfileConfig(tm.queueProfileId, tm.dropProbability,
                                           tm.minThreshold, tm.maxThreshold);
            break;

        case FAP_IOCTL_TM_CMD_GET_QUEUE_PROFILE_CONFIG:
            ret = fapTm_getQueueProfileConfig(tm.queueProfileId, &tm.dropProbability,
                                              &tm.minThreshold, &tm.maxThreshold);

            __trace("GET_QUEUE_PROFILE_CONFIG: queueProfileId <%u>, dropProbability <%d>, "
                    "minThreshold <%d>, maxThreshold <%d>", tm.queueProfileId,
                    tm.dropProbability, tm.minThreshold, tm.maxThreshold);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;

        case FAP_IOCTL_TM_CMD_QUEUE_DROP_ALG_CONFIG:
            __trace("QUEUE_DROP_ALG_CONFIG: port <%u>, queue <%u>, dropAlgorithm <%d>, "
                    "queueProfileId <%d>, queueProfileIdHi <%d>, priorityMask0 <0x%x>, "
                    "priorityMask1 <0x%x>",
                    tm.port, tm.queue, tm.dropAlgorithm, tm.queueProfileId,
                    tm.queueProfileIdHi, tm.priorityMask0, tm.priorityMask1);
            ret = fapTm_queueDropAlgConfig(tm.port, tm.queue, tm.dropAlgorithm,
                                           tm.queueProfileId,
                                           tm.queueProfileIdHi,
                                           tm.priorityMask0, tm.priorityMask1);
            break;

        case FAP_IOCTL_TM_CMD_QUEUE_DROP_ALG_CONFIG_EXT:
            __trace("QUEUE_DROP_ALG_CONFIG_EXT: port <%u>, queue <%u>, dropAlgorithm <%d>, "
                    "dropProbabilityLo <%d>, minThresholdLo <%d>, maxThresholdLo <%d>, "
                    "dropProbabilityHi <%d>, minThresholdHi <%d>, maxThresholdHi <%d>, "
                    "priorityMask0 <0x%x>, priorityMask1 <0x%x>",
                    tm.port, tm.queue, tm.dropAlgorithm,
                    tm.dropProbability, tm.minThreshold, tm.maxThreshold,
                    tm.dropProbabilityHi, tm.minThresholdHi, tm.maxThresholdHi,
                    tm.priorityMask0, tm.priorityMask1);
            ret = fapTm_queueDropAlgConfigExt(tm.port, tm.queue, tm.dropAlgorithm,
                                              tm.dropProbability, tm.minThreshold, tm.maxThreshold,
                                              tm.dropProbabilityHi, tm.minThresholdHi, tm.maxThresholdHi,
                                              tm.priorityMask0, tm.priorityMask1);
            break;            

        case FAP_IOCTL_TM_CMD_GET_QUEUE_DROP_ALG_CONFIG:
            ret = fapTm_getQueueDropAlgConfig(tm.port, tm.queue,
                                              (fapTm_dropAlg_t *)&tm.dropAlgorithm,
                                              &tm.queueProfileId,
                                              &tm.queueProfileIdHi,
                                              &tm.priorityMask0,
                                              &tm.priorityMask1);
            __trace("GET_QUEUE_DROP_ALG_CONFIG: port <%u>, queue <%u>, dropAlgorithm <%d>, "
                    "queueProfileId <%d>, queueProfileIdHi <%d>, priorityMask0 <0x%x>, "
                    "priorityMask1 <0x%x>",
                    tm.port, tm.queue, tm.dropAlgorithm, tm.queueProfileId,
                    tm.queueProfileIdHi, tm.priorityMask0, tm.priorityMask1);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
        case FAP_IOCTL_TM_CMD_XTM_QUEUE_DROP_ALG_CONFIG:
            __trace("XTM_QUEUE_DROP_ALG_CONFIG: channel <%u>, dropAlgorithm <%d>, "
                    "queueProfileId <%d>, queueProfileIdHi <%d>, priorityMask0 <0x%x>, "
                    "priorityMask1 <0x%x>",
                    tm.channel, tm.dropAlgorithm, tm.queueProfileId,
                    tm.queueProfileIdHi, tm.priorityMask0, tm.priorityMask1);
            ret = fapTm_XtmQueueDropAlgConfig(tm.channel, tm.dropAlgorithm,
                                              tm.queueProfileId,
                                              tm.queueProfileIdHi,
                                              tm.priorityMask0, tm.priorityMask1);
            break;

        case FAP_IOCTL_TM_CMD_XTM_QUEUE_DROP_ALG_CONFIG_EXT:
            __trace("XTM_QUEUE_DROP_ALG_CONFIG_EXT: channel <%u>, dropAlgorithm <%d>, "
                    "dropProbabilityLo <%d>, minThresholdLo <%d>, maxThresholdLo <%d>, "
                    "dropProbabilityHi <%d>, minThresholdHi <%d>, maxThresholdHi <%d>",
                    tm.channel, tm.dropAlgorithm,
                    tm.dropProbability, tm.minThreshold, tm.maxThreshold,
                    tm.dropProbabilityHi, tm.minThresholdHi, tm.maxThresholdHi);
            ret = fapTm_XtmQueueDropAlgConfigExt(tm.channel, tm.dropAlgorithm,
                                                 tm.dropProbability, tm.minThreshold, tm.maxThreshold,
                                                 tm.dropProbabilityHi, tm.minThresholdHi, tm.maxThresholdHi);
            break;


        case FAP_IOCTL_TM_CMD_GET_XTM_QUEUE_DROP_ALG_CONFIG:
            ret = fapTm_getXtmQueueDropAlgConfig(tm.channel,
                                                 (fapTm_dropAlg_t *)&tm.dropAlgorithm,
                                                 &tm.queueProfileId,
                                                 &tm.queueProfileIdHi,
                                                 &tm.priorityMask0,
                                                 &tm.priorityMask1);
            __trace("GET_XTM_QUEUE_DROP_ALG_CONFIG: channel <%u>, dropAlgorithm <%d>, "
                    "queueProfileId <%d>, queueProfileIdHi <%d>, priorityMask0 <0x%x>, "
                    "priorityMask1 <0x%x>",
                    tm.channel, tm.dropAlgorithm, tm.queueProfileId,
                    tm.queueProfileIdHi, tm.priorityMask0, tm.priorityMask1);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;
#else
        case FAP_IOCTL_TM_CMD_XTM_QUEUE_DROP_ALG_CONFIG:
            __trace("GET_XTM_QUEUE_DROP_ALG_CONFIG: FAIL! XTM is not enabled!");
            break;

        case FAP_IOCTL_TM_CMD_GET_XTM_QUEUE_DROP_ALG_CONFIG:
            __trace("GET_XTM_QUEUE_DROP_ALG_CONFIG: FAIL! XTM is not enabled!");
	    break;
#endif

        case FAP_IOCTL_TM_CMD_GET_QUEUE_STATS:
            ret = fapTm_getQueueStats(tm.port, tm.mode, tm.queue,
                                     &tm.queueStats.txPackets,
                                     &tm.queueStats.txBytes,
                                     &tm.queueStats.droppedPackets,
                                     &tm.queueStats.droppedBytes);
            __trace("GET_QUEUE_STATS: port <%u>, mode <%u>, queue <%u> "
                    "txPackets <%u>, txBytes <%u>, droppedPackets <%u>, droppedBytes <%u>",
                    tm.port, tm.mode, tm.queue,
                    tm.queueStats.txPackets,
                    tm.queueStats.txBytes,
                    tm.queueStats.droppedPackets,
                    tm.queueStats.droppedBytes);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;
            
        case FAP_IOCTL_TM_CMD_QUEUE_WEIGHT:
            __trace("QUEUE_WEIGHT: port <%u>, mode <%u>, queue <%u>, weight <%u>",
                    tm.port, tm.mode, tm.queue, tm.weight);
            ret = fapTm_setQueueWeight(tm.port, tm.mode, tm.queue, tm.weight);
            break;

        case FAP_IOCTL_TM_CMD_ARBITER_CONFIG:
            __trace("ARBITER_CONFIG: port <%u>, mode <%u>, arbiterType <%u>, arbiterArg <%u>",
                    tm.port, tm.mode, tm.arbiterType, tm.arbiterArg);
            ret = fapTm_arbiterConfig(tm.port, tm.mode, tm.arbiterType, tm.arbiterArg);
            break;

        case FAP_IOCTL_TM_CMD_GET_ARBITER_CONFIG:
            ret = fapTm_getArbiterConfig(tm.port, tm.mode, (fap4keTm_arbiterType_t *)&tm.arbiterType, &tm.arbiterArg);
            __trace("GET_ARBITER_CONFIG: port <%u>, mode <%u>, arbiterType <%u>, arbiterArg <%u>",
                    tm.port, tm.mode, tm.arbiterType, tm.arbiterArg);
            copy_to_user(userTm_p, &tm, sizeof(fapIoctl_tm_t));
            break;

        case FAP_IOCTL_TM_MAP_TMQUEUE_TO_SWQUEUE:
            __trace("TMQUEUE_TO_SWQUEUE: port <%u>, queue <%u>, swQueue <%u>",
                    tm.port, tm.queue, tm.swQueue);
            ret = fapTm_mapTmQueueToSwQueue(tm.port, tm.queue, tm.swQueue);
            break;

        case FAP_IOCTL_TM_CMD_STATUS:
            fapTm_status();
            break;

        case FAP_IOCTL_TM_CMD_STATS:
            fapTm_stats(tm.port);
            break;

        case FAP_IOCTL_TM_CMD_DUMP_MAPS:
            fapTm_dumpMaps();
            break;

        default:
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid IOCTL cmd %d", tm.cmd);
            ret = FAP_ERROR;
    }

    return ret;
}

/*******************************************************************************
 *
 * Function: fapTm_init
 *
 * Initializes the FAP TM API.
 *
 *******************************************************************************/
void fapTm_init(void)
{
    int port;

    memset(&fapTmCtrl_g, 0, sizeof(fapTm_ctrl_t));

    for(port=0; port<FAP4KE_TM_MAX_PORTS; ++port)
    {
        fapTm_port_t *port_p = &fapTmCtrl_g.port[port];

        port_p->schedulerIndex = FAP_TM_SCHEDULER_INDEX_INVALID;
    }

    fapTm_masterConfig(1);

    if(sizeof(xmit2FapMsg_tm_t) > 12)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "xmit2FapMsg_tm_t Overflow! (%d)",
                      sizeof(xmit2FapMsg_tm_t));
    }

    /* reserve by marking the queue profile index#0 as used */
    fapTmCtrl_g.queueProfileMask[0] = 0x1;
}

EXPORT_SYMBOL(fapTm_masterConfig);
EXPORT_SYMBOL(fapTm_portConfig);
EXPORT_SYMBOL(fapTm_getPortConfig);
EXPORT_SYMBOL(fapTm_getPortCapability);
EXPORT_SYMBOL(fapTm_queueConfig);
EXPORT_SYMBOL(fapTm_queueUnconfig);
EXPORT_SYMBOL(fapTm_getQueueConfig);
EXPORT_SYMBOL(fapTm_allocQueueProfileId);
EXPORT_SYMBOL(fapTm_freeQueueProfileId);
EXPORT_SYMBOL(fapTm_queueProfileConfig);
EXPORT_SYMBOL(fapTm_getQueueProfileConfig);
EXPORT_SYMBOL(fapTm_queueDropAlgConfig);
EXPORT_SYMBOL(fapTm_getQueueDropAlgConfig);
EXPORT_SYMBOL(fapTm_checkSetHighPrio);
EXPORT_SYMBOL(fapTm_getQueueStats);
EXPORT_SYMBOL(fapTm_setQueueWeight);
EXPORT_SYMBOL(fapTm_arbiterConfig);
EXPORT_SYMBOL(fapTm_getArbiterConfig);
EXPORT_SYMBOL(fapTm_setPortMode);
EXPORT_SYMBOL(fapTm_getPortMode);
EXPORT_SYMBOL(fapTm_modeReset);
EXPORT_SYMBOL(fapTm_portType);
EXPORT_SYMBOL(fapTm_portEnable);
EXPORT_SYMBOL(fapTm_apply);
EXPORT_SYMBOL(fapTm_getQueueRate);
EXPORT_SYMBOL(fapTm_mapTmQueueToSwQueue);

#endif /* CC_FAP4KE_TM */
