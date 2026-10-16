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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <net/if_arp.h>

#include <xtables.h>
#include <libiptc/libxtc.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>

#include <linux/netfilter_arp/arp_tables.h>
#include <linux/netfilter/nf_tables.h>

#include "nft-shared.h"
#include "nft.h"
#include "xshared.h"

static bool need_devaddr(struct arpt_devaddr_info *info)
{
	int i;

	for (i = 0; i < ETH_ALEN; i++) {
		if (info->addr[i] || info->mask[i])
			return true;
	}

	return false;
}

static int nft_arp_add(struct nft_handle *h, struct nft_rule_ctx *ctx,
		       struct nftnl_rule *r, struct iptables_command_state *cs)
{
	struct arpt_entry *fw = &cs->arp;
	uint32_t op;
	int ret = 0;

	if (fw->arp.iniface[0] != '\0') {
		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_VIA_IN);
		add_iface(h, r, fw->arp.iniface, NFT_META_IIFNAME, op);
	}

	if (fw->arp.outiface[0] != '\0') {
		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_VIA_OUT);
		add_iface(h, r, fw->arp.outiface, NFT_META_OIFNAME, op);
	}

	if (fw->arp.arhrd != 0 ||
	    fw->arp.arhrd_mask != 0xffff ||
	    fw->arp.invflags & IPT_INV_ARPHRD) {
		uint8_t reg;

		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_ARPHRD);
		add_payload(h, r, offsetof(struct arphdr, ar_hrd), 2,
			    NFT_PAYLOAD_NETWORK_HEADER, &reg);
		if (fw->arp.arhrd_mask != 0xffff)
			add_bitwise_u16(h, r, fw->arp.arhrd_mask, 0, reg, &reg);
		add_cmp_u16(r, fw->arp.arhrd, op, reg);
	}

	if (fw->arp.arpro != 0 ||
	    fw->arp.arpro_mask != 0xffff ||
	    fw->arp.invflags & IPT_INV_PROTO) {
		uint8_t reg;

		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_PROTO);
	        add_payload(h, r, offsetof(struct arphdr, ar_pro), 2,
			    NFT_PAYLOAD_NETWORK_HEADER, &reg);
		if (fw->arp.arpro_mask != 0xffff)
			add_bitwise_u16(h, r, fw->arp.arpro_mask, 0, reg, &reg);
		add_cmp_u16(r, fw->arp.arpro, op, reg);
	}

	if (fw->arp.arhln != 0 ||
	    fw->arp.arhln_mask != 255 ||
	    fw->arp.invflags & IPT_INV_ARPHLN) {
		uint8_t reg;

		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_ARPHLN);
		add_payload(h, r, offsetof(struct arphdr, ar_hln), 1,
			    NFT_PAYLOAD_NETWORK_HEADER, &reg);
		if (fw->arp.arhln_mask != 255)
			add_bitwise(h, r, &fw->arp.arhln_mask, 1, reg, &reg);
		add_cmp_u8(r, fw->arp.arhln, op, reg);
	}

	add_proto(h, r, offsetof(struct arphdr, ar_pln), 1, 4, NFT_CMP_EQ);

	if (fw->arp.arpop != 0 ||
	    fw->arp.arpop_mask != 0xffff ||
	    fw->arp.invflags & IPT_INV_ARPOP) {
		uint8_t reg;

		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_ARPOP);
		add_payload(h, r, offsetof(struct arphdr, ar_op), 2,
			    NFT_PAYLOAD_NETWORK_HEADER, &reg);
		if (fw->arp.arpop_mask != 0xffff)
			add_bitwise_u16(h, r, fw->arp.arpop_mask, 0, reg, &reg);
		add_cmp_u16(r, fw->arp.arpop, op, reg);
	}

	if (need_devaddr(&fw->arp.src_devaddr)) {
		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_SRCDEVADDR);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 sizeof(struct arphdr),
			 &fw->arp.src_devaddr.addr,
			 &fw->arp.src_devaddr.mask,
			 fw->arp.arhln, op);

	}

	if (fw->arp.src.s_addr != 0 ||
	    fw->arp.smsk.s_addr != 0 ||
	    fw->arp.invflags & IPT_INV_SRCIP) {
		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_SRCIP);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 sizeof(struct arphdr) + fw->arp.arhln,
			 &fw->arp.src.s_addr, &fw->arp.smsk.s_addr,
			 sizeof(struct in_addr), op);
	}


	if (need_devaddr(&fw->arp.tgt_devaddr)) {
		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_TGTDEVADDR);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 sizeof(struct arphdr) + fw->arp.arhln + sizeof(struct in_addr),
			 &fw->arp.tgt_devaddr.addr,
			 &fw->arp.tgt_devaddr.mask,
			 fw->arp.arhln, op);
	}

	if (fw->arp.tgt.s_addr != 0 ||
	    fw->arp.tmsk.s_addr != 0 ||
	    fw->arp.invflags & IPT_INV_DSTIP) {
		op = nft_invflags2cmp(fw->arp.invflags, IPT_INV_DSTIP);
		add_addr(h, r, NFT_PAYLOAD_NETWORK_HEADER,
			 sizeof(struct arphdr) + fw->arp.arhln + sizeof(struct in_addr) + fw->arp.arhln,
			 &fw->arp.tgt.s_addr, &fw->arp.tmsk.s_addr,
			 sizeof(struct in_addr), op);
	}

	/* Counters need to me added before the target, otherwise they are
	 * increased for each rule because of the way nf_tables works.
	 */
	if (add_counters(r, fw->counters.pcnt, fw->counters.bcnt) < 0)
		return -1;

	if (cs->target != NULL) {
		/* Standard target? */
		if (strcmp(cs->jumpto, XTC_LABEL_ACCEPT) == 0)
			ret = add_verdict(r, NF_ACCEPT);
		else if (strcmp(cs->jumpto, XTC_LABEL_DROP) == 0)
			ret = add_verdict(r, NF_DROP);
		else if (strcmp(cs->jumpto, XTC_LABEL_RETURN) == 0)
			ret = add_verdict(r, NFT_RETURN);
		else
			ret = add_target(r, cs->target->t);
	} else if (strlen(cs->jumpto) > 0) {
		/* No goto in arptables */
		ret = add_jumpto(r, cs->jumpto, NFT_JUMP);
	}

	return ret;
}

