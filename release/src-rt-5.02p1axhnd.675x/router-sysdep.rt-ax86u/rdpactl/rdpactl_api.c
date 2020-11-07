/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <net/if.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "os_defs.h"

#include "bcmtypes.h"
#include "bcmnet.h"
#include "rdpa_types.h"
#include "rdpa_drv.h"
#include "rdpactl_api.h"

/*
 * Macros
 */

#define RDPACTL_IOCTL_FILE_NAME "/dev/bcmrdpa"

/* #define CC_RDPACTL_DEBUG */

#ifdef CC_RDPACTL_DEBUG
#define rdpaCtl_debug(fmt, arg...) {printf(">>> %s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg); fflush(stdout);}
#define rdpaCtl_error(fmt, arg...) {printf("ERROR[%s.%u]: " fmt, __FUNCTION__, __LINE__, ##arg); fflush(stdout);}
#else
#define rdpaCtl_debug(fmt, arg...)
#define rdpaCtl_error(fmt, arg...)
#endif

/*
 * Static functions
 */

static inline int __rdpa_ioctl(int code, uintptr_t ctx)
{
#ifndef DESKTOP_LINUX
    int fd, ret = 0;
    fd = open(RDPACTL_IOCTL_FILE_NAME, O_RDWR);
    if (fd < 0)
    {
        rdpaCtl_error("%s: %s\n", RDPACTL_IOCTL_FILE_NAME, strerror(errno));
        return -EINVAL;
    }

    ret = ioctl(fd, code, ctx);
    if (ret)
        rdpaCtl_error("ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);

    close(fd);
    return ret;
#else
	rdpaCtl_error("rdpa ioctl %d unsupported in desktop\n", code);
	return EINVAL;
#endif
}

static int __sendTmCommand(rdpa_drv_ioctl_tm_t *tm_p)
{
    return __rdpa_ioctl(RDPA_IOC_TM, (uintptr_t)tm_p);
}

static int __sendIcCommand(rdpa_drv_ioctl_ic_t *ic_p)
{
    return __rdpa_ioctl(RDPA_IOC_IC, (uintptr_t)ic_p);
}

static int __sendPortCommand(rdpa_drv_ioctl_port_t *port_p)
{
    return __rdpa_ioctl(RDPA_IOC_PORT, (uintptr_t)port_p);
}

static int __sendBrCommand(rdpa_drv_ioctl_br_t *br_p)
{
    return __rdpa_ioctl(RDPA_IOC_BRIDGE, (uintptr_t)br_p);
}

static int __sendSysCommand(rdpa_drv_ioctl_sys_t *sys_p)
{
    return __rdpa_ioctl(RDPA_IOC_SYS, (uintptr_t)sys_p);
}

static int __sendLlidCommand(rdpa_drv_ioctl_llid_t *llid_p)
{
    return __rdpa_ioctl(RDPA_IOC_LLID, (uintptr_t)llid_p);
}

static int __sendDsWanUdpFilterCommand(rdpa_drv_ioctl_ds_wan_udp_filter_t *filter_p)
{
    return __rdpa_ioctl(RDPA_IOC_DS_WAN_UDP_FILTER, (uintptr_t)filter_p);
}

static int __sendFilterCommand(rdpa_drv_ioctl_filter_t *filters_p)
{
    return __rdpa_ioctl(RDPA_IOC_FILTERS, (uintptr_t)filters_p);
}

static int __sendDscpToPbitCommand(rdpa_drv_ioctl_dscp_to_pbit_t *dscp_to_pbit_p)
{
    return __rdpa_ioctl(RDPA_IOC_DSCP_TO_PBIT, (uintptr_t)dscp_to_pbit_p);
}

static int __sendPbitToQCommand(rdpa_drv_ioctl_pbit_to_q_t *pbit_to_q_p)
{
    return __rdpa_ioctl(RDPA_IOC_PBIT_TO_Q, (uintptr_t)pbit_to_q_p);
}

static int __sendMiscCommand(rdpa_drv_ioctl_misc_t *misc_cfg_p)
{
    return __rdpa_ioctl(RDPA_IOC_MISC, (uintptr_t)misc_cfg_p);
}

/*
 * Public functions
 */

int rdpaCtl_GetTmMemoryInfo(int  *fpmPoolMemorySize)
{
   int rc = 0;
   rdpa_drv_ioctl_tm_t tm;
   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd = RDPA_IOCTL_TM_CMD_GET_TM_MEMORY_INFO;
   rc = __sendTmCommand(&tm);
   *fpmPoolMemorySize = tm.fpm_pool_memory_size;

   if (rc)
      rdpaCtl_debug("Error!");
   else 
      rdpaCtl_debug("Success!");

   return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetPortTmParms
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   dir,                               // IN: Tm direction
 *   ...,                               // OUT:Tm parameters
 *
 *******************************************************************************/
int rdpaCtl_GetPortTmParms(
        int  devType,
        int  devId,
        int  dir,
        int  *pPortSchedCaps,
        int  *pMaxQueues,
        int  *pMaxSpQueues,
        BOOL *pPortShaper,
        BOOL *pQueueShaper,
        int  *pCfgFlags,
        BOOL *pbFound)
{
   int rc = 0;
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId [%d]", devId);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd = RDPA_IOCTL_TM_CMD_GET_TM_CAPS;
   tm.dev_type = devType;
   tm.dev_id   = devId;
   tm.dir      = dir;

   rc = __sendTmCommand(&tm);

   if (!rc) {
      if (tm.found) {
         *pPortSchedCaps = tm.port_sched_caps;
         *pMaxQueues     = tm.max_queues;
         *pMaxSpQueues   = tm.max_sp_queues;
         *pPortShaper    = tm.port_shaper;
         *pQueueShaper   = tm.queue_shaper;
         *pCfgFlags      = tm.cfg_flags;
         *pbFound        = TRUE;
      } else {
         *pbFound = FALSE;
      }
      rdpaCtl_debug("Success!");
   } else {
      rdpaCtl_debug("Error!");
   }

   return rc;
}


/*******************************************************************************
 *
 * Function: rdpaCtl_GetRootTm
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   pRootTmId,                         // OUT: NEW ROOT Tm ID config
 *   pbFound,                           // OUT: Root Found or NOT
 *
 *******************************************************************************/
int rdpaCtl_GetRootTm(
        int devType,
        int devId,
        int *pRootTmId,
        BOOL *pbFound)
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("dev_id [%d]", devId);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_GET_ROOT_TM;
    tm.dev_type = devType;
    tm.dev_id = devId;

    rc = __sendTmCommand(&tm);

    if (!rc) {
        if (tm.found) {
                        *pRootTmId = tm.root_tm_id;
                        *pbFound   = TRUE;

                        rdpaCtl_debug("FOUND: dev_type [%d] dev_id [%d], ROOT TM ID %d", devType, devId, *pRootTmId);
        } else {
                *pbFound = FALSE;

                        rdpaCtl_debug("dev_type [%d] dev_id [%d], ROOT TM ID not FOUND", devType, devId);
        }
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetRootSpTm
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   pRootSpTmId,                       // OUT: ROOT SP Tm ID
 *   pbFound,                           // OUT: Found or NOT
 *
 *******************************************************************************/
int rdpaCtl_GetRootSpTm(
        int devType,
        int devId,
        int *pRootSpTmId,
        BOOL *pbFound)
{
   int rc = 0;
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId [%d]", devId);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd = RDPA_IOCTL_TM_CMD_GET_ROOT_SP_TM;
   tm.dev_type = devType;
   tm.dev_id = devId;

   rc = __sendTmCommand(&tm);

   if (!rc) {
      if (tm.found) {
         *pRootSpTmId = tm.tm_id;
         *pbFound     = TRUE;

         rdpaCtl_debug("FOUND: devId [%d], ROOT SP TM ID %d", devId, *pRootSpTmId);
      } else {
         *pbFound = FALSE;

         rdpaCtl_debug("devId [%d], ROOT SP TM ID not FOUND", devId);
      }
   }

   return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetRootWrrTm
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   pRootWrrTmId,                      // OUT: ROOT WRR Tm ID
 *   pbFound,                           // OUT: Found or NOT
 *
 *******************************************************************************/
int rdpaCtl_GetRootWrrTm(
        int devType,
        int devId,
        int *pRootWrrTmId,
        BOOL *pbFound)
{
   int rc = 0;
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId [%d]", devId);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd = RDPA_IOCTL_TM_CMD_GET_ROOT_WRR_TM;
   tm.dev_type = devType;
   tm.dev_id = devId;

   rc = __sendTmCommand(&tm);

   if (!rc) {
      if (tm.found) {
         *pRootWrrTmId = tm.tm_id;
         *pbFound      = TRUE;

         rdpaCtl_debug("FOUND: devId [%d], ROOT WRR TM ID %d", devId, *pRootWrrTmId);
      } else {
         *pbFound = FALSE;

         rdpaCtl_debug("devId [%d], ROOT WRR TM ID not FOUND", devId);
      }
   }

   return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetOrl
 *   devType,                   // IN: RDPA device type
 *   devId,                     // IN: RDPA device id
 *   dir,                       // IN: Tm direction
 *   pOrlId,                    // OUT: ORL ID
 *   pShapingRate,              // OUT: Shaping rate
 *   pbOrlLinked,               // OUT: Whether ORL is linked to port or not
 *   pbFound,                   // OUT: Found or NOT
 *
 *******************************************************************************/
int rdpaCtl_GetOrl(
        int  devType,
        int  devId,
        int  dir,
        int  *pOrlId,
        int  *pShapingRate,
        BOOL *pbOrlLinked,
        BOOL *pbFound)
{
   int rc = 0;
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId [%d]", devId);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd = RDPA_IOCTL_TM_CMD_GET_PORT_ORL;
   tm.dev_type = devType;
   tm.dev_id = devId;
   tm.dir    = dir;

   rc = __sendTmCommand(&tm);

   if (!rc) {
      if (tm.found && tm.tm_id != -1) {
         *pOrlId       = tm.tm_id;
         *pShapingRate = tm.shaping_rate;
         *pbOrlLinked  = tm.orl_linked;
         *pbFound      = TRUE;
         rdpaCtl_debug("FOUND: devId [%d], PORT ORL ID %d", devId, *pOrlId);
      } else {
         *pOrlId  = -1;
         *pbFound = FALSE;

         rdpaCtl_debug("devId [%d], PORT ORL ID not FOUND", devId);
      }
   }

   return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetTmByQid
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   qId                                // IN: queue ID
 *   pTmId,                             // OUT: NEW ROOT Tm ID config
 *   pbFound,                           // OUT: Tm ID Found or NOT
 *
 *******************************************************************************/
int rdpaCtl_GetTmByQid(
        int devType,
        int devId,
        int qId,
        int *pTmId,
        BOOL *pbFound)
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devType [%d] devId [%d]", devType, devId);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_GET_BY_QID;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qId;

    rc = __sendTmCommand(&tm);

    if (!rc) {
        *pTmId   = tm.tm_id;
        *pbFound = tm.found;

                /*              rdpaCtl_debug("devId [%d], TM ID %d", devId, *pTmId); */
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetQueueConfig
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   qId,                               // IN: queue ID
 *   dir,                               // IN: Tm direction
 *   pTmId,                             // OUT: NEW ROOT Tm ID config
 *   pbFound,                           // OUT: Tm ID Found or NOT
 *
 *******************************************************************************/
int rdpaCtl_GetQueueConfig(
        int  devType,
        int  devId,
        int  qId,
        int  dir,
        int  *pTmId,
        int  *pQsize,
        int  *pWeight,
        int  *pMinBufs,
        int  *pMinRate,
        int  *pShapingRate,
        BOOL *pBestEffort,
        BOOL *pbFound)
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devId [%d]", devId);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_GET_QUEUE_CONFIG;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qId;
    tm.dir      = dir;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       *pTmId        = tm.tm_id;
       *pQsize       = tm.qsize;
       *pWeight      = tm.weight;
       *pMinRate     = tm.min_rate;
       *pMinBufs     = tm.minBufs;
       *pShapingRate = tm.shaping_rate;
       *pbFound      = tm.found;
       *pBestEffort  = tm.best_effort;

       rdpaCtl_debug("devId [%d], TM ID %d",
                     devId, *pTmId);
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_SetQueueDropAlg
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   qId,                               // IN: queue ID
 *   dir,                               // IN: Tm direction
 *   dropAlg,                           // IN: drop algorithm
 *   redMinThr,                         // IN: RED mininum threshold
 *   redMaxThr,                         // IN: RED maximum thresdhold
 *   priorityMask0,                     // IN: priority mask used for classify high vs low class
 *   priorityMask1,                     // IN: priority mask used for classify high vs low class
 *
 *******************************************************************************/
int rdpaCtl_SetQueueDropAlg(
        int devType,
        int devId,
        int qId,
        int dir,
        int dropAlg,
        int redMinThrLo,
        int redMaxThrLo,
        int redMinThrHi,
        int redMaxThrHi,
        uint32 priorityMask0,
        uint32 priorityMask1)
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devId [%d]", devId);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_SET_Q_DROP_ALG;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qId;
    tm.dir      = dir;
    tm.drop_alg = dropAlg;

    tm.red_min_thr_lo = redMinThrLo;
    tm.red_max_thr_lo = redMaxThrLo;
    tm.red_min_thr_hi = redMinThrHi;
    tm.red_max_thr_hi = redMaxThrHi;
    tm.priority_mask_0 = priorityMask0;
    tm.priority_mask_1 = priorityMask1;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       rdpaCtl_debug("devId [%d], Q ID %d",
                     devId, qId);
    }

    return rc;
}


/*******************************************************************************
 *
 * Function: rdpaCtl_GetQueueDropAlg
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   qId,                               // IN: queue ID
 *   dir,                               // IN: Tm direction
 *   dropAlg,                           // OUT: drop algorithm
 *   redMinThr,                         // OUT: RED mininum threshold
 *   redMaxThr,                         // OUT: RED maximum thresdhold
 *   redDropRate,                       // OUT: RED drop rate in percentage
 *   priorityMask0,                     // OUT: priority mask used for classify high vs low class
 *   priorityMask1,                     // OUT: priority mask used for classify high vs low class
 *
 *******************************************************************************/
int rdpaCtl_GetQueueDropAlg(
        int devType,
        int devId,
        int qId,
        int dir,
        int *dropAlg,
        int *redMinThrLo,
        int *redMaxThrLo,
        int *redDropRateLo,
        int *redMinThrHi,
        int *redMaxThrHi,
        int *redDropRateHi,
        uint32 *priorityMask0,
        uint32 *priorityMask1)
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devId [%d]", devId);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_GET_QUEUE_CONFIG;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qId;
    tm.dir      = dir;

    rc = __sendTmCommand(&tm);

    if (!rc) {
        *dropAlg = tm.drop_alg;
        *redMinThrLo = tm.red_min_thr_lo;
        *redMaxThrLo = tm.red_max_thr_lo;
        *redDropRateLo = tm.red_drop_percent_lo;
        *redMinThrHi = tm.red_min_thr_hi;
        *redMaxThrHi = tm.red_max_thr_hi;
        *redDropRateHi = tm.red_drop_percent_hi;
        *priorityMask0 = tm.priority_mask_0;
        *priorityMask1 = tm.priority_mask_1;
        rdpaCtl_debug("devId [%d], Q ID %d",
                     devId, qId);
    }

    return rc;
}


/*******************************************************************************
 *
 * Function: rdpaCtl_SetQueueSize
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   qId,                               // IN: queue ID
 *   dir,                               // IN: Tm direction
 *   size,                              // IN: drop threshold
 *
 *******************************************************************************/
int rdpaCtl_SetQueueSize(
        int devType,
        int devId,
        int qId,
        int dir,
        int size)
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devType [%d] devId [%d] qId [%d] dir [%d] size [%d]", 
        devType, devId, qId, dir, size );

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_SET_Q_SIZE;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qId;
    tm.dir      = dir;
    tm.qsize = size;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       rdpaCtl_debug("__sendTmCommand return %d", rc);
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_SetQueueShaper
 *   devType,                        // IN: RDPA device type
 *   devId,                            // IN: RDPA device id
 *   qId,                               // IN: queue ID
 *   dir,                                // IN: Tm direction
 *   cir,                                // IN: Committed Information Rate
 *   pir,                                // IN: Peak Information Rate
*    bsz,                                // IN: Burst Size
 *
 *******************************************************************************/
int rdpaCtl_SetQueueShaper(
        int devType,
        int devId,
        int qId,
        int dir,
        int cir,
        int pir,
        int bsz
        )
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devType [%d] devId [%d] qId [%d] dir [%d] cir [%d] pir [%d] bsz [%d]", 
        devType, devId, qId, dir, cir, pir, bsz);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_SET_Q_SHAPER;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qId;
    tm.dir      = dir;
    tm.min_rate = cir;
    tm.shaping_rate = pir;
    tm.burst = bsz;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       rdpaCtl_debug("__sendTmCommand return %d", rc);
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_RootTmConfig
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   dir,                               // IN: Tm direction set
 *   level,                             // IN: Tm type
 *   arbiterMode                        // IN: Tm arbiter mode
 *   rlMode                             // IN: Tm rate limit rate mode
 *   cfgFlags                           // IN: Tm initialization config flags
 *   pRootTmId,                         // OUT: NEW ROOT Tm ID config
 *
 *******************************************************************************/
int rdpaCtl_RootTmConfig(
        int devType,
        int devId,
        int dir,
        int level,
        int arbiterMode,
        int rlMode,
        int cfgFlags,
        int *pRootTmId)
{
    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devId %d dir %d, level %d, arbiterMode %d",
                  devId, dir, level, arbiterMode);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_ROOT_TM_CONFIG;
    tm.dev_type = devType;
    tm.dev_id = devId;
    tm.dir = dir;
    tm.level = level;
    tm.arbiter_mode = arbiterMode;
    tm.rl_mode = rlMode;
    tm.cfg_flags = cfgFlags;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       *pRootTmId = tm.root_tm_id;

       rdpaCtl_debug("NEW ROOT TM ID %d, dir %d, level %d, arbiterMode %d",
                     *pRootTmId, dir, level, arbiterMode);
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_TmRlConfig
 
 *   dir,                             // IN: Tm direction set
 *   tmId,                            // IN: Tm ID config
 *   shapingRate,                     // IN: af_rate
 *   burst,                           // IN: burst_size
 *******************************************************************************/
int rdpaCtl_TmRlConfig(
        int dir,
        int tmId,
        int shapingRate,
        int burst)
{

    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("tmid %d, dir %d, shapingRate %d", tmId, dir, shapingRate);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_TM_RL_CONFIG;
    tm.tm_id = tmId;
    tm.dir = dir;
    tm.shaping_rate = shapingRate;
    tm.burst = burst;

    rc = __sendTmCommand(&tm);

    return rc;
}
/*******************************************************************************
 *
 * Function: rdpaCtl_GetTmRlConfig
 
 *   dir,                                       // IN: Tm direction set
 *   tmId,                                      // IN: Tm ID config
 *   pShapingRate,                              // OUT: af_rate
 *   pBurst,                                    // OUT: burst_size
 *   pbFound                                    // OUT: Exist?
 *******************************************************************************/
int rdpaCtl_GetTmRlConfig(
        int dir,
        int tmId,
        int* pShapingRate,
        int* pBurst,
        BOOL* pbFound)
{
    int rc = 0;

    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("tmid %d, dir %d", tmId, dir);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_GET_TM_CONFIG;
    tm.root_tm_id = tmId;
    tm.dir = dir;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       if (tm.found) {
          *pShapingRate   = tm.shaping_rate;
          *pBurst         = tm.burst;
          *pbFound        = TRUE;
       } else {
          *pbFound = FALSE;
       }
       rdpaCtl_debug("Success!");
    } else {
      rdpaCtl_debug("Error!");
    }

    return rc;
}


/*******************************************************************************
 *
 * Function: rdpaCtl_TmConfig
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   rootTmId,                          // IN: Root TM Id
 *   dir,                               // IN: Tm direction set
 *   level,                             // IN: Tm type
 *   arbiterMode                        // IN: Tm arbiter mode
 *   priorityIndex,                     // IN: Tm Priority index (for SP)
 *   weight,                            // IN: TM Weight (for WRR)
 *   minRate,                           // IN: TM minimum shaping rate in kbps (for SP)
 *   shapingRate,                       // IN: TM maximum shaping rate in kbps (for SP)
 *   burst,                             // IN: burst_size
 *   pTmId,                             // OUT: NEW Tm ID config
 *
 *******************************************************************************/
int rdpaCtl_TmConfig(
        int devType,
        int devId,
        int rootTmId,
        int dir,
        int level,
        int arbiterMode,
        int priorityIndex,
        int weight,
        int minRate,
        int shapingRate,
        int burst,
        int *pTmId)
{

    int rc = 0;
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("rootTmId %d, devType %d, devId %d, dir %d, level %d, "
        "arbiterMode %d priority %d weight %d minRate %d shapingRate %d burst %d",
        rootTmId, devType, devId, dir, level, arbiterMode, priorityIndex, weight,
        minRate, shapingRate, burst);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_TM_CONFIG;
    tm.root_tm_id = rootTmId;
    tm.dev_type = devType;
    tm.dev_id = devId;
    tm.dir = dir;
    tm.level = level;
    tm.arbiter_mode = arbiterMode;
    tm.rl_mode = rdpa_tm_rl_single_rate;
    tm.index = priorityIndex;
    tm.weight = weight;
    tm.min_rate = minRate;
    tm.shaping_rate = shapingRate;
    tm.burst = burst;
    tm.tm_id = *pTmId;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       *pTmId = tm.tm_id;

       rdpaCtl_debug("NEW TM ID %d, dir %d, level %d, arbiterMode %d",
                     *pTmId, dir, level, arbiterMode);
    }

    return rc;
}


