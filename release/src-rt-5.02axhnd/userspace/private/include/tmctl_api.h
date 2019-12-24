/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:proprietary:standard

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


#ifndef _TMCTL_API_H_
#define _TMCTL_API_H_

/*!\file tmctl_api.h
 * \brief This file contains declarations for QoS related functions.
 *
 */

/* ----------------------------------------------------------------------------
 * Version  Description
 *
 * 1.0      Initial release
 * 1.1      Integrated TMCTL with FAP TM.
 *          Added support for Switch LAG.
 *          Added queue priority field to data structure tmctl_queueCfg_t.
 *          Updated tmctl_portTmInit()  with qcfgFlag.
 *          Added redMaxThreshold field to data structure tmctl_queueDropAlg_t.
 *          Added T-CONT and LLID default queue sizes.
 * 1.2      Added support for Runner WAN queue CIR shaping.
 *          Added port TM initialization config flags.
 * ----------------------------------------------------------------------------
 */
#define TMCTL_VERSION   "1.1"


#include "bcmtypes.h"
#include "bcmctl_syslogdefs.h"


/*#define CC_TMCTL_DEBUG 1 */

#if defined(CC_TMCTL_DEBUG)
#define TMCTL_DEBUGCODE(code)    code
#else
#define TMCTL_DEBUGCODE(code)
#endif /* CC_TMCTL_DEBUG */

#define TMCTL_ERRCODE(code)    code

