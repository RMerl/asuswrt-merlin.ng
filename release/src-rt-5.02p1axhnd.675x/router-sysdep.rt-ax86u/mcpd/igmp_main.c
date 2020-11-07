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
 * File Name  : igmp.c
 *
 * Description:
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <assert.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include "mcpd.h"
#include "common.h"
#include "igmp.h"
#include "igmp_main.h"
#include "igmp_proxy.h"
#include "igmp_snooping.h"
#include "obj_hndlr.h"
#include "ssm_hndlr.h"
#include "mcpd_mroute.h"
#include "mcpd_nl.h"
#include "mcpd_main.h"
#include "common.h"
#include "mcpd_timer.h"
#include "mcpd_omci.h"

extern t_MCPD_ROUTER mcpd_router;

static void mpcd_igmpv2_start_last_member_query_tmr(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_GROUP_OBJ *gp);

static int mcpd_igmp_join_filter(const struct in_addr *group)
{
    if(!group)
        return 0;

    if(!IN_MULTICAST(ntohl(group->s_addr)) ||
       ((group->s_addr & htonl(0xFFFFFF00)) == htonl(0xE0000000)))
    {
      return 0;
    }
    else
    {
        t_MCPD_FILTER_EXCEPTION *exceptionObj = mcpd_router.igmp_config.filter_list;

        while (exceptionObj) {
            if ( (group->s_addr & exceptionObj->mask.s6_addr32[0]) == (exceptionObj->address.s6_addr32[0] & exceptionObj->mask.s6_addr32[0]) )
            {
                return 0;
            }
            exceptionObj = exceptionObj->next;
        }
    }

    return 1;
} /* mcpd_igmp_join_filter */

t_MCPD_RET_CODE mcpd_igmp_init(void)
{
   int optval = 0;
   char ra[4];

   /* Set router alert */
   ra[0] = 148;
   ra[1] = 4;
   ra[2] = 0;
   ra[3] = 0;

   if(setsockopt(mcpd_router.sock_igmp, IPPROTO_IP, IP_OPTIONS, ra, 4) < 0)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "can't set IP_OPTIONS");
      return MCPD_RET_GENERR;
   }

   /* Set reuseaddr, ttl and loopback */
   optval = 1; /* enable resuse addr option */
   if(setsockopt(mcpd_router.sock_igmp, SOL_SOCKET, SO_REUSEADDR,
                                     (void*)&optval, sizeof(optval)) < 0)
   {
      MCPD_TRACE(MCPD_TRC_ERR, "can't set SO_REUSEADDR");
      return MCPD_RET_GENERR;
   }

   optval = 1; /* set number of hops to 1 */
   if(setsockopt(mcpd_router.sock_igmp, IPPROTO_IP, IP_MULTICAST_TTL,
                                     (void*)&optval, sizeof(optval))< 0)
   {
      MCPD_TRACE(MCPD_TRC_ERR, "can't set IP_MULTICAST_TTL");
      return MCPD_RET_GENERR;
   }

   optval = 0; /* disable loopback */
   if(setsockopt(mcpd_router.sock_igmp, IPPROTO_IP, IP_MULTICAST_LOOP,
                                     (void*)&optval, sizeof(optval)) < 0)
   {
      MCPD_TRACE(MCPD_TRC_ERR, "can't set IP_MULTICAST_LOOP");
      return MCPD_RET_GENERR;
   }

   if ( mcpd_igmp_mroute_init() < 0 )
   {
      return MCPD_RET_GENERR;
   }

   return MCPD_RET_OK;
} /* mcpd_igmp_init */

t_MCPD_RET_CODE mcpd_igmp_interface_init(t_MCPD_INTERFACE_OBJ *ifp)
{
   int i = 1;

   if(setsockopt(mcpd_router.sock_igmp,
                 IPPROTO_IP,
                 IP_PKTINFO,
                 &i,
                 sizeof(i)) < 0)
   {
      MCPD_TRACE(MCPD_TRC_ERR, "can't set IP_PKTINFO");
      return MCPD_RET_GENERR;
   }

   if (ifp->if_dir == MCPD_DOWNSTREAM )
   {
      struct ip_mreqn mreq;
      mreq.imr_address.s_addr = 0;
      mreq.imr_ifindex = ifp->if_index;

      /* Add membership to ALL_ROUTERS and ALL_ROUTERS_V3 on this interface */
      mreq.imr_multiaddr.s_addr = htonl(INADDR_ALLHOSTS_GROUP);
      if(setsockopt(mcpd_router.sock_igmp, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                               (void*)&mreq, sizeof(mreq)) < 0)
      {
         MCPD_TRACE(MCPD_TRC_LOG, "IP_ADD_MEMBERSHIP fails");
         return MCPD_RET_GENERR;
      }

      mreq.imr_multiaddr.s_addr = htonl(INADDR_ALLRTRS_IGMPV3_GROUP);
      if(setsockopt(mcpd_router.sock_igmp, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                               (void*)&mreq, sizeof(mreq)) < 0)
      {
         MCPD_TRACE(MCPD_TRC_LOG, "IP_ADD_MEMBERSHIP fails");
         return MCPD_RET_GENERR;
      }
   }

   return MCPD_RET_OK;
} /* mcpd_igmp_interface_init */

