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
 * File Name  : mcpd_mroute.c
 *
 * Description: API for IPv4 multicast routing 
 *              
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/mroute.h>
#include "mcpd.h"
#include "common.h"
#include "igmp.h"
#include "mcpd_nl.h"
#include "mcpd_mroute.h"
#include "obj_hndlr.h"
#include "prctl.h"

extern t_MCPD_ROUTER mcpd_router;

int mcpd_igmp_mroute_init( void )
{
   int optval = 1; /* initialize multicast routing, 
                      option is required though unused */
   if (setsockopt(mcpd_router.sock_igmp, IPPROTO_IP, MRT_INIT, 
                                      (void*)&optval, sizeof(optval)) < 0)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "MRT_INIT failed with err %d, %s\n", errno, strerror(errno));
      return -1;
   }
   return 0;
}

/* 
 * Add a multicast interface to the kernel 
 */
int mcpd_igmp_krnl_proxy_add_vif (unsigned short ifindex, unsigned short *pVifi)
{
   struct vifctl vc;
   int error;
   unsigned short vifi = *pVifi;

   /* assign an index to this interface if not already set */
   if(vifi >= MAXVIFS)
   {
       for ( vifi = 0; vifi < MAXVIFS; vifi++)
       {
           if( 0 == (mcpd_router.vifiBits & (1<<vifi)))
           {
              mcpd_router.vifiBits |= (1<<vifi);
              *pVifi = vifi;
              break;
           }
       }

       if ( vifi >= MAXVIFS )
       {
           MCPD_TRACE(MCPD_TRC_ERR, "ERROR: Can't add multicast interface : Exceeds limit\n");
           return 0;
       }
   }

   bzero(&vc, sizeof(struct vifctl));
   vc.vifc_vifi = vifi;
   vc.vifc_flags = VIFF_USE_IFINDEX;
   vc.vifc_threshold = 0;
   vc.vifc_rate_limit = 0;
   vc.vifc_lcl_ifindex = ifindex;
   vc.vifc_rmt_addr.s_addr = INADDR_ANY;

   if ((error = setsockopt(mcpd_router.sock_igmp, 
                           IPPROTO_IP, 
                           MRT_ADD_VIF, 
                           (char *)&vc,
                           sizeof(vc))) < 0)
   {
      MCPD_TRACE(MCPD_TRC_LOG, 
                 "setsockopt - MRT_ADD_VIF %d: %d, with err %d: %s",
                 ifindex, vifi, errno, strerror(errno));
      return -1;
   }

  return 0;
} /* mcpd_igmp_krnl_proxy_add_vif */

/* 
 * Remove a mutlicast interface from the kernel 
 */
int mcpd_igmp_krnl_proxy_del_vif (unsigned short ifindex, unsigned short vifi)
{
   struct vifctl vc;
   int error;

   if(vifi >= MAXVIFS)
   {
       return 0;
   }

   mcpd_router.vifiBits &= ~(1 << vifi);

   bzero(&vc, sizeof(struct vifctl));
   vc.vifc_vifi = vifi;
   vc.vifc_flags = VIFF_USE_IFINDEX;
   vc.vifc_threshold = 0;
   vc.vifc_rate_limit = 0;
   vc.vifc_lcl_ifindex = ifindex;
   vc.vifc_rmt_addr.s_addr = INADDR_ANY;

   if ((error = setsockopt(mcpd_router.sock_igmp, 
                           IPPROTO_IP, 
                           MRT_DEL_VIF, 
                           (char *)&vc,
                           sizeof(vc))) < 0)
   {
      MCPD_TRACE(MCPD_TRC_LOG, 
                 "setsockopt - MRT_DEL_VIF %d: %d, with err %d: %s",
                 ifindex, vifi, errno, strerror(errno));
      return -1;
   }

  return 0;
} /* mcpd_igmp_krnl_proxy_add_vif */

/*
 * Install or modify an MFC entry in the kernel
 */
t_MCPD_RET_CODE mcpd_igmp_krnl_proxy_chg_mfc (UINT8 *source, 
                                              UINT8 *group)
{
   struct mfcctl mc;
   t_MCPD_INTERFACE_OBJ *ifp;
   t_MCPD_INTERFACE_OBJ *ifp_br;
   struct in_addr src;
   struct in_addr grp;
   t_MCPD_GROUP_OBJ *gp;
   t_MCPD_SRC_OBJ *srcObj;
   int delmfc = 1;
   int mode;

   if(!group)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "Group not specified");
      return MCPD_RET_GENERR;
   }

   grp.s_addr = ((struct in_addr *)group)->s_addr;
   if(source)
   {
      src.s_addr = ((struct in_addr *)source)->s_addr;
   }
   else
   {
      src.s_addr = 0;
   }

   if ( 0 == src.s_addr )
   {
       mode = MCPD_FMODE_EXCLUDE;
   }
   else
   {
        mode = MCPD_FMODE_INCLUDE;
   }

   memset(&mc, 0, sizeof(struct mfcctl));
   mc.mfcc_origin.s_addr = src.s_addr;
   mc.mfcc_mcastgrp.s_addr = grp.s_addr;

   for (ifp = mcpd_router.interfaces; ifp; ifp = ifp->next) 
   {
      if ((ifp->if_dir == MCPD_UPSTREAM) &&
          (ifp->if_type & MCPD_IF_TYPE_ROUTED))
      {
         mc.mfcc_parent = ifp->vifi;
         delmfc = 1;
         memset (mc.mfcc_ttls, 0, sizeof(mc.mfcc_ttls));
         for(ifp_br = mcpd_router.interfaces; ifp_br; ifp_br = ifp_br->next)
         {
            if ((ifp_br->if_dir == MCPD_DOWNSTREAM) &&
                (ifp_br->if_type & MCPD_IF_TYPE_ROUTED))
            {
               gp = mcpd_interface_group_lookup(MCPD_PROTO_IGMP, ifp_br, group);
               if(gp)
               {
                  srcObj = mcpd_group_src_lookup(MCPD_PROTO_IGMP, 
                                                 gp, 
                                                 (UINT8 *)&src.s_addr, 
                                                 mode);
                  if ((srcObj) && (mcpd_is_wan_service_associated_with_bridge(ifp, ifp_br) == MCPD_TRUE) && (ifp_br->vifi < MAXVIFS))
                  {
                     mc.mfcc_ttls[ifp_br->vifi] = 1;
                     delmfc = 0;
                  }
               }
            }
         }
         if (setsockopt(mcpd_router.sock_igmp, 
                        IPPROTO_IP, 
#if defined(MRT_DEL_MFC_PROXY) && defined(MRT_ADD_MFC_PROXY)
                        delmfc ? MRT_DEL_MFC_PROXY : MRT_ADD_MFC_PROXY,
#else                        
                        delmfc ? MRT_DEL_MFC : MRT_ADD_MFC,
#endif                        
                        (char *)&mc, 
                        sizeof(mc)) < 0)
         {
            MCPD_TRACE(MCPD_TRC_LOG, 
                       "setsockopt- %s with err %d: %s",
#if defined(MRT_DEL_MFC_PROXY) && defined(MRT_ADD_MFC_PROXY)
                       delmfc ? "MRT_DEL_MFC_PROXY" : "MRT_ADD_MFC_PROXY",
#else                       
                       delmfc ? "MRT_DEL_MFC" : "MRT_ADD_MFC",
#endif                       
                       errno, strerror(errno));
         }
      }
   }

   return 0;
} /* mcpd_igmp_krnl_proxy_chg_mfc */

