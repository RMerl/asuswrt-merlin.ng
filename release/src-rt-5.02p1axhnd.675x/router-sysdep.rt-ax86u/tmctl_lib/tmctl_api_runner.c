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
#include "board.h"
#include "bcm_boarddriverctl.h"
#include "tmctl_api.h"
#include "tmctl_api_trace.h"
#include "tmctl_ethsw.h"
#if defined(BCM_PON) || defined(BCM_DSL_XRDP)
#include "user_api.h"
#include "tmctl_bdmf_rdpa.h"
#endif

#if defined(SUPPORT_RDPA)
#include "tmctl_rdpa.h"
#endif
#include "tmctl_api_runner.h"

#if defined(SUPPORT_RDPA)
/* ----------------------------------------------------------------------------
 * This function converts device type and interface from TMCtl to RDPA format.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port/TCONT/LLID identifier.
 *    rdpaDev_p (OUT) rdpa device type.
 *    ifId_p (OUT) interface id.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */

static inline tmctl_ret_e tmctl_ifConvert(tmctl_devType_e devType,
                                          tmctl_if_t*     if_p,
                                          int*            rdpaDev_p,
                                          int*            ifId_p)
{
   tmctl_ret_e ret = TMCTL_SUCCESS;

   switch (devType)
   {
      case TMCTL_DEV_ETH:
         *rdpaDev_p = RDPA_IOCTL_DEV_PORT;
         ret = tmctlRdpa_getRdpaIfByIfname(if_p->ethIf.ifname, (rdpa_if*)ifId_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("Cannot find rdpaIf by ifname %s", if_p->ethIf.ifname);
            return ret;
         }
         break;

      case TMCTL_DEV_EPON:
         *rdpaDev_p = RDPA_IOCTL_DEV_LLID;
         *ifId_p = if_p->eponIf.llid;
         break;

      case TMCTL_DEV_GPON:
         *rdpaDev_p = RDPA_IOCTL_DEV_TCONT;
         *ifId_p = if_p->gponIf.tcontid;
         break;

      case TMCTL_DEV_SVCQ:
         *rdpaDev_p = RDPA_IOCTL_DEV_NONE;
         *ifId_p = 0;
         break;

      case TMCTL_DEV_XTM:
         *rdpaDev_p = RDPA_IOCTL_DEV_XTM;
         *ifId_p = rdpa_wan_type_to_if(rdpa_wan_dsl);
         break;

      case TMCTL_DEV_NONE:
         *rdpaDev_p = RDPA_IOCTL_DEV_NONE;
         *ifId_p = 0;
         break;

      default:
         tmctl_error("tmctl_ifConvert: invalid device type %d", devType);
         return TMCTL_ERROR;
   }

   return ret;
}

#else

static inline tmctl_ret_e tmctl_ifConvert(tmctl_devType_e devType,
                                          tmctl_if_t*     if_p,
                                          int*            rdpaDev_p,
                                          int*            ifId_p)
{
	return -1;
}
#endif

#if defined(BCM_PON) || defined(BCM_DSL_XRDP)
static int is_supported_wan_mode(void)
{
   rdpa_epon_mode eponMode = rdpa_epon_none;
   int ret;

   ret = rdpaCtl_get_epon_mode(&eponMode);
   if (ret)
   {
      /* if get epon mode fail, return support */
      return 1;
   }

#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
   if (eponMode == rdpa_epon_dpoe)
   {
      bdmf_object_handle system = 0;
      rdpa_system_cfg_t cfg = {0};

      rdpa_system_get(&system);
      rdpa_system_cfg_get(system, &cfg);
      bdmf_put(system);
      return !cfg.car_mode;
   }
#endif

   return 1;
}

static int is_supported_rdpa_api(tmctl_devType_e devType, tmctl_if_t *if_p, int ifId)
{
   int is_supported = 0;

   switch (devType)
   {
      case TMCTL_DEV_ETH:
      {
         if (is_lan(devType, ifId))
         {
            is_supported = 1;
            goto exit;
         }
    
         is_supported = is_supported_wan_mode();
         break;
      }
      case TMCTL_DEV_EPON:
      {
         is_supported = is_supported_wan_mode();
         break;
      }
      case TMCTL_DEV_XTM:
      case TMCTL_DEV_SVCQ:
        goto exit;

      default:
        is_supported = 1;
   }

exit:
    return is_supported;
}
        
#endif /* BCM_PON */
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

   tmctl_debug("Enter: devType=%d cfgFlags=0x%04x numQueues=%d", devType, cfgFlags, numQueues);

