/*
 *	IPv6 tunneling device
 *	Linux INET6 implementation
 *
 *	Authors:
 *	Ville Nuorvala		<vnuorval@tcs.hut.fi>
 *	Yasuyuki Kozakai	<kozakai@linux-ipv6.org>
 *
 *      Based on:
 *      linux/net/ipv6/sit.c and linux/net/ipv4/ipip.c
 *
 *      RFC 2473
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/sockios.h>
#include <linux/icmp.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <linux/in6.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/icmpv6.h>
#include <linux/init.h>
#include <linux/route.h>
#include <linux/rtnetlink.h>
#include <linux/netfilter_ipv6.h>
#include <linux/slab.h>
#include <linux/hash.h>
#include <linux/etherdevice.h>

#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/string.h>
#include <linux/file.h>
#include <linux/fs.h>

#include <net/icmp.h>
#include <net/ip.h>
#include <net/ip_tunnels.h>
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <net/addrconf.h>
#include <net/ip6_tunnel.h>
#include <net/xfrm.h>
#include <net/dsfield.h>
#include <net/inet_ecn.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/dst_metadata.h>
#include <linux/inet.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <net/udp.h>

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

MODULE_AUTHOR("Ville Nuorvala");
MODULE_DESCRIPTION("IPv6 tunneling device");
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK("ip6tnl");
MODULE_ALIAS_NETDEV("ip6tnl0");

#define IP6_TUNNEL_HASH_SIZE_SHIFT  5
#define IP6_TUNNEL_HASH_SIZE (1 << IP6_TUNNEL_HASH_SIZE_SHIFT)

static bool log_ecn_error = true;
module_param(log_ecn_error, bool, 0644);
MODULE_PARM_DESC(log_ecn_error, "Log packets received with corrupted ECN");

struct file *OpenFile(char *path, int flag, int mode)
{
	struct filename *name = getname_kernel(path);
	struct file *fp = ERR_CAST(name);

	if (!IS_ERR(name)) {
		fp = filp_open(path, flag, 0);
		if (IS_ERR(fp)) {
			printk("[%s(%d)] File didn't exist: %s [%ld]\n",
				__FUNCTION__, __LINE__, path, PTR_ERR(fp));
			return NULL;
		}
		return fp;
	}
	return NULL;
}

int CloseFile(struct file *fp)
{
	filp_close(fp,NULL);
	return 0;
}

static u32 HASH(const struct in6_addr *addr)
{
	u32 hash = ipv6_addr_hash(addr);

	return hash_32(hash, IP6_TUNNEL_HASH_SIZE_SHIFT);
}

static void reChecksum(struct sk_buff *skb);
static int ip6_tnl_dev_init(struct net_device *dev);
static void ip6_tnl_dev_setup(struct net_device *dev);
static struct rtnl_link_ops ip6_link_ops __read_mostly;

static unsigned int ip6_tnl_net_id __read_mostly;
struct ip6_tnl_net {
	/* the IPv6 tunnel fallback device */
	struct net_device *fb_tnl_dev;
	/* lists for storing tunnels in use */
	struct ip6_tnl __rcu *tnls_r_l[IP6_TUNNEL_HASH_SIZE];
	struct ip6_tnl __rcu *tnls_wc[1];
	struct ip6_tnl __rcu **tnls[2];
	struct ip6_tnl __rcu *collect_md_tun;
};

static struct net_device_stats *ip6_get_stats(struct net_device *dev)
{
	struct pcpu_sw_netstats tmp, sum = { 0 };
	int i;

	for_each_possible_cpu(i) {
		unsigned int start;
		const struct pcpu_sw_netstats *tstats =
						   per_cpu_ptr(dev->tstats, i);

		do {
			start = u64_stats_fetch_begin_irq(&tstats->syncp);
			tmp.rx_packets = tstats->rx_packets;
			tmp.rx_bytes = tstats->rx_bytes;
			tmp.tx_packets = tstats->tx_packets;
			tmp.tx_bytes =  tstats->tx_bytes;
		} while (u64_stats_fetch_retry_irq(&tstats->syncp, start));

		sum.rx_packets += tmp.rx_packets;
		sum.rx_bytes   += tmp.rx_bytes;
		sum.tx_packets += tmp.tx_packets;
		sum.tx_bytes   += tmp.tx_bytes;
	}
	dev->stats.rx_packets = sum.rx_packets;
	dev->stats.rx_bytes   = sum.rx_bytes;
	dev->stats.tx_packets = sum.tx_packets;
	dev->stats.tx_bytes   = sum.tx_bytes;
	return &dev->stats;
}

/**
 * ip6_tnl_lookup - fetch tunnel matching the end-point addresses
 *   @remote: the address of the tunnel exit-point
 *   @local: the address of the tunnel entry-point
 *
 * Return:
 *   tunnel matching given end-points if found,
 *   else fallback tunnel if its device is up,
 *   else %NULL
 **/

#define for_each_ip6_tunnel_rcu(start) \
	for (t = rcu_dereference(start); t; t = rcu_dereference(t->next))

static struct ip6_tnl *
ip6_tnl_lookup(struct net *net, const struct in6_addr *remote, const struct in6_addr *local)
{
	unsigned int hash = HASH(local);
	struct ip6_tnl *t;
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);
	struct in6_addr any;
	struct __ip6_tnl_fmr *fmr;

	for_each_ip6_tunnel_rcu(ip6n->tnls_r_l[hash]) {
		if (!ipv6_addr_equal(local, &t->parms.laddr) ||
				!(t->dev->flags & IFF_UP))
			continue;

		if (ipv6_addr_equal(remote, &t->parms.raddr))
			return t;

		for (fmr = t->parms.fmrs; fmr; fmr = fmr->next) {
			if (ipv6_prefix_equal(remote, &fmr->ip6_prefix,
					fmr->ip6_prefix_len))
				return t;
		}
	}

	memset(&any, 0, sizeof(any));
	hash = HASH(local);
	for_each_ip6_tunnel_rcu(ip6n->tnls_r_l[hash]) {
		if (ipv6_addr_equal(local, &t->parms.laddr) &&
		    ipv6_addr_any(&t->parms.raddr) &&
		    (t->dev->flags & IFF_UP))
			return t;
	}

	hash = HASH(&any);
	for_each_ip6_tunnel_rcu(ip6n->tnls_r_l[hash]) {
		if (ipv6_addr_equal(remote, &t->parms.raddr) &&
		    ipv6_addr_any(&t->parms.laddr) &&
		    (t->dev->flags & IFF_UP))
			return t;
	}

	t = rcu_dereference(ip6n->collect_md_tun);
	if (t && t->dev->flags & IFF_UP)
		return t;

	t = rcu_dereference(ip6n->tnls_wc[0]);
	if (t && (t->dev->flags & IFF_UP))
		return t;

	return NULL;
}

/**
 * ip6_tnl_bucket - get head of list matching given tunnel parameters
 *   @p: parameters containing tunnel end-points
 *
 * Description:
 *   ip6_tnl_bucket() returns the head of the list matching the
 *   &struct in6_addr entries laddr and raddr in @p.
 *
 * Return: head of IPv6 tunnel list
 **/

static struct ip6_tnl __rcu **
ip6_tnl_bucket(struct ip6_tnl_net *ip6n, const struct __ip6_tnl_parm *p)
{
	const struct in6_addr *remote = &p->raddr;
	const struct in6_addr *local = &p->laddr;
	unsigned int h = 0;
	int prio = 0;

	if (!ipv6_addr_any(remote) || !ipv6_addr_any(local)) {
		prio = 1;
		h = HASH(local);
	}
	return &ip6n->tnls[prio][h];
}

/**
 * ip6_tnl_link - add tunnel to hash table
 *   @t: tunnel to be added
 **/

static void
ip6_tnl_link(struct ip6_tnl_net *ip6n, struct ip6_tnl *t)
{
	struct ip6_tnl __rcu **tp = ip6_tnl_bucket(ip6n, &t->parms);

	if (t->parms.collect_md)
		rcu_assign_pointer(ip6n->collect_md_tun, t);
	rcu_assign_pointer(t->next , rtnl_dereference(*tp));
	rcu_assign_pointer(*tp, t);
}

/**
 * ip6_tnl_unlink - remove tunnel from hash table
 *   @t: tunnel to be removed
 **/

static void
ip6_tnl_unlink(struct ip6_tnl_net *ip6n, struct ip6_tnl *t)
{
	struct ip6_tnl __rcu **tp;
	struct ip6_tnl *iter;

	if (t->parms.collect_md)
		rcu_assign_pointer(ip6n->collect_md_tun, NULL);

	for (tp = ip6_tnl_bucket(ip6n, &t->parms);
	     (iter = rtnl_dereference(*tp)) != NULL;
	     tp = &iter->next) {
		if (t == iter) {
			rcu_assign_pointer(*tp, t->next);
			break;
		}
	}
}

static void ip6_dev_free(struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);

	gro_cells_destroy(&t->gro_cells);
	dst_cache_destroy(&t->dst_cache);
	free_percpu(dev->tstats);
}

static int ip6_tnl_create2(struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct net *net = dev_net(dev);
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);
	int err;

	t = netdev_priv(dev);

	dev->rtnl_link_ops = &ip6_link_ops;
	err = register_netdevice(dev);
	if (err < 0)
		goto out;

	strcpy(t->parms.name, dev->name);

	dev_hold(dev);
	ip6_tnl_link(ip6n, t);
	return 0;

out:
	return err;
}

/**
 * ip6_tnl_create - create a new tunnel
 *   @p: tunnel parameters
 *   @pt: pointer to new tunnel
 *
 * Description:
 *   Create tunnel matching given parameters.
 *
 * Return:
 *   created tunnel or error pointer
 **/

static struct ip6_tnl *ip6_tnl_create(struct net *net, struct __ip6_tnl_parm *p)
{
	struct net_device *dev;
	struct ip6_tnl *t;
	char name[IFNAMSIZ];
	int err = -E2BIG;

	if (p->name[0]) {
		if (!dev_valid_name(p->name))
			goto failed;
		strlcpy(name, p->name, IFNAMSIZ);
	} else {
		sprintf(name, "ip6tnl%%d");
	}
	err = -ENOMEM;
	dev = alloc_netdev(sizeof(*t), name, NET_NAME_UNKNOWN,
			   ip6_tnl_dev_setup);
	if (!dev)
		goto failed;

	dev_net_set(dev, net);

	t = netdev_priv(dev);
	t->parms = *p;
	t->net = dev_net(dev);
	err = ip6_tnl_create2(dev);
	if (err < 0)
		goto failed_free;

	return t;

failed_free:
	free_netdev(dev);
failed:
	return ERR_PTR(err);
}

/**
 * ip6_tnl_locate - find or create tunnel matching given parameters
 *   @p: tunnel parameters
 *   @create: != 0 if allowed to create new tunnel if no match found
 *
 * Description:
 *   ip6_tnl_locate() first tries to locate an existing tunnel
 *   based on @parms. If this is unsuccessful, but @create is set a new
 *   tunnel device is created and registered for use.
 *
 * Return:
 *   matching tunnel or error pointer
 **/

static struct ip6_tnl *ip6_tnl_locate(struct net *net,
		struct __ip6_tnl_parm *p, int create)
{
	const struct in6_addr *remote = &p->raddr;
	const struct in6_addr *local = &p->laddr;
	struct ip6_tnl __rcu **tp;
	struct ip6_tnl *t;
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);

	for (tp = ip6_tnl_bucket(ip6n, p);
	     (t = rtnl_dereference(*tp)) != NULL;
	     tp = &t->next) {
		if (ipv6_addr_equal(local, &t->parms.laddr) &&
		    ipv6_addr_equal(remote, &t->parms.raddr)) {
			if (create)
				return ERR_PTR(-EEXIST);

			return t;
		}
	}
	if (!create)
		return ERR_PTR(-ENODEV);
	return ip6_tnl_create(net, p);
}

/**
 * ip6_tnl_dev_uninit - tunnel device uninitializer
 *   @dev: the device to be destroyed
 *
 * Description:
 *   ip6_tnl_dev_uninit() removes tunnel from its list
 **/

static void
ip6_tnl_dev_uninit(struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct net *net = t->net;
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);

	while (t->parms.fmrs) {
		struct __ip6_tnl_fmr *next = t->parms.fmrs->next;
		kfree(t->parms.fmrs);
		t->parms.fmrs = next;
	}

	if (dev == ip6n->fb_tnl_dev)
		RCU_INIT_POINTER(ip6n->tnls_wc[0], NULL);
	else
		ip6_tnl_unlink(ip6n, t);
	dst_cache_reset(&t->dst_cache);
	dev_put(dev);
}

