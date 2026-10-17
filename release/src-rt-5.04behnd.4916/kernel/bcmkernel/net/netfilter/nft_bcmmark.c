/*
*    Copyright (c) 2003-2024 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2024:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#include <net/netfilter/nf_tables.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft: Broadcom bcmmark match support");
MODULE_ALIAS("nft_bcmmark");

#if 0
#define DEBUG_BCMMARK(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_BCMMARK(fmt, ...) {}
#endif

enum nft_bcmmark_attributes {
	NFTA_BCMMARK_UNSPEC,
	NFTA_BCMMARK_INVERT,
	NFTA_BCMMARK_MARK,
	NFTA_BCMMARK_MASK,
	NFTA_BCMMARK_SET_MARK,
	NFTA_BCMMARK_SET_MASK,
	__NFTA_BCMMARK_MAX,
};
#define NFTA_BCMMARK_MAX (__NFTA_BCMMARK_MAX - 1)

struct nft_bcmmark {
	uint8_t invert;
	uint32_t mark;
	uint32_t mask;
	uint32_t set_mark;
	uint32_t set_mask;
};

static const struct nla_policy nft_bcmmark_policy[NFTA_BCMMARK_MAX + 1] = {
	[NFTA_BCMMARK_INVERT]   = { .type = NLA_U8 },
	[NFTA_BCMMARK_MARK]     = { .type = NLA_U32 },
	[NFTA_BCMMARK_MASK]     = { .type = NLA_U32 },
	[NFTA_BCMMARK_SET_MARK] = { .type = NLA_U32 },
	[NFTA_BCMMARK_SET_MASK] = { .type = NLA_U32 },
};

static void nft_bcmmark_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct nft_bcmmark *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;

	DEBUG_BCMMARK("invert=%u, mark=0x%x, mask=0x%x, set_mark=0x%x, set_mask=0x%x.\n",
	               priv->invert, priv->mark, priv->mask, priv->set_mark, priv->set_mask);

	if(priv->mask)
	{
		if((((skb->bcm_ext.bcmmark & priv->mask) == priv->mark) ^ priv->invert) == 0)
		{
			regs->verdict.code = NFT_BREAK;
			return;
		}
	}
	else if(priv->set_mask)
	{
		skb->bcm_ext.bcmmark = (skb->bcm_ext.bcmmark & ~priv->set_mask) ^ priv->set_mark;		
	}

	regs->verdict.code = NFT_CONTINUE;
	return;

}

static int nft_bcmmark_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_bcmmark *priv = nft_expr_priv(expr);

	priv->invert = 0;
	priv->mark = 0;
	priv->mask = 0;
	priv->set_mark = 0;
	priv->set_mask = 0;

	if (tb[NFTA_BCMMARK_INVERT])
	{
		priv->invert = nla_get_u8(tb[NFTA_BCMMARK_INVERT]);
	}

	if (tb[NFTA_BCMMARK_MARK])
	{
		priv->mark = nla_get_u32(tb[NFTA_BCMMARK_MARK]);
	}

	if (tb[NFTA_BCMMARK_MASK])
	{
		priv->mask = nla_get_u32(tb[NFTA_BCMMARK_MASK]);
	}

	if (tb[NFTA_BCMMARK_SET_MARK])
	{
		priv->set_mark = nla_get_u32(tb[NFTA_BCMMARK_SET_MARK]);
	}

	if (tb[NFTA_BCMMARK_SET_MASK])
	{
		priv->set_mask = nla_get_u32(tb[NFTA_BCMMARK_SET_MASK]);
	}

	DEBUG_BCMMARK("invert=%u, mark=0x%x, mask=0x%x, set_mark=0x%x, set_mask=0x%x.\n",
	               priv->invert, priv->mark, priv->mask, priv->set_mark, priv->set_mask);

	return 0;

}

static int nft_bcmmark_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	const struct nft_bcmmark *priv = nft_expr_priv(expr);

	if (nla_put_u8(skb, NFTA_BCMMARK_INVERT, priv->invert))
		return -1;

	if (nla_put_u32(skb, NFTA_BCMMARK_MARK, priv->mark))
		return -1;

	if (nla_put_u32(skb, NFTA_BCMMARK_MASK, priv->mask))
		return -1;

	if (nla_put_u32(skb, NFTA_BCMMARK_SET_MARK, priv->set_mark))
		return -1;

	if (nla_put_u32(skb, NFTA_BCMMARK_SET_MASK, priv->set_mask))
		return -1;

	DEBUG_BCMMARK("invert=%u, mark=0x%x, mask=0x%x, set_mark=0x%x, set_mask=0x%x.\n",
	               priv->invert, priv->mark, priv->mask, priv->set_mark, priv->set_mask);

	return 0;
}

static struct nft_expr_type nft_bcmmark_type;
static const struct nft_expr_ops nft_bcmmark_op = {
	.eval    = nft_bcmmark_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_bcmmark)),
	.init    = nft_bcmmark_init,
	.dump    = nft_bcmmark_dump,
	.type    = &nft_bcmmark_type,
};

static struct nft_expr_type nft_bcmmark_type __read_mostly =  {
	.ops = &nft_bcmmark_op,
	.name = "bcmmark",
	.owner = THIS_MODULE,
	.policy = nft_bcmmark_policy,
	.maxattr = NFTA_BCMMARK_MAX,
};

static int __init nft_bcmmark_module_init(void)
{       
	return nft_register_expr(&nft_bcmmark_type);
}

static void __exit nft_bcmmark_module_exit(void)
{
	nft_unregister_expr(&nft_bcmmark_type);
}

module_init(nft_bcmmark_module_init);
module_exit(nft_bcmmark_module_exit);

