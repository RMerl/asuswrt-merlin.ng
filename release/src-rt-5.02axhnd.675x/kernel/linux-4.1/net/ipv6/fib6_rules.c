/*
 * net/ipv6/fib6_rules.c	IPv6 Routing Policy Rules
 *
 * Copyright (C)2003-2006 Helsinki University of Technology
 * Copyright (C)2003-2006 USAGI/WIDE Project
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * Authors
 *	Thomas Graf		<tgraf@suug.ch>
 *	Ville Nuorvala		<vnuorval@tcs.hut.fi>
 */

#include <linux/netdevice.h>
#include <linux/export.h>

#include <net/fib_rules.h>
#include <net/ipv6.h>
#include <net/addrconf.h>
#include <net/ip6_route.h>
#include <net/netlink.h>

struct fib6_rule {
	struct fib_rule		common;
	struct rt6key		src;
	struct rt6key		dst;
	u8			tclass;
};

struct dst_entry *fib6_rule_lookup(struct net *net, struct flowi6 *fl6,
				   int flags, pol_lookup_t lookup)
{
	struct fib_lookup_arg arg = {
		.lookup_ptr = lookup,
		.flags = FIB_LOOKUP_NOREF,
	};

	fib_rules_lookup(net->ipv6.fib6_rules_ops,
			 flowi6_to_flowi(fl6), flags, &arg);

	if (arg.result)
		return arg.result;

	dst_hold(&net->ipv6.ip6_null_entry->dst);
	return &net->ipv6.ip6_null_entry->dst;
}

static int fib6_rule_action(struct fib_rule *rule, struct flowi *flp,
			    int flags, struct fib_lookup_arg *arg)
{
	struct flowi6 *flp6 = &flp->u.ip6;
	struct rt6_info *rt = NULL;
	struct fib6_table *table;
	struct net *net = rule->fr_net;
	pol_lookup_t lookup = arg->lookup_ptr;
	int err = 0;

	switch (rule->action) {
	case FR_ACT_TO_TBL:
		break;
	case FR_ACT_UNREACHABLE:
		err = -ENETUNREACH;
		rt = net->ipv6.ip6_null_entry;
		goto discard_pkt;
	default:
	case FR_ACT_BLACKHOLE:
		err = -EINVAL;
		rt = net->ipv6.ip6_blk_hole_entry;
		goto discard_pkt;
	case FR_ACT_PROHIBIT:
		err = -EACCES;
		rt = net->ipv6.ip6_prohibit_entry;
		goto discard_pkt;
	}

	table = fib6_get_table(net, rule->table);
	if (!table) {
		err = -EAGAIN;
		goto out;
	}

	rt = lookup(net, table, flp6, flags);
	if (rt != net->ipv6.ip6_null_entry) {
		struct fib6_rule *r = (struct fib6_rule *)rule;

		/*
		 * If we need to find a source address for this traffic,
		 * we check the result if it meets requirement of the rule.
		 */
		if ((rule->flags & FIB_RULE_FIND_SADDR) &&
		    r->src.plen && !(flags & RT6_LOOKUP_F_HAS_SADDR)) {
			struct in6_addr saddr;

			if (ipv6_dev_get_saddr(net,
					       ip6_dst_idev(&rt->dst)->dev,
					       &flp6->daddr,
					       rt6_flags2srcprefs(flags),
					       &saddr))
				goto again;
			if (!ipv6_prefix_equal(&saddr, &r->src.addr,
					       r->src.plen))
				goto again;
			flp6->saddr = saddr;
		}
		err = rt->dst.error;
		goto out;
	}
again:
	ip6_rt_put(rt);
	err = -EAGAIN;
	rt = NULL;
	goto out;

discard_pkt:
	dst_hold(&rt->dst);
out:
	arg->result = rt;
	return err;
}

static bool fib6_rule_suppress(struct fib_rule *rule, struct fib_lookup_arg *arg)
{
	struct rt6_info *rt = (struct rt6_info *) arg->result;
	struct net_device *dev = NULL;

	if (rt->rt6i_idev)
		dev = rt->rt6i_idev->dev;

	/* do not accept result if the route does
	 * not meet the required prefix length
	 */
	if (rt->rt6i_dst.plen <= rule->suppress_prefixlen)
		goto suppress_route;

	/* do not accept result if the route uses a device
	 * belonging to a forbidden interface group
	 */
	if (rule->suppress_ifgroup != -1 && dev && dev->group == rule->suppress_ifgroup)
		goto suppress_route;

	return false;

suppress_route:
	ip6_rt_put(rt);
	return true;
}

static int fib6_rule_match(struct fib_rule *rule, struct flowi *fl, int flags)
{
	struct fib6_rule *r = (struct fib6_rule *) rule;
	struct flowi6 *fl6 = &fl->u.ip6;

	if (r->dst.plen &&
	    !ipv6_prefix_equal(&fl6->daddr, &r->dst.addr, r->dst.plen))
		return 0;

	/*
	 * If FIB_RULE_FIND_SADDR is set and we do not have a
	 * source address for the traffic, we defer check for
	 * source address.
	 */
	if (r->src.plen) {
		if (flags & RT6_LOOKUP_F_HAS_SADDR) {
			if (!ipv6_prefix_equal(&fl6->saddr, &r->src.addr,
					       r->src.plen))
				return 0;
		} else if (!(r->common.flags & FIB_RULE_FIND_SADDR))
			return 0;
	}

	if (r->tclass && r->tclass != ip6_tclass(fl6->flowlabel))
		return 0;

	return 1;
}

