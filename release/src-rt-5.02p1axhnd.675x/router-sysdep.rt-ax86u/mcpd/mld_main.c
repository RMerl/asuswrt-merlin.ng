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
 * File Name  : mld_main.c
 *
 * Description: API for MLD message processing
 *              
 ***************************************************************************/
#ifdef SUPPORT_MLD
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include "mcpd.h"
#include "common.h"
#include "mld.h"
#include "obj_hndlr.h"
#include "ssm_hndlr.h"
#include "mcpd_mroute6.h"
#include "mld_main.h"
#include "mld_proxy.h"
#include "mld_snooping.h"
#include "mcpd_nl.h"
#include "mcpd_main.h"
#include "common.h"
#include "bridgeutil.h"
#include "mcpd_timer.h"

extern t_MCPD_ROUTER mcpd_router;

void mpcd_mld_start_last_member_query_tmr(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_GROUP_OBJ *gp);

static int mcpd_mld_does_address_match_mask(const struct in6_addr *group, struct in6_addr *rule, struct in6_addr *mask)
{
  int index = 0;
  for ( ; index < 4 ; index ++)
  {
    if ( (group->s6_addr32[index] & mask->s6_addr32[index]) != (rule->s6_addr32[index] & mask->s6_addr32[index] ) )
    {
      return 0;
    }
  }
  return 1;
}

static int mcpd_mld_join_filter(const struct in6_addr *ipv6)
{
    if(!ipv6)
        return 0;

    if((!IN6_IS_ADDR_MULTICAST(ipv6)) ||
       (IN6_IS_ADDR_MC_SCOPE0(ipv6)) ||
       (IN6_IS_ADDR_MC_NODELOCAL(ipv6)) ||
       (IN6_IS_ADDR_MC_LINKLOCAL(ipv6)) ) 
    {
        return 0;
    }
    else
    {
        t_MCPD_FILTER_EXCEPTION *exceptionObj = mcpd_router.mld_config.filter_list;

        while (exceptionObj) {
            if ( mcpd_mld_does_address_match_mask(ipv6, &exceptionObj->address, &exceptionObj->mask) )
            {
                return 0;
            }
            exceptionObj = exceptionObj->next;
        }
    }
    return 1;
}

enum e_multicast_type mcpd_mld_getMulticastGroupType (unsigned int *address)
{
    if (mcpd_get_mcast_group_mode() == MULTICAST_MODE_FIRST_IN)
    {
        UINT8 isSsm = 0;
        if (mcpd_does_group_exist_anywhere (MCPD_PROTO_MLD, (UINT8*)address, &isSsm))
        {
            return isSsm ? MULTICAST_TYPE_SSM : MULTICAST_TYPE_ASM;
        }
        return MULTICAST_TYPE_UNK;
    }
    else 
    {
        if ( (address[0] & htonl(0xFFF0FFFF))  == htonl(0xFF300000) )
        {
            return MULTICAST_TYPE_SSM;
        }
        else 
        {
            return MULTICAST_TYPE_ASM;
        }
    }
}

t_MCPD_RET_CODE mcpd_mld_init(void)
{
    int optval;
    char ra[4];

    /* Set router alert */
    ra[0] = 148;
    ra[1] = 4;
    ra[2] = 0;
    ra[3] = 0;

    if(setsockopt(mcpd_router.sock_mld, IPPROTO_IPV6, IP_OPTIONS, ra, 4) < 0)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "can't set IP_OPTIONS");
        return MCPD_RET_GENERR;
    }

    /* Set reuseaddr, ttl and loopback */
    optval = 1; /* enable resuse addr option */
    if(setsockopt(mcpd_router.sock_mld, SOL_SOCKET, SO_REUSEADDR, 
                                     (void*)&optval, sizeof(optval)) < 0)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "can't set SO_REUSEADDR");
        return MCPD_RET_GENERR;
    }

    optval = 1; /* set number of hops to 1 */
    if(setsockopt(mcpd_router.sock_mld, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, 
                                     (void*)&optval, sizeof(optval))< 0)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "can't set IPV6_MULTICAST_HOPS");
        return MCPD_RET_GENERR;
    }

    optval = 0; /* disable loopback */
    if(setsockopt(mcpd_router.sock_mld, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, 
                                     (void*)&optval, sizeof(optval)) < 0)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "can't set IPV6_MULTICAST_LOOP");
        return MCPD_RET_GENERR;
    }

    if ( mcpd_mld_mroute_init() < 0 )
    {
        MCPD_TRACE(MCPD_TRC_LOG, "MRT6_INIT failed");
        return MCPD_RET_GENERR;
    }

    return MCPD_RET_OK;
} /* mcpd_mld_init */

