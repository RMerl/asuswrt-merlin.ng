/*
 *	Linux NET3:	GRE over IP protocol decoder.
 *
 *	Authors: Alexey Kuznetsov (kuznet@ms2.inr.ac.ru)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/capability.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/if_arp.h>
#include <linux/mroute.h>
#include <linux/init.h>
#include <linux/in6.h>
#include <linux/inetdevice.h>
#include <linux/igmp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>

#include <net/sock.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/protocol.h>
#include <net/ip_tunnels.h>
#include <net/arp.h>
#include <net/checksum.h>
#include <net/dsfield.h>
#include <net/inet_ecn.h>
#include <net/xfrm.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/rtnetlink.h>
#include <net/gre.h>

#if IS_ENABLED(CONFIG_IPV6)
#include <net/ipv6.h>
#include <net/ip6_fib.h>
#include <net/ip6_route.h>
#endif

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/nbuff.h>
#include <linux/blog.h>
#endif

/*
   Problems & solutions
   --------------------

   1. The most important issue is detecting local dead loops.
   They would cause complete host lockup in transmit, which
   would be "resolved" by stack overflow or, if queueing is enabled,
   with infinite looping in net_bh.

   We cannot track such dead loops during route installation,
   it is infeasible task. The most general solutions would be
   to keep skb->encapsulation counter (sort of local ttl),
   and silently drop packet when it expires. It is a good
   solution, but it supposes maintaining new variable in ALL
   skb, even if no tunneling is used.

   Current solution: xmit_recursion breaks dead loops. This is a percpu
   counter, since when we enter the first ndo_xmit(), cpu migration is
   forbidden. We force an exit if this counter reaches RECURSION_LIMIT

   2. Networking dead loops would not kill routers, but would really
   kill network. IP hop limit plays role of "t->recursion" in this case,
   if we copy it from packet being encapsulated to upper header.
   It is very good solution, but it introduces two problems:

   - Routing protocols, using packets with ttl=1 (OSPF, RIP2),
     do not work over tunnels.
   - traceroute does not work. I planned to relay ICMP from tunnel,
     so that this problem would be solved and traceroute output
     would even more informative. This idea appeared to be wrong:
     only Linux complies to rfc1812 now (yes, guys, Linux is the only
     true router now :-)), all routers (at least, in neighbourhood of mine)
     return only 8 bytes of payload. It is the end.

   Hence, if we want that OSPF worked or traceroute said something reasonable,
   we should search for another solution.

   One of them is to parse packet trying to detect inner encapsulation
   made by our node. It is difficult or even impossible, especially,
   taking into account fragmentation. TO be short, ttl is not solution at all.

   Current solution: The solution was UNEXPECTEDLY SIMPLE.
   We force DF flag on tunnels with preconfigured hop limit,
   that is ALL. :-) Well, it does not remove the problem completely,
   but exponential growth of network traffic is changed to linear
   (branches, that exceed pmtu are pruned) and tunnel mtu
   rapidly degrades to value <68, where looping stops.
   Yes, it is not good if there exists a router in the loop,
   which does not force DF, even when encapsulating packets have DF set.
   But it is not our problem! Nobody could accuse us, we made
   all that we could make. Even if it is your gated who injected
   fatal route to network, even if it were you who configured
   fatal static route: you are innocent. :-)

   Alexey Kuznetsov.
 */

static bool log_ecn_error = true;
module_param(log_ecn_error, bool, 0644);
MODULE_PARM_DESC(log_ecn_error, "Log packets received with corrupted ECN");

static struct rtnl_link_ops ipgre_link_ops __read_mostly;
static int ipgre_tunnel_init(struct net_device *dev);

static int ipgre_net_id __read_mostly;
static int gre_tap_net_id __read_mostly;