/**
 * parse_tvl_tnl_enc_lim - handle encapsulation limit option
 *   @skb: received socket buffer
 *
 * Return:
 *   0 if none was found,
 *   else index to encapsulation limit
 **/

__u16 ip6_tnl_parse_tlv_enc_lim(struct sk_buff *skb, __u8 *raw)
{
	const struct ipv6hdr *ipv6h = (const struct ipv6hdr *)raw;
	unsigned int nhoff = raw - skb->data;
	unsigned int off = nhoff + sizeof(*ipv6h);
	u8 next, nexthdr = ipv6h->nexthdr;

	while (ipv6_ext_hdr(nexthdr) && nexthdr != NEXTHDR_NONE) {
		struct ipv6_opt_hdr *hdr;
		u16 optlen;

		if (!pskb_may_pull(skb, off + sizeof(*hdr)))
			break;

		hdr = (struct ipv6_opt_hdr *)(skb->data + off);
		if (nexthdr == NEXTHDR_FRAGMENT) {
			struct frag_hdr *frag_hdr = (struct frag_hdr *) hdr;
			if (frag_hdr->frag_off)
				break;
			optlen = 8;
		} else if (nexthdr == NEXTHDR_AUTH) {
			optlen = (hdr->hdrlen + 2) << 2;
		} else {
			optlen = ipv6_optlen(hdr);
		}
		/* cache hdr->nexthdr, since pskb_may_pull() might
		 * invalidate hdr
		 */
		next = hdr->nexthdr;
		if (nexthdr == NEXTHDR_DEST) {
			u16 i = 2;

			/* Remember : hdr is no longer valid at this point. */
			if (!pskb_may_pull(skb, off + optlen))
				break;

			while (1) {
				struct ipv6_tlv_tnl_enc_lim *tel;

				/* No more room for encapsulation limit */
				if (i + sizeof(*tel) > optlen)
					break;

				tel = (struct ipv6_tlv_tnl_enc_lim *)(skb->data + off + i);
				/* return index of option if found and valid */
				if (tel->type == IPV6_TLV_TNL_ENCAP_LIMIT &&
				    tel->length == 1)
					return i + off - nhoff;
				/* else jump to next option */
				if (tel->type)
					i += tel->length + 2;
				else
					i++;
			}
		}
		nexthdr = next;
		off += optlen;
	}
	return 0;
}
EXPORT_SYMBOL(ip6_tnl_parse_tlv_enc_lim);

/**
 * ip6_tnl_err - tunnel error handler
 *
 * Description:
 *   ip6_tnl_err() should handle errors in the tunnel according
 *   to the specifications in RFC 2473.
 **/

static int
ip6_tnl_err(struct sk_buff *skb, __u8 ipproto, struct inet6_skb_parm *opt,
	    u8 *type, u8 *code, int *msg, __u32 *info, int offset)
{
	const struct ipv6hdr *ipv6h = (const struct ipv6hdr *)skb->data;
	struct net *net = dev_net(skb->dev);
	u8 rel_type = ICMPV6_DEST_UNREACH;
	u8 rel_code = ICMPV6_ADDR_UNREACH;
	__u32 rel_info = 0;
	struct ip6_tnl *t;
	int err = -ENOENT;
	int rel_msg = 0;
	u8 tproto;
	__u16 len;

	/* If the packet doesn't contain the original IPv6 header we are
	   in trouble since we might need the source address for further
	   processing of the error. */

	rcu_read_lock();
	t = ip6_tnl_lookup(dev_net(skb->dev), &ipv6h->daddr, &ipv6h->saddr);
	if (!t)
		goto out;

	tproto = READ_ONCE(t->parms.proto);
	if (tproto != ipproto && tproto != 0)
		goto out;

	err = 0;

	switch (*type) {
		struct ipv6_tlv_tnl_enc_lim *tel;
		__u32 mtu, teli;
	case ICMPV6_DEST_UNREACH:
		net_dbg_ratelimited("%s: Path to destination invalid or inactive!\n",
				    t->parms.name);
		rel_msg = 1;
		break;
	case ICMPV6_TIME_EXCEED:
		if ((*code) == ICMPV6_EXC_HOPLIMIT) {
			net_dbg_ratelimited("%s: Too small hop limit or routing loop in tunnel!\n",
					    t->parms.name);
			rel_msg = 1;
		}
		break;
	case ICMPV6_PARAMPROB:
		teli = 0;
		if ((*code) == ICMPV6_HDR_FIELD)
			teli = ip6_tnl_parse_tlv_enc_lim(skb, skb->data);

		if (teli && teli == *info - 2) {
			tel = (struct ipv6_tlv_tnl_enc_lim *) &skb->data[teli];
			if (tel->encap_limit == 0) {
				net_dbg_ratelimited("%s: Too small encapsulation limit or routing loop in tunnel!\n",
						    t->parms.name);
				rel_msg = 1;
			}
		} else {
			net_dbg_ratelimited("%s: Recipient unable to parse tunneled packet!\n",
					    t->parms.name);
		}
		break;
	case ICMPV6_PKT_TOOBIG:
		ip6_update_pmtu(skb, net, htonl(*info), 0, 0,
				sock_net_uid(net, NULL));
		mtu = *info - offset;
		if (mtu < IPV6_MIN_MTU)
			mtu = IPV6_MIN_MTU;
		len = sizeof(*ipv6h) + ntohs(ipv6h->payload_len);
		if (len > mtu) {
			rel_type = ICMPV6_PKT_TOOBIG;
			rel_code = 0;
			rel_info = mtu;
			rel_msg = 1;
		}
		break;
	case NDISC_REDIRECT:
		ip6_redirect(skb, net, skb->dev->ifindex, 0,
			     sock_net_uid(net, NULL));
		break;
	}

	*type = rel_type;
	*code = rel_code;
	*info = rel_info;
	*msg = rel_msg;

out:
	rcu_read_unlock();
	return err;
}

static int
ip4ip6_err(struct sk_buff *skb, struct inet6_skb_parm *opt,
	   u8 type, u8 code, int offset, __be32 info)
{
	__u32 rel_info = ntohl(info);
	const struct iphdr *eiph;
	struct sk_buff *skb2;
	int err, rel_msg = 0;
	u8 rel_type = type;
	u8 rel_code = code;
	struct rtable *rt;
	struct flowi4 fl4;

	err = ip6_tnl_err(skb, IPPROTO_IPIP, opt, &rel_type, &rel_code,
			  &rel_msg, &rel_info, offset);
	if (err < 0)
		return err;

	if (rel_msg == 0)
		return 0;

	switch (rel_type) {
	case ICMPV6_DEST_UNREACH:
		if (rel_code != ICMPV6_ADDR_UNREACH)
			return 0;
		rel_type = ICMP_DEST_UNREACH;
		rel_code = ICMP_HOST_UNREACH;
		break;
	case ICMPV6_PKT_TOOBIG:
		if (rel_code != 0)
			return 0;
		rel_type = ICMP_DEST_UNREACH;
		rel_code = ICMP_FRAG_NEEDED;
		break;
	default:
		return 0;
	}

	if (!pskb_may_pull(skb, offset + sizeof(struct iphdr)))
		return 0;

	skb2 = skb_clone(skb, GFP_ATOMIC);
	if (!skb2)
		return 0;

	skb_dst_drop(skb2);

	skb_pull(skb2, offset);
	skb_reset_network_header(skb2);
	eiph = ip_hdr(skb2);

	/* Try to guess incoming interface */
	rt = ip_route_output_ports(dev_net(skb->dev), &fl4, NULL, eiph->saddr,
				   0, 0, 0, IPPROTO_IPIP, RT_TOS(eiph->tos), 0);
	if (IS_ERR(rt))
		goto out;

	skb2->dev = rt->dst.dev;
	ip_rt_put(rt);

	/* route "incoming" packet */
	if (rt->rt_flags & RTCF_LOCAL) {
		rt = ip_route_output_ports(dev_net(skb->dev), &fl4, NULL,
					   eiph->daddr, eiph->saddr, 0, 0,
					   IPPROTO_IPIP, RT_TOS(eiph->tos), 0);
		if (IS_ERR(rt) || rt->dst.dev->type != ARPHRD_TUNNEL6) {
			if (!IS_ERR(rt))
				ip_rt_put(rt);
			goto out;
		}
		skb_dst_set(skb2, &rt->dst);
	} else {
		if (ip_route_input(skb2, eiph->daddr, eiph->saddr, eiph->tos,
				   skb2->dev) ||
		    skb_dst(skb2)->dev->type != ARPHRD_TUNNEL6)
			goto out;
	}

	/* change mtu on this route */
	if (rel_type == ICMP_DEST_UNREACH && rel_code == ICMP_FRAG_NEEDED) {
		if (rel_info > dst_mtu(skb_dst(skb2)))
			goto out;

		skb_dst_update_pmtu_no_confirm(skb2, rel_info);
	}

	icmp_send(skb2, rel_type, rel_code, htonl(rel_info));

out:
	kfree_skb(skb2);
	return 0;
}

static int
ip6ip6_err(struct sk_buff *skb, struct inet6_skb_parm *opt,
	   u8 type, u8 code, int offset, __be32 info)
{
	__u32 rel_info = ntohl(info);
	int err, rel_msg = 0;
	u8 rel_type = type;
	u8 rel_code = code;

	err = ip6_tnl_err(skb, IPPROTO_IPV6, opt, &rel_type, &rel_code,
			  &rel_msg, &rel_info, offset);
	if (err < 0)
		return err;

	if (rel_msg && pskb_may_pull(skb, offset + sizeof(struct ipv6hdr))) {
		struct rt6_info *rt;
		struct sk_buff *skb2 = skb_clone(skb, GFP_ATOMIC);

		if (!skb2)
			return 0;

		skb_dst_drop(skb2);
		skb_pull(skb2, offset);
		skb_reset_network_header(skb2);

		/* Try to guess incoming interface */
		rt = rt6_lookup(dev_net(skb->dev), &ipv6_hdr(skb2)->saddr,
				NULL, 0, skb2, 0);

		if (rt && rt->dst.dev)
			skb2->dev = rt->dst.dev;

		icmpv6_send(skb2, rel_type, rel_code, rel_info);

		ip6_rt_put(rt);

		kfree_skb(skb2);
	}

	return 0;
}

static int ip4ip6_dscp_ecn_decapsulate(const struct ip6_tnl *t,
				       const struct ipv6hdr *ipv6h,
				       struct sk_buff *skb)
{
	__u8 dsfield = ipv6_get_dsfield(ipv6h) & ~INET_ECN_MASK;

	if (t->parms.flags & IP6_TNL_F_RCV_DSCP_COPY)
		ipv4_change_dsfield(ip_hdr(skb), INET_ECN_MASK, dsfield);

	return IP6_ECN_decapsulate(ipv6h, skb);
}

static int ip6ip6_dscp_ecn_decapsulate(const struct ip6_tnl *t,
				       const struct ipv6hdr *ipv6h,
				       struct sk_buff *skb)
{
	if (t->parms.flags & IP6_TNL_F_RCV_DSCP_COPY)
		ipv6_copy_dscp(ipv6_get_dsfield(ipv6h), ipv6_hdr(skb));

	return IP6_ECN_decapsulate(ipv6h, skb);
}

__u32 ip6_tnl_get_cap(struct ip6_tnl *t,
			     const struct in6_addr *laddr,
			     const struct in6_addr *raddr)
{
	struct __ip6_tnl_parm *p = &t->parms;
	int ltype = ipv6_addr_type(laddr);
	int rtype = ipv6_addr_type(raddr);
	__u32 flags = 0;

	if (ltype == IPV6_ADDR_ANY || rtype == IPV6_ADDR_ANY) {
		flags = IP6_TNL_F_CAP_PER_PACKET;
	} else if (ltype & (IPV6_ADDR_UNICAST|IPV6_ADDR_MULTICAST) &&
		   rtype & (IPV6_ADDR_UNICAST|IPV6_ADDR_MULTICAST) &&
		   !((ltype|rtype) & IPV6_ADDR_LOOPBACK) &&
		   (!((ltype|rtype) & IPV6_ADDR_LINKLOCAL) || p->link)) {
		if (ltype&IPV6_ADDR_UNICAST)
			flags |= IP6_TNL_F_CAP_XMIT;
		if (rtype&IPV6_ADDR_UNICAST)
			flags |= IP6_TNL_F_CAP_RCV;
	}
	return flags;
}
EXPORT_SYMBOL(ip6_tnl_get_cap);

