#ifndef __FAP4KE_TM_H_INCLUDED__
#define __FAP4KE_TM_H_INCLUDED__

/*

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved

<:label-BRCM:2011:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *******************************************************************************
 *
 * File Name  : fap4ke_tm.h
 *
 * Description: This file contains the FAP Traffic Management global definitions.
 *
 *******************************************************************************
 */

#define CC_FAP4KE_TM

//#define CC_FAP4KE_TM_TIMER_DEBUG

#if defined(CC_FAP4KE_TM)

#if !defined(CC_FAP4KE_TIMER_HIRES)
#error "FAP Traffic Management: High Resolution Timer is required (CC_FAP4KE_TIMER_HIRES)"
#endif

#define FAP4KE_TM_TIMER_JIFFIES      2  /* 400 usec */

#define FAP4KE_TM_ETH_IFG            20 /* bytes */
#define FAP4KE_TM_ETH_CRC_LEN        4
#define FAP4KE_TM_ETH_OVERHEAD       (FAP4KE_TM_ETH_CRC_LEN + FAP4KE_TM_ETH_IFG) 

/* Ethernet packet + 2 VLAN Tags + PPPoE + Overhead */
#define FAP4KE_TM_BUCKET_SIZE_MIN    (1514 + 8 + 8 + FAP4KE_TM_ETH_OVERHEAD)

#define FAP4KE_TM_LAN_QUEUE_MAX      4
#define FAP4KE_TM_LAN_QUEUE_MASK     0x3
#define FAP4KE_TM_WAN_QUEUE_MAX      8
#define FAP4KE_TM_WAN_QUEUE_MASK     0x7
#define FAP4KE_TM_HIGH_PRIO_MASK     0x8
#define FAP4KE_TM_TC_MAX             64
#define FAP4KE_TM_QUEUE_WEIGHT_MAX   255 /* MUST be kept in sync with fap4keTm_arbiterQueue_t */
#define FAP4KE_TM_QUEUE_MAX          FAP4KE_TM_WAN_QUEUE_MAX
#define FAP4KE_TM_QUEUE_NONE         0xFF
#define FAP4KE_TM_QUEUE_LOCAL_DEPTH  4 /* entries in local memory */

#define FAP4KE_TM_QUEUE_SDRAM_DEPTH  508 /* entries in SDRAM memory */

#if defined(CONFIG_BCM963268)
#define FAP4KE_TM_SCHEDULER_MAX      7
#define FAP4KE_TM_SCHEDULER_MASK     0x7F
#else
#error "FAP Traffic Manager: Unsupported chip"
#endif

#define FAP4KE_TM_WAN_SCHEDULER_IDX  (FAP4KE_TM_SCHEDULER_MAX - 1)
#define FAP4KE_TM_LAN_SCHEDULER_MAX  (FAP4KE_TM_SCHEDULER_MAX - 1)

#define FAP4KE_TM_MAX_PORTS          16 /* MUST be kept in sync with portEnableMask */
/* we support less number of queueu profile to support each queue to have 2 unique
 * queue profile, because we assume those queue profile should be shared */
#define FAP4KE_TM_QUEUE_PROFILE_MAX  8
#define FAP4KE_TM_QUEUE_DROP_ALG_MAX ((FAP4KE_TM_LAN_SCHEDULER_MAX * FAP4KE_TM_LAN_QUEUE_MAX) + FAP4KE_TM_WAN_QUEUE_MAX)

#define p4keTmCtrl ( &p4keDspramGbl->tmCtrl )
#define p4keTmStorage ( &p4kePsmGbl->tmStorage )

typedef union {
    struct {
        void *txdma; /* BcmPktDma_LocalEthTxDma */
        uint8 *pBuf;
        uint32 key;
        int param1;
        uint16 len;
        uint16 dmaStatus;
        uint8 bufSource;
        uint8 extSwTagLen;
    };
    uint32 u32[6];
} fap4keTm_queueEntry_t;

typedef struct {
    fap4keTm_queueEntry_t *entry_p;
    uint16 depth;
    uint16 count;
    uint16 write;
    uint16 read;
} fap4keTm_queueCtrl_t;

typedef struct {
    uint16 tokens;
    uint16 bucketSize;
    int bucket;
} fap4keTm_shaper_t;

typedef enum {
    FAP4KE_TM_QUEUE_TYPE_LOCAL=0,
    FAP4KE_TM_QUEUE_TYPE_SDRAM,
    FAP4KE_TM_QUEUE_TYPE_TOTAL
} fap4keTm_queueType_t;

typedef enum {
    FAP4KE_TM_SHAPER_TYPE_MIN=0,
    FAP4KE_TM_SHAPER_TYPE_MAX,
    FAP4KE_TM_SHAPER_TYPE_TOTAL
} fap4keTm_shaperType_t;

