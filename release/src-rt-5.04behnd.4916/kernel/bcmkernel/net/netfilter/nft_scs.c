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
MODULE_DESCRIPTION("nft: stream classification service");
MODULE_ALIAS("nft_scs");

#if 0
#define DEBUG_SCS(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_SCS(fmt, ...) {}
#endif

enum nft_scs_attributes {
	NFTA_SCS_UNSPEC,
	NFTA_SCS_UP_OVRD,
	NFTA_SCS_UP_VAL,
	__NFTA_SCS_MAX,
};
#define NFTA_SCS_MAX (__NFTA_SCS_MAX - 1)
#define SCS_CMD_INVALID 255

struct nft_scs {
	uint8_t up_ovrd;
	uint8_t up_ovrd_invert;
	uint8_t up_val;
	
	/* Used internally by the kernel */
	void *data __attribute__((aligned(8)));	
};

struct nft_scs_data {
    struct list_head    head;
    spinlock_t          lock;
};

static const struct nla_policy nft_scs_policy[NFTA_SCS_MAX + 1] = {
	[NFTA_SCS_UP_OVRD]	= { .type = NLA_U8 },
	[NFTA_SCS_UP_VAL]	= { .type = NLA_U8 },
};

static void 
scs_flow_flush(struct nf_conn *ct)
{
#if defined(CONFIG_BLOG)
	blog_lock();
	/* remove flow from flow cache if exists */
    if ((ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID)
	    || (ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID)) {
        blog_notify(DESTROY_FLOWTRACK, (void*)ct,
                            (uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
                            (uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);
    }
	blog_unlock();
#endif
}

static void nft_scs_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct nft_scs *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;
	struct nft_scs_data *data = priv->data;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;

	// DEBUG_SCS("skb=%8p, priv=%8p, up_ovrd=0x%02x, up_ovrd_invert=0x%02x, up_val=%d.\n",
	//            skb, priv, priv->up_ovrd, priv->up_ovrd_invert, priv->up_val);

	if (priv->up_ovrd != SCS_CMD_INVALID) {
		if (GET_WLAN_PRIORITY_OVRD(skb) && !priv->up_ovrd_invert) {
			regs->verdict.code = NFT_CONTINUE;		
		}    
		else if (!GET_WLAN_PRIORITY_OVRD(skb) && priv->up_ovrd_invert) {
			regs->verdict.code = NFT_CONTINUE;
	  	}
		else {
			regs->verdict.code = NFT_BREAK;
		}    
		
	}
	
	if (priv->up_val != SCS_CMD_INVALID) {
		SET_WLAN_PRIORITY_OVRD(skb, 1);
		skb->mark = SKBMARK_SET_Q_PRIO(skb->mark, priv->up_val);
	        
		ct = nf_ct_get(skb, &ctinfo);
	      	
		if (ct && !CT_SCS_INITED(ct)) {
			struct nf_scs *ct_scs = &ct->bcm_ext.scs;
			ct_scs->entry.next = ct_scs->entry.prev = &ct_scs->entry;

			spin_lock_bh(&data->lock);
			ct_scs->lock = &data->lock;
			list_add_tail(&ct_scs->entry, &data->head);
			spin_unlock_bh(&data->lock);
			DEBUG_SCS("add ct=%8p\n", ct);
	        }
		regs->verdict.code = NFT_CONTINUE;
	  }

	return;	
}

static int nft_scs_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_scs *priv = nft_expr_priv(expr);
	struct nft_scs_data *data;

	priv->up_ovrd = SCS_CMD_INVALID;
	priv->up_ovrd_invert = 0;
	priv->up_val = SCS_CMD_INVALID;

	if (tb[NFTA_SCS_UP_OVRD])
	{
		priv->up_ovrd = nla_get_u8(tb[NFTA_SCS_UP_OVRD]);
		if(priv->up_ovrd == 0)
			priv->up_ovrd_invert = 1;
	}

	if (tb[NFTA_SCS_UP_VAL])
	{
		priv->up_val = nla_get_u8(tb[NFTA_SCS_UP_VAL]);
	}

	// DEBUG_SCS("up_ovrd=0x%02x, up_val=%d .\n",
	//            priv->up_ovrd, priv->up_val);

	if (priv->up_val != SCS_CMD_INVALID) {
		/* init private data */
		data = kmalloc(sizeof(*data), GFP_KERNEL);
		priv->data = data;
		if (data) {
			data->head.next = data->head.prev = &data->head;
			spin_lock_init(&data->lock);
		}

		return PTR_ERR_OR_ZERO(data);
	}
	else
		return 0;
}

static void nft_scs_destroy(const struct nft_ctx *ctx,	const struct nft_expr *expr)
{
	struct nft_scs *priv = nft_expr_priv(expr);
	struct nft_scs_data *data = priv->data;

	// DEBUG_SCS("up_ovrd=0x%02x, up_val=%d .\n",
	//            priv->up_ovrd, priv->up_val);

	if (priv->up_val == SCS_CMD_INVALID) return;

	/* traverse list to deactivate entry, flush flow */
	if (!data)
	{
		return;
	}

	spin_lock_bh(&data->lock);
	while (!list_empty(&data->head)) {
		struct nf_conn *ct;

		ct = container_of(data->head.next, struct nf_conn, bcm_ext.scs.entry);
		list_del(&ct->bcm_ext.scs.entry);
		ct->bcm_ext.scs.lock = NULL;

		spin_unlock_bh(&data->lock);
		scs_flow_flush(ct);
		DEBUG_SCS("remove ct=%8p\n", ct);
		spin_lock_bh(&data->lock);
	}
	spin_unlock_bh(&data->lock);

	priv->data = NULL;
	kfree(data);
}

static int nft_scs_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	const struct nft_scs *priv = nft_expr_priv(expr);

	if (nla_put_u8(skb, NFTA_SCS_UP_OVRD, priv->up_ovrd))
		return -1;

	if (nla_put_u8(skb, NFTA_SCS_UP_VAL, priv->up_val))
		return -1;

	DEBUG_SCS("up_ovrd=0x%02x, up_val=%d.\n",
	            priv->up_ovrd, priv->up_val);

	return 0;
}

static struct nft_expr_type nft_scs_type;
static const struct nft_expr_ops nft_scs_op = {
	.eval    = nft_scs_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_scs)),
	.init    = nft_scs_init,
	.destroy = nft_scs_destroy,
	.dump    = nft_scs_dump,
	.type    = &nft_scs_type,
};

static struct nft_expr_type nft_scs_type __read_mostly =  {
	.ops = &nft_scs_op,
	.name = "scs",
	.owner = THIS_MODULE,
	.policy = nft_scs_policy,
	.maxattr = NFTA_SCS_MAX,
};

static int __init nft_scs_module_init(void)
{       
	return nft_register_expr(&nft_scs_type);
}

static void __exit nft_scs_module_exit(void)
{
	nft_unregister_expr(&nft_scs_type);
}

module_init(nft_scs_module_init);
module_exit(nft_scs_module_exit);

