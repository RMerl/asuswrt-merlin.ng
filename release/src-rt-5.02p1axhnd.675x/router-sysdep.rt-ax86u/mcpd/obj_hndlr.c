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
 * File Name  : obj_hndlr.c
 *
 * Description: API for object processing
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <bridgeutil.h>
#include "mcpd.h"
#include "common.h"
#include "igmp.h"
#include "igmp_main.h"
#include "mld.h"
#include "mld_main.h"
#include "mcpd_mroute.h"
#include "mcpd_mroute6.h"
#include "obj_hndlr.h"
#include "mcpd_nl.h"
#include "mcpd_timer.h"

extern t_MCPD_ROUTER mcpd_router;

t_MCPD_INTERFACE_OBJ *mcpd_interface_add(struct in_addr  *addr,
                                         struct in6_addr *addr6,
                                         char *name,
                                         int index,
                                         int if_type)
{
    t_MCPD_INTERFACE_OBJ *ifp = NULL;

    MCPD_TRACE(MCPD_TRC_INFO, "Adding if: %s with idx %d", name, index);

    ifp = mcpd_interface_lookup(index);
    if(ifp == NULL)
    {
        if((ifp = mcpd_interface_create(addr, addr6, name, index)))
        {
            ifp->next = mcpd_router.interfaces;
            mcpd_router.interfaces = ifp;
        }
    }
    else
    {
        if( (0 == mcpd_is_bridge(name)) &&
            (ifp->if_type != if_type))
        {
            /* cleanup all mcast forwarding entries for interface */
            if(bcm_mcast_api_if_change(mcpd_router.sock_nl, ifp->if_index, BCM_MCAST_PROTO_ALL) < 0)
            {
                MCPD_TRACE(MCPD_TRC_ERR, "Error while sending ifchange msg");
                return NULL;
            }
        }
    }

    if(ifp != NULL)
    {
        mcpd_interface_update(ifp, addr, addr6, name, index, if_type);
    }

    return ifp;
} /* mcpd_interface_add */

t_MCPD_REP_OBJ* mcpd_rep_create(t_MCPD_PROTO_TYPE proto, UINT8 *src_addr)
{
    t_MCPD_REP_OBJ* rep = NULL;
    int rep_obj_type = 0;
    int obj_type = 0;
    int addr_len = 0;

    if(proto == MCPD_PROTO_IGMP)
    {
        rep_obj_type = MCPD_IGMP_REP_OBJ;
        obj_type = MCPD_IPV4_ADDR_OBJ;
        addr_len = MCPD_IPV4_ADDR_SIZE;
    }
#ifdef SUPPORT_MLD
    else
    {
        rep_obj_type = MCPD_MLD_REP_OBJ;
        obj_type = MCPD_IPV6_ADDR_OBJ;
        addr_len = MCPD_IPV6_ADDR_SIZE;
    }
#endif

    if ((rep = (t_MCPD_REP_OBJ *) MCPD_ALLOC(rep_obj_type,
                                        sizeof(t_MCPD_REP_OBJ))))
    {
        bzero(rep, sizeof(t_MCPD_REP_OBJ));

        rep->addr = MCPD_ALLOC(obj_type, addr_len);
        if(rep->addr)
        {
            memcpy(rep->addr, src_addr, addr_len);
            rep->next  = NULL;
            rep->rsrc = NULL;
        }
        else
        {
            MCPD_FREE(rep_obj_type, rep);
            rep = NULL;
        }
    }

    return rep;
}  /* mcpd_rep_create */

int mcpd_rep_count(t_MCPD_GROUP_OBJ *gp)
{
    t_MCPD_REP_OBJ *re = NULL;
    int count = 0;

    MCPD_ASSERT(gp != NULL);

    if(!gp)
        return 0;

    for (re = gp->members ; re != NULL; re = (t_MCPD_REP_OBJ *)re->next)
        count++;

    return count;
} /* mcpd_rep_count */

t_MCPD_RET_CODE mcpd_compare_ipv4_addr(UINT8 const *ip_addr1, UINT8 const *ip_addr2)
{
    struct in_addr *addr1 = (struct in_addr *)ip_addr1;
    struct in_addr *addr2 = (struct in_addr *)ip_addr2;

    if(addr1->s_addr == addr2->s_addr) {
        return MCPD_RET_OK;
    }
    else
        return MCPD_RET_GENERR;
} /* mcpd_compare_ipv4_addr */

#ifdef SUPPORT_MLD
t_MCPD_RET_CODE mcpd_compare_ipv6_addr(UINT8 const *ip_addr1, UINT8 const *ip_addr2)
{
    struct in6_addr *addr61 = (struct in6_addr *)ip_addr1;
    struct in6_addr *addr62 = (struct in6_addr *)ip_addr2;

    if(IN6_ARE_ADDR_EQUAL(addr61, addr62))
        return MCPD_RET_OK;
    else
        return MCPD_RET_GENERR;

} /* mcpd_compare_ip_addr_obj */
#endif

void mcpd_rep_cleanup(t_MCPD_PROTO_TYPE proto,
                      t_MCPD_GROUP_OBJ *gp,
                      t_MCPD_REP_OBJ   *rep)
{
    t_MCPD_REP_OBJ     *re = NULL;
    t_MCPD_REP_SRC_OBJ *rsrc = NULL;
    t_MCPD_REP_SRC_OBJ *rsrc_next = NULL;
 
    MCPD_ASSERT(rep != NULL);
    MCPD_ASSERT(gp != NULL);

    if(!gp || !rep)
        return;

    if(!gp->members)
        return;

    /* clean up reporter timer */
    if (proto == MCPD_PROTO_IGMP)
    {
       mcpd_timer_cancel(mcpd_igmp_timer_reporter, rep);
    }
#ifdef SUPPORT_MLD
    else if (proto == MCPD_PROTO_MLD)
    {
       mcpd_timer_cancel(mcpd_mld_timer_reporter, rep);
    }
#endif

    if (rep != gp->members)
    {
        for(re = gp->members; 
            (t_MCPD_REP_OBJ *)re->next != rep;
                                 re = (t_MCPD_REP_OBJ *)re->next);

        /* remove the object from the list */
        re->next = rep->next;
    }
    else
    {
        /* delete the head */
        gp->members = (t_MCPD_REP_OBJ *)rep->next;
    }

    /* clean up all reporter source objects and related
       source reporter objects */
    for (rsrc = (t_MCPD_REP_SRC_OBJ *)rep->rsrc; rsrc != NULL; )
    {
        rsrc_next = (t_MCPD_REP_SRC_OBJ *)rsrc->next;
        mcpd_src_rep_cleanup(proto, rsrc->src, rep);
        mcpd_rep_src_cleanup(proto, rep, rsrc->src);
        rsrc = rsrc_next;
    }

    /* clean up the reporter object */
    if(proto == MCPD_PROTO_IGMP)
    {
        MCPD_FREE(MCPD_IPV4_ADDR_OBJ, rep->addr);
        MCPD_FREE(MCPD_IGMP_REP_OBJ, rep);
    }
    else
    {
        MCPD_FREE(MCPD_IPV6_ADDR_OBJ, rep->addr);
        MCPD_FREE(MCPD_MLD_REP_OBJ, rep);
    }

    return;
} /* mcpd_rep_cleanup */

t_MCPD_REP_OBJ* mcpd_group_rep_lookup(t_MCPD_PROTO_TYPE proto,
                                      t_MCPD_GROUP_OBJ *gp,
                                      UINT8 *src_addr)
{
    t_MCPD_REP_OBJ* rep = NULL;

    MCPD_ASSERT(gp != NULL);

    if(!gp)
        return NULL;

    if(!gp->members)
        return NULL;

    for (rep = gp->members; rep; rep = rep->next)
    {
        if ( MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](rep->addr, src_addr) )
        {
            return rep;
        }
    }

    return NULL;
} /* mcpd_group_rep_lookup */

t_MCPD_REP_OBJ* mcpd_group_rep_port_lookup(t_MCPD_PROTO_TYPE proto,
                                           t_MCPD_GROUP_OBJ *gp,
                                           UINT8 *src_addr,
                                           int rep_ifi)
{
    t_MCPD_REP_OBJ* rep = NULL;
    t_MCPD_RET_CODE   ret;

    MCPD_ASSERT(gp != NULL);

    if(!gp)
        return NULL;

    if(!gp->members)
        return NULL;

    for (rep = gp->members; rep; rep = rep->next)
    {
        if (rep->rep_ifi == rep_ifi)
        {
            ret = mcpd_router.cmp_ip_obj_func[proto](rep->addr, src_addr);
            if ( MCPD_RET_OK == ret )
            {
                return rep;
            }
        }
    }

    return NULL;
} /* mcpd_group_rep_lookup */


