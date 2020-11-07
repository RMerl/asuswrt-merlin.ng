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

#include "tmctl_ethsw.h"

/* ----------------------------------------------------------------------------
 * This function check if an interface is WAN or not.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    isWan (OUT) Is WAN interface or not
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_isWanIntf(const char* ifname, BOOL* isWan)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)
   char wan_ifname[32];
   wan_ifname[0] = '\0';

   tmctl_debug("Enter: ifname=%s", ifname);

   ret = bcm_enet_driver_wan_interface_get(wan_ifname, 32);
   if (ret)
   {
      tmctl_error("bcm_enet_driver_wan_interface_get returns error %d", ret);
      return TMCTL_ERROR;
   }
   tmctl_debug("Enter: wan_ifname=%s", wan_ifname);
   if (!strcmp(ifname, wan_ifname))
   {
      *isWan = TRUE;
   }
   else
   {
      *isWan = FALSE;
   }   
#endif

   return ret;
       
}  /* End of tmctlEthSw_isWanIntf() */

/* ----------------------------------------------------------------------------
 * This function determine new port queue configurations,
 * based on current port queue configurations and new queue config.
 * When new queue config is applied, ethsw cosqsched state will
 * be switched between below 11 states:
 *
 * --------------------------------------------
 *          ethsw cosqsched state list
 * --------------------------------------------
 * St.  Q0  Q1  Q2  Q3  Q4  Q5  Q6  Q7 =>  Mode
 * 01)  SP  SP  SP  SP  SP  SP  SP  SP =>    SP
 * 02) WRR WRR WRR WRR  SP  SP  SP  SP => SPWRR
 * 03) WRR WRR WRR WRR WRR  SP  SP  SP => SPWRR
 * 04) WRR WRR WRR WRR WRR WRR  SP  SP => SPWRR
 * 05) WRR WRR WRR WRR WRR WRR WRR  SP => SPWRR
 * 06) WRR WRR WRR WRR WRR WRR WRR WRR =>   WRR
 * 07) WDR WDR WDR WDR  SP  SP  SP  SP => SPWDR
 * 08) WDR WDR WDR WDR WDR  SP  SP  SP => SPWDR
 * 09) WDR WDR WDR WDR WDR WDR  SP  SP => SPWDR
 * 10) WDR WDR WDR WDR WDR WDR WDR  SP => SPWDR
 * 11) WDR WDR WDR WDR WDR WDR WDR WDR =>   WDR
 *
 * Parameters:
 *    ifname (IN)         Linux interface name.
 *    newQcfg_p (IN)      Structure containing the new queue config parameters.
 *    currPortQcfg_p (IN) The current port queue configurations.
 *    newPortQcfg_p (OUT) The new port queue configurations.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_determineNewPortQcfg(tmctl_portTmParms_t* tmParms_p,
                                            tmctl_queueCfg_t*    newQcfg_p,
                                            tmctl_portQcfg_t*    currPortQcfg_p,
                                            tmctl_portQcfg_t*    newPortQcfg_p)
{
   int i;
   BOOL foundSp = FALSE;
   BOOL foundWrr = FALSE;
   BOOL foundWdrr = FALSE;
   tmctl_queueCfg_t* qcfg_p;
   tmctl_cosq_sched_e newCosqSched = TMCTL_COSQ_SCHED_INVALID;
   int lowestSpQid = tmParms_p->maxQueues;
   int newLowestSpQid = tmParms_p->maxQueues;
   int minSpQid = tmParms_p->maxQueues - tmParms_p->maxSpQueues;

   tmctl_debug("Enter: maxQueues=%d maxSpQueues=%d", tmParms_p->maxQueues, tmParms_p->maxSpQueues);

   memcpy(newPortQcfg_p, currPortQcfg_p, sizeof(tmctl_portQcfg_t));
  
   for (i = 0; i < tmParms_p->maxQueues; i++)
   {
      qcfg_p = &(currPortQcfg_p->qcfg[i]);
      if (qcfg_p->qid >= 0)
      {
         if (qcfg_p->schedMode == TMCTL_SCHED_SP)
         {
            foundSp = TRUE;
            if (qcfg_p->qid < lowestSpQid)
               lowestSpQid = qcfg_p->qid;
         }
         else if(qcfg_p->schedMode == TMCTL_SCHED_WRR)
         {
            foundWrr = TRUE;
         }
         else if(qcfg_p->schedMode == TMCTL_SCHED_WDRR)
         {
            foundWdrr = TRUE;
         }
      }
   }

   tmctl_debug("portQcfg foundSp=%d, foundWrr=%d, foundWdrr=%d", foundSp, foundWrr, foundWdrr);

   if(newQcfg_p->schedMode == TMCTL_SCHED_SP)
   {
      if (newQcfg_p->qid < tmParms_p->maxSpQueues)
      {
         newCosqSched = TMCTL_COSQ_SCHED_SP;
      }
      else
      {
         if(foundWrr)
         {
            newCosqSched = TMCTL_COSQ_SCHED_SPWRR;
         }
         else if(foundWdrr)
         {
            newCosqSched = TMCTL_COSQ_SCHED_SPWDRR;
         }
         else
         {
            newCosqSched = TMCTL_COSQ_SCHED_SP;
         }
      }
      if(newCosqSched == TMCTL_COSQ_SCHED_SP)
      {
         newLowestSpQid = 0;
      }
      else
      {
         if(newQcfg_p->qid < lowestSpQid)
         {
            newLowestSpQid = newQcfg_p->qid;
         }
         else
         {
            newLowestSpQid = lowestSpQid;
         }
      }      
   }
   else if(newQcfg_p->schedMode == TMCTL_SCHED_WRR)
   {
      if (newQcfg_p->qid == tmParms_p->maxQueues)
      {
         newCosqSched = TMCTL_COSQ_SCHED_WRR;
      }
      else
      {
         if(foundSp)
         {
            newCosqSched = TMCTL_COSQ_SCHED_SPWRR;
         }
         else
         {
            newCosqSched = TMCTL_COSQ_SCHED_WRR;
         }
      }
      if(newCosqSched == TMCTL_COSQ_SCHED_WRR)
      {
         newLowestSpQid = tmParms_p->maxQueues;
      }
      else
      {
         if(newQcfg_p->qid >= lowestSpQid)
         {
            newLowestSpQid = newQcfg_p->qid + 1;
         }
         else
         {
            newLowestSpQid = lowestSpQid;
         }
         if(newLowestSpQid < minSpQid)
         {
            newLowestSpQid = minSpQid;
         }
      }
   }
   else if(newQcfg_p->schedMode == TMCTL_SCHED_WDRR)
   {
      if (newQcfg_p->qid == tmParms_p->maxQueues)
      {
         newCosqSched = TMCTL_COSQ_SCHED_WDRR;
      }
      else
      {
         if(foundSp)
         {
            newCosqSched = TMCTL_COSQ_SCHED_SPWDRR;
         }
         else
         {
            newCosqSched = TMCTL_COSQ_SCHED_WDRR;
         }
      }
      if(newCosqSched == TMCTL_COSQ_SCHED_WDRR)
      {
         newLowestSpQid = tmParms_p->maxQueues;
      }
      else
      {
         if(newQcfg_p->qid >= lowestSpQid)
         {
            newLowestSpQid = newQcfg_p->qid + 1;
         }
         else
         {
            newLowestSpQid = lowestSpQid;
         }
         if(newLowestSpQid < minSpQid)
         {
            newLowestSpQid = minSpQid;
         }

      }
   }   
   else
   {
      tmctl_error("Queue schedMode %d is not supported.", newQcfg_p->schedMode);
      return TMCTL_ERROR;
   }

   tmctl_debug("Q schedMode=%d newCosqSched=%d newLowestSpQid=%d",
                newQcfg_p->schedMode, newCosqSched, newLowestSpQid);

   /* Set newPortQcfg_p according to newCosqSched and newLowestSpQid */
   for (i = 0; i < tmParms_p->maxQueues; i++)
   {
      qcfg_p = &(newPortQcfg_p->qcfg[i]);
      if((newCosqSched == TMCTL_COSQ_SCHED_SP) ||
         (qcfg_p->qid >= newLowestSpQid))
      {
         qcfg_p->schedMode = TMCTL_SCHED_SP;
         qcfg_p->priority = i;
         qcfg_p->weight= 0;
      }
      else if((newCosqSched == TMCTL_COSQ_SCHED_SPWRR) ||
              (newCosqSched == TMCTL_COSQ_SCHED_WRR))
      {
         qcfg_p->schedMode = TMCTL_SCHED_WRR;
         qcfg_p->priority = 0;
         if(qcfg_p->qid == newQcfg_p->qid)
         {
            qcfg_p->weight = newQcfg_p->weight;            
         }
         if(qcfg_p->weight == 0)
         {
            qcfg_p->weight = 1;
         }
      }
      else if((newCosqSched == TMCTL_COSQ_SCHED_SPWDRR) ||
              (newCosqSched == TMCTL_COSQ_SCHED_WDRR))
      {
         qcfg_p->schedMode = TMCTL_SCHED_WDRR;
         qcfg_p->priority = 0;
         if(qcfg_p->qid == newQcfg_p->qid)
         {
            qcfg_p->weight = newQcfg_p->weight;            
         }         
         if(qcfg_p->weight == 0)
         {
            qcfg_p->weight = 1;
         }
      }
      else
      {
         tmctl_error("newCosqSched %d is not supported.", newCosqSched);
         return TMCTL_ERROR;
      }
   }   
   return TMCTL_SUCCESS;

} /* End of tmctlEthSw_determineNewPortQcfg() */


