/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include "os_defs.h"

#include "tmctl_rdpa.h"
#include "ethswctl_api.h"
#if defined(SUPPORT_DPI)
#include <bcmdpi.h>
#endif

#define INVALID_ID   -1

#define getDir(devType, rdpaIf) \
  ((((devType == RDPA_IOCTL_DEV_PORT) && (!rdpa_if_is_wan(rdpaIf))) ||\
    (devType == RDPA_IOCTL_DEV_NONE))? rdpa_dir_ds : rdpa_dir_us)
#ifdef BCM_RDP
#define getDefaultQueueSize(devType, dir) \
  ((devType == RDPA_IOCTL_DEV_TCONT) ? TMCTL_DEF_TCONT_Q_SZ :\
  ((devType == RDPA_IOCTL_DEV_LLID) ? TMCTL_DEF_LLID_Q_SZ :\
  ((devType == RDPA_IOCTL_DEV_PORT && dir == rdpa_dir_us) ? \
  TMCTL_DEF_ETH_Q_SZ_US: TMCTL_DEF_ETH_Q_SZ_DS)))
#else
#define getDefaultQueueSize(devType, dir) \
  ((dir == rdpa_dir_us) ? TMCTL_DEF_ETH_Q_SZ_US: TMCTL_DEF_ETH_Q_SZ_DS)
#endif /* #ifdef BCM_RDP */
#define convertDropAlg(tmDropAlg) \
  ((tmDropAlg == TMCTL_DROP_DT) ? rdpa_tm_drop_alg_dt :\
  ((tmDropAlg == TMCTL_DROP_RED) ? rdpa_tm_drop_alg_red :\
  ((tmDropAlg == TMCTL_DROP_WRED) ? rdpa_tm_drop_alg_wred :\
  rdpa_tm_drop_alg_dt)))
#define isDsEthPort(devType, dir) \
  ((dir == rdpa_dir_ds) && (devType == RDPA_IOCTL_DEV_PORT))

/* Local functions */

static tmctl_ret_e addQueue(int devType, int rdpaIf, rdpa_traffic_dir dir,
                            int parentTmId, int qid, int qsize,
                            rdpa_tm_sched_mode tmMode, int prioIndex, int weight,
                            int minBufs, int minRate, int shapingRate, int burst,
                            BOOL bestEffort, BOOL qClean);

static tmctl_ret_e addSvcQueue(int parentTmId, int qid, int qsize,
                               int prioIndex, int shapingRate, int burst);

static tmctl_ret_e delQueue(int devType, int rdpaIf, rdpa_traffic_dir dir,
                            int tmId, int qid);

static tmctl_ret_e checkPortQueueSched(int maxQueues, int maxSpQueues,
                                       tmctl_portQcfg_t* portQcfg_p);

#if defined(SUPPORT_DPI)
extern int rdp_add_us_dpi_queues(int parent_tm_id, tmctl_queueCfg_t *parent_queue);
#endif

/*
* 6858 CTC mode requires the following egress_tm model:
* egress_tm_0 (level-group, sub[0-6])----->egress_tm_1 (level-queue, queue prio0)
*                                                   |------>egress_tm_2 (level-queue, queue prio1)
*                                                   .....
*                                                   |------>egress_tm_6 (level-queue, queue prio5)
*                                                   |------>egress_tm_7 (level-queue, queue prio6 / queue prio7)
* queue prio7 need to be associated to the sub_tm together with queue prio6.
*/
tmctl_ret_e addQueueHandleEponCtcCase(int devType, int rdpaIf,
                     rdpa_traffic_dir dir, int parentTmId, int qid, int qsize,
                     rdpa_tm_sched_mode tmMode, int prioIndex, int weight, int minBufs,
                     int minRate, int shapingRate, int burst, BOOL bestEffort, BOOL qClean)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int rc;
   int tmId;
   int drvPrioIndex;
   int qShapingRate;
   static int tmId_cache = INVALID_ID;

   tmctl_debug("rdpaIf=%d parentTmId=%d dir=%d qid=%d qsize=%d sched=%d wt=%d min_kbps=%d shaping_kbps=%d",
               rdpaIf, parentTmId, dir, qid, qsize, tmMode, weight, minRate, shapingRate);

   tmctl_debug("Configure rate control for qid %d rdpaIf=%d parentTmId=%d prio=%d wt=%d",
                  qid, rdpaIf, parentTmId, prioIndex, weight);

   tmId = INVALID_ID;
   if (prioIndex == 7 && tmId_cache != INVALID_ID)
   {
      /* re-use tm associated with prio6*/
      tmId = tmId_cache;
      drvPrioIndex = 1;
   }
   else
   {
      rc = rdpaCtl_TmConfig(devType,
                            rdpaIf,
                            parentTmId,               /* parent tm_id */
                            dir,
                            rdpa_tm_level_queue,      /* queue tm level */
                            prioIndex == 6 ? rdpa_tm_sched_sp : rdpa_tm_sched_disabled,   /* rate control */
                            prioIndex,                /* TM priority. For parentTm.subsidiary[index] */
                            weight,                   /* TM weight */
                            minRate,                  /* mininum rate */
                            prioIndex == 6 ? 0 : shapingRate,              /* shaping rate */
                            burst,                    /* burst */
                            &tmId);
      if (rc || (tmId < 0))
      {
         tmctl_error("rdpaCtl_TmConfig ERROR! rdpaIf=%d parentTmId=%d prio=%d wt=%d tmId=%d rc=%d",
                  rdpaIf, parentTmId, prioIndex, weight, tmId, rc);
         return TMCTL_ERROR;
      }
      if (prioIndex == 6)
      {
         tmId_cache = tmId;
      }
      tmctl_debug("New rate control tmId=%d", tmId);
      drvPrioIndex = 0;
   }

   
   /* Configure the queue. */
   tmctl_debug("Configure Runner queue: tmId=%d qid=%d weight=%d", tmId, qid, weight);
   
   if((prioIndex == 6 || prioIndex == 7) && (qsize > 0))
   {
       qShapingRate = shapingRate;
   }
   else
   {
       qShapingRate = 0;
   }
   
   if ((rc = rdpaCtl_QueueConfig(devType,
                                 rdpaIf,
                                 tmId,         /* Tm ID */
                                 qid,
                                 drvPrioIndex,
                                 dir,
                                 qsize,
                                 weight,
                                 minBufs,
                                 qShapingRate,
                                 bestEffort,   /* best effort */
                                 qClean)))
   {
      tmctl_error("rdpaCtl_QueueConfig ERROR! rdpaIf=%d qid=%d size=%d sched=%d rc=%d",
                   rdpaIf, qid, qsize, tmMode, rc);

      if (tmMode == rdpa_tm_sched_disabled)
      {
         tmctl_debug("Remove the just added rate control tm. tmId=%d", tmId);

         if ((rc = rdpaCtl_TmRemove(tmId, dir, devType, rdpaIf)))
         {
            tmctl_error("rdpaCtl_TmRemove ERROR! tmId=%d rc=%d", tmId, rc);
         }
      }
      ret = TMCTL_ERROR;
   }

   return ret;
}

/* ----------------------------------------------------------------------------
 * This function adds a runner queue to the port. If the port supports
 * queue shaping or if the queue is WRR, the queue shaper TM will
 * also be added.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    dir (IN) tm direction
 *    parentTmId (IN) If queue shaper tm is needed, it is the
 *                    parent tm of the queue shaper tm. Otherwise,
 *                    it will be used as the tm for the queue.
 *    qid (IN) queue ID
 *    qsize (IN) queue size
 *    tmMode (IN) scheduler mode for this queue
 *    prioIndex (IN) queue priority index
 *    weight (IN) queue weight
 *    minRate (IN) queue mininum shaping rate in kbps.
 *    shapingRate (IN) queue maximum shaping rate in kbps.
 *    burst (IN) queue shaping burst size in bytes
 *    bestEffort (IN) queue is best-effort queue
 *    qClean (IN) hidden queue clean up flag
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e addQueue(int devType, int rdpaIf, rdpa_traffic_dir dir,
                     int parentTmId, int qid, int qsize,
                     rdpa_tm_sched_mode tmMode, int prioIndex, int weight, int minBufs,
                     int minRate, int shapingRate, int burst, BOOL bestEffort,
                     BOOL qClean)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int rc;
   int tmId;
   int drvPrioIndex;

   tmctl_debug("rdpaIf=%d parentTmId=%d dir=%d qid=%d qsize=%d sched=%d wt=%d min_kbps=%d shaping_kbps=%d",
               rdpaIf, parentTmId, dir, qid, qsize, tmMode, weight, minRate, shapingRate);

   if (tmMode == rdpa_tm_sched_disabled)
   {
#ifdef BCM_XRDP
      // TODO: We need a runtime way to figure this is EPON CTC case.
      // Currently, we use the above macro together with
      // if (tmMode == rdpa_tm_sched_disabled)
      // to assume this is EPON CTC case.
      rdpa_wan_type wan_type = rdpa_wan_none;
      rdpa_if  wan_if = rdpa_wan_type_to_if(rdpa_wan_epon); /* Ok to assume rdpa_if_wanX is same for epon/xepon */

      if (rdpaCtl_get_wan_type(wan_if, &wan_type) == 0 && (wan_type == rdpa_wan_epon || wan_type == rdpa_wan_xepon))
      {
          //Check if ae mode is not enabled
          BOOL epon_enable = FALSE;

          ret = rdpaCtl_get_epon_status(&epon_enable);
          if(!ret && (epon_enable == TRUE))
          {
              return addQueueHandleEponCtcCase(devType, rdpaIf, dir, parentTmId, qid,
                  qsize, tmMode, prioIndex, weight, minBufs, minRate, shapingRate, burst, bestEffort, qClean);
          }
      }