int mcpd_group_members_num_on_rep_port(t_MCPD_GROUP_OBJ *gp,UINT16 rep_ifi)
{
    t_MCPD_REP_OBJ* rep = NULL;
    int num = 0;
    MCPD_ASSERT(gp != NULL);

    if(!gp)
        return num;

    if(!gp->members)
        return num;

    for (rep = gp->members; rep; rep = rep->next)
    {
        if (rep->rep_ifi == rep_ifi)
        {
            num ++;
        }
    }

    return num;
} /* mcpd_group_members_num_on_rep_port */


void mcpd_wipe_reporter_for_old_port (t_MCPD_PROTO_TYPE proto,
                                      t_MCPD_OLD_REPORTER_TO_DELETE *reporterOnOldPort)
{       
    if(proto == MCPD_PROTO_IGMP)
    {
        t_MCPD_GROUP_OBJ *gp = reporterOnOldPort->ifp->igmp_proxy.groups;

        for ( ; gp; gp = gp->next)
        {
            t_MCPD_REP_OBJ *rep = gp->members;
            for (; rep; rep = rep->next) 
            {
                if (((mcpd_compare_ipv4_addr(rep->addr, (UINT8 *)&reporterOnOldPort->reporter_v4)) == MCPD_RET_OK) &&
                     (rep->rep_ifi == reporterOnOldPort->rep_ifi))
                {
                    mcpd_rep_cleanup(proto, gp, rep);
                    break;
                }
            }
        }

        MCPD_TRACE(MCPD_TRC_LOG, "send ipv4 purge reporter -%s- %s %d",
                           reporterOnOldPort->ifp->if_name,
                           inet_ntoa(reporterOnOldPort->reporter_v4),
                           reporterOnOldPort->rep_ifi);
        
        if ( bcm_mcast_api_ipv4_purge_reporter(mcpd_router.sock_nl,
                                               reporterOnOldPort->ifp->if_index,
                                               reporterOnOldPort->rep_ifi,
                                               &reporterOnOldPort->reporter_v4) < 0 )
        {
           MCPD_TRACE(MCPD_TRC_ERR, "failed to send ipv4 purge reporter");
        }
    }
#ifdef SUPPORT_MLD
    else if (proto == MCPD_PROTO_MLD)
    {
        t_MCPD_GROUP_OBJ *gp = reporterOnOldPort->ifp->mld_proxy.groups;

        for ( ; gp; gp = gp->next)
        {
            t_MCPD_REP_OBJ *rep = gp->members;
            for (; rep; rep = rep->next) 
            {
                if (((mcpd_compare_ipv6_addr(rep->addr, (UINT8 *)&reporterOnOldPort->reporter_v6)) == MCPD_RET_OK) &&
                     (rep->rep_ifi == reporterOnOldPort->rep_ifi))
                {
                    /* erase it */
                    MCPD_TRACE(MCPD_TRC_LOG, "Cleanup ordered");
                    mcpd_rep_cleanup(proto, gp, rep);
                }
            }
        }

        if ( bcm_mcast_api_ipv6_purge_reporter(mcpd_router.sock_nl,
                                               reporterOnOldPort->ifp->if_index,
                                               reporterOnOldPort->rep_ifi,
                                               &reporterOnOldPort->reporter_v6) < 0 )
        {
           MCPD_TRACE(MCPD_TRC_ERR, "failed to send ipv6 purge reporter");
        }
    }
#endif /* SUPPORT_MLD */
}

/* It is the caller's reponsibility to set reporterOnOldPort->valid to zero */
t_MCPD_REP_OBJ * mcpd_group_rep_add(t_MCPD_PROTO_TYPE proto,
                                    t_MCPD_GROUP_OBJ *gp,
                                    UINT8 *reporterAddr,
                                    int rep_ifi,
                                    t_MCPD_OLD_REPORTER_TO_DELETE *reporterOnOldPort)
{
    t_MCPD_REP_OBJ* rep = NULL;
    unsigned int num_reps = 0;

    MCPD_ASSERT(gp != NULL);

    if(!gp)
        return NULL;
  
    /* Return the source if it's already present */
    rep = mcpd_group_rep_port_lookup(proto, gp, reporterAddr, rep_ifi);
    if (rep)
    {
        /* the reporter is already subscribed, via the exact same port */
        return rep;
    }

    rep = mcpd_group_rep_lookup(proto, gp, reporterAddr);
    if (rep) {
        /* the reporter is subscribed, but via a different port (since the previous if failed) */
        /* Therefore, the reporter has moved and we flag the old port for later deletion*/
        if (proto == MCPD_PROTO_IGMP) {
            memcpy(&reporterOnOldPort->reporter_v4, reporterAddr, sizeof(reporterOnOldPort->reporter_v4) );
            reporterOnOldPort->rep_ifi = rep->rep_ifi;
            reporterOnOldPort->ifp = gp->ifp;
            reporterOnOldPort->valid = 1;
        }
#ifdef SUPPORT_MLD
        else if (proto == MCPD_PROTO_MLD) {
            memcpy(&reporterOnOldPort->reporter_v6, reporterAddr, sizeof(reporterOnOldPort->reporter_v6) );
            reporterOnOldPort->rep_ifi = rep->rep_ifi;
            reporterOnOldPort->ifp = gp->ifp;
            reporterOnOldPort->valid = 1;
        }
#endif
    }

    num_reps = mcpd_rep_count(gp);
    if(MCPD_PROTO_IGMP == proto)
    {
        if(num_reps >= mcpd_router.igmp_config.max_members)
        {
            MCPD_TRACE(MCPD_TRC_ERR, "Reached max. reporters");
            return NULL;
        }
    }
#ifdef SUPPORT_MLD
    else
    {
        if(num_reps >= mcpd_router.mld_config.max_members)
        {
            MCPD_TRACE(MCPD_TRC_ERR, "Reached max. reporters");
            return NULL;
        }
    }
#endif /* SUPPORT_MLD */

    /* Create the source and add to the set */
    rep = mcpd_rep_create(proto, reporterAddr);
    if (rep)
    {
        rep->parent_group = gp;
        rep->next = (t_MCPD_REP_OBJ *)gp->members;
        gp->members = rep;
        rep->rep_ifi = rep_ifi;
    }

    return rep;
} /* mcpd_group_rep_add */

t_MCPD_SRC_OBJ * mcpd_group_rep_lookup_src(t_MCPD_PROTO_TYPE proto __attribute__((unused)), 
                               t_MCPD_REP_OBJ *rep,
                               int filter)
{
    t_MCPD_REP_SRC_OBJ *rsrc = NULL;

    MCPD_ASSERT(rep != NULL);

    if(!rep)
        return NULL;

    if(!rep->rsrc)
        return NULL;

    for(rsrc = (t_MCPD_REP_SRC_OBJ *)rep->rsrc; rsrc != NULL;
                       rsrc = (t_MCPD_REP_SRC_OBJ *)rsrc->next)
    {
        if(filter == rsrc->src->fmode)
            return rsrc->src;
    }

    return NULL;
} /* mcpd_group_rep_lookup_src */

void mcpd_group_flood_action_get(t_MCPD_GROUP_OBJ *group,
                                                t_MCPD_PROTO_TYPE proto,
                                                int  *action)
{
    if (proto == MCPD_PROTO_IGMP)
    {  
        if (mcpd_router.igmp_config.flood_enable)
        {
            if (group->fmode == MCPD_FMODE_EXCLUDE)
            {
                *action = MCPD_SNOOP_EX_ADD;
            }
            else
            {
                *action = MCPD_SNOOP_IN_ADD;
            }
        }
        else
        {
            if (group->fmode == MCPD_FMODE_EXCLUDE)
            {
                *action = MCPD_SNOOP_EX_CLEAR;
            }
            else
            {
                *action = MCPD_SNOOP_IN_CLEAR;
            }
        }
    }
#ifdef SUPPORT_MLD
    else 
    {
        if (mcpd_router.mld_config.flood_enable)
        {
            if (group->fmode == MCPD_FMODE_EXCLUDE)
            {
                *action = BCM_MCAST_SNOOP_EX_ADD;
            }
            else
            {
                *action = BCM_MCAST_SNOOP_IN_ADD;
            }
        }
        else
        {
            if (group->fmode == MCPD_FMODE_EXCLUDE)
            {
                *action = BCM_MCAST_SNOOP_EX_CLEAR;
            }
            else
            {
                *action = BCM_MCAST_SNOOP_IN_CLEAR;
            }
        }
    }
#endif    
}/*mcpd_group_flood_action_get*/

t_MCPD_SRC_REP_OBJ* mcpd_src_rep_create(t_MCPD_PROTO_TYPE proto,
                            t_MCPD_REP_OBJ *rep)
{
    t_MCPD_SRC_REP_OBJ* srep = NULL;
    int obj_type;

    if(proto == MCPD_PROTO_IGMP)
        obj_type = MCPD_IGMP_SRC_REP_OBJ;
    else
        obj_type = MCPD_MLD_SRC_REP_OBJ;

    if((srep = (t_MCPD_SRC_REP_OBJ *)
                      MCPD_ALLOC(obj_type, sizeof(t_MCPD_SRC_REP_OBJ))))
    {
        bzero(srep, sizeof(t_MCPD_SRC_REP_OBJ));
        srep->rep = rep;
        srep->next = NULL;
    }

    return srep;
} /* mcpd_src_rep_create */