/* called with rcu_read_lock() */
int ip6_tnl_rcv_ctl(struct ip6_tnl *t,
				  const struct in6_addr *laddr,
				  const struct in6_addr *raddr)
{
	struct __ip6_tnl_parm *p = &t->parms;
	int ret = 0;
	struct net *net = t->net;

	if ((p->flags & IP6_TNL_F_CAP_RCV) ||
	    ((p->flags & IP6_TNL_F_CAP_PER_PACKET) &&
	     (ip6_tnl_get_cap(t, laddr, raddr) & IP6_TNL_F_CAP_RCV))) {
		struct net_device *ldev = NULL;

		if (p->link)
			ldev = dev_get_by_index_rcu(net, p->link);

		if ((ipv6_addr_is_multicast(laddr) ||
		     likely(ipv6_chk_addr_and_flags(net, laddr, ldev, false,
						    0, IFA_F_TENTATIVE))) &&
		    ((p->flags & IP6_TNL_F_ALLOW_LOCAL_REMOTE) ||
		     likely(!ipv6_chk_addr_and_flags(net, raddr, ldev, true,
						     0, IFA_F_TENTATIVE))))
			ret = 1;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(ip6_tnl_rcv_ctl);

/**
 * ip4ip6_fmr_calc - calculate target / source IPv6-address based on FMR
 *   @dest: destination IPv6 address buffer
 *   @skb: received socket buffer
 *   @fmr: MAP FMR
 *   @xmit: Calculate for xmit or rcv
 **/
static void ip4ip6_fmr_calc(struct in6_addr *dest,
		const struct iphdr *iph, const uint8_t *end,
		const struct __ip6_tnl_fmr *fmr, bool xmit, bool draft)
{
	int psidlen = fmr->ea_len - (32 - fmr->ip4_prefix_len);
	u8 *portp = NULL;
	bool use_dest_addr;
	const struct iphdr *dsth = iph;

	if ((u8*)dsth >= end)
		return;

	/* find significant IP header */
	if (iph->protocol == IPPROTO_ICMP) {
		struct icmphdr *ih = (struct icmphdr*)(((u8*)dsth) + dsth->ihl * 4);
		if (ih && ((u8*)&ih[1]) <= end && (
			ih->type == ICMP_DEST_UNREACH ||
			ih->type == ICMP_SOURCE_QUENCH ||
			ih->type == ICMP_TIME_EXCEEDED ||
			ih->type == ICMP_PARAMETERPROB ||
			ih->type == ICMP_REDIRECT))
				dsth = (const struct iphdr*)&ih[1];
	}

	/* in xmit-path use dest port by default and source port only if
		this is an ICMP reply to something else; vice versa in rcv-path */
	use_dest_addr = (xmit && dsth == iph) || (!xmit && dsth != iph);

	/* get dst port */
	if (((u8*)&dsth[1]) <= end && (
		dsth->protocol == IPPROTO_UDP ||
		dsth->protocol == IPPROTO_TCP ||
		dsth->protocol == IPPROTO_SCTP ||
		dsth->protocol == IPPROTO_DCCP)) {
			/* for UDP, TCP, SCTP and DCCP source and dest port
			follow IPv4 header directly */
			portp = ((u8*)dsth) + dsth->ihl * 4;

			if (use_dest_addr)
				portp += sizeof(u16);
	} else if (iph->protocol == IPPROTO_ICMP) {
		struct icmphdr *ih = (struct icmphdr*)(((u8*)dsth) + dsth->ihl * 4);

		/* use icmp identifier as port */
		if (((u8*)&ih) <= end && (
		    (use_dest_addr && (
		    ih->type == ICMP_ECHOREPLY ||
			ih->type == ICMP_TIMESTAMPREPLY ||
			ih->type == ICMP_INFO_REPLY ||
			ih->type == ICMP_ADDRESSREPLY)) ||
			(!use_dest_addr && (
			ih->type == ICMP_ECHO ||
			ih->type == ICMP_TIMESTAMP ||
			ih->type == ICMP_INFO_REQUEST ||
			ih->type == ICMP_ADDRESS)
			)))
				portp = (u8*)&ih->un.echo.id;
	}

	if ((portp && &portp[2] <= end) || psidlen == 0) {
		int frombyte = fmr->ip6_prefix_len / 8;
		int fromrem = fmr->ip6_prefix_len % 8;
		int bytes = sizeof(struct in6_addr) - frombyte;
		const u32 *addr = (use_dest_addr) ? &iph->daddr : &iph->saddr;
		u64 eabits = ((u64)ntohl(*addr)) << (32 + fmr->ip4_prefix_len);
		u64 t = 0;

		/* extract PSID from port and add it to eabits */
		u16 psidbits = 0;
		if (psidlen > 0) {
			psidbits = ((u16)portp[0]) << 8 | ((u16)portp[1]);
			psidbits >>= 16 - psidlen - fmr->offset;
			psidbits = (u16)(psidbits << (16 - psidlen));
			eabits |= ((u64)psidbits) << (48 - (fmr->ea_len - psidlen));
		}

		/* rewrite destination address */
		int i = draft ? 9 : 10;
		*dest = fmr->ip6_prefix;
		memcpy(&dest->s6_addr[i], addr, sizeof(*addr));
		psidbits = htons(psidbits >> (16 - psidlen));
		memcpy(&dest->s6_addr[i + 4], &psidbits, sizeof(psidbits));

		/*
		*dest = fmr->ip6_prefix;
		memcpy(&dest->s6_addr[10], addr, sizeof(*addr));
		dest->s6_addr16[7] = htons(psidbits >> (16 - psidlen));
		*/

		if (bytes > sizeof(u64))
			bytes = sizeof(u64);

		/* insert eabits */
		memcpy(&t, &dest->s6_addr[frombyte], bytes);
		t = be64_to_cpu(t) & ~(((((u64)1) << fmr->ea_len) - 1)
			<< (64 - fmr->ea_len - fromrem));
		t = cpu_to_be64(t | (eabits >> fromrem));
		memcpy(&dest->s6_addr[frombyte], &t, bytes);
	}
}


static int __ip6_tnl_rcv(struct ip6_tnl *tunnel, struct sk_buff *skb,
			 const struct tnl_ptk_info *tpi,
			 struct metadata_dst *tun_dst,
			 int (*dscp_ecn_decapsulate)(const struct ip6_tnl *t,
						const struct ipv6hdr *ipv6h,
						struct sk_buff *skb),
			 bool log_ecn_err)
{
	struct pcpu_sw_netstats *tstats;
	const struct ipv6hdr *ipv6h = ipv6_hdr(skb);
	int err;
	bool draft = false;

	if ((!(tpi->flags & TUNNEL_CSUM) &&
	     (tunnel->parms.i_flags & TUNNEL_CSUM)) ||
	    ((tpi->flags & TUNNEL_CSUM) &&
	     !(tunnel->parms.i_flags & TUNNEL_CSUM))) {
		tunnel->dev->stats.rx_crc_errors++;
		tunnel->dev->stats.rx_errors++;
		goto drop;
	}

#if (defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG))
	/* as bcm_gre_rcv_check increments seqno (for all tunnel packets) when  
	 * GRE acceleration is enabled, we try to avoid double increment here 
	 */
	if ((tunnel->parms.i_flags & TUNNEL_SEQ) && 
		!blog_gre_tunnel_accelerated()) {
#else
	if (tunnel->parms.i_flags & TUNNEL_SEQ) {
#endif
		if (!(tpi->flags & TUNNEL_SEQ) ||
		    (tunnel->i_seqno &&
		     (s32)(ntohl(tpi->seq) - tunnel->i_seqno) < 0)) {
			tunnel->dev->stats.rx_fifo_errors++;
			tunnel->dev->stats.rx_errors++;
			goto drop;
		}
		tunnel->i_seqno = ntohl(tpi->seq) + 1;
	}

	skb->protocol = tpi->proto;

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
	} else {
		skb->dev = tunnel->dev;
	}

	skb_reset_network_header(skb);
	memset(skb->cb, 0, sizeof(struct inet6_skb_parm));

	if (tpi->proto == htons(ETH_P_IP) && tunnel->parms.fmrs &&
		!ipv6_addr_equal(&ipv6h->saddr, &tunnel->parms.raddr)) {
			/* Packet didn't come from BR, so lookup FMR */
			struct __ip6_tnl_fmr *fmr;
			struct in6_addr expected = tunnel->parms.raddr;
			for (fmr = tunnel->parms.fmrs; fmr; fmr = fmr->next)
				if (ipv6_prefix_equal(&ipv6h->saddr,
					&fmr->ip6_prefix, fmr->ip6_prefix_len))
						break;

			if (tunnel->parms.flags & IP6_TNL_F_USE_FMR_DRAFT) {
				draft = true;
			}
			/* Check that IPv6 matches IPv4 source to prevent spoofing */
			if (fmr)
				ip4ip6_fmr_calc(&expected, ip_hdr(skb),
						skb_tail_pointer(skb), fmr, false, draft);

			if (!ipv6_addr_equal(&ipv6h->saddr, &expected)) {
				rcu_read_unlock();
				goto drop;
			}
	}

	/* For CE to CE in same address case.
	   Change back source address to fake for match MASQUERADE contrack. */
	struct iphdr  *iph = ip_hdr(skb);
	if (iph->saddr == iph->daddr) {
		iph->saddr = in_aton("169.254.7.7");
		//re-calculator checksum
		reChecksum(skb);
	}

	__skb_tunnel_rx(skb, tunnel->dev, tunnel->net);

	err = dscp_ecn_decapsulate(tunnel, ipv6h, skb);
	if (unlikely(err)) {
		if (log_ecn_err)
			net_info_ratelimited("non-ECT from %pI6 with DS=%#x\n",
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

	skb_scrub_packet(skb, !net_eq(tunnel->net, dev_net(tunnel->dev)));

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
		blog_link(IF_DEVICE, blog_ptr(skb), (void*)tunnel->dev, DIR_RX, skb->len);
		/*TODO check here proto is set prorpely from userspace
		  when tunnle is created */
		if(tunnel->parms.proto == IPPROTO_GRE){
			blog_link(GRE_TUNL, blog_ptr(skb), (void*)tunnel, DIR_RX, 0);
			blog_link(TOS_MODE, blog_ptr(skb), tunnel, DIR_RX, BLOG_TOS_FIXED);
		}else{

			blog_link(TOS_MODE, blog_ptr(skb), NULL, DIR_RX, 
					(tunnel->parms.flags & IP6_TNL_F_RCV_DSCP_COPY) ?
					BLOG_TOS_INHERIT : BLOG_TOS_FIXED);
		}
#endif

	if (tun_dst)
		skb_dst_set(skb, (struct dst_entry *)tun_dst);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
		if ((skb->protocol != htons(ETH_P_IP))
#if IS_ENABLED(CONFIG_IPV6)
			&& (skb->protocol != htons(ETH_P_IPV6)) &&
            (tpi->proto != htons(ETH_P_TEB) || skb->protocol != htons(ETH_P_8021Q))
#endif
		) {
			blog_skip(skb, blog_skip_reason_unknown_proto);      /* No blogging */
		}
#endif

	gro_cells_receive(&tunnel->gro_cells, skb);
	return 0;

drop:
	if (tun_dst)
		dst_release((struct dst_entry *)tun_dst);
	kfree_skb(skb);
	return 0;
}

int ip6_tnl_rcv(struct ip6_tnl *t, struct sk_buff *skb,
		const struct tnl_ptk_info *tpi,
		struct metadata_dst *tun_dst,
		bool log_ecn_err)
{
	int (*dscp_ecn_decapsulate)(const struct ip6_tnl *t,
				    const struct ipv6hdr *ipv6h,
				    struct sk_buff *skb);

	dscp_ecn_decapsulate = ip6ip6_dscp_ecn_decapsulate;
	if (tpi->proto == htons(ETH_P_IP))
		dscp_ecn_decapsulate = ip4ip6_dscp_ecn_decapsulate;

	return __ip6_tnl_rcv(t, skb, tpi, tun_dst, dscp_ecn_decapsulate,
			     log_ecn_err);
}
EXPORT_SYMBOL(ip6_tnl_rcv);

static const struct tnl_ptk_info tpi_v6 = {
	/* no tunnel info required for ipxip6. */
	.proto = htons(ETH_P_IPV6),
};

static const struct tnl_ptk_info tpi_v4 = {
	/* no tunnel info required for ipxip6. */
	.proto = htons(ETH_P_IP),
};

static int ipxip6_rcv(struct sk_buff *skb, u8 ipproto,
		      const struct tnl_ptk_info *tpi,
		      int (*dscp_ecn_decapsulate)(const struct ip6_tnl *t,
						  const struct ipv6hdr *ipv6h,
						  struct sk_buff *skb))
{
	struct ip6_tnl *t;
	const struct ipv6hdr *ipv6h = ipv6_hdr(skb);
	struct metadata_dst *tun_dst = NULL;
	int ret = -1;

	rcu_read_lock();
	t = ip6_tnl_lookup(dev_net(skb->dev), &ipv6h->saddr, &ipv6h->daddr);

	if (t) {
		u8 tproto = READ_ONCE(t->parms.proto);

		if (tproto != ipproto && tproto != 0)
			goto drop;
		if (!xfrm6_policy_check(NULL, XFRM_POLICY_IN, skb))
			goto drop;
		ipv6h = ipv6_hdr(skb);
		if (!ip6_tnl_rcv_ctl(t, &ipv6h->daddr, &ipv6h->saddr))
			goto drop;
		if (iptunnel_pull_header(skb, 0, tpi->proto, false))
			goto drop;
		if (t->parms.collect_md) {
			tun_dst = ipv6_tun_rx_dst(skb, 0, 0, 0);
			if (!tun_dst)
				goto drop;
		}
		ret = __ip6_tnl_rcv(t, skb, tpi, tun_dst, dscp_ecn_decapsulate,
				    log_ecn_error);
	}

	rcu_read_unlock();

	return ret;

drop:
	rcu_read_unlock();
	kfree_skb(skb);
	return 0;
}

static int ip4ip6_rcv(struct sk_buff *skb)
{
	return ipxip6_rcv(skb, IPPROTO_IPIP, &tpi_v4,
			  ip4ip6_dscp_ecn_decapsulate);
}

static int ip6ip6_rcv(struct sk_buff *skb)
{
	return ipxip6_rcv(skb, IPPROTO_IPV6, &tpi_v6,
			  ip6ip6_dscp_ecn_decapsulate);
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

	opt->ops.dst1opt = (struct ipv6_opt_hdr *) opt->dst_opt;
	opt->ops.opt_nflen = 8;
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

static inline bool
ip6_tnl_addr_conflict(const struct ip6_tnl *t, const struct ipv6hdr *hdr)
{
	return ipv6_addr_equal(&t->parms.raddr, &hdr->saddr);
}

int ip6_tnl_xmit_ctl(struct ip6_tnl *t,
		     const struct in6_addr *laddr,
		     const struct in6_addr *raddr)
{
	struct __ip6_tnl_parm *p = &t->parms;
	int ret = 0;
	struct net *net = t->net;

	if (t->parms.collect_md)
		return 1;

	if ((p->flags & IP6_TNL_F_CAP_XMIT) ||
	    ((p->flags & IP6_TNL_F_CAP_PER_PACKET) &&
	     (ip6_tnl_get_cap(t, laddr, raddr) & IP6_TNL_F_CAP_XMIT))) {
		struct net_device *ldev = NULL;

		rcu_read_lock();
		if (p->link)
			ldev = dev_get_by_index_rcu(net, p->link);

		if (unlikely(!ipv6_chk_addr_and_flags(net, laddr, ldev, false,
						      0, IFA_F_TENTATIVE)))
			pr_warn("%s xmit: Local address not yet configured!\n",
				p->name);
		else if (!(p->flags & IP6_TNL_F_ALLOW_LOCAL_REMOTE) &&
			 !ipv6_addr_is_multicast(raddr) &&
			 unlikely(ipv6_chk_addr_and_flags(net, raddr, ldev,
							  true, 0, IFA_F_TENTATIVE)))
			pr_warn("%s xmit: Routing loop! Remote address found on this node!\n",
				p->name);
		else
			ret = 1;
		rcu_read_unlock();
	}
	return ret;
}
EXPORT_SYMBOL_GPL(ip6_tnl_xmit_ctl);

/**
 * ip6_tnl_xmit - encapsulate packet and send
 *   @skb: the outgoing socket buffer
 *   @dev: the outgoing tunnel device
 *   @dsfield: dscp code for outer header
 *   @fl6: flow of tunneled packet
 *   @encap_limit: encapsulation limit
 *   @pmtu: Path MTU is stored if packet is too big
 *   @proto: next header value
 *
 * Description:
 *   Build new header and do some sanity checks on the packet before sending
 *   it.
 *
 * Return:
 *   0 on success
 *   -1 fail
 *   %-EMSGSIZE message too big. return mtu in this case.
 **/

int ip6_tnl_xmit(struct sk_buff *skb, struct net_device *dev, __u8 dsfield,
		 struct flowi6 *fl6, int encap_limit, __u32 *pmtu,
		 __u8 proto)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct net *net = t->net;
	struct net_device_stats *stats = &t->dev->stats;
	struct ipv6hdr *ipv6h;
	struct ipv6_tel_txoption opt;
	struct dst_entry *dst = NULL, *ndst = NULL;
	struct net_device *tdev;
	int mtu;
	unsigned int eth_hlen = t->dev->type == ARPHRD_ETHER ? ETH_HLEN : 0;
	unsigned int psh_hlen = sizeof(struct ipv6hdr) + t->encap_hlen;
	unsigned int max_headroom = psh_hlen;
	bool use_cache = false;
	u8 hop_limit;
	int err = -1;
#if defined(CONFIG_BCM_KF_IPV6)
	u8 needFrag = 0;
#endif

	if (t->parms.collect_md) {
		hop_limit = skb_tunnel_info(skb)->key.ttl;
		goto route_lookup;
	} else {
		hop_limit = t->parms.hop_limit;
	}

	/* NBMA tunnel */
	if (ipv6_addr_any(&t->parms.raddr)) {
		if (skb->protocol == htons(ETH_P_IPV6)) {
			struct in6_addr *addr6;
			struct neighbour *neigh;
			int addr_type;

			if (!skb_dst(skb))
				goto tx_err_link_failure;

			neigh = dst_neigh_lookup(skb_dst(skb),
						 &ipv6_hdr(skb)->daddr);
			if (!neigh)
				goto tx_err_link_failure;

			addr6 = (struct in6_addr *)&neigh->primary_key;
			addr_type = ipv6_addr_type(addr6);

			if (addr_type == IPV6_ADDR_ANY)
				addr6 = &ipv6_hdr(skb)->daddr;

			memcpy(&fl6->daddr, addr6, sizeof(fl6->daddr));
			neigh_release(neigh);
		}
	} else if (t->parms.proto != 0 && !(t->parms.flags &
					    (IP6_TNL_F_USE_ORIG_TCLASS |
					     IP6_TNL_F_USE_ORIG_FWMARK))) {
		/* enable the cache only if neither the outer protocol nor the
		 * routing decision depends on the current inner header value
		 */
		use_cache = true;
	}

	if (use_cache)
		dst = dst_cache_get(&t->dst_cache);

	if (!ip6_tnl_xmit_ctl(t, &fl6->saddr, &fl6->daddr))
		goto tx_err_link_failure;

	if (!dst) {
route_lookup:
		/* add dsfield to flowlabel for route lookup */
		fl6->flowlabel = ip6_make_flowinfo(dsfield, fl6->flowlabel);

		dst = ip6_route_output(net, NULL, fl6);

		if (dst->error)
			goto tx_err_link_failure;
		dst = xfrm_lookup(net, dst, flowi6_to_flowi(fl6), NULL, 0);
		if (IS_ERR(dst)) {
			err = PTR_ERR(dst);
			dst = NULL;
			goto tx_err_link_failure;
		}
		if (t->parms.collect_md && ipv6_addr_any(&fl6->saddr) &&
		    ipv6_dev_get_saddr(net, ip6_dst_idev(dst)->dev,
				       &fl6->daddr, 0, &fl6->saddr))
			goto tx_err_link_failure;
		ndst = dst;
	}

	tdev = dst->dev;

	if (tdev == dev) {
		stats->collisions++;
		net_warn_ratelimited("%s: Local routing loop detected!\n",
				     t->parms.name);
		goto tx_err_dst_release;
	}
	mtu = dst_mtu(dst) - eth_hlen - psh_hlen - t->tun_hlen;
	if (encap_limit >= 0) {
		max_headroom += 8;
		mtu -= 8;
	}
	mtu = max(mtu, skb->protocol == htons(ETH_P_IPV6) ?
		       IPV6_MIN_MTU : IPV4_MIN_MTU);

	skb_dst_update_pmtu_no_confirm(skb, mtu);
	if (skb->len - t->tun_hlen - eth_hlen > mtu && !skb_is_gso(skb)) {
		*pmtu = mtu;
#if defined(CONFIG_BCM_KF_IPV6)
		needFrag = 1;
#else      
		err = -EMSGSIZE;
		goto tx_err_dst_release;
#endif
	}

	if (t->err_count > 0) {
		if (time_before(jiffies,
				t->err_time + IP6TUNNEL_ERR_TIMEO)) {
			t->err_count--;

			dst_link_failure(skb);
		} else {
			t->err_count = 0;
		}
	}

	skb_scrub_packet(skb, !net_eq(t->net, dev_net(dev)));

	/*
	 * Okay, now see if we can stuff it in the buffer as-is.
	 */
	max_headroom += LL_RESERVED_SPACE(tdev);

	if (skb_headroom(skb) < max_headroom || skb_shared(skb) ||
	    (skb_cloned(skb) && !skb_clone_writable(skb, 0))) {
		struct sk_buff *new_skb;

		new_skb = skb_realloc_headroom(skb, max_headroom);
		if (!new_skb)
			goto tx_err_dst_release;

		if (skb->sk)
			skb_set_owner_w(new_skb, skb->sk);
		consume_skb(skb);
		skb = new_skb;
	}

	if (t->parms.collect_md) {
		if (t->encap.type != TUNNEL_ENCAP_NONE)
			goto tx_err_dst_release;
	} else {
		if (use_cache && ndst)
			dst_cache_set_ip6(&t->dst_cache, ndst, &fl6->saddr);
	}
	skb_dst_set(skb, dst);

	if (hop_limit == 0) {
		if (skb->protocol == htons(ETH_P_IP))
			hop_limit = ip_hdr(skb)->ttl;
		else if (skb->protocol == htons(ETH_P_IPV6))
			hop_limit = ipv6_hdr(skb)->hop_limit;
		else
			hop_limit = ip6_dst_hoplimit(dst);
	}

	/* Calculate max headroom for all the headers and adjust
	 * needed_headroom if necessary.
	 */
	max_headroom = LL_RESERVED_SPACE(dst->dev) + sizeof(struct ipv6hdr)
			+ dst->header_len + t->hlen;
	if (max_headroom > dev->needed_headroom)
		dev->needed_headroom = max_headroom;

	err = ip6_tnl_encap(skb, t, &proto, fl6);
	if (err)
		return err;

	if (encap_limit >= 0) {
		init_tel_txopt(&opt, encap_limit);
		ipv6_push_frag_opts(skb, &opt.ops, &proto);
	}

	skb_push(skb, sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	ipv6h = ipv6_hdr(skb);
	ip6_flow_hdr(ipv6h, dsfield,
		     ip6_make_flowlabel(net, skb, fl6->flowlabel, true, fl6));
	ipv6h->hop_limit = hop_limit;
	ipv6h->nexthdr = proto;
	ipv6h->saddr = fl6->saddr;
	ipv6h->daddr = fl6->daddr;
#if defined(CONFIG_BCM_KF_IPV6)
	if (needFrag) {
		skb->ignore_df = 1;
		ip6_fragment(net, skb->sk, skb, ip6_local_out);
	}
	else {
		ip6tunnel_xmit(skb->sk, skb, dev);
	}
#else
	ip6tunnel_xmit(NULL, skb, dev);
#endif
	return 0;
tx_err_link_failure:
	stats->tx_carrier_errors++;
	dst_link_failure(skb);
tx_err_dst_release:
	dst_release(dst);
	return err;
}
EXPORT_SYMBOL(ip6_tnl_xmit);

static void reChecksum(struct sk_buff *skb)
{
	struct iphdr  *iph = ip_hdr(skb);

	//Repoint to the correct ip header.
	skb_set_transport_header(skb, sizeof(struct iphdr));

	skb->ip_summed = CHECKSUM_NONE;
	skb->csum_valid = 0;
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

	if ( (iph->protocol == IPPROTO_TCP) || (iph->protocol == IPPROTO_UDP) ) {
		if(skb_is_nonlinear(skb))
			skb_linearize(skb);

		if (iph->protocol == IPPROTO_TCP) {
			struct tcphdr *tcpHdr;
			unsigned int tcplen;

			tcpHdr = tcp_hdr(skb);
			skb->csum =0;
			tcplen = ntohs(iph->tot_len) - iph->ihl*4;
			tcpHdr->check = 0;
			tcpHdr->check = tcp_v4_check(tcplen, iph->saddr, iph->daddr, csum_partial((char *)tcpHdr, tcplen, 0));

			//printk(KERN_INFO "[checksum]: TCP Len :%d, Computed TCP Checksum :%x : Network : %x\n", tcplen, tcpHdr->check, htons(tcpHdr->check));
		} else if (iph->protocol == IPPROTO_UDP) {
			struct udphdr *udpHdr;
			unsigned int udplen;

			udpHdr = udp_hdr(skb);
			skb->csum =0;
			udplen = ntohs(iph->tot_len) - iph->ihl*4;
			udpHdr->check = 0;
			udpHdr->check = udp_v4_check(udplen, iph->saddr, iph->daddr,csum_partial((char *)udpHdr, udplen, 0));;

			//printk(KERN_INFO "[checksum]: UDP Len :%d, Computed UDP Checksum :%x : Network : %x\n", udplen, udpHdr->check, htons(udpHdr->check));
		}
	}
}

static inline int
ip4ip6_tnl_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	const struct iphdr  *iph;
	struct __ip6_tnl_fmr *fmr;
	int encap_limit = -1;
	struct flowi6 fl6;
	__u8 dsfield;
	__u32 mtu;
	u8 tproto;
	int err;
	bool draft = false;

	iph = ip_hdr(skb);
	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));

	tproto = READ_ONCE(t->parms.proto);
	if (tproto != IPPROTO_IPIP && tproto != 0)
		return -1;

	if (t->parms.collect_md) {
		struct ip_tunnel_info *tun_info;
		const struct ip_tunnel_key *key;

		tun_info = skb_tunnel_info(skb);
		if (unlikely(!tun_info || !(tun_info->mode & IP_TUNNEL_INFO_TX) ||
			     ip_tunnel_info_af(tun_info) != AF_INET6))
			return -1;
		key = &tun_info->key;
		memset(&fl6, 0, sizeof(fl6));
		fl6.flowi6_proto = IPPROTO_IPIP;
		fl6.saddr = key->u.ipv6.src;
		fl6.daddr = key->u.ipv6.dst;
		fl6.flowlabel = key->label;
		dsfield =  key->tos;
	} else {

		/* if match fake destination addr,
		   replace destination address to source address */
		if(iph->daddr == in_aton("169.254.7.7")) {
			memcpy(skb->data+16, skb->data+12, 4);
			//re-calculator checksum
			reChecksum(skb);
		}

		if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
			encap_limit = t->parms.encap_limit;

		memcpy(&fl6, &t->fl.u.ip6, sizeof(fl6));
		fl6.flowi6_proto = IPPROTO_IPIP;

		if (t->parms.flags & IP6_TNL_F_USE_ORIG_TCLASS)
			dsfield = ipv4_get_dsfield(iph);
		else
			dsfield = ip6_tclass(t->parms.flowinfo);
		if (t->parms.flags & IP6_TNL_F_USE_ORIG_FWMARK)
			fl6.flowi6_mark = skb->mark;
		else
			fl6.flowi6_mark = t->parms.fwmark;
	}

	/* try to find matching FMR */
	for (fmr = t->parms.fmrs; fmr; fmr = fmr->next) {
		unsigned mshift = 32 - fmr->ip4_prefix_len;
		if (ntohl(fmr->ip4_prefix.s_addr) >> mshift ==
				ntohl(ip_hdr(skb)->daddr) >> mshift)
			break;
	}

	if (t->parms.flags & IP6_TNL_F_USE_FMR_DRAFT) {
		draft = true;
	}
	/* change dstaddr according to FMR */
	if (fmr)
		ip4ip6_fmr_calc(&fl6.daddr, ip_hdr(skb), skb_tail_pointer(skb), fmr, true, draft);

	fl6.flowi6_uid = sock_net_uid(dev_net(dev), NULL);
	dsfield = INET_ECN_encapsulate(dsfield, ipv4_get_dsfield(iph));

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_link(TOS_MODE, blog_ptr(skb), NULL, DIR_TX,
		(t->parms.flags & IP6_TNL_F_USE_ORIG_TCLASS) ?
			BLOG_TOS_INHERIT : BLOG_TOS_FIXED);
	blog_link(IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_TX, skb->len);
#endif

	if (iptunnel_handle_offloads(skb, SKB_GSO_IPXIP6))
		return -1;

	skb_set_inner_ipproto(skb, IPPROTO_IPIP);

	err = ip6_tnl_xmit(skb, dev, dsfield, &fl6, encap_limit, &mtu,
			   IPPROTO_IPIP);
	if (err != 0) {
		/* XXX: send ICMP error even if DF is not set. */
		if (err == -EMSGSIZE)
			icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED,
				  htonl(mtu));
		return -1;
	}

	return 0;
}

static inline int
ip6ip6_tnl_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct ipv6hdr *ipv6h;
	struct __ip6_tnl_fmr *fmr;
	int encap_limit = -1;
	__u16 offset;
	struct flowi6 fl6;
	__u8 dsfield;
	__u32 mtu;
	u8 tproto;
	int err;
	bool draft = false;

	ipv6h = ipv6_hdr(skb);
	tproto = READ_ONCE(t->parms.proto);
	if ((tproto != IPPROTO_IPV6 && tproto != 0) ||
	    ip6_tnl_addr_conflict(t, ipv6h))
		return -1;

	if (t->parms.collect_md) {
		struct ip_tunnel_info *tun_info;
		const struct ip_tunnel_key *key;

		tun_info = skb_tunnel_info(skb);
		if (unlikely(!tun_info || !(tun_info->mode & IP_TUNNEL_INFO_TX) ||
			     ip_tunnel_info_af(tun_info) != AF_INET6))
			return -1;
		key = &tun_info->key;
		memset(&fl6, 0, sizeof(fl6));
		fl6.flowi6_proto = IPPROTO_IPV6;
		fl6.saddr = key->u.ipv6.src;
		fl6.daddr = key->u.ipv6.dst;
		fl6.flowlabel = key->label;
		dsfield = key->tos;
	} else {
		offset = ip6_tnl_parse_tlv_enc_lim(skb, skb_network_header(skb));
		/* ip6_tnl_parse_tlv_enc_lim() might have reallocated skb->head */
		ipv6h = ipv6_hdr(skb);
		if (offset > 0) {
			struct ipv6_tlv_tnl_enc_lim *tel;

			tel = (void *)&skb_network_header(skb)[offset];
			if (tel->encap_limit == 0) {
				icmpv6_send(skb, ICMPV6_PARAMPROB,
					    ICMPV6_HDR_FIELD, offset + 2);
				return -1;
			}
			encap_limit = tel->encap_limit - 1;
		} else if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT)) {
			encap_limit = t->parms.encap_limit;
		}

		memcpy(&fl6, &t->fl.u.ip6, sizeof(fl6));
		fl6.flowi6_proto = IPPROTO_IPV6;

		if (t->parms.flags & IP6_TNL_F_USE_ORIG_TCLASS)
			dsfield = ipv6_get_dsfield(ipv6h);
		else
			dsfield = ip6_tclass(t->parms.flowinfo);
		if (t->parms.flags & IP6_TNL_F_USE_ORIG_FLOWLABEL)
			fl6.flowlabel |= ip6_flowlabel(ipv6h);
		if (t->parms.flags & IP6_TNL_F_USE_ORIG_FWMARK)
			fl6.flowi6_mark = skb->mark;
		else
			fl6.flowi6_mark = t->parms.fwmark;
	}

	fl6.flowi6_uid = sock_net_uid(dev_net(dev), NULL);
	dsfield = INET_ECN_encapsulate(dsfield, ipv6_get_dsfield(ipv6h));

	/* try to find matching FMR */
	for (fmr = t->parms.fmrs; fmr; fmr = fmr->next) {
		unsigned mshift = 32 - fmr->ip4_prefix_len;
		if (ntohl(fmr->ip4_prefix.s_addr) >> mshift ==
				ntohl(ip_hdr(skb)->daddr) >> mshift)
			break;
	}

	if (t->parms.flags & IP6_TNL_F_USE_FMR_DRAFT) {
		draft = true;
	}
	/* change dstaddr according to FMR */
	if (fmr)
		ip4ip6_fmr_calc(&fl6.daddr, ip_hdr(skb), skb_tail_pointer(skb), fmr, true, draft);

	if (iptunnel_handle_offloads(skb, SKB_GSO_IPXIP6))
		return -1;

	skb_set_inner_ipproto(skb, IPPROTO_IPV6);

	err = ip6_tnl_xmit(skb, dev, dsfield, &fl6, encap_limit, &mtu,
			   IPPROTO_IPV6);
	if (err != 0) {
		if (err == -EMSGSIZE)
			icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
		return -1;
	}

	return 0;
}

