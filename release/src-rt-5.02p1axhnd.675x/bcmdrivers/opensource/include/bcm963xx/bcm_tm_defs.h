#ifndef __BCM_TM_DEFS_H_INCLUDED__
#define __BCM_TM_DEFS_H_INCLUDED__

/*
  Copyright (c) 2015 Broadcom Corporation
  All Rights Reserved

  <:label-BRCM:2015:DUAL/GPL:standard

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
* File Name  : bcm_tm_defs.h
*
* Description: This file contains the specification of some common definitions
*      and interfaces to other modules. This file may be included by both
*      Kernel and userapp (C only).
*
*******************************************************************************
*/

#include <pktHdr.h>

/*----- Defines -----*/

#define BCM_TM_VERSION              "0.1"
#if defined (__KERNEL__)
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0)
#define BCM_TM_VER_STR              "v" BCM_TM_VERSION
#else
#define BCM_TM_VER_STR              "v" BCM_TM_VERSION " " __DATE__ " " __TIME__
#endif
#else
#define BCM_TM_VER_STR              "v" BCM_TM_VERSION " " __DATE__ " " __TIME__
#endif
#define BCM_TM_MODNAME              "Broadcom Traffic Manager (bcmtm)"

#define BCM_TM_NAME                 "bcmtm"

#define BCM_TM_ERROR                (-1)
#define BCM_TM_SUCCESS              0

#define BCM_TM_TX_SUCCESS           0
#define BCM_TM_TX_FULL              1
#define BCM_TM_TX_DISABLED          2

/* BCM TM Character Device */
#define BCM_TM_DRV_MAJOR            327
#define BCM_TM_DRV_NAME             BCM_TM_NAME
#define BCM_TM_DRV_DEVICE_NAME      "/dev/" BCM_TM_NAME

/* BCM TM Control Utility Executable */
#define BCM_TM_CTL_UTILITY_PATH     "/bin/bcmtmctl"

/* BCM TM Proc FS Directory Path */
#define BCM_TM_PROC_FS_DIR_PATH     BCM_TM_NAME

#if defined( __KERNEL__ )
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
#include <asm/system.h>
#else
#include <asm/cmpxchg.h>
#endif
#define BCM_TM_KERNEL_LOCK(level)          local_irq_save(level)
#define BCM_TM_KERNEL_UNLOCK(level)        local_irq_restore(level)
#endif

#define BCM_TM_DONT_CARE        ~0
#define BCM_TM_IS_DONT_CARE(_x) ( ((_x) == (typeof(_x))(BCM_TM_DONT_CARE)) )

#define BCM_TM_DRV_PORT_MAX     16

/*
 *------------------------------------------------------------------------------
 * Common defines for BCM TM layers.
 *------------------------------------------------------------------------------
 */
typedef enum {
    BCM_TM_IOCTL_MASTER_CONFIG=0,
    BCM_TM_IOCTL_MASTER_ON,
    BCM_TM_IOCTL_MASTER_OFF,
    BCM_TM_IOCTL_PORT_CONFIG,
    BCM_TM_IOCTL_GET_PORT_CONFIG,
    BCM_TM_IOCTL_GET_PORT_CAPABILITY,
    BCM_TM_IOCTL_PORT_MODE,
    BCM_TM_IOCTL_MODE_RESET,
    BCM_TM_IOCTL_PORT_ENABLE,
    BCM_TM_IOCTL_PORT_DISABLE,
    BCM_TM_IOCTL_PORT_APPLY,
    BCM_TM_IOCTL_QUEUE_CONFIG,
    BCM_TM_IOCTL_QUEUE_UNCONFIG,
    BCM_TM_IOCTL_GET_QUEUE_CONFIG,
    BCM_TM_IOCTL_SET_QUEUE_CAP,
    BCM_TM_IOCTL_ALLOC_QUEUE_PROFILE_ID,
    BCM_TM_IOCTL_FREE_QUEUE_PROFILE_ID,
    BCM_TM_IOCTL_QUEUE_PROFILE_CONFIG,
    BCM_TM_IOCTL_GET_QUEUE_PROFILE_CONFIG,
    BCM_TM_IOCTL_QUEUE_DROP_ALG_CONFIG,
    BCM_TM_IOCTL_GET_QUEUE_DROP_ALG_CONFIG,
    BCM_TM_IOCTL_XTM_QUEUE_DROP_ALG_CONFIG,
    BCM_TM_IOCTL_GET_XTM_QUEUE_DROP_ALG_CONFIG,
    BCM_TM_IOCTL_GET_QUEUE_STATS,
    BCM_TM_IOCTL_QUEUE_WEIGHT,
    BCM_TM_IOCTL_ARBITER_CONFIG,
    BCM_TM_IOCTL_GET_ARBITER_CONFIG,
    BCM_TM_IOCTL_STATUS,
    BCM_TM_IOCTL_STATS,
    BCM_TM_IOCTL_MAX
} bcmTmIoctl_cmd_t;

