#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
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