#if defined(BCM_DSL_RDP)

   if ((devType == TMCTL_DEV_ETH) || (devType == TMCTL_DEV_GPON))
   {
      uint32_t schedType;
      int      rdpaDev;
      rdpa_if  rdpaIf;
      tmctl_portTmParms_t tmParms;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

      ret = tmctl_getPortTmParms(devType, if_p, &tmParms);
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

      /* check the configured scheduler type */
      schedType = cfgFlags & TMCTL_SCHED_TYPE_MASK;

      if ((schedType == TMCTL_SCHED_TYPE_SP     && !(tmParms.schedCaps & TMCTL_SP_CAPABLE)) ||
          (schedType == TMCTL_SCHED_TYPE_WRR    && !(tmParms.schedCaps & TMCTL_WRR_CAPABLE)) ||
          (schedType == TMCTL_SCHED_TYPE_SP_WRR && !(tmParms.schedCaps & TMCTL_SP_WRR_CAPABLE)))
      {
         tmctl_error("Configured scheduler type %d is not supported", schedType);
         return TMCTL_ERROR;
      }

      if (devType == TMCTL_DEV_ETH)
      {
      /* In 631x8, LAN port and queue shapings are performed by Ethernet switch.
      * So we don't init rdpa tm for port and queue shaper.
      */
        if (is_lan(devType, rdpaIf))
        {
           tmParms.portShaper  = FALSE;
           tmParms.queueShaper = FALSE;
        }

        ret = tmctlRdpa_TmInit(rdpaDev, rdpaIf, &tmParms, cfgFlags);
        if (ret == TMCTL_ERROR)
        {
           tmctl_error("tmctlRdpa_TmInit ERROR! ret=%d", ret);
        }
      }
      else /* if (devType == TMCTL_DEV_GPON) */
      {
         tmParms.portShaper  = FALSE;
         ret = tmctlRdpa_TmInit(RDPA_IOCTL_DEV_TCONT, rdpaIf, &tmParms, cfgFlags);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlRdpa_TmInit ERROR! ret=%d", ret);
         }
      }
      return ret;
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP) 

   tmctl_portTmParms_t tmParms;
   uint32_t            schedType;
   int                 rdpaDev;
   int             ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       ret = tmctl_RdpaTmInit(rdpaDev, devType, ifId, cfgFlags, numQueues);
       if (is_switch_intf(devType, ifId)) 
       {
           /* No TM Init for Embedded Ethernet Switch;
              If queue scheduling is not SP - we may have to
              call some APIs - TODO */
       }
       return ret;
   }
   else if (ret)
   {
      tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                  devType, ifId, ret);
      return TMCTL_ERROR;
   }

   ret = tmctl_getPortTmParms(devType, if_p, &tmParms);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctl_getPortTmParms returns error, devType=%d, if=%d",
                  devType, ifId);
      return ret;
   }

   tmctl_debug("tmctl_getPortTmParms(): devType=%d, if=%d, "
               "schedCaps=%d, maxQueues=%d, maxSpQueues=%d,"
               "portShaper=%d, queueShaper=%d, cfgFlags=%d",
               devType, *if_p, tmParms.schedCaps,
               tmParms.maxQueues, tmParms.maxSpQueues,
               tmParms.portShaper, tmParms.queueShaper, tmParms.cfgFlags);

   if (tmParms.maxQueues == 0 || tmParms.maxQueues > MAX_TMCTL_QUEUES_BASELINE)
   {
      tmctl_error("Invalid maxQueues=%d devType=%d if=%d",
                  tmParms.maxQueues, devType, ifId);
      return TMCTL_ERROR;
   }

   if (tmParms.maxSpQueues > tmParms.maxQueues)
   {
      tmctl_error("Invalid maxSpQueues=%d maxQueues=%d devType=%d",
                  tmParms.maxSpQueues, tmParms.maxQueues, devType);
      return TMCTL_ERROR;
   }

   /* check the configured scheduler type */
   schedType = cfgFlags & TMCTL_SCHED_TYPE_MASK;

   if ((schedType == TMCTL_SCHED_TYPE_SP     && !(tmParms.schedCaps & TMCTL_SP_CAPABLE)) ||
       (schedType == TMCTL_SCHED_TYPE_WRR    && !(tmParms.schedCaps & TMCTL_WRR_CAPABLE)) ||
       (schedType == TMCTL_SCHED_TYPE_SP_WRR && !(tmParms.schedCaps & TMCTL_SP_WRR_CAPABLE)))
   {
      tmctl_error("Configured scheduler type %d is not supported", schedType);
      return TMCTL_ERROR;
   }

   if ((devType == TMCTL_DEV_GPON) || (devType == TMCTL_DEV_EPON) ||
     (devType == TMCTL_DEV_SVCQ))
   {
      tmParms.portShaper = FALSE;
   }
   else if (devType == TMCTL_DEV_ETH)
   {
      if (rdpa_if_is_wan(ifId))
      {
         /* Keep the queueShaper setting. */
         tmParms.portShaper  = FALSE;
      }
      else
      {
         tmParms.portShaper  = FALSE;
         tmParms.queueShaper = FALSE;
      }
   }
   else if(devType == TMCTL_DEV_XTM)
   {
      tmParms.portShaper = FALSE;
   }
#if defined(BUILD_DSL)
   if(devType == TMCTL_DEV_XTM)
   {
      ret = tmctl_xtm_TmInit();
   }
   else
#endif   //BUILD_DSL
   {
      ret = tmctlRdpa_TmInit(rdpaDev, ifId, &tmParms, cfgFlags);
   }
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_TmInit ERROR! rdpaDev=%d, ifId=%d, ret=%d",
                  rdpaDev, ifId, ret);
   }
#endif

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

#if defined(BCM_DSL_RDP)

   if ((devType == TMCTL_DEV_ETH) || (devType == TMCTL_DEV_GPON))
   {
      /* TMCTL must apply the same port setting to all the ports in LAG/Trunk group */
      rdpa_if  rdpaIf;
      int      rdpaDev;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

     if (is_lan(devType, rdpaIf))
     {
        tmctl_debug("Don't uninit TM on LAN port");
     }
     else
     {
        /* EthWAN */
        ret = tmctlRdpa_TmUninit(rdpaDev, rdpaIf);
        if (ret == TMCTL_ERROR)
        {
           tmctl_error("tmctlRdpa_TmUninit ERROR! ret=%d", ret);
        }
     }
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)

   int rdpaDev;
   int ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       ret = tmctl_RdpaTmUninit(rdpaDev, devType, ifId);
       /* TODO - revert back the Ethernet switch queue/port shapers as well */
       return ret;
   }
   else if (ret)
   {
      tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                  devType, ifId, ret);
      return TMCTL_ERROR;
   }
#if defined(BUILD_DSL)
   if(devType == TMCTL_DEV_XTM)
   {
      ret = tmctl_xtm_TmUnInit();
   }
   else
#endif   //BUILD_DSL
   {
      ret = tmctlRdpa_TmUninit(rdpaDev, ifId);
   }
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_TmUninit ERROR! rdpaDev=%d, ifId=%d, ret=%d",
                  rdpaDev, ifId, ret);
   }
#endif

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

#if defined(BCM_DSL_RDP)
   if ((devType == TMCTL_DEV_ETH) || (devType == TMCTL_DEV_GPON) || (devType == TMCTL_DEV_XTM))
   {
      rdpa_if rdpaIf;
      int rdpaDev;
      int tmId;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

      ret = tmctlRdpa_getQueueCfg(rdpaDev, rdpaIf, queueId, &tmId, qcfg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlRdpa_getQueueCfg ERROR! ret=%d", ret);
      }

      /* In 631x8, LAN queue shaping is performed by Ethernet switch.
       *           EthWAN queue shaping is performed by Runner Overall RL TM
       */
      if (is_lan(devType, rdpaIf))
      {
         /* LAN */
         ret = tmctlEthSw_getQueueShaper(if_p->ethIf.ifname,
                                         qcfg_p->qid,
                                         &(qcfg_p->shaper));
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlEthSw_getQueueShaper ERROR!");
            return ret;
         }
      }
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)

   int rdpaDev;
   int ifId;
   int tmId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       ret = tmctl_RdpaQueueCfgGet(rdpaDev, devType, ifId, queueId, qcfg_p);
       if (is_switch_intf(devType, ifId))
       {
           /* Retrieve Queue Shaper info from Ethernet Switch
            */
           ret = tmctlEthSw_getQueueShaper(if_p->ethIf.ifname,
                                           qcfg_p->qid,
                                           &(qcfg_p->shaper));
       }
       return ret;
   }
   else if (ret)
   {
      tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                  devType, ifId, ret);
      return TMCTL_ERROR;
   }