/*******************************************************************************
 *
 * Function: rdpaCtl_SvcQTmConfig
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   parentTmId,                        // IN: RDPA parent tm id
 *   level,                             // IN: Tm type
 *   arbiterMode                        // IN: Tm arbiter mode
 *   pTmId,                             // OUT: NEW Tm ID config
 *
 *******************************************************************************/
int rdpaCtl_SvcQTmConfig(
         int devType,
         int devId,
         int parentTmId,
         int index,
         int level,
         int arbiterMode,
         int *pTmId)
{
    int rc = 0;

    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devType %d, devId %d, parentTmId %d, level %d, arbiterMode %d",
        devType, devId, parentTmId, level, arbiterMode);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_TM_CONFIG;
    tm.root_tm_id = parentTmId;
    tm.dev_type = devType;
    tm.dev_id = devId;
    tm.dir = rdpa_dir_us;
    tm.level = level;
    tm.arbiter_mode = arbiterMode;
    tm.index = index;
    tm.service_queue = 1;
    tm.tm_id = *pTmId;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       *pTmId = tm.tm_id;

       rdpaCtl_debug("NEW TM ID %d, level %d, arbiterMode %d",
                     *pTmId, level, arbiterMode);
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetBestEffortTm
 *   devId,                             // IN: RDPA device id
 *   dir,                               // IN: RDPA direction
 *   pTmId                              // OUT: Tm ID
 *
 *******************************************************************************/
