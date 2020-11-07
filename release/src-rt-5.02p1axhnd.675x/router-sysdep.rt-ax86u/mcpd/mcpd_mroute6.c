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
 * File Name  : mcpd_mroute6.c
 *
 * Description: API for kernel IPv6 multicast route and ssm interaction
 *              
 ***************************************************************************/
#ifdef SUPPORT_MLD
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/mroute6.h>
#include "mcpd.h"
#include "common.h"
#include "mld.h"
#include "mcpd_nl.h"
#include "mcpd_mroute6.h"
#include "obj_hndlr.h"

extern t_MCPD_ROUTER mcpd_router;

int mcpd_mld_mroute_init( void )
{
    int optval = 1; /* initialize multicast routing, 
                       option is required though unused */
    if(setsockopt(mcpd_router.sock_mld, IPPROTO_IPV6, MRT6_INIT, 
                                     (void*)&optval, sizeof(optval)) < 0)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "MRT6_INIT failed with err %d: %s", errno, strerror(errno));
        return -1;
    }
    return 0;
}

/* 
 * Add a multicast interface to the kernel 
 */
int mcpd_mld_krnl_proxy_add_mif(unsigned short ifIndex, unsigned short *pMifi)
{
   struct mif6ctl mc;
   int error;
   unsigned int mifi = *pMifi;

   /* assign an index to this interface if not already set */
   if(mifi >= MAXMIFS)
   {
       for ( mifi= 0; mifi < MAXMIFS; mifi++)
       {
           if( 0 == (mcpd_router.mifiBits & (1<<mifi)))
           {
              mcpd_router.mifiBits |= (1<<mifi);
              *pMifi = mifi;
              break;
           }
       }

       if ( mifi >= MAXMIFS )
       {
           MCPD_TRACE(MCPD_TRC_ERR, "ERROR: Can't add IPV6 multicast interface : Exceeds limit\n");
           return 0;
       }
   }

   bzero(&mc, sizeof(struct mif6ctl));
   mc.mif6c_mifi = mifi;
   mc.mif6c_flags = 0;
   mc.vifc_threshold = 0;
   mc.vifc_rate_limit = 0;
   mc.mif6c_pifi = ifIndex;

   error = setsockopt(mcpd_router.sock_mld, 
                      IPPROTO_IPV6, 
                      MRT6_ADD_MIF,
                      (char *)&mc, 
                      sizeof(mc));
   if(error < 0) 
   {
      MCPD_TRACE(MCPD_TRC_LOG, "setsockopt - MRT6_ADD_MIF with err %d: %s",
                 errno, strerror(errno));
   }

   return error;
} /* mcpd_mld_krnl_proxy_add_mif */

/* 
 * delete a multicast interface from the kernel 
 */
int mcpd_mld_krnl_proxy_del_mif(unsigned short ifIndex, unsigned short mifi)
{
   struct mif6ctl mc;
   int error;

   if(mifi >= MAXMIFS)
   {
       return 0;
   }

   mcpd_router.mifiBits &= ~(1 << mifi);

   bzero(&mc, sizeof(struct mif6ctl));
   mc.mif6c_mifi = mifi;
   mc.mif6c_flags = 0;
   mc.vifc_threshold = 0;
   mc.vifc_rate_limit = 0;
   mc.mif6c_pifi = ifIndex;

   error = setsockopt(mcpd_router.sock_mld, 
                      IPPROTO_IPV6, 
                      MRT6_DEL_MIF,
                      (char *)&mc, 
                      sizeof(mc));
   if(error < 0) 
   {
      MCPD_TRACE(MCPD_TRC_LOG, "setsockopt - MRT6_DEL_MIF with err %d: %s",
                 errno, strerror(errno));
   }

   return error;
} /* mcpd_mld_krnl_proxy_add_mif */

/*
 * Install or modify an MFC entry in the kernel
 */
