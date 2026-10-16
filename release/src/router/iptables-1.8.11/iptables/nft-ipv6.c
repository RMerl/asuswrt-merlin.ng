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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netdb.h>

#include <xtables.h>

#include <linux/netfilter/nf_tables.h>
#include "nft.h"
#include "nft-shared.h"

static int nft_ipv6_add(struct nft_handle *h, struct nft_rule_ctx *ctx,
			struct nftnl_rule *r, struct iptables_command_state *cs)
{
	struct xtables_rule_match *matchp;
	uint32_t op;
	int ret;

	if (!IN6_IS_ADDR_UNSPECIFIED(&cs->fw6.ipv6.src) ||
	    !IN6_IS_ADDR_UNSPECIFIED(&cs->fw6.ipv6.smsk) ||
	    (cs->fw6.ipv6.invflags & IPT_INV_SRCIP)) {
		op = nft_invflags2cmp(cs->fw6.ipv6.invflags, IPT_INV_SRCIP);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 offsetof(struct ip6_hdr, ip6_src),
			 &cs->fw6.ipv6.src, &cs->fw6.ipv6.smsk,
			 sizeof(struct in6_addr), op);
	}

	if (!IN6_IS_ADDR_UNSPECIFIED(&cs->fw6.ipv6.dst) ||
	    !IN6_IS_ADDR_UNSPECIFIED(&cs->fw6.ipv6.dmsk) ||
	    (cs->fw6.ipv6.invflags & IPT_INV_DSTIP)) {
		op = nft_invflags2cmp(cs->fw6.ipv6.invflags, IPT_INV_DSTIP);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 offsetof(struct ip6_hdr, ip6_dst),
			 &cs->fw6.ipv6.dst, &cs->fw6.ipv6.dmsk,
			 sizeof(struct in6_addr), op);
	}

	if (cs->fw6.ipv6.iniface[0] != '\0') {
		op = nft_invflags2cmp(cs->fw6.ipv6.invflags, IPT_INV_VIA_IN);
		add_iface(h, r, cs->fw6.ipv6.iniface, NFT_META_IIFNAME, op);
	}

	if (cs->fw6.ipv6.outiface[0] != '\0') {
		op = nft_invflags2cmp(cs->fw6.ipv6.invflags, IPT_INV_VIA_OUT);
		add_iface(h, r, cs->fw6.ipv6.outiface, NFT_META_OIFNAME, op);
	}

	if (cs->fw6.ipv6.proto != 0) {
		op = nft_invflags2cmp(cs->fw6.ipv6.invflags, XT_INV_PROTO);
		add_l4proto(h, r, cs->fw6.ipv6.proto, op);
	}

	add_compat(r, cs->fw6.ipv6.proto, cs->fw6.ipv6.invflags & XT_INV_PROTO);

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

	return add_action(r, cs, !!(cs->fw6.ipv6.flags & IP6T_F_GOTO));
}

static bool nft_ipv6_is_same(const struct iptables_command_state *a,
			     const struct iptables_command_state *b)
{
	if (memcmp(a->fw6.ipv6.src.s6_addr, b->fw6.ipv6.src.s6_addr,
		   sizeof(struct in6_addr)) != 0
	    || memcmp(a->fw6.ipv6.dst.s6_addr, b->fw6.ipv6.dst.s6_addr,
		    sizeof(struct in6_addr)) != 0
	    || a->fw6.ipv6.proto != b->fw6.ipv6.proto
	    || a->fw6.ipv6.flags != b->fw6.ipv6.flags
	    || a->fw6.ipv6.invflags != b->fw6.ipv6.invflags) {
		DEBUGP("different src/dst/proto/flags/invflags\n");
		return false;
	}

	return is_same_interfaces(a->fw6.ipv6.iniface, a->fw6.ipv6.outiface,
				  a->fw6.ipv6.iniface_mask,
				  a->fw6.ipv6.outiface_mask,
				  b->fw6.ipv6.iniface, b->fw6.ipv6.outiface,
				  b->fw6.ipv6.iniface_mask,
				  b->fw6.ipv6.outiface_mask);
}

static void nft_ipv6_set_goto_flag(struct iptables_command_state *cs)
{
	cs->fw6.ipv6.flags |= IP6T_F_GOTO;
}