int mcpd_src_rep_count(t_MCPD_PROTO_TYPE proto __attribute__((unused)), t_MCPD_SRC_OBJ *src)
{
    t_MCPD_SRC_REP_OBJ *srep = NULL;
    int count=0;

    MCPD_ASSERT(src != NULL);

    if(!src)
        return count;

    for(srep = (t_MCPD_SRC_REP_OBJ *)src->srep; srep != NULL;
                           srep = (t_MCPD_SRC_REP_OBJ *)srep->next)
        count++;

    return count;
} /* mcpd_src_rep_count */

t_MCPD_SRC_REP_OBJ* mcpd_group_src_rep_lookup(t_MCPD_PROTO_TYPE proto,
                                  t_MCPD_SRC_OBJ* src,
                                  t_MCPD_REP_OBJ* rep)
{
    t_MCPD_SRC_REP_OBJ *srep = NULL;
    t_MCPD_RET_CODE     ret;

    MCPD_ASSERT(src != NULL);

    if(!src)
        return NULL;

    if(!src->srep)
        return NULL;

    for(srep = (t_MCPD_SRC_REP_OBJ *)src->srep; srep != NULL;
                      srep = (t_MCPD_SRC_REP_OBJ *)srep->next)
    {
        if ( srep->rep->rep_ifi == rep->rep_ifi )
        {
           ret = mcpd_router.cmp_ip_obj_func[proto](srep->rep->addr, rep->addr);
           if (MCPD_RET_OK == ret)
           {
              return srep;
           }
        }
    }

    return NULL;
} /* mcpd_group_src_rep_lookup */

void mcpd_src_rep_cleanup(t_MCPD_PROTO_TYPE proto,
                          t_MCPD_SRC_OBJ* src,
                          t_MCPD_REP_OBJ* rep)
{
    t_MCPD_SRC_REP_OBJ *re = NULL;
    t_MCPD_SRC_REP_OBJ *srep = NULL;

    MCPD_ASSERT(rep != NULL);
    MCPD_ASSERT(src != NULL);

    if(!rep || !src)
        return;

    if(!src->srep)
        return;

    srep = mcpd_group_src_rep_lookup(proto, src, rep);
    if(srep != NULL)
    {
        if (srep != src->srep)
        {
            for (re = (t_MCPD_SRC_REP_OBJ *)src->srep;
                   (t_MCPD_SRC_REP_OBJ *)re->next != srep;
              re = (t_MCPD_SRC_REP_OBJ *)re->next){}

                re->next = srep->next;
        }
        else
        {
            /*delete the head*/
            src->srep = srep->next;
        }

            if(proto == MCPD_PROTO_IGMP)
                MCPD_FREE(MCPD_IGMP_SRC_REP_OBJ, srep);
            else
                MCPD_FREE(MCPD_MLD_SRC_REP_OBJ, srep);
        }
} /* mcpd_src_rep_cleanup */

t_MCPD_SRC_REP_OBJ* mcpd_group_src_rep_add(t_MCPD_PROTO_TYPE proto,
                                   t_MCPD_SRC_OBJ *src,
                                   t_MCPD_REP_OBJ *rep)
{
    t_MCPD_SRC_REP_OBJ* srep = NULL;

    MCPD_ASSERT(src != NULL);

    if(!src)
        return NULL;

    if ((srep = mcpd_group_src_rep_lookup(proto, src, rep)))
        return srep;

    if ((srep = mcpd_src_rep_create(proto, rep)))
    {
        srep->next = (t_MCPD_SRC_REP_OBJ *)src->srep;
        src->srep = srep;
    }

    return srep;
} /* mcpd_group_src_rep_add */

t_MCPD_REP_SRC_OBJ* mcpd_rep_src_create(t_MCPD_PROTO_TYPE proto,
                            t_MCPD_SRC_OBJ *src)
{
    t_MCPD_REP_SRC_OBJ* rsrc = NULL;
    int obj_type;

    if(proto == MCPD_PROTO_IGMP)
        obj_type = MCPD_IGMP_REP_SRC_OBJ;
    else
        obj_type = MCPD_MLD_REP_SRC_OBJ;

    if ((rsrc = (t_MCPD_REP_SRC_OBJ *)
                     MCPD_ALLOC(obj_type, sizeof(t_MCPD_REP_SRC_OBJ))))
    {
        bzero(rsrc, sizeof(t_MCPD_REP_SRC_OBJ));
        rsrc->src = src;
        rsrc->next = NULL;
    }

    return rsrc;
} /* mcpd_rep_src_create */

int mcpd_rep_src_count(t_MCPD_PROTO_TYPE proto __attribute__((unused)), t_MCPD_REP_OBJ *rep)
{
    t_MCPD_REP_SRC_OBJ *rsrc = NULL;
    int count=0;

    MCPD_ASSERT(rep != NULL);

    if(!rep)
        return count;

    for (rsrc = (t_MCPD_REP_SRC_OBJ *)rep->rsrc; rsrc != NULL;
                     rsrc = (t_MCPD_REP_SRC_OBJ *)rsrc->next)
        count++;

    return count;
} /* mcpd_rep_src_count */

t_MCPD_REP_SRC_OBJ* mcpd_group_rep_src_lookup(t_MCPD_PROTO_TYPE proto,
                                  t_MCPD_REP_OBJ* rep,
                                  t_MCPD_SRC_OBJ* src)

{
    t_MCPD_REP_SRC_OBJ *rsrc = NULL;

    MCPD_ASSERT(src != NULL);
    MCPD_ASSERT(rep != NULL);

    if(!src || !rep)
        return NULL;

    for (rsrc = (t_MCPD_REP_SRC_OBJ *) rep->rsrc; rsrc != NULL;
                      rsrc = (t_MCPD_REP_SRC_OBJ *)rsrc->next)
    {
        if (MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](rsrc->src->addr,
                                                    src->addr))
            return rsrc;
    }

    return NULL;
} /* mcpd_group_rep_src_lookup */

void mcpd_rep_src_cleanup(t_MCPD_PROTO_TYPE proto,
              t_MCPD_REP_OBJ* rep,
              t_MCPD_SRC_OBJ* src)
{
    t_MCPD_REP_SRC_OBJ *re;
    t_MCPD_REP_SRC_OBJ *rsrc;

    MCPD_ASSERT(rep != NULL);
    MCPD_ASSERT(src != NULL);

    if(!rep || !src)
        return;

    if(!rep->rsrc)
        return;

    rsrc = mcpd_group_rep_src_lookup(proto, rep, src);
    if(rsrc != NULL)
    {
        if (rsrc != rep->rsrc)
        {
            for(re = (t_MCPD_REP_SRC_OBJ *)rep->rsrc;
                     (t_MCPD_REP_SRC_OBJ *)re->next != rsrc;
                re = (t_MCPD_REP_SRC_OBJ *)re->next){}

                re->next = rsrc->next;
        }
        else
        {
            /*delete the head*/
            rep->rsrc = (t_MCPD_REP_SRC_OBJ *)rsrc->next;
        }

        if(proto == MCPD_PROTO_IGMP)
            MCPD_FREE(MCPD_IGMP_REP_SRC_OBJ, rsrc);
        else
            MCPD_FREE(MCPD_MLD_REP_SRC_OBJ, rsrc);
        }

    return;
} /* mcpd_rep_src_cleanup */

t_MCPD_REP_SRC_OBJ* mcpd_group_rep_src_add(t_MCPD_PROTO_TYPE proto,
                               t_MCPD_REP_OBJ *rep,
                               t_MCPD_SRC_OBJ *src)
{
    t_MCPD_REP_SRC_OBJ *rsrc;

    MCPD_ASSERT(src != NULL);
    MCPD_ASSERT(rep != NULL);

    if(!src || !rep)
        return NULL;

    /* Return the source if it's already present */
    if((rsrc = mcpd_group_rep_src_lookup(proto, rep, src)))
        return rsrc;

    /* Create the source and add to the set */
    if ((rsrc = mcpd_rep_src_create(proto, src)))
    {
        rsrc->next = (t_MCPD_REP_SRC_OBJ *)rep->rsrc;
        rep->rsrc = rsrc;
    }

    return rsrc;
} /* mcpd_group_rep_src_add */