#if defined(BUILD_DSL)
   if(devType == TMCTL_DEV_XTM)
   {
      ret = tmctl_xtm_getQueueCfg(queueId, &tmId, qcfg_p);
   }
   else
#endif   //BUILD_DSL
   {
      ret = tmctlRdpa_getQueueCfg(rdpaDev, ifId,
                                  queueId, &tmId, qcfg_p);
   }
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_getQueueCfg ERROR! ret=%d", ret);
   }

#endif
   return ret;

}  /* End of tmctl_getQueueCfg() */

/* ----------------------------------------------------------------------------
 * This function updates the switch port scheduer based on the current queue
 * configuration. Port scheduler is updated only when all the queues are
 * configured in runner.
 *
 * Parameters:
 *    rdpaDev (IN) rdpa device type.
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *    ifId (IN) rdpa_if.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e ethSw_update_port_sched(rdpa_drv_ioctl_dev_type rdpaDev,
                                    tmctl_devType_e         devType,
                                    tmctl_if_t              *if_p,
                                    rdpa_if                 ifId)
{
#if defined(BCM_DSL_XRDP)
   /* Must only be called for Ethernet Switch LAN Port*/
   tmctl_portQcfg_t portQcfg;
   tmctl_portTmParms_t tmParms;
   tmctl_queueCfg_t* qcfg_p;
   int qidx;
   tmctl_ret_e ret = TMCTL_ERROR;

   if (!is_switch_intf(devType, ifId)) 
   {
       tmctl_error("not a switch lan port");
       return ret;
   }
   /* get the port tm parameters */
   ret = tmctl_getPortTmParms(devType, if_p, &tmParms);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctl_getPortTmParms returns error");
      return ret;
   }

   portQcfg.numQueues = 0;

   qcfg_p = &(portQcfg.qcfg[0]);
   for (qidx = 0; qidx < tmParms.maxQueues; qidx++, qcfg_p++)
   {
      ret = tmctl_RdpaQueueCfgGet(rdpaDev, devType, ifId, qidx, qcfg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctl_RdpaQueueCfgGet ERROR for qidx = %d!", qidx);
         return ret;
      }
      else if (ret == TMCTL_SUCCESS)
      {
         portQcfg.numQueues++;
      }
   }

   if (portQcfg.numQueues == tmParms.maxQueues)
   {
      ret = tmctlEthSw_setPortSched(if_p->ethIf.ifname, &portQcfg);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlEthSw_setPortSched ERROR!");
         return ret;
      }
   }

   /* Force return success since the "ret" variable may not contain success.
      It can be TMCTL_NOT_FOUND when calling tmctl_RdpaQueueCfgGet earlier
      We bail out for error cases before reaching this point anyway */
   return TMCTL_SUCCESS;
#else
   return TMCTL_ERROR;
#endif
}

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

   tmctl_debug("Enter: devType=%d qid=%d priority=%d qsize=%d schedMode=%d wt=%d minBufs=%d minRate=%d kbps=%d mbs=%d",
               devType, qcfg_p->qid, qcfg_p->priority, qcfg_p->qsize, qcfg_p->schedMode,
               qcfg_p->weight, qcfg_p->minBufs, qcfg_p->shaper.minRate,
               qcfg_p->shaper.shapingRate, qcfg_p->shaper.shapingBurstSize);

