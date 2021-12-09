#ifndef __FAP_H_INCLUDED__
#define __FAP_H_INCLUDED__

/*
   Copyright (c) 2007-2012 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2007:DUAL/GPL:standard

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
 * File Name  : fap.h
 *
 * Description: This file contains the specification of some common definitions
 *      and interfaces to other modules. This file may be included by both
 *      Kernel and userapp (C only).
 *
 *******************************************************************************
 */

#include <pktHdr.h>

/*----- Defines -----*/

#define FAP_VERSION              "0.1"
#define FAP_VER_STR              "v" FAP_VERSION
#define FAP_MODNAME              "Broadcom Forwarding Assist Processor (FAP)"

#define FAP_NAME                 "bcmfap"

#ifndef FAP_ERROR
#define FAP_ERROR                (-1)
#endif
#ifndef FAP_SUCCESS
#define FAP_SUCCESS              0
#endif

/* FAP Character Device */
#define FAPDRV_MAJOR             301
#define FAPDRV_NAME              FAP_NAME
#define FAPDRV_DEVICE_NAME       "/dev/" FAPDRV_NAME

/* FAP Control Utility Executable */
#define FAP_CTL_UTILITY_PATH     "/bin/fapctl"

/* FAP Proc FS Directory Path */
#define FAP_PROC_FS_DIR_PATH     FAP_NAME

/* Menuconfig: BRCM_DRIVER_PKTFLOW_DEBUG selection will cause -DPKTDBG C Flags*/
#ifdef PKTDBG
#define CC_FAP_DEBUG
#define CC_FAP_ASSERT
#endif

#if defined( __KERNEL__ )
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
#include <asm/system.h>
#else
#include <asm/cmpxchg.h>
#endif
#define KERNEL_LOCK(level)          local_irq_save(level)
#define KERNEL_UNLOCK(level)        local_irq_restore(level)
#endif

#define FAP_DONT_CARE        ~0
#define FAP_IS_DONT_CARE(_x) ( ((_x) == (typeof(_x))(FAP_DONT_CARE)) )

/*
 *------------------------------------------------------------------------------
 * Common defines for FAP layers.
 *------------------------------------------------------------------------------
 */
#undef FAP_DECL
#define FAP_DECL(x)                 x,  /* for enum declaration in H file */

