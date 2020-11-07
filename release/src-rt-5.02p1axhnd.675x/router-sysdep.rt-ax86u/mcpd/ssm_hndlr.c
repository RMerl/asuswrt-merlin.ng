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
 * File Name  : ssm_hndlr.c
 *
 * Description: API for Source Specific Multicast filter processing
 *              
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mcpd.h"
#include "common.h"
#include "mcpd_nl.h"
#include "igmp.h"
#include "igmp_proxy.h"
#include "mld.h"
#include "obj_hndlr.h"
#include "ssm_hndlr.h"
#include "igmp_main.h"
#include "mld_main.h"
#include "mcpd_timer.h"

extern t_MCPD_ROUTER mcpd_router;

struct srcList
{
   UINT8 src_action;
   struct in_addr  srcAddr4;
   struct in6_addr srcAddr6;
};

/***************************************************************************
 * Function Name: mcpd_ssm_post_process
 *
 * Description  : handle post process issues
 *
 * Returns      : None
 ***************************************************************************/
t_MCPD_RET_CODE mcpd_ssm_post_process(t_MCPD_PROTO_TYPE     proto,
                                      t_MCPD_INTERFACE_OBJ *ifp,
                                      t_MCPD_GROUP_OBJ     *gp,
                                      t_MCPD_PKT_INFO      *pkt_info,
                                      int                   numsrc,
                                      struct srcList       *snSrcList,
                                      t_MCPD_REP_OBJ       *rep )
{
   int                   i = 0;
   UINT8                *src_elm = NULL;
   int                   action = 0;

   for(i = 0; i < numsrc; i++) 
   {
      if(proto == MCPD_PROTO_IGMP)
      {
         src_elm = (UINT8 *)&snSrcList[i].srcAddr4.s_addr;
         MCPD_TRACE(MCPD_TRC_INFO, "found IP4 src %x, action %x", 
                    *(UINT32 *)src_elm, snSrcList[i].src_action); 
      }
      else
      {
         src_elm = (UINT8 *)&snSrcList[i].srcAddr6.s6_addr[0];
         MCPD_TRACE(MCPD_TRC_INFO, "found IP6 src %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, action %x", 
                    snSrcList[i].srcAddr6.s6_addr16[0], snSrcList[i].srcAddr6.s6_addr16[1],
                    snSrcList[i].srcAddr6.s6_addr16[2], snSrcList[i].srcAddr6.s6_addr16[3],
                    snSrcList[i].srcAddr6.s6_addr16[4], snSrcList[i].srcAddr6.s6_addr16[5],
                    snSrcList[i].srcAddr6.s6_addr16[6], snSrcList[i].srcAddr6.s6_addr16[7],
                 snSrcList[i].src_action);
      }

      /* process snooping action */
      action = snSrcList[i].src_action & 0x0F;
      if(action != 0)
      {
         if (((proto == MCPD_PROTO_IGMP) && (mcpd_router.igmp_config.flood_enable))
#ifdef SUPPORT_MLD
            || ((proto == MCPD_PROTO_MLD) && (mcpd_router.mld_config.flood_enable))
#endif
            )
         {
			 MCPD_TRACE(MCPD_TRC_INFO, "Flooding case");
             mcpd_router.update_flooding_info_func[proto](ifp, gp, rep, src_elm, action, pkt_info, FLOOD_TYPE_ALL);
         }
         else
         {
		     MCPD_TRACE(MCPD_TRC_INFO, "Snooping case");
             mcpd_router.update_snooping_info_func[proto](ifp, gp, rep, src_elm, action, pkt_info);
         }         
      }

      /* process proxy action */
      action = snSrcList[i].src_action & 0xF0;
      if((action) && (ifp->if_type & MCPD_IF_TYPE_ROUTED))
      {
         mcpd_router.chg_mfc_func[proto](src_elm, gp->addr);
      }
   }
   return MCPD_RET_OK;
} /* mcpd_ssm_post_process */

static t_MCPD_RET_CODE mcpd_ssm_handle_ssm_info(t_MCPD_PROTO_TYPE    proto, 
                                                t_MCPD_GROUP_OBJ     *gp,
                                                t_MCPD_INTERFACE_OBJ *ifp,
                                                t_MCPD_PKT_INFO      *pkt_info)
{
   UINT8 *ssm_info = NULL;

   if (!gp || !ifp || !pkt_info)
   {
      return MCPD_RET_GENERR;
   }
      
   if(proto == MCPD_PROTO_IGMP)
   {
      ssm_info = (UINT8*)malloc(sizeof(t_MCPD_IGMP_UPSTREAM_SSM));
   }
   else
   {
#ifdef SUPPORT_MLD
      ssm_info = (UINT8*)malloc(sizeof(t_MCPD_MLD_UPSTREAM_SSM));
#else
      return MCPD_RET_GENERR;
#endif      
   }

   if (ssm_info == NULL)
   {
      return MCPD_RET_MEMERR;
   }

   if (mcpd_router.update_upstream_ssm_func[proto](gp, ssm_info) == MCPD_RET_OK)
   {
      mcpd_router.krnl_update_ssm_filters_func[proto](ifp, 
                                                      gp, 
                                                      ssm_info,
                                                      pkt_info);
   }
   free (ssm_info);

   return MCPD_RET_OK;
}



