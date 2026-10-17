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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft: mirrored stream classification service");
MODULE_ALIAS("nft_mscs");

#if 0
#define DEBUG_MSCS(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_MSCS(fmt, ...) {}
#endif

enum nft_mscs_attributes {
	NFTA_MSCS_UNSPEC,
	NFTA_MSCS_UP_BITMAP,
	NFTA_MSCS_UP_LIMIT,
	NFTA_MSCS_IS_GLOBAL,
	__NFTA_MSCS_MAX,
};
#define NFTA_MSCS_MAX (__NFTA_MSCS_MAX - 1)

struct nft_mscs {
	uint8_t up_bitmap;
	uint8_t up_limit;
	uint8_t is_global;

	/* Used internally by the kernel */
	void *data __attribute__((aligned(8)));	
};

struct nft_mscs_data {
    struct list_head    head;
    spinlock_t          lock;
};

static const struct nla_policy nft_mscs_policy[NFTA_MSCS_MAX + 1] = {
	[NFTA_MSCS_UP_BITMAP]	= { .type = NLA_U8 },
	[NFTA_MSCS_UP_LIMIT]	= { .type = NLA_U8 },
	[NFTA_MSCS_IS_GLOBAL]	= { .type = NLA_U8 },
};

static void 
nft_mscs_flow_flush(struct nf_conn *ct)
{
#if defined(CONFIG_BLOG)
	blog_lock();
	if (ct->bcm_ext.blog_key[ct->bcm_ext.mscs.dir] != BLOG_KEY_FC_INVALID) {
		/* remove flow from flow cache if exists */
		blog_notify(DESTROY_FLOWTRACK, (void*)ct,
		            (uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
		            (uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);
	}
	blog_unlock();
#endif
}

static void nft_mscs_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct nft_mscs *priv = nft_expr_priv(expr);
	struct nft_mscs_data *data = priv->data;
	struct sk_buff *skb = pkt->skb;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	struct nf_mscs *ct_mscs;

	//DEBUG_MSCS("skb=%8p, priv=%8p, up_bitmap=0x%02x, up_limit=%d, is_global=%d.\n",
	//            skb, priv, priv->up_bitmap, priv->up_limit, priv->is_global);

	ct = nf_ct_get(skb, &ctinfo);
	if (unlikely(!ct))
	{
		regs->verdict.code = NFT_BREAK;
		return;
	}
	ct_mscs = &ct->bcm_ext.mscs;
	
	if (priv->is_global) {
		if (!CT_MSCS_INITED(ct))
		{
			regs->verdict.code = NFT_BREAK;
			return;
		}
		if (ct_mscs->dir==CTINFO2DIR(ctinfo)) { // global-DS: do mirroring
			struct sk_buff *s = (struct sk_buff*)skb;
			SET_WLAN_PRIORITY_OVRD(s, 1);
			s->mark = SKBMARK_SET_Q_PRIO(skb->mark, ct_mscs->priority);

			DEBUG_MSCS("gd>> ct=%8p skb=%8p pri=%x mark=%x mscs.dir=%x\n", ct, skb, skb->priority, skb->mark, ct_mscs->dir);
		} else {	// global-US: update priority if necessary
			uint32_t priority = GET_WLAN_PRIORITY(skb->priority);

			if (!(ct_mscs->up_bitmap & (1<<priority)))
			{
				regs->verdict.code = NFT_CONTINUE;
				return;
			}

			priority = (priority < ct_mscs->up_limit) ? priority : ct_mscs->up_limit;
			if (ct_mscs->priority != priority) {
				ct_mscs->priority = priority;
				nft_mscs_flow_flush(ct);
				DEBUG_MSCS("gu>> ct=%8p skb=%8p update pri=%d\n", ct, skb, ct_mscs->priority);
			}
		}
	} else if (!CT_MSCS_INITED(ct)) {  // per-rule-US: matching
		uint32_t priority = GET_WLAN_PRIORITY(skb->priority);

		if (!(priv->up_bitmap & (1<<priority)))
		{
			//DEBUG_MSCS("up_bitmap=0x%02x priority=%d\n", priv->up_bitmap, priority);
			regs->verdict.code = NFT_BREAK;
			return;
		}

		priority = (priority < priv->up_limit) ? priority : priv->up_limit;			
		ct_mscs->dir = (CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL) ? IP_CT_DIR_ORIGINAL : IP_CT_DIR_REPLY;  // direction to apply mirrored priority is reversed
		ct_mscs->priority = priority;
		ct_mscs->up_limit = priv->up_limit;
		ct_mscs->up_bitmap = priv->up_bitmap;
		ct_mscs->entry.next = ct_mscs->entry.prev = &ct_mscs->entry;

		spin_lock_bh(&data->lock);
		ct_mscs->lock = &data->lock;
		list_add_tail(&ct_mscs->entry, &data->head);
		spin_unlock_bh(&data->lock);
		
		nft_mscs_flow_flush(ct);
		DEBUG_MSCS("iu<< add ct=%8p skb=%8p pri=%x mark=%x up_limit=%d mscs.dir=%x pri=%x\n", ct, skb,
		            skb->priority, skb->mark, priv->up_limit, ct_mscs->dir, ct_mscs->priority);
	}

	regs->verdict.code = NFT_CONTINUE;
	return;	
}

static int nft_mscs_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_mscs *priv = nft_expr_priv(expr);
	struct nft_mscs_data *data;

	priv->up_bitmap = 0;
	priv->up_limit = 0;
	priv->is_global = 0;

	if (tb[NFTA_MSCS_UP_BITMAP])
	{
		priv->up_bitmap = nla_get_u8(tb[NFTA_MSCS_UP_BITMAP]);
	}

	if (tb[NFTA_MSCS_UP_LIMIT])
	{
		priv->up_limit = nla_get_u8(tb[NFTA_MSCS_UP_LIMIT]);
	}

	if (tb[NFTA_MSCS_IS_GLOBAL])
	{
		priv->is_global = nla_get_u8(tb[NFTA_MSCS_IS_GLOBAL]);
	}

	DEBUG_MSCS("up_bitmap=0x%02x, up_limit=%d, is_global=%d.\n",
	            priv->up_bitmap, priv->up_limit, priv->is_global);

	/* ct linked list is for per station rules */
	if (priv->is_global)
	{
		return 0;
	}

	/* init private data */
	data = kmalloc(sizeof(*data), GFP_KERNEL);
	priv->data = data;
	if (data) {
		data->head.next = data->head.prev = &data->head;
		spin_lock_init(&data->lock);
	}

	return PTR_ERR_OR_ZERO(data);

}

