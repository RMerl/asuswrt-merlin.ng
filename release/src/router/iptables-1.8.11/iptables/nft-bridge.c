/*
 * (C) 2014 by Giuseppe Longo <giuseppelng@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ether.h>
#include <inttypes.h>

#include <xtables.h>
#include <libiptc/libxtc.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/set.h>

#include "nft-shared.h"
#include "nft-bridge.h"
#include "nft-cache.h"
#include "nft.h"

void ebt_cs_clean(struct iptables_command_state *cs)
{
	struct ebt_match *m, *nm;

	xtables_rule_matches_free(&cs->matches);

	for (m = cs->match_list; m;) {
		if (!m->ismatch) {
			struct xtables_target *target = m->u.watcher;

			if (target->t) {
				free(target->t);
				target->t = NULL;
			}
			if (target == target->next)
				free(target);
		}

		nm = m->next;
		free(m);
		m = nm;
	}
	cs->match_list = NULL;

	if (cs->target) {
		free(cs->target->t);
		cs->target->t = NULL;

		if (cs->target == cs->target->next) {
			free(cs->target);
			cs->target = NULL;
		}
	}
}

/* Put the mac address into 6 (ETH_ALEN) bytes returns 0 on success. */
static void ebt_print_mac_and_mask(const unsigned char *mac, const unsigned char *mask)
{
	if (xtables_print_well_known_mac_and_mask(mac, mask))
		xtables_print_mac_and_mask(mac, mask);
}

static int add_meta_broute(struct nftnl_rule *r)
{
	struct nftnl_expr *expr;

	expr = nftnl_expr_alloc("immediate");
	if (expr == NULL)
		return -1;

	nftnl_expr_set_u32(expr, NFTNL_EXPR_IMM_DREG, NFT_REG32_01);
	nftnl_expr_set_u8(expr, NFTNL_EXPR_IMM_DATA, 1);
	nftnl_rule_add_expr(r, expr);

	expr = nftnl_expr_alloc("meta");
	if (expr == NULL)
		return -1;
	nftnl_expr_set_u32(expr, NFTNL_EXPR_META_KEY, NFT_META_BRI_BROUTE);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_META_SREG, NFT_REG32_01);

	nftnl_rule_add_expr(r, expr);
	return 0;
}

static int _add_action(struct nftnl_rule *r, struct iptables_command_state *cs)
{
	const char *table = nftnl_rule_get_str(r, NFTNL_RULE_TABLE);

	if (cs->target &&
	    table && strcmp(table, "broute") == 0) {
		if (strcmp(cs->jumpto, XTC_LABEL_DROP) == 0) {
			int ret = add_meta_broute(r);

			if (ret)
				return ret;

			cs->jumpto = "ACCEPT";
		}
	}

	return add_action(r, cs, false);
}

static int
nft_bridge_add_match(struct nft_handle *h, const struct ebt_entry *fw,
		     struct nft_rule_ctx *ctx, struct nftnl_rule *r,
		     struct xt_entry_match *m)
{
	if (!strcmp(m->u.user.name, "802_3") && !(fw->bitmask & EBT_802_3))
		xtables_error(PARAMETER_PROBLEM,
			      "For 802.3 DSAP/SSAP filtering the protocol must be LENGTH");

	if (!strcmp(m->u.user.name, "ip") && fw->ethproto != htons(ETH_P_IP))
		xtables_error(PARAMETER_PROBLEM,
			      "For IP filtering the protocol must be specified as IPv4.");

	if (!strcmp(m->u.user.name, "ip6") && fw->ethproto != htons(ETH_P_IPV6))
		xtables_error(PARAMETER_PROBLEM,
			      "For IPv6 filtering the protocol must be specified as IPv6.");

	return add_match(h, ctx, r, m);
}

static int nft_bridge_add(struct nft_handle *h, struct nft_rule_ctx *ctx,
			  struct nftnl_rule *r,
			  struct iptables_command_state *cs)
{
	struct ebt_match *iter;
	struct ebt_entry *fw = &cs->eb;
	uint32_t op;