/***************************************************************************
 * Function Name: mcpd_ssm_process_allow_newsrc_filter
 *
 * Description  : handle allow new source filter
 *
 * Returns      : None
 ***************************************************************************/
t_MCPD_RET_CODE mcpd_ssm_process_allow_newsrc_filter(
                                          t_MCPD_PROTO_TYPE     proto,
                                          t_MCPD_INTERFACE_OBJ *ifp,
                                          t_MCPD_GROUP_OBJ     *gp,
                                          int                   num_src,
                                          UINT8                *sources,
                                          t_MCPD_REP_OBJ       *rep,
                                          t_MCPD_PKT_INFO      *pkt_info)
{
   t_MCPD_SRC_OBJ     *src = NULL;
   t_MCPD_SRC_REP_OBJ *srep = NULL;
   int                 i;
   UINT8              *src_elm = NULL;
   struct in_addr     *source_list  = (struct in_addr *)sources;
   struct in6_addr    *source_list6 = (struct in6_addr *)sources;
   t_MCPD_RET_CODE     ret = MCPD_RET_GENERR;
   int                 snSrcCnt = 0;
   struct srcList      snSrcList[MCPD_MAX_UPSTREAM_SSM_SRS];
   int                 adminType;
   int                 fmode = 0xFF;

   if(num_src >= MCPD_MAX_UPSTREAM_SSM_SRS)
   {
       MCPD_TRACE(MCPD_TRC_ERR, "not enough array elements");
       return MCPD_RET_DROP;
   }
   memset(snSrcList, 0, sizeof(struct srcList) * MCPD_MAX_UPSTREAM_SSM_SRS);

   /* Reset timer */
   mcpd_router.update_rep_tmr_func[proto](rep);

   /* no sources or include sources - set reporter to include mode */
   fmode = MCPD_FMODE_INCLUDE;
   for(src = gp->ex_sources; src != NULL;
             src = (t_MCPD_SRC_OBJ *)src->next)
   {
      srep = mcpd_group_src_rep_lookup(proto, src, rep);
      if ( srep )
      {
         /* found an exclude source */
         fmode = MCPD_FMODE_EXCLUDE;
      }
   }

   if ( MCPD_FMODE_EXCLUDE == fmode )
   {
      /* allow new sources means remove excluded sources for exclude mode */
      for(i = 0; i < num_src; i++) 
      {
         if(proto == MCPD_PROTO_IGMP)
         {
            src_elm = (UINT8 *) &source_list[i];
         }
         else
         {
            src_elm = (UINT8 *) &source_list6[i];
         }
         src = mcpd_group_src_lookup(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     fmode);
         if ( src )
         {
            srep = mcpd_group_src_rep_lookup(proto, src, rep);
            if ( srep )
            {
               /* found source with matching reporter */
               snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_EX_CLEAR;
               if(proto == MCPD_PROTO_IGMP)
               {
                  snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
               }
               else
               {
                  IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
               }

               /* clean up reporter source information */
               if (MCPD_RET_OK == mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                            pkt_info->rxdev_ifi,
                                                                            gp->addr,
                                                                            src->addr,
                                                                            rep->addr,
                                                                            pkt_info->tci,
                                                                            rep->version)) {
                  ret = MCPD_RET_OK;
               }

               mcpd_src_rep_cleanup(proto, src, rep);
               mcpd_rep_src_cleanup(proto, rep, src);
               if(0 == mcpd_src_rep_count(proto, src))
               {
                  /* no more reporters for this source send a query */
                  mcpd_router.membership_query_func[proto](ifp,
                                                           gp,
                                                           src->addr,
                                                           1);
                  snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
               }
               snSrcCnt++;
            }
         }
      }
   }
   else
   {
      /* allow new source for include mode means adding new sources */
      for(i = 0; i < num_src; i++) 
      {                  
         if(proto == MCPD_PROTO_IGMP)
         {
            src_elm = (UINT8 *) &source_list[i];
         }
         else
         {
            src_elm = (UINT8 *) &source_list6[i];
         }
         src = mcpd_group_src_lookup(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     fmode);
         if(src)
         {
            /* source already present, check to see if this is a join or
               a rejoin */
            srep = mcpd_group_src_rep_lookup(proto, src, rep);
            if ( NULL == srep )
            {
               adminType = MCPD_ADMISSION_JOIN;
            }
            else
            {
               adminType = MCPD_ADMISSION_RE_JOIN;
            }
         }
         else
         {
            /* new source - create it */
            src = mcpd_group_src_add(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     MCPD_FMODE_INCLUDE);
            if ( NULL == src )
            {
               return MCPD_RET_MEMERR;
            }
            adminType = MCPD_ADMISSION_JOIN;
         }
  
         if( MCPD_RET_OK != mcpd_router.admission_control_func[proto](adminType,
                                                                      pkt_info->rxdev_ifi,
                                                                      gp->addr,
                                                                      src->addr,
                                                                      rep->addr,
                                                                      pkt_info->tci,
                                                                      rep->version) )
         {
            continue;
         }
         ret = MCPD_RET_OK;
         
         /* add repoter to source and source to reporter */
         mcpd_group_src_rep_add(proto, src, rep);
         mcpd_group_rep_src_add(proto, rep, src);
         snSrcList[snSrcCnt].src_action = (MCPD_SNOOP_IN_ADD | MCPD_MFC_ADD);
         if(proto == MCPD_PROTO_IGMP)
         {
            snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
         }
         else
         {
            IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
         }
        
         mcpd_router.update_src_tmr_func[proto](src);
         snSrcCnt++;
      }
   }

   if (MCPD_RET_OK == ret)
   {
       ret = mcpd_ssm_post_process(proto, ifp, gp, 
                                   pkt_info, snSrcCnt, &snSrcList[0],
                                   rep);

       /* clean up reporter and source objects
          rep should not be used beyond this point
          this call has to be made before mcpd_ssm_handle_ssm_info
          which relies on the removal of unused source objects */
       mcpd_update_group_src_reporters(proto, gp);

       if(ifp->if_type & MCPD_IF_TYPE_ROUTED) 
       {
          mcpd_ssm_handle_ssm_info(proto, gp, ifp, pkt_info);
       }
   }

   return ret;
} /* mcpd_ssm_process_allow_newsrc_filter */


