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

#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <xtables.h>

#include <linux/netfilter/nf_tables.h>

#include "nft.h"
#include "nft-shared.h"

static int nft_ipv4_add(struct nft_handle *h, struct nft_rule_ctx *ctx,
			struct nftnl_rule *r, struct iptables_command_state *cs)
{
	struct xtables_rule_match *matchp;
	uint32_t op;
	int ret;

	if (cs->fw.ip.src.s_addr || cs->fw.ip.smsk.s_addr || cs->fw.ip.invflags & IPT_INV_SRCIP) {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_SRCIP);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 offsetof(struct iphdr, saddr),
			 &cs->fw.ip.src.s_addr, &cs->fw.ip.smsk.s_addr,
			 sizeof(struct in_addr), op);
	}

	if (cs->fw.ip.dst.s_addr || cs->fw.ip.dmsk.s_addr || cs->fw.ip.invflags & IPT_INV_DSTIP) {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_DSTIP);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 offsetof(struct iphdr, daddr),
			 &cs->fw.ip.dst.s_addr, &cs->fw.ip.dmsk.s_addr,
			 sizeof(struct in_addr), op);
	}

	if (cs->fw.ip.iniface[0] != '\0') {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_VIA_IN);
		add_iface(h, r, cs->fw.ip.iniface, NFT_META_IIFNAME, op);
	}

	if (cs->fw.ip.outiface[0] != '\0') {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_VIA_OUT);
		add_iface(h, r, cs->fw.ip.outiface, NFT_META_OIFNAME, op);
	}

	if (cs->fw.ip.proto != 0) {
		op = nft_invflags2cmp(cs->fw.ip.invflags, XT_INV_PROTO);
		add_proto(h, r, offsetof(struct iphdr, protocol),
			  sizeof(uint8_t), cs->fw.ip.proto, op);
	}

	if (cs->fw.ip.flags & IPT_F_FRAG) {
		uint8_t reg;

		add_payload(h, r, offsetof(struct iphdr, frag_off), 2,
			    NFT_PAYLOAD_NETWORK_HEADER, &reg);
		/* get the 13 bits that contain the fragment offset */
		add_bitwise_u16(h, r, htons(0x1fff), 0, reg, &reg);

		/* if offset is non-zero, this is a fragment */
		op = NFT_CMP_NEQ;
		if (cs->fw.ip.invflags & IPT_INV_FRAG)
			op = NFT_CMP_EQ;

		add_cmp_u16(r, 0, op, reg);
	}

	add_compat(r, cs->fw.ip.proto, cs->fw.ip.invflags & XT_INV_PROTO);

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		ret = add_match(h, ctx, r, matchp->match->m);
		if (ret < 0)
			return ret;
	}

	/* Counters need to me added before the target, otherwise they are
	 * increased for each rule because of the way nf_tables works.
	 */
	if (add_counters(r, cs->counters.pcnt, cs->counters.bcnt) < 0)
		return -1;

	return add_action(r, cs, !!(cs->fw.ip.flags & IPT_F_GOTO));
}

static bool nft_ipv4_is_same(const struct iptables_command_state *a,
			     const struct iptables_command_state *b)
{
	if (a->fw.ip.src.s_addr != b->fw.ip.src.s_addr
	    || a->fw.ip.dst.s_addr != b->fw.ip.dst.s_addr
	    || a->fw.ip.smsk.s_addr != b->fw.ip.smsk.s_addr
	    || a->fw.ip.dmsk.s_addr != b->fw.ip.dmsk.s_addr
	    || a->fw.ip.proto != b->fw.ip.proto
	    || a->fw.ip.flags != b->fw.ip.flags
	    || a->fw.ip.invflags != b->fw.ip.invflags) {
		DEBUGP("different src/dst/proto/flags/invflags\n");
		return false;
	}

	return is_same_interfaces(a->fw.ip.iniface, a->fw.ip.outiface,
				  a->fw.ip.iniface_mask, a->fw.ip.outiface_mask,
				  b->fw.ip.iniface, b->fw.ip.outiface,
				  b->fw.ip.iniface_mask, b->fw.ip.outiface_mask);
}

static void nft_ipv4_set_goto_flag(struct iptables_command_state *cs)
{
	cs->fw.ip.flags |= IPT_F_GOTO;
}

