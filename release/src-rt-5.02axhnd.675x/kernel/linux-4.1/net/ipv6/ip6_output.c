/*
 *	IPv6 output functions
 *	Linux INET6 implementation
 *
 *	Authors:
 *	Pedro Roque		<roque@di.fc.ul.pt>
 *
 *	Based on linux/net/ipv4/ip_output.c
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *	Changes:
 *	A.N.Kuznetsov	:	airthmetics in fragmentation.
 *				extension headers are implemented.
 *				route changes now work.
 *				ip6_forward does not confuse sniffers.
 *				etc.
 *
 *      H. von Brand    :       Added missing #include <linux/string.h>
 *	Imran Patel	:	frag id should be in NBO
 *      Kazunori MIYAZAWA @USAGI
 *			:       add ip6_append_data and related functions
 *				for datagram xmit
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/in6.h>
#include <linux/tcp.h>
#include <linux/route.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv6.h>

#include <net/sock.h>
#include <net/snmp.h>

#include <net/ipv6.h>
#include <net/ndisc.h>
#include <net/protocol.h>
#include <net/ip6_route.h>
#include <net/addrconf.h>
#include <net/rawv6.h>
#include <net/icmp.h>
#include <net/xfrm.h>
#include <net/checksum.h>
#include <linux/mroute6.h>

static int ip6_finish_output2(struct sock *sk, struct sk_buff *skb)
{
	struct dst_entry *dst = skb_dst(skb);
	struct net_device *dev = dst->dev;
	struct neighbour *neigh;
	struct in6_addr *nexthop;
	int ret;

#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	skb->protocol = htons(ETH_P_IPV6);
	skb->dev = dev;
#endif

	if (ipv6_addr_is_multicast(&ipv6_hdr(skb)->daddr)) {
		struct inet6_dev *idev = ip6_dst_idev(skb_dst(skb));

		if (!(dev->flags & IFF_LOOPBACK) && sk_mc_loop(sk) &&
		    ((mroute6_socket(dev_net(dev), skb) &&
		     !(IP6CB(skb)->flags & IP6SKB_FORWARDED)) ||
		     ipv6_chk_mcast_addr(dev, &ipv6_hdr(skb)->daddr,
					 &ipv6_hdr(skb)->saddr))) {
			struct sk_buff *newskb = skb_clone(skb, GFP_ATOMIC);

			/* Do not check for IFF_ALLMULTI; multicast routing
			   is not supported in any case.
			 */
			if (newskb)
				NF_HOOK(NFPROTO_IPV6, NF_INET_POST_ROUTING,
					sk, newskb, NULL, newskb->dev,
					dev_loopback_xmit);

			if (ipv6_hdr(skb)->hop_limit == 0) {
				IP6_INC_STATS(dev_net(dev), idev,
					      IPSTATS_MIB_OUTDISCARDS);
				kfree_skb(skb);
				return 0;
			}
		}

		IP6_UPD_PO_STATS(dev_net(dev), idev, IPSTATS_MIB_OUTMCAST,
				skb->len);

		if (IPV6_ADDR_MC_SCOPE(&ipv6_hdr(skb)->daddr) <=
		    IPV6_ADDR_SCOPE_NODELOCAL &&
		    !(dev->flags & IFF_LOOPBACK)) {
			kfree_skb(skb);
			return 0;
		}
	}

	rcu_read_lock_bh();
	nexthop = rt6_nexthop((struct rt6_info *)dst);
	neigh = __ipv6_neigh_lookup_noref(dst->dev, nexthop);
	if (unlikely(!neigh))
		neigh = __neigh_create(&nd_tbl, nexthop, dst->dev, false);
	if (!IS_ERR(neigh)) {
		ret = dst_neigh_output(dst, neigh, skb);
		rcu_read_unlock_bh();
		return ret;
	}
	rcu_read_unlock_bh();

	IP6_INC_STATS(dev_net(dst->dev),
		      ip6_dst_idev(dst), IPSTATS_MIB_OUTNOROUTES);
	kfree_skb(skb);
	return -EINVAL;
}

static int ip6_finish_output(struct sock *sk, struct sk_buff *skb)
{
	if ((skb->len > ip6_skb_dst_mtu(skb) && !skb_is_gso(skb)) ||
	    dst_allfrag(skb_dst(skb)) ||
	    (IP6CB(skb)->frag_max_size && skb->len > IP6CB(skb)->frag_max_size))
		return ip6_fragment(sk, skb, ip6_finish_output2);
	else
		return ip6_finish_output2(sk, skb);
}

int ip6_output(struct sock *sk, struct sk_buff *skb)
{
	struct net_device *dev = skb_dst(skb)->dev;
	struct inet6_dev *idev = ip6_dst_idev(skb_dst(skb));
	if (unlikely(idev->cnf.disable_ipv6)) {
		IP6_INC_STATS(dev_net(dev), idev,
			      IPSTATS_MIB_OUTDISCARDS);
		kfree_skb(skb);
		return 0;
	}

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	/*
	 * IMQ-patch: moved setting skb->dev and skb->protocol from
	 * ip6_finish_output2 to fix crashing at netif_skb_features().
	 */
	skb->protocol = htons(ETH_P_IPV6);
	skb->dev = dev;
#endif

	return NF_HOOK_COND(NFPROTO_IPV6, NF_INET_POST_ROUTING, sk, skb,
			    NULL, dev,
			    ip6_finish_output,
			    !(IP6CB(skb)->flags & IP6SKB_REROUTED));
}

/*
 *	xmit an sk_buff (used by TCP, SCTP and DCCP)
 */

int ip6_xmit(struct sock *sk, struct sk_buff *skb, struct flowi6 *fl6,
	     struct ipv6_txoptions *opt, int tclass)
{
	struct net *net = sock_net(sk);
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct in6_addr *first_hop = &fl6->daddr;
	struct dst_entry *dst = skb_dst(skb);
	struct ipv6hdr *hdr;
	u8  proto = fl6->flowi6_proto;
	int seg_len = skb->len;
	int hlimit = -1;
	u32 mtu;

	if (opt) {
		unsigned int head_room;

		/* First: exthdrs may take lots of space (~8K for now)
		   MAX_HEADER is not enough.
		 */
		head_room = opt->opt_nflen + opt->opt_flen;
		seg_len += head_room;
		head_room += sizeof(struct ipv6hdr) + LL_RESERVED_SPACE(dst->dev);

		if (skb_headroom(skb) < head_room) {
			struct sk_buff *skb2 = skb_realloc_headroom(skb, head_room);
			if (!skb2) {
				IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
					      IPSTATS_MIB_OUTDISCARDS);
				kfree_skb(skb);
				return -ENOBUFS;
			}
			consume_skb(skb);
			skb = skb2;
			skb_set_owner_w(skb, sk);
		}
		if (opt->opt_flen)
			ipv6_push_frag_opts(skb, opt, &proto);
		if (opt->opt_nflen)
			ipv6_push_nfrag_opts(skb, opt, &proto, &first_hop);
	}

	skb_push(skb, sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	hdr = ipv6_hdr(skb);

	/*
	 *	Fill in the IPv6 header
	 */
	if (np)
		hlimit = np->hop_limit;
	if (hlimit < 0)
		hlimit = ip6_dst_hoplimit(dst);

	ip6_flow_hdr(hdr, tclass, ip6_make_flowlabel(net, skb, fl6->flowlabel,
						     np->autoflowlabel));

	hdr->payload_len = htons(seg_len);
	hdr->nexthdr = proto;
	hdr->hop_limit = hlimit;

	hdr->saddr = fl6->saddr;
	hdr->daddr = *first_hop;

	skb->protocol = htons(ETH_P_IPV6);
	skb->priority = sk->sk_priority;
	skb->mark = sk->sk_mark;

	mtu = dst_mtu(dst);
	if ((skb->len <= mtu) || skb->ignore_df || skb_is_gso(skb)) {
		IP6_UPD_PO_STATS(net, ip6_dst_idev(skb_dst(skb)),
			      IPSTATS_MIB_OUT, skb->len);
		return NF_HOOK(NFPROTO_IPV6, NF_INET_LOCAL_OUT, sk, skb,
			       NULL, dst->dev, dst_output_sk);
	}

	skb->dev = dst->dev;
	ipv6_local_error(sk, EMSGSIZE, fl6, mtu);
	IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)), IPSTATS_MIB_FRAGFAILS);
	kfree_skb(skb);
	return -EMSGSIZE;
}
EXPORT_SYMBOL(ip6_xmit);