	if (fw->bitmask & EBT_SOURCEMAC) {
		op = nft_invflags2cmp(fw->invflags, EBT_ISOURCE);
		add_addr(h, r, NFT_PAYLOAD_LL_HEADER,
			 offsetof(struct ethhdr, h_source),
			 fw->sourcemac, fw->sourcemsk, ETH_ALEN, op);
	}

	if (fw->bitmask & EBT_DESTMAC) {
		op = nft_invflags2cmp(fw->invflags, EBT_IDEST);
		add_addr(h, r, NFT_PAYLOAD_LL_HEADER,
			 offsetof(struct ethhdr, h_dest),
			 fw->destmac, fw->destmsk, ETH_ALEN, op);
	}

	if (fw->in[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_IIN);
		add_iface(h, r, fw->in, NFT_META_IIFNAME, op);
	}

	if (fw->out[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_IOUT);
		add_iface(h, r, fw->out, NFT_META_OIFNAME, op);
	}

	if (fw->logical_in[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_ILOGICALIN);
		add_iface(h, r, fw->logical_in, NFT_META_BRI_IIFNAME, op);
	}

	if (fw->logical_out[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_ILOGICALOUT);
		add_iface(h, r, fw->logical_out, NFT_META_BRI_OIFNAME, op);
	}

	if ((fw->bitmask & EBT_NOPROTO) == 0) {
		uint16_t ethproto = fw->ethproto;
		uint8_t reg;

		op = nft_invflags2cmp(fw->invflags, EBT_IPROTO);
		add_payload(h, r, offsetof(struct ethhdr, h_proto), 2,
			    NFT_PAYLOAD_LL_HEADER, &reg);

		if (fw->bitmask & EBT_802_3) {
			op = (op == NFT_CMP_EQ ? NFT_CMP_LT : NFT_CMP_GTE);
			ethproto = htons(0x0600);
		}

		add_cmp_u16(r, ethproto, op, reg);
	}

	add_compat(r, fw->ethproto, fw->invflags & EBT_IPROTO);

	for (iter = cs->match_list; iter; iter = iter->next) {
		if (iter->ismatch) {
			if (nft_bridge_add_match(h, fw, ctx, r, iter->u.match->m))
				break;
		} else {
			if (add_target(r, iter->u.watcher->t))
				break;
		}
	}

	if (add_counters(r, cs->counters.pcnt, cs->counters.bcnt) < 0)
		return -1;

	return _add_action(r, cs);
}

static void nft_bridge_init_cs(struct iptables_command_state *cs)
{
	cs->eb.bitmask = EBT_NOPROTO;
}

static void print_iface(const char *option, const char *name, bool invert)
{
	if (*name && (strcmp(name, "+") || invert))
		printf("%s%s %s ", invert ? "! " : "", option, name);
}

static void nft_bridge_print_table_header(const char *tablename)
{
	printf("Bridge table: %s\n\n", tablename);
}

static void nft_bridge_print_header(unsigned int format, const char *chain,
				    const char *pol,
				    const struct xt_counters *counters,
				    int refs, uint32_t entries)
{
	printf("Bridge chain: %s, entries: %u, policy: %s\n",
	       chain, entries, pol ?: "RETURN");
}

static void print_matches_and_watchers(const struct iptables_command_state *cs,
				       unsigned int format)
{
	struct xtables_target *watcherp;
	struct xtables_match *matchp;
	struct ebt_match *m;

	for (m = cs->match_list; m; m = m->next) {
		if (m->ismatch) {
			matchp = m->u.match;
			if (matchp->print != NULL) {
				matchp->print(&cs->eb, matchp->m,
					      format & FMT_NUMERIC);
			}
		} else {
			watcherp = m->u.watcher;
			if (watcherp->print != NULL) {
				watcherp->print(&cs->eb, watcherp->t,
						format & FMT_NUMERIC);
			}
		}
	}
}

static void print_mac(char option, const unsigned char *mac,
		      const unsigned char *mask,
		      bool invert)
{
	printf("%s-%c ", invert ? "! " : "", option);
	ebt_print_mac_and_mask(mac, mask);
	printf(" ");
}


