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
#include <linux/tcp.h>

#include <net/netfilter/nf_conntrack.h>

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include <linux/bcm_skb_defines.h>
#include <linux/netfilter/xt_FSMARK_sh.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft: flow stat operations");
MODULE_ALIAS("nft_fsmark");

enum nft_fsmark_attributes {
	NFTA_FSMARK_UNSPEC,
	NFTA_FSMARK_ID,
	__NFTA_FSMARK_MAX,
};
#define NFTA_FSMARK_MAX (__NFTA_FSMARK_MAX - 1)
#define FSMAKR_CMD_INVALID 255

struct nft_fsmark {
	uint16_t id;
	
	/* Used internally by the kernel */
	void *data __attribute__((aligned(16)));	
};

static const struct nla_policy nft_fsmark_policy[NFTA_FSMARK_MAX + 1] = {
	[NFTA_FSMARK_ID]	= { .type = NLA_U16 },
};

static void nft_fsmark_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct nft_fsmark *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;

	//DEBUG_FSMARK("SKB=%px id%d\n", skb, priv->id);
	fsmark_update(skb, priv->id);
	regs->verdict.code = NFT_CONTINUE;

	return;	
}

static int nft_fsmark_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_fsmark *priv = nft_expr_priv(expr);

	priv->id = FSMAKR_CMD_INVALID;

	if (tb[NFTA_FSMARK_ID])
	{
		priv->id = nla_get_u16(tb[NFTA_FSMARK_ID]);
	}

	return 0;
}

static int nft_fsmark_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	const struct nft_fsmark *priv = nft_expr_priv(expr);

	if (nla_put_u16(skb, NFTA_FSMARK_ID, priv->id))
		return -1;

	//DEBUG_FSMARK("id=0x%02x\n", priv->id);

	return 0;
}

static struct nft_expr_type nft_fsmark_type;
static const struct nft_expr_ops nft_fsmark_op = {
	.eval    = nft_fsmark_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_fsmark)),
	.init    = nft_fsmark_init,
	.dump    = nft_fsmark_dump,
	.type    = &nft_fsmark_type,
};

static struct nft_expr_type nft_fsmark_type __read_mostly =  {
	.ops = &nft_fsmark_op,
	.name = "fsmark",
	.owner = THIS_MODULE,
	.policy = nft_fsmark_policy,
	.maxattr = NFTA_FSMARK_MAX,
};

static int __init nft_fsmark_module_init(void)
{       
	return nft_register_expr(&nft_fsmark_type);
}

static void __exit nft_fsmark_module_exit(void)
{
	nft_unregister_expr(&nft_fsmark_type);
}

module_init(nft_fsmark_module_init);
module_exit(nft_fsmark_module_exit);

