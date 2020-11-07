/***********************************************************************
 *
 *  Copyright (c) 2009-2010  Broadcom Corporation
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

/***************************************************************************
 * File Name  : igmp_proxy.c
 *
 * Description: API for IGMP proxy processing
 *              
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mcpd.h"
#include "common.h"
#include "igmp.h"
#include "igmp_proxy.h"
#include "igmp_main.h"
#include "obj_hndlr.h"

extern t_MCPD_ROUTER mcpd_router;

t_MCPD_RET_CODE mcpd_igmp_proxy_init(void)
{

   if(MCPD_RET_OK != mcpd_igmp_init())
   {
      MCPD_TRACE(MCPD_TRC_ERR, "Error mcpd_igmp_init");
      return MCPD_RET_GENERR;
   }

   return MCPD_RET_OK;
} /* mcpd_igmp_proxy_init */

void mcpd_igmp_order_upstream_ssm_source_list(int nsources, 
                                              struct in_addr *sources)
{
   int i,j;
   struct in_addr sr[MCPD_MAX_UPSTREAM_SSM_SRS];

   for(i = 0; i < MCPD_MAX_UPSTREAM_SSM_SRS; i++) 
   {
      sr[i].s_addr = sources[i].s_addr;
      sources[i].s_addr = 0;
   }

   j = 0;
   for(i=0; ((i < MCPD_MAX_UPSTREAM_SSM_SRS) && (j < nsources)); i++) 
   {
      if (sr[i].s_addr != 0) 
      {
         sources[j].s_addr = sr[i].s_addr;
         j++;
      }
   }

   return;
} /* mcpd_igmp_order_upstream_ssm_source_list */

