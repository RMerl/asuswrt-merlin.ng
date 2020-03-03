/*
 *	GRE over IPv6 protocol decoder.
 *
 *	Authors: Dmitry Kozlov (xeb@mail.ru)
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
#include <linux/uaccess.h>
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
#include <linux/hash.h>
#include <linux/if_tunnel.h>
#include <linux/ip6_tunnel.h>

#include <net/sock.h>
#include <net/ip.h>
#include <net/ip_tunnels.h>
#include <net/icmp.h>
#include <net/protocol.h>
#include <net/addrconf.h>
#include <net/arp.h>
#include <net/checksum.h>
#include <net/dsfield.h>
#include <net/inet_ecn.h>
#include <net/xfrm.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/rtnetlink.h>

#include <net/ipv6.h>
#include <net/ip6_fib.h>
#include <net/ip6_route.h>
#include <net/ip6_tunnel.h>

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/nbuff.h>
#include <linux/blog.h>
#endif

static bool log_ecn_error = true;
module_param(log_ecn_error, bool, 0644);
MODULE_PARM_DESC(log_ecn_error, "Log packets received with corrupted ECN");

#define HASH_SIZE_SHIFT  5
#define HASH_SIZE (1 << HASH_SIZE_SHIFT)

static int ip6gre_net_id __read_mostly;
struct ip6gre_net {
	struct ip6_tnl __rcu *tunnels[4][HASH_SIZE];

	struct net_device *fb_tunnel_dev;
};

static struct rtnl_link_ops ip6gre_link_ops __read_mostly;
static struct rtnl_link_ops ip6gre_tap_ops __read_mostly;
static int ip6gre_tunnel_init(struct net_device *dev);
static void ip6gre_tunnel_setup(struct net_device *dev);
static void ip6gre_tunnel_link(struct ip6gre_net *ign, struct ip6_tnl *t);
static void ip6gre_tnl_link_config(struct ip6_tnl *t, int set_mtu);

/* Tunnel hash table */

/*
   4 hash tables:

   3: (remote,local)
   2: (remote,*)
   1: (*,local)
   0: (*,*)

   We require exact key match i.e. if a key is present in packet
   it will match only tunnel with the same key; if it is not present,
   it will match only keyless tunnel.

   All keysless packets, if not matched configured keyless tunnels
   will match fallback tunnel.
 */

#define HASH_KEY(key) (((__force u32)key^((__force u32)key>>4))&(HASH_SIZE - 1))
static u32 HASH_ADDR(const struct in6_addr *addr)
{
	u32 hash = ipv6_addr_hash(addr);

	return hash_32(hash, HASH_SIZE_SHIFT);
}

#define tunnels_r_l	tunnels[3]
#define tunnels_r	tunnels[2]
#define tunnels_l	tunnels[1]
#define tunnels_wc	tunnels[0]

/* Given src, dst and key, find appropriate for input tunnel. */

static struct ip6_tnl *ip6gre_tunnel_lookup(struct net_device *dev,
		const struct in6_addr *remote, const struct in6_addr *local,
		__be32 key, __be16 gre_proto)
{
	struct net *net = dev_net(dev);
	int link = dev->ifindex;
	unsigned int h0 = HASH_ADDR(remote);
	unsigned int h1 = HASH_KEY(key);
	struct ip6_tnl *t, *cand = NULL;
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);
	int dev_type = (gre_proto == htons(ETH_P_TEB)) ?
		       ARPHRD_ETHER : ARPHRD_IP6GRE;
	int score, cand_score = 4;

	for_each_ip_tunnel_rcu(t, ign->tunnels_r_l[h0 ^ h1]) {
		if (!ipv6_addr_equal(local, &t->parms.laddr) ||
		    !ipv6_addr_equal(remote, &t->parms.raddr) ||
		    key != t->parms.i_key ||
		    !(t->dev->flags & IFF_UP))
			continue;

		if (t->dev->type != ARPHRD_IP6GRE &&
		    t->dev->type != dev_type)
			continue;

		score = 0;
		if (t->parms.link != link)
			score |= 1;
		if (t->dev->type != dev_type)
			score |= 2;
		if (score == 0)
			return t;

		if (score < cand_score) {
			cand = t;
			cand_score = score;
		}
	}

	for_each_ip_tunnel_rcu(t, ign->tunnels_r[h0 ^ h1]) {
		if (!ipv6_addr_equal(remote, &t->parms.raddr) ||
		    key != t->parms.i_key ||
		    !(t->dev->flags & IFF_UP))
			continue;

		if (t->dev->type != ARPHRD_IP6GRE &&
		    t->dev->type != dev_type)
			continue;

		score = 0;
		if (t->parms.link != link)
			score |= 1;
		if (t->dev->type != dev_type)
			score |= 2;
		if (score == 0)
			return t;

		if (score < cand_score) {
			cand = t;
			cand_score = score;
		}
	}

	for_each_ip_tunnel_rcu(t, ign->tunnels_l[h1]) {
		if ((!ipv6_addr_equal(local, &t->parms.laddr) &&
			  (!ipv6_addr_equal(local, &t->parms.raddr) ||
				 !ipv6_addr_is_multicast(local))) ||
		    key != t->parms.i_key ||
		    !(t->dev->flags & IFF_UP))
			continue;

		if (t->dev->type != ARPHRD_IP6GRE &&
		    t->dev->type != dev_type)
			continue;

		score = 0;
		if (t->parms.link != link)
			score |= 1;
		if (t->dev->type != dev_type)
			score |= 2;
		if (score == 0)
			return t;

		if (score < cand_score) {
			cand = t;
			cand_score = score;
		}
	}

	for_each_ip_tunnel_rcu(t, ign->tunnels_wc[h1]) {
		if (t->parms.i_key != key ||
		    !(t->dev->flags & IFF_UP))
			continue;

		if (t->dev->type != ARPHRD_IP6GRE &&
		    t->dev->type != dev_type)
			continue;

		score = 0;
		if (t->parms.link != link)
			score |= 1;
		if (t->dev->type != dev_type)
			score |= 2;
		if (score == 0)
			return t;

		if (score < cand_score) {
			cand = t;
			cand_score = score;
		}
	}

	if (cand)
		return cand;

	dev = ign->fb_tunnel_dev;
	if (dev->flags & IFF_UP)
		return netdev_priv(dev);

	return NULL;
}

static struct ip6_tnl __rcu **__ip6gre_bucket(struct ip6gre_net *ign,
		const struct __ip6_tnl_parm *p)
{
	const struct in6_addr *remote = &p->raddr;
	const struct in6_addr *local = &p->laddr;
	unsigned int h = HASH_KEY(p->i_key);
	int prio = 0;

	if (!ipv6_addr_any(local))
		prio |= 1;
	if (!ipv6_addr_any(remote) && !ipv6_addr_is_multicast(remote)) {
		prio |= 2;
		h ^= HASH_ADDR(remote);
	}

	return &ign->tunnels[prio][h];
}

static inline struct ip6_tnl __rcu **ip6gre_bucket(struct ip6gre_net *ign,
		const struct ip6_tnl *t)
{
	return __ip6gre_bucket(ign, &t->parms);
}

static void ip6gre_tunnel_link(struct ip6gre_net *ign, struct ip6_tnl *t)
{
	struct ip6_tnl __rcu **tp = ip6gre_bucket(ign, t);

	rcu_assign_pointer(t->next, rtnl_dereference(*tp));
	rcu_assign_pointer(*tp, t);
}

static void ip6gre_tunnel_unlink(struct ip6gre_net *ign, struct ip6_tnl *t)
{
	struct ip6_tnl __rcu **tp;
	struct ip6_tnl *iter;

	for (tp = ip6gre_bucket(ign, t);
	     (iter = rtnl_dereference(*tp)) != NULL;
	     tp = &iter->next) {
		if (t == iter) {
			rcu_assign_pointer(*tp, t->next);
			break;
		}
	}
}