static netdev_tx_t
ip6_tnl_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct net_device_stats *stats = &t->dev->stats;
	int ret;

	if (!pskb_inet_may_pull(skb))
		goto tx_err;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		ret = ip4ip6_tnl_xmit(skb, dev);
		break;
	case htons(ETH_P_IPV6):
		ret = ip6ip6_tnl_xmit(skb, dev);
		break;
	default:
		goto tx_err;
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

static void ip6_tnl_link_config(struct ip6_tnl *t)
{
	struct net_device *dev = t->dev;
	struct __ip6_tnl_parm *p = &t->parms;
	struct flowi6 *fl6 = &t->fl.u.ip6;
	int t_hlen;

	memcpy(dev->dev_addr, &p->laddr, sizeof(struct in6_addr));
	memcpy(dev->broadcast, &p->raddr, sizeof(struct in6_addr));

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

	if (p->flags&IP6_TNL_F_CAP_XMIT && p->flags&IP6_TNL_F_CAP_RCV)
		dev->flags |= IFF_POINTOPOINT;
	else
		dev->flags &= ~IFF_POINTOPOINT;

	t->tun_hlen = 0;
	t->hlen = t->encap_hlen + t->tun_hlen;
	t_hlen = t->hlen + sizeof(struct ipv6hdr);

	if (p->flags & IP6_TNL_F_CAP_XMIT) {
		int strict = (ipv6_addr_type(&p->raddr) &
			      (IPV6_ADDR_MULTICAST|IPV6_ADDR_LINKLOCAL));

		struct rt6_info *rt = rt6_lookup(t->net,
						 &p->raddr, &p->laddr,
						 p->link, NULL, strict);

		if (!rt)
			return;

		if (rt->dst.dev) {
			dev->hard_header_len = rt->dst.dev->hard_header_len +
				t_hlen;

			dev->mtu = rt->dst.dev->mtu - t_hlen;
			if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
				dev->mtu -= 8;

			if (dev->mtu < IPV6_MIN_MTU)
				dev->mtu = IPV6_MIN_MTU;
		}
		ip6_rt_put(rt);
	}
}

