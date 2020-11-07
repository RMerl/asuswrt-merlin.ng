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
 * File Name  : mcpd_main.c
 *
 * Description: API for mcpd init and run
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include "mcpd.h"
#include "common.h"
#include "igmp.h"
#include "obj_hndlr.h"
#include "ssm_hndlr.h"
#include "mcpd_mroute.h"
#include "mcpd_mroute6.h"
#include "igmp_snooping.h"
#include "igmp_proxy.h"
#include "igmp_main.h"
#include "mld.h"
#include "mld_main.h"
#include "mld_proxy.h"
#include "mld_snooping.h"
#include "mcpd_main.h"
#include "mcpd_config.h"
#include "mcpd_nl.h"
#include "mcpd_timer.h"
#include "mcpd_cms.h"
#include "mcpd_ctl.h"
#if defined(CONFIG_BCM_OVS_MCAST)
#include "mcpd_ovs.h"
#endif

#include <bcm_mcast_api.h>

t_MCPD_ROUTER mcpd_router;
char mcpd_igmp_upstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
char mcpd_igmp_downstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
char mcpd_igmp_mcast_interface[MCPD_MAX_IFS][IFNAMSIZ];
char mcpd_strict_wan_associations[MCPD_MAX_IFS][IFNAMSIZ * 2 + 1];
#ifdef SUPPORT_MLD
char mcpd_mld_upstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
char mcpd_mld_downstream_interface[MCPD_MAX_IFS][IFNAMSIZ];
char mcpd_mld_mcast_interface[MCPD_MAX_IFS][IFNAMSIZ];
#endif
t_MCPD_OBJ_TYPE mcpd_malloced_objs[MCPD_MAX_OBJ];

int mcpd_main_quit;

static enum e_multicast_group_mode mcastGroupMode = IANA_MULTICAST_DEFAULT_GROUP_MODE;

enum e_multicast_group_mode mcpd_get_mcast_group_mode(void)
{
    return mcastGroupMode;
}

void mcpd_display_group_mode(void)
{
    printf("\nMulticast Group Mode set to: %s\n", 
           (mcastGroupMode == MULTICAST_MODE_IANA) ? "IANA" : "FIRST IN");
}

void mcpd_setMcastGroupMode(enum e_multicast_group_mode newMode)
{
    if (newMode < MULTICAST_MODE_INVALID)
    {
        mcastGroupMode = newMode;
    }
    mcpd_display_group_mode();
}

void mcpd_drain_message(int sock)
{
    struct sockaddr from;
    socklen_t len = sizeof(struct sockaddr);
    int nbytes;

    nbytes = recvfrom(sock,
                     mcpd_router.sock_buff,
                     BCM_MCAST_NL_RX_BUF_SIZE,
                     MSG_DONTWAIT,
                     &from,
                     &len);

    if(nbytes < 0)
        MCPD_TRACE(MCPD_TRC_LOG, "error while draining socket");

    return;

} /* mcpd_drain_message */

void mcpd_input_run(void)
{
    fd_set rfds;
    int n, maxfd;
    struct timeval time_out;
    struct timeval *ptime;

    ptime = mcpd_timer_timeToNextEvent_msec(&time_out);

    /* Add the sockets to the set */
    FD_ZERO(&rfds);

#ifdef BRCM_CMS_BUILD
    maxfd = mcpd_cms_get_fd();
    FD_SET(maxfd, &rfds);
#else
    maxfd = -1;
#endif // BRCM_CMS_BUILD

    FD_SET(mcpd_router.sock_nl, &rfds);
    if(maxfd < mcpd_router.sock_nl)
        maxfd = mcpd_router.sock_nl;

    FD_SET(mcpd_router.sock_ctl, &rfds);
    if(maxfd < mcpd_router.sock_ctl)
        maxfd = mcpd_router.sock_ctl;

    if (mcpd_router.sock_ctl_con != -1) 
    {
        FD_SET(mcpd_router.sock_ctl_con, &rfds);
        if(maxfd < mcpd_router.sock_ctl_con)
            maxfd = mcpd_router.sock_ctl_con;
    }

#if defined(CONFIG_BCM_OVS_MCAST)
    FD_SET(mcpd_router.ovs_info.sock_ovs, &rfds);
    if(maxfd < mcpd_router.ovs_info.sock_ovs)
        maxfd = mcpd_router.ovs_info.sock_ovs;

    if (mcpd_router.ovs_info.sock_ovs_con != -1) 
    {
        FD_SET(mcpd_router.ovs_info.sock_ovs_con, &rfds);
        if(maxfd < mcpd_router.ovs_info.sock_ovs_con)
            maxfd = mcpd_router.ovs_info.sock_ovs_con;
    }
#endif

    FD_SET(mcpd_router.sock_igmp, &rfds);
    if(maxfd < mcpd_router.sock_igmp)
        maxfd = mcpd_router.sock_igmp;

#ifdef SUPPORT_MLD
    FD_SET(mcpd_router.sock_mld, &rfds);
    if(maxfd < mcpd_router.sock_mld)
        maxfd = mcpd_router.sock_mld;
#endif

    n = select(maxfd+1, &rfds, NULL, NULL, ptime);
    if (n > 0)
    {
#ifdef BRCM_CMS_BUILD
        if(FD_ISSET(mcpd_cms_get_fd(), &rfds))
            mcpd_cms_process_msg();
#endif // BRCM_CMS_BUILD

        if(FD_ISSET(mcpd_router.sock_nl, &rfds))
            mcpd_netlink_recv_mesg();

        if(FD_ISSET(mcpd_router.sock_ctl, &rfds))
            mcpd_control_socket_accept();

        if((mcpd_router.sock_ctl_con != -1) && FD_ISSET(mcpd_router.sock_ctl_con, &rfds))
            mcpd_control_socket_receive(); 

#if defined(CONFIG_BCM_OVS_MCAST)
        if(FD_ISSET(mcpd_router.ovs_info.sock_ovs, &rfds))
            mcpd_ovs_socket_accept();

        if((mcpd_router.ovs_info.sock_ovs_con != -1) &&
            FD_ISSET(mcpd_router.ovs_info.sock_ovs_con, &rfds))
            mcpd_ovs_socket_receive(); 
#endif

        if(FD_ISSET(mcpd_router.sock_igmp, &rfds))
            mcpd_drain_message(mcpd_router.sock_igmp);

#ifdef SUPPORT_MLD
        if(FD_ISSET(mcpd_router.sock_mld, &rfds))
            mcpd_drain_message(mcpd_router.sock_mld);
#endif
    }

    /*
    * service all timer events that are due (there may be no events due
    * if we woke up from select because of activity on the fds).
    */
    if (!mcpd_main_quit)
    {
        mcpd_timer_expires();
    }

    return;
} /* mcpd_input_run */

