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
#include <netinet/ip.h>

#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "nft-shared.h"
#include "nft-ruleparse.h"
#include "xshared.h"

static void nft_ipv4_parse_meta(struct nft_xt_ctx *ctx,
				const struct nft_xt_ctx_reg *reg,
				struct nftnl_expr *e,
				struct iptables_command_state *cs)
{
	switch (reg->meta_dreg.key) {
	case NFT_META_L4PROTO:
		cs->fw.ip.proto = nftnl_expr_get_u8(e, NFTNL_EXPR_CMP_DATA);
		if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
			cs->fw.ip.invflags |= XT_INV_PROTO;
		return;
	default:
		break;
	}

	if (parse_meta(ctx, e, reg->meta_dreg.key, cs->fw.ip.iniface,
		       cs->fw.ip.outiface, &cs->fw.ip.invflags) == 0)
		return;

	ctx->errmsg = "unknown ipv4 meta key";
}

static void parse_mask_ipv4(const struct nft_xt_ctx_reg *sreg, struct in_addr *mask)
{
	mask->s_addr = sreg->bitwise.mask[0];
}

static bool get_frag(const struct nft_xt_ctx_reg *reg, struct nftnl_expr *e)
{
	uint8_t op;

	/* we assume correct mask and xor */
	if (!reg->bitwise.set)
		return false;

	/* we assume correct data */
	op = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP);
	if (op == NFT_CMP_EQ)
		return true;

	return false;
}

static void nft_ipv4_parse_payload(struct nft_xt_ctx *ctx,
				   const struct nft_xt_ctx_reg *sreg,
				   struct nftnl_expr *e,
				   struct iptables_command_state *cs)
{
	struct in_addr addr;
	uint8_t proto;
	bool inv;

	switch (sreg->payload.offset) {
	case offsetof(struct iphdr, saddr):
		get_cmp_data(e, &addr, sizeof(addr), &inv);
		cs->fw.ip.src.s_addr = addr.s_addr;
		if (sreg->bitwise.set) {
			parse_mask_ipv4(sreg, &cs->fw.ip.smsk);
		} else {
			memset(&cs->fw.ip.smsk, 0xff,
			       min(sreg->payload.len, sizeof(struct in_addr)));
		}

		if (inv)
			cs->fw.ip.invflags |= IPT_INV_SRCIP;
		break;
	case offsetof(struct iphdr, daddr):
		get_cmp_data(e, &addr, sizeof(addr), &inv);
		cs->fw.ip.dst.s_addr = addr.s_addr;
		if (sreg->bitwise.set)
			parse_mask_ipv4(sreg, &cs->fw.ip.dmsk);
		else
			memset(&cs->fw.ip.dmsk, 0xff,
			       min(sreg->payload.len, sizeof(struct in_addr)));

		if (inv)
			cs->fw.ip.invflags |= IPT_INV_DSTIP;
		break;
	case offsetof(struct iphdr, protocol):
		get_cmp_data(e, &proto, sizeof(proto), &inv);
		cs->fw.ip.proto = proto;
		if (inv)
			cs->fw.ip.invflags |= IPT_INV_PROTO;
		break;
	case offsetof(struct iphdr, frag_off):
		cs->fw.ip.flags |= IPT_F_FRAG;
		inv = get_frag(sreg, e);
		if (inv)
			cs->fw.ip.invflags |= IPT_INV_FRAG;
		break;
	case offsetof(struct iphdr, ttl):
		if (nft_parse_hl(ctx, e, cs) < 0)
			ctx->errmsg = "invalid ttl field match";
		break;
	default:
		DEBUGP("unknown payload offset %d\n", sreg->payload.offset);
		ctx->errmsg = "unknown payload offset";
		break;
	}
}

struct nft_ruleparse_ops nft_ruleparse_ops_ipv4 = {
	.meta		= nft_ipv4_parse_meta,
	.payload	= nft_ipv4_parse_payload,
};
