/*
 * (C) 2014 by Giuseppe Longo <giuseppelng@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <net/if.h>
//#include <net/if_arp.h>
#include <netinet/if_ether.h>

#include <libnftnl/rule.h>
#include <libnftnl/expr.h>
#include <libnftnl/set.h>

#include <xtables.h>

#include "nft.h" /* just for nft_set_batch_lookup_byid? */
#include "nft-bridge.h"
#include "nft-cache.h"
#include "nft-shared.h"
#include "nft-ruleparse.h"
#include "xshared.h"

static void nft_bridge_parse_meta(struct nft_xt_ctx *ctx,
				  const struct nft_xt_ctx_reg *reg,
				  struct nftnl_expr *e,
				  struct iptables_command_state *cs)
{
	struct ebt_entry *fw = &cs->eb;
	uint8_t invflags = 0;
	char iifname[IFNAMSIZ] = {}, oifname[IFNAMSIZ] = {};

	switch (reg->meta_dreg.key) {
	case NFT_META_PROTOCOL:
		return;
	}

	if (parse_meta(ctx, e, reg->meta_dreg.key,
		       iifname, oifname, &invflags) < 0) {
		ctx->errmsg = "unknown meta key";
		return;
	}

	switch (reg->meta_dreg.key) {
	case NFT_META_BRI_IIFNAME:
		if (invflags & IPT_INV_VIA_IN)
			cs->eb.invflags |= EBT_ILOGICALIN;
		snprintf(fw->logical_in, sizeof(fw->logical_in), "%s", iifname);
		break;
	case NFT_META_IIFNAME:
		if (invflags & IPT_INV_VIA_IN)
			cs->eb.invflags |= EBT_IIN;
		snprintf(fw->in, sizeof(fw->in), "%s", iifname);
		break;
	case NFT_META_BRI_OIFNAME:
		if (invflags & IPT_INV_VIA_OUT)
			cs->eb.invflags |= EBT_ILOGICALOUT;
		snprintf(fw->logical_out, sizeof(fw->logical_out), "%s", oifname);
		break;
	case NFT_META_OIFNAME:
		if (invflags & IPT_INV_VIA_OUT)
			cs->eb.invflags |= EBT_IOUT;
		snprintf(fw->out, sizeof(fw->out), "%s", oifname);
		break;
	default:
		ctx->errmsg = "unknown bridge meta key";
		break;
	}
}

static void nft_bridge_parse_payload(struct nft_xt_ctx *ctx,
				     const struct nft_xt_ctx_reg *reg,
				     struct nftnl_expr *e,
				     struct iptables_command_state *cs)
{
	struct ebt_entry *fw = &cs->eb;
	unsigned char addr[ETH_ALEN];
	unsigned short int ethproto;
	uint8_t op;
	bool inv;
	int i;

	switch (reg->payload.offset) {
	case offsetof(struct ethhdr, h_dest):
		get_cmp_data(e, addr, sizeof(addr), &inv);
		for (i = 0; i < ETH_ALEN; i++)
			fw->destmac[i] = addr[i];
		if (inv)
			fw->invflags |= EBT_IDEST;

		if (reg->bitwise.set)
                        memcpy(fw->destmsk, reg->bitwise.mask, ETH_ALEN);
                else
			memset(&fw->destmsk, 0xff,
			       min(reg->payload.len, ETH_ALEN));
		fw->bitmask |= EBT_IDEST;
		break;
	case offsetof(struct ethhdr, h_source):
		get_cmp_data(e, addr, sizeof(addr), &inv);
		for (i = 0; i < ETH_ALEN; i++)
			fw->sourcemac[i] = addr[i];
		if (inv)
			fw->invflags |= EBT_ISOURCE;
		if (reg->bitwise.set)
                        memcpy(fw->sourcemsk, reg->bitwise.mask, ETH_ALEN);
                else
			memset(&fw->sourcemsk, 0xff,
			       min(reg->payload.len, ETH_ALEN));
		fw->bitmask |= EBT_ISOURCE;
		break;
	case offsetof(struct ethhdr, h_proto):
		__get_cmp_data(e, &ethproto, sizeof(ethproto), &op);
		if (ethproto == htons(0x0600)) {
			fw->bitmask |= EBT_802_3;
			inv = (op == NFT_CMP_GTE);
		} else {
			fw->ethproto = ethproto;
			inv = (op == NFT_CMP_NEQ);
		}
		if (inv)
			fw->invflags |= EBT_IPROTO;
		fw->bitmask &= ~EBT_NOPROTO;
		break;
	default:
		DEBUGP("unknown payload offset %d\n", reg->payload.offset);
		ctx->errmsg = "unknown payload offset";
		break;
	}
}