t_MCPD_RET_CODE mcpd_igmp_update_upstream_ssm(t_MCPD_GROUP_OBJ *group, 
                                              UINT8 *ptr_ssm_info)
{

   int i;
   int k;
   t_MCPD_SRC_OBJ *src = NULL;
   struct in_addr saddr;
   t_MCPD_INTERFACE_OBJ *ifi;
   t_MCPD_GROUP_OBJ *grp;
   int bInclude = MCPD_FALSE;
   int bExclude = MCPD_FALSE;

   struct in_addr * pExSources;
   int exSourcesIdx;
   struct in_addr * pInSources;
   int inSourcesIdx ;

   t_MCPD_IGMP_UPSTREAM_SSM *ssm_info = (t_MCPD_IGMP_UPSTREAM_SSM *)ptr_ssm_info;
   if(!ptr_ssm_info)
   {
      return MCPD_RET_MEMERR;
   }
   
   pExSources = (struct in_addr *)malloc(sizeof(struct in_addr) * MCPD_MAX_UPSTREAM_SSM_SRS);
   if(!pExSources)     
   {
      return MCPD_RET_MEMERR;
   }
   
   pInSources = (struct in_addr *)malloc(sizeof(struct in_addr) * MCPD_MAX_UPSTREAM_SSM_SRS);
   if(!pInSources)
   {
      free(pExSources);
      return MCPD_RET_MEMERR;
   }

   bzero(ssm_info, sizeof(t_MCPD_IGMP_UPSTREAM_SSM));
   bzero(pExSources, (sizeof(struct in_addr) * MCPD_MAX_UPSTREAM_SSM_SRS));
   bzero(pInSources, (sizeof(struct in_addr) * MCPD_MAX_UPSTREAM_SSM_SRS));

   /* update group information for the received interface */
   saddr.s_addr = 0;
   src = mcpd_group_src_lookup(MCPD_PROTO_IGMP, 
                               group, 
                               (UINT8 *)&saddr, 
                               MCPD_FMODE_EXCLUDE);
   if(src)
   {
      /* update group to exclude if there is an exlcude source 
         with addr 0.0.0.0 
         this covers the case where v1 or v2 host is present */ 
      group->fmode = MCPD_FMODE_EXCLUDE;
   }
   else if(group->ex_sources) 
   {
      /* update group to exclude if there are exlcude sources */ 
      group->fmode = MCPD_FMODE_EXCLUDE;
   }
   else if(group->in_sources) 
   {
      /* update group to include if there are only inlcude sources */ 
      group->fmode = MCPD_FMODE_INCLUDE;
   } 
   else
   {
      /* no sources - change group to include */
      group->fmode = MCPD_FMODE_INCLUDE;
   }

   ssm_info->group.s_addr = *(UINT32 *)(group->addr);
   inSourcesIdx = 0;
   exSourcesIdx = 0;
   for(ifi = mcpd_router.interfaces; ifi; ifi = ifi->next)
   {
      if ((ifi->if_dir == MCPD_DOWNSTREAM) &&
          (ifi->if_type & MCPD_IF_TYPE_ROUTED))
      {
         grp = mcpd_interface_group_lookup(MCPD_PROTO_IGMP, ifi, group->addr);
         if(!grp)
         {
            continue;
         }

         /* look for an exclude filter with exclude source address of 0.0.0.0
            this covers the v1 or v2 host present case as well */
         saddr.s_addr = 0;
         src = mcpd_group_src_lookup(MCPD_PROTO_IGMP, 
                                     grp, 
                                     (UINT8 *)&saddr, 
                                     MCPD_FMODE_EXCLUDE);
         if( src )
         {
            ssm_info->fmode = MCPD_FMODE_EXCLUDE;
            ssm_info->numsources = 0;
            free(pExSources);
            free(pInSources);
            return MCPD_RET_OK;
         }

         if ( grp->ex_sources )
         {
            bExclude = MCPD_TRUE;
            for(src = grp->ex_sources; 
                ((NULL != src) && (exSourcesIdx < MCPD_MAX_UPSTREAM_SSM_SRS)); 
                src = (t_MCPD_SRC_OBJ *)src->next, exSourcesIdx++) 
            {
               pExSources[exSourcesIdx].s_addr = *(UINT32 *)(src->addr);
            }
         }

         if ( grp->in_sources )
         {
            bInclude = MCPD_TRUE;
            for(src = grp->in_sources; 
                ((NULL != src) && (inSourcesIdx < MCPD_MAX_UPSTREAM_SSM_SRS)); 
                src = (t_MCPD_SRC_OBJ *)src->next, inSourcesIdx++) 
            {
               pInSources[inSourcesIdx].s_addr = *(UINT32 *)(src->addr);
            }

         }
      }
   }

   if ( MCPD_TRUE == bExclude )
   {
      /* all exclude sources minus the include sources */
      for(i = 0; i < MCPD_MAX_UPSTREAM_SSM_SRS; i++) 
      {
         for(k = 0; k < MCPD_MAX_UPSTREAM_SSM_SRS; k++)  
         {
            if((pExSources[i].s_addr) && (pExSources[i].s_addr == pInSources[k].s_addr))
               pExSources[i].s_addr = 0;
         }
      }

      for(i = 0; i < MCPD_MAX_UPSTREAM_SSM_SRS; i++) 
      {
         if(pExSources[i].s_addr)
         {
            ssm_info->sources[ssm_info->numsources].s_addr = pExSources[i].s_addr;
            ssm_info->numsources++;
         }
      }
      
      ssm_info->fmode = MCPD_FMODE_EXCLUDE;
   }
   else if (MCPD_TRUE == bInclude )
   {
      /* all include sources */
      for(i = 0; i < MCPD_MAX_UPSTREAM_SSM_SRS; i++) 
      {
         if(pInSources[i].s_addr)
         {
            ssm_info->sources[ssm_info->numsources].s_addr = pInSources[i].s_addr;
            ssm_info->numsources++;
         }
      } 
      ssm_info->fmode = MCPD_FMODE_INCLUDE;
   }
   else
   {
      /* no sources - should leave the group */
      ssm_info->fmode = MCPD_FMODE_INCLUDE;
      ssm_info->numsources = 0;
   }

   if ( ssm_info->numsources )
   {
      mcpd_igmp_order_upstream_ssm_source_list(ssm_info->numsources, 
                                               ssm_info->sources);
   }

   free(pExSources);
   free(pInSources);

   return MCPD_RET_OK;
   
} /* mcpd_igmp_update_upstream_ssm */

/*
 * set the source list and the source filter
 * on upstream interface
 */