t_MCPD_RET_CODE mcpd_igmp_process_input(t_MCPD_PKT_INFO *pkt_info)
{
   t_IGMPv12_REPORT *report12;
   t_IGMPv3_REPORT *report3;
   t_IGMPv3_QUERY *query3 = NULL;
   t_MCPD_INTERFACE_OBJ *ifp = NULL;
   t_MCPD_RET_CODE ret = MCPD_RET_OK;

#ifdef MCPD_DEBUG
   mcpd_display_mem_usage();
   mcpd_dump_obj_tree();
#endif

   if(!pkt_info)
   {
      MCPD_TRACE(MCPD_TRC_ERR, "invalid ptr rcvd");
      return MCPD_RET_GENERR;
   }

   if(pkt_info->data_len < (int) sizeof(*report12))
   {
      MCPD_TRACE(MCPD_TRC_ERR, "invalid length pkt rcvd");
      return MCPD_RET_GENERR;
   }

   ifp = mcpd_interface_lookup(pkt_info->parent_ifi);
   if (ifp == NULL)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "Unexpected packet from interface %d\n", pkt_info->parent_ifi);
      return MCPD_RET_GENERR;
   }

   report12 = (t_IGMPv12_REPORT *)&pkt_info->pkt[0];
   MCPD_TRACE(MCPD_TRC_LOG, "IGMP Pkt Rcvd from brif %d, if %d, type %d," \
                            "len %d, ip %s, ifdir %d", 
                            pkt_info->parent_ifi, pkt_info->rxdev_ifi, 
                            report12->type, pkt_info->data_len,
                            inet_ntoa(pkt_info->ipv4rep), ifp->if_dir);

   /* don't process any messages from upstream in mcpd*/
   if(ifp->if_dir == MCPD_UPSTREAM)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "Ignoring message from UPSTREAM interface %s", ifp->if_name);
      return MCPD_RET_GENERR;
   }

   switch (report12->type)
   {
      case IGMP_MEMBERSHIP_QUERY:
         if ( pkt_info->data_len == sizeof(*report12) )
         {
            if (report12->code == 0)
            {
               /*version 1 query*/
               MCPD_TRACE(MCPD_TRC_LOG, "igmpv1 query: %s", inet_ntoa(report12->group));
               mcpd_igmp_receive_membership_query(ifp, &report12->group, NULL,
                                                  &pkt_info->ipv4rep, 0, 0);
            }
            else
            {
               /*version 2 query*/
               MCPD_TRACE(MCPD_TRC_LOG, "igmpv2 query: %s", inet_ntoa(report12->group));
               mcpd_igmp_receive_membership_query(ifp, &report12->group, NULL,
                                                  &pkt_info->ipv4rep, 0, 0);
            }
         }
         else if (pkt_info->data_len >= (int) sizeof(*query3))
         {
            /* version 3 query */
            query3 = (t_IGMPv3_QUERY *)&pkt_info->pkt[0];
            MCPD_TRACE(MCPD_TRC_LOG, "igmpv3 query: %s", inet_ntoa(query3->group));
            mcpd_igmp_receive_membership_query(ifp, &query3->group, query3->sources, 
                                               &pkt_info->ipv4rep, query3->numsrc, 
                                               query3->suppress);
         }
         else
         {
            MCPD_TRACE(MCPD_TRC_ERR, "invalid length for query packet");
            ret = MCPD_RET_GENERR;
         }
         break;

      case IGMP_V1_MEMBERSHIP_REPORT:
      case IGMP_V2_MEMBERSHIP_REPORT:
          MCPD_TRACE(MCPD_TRC_LOG, "igmpv1 or v2 report: %s", inet_ntoa(report12->group));
          ret = mcpd_igmp_interface_membership_report_v12(ifp, &pkt_info->ipv4rep, report12, pkt_info);
          break;

      case IGMP_V2_MEMBERSHIP_LEAVE:
         MCPD_TRACE(MCPD_TRC_LOG, "igmpv2 leave: %s", inet_ntoa(report12->group));
         ret = mcpd_igmp_interface_membership_leave_v2(ifp, &pkt_info->ipv4rep, report12, pkt_info);
         break;

      case IGMP_V3_MEMBERSHIP_REPORT:
         if(pkt_info->data_len < (int) sizeof(*report3))
         {
            MCPD_TRACE(MCPD_TRC_ERR, "invalid length for IGMPv3 report");
            return MCPD_RET_GENERR;
         }
         report3 = (t_IGMPv3_REPORT *)&pkt_info->pkt[0];
         ret = mcpd_igmp_interface_membership_report_v3(ifp, &pkt_info->ipv4rep, report3, pkt_info);
         break;

      default:
         MCPD_TRACE(MCPD_TRC_ERR, "IGMP message type %d not supported", report12->type);
         break;
   }

#ifdef MCPD_DEBUG
   mcpd_display_mem_usage();
   mcpd_dump_obj_tree();
#endif

   return ret;
} /* mcpd_igmp_process_input */

enum e_multicast_type mcpd_igmp_getMulticastGroupType (unsigned int address)
{
    if (mcpd_get_mcast_group_mode() == MULTICAST_MODE_FIRST_IN)
    {
        UINT8 isSsm = 0;
        if (mcpd_does_group_exist_anywhere (MCPD_PROTO_IGMP, (UINT8*)&address, &isSsm))
        {
            return isSsm ? MULTICAST_TYPE_SSM : MULTICAST_TYPE_ASM;
        }
        return MULTICAST_TYPE_UNK;
    }
    else 
    {
        if ((address & htonl(0xFF000000)) == htonl(0xE8000000))
        {
            return MULTICAST_TYPE_SSM;
        }
        else 
        {
            return MULTICAST_TYPE_ASM;
        }
    }
}

