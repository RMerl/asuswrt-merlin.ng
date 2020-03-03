/*
 * IPv6 specific functions of netfilter core
 *
 * Rusty Russell (C) 2000 -- This code is GPL.
 * Patrick McHardy (C) 2006-2012
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ipv6.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv6.h>
#include <linux/export.h>
#include <net/addrconf.h>
#include <net/dst.h>
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <net/xfrm.h>
#include <net/ip6_checksum.h>
#include <net/netfilter/nf_queue.h>

int ip6_route_me_harder(struct sk_buff *skb)
{
	struct net *net = dev_net(skb_dst(skb)->dev);
	const struct ipv6hdr *iph = ipv6_hdr(skb);
	unsigned int hh_len;
	struct dst_entry *dst;
	struct flowi6 fl6 = {
		.flowi6_oif = skb->sk ? skb->sk->sk_bound_dev_if : 0,
		.flowi6_mark = skb->mark,
		.daddr = iph->daddr,
		.saddr = iph->saddr,
	};
	int err;

#if defined(CONFIG_BCM_KF_NETFILTER)
	/* keep original output dev if destination is multicast/linklocal */
	if  (    ( fl6.flowi6_oif == 0 ) && ((skb_dst(skb)->dev))
	      && ( __ipv6_addr_type(&fl6.daddr) & (IPV6_ADDR_MULTICAST | IPV6_ADDR_LINKLOCAL))
	    )
	{
		fl6.flowi6_oif = (skb_dst(skb)->dev)->ifindex;
	}
#endif

	dst = ip6_route_output(net, skb->sk, &fl6);
	err = dst->error;
	if (err) {
		IP6_INC_STATS(net, ip6_dst_idev(dst), IPSTATS_MIB_OUTNOROUTES);
		net_dbg_ratelimited("ip6_route_me_harder: No more route\n");
		dst_release(dst);
		return err;
	}

	/* Drop old route. */
	skb_dst_drop(skb);

	skb_dst_set(skb, dst);

#ifdef CONFIG_XFRM
	if (!(IP6CB(skb)->flags & IP6SKB_XFRM_TRANSFORMED) &&
	    xfrm_decode_session(skb, flowi6_to_flowi(&fl6), AF_INET6) == 0) {
		skb_dst_set(skb, NULL);
		dst = xfrm_lookup(net, dst, flowi6_to_flowi(&fl6), skb->sk, 0);
		if (IS_ERR(dst))
			return PTR_ERR(dst);
		skb_dst_set(skb, dst);
	}
#endif

	/* Change in oif may mean change in hh_len. */
	hh_len = skb_dst(skb)->dev->hard_header_len;
	if (skb_headroom(skb) < hh_len &&
	    pskb_expand_head(skb, HH_DATA_ALIGN(hh_len - skb_headroom(skb)),
			     0, GFP_ATOMIC))
		return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL(ip6_route_me_harder);

/*
 * Extra routing may needed on local out, as the QUEUE target never
 * returns control to the table.
 */

struct ip6_rt_info {
	struct in6_addr daddr;
	struct in6_addr saddr;
	u_int32_t mark;
};

static void nf_ip6_saveroute(const struct sk_buff *skb,
			     struct nf_queue_entry *entry)
{
	struct ip6_rt_info *rt_info = nf_queue_entry_reroute(entry);

	if (entry->state.hook == NF_INET_LOCAL_OUT) {
		const struct ipv6hdr *iph = ipv6_hdr(skb);

		rt_info->daddr = iph->daddr;
		rt_info->saddr = iph->saddr;
		rt_info->mark = skb->mark;
	}
}

static int nf_ip6_reroute(struct sk_buff *skb,
			  const struct nf_queue_entry *entry)
{
	struct ip6_rt_info *rt_info = nf_queue_entry_reroute(entry);

	if (entry->state.hook == NF_INET_LOCAL_OUT) {
		const struct ipv6hdr *iph = ipv6_hdr(skb);
		if (!ipv6_addr_equal(&iph->daddr, &rt_info->daddr) ||
		    !ipv6_addr_equal(&iph->saddr, &rt_info->saddr) ||
		    skb->mark != rt_info->mark)
			return ip6_route_me_harder(skb);
	}
	return 0;
}

static int nf_ip6_route(struct net *net, struct dst_entry **dst,
			struct flowi *fl, bool strict)
{
	static const struct ipv6_pinfo fake_pinfo;
	static const struct inet_sock fake_sk = {
		/* makes ip6_route_output set RT6_LOOKUP_F_IFACE: */
		.sk.sk_bound_dev_if = 1,
		.pinet6 = (struct ipv6_pinfo *) &fake_pinfo,
	};
	const void *sk = strict ? &fake_sk : NULL;
	struct dst_entry *result;
	int err;

	result = ip6_route_output(net, sk, &fl->u.ip6);
	err = result->error;
	if (err)
		dst_release(result);
	else
		*dst = result;
	return err;
}

