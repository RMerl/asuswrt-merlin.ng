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
 * File Name  : mld_proxy.c
 *
 * Description: API for MLD proxy processing
 *              
 ***************************************************************************/
#ifdef SUPPORT_MLD 
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mcpd.h"
#include "common.h"
#include "mld.h"
#include "mld_main.h"
#include "mld_proxy.h"
#include "obj_hndlr.h"

extern t_MCPD_ROUTER mcpd_router;

t_MCPD_RET_CODE mcpd_mld_proxy_init(void)
{

   if(MCPD_RET_OK != mcpd_mld_init())
   {
      MCPD_TRACE(MCPD_TRC_ERR, "Error mcpd_mld_init");
      return MCPD_RET_GENERR;
   }

   return MCPD_RET_OK;
} /* mcpd_mld_proxy_init */

void mcpd_mld_order_upstream_ssm_source_list(int nsources, 
                                             struct in6_addr *sources)
{
   int i,j;
   struct in6_addr sr[MCPD_MAX_UPSTREAM_SSM_SRS];

   for(i = 0; i < MCPD_MAX_UPSTREAM_SSM_SRS; i++) 
   {
      memcpy(&sr[i], &sources[i], sizeof(struct in6_addr));
      bzero(&sources[i], sizeof(struct in6_addr));
   }

   j = 0;
   for(i=0; ((i < MCPD_MAX_UPSTREAM_SSM_SRS) && (j < nsources)); i++) 
   {
      if (sr[i].s6_addr32[0] != 0) 
      {
         memcpy(&sources[i], &sr[i], sizeof(struct in6_addr));
         j++;
      }
   }

   return;
} /* mcpd_mld_order_upstream_ssm_source_list */

