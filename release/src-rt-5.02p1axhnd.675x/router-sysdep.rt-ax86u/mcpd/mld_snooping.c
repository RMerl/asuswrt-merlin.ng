/*
* <:copyright-BRCM:2006:proprietary:standard
* 
*    Copyright (c) 2006 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/
/***************************************************************************
 * File Name  : mld_snooping.c
 *
 * Description: API for MLD snooping processing
 *              
 ***************************************************************************/
#ifdef SUPPORT_MLD
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mcpd.h"
#include "common.h"
#include "mld.h"
#include "mld_snooping.h"
#include "mcpd_nl.h"
#include "bridgeutil.h"

extern t_MCPD_ROUTER mcpd_router;

t_MCPD_RET_CODE mcpd_mld_snooping_init(void)
{
   return MCPD_RET_OK;
} /* mcpd_mld_snooping_init */

void mcpd_mld_set_wan_info(t_MCPD_INTERFACE_OBJ *ifp_in, t_MCPD_WAN_INFO_ARRAY wan_info)
{
   t_MCPD_INTERFACE_OBJ *ifp = NULL;
   int idx = 0;

   for(ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
   {
      if ((ifp->if_dir == MCPD_UPSTREAM) && (ifp->proto_enable & MCPD_IPV6_MCAST_ENABLE))
      {
          /* add entries for routed and bridge upstream interfaces 
             according to the downstream interface type */
          if((ifp_in->if_type & MCPD_IF_TYPE_ROUTED) &&
             (ifp->if_type & MCPD_IF_TYPE_ROUTED) &&
             (!IN6_IS_ADDR_UNSPECIFIED(&ifp->if_addr6))&&
             (mcpd_is_wan_service_associated_with_bridge(ifp, ifp_in) == MCPD_TRUE))
          {
              wan_info[idx].ifi = ifp->if_index;
              wan_info[idx].if_ops = MCPD_IF_TYPE_ROUTED;
              idx++;
          }

          if((ifp_in->if_type & MCPD_IF_TYPE_BRIDGED) &&
             (ifp->if_type & MCPD_IF_TYPE_BRIDGED) &&
             (MCPD_TRUE == mcpd_is_bridge_member(ifp_in->if_name, ifp->if_index)))
          {
              wan_info[idx].ifi = ifp->if_index;
              wan_info[idx].if_ops = MCPD_IF_TYPE_BRIDGED;
              idx++;
          }
      }

      if(idx >= MCPD_MAX_IFS) 
      {
          MCPD_ASSERT(0);
          break;
      }
   }
}


t_MCPD_RET_CODE mcpd_mld_update_snooping_info(t_MCPD_INTERFACE_OBJ *ifp_in,
                                              t_MCPD_GROUP_OBJ *gp, 
                                              t_MCPD_REP_OBJ *rep,
                                              UINT8 *src,
                                              int mode, 
                                              t_MCPD_PKT_INFO *pkt_info)
{
    struct in6_addr zero_addr = {.s6_addr32 = {0,0,0,0}};
    t_MCPD_WAN_INFO_ARRAY wan_info;

    if (!(ifp_in->proto_enable & MCPD_MLD_SNOOPING_ENABLE))
    {
        MCPD_TRACE(MCPD_TRC_INFO, "Snooping not enabled");
        return MCPD_RET_OK;
    }

    if(!gp || !rep || !pkt_info)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "invalid snoop entry");
        return MCPD_RET_GENERR;
    }
    
    if (!src)
    {
       src = (UINT8 *)&zero_addr;
    }

    bzero((char *)&wan_info, sizeof(t_MCPD_WAN_INFO_ARRAY));
    mcpd_mld_set_wan_info(ifp_in, wan_info);
    
    if (bcm_mcast_api_update_mld_snoop(mcpd_router.sock_nl, 
                                       pkt_info->parent_ifi, 
                                       pkt_info->rxdev_ifi, 
                                       pkt_info->to_acceldev_ifi,
                                       pkt_info->tci, 
                                       pkt_info->lanppp, 
                                       (const struct in6_addr *)gp->addr, 
                                       (const struct in6_addr *)src, 
                                       (const struct in6_addr *)rep->addr,
                                       (UINT8 *)pkt_info->repMac,
                                       rep->version,
                                       mode, 
                                       &wan_info) < 0)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "Error while updating snooping info");
        return MCPD_RET_GENERR;
    }