#endif      

      /* Configure the rate control tm. */
      tmctl_debug("Configure rate control for qid %d rdpaIf=%d parentTmId=%d prio=%d wt=%d",
                  qid, rdpaIf, parentTmId, prioIndex, weight);

      tmId = INVALID_ID;
      rc = rdpaCtl_TmConfig(devType,
                            rdpaIf,
                            parentTmId,               /* parent tm_id */
                            dir,
                            rdpa_tm_level_queue,      /* queue tm level */
                            rdpa_tm_sched_disabled,   /* rate control */
                            prioIndex,                /* TM priority. For parentTm.subsidiary[index] */
                            weight,                   /* TM weight */
                            minRate,                  /* mininum rate */
                            shapingRate,              /* shaping rate */
                            burst,                    /* burst */
                            &tmId);
         if (rc || (tmId < 0))
         {
            tmctl_error("rdpaCtl_TmConfig ERROR! rdpaIf=%d parentTmId=%d prio=%d wt=%d tmId=%d rc=%d",
                     rdpaIf, parentTmId, prioIndex, weight, tmId, rc);
            return TMCTL_ERROR;
         }

         tmctl_debug("New rate control tmId=%d", tmId);
         drvPrioIndex = 0;
   }
   else
   {
      /* add the queue directly to parent tm */
      tmId = parentTmId;
      drvPrioIndex = prioIndex;
   }

   /* Configure the queue. */
   tmctl_debug("Configure Runner queue: tmId=%d qid=%d weight=%d bestEffort=%d", tmId, qid, weight, bestEffort);

   if ((rc = rdpaCtl_QueueConfig(devType,
                                 rdpaIf,
                                 tmId,         /* Tm ID */
                                 qid,
                                 drvPrioIndex,
                                 dir,
                                 qsize,
                                 weight,
                                 minBufs,
                                 shapingRate,
                                 bestEffort,
                                 qClean)))
   {
      tmctl_error("rdpaCtl_QueueConfig ERROR! rdpaIf=%d qid=%d size=%d sched=%d rc=%d",
                   rdpaIf, qid, qsize, tmMode, rc);

      if (tmMode == rdpa_tm_sched_disabled)
      {
         tmctl_debug("Remove the just added rate control tm. tmId=%d", tmId);

         if ((rc = rdpaCtl_TmRemove(tmId, dir, devType, rdpaIf)))
         {
            tmctl_error("rdpaCtl_TmRemove ERROR! tmId=%d rc=%d", tmId, rc);
         }
      }
      ret = TMCTL_ERROR;
   }

   return ret;

}  /* End of addQueue() */

/* ----------------------------------------------------------------------------
 * This function adds a runner service queue to the port.
 * Service queue is always downstream only, and needs to have the queue
 * created before it is linked to the scheduler.
 *
 * Parameters:
 *    parentTmId (IN) If queue shaper tm is needed, it is the
 *                    parent tm of the queue shaper tm. Otherwise,
 *                    it will be used as the tm for the queue.
 *    qid (IN) queue ID
 *    qsize (IN) queue size
 *    prioIndex (IN) queue priority index
 *    shapingRate (IN) queue shaping rate in kbps.
 *    burst (IN) queue shaping burst size in bytes
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e addSvcQueue(int parentTmId, int qid, int qsize, int prioIndex,
                        int shapingRate, int burst)
{
   int rc;
   int tmId;

   tmctl_debug("parentTmId=%d qid=%d qsize=%d prio=%d kbps=%d",
               parentTmId, qid, qsize, prioIndex, shapingRate);

   rc = rdpaCtl_RootTmConfig(RDPA_IOCTL_DEV_NONE, 0, rdpa_dir_ds,
                             rdpa_tm_level_queue, rdpa_tm_sched_disabled,
                             rdpa_tm_rl_single_rate, TMCTL_SCHED_TYPE_SP, &tmId);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_RootTmConfig() failed, parentTmId=%d qid=%d "
                  "qsize=%d prio=%d kbps=%d rc=%d",
                  parentTmId, qid, qsize, prioIndex, shapingRate, rc);
      goto ADD_SVC_Q_EXIT_1;
   }

   rc = rdpaCtl_QueueConfig(RDPA_IOCTL_DEV_NONE, 0, tmId,
                            qid, 0, rdpa_dir_ds, qsize, 0, 0, 0, 0, 0);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_QueueConfig failed, qid=%d size=%d "
                  "rc=%d",
                  qid, qsize, rc);
      goto ADD_SVC_Q_EXIT_1;
   }

   rc = rdpaCtl_TmConfig(RDPA_IOCTL_DEV_NONE, 0, parentTmId, /* Why is this function passing rdpa_if = 0 ?? */
                         rdpa_dir_ds, rdpa_tm_level_queue,
                         rdpa_tm_sched_disabled, prioIndex,
                         0, 0, shapingRate, burst, &tmId);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_TmConfig() failed, parentTmId=%d qid=%d "
                  "qsize=%d prio=%d kbps=%d rc=%d",
                  parentTmId, qid, qsize, prioIndex, shapingRate, rc);
      goto ADD_SVC_Q_EXIT_2;
   }

   return TMCTL_SUCCESS;

ADD_SVC_Q_EXIT_2:
   /*
    * Service queue configuration sequence is different from others,
    * the queue deletion sequence is not the exact reverse order of
    * the queue creation.
    */
   rc = rdpaCtl_QueueRemove(RDPA_IOCTL_DEV_NONE, 0, tmId,
                            rdpa_dir_ds, qid, INVALID_ID);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_QueueRemove failed, tmId=%d qid=%d rc=%d",
                  tmId, qid, rc);
   }

ADD_SVC_Q_EXIT_1:
   rc = rdpaCtl_RootTmRemove(tmId, rdpa_dir_ds, RDPA_IOCTL_DEV_NONE, 0);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_RootTmRemove failed, tmId=%d rc=%d",
                  tmId, rc);
   }

   return TMCTL_ERROR;

}  /* End of addSvcQueue() */