int rdpaCtl_GetBestEffortTm(int devId, int dir, int *pTmId)
{
    int rc = 0;

    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devId %d, dir %d", devId, dir);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_GET_BEST_EFFORT_TM_ID;
    tm.dev_id = devId;
    tm.dir = rdpa_dir_us;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       *pTmId = tm.tm_id;
       rdpaCtl_debug("TM ID %d", *pTmId);
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetTmSubsidiary
 *   dir,                                       // IN: Tm direction set
 *   tmId,                                      // IN: Tm ID config
 *   index,                                     // IN: subsidiary index
 *   pTmId                                      // OUT: Tm ID
 *******************************************************************************/
int rdpaCtl_GetTmSubsidiary(
        int dir,
        int tmId,
        int index,
        int *pTmId,
        BOOL* pbFound)
{
    int rc = 0;

    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("tmid %d, dir %d, index %d", tmId, dir, index);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_GET_TM_SUBSIDIARY;
    tm.root_tm_id = tmId;
    tm.dir = dir;

    rc = __sendTmCommand(&tm);

    if (!rc) {
       if (tm.found) {
          *pTmId   = tm.tm_id;
          *pbFound = TRUE;
       } else {
          *pbFound = FALSE;
       }
       rdpaCtl_debug("Success!");
    } else {
      rdpaCtl_debug("Error!");
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_RootTmRemove
 *   tmId,                              // IN: Tm ID config
 *   dir,                               // IN: Tm direction set
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *
 *******************************************************************************/
int rdpaCtl_RootTmRemove(
        int tmId,
        int dir,
        int devType,
        int devId)
{
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("tmId %u, dir %d, dev_type %d, dev_id %d", tmId, dir, devType, devId);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_ROOT_TM_REMOVE;
    tm.root_tm_id = tmId;
    tm.dir = dir;
    tm.dev_type = devType;
    tm.dev_id = devId;

    return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_TmRemove
 *   tmId,                              // IN: Tm ID config
 *   dir,                               // IN: Tm direction set
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *
 *******************************************************************************/
int rdpaCtl_TmRemove(
        int tmId,
        int dir,
        int devType,
        int devId)
{
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("tmId %u, dir %d, dev_type %d, dev_id %d", tmId, dir, devType, devId);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_TM_REMOVE;
    tm.tm_id = tmId;
    tm.dir = dir;
    tm.dev_type = devType;
    tm.dev_id = devId;

    return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_OrlConfig
 *   devType,                           // IN: RDPA device type
 *   devId,                             // IN: RDPA device id
 *   dir,                               // IN: Tm direction set
 *   shapingRate,                       // IN: af_rate
 *   pOrlTmId,                          // OUT: NEW Overall Rate Controller Tm ID config
 *
 *******************************************************************************/
int rdpaCtl_OrlConfig(
        int devType,
        int devId,
        int dir,
        int shapingRate,
        int *pOrlTmId)
{
   int rc = 0;
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId %u, dir %d, shapingRate %d",
        devId, dir, shapingRate);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd = RDPA_IOCTL_TM_CMD_ORL_CONFIG;
   tm.dev_type = devType;
   tm.dev_id = devId;
   tm.dir = dir;
   tm.shaping_rate = shapingRate;

   rc = __sendTmCommand(&tm);

   if (!rc) {
      *pOrlTmId = tm.tm_id;
      rdpaCtl_debug("NEW ORL TM ID %d", *pOrlTmId);
   }

   return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_OrlRemove
 *   devType,   // IN: RDPA device type
 *   devId,     // IN: RDPA device id
 *   dir,       // IN: Tm direction set
 *
 *******************************************************************************/
int rdpaCtl_OrlRemove(
        int devType,
        int devId,
        int dir)
{
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId %u", devId);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd    = RDPA_IOCTL_TM_CMD_ORL_REMOVE;
   tm.dev_type = devType;
   tm.dev_id = devId;
   tm.dir    = dir;

   return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_OrlLink
 *   devType,           // IN: RDPA device type
 *   devId,             // IN: RDPA device id
 *   tmId,              // IN: Overall rate limitter Tm ID config
 *   dir,               // IN: Tm direction set
 *   shapingRate,       // IN: af_rate
 *
 *******************************************************************************/
int rdpaCtl_OrlLink(
        int devType,
        int devId,
        int tmId,
        int dir,
        int shapingRate)
{
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId=%d OrlTmId=%d shapingRate=%d", devId, tmId, shapingRate);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd      = RDPA_IOCTL_TM_CMD_ORL_LINK;
   tm.dev_type = devType;
   tm.dev_id   = devId;
   tm.tm_id    = tmId;
   tm.dir      = dir;
   tm.shaping_rate = shapingRate;

   return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_OrlUnlink
 *   devType,           // IN: RDPA device type
 *   devId,             // IN: RDPA device id
 *   tmId,              // IN: Overall rate limitter Tm ID config
 *   dir,               // IN: Tm direction set
 *
 *******************************************************************************/
int rdpaCtl_OrlUnlink(
        int devType,
        int devId,
        int tmId,
        int dir)
{
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devId=%d OrlTmId=%d", devId, tmId);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd     = RDPA_IOCTL_TM_CMD_ORL_UNLINK;
   tm.dev_type = devType;
   tm.dev_id = devId;
   tm.tm_id   = tmId;
   tm.dir     = dir;

   return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_QueueConfig
 *   devType,                  // IN: RDPA interface type
 *   devId,                    // IN: RDPA interface id
 *   tmId ,                    // IN: Tm ID config
 *   qid ,                     // IN: Queue ID
 *   index,                    // IN: Index in RC
 *   dir ,                     // IN: Tm direction set
 *   qsize ,                   // IN: Queue Size
 *   weight,                   // IN: Queue weight (for WRR)
 *   shapingRate ,             // IN: Shaping Rate (6858 only)
 *   bestEffort ,              // IN: Best effort queue (upstream WAN)
 *   qClean ,                  // IN: Redundant queue clean up flag
 *
 *******************************************************************************/

int rdpaCtl_QueueConfig(
        int devType,
        int devId,
        int tmId,
        int qid,
        int index,
        int dir,
        int qsize,
        int weight,
        int minBufs,
        int shapingRate,
        BOOL bestEffort,
        BOOL qClean)
{
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("dev_type %u, dev_id %u, tmId %u, dir %d, qid %d, qsize %d, wt %d, be %d",
        devType, devId, tmId, dir, qid, qsize, weight, bestEffort ? 1 : 0);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_QUEUE_CONFIG;
    tm.dev_type = devType;
    tm.dev_id = devId;
    tm.tm_id = tmId;
    tm.q_id = qid;
    tm.index = index;
    tm.dir = dir;
    tm.qsize = qsize;
    tm.weight = weight;
    tm.minBufs = minBufs;
    tm.shaping_rate = shapingRate;
    tm.best_effort = bestEffort;
    tm.q_clean = qClean;

    return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_QueueRemove
 *   devType,                    // IN: RDPA interface type
 *   devId,                        // IN: RDPA interface id
 *       tmId,                                  // IN: Tm ID config
 *       dir,                                   // IN: Tm direction set
 *       qid,                                   // IN: Queue ID
 *   index,                 // IN: Index in RC
 *
 *******************************************************************************/

int rdpaCtl_QueueRemove(
      int devType,
      int devId,
      int tmId,
      int dir,
      int qid,
      int index)
{
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devType %u, devId %u, tmId %u, qid %d", devType, devId, tmId, qid);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_QUEUE_REMOVE;
    tm.dev_type = devType;
    tm.dev_id = devId;
    tm.tm_id = tmId;
    tm.dir = dir;
    tm.q_id = qid;
    tm.index = index;
    tm.qsize = 0; /* Remove */

    return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_QueueAllocate
 *   devType,  // IN: RDPA interface type
 *   devId,    // IN: RDPA interface id
 *   qid,      // IN: Queue ID
 *
 *******************************************************************************/

int rdpaCtl_QueueAllocate(
      int devType,
      int devId,
      int qid)
{
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devType %u, devId %u, qid %d", devType, devId, qid);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_QUEUE_ALLOCATE;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qid;

    return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_QueueDislocate
 *   devType,  // IN: RDPA interface type
 *   devId,    // IN: RDPA interface id
 *   qid,      // IN: Queue ID
 *
 *******************************************************************************/

int rdpaCtl_QueueDislocate(
        int devType,
        int devId,
        int qid)
{
    rdpa_drv_ioctl_tm_t tm;

    rdpaCtl_debug("devType %u, devId %u, qid %d", devType, devId, qid);

    memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

    tm.cmd = RDPA_IOCTL_TM_CMD_QUEUE_DISLOCATE;
    tm.dev_type = devType;
    tm.dev_id   = devId;
    tm.q_id     = qid;

    return __sendTmCommand(&tm);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_GetQueueStats
 *   devType,  // IN: RDPA interface type
 *   devId,    // IN: RDPA interface id
 *   dir,      // IN: Tm direction
 *   qid,      // IN: Queue ID
 *   pStats    // OUT: Queue stats
 *
 *******************************************************************************/

int rdpaCtl_GetQueueStats(
        int devType,
        int devId,
        int dir,
        int qid,
        rdpa_stat_1way_t *pStats)
{
   int rc = 0;
   rdpa_drv_ioctl_tm_t tm;

   rdpaCtl_debug("devType %u, devId %u, qid %d", devType, devId, qid);

   memset(&tm, 0, sizeof(rdpa_drv_ioctl_tm_t));

   tm.cmd = RDPA_IOCTL_TM_CMD_GET_QUEUE_STATS;
   tm.dev_type = devType;
   tm.dev_id   = devId;
   tm.dir      = dir;
   tm.q_id     = qid;

   rc = __sendTmCommand(&tm);
   if (!rc) {
      *pStats = tm.qstats;

      rdpaCtl_debug("[Stats] PASSED: packets=%u bytes=%u  DISCARDED: packets=%u bytes=%u",
                    pStats->passed.packets, pStats->passed.bytes,
                    pStats->discarded.packets, pStats->discarded.bytes);
   }

   return rc;
}


/*******************************************************************************
 * This function gets rdpa subsidiary or queue index.
 *
 * Parameters:
 *   qid,           // IN: Queue ID
 *   dir,           // IN: Tm direction
 *   maxQueues,     // IN: max number of queues supported by the device
 *   prio,          // Priority (802.1p scheme, higher value, higher priority)
 *                  // Use qid to look up queue index when prio is set to -1.
 * Return:
 *    rdpa subsidiary or queue index
 *******************************************************************************/

uint32 rdpaCtl_getQueuePrioIndex(
           uint32 qid,
           rdpa_traffic_dir dir,
           uint32 maxQueues,
           int prio)
{
   uint32 queueIndex = 0;

   if (prio == -1)
   {
      rdpaCtl_error("Invalid priority value qid=%lu dir=%d maxQ=%lu prio=%d\n",
        qid, dir, maxQueues, prio);
   }
   else
   {
#ifdef BCM_PON
      queueIndex = maxQueues - (uint32)prio - 1;
#else
      /* DSL platform will always do Q7P7 */
      queueIndex = maxQueues - qid - 1;
#endif
   }

   if (queueIndex >= maxQueues)
   {
      rdpaCtl_error("Invalid input, qid=%lu dir=%d maxQ=%lu prio=%d\n",
                  qid, dir, maxQueues, prio);
      queueIndex = 0;
   }

   return queueIndex;
}

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908) && !defined(BCM63158)
/*******************************************************************************/
/* iptv API                                                                    */
/*******************************************************************************/

/*
 * Static functions
 */

static int __sendIptvCommand(rdpa_drv_ioctl_iptv_t *iptv_p)
{
    return __rdpa_ioctl(RDPA_IOC_IPTV, (uintptr_t)iptv_p);
}

/*
 * Public functions
 */

/*******************************************************************************
 *
 * Function: rdpaCtl_IptvLookupMethodSet
 *   method                                     // IN: iptv lookup method
 *
 *******************************************************************************/

int rdpaCtl_IptvLookupMethodSet(
        rdpa_iptv_lookup_method method)
{
    rdpa_drv_ioctl_iptv_t iptv;

    rdpaCtl_debug("method %d", method);
    
    if (method > iptv_lookup_method_group_ip_src_ip_vid)
    {
        printf("Invalid IPTV lookup method(%d)\n", method);
        return -1;
    }

    memset(&iptv, 0, sizeof(rdpa_drv_ioctl_iptv_t));
    iptv.cmd = RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_SET;
    iptv.method = method;

    return __sendIptvCommand(&iptv);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_IptvLookupMethodGet
 *   method                                     // OUT: iptv lookup method
 *
 *******************************************************************************/

int rdpaCtl_IptvLookupMethodGet(
        rdpa_iptv_lookup_method *method)
{
    rdpa_drv_ioctl_iptv_t iptv;
    int rc;
    
    memset(&iptv, 0, sizeof(rdpa_drv_ioctl_iptv_t));
    iptv.cmd = RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_GET;

    rc = __sendIptvCommand(&iptv);
    if (!rc)
    {
        *method = iptv.method;
        rdpaCtl_debug("method %d", *method);
    }
  
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_IptvPrefixFilterMethodSet
 *   method                                     // IN: iptv prefix filter method
 *
 *******************************************************************************/

int rdpaCtl_IptvPrefixFilterMethodSet(
        rdpa_mcast_filter_method method)
{
    rdpa_drv_ioctl_iptv_t iptv;

    rdpaCtl_debug("method %d", method);
    
    if (method > rdpa_mcast_filter_method_mac_and_ip)
    {
        printf("Invalid IPTV prefix filter method(%d)\n", method);
        return -1;
    }

    memset(&iptv, 0, sizeof(rdpa_drv_ioctl_iptv_t));
    iptv.cmd = RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_SET;
    iptv.filter_method = method;

    return __sendIptvCommand(&iptv);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_IptvPrefixFilterMethodGet
 *   method                                     // OUT: iptv prefix filter method
 *
 *******************************************************************************/

int rdpaCtl_IptvPrefixFilterMethodGet(
        rdpa_mcast_filter_method *method)
{
    rdpa_drv_ioctl_iptv_t iptv;
    int rc;
    
    memset(&iptv, 0, sizeof(rdpa_drv_ioctl_iptv_t));
    iptv.cmd = RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_GET;

    rc = __sendIptvCommand(&iptv);
    if (!rc)
    {
        *method = iptv.filter_method;
        rdpaCtl_debug("method %d", *method);
    }
  
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_IptvEntryAdd
 *   egress_port                                // IN: egress port
 *   key                                        // IN: multicast info 
 *   vlan_action                                // IN: vlan action 
 *   action_vid                                 // IN: translation vid 
 *   index                                      // OUT: iptv channel index
 * 
 *******************************************************************************/

int rdpaCtl_IptvEntryAdd(
        uint32_t egress_port,
        rdpa_drv_ioctl_iptv_key_t *key,
        rdpa_vlan_command  vlan_action,
        uint16_t action_vid,
        uint32_t *index)
{
    rdpa_drv_ioctl_iptv_t iptv;
    int rc;

    rdpaCtl_debug("egress_port: %d", egress_port);
    rdpaCtl_debug("vlan_action: %d", vlan_action);
    rdpaCtl_debug(" action_vid: %d", action_vid);
    
    memset(&iptv, 0, sizeof(rdpa_drv_ioctl_iptv_t));
    iptv.cmd = RDPA_IOCTL_IPTV_CMD_ENTRY_ADD;
    iptv.egress_port = egress_port;
    memcpy(&iptv.entry.key, key, sizeof(rdpa_drv_ioctl_iptv_key_t));
    iptv.entry.vlan.action = vlan_action;
    iptv.entry.vlan.vid = action_vid;

    rc = __sendIptvCommand(&iptv);
    if (!rc)
    {
        *index = iptv.index;
        rdpaCtl_debug(" return index: %d", *index);
    }
   
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_IptvEntryRemove
 *   index                                      // IN: iptv channel index
 *   egress_port                                // IN: egress port
 * 
 *******************************************************************************/

int rdpaCtl_IptvEntryRemove(
        uint32_t index,
        uint32_t egress_port)
{
    rdpa_drv_ioctl_iptv_t iptv;

    rdpaCtl_debug("      innex: %d", index);
    rdpaCtl_debug("egress_port: %d", egress_port);
    
    memset(&iptv, 0, sizeof(rdpa_drv_ioctl_iptv_t));
    iptv.cmd = RDPA_IOCTL_IPTV_CMD_ENTRY_REMOVE;
    iptv.index = index;
    iptv.egress_port = egress_port;

    return __sendIptvCommand(&iptv);
}

/*******************************************************************************
 *
 * Function: rdpaCtl_IptvEntryFlush
 * 
 *******************************************************************************/

int rdpaCtl_IptvEntryFlush(void)
{
    rdpa_drv_ioctl_iptv_t iptv;
    
    memset(&iptv, 0, sizeof(rdpa_drv_ioctl_iptv_t));
    iptv.cmd = RDPA_IOCTL_IPTV_CMD_ENTRY_FLUSH;

    return __sendIptvCommand(&iptv);
}

#endif

/*******************************************************************************/
/* Ingress Classifier  API                                                                    */
/*******************************************************************************/

/*******************************************************************************
 * Function: rdpaCtl_add_classification_rule
 *   rule                                       // IN: classification rule
*******************************************************************************/

int rdpaCtl_add_classification_rule(rdpactl_classification_rule_t *rule, uint8_t *prty)
{
    rdpa_drv_ioctl_ic_t ic;
    int ret;

    rdpaCtl_debug("Rule classifier, dir: %d, type: %d, prty: %d, field_mask: 0x%x, port_mask: 0x%0x", 
        rule->dir, rule->type, rule->prty, rule->field_mask, rule->port_mask);

    rdpaCtl_debug("Class key, src_ip: 0x%0x, dst_ip: 0x%0x, src_port: %d, dst_port: %d, protocol: %d, outer_vid: %d, inner_vid: %d,\
        dst_mac: %02x%02x%02x%02x%02x%02x, src_mac: %02x%02x%02x%02x%02x%02x,\
        etype: 0x%04x, dscp: %d, ingress_port: %d, opbits: %d, ipbits: %d, num_vlan: %d, ipv6_label: 0x%x, otpid: 0x%04x, itpid: 0x%04x", 
        rule->src_ip.ipv4, rule->dst_ip.ipv4, rule->src_port, rule->dst_port, rule->protocol, rule->outer_vid, rule->inner_vid, 
        rule->dst_mac[0], rule->dst_mac[1], rule->dst_mac[2], rule->dst_mac[3], rule->dst_mac[4], rule->dst_mac[5],
        rule->src_mac[0], rule->src_mac[1], rule->src_mac[2], rule->src_mac[3], rule->src_mac[4], rule->src_mac[5],
        rule->etype, rule->dscp, rule->ingress_port_id, rule->outer_pbits, rule->inner_pbits, rule->number_of_vlans, 
        rule->ipv6_label, rule->outer_tpid, rule->inner_tpid);
    
    rdpaCtl_debug("Class result, qos_method: %d, wan_flow: %d, action: %d, forw_mode: %d, egress_port: %d, queue_id: %d, \
        opbit_remark: %d, ipbit_remark: %d, dscp_remark: %d, pbit_to_gem: %d", 
        rule->qos_method, rule->wan_flow, rule->action, rule->forw_mode, rule->egress_port, rule->queue_id,
        rule->opbit_remark, rule->ipbit_remark, rule->dscp_remark, rule->pbit_to_gem);

    memset(&ic, 0, sizeof(rdpa_drv_ioctl_ic_t));

    ic.cmd = RDPA_IOCTL_IC_CMD_ADD_CLASSIFICATION_RULE;
    ic.param.rule = rule;

    ret = __sendIcCommand(&ic);
    *prty = ic.param.prty;
    rdpaCtl_debug("return ic priority: %d\n", *prty);

    return ret;
}

/*******************************************************************************
 * Function: rdpaCtl_del_classification_rule
 *   rule                                       // IN: classification rule
*******************************************************************************/

int rdpaCtl_del_classification_rule(rdpactl_classification_rule_t *rule)
{
    rdpa_drv_ioctl_ic_t ic;

    rdpaCtl_debug("Rule classifier, dir: %d, type: %d, prty: %d, field_mask: 0x%x, port_mask: 0x%0x", 
        rule->dir, rule->type, rule->prty, rule->field_mask, rule->port_mask);

    rdpaCtl_debug("Class key, src_ip: 0x%0x, dst_ip: 0x%0x, src_port: %d, dst_port: %d, protocol: %d, outer_vid: %d, inner_vid: %d,\
        dst_mac: %02x%02x%02x%02x%02x%02x, src_mac: %02x%02x%02x%02x%02x%02x,\
        etype: 0x%04x, dscp: %d, ingress_port: %d, opbits: %d, ipbits: %d, num_vlan: %d, ipv6_label: 0x%x, otpid: 0x%04x, itpid: 0x%04x", 
        rule->src_ip.ipv4, rule->dst_ip.ipv4, rule->src_port, rule->dst_port, rule->protocol, rule->outer_vid, rule->inner_vid, 
        rule->dst_mac[0], rule->dst_mac[1], rule->dst_mac[2], rule->dst_mac[3], rule->dst_mac[4], rule->dst_mac[5],
        rule->src_mac[0], rule->src_mac[1], rule->src_mac[2], rule->src_mac[3], rule->src_mac[4], rule->src_mac[5],
        rule->etype, rule->dscp, rule->ingress_port_id, rule->outer_pbits, rule->inner_pbits, rule->number_of_vlans, 
        rule->ipv6_label, rule->outer_tpid, rule->inner_tpid);
    
    rdpaCtl_debug("Class result, qos_method: %d, wan_flow: %d, action: %d, forw_mode: %d, egress_port: %d, queue_id: %d, \
        opbit_remark: %d, ipbit_remark: %d, dscp_remark: %d, pbit_to_gem: %d", 
        rule->qos_method, rule->wan_flow, rule->action, rule->forw_mode, rule->egress_port, rule->queue_id,
        rule->opbit_remark, rule->ipbit_remark, rule->dscp_remark, rule->pbit_to_gem);

    memset(&ic, 0, sizeof(rdpa_drv_ioctl_ic_t));

    ic.cmd = RDPA_IOCTL_IC_CMD_DEL_CLASSIFICATION_RULE;
    ic.param.rule = rule;

    return __sendIcCommand(&ic);
}


/*******************************************************************************
 * Function: rdpaCtl_add_classification
 *   rule                                       // IN: classification rule
*******************************************************************************/

int rdpaCtl_add_classification(rdpactl_classification_rule_t *rule)
{
    rdpa_drv_ioctl_ic_t ic;

    rdpaCtl_debug("Rule classifier, dir: %d, type: %d, prty: %d, field_mask: 0x%x, port_mask: 0x%0x", 
        rule->dir, rule->type, rule->prty, rule->field_mask, rule->port_mask);

    memset(&ic, 0, sizeof(rdpa_drv_ioctl_ic_t));

    ic.cmd = RDPA_IOCTL_IC_CMD_ADD;
    ic.param.rule = rule;

    return __sendIcCommand(&ic);
}

/*******************************************************************************
 * Function: rdpaCtl_del_classification
 *   rule                                       // IN: classification rule
*******************************************************************************/
int rdpaCtl_del_classification(rdpactl_classification_rule_t *rule)
{
    rdpa_drv_ioctl_ic_t ic;

    rdpaCtl_debug("Rule classifier, dir: %d, type: %d, prty: %d, field_mask: 0x%x, port_mask: 0x%0x", 
        rule->dir, rule->type, rule->prty, rule->field_mask, rule->port_mask);

    memset(&ic, 0, sizeof(rdpa_drv_ioctl_ic_t));

    ic.cmd = RDPA_IOCTL_IC_CMD_DEL;
    ic.param.rule = rule;

    return __sendIcCommand(&ic);
}

/*******************************************************************************
 * Function: rdpaCtl_find_classification
 *   rule                                       // IN: classification rule
*******************************************************************************/

int rdpaCtl_find_classification(rdpactl_classification_rule_t *rule)
{
    rdpa_drv_ioctl_ic_t ic;
    int ret;

    rdpaCtl_debug("Rule classifier, dir: %d, type: %d, prty: %d, field_mask: 0x%x, port_mask: 0x%0x", 
        rule->dir, rule->type, rule->prty, rule->field_mask, rule->port_mask);

    memset(&ic, 0, sizeof(rdpa_drv_ioctl_ic_t));

    ic.cmd = RDPA_IOCTL_IC_CMD_FIND;
    ic.param.rule = rule;

    ret = __sendIcCommand(&ic);
    rdpaCtl_debug("return ic priority: %d\n", *prty);

    return ret;
}



/*********************************************************************************************/
/*                                     Generic System accessors                              */
/*********************************************************************************************/

int rdpaCtl_get_wan_type(rdpa_if wan_if, rdpa_wan_type *wanType) 
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.param.rdpa_if = wan_if;
    sys.cmd = RDPA_IOCTL_SYS_CMD_WANTYPE_GET;

    rc = __sendSysCommand(&sys);
    if (!rc)
    {
        *wanType = sys.param.wan_type;
        rdpaCtl_debug("wantype %d", *wanType);
    }
  
    return rc;
}


int rdpaCtl_get_in_tpid(uint16_t *inTpid)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_IN_TPID_GET;

    rc = __sendSysCommand(&sys);
    if (!rc)
    {
        *inTpid = sys.param.inner_tpid;
        rdpaCtl_debug("inner tipid: 0x%04x", *inTpid);
    }
  
    return rc;
}


int rdpaCtl_set_in_tpid(uint16_t inTpid)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_IN_TPID_SET;
    sys.param.inner_tpid = inTpid;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("set inner tpid to 0x%04x failed", inTpid);
    }
  
    return rc;
}


int rdpaCtl_get_out_tpid(uint16_t *outTpid)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_OUT_TPID_GET;

    rc = __sendSysCommand(&sys);
    if (!rc)
    {
        *outTpid = sys.param.outer_tpid;
        rdpaCtl_debug("outer tpid: 0x%04x", *outTpid);
    }
  
    return rc;
}


int rdpaCtl_set_out_tpid(uint16_t outTpid)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_OUT_TPID_SET;
    sys.param.outer_tpid= outTpid;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("set outer tpid to 0x%04x failed", outTpid);
    }
  
    return rc;
}

int rdpaCtl_set_detect_tpid(uint16_t tpid, BOOL is_inner)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_DETECT_TPID_SET;
    sys.param.detect_tpid.is_inner = is_inner;
    sys.param.detect_tpid.tpid = tpid;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("set detect tpid[%s] to 0x%04x failed",is_inner?"inner":"outer", tpid);
    }
  
    return rc;
}

int rdpaCtl_set_epon_mode(rdpa_epon_mode eponMode)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_EPON_MODE_SET;
    sys.param.epon_mode = eponMode;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("set epon mode %u failed", eponMode);
    }
  
    return rc;
}

int rdpaCtl_get_epon_mode(rdpa_epon_mode *eponMode)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_EPON_MODE_GET;

    rc = __sendSysCommand(&sys);
    if (!rc)
    {
        *eponMode = sys.param.epon_mode;
        rdpaCtl_debug("get epon mode %u success", *eponMode);
    }
  
    return rc;
}


int rdpaCtl_get_epon_status(BOOL *enable)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_EPON_STATUS_GET;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("get epon status failed");
    }
    else
    {
        *enable = sys.param.epon_enable;
    }
  
    return rc;
}

int rdpaCtl_set_always_tpid(uint16_t alwaysTpid)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_ALWAYS_TPID_SET;
    sys.param.always_tpid = alwaysTpid;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("set always tpid to 0x%04x failed", alwaysTpid);
    }
  
    return rc;
}