#if defined(CONFIG_BCM_OVS_MCAST)
    {
        int addentry = 1;
        if ((mode == BCM_MCAST_SNOOP_IN_CLEAR) ||
            (mode == BCM_MCAST_SNOOP_EX_CLEAR)) 
        {
            addentry = 0;
        }
        mcpd_ovs_manage_snoop_entry(pkt_info->rxdev_ifi,
                                    pkt_info->parent_ifi, 
                                    0, /* v4 Group address N/A */
                                    gp->addr, /* v6 Group address */
                                    0, /* v4 Src address N/A */
                                    src, /* v6 Src address */
                                    addentry); /* 1 - Add, 0 - Remove */
    }
#endif

    return MCPD_RET_OK;
} /* mcpd_mld_update_snooping_info */

t_MCPD_RET_CODE mcpd_mld_update_flooding_info(t_MCPD_INTERFACE_OBJ *ifp_in,
                                              t_MCPD_GROUP_OBJ *gp, 
                                              t_MCPD_REP_OBJ *rep,
                                              UINT8 *src,
                                              int mode, 
                                              t_MCPD_PKT_INFO *pkt_info,
                                              t_MCPD_FLOOD_TYPE type)
{
    int  ifInBridge[BRIDGE_MAX_IFS];
    unsigned int  portNum    =  BRIDGE_MAX_IFS;
    unsigned int  idx = 0;
	char ifname[IFNAMSIZ];
    struct in6_addr zero_addr = {.s6_addr32 = {0,0,0,0}};
    t_MCPD_WAN_INFO_ARRAY wan_info;
    short ifflags;
    
    if (!(ifp_in->proto_enable & MCPD_MLD_SNOOPING_ENABLE))
    {
        return MCPD_RET_OK;
    }

    if(!gp || !rep || !pkt_info)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "invalid snoop entry");
        return MCPD_RET_GENERR;
    }
    
    if (!src)
    {
       src = (UINT8 *)&zero_addr;
    }

    bzero((char *)&wan_info, sizeof(t_MCPD_WAN_INFO_ARRAY));
    mcpd_mld_set_wan_info(ifp_in, wan_info);

    if (mcpd_get_bridge_members(gp->ifp->if_name, ifInBridge, &portNum) != 0 ||
        portNum == 0)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "Get bridge members error");
        return MCPD_RET_GENERR;
    }
    
    for (idx = 0; idx < portNum; idx++)
    {
       if (type == FLOOD_TYPE_OTHERS)
       {
           if (ifInBridge[idx] == rep->rep_ifi)
           {
               continue;
           }
       }

       if (mcpd_ifindex_to_name(mcpd_router.sock_igmp, ifInBridge[idx], ifname) == NULL)
       {
           continue;
       }
       
       if (mcpd_is_wan_interface(ifname) == MCPD_TRUE)
       {
            continue;
       }

       ifflags = mcpd_get_interface_flags(ifname);
       if (!(ifflags & IFF_RUNNING))
       {
            continue;
       }

       MCPD_TRACE(MCPD_TRC_INFO, "updating mld flooding info for %s,action=%d\n", ifname, mode);
	   
       if (bcm_mcast_api_update_mld_snoop(mcpd_router.sock_nl, 
                                          pkt_info->parent_ifi, 
                                          ifInBridge[idx],
                                          /* Flooding is a CTC PON scenario where traffic is flooded
                                             to all ports of the bridge when a join is received from
                                             one of the ports. Set acceldev to -1 so that the acceldev
                                             can be found using the legacy netdev_path_get_root logic
                                             in multicast driver  */
                                          -1, 
                                          pkt_info->tci, 
                                          pkt_info->lanppp, 
                                          (const struct in6_addr *)gp->addr, 
                                          (const struct in6_addr *)src, 
                                          (const struct in6_addr *)rep->addr,
                                          (UINT8 *)pkt_info->repMac,
                                          rep->version,
                                          mode, 
                                          &wan_info) < 0)
        {
            MCPD_TRACE(MCPD_TRC_ERR, "Error while updating flooding info for %s\n", ifname);
        }
    }
    
    return MCPD_RET_OK;
} /* mcpd_mld_update_snooping_info */

#endif