/* ----------------------------------------------------------------------------
 * This function deletes a runner queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    dir (IN) tm direction
 *    tmId (IN) tm ID
 *    qid (IN) queue ID
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e delQueue(int devType, int rdpaIf, rdpa_traffic_dir dir,
                     int tmId, int qid)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   int  rc;
   int  rootTmId;
   BOOL found;
   BOOL delQueueOnly = FALSE;

   tmctl_debug("Enter: rdpaIf=%d dir=%d tmId=%d qid=%d", rdpaIf, dir, tmId, qid);

   found    = FALSE;
   rootTmId = INVALID_ID;
   if ((rc = rdpaCtl_GetRootTm(devType, rdpaIf, &rootTmId, &found)))
   {
      tmctl_error("rdpaCtl_GetRootTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (!found || rootTmId < 0)
   {
      tmctl_error("Cannot find root tm for rdpaIf %d", rdpaIf);
      return TMCTL_ERROR;
   }
   if (tmId == rootTmId)
   {
      delQueueOnly = TRUE;  /* don't delete tmId */
   }
   else
   {
      found    = FALSE;
      rootTmId = INVALID_ID;
      if ((rc = rdpaCtl_GetRootSpTm(devType, rdpaIf, &rootTmId, &found)))
      {
         tmctl_error("rdpaCtl_GetRootSpTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
         return TMCTL_ERROR;
      }
      if (tmId == rootTmId)
      {
         delQueueOnly = TRUE;  /* don't delete tmId */
      }
      else
      {
         found    = FALSE;
         rootTmId = INVALID_ID;
         if ((rc = rdpaCtl_GetRootWrrTm(devType, rdpaIf, &rootTmId, &found)))
         {
            tmctl_error("rdpaCtl_GetRootWrrTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
            return TMCTL_ERROR;
         }
         if (tmId == rootTmId)
         {
            delQueueOnly = TRUE;  /* don't delete tmId */
         }
      }
   }

   if (delQueueOnly)
   {
      /* Don't delete the tm */
      /* set index to INVALID_ID so that the driver will derive it from qid. */
      if ((rc = rdpaCtl_QueueRemove(devType, rdpaIf, tmId, dir, qid, INVALID_ID)))
      {
         tmctl_error("rdpaCtl_QueueRemove ERROR! rdpaIf=%d tmId=%d dir=%d qid=%d rc=%d",
                      rdpaIf, tmId, dir, qid, rc);
         ret = TMCTL_ERROR;
      }
   }
   else
   {
      /* Delete the tm. Its associated queue will be deleted automatically. */
      tmctl_debug("rdpaCtl_TmRemove: tmId=%d", tmId);
      if ((rc = rdpaCtl_TmRemove(tmId, dir, devType, rdpaIf)))
      {
         tmctl_error("rdpaCtl_TmRemove ERROR! rdpaIf=%d tmId=%d dir=%d rc=%d",
                      rdpaIf, tmId, dir, rc);
         ret = TMCTL_ERROR;
      }
   }

   return ret;

}  /* End of delQueue() */


/* ----------------------------------------------------------------------------
 * This function checks the priority queuing configuration of the port.
 * For example, on the SF2 LAN port, only one of the following priority
 * queuing options is supported:
 *
 *    Q0  Q1  Q2  Q3  Q4  Q5  Q6  Q7
 * 1) SP  SP  SP  SP  SP  SP  SP  SP
 * 2) WRR WRR WRR WRR WRR WRR WRR SP
 * 3) WRR WRR WRR WRR WRR WRR SP  SP
 * 4) WRR WRR WRR WRR WRR SP  SP  SP
 * 5) WRR WRR WRR WRR SP  SP  SP  SP
 * 6) WRR WRR WRR WRR WRR WRR WRR WRR
 *
 * Parameters:
 *    maxQueues (IN) max number of queues supported by the port.
 *    maxSpQueues (IN) max number of SP queues supported by the port
 *                     when mixing with WRR queues.
 *    portQcfg_p (IN) Structure of the port queue configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e checkPortQueueSched(int maxQueues, int maxSpQueues,
                                tmctl_portQcfg_t* portQcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int  numSpQueues = 0;
   int  highestWrrQid;
   int  lowestSpQid;
   int  i;
   tmctl_queueCfg_t* qcfg_p;

   tmctl_debug("Enter: maxQueues=%d maxSpQueues=%d", maxQueues, maxSpQueues);

   highestWrrQid = INVALID_ID;
   lowestSpQid   = maxQueues;

   qcfg_p = &(portQcfg_p->qcfg[0]);
   for (i = 0; i < maxQueues; i++, qcfg_p++)
   {
      if (qcfg_p->qid >= 0)
      {
         if (qcfg_p->schedMode == TMCTL_SCHED_SP)
         {
            numSpQueues++;
            if (qcfg_p->qid < lowestSpQid)
               lowestSpQid = qcfg_p->qid;
         }
         else /* WRR */
         {
            if (qcfg_p->qid > highestWrrQid)
               highestWrrQid = qcfg_p->qid;
         }
      }
   }

   if ((lowestSpQid == maxQueues) || (highestWrrQid == INVALID_ID))
   {
      /* Either all wrr queues or all sp queues. */
      ret = TMCTL_SUCCESS;
   }
   else if ((lowestSpQid > highestWrrQid) &&
            (lowestSpQid >= (maxQueues - maxSpQueues)))
   {
      /* There are some sp and some wrr queues.
       * The lowestSpQid is higher than the highestWrrQid, and
       * the lowestSpQid is higher than or equal to the lowest allowable sp qid.
       */
      ret = TMCTL_SUCCESS;
   }
   else
   {
      tmctl_error("Port queue scheduling option is not supported.");
      tmctl_error("maxQueues=%d maxSpQueues=%d numSpQueues=%d lowestSpQid=%d highestWrrQid=%d",
                  maxQueues, maxSpQueues, numSpQueues, lowestSpQid, highestWrrQid);
      ret = TMCTL_ERROR;
   }

   return ret;

}  /* End of checkPortQueueSched() */


/* ----------------------------------------------------------------------------
 * This function gets rdpa interface (port) ID by interface name.
 *
 * Parameters:
 *    ifname (IN) Interface name
 *    rdpaIf_p (OUT) rdpa interface ID .
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getRdpaIfByIfname(const char* ifname, rdpa_if* rdpaIf_p)
{
   int rc;
   int unit;
   bcm_port_t port;
   uint32_t portmap = 0;
   int idx=0;

   if (ifname == NULL)
      return TMCTL_ERROR;

   if (strstr(ifname, GPON_IFC_STR))
   {
      rdpaIf_p[0] = rdpa_wan_type_to_if(rdpa_wan_gpon);
      return TMCTL_SUCCESS;
   }
   if (strstr(ifname, EPON_IFC_STR))
   {
      rdpaIf_p[0] = rdpa_wan_type_to_if(rdpa_wan_epon);
      return TMCTL_SUCCESS;
   }

   rc = bcm_enet_get_rdpa_if_from_if_name(ifname, (int *)rdpaIf_p);
   if (rc)
      return TMCTL_ERROR;

   tmctl_debug("ifname %s rdpaIf=%d", ifname, *rdpaIf_p);
   return TMCTL_SUCCESS;

}  /* End of tmctlRdpa_getRdpaIfByIfname() */