t_MCPD_RET_CODE mcpd_igmp_interface_membership_report_v12(
                                               t_MCPD_INTERFACE_OBJ *ifp,
                                               struct in_addr *src,
                                               t_IGMPv12_REPORT *report,
                                               t_MCPD_PKT_INFO *pkt_info)
{
   t_MCPD_GROUP_OBJ *gp = NULL;;
   t_MCPD_REP_OBJ *rep = NULL;
   t_MCPD_RET_CODE ret = MCPD_RET_OK;
   t_MCPD_OLD_REPORTER_TO_DELETE oldReporter;

   /* check if the group should be ignored */
   if (0 == mcpd_igmp_join_filter(&report->group))
   {
       return MCPD_RET_GENERR;
   }

   if ( mcpd_igmp_getMulticastGroupType (report->group.s_addr) == MULTICAST_TYPE_SSM) 
   {
       MCPD_TRACE(MCPD_TRC_LOG, "Illegal IGMP V2 Join for SSM address %s", inet_ntoa(report->group));
       return MCPD_RET_GENERR;
   }

   if((gp = mcpd_interface_group_add(MCPD_PROTO_IGMP,
                                     ifp,
                                     (UINT8 *)&report->group,
                                     pkt_info->rxdev_ifi)) == NULL)
   {
      return MCPD_RET_GENERR;
   }

   if(report->type == IGMP_V2_MEMBERSHIP_REPORT)
   {
      gp->v2_host_prsnt_timer = MCPD_TRUE;

      mcpd_timer_cancel(mpcd_igmpv2_last_member_query_tmr, gp);
      mcpd_timer_cancel(mcpd_igmp_v2_bckcomp_tmr, gp);
      if(NULL == mcpd_timer_new(mcpd_router.igmp_config.query_interval * mcpd_router.igmp_config.robust_val * MSECS_IN_SEC +
                                mcpd_router.igmp_config.query_resp_interval * MSECS_IN_SEC / 10,
                                mcpd_igmp_v2_bckcomp_tmr,
                                gp) )
      {
         MCPD_TRACE(MCPD_TRC_LOG, "mcpd_igmp_v2_bckcomp_tmr failed");
      }
   }
   else if(report->type == IGMP_V1_MEMBERSHIP_REPORT)
   {
      gp->v1_host_prsnt_timer = MCPD_TRUE;

      mcpd_timer_cancel (mpcd_igmpv2_last_member_query_tmr, gp);
      mcpd_timer_cancel (mcpd_igmp_v1_bckcomp_tmr, gp);
      if(NULL != mcpd_timer_new(mcpd_router.igmp_config.query_interval * mcpd_router.igmp_config.robust_val * MSECS_IN_SEC +
                                mcpd_router.igmp_config.query_resp_interval * MSECS_IN_SEC / 10,
                                mcpd_igmp_v1_bckcomp_tmr,
                                gp))
      {
         MCPD_TRACE(MCPD_TRC_LOG, "mcpd_igmp_v1_bckcomp_tmr failed");
      }
   }

   gp->leaveQueriesLeft = -1; // We are not doing Leave Queries

   oldReporter.valid = 0;
   rep = mcpd_group_rep_add(MCPD_PROTO_IGMP, gp, (UINT8 *)src, pkt_info->rxdev_ifi, &oldReporter);

   if(!rep)
   {
       ret = MCPD_RET_GENERR;
       return ret;
   }
   rep->version = report->type;

   ret = mcpd_ssm_process_ex_filter(MCPD_PROTO_IGMP,
                                    ifp,
                                    gp,
                                    0,
                                    NULL,
                                    rep,
                                    pkt_info);
   /* rep may not be valid and should not be used beyond this point */

   if (oldReporter.valid) {
      mcpd_wipe_reporter_for_old_port(MCPD_PROTO_IGMP, &oldReporter);
   }

   return ret;
} /* mcpd_igmp_interface_membership_report_v12 */

