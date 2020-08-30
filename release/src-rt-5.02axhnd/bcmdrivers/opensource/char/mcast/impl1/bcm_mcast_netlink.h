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

#ifndef _BCM_MCAST_NETLINK_H_
#define _BCM_MCAST_NETLINK_H_

struct sk_buff *bcm_mcast_netlink_alloc_skb(int len, int proto);
int bcm_mcast_netlink_send_skb(struct sk_buff *skb, int msg_type);
void bcm_mcast_netlink_send_igmp_purge_entry(bcm_mcast_ifdata *pif,
                                             t_igmp_grp_entry *igmp_entry, 
                                             t_igmp_rep_entry *rep_entry);
void bcm_mcast_netlink_send_query_trigger(int rep_ifi);
void bcm_mcast_notify_event(int event, int proto, void *grp_entry, void *rep_entry);

__init int bcm_mcast_netlink_init(void);
void bcm_mcast_netlink_deinit(void);

#endif /* _BCM_MCAST_NETLINK_H_ */