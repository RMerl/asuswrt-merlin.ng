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

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "os_defs.h"
#include "tmctl_api.h"
#include "tmctl_api_trace.h"
#include "tmctl_ethsw.h"
#include "tmctl_bcmtm.h"


/* ----------------------------------------------------------------------------
 * This function initializes the basic TM settings for a port/tcont/llid based
 * on TM capabilities.
 *
 * Note that if the port had already been initialized, all its existing
 * configuration will be deleted before re-initialization.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port/TCONT/LLID identifier.
 *    cfgFlags (IN) Port TM initialization config flags.
 *                  See bit definitions in tmctl_api.h
 *    numQueues (IN) Number of queues to be set for TM.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_portTmInit_plat(tmctl_devType_e devType,
                                  tmctl_if_t*     if_p,
                                  uint32_t        cfgFlags,
                                  int             numQueues)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d", devType);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      tmctl_portTmParms_t tmParms;

      ret = tmctl_getPortTmParms(TMCTL_DEV_ETH, if_p, &tmParms);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctl_getPortTmParms returns error");
         return ret;
      }

      if (tmParms.maxQueues == 0 || tmParms.maxQueues > MAX_TMCTL_QUEUES_BASELINE)
      {
         tmctl_error("Invalid maxQueues=%d ifname=%s", tmParms.maxQueues,
                     if_p->ethIf.ifname);
         return TMCTL_ERROR;
      }

      if (tmParms.maxSpQueues > tmParms.maxQueues)
      {
         tmctl_error("Invalid maxSpQueues=%d maxQueues=%d ifname=%s",
                     tmParms.maxSpQueues, tmParms.maxQueues, if_p->ethIf.ifname);
         return TMCTL_ERROR;
      }
      ret = tmctlBcmTm_portTmInit(if_p->ethIf.ifname, &tmParms);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_portTmInit ERROR! ret=%d", ret);
      }
#endif
   }

   return ret;

}  /* End of tmctl_portTmInit() */


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
tmctl_ret_e tmctl_portTmUninit_plat(tmctl_devType_e devType,
                                    tmctl_if_t*     if_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d", devType);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_portTmUninit(if_p->ethIf.ifname);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_portTmUninit ERROR! ret=%d", ret);
      }