/*
 *------------------------------------------------------------------------------
 *              Packet CFM character device driver IOCTL enums
 * A character device and the associated userspace utility for design debug.
 * Include fapParser.h for ACTIVATE/DEACTIVATE IOCTLs
 *------------------------------------------------------------------------------
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    FAP_IOC_DUMMY=99,
    FAP_DECL(FAP_IOC_HW)
    FAP_DECL(FAP_IOC_STATUS)
    FAP_DECL(FAP_IOC_INIT)
    FAP_DECL(FAP_IOC_ENABLE)
    FAP_DECL(FAP_IOC_DISABLE)
    FAP_DECL(FAP_IOC_DEBUG)
    FAP_DECL(FAP_IOC_PRINT)
    FAP_DECL(FAP_IOC_CPU)
    FAP_DECL(FAP_IOC_SWQ)
    FAP_DECL(FAP_IOC_DMA_DEBUG)
    FAP_DECL(FAP_IOC_MEM_DEBUG)
    FAP_DECL(FAP_IOC_MTU)
    FAP_DECL(FAP_IOC_TM)
    FAP_DECL(FAP_IOC_DM_DEBUG)
    FAP_DECL(FAP_IOC_FLOODING_MASK)
    FAP_DECL(FAP_IOC_ARL_FLUSH)
    FAP_DECL(FAP_IOC_ARL_SHOW)
    FAP_DECL(FAP_IOC_DO_4KE_TEST)
    FAP_DECL(FAP_IOC_MAX)
} fapIoctl_t;

typedef enum {
    FAP_IOCTL_TM_CMD_MASTER_CONFIG=0,
    FAP_IOCTL_TM_CMD_PORT_CONFIG,
    FAP_IOCTL_TM_CMD_GET_PORT_CONFIG,
    FAP_IOCTL_TM_CMD_GET_PORT_CAPABILITY,
    FAP_IOCTL_TM_CMD_PORT_MODE,
    FAP_IOCTL_TM_CMD_MODE_RESET,
    FAP_IOCTL_TM_CMD_PORT_TYPE,
    FAP_IOCTL_TM_CMD_PORT_ENABLE,
    FAP_IOCTL_TM_CMD_PORT_APPLY,
    FAP_IOCTL_TM_CMD_QUEUE_CONFIG,
    FAP_IOCTL_TM_CMD_QUEUE_UNCONFIG,
    FAP_IOCTL_TM_CMD_GET_QUEUE_CONFIG,
    FAP_IOCTL_TM_CMD_ALLOC_QUEUE_PROFILE_ID,
    FAP_IOCTL_TM_CMD_FREE_QUEUE_PROFILE_ID,
    FAP_IOCTL_TM_CMD_QUEUE_PROFILE_CONFIG,
    FAP_IOCTL_TM_CMD_GET_QUEUE_PROFILE_CONFIG,
    FAP_IOCTL_TM_CMD_QUEUE_DROP_ALG_CONFIG,
    FAP_IOCTL_TM_CMD_QUEUE_DROP_ALG_CONFIG_EXT,
    FAP_IOCTL_TM_CMD_GET_QUEUE_DROP_ALG_CONFIG,
    FAP_IOCTL_TM_CMD_XTM_QUEUE_DROP_ALG_CONFIG,
    FAP_IOCTL_TM_CMD_XTM_QUEUE_DROP_ALG_CONFIG_EXT,
    FAP_IOCTL_TM_CMD_GET_XTM_QUEUE_DROP_ALG_CONFIG,
    FAP_IOCTL_TM_CMD_GET_QUEUE_STATS,    
    FAP_IOCTL_TM_CMD_QUEUE_WEIGHT,
    FAP_IOCTL_TM_CMD_ARBITER_CONFIG,
    FAP_IOCTL_TM_CMD_GET_ARBITER_CONFIG,
    FAP_IOCTL_TM_MAP_TMQUEUE_TO_SWQUEUE,
    FAP_IOCTL_TM_CMD_STATUS,
    FAP_IOCTL_TM_CMD_STATS,
    FAP_IOCTL_TM_CMD_DUMP_MAPS,
    FAP_IOCTL_TM_CMD_MAX
} fapIoctl_tmCmd_t;

/* This MUST be kept in sync with fapTm_mode_t */
typedef enum {
    FAP_IOCTL_TM_MODE_AUTO=0,
    FAP_IOCTL_TM_MODE_MANUAL,
    FAP_IOCTL_TM_MODE_MAX
} fapIoctl_tmMode_t;

/* This MUST be kept in sync with fapTm_portType_t */
typedef enum {
    FAP_IOCTL_TM_PORT_TYPE_LAN=0,
    FAP_IOCTL_TM_PORT_TYPE_WAN,
    FAP_IOCTL_TM_PORT_TYPE_MAX
} fapIoctl_tmPortType_t;

/* This MUST be kept in sync with fap4keTm_shaperType_t */
typedef enum {
    FAP_IOCTL_TM_SHAPER_TYPE_MIN=0,
    FAP_IOCTL_TM_SHAPER_TYPE_MAX,
    FAP_IOCTL_TM_SHAPER_TYPE_TOTAL
} fapIoctl_tmShaperType_t;

/* This MUST be kept in sync with fap4keTm_arbiterType_t */
typedef enum {
    FAP_IOCTL_TM_ARBITER_TYPE_SP=0,
    FAP_IOCTL_TM_ARBITER_TYPE_WRR,
    FAP_IOCTL_TM_ARBITER_TYPE_SP_WRR,
    FAP_IOCTL_TM_ARBITER_TYPE_WFQ,
    FAP_IOCTL_TM_ARBITER_TYPE_TOTAL
} fapIoctl_tmArbiterType_t;

/* This MUST be kept in sync with fapTm_shapingType_t */
typedef enum {
    FAP_IOCTL_TM_SHAPING_TYPE_DISABLED=0,
    FAP_IOCTL_TM_SHAPING_TYPE_RATE,
    FAP_IOCTL_TM_SHAPING_TYPE_RATIO,
    FAP_IOCTL_TM_SHAPING_TYPE_MAX
} fapIoctl_tmShapingType_t;