static void print_protocol(uint16_t ethproto, bool invert, unsigned int bitmask)
{
	struct xt_ethertypeent *ent;

	/* Dont print anything about the protocol if no protocol was
	 * specified, obviously this means any protocol will do. */
	if (bitmask & EBT_NOPROTO)
		return;

	printf("%s-p ", invert ? "! " : "");

	if (bitmask & EBT_802_3) {
		printf("Length ");
		return;
	}

	ent = xtables_getethertypebynumber(ntohs(ethproto));
	if (!ent)
		printf("0x%x ", ntohs(ethproto));
	else
		printf("%s ", ent->e_name);
}

static void __nft_bridge_save_rule(const struct iptables_command_state *cs,
				   unsigned int format)
{
	if (!(cs->eb.bitmask & EBT_NOPROTO))
		print_protocol(cs->eb.ethproto, cs->eb.invflags & EBT_IPROTO,
			       cs->eb.bitmask);
	if (cs->eb.bitmask & EBT_ISOURCE)
		print_mac('s', cs->eb.sourcemac, cs->eb.sourcemsk,
		          cs->eb.invflags & EBT_ISOURCE);
	if (cs->eb.bitmask & EBT_IDEST)
		print_mac('d', cs->eb.destmac, cs->eb.destmsk,
		          cs->eb.invflags & EBT_IDEST);

	print_iface("-i", cs->eb.in, cs->eb.invflags & EBT_IIN);
	print_iface("--logical-in", cs->eb.logical_in,
		    cs->eb.invflags & EBT_ILOGICALIN);
	print_iface("-o", cs->eb.out, cs->eb.invflags & EBT_IOUT);
	print_iface("--logical-out", cs->eb.logical_out,
		    cs->eb.invflags & EBT_ILOGICALOUT);

	print_matches_and_watchers(cs, format);

	printf("-j ");

	if (cs->jumpto != NULL) {
		if (strcmp(cs->jumpto, "") != 0)
			printf("%s", cs->jumpto);
		else
			printf("CONTINUE");
	}
	if (cs->target != NULL && cs->target->print != NULL) {
		printf(" ");
		cs->target->print(&cs->fw, cs->target->t, format & FMT_NUMERIC);
	}

	if ((format & (FMT_NOCOUNTS | FMT_C_COUNTS)) == FMT_C_COUNTS) {
		if (format & FMT_EBT_SAVE)
			printf(" -c %"PRIu64" %"PRIu64"",
			       (uint64_t)cs->counters.pcnt,
			       (uint64_t)cs->counters.bcnt);
		else
			printf(" , pcnt = %"PRIu64" -- bcnt = %"PRIu64"",
			       (uint64_t)cs->counters.pcnt,
			       (uint64_t)cs->counters.bcnt);
	}

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);
}

static void nft_bridge_save_rule(const struct iptables_command_state *cs,
				 unsigned int format)
{
	printf(" ");
	__nft_bridge_save_rule(cs, format);
}

static void nft_bridge_print_rule(struct nft_handle *h, struct nftnl_rule *r,
				  unsigned int num, unsigned int format)
{
	struct iptables_command_state cs = {};

	if (format & FMT_LINENUMBERS)
		printf("%d. ", num);

	nft_bridge_init_cs(&cs);
	nft_rule_to_iptables_command_state(h, r, &cs);
	__nft_bridge_save_rule(&cs, format);
	ebt_cs_clean(&cs);
}

static void nft_bridge_save_chain(const struct nftnl_chain *c,
				  const char *policy)
{
	const char *chain = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);

	printf(":%s %s\n", chain, policy ?: "ACCEPT");
}