t_MCPD_RET_CODE mcpd_mld_krnl_proxy_chg_mfc (UINT8 *source, 
                                             UINT8 *group)
{
   struct mf6cctl mc;
   t_MCPD_INTERFACE_OBJ *ifp;
   t_MCPD_INTERFACE_OBJ *ifp_br;
   t_MCPD_GROUP_OBJ *gp;
   t_MCPD_SRC_OBJ *srcObj;
   int delmfc = 1;
   int mode;

   if(!group)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "Group not specified");
      return MCPD_RET_GENERR;
   }

   memset(&mc, 0, sizeof(struct mf6cctl));
   mc.mf6cc_mcastgrp.sin6_family = AF_INET6;
   memcpy(&mc.mf6cc_mcastgrp.sin6_addr, group, sizeof(struct in6_addr));
   mc.mf6cc_origin.sin6_family = AF_INET6;
   if(source)
   {
      memcpy(&mc.mf6cc_origin.sin6_addr, source, sizeof(struct in6_addr));
   }
   else
   {
      memset(&mc.mf6cc_origin.sin6_addr, 0, sizeof(struct in6_addr));
   }

   if (IN6_IS_ADDR_UNSPECIFIED(&mc.mf6cc_origin.sin6_addr))
   {
      mode = MCPD_FMODE_EXCLUDE;
   }
   else
   {
       mode = MCPD_FMODE_INCLUDE;
   }

   for (ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
   {
      if ((ifp->if_dir == MCPD_UPSTREAM) &&
          (ifp->if_type & MCPD_IF_TYPE_ROUTED))
      {
         mc.mf6cc_parent = ifp->mifi;
         delmfc = 1;
         memset (&mc.mf6cc_ifset, 0, sizeof(mc.mf6cc_ifset));
         for(ifp_br = mcpd_router.interfaces; ifp_br; ifp_br = ifp_br->next)
         {
            if ((ifp_br->if_dir == MCPD_DOWNSTREAM) &&
                (ifp_br->if_type & MCPD_IF_TYPE_ROUTED))
            {
               gp = mcpd_interface_group_lookup(MCPD_PROTO_MLD, ifp_br, group);
               if(gp)
               {
                  srcObj = mcpd_group_src_lookup(MCPD_PROTO_MLD,
                                                 gp, 
                                                 (UINT8 *)&mc.mf6cc_origin.sin6_addr,
                                                 mode);
                  if ((srcObj) && (mcpd_is_wan_service_associated_with_bridge(ifp, ifp_br) == MCPD_TRUE) && (ifp_br->mifi < MAXMIFS))
                  {
                     IF_SET(ifp_br->mifi, &(mc.mf6cc_ifset));
                     delmfc = 0;
                  }
               }
            }
         }
         if (setsockopt(mcpd_router.sock_mld, 
                        IPPROTO_IPV6, 
#if defined(MRT6_DEL_MFC_PROXY) && defined(MRT6_ADD_MFC_PROXY)
                        delmfc ? MRT6_DEL_MFC_PROXY : MRT6_ADD_MFC_PROXY,
#else                        
                        delmfc ? MRT6_DEL_MFC : MRT6_ADD_MFC, 
#endif                        
                        (char *)&mc, 
                        sizeof(mc)) < 0)
         {
            MCPD_TRACE(MCPD_TRC_LOG, 
                       "setsockopt- %s with err %d: %s",
#if defined(MRT6_DEL_MFC_PROXY) && defined(MRT6_ADD_MFC_PROXY)
                       delmfc ? "MRT6_DEL_MFC_PROXY" : "MRT6_ADD_MFC_PROXY",
#else                       
                       delmfc ? "MRT6_DEL_MFC" : "MRT6_ADD_MFC",
#endif                       
                       errno, strerror(errno));
         }
      }
   }
   return MCPD_RET_OK;

} /* mcpd_mld_krnl_proxy_chg_mfc */
#endif /* SUPPORT_MLD */