/* ----------------------------------------------------------------------------
 * This function initializes the Runner TM configuration for a port, TCONT,
 * or LLID.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    tmParms_p (IN) Port tm parameters.
 *    cfgFlags (IN)  Configuration flags.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_TmInit(int devType, int rdpaIf,
                             tmctl_portTmParms_t* tmParms_p,
                             uint32_t             cfgFlags)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   int  rc;
   int  qid;
   int  prioIndex;
   int  wt;
   int  qsize;
   int  rootTmId   = INVALID_ID;
   int  spTmId     = INVALID_ID;
   int  wrrTmId    = INVALID_ID;
#ifndef BCM_XRDP
   int  orlId      = INVALID_ID;
#endif
   int  parentTmId = INVALID_ID;
   BOOL found;
   BOOL useSingleTm = FALSE;
   rdpa_traffic_dir     dir;
   rdpa_tm_rl_rate_mode rlMode  = rdpa_tm_rl_single_rate;
   rdpa_tm_level_type   tmLevel = rdpa_tm_level_egress_tm;
   rdpa_tm_sched_mode   tmMode;
   uint32_t             schedType;

   tmctl_debug("Enter: rdpaIf=%d schedType=0x%x maxQueues=%d portShaper=%d queueShaper=%d cfgFlags=0x%x",
               rdpaIf, tmParms_p->schedCaps, tmParms_p->maxQueues,
               tmParms_p->portShaper, tmParms_p->queueShaper, cfgFlags);

   dir = getDir(devType, rdpaIf);
   qsize = getDefaultQueueSize(devType, dir);

   if (dir == rdpa_dir_ds)
   {
      /* Currently, Runner only supports SP for downstream */
      if (tmParms_p->schedCaps & TMCTL_SP_CAPABLE)
      {
         schedType = TMCTL_SCHED_TYPE_SP;
      }
      else
      {
         tmctl_error("Runner only supports SP for downstream TM.");
         return TMCTL_ERROR;
      }
   }
   else
   {
      schedType = cfgFlags & TMCTL_SCHED_TYPE_MASK;
   }

   useSingleTm = (TMCTL_1LEVEL_CAPABLE == (tmParms_p->schedCaps & TMCTL_1LEVEL_CAPABLE));

   /* check if the root tm already exists */
   found = FALSE;
   if ((rc = rdpaCtl_GetRootTm(devType, rdpaIf, &rootTmId, &found)))
   {
      tmctl_error("rdpaCtl_GetRootTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (found && (rootTmId >= 0))
   {
      tmctl_debug("Delete the existing Root TM.");

      /* Removing Root TM will also remove ORL, SP and WRR root TMs
       * and all the queues.
       */
      if ((rc = rdpaCtl_RootTmRemove(rootTmId, dir, devType, rdpaIf)))
      {
         tmctl_error("rdpaCtl_RootTmRemove ERROR! rdpaIf=%d rootTmId=%d rc=%d",
                     rdpaIf, rootTmId, rc);
         return TMCTL_ERROR;
      }

      rootTmId = INVALID_ID;
   }

   /* Configure ROOT tm for the port */
   switch (schedType)
   {
   case TMCTL_SCHED_TYPE_SP_WRR:
      if (useSingleTm)
         tmMode = rdpa_tm_sched_sp_wrr;
      else
         tmMode = rdpa_tm_sched_sp;
		 
      break;

   case TMCTL_SCHED_TYPE_SP:
      tmMode = rdpa_tm_sched_sp;
      if (tmParms_p->queueShaper)
         rlMode  = rdpa_tm_rl_dual_rate;
      else
         tmLevel = rdpa_tm_level_queue;
      break;
   
   case TMCTL_SCHED_TYPE_WRR:
      tmMode = rdpa_tm_sched_wrr;
      break;

   default:
      tmctl_error("Port sched type 0x%x is not supported.", schedType);
      return TMCTL_ERROR;
   }

   if (useSingleTm)
   {
      tmLevel = rdpa_tm_level_queue;
   }

   rootTmId = INVALID_ID;
   rc = rdpaCtl_RootTmConfig(devType, rdpaIf, dir, tmLevel, tmMode, rlMode,
                             (cfgFlags & ~TMCTL_INIT_DEFAULT_QUEUES), &rootTmId);
   if (rc || (rootTmId < 0))
   {
      tmctl_error("rdpaCtl_RootTmConfig ERROR! rdpaIf=%d", rdpaIf);
      return TMCTL_ERROR;
   }
   tmctl_debug("rdpaCtl_RootTmConfig SUCCESS! rootTmId=%d schedType=0x%x, cfgFlags=0x%x", rootTmId, schedType, cfgFlags);

   if (schedType == TMCTL_SCHED_TYPE_SP_WRR && useSingleTm == FALSE)
   {
      /* Create the SP group TM for the root TM. */
#ifndef BCM_XRDP
      tmLevel = tmParms_p->queueShaper ? rdpa_tm_level_egress_tm : rdpa_tm_level_queue;
#else
      tmLevel = rdpa_tm_level_queue;
#endif

      rc = rdpaCtl_TmConfig(devType,
                            rdpaIf,
                            rootTmId,           /* parent is rootTmId */
                            dir,
                            tmLevel,
                            rdpa_tm_sched_sp,
                            0,                  /* TM priority. root_tm.subsidiary[0] */
                            0,                  /* TM weight */
                            0,                  /* mininum rate */
                            0,                  /* shaping rate */
                            0,                  /* burst */
                            &spTmId);
      if (rc || (spTmId < 0))
      {
         tmctl_error("rdpaCtl_TmConfig ERROR! SP_TM rootTmId=%d rc=%d",
                     rootTmId, rc);
         rdpaCtl_RootTmRemove(rootTmId, dir, devType, rdpaIf);
         return TMCTL_ERROR;
      }
      tmctl_debug("rdpaCtl_TmConfig SUCCESS! rootTmId=%d spTmId=%d",
                  rootTmId, spTmId);

      /* Create the WRR group TM for the root TM. */
      rc = rdpaCtl_TmConfig(devType,
                            rdpaIf,
                            rootTmId,                 /* parent is rootTmId */
                            dir,
                            tmLevel,
                            rdpa_tm_sched_wrr,
                            1,                        /* TM priority. root_tm.subsidiary[1] */
                            0,                        /* TM weight */
                            0,                        /* mininum rate */
                            0,                        /* shaping rate */
                            0,                        /* burst */
                            &wrrTmId);
      if (rc || (wrrTmId < 0))
      {
         tmctl_error("rdpaCtl_TmConfig ERROR! WRR_TM rootTmId=%d rc=%d",
                     rootTmId, rc);
         rdpaCtl_RootTmRemove(rootTmId, dir, devType, rdpaIf);
         return TMCTL_ERROR;
      }
      tmctl_debug("rdpaCtl_TmConfig SUCCESS! rootTmId=%d wrrTmId=%d",
                   rootTmId, wrrTmId);

      /* Default queues will be added to spTmId */
      parentTmId = spTmId;
   }
   else
   {
      /* Default queues will be added directly to rootTmId. */
      parentTmId = rootTmId;
   }

#ifndef BCM_XRDP
   if (tmParms_p->portShaper)
   {
      /* Create the overall rate limitter for the port. */
      rc = rdpaCtl_OrlConfig(devType, rdpaIf, dir, 0, &orlId);
      if (rc || (orlId < 0))
      {
         tmctl_error("rdpaCtl_OrlConfig ERROR! rc=%d", rc);
         rdpaCtl_RootTmRemove(rootTmId, dir, devType, rdpaIf);
         return TMCTL_ERROR;
      }
      tmctl_debug("rdpaCtl_OrlConfig SUCCESS! orlId=%d shapingRate=0", orlId);
   }
#endif

   if (cfgFlags & TMCTL_INIT_DEFAULT_QUEUES)      /* The default queues initialization is required */
   {
      tmctl_debug("Configure default runner queues, cfgflags=0x%x", cfgFlags);

      if (useSingleTm == TRUE)
      {
         wt = (tmMode == rdpa_tm_sched_sp || tmMode == rdpa_tm_sched_sp_wrr)? 0 : 1;
      }
      else
      {
         wt = (tmMode == rdpa_tm_sched_sp)? 0 : 1;

         if (tmMode == rdpa_tm_sched_wrr || tmParms_p->queueShaper)
            tmMode = rdpa_tm_sched_disabled;
      }

      /* Initialize the default queues */
      for (qid = 0; qid < tmParms_p->maxQueues; qid++)
      {
         /* TM priority. For parentTm.subsidiary[index] */
         if (cfgFlags & TMCTL_QIDPRIO_MAP_Q7P7)
         {
             prioIndex = rdpaCtl_getQueuePrioIndex(qid, dir, tmParms_p->maxQueues, qid);
         }
         else
         {
             prioIndex = rdpaCtl_getQueuePrioIndex(qid, dir, tmParms_p->maxQueues,
               (tmParms_p->maxQueues - (uint32)qid - 1));
         }

         tmctl_debug("Add queue: sched(tmMode)=%d qid=%d qsize=%d prio=%d wt=%d shapingRate=512",
                     tmMode, qid, qsize, prioIndex, wt);

         if (devType == RDPA_IOCTL_DEV_NONE)
         {
            ret = addSvcQueue(parentTmId, qid, qsize, prioIndex, 0, 0);
         }
         else
         {
            ret = addQueue(devType, rdpaIf, dir, parentTmId, qid, qsize,
                           tmMode, prioIndex, wt, 0, 0, 0, 0, 0, 0);
         }

         if (ret == TMCTL_ERROR)
         {
            tmctl_error("addQueue ERROR! ret=%d", ret);
            break;
         }
         /* Dislocate the Runner queue from rdpactl config driver. */
         if ((rc = rdpaCtl_QueueDislocate(devType, rdpaIf, qid)))
         {
            tmctl_error("rdpaCtl_QueueDislocate ERROR! rdpaIf=%d qid=%d rc=%d",
                        rdpaIf, qid, rc);
            ret = TMCTL_ERROR;
            break;
         }
      }
   }

   if (ret == TMCTL_ERROR)
   {
      rdpaCtl_RootTmRemove(rootTmId, dir, devType, rdpaIf);
   }

   return ret;

}  /* End of tmctlRdpa_TmInit() */


/* ----------------------------------------------------------------------------
 * This function un-initializes the Runner TM configuration of a port, TCONT,
*  or LLID.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_TmUninit(int devType, int rdpaIf)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   int  rc;
   int  rootTmId = INVALID_ID;
   BOOL found;
   rdpa_traffic_dir dir;

   tmctl_debug("Enter: rdpaIf=%d", rdpaIf);

   dir = getDir(devType, rdpaIf);

   /* Find the root tm id */
   found = FALSE;
   if ((rc = rdpaCtl_GetRootTm(devType, rdpaIf, &rootTmId, &found)))
   {
      tmctl_error("rdpaCtl_GetRootTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (!found || (rootTmId == INVALID_ID))
   {
      tmctl_debug("Root TM had not been initialized. rdpaIf=%d rootTmId=%d",
                  rdpaIf, rootTmId);
      return TMCTL_SUCCESS;
   }

   /* Removing Root TM will also remove ORL, SP and WRR root TMs
    * and all the queues.
    */
   if ((rc = rdpaCtl_RootTmRemove(rootTmId, dir, devType, rdpaIf)))
   {
      tmctl_error("rdpaCtl_RootTmRemove ERROR! rdpaIf=%d rootTmId=%d rc=%d",
                  rdpaIf, rootTmId, rc);
      return TMCTL_ERROR;
   }

   return ret;

}  /* End of tmctlRdpa_TmUninit() */


/* ----------------------------------------------------------------------------
 * This function gets the configuration of a runner queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    qid (IN) Queue ID.
 *    tmId_p (OUT) tmId of the queue.
 *    qcfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getQueueCfg(int devType, int rdpaIf,
                                  int qid, int* tmId_p, tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   int  rc;
   BOOL found;
   rdpa_traffic_dir   dir;

   dir = getDir(devType, rdpaIf);

   found = FALSE;
   if ((rc = rdpaCtl_GetQueueConfig(devType, rdpaIf, qid, dir,
                                    tmId_p,
                                    &qcfg_p->qsize,
                                    &qcfg_p->weight,
                                    &qcfg_p->minBufs,
                                    &(qcfg_p->shaper.minRate),
                                    &(qcfg_p->shaper.shapingRate),
                                    &qcfg_p->bestEffort,
                                    &found)))
   {
      tmctl_error("rdpaCtl_GetQueueConfig ERROR! rdpaIf=%d qid=%d rc=%d",
                  rdpaIf, qcfg_p->qid, rc);
      return TMCTL_ERROR;
   }

   if (*tmId_p >= 0)
   {
      if (qcfg_p->weight == 0)
      {
         qcfg_p->schedMode = TMCTL_SCHED_SP;
         /* TMCtl does not maintain the priority. It is expected that user
          * obtains the priority information from management layer.
          */
         qcfg_p->priority  = INVALID_ID;
      }
      else
      {
         qcfg_p->schedMode = TMCTL_SCHED_WRR;
         qcfg_p->priority  = 0;
      }

      qcfg_p->qid                     = qid;
      qcfg_p->shaper.shapingBurstSize = 0;  /* not supported */
   }

   if (!found)
   {
      qcfg_p->qid = INVALID_ID;
      ret = TMCTL_NOT_FOUND;
   }

   return ret;

}  /* End of tmctlRdpa_getQueueCfg() */


