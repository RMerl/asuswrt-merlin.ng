/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Tomasz Bursztyka <tomasz.bursztyka@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <linux/netfilter/nf_log.h>
#include <linux/netfilter/xt_limit.h>
#include <linux/netfilter/xt_mark.h>
#include <linux/netfilter/xt_NFLOG.h>
#include <linux/netfilter/xt_pkttype.h>

#include <linux/netfilter_ipv6/ip6t_hl.h>

#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include <xtables.h>

#include "nft-ruleparse.h"
#include "nft.h"

static struct xtables_match *
nft_find_match_in_cs(struct iptables_command_state *cs, const char *name)
{
	struct xtables_rule_match *rm;
	struct ebt_match *ebm;

	for (ebm = cs->match_list; ebm; ebm = ebm->next) {
		if (ebm->ismatch &&
		    !strcmp(ebm->u.match->m->u.user.name, name))
			return ebm->u.match;
	}
	for (rm = cs->matches; rm; rm = rm->next) {
		if (!strcmp(rm->match->m->u.user.name, name))
			return rm->match;
	}
	return NULL;
}

void *
nft_create_match(struct nft_xt_ctx *ctx,
		 struct iptables_command_state *cs,
		 const char *name, bool reuse)
{
	struct xtables_match *match;
	struct xt_entry_match *m;
	unsigned int size;

	if (reuse) {
		match = nft_find_match_in_cs(cs, name);
		if (match)
			return match->m->data;
	}

	match = xtables_find_match(name, XTF_TRY_LOAD,
				   &cs->matches);
	if (!match)
		return NULL;

	size = XT_ALIGN(sizeof(struct xt_entry_match)) + match->size;
	m = xtables_calloc(1, size);
	m->u.match_size = size;
	m->u.user.revision = match->revision;

	strcpy(m->u.user.name, match->name);
	match->m = m;

	xs_init_match(match);

	if (ctx->h->ops->rule_parse->match)
		ctx->h->ops->rule_parse->match(match, cs);

	return match->m->data;
}

static void *
__nft_create_target(struct nft_xt_ctx *ctx, const char *name, size_t tgsize)
{
	struct xtables_target *target;
	size_t size;

	target = xtables_find_target(name, XTF_TRY_LOAD);
	if (!target)
		return NULL;

	size = XT_ALIGN(sizeof(*target->t)) + (tgsize ?: target->size);

	target->t = xtables_calloc(1, size);
	target->t->u.target_size = size;
	target->t->u.user.revision = target->revision;
	strcpy(target->t->u.user.name, name);

	xs_init_target(target);

	ctx->cs->jumpto = name;
	ctx->cs->target = target;

	if (ctx->h->ops->rule_parse->target)
		ctx->h->ops->rule_parse->target(target, ctx->cs);

	return target->t->data;
}

void *
nft_create_target(struct nft_xt_ctx *ctx, const char *name)
{
	return __nft_create_target(ctx, name, 0);
}

static void nft_parse_counter(struct nftnl_expr *e, struct xt_counters *counters)
{
	counters->pcnt = nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_PACKETS);
	counters->bcnt = nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_BYTES);
}

static void nft_parse_payload(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	enum nft_registers regnum = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_DREG);
	struct nft_xt_ctx_reg *reg = nft_xt_ctx_get_dreg(ctx, regnum);

	if (!reg)
		return;

	reg->type = NFT_XT_REG_PAYLOAD;
	reg->payload.base = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_BASE);
	reg->payload.offset = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_OFFSET);
	reg->payload.len = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_LEN);
}

static bool nft_parse_meta_set_common(struct nft_xt_ctx* ctx,
				      struct nft_xt_ctx_reg *sreg)
{
	if ((sreg->type != NFT_XT_REG_IMMEDIATE)) {
		ctx->errmsg = "meta sreg is not an immediate";
		return false;
	}

	return true;
}

static void nft_parse_meta_set(struct nft_xt_ctx *ctx,
			       struct nftnl_expr *e)
{
	struct nft_xt_ctx_reg *sreg;
	enum nft_registers sregnum;

	sregnum = nftnl_expr_get_u32(e, NFTNL_EXPR_META_SREG);
	sreg = nft_xt_ctx_get_sreg(ctx, sregnum);
	if (!sreg)
		return;