/***************************************************************************
 * Function Name: mcpd_ssm_process_block_oldsrc_filter
 *
 * Description  : handle block old source filters
 *
 * Returns      : None
 ***************************************************************************/
t_MCPD_RET_CODE mcpd_ssm_process_block_oldsrc_filter(
                                          t_MCPD_PROTO_TYPE     proto,
                                          t_MCPD_INTERFACE_OBJ *ifp,
                                          t_MCPD_GROUP_OBJ     *gp,
                                          int                   num_src,
                                          UINT8                *sources,
                                          t_MCPD_REP_OBJ       *rep,
                                          t_MCPD_PKT_INFO      *pkt_info)
{
   t_MCPD_SRC_OBJ     *src = NULL;
   t_MCPD_SRC_REP_OBJ *srep = NULL;
   int                 i;
   UINT8              *src_elm = NULL;
   struct in_addr     *source_list  = (struct in_addr *)sources;
   struct in6_addr    *source_list6 = (struct in6_addr *)sources;
   t_MCPD_RET_CODE     ret = MCPD_RET_GENERR;
   int                 snSrcCnt = 0;
   struct srcList      snSrcList[MCPD_MAX_UPSTREAM_SSM_SRS];
   int                 adminType;
   int                 fmode;

   if(num_src >= MCPD_MAX_UPSTREAM_SSM_SRS)
   {
       MCPD_TRACE(MCPD_TRC_ERR, "not enough array elements");
       return MCPD_RET_DROP;
   }
   memset(snSrcList, 0, sizeof(struct srcList) * MCPD_MAX_UPSTREAM_SSM_SRS);

   /* Reset timer */
   mcpd_router.update_rep_tmr_func[proto](rep);

   /* no sources or include sources - set reporter to include mode */
   fmode = MCPD_FMODE_INCLUDE;
   for(src = gp->ex_sources; src != NULL;
             src = (t_MCPD_SRC_OBJ *)src->next)
   {
      srep = mcpd_group_src_rep_lookup(proto, src, rep);
      if ( srep )
      {
         /* found an exclude source */
         fmode = MCPD_FMODE_EXCLUDE;
      }
   }

   if ( MCPD_FMODE_INCLUDE == fmode )
   {
      /* block old source means remove source for include mode */
      for(i = 0; i < num_src; i++) 
      {
         if(proto == MCPD_PROTO_IGMP)
         {
            src_elm = (UINT8 *) &source_list[i];
         }
         else
         {
            src_elm = (UINT8 *) &source_list6[i];
         }
         src = mcpd_group_src_lookup(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     fmode);
         if ( src )
         {
            srep = mcpd_group_src_rep_lookup(proto, src, rep);
            if ( srep )
            {
               /* found source with matching reporter */
               snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_IN_CLEAR;
               if(proto == MCPD_PROTO_IGMP)
               {
                  snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
               }
               else
               {
                  IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
               }

               /* clean up reporter source information */
               if (MCPD_RET_OK == mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                            pkt_info->rxdev_ifi,
                                                                            gp->addr,
                                                                            src->addr,
                                                                            rep->addr,
                                                                            pkt_info->tci,
                                                                            rep->version))
               {
                  ret = MCPD_RET_OK;
               }

               mcpd_src_rep_cleanup(proto, src, rep);
               mcpd_rep_src_cleanup(proto, rep, src);
               if(0 == mcpd_src_rep_count(proto, src))
               {
                  /* no more reporters for this source send a query */
                  mcpd_router.membership_query_func[proto](ifp,
                                                           gp,
                                                           src->addr,
                                                           1);
                  snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
               }
               snSrcCnt++;
            }
         }
      }
   }
   else
   {
      /* block old soruce means add new source for exclude mode */
      for(i = 0; i < num_src; i++) 
      {
         if(proto == MCPD_PROTO_IGMP)
         {
            src_elm = (UINT8 *) &source_list[i];
         }
         else
         {
            src_elm = (UINT8 *) &source_list6[i];
         }
         src = mcpd_group_src_lookup(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     fmode);
         if(src)
         {
            /* found existing source - determine join type */
            srep = mcpd_group_src_rep_lookup(proto, src, rep);
            if ( NULL == srep )
            {
               adminType = MCPD_ADMISSION_JOIN;
            }
            else
            {
               adminType = MCPD_ADMISSION_RE_JOIN;
            }
         }
         else
         {
            /* new source - create it */
            src = mcpd_group_src_add(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     MCPD_FMODE_EXCLUDE);
            if(NULL == src)
            {
               return MCPD_RET_MEMERR;
            }
            adminType = MCPD_ADMISSION_JOIN;
         }

         if (MCPD_RET_OK != mcpd_router.admission_control_func[proto](adminType,
                                                                      pkt_info->rxdev_ifi,
                                                                      gp->addr,
                                                                      src->addr,
                                                                      rep->addr,
                                                                      pkt_info->tci,
                                                                      rep->version) )
         {
            continue;
         }
         ret = MCPD_RET_OK;

         mcpd_group_src_rep_add(proto, src, rep);
         mcpd_group_rep_src_add(proto, rep, src);
         snSrcList[snSrcCnt].src_action = (MCPD_SNOOP_EX_ADD | MCPD_MFC_ADD);
         if(proto == MCPD_PROTO_IGMP)
         {
            snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
         }
         else
         {
            IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
         }
         
         mcpd_router.update_src_tmr_func[proto](src);
         snSrcCnt++;
      }
   }

   if (MCPD_RET_OK == ret)
   {
      ret = mcpd_ssm_post_process(proto, ifp, gp, 
                                  pkt_info, snSrcCnt, &snSrcList[0],
                                  rep);

      /* clean up reporter and source objects
         rep should not be used beyond this point
         this call has to be made before mcpd_ssm_handle_ssm_info
         which relies on the removal of unused source objects */
      mcpd_update_group_src_reporters(proto, gp);
 
      if(ifp->if_type & MCPD_IF_TYPE_ROUTED) 
      {
         mcpd_ssm_handle_ssm_info(proto, gp, ifp, pkt_info);
      }
   }
   return ret;
} /* mcpd_ssm_process_block_oldsrc_filter */