void mcpd_process_query_timer(void *handle __attribute__((unused)))
{
    t_MCPD_INTERFACE_OBJ *ifp = NULL;
    struct in_addr zero;
#ifdef SUPPORT_MLD
    struct in6_addr zero6;
#endif
    static unsigned int count;
    int timer_val = 0;

    zero.s_addr = 0;

    /* Handle every interface */
    for(ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
    {
        if((ifp->if_dir == MCPD_DOWNSTREAM) &&
           (ifp->if_type & MCPD_IF_TYPE_ROUTED))
        {
            if(ifp->igmp_proxy.is_querier == MCPD_TRUE)
            {
                mcpd_igmp_membership_query(ifp, &zero, NULL, 0, 0, 0);
            }
            else
            {
                /* If not the querier, deal with other-querier-present timer*/
                mcpd_igmp_timer_querier(ifp);
            }

#ifdef SUPPORT_MLD
            if(ifp->mld_proxy.is_querier == MCPD_TRUE)
            {
                mcpd_mld_membership_query(ifp, &zero6, NULL, 0, 0, 0);
            }
            else
            {
                /* If not the querier, deal with other-querier-present timer*/
                mcpd_mld_timer_querier(ifp);
            }
#endif /* SUPPORT_MLD */
        }
    }

    count++;

    if(count <= mcpd_router.igmp_config.startup_query_count)
    {
        timer_val = mcpd_router.igmp_config.startup_query_interval;
    }
    else 
    {
        timer_val = mcpd_router.igmp_config.query_interval;
    }

    if(NULL == mcpd_get_timer(mcpd_process_query_timer, NULL) )
    {
        if(NULL == mcpd_timer_new((timer_val * MSECS_IN_SEC),
                                  mcpd_process_query_timer,
                                  NULL) )
        {
            MCPD_TRACE(MCPD_TRC_ERR, "setting query timer failed");
        }
    }

    return;
} /* mcpd_process_query_timer */

void mcpd_run(void)
{
    mcpd_main_quit = 0;

    mcpd_process_query_timer(NULL);

    /* loop */
    while(!mcpd_main_quit)
    {
        mcpd_input_run();
    }

    return;
} /* mcpd_run */

void mcpd_cleanup(void)
{
    mcpd_timer_cleanup();

#ifdef BRCM_CMS_BUILD
    mcpd_cms_cleanup();
#endif // BRCM_CMS_BUILD    
    if(mcpd_router.sock_buff)
        free(mcpd_router.sock_buff);

    return;
} /* mcpd_cleanup */

void mcpd_update_interface_info(int protoType)
{
    t_MCPD_IFINFO_OBJ *if_info1 = NULL;
    t_MCPD_IFINFO_OBJ *if_info2 = NULL;
    struct sockaddr_in *sa_addr = NULL;
    struct sockaddr_in6 *sa_addr6 = NULL;
    char cmd[CLI_MAX_BUF_SZ];
    t_MCPD_INTERFACE_OBJ *if_intf = NULL;
    t_MCPD_INTERFACE_OBJ *if_intf_next = NULL;
    t_MCPD_INTERFACE_OBJ *if_intf_prev = NULL;

    /* make all the interfaces invalid */
    for(if_intf = mcpd_router.interfaces; if_intf; if_intf = if_intf->next)
    {
        if_intf->audit_done = MCPD_FALSE;
    }

    if_info1 = mcpd_get_ifinfo(IFF_MULTICAST, IFF_LOOPBACK);
    for(if_info2 = if_info1; if_info2 != NULL; if_info2 = if_info2->next)
    {
        sa_addr = (struct sockaddr_in *) &if_info2->addr;
        sa_addr6 = (struct sockaddr_in6 *) &if_info2->addr6;
        mcpd_interface_add(&sa_addr->sin_addr,
                           &sa_addr6->sin6_addr,
                           if_info2->name,
                           if_info2->index,
                           if_info2->iftype);
    }

    mcpd_free_ifinfo(if_info1);

    /* delete invalid interfaces */
    for(if_intf = mcpd_router.interfaces; if_intf; )
    {
        if_intf_next = if_intf->next;

        if((if_intf->if_type == MCPD_IF_TYPE_UNKWN) ||
           (if_intf->audit_done == MCPD_FALSE))
        {
            /* clean up all objects, flush all snooping entries and 
               delete the interface */
            mcpd_interface_cleanup(if_intf, 1, 1, MCPD_PROTO_MAX);
            if(if_intf == mcpd_router.interfaces)
            {
                mcpd_router.interfaces = if_intf->next;
            }
            else
            {
                if_intf_prev->next = if_intf->next;
            }
            /* clean up iptables */
            /* IPV4 filter and NAT */
			if (protoType & BCM_MCAST_PROTO_IPV4)
			{
				sprintf(cmd, "iptables -w -t filter -D FORWARD -i %s "
	                         "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null; ",
	                         if_intf->if_name );
	            if (system(cmd) == -1 )
	                MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
	            sprintf(cmd, "iptables -w -t nat -D PREROUTING -i %s "
	                         "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null; ",
	                         if_intf->if_name );
	            if (system(cmd) == -1 )
	                MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
			}
#ifdef SUPPORT_MLD
            /* IPV6 filter */
			if (protoType & BCM_MCAST_PROTO_IPV6)
			{
	            sprintf(cmd, "ip6tables -w -t filter -D FORWARD -i %s "
	                         "-d FF00::0/8 -j ACCEPT 2>/dev/null; ",
	                         if_intf->if_name );
	            if (system(cmd) == -1 )
	                MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
			}
#endif
            /* igmp packets */
			if (protoType & BCM_MCAST_PROTO_IPV4)
			{
	            sprintf(cmd, "iptables -w -t filter -D INPUT -i %s "
	                         "-p 2 -j ACCEPT 2>/dev/null; " ,
	                         if_intf->if_name );
	            if (system(cmd) == -1 )
	                MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
			}
        }
        else
        {
            if_intf_prev = if_intf;
        }
        if_intf = if_intf_next;
    }

    for(if_intf = mcpd_router.interfaces; if_intf; if_intf = if_intf->next)
    {
        if_intf->proto_enable = 0;
        if (if_intf->if_dir == MCPD_UPSTREAM)
        {
            int igmpSourceEnabled = mcpd_mcast_interface_lookup(if_intf->if_name, MCPD_PROTO_IGMP);
            int igmpProxyEnabled = (igmpSourceEnabled == MCPD_TRUE) ? 
                                   mcpd_upstream_interface_lookup(if_intf->if_name, MCPD_PROTO_IGMP) : 
                                   MCPD_FALSE;
#ifdef SUPPORT_MLD
            int mldSourceEnabled = mcpd_mcast_interface_lookup(if_intf->if_name, MCPD_PROTO_MLD);
            int mldProxyEnabled = (mldSourceEnabled == MCPD_TRUE) ? 
                                  mcpd_upstream_interface_lookup(if_intf->if_name, MCPD_PROTO_MLD) : 
                                  MCPD_FALSE;
#endif
            if ( igmpSourceEnabled == MCPD_TRUE )
            {
                if_intf->proto_enable |= MCPD_IPV4_MCAST_ENABLE;
                /* add iptables rules to accept all multicast*/
                if (if_intf->if_type & MCPD_IF_TYPE_ROUTED)
                {
                	if (protoType & BCM_MCAST_PROTO_IPV4)
                	{
	                    sprintf(cmd, "iptables -w -t filter -D FORWARD -i %s "
	                                 "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null; "
	                                 "iptables -w -t filter -I FORWARD 1 -i %s "
	                                 "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null",
	                                 if_intf->if_name, if_intf->if_name );
	                    if (system(cmd) == -1 )
	                        MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
	                
	                    sprintf(cmd, "iptables -w -t nat -D PREROUTING -i %s "
	                                 "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null; "
	                                 "iptables -w -t nat -I PREROUTING 1 -i %s "
	                                 "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null",
	                                 if_intf->if_name, if_intf->if_name );
	                    if (system(cmd) == -1 )
	                        MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
                	}
                }
            }
            else
            {
                /* clean up all objects and flush snooping entries but do not 
                   delete the interface */
                mcpd_interface_cleanup(if_intf, 0, 1, MCPD_PROTO_IGMP);
                /* add iptables rules to accept all multicast */
                if (if_intf->if_type & MCPD_IF_TYPE_ROUTED)
                {
                    /* clear iptables */
					if (protoType & BCM_MCAST_PROTO_IPV4)
                	{
	                    sprintf(cmd, "iptables -w -t filter -D FORWARD -i %s "
	                                 "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null; ",
	                                 if_intf->if_name );
	                    if (system(cmd) == -1 )
	                        MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
	                
	                    sprintf(cmd, "iptables -w -t nat -D PREROUTING -i %s "
	                                 "-d 224.0.0.0/4 -j ACCEPT 2>/dev/null; ",
	                                 if_intf->if_name );
	                    if (system(cmd) == -1 )
	                        MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
					}
                }
            }
            
#ifdef SUPPORT_MLD
            if ( mldSourceEnabled == MCPD_TRUE )
            {
                if_intf->proto_enable |= MCPD_IPV6_MCAST_ENABLE;
                if (if_intf->if_type & MCPD_IF_TYPE_ROUTED)
                {
					if (protoType & BCM_MCAST_PROTO_IPV6)
                	{
	                    sprintf(cmd, "ip6tables -w -t filter -D FORWARD -i %s "
	                                 "-d FF00::0/8 -j ACCEPT 2>/dev/null; "
	                                 "ip6tables -w -t filter -I FORWARD 1 -i %s "
	                                 "-d FF00::0/8 -j ACCEPT 2>/dev/null",
	                                 if_intf->if_name, if_intf->if_name );
	                    if (system(cmd) == -1 )
	                        MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
					}
                }
            }  
            else
            {
                /* clean up all objects and flush snooping entries but do not 
                   delete the interface */
                mcpd_interface_cleanup(if_intf, 0, 1, MCPD_PROTO_MLD);
                if (if_intf->if_type & MCPD_IF_TYPE_ROUTED)
                {
					if (protoType & BCM_MCAST_PROTO_IPV6)
                	{
	                    sprintf(cmd, "ip6tables -w -t filter -D FORWARD -i %s "
	                                 "-d FF00::0/8 -j ACCEPT 2>/dev/null; ",
	                                 if_intf->if_name );
	                    if (system(cmd) == -1 )
	                        MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
					}
                }
            }

#endif
            /* Checking Proxy Interfaces */
            if ( igmpProxyEnabled == MCPD_TRUE )
            {
                if (if_intf->if_addr.s_addr != 0)
                {
                    if_intf->proto_enable |= MCPD_IGMP_PROXY_ENABLE;
                    /* accept all igmp packets */
					if (protoType & BCM_MCAST_PROTO_IPV4)
                	{
	                    sprintf(cmd, "iptables -w -t filter -D INPUT -i %s "
	                                 "-p 2 -j ACCEPT 2>/dev/null; "
	                                 "iptables -w -t filter -I INPUT 1 -i %s "
	                                 "-p 2 -j ACCEPT 2>/dev/null",
	                                 if_intf->if_name, if_intf->if_name );
	                    if (system(cmd) == -1 )
	                        MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
					}
                }
            }
            else
            {
                /* if proxy is not enabled then we want to flush all kernel
                   memberships for this upstream */
                mcpd_interface_cleanup(if_intf, 0, 0, MCPD_PROTO_IGMP);
                /* ignore all igmp packets */
				if (protoType & BCM_MCAST_PROTO_IPV4)
				{
	                sprintf(cmd, "iptables -w -t filter -D INPUT -i %s "
	                             "-p 2 -j ACCEPT 2>/dev/null; " ,
	                             if_intf->if_name );
	                if (system(cmd) == -1 )
	                    MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
				}
            }
#ifdef SUPPORT_MLD
            if ( mldProxyEnabled == MCPD_TRUE )
            {
                if (!IN6_IS_ADDR_UNSPECIFIED(&if_intf->if_addr6))
                {
                    if_intf->proto_enable |= MCPD_MLD_PROXY_ENABLE;
                    /* accept MLD packets */
					if (protoType & BCM_MCAST_PROTO_IPV6)
                	{
	                    sprintf(cmd, "ip6tables -w -t filter -D INPUT -i %s -p icmpv6 "
	                                 "--icmpv6-type 130 -j ACCEPT 2>/dev/null; "
	                                 "ip6tables -w -t filter -I INPUT 1 -i %s -p icmpv6 "
	                                 "--icmpv6-type 130 -j ACCEPT 2>/dev/null",
	                                 if_intf->if_name, if_intf->if_name );
	                    if (system(cmd) == -1 )
	                         MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
					}
                }
            }
            else
            {
                /* if proxy is not enabled then we want to flush entries for this upstream */
	                mcpd_interface_cleanup(if_intf, 0, 0, MCPD_PROTO_MLD);
	                /* ignore MLD packets */
					if (protoType & BCM_MCAST_PROTO_IPV6)
					{
		                sprintf(cmd, "ip6tables -w -t filter -D INPUT -i %s -p icmpv6 "
		                              "--icmpv6-type 130 -j ACCEPT 2>/dev/null; " ,
		                             if_intf->if_name );
		                if (system(cmd) == -1 )
		                     MCPD_TRACE(MCPD_TRC_ERR, "Failed to system call cmd: %s", cmd);
					}
            }
#endif
        }
        else
        {
            if ( mcpd_downstream_interface_lookup(if_intf->if_name, MCPD_PROTO_IGMP) )
            {
                if_intf->proto_enable |= MCPD_IGMP_SNOOPING_ENABLE;
            }
            else
            {
                /* clean up all objects and flush snooping entries but do not 
                   delete the interface */
                mcpd_interface_cleanup(if_intf, 0, 1, MCPD_PROTO_IGMP);
            }
#ifdef SUPPORT_MLD
            if ( mcpd_downstream_interface_lookup(if_intf->if_name, MCPD_PROTO_MLD) )
            {
                if_intf->proto_enable |= MCPD_MLD_SNOOPING_ENABLE;
            }
            else
            {
                /* clean up all objects and flush snooping entries but do not 
                   delete the interface */
                mcpd_interface_cleanup(if_intf, 0, 1, MCPD_PROTO_MLD);
            }
#endif
            /* if the link changed, the mroute and iptables entries may no longer
               be valid so mark any groups as new to ensure they are reconfigured */
            mcpd_interface_refresh_groups(if_intf); 
        }

        if ((if_intf->if_addr.s_addr != 0) ||
            (if_intf->if_dir == MCPD_DOWNSTREAM))
        {
           mcpd_igmp_krnl_proxy_add_vif(if_intf->if_index, &if_intf->vifi);
        }
#ifdef SUPPORT_MLD
        if (!IN6_IS_ADDR_UNSPECIFIED(&if_intf->if_addr6) || 
            (if_intf->if_dir == MCPD_DOWNSTREAM))
        {
           mcpd_mld_krnl_proxy_add_mif(if_intf->if_index, &if_intf->mifi);
        }
#endif
    }


    return;
} /* mcpd_update_interface_info */

int mcpd_count_filter_entries (int proto)
{
  if (IPPROTO_IGMP == proto)
  {
    int count = 0;
    t_MCPD_FILTER_EXCEPTION *current = mcpd_router.igmp_config.filter_list;
    while (current)
    {
      count ++;
      current = current->next;
    }
    return count;
  }
#ifdef SUPPORT_MLD
  else if (IPPROTO_ICMP == proto)
  {
    int count = 0;
    t_MCPD_FILTER_EXCEPTION *current = mcpd_router.mld_config.filter_list;
    while (current)
    {
      count ++;
      current = current->next;
    }
    return count;
  }
#endif
  else 
  {
    return 0;
  }
}

void mcpd_update_multicast_info()
{
    UINT8 filters[2] = {0,0};
    int foundUpstream = 0;
    t_MCPD_INTERFACE_OBJ *if_intf = NULL;

    t_MCPD_FILTER_EXCEPTION *current = mcpd_router.igmp_config.filter_list;
    t_BCM_MCAST_IGNORE_GROUP_ENTRY *groupEntries = NULL;
    int filterCount = mcpd_count_filter_entries (IPPROTO_IGMP);
    int filterIndex = 0;
#ifdef SUPPORT_MLD
    t_MCPD_FILTER_EXCEPTION *currentMld = mcpd_router.mld_config.filter_list;
#endif
    int ret = 0;
    int snoopagingtime=0;

    if (mcpd_router.igmp_config.admission_required && mcpd_router.igmp_config.admission_bridging_filter) {
      filters[0] = TRUE;
    }
#ifdef SUPPORT_MLD
    if (mcpd_router.mld_config.admission_required && mcpd_router.mld_config.admission_bridging_filter) {
      filters[1] = TRUE;
    }
#endif

    for(if_intf = mcpd_router.interfaces; if_intf; if_intf = if_intf->next)
    {
        if (if_intf->if_dir == MCPD_UPSTREAM)
        {
          foundUpstream = 1;
          break;
        }
    }

    if ( bcm_mcast_api_admission_filter(mcpd_router.sock_nl, filters[0], filters[1]) < 0 )
    {
        MCPD_TRACE(MCPD_TRC_ERR, "Error sending admission filter");
    }

    /* update kernel on the existence/non-existence of a WAN link */
    if ( bcm_mcast_api_uplink(mcpd_router.sock_nl, foundUpstream) < 0 )
    {
        MCPD_TRACE(MCPD_TRC_ERR, "Error sending uplink indication");
    }    

    groupEntries = malloc(sizeof(groupEntries[0]) * filterCount);
    
    while ((current) && (filterIndex < filterCount))
    {
      groupEntries[filterIndex].address.s6_addr32[0] = current->address.s6_addr32[0];
      groupEntries[filterIndex].mask.s6_addr32[0] = current->mask.s6_addr32[0];
      current = current->next;
      filterIndex++;
    }
    ret = bcm_mcast_api_snooping_exception (mcpd_router.sock_nl, BCM_MCAST_PROTO_IPV4, 
                                            filterCount, groupEntries);

    snoopagingtime = mcpd_router.igmp_config.query_interval * mcpd_router.igmp_config.robust_val + 
                     (mcpd_router.igmp_config.query_resp_interval+9)/10;
    /* Set snoop aging timeout in multicast driver snooping table */
    bcm_mcast_api_send_group_timeout (mcpd_router.sock_nl, IPPROTO_IGMP, 
                                      snoopagingtime);

#if defined(CONFIG_BCM_OVS_MCAST)
    /* Set snoop aging timeout in OvS */
    mcpd_ovs_set_snoop_aging_time(snoopagingtime);
#endif

    if (ret != 0)
    {
      MCPD_TRACE(MCPD_TRC_ERR, "Error sending snooping exceptions %d", ret);
    }
    free (groupEntries);

#ifdef SUPPORT_MLD
    filterCount = mcpd_count_filter_entries (IPPROTO_ICMP);
    filterIndex = 0;

    groupEntries = malloc(sizeof(groupEntries[0]) * filterCount);
    while ((currentMld) && (filterIndex < filterCount))
    {
      memcpy (&groupEntries[filterIndex].address, &currentMld->address, sizeof(groupEntries[filterIndex].address) );
      memcpy (&groupEntries[filterIndex].mask,    &currentMld->mask,    sizeof(groupEntries[filterIndex].mask));
      currentMld = currentMld->next;
      filterIndex++;
    }
    bcm_mcast_api_snooping_exception (mcpd_router.sock_nl, BCM_MCAST_PROTO_IPV6, 
                                      filterCount, groupEntries);
    bcm_mcast_api_send_group_timeout (mcpd_router.sock_nl, IPPROTO_ICMP, 
                                      mcpd_router.mld_config.query_interval * mcpd_router.mld_config.robust_val + 
                                      (mcpd_router.mld_config.query_resp_interval + 9)/10);
    free (groupEntries);    
#endif

}


static void mcpd_update_flooding_process(t_MCPD_INTERFACE_OBJ *ifp, t_MCPD_PROTO_TYPE  proto)
{
    t_MCPD_GROUP_OBJ     *groups_ptr = NULL;
    t_MCPD_GROUP_OBJ     *group      = NULL;
    t_MCPD_REP_OBJ       *rep        = NULL;
    t_MCPD_REP_SRC_OBJ   *rsrc       = NULL;
    t_MCPD_FLOOD_TYPE    flood_type;
    t_BCM_MCAST_PKT_INFO pkt_info;
    int                  action;

    if (proto == MCPD_PROTO_IGMP)
        groups_ptr = ifp->igmp_proxy.groups;
#ifdef SUPPORT_MLD    
    else
        groups_ptr = ifp->mld_proxy.groups;
#endif

    if (!groups_ptr) 
        return;

    memset(&pkt_info, 0, sizeof(pkt_info));
    
    for (group = groups_ptr; group; group = group->next)
    {
        MCPD_ASSERT(group != NULL);
        
        if (!group->members)
            continue;
    
        mcpd_group_flood_action_get(group, proto, &action);
        if ((action == MCPD_SNOOP_EX_ADD) || (action == MCPD_SNOOP_IN_ADD))
        {
            flood_type = FLOOD_TYPE_ALL; 
        }
        else 
        {
            flood_type = FLOOD_TYPE_OTHERS;
        }
        
        for (rep = group->members; rep; rep = rep->next)
        {
            MCPD_ASSERT(rep != NULL);

            pkt_info.parent_ifi = group->ifp->if_index;

            if (rep->rsrc != NULL)
            {
                for (rsrc = rep->rsrc; rsrc; rsrc = rsrc->next)
                {   
                    MCPD_ASSERT(rsrc != NULL);
                    
                    if (!rsrc->src->addr)
                    {
                        continue;
                    }
                    
                    MCPD_TRACE(MCPD_TRC_INFO, "flooding process for ifindex %d with src, action=%d",rep->rep_ifi,action);
                    mcpd_router.update_flooding_info_func[proto](ifp, group, rep, rsrc->src->addr, action, &pkt_info, flood_type);
                }
            }
            else
            {
                MCPD_TRACE(MCPD_TRC_INFO, "flooding process for ifindex %d without src, action=%d",rep->rep_ifi,action);
                mcpd_router.update_flooding_info_func[proto](ifp, group, rep, NULL, action, &pkt_info, flood_type);
            }
        }
    }
}

void mcpd_update_flooding_status(int *pre_igmp_flood, int *pre_mld_flood)
{
    t_MCPD_INTERFACE_OBJ *ifp   = NULL;
    int igmp_flood_changed = 0;
    int mld_flood_changed  = 0;    

    if (*pre_igmp_flood != mcpd_router.igmp_config.flood_enable)
    {
        igmp_flood_changed = 1;
    }

#ifdef SUPPORT_MLD
    if (pre_mld_flood != NULL) 
    {
        if (*pre_mld_flood != mcpd_router.mld_config.flood_enable)
        {
            mld_flood_changed = 1;
        }
    }
#endif

    if (!igmp_flood_changed && !mld_flood_changed)
        return;
    
    for (ifp = mcpd_router.interfaces; ifp; ifp = ifp->next)
    {   
        if (igmp_flood_changed)
        {
            if (ifp->igmp_proxy.groups)
            {
                MCPD_TRACE(MCPD_TRC_INFO, "igmp flooding status changed");
                mcpd_update_flooding_process(ifp, MCPD_PROTO_IGMP);
            }
        }
        
#ifdef SUPPORT_MLD        
        if (mld_flood_changed)        
        {
            if (ifp->mld_proxy.groups)
            {
                MCPD_TRACE(MCPD_TRC_INFO, "mld flooding status changed");
                mcpd_update_flooding_process(ifp, MCPD_PROTO_MLD);
            }
        }
#endif
    }

    return;
}

t_MCPD_RET_CODE mcpd_reset_handler(void)
{
    /* clean up all objects and flush snooping entries but do not 
       delete the interface */

    MCPD_TRACE(MCPD_TRC_LOG, "MCPD Reset activated");

    mcpd_interface_cleanup(NULL, 0, 1, MCPD_PROTO_MAX);

    /* cleanup all mcast forwarding entries*/
    if(bcm_mcast_api_fdb_cleanup(mcpd_router.sock_nl) < 0)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "Error while sending clean up msg");
        return MCPD_RET_GENERR;
    }

    return MCPD_RET_OK;
} /* mcpd_reset_handler*/