__sum16 nf_ip6_checksum(struct sk_buff *skb, unsigned int hook,
			     unsigned int dataoff, u_int8_t protocol)
{
	const struct ipv6hdr *ip6h = ipv6_hdr(skb);
	__sum16 csum = 0;
#if defined(CONFIG_MIPS_BCM963XX) && defined(CONFIG_BCM_KF_UNALIGNED_EXCEPTION)
	struct in6_addr sAddr;
	struct in6_addr dAddr;

	memcpy(&sAddr, &ip6h->saddr, sizeof(struct in6_addr));
	memcpy(&dAddr, &ip6h->daddr, sizeof(struct in6_addr));
#endif

	switch (skb->ip_summed) {
	case CHECKSUM_COMPLETE:
		if (hook != NF_INET_PRE_ROUTING && hook != NF_INET_LOCAL_IN)
			break;
#if defined(CONFIG_MIPS_BCM963XX) && defined(CONFIG_BCM_KF_UNALIGNED_EXCEPTION)
		if (!csum_ipv6_magic(&sAddr, &dAddr,
				     skb->len - dataoff, protocol,
				     csum_sub(skb->csum,
					      skb_checksum(skb, 0,
							   dataoff, 0)))) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			break;
		}
#else
		if (!csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
				     skb->len - dataoff, protocol,
				     csum_sub(skb->csum,
					      skb_checksum(skb, 0,
							   dataoff, 0)))) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			break;
		}
#endif
		/* fall through */
	case CHECKSUM_NONE:
#if defined(CONFIG_MIPS_BCM963XX) && defined(CONFIG_BCM_KF_UNALIGNED_EXCEPTION)
		skb->csum = ~csum_unfold(
				csum_ipv6_magic(&sAddr, &dAddr,
					     skb->len - dataoff,
					     protocol,
					     csum_sub(0,
						      skb_checksum(skb, 0,
								   dataoff, 0))));
#else
		skb->csum = ~csum_unfold(
				csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
					     skb->len - dataoff,
					     protocol,
					     csum_sub(0,
						      skb_checksum(skb, 0,
								   dataoff, 0))));
#endif
		csum = __skb_checksum_complete(skb);
	}
	return csum;
}
EXPORT_SYMBOL(nf_ip6_checksum);

static __sum16 nf_ip6_checksum_partial(struct sk_buff *skb, unsigned int hook,
				       unsigned int dataoff, unsigned int len,
				       u_int8_t protocol)
{
	const struct ipv6hdr *ip6h = ipv6_hdr(skb);
	__wsum hsum;
	__sum16 csum = 0;
#if defined(CONFIG_MIPS_BCM963XX) && defined(CONFIG_BCM_KF_UNALIGNED_EXCEPTION)
	struct in6_addr sAddr;
	struct in6_addr dAddr;

	memcpy(&sAddr, &ip6h->saddr, sizeof(struct in6_addr));
	memcpy(&dAddr, &ip6h->daddr, sizeof(struct in6_addr));
#endif

	switch (skb->ip_summed) {
	case CHECKSUM_COMPLETE:
		if (len == skb->len - dataoff)
			return nf_ip6_checksum(skb, hook, dataoff, protocol);
		/* fall through */
	case CHECKSUM_NONE:
		hsum = skb_checksum(skb, 0, dataoff, 0);
#if defined(CONFIG_MIPS_BCM963XX) && defined(CONFIG_BCM_KF_UNALIGNED_EXCEPTION)
		skb->csum = ~csum_unfold(csum_ipv6_magic(&sAddr,
							 &dAddr,
							 skb->len - dataoff,
							 protocol,
							 csum_sub(0, hsum)));
#else
		skb->csum = ~csum_unfold(csum_ipv6_magic(&ip6h->saddr,
							 &ip6h->daddr,
							 skb->len - dataoff,
							 protocol,
							 csum_sub(0, hsum)));
#endif
		skb->ip_summed = CHECKSUM_NONE;
		return __skb_checksum_complete_head(skb, dataoff + len);
	}
	return csum;
};

static const struct nf_ipv6_ops ipv6ops = {
	.chk_addr	= ipv6_chk_addr,
};

static const struct nf_afinfo nf_ip6_afinfo = {
	.family			= AF_INET6,
	.checksum		= nf_ip6_checksum,
	.checksum_partial	= nf_ip6_checksum_partial,
	.route			= nf_ip6_route,
	.saveroute		= nf_ip6_saveroute,
	.reroute		= nf_ip6_reroute,
	.route_key_size		= sizeof(struct ip6_rt_info),
};

int __init ipv6_netfilter_init(void)
{
	RCU_INIT_POINTER(nf_ipv6_ops, &ipv6ops);
	return nf_register_afinfo(&nf_ip6_afinfo);
}

/* This can be called from inet6_init() on errors, so it cannot
 * be marked __exit. -DaveM
 */
void ipv6_netfilter_fini(void)
{
	RCU_INIT_POINTER(nf_ipv6_ops, NULL);
	nf_unregister_afinfo(&nf_ip6_afinfo);
}