t_MCPD_RET_CODE mcpd_igmp_interface_membership_leave_v2(t_MCPD_INTERFACE_OBJ *ifp,
                                             struct in_addr *reporter_addr,
                                             t_IGMPv12_REPORT *report,
                                             t_MCPD_PKT_INFO *pkt_info)
{
   t_MCPD_GROUP_OBJ *gp  = NULL;
   t_MCPD_REP_OBJ   *rep = NULL;
   t_MCPD_RET_CODE   ret = MCPD_RET_OK;

   if (!IN_MULTICAST(ntohl(report->group.s_addr) )) // MACRO needs the address in host order here 
   {
      MCPD_TRACE(MCPD_TRC_ERR, "Ignore non-multicast ...");
      return MCPD_RET_GENERR;
   }

   if(NULL == ifp->igmp_proxy.groups)
   {
       return MCPD_RET_GENERR;
   }

   gp = mcpd_interface_group_lookup(MCPD_PROTO_IGMP, ifp, 
                                    (UINT8 *)&(report->group));
   if( gp )
   {
      if (!mcpd_router.igmp_config.fast_leave_enable)
      {
         MCPD_TRACE(MCPD_TRC_LOG, "Leave V2 rx'd with No Fast Leave");
         if ((gp->members != NULL) && (-1 == gp->leaveQueriesLeft))
         {
            // Set all reporters, on all ports, for this group to have the really short, Last Member Timeout
            for (rep = gp->members; rep; rep = rep->next)
            {
               mcpd_igmp_set_last_member_rep_timer(rep);        
            }
            mpcd_igmpv2_start_last_member_query_tmr(ifp, gp);
         }
      }
      else 
      {         
         rep = mcpd_group_rep_port_lookup(MCPD_PROTO_IGMP, gp, (UINT8 *)reporter_addr, pkt_info->rxdev_ifi);
         if (rep)
         {
            ret = mcpd_ssm_process_in_filter(MCPD_PROTO_IGMP,
                                             ifp,
                                             gp,
                                             0,
                                             NULL,
                                             rep,
                                             pkt_info);
         }
         /* rep may not be valid and should not be used beyond this point */
      }
   }

   return ret;
} /* mcpd_igmp_interface_membership_leave_v2 */

/* processing IGMP V3 membership group records */
t_MCPD_RET_CODE mcpd_igmp_process_group_record(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in_addr *src,
                                              t_IGMPv3_REPORT *report,
                                              t_MCPD_PKT_INFO *pkt_info,
                                              t_IGMP_GRP_RECORD *grp_rec)
{
    t_MCPD_GROUP_OBJ *gp = NULL;
    t_MCPD_REP_OBJ *rep = NULL;
    UINT16 num_src = 0;
    UINT8 type;
    t_MCPD_RET_CODE ret = MCPD_RET_OK;
    UINT8 *sources = NULL;
    enum e_multicast_type reportType = MULTICAST_TYPE_ASM;
    enum e_multicast_type groupType = MULTICAST_TYPE_UNK;
    t_MCPD_OLD_REPORTER_TO_DELETE oldReporter;

    /* check if the group should be ignored */
    if (0 == mcpd_igmp_join_filter(&grp_rec->group))
    {
        return MCPD_RET_GENERR;
    }

    type   = (UINT8)grp_rec->type;
    num_src = ntohs(grp_rec->numsrc);

    MCPD_TRACE(MCPD_TRC_LOG, "%s, type %d, source count %d", inet_ntoa(grp_rec->group), type, num_src);

    /* RFC4607/5771 - for MCAST SSM */
    if (num_src != 0 && ((MODE_TO_EXCLUDE == type) || (MODE_IS_EXCLUDE == type)) ) 
    {
        MCPD_TRACE(MCPD_TRC_ERR, 
                    "IGMPV3 EXCLUDE messages may not contain sources (type %d, Group %s)",
                    type, inet_ntoa(grp_rec->group));
        return MCPD_RET_GENERR;
    }

    if ((type == MODE_TO_INCLUDE) || (type == MODE_IS_INCLUDE))
    {
        /* This is an SSM report if there are sources */
        if (grp_rec->numsrc != 0)
        {
            MCPD_TRACE(MCPD_TRC_LOG, "SSM report rx'd group = %08x", grp_rec->group.s_addr);
            sources = (UINT8 *)&grp_rec->sources;
            reportType = MULTICAST_TYPE_SSM;
        }
        else
        {
            MCPD_TRACE(MCPD_TRC_LOG, "ASM/SSM report rx'd group = %08x", grp_rec->group.s_addr);
            reportType = MULTICAST_TYPE_BOTH;
        }
    } 
    else if ((type == MODE_ALLOW_NEW_SRCS) || (type == MODE_BLOCK_OLD_SRCS))
    {
        MCPD_TRACE(MCPD_TRC_LOG, "SSM report rx'd group = %08x", grp_rec->group.s_addr);
        sources = (UINT8 *)&grp_rec->sources;
        reportType = MULTICAST_TYPE_SSM;        
    }

    groupType = mcpd_igmp_getMulticastGroupType(grp_rec->group.s_addr);
    MCPD_TRACE(MCPD_TRC_LOG, "Group %08x is of type %s", grp_rec->group.s_addr,
               (groupType == MULTICAST_TYPE_UNK) ? "UNK" : 
               ((groupType == MULTICAST_TYPE_ASM) ? "ASM" : "SSM"));

    if ((reportType != groupType) && (groupType != MULTICAST_TYPE_UNK) && (reportType != MULTICAST_TYPE_BOTH))
    {
        if (mcpd_get_mcast_group_mode() == MULTICAST_MODE_FIRST_IN) 
        {
            MCPD_TRACE(MCPD_TRC_ERR, 
                       "Report type %s not allowed for group %x that is already set up as type %s",
                       reportType == 0 ? "ASM" : "SSM", 
                       (int)grp_rec->group.s_addr,
                       groupType == 0 ? "ASM" : "SSM");
        }
        else 
        {
            MCPD_TRACE(MCPD_TRC_ERR, 
                       "Report type %s not allowed for group %x which is reserved for type %s",
                       reportType == 0 ? "ASM" : "SSM", 
                       (int)grp_rec->group.s_addr,
                       groupType == 0 ? "ASM" : "SSM");
        }
        return MCPD_RET_GENERR;
    }

    if ((gp = mcpd_interface_group_add(MCPD_PROTO_IGMP,
                                       ifp,
                                       (UINT8 *)&grp_rec->group,
                                       pkt_info->rxdev_ifi)) == NULL)
    {
        return MCPD_RET_GENERR;
    }

    oldReporter.valid = 0;
    rep = mcpd_group_rep_add(MCPD_PROTO_IGMP, gp, (UINT8 *)src, pkt_info->rxdev_ifi, &oldReporter);
    if(!rep)
    {
        return MCPD_RET_GENERR;
    }
    rep->version = report->type;

    switch(type)
    {
        case MODE_TO_INCLUDE:
        case MODE_IS_INCLUDE:
            ret = mcpd_ssm_process_in_filter(MCPD_PROTO_IGMP,
                                             ifp,
                                             gp,
                                             num_src,
                                             sources,
                                             rep,
                                             pkt_info);
            break;

        case MODE_TO_EXCLUDE:
        case MODE_IS_EXCLUDE:
            ret = mcpd_ssm_process_ex_filter(MCPD_PROTO_IGMP,
                                             ifp,
                                             gp,
                                             num_src,
                                             sources,
                                             rep,
                                             pkt_info);
            break;

        case MODE_ALLOW_NEW_SRCS:
            ret = mcpd_ssm_process_allow_newsrc_filter(
                                         MCPD_PROTO_IGMP,
                                         ifp,
                                         gp,
                                         num_src,
                                         sources,
                                         rep,
                                         pkt_info);
            break;

        case MODE_BLOCK_OLD_SRCS:
            ret = mcpd_ssm_process_block_oldsrc_filter(
                                         MCPD_PROTO_IGMP,
                                         ifp,
                                         gp,
                                         num_src,
                                         sources,
                                         rep,
                                         pkt_info);
            break;

        default:
            MCPD_TRACE(MCPD_TRC_ERR, "group record type undefined");
    }
    /* rep may not be valid and should not be used beyond this point */

    if (oldReporter.valid) {
        mcpd_wipe_reporter_for_old_port(MCPD_PROTO_IGMP, &oldReporter);
    }

    return ret;
    
} /* mcpd_igmp_process_group_record */