/* This MUST be kept in sync with fapTm_tmDropAlg_t */
typedef enum
{
    FAP_IOCTL_TM_DROP_ALG_DT=0,
    FAP_IOCTL_TM_DROP_ALG_RED,
    FAP_IOCTL_TM_DROP_ALG_WRED   
} fapIoctl_tmDropAlg_t;


/* Port TM scheduling capability */
#define FAP_TM_SP_CAPABLE       (1 << 0)
#define FAP_TM_WRR_CAPABLE      (1 << 1)
#define FAP_TM_WDRR_CAPABLE     (1 << 2)
#define FAP_TM_WFQ_CAPABLE      (1 << 3)
#define FAP_TM_SP_WRR_CAPABLE   (1 << 4)

#define FAP_TM_LAN_QUEUE_MAX      4
#define FAP_TM_WAN_QUEUE_MAX      8

typedef struct
{
   uint32_t schedType;   /* A bitmap indicating the queue scheduler
                          * capability of this port. Each bit denotes 
                          * a scheduling capability defined by constants
                          * TMCTL_SP_CAPABLE, TMCTL_WRR_CAPABLE, etc.
                          */
   int      maxQueues;   /* Max number of queues supported by this port */
   int      maxSpQueues; /* Max number of SP queues allowed by the
                          * queue scheduler when co-exist with other
                          * WRR queues on this port.
                          */
   uint8_t  portShaper;  /* Boolean to indicate whether port rate
                          * shaping is supported by this port.
                          */
   uint8_t  queueShaper; /* Boolean to indicate whether queue rate
                          * shaping is supported by this port.
                          */
} fapIoctl_tmPortCapability_t;

typedef struct
{
   uint32_t txPackets;        
   uint32_t txBytes;
   uint32_t droppedPackets;
   uint32_t droppedBytes;
   
} fapIoctl_tmQueueStats_t;

typedef struct {
    fapIoctl_tmCmd_t cmd;
    int enable;
    int port;
    fapIoctl_tmMode_t mode;
    int queue;
    int channel;
    int qsize;
    fapIoctl_tmDropAlg_t dropAlgorithm;
    int minThreshold;
    int minThresholdHi;
    int maxThreshold;
    int maxThresholdHi;
    int dropProbability;
    int dropProbabilityHi;
    int queueProfileId;
    int queueProfileIdHi;
    uint32_t priorityMask0;
    uint32_t priorityMask1;
    int swQueue;
    fapIoctl_tmShaperType_t shaperType;
    int kbps;
    int minKbps;
    int mbs;
    int weight;
    fapIoctl_tmArbiterType_t arbiterType;
    int arbiterArg;
    fapIoctl_tmPortType_t portType;
    fapIoctl_tmShapingType_t shapingType;
    fapIoctl_tmPortCapability_t portCapability;
    fapIoctl_tmQueueStats_t queueStats;
} fapIoctl_tm_t;

typedef struct {
    uint32_t packets;
    uint32_t bytes;
    uint32_t time_usec;
} fapPerf_rxResults_t;

typedef struct {
    uint32_t dropped;
} fapPerf_txResults_t;

typedef struct {
    uint8_t running;
    fapPerf_rxResults_t rx;
    fapPerf_txResults_t tx;
} fapPerf_results_t;

typedef struct {
    uint32_t ipSa;
    uint32_t ipDa;
    uint16_t sPort;  /* UDP source port */
    uint16_t dPort;  /* UDP dest port */
} fapPerf_analyzer_t;

typedef struct {
    uint32_t kbps;
    uint32_t copies;
    uint16_t mbs;
    uint16_t total_length;
} fapPerf_generator_t;

typedef enum {
    FAP_DECL(FAP_HW_ENGINE_SWC)
    FAP_DECL(FAP_HW_ENGINE_MCAST)
    FAP_DECL(FAP_HW_ENGINE_ARL)
    FAP_DECL(FAP_HW_ENGINE_L2FLOW)
    FAP_DECL(FAP_HW_ENGINE_ALL) /* max number of FAP_HW enum */
} FapHwEngine_t;

