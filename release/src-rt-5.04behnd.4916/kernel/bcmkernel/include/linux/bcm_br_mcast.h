/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE))

#ifndef _BCM_BR_MCAST_H
#define _BCM_BR_MCAST_H

#include <linux/skbuff.h> 
#include <linux/netdevice.h>

typedef int (*br_bcm_mcast_receive_hook)(int ifindex, struct sk_buff *skb, int is_routed);
typedef int (*br_bcm_mcast_should_deliver_hook)(int ifindex, const struct sk_buff *skb, struct net_device *src_dev, bool dst_mrouter);
typedef int (*br_bcm_mcast_local_in_hook)(struct sk_buff *skb);

int br_bcm_mcast_flood_forward(struct net_device *dev, struct sk_buff *skb);
int br_bcm_mcast_bind(br_bcm_mcast_receive_hook bcm_rx_hook, 
                      br_bcm_mcast_should_deliver_hook bcm_should_deliver_hook);

#endif /* _BCM_BR_MCAST_H */
#endif