int mcpd_grp_src_count(t_MCPD_GROUP_OBJ *gp)
{
    t_MCPD_SRC_OBJ *src = NULL;
    int count = 0;

    MCPD_ASSERT(gp != NULL);

    if(!gp)
        return 0;

    for (src = gp->in_sources ; src != NULL; src = (t_MCPD_SRC_OBJ *)src->next)
        count++;

    for (src = gp->ex_sources ; src != NULL; src = (t_MCPD_SRC_OBJ *)src->next)
        count++;

    return count;
} /* mcpd_grp_src_count */

t_MCPD_SRC_OBJ * mcpd_src_create(t_MCPD_PROTO_TYPE proto, UINT8 *src_addr)
{
    t_MCPD_SRC_OBJ *src = NULL;
    int src_obj_type = 0;
    int ip_obj_type = 0;
    int addr_len = 0;

    if(proto == MCPD_PROTO_IGMP)
    {
        src_obj_type = MCPD_IGMP_SRC_OBJ;
        ip_obj_type = MCPD_IPV4_ADDR_OBJ;
        addr_len = MCPD_IPV4_ADDR_SIZE;
    }
#ifdef SUPPORT_MLD
    else
    {
        src_obj_type = MCPD_MLD_SRC_OBJ;
        ip_obj_type = MCPD_IPV6_ADDR_OBJ;
        addr_len = MCPD_IPV6_ADDR_SIZE;
    }
#endif

    if((src = (t_MCPD_SRC_OBJ *) MCPD_ALLOC(src_obj_type, sizeof(t_MCPD_SRC_OBJ))))
    {
        bzero(src, sizeof(t_MCPD_SRC_OBJ));

        if((src->addr = (UINT8 *)MCPD_ALLOC(ip_obj_type, addr_len)))
        {
            memcpy(src->addr , src_addr, addr_len);
            src->next  = NULL;
            src->srep   = NULL;
        }
        else
        {
            MCPD_FREE(src_obj_type, src);
            src = NULL;
        }
    }

    return src;
} /* mcpd_src_create */

void mcpd_src_cleanup(t_MCPD_PROTO_TYPE proto,
          t_MCPD_GROUP_OBJ *gp,
          t_MCPD_SRC_OBJ *src,
          int filter)
{
    t_MCPD_SRC_OBJ *sr;
    t_MCPD_SRC_REP_OBJ *srep = NULL;
    t_MCPD_SRC_REP_OBJ *srep_next = NULL;

    MCPD_ASSERT(src != NULL);
    MCPD_ASSERT(gp != NULL);

    if(!src || !gp)
        return ;

    if(proto == MCPD_PROTO_IGMP)
    {
        mcpd_timer_cancel(mcpd_igmp_timer_source, src);
    }
#ifdef SUPPORT_MLD
    else
    {
        mcpd_timer_cancel(mcpd_mld_timer_source, src);
    }
#endif /* SUPPORT_MLD */

    if(filter == MODE_IS_INCLUDE)
    {
        if(!gp->in_sources)
            return;

        if(src != gp->in_sources)
        {
            for(sr = gp->in_sources; 
                (t_MCPD_SRC_OBJ *)sr->next != src;
                                      sr = (t_MCPD_SRC_OBJ *)sr->next);

            sr->next = src->next;
            }
            else
            {
            /* delete the head */
            gp->in_sources = (t_MCPD_SRC_OBJ *)src->next;
            }
         }
        else
        {
        if(!gp->ex_sources)
            return;

        if (src != gp->ex_sources)
            {
            for (sr = gp->ex_sources; 
                (t_MCPD_SRC_OBJ *)sr->next != src;
                sr = (t_MCPD_SRC_OBJ *)sr->next);

            sr->next = src->next;
            }
            else
            {
            /*delete the head*/
            gp->ex_sources = (t_MCPD_SRC_OBJ *)src->next;
            }
        }

    /* remove multicast route */
    mcpd_router.chg_mfc_func[proto](src->addr, gp->addr);

                /* clean up all source reporter objects and related
                   reporter source objects */
                for (srep = (t_MCPD_SRC_REP_OBJ *)src->srep; srep != NULL; )
                {
                    srep_next = (t_MCPD_SRC_REP_OBJ *)srep->next;
                    mcpd_rep_src_cleanup(proto, srep->rep, src );
                    mcpd_src_rep_cleanup(proto, src, srep->rep);
                    srep = srep_next;
                }

    /* remove source object */
                if(proto == MCPD_PROTO_IGMP)
                {
                    MCPD_FREE(MCPD_IPV4_ADDR_OBJ, src->addr);
                    MCPD_FREE(MCPD_IGMP_SRC_OBJ, src);
                }
                else
                {
                    MCPD_FREE(MCPD_IPV6_ADDR_OBJ, src->addr);
                    MCPD_FREE(MCPD_MLD_SRC_OBJ, src);
                }
    return;
} /* mcpd_src_cleanup */

t_MCPD_SRC_OBJ * mcpd_group_src_lookup(t_MCPD_PROTO_TYPE proto,
                           t_MCPD_GROUP_OBJ *gp,
                           UINT8 *srcaddr,
                           int filter)
{
    t_MCPD_SRC_OBJ *src;

    if(filter == MCPD_FMODE_INCLUDE)
    {
        if (gp != NULL)
        {
            for(src = gp->in_sources; src != NULL;
                           src = (t_MCPD_SRC_OBJ *)src->next)
            {
                if(MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](src->addr,
                                                         srcaddr))
                    return src;
            }
        }
    }
    else
    {
        if (gp != NULL)
        {
            for(src = gp->ex_sources; src != NULL;
                           src = (t_MCPD_SRC_OBJ *)src->next)
            {
                if (MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](src->addr,
                                                          srcaddr))
                    return src;
            }
        }
    }

    return NULL;
} /* mcpd_group_src_lookup */

t_MCPD_SRC_OBJ * mcpd_group_src_add(t_MCPD_PROTO_TYPE proto,
                        t_MCPD_GROUP_OBJ *gp,
                        UINT8 *srcaddr,
                        int filter)
{
    t_MCPD_SRC_OBJ* src;
    unsigned int numsrc = 0;

    MCPD_ASSERT(gp != NULL);

    if(!gp)
        return NULL;

    numsrc = mcpd_grp_src_count(gp);

    if(filter == MCPD_FMODE_INCLUDE)
    {
        if((src = mcpd_group_src_lookup(proto, gp, srcaddr, filter)))
        return src;

        if(MCPD_PROTO_IGMP == proto)
        {
            if(numsrc >= mcpd_router.igmp_config.max_sources)
                return NULL;
        }
#ifdef SUPPORT_MLD
        else
        {
            if(numsrc >= mcpd_router.mld_config.max_sources)
                return NULL;
        }
#endif /* SUPPORT_MLD */

        if((src = mcpd_src_create(proto, srcaddr)))
        {
            src->fmode = MCPD_FMODE_INCLUDE;
            src->next = gp->in_sources;
            gp->in_sources = src;
            src->gp = gp;
        }
        return src;
    }
    else
    {
        if((src = mcpd_group_src_lookup(proto, gp, srcaddr, filter)))
            return src;

        if(MCPD_PROTO_IGMP == proto)
        {
            if(numsrc >= mcpd_router.igmp_config.max_sources)
                return NULL;
        }
#ifdef SUPPORT_MLD
        else
        {
            if(numsrc >= mcpd_router.mld_config.max_sources)
                return NULL;
        }
#endif /* SUPPORT_MLD */

        if ((src = mcpd_src_create(proto, srcaddr)))
        {
            src->fmode = MCPD_FMODE_EXCLUDE;
            src->next = gp->ex_sources;
            gp->ex_sources = src;
            src->gp = gp;
        }

        return src;
    }
} /* mcpd_group_src_add */

t_MCPD_GROUP_OBJ * mcpd_group_create(t_MCPD_PROTO_TYPE proto, UINT8 *groupaddr)
{
    t_MCPD_GROUP_OBJ *gp;
    t_MCPD_OBJ_TYPE  grp_obj_type = 0;
    t_MCPD_OBJ_TYPE  ip_obj_type = 0;
    int addr_len = 0;

    if(proto == MCPD_PROTO_IGMP)
    {
        grp_obj_type = MCPD_IGMP_GRP_OBJ;
        ip_obj_type = MCPD_IPV4_ADDR_OBJ;
        addr_len = MCPD_IPV4_ADDR_SIZE;
    }
#ifdef SUPPORT_MLD
    else
    {
        grp_obj_type = MCPD_MLD_GRP_OBJ;
        ip_obj_type = MCPD_IPV6_ADDR_OBJ;
        addr_len = MCPD_IPV6_ADDR_SIZE;
    }
#endif

    if ((gp = (t_MCPD_GROUP_OBJ *) MCPD_ALLOC(grp_obj_type,
                                      sizeof(t_MCPD_GROUP_OBJ))))
    {
        bzero(gp, sizeof(t_MCPD_GROUP_OBJ));

        gp->addr = (UINT8 *)MCPD_ALLOC(ip_obj_type, addr_len);
        if(gp->addr != NULL)
        {
            memcpy(gp->addr, groupaddr, addr_len);
            gp->fmode = MCPD_FMODE_EXCLUDE;
            if(proto == MCPD_PROTO_IGMP)
                gp->version = IGMP_VERSION_3;
            else
                gp->version = MLD_VERSION_2;
            gp->in_sources = NULL;
            gp->ex_sources = NULL;
            gp->members = NULL;
            gp->next = NULL;
            gp->v2_host_prsnt_timer = MCPD_FALSE;
            gp->v1_host_prsnt_timer = MCPD_FALSE;
        }
        else
        {
            MCPD_FREE(grp_obj_type, gp);
            gp = NULL;
        }

        return gp;
    }
    else
        return NULL;
} /* mcpd_group_create */