/***************************************************************************
 * Function Name: mcpd_ssm_process_toex_filter
 *
 * Description  : handle TO_EXCLUDE filters
 *
 * Returns      : None
 ***************************************************************************/
t_MCPD_RET_CODE mcpd_ssm_process_ex_filter(t_MCPD_PROTO_TYPE     proto,
                                           t_MCPD_INTERFACE_OBJ *ifp,
                                           t_MCPD_GROUP_OBJ     *gp,
                                           int                   num_src,
                                           UINT8                *sources,
                                           t_MCPD_REP_OBJ       *rep,
                                           t_MCPD_PKT_INFO      *pkt_info)
{
   t_MCPD_SRC_OBJ     *src = NULL;
   t_MCPD_SRC_REP_OBJ *srep = NULL;
   int                 i;
   UINT8              *src_elm = NULL;
   struct in_addr     *source_list  = (struct in_addr *)sources;
   struct in6_addr    *source_list6 = (struct in6_addr *)sources;
   t_MCPD_RET_CODE     ret = MCPD_RET_GENERR;
   int                 snSrcCnt = 0;
   struct srcList      snSrcList[MCPD_MAX_UPSTREAM_SSM_SRS];
   int                 adminType;
   struct in_addr      saddr;
   struct in6_addr     saddr6;
   
   if(num_src >= MCPD_MAX_UPSTREAM_SSM_SRS)
   {
       MCPD_TRACE(MCPD_TRC_ERR, "not enough array elements");
       return MCPD_RET_DROP;
   }

   memset(snSrcList, 0, sizeof(struct srcList) * MCPD_MAX_UPSTREAM_SSM_SRS);

   /* Reset timer */
   mcpd_router.update_rep_tmr_func[proto](rep);
           
   if ( 0 == num_src )
   {
      /* exclude - no source - join all sources 
         go through all the sources and find anything that matches
         this reporter */
      for(src = gp->in_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            /* found source with matching reporter */
            snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_IN_CLEAR;
            if(proto == MCPD_PROTO_IGMP)
            {
               snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
            }
            else
            {
               IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
            }

            /* clean up reporter source information */
            if ( MCPD_RET_OK ==  mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                           pkt_info->rxdev_ifi,
                                                                           gp->addr,
                                                                           src->addr,
                                                                           rep->addr,
                                                                           pkt_info->tci,
                                                                           rep->version) )
            {
               ret = MCPD_RET_OK;
            }

            
            mcpd_src_rep_cleanup(proto, src, rep);
            mcpd_rep_src_cleanup(proto, rep, src);
            if(0 == mcpd_src_rep_count(proto, src))
            {
               /* no more reporters for this source send a query */
               mcpd_router.membership_query_func[proto](ifp,
                                                        gp,
                                                        src->addr,
                                                        1);
               snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
            }
            snSrcCnt++;
         }
      }

      for(src = gp->ex_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            /* found source with matching reporter */

            /* skip exclude no source */
            if(proto == MCPD_PROTO_IGMP)
            {
               saddr.s_addr = 0;
               src_elm = (UINT8 *)&saddr;
            }
            else
            {
               memset(&saddr6, 0, sizeof(struct in6_addr));
               src_elm = (UINT8 *)&saddr6;
            }
            if (MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](src_elm, src->addr))
            {
               continue;
            }
            
            snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_EX_CLEAR;
            if(proto == MCPD_PROTO_IGMP)
            {
               snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
            }
            else
            {
               IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
            }

            if ( MCPD_RET_OK == mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                          pkt_info->rxdev_ifi,
                                                                          gp->addr,
                                                                          src->addr,
                                                                          rep->addr,
                                                                          pkt_info->tci,
                                                                          rep->version) )
            {
               ret = MCPD_RET_OK;
            }

            /* clean up reporter source information */
            mcpd_src_rep_cleanup(proto, src, rep);
            mcpd_rep_src_cleanup(proto, rep, src);
            if(0 == mcpd_src_rep_count(proto, src)) 
            {
               /* no more reporters for this source send a query */
               mcpd_router.membership_query_func[proto](ifp,
                                                        gp,
                                                        src->addr,
                                                        1);
               snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
            }  
            snSrcCnt++;
         }
      }

      /* add snooping entry for exlcude no source */
      if(proto == MCPD_PROTO_IGMP)
      {
         saddr.s_addr = 0;
         src_elm = (UINT8 *)&saddr;
      }
      else
      {
         memset(&saddr6, 0, sizeof(struct in6_addr));
         src_elm = (UINT8 *)&saddr6;
      }
      
      src = mcpd_group_src_lookup(proto, 
                                  gp, 
                                  (UINT8 *)src_elm, 
                                  MCPD_FMODE_EXCLUDE);
      if(src)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( NULL == srep )
         {
            adminType = MCPD_ADMISSION_JOIN;
         }
         else
         {
            adminType = MCPD_ADMISSION_RE_JOIN;
         }
      }
      else
      {
         src = mcpd_group_src_add(proto, 
                                  gp, 
                                  (UINT8 *)src_elm, 
                                  MCPD_FMODE_EXCLUDE);
         if ( NULL == src )
         {
            return MCPD_RET_MEMERR;
         }
         adminType = MCPD_ADMISSION_JOIN;
      }

      if (MCPD_RET_OK == mcpd_router.admission_control_func[proto](adminType,
                                                                   pkt_info->rxdev_ifi,
                                                                   gp->addr,
                                                                   src->addr,
                                                                   rep->addr,
                                                                   pkt_info->tci,
                                                                   rep->version) )
      {
         ret = MCPD_RET_OK;
 
         mcpd_group_src_rep_add(proto, src, rep);
         mcpd_group_rep_src_add(proto, rep, src);
         snSrcList[snSrcCnt].src_action = (MCPD_SNOOP_EX_ADD | MCPD_MFC_ADD);
         if(proto == MCPD_PROTO_IGMP)
         {
            snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
         }
         else
         {
            IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
         }
         
         mcpd_router.update_src_tmr_func[proto](src);
         snSrcCnt++;
      }
   }
   else
   {
      /* exclude filter for reporter - remove any included sources */
      for(src = gp->in_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            /* found source with matching reporter */
            snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_IN_CLEAR;
            if(proto == MCPD_PROTO_IGMP)
            {
               snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
            }
            else
            {
               IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
            }

            if (MCPD_RET_OK != mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                         pkt_info->rxdev_ifi,
                                                                         gp->addr,
                                                                         src->addr,
                                                                         rep->addr,
                                                                         pkt_info->tci,
                                                                         rep->version) )
            {
               ret = MCPD_RET_OK;
            }

            /* clean up reporter source information */
            mcpd_src_rep_cleanup(proto, src, rep);
            mcpd_rep_src_cleanup(proto, rep, src);
            if(0 == mcpd_src_rep_count(proto, src)) 
            {
               /* no more reporters for this source send a query */
               mcpd_router.membership_query_func[proto](ifp,
                                                        gp,
                                                        src->addr,
                                                        1);
               snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
            }  
            snSrcCnt++;
         }
      }

      /* remove exclude sources that are not part of current list */
      for(src = gp->ex_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            for( i = 0; i < num_src; i++ )
            {
               if(proto == MCPD_PROTO_IGMP)
               {
                  src_elm = (UINT8 *) &source_list[i];
               }
               else
               {
                  src_elm = (UINT8 *) &source_list6[i];
               }
               
               if (MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](src->addr, src_elm) )
               {
                  
                  /* found source - update timer, continue search */
                  if(MCPD_RET_OK != mcpd_router.admission_control_func[proto](MCPD_ADMISSION_RE_JOIN,
                                                                              pkt_info->rxdev_ifi,
                                                                              gp->addr,
                                                                              src->addr,
                                                                              rep->addr,
                                                                              pkt_info->tci,
                                                                              rep->version))
                  {
                     continue;
                  }
                  ret = MCPD_RET_OK;
                  mcpd_router.update_src_tmr_func[proto](src);
                  break;
               }
            }
            if ( i == num_src )
            {
               /* did not find source/reporter combination */
               snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_EX_CLEAR;
               if(proto == MCPD_PROTO_IGMP)
               {
                  snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
               }
               else
               {
                  IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
               }

               /* clean up reporter source information */
               if (MCPD_RET_OK == mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                            pkt_info->rxdev_ifi,
                                                                            gp->addr,
                                                                            src->addr,
                                                                            rep->addr,
                                                                            pkt_info->tci,
                                                                            rep->version) )
               {
                  ret = MCPD_RET_OK;
               }
               
               mcpd_src_rep_cleanup(proto, src, rep);
               mcpd_rep_src_cleanup(proto, rep, src);
               if(0 == mcpd_src_rep_count(proto, src))
               {
                  /* no more reporters for this source send a query */
                  mcpd_router.membership_query_func[proto](ifp,
                                                           gp,
                                                           src->addr,
                                                           1);
                  snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
               }
               snSrcCnt++;
            }
         }
      }

      /* add new entries */
      for(i = 0; i < num_src; i++) 
      {
         if(proto == MCPD_PROTO_IGMP)
         {
            src_elm = (UINT8 *) &source_list[i];
         }
         else
         {
            src_elm = (UINT8 *) &source_list6[i];
         }
         src = mcpd_group_src_lookup(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     MCPD_FMODE_EXCLUDE);
         /* source does not exist for the group - create entry */
         if ( NULL == src )
         {
            src = mcpd_group_src_add(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     MCPD_FMODE_EXCLUDE);

            if(src == NULL) 
            {
               return MCPD_RET_MEMERR;
            }
         }

         /* add reporter to the soruce and create snooping info */
         if (MCPD_RET_OK != mcpd_router.admission_control_func[proto](MCPD_ADMISSION_JOIN,
                                                                      pkt_info->rxdev_ifi,
                                                                      gp->addr,
                                                                      src->addr,
                                                                      rep->addr,
                                                                      pkt_info->tci,
                                                                      rep->version) )
         {
            continue;
         }
         ret = MCPD_RET_OK;

         mcpd_group_src_rep_add(proto, src, rep);
         mcpd_group_rep_src_add(proto, rep, src);

         mcpd_router.update_src_tmr_func[proto](src);

         snSrcList[snSrcCnt].src_action = (MCPD_SNOOP_EX_ADD | MCPD_MFC_ADD);
         if(proto == MCPD_PROTO_IGMP)
         {
            snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
         }
         else
         {
            IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
         }
         snSrcCnt++;
      }
   }

   if (MCPD_RET_OK == ret) 
   {
      ret = mcpd_ssm_post_process(proto, ifp, gp, 
                                  pkt_info, snSrcCnt, &snSrcList[0],
                                  rep);

      /* clean up reporter and source objects
         rep should not be used beyond this point
         this call has to be made before mcpd_ssm_handle_ssm_info
         which relies on the removal of unused source objects */
      mcpd_update_group_src_reporters(proto, gp);
 
      if(ifp->if_type & MCPD_IF_TYPE_ROUTED) 
      {
         mcpd_ssm_handle_ssm_info(proto, gp, ifp, pkt_info);
      }
   }
   return ret;
}