/* ----------------------------------------------------------------------------
 * This function gets the Ethernet switch port scheduler.
 *
 * Parameters:
 *    ifname (IN)      Linux interface name.
 *    portQcfg_p (OUT) The port queue configurations.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getPortSched(const char*       ifname,
                                    tmctl_portQcfg_t* portQcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)   

   int i;
   int rc;
   int unit;
   bcm_port_t port;
   int weights[BCM_COS_COUNT] = {0};
   tmctl_queueCfg_t* qcfg_p;
   port_qos_sched_t qs;

   tmctl_debug("Enter: ifname=%s", ifname);

   /* get unit and port number from ifname */
   rc = bcm_enet_map_ifname_to_unit_port(ifname, &unit, &port);
   if (rc)
   {
      tmctl_error("bcm_enet_map_ifname_to_unit_port returns error %d", rc);
      return TMCTL_ERROR;
   }
   
   memset(portQcfg_p, 0, sizeof(tmctl_portQcfg_t));
   
   tmctl_debug("Calling bcm_cosq_sched_get_X: unit=%d port=%d", unit, port);
   rc = bcm_cosq_sched_get_X(unit, port, weights, &qs);
   if (rc)
   {
      tmctl_error("bcm_cosq_sched_get_X returns error %d", rc);
      return TMCTL_ERROR;
   }

   if(qs.sched_mode == BCM_COSQ_STRICT)
   {
      tmctl_debug("Strict Priority Scheduling");
   } else if (qs.sched_mode == BCM_COSQ_WRR)
   {
      tmctl_debug("%s Round Robin Scheduling", qs.wrr_type == 1? "Weighted": "Deficit");
      tmctl_debug("Weights: %d %d %d %d %d %d %d %d",
              weights[7], weights[6], weights[5], weights[4],
              weights[3], weights[2], weights[1], weights[0]);
   } else if (qs.sched_mode == BCM_COSQ_COMBO)
   {
      tmctl_debug("SP+%s combo scheduling",  qs.wrr_type == 1? "WRR": "WDR");
      tmctl_debug("SP from Queue7 to Queue%d and WRR for"
                  " remaining queues", BCM_COS_COUNT - qs.num_spq);
      tmctl_debug("Weights: %d %d %d %d %d %d %d %d",
                   weights[7], weights[6], weights[5], weights[4],
                   weights[3], weights[2], weights[1], weights[0]);
   }

   portQcfg_p->numQueues = BCM_COS_COUNT;   
   for(i = 0; i < BCM_COS_COUNT; i++)
   {
      qcfg_p = &(portQcfg_p->qcfg[i]);
      qcfg_p->qid = i;
      if(qs.sched_mode == BCM_COSQ_STRICT) {
         qcfg_p->schedMode = TMCTL_SCHED_SP;
         qcfg_p->priority = i;         
         qcfg_p->weight = 0;
      } else if (qs.sched_mode == BCM_COSQ_WRR) {
         qcfg_p->schedMode = qs.wrr_type == 1 ? TMCTL_SCHED_WRR : TMCTL_SCHED_WDRR;
         qcfg_p->priority = 0;
         qcfg_p->weight = weights[i];
      } else if (qs.sched_mode == BCM_COSQ_COMBO) {
         if(i < (qs.max_egress_q - qs.num_spq))
         {
            qcfg_p->schedMode = qs.wrr_type == 1 ? TMCTL_SCHED_WRR : TMCTL_SCHED_WDRR;
            qcfg_p->priority = 0;
            qcfg_p->weight = weights[i];
         }
         else
         {
            qcfg_p->schedMode = TMCTL_SCHED_SP;
            qcfg_p->priority = i;
            qcfg_p->weight = 0;
         }
      }      
   }