#if defined(BCM_DSL_RDP)
   if (devType == TMCTL_DEV_ETH)
   {
      /* TMCTL must apply the same port setting to all the ports in LAG/Trunk group */
      rdpa_if             rdpaIf;
      tmctl_portTmParms_t tmParms;
      uint32_t            schedType;
      BOOL                dsQueueShaper = FALSE ;
      int                 rdpaDev;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

      if ((devType == TMCTL_DEV_ETH) && (qcfg_p->schedMode == TMCTL_SCHED_SP) && (qcfg_p->qid != qcfg_p->priority))
      {
         tmctl_error("Priority of SP queue must be equal to qid. qid=%d priority=%d",
                     qcfg_p->qid, qcfg_p->priority);
         return TMCTL_ERROR;
      }

      if ((qcfg_p->schedMode == TMCTL_SCHED_WRR || qcfg_p->schedMode == TMCTL_SCHED_WFQ) &&
          (qcfg_p->priority != 0))
      {
         tmctl_error("Priority of WRR/WFQ queue must be 0. qid=%d priority=%d",
                     qcfg_p->qid, qcfg_p->priority);
         return TMCTL_ERROR;
      }

      if ((qcfg_p->schedMode == TMCTL_SCHED_WRR || qcfg_p->schedMode == TMCTL_SCHED_WFQ) &&
           (qcfg_p->weight == 0))
      {
         tmctl_error("Weight of WRR/WFQ queue must be non-zero. qid=%d",
                     qcfg_p->qid);
         return TMCTL_ERROR;
      }

      /* get the port tm parameters */
      ret = tmctl_getPortTmParms(devType, if_p, &tmParms);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctl_getPortTmParms returns error");
         return ret;
      }

      schedType = tmParms.cfgFlags & TMCTL_SCHED_TYPE_MASK;

      if ((qcfg_p->schedMode == TMCTL_SCHED_SP &&
           schedType != TMCTL_SCHED_TYPE_SP  && schedType != TMCTL_SCHED_TYPE_SP_WRR) ||
          (qcfg_p->schedMode == TMCTL_SCHED_WRR &&
           schedType != TMCTL_SCHED_TYPE_WRR && schedType != TMCTL_SCHED_TYPE_SP_WRR))
      {
         tmctl_error("Queue sched mode %d is not supported", qcfg_p->schedMode);
         return TMCTL_ERROR;
      }

     /* In 631x8, LAN queue shapings are performed by the switch. */
     if (is_lan(devType, rdpaIf))
     {
        dsQueueShaper       = tmParms.queueShaper;
        tmParms.queueShaper = FALSE;
     }

     ret = tmctlRdpa_setQueueCfg(rdpaDev, rdpaIf, &tmParms, qcfg_p);
     if (ret == TMCTL_ERROR)
     {
        tmctl_error("tmctlRdpa_setQueueCfg ERROR! ret=%d", ret);
        return ret;
     }

     /* Commit port queue configuration to Ethernet Switch as needed.
      * In 631x8, only LAN interface is facilitated with an external switch.
      */
     if (is_lan(devType, rdpaIf))
     {
        /* LAN */
        tmctl_portQcfg_t portQcfg;

        /* set Eth Switch queue shaper */
        if (dsQueueShaper)
        {
           ret = tmctlEthSw_setQueueShaper(if_p->ethIf.ifname,
                                           qcfg_p->qid,
                                           &(qcfg_p->shaper));
           if (ret == TMCTL_ERROR)
           {
              tmctl_error("tmctlEthSw_setQueueShaper ERROR!");
              return ret;
           }
        }

        /* set Eth Switch port scheduler only when all queues had been configured */
        ret = tmctlRdpa_getDevQueueCfg(RDPA_IOCTL_DEV_PORT, rdpaIf,
                                       tmParms.maxQueues, &portQcfg);
        if (ret == TMCTL_ERROR)
        {
           tmctl_error("tmctlRdpa_getPortQueueCfg ERROR! ret=%d", ret);
           return ret;
        }

        if (portQcfg.numQueues == tmParms.maxQueues)
        {
           ret = tmctlEthSw_setPortSched(if_p->ethIf.ifname, &portQcfg);
           if (ret == TMCTL_ERROR)
           {
              tmctl_error("tmctlEthSw_setPortSched ERROR!");
              return ret;
           }
        }
     }
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)

   tmctl_portTmParms_t tmParms;
   uint32_t            schedType;
   int                 rdpaDev;
   int                 ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
       tmctl_debug("Exit: %d", TMCTL_ERROR);
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
      if (is_switch_intf(devType, ifId))
      {
          /* Set Queue Shaper info in Ethernet Switch */
          ret = tmctlEthSw_setQueueShaper(if_p->ethIf.ifname,
                                          qcfg_p->qid,
                                          &(qcfg_p->shaper));
          /* Runner does not perform shaping towards switch */
          memset(&(qcfg_p->shaper), 0, sizeof(qcfg_p->shaper));
      }
      /* Intentionally RDPA is called after switch, to wipe-out shaper config */
      ret = tmctl_RdpaTmQueueSet(rdpaDev, devType, ifId, qcfg_p);
      /* Update port scheduler if needed */
      if (is_switch_intf(devType, ifId))
      {
         ret = ethSw_update_port_sched(rdpaDev, devType, if_p, ifId);
      }
      tmctl_debug("Exit: %d", ret);
      return ret;
   }
   else if (ret)
   {
      tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                  devType, ifId, ret);
      return TMCTL_ERROR;
   }

   if (qcfg_p->qid >= MAX_TMCTL_QUEUES_BASELINE)
   {
      tmctl_error("Queue id must < %d, qid=%d",
                  MAX_TMCTL_QUEUES_BASELINE, qcfg_p->qid);
      return TMCTL_ERROR;
   }

   if (qcfg_p->priority > TMCTL_PRIO_MAX)
   {
      tmctl_error("Priority of SP queue must < %d, qid=%d priority=%d",
                  TMCTL_PRIO_MAX, qcfg_p->qid, qcfg_p->priority);
      return TMCTL_ERROR;
   }

   if ((qcfg_p->schedMode == TMCTL_SCHED_WRR || qcfg_p->schedMode == TMCTL_SCHED_WFQ) &&
        (qcfg_p->priority != 0))
   {
      tmctl_error("Priority of WRR/WFQ queue must be 0. qid=%d priority=%d",
                  qcfg_p->qid, qcfg_p->priority);
      return TMCTL_ERROR;
   }

   if ((qcfg_p->schedMode == TMCTL_SCHED_WRR || qcfg_p->schedMode == TMCTL_SCHED_WFQ) &&
        (qcfg_p->weight == 0))
   {
      tmctl_error("Weight of WRR/WFQ queue must be non-zero. qid=%d",
                  qcfg_p->qid);
      return TMCTL_ERROR;
   }

   /* get the port tm parameters */
   ret = tmctl_getPortTmParms(devType, if_p, &tmParms);
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
#if defined(BUILD_DSL)
   if(devType == TMCTL_DEV_XTM)
   {
      ret = tmctl_xtm_setQueueCfg(&tmParms, qcfg_p);
   }
   else
#endif   //BUILD_DSL
   {
      ret = tmctlRdpa_setQueueCfg(rdpaDev, ifId, &tmParms, qcfg_p);
   }
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_setQueueCfg ERROR! ret=%d", ret);
   }
#endif

   tmctl_debug("Exit: %d", ret);
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

#if defined(BCM_DSL_RDP)

   if ((devType == TMCTL_DEV_ETH) || (devType == TMCTL_DEV_GPON))
   {
      int      rdpaDev;
      rdpa_if  rdpaIf;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

     ret = tmctlRdpa_delQueueCfg(rdpaDev, rdpaIf, queueId);
     if (ret == TMCTL_ERROR)
     {
        tmctl_error("tmctlRdpa_delQueueCfg ERROR! ret=%d", ret);
     }

     if (is_lan(devType, rdpaIf))
     {
        /* LAN */
        tmctl_shaper_t shaper;

        /* Disable Eth Switch queue shaper */
        memset(&shaper, 0, sizeof(tmctl_shaper_t));

        ret = tmctlEthSw_setQueueShaper(if_p->ethIf.ifname, queueId, &shaper);
        if (ret == TMCTL_ERROR)
        {
           tmctl_error("tmctlEthSw_setQueueShaper ERROR!");
           return ret;
        }
     }
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)
   int      rdpaDev;
   int      ifId; 

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       ret = tmctl_RdpaTmQueueDel(rdpaDev, devType, ifId, queueId);
       if (is_switch_intf(devType, ifId))
       {
           tmctl_shaper_t shaper;
           /* Disable Eth Switch queue shaper */
           memset(&shaper, 0, sizeof(tmctl_shaper_t));

           ret = tmctlEthSw_setQueueShaper(if_p->ethIf.ifname, queueId, &shaper);
       }
       return ret;
   }
   else if (ret)
   {
      tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                  devType, ifId, ret);
      return TMCTL_ERROR;
   }
#if defined(BUILD_DSL)
   if(devType == TMCTL_DEV_XTM)
   {
      ret = tmctl_xtm_remQueueCfg(queueId);
   }
   else
#endif   //BUILD_DSL
   {
      ret = tmctlRdpa_remQueueCfg(rdpaDev, ifId, queueId);
   }
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_delQueueCfg ERROR! ret=%d", ret);
   }
#endif
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

