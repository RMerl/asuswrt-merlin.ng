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
#include "tmctl_api_plat.h"
#include "tmctl_api_trace.h"
#include "bcmctl_syslog.h"


#if defined(BCMCTL_SYSLOG_SUPPORTED)
IMPL_setSyslogLevel(tmctl);
IMPL_getSyslogLevel(tmctl);
IMPL_isSyslogLevelEnabled(tmctl);
IMPL_setSyslogMode(tmctl);
IMPL_isSyslogEnabled(tmctl);
#endif /* BCMCTL_SYSLOG_SUPPORTED */


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
tmctl_ret_e tmctl_portTmInit(tmctl_devType_e devType,
                             tmctl_if_t*     if_p,
                             uint32_t        cfgFlags,
                             int             numQueues)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_portTmInitTrace(devType, if_p, cfgFlags, numQueues);

   tmctl_debug("Enter: devType=%d", devType);

   ret = tmctl_portTmInit_plat(devType, if_p, cfgFlags, numQueues);

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
tmctl_ret_e tmctl_portTmUninit(tmctl_devType_e devType,
                               tmctl_if_t*     if_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_portTmUninitTrace(devType, if_p);

   tmctl_debug("Enter: devType=%d", devType);

   ret = tmctl_portTmUninit_plat(devType, if_p);

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
tmctl_ret_e tmctl_getQueueCfg(tmctl_devType_e   devType,
                              tmctl_if_t*       if_p,
                              int               queueId,
                              tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getQueueCfgTrace(devType, if_p, queueId, qcfg_p);

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);


   ret = tmctl_getQueueCfg_plat(devType, if_p, queueId, qcfg_p);

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
tmctl_ret_e tmctl_setQueueCfg(tmctl_devType_e   devType,
                              tmctl_if_t*       if_p,
                              tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;
#if (defined(BCM_PON_XRDP))
   int min_bufs = 0;
#if defined(CHIP_6856) || defined(CHIP_6878)
   int fpmPoolMemorySize;
   /*calculate minimum minimum buffer reservation(minimum MBR) according to tm memory and make sure MBd is >= minumum mbr */
   ret = tmctlRdpa_getMemoryInfo(&fpmPoolMemorySize);
   if (ret == TMCTL_SUCCESS)
   {
      switch (fpmPoolMemorySize)
      { 
         case 16:
            min_bufs = 8;
         break;

         case 32:
            min_bufs = 4;
         break;

         case 64:
            min_bufs = 2;
         break;

         case 128:
            min_bufs = 1;
         break;

         defaults:
            return TMCTL_ERROR;
      }
   }
   else
      return ret;
#else
   min_bufs = 1;
#endif

   
   if (qcfg_p->minBufs < min_bufs)
       qcfg_p->minBufs = min_bufs;
#endif

   tmctl_setQueueCfgTrace(devType, if_p, qcfg_p);

   tmctl_debug("Enter: devType=%d qid=%d priority=%d qsize=%d schedMode=%d wt=%d minBufs=%d minRate=%d kbps=%d mbs=%d",
               devType, qcfg_p->qid, qcfg_p->priority, qcfg_p->qsize, qcfg_p->schedMode,
               qcfg_p->weight, qcfg_p->minBufs, qcfg_p->shaper.minRate,
               qcfg_p->shaper.shapingRate, qcfg_p->shaper.shapingBurstSize);

   ret = tmctl_setQueueCfg_plat(devType, if_p, qcfg_p);

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
tmctl_ret_e tmctl_delQueueCfg(tmctl_devType_e devType,
                              tmctl_if_t*     if_p,
                              int             queueId)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_delQueueCfgTrace(devType, if_p, queueId);

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);

   ret = tmctl_delQueueCfg_plat(devType, if_p, queueId);

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
tmctl_ret_e tmctl_getPortShaper(tmctl_devType_e devType,
                                tmctl_if_t*     if_p,
                                tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getPortShaperTrace(devType, if_p, shaper_p);

   tmctl_debug("Enter: devType=%d", devType);

   ret = tmctl_getPortShaper_plat(devType, if_p, shaper_p);

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
tmctl_ret_e tmctl_setPortShaper(tmctl_devType_e devType,
                                tmctl_if_t*     if_p,
                                tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_setPortShaperTrace(devType, if_p, shaper_p);

   tmctl_debug("Enter: devType=%d portKbps=%d portMbs=%d", devType,
               shaper_p->shapingRate, shaper_p->shapingBurstSize);

   ret = tmctl_setPortShaper_plat(devType, if_p, shaper_p);

   return ret;

}  /* End of tmctl_setPortShaper() */

/* ----------------------------------------------------------------------------
 * This function gets the overall shaper configuration.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    shaper_p (OUT) The shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getOverAllShaper(tmctl_devType_e devType,
                                tmctl_if_t*     if_p,
                                tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getOverAllShaperTrace(devType, if_p, shaper_p);

   tmctl_debug("Enter: devType=%d", devType);

   ret = tmctl_getOverAllShaper_plat(devType, if_p, shaper_p);

   return ret;

}  /* End of tmctl_getOverAllShaper() */

/* ----------------------------------------------------------------------------
 * This function configures the overall shaper for shaping rate, shaping burst
 * size and max rate. 
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    shaper_p (IN) The shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setOverAllShaper(tmctl_devType_e devType,
                                tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_setOverAllShaperTrace(devType, shaper_p);

   tmctl_debug("Enter: devType=%d Rate=%d BurstSize=%d", devType, 
               shaper_p->shapingRate, shaper_p->shapingBurstSize);

   ret = tmctl_setOverAllShaper_plat(devType, shaper_p);

   return ret;

}  /* End of tmctl_setOverAllShaper() */

/* ----------------------------------------------------------------------------
 * This function link the port to the overall shaper.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_linkOverAllShaper(tmctl_devType_e devType,
                                tmctl_if_t*     if_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_linkOverAllShaperTrace(devType, if_p);

   tmctl_debug("Enter: devType=%d", devType);

   ret = tmctl_linkOverAllShaper_plat(devType, if_p);

   return ret;

}  /* End of tmctl_getPortShaper() */

/* ----------------------------------------------------------------------------
 * This function unlink the port from the overall shaper.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_unlinkOverAllShaper(tmctl_devType_e devType,
                                tmctl_if_t*     if_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_unlinkOverAllShaperTrace(devType, if_p);

   tmctl_debug("Enter: devType=%d", devType);

   ret = tmctl_unlinkOverAllShaper_plat(devType, if_p);

   return ret;

}  /* End of tmctl_getPortShaper() */


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
tmctl_ret_e tmctl_allocQueueProfileId(int* queueProfileId_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_allocQueueProfileIdTrace(queueProfileId_p);

   tmctl_debug("Enter: ");

   ret = tmctl_allocQueueProfileId_plat(queueProfileId_p);

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
tmctl_ret_e tmctl_freeQueueProfileId(int queueProfileId)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_freeQueueProfileIdTrace(queueProfileId);

   tmctl_debug("Enter: queueProfileId=%d ", queueProfileId);

   ret = tmctl_freeQueueProfileId_plat(queueProfileId);

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
tmctl_ret_e tmctl_getQueueProfile(int                   queueProfileId,
                                  tmctl_queueProfile_t* queueProfile_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getQueueProfileTrace(queueProfileId, queueProfile_p);

   tmctl_debug("Enter: qProfileId=%d", queueProfileId);

   ret = tmctl_getQueueProfile_plat(queueProfileId, queueProfile_p);

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
tmctl_ret_e tmctl_setQueueProfile(int                   queueProfileId,
                                  tmctl_queueProfile_t* queueProfile_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_setQueueProfileTrace(queueProfileId, queueProfile_p);

   tmctl_debug("Enter: queueProfileId=%d dropProb=%d minThreshold=%d maxThreshold=%d",
               queueProfileId, queueProfile_p->dropProb, queueProfile_p->minThreshold,
               queueProfile_p->maxThreshold);

   ret = tmctl_setQueueProfile_plat(queueProfileId, queueProfile_p);

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
tmctl_ret_e tmctl_getQueueDropAlg(tmctl_devType_e       devType,
                                  tmctl_if_t*           if_p,
                                  int                   queueId,
                                  tmctl_queueDropAlg_t* dropAlg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getQueueDropAlgTrace(devType, if_p, queueId, dropAlg_p);

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);

   ret = tmctl_getQueueDropAlg_plat(devType, if_p, queueId, dropAlg_p);

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
tmctl_ret_e tmctl_setQueueDropAlgExt(tmctl_devType_e          devType,
                                     tmctl_if_t*              if_p,
                                     int                      queueId,
                                     tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: devType=%d ifname=%s qid=%d dropAlgorithm=%d "
               "Lo(minThr=%d maxThr=%d pct=%d) "
               "Hi(minThr=%d maxThr=%d pct=%d)",
               devType, if_p->ethIf.ifname, queueId, dropAlg_p->dropAlgorithm,
               dropAlg_p->dropAlgLo.redMinThreshold, dropAlg_p->dropAlgLo.redMaxThreshold,
               dropAlg_p->dropAlgLo.redPercentage,
               dropAlg_p->dropAlgHi.redMinThreshold, dropAlg_p->dropAlgHi.redMaxThreshold,
               dropAlg_p->dropAlgHi.redPercentage);

   tmctl_setQueueDropAlgExtTrace(devType, if_p, queueId, dropAlg_p);

   ret = tmctl_setQueueDropAlgExt_plat(devType, if_p, queueId, dropAlg_p);

   return ret;

}  /* End of tmctl_setQueueDropAlgExt() */


tmctl_ret_e tmctl_setQueueDropAlg(tmctl_devType_e       devType,
                                  tmctl_if_t*           if_p,
                                  int                   queueId,
                                  tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_setQueueDropAlgTrace(devType, if_p, queueId, dropAlg_p);

   tmctl_debug("Enter: devType=%d qid=%d dropAlgorithm=%d queueProfileIdLo=%d "
               "queueProfileIdHi=%d priorityMask0=0x%x priorityMask1=0x%x",
               devType, queueId, dropAlg_p->dropAlgorithm, dropAlg_p->queueProfileIdLo,
               dropAlg_p->queueProfileIdHi, dropAlg_p->priorityMask0,
               dropAlg_p->priorityMask1);

   ret = tmctl_setQueueDropAlg_plat(devType, if_p, queueId, dropAlg_p);

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
tmctl_ret_e tmctl_getXtmChannelDropAlg(tmctl_devType_e       devType,
                                       int                   channelId,
                                       tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getXtmChannelDropAlgTrace(devType, channelId, dropAlg_p);

   tmctl_debug("Enter: devType=%d channelId=%d", devType, channelId);

   ret = tmctl_getXtmChannelDropAlg_plat(devType, channelId, dropAlg_p);

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
tmctl_ret_e tmctl_setXtmChannelDropAlg(tmctl_devType_e       devType,
                                       int                   channelId,
                                       tmctl_queueDropAlg_t* dropAlg_p)
{

   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_setXtmChannelDropAlgTrace(devType, channelId, dropAlg_p);

   tmctl_debug("Enter: devType=%d channelId=%d dropAlgorithm=%d queueProfileIdLo=%d "
               "queueProfileIdHi=%d priorityMask0=0x%x priorityMask1=0x%x",
               devType, channelId, dropAlg_p->dropAlgorithm, dropAlg_p->queueProfileIdLo,
               dropAlg_p->queueProfileIdHi, dropAlg_p->priorityMask0,
               dropAlg_p->priorityMask1);

   ret = tmctl_setXtmChannelDropAlg_plat(devType, channelId, dropAlg_p);

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
tmctl_ret_e tmctl_getQueueStats(tmctl_devType_e     devType,
                                tmctl_if_t*         if_p,
                                int                 queueId,
                                tmctl_queueStats_t* stats_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getQueueStatsTrace(devType, if_p, queueId, stats_p);

   tmctl_debug("Enter: devType=%d qid=%d", devType, queueId);

   ret = tmctl_getQueueStats_plat(devType, if_p, queueId, stats_p);

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
tmctl_ret_e tmctl_getPortTmParms(tmctl_devType_e      devType,
                                 tmctl_if_t*          if_p,
                                 tmctl_portTmParms_t* tmParms_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getPortTmParmsTrace(devType, if_p, tmParms_p);

   tmctl_debug("Enter: devType=%d", devType);

   ret = tmctl_getPortTmParms_plat(devType, if_p, tmParms_p);

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
tmctl_ret_e tmctl_getDscpToPbit(tmctl_dscpToPbitCfg_t* cfg_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_getDscpToPbitTrace();

   tmctl_debug("Enter: ");

   ret = tmctl_getDscpToPbit_plat(cfg_p);

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
tmctl_ret_e tmctl_setDscpToPbit(tmctl_dscpToPbitCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;
    int i = 0;

    tmctl_setDscpToPbitTrace(cfg_p);
    tmctl_debug("Enter: ");
    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        tmctl_debug("dscp[%d]=%d", i, cfg_p->dscp[i]);
    }
    tmctl_debug("remark %d", i, cfg_p->remark);

    ret = tmctl_setDscpToPbit_plat(cfg_p);

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
tmctl_ret_e tmctl_getPbitToQ(tmctl_devType_e devType,
                                 tmctl_if_t* if_p,
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_getPbitToQTrace(devType, if_p, cfg_p);

    tmctl_debug("Enter: devType=%d", devType);

    ret = tmctl_getPbitToQ_plat(devType, if_p, cfg_p);

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
tmctl_ret_e tmctl_setPbitToQ(tmctl_devType_e devType,
                                 tmctl_if_t* if_p,
                                 tmctl_pbitToQCfg_t* cfg_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;
    int i = 0;

    tmctl_setPbitToQTrace(devType, if_p, cfg_p);

    tmctl_debug("Enter: devType=%d", devType);

    for (i = 0; i < TOTAL_PBIT_NUM; i++)
    {
        tmctl_debug("pbit[%d]=%d", i, cfg_p->pbit[i]);
    }

    ret = tmctl_setPbitToQ_plat(devType, if_p, cfg_p);

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
tmctl_ret_e tmctl_getForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_getForceDscpToPbitTrace(dir, enable_p);

    tmctl_debug("Enter: dir=%d", dir);

    ret = tmctl_getForceDscpToPbit_plat(dir, enable_p);

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
tmctl_ret_e tmctl_setForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_setForceDscpToPbitTrace(dir, enable_p);

    tmctl_debug("Enter: dir=%d enable=%d", dir, (*enable_p));

    ret = tmctl_setForceDscpToPbit_plat(dir, enable_p);

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
tmctl_ret_e tmctl_getPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_getPktBasedQosTrace(dir, type, enable_p);

    tmctl_debug("Enter: dir=%d type=%d", dir, type);

    ret = tmctl_getPktBasedQos_plat(dir, type, enable_p);

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
tmctl_ret_e tmctl_setPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_setPktBasedQosTrace(dir, type, enable_p);

    tmctl_debug("Enter: dir=%d type=%d enable=%d", dir, type, (*enable_p));

    ret = tmctl_setPktBasedQos_plat(dir, type, enable_p);

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
tmctl_ret_e tmctl_setQueueSize(tmctl_devType_e          devType,
                                     tmctl_if_t*        if_p,
                                     int                queueId,
                                     int                size)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;
   tmctl_setQueueSizeTrace(devType, if_p, queueId, size);

   ret = tmctl_setQueueSize_plat(devType, if_p, queueId, size);

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
tmctl_ret_e tmctl_setQueueShaper(tmctl_devType_e          devType,
                                     tmctl_if_t*        if_p,
                                     int                queueId,
                                     tmctl_shaper_t     *shaper_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;

    tmctl_setQueueSizeShaperTrace(devType, if_p, queueId, shaper_p);

    ret = tmctl_setQueueShaper_plat(devType, if_p, queueId, shaper_p);

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

int tmctl_getDefQSize(tmctl_devType_e devType, tmctl_dir_e dir)
{
    return tmctl_getDefQSize_plat(devType, dir);
}

/* ----------------------------------------------------------------------------
 * This function return the default queue size according to tm memory.
 *
 * Parameters:
 *
 * Return:
 *    queue size.
 * ----------------------------------------------------------------------------
 */
#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
int tmctl_getAutoQSize(void)
{
    int fpmPoolMemorySize;
    tmctl_ret_e ret;
    ret = tmctlRdpa_getMemoryInfo(&fpmPoolMemorySize);
    if (ret == TMCTL_SUCCESS)
    {
        if (fpmPoolMemorySize >= 32)
        { 
            return TMCTL_DEF_Q_SIZE_1_MB;
        }
        else
        {
            return TMCTL_DEF_Q_SIZE_400KB;
        }
    }
    tmctl_debug("tmctl_getAutoQSize failed, return default 32 MB\n");
    return 32;
}
#endif

/* ----------------------------------------------------------------------------
 * This function creates a policer.
 *
 * Parameters:
 *    tmctl_policer_t (IN) tmctl policer.
 *    Note: policerId in tmctl_policer_t can be input and output parameter
 *          If policerId is expected as output, please use TMCTL_INVALID_KEY as input.
 *          Then you can get the system allocated index in policerId.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_createPolicer(tmctl_policer_t *policer_p)
{
    return tmctl_createPolicer_plat(policer_p);
}

/* ----------------------------------------------------------------------------
 * This function modify a policer.
 *
 * Parameters:
 *    tmctl_policer_t (IN) tmctl policer
 *    Note: policerId, dir and type can not be modified.
 *          Please use TMCTL_INVALID_KEY to indicate the value is not meant to be modified.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_modifyPolicer(tmctl_policer_t *policer_p)
{
    return tmctl_modifyPolicer_plat(policer_p);
}

/* ----------------------------------------------------------------------------
 * This function deletes a policer.
 *
 * Parameters:
 *    dir       (IN) direction.
 *    policerId (IN) index
 *    Note: Dir and policerId compose the key of the policer to be deleted.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_deletePolicer(tmctl_dir_e dir, int policerId)
{
    return tmctl_deletePolicer_plat(dir, policerId);
}