static int ip6_call_ra_chain(struct sk_buff *skb, int sel)
{
	struct ip6_ra_chain *ra;
	struct sock *last = NULL;

	read_lock(&ip6_ra_lock);
	for (ra = ip6_ra_chain; ra; ra = ra->next) {
		struct sock *sk = ra->sk;
		if (sk && ra->sel == sel &&
		    (!sk->sk_bound_dev_if ||
		     sk->sk_bound_dev_if == skb->dev->ifindex)) {
			if (last) {
				struct sk_buff *skb2 = skb_clone(skb, GFP_ATOMIC);
				if (skb2)
					rawv6_rcv(last, skb2);
			}
			last = sk;
		}
	}

	if (last) {
		rawv6_rcv(last, skb);
		read_unlock(&ip6_ra_lock);
		return 1;
	}
	read_unlock(&ip6_ra_lock);
	return 0;
}

static int ip6_forward_proxy_check(struct sk_buff *skb)
{
	struct ipv6hdr *hdr = ipv6_hdr(skb);
	u8 nexthdr = hdr->nexthdr;
	__be16 frag_off;
	int offset;

	if (ipv6_ext_hdr(nexthdr)) {
		offset = ipv6_skip_exthdr(skb, sizeof(*hdr), &nexthdr, &frag_off);
		if (offset < 0)
			return 0;
	} else
		offset = sizeof(struct ipv6hdr);

	if (nexthdr == IPPROTO_ICMPV6) {
		struct icmp6hdr *icmp6;

		if (!pskb_may_pull(skb, (skb_network_header(skb) +
					 offset + 1 - skb->data)))
			return 0;

		icmp6 = (struct icmp6hdr *)(skb_network_header(skb) + offset);

		switch (icmp6->icmp6_type) {
		case NDISC_ROUTER_SOLICITATION:
		case NDISC_ROUTER_ADVERTISEMENT:
		case NDISC_NEIGHBOUR_SOLICITATION:
		case NDISC_NEIGHBOUR_ADVERTISEMENT:
		case NDISC_REDIRECT:
			/* For reaction involving unicast neighbor discovery
			 * message destined to the proxied address, pass it to
			 * input function.
			 */
			return 1;
		default:
			break;
		}
	}

	/*
	 * The proxying router can't forward traffic sent to a link-local
	 * address, so signal the sender and discard the packet. This
	 * behavior is clarified by the MIPv6 specification.
	 */
	if (ipv6_addr_type(&hdr->daddr) & IPV6_ADDR_LINKLOCAL) {
		dst_link_failure(skb);
		return -1;
	}

	return 0;
}

static inline int ip6_forward_finish(struct sock *sk, struct sk_buff *skb)
{
	skb_sender_cpu_clear(skb);
	return dst_output_sk(sk, skb);
}

#if defined(CONFIG_BCM_KF_IP)
static inline int isULA(const struct in6_addr *addr)
{
	__be32 st;

	st = addr->s6_addr32[0];

	/* RFC 4193 */
	if ((st & htonl(0xFE000000)) == htonl(0xFC000000))
		return	1;
	else
		return	0;
}

static inline int isSpecialAddr(const struct in6_addr *addr)
{
	__be32 st;

	st = addr->s6_addr32[0];

	/* RFC 5156 */
	if (((st & htonl(0xFFFFFFFF)) == htonl(0x20010db8)) ||
		((st & htonl(0xFFFFFFF0)) == htonl(0x20010010)))
		return	1;
	else
		return	0;
}
#endif

static unsigned int ip6_dst_mtu_forward(const struct dst_entry *dst)
{
	unsigned int mtu;
	struct inet6_dev *idev;

	if (dst_metric_locked(dst, RTAX_MTU)) {
		mtu = dst_metric_raw(dst, RTAX_MTU);
		if (mtu)
			return mtu;
	}

	mtu = IPV6_MIN_MTU;
	rcu_read_lock();
	idev = __in6_dev_get(dst->dev);
	if (idev)
		mtu = idev->cnf.mtu6;
	rcu_read_unlock();

	return mtu;
}

static bool ip6_pkt_too_big(const struct sk_buff *skb, unsigned int mtu)
{
	if (skb->len <= mtu)
		return false;

	/* ipv6 conntrack defrag sets max_frag_size + ignore_df */
	if (IP6CB(skb)->frag_max_size && IP6CB(skb)->frag_max_size > mtu)
		return true;

	if (skb->ignore_df)
		return false;

	if (skb_is_gso(skb) && skb_gso_network_seglen(skb) <= mtu)
		return false;

	return true;
}

int ip6_forward(struct sk_buff *skb)
{
	struct dst_entry *dst = skb_dst(skb);
	struct ipv6hdr *hdr = ipv6_hdr(skb);
	struct inet6_skb_parm *opt = IP6CB(skb);
	struct net *net = dev_net(dst->dev);
	u32 mtu;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
	int needfrag = 0;
#endif

	if (net->ipv6.devconf_all->forwarding == 0)
		goto error;

	if (skb->pkt_type != PACKET_HOST)
		goto drop;

	if (unlikely(skb->sk))
		goto drop;

	if (skb_warn_if_lro(skb))
		goto drop;

	if (!xfrm6_policy_check(NULL, XFRM_POLICY_FWD, skb)) {
		IP6_INC_STATS_BH(net, ip6_dst_idev(dst),
				 IPSTATS_MIB_INDISCARDS);
		goto drop;
	}

	skb_forward_csum(skb);

	/*
	 *	We DO NOT make any processing on
	 *	RA packets, pushing them to user level AS IS
	 *	without ane WARRANTY that application will be able
	 *	to interpret them. The reason is that we
	 *	cannot make anything clever here.
	 *
	 *	We are not end-node, so that if packet contains
	 *	AH/ESP, we cannot make anything.
	 *	Defragmentation also would be mistake, RA packets
	 *	cannot be fragmented, because there is no warranty
	 *	that different fragments will go along one path. --ANK
	 */
	if (unlikely(opt->flags & IP6SKB_ROUTERALERT)) {
		if (ip6_call_ra_chain(skb, ntohs(opt->ra)))
			return 0;
	}

	/*
	 *	check and decrement ttl
	 */
	if (hdr->hop_limit <= 1) {
		/* Force OUTPUT device used as source address */
		skb->dev = dst->dev;
		icmpv6_send(skb, ICMPV6_TIME_EXCEED, ICMPV6_EXC_HOPLIMIT, 0);
		IP6_INC_STATS_BH(net, ip6_dst_idev(dst),
				 IPSTATS_MIB_INHDRERRORS);

		kfree_skb(skb);
		return -ETIMEDOUT;
	}

#if defined(CONFIG_BCM_KF_IP)
    /* No traffic with ULA address should be forwarded at WAN intf */
	if ( isULA(&hdr->daddr) || isULA(&hdr->saddr) )
		if ((skb->dev->priv_flags & IFF_WANDEV) || 
			(dst->dev->priv_flags & IFF_WANDEV) )
			goto drop;
#endif

	/* XXX: idev->cnf.proxy_ndp? */
	if (net->ipv6.devconf_all->proxy_ndp &&
	    pneigh_lookup(&nd_tbl, net, &hdr->daddr, skb->dev, 0)) {
		int proxied = ip6_forward_proxy_check(skb);
		if (proxied > 0)
			return ip6_input(skb);
		else if (proxied < 0) {
			IP6_INC_STATS_BH(net, ip6_dst_idev(dst),
					 IPSTATS_MIB_INDISCARDS);
			goto drop;
		}
	}

	if (!xfrm6_route_forward(skb)) {
		IP6_INC_STATS_BH(net, ip6_dst_idev(dst),
				 IPSTATS_MIB_INDISCARDS);
		goto drop;
	}
	dst = skb_dst(skb);

	/* IPv6 specs say nothing about it, but it is clear that we cannot
	   send redirects to source routed frames.
	   We don't send redirects to frames decapsulated from IPsec.
	 */
	if (skb->dev == dst->dev && opt->srcrt == 0 && !skb_sec_path(skb)) {
		struct in6_addr *target = NULL;
		struct inet_peer *peer;
		struct rt6_info *rt;

		/*
		 *	incoming and outgoing devices are the same
		 *	send a redirect.
		 */

		rt = (struct rt6_info *) dst;
		if (rt->rt6i_flags & RTF_GATEWAY)
			target = &rt->rt6i_gateway;
		else
			target = &hdr->daddr;

		peer = inet_getpeer_v6(net->ipv6.peers, &rt->rt6i_dst.addr, 1);

		/* Limit redirects both by destination (here)
		   and by source (inside ndisc_send_redirect)
		 */
		if (inet_peer_xrlim_allow(peer, 1*HZ))
			ndisc_send_redirect(skb, target);
		if (peer)
			inet_putpeer(peer);
	} else {
		int addrtype = ipv6_addr_type(&hdr->saddr);

		/* This check is security critical. */
		if (addrtype == IPV6_ADDR_ANY ||
#if defined(CONFIG_BCM_KF_IP)
			/* 
			 * RFC 5156: IPv4 mapped addr and IPv4-compatible addr
			 * should not appear on the Internet. In addition,
			 * 2001:db8::/32 and 2001:10::/28 should not appear either.
			 */
			(addrtype & (IPV6_ADDR_MULTICAST | IPV6_ADDR_LOOPBACK | 
				IPV6_ADDR_COMPATv4 | IPV6_ADDR_MAPPED | 
				IPV6_ADDR_SITELOCAL)) ||
			isSpecialAddr(&hdr->saddr))
#else
		    addrtype & (IPV6_ADDR_MULTICAST | IPV6_ADDR_LOOPBACK))
#endif
			goto error;
		if (addrtype & IPV6_ADDR_LINKLOCAL) {
			icmpv6_send(skb, ICMPV6_DEST_UNREACH,
				    ICMPV6_NOT_NEIGHBOUR, 0);
			goto error;
		}
	}

	mtu = ip6_dst_mtu_forward(dst);
	if (mtu < IPV6_MIN_MTU)
		mtu = IPV6_MIN_MTU;

	if (ip6_pkt_too_big(skb, mtu)) {
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		/*
		 * MAP_FORWARD_MODE2/MAP_FORWARD_MODE3 is used to fragment translated IPv6 packet
		 * for MAP-T/MAP-E feature
		 */
		if (skb->map_forward != MAP_FORWARD_MODE2 && skb->map_forward != MAP_FORWARD_MODE3)
		{
#endif
		/* Again, force OUTPUT device used as source address */
		skb->dev = dst->dev;
		icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
		IP6_INC_STATS_BH(net, ip6_dst_idev(dst),
				 IPSTATS_MIB_INTOOBIGERRORS);
		IP6_INC_STATS_BH(net, ip6_dst_idev(dst),
				 IPSTATS_MIB_FRAGFAILS);
		kfree_skb(skb);
		return -EMSGSIZE;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		}
		else
			needfrag = 1;
	}
	else if ((skb->map_forward == MAP_FORWARD_MODE2) && 
			 (skb->len > (mtu-sizeof(struct frag_hdr)))) {
		needfrag = 1;
#endif
	}

	if (skb_cow(skb, dst->dev->hard_header_len)) {
		IP6_INC_STATS_BH(net, ip6_dst_idev(dst),
				 IPSTATS_MIB_OUTDISCARDS);
		goto drop;
	}

	hdr = ipv6_hdr(skb);

	/* Mangling hops number delayed to point after skb COW */

	hdr->hop_limit--;