/**
 * ip6_tnl_change - update the tunnel parameters
 *   @t: tunnel to be changed
 *   @p: tunnel configuration parameters
 *
 * Description:
 *   ip6_tnl_change() updates the tunnel parameters
 **/

static int
ip6_tnl_change(struct ip6_tnl *t, const struct __ip6_tnl_parm *p)
{
	t->parms.laddr = p->laddr;
	t->parms.raddr = p->raddr;
	t->parms.flags = p->flags;
	t->parms.hop_limit = p->hop_limit;
	t->parms.encap_limit = p->encap_limit;
	t->parms.flowinfo = p->flowinfo;
	t->parms.link = p->link;
	t->parms.proto = p->proto;
	t->parms.fwmark = p->fwmark;

	while (t->parms.fmrs) {
		struct __ip6_tnl_fmr *next = t->parms.fmrs->next;
		kfree(t->parms.fmrs);
		t->parms.fmrs = next;
	}
	t->parms.fmrs = p->fmrs;

	dst_cache_reset(&t->dst_cache);
	ip6_tnl_link_config(t);
	return 0;
}

static int ip6_tnl_update(struct ip6_tnl *t, struct __ip6_tnl_parm *p)
{
	struct net *net = t->net;
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);
	int err;

	ip6_tnl_unlink(ip6n, t);
	synchronize_net();
	err = ip6_tnl_change(t, p);
	ip6_tnl_link(ip6n, t);
	netdev_state_change(t->dev);
	return err;
}

static int ip6_tnl0_update(struct ip6_tnl *t, struct __ip6_tnl_parm *p)
{
	/* for default tnl0 device allow to change only the proto */
	t->parms.proto = p->proto;
	netdev_state_change(t->dev);
	return 0;
}

static void
ip6_tnl_parm_from_user(struct __ip6_tnl_parm *p, const struct ip6_tnl_parm *u)
{
	p->laddr = u->laddr;
	p->raddr = u->raddr;
	p->flags = u->flags;
	p->hop_limit = u->hop_limit;
	p->encap_limit = u->encap_limit;
	p->flowinfo = u->flowinfo;
	p->link = u->link;
	p->proto = u->proto;
	p->fmrs = NULL;
	memcpy(p->name, u->name, sizeof(u->name));
}

