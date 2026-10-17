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

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include <linux/bcm_skb_defines.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("qosmap: support QOS map per IP packets");
MODULE_ALIAS("nft_qosmap");

#if 0
#define DEBUG_QOSMAP(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_QOSMAP(fmt, ...) {}
#endif

enum nft_qosmap_attributes {
	NFTA_QOSMAP_UNSPEC,
	NFTA_QOSMAP_DSCP2PBIT,
	NFTA_QOSMAP_DSCP2Q,
	__NFTA_QOSMAP_MAX,
};
#define NFTA_QOSMAP_MAX (__NFTA_QOSMAP_MAX - 1)
#define QOSMAP_CMD_INVALID 255

struct nft_qosmap {
	uint8_t dscp2pbit;
	uint8_t dscp2q;
	
	/* Used internally by the kernel */
	void *data __attribute__((aligned(8)));	
};

static const struct nla_policy nft_qosmap_policy[NFTA_QOSMAP_MAX + 1] = {
	[NFTA_QOSMAP_DSCP2PBIT]	= { .type = NLA_U8 },
	[NFTA_QOSMAP_DSCP2Q]	= { .type = NLA_U8 },
};

static void nft_qosmap_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct nft_qosmap *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;

	DEBUG_QOSMAP("skb=%8p, priv=%8p, dscp2pbit=%d, dscp2q=%d.\n",
	            skb, priv, priv->dscp2pbit, priv->dscp2q);

#if defined(CONFIG_BLOG)
	if (skb->blog_p) {
		skb->blog_p->dscp2pbit = priv->dscp2pbit ? 1:0;
		skb->blog_p->dscp2q = priv->dscp2q ? 1:0;
	}
#endif
	regs->verdict.code = NFT_CONTINUE;

	return;	
}

static int nft_qosmap_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_qosmap *priv = nft_expr_priv(expr);

	priv->dscp2pbit = QOSMAP_CMD_INVALID;
	priv->dscp2q = QOSMAP_CMD_INVALID;
	
	if (tb[NFTA_QOSMAP_DSCP2PBIT])
	{
		priv->dscp2pbit = nla_get_u8(tb[NFTA_QOSMAP_DSCP2PBIT]);		
	}

	if (tb[NFTA_QOSMAP_DSCP2Q])
	{
		priv->dscp2q = nla_get_u8(tb[NFTA_QOSMAP_DSCP2Q]);
	}

	DEBUG_QOSMAP("dscp2pbit=%d, dscp2q=%d .\n", priv->dscp2pbit, priv->dscp2q);

	return 0;
}

static int nft_qosmap_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	const struct nft_qosmap *priv = nft_expr_priv(expr);

	if (nla_put_u8(skb, NFTA_QOSMAP_DSCP2PBIT, priv->dscp2pbit))
		return -1;

	if (nla_put_u8(skb, NFTA_QOSMAP_DSCP2Q, priv->dscp2q))
		return -1;

	DEBUG_QOSMAP("dscp2pbit=%d, dscp2q=%d.\n",
	            priv->dscp2pbit, priv->dscp2q);

	return 0;
}

static struct nft_expr_type nft_qosmap_type;
static const struct nft_expr_ops nft_qosmap_op = {
	.eval    = nft_qosmap_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_qosmap)),
	.init    = nft_qosmap_init,
	.dump    = nft_qosmap_dump,
	.type    = &nft_qosmap_type,
};

static struct nft_expr_type nft_qosmap_type __read_mostly =  {
	.ops = &nft_qosmap_op,
	.name = "qosmap",
	.owner = THIS_MODULE,
	.policy = nft_qosmap_policy,
	.maxattr = NFTA_QOSMAP_MAX,
};

static int __init nft_qosmap_module_init(void)
{       
	return nft_register_expr(&nft_qosmap_type);
}

static void __exit nft_qosmap_module_exit(void)
{
	nft_unregister_expr(&nft_qosmap_type);
}

module_init(nft_qosmap_module_init);
module_exit(nft_qosmap_module_exit);