void mcpd_group_cleanup(t_MCPD_PROTO_TYPE proto,
            t_MCPD_INTERFACE_OBJ *ifp,
            t_MCPD_GROUP_OBJ* gp)
{
    t_MCPD_GROUP_OBJ *g;
    t_MCPD_OBJ_TYPE obj_type;
    t_MCPD_OBJ_TYPE addr_type;

    MCPD_ASSERT(gp != NULL);
    MCPD_ASSERT(ifp != NULL);

    if(!gp || !ifp)
        return ;

#ifdef SUPPORT_MLD
    if(proto == MCPD_PROTO_MLD)
    {
        obj_type = MCPD_MLD_GRP_OBJ;
        addr_type = MCPD_IPV6_ADDR_OBJ;
    }
    else
#endif
    {
        obj_type = MCPD_IGMP_GRP_OBJ;
        addr_type = MCPD_IPV4_ADDR_OBJ;
    }

    if(proto == MCPD_PROTO_IGMP)
    {
        if(NULL == ifp->igmp_proxy.groups)
            return;

        mcpd_timer_cancel (mpcd_igmpv2_last_member_query_tmr, gp);
        mcpd_timer_cancel (mcpd_igmp_timer_group, gp);
        mcpd_timer_cancel (mcpd_igmp_v2_bckcomp_tmr, gp);
        mcpd_timer_cancel (mcpd_igmp_v1_bckcomp_tmr, gp);
    }
#ifdef SUPPORT_MLD
    else
    {
        if(NULL == ifp->mld_proxy.groups)
            return;

        mcpd_timer_cancel (mpcd_mld_last_member_query_tmr, gp);
        mcpd_timer_cancel (mcpd_mld_timer_group, gp);
        mcpd_timer_cancel (mcpd_mld_v1_bckcomp_tmr, gp);
    }
#endif /* SUPPORT_MLD */

    if(proto == MCPD_PROTO_IGMP)
    {
        if(ifp->igmp_proxy.groups != gp)
        {
            g = ifp->igmp_proxy.groups;

            while((t_MCPD_GROUP_OBJ *)g->next != gp)
                g = (t_MCPD_GROUP_OBJ *)g->next;

            g->next = gp->next;
        }
        else
        {
            ifp->igmp_proxy.groups = (t_MCPD_GROUP_OBJ *)gp->next;
        }
    }
#ifdef SUPPORT_MLD
    else
    {
        if(ifp->mld_proxy.groups != gp)
        {
            g = ifp->mld_proxy.groups;
            while((t_MCPD_GROUP_OBJ *)g->next != gp)
                g = (t_MCPD_GROUP_OBJ *)g->next;

            g->next = gp->next;
        }
        else
        {
            ifp->mld_proxy.groups = (t_MCPD_GROUP_OBJ *)gp->next;
        }
    }
#endif /* SUPPORT_MLD */

     MCPD_FREE(addr_type, gp->addr);
     MCPD_FREE(obj_type, gp);

} /* mcpd_group_cleanup */

int mcpd_interface_update(t_MCPD_INTERFACE_OBJ  *ifp,
                                struct in_addr  *ifaddr,
                                struct in6_addr *ifaddr6,
                                char *ifname,
                                UINT16 index,
                                int if_type)
{
    short flags;

    strncpy(ifp->if_name, ifname, IFNAMSIZ);
    ifp->if_index = index;
    ifp->if_type = if_type;

    if (mcpd_is_bridge(ifname))
    {
        ifp->if_dir = MCPD_DOWNSTREAM;
    }
    else
    {
        ifp->if_dir = MCPD_UPSTREAM;
    }

    if ((ifaddr->s_addr != 0) ||
        (ifp->if_dir == MCPD_DOWNSTREAM))
    {
       ifp->igmp_proxy.is_querier = MCPD_TRUE;
       if(mcpd_router.igmp_config.default_version == 3)
           ifp->igmp_proxy.version = IGMP_VERSION_3;
       else if(mcpd_router.igmp_config.default_version == 2)
           ifp->igmp_proxy.version = IGMP_VERSION_2;
       else
           ifp->igmp_proxy.version = IGMP_VERSION_1;
       ifp->igmp_proxy.query_interval = mcpd_router.igmp_config.query_interval;
       ifp->igmp_proxy.query_resp_interval =
                         mcpd_router.igmp_config.query_resp_interval;
       ifp->igmp_proxy.rv =
                  mcpd_router.igmp_config.robust_val;
       ifp->igmp_proxy.gmi = ifp->igmp_proxy.rv * ifp->igmp_proxy.query_interval
                                   + ifp->igmp_proxy.query_resp_interval;
       ifp->igmp_proxy.ti_qi = 0;
       ifp->if_addr.s_addr = ifaddr->s_addr;
       mcpd_igmp_interface_init(ifp);
    }

#ifdef SUPPORT_MLD
    if (!IN6_IS_ADDR_UNSPECIFIED(ifaddr6) ||
        (ifp->if_dir == MCPD_DOWNSTREAM))
    {
       ifp->mld_proxy.is_querier = MCPD_TRUE;
       if(mcpd_router.mld_config.default_version == 2)
           ifp->mld_proxy.version = MLD_VERSION_2;
       else
           ifp->mld_proxy.version = MLD_VERSION_1;
       ifp->mld_proxy.query_interval = mcpd_router.mld_config.query_interval;
       ifp->mld_proxy.query_resp_interval =
                         mcpd_router.mld_config.query_resp_interval;
       ifp->mld_proxy.rv =
                  mcpd_router.mld_config.robust_val;
       ifp->mld_proxy.gmi = ifp->mld_proxy.rv * ifp->mld_proxy.query_interval
                                   + ifp->mld_proxy.query_resp_interval;
       ifp->mld_proxy.ti_qi = 0;
       memcpy(&ifp->if_addr6, ifaddr6, sizeof(struct in6_addr));
       mcpd_mld_interface_init(ifp);
    }
#endif

    /* Set the interface flags to receive all multicast packets */
    ifp->setFlag = 0;
    flags = mcpd_get_interface_flags(ifname);
    if ((flags != -1) && (0 == (flags & IFF_ALLMULTI)))
    {
        /* set IFF_ALLMULTI and make sure the set worked */
        mcpd_set_interface_flags(ifname, flags | IFF_ALLMULTI);
        flags = mcpd_get_interface_flags(ifname);
        if (0 == ((flags & IFF_ALLMULTI)))
        {
            MCPD_TRACE(MCPD_TRC_ERR, "Could not set IFF_ALLMULTI for %s", ifp->if_name);
        }
        else
        {
            ifp->setFlag = IFF_ALLMULTI;
        }
    }

    ifp->audit_done = MCPD_TRUE;

    return 1;

} /* mcpd_interface_update */

t_MCPD_INTERFACE_OBJ* mcpd_interface_create(struct in_addr  *ifaddr __attribute__((unused)),
                                            struct in6_addr *ifaddr6 __attribute__((unused)),
                                            char *ifname __attribute__((unused)),
                                            UINT16 index __attribute__((unused)))
{
    t_MCPD_INTERFACE_OBJ* ifp;

    if (((ifp = (t_MCPD_INTERFACE_OBJ *) MCPD_ALLOC(MCPD_INTERFACE_OBJ,
                            sizeof(t_MCPD_INTERFACE_OBJ)))) == NULL)
        return NULL;

    bzero(ifp, sizeof(t_MCPD_INTERFACE_OBJ));
    ifp->vifi = 0xFFFF;
#ifdef SUPPORT_MLD
    ifp->mifi = 0xFFFF;
#endif

    return ifp;
} /* mcpd_interface_create */

void mcpd_group_destroy(t_MCPD_PROTO_TYPE proto, t_MCPD_GROUP_OBJ *gp)
{
   if(!gp)
   {
      MCPD_TRACE(MCPD_TRC_LOG, "Group not specified");
      return;
   }

   while (gp->in_sources)
   {
      mcpd_src_cleanup(proto, gp, gp->in_sources, MODE_IS_INCLUDE);
   }

   while (gp->ex_sources)
   {
      mcpd_src_cleanup(proto, gp, gp->ex_sources, MODE_IS_EXCLUDE);
   }

   while (gp->members)
   {
      mcpd_rep_cleanup(proto, gp, gp->members);
   }
   
   mcpd_group_cleanup(proto, gp->ifp, gp);

   return;
} /* mcpd_group_destroy */