/* ----------------------------------------------------------------------------
 * This function sets the configuration of a queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    tmParms_p (IN) port tm parameters.
 *    qcfg_p (IN) structure containing the queue config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setQueueCfg(int devType, int rdpaIf,
                                  tmctl_portTmParms_t* tmParms_p,
                                  tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int  rc;
   int  spTmId     = INVALID_ID;
   int  wrrTmId    = INVALID_ID;
   int  parentTmId = INVALID_ID;
   int  prioIndex;
   int  weight;
   int  minRate;
   int  shapingRate;
   BOOL found;
   int                tmIdSav;
   tmctl_queueCfg_t   qcfgSav;
   tmctl_portQcfg_t   portQcfg;
   uint32_t           schedType;
   rdpa_tm_sched_mode schedMode;
   rdpa_traffic_dir   dir;
   BOOL               qClean = FALSE;
   BOOL               useSingleTm = FALSE;
   BOOL               bestEffort = qcfg_p->bestEffort;


   tmctl_debug("Enter: devType=%d rdpaIf=%d qid=%d priority=%d schedMode=%d qsize=%d "
               "wt=%d minBufs=%d shapingRate=%d burstSize=%d minRate=%d bestEffort=%d",
                devType, rdpaIf, qcfg_p->qid, qcfg_p->priority, qcfg_p->schedMode,
                qcfg_p->qsize, qcfg_p->weight, qcfg_p->minBufs, qcfg_p->shaper.shapingRate,
                qcfg_p->shaper.shapingBurstSize, qcfg_p->shaper.minRate,
                qcfg_p->bestEffort ? 1 : 0);

   useSingleTm = ((tmParms_p->schedCaps & TMCTL_1LEVEL_CAPABLE) == TMCTL_1LEVEL_CAPABLE);

   dir = getDir(devType, rdpaIf);

   schedType = tmParms_p->cfgFlags & TMCTL_SCHED_TYPE_MASK;
      
   if (qcfg_p->schedMode == TMCTL_SCHED_SP)
   {
      schedMode = rdpa_tm_sched_sp;
      weight    = 0; /* change configured weight to 0 for SP */
   }
   else
   {
      schedMode = rdpa_tm_sched_wrr;
      weight    = qcfg_p->weight;
   }

   /* Currently, Runner only supports SP for downstream */
   if (dir == rdpa_dir_ds)
   {
      schedType = TMCTL_SCHED_TYPE_SP;
      schedMode = rdpa_tm_sched_sp;
      if ((devType == RDPA_IOCTL_DEV_NONE) || (devType == RDPA_IOCTL_DEV_PORT))
      {
          minRate     = qcfg_p->shaper.minRate;
          shapingRate = qcfg_p->shaper.shapingRate;
      }
      else
      {
          minRate     = 0;
          shapingRate = 0;
      }
   }
   else
   {
      minRate     = qcfg_p->shaper.minRate;
      shapingRate = qcfg_p->shaper.shapingRate;
   }

   /* get the existing queue config */
   ret = tmctlRdpa_getQueueCfg(devType, rdpaIf, qcfg_p->qid, &tmIdSav, &qcfgSav);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_getQueueCfg ERROR!");
      return ret;
   }

   /* Check if the queue is still configured in Runner. */
#if defined(GPON_HGU) || defined(EPON_HGU)
   /*
    * In the HGU case, different management protocols (user) may use different
    * QID/priority schemes on a DS ETH port. The queue to be configured by user
    * may have a different priority value from the existing hidden queue with
    * the same qid. In this case, the existing hidden queue needs to be first
    * removed. The same is true if arbitrary QID/priority mapping scheme is
    * supported.
    */
   if ((tmIdSav >= 0) && (isDsEthPort(devType, dir) == TRUE))
   {
      qClean = TRUE;
   }
   if ((tmIdSav >= 0) && (isDsEthPort(devType, dir) != TRUE))
#else
   if (tmIdSav >= 0)
#endif /* defined(GPON_HGU) || defined(EPON_HGU) */
   {
      /* The queue is still configured in Runner. See if there is
       * any change in queue config.
       */
      tmctl_debug("qcfg:    qid=%d schedMode=%d qsize=%d wt=%d minBufs=%d shapingRate=%d burstSize=%d minRate=%d bestEffort=%d",
                   qcfg_p->qid, qcfg_p->schedMode, qcfg_p->qsize, weight, qcfg_p->minBufs,
                   shapingRate, qcfg_p->shaper.shapingBurstSize,
                   qcfg_p->shaper.minRate, qcfg_p->bestEffort ? 1 : 0);
      tmctl_debug("qcfgSav: qid=%d schedMode=%d qsize=%d wt=%d minBufs=%d shapingRate=%d burstSize=%d minRate=%d bestEffort=%d tmId=%d",
                   qcfgSav.qid, qcfgSav.schedMode, qcfgSav.qsize, qcfgSav.weight, qcfgSav.minBufs,
                   qcfgSav.shaper.shapingRate, qcfgSav.shaper.shapingBurstSize,
                   qcfgSav.shaper.minRate, qcfgSav.bestEffort ? 1 : 0, tmIdSav);

      /* Note: Don't compare shapingBurstSize because it is not
       * supported by Runner TM.
       */
      if (qcfg_p->qsize     == qcfgSav.qsize &&
          qcfg_p->schedMode == qcfgSav.schedMode &&
          qcfg_p->minBufs   == qcfgSav.minBufs &&
          weight            == qcfgSav.weight &&
          minRate           == qcfgSav.shaper.minRate &&
          shapingRate       == qcfgSav.shaper.shapingRate &&
          bestEffort        == qcfgSav.bestEffort)
      {
            /* New and old config are the same. If the queue had been
             * dislocated from the rdpactl config driver, just need allocate
             * it back to the driver.
             */
            tmctl_debug("New and old config are the same.");
            if (qcfgSav.qid < 0)
            {
               tmctl_debug("Allocate qid %d config to rdpactl driver.", qcfg_p->qid);
               if ((rc = rdpaCtl_QueueAllocate(devType, rdpaIf, qcfg_p->qid)))
               {
                  tmctl_error("rdpaCtl_QueueAllocate ERROR! rdpaIf=%d qid=%d rc=%d",
                              rdpaIf, qcfg_p->qid, rc);
                  return TMCTL_ERROR;
               }
            }

            return TMCTL_SUCCESS;
      }
   }

   /* Get the configuration of all the device queues.
    * This is used for verifying whether the new queue config
    * would break the allowable queue scheduling scheme.
    */
   ret = tmctlRdpa_getDevQueueCfg(devType, rdpaIf, tmParms_p->maxQueues,
                                   &portQcfg);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_getDevQueueCfg ERROR! ret=%d", ret);
      return ret;
   }

   /* Replace the existing queue config with the new qcfg */
   memcpy(&(portQcfg.qcfg[qcfg_p->qid]), qcfg_p, sizeof(tmctl_queueCfg_t));

#ifndef BCM_PON
   /* Check if the new qcfg breaks the allowable scheduling scheme. */
   ret = checkPortQueueSched(tmParms_p->maxQueues, tmParms_p->maxSpQueues,
                             &portQcfg);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("checkPortQueueSched ERROR! rdpaIf=%d ret=%d", rdpaIf, ret);
      return ret;
   }