t_MCPD_RET_CODE mcpd_igmp_interface_membership_report_v3(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in_addr *src,
                                              t_IGMPv3_REPORT *report,
                                              t_MCPD_PKT_INFO *pkt_info)
{
    UINT16 num_src = 0;
    UINT16 num_grps = 0;
    UINT8 i;
    t_IGMP_GRP_RECORD *grp_rec = NULL;
    t_MCPD_RET_CODE ret = MCPD_RET_OK;
    unsigned int                remlen;
    unsigned int                grplen;

    num_grps = ntohs(report->numgrps);
    grp_rec = report->group;
    remlen = pkt_info->data_len - sizeof(*report);
    for(i = 0; i < num_grps; i++)
    {
        if ( remlen < sizeof(*grp_rec) )
        {
           MCPD_TRACE(MCPD_TRC_ERR, "Invalid packet: not enough data for group record");
           return MCPD_RET_GENERR;
        }
        remlen -= sizeof(*grp_rec);
        num_src = ntohs(grp_rec->numsrc);

        if (remlen < (num_src * sizeof(struct in_addr)))
        {
           MCPD_TRACE(MCPD_TRC_ERR, "Invalid packet: not enough data for sources");
           return MCPD_RET_GENERR;
        }
        remlen -= (num_src * sizeof(struct in_addr));
        grplen = sizeof(*grp_rec) + (num_src * sizeof(struct in_addr));
        grp_rec = (t_IGMP_GRP_RECORD *)((UINT8 *)grp_rec + grplen);
    }

    /* all data is present - continue processing */
    grp_rec = report->group;
    for(i = 0; i < num_grps; i++)
    {
        ret = mcpd_igmp_process_group_record(ifp,
                                            src,
                                            report,
                                            pkt_info,
                                            grp_rec);
        num_src = ntohs(grp_rec->numsrc);
        grplen = sizeof(*grp_rec) + (num_src * sizeof(struct in_addr));
        grp_rec = (t_IGMP_GRP_RECORD *)((UINT8 *)grp_rec + grplen);
    }

    return ret;
} /* mcpd_igmp_interface_membership_report_v3 */

void mcpd_igmp_timer_querier(t_MCPD_INTERFACE_OBJ *ifp)
{
   if (ifp->igmp_proxy.oqp > 0)
      --ifp->igmp_proxy.oqp;
   if (ifp->igmp_proxy.oqp == 0)
      ifp->igmp_proxy.is_querier = MCPD_TRUE;

   return;
} /* mcpd_igmp_timer_querier */

void mcpd_igmp_timer_group(void *handle)
{
   t_MCPD_GROUP_OBJ *gp = (t_MCPD_GROUP_OBJ *)handle;

   if(gp)
   {
      mcpd_igmp_krnl_drop_membership(NULL, gp);
      mcpd_group_destroy(MCPD_PROTO_IGMP, gp);
   }
   return;
} /* mcpd_igmp_timer_group */