static void nft_arp_print_header(unsigned int format, const char *chain,
				 const char *pol,
				 const struct xt_counters *counters,
				 int refs, uint32_t entries)
{
	printf("Chain %s", chain);
	if (pol) {
		printf(" (policy %s", pol);
		if (!(format & FMT_NOCOUNTS)) {
			fputc(' ', stdout);
			xtables_print_num(counters->pcnt, (format|FMT_NOTABLE));
			fputs("packets, ", stdout);
			xtables_print_num(counters->bcnt, (format|FMT_NOTABLE));
			fputs("bytes", stdout);
		}
		printf(")\n");
	} else {
		printf(" (%d references)\n", refs);
	}
}

static void print_iface(char letter, const char *iface,
			unsigned int format, bool invert, const char **sep)
{
	if (iface[0] == '\0' || (!strcmp(iface, "+") && !invert)) {
		if (!(format & FMT_VIA))
			return;
		iface = (format & FMT_NUMERIC) ? "*" : "any";
	}
	printf("%s%s-%c %s", *sep, invert ? "! " : "", letter, iface);
	*sep = " ";
}

static void nft_arp_print_rule_details(const struct iptables_command_state *cs,
				       unsigned int format)
{
	const struct arpt_entry *fw = &cs->arp;
	const char *sep = "";
	int i;

	if (strlen(cs->jumpto)) {
		printf("%s-j %s", sep, cs->jumpto);
		sep = " ";
	}

	print_iface('i', fw->arp.iniface, format,
		    fw->arp.invflags & IPT_INV_VIA_IN, &sep);
	print_iface('o', fw->arp.outiface, format,
		    fw->arp.invflags & IPT_INV_VIA_OUT, &sep);

	if (fw->arp.smsk.s_addr != 0L) {
		printf("%s%s-s %s", sep,
		       fw->arp.invflags & IPT_INV_SRCIP ? "! " : "",
		       ipv4_addr_to_string(&fw->arp.src,
					   &fw->arp.smsk, format));
		sep = " ";
	}

	for (i = 0; i < ARPT_DEV_ADDR_LEN_MAX; i++)
		if (fw->arp.src_devaddr.mask[i] != 0)
			break;
	if (i == ARPT_DEV_ADDR_LEN_MAX)
		goto after_devsrc;
	printf("%s%s", sep, fw->arp.invflags & IPT_INV_SRCDEVADDR
		? "! " : "");
	printf("--src-mac ");
	xtables_print_mac_and_mask((unsigned char *)fw->arp.src_devaddr.addr,
				   (unsigned char *)fw->arp.src_devaddr.mask);
	sep = " ";
after_devsrc:

	if (fw->arp.tmsk.s_addr != 0L) {
		printf("%s%s-d %s", sep,
		       fw->arp.invflags & IPT_INV_DSTIP ? "! " : "",
		       ipv4_addr_to_string(&fw->arp.tgt,
					   &fw->arp.tmsk, format));
		sep = " ";
	}

	for (i = 0; i <ARPT_DEV_ADDR_LEN_MAX; i++)
		if (fw->arp.tgt_devaddr.mask[i] != 0)
			break;
	if (i == ARPT_DEV_ADDR_LEN_MAX)
		goto after_devdst;
	printf("%s%s", sep, fw->arp.invflags & IPT_INV_TGTDEVADDR
		? "! " : "");
	printf("--dst-mac ");
	xtables_print_mac_and_mask((unsigned char *)fw->arp.tgt_devaddr.addr,
				   (unsigned char *)fw->arp.tgt_devaddr.mask);
	sep = " ";

after_devdst:

	if (fw->arp.arhln_mask != 255 || fw->arp.arhln != 6 ||
	    fw->arp.invflags & IPT_INV_ARPHLN) {
		printf("%s%s", sep, fw->arp.invflags & IPT_INV_ARPHLN
			? "! " : "");
		printf("--h-length %d", fw->arp.arhln);
		if (fw->arp.arhln_mask != 255)
			printf("/%d", fw->arp.arhln_mask);
		sep = " ";
	}

	if (fw->arp.arpop_mask != 65535 || fw->arp.arpop != 0 ||
	    fw->arp.invflags & IPT_INV_ARPOP) {
		int tmp = ntohs(fw->arp.arpop);

		printf("%s%s", sep, fw->arp.invflags & IPT_INV_ARPOP
			? "! " : "");
		if (tmp <= ARP_NUMOPCODES && !(format & FMT_NUMERIC))
			printf("--opcode %s", arp_opcodes[tmp-1]);
		else
			printf("--opcode %d", tmp);

		if (fw->arp.arpop_mask != 65535)
			printf("/%d", ntohs(fw->arp.arpop_mask));
		sep = " ";
	}

	if (fw->arp.arhrd_mask != 65535 || fw->arp.arhrd != htons(1) ||
	    fw->arp.invflags & IPT_INV_ARPHRD) {
		uint16_t tmp = ntohs(fw->arp.arhrd);

		printf("%s%s", sep, fw->arp.invflags & IPT_INV_ARPHRD
			? "! " : "");
		if (tmp == 1 && !(format & FMT_NUMERIC))
			printf("--h-type %s", "Ethernet");
		else
			printf("--h-type 0x%x", tmp);
		if (fw->arp.arhrd_mask != 65535)
			printf("/0x%x", ntohs(fw->arp.arhrd_mask));
		sep = " ";
	}

	if (fw->arp.arpro_mask != 65535 || fw->arp.arpro != 0 ||
	    fw->arp.invflags & IPT_INV_PROTO) {
		int tmp = ntohs(fw->arp.arpro);

		printf("%s%s", sep, fw->arp.invflags & IPT_INV_PROTO
			? "! " : "");
		if (tmp == 0x0800 && !(format & FMT_NUMERIC))
			printf("--proto-type %s", "IPv4");
		else
			printf("--proto-type 0x%x", tmp);
		if (fw->arp.arpro_mask != 65535)
			printf("/0x%x", ntohs(fw->arp.arpro_mask));
		sep = " ";
	}
}