#endif

   /* qcfgNew is good. Continue to configure it. */
   if (schedType == TMCTL_SCHED_TYPE_SP_WRR && useSingleTm == FALSE)
   {
      /* The Runner TM topology must consist of a Root SP TM,
       * and a Root WRR tm.
       */
      /* Find the root sp tm */
      found = FALSE;
      if ((rc = rdpaCtl_GetRootSpTm(devType, rdpaIf, &spTmId, &found)))
      {
         tmctl_error("rdpaCtl_GetRootSpTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
         return TMCTL_ERROR;
      }
      if (!found || (spTmId < 0))
      {
         tmctl_error("Cannot find spTmId! rdpaIf=%d", rdpaIf);
         return TMCTL_ERROR;
      }

      /* Find the root wrr tm */
      found = FALSE;
      if ((rc = rdpaCtl_GetRootWrrTm(devType, rdpaIf, &wrrTmId, &found)))
      {
         tmctl_error("rdpaCtl_GetRootWrrTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
         return TMCTL_ERROR;
      }
      if (!found || (wrrTmId < 0))
      {
         tmctl_error("Cannot find wrrTmId! rdpaIf=%d", rdpaIf);
         return TMCTL_ERROR;
      }

      /* Select the proper parent TM for the queue. */
      parentTmId = (schedMode == rdpa_tm_sched_sp)? spTmId : wrrTmId;
   }
   else
   {
      /* port sched mode is either SP or WRR. parentTmId shall be the rootTmId. */
      found = FALSE;
      if ((rc = rdpaCtl_GetRootTm(devType, rdpaIf, &parentTmId, &found)))
      {
         tmctl_error("rdpaCtl_GetRootTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
         return TMCTL_ERROR;
      }
      if (!found || (parentTmId < 0))
      {
         tmctl_error("Cannot find rootTmId! rdpaIf=%d", rdpaIf);
         return TMCTL_ERROR;
      }
   }

   if (tmIdSav >= 0)
   {
      /* delete the original config before configuring it for new qcfg */
      tmctl_debug("Delete the existing queue configuration");

      ret = delQueue(devType, rdpaIf, dir, tmIdSav, qcfg_p->qid);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("delQueue ERROR! rdpaIf=%d dir=%d tmId=%d qid=%d",
                     rdpaIf, dir, tmIdSav, qcfg_p->qid);
         return ret;
      }
   }

   /*  if  qsize is 0 - we are done (it's unconfigure queue) */
   if (!qcfg_p->qsize)
   {
       return  ret;
   }

   /* Now configure the queue*/
   tmctl_debug("Set the new qcfg: rdpaIf=%d parentTmId=%d qid=%d qsize=%d sched=%d wt=%d minBufs=%d shapingRate=%d",
                rdpaIf, parentTmId, qcfg_p->qid, qcfg_p->qsize, schedMode, weight, qcfg_p->minBufs, shapingRate);

   if (schedMode == rdpa_tm_sched_wrr)
   {
      prioIndex = qcfg_p->qid;
   }
   else
   {
      /* TM priority. For parentTm.subsidiary[index] */
      prioIndex = rdpaCtl_getQueuePrioIndex(qcfg_p->qid, dir, tmParms_p->maxQueues,
                                            qcfg_p->priority);
   }

   if (useSingleTm == FALSE)
   {
      if (schedMode == rdpa_tm_sched_wrr || tmParms_p->queueShaper)
         schedMode = rdpa_tm_sched_disabled;
   }

   if (devType == RDPA_IOCTL_DEV_NONE)
   {
      // IS minBufs RELEVANT HERE?
      ret = addSvcQueue(parentTmId, qcfg_p->qid, qcfg_p->qsize, prioIndex,
                        shapingRate, qcfg_p->shaper.shapingBurstSize);
   }
   else
   {
      ret = addQueue(devType, rdpaIf, dir,
                     parentTmId,
                     qcfg_p->qid,
                     qcfg_p->qsize,
                     schedMode,
                     prioIndex,
                     weight,
                     qcfg_p->minBufs,
                     minRate,
                     shapingRate,
                     qcfg_p->shaper.shapingBurstSize,
                     bestEffort,
                     qClean);

#if defined(SUPPORT_DPI) && !defined(BCM_XRDP)
      /* Add US service queue scheduler after queue config */
      if (!ret && bestEffort && dir == rdpa_dir_us && rdpa_if_is_wan(rdpaIf))
          ret = rdp_add_us_dpi_queues(parentTmId, qcfg_p);
#endif
   }

   if (ret == TMCTL_ERROR)
   {
      tmctl_error("addQueue ERROR! rdpaIf=%d parentTmId=%d qid=%d qsize=%d sched=%d wt=%d minBufs=%d shapingRate=%d minRate=%d bestEffort=%d",
                   rdpaIf, parentTmId, qcfg_p->qid, qcfg_p->qsize, schedMode, weight, qcfg_p->minBufs, shapingRate, minRate, bestEffort ? 1 : 0);

      /* Config failed. Need to restore the original queue config */
      if (tmIdSav >= 0)
      {
         if (dir == rdpa_dir_ds)
            schedMode = rdpa_tm_sched_sp; /* always SP for downstream */
         else
            schedMode = (qcfgSav.schedMode == TMCTL_SCHED_SP)?
                              rdpa_tm_sched_sp : rdpa_tm_sched_wrr;

         if (schedType == TMCTL_SCHED_TYPE_SP_WRR && useSingleTm == FALSE)
            parentTmId = (schedMode == rdpa_tm_sched_sp)? spTmId : wrrTmId;

         if (useSingleTm == FALSE)
         {
            if (schedMode == rdpa_tm_sched_wrr || tmParms_p->queueShaper)
               schedMode = rdpa_tm_sched_disabled;
         }

         if (devType == RDPA_IOCTL_DEV_NONE)
         {
            addSvcQueue(parentTmId, qcfg_p->qid, qcfgSav.qsize, prioIndex,
                        qcfgSav.shaper.shapingRate,
                        qcfgSav.shaper.shapingBurstSize);
         }
         else
         {
            addQueue(devType, rdpaIf, dir,
                     parentTmId,
                     qcfg_p->qid,
                     qcfgSav.qsize,
                     schedMode,
                     prioIndex,
                     qcfgSav.weight,
                     qcfgSav.minBufs,
                     qcfgSav.shaper.minRate,
                     qcfgSav.shaper.shapingRate,
                     qcfgSav.shaper.shapingBurstSize,
                     qcfgSav.bestEffort,
                     0);
         }
      }
   }

   return ret;

}  /* End of tmctlRdpa_setQueueCfg() */


/* ----------------------------------------------------------------------------
 * This function dislocates the Runner queue from the rdpactl config driver.
 * Note that the Runner queue is not deleted.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    qid (IN) Queue ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_delQueueCfg(int devType, int rdpaIf, int qid)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   int  rc;

   tmctl_debug("Enter: rdpaIf=%d qid=%d", rdpaIf, qid);

   if ((rc = rdpaCtl_QueueDislocate(devType, rdpaIf, qid)))
   {
      tmctl_error("rdpaCtl_QueueDislocate ERROR! rdpaIf=%d qid=%d rc=%d",
                  rdpaIf, qid, rc);
      return TMCTL_ERROR;
   }

   return ret;

}  /* End of tmctlRdpa_delQueueCfg() */

/* ----------------------------------------------------------------------------
 * This function Remove the Runner queue from the rdpactl config driver.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    qid (IN) Queue ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_remQueueCfg(int devType, int rdpaIf, int qid)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   rdpa_traffic_dir dir;
   int tmIdSav;
   tmctl_queueCfg_t qcfgSav;

   tmctl_debug("Enter: devType=%d rdpaIf=%d qid=%d", devType, rdpaIf, qid);

   dir = getDir(devType, rdpaIf);

   /* get the existing queue config */
   ret = tmctlRdpa_getQueueCfg(devType, rdpaIf, qid, &tmIdSav, &qcfgSav);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_getQueueCfg ERROR!");
      return ret;
   }

   if (tmIdSav >= 0)
   {
      /* delete the queue */
      tmctl_debug("Delete the queue configuration");

      ret = delQueue(devType, rdpaIf, dir, tmIdSav, qid);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("delQueue ERROR! rdpaIf=%d dir=%d tmId=%d qid=%d",
                     rdpaIf, dir, tmIdSav, qid);
         return ret;
      }
   }

   return ret;
}  /* End of tmctlRdpa_remQueueCfg() */


/* ----------------------------------------------------------------------------
 * This function gets the Runner queue configuration of the device.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    maxQueues (IN) max number of queues supported by the port.
 *    portQcfg_p (OUT) Structure to receive the port queue configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getDevQueueCfg(int devType, int rdpaIf,
                                      int maxQueues,
                                      tmctl_portQcfg_t* portQcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int  tmId = 0;
   int  qid = 0;
   tmctl_queueCfg_t* qcfg_p= NULL;

   portQcfg_p->numQueues = 0;

   qcfg_p = &(portQcfg_p->qcfg[0]);
   for (qid = 0; qid < maxQueues; qid++, qcfg_p++)
   {
      ret = tmctlRdpa_getQueueCfg(devType, rdpaIf, qid, &tmId, qcfg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlRdpa_getQueueCfg ERROR!");
         break;
      }
      else if (ret == TMCTL_SUCCESS)
      {
         portQcfg_p->numQueues++;
      }
   }

   return TMCTL_SUCCESS;

}  /* End of tmctlRdpa_getDevQueueCfg() */