void mcpd_igmp_timer_reporter(void *handle)
{
   t_MCPD_REP_OBJ *rep = (t_MCPD_REP_OBJ *)handle;

   if(rep)
   {
      t_MCPD_GROUP_OBJ *gp = rep->parent_group;
       
      /* remove reporter from parent group */
      mcpd_rep_cleanup(MCPD_PROTO_IGMP, gp, rep);

      if(mcpd_group_members_num_on_rep_port(gp, rep->rep_ifi) == 0)
      {
          bcm_mcast_api_igmp_drop_group(mcpd_router.sock_nl,
                                            gp->ifp->if_index,
                                            rep->rep_ifi,
                                            (struct in_addr *)(gp->addr));

#if defined(CONFIG_BCM_OVS_MCAST)
          mcpd_ovs_manage_snoop_entry(rep->rep_ifi, 
                                      gp->ifp->if_index, 
                                      ((struct in_addr *)(gp->addr))->s_addr, 
                                      NULL, /* Mcast V6 grp address N/A */
                                      0,    /* Mcast V4Src 0 for IGMPv2 */
                                      NULL, /* Mcast V6src N/A */ 
                                      0     /* 0 - remove snoop entry */
                                      );
#endif
      } 
      
      /* if no reporters remain, remove group */
      if (gp->members == NULL)
      {
          mcpd_igmp_krnl_drop_membership(NULL, gp);

          mcpd_group_destroy(MCPD_PROTO_IGMP, gp);
      }
   }
   return;
} /* mcpd_igmp_timer_reporter */

void mcpd_igmp_timer_source(void *handle)
{
   t_MCPD_GROUP_OBJ *gp = NULL;
   t_MCPD_SRC_OBJ *src = (t_MCPD_SRC_OBJ *)handle;

   if(src)
   {
      gp = src->gp;
      mcpd_src_cleanup(MCPD_PROTO_IGMP, gp, src, src->fmode);
   }
   return;
} /* mcpd_igmp_timer_source */

void mcpd_igmp_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                struct in_addr *group, 
                                struct in_addr *sources, 
                                int numsrc, 
                                int SRSP,
                                int leave_qry)
{
   char buf[12], *pbuf = NULL;
   t_IGMPv3_QUERY *query = NULL;
   t_IGMPv12_QUERY *queryv12 = NULL;
   struct sockaddr_in sin;
   struct ip_mreqn ipmreqn;
   int i, igmplen, version;
#if !defined(EPON_SFU)
   int mode;
   int l2lenable = 0;
#endif   

   if (ifp == NULL)
   {
      MCPD_ASSERT(ifp != NULL);
      return;
   }

   if (ifp->if_addr.s_addr == 0)
   {
      return;      
   }

   if (ifp->if_dir == MCPD_UPSTREAM)
   {
      return;
   }

   if (0 == (ifp->proto_enable & MCPD_IGMP_SNOOPING_ENABLE))
   {
      return;
   }

/* For epon wan, only check fast_leave flag */
#if !defined(EPON_SFU)
   bcm_mcast_api_get_snooping_cfg(ifp->if_index, BCM_MCAST_PROTO_IPV4, &mode, &l2lenable);
   if ( (0 == l2lenable) &&
        ((0 == (ifp->if_type & MCPD_IF_TYPE_ROUTED)) ||
         (mcpd_is_bridge_associated_with_mcast_wan_service(ifp, MCPD_PROTO_IGMP) != MCPD_TRUE)) )
   {
      return;
   }
#endif
   if((leave_qry) && (mcpd_router.igmp_config.fast_leave_enable))
      return;

   if(mcpd_router.igmp_config.default_version == 3)
       version = IGMP_VERSION_3;
   else if(mcpd_router.igmp_config.default_version == 2)
       version = IGMP_VERSION_2;
   else
       version = IGMP_VERSION_1;

   if(version == IGMP_VERSION_3)
   {
      /* Allocate a buffer to build the query */ 
      if (numsrc > 0) 
      {
         pbuf = (char *)malloc(sizeof(*query) + (numsrc * sizeof(struct in_addr)));
         if (pbuf == NULL)
            return;
         query = (t_IGMPv3_QUERY *) pbuf;
      }
      else
      {
         query = (t_IGMPv3_QUERY *) buf;
      }
      query->type = IGMP_MEMBERSHIP_QUERY;

      if(group)
         query->group.s_addr = group->s_addr; 
      query->cksum = 0;
   }
   else
   {
      queryv12 = (t_IGMPv12_QUERY *) buf;
      queryv12->type = IGMP_MEMBERSHIP_QUERY;
      if(group)
         queryv12->group.s_addr = group->s_addr; 
      queryv12->cksum = 0;
   }

   /* Set the version specific fields */
   switch (version) 
   {
      case IGMP_VERSION_1:
         igmplen = sizeof(*queryv12);
         queryv12->code = 0;
         break;

      case IGMP_VERSION_2:
         igmplen = sizeof(*queryv12);
         if ( (!group) || (0 == group->s_addr) )
         {
            // No group address means a GMQ
            if (mcpd_router.igmp_config.query_resp_interval <= IGMP_MAX_RESPONSE_TIME)
            {
               queryv12->code = mcpd_router.igmp_config.query_resp_interval;
            }
            else
            {
               queryv12->code = IGMP_MAX_RESPONSE_TIME;
            }
         }
         else
         {
            // non-zero group address means a GSQ
            if (mcpd_router.igmp_config.lmqi <= IGMP_MAX_RESPONSE_TIME)
            {
               queryv12->code = mcpd_router.igmp_config.lmqi;
            }
            else
            {
               queryv12->code = IGMP_MAX_RESPONSE_TIME;
            }
         }         
         break;
      case IGMP_VERSION_3:
         igmplen = sizeof(*query);
         query->code = mcpd_router.igmp_config.query_resp_interval;
         query->resv = 0;
         query->qrv = mcpd_router.igmp_config.robust_val;
         if( (!group) || (0 == group->s_addr) )
         {
            query->code = mcpd_router.igmp_config.query_resp_interval;
         }
         else 
         {
            query->code = mcpd_router.igmp_config.lmqi;
         }
         if (SRSP == MCPD_TRUE) /*set supress router-side Processing*/
         {
            query->suppress = 1;
         }
         else
         {
            query->suppress = 0;
         }
         query->qqi = mcpd_router.igmp_config.query_interval;
         query->numsrc = htons(numsrc);

         for (i = 0; i < numsrc; i++)
         {
            query->sources[i].s_addr = sources[i].s_addr;
         }
         igmplen += (numsrc * sizeof(struct in_addr));
         break;

      default:
         if(pbuf)
            free(pbuf);
         return;
   }

   memset(&ipmreqn, 0, sizeof(ipmreqn));
   if((!group) || ((group) && group->s_addr == 0))
   {
      ipmreqn.imr_multiaddr.s_addr = htonl(INADDR_ALLHOSTS_GROUP);
   }
   else
   {
      ipmreqn.imr_multiaddr.s_addr = group->s_addr;
   }
   ipmreqn.imr_address.s_addr = ifp->if_addr.s_addr;
   ipmreqn.imr_ifindex = ifp->if_index;
   if(setsockopt(mcpd_router.sock_igmp, 
                 IPPROTO_IP, 
                 IP_MULTICAST_IF, 
                 (void*)&ipmreqn, 
                 sizeof(struct ip_mreqn)) < 0)
   {
      MCPD_TRACE(MCPD_TRC_ERR, "can't set IP_MULTICAST_IF");
   }

   /* Checksum */
   if(version == IGMP_VERSION_3)
   {
      query->cksum = mcpd_in_cksum((unsigned short*) query, igmplen);
   }
   else
   {
      queryv12->cksum = mcpd_in_cksum((unsigned short*) queryv12, igmplen);
   }

   sin.sin_family = AF_INET;
   if((!group) || ((group) && group->s_addr == 0))
      sin.sin_addr.s_addr = htonl(INADDR_ALLHOSTS_GROUP);
   else
      sin.sin_addr.s_addr = group->s_addr;

   MCPD_TRACE(MCPD_TRC_LOG, "Sending IGMPv%d query of len: %d", mcpd_router.igmp_config.default_version, igmplen);
   if(version == IGMP_VERSION_3)
   {
      if(sendto(mcpd_router.sock_igmp, (void*)query, igmplen, 0, 
                    (struct sockaddr *)&sin, sizeof(sin)) < 0)
      {
         MCPD_TRACE(MCPD_TRC_ERR, "sendto failed with err %d: %s", errno, strerror(errno));
      }
   }
   else
   {
      if(sendto(mcpd_router.sock_igmp, (void*)queryv12, igmplen, 0, 
                    (struct sockaddr *)&sin, sizeof(sin)) < 0)
      {
         MCPD_TRACE(MCPD_TRC_ERR, "sendto failed with err %d: %s", errno, strerror(errno));
      }
   }

   if(pbuf)
      free(pbuf);

   return;
} /* mcpd_igmp_membership_query */