t_MCPD_RET_CODE mcpd_igmp_krnl_update_ssm_filters(t_MCPD_INTERFACE_OBJ *ifp1,
                                                  t_MCPD_GROUP_OBJ *gp,
                                                  UINT8 *ptr_ssm_info,
                                                  t_MCPD_PKT_INFO *pkt_info)
{
   struct group_filter *gf = NULL;
   struct sockaddr_in *grp_addr = NULL;
   struct sockaddr_in *pSrcList = NULL;
   unsigned int size = 0;
   t_MCPD_IGMP_UPSTREAM_SSM const *ssm_info =(t_MCPD_IGMP_UPSTREAM_SSM const *)ptr_ssm_info;
   t_MCPD_INTERFACE_OBJ *ifp;

   if(!ssm_info || !gp || !pkt_info)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "invalid pointer.");
      return MCPD_RET_OK;
   }

   if ((ssm_info->fmode != MCPD_FMODE_INCLUDE) && 
       (ssm_info->fmode != MCPD_FMODE_EXCLUDE)) 
   {
      MCPD_TRACE(MCPD_TRC_LOG, "Wrong filter mode.");
      return MCPD_RET_GENERR;
   }

   size = GROUP_FILTER_SIZE(ssm_info->numsources);
   gf = (struct group_filter *)malloc(size);
   if(NULL == gf)
   {
      MCPD_TRACE(MCPD_TRC_ERR, "Not enough memory to allocate group filter.");
      return MCPD_RET_GENERR;
   }

   for (ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
   {
      if ((ifp->if_dir == MCPD_UPSTREAM) &&
          (ifp->if_type & MCPD_IF_TYPE_ROUTED) &&
          (ifp->proto_enable & MCPD_IGMP_PROXY_ENABLE)&&
          (mcpd_is_wan_service_associated_with_bridge (ifp, ifp1) == MCPD_TRUE ) )
      {
         int msFilterFetchRet = 0;
         int i;

         grp_addr = (struct sockaddr_in *) &gf->gf_group;
         memcpy(&grp_addr->sin_addr, gp->addr, sizeof(struct in_addr));
         grp_addr->sin_family = AF_INET;
         gf->gf_fmode = 0;
         gf->gf_numsrc = ssm_info->numsources;
         gf->gf_interface = ifp->if_index;
         msFilterFetchRet = getsockopt(mcpd_router.sock_igmp, 
                                       IPPROTO_IP, 
                                       MCAST_MSFILTER, 
                                       gf, 
                                       &size);

         if (msFilterFetchRet < 0 ) 
         {
             if (EADDRNOTAVAIL != errno) {
                MCPD_TRACE(MCPD_TRC_LOG, "getsockopt MCAST_MSFILTER err %d: %s",
                            errno, strerror(errno));
                free(gf);
                return MCPD_RET_GENERR;
             }

             /* this is a new group for this interface, so add it */
             if (ssm_info->fmode == MCPD_FMODE_EXCLUDE)
             {
               struct ip_mreqn mreq;
               mreq.imr_multiaddr.s_addr = grp_addr->sin_addr.s_addr;
               mreq.imr_address.s_addr = 0; /* using if index */
               mreq.imr_ifindex = ifp->if_index;
      
               if(setsockopt(mcpd_router.sock_igmp, 
                             IPPROTO_IP, 
                             IP_ADD_MEMBERSHIP,
                             (void *) &mreq, 
                             sizeof(mreq)) < 0) 
               { 
                  MCPD_TRACE(MCPD_TRC_LOG, "IP_ADD_MEMBERSHIP err %d: %s", 
                             errno, strerror(errno));
               }
               if (ssm_info->numsources == 0)
               {
                  /* Without any sources, there is no need to go through MCAST_MSFILTER */
                  continue;
               }
             } 
             else if (ssm_info->numsources > 0)
             {
               struct group_source_req mreq_src;
               struct sockaddr_in *group;
               struct sockaddr_in *source;

               mreq_src.gsr_interface = ifp->if_index;

               group=(struct sockaddr_in*)&mreq_src.gsr_group;
               group->sin_family = AF_INET;
               group->sin_port   = 0;
               memcpy(&group->sin_addr, gp->addr, sizeof(struct in_addr));

               source=(struct sockaddr_in*)&mreq_src.gsr_source;
               source->sin_family = AF_INET;
               source->sin_port   = 0;
               memcpy(&source->sin_addr, &ssm_info->sources[0].s_addr, sizeof(struct in_addr));
               if(setsockopt(mcpd_router.sock_igmp,
                             IPPROTO_IP,
                             MCAST_JOIN_SOURCE_GROUP,
                             (void *)&mreq_src,
                             sizeof(mreq_src)) < 0)
               { 
                  MCPD_TRACE(MCPD_TRC_LOG, "MCAST_JOIN_SOURCE_GROUP err %d: %s",
                             errno, strerror(errno));
               }
               if (ssm_info->numsources == 1)
               {
                  /* With only one source, there is no need to go through MCAST_MSFILTER */
                  continue;
               }
             }
             else
             {
               /* The group doesn't exist, and the report was INCLUDE(NONE) (i.e. a Leave) */
               continue;
             }
         }

         /* By this point, the group exists and an MCAST_MSFILTER call is needed */
         size = GROUP_FILTER_SIZE(ssm_info->numsources);
         bzero(gf, size);

         grp_addr = (struct sockaddr_in *) &gf->gf_group;
         memcpy(&grp_addr->sin_addr, gp->addr, sizeof(struct in_addr));
         grp_addr->sin_family = AF_INET;
         gf->gf_interface = ifp->if_index;
         gf->gf_fmode     = ssm_info->fmode;
         gf->gf_numsrc    = ssm_info->numsources;

         for(i = 0; i < ssm_info->numsources; i++) 
         {
            pSrcList = (struct sockaddr_in *)&gf->gf_slist[i];
            memcpy(&pSrcList->sin_addr, &ssm_info->sources[i], sizeof(struct in_addr));
         }

         if(setsockopt(mcpd_router.sock_igmp,
                       IPPROTO_IP, 
                       MCAST_MSFILTER, 
                       gf,
                       size) < 0 ) 
         {
            MCPD_TRACE(MCPD_TRC_LOG, "setsockopt MCAST_MSFILTER err %d: %s",
                       errno, strerror(errno)); 
         }         
      }
   }

   free(gf);

   return MCPD_RET_OK;
} /*mcpd_igmp_krnl_update_ssm_filters */