typedef struct {
    uint16 totalCount;
    uint8 inSdram;
    uint8 weight; /* MUST be kept in sync with FAP4KE_TM_QUEUE_WEIGHT_MAX */
    uint32 packets;
    uint32 bytes;
    uint32 dropped;
    fap4keTm_queueCtrl_t ctrl[FAP4KE_TM_QUEUE_TYPE_TOTAL];
    fap4keTm_shaper_t shaper[FAP4KE_TM_SHAPER_TYPE_TOTAL];
} fap4keTm_arbiterQueue_t;

typedef enum {
    FAP4KE_TM_ARBITER_TYPE_SP=0,
    FAP4KE_TM_ARBITER_TYPE_WRR,
    FAP4KE_TM_ARBITER_TYPE_SP_WRR,
    FAP4KE_TM_ARBITER_TYPE_WFQ,
    FAP4KE_TM_ARBITER_TYPE_TOTAL
} fap4keTm_arbiterType_t;

typedef struct {
    struct {
        uint8 nbrOfQueues : 4;
        uint8 qShaping    : 1;
        uint8 unused      : 3;
    };
    struct {
        uint8 isSp         : 1;
        uint8 arbiterType  : 3;
        uint8 lowPrioQueue : 4;
    };
    union {
        /* Arbiter specific variables */
        struct {
            uint8 currQueueIndex[FAP4KE_TM_SHAPER_TYPE_TOTAL];
        } wfq;
        struct {
            uint8 currQueueIndex;
            uint8 currWeightCount;
        } wrr;
    };
} fap4keTm_arbiterCtrl_t;

typedef struct {
    fap4keTm_arbiterQueue_t queue[FAP4KE_TM_LAN_QUEUE_MAX];
    fap4keTm_arbiterCtrl_t ctrl;
} fap4keTm_lanArbiter_t;

typedef struct {
    fap4keTm_arbiterQueue_t queue[FAP4KE_TM_WAN_QUEUE_MAX];
    fap4keTm_arbiterCtrl_t ctrl;
} fap4keTm_wanArbiter_t;

typedef struct fap4keTm_scheduler_s {
    fap4keTm_shaper_t shaper;
    fap4keTm_arbiterQueue_t *(* arbiterQueueFunc)(struct fap4keTm_scheduler_s *scheduler_p);
    int (* arbiterCtrlFunc)(struct fap4keTm_scheduler_s *scheduler_p);
    fap4keTm_arbiterQueue_t *arbiterQueue_p;
    fap4keTm_arbiterCtrl_t *arbiterCtrl_p;
    uint8 tmQueueToSwQueue[FAP4KE_TM_QUEUE_MAX];
    uint16 currShaperType;
    uint8 priorityToQueueMask;
    uint32 totalCount;
    uint16 schedPauseCount;
} fap4keTm_scheduler_t;

typedef enum {
    FAP4KE_TM_DROPALG_TYPE_DT=0,
    FAP4KE_TM_DROPALG_TYPE_RED,
    FAP4KE_TM_DROPALG_TYPE_WRED,
    FAP4KE_TM_DROPALG_TYPE_MAX
} fap4kTm_dropAlgType_t;

typedef struct fap4keTm_queueProfile_s {
    uint32 dropProb     : 4,
           minThreshold : 14,
           maxThreshold : 14;
} fap4keTm_queueProfile_t;

typedef struct fap4keTm_queueDropAlg_s {
    uint32 dropAlg          : 4,
           queueProfileIdLo : 14,
           queueProfileIdHi : 14;
} fap4keTm_queueDropAlg_t;

#if defined(CC_FAP4KE_TM_TIMER_DEBUG)
typedef struct {
    uint32 startTime;
    uint32 timerCount;
    int hwTimerCount;
    int error;
    uint32 overflow;
} fap4keTm_ctrlDebug_t;
#endif

typedef struct {
    fap4keTm_queueEntry_t queueMem[FAP4KE_TM_LAN_QUEUE_MAX][FAP4KE_TM_QUEUE_LOCAL_DEPTH];
} fap4keTm_localLanStorage_t;

typedef struct {
    fap4keTm_queueEntry_t queueMem[FAP4KE_TM_WAN_QUEUE_MAX][FAP4KE_TM_QUEUE_LOCAL_DEPTH];
} fap4keTm_localWanStorage_t;

typedef struct {
     fap4keTm_localLanStorage_t lan[FAP4KE_TM_LAN_SCHEDULER_MAX];
     fap4keTm_localWanStorage_t wan;
} fap4keTm_storage_t;

typedef struct {
    fap4keTm_queueEntry_t queueMem[FAP4KE_TM_LAN_QUEUE_MAX][FAP4KE_TM_QUEUE_SDRAM_DEPTH];
} fap4keTm_sdramLanStorage_t;

typedef struct {
    fap4keTm_queueEntry_t queueMem[FAP4KE_TM_WAN_QUEUE_MAX][FAP4KE_TM_QUEUE_SDRAM_DEPTH];
} fap4keTm_sdramWanStorage_t;