static struct ip6_tnl *ip6gre_tunnel_find(struct net *net,
					   const struct __ip6_tnl_parm *parms,
					   int type)
{
	const struct in6_addr *remote = &parms->raddr;
	const struct in6_addr *local = &parms->laddr;
	__be32 key = parms->i_key;
	int link = parms->link;
	struct ip6_tnl *t;
	struct ip6_tnl __rcu **tp;
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);

	for (tp = __ip6gre_bucket(ign, parms);
	     (t = rtnl_dereference(*tp)) != NULL;
	     tp = &t->next)
		if (ipv6_addr_equal(local, &t->parms.laddr) &&
		    ipv6_addr_equal(remote, &t->parms.raddr) &&
		    key == t->parms.i_key &&
		    link == t->parms.link &&
		    type == t->dev->type)
			break;

	return t;
}

static struct ip6_tnl *ip6gre_tunnel_locate(struct net *net,
		const struct __ip6_tnl_parm *parms, int create)
{
	struct ip6_tnl *t, *nt;
	struct net_device *dev;
	char name[IFNAMSIZ];
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);

	t = ip6gre_tunnel_find(net, parms, ARPHRD_IP6GRE);
	if (t && create)
		return NULL;
	if (t || !create)
		return t;

	if (parms->name[0]) {
		if (!dev_valid_name(parms->name))
			return NULL;
		strlcpy(name, parms->name, IFNAMSIZ);
	} else {
		strcpy(name, "ip6gre%d");
	}
	dev = alloc_netdev(sizeof(*t), name, NET_NAME_UNKNOWN,
			   ip6gre_tunnel_setup);
	if (!dev)
		return NULL;

	dev_net_set(dev, net);

	nt = netdev_priv(dev);
	nt->parms = *parms;
	dev->rtnl_link_ops = &ip6gre_link_ops;

	nt->dev = dev;
	nt->net = dev_net(dev);
	ip6gre_tnl_link_config(nt, 1);

	if (register_netdevice(dev) < 0)
		goto failed_free;

	/* Can use a lockless transmit, unless we generate output sequences */
	if (!(nt->parms.o_flags & GRE_SEQ))
		dev->features |= NETIF_F_LLTX;

	dev_hold(dev);
	ip6gre_tunnel_link(ign, nt);
	return nt;

failed_free:
	free_netdev(dev);
	return NULL;
}

static void ip6gre_tunnel_uninit(struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct ip6gre_net *ign = net_generic(t->net, ip6gre_net_id);

	ip6gre_tunnel_unlink(ign, t);
	ip6_tnl_dst_reset(t);
	dev_put(dev);
}


static void ip6gre_err(struct sk_buff *skb, struct inet6_skb_parm *opt,
		u8 type, u8 code, int offset, __be32 info)
{
	const struct ipv6hdr *ipv6h = (const struct ipv6hdr *)skb->data;
	__be16 *p = (__be16 *)(skb->data + offset);
	int grehlen = offset + 4;
	struct ip6_tnl *t;
	__be16 flags;

	flags = p[0];
	if (flags&(GRE_CSUM|GRE_KEY|GRE_SEQ|GRE_ROUTING|GRE_VERSION)) {
		if (flags&(GRE_VERSION|GRE_ROUTING))
			return;
		if (flags&GRE_KEY) {
			grehlen += 4;
			if (flags&GRE_CSUM)
				grehlen += 4;
		}
	}

	/* If only 8 bytes returned, keyed message will be dropped here */
	if (!pskb_may_pull(skb, grehlen))
		return;
	ipv6h = (const struct ipv6hdr *)skb->data;
	p = (__be16 *)(skb->data + offset);

	t = ip6gre_tunnel_lookup(skb->dev, &ipv6h->daddr, &ipv6h->saddr,
				flags & GRE_KEY ?
				*(((__be32 *)p) + (grehlen / 4) - 1) : 0,
				p[1]);
	if (!t)
		return;

	switch (type) {
		__u32 teli;
		struct ipv6_tlv_tnl_enc_lim *tel;
		__u32 mtu;
	case ICMPV6_DEST_UNREACH:
		net_warn_ratelimited("%s: Path to destination invalid or inactive!\n",
				     t->parms.name);
		break;
	case ICMPV6_TIME_EXCEED:
		if (code == ICMPV6_EXC_HOPLIMIT) {
			net_warn_ratelimited("%s: Too small hop limit or routing loop in tunnel!\n",
					     t->parms.name);
		}
		break;
	case ICMPV6_PARAMPROB:
		teli = 0;
		if (code == ICMPV6_HDR_FIELD)
			teli = ip6_tnl_parse_tlv_enc_lim(skb, skb->data);

		if (teli && teli == be32_to_cpu(info) - 2) {
			tel = (struct ipv6_tlv_tnl_enc_lim *) &skb->data[teli];
			if (tel->encap_limit == 0) {
				net_warn_ratelimited("%s: Too small encapsulation limit or routing loop in tunnel!\n",
						     t->parms.name);
			}
		} else {
			net_warn_ratelimited("%s: Recipient unable to parse tunneled packet!\n",
					     t->parms.name);
		}
		break;
	case ICMPV6_PKT_TOOBIG:
		mtu = be32_to_cpu(info) - offset;
		if (mtu < IPV6_MIN_MTU)
			mtu = IPV6_MIN_MTU;
		t->dev->mtu = mtu;
		break;
	}

	if (time_before(jiffies, t->err_time + IP6TUNNEL_ERR_TIMEO))
		t->err_count++;
	else
		t->err_count = 1;
	t->err_time = jiffies;
}

