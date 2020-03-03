/*
 * Copyright (c) 2013 Eric Leblond <eric@regit.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code partly funded by OISF
 * (http://www.openinfosecfoundation.org/)
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/jhash.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_queue.h>

static u32 jhash_initval __read_mostly;

struct nft_queue {
	u16	queuenum;
	u16	queues_total;
	u16	flags;
};

static void nft_queue_eval(const struct nft_expr *expr,
			   struct nft_regs *regs,
			   const struct nft_pktinfo *pkt)
{
	struct nft_queue *priv = nft_expr_priv(expr);
	u32 queue = priv->queuenum;
	u32 ret;

	if (priv->queues_total > 1) {
		if (priv->flags & NFT_QUEUE_FLAG_CPU_FANOUT) {
			int cpu = raw_smp_processor_id();

			queue = priv->queuenum + cpu % priv->queues_total;
		} else {
			queue = nfqueue_hash(pkt->skb, queue,
					     priv->queues_total, pkt->ops->pf,
					     jhash_initval);
		}
	}

	ret = NF_QUEUE_NR(queue);
	if (priv->flags & NFT_QUEUE_FLAG_BYPASS)
		ret |= NF_VERDICT_FLAG_QUEUE_BYPASS;

	regs->verdict.code = ret;
}

static const struct nla_policy nft_queue_policy[NFTA_QUEUE_MAX + 1] = {
	[NFTA_QUEUE_NUM]	= { .type = NLA_U16 },
	[NFTA_QUEUE_TOTAL]	= { .type = NLA_U16 },
	[NFTA_QUEUE_FLAGS]	= { .type = NLA_U16 },
};

static int nft_queue_init(const struct nft_ctx *ctx,
			   const struct nft_expr *expr,
			   const struct nlattr * const tb[])
{
	struct nft_queue *priv = nft_expr_priv(expr);

	if (tb[NFTA_QUEUE_NUM] == NULL)
		return -EINVAL;

	init_hashrandom(&jhash_initval);
	priv->queuenum = ntohs(nla_get_be16(tb[NFTA_QUEUE_NUM]));

	if (tb[NFTA_QUEUE_TOTAL] != NULL)
		priv->queues_total = ntohs(nla_get_be16(tb[NFTA_QUEUE_TOTAL]));
	if (tb[NFTA_QUEUE_FLAGS] != NULL) {
		priv->flags = ntohs(nla_get_be16(tb[NFTA_QUEUE_FLAGS]));
		if (priv->flags & ~NFT_QUEUE_FLAG_MASK)
			return -EINVAL;
	}
	return 0;
}

static int nft_queue_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct nft_queue *priv = nft_expr_priv(expr);

	if (nla_put_be16(skb, NFTA_QUEUE_NUM, htons(priv->queuenum)) ||
	    nla_put_be16(skb, NFTA_QUEUE_TOTAL, htons(priv->queues_total)) ||
	    nla_put_be16(skb, NFTA_QUEUE_FLAGS, htons(priv->flags)))
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -1;
}

static struct nft_expr_type nft_queue_type;
static const struct nft_expr_ops nft_queue_ops = {
	.type		= &nft_queue_type,
	.size		= NFT_EXPR_SIZE(sizeof(struct nft_queue)),
	.eval		= nft_queue_eval,
	.init		= nft_queue_init,
	.dump		= nft_queue_dump,
};

static struct nft_expr_type nft_queue_type __read_mostly = {
	.name		= "queue",
	.ops		= &nft_queue_ops,
	.policy		= nft_queue_policy,
	.maxattr	= NFTA_QUEUE_MAX,
	.owner		= THIS_MODULE,
};

static int __init nft_queue_module_init(void)
{
	return nft_register_expr(&nft_queue_type);
}

static void __exit nft_queue_module_exit(void)
{
	nft_unregister_expr(&nft_queue_type);
}

module_init(nft_queue_module_init);
module_exit(nft_queue_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Leblond <eric@regit.org>");
MODULE_ALIAS_NFT_EXPR("queue");