static int ipgre_err(struct sk_buff *skb, u32 info,
		     const struct tnl_ptk_info *tpi)
{

	/* All the routers (except for Linux) return only
	   8 bytes of packet payload. It means, that precise relaying of
	   ICMP in the real Internet is absolutely infeasible.

	   Moreover, Cisco "wise men" put GRE key to the third word
	   in GRE header. It makes impossible maintaining even soft
	   state for keyed GRE tunnels with enabled checksum. Tell
	   them "thank you".

	   Well, I wonder, rfc1812 was written by Cisco employee,
	   what the hell these idiots break standards established
	   by themselves???
	   */
	struct net *net = dev_net(skb->dev);
	struct ip_tunnel_net *itn;
	const struct iphdr *iph;
	const int type = icmp_hdr(skb)->type;
	const int code = icmp_hdr(skb)->code;
	struct ip_tunnel *t;

	switch (type) {
	default:
	case ICMP_PARAMETERPROB:
		return PACKET_RCVD;

	case ICMP_DEST_UNREACH:
		switch (code) {
		case ICMP_SR_FAILED:
		case ICMP_PORT_UNREACH:
			/* Impossible event. */
			return PACKET_RCVD;
		default:
			/* All others are translated to HOST_UNREACH.
			   rfc2003 contains "deep thoughts" about NET_UNREACH,
			   I believe they are just ether pollution. --ANK
			 */
			break;
		}
		break;
	case ICMP_TIME_EXCEEDED:
		if (code != ICMP_EXC_TTL)
			return PACKET_RCVD;
		break;

	case ICMP_REDIRECT:
		break;
	}

	if (tpi->proto == htons(ETH_P_TEB))
		itn = net_generic(net, gre_tap_net_id);
	else
		itn = net_generic(net, ipgre_net_id);

	iph = (const struct iphdr *)(icmp_hdr(skb) + 1);
	t = ip_tunnel_lookup(itn, skb->dev->ifindex, tpi->flags,
			     iph->daddr, iph->saddr, tpi->key);

	if (!t)
		return PACKET_REJECT;

	if (t->parms.iph.daddr == 0 ||
	    ipv4_is_multicast(t->parms.iph.daddr))
		return PACKET_RCVD;

	if (t->parms.iph.ttl == 0 && type == ICMP_TIME_EXCEEDED)
		return PACKET_RCVD;

	if (time_before(jiffies, t->err_time + IPTUNNEL_ERR_TIMEO))
		t->err_count++;
	else
		t->err_count = 1;
	t->err_time = jiffies;
	return PACKET_RCVD;
}

static int ipgre_rcv(struct sk_buff *skb, const struct tnl_ptk_info *tpi)
{
	struct net *net = dev_net(skb->dev);
	struct ip_tunnel_net *itn;
	const struct iphdr *iph;
	struct ip_tunnel *tunnel;

	if (tpi->proto == htons(ETH_P_TEB))
		itn = net_generic(net, gre_tap_net_id);
	else
		itn = net_generic(net, ipgre_net_id);

	iph = ip_hdr(skb);
	tunnel = ip_tunnel_lookup(itn, skb->dev->ifindex, tpi->flags,
				  iph->saddr, iph->daddr, tpi->key);

	if (tunnel) {
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
		blog_lock();
		blog_link(IF_DEVICE, blog_ptr(skb), (void*)tunnel->dev, DIR_RX, 
				skb->len);
		blog_link(GRE_TUNL, blog_ptr(skb), (void*)tunnel, DIR_RX, 0);
		blog_link(TOS_MODE, blog_ptr(skb), tunnel, DIR_RX, BLOG_TOS_FIXED);
		blog_unlock();
#endif   
		skb_pop_mac_header(skb);
		ip_tunnel_rcv(tunnel, skb, tpi, log_ecn_error);
		return PACKET_RCVD;
	}
	return PACKET_REJECT;
}

static void __gre_xmit(struct sk_buff *skb, struct net_device *dev,
		       const struct iphdr *tnl_params,
		       __be16 proto)
{
	struct ip_tunnel *tunnel = netdev_priv(dev);
	struct tnl_ptk_info tpi;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_lock();
	blog_link(IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_TX, skb->len);
	blog_link(GRE_TUNL, blog_ptr(skb), (void*)tunnel, DIR_TX, 0);
	blog_link(TOS_MODE, blog_ptr(skb), tunnel, DIR_TX, tnl_params->tos);
	blog_unlock();
#endif   

	tpi.flags = tunnel->parms.o_flags;
	tpi.proto = proto;
	tpi.key = tunnel->parms.o_key;
	if (tunnel->parms.o_flags & TUNNEL_SEQ)
#if (defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG))
		if (!blog_gre_tunnel_accelerated())
			tunnel->o_seqno++;
#else
		tunnel->o_seqno++;
#endif
	tpi.seq = htonl(tunnel->o_seqno);

	/* Push GRE header. */
	gre_build_header(skb, &tpi, tunnel->tun_hlen);

	skb_set_inner_protocol(skb, tpi.proto);

#if (defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG))
	skb->tunl = tunnel;
#endif
	ip_tunnel_xmit(skb, dev, tnl_params, tnl_params->protocol);
}

static netdev_tx_t ipgre_xmit(struct sk_buff *skb,
			      struct net_device *dev)
{
	struct ip_tunnel *tunnel = netdev_priv(dev);
	const struct iphdr *tnl_params;

	if (dev->header_ops) {
		/* Need space for new headers */
		if (skb_cow_head(skb, dev->needed_headroom -
				      (tunnel->hlen + sizeof(struct iphdr))))
			goto free_skb;

		tnl_params = (const struct iphdr *)skb->data;

		/* Pull skb since ip_tunnel_xmit() needs skb->data pointing
		 * to gre header.
		 */
		skb_pull(skb, tunnel->hlen + sizeof(struct iphdr));
		skb_reset_mac_header(skb);
	} else {
		if (skb_cow_head(skb, dev->needed_headroom))
			goto free_skb;

		tnl_params = &tunnel->parms.iph;
	}