#endif

   return ret;
       
}  /* End of tmctlEthSw_getPortSched() */


/* ----------------------------------------------------------------------------
 * This function sets the Ethernet switch port scheduler.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    portQcfg_p (IN) The port queue configurations.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_setPortSched(const char*       ifname,
                                    tmctl_portQcfg_t* portQcfg_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)   

   int i;
   int rc;
   int unit;
   bcm_port_t port;
   int schedMode = 0;
   int numSpQ    = 0;
   int wrrType   = 0;
   int weights[BCM_COS_COUNT] = {0};
   tmctl_queueCfg_t* qcfg_p;
   port_qos_sched_t  qs;
   uint32_t portmap = 0;
   
   tmctl_debug("Enter: ifname=%s numQueues=%d", ifname, portQcfg_p->numQueues);

   if (portQcfg_p->numQueues != BCM_COS_COUNT)
   {
      tmctl_error("number of configured queues (%d) is less than port queues (%d)",
                  portQcfg_p->numQueues, BCM_COS_COUNT);
      return TMCTL_ERROR;
   }

   for (i = 0; i < portQcfg_p->numQueues; i++)
   {
      qcfg_p = &(portQcfg_p->qcfg[i]);
      
      if (qcfg_p->qid < 0)
      {
         tmctl_error("queue #%d is not configured. qid=%d", i, qcfg_p->qid);
         ret = TMCTL_ERROR;
         break;
      }
      
      if (qcfg_p->schedMode == TMCTL_SCHED_SP)
      {
         numSpQ++;
      }
      else
      {
         weights[i] = qcfg_p->weight;
         if(qcfg_p->schedMode == TMCTL_SCHED_WRR)
         {
            wrrType = 1; /* QOS_ENUM_WRR_PKT, 1=WRR, 2=WDRR */
         }
         else
         {
            wrrType = 2; /* QOS_ENUM_WDRR_PKT */
         }
      }
   }
   
   if (ret != TMCTL_SUCCESS)
      return ret;
   
   if (numSpQ == 0)
   {
      schedMode = BCM_COSQ_WRR;
   }   
   else if (numSpQ == portQcfg_p->numQueues)
   {
      schedMode = BCM_COSQ_STRICT;
      numSpQ = 0; /* all SP queues */
   }
   else
   {
      schedMode = BCM_COSQ_COMBO;
   }
         
   /* get unit and port number from ifname */
   rc = bcm_enet_map_ifname_to_unit_portmap(ifname, &unit, &portmap);
   if (rc)
   {
      tmctl_error("bcm_enet_map_ifname_to_unit_port returns error %d", rc);
      return TMCTL_ERROR;
   }
   /* TMCTL must apply the same port setting to all the ports in LAG/Trunk group */
   port = -1; /* Start with -1 to increment with */
   while (portmap)
   {
      while( ! (portmap & (1<<(++port))) );
      portmap &= ~(1<<port); /* Reset the port we are execting below -- important */

      /* set port sched */
      memset(&qs, 0, sizeof(port_qos_sched_t));
      qs.sched_mode    = schedMode;
      qs.num_spq       = numSpQ;
      qs.wrr_type      = wrrType; /* 1=WRR, 2=WDRR */
      qs.weights_upper = 0; /* 0=lower, 1=upper */
      
      tmctl_debug("Calling bcm_cosq_sched_set: unit=%d port=%d schedMode=%d numSpQ=%d wt_lower=%d,%d,%d,%d",
                  unit, port, schedMode, numSpQ, weights[0], weights[1], weights[2], weights[3]);
      rc = bcm_cosq_sched_set_X(unit, port, &weights[0], &qs);
      if (rc)
      {
         tmctl_error("bcm_cosq_sched_set for lower weights returns error %d", rc);
         return TMCTL_ERROR;
      }
      
      qs.weights_upper = 1;   /* 0=lower, 1=upper */
      
      tmctl_debug("Calling bcm_cosq_sched_set: unit=%d port=%d schedMode=%d numSpQ=%d wt_upper=%d,%d,%d,%d",
                  unit, port, schedMode, numSpQ, weights[4], weights[5], weights[6], weights[7]);
      rc = bcm_cosq_sched_set_X(unit, port, &weights[0], &qs);
      if (rc)
      {
         tmctl_error("bcm_cosq_sched_set for upper weights returns error %d", rc);
         return TMCTL_ERROR;
      }
   }