static const struct nla_policy fib6_rule_policy[FRA_MAX+1] = {
	FRA_GENERIC_POLICY,
};

static int fib6_rule_configure(struct fib_rule *rule, struct sk_buff *skb,
			       struct fib_rule_hdr *frh,
			       struct nlattr **tb)
{
	int err = -EINVAL;
	struct net *net = sock_net(skb->sk);
	struct fib6_rule *rule6 = (struct fib6_rule *) rule;

	if (rule->action == FR_ACT_TO_TBL) {
		if (rule->table == RT6_TABLE_UNSPEC)
			goto errout;

		if (fib6_new_table(net, rule->table) == NULL) {
			err = -ENOBUFS;
			goto errout;
		}
	}

	if (frh->src_len)
		rule6->src.addr = nla_get_in6_addr(tb[FRA_SRC]);

	if (frh->dst_len)
		rule6->dst.addr = nla_get_in6_addr(tb[FRA_DST]);

	rule6->src.plen = frh->src_len;
	rule6->dst.plen = frh->dst_len;
	rule6->tclass = frh->tos;

	err = 0;
errout:
	return err;
}

static int fib6_rule_compare(struct fib_rule *rule, struct fib_rule_hdr *frh,
			     struct nlattr **tb)
{
	struct fib6_rule *rule6 = (struct fib6_rule *) rule;

	if (frh->src_len && (rule6->src.plen != frh->src_len))
		return 0;

	if (frh->dst_len && (rule6->dst.plen != frh->dst_len))
		return 0;

	if (frh->tos && (rule6->tclass != frh->tos))
		return 0;

	if (frh->src_len &&
	    nla_memcmp(tb[FRA_SRC], &rule6->src.addr, sizeof(struct in6_addr)))
		return 0;

	if (frh->dst_len &&
	    nla_memcmp(tb[FRA_DST], &rule6->dst.addr, sizeof(struct in6_addr)))
		return 0;

	return 1;
}

static int fib6_rule_fill(struct fib_rule *rule, struct sk_buff *skb,
			  struct fib_rule_hdr *frh)
{
	struct fib6_rule *rule6 = (struct fib6_rule *) rule;

	frh->dst_len = rule6->dst.plen;
	frh->src_len = rule6->src.plen;
	frh->tos = rule6->tclass;

	if ((rule6->dst.plen &&
	     nla_put_in6_addr(skb, FRA_DST, &rule6->dst.addr)) ||
	    (rule6->src.plen &&
	     nla_put_in6_addr(skb, FRA_SRC, &rule6->src.addr)))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -ENOBUFS;
}

static u32 fib6_rule_default_pref(struct fib_rules_ops *ops)
{
	return 0x3FFF;
}

static size_t fib6_rule_nlmsg_payload(struct fib_rule *rule)
{
	return nla_total_size(16) /* dst */
	       + nla_total_size(16); /* src */
}

static const struct fib_rules_ops __net_initconst fib6_rules_ops_template = {
	.family			= AF_INET6,
	.rule_size		= sizeof(struct fib6_rule),
	.addr_size		= sizeof(struct in6_addr),
	.action			= fib6_rule_action,
	.match			= fib6_rule_match,
	.suppress		= fib6_rule_suppress,
	.configure		= fib6_rule_configure,
	.compare		= fib6_rule_compare,
	.fill			= fib6_rule_fill,
	.default_pref		= fib6_rule_default_pref,
	.nlmsg_payload		= fib6_rule_nlmsg_payload,
	.nlgroup		= RTNLGRP_IPV6_RULE,
	.policy			= fib6_rule_policy,
	.owner			= THIS_MODULE,
	.fro_net		= &init_net,
};

static int __net_init fib6_rules_net_init(struct net *net)
{
	struct fib_rules_ops *ops;
	int err = -ENOMEM;

	ops = fib_rules_register(&fib6_rules_ops_template, net);
	if (IS_ERR(ops))
		return PTR_ERR(ops);

	err = fib_default_rule_add(ops, 0, RT6_TABLE_LOCAL, 0);
	if (err)
		goto out_fib6_rules_ops;

	err = fib_default_rule_add(ops, 0x7FFE, RT6_TABLE_MAIN, 0);
	if (err)
		goto out_fib6_rules_ops;

	net->ipv6.fib6_rules_ops = ops;
out:
	return err;

out_fib6_rules_ops:
	fib_rules_unregister(ops);
	goto out;
}

static void __net_exit fib6_rules_net_exit(struct net *net)
{
	rtnl_lock();
	fib_rules_unregister(net->ipv6.fib6_rules_ops);
	rtnl_unlock();
}

static struct pernet_operations fib6_rules_net_ops = {
	.init = fib6_rules_net_init,
	.exit = fib6_rules_net_exit,
};

int __init fib6_rules_init(void)
{
	return register_pernet_subsys(&fib6_rules_net_ops);
}


void fib6_rules_cleanup(void)
{
	unregister_pernet_subsys(&fib6_rules_net_ops);
}