	skb = gre_handle_offloads(skb, !!(tunnel->parms.o_flags&TUNNEL_CSUM));
	if (IS_ERR(skb))
		goto out;

	__gre_xmit(skb, dev, tnl_params, skb->protocol);

	return NETDEV_TX_OK;

free_skb:
	kfree_skb(skb);
out:
	dev->stats.tx_dropped++;
	return NETDEV_TX_OK;
}

static netdev_tx_t gre_tap_xmit(struct sk_buff *skb,
				struct net_device *dev)
{
	struct ip_tunnel *tunnel = netdev_priv(dev);

	skb = gre_handle_offloads(skb, !!(tunnel->parms.o_flags&TUNNEL_CSUM));
	if (IS_ERR(skb))
		goto out;

	if (skb_cow_head(skb, dev->needed_headroom))
		goto free_skb;

	__gre_xmit(skb, dev, &tunnel->parms.iph, htons(ETH_P_TEB));

	return NETDEV_TX_OK;

free_skb:
	kfree_skb(skb);
out:
	dev->stats.tx_dropped++;
	return NETDEV_TX_OK;
}

static int ipgre_tunnel_ioctl(struct net_device *dev,
			      struct ifreq *ifr, int cmd)
{
	int err;
	struct ip_tunnel_parm p;

	if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p)))
		return -EFAULT;
	if (cmd == SIOCADDTUNNEL || cmd == SIOCCHGTUNNEL) {
		if (p.iph.version != 4 || p.iph.protocol != IPPROTO_GRE ||
		    p.iph.ihl != 5 || (p.iph.frag_off&htons(~IP_DF)) ||
		    ((p.i_flags|p.o_flags)&(GRE_VERSION|GRE_ROUTING)))
			return -EINVAL;
	}
	p.i_flags = gre_flags_to_tnl_flags(p.i_flags);
	p.o_flags = gre_flags_to_tnl_flags(p.o_flags);

	err = ip_tunnel_ioctl(dev, &p, cmd);
	if (err)
		return err;

	p.i_flags = tnl_flags_to_gre_flags(p.i_flags);
	p.o_flags = tnl_flags_to_gre_flags(p.o_flags);

	if (copy_to_user(ifr->ifr_ifru.ifru_data, &p, sizeof(p)))
		return -EFAULT;
	return 0;
}

/* Nice toy. Unfortunately, useless in real life :-)
   It allows to construct virtual multiprotocol broadcast "LAN"
   over the Internet, provided multicast routing is tuned.


   I have no idea was this bicycle invented before me,
   so that I had to set ARPHRD_IPGRE to a random value.
   I have an impression, that Cisco could make something similar,
   but this feature is apparently missing in IOS<=11.2(8).

   I set up 10.66.66/24 and fec0:6666:6666::0/96 as virtual networks
   with broadcast 224.66.66.66. If you have access to mbone, play with me :-)

   ping -t 255 224.66.66.66

   If nobody answers, mbone does not work.

   ip tunnel add Universe mode gre remote 224.66.66.66 local <Your_real_addr> ttl 255
   ip addr add 10.66.66.<somewhat>/24 dev Universe
   ifconfig Universe up
   ifconfig Universe add fe80::<Your_real_addr>/10
   ifconfig Universe add fec0:6666:6666::<Your_real_addr>/96
   ftp 10.66.66.66
   ...
   ftp fec0:6666:6666::193.233.7.65
   ...
 */
static int ipgre_header(struct sk_buff *skb, struct net_device *dev,
			unsigned short type,
			const void *daddr, const void *saddr, unsigned int len)
{
	struct ip_tunnel *t = netdev_priv(dev);
	struct iphdr *iph;
	struct gre_base_hdr *greh;

	iph = (struct iphdr *)skb_push(skb, t->hlen + sizeof(*iph));
	greh = (struct gre_base_hdr *)(iph+1);
	greh->flags = tnl_flags_to_gre_flags(t->parms.o_flags);
	greh->protocol = htons(type);

	memcpy(iph, &t->parms.iph, sizeof(struct iphdr));

	/* Set the source hardware address. */
	if (saddr)
		memcpy(&iph->saddr, saddr, 4);
	if (daddr)
		memcpy(&iph->daddr, daddr, 4);
	if (iph->daddr)
		return t->hlen + sizeof(*iph);

	return -(t->hlen + sizeof(*iph));
}

static int ipgre_header_parse(const struct sk_buff *skb, unsigned char *haddr)
{
	const struct iphdr *iph = (const struct iphdr *) skb_mac_header(skb);
	memcpy(haddr, &iph->saddr, 4);
	return 4;
}

static const struct header_ops ipgre_header_ops = {
	.create	= ipgre_header,
	.parse	= ipgre_header_parse,
};