int mcpd_router_init(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
    mcpd_router.interfaces = NULL;
    mcpd_router.vifiBits   = 0;
    mcpd_router.sock_igmp = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
    if(mcpd_router.sock_igmp < 0 )
        MCPD_TRACE(MCPD_TRC_ERR, "error opening ipv4 socket");

    mcpd_router.sock_nl = -1;
    mcpd_router.sock_ctl = -1;
    mcpd_router.sock_ctl_con = -1;
#if defined(CONFIG_BCM_OVS_MCAST)
    mcpd_router.ovs_info.sock_ovs = -1;
    mcpd_router.ovs_info.sock_ovs_con = -1;
#endif
    mcpd_router.cmp_ip_obj_func[MCPD_PROTO_IGMP] = mcpd_compare_ipv4_addr;
    mcpd_router.chg_mfc_func[MCPD_PROTO_IGMP] = mcpd_igmp_krnl_proxy_chg_mfc;
    mcpd_router.update_snooping_info_func[MCPD_PROTO_IGMP]
                                           = mcpd_igmp_update_snooping_info;
    mcpd_router.update_flooding_info_func[MCPD_PROTO_IGMP]
                                           = mcpd_igmp_update_flooding_info;
    mcpd_router.update_rep_tmr_func[MCPD_PROTO_IGMP]
                                           = mcpd_igmp_reset_rep_timer;
    mcpd_router.update_src_tmr_func[MCPD_PROTO_IGMP]
                                           = mcpd_igmp_update_source_timer;
    mcpd_router.krnl_update_ssm_filters_func[MCPD_PROTO_IGMP]
                                           = mcpd_igmp_krnl_update_ssm_filters;
    mcpd_router.membership_query_func[MCPD_PROTO_IGMP]
                                         = mcpd_igmp_leave_membership_query;
    mcpd_router.update_upstream_ssm_func[MCPD_PROTO_IGMP]
                                          = mcpd_igmp_update_upstream_ssm;
    mcpd_router.admission_control_func[MCPD_PROTO_IGMP]
                                           = mcpd_igmp_admission_control;
    mcpd_router.krnl_drop_membership_func[MCPD_PROTO_IGMP]
                                           = mcpd_igmp_krnl_drop_membership;
    
#ifdef SUPPORT_MLD

    mcpd_router.mifiBits = 0;
    mcpd_router.cmp_ip_obj_func[MCPD_PROTO_MLD] = mcpd_compare_ipv6_addr;
    mcpd_router.chg_mfc_func[MCPD_PROTO_MLD] = mcpd_mld_krnl_proxy_chg_mfc;
    mcpd_router.update_snooping_info_func[MCPD_PROTO_MLD]
                                           = mcpd_mld_update_snooping_info;
    mcpd_router.update_flooding_info_func[MCPD_PROTO_MLD]
                                           = mcpd_mld_update_flooding_info;
    mcpd_router.update_rep_tmr_func[MCPD_PROTO_MLD]
                                           = mcpd_mld_reset_rep_timer;
    mcpd_router.update_src_tmr_func[MCPD_PROTO_MLD]
                                           = mcpd_mld_update_source_timer;
    mcpd_router.krnl_update_ssm_filters_func[MCPD_PROTO_MLD]
                                           = mcpd_mld_krnl_update_ssm_filters;
    mcpd_router.membership_query_func[MCPD_PROTO_MLD]
                                             = mcpd_mld_leave_membership_query;
    mcpd_router.update_upstream_ssm_func[MCPD_PROTO_MLD]
                                          = mcpd_mld_update_upstream_ssm;
    mcpd_router.admission_control_func[MCPD_PROTO_MLD]
                                           = mcpd_mld_admission_control;
    mcpd_router.krnl_drop_membership_func[MCPD_PROTO_MLD]
                                           = mcpd_mld_krnl_drop_membership;
    
    mcpd_router.sock_mld = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

    if(mcpd_router.sock_mld < 0)
        MCPD_TRACE(MCPD_TRC_ERR, "error opending ipv6 socket");
#endif

    /* detach from terminal and detach from smd session group. */
    if (setsid() < 0)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "could not detach from terminal");
        exit(-1);
    }

    signal(SIGUSR1, mcpd_signal);
    signal(SIGKILL, mcpd_signal);
    signal(SIGABRT, mcpd_signal);
    signal(SIGTERM, mcpd_signal);
    signal(SIGHUP, mcpd_signal);

    /* ignore some common, problematic signals */
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    mcpd_init_mem_utility();