void mcpd_interface_cleanup(t_MCPD_INTERFACE_OBJ *ifp, int del, int flushSnoop, t_MCPD_PROTO_TYPE proto)
{
    t_MCPD_INTERFACE_OBJ *ifi;
    
    for(ifi = mcpd_router.interfaces; ifi; ifi = ifi->next)
    {
        if ((NULL == ifp) || (ifi == ifp))
        {
            if ((1 == del) || (proto != MCPD_PROTO_MLD))
            {
                if (ifi->if_dir == MCPD_DOWNSTREAM)
                {
                    while(ifi->igmp_proxy.groups)
                    {
                        mcpd_router.krnl_drop_membership_func[MCPD_PROTO_IGMP](NULL, ifi->igmp_proxy.groups);
                        mcpd_group_destroy(MCPD_PROTO_IGMP, ifi->igmp_proxy.groups);
                    }
                }
                else
                {
                    mcpd_cleanup_memberships(ifp, MCPD_PROTO_IGMP);
                }
            }
    
#ifdef SUPPORT_MLD
            if ((1 == del) || (proto != MCPD_PROTO_IGMP))
            {
                if (ifi->if_dir == MCPD_DOWNSTREAM)
                {
                    while(ifi->mld_proxy.groups)
                    {
                        mcpd_router.krnl_drop_membership_func[MCPD_PROTO_MLD](NULL, ifi->mld_proxy.groups);
                        mcpd_group_destroy(MCPD_PROTO_MLD, ifi->mld_proxy.groups);
                    }
                }
                else
                {
                    mcpd_cleanup_memberships(ifp, MCPD_PROTO_MLD);
                }
            }
#endif
            if ((1 == del) || (1 == flushSnoop))
            {
                int msgproto;
                if ( 1 == del )
                {
                    msgproto = BCM_MCAST_PROTO_ALL;
                }
                else
                {
                    if ( proto == MCPD_PROTO_MLD )
                    {
                       msgproto = BCM_MCAST_PROTO_IPV6;
                    }
                    else
                    {
                       msgproto = BCM_MCAST_PROTO_IPV4;
                    }
                }

                /* cleanup mcast forwarding entries for interface */
                if(bcm_mcast_api_if_change(mcpd_router.sock_nl, ifi->if_index, msgproto) < 0)
                {
                    MCPD_TRACE(MCPD_TRC_ERR, "Error while sending ifchange msg");
                }
            }

            if ( del )
            {
                /* If we managed to get/set the interface flags, revert */
                if (ifi->setFlag)
                {
                    short flags = mcpd_get_interface_flags(ifi->if_name);
                    flags &= ~ifi->setFlag;
                    mcpd_set_interface_flags(ifi->if_name, flags);
                }

                mcpd_igmp_krnl_proxy_del_vif(ifi->if_index, ifi->vifi);
#ifdef SUPPORT_MLD
                mcpd_mld_krnl_proxy_del_mif(ifi->if_index, ifi->mifi);
#endif
                MCPD_FREE(MCPD_INTERFACE_OBJ, ifi);
            }
        }
    }

    return;
} /* mcpd_interface_cleanup */

void mcpd_interface_refresh_groups(t_MCPD_INTERFACE_OBJ* ifp)
{
    t_MCPD_GROUP_OBJ *gp;

    MCPD_ASSERT(ifp != NULL);

    if(!ifp)
        return ;

    gp = ifp->igmp_proxy.groups;
    while(gp != NULL)
    {
        gp = gp->next;
    }

#ifdef SUPPORT_MLD
    gp = ifp->mld_proxy.groups;
    while(gp != NULL)
    {
        gp = gp->next;
    }
#endif
    return;
}

int mcpd_interface_group_count(t_MCPD_PROTO_TYPE proto,
                   t_MCPD_INTERFACE_OBJ *ifp)
{
    t_MCPD_GROUP_OBJ *gp = NULL;
    int count = 0;

    MCPD_ASSERT(ifp != NULL);

    if(!ifp)
        return count;

    if(proto == MCPD_PROTO_IGMP)
    {
        for (gp = ifp->igmp_proxy.groups; gp; gp = gp->next)
        count++;
    }
#ifdef SUPPORT_MLD
    else
    {
        for (gp = ifp->mld_proxy.groups; gp; gp = gp->next)
        count++;
    }
#endif /* SUPPORT_MLD */

    return count;
} /* mcpd_interface_group_count*/

static int mcpd_port_group_count_proto (t_MCPD_GROUP_OBJ* mcpdGroup, int rep_ifi)
{
   int groupCount = 0;
   while (mcpdGroup)
   {
      t_MCPD_REP_OBJ* reporter = mcpdGroup->members;
      while (reporter)
      {
         if (reporter->rep_ifi == rep_ifi) {
            groupCount ++;
            break;
         }
         reporter = reporter->next;
      }
      mcpdGroup = mcpdGroup->next;
   }
   return groupCount;
}

int mcpd_port_group_count(int rep_ifi, t_MCPD_INTERFACE_OBJ *ifp)
{
   int groupCount = 0;
   
   MCPD_TRACE(MCPD_TRC_INFO, "Counting groups on port number = %d\n", rep_ifi);
   groupCount += mcpd_port_group_count_proto (ifp->igmp_proxy.groups, rep_ifi); 
#ifdef SUPPORT_MLD
   groupCount += mcpd_port_group_count_proto (ifp->mld_proxy.groups, rep_ifi); 
#endif
   MCPD_TRACE(MCPD_TRC_INFO, "Found %d groups\n", groupCount);
   return groupCount;
}

int mcpd_port_max_group_count(int rep_ifi)
{
   int i = 0;

   /* See if the port is in the list of configured ports */
   for ( ; i < MAX_PORTS; i++)
   {
      if (mcpd_router.max_group_port_list[i].port_ifi == rep_ifi)
      {              
         return mcpd_router.max_group_port_list[i].maxGroupsForPort;
      }
   }
   return -1;
}

/*
 * Called when the group does not exist yet and we need to know if the
 * group can be created
 *
 * returns FALSE if either
 *        the port is at its limit for multicast groups (assuming limits are turned on)
 *        either IGMP or MLD is at its general limit
 */
 
t_MCPD_BOOL mcpd_is_there_space_for_group(t_MCPD_PROTO_TYPE proto, int rep_ifi, t_MCPD_INTERFACE_OBJ *ifp)
{
    unsigned int num_grps = mcpd_interface_group_count(proto, ifp);

    MCPD_TRACE(MCPD_TRC_INFO, "Checking with interface = %d", rep_ifi);

    if (mcpd_router.max_group_port_list[0].brPortName[0] != '\0')
    {
       int maxGroups = mcpd_port_max_group_count(rep_ifi);
       if (maxGroups != -1)
       {
          int groupCount = mcpd_port_group_count (rep_ifi, ifp);
          MCPD_TRACE(MCPD_TRC_INFO, "%d of %d used", groupCount, maxGroups);
          if (groupCount >= maxGroups)
          {
             return MCPD_FALSE;
          }
       }
    }

    if(MCPD_PROTO_IGMP == proto)
    {
        if (num_grps >= mcpd_router.igmp_config.max_groups)
        {
          return MCPD_FALSE;
        }
    }
#ifdef SUPPORT_MLD
    else
    {
       if(num_grps >= mcpd_router.mld_config.max_groups)
       {
          return MCPD_FALSE;
       }
    }
#endif /* SUPPORT_MLD */
    return MCPD_TRUE;
}

/*
 * Called when the group already exists and we need to find out if the
 * per-port multicast-group limit will allow or prevent adding the port to the group
 *
 * returns TRUE if any of these are true:
 *       the port is already in the group
 *       the port is under its limit
 *       the limits are not in effect (in general, or for this port)
 */

t_MCPD_BOOL mcpd_can_add_port_to_group(t_MCPD_GROUP_OBJ *gp, int rep_ifi, t_MCPD_INTERFACE_OBJ *ifp)
{
   t_MCPD_REP_OBJ* reporter = gp->members;
   int groupCount = 0;
   int maxGroups = 0;

   MCPD_TRACE(MCPD_TRC_INFO, "Checking with interface = %d", rep_ifi);

   if (mcpd_router.max_group_port_list[0].brPortName[0] == '\0')
   {
      return MCPD_TRUE;
   }
   
   while (reporter)
   {
      if (reporter->rep_ifi == rep_ifi) {
         MCPD_TRACE(MCPD_TRC_INFO, "Port already a member of group!");
         return MCPD_TRUE;
      }
      reporter = reporter->next;
   }

   /* At this point, we know the rep_ifi is not in the group */
   /* Check how many groups have that rep_ifi already */
   maxGroups = mcpd_port_max_group_count(rep_ifi); 
   if (maxGroups != -1) 
   {
      groupCount = mcpd_port_group_count (rep_ifi, ifp);
      MCPD_TRACE(MCPD_TRC_INFO, "%d of %d used", groupCount, maxGroups);
      if (groupCount >= maxGroups)
      {
         return MCPD_FALSE;
      }
   }
   return MCPD_TRUE;
}
  