#define FAP_HW_TUPLE_MCAST_MASK    (1<<12)  /* must be an integer power of 2 */
#define FAP_HW_TUPLE_ARL_MASK      (1<<13)  /* must be an integer power of 2 */
#define FAP_HW_TUPLE_L2FLOW_MASK   (1<<14)  /* must be an integer power of 2 */

/* Construct a 16bit tuple from the Engine and matched FlowInfo Element. */
#define FAP_HW_TUPLE(eng,entIx)                                        \
    ( (eng == FAP_HW_ENGINE_MCAST) ?                                   \
      (__force uint16_t)(entIx | FAP_HW_TUPLE_MCAST_MASK) :            \
      ( (eng == FAP_HW_ENGINE_ARL) ?                                   \
        (__force uint16_t)(FAP_HW_TUPLE_ARL_MASK) :                    \
        ( (eng == FAP_HW_ENGINE_L2FLOW) ?                              \
          (__force uint16_t)(entIx | FAP_HW_TUPLE_L2FLOW_MASK) :       \
          (__force uint16_t)(entIx) ) ) )

#define FAP_MAX_FLOWS            512        /* per FAP, should be power of 2 */

extern uint8_t fapGetHwEngine(uint32_t hwTuple);
extern uint16_t fapGetHwEntIx(uint32_t hwTuple);

//#define CC_FAP_ENET_STATS

#if defined(CC_FAP_ENET_STATS)
void fapEnetStats_contextFull(void);
void fapEnetStats_dqmRxFull(void);
void fapEnetStats_rxPackets(void);
void fapEnetStats_txPackets(uint32_t contextCount);
void fapEnetStats_interrupts(void);
void fapEnetStats_dump(void);
#else
#define fapEnetStats_contextFull()
#define fapEnetStats_dqmRxFull()
#define fapEnetStats_rxPackets()
#define fapEnetStats_txPackets(_contextCount)
#define fapEnetStats_interrupts()
#define fapEnetStats_dump()
#endif


//#define CC_FAP_EVENTS

#if defined(CC_FAP_EVENTS)
#undef FAP_DECL
#define FAP_DECL(x) #x,

#define FAP_EVENT_TYPE_NAME                     \
    {                                           \
        FAP_DECL(RX_BEGIN)            \
        FAP_DECL(RX_END)          \
        FAP_DECL(TX_SCHED)    \
        FAP_DECL(TX_BEGIN)    \
        FAP_DECL(TX_END)      \
    }

typedef enum {
    FAP_EVENT_RX_BEGIN,
    FAP_EVENT_RX_END,
    FAP_EVENT_TX_SCHED,
    FAP_EVENT_TX_BEGIN,
    FAP_EVENT_TX_END,
    FAP_EVENT_MAX
} fapEvent_type_t;

void fapEvent_record(fapEvent_type_t type, uint32_t arg);
void fapEvent_print(void);
uint32_t fapEnet_txQueueUsage(uint32 fapIdx);
#else
#define fapEvent_record(_type, _arg)
#define fapEvent_print()
#define fapEnet_txQueueUsage() 0
#endif

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
/*
 *------------------------------------------------------------------------------
 *  Invoked by ARL Protocol layer to clear HW association.
 *  Based on the scope of the request:
 *------------------------------------------------------------------------------
 */

typedef int ( *FAP_CLEAR_HOOK)(uint32_t mcast, uint32_t port);

/*
 *------------------------------------------------------------------------------
 * Flow cache binding to ARL to register ARL upcalls and downcalls
 * Upcalls from FAP to ARL: activate, deactivate and refresh functions.
 * Downcalls from ARL to FAP: clear hardware associations function.
 *------------------------------------------------------------------------------
 */
extern void fap_bind_arl(HOOKP activate_fn, HOOK4PARM deactivate_fn,
                        HOOK3PARM refresh_fn, HOOK32 reset_stats_fn, 
                        HOOK32 clear_fn, FAP_CLEAR_HOOK *fap_clear_fn);
#endif

#endif  /* defined(__FAP_H_INCLUDED__) */