void mcpd_igmp_receive_membership_query(t_MCPD_INTERFACE_OBJ *ifp __attribute__((unused)),
                                   struct in_addr *gp __attribute__((unused)),
                                   struct in_addr *sources __attribute__((unused)),
                                   struct in_addr *src_query __attribute__((unused)),
                                   int numsrc __attribute__((unused)),
                                   int srsp __attribute__((unused)) )
{
#if 0
   /* TODO: for now assume there is only one querier */
   t_MCPD_GROUP_OBJ *gr;
   t_MCPD_SRC_OBJ *src;
   int i;

   if (srsp == MCPD_FALSE)
   { /* Supress Router-Side Processing flag not set*/
      gr = mcpd_interface_group_lookup(MCPD_PROTO_IGMP, ifp, (UINT8 *)&gp);

      if (gr != NULL)
      {
         if (numsrc > 0)
         {
            /*group and source specific query*/
            for (i=0; i<numsrc; i++)
            {
               src = mcpd_group_src_lookup(MCPD_PROTO_IGMP,
                                           gr,
                                           (UINT8 *)&sources[i],
                                           MCPD_FMODE_INCLUDE);
            }
         }
      }
   }
#endif
   return;
} /* mcpd_igmp_receive_membership_query */


t_MCPD_RET_CODE mcpd_igmp_update_rep_timer(t_MCPD_REP_OBJ *rep, int timeMsecs)
{
   mcpd_timer_cancel(mcpd_igmp_timer_reporter, rep);
   if(NULL == mcpd_timer_new(timeMsecs,
                             mcpd_igmp_timer_reporter,
                             rep) )
   {
      MCPD_TRACE(MCPD_TRC_ERR, "mcpd_igmp_timer_reporter set failed");
   }

   return MCPD_RET_OK;
} /* mcpd_igmp_update_group_timer */