static int ip6gre_rcv(struct sk_buff *skb)
{
	const struct ipv6hdr *ipv6h;
	u8     *h;
	__be16    flags;
	__sum16   csum = 0;
	__be32 key = 0;
	u32    seqno = 0;
	struct ip6_tnl *tunnel;
	int    offset = 4;
	__be16 gre_proto;
	int err;

	if (!pskb_may_pull(skb, sizeof(struct in6_addr)))
		goto drop;

	ipv6h = ipv6_hdr(skb);
	h = skb->data;
	flags = *(__be16 *)h;

	if (flags&(GRE_CSUM|GRE_KEY|GRE_ROUTING|GRE_SEQ|GRE_VERSION)) {
		/* - Version must be 0.
		   - We do not support routing headers.
		 */
		if (flags&(GRE_VERSION|GRE_ROUTING))
			goto drop;

		if (flags&GRE_CSUM) {
			csum = skb_checksum_simple_validate(skb);
			offset += 4;
		}
		if (flags&GRE_KEY) {
			key = *(__be32 *)(h + offset);
			offset += 4;
		}
		if (flags&GRE_SEQ) {
			seqno = ntohl(*(__be32 *)(h + offset));
			offset += 4;
		}
	}

	gre_proto = *(__be16 *)(h + 2);

	tunnel = ip6gre_tunnel_lookup(skb->dev,
					  &ipv6h->saddr, &ipv6h->daddr, key,
					  gre_proto);
	if (tunnel) {
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
		blog_link(IF_DEVICE, blog_ptr(skb), (void*)tunnel->dev, DIR_RX, 
				skb->len);
		blog_link(GRE_TUNL, blog_ptr(skb), (void*)tunnel, DIR_RX, 0);
		blog_link(TOS_MODE, blog_ptr(skb), tunnel, DIR_RX, BLOG_TOS_FIXED);
#endif   

		struct pcpu_sw_netstats *tstats;

		if (!xfrm6_policy_check(NULL, XFRM_POLICY_IN, skb))
			goto drop;

		if (!ip6_tnl_rcv_ctl(tunnel, &ipv6h->daddr, &ipv6h->saddr)) {
			tunnel->dev->stats.rx_dropped++;
			goto drop;
		}

		skb->protocol = gre_proto;
		/* WCCP version 1 and 2 protocol decoding.
		 * - Change protocol to IPv6
		 * - When dealing with WCCPv2, Skip extra 4 bytes in GRE header
		 */
		if (flags == 0 && gre_proto == htons(ETH_P_WCCP)) {
			skb->protocol = htons(ETH_P_IPV6);
			if ((*(h + offset) & 0xF0) != 0x40)
				offset += 4;
		}

		skb->mac_header = skb->network_header;
		__pskb_pull(skb, offset);
		skb_postpull_rcsum(skb, skb_transport_header(skb), offset);

		if (((flags&GRE_CSUM) && csum) ||
		    (!(flags&GRE_CSUM) && tunnel->parms.i_flags&GRE_CSUM)) {
			tunnel->dev->stats.rx_crc_errors++;
			tunnel->dev->stats.rx_errors++;
			goto drop;
		}
		if (tunnel->parms.i_flags&GRE_SEQ) {
			if (!(flags&GRE_SEQ) ||
			    (tunnel->i_seqno &&
					(s32)(seqno - tunnel->i_seqno) < 0)) {
				tunnel->dev->stats.rx_fifo_errors++;
				tunnel->dev->stats.rx_errors++;
				goto drop;
			}
			tunnel->i_seqno = seqno + 1;
		}

		/* Warning: All skb pointers will be invalidated! */
		if (tunnel->dev->type == ARPHRD_ETHER) {
			if (!pskb_may_pull(skb, ETH_HLEN)) {
				tunnel->dev->stats.rx_length_errors++;
				tunnel->dev->stats.rx_errors++;
				goto drop;
			}

			ipv6h = ipv6_hdr(skb);
			skb->protocol = eth_type_trans(skb, tunnel->dev);
			skb_postpull_rcsum(skb, eth_hdr(skb), ETH_HLEN);
		}

		__skb_tunnel_rx(skb, tunnel->dev, tunnel->net);

		skb_reset_network_header(skb);

		err = IP6_ECN_decapsulate(ipv6h, skb);
		if (unlikely(err)) {
			if (log_ecn_error)
				net_info_ratelimited("non-ECT from %pI6 with dsfield=%#x\n",
						     &ipv6h->saddr,
						     ipv6_get_dsfield(ipv6h));
			if (err > 1) {
				++tunnel->dev->stats.rx_frame_errors;
				++tunnel->dev->stats.rx_errors;
				goto drop;
			}
		}

		tstats = this_cpu_ptr(tunnel->dev->tstats);
		u64_stats_update_begin(&tstats->syncp);
		tstats->rx_packets++;
		tstats->rx_bytes += skb->len;
		u64_stats_update_end(&tstats->syncp);

		netif_rx(skb);

		return 0;
	}
	icmpv6_send(skb, ICMPV6_DEST_UNREACH, ICMPV6_PORT_UNREACH, 0);

drop:
	kfree_skb(skb);
	return 0;
}

struct ipv6_tel_txoption {
	struct ipv6_txoptions ops;
	__u8 dst_opt[8];
};

static void init_tel_txopt(struct ipv6_tel_txoption *opt, __u8 encap_limit)
{
	memset(opt, 0, sizeof(struct ipv6_tel_txoption));

	opt->dst_opt[2] = IPV6_TLV_TNL_ENCAP_LIMIT;
	opt->dst_opt[3] = 1;
	opt->dst_opt[4] = encap_limit;
	opt->dst_opt[5] = IPV6_TLV_PADN;
	opt->dst_opt[6] = 1;

	opt->ops.dst0opt = (struct ipv6_opt_hdr *) opt->dst_opt;
	opt->ops.opt_nflen = 8;
}

static netdev_tx_t ip6gre_xmit2(struct sk_buff *skb,
			 struct net_device *dev,
			 __u8 dsfield,
			 struct flowi6 *fl6,
			 int encap_limit,
			 __u32 *pmtu)
{
	struct ip6_tnl *tunnel = netdev_priv(dev);
	struct net *net = tunnel->net;
	struct net_device *tdev;    /* Device to other host */
	struct ipv6hdr  *ipv6h;     /* Our new IP header */
	unsigned int max_headroom = 0; /* The extra header space needed */
	int    gre_hlen;
	struct ipv6_tel_txoption opt;
	int    mtu;
	struct dst_entry *dst = NULL, *ndst = NULL;
	struct net_device_stats *stats = &tunnel->dev->stats;
	int err = -1;
	u8 proto;
	struct sk_buff *new_skb;
	__be16 protocol;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_link(IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_TX, skb->len);
	blog_link(GRE_TUNL, blog_ptr(skb), (void*)tunnel, DIR_TX, 0);
	blog_link(TOS_MODE, blog_ptr(skb), tunnel, DIR_TX,
		(tunnel->parms.flags & IP6_TNL_F_USE_ORIG_TCLASS) ?
			BLOG_TOS_INHERIT : BLOG_TOS_FIXED);
#endif

	if (dev->type == ARPHRD_ETHER)
		IPCB(skb)->flags = 0;

	if (dev->header_ops && dev->type == ARPHRD_IP6GRE) {
		gre_hlen = 0;
		ipv6h = (struct ipv6hdr *)skb->data;
		fl6->daddr = ipv6h->daddr;
	} else {
		gre_hlen = tunnel->hlen;
		fl6->daddr = tunnel->parms.raddr;
	}

	if (!fl6->flowi6_mark)
		dst = ip6_tnl_dst_check(tunnel);

	if (!dst) {
		ndst = ip6_route_output(net, NULL, fl6);

		if (ndst->error)
			goto tx_err_link_failure;
		ndst = xfrm_lookup(net, ndst, flowi6_to_flowi(fl6), NULL, 0);
		if (IS_ERR(ndst)) {
			err = PTR_ERR(ndst);
			ndst = NULL;
			goto tx_err_link_failure;
		}
		dst = ndst;
	}

	tdev = dst->dev;

	if (tdev == dev) {
		stats->collisions++;
		net_warn_ratelimited("%s: Local routing loop detected!\n",
				     tunnel->parms.name);
		goto tx_err_dst_release;
	}

	mtu = dst_mtu(dst) - sizeof(*ipv6h);
	if (encap_limit >= 0) {
		max_headroom += 8;
		mtu -= 8;
	}
	if (mtu < IPV6_MIN_MTU)
		mtu = IPV6_MIN_MTU;
	if (skb_dst(skb))
		skb_dst(skb)->ops->update_pmtu(skb_dst(skb), NULL, skb, mtu);
	if (skb->len > mtu) {
		*pmtu = mtu;
		err = -EMSGSIZE;
		goto tx_err_dst_release;
	}

	if (tunnel->err_count > 0) {
		if (time_before(jiffies,
				tunnel->err_time + IP6TUNNEL_ERR_TIMEO)) {
			tunnel->err_count--;

			dst_link_failure(skb);
		} else
			tunnel->err_count = 0;
	}

	skb_scrub_packet(skb, !net_eq(tunnel->net, dev_net(dev)));

	max_headroom += LL_RESERVED_SPACE(tdev) + gre_hlen + dst->header_len;

	if (skb_headroom(skb) < max_headroom || skb_shared(skb) ||
	    (skb_cloned(skb) && !skb_clone_writable(skb, 0))) {
		new_skb = skb_realloc_headroom(skb, max_headroom);
		if (max_headroom > dev->needed_headroom)
			dev->needed_headroom = max_headroom;
		if (!new_skb)
			goto tx_err_dst_release;

		if (skb->sk)
			skb_set_owner_w(new_skb, skb->sk);
		consume_skb(skb);
		skb = new_skb;
	}

	if (fl6->flowi6_mark) {
		skb_dst_set(skb, dst);
		ndst = NULL;
	} else {
		skb_dst_set_noref(skb, dst);
	}

	proto = NEXTHDR_GRE;
	if (encap_limit >= 0) {
		init_tel_txopt(&opt, encap_limit);
		ipv6_push_nfrag_opts(skb, &opt.ops, &proto, NULL);
	}

	if (likely(!skb->encapsulation)) {
		skb_reset_inner_headers(skb);
		skb->encapsulation = 1;
	}

	skb_push(skb, gre_hlen);
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, sizeof(*ipv6h));

	/*
	 *	Push down and install the IP header.
	 */
	ipv6h = ipv6_hdr(skb);
	ip6_flow_hdr(ipv6h, INET_ECN_encapsulate(0, dsfield),
		     ip6_make_flowlabel(net, skb, fl6->flowlabel, false));
	ipv6h->hop_limit = tunnel->parms.hop_limit;
	ipv6h->nexthdr = proto;
	ipv6h->saddr = fl6->saddr;
	ipv6h->daddr = fl6->daddr;

	((__be16 *)(ipv6h + 1))[0] = tunnel->parms.o_flags;
	protocol = (dev->type == ARPHRD_ETHER) ?
		    htons(ETH_P_TEB) : skb->protocol;
	((__be16 *)(ipv6h + 1))[1] = protocol;

	if (tunnel->parms.o_flags&(GRE_KEY|GRE_CSUM|GRE_SEQ)) {
		__be32 *ptr = (__be32 *)(((u8 *)ipv6h) + tunnel->hlen - 4);

		if (tunnel->parms.o_flags&GRE_SEQ) {
#if (defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG))
		if (!blog_gre_tunnel_accelerated())
			++tunnel->o_seqno;
#else
			++tunnel->o_seqno;
#endif
			*ptr = htonl(tunnel->o_seqno);
			ptr--;
		}
		if (tunnel->parms.o_flags&GRE_KEY) {
			*ptr = tunnel->parms.o_key;
			ptr--;
		}
		if (tunnel->parms.o_flags&GRE_CSUM) {
			*ptr = 0;
			*(__sum16 *)ptr = ip_compute_csum((void *)(ipv6h+1),
				skb->len - sizeof(struct ipv6hdr));
		}
	}

	skb_set_inner_protocol(skb, protocol);