static bool nft_bridge_is_same(const struct iptables_command_state *cs_a,
			       const struct iptables_command_state *cs_b)
{
	const struct ebt_entry *a = &cs_a->eb;
	const struct ebt_entry *b = &cs_b->eb;
	int i;

	if (a->ethproto != b->ethproto ||
	    a->bitmask != b->bitmask ||
	    a->invflags != b->invflags) {
		DEBUGP("different proto/bitmask/invflags\n");
		return false;
	}

	for (i = 0; i < ETH_ALEN; i++) {
		if (a->sourcemac[i] != b->sourcemac[i]) {
			DEBUGP("different source mac %x, %x (%d)\n",
			a->sourcemac[i] & 0xff, b->sourcemac[i] & 0xff, i);
			return false;
		}

		if (a->destmac[i] != b->destmac[i]) {
			DEBUGP("different destination mac %x, %x (%d)\n",
			a->destmac[i] & 0xff, b->destmac[i] & 0xff, i);
			return false;
		}
	}

	for (i = 0; i < IFNAMSIZ; i++) {
		if (a->logical_in[i] != b->logical_in[i]) {
			DEBUGP("different logical iniface %x, %x (%d)\n",
			a->logical_in[i] & 0xff, b->logical_in[i] & 0xff, i);
			return false;
		}

		if (a->logical_out[i] != b->logical_out[i]) {
			DEBUGP("different logical outiface %x, %x (%d)\n",
			a->logical_out[i] & 0xff, b->logical_out[i] & 0xff, i);
			return false;
		}
	}

	return strcmp(a->in, b->in) == 0 && strcmp(a->out, b->out) == 0;
}

static int xlate_ebmatches(const struct iptables_command_state *cs, struct xt_xlate *xl)
{
	int ret = 1, numeric = cs->options & OPT_NUMERIC;
	struct ebt_match *m;

	for (m = cs->match_list; m; m = m->next) {
		if (m->ismatch) {
			struct xtables_match *matchp = m->u.match;
			struct xt_xlate_mt_params mt_params = {
				.ip		= (const void *)&cs->eb,
				.numeric	= numeric,
				.match		= matchp->m,
			};

			if (!matchp->xlate)
				return 0;

			ret = matchp->xlate(xl, &mt_params);
		} else {
			struct xtables_target *watcherp = m->u.watcher;
			struct xt_xlate_tg_params wt_params = {
				.ip		= (const void *)&cs->eb,
				.numeric	= numeric,
				.target		= watcherp->t,
			};

			if (!watcherp->xlate)
				return 0;

			ret = watcherp->xlate(xl, &wt_params);
		}

		if (!ret)
			break;
	}

	return ret;
}

static int xlate_ebaction(const struct iptables_command_state *cs, struct xt_xlate *xl)
{
	int ret = 1, numeric = cs->options & OPT_NUMERIC;

	/* If no target at all, add nothing (default to continue) */
	if (cs->target != NULL) {
		/* Standard target? */
		if (strcmp(cs->jumpto, XTC_LABEL_ACCEPT) == 0)
			xt_xlate_add(xl, " accept");
		else if (strcmp(cs->jumpto, XTC_LABEL_DROP) == 0)
			xt_xlate_add(xl, " drop");
		else if (strcmp(cs->jumpto, XTC_LABEL_RETURN) == 0)
			xt_xlate_add(xl, " return");
		else if (cs->target->xlate) {
			struct xt_xlate_tg_params params = {
				.ip		= (const void *)&cs->eb,
				.target		= cs->target->t,
				.numeric	= numeric,
			};
			ret = cs->target->xlate(xl, &params);
		}
		else
			return 0;
	} else if (cs->jumpto == NULL) {
	} else if (strlen(cs->jumpto) > 0)
		xt_xlate_add(xl, " jump %s", cs->jumpto);

	return ret;
}

static void xlate_mac(struct xt_xlate *xl, const unsigned char *mac)
{
	int i;

	xt_xlate_add(xl, "%02x", mac[0]);

	for (i=1; i < ETH_ALEN; i++)
		xt_xlate_add(xl, ":%02x", mac[i]);
}

