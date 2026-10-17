/*
*    Copyright (c) 2003-2018 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2018:DUAL/GPL:standard

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
#include <linux/tcp.h>

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft wmm mark module");
MODULE_ALIAS("nft_wmm_mark");

enum nft_wmm_mark_attributes {
	NFTA_WMM_MARK_UNSPEC,
	NFTA_WMM_MARK_MARKPOS,
	NFTA_WMM_MARK_MARKSET,
	__NFTA_WMM_MARK_MAX,
};
#define NFTA_WMM_MARK_MAX (__NFTA_WMM_MARK_MAX - 1)

#define NFT_PRIO_LOC_NFMARK 0
#define NFT_PRIO_LOC_NFMASK 7	

struct nft_wmm_mark {
	uint32_t markpos;
	uint32_t markset;
};

static const struct nla_policy nft_wmm_mark_policy[NFTA_WMM_MARK_MAX + 1] = {
	[NFTA_WMM_MARK_MARKPOS]	= { .type = NLA_U32 },
	[NFTA_WMM_MARK_MARKSET] = { .type = NLA_U32 },
};

static void nft_wmm_mark_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct nft_wmm_mark *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;

	if(priv->markset) {
		skb->mark &= ~(NFT_PRIO_LOC_NFMASK << priv->markpos);		
		skb->mark |= (priv->markset << priv->markpos);
	}

}

static int nft_wmm_mark_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_wmm_mark *priv = nft_expr_priv(expr);

	priv->markset = 0;
	priv->markpos = NFT_PRIO_LOC_NFMARK;

	if (tb[NFTA_WMM_MARK_MARKPOS])
	{
		priv->markpos = ntohl(nla_get_be32(tb[NFTA_WMM_MARK_MARKPOS]));
	}

	if (tb[NFTA_WMM_MARK_MARKSET])
	{
		priv->markset = ntohl(nla_get_be32(tb[NFTA_WMM_MARK_MARKSET]));
	}

	return 0;
}

static int nft_wmm_mark_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	const struct nft_wmm_mark *priv = nft_expr_priv(expr);

	if (nla_put_be32(skb, NFTA_WMM_MARK_MARKPOS, htonl(priv->markpos)))
		return -1;

	if (nla_put_be32(skb, NFTA_WMM_MARK_MARKSET, htonl(priv->markset)))
		return -1;

	return 0;
}

static struct nft_expr_type nft_wmm_mark_type;
static const struct nft_expr_ops nft_wmm_mark_op = {
	.eval = nft_wmm_mark_eval,
	.size = NFT_EXPR_SIZE(sizeof(struct nft_wmm_mark)),
	.init = nft_wmm_mark_init,
	.dump = nft_wmm_mark_dump,
	.type = &nft_wmm_mark_type,
};

static struct nft_expr_type nft_wmm_mark_type __read_mostly =  {
	.ops = &nft_wmm_mark_op,
	.name = "wmm_mark",
	.owner = THIS_MODULE,
	.policy = nft_wmm_mark_policy,
	.maxattr = NFTA_WMM_MARK_MAX,
};

static int __init nft_wmm_mark_module_init(void)
{       
	return nft_register_expr(&nft_wmm_mark_type);
}

static void __exit nft_wmm_mark_module_exit(void)
{
	nft_unregister_expr(&nft_wmm_mark_type);
}

module_init(nft_wmm_mark_module_init);
module_exit(nft_wmm_mark_module_exit);