#ifdef BRCM_CMS_BUILD
    if ( mcpd_cms_init() != 0 )
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd cmsMsg_init() failed");
        exit(1);
    }
#endif //BRCM_CMS_BUILD

    /* init timers and interfaces */

    return MCPD_RET_OK;
} /* mcpd_router_init */

void mcpd_shutdown(void)
{
    t_MCPD_INTERFACE_OBJ *ifp1 = NULL;
    t_MCPD_INTERFACE_OBJ *ifp2 = NULL;

    for(ifp1 = mcpd_router.interfaces; ifp1 != NULL; )
    {
        ifp2 = ifp1;
        ifp1 = ifp1->next;
        mcpd_interface_cleanup(ifp2, 1, 1, MCPD_PROTO_MAX);
    }

    return;
} /* mcpd_shutdown */

void mcpd_signal(int sig __attribute__((unused)))
{
    mcpd_shutdown();
    exit(0);

    return;
} /* mcpd_signal */

#ifdef BUILD_STATIC
int mcpd_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /* Local definitions */
    struct sched_param sp;

    argc--;
    argv++;

    memset(&mcpd_router, 0, sizeof(t_MCPD_ROUTER));

    /* Read config file */
#ifdef SUPPORT_MLD
    mcpd_config_read(&mcpd_router.igmp_config,
                     &mcpd_router.mld_config);