#if defined(CONFIG_BCM_KF_WANDEV)
#if !defined(CONFIG_BCM_WAN_2_WAN_FWD_ENABLED)
	/* Never forward a packet from a WAN intf to the other WAN intf */
	if( (skb->dev) && (dst->dev) && 
		((skb->dev->priv_flags & dst->dev->priv_flags) & IFF_WANDEV) )
		goto drop;
#endif
#endif

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
	if (needfrag)
	{
		skb->ignore_df = 1;
		return ip6_fragment(skb->sk, skb, ip6_forward_finish);
	}
#endif

	IP6_INC_STATS_BH(net, ip6_dst_idev(dst), IPSTATS_MIB_OUTFORWDATAGRAMS);
	IP6_ADD_STATS_BH(net, ip6_dst_idev(dst), IPSTATS_MIB_OUTOCTETS, skb->len);
	return NF_HOOK(NFPROTO_IPV6, NF_INET_FORWARD, NULL, skb,
		       skb->dev, dst->dev,
		       ip6_forward_finish);

error:
	IP6_INC_STATS_BH(net, ip6_dst_idev(dst), IPSTATS_MIB_INADDRERRORS);
drop:
	kfree_skb(skb);
	return -EINVAL;
}

static void ip6_copy_metadata(struct sk_buff *to, struct sk_buff *from)
{
	to->pkt_type = from->pkt_type;
	to->priority = from->priority;
	to->protocol = from->protocol;
	skb_dst_drop(to);
	skb_dst_set(to, dst_clone(skb_dst(from)));
	to->dev = from->dev;
	to->mark = from->mark;

#ifdef CONFIG_NET_SCHED
	to->tc_index = from->tc_index;
#endif
	nf_copy(to, from);
	skb_copy_secmark(to, from);
}

int ip6_fragment(struct sock *sk, struct sk_buff *skb,
		 int (*output)(struct sock *, struct sk_buff *))
{
	struct sk_buff *frag;
	struct rt6_info *rt = (struct rt6_info *)skb_dst(skb);
	struct ipv6_pinfo *np = skb->sk && !dev_recursion_level() ?
				inet6_sk(skb->sk) : NULL;
	struct ipv6hdr *tmp_hdr;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
	struct iphdr *tmp_hdr2 = NULL;
	struct iphdr *iph = NULL;
	struct frag_hdr *fh = NULL;
	__be16 not_last_frag = 0;
#else
	struct frag_hdr *fh;
#endif
	unsigned int mtu, hlen, left, len;
	int hroom, troom;
	__be32 frag_id = 0;
	int ptr, offset = 0, err = 0;
	u8 *prevhdr, nexthdr = 0;
	struct net *net = dev_net(skb_dst(skb)->dev);

	err = ip6_find_1stfragopt(skb, &prevhdr);
	if (err < 0)
		goto fail;
	hlen = err;
	nexthdr = *prevhdr;

	mtu = ip6_skb_dst_mtu(skb);

	/* We must not fragment if the socket is set to force MTU discovery
	 * or if the skb it not generated by a local socket.
	 */
	if (unlikely(!skb->ignore_df && skb->len > mtu) ||
		     (IP6CB(skb)->frag_max_size &&
		      IP6CB(skb)->frag_max_size > mtu)) {
		if (skb->sk && dst_allfrag(skb_dst(skb)))
			sk_nocaps_add(skb->sk, NETIF_F_GSO_MASK);

		skb->dev = skb_dst(skb)->dev;
		icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
		IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
			      IPSTATS_MIB_FRAGFAILS);
		kfree_skb(skb);
		return -EMSGSIZE;
	}

	if (np && np->frag_size < mtu) {
		if (np->frag_size)
			mtu = np->frag_size;
	}
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
	mtu -= hlen + (skb->map_forward == MAP_FORWARD_MODE3 ? sizeof(struct iphdr) : sizeof(struct frag_hdr));
#else
	mtu -= hlen + sizeof(struct frag_hdr);
#endif

	if (skb_has_frag_list(skb)) {
		int first_len = skb_pagelen(skb);
		struct sk_buff *frag2;

		if (first_len - hlen > mtu ||
		    ((first_len - hlen) & 7) ||
		    skb_cloned(skb))
			goto slow_path;

		skb_walk_frags(skb, frag) {
			/* Correct geometry. */
			if (frag->len > mtu ||
			    ((frag->len & 7) && frag->next) ||
			    skb_headroom(frag) < hlen)
				goto slow_path_clean;

			/* Partially cloned skb? */
			if (skb_shared(frag))
				goto slow_path_clean;

			BUG_ON(frag->sk);
			if (skb->sk) {
				frag->sk = skb->sk;
				frag->destructor = sock_wfree;
			}
			skb->truesize -= frag->truesize;
		}

		err = 0;
		offset = 0;
		frag = skb_shinfo(skb)->frag_list;
		skb_frag_list_init(skb);
		/* BUILD HEADER */

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		*prevhdr = skb->map_forward == MAP_FORWARD_MODE3 ? IPPROTO_IPIP : NEXTHDR_FRAGMENT;
		if (skb->map_forward == MAP_FORWARD_MODE3) {
			tmp_hdr2 = kmemdup(skb_network_header(skb) + hlen, sizeof(struct iphdr), GFP_ATOMIC);
			if (!tmp_hdr2) {
				IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
					      IPSTATS_MIB_FRAGFAILS);
				return -ENOMEM;
			}
		}
#else
		*prevhdr = NEXTHDR_FRAGMENT;
#endif
		tmp_hdr = kmemdup(skb_network_header(skb), hlen, GFP_ATOMIC);
		if (!tmp_hdr) {
			IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
				      IPSTATS_MIB_FRAGFAILS);
			return -ENOMEM;
		}

		__skb_pull(skb, hlen);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		if (skb->map_forward == MAP_FORWARD_MODE3)
			iph = (struct iphdr *)__skb_push(skb, sizeof(struct iphdr));
		else