#endif

   return ret;
       
}  /* End of tmctlEthSw_setPortSched() */


/* ----------------------------------------------------------------------------
 * This function resets the Ethernet switch port scheduler to Strict Priority.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_resetPortSched(const char* ifname)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)

   int i;
   tmctl_portQcfg_t portQcfg;
   tmctl_queueCfg_t* qcfg_p;

   tmctl_debug("Enter: ifname=%s", ifname);

   memset(&portQcfg, 0, sizeof(tmctl_portQcfg_t));
   portQcfg.numQueues = BCM_COS_COUNT;
   for(i = 0; i < BCM_COS_COUNT; i++)
   {
      qcfg_p = &(portQcfg.qcfg[i]);
      qcfg_p->qid = i;
      qcfg_p->schedMode = TMCTL_SCHED_SP;
      qcfg_p->priority = i;         
      qcfg_p->weight = 0;
   }

   ret = tmctlEthSw_setPortSched(ifname, &portQcfg);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlEthSw_setPortSched ERROR!");
      return ret;
   }

#endif

   return ret;
       
}  /* End of tmctlEthSw_resetPortSched() */


/* ----------------------------------------------------------------------------
 * This function gets the Ethernet switch port shaper configuration.
 *
 * Parameters:
 *    ifname (IN)    Linux interface name.
 *    shaper_p (OUT) The queue shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getPortShaper(const char*     ifname,
                                     tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)   

   int        rc;
   int        unit;
   bcm_port_t port;
   uint32_t   shapingRate = 0;
   uint32_t   burstSize   = 0;

   tmctl_debug("Enter: ifname=%s", ifname);

   /* get unit and port number from ifname */
   rc = bcm_enet_map_ifname_to_unit_port(ifname, &unit, &port);
   if (rc)
   {
      tmctl_error("bcm_enet_map_ifname_to_unit_port returns error %d", rc);
      return TMCTL_ERROR;
   }
   
   memset(shaper_p, 0, sizeof(tmctl_shaper_t));
   
   tmctl_debug("Calling bcm_port_rate_egress_get_X: unit=%d port=%d",
               unit, port);
   rc = bcm_port_rate_egress_get_X(unit, port, &shapingRate, &burstSize, -1, NULL);
   if (rc)
   {
      tmctl_error("bcm_port_rate_egress_get_X returns error %d", rc);
      return TMCTL_ERROR;
   }

   tmctl_debug("Got: shapingRate=%d burstSize=%d", shapingRate, burstSize);

   shaper_p->shapingRate      = shapingRate;    /* in kbps */
   shaper_p->shapingBurstSize /* Bytes */ = (burstSize /* in kbits */ * 1000)/8;
   