#else
    mcpd_config_read(&mcpd_router.igmp_config, NULL);
#endif

    /* init mcpd router */
    if(mcpd_router_init(argc, argv) != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_router_init failed");
    }

    mcpd_router.sock_buff = (char *)malloc(BCM_MCAST_NL_RX_BUF_SIZE);
    if ( NULL == mcpd_router.sock_buff )
    {
       MCPD_TRACE(MCPD_TRC_ERR, "mcpd socket buffer allocation failed");
    }

    if(mcpd_netlink_init() != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_netlink_init failed");
    }

    if(mcpd_control_socket_init() != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_control_socket_init failed");
    }

#if defined(CONFIG_BCM_OVS_MCAST)
    mcpd_router.ovs_info.sock_ovs_buff = (char *)malloc(BCM_MCAST_NL_RX_BUF_SIZE);
    if ( NULL == mcpd_router.ovs_info.sock_ovs_buff )
    {
       MCPD_TRACE(MCPD_TRC_ERR, "mcpd ovs socket buffer allocation failed");
    }

    if(mcpd_ovs_socket_init() != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_ovs_socket_init failed");
    }
#endif

    /* Do igmp proxy init */
    if(mcpd_igmp_proxy_init() != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_igmp_proxy_init failed");
    }

    /* Do igmp snooping init */
    if(mcpd_igmp_snooping_init() != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_igmp_snooping_init failed");
    }