static void
nft_arp_save_rule(const struct iptables_command_state *cs, unsigned int format)
{
	format |= FMT_NUMERIC;

	printf(" ");
	nft_arp_print_rule_details(cs, format);
	if (cs->target && cs->target->save)
		cs->target->save(&cs->fw, cs->target->t);
	printf("\n");
}

static void nft_arp_init_cs(struct iptables_command_state *cs);

static void
nft_arp_print_rule(struct nft_handle *h, struct nftnl_rule *r,
		   unsigned int num, unsigned int format)
{
	struct iptables_command_state cs = {};

	if (format & FMT_LINENUMBERS)
		printf("%u ", num);

	nft_arp_init_cs(&cs);
	nft_rule_to_iptables_command_state(h, r, &cs);

	nft_arp_print_rule_details(&cs, format);
	print_matches_and_target(&cs, format);

	if (!(format & FMT_NOCOUNTS)) {
		printf(" , pcnt=");
		xtables_print_num(cs.counters.pcnt, format | FMT_NOTABLE);
		printf("-- bcnt=");
		xtables_print_num(cs.counters.bcnt, format | FMT_NOTABLE);
	}

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);

	xtables_clear_iptables_command_state(&cs);
}

static bool nft_arp_is_same(const struct iptables_command_state *cs_a,
			    const struct iptables_command_state *cs_b)
{
	const struct arpt_entry *a = &cs_a->arp;
	const struct arpt_entry *b = &cs_b->arp;

	if (a->arp.src.s_addr != b->arp.src.s_addr
	    || a->arp.tgt.s_addr != b->arp.tgt.s_addr
	    || a->arp.smsk.s_addr != b->arp.smsk.s_addr
	    || a->arp.tmsk.s_addr != b->arp.tmsk.s_addr
	    || a->arp.arpro != b->arp.arpro
	    || a->arp.flags != b->arp.flags
	    || a->arp.invflags != b->arp.invflags) {
		DEBUGP("different src/dst/proto/flags/invflags\n");
		return false;
	}

	return is_same_interfaces(a->arp.iniface,
				  a->arp.outiface,
				  (unsigned char *)a->arp.iniface_mask,
				  (unsigned char *)a->arp.outiface_mask,
				  b->arp.iniface,
				  b->arp.outiface,
				  (unsigned char *)b->arp.iniface_mask,
				  (unsigned char *)b->arp.outiface_mask);
}