#endif

   return ret;
   
}  /* End of tmctlEthSw_getPortShaper() */


/* ----------------------------------------------------------------------------
 * This function sets the Ethernet switch port shaper configuration.
 *
 * Parameters:
 *    ifname (IN)   Linux interface name.
 *    shaper_p (IN) The port shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_setPortShaper(const char*     ifname,
                                     tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)   

   int        rc;
   int        unit;
   bcm_port_t port;
   uint32_t portmap = 0;
   
   tmctl_debug("Enter: ifname=%s minRate=%d shapingRate=%d burstSize=%d",
               ifname, shaper_p->minRate, shaper_p->shapingRate,
               shaper_p->shapingBurstSize);

   /* get unit and port number from ifname */
   rc = bcm_enet_map_ifname_to_unit_portmap(ifname, &unit, &portmap);
   if (rc)
   {
      tmctl_error("bcm_enet_map_ifname_to_unit_portmap returns error %d", rc);
      return TMCTL_ERROR;
   }
   
   /* TMCTL must apply the same port setting to all the ports in LAG/Trunk group */
   port = -1; /* Start with -1 to increment with */
   while (portmap)
   {
      while( ! (portmap & (1<<(++port))) );
      portmap &= ~(1<<port); /* Reset the port we are execting below -- important */

      {
         /* set port shaping */      
         tmctl_debug("Calling bcm_port_rate_egress_set_X: unit=%d port=%d ercLimit=%d ercBurst=%d",
                     unit, port, shaper_p->shapingRate, shaper_p->shapingBurstSize);
         rc = bcm_port_rate_egress_set_X(unit, port, shaper_p->shapingRate /* kbps */, (shaper_p->shapingBurstSize /* Bytes */ * 8)/1000,
                                         -1,    /* qid -1 denotes port shaping */
                                         0);    /* byte mode */
         if (rc)
         {
            tmctl_error("bcm_port_rate_egress_set_X returns error %d", rc);
            return TMCTL_ERROR;
         }
      }
   }
   