	switch (nftnl_expr_get_u32(e, NFTNL_EXPR_META_KEY)) {
	case NFT_META_NFTRACE:
		if (!nft_parse_meta_set_common(ctx, sreg))
			return;

		if (sreg->immediate.data[0] == 0) {
			ctx->errmsg = "meta sreg immediate is 0";
			return;
		}

		if (!nft_create_target(ctx, "TRACE"))
			ctx->errmsg = "target TRACE not found";
		break;
	case NFT_META_BRI_BROUTE:
		if (!nft_parse_meta_set_common(ctx, sreg))
			return;

		ctx->cs->jumpto = "DROP";
		break;
	case NFT_META_MARK: {
		struct xt_mark_tginfo2 *mt;

		if (!nft_parse_meta_set_common(ctx, sreg))
			return;

		mt = nft_create_target(ctx, "MARK");
		if (!mt) {
			ctx->errmsg = "target MARK not found";
			return;
		}

		mt->mark = sreg->immediate.data[0];
		if (sreg->bitwise.set)
			mt->mask = sreg->bitwise.mask[0];
		else
			mt->mask = ~0u;
		break;
	}
	default:
		ctx->errmsg = "meta sreg key not supported";
		break;
	}
}

static void nft_parse_meta(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
        struct nft_xt_ctx_reg *reg;

	if (nftnl_expr_is_set(e, NFTNL_EXPR_META_SREG)) {
		nft_parse_meta_set(ctx, e);
		return;
	}

	reg = nft_xt_ctx_get_dreg(ctx, nftnl_expr_get_u32(e, NFTNL_EXPR_META_DREG));
	if (!reg)
		return;

	reg->meta_dreg.key = nftnl_expr_get_u32(e, NFTNL_EXPR_META_KEY);
	reg->type = NFT_XT_REG_META_DREG;
}

static void nft_parse_bitwise(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	enum nft_registers sregnum = nftnl_expr_get_u32(e, NFTNL_EXPR_BITWISE_SREG);
	enum nft_registers dregnum = nftnl_expr_get_u32(e, NFTNL_EXPR_BITWISE_DREG);
	struct nft_xt_ctx_reg *sreg = nft_xt_ctx_get_sreg(ctx, sregnum);
	struct nft_xt_ctx_reg *dreg = sreg;
	const void *data;
	uint32_t len;

	if (!sreg)
		return;

	if (sregnum != dregnum) {
		dreg = nft_xt_ctx_get_sreg(ctx, dregnum); /* sreg, do NOT clear ... */
		if (!dreg)
			return;

		*dreg = *sreg;  /* .. and copy content instead */
	}

	data = nftnl_expr_get(e, NFTNL_EXPR_BITWISE_XOR, &len);

	if (len > sizeof(dreg->bitwise.xor)) {
		ctx->errmsg = "bitwise xor too large";
		return;
	}

	memcpy(dreg->bitwise.xor, data, len);

	data = nftnl_expr_get(e, NFTNL_EXPR_BITWISE_MASK, &len);

	if (len > sizeof(dreg->bitwise.mask)) {
		ctx->errmsg = "bitwise mask too large";
		return;
	}

	memcpy(dreg->bitwise.mask, data, len);

	dreg->bitwise.set = true;
}

static void nft_parse_icmp(struct nft_xt_ctx *ctx,
			   struct iptables_command_state *cs,
			   struct nft_xt_ctx_reg *sreg,
			   uint8_t op, const char *data, size_t dlen)
{
	struct ipt_icmp icmp = {
		.type = UINT8_MAX,
		.code = { 0, UINT8_MAX },
	}, *icmpp;

	if (dlen < 1)
		goto out_err_len;

	switch (sreg->payload.offset) {
	case 0:
		icmp.type = data[0];
		if (dlen == 1)
			break;
		dlen--;
		data++;
		/* fall through */
	case 1:
		if (dlen > 1)
			goto out_err_len;
		icmp.code[0] = icmp.code[1] = data[0];
		break;
	default:
		ctx->errmsg = "unexpected payload offset";
		return;
	}

	switch (ctx->h->family) {
	case NFPROTO_IPV4:
		icmpp = nft_create_match(ctx, cs, "icmp", false);
		break;
	case NFPROTO_IPV6:
		if (icmp.type == UINT8_MAX) {
			ctx->errmsg = "icmp6 code with any type match not supported";
			return;
		}
		icmpp = nft_create_match(ctx, cs, "icmp6", false);
		break;
	default:
		ctx->errmsg = "unexpected family for icmp match";
		return;
	}

	if (!icmpp) {
		ctx->errmsg = "icmp match extension not found";
		return;
	}
	memcpy(icmpp, &icmp, sizeof(icmp));
	return;

out_err_len:
	ctx->errmsg = "unexpected RHS data length";
}

