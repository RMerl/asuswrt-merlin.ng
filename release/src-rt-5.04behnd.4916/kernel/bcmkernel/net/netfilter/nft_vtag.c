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
MODULE_DESCRIPTION("nft: vtag marking support");
MODULE_ALIAS("nft_vtag");

#if 0
#define DEBUG_VTAG(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_VTAG(fmt, ...) {}
#endif

enum nft_vtag_attributes {
	NFTA_VTAG_UNSPEC,
	NFTA_VTAG_PRIO,
	__NFTA_VTAG_MAX,
};
#define NFTA_VTAG_MAX (__NFTA_VTAG_MAX - 1)

struct nft_vtag {
	uint8_t prio;
};

static const struct nla_policy nft_vtag_policy[NFTA_VTAG_MAX + 1] = {
	[NFTA_VTAG_PRIO]     = { .type = NLA_U8 },
};

static void nft_vtag_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct nft_vtag *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;

	DEBUG_VTAG("skb=%8p, priv=%8p, prio=%u.\n", skb, priv, priv->prio);

	/* If the 802.1p priority is in the range of 0-7, we need to do p-bit marking. */
	if(priv->prio <= 7)
	{
		unsigned short vlan_tci = 0;
		
		/* If it's a VLAN frame, we want to re-mark its p-bit with the 802.1p priority
		 * from the vtag. If it's not a VLAN frame, we want to add an 802.1p tag to it,
		 * setting the vid to 0 and the p-bit to the 802.1p priority from the vtag.
		 */
		if (skb_vlan_tag_present(skb)) {
			DEBUG_VTAG("The VLAN tag is present. Remark its p-bit.\n");
			vlan_tci = skb->vlan_tci;
			vlan_tci = (vlan_tci & 0x1fff) | (priv->prio << 13);
			skb->vlan_tci = vlan_tci;
		}
		else {
			DEBUG_VTAG("The VLAN tag is not present. Add a VLAN tag with the configured p-bit.\n");
			vlan_tci = priv->prio << 13;
			if (skb_vlan_push(skb, htons(ETH_P_8021Q), vlan_tci)) {
				printk("nft_vtag_eval: Failed to push VLAN tag.\n");
			}
		}
	}
	regs->verdict.code = NFT_CONTINUE;
	return;
}

static int nft_vtag_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_vtag *priv = nft_expr_priv(expr);

	/* vtag prio marking ranges from 0-7. An initial value of 8 means to ignore marking. */
	priv->prio = 8;

	if (tb[NFTA_VTAG_PRIO])
	{
		priv->prio = nla_get_u8(tb[NFTA_VTAG_PRIO]);
	}

	DEBUG_VTAG("prio=%u.\n", priv->prio);

	return 0;

}

static int nft_vtag_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	const struct nft_vtag *priv = nft_expr_priv(expr);

	if (nla_put_u8(skb, NFTA_VTAG_PRIO, priv->prio))
		return -1;

	DEBUG_VTAG("prio=%u.\n", priv->prio);

	return 0;
}

static struct nft_expr_type nft_vtag_type;
static const struct nft_expr_ops nft_vtag_op = {
	.eval    = nft_vtag_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_vtag)),
	.init    = nft_vtag_init,
	.dump    = nft_vtag_dump,
	.type    = &nft_vtag_type,
};

static struct nft_expr_type nft_vtag_type __read_mostly =  {
	.ops = &nft_vtag_op,
	.name = "vtag",
	.owner = THIS_MODULE,
	.policy = nft_vtag_policy,
	.maxattr = NFTA_VTAG_MAX,
};

static int __init nft_vtag_module_init(void)
{       
	return nft_register_expr(&nft_vtag_type);
}

static void __exit nft_vtag_module_exit(void)
{
	nft_unregister_expr(&nft_vtag_type);
}

module_init(nft_vtag_module_init);
module_exit(nft_vtag_module_exit);