static void nft_ipv6_print_rule(struct nft_handle *h, struct nftnl_rule *r,
				unsigned int num, unsigned int format)
{
	struct iptables_command_state cs = {};

	nft_rule_to_iptables_command_state(h, r, &cs);

	print_rule_details(num, &cs.counters, cs.jumpto, cs.fw6.ipv6.proto,
			   cs.fw6.ipv6.flags, cs.fw6.ipv6.invflags, format);
	print_fragment(cs.fw6.ipv6.flags, cs.fw6.ipv6.invflags, format, true);
	print_ifaces(cs.fw6.ipv6.iniface, cs.fw6.ipv6.outiface,
		     cs.fw6.ipv6.invflags, format);
	print_ipv6_addresses(&cs.fw6, format);

	if (format & FMT_NOTABLE)
		fputs("  ", stdout);

	if (cs.fw6.ipv6.flags & IP6T_F_GOTO)
		printf("[goto] ");

	print_matches_and_target(&cs, format);

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);

	xtables_clear_iptables_command_state(&cs);
}

static void nft_ipv6_save_rule(const struct iptables_command_state *cs,
			       unsigned int format)
{
	save_ipv6_addr('s', &cs->fw6.ipv6.src, &cs->fw6.ipv6.smsk,
		       cs->fw6.ipv6.invflags & IP6T_INV_SRCIP);
	save_ipv6_addr('d', &cs->fw6.ipv6.dst, &cs->fw6.ipv6.dmsk,
		       cs->fw6.ipv6.invflags & IP6T_INV_DSTIP);

	save_rule_details(cs->fw6.ipv6.iniface, cs->fw6.ipv6.outiface,
			  cs->fw6.ipv6.proto, 0, cs->fw6.ipv6.invflags);

	save_matches_and_target(cs, cs->fw6.ipv6.flags & IP6T_F_GOTO,
				&cs->fw6, format);
}

static void xlate_ipv6_addr(const char *selector, const struct in6_addr *addr,
			    const struct in6_addr *mask,
			    int invert, struct xt_xlate *xl)
{
	const char *op = invert ? "!= " : "";
	char addr_str[INET6_ADDRSTRLEN];
	int cidr;

	if (!invert && IN6_IS_ADDR_UNSPECIFIED(addr) && IN6_IS_ADDR_UNSPECIFIED(mask))
		return;

	inet_ntop(AF_INET6, addr, addr_str, INET6_ADDRSTRLEN);
	cidr = xtables_ip6mask_to_cidr(mask);
	switch (cidr) {
	case -1:
		xt_xlate_add(xl, "%s & %s %s %s ", selector,
			     xtables_ip6addr_to_numeric(mask),
			     invert ? "!=" : "==", addr_str);
		break;
	case 128:
		xt_xlate_add(xl, "%s %s%s ", selector, op, addr_str);
		break;
	default:
		xt_xlate_add(xl, "%s %s%s/%d ", selector, op, addr_str, cidr);
	}
}

static int nft_ipv6_xlate(const struct iptables_command_state *cs,
			  struct xt_xlate *xl)
{
	uint16_t proto = cs->fw6.ipv6.proto;
	const char *comment;
	int ret;

	xlate_ifname(xl, "iifname", cs->fw6.ipv6.iniface,
		     cs->fw6.ipv6.invflags & IP6T_INV_VIA_IN);
	xlate_ifname(xl, "oifname", cs->fw6.ipv6.outiface,
		     cs->fw6.ipv6.invflags & IP6T_INV_VIA_OUT);

	if (proto != 0 && !xlate_find_protomatch(cs, proto)) {
		const char *pname = proto_to_name(proto, 0);

		xt_xlate_add(xl, "meta l4proto");
		if (cs->fw6.ipv6.invflags & IP6T_INV_PROTO)
			xt_xlate_add(xl, " !=");
		if (pname)
			xt_xlate_add(xl, "%s", pname);
		else
			xt_xlate_add(xl, "%hu", proto);
	}

	xlate_ipv6_addr("ip6 saddr", &cs->fw6.ipv6.src, &cs->fw6.ipv6.smsk,
			cs->fw6.ipv6.invflags & IP6T_INV_SRCIP, xl);
	xlate_ipv6_addr("ip6 daddr", &cs->fw6.ipv6.dst, &cs->fw6.ipv6.dmsk,
			cs->fw6.ipv6.invflags & IP6T_INV_DSTIP, xl);

	ret = xlate_matches(cs, xl);
	if (!ret)
		return ret;

	/* Always add counters per rule, as in iptables */
	xt_xlate_add(xl, "counter");
	ret = xlate_action(cs, !!(cs->fw6.ipv6.flags & IP6T_F_GOTO), xl);

	comment = xt_xlate_get_comment(xl);
	if (comment)
		xt_xlate_add(xl, " comment %s", comment);

	return ret;
}