static void port_match_single_to_range(__u16 *ports, __u8 *invflags,
				       uint8_t op, int port, __u8 invflag)
{
	if (port < 0)
		return;

	switch (op) {
	case NFT_CMP_NEQ:
		*invflags |= invflag;
		/* fallthrough */
	case NFT_CMP_EQ:
		ports[0] = port;
		ports[1] = port;
		break;
	case NFT_CMP_LT:
		ports[1] = max(port - 1, 1);
		break;
	case NFT_CMP_LTE:
		ports[1] = port;
		break;
	case NFT_CMP_GT:
		ports[0] = min(port + 1, UINT16_MAX);
		break;
	case NFT_CMP_GTE:
		ports[0] = port;
		break;
	}
}

static void nft_parse_udp(struct nft_xt_ctx *ctx,
			  struct iptables_command_state *cs,
			  int sport, int dport,
			  uint8_t op)
{
	struct xt_udp *udp = nft_create_match(ctx, cs, "udp", true);

	if (!udp) {
		ctx->errmsg = "udp match extension not found";
		return;
	}

	port_match_single_to_range(udp->spts, &udp->invflags,
				   op, sport, XT_UDP_INV_SRCPT);
	port_match_single_to_range(udp->dpts, &udp->invflags,
				   op, dport, XT_UDP_INV_DSTPT);
}

static void nft_parse_tcp(struct nft_xt_ctx *ctx,
			  struct iptables_command_state *cs,
			  int sport, int dport,
			  uint8_t op)
{
	struct xt_tcp *tcp = nft_create_match(ctx, cs, "tcp", true);

	if (!tcp) {
		ctx->errmsg = "tcp match extension not found";
		return;
	}

	port_match_single_to_range(tcp->spts, &tcp->invflags,
				   op, sport, XT_TCP_INV_SRCPT);
	port_match_single_to_range(tcp->dpts, &tcp->invflags,
				   op, dport, XT_TCP_INV_DSTPT);
}

static void nft_parse_th_port(struct nft_xt_ctx *ctx,
			      struct iptables_command_state *cs,
			      uint8_t proto,
			      int sport, int dport, uint8_t op)
{
	switch (proto) {
	case IPPROTO_UDP:
		nft_parse_udp(ctx, cs, sport, dport, op);
		break;
	case IPPROTO_TCP:
		nft_parse_tcp(ctx, cs, sport, dport, op);
		break;
	default:
		ctx->errmsg = "unknown layer 4 protocol for TH match";
	}
}

static void nft_parse_tcp_flags(struct nft_xt_ctx *ctx,
				struct iptables_command_state *cs,
				uint8_t op, uint8_t flags, uint8_t mask)
{
	struct xt_tcp *tcp = nft_create_match(ctx, cs, "tcp", true);

	if (!tcp) {
		ctx->errmsg = "tcp match extension not found";
		return;
	}

	if (op == NFT_CMP_NEQ)
		tcp->invflags |= XT_TCP_INV_FLAGS;
	tcp->flg_cmp = flags;
	tcp->flg_mask = mask;
}

static void nft_parse_transport(struct nft_xt_ctx *ctx,
				struct nftnl_expr *e,
				struct iptables_command_state *cs)
{
	struct nft_xt_ctx_reg *sreg;
	enum nft_registers reg;
	uint32_t sdport;
	uint16_t port;
	uint8_t proto, op;
	unsigned int len;

	switch (ctx->h->family) {
	case NFPROTO_IPV4:
		proto = ctx->cs->fw.ip.proto;
		break;
	case NFPROTO_IPV6:
		proto = ctx->cs->fw6.ipv6.proto;
		break;
	default:
		ctx->errmsg = "invalid family for TH match";
		return;
	}

	nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &len);
	op = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP);

	reg = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_SREG);
	sreg = nft_xt_ctx_get_sreg(ctx, reg);
	if (!sreg)
		return;

	if (sreg->type != NFT_XT_REG_PAYLOAD) {
		ctx->errmsg = "sgreg not payload";
		return;
	}

	switch (proto) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
		break;
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		nft_parse_icmp(ctx, cs, sreg, op,
			       nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &len),
			       len);
		return;
	default:
		ctx->errmsg = "unsupported layer 4 protocol value";
		return;
	}

	switch(sreg->payload.offset) {
	case 0: /* th->sport */
		switch (len) {
		case 2: /* load sport only */
			port = ntohs(nftnl_expr_get_u16(e, NFTNL_EXPR_CMP_DATA));
			nft_parse_th_port(ctx, cs, proto, port, -1, op);
			return;
		case 4: /* load both src and dst port */
			sdport = ntohl(nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_DATA));
			nft_parse_th_port(ctx, cs, proto, sdport >> 16, sdport & 0xffff, op);
			return;
		}
		break;
	case 2: /* th->dport */
		switch (len) {
		case 2:
			port = ntohs(nftnl_expr_get_u16(e, NFTNL_EXPR_CMP_DATA));
			nft_parse_th_port(ctx, cs, proto, -1, port, op);
			return;
		}
		break;
	case 13: /* th->flags */
		if (len == 1 && proto == IPPROTO_TCP) {
			uint8_t flags = nftnl_expr_get_u8(e, NFTNL_EXPR_CMP_DATA);
			uint8_t mask = ~0;

			if (sreg->bitwise.set)
				memcpy(&mask, &sreg->bitwise.mask, sizeof(mask));

			nft_parse_tcp_flags(ctx, cs, op, flags, mask);
		}
		return;
	}
}

static void nft_parse_cmp(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	struct nft_xt_ctx_reg *sreg;
	uint32_t reg;

	reg = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_SREG);

	sreg = nft_xt_ctx_get_sreg(ctx, reg);
	if (!sreg)
		return;

	switch (sreg->type) {
	case NFT_XT_REG_UNDEF:
		ctx->errmsg = "cmp sreg undef";
		break;
	case NFT_XT_REG_META_DREG:
		ctx->h->ops->rule_parse->meta(ctx, sreg, e, ctx->cs);
		break;
	case NFT_XT_REG_PAYLOAD:
		switch (sreg->payload.base) {
		case NFT_PAYLOAD_LL_HEADER:
			if (ctx->h->family == NFPROTO_BRIDGE)
				ctx->h->ops->rule_parse->payload(ctx, sreg, e, ctx->cs);
			break;
		case NFT_PAYLOAD_NETWORK_HEADER:
			ctx->h->ops->rule_parse->payload(ctx, sreg, e, ctx->cs);
			break;
		case NFT_PAYLOAD_TRANSPORT_HEADER:
			nft_parse_transport(ctx, e, ctx->cs);
			break;
		}

		break;
	default:
		ctx->errmsg = "cmp sreg has unknown type";
		break;
	}
}

static void nft_parse_immediate(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	const char *chain = nftnl_expr_get_str(e, NFTNL_EXPR_IMM_CHAIN);
	struct iptables_command_state *cs = ctx->cs;
	int verdict;

	if (nftnl_expr_is_set(e, NFTNL_EXPR_IMM_DATA)) {
		struct nft_xt_ctx_reg *dreg;
		const void *imm_data;
		uint32_t len;

		imm_data = nftnl_expr_get(e, NFTNL_EXPR_IMM_DATA, &len);
		dreg = nft_xt_ctx_get_dreg(ctx, nftnl_expr_get_u32(e, NFTNL_EXPR_IMM_DREG));
		if (!dreg)
			return;

		if (len > sizeof(dreg->immediate.data)) {
			ctx->errmsg = "oversized immediate data";
			return;
		}

		memcpy(dreg->immediate.data, imm_data, len);
		dreg->immediate.len = len;
		dreg->type = NFT_XT_REG_IMMEDIATE;

		return;
	}

	verdict = nftnl_expr_get_u32(e, NFTNL_EXPR_IMM_VERDICT);
	/* Standard target? */
	switch(verdict) {
	case NF_ACCEPT:
		if (cs->jumpto && strcmp(ctx->table, "broute") == 0)
			break;
		cs->jumpto = "ACCEPT";
		break;
	case NF_DROP:
		cs->jumpto = "DROP";
		break;
	case NFT_RETURN:
		cs->jumpto = "RETURN";
		break;;
	case NFT_GOTO:
		if (ctx->h->ops->set_goto_flag)
			ctx->h->ops->set_goto_flag(cs);
		/* fall through */
	case NFT_JUMP:
		cs->jumpto = chain;
		/* fall through */
	default:
		return;
	}

	if (!nft_create_target(ctx, cs->jumpto))
		ctx->errmsg = "verdict extension not found";
}