static void nft_arp_save_chain(const struct nftnl_chain *c, const char *policy)
{
	const char *chain = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);

	printf(":%s %s\n", chain, policy ?: "-");
}

static int getlength_and_mask(const char *from, uint8_t *to, uint8_t *mask)
{
	char *dup = strdup(from);
	char *p, *buffer;
	int i, ret = -1;

	if (!dup)
		return -1;

	if ( (p = strrchr(dup, '/')) != NULL) {
		*p = '\0';
		i = strtol(p+1, &buffer, 10);
		if (*buffer != '\0' || i < 0 || i > 255)
			goto out_err;
		*mask = (uint8_t)i;
	} else
		*mask = 255;
	i = strtol(dup, &buffer, 10);
	if (*buffer != '\0' || i < 0 || i > 255)
		goto out_err;
	*to = (uint8_t)i;
	ret = 0;
out_err:
	free(dup);
	return ret;

}

static int get16_and_mask(const char *from, uint16_t *to,
			  uint16_t *mask, int base)
{
	char *dup = strdup(from);
	char *p, *buffer;
	int i, ret = -1;

	if (!dup)
		return -1;

	if ( (p = strrchr(dup, '/')) != NULL) {
		*p = '\0';
		i = strtol(p+1, &buffer, base);
		if (*buffer != '\0' || i < 0 || i > 65535)
			goto out_err;
		*mask = htons((uint16_t)i);
	} else
		*mask = 65535;
	i = strtol(dup, &buffer, base);
	if (*buffer != '\0' || i < 0 || i > 65535)
		goto out_err;
	*to = htons((uint16_t)i);
	ret = 0;
out_err:
	free(dup);
	return ret;
}