/***************************************************************************
 * Function Name: mcpd_ssm_process_toin_filter
 *
 * Description  : handle TO_IN filters
 *
 * Returns      : None
 ***************************************************************************/
t_MCPD_RET_CODE mcpd_ssm_process_in_filter(t_MCPD_PROTO_TYPE     proto,
                                           t_MCPD_INTERFACE_OBJ *ifp,
                                           t_MCPD_GROUP_OBJ     *gp,
                                           int                   num_src,
                                           UINT8                *sources,
                                           t_MCPD_REP_OBJ       *rep,
                                           t_MCPD_PKT_INFO      *pkt_info)
{
   t_MCPD_SRC_OBJ     *src = NULL;
   t_MCPD_SRC_REP_OBJ *srep = NULL;
   int                 i;
   UINT8              *src_elm = NULL;
   struct in_addr     *source_list = (struct in_addr *)sources;
   struct in6_addr    *source_list6 = (struct in6_addr *)sources;
   t_MCPD_RET_CODE     ret = MCPD_RET_GENERR;
   int                 snSrcCnt = 0;
   struct srcList      snSrcList[MCPD_MAX_UPSTREAM_SSM_SRS]; 

   MCPD_TRACE(MCPD_TRC_INFO,"numsource = %d", num_src);

   if(num_src >= MCPD_MAX_UPSTREAM_SSM_SRS)
   {
       MCPD_TRACE(MCPD_TRC_ERR, "not enough array elements");
       return MCPD_RET_DROP;
   }

   memset(snSrcList, 0, sizeof(struct srcList) * MCPD_MAX_UPSTREAM_SSM_SRS);
 
   /* Reset timer */
   mcpd_router.update_rep_tmr_func[proto](rep);
           
   if ( 0 == num_src )
   {
      /* include - no source - leave group 
         go through all the sources and find anything that matches
         this reporter */
      MCPD_TRACE(MCPD_TRC_INFO,"Processing INCLUDE NONE");
      ret = MCPD_RET_OK;
      for(src = gp->in_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            /* found source with matching reporter */
            snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_IN_CLEAR;
            if(proto == MCPD_PROTO_IGMP)
            {
               snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
            }
            else
            {
               IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
            }

            /* clean up reporter source information */
            mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                      pkt_info->rxdev_ifi,
                                                      gp->addr,
                                                      src->addr,
                                                      rep->addr,
                                                      pkt_info->tci,
                                                      rep->version);

            
            mcpd_src_rep_cleanup(proto, src, rep);
            mcpd_rep_src_cleanup(proto, rep, src);
            if(0 == mcpd_src_rep_count(proto, src))
            {
               /* no more reporters for this source send a query */
               mcpd_router.membership_query_func[proto](ifp,
                                                        gp,
                                                        src->addr,
                                                        1);
               snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
            }
            snSrcCnt++;
         }
      }

      for(src = gp->ex_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            /* found source with matching reporter */
            snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_EX_CLEAR;
            if(proto == MCPD_PROTO_IGMP)
            {
               snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
            }
            else
            {
               IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
            }
            
            mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                      pkt_info->rxdev_ifi,
                                                      gp->addr,
                                                      src->addr,
                                                      rep->addr,
                                                      pkt_info->tci,
                                                      rep->version);

            /* clean up reporter source information */
            mcpd_src_rep_cleanup(proto, src, rep);
            mcpd_rep_src_cleanup(proto, rep, src);
            if(0 == mcpd_src_rep_count(proto, src)) 
            {
               /* no more reporters for this source send a query */
               mcpd_router.membership_query_func[proto](ifp,
                                                        gp,
                                                        src->addr,
                                                        1);
               snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
            }  
            snSrcCnt++;
         }
      }
   }
   else
   {
      MCPD_TRACE(MCPD_TRC_INFO,"Processing INCLUDE with source");
      /* include filter for reporter - remove any excluded sources */
      for(src = gp->ex_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            /* found source with matching reporter */
            snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_EX_CLEAR;
            if(proto == MCPD_PROTO_IGMP)
            {
               snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
            }
            else
            {
               IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
            }

            if (MCPD_RET_OK == mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                         pkt_info->rxdev_ifi,
                                                                         gp->addr,
                                                                         src->addr,
                                                                         rep->addr,
                                                                         pkt_info->tci,
                                                                         rep->version) )
            {
               MCPD_TRACE(MCPD_TRC_INFO,"Found valid excluded source");
               ret = MCPD_RET_OK;
            }

            /* clean up reporter source information */
            mcpd_src_rep_cleanup(proto, src, rep);
            mcpd_rep_src_cleanup(proto, rep, src);
            if(0 == mcpd_src_rep_count(proto, src)) 
            {
               /* no more reporters for this source send a query */
               mcpd_router.membership_query_func[proto](ifp,
                                                        gp,
                                                        src->addr,
                                                        1);
               snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
            }  
            snSrcCnt++;
         }
      }

      MCPD_TRACE(MCPD_TRC_INFO,"Processing included sources");
      /* remove include sources that are not part of current list */
      for(src = gp->in_sources; src != NULL;
                src = (t_MCPD_SRC_OBJ *)src->next)
      {
         srep = mcpd_group_src_rep_lookup(proto, src, rep);
         if ( srep )
         {
            for( i = 0; i < num_src; i++ )
            {
               if(proto == MCPD_PROTO_IGMP)
               {
                  src_elm = (UINT8 *) &source_list[i];
               }
               else
               {
                  src_elm = (UINT8 *) &source_list6[i];
               }

               if (MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](src->addr, src_elm) )
               {
                  /* found source - update timer, continue search */
                  if (MCPD_RET_OK != mcpd_router.admission_control_func[proto](MCPD_ADMISSION_RE_JOIN,
                                                                               pkt_info->rxdev_ifi,
                                                                               gp->addr,
                                                                               src->addr,
                                                                               rep->addr,
                                                                               pkt_info->tci,
                                                                               rep->version) )
                  {
                     continue;
                  }
                  MCPD_TRACE(MCPD_TRC_INFO,"Found valid included source");
                  ret = MCPD_RET_OK;
                  mcpd_router.update_src_tmr_func[proto](src);
                  break;
               }
            }
            if ( i == num_src )
            {
               /* did not find source/reporter combination */
               snSrcList[snSrcCnt].src_action |= MCPD_SNOOP_IN_CLEAR;
               if(proto == MCPD_PROTO_IGMP)
               {
                  snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
               }
               else
               {
                  IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
               }

               /* clean up reporter source information */
               if (MCPD_RET_OK == mcpd_router.admission_control_func[proto](MCPD_ADMISSION_LEAVE,
                                                                            pkt_info->rxdev_ifi,
                                                                            gp->addr,
                                                                            src->addr,
                                                                            rep->addr,
                                                                            pkt_info->tci,
                                                                            rep->version) )
               {
                  MCPD_TRACE(MCPD_TRC_INFO,"Cleanup validated");
                  ret = MCPD_RET_OK;
               }
               
               mcpd_src_rep_cleanup(proto, src, rep);
               mcpd_rep_src_cleanup(proto, rep, src);
               if(0 == mcpd_src_rep_count(proto, src))
               {
                  /* no more reporters for this source send a query */
                  mcpd_router.membership_query_func[proto](ifp,
                                                           gp,
                                                           src->addr,
                                                           1);
                  snSrcList[snSrcCnt].src_action |= MCPD_MFC_DEL;
               }
               snSrcCnt++;
            }
         }
      }

      MCPD_TRACE(MCPD_TRC_INFO,"Going through All sources %d", num_src);
      /* add new entries */
      for(i = 0; i < num_src; i++) 
      {
         if(proto == MCPD_PROTO_IGMP)
         {
            src_elm = (UINT8 *) &source_list[i];
         }
         else
         {
            src_elm = (UINT8 *) &source_list6[i];
         }
         src = mcpd_group_src_lookup(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     MCPD_FMODE_INCLUDE);
         /* source does not exist for the group - create entry */
         if ( NULL == src )
         {
            src = mcpd_group_src_add(proto, 
                                     gp, 
                                     (UINT8 *)src_elm, 
                                     MCPD_FMODE_INCLUDE);

            if(src == NULL) 
            {
               MCPD_TRACE(MCPD_TRC_INFO,"Memory error");
               return MCPD_RET_MEMERR;
            }
         }

         /* add reporter to the source and create snooping info */
         if (MCPD_RET_OK != mcpd_router.admission_control_func[proto](MCPD_ADMISSION_JOIN,
                                                                      pkt_info->rxdev_ifi,
                                                                      gp->addr,
                                                                      src->addr,
                                                                      rep->addr,
                                                                      pkt_info->tci,
                                                                      rep->version) )
         {
            continue;
         }
         MCPD_TRACE(MCPD_TRC_INFO,"Source validated");
         ret = MCPD_RET_OK;

         mcpd_group_src_rep_add(proto, src, rep);
         mcpd_group_rep_src_add(proto, rep, src);

         mcpd_router.update_src_tmr_func[proto](src);

         snSrcList[snSrcCnt].src_action = (MCPD_SNOOP_IN_ADD | MCPD_MFC_ADD);
         if(proto == MCPD_PROTO_IGMP)
         {
            snSrcList[snSrcCnt].srcAddr4.s_addr = ((struct in_addr *)src->addr)->s_addr;
         }
         else
         {
            IN6_ASSIGN_ADDR(&snSrcList[snSrcCnt].srcAddr6, src->addr);
         }
         snSrcCnt++;
      }
   }

   if (MCPD_RET_OK == ret) 
   {
      ret = mcpd_ssm_post_process(proto, ifp, gp, 
                                  pkt_info, snSrcCnt, &snSrcList[0],
                                  rep);

      /* clean up reporter and source objects
         rep should not be used beyond this point
         this call has to be made before mcpd_ssm_handle_ssm_info
         which relies on the removal of unused source objects */
      mcpd_update_group_src_reporters(proto, gp);

      if(ifp->if_type & MCPD_IF_TYPE_ROUTED) 
      {
         mcpd_ssm_handle_ssm_info(proto, gp, ifp, pkt_info);
      }
   } 
   else 
   {
     MCPD_TRACE(MCPD_TRC_INFO,"Message contained no valid information");
   }

   return ret;
} /* mcpd_ssm_process_toin_filter */