int rdpaCtl_get_force_dscp(uint16_t dir, BOOL *enable)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_FORCE_DSCP_GET;
    sys.param.force_dscp.dir = dir;

    rc = __sendSysCommand(&sys);
    if (!rc)
    {
        *enable = sys.param.force_dscp.enable;
        rdpaCtl_debug("dir %d force_dscp: %u", dir, *enable);
    }
  
    return rc;
}

int rdpaCtl_set_force_dscp(uint16_t dir, BOOL enable)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_FORCE_DSCP_SET;
    sys.param.force_dscp.dir = dir;
    sys.param.force_dscp.enable = enable;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("set dir %d force dscp to %u failed", dir, enable);
    }
  
    return rc;
}

int rdpaCtl_set_sys_car_mode(BOOL enable)
{
    int rc;
    rdpa_drv_ioctl_sys_t sys;

    memset(&sys, 0, sizeof(rdpa_drv_ioctl_sys_t));
    sys.cmd = RDPA_IOCTL_SYS_CMD_CAR_MODE_SET;
    sys.param.car_mode = enable;

    rc = __sendSysCommand(&sys);
    if (rc)
    {
        rdpaCtl_debug("set car mode %d failed", enable);
    }
  
    return rc;
}

int rdpaCtl_set_port_sa_limit(rdpa_if rdpaIf, uint16_t max_limit)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    memset(&port, 0, sizeof(rdpa_drv_ioctl_port_t));
    port.cmd = RDPA_IOCTL_PORT_CMD_SA_LIMIT_SET;
    port.port_idx = rdpaIf;
    port.param.sa_limit.max_sa = max_limit;

    rc = __sendPortCommand(&port);
    if (rc)
    {
        rdpaCtl_debug("set port %d sa limit to %d failed", rdpaIf, max_limit);
    }

    return rc;
}