#endif
   }

   return ret;

}  /* End of tmctl_portTmUninit() */


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
tmctl_ret_e tmctl_getQueueCfg_plat(tmctl_devType_e   devType,
                                   tmctl_if_t*       if_p,
                                   int               queueId,
                                   tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_getQueueCfg(if_p->ethIf.ifname, queueId, qcfg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_getQueueCfg ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_getQueueCfg() */


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
tmctl_ret_e tmctl_setQueueCfg_plat(tmctl_devType_e   devType,
                                   tmctl_if_t*       if_p,
                                   tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d qid=%d priority=%d qsize=%d schedMode=%d wt=%d minRate=%d kbps=%d mbs=%d",
               devType, qcfg_p->qid, qcfg_p->priority, qcfg_p->qsize, qcfg_p->schedMode,
               qcfg_p->weight, qcfg_p->shaper.minRate,
               qcfg_p->shaper.shapingRate, qcfg_p->shaper.shapingBurstSize);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      tmctl_portTmParms_t tmParms;
      uint32_t            schedType;

      /* get the port tm parameters */
      ret = tmctl_getPortTmParms(TMCTL_DEV_ETH, if_p, &tmParms);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctl_getPortTmParms returns error");
         return ret;
      }

      schedType = tmParms.cfgFlags & TMCTL_SCHED_TYPE_MASK;

      if ((qcfg_p->schedMode == TMCTL_SCHED_SP &&
           schedType != TMCTL_SCHED_TYPE_SP  && schedType != TMCTL_SCHED_TYPE_SP_WRR) ||
          (qcfg_p->schedMode == TMCTL_SCHED_WRR &&
           schedType != TMCTL_SCHED_TYPE_WRR && schedType != TMCTL_SCHED_TYPE_SP_WRR) ||
          (qcfg_p->schedMode == TMCTL_SCHED_WFQ &&
           schedType != TMCTL_SCHED_TYPE_WFQ))
      {
         tmctl_error("Queue sched mode %d is not supported", qcfg_p->schedMode);
         return TMCTL_ERROR;
      }

      ret = tmctlBcmTm_setQueueCfg(if_p->ethIf.ifname, &tmParms, qcfg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_setQueueCfg ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_setQueueCfg() */


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
tmctl_ret_e tmctl_delQueueCfg_plat(tmctl_devType_e devType,
                                   tmctl_if_t*     if_p,
                                   int             queueId)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_delQueueCfg(if_p->ethIf.ifname, queueId);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_delQueueCfg ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_delQueueCfg() */


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
tmctl_ret_e tmctl_getPortShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p,
                                     tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d", devType);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_getPortShaper(if_p->ethIf.ifname, shaper_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_getPortShaper ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_getPortShaper() */


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
tmctl_ret_e tmctl_setPortShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p,
                                     tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d portKbps=%d portMbs=%d", devType,
               shaper_p->shapingRate, shaper_p->shapingBurstSize);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_setPortShaper(if_p->ethIf.ifname, shaper_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_setPortShaper ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_setPortShaper() */


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
tmctl_ret_e tmctl_allocQueueProfileId_plat(int* queueProfileId_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: ");

#if defined(SUPPORT_BCMTM)
   ret = tmctlBcmTm_allocQueueProfileId(queueProfileId_p);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlBcmTm_allocQueueProfileId ERROR! ret=%d", ret);
      return ret;
   }
#endif

   return ret;

}  /* End of tmctl_allocQueueProfileId() */


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
tmctl_ret_e tmctl_freeQueueProfileId_plat(int queueProfileId)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: queueProfileId=%d ", queueProfileId);

#if defined(SUPPORT_BCMTM)
   ret = tmctlBcmTm_freeQueueProfileId(queueProfileId);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlBcmTm_freeQueueProfileId ERROR! ret=%d", ret);
      return ret;
   }
#endif

   return ret;

}  /* End of tmctl_freeQueueProfileId() */


/* ----------------------------------------------------------------------------
 * This function gets the queue profile of a queue profile index.
 *
 * Parameters:
 *    queueProfileId (IN) Queue ID.
 *    queueProfile_p (OUT) The drop algorithm configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getQueueProfile_plat(int                   queueProfileId,
                                       tmctl_queueProfile_t* queueProfile_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: qProfileId=%d", queueProfileId);

#if defined(SUPPORT_BCMTM)
   ret = tmctlBcmTm_getQueueProfile(queueProfileId, queueProfile_p);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlBcmTm_getQueueProfile ERROR! ret=%d", ret);
      return ret;
   }
#endif

   return ret;

} /* End of tmctl_getQueueProfile() */


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
tmctl_ret_e tmctl_setQueueProfile_plat(int                   queueProfileId,
                                       tmctl_queueProfile_t* queueProfile_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: queueProfileId=%d dropProb=%d minThreshold=%d maxThreshold=%d",
               queueProfileId, queueProfile_p->dropProb, queueProfile_p->minThreshold,
               queueProfile_p->maxThreshold);


#if defined(SUPPORT_BCMTM)
   ret = tmctlBcmTm_setQueueProfile(queueProfileId, queueProfile_p);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlBcmTm_setQueueProfile ERROR! ret=%d", ret);
      return ret;
   }