#define BCM_TM_DRV_PHY_TYPE_ETH_PREFIX  "eth"
#define BCM_TM_DRV_PHY_TYPE_PTM_PREFIX  "ptm"
#define BCM_TM_DRV_PHY_TYPE_ATM_PREFIX  "atm"
#define BCM_TM_DRV_PHY_TYPE_DPI_PREFIX  "dpi"

typedef enum {
    BCM_TM_DRV_PHY_TYPE_ETH=0,
    BCM_TM_DRV_PHY_TYPE_XTM,
    BCM_TM_DRV_PHY_TYPE_DPI,
    BCM_TM_DRV_PHY_TYPE_MAX
} bcmTmDrv_phyType_t;

typedef enum {
    BCM_TM_DRV_SHAPER_TYPE_MIN=0,
    BCM_TM_DRV_SHAPER_TYPE_MAX,
    BCM_TM_DRV_SHAPER_TYPE_TOTAL
} bcmTmDrv_shaperType_t;

typedef enum {
    BCM_TM_DRV_MODE_AUTO=0,
    BCM_TM_DRV_MODE_MANUAL,
    BCM_TM_DRV_MODE_MAX
} bcmTmDrv_mode_t;

typedef enum {
    BCM_TM_DRV_SHAPING_TYPE_DISABLED=0,
    BCM_TM_DRV_SHAPING_TYPE_RATE,
    BCM_TM_DRV_SHAPING_TYPE_RATIO,
    BCM_TM_DRV_SHAPING_TYPE_MAX
} bcmTmDrv_shapingType_t;

typedef enum {
    BCM_TM_DRV_DROP_ALG_DT=0,
    BCM_TM_DRV_DROP_ALG_RED,
    BCM_TM_DRV_DROP_ALG_WRED,
    BCM_TM_DRV_DROP_ALG_MAX
} bcmTmDrv_dropAlg_t;

typedef enum {
    BCM_TM_DRV_ARBITER_TYPE_SP=0,
    BCM_TM_DRV_ARBITER_TYPE_WRR,
    BCM_TM_DRV_ARBITER_TYPE_SP_WRR,
    BCM_TM_DRV_ARBITER_TYPE_WFQ,
    BCM_TM_DRV_ARBITER_TYPE_TOTAL
} bcmTmDrv_arbiterType_t;

typedef struct {
    uint32_t txPackets;        
    uint32_t txBytes;
    uint32_t droppedPackets;
    uint32_t droppedBytes;
    uint32_t bps;
} bcmTmDrv_queueStats_t;

/* Port TM scheduling capability */
#define BCM_TM_SP_CAPABLE       (1 << 0)
#define BCM_TM_WRR_CAPABLE      (1 << 1)
#define BCM_TM_WDRR_CAPABLE     (1 << 2)
#define BCM_TM_WFQ_CAPABLE      (1 << 3)
#define BCM_TM_SP_WRR_CAPABLE   (1 << 4)

typedef struct {
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
} bcmTmDrv_portCapability_t;

typedef int (* bcmTmDrv_txCallback_t)(uint16_t length, void *param_p);
typedef int (* bcmTmDrv_freeCallback_t)(uint16_t length, void *param_p);

typedef struct {
    bcmTmDrv_phyType_t phy;
    uint32_t port;
    uint32_t queue;
    uint16_t length;
    void *param_p;
} bcmTmDrv_enqueue_t;

typedef struct {
    bcmTmDrv_phyType_t phy;
    int port;
    int enable;
    bcmTmDrv_mode_t mode;
    int queue;
    int channel;
    int qsize;
    bcmTmDrv_dropAlg_t dropAlgorithm;
    int minThreshold;
    int maxThreshold;
    int dropProbability;
    int queueProfileId;
    int queueProfileIdHi;
    uint32_t priorityMask0;
    uint32_t priorityMask1;
    bcmTmDrv_shaperType_t shaperType;
    int kbps;
    int minKbps;
    int mbs;
    int weight;
    int nbrOfEntriesCap;
    bcmTmDrv_arbiterType_t arbiterType;
    int arbiterArg;
    bcmTmDrv_shapingType_t shapingType;
    uint32_t nbrOfQueues;
    uint32_t nbrOfEntries;
    uint32_t paramSize;
    bcmTmDrv_portCapability_t portCapability;
    bcmTmDrv_queueStats_t queueStats;
    bcmTmDrv_txCallback_t txCallbackFunc;
    bcmTmDrv_freeCallback_t freeCallbackFunc;
} bcmTmDrv_arg_t;

#endif  /* defined(__BCM_TM_DEFS_H_INCLUDED__) */