#if (defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG))
	skb->tunl = tunnel;
#endif
	ip6tunnel_xmit(NULL, skb, dev);
	if (ndst)
		ip6_tnl_dst_store(tunnel, ndst);
	return 0;
tx_err_link_failure:
	stats->tx_carrier_errors++;
	dst_link_failure(skb);
tx_err_dst_release:
	dst_release(ndst);
	return err;
}

static inline int ip6gre_xmit_ipv4(struct sk_buff *skb, struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	const struct iphdr  *iph = ip_hdr(skb);
	int encap_limit = -1;
	struct flowi6 fl6;
	__u8 dsfield;
	__u32 mtu;
	int err;

	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));

	if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
		encap_limit = t->parms.encap_limit;

	memcpy(&fl6, &t->fl.u.ip6, sizeof(fl6));
	fl6.flowi6_proto = IPPROTO_GRE;

	dsfield = ipv4_get_dsfield(iph);

	if (t->parms.flags & IP6_TNL_F_USE_ORIG_TCLASS)
		fl6.flowlabel |= htonl((__u32)iph->tos << IPV6_TCLASS_SHIFT)
					  & IPV6_TCLASS_MASK;
	if (t->parms.flags & IP6_TNL_F_USE_ORIG_FWMARK)
		fl6.flowi6_mark = skb->mark;

	err = ip6gre_xmit2(skb, dev, dsfield, &fl6, encap_limit, &mtu);
	if (err != 0) {
		/* XXX: send ICMP error even if DF is not set. */
		if (err == -EMSGSIZE)
			icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED,
				  htonl(mtu));
		return -1;
	}

	return 0;
}

static inline int ip6gre_xmit_ipv6(struct sk_buff *skb, struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct ipv6hdr *ipv6h = ipv6_hdr(skb);
	int encap_limit = -1;
	__u16 offset;
	struct flowi6 fl6;
	__u8 dsfield;
	__u32 mtu;
	int err;

	if (ipv6_addr_equal(&t->parms.raddr, &ipv6h->saddr))
		return -1;

	offset = ip6_tnl_parse_tlv_enc_lim(skb, skb_network_header(skb));
	if (offset > 0) {
		struct ipv6_tlv_tnl_enc_lim *tel;
		tel = (struct ipv6_tlv_tnl_enc_lim *)&skb_network_header(skb)[offset];
		if (tel->encap_limit == 0) {
			icmpv6_send(skb, ICMPV6_PARAMPROB,
				    ICMPV6_HDR_FIELD, offset + 2);
			return -1;
		}
		encap_limit = tel->encap_limit - 1;
	} else if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
		encap_limit = t->parms.encap_limit;

	memcpy(&fl6, &t->fl.u.ip6, sizeof(fl6));
	fl6.flowi6_proto = IPPROTO_GRE;

	dsfield = ipv6_get_dsfield(ipv6h);
	if (t->parms.flags & IP6_TNL_F_USE_ORIG_TCLASS)
		fl6.flowlabel |= (*(__be32 *) ipv6h & IPV6_TCLASS_MASK);
	if (t->parms.flags & IP6_TNL_F_USE_ORIG_FLOWLABEL)
		fl6.flowlabel |= ip6_flowlabel(ipv6h);
	if (t->parms.flags & IP6_TNL_F_USE_ORIG_FWMARK)
		fl6.flowi6_mark = skb->mark;

	err = ip6gre_xmit2(skb, dev, dsfield, &fl6, encap_limit, &mtu);
	if (err != 0) {
		if (err == -EMSGSIZE)
			icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
		return -1;
	}

	return 0;
}

/**
 * ip6_tnl_addr_conflict - compare packet addresses to tunnel's own
 *   @t: the outgoing tunnel device
 *   @hdr: IPv6 header from the incoming packet
 *
 * Description:
 *   Avoid trivial tunneling loop by checking that tunnel exit-point
 *   doesn't match source of incoming packet.
 *
 * Return:
 *   1 if conflict,
 *   0 else
 **/

static inline bool ip6gre_tnl_addr_conflict(const struct ip6_tnl *t,
	const struct ipv6hdr *hdr)
{
	return ipv6_addr_equal(&t->parms.raddr, &hdr->saddr);
}

static int ip6gre_xmit_other(struct sk_buff *skb, struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	int encap_limit = -1;
	struct flowi6 fl6;
	__u32 mtu;
	int err;

	if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
		encap_limit = t->parms.encap_limit;

	memcpy(&fl6, &t->fl.u.ip6, sizeof(fl6));
	fl6.flowi6_proto = skb->protocol;

	err = ip6gre_xmit2(skb, dev, 0, &fl6, encap_limit, &mtu);

	return err;
}

static netdev_tx_t ip6gre_tunnel_xmit(struct sk_buff *skb,
	struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct net_device_stats *stats = &t->dev->stats;
	int ret;

	if (!ip6_tnl_xmit_ctl(t, &t->parms.laddr, &t->parms.raddr))
		goto tx_err;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		ret = ip6gre_xmit_ipv4(skb, dev);
		break;
	case htons(ETH_P_IPV6):
		ret = ip6gre_xmit_ipv6(skb, dev);
		break;
	default:
		ret = ip6gre_xmit_other(skb, dev);
		break;
	}

	if (ret < 0)
		goto tx_err;

	return NETDEV_TX_OK;

