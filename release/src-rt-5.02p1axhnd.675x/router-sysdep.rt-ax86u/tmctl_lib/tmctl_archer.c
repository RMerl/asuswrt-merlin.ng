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
#include <sys/ioctl.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "os_defs.h"
#include "tmctl_ethsw.h"
#include "tmctl_archer.h"

#include "archer.h"

static int archer_cmd_send(archer_ioctl_cmd_t cmd, unsigned long arg)
{
    int ret;
    int fd;

    fd = open(ARCHER_DRV_DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        fprintf( stderr, "%s: %s", ARCHER_DRV_DEVICE_NAME, strerror(errno) );

        return -1;
    }

    ret = ioctl(fd, cmd, arg);
    if(ret)
    {
        fprintf( stderr, "ioctl: %s\n", strerror(errno) );
    }

    close(fd);

    return ret;
}


/* ----------------------------------------------------------------------------
 * This function initializes the Archer QoS configuration for a port.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlArcher_TmInit(const char* ifname)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   tmctl_shaper_t shaper;
   int i;

   tmctl_debug("Enter: ifname=%s", ifname);

   /* Reset port sched mode to sp */
   ret = tmctlEthSw_resetPortSched(ifname);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_resetPortSched ERROR!");
      return ret;
   }

   /* Reset port shaper */
   memset(&shaper, 0, sizeof(tmctl_shaper_t));

   ret = tmctlEthSw_setPortShaper(ifname, &shaper);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_setPortShaper ERROR! ret=%d", ret);
      return ret;
   }

   /* Reset queue shaper */
   for(i = 0; i < BCM_COS_COUNT; i++)
   {
      ret = tmctlEthSw_setQueueShaper(ifname, i, &shaper);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlEthSw_setQueueShaper ERROR!");
         return ret;
      }
   }   
   
   return ret;
}


/* ----------------------------------------------------------------------------
 * This function un-initializes the Archer QoS configuration for a port.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlArcher_TmUninit(const char* ifname)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   tmctl_debug("Enter: ifname=%s", ifname);

   ret = tmctlArcher_TmInit(ifname);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlArcher_TmInit ERROR!");
      return ret;
   }
   
   return ret;
}


/* ----------------------------------------------------------------------------
 * This function gets the configuration of a Archer QoS queue.
 *
 * Parameters:
 *    ifname (IN)  Linux interface name.
 *    qid (IN)     queue id.
 *    qcfg_p (OUT) structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlArcher_getQueueCfg(const char* ifname, int qid,
                                    tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   tmctl_portQcfg_t portQcfg;

   tmctl_debug("Enter: ifname=%s qid=%d", ifname, qid);

   if (qid >= BCM_COS_COUNT)
   {
      tmctl_error("qid should be smaller than %d, input qid=%d", BCM_COS_COUNT, qid);
      return TMCTL_ERROR;
   }
   
   ret = tmctlEthSw_getPortSched(ifname, &portQcfg);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_getPortSched ERROR!");
      return ret;
   }
   
   memset(qcfg_p, 0, sizeof(tmctl_queueCfg_t));
   qcfg_p->schedMode = portQcfg.qcfg[qid].schedMode;
   qcfg_p->priority = portQcfg.qcfg[qid].priority;
   qcfg_p->weight = portQcfg.qcfg[qid].weight;
   qcfg_p->qid = qid;

   /*TODO: Get queue size using archerctl api.
     Before that, qsize is hardcoded to ARCHER_DMA_NUM_TX_PKT_DESC(512)
     which is defined archer iudma driver. */
   qcfg_p->qsize = 512;

   ret = tmctlEthSw_getQueueShaper(ifname, qid, &(qcfg_p->shaper));
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_getQueueShaper ERROR!");
      return ret;
   }

   tmctl_debug("Done: ifname=%s qid=%d priority=%d schedMode=%d qsize=%d wt=%d shapingRate=%d burstSize=%d minRate=%d",
               ifname, qcfg_p->qid, qcfg_p->priority, qcfg_p->schedMode, qcfg_p->qsize, qcfg_p->weight,
               qcfg_p->shaper.shapingRate, qcfg_p->shaper.shapingBurstSize, qcfg_p->shaper.minRate);

   return TMCTL_SUCCESS;

}