static void nft_parse_match(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	uint32_t mt_len;
	const char *mt_name = nftnl_expr_get_str(e, NFTNL_EXPR_MT_NAME);
	const void *mt_info = nftnl_expr_get(e, NFTNL_EXPR_MT_INFO, &mt_len);
	struct xtables_match *match;
	struct xtables_rule_match **matches;
	struct xt_entry_match *m;

	switch (ctx->h->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
	case NFPROTO_BRIDGE:
		matches = &ctx->cs->matches;
		break;
	default:
		fprintf(stderr, "BUG: nft_parse_match() unknown family %d\n",
			ctx->h->family);
		exit(EXIT_FAILURE);
	}

	match = xtables_find_match(mt_name, XTF_TRY_LOAD, matches);
	if (match == NULL) {
		ctx->errmsg = "match extension not found";
		return;
	}

	m = xtables_calloc(1, sizeof(struct xt_entry_match) + mt_len);
	memcpy(&m->data, mt_info, mt_len);
	m->u.match_size = mt_len + XT_ALIGN(sizeof(struct xt_entry_match));
	m->u.user.revision = nftnl_expr_get_u32(e, NFTNL_EXPR_TG_REV);
	strcpy(m->u.user.name, match->name);

	match->m = m;

	if (ctx->h->ops->rule_parse->match != NULL)
		ctx->h->ops->rule_parse->match(match, ctx->cs);
}

static void nft_parse_target(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	uint32_t tg_len;
	const char *targname = nftnl_expr_get_str(e, NFTNL_EXPR_TG_NAME);
	const void *targinfo = nftnl_expr_get(e, NFTNL_EXPR_TG_INFO, &tg_len);
	void *data;

	data = __nft_create_target(ctx, targname, tg_len);
	if (!data)
		ctx->errmsg = "target extension not found";
	else
		memcpy(data, targinfo, tg_len);
}

static void nft_parse_limit(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	__u32 burst = nftnl_expr_get_u32(e, NFTNL_EXPR_LIMIT_BURST);
	__u64 unit = nftnl_expr_get_u64(e, NFTNL_EXPR_LIMIT_UNIT);
	__u64 rate = nftnl_expr_get_u64(e, NFTNL_EXPR_LIMIT_RATE);
	struct xt_rateinfo *rinfo;

	switch (ctx->h->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
	case NFPROTO_BRIDGE:
		break;
	default:
		fprintf(stderr, "BUG: nft_parse_limit() unknown family %d\n",
			ctx->h->family);
		exit(EXIT_FAILURE);
	}

	rinfo = nft_create_match(ctx, ctx->cs, "limit", false);
	if (!rinfo) {
		ctx->errmsg = "limit match extension not found";
		return;
	}

	rinfo->avg = XT_LIMIT_SCALE * unit / rate;
	rinfo->burst = burst;
}

static void nft_parse_lookup(struct nft_xt_ctx *ctx, struct nft_handle *h,
			     struct nftnl_expr *e)
{
	if (ctx->h->ops->rule_parse->lookup)
		ctx->h->ops->rule_parse->lookup(ctx, e);
}

static void nft_parse_log(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	/*
	 * In order to handle the longer log-prefix supported by nft, instead of
	 * using struct xt_nflog_info, we use a struct with a compatible layout, but
	 * a larger buffer for the prefix.
	 */
	struct xt_nflog_info_nft {
		__u32 len;
		__u16 group;
		__u16 threshold;
		__u16 flags;
		__u16 pad;
		char  prefix[NF_LOG_PREFIXLEN];
	} info = {
		.group     = nftnl_expr_get_u16(e, NFTNL_EXPR_LOG_GROUP),
		.threshold = nftnl_expr_get_u16(e, NFTNL_EXPR_LOG_QTHRESHOLD),
	};
	void *data;

	if (nftnl_expr_is_set(e, NFTNL_EXPR_LOG_SNAPLEN)) {
		info.len = nftnl_expr_get_u32(e, NFTNL_EXPR_LOG_SNAPLEN);
		info.flags = XT_NFLOG_F_COPY_LEN;
	}
	if (nftnl_expr_is_set(e, NFTNL_EXPR_LOG_PREFIX))
		snprintf(info.prefix, sizeof(info.prefix), "%s",
			 nftnl_expr_get_str(e, NFTNL_EXPR_LOG_PREFIX));

	data = __nft_create_target(ctx, "NFLOG",
				   XT_ALIGN(sizeof(struct xt_nflog_info_nft)));
	if (!data)
		ctx->errmsg = "NFLOG target extension not found";
	else
		memcpy(data, &info, sizeof(info));
}