static void
ip6_tnl_parm_to_user(struct ip6_tnl_parm *u, const struct __ip6_tnl_parm *p)
{
	u->laddr = p->laddr;
	u->raddr = p->raddr;
	u->flags = p->flags;
	u->hop_limit = p->hop_limit;
	u->encap_limit = p->encap_limit;
	u->flowinfo = p->flowinfo;
	u->link = p->link;
	u->proto = p->proto;
	memcpy(u->name, p->name, sizeof(u->name));
}

/**
 * ip6_tnl_ioctl - configure ipv6 tunnels from userspace
 *   @dev: virtual device associated with tunnel
 *   @ifr: parameters passed from userspace
 *   @cmd: command to be performed
 *
 * Description:
 *   ip6_tnl_ioctl() is used for managing IPv6 tunnels
 *   from userspace.
 *
 *   The possible commands are the following:
 *     %SIOCGETTUNNEL: get tunnel parameters for device
 *     %SIOCADDTUNNEL: add tunnel matching given tunnel parameters
 *     %SIOCCHGTUNNEL: change tunnel parameters to those given
 *     %SIOCDELTUNNEL: delete tunnel
 *
 *   The fallback device "ip6tnl0", created during module
 *   initialization, can be used for creating other tunnel devices.
 *
 * Return:
 *   0 on success,
 *   %-EFAULT if unable to copy data to or from userspace,
 *   %-EPERM if current process hasn't %CAP_NET_ADMIN set
 *   %-EINVAL if passed tunnel parameters are invalid,
 *   %-EEXIST if changing a tunnel's parameters would cause a conflict
 *   %-ENODEV if attempting to change or delete a nonexisting device
 **/

static int
ip6_tnl_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int err = 0;
	struct ip6_tnl_parm p;
	struct __ip6_tnl_parm p1;
	struct ip6_tnl *t = netdev_priv(dev);
	struct net *net = t->net;
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);

	memset(&p1, 0, sizeof(p1));

	switch (cmd) {
	case SIOCGETTUNNEL:
		if (dev == ip6n->fb_tnl_dev) {
			if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p))) {
				err = -EFAULT;
				break;
			}
			ip6_tnl_parm_from_user(&p1, &p);
			t = ip6_tnl_locate(net, &p1, 0);
			if (IS_ERR(t))
				t = netdev_priv(dev);
		} else {
			memset(&p, 0, sizeof(p));
		}
		ip6_tnl_parm_to_user(&p, &t->parms);
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &p, sizeof(p))) {
			err = -EFAULT;
		}
		break;
	case SIOCADDTUNNEL:
	case SIOCCHGTUNNEL:
		err = -EPERM;
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;
		err = -EFAULT;
		if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p)))
			break;
		err = -EINVAL;
		if (p.proto != IPPROTO_IPV6 && p.proto != IPPROTO_IPIP &&
		    p.proto != 0)
			break;
		ip6_tnl_parm_from_user(&p1, &p);
		t = ip6_tnl_locate(net, &p1, cmd == SIOCADDTUNNEL);
		if (cmd == SIOCCHGTUNNEL) {
			if (!IS_ERR(t)) {
				if (t->dev != dev) {
					err = -EEXIST;
					break;
				}
			} else
				t = netdev_priv(dev);
			if (dev == ip6n->fb_tnl_dev)
				err = ip6_tnl0_update(t, &p1);
			else
				err = ip6_tnl_update(t, &p1);
		}
		if (!IS_ERR(t)) {
			err = 0;
			ip6_tnl_parm_to_user(&p, &t->parms);
			if (copy_to_user(ifr->ifr_ifru.ifru_data, &p, sizeof(p)))
				err = -EFAULT;

		} else {
			err = PTR_ERR(t);
		}
		break;
	case SIOCDELTUNNEL:
		err = -EPERM;
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;

		if (dev == ip6n->fb_tnl_dev) {
			err = -EFAULT;
			if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p)))
				break;
			err = -ENOENT;
			ip6_tnl_parm_from_user(&p1, &p);
			t = ip6_tnl_locate(net, &p1, 0);
			if (IS_ERR(t))
				break;
			err = -EPERM;
			if (t->dev == ip6n->fb_tnl_dev)
				break;
			dev = t->dev;
		}
		err = 0;
		unregister_netdevice(dev);
		break;
	default:
		err = -EINVAL;
	}
	return err;
}

/**
 * ip6_tnl_change_mtu - change mtu manually for tunnel device
 *   @dev: virtual device associated with tunnel
 *   @new_mtu: the new mtu
 *
 * Return:
 *   0 on success,
 *   %-EINVAL if mtu too small
 **/

int ip6_tnl_change_mtu(struct net_device *dev, int new_mtu)
{
	struct ip6_tnl *tnl = netdev_priv(dev);

	if (tnl->parms.proto == IPPROTO_IPV6) {
		if (new_mtu < IPV6_MIN_MTU)
			return -EINVAL;
	} else {
		if (new_mtu < ETH_MIN_MTU)
			return -EINVAL;
	}
	if (tnl->parms.proto == IPPROTO_IPV6 || tnl->parms.proto == 0) {
		if (new_mtu > IP6_MAX_MTU - dev->hard_header_len)
			return -EINVAL;
	} else {
		if (new_mtu > IP_MAX_MTU - dev->hard_header_len)
			return -EINVAL;
	}
	dev->mtu = new_mtu;
	return 0;
}
EXPORT_SYMBOL(ip6_tnl_change_mtu);

int ip6_tnl_get_iflink(const struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);

	return t->parms.link;
}
EXPORT_SYMBOL(ip6_tnl_get_iflink);

int ip6_tnl_encap_add_ops(const struct ip6_tnl_encap_ops *ops,
			  unsigned int num)
{
	if (num >= MAX_IPTUN_ENCAP_OPS)
		return -ERANGE;

	return !cmpxchg((const struct ip6_tnl_encap_ops **)
			&ip6tun_encaps[num],
			NULL, ops) ? 0 : -1;
}
EXPORT_SYMBOL(ip6_tnl_encap_add_ops);

int ip6_tnl_encap_del_ops(const struct ip6_tnl_encap_ops *ops,
			  unsigned int num)
{
	int ret;

	if (num >= MAX_IPTUN_ENCAP_OPS)
		return -ERANGE;

	ret = (cmpxchg((const struct ip6_tnl_encap_ops **)
		       &ip6tun_encaps[num],
		       ops, NULL) == ops) ? 0 : -1;

	synchronize_net();

	return ret;
}
EXPORT_SYMBOL(ip6_tnl_encap_del_ops);

int ip6_tnl_encap_setup(struct ip6_tnl *t,
			struct ip_tunnel_encap *ipencap)
{
	int hlen;

	memset(&t->encap, 0, sizeof(t->encap));

	hlen = ip6_encap_hlen(ipencap);
	if (hlen < 0)
		return hlen;

	t->encap.type = ipencap->type;
	t->encap.sport = ipencap->sport;
	t->encap.dport = ipencap->dport;
	t->encap.flags = ipencap->flags;

	t->encap_hlen = hlen;
	t->hlen = t->encap_hlen + t->tun_hlen;

	return 0;
}
EXPORT_SYMBOL_GPL(ip6_tnl_encap_setup);

static const struct net_device_ops ip6_tnl_netdev_ops = {
	.ndo_init	= ip6_tnl_dev_init,
	.ndo_uninit	= ip6_tnl_dev_uninit,
	.ndo_start_xmit = ip6_tnl_start_xmit,
	.ndo_do_ioctl	= ip6_tnl_ioctl,
	.ndo_change_mtu = ip6_tnl_change_mtu,
	.ndo_get_stats	= ip6_get_stats,
	.ndo_get_iflink = ip6_tnl_get_iflink,
};

#define IPXIPX_FEATURES (NETIF_F_SG |		\
			 NETIF_F_FRAGLIST |	\
			 NETIF_F_HIGHDMA |	\
			 NETIF_F_GSO_SOFTWARE |	\
			 NETIF_F_HW_CSUM)

/**
 * ip6_tnl_dev_setup - setup virtual tunnel device
 *   @dev: virtual device associated with tunnel
 *
 * Description:
 *   Initialize function pointers and device parameters
 **/

static void ip6_tnl_dev_setup(struct net_device *dev)
{
	dev->netdev_ops = &ip6_tnl_netdev_ops;
	dev->needs_free_netdev = true;
	dev->priv_destructor = ip6_dev_free;

	dev->type = ARPHRD_TUNNEL6;
	dev->flags |= IFF_NOARP;
	dev->addr_len = sizeof(struct in6_addr);
	dev->features |= NETIF_F_LLTX;
	netif_keep_dst(dev);

	dev->features		|= IPXIPX_FEATURES;
	dev->hw_features	|= IPXIPX_FEATURES;

	/* This perm addr will be used as interface identifier by IPv6 */
	dev->addr_assign_type = NET_ADDR_RANDOM;
	eth_random_addr(dev->perm_addr);
}


/**
 * ip6_tnl_dev_init_gen - general initializer for all tunnel devices
 *   @dev: virtual device associated with tunnel
 **/

static inline int
ip6_tnl_dev_init_gen(struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	int ret;
	int t_hlen;

	t->dev = dev;
	t->net = dev_net(dev);
	dev->tstats = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!dev->tstats)
		return -ENOMEM;

	ret = dst_cache_init(&t->dst_cache, GFP_KERNEL);
	if (ret)
		goto free_stats;

	ret = gro_cells_init(&t->gro_cells, dev);
	if (ret)
		goto destroy_dst;

	t->tun_hlen = 0;
	t->hlen = t->encap_hlen + t->tun_hlen;
	t_hlen = t->hlen + sizeof(struct ipv6hdr);

	dev->type = ARPHRD_TUNNEL6;
	dev->hard_header_len = LL_MAX_HEADER + t_hlen;
	dev->mtu = ETH_DATA_LEN - t_hlen;
	if (!(t->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT))
		dev->mtu -= 8;
	dev->min_mtu = ETH_MIN_MTU;
	dev->max_mtu = IP6_MAX_MTU - dev->hard_header_len;

	return 0;

destroy_dst:
	dst_cache_destroy(&t->dst_cache);
free_stats:
	free_percpu(dev->tstats);
	dev->tstats = NULL;

	return ret;
}

/**
 * ip6_tnl_dev_init - initializer for all non fallback tunnel devices
 *   @dev: virtual device associated with tunnel
 **/

static int ip6_tnl_dev_init(struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	int err = ip6_tnl_dev_init_gen(dev);

	if (err)
		return err;
	ip6_tnl_link_config(t);
	if (t->parms.collect_md)
		netif_keep_dst(dev);
	return 0;
}

/**
 * ip6_fb_tnl_dev_init - initializer for fallback tunnel device
 *   @dev: fallback device
 *
 * Return: 0
 **/

static int __net_init ip6_fb_tnl_dev_init(struct net_device *dev)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct net *net = dev_net(dev);
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);

	t->parms.proto = IPPROTO_IPV6;
	dev_hold(dev);

	rcu_assign_pointer(ip6n->tnls_wc[0], t);
	return 0;
}

static int ip6_tnl_validate(struct nlattr *tb[], struct nlattr *data[],
			    struct netlink_ext_ack *extack)
{
	u8 proto;

	if (!data || !data[IFLA_IPTUN_PROTO])
		return 0;

	proto = nla_get_u8(data[IFLA_IPTUN_PROTO]);
	if (proto != IPPROTO_IPV6 &&
	    proto != IPPROTO_IPIP &&
	    proto != 0)
		return -EINVAL;

	return 0;
}

static const struct nla_policy ip6_tnl_fmr_policy[IFLA_IPTUN_FMR_MAX + 1] = {
	[IFLA_IPTUN_FMR_IP6_PREFIX] = { .len = sizeof(struct in6_addr) },
	[IFLA_IPTUN_FMR_IP4_PREFIX] = { .len = sizeof(struct in_addr) },
	[IFLA_IPTUN_FMR_IP6_PREFIX_LEN] = { .type = NLA_U8 },
	[IFLA_IPTUN_FMR_IP4_PREFIX_LEN] = { .type = NLA_U8 },
	[IFLA_IPTUN_FMR_EA_LEN] = { .type = NLA_U8 },
	[IFLA_IPTUN_FMR_OFFSET] = { .type = NLA_U8 }
};

static int fmr_install(struct net_device *dev,
		       struct __ip6_tnl_parm *parms)
{
	struct file *fp;
	char fpath[20];
	char *line_buf;
	int BUF_SIZE = 1024;
	int line_buf_pos = 0;
	int ret, cnt = 0, unit = -1;

