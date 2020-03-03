/*
 * net/sched/act_connmark.c  netfilter connmark retriever action
 * skb mark is over-written
 *
 * Copyright (c) 2011 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/pkt_cls.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/netlink.h>
#include <net/pkt_sched.h>
#include <net/act_api.h>
#include <uapi/linux/tc_act/tc_connmark.h>
#include <net/tc_act/tc_connmark.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_zones.h>

#define CONNMARK_TAB_MASK     3

static int tcf_connmark(struct sk_buff *skb, const struct tc_action *a,
			struct tcf_result *res)
{
	const struct nf_conntrack_tuple_hash *thash;
	struct nf_conntrack_tuple tuple;
	enum ip_conntrack_info ctinfo;
	struct tcf_connmark_info *ca = a->priv;
	struct nf_conn *c;
	int proto;

	spin_lock(&ca->tcf_lock);
	ca->tcf_tm.lastuse = jiffies;
	bstats_update(&ca->tcf_bstats, skb);

	if (skb->protocol == htons(ETH_P_IP)) {
		if (skb->len < sizeof(struct iphdr))
			goto out;

		proto = NFPROTO_IPV4;
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		if (skb->len < sizeof(struct ipv6hdr))
			goto out;

		proto = NFPROTO_IPV6;
	} else {
		goto out;
	}

	c = nf_ct_get(skb, &ctinfo);
	if (c) {
		skb->mark = c->mark;
		/* using overlimits stats to count how many packets marked */
		ca->tcf_qstats.overlimits++;
		goto out;
	}

	if (!nf_ct_get_tuplepr(skb, skb_network_offset(skb),
			       proto, &tuple))
		goto out;

	thash = nf_conntrack_find_get(dev_net(skb->dev), ca->zone, &tuple);
	if (!thash)
		goto out;

	c = nf_ct_tuplehash_to_ctrack(thash);
	/* using overlimits stats to count how many packets marked */
	ca->tcf_qstats.overlimits++;
	skb->mark = c->mark;
	nf_ct_put(c);

out:
	spin_unlock(&ca->tcf_lock);
	return ca->tcf_action;
}

static const struct nla_policy connmark_policy[TCA_CONNMARK_MAX + 1] = {
	[TCA_CONNMARK_PARMS] = { .len = sizeof(struct tc_connmark) },
};

static int tcf_connmark_init(struct net *net, struct nlattr *nla,
			     struct nlattr *est, struct tc_action *a,
			     int ovr, int bind)
{
	struct nlattr *tb[TCA_CONNMARK_MAX + 1];
	struct tcf_connmark_info *ci;
	struct tc_connmark *parm;
	int ret = 0;

	if (!nla)
		return -EINVAL;

	ret = nla_parse_nested(tb, TCA_CONNMARK_MAX, nla, connmark_policy);
	if (ret < 0)
		return ret;

	if (!tb[TCA_CONNMARK_PARMS])
		return -EINVAL;

	parm = nla_data(tb[TCA_CONNMARK_PARMS]);

	if (!tcf_hash_check(parm->index, a, bind)) {
		ret = tcf_hash_create(parm->index, est, a, sizeof(*ci), bind);
		if (ret)
			return ret;

		ci = to_connmark(a);
		ci->tcf_action = parm->action;
		ci->zone = parm->zone;

		tcf_hash_insert(a);
		ret = ACT_P_CREATED;
	} else {
		ci = to_connmark(a);
		if (bind)
			return 0;
		tcf_hash_release(a, bind);
		if (!ovr)
			return -EEXIST;
		/* replacing action and zone */
		ci->tcf_action = parm->action;
		ci->zone = parm->zone;
	}

	return ret;
}

static inline int tcf_connmark_dump(struct sk_buff *skb, struct tc_action *a,
				    int bind, int ref)
{
	unsigned char *b = skb_tail_pointer(skb);
	struct tcf_connmark_info *ci = a->priv;

	struct tc_connmark opt = {
		.index   = ci->tcf_index,
		.refcnt  = ci->tcf_refcnt - ref,
		.bindcnt = ci->tcf_bindcnt - bind,
		.action  = ci->tcf_action,
		.zone   = ci->zone,
	};
	struct tcf_t t;

	if (nla_put(skb, TCA_CONNMARK_PARMS, sizeof(opt), &opt))
		goto nla_put_failure;

	t.install = jiffies_to_clock_t(jiffies - ci->tcf_tm.install);
	t.lastuse = jiffies_to_clock_t(jiffies - ci->tcf_tm.lastuse);
	t.expires = jiffies_to_clock_t(ci->tcf_tm.expires);
	if (nla_put(skb, TCA_CONNMARK_TM, sizeof(t), &t))
		goto nla_put_failure;

	return skb->len;
nla_put_failure:
	nlmsg_trim(skb, b);
	return -1;
}

static struct tc_action_ops act_connmark_ops = {
	.kind		=	"connmark",
	.type		=	TCA_ACT_CONNMARK,
	.owner		=	THIS_MODULE,
	.act		=	tcf_connmark,
	.dump		=	tcf_connmark_dump,
	.init		=	tcf_connmark_init,
};

static int __init connmark_init_module(void)
{
	return tcf_register_action(&act_connmark_ops, CONNMARK_TAB_MASK);
}

static void __exit connmark_cleanup_module(void)
{
	tcf_unregister_action(&act_connmark_ops);
}

module_init(connmark_init_module);
module_exit(connmark_cleanup_module);
MODULE_AUTHOR("Felix Fietkau <nbd@openwrt.org>");
MODULE_DESCRIPTION("Connection tracking mark restoring");
MODULE_LICENSE("GPL");