int rdpaCtl_get_port_sa_limit(rdpa_if rdpaIf, uint16_t *max_limit)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    memset(&port, 0, sizeof(rdpa_drv_ioctl_port_t));
    port.cmd = RDPA_IOCTL_PORT_CMD_SA_LIMIT_GET;
    port.port_idx = rdpaIf;

    rc = __sendPortCommand(&port);
    if (rc)
    {
        rdpaCtl_debug("get port %d sa limit failed", rdpaIf);
    }
    else
    {
        *max_limit = port.param.sa_limit.max_sa;
    }

    return rc;
}

int rdpaCtl_set_port_sal_miss_action(rdpa_if rdpaIf, rdpa_forward_action act)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    rc = rdpaCtl_get_port_param(rdpaIf, &port.param);
    if (rc)
    {
        rdpaCtl_debug("get port %d sal miss action failed", rdpaIf);
    }
    else
    {
        port.param.sal_miss_action = act;
        rc = rdpaCtl_set_port_param(rdpaIf, &port.param);
    }

    return rc;
}

int rdpaCtl_get_port_sal_miss_action(rdpa_if rdpaIf, rdpa_forward_action *act)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    rc = rdpaCtl_get_port_param(rdpaIf, &port.param);
    if (rc)
    {
        rdpaCtl_debug("get port %d sal miss action failed", rdpaIf);
    }
    else
    {
        *act = port.param.sal_miss_action;
    }

    return rc;
}