/* return 0 if saddr, 1 if daddr, -1 on error */
static int
lookup_check_ether_payload(uint32_t base, uint32_t offset, uint32_t len)
{
	if (base != 0 || len != ETH_ALEN)
		return -1;

	switch (offset) {
	case offsetof(struct ether_header, ether_dhost):
		return 1;
	case offsetof(struct ether_header, ether_shost):
		return 0;
	default:
		return -1;
	}
}

/* return 0 if saddr, 1 if daddr, -1 on error */
static int
lookup_check_iphdr_payload(uint32_t base, uint32_t offset, uint32_t len)
{
	if (base != 1 || len != 4)
		return -1;

	switch (offset) {
	case offsetof(struct iphdr, daddr):
		return 1;
	case offsetof(struct iphdr, saddr):
		return 0;
	default:
		return -1;
	}
}

/* Make sure previous payload expression(s) is/are consistent and extract if
 * matching on source or destination address and if matching on MAC and IP or
 * only MAC address. */
static int lookup_analyze_payloads(struct nft_xt_ctx *ctx,
				   enum nft_registers sreg,
				   uint32_t key_len,
				   bool *dst, bool *ip)
{
	const struct nft_xt_ctx_reg *reg;
	int val, val2 = -1;

	reg = nft_xt_ctx_get_sreg(ctx, sreg);
	if (!reg)
		return -1;

	if (reg->type != NFT_XT_REG_PAYLOAD) {
		ctx->errmsg = "lookup reg is not payload type";
		return -1;
	}

	switch (key_len) {
	case 12: /* ether + ipv4addr */
		val = lookup_check_ether_payload(reg->payload.base,
						 reg->payload.offset,
						 reg->payload.len);
		if (val < 0) {
			DEBUGP("unknown payload base/offset/len %d/%d/%d\n",
			       reg->payload.base, reg->payload.offset,
			       reg->payload.len);
			return -1;
		}

		sreg = nft_get_next_reg(sreg, ETH_ALEN);

		reg = nft_xt_ctx_get_sreg(ctx, sreg);
		if (!reg) {
			ctx->errmsg = "next lookup register is invalid";
			return -1;
		}

		if (reg->type != NFT_XT_REG_PAYLOAD) {
			ctx->errmsg = "next lookup reg is not payload type";
			return -1;
		}

		val2 = lookup_check_iphdr_payload(reg->payload.base,
						  reg->payload.offset,
						  reg->payload.len);
		if (val2 < 0) {
			DEBUGP("unknown payload base/offset/len %d/%d/%d\n",
			       reg->payload.base, reg->payload.offset,
			       reg->payload.len);
			return -1;
		} else if (val != val2) {
			DEBUGP("mismatching payload match offsets\n");
			return -1;
		}
		break;
	case 6: /* ether */
		val = lookup_check_ether_payload(reg->payload.base,
						 reg->payload.offset,
						 reg->payload.len);
		if (val < 0) {
			DEBUGP("unknown payload base/offset/len %d/%d/%d\n",
			       reg->payload.base, reg->payload.offset,
			       reg->payload.len);
			return -1;
		}
		break;
	default:
		ctx->errmsg = "unsupported lookup key length";
		return -1;
	}

	if (dst)
		*dst = (val == 1);
	if (ip)
		*ip = (val2 != -1);
	return 0;
}

static int set_elems_to_among_pairs(struct nft_among_pair *pairs,
				    const struct nftnl_set *s, int cnt)
{
	struct nftnl_set_elems_iter *iter = nftnl_set_elems_iter_create(s);
	struct nftnl_set_elem *elem;
	size_t tmpcnt = 0;
	const void *data;
	uint32_t datalen;
	int ret = -1;

	if (!iter) {
		fprintf(stderr, "BUG: set elems iter allocation failed\n");
		return ret;
	}

	while ((elem = nftnl_set_elems_iter_next(iter))) {
		data = nftnl_set_elem_get(elem, NFTNL_SET_ELEM_KEY, &datalen);
		if (!data) {
			fprintf(stderr, "BUG: set elem without key\n");
			goto err;
		}
		if (datalen > sizeof(*pairs)) {
			fprintf(stderr, "BUG: overlong set elem\n");
			goto err;
		}
		nft_among_insert_pair(pairs, &tmpcnt, data);
	}
	ret = 0;
err:
	nftnl_set_elems_iter_destroy(iter);
	return ret;
}

