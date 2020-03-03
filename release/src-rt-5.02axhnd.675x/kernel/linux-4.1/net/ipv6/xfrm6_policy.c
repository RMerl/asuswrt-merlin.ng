/*
 * xfrm6_policy.c: based on xfrm4_policy.c
 *
 * Authors:
 *	Mitsuru KANDA @USAGI
 *	Kazunori MIYAZAWA @USAGI
 *	Kunihiro Ishiguro <kunihiro@ipinfusion.com>
 *		IPv6 support
 *	YOSHIFUJI Hideaki
 *		Split up af-specific portion
 *
 */

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <net/addrconf.h>
#include <net/dst.h>
#include <net/xfrm.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/ip6_route.h>
#if IS_ENABLED(CONFIG_IPV6_MIP6)
#include <net/mip6.h>
#endif

static struct xfrm_policy_afinfo xfrm6_policy_afinfo;

static struct dst_entry *xfrm6_dst_lookup(struct net *net, int tos,
					  const xfrm_address_t *saddr,
					  const xfrm_address_t *daddr)
{
	struct flowi6 fl6;
	struct dst_entry *dst;
	int err;

	memset(&fl6, 0, sizeof(fl6));
	memcpy(&fl6.daddr, daddr, sizeof(fl6.daddr));
	if (saddr)
		memcpy(&fl6.saddr, saddr, sizeof(fl6.saddr));

	dst = ip6_route_output(net, NULL, &fl6);

	err = dst->error;
	if (dst->error) {
		dst_release(dst);
		dst = ERR_PTR(err);
	}

	return dst;
}

static int xfrm6_get_saddr(struct net *net,
			   xfrm_address_t *saddr, xfrm_address_t *daddr)
{
	struct dst_entry *dst;
	struct net_device *dev;

	dst = xfrm6_dst_lookup(net, 0, NULL, daddr);
	if (IS_ERR(dst))
		return -EHOSTUNREACH;

	dev = ip6_dst_idev(dst)->dev;
	ipv6_dev_get_saddr(dev_net(dev), dev, &daddr->in6, 0, &saddr->in6);
	dst_release(dst);
	return 0;
}

static int xfrm6_get_tos(const struct flowi *fl)
{
	return 0;
}

static void xfrm6_init_dst(struct net *net, struct xfrm_dst *xdst)
{
	struct rt6_info *rt = (struct rt6_info *)xdst;

	rt6_init_peer(rt, net->ipv6.peers);
}

static int xfrm6_init_path(struct xfrm_dst *path, struct dst_entry *dst,
			   int nfheader_len)
{
	if (dst->ops->family == AF_INET6) {
		struct rt6_info *rt = (struct rt6_info *)dst;
		if (rt->rt6i_node)
			path->path_cookie = rt->rt6i_node->fn_sernum;
	}

	path->u.rt6.rt6i_nfheader_len = nfheader_len;

	return 0;
}

static int xfrm6_fill_dst(struct xfrm_dst *xdst, struct net_device *dev,
			  const struct flowi *fl)
{
	struct rt6_info *rt = (struct rt6_info *)xdst->route;

	xdst->u.dst.dev = dev;
	dev_hold(dev);

	xdst->u.rt6.rt6i_idev = in6_dev_get(dev);
	if (!xdst->u.rt6.rt6i_idev) {
		dev_put(dev);
		return -ENODEV;
	}

	rt6_transfer_peer(&xdst->u.rt6, rt);

	/* Sheit... I remember I did this right. Apparently,
	 * it was magically lost, so this code needs audit */
	xdst->u.rt6.rt6i_flags = rt->rt6i_flags & (RTF_ANYCAST |
						   RTF_LOCAL);
	xdst->u.rt6.rt6i_metric = rt->rt6i_metric;
	xdst->u.rt6.rt6i_node = rt->rt6i_node;
	if (rt->rt6i_node)
		xdst->route_cookie = rt->rt6i_node->fn_sernum;
	xdst->u.rt6.rt6i_gateway = rt->rt6i_gateway;
	xdst->u.rt6.rt6i_dst = rt->rt6i_dst;
	xdst->u.rt6.rt6i_src = rt->rt6i_src;

	return 0;
}