	struct __ip6_tnl_fmr *nfmr;
	struct in6_addr ddr6;
	struct in_addr ddr;
	char ip6_prefix[INET6_ADDRSTRLEN], ip4_prefix[INET_ADDRSTRLEN];
	int ip6_prefix_len, ip4_prefix_len, offset, ea_len;

	loff_t pos = 0;
	mm_segment_t oldfs;

	if (sscanf(dev->name, "v4tun%d", &unit) != 1) {
		printk(KERN_ERR "[%s(%d)][%s]tunnel dev mismatch.\n",
			__FUNCTION__, __LINE__, dev->name);
		return 0;
	}

	if (unit < 0)
		return 0;

	snprintf(fpath, sizeof(fpath), "/tmp/v6maps.%d", unit);
	if (!(fp = OpenFile(fpath, O_RDONLY, 0664)))
		return 0;

	line_buf = kcalloc(BUF_SIZE + 1, sizeof(char), GFP_KERNEL);
	if (!line_buf) {
		printk(KERN_ERR "[%s(%d)][%s]kcalloc failed.\n",
			__FUNCTION__, __LINE__, dev->name);
		CloseFile(fp);
		return 0;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	while ((ret = kernel_read(fp, &line_buf[line_buf_pos], BUF_SIZE - line_buf_pos - 1, &pos)) > 0) {

		line_buf_pos += ret;
		line_buf[line_buf_pos] = '\0';

		char *line_start = line_buf;
		char *line_end = strchr(line_start, '\n');

		while (line_end != NULL) {

			*line_end = '\0';

			if (sscanf(line_start, "%s %d %s %d %d %d",
				ip4_prefix, &ip4_prefix_len, ip6_prefix,
				&ip6_prefix_len, &ea_len, &offset) != 6)
				goto next;
			if (!(nfmr = kzalloc(sizeof(*nfmr), GFP_KERNEL)))
				goto next;

			nfmr->offset = 6;

			in6_pton(ip6_prefix, sizeof(ip6_prefix), (void *)&ddr6, -1, NULL);
			memcpy(&nfmr->ip6_prefix, &ddr6, sizeof(nfmr->ip6_prefix));

			in4_pton(ip4_prefix, sizeof(ip4_prefix), (void*)&ddr, -1, NULL);
			memcpy(&nfmr->ip4_prefix, &ddr, sizeof(nfmr->ip4_prefix));

			nfmr->ip6_prefix_len = ip6_prefix_len;
			nfmr->ip4_prefix_len = ip4_prefix_len;
			nfmr->ea_len = ea_len;
			nfmr->offset = offset;

			nfmr->next = parms->fmrs;
			parms->fmrs = nfmr;
			cnt++;
			printk(KERN_INFO "[%s(%d)]FMR:[%s]\n",
				__FUNCTION__, __LINE__, line_start);
		next:
			line_start = line_end + 1;
			line_end = strchr(line_start, '\n');
		}

		// Move the remaining partial line to the beginning of the buffer
		int remaining_len = line_buf_pos - (line_start - line_buf);
		memmove(line_buf, line_start, remaining_len);
		line_buf_pos = remaining_len;

		// If the remaining space in the buffer is not enough for a line,
		// increase the buffer size
		if (line_buf_pos >= BUF_SIZE - 1) {
			BUF_SIZE *= 2;
			line_buf = krealloc(line_buf, BUF_SIZE, GFP_KERNEL);
			if (line_buf == NULL) {
				printk(KERN_ERR "[%s(%d)][%s]krealloc failed.\n",
					__FUNCTION__, __LINE__, dev->name);
				return 0;
			}
		}
	}

	// Process the remaining partial line if there is any
	if (line_buf_pos > 0) {
		printk(KERN_INFO "[%s(%d)]LAST:[%s]\n",
			__FUNCTION__, __LINE__, line_buf);
	}

	set_fs(oldfs);
	kfree(line_buf);
	CloseFile(fp);

	if (cnt)
		printk("[%s][%s]FMR is installed.(%d)\n",
			__FUNCTION__, dev->name, cnt);
	else
		printk(KERN_ERR "[%s][%s]Install FMR err.\n",
			__FUNCTION__, dev->name);
	return cnt;
}

static void fmr_nla_install(struct nlattr *data[],
			   struct net_device *dev,
			   struct __ip6_tnl_parm *parms)
{
	unsigned rem;
	struct nlattr *fmr;
	int cnt = 0;

	nla_for_each_nested(fmr, data[IFLA_IPTUN_FMRS], rem) {
		struct nlattr *fmrd[IFLA_IPTUN_FMR_MAX + 1], *c;
		struct __ip6_tnl_fmr *nfmr;

		nla_parse_nested(fmrd, IFLA_IPTUN_FMR_MAX,
			fmr, ip6_tnl_fmr_policy, NULL);

		if (!(nfmr = kzalloc(sizeof(*nfmr), GFP_KERNEL)))
			continue;

		nfmr->offset = 6;

		if ((c = fmrd[IFLA_IPTUN_FMR_IP6_PREFIX])) {
			nla_memcpy(&nfmr->ip6_prefix, fmrd[IFLA_IPTUN_FMR_IP6_PREFIX],
				sizeof(nfmr->ip6_prefix));
			//printk("[%s(%d)]%lu<%pI6>\n", __FUNCTION__, __LINE__, sizeof(nfmr->ip6_prefix), &nfmr->ip6_prefix);
		}

		if ((c = fmrd[IFLA_IPTUN_FMR_IP4_PREFIX])) {
			nla_memcpy(&nfmr->ip4_prefix, fmrd[IFLA_IPTUN_FMR_IP4_PREFIX],
				sizeof(nfmr->ip4_prefix));
			//printk("[%s(%d)]%lu<%pI4>\n", __FUNCTION__, __LINE__, sizeof(nfmr->ip4_prefix), &nfmr->ip4_prefix);
		}

		if ((c = fmrd[IFLA_IPTUN_FMR_IP6_PREFIX_LEN]))
			nfmr->ip6_prefix_len = nla_get_u8(c);

		if ((c = fmrd[IFLA_IPTUN_FMR_IP4_PREFIX_LEN]))
			nfmr->ip4_prefix_len = nla_get_u8(c);

		if ((c = fmrd[IFLA_IPTUN_FMR_EA_LEN]))
			nfmr->ea_len = nla_get_u8(c);

		if ((c = fmrd[IFLA_IPTUN_FMR_OFFSET]))
			nfmr->offset = nla_get_u8(c);

		cnt++;
		nfmr->next = parms->fmrs;
		parms->fmrs = nfmr;
	}

	if (cnt)
		printk("[%s][%s]FMR is installed.(%d)\n",
			__FUNCTION__, dev->name, cnt);
	else
		printk(KERN_ERR "[%s][%s]Install FMR err.\n",
			__FUNCTION__, dev->name);
}

static void ip6_tnl_netlink_parms(struct nlattr *data[],
				  struct __ip6_tnl_parm *parms,
				  struct net_device *dev)
{
	memset(parms, 0, sizeof(*parms));

	if (!data)
		return;

	if (data[IFLA_IPTUN_LINK])
		parms->link = nla_get_u32(data[IFLA_IPTUN_LINK]);

	if (data[IFLA_IPTUN_LOCAL])
		parms->laddr = nla_get_in6_addr(data[IFLA_IPTUN_LOCAL]);

	if (data[IFLA_IPTUN_REMOTE])
		parms->raddr = nla_get_in6_addr(data[IFLA_IPTUN_REMOTE]);

	if (data[IFLA_IPTUN_TTL])
		parms->hop_limit = nla_get_u8(data[IFLA_IPTUN_TTL]);

	if (data[IFLA_IPTUN_ENCAP_LIMIT])
		parms->encap_limit = nla_get_u8(data[IFLA_IPTUN_ENCAP_LIMIT]);

	if (data[IFLA_IPTUN_FLOWINFO])
		parms->flowinfo = nla_get_be32(data[IFLA_IPTUN_FLOWINFO]);

	if (data[IFLA_IPTUN_FLAGS])
		parms->flags = nla_get_u32(data[IFLA_IPTUN_FLAGS]);

	if (data[IFLA_IPTUN_PROTO])
		parms->proto = nla_get_u8(data[IFLA_IPTUN_PROTO]);

	if (data[IFLA_IPTUN_COLLECT_METADATA])
		parms->collect_md = true;

	if (data[IFLA_IPTUN_FWMARK])
		parms->fwmark = nla_get_u32(data[IFLA_IPTUN_FWMARK]);

	if (data[IFLA_IPTUN_FMRS]) {
		if (!fmr_install(dev, parms)) {
			/* secondary solution for install FMR, but there are
			   only some rules because the data exceeds 1024 bytes. */
			fmr_nla_install(data, dev, parms);
		}
	}
}

static bool ip6_tnl_netlink_encap_parms(struct nlattr *data[],
					struct ip_tunnel_encap *ipencap)
{
	bool ret = false;

	memset(ipencap, 0, sizeof(*ipencap));

	if (!data)
		return ret;

	if (data[IFLA_IPTUN_ENCAP_TYPE]) {
		ret = true;
		ipencap->type = nla_get_u16(data[IFLA_IPTUN_ENCAP_TYPE]);
	}

	if (data[IFLA_IPTUN_ENCAP_FLAGS]) {
		ret = true;
		ipencap->flags = nla_get_u16(data[IFLA_IPTUN_ENCAP_FLAGS]);
	}

	if (data[IFLA_IPTUN_ENCAP_SPORT]) {
		ret = true;
		ipencap->sport = nla_get_be16(data[IFLA_IPTUN_ENCAP_SPORT]);
	}

	if (data[IFLA_IPTUN_ENCAP_DPORT]) {
		ret = true;
		ipencap->dport = nla_get_be16(data[IFLA_IPTUN_ENCAP_DPORT]);
	}

	return ret;
}

static int ip6_tnl_newlink(struct net *src_net, struct net_device *dev,
			   struct nlattr *tb[], struct nlattr *data[],
			   struct netlink_ext_ack *extack)
{
	struct net *net = dev_net(dev);
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);
	struct ip_tunnel_encap ipencap;
	struct ip6_tnl *nt, *t;
	int err;

	nt = netdev_priv(dev);

	if (ip6_tnl_netlink_encap_parms(data, &ipencap)) {
		err = ip6_tnl_encap_setup(nt, &ipencap);
		if (err < 0)
			return err;
	}

	ip6_tnl_netlink_parms(data, &nt->parms, dev);

	if (nt->parms.collect_md) {
		if (rtnl_dereference(ip6n->collect_md_tun))
			return -EEXIST;
	} else {
		t = ip6_tnl_locate(net, &nt->parms, 0);
		if (!IS_ERR(t))
			return -EEXIST;
	}

	err = ip6_tnl_create2(dev);
	if (!err && tb[IFLA_MTU])
		ip6_tnl_change_mtu(dev, nla_get_u32(tb[IFLA_MTU]));

	return err;
}

static int ip6_tnl_changelink(struct net_device *dev, struct nlattr *tb[],
			      struct nlattr *data[],
			      struct netlink_ext_ack *extack)
{
	struct ip6_tnl *t = netdev_priv(dev);
	struct __ip6_tnl_parm p;
	struct net *net = t->net;
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);
	struct ip_tunnel_encap ipencap;

	if (dev == ip6n->fb_tnl_dev)
		return -EINVAL;

	if (ip6_tnl_netlink_encap_parms(data, &ipencap)) {
		int err = ip6_tnl_encap_setup(t, &ipencap);

		if (err < 0)
			return err;
	}
	ip6_tnl_netlink_parms(data, &p, dev);
	if (p.collect_md)
		return -EINVAL;

	t = ip6_tnl_locate(net, &p, 0);
	if (!IS_ERR(t)) {
		if (t->dev != dev)
			return -EEXIST;
	} else
		t = netdev_priv(dev);

	return ip6_tnl_update(t, &p);
}

static void ip6_tnl_dellink(struct net_device *dev, struct list_head *head)
{
	struct net *net = dev_net(dev);
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);

	if (dev != ip6n->fb_tnl_dev)
		unregister_netdevice_queue(dev, head);
}