static void nft_arp_post_parse(int command,
			       struct iptables_command_state *cs,
			       struct xtables_args *args)
{
	cs->arp.arp.invflags = args->invflags;

	memcpy(cs->arp.arp.iniface, args->iniface, IFNAMSIZ);
	memcpy(cs->arp.arp.iniface_mask, args->iniface_mask, IFNAMSIZ);

	memcpy(cs->arp.arp.outiface, args->outiface, IFNAMSIZ);
	memcpy(cs->arp.arp.outiface_mask, args->outiface_mask, IFNAMSIZ);

	cs->arp.counters.pcnt = args->pcnt_cnt;
	cs->arp.counters.bcnt = args->bcnt_cnt;

	if (command & (CMD_REPLACE | CMD_INSERT |
			CMD_DELETE | CMD_APPEND | CMD_CHECK)) {
		if (!(cs->options & OPT_DESTINATION))
			args->dhostnetworkmask = "0.0.0.0/0";
		if (!(cs->options & OPT_SOURCE))
			args->shostnetworkmask = "0.0.0.0/0";
	}

	if (args->shostnetworkmask)
		xtables_ipparse_multiple(args->shostnetworkmask,
					 &args->s.addr.v4, &args->s.mask.v4,
					 &args->s.naddrs);
	if (args->dhostnetworkmask)
		xtables_ipparse_multiple(args->dhostnetworkmask,
					 &args->d.addr.v4, &args->d.mask.v4,
					 &args->d.naddrs);

	if ((args->s.naddrs > 1 || args->d.naddrs > 1) &&
	    (cs->arp.arp.invflags & (IPT_INV_SRCIP | IPT_INV_DSTIP)))
		xtables_error(PARAMETER_PROBLEM,
			      "! not allowed with multiple"
			      " source or destination IP addresses");

	if (args->src_mac &&
	    xtables_parse_mac_and_mask(args->src_mac,
				       cs->arp.arp.src_devaddr.addr,
				       cs->arp.arp.src_devaddr.mask))
		xtables_error(PARAMETER_PROBLEM,
			      "Problem with specified source mac");
	if (args->dst_mac &&
	    xtables_parse_mac_and_mask(args->dst_mac,
				       cs->arp.arp.tgt_devaddr.addr,
				       cs->arp.arp.tgt_devaddr.mask))
		xtables_error(PARAMETER_PROBLEM,
			      "Problem with specified destination mac");
	if (args->arp_hlen) {
		getlength_and_mask(args->arp_hlen, &cs->arp.arp.arhln,
				   &cs->arp.arp.arhln_mask);

		if (cs->arp.arp.arhln != 6)
			xtables_error(PARAMETER_PROBLEM,
				      "Only hardware address length of 6 is supported currently.");
	}
	if (args->arp_opcode) {
		if (get16_and_mask(args->arp_opcode, &cs->arp.arp.arpop,
				   &cs->arp.arp.arpop_mask, 10)) {
			int i;

			for (i = 0; i < ARP_NUMOPCODES; i++)
				if (!strcasecmp(arp_opcodes[i],
						args->arp_opcode))
					break;
			if (i == ARP_NUMOPCODES)
				xtables_error(PARAMETER_PROBLEM,
					      "Problem with specified opcode");
			cs->arp.arp.arpop = htons(i+1);
		}
	}
	if (args->arp_htype) {
		if (get16_and_mask(args->arp_htype, &cs->arp.arp.arhrd,
				   &cs->arp.arp.arhrd_mask, 16)) {
			if (strcasecmp(args->arp_htype, "Ethernet"))
				xtables_error(PARAMETER_PROBLEM,
					      "Problem with specified hardware type");
			cs->arp.arp.arhrd = htons(1);
		}
	}
	if (args->arp_ptype) {
		if (get16_and_mask(args->arp_ptype, &cs->arp.arp.arpro,
				   &cs->arp.arp.arpro_mask, 0)) {
			if (strcasecmp(args->arp_ptype, "ipv4"))
				xtables_error(PARAMETER_PROBLEM,
					      "Problem with specified protocol type");
			cs->arp.arp.arpro = htons(0x800);
		}
	}
}

