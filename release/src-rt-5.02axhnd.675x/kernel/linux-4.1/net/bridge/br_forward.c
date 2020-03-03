/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/netpoll.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

static int deliver_clone(const struct net_bridge_port *prev,
			 struct sk_buff *skb,
			 void (*__packet_hook)(const struct net_bridge_port *p,
					       struct sk_buff *skb));

#if defined(CONFIG_BCM_KF_WL)
static __inline__ int shouldBypassStp (const struct sk_buff *skb, int state) {
	if (skb->pkt_type == PACKET_BROADCAST || skb->pkt_type == PACKET_MULTICAST)
		return 0;
	if (state == BR_STATE_DISABLED)
		return 0;
	return ( (skb->protocol == htons(0x888e) /* ETHER_TYPE_802_1X */) || 
	         (skb->protocol == htons(0x88c7) /* ETHER_TYPE_802_1X_PREAUTH */) ||
	         (skb->protocol == htons(0x886c) /* ETHER_TYPE_BRCM */ ) );
}
#endif

/* Don't forward packets to originating port or forwarding disabled */
static inline int should_deliver(const struct net_bridge_port *p,
				 const struct sk_buff *skb)
{
#if defined(CONFIG_BCM_KF_WANDEV)
#if !defined(CONFIG_BCM_WAN_2_WAN_FWD_ENABLED)
	/*
	* Do not forward any packets received from one WAN interface
	* to another in multiple PVC case
	*/
	if( (skb->dev->priv_flags & p->dev->priv_flags) & IFF_WANDEV )
	{
		return 0;
	}
#endif

	/* Runner can flood both US/LS and DS traffic */
#if !defined(CONFIG_BCM_KF_RUNNER_FLOODING) && !defined(CONFIG_BCM_RUNNER_FLOODING)
	if ((skb->dev->priv_flags & IFF_WANDEV) == 0 &&
	     (p->dev->priv_flags   & IFF_WANDEV) == 0)
#endif
	{
		struct net_device *sdev = skb->dev;
		struct net_device *ddev = p->dev;

#if defined(CONFIG_BCM_KF_NETDEV_PATH)
		/* From LAN to LAN */
		/* Do not forward any packets to virtual interfaces on the same
		 * real interface of the originating virtual interface.
		 */
		while (!netdev_path_is_root(sdev))
		{
			sdev = netdev_path_next_dev(sdev);
		}

		while (!netdev_path_is_root(ddev))
		{
			ddev = netdev_path_next_dev(ddev);
		}
#endif

#if defined(CONFIG_BCM_KF_LOCAL_SWITCHING_DISABLE)
        /* Drop LAN-LAN traffic; do not drop local packets */
        if (p->br->local_switching_disable && !(sdev->priv_flags & IFF_EBRIDGE) && !(sdev->priv_flags & ddev->priv_flags & IFF_WANDEV))
        {
            return 0;
        }
#endif

		if (sdev == ddev)
		{
			return 0;
		}

#if defined(CONFIG_BCM_KF_RUNNER_FLOODING) && defined(CONFIG_BCM_RUNNER_FLOODING)
		if ((skb->recycle_and_rnr_flags & SKB_RNR_FLOOD) && (ddev->priv_flags & IFF_HW_SWITCH))
		{
			/* Flooded by Runner */
			return 0;
		}

#else

		if (skb->pkt_type == PACKET_BROADCAST)
		{
#if defined(CONFIG_BCM_KF_ENET_SWITCH)
			if (sdev->priv_flags & IFF_HW_SWITCH & ddev->priv_flags)
			{
				/* both source and destination are IFF_HW_SWITCH 
				   if they are also on the same switch, reject the packet */
				if (!((sdev->priv_flags & IFF_EXT_SWITCH) ^ (ddev->priv_flags & IFF_EXT_SWITCH)))
				{
					return 0;
				}
			}
#endif /* CONFIG_BCM_KF_ENET_SWITCH */
		}

#endif /* CONFIG_BCM_KF_RUNNER_FLOODING */
	}
#endif /* CONFIG_BCM_KF_WANDEV */

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
	if ( br_bcm_mcast_should_deliver != NULL )
	{
		if ( 0 == br_bcm_mcast_should_deliver(p->br->dev->ifindex, skb, p->dev,
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
		      p->multicast_router == 2 || (p->multicast_router == 1 && timer_pending(&p->multicast_router_timer))) )
#else
		      false) )