#ifdef SUPPORT_MLD
    /* Do mld proxy init */
    if(mcpd_mld_proxy_init() != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_mld_proxy_init failed");
    }

    /* Do mld snooping init */
    if(mcpd_mld_snooping_init() != MCPD_RET_OK)
    {
        MCPD_TRACE(MCPD_TRC_ERR, "mcpd_mld_snooping_init failed");
    }
#endif

#ifdef SUPPORT_MLD
    mcpd_config_read(&mcpd_router.igmp_config,
                     &mcpd_router.mld_config);
#else
    mcpd_config_read(&mcpd_router.igmp_config, NULL);
#endif
    mcpd_update_interface_info(BCM_MCAST_PROTO_ALL);
    mcpd_process_query_timer(NULL); /* start general query timer */
    mcpd_update_multicast_info();

    /* make this process real time */
    sp.sched_priority = 5;/*should be same as CONFIG_BRCM_SOFTIRQ_BASE_RT_PRIO*/

    if ( sched_setscheduler(0, SCHED_RR, &sp) == -1 )
    {
        MCPD_TRACE(MCPD_TRC_ERR, "Failed to make mcpd as realtime process");
    }

    /* start the daemon */
    mcpd_run();

    /* clean up */
    mcpd_cleanup();

    exit(0);
} /* main */
