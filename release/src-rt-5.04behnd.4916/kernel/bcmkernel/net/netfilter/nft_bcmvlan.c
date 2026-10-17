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
#include <linux/if_ether.h>
#include <linux/if_vlan.h> 

#include "bcm_vlan_defs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft: Broadcom VLAN header match support");
MODULE_ALIAS("nft_bcmvlan");

#if 0
#define DEBUG_BCMVLAN(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_BCMVLAN(fmt, ...) {}
#endif

#define GET_SKBVLAN_ID(VTAG)   (VTAG & VLAN_VID_MASK)
#define GET_SKBVLAN_PRIO(VTAG) ((VTAG & VLAN_PRIO_MASK) >> VLAN_PRIO_SHIFT)
#define GET_SKBVLAN_PRIO(VTAG) ((VTAG & VLAN_PRIO_MASK) >> VLAN_PRIO_SHIFT)
#define GET_BITMASK(_BIT_MASK_) priv->bitmask & _BIT_MASK_

#define NFT_TPID_MASK 0xFFFF0000

/* Test a struct->invflags and a boolean for inequality */
#define NF_INVF(ptr,flag,boolean)					\
	((boolean) ^ !!((ptr)->invflags & (flag)))

#define EXIT_ON_MISMATCH(_MATCH_,_MASK_) \
{ \
	if(!vlanCount) \
	{ \
		regs->verdict.code = NFT_BREAK; \
		return; \
	} \
	if (!((priv->_MATCH_ == _MATCH_)^!!(priv->invflags & _MASK_))) \
	{ \
		regs->verdict.code = NFT_BREAK; \
		return; \
	} \
}

