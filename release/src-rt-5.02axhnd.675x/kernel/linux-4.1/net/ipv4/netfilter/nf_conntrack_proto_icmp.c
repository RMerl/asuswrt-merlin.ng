/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 * (C) 2006-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <linux/in.h>
#include <linux/icmp.h>
#include <linux/seq_file.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_log.h>

static unsigned int nf_ct_icmp_timeout __read_mostly = 30*HZ;

static inline struct nf_icmp_net *icmp_pernet(struct net *net)
{
	return &net->ct.nf_ct_proto.icmp;
}

static bool icmp_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
			      struct nf_conntrack_tuple *tuple)
{
	const struct icmphdr *hp;
	struct icmphdr _hdr;

	hp = skb_header_pointer(skb, dataoff, sizeof(_hdr), &_hdr);
	if (hp == NULL)
		return false;

	tuple->dst.u.icmp.type = hp->type;
	tuple->src.u.icmp.id = hp->un.echo.id;
	tuple->dst.u.icmp.code = hp->code;

	return true;
}

/* Add 1; spaces filled with 0. */
static const u_int8_t invmap[] = {
	[ICMP_ECHO] = ICMP_ECHOREPLY + 1,
	[ICMP_ECHOREPLY] = ICMP_ECHO + 1,
	[ICMP_TIMESTAMP] = ICMP_TIMESTAMPREPLY + 1,
	[ICMP_TIMESTAMPREPLY] = ICMP_TIMESTAMP + 1,
	[ICMP_INFO_REQUEST] = ICMP_INFO_REPLY + 1,
	[ICMP_INFO_REPLY] = ICMP_INFO_REQUEST + 1,
	[ICMP_ADDRESS] = ICMP_ADDRESSREPLY + 1,
	[ICMP_ADDRESSREPLY] = ICMP_ADDRESS + 1
};

static bool icmp_invert_tuple(struct nf_conntrack_tuple *tuple,
			      const struct nf_conntrack_tuple *orig)
{
	if (orig->dst.u.icmp.type >= sizeof(invmap) ||
	    !invmap[orig->dst.u.icmp.type])
		return false;

	tuple->src.u.icmp.id = orig->src.u.icmp.id;
	tuple->dst.u.icmp.type = invmap[orig->dst.u.icmp.type] - 1;
	tuple->dst.u.icmp.code = orig->dst.u.icmp.code;
	return true;
}

/* Print out the per-protocol part of the tuple. */
static void icmp_print_tuple(struct seq_file *s,
			    const struct nf_conntrack_tuple *tuple)
{
	seq_printf(s, "type=%u code=%u id=%u ",
		   tuple->dst.u.icmp.type,
		   tuple->dst.u.icmp.code,
		   ntohs(tuple->src.u.icmp.id));
}

static unsigned int *icmp_get_timeouts(struct net *net)
{
	return &icmp_pernet(net)->timeout;
}