/* ----------------------------------------------------------------------------
 * This function sets the configuration of a queue.
 *
 * Parameters:
 *    ifname (IN)    Linux interface name.
 *    tmParms_p (IN) port tm parameters.
 *    qcfg_p (IN)    structure containing the queue config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlArcher_setQueueCfg(const char* ifname,
                                    tmctl_portTmParms_t* tmParms_p,
                                    tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   tmctl_portQcfg_t portQcfg;
   tmctl_portQcfg_t newPortQcfg;

   tmctl_debug("Enter: ifname=%s qid=%d priority=%d schedMode=%d qsize=%d wt=%d shapingRate=%d burstSize=%d minRate=%d",
               ifname, qcfg_p->qid, qcfg_p->priority, qcfg_p->schedMode, qcfg_p->qsize, qcfg_p->weight,
               qcfg_p->shaper.shapingRate, qcfg_p->shaper.shapingBurstSize, qcfg_p->shaper.minRate);

   if((qcfg_p->schedMode == TMCTL_SCHED_SP) && (qcfg_p->qid != qcfg_p->priority))
   {
      tmctl_error("qid and priority of SP queue should be the same, qid=%d, priority=%d", qcfg_p->qid, qcfg_p->priority);
      return TMCTL_ERROR;
   }   

   ret = tmctlEthSw_getPortSched(ifname, &portQcfg);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_getPortSched ERROR!");
      return ret;
   }

   ret = tmctlEthSw_determineNewPortQcfg(tmParms_p, qcfg_p, &portQcfg, &newPortQcfg);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlArcher_determineNewPortQcfg ERROR! ifname=%s ret=%d", ifname, ret);
      return ret;
   }

   ret = tmctlEthSw_setPortSched(ifname, &newPortQcfg);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_setPortSched ERROR!");
      return ret;
   }

   ret = tmctlEthSw_setQueueShaper(ifname, qcfg_p->qid, &(qcfg_p->shaper));
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_setQueueShaper ERROR!");
      return ret;
   }

   return ret;
}


/* ----------------------------------------------------------------------------
 * This function gets the drop algorithm of a Archer QoS queue.
 *
 * Parameters:
 *    devType (IN)    tmctl device type.
 *    qid (IN)        queue id.
 *    dropAlg_p (OUT) structure to receive the drop algorithm parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlArcher_getQueueDropAlg(int devType, const char* ifname, int qid,
                                        tmctl_queueDropAlg_t* dropAlg_p)
{
    archer_drop_ioctl_t drop_ioctl;
    archer_drop_config_t *config_p = &drop_ioctl.config;
    archer_ioctl_cmd_t ioctl_cmd;

    switch(devType)
    {
        case TMCTL_DEV_XTM:
            strncpy(drop_ioctl.if_name, "xtm", ARCHER_IFNAMSIZ);
            ioctl_cmd = ARCHER_IOC_XTMDROPALG_GET;
            break;

        case TMCTL_DEV_ETH:
            strncpy(drop_ioctl.if_name, ifname, ARCHER_IFNAMSIZ);
            ioctl_cmd = ARCHER_IOC_ENETDROPALG_GET;
            break;

        default:
            tmctl_error("Invalid devType: %d", devType);
            return TMCTL_UNSUPPORTED;
    }

    drop_ioctl.queue_id = qid;

    if(archer_cmd_send(ioctl_cmd, (unsigned long)&drop_ioctl))
    {
        tmctl_error("Could not archer_cmd_send");
        return TMCTL_ERROR;
    }

    dropAlg_p->dropAlgorithm = config_p->algorithm;
    dropAlg_p->queueProfileIdLo = 0;
    dropAlg_p->queueProfileIdHi = 0;
    dropAlg_p->priorityMask0 = config_p->priorityMask_0;
    dropAlg_p->priorityMask1 = config_p->priorityMask_1;
    dropAlg_p->dropAlgLo.redMinThreshold = config_p->profile[ARCHER_DROP_PROFILE_LOW].minThres;
    dropAlg_p->dropAlgLo.redMaxThreshold = config_p->profile[ARCHER_DROP_PROFILE_LOW].maxThres;
    dropAlg_p->dropAlgLo.redPercentage = config_p->profile[ARCHER_DROP_PROFILE_LOW].dropProb;
    dropAlg_p->dropAlgHi.redMinThreshold = config_p->profile[ARCHER_DROP_PROFILE_HIGH].minThres;
    dropAlg_p->dropAlgHi.redMaxThreshold = config_p->profile[ARCHER_DROP_PROFILE_HIGH].maxThres;
    dropAlg_p->dropAlgHi.redPercentage = config_p->profile[ARCHER_DROP_PROFILE_HIGH].dropProb;

    return TMCTL_SUCCESS;
}


/* ----------------------------------------------------------------------------
 * This function sets the drop algorithm of a Archer QoS queue.
 *
 * Parameters:
 *    devType (IN)   tmctl device type.
 *    qid (IN)       queue id.
 *    dropAlg_p (IN) structure to receive the drop algorithm parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlArcher_setQueueDropAlg(int devType, const char* ifname, int qid,
                                        tmctl_queueDropAlg_t* dropAlg_p)
{
    archer_drop_ioctl_t drop_ioctl;
    archer_drop_config_t *config_p = &drop_ioctl.config;
    archer_ioctl_cmd_t ioctl_cmd;

    switch(devType)
    {
        case TMCTL_DEV_XTM:
            strncpy(drop_ioctl.if_name, "xtm", ARCHER_IFNAMSIZ);
            ioctl_cmd = ARCHER_IOC_XTMDROPALG_SET;
            break;

        case TMCTL_DEV_ETH:
            strncpy(drop_ioctl.if_name, ifname, ARCHER_IFNAMSIZ);
            ioctl_cmd = ARCHER_IOC_ENETDROPALG_SET;
            break;

        default:
            tmctl_error("Invalid devType: %d", devType);
            return TMCTL_UNSUPPORTED;
    }

    drop_ioctl.queue_id = qid;

    config_p->algorithm = dropAlg_p->dropAlgorithm;
    config_p->priorityMask_0 = dropAlg_p->priorityMask0;
    config_p->priorityMask_1 = dropAlg_p->priorityMask1;
    config_p->profile[ARCHER_DROP_PROFILE_LOW].minThres = dropAlg_p->dropAlgLo.redMinThreshold;
    config_p->profile[ARCHER_DROP_PROFILE_LOW].maxThres = dropAlg_p->dropAlgLo.redMaxThreshold;
    config_p->profile[ARCHER_DROP_PROFILE_LOW].dropProb = dropAlg_p->dropAlgLo.redPercentage;
    config_p->profile[ARCHER_DROP_PROFILE_HIGH].minThres = dropAlg_p->dropAlgHi.redMinThreshold;
    config_p->profile[ARCHER_DROP_PROFILE_HIGH].maxThres = dropAlg_p->dropAlgHi.redMaxThreshold;
    config_p->profile[ARCHER_DROP_PROFILE_HIGH].dropProb = dropAlg_p->dropAlgHi.redPercentage;

    if(archer_cmd_send(ioctl_cmd, (unsigned long)&drop_ioctl))
    {
        tmctl_error("Could not archer_cmd_send");
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}
/* ----------------------------------------------------------------------------
 * This function gets the queue statistics of an Archer tx queue.
 * ARCHER_IOC_ENETTXQSTATS_GET gets the stats based on queue remapping.
 * So the stats of remapped queues will be the same.
 *
 * Parameters:
 *    devType (IN)  tmctl device type.
 *    ifname (IN)   Linux interface name.
 *    qid (IN)      Queue ID.
 *    stats_p (OUT) structure to receive the queue statistics.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlArcher_getQueueStats(int devType, const char* ifname, int qid,
                                      tmctl_queueStats_t* stats_p)
{
    archer_txq_stats_ioctl_t stats_ioctl;
    archer_ioctl_cmd_t ioctl_cmd;

    switch(devType)
    {
        case TMCTL_DEV_XTM:
            strncpy(stats_ioctl.if_name, "xtm", ARCHER_IFNAMSIZ);
            ioctl_cmd = ARCHER_IOC_XTMTXQSTATS_GET;
            break;

        case TMCTL_DEV_ETH:
            strncpy(stats_ioctl.if_name, ifname, ARCHER_IFNAMSIZ);
            ioctl_cmd = ARCHER_IOC_ENETTXQSTATS_GET;
            break;

        default:
            tmctl_error("Invalid devType: %d", devType);
            return TMCTL_UNSUPPORTED;
    }

    stats_ioctl.queue_id = qid;

    if(archer_cmd_send(ioctl_cmd, (unsigned long)&stats_ioctl))
    {
        tmctl_error("Could not archer_cmd_send");
        return TMCTL_ERROR;
    }

    stats_p->txPackets = stats_ioctl.stats.txPackets;
    stats_p->txBytes = stats_ioctl.stats.txBytes;
    stats_p->droppedPackets = stats_ioctl.stats.droppedPackets;
    stats_p->droppedBytes = stats_ioctl.stats.droppedBytes;

    return TMCTL_SUCCESS;
    
} /* End of tmctlArcher_getQueueStats() */