t_MCPD_RET_CODE mcpd_mld_interface_init(t_MCPD_INTERFACE_OBJ *ifp)
{
    int i = 1;

    if(setsockopt(mcpd_router.sock_mld, 
                 IPPROTO_IPV6, 
                 IPV6_RECVPKTINFO, 
                 &i, 
                 sizeof(i)) < 0)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "can't set IPV6_RCVPKTINFO");
        return MCPD_RET_GENERR;
    }

    if (ifp->if_dir == MCPD_DOWNSTREAM ) 
    {
        struct ipv6_mreq mreq;
        mreq.ipv6mr_interface = ifp->if_index;

        /* Add membership for MLD_LL_V1_ALL_ROUTERS */
        if((inet_pton(AF_INET6, MLD_LL_V1_ALL_ROUTERS, 
                                          &mreq.ipv6mr_multiaddr)) == -1) 
        {
            MCPD_TRACE(MCPD_TRC_LOG, "can't create mcast address");
            return MCPD_RET_GENERR;
        }
        if(setsockopt(mcpd_router.sock_mld, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, 
                          (char *)&mreq, sizeof(struct ipv6_mreq) ) == -1 ) 
        {
            MCPD_TRACE(MCPD_TRC_LOG, "can't join mcast group");
            return MCPD_RET_GENERR;
        }

        /* Add membership for MLD_LL_V2_ALL_ROUTERS */
        if((inet_pton(AF_INET6, MLD_LL_V2_ALL_ROUTERS, 
                                           &mreq.ipv6mr_multiaddr)) == -1) 
        {
            MCPD_TRACE(MCPD_TRC_LOG, "can't create mcast address");
            return MCPD_RET_GENERR;
        }
        if(setsockopt(mcpd_router.sock_mld, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, 
                          (char *)&mreq, sizeof(struct ipv6_mreq) ) == -1 ) 
        {
            MCPD_TRACE(MCPD_TRC_LOG, "can't join mcast group");
            return MCPD_RET_GENERR;
        }
    }

    return MCPD_RET_OK;
} /* mcpd_mld_interface_init */

t_MCPD_RET_CODE mcpd_mld_process_input(t_MCPD_PKT_INFO *pkt_info)
{
    t_MLDv1_REPORT *mldv1;
    t_MLDv2_REPORT *mldv2;
    t_MCPD_INTERFACE_OBJ *ifp = NULL;
    t_MLDv2_QUERY *mld2_qry;
    t_MCPD_RET_CODE ret = MCPD_RET_OK;

#ifdef MCPD_DEBUG
    mcpd_display_mem_usage();
    mcpd_dump_obj_tree();
#endif

    if(!pkt_info)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "invalid argument\n");
        return MCPD_RET_GENERR;
    }

    if(pkt_info->data_len < (int) sizeof(*mldv1))
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

    mldv1 = (t_MLDv1_REPORT *)&pkt_info->pkt[0];
    MCPD_TRACE(MCPD_TRC_LOG, "MLD Pkt Rcvd from brif %d, if %d, type %d," \
                             "len %d, ip %08x:%08x:%08x:%08x, ifdir %d", 
                             pkt_info->parent_ifi, pkt_info->rxdev_ifi, 
                             mldv1->icmp6_hdr.icmp6_type, pkt_info->data_len,
                             pkt_info->ipv6rep.s6_addr32[0],
                             pkt_info->ipv6rep.s6_addr32[1],
                             pkt_info->ipv6rep.s6_addr32[2], 
                             pkt_info->ipv6rep.s6_addr32[3],
                             ifp->if_dir);


    if ( (0x00000000 == pkt_info->ipv6rep.s6_addr32[0]) &&
         (0x00000000 == pkt_info->ipv6rep.s6_addr32[1]) &&
         (0x00000000 == pkt_info->ipv6rep.s6_addr32[2]) &&
         (0x00000000 == pkt_info->ipv6rep.s6_addr32[3]) )
    {
       /* ignore this report */
       return MCPD_RET_GENERR;
    }

    /* don't process any messages from upstream in mcpd*/
    if(ifp->if_dir == MCPD_UPSTREAM)
    {
       MCPD_TRACE(MCPD_TRC_LOG, "Ignoring message from UPSTREAM if");
        return MCPD_RET_GENERR;
    }

    switch (mldv1->icmp6_hdr.icmp6_type) 
    {
        case ICMPV6_MLD_V1V2_QUERY:
            if(pkt_info->data_len == sizeof(*mldv1))
            {
                MCPD_TRACE(MCPD_TRC_LOG, "MLD v1 query: %08x:%08x:%08x:%08x", 
                                         ntohl(mldv1->grp_addr.s6_addr32[0]),
                                         ntohl(mldv1->grp_addr.s6_addr32[1]),
                                         ntohl(mldv1->grp_addr.s6_addr32[2]), 
                                         ntohl(mldv1->grp_addr.s6_addr32[3]));
            }
            else if(pkt_info->data_len >= (int) sizeof(*mld2_qry))
            {
                mld2_qry = (t_MLDv2_QUERY *)&pkt_info->pkt[0];
                MCPD_TRACE(MCPD_TRC_LOG, "MLD v2 query: %08x:%08x:%08x:%08x", 
                                         ntohl(mld2_qry->grp_addr.s6_addr32[0]),
                                         ntohl(mld2_qry->grp_addr.s6_addr32[1]),
                                         ntohl(mld2_qry->grp_addr.s6_addr32[2]), 
                                         ntohl(mld2_qry->grp_addr.s6_addr32[3]));
            }
            else 
            {
                MCPD_TRACE(MCPD_TRC_LOG, "Invalid query");
                ret = MCPD_RET_GENERR;
            }
            break;

        case ICMPV6_MLD_V1_REPORT:
            MCPD_TRACE(MCPD_TRC_LOG, "MLDv1 Report: %08x:%08x:%08x:%08x", 
                                     ntohl(mldv1->grp_addr.s6_addr32[0]),
                                     ntohl(mldv1->grp_addr.s6_addr32[1]),
                                     ntohl(mldv1->grp_addr.s6_addr32[2]), 
                                     ntohl(mldv1->grp_addr.s6_addr32[3]));
            ret = mcpd_mld_interface_membership_report_v1(ifp, 
                                                          &pkt_info->ipv6rep, 
                                                          mldv1, 
                                                          pkt_info->data_len, 
                                                          pkt_info);
            break;

        case ICMPV6_MLD_V1_DONE:
            MCPD_TRACE(MCPD_TRC_LOG, "MLDv1 Reduction: %08x:%08x:%08x:%08x", 
                                     ntohl(mldv1->grp_addr.s6_addr32[0]),
                                     ntohl(mldv1->grp_addr.s6_addr32[1]),
                                     ntohl(mldv1->grp_addr.s6_addr32[2]), 
                                     ntohl(mldv1->grp_addr.s6_addr32[3]));
            ret = mcpd_mld_interface_membership_leave_v1(ifp, 
                                                         &pkt_info->ipv6rep, 
                                                         mldv1,
                                                         pkt_info->data_len,
                                                         pkt_info);
            break;

        case ICMPV6_MLD_V2_REPORT:
            /* have full icmp header so no additional data needed at this point */
            mldv2 = (t_MLDv2_REPORT *)&pkt_info->pkt[0];
            MCPD_TRACE(MCPD_TRC_LOG, "MLDv2 report");
            ret = mcpd_mld_interface_membership_report_v2(ifp, 
                                                          &pkt_info->ipv6rep, 
                                                          mldv2, 
                                                          pkt_info->data_len, 
                                                          pkt_info);
            break;

        default:
            MCPD_TRACE(MCPD_TRC_LOG, "Unknown MLD message type");
            ret = MCPD_RET_GENERR;
            break;
    }

    return ret;
} /* mcpd_mld_process_input */