/* ----------------------------------------------------------------------------
 * This function gets the port shaping rate.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (OUT) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getPortShaper(int devType, int rdpaIf,
                                    tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   BOOL found = FALSE;

   int  rc;
   int  orlId       = INVALID_ID;
   int  shapingRate = 0;
   BOOL orlLinked   = FALSE;
   rdpa_traffic_dir  dir;

   dir = getDir(devType, rdpaIf);

   shaper_p->shapingRate      = 0;
   shaper_p->shapingBurstSize = 0;  /* not supported */
   shaper_p->minRate          = 0;  /* not supported */

   if ((rc = rdpaCtl_GetOrl(devType, rdpaIf, dir,
                            &orlId, &shapingRate, &orlLinked, &found)))
   {
      tmctl_error("rdpaCtl_GetOrl ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }

   shaper_p->shapingRate = shapingRate;

   if (!found)
      ret = TMCTL_NOT_FOUND;

   return ret;

}  /* End of tmctlRdpa_getPortShaper() */


/* ----------------------------------------------------------------------------
 * This function sets the port shaping rate. If the specified shaping rate
 * is greater than 0, the pre-configured Runner overall rate limiter will
 * be set with the shaping rate and linked to the port. Otherwise, the
 * overall rate limiter will be un-linked from the port.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (IN) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setPortShaper(int devType, int rdpaIf,
                                    tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   int  rc;
   rdpa_traffic_dir  dir;
   int  orlId       = INVALID_ID;
   int  shapingRate = 0;
   BOOL orlLinked   = FALSE;
   BOOL found     = FALSE;

   dir = getDir(devType, rdpaIf);

   if ((rc = rdpaCtl_GetOrl(devType, rdpaIf, dir,
                           &orlId, &shapingRate, &orlLinked, &found)))
   {
      tmctl_error("rdpaCtl_GetOrl ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (!found || (orlId < 0))
   {
      if (shaper_p->shapingRate <= 0)
         return TMCTL_SUCCESS;

      tmctl_error("Cannot find the orl by rdpaIf=%d", rdpaIf);
      return TMCTL_ERROR;
   }

   if (shaper_p->shapingRate <= 0)
   {
      /* Unlink the orl from the port */
      tmctl_debug("Unlink the orl from port. rdpaIf=%d orlId=%d", rdpaIf, orlId);

      if ((rc = rdpaCtl_OrlUnlink(devType, rdpaIf, orlId, dir)))
      {
         tmctl_error("rdpaCtl_OrlUnlink ERROR! rdpaIf=%d orlId=%d rc=%d",
                      rdpaIf, orlId, rc);
         ret = TMCTL_ERROR;
      }
   }
   else
   {
      /* Note: Runner TM does not support min rate shaping.
       *       It does not support burst size either.
       */
//      int shapingRate = (shaper_p->shapingRate > 0)? shaper_p->shapingRate : 0;

      tmctl_debug("Link the orl to port. rdpaIf=%d orlId=%d shapingRate=%d",
                   rdpaIf, orlId, shaper_p->shapingRate);
      if ((rc = rdpaCtl_OrlLink(devType, rdpaIf, orlId, dir, shaper_p->shapingRate)))
      {
         tmctl_error("rdpaCtl_OrlLink ERROR! rdpaIf=%d orlId=%d shapingRate=%d rc=%d",
                      rdpaIf, orlId, shaper_p->shapingRate, rc);
         ret = TMCTL_ERROR;
      }
   }

   return ret;

}  /* End of tmctlRdpa_setPortShaper() */


/* ----------------------------------------------------------------------------
 * This function gets the statistics of a queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    qid (IN) Queue ID.
 *    queueStats_p (OUT) Structure to return the queue stats.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getQueueStats(int devType, int rdpaIf, int qid,
                                    tmctl_queueStats_t* queueStats_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   int  rc;
   rdpa_traffic_dir dir;
   rdpa_stat_1way_t stats;

   dir = getDir(devType, rdpaIf);

   if ((rc = rdpaCtl_GetQueueStats(devType, rdpaIf, dir, qid, &stats)))
   {
      tmctl_error("rdpaCtl_GetQueueStats ERROR! rdpaIf=%d qid=%d rc=%d",
                  rdpaIf, qid, rc);
      return TMCTL_ERROR;
   }

   queueStats_p->txPackets      = stats.passed.packets;
   queueStats_p->txBytes        = stats.passed.bytes;
   queueStats_p->droppedPackets = stats.discarded.packets;
   queueStats_p->droppedBytes   = stats.discarded.bytes;

   return ret;

}  /* End of tmctlRdpa_getQueueStats() */


/* ----------------------------------------------------------------------------
 * This function gets TM memory info (capabilities) from rdpactl driver.
 *    fpmPoolMemorySize (OUT) fpm pool memory size in MB
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */

tmctl_ret_e tmctlRdpa_getMemoryInfo(int * fpmPoolMemorySize)
{
   int  rc;
   if (rc = rdpaCtl_GetTmMemoryInfo(fpmPoolMemorySize))
   {
      tmctl_error("rdpaCtl_GetTmMemoryInfo ERROR! rc=%d", rc);
      return TMCTL_ERROR;
   }
   return TMCTL_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * This function gets TM parameters (capabilities) from rdpactl driver.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID
 *    tmParms_p (OUT) Structure to return port TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */

tmctl_ret_e tmctlRdpa_getTmParms(int devType, int rdpaIf,
                                 tmctl_portTmParms_t* tmParms_p)
{
   int  rc;
   rdpa_traffic_dir  dir;
   int  schedCaps;
   int  maxQueues;
   int  maxSpQueues;
   int  cfgFlags;
   BOOL portShaper;
   BOOL queueShaper;
   BOOL found = FALSE;

   tmctl_debug("Enter: rdpaIf=%d", rdpaIf);

   dir = getDir(devType, rdpaIf);

   if ((rc = rdpaCtl_GetPortTmParms(devType, rdpaIf, dir,
                                    &schedCaps, &maxQueues, &maxSpQueues,
                                    &portShaper, &queueShaper, &cfgFlags, &found)))
   {
      tmctl_error("rdpaCtl_GetPortTmParms ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (!found)
   {
      tmctl_error("Cannot find port tm! rdpaIf=%d", rdpaIf);
      return TMCTL_ERROR;
   }

   tmParms_p->maxQueues   = maxQueues;
   tmParms_p->maxSpQueues = maxSpQueues;
   tmParms_p->portShaper  = portShaper;
   tmParms_p->queueShaper = queueShaper;
   tmParms_p->cfgFlags    = cfgFlags;

   tmParms_p->schedCaps = 0;
   if (schedCaps & RDPA_TM_SP_CAPABLE)
      tmParms_p->schedCaps |= TMCTL_SP_CAPABLE;
   if (schedCaps & RDPA_TM_WRR_CAPABLE)
      tmParms_p->schedCaps |= TMCTL_WRR_CAPABLE;
   if (schedCaps & RDPA_TM_WDRR_CAPABLE)
      tmParms_p->schedCaps |= TMCTL_WDRR_CAPABLE;
   if (schedCaps & RDPA_TM_WFQ_CAPABLE)
      tmParms_p->schedCaps |= TMCTL_WFQ_CAPABLE;
   if (schedCaps & RDPA_TM_SP_WRR_CAPABLE)
      tmParms_p->schedCaps |= TMCTL_SP_WRR_CAPABLE;
   if (schedCaps & RDPA_TM_1LEVEL_CAPABLE)
      tmParms_p->schedCaps |= TMCTL_1LEVEL_CAPABLE;

   return TMCTL_SUCCESS;

}  /* End of tmctlRdpa_getTmParms() */

/* ----------------------------------------------------------------------------
 * This function sets the port rate limiting in the Runner thus enabling the per-port
 * shaping. Shaping rate less than or equal to 0 disables the per-port shaping
 * enables
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (IN) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setTmRlCfg(int devType, int rdpaIf,
                                 tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   rdpa_traffic_dir  dir;
   int  rc;
   int  rootTmId  = INVALID_ID;
   BOOL found     = FALSE;

   dir = getDir(devType, rdpaIf);

   if ((rc = rdpaCtl_GetRootTm(devType, rdpaIf, &rootTmId, &found)))
   {
     tmctl_error("rdpaCtl_GetRootTm ERROR! devType=%d rdpaIf=%d rc=%d",devType, rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (!found || (rootTmId < 0))
   {
      tmctl_error("Cannot find Root TM for devType=%d rdpaIf=%d", devType, rdpaIf);
      return TMCTL_ERROR;
   }

   if (shaper_p->shapingRate < 0)
   {
      shaper_p->shapingRate = 0;
   }
   /* Note: Runner TM does not support min rate shaping.
    *       It does not support burst size either.
    */
   if ((rc = rdpaCtl_TmRlConfig(dir, rootTmId, shaper_p->shapingRate,
                                shaper_p->shapingBurstSize)))
   {
         tmctl_error("rdpaCtl_TmRlConfig ERROR! rdpaIf=%d rootTmId=%d shapingRate=%d rc=%d",
                      rdpaIf, rootTmId, shaper_p->shapingRate, rc);
         ret = TMCTL_ERROR;
   }

   return ret;

}  /* End of tmctlRdpa_setTmRlCfg() */


/* ----------------------------------------------------------------------------
 * This function gets the port rate limiting parameters
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (OUT) Shaper (rate limiting) parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getTmRlCfg(int devType, int rdpaIf,
                                 tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   rdpa_traffic_dir  dir;
   int  rc;
   int  rootTmId;
   int  shapingRate;
   int  burstSize;
   BOOL found = FALSE;

   dir = getDir(devType, rdpaIf);


   if ((rc = rdpaCtl_GetRootTm(devType, rdpaIf, &rootTmId, &found)))
   {
      tmctl_error("rdpaCtl_GetRootTm ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (!found || rootTmId < 0)
   {
      tmctl_error("Cannot find root tm for rdpaIf %d", rdpaIf);
      return TMCTL_ERROR;
   }

   if ((rc = rdpaCtl_GetTmRlConfig((int)dir, rootTmId,
                                   &shapingRate, &burstSize, &found)))
   {
      tmctl_error("rdpaCtl_GetTmRlConfig ERROR! rdpaIf=%d rc=%d", rdpaIf, rc);
      return TMCTL_ERROR;
   }
   if (!found)
   {
      tmctl_error("Cannot find port tm! rdpaIf=%d", rdpaIf);
      return TMCTL_ERROR;
   }

   shaper_p->shapingRate = shapingRate;
   shaper_p->shapingBurstSize = burstSize;

   return ret;

}  /* End of tmctlRdpa_getTmRlCfg() */


/* ----------------------------------------------------------------------------
 * This function sets the drop algorithm configuration of a runner queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID.
 *    qid (IN) Queue ID.
 *    dropAlgorithm (IN) drop algorithm.
 *    dropAlgLo_p (IN) pointer to drop algorithm structure.
 *    dropAlgHi_p (IN) pointer to drop algorithm structure.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setQueueDropAlg(int devType, int rdpaIf, int qid,
                                      tmctl_queueDropAlg_t* dropAlg_p)
{
   int rc;
   int rdpaDropAlg;
   tmctl_queueDropAlgExt_t* dropAlgLo_p = &dropAlg_p->dropAlgLo;
   tmctl_queueDropAlgExt_t* dropAlgHi_p = &dropAlg_p->dropAlgHi;

   rdpa_traffic_dir dir;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   rdpaDropAlg = convertDropAlg(dropAlg_p->dropAlgorithm);

   dir = getDir(devType, rdpaIf);
   rc = rdpaCtl_SetQueueDropAlg(devType, rdpaIf, qid, dir,
                                rdpaDropAlg,
                                dropAlgLo_p->redMinThreshold,
                                dropAlgLo_p->redMaxThreshold,
                                dropAlgHi_p->redMinThreshold,
                                dropAlgHi_p->redMaxThreshold,
                                dropAlg_p->priorityMask0,
                                dropAlg_p->priorityMask1);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_SetQueueDropAlg failed, rdpaIf=%d qid=%d rc=%d",
                  rdpaIf, qid, rc);
      return TMCTL_ERROR;
   }

   return ret;

}  /* End of tmctlRdpa_setQueueDropAlg() */

tmctl_ret_e tmctlRdpa_getQueueDropAlg(int devType, int rdpaIf, int qid,
                                      tmctl_queueDropAlg_t* dropAlg_p)
{
   int rc;
   int rdpaDropAlg;
   tmctl_queueDropAlgExt_t* dropAlgLo_p = &dropAlg_p->dropAlgLo;
   tmctl_queueDropAlgExt_t* dropAlgHi_p = &dropAlg_p->dropAlgHi;
   rdpa_traffic_dir dir;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   dir = getDir(devType, rdpaIf);
   rc = rdpaCtl_GetQueueDropAlg(devType, rdpaIf, qid, dir,
                                &rdpaDropAlg,
                                &dropAlgLo_p->redMinThreshold,
                                &dropAlgLo_p->redMaxThreshold,
                                &dropAlgLo_p->redPercentage,
                                &dropAlgHi_p->redMinThreshold,
                                &dropAlgHi_p->redMaxThreshold,
                                &dropAlgHi_p->redPercentage,
                                &dropAlg_p->priorityMask0,
                                &dropAlg_p->priorityMask1);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_SetQueueDropAlg failed, rdpaIf=%d qid=%d rc=%d",
                  rdpaIf, qid, rc);
      return TMCTL_ERROR;
   }

   dropAlg_p->dropAlgorithm = ((rdpaDropAlg == rdpa_tm_drop_alg_red) ? TMCTL_DROP_RED :
                                ((rdpaDropAlg == rdpa_tm_drop_alg_wred) ? TMCTL_DROP_WRED :
                                 TMCTL_DROP_DT));

   return ret;
}  /* End of tmctlRdpa_getQueueDropAlg() */

