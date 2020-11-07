/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2019:proprietary:standard

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

#include "tmctl_sysporttm.h"


/* ----------------------------------------------------------------------------
 * This function gets Sysport TM capabilities from Sysport driver.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    tmParms_p (OUT) structure to return Sysport TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getPortTmParms(const char* ifname,
                                          tmctl_portTmParms_t* tmParms_p)
{
   tmctl_debug("Enter: ifname=%s", ifname);

   memset(tmParms_p, 0, sizeof(tmctl_portTmParms_t));

   tmParms_p->schedCaps   = TMCTL_SP_CAPABLE | TMCTL_WFQ_CAPABLE;
   tmParms_p->maxQueues   = TMCTL_SYSPORT_TM_MAX_QUEUES;
   tmParms_p->maxSpQueues = TMCTL_SYSPORT_TM_MAX_QUEUES;
   tmParms_p->portShaper  = TRUE;
   tmParms_p->queueShaper = TRUE;

   return TMCTL_SUCCESS;
}  /* End of tmctlSysportTm_getPortTmParms() */


/* ----------------------------------------------------------------------------
 * This function initializes the Sysport TM configuration for a port.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_portTmInit(const char* ifname)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int i;

   tmctl_debug("Enter: ifname=%s", ifname);

   /* Enable Sysport TM */
   ret = archer_sysport_tm_enable();
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_enable ERROR! ret=%d", ret);
      return ret;
   }

   /* Rate could be set only in MANUAL mode */
   ret = archer_sysport_tm_mode_set(ifname, SYSPORT_TM_MODE_MANUAL);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_mode_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   } 

   /* Reset Sysport TM port shaper */
   ret = archer_sysport_tm_port_set(ifname, TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS, TMCTL_SYSPORT_TM_DEFAULT_SHAPER_MBS);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_port_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   /* Change back to AUTO mode after rate is set */
   ret = archer_sysport_tm_mode_set(ifname, SYSPORT_TM_MODE_AUTO);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_mode_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   /* Set Sysport TM arbiter to SP */
   ret = archer_sysport_tm_arbiter_set(ifname, SYSPORT_TM_ARBITER_SP);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_arbiter_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   /* Reset Sysport TM queue shaper to no shaping*/
   for (i = 0; i < TMCTL_SYSPORT_TM_MAX_QUEUES; i++)
   {
      ret = archer_sysport_tm_queue_set(ifname, i, 0, 0, TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS, TMCTL_SYSPORT_TM_DEFAULT_SHAPER_MBS);
      if (ret != TMCTL_SUCCESS)
      {
         tmctl_error("archer_sysport_tm_queue_set ERROR! ifname=%s ret=%d", ifname, ret);
         return ret;
      }
   }
   return ret;

}  /* End of tmctlSysportTm_portTmInit() */