#ifdef CONFIG_NET_IPGRE_BROADCAST
static int ipgre_open(struct net_device *dev)
{
	struct ip_tunnel *t = netdev_priv(dev);

	if (ipv4_is_multicast(t->parms.iph.daddr)) {
		struct flowi4 fl4;
		struct rtable *rt;

		rt = ip_route_output_gre(t->net, &fl4,
					 t->parms.iph.daddr,
					 t->parms.iph.saddr,
					 t->parms.o_key,
					 RT_TOS(t->parms.iph.tos),
					 t->parms.link);
		if (IS_ERR(rt))
			return -EADDRNOTAVAIL;
		dev = rt->dst.dev;
		ip_rt_put(rt);
		if (!__in_dev_get_rtnl(dev))
			return -EADDRNOTAVAIL;
		t->mlink = dev->ifindex;
		ip_mc_inc_group(__in_dev_get_rtnl(dev), t->parms.iph.daddr);
	}
	return 0;
}

static int ipgre_close(struct net_device *dev)
{
	struct ip_tunnel *t = netdev_priv(dev);

	if (ipv4_is_multicast(t->parms.iph.daddr) && t->mlink) {
		struct in_device *in_dev;
		in_dev = inetdev_by_index(t->net, t->mlink);
		if (in_dev)
			ip_mc_dec_group(in_dev, t->parms.iph.daddr);
	}
	return 0;
}
#endif

static const struct net_device_ops ipgre_netdev_ops = {
	.ndo_init		= ipgre_tunnel_init,
	.ndo_uninit		= ip_tunnel_uninit,
#ifdef CONFIG_NET_IPGRE_BROADCAST
	.ndo_open		= ipgre_open,
	.ndo_stop		= ipgre_close,
#endif
	.ndo_start_xmit		= ipgre_xmit,
	.ndo_do_ioctl		= ipgre_tunnel_ioctl,
	.ndo_change_mtu		= ip_tunnel_change_mtu,
	.ndo_get_stats64	= ip_tunnel_get_stats64,
	.ndo_get_iflink		= ip_tunnel_get_iflink,
};

#define GRE_FEATURES (NETIF_F_SG |		\
		      NETIF_F_FRAGLIST |	\
		      NETIF_F_HIGHDMA |		\
		      NETIF_F_HW_CSUM)

static void ipgre_tunnel_setup(struct net_device *dev)
{
	dev->netdev_ops		= &ipgre_netdev_ops;
	dev->type		= ARPHRD_IPGRE;
	ip_tunnel_setup(dev, ipgre_net_id);
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	dev->blog_stats_flags |= BLOG_DEV_STAT_FLAG_INCLUDE_ALL;
#endif
}

static void __gre_tunnel_init(struct net_device *dev)
{
	struct ip_tunnel *tunnel;
	int t_hlen;

	tunnel = netdev_priv(dev);
	tunnel->tun_hlen = ip_gre_calc_hlen(tunnel->parms.o_flags);
	tunnel->parms.iph.protocol = IPPROTO_GRE;

	tunnel->hlen = tunnel->tun_hlen + tunnel->encap_hlen;

	t_hlen = tunnel->hlen + sizeof(struct iphdr);

	dev->needed_headroom	= LL_MAX_HEADER + t_hlen + 4;
	dev->mtu		= ETH_DATA_LEN - t_hlen - 4;

	dev->features		|= GRE_FEATURES;
	dev->hw_features	|= GRE_FEATURES;

	if (!(tunnel->parms.o_flags & TUNNEL_SEQ)) {
		/* TCP offload with GRE SEQ is not supported. */
		dev->features    |= NETIF_F_GSO_SOFTWARE;
		dev->hw_features |= NETIF_F_GSO_SOFTWARE;
		/* Can use a lockless transmit, unless we generate
		 * output sequences
		 */
		dev->features |= NETIF_F_LLTX;
	}
}

static int ipgre_tunnel_init(struct net_device *dev)
{
	struct ip_tunnel *tunnel = netdev_priv(dev);
	struct iphdr *iph = &tunnel->parms.iph;

	__gre_tunnel_init(dev);

	memcpy(dev->dev_addr, &iph->saddr, 4);
	memcpy(dev->broadcast, &iph->daddr, 4);

	dev->flags		= IFF_NOARP;
	netif_keep_dst(dev);
	dev->addr_len		= 4;

	if (iph->daddr) {
#ifdef CONFIG_NET_IPGRE_BROADCAST
		if (ipv4_is_multicast(iph->daddr)) {
			if (!iph->saddr)
				return -EINVAL;
			dev->flags = IFF_BROADCAST;
			dev->header_ops = &ipgre_header_ops;
		}
#endif
	} else
		dev->header_ops = &ipgre_header_ops;

	return ip_tunnel_init(dev);
}

static struct gre_cisco_protocol ipgre_protocol = {
	.handler        = ipgre_rcv,
	.err_handler    = ipgre_err,
	.priority       = 0,
};

static int __net_init ipgre_init_net(struct net *net)
{
	return ip_tunnel_init_net(net, ipgre_net_id, &ipgre_link_ops, NULL);
}