tmctl_ret_e tmctlRdpa_setQueueSize(int devType, int rdpaIf, int qid, int size)
{
   int rc;
   rdpa_traffic_dir dir;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   dir = getDir(devType, rdpaIf);
   rc = rdpaCtl_SetQueueSize(devType, rdpaIf, qid, dir,size);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_SetQueueSize failed, rdpaIf=%d qid=%d rc=%d",
                  rdpaIf, qid, rc);
      return TMCTL_ERROR;
   }
   return ret;
}  /* End of tmctlRdpa_setQueueSize() */

tmctl_ret_e tmctlRdpa_setQueueShaper(int devType, int rdpaIf, int qid, tmctl_shaper_t *shaper_p)
{
   int rc;
   rdpa_traffic_dir dir;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   dir = getDir(devType, rdpaIf);
   rc = rdpaCtl_SetQueueShaper(devType, rdpaIf, qid, dir, shaper_p->minRate, 
                            shaper_p->shapingRate, shaper_p->shapingBurstSize);
   if (rc != 0)
   {
      tmctl_error("tmctlRdpa_setQueueShaper failed, rdpaIf=%d qid=%d rc=%d",
                  rdpaIf, qid, rc);
      return TMCTL_ERROR;
   }
   return ret;
}  /* End of tmctlRdpa_setQueueShaper() */

/* ----------------------------------------------------------------------------
 * This function gets the configuration of pbit to q table. If the
 * configuration is not found, ....
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    cfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getPbitToQ(int devType,
                                 int rdpaIf,
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_SUCCESS;
    int  rc;
    BOOL found;

    found = FALSE;
    if ((rc = rdpaCtl_PitToQueueGet(devType, rdpaIf, &found, cfg_p)))
    {
        tmctl_error("rdpaCtl_PitToQueueGet dev:%d if:%d ERROR! rc=%d", \
            devType, rdpaIf, rc);
        return TMCTL_ERROR;
    }

    if (!found)
    {
        ret = TMCTL_NOT_FOUND;
    }

    return ret;

}  /* End of tmctlRdpa_getPbitToQ() */


/* ----------------------------------------------------------------------------
 * This function sets the configuration of pbit to q table.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    cfg_p (IN) config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setPbitToQ(int devType,
                                 int rdpaIf,
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_SUCCESS;
    int  rc;

    if ((rc = rdpaCtl_PitToQueueSet(devType, rdpaIf, *cfg_p)))
    {
        tmctl_error("rdpaCtl_PitToQueueSet dev:%d if:%d ERROR! rc=%d", \
            devType, rdpaIf, rc);
        return TMCTL_ERROR;
    }

    return ret;

}  /* End of tmctlRdpa_setPbitToQ() */


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
tmctl_ret_e tmctlRdpa_getForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p)
{
   int rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   rc = rdpaCtl_get_force_dscp(dir, enable_p);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_GetForceDscp failed, dir=%d rc=%d",
                  dir, rc);
      return TMCTL_ERROR;
   }

   return ret;

}  /* End of tmctlRdpa_getForceDscpToPbit() */


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
tmctl_ret_e tmctlRdpa_setForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p)
{
   int rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   rc = rdpaCtl_set_force_dscp(dir, *enable_p);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_SetForceDscp failed, dir=%d enable=%d rc=%d",
                  dir, (*enable_p), rc);
      return TMCTL_ERROR;
   }

   return ret;

}  /* End of tmctlRdpa_setForceDscpToPbit() */


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
tmctl_ret_e tmctlRdpa_getPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p)
{
   int rc;
   BOOL enable;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   rc = rdpaCtl_PktBasedQosGet(dir, type, &enable);
   if (rc != 0)
   {
      tmctl_error("rdpaCtl_PktBasedQosGet failed, dir=%d type=%d rc=%d",
                  dir, type, rc);
      return TMCTL_ERROR;
   }
   *enable_p = enable;

   return ret;

}  /* End of tmctlRdpa_getPktBasedQos() */


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
tmctl_ret_e tmctlRdpa_setPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p)
{
   int rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   rc = rdpaCtl_PktBasedQosSet(dir, type, *enable_p);
   if (rc != 0)
   {
      tmctl_error(
        "rdpaCtl_PktBasedQosSet failed, dir=%d type=%d enable=%d rc=%d", \
                  dir, type, (*enable_p), rc);
      return TMCTL_ERROR;
   }

   return ret;

}  /* End of tmctlRdpa_setPktBasedQos() */