int rdpaCtl_set_port_dal_miss_action(rdpa_if rdpaIf, rdpa_forward_action act)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    rc = rdpaCtl_get_port_param(rdpaIf, &port.param);
    if (rc)
    {
        rdpaCtl_debug("get port %d dal miss action failed", rdpaIf);
    }
    else
    {
        port.param.dal_miss_action = act;
        rc = rdpaCtl_set_port_param(rdpaIf, &port.param);
    }

    return rc;
}

int rdpaCtl_get_port_dal_miss_action(rdpa_if rdpaIf, rdpa_forward_action *act)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    rc = rdpaCtl_get_port_param(rdpaIf, &port.param);
    if (rc)
    {
        rdpaCtl_debug("get port %d dal miss action failed", rdpaIf);
    }
    else
    {
        *act = port.param.dal_miss_action;
    }    
  
    return rc;
}

int rdpaCtl_set_port_param(rdpa_if rdpaIf, 
  rdpa_drv_ioctl_port_param_t *portParam)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    memset(&port, 0, sizeof(rdpa_drv_ioctl_port_t));
    port.cmd = RDPA_IOCTL_PORT_CMD_PARAM_SET;
    port.port_idx = rdpaIf;
    memcpy(&port.param, portParam, sizeof(rdpa_drv_ioctl_port_param_t));

    rc = __sendPortCommand(&port);
    if (rc)
    {
        rdpaCtl_debug("set port %d param failed", rdpaIf);
    }

    return rc;
}