t_MCPD_RET_CODE mcpd_mld_interface_membership_report_v1(t_MCPD_INTERFACE_OBJ * ifp,
                                             struct in6_addr *src, 
                                             t_MLDv1_REPORT *report,
                                             int len __attribute__((unused)), 
                                             t_MCPD_PKT_INFO *pkt_info)
{
    t_MCPD_GROUP_OBJ *gp;
    t_MCPD_REP_OBJ *rep;
    t_MCPD_RET_CODE ret = MCPD_RET_OK;
    t_MCPD_OLD_REPORTER_TO_DELETE oldReporter;

    /* check if the group should be ignored */
    if (0 == mcpd_mld_join_filter(&report->grp_addr))
    {
        return MCPD_RET_GENERR;
    }

    if (mcpd_mld_getMulticastGroupType (report->grp_addr.s6_addr32)== MULTICAST_TYPE_SSM)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "Illegal MLD V1 Join for SSM address");
        return MCPD_RET_GENERR;        
    }

    /* Find the group, and if not present, add it */
    if((gp = mcpd_interface_group_add(MCPD_PROTO_MLD, 
                                      ifp, 
                                      (UINT8 *)&report->grp_addr,
                                      pkt_info->rxdev_ifi)) == NULL) 
    {
        return MCPD_RET_GENERR;
    }
        
    /* set the v1 host timer preset */
    if(report->icmp6_hdr.icmp6_type == ICMPV6_MLD_V1_REPORT)
    {
        gp->v1_host_prsnt_timer = MCPD_TRUE;

        mcpd_timer_cancel(mpcd_mld_last_member_query_tmr, gp);
        mcpd_timer_cancel(mcpd_mld_v1_bckcomp_tmr, gp);

        if(NULL == mcpd_timer_new(mcpd_router.mld_config.query_interval * mcpd_router.mld_config.robust_val * MSECS_IN_SEC +
                                  mcpd_router.mld_config.query_resp_interval * MSECS_IN_SEC / 10,
                                  mcpd_mld_v1_bckcomp_tmr, 
                                  gp) )
        {
            MCPD_TRACE(MCPD_TRC_LOG, "mcpd_mld_v1_bckcomp_tmr failed");
        }
    }

    gp->leaveQueriesLeft = -1; // We are not doing Leave Queries

    /* find the member and add it if not present*/
    oldReporter.valid = 0;
    rep = mcpd_group_rep_add(MCPD_PROTO_MLD, gp, (UINT8 *)src, pkt_info->rxdev_ifi, &oldReporter);

    if(!rep)
    {
       ret = MCPD_RET_GENERR;
       return ret;
    }
    rep->version = report->icmp6_hdr.icmp6_type;

    /* Consider this to be a v2 is_ex{} report */
    ret = mcpd_ssm_process_ex_filter(MCPD_PROTO_MLD,
                                     ifp, 
                                     gp, 
                                     0, 
                                     NULL, 
                                     rep, 
                                     pkt_info);
    /* rep may not be valid and should not be used beyond this point */

    if (oldReporter.valid) {
        mcpd_wipe_reporter_for_old_port(MCPD_PROTO_MLD, &oldReporter);
    }

    return ret;
} /* mcpd_mld_interface_membership_report_v1 */

