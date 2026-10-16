/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Giuseppe Longo <giuseppelng@gmail.com>
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
#include <net/if_arp.h>
#include <netinet/if_ether.h>

#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "nft-shared.h"
#include "nft-ruleparse.h"
#include "xshared.h"

static void nft_arp_parse_meta(struct nft_xt_ctx *ctx,
			       const struct nft_xt_ctx_reg *reg,
			       struct nftnl_expr *e,
			       struct iptables_command_state *cs)
{
	struct arpt_entry *fw = &cs->arp;
	uint8_t flags = 0;

	if (parse_meta(ctx, e, reg->meta_dreg.key, fw->arp.iniface,
		       fw->arp.outiface, &flags) == 0) {
		fw->arp.invflags |= flags;
		return;
	}

	ctx->errmsg = "Unknown arp meta key";
}

static void parse_mask_ipv4(const struct nft_xt_ctx_reg *reg, struct in_addr *mask)
{
	mask->s_addr = reg->bitwise.mask[0];
}

static bool nft_arp_parse_devaddr(const struct nft_xt_ctx_reg *reg,
				  struct nftnl_expr *e,
				  struct arpt_devaddr_info *info)
{
	uint32_t hlen;
	bool inv;

	nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &hlen);

	if (hlen != ETH_ALEN)
		return false;

	get_cmp_data(e, info->addr, ETH_ALEN, &inv);

	if (reg->bitwise.set)
		memcpy(info->mask, reg->bitwise.mask, ETH_ALEN);
	else
		memset(info->mask, 0xff,
		       min(reg->payload.len, ETH_ALEN));

	return inv;
}

static void nft_arp_parse_payload(struct nft_xt_ctx *ctx,
				  const struct nft_xt_ctx_reg *reg,
				  struct nftnl_expr *e,
				  struct iptables_command_state *cs)
{
	struct arpt_entry *fw = &cs->arp;
	struct in_addr addr;
	uint16_t ar_hrd, ar_pro, ar_op;
	uint8_t ar_hln, ar_pln;
	bool inv;

	switch (reg->payload.offset) {
	case offsetof(struct arphdr, ar_hrd):
		get_cmp_data(e, &ar_hrd, sizeof(ar_hrd), &inv);
		fw->arp.arhrd = ar_hrd;
		fw->arp.arhrd_mask = 0xffff;
		if (inv)
			fw->arp.invflags |= IPT_INV_ARPHRD;
		if (reg->bitwise.set)
			fw->arp.arhrd_mask = reg->bitwise.mask[0];
		break;
	case offsetof(struct arphdr, ar_pro):
		get_cmp_data(e, &ar_pro, sizeof(ar_pro), &inv);
		fw->arp.arpro = ar_pro;
		fw->arp.arpro_mask = 0xffff;
		if (inv)
			fw->arp.invflags |= IPT_INV_PROTO;
		if (reg->bitwise.set)
			fw->arp.arpro_mask = reg->bitwise.mask[0];
		break;
	case offsetof(struct arphdr, ar_op):
		get_cmp_data(e, &ar_op, sizeof(ar_op), &inv);
		fw->arp.arpop = ar_op;
		fw->arp.arpop_mask = 0xffff;
		if (inv)
			fw->arp.invflags |= IPT_INV_ARPOP;
		if (reg->bitwise.set)
			fw->arp.arpop_mask = reg->bitwise.mask[0];
		break;
	case offsetof(struct arphdr, ar_hln):
		get_cmp_data(e, &ar_hln, sizeof(ar_hln), &inv);
		fw->arp.arhln = ar_hln;
		fw->arp.arhln_mask = 0xff;
		if (inv)
			fw->arp.invflags |= IPT_INV_ARPHLN;
		if (reg->bitwise.set)
			fw->arp.arhln_mask = reg->bitwise.mask[0];
		break;
	case offsetof(struct arphdr, ar_pln):
		get_cmp_data(e, &ar_pln, sizeof(ar_pln), &inv);
		if (ar_pln != 4 || inv)
			ctx->errmsg = "unexpected ARP protocol length match";
		break;
	default:
		if (reg->payload.offset == sizeof(struct arphdr)) {
			if (nft_arp_parse_devaddr(reg, e, &fw->arp.src_devaddr))
				fw->arp.invflags |= IPT_INV_SRCDEVADDR;
		} else if (reg->payload.offset == sizeof(struct arphdr) +
					   fw->arp.arhln) {
			get_cmp_data(e, &addr, sizeof(addr), &inv);
			fw->arp.src.s_addr = addr.s_addr;
			if (reg->bitwise.set)
				parse_mask_ipv4(reg, &fw->arp.smsk);
			else
				memset(&fw->arp.smsk, 0xff,
				       min(reg->payload.len,
					   sizeof(struct in_addr)));

			if (inv)
				fw->arp.invflags |= IPT_INV_SRCIP;
		} else if (reg->payload.offset == sizeof(struct arphdr) +
						  fw->arp.arhln +
						  sizeof(struct in_addr)) {
			if (nft_arp_parse_devaddr(reg, e, &fw->arp.tgt_devaddr))
				fw->arp.invflags |= IPT_INV_TGTDEVADDR;
		} else if (reg->payload.offset == sizeof(struct arphdr) +
						  fw->arp.arhln +
						  sizeof(struct in_addr) +
						  fw->arp.arhln) {
			get_cmp_data(e, &addr, sizeof(addr), &inv);
			fw->arp.tgt.s_addr = addr.s_addr;
			if (reg->bitwise.set)
				parse_mask_ipv4(reg, &fw->arp.tmsk);
			else
				memset(&fw->arp.tmsk, 0xff,
				       min(reg->payload.len,
					   sizeof(struct in_addr)));

			if (inv)
				fw->arp.invflags |= IPT_INV_DSTIP;
		} else {
			ctx->errmsg = "unknown payload offset";
		}
		break;
	}
}

struct nft_ruleparse_ops nft_ruleparse_ops_arp = {
	.meta		= nft_arp_parse_meta,
	.payload	= nft_arp_parse_payload,
};