#endif
		{
			return 0;
		}
	}
#endif

#if defined(CONFIG_BCM_KF_WL)
	return (((p->flags & BR_HAIRPIN_MODE) || skb->dev != p->dev) &&
	        br_allowed_egress(p->br, nbp_get_vlan_info(p), skb) &&
	        ((p->state == BR_STATE_FORWARDING) || shouldBypassStp(skb, p->state)));
#else
	return ((p->flags & BR_HAIRPIN_MODE) || skb->dev != p->dev) &&
		br_allowed_egress(p->br, nbp_get_vlan_info(p), skb) &&
		p->state == BR_STATE_FORWARDING;
#endif
}


#ifdef CONFIG_BCM_KF_MISC_BACKPORTS
static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
}
#endif

int br_dev_queue_push_xmit(struct sock *sk, struct sk_buff *skb)
{
#ifdef CONFIG_BCM_KF_MISC_BACKPORTS
	/* is_skb_forwardable() is assuming skb->len already has ETH_LEN and
	 * that is not correct here and this can cause the packets >MTU 
	 * to be forwarded, adding proper checks
	 */
	if((packet_length(skb) > skb->dev->mtu ) && !skb_is_gso(skb)) {
#else
	if (!is_skb_forwardable(skb->dev, skb)) {
#endif
		kfree_skb(skb);
	} else {
		skb_push(skb, ETH_HLEN);
		br_drop_fake_rtable(skb);
		skb_sender_cpu_clear(skb);
		dev_queue_xmit(skb);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(br_dev_queue_push_xmit);

int br_forward_finish(struct sock *sk, struct sk_buff *skb)
{
	return NF_HOOK(NFPROTO_BRIDGE, NF_BR_POST_ROUTING, sk, skb,
		       NULL, skb->dev,
		       br_dev_queue_push_xmit);

}
EXPORT_SYMBOL_GPL(br_forward_finish);

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb = br_handle_vlan(to->br, nbp_get_vlan_info(to), skb);
	if (!skb)
		return;

	skb->dev = to->dev;

	if (unlikely(netpoll_tx_running(to->br->dev))) {
#ifdef CONFIG_BCM_KF_MISC_BACKPORTS
		/* is_skb_forwardable() is assuming skb->len already has ETH_LEN and
		 * that is not correct here, this can cause the packets >MTU 
		 * to be forwarded, adding proper checks
		 */
		if (packet_length(skb) > skb->dev->mtu && !skb_is_gso(skb))
#else
		if (!is_skb_forwardable(skb->dev, skb))
#endif
			kfree_skb(skb);
		else {
			skb_push(skb, ETH_HLEN);
			br_netpoll_send_skb(to, skb);
		}
		return;
	}

	NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_OUT, NULL, skb,
		NULL, skb->dev,
		br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	if (skb_warn_if_lro(skb)) {
		kfree_skb(skb);
		return;
	}

	skb = br_handle_vlan(to->br, nbp_get_vlan_info(to), skb);
	if (!skb)
		return;

	indev = skb->dev;
	skb->dev = to->dev;
	skb_forward_csum(skb);

	NF_HOOK(NFPROTO_BRIDGE, NF_BR_FORWARD, NULL, skb,
		indev, skb->dev,
		br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (to && should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}
EXPORT_SYMBOL_GPL(br_deliver);

/* called with rcu_read_lock */
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb, struct sk_buff *skb0)
{
	if (should_deliver(to, skb)) {
		if (skb0)
			deliver_clone(to, skb, __br_forward);
		else
			__br_forward(to, skb);
		return;
	}

	if (!skb0)
		kfree_skb(skb);
}

static int deliver_clone(const struct net_bridge_port *prev,
			 struct sk_buff *skb,
			 void (*__packet_hook)(const struct net_bridge_port *p,
					       struct sk_buff *skb))
{
	struct net_device *dev = BR_INPUT_SKB_CB(skb)->brdev;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	struct sk_buff *skb2 = skb;
#endif
	skb = skb_clone(skb, GFP_ATOMIC);
	if (!skb) {
		dev->stats.tx_dropped++;
		return -ENOMEM;
	}

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_clone(skb2, blog_ptr(skb));
#endif

	__packet_hook(prev, skb);
	return 0;
}

static struct net_bridge_port *maybe_deliver(
	struct net_bridge_port *prev, struct net_bridge_port *p,
	struct sk_buff *skb,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
	int err;

	if (!should_deliver(p, skb))
		return prev;

	if (!prev)
		goto out;

	err = deliver_clone(prev, skb, __packet_hook);
	if (err)
		return ERR_PTR(err);

out:
	return p;
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb,
		     struct sk_buff *skb0,
		     void (*__packet_hook)(const struct net_bridge_port *p,
					   struct sk_buff *skb),
		     bool unicast)
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	Blog_t * blog_p = blog_ptr(skb);

	if (blog_p && !blog_p->rx.multicast)
	{
#if defined(CONFIG_BCM_KF_INTF_BRG) && defined (CONFIG_BCM_INTF_BRG_ENABLED)
		/* Keep blog for interface-based bridging. */
		if (!is_interface_br(br, skb))
#endif /* CONFIG_BCM_KF_INTF_BRG && CONFIG_BCM_INTF_BRG_ENABLED */
		{
			blog_skip(skb, blog_skip_reason_br_flood);
		}
	}
#endif
	prev = NULL;

	list_for_each_entry_rcu(p, &br->port_list, list) {
		/* Do not flood unicast traffic to ports that turn it off */
		if (unicast && !(p->flags & BR_FLOOD))
			continue;

		/* Do not flood to ports that enable proxy ARP */
		if (p->flags & BR_PROXYARP)
			continue;
		if ((p->flags & BR_PROXYARP_WIFI) &&
		    BR_INPUT_SKB_CB(skb)->proxyarp_replied)
			continue;

		prev = maybe_deliver(prev, p, skb, __packet_hook);
		if (IS_ERR(prev))
			goto out;
	}

	if (!prev)
		goto out;

	if (skb0)
		deliver_clone(prev, skb, __packet_hook);
	else
		__packet_hook(prev, skb);
	return;

out:
	if (!skb0)
		kfree_skb(skb);
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, bool unicast)
{
	br_flood(br, skb, NULL, __br_deliver, unicast);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb,
		      struct sk_buff *skb2, bool unicast)
{
	br_flood(br, skb, skb2, __br_forward, unicast);
}

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
/* called with rcu_read_lock */
static void br_multicast_flood(struct net_bridge_mdb_entry *mdst,
			       struct sk_buff *skb, struct sk_buff *skb0,
			       void (*__packet_hook)(
					const struct net_bridge_port *p,
					struct sk_buff *skb))
{
	struct net_device *dev = BR_INPUT_SKB_CB(skb)->brdev;
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_port *prev = NULL;
	struct net_bridge_port_group *p;
	struct hlist_node *rp;

	rp = rcu_dereference(hlist_first_rcu(&br->router_list));
	p = mdst ? rcu_dereference(mdst->ports) : NULL;
	while (p || rp) {
		struct net_bridge_port *port, *lport, *rport;

		lport = p ? p->port : NULL;
		rport = rp ? hlist_entry(rp, struct net_bridge_port, rlist) :
			     NULL;

		port = (unsigned long)lport > (unsigned long)rport ?
		       lport : rport;

		prev = maybe_deliver(prev, port, skb, __packet_hook);
		if (IS_ERR(prev))
			goto out;

		if ((unsigned long)lport >= (unsigned long)port)
			p = rcu_dereference(p->next);
		if ((unsigned long)rport >= (unsigned long)port)
			rp = rcu_dereference(hlist_next_rcu(rp));
	}

	if (!prev)
		goto out;

	if (skb0)
		deliver_clone(prev, skb, __packet_hook);
	else
		__packet_hook(prev, skb);
	return;

out:
	if (!skb0)
		kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_multicast_deliver(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb)
{
	br_multicast_flood(mdst, skb, NULL, __br_deliver);
}

/* called with rcu_read_lock */
void br_multicast_forward(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb, struct sk_buff *skb2)
{
	br_multicast_flood(mdst, skb, skb2, __br_forward);
}
#endif