/* ----------------------------------------------------------------------------
 * This function gets the configuration of a Sysport TM queue.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    qid (IN) Queue ID.
 *    qcfg_p (OUT) structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getQueueCfg(const char* ifname,
                                       int qid,
                                       tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   sysport_tm_arbiter_t arbiter;
   int min_kbps;
   int min_mbs;
   int max_kbps;
   int max_mbs;

   tmctl_debug("Enter: ifname=%s qid=%d", ifname, qid);

   ret = archer_sysport_tm_arbiter_get(ifname, &arbiter);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_arbiter_get ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   ret = archer_sysport_tm_queue_get(ifname, qid,
                                     &min_kbps, &min_mbs,
                                     &max_kbps, &max_mbs);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_queue_get ERROR! ifname=%s qid=%d ret=%d", ifname, qid, ret);
      return ret;
   }

   memset(qcfg_p, 0, sizeof(tmctl_queueCfg_t));

   if (arbiter == SYSPORT_TM_ARBITER_WFQ)
   {
      qcfg_p->schedMode = TMCTL_SCHED_WFQ;
      qcfg_p->priority  = 0;
      qcfg_p->weight = 1;
   }
   else
   {
      qcfg_p->schedMode = TMCTL_SCHED_SP;
      qcfg_p->priority  = qid;
      qcfg_p->weight = 0;
   }
   qcfg_p->qid = qid;
   qcfg_p->qsize = SYSPORT_TM_QUEUE_SIZE;

   if((max_kbps <= 0) || (max_kbps >= TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS))
   {
      qcfg_p->shaper.shapingRate = 0; /* means no shaping */
   }
   else
   {
      qcfg_p->shaper.shapingRate = max_kbps;
   }
   qcfg_p->shaper.shapingBurstSize = max_mbs;
   qcfg_p->shaper.minRate = min_kbps;

   tmctl_debug("Done: ifname=%s qid=%d priority=%d schedMode=%d qsize=%d wt=%d shapingRate=%d burstSize=%d minRate=%d",
               ifname, qcfg_p->qid, qcfg_p->priority, qcfg_p->schedMode, qcfg_p->qsize, qcfg_p->weight,
               qcfg_p->shaper.shapingRate, qcfg_p->shaper.shapingBurstSize, qcfg_p->shaper.minRate);

   return TMCTL_SUCCESS;

}  /* End of tmctlSysportTm_getQueueCfg() */


/* ----------------------------------------------------------------------------
 * This function sets the configuration of a Sysport TM queue.
 *
 * Parameters:
 *    ifname (IN) Linux interface name. 
 *    tmParms_p (IN) port tm parameters.
 *    qcfg_p (IN) structure containing the queue config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_setQueueCfg(const char* ifname,
                                       tmctl_portTmParms_t* tmParms_p,
                                       tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   sysport_tm_arbiter_t arbiter;
   int min_kbps;
   int min_mbs;
   int max_kbps;
   int max_mbs;

   tmctl_debug("Enter: ifname=%s qid=%d priority=%d schedMode=%d qsize=%d wt=%d shapingRate=%d burstSize=%d minRate=%d",
               ifname, qcfg_p->qid, qcfg_p->priority, qcfg_p->schedMode, qcfg_p->qsize, qcfg_p->weight,
               qcfg_p->shaper.shapingRate, qcfg_p->shaper.shapingBurstSize, qcfg_p->shaper.minRate);

   if(qcfg_p->schedMode == TMCTL_SCHED_WFQ)
   {
      arbiter = SYSPORT_TM_ARBITER_WFQ;
   }
   else
   {
      arbiter = SYSPORT_TM_ARBITER_SP;
   }

   ret = archer_sysport_tm_arbiter_set(ifname, arbiter);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_arbiter_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   if(qcfg_p->shaper.shapingRate <= 0 || qcfg_p->shaper.shapingRate >= TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS)
   {
      max_kbps = TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS;
   }
   else
   {
      max_kbps = qcfg_p->shaper.shapingRate;
   }

   if(qcfg_p->shaper.shapingBurstSize <= 0)
   {
      max_mbs = TMCTL_SYSPORT_TM_DEFAULT_SHAPER_MBS;
   }
   else
   {
      max_mbs = qcfg_p->shaper.shapingBurstSize;
   }
   
   min_kbps = qcfg_p->shaper.minRate;
   min_mbs = max_mbs;
   
   ret = archer_sysport_tm_queue_set(ifname, qcfg_p->qid,
                                     min_kbps, min_mbs,
                                     max_kbps, max_mbs);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_queue_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   return ret;

}  /* End of tmctlSysportTm_setQueueCfg() */