static void __net_exit ipgre_exit_net(struct net *net)
{
	struct ip_tunnel_net *itn = net_generic(net, ipgre_net_id);
	ip_tunnel_delete_net(itn, &ipgre_link_ops);
}

static struct pernet_operations ipgre_net_ops = {
	.init = ipgre_init_net,
	.exit = ipgre_exit_net,
	.id   = &ipgre_net_id,
	.size = sizeof(struct ip_tunnel_net),
};

static int ipgre_tunnel_validate(struct nlattr *tb[], struct nlattr *data[])
{
	__be16 flags;

	if (!data)
		return 0;

	flags = 0;
	if (data[IFLA_GRE_IFLAGS])
		flags |= nla_get_be16(data[IFLA_GRE_IFLAGS]);
	if (data[IFLA_GRE_OFLAGS])
		flags |= nla_get_be16(data[IFLA_GRE_OFLAGS]);
	if (flags & (GRE_VERSION|GRE_ROUTING))
		return -EINVAL;

	return 0;
}

static int ipgre_tap_validate(struct nlattr *tb[], struct nlattr *data[])
{
	__be32 daddr;

	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}

	if (!data)
		goto out;

	if (data[IFLA_GRE_REMOTE]) {
		memcpy(&daddr, nla_data(data[IFLA_GRE_REMOTE]), 4);
		if (!daddr)
			return -EINVAL;
	}

out:
	return ipgre_tunnel_validate(tb, data);
}

static void ipgre_netlink_parms(struct nlattr *data[], struct nlattr *tb[],
			       struct ip_tunnel_parm *parms)
{
	memset(parms, 0, sizeof(*parms));

	parms->iph.protocol = IPPROTO_GRE;

	if (!data)
		return;

	if (data[IFLA_GRE_LINK])
		parms->link = nla_get_u32(data[IFLA_GRE_LINK]);

	if (data[IFLA_GRE_IFLAGS])
		parms->i_flags = gre_flags_to_tnl_flags(nla_get_be16(data[IFLA_GRE_IFLAGS]));

	if (data[IFLA_GRE_OFLAGS])
		parms->o_flags = gre_flags_to_tnl_flags(nla_get_be16(data[IFLA_GRE_OFLAGS]));

	if (data[IFLA_GRE_IKEY])
		parms->i_key = nla_get_be32(data[IFLA_GRE_IKEY]);

	if (data[IFLA_GRE_OKEY])
		parms->o_key = nla_get_be32(data[IFLA_GRE_OKEY]);

	if (data[IFLA_GRE_LOCAL])
		parms->iph.saddr = nla_get_in_addr(data[IFLA_GRE_LOCAL]);

	if (data[IFLA_GRE_REMOTE])
		parms->iph.daddr = nla_get_in_addr(data[IFLA_GRE_REMOTE]);

	if (data[IFLA_GRE_TTL])
		parms->iph.ttl = nla_get_u8(data[IFLA_GRE_TTL]);

	if (data[IFLA_GRE_TOS])
		parms->iph.tos = nla_get_u8(data[IFLA_GRE_TOS]);

	if (!data[IFLA_GRE_PMTUDISC] || nla_get_u8(data[IFLA_GRE_PMTUDISC]))
		parms->iph.frag_off = htons(IP_DF);
}

/* This function returns true when ENCAP attributes are present in the nl msg */
static bool ipgre_netlink_encap_parms(struct nlattr *data[],
				      struct ip_tunnel_encap *ipencap)
{
	bool ret = false;

	memset(ipencap, 0, sizeof(*ipencap));

	if (!data)
		return ret;

	if (data[IFLA_GRE_ENCAP_TYPE]) {
		ret = true;
		ipencap->type = nla_get_u16(data[IFLA_GRE_ENCAP_TYPE]);
	}

	if (data[IFLA_GRE_ENCAP_FLAGS]) {
		ret = true;
		ipencap->flags = nla_get_u16(data[IFLA_GRE_ENCAP_FLAGS]);
	}

	if (data[IFLA_GRE_ENCAP_SPORT]) {
		ret = true;
		ipencap->sport = nla_get_be16(data[IFLA_GRE_ENCAP_SPORT]);
	}

	if (data[IFLA_GRE_ENCAP_DPORT]) {
		ret = true;
		ipencap->dport = nla_get_be16(data[IFLA_GRE_ENCAP_DPORT]);
	}

	return ret;
}

static int gre_tap_init(struct net_device *dev)
{
	__gre_tunnel_init(dev);
	dev->priv_flags |= IFF_LIVE_ADDR_CHANGE;

	return ip_tunnel_init(dev);
}

static const struct net_device_ops gre_tap_netdev_ops = {
	.ndo_init		= gre_tap_init,
	.ndo_uninit		= ip_tunnel_uninit,
	.ndo_start_xmit		= gre_tap_xmit,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= ip_tunnel_change_mtu,
	.ndo_get_stats64	= ip_tunnel_get_stats64,
	.ndo_get_iflink		= ip_tunnel_get_iflink,
};