#if defined(BCM_DSL_RDP)

   if (devType == TMCTL_DEV_ETH)
   {
      int      rdpaDev;
      rdpa_if  rdpaIf;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf by ifname %s", if_p->ethIf.ifname);
         return ret;
      }

      /* In 631x8, LAN port shaping is performed by Ethernet switch.
       *           EthWAN port shaping is performed by Runner Overall RL TM
       */
      if (is_lan(devType, rdpaIf))
      {
         /* LAN */
         ret = tmctlEthSw_getPortShaper(if_p->ethIf.ifname, shaper_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlEthSw_getQueueShaper ERROR!");
         }
      }
      else
      {
         /* WAN */
#if defined(BCM_DSL_RDP)
         ret = tmctlRdpa_getPortShaper(rdpaDev, rdpaIf, shaper_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlRdpa_getPortShaper ERROR! ret=%d", ret);
         }
#else /* #if defined(BCM_DSL_XRDP) */
         ret = tmctlRdpa_getTmRlCfg(rdpaDev, rdpaIf, shaper_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlRdpa_getTmRlCfg ERROR! ret=%d", ret);
         }
#endif
      }
   }
   else if (devType == TMCTL_DEV_GPON)
   {
      tmctl_error("GPON device is not supported");
      ret = TMCTL_ERROR;
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)
   int                 rdpaDev;
   int                 ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       if (is_switch_intf(devType, ifId))
       {
           ret = tmctlEthSw_getPortShaper(if_p->ethIf.ifname, shaper_p);
       }
       else
       {
           ret = tmctl_RdpaGetPortShaper(rdpaDev, devType, ifId, shaper_p);
       }
       return ret;
   }
   else if (ret)
   {
       tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
               devType, ifId, ret);
       return TMCTL_ERROR;
   }

   switch (devType)
   {
      case TMCTL_DEV_ETH:
      case TMCTL_DEV_SVCQ:
          ret = tmctlRdpa_getTmRlCfg(rdpaDev, ifId, shaper_p);
          if (ret == TMCTL_ERROR)
          {
              tmctl_error("tmctlRdpa_getTmRlCfg ERROR! ret=%d", ret);
              return ret;
          }
          break;

      case TMCTL_DEV_EPON:
          tmctl_error("EPON device is not supported");
          ret = TMCTL_ERROR;
          break;

      case TMCTL_DEV_GPON:
          tmctl_error("GPON device is not supported");
          ret = TMCTL_ERROR;
          break;

      default:
         tmctl_error("Invalid device type %d", devType);
         ret = TMCTL_ERROR;
   }

#endif
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

#if defined(BCM_DSL_RDP)
   if (devType == TMCTL_DEV_ETH)
   {
      int      rdpaDev;
      int      rdpaIf;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf by ifname %s", if_p->ethIf.ifname);
         return ret;
      }

      /* In 631x8, LAN port shaping is performed by Ethernet switch.
       *           EthWAN port shaping is performed by Runner Overall RL TM
       */
      if (is_lan(devType, rdpaIf))
      {
         /* LAN */
         ret = tmctlEthSw_setPortShaper(if_p->ethIf.ifname, shaper_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlEthSw_setPortShaper ERROR! ret=%d", ret);
         }
      }
      else
      {
         /* WAN */
         ret = tmctlRdpa_setPortShaper(RDPA_IOCTL_DEV_PORT, rdpaIf, shaper_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlRdpa_setPortShaper ERROR! ret=%d", ret);
         }
      }
   }
   else if (devType == TMCTL_DEV_GPON)
   {
      tmctl_error("GPON device is not supported");
      ret = TMCTL_ERROR;
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)
   int                 rdpaDev;
   int                 ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
       return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       if (is_switch_intf(devType, ifId))
       {
           ret = tmctlEthSw_setPortShaper(if_p->ethIf.ifname, shaper_p);
       }
       else
       {
           ret = tmctl_RdpaSetPortShaper(rdpaDev, devType, ifId, shaper_p);
       }
       return ret;
   }
   else if (ret)
   {
       tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
               devType, ifId, ret);
       return TMCTL_ERROR;
   }

   switch (devType)
   {
      case TMCTL_DEV_ETH:
      case TMCTL_DEV_SVCQ:
      case TMCTL_DEV_EPON:
          ret = tmctlRdpa_setTmRlCfg(rdpaDev, ifId, shaper_p);
          if (ret == TMCTL_ERROR)
          {
              tmctl_error("tmctlRdpa_setTmRlCfg ERROR! ret=%d", ret);
              return ret;
          }
          break;

      case TMCTL_DEV_GPON:
          tmctl_error("GPON device is not supported yet");
          ret = TMCTL_ERROR;
          break;

      default:
         tmctl_error("Invalid device type %d", devType);
         ret = TMCTL_ERROR;
   }


#endif
   return ret;

}  /* End of tmctl_setPortShaper() */


/* ----------------------------------------------------------------------------
 * This function gets the overall shaper configuration.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p     (OUT) The portMap which linked to the overall shaper 
 *    shaper_p (OUT) The shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_getOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p,
                                     tmctl_shaper_t* shaper_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;
    tmctl_debug(" Enter: devType=%d \n", devType);
#if defined(BCM_PON)        
    if (devType == TMCTL_DEV_GPON)
    {        
        ret = tmctl_RdpaGetOverAllShaper(shaper_p, &(if_p->portId.portMap));
        if (ret == TMCTL_ERROR)
        {
            tmctl_error("tmctl_RdpaGetOverAllShaper ERROR! ret=%d", ret);
            return ret;
        }
    }
#endif    
    return ret;
}  /* End of tmctl_getOverAllShaper_plat() */


/* ----------------------------------------------------------------------------
 * This function configures a overall shaper for shaping rate, shaping burst
 * size.
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    shaper_p (IN) The shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_setOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_shaper_t* shaper_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;
    
    tmctl_debug(" Enter: devType=%d Rate=%d BurstSize=%d", devType,
               shaper_p->shapingRate, shaper_p->shapingBurstSize);
#if defined(BCM_PON)    
    if (devType == TMCTL_DEV_GPON)
    {     
        ret = tmctl_RdpaSetOverAllShaper(shaper_p);
        if (ret)
        {
            tmctl_error("tmctl_RdpaConfigOverAllShaper ERROR! ret=%d", ret);
            return ret;
        }
    }
#endif
    return ret;
}  /* End of tmctl_setOverAllShaper_plat() */


/* ----------------------------------------------------------------------------
 * This function link port to the overall shaper
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_linkOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;
    
    tmctl_debug(" Enter: devType=%d \n", devType);
#if defined(BCM_PON)    
    if (devType == TMCTL_DEV_GPON)
    {     
        ret = tmctl_RdpaLinkOverAllShaper(if_p->gponIf.tcontid, 1);
        if (ret == TMCTL_ERROR)
        {
            tmctl_error("tmctl_RdpaLinkOverAllShaper ERROR! ret=%d", ret);
            return ret;
        }
    }
#endif
    return ret;
}  /* End of tmctl_linkOverAllShaper_plat() */

