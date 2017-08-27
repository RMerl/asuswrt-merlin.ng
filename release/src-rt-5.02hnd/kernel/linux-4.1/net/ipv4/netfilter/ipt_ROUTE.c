/*
 * This implements the ROUTE target, which enables you to setup unusual
 * routes not supported by the standard kernel routing table.
 *
 * Copyright (C) 2002 Cedric de Launois <delaunois@info.ucl.ac.be>
 *
 * v 1.11 2004/11/23
 *
 * This software is distributed under GNU GPL v2, 1991
 */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ipt_ROUTE.h>
#include <linux/netdevice.h>
#include <linux/route.h>
#include <linux/version.h>
#include <linux/if_arp.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/icmp.h>
#include <net/checksum.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#define NF_IP_PRE_ROUTING	NF_INET_PRE_ROUTING
#define NF_IP_LOCAL_IN		NF_INET_LOCAL_IN
#define NF_IP_FORWARD		NF_INET_FORWARD
#define NF_IP_LOCAL_OUT		NF_INET_LOCAL_OUT
#define NF_IP_POST_ROUTING	NF_INET_POST_ROUTING
#define NF_IP_NUMHOOKS		NF_INET_NUMHOOKS

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#define IPT_CONTINUE XT_CONTINUE
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cedric de Launois <delaunois@info.ucl.ac.be>");
MODULE_DESCRIPTION("iptables ROUTE target module");

/* Try to route the packet according to the routing keys specified in
 * route_info. Keys are :
 *  - ifindex : 
 *      0 if no oif preferred, 
 *      otherwise set to the index of the desired oif
 *  - route_info->gw :
 *      0 if no gateway specified,
 *      otherwise set to the next host to which the pkt must be routed
 * If success, skb->dev is the output device to which the packet must 
 * be sent and skb->dst is not NULL
 *
 * RETURN: -1 if an error occured
 *          1 if the packet was succesfully routed to the 
 *            destination desired
 *          0 if the kernel routing table could not route the packet
 *            according to the keys specified
 */
static int route(struct sk_buff *skb,
		 unsigned int ifindex,
		 const struct ipt_route_target_info *route_info)
{
	int err;
	struct rtable *rt;
	struct iphdr *iph  = ip_hdr(skb);
	struct flowi fl = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
		.u = {
			.ip4 = {
				.flowi4_oif = ifindex,
				.daddr = iph->daddr,
				.saddr = 0,
				.flowi4_tos = RT_TOS(iph->tos),
				.flowi4_scope = RT_SCOPE_UNIVERSE,
			}
		}
#else
		.oif = ifindex,
		.nl_u = {
			.ip4_u = {
				.daddr = iph->daddr,
				.saddr = 0,
				.tos = RT_TOS(iph->tos),
				.scope = RT_SCOPE_UNIVERSE,
			}
		} 
#endif
	};
	
	/* The destination address may be overloaded by the target */
	if (route_info->gw)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
		fl.u.ip4.daddr = route_info->gw;
#else
		fl.fl4_dst = route_info->gw;
#endif
	
	/* Trying to route the packet using the standard routing table. */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	rt = ip_route_output_key(&init_net, (struct flowi4*)&fl);
	if (IS_ERR(rt)) {
		err = (int)PTR_ERR(rt);
#else
	if ((err = ip_route_output_key(&init_net, &rt, &fl))) {
#endif
		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: couldn't route pkt (err: %i)",err);
		return -1;
	}
	
	/* Drop old route. */
	dst_release(skb_dst(skb));
	skb_dst_set(skb, NULL);

	if (!ifindex || rt->dst.dev->ifindex == ifindex) {
		struct dst_entry *dst;

		skb_dst_set(skb, &rt->dst);
		dst = skb_dst(skb);
		skb->dev = dst->dev;
		skb->protocol = htons(ETH_P_IP);
		return 1;
	}

	if (net_ratelimit()) 
		DEBUGP("ipt_ROUTE: failed to route as desired gw=%u.%u.%u.%u oif=%i (got oif=%i)\n", 
		       NIPQUAD(route_info->gw), ifindex, rt->dst.dev->ifindex);
	return 0;
}


/* Stolen from ip_finish_output2
 * PRE : skb->dev is set to the device we are leaving by
 *       skb->dst is not NULL
 * POST: the packet is sent with the link layer header pushed
 *       the packet is destroyed
 */