t_MCPD_GROUP_OBJ* mcpd_interface_group_add(t_MCPD_PROTO_TYPE proto,
                               t_MCPD_INTERFACE_OBJ *ifp,
                               UINT8 *groupaddr,
                               int rep_ifi)
{
    t_MCPD_GROUP_OBJ* gp;

    MCPD_ASSERT(ifp != NULL);

    if(!ifp)
        return NULL;

    /* Return the group if it's already present */
    if ((gp = mcpd_interface_group_lookup(proto, ifp, groupaddr)))
    {
        if (mcpd_can_add_port_to_group(gp, rep_ifi, ifp) == MCPD_FALSE) {
           return NULL;
        }

        if (proto == MCPD_PROTO_IGMP)
        {
           mcpd_timer_cancel (mcpd_igmp_timer_group, gp);
        }
#ifdef SUPPORT_MLD
        else if (proto == MCPD_PROTO_MLD) 
        {
           mcpd_timer_cancel (mcpd_mld_timer_group, gp);
        }
#endif
        return gp;
    }

    if (MCPD_TRUE != mcpd_is_there_space_for_group(proto, rep_ifi, ifp) )
    {
        return NULL;
    }

    /* Create the group and add to the set */
    gp = mcpd_group_create(proto, groupaddr);

    if((proto == MCPD_PROTO_IGMP) && (gp))
    {
        gp->next = ifp->igmp_proxy.groups;
        ifp->igmp_proxy.groups = gp;
        gp->ifp = ifp;
    } 
#ifdef SUPPORT_MLD
    else if((proto == MCPD_PROTO_MLD) && (gp))
    {
        gp->next = ifp->mld_proxy.groups;
        ifp->mld_proxy.groups = gp;
        gp->ifp = ifp;
    }
#endif /* SUPPORT_MLD */

    return gp;
} /* mcpd_interface_group_add */

t_MCPD_GROUP_OBJ * mcpd_interface_group_lookup(t_MCPD_PROTO_TYPE proto,
                                   t_MCPD_INTERFACE_OBJ *ifp,
                                   UINT8 const *groupaddr)
{
    t_MCPD_GROUP_OBJ *gp = NULL;

    MCPD_ASSERT(ifp != NULL);

    if(!ifp)
        return NULL;

    if(proto == MCPD_PROTO_IGMP)
    {
        for (gp = ifp->igmp_proxy.groups; gp; gp = gp->next)
        {
            if(MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](gp->addr,
                                                          groupaddr))
            {
                return gp;
            }
        }
    }
#ifdef SUPPORT_MLD
    else
    {
        for (gp = ifp->mld_proxy.groups; gp; gp = gp->next)
        {
            if(MCPD_RET_OK == mcpd_router.cmp_ip_obj_func[proto](gp->addr,
                                                          groupaddr))
            {
                return gp;
            }
        }
    }
#endif /* SUPPORT_MLD */

    return NULL;
} /* mcpd_interface_group_lookup */

