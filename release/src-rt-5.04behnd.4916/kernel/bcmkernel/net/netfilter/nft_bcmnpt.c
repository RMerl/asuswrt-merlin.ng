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
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/x_tables.h>


#include <linux/bcm_skb_defines.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("NPTv6: NAT Prefix Translation for IPv6");
MODULE_ALIAS("nft_bcmnpt");


enum nft_npt_types {
		NFT_NPT_SNPT,
		NFT_NPT_DNPT,
	};

enum nft_bcmnpt_attributes {
	NFTA_BCMNPT_UNSPEC,
	NFTA_BCMNPT_TYPE,
	NFTA_BCMNPT_REG_SRC_PFX_ADDR_MIN,
	NFTA_BCMNPT_REG_SRC_PFX_ADDR_MAX,
	NFTA_BCMNPT_REG_DST_PFX_ADDR_MIN,
	NFTA_BCMNPT_REG_DST_PFX_ADDR_MAX,
	__NFTA_BCMNPT_MAX,
};

#define NFTA_BCMNPT_MAX (__NFTA_BCMNPT_MAX - 1)



struct nft_bcmnpt {
	u8			sreg_src_pfx_addr_min;
	u8			sreg_src_pfx_addr_max;
	u8			sreg_dst_pfx_addr_min;
	u8			sreg_dst_pfx_addr_max;
	u32         type;
};

struct nf_bcmnpt_range {
	union nf_inet_addr		min_addr;
	union nf_inet_addr		max_addr;
};

struct nf_bcmnpt_addr_info {
	struct nf_bcmnpt_range src_addr_range;
	struct nf_bcmnpt_range dst_addr_range;
};

static const struct nla_policy nft_bcmnpt_policy[NFTA_BCMNPT_MAX + 1] = {
	[NFTA_BCMNPT_TYPE]		 = { .type = NLA_U32 },
	[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MIN]	 = { .type = NLA_U32 },
	[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MAX]	 = { .type = NLA_U32 },
	[NFTA_BCMNPT_REG_DST_PFX_ADDR_MIN]	 = { .type = NLA_U32 },
	[NFTA_BCMNPT_REG_DST_PFX_ADDR_MAX]	 = { .type = NLA_U32 },
};

struct ip6t_npt_tginfo {
	union nf_inet_addr	src_pfx;
	union nf_inet_addr	dst_pfx;
	__u8			src_pfx_len;
	__u8			dst_pfx_len;
	/* Used internally by the kernel */
	__sum16			adjustment;
};

static bool ip6t_npt_map_pfx(const struct ip6t_npt_tginfo *npt,
			     struct in6_addr *addr)
{
	unsigned int pfx_len;
	unsigned int i, idx;
	__be32 mask;
	__sum16 sum;

	pfx_len = max(npt->src_pfx_len, npt->dst_pfx_len);
	for (i = 0; i < pfx_len; i += 32) {
		if (pfx_len - i >= 32)
			mask = 0;
		else
			mask = htonl((1 << (i - pfx_len + 32)) - 1);

		idx = i / 32;
		addr->s6_addr32[idx] &= mask;
		addr->s6_addr32[idx] |= ~mask & npt->dst_pfx.in6.s6_addr32[idx];
	}

	if (pfx_len <= 48)
		idx = 3;
	else {
		for (idx = 4; idx < ARRAY_SIZE(addr->s6_addr16); idx++) {
			if ((__force __sum16)addr->s6_addr16[idx] !=
			    CSUM_MANGLED_0)
				break;
		}
		if (idx == ARRAY_SIZE(addr->s6_addr16))
			return false;
	}

	sum = ~csum_fold(csum_add(csum_unfold((__force __sum16)addr->s6_addr16[idx]),
				  csum_unfold(npt->adjustment)));
	if (sum == CSUM_MANGLED_0)
		sum = 0;
	*(__force __sum16 *)&addr->s6_addr16[idx] = sum;

	return true;
}

static struct ipv6hdr *icmpv6_bounced_ipv6hdr(struct sk_buff *skb,
					      struct ipv6hdr *_bounced_hdr)
{
	if (ipv6_hdr(skb)->nexthdr != IPPROTO_ICMPV6)
		return NULL;

	if (!icmpv6_is_err(icmp6_hdr(skb)->icmp6_type))
		return NULL;

	return skb_header_pointer(skb,
				  skb_transport_offset(skb) + sizeof(struct icmp6hdr),
				  sizeof(struct ipv6hdr),
				  _bounced_hdr);
}

static unsigned int
ip6t_snpt_tg(struct sk_buff *skb, const struct ip6t_npt_tginfo *npt)
{
	struct ipv6hdr _bounced_hdr;
	struct ipv6hdr *bounced_hdr;
	struct in6_addr bounced_pfx;

	if (!ip6t_npt_map_pfx(npt, &ipv6_hdr(skb)->saddr)) {
		icmpv6_send(skb, ICMPV6_PARAMPROB, ICMPV6_HDR_FIELD,
			    offsetof(struct ipv6hdr, saddr));
		return NF_DROP;
	}

	/* rewrite dst addr of bounced packet which was sent to dst range */
	bounced_hdr = icmpv6_bounced_ipv6hdr(skb, &_bounced_hdr);
	if (bounced_hdr) {
		ipv6_addr_prefix(&bounced_pfx, &bounced_hdr->daddr, npt->src_pfx_len);
		if (ipv6_addr_cmp(&bounced_pfx, &npt->src_pfx.in6) == 0)
			ip6t_npt_map_pfx(npt, &bounced_hdr->daddr);
	}

	return NFT_CONTINUE;
}