static void ipgre_tap_setup(struct net_device *dev)
{
	ether_setup(dev);
	dev->netdev_ops		= &gre_tap_netdev_ops;
	dev->priv_flags 	|= IFF_LIVE_ADDR_CHANGE;
	ip_tunnel_setup(dev, gre_tap_net_id);
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	dev->blog_stats_flags |= BLOG_DEV_STAT_FLAG_INCLUDE_ALL;
#endif
}

static int ipgre_newlink(struct net *src_net, struct net_device *dev,
			 struct nlattr *tb[], struct nlattr *data[])
{
	struct ip_tunnel_parm p;
	struct ip_tunnel_encap ipencap;

	if (ipgre_netlink_encap_parms(data, &ipencap)) {
		struct ip_tunnel *t = netdev_priv(dev);
		int err = ip_tunnel_encap_setup(t, &ipencap);

		if (err < 0)
			return err;
	}

	ipgre_netlink_parms(data, tb, &p);
	return ip_tunnel_newlink(dev, tb, &p);
}

static int ipgre_changelink(struct net_device *dev, struct nlattr *tb[],
			    struct nlattr *data[])
{
	struct ip_tunnel_parm p;
	struct ip_tunnel_encap ipencap;

	if (ipgre_netlink_encap_parms(data, &ipencap)) {
		struct ip_tunnel *t = netdev_priv(dev);
		int err = ip_tunnel_encap_setup(t, &ipencap);

		if (err < 0)
			return err;
	}

	ipgre_netlink_parms(data, tb, &p);
	return ip_tunnel_changelink(dev, tb, &p);
}

static size_t ipgre_get_size(const struct net_device *dev)
{
	return
		/* IFLA_GRE_LINK */
		nla_total_size(4) +
		/* IFLA_GRE_IFLAGS */
		nla_total_size(2) +
		/* IFLA_GRE_OFLAGS */
		nla_total_size(2) +
		/* IFLA_GRE_IKEY */
		nla_total_size(4) +
		/* IFLA_GRE_OKEY */
		nla_total_size(4) +
		/* IFLA_GRE_LOCAL */
		nla_total_size(4) +
		/* IFLA_GRE_REMOTE */
		nla_total_size(4) +
		/* IFLA_GRE_TTL */
		nla_total_size(1) +
		/* IFLA_GRE_TOS */
		nla_total_size(1) +
		/* IFLA_GRE_PMTUDISC */
		nla_total_size(1) +
		/* IFLA_GRE_ENCAP_TYPE */
		nla_total_size(2) +
		/* IFLA_GRE_ENCAP_FLAGS */
		nla_total_size(2) +
		/* IFLA_GRE_ENCAP_SPORT */
		nla_total_size(2) +
		/* IFLA_GRE_ENCAP_DPORT */
		nla_total_size(2) +
		0;
}

static int ipgre_fill_info(struct sk_buff *skb, const struct net_device *dev)
{
	struct ip_tunnel *t = netdev_priv(dev);
	struct ip_tunnel_parm *p = &t->parms;

	if (nla_put_u32(skb, IFLA_GRE_LINK, p->link) ||
	    nla_put_be16(skb, IFLA_GRE_IFLAGS, tnl_flags_to_gre_flags(p->i_flags)) ||
	    nla_put_be16(skb, IFLA_GRE_OFLAGS, tnl_flags_to_gre_flags(p->o_flags)) ||
	    nla_put_be32(skb, IFLA_GRE_IKEY, p->i_key) ||
	    nla_put_be32(skb, IFLA_GRE_OKEY, p->o_key) ||
	    nla_put_in_addr(skb, IFLA_GRE_LOCAL, p->iph.saddr) ||
	    nla_put_in_addr(skb, IFLA_GRE_REMOTE, p->iph.daddr) ||
	    nla_put_u8(skb, IFLA_GRE_TTL, p->iph.ttl) ||
	    nla_put_u8(skb, IFLA_GRE_TOS, p->iph.tos) ||
	    nla_put_u8(skb, IFLA_GRE_PMTUDISC,
		       !!(p->iph.frag_off & htons(IP_DF))))
		goto nla_put_failure;

	if (nla_put_u16(skb, IFLA_GRE_ENCAP_TYPE,
			t->encap.type) ||
	    nla_put_be16(skb, IFLA_GRE_ENCAP_SPORT,
			 t->encap.sport) ||
	    nla_put_be16(skb, IFLA_GRE_ENCAP_DPORT,
			 t->encap.dport) ||
	    nla_put_u16(skb, IFLA_GRE_ENCAP_FLAGS,
			t->encap.flags))
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -EMSGSIZE;
}

