/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This software has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_tables_compat.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_arp/arp_tables.h>
#include <net/netfilter/nf_tables.h>

static int nft_compat_chain_validate_dependency(const char *tablename,
						const struct nft_chain *chain)
{
	const struct nft_base_chain *basechain;

	if (!tablename || !(chain->flags & NFT_BASE_CHAIN))
		return 0;

	basechain = nft_base_chain(chain);
	if (strcmp(tablename, "nat") == 0 &&
	    basechain->type->type != NFT_CHAIN_T_NAT)
		return -EINVAL;

	return 0;
}

union nft_entry {
	struct ipt_entry e4;
	struct ip6t_entry e6;
	struct ebt_entry ebt;
	struct arpt_entry arp;
};

static inline void
nft_compat_set_par(struct xt_action_param *par, void *xt, const void *xt_info)
{
	par->target	= xt;
	par->targinfo	= xt_info;
	par->hotdrop	= false;
}

static void nft_target_eval_xt(const struct nft_expr *expr,
			       struct nft_regs *regs,
			       const struct nft_pktinfo *pkt)
{
	void *info = nft_expr_priv(expr);
	struct xt_target *target = expr->ops->data;
	struct sk_buff *skb = pkt->skb;
	int ret;

	nft_compat_set_par((struct xt_action_param *)&pkt->xt, target, info);

	ret = target->target(skb, &pkt->xt);

	if (pkt->xt.hotdrop)
		ret = NF_DROP;

	switch (ret) {
	case XT_CONTINUE:
		regs->verdict.code = NFT_CONTINUE;
		break;
	default:
		regs->verdict.code = ret;
		break;
	}
}

static void nft_target_eval_bridge(const struct nft_expr *expr,
				   struct nft_regs *regs,
				   const struct nft_pktinfo *pkt)
{
	void *info = nft_expr_priv(expr);
	struct xt_target *target = expr->ops->data;
	struct sk_buff *skb = pkt->skb;
	int ret;

	nft_compat_set_par((struct xt_action_param *)&pkt->xt, target, info);

	ret = target->target(skb, &pkt->xt);

	if (pkt->xt.hotdrop)
		ret = NF_DROP;

	switch (ret) {
	case EBT_ACCEPT:
		regs->verdict.code = NF_ACCEPT;
		break;
	case EBT_DROP:
		regs->verdict.code = NF_DROP;
		break;
	case EBT_CONTINUE:
		regs->verdict.code = NFT_CONTINUE;
		break;
	case EBT_RETURN:
		regs->verdict.code = NFT_RETURN;
		break;
	default:
		regs->verdict.code = ret;
		break;
	}
}

static const struct nla_policy nft_target_policy[NFTA_TARGET_MAX + 1] = {
	[NFTA_TARGET_NAME]	= { .type = NLA_NUL_STRING },
	[NFTA_TARGET_REV]	= { .type = NLA_U32 },
	[NFTA_TARGET_INFO]	= { .type = NLA_BINARY },
};

static void
nft_target_set_tgchk_param(struct xt_tgchk_param *par,
			   const struct nft_ctx *ctx,
			   struct xt_target *target, void *info,
			   union nft_entry *entry, u16 proto, bool inv)
{
	par->net	= ctx->net;
	par->table	= ctx->table->name;
	switch (ctx->afi->family) {
	case AF_INET:
		entry->e4.ip.proto = proto;
		entry->e4.ip.invflags = inv ? IPT_INV_PROTO : 0;
		break;
	case AF_INET6:
		if (proto)
			entry->e6.ipv6.flags |= IP6T_F_PROTO;

		entry->e6.ipv6.proto = proto;
		entry->e6.ipv6.invflags = inv ? IP6T_INV_PROTO : 0;
		break;
	case NFPROTO_BRIDGE:
		entry->ebt.ethproto = (__force __be16)proto;
		entry->ebt.invflags = inv ? EBT_IPROTO : 0;
		break;
	case NFPROTO_ARP:
		break;
	}
	par->entryinfo	= entry;
	par->target	= target;
	par->targinfo	= info;
	if (ctx->chain->flags & NFT_BASE_CHAIN) {
		const struct nft_base_chain *basechain =
						nft_base_chain(ctx->chain);
		const struct nf_hook_ops *ops = &basechain->ops[0];

		par->hook_mask = 1 << ops->hooknum;
	} else {
		par->hook_mask = 0;
	}
	par->family	= ctx->afi->family;
}