/* ----------------------------------------------------------------------------
 * This function unlink port from the overall shaper
 *
 * Parameters:
 *    devType (IN) tmctl device type.
 *    if_p (IN) Port identifier.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctl_unlinkOverAllShaper_plat(tmctl_devType_e devType,
                                     tmctl_if_t*     if_p)
{
    tmctl_ret_e ret  = TMCTL_UNSUPPORTED;
    
    tmctl_debug(" Enter: devType=%d", devType);
#if defined(BCM_PON)    
    if (devType == TMCTL_DEV_GPON)
    {     
        ret = tmctl_RdpaLinkOverAllShaper(if_p->gponIf.tcontid, 0);
        if (ret == TMCTL_ERROR)
        {
            tmctl_error("tmctl_RdpaLinkOverAllShaper ERROR! ret=%d", ret);
            return ret;
        }
    }
#endif
    return ret;
}  /* End of tmctl_unLinkOverAllShaper_plat() */

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
tmctl_ret_e tmctl_getQueueProfile_plat(int queueProfileId,
                                       tmctl_queueProfile_t* queueProfile_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: qProfileId=%d", queueProfileId);

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
tmctl_ret_e tmctl_setQueueProfile_plat(int queueProfileId,
                                       tmctl_queueProfile_t* queueProfile_p)
{
   tmctl_ret_e ret = TMCTL_UNSUPPORTED;

   tmctl_debug("Enter: queueProfileId=%d dropProb=%d minThreshold=%d maxThreshold=%d",
               queueProfileId, queueProfile_p->dropProb, queueProfile_p->minThreshold,
               queueProfile_p->maxThreshold);

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

#if defined(BCM_DSL_RDP)
   if (devType == TMCTL_DEV_ETH)
   {
      rdpa_if rdpaIf;

      ret = tmctlRdpa_getRdpaIfByIfname(if_p->ethIf.ifname, &rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf by ifname %s", if_p->ethIf.ifname);
         return ret;
      }

      ret = tmctlRdpa_getQueueDropAlg(RDPA_IOCTL_DEV_PORT, rdpaIf, queueId, dropAlg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlRdpa_getQueueDropAlg ERROR! ret=%d", ret);
      }
   }

#elif defined(BCM_PON) || defined(BCM_DSL_XRDP)
   int rdpaDev;
   int ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
       return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) 
   {
       return tmctl_RdpaGetQueueDropAlg(rdpaDev, devType, ifId, queueId, dropAlg_p);
   }
   else if (ret)
   {
       tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                   devType, ifId, ret);
       return ret;
   }

   ret = tmctlRdpa_getQueueDropAlg(rdpaDev, ifId, queueId, dropAlg_p);
   if (ret == TMCTL_ERROR)
   {
       tmctl_error("tmctlRdpa_getQueueDropAlg ERROR! ret=%d", ret);
   }
#endif

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

#if defined(BCM_DSL_RDP)

   if (dropAlg_p->dropAlgorithm == TMCTL_DROP_RED)
   {
      /* Currently Runner firmware does not support RED.
       * RED can be implemented as WRED with equal low and high
       * class thresholds.
       */
      dropAlg_p->dropAlgorithm = TMCTL_DROP_WRED;
      dropAlgHi_p->redMinThreshold = dropAlgLo_p->redMinThreshold;
      dropAlgHi_p->redMaxThreshold = dropAlgLo_p->redMaxThreshold;
   }

   if ((devType == TMCTL_DEV_ETH) || (devType == TMCTL_DEV_XTM) || (devType == TMCTL_DEV_GPON))
   {
      int      rdpaDev;
      rdpa_if  rdpaIf;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

     ret = tmctlRdpa_setQueueDropAlg(rdpaDev, rdpaIf, queueId, dropAlg_p);
     if (ret == TMCTL_ERROR)
     {
        tmctl_error("tmctlRdpa_setQueueDropAlg ERROR! ret=%d", ret);
        return ret;
     }
   }

#elif defined(BCM_PON) || defined(BCM_DSL_XRDP)
   int      rdpaDev;
   int      ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       return tmctl_RdpaSetQueueDropAlg(rdpaDev, devType, ifId, queueId, dropAlg_p);
   }
   else if (ret)
   {
      tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                  devType, ifId, ret);
      return TMCTL_ERROR;
   }

   ret = tmctlRdpa_setQueueDropAlg(rdpaDev, ifId, queueId, dropAlg_p);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_setQueueDropAlg ERROR! ret=%d", ret);
      return ret;
   }
#endif

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
#if defined(BCM_DSL_RDP) || defined(BCM_DSL_XRDP) /* What to do for XTM */
      ret = tmctlRdpa_getQueueDropAlg(RDPA_IOCTL_DEV_XTM, rdpa_wan_type_to_if(rdpa_wan_dsl), channelId, dropAlg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlRdpa_getQueueDropAlg ERROR! ret=%d", ret);
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
#if defined(BCM_DSL_RDP) || defined(BCM_DSL_XRDP) /* XTM ?? */
      ret = tmctlRdpa_setQueueDropAlg(RDPA_IOCTL_DEV_XTM, rdpa_wan_type_to_if(rdpa_wan_dsl), channelId, dropAlg_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlRdpa_setQueueDropAlg ERROR! ret=%d", ret);
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

#if defined(BCM_DSL_RDP)

   if ((devType == TMCTL_DEV_ETH) || (devType == TMCTL_DEV_GPON) || (devType == TMCTL_DEV_XTM))
   {
      int rdpaDev;
      rdpa_if rdpaIf;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

      /* get the queue statistics */
      ret = tmctlRdpa_getQueueStats(rdpaDev, rdpaIf, queueId, stats_p);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("tmctlRdpa_getQueueStats ERROR! ret=%d", ret);
         return ret;
      }
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)
   int rdpaDev;
   int ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       /* Get Queue stats from runner even for Ethernet switch interface */
       ret = tmctl_RdpaGetQueueStats(rdpaDev, devType, ifId, queueId, stats_p);
       return ret;
   }
   else if (ret)
   {
       tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
               devType, ifId, ret);
       return TMCTL_ERROR;
   }

   ret = tmctlRdpa_getQueueStats(rdpaDev, ifId, queueId, stats_p);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_getQueueStats ERROR, ret=%d", ret);
      return ret;
   }