static void nft_bridge_xlate_mac(struct xt_xlate *xl, const char *type, bool invert,
				 const unsigned char *mac, const unsigned char *mask)
{
	char one_msk[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	xt_xlate_add(xl, "ether %s %s", type, invert ? "!= " : "");

	if (memcmp(mask, one_msk, ETH_ALEN)) {
		int i;
		xt_xlate_add(xl, "and");

		xlate_mac(xl, mask);

		xt_xlate_add(xl, " == %02x", mac[0] & mask[0]);
		for (i=1; i < ETH_ALEN; i++)
			xt_xlate_add(xl, ":%02x", mac[i] & mask[i]);
	} else {
		xlate_mac(xl, mac);
	}
}

static int nft_bridge_xlate(const struct iptables_command_state *cs,
			    struct xt_xlate *xl)
{
	int ret;

	xlate_ifname(xl, "iifname", cs->eb.in,
		     cs->eb.invflags & EBT_IIN);
	xlate_ifname(xl, "meta ibrname", cs->eb.logical_in,
		     cs->eb.invflags & EBT_ILOGICALIN);
	xlate_ifname(xl, "oifname", cs->eb.out,
		     cs->eb.invflags & EBT_IOUT);
	xlate_ifname(xl, "meta obrname", cs->eb.logical_out,
		     cs->eb.invflags & EBT_ILOGICALOUT);

	if (cs->eb.bitmask & EBT_802_3) {
		xt_xlate_add(xl, "ether type %s 0x0600 ",
			     cs->eb.invflags & EBT_IPROTO ? ">=" : "<");
	} else if ((cs->eb.bitmask & EBT_NOPROTO) == 0) {
		const char *implicit = NULL;

		switch (ntohs(cs->eb.ethproto)) {
		case ETH_P_IP:
			implicit = "ip";
			break;
		case ETH_P_IPV6:
			implicit = "ip6";
			break;
		case ETH_P_8021Q:
			implicit = "vlan";
			break;
		default:
			break;
		}

		if (!implicit || !xlate_find_match(cs, implicit))
			xt_xlate_add(xl, "ether type %s0x%x ",
				     cs->eb.invflags & EBT_IPROTO ? "!= " : "",
				     ntohs(cs->eb.ethproto));
	}

	if (cs->eb.bitmask & EBT_ISOURCE)
		nft_bridge_xlate_mac(xl, "saddr", cs->eb.invflags & EBT_ISOURCE,
				     cs->eb.sourcemac, cs->eb.sourcemsk);
	if (cs->eb.bitmask & EBT_IDEST)
		nft_bridge_xlate_mac(xl, "daddr", cs->eb.invflags & EBT_IDEST,
				     cs->eb.destmac, cs->eb.destmsk);
	ret = xlate_ebmatches(cs, xl);
	if (ret == 0)
		return ret;

	/* Always add counters per rule, as in ebtables */
	xt_xlate_add(xl, "counter");
	ret = xlate_ebaction(cs, xl);

	return ret;
}

static const char *nft_bridge_option_name(int option)
{
	switch (option) {
	/* ebtables specific ones */
	case OPT_LOGICALIN:	return "--logical-in";
	case OPT_LOGICALOUT:	return "--logical-out";
	case OPT_LINENUMBERS:	return "--Ln";
	case OPT_LIST_C:	return "--Lc";
	case OPT_LIST_X:	return "--Lx";
	case OPT_LIST_MAC2:	return "--Lmac2";
	default:		return ip46t_option_name(option);
	}
}

static int nft_bridge_option_invert(int option)
{
	switch (option) {
	case OPT_SOURCE:	return EBT_ISOURCE;
	case OPT_DESTINATION:	return EBT_IDEST;
	case OPT_PROTOCOL:	return EBT_IPROTO;
	case OPT_VIANAMEIN:	return EBT_IIN;
	case OPT_VIANAMEOUT:	return EBT_IOUT;
	case OPT_LOGICALIN:	return EBT_ILOGICALIN;
	case OPT_LOGICALOUT:	return EBT_ILOGICALOUT;
	default:		return -1;
	}
}

static void nft_bridge_proto_parse(struct iptables_command_state *cs,
				   struct xtables_args *args)
{
	char *buffer;
	int i;

	cs->eb.bitmask &= ~((unsigned int)EBT_NOPROTO);

	i = strtol(cs->protocol, &buffer, 16);
	if (*buffer == '\0' && (i < 0 || i > 0xFFFF))
		xtables_error(PARAMETER_PROBLEM,
			      "Problem with the specified protocol");
	if (*buffer != '\0') {
		struct xt_ethertypeent *ent;

		if (!strcmp(cs->protocol, "length")) {
			cs->eb.bitmask |= EBT_802_3;
			return;
		}
		ent = xtables_getethertypebyname(cs->protocol);
		if (!ent)
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with the specified Ethernet protocol '%s', perhaps "XT_PATH_ETHERTYPES " is missing",
				      cs->protocol);
		cs->eb.ethproto = ent->e_ethertype;
	} else
		cs->eb.ethproto = i;

	if (cs->eb.ethproto < 0x0600)
		xtables_error(PARAMETER_PROBLEM,
			      "Sorry, protocols have values above or equal to 0x0600");
}