static int
nft_ipv6_add_entry(struct nft_handle *h,
		   const char *chain, const char *table,
		   struct iptables_command_state *cs,
		   struct xtables_args *args, bool verbose,
		   bool append, int rulenum)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		memcpy(&cs->fw6.ipv6.src,
		       &args->s.addr.v6[i], sizeof(struct in6_addr));
		memcpy(&cs->fw6.ipv6.smsk,
		       &args->s.mask.v6[i], sizeof(struct in6_addr));
		for (j = 0; j < args->d.naddrs; j++) {
			memcpy(&cs->fw6.ipv6.dst,
			       &args->d.addr.v6[j], sizeof(struct in6_addr));
			memcpy(&cs->fw6.ipv6.dmsk,
			       &args->d.mask.v6[j], sizeof(struct in6_addr));
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
nft_ipv6_delete_entry(struct nft_handle *h,
		      const char *chain, const char *table,
		      struct iptables_command_state *cs,
		      struct xtables_args *args, bool verbose)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		memcpy(&cs->fw6.ipv6.src,
		       &args->s.addr.v6[i], sizeof(struct in6_addr));
		memcpy(&cs->fw6.ipv6.smsk,
		       &args->s.mask.v6[i], sizeof(struct in6_addr));
		for (j = 0; j < args->d.naddrs; j++) {
			memcpy(&cs->fw6.ipv6.dst,
			       &args->d.addr.v6[j], sizeof(struct in6_addr));
			memcpy(&cs->fw6.ipv6.dmsk,
			       &args->d.mask.v6[j], sizeof(struct in6_addr));
			ret = nft_cmd_rule_delete(h, chain, table, cs, verbose);
		}
	}

	return ret;
}

static int
nft_ipv6_check_entry(struct nft_handle *h,
		     const char *chain, const char *table,
		     struct iptables_command_state *cs,
		     struct xtables_args *args, bool verbose)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		memcpy(&cs->fw6.ipv6.src,
		       &args->s.addr.v6[i], sizeof(struct in6_addr));
		memcpy(&cs->fw6.ipv6.smsk,
		       &args->s.mask.v6[i], sizeof(struct in6_addr));
		for (j = 0; j < args->d.naddrs; j++) {
			memcpy(&cs->fw6.ipv6.dst,
			       &args->d.addr.v6[j], sizeof(struct in6_addr));
			memcpy(&cs->fw6.ipv6.dmsk,
			       &args->d.mask.v6[j], sizeof(struct in6_addr));
			ret = nft_cmd_rule_check(h, chain, table, cs, verbose);
		}
	}

	return ret;
}

static int
nft_ipv6_replace_entry(struct nft_handle *h,
		       const char *chain, const char *table,
		       struct iptables_command_state *cs,
		       struct xtables_args *args, bool verbose,
		       int rulenum)
{
	memcpy(&cs->fw6.ipv6.src, args->s.addr.v6, sizeof(struct in6_addr));
	memcpy(&cs->fw6.ipv6.dst, args->d.addr.v6, sizeof(struct in6_addr));
	memcpy(&cs->fw6.ipv6.smsk, args->s.mask.v6, sizeof(struct in6_addr));
	memcpy(&cs->fw6.ipv6.dmsk, args->d.mask.v6, sizeof(struct in6_addr));

	return nft_cmd_rule_replace(h, chain, table, cs, rulenum, verbose);
}

struct nft_family_ops nft_family_ops_ipv6 = {
	.add			= nft_ipv6_add,
	.is_same		= nft_ipv6_is_same,
	.set_goto_flag		= nft_ipv6_set_goto_flag,
	.print_header		= print_header,
	.print_rule		= nft_ipv6_print_rule,
	.save_rule		= nft_ipv6_save_rule,
	.save_chain		= nft_ipv46_save_chain,
	.rule_parse		= &nft_ruleparse_ops_ipv6,
	.cmd_parse		= {
		.proto_parse	= ipv6_proto_parse,
		.post_parse	= ipv6_post_parse,
		.option_name	= ip46t_option_name,
		.option_invert	= ip46t_option_invert,
		.command_default = command_default,
		.print_help	= xtables_printhelp,
	},
	.rule_to_cs		= nft_rule_to_iptables_command_state,
	.clear_cs		= xtables_clear_iptables_command_state,
	.xlate			= nft_ipv6_xlate,
	.add_entry		= nft_ipv6_add_entry,
	.delete_entry		= nft_ipv6_delete_entry,
	.check_entry		= nft_ipv6_check_entry,
	.replace_entry		= nft_ipv6_replace_entry,
};