static const struct nla_policy ipgre_policy[IFLA_GRE_MAX + 1] = {
	[IFLA_GRE_LINK]		= { .type = NLA_U32 },
	[IFLA_GRE_IFLAGS]	= { .type = NLA_U16 },
	[IFLA_GRE_OFLAGS]	= { .type = NLA_U16 },
	[IFLA_GRE_IKEY]		= { .type = NLA_U32 },
	[IFLA_GRE_OKEY]		= { .type = NLA_U32 },
	[IFLA_GRE_LOCAL]	= { .len = FIELD_SIZEOF(struct iphdr, saddr) },
	[IFLA_GRE_REMOTE]	= { .len = FIELD_SIZEOF(struct iphdr, daddr) },
	[IFLA_GRE_TTL]		= { .type = NLA_U8 },
	[IFLA_GRE_TOS]		= { .type = NLA_U8 },
	[IFLA_GRE_PMTUDISC]	= { .type = NLA_U8 },
	[IFLA_GRE_ENCAP_TYPE]	= { .type = NLA_U16 },
	[IFLA_GRE_ENCAP_FLAGS]	= { .type = NLA_U16 },
	[IFLA_GRE_ENCAP_SPORT]	= { .type = NLA_U16 },
	[IFLA_GRE_ENCAP_DPORT]	= { .type = NLA_U16 },
};

static struct rtnl_link_ops ipgre_link_ops __read_mostly = {
	.kind		= "gre",
	.maxtype	= IFLA_GRE_MAX,
	.policy		= ipgre_policy,
	.priv_size	= sizeof(struct ip_tunnel),
	.setup		= ipgre_tunnel_setup,
	.validate	= ipgre_tunnel_validate,
	.newlink	= ipgre_newlink,
	.changelink	= ipgre_changelink,
	.dellink	= ip_tunnel_dellink,
	.get_size	= ipgre_get_size,
	.fill_info	= ipgre_fill_info,
	.get_link_net	= ip_tunnel_get_link_net,
};

static struct rtnl_link_ops ipgre_tap_ops __read_mostly = {
	.kind		= "gretap",
	.maxtype	= IFLA_GRE_MAX,
	.policy		= ipgre_policy,
	.priv_size	= sizeof(struct ip_tunnel),
	.setup		= ipgre_tap_setup,
	.validate	= ipgre_tap_validate,
	.newlink	= ipgre_newlink,
	.changelink	= ipgre_changelink,
	.dellink	= ip_tunnel_dellink,
	.get_size	= ipgre_get_size,
	.fill_info	= ipgre_fill_info,
	.get_link_net	= ip_tunnel_get_link_net,
};

static int __net_init ipgre_tap_init_net(struct net *net)
{
	return ip_tunnel_init_net(net, gre_tap_net_id, &ipgre_tap_ops, NULL);
}

static void __net_exit ipgre_tap_exit_net(struct net *net)
{
	struct ip_tunnel_net *itn = net_generic(net, gre_tap_net_id);
	ip_tunnel_delete_net(itn, &ipgre_tap_ops);
}

static struct pernet_operations ipgre_tap_net_ops = {
	.init = ipgre_tap_init_net,
	.exit = ipgre_tap_exit_net,
	.id   = &gre_tap_net_id,
	.size = sizeof(struct ip_tunnel_net),
};


#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
static inline int __bcm_gre_rcv_check(struct ip_tunnel *tunnel, struct iphdr *iph, 
	uint16_t len, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_SEQNO;
	int grehlen = 4;
	int iph_len = iph->ihl<<2;
	__be16 *p = (__be16*)((uint8_t *)iph+iph_len);
	__be16 flags;

	flags = p[0];

	if (tunnel->parms.i_flags & TUNNEL_CSUM) {
		uint16_t csum;

		grehlen += 4;
		csum = *(((__be16 *)p) + 2);

		if (!csum)
			goto no_csum;

		csum = ip_compute_csum((void*)(iph+1), len - iph_len);

		if (csum) {
			tunnel->dev->stats.rx_crc_errors++;
			tunnel->dev->stats.rx_errors++;
			ret = BLOG_GRE_RCV_CHKSUM_ERR;
			goto rcv_done;
		}
	}

no_csum:
	if ((tunnel->parms.i_flags & TUNNEL_KEY) && (flags&GRE_KEY))
		grehlen += 4;

	if (tunnel->parms.i_flags & TUNNEL_SEQ) {
		uint32_t seqno = *(((__be32 *)p) + (grehlen / 4));
		*pkt_seqno = seqno;
		if (tunnel->i_seqno && (s32)(seqno - tunnel->i_seqno) == 0) {
			tunnel->i_seqno = seqno + 1;
			ret = BLOG_GRE_RCV_IN_SEQ;
		} else if (tunnel->i_seqno && (s32)(seqno - tunnel->i_seqno) < 0) {
			tunnel->dev->stats.rx_fifo_errors++;
			tunnel->dev->stats.rx_errors++;
			ret = BLOG_GRE_RCV_OOS_LT;
		} else {
			tunnel->i_seqno = seqno + 1;
			ret = BLOG_GRE_RCV_OOS_GT;
		}
	}

rcv_done:
	return ret;
}