t_MCPD_RET_CODE mcpd_mld_update_upstream_ssm(t_MCPD_GROUP_OBJ *group, 
                                             UINT8 *ptr_ssm_info)
{

   int i;
   int k;
   t_MCPD_SRC_OBJ *src = NULL;
   struct in6_addr saddr;
   t_MCPD_INTERFACE_OBJ *ifi;
   t_MCPD_GROUP_OBJ *grp;
   int bInclude = MCPD_FALSE;
   int bExclude = MCPD_FALSE;

   struct in6_addr * pExSources;
   int exSourcesIdx;
   struct in6_addr * pInSources;
   int inSourcesIdx ;

   t_MCPD_MLD_UPSTREAM_SSM * ssm_info = (t_MCPD_MLD_UPSTREAM_SSM *)ptr_ssm_info;
   if(!ptr_ssm_info)
   {
      return MCPD_RET_MEMERR;
   }
   
   pExSources = (struct in6_addr *)malloc(sizeof(struct in6_addr) * MCPD_MAX_UPSTREAM_SSM_SRS);
   if(!pExSources)
   {
      return MCPD_RET_MEMERR;
   }
   
   pInSources = (struct in6_addr *)malloc(sizeof(struct in6_addr) * MCPD_MAX_UPSTREAM_SSM_SRS);
   if(!pInSources)
   {
      free(pExSources);
      return MCPD_RET_MEMERR;
   }

   bzero(ssm_info, sizeof(t_MCPD_MLD_UPSTREAM_SSM));
   bzero(pExSources, (sizeof(struct in6_addr) * MCPD_MAX_UPSTREAM_SSM_SRS));
   bzero(pInSources, (sizeof(struct in6_addr) * MCPD_MAX_UPSTREAM_SSM_SRS));

   /* update group information for the received interface */
   bzero(&saddr, sizeof(struct in6_addr));
   src = mcpd_group_src_lookup(MCPD_PROTO_MLD, 
                               group, 
                               (UINT8 *)&saddr, 
                               MCPD_FMODE_EXCLUDE);
   if(src)
   {
      /* update group to exclude if there is an exlcude source 
         with addr 0.0.0.0 
         this covers the case where v1 host is present */ 
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

   IN6_ASSIGN_ADDR(&ssm_info->group, group->addr);
   inSourcesIdx = 0;
   exSourcesIdx = 0;
   for(ifi = mcpd_router.interfaces; ifi; ifi = ifi->next)
   {
      if ((ifi->if_dir == MCPD_DOWNSTREAM) &&
          (ifi->if_type & MCPD_IF_TYPE_ROUTED))
      {
         grp = mcpd_interface_group_lookup(MCPD_PROTO_MLD, ifi, group->addr);
         if(!grp)
         {
            continue;
         }

         /* look for an exclude filter with exclude source address of 0.0.0.0
            this covers the v1 host present case as well */
         src = mcpd_group_src_lookup(MCPD_PROTO_MLD, 
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
               IN6_ASSIGN_ADDR(&pExSources[exSourcesIdx], src->addr);
            }
         }

         if ( grp->in_sources )
         {
            bInclude = MCPD_TRUE;
            for(src = grp->in_sources; 
                ((NULL != src) && (inSourcesIdx < MCPD_MAX_UPSTREAM_SSM_SRS)); 
                src = (t_MCPD_SRC_OBJ *)src->next, inSourcesIdx++) 
            {
               IN6_ASSIGN_ADDR(&pInSources[inSourcesIdx], src->addr);
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
            if(!IN6_ARE_ADDR_EQUAL(&saddr, &pExSources[i]) &&
                IN6_ARE_ADDR_EQUAL(&pExSources[i], &pInSources[k]))
            {
               bzero(&pExSources[i], sizeof(struct in6_addr));
            }
         }
      }

      for(i = 0; i < MCPD_MAX_UPSTREAM_SSM_SRS; i++) 
      {
         if(!IN6_ARE_ADDR_EQUAL(&saddr, &pExSources[i]))
         {
            IN6_ASSIGN_ADDR(&ssm_info->sources[ssm_info->numsources], &pExSources[i]);
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
         if(!IN6_ARE_ADDR_EQUAL(&saddr, &pInSources[i]))
         {
            IN6_ASSIGN_ADDR(&ssm_info->sources[ssm_info->numsources], &pInSources[i]);
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
      mcpd_mld_order_upstream_ssm_source_list(ssm_info->numsources, 
                                              ssm_info->sources);
   }

   free(pExSources);
   free(pInSources);

   return MCPD_RET_OK;
   
} /* mcpd_mld_update_upstream_ssm */


/*
 * set the source list and the source filter
 * on upstream interface
 */
t_MCPD_RET_CODE mcpd_mld_krnl_update_ssm_filters(t_MCPD_INTERFACE_OBJ *ifp1,
                                                 t_MCPD_GROUP_OBJ *gp,
                                                 UINT8 *ptr_ssm_info,
                                                 t_MCPD_PKT_INFO *pkt_info)
{
   struct group_filter *gf = NULL;
   struct sockaddr_in6 *grp_addr = NULL;
   struct sockaddr_in6 *pSrcList = NULL;
   unsigned int size;
   t_MCPD_MLD_UPSTREAM_SSM *ssm_info =(t_MCPD_MLD_UPSTREAM_SSM *)ptr_ssm_info;
   t_MCPD_INTERFACE_OBJ *ifp = ifp1;

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
      MCPD_TRACE(MCPD_TRC_ERR, "Not enough memory to reserve for group filter");
      return MCPD_RET_GENERR;
   }

   for (ifp = mcpd_router.interfaces; ifp; ifp = ifp->next) 
   {
      if ((ifp->if_dir == MCPD_UPSTREAM) &&
      (ifp->if_type & MCPD_IF_TYPE_ROUTED) &&
      (ifp->proto_enable & MCPD_MLD_PROXY_ENABLE) &&
      (mcpd_is_wan_service_associated_with_bridge (ifp, ifp1) == MCPD_TRUE ) )
      {
         int i;
         int msFilterFetch = 0;

         grp_addr = (struct sockaddr_in6 *) &gf->gf_group;
         memcpy(&grp_addr->sin6_addr, gp->addr, sizeof(struct in6_addr));
         grp_addr->sin6_family = AF_INET6;
         gf->gf_fmode = 0;
         gf->gf_numsrc = ssm_info->numsources;
         gf->gf_interface = ifp->if_index;
         msFilterFetch = getsockopt(mcpd_router.sock_mld, 
                                    IPPROTO_IPV6, 
                                    MCAST_MSFILTER, 
                                    gf, 
                                    &size);

         if (msFilterFetch < 0)
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
               struct ipv6_mreq mreq6;
               memcpy(&mreq6.ipv6mr_multiaddr, gp->addr, sizeof(struct in6_addr));
               mreq6.ipv6mr_interface = ifp->if_index;

               if(setsockopt(mcpd_router.sock_mld, 
                             IPPROTO_IPV6, 
                             IPV6_ADD_MEMBERSHIP,
                             (void *) &mreq6, 
                             sizeof(mreq6)) < 0) 
               { 
                  MCPD_TRACE(MCPD_TRC_LOG, "IPV6_ADD_MEMBERSHIP err %d: %s",
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
               struct sockaddr_in6 *group;
               struct sockaddr_in6 *source;
               struct group_source_req mreq6_src;

               mreq6_src.gsr_interface = ifp->if_index;

               group=(struct sockaddr_in6*)&mreq6_src.gsr_group;
               group->sin6_family = AF_INET6;
               group->sin6_port   = 0;
               memcpy(&group->sin6_addr, gp->addr, sizeof(struct in6_addr));

               source=(struct sockaddr_in6*)&mreq6_src.gsr_source;
               source->sin6_family = AF_INET6;
               source->sin6_port   = 0;
               memcpy(&source->sin6_addr, &ssm_info->sources[0].s6_addr, sizeof(struct in6_addr));
               if(setsockopt(mcpd_router.sock_mld,
                             IPPROTO_IPV6,
                             MCAST_JOIN_SOURCE_GROUP,
                             (void *)&mreq6_src,
                             sizeof(mreq6_src)) < 0)
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

         grp_addr = (struct sockaddr_in6 *) &gf->gf_group;
         memcpy(&grp_addr->sin6_addr, gp->addr, sizeof(struct in6_addr));
         grp_addr->sin6_family = AF_INET6;
         gf->gf_interface = ifp->if_index;
         gf->gf_fmode     = ssm_info->fmode;
         gf->gf_numsrc    = ssm_info->numsources;
         for(i = 0; i < ssm_info->numsources; i++)
         { 
            pSrcList = (struct sockaddr_in6 *)&gf->gf_slist[i];
            memcpy(&pSrcList->sin6_addr, &ssm_info->sources[i], sizeof(struct in6_addr));
         }

         if(setsockopt(mcpd_router.sock_mld, 
                       IPPROTO_IPV6, 
                       MCAST_MSFILTER, 
                       gf, 
                       size) < 0)
         {
            MCPD_TRACE(MCPD_TRC_LOG, "setsockopt MCAST_MSFILTER err %d: %s", 
                       errno, strerror(errno));
         }
      }
   }

   free(gf);

   return MCPD_RET_OK;
} /* mcpd_mld_krnl_update_ssm_filters */

t_MCPD_RET_CODE mcpd_mld_krnl_drop_membership(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_GROUP_OBJ *gp)
{
    struct ipv6_mreq mreq;
    t_MCPD_INTERFACE_OBJ *ifi;
    int leaveGroup = 1;

    if (NULL == gp)
    {
        return MCPD_RET_GENERR;
    }
    
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
                if(mcpd_interface_group_lookup(MCPD_PROTO_MLD, ifi, gp->addr))
                {
                    leaveGroup = 0;
                }
            }
        }
    }
    
    if ( leaveGroup )
    {
        /* Tell all upstream interfaces that we are gone */
        for (ifi = mcpd_router.interfaces; ifi; ifi = ifi->next) 
        {
            if ((ifi->if_dir == MCPD_UPSTREAM) &&
                (ifi->if_type & MCPD_IF_TYPE_ROUTED) &&
                ((NULL == ifp) || (ifi == ifp))) 
            {
                memcpy(&mreq.ipv6mr_multiaddr, 
                       gp->addr, 
                       sizeof(struct in6_addr));
                mreq.ipv6mr_interface = ifi->if_index;
                if(MCPD_TRUE == mcpd_is_valid_addr(MCPD_PROTO_MLD, 
                                                (UINT8 *)&mreq.ipv6mr_multiaddr))
                {
                    if(setsockopt(mcpd_router.sock_mld, 
                             IPPROTO_IPV6, 
                             IPV6_DROP_MEMBERSHIP, 
                             (void *)&mreq, 
                             sizeof(mreq)) < 0)
                    { 
                        MCPD_TRACE(MCPD_TRC_LOG, "err - IPV6_DROP_MEMBERSHIP");
                    }
                } 
            } 
        }
    }

   return MCPD_RET_OK;
   
}
#endif /* SUPPORT_MLD */