tx_err:
	stats->tx_errors++;
	stats->tx_dropped++;
	kfree_skb(skb);
	return NETDEV_TX_OK;
}

static void ip6gre_tnl_link_config(struct ip6_tnl *t, int set_mtu)
{
	struct net_device *dev = t->dev;
	struct __ip6_tnl_parm *p = &t->parms;
	struct flowi6 *fl6 = &t->fl.u.ip6;
	int addend = sizeof(struct ipv6hdr) + 4;

	if (dev->type != ARPHRD_ETHER) {
		memcpy(dev->dev_addr, &p->laddr, sizeof(struct in6_addr));
		memcpy(dev->broadcast, &p->raddr, sizeof(struct in6_addr));
	}

	/* Set up flowi template */
	fl6->saddr = p->laddr;
	fl6->daddr = p->raddr;
	fl6->flowi6_oif = p->link;
	fl6->flowlabel = 0;

	if (!(p->flags&IP6_TNL_F_USE_ORIG_TCLASS))
		fl6->flowlabel |= IPV6_TCLASS_MASK & p->flowinfo;
	if (!(p->flags&IP6_TNL_F_USE_ORIG_FLOWLABEL))
		fl6->flowlabel |= IPV6_FLOWLABEL_MASK & p->flowinfo;

	p->flags &= ~(IP6_TNL_F_CAP_XMIT|IP6_TNL_F_CAP_RCV|IP6_TNL_F_CAP_PER_PACKET);
	p->flags |= ip6_tnl_get_cap(t, &p->laddr, &p->raddr);

	if (p->flags&IP6_TNL_F_CAP_XMIT &&
			p->flags&IP6_TNL_F_CAP_RCV && dev->type != ARPHRD_ETHER)
		dev->flags |= IFF_POINTOPOINT;
	else
		dev->flags &= ~IFF_POINTOPOINT;

	/* Precalculate GRE options length */
	if (t->parms.o_flags&(GRE_CSUM|GRE_KEY|GRE_SEQ)) {
		if (t->parms.o_flags&GRE_CSUM)
			addend += 4;
		if (t->parms.o_flags&GRE_KEY)
			addend += 4;
		if (t->parms.o_flags&GRE_SEQ)
			addend += 4;
	}
	t->hlen = addend;

	if (p->flags & IP6_TNL_F_CAP_XMIT) {
		int strict = (ipv6_addr_type(&p->raddr) &
			      (IPV6_ADDR_MULTICAST|IPV6_ADDR_LINKLOCAL));

		struct rt6_info *rt = rt6_lookup(t->net,
						 &p->raddr, &p->laddr,
						 p->link, strict);

		if (!rt)
			return;

		if (rt->dst.dev) {
			dev->hard_header_len = rt->dst.dev->hard_header_len + addend;

			if (set_mtu) {
				dev->mtu = rt->dst.dev->mtu - addend;
				if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
					dev->mtu -= 8;

				if (dev->mtu < IPV6_MIN_MTU)
					dev->mtu = IPV6_MIN_MTU;
			}
		}
		ip6_rt_put(rt);
	}
}

static int ip6gre_tnl_change(struct ip6_tnl *t,
	const struct __ip6_tnl_parm *p, int set_mtu)
{
	t->parms.laddr = p->laddr;
	t->parms.raddr = p->raddr;
	t->parms.flags = p->flags;
	t->parms.hop_limit = p->hop_limit;
	t->parms.encap_limit = p->encap_limit;
	t->parms.flowinfo = p->flowinfo;
	t->parms.link = p->link;
	t->parms.proto = p->proto;
	t->parms.i_key = p->i_key;
	t->parms.o_key = p->o_key;
	t->parms.i_flags = p->i_flags;
	t->parms.o_flags = p->o_flags;
	ip6_tnl_dst_reset(t);
	ip6gre_tnl_link_config(t, set_mtu);
	return 0;
}

static void ip6gre_tnl_parm_from_user(struct __ip6_tnl_parm *p,
	const struct ip6_tnl_parm2 *u)
{
	p->laddr = u->laddr;
	p->raddr = u->raddr;
	p->flags = u->flags;
	p->hop_limit = u->hop_limit;
	p->encap_limit = u->encap_limit;
	p->flowinfo = u->flowinfo;
	p->link = u->link;
	p->i_key = u->i_key;
	p->o_key = u->o_key;
	p->i_flags = u->i_flags;
	p->o_flags = u->o_flags;
	memcpy(p->name, u->name, sizeof(u->name));
}

static void ip6gre_tnl_parm_to_user(struct ip6_tnl_parm2 *u,
	const struct __ip6_tnl_parm *p)
{
	u->proto = IPPROTO_GRE;
	u->laddr = p->laddr;
	u->raddr = p->raddr;
	u->flags = p->flags;
	u->hop_limit = p->hop_limit;
	u->encap_limit = p->encap_limit;
	u->flowinfo = p->flowinfo;
	u->link = p->link;
	u->i_key = p->i_key;
	u->o_key = p->o_key;
	u->i_flags = p->i_flags;
	u->o_flags = p->o_flags;
	memcpy(u->name, p->name, sizeof(u->name));
}

static int ip6gre_tunnel_ioctl(struct net_device *dev,
	struct ifreq *ifr, int cmd)
{
	int err = 0;
	struct ip6_tnl_parm2 p;
	struct __ip6_tnl_parm p1;
	struct ip6_tnl *t = netdev_priv(dev);
	struct net *net = t->net;
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);

	switch (cmd) {
	case SIOCGETTUNNEL:
		if (dev == ign->fb_tunnel_dev) {
			if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p))) {
				err = -EFAULT;
				break;
			}
			ip6gre_tnl_parm_from_user(&p1, &p);
			t = ip6gre_tunnel_locate(net, &p1, 0);
			if (!t)
				t = netdev_priv(dev);
		}
		memset(&p, 0, sizeof(p));
		ip6gre_tnl_parm_to_user(&p, &t->parms);
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &p, sizeof(p)))
			err = -EFAULT;
		break;

	case SIOCADDTUNNEL:
	case SIOCCHGTUNNEL:
		err = -EPERM;
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			goto done;

		err = -EFAULT;
		if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p)))
			goto done;

		err = -EINVAL;
		if ((p.i_flags|p.o_flags)&(GRE_VERSION|GRE_ROUTING))
			goto done;

		if (!(p.i_flags&GRE_KEY))
			p.i_key = 0;
		if (!(p.o_flags&GRE_KEY))
			p.o_key = 0;

		ip6gre_tnl_parm_from_user(&p1, &p);
		t = ip6gre_tunnel_locate(net, &p1, cmd == SIOCADDTUNNEL);

		if (dev != ign->fb_tunnel_dev && cmd == SIOCCHGTUNNEL) {
			if (t) {
				if (t->dev != dev) {
					err = -EEXIST;
					break;
				}
			} else {
				t = netdev_priv(dev);

				ip6gre_tunnel_unlink(ign, t);
				synchronize_net();
				ip6gre_tnl_change(t, &p1, 1);
				ip6gre_tunnel_link(ign, t);
				netdev_state_change(dev);
			}
		}

		if (t) {
			err = 0;

			memset(&p, 0, sizeof(p));
			ip6gre_tnl_parm_to_user(&p, &t->parms);
			if (copy_to_user(ifr->ifr_ifru.ifru_data, &p, sizeof(p)))
				err = -EFAULT;
		} else
			err = (cmd == SIOCADDTUNNEL ? -ENOBUFS : -ENOENT);
		break;

	case SIOCDELTUNNEL:
		err = -EPERM;
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			goto done;

		if (dev == ign->fb_tunnel_dev) {
			err = -EFAULT;
			if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p)))
				goto done;
			err = -ENOENT;
			ip6gre_tnl_parm_from_user(&p1, &p);
			t = ip6gre_tunnel_locate(net, &p1, 0);
			if (!t)
				goto done;
			err = -EPERM;
			if (t == netdev_priv(ign->fb_tunnel_dev))
				goto done;
			dev = t->dev;
		}
		unregister_netdevice(dev);
		err = 0;
		break;

	default:
		err = -EINVAL;
	}