static void nft_mscs_destroy(const struct nft_ctx *ctx,	const struct nft_expr *expr)
{
	struct nft_mscs *priv = nft_expr_priv(expr);
	struct nft_mscs_data *data = priv->data;

	/* ct linked list is for per station rules */
	if (priv->is_global) return;

	DEBUG_MSCS("priv=%8p, up_bitmap=0x%02x, up_limit=%d, is_global=%d.\n",
	            priv, priv->up_bitmap, priv->up_limit, priv->is_global);

	/* traverse list to deactivate entry, flush flow */
	if (!data)
	{
		return;
	}

	spin_lock_bh(&data->lock);
	while (!list_empty(&data->head)) {
		struct nf_conn *ct;

		ct = container_of(data->head.next, struct nf_conn, bcm_ext.mscs.entry);
		list_del(&ct->bcm_ext.mscs.entry);
		ct->bcm_ext.mscs.lock = NULL;

		spin_unlock_bh(&data->lock);
		nft_mscs_flow_flush(ct);
		DEBUG_MSCS("remove ct=%8p\n", ct);
		spin_lock_bh(&data->lock);
	}
	spin_unlock_bh(&data->lock);
	
	priv->data = NULL;
	kfree(data);
}

static int nft_mscs_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	const struct nft_mscs *priv = nft_expr_priv(expr);

	if (nla_put_u8(skb, NFTA_MSCS_UP_BITMAP, priv->up_bitmap))
		return -1;

	if (nla_put_u8(skb, NFTA_MSCS_UP_LIMIT, priv->up_limit))
		return -1;

	if (nla_put_u8(skb, NFTA_MSCS_IS_GLOBAL, priv->is_global))
		return -1;

	DEBUG_MSCS("up_bitmap=0x%02x, up_limit=%d, is_global=%d.\n",
	            priv->up_bitmap, priv->up_limit, priv->is_global);

	return 0;
}

static struct nft_expr_type nft_mscs_type;
static const struct nft_expr_ops nft_mscs_op = {
	.eval    = nft_mscs_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_mscs)),
	.init    = nft_mscs_init,
	.destroy = nft_mscs_destroy,
	.dump    = nft_mscs_dump,
	.type    = &nft_mscs_type,
};

static struct nft_expr_type nft_mscs_type __read_mostly =  {
	.ops = &nft_mscs_op,
	.name = "mscs",
	.owner = THIS_MODULE,
	.policy = nft_mscs_policy,
	.maxattr = NFTA_MSCS_MAX,
};

static int __init nft_mscs_module_init(void)
{       
	return nft_register_expr(&nft_mscs_type);
}

static void __exit nft_mscs_module_exit(void)
{
	nft_unregister_expr(&nft_mscs_type);
}

module_init(nft_mscs_module_init);
module_exit(nft_mscs_module_exit);