static size_t ip6_tnl_get_size(const struct net_device *dev)
{
	const struct ip6_tnl *t = netdev_priv(dev);
	struct __ip6_tnl_fmr *c;
	int fmrs = 0;
	for (c = t->parms.fmrs; c; c = c->next)
		++fmrs;

	return
		/* IFLA_IPTUN_LINK */
		nla_total_size(4) +
		/* IFLA_IPTUN_LOCAL */
		nla_total_size(sizeof(struct in6_addr)) +
		/* IFLA_IPTUN_REMOTE */
		nla_total_size(sizeof(struct in6_addr)) +
		/* IFLA_IPTUN_TTL */
		nla_total_size(1) +
		/* IFLA_IPTUN_ENCAP_LIMIT */
		nla_total_size(1) +
		/* IFLA_IPTUN_FLOWINFO */
		nla_total_size(4) +
		/* IFLA_IPTUN_FLAGS */
		nla_total_size(4) +
		/* IFLA_IPTUN_PROTO */
		nla_total_size(1) +
		/* IFLA_IPTUN_ENCAP_TYPE */
		nla_total_size(2) +
		/* IFLA_IPTUN_ENCAP_FLAGS */
		nla_total_size(2) +
		/* IFLA_IPTUN_ENCAP_SPORT */
		nla_total_size(2) +
		/* IFLA_IPTUN_ENCAP_DPORT */
		nla_total_size(2) +
		/* IFLA_IPTUN_COLLECT_METADATA */
		nla_total_size(0) +
		/* IFLA_IPTUN_FWMARK */
		nla_total_size(4) +
		/* IFLA_IPTUN_FMRS */
		nla_total_size(0) +
		(
			/* nest */
			nla_total_size(0) +
			/* IFLA_IPTUN_FMR_IP6_PREFIX */
			nla_total_size(sizeof(struct in6_addr)) +
			/* IFLA_IPTUN_FMR_IP4_PREFIX */
			nla_total_size(sizeof(struct in_addr)) +
			/* IFLA_IPTUN_FMR_EA_LEN */
			nla_total_size(1) +
			/* IFLA_IPTUN_FMR_IP6_PREFIX_LEN */
			nla_total_size(1) +
			/* IFLA_IPTUN_FMR_IP4_PREFIX_LEN */
			nla_total_size(1) +
			/* IFLA_IPTUN_FMR_OFFSET */
			nla_total_size(1)
		) * fmrs +
		0;
}

static int ip6_tnl_fill_info(struct sk_buff *skb, const struct net_device *dev)
{
	struct ip6_tnl *tunnel = netdev_priv(dev);
	struct __ip6_tnl_parm *parm = &tunnel->parms;
	struct __ip6_tnl_fmr *c;
	int fmrcnt = 0;
	struct nlattr *fmrs;

	if (nla_put_u32(skb, IFLA_IPTUN_LINK, parm->link) ||
	    nla_put_in6_addr(skb, IFLA_IPTUN_LOCAL, &parm->laddr) ||
	    nla_put_in6_addr(skb, IFLA_IPTUN_REMOTE, &parm->raddr) ||
	    nla_put_u8(skb, IFLA_IPTUN_TTL, parm->hop_limit) ||
	    nla_put_u8(skb, IFLA_IPTUN_ENCAP_LIMIT, parm->encap_limit) ||
	    nla_put_be32(skb, IFLA_IPTUN_FLOWINFO, parm->flowinfo) ||
	    nla_put_u32(skb, IFLA_IPTUN_FLAGS, parm->flags) ||
	    nla_put_u8(skb, IFLA_IPTUN_PROTO, parm->proto) ||
	    nla_put_u32(skb, IFLA_IPTUN_FWMARK, parm->fwmark) ||
	    !(fmrs = nla_nest_start(skb, IFLA_IPTUN_FMRS)))
		goto nla_put_failure;

	for (c = parm->fmrs; c; c = c->next) {
		struct nlattr *fmr = nla_nest_start(skb, ++fmrcnt);
		if (!fmr ||
			nla_put(skb, IFLA_IPTUN_FMR_IP6_PREFIX,
				sizeof(c->ip6_prefix), &c->ip6_prefix) ||
			nla_put(skb, IFLA_IPTUN_FMR_IP4_PREFIX,
				sizeof(c->ip4_prefix), &c->ip4_prefix) ||
			nla_put_u8(skb, IFLA_IPTUN_FMR_IP6_PREFIX_LEN, c->ip6_prefix_len) ||
			nla_put_u8(skb, IFLA_IPTUN_FMR_IP4_PREFIX_LEN, c->ip4_prefix_len) ||
			nla_put_u8(skb, IFLA_IPTUN_FMR_EA_LEN, c->ea_len) ||
			nla_put_u8(skb, IFLA_IPTUN_FMR_OFFSET, c->offset))
				goto nla_put_failure;

		nla_nest_end(skb, fmr);
	}
	nla_nest_end(skb, fmrs);

	if (nla_put_u16(skb, IFLA_IPTUN_ENCAP_TYPE, tunnel->encap.type) ||
	    nla_put_be16(skb, IFLA_IPTUN_ENCAP_SPORT, tunnel->encap.sport) ||
	    nla_put_be16(skb, IFLA_IPTUN_ENCAP_DPORT, tunnel->encap.dport) ||
	    nla_put_u16(skb, IFLA_IPTUN_ENCAP_FLAGS, tunnel->encap.flags))
		goto nla_put_failure;

	if (parm->collect_md)
		if (nla_put_flag(skb, IFLA_IPTUN_COLLECT_METADATA))
			goto nla_put_failure;

	return 0;

nla_put_failure:
	return -EMSGSIZE;
}

struct net *ip6_tnl_get_link_net(const struct net_device *dev)
{
	struct ip6_tnl *tunnel = netdev_priv(dev);

	return tunnel->net;
}
EXPORT_SYMBOL(ip6_tnl_get_link_net);

static const struct nla_policy ip6_tnl_policy[IFLA_IPTUN_MAX + 1] = {
	[IFLA_IPTUN_LINK]		= { .type = NLA_U32 },
	[IFLA_IPTUN_LOCAL]		= { .len = sizeof(struct in6_addr) },
	[IFLA_IPTUN_REMOTE]		= { .len = sizeof(struct in6_addr) },
	[IFLA_IPTUN_TTL]		= { .type = NLA_U8 },
	[IFLA_IPTUN_ENCAP_LIMIT]	= { .type = NLA_U8 },
	[IFLA_IPTUN_FLOWINFO]		= { .type = NLA_U32 },
	[IFLA_IPTUN_FLAGS]		= { .type = NLA_U32 },
	[IFLA_IPTUN_PROTO]		= { .type = NLA_U8 },
	[IFLA_IPTUN_ENCAP_TYPE]		= { .type = NLA_U16 },
	[IFLA_IPTUN_ENCAP_FLAGS]	= { .type = NLA_U16 },
	[IFLA_IPTUN_ENCAP_SPORT]	= { .type = NLA_U16 },
	[IFLA_IPTUN_ENCAP_DPORT]	= { .type = NLA_U16 },
	[IFLA_IPTUN_COLLECT_METADATA]	= { .type = NLA_FLAG },
	[IFLA_IPTUN_FWMARK]		= { .type = NLA_U32 },
	[IFLA_IPTUN_FMRS]		= { .type = NLA_NESTED },
};

static struct rtnl_link_ops ip6_link_ops __read_mostly = {
	.kind		= "ip6tnl",
	.maxtype	= IFLA_IPTUN_MAX,
	.policy		= ip6_tnl_policy,
	.priv_size	= sizeof(struct ip6_tnl),
	.setup		= ip6_tnl_dev_setup,
	.validate	= ip6_tnl_validate,
	.newlink	= ip6_tnl_newlink,
	.changelink	= ip6_tnl_changelink,
	.dellink	= ip6_tnl_dellink,
	.get_size	= ip6_tnl_get_size,
	.fill_info	= ip6_tnl_fill_info,
	.get_link_net	= ip6_tnl_get_link_net,
};

static struct xfrm6_tunnel ip4ip6_handler __read_mostly = {
	.handler	= ip4ip6_rcv,
	.err_handler	= ip4ip6_err,
	.priority	=	1,
};

static struct xfrm6_tunnel ip6ip6_handler __read_mostly = {
	.handler	= ip6ip6_rcv,
	.err_handler	= ip6ip6_err,
	.priority	=	1,
};

static void __net_exit ip6_tnl_destroy_tunnels(struct net *net, struct list_head *list)
{
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);
	struct net_device *dev, *aux;
	int h;
	struct ip6_tnl *t;

	for_each_netdev_safe(net, dev, aux)
		if (dev->rtnl_link_ops == &ip6_link_ops)
			unregister_netdevice_queue(dev, list);

	for (h = 0; h < IP6_TUNNEL_HASH_SIZE; h++) {
		t = rtnl_dereference(ip6n->tnls_r_l[h]);
		while (t) {
			/* If dev is in the same netns, it has already
			 * been added to the list by the previous loop.
			 */
			if (!net_eq(dev_net(t->dev), net))
				unregister_netdevice_queue(t->dev, list);
			t = rtnl_dereference(t->next);
		}
	}
}

static int __net_init ip6_tnl_init_net(struct net *net)
{
	struct ip6_tnl_net *ip6n = net_generic(net, ip6_tnl_net_id);
	struct ip6_tnl *t = NULL;
	int err;

	ip6n->tnls[0] = ip6n->tnls_wc;
	ip6n->tnls[1] = ip6n->tnls_r_l;

	if (!net_has_fallback_tunnels(net))
		return 0;
	err = -ENOMEM;
	ip6n->fb_tnl_dev = alloc_netdev(sizeof(struct ip6_tnl), "ip6tnl0",
					NET_NAME_UNKNOWN, ip6_tnl_dev_setup);

	if (!ip6n->fb_tnl_dev)
		goto err_alloc_dev;
	dev_net_set(ip6n->fb_tnl_dev, net);
	ip6n->fb_tnl_dev->rtnl_link_ops = &ip6_link_ops;
	/* FB netdevice is special: we have one, and only one per netns.
	 * Allowing to move it to another netns is clearly unsafe.
	 */
	ip6n->fb_tnl_dev->features |= NETIF_F_NETNS_LOCAL;

	err = ip6_fb_tnl_dev_init(ip6n->fb_tnl_dev);
	if (err < 0)
		goto err_register;

	err = register_netdev(ip6n->fb_tnl_dev);
	if (err < 0)
		goto err_register;

	t = netdev_priv(ip6n->fb_tnl_dev);

	strcpy(t->parms.name, ip6n->fb_tnl_dev->name);
	return 0;

err_register:
	free_netdev(ip6n->fb_tnl_dev);
err_alloc_dev:
	return err;
}

static void __net_exit ip6_tnl_exit_batch_net(struct list_head *net_list)
{
	struct net *net;
	LIST_HEAD(list);

	rtnl_lock();
	list_for_each_entry(net, net_list, exit_list)
		ip6_tnl_destroy_tunnels(net, &list);
	unregister_netdevice_many(&list);
	rtnl_unlock();
}

static struct pernet_operations ip6_tnl_net_ops = {
	.init = ip6_tnl_init_net,
	.exit_batch = ip6_tnl_exit_batch_net,
	.id   = &ip6_tnl_net_id,
	.size = sizeof(struct ip6_tnl_net),
};

/**
 * ip6_tunnel_init - register protocol and reserve needed resources
 *
 * Return: 0 on success
 **/

static int __init ip6_tunnel_init(void)
{
	int  err;

	if (!ipv6_mod_enabled())
		return -EOPNOTSUPP;

	err = register_pernet_device(&ip6_tnl_net_ops);
	if (err < 0)
		goto out_pernet;

	err = xfrm6_tunnel_register(&ip4ip6_handler, AF_INET);
	if (err < 0) {
		pr_err("%s: can't register ip4ip6\n", __func__);
		goto out_ip4ip6;
	}

	err = xfrm6_tunnel_register(&ip6ip6_handler, AF_INET6);
	if (err < 0) {
		pr_err("%s: can't register ip6ip6\n", __func__);
		goto out_ip6ip6;
	}
	err = rtnl_link_register(&ip6_link_ops);
	if (err < 0)
		goto rtnl_link_failed;

	return 0;

rtnl_link_failed:
	xfrm6_tunnel_deregister(&ip6ip6_handler, AF_INET6);
out_ip6ip6:
	xfrm6_tunnel_deregister(&ip4ip6_handler, AF_INET);
out_ip4ip6:
	unregister_pernet_device(&ip6_tnl_net_ops);
out_pernet:
	return err;
}

/**
 * ip6_tunnel_cleanup - free resources and unregister protocol
 **/

static void __exit ip6_tunnel_cleanup(void)
{
	rtnl_link_unregister(&ip6_link_ops);
	if (xfrm6_tunnel_deregister(&ip4ip6_handler, AF_INET))
		pr_info("%s: can't deregister ip4ip6\n", __func__);

	if (xfrm6_tunnel_deregister(&ip6ip6_handler, AF_INET6))
		pr_info("%s: can't deregister ip6ip6\n", __func__);

	unregister_pernet_device(&ip6_tnl_net_ops);
}

module_init(ip6_tunnel_init);
module_exit(ip6_tunnel_cleanup);