static void target_compat_from_user(struct xt_target *t, void *in, void *out)
{
	int pad;

	memcpy(out, in, t->targetsize);
	pad = XT_ALIGN(t->targetsize) - t->targetsize;
	if (pad > 0)
		memset(out + t->targetsize, 0, pad);
}

static const struct nla_policy nft_rule_compat_policy[NFTA_RULE_COMPAT_MAX + 1] = {
	[NFTA_RULE_COMPAT_PROTO]	= { .type = NLA_U32 },
	[NFTA_RULE_COMPAT_FLAGS]	= { .type = NLA_U32 },
};

static int nft_parse_compat(const struct nlattr *attr, u16 *proto, bool *inv)
{
	struct nlattr *tb[NFTA_RULE_COMPAT_MAX+1];
	u32 flags;
	int err;

	err = nla_parse_nested(tb, NFTA_RULE_COMPAT_MAX, attr,
			       nft_rule_compat_policy);
	if (err < 0)
		return err;

	if (!tb[NFTA_RULE_COMPAT_PROTO] || !tb[NFTA_RULE_COMPAT_FLAGS])
		return -EINVAL;

	flags = ntohl(nla_get_be32(tb[NFTA_RULE_COMPAT_FLAGS]));
	if (flags & ~NFT_RULE_COMPAT_F_MASK)
		return -EINVAL;
	if (flags & NFT_RULE_COMPAT_F_INV)
		*inv = true;

	*proto = ntohl(nla_get_be32(tb[NFTA_RULE_COMPAT_PROTO]));
	return 0;
}

static int
nft_target_init(const struct nft_ctx *ctx, const struct nft_expr *expr,
		const struct nlattr * const tb[])
{
	void *info = nft_expr_priv(expr);
	struct xt_target *target = expr->ops->data;
	struct xt_tgchk_param par;
	size_t size = XT_ALIGN(nla_len(tb[NFTA_TARGET_INFO]));
	u16 proto = 0;
	bool inv = false;
	union nft_entry e = {};
	int ret;

	ret = nft_compat_chain_validate_dependency(target->table, ctx->chain);
	if (ret < 0)
		goto err;

	target_compat_from_user(target, nla_data(tb[NFTA_TARGET_INFO]), info);

	if (ctx->nla[NFTA_RULE_COMPAT]) {
		ret = nft_parse_compat(ctx->nla[NFTA_RULE_COMPAT], &proto, &inv);
		if (ret < 0)
			goto err;
	}

	nft_target_set_tgchk_param(&par, ctx, target, info, &e, proto, inv);

	ret = xt_check_target(&par, size, proto, inv);
	if (ret < 0)
		goto err;

	/* The standard target cannot be used */
	if (target->target == NULL) {
		ret = -EINVAL;
		goto err;
	}

	return 0;
err:
	module_put(target->me);
	return ret;
}

static void
nft_target_destroy(const struct nft_ctx *ctx, const struct nft_expr *expr)
{
	struct xt_target *target = expr->ops->data;
	void *info = nft_expr_priv(expr);
	struct xt_tgdtor_param par;

	par.net = ctx->net;
	par.target = target;
	par.targinfo = info;
	par.family = ctx->afi->family;
	if (par.target->destroy != NULL)
		par.target->destroy(&par);

	module_put(target->me);
}

static int nft_target_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct xt_target *target = expr->ops->data;
	void *info = nft_expr_priv(expr);

	if (nla_put_string(skb, NFTA_TARGET_NAME, target->name) ||
	    nla_put_be32(skb, NFTA_TARGET_REV, htonl(target->revision)) ||
	    nla_put(skb, NFTA_TARGET_INFO, XT_ALIGN(target->targetsize), info))
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -1;
}