static inline void
_decode_session6(struct sk_buff *skb, struct flowi *fl, int reverse)
{
	struct flowi6 *fl6 = &fl->u.ip6;
	int onlyproto = 0;
	const struct ipv6hdr *hdr = ipv6_hdr(skb);
	u16 offset = sizeof(*hdr);
	struct ipv6_opt_hdr *exthdr;
	const unsigned char *nh = skb_network_header(skb);
	u16 nhoff = IP6CB(skb)->nhoff;
	int oif = 0;
	u8 nexthdr;

	if (!nhoff)
		nhoff = offsetof(struct ipv6hdr, nexthdr);

	nexthdr = nh[nhoff];

	if (skb_dst(skb))
		oif = skb_dst(skb)->dev->ifindex;

	memset(fl6, 0, sizeof(struct flowi6));
	fl6->flowi6_mark = skb->mark;
	fl6->flowi6_oif = reverse ? skb->skb_iif : oif;

	fl6->daddr = reverse ? hdr->saddr : hdr->daddr;
	fl6->saddr = reverse ? hdr->daddr : hdr->saddr;

	while (nh + offset + 1 < skb->data ||
	       pskb_may_pull(skb, nh + offset + 1 - skb->data)) {
		nh = skb_network_header(skb);
		exthdr = (struct ipv6_opt_hdr *)(nh + offset);

		switch (nexthdr) {
		case NEXTHDR_FRAGMENT:
			onlyproto = 1;
		case NEXTHDR_ROUTING:
		case NEXTHDR_HOP:
		case NEXTHDR_DEST:
			offset += ipv6_optlen(exthdr);
			nexthdr = exthdr->nexthdr;
			exthdr = (struct ipv6_opt_hdr *)(nh + offset);
			break;

		case IPPROTO_UDP:
		case IPPROTO_UDPLITE:
		case IPPROTO_TCP:
		case IPPROTO_SCTP:
		case IPPROTO_DCCP:
			if (!onlyproto && (nh + offset + 4 < skb->data ||
			     pskb_may_pull(skb, nh + offset + 4 - skb->data))) {
				__be16 *ports;

				nh = skb_network_header(skb);
				ports = (__be16 *)(nh + offset);
				fl6->fl6_sport = ports[!!reverse];
				fl6->fl6_dport = ports[!reverse];
			}
			fl6->flowi6_proto = nexthdr;
			return;

		case IPPROTO_ICMPV6:
			if (!onlyproto && pskb_may_pull(skb, nh + offset + 2 - skb->data)) {
				u8 *icmp;

				nh = skb_network_header(skb);
				icmp = (u8 *)(nh + offset);
				fl6->fl6_icmp_type = icmp[0];
				fl6->fl6_icmp_code = icmp[1];
			}
			fl6->flowi6_proto = nexthdr;
			return;

#if IS_ENABLED(CONFIG_IPV6_MIP6)
		case IPPROTO_MH:
			offset += ipv6_optlen(exthdr);
			if (!onlyproto && pskb_may_pull(skb, nh + offset + 3 - skb->data)) {
				struct ip6_mh *mh;

				nh = skb_network_header(skb);
				mh = (struct ip6_mh *)(nh + offset);
				fl6->fl6_mh_type = mh->ip6mh_type;
			}
			fl6->flowi6_proto = nexthdr;
			return;
#endif

		/* XXX Why are there these headers? */
		case IPPROTO_AH:
		case IPPROTO_ESP:
		case IPPROTO_COMP:
		default:
			fl6->fl6_ipsec_spi = 0;
			fl6->flowi6_proto = nexthdr;
			return;
		}
	}
}

static inline int xfrm6_garbage_collect(struct dst_ops *ops)
{
	struct net *net = container_of(ops, struct net, xfrm.xfrm6_dst_ops);

	xfrm6_policy_afinfo.garbage_collect(net);
	return dst_entries_get_fast(ops) > ops->gc_thresh * 2;
}

static void xfrm6_update_pmtu(struct dst_entry *dst, struct sock *sk,
			      struct sk_buff *skb, u32 mtu)
{
	struct xfrm_dst *xdst = (struct xfrm_dst *)dst;
	struct dst_entry *path = xdst->route;

	path->ops->update_pmtu(path, sk, skb, mtu);
}

static void xfrm6_redirect(struct dst_entry *dst, struct sock *sk,
			   struct sk_buff *skb)
{
	struct xfrm_dst *xdst = (struct xfrm_dst *)dst;
	struct dst_entry *path = xdst->route;

	path->ops->redirect(path, sk, skb);
}

static void xfrm6_dst_destroy(struct dst_entry *dst)
{
	struct xfrm_dst *xdst = (struct xfrm_dst *)dst;

	if (likely(xdst->u.rt6.rt6i_idev))
		in6_dev_put(xdst->u.rt6.rt6i_idev);
	dst_destroy_metrics_generic(dst);
	if (rt6_has_peer(&xdst->u.rt6)) {
		struct inet_peer *peer = rt6_peer_ptr(&xdst->u.rt6);
		inet_putpeer(peer);
	}
	xfrm_dst_destroy(xdst);
}

static void xfrm6_dst_ifdown(struct dst_entry *dst, struct net_device *dev,
			     int unregister)
{
	struct xfrm_dst *xdst;

	if (!unregister)
		return;

	xdst = (struct xfrm_dst *)dst;
	if (xdst->u.rt6.rt6i_idev->dev == dev) {
		struct inet6_dev *loopback_idev =
			in6_dev_get(dev_net(dev)->loopback_dev);
		BUG_ON(!loopback_idev);

		do {
			in6_dev_put(xdst->u.rt6.rt6i_idev);
			xdst->u.rt6.rt6i_idev = loopback_idev;
			in6_dev_hold(loopback_idev);
			xdst = (struct xfrm_dst *)xdst->u.dst.child;
		} while (xdst->u.dst.xfrm);

		__in6_dev_put(loopback_idev);
	}

	xfrm_dst_ifdown(dst, dev);
}