static void nft_parse_udp_range(struct nft_xt_ctx *ctx,
			        struct iptables_command_state *cs,
			        int sport_from, int sport_to,
			        int dport_from, int dport_to,
				uint8_t op)
{
	struct xt_udp *udp = nft_create_match(ctx, cs, "udp", true);

	if (!udp) {
		ctx->errmsg = "udp match extension not found";
		return;
	}

	if (sport_from >= 0) {
		switch (op) {
		case NFT_RANGE_NEQ:
			udp->invflags |= XT_UDP_INV_SRCPT;
			/* fallthrough */
		case NFT_RANGE_EQ:
			udp->spts[0] = sport_from;
			udp->spts[1] = sport_to;
			break;
		}
	}

	if (dport_to >= 0) {
		switch (op) {
		case NFT_CMP_NEQ:
			udp->invflags |= XT_UDP_INV_DSTPT;
			/* fallthrough */
		case NFT_CMP_EQ:
			udp->dpts[0] = dport_from;
			udp->dpts[1] = dport_to;
			break;
		}
	}
}

static void nft_parse_tcp_range(struct nft_xt_ctx *ctx,
			        struct iptables_command_state *cs,
			        int sport_from, int sport_to,
			        int dport_from, int dport_to,
				uint8_t op)
{
	struct xt_tcp *tcp = nft_create_match(ctx, cs, "tcp", true);

	if (!tcp) {
		ctx->errmsg = "tcp match extension not found";
		return;
	}

	if (sport_from >= 0) {
		switch (op) {
		case NFT_RANGE_NEQ:
			tcp->invflags |= XT_TCP_INV_SRCPT;
			/* fallthrough */
		case NFT_RANGE_EQ:
			tcp->spts[0] = sport_from;
			tcp->spts[1] = sport_to;
			break;
		}
	}

	if (dport_to >= 0) {
		switch (op) {
		case NFT_CMP_NEQ:
			tcp->invflags |= XT_TCP_INV_DSTPT;
			/* fallthrough */
		case NFT_CMP_EQ:
			tcp->dpts[0] = dport_from;
			tcp->dpts[1] = dport_to;
			break;
		}
	}
}

static void nft_parse_th_port_range(struct nft_xt_ctx *ctx,
				    struct iptables_command_state *cs,
				    uint8_t proto,
				    int sport_from, int sport_to,
				    int dport_from, int dport_to, uint8_t op)
{
	switch (proto) {
	case IPPROTO_UDP:
		nft_parse_udp_range(ctx, cs, sport_from, sport_to, dport_from, dport_to, op);
		break;
	case IPPROTO_TCP:
		nft_parse_tcp_range(ctx, cs, sport_from, sport_to, dport_from, dport_to, op);
		break;
	}
}

static void nft_parse_transport_range(struct nft_xt_ctx *ctx,
				      const struct nft_xt_ctx_reg *sreg,
				      struct nftnl_expr *e,
				      struct iptables_command_state *cs)
{
	unsigned int len_from, len_to;
	uint8_t proto, op;
	uint16_t from, to;

	switch (ctx->h->family) {
	case NFPROTO_IPV4:
		proto = ctx->cs->fw.ip.proto;
		break;
	case NFPROTO_IPV6:
		proto = ctx->cs->fw6.ipv6.proto;
		break;
	default:
		proto = 0;
		break;
	}

	nftnl_expr_get(e, NFTNL_EXPR_RANGE_FROM_DATA, &len_from);
	nftnl_expr_get(e, NFTNL_EXPR_RANGE_FROM_DATA, &len_to);
	if (len_to != len_from || len_to != 2)
		return;

	op = nftnl_expr_get_u32(e, NFTNL_EXPR_RANGE_OP);

	from = ntohs(nftnl_expr_get_u16(e, NFTNL_EXPR_RANGE_FROM_DATA));
	to = ntohs(nftnl_expr_get_u16(e, NFTNL_EXPR_RANGE_TO_DATA));

	switch (sreg->payload.offset) {
	case 0:
		nft_parse_th_port_range(ctx, cs, proto, from, to, -1, -1, op);
		return;
	case 2:
		to = ntohs(nftnl_expr_get_u16(e, NFTNL_EXPR_RANGE_TO_DATA));
		nft_parse_th_port_range(ctx, cs, proto, -1, -1, from, to, op);
		return;
	}
}