/* Returns verdict for packet, or -1 for invalid. */
static int icmp_packet(struct nf_conn *ct,
		       const struct sk_buff *skb,
		       unsigned int dataoff,
		       enum ip_conntrack_info ctinfo,
		       u_int8_t pf,
		       unsigned int hooknum,
		       unsigned int *timeout)
{
	/* Do not immediately delete the connection after the first
	   successful reply to avoid excessive conntrackd traffic
	   and also to handle correctly ICMP echo reply duplicates. */
	nf_ct_refresh_acct(ct, ctinfo, skb, *timeout);

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static bool icmp_new(struct nf_conn *ct, const struct sk_buff *skb,
		     unsigned int dataoff, unsigned int *timeouts)
{
	static const u_int8_t valid_new[] = {
		[ICMP_ECHO] = 1,
		[ICMP_TIMESTAMP] = 1,
		[ICMP_INFO_REQUEST] = 1,
		[ICMP_ADDRESS] = 1
	};

	if (ct->tuplehash[0].tuple.dst.u.icmp.type >= sizeof(valid_new) ||
	    !valid_new[ct->tuplehash[0].tuple.dst.u.icmp.type]) {
		/* Can't create a new ICMP `conn' with this. */
		pr_debug("icmp: can't create new conn with type %u\n",
			 ct->tuplehash[0].tuple.dst.u.icmp.type);
		nf_ct_dump_tuple_ip(&ct->tuplehash[0].tuple);
		return false;
	}
	return true;
}

/* Returns conntrack if it dealt with ICMP, and filled in skb fields */
static int
icmp_error_message(struct net *net, struct nf_conn *tmpl, struct sk_buff *skb,
		 enum ip_conntrack_info *ctinfo,
		 unsigned int hooknum)
{
#if defined(CONFIG_BCM_KF_NETFILTER)
	struct inside {
		struct icmphdr icmp;
		struct iphdr ip;
	} __attribute__((packed));
	struct inside _in, *pIn; 
#endif
	struct nf_conntrack_tuple innertuple, origtuple;
	const struct nf_conntrack_l4proto *innerproto;
	const struct nf_conntrack_tuple_hash *h;
	u16 zone = tmpl ? nf_ct_zone(tmpl) : NF_CT_DEFAULT_ZONE;

	NF_CT_ASSERT(skb->nfct == NULL);

#if defined(CONFIG_BCM_KF_NETFILTER)
	/* Not enough header? */
	pIn = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_in), &_in);
	if (pIn == NULL)
		return -NF_ACCEPT;
#endif

	/* Are they talking about one of our connections? */
	if (!nf_ct_get_tuplepr(skb,
			       skb_network_offset(skb) + ip_hdrlen(skb)
						       + sizeof(struct icmphdr),
			       PF_INET, &origtuple)) {
		pr_debug("icmp_error_message: failed to get tuple\n");
		return -NF_ACCEPT;
	}

	/* rcu_read_lock()ed by nf_hook_slow */
#if defined(CONFIG_BCM_KF_NETFILTER)
	innerproto = __nf_ct_l4proto_find(PF_INET, pIn->ip.protocol);
	origtuple.src.u3.ip = pIn->ip.saddr;
	origtuple.dst.u3.ip = pIn->ip.daddr;
#else
	innerproto = __nf_ct_l4proto_find(PF_INET, origtuple.dst.protonum);
#endif

	/* Ordinarily, we'd expect the inverted tupleproto, but it's
	   been preserved inside the ICMP. */
	if (!nf_ct_invert_tuple(&innertuple, &origtuple,
				&nf_conntrack_l3proto_ipv4, innerproto)) {
		pr_debug("icmp_error_message: no match\n");
		return -NF_ACCEPT;
	}

	*ctinfo = IP_CT_RELATED;

	h = nf_conntrack_find_get(net, zone, &innertuple);
	if (!h) {
		pr_debug("icmp_error_message: no match\n");
		return -NF_ACCEPT;
	}

	if (NF_CT_DIRECTION(h) == IP_CT_DIR_REPLY)
		*ctinfo += IP_CT_IS_REPLY;

	/* Update skb to refer to this connection */
	skb->nfct = &nf_ct_tuplehash_to_ctrack(h)->ct_general;
	skb->nfctinfo = *ctinfo;
	return NF_ACCEPT;
}

/* Small and modified version of icmp_rcv */
static int
icmp_error(struct net *net, struct nf_conn *tmpl,
	   struct sk_buff *skb, unsigned int dataoff,
	   enum ip_conntrack_info *ctinfo, u_int8_t pf, unsigned int hooknum)
{
	const struct icmphdr *icmph;
	struct icmphdr _ih;

