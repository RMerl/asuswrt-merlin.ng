/*
*    Copyright (c) 2015 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2015:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef _BCM_MCAST_PRIV_H_
#define _BCM_MCAST_PRIV_H_

#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_bridge.h>
#include <net/sock.h>
#include <linux/bcm_log.h>

#include <bcm_mcast.h>
#include "bcm_mcast_if.h"
#include "bcm_mcast_igmp.h"
#include "bcm_mcast_mld.h"
#include "bcm_mcast_netlink.h"
#include "bcm_mcast_blog.h"


#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_MCAST, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_MCAST, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_MCAST, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_MCAST, fmt, ##arg)

#define BCM_MCAST_MAX_CLIENTS 2

#define BCM_MCAST_FLAG_PPP    1

#define BCM_MCAST_FDB_TIMER_TIMEOUT  (2*HZ)
#define BCM_MCAST_MEMBERSHIP_TIMEOUT 260

struct bcm_mcast_control_filter_entry {
   struct hlist_node            node;
   struct in6_addr              group;
   struct in6_addr              mask;
};

typedef struct
{
   struct sock              *netlink_sock;
   int                       ctrl_client[BCM_MCAST_MAX_CLIENTS];
   struct hlist_head         if_hlist;
   struct notifier_block     netdev_notifier;
   struct notifier_block     bridge_notifier;
   struct raw_notifier_head  mcast_snooping_chain;
   int                       mcastPriQueue;
#if defined(CONFIG_BLOG)
   int                       blog_enable;
#endif
#if defined(CONFIG_BR_IGMP_SNOOP)
   struct proc_dir_entry    *ipv4_proc_entry;
   struct file_operations    ipv4_file_io;
   struct seq_operations     ipv4_seq_ops;
   int                       ipv4_hash_salt;
   struct kmem_cache        *ipv4_grp_cache;
   struct kmem_cache        *ipv4_rep_cache;
   int                       igmp_admission;
   struct kmem_cache        *ipv4_exception_cache;
   struct hlist_head         igmp_snoopExceptionList;
   int                       igmp_general_query_timeout_secs;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
   struct proc_dir_entry    *ipv6_proc_entry;
   struct file_operations    ipv6_file_io;
   struct seq_operations     ipv6_seq_ops;
   int                       ipv6_hash_salt;
   struct kmem_cache        *ipv6_grp_cache;
   struct kmem_cache        *ipv6_rep_cache;
   int                       mld_admission;
   struct kmem_cache        *ipv6_exception_cache;
   struct hlist_head         mld_snoopExceptionList;
   int                       mld_general_query_timeout_secs;
#endif

#if defined(CONFIG_BR_IGMP_SNOOP) || defined(CONFIG_BR_MLD_SNOOP)
   int                       thereIsAnUplink;
   spinlock_t                cfgLock;
#endif
} bcm_mcast_ctrl;

extern bcm_mcast_ctrl *mcast_ctrl;

void bcm_mcast_set_pri_queue(int queue);
void bcm_mcast_set_uplink(int uplinkExists);
void bcm_mcast_set_admission(int proto, int enable);
int bcm_mcast_get_lan2lan_snooping(int proto, bcm_mcast_ifdata *pif);
#if defined(CONFIG_BLOG)
void bcm_mcast_process_blog_enable(int enable);
#endif

#endif /* _BCM_MCAST_PRIV_H_ */