static void nft_ipv4_print_rule(struct nft_handle *h, struct nftnl_rule *r,
				unsigned int num, unsigned int format)
{
	struct iptables_command_state cs = {};

	nft_rule_to_iptables_command_state(h, r, &cs);

	print_rule_details(num, &cs.counters, cs.jumpto, cs.fw.ip.proto,
			   cs.fw.ip.flags, cs.fw.ip.invflags, format);
	print_fragment(cs.fw.ip.flags, cs.fw.ip.invflags, format, false);
	print_ifaces(cs.fw.ip.iniface, cs.fw.ip.outiface, cs.fw.ip.invflags,
		     format);
	print_ipv4_addresses(&cs.fw, format);

	if (format & FMT_NOTABLE)
		fputs("  ", stdout);

#ifdef IPT_F_GOTO
	if (cs.fw.ip.flags & IPT_F_GOTO)
		printf("[goto] ");
#endif

	print_matches_and_target(&cs, format);

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);

	xtables_clear_iptables_command_state(&cs);
}

static void nft_ipv4_save_rule(const struct iptables_command_state *cs,
			       unsigned int format)
{
	save_ipv4_addr('s', &cs->fw.ip.src, &cs->fw.ip.smsk,
		       cs->fw.ip.invflags & IPT_INV_SRCIP);
	save_ipv4_addr('d', &cs->fw.ip.dst, &cs->fw.ip.dmsk,
		       cs->fw.ip.invflags & IPT_INV_DSTIP);

	save_rule_details(cs->fw.ip.iniface, cs->fw.ip.outiface,
			  cs->fw.ip.proto, cs->fw.ip.flags & IPT_F_FRAG,
			  cs->fw.ip.invflags);

	save_matches_and_target(cs, cs->fw.ip.flags & IPT_F_GOTO,
				&cs->fw, format);
}

static void xlate_ipv4_addr(const char *selector, const struct in_addr *addr,
			    const struct in_addr *mask,
			    bool inv, struct xt_xlate *xl)
{
	char mbuf[INET_ADDRSTRLEN], abuf[INET_ADDRSTRLEN];
	const char *op = inv ? "!= " : "";
	int cidr;

	if (!inv && !addr->s_addr && !mask->s_addr)
		return;

	inet_ntop(AF_INET, addr, abuf, sizeof(abuf));

	cidr = xtables_ipmask_to_cidr(mask);
	switch (cidr) {
	case -1:
		xt_xlate_add(xl, "%s & %s %s %s ", selector,
			     inet_ntop(AF_INET, mask, mbuf, sizeof(mbuf)),
			     inv ? "!=" : "==", abuf);
		break;
	case 32:
		xt_xlate_add(xl, "%s %s%s ", selector, op, abuf);
		break;
	default:
		xt_xlate_add(xl, "%s %s%s/%d ", selector, op, abuf, cidr);
	}
}

static int nft_ipv4_xlate(const struct iptables_command_state *cs,
			  struct xt_xlate *xl)
{
	uint16_t proto = cs->fw.ip.proto;
	const char *comment;
	int ret;

	xlate_ifname(xl, "iifname", cs->fw.ip.iniface,
		     cs->fw.ip.invflags & IPT_INV_VIA_IN);
	xlate_ifname(xl, "oifname", cs->fw.ip.outiface,
		     cs->fw.ip.invflags & IPT_INV_VIA_OUT);

	if (cs->fw.ip.flags & IPT_F_FRAG) {
		xt_xlate_add(xl, "ip frag-off & 0x1fff %s%x ",
			   cs->fw.ip.invflags & IPT_INV_FRAG? "" : "!= ", 0);
	}

	if (proto != 0 && !xlate_find_protomatch(cs, proto)) {
		const char *pname = proto_to_name(proto, 0);

		xt_xlate_add(xl, "ip protocol");
		if (cs->fw.ip.invflags & IPT_INV_PROTO)
			xt_xlate_add(xl, " !=");
		if (pname)
			xt_xlate_add(xl, "%s", pname);
		else
			xt_xlate_add(xl, "%hu", proto);
	}

	xlate_ipv4_addr("ip saddr", &cs->fw.ip.src, &cs->fw.ip.smsk,
			cs->fw.ip.invflags & IPT_INV_SRCIP, xl);
	xlate_ipv4_addr("ip daddr", &cs->fw.ip.dst, &cs->fw.ip.dmsk,
			cs->fw.ip.invflags & IPT_INV_DSTIP, xl);

	ret = xlate_matches(cs, xl);
	if (!ret)
		return ret;

	/* Always add counters per rule, as in iptables */
	xt_xlate_add(xl, "counter");
	ret = xlate_action(cs, !!(cs->fw.ip.flags & IPT_F_GOTO), xl);