#define EXIT_ON_MISMATCH_VLAN(MATCH,FLAG,INST) \
{ \
	if (priv->MATCH##_mask >= NFT_TPID_MASK) \
	{ \
		find = nft_bcmvlan_get_ext_vlan_by_type(skb, priv->MATCH[0] & NFT_TPID_MASK, INST, &extVlan); \
		if (find) \
		{ \
			if (NF_INVF(priv, FLAG, (extVlan & priv->MATCH##_mask) < (priv->MATCH[0] & priv->MATCH##_mask) || \
				(extVlan & priv->MATCH##_mask) > (priv->MATCH[1] & priv->MATCH##_mask))) \
			{ \
				regs->verdict.code = NFT_BREAK; \
				return; \
			} \
		} \
		else \
		{ \
			if (!((priv->invflags & FLAG) && \
				(priv->MATCH##_mask == NFT_TPID_MASK) && \
				((priv->MATCH[0] & VLAN_VID_MASK) == 0))) \
			{ \
				regs->verdict.code = NFT_BREAK; \
				return; \
			} \
		} \
	} \
	else \
	{ \
		extVlan = (skbuff_bcm_ext_vlan_get(skb, vlan_tpid) << 16) | (skbuff_bcm_ext_vlan_get(skb, vlan_header)[0] >> 16); \
		if (NF_INVF(priv,FLAG,(extVlan & priv->MATCH##_mask) < (priv->MATCH[0] & priv->MATCH##_mask) || \
				(extVlan & priv->MATCH##_mask) > (priv->MATCH[1] & priv->MATCH##_mask))) \
		{ \
			regs->verdict.code = NFT_BREAK; \
			return; \
		} \
	} \
}

enum nft_bcmvlan_flags {
	NFT_BCMVLAN_ID          = (1 << 0),
	NFT_BCMVLAN_PRIO        = (1 << 1),
	NFT_BCMVLAN_ENCAP       = (1 << 2),
	NFT_BCMVLAN_SVLAN0      = (1 << 3),
	NFT_BCMVLAN_SVLAN1      = (1 << 4),
	NFT_BCMVLAN_CVLAN0      = (1 << 5),
	NFT_BCMVLAN_CVLAN1      = (1 << 6),
	NFT_BCMVLAN_MASK        = ( NFT_BCMVLAN_ID    | NFT_BCMVLAN_PRIO   | \
                                NFT_BCMVLAN_ENCAP | NFT_BCMVLAN_SVLAN0 | \
                                NFT_BCMVLAN_SVLAN1| NFT_BCMVLAN_CVLAN0 | NFT_BCMVLAN_CVLAN1),
};

enum nft_bcmvlan_attributes {
	NFTA_BCMVLAN_UNSPEC,
	NFTA_BCMVLAN_ID,
	NFTA_BCMVLAN_PRIO,
	NFTA_BCMVLAN_ENCAP,
	NFTA_BCMVLAN_BITMASK,
	NFTA_BCMVLAN_INVFLAGS,
	NFTA_BCMVLAN_SVLAN0,
	NFTA_BCMVLAN_SVLAN0_MASK,
	NFTA_BCMVLAN_SVLAN1,
	NFTA_BCMVLAN_SVLAN1_MASK,
	NFTA_BCMVLAN_CVLAN0,
	NFTA_BCMVLAN_CVLAN0_MASK,
	NFTA_BCMVLAN_CVLAN1,
	NFTA_BCMVLAN_CVLAN1_MASK,
	__NFTA_BCMVLAN_MAX,
};

#define NFTA_BCMVLAN_MAX (__NFTA_BCMVLAN_MAX - 1)

struct nft_bcmvlan {
	uint16_t id;
	uint8_t  prio;
	uint16_t encap;
	uint16_t bitmask;
	uint16_t invflags;
	uint32_t svlan0[2];
	uint32_t svlan0_mask;
	uint32_t svlan1[2];
	uint32_t svlan1_mask;
	uint32_t cvlan0[2];
	uint32_t cvlan0_mask;
	uint32_t cvlan1[2];
	uint32_t cvlan1_mask;
};

static const struct nla_policy nft_bcmvlan_policy[NFTA_BCMVLAN_MAX + 1] = {
	[NFTA_BCMVLAN_ID]           = { .type = NLA_U16 },    
	[NFTA_BCMVLAN_PRIO]         = { .type = NLA_U8 },
	[NFTA_BCMVLAN_ENCAP]        = { .type = NLA_U16 },    
	[NFTA_BCMVLAN_BITMASK]      = { .type = NLA_U16 },
	[NFTA_BCMVLAN_INVFLAGS]     = { .type = NLA_U16 },
	[NFTA_BCMVLAN_SVLAN0]       = { .len =  2*sizeof(uint32_t)},
	[NFTA_BCMVLAN_SVLAN0_MASK]  = { .type = NLA_U32},
	[NFTA_BCMVLAN_SVLAN1]       = { .len =  2*sizeof(uint32_t)},
	[NFTA_BCMVLAN_SVLAN1_MASK]  = { .type = NLA_U32},
	[NFTA_BCMVLAN_CVLAN0]       = { .len =  2*sizeof(uint32_t)},
	[NFTA_BCMVLAN_CVLAN0_MASK]  = { .type = NLA_U32},
	[NFTA_BCMVLAN_CVLAN1]       = { .len =  2*sizeof(uint32_t)},
	[NFTA_BCMVLAN_CVLAN1_MASK]  = { .type = NLA_U32},
};

static bool nft_bcmvlan_get_ext_vlan_by_type(const struct sk_buff *skb, __u32 tpid, __u8 layer, __u32 *ext_vlan)
{
	int i;
	__u32 vlantag;
	__u8 count = 0;
	
	for (i = 0; (i < skbuff_bcm_ext_vlan_get(skb, vlan_count)) && (i < SKB_VLAN_MAX_TAGS); i++)
	{
		if (i == 0)
		{
		    vlantag = (skbuff_bcm_ext_vlan_get(skb, vlan_tpid) << 16) | (skbuff_bcm_ext_vlan_get(skb, vlan_header)[0] >> 16);
 	 	}
		else
		{
 	 	 	vlantag = (skbuff_bcm_ext_vlan_get(skb, vlan_header)[i - 1] << 16) | (skbuff_bcm_ext_vlan_get(skb, vlan_header)[i] >> 16);
 	 	}
        
		if ((vlantag & NFT_TPID_MASK) == tpid)
		{
			if (count == layer)
			{
				*ext_vlan = vlantag;
				return true;
			}
 	 	 	count++;
		}
	}

	return false;
}

/* get svlan0[0] or svlan0[1]*/
static inline u32 nla_bcmvlan_get_tag(const struct nlattr *nla, uint8_t num)
{
	if (!num)
		return *(u32 *) nla_data(nla);
	else
		return *(u32 *) (nla_data(nla) + sizeof(u32));
}

static void nft_bcmvlan_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt)
{
	struct nft_bcmvlan *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;
	uint16_t  id        = 0; 
	uint8_t   prio      = 0;
	__be16    encap     = 0;
	uint32_t  extVlan   = 0;
	uint8_t   vlanCount = 0;
	bool find = true;
        
	if (skb == NULL)
	{
		regs->verdict.code = NFT_BREAK;
		return;
	}

	vlanCount = skbuff_bcm_ext_vlan_get(skb, vlan_count);

	if (vlanCount)
	{
		id = GET_SKBVLAN_ID(skbuff_bcm_ext_vlan_get(skb, vlan_header)[0] >> 16);
		prio = GET_SKBVLAN_PRIO(skbuff_bcm_ext_vlan_get(skb, vlan_header)[0] >> 16);
		encap = skbuff_bcm_ext_vlan_get(skb, vlan_header)[0]; 
	}
    
	if (GET_BITMASK(NFT_BCMVLAN_PRIO))
		EXIT_ON_MISMATCH(prio,NFT_BCMVLAN_PRIO);

	/* Checking VLAN Identifier (VID) */
	if (GET_BITMASK(NFT_BCMVLAN_ID))
		EXIT_ON_MISMATCH(id,NFT_BCMVLAN_ID);

	/* Checking Encapsulated Proto (Length/Type) field */
	if (GET_BITMASK(NFT_BCMVLAN_ENCAP))
		EXIT_ON_MISMATCH(encap,NFT_BCMVLAN_ENCAP);

	if (GET_BITMASK(NFT_BCMVLAN_SVLAN0))
		EXIT_ON_MISMATCH_VLAN(svlan0,NFT_BCMVLAN_SVLAN0,0);

	if (GET_BITMASK(NFT_BCMVLAN_SVLAN1))
		EXIT_ON_MISMATCH_VLAN(svlan1,NFT_BCMVLAN_SVLAN1,1);

	if (GET_BITMASK(NFT_BCMVLAN_CVLAN0))
		EXIT_ON_MISMATCH_VLAN(cvlan0,NFT_BCMVLAN_CVLAN0,0);

	if (GET_BITMASK(NFT_BCMVLAN_CVLAN0))
		EXIT_ON_MISMATCH_VLAN(cvlan1,NFT_BCMVLAN_CVLAN1,1);

	regs->verdict.code = NFT_CONTINUE;
	return;
}

static int nft_bcmvlan_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[])
{
	struct nft_bcmvlan *priv = nft_expr_priv(expr);
    
	memset(priv, 0, sizeof(struct nft_bcmvlan));

	if (tb[NFTA_BCMVLAN_ID])
	{
		priv->id = nla_get_u16(tb[NFTA_BCMVLAN_ID]);
	}
   
	if (tb[NFTA_BCMVLAN_PRIO])
	{
		priv->prio = nla_get_u8(tb[NFTA_BCMVLAN_PRIO]);
	}

	if (tb[NFTA_BCMVLAN_ENCAP])
	{
		priv->encap = nla_get_u16(tb[NFTA_BCMVLAN_ENCAP]);
	}

	if (tb[NFTA_BCMVLAN_BITMASK])
	{
		priv->bitmask = nla_get_u16(tb[NFTA_BCMVLAN_BITMASK]);
	}

	if (tb[NFTA_BCMVLAN_INVFLAGS])
	{
		priv->invflags = nla_get_u16(tb[NFTA_BCMVLAN_INVFLAGS]);
	}

	if (tb[NFTA_BCMVLAN_SVLAN0])
	{    
		priv->svlan0[0] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_SVLAN0], 0);
		priv->svlan0[1] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_SVLAN0], 1);
	}
    
	if (tb[NFTA_BCMVLAN_SVLAN0_MASK])
	{
		priv->svlan0_mask = nla_get_u32(tb[NFTA_BCMVLAN_SVLAN0_MASK]);
	}

	if (tb[NFTA_BCMVLAN_SVLAN1])
	{    
		priv->svlan1[0] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_SVLAN1], 0);
		priv->svlan1[1] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_SVLAN1], 1);
	}

	if (tb[NFTA_BCMVLAN_SVLAN1_MASK])
	{
		priv->svlan1_mask = nla_get_u32(tb[NFTA_BCMVLAN_SVLAN1_MASK]);
	}

	if (tb[NFTA_BCMVLAN_CVLAN0])
	{    
		priv->cvlan0[0] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_CVLAN0], 0);
		priv->cvlan0[1] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_CVLAN0], 1);
	}
    
	if (tb[NFTA_BCMVLAN_CVLAN0_MASK])
	{
		priv->cvlan0_mask = nla_get_u32(tb[NFTA_BCMVLAN_CVLAN0_MASK]);
	}

	if (tb[NFTA_BCMVLAN_CVLAN1])
	{   
		priv->cvlan1[0] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_CVLAN1], 0);
		priv->cvlan1[1] = nla_bcmvlan_get_tag(tb[NFTA_BCMVLAN_CVLAN1], 1);
	}
    
	if (tb[NFTA_BCMVLAN_CVLAN1_MASK])
	{
		priv->cvlan1_mask = nla_get_u32(tb[NFTA_BCMVLAN_CVLAN1_MASK]);
	}

	return 0;
}