static void nft_parse_range(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	struct nft_xt_ctx_reg *sreg;
	uint32_t reg;

	reg = nftnl_expr_get_u32(e, NFTNL_EXPR_RANGE_SREG);
	sreg = nft_xt_ctx_get_sreg(ctx, reg);

	switch (sreg->type) {
	case NFT_XT_REG_UNDEF:
		ctx->errmsg = "range sreg undef";
		break;
	case NFT_XT_REG_PAYLOAD:
		switch (sreg->payload.base) {
		case NFT_PAYLOAD_TRANSPORT_HEADER:
			nft_parse_transport_range(ctx, sreg, e, ctx->cs);
			break;
		default:
			ctx->errmsg = "range with unknown payload base";
			break;
		}
		break;
	default:
		ctx->errmsg = "range sreg type unsupported";
		break;
	}
}

bool nft_rule_to_iptables_command_state(struct nft_handle *h,
					const struct nftnl_rule *r,
					struct iptables_command_state *cs)
{
	struct nftnl_expr *expr;
	struct nft_xt_ctx ctx = {
		.cs = cs,
		.h = h,
		.table = nftnl_rule_get_str(r, NFTNL_RULE_TABLE),
	};
	bool ret = true;

	ctx.iter = nftnl_expr_iter_create(r);
	if (ctx.iter == NULL)
		return false;

	expr = nftnl_expr_iter_next(ctx.iter);
	while (expr != NULL) {
		const char *name =
			nftnl_expr_get_str(expr, NFTNL_EXPR_NAME);

		if (strcmp(name, "counter") == 0)
			nft_parse_counter(expr, &ctx.cs->counters);
		else if (strcmp(name, "payload") == 0)
			nft_parse_payload(&ctx, expr);
		else if (strcmp(name, "meta") == 0)
			nft_parse_meta(&ctx, expr);
		else if (strcmp(name, "bitwise") == 0)
			nft_parse_bitwise(&ctx, expr);
		else if (strcmp(name, "cmp") == 0)
			nft_parse_cmp(&ctx, expr);
		else if (strcmp(name, "immediate") == 0)
			nft_parse_immediate(&ctx, expr);
		else if (strcmp(name, "match") == 0)
			nft_parse_match(&ctx, expr);
		else if (strcmp(name, "target") == 0)
			nft_parse_target(&ctx, expr);
		else if (strcmp(name, "limit") == 0)
			nft_parse_limit(&ctx, expr);
		else if (strcmp(name, "lookup") == 0)
			nft_parse_lookup(&ctx, h, expr);
		else if (strcmp(name, "log") == 0)
			nft_parse_log(&ctx, expr);
		else if (strcmp(name, "range") == 0)
			nft_parse_range(&ctx, expr);

		if (ctx.errmsg) {
			fprintf(stderr, "Error: %s\n", ctx.errmsg);
			ctx.errmsg = NULL;
			ret = false;
		}

		expr = nftnl_expr_iter_next(ctx.iter);
	}

	nftnl_expr_iter_destroy(ctx.iter);

	if (nftnl_rule_is_set(r, NFTNL_RULE_USERDATA)) {
		const void *data;
		uint32_t len, size;
		const char *comment;

		data = nftnl_rule_get_data(r, NFTNL_RULE_USERDATA, &len);
		comment = get_comment(data, len);
		if (comment) {
			struct xtables_match *match;
			struct xt_entry_match *m;

			match = xtables_find_match("comment", XTF_TRY_LOAD,
						   &cs->matches);
			if (match == NULL)
				return false;

			size = XT_ALIGN(sizeof(struct xt_entry_match))
				+ match->size;
			m = xtables_calloc(1, size);

			strncpy((char *)m->data, comment, match->size - 1);
			m->u.match_size = size;
			m->u.user.revision = 0;
			strcpy(m->u.user.name, match->name);

			match->m = m;
		}
	}

	if (!cs->jumpto)
		cs->jumpto = "";

	if (!ret)
		xtables_error(VERSION_PROBLEM, "Parsing nftables rule failed");
	return ret;
}

static void parse_ifname(const char *name, unsigned int len, char *dst)
{
	if (len == 0)
		return;

	memcpy(dst, name, len);
	if (name[len - 1] == '\0')
		return;

	if (len >= IFNAMSIZ)
		return;

	/* wildcard */
	dst[len++] = '+';
	if (len >= IFNAMSIZ)
		return;
	dst[len++] = 0;
}