#endif
		fh = (struct frag_hdr *)__skb_push(skb, sizeof(struct frag_hdr));
		__skb_push(skb, hlen);
		skb_reset_network_header(skb);
		memcpy(skb_network_header(skb), tmp_hdr, hlen);

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		if (skb->map_forward != MAP_FORWARD_MODE3)
		{
			if (skb->map_id)
				fh->identification = skb->map_id;
			else
#endif
		ipv6_select_ident(net, fh, rt);
		fh->nexthdr = nexthdr;
		fh->reserved = 0;
		fh->frag_off = htons(IP6_MF);
		frag_id = fh->identification;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		}
		else {
			memcpy(skb_network_header(skb) + hlen, tmp_hdr2, sizeof(struct iphdr));
			iph->tot_len = htons(first_len - hlen);
			iph->frag_off = htons(IP_MF);
			if (skb->map_id)
				iph->id = skb->map_id;
			ip_send_check(iph);
		}
#endif

		first_len = skb_pagelen(skb);
		skb->data_len = first_len - skb_headlen(skb);
		skb->len = first_len;
		ipv6_hdr(skb)->payload_len = htons(first_len -
						   sizeof(struct ipv6hdr));

		dst_hold(&rt->dst);

		for (;;) {
			/* Prepare header of the next frame,
			 * before previous one went down. */
			if (frag) {
				frag->ip_summed = CHECKSUM_NONE;
				skb_reset_transport_header(frag);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
				if (skb->map_forward == MAP_FORWARD_MODE3)
					iph = (struct iphdr *)__skb_push(frag, sizeof(struct iphdr));
				else
#endif
				fh = (struct frag_hdr *)__skb_push(frag, sizeof(struct frag_hdr));
				__skb_push(frag, hlen);
				skb_reset_network_header(frag);
				memcpy(skb_network_header(frag), tmp_hdr,
				       hlen);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
				if (skb->map_forward != MAP_FORWARD_MODE3)
				{
#endif
				offset += skb->len - hlen - sizeof(struct frag_hdr);
				fh->nexthdr = nexthdr;
				fh->reserved = 0;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
				fh->frag_off = htons(offset+skb->map_offset);
#else
				fh->frag_off = htons(offset);
#endif
				if (frag->next)
					fh->frag_off |= htons(IP6_MF);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
				else if (skb->map_mf)
					fh->frag_off |= htons(IP6_MF);
#endif
				fh->identification = frag_id;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
				}
				else {
					memcpy(skb_network_header(frag) + hlen, tmp_hdr2,
					       sizeof(struct iphdr));
					iph->tot_len = htons(frag->len - hlen);
					if (offset == 0)
						ip_options_fragment(frag);
					offset += skb->len - hlen - sizeof(struct iphdr);
					iph->frag_off = htons(offset>>3);
					if (frag->next)
						iph->frag_off |= htons(IP_MF);
					/* Ready, complete checksum */
					ip_send_check(iph);
				}
#endif
				ipv6_hdr(frag)->payload_len =
						htons(frag->len -
						      sizeof(struct ipv6hdr));
				ip6_copy_metadata(frag, skb);
			}

			err = output(sk, skb);
			if (!err)
				IP6_INC_STATS(net, ip6_dst_idev(&rt->dst),
					      IPSTATS_MIB_FRAGCREATES);

			if (err || !frag)
				break;

			skb = frag;
			frag = skb->next;
			skb->next = NULL;
		}

		kfree(tmp_hdr);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		kfree(tmp_hdr2);
#endif

		if (err == 0) {
			IP6_INC_STATS(net, ip6_dst_idev(&rt->dst),
				      IPSTATS_MIB_FRAGOKS);
			ip6_rt_put(rt);
			return 0;
		}

		kfree_skb_list(frag);

		IP6_INC_STATS(net, ip6_dst_idev(&rt->dst),
			      IPSTATS_MIB_FRAGFAILS);
		ip6_rt_put(rt);
		return err;

slow_path_clean:
		skb_walk_frags(skb, frag2) {
			if (frag2 == frag)
				break;
			frag2->sk = NULL;
			frag2->destructor = NULL;
			skb->truesize += frag2->truesize;
		}
	}

slow_path:
	if ((skb->ip_summed == CHECKSUM_PARTIAL) &&
	    skb_checksum_help(skb))
		goto fail;

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
	left = skb->len - hlen - (skb->map_forward == MAP_FORWARD_MODE3 ? sizeof(struct iphdr) : 0);
	ptr = hlen + (skb->map_forward == MAP_FORWARD_MODE3 ? sizeof(struct iphdr) : 0);
#else
	left = skb->len - hlen;		/* Space per frame */
	ptr = hlen;			/* Where to start from */
#endif

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
	if (skb->map_forward == MAP_FORWARD_MODE3) {
		tmp_hdr2 = kmemdup(skb_network_header(skb) + hlen, sizeof(struct iphdr), GFP_ATOMIC);
		if (!tmp_hdr2) {
			IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
				      IPSTATS_MIB_FRAGFAILS);
			return -ENOMEM;
		}
		offset = (ntohs(tmp_hdr2->frag_off) & IP_OFFSET) << 3;
		not_last_frag = tmp_hdr2->frag_off & htons(IP_MF);
		kfree(tmp_hdr2);
	}
#endif

	/*
	 *	Fragment the datagram.
	 */

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
	*prevhdr = skb->map_forward == MAP_FORWARD_MODE3 ? IPPROTO_IPIP : NEXTHDR_FRAGMENT;
#else
	*prevhdr = NEXTHDR_FRAGMENT;
#endif
	hroom = LL_RESERVED_SPACE(rt->dst.dev);
	troom = rt->dst.dev->needed_tailroom;

	/*
	 *	Keep copying data until we run out.
	 */
	while (left > 0)	{
		len = left;
		/* IF: it doesn't fit, use 'mtu' - the data space left */
		if (len > mtu)
			len = mtu;
		/* IF: we are not sending up to and including the packet end
		   then align the next start on an eight byte boundary */
		if (len < left)	{
			len &= ~7;
		}

		/* Allocate buffer */
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		frag = alloc_skb(len + hlen + (skb->map_forward == MAP_FORWARD_MODE3 ? sizeof(struct iphdr) : sizeof(struct frag_hdr)) +
				 hroom + troom, GFP_ATOMIC);
#else
		frag = alloc_skb(len + hlen + sizeof(struct frag_hdr) +
				 hroom + troom, GFP_ATOMIC);
#endif
		if (!frag) {
			IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
				      IPSTATS_MIB_FRAGFAILS);
			err = -ENOMEM;
			goto fail;
		}

		/*
		 *	Set up data on packet
		 */

		ip6_copy_metadata(frag, skb);
		skb_reserve(frag, hroom);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		skb_put(frag, len + hlen + (skb->map_forward == MAP_FORWARD_MODE3 ? sizeof(struct iphdr) : sizeof(struct frag_hdr)));
#else
		skb_put(frag, len + hlen + sizeof(struct frag_hdr));
#endif
		skb_reset_network_header(frag);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		if (skb->map_forward != MAP_FORWARD_MODE3)
		{
#endif
		fh = (struct frag_hdr *)(skb_network_header(frag) + hlen);
		frag->transport_header = (frag->network_header + hlen +
					  sizeof(struct frag_hdr));
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		}
		else {
			iph = (struct iphdr *)(skb_network_header(frag) + hlen);
			frag->transport_header = (frag->network_header + hlen +
						  sizeof(struct iphdr));
		}
#endif

		/*
		 *	Charge the memory for the fragment to any owner
		 *	it might possess
		 */
		if (skb->sk)
			skb_set_owner_w(frag, skb->sk);

		/*
		 *	Copy the packet header into the new buffer.
		 */
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		skb_copy_from_linear_data(skb, skb_network_header(frag), hlen + (skb->map_forward == MAP_FORWARD_MODE3 ? sizeof(struct iphdr) : 0));
#else
		skb_copy_from_linear_data(skb, skb_network_header(frag), hlen);
#endif

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		if (skb->map_forward != MAP_FORWARD_MODE3)
		{
#endif
		/*
		 *	Build fragment header.
		 */
		fh->nexthdr = nexthdr;
		fh->reserved = 0;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		if (skb->map_id)
			fh->identification = skb->map_id;
		else
		{
#endif
		if (!frag_id) {
			ipv6_select_ident(net, fh, rt);
			frag_id = fh->identification;
		} else
			fh->identification = frag_id;
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		}
		}
		else {
			/*
			 *	Build pre-fragment/IPv4 header.
			 */
			iph->frag_off = htons((offset >> 3));
			if (offset == 0)
				ip_options_fragment(skb);
			iph->tot_len = htons(len + sizeof(struct iphdr));
			if (skb->map_id)
				iph->id = skb->map_id;
		}
#endif

		/*
		 *	Copy a block of the IP datagram.
		 */
		BUG_ON(skb_copy_bits(skb, ptr, skb_transport_header(frag),
				     len));
		left -= len;