UINT8 mcpd_does_group_exist_anywhere (t_MCPD_PROTO_TYPE protocol, UINT8 const *groupAddress, UINT8 *isSsm)
{
    t_MCPD_INTERFACE_OBJ *ifp = NULL;

    if (!groupAddress || !isSsm)
    {
        return FALSE;
    }

    /* search all interfaces */
    for(ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
    {
        t_MCPD_GROUP_OBJ* group = mcpd_interface_group_lookup(protocol, ifp, groupAddress);
        if (group) 
        {
            MCPD_TRACE(MCPD_TRC_LOG, "Found matching group on interface %s", ifp->if_name);
            *isSsm = FALSE;
            if (!group->ex_sources && !group->in_sources)
            {
               return FALSE;
            }
            else if (group->in_sources) 
            {
                /* there is at least one include source */
                *isSsm = TRUE;
            }
            return TRUE;
        } else {
            MCPD_TRACE(MCPD_TRC_LOG, "No matching group on interface %s", ifp->if_name);
        }
    }
    return FALSE;
} /* mcpd_does_group_exist_anywhere */

t_MCPD_INTERFACE_OBJ *mcpd_interface_lookup(int if_index)
{
    t_MCPD_INTERFACE_OBJ *ifp = NULL;

    for(ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
        if(ifp->if_index == if_index)
            return ifp;

    return NULL;
} /* mcpd_interface_lookup */

t_MCPD_BOOL mcpd_is_valid_addr(t_MCPD_PROTO_TYPE proto, UINT8 *addr)
{
    struct in_addr *addr4;

    if(proto == MCPD_PROTO_IGMP)
    {
        addr4 = (struct in_addr *)addr;
        if((addr4->s_addr != htonl(INADDR_ALLRTRS_GROUP)) &&
           (addr4->s_addr != htonl(INADDR_ALLRTRS_IGMPV3_GROUP)) &&
           (addr4->s_addr != htonl(INADDR_ALLHOSTS_GROUP)) &&
           (addr4->s_addr != htonl(INADDR_DVMRP_RTRS_GROUP)) &&
           (addr4->s_addr != htonl(INADDR_RIP2_RTRS_GROUP)) )
            return MCPD_TRUE;
    }
#ifdef SUPPORT_MLD
    else
    {
        if(IN6_IS_ADDR_MULTICAST((struct in6_addr *)addr))
            return MCPD_TRUE;
    }
#endif /* SUPPORT_MLD */

    return MCPD_FALSE;
} /* mcpd_is_valid_addr */

void mcpd_cleanup_memberships(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_PROTO_TYPE proto)
{
    t_MCPD_INTERFACE_OBJ *ifd = NULL;
    t_MCPD_INTERFACE_OBJ *ifu = NULL;
    t_MCPD_GROUP_OBJ     *gp  = NULL;

    for(ifd = mcpd_router.interfaces; ifd; ifd = ifd->next)
    {
        if (proto != MCPD_PROTO_MLD)
        {
            if((ifd->if_dir == MCPD_DOWNSTREAM) &&
               (ifd->if_type & MCPD_IF_TYPE_ROUTED))
            {
                for(gp = ifd->igmp_proxy.groups; gp; gp = gp->next)
                {
                    for (ifu = mcpd_router.interfaces; ifu; ifu = ifu->next)
                    {
                        if ((NULL == ifp) || (ifu == ifp))
                        {
                            mcpd_router.krnl_drop_membership_func[MCPD_PROTO_IGMP](ifp, gp);
                        }
                    }
                }
            }
        }
#ifdef SUPPORT_MLD
        if (proto != MCPD_PROTO_IGMP)
        {
            if((ifd->if_dir == MCPD_DOWNSTREAM) &&
               (ifd->if_type & MCPD_IF_TYPE_ROUTED))
            {
                for(gp = ifd->mld_proxy.groups; gp; gp = gp->next)
                {
                    for (ifu = mcpd_router.interfaces; ifu; ifu = ifu->next)
                    {
                        if ((NULL == ifp) || (ifu == ifp))
                        {
                            mcpd_router.krnl_drop_membership_func[MCPD_PROTO_MLD](ifp, gp);
                        }
                    }
                }
            }
        }
#endif
    }
    return;
} /* mcpd_cleanup_memberships */

UINT32 mcpd_get_largest_timeout_secs (t_MCPD_GROUP_OBJ *gp, McpdEventHandler func)
{
   UINT32 largestTimeout = 0;
   t_MCPD_REP_OBJ *rep = gp->members;

   while (rep) {
      UINT32 timeout = mcpd_timer_getTimeRemaining_msec(func, rep);
      if (timeout > largestTimeout) {
         largestTimeout = timeout;
      }
      rep = rep->next;
   }

   return largestTimeout / MSECS_IN_SEC;
}  

void mcpd_dump_obj_tree(void)
{
    t_MCPD_INTERFACE_OBJ *ifp = NULL;
    t_MCPD_GROUP_OBJ *gp = NULL;
    t_MCPD_SRC_OBJ *src = NULL;
    t_MCPD_SRC_REP_OBJ *srep = NULL;
    t_MCPD_REP_OBJ *rep = NULL;
    t_MCPD_REP_SRC_OBJ *rsrc = NULL;
    UINT32 timeOut;

    printf("\nMCPD Object Tree Info\n");
    for(ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
    {
        printf("Interface: %s, Index: %d, Mode %s%s, Dir: %s, Proto: %s%s%s%s%s%s\n",
              ifp->if_name,
              ifp->if_index,
              ifp->if_type & MCPD_IF_TYPE_ROUTED            ? "Routed" : "",
              ifp->if_type & MCPD_IF_TYPE_BRIDGED           ? "Bridged" : "",
              ifp->if_dir == MCPD_DOWNSTREAM                ? "Downstream" : "Upstream",
              ifp->proto_enable & MCPD_IGMP_PROXY_ENABLE    ? "MCPD_IGMP_PROXY_ENABLE "    : "",
              ifp->proto_enable & MCPD_IGMP_SNOOPING_ENABLE ? "MCPD_IGMP_SNOOPING_ENABLE " : "",
              ifp->proto_enable & MCPD_MLD_PROXY_ENABLE     ? "MCPD_MLD_PROXY_ENABLE "     : "",
              ifp->proto_enable & MCPD_MLD_SNOOPING_ENABLE  ? "MCPD_MLD_SNOOPING_ENABLE "  : "",
              ifp->proto_enable & MCPD_IPV4_MCAST_ENABLE    ? "MCPD_IPV4_MCAST_ENABLE "    : "",
              ifp->proto_enable & MCPD_IPV6_MCAST_ENABLE    ? "MCPD_IPV6_MCAST_ENABLE "    : ""
              );
        for(gp = ifp->igmp_proxy.groups; gp; gp = gp->next)
        {
            UINT8 isSsm = 0;
            UINT8 groupOkay = 0;
            if (gp && gp->addr) {
                groupOkay = mcpd_does_group_exist_anywhere (MCPD_PROTO_IGMP, gp->addr, &isSsm);
            }
            printf("   Group %s, Filter Type %s, Version %d, Timeout %d Type %s\n"\
                   "                   v2HostPresent %d v1HostPresent %d\n",
                   inet_ntoa(*(struct in_addr *)gp->addr),
                   (gp->fmode == MCPD_FMODE_INCLUDE) ? "INCLUDE" : "EXCLUDE",
                   gp->version,
                   mcpd_get_largest_timeout_secs(gp, mcpd_igmp_timer_reporter),
                   (groupOkay == 0) ? "UNK" : (isSsm ? "SSM" : "ASM"),
                   gp->v2_host_prsnt_timer,
                   gp->v1_host_prsnt_timer);
            printf("      INLCUDE Sources\n");
            for(src = gp->in_sources; src; src = (t_MCPD_SRC_OBJ *)src->next)
            {
                timeOut = mcpd_timer_getTimeRemaining_msec(mcpd_igmp_timer_source, src);
                timeOut /= MSECS_IN_SEC;
                printf("      Src %s, Filter Type %s, Timeout %d\n",
                         inet_ntoa(*(struct in_addr *)src->addr),
                         (src->fmode == MCPD_FMODE_INCLUDE) ? "INCLUDE" : "EXCLUDE",
                         timeOut);
                for(srep = (t_MCPD_SRC_REP_OBJ *)src->srep; srep != NULL;
                                       srep = (t_MCPD_SRC_REP_OBJ *)srep->next)
                {
                    printf("      Src-Reporter %s\n",
                                    inet_ntoa(*(struct in_addr*)srep->rep->addr));
                }
            }

            printf("      EXCLUDE Sources\n");
            for (src = gp->ex_sources; src; src = (t_MCPD_SRC_OBJ *)src->next)
            {
                timeOut = mcpd_timer_getTimeRemaining_msec(mcpd_igmp_timer_source, src);
                timeOut /= MSECS_IN_SEC;
                printf("      Src %s, Filter Type %s, Timeout %d\n",
                         inet_ntoa(*(struct in_addr *)src->addr),
                         (src->fmode == MCPD_FMODE_INCLUDE) ? "INCLUDE" : "EXCLUDE",
                         timeOut);
                for(srep = (t_MCPD_SRC_REP_OBJ *)src->srep; srep != NULL;
                                        srep = (t_MCPD_SRC_REP_OBJ *)srep->next)
                {
                    printf("      Src-Reporter %s\n",
                                  inet_ntoa(*(struct in_addr*)srep->rep->addr));
                }
            }

            printf("         Group Reporters/Members\n");
            for (rep = (t_MCPD_REP_OBJ *) gp->members; rep;
                                     rep = (t_MCPD_REP_OBJ *)rep->next)
            {
                timeOut = mcpd_timer_getTimeRemaining_msec(mcpd_igmp_timer_reporter, rep);
                printf("         Reporter %s Version %d ifindex %d Timeout %d sec\n",
                             inet_ntoa(*(struct in_addr *)rep->addr),
                             rep->version,
                             rep->rep_ifi,
                             timeOut / MSECS_IN_SEC);

                for (rsrc = (t_MCPD_REP_SRC_OBJ *)rep->rsrc; rsrc != NULL;
                                 rsrc = (t_MCPD_REP_SRC_OBJ *)rsrc->next)
                {
                    printf("         Rep-Sources %s\n",
                                   inet_ntoa(*(struct in_addr *)rsrc->src->addr));
                }
            }
            printf("\n\n");
        }
#ifdef SUPPORT_MLD
        for(gp = ifp->mld_proxy.groups; gp; gp = gp->next)
        {
            UINT8 isSsm = 0;
            UINT8 groupOkay = 0;
            printf("   MLD Group ");
            mcpd_print_ipv6_addr((struct in6_addr *)gp->addr);

            if (gp && gp->addr) {
                groupOkay = mcpd_does_group_exist_anywhere (MCPD_PROTO_MLD, gp->addr, &isSsm);
            }
            printf("   Filter Type %s, Version %d, Timeout %d Type %s\n"\
                   "                   v1HostPresent %d\n",
                   (gp->fmode == MCPD_FMODE_INCLUDE) ? "INCLUDE" : "EXCLUDE",
                   gp->version,
                   mcpd_get_largest_timeout_secs(gp, mcpd_mld_timer_reporter),
                   (groupOkay == 0) ? "UNK" : (isSsm ? "SSM" : "ASM"),
                   gp->v1_host_prsnt_timer);
            printf("      INLCUDE Sources\n");
            for(src = gp->in_sources; src; src = (t_MCPD_SRC_OBJ *)src->next)
            {
                printf("      Src ");
                mcpd_print_ipv6_addr((struct in6_addr *)src->addr);

                timeOut = mcpd_timer_getTimeRemaining_msec(mcpd_mld_timer_source, src);
                timeOut /= MSECS_IN_SEC;
                printf("      Filter Type %s, Timeout %d\n",
                         (src->fmode == MCPD_FMODE_INCLUDE) ? "INCLUDE" : "EXCLUDE",
                         timeOut);
                for(srep = (t_MCPD_SRC_REP_OBJ *)src->srep; srep != NULL;
                                       srep = (t_MCPD_SRC_REP_OBJ *)srep->next)
                {
                    printf("      Src-Reporter ");
                    mcpd_print_ipv6_addr((struct in6_addr *)srep->rep->addr);
                }
            }

            printf("      EXCLUDE Sources\n");
            for (src = gp->ex_sources; src; src = (t_MCPD_SRC_OBJ *)src->next)
            {
                printf("      Src ");
                mcpd_print_ipv6_addr((struct in6_addr *)src->addr);

                timeOut = mcpd_timer_getTimeRemaining_msec(mcpd_mld_timer_source, src);
                timeOut /= MSECS_IN_SEC;
                printf("      Filter Type %s, Timeout %d\n",
                         (src->fmode == MCPD_FMODE_INCLUDE) ? "INCLUDE" : "EXCLUDE",
                         timeOut);
                for(srep = (t_MCPD_SRC_REP_OBJ *)src->srep; srep != NULL;
                                        srep = (t_MCPD_SRC_REP_OBJ *)srep->next)
                {
                    printf("      Src-Reporter ");
                    mcpd_print_ipv6_addr((struct in6_addr *)srep->rep->addr);
                }
            }

            printf("         Group Reporters/Members\n");
            for (rep = (t_MCPD_REP_OBJ *) gp->members; rep;
                                     rep = (t_MCPD_REP_OBJ *)rep->next)
            {
                timeOut = mcpd_timer_getTimeRemaining_msec(mcpd_mld_timer_reporter, rep);
                printf("         Reporter ");
                mcpd_print_ipv6_addr((struct in6_addr *)rep->addr);
                printf("         Version %d ifindex %d Timeout %d sec\n",
                             rep->version,
                             rep->rep_ifi,
                             timeOut / MSECS_IN_SEC);

                for (rsrc = (t_MCPD_REP_SRC_OBJ *)rep->rsrc; rsrc != NULL;
                                 rsrc = (t_MCPD_REP_SRC_OBJ *)rsrc->next)
                {
                    printf("         Rep-Sources ");
                    mcpd_print_ipv6_addr((struct in6_addr *)rsrc->src->addr);
                }
            }
            printf("\n\n");
        }
#endif /* SUPPORT_MLD */
        printf("\n\n");
    }
    return;
} /* mcpd_dump_obj_tree */