void mcpd_update_group_src_reporters(t_MCPD_PROTO_TYPE proto, 
                                     t_MCPD_GROUP_OBJ *gp)
{
   t_MCPD_SRC_OBJ *src = NULL;
   t_MCPD_SRC_OBJ *src_next = NULL;
   t_MCPD_REP_OBJ *rep = NULL;
   t_MCPD_REP_OBJ *rep_next = NULL;

   for(src = gp->in_sources; src; ) 
   {
      src_next = (t_MCPD_SRC_OBJ *)src->next;
      if(mcpd_src_rep_count(proto, src) == 0) 
      {
         mcpd_src_cleanup(proto, gp, src, MCPD_FMODE_INCLUDE);
      }
      src = src_next;
   }

   for(src = gp->ex_sources; src; ) 
   {
      src_next = (t_MCPD_SRC_OBJ *)src->next;
      if(mcpd_src_rep_count(proto, src) == 0) 
      {
         mcpd_src_cleanup(proto, gp, src, MCPD_FMODE_EXCLUDE);
      }
      src = src_next;
   }

   for(rep = gp->members; rep; ) 
   {
      rep_next = (t_MCPD_REP_OBJ *)rep->next;

      if(mcpd_rep_src_count(proto, rep) == 0) 
      { 
         mcpd_rep_cleanup(proto, gp, rep);
      }

      rep = rep_next;
   }

   if (proto == MCPD_PROTO_IGMP)
   {
      /* no leave for v1 IGMP so need to wait for group timeout
         null sources, remove the group */
      if(!gp->in_sources && !gp->ex_sources &&
         (gp->v1_host_prsnt_timer == MCPD_FALSE))
      {
         if(NULL == mcpd_get_timer(mcpd_igmp_timer_group, gp))
         {
            if(NULL == mcpd_timer_new(IGMP_TIMER_GROUP_DELAY_MSEC, 
                                      mcpd_igmp_timer_group, 
                                      gp))
            {
               MCPD_TRACE(MCPD_TRC_LOG, "mcpd_igmp_timer_group failed");
            } 
         }
      }
   }
#ifdef SUPPORT_MLD
   else
   {
      if(!gp->in_sources && !gp->ex_sources)
      {
         if(NULL == mcpd_get_timer(mcpd_mld_timer_group, gp))
         {
            if(NULL == mcpd_timer_new(MLD_TIMER_GROUP_DELAY_MSEC, 
                                      mcpd_mld_timer_group, 
                                      gp)) 
            {
               MCPD_TRACE(MCPD_TRC_LOG, "mcpd_mld_timer_group failed");
            }
         }
      }
   }
#endif /* SUPPORT_MLD */
  return;
} /* mcpd_update_group_src_reporters*/