t_MCPD_RET_CODE mcpd_mld_interface_membership_leave_v1(
                                            t_MCPD_INTERFACE_OBJ *ifp,
                                            struct in6_addr      *src,
                                            t_MLDv1_REPORT       *report,
                                            int                   len __attribute__((unused)),
                                            t_MCPD_PKT_INFO      *pkt_info)
{
   t_MCPD_GROUP_OBJ *gp  = NULL;
   t_MCPD_REP_OBJ   *rep = NULL;
   t_MCPD_RET_CODE   ret = MCPD_RET_OK;

   /* check if the group should be ignored */
   if (0 == mcpd_mld_join_filter(&report->grp_addr))
   {
      return MCPD_RET_GENERR;
   }
   
   if(NULL == ifp->mld_proxy.groups)
   {
      return MCPD_RET_GENERR;
   }

   gp = mcpd_interface_group_lookup(MCPD_PROTO_MLD, ifp, 
                                    (UINT8 *)&(report->grp_addr));
   if( gp )
   {
      if (!mcpd_router.mld_config.fast_leave_enable)
      {
         MCPD_TRACE(MCPD_TRC_LOG, "MLD Leave V1 rx'd with No Fast Leave"); 
         if ((gp->members != NULL) && (-1 == gp->leaveQueriesLeft))
         {
            // Set all reporters, for all ports, for this group to have the really short, Last Member Timeout
            for (rep = gp->members; rep; rep = rep->next)
            {
               mcpd_mld_set_last_member_rep_timer(rep);        
            }
            mpcd_mld_start_last_member_query_tmr(ifp, gp);
         }
      }
      else
      {
         rep = mcpd_group_rep_port_lookup(MCPD_PROTO_MLD, gp, (UINT8 *)src, pkt_info->rxdev_ifi);
         if (rep)
         {
            ret = mcpd_ssm_process_in_filter(MCPD_PROTO_MLD,
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
} /* mcpd_mld_interface_membership_leave_v1 */

t_MCPD_RET_CODE mcpd_mld_process_group_record(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in6_addr *src,
                                              t_MLDv2_REPORT *report,
                                              int len __attribute__((unused)),
                                              t_MCPD_PKT_INFO *pkt_info,
                                              t_MLD_GRP_RECORD *grp_rec)
{
    t_MCPD_GROUP_OBJ *gp = NULL;
    t_MCPD_REP_OBJ *rep = NULL;
    UINT16 num_src;
    UINT8 type;
    t_MCPD_RET_CODE ret = MCPD_RET_OK;
    UINT8 *sources = NULL;
    enum e_multicast_type reportType = MULTICAST_TYPE_ASM;
    enum e_multicast_type groupType = MULTICAST_TYPE_UNK;
    t_MCPD_OLD_REPORTER_TO_DELETE oldReporter;

    if(!ifp || !src || !report || !pkt_info || !grp_rec)
        return MCPD_RET_GENERR;

    /* check if the group should be ignored */
    if (0 == mcpd_mld_join_filter(&grp_rec->group))
    {
        return MCPD_RET_GENERR;
    }

    type   = (UINT8)grp_rec->type;
    num_src = ntohs(grp_rec->numsrc);

    MCPD_TRACE(MCPD_TRC_LOG, "group %08x:%08x:%08x:%08x, type %d, source count %d", 
                             ntohl(pkt_info->ipv6rep.s6_addr32[0]),
                             ntohl(pkt_info->ipv6rep.s6_addr32[1]),
                             ntohl(pkt_info->ipv6rep.s6_addr32[2]), 
                             ntohl(pkt_info->ipv6rep.s6_addr32[3]),
                             type, num_src);

    /* RFC4607/5771 - for MCAST SSM */
    if (num_src != 0 && ((MODE_TO_EXCLUDE == type) || (MODE_IS_EXCLUDE == type)) ) 
    {
        MCPD_TRACE(MCPD_TRC_ERR, 
                       "MLDV2 EXCLUDE messages may not contain sources (error in group report)");
        return MCPD_RET_GENERR;
    }

    if ((type == MODE_TO_INCLUDE) || (type == MODE_IS_INCLUDE))
    {
        /* May be SSM if there are sources */
        if (grp_rec->numsrc != 0)
        {
            MCPD_TRACE(MCPD_TRC_LOG, "SSM report rx'd group = %08x::", ntohl(grp_rec->group.s6_addr32[0]) );
            sources = (UINT8 *)&grp_rec->sources;
            reportType = MULTICAST_TYPE_SSM;
        }
        else
        {
            MCPD_TRACE(MCPD_TRC_LOG, "ASM/SSM report rx'd group = %08x::", ntohl(grp_rec->group.s6_addr32[0]) );
            /* Include with no sources - valid for both ASM and SSM */
            reportType = MULTICAST_TYPE_BOTH;
        }
    }    
    else if ((type == MODE_ALLOW_NEW_SRCS) || (type == MODE_BLOCK_OLD_SRCS))
    {
        MCPD_TRACE(MCPD_TRC_LOG, "SSM report rx'd group = %08x::", ntohl(grp_rec->group.s6_addr32[0]) );
        sources = (UINT8 *)&grp_rec->sources;
        reportType = MULTICAST_TYPE_SSM;        
    }

    groupType = mcpd_mld_getMulticastGroupType(grp_rec->group.s6_addr32);
    
    MCPD_TRACE(MCPD_TRC_LOG, "Group %08x:%08x:%08x:%08x is of type %s", 
               ntohl(grp_rec->group.s6_addr32[0]),ntohl(grp_rec->group.s6_addr32[1]),
               ntohl(grp_rec->group.s6_addr32[2]),ntohl(grp_rec->group.s6_addr32[3]),
               (groupType == MULTICAST_TYPE_UNK) ? "UNK" : 
                   ((groupType == MULTICAST_TYPE_ASM) ? "ASM" : "SSM"));

    if ((reportType != groupType) && (groupType != MULTICAST_TYPE_UNK) && (reportType != MULTICAST_TYPE_BOTH))
    {
        if (mcpd_get_mcast_group_mode() == MULTICAST_MODE_FIRST_IN) {
          MCPD_TRACE(MCPD_TRC_LOG, 
                         "Report type %s not allowed for group %08x:: that is already set up as type %s",
                         reportType == 0 ? "ASM" : "SSM", 
                         ntohl(grp_rec->group.s6_addr32[0]),
                         groupType == 0 ? "ASM" : "SSM");
        }
        else 
        {
          MCPD_TRACE(MCPD_TRC_LOG, 
                         "Report type %s not allowed for group %08x:: of type %s",
                         reportType == 0 ? "ASM" : "SSM", 
                         ntohl(grp_rec->group.s6_addr32[0]),
                         groupType == 0 ? "ASM" : "SSM");
        }
        return MCPD_RET_GENERR;
    }

    /* Find the group, and if not present, add it */
    if ((gp = mcpd_interface_group_add(MCPD_PROTO_MLD, 
                                         ifp, 
                                         (UINT8 *)&grp_rec->group,
                                         pkt_info->rxdev_ifi)) == NULL)
    {
        return MCPD_RET_GENERR;
    }

    /* find the source of the report and add it if not present*/
    oldReporter.valid = 0;
    rep = mcpd_group_rep_add(MCPD_PROTO_MLD, gp, (UINT8 *)src,pkt_info->rxdev_ifi, &oldReporter);
    if(!rep)
    {
        return MCPD_RET_GENERR;
    }
    rep->version = report->type;

    switch(type) 
    {
        case MODE_TO_INCLUDE:
        case MODE_IS_INCLUDE:
            ret = mcpd_ssm_process_in_filter(MCPD_PROTO_MLD,
                                             ifp, 
                                             gp, 
                                             num_src, 
                                             sources, 
                                             rep, 
                                             pkt_info);
            break;

        case MODE_TO_EXCLUDE:
        case MODE_IS_EXCLUDE:
            ret = mcpd_ssm_process_ex_filter(MCPD_PROTO_MLD,
                                             ifp, 
                                             gp, 
                                             num_src,
                                             sources, 
                                             rep, 
                                             pkt_info);
            break;

        case MODE_ALLOW_NEW_SRCS:
            ret = mcpd_ssm_process_allow_newsrc_filter(
                                         MCPD_PROTO_MLD,
                                         ifp, 
                                         gp, 
                                         num_src,
                                         sources,
                                         rep, 
                                         pkt_info);
            break;

        case MODE_BLOCK_OLD_SRCS: 
            ret = mcpd_ssm_process_block_oldsrc_filter(
                                         MCPD_PROTO_MLD,
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
        mcpd_wipe_reporter_for_old_port(MCPD_PROTO_MLD, &oldReporter);
    }

    return ret;
} /* mcpd_mld_process_group_record */

t_MCPD_RET_CODE mcpd_mld_interface_membership_report_v2(t_MCPD_INTERFACE_OBJ *ifp,
                                              struct in6_addr *src,
                                              t_MLDv2_REPORT *report,
                                              int len, 
                                              t_MCPD_PKT_INFO *pkt_info)
{
    UINT16 num_src;
    UINT16 num_grps;
    UINT8 i;
    t_MLD_GRP_RECORD *grp_rec = NULL;
    t_MCPD_RET_CODE ret = MCPD_RET_OK;
    unsigned int               remlen;
    unsigned int               grplen;

    if(!ifp || !src || !report || !pkt_info)
        return MCPD_RET_GENERR;

    num_grps = ntohs(report->numgrps);
    grp_rec = report->group;
    remlen   = pkt_info->data_len - sizeof(t_MLDv2_REPORT);
    for(i = 0; i < num_grps; i++)
    {
        if ( remlen < sizeof(*grp_rec) )
        {
            MCPD_TRACE(MCPD_TRC_ERR, "Invalid packet: not enough data for group record");
            return MCPD_RET_GENERR;
        }
        remlen -= sizeof(*grp_rec);
        num_src = ntohs(grp_rec->numsrc);

        if (remlen < (num_src * sizeof(struct in6_addr))) 
        {
            MCPD_TRACE(MCPD_TRC_ERR, "Invalid packet: not enough data for sources");
            return MCPD_RET_GENERR;
        }
        remlen -= (num_src * sizeof(struct in6_addr));
        grplen = sizeof(*grp_rec) + (num_src * sizeof(struct in6_addr));
        grp_rec = (t_MLD_GRP_RECORD *)((UINT8 *)grp_rec + grplen);
    }

    grp_rec = report->group;
    for(i = 0; i < num_grps; i++)
    {
        ret = mcpd_mld_process_group_record(ifp,
                                            src,
                                            report,
                                            len,
                                            pkt_info,
                                            grp_rec);
        num_src = ntohs(grp_rec->numsrc);
        grplen = sizeof(*grp_rec) + (num_src * sizeof(struct in6_addr));
        grp_rec = (t_MLD_GRP_RECORD *)((UINT8 *)grp_rec + grplen);
    }

    return ret;
} /* mcpd_mld_interface_membership_report_v2 */

void mcpd_mld_timer_querier(t_MCPD_INTERFACE_OBJ *ifp)
{
    if (ifp->mld_proxy.oqp > 0)
        --ifp->mld_proxy.oqp;
    if (ifp->mld_proxy.oqp == 0)
        ifp->mld_proxy.is_querier = MCPD_TRUE;

   return;
} /* mcpd_mld_timer_querier */

void mcpd_mld_timer_group(void *handle)
{
    t_MCPD_GROUP_OBJ *gp = (t_MCPD_GROUP_OBJ *)handle;
    if(gp)
    {
        mcpd_mld_krnl_drop_membership(NULL, gp);
        mcpd_group_destroy(MCPD_PROTO_MLD, gp);
    }
    return;
} /* mcpd_mld_timer_group */


void mcpd_mld_timer_reporter(void *handle)
{
   t_MCPD_REP_OBJ *rep = (t_MCPD_REP_OBJ *)handle;

   if(rep)
   {
      t_MCPD_GROUP_OBJ *gp = rep->parent_group;
      /* remove reporter from parent group */
      mcpd_rep_cleanup(MCPD_PROTO_MLD, gp, rep);
      
      if(mcpd_group_members_num_on_rep_port(gp, rep->rep_ifi) == 0)
      {
	      bcm_mcast_api_mld_drop_group(mcpd_router.sock_nl,
	                                   gp->ifp->if_index,
	                                   rep->rep_ifi,
	                                   (struct in6_addr *)(gp->addr));  
#if defined(CONFIG_BCM_OVS_MCAST)
          mcpd_ovs_manage_snoop_entry(rep->rep_ifi, 
                                      gp->ifp->if_index, 
                                      0,    /* v4 grp address N/A for MLD */
                                      gp->addr, 
                                      0,    /* v4src N/A for MLD*/
                                      NULL, /* v6src NULL for MLDv1 */
                                      0     /* 0 - remove snoop entry */
                                     );
#endif
      }
	  
      /* if no reporters remain, remove group */
      if (gp->members == NULL)
      {
          mcpd_mld_krnl_drop_membership(NULL, gp);
     
          mcpd_group_destroy(MCPD_PROTO_MLD, gp);
      }
   }
   return;
} /* mcpd_mld_timer_reporter */

void mcpd_mld_timer_source(void *handle)
{
    t_MCPD_GROUP_OBJ *gp = NULL;
    t_MCPD_SRC_OBJ *src = (t_MCPD_SRC_OBJ *)handle;

    if(src)
    {
        gp = src->gp;
        mcpd_src_cleanup(MCPD_PROTO_MLD, gp, src, src->fmode);
    }
    return;
} /* mcpd_mld_timer_source */

void mcpd_mld_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                struct in6_addr *group, 
                                struct in6_addr *sources, 
                                int numsrc, 
                                int SRSP,
                                int leave_qry)
{
    char buf[MLDV2_QUERY_SIZE], *pbuf = NULL;
    t_MLDv2_QUERY *mld_query;
    int offset = 2;
    int sockfd = mcpd_router.sock_mld;
    unsigned int ifindex = ifp->if_index;
    int version = ifp->mld_proxy.version;
    int mld_len, i;
    struct sockaddr_in6 dst;
    struct msghdr msgh;
    struct cmsghdr *cmsgh;
    struct iovec iovector;
    char control[128];
    char ra[8] = { IPPROTO_ICMPV6, 0, IP6OPT_PADN, 0,
                   IP6OPT_ROUTER_ALERT, 2, 0, 0 };
#if !defined(EPON_SFU)
    int mode;
    int l2lenable;
#endif

    if(ifp->if_dir == MCPD_UPSTREAM)
    {
        return;
    }

    if (0 == (ifp->proto_enable & MCPD_MLD_SNOOPING_ENABLE))
    {
       return;
    }

/* For epon wan, only check fast_leave flag */
#if !defined(EPON_SFU)
   bcm_mcast_api_get_snooping_cfg(ifp->if_index, BCM_MCAST_PROTO_IPV6, &mode, &l2lenable);
   if ( (0 == l2lenable) &&
        ((0 == (ifp->if_type & MCPD_IF_TYPE_ROUTED)) ||
         (mcpd_is_bridge_associated_with_mcast_wan_service(ifp, MCPD_PROTO_MLD) != MCPD_TRUE)) )
   {
      return;
   }
#endif
    if((leave_qry) && (mcpd_router.mld_config.fast_leave_enable))
        return;

    if (-1 == mcpd_check_ipv6_dad_status(ifp->if_index))
    {
        return;
    }

    /* Allocate a buffer to build the query */
    if (numsrc != 0 && version == MLD_VERSION_2)
    {
        mld_len = sizeof(*mld_query) + (numsrc * sizeof(struct in6_addr));
        pbuf = (char*) malloc(mld_len);
        if (pbuf == NULL)
            return;
        mld_query = (t_MLDv2_QUERY *)pbuf;
    }
    else
    {
        if (version == MLD_VERSION_2)
        {
            mld_len = MLDV2_QUERY_SIZE;
        }
        else
        {
            mld_len = MLDV1_QUERY_SIZE;
        }
        mld_query = (t_MLDv2_QUERY *) buf;
    }

    /* Set the common fields */
    memset( &dst, 0, sizeof(struct sockaddr_in6) );
    memset( &mld_query->grp_addr, 0, sizeof(struct in6_addr) );
    if((!group) || ((group) && (!IN6_IS_ADDR_MULTICAST(group))))
    {
        if((inet_pton(AF_INET6, MLD_LL_ALL_HOSTS, &dst.sin6_addr)) == -1) 
        {
            MCPD_TRACE(MCPD_TRC_LOG, "inet_pton: error");
        }
    }
    else
    {
        memcpy(&dst.sin6_addr, group, sizeof(struct in6_addr));
        memcpy(&mld_query->grp_addr, group, sizeof(struct in6_addr));
    }
    dst.sin6_family = AF_INET6;

    if( (!group) || (!IN6_IS_ADDR_MULTICAST(group)) )
    {
        mld_query->icmp6_hdr.icmp6_maxdelay = htons(mcpd_router.mld_config.query_resp_interval);
    }
    else
    {
        mld_query->icmp6_hdr.icmp6_maxdelay = htons(mcpd_router.mld_config.lmqi);
    }
    mld_query->icmp6_hdr.icmp6_data16[1] = htons(0);

    /* the MLD v1/v2 query message */
    switch(version)
    {
        case MLD_VERSION_1:
            /*do nothing*/
            break;
               
        case MLD_VERSION_2:
            mld_query->res = 0;
            mld_query->qrv = ifp->mld_proxy.rv;
            if (SRSP == MCPD_TRUE) /*set supress router-side Processing*/
               mld_query->suppress = 1;
            else
               mld_query->suppress = 0;
            mld_query->qqi = ifp->mld_proxy.query_interval;
            mld_query->num_srcs = htons(numsrc);

            for (i = 0; i < numsrc; i++)
            {
                memcpy(&mld_query->sources[i], &sources[i], sizeof(struct in6_addr));
            }
            break;

        default:
            if(pbuf)
                free(pbuf);
            return;
    }
    mld_query->icmp6_hdr.icmp6_type = ICMPV6_MLD_V1V2_QUERY;
    mld_query->icmp6_hdr.icmp6_code = 0;
    mld_query->icmp6_hdr.icmp6_cksum = 0;

    memset( &iovector, 0, sizeof(struct iovec) );
    iovector.iov_base = (void*)mld_query;
    iovector.iov_len = mld_len;

    memset(&msgh, 0, sizeof(struct msghdr));
    msgh.msg_name = &dst;
    msgh.msg_namelen = sizeof(struct sockaddr_in6);
    msgh.msg_iov = &iovector;
    msgh.msg_iovlen = 1;
    msgh.msg_control = control;
    msgh.msg_controllen = 128;

    /* set ancillary data options + copy hop-by-hop option */
    cmsgh = CMSG_FIRSTHDR(&msgh);
    cmsgh->cmsg_len = CMSG_LEN(8);
    cmsgh->cmsg_level = IPPROTO_IPV6;
    cmsgh->cmsg_type = IPV6_HOPOPTS;
    memcpy( (void *)CMSG_DATA(cmsgh), (void *)ra, sizeof(ra) );
    msgh.msg_controllen = cmsgh->cmsg_len;

    MCPD_TRACE(MCPD_TRC_LOG, "MLD QUERY: MRT is %02x:%02x ifindex=%d",
      mld_query->icmp6_hdr.icmp6_data8[0],
      mld_query->icmp6_hdr.icmp6_data8[1],
      ifindex);

    if(setsockopt(sockfd,
                 IPPROTO_IPV6, 
                 IPV6_MULTICAST_IF, 
                 &ifindex,
                 sizeof(ifindex)) < 0)
    {
        MCPD_TRACE(MCPD_TRC_LOG, "can't set IPV6_MULTICAST_IF");
        goto mld_buf_free;
    }

    /* ask kernel to compute ICMP checksum */
    if(setsockopt(sockfd, IPPROTO_IPV6,IPV6_CHECKSUM, &offset, 
                                                            sizeof(offset)) < 0)
    {  
        MCPD_TRACE(MCPD_TRC_LOG, "IPV6_CHECKSUM: failed");
        goto mld_buf_free;
    }

    MCPD_TRACE(MCPD_TRC_LOG, "Sending Query size: %d", mld_len);
    mcpd_dump_buf((char *)mld_query, mld_len);

    if((sendmsg(sockfd, &msgh, MSG_DONTROUTE )) <= 0) 
    {
        MCPD_TRACE(MCPD_TRC_ERR, "sendmsg failed with err %d: %s", errno, strerror(errno));
    }

mld_buf_free:
    if(pbuf)
        free(pbuf);
    return;
} /* mcpd_mld_membership_query */

void mcpd_mld_receive_membership_query(t_MCPD_INTERFACE_OBJ *ifp __attribute__((unused)),
                                   struct in6_addr gp __attribute__((unused)),
                                   struct in6_addr *sources __attribute__((unused)),
                                   UINT32 src_query __attribute__((unused)),
                                   int numsrc __attribute__((unused)),
                                   int srsp __attribute__((unused)))
{
#if 0
    /* TODO: for now assume there is only one querier */
    t_MCPD_GROUP_OBJ *gr;
    t_MCPD_SRC_OBJ *src;
    int i;

    if (src_query < ifp->mldi_addr.s6_addr){ /* another querier is present with lower IP adress*/
        ifp->mldi_oqp= mcpd_router.mld_config.query_interval * mcpd_router.mld_config.robust_val * MSECS_IN_SEC +
                       mcpd_router.mld_config.query_resp_interval * MSECS_IN_SEC / 2 / 10;
        ifp->mldi_isquerier = FALSE;
    }

    if (srsp == MCPD_FALSE)
    { 
        /* Supress Router-Side Processing flag not set*/
        gr = mcpd_interface_group_lookup(MCPD_PROTO_MLD, ifp, (UINT8 *)&gp);

        if (gr != NULL)
        {
            if (numsrc > 0)
            {
                /*group and source specific query*/
                for (i=0;i < numsrc; i++)
                {
                    src = mcpd_group_src_lookup(MCPD_PROTO_MLD, 
                                           gr, 
                                           (UINT8 *)&sources[i], 
                                           MCPD_FMODE_INCLUDE);
                }
            }
        }
    }
#endif
    return;
} /* mcpd_mld_receive_membership_query */

t_MCPD_RET_CODE mcpd_mld_update_rep_timer(t_MCPD_REP_OBJ *rep, int timeMsecs)
{
    mcpd_timer_cancel(mcpd_mld_timer_reporter, rep);
    if(NULL == mcpd_timer_new(timeMsecs, 
                              mcpd_mld_timer_reporter,
                              rep) )
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_mld_timer_reporter set failed");
    }

    return MCPD_RET_OK;
} /* mcpd_mld_update_group_timer */

t_MCPD_RET_CODE mcpd_mld_set_last_member_rep_timer(t_MCPD_REP_OBJ *rep)
{
  return mcpd_mld_update_rep_timer (rep, mcpd_router.mld_config.lmqi * mcpd_router.mld_config.robust_val * (MSECS_IN_SEC/10));
}

t_MCPD_RET_CODE mcpd_mld_reset_rep_timer(t_MCPD_REP_OBJ *rep)
{
  return mcpd_mld_update_rep_timer (rep,  mcpd_router.mld_config.query_interval * mcpd_router.mld_config.robust_val * MSECS_IN_SEC +
                                          mcpd_router.mld_config.query_resp_interval * MSECS_IN_SEC / 10 );
}

t_MCPD_RET_CODE mcpd_mld_update_source_timer(t_MCPD_SRC_OBJ *src)
{
    mcpd_timer_cancel(mcpd_mld_timer_source, src);
    if(NULL == mcpd_timer_new(mcpd_router.mld_config.query_interval * mcpd_router.mld_config.robust_val * MSECS_IN_SEC +
                              mcpd_router.mld_config.query_resp_interval * MSECS_IN_SEC / 10, 
                              mcpd_mld_timer_source,
                              src) )
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_mld_timer_source set failed");
    }

    return MCPD_RET_OK;
} /* mcpd_mld_update_source_timer */

void mcpd_mld_v1_bckcomp_tmr(void *handle)
{
    t_MCPD_GROUP_OBJ *gp = (t_MCPD_GROUP_OBJ *)handle;

    gp->v1_host_prsnt_timer = MCPD_FALSE;

    return;
} /* mcpd_mld_v1_bckcomp_tmr */

void mpcd_mld_last_member_query_tmr(void *handle)
{
   t_MCPD_GROUP_OBJ *gp = (t_MCPD_GROUP_OBJ *) handle;
   MCPD_TRACE(MCPD_TRC_LOG, "lmqi=%d queriesLeft=%d", mcpd_router.mld_config.lmqi, gp->leaveQueriesLeft);

   mcpd_mld_membership_query(gp->ifp, (struct in6_addr *)(gp->addr), NULL, 0, 0, 1);
   gp->leaveQueriesLeft --;
   
   if (gp->leaveQueriesLeft <= 0)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "GSQs complete");
   }
   else
   {
      MCPD_TRACE(MCPD_TRC_LOG, "At least one more GSQ");
      if(NULL == mcpd_timer_new(mcpd_router.mld_config.lmqi * (MSECS_IN_SEC/10),
                                mpcd_mld_last_member_query_tmr,
                                gp) )
      {
         MCPD_TRACE(MCPD_TRC_ERR, "set failed");
      }    
   }
}

