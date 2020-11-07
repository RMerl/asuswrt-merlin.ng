/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Corporation
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

#include "tmctl_api.h"
#include "tmctl_xtm.h"
#include "devctl_xtm.h"

#define INVALID_ID          -1
/***************************************************************************
 * Function Name: GetConnAddrsToUse
 * Description  : Returns an array XTM_ADDR structures that the caller should
 *                use.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static void getconn_addrs(PXTM_ADDR *ppAddrs,uint32_t *pnumaddrs)
{
    PXTM_ADDR pAddrs = NULL;
    uint32_t ulNumAddrs = 0;

    *ppAddrs = NULL;
    *pnumaddrs = 0;

    /* No connection address was passed on the commnd line.  Use all
     * configured connections.
     */
    devCtl_xtmGetConnAddrs( NULL, &ulNumAddrs );

    if( ulNumAddrs )
    {
        pAddrs = (PXTM_ADDR) malloc(ulNumAddrs * sizeof(XTM_ADDR)); 

        /* Get the addresses of all configured connections. */
        if( pAddrs )
        {
            devCtl_xtmGetConnAddrs( pAddrs, &ulNumAddrs );
            *ppAddrs = pAddrs;
            *pnumaddrs = ulNumAddrs;
            tmctl_debug("pnumaddrs:%d ulPortMask:%08x ulPtmPriority:%d\n",*pnumaddrs,
                         pAddrs->u.Conn.ulPortMask,
                         pAddrs->u.Conn.ulPtmPriority);
        }
    }

    return;
} /* getconn_addrs */

