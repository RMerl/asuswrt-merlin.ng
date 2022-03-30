/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

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
#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE))

#ifndef _BCM_BR_MCAST_H
#define _BCM_BR_MCAST_H

#include <linux/skbuff.h> 
#include <linux/netdevice.h>

typedef int (*br_bcm_mcast_receive_hook)(int ifindex, struct sk_buff *skb, int is_routed);
typedef int (*br_bcm_mcast_should_deliver_hook)(int ifindex, struct sk_buff *skb, struct net_device *src_dev, bool dst_mrouter);

int br_bcm_mcast_flood_forward(struct net_device *dev, struct sk_buff *skb);
int br_bcm_mcast_bind(br_bcm_mcast_receive_hook bcm_rx_hook, br_bcm_mcast_should_deliver_hook bcm_should_deliver_hook);

#endif /* _BCM_BR_MCAST_H */
#endif