	comment = xt_xlate_get_comment(xl);
	if (comment)
		xt_xlate_add(xl, " comment %s", comment);

	return ret;
}

static int
nft_ipv4_add_entry(struct nft_handle *h,
		   const char *chain, const char *table,
		   struct iptables_command_state *cs,
		   struct xtables_args *args, bool verbose,
		   bool append, int rulenum)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		cs->fw.ip.src.s_addr = args->s.addr.v4[i].s_addr;
		cs->fw.ip.smsk.s_addr = args->s.mask.v4[i].s_addr;
		for (j = 0; j < args->d.naddrs; j++) {
			cs->fw.ip.dst.s_addr = args->d.addr.v4[j].s_addr;
			cs->fw.ip.dmsk.s_addr = args->d.mask.v4[j].s_addr;

			if (append) {
				ret = nft_cmd_rule_append(h, chain, table,
						      cs, verbose);
			} else {
				ret = nft_cmd_rule_insert(h, chain, table,
						      cs, rulenum, verbose);
			}
		}
	}

	return ret;
}

static int
nft_ipv4_delete_entry(struct nft_handle *h,
		      const char *chain, const char *table,
		      struct iptables_command_state *cs,
		      struct xtables_args *args, bool verbose)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		cs->fw.ip.src.s_addr = args->s.addr.v4[i].s_addr;
		cs->fw.ip.smsk.s_addr = args->s.mask.v4[i].s_addr;
		for (j = 0; j < args->d.naddrs; j++) {
			cs->fw.ip.dst.s_addr = args->d.addr.v4[j].s_addr;
			cs->fw.ip.dmsk.s_addr = args->d.mask.v4[j].s_addr;
			ret = nft_cmd_rule_delete(h, chain, table, cs, verbose);
		}
	}

	return ret;
}

static int
nft_ipv4_check_entry(struct nft_handle *h,
		     const char *chain, const char *table,
		     struct iptables_command_state *cs,
		     struct xtables_args *args, bool verbose)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		cs->fw.ip.src.s_addr = args->s.addr.v4[i].s_addr;
		cs->fw.ip.smsk.s_addr = args->s.mask.v4[i].s_addr;
		for (j = 0; j < args->d.naddrs; j++) {
			cs->fw.ip.dst.s_addr = args->d.addr.v4[j].s_addr;
			cs->fw.ip.dmsk.s_addr = args->d.mask.v4[j].s_addr;
			ret = nft_cmd_rule_check(h, chain, table, cs, verbose);
		}
	}

	return ret;
}

static int
nft_ipv4_replace_entry(struct nft_handle *h,
		       const char *chain, const char *table,
		       struct iptables_command_state *cs,
		       struct xtables_args *args, bool verbose,
		       int rulenum)
{
	cs->fw.ip.src.s_addr = args->s.addr.v4->s_addr;
	cs->fw.ip.dst.s_addr = args->d.addr.v4->s_addr;
	cs->fw.ip.smsk.s_addr = args->s.mask.v4->s_addr;
	cs->fw.ip.dmsk.s_addr = args->d.mask.v4->s_addr;

	return nft_cmd_rule_replace(h, chain, table, cs, rulenum, verbose);
}

struct nft_family_ops nft_family_ops_ipv4 = {
	.add			= nft_ipv4_add,
	.is_same		= nft_ipv4_is_same,
	.set_goto_flag		= nft_ipv4_set_goto_flag,
	.print_header		= print_header,
	.print_rule		= nft_ipv4_print_rule,
	.save_rule		= nft_ipv4_save_rule,
	.save_chain		= nft_ipv46_save_chain,
	.rule_parse		= &nft_ruleparse_ops_ipv4,
	.cmd_parse		= {
		.proto_parse	= ipv4_proto_parse,
		.post_parse	= ipv4_post_parse,
		.option_name	= ip46t_option_name,
		.option_invert	= ip46t_option_invert,
		.command_default = command_default,
		.print_help	= xtables_printhelp,
	},
	.rule_to_cs		= nft_rule_to_iptables_command_state,
	.clear_cs		= xtables_clear_iptables_command_state,
	.xlate			= nft_ipv4_xlate,
	.add_entry		= nft_ipv4_add_entry,
	.delete_entry		= nft_ipv4_delete_entry,
	.check_entry		= nft_ipv4_check_entry,
	.replace_entry		= nft_ipv4_replace_entry,
};