#endif

   return ret;

} /* End of tmctl_setQueueProfile() */

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
tmctl_ret_e tmctl_getQueueDropAlg_plat(tmctl_devType_e       devType,
                                       tmctl_if_t*           if_p,
                                       int                   queueId,
                                       tmctl_queueDropAlg_t* dropAlg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_getQueueDropAlg(if_p->ethIf.ifname, queueId, dropAlg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_getQueueDropAlg ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_getQueueDropAlg() */


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
tmctl_ret_e tmctl_setQueueDropAlgExt_plat(tmctl_devType_e          devType,
                                          tmctl_if_t*              if_p,
                                          int                      queueId,
                                          tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;
   tmctl_queueDropAlgExt_t* dropAlgLo_p = &dropAlg_p->dropAlgLo;
   tmctl_queueDropAlgExt_t* dropAlgHi_p = &dropAlg_p->dropAlgHi;

   tmctl_debug("Enter: devType=%d ifname=%s qid=%d dropAlgorithm=%d "
               "Lo(minThr=%d maxThr=%d pct=%d) "
               "Hi(minThr=%d maxThr=%d pct=%d)",
               devType, if_p->ethIf.ifname, queueId, dropAlg_p->dropAlgorithm,
               dropAlgLo_p->redMinThreshold, dropAlgLo_p->redMaxThreshold,
               dropAlgLo_p->redPercentage,
               dropAlgHi_p->redMinThreshold, dropAlgHi_p->redMaxThreshold,
               dropAlgHi_p->redPercentage);

   return ret;

}  /* End of tmctl_setQueueDropAlgExt() */


tmctl_ret_e tmctl_setQueueDropAlg_plat(tmctl_devType_e       devType,
                                       tmctl_if_t*           if_p,
                                       int                   queueId,
                                       tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d qid=%d dropAlgorithm=%d queueProfileIdLo=%d "
               "queueProfileIdHi=%d priorityMask0=0x%x priorityMask1=0x%x",
               devType, queueId, dropAlg_p->dropAlgorithm, dropAlg_p->queueProfileIdLo,
               dropAlg_p->queueProfileIdHi, dropAlg_p->priorityMask0,
               dropAlg_p->priorityMask1);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_setQueueDropAlg(if_p->ethIf.ifname, queueId, dropAlg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_setQueueDropAlg ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_setQueueDropAlg() */

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
tmctl_ret_e tmctl_getXtmChannelDropAlg_plat(tmctl_devType_e       devType,
                                            int                   channelId,
                                            tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d channelId=%d", devType, channelId);

   if (devType == TMCTL_DEV_XTM)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_getXtmChannelDropAlg(channelId, dropAlg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_getXtmChannelDropAlg ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_getXtmChannelDropAlg() */


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
tmctl_ret_e tmctl_setXtmChannelDropAlg_plat(tmctl_devType_e       devType,
                                            int                   channelId,
                                            tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d channelId=%d dropAlgorithm=%d queueProfileIdLo=%d "
               "queueProfileIdHi=%d priorityMask0=0x%x priorityMask1=0x%x",
               devType, channelId, dropAlg_p->dropAlgorithm, dropAlg_p->queueProfileIdLo,
               dropAlg_p->queueProfileIdHi, dropAlg_p->priorityMask0,
               dropAlg_p->priorityMask1);

   if (devType == TMCTL_DEV_XTM)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_setXtmChannelDropAlg(channelId, dropAlg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_setXtmChannelDropAlg ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_setXtmChannelDropAlg() */

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
tmctl_ret_e tmctl_getQueueStats_plat(tmctl_devType_e     devType,
                                     tmctl_if_t*         if_p,
                                     int                 queueId,
                                     tmctl_queueStats_t* stats_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_getQueueStats(if_p->ethIf.ifname, queueId, stats_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_getQueueStats ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_getQueueStats() */


/* ----------------------------------------------------------------------------
 * This function gets port TM parameters (capabilities).
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
tmctl_ret_e tmctl_getPortTmParms_plat(tmctl_devType_e      devType,
                                      tmctl_if_t*          if_p,
                                      tmctl_portTmParms_t* tmParms_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d", devType);

   if (devType == TMCTL_DEV_ETH)
   {
#if defined(SUPPORT_BCMTM)
      ret = tmctlBcmTm_getPortTmParms(if_p->ethIf.ifname, tmParms_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlBcmTm_getPortTmParms ERROR! ret=%d", ret);
         return ret;
      }
#endif
   }

   return ret;

}  /* End of tmctl_getPortTmParms() */


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
tmctl_ret_e tmctl_getDscpToPbit_plat(tmctl_dscpToPbitCfg_t* cfg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: ");

   return ret;

}  /* End of tmctl_getDscpToPbit() */


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
tmctl_ret_e tmctl_setDscpToPbit_plat(tmctl_dscpToPbitCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_debug("Enter: ");

    return ret;

}  /* End of tmctl_setDscpToPbit() */


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
tmctl_ret_e tmctl_getPbitToQ_plat(tmctl_devType_e devType,
                                 tmctl_if_t* if_p,
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_debug("Enter: devType=%d", devType);

    return ret;

}  /* End of tmctl_getPbitToQ() */


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
tmctl_ret_e tmctl_setPbitToQ_plat(tmctl_devType_e devType,
                                 tmctl_if_t* if_p,
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_debug("Enter: devType=%d", devType);

    return ret;

}  /* End of tmctl_setPbitToQ() */


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
tmctl_ret_e tmctl_getForceDscpToPbit_plat(tmctl_dir_e dir, BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_debug("Enter: dir=%d", dir);

    return ret;

}  /* End of tmctl_getForceDscpToPbit() */


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
tmctl_ret_e tmctl_setForceDscpToPbit_plat(tmctl_dir_e dir, BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_debug("Enter: dir=%d enable=%d", dir, (*enable_p));

    return ret;

}  /* End of tmctl_setForceDscpToPbit() */


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
tmctl_ret_e tmctl_getPktBasedQos_plat(tmctl_dir_e dir,
                                      tmctl_qosType_e type,
                                      BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_debug("Enter: dir=%d type=%d", dir, type);

    return ret;

}  /* End of tmctl_getPktBasedQos() */


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
tmctl_ret_e tmctl_setPktBasedQos_plat(tmctl_dir_e dir,
                                      tmctl_qosType_e type,
                                      BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_debug("Enter: dir=%d type=%d enable=%d", dir, type, (*enable_p));

    return ret;

}  /* End of tmctl_setPktBasedQos() */

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
tmctl_ret_e tmctl_setQueueSize_plat(tmctl_devType_e          devType,
                                    tmctl_if_t*        if_p,
                                    int                queueId,
                                    int                size)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   return ret;

}  /* End of tmctl_setQueueSize() */

/* ----------------------------------------------------------------------------
 * This function sets shaper of a queue.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p    (IN) Port identifier.
 *    queueId (IN) Queue ID.
 *    shaper  (IN) Queue Shaper configuration
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setQueueShaper_plat(tmctl_devType_e          devType,
                                      tmctl_if_t*        if_p,
                                      int                queueId,
                                      tmctl_shaper_t     *shaper_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    return ret;
}  /* End of tmctl_setQueueShaper() */


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

int tmctl_getDefQSize_plat(tmctl_devType_e devType, tmctl_dir_e dir)
{
    if (TMCTL_DIR_DN == dir)
        return TMCTL_DEF_ETH_Q_SZ_DS;

    return TMCTL_DEF_ETH_Q_SZ_US;
}

tmctl_ret_e tmctl_createPolicer_plat(tmctl_policer_t *policer_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    return ret;
}  /* End of tmctl_createPolicer_plat() */

tmctl_ret_e tmctl_modifyPolicer_plat(tmctl_policer_t *policer_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    return ret;
}  /* End of tmctl_modifyPolicer_plat() */

tmctl_ret_e tmctl_deletePolicer_plat(tmctl_dir_e dir, int policerId)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    return ret;
}  /* End of tmctl_deletePolicer_plat() */

tmctl_ret_e tmctl_getOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p,
                                     tmctl_shaper_t* shaper_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;
    
    return ret;
}  /* End of tmctl_getOverAllShaper_plat() */


tmctl_ret_e tmctl_setOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_shaper_t* shaper_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;
    
    return ret;
}  /* End of tmctl_setOverAllShaper_plat() */

tmctl_ret_e tmctl_linkOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;

    return ret;
}  /* End of tmctl_linkOverAllShaper_plat() */


tmctl_ret_e tmctl_unlinkOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;
    
    return ret;
}  /* End of tmctl_unLinkOverAllShaper_plat() */