t_MCPD_RET_CODE mcpd_igmp_krnl_drop_membership(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_GROUP_OBJ *gp)
{
   struct ip_mreqn       mreq;
   t_MCPD_INTERFACE_OBJ *ifi;
   struct in_addr       *grp_addr = NULL;
   int                   leaveGroup = 1;

   if (NULL == gp)
   {
       return MCPD_RET_GENERR;
   }

   grp_addr = (struct in_addr *)(gp->addr);

    /* NULL ifp means leave the group if there are no downstream memberships 
       When ifp is set, then only leave group for that upstream */
   if ( NULL == ifp )
   {
      /* look through downstream interfaces to determine if we really need
         the upstream interfaces to leave the group */
      for(ifi = mcpd_router.interfaces; ifi; ifi = ifi->next)
      {
         if ((ifi != gp->ifp) && (ifi->if_dir == MCPD_DOWNSTREAM))
         {
            if(mcpd_interface_group_lookup(MCPD_PROTO_IGMP, ifi, gp->addr))
            {
               leaveGroup = 0;
            }
         }
      }
   }

   if ( leaveGroup )
   {
      /* tell kernel to leave the group */
      for (ifi = mcpd_router.interfaces; ifi; ifi = ifi->next)
      {
         if ((ifi->if_dir == MCPD_UPSTREAM) &&
             (ifi->if_type & MCPD_IF_TYPE_ROUTED) &&
             ((NULL == ifp) || (ifi == ifp)))
         {
            mreq.imr_multiaddr.s_addr = grp_addr->s_addr;
            mreq.imr_address.s_addr = 0; /* use ifindex to identify the interface */
            mreq.imr_ifindex = ifi->if_index;

            if(MCPD_TRUE == mcpd_is_valid_addr(MCPD_PROTO_IGMP,
                                               (UINT8 *)&mreq.imr_multiaddr))
            {
               if(setsockopt(mcpd_router.sock_igmp, IPPROTO_IP,
                          IP_DROP_MEMBERSHIP, (void *) &mreq, sizeof(mreq)) < 0)
               {
                  MCPD_TRACE(MCPD_TRC_LOG, "err - IP_DROP_MEMBERSHIP");
               }
            }
         }
      }
   }

   return MCPD_RET_OK;
}


