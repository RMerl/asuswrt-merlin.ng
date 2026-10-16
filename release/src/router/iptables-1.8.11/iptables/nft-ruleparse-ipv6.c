/*
 * (C) 2012-2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Tomasz Bursztyka <tomasz.bursztyka@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip6.h>

#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "nft-shared.h"
#include "nft-ruleparse.h"
#include "xshared.h"

static void nft_ipv6_parse_meta(struct nft_xt_ctx *ctx,
				const struct nft_xt_ctx_reg *reg,
				struct nftnl_expr *e,
				struct iptables_command_state *cs)
{
	switch (reg->meta_dreg.key) {
	case NFT_META_L4PROTO:
		cs->fw6.ipv6.proto = nftnl_expr_get_u8(e, NFTNL_EXPR_CMP_DATA);
		if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
			cs->fw6.ipv6.invflags |= XT_INV_PROTO;
		return;
	default:
		break;
	}

	if (parse_meta(ctx, e, reg->meta_dreg.key, cs->fw6.ipv6.iniface,
		       cs->fw6.ipv6.outiface, &cs->fw6.ipv6.invflags) == 0)
		return;

	ctx->errmsg = "unknown ipv6 meta key";
}

static void parse_mask_ipv6(const struct nft_xt_ctx_reg *reg,
			    struct in6_addr *mask)
{
	memcpy(mask, reg->bitwise.mask, sizeof(struct in6_addr));
}

static void nft_ipv6_parse_payload(struct nft_xt_ctx *ctx,
				   const struct nft_xt_ctx_reg *reg,
				   struct nftnl_expr *e,
				   struct iptables_command_state *cs)
{
	struct in6_addr addr;
	uint8_t proto;
	bool inv;

	switch (reg->payload.offset) {
	case offsetof(struct ip6_hdr, ip6_src):
		get_cmp_data(e, &addr, sizeof(addr), &inv);
		memcpy(cs->fw6.ipv6.src.s6_addr, &addr, sizeof(addr));
		if (reg->bitwise.set)
			parse_mask_ipv6(reg, &cs->fw6.ipv6.smsk);
		else
			memset(&cs->fw6.ipv6.smsk, 0xff,
			       min(reg->payload.len, sizeof(struct in6_addr)));

		if (inv)
			cs->fw6.ipv6.invflags |= IP6T_INV_SRCIP;
		break;
	case offsetof(struct ip6_hdr, ip6_dst):
		get_cmp_data(e, &addr, sizeof(addr), &inv);
		memcpy(cs->fw6.ipv6.dst.s6_addr, &addr, sizeof(addr));
		if (reg->bitwise.set)
			parse_mask_ipv6(reg, &cs->fw6.ipv6.dmsk);
		else
			memset(&cs->fw6.ipv6.dmsk, 0xff,
			       min(reg->payload.len, sizeof(struct in6_addr)));

		if (inv)
			cs->fw6.ipv6.invflags |= IP6T_INV_DSTIP;
		break;
	case offsetof(struct ip6_hdr, ip6_nxt):
		get_cmp_data(e, &proto, sizeof(proto), &inv);
		cs->fw6.ipv6.proto = proto;
		if (inv)
			cs->fw6.ipv6.invflags |= IP6T_INV_PROTO;
	case offsetof(struct ip6_hdr, ip6_hlim):
		if (nft_parse_hl(ctx, e, cs) < 0)
			ctx->errmsg = "invalid ttl field match";
		break;
	default:
		DEBUGP("unknown payload offset %d\n", reg->payload.offset);
		ctx->errmsg = "unknown payload offset";
		break;
	}
}

struct nft_ruleparse_ops nft_ruleparse_ops_ipv6 = {
	.meta		= nft_ipv6_parse_meta,
	.payload	= nft_ipv6_parse_payload,
};