static unsigned int
ip6t_dnpt_tg(struct sk_buff *skb, const struct ip6t_npt_tginfo *npt)
{
	struct ipv6hdr _bounced_hdr;
	struct ipv6hdr *bounced_hdr;
	struct in6_addr bounced_pfx;

	if (!ip6t_npt_map_pfx(npt, &ipv6_hdr(skb)->daddr)) {
		icmpv6_send(skb, ICMPV6_PARAMPROB, ICMPV6_HDR_FIELD,
			    offsetof(struct ipv6hdr, daddr));
		return NF_DROP;
	}

	/* rewrite src addr of bounced packet which was sent from dst range */
	bounced_hdr = icmpv6_bounced_ipv6hdr(skb, &_bounced_hdr);
	if (bounced_hdr) {
		ipv6_addr_prefix(&bounced_pfx, &bounced_hdr->saddr, npt->src_pfx_len);
		if (ipv6_addr_cmp(&bounced_pfx, &npt->src_pfx.in6) == 0)
			ip6t_npt_map_pfx(npt, &bounced_hdr->saddr);
	}

	return NFT_CONTINUE;
}


static void nft_bcmnpt_setup_addr(struct nf_bcmnpt_addr_info *addr_info,
			       const struct nft_regs *regs,
			       const struct nft_bcmnpt *priv)
{
	memcpy(addr_info->src_addr_range.min_addr.ip6, &regs->data[priv->sreg_src_pfx_addr_min],
		       sizeof(addr_info->src_addr_range.min_addr.ip6));
	memcpy(addr_info->src_addr_range.min_addr.ip6, &regs->data[priv->sreg_src_pfx_addr_max],
	       sizeof(addr_info->src_addr_range.min_addr.ip6));
	memcpy(addr_info->dst_addr_range.min_addr.ip6, &regs->data[priv->sreg_dst_pfx_addr_min],
		       sizeof(addr_info->dst_addr_range.min_addr.ip6));
	memcpy(addr_info->dst_addr_range.min_addr.ip6, &regs->data[priv->sreg_dst_pfx_addr_max],
	       sizeof(addr_info->dst_addr_range.min_addr.ip6));
}

static int nft_bcmnpt_prefix_len(struct nf_bcmnpt_range * range)
{
	int i;
	int j;
	__be32 netmask;
	int count = 0;
	int len = sizeof(struct in6_addr);

	for (i = 0; i < len / sizeof(__be32); i++) {
		netmask = ~(range->min_addr.ip6[i] ^ range->max_addr.ip6[i]);

		for (j = 0; j < 32; j++)
		{
			if (netmask & (1 << j))
				count++;
		}
	}

	return count;
}

static void nft_bcmnpt_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	const struct nft_bcmnpt *priv = nft_expr_priv(expr);
	struct sk_buff *skb = pkt->skb;
	struct nf_bcmnpt_addr_info addr;
	struct ip6t_npt_tginfo npt;
	__wsum src_sum, dst_sum;

	memset(&addr, 0, sizeof(addr));
	nft_bcmnpt_setup_addr(&addr, regs, priv);

	npt.src_pfx_len = nft_bcmnpt_prefix_len(&addr.src_addr_range);
	npt.dst_pfx_len = nft_bcmnpt_prefix_len(&addr.dst_addr_range);
	memcpy(&npt.src_pfx, &addr.src_addr_range.min_addr, sizeof(addr.src_addr_range.min_addr));
	memcpy(&npt.dst_pfx, &addr.dst_addr_range.min_addr, sizeof(addr.dst_addr_range.min_addr));

	src_sum = csum_partial(&npt.src_pfx.in6, sizeof(npt.src_pfx.in6), 0);
	dst_sum = csum_partial(&npt.dst_pfx.in6, sizeof(npt.dst_pfx.in6), 0);

	npt.adjustment = ~csum_fold(csum_sub(src_sum, dst_sum));

	regs->verdict.code = (priv->type == NFT_NPT_SNPT)?ip6t_snpt_tg(skb, &npt):ip6t_dnpt_tg(skb, &npt);
}

static int nft_bcmnpt_validate(const struct nft_ctx *ctx,
			    const struct nft_expr *expr,
			    const struct nft_data **data)
{
	struct nft_bcmnpt *priv = nft_expr_priv(expr);
	int err = 0;

	if (ctx->family != NFPROTO_IPV6)
		return -EOPNOTSUPP;

	switch (priv->type) {
	case NFT_NPT_SNPT:
		err = nft_chain_validate_hooks(ctx->chain,
					       (1 << NF_INET_POST_ROUTING) |
					       (1 << NF_INET_LOCAL_IN));
		break;
	case NFT_NPT_DNPT:
		err = nft_chain_validate_hooks(ctx->chain,
					       (1 << NF_INET_PRE_ROUTING) |
					       (1 << NF_INET_LOCAL_OUT));
		break;
	}

	return err;
}