static struct nftnl_set *set_from_lookup_expr(struct nft_xt_ctx *ctx,
					      const struct nftnl_expr *e)
{
	const char *set_name = nftnl_expr_get_str(e, NFTNL_EXPR_LOOKUP_SET);
	uint32_t set_id = nftnl_expr_get_u32(e, NFTNL_EXPR_LOOKUP_SET_ID);
	struct nftnl_set_list *slist;
	struct nftnl_set *set;

	slist = nft_set_list_get(ctx->h, ctx->table, set_name);
	if (slist) {
		set = nftnl_set_list_lookup_byname(slist, set_name);
		if (set)
			return set;

		set = nft_set_batch_lookup_byid(ctx->h, set_id);
		if (set)
			return set;
	}

	return NULL;
}

static void nft_bridge_parse_lookup(struct nft_xt_ctx *ctx,
				    struct nftnl_expr *e)
{
	struct xtables_match *match = NULL;
	struct nft_among_data *among_data;
	bool is_dst, have_ip, inv;
	struct ebt_match *ematch;
	struct nftnl_set *s;
	size_t poff, size;
	uint32_t cnt;

	s = set_from_lookup_expr(ctx, e);
	if (!s)
		xtables_error(OTHER_PROBLEM,
			      "BUG: lookup expression references unknown set");

	if (lookup_analyze_payloads(ctx,
				    nftnl_expr_get_u32(e, NFTNL_EXPR_LOOKUP_SREG),
				    nftnl_set_get_u32(s, NFTNL_SET_KEY_LEN),
				    &is_dst, &have_ip))
		return;

	cnt = nftnl_set_get_u32(s, NFTNL_SET_DESC_SIZE);

	for (ematch = ctx->cs->match_list; ematch; ematch = ematch->next) {
		if (!ematch->ismatch || strcmp(ematch->u.match->name, "among"))
			continue;

		match = ematch->u.match;
		among_data = (struct nft_among_data *)match->m->data;

		size = cnt + among_data->src.cnt + among_data->dst.cnt;
		size *= sizeof(struct nft_among_pair);

		size += XT_ALIGN(sizeof(struct xt_entry_match)) +
			sizeof(struct nft_among_data);

		match->m = xtables_realloc(match->m, size);
		break;
	}
	if (!match) {
		match = xtables_find_match("among", XTF_TRY_LOAD,
					   &ctx->cs->matches);

		size = cnt * sizeof(struct nft_among_pair);
		size += XT_ALIGN(sizeof(struct xt_entry_match)) +
			sizeof(struct nft_among_data);

		match->m = xtables_calloc(1, size);
		strcpy(match->m->u.user.name, match->name);
		match->m->u.user.revision = match->revision;
		xs_init_match(match);

		if (ctx->h->ops->rule_parse->match != NULL)
			ctx->h->ops->rule_parse->match(match, ctx->cs);
	}
	if (!match)
		return;

	match->m->u.match_size = size;

	inv = !!(nftnl_expr_get_u32(e, NFTNL_EXPR_LOOKUP_FLAGS) &
				    NFT_LOOKUP_F_INV);

	among_data = (struct nft_among_data *)match->m->data;
	poff = nft_among_prepare_data(among_data, is_dst, cnt, inv, have_ip);
	if (set_elems_to_among_pairs(among_data->pairs + poff, s, cnt))
		xtables_error(OTHER_PROBLEM,
			      "ebtables among pair parsing failed");
}

static void parse_watcher(void *object, struct ebt_match **match_list,
			  bool ismatch)
{
	struct ebt_match *m = xtables_calloc(1, sizeof(struct ebt_match));

	if (ismatch)
		m->u.match = object;
	else
		m->u.watcher = object;

	m->ismatch = ismatch;
	if (*match_list == NULL)
		*match_list = m;
	else
		(*match_list)->next = m;
}

static void nft_bridge_parse_match(struct xtables_match *m,
				   struct iptables_command_state *cs)
{
	parse_watcher(m, &cs->match_list, true);
}

static void nft_bridge_parse_target(struct xtables_target *t,
				    struct iptables_command_state *cs)
{
	/* harcoded names :-( */
	if (strcmp(t->name, "log") == 0 ||
	    strcmp(t->name, "nflog") == 0) {
		parse_watcher(t, &cs->match_list, false);
		cs->jumpto = NULL;
		cs->target = NULL;
		return;
	}
}

struct nft_ruleparse_ops nft_ruleparse_ops_bridge = {
	.meta		= nft_bridge_parse_meta,
	.payload	= nft_bridge_parse_payload,
	.lookup		= nft_bridge_parse_lookup,
	.match		= nft_bridge_parse_match,
	.target		= nft_bridge_parse_target,
};