#endif

   return ret;
   
}  /* End of tmctlEthSw_setPortShaper() */


/* ----------------------------------------------------------------------------
 * This function gets the Ethernet switch queue shaper configuration.
 *
 * Parameters:
 *    ifname (IN)    Linux interface name.
 *    qid (IN)       Queue ID.
 *    shaper_p (OUT) The queue shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getQueueShaper(const char*     ifname,
                                      int             qid,
                                      tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)   

   int        rc;
   int        unit;
   bcm_port_t port;
   uint32_t   shapingRate = 0;
   uint32_t   burstSize   = 0;

   tmctl_debug("Enter: ifname=%s qid=%d", ifname, qid);

   /* get unit and port number from ifname */
   rc = bcm_enet_map_ifname_to_unit_port(ifname, &unit, &port);
   if (rc)
   {
      tmctl_error("bcm_enet_map_ifname_to_unit_port returns error %d", rc);
      return TMCTL_ERROR;
   }
   
   memset(shaper_p, 0, sizeof(tmctl_shaper_t));
   
   tmctl_debug("Calling bcm_port_rate_egress_get_X: unit=%d port=%d qid=%d",
               unit, port, qid);
               
   rc = bcm_port_rate_egress_get_X(unit, port, &shapingRate, &burstSize, qid, NULL);
   if (rc)
   {
      tmctl_error("bcm_port_rate_egress_get_X returns error %d", rc);
      return TMCTL_ERROR;
   }

   tmctl_debug("Got: shapingRate=%d burstSize=%d", shapingRate, burstSize);

   shaper_p->shapingRate      = shapingRate;    /* in kbps */
   shaper_p->shapingBurstSize /* Bytes */ = (burstSize /* in kbits */ * 1000)/8;
   
#endif

   return ret;
   
}  /* End of tmctlEthSw_getQueueShaper() */