static struct dst_ops xfrm6_dst_ops_template = {
	.family =		AF_INET6,
	.gc =			xfrm6_garbage_collect,
	.update_pmtu =		xfrm6_update_pmtu,
	.redirect =		xfrm6_redirect,
	.cow_metrics =		dst_cow_metrics_generic,
	.destroy =		xfrm6_dst_destroy,
	.ifdown =		xfrm6_dst_ifdown,
	.local_out =		__ip6_local_out,
	.gc_thresh =		32768,
};

static struct xfrm_policy_afinfo xfrm6_policy_afinfo = {
	.family =		AF_INET6,
	.dst_ops =		&xfrm6_dst_ops_template,
	.dst_lookup =		xfrm6_dst_lookup,
	.get_saddr =		xfrm6_get_saddr,
	.decode_session =	_decode_session6,
	.get_tos =		xfrm6_get_tos,
	.init_dst =		xfrm6_init_dst,
	.init_path =		xfrm6_init_path,
	.fill_dst =		xfrm6_fill_dst,
	.blackhole_route =	ip6_blackhole_route,
};

static int __init xfrm6_policy_init(void)
{
	return xfrm_policy_register_afinfo(&xfrm6_policy_afinfo);
}

static void xfrm6_policy_fini(void)
{
	xfrm_policy_unregister_afinfo(&xfrm6_policy_afinfo);
}

#ifdef CONFIG_SYSCTL
static struct ctl_table xfrm6_policy_table[] = {
	{
		.procname       = "xfrm6_gc_thresh",
		.data		= &init_net.xfrm.xfrm6_dst_ops.gc_thresh,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler   = proc_dointvec,
	},
	{ }
};

static int __net_init xfrm6_net_sysctl_init(struct net *net)
{
	struct ctl_table *table;
	struct ctl_table_header *hdr;

	table = xfrm6_policy_table;
	if (!net_eq(net, &init_net)) {
		table = kmemdup(table, sizeof(xfrm6_policy_table), GFP_KERNEL);
		if (!table)
			goto err_alloc;

		table[0].data = &net->xfrm.xfrm6_dst_ops.gc_thresh;
	}

	hdr = register_net_sysctl(net, "net/ipv6", table);
	if (!hdr)
		goto err_reg;

	net->ipv6.sysctl.xfrm6_hdr = hdr;
	return 0;

err_reg:
	if (!net_eq(net, &init_net))
		kfree(table);
err_alloc:
	return -ENOMEM;
}

static void __net_exit xfrm6_net_sysctl_exit(struct net *net)
{
	struct ctl_table *table;

	if (!net->ipv6.sysctl.xfrm6_hdr)
		return;

	table = net->ipv6.sysctl.xfrm6_hdr->ctl_table_arg;
	unregister_net_sysctl_table(net->ipv6.sysctl.xfrm6_hdr);
	if (!net_eq(net, &init_net))
		kfree(table);
}
#else /* CONFIG_SYSCTL */
static int inline xfrm6_net_sysctl_init(struct net *net)
{
	return 0;
}

static void inline xfrm6_net_sysctl_exit(struct net *net)
{
}
#endif

static int __net_init xfrm6_net_init(struct net *net)
{
	int ret;

	memcpy(&net->xfrm.xfrm6_dst_ops, &xfrm6_dst_ops_template,
	       sizeof(xfrm6_dst_ops_template));
	ret = dst_entries_init(&net->xfrm.xfrm6_dst_ops);
	if (ret)
		return ret;

	ret = xfrm6_net_sysctl_init(net);
	if (ret)
		dst_entries_destroy(&net->xfrm.xfrm6_dst_ops);

	return ret;
}

static void __net_exit xfrm6_net_exit(struct net *net)
{
	xfrm6_net_sysctl_exit(net);
	dst_entries_destroy(&net->xfrm.xfrm6_dst_ops);
}

static struct pernet_operations xfrm6_net_ops = {
	.init	= xfrm6_net_init,
	.exit	= xfrm6_net_exit,
};

int __init xfrm6_init(void)
{
	int ret;

	ret = xfrm6_policy_init();
	if (ret)
		goto out;
	ret = xfrm6_state_init();
	if (ret)
		goto out_policy;

	ret = xfrm6_protocol_init();
	if (ret)
		goto out_state;

	register_pernet_subsys(&xfrm6_net_ops);
out:
	return ret;
out_state:
	xfrm6_state_fini();
out_policy:
	xfrm6_policy_fini();
	goto out;
}

void xfrm6_fini(void)
{
	unregister_pernet_subsys(&xfrm6_net_ops);
	xfrm6_protocol_fini();
	xfrm6_policy_fini();
	xfrm6_state_fini();
}