t_MCPD_RET_CODE mcpd_igmp_set_last_member_rep_timer(t_MCPD_REP_OBJ *rep)
{
   return mcpd_igmp_update_rep_timer (rep, mcpd_router.igmp_config.lmqi * mcpd_router.igmp_config.robust_val * (MSECS_IN_SEC/10));
}

t_MCPD_RET_CODE mcpd_igmp_reset_rep_timer(t_MCPD_REP_OBJ *rep)
{
   return mcpd_igmp_update_rep_timer (rep, mcpd_router.igmp_config.query_interval * mcpd_router.igmp_config.robust_val * MSECS_IN_SEC +
                                           mcpd_router.igmp_config.query_resp_interval * MSECS_IN_SEC / 10 );
}

t_MCPD_RET_CODE mcpd_igmp_update_source_timer(t_MCPD_SRC_OBJ *src)
{
   mcpd_timer_cancel (mcpd_igmp_timer_source, src);
   if(NULL == mcpd_timer_new(mcpd_router.igmp_config.query_interval * mcpd_router.igmp_config.robust_val * MSECS_IN_SEC +
                             mcpd_router.igmp_config.query_resp_interval * MSECS_IN_SEC / 10 ,
                             mcpd_igmp_timer_source,
                             src) )
   {
      MCPD_TRACE(MCPD_TRC_ERR, "mcpd_igmp_timer_source set failed");
   }

   return MCPD_RET_OK;
} /* mcpd_igmp_update_source_timer */

void mcpd_igmp_v2_bckcomp_tmr(void *handle)
{
    t_MCPD_GROUP_OBJ *gp = (t_MCPD_GROUP_OBJ *)handle;

    if(gp)
        gp->v2_host_prsnt_timer = MCPD_FALSE;

    return;
} /* mcpd_igmp_v2_bckcomp_tmr */

void mcpd_igmp_v1_bckcomp_tmr(void *handle)
{
    t_MCPD_GROUP_OBJ *gp = (t_MCPD_GROUP_OBJ *)handle;

    if(gp)
        gp->v1_host_prsnt_timer = MCPD_FALSE;

    return;
} /* mcpd_igmp_v1_bckcomp_tmr */

void mpcd_igmpv2_last_member_query_tmr(void *handle)
{
   t_MCPD_GROUP_OBJ *gp = (t_MCPD_GROUP_OBJ *) handle;
   MCPD_TRACE(MCPD_TRC_LOG, "lmqi=%d queriesLeft=%d", mcpd_router.igmp_config.lmqi, gp->leaveQueriesLeft);

   mcpd_igmp_membership_query(gp->ifp, (struct in_addr *)gp->addr, NULL, 0, 0, 1);
   gp->leaveQueriesLeft --;
   
   if (gp->leaveQueriesLeft <= 0)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "GSQs complete");
   }
   else
   {
      MCPD_TRACE(MCPD_TRC_LOG, "At least one more GSQ");
      if(NULL == mcpd_timer_new(mcpd_router.igmp_config.lmqi * (MSECS_IN_SEC/10),
                                mpcd_igmpv2_last_member_query_tmr,
                                gp) )
      {
         MCPD_TRACE(MCPD_TRC_ERR, "set failed");
      }    
   }
}

void mpcd_igmpv2_start_last_member_query_tmr(t_MCPD_INTERFACE_OBJ *ifp __attribute__((unused)),
                                             t_MCPD_GROUP_OBJ *gp)
{
   MCPD_TRACE(MCPD_TRC_LOG, "starting leave queries %d.%d.%d.%d",
              gp->addr[0],gp->addr[1],gp->addr[2],gp->addr[3]);
     
   gp->leaveQueriesLeft = mcpd_router.igmp_config.robust_val;
   mpcd_igmpv2_last_member_query_tmr ( gp );
}

void mcpd_igmp_leave_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                      t_MCPD_GROUP_OBJ *gp,
                                      UINT8 *srcs,
                                      int numsrc)
{
   struct in_addr *gp_addr = (struct in_addr *)(gp->addr);

   mcpd_igmp_membership_query(ifp, gp_addr, (struct in_addr *)srcs, numsrc, 0, 1);

   return;

} /* mcpd_igmp_leave_membership_query */

#if defined(BRCM_CMS_BUILD) && defined(DMP_X_BROADCOM_COM_GPON_1)
t_MCPD_RET_CODE mcpd_igmp_admission_control(int msg_type,
                                            int rxdev_ifi,
                                            UINT8 *gp,
                                            UINT8 *src,
                                            UINT8 *rep,
                                            unsigned short tci,
                                            UINT8 rep_proto_ver)
{
    t_MCPD_RET_CODE ret;
    ret = mcpd_omci_igmp_admission_control(msg_type, rxdev_ifi, gp, src, rep,
                                           tci, rep_proto_ver);
    return ret;

}
#else
t_MCPD_RET_CODE mcpd_igmp_admission_control(int msg_type __attribute__((unused)),
                                            int rxdev_ifi __attribute__((unused)),
                                            UINT8 *gp __attribute__((unused)),
                                            UINT8 *src __attribute__((unused)),
                                            UINT8 *rep __attribute__((unused)),
                                            unsigned short tci __attribute__((unused)),
                                            UINT8 rep_proto_ver __attribute__((unused)))
{
    return MCPD_RET_OK;
}
#endif