#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		if (skb->map_forward != MAP_FORWARD_MODE3)
		{
		fh->frag_off = htons(offset+skb->map_offset);
#else
		fh->frag_off = htons(offset);
#endif
		if (left > 0)
			fh->frag_off |= htons(IP6_MF);
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		else if (skb->map_mf)
			fh->frag_off |= htons(IP6_MF);
		}
		else {
			if (left > 0 || not_last_frag)
				iph->frag_off |= htons(IP_MF);
		}
#endif
		ipv6_hdr(frag)->payload_len = htons(frag->len -
						    sizeof(struct ipv6hdr));

		ptr += len;
		offset += len;

		/*
		 *	Put this fragment into the sending queue.
		 */
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
		if (skb->map_forward == MAP_FORWARD_MODE3)
			ip_send_check(iph);
#endif
		err = output(sk, frag);
		if (err)
			goto fail;

		IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
			      IPSTATS_MIB_FRAGCREATES);
	}
	IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
		      IPSTATS_MIB_FRAGOKS);
	consume_skb(skb);
	return err;

fail:
	IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
		      IPSTATS_MIB_FRAGFAILS);
	kfree_skb(skb);
	return err;
}
#if defined(CONFIG_BCM_KF_IP)
EXPORT_SYMBOL_GPL(ip6_fragment);
#endif

static inline int ip6_rt_check(const struct rt6key *rt_key,
			       const struct in6_addr *fl_addr,
			       const struct in6_addr *addr_cache)
{
	return (rt_key->plen != 128 || !ipv6_addr_equal(fl_addr, &rt_key->addr)) &&
		(!addr_cache || !ipv6_addr_equal(fl_addr, addr_cache));
}

static struct dst_entry *ip6_sk_dst_check(struct sock *sk,
					  struct dst_entry *dst,
					  const struct flowi6 *fl6)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct rt6_info *rt;

	if (!dst)
		goto out;

	if (dst->ops->family != AF_INET6) {
		dst_release(dst);
		return NULL;
	}

	rt = (struct rt6_info *)dst;
	/* Yes, checking route validity in not connected
	 * case is not very simple. Take into account,
	 * that we do not support routing by source, TOS,
	 * and MSG_DONTROUTE		--ANK (980726)
	 *
	 * 1. ip6_rt_check(): If route was host route,
	 *    check that cached destination is current.
	 *    If it is network route, we still may
	 *    check its validity using saved pointer
	 *    to the last used address: daddr_cache.
	 *    We do not want to save whole address now,
	 *    (because main consumer of this service
	 *    is tcp, which has not this problem),
	 *    so that the last trick works only on connected
	 *    sockets.
	 * 2. oif also should be the same.
	 */
	if (ip6_rt_check(&rt->rt6i_dst, &fl6->daddr, np->daddr_cache) ||
#ifdef CONFIG_IPV6_SUBTREES
	    ip6_rt_check(&rt->rt6i_src, &fl6->saddr, np->saddr_cache) ||
#endif
	    (fl6->flowi6_oif && fl6->flowi6_oif != dst->dev->ifindex)) {
		dst_release(dst);
		dst = NULL;
	}

out:
	return dst;
}

static int ip6_dst_lookup_tail(struct sock *sk,
			       struct dst_entry **dst, struct flowi6 *fl6)
{
	struct net *net = sock_net(sk);
#ifdef CONFIG_IPV6_OPTIMISTIC_DAD
	struct neighbour *n;
	struct rt6_info *rt;
#endif
	int err;
	int flags = 0;

	/* The correct way to handle this would be to do
	 * ip6_route_get_saddr, and then ip6_route_output; however,
	 * the route-specific preferred source forces the
	 * ip6_route_output call _before_ ip6_route_get_saddr.
	 *
	 * In source specific routing (no src=any default route),
	 * ip6_route_output will fail given src=any saddr, though, so
	 * that's why we try it again later.
	 */
	if (ipv6_addr_any(&fl6->saddr) && (!*dst || !(*dst)->error)) {
		struct rt6_info *rt;
		bool had_dst = *dst != NULL;

		if (!had_dst)
			*dst = ip6_route_output(net, sk, fl6);
		rt = (*dst)->error ? NULL : (struct rt6_info *)*dst;
		err = ip6_route_get_saddr(net, rt, &fl6->daddr,
					  sk ? inet6_sk(sk)->srcprefs : 0,
					  &fl6->saddr);
		if (err)
			goto out_err_release;

		/* If we had an erroneous initial result, pretend it
		 * never existed and let the SA-enabled version take
		 * over.
		 */
		if (!had_dst && (*dst)->error) {
			dst_release(*dst);
			*dst = NULL;
		}

		if (fl6->flowi6_oif)
			flags |= RT6_LOOKUP_F_IFACE;
	}

	if (!*dst)
		*dst = ip6_route_output_flags(net, sk, fl6, flags);

	err = (*dst)->error;
	if (err)
		goto out_err_release;

#ifdef CONFIG_IPV6_OPTIMISTIC_DAD
	/*
	 * Here if the dst entry we've looked up
	 * has a neighbour entry that is in the INCOMPLETE
	 * state and the src address from the flow is
	 * marked as OPTIMISTIC, we release the found
	 * dst entry and replace it instead with the
	 * dst entry of the nexthop router
	 */
	rt = (struct rt6_info *) *dst;
	rcu_read_lock_bh();
	n = __ipv6_neigh_lookup_noref(rt->dst.dev, rt6_nexthop(rt));
	err = n && !(n->nud_state & NUD_VALID) ? -EINVAL : 0;
	rcu_read_unlock_bh();

	if (err) {
		struct inet6_ifaddr *ifp;
		struct flowi6 fl_gw6;
		int redirect;

		ifp = ipv6_get_ifaddr(net, &fl6->saddr,
				      (*dst)->dev, 1);

		redirect = (ifp && ifp->flags & IFA_F_OPTIMISTIC);
		if (ifp)
			in6_ifa_put(ifp);

		if (redirect) {
			/*
			 * We need to get the dst entry for the
			 * default router instead
			 */
			dst_release(*dst);
			memcpy(&fl_gw6, fl6, sizeof(struct flowi6));
			memset(&fl_gw6.daddr, 0, sizeof(struct in6_addr));
			*dst = ip6_route_output(net, sk, &fl_gw6);
			err = (*dst)->error;
			if (err)
				goto out_err_release;
		}
	}
#endif
	if (ipv6_addr_v4mapped(&fl6->saddr) &&
	    !(ipv6_addr_v4mapped(&fl6->daddr) || ipv6_addr_any(&fl6->daddr))) {
		err = -EAFNOSUPPORT;
		goto out_err_release;
	}

	return 0;

out_err_release:
	if (err == -ENETUNREACH)
		IP6_INC_STATS(net, NULL, IPSTATS_MIB_OUTNOROUTES);
	dst_release(*dst);
	*dst = NULL;
	return err;
}

/**
 *	ip6_dst_lookup - perform route lookup on flow
 *	@sk: socket which provides route info
 *	@dst: pointer to dst_entry * for result
 *	@fl6: flow to lookup
 *
 *	This function performs a route lookup on the given flow.
 *
 *	It returns zero on success, or a standard errno code on error.
 */
int ip6_dst_lookup(struct sock *sk, struct dst_entry **dst, struct flowi6 *fl6)
{
	*dst = NULL;
	return ip6_dst_lookup_tail(sk, dst, fl6);
}
EXPORT_SYMBOL_GPL(ip6_dst_lookup);

/**
 *	ip6_dst_lookup_flow - perform route lookup on flow with ipsec
 *	@sk: socket which provides route info
 *	@fl6: flow to lookup
 *	@final_dst: final destination address for ipsec lookup
 *
 *	This function performs a route lookup on the given flow.
 *
 *	It returns a valid dst pointer on success, or a pointer encoded
 *	error code.
 */
struct dst_entry *ip6_dst_lookup_flow(struct sock *sk, struct flowi6 *fl6,
				      const struct in6_addr *final_dst)
{
	struct dst_entry *dst = NULL;
	int err;

	err = ip6_dst_lookup_tail(sk, &dst, fl6);
	if (err)
		return ERR_PTR(err);
	if (final_dst)
		fl6->daddr = *final_dst;

	return xfrm_lookup_route(sock_net(sk), dst, flowi6_to_flowi(fl6), sk, 0);
}
EXPORT_SYMBOL_GPL(ip6_dst_lookup_flow);