static void ip_direct_send(struct sk_buff *skb)
{
	struct dst_entry *dst = skb_dst(skb);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	struct neighbour *neigh = dst_neigh_lookup_skb(dst, skb) ;
#else
	struct hh_cache *hh = dst->hh;
#endif
	struct net_device *dev = dst->dev;
	int hh_len = LL_RESERVED_SPACE(dev);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	unsigned seq;
#endif

	/* Be paranoid, rather than too clever. */
	if (unlikely(skb_headroom(skb) < hh_len && dev->header_ops->create)) {
		struct sk_buff *skb2;

		skb2 = skb_realloc_headroom(skb, LL_RESERVED_SPACE(dev));
		if (skb2 == NULL) {
			kfree_skb(skb);
			return;
		}
		if (skb->sk)
			skb_set_owner_w(skb2, skb->sk);
		kfree_skb(skb);
		skb = skb2;
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	if (neigh)
		dst_neigh_output(dst, neigh, skb);
#else
	if (hh) {
		do {
			int hh_alen;

			seq = read_seqbegin(&hh->hh_lock);
			hh_alen = HH_DATA_ALIGN(hh->hh_len);
			memcpy(skb->data - hh_alen, hh->hh_data, hh_alen);
		} while (read_seqretry(&hh->hh_lock, seq));
		skb_push(skb, hh->hh_len);
		hh->hh_output(skb);
	} else if (dst->neighbour)
		dst->neighbour->output(skb);
#endif
	else {
		if (net_ratelimit())
			DEBUGP(KERN_DEBUG "ipt_ROUTE: no hdr & no neighbour cache!\n");
		kfree_skb(skb);
	}
}


/* PRE : skb->dev is set to the device we are leaving by
 * POST: - the packet is directly sent to the skb->dev device, without 
 *         pushing the link layer header.
 *       - the packet is destroyed
 */
static inline int dev_direct_send(struct sk_buff *skb)
{
	return dev_queue_xmit(skb);
}


static unsigned int route_oif(const struct ipt_route_target_info *route_info,
			      struct sk_buff *skb) 
{
	unsigned int ifindex = 0;
	struct net_device *dev_out = NULL;

	/* The user set the interface name to use.
	 * Getting the current interface index.
	 */
	if ((dev_out = dev_get_by_name(&init_net, route_info->oif))) {
		ifindex = dev_out->ifindex;
	} else {
		/* Unknown interface name : packet dropped */
		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: oif interface %s not found\n", route_info->oif);
		return NF_DROP;
	}

	/* Trying the standard way of routing packets */
	switch (route(skb, ifindex, route_info)) {
	case 1:
		dev_put(dev_out);
		if (route_info->flags & IPT_ROUTE_CONTINUE)
			return IPT_CONTINUE;

		ip_direct_send(skb);
		return NF_STOLEN;

	case 0:
		/* Failed to send to oif. Trying the hard way */
		if (route_info->flags & IPT_ROUTE_CONTINUE)
			return NF_DROP;

		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: forcing the use of %i\n",
			       ifindex);

		/* We have to force the use of an interface.
		 * This interface must be a tunnel interface since
		 * otherwise we can't guess the hw address for
		 * the packet. For a tunnel interface, no hw address
		 * is needed.
		 */
		if ((dev_out->type != ARPHRD_TUNNEL)
		    && (dev_out->type != ARPHRD_IPGRE)) {
			if (net_ratelimit()) 
				DEBUGP("ipt_ROUTE: can't guess the hw addr !\n");
			dev_put(dev_out);
			return NF_DROP;
		}
	
		/* Send the packet. This will also free skb
		 * Do not go through the POST_ROUTING hook because 
		 * skb->dst is not set and because it will probably
		 * get confused by the destination IP address.
		 */
		skb->dev = dev_out;
		dev_direct_send(skb);
		dev_put(dev_out);
		return NF_STOLEN;
		
	default:
		/* Unexpected error */
		dev_put(dev_out);
		return NF_DROP;
	}
}


static unsigned int route_iif(const struct ipt_route_target_info *route_info,
			      struct sk_buff *skb) 
{
	struct net_device *dev_in = NULL;

	/* Getting the current interface index. */
	if (!(dev_in = dev_get_by_name(&init_net, route_info->iif))) {
		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: iif interface %s not found\n", route_info->iif);
		return NF_DROP;
	}

	skb->dev = dev_in;
	dst_release(skb_dst(skb));
	skb_dst_set(skb, NULL);

	netif_rx(skb);
	dev_put(dev_in);
	return NF_STOLEN;
}


static unsigned int route_gw(const struct ipt_route_target_info *route_info,
			     struct sk_buff *skb) 
{
	if (route(skb, 0, route_info)!=1)
		return NF_DROP;

	if (route_info->flags & IPT_ROUTE_CONTINUE)
		return IPT_CONTINUE;

	ip_direct_send(skb);
	return NF_STOLEN;
}


/* To detect and deter routed packet loopback when using the --tee option,
 * we take a page out of the raw.patch book: on the copied skb, we set up
 * a fake ->nfct entry, pointing to the local &route_tee_track. We skip
 * routing packets when we see they already have that ->nfct.
 */

static struct nf_conn route_tee_track;