static int nft_bcmnpt_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	struct nft_bcmnpt *priv = nft_expr_priv(expr);
	int err;
	unsigned int alen;

	if (tb[NFTA_BCMNPT_TYPE] == NULL ||
		(tb[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MIN] == NULL ||
		 tb[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MAX] == NULL)||
		(tb[NFTA_BCMNPT_REG_DST_PFX_ADDR_MIN] == NULL ||
		 tb[NFTA_BCMNPT_REG_DST_PFX_ADDR_MAX] == NULL))
		return -EINVAL;

	if (tb[NFTA_BCMNPT_TYPE]){
		switch (ntohl(nla_get_be32(tb[NFTA_BCMNPT_TYPE]))) {
		case NFT_NPT_SNPT:
			priv->type = NFT_NPT_SNPT;
			break;
		case NFT_NPT_DNPT:
			priv->type = NFT_NPT_DNPT;
			break;
		default:
			return -EOPNOTSUPP;
		}
	}

	alen = sizeof_field(struct nf_bcmnpt_range, min_addr.ip6);

	if (tb[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MIN]){
		err = nft_parse_register_load(tb[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MIN],
						  &priv->sreg_src_pfx_addr_min, alen);
		if (err < 0)
			return err;

		if (tb[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MAX]) {
			err = nft_parse_register_load(tb[NFTA_BCMNPT_REG_SRC_PFX_ADDR_MAX],
							  &priv->sreg_src_pfx_addr_max,
							  alen);
			if (err < 0)
				return err;
		} else {
			priv->sreg_src_pfx_addr_max = priv->sreg_src_pfx_addr_min;
		}
	}

	if (tb[NFTA_BCMNPT_REG_DST_PFX_ADDR_MIN]){
		err = nft_parse_register_load(tb[NFTA_BCMNPT_REG_DST_PFX_ADDR_MIN],
						  &priv->sreg_dst_pfx_addr_min, alen);
		if (err < 0)
			return err;

		if (tb[NFTA_BCMNPT_REG_DST_PFX_ADDR_MAX]) {
			err = nft_parse_register_load(tb[NFTA_BCMNPT_REG_DST_PFX_ADDR_MAX],
							  &priv->sreg_dst_pfx_addr_max,
							  alen);
			if (err < 0)
				return err;
		} else {
			priv->sreg_dst_pfx_addr_max = priv->sreg_dst_pfx_addr_min;
		}
	}

	return 0;

}

static int nft_bcmnpt_dump(struct sk_buff *skb, const struct nft_expr *expr) {
		const struct nft_bcmnpt *priv = nft_expr_priv(expr);
	
		if (nla_put_be32(skb, NFTA_BCMNPT_TYPE, htonl(priv->type)))
			goto nla_put_failure;
	
		if (priv->sreg_src_pfx_addr_min){
			if (nft_dump_register(skb, NFTA_BCMNPT_REG_SRC_PFX_ADDR_MIN,
						  priv->sreg_src_pfx_addr_min) ||
				nft_dump_register(skb, NFTA_BCMNPT_REG_SRC_PFX_ADDR_MAX,
						  priv->sreg_src_pfx_addr_max))
				goto nla_put_failure;
		}

		if (priv->sreg_dst_pfx_addr_min){
			if (nft_dump_register(skb, NFTA_BCMNPT_REG_DST_PFX_ADDR_MIN,
						  priv->sreg_dst_pfx_addr_min) ||
				nft_dump_register(skb, NFTA_BCMNPT_REG_DST_PFX_ADDR_MAX,
						  priv->sreg_dst_pfx_addr_max))
				goto nla_put_failure;
		}
	
		return 0;
	
	nla_put_failure:
		return -1;

}

static struct nft_expr_type nft_bcmnpt_type;
static const struct nft_expr_ops nft_bcmnpt_op = {
	.eval    = nft_bcmnpt_eval,
	.size    = NFT_EXPR_SIZE(sizeof(struct nft_bcmnpt)),
	.init    = nft_bcmnpt_init,
	.dump    = nft_bcmnpt_dump,
	.type    = &nft_bcmnpt_type,
	.validate	= nft_bcmnpt_validate,
};

static struct nft_expr_type nft_bcmnpt_type __read_mostly =  {
	.family	= NFPROTO_IPV6,
	.ops = &nft_bcmnpt_op,
	.name = "bcmnpt",
	.owner = THIS_MODULE,
	.policy = nft_bcmnpt_policy,
	.maxattr = NFTA_BCMNPT_MAX,
};

static int __init nft_bcmnpt_module_init(void)
{       
	return nft_register_expr(&nft_bcmnpt_type);
}

static void __exit nft_bcmnpt_module_exit(void)
{
	nft_unregister_expr(&nft_bcmnpt_type);
}

module_init(nft_bcmnpt_module_init);
module_exit(nft_bcmnpt_module_exit);