static void nft_arp_init_cs(struct iptables_command_state *cs)
{
	cs->arp.arp.arhln = 6;
	cs->arp.arp.arhln_mask = 255;
	cs->arp.arp.arhrd = htons(ARPHRD_ETHER);
	cs->arp.arp.arhrd_mask = 65535;
	cs->arp.arp.arpop_mask = 65535;
	cs->arp.arp.arpro_mask = 65535;
}

static int
nft_arp_add_entry(struct nft_handle *h,
		  const char *chain, const char *table,
		  struct iptables_command_state *cs,
		  struct xtables_args *args, bool verbose,
		  bool append, int rulenum)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		cs->arp.arp.src.s_addr = args->s.addr.v4[i].s_addr;
		cs->arp.arp.smsk.s_addr = args->s.mask.v4[i].s_addr;
		for (j = 0; j < args->d.naddrs; j++) {
			cs->arp.arp.tgt.s_addr = args->d.addr.v4[j].s_addr;
			cs->arp.arp.tmsk.s_addr = args->d.mask.v4[j].s_addr;
			if (append) {
				ret = nft_cmd_rule_append(h, chain, table, cs,
						          verbose);
			} else {
				ret = nft_cmd_rule_insert(h, chain, table, cs,
						          rulenum, verbose);
			}
		}
	}

	return ret;
}

static int
nft_arp_delete_entry(struct nft_handle *h,
		     const char *chain, const char *table,
		     struct iptables_command_state *cs,
		     struct xtables_args *args, bool verbose)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		cs->arp.arp.src.s_addr = args->s.addr.v4[i].s_addr;
		cs->arp.arp.smsk.s_addr = args->s.mask.v4[i].s_addr;
		for (j = 0; j < args->d.naddrs; j++) {
			cs->arp.arp.tgt.s_addr = args->d.addr.v4[j].s_addr;
			cs->arp.arp.tmsk.s_addr = args->d.mask.v4[j].s_addr;
			ret = nft_cmd_rule_delete(h, chain, table, cs, verbose);
		}
	}

	return ret;
}

static int
nft_arp_check_entry(struct nft_handle *h,
		    const char *chain, const char *table,
		    struct iptables_command_state *cs,
		    struct xtables_args *args, bool verbose)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		cs->arp.arp.src.s_addr = args->s.addr.v4[i].s_addr;
		cs->arp.arp.smsk.s_addr = args->s.mask.v4[i].s_addr;
		for (j = 0; j < args->d.naddrs; j++) {
			cs->arp.arp.tgt.s_addr = args->d.addr.v4[j].s_addr;
			cs->arp.arp.tmsk.s_addr = args->d.mask.v4[j].s_addr;
			ret = nft_cmd_rule_check(h, chain, table, cs, verbose);
		}
	}

	return ret;
}

static int
nft_arp_replace_entry(struct nft_handle *h,
		      const char *chain, const char *table,
		      struct iptables_command_state *cs,
		      struct xtables_args *args, bool verbose,
		      int rulenum)
{
	cs->arp.arp.src.s_addr = args->s.addr.v4->s_addr;
	cs->arp.arp.tgt.s_addr = args->d.addr.v4->s_addr;
	cs->arp.arp.smsk.s_addr = args->s.mask.v4->s_addr;
	cs->arp.arp.tmsk.s_addr = args->d.mask.v4->s_addr;

	return nft_cmd_rule_replace(h, chain, table, cs, rulenum, verbose);
}