static void parse_invalid_iface(char *iface, uint8_t *invflags, uint8_t invbit)
{
	if (*invflags & invbit || strcmp(iface, "INVAL/D"))
		return;

	/* nft's poor "! -o +" excuse */
	*invflags |= invbit;
	iface[0] = '+';
	iface[1] = '\0';
}

static uint32_t get_meta_mask(struct nft_xt_ctx *ctx, enum nft_registers sreg)
{
	struct nft_xt_ctx_reg *reg = nft_xt_ctx_get_sreg(ctx, sreg);

	if (reg->bitwise.set)
		return reg->bitwise.mask[0];

	return ~0u;
}

static int parse_meta_mark(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	struct xt_mark_mtinfo1 *mark;
	uint32_t value;

	mark = nft_create_match(ctx, ctx->cs, "mark", false);
	if (!mark)
		return -1;

	if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
		mark->invert = 1;

	value = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_DATA);
	mark->mark = value;
	mark->mask = get_meta_mask(ctx, nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_SREG));

	return 0;
}

static int parse_meta_pkttype(struct nft_xt_ctx *ctx, struct nftnl_expr *e)
{
	struct xt_pkttype_info *pkttype;
	uint8_t value;

	pkttype = nft_create_match(ctx, ctx->cs, "pkttype", false);
	if (!pkttype)
		return -1;

	if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
		pkttype->invert = 1;

	value = nftnl_expr_get_u8(e, NFTNL_EXPR_CMP_DATA);
	pkttype->pkttype = value;

	return 0;
}

int parse_meta(struct nft_xt_ctx *ctx, struct nftnl_expr *e, uint8_t key,
	       char *iniface, char *outiface, uint8_t *invflags)
{
	uint32_t value;
	const void *ifname;
	uint32_t len;

	switch(key) {
	case NFT_META_IIF:
		value = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_DATA);
		if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
			*invflags |= IPT_INV_VIA_IN;

		if_indextoname(value, iniface);
		break;
	case NFT_META_OIF:
		value = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_DATA);
		if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
			*invflags |= IPT_INV_VIA_OUT;

		if_indextoname(value, outiface);
		break;
	case NFT_META_BRI_IIFNAME:
	case NFT_META_IIFNAME:
		ifname = nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &len);
		if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
			*invflags |= IPT_INV_VIA_IN;

		parse_ifname(ifname, len, iniface);
		parse_invalid_iface(iniface, invflags, IPT_INV_VIA_IN);
		break;
	case NFT_META_BRI_OIFNAME:
	case NFT_META_OIFNAME:
		ifname = nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &len);
		if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
			*invflags |= IPT_INV_VIA_OUT;

		parse_ifname(ifname, len, outiface);
		parse_invalid_iface(outiface, invflags, IPT_INV_VIA_OUT);
		break;
	case NFT_META_MARK:
		parse_meta_mark(ctx, e);
		break;
	case NFT_META_PKTTYPE:
		parse_meta_pkttype(ctx, e);
		break;
	default:
		return -1;
	}

	return 0;
}

int nft_parse_hl(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
		 struct iptables_command_state *cs)
{
	struct ip6t_hl_info *info;
	uint8_t hl, mode;
	int op;

	hl = nftnl_expr_get_u8(e, NFTNL_EXPR_CMP_DATA);
	op = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP);

	switch (op) {
	case NFT_CMP_NEQ:
		mode = IP6T_HL_NE;
		break;
	case NFT_CMP_EQ:
		mode = IP6T_HL_EQ;
		break;
	case NFT_CMP_LT:
		mode = IP6T_HL_LT;
		break;
	case NFT_CMP_GT:
		mode = IP6T_HL_GT;
		break;
	case NFT_CMP_LTE:
		mode = IP6T_HL_LT;
		if (hl == 255)
			return -1;
		hl++;
		break;
	case NFT_CMP_GTE:
		mode = IP6T_HL_GT;
		if (hl == 0)
			return -1;
		hl--;
		break;
	default:
		return -1;
	}

	/* ipt_ttl_info and ip6t_hl_info have same layout,
	 * IPT_TTL_x and IP6T_HL_x are aliases as well, so
	 * just use HL for both ipv4 and ipv6.
	 */
	switch (ctx->h->family) {
	case NFPROTO_IPV4:
		info = nft_create_match(ctx, ctx->cs, "ttl", false);
		break;
	case NFPROTO_IPV6:
		info = nft_create_match(ctx, ctx->cs, "hl", false);
		break;
	default:
		return -1;
	}

	if (!info)
		return -1;

	info->hop_limit = hl;
	info->mode = mode;

	return 0;
}