#define tmctl_debug(fmt, arg...) \
{ \
    TMCTL_DEBUGCODE(printf("%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg); ) \
    BCMCTL_SYSLOGCODE(tmctl, LOG_DEBUG, fmt, ##arg); \
}

#define tmctl_error(fmt, arg...) \
{ \
    TMCTL_ERRCODE(printf("ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg); ) \
    BCMCTL_SYSLOGCODE(tmctl, LOG_ERR, fmt, ##arg); \
}

#if defined(BCM_PON_RDP)
#define TMCTL_DEF_ETH_Q_SZ_US     (256)
#define TMCTL_DEF_ETH_Q_SZ_DS     (128)
#define TMCTL_DEF_TCONT_Q_SZ      (256)
#define TMCTL_DEF_LLID_Q_SZ       (256)
#elif defined(BCM_PON_XRDP) || defined(CONFIG_BCM963158)
/* Default ethernet queue sizes for XRDP are configured in bytes in RDPA; sizes below are multiplied by QUEUE_SIZE_FACTOR bytes */
#define TMCTL_DEF_TCONT_Q_SZ      tmctl_getDefQSize(TMCTL_DEV_GPON, TMCTL_DIR_UP)
#define TMCTL_DEF_LLID_Q_SZ       tmctl_getDefQSize(TMCTL_DEV_EPON, TMCTL_DIR_UP)
#define TMCTL_DEF_ETH_Q_SZ_US_10G (768) /* ~1200k bytes */
#define TMCTL_DEF_ETH_Q_SZ_US_1G_GPON  (195) /* ~300k bytes */
#define TMCTL_DEF_ETH_Q_SZ_US_1G_EPON  (130) /* ~200K bytes */
#define TMCTL_DEF_ETH_Q_SZ_US     tmctl_getDefQSize(TMCTL_DEV_ETH, TMCTL_DIR_UP)
#if defined(CHIP_6846)
#define TMCTL_DEF_ETH_Q_SZ_DS     (130) /* ~200k bytes */
#else
#define TMCTL_DEF_ETH_Q_SZ_DS     (260) /* ~400k bytes*/
#endif
#elif defined(CHIP_63268)
/* FAP TM queue size = 512 (4 local sram + 508 sdram) */
#define TMCTL_DEF_ETH_Q_SZ_US     (512)
#define TMCTL_DEF_ETH_Q_SZ_DS     (512)
#define TMCTL_DEF_TCONT_Q_SZ      (256)
#define TMCTL_DEF_LLID_Q_SZ       (256)
#else
#define TMCTL_DEF_ETH_Q_SZ_US     (384)
#define TMCTL_DEF_ETH_Q_SZ_DS     (1024)
#define TMCTL_DEF_TCONT_Q_SZ      (256)
#define TMCTL_DEF_LLID_Q_SZ       (256)
#endif

#define TMCTL_PRIO_MIN            (0)
#define TMCTL_PRIO_MAX            (7)

/* Port TM scheduling capability */
#define TMCTL_SP_CAPABLE      0x1
#define TMCTL_WRR_CAPABLE     0x2
#define TMCTL_WDRR_CAPABLE    0x4
#define TMCTL_WFQ_CAPABLE     0x8
#define TMCTL_SP_WRR_CAPABLE  0x10
#define TMCTL_1LEVEL_CAPABLE  0x20

/* Port TM initialization config flags
 * bit 0: indicate whether the default queues for the port will be automatically
 *        configured or not.
 * bits 8-11: indicate the scheduler type to be configured for the port.
 * Other bits: reserved.
 */
#define TMCTL_INIT_DEFAULT_QUEUES   0x00000001
#define TMCTL_QIDPRIO_MAP_Q7P7      0x00000002
#define TMCTL_QIDPRIO_MAP_Q0P7      0x00000004

#define TMCTL_SET_DUAL_RATE         0x00000008

#define TMCTL_SCHED_TYPE_MASK       0x00000F00

#define TMCTL_SCHED_TYPE_SP_WRR     0x00000000
#define TMCTL_SCHED_TYPE_SP         0x00000100
#define TMCTL_SCHED_TYPE_WRR        0x00000200
#define TMCTL_SCHED_TYPE_WDRR       0x00000300
#define TMCTL_SCHED_TYPE_WFQ        0x00000400


typedef enum
{
   TMCTL_SCHED_INVALID = 0,
   TMCTL_SCHED_SP,
   TMCTL_SCHED_WRR,
   TMCTL_SCHED_WDRR,
   TMCTL_SCHED_WFQ

} tmctl_sched_e;

typedef enum
{
   TMCTL_DROP_DT = 0,
   TMCTL_DROP_RED,
   TMCTL_DROP_WRED

} tmctl_dropAlg_e;

typedef enum
{
   TMCTL_ERROR   = -1,
   TMCTL_SUCCESS = 0,
   TMCTL_NOT_FOUND,
   TMCTL_UNSUPPORTED

} tmctl_ret_e;

/* ----------------------------------------------------------------------------
 * tmctl device type.
 * Used for selecting the specific member of the union structure tmctl_if_t.
 * ----------------------------------------------------------------------------
 */
typedef enum
{
   TMCTL_DEV_ETH = 0,
   TMCTL_DEV_EPON,
   TMCTL_DEV_GPON,
   TMCTL_DEV_XTM,
   TMCTL_DEV_SVCQ

} tmctl_devType_e;


/* ----------------------------------------------------------------------------
 * Union structure for tmctl interface/port identification
 * Union member can be selected by tmctl device type.
 * ----------------------------------------------------------------------------
 */
typedef struct
{
   const char* ifname;

} tmctl_ethIf_t;

typedef struct
{
   int llid;

} tmctl_eponIf_t;

typedef struct
{
   int tcontid;

} tmctl_gponIf_t;

typedef struct
{
   const char* ifname;

} tmctl_xtmIf_t;

typedef union
{
   tmctl_ethIf_t  ethIf;
   tmctl_eponIf_t eponIf;
   tmctl_gponIf_t gponIf;
   tmctl_xtmIf_t  xtmIf;

} tmctl_if_t;

/* ----------------------------------------------------------------------------
 * Structure for port tm parameters
 * ----------------------------------------------------------------------------
 */
typedef struct
{
   uint32_t schedCaps;   /* A bitmap indicating the queue scheduler
                          * capability of this port. Each bit denotes
                          * a scheduling capability defined by constants
                          * TMCTL_SP_CAPABLE, TMCTL_WRR_CAPABLE, etc.
                          */
   int      maxQueues;   /* Max number of queues supported by this port */
   int      maxSpQueues; /* Max number of SP queues allowed by the
                          * queue scheduler when co-exist with other
                          * WRR queues on this port.
                          */
   BOOL     portShaper;  /* Boolean to indicate whether port rate
                          * shaping is supported by this port.
                          */
   BOOL     queueShaper; /* Boolean to indicate whether queue rate
                          * shaping is supported by this port.
                          */
   BOOL     dualRate;    /* Boolean to indicate whether dual rate
                          * is supported by this port.
                          */
   uint32_t cfgFlags;    /* Port TM actual initialization config flags.
                          * See bit definitions above. (dynamic variable)
                          */
   int      numQueues;   /* Port TM actual number of queues set at initialization.
                          * supported only for PON. (dynamic variable)
                          */
} tmctl_portTmParms_t;

/* ----------------------------------------------------------------------------
 * Structure for shaper configuration parameters
 * ----------------------------------------------------------------------------
 */
typedef struct
{
   int shapingRate;           /* Shaping rate in kbps. 0 implies no shaping, -1 implies not supported. */
   int shapingBurstSize;      /* Shaping burst size in bytes. -1 implies not supported. */
   int minRate;               /* Minimum rate in kbps. 0 implies no shaping, -1 implies not supported. */

} tmctl_shaper_t;

/* ----------------------------------------------------------------------------
 * Structure for queue configuration parameters
 * ----------------------------------------------------------------------------
 */
typedef struct
{
   int            qid;        /* Queue ID. [0..maxQueues-1]. */
   int            priority;   /* Queue priority. [0..highestPriority].
                               * Greater value denotes higher priority level.
                               * i.e. 0 is the lowest priority level.
                               */
   int            qsize;      /* Queue size */
   int            weight;     /* Queue weight. Ignored if SP */
   tmctl_sched_e  schedMode;  /* Queue scheduling mode */
   tmctl_shaper_t shaper;     /* Queue Shaper configuration */
   BOOL           bestEffort; /* Queue is best effort */

} tmctl_queueCfg_t;

/* ----------------------------------------------------------------------------
 * Structure for port queue configurations
 * ----------------------------------------------------------------------------
 */
#define MAX_TMCTL_QUEUES_BASELINE   8
#define MAX_TMCTL_QUEUES_EXTENDED   32

typedef struct
{
   int              numQueues;   /* Number of queues configured */
   tmctl_queueCfg_t qcfg[MAX_TMCTL_QUEUES_BASELINE];

} tmctl_portQcfg_t;

/* ----------------------------------------------------------------------------
 * Structure for queue profile configuration
 * ----------------------------------------------------------------------------
 */
typedef struct
{
   int             dropProb; /* 0 to 100 */
   int             minThreshold;
   int             maxThreshold;

} tmctl_queueProfile_t;

/* ----------------------------------------------------------------------------
 * Structure for queue drop algorithm configuration
 * ----------------------------------------------------------------------------
 */
typedef struct
{
   int redMinThreshold;
   int redMaxThreshold;
   int redPercentage;

} tmctl_queueDropAlgExt_t;

typedef struct
{
   tmctl_dropAlg_e dropAlgorithm; /* DT, RED, WRED */
   int             queueProfileIdLo;
   int             queueProfileIdHi;
   uint32_t        priorityMask0;
   uint32_t        priorityMask1;
   tmctl_queueDropAlgExt_t dropAlgLo;
   tmctl_queueDropAlgExt_t dropAlgHi;

} tmctl_queueDropAlg_t;


/* ----------------------------------------------------------------------------
 * Structure for queue statistics
 * ----------------------------------------------------------------------------
 */
typedef struct
{
   uint32_t txPackets;
   uint32_t txBytes;
   uint32_t droppedPackets;
   uint32_t droppedBytes;

} tmctl_queueStats_t;


/* ----------------------------------------------------------------------------
 * Data structure for dscp to pbit feature
 * ----------------------------------------------------------------------------
 */

#define MAX_PBIT_VALUE   7
#define TOTAL_PBIT_NUM   8
#define TOTAL_DSCP_NUM   64

typedef struct
{
    uint32_t dscp[TOTAL_DSCP_NUM];
} tmctl_dscpToPbitCfg_t;


typedef struct
{
    int pbit[TOTAL_PBIT_NUM];
} tmctl_pbitToQCfg_t;


typedef enum
{
    TMCTL_QOS_FC = 0,
    TMCTL_QOS_IC,
    TMCTL_QOS_MCAST,
    TMCTL_QOS_MAX
} tmctl_qosType_e;


typedef enum
{
    TMCTL_DIR_DN = 0,
    TMCTL_DIR_UP,
    TMCTL_DIR_MAX
} tmctl_dir_e;


/* ----------------------------------------------------------------------------
 * This function initializes the basic TM settings for a port based on its
 * TM capability.
 *
 * Note that if the port had already been initialized, all its existing
 * configuration will be deleted before re-initialization.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    cfgFlags (IN) Port TM initialization flags. See bit definitions above.
 *    numQueues (IN) Number of queues to be set for TM.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_portTmInit(tmctl_devType_e devType,
                             tmctl_if_t*     if_p,
                             uint32_t        cfgFlags,
                             int             numQueues);


/* ----------------------------------------------------------------------------
 * This function un-initializes all TM configurations of a port. This
 * function may be called when the port is down.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_portTmUninit(tmctl_devType_e devType,
                               tmctl_if_t*     if_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of a software queue. If the
 * configuration is not found, qid in the config structure will be
 * returned as -1.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    queueId (IN) Queue ID must be in the range of [0..maxQueues-1].
 *    qcfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getQueueCfg(tmctl_devType_e   devType,
                              tmctl_if_t*       if_p,
                              int               queueId,
                              tmctl_queueCfg_t* qcfg_p);


/* ----------------------------------------------------------------------------
 * This function configures a software queue for a port. The qeueu ID shall
 * be specified in the configuration parameter structure. If the queue
 * already exists, its configuration will be modified. Otherwise, the queue
 * will be added.
 *
 * Note that for Ethernet port with an external Switch, the new queue
 * configuration may not be applied immediately to the Switch. For instance,
 * SF2 only supports one of the following priority queuing options:
 *
 *    Q0  Q1  Q2  Q3  Q4  Q5  Q6  Q7
 * 1) SP  SP  SP  SP  SP  SP  SP  SP
 * 2) WRR WRR WRR WRR WRR WRR WRR SP
 * 3) WRR WRR WRR WRR WRR WRR SP  SP
 * 4) WRR WRR WRR WRR WRR SP  SP  SP
 * 5) WRR WRR WRR WRR SP  SP  SP  SP
 * 6) WRR WRR WRR WRR WRR WRR WRR WRR
 *
 * This function will commit the new queue configuration to SF2 only when
 * all the queue configurations of the port match one of the priority
 * queuing options supported by the Switch.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    qcfg_p (IN) Queue config parameters.
 *                Notes:
 *                - qid must be in the range of [0..maxQueues-1].
 *                - For 63268, 63138 or 63148 TMCTL_DEV_ETH device type,
 *                  -- the priority of SP queue must be set to qid.
 *                  -- the priority of WRR/WDRR/WFQ queue must be set to 0.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setQueueCfg(tmctl_devType_e   devType,
                              tmctl_if_t*       if_p,
                              tmctl_queueCfg_t* qcfg_p);


/* ----------------------------------------------------------------------------
 * This function deletes a software queue from a port.
 *
 * Note that for Ethernet port with an external Switch, the corresponding
 * Switch queue will not be deleted.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    queueId (IN) The queue ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_delQueueCfg(tmctl_devType_e devType,
                              tmctl_if_t*     if_p,
                              int             queueId);


/* ----------------------------------------------------------------------------
 * This function gets the port shaper configuration.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    shaper_p (OUT) The shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getPortShaper(tmctl_devType_e devType,
                                tmctl_if_t*     if_p,
                                tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function configures the port shaper for shaping rate, shaping burst
 * size and minimum rate. If port shaping is to be done by the external
 * Switch, the corresponding Switch port shaper will be configured.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    shaper_p (IN) The shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setPortShaper(tmctl_devType_e devType,
                                tmctl_if_t*     if_p,
                                tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function allocates a free queue profile index.
 *
 * Parameters:
 *    queueProfileId_p (OUT) Queue Profile ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_allocQueueProfileId(int* queueProfileId_p);


/* ----------------------------------------------------------------------------
 * This function free a queue profile index.
 *
 * Parameters:
 *    queueProfileId (IN) Queue Profile ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_freeQueueProfileId(int queueProfileId);


/* ----------------------------------------------------------------------------
 * This function gets the queue profile of a queue profile index.
 *
 * Parameters:
 *    queueProfileId (IN) Queue Profile ID.
 *    queueProfile_p (OUT) The drop algorithm configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getQueueProfile(int                   queueProfileId,
                                  tmctl_queueProfile_t* queueProfile_p);


/* ----------------------------------------------------------------------------
 * This function sets the queue profile of a queue profile index.
 *
 * Parameters:
 *    queueProfileId (IN) Queue Profile ID.
 *    queueProfile_p (IN) The drop algorithm configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setQueueProfile(int                   queueProfileId,
                                  tmctl_queueProfile_t* queueProfile_p);


/* ----------------------------------------------------------------------------
 * This function gets the drop algorithm of a queue.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    queueId (IN) Queue ID.
 *    dropAlg_p (OUT) The drop algorithm configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getQueueDropAlg(tmctl_devType_e       devType,
                                  tmctl_if_t*           if_p,
                                  int                   queueId,
                                  tmctl_queueDropAlg_t* dropAlg_p);


/* ----------------------------------------------------------------------------
 * This function sets the drop algorithm of a queue.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    queueId (IN) Queue ID.
 *    dropAlg_p (IN) The drop algorithm configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setQueueDropAlg(tmctl_devType_e       devType,
                                  tmctl_if_t*           if_p,
                                  int                   queueId,
                                  tmctl_queueDropAlg_t* dropAlg_p);

tmctl_ret_e tmctl_setQueueDropAlgExt(tmctl_devType_e          devType,
                                     tmctl_if_t*              if_p,
                                     int                      queueId,
                                     tmctl_queueDropAlg_t* dropAlg_p);


/* ----------------------------------------------------------------------------
 * This function gets the drop algorithm of a XTM channel.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    channelId (IN) Channel ID.
 *    dropAlg_p (OUT) The drop algorithm configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getXtmChannelDropAlg(tmctl_devType_e       devType,
                                       int                   channelId,
                                       tmctl_queueDropAlg_t* dropAlg_p);


/* ----------------------------------------------------------------------------
 * This function sets the drop algorithm of a XTM channel.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    channelId (IN) Channel ID.
 *    dropAlg_p (IN) The drop algorithm configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setXtmChannelDropAlg(tmctl_devType_e       devType,
                                       int                   channelId,
                                       tmctl_queueDropAlg_t* dropAlg_p);


/* ----------------------------------------------------------------------------
 * This function gets the statistics of a queue.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    queueId (IN) Queue ID.
 *    stats_p (OUT) The queue stats.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getQueueStats(tmctl_devType_e     devType,
                                tmctl_if_t*         if_p,
                                int                 queueId,
                                tmctl_queueStats_t* stats_p);


/* ----------------------------------------------------------------------------
 * This function gets port TM parameters (capabilities) from the device driver.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    tmParms_p (OUT) Structure to return port TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getPortTmParms(tmctl_devType_e      devType,
                                 tmctl_if_t*          if_p,
                                 tmctl_portTmParms_t* tmParms_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of dscp to pbit table. If the
 * configuration is not found, ....
 *
 * Parameters:
 *    cfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getDscpToPbit(tmctl_dscpToPbitCfg_t* cfg_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of dscp to pbit table.
 *
 * Parameters:
 *    cfg_p (IN) config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setDscpToPbit(tmctl_dscpToPbitCfg_t* cfg_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of pbit to q table. If the
 * configuration is not found, ....
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    cfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getPbitToQ(tmctl_devType_e devType,
                                 tmctl_if_t* if_p,
                                 tmctl_pbitToQCfg_t* cfg_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of pbit to q table.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    cfg_p (IN) config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setPbitToQ(tmctl_devType_e devType,
                                 tmctl_if_t* if_p,
                                 tmctl_pbitToQCfg_t* cfg_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of dscp to pbit feature.
 *
 * Parameters:
 *    dir (IN) direction.
 *    enable_p (OUT) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of dscp to pbit feature.
 *
 * Parameters:
 *    dir (IN) direction.
 *    enable_p (IN) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of packet based qos.
 *
 * Parameters:
 *    dir (IN) direction.
 *    type (IN) qos type
 *    enable_p (OUT) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of packet based qos.
 *
 * Parameters:
 *    dir (IN) direction.
 *    type (IN) qos type
 *    enable_p (IN) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p);

/* ----------------------------------------------------------------------------
 * This function sets the size of a queue.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p    (IN) Port identifier.
 *    queueId (IN) Queue ID.
 *    size    (IN) The drop threshold configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setQueueSize(tmctl_devType_e devType,
                                     tmctl_if_t* if_p,
                                     int queueId,
                                     int size);

/* ----------------------------------------------------------------------------
 * This function sets shaper of a queue.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p    (IN) Port identifier.
 *    queueId (IN) Queue ID.
 *    shaper    (IN)  Queue Shaper configuration
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setQueueShaper(tmctl_devType_e devType,
                                     tmctl_if_t* if_p,
                                     int queueId,
                                     tmctl_shaper_t *shaper_p);

/* ----------------------------------------------------------------------------
 * This function get the default queue size.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    dir (IN) direction.
 *
 * Return:
 *    queue size.
 * ----------------------------------------------------------------------------
 */
int tmctl_getDefQSize(tmctl_devType_e devType, tmctl_dir_e dir);

#if defined(BCMCTL_SYSLOG_SUPPORTED)
DECLARE_setSyslogLevel(tmctl);
DECLARE_getSyslogLevel(tmctl);
DECALRE_isSyslogLevelEnabled(tmctl);
DECLARE_setSyslogMode(tmctl);
DECLARE_isSyslogEnabled(tmctl);
#endif /* BCMCTL_SYSLOG_SUPPORTED */


#endif /* _TMCTL_API_H_ */