typedef struct {
    fap4keTm_sdramLanStorage_t lan[FAP4KE_TM_LAN_SCHEDULER_MAX];
    fap4keTm_sdramWanStorage_t wan;
} fap4keTm_sdram_t;

typedef struct {
    uint8 isTmInitialized;
    uint8 masterEnable;
    uint16 portEnableMask;
    uint16 portEnableMaskConfig;
    uint16 portPauseEnableMask;
    uint8 portToScheduler[FAP4KE_TM_MAX_PORTS];
    fap4keTmr_timer_t shaperTimer;
    fap4keTm_scheduler_t scheduler[FAP4KE_TM_SCHEDULER_MAX];
    fap4keTm_queueProfile_t queueProfile[FAP4KE_TM_QUEUE_PROFILE_MAX];
    fap4keTm_queueDropAlg_t queueDropAlg[FAP4KE_TM_QUEUE_DROP_ALG_MAX];
    fap4keTm_lanArbiter_t lanArbiter[FAP4KE_TM_LAN_SCHEDULER_MAX];
    fap4keTm_wanArbiter_t wanArbiter;
    fap4keTm_sdram_t *tmSdram_p;
    uint8 dmaLength[FAP4KE_TM_QUEUE_LOCAL_DEPTH+1];
#if defined(CC_FAP4KE_TM_TIMER_DEBUG)
    fap4keTm_ctrlDebug_t debug;
#endif
} fap4keTm_ctrl_t;


/*
 * Rate Stats
 */

typedef struct {
    uint32 bps;
} fap4keTm_rateStatsQueue_t;

typedef struct {
    fap4keTm_rateStatsQueue_t queue[FAP4KE_TM_LAN_QUEUE_MAX];
} fap4keTm_rateStatsLan_t;

typedef struct {
    fap4keTm_rateStatsQueue_t queue[FAP4KE_TM_WAN_QUEUE_MAX];
} fap4keTm_rateStatsWan_t;

typedef struct {
    fap4keTm_rateStatsLan_t lan[FAP4KE_TM_LAN_SCHEDULER_MAX];
    fap4keTm_rateStatsWan_t wan;
} fap4keTm_rateStats_t;

#define p4keTmRateStats (&p4keSdram->tm.rateStats)

/*
 * Functions
 */

static inline int fap4keTm_firstBitSet(uint32 mask)
{
    int bit = 0;

    while(mask >>= 1)
    {
        bit++;
    }

    return bit;
}

static inline char *arbiterTypeName(fap4keTm_arbiterType_t arbiterType)
{
    switch(arbiterType)
    {
        case FAP4KE_TM_ARBITER_TYPE_SP:
            return "SP";

        case FAP4KE_TM_ARBITER_TYPE_WRR:
            return "WRR";

        case FAP4KE_TM_ARBITER_TYPE_SP_WRR:
            return "SP+WRR";

        case FAP4KE_TM_ARBITER_TYPE_WFQ:
            return "WFQ";

        default:
            return "ERROR";
    }
}

static inline uint32 fap4keTm_kbpsToTokens(int kbps)
{
    return (((1000/8) * kbps) / (FAPTMR_HZ_HIRES / FAP4KE_TM_TIMER_JIFFIES));
}

static inline uint32 fap4keTm_mbsToBucketSize(uint32 mbs)
{
    uint32 bucketSize = mbs;

    if(bucketSize < FAP4KE_TM_BUCKET_SIZE_MIN)
    {
        bucketSize = FAP4KE_TM_BUCKET_SIZE_MIN;
    }

    return bucketSize;
}

void fap4keTm_enable(uint8 masterEnable);
void fap4keTm_portConfig(uint8 port, uint8 enable,
                         uint16 tokens, uint16 bucketSize);
void fap4keTm_pauseEnable(uint8 port, uint8 enable);
void fap4keTm_pauseScheduler(uint8 port, uint16 count);
void fap4keTm_queueConfig(uint8 port, uint8 queue, uint8 shaperType,
                          uint16 tokens, uint16 bucketSize, uint8 weight);
void fap4keTm_queueProfileConfig(uint16 queueProfileId, uint8 dropProb,
                          uint16 minThreshold, uint16 maxThreshold);
void fap4keTm_queueDropAlgConfig(uint8 port, uint8 queue, uint8 dropAlg,
                          uint16 queueProfileIdLo, uint16 queueProfileIdHi);
int fap4keTm_arbiterConfig(uint8 port, uint8 arbiterType, uint8 arbiterArg, uint8 qShaping);
void fap4keTm_stats(uint8 port);
void fap4keTm_init(void);
int fap4keTm_mapPortToScheduler(uint8 port, int schedulerIndex);
int fap4keTm_mapTmQueueToSwQueue(uint8 port, uint8 queue, uint8 swQueue);
void fap4keTm_dumpMaps(void);
void fap4keTm_refreshQueueRates(void);

#endif /* CC_FAP4KE_TM */

#endif  /* defined(__FAP4KE_TM_H_INCLUDED__) */