	/* Not enough header? */
	icmph = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_ih), &_ih);
	if (icmph == NULL) {
		if (LOG_INVALID(net, IPPROTO_ICMP))
			nf_log_packet(net, PF_INET, 0, skb, NULL, NULL,
				      NULL, "nf_ct_icmp: short packet ");
		return -NF_ACCEPT;
	}

	/* See ip_conntrack_proto_tcp.c */
	if (net->ct.sysctl_checksum && hooknum == NF_INET_PRE_ROUTING &&
	    nf_ip_checksum(skb, hooknum, dataoff, 0)) {
		if (LOG_INVALID(net, IPPROTO_ICMP))
			nf_log_packet(net, PF_INET, 0, skb, NULL, NULL, NULL,
				      "nf_ct_icmp: bad HW ICMP checksum ");
		return -NF_ACCEPT;
	}

	/*
	 *	18 is the highest 'known' ICMP type. Anything else is a mystery
	 *
	 *	RFC 1122: 3.2.2  Unknown ICMP messages types MUST be silently
	 *		  discarded.
	 */
	if (icmph->type > NR_ICMP_TYPES) {
		if (LOG_INVALID(net, IPPROTO_ICMP))
			nf_log_packet(net, PF_INET, 0, skb, NULL, NULL, NULL,
				      "nf_ct_icmp: invalid ICMP type ");
		return -NF_ACCEPT;
	}

	/* Need to track icmp error message? */
	if (icmph->type != ICMP_DEST_UNREACH &&
	    icmph->type != ICMP_SOURCE_QUENCH &&
	    icmph->type != ICMP_TIME_EXCEEDED &&
	    icmph->type != ICMP_PARAMETERPROB &&
	    icmph->type != ICMP_REDIRECT)
		return NF_ACCEPT;

	return icmp_error_message(net, tmpl, skb, ctinfo, hooknum);
}

#if IS_ENABLED(CONFIG_NF_CT_NETLINK)

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

static int icmp_tuple_to_nlattr(struct sk_buff *skb,
				const struct nf_conntrack_tuple *t)
{
	if (nla_put_be16(skb, CTA_PROTO_ICMP_ID, t->src.u.icmp.id) ||
	    nla_put_u8(skb, CTA_PROTO_ICMP_TYPE, t->dst.u.icmp.type) ||
	    nla_put_u8(skb, CTA_PROTO_ICMP_CODE, t->dst.u.icmp.code))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -1;
}

static const struct nla_policy icmp_nla_policy[CTA_PROTO_MAX+1] = {
	[CTA_PROTO_ICMP_TYPE]	= { .type = NLA_U8 },
	[CTA_PROTO_ICMP_CODE]	= { .type = NLA_U8 },
	[CTA_PROTO_ICMP_ID]	= { .type = NLA_U16 },
};

static int icmp_nlattr_to_tuple(struct nlattr *tb[],
				struct nf_conntrack_tuple *tuple)
{
	if (!tb[CTA_PROTO_ICMP_TYPE] ||
	    !tb[CTA_PROTO_ICMP_CODE] ||
	    !tb[CTA_PROTO_ICMP_ID])
		return -EINVAL;

	tuple->dst.u.icmp.type = nla_get_u8(tb[CTA_PROTO_ICMP_TYPE]);
	tuple->dst.u.icmp.code = nla_get_u8(tb[CTA_PROTO_ICMP_CODE]);
	tuple->src.u.icmp.id = nla_get_be16(tb[CTA_PROTO_ICMP_ID]);

	if (tuple->dst.u.icmp.type >= sizeof(invmap) ||
	    !invmap[tuple->dst.u.icmp.type])
		return -EINVAL;

	return 0;
}

static int icmp_nlattr_tuple_size(void)
{
	return nla_policy_len(icmp_nla_policy, CTA_PROTO_MAX + 1);
}
#endif

#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_cttimeout.h>

static int icmp_timeout_nlattr_to_obj(struct nlattr *tb[],
				      struct net *net, void *data)
{
	unsigned int *timeout = data;
	struct nf_icmp_net *in = icmp_pernet(net);

	if (tb[CTA_TIMEOUT_ICMP_TIMEOUT]) {
		*timeout =
			ntohl(nla_get_be32(tb[CTA_TIMEOUT_ICMP_TIMEOUT])) * HZ;
	} else {
		/* Set default ICMP timeout. */
		*timeout = in->timeout;
	}
	return 0;
}