static unsigned int ipt_route_target (struct sk_buff *skb,
				    const struct xt_action_param *par)
{
	const struct ipt_route_target_info *route_info = par->targinfo;
	unsigned int res;

	if (skb->nfct == &route_tee_track.ct_general) {
		/* Loopback - a packet we already routed, is to be
		 * routed another time. Avoid that, now.
		 */
		if (net_ratelimit()) 
			DEBUGP(KERN_DEBUG "ipt_ROUTE: loopback - DROP!\n");
		return NF_DROP;
	}

	/* If we are at PREROUTING or INPUT hook
	 * the TTL isn't decreased by the IP stack
	 */
	if (par->hooknum == NF_IP_PRE_ROUTING ||
	    par->hooknum == NF_IP_LOCAL_IN) {

		struct iphdr *iph = ip_hdr(skb);

		if (iph->ttl <= 1) {
			struct rtable *rt;
			struct flowi fl = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
				.u = {
					.ip4 = {
						.flowi4_oif = 0,
						.daddr = iph->daddr,
						.saddr = iph->saddr,
						.flowi4_tos = RT_TOS(iph->tos),
						.flowi4_scope = ((iph->tos & RTO_ONLINK) ?
							  RT_SCOPE_LINK :
							  RT_SCOPE_UNIVERSE)
					}
				}
#else
				.oif = 0,
				.nl_u = {
					.ip4_u = {
						.daddr = iph->daddr,
						.saddr = iph->saddr,
						.tos = RT_TOS(iph->tos),
						.scope = ((iph->tos & RTO_ONLINK) ?
							  RT_SCOPE_LINK :
							  RT_SCOPE_UNIVERSE)
					}
				} 
#endif
			};

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
			rt = ip_route_output_key(&init_net, (struct flowi4*)&fl);
			if (IS_ERR(rt)) {
#else
			if (ip_route_output_key(&init_net, &rt, &fl)) {
#endif
				return NF_DROP;
			}

			if (skb->dev == rt->dst.dev) {
				/* Drop old route. */
				dst_release(skb_dst(skb));
				skb_dst_set(skb, &rt->dst);

				/* this will traverse normal stack, and 
				 * thus call conntrack on the icmp packet */
				icmp_send(skb, ICMP_TIME_EXCEEDED, 
					  ICMP_EXC_TTL, 0);
			}

			return NF_DROP;
		}

		/*
		 * If we are at INPUT the checksum must be recalculated since
		 * the length could change as the result of a defragmentation.
		 */
		if(par->hooknum == NF_IP_LOCAL_IN) {
			iph->ttl = iph->ttl - 1;
			iph->check = 0;
			iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
		} else {
			ip_decrease_ttl(iph);
		}
	}

	if ((route_info->flags & IPT_ROUTE_TEE)) {
		/*
		 * Copy the *pskb, and route the copy. Will later return
		 * IPT_CONTINUE for the original skb, which should continue
		 * on its way as if nothing happened. The copy should be
		 * independantly delivered to the ROUTE --gw.
		 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
		skb = skb_copy(skb, GFP_ATOMIC);
#else
		skb = skb_copy(*pskb, GFP_ATOMIC);
#endif
		if (!skb) {
			if (net_ratelimit()) 
				DEBUGP(KERN_DEBUG "ipt_ROUTE: copy failed!\n");
			return IPT_CONTINUE;
		}
	}

	/* Tell conntrack to forget this packet since it may get confused 
	 * when a packet is leaving with dst address == our address.
	 * Good idea ? Dunno. Need advice.
	 *
	 * NEW: mark the skb with our &route_tee_track, so we avoid looping
	 * on any already routed packet.
	 */
	if (!(route_info->flags & IPT_ROUTE_CONTINUE)) {
		nf_conntrack_put(skb->nfct);
		skb->nfct = &route_tee_track.ct_general;
		skb->nfctinfo = IP_CT_NEW;
		nf_conntrack_get(skb->nfct);
	}

	if (route_info->oif[0] != '\0') {
		res = route_oif(route_info, skb);
	} else if (route_info->iif[0] != '\0') {
		res = route_iif(route_info, skb);
	} else if (route_info->gw) {
		res = route_gw(route_info, skb);
	} else {
		if (net_ratelimit()) 
			DEBUGP(KERN_DEBUG "ipt_ROUTE: no parameter !\n");
		res = IPT_CONTINUE;
	}

	if ((route_info->flags & IPT_ROUTE_TEE))
		res = IPT_CONTINUE;

	return res;
}


static int ipt_route_checkentry(const struct xt_tgchk_param *par)
{
	return 0;	/* means success */
}


static struct xt_target xt_route_reg = { 
	.name = "ROUTE",
	.target = ipt_route_target,
	.family     = AF_INET,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)
	.targetsize = sizeof(struct ipt_route_target_info),
#endif
	.checkentry = ipt_route_checkentry,
	.me = THIS_MODULE,
};

static int __init init(void)
{
	/* Set up fake conntrack (stolen from raw.patch):
	    - to never be deleted, not in any hashes */
	atomic_set(&route_tee_track.ct_general.use, 1);
	/*  - and look it like as a confirmed connection */
	set_bit(IPS_CONFIRMED_BIT, &route_tee_track.status);
	/* Initialize fake conntrack so that NAT will skip it */
	route_tee_track.status |= IPS_NAT_DONE_MASK;

	return xt_register_target(&xt_route_reg);
}


static void __exit fini(void)
{
	xt_unregister_target(&xt_route_reg);
}

module_init(init);
module_exit(fini);