/**
 *	ip6_sk_dst_lookup_flow - perform socket cached route lookup on flow
 *	@sk: socket which provides the dst cache and route info
 *	@fl6: flow to lookup
 *	@final_dst: final destination address for ipsec lookup
 *
 *	This function performs a route lookup on the given flow with the
 *	possibility of using the cached route in the socket if it is valid.
 *	It will take the socket dst lock when operating on the dst cache.
 *	As a result, this function can only be used in process context.
 *
 *	It returns a valid dst pointer on success, or a pointer encoded
 *	error code.
 */
struct dst_entry *ip6_sk_dst_lookup_flow(struct sock *sk, struct flowi6 *fl6,
					 const struct in6_addr *final_dst)
{
	struct dst_entry *dst = sk_dst_check(sk, inet6_sk(sk)->dst_cookie);
	int err;

	dst = ip6_sk_dst_check(sk, dst, fl6);

	err = ip6_dst_lookup_tail(sk, &dst, fl6);
	if (err)
		return ERR_PTR(err);
	if (final_dst)
		fl6->daddr = *final_dst;

	return xfrm_lookup_route(sock_net(sk), dst, flowi6_to_flowi(fl6), sk, 0);
}
EXPORT_SYMBOL_GPL(ip6_sk_dst_lookup_flow);

static inline int ip6_ufo_append_data(struct sock *sk,
			struct sk_buff_head *queue,
			int getfrag(void *from, char *to, int offset, int len,
			int odd, struct sk_buff *skb),
			void *from, int length, int hh_len, int fragheaderlen,
			int transhdrlen, int mtu, unsigned int flags,
			struct rt6_info *rt)

{
	struct sk_buff *skb;
	struct frag_hdr fhdr;
	int err;

	/* There is support for UDP large send offload by network
	 * device, so create one single skb packet containing complete
	 * udp datagram
	 */
	skb = skb_peek_tail(queue);
	if (!skb) {
		skb = sock_alloc_send_skb(sk,
			hh_len + fragheaderlen + transhdrlen + 20,
			(flags & MSG_DONTWAIT), &err);
		if (!skb)
			return err;

		/* reserve space for Hardware header */
		skb_reserve(skb, hh_len);

		/* create space for UDP/IP header */
		skb_put(skb, fragheaderlen + transhdrlen);

		/* initialize network header pointer */
		skb_reset_network_header(skb);

		/* initialize protocol header pointer */
		skb->transport_header = skb->network_header + fragheaderlen;

		skb->protocol = htons(ETH_P_IPV6);
		skb->csum = 0;

		__skb_queue_tail(queue, skb);
	} else if (skb_is_gso(skb)) {
		goto append;
	}

	skb->ip_summed = CHECKSUM_PARTIAL;
	/* Specify the length of each IPv6 datagram fragment.
	 * It has to be a multiple of 8.
	 */
	skb_shinfo(skb)->gso_size = (mtu - fragheaderlen -
				     sizeof(struct frag_hdr)) & ~7;
	skb_shinfo(skb)->gso_type = SKB_GSO_UDP;
	ipv6_select_ident(sock_net(sk), &fhdr, rt);
	skb_shinfo(skb)->ip6_frag_id = fhdr.identification;

append:
	return skb_append_datato_frags(sk, skb, getfrag, from,
				       (length - transhdrlen));
}

static inline struct ipv6_opt_hdr *ip6_opt_dup(struct ipv6_opt_hdr *src,
					       gfp_t gfp)
{
	return src ? kmemdup(src, (src->hdrlen + 1) * 8, gfp) : NULL;
}

static inline struct ipv6_rt_hdr *ip6_rthdr_dup(struct ipv6_rt_hdr *src,
						gfp_t gfp)
{
	return src ? kmemdup(src, (src->hdrlen + 1) * 8, gfp) : NULL;
}

static void ip6_append_data_mtu(unsigned int *mtu,
				int *maxfraglen,
				unsigned int fragheaderlen,
				struct sk_buff *skb,
				struct rt6_info *rt,
				unsigned int orig_mtu)
{
	if (!(rt->dst.flags & DST_XFRM_TUNNEL)) {
		if (!skb) {
			/* first fragment, reserve header_len */
			*mtu = orig_mtu - rt->dst.header_len;

		} else {
			/*
			 * this fragment is not first, the headers
			 * space is regarded as data space.
			 */
			*mtu = orig_mtu;
		}
		*maxfraglen = ((*mtu - fragheaderlen) & ~7)
			      + fragheaderlen - sizeof(struct frag_hdr);
	}
}

static int ip6_setup_cork(struct sock *sk, struct inet_cork_full *cork,
			  struct inet6_cork *v6_cork,
			  int hlimit, int tclass, struct ipv6_txoptions *opt,
			  struct rt6_info *rt, struct flowi6 *fl6)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	unsigned int mtu;

	/*
	 * setup for corking
	 */
	if (opt) {
		if (WARN_ON(v6_cork->opt))
			return -EINVAL;

		v6_cork->opt = kzalloc(sizeof(*opt), sk->sk_allocation);
		if (unlikely(!v6_cork->opt))
			return -ENOBUFS;

		v6_cork->opt->tot_len = sizeof(*opt);
		v6_cork->opt->opt_flen = opt->opt_flen;
		v6_cork->opt->opt_nflen = opt->opt_nflen;

		v6_cork->opt->dst0opt = ip6_opt_dup(opt->dst0opt,
						    sk->sk_allocation);
		if (opt->dst0opt && !v6_cork->opt->dst0opt)
			return -ENOBUFS;

		v6_cork->opt->dst1opt = ip6_opt_dup(opt->dst1opt,
						    sk->sk_allocation);
		if (opt->dst1opt && !v6_cork->opt->dst1opt)
			return -ENOBUFS;

		v6_cork->opt->hopopt = ip6_opt_dup(opt->hopopt,
						   sk->sk_allocation);
		if (opt->hopopt && !v6_cork->opt->hopopt)
			return -ENOBUFS;

		v6_cork->opt->srcrt = ip6_rthdr_dup(opt->srcrt,
						    sk->sk_allocation);
		if (opt->srcrt && !v6_cork->opt->srcrt)
			return -ENOBUFS;

		/* need source address above miyazawa*/
	}
	dst_hold(&rt->dst);
	cork->base.dst = &rt->dst;
	cork->fl.u.ip6 = *fl6;
	v6_cork->hop_limit = hlimit;
	v6_cork->tclass = tclass;
	if (rt->dst.flags & DST_XFRM_TUNNEL)
		mtu = np->pmtudisc >= IPV6_PMTUDISC_PROBE ?
		      READ_ONCE(rt->dst.dev->mtu) : dst_mtu(&rt->dst);
	else
		mtu = np->pmtudisc >= IPV6_PMTUDISC_PROBE ?
		      READ_ONCE(rt->dst.dev->mtu) : dst_mtu(rt->dst.path);
	if (np->frag_size < mtu) {
		if (np->frag_size)
			mtu = np->frag_size;
	}
	if (mtu < IPV6_MIN_MTU)
		return -EINVAL;
	cork->base.fragsize = mtu;
	if (dst_allfrag(rt->dst.path))
		cork->base.flags |= IPCORK_ALLFRAG;
	cork->base.length = 0;

	return 0;
}