int bcm_gre_rcv_check(struct net_device *dev, struct iphdr *iph, 
	uint16_t len, void **tunl, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_TUNNEL;

	struct net *net = dev_net(dev);
	struct ip_tunnel_net *itn;
	struct ip_tunnel *tunnel;
    struct tnl_ptk_info tpi;

	int iph_len = iph->ihl<<2;
	struct gre_base_hdr *greh = (struct gre_base_hdr *)((uint8_t *)iph+iph_len);
    __be32 *options =(__be32*)(__be32 *)(greh + 1);

	tpi.flags = gre_flags_to_tnl_flags(greh->flags);
    tpi.proto = greh->protocol;

	if (greh->flags & GRE_CSUM) {
		options++;
	}

	if (greh->flags & GRE_KEY) {
		tpi.key = *options;
		options++;
	} else
		tpi.key = 0;

#if 1
	if (tpi.proto == htons(ETH_P_TEB))
		itn = net_generic(net, gre_tap_net_id);
	else
#endif
		itn = net_generic(net, ipgre_net_id);

	tunnel = ip_tunnel_lookup(itn, dev->ifindex, tpi.flags,
				  iph->saddr, iph->daddr, tpi.key);

    if (tunnel) {
        rcu_read_lock();
        ret =  __bcm_gre_rcv_check(tunnel, iph, len, pkt_seqno);
        rcu_read_unlock();
    }

	*tunl = (void *) tunnel;
	return ret;
}

/* Adds the TX seqno, Key and updates the GRE checksum */
static inline 
void __bcm_gre_xmit_update(struct ip_tunnel *tunnel, struct iphdr *iph, 
	uint16_t len)
{
	/* IPV4 Tunnel doesn't use GRE flags, uses TUNNEL flags */
	if (tunnel->parms.o_flags&(TUNNEL_KEY|TUNNEL_CSUM|TUNNEL_SEQ)) {
		int iph_len = iph->ihl<<2;
		/* tunnel->encap_hlen is only used for FOU, GUE. Otherwise it's 0 */
		__be32 *ptr = (__be32*)(((u8*)iph) + iph_len + tunnel->hlen - 4);

		if (tunnel->parms.o_flags&TUNNEL_SEQ) {
			++tunnel->o_seqno;
			*ptr = htonl(tunnel->o_seqno);
			ptr--;
		}

		if (tunnel->parms.o_flags&TUNNEL_KEY) {
			*ptr = tunnel->parms.o_key;
			ptr--;
		}

		if (tunnel->parms.o_flags&TUNNEL_CSUM) {
			*ptr = 0;
			*(__sum16*)ptr = ip_compute_csum((void*)(iph+1), len - iph_len);
		}
		cache_flush_len(ptr, tunnel->hlen);
	}
}

/* Adds the oseqno and updates the GRE checksum */
void bcm_gre_xmit_update(struct ip_tunnel *tunnel, struct iphdr *iph, 
	uint16_t len)
{
	rcu_read_lock();
	__bcm_gre_xmit_update(tunnel, iph, len);
	rcu_read_unlock();
}

#endif

static int __init ipgre_init(void)
{
	int err;

	pr_info("GRE over IPv4 tunneling driver\n");

	err = register_pernet_device(&ipgre_net_ops);
	if (err < 0)
		return err;

	err = register_pernet_device(&ipgre_tap_net_ops);
	if (err < 0)
		goto pnet_tap_faied;

	err = gre_cisco_register(&ipgre_protocol);
	if (err < 0) {
		pr_info("%s: can't add protocol\n", __func__);
		goto add_proto_failed;
	}

	err = rtnl_link_register(&ipgre_link_ops);
	if (err < 0)
		goto rtnl_link_failed;

	err = rtnl_link_register(&ipgre_tap_ops);
	if (err < 0)
		goto tap_ops_failed;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_gre_rcv_check_fn = (blog_gre_rcv_check_t) bcm_gre_rcv_check;
	blog_gre_xmit_update_fn = (blog_gre_xmit_upd_t) bcm_gre_xmit_update;
#endif

	return 0;

tap_ops_failed:
	rtnl_link_unregister(&ipgre_link_ops);
rtnl_link_failed:
	gre_cisco_unregister(&ipgre_protocol);
add_proto_failed:
	unregister_pernet_device(&ipgre_tap_net_ops);
pnet_tap_faied:
	unregister_pernet_device(&ipgre_net_ops);
	return err;
}

static void __exit ipgre_fini(void)
{
	rtnl_link_unregister(&ipgre_tap_ops);
	rtnl_link_unregister(&ipgre_link_ops);
	gre_cisco_unregister(&ipgre_protocol);
	unregister_pernet_device(&ipgre_tap_net_ops);
	unregister_pernet_device(&ipgre_net_ops);
}

module_init(ipgre_init);
module_exit(ipgre_fini);
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK("gre");
MODULE_ALIAS_RTNL_LINK("gretap");
MODULE_ALIAS_NETDEV("gre0");
MODULE_ALIAS_NETDEV("gretap0");