tmctl_ret_e tmctl_xtm_getTmParms(tmctl_portTmParms_t* tmParms_ps)
{
   tmParms_ps->maxQueues    = XTM_MAX_QOS_QUEUES;
   tmParms_ps->maxSpQueues  = XTM_MAX_QOS_QUEUES;
   tmParms_ps->portShaper   = FALSE;
   tmParms_ps->queueShaper  = FALSE;
   tmParms_ps->dualRate     = FALSE;
   tmParms_ps->cfgFlags     = TMCTL_INIT_DEFAULT_QUEUES | \
                              TMCTL_QIDPRIO_MAP_Q7P7 | \
                              TMCTL_SCHED_TYPE_SP;
//  tmParms_ps->cfgFlags     = TMCTL_INIT_DEFAULT_QUEUES | \
//                             TMCTL_QIDPRIO_MAP_Q7P7 | \
//                             TMCTL_SCHED_TYPE_SP | \
//                             TMCTL_SCHED_TYPE_WRR | \
//                             TMCTL_SCHED_TYPE_WFQ;
   tmParms_ps->schedCaps    = TMCTL_SP_CAPABLE | \
                              TMCTL_WRR_CAPABLE | \
                              TMCTL_WFQ_CAPABLE;
   return TMCTL_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * This function Remove the Runner queue from the rdpactl config driver.
 *
 * Parameters:
 *    qid (IN) Queue ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_xtm_remQueueCfg(int qid)
{
   PXTM_ADDR pxtm_addr;
   XTM_CONN_CFG conn_cfg;
   uint32_t num_addrs;
   int  rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int found;

   pxtm_addr = NULL;
   num_addrs = 0;
   found = FALSE;

   if((qid < XTM_MIN_QOS_QUEUE_IDX) || (qid > XTM_MAX_QOS_QUEUE_IDX))
   {
      tmctl_error("qid is out of range and invalid\n");
      return TMCTL_ERROR;
   }

   memset(&conn_cfg,0x0,sizeof(XTM_CONN_CFG));
   getconn_addrs(&pxtm_addr,&num_addrs);

   if((pxtm_addr != NULL) && (num_addrs > 0))
   {
      rc = devCtl_xtmGetConnCfg(pxtm_addr,&conn_cfg);
      if(rc == CMSRET_SUCCESS)
      {
         if(conn_cfg.ulTransmitQParmsSize >= qid)
         {
            if(conn_cfg.TransmitQParms[qid].ucQosQId == qid)
            {
               //Here the assumption is the qid matches the priority.
               //So when deleting a queue means we are deleting the last queue
               //that was created.Once the qid is maintained properly we will
               //allow deleting any queue.
               found = TRUE;
               memset(&conn_cfg.TransmitQParms[qid],0x0,sizeof(XTM_TRANSMIT_QUEUE_PARMS));
               conn_cfg.ulTransmitQParmsSize--;
            }
         }
         else
         {
            found = FALSE;
         }
         if(found)
         {
            rc = devCtl_xtmSetConnCfg(pxtm_addr,&conn_cfg);
            if(rc != CMSRET_SUCCESS)
            {
               tmctl_error("");
               ret = TMCTL_ERROR;
            }
         }
         else
         {
            ret = TMCTL_NOT_FOUND;
         }
      }
      else
      {
         ret = TMCTL_ERROR;
      }
   }

   return ret;
}

/* ----------------------------------------------------------------------------
 * This function gets the configuration of a xtm connection configuration. 
 *
 * Parameters:
 *    qid (IN) Queue ID.
 *    tmId_p (OUT) tmId of the queue.
 *    qcfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_xtm_getQueueCfg(int qid, int* tmId_p ,tmctl_queueCfg_t* qcfg_p)
{
   PXTM_ADDR pxtm_addr;
   XTM_CONN_CFG conn_cfg;
   uint32_t num_addrs;
   int  rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int found;

   pxtm_addr = NULL;
   num_addrs = 0;
   found = FALSE;

   if((qid < XTM_MIN_QOS_QUEUE_IDX) || (qid > XTM_MAX_QOS_QUEUE_IDX))
   {
      tmctl_error("qid is out of range and invalid\n");
      return TMCTL_ERROR;
   }
   memset(&conn_cfg,0x0,sizeof(XTM_CONN_CFG));
   getconn_addrs(&pxtm_addr,&num_addrs);

   if((pxtm_addr != NULL) && (num_addrs > 0))
   {
      rc = devCtl_xtmGetConnCfg(pxtm_addr,&conn_cfg);
      if(rc == CMSRET_SUCCESS)
      {
         if(conn_cfg.ulTransmitQParmsSize >= qid)
         {
            if(conn_cfg.TransmitQParms[qid].ucQosQId != qid)
               found = TRUE;
            else
               found = FALSE;
         }
         else
         {
            found = FALSE;
         }
         if(found)
         {
            qcfg_p->qid           = qid;
            qcfg_p->priority      = conn_cfg.TransmitQParms[qid].ucSubPriority;
            qcfg_p->qsize         = conn_cfg.TransmitQParms[qid].usSize;
            qcfg_p->weight        = conn_cfg.TransmitQParms[qid].ulWeightValue;
            switch(conn_cfg.TransmitQParms[qid].ucWeightAlg)
            {
               case WA_DISABLED:
                  qcfg_p->schedMode = TMCTL_SCHED_SP;
                  break;
               case WA_WFQ:
                  qcfg_p->schedMode = TMCTL_SCHED_WFQ;
                  break;
               default:
                  qcfg_p->schedMode = TMCTL_SCHED_WRR;
                  break;   
            }
            qcfg_p->shaper.shapingRate      = conn_cfg.TransmitQParms[qid].ulShapingRate;
            qcfg_p->shaper.shapingBurstSize = conn_cfg.TransmitQParms[qid].usShapingBurstSize;
            qcfg_p->shaper.minRate          = conn_cfg.TransmitQParms[qid].ulMinBitRate;
            qcfg_p->bestEffort              = FALSE;
            //For 158 minimum reserved buffer size is based on priority
            qcfg_p->minBufs                 = (conn_cfg.TransmitQParms[qid].ucSubPriority * 16);
         }
         else
         {
            ret = TMCTL_NOT_FOUND;
         }
      }
      else
      {
         qcfg_p->qid = INVALID_ID;
         ret = TMCTL_ERROR;
      }
   }

   return ret;

}  /* End of tmctl_xtm_getQueueCfg() */

/* ----------------------------------------------------------------------------
 * This function sets the configuration of a queue.
 *
 * Parameters:
 *    tmParms_p (IN) port tm parameters.
 *    qcfg_p (IN) structure containing the queue config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_xtm_setQueueCfg(tmctl_portTmParms_t* tmParms_p,
                                  tmctl_queueCfg_t* qcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   int  rc;
   BOOL found;
   uint32_t           schedType;
   PXTM_ADDR pxtm_addr;
   XTM_CONN_CFG conn_cfg;
   uint32_t num_addrs;
   uint32_t ulQueueIdx;
   int qid;

   tmctl_debug("Enter: qid=%d priority=%d schedMode=%d qsize=%d "
               "wt=%d minBufs=%d shapingRate=%d burstSize=%d minRate=%d bestEffort=%d",
                qcfg_p->qid, qcfg_p->priority, qcfg_p->schedMode,
                qcfg_p->qsize, qcfg_p->weight, qcfg_p->minBufs, qcfg_p->shaper.shapingRate,
                qcfg_p->shaper.shapingBurstSize, qcfg_p->shaper.minRate,
                qcfg_p->bestEffort ? 1 : 0);

   if(qcfg_p->qid != qcfg_p->priority)
   {
      //Since qid is not maintained in xtmctl and xtm drivers currently, We are
      //enforcing this check that qid and priority needs to be same.
      tmctl_error("Priority of queue must be equal to qid. qid=%d priority=%d",
                  qcfg_p->qid, qcfg_p->priority);
      return TMCTL_ERROR;
   }
   qid = qcfg_p->qid;
   memset(&conn_cfg,0x0,sizeof(XTM_CONN_CFG));
   getconn_addrs(&pxtm_addr,&num_addrs);
   schedType = tmParms_p->cfgFlags & TMCTL_SCHED_TYPE_MASK;
      
   if((pxtm_addr != NULL) && (num_addrs > 0))
   {
      rc = devCtl_xtmGetConnCfg(pxtm_addr,&conn_cfg);
      if(rc == CMSRET_SUCCESS)
      {
         tmctl_debug("TransmitQParmsSize:%d ucQosQId:%d qid:%d\n",conn_cfg.ulTransmitQParmsSize,
                                                                  conn_cfg.TransmitQParms[qid].ucQosQId,
                                                                  qid);
         if(conn_cfg.ulTransmitQParmsSize >= qid)
         {
            if(conn_cfg.TransmitQParms[qid].ucQosQId == qid)
               found = TRUE;
            else
               found = FALSE;
         }
         else
         {
            found = FALSE;
         }
         if(found)
         {
            //Queue found
            tmctl_debug("qid:%d priority:%d Already created",qcfg_p->qid,qcfg_p->priority);
            ret = TMCTL_SUCCESS;
         }
         else
         {
            //Queue not found, need to create it.
            ulQueueIdx = conn_cfg.ulTransmitQParmsSize++;
            conn_cfg.TransmitQParms[qid].usSize        = TMCTL_DEF_XTM_DPU_Q_SZ;
            conn_cfg.TransmitQParms[qid].ucSubPriority = qcfg_p->priority;
            conn_cfg.TransmitQParms[qid].ucQosQId      = qcfg_p->qid;
            conn_cfg.TransmitQParms[qid].ucWeightAlg   = schedType;
            conn_cfg.TransmitQParms[qid].ulWeightValue = 1;
            conn_cfg.TransmitQParms[qid].ulMinBitRate  = 0;
            conn_cfg.TransmitQParms[qid].ulShapingRate = 0;
            conn_cfg.TransmitQParms[qid].usShapingBurstSize = 0;
            conn_cfg.TransmitQParms[qid].ucDropAlg     = WA_DT;
            conn_cfg.TransmitQParms[qid].ucLoMinThresh = 0;
            conn_cfg.TransmitQParms[qid].ucLoMaxThresh = 0;
            conn_cfg.TransmitQParms[qid].ucHiMinThresh = 0;
            conn_cfg.TransmitQParms[qid].ucHiMaxThresh = 0;
            conn_cfg.TransmitQParms[qid].ulPortId      = PORT_PHY0_PATH0;
            conn_cfg.TransmitQParms[qid].ulPtmPriority = PTM_PRI_LOW;
            tmctl_debug("TransmitQParmsSize:%d ucQosQId:%d qid:%d"
                        "usSize:%d, ucSubPriority:%d ucWeightAlg:%d"
                        "ucDropAlg:%d ulPortId:%d ulPtmPriority:%d\n",conn_cfg.ulTransmitQParmsSize,
                                                                     conn_cfg.TransmitQParms[qid].ucQosQId,
                                                                     qid,
                                                                     conn_cfg.TransmitQParms[qid].usSize,
                                                                     conn_cfg.TransmitQParms[qid].ucSubPriority,
                                                                     conn_cfg.TransmitQParms[qid].ucWeightAlg,
                                                                     conn_cfg.TransmitQParms[qid].ucDropAlg,
                                                                     conn_cfg.TransmitQParms[qid].ulPortId,
                                                                     conn_cfg.TransmitQParms[qid].ulPtmPriority);
            rc = devCtl_xtmSetConnCfg(pxtm_addr,&conn_cfg);
            if(rc != CMSRET_SUCCESS)
            {
               tmctl_error("Error! Failed to set the new connection configuration\n");
               ret = TMCTL_ERROR;
            }
         }
      }
      else
      {
         tmctl_error("Error! Failed to get connection configuration");
         ret = TMCTL_ERROR;
      }
   }
   return ret;
}  /* End of tmctl_xtm_setQueueCfg() */

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
tmctl_ret_e tmctl_xtm_getQueueStats(int devType, int rdpaIf, int qid,
                                    tmctl_queueStats_t* queueStats_p)
{
   return TMCTL_UNSUPPORTED;
}

/* ----------------------------------------------------------------------------
 * This function gets the size of a queue.
 *
 * Parameters:
 *    size_p (OUT) uint32_t to return the queue size.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_xtm_getQueueSize(int32_t *psize)
{
   int rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;
   XTM_THRESHOLD_PARMS xtm_threshold;
   memset(&xtm_threshold,0x0,sizeof(XTM_THRESHOLD_PARMS));
   xtm_threshold.sParams.gfastParam = XTM_THRESHOLD_PARM_GET;
   rc = devCtl_xtmManageThreshold(&xtm_threshold);
   if(rc != CMSRET_SUCCESS)
   {
      tmctl_error("Error! Failed to get xtm thresholds\n");
      ret = TMCTL_ERROR;
   }
   else
   {
      *psize = (int32_t)xtm_threshold.gfastThreshold;
   }
   return ret;
}

/* ----------------------------------------------------------------------------
 * This function sets the size of all queues.
 *
 * Parameters:
 *    size_p (OUT) uint32_t to return the queue size.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_xtm_setQueueSize(int32_t size)
{
   int rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;
   XTM_THRESHOLD_PARMS xtm_threshold;
   if((size < TMCTL_MIN_XTM_DPU_Q_SZ) || (size > TMCTL_MAX_XTM_DPU_Q_SZ))
      return TMCTL_ERROR;
   memset(&xtm_threshold,0x0,sizeof(XTM_THRESHOLD_PARMS));
   xtm_threshold.sParams.gfastParam = XTM_THRESHOLD_PARM_SET;
   xtm_threshold.gfastThreshold = (uint32_t)size;
   rc = devCtl_xtmManageThreshold(&xtm_threshold);
   if(rc != CMSRET_SUCCESS)
   {
      tmctl_error("Error! Failed to set xtm thresholds\n");
      ret = TMCTL_ERROR;
   }
   return ret;
}
/* ----------------------------------------------------------------------------
 * This function initializes the XTM TM configuration 
 *
 * Parameters:
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_xtm_TmInit()
{
   return TMCTL_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * This function uninitializes the XTM TM configuration 
 *
 * Parameters:
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_xtm_TmUnInit()
{
   return TMCTL_SUCCESS;
}