void mpcd_mld_start_last_member_query_tmr(t_MCPD_INTERFACE_OBJ *ifp __attribute__((unused)),
                                          t_MCPD_GROUP_OBJ *gp)
{     
   gp->leaveQueriesLeft = mcpd_router.mld_config.robust_val;
   mpcd_mld_last_member_query_tmr ( gp );
}


void mcpd_mld_leave_membership_query(t_MCPD_INTERFACE_OBJ *ifp,
                                     t_MCPD_GROUP_OBJ *gp,
                                     UINT8 *srcs,
                                     int numsrc)
{
    struct in6_addr *gp_addr = (struct in6_addr *)(gp->addr);

    mcpd_mld_membership_query(ifp, gp_addr, (struct in6_addr *)srcs, numsrc, 0, 1);

    return;

} /* mcpd_mld_leave_membership_query */

/** determine if the join request is allowed
 *
 * @param msg_type        (IN) message type
 *
 * @param rxdev_ifi       (IN) receive interface
 *
 * @param *gp             (IN) group address
 *
 * @param *src            (IN) multicast source address
 *
 * @param *rep            (IN) reporter address
 *
 * @param tci             (IN) vlan
 *
 * @param rep_proto_ver   (IN) IGMP message type
 *
 * @return t_MCPD_RET_CODE
 *
 */
t_MCPD_RET_CODE mcpd_mld_admission_control(int msg_type __attribute__((unused)),
                                           int rxdev_ifi __attribute__((unused)),
                                           UINT8 *gp __attribute__((unused)),
                                           UINT8 *src __attribute__((unused)),
                                           UINT8 *rep __attribute__((unused)),
                                           unsigned short tci __attribute__((unused)),
                                           UINT8 rep_proto_ver __attribute__((unused)))
{
    return MCPD_RET_OK;
}

#endif /* SUPPORT_MLD */