static int
icmp_timeout_obj_to_nlattr(struct sk_buff *skb, const void *data)
{
	const unsigned int *timeout = data;

	if (nla_put_be32(skb, CTA_TIMEOUT_ICMP_TIMEOUT, htonl(*timeout / HZ)))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -ENOSPC;
}

static const struct nla_policy
icmp_timeout_nla_policy[CTA_TIMEOUT_ICMP_MAX+1] = {
	[CTA_TIMEOUT_ICMP_TIMEOUT]	= { .type = NLA_U32 },
};
#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */

#ifdef CONFIG_SYSCTL
static struct ctl_table icmp_sysctl_table[] = {
	{
		.procname	= "nf_conntrack_icmp_timeout",
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{ }
};
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
static struct ctl_table icmp_compat_sysctl_table[] = {
	{
		.procname	= "ip_conntrack_icmp_timeout",
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{ }
};
#endif /* CONFIG_NF_CONNTRACK_PROC_COMPAT */
#endif /* CONFIG_SYSCTL */

static int icmp_kmemdup_sysctl_table(struct nf_proto_net *pn,
				     struct nf_icmp_net *in)
{
#ifdef CONFIG_SYSCTL
	pn->ctl_table = kmemdup(icmp_sysctl_table,
				sizeof(icmp_sysctl_table),
				GFP_KERNEL);
	if (!pn->ctl_table)
		return -ENOMEM;

	pn->ctl_table[0].data = &in->timeout;
#endif
	return 0;
}

static int icmp_kmemdup_compat_sysctl_table(struct nf_proto_net *pn,
					    struct nf_icmp_net *in)
{
#ifdef CONFIG_SYSCTL
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
	pn->ctl_compat_table = kmemdup(icmp_compat_sysctl_table,
				       sizeof(icmp_compat_sysctl_table),
				       GFP_KERNEL);
	if (!pn->ctl_compat_table)
		return -ENOMEM;

	pn->ctl_compat_table[0].data = &in->timeout;
#endif
#endif
	return 0;
}

static int icmp_init_net(struct net *net, u_int16_t proto)
{
	int ret;
	struct nf_icmp_net *in = icmp_pernet(net);
	struct nf_proto_net *pn = &in->pn;

	in->timeout = nf_ct_icmp_timeout;

	ret = icmp_kmemdup_compat_sysctl_table(pn, in);
	if (ret < 0)
		return ret;

	ret = icmp_kmemdup_sysctl_table(pn, in);
	if (ret < 0)
		nf_ct_kfree_compat_sysctl_table(pn);

	return ret;
}

static struct nf_proto_net *icmp_get_net_proto(struct net *net)
{
	return &net->ct.nf_ct_proto.icmp.pn;
}

struct nf_conntrack_l4proto nf_conntrack_l4proto_icmp __read_mostly =
{
	.l3proto		= PF_INET,
	.l4proto		= IPPROTO_ICMP,
	.name			= "icmp",
	.pkt_to_tuple		= icmp_pkt_to_tuple,
	.invert_tuple		= icmp_invert_tuple,
	.print_tuple		= icmp_print_tuple,
	.packet			= icmp_packet,
	.get_timeouts		= icmp_get_timeouts,
	.new			= icmp_new,
	.error			= icmp_error,
	.destroy		= NULL,
	.me			= NULL,
#if IS_ENABLED(CONFIG_NF_CT_NETLINK)
	.tuple_to_nlattr	= icmp_tuple_to_nlattr,
	.nlattr_tuple_size	= icmp_nlattr_tuple_size,
	.nlattr_to_tuple	= icmp_nlattr_to_tuple,
	.nla_policy		= icmp_nla_policy,
#endif
#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)
	.ctnl_timeout		= {
		.nlattr_to_obj	= icmp_timeout_nlattr_to_obj,
		.obj_to_nlattr	= icmp_timeout_obj_to_nlattr,
		.nlattr_max	= CTA_TIMEOUT_ICMP_MAX,
		.obj_size	= sizeof(unsigned int),
		.nla_policy	= icmp_timeout_nla_policy,
	},
#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */
	.init_net		= icmp_init_net,
	.get_net_proto		= icmp_get_net_proto,
};