#define NFT_BCMVLAN_DUMP(VLAN,FLAG) \
{ \
	if (nla_put(skb, NFTA_BCMVLAN_##FLAG, sizeof(priv->VLAN), &priv->VLAN[0])) \
		return -1; \
	if (nla_put_u32(skb, NFTA_BCMVLAN_##FLAG##_MASK, priv->VLAN##_mask)) \
		return -1; \
}

static int nft_bcmvlan_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct nft_bcmvlan *priv = nft_expr_priv(expr);
    
	if (nla_put_u16(skb, NFTA_BCMVLAN_ID, priv->id))
		return -1;

	if (nla_put_u8(skb, NFTA_BCMVLAN_PRIO, priv->prio))
		return -1;

	if (nla_put_u16(skb, NFTA_BCMVLAN_ENCAP, priv->encap))
		return -1;

	if (nla_put_u16(skb, NFTA_BCMVLAN_BITMASK, priv->bitmask))
		return -1;

	if (nla_put_u16(skb, NFTA_BCMVLAN_INVFLAGS, priv->invflags))
		return -1;

	NFT_BCMVLAN_DUMP(svlan0,SVLAN0);
	NFT_BCMVLAN_DUMP(svlan1,SVLAN1);
	NFT_BCMVLAN_DUMP(cvlan0,CVLAN0);
	NFT_BCMVLAN_DUMP(cvlan1,CVLAN1);
    
	return 0;
}

static struct nft_expr_type nft_bcmvlan_type;
static const struct nft_expr_ops nft_bcmvlan_op = {
	.eval    = nft_bcmvlan_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_bcmvlan)),
	.init    = nft_bcmvlan_init,
	.dump    = nft_bcmvlan_dump,
	.type    = &nft_bcmvlan_type,
};

static struct nft_expr_type nft_bcmvlan_type __read_mostly =  {
	.ops = &nft_bcmvlan_op,
	.name = "bcmvlan",
	.owner = THIS_MODULE,
	.policy = nft_bcmvlan_policy,
	.maxattr = NFTA_BCMVLAN_MAX,
};

static int __init nft_bcmvlan_module_init(void)
{       
	return nft_register_expr(&nft_bcmvlan_type);
}

static void __exit nft_bcmvlan_module_exit(void)
{
	nft_unregister_expr(&nft_bcmvlan_type);
}

module_init(nft_bcmvlan_module_init);
module_exit(nft_bcmvlan_module_exit);