/* ----------------------------------------------------------------------------
 * This function gets the Sysport TM port shaping rate.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    shaper_p (OUT) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getPortShaper(const char* ifname,
                                         tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int kbps;
   int mbs;

   tmctl_debug("Enter: ifname=%s", ifname);

   ret = archer_sysport_tm_port_get(ifname, &kbps, &mbs);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_queue_get ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   if((kbps <= 0) || (kbps >= TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS))
   {
      shaper_p->shapingRate = 0; /* means no shaping */
   }
   else
   {
      shaper_p->shapingRate = kbps;
   }

   shaper_p->shapingBurstSize = mbs;
   shaper_p->minRate = 0;  /* not supported */

   tmctl_debug("Done: ifname=%s shapingRate=%d shapingBurstSize=%d minRate=%d",
               ifname, shaper_p->shapingRate, shaper_p->shapingBurstSize, shaper_p->minRate);

   return ret;

}  /* End of tmctlSysportTm_getPortShaper() */


/* ----------------------------------------------------------------------------
 * This function sets the port shaping rate. If the specified shaping rate
 * is greater than 0, Sysport TM mode will be switched from auto to manual.
 * And the shaper rate will be set to this value. Otherwise, the shaper rate
 * of manual mode will be set according to 1Gbps and change back to auto mode.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    shaper_p (IN) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_setPortShaper(const char* ifname,
                                         tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int kbps;
   int mbs;

   tmctl_debug("Enter: ifname=%s shapingRate=%d shapingBurstSize=%d minRate=%d",
               ifname, shaper_p->shapingRate, shaper_p->shapingBurstSize, shaper_p->minRate);

   if(shaper_p->shapingRate <= 0 || shaper_p->shapingRate >= TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS)
   {
      kbps = TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS;
   }
   else
   {
      kbps = shaper_p->shapingRate;
   }

   if(shaper_p->shapingBurstSize <= 0)
   {
      mbs = TMCTL_SYSPORT_TM_DEFAULT_SHAPER_MBS;
   }
   else
   {
      mbs = shaper_p->shapingBurstSize;
   }

   /* Rate could be set only in MANUAL mode */
   ret = archer_sysport_tm_mode_set(ifname, SYSPORT_TM_MODE_MANUAL);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_mode_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   /* Set port shaper rate and burst size */
   ret = archer_sysport_tm_port_set(ifname, kbps, mbs);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_port_set ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   if(TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS == kbps)
   {
      /* If no shaping, change back to AUTO mode after rate is set */
      ret = archer_sysport_tm_mode_set(ifname, SYSPORT_TM_MODE_AUTO);
      if (ret != TMCTL_SUCCESS)
      {
         tmctl_error("archer_sysport_tm_mode_set ERROR! ifname=%s ret=%d", ifname, ret);
         return ret;
      }      
   }

   return ret;

}  /* End of tmctlSysportTm_setPortShaper() */


/* ----------------------------------------------------------------------------
 * This function gets the queue statistics of a Sysport TM queue.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    qid (IN) Queue ID.
 *    stats_p (OUT) structure to receive the queue statistics.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getQueueStats(const char* ifname,
                                         int qid,
                                         tmctl_queueStats_t* stats_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   sysport_tm_txq_stats_t qStats;

   tmctl_debug("Enter: ifname=%s qid=%d", ifname, qid);

   memset(&qStats, 0, sizeof(sysport_tm_txq_stats_t));

   ret = archer_sysport_tm_stats_get(ifname, qid,
                                     &qStats.txPackets, &qStats.txBytes, 
                                     &qStats.droppedPackets, &qStats.droppedBytes);
   if (ret != TMCTL_SUCCESS)
   {
      tmctl_error("archer_sysport_tm_stats_get ERROR! ifname=%s qid=%d ret=%d", ifname, qid, ret);
      return ret;
   }

   stats_p->txPackets = qStats.txPackets;
   stats_p->txBytes = qStats.txBytes;
   stats_p->droppedPackets = qStats.droppedPackets;
   stats_p->droppedBytes = qStats.droppedBytes;

   tmctl_debug("Done: ifname=%s qid=%d txPackets=%u txBytes=%u droppedPackets=%u droppedBytes=%u",
               ifname, qid, stats_p->txPackets, stats_p->txBytes,
               stats_p->droppedPackets, stats_p->droppedBytes);

   return TMCTL_SUCCESS;

} /* End of tmctlSysportTm_getQueueStats() */