static void nft_bridge_post_parse(int command,
				  struct iptables_command_state *cs,
				  struct xtables_args *args)
{
	struct ebt_match *match;

	cs->eb.invflags = args->invflags;

	memcpy(cs->eb.in, args->iniface, IFNAMSIZ);
	memcpy(cs->eb.out, args->outiface, IFNAMSIZ);
	memcpy(cs->eb.logical_in, args->bri_iniface, IFNAMSIZ);
	memcpy(cs->eb.logical_out, args->bri_outiface, IFNAMSIZ);

	cs->counters.pcnt = args->pcnt_cnt;
	cs->counters.bcnt = args->bcnt_cnt;

	if (args->shostnetworkmask) {
		if (xtables_parse_mac_and_mask(args->shostnetworkmask,
					       cs->eb.sourcemac,
					       cs->eb.sourcemsk))
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with specified source mac '%s'",
				      args->shostnetworkmask);
		cs->eb.bitmask |= EBT_SOURCEMAC;
	}
	if (args->dhostnetworkmask) {
		if (xtables_parse_mac_and_mask(args->dhostnetworkmask,
					       cs->eb.destmac,
					       cs->eb.destmsk))
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with specified destination mac '%s'",
				      args->dhostnetworkmask);
		cs->eb.bitmask |= EBT_DESTMAC;
	}

	if ((cs->options & (OPT_LIST_X | OPT_LINENUMBERS)) ==
			(OPT_LIST_X | OPT_LINENUMBERS))
		xtables_error(PARAMETER_PROBLEM,
			      "--Lx is not compatible with --Ln");

	/* So, the extensions can work with the host endian.
	 * The kernel does not have to do this of course */
	cs->eb.ethproto = htons(cs->eb.ethproto);

	for (match = cs->match_list; match; match = match->next) {
		if (match->ismatch)
			continue;

		xtables_option_tfcall(match->u.watcher);
	}
}

struct nft_family_ops nft_family_ops_bridge = {
	.add			= nft_bridge_add,
	.is_same		= nft_bridge_is_same,
	.print_payload		= NULL,
	.rule_parse		= &nft_ruleparse_ops_bridge,
	.cmd_parse		= {
		.proto_parse	= nft_bridge_proto_parse,
		.post_parse	= nft_bridge_post_parse,
		.option_name	= nft_bridge_option_name,
		.option_invert	= nft_bridge_option_invert,
		.command_default = ebt_command_default,
		.print_help	= nft_bridge_print_help,
	},
	.print_table_header	= nft_bridge_print_table_header,
	.print_header		= nft_bridge_print_header,
	.print_rule		= nft_bridge_print_rule,
	.save_rule		= nft_bridge_save_rule,
	.save_chain		= nft_bridge_save_chain,
	.rule_to_cs		= nft_rule_to_iptables_command_state,
	.init_cs		= nft_bridge_init_cs,
	.clear_cs		= ebt_cs_clean,
	.xlate			= nft_bridge_xlate,
};