int rdpaCtl_get_port_param(rdpa_if rdpaIf,
  rdpa_drv_ioctl_port_param_t *portParam)
{
    int rc;
    rdpa_drv_ioctl_port_t port;

    memset(&port, 0, sizeof(rdpa_drv_ioctl_port_t));
    port.cmd = RDPA_IOCTL_PORT_CMD_PARAM_GET;
    port.port_idx = rdpaIf;

    rc = __sendPortCommand(&port);
    if (rc)
    {
        rdpaCtl_debug("get port %d param failed", rdpaIf);
    }
    else
    {
        memcpy(portParam, &port.param, sizeof(rdpa_drv_ioctl_port_param_t));
    }

    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_BrExist
 *   brId,                                      // IN: br id
 *   found,                                     // OUT: exist or not
 *
 *******************************************************************************/
int rdpaCtl_BrExist(uint8_t brId, BOOL *found)
{
    int rc = 0;
    rdpa_drv_ioctl_br_t br;

    rdpaCtl_debug("exist or not brId %u,", brId);

    *found = FALSE;
    memset(&br, 0, sizeof(rdpa_drv_ioctl_br_t));

    br.cmd = RDPA_IOCTL_BR_CMD_FIND_OBJ;
    br.br_index = brId;

    rc = __sendBrCommand(&br);

    if(!rc)
    {
        *found = br.found;
        if(br.found)
        {
            rdpaCtl_debug("br %u exist", brId);
        }
        else
        {
            rdpaCtl_debug("br %u not exist", brId);
        }
    }
    return rc;
}


/*******************************************************************************
 *
 * Function: rdpaCtl_BrLocalSwitchSet
 *   brId,                                 // IN: br id
 *   local_switch,                      // IN: mode to set
 *
 *******************************************************************************/
int rdpaCtl_BrLocalSwitchSet(uint8_t brId, BOOL local_switch)
{
    rdpa_drv_ioctl_br_t br;

    rdpaCtl_debug("set bridge %u local switch %u", brId, (uint8_t)local_switch);

    memset(&br, 0, sizeof(rdpa_drv_ioctl_br_t));

    br.cmd = RDPA_IOCTL_BR_CMD_LOCAL_SWITCH_SET;
    br.br_index = brId;
    br.local_switch = local_switch;

    return __sendBrCommand(&br);
}


int rdpaCtl_LlidCreate(uint8_t llid_index)
{
    rdpa_drv_ioctl_llid_t llidPara;
    memset(&llidPara, 0, sizeof(rdpa_drv_ioctl_llid_t));

    llidPara.cmd = RDPA_IOCTL_LLID_CMD_NEW;
    llidPara.llid_index = llid_index;
    
    return __sendLlidCommand(&llidPara);
}

int rdpaCtl_DsWanUdpFilterAdd(rdpactl_ds_wan_udp_filter_t *filter_p)
{
    rdpa_drv_ioctl_ds_wan_udp_filter_t ds_wan_udp_filter;
    int ret;

    rdpaCtl_debug();

    ds_wan_udp_filter.cmd = RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_ADD;

    ds_wan_udp_filter.filter = *filter_p;

    ret = __sendDsWanUdpFilterCommand(&ds_wan_udp_filter);

    filter_p->index = ds_wan_udp_filter.filter.index;

    return ret;
}

int rdpaCtl_DsWanUdpFilterDelete(long index)
{
    rdpa_drv_ioctl_ds_wan_udp_filter_t ds_wan_udp_filter;

    rdpaCtl_debug();

    ds_wan_udp_filter.cmd = RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_DELETE;

    ds_wan_udp_filter.filter.index = index;

    return __sendDsWanUdpFilterCommand(&ds_wan_udp_filter);
}

int rdpaCtl_DsWanUdpFilterGet(rdpactl_ds_wan_udp_filter_t *filter_p)
{
    rdpa_drv_ioctl_ds_wan_udp_filter_t ds_wan_udp_filter;
    int ret;

    rdpaCtl_debug();

    ds_wan_udp_filter.cmd = RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_GET;

    ds_wan_udp_filter.filter.index = filter_p->index;

    ret = __sendDsWanUdpFilterCommand(&ds_wan_udp_filter);

    *filter_p = ds_wan_udp_filter.filter;

    return ret;
}

/* Set -1 to disble remark */
int rdpaCtl_RdpaMwMCastSet(int dscp_val)
{
    rdpaCtl_debug();
    rdpaCtl_debug("Setting rdpa_mw multicast DSCP remark with %d,", dscp_val);

    return __rdpa_ioctl(RDPA_IOC_RDPA_MW_SET_MCAST_DSCP_REMARK, dscp_val);
}

int rdpaCtl_time_sync_init(void)
{
    return __rdpa_ioctl(RDPA_IOC_TIME_SYNC, 0);
}

int rdpaCtl_filter_entry_create(rdpa_filter_key_t *key, rdpa_filter_ctrl_t *ctrl)
{
    rdpa_drv_ioctl_filter_t filter;

    rdpaCtl_debug();

    filter.cmd = RDPA_IOCTL_FILTER_CMD_ADD_ENTRY;
    filter.param.key.filter = key->filter;
    filter.param.key.ports = key->ports;
    filter.param.ctrl.action = ctrl->action;
    filter.param.ctrl.enabled = ctrl->enabled;
    return __sendFilterCommand(&filter);
}

int rdpaCtl_filter_set_global_cfg(rdpa_filter_global_cfg_t *global_cfg)
{
    rdpa_drv_ioctl_filter_t filter;
    rdpaCtl_debug();

    filter.cmd = RDPA_IOCTL_FILTER_CMD_GLOBAL_CFG;
    filter.param.global_cfg.ls_enabled = global_cfg->ls_enabled;

    return __sendFilterCommand(&filter);
}

int rdpaCtl_filter_etyp_udef_cfg(uint32_t udef_inx, uint32_t udef_val)
{
    rdpa_drv_ioctl_filter_t filter;

    rdpaCtl_debug();

    filter.cmd = RDPA_IOCTL_FILTER_CMD_ETYPE_UDEF_CFG;
    filter.param.udef_inx = udef_inx;
    filter.param.udef_val = udef_val;
    return __sendFilterCommand(&filter);
}

int rdpaCtl_filter_tpid_vals_cfg(rdpa_filter_tpid_vals_t *tpid_vals, BOOL direction)
{
    rdpa_drv_ioctl_filter_t filter;

    rdpaCtl_debug();

    filter.cmd = RDPA_IOCTL_FILTER_CMD_TPID_VALS_CFG;
    filter.param.tpid_vals.val_ds = tpid_vals->val_ds;
    filter.param.tpid_vals.val_us = tpid_vals->val_us;
    filter.param.tpid_direction = direction;
    return __sendFilterCommand(&filter);
}

int rdpaCtl_filter_oui_cfg(rdpa_filter_oui_val_key_t *oui_val_key, uint32_t oui_val)
{
    rdpa_drv_ioctl_filter_t filter;

    rdpaCtl_debug();

    filter.cmd = RDPA_IOCTL_FILTER_CMD_OUI_CFG;
    filter.param.oui_val_key.ports = oui_val_key->ports;
    filter.param.oui_val_key.val_id = oui_val_key->val_id;
    filter.param.oui_val = oui_val;
    return __sendFilterCommand(&filter);
}

int rdpaCtl_filter_stats_get(rdpa_filter_stats_key_t *stats_params, int64_t *stats_val)
{
    rdpa_drv_ioctl_filter_t filter;
    int ret;

    rdpaCtl_debug();

    filter.cmd = RDPA_IOCTL_FILTER_CMD_GET_STAT;
    filter.param.stats_params.dir = stats_params->dir;
    filter.param.stats_params.filter = stats_params->filter;
    ret = __sendFilterCommand(&filter);

    *stats_val = filter.param.stats_val;

    return ret;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_DscpToPitGet
 *   found,                                   // OUT: found or not
 *   map,                                     // OUT: dscp tp pbit map
 *
 *******************************************************************************/
int rdpaCtl_DscpToPitGet(BOOL *found, tmctl_dscpToPbitCfg_t *map)
{
    int i;
    int rc = 0;
    rdpa_drv_ioctl_dscp_to_pbit_t dscp_to_pbit;
    
    rdpaCtl_debug("rdpaCtl_DscpToPitGet");
    
    memset(&dscp_to_pbit, 0, sizeof(rdpa_drv_ioctl_dscp_to_pbit_t));

    dscp_to_pbit.found = FALSE;
    dscp_to_pbit.cmd = RDPA_IOCTL_D_TO_P_CMD_GET;
    rc = __sendDscpToPbitCommand(&dscp_to_pbit);

    if (!rc)
    {
        *found = dscp_to_pbit.found;
        if (dscp_to_pbit.found)
        {
            for (i = 0; i < TOTAL_DSCP_NUM; i++)
            {
                map->dscp[i] = dscp_to_pbit.dscp_pbit_map[i];
                rdpaCtl_debug(
                    "get dscp %d to pbit map %u", i, dscp_to_pbit.dscp_pbit_map[i]);
            }
        }
    }
    
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_DscpToPitSet
 *   map,                                     // IN: dscp tp pbit map
 *
 *******************************************************************************/
int rdpaCtl_DscpToPitSet(tmctl_dscpToPbitCfg_t map)
{
    int i;
    int rc = 0;
    rdpa_drv_ioctl_dscp_to_pbit_t dscp_to_pbit;
    
    rdpaCtl_debug("rdpaCtl_DscpToPitSet");
    
    memset(&dscp_to_pbit, 0, sizeof(rdpa_drv_ioctl_dscp_to_pbit_t));

    dscp_to_pbit.cmd = RDPA_IOCTL_D_TO_P_CMD_SET;
    
    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        dscp_to_pbit.dscp_pbit_map[i] = map.dscp[i];
    }
     
    rc = __sendDscpToPbitCommand(&dscp_to_pbit);
    
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_PitToQueueGet
 *   devType,                          // IN: RDPA device type
 *   devId,                              // IN: RDPA device id
 *   found,                              // OUT: found or not
 *   pbit_q_map,                      // OUT: pbit to q map
 *
 *******************************************************************************/
int rdpaCtl_PitToQueueGet(int devType, 
                        int devId,
                        BOOL *found,
                        tmctl_pbitToQCfg_t *pbit_q_map)
{
    int i;
    int rc = 0;
    rdpa_drv_ioctl_pbit_to_q_t pbit_to_q;
    
    rdpaCtl_debug(
        "rdpaCtl_PitToQueueGet devType [%d] devId [%d]", devType, devId);
    
    memset(&pbit_to_q, 0, sizeof(rdpa_drv_ioctl_pbit_to_q_t));

    pbit_to_q.dev_type = devType;
    pbit_to_q.dev_id = devId;
    pbit_to_q.cmd = RDPA_IOCTL_P_TO_Q_CMD_GET;

    rc = __sendPbitToQCommand(&pbit_to_q);

    if (!rc)
    {
        *found = pbit_to_q.found;
        if (pbit_to_q.found)
        {
            for (i = 0; i < TOTAL_PBIT_NUM; i++)
            {
                pbit_q_map->pbit[i] = pbit_to_q.pbit_q_map[i];
                rdpaCtl_debug(
                    "get pbit %d to q map %u", i, pbit_to_q.pbit_q_map[i]);
            }
        }
    }
    
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_PitToQueueSet
 *   devType,                          // IN: RDPA device type
 *   devId,                              // IN: RDPA device id
 *   pbit_q_map,                      // IN: pbit to q map
 *
 *******************************************************************************/
int rdpaCtl_PitToQueueSet(int devType, 
                          int devId, 
                          tmctl_pbitToQCfg_t pbit_q_map)
{
    int i;
    int rc = 0;
    rdpa_drv_ioctl_pbit_to_q_t pbit_to_q;
    
    rdpaCtl_debug(
        "rdpaCtl_PitToQueueSet devType [%d] devId [%d]", devType, devId);

    for (i = 0; i < TOTAL_PBIT_NUM; i++)
    {
        rdpaCtl_debug("pbit %d map to q %u", i, pbit_to_q.pbit_q_map[i]);
    }
    
    memset(&pbit_to_q, 0, sizeof(rdpa_drv_ioctl_pbit_to_q_t));

    pbit_to_q.dev_type = devType;
    pbit_to_q.dev_id = devId;
    pbit_to_q.cmd = RDPA_IOCTL_P_TO_Q_CMD_SET;

    for (i = 0; i < TOTAL_PBIT_NUM; i++)
    {
        pbit_to_q.pbit_q_map[i] = pbit_q_map.pbit[i];
    }
   
    rc = __sendPbitToQCommand(&pbit_to_q);
    
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_PktBasedQosGet
 *   dir,                                 // IN: RDPA direction
 *   type,                              // IN: RDPA qos type
 *   enable,                           // OUT: enable or not
 *
 *******************************************************************************/
int rdpaCtl_PktBasedQosGet(int dir, int type, BOOL *enable)
{
    int rc = 0;
    rdpa_drv_ioctl_misc_t misc_cfg;
    
    rdpaCtl_debug(
        "rdpaCtl_PktBasedQosGet dir [%d] type [%d]", dir, type);
    
    memset(&misc_cfg, 0, sizeof(rdpa_drv_ioctl_misc_t));

    misc_cfg.dir = dir;
    misc_cfg.type = type;
    misc_cfg.cmd = RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_GET;

    rc = __sendMiscCommand(&misc_cfg);

    if (!rc)
    {
        *enable = misc_cfg.enable;
    }
    
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpaCtl_PktBasedQosSet
 *   dir,                                 // IN: RDPA direction
 *   type,                              // IN: RDPA qos type
 *   enable,                           // IN: enable or not
 *
 *******************************************************************************/
int rdpaCtl_PktBasedQosSet(int dir, int type, BOOL enable)
{
    int rc = 0;
    rdpa_drv_ioctl_misc_t misc_cfg;
    
    rdpaCtl_debug(
        "rdpaCtl_PktBasedQosSet dir [%d] type [%d] en [%d]", dir, type, enable);
    
    memset(&misc_cfg, 0, sizeof(rdpa_drv_ioctl_misc_t));

    misc_cfg.dir = dir;
    misc_cfg.type = type;
    misc_cfg.enable = enable;
    misc_cfg.cmd = RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_SET;

    rc = __sendMiscCommand(&misc_cfg);

    return rc;
}

int rdpaCtl_get_svc_q_mode(int *enable)
{
    int rc;
    rdpa_drv_ioctl_tm_t tm;

    memset(&tm, 0, sizeof(tm));
    tm.cmd = RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_GET;

    rc = __sendTmCommand(&tm);
    if (!rc)
    {
        *enable = tm.service_queue;
        rdpaCtl_debug("get service queue mode: %u", *enable);
    }

    return rc;
}

int rdpaCtl_set_svc_q_mode(int enable)
{
    int rc;
    rdpa_drv_ioctl_tm_t tm;

    memset(&tm, 0, sizeof(tm));
    tm.cmd = RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_SET;
    tm.service_queue = enable;

    rc = __sendTmCommand(&tm);
    if (rc)
    {
        rdpaCtl_debug("set service queue mode %d failed", enable);
    }

    return rc;
}