#endif

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

#if defined(BCM_DSL_RDP)
   if (devType == TMCTL_DEV_ETH)
   {
      int rdpaDev;
      rdpa_if rdpaIf;

      ret = tmctl_ifConvert(devType, if_p, &rdpaDev, (int *)&rdpaIf);
      if (ret == TMCTL_ERROR)
      {
         tmctl_error("Cannot find rdpaIf");
         return ret;
      }

      if (is_lan(devType, rdpaIf))
      {
         ret = tmctlEthSw_getPortTmParms(if_p->ethIf.ifname, tmParms_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlEthSw_getPortTmParms ERROR! ret=%d", ret);
            return ret;
         }
      }
      else
      {
         ret = tmctlRdpa_getTmParms(rdpaDev, rdpaIf, tmParms_p);
         if (ret == TMCTL_ERROR)
         {
            tmctl_error("tmctlRdpa_getTmParms ERROR! ret=%d", ret);
            return ret;
         }
      }
   }

#elif  defined(BCM_PON) || defined(BCM_DSL_XRDP)
   int rdpaDev;
   int ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       if (is_switch_intf(devType, ifId))
       {
           ret = tmctlEthSw_getPortTmParms(if_p->ethIf.ifname, tmParms_p);
       }
       else
       {
           ret = tmctl_RdpaGetPortTmParms(rdpaDev, devType, ifId, tmParms_p);
       }
       return ret;
   }
   else if (ret)
   {
       tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
               devType, ifId, ret);
       return TMCTL_ERROR;
   }
#if defined(BUILD_DSL)
   if(devType == TMCTL_DEV_XTM)
   {
      ret = tmctl_xtm_getTmParms(tmParms_p);
   }
   else
#endif   //BUILD_DSL
   {
      ret = tmctlRdpa_getTmParms(rdpaDev, ifId, tmParms_p);
   }
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_getTmParms ERROR! ret=%d", ret);
      return ret;
   }
#endif

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

#if defined(BCM_PON)
   int rdpaDev;
   int ifId;
   
   ret = tmctl_ifConvert(cfg_p->devType, cfg_p->if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
       return TMCTL_ERROR;
   }

   ret = tmctl_RdpaGetDscpToPbit(rdpaDev, ifId, cfg_p);
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_getDscpToPbit ERROR! ret=%d", ret);
   }
#endif

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
    int i = 0;

    tmctl_debug("Enter: ");
    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        tmctl_debug("dscp[%d]=%d", i, cfg_p->dscp[i]);
    }

#if defined(BCM_PON)
    int rdpaDev;
    int ifId;

    ret = tmctl_ifConvert(cfg_p->devType, cfg_p->if_p, &rdpaDev, &ifId);
    if (ret != TMCTL_SUCCESS)
    {
        return TMCTL_ERROR;
    }

    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        if (cfg_p->dscp[i]!= U32_MAX && cfg_p->dscp[i] > MAX_PBIT_VALUE)
        {
            tmctl_error("Pbit must < %d, dscp[%d]=%d",
                MAX_PBIT_VALUE, i, cfg_p->dscp[i]);
            return TMCTL_ERROR;
        }
    }

    ret = tmctl_RdpaSetDscpToPbit(rdpaDev, ifId, cfg_p);
    if (ret == TMCTL_ERROR)
    {
        tmctl_error("tmctl_setDscpToPbit ERROR! ret=%d", ret);
    }

#endif

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

#if defined(BCM_PON)
    int rdpaDev;
    int ifId;

    ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
    if (ret != TMCTL_SUCCESS)
    {
        return TMCTL_ERROR;
    }

    ret = tmctlRdpa_getPbitToQ(rdpaDev, ifId, cfg_p);

    if (ret == TMCTL_ERROR)
    {
        tmctl_error("tmctlRdpa_getPbitToQ ERROR! ret=%d", ret);
    }
#endif

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
    int i = 0;

    tmctl_debug("Enter: devType=%d", devType);

    for (i = 0; i < TOTAL_PBIT_NUM; i++)
    {
        tmctl_debug("pbit[%d]=%d", i, cfg_p->pbit[i]);
    }

#if defined(BCM_PON)
    int rdpaDev;
    int ifId;

    ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
    if (ret != TMCTL_SUCCESS)
    {
        return TMCTL_ERROR;
    }

    ret = tmctlRdpa_setPbitToQ(rdpaDev, ifId, cfg_p);

    if (ret == TMCTL_ERROR)
    {
        tmctl_error("tmctlRdpa_setPbitToQ ERROR! ret=%d", ret);
    }

#endif

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

#if defined(BCM_PON)
    ret = tmctlRdpa_getForceDscpToPbit(dir, enable_p);
    if (ret == TMCTL_ERROR)
    {
        tmctl_error("tmctlRdpa_getForceDscpToPbit ERROR! ret=%d", ret);
    }
#endif

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

#if defined(BCM_PON)
    ret = tmctlRdpa_setForceDscpToPbit(dir, enable_p);
    if (ret == TMCTL_ERROR)
    {
        tmctl_error("tmctlRdpa_setForceDscpToPbit ERROR! ret=%d", ret);
    }

#endif

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

#if defined(BCM_PON)
    if (dir >= TMCTL_DIR_MAX)
    {
        tmctl_error("dir our of range. dir=%d", dir);
        return TMCTL_ERROR;
    }

    if (type >= TMCTL_QOS_MAX)
    {
        tmctl_error("type our of range. type=%d", type);
        return TMCTL_ERROR;
    }

    ret = tmctlRdpa_getPktBasedQos(dir, type, enable_p);
    if (ret == TMCTL_ERROR)
    {
        tmctl_error("tmctlRdpa_getPktBasedQos ERROR! ret=%d", ret);
    }
#endif

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

#if defined(BCM_PON)
    if (dir >= TMCTL_DIR_MAX)
    {
        tmctl_error("dir our of range. dir=%d", dir);
        return TMCTL_ERROR;
    }

    if (type >= TMCTL_QOS_MAX)
    {
        tmctl_error("type our of range. type=%d", type);
        return TMCTL_ERROR;
    }

    ret = tmctlRdpa_setPktBasedQos(dir, type, enable_p);
    if (ret == TMCTL_ERROR)
    {
        tmctl_error("tmctlRdpa_setPktBasedQos ERROR! ret=%d", ret);
    }
#endif

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