static int __ip6_append_data(struct sock *sk,
			     struct flowi6 *fl6,
			     struct sk_buff_head *queue,
			     struct inet_cork *cork,
			     struct inet6_cork *v6_cork,
			     struct page_frag *pfrag,
			     int getfrag(void *from, char *to, int offset,
					 int len, int odd, struct sk_buff *skb),
			     void *from, int length, int transhdrlen,
			     unsigned int flags, int dontfrag)
{
	struct sk_buff *skb, *skb_prev = NULL;
	unsigned int maxfraglen, fragheaderlen, mtu, orig_mtu;
	int exthdrlen = 0;
	int dst_exthdrlen = 0;
	int hh_len;
	int copy;
	int err;
	int offset = 0;
	__u8 tx_flags = 0;
	u32 tskey = 0;
	struct rt6_info *rt = (struct rt6_info *)cork->dst;
	struct ipv6_txoptions *opt = v6_cork->opt;
	int csummode = CHECKSUM_NONE;

	skb = skb_peek_tail(queue);
	if (!skb) {
		exthdrlen = opt ? opt->opt_flen : 0;
		dst_exthdrlen = rt->dst.header_len - rt->rt6i_nfheader_len;
	}

	mtu = cork->fragsize;
	orig_mtu = mtu;

	hh_len = LL_RESERVED_SPACE(rt->dst.dev);

	fragheaderlen = sizeof(struct ipv6hdr) + rt->rt6i_nfheader_len +
			(opt ? opt->opt_nflen : 0);
	maxfraglen = ((mtu - fragheaderlen) & ~7) + fragheaderlen -
		     sizeof(struct frag_hdr);

	if (mtu <= sizeof(struct ipv6hdr) + IPV6_MAXPLEN) {
		unsigned int maxnonfragsize, headersize;

		headersize = sizeof(struct ipv6hdr) +
			     (opt ? opt->opt_flen + opt->opt_nflen : 0) +
			     (dst_allfrag(&rt->dst) ?
			      sizeof(struct frag_hdr) : 0) +
			     rt->rt6i_nfheader_len;

		if (ip6_sk_ignore_df(sk))
			maxnonfragsize = sizeof(struct ipv6hdr) + IPV6_MAXPLEN;
		else
			maxnonfragsize = mtu;

		/* dontfrag active */
		if ((cork->length + length > mtu - headersize) && dontfrag &&
		    (sk->sk_protocol == IPPROTO_UDP ||
		     sk->sk_protocol == IPPROTO_RAW)) {
			ipv6_local_rxpmtu(sk, fl6, mtu - headersize +
						   sizeof(struct ipv6hdr));
			goto emsgsize;
		}

		if (cork->length + length > maxnonfragsize - headersize) {
emsgsize:
			ipv6_local_error(sk, EMSGSIZE, fl6,
					 mtu - headersize +
					 sizeof(struct ipv6hdr));
			return -EMSGSIZE;
		}
	}

	if (sk->sk_type == SOCK_DGRAM || sk->sk_type == SOCK_RAW) {
		sock_tx_timestamp(sk, &tx_flags);
		if (tx_flags & SKBTX_ANY_SW_TSTAMP &&
		    sk->sk_tsflags & SOF_TIMESTAMPING_OPT_ID)
			tskey = sk->sk_tskey++;
	}

	/* If this is the first and only packet and device
	 * supports checksum offloading, let's use it.
	 * Use transhdrlen, same as IPv4, because partial
	 * sums only work when transhdrlen is set.
	 */
	if (transhdrlen && sk->sk_protocol == IPPROTO_UDP &&
	    length + fragheaderlen < mtu &&
	    rt->dst.dev->features & NETIF_F_V6_CSUM &&
	    !exthdrlen)
		csummode = CHECKSUM_PARTIAL;
	/*
	 * Let's try using as much space as possible.
	 * Use MTU if total length of the message fits into the MTU.
	 * Otherwise, we need to reserve fragment header and
	 * fragment alignment (= 8-15 octects, in total).
	 *
	 * Note that we may need to "move" the data from the tail of
	 * of the buffer to the new fragment when we split
	 * the message.
	 *
	 * FIXME: It may be fragmented into multiple chunks
	 *        at once if non-fragmentable extension headers
	 *        are too large.
	 * --yoshfuji
	 */

	cork->length += length;
	if ((((length + fragheaderlen) > mtu) ||
	     (skb && skb_is_gso(skb))) &&
	    (sk->sk_protocol == IPPROTO_UDP) &&
	    (rt->dst.dev->features & NETIF_F_UFO) &&
	    (sk->sk_type == SOCK_DGRAM) && !udp_get_no_check6_tx(sk)) {
		err = ip6_ufo_append_data(sk, queue, getfrag, from, length,
					  hh_len, fragheaderlen,
					  transhdrlen, mtu, flags, rt);
		if (err)
			goto error;
		return 0;
	}

	if (!skb)
		goto alloc_new_skb;

	while (length > 0) {
		/* Check if the remaining data fits into current packet. */
		copy = (cork->length <= mtu && !(cork->flags & IPCORK_ALLFRAG) ? mtu : maxfraglen) - skb->len;
		if (copy < length)
			copy = maxfraglen - skb->len;

		if (copy <= 0) {
			char *data;
			unsigned int datalen;
			unsigned int fraglen;
			unsigned int fraggap;
			unsigned int alloclen;
alloc_new_skb:
			/* There's no room in the current skb */
			if (skb)
				fraggap = skb->len - maxfraglen;
			else
				fraggap = 0;
			/* update mtu and maxfraglen if necessary */
			if (!skb || !skb_prev)
				ip6_append_data_mtu(&mtu, &maxfraglen,
						    fragheaderlen, skb, rt,
						    orig_mtu);

			skb_prev = skb;

			/*
			 * If remaining data exceeds the mtu,
			 * we know we need more fragment(s).
			 */
			datalen = length + fraggap;

			if (datalen > (cork->length <= mtu && !(cork->flags & IPCORK_ALLFRAG) ? mtu : maxfraglen) - fragheaderlen)
				datalen = maxfraglen - fragheaderlen - rt->dst.trailer_len;
			if ((flags & MSG_MORE) &&
			    !(rt->dst.dev->features&NETIF_F_SG))
				alloclen = mtu;
			else
				alloclen = datalen + fragheaderlen;

			alloclen += dst_exthdrlen;

			if (datalen != length + fraggap) {
				/*
				 * this is not the last fragment, the trailer
				 * space is regarded as data space.
				 */
				datalen += rt->dst.trailer_len;
			}

			alloclen += rt->dst.trailer_len;
			fraglen = datalen + fragheaderlen;

			/*
			 * We just reserve space for fragment header.
			 * Note: this may be overallocation if the message
			 * (without MSG_MORE) fits into the MTU.
			 */
			alloclen += sizeof(struct frag_hdr);

			copy = datalen - transhdrlen - fraggap;
			if (copy < 0) {
				err = -EINVAL;
				goto error;
			}
			if (transhdrlen) {
				skb = sock_alloc_send_skb(sk,
						alloclen + hh_len,
						(flags & MSG_DONTWAIT), &err);
			} else {
				skb = NULL;
				if (atomic_read(&sk->sk_wmem_alloc) <=
				    2 * sk->sk_sndbuf)
					skb = sock_wmalloc(sk,
							   alloclen + hh_len, 1,
							   sk->sk_allocation);
				if (unlikely(!skb))
					err = -ENOBUFS;
			}
			if (!skb)
				goto error;
			/*
			 *	Fill in the control structures
			 */
			skb->protocol = htons(ETH_P_IPV6);
			skb->ip_summed = csummode;
			skb->csum = 0;
			/* reserve for fragmentation and ipsec header */
			skb_reserve(skb, hh_len + sizeof(struct frag_hdr) +
				    dst_exthdrlen);

			/* Only the initial fragment is time stamped */
			skb_shinfo(skb)->tx_flags = tx_flags;
			tx_flags = 0;
			skb_shinfo(skb)->tskey = tskey;
			tskey = 0;

			/*
			 *	Find where to start putting bytes
			 */
			data = skb_put(skb, fraglen);
			skb_set_network_header(skb, exthdrlen);
			data += fragheaderlen;
			skb->transport_header = (skb->network_header +
						 fragheaderlen);
			if (fraggap) {
				skb->csum = skb_copy_and_csum_bits(
					skb_prev, maxfraglen,
					data + transhdrlen, fraggap, 0);
				skb_prev->csum = csum_sub(skb_prev->csum,
							  skb->csum);
				data += fraggap;
				pskb_trim_unique(skb_prev, maxfraglen);
			}
			if (copy > 0 &&
			    getfrag(from, data + transhdrlen, offset,
				    copy, fraggap, skb) < 0) {
				err = -EFAULT;
				kfree_skb(skb);
				goto error;
			}

			offset += copy;
			length -= datalen - fraggap;
			transhdrlen = 0;
			exthdrlen = 0;
			dst_exthdrlen = 0;

			/*
			 * Put the packet on the pending queue
			 */
			__skb_queue_tail(queue, skb);
			continue;
		}

		if (copy > length)
			copy = length;

		if (!(rt->dst.dev->features&NETIF_F_SG)) {
			unsigned int off;

			off = skb->len;
			if (getfrag(from, skb_put(skb, copy),
						offset, copy, off, skb) < 0) {
				__skb_trim(skb, off);
				err = -EFAULT;
				goto error;
			}
		} else {
			int i = skb_shinfo(skb)->nr_frags;

			err = -ENOMEM;
			if (!sk_page_frag_refill(sk, pfrag))
				goto error;

			if (!skb_can_coalesce(skb, i, pfrag->page,
					      pfrag->offset)) {
				err = -EMSGSIZE;
				if (i == MAX_SKB_FRAGS)
					goto error;

				__skb_fill_page_desc(skb, i, pfrag->page,
						     pfrag->offset, 0);
				skb_shinfo(skb)->nr_frags = ++i;
				get_page(pfrag->page);
			}
			copy = min_t(int, copy, pfrag->size - pfrag->offset);
			if (getfrag(from,
				    page_address(pfrag->page) + pfrag->offset,
				    offset, copy, skb->len, skb) < 0)
				goto error_efault;

			pfrag->offset += copy;
			skb_frag_size_add(&skb_shinfo(skb)->frags[i - 1], copy);
			skb->len += copy;
			skb->data_len += copy;
			skb->truesize += copy;
			atomic_add(copy, &sk->sk_wmem_alloc);
		}
		offset += copy;
		length -= copy;
	}

	return 0;

error_efault:
	err = -EFAULT;
error:
	cork->length -= length;
	IP6_INC_STATS(sock_net(sk), rt->rt6i_idev, IPSTATS_MIB_OUTDISCARDS);
	return err;
}