done:
	return err;
}

static int ip6gre_tunnel_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 ||
	    new_mtu > 0xFFF8 - dev->hard_header_len)
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

static int ip6gre_header(struct sk_buff *skb, struct net_device *dev,
			unsigned short type,
			const void *daddr, const void *saddr, unsigned int len)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct ipv6hdr *ipv6h = (struct ipv6hdr *)skb_push(skb, t->hlen);
	__be16 *p = (__be16 *)(ipv6h+1);

	ip6_flow_hdr(ipv6h, 0,
		     ip6_make_flowlabel(dev_net(dev), skb,
					t->fl.u.ip6.flowlabel, false));
	ipv6h->hop_limit = t->parms.hop_limit;
	ipv6h->nexthdr = NEXTHDR_GRE;
	ipv6h->saddr = t->parms.laddr;
	ipv6h->daddr = t->parms.raddr;

	p[0]		= t->parms.o_flags;
	p[1]		= htons(type);

	/*
	 *	Set the source hardware address.
	 */

	if (saddr)
		memcpy(&ipv6h->saddr, saddr, sizeof(struct in6_addr));
	if (daddr)
		memcpy(&ipv6h->daddr, daddr, sizeof(struct in6_addr));
	if (!ipv6_addr_any(&ipv6h->daddr))
		return t->hlen;

	return -t->hlen;
}

static const struct header_ops ip6gre_header_ops = {
	.create	= ip6gre_header,
};

static const struct net_device_ops ip6gre_netdev_ops = {
	.ndo_init		= ip6gre_tunnel_init,
	.ndo_uninit		= ip6gre_tunnel_uninit,
	.ndo_start_xmit		= ip6gre_tunnel_xmit,
	.ndo_do_ioctl		= ip6gre_tunnel_ioctl,
	.ndo_change_mtu		= ip6gre_tunnel_change_mtu,
	.ndo_get_stats64	= ip_tunnel_get_stats64,
	.ndo_get_iflink		= ip6_tnl_get_iflink,
};

static void ip6gre_dev_free(struct net_device *dev)
{
	free_percpu(dev->tstats);
	free_netdev(dev);
}

static void ip6gre_tunnel_setup(struct net_device *dev)
{
	struct ip6_tnl *t;

	dev->netdev_ops = &ip6gre_netdev_ops;
	dev->destructor = ip6gre_dev_free;

	dev->type = ARPHRD_IP6GRE;
	dev->hard_header_len = LL_MAX_HEADER + sizeof(struct ipv6hdr) + 4;
	dev->mtu = ETH_DATA_LEN - sizeof(struct ipv6hdr) - 4;
	t = netdev_priv(dev);
	if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
		dev->mtu -= 8;
	dev->flags |= IFF_NOARP;
	dev->addr_len = sizeof(struct in6_addr);
	netif_keep_dst(dev);
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	dev->blog_stats_flags |= BLOG_DEV_STAT_FLAG_INCLUDE_ALL;
#endif
}

static int ip6gre_tunnel_init(struct net_device *dev)
{
	struct ip6_tnl *tunnel;

	tunnel = netdev_priv(dev);

	tunnel->dev = dev;
	tunnel->net = dev_net(dev);
	strcpy(tunnel->parms.name, dev->name);

	memcpy(dev->dev_addr, &tunnel->parms.laddr, sizeof(struct in6_addr));
	memcpy(dev->broadcast, &tunnel->parms.raddr, sizeof(struct in6_addr));

	if (ipv6_addr_any(&tunnel->parms.raddr))
		dev->header_ops = &ip6gre_header_ops;

	dev->tstats = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!dev->tstats)
		return -ENOMEM;

	return 0;
}

static void ip6gre_fb_tunnel_init(struct net_device *dev)
{
	struct ip6_tnl *tunnel = netdev_priv(dev);

	tunnel->dev = dev;
	tunnel->net = dev_net(dev);
	strcpy(tunnel->parms.name, dev->name);

	tunnel->hlen		= sizeof(struct ipv6hdr) + 4;

	dev_hold(dev);
}


static struct inet6_protocol ip6gre_protocol __read_mostly = {
	.handler     = ip6gre_rcv,
	.err_handler = ip6gre_err,
	.flags       = INET6_PROTO_NOPOLICY|INET6_PROTO_FINAL,
};

static void ip6gre_destroy_tunnels(struct net *net, struct list_head *head)
{
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);
	struct net_device *dev, *aux;
	int prio;

	for_each_netdev_safe(net, dev, aux)
		if (dev->rtnl_link_ops == &ip6gre_link_ops ||
		    dev->rtnl_link_ops == &ip6gre_tap_ops)
			unregister_netdevice_queue(dev, head);

	for (prio = 0; prio < 4; prio++) {
		int h;
		for (h = 0; h < HASH_SIZE; h++) {
			struct ip6_tnl *t;

			t = rtnl_dereference(ign->tunnels[prio][h]);

			while (t) {
				/* If dev is in the same netns, it has already
				 * been added to the list by the previous loop.
				 */
				if (!net_eq(dev_net(t->dev), net))
					unregister_netdevice_queue(t->dev,
								   head);
				t = rtnl_dereference(t->next);
			}
		}
	}
}

static int __net_init ip6gre_init_net(struct net *net)
{
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);
	int err;

	ign->fb_tunnel_dev = alloc_netdev(sizeof(struct ip6_tnl), "ip6gre0",
					  NET_NAME_UNKNOWN,
					  ip6gre_tunnel_setup);
	if (!ign->fb_tunnel_dev) {
		err = -ENOMEM;
		goto err_alloc_dev;
	}
	dev_net_set(ign->fb_tunnel_dev, net);
	/* FB netdevice is special: we have one, and only one per netns.
	 * Allowing to move it to another netns is clearly unsafe.
	 */
	ign->fb_tunnel_dev->features |= NETIF_F_NETNS_LOCAL;


	ip6gre_fb_tunnel_init(ign->fb_tunnel_dev);
	ign->fb_tunnel_dev->rtnl_link_ops = &ip6gre_link_ops;

	err = register_netdev(ign->fb_tunnel_dev);
	if (err)
		goto err_reg_dev;

	rcu_assign_pointer(ign->tunnels_wc[0],
			   netdev_priv(ign->fb_tunnel_dev));
	return 0;

err_reg_dev:
	ip6gre_dev_free(ign->fb_tunnel_dev);
err_alloc_dev:
	return err;
}

static void __net_exit ip6gre_exit_net(struct net *net)
{
	LIST_HEAD(list);

	rtnl_lock();
	ip6gre_destroy_tunnels(net, &list);
	unregister_netdevice_many(&list);
	rtnl_unlock();
}

static struct pernet_operations ip6gre_net_ops = {
	.init = ip6gre_init_net,
	.exit = ip6gre_exit_net,
	.id   = &ip6gre_net_id,
	.size = sizeof(struct ip6gre_net),
};

static int ip6gre_tunnel_validate(struct nlattr *tb[], struct nlattr *data[])
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

static int ip6gre_tap_validate(struct nlattr *tb[], struct nlattr *data[])
{
	struct in6_addr daddr;

	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}

	if (!data)
		goto out;

	if (data[IFLA_GRE_REMOTE]) {
		daddr = nla_get_in6_addr(data[IFLA_GRE_REMOTE]);
		if (ipv6_addr_any(&daddr))
			return -EINVAL;
	}

