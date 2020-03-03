/*
 * Copyright (c) 2015 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables_core.h>

struct nft_dynset {
	struct nft_set			*set;
	struct nft_set_ext_tmpl		tmpl;
	enum nft_dynset_ops		op:8;
	enum nft_registers		sreg_key:8;
	enum nft_registers		sreg_data:8;
	u64				timeout;
	struct nft_expr			*expr;
	struct nft_set_binding		binding;
};

static void *nft_dynset_new(struct nft_set *set, const struct nft_expr *expr,
			    struct nft_regs *regs)
{
	const struct nft_dynset *priv = nft_expr_priv(expr);
	struct nft_set_ext *ext;
	u64 timeout;
	void *elem;

	if (set->size && !atomic_add_unless(&set->nelems, 1, set->size))
		return NULL;

	timeout = priv->timeout ? : set->timeout;
	elem = nft_set_elem_init(set, &priv->tmpl,
				 &regs->data[priv->sreg_key],
				 &regs->data[priv->sreg_data],
				 timeout, GFP_ATOMIC);
	if (elem == NULL) {
		if (set->size)
			atomic_dec(&set->nelems);
		return NULL;
	}

	ext = nft_set_elem_ext(set, elem);
	if (priv->expr != NULL)
		nft_expr_clone(nft_set_ext_expr(ext), priv->expr);

	return elem;
}

static void nft_dynset_eval(const struct nft_expr *expr,
			    struct nft_regs *regs,
			    const struct nft_pktinfo *pkt)
{
	const struct nft_dynset *priv = nft_expr_priv(expr);
	struct nft_set *set = priv->set;
	const struct nft_set_ext *ext;
	const struct nft_expr *sexpr;
	u64 timeout;

	if (set->ops->update(set, &regs->data[priv->sreg_key], nft_dynset_new,
			     expr, regs, &ext)) {
		sexpr = NULL;
		if (nft_set_ext_exists(ext, NFT_SET_EXT_EXPR))
			sexpr = nft_set_ext_expr(ext);

		if (priv->op == NFT_DYNSET_OP_UPDATE &&
		    nft_set_ext_exists(ext, NFT_SET_EXT_EXPIRATION)) {
			timeout = priv->timeout ? : set->timeout;
			*nft_set_ext_expiration(ext) = jiffies + timeout;
		} else if (sexpr == NULL)
			goto out;

		if (sexpr != NULL)
			sexpr->ops->eval(sexpr, regs, pkt);
		return;
	}
out:
	regs->verdict.code = NFT_BREAK;
}

static const struct nla_policy nft_dynset_policy[NFTA_DYNSET_MAX + 1] = {
	[NFTA_DYNSET_SET_NAME]	= { .type = NLA_STRING },
	[NFTA_DYNSET_SET_ID]	= { .type = NLA_U32 },
	[NFTA_DYNSET_OP]	= { .type = NLA_U32 },
	[NFTA_DYNSET_SREG_KEY]	= { .type = NLA_U32 },
	[NFTA_DYNSET_SREG_DATA]	= { .type = NLA_U32 },
	[NFTA_DYNSET_TIMEOUT]	= { .type = NLA_U64 },
	[NFTA_DYNSET_EXPR]	= { .type = NLA_NESTED },
};

static int nft_dynset_init(const struct nft_ctx *ctx,
			   const struct nft_expr *expr,
			   const struct nlattr * const tb[])
{
	struct nft_dynset *priv = nft_expr_priv(expr);
	struct nft_set *set;
	u64 timeout;
	int err;

	if (tb[NFTA_DYNSET_SET_NAME] == NULL ||
	    tb[NFTA_DYNSET_OP] == NULL ||
	    tb[NFTA_DYNSET_SREG_KEY] == NULL)
		return -EINVAL;

	set = nf_tables_set_lookup(ctx->table, tb[NFTA_DYNSET_SET_NAME]);
	if (IS_ERR(set)) {
		if (tb[NFTA_DYNSET_SET_ID])
			set = nf_tables_set_lookup_byid(ctx->net,
							tb[NFTA_DYNSET_SET_ID]);
		if (IS_ERR(set))
			return PTR_ERR(set);
	}

	if (set->flags & NFT_SET_CONSTANT)
		return -EBUSY;

	priv->op = ntohl(nla_get_be32(tb[NFTA_DYNSET_OP]));
	switch (priv->op) {
	case NFT_DYNSET_OP_ADD:
		break;
	case NFT_DYNSET_OP_UPDATE:
		if (!(set->flags & NFT_SET_TIMEOUT))
			return -EOPNOTSUPP;
		break;
	default:
		return -EOPNOTSUPP;
	}

	timeout = 0;
	if (tb[NFTA_DYNSET_TIMEOUT] != NULL) {
		if (!(set->flags & NFT_SET_TIMEOUT))
			return -EINVAL;
		timeout = be64_to_cpu(nla_get_be64(tb[NFTA_DYNSET_TIMEOUT]));
	}

	priv->sreg_key = nft_parse_register(tb[NFTA_DYNSET_SREG_KEY]);
	err = nft_validate_register_load(priv->sreg_key, set->klen);;
	if (err < 0)
		return err;

	if (tb[NFTA_DYNSET_SREG_DATA] != NULL) {
		if (!(set->flags & NFT_SET_MAP))
			return -EINVAL;
		if (set->dtype == NFT_DATA_VERDICT)
			return -EOPNOTSUPP;

		priv->sreg_data = nft_parse_register(tb[NFTA_DYNSET_SREG_DATA]);
		err = nft_validate_register_load(priv->sreg_data, set->dlen);
		if (err < 0)
			return err;
	} else if (set->flags & NFT_SET_MAP)
		return -EINVAL;

	if (tb[NFTA_DYNSET_EXPR] != NULL) {
		if (!(set->flags & NFT_SET_EVAL))
			return -EINVAL;
		if (!(set->flags & NFT_SET_ANONYMOUS))
			return -EOPNOTSUPP;

		priv->expr = nft_expr_init(ctx, tb[NFTA_DYNSET_EXPR]);
		if (IS_ERR(priv->expr))
			return PTR_ERR(priv->expr);

		err = -EOPNOTSUPP;
		if (!(priv->expr->ops->type->flags & NFT_EXPR_STATEFUL))
			goto err1;
	} else if (set->flags & NFT_SET_EVAL)
		return -EINVAL;

	nft_set_ext_prepare(&priv->tmpl);
	nft_set_ext_add_length(&priv->tmpl, NFT_SET_EXT_KEY, set->klen);
	if (set->flags & NFT_SET_MAP)
		nft_set_ext_add_length(&priv->tmpl, NFT_SET_EXT_DATA, set->dlen);
	if (priv->expr != NULL)
		nft_set_ext_add_length(&priv->tmpl, NFT_SET_EXT_EXPR,
				       priv->expr->ops->size);
	if (set->flags & NFT_SET_TIMEOUT) {
		if (timeout || set->timeout)
			nft_set_ext_add(&priv->tmpl, NFT_SET_EXT_EXPIRATION);
	}

	priv->timeout = timeout;

	err = nf_tables_bind_set(ctx, set, &priv->binding);
	if (err < 0)
		goto err1;

	priv->set = set;
	return 0;

err1:
	if (priv->expr != NULL)
		nft_expr_destroy(ctx, priv->expr);
	return err;
}

static void nft_dynset_destroy(const struct nft_ctx *ctx,
			       const struct nft_expr *expr)
{
	struct nft_dynset *priv = nft_expr_priv(expr);

	nf_tables_unbind_set(ctx, priv->set, &priv->binding);
	if (priv->expr != NULL)
		nft_expr_destroy(ctx, priv->expr);
}

static int nft_dynset_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct nft_dynset *priv = nft_expr_priv(expr);

	if (nft_dump_register(skb, NFTA_DYNSET_SREG_KEY, priv->sreg_key))
		goto nla_put_failure;
	if (priv->set->flags & NFT_SET_MAP &&
	    nft_dump_register(skb, NFTA_DYNSET_SREG_DATA, priv->sreg_data))
		goto nla_put_failure;
	if (nla_put_be32(skb, NFTA_DYNSET_OP, htonl(priv->op)))
		goto nla_put_failure;
	if (nla_put_string(skb, NFTA_DYNSET_SET_NAME, priv->set->name))
		goto nla_put_failure;
	if (nla_put_be64(skb, NFTA_DYNSET_TIMEOUT, cpu_to_be64(priv->timeout)))
		goto nla_put_failure;
	if (priv->expr && nft_expr_dump(skb, NFTA_DYNSET_EXPR, priv->expr))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -1;
}

static struct nft_expr_type nft_dynset_type;
static const struct nft_expr_ops nft_dynset_ops = {
	.type		= &nft_dynset_type,
	.size		= NFT_EXPR_SIZE(sizeof(struct nft_dynset)),
	.eval		= nft_dynset_eval,
	.init		= nft_dynset_init,
	.destroy	= nft_dynset_destroy,
	.dump		= nft_dynset_dump,
};

static struct nft_expr_type nft_dynset_type __read_mostly = {
	.name		= "dynset",
	.ops		= &nft_dynset_ops,
	.policy		= nft_dynset_policy,
	.maxattr	= NFTA_DYNSET_MAX,
	.owner		= THIS_MODULE,
};

int __init nft_dynset_module_init(void)
{
	return nft_register_expr(&nft_dynset_type);
}

void nft_dynset_module_exit(void)
{
	nft_unregister_expr(&nft_dynset_type);
}