/* ----------------------------------------------------------------------------
 * This function sets the Ethernet switch queue shaper configuration.
 *
 * Parameters:
 *    ifname (IN)   Linux interface name.
 *    qid (IN)      Queue ID.
 *    shaper_p (IN) The queue shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_setQueueShaper(const char*     ifname,
                                      int             qid,
                                      tmctl_shaper_t* shaper_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)   

   int        rc;
   int        unit;
   bcm_port_t port;
   uint32_t portmap = 0;

   tmctl_debug("Enter: ifname=%s qid=%d minRate=%d shapingRate=%d burstSize=%d",
               ifname, qid, shaper_p->minRate, shaper_p->shapingRate,
               shaper_p->shapingBurstSize);

   /* get unit and port number from ifname */
   rc = bcm_enet_map_ifname_to_unit_portmap(ifname, &unit, &portmap);
   if (rc)
   {
      tmctl_error("bcm_enet_map_ifname_to_unit_port returns error %d", rc);
      return TMCTL_ERROR;
   }
   port = 0; 
   while (portmap)
   {
      if ( (portmap & (1<<port)) )
      {
         portmap &= ~(1<<port); /* Reset the port we are execting below -- important */

         tmctl_debug("Calling bcm_port_rate_egress_set_X: unit=%d port=%d ercLimit=%d ercBurst=%d qid=%d",
                     unit, port, shaper_p->shapingRate, shaper_p->shapingBurstSize /* kb */, qid);
         rc = bcm_port_rate_egress_set_X(unit, port, shaper_p->shapingRate/*Kbps*/,(shaper_p->shapingBurstSize /* Bytes */ * 8)/1000,
                                         qid,
                                         0);    /* byte mode */
         if (rc)
         {
            tmctl_error("bcm_port_rate_egress_set_X returns error %d", rc);
            return TMCTL_ERROR;
         }
      }
      port++;
   }
   
#endif

   return ret;
   
}  /* End of tmctlEthSw_setQueueShaper() */


/* ----------------------------------------------------------------------------
 * This function gets port TM parameters (capabilities) from Ethernet Switch
 * driver.
 *
 * Parameters:
 *    ifname (IN)     Linux interface name.
 *    tmParms_p (OUT) Structure to return port TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getPortTmParms(const char*          ifname,
                                      tmctl_portTmParms_t* tmParms_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;
   
#if defined(HAS_SF2)   

   int        rc;
   int        unit;
   bcm_port_t port;
   int               weights[BCM_COS_COUNT] = {0};
   port_qos_sched_t  qs;
   
   tmctl_debug("Enter: ifname=%s", ifname);

   /* get unit and port number from ifname */
   rc = bcm_enet_map_ifname_to_unit_port(ifname, &unit, &port);
   if (rc)
   {
      tmctl_error("bcm_enet_map_ifname_to_unit_port returns error %d", rc);
      return TMCTL_ERROR;
   }

   memset(&qs, 0, sizeof(port_qos_sched_t));   
   
   rc = bcm_cosq_sched_get_X(unit, port, &weights[0], &qs);
   if (rc)
   {
      tmctl_error("bcm_cosq_sched_get_X returns error %d", rc);
      return TMCTL_ERROR;
   }

   tmParms_p->schedCaps = 0;

   if (qs.port_qos_caps & QOS_SCHED_SP_CAP)
      tmParms_p->schedCaps |= TMCTL_SP_CAPABLE;
   if (qs.port_qos_caps & QOS_SCHED_WRR_CAP)
      tmParms_p->schedCaps |= TMCTL_WRR_CAPABLE;
   if (qs.port_qos_caps & QOS_SCHED_WDR_CAP)
      tmParms_p->schedCaps |= TMCTL_WDRR_CAPABLE;
   if (qs.port_qos_caps & QOS_SCHED_COMBO)
      tmParms_p->schedCaps |= (TMCTL_SP_WRR_CAPABLE | TMCTL_SP_WDRR_CAPABLE);

   tmParms_p->maxQueues   = qs.max_egress_q;
   tmParms_p->maxSpQueues = qs.max_egress_spq;
   
   if (qs.port_qos_caps & QOS_QUEUE_SHAPER_CAP)
      tmParms_p->queueShaper = TRUE;
   else
      tmParms_p->queueShaper = FALSE;
      
   if (qs.port_qos_caps & QOS_PORT_SHAPER_CAP)
      tmParms_p->portShaper = TRUE;
   else
      tmParms_p->portShaper = FALSE;

   tmParms_p->cfgFlags = 0;
   
   tmctl_debug("schedType=0x%x maxQueues=%d maxSpQueues=%d queueShaper=%d portShaper=%d",
               tmParms_p->schedCaps, tmParms_p->maxQueues, tmParms_p->maxSpQueues,
               tmParms_p->queueShaper, tmParms_p->portShaper);
   
#endif

   return ret;
      
}  /* End of tmctlEthSw_getPortTmParms() */

