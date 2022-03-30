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

#if IS_ENABLED(CONFIG_BCM_MCAST)

#include <linux/netfilter.h>
#include <linux/bcm_br_mcast.h>

#include "bcm_br_hooks_mcast.h"
#include "br_private.h"

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
    if (IFF_EBRIDGE & dev->priv_flags)
    {
        /*
         * This is to continue the process of the skb which was hold in admission queue.
         * When the packet was hold in admission queue, it returns NF_STOLEN in NF_BR_PRE_ROUTING hook point.
         * So the okfn br_handle_frame_finish was bypassed then.
         * Since the admission check of this skb is OK, let's continue the call of br_handle_frame_finish.
         */
        br_handle_frame_finish(dev_net(skb->dev), NULL, skb);
        return 0;
    }
    return -EINVAL;
}
EXPORT_SYMBOL(br_bcm_mcast_flood_forward);

unsigned int mcast_receive(struct sk_buff *skb)
{
    struct net_bridge_port *p;

	if (br_bcm_mcast_receive)
	{
        int rv;

        p = br_port_get_rcu(skb->dev);
        if (!p)
            return NF_ACCEPT;

		rv = br_bcm_mcast_receive(p->br->dev->ifindex, skb, 0);
		if (rv < 0)
		{
			/* there was an error with the packet */
			return NF_DROP;
		}
		else if (rv > 0)
		{
			/* the packet was consumed */
			return NF_STOLEN;
		}
		/* continue */
	}
    return NF_ACCEPT;
}

unsigned int mcast_should_deliver(struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct net_bridge_port *p;
   
	if (!br_bcm_mcast_should_deliver)
        return NF_ACCEPT;

    p = br_port_get_rcu(skb->dev);
    if (!p)
        return NF_ACCEPT;

    /* The source device is stored in the state->in pointer for forwarded packets,
     * for local out packets the source device is the bridge device */
    if (0 == br_bcm_mcast_should_deliver(p->br->dev->ifindex, skb, state->in ?: p->br->dev,
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
        p->multicast_router == 2 || (p->multicast_router == 1 && timer_pending(&p->multicast_router_timer))))
#else
		false))
#endif
	{
		return NF_DROP;
	}
    return NF_ACCEPT;
}

#endif