static void nft_arp_xlate_mac_and_mask(const struct arpt_devaddr_info *devaddr,
				       const char *addr,
				       bool invert,
				       struct xt_xlate *xl)
{
	unsigned int i;

	for (i = 0; i < 6; ++i) {
		if (devaddr->mask[i])
			break;
	}

	if (i == 6)
		return;

	xt_xlate_add(xl, "arp %s ether ", addr);
	if (invert)
		xt_xlate_add(xl, "!= ");

	xt_xlate_add(xl, "%02x", (uint8_t)devaddr->addr[0]);
	for (i = 1; i < 6; ++i)
		xt_xlate_add(xl, ":%02x", (uint8_t)devaddr->addr[i]);

	for (i = 0; i < 6; ++i) {
		int j;

		if ((uint8_t)devaddr->mask[i] == 0xff)
			continue;

		xt_xlate_add(xl, "/%02x", (uint8_t)devaddr->mask[0]);

		for (j = 1; j < 6; ++j)
			xt_xlate_add(xl, ":%02x", (uint8_t)devaddr->mask[j]);
		return;
	}
}

static void nft_arp_xlate16(uint16_t v, uint16_t m, const char *what,
			    bool hex, bool inverse,
			    struct xt_xlate *xl)
{
	const char *fmt = hex ? "0x%x " : "%d ";

	if (m) {
		xt_xlate_add(xl, "arp %s ", what);
		if (inverse)
			xt_xlate_add(xl, " !=");
		if (m != 0xffff) {
			xt_xlate_add(xl, "& ");
			xt_xlate_add(xl, fmt, ntohs(m));;

		}
		xt_xlate_add(xl, fmt, ntohs(v));
	}
}

static void nft_arp_xlate_ipv4_addr(const char *what, const struct in_addr *addr,
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
		xt_xlate_add(xl, "arp %s ip & %s %s %s ", what,
			     inet_ntop(AF_INET, mask, mbuf, sizeof(mbuf)),
			     inv ? "!=" : "==", abuf);
		break;
	case 32:
		xt_xlate_add(xl, "arp %s ip %s%s ", what, op, abuf);
		break;
	default:
		xt_xlate_add(xl, "arp %s ip %s%s/%d ", what, op, abuf, cidr);
	}
}

static int nft_arp_xlate(const struct iptables_command_state *cs,
			 struct xt_xlate *xl)
{
	const struct arpt_entry *fw = &cs->arp;
	int ret;

	xlate_ifname(xl, "iifname", fw->arp.iniface,
		     fw->arp.invflags & IPT_INV_VIA_IN);
	xlate_ifname(xl, "oifname", fw->arp.outiface,
		     fw->arp.invflags & IPT_INV_VIA_OUT);

	if (fw->arp.arhrd ||
	    fw->arp.arhrd_mask != 0xffff ||
	    fw->arp.invflags & IPT_INV_ARPHRD)
		nft_arp_xlate16(fw->arp.arhrd, fw->arp.arhrd_mask,
				"htype", false,
				 fw->arp.invflags & IPT_INV_ARPHRD, xl);

	if (fw->arp.arhln_mask != 255 || fw->arp.arhln ||
	    fw->arp.invflags & IPT_INV_ARPHLN) {
		xt_xlate_add(xl, "arp hlen ");
		if (fw->arp.invflags & IPT_INV_ARPHLN)
			xt_xlate_add(xl, " !=");
		if (fw->arp.arhln_mask != 255)
			xt_xlate_add(xl, "& %d ", fw->arp.arhln_mask);
		xt_xlate_add(xl, "%d ", fw->arp.arhln);
	}

	/* added implicitly by arptables-nft */
	xt_xlate_add(xl, "arp plen %d", 4);