#if defined(BCM_PON) || defined(BCM_DSL_XRDP) /* What about DSL-RDP ? */
   int rdpaDev;
   int ifId;

   ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
   if (ret != TMCTL_SUCCESS)
   {
      return TMCTL_ERROR;
   }

   ret = is_supported_rdpa_api(devType, if_p, ifId);
   if (ret == 1) /* rdpa_user_apis supported */
   {
       /* Set the queue size in runner for Ethernet switch LAN ports as well */
       ret =  tmctl_RdpaSetQueueSize(rdpaDev, devType, ifId, queueId, size);
       return ret;
   }
   else if (ret)
   {
      tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                  devType, ifId, ret);
      return TMCTL_ERROR;
   }
#if defined(BUILD_DSL)
   if(devType == TMCTL_DEV_XTM)
   {
      ret = tmctl_xtm_setQueueSize(size);
   }
   else
#endif   //BUILD_DSL
   {
      ret = tmctlRdpa_setQueueSize(rdpaDev, ifId, queueId, size);
   }
   if (ret == TMCTL_ERROR)
   {
      tmctl_error("tmctlRdpa_setQueueSize ERROR! ret=%d", ret);
      return ret;
   }
#endif

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

#if defined(BCM_PON) || defined(BCM_DSL_XRDP)
    int rdpaDev, ifId;

    ret = tmctl_ifConvert(devType, if_p, &rdpaDev, &ifId);
    if (ret != TMCTL_SUCCESS)
    {
      return TMCTL_ERROR;
    }

    ret = is_supported_rdpa_api(devType, if_p, ifId);
    if (ret == 1) /* rdpa_user_apis supported */
    {
        if (is_switch_intf(devType, ifId))
        {
            ret = TMCTL_UNSUPPORTED; /* Queue shaper should be set in switch */
        }
        else
        {
            ret = tmctl_RdpaSetQueueShaper(rdpaDev, devType, ifId, queueId, shaper_p);
        }
        return ret;
    }
    else if (ret)
    {
        tmctl_error("is_supported_rdpa_api returns error, devType=%d, if=%d, ret%d",
                devType, ifId, ret);
        return TMCTL_ERROR;
    }

    ret = tmctlRdpa_setQueueShaper(rdpaDev, ifId, queueId, shaper_p);
    if (ret == TMCTL_ERROR)
    {
      tmctl_error("tmctl_setQueueShaper ERROR! ret=%d", ret);
      return ret;
    }
#endif

   return ret;
}  /* End of tmctl_setQueueShaper() */

static int is_wan_type_us_rate(char *wan_type, char *rate)
{
#define RATE_STR_LEN 2
    int count;
    char buf[16] = {0};

    if (wan_type)
    {
        count = (int)devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD,
                                       RDPA_WAN_TYPE_PSP_KEY, 0, sizeof(buf), (void *)buf);
        if (count <= 0)
            return 0;

        if (strcasecmp(buf, wan_type))
            return 0;
    }

    count = (int)devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD,
                                   RDPA_WAN_RATE_PSP_KEY, 0, sizeof(buf), (void *)buf);
    if (count <= 0)
        return 0;

    if (!strncasecmp(&buf[RATE_STR_LEN], rate, RATE_STR_LEN))
        return 1;

    return 0;
}

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
#if defined(BUILD_DSL)
    uint32_t ulQueueSize = 0;
#endif   //BUILD_DSL
    if (TMCTL_DIR_DN == dir)
        return TMCTL_DEF_ETH_Q_SZ_DS;

#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
    switch (devType)
    {
        case TMCTL_DEV_EPON:
            if (is_wan_type_us_rate(NULL, RDPA_WAN_RATE_10G) || is_wan_type_us_rate(NULL, RDPA_WAN_RATE_2_5G))
                return TMCTL_DEF_ETH_Q_SZ_US_10G;

            return TMCTL_DEF_ETH_Q_SZ_US_1G_EPON;

        case TMCTL_DEV_GPON:
            if (is_wan_type_us_rate(NULL, RDPA_WAN_RATE_10G) || is_wan_type_us_rate(NULL, RDPA_WAN_RATE_2_5G))
                return TMCTL_DEF_ETH_Q_SZ_US_10G;

            return TMCTL_DEF_ETH_Q_SZ_US_1G_GPON;

        case TMCTL_DEV_ETH:
            if (is_wan_type_us_rate(RDPA_WAN_TYPE_VALUE_EPON, RDPA_WAN_RATE_10G) ||
                is_wan_type_us_rate(RDPA_WAN_TYPE_VALUE_XGPON1, RDPA_WAN_RATE_2_5G) ||
                is_wan_type_us_rate(RDPA_WAN_TYPE_VALUE_XGS, RDPA_WAN_RATE_10G) ||
                is_wan_type_us_rate(RDPA_WAN_TYPE_VALUE_NGPON2, RDPA_WAN_RATE_10G))
            {
                return TMCTL_DEF_ETH_Q_SZ_US_10G;
            }
            else if(is_wan_type_us_rate(RDPA_WAN_TYPE_VALUE_EPON, RDPA_WAN_RATE_1G))
            {
                return TMCTL_DEF_ETH_Q_SZ_US_1G_EPON;
            }
#if defined(BCM_DSL_XRDP)
            return TMCTL_DEF_ETH_Q_SZ_US_10G;
#else
            return TMCTL_DEF_ETH_Q_SZ_US_1G_GPON; /* part of auto calculation for TM memory size */
#endif
#if defined(BUILD_DSL)
        case TMCTL_DEV_XTM:
            tmctl_xtm_getQueueSize(&ulQueueSize);
            return ulQueueSize;
#endif   //BUILD_DSL
        default:
            tmctl_debug("warning: device type not found, queue size set to 0");
            return 0;
    }
#else
    return TMCTL_DEF_ETH_Q_SZ_US;
#endif
}

tmctl_ret_e tmctl_createPolicer_plat(tmctl_policer_t *policer_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;
#if defined(BCM_PON) 
    ret = tmctl_RdpaCreatePolicer(policer_p);
#endif
    return ret;
}  /* End of tmctl_createPolicer_plat() */

tmctl_ret_e tmctl_modifyPolicer_plat(tmctl_policer_t *policer_p)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;
#if defined(BCM_PON) 
		ret = tmctl_RdpaModifyPolicer(policer_p);
#endif
    return ret;
}  /* End of tmctl_modifyPolicer_plat() */

tmctl_ret_e tmctl_deletePolicer_plat(tmctl_dir_e dir, int policerId)
{
    tmctl_ret_e ret = TMCTL_UNSUPPORTED;
#if defined(BCM_PON) 
    ret = tmctl_RdpaDeletePolicer(dir, policerId);
#endif
    return ret;
}    /* End of tmctl_deletePolicer_plat() */