out:
	return ip6gre_tunnel_validate(tb, data);
}


static void ip6gre_netlink_parms(struct nlattr *data[],
				struct __ip6_tnl_parm *parms)
{
	memset(parms, 0, sizeof(*parms));

	if (!data)
		return;

	if (data[IFLA_GRE_LINK])
		parms->link = nla_get_u32(data[IFLA_GRE_LINK]);

	if (data[IFLA_GRE_IFLAGS])
		parms->i_flags = nla_get_be16(data[IFLA_GRE_IFLAGS]);

	if (data[IFLA_GRE_OFLAGS])
		parms->o_flags = nla_get_be16(data[IFLA_GRE_OFLAGS]);

	if (data[IFLA_GRE_IKEY])
		parms->i_key = nla_get_be32(data[IFLA_GRE_IKEY]);

	if (data[IFLA_GRE_OKEY])
		parms->o_key = nla_get_be32(data[IFLA_GRE_OKEY]);

	if (data[IFLA_GRE_LOCAL])
		parms->laddr = nla_get_in6_addr(data[IFLA_GRE_LOCAL]);

	if (data[IFLA_GRE_REMOTE])
		parms->raddr = nla_get_in6_addr(data[IFLA_GRE_REMOTE]);

	if (data[IFLA_GRE_TTL])
		parms->hop_limit = nla_get_u8(data[IFLA_GRE_TTL]);

	if (data[IFLA_GRE_ENCAP_LIMIT])
		parms->encap_limit = nla_get_u8(data[IFLA_GRE_ENCAP_LIMIT]);

	if (data[IFLA_GRE_FLOWINFO])
		parms->flowinfo = nla_get_u32(data[IFLA_GRE_FLOWINFO]);

	if (data[IFLA_GRE_FLAGS])
		parms->flags = nla_get_u32(data[IFLA_GRE_FLAGS]);
}

static int ip6gre_tap_init(struct net_device *dev)
{
	struct ip6_tnl *tunnel;

	tunnel = netdev_priv(dev);

	tunnel->dev = dev;
	tunnel->net = dev_net(dev);
	strcpy(tunnel->parms.name, dev->name);

	ip6gre_tnl_link_config(tunnel, 1);

	dev->tstats = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!dev->tstats)
		return -ENOMEM;

	return 0;
}

static const struct net_device_ops ip6gre_tap_netdev_ops = {
	.ndo_init = ip6gre_tap_init,
	.ndo_uninit = ip6gre_tunnel_uninit,
	.ndo_start_xmit = ip6gre_tunnel_xmit,
	.ndo_set_mac_address = eth_mac_addr,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_change_mtu = ip6gre_tunnel_change_mtu,
	.ndo_get_stats64 = ip_tunnel_get_stats64,
	.ndo_get_iflink = ip6_tnl_get_iflink,
};

static void ip6gre_tap_setup(struct net_device *dev)
{

	ether_setup(dev);

	dev->netdev_ops = &ip6gre_tap_netdev_ops;
	dev->destructor = ip6gre_dev_free;

	dev->features |= NETIF_F_NETNS_LOCAL;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	dev->blog_stats_flags |= BLOG_DEV_STAT_FLAG_INCLUDE_ALL;
#endif
}

static int ip6gre_newlink(struct net *src_net, struct net_device *dev,
	struct nlattr *tb[], struct nlattr *data[])
{
	struct ip6_tnl *nt;
	struct net *net = dev_net(dev);
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);
	int err;

	nt = netdev_priv(dev);
	ip6gre_netlink_parms(data, &nt->parms);

	if (ip6gre_tunnel_find(net, &nt->parms, dev->type))
		return -EEXIST;

	if (dev->type == ARPHRD_ETHER && !tb[IFLA_ADDRESS])
		eth_hw_addr_random(dev);

	nt->dev = dev;
	nt->net = dev_net(dev);
	ip6gre_tnl_link_config(nt, !tb[IFLA_MTU]);

	/* Can use a lockless transmit, unless we generate output sequences */
	if (!(nt->parms.o_flags & GRE_SEQ))
		dev->features |= NETIF_F_LLTX;

	err = register_netdevice(dev);
	if (err)
		goto out;

	dev_hold(dev);
	ip6gre_tunnel_link(ign, nt);

out:
	return err;
}

static int ip6gre_changelink(struct net_device *dev, struct nlattr *tb[],
			    struct nlattr *data[])
{
	struct ip6_tnl *t, *nt = netdev_priv(dev);
	struct net *net = nt->net;
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);
	struct __ip6_tnl_parm p;

	if (dev == ign->fb_tunnel_dev)
		return -EINVAL;

	ip6gre_netlink_parms(data, &p);

	t = ip6gre_tunnel_locate(net, &p, 0);

	if (t) {
		if (t->dev != dev)
			return -EEXIST;
	} else {
		t = nt;
	}

	ip6gre_tunnel_unlink(ign, t);
	ip6gre_tnl_change(t, &p, !tb[IFLA_MTU]);
	ip6gre_tunnel_link(ign, t);
	return 0;
}

static void ip6gre_dellink(struct net_device *dev, struct list_head *head)
{
	struct net *net = dev_net(dev);
	struct ip6gre_net *ign = net_generic(net, ip6gre_net_id);

	if (dev != ign->fb_tunnel_dev)
		unregister_netdevice_queue(dev, head);
}

static size_t ip6gre_get_size(const struct net_device *dev)
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
		nla_total_size(sizeof(struct in6_addr)) +
		/* IFLA_GRE_REMOTE */
		nla_total_size(sizeof(struct in6_addr)) +
		/* IFLA_GRE_TTL */
		nla_total_size(1) +
		/* IFLA_GRE_TOS */
		nla_total_size(1) +
		/* IFLA_GRE_ENCAP_LIMIT */
		nla_total_size(1) +
		/* IFLA_GRE_FLOWINFO */
		nla_total_size(4) +
		/* IFLA_GRE_FLAGS */
		nla_total_size(4) +
		0;
}

static int ip6gre_fill_info(struct sk_buff *skb, const struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct __ip6_tnl_parm *p = &t->parms;

	if (nla_put_u32(skb, IFLA_GRE_LINK, p->link) ||
	    nla_put_be16(skb, IFLA_GRE_IFLAGS, p->i_flags) ||
	    nla_put_be16(skb, IFLA_GRE_OFLAGS, p->o_flags) ||
	    nla_put_be32(skb, IFLA_GRE_IKEY, p->i_key) ||
	    nla_put_be32(skb, IFLA_GRE_OKEY, p->o_key) ||
	    nla_put_in6_addr(skb, IFLA_GRE_LOCAL, &p->laddr) ||
	    nla_put_in6_addr(skb, IFLA_GRE_REMOTE, &p->raddr) ||
	    nla_put_u8(skb, IFLA_GRE_TTL, p->hop_limit) ||
	    /*nla_put_u8(skb, IFLA_GRE_TOS, t->priority) ||*/
	    nla_put_u8(skb, IFLA_GRE_ENCAP_LIMIT, p->encap_limit) ||
	    nla_put_be32(skb, IFLA_GRE_FLOWINFO, p->flowinfo) ||
	    nla_put_u32(skb, IFLA_GRE_FLAGS, p->flags))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -EMSGSIZE;
}

