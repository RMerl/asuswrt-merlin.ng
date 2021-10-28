#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
/*
*    Copyright (c) 2015 Broadcom Corporation
*    All Rights Reserved
* 
<:label-BRCM:2015:DUAL/GPL:standard

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
#include "br_private.h"
#include <linux/if_bridge.h>

br_bcm_mcast_receive_hook br_bcm_mcast_receive = NULL;
br_bcm_mcast_should_deliver_hook br_bcm_mcast_should_deliver = NULL;

int br_bcm_mcast_bind(br_bcm_mcast_receive_hook bcm_rx_hook, br_bcm_mcast_should_deliver_hook bcm_should_deliver_hook)
{
   br_bcm_mcast_receive = bcm_rx_hook;
   br_bcm_mcast_should_deliver = bcm_should_deliver_hook;
   return 0;
}
EXPORT_SYMBOL(br_bcm_mcast_bind);

/* must be called with rcu_read_lock */
int br_bcm_mcast_flood_forward(struct net_device *dev, struct sk_buff *skb)
{
   if ( IFF_EBRIDGE & dev->priv_flags )
   {
      struct net_bridge *br = netdev_priv(dev);
      if ( NULL == br ) 
      {
         return -EINVAL;
      }
      br_flood_forward(br, skb, NULL, 0);
   }
   else
   {
      return -EINVAL;
   }
   return 0;
}
EXPORT_SYMBOL(br_bcm_mcast_flood_forward);

#endif /* CONFIG_BCM_MCAST && CONFIG_BCM_KF_MCAST */