int ip6_append_data(struct sock *sk,
		    int getfrag(void *from, char *to, int offset, int len,
				int odd, struct sk_buff *skb),
		    void *from, int length, int transhdrlen, int hlimit,
		    int tclass, struct ipv6_txoptions *opt, struct flowi6 *fl6,
		    struct rt6_info *rt, unsigned int flags, int dontfrag)
{
	struct inet_sock *inet = inet_sk(sk);
	struct ipv6_pinfo *np = inet6_sk(sk);
	int exthdrlen;
	int err;

	if (flags&MSG_PROBE)
		return 0;
	if (skb_queue_empty(&sk->sk_write_queue)) {
		/*
		 * setup for corking
		 */
		err = ip6_setup_cork(sk, &inet->cork, &np->cork, hlimit,
				     tclass, opt, rt, fl6);
		if (err)
			return err;

		exthdrlen = (opt ? opt->opt_flen : 0);
		length += exthdrlen;
		transhdrlen += exthdrlen;
	} else {
		fl6 = &inet->cork.fl.u.ip6;
		transhdrlen = 0;
	}

	return __ip6_append_data(sk, fl6, &sk->sk_write_queue, &inet->cork.base,
				 &np->cork, sk_page_frag(sk), getfrag,
				 from, length, transhdrlen, flags, dontfrag);
}
EXPORT_SYMBOL_GPL(ip6_append_data);

static void ip6_cork_release(struct inet_cork_full *cork,
			     struct inet6_cork *v6_cork)
{
	if (v6_cork->opt) {
		kfree(v6_cork->opt->dst0opt);
		kfree(v6_cork->opt->dst1opt);
		kfree(v6_cork->opt->hopopt);
		kfree(v6_cork->opt->srcrt);
		kfree(v6_cork->opt);
		v6_cork->opt = NULL;
	}

	if (cork->base.dst) {
		dst_release(cork->base.dst);
		cork->base.dst = NULL;
		cork->base.flags &= ~IPCORK_ALLFRAG;
	}
	memset(&cork->fl, 0, sizeof(cork->fl));
}

struct sk_buff *__ip6_make_skb(struct sock *sk,
			       struct sk_buff_head *queue,
			       struct inet_cork_full *cork,
			       struct inet6_cork *v6_cork)
{
	struct sk_buff *skb, *tmp_skb;
	struct sk_buff **tail_skb;
	struct in6_addr final_dst_buf, *final_dst = &final_dst_buf;
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct net *net = sock_net(sk);
	struct ipv6hdr *hdr;
	struct ipv6_txoptions *opt = v6_cork->opt;
	struct rt6_info *rt = (struct rt6_info *)cork->base.dst;
	struct flowi6 *fl6 = &cork->fl.u.ip6;
	unsigned char proto = fl6->flowi6_proto;

	skb = __skb_dequeue(queue);
	if (!skb)
		goto out;
	tail_skb = &(skb_shinfo(skb)->frag_list);

	/* move skb->data to ip header from ext header */
	if (skb->data < skb_network_header(skb))
		__skb_pull(skb, skb_network_offset(skb));
	while ((tmp_skb = __skb_dequeue(queue)) != NULL) {
		__skb_pull(tmp_skb, skb_network_header_len(skb));
		*tail_skb = tmp_skb;
		tail_skb = &(tmp_skb->next);
		skb->len += tmp_skb->len;
		skb->data_len += tmp_skb->len;
		skb->truesize += tmp_skb->truesize;
		tmp_skb->destructor = NULL;
		tmp_skb->sk = NULL;
	}

	/* Allow local fragmentation. */
	skb->ignore_df = ip6_sk_ignore_df(sk);

	*final_dst = fl6->daddr;
	__skb_pull(skb, skb_network_header_len(skb));
	if (opt && opt->opt_flen)
		ipv6_push_frag_opts(skb, opt, &proto);
	if (opt && opt->opt_nflen)
		ipv6_push_nfrag_opts(skb, opt, &proto, &final_dst);

	skb_push(skb, sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	hdr = ipv6_hdr(skb);

	ip6_flow_hdr(hdr, v6_cork->tclass,
		     ip6_make_flowlabel(net, skb, fl6->flowlabel,
					np->autoflowlabel));
	hdr->hop_limit = v6_cork->hop_limit;
	hdr->nexthdr = proto;
	hdr->saddr = fl6->saddr;
	hdr->daddr = *final_dst;

	skb->priority = sk->sk_priority;
	skb->mark = sk->sk_mark;

	skb_dst_set(skb, dst_clone(&rt->dst));
	IP6_UPD_PO_STATS(net, rt->rt6i_idev, IPSTATS_MIB_OUT, skb->len);
	if (proto == IPPROTO_ICMPV6) {
		struct inet6_dev *idev = ip6_dst_idev(skb_dst(skb));

		ICMP6MSGOUT_INC_STATS(net, idev, icmp6_hdr(skb)->icmp6_type);
		ICMP6_INC_STATS(net, idev, ICMP6_MIB_OUTMSGS);
	}

	ip6_cork_release(cork, v6_cork);
out:
	return skb;
}

int ip6_send_skb(struct sk_buff *skb)
{
	struct net *net = sock_net(skb->sk);
	struct rt6_info *rt = (struct rt6_info *)skb_dst(skb);
	int err;

	err = ip6_local_out(skb);
	if (err) {
		if (err > 0)
			err = net_xmit_errno(err);
		if (err)
			IP6_INC_STATS(net, rt->rt6i_idev,
				      IPSTATS_MIB_OUTDISCARDS);
	}

	return err;
}

int ip6_push_pending_frames(struct sock *sk)
{
	struct sk_buff *skb;

	skb = ip6_finish_skb(sk);
	if (!skb)
		return 0;

	return ip6_send_skb(skb);
}
EXPORT_SYMBOL_GPL(ip6_push_pending_frames);

static void __ip6_flush_pending_frames(struct sock *sk,
				       struct sk_buff_head *queue,
				       struct inet_cork_full *cork,
				       struct inet6_cork *v6_cork)
{
	struct sk_buff *skb;

	while ((skb = __skb_dequeue_tail(queue)) != NULL) {
		if (skb_dst(skb))
			IP6_INC_STATS(sock_net(sk), ip6_dst_idev(skb_dst(skb)),
				      IPSTATS_MIB_OUTDISCARDS);
		kfree_skb(skb);
	}

	ip6_cork_release(cork, v6_cork);
}

void ip6_flush_pending_frames(struct sock *sk)
{
	__ip6_flush_pending_frames(sk, &sk->sk_write_queue,
				   &inet_sk(sk)->cork, &inet6_sk(sk)->cork);
}
EXPORT_SYMBOL_GPL(ip6_flush_pending_frames);

struct sk_buff *ip6_make_skb(struct sock *sk,
			     int getfrag(void *from, char *to, int offset,
					 int len, int odd, struct sk_buff *skb),
			     void *from, int length, int transhdrlen,
			     int hlimit, int tclass,
			     struct ipv6_txoptions *opt, struct flowi6 *fl6,
			     struct rt6_info *rt, unsigned int flags,
			     int dontfrag)
{
	struct inet_cork_full cork;
	struct inet6_cork v6_cork;
	struct sk_buff_head queue;
	int exthdrlen = (opt ? opt->opt_flen : 0);
	int err;

	if (flags & MSG_PROBE)
		return NULL;

	__skb_queue_head_init(&queue);

	cork.base.flags = 0;
	cork.base.addr = 0;
	cork.base.opt = NULL;
	cork.base.dst = NULL;
	v6_cork.opt = NULL;
	err = ip6_setup_cork(sk, &cork, &v6_cork, hlimit, tclass, opt, rt, fl6);
	if (err) {
		ip6_cork_release(&cork, &v6_cork);
		return ERR_PTR(err);
	}

	if (dontfrag < 0)
		dontfrag = inet6_sk(sk)->dontfrag;

	err = __ip6_append_data(sk, fl6, &queue, &cork.base, &v6_cork,
				&current->task_frag, getfrag, from,
				length + exthdrlen, transhdrlen + exthdrlen,
				flags, dontfrag);
	if (err) {
		__ip6_flush_pending_frames(sk, &queue, &cork, &v6_cork);
		return ERR_PTR(err);
	}

	return __ip6_make_skb(sk, &queue, &cork, &v6_cork);
}