static const struct nla_policy ip6gre_policy[IFLA_GRE_MAX + 1] = {
	[IFLA_GRE_LINK]        = { .type = NLA_U32 },
	[IFLA_GRE_IFLAGS]      = { .type = NLA_U16 },
	[IFLA_GRE_OFLAGS]      = { .type = NLA_U16 },
	[IFLA_GRE_IKEY]        = { .type = NLA_U32 },
	[IFLA_GRE_OKEY]        = { .type = NLA_U32 },
	[IFLA_GRE_LOCAL]       = { .len = FIELD_SIZEOF(struct ipv6hdr, saddr) },
	[IFLA_GRE_REMOTE]      = { .len = FIELD_SIZEOF(struct ipv6hdr, daddr) },
	[IFLA_GRE_TTL]         = { .type = NLA_U8 },
	[IFLA_GRE_ENCAP_LIMIT] = { .type = NLA_U8 },
	[IFLA_GRE_FLOWINFO]    = { .type = NLA_U32 },
	[IFLA_GRE_FLAGS]       = { .type = NLA_U32 },
};

static struct rtnl_link_ops ip6gre_link_ops __read_mostly = {
	.kind		= "ip6gre",
	.maxtype	= IFLA_GRE_MAX,
	.policy		= ip6gre_policy,
	.priv_size	= sizeof(struct ip6_tnl),
	.setup		= ip6gre_tunnel_setup,
	.validate	= ip6gre_tunnel_validate,
	.newlink	= ip6gre_newlink,
	.changelink	= ip6gre_changelink,
	.dellink	= ip6gre_dellink,
	.get_size	= ip6gre_get_size,
	.fill_info	= ip6gre_fill_info,
	.get_link_net	= ip6_tnl_get_link_net,
};

static struct rtnl_link_ops ip6gre_tap_ops __read_mostly = {
	.kind		= "ip6gretap",
	.maxtype	= IFLA_GRE_MAX,
	.policy		= ip6gre_policy,
	.priv_size	= sizeof(struct ip6_tnl),
	.setup		= ip6gre_tap_setup,
	.validate	= ip6gre_tap_validate,
	.newlink	= ip6gre_newlink,
	.changelink	= ip6gre_changelink,
	.get_size	= ip6gre_get_size,
	.fill_info	= ip6gre_fill_info,
	.get_link_net	= ip6_tnl_get_link_net,
};

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
static inline int __bcm_gre6_rcv_check(struct ip6_tnl *tunnel, struct ipv6hdr *ipv6h, 
	uint16_t len, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_SEQNO;
	int grehlen = 4;
	/* Not supporting extension headers. If this changes, need to calculate. */
	__be16 *p = (__be16*)((uint8_t *)ipv6h+BLOG_IPV6_HDR_LEN);
	__be16 flags;

	flags = p[0];

	if (tunnel->parms.i_flags & GRE_CSUM) {
		uint16_t csum;

		grehlen += 4;
		csum = *(((__be16 *)p) + 2);

		if (!csum)
			goto no_csum;

		csum = ip_compute_csum((void*)(ipv6h+1), len - BLOG_IPV6_HDR_LEN);

		if (csum) {
			tunnel->dev->stats.rx_crc_errors++;
			tunnel->dev->stats.rx_errors++;
			ret = BLOG_GRE_RCV_CHKSUM_ERR;
			goto rcv_done;
		}
	}

no_csum:
	if ((tunnel->parms.i_flags & GRE_KEY) && (flags&GRE_KEY))
		grehlen += 4;

	if (tunnel->parms.i_flags & GRE_SEQ) {
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

int bcm_gre6_rcv_check(struct net_device *dev, struct ipv6hdr *ipv6h,
	uint16_t len, void **tunl, uint32_t *pkt_seqno)
{
	int ret = BLOG_GRE_RCV_NO_TUNNEL;
	struct ip6_tnl *tunnel;

	u8 *h = (uint8_t *)ipv6h+BLOG_IPV6_HDR_LEN;
	__be16 flags = *(__be16 *)h;
	__be16 protocol = *((__be16 *)h + 1);
	__be32 *options =(__be32*)(__be32 *)(h + BLOG_GRE_HDR_LEN);
	__be32 key = 0;

	if (flags & GRE_CSUM) {
		options++;
	}

	if (flags & GRE_KEY) {
		key = *options;
	}

	tunnel = ip6gre_tunnel_lookup(dev, &ipv6h->daddr, &ipv6h->saddr,
				key, protocol);

	if (tunnel) {
		rcu_read_lock();
		ret =  __bcm_gre6_rcv_check(tunnel, ipv6h, len, pkt_seqno);
		rcu_read_unlock();
	}

	*tunl = (void *) tunnel;
	return ret;
}

/* Adds the TX seqno, Key and updates the GRE checksum */
static inline
void __bcm_gre6_xmit_update(struct ip6_tnl *tunnel, struct ipv6hdr *ipv6h,
	uint16_t len)
{
	if (tunnel->parms.o_flags&(GRE_KEY|GRE_CSUM|GRE_SEQ)) {
		/* ip6 tunnel hlen is precalculated GRE header length (includes encap). */
		__be32 *ptr = (__be32*)(((u8*)ipv6h) + tunnel->hlen - 4);

		if (tunnel->parms.o_flags&GRE_SEQ) {
			++tunnel->o_seqno;
			*ptr = htonl(tunnel->o_seqno);
			ptr--;
		}

		if (tunnel->parms.o_flags&GRE_KEY) {
			*ptr = tunnel->parms.o_key;
			ptr--;
		}

		if (tunnel->parms.o_flags&GRE_CSUM) {
			*ptr = 0;
			*(__sum16*)ptr = ip_compute_csum((void*)(ipv6h+1), len - BLOG_IPV6_HDR_LEN);
		}
		cache_flush_len(ptr, tunnel->hlen);
	}
}

/* Adds the oseqno and updates the GRE checksum */
void bcm_gre6_xmit_update(struct ip6_tnl *tunnel, struct ipv6hdr *ipv6h,
	uint16_t len)
{
	rcu_read_lock();
	__bcm_gre6_xmit_update(tunnel, ipv6h, len);
	rcu_read_unlock();
}

#endif

/*
 *	And now the modules code and kernel interface.
 */

static int __init ip6gre_init(void)
{
	int err;

	pr_info("GRE over IPv6 tunneling driver\n");

	err = register_pernet_device(&ip6gre_net_ops);
	if (err < 0)
		return err;

	err = inet6_add_protocol(&ip6gre_protocol, IPPROTO_GRE);
	if (err < 0) {
		pr_info("%s: can't add protocol\n", __func__);
		goto add_proto_failed;
	}

	err = rtnl_link_register(&ip6gre_link_ops);
	if (err < 0)
		goto rtnl_link_failed;

	err = rtnl_link_register(&ip6gre_tap_ops);
	if (err < 0)
		goto tap_ops_failed;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_gre6_rcv_check_fn = (blog_gre6_rcv_check_t) bcm_gre6_rcv_check;
	blog_gre6_xmit_update_fn = (blog_gre6_xmit_upd_t) bcm_gre6_xmit_update;
#endif

out:
	return err;

tap_ops_failed:
	rtnl_link_unregister(&ip6gre_link_ops);
rtnl_link_failed:
	inet6_del_protocol(&ip6gre_protocol, IPPROTO_GRE);
add_proto_failed:
	unregister_pernet_device(&ip6gre_net_ops);
	goto out;
}

static void __exit ip6gre_fini(void)
{
	rtnl_link_unregister(&ip6gre_tap_ops);
	rtnl_link_unregister(&ip6gre_link_ops);
	inet6_del_protocol(&ip6gre_protocol, IPPROTO_GRE);
	unregister_pernet_device(&ip6gre_net_ops);
}

module_init(ip6gre_init);
module_exit(ip6gre_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("D. Kozlov (xeb@mail.ru)");
MODULE_DESCRIPTION("GRE over IPv6 tunneling device");
MODULE_ALIAS_RTNL_LINK("ip6gre");
MODULE_ALIAS_RTNL_LINK("ip6gretap");
MODULE_ALIAS_NETDEV("ip6gre0");