	if (fw->arp.arpop_mask != 65535 ||
	    fw->arp.arpop != 0 ||
	    fw->arp.invflags & IPT_INV_ARPOP)
		nft_arp_xlate16(fw->arp.arpop, fw->arp.arpop_mask,
				"operation", false,
				fw->arp.invflags & IPT_INV_ARPOP, xl);

	if (fw->arp.arpro_mask != 65535 ||
	    fw->arp.invflags & IPT_INV_PROTO ||
	    fw->arp.arpro)
		nft_arp_xlate16(fw->arp.arpro, fw->arp.arpro_mask,
				"ptype", true,
				fw->arp.invflags & IPT_INV_PROTO, xl);

	if (fw->arp.smsk.s_addr != 0L)
		nft_arp_xlate_ipv4_addr("saddr", &fw->arp.src, &fw->arp.smsk,
					fw->arp.invflags & IPT_INV_SRCIP, xl);

	if (fw->arp.tmsk.s_addr != 0L)
		nft_arp_xlate_ipv4_addr("daddr", &fw->arp.tgt, &fw->arp.tmsk,
					fw->arp.invflags & IPT_INV_DSTIP, xl);

	nft_arp_xlate_mac_and_mask(&fw->arp.src_devaddr, "saddr",
				   fw->arp.invflags & IPT_INV_SRCDEVADDR, xl);
	nft_arp_xlate_mac_and_mask(&fw->arp.tgt_devaddr, "daddr",
				   fw->arp.invflags & IPT_INV_TGTDEVADDR, xl);

	ret = xlate_matches(cs, xl);
	if (!ret)
		return ret;

	/* Always add counters per rule, as in iptables */
	xt_xlate_add(xl, "counter");
	return xlate_action(cs, false, xl);
}

static const char *nft_arp_option_name(int option)
{
	switch (option) {
	default:		return ip46t_option_name(option);
	/* different name than iptables */
	case OPT_SOURCE:	return "--source-ip";
	case OPT_DESTINATION:	return "--destination-ip";
	/* arptables specific ones */
	case OPT_S_MAC:		return "--source-mac";
	case OPT_D_MAC:		return "--destination-mac";
	case OPT_H_LENGTH:	return "--h-length";
	case OPT_OPCODE:	return "--opcode";
	case OPT_H_TYPE:	return "--h-type";
	case OPT_P_TYPE:	return "--proto-type";
	}
}

static int nft_arp_option_invert(int option)
{
	switch (option) {
	case OPT_S_MAC:		return IPT_INV_SRCDEVADDR;
	case OPT_D_MAC:		return IPT_INV_TGTDEVADDR;
	case OPT_H_LENGTH:	return IPT_INV_ARPHLN;
	case OPT_OPCODE:	return IPT_INV_ARPOP;
	case OPT_H_TYPE:	return IPT_INV_ARPHRD;
	case OPT_P_TYPE:	return IPT_INV_PROTO;
	default:		return ip46t_option_invert(option);
	}
}

struct nft_family_ops nft_family_ops_arp = {
	.add			= nft_arp_add,
	.is_same		= nft_arp_is_same,
	.print_payload		= NULL,
	.print_header		= nft_arp_print_header,
	.print_rule		= nft_arp_print_rule,
	.save_rule		= nft_arp_save_rule,
	.save_chain		= nft_arp_save_chain,
	.rule_parse		= &nft_ruleparse_ops_arp,
	.cmd_parse		= {
		.post_parse	= nft_arp_post_parse,
		.option_name	= nft_arp_option_name,
		.option_invert	= nft_arp_option_invert,
		.command_default = command_default,
		.print_help	= xtables_printhelp,
	},
	.rule_to_cs		= nft_rule_to_iptables_command_state,
	.init_cs		= nft_arp_init_cs,
	.clear_cs		= xtables_clear_iptables_command_state,
	.xlate			= nft_arp_xlate,
	.add_entry		= nft_arp_add_entry,
	.delete_entry		= nft_arp_delete_entry,
	.check_entry		= nft_arp_check_entry,
	.replace_entry		= nft_arp_replace_entry,
};