static int nft_target_validate(const struct nft_ctx *ctx,
			       const struct nft_expr *expr,
			       const struct nft_data **data)
{
	struct xt_target *target = expr->ops->data;
	unsigned int hook_mask = 0;
	int ret;

	if (ctx->chain->flags & NFT_BASE_CHAIN) {
		const struct nft_base_chain *basechain =
						nft_base_chain(ctx->chain);
		const struct nf_hook_ops *ops = &basechain->ops[0];

		hook_mask = 1 << ops->hooknum;
		if (!(hook_mask & target->hooks))
			return -EINVAL;

		ret = nft_compat_chain_validate_dependency(target->table,
							   ctx->chain);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static void nft_match_eval(const struct nft_expr *expr,
			   struct nft_regs *regs,
			   const struct nft_pktinfo *pkt)
{
	void *info = nft_expr_priv(expr);
	struct xt_match *match = expr->ops->data;
	struct sk_buff *skb = pkt->skb;
	bool ret;

	nft_compat_set_par((struct xt_action_param *)&pkt->xt, match, info);

	ret = match->match(skb, (struct xt_action_param *)&pkt->xt);

	if (pkt->xt.hotdrop) {
		regs->verdict.code = NF_DROP;
		return;
	}

	switch (ret ? 1 : 0) {
	case 1:
		regs->verdict.code = NFT_CONTINUE;
		break;
	case 0:
		regs->verdict.code = NFT_BREAK;
		break;
	}
}

static const struct nla_policy nft_match_policy[NFTA_MATCH_MAX + 1] = {
	[NFTA_MATCH_NAME]	= { .type = NLA_NUL_STRING },
	[NFTA_MATCH_REV]	= { .type = NLA_U32 },
	[NFTA_MATCH_INFO]	= { .type = NLA_BINARY },
};

/* struct xt_mtchk_param and xt_tgchk_param look very similar */
static void
nft_match_set_mtchk_param(struct xt_mtchk_param *par, const struct nft_ctx *ctx,
			  struct xt_match *match, void *info,
			  union nft_entry *entry, u16 proto, bool inv)
{
	par->net	= ctx->net;
	par->table	= ctx->table->name;
	switch (ctx->afi->family) {
	case AF_INET:
		entry->e4.ip.proto = proto;
		entry->e4.ip.invflags = inv ? IPT_INV_PROTO : 0;
		break;
	case AF_INET6:
		if (proto)
			entry->e6.ipv6.flags |= IP6T_F_PROTO;

		entry->e6.ipv6.proto = proto;
		entry->e6.ipv6.invflags = inv ? IP6T_INV_PROTO : 0;
		break;
	case NFPROTO_BRIDGE:
		entry->ebt.ethproto = (__force __be16)proto;
		entry->ebt.invflags = inv ? EBT_IPROTO : 0;
		break;
	case NFPROTO_ARP:
		break;
	}
	par->entryinfo	= entry;
	par->match	= match;
	par->matchinfo	= info;
	if (ctx->chain->flags & NFT_BASE_CHAIN) {
		const struct nft_base_chain *basechain =
						nft_base_chain(ctx->chain);
		const struct nf_hook_ops *ops = &basechain->ops[0];

		par->hook_mask = 1 << ops->hooknum;
	} else {
		par->hook_mask = 0;
	}
	par->family	= ctx->afi->family;
}

static void match_compat_from_user(struct xt_match *m, void *in, void *out)
{
	int pad;

	memcpy(out, in, m->matchsize);
	pad = XT_ALIGN(m->matchsize) - m->matchsize;
	if (pad > 0)
		memset(out + m->matchsize, 0, pad);
}

static int
nft_match_init(const struct nft_ctx *ctx, const struct nft_expr *expr,
		const struct nlattr * const tb[])
{
	void *info = nft_expr_priv(expr);
	struct xt_match *match = expr->ops->data;
	struct xt_mtchk_param par;
	size_t size = XT_ALIGN(nla_len(tb[NFTA_MATCH_INFO]));
	u16 proto = 0;
	bool inv = false;
	union nft_entry e = {};
	int ret;

	ret = nft_compat_chain_validate_dependency(match->table, ctx->chain);
	if (ret < 0)
		goto err;

	match_compat_from_user(match, nla_data(tb[NFTA_MATCH_INFO]), info);

	if (ctx->nla[NFTA_RULE_COMPAT]) {
		ret = nft_parse_compat(ctx->nla[NFTA_RULE_COMPAT], &proto, &inv);
		if (ret < 0)
			goto err;
	}

	nft_match_set_mtchk_param(&par, ctx, match, info, &e, proto, inv);

	ret = xt_check_match(&par, size, proto, inv);
	if (ret < 0)
		goto err;

	return 0;
err:
	module_put(match->me);
	return ret;
}

static void
nft_match_destroy(const struct nft_ctx *ctx, const struct nft_expr *expr)
{
	struct xt_match *match = expr->ops->data;
	void *info = nft_expr_priv(expr);
	struct xt_mtdtor_param par;

	par.net = ctx->net;
	par.match = match;
	par.matchinfo = info;
	par.family = ctx->afi->family;
	if (par.match->destroy != NULL)
		par.match->destroy(&par);

	module_put(match->me);
}

static int nft_match_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	void *info = nft_expr_priv(expr);
	struct xt_match *match = expr->ops->data;

	if (nla_put_string(skb, NFTA_MATCH_NAME, match->name) ||
	    nla_put_be32(skb, NFTA_MATCH_REV, htonl(match->revision)) ||
	    nla_put(skb, NFTA_MATCH_INFO, XT_ALIGN(match->matchsize), info))
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -1;
}

static int nft_match_validate(const struct nft_ctx *ctx,
			      const struct nft_expr *expr,
			      const struct nft_data **data)
{
	struct xt_match *match = expr->ops->data;
	unsigned int hook_mask = 0;
	int ret;

	if (ctx->chain->flags & NFT_BASE_CHAIN) {
		const struct nft_base_chain *basechain =
						nft_base_chain(ctx->chain);
		const struct nf_hook_ops *ops = &basechain->ops[0];

		hook_mask = 1 << ops->hooknum;
		if (!(hook_mask & match->hooks))
			return -EINVAL;

		ret = nft_compat_chain_validate_dependency(match->table,
							   ctx->chain);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int
nfnl_compat_fill_info(struct sk_buff *skb, u32 portid, u32 seq, u32 type,
		      int event, u16 family, const char *name,
		      int rev, int target)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfmsg;
	unsigned int flags = portid ? NLM_F_MULTI : 0;

	event |= NFNL_SUBSYS_NFT_COMPAT << 8;
	nlh = nlmsg_put(skb, portid, seq, event, sizeof(*nfmsg), flags);
	if (nlh == NULL)
		goto nlmsg_failure;

	nfmsg = nlmsg_data(nlh);
	nfmsg->nfgen_family = family;
	nfmsg->version = NFNETLINK_V0;
	nfmsg->res_id = 0;

	if (nla_put_string(skb, NFTA_COMPAT_NAME, name) ||
	    nla_put_be32(skb, NFTA_COMPAT_REV, htonl(rev)) ||
	    nla_put_be32(skb, NFTA_COMPAT_TYPE, htonl(target)))
		goto nla_put_failure;

	nlmsg_end(skb, nlh);
	return skb->len;

nlmsg_failure:
nla_put_failure:
	nlmsg_cancel(skb, nlh);
	return -1;
}

static int
nfnl_compat_get(struct sock *nfnl, struct sk_buff *skb,
		const struct nlmsghdr *nlh, const struct nlattr * const tb[])
{
	int ret = 0, target;
	struct nfgenmsg *nfmsg;
	const char *fmt;
	const char *name;
	u32 rev;
	struct sk_buff *skb2;

	if (tb[NFTA_COMPAT_NAME] == NULL ||
	    tb[NFTA_COMPAT_REV] == NULL ||
	    tb[NFTA_COMPAT_TYPE] == NULL)
		return -EINVAL;

	name = nla_data(tb[NFTA_COMPAT_NAME]);
	rev = ntohl(nla_get_be32(tb[NFTA_COMPAT_REV]));
	target = ntohl(nla_get_be32(tb[NFTA_COMPAT_TYPE]));

	nfmsg = nlmsg_data(nlh);

	switch(nfmsg->nfgen_family) {
	case AF_INET:
		fmt = "ipt_%s";
		break;
	case AF_INET6:
		fmt = "ip6t_%s";
		break;
	case NFPROTO_BRIDGE:
		fmt = "ebt_%s";
		break;
	case NFPROTO_ARP:
		fmt = "arpt_%s";
		break;
	default:
		pr_err("nft_compat: unsupported protocol %d\n",
			nfmsg->nfgen_family);
		return -EINVAL;
	}

	try_then_request_module(xt_find_revision(nfmsg->nfgen_family, name,
						 rev, target, &ret),
						 fmt, name);

	if (ret < 0)
		return ret;

	skb2 = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (skb2 == NULL)
		return -ENOMEM;

	/* include the best revision for this extension in the message */
	if (nfnl_compat_fill_info(skb2, NETLINK_CB(skb).portid,
				  nlh->nlmsg_seq,
				  NFNL_MSG_TYPE(nlh->nlmsg_type),
				  NFNL_MSG_COMPAT_GET,
				  nfmsg->nfgen_family,
				  name, ret, target) <= 0) {
		kfree_skb(skb2);
		return -ENOSPC;
	}

	ret = netlink_unicast(nfnl, skb2, NETLINK_CB(skb).portid,
				MSG_DONTWAIT);
	if (ret > 0)
		ret = 0;

	return ret == -EAGAIN ? -ENOBUFS : ret;
}

static const struct nla_policy nfnl_compat_policy_get[NFTA_COMPAT_MAX+1] = {
	[NFTA_COMPAT_NAME]	= { .type = NLA_NUL_STRING,
				    .len = NFT_COMPAT_NAME_MAX-1 },
	[NFTA_COMPAT_REV]	= { .type = NLA_U32 },
	[NFTA_COMPAT_TYPE]	= { .type = NLA_U32 },
};

static const struct nfnl_callback nfnl_nft_compat_cb[NFNL_MSG_COMPAT_MAX] = {
	[NFNL_MSG_COMPAT_GET]		= { .call = nfnl_compat_get,
					    .attr_count = NFTA_COMPAT_MAX,
					    .policy = nfnl_compat_policy_get },
};

static const struct nfnetlink_subsystem nfnl_compat_subsys = {
	.name		= "nft-compat",
	.subsys_id	= NFNL_SUBSYS_NFT_COMPAT,
	.cb_count	= NFNL_MSG_COMPAT_MAX,
	.cb		= nfnl_nft_compat_cb,
};

static LIST_HEAD(nft_match_list);

struct nft_xt {
	struct list_head	head;
	struct nft_expr_ops	ops;
};

static struct nft_expr_type nft_match_type;

static bool nft_match_cmp(const struct xt_match *match,
			  const char *name, u32 rev, u32 family)
{
	return strcmp(match->name, name) == 0 && match->revision == rev &&
	       (match->family == NFPROTO_UNSPEC || match->family == family);
}

static const struct nft_expr_ops *
nft_match_select_ops(const struct nft_ctx *ctx,
		     const struct nlattr * const tb[])
{
	struct nft_xt *nft_match;
	struct xt_match *match;
	char *mt_name;
	u32 rev, family;

	if (tb[NFTA_MATCH_NAME] == NULL ||
	    tb[NFTA_MATCH_REV] == NULL ||
	    tb[NFTA_MATCH_INFO] == NULL)
		return ERR_PTR(-EINVAL);

	mt_name = nla_data(tb[NFTA_MATCH_NAME]);
	rev = ntohl(nla_get_be32(tb[NFTA_MATCH_REV]));
	family = ctx->afi->family;

	/* Re-use the existing match if it's already loaded. */
	list_for_each_entry(nft_match, &nft_match_list, head) {
		struct xt_match *match = nft_match->ops.data;

		if (nft_match_cmp(match, mt_name, rev, family)) {
			if (!try_module_get(match->me))
				return ERR_PTR(-ENOENT);

			return &nft_match->ops;
		}
	}

	match = xt_request_find_match(family, mt_name, rev);
	if (IS_ERR(match))
		return ERR_PTR(-ENOENT);

	/* This is the first time we use this match, allocate operations */
	nft_match = kzalloc(sizeof(struct nft_xt), GFP_KERNEL);
	if (nft_match == NULL)
		return ERR_PTR(-ENOMEM);

	nft_match->ops.type = &nft_match_type;
	nft_match->ops.size = NFT_EXPR_SIZE(XT_ALIGN(match->matchsize));
	nft_match->ops.eval = nft_match_eval;
	nft_match->ops.init = nft_match_init;
	nft_match->ops.destroy = nft_match_destroy;
	nft_match->ops.dump = nft_match_dump;
	nft_match->ops.validate = nft_match_validate;
	nft_match->ops.data = match;

	list_add(&nft_match->head, &nft_match_list);

	return &nft_match->ops;
}

static void nft_match_release(void)
{
	struct nft_xt *nft_match, *tmp;

	list_for_each_entry_safe(nft_match, tmp, &nft_match_list, head)
		kfree(nft_match);
}

static struct nft_expr_type nft_match_type __read_mostly = {
	.name		= "match",
	.select_ops	= nft_match_select_ops,
	.policy		= nft_match_policy,
	.maxattr	= NFTA_MATCH_MAX,
	.owner		= THIS_MODULE,
};

static LIST_HEAD(nft_target_list);

static struct nft_expr_type nft_target_type;

static bool nft_target_cmp(const struct xt_target *tg,
			   const char *name, u32 rev, u32 family)
{
	return strcmp(tg->name, name) == 0 && tg->revision == rev &&
	       (tg->family == NFPROTO_UNSPEC || tg->family == family);
}

static const struct nft_expr_ops *
nft_target_select_ops(const struct nft_ctx *ctx,
		      const struct nlattr * const tb[])
{
	struct nft_xt *nft_target;
	struct xt_target *target;
	char *tg_name;
	u32 rev, family;

	if (tb[NFTA_TARGET_NAME] == NULL ||
	    tb[NFTA_TARGET_REV] == NULL ||
	    tb[NFTA_TARGET_INFO] == NULL)
		return ERR_PTR(-EINVAL);

	tg_name = nla_data(tb[NFTA_TARGET_NAME]);
	rev = ntohl(nla_get_be32(tb[NFTA_TARGET_REV]));
	family = ctx->afi->family;

	/* Re-use the existing target if it's already loaded. */
	list_for_each_entry(nft_target, &nft_target_list, head) {
		struct xt_target *target = nft_target->ops.data;

		if (nft_target_cmp(target, tg_name, rev, family)) {
			if (!try_module_get(target->me))
				return ERR_PTR(-ENOENT);

			return &nft_target->ops;
		}
	}

	target = xt_request_find_target(family, tg_name, rev);
	if (IS_ERR(target))
		return ERR_PTR(-ENOENT);

	/* This is the first time we use this target, allocate operations */
	nft_target = kzalloc(sizeof(struct nft_xt), GFP_KERNEL);
	if (nft_target == NULL)
		return ERR_PTR(-ENOMEM);

	nft_target->ops.type = &nft_target_type;
	nft_target->ops.size = NFT_EXPR_SIZE(XT_ALIGN(target->targetsize));
	nft_target->ops.init = nft_target_init;
	nft_target->ops.destroy = nft_target_destroy;
	nft_target->ops.dump = nft_target_dump;
	nft_target->ops.validate = nft_target_validate;
	nft_target->ops.data = target;

	if (family == NFPROTO_BRIDGE)
		nft_target->ops.eval = nft_target_eval_bridge;
	else
		nft_target->ops.eval = nft_target_eval_xt;

	list_add(&nft_target->head, &nft_target_list);

	return &nft_target->ops;
}

static void nft_target_release(void)
{
	struct nft_xt *nft_target, *tmp;

	list_for_each_entry_safe(nft_target, tmp, &nft_target_list, head)
		kfree(nft_target);
}

static struct nft_expr_type nft_target_type __read_mostly = {
	.name		= "target",
	.select_ops	= nft_target_select_ops,
	.policy		= nft_target_policy,
	.maxattr	= NFTA_TARGET_MAX,
	.owner		= THIS_MODULE,
};

static int __init nft_compat_module_init(void)
{
	int ret;

	ret = nft_register_expr(&nft_match_type);
	if (ret < 0)
		return ret;

	ret = nft_register_expr(&nft_target_type);
	if (ret < 0)
		goto err_match;

	ret = nfnetlink_subsys_register(&nfnl_compat_subsys);
	if (ret < 0) {
		pr_err("nft_compat: cannot register with nfnetlink.\n");
		goto err_target;
	}

	pr_info("nf_tables_compat: (c) 2012 Pablo Neira Ayuso <pablo@netfilter.org>\n");

	return ret;

err_target:
	nft_unregister_expr(&nft_target_type);
err_match:
	nft_unregister_expr(&nft_match_type);
	return ret;
}

static void __exit nft_compat_module_exit(void)
{
	nfnetlink_subsys_unregister(&nfnl_compat_subsys);
	nft_unregister_expr(&nft_target_type);
	nft_unregister_expr(&nft_match_type);
	nft_match_release();
	nft_target_release();
}

MODULE_ALIAS_NFNL_SUBSYS(NFNL_SUBSYS_NFT_COMPAT);

module_init(nft_compat_module_init);
module_exit(nft_compat_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pablo Neira Ayuso <pablo@netfilter.org>");
MODULE_ALIAS_NFT_EXPR("match");
MODULE_ALIAS_NFT_EXPR("target");
