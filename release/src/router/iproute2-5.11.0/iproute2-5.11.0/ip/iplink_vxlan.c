/*
 * iplink_vxlan.c	VXLAN device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Stephen Hemminger <shemminger@vyatta.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/if_link.h>
#include <arpa/inet.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

#define VXLAN_ATTRSET(attrs, type) (((attrs) & (1L << (type))) != 0)

static void print_explain(FILE *f)
{
	fprintf(f,
		"Usage: ... vxlan id VNI\n"
		"		[ { group | remote } IP_ADDRESS ]\n"
		"		[ local ADDR ]\n"
		"		[ ttl TTL ]\n"
		"		[ tos TOS ]\n"
		"		[ df DF ]\n"
		"		[ flowlabel LABEL ]\n"
		"		[ dev PHYS_DEV ]\n"
		"		[ dstport PORT ]\n"
		"		[ srcport MIN MAX ]\n"
		"		[ [no]learning ]\n"
		"		[ [no]proxy ]\n"
		"		[ [no]rsc ]\n"
		"		[ [no]l2miss ]\n"
		"		[ [no]l3miss ]\n"
		"		[ ageing SECONDS ]\n"
		"		[ maxaddress NUMBER ]\n"
		"		[ [no]udpcsum ]\n"
		"		[ [no]udp6zerocsumtx ]\n"
		"		[ [no]udp6zerocsumrx ]\n"
		"		[ [no]remcsumtx ] [ [no]remcsumrx ]\n"
		"		[ [no]external ] [ gbp ] [ gpe ]\n"
		"\n"
		"Where:	VNI	:= 0-16777215\n"
		"	ADDR	:= { IP_ADDRESS | any }\n"
		"	TOS	:= { NUMBER | inherit }\n"
		"	TTL	:= { 1..255 | auto | inherit }\n"
		"	DF	:= { unset | set | inherit }\n"
		"	LABEL := 0-1048575\n"
	);
}

static void explain(void)
{
	print_explain(stderr);
}

static void check_duparg(__u64 *attrs, int type, const char *key,
			 const char *argv)
{
	if (!VXLAN_ATTRSET(*attrs, type)) {
		*attrs |= (1L << type);
		return;
	}
	duparg2(key, argv);
}

static int vxlan_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{
	inet_prefix saddr, daddr;
	__u32 vni = 0;
	__u8 learning = 1;
	__u16 dstport = 0;
	__u8 metadata = 0;
	__u64 attrs = 0;
	bool set_op = (n->nlmsg_type == RTM_NEWLINK &&
		       !(n->nlmsg_flags & NLM_F_CREATE));
	bool selected_family = false;

	saddr.family = daddr.family = AF_UNSPEC;

	inet_prefix_reset(&saddr);
	inet_prefix_reset(&daddr);

	while (argc > 0) {
		if (!matches(*argv, "id") ||
		    !matches(*argv, "vni")) {
			/* We will add ID attribute outside of the loop since we
			 * need to consider metadata information as well.
			 */
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_ID, "id", *argv);
			if (get_u32(&vni, *argv, 0) ||
			    vni >= 1u << 24)
				invarg("invalid id", *argv);
		} else if (!matches(*argv, "group")) {
			if (is_addrtype_inet_not_multi(&daddr)) {
				fprintf(stderr, "vxlan: both group and remote");
				fprintf(stderr, " cannot be specified\n");
				return -1;
			}
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_GROUP, "group", *argv);
			get_addr(&daddr, *argv, saddr.family);
			if (!is_addrtype_inet_multi(&daddr))
				invarg("invalid group address", *argv);
		} else if (!matches(*argv, "remote")) {
			if (is_addrtype_inet_multi(&daddr)) {
				fprintf(stderr, "vxlan: both group and remote");
				fprintf(stderr, " cannot be specified\n");
				return -1;
			}
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_GROUP, "remote", *argv);
			get_addr(&daddr, *argv, saddr.family);
			if (!is_addrtype_inet_not_multi(&daddr))
				invarg("invalid remote address", *argv);
		} else if (!matches(*argv, "local")) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LOCAL, "local", *argv);
			get_addr(&saddr, *argv, daddr.family);
			if (!is_addrtype_inet_not_multi(&saddr))
				invarg("invalid local address", *argv);
		} else if (!matches(*argv, "dev")) {
			unsigned int link;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LINK, "dev", *argv);
			link = ll_name_to_index(*argv);
			if (!link)
				exit(nodev(*argv));
			addattr32(n, 1024, IFLA_VXLAN_LINK, link);
		} else if (!matches(*argv, "ttl") ||
			   !matches(*argv, "hoplimit")) {
			unsigned int uval;
			__u8 ttl = 0;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_TTL, "ttl", *argv);
			if (strcmp(*argv, "inherit") == 0) {
				addattr(n, 1024, IFLA_VXLAN_TTL_INHERIT);
			} else if (strcmp(*argv, "auto") == 0) {
				addattr8(n, 1024, IFLA_VXLAN_TTL, ttl);
			} else {
				if (get_unsigned(&uval, *argv, 0))
					invarg("invalid TTL", *argv);
				if (uval > 255)
					invarg("TTL must be <= 255", *argv);
				ttl = uval;
				addattr8(n, 1024, IFLA_VXLAN_TTL, ttl);
			}
		} else if (!matches(*argv, "tos") ||
			   !matches(*argv, "dsfield")) {
			__u32 uval;
			__u8 tos;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_TOS, "tos", *argv);
			if (strcmp(*argv, "inherit") != 0) {
				if (rtnl_dsfield_a2n(&uval, *argv))
					invarg("bad TOS value", *argv);
				tos = uval;
			} else
				tos = 1;
			addattr8(n, 1024, IFLA_VXLAN_TOS, tos);
		} else if (!matches(*argv, "df")) {
			enum ifla_vxlan_df df;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_DF, "df", *argv);
			if (strcmp(*argv, "unset") == 0)
				df = VXLAN_DF_UNSET;
			else if (strcmp(*argv, "set") == 0)
				df = VXLAN_DF_SET;
			else if (strcmp(*argv, "inherit") == 0)
				df = VXLAN_DF_INHERIT;
			else
				invarg("DF must be 'unset', 'set' or 'inherit'",
				       *argv);

			addattr8(n, 1024, IFLA_VXLAN_DF, df);
		} else if (!matches(*argv, "label") ||
			   !matches(*argv, "flowlabel")) {
			__u32 uval;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LABEL, "flowlabel",
				     *argv);
			if (get_u32(&uval, *argv, 0) ||
			    (uval & ~LABEL_MAX_MASK))
				invarg("invalid flowlabel", *argv);
			addattr32(n, 1024, IFLA_VXLAN_LABEL, htonl(uval));
		} else if (!matches(*argv, "ageing")) {
			__u32 age;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_AGEING, "ageing",
				     *argv);
			if (strcmp(*argv, "none") == 0)
				age = 0;
			else if (get_u32(&age, *argv, 0))
				invarg("ageing timer", *argv);
			addattr32(n, 1024, IFLA_VXLAN_AGEING, age);
		} else if (!matches(*argv, "maxaddress")) {
			__u32 maxaddr;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_LIMIT,
				     "maxaddress", *argv);
			if (strcmp(*argv, "unlimited") == 0)
				maxaddr = 0;
			else if (get_u32(&maxaddr, *argv, 0))
				invarg("max addresses", *argv);
			addattr32(n, 1024, IFLA_VXLAN_LIMIT, maxaddr);
		} else if (!matches(*argv, "port") ||
			   !matches(*argv, "srcport")) {
			struct ifla_vxlan_port_range range = { 0, 0 };

			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_PORT_RANGE, "srcport",
				     *argv);
			if (get_be16(&range.low, *argv, 0))
				invarg("min port", *argv);
			NEXT_ARG();
			if (get_be16(&range.high, *argv, 0))
				invarg("max port", *argv);
			if (range.low || range.high) {
				addattr_l(n, 1024, IFLA_VXLAN_PORT_RANGE,
					  &range, sizeof(range));
			}
		} else if (!matches(*argv, "dstport")) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_VXLAN_PORT, "dstport", *argv);
			if (get_u16(&dstport, *argv, 0))
				invarg("dst port", *argv);
		} else if (!matches(*argv, "nolearning")) {
			check_duparg(&attrs, IFLA_VXLAN_LEARNING, *argv, *argv);
			learning = 0;
		} else if (!matches(*argv, "learning")) {
			check_duparg(&attrs, IFLA_VXLAN_LEARNING, *argv, *argv);
			learning = 1;
		} else if (!matches(*argv, "noproxy")) {
			check_duparg(&attrs, IFLA_VXLAN_PROXY, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_PROXY, 0);
		} else if (!matches(*argv, "proxy")) {
			check_duparg(&attrs, IFLA_VXLAN_PROXY, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_PROXY, 1);
		} else if (!matches(*argv, "norsc")) {
			check_duparg(&attrs, IFLA_VXLAN_RSC, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_RSC, 0);
		} else if (!matches(*argv, "rsc")) {
			check_duparg(&attrs, IFLA_VXLAN_RSC, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_RSC, 1);
		} else if (!matches(*argv, "nol2miss")) {
			check_duparg(&attrs, IFLA_VXLAN_L2MISS, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_L2MISS, 0);
		} else if (!matches(*argv, "l2miss")) {
			check_duparg(&attrs, IFLA_VXLAN_L2MISS, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_L2MISS, 1);
		} else if (!matches(*argv, "nol3miss")) {
			check_duparg(&attrs, IFLA_VXLAN_L3MISS, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_L3MISS, 0);
		} else if (!matches(*argv, "l3miss")) {
			check_duparg(&attrs, IFLA_VXLAN_L3MISS, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_L3MISS, 1);
		} else if (!matches(*argv, "udpcsum")) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_CSUM, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_UDP_CSUM, 1);
		} else if (!matches(*argv, "noudpcsum")) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_CSUM, *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_UDP_CSUM, 0);
		} else if (!matches(*argv, "udp6zerocsumtx")) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_TX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_UDP_ZERO_CSUM6_TX, 1);
		} else if (!matches(*argv, "noudp6zerocsumtx")) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_TX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_UDP_ZERO_CSUM6_TX, 0);
		} else if (!matches(*argv, "udp6zerocsumrx")) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_RX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_UDP_ZERO_CSUM6_RX, 1);
		} else if (!matches(*argv, "noudp6zerocsumrx")) {
			check_duparg(&attrs, IFLA_VXLAN_UDP_ZERO_CSUM6_RX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_UDP_ZERO_CSUM6_RX, 0);
		} else if (!matches(*argv, "remcsumtx")) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_TX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_REMCSUM_TX, 1);
		} else if (!matches(*argv, "noremcsumtx")) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_TX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_REMCSUM_TX, 0);
		} else if (!matches(*argv, "remcsumrx")) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_RX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_REMCSUM_RX, 1);
		} else if (!matches(*argv, "noremcsumrx")) {
			check_duparg(&attrs, IFLA_VXLAN_REMCSUM_RX,
				     *argv, *argv);
			addattr8(n, 1024, IFLA_VXLAN_REMCSUM_RX, 0);
		} else if (!matches(*argv, "external")) {
			check_duparg(&attrs, IFLA_VXLAN_COLLECT_METADATA,
				     *argv, *argv);
			metadata = 1;
			learning = 0;
			/* we will add LEARNING attribute outside of the loop */
			addattr8(n, 1024, IFLA_VXLAN_COLLECT_METADATA,
				 metadata);
		} else if (!matches(*argv, "noexternal")) {
			check_duparg(&attrs, IFLA_VXLAN_COLLECT_METADATA,
				     *argv, *argv);
			metadata = 0;
			addattr8(n, 1024, IFLA_VXLAN_COLLECT_METADATA,
				 metadata);
		} else if (!matches(*argv, "gbp")) {
			check_duparg(&attrs, IFLA_VXLAN_GBP, *argv, *argv);
			addattr_l(n, 1024, IFLA_VXLAN_GBP, NULL, 0);
		} else if (!matches(*argv, "gpe")) {
			check_duparg(&attrs, IFLA_VXLAN_GPE, *argv, *argv);
			addattr_l(n, 1024, IFLA_VXLAN_GPE, NULL, 0);
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "vxlan: unknown command \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	if (metadata && VXLAN_ATTRSET(attrs, IFLA_VXLAN_ID)) {
		fprintf(stderr, "vxlan: both 'external' and vni cannot be specified\n");
		return -1;
	}

	if (!metadata && !VXLAN_ATTRSET(attrs, IFLA_VXLAN_ID) && !set_op) {
		fprintf(stderr, "vxlan: missing virtual network identifier\n");
		return -1;
	}

	if (is_addrtype_inet_multi(&daddr) &&
	    !VXLAN_ATTRSET(attrs, IFLA_VXLAN_LINK)) {
		fprintf(stderr, "vxlan: 'group' requires 'dev' to be specified\n");
		return -1;
	}

	if (!VXLAN_ATTRSET(attrs, IFLA_VXLAN_PORT) &&
	    VXLAN_ATTRSET(attrs, IFLA_VXLAN_GPE)) {
		dstport = 4790;
	} else if (!VXLAN_ATTRSET(attrs, IFLA_VXLAN_PORT) && !set_op) {
		fprintf(stderr, "vxlan: destination port not specified\n"
			"Will use Linux kernel default (non-standard value)\n");
		fprintf(stderr,
			"Use 'dstport 4789' to get the IANA assigned value\n"
			"Use 'dstport 0' to get default and quiet this message\n");
	}

	if (VXLAN_ATTRSET(attrs, IFLA_VXLAN_ID))
		addattr32(n, 1024, IFLA_VXLAN_ID, vni);

	if (is_addrtype_inet(&saddr)) {
		int type = (saddr.family == AF_INET) ? IFLA_VXLAN_LOCAL
						     : IFLA_VXLAN_LOCAL6;
		addattr_l(n, 1024, type, saddr.data, saddr.bytelen);
		selected_family = true;
	}

	if (is_addrtype_inet(&daddr)) {
		int type = (daddr.family == AF_INET) ? IFLA_VXLAN_GROUP
						     : IFLA_VXLAN_GROUP6;
		addattr_l(n, 1024, type, daddr.data, daddr.bytelen);
		selected_family = true;
	}

	if (!selected_family) {
		if (preferred_family == AF_INET) {
			get_addr(&daddr, "default", AF_INET);
			addattr_l(n, 1024, IFLA_VXLAN_GROUP,
				  daddr.data, daddr.bytelen);
		} else if (preferred_family == AF_INET6) {
			get_addr(&daddr, "default", AF_INET6);
			addattr_l(n, 1024, IFLA_VXLAN_GROUP6,
				  daddr.data, daddr.bytelen);
		}
	}

	if (!set_op || VXLAN_ATTRSET(attrs, IFLA_VXLAN_LEARNING))
		addattr8(n, 1024, IFLA_VXLAN_LEARNING, learning);

	if (dstport)
		addattr16(n, 1024, IFLA_VXLAN_PORT, htons(dstport));

	return 0;
}

static void vxlan_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	__u32 vni;
	__u8 ttl = 0;
	__u8 tos = 0;
	__u32 maxaddr;

	if (!tb)
		return;

	if (tb[IFLA_VXLAN_COLLECT_METADATA] &&
	    rta_getattr_u8(tb[IFLA_VXLAN_COLLECT_METADATA])) {
		print_bool(PRINT_ANY, "external", "external ", true);
		return;
	}

	if (!tb[IFLA_VXLAN_ID] ||
	    RTA_PAYLOAD(tb[IFLA_VXLAN_ID]) < sizeof(__u32))
		return;

	vni = rta_getattr_u32(tb[IFLA_VXLAN_ID]);
	print_uint(PRINT_ANY, "id", "id %u ", vni);

	if (tb[IFLA_VXLAN_GROUP]) {
		__be32 addr = rta_getattr_u32(tb[IFLA_VXLAN_GROUP]);

		if (addr) {
			if (IN_MULTICAST(ntohl(addr)))
				print_string(PRINT_ANY,
					     "group",
					     "group %s ",
					     format_host(AF_INET, 4, &addr));
			else
				print_string(PRINT_ANY,
					     "remote",
					     "remote %s ",
					     format_host(AF_INET, 4, &addr));
		}
	} else if (tb[IFLA_VXLAN_GROUP6]) {
		struct in6_addr addr;

		memcpy(&addr, RTA_DATA(tb[IFLA_VXLAN_GROUP6]), sizeof(struct in6_addr));
		if (!IN6_IS_ADDR_UNSPECIFIED(&addr)) {
			if (IN6_IS_ADDR_MULTICAST(&addr))
				print_string(PRINT_ANY,
					     "group6",
					     "group %s ",
					     format_host(AF_INET6,
							 sizeof(struct in6_addr),
							 &addr));
			else
				print_string(PRINT_ANY,
					     "remote6",
					     "remote %s ",
					     format_host(AF_INET6,
							 sizeof(struct in6_addr),
							 &addr));
		}
	}

	if (tb[IFLA_VXLAN_LOCAL]) {
		__be32 addr = rta_getattr_u32(tb[IFLA_VXLAN_LOCAL]);

		if (addr)
			print_string(PRINT_ANY,
				     "local",
				     "local %s ",
				     format_host(AF_INET, 4, &addr));
	} else if (tb[IFLA_VXLAN_LOCAL6]) {
		struct in6_addr addr;

		memcpy(&addr, RTA_DATA(tb[IFLA_VXLAN_LOCAL6]), sizeof(struct in6_addr));
		if (!IN6_IS_ADDR_UNSPECIFIED(&addr))
			print_string(PRINT_ANY,
				     "local6",
				     "local %s ",
				     format_host(AF_INET6,
						 sizeof(struct in6_addr),
						 &addr));
	}

	if (tb[IFLA_VXLAN_LINK]) {
		unsigned int link = rta_getattr_u32(tb[IFLA_VXLAN_LINK]);

		if (link) {
			print_string(PRINT_ANY, "link", "dev %s ",
				     ll_index_to_name(link));
		}
	}

	if (tb[IFLA_VXLAN_PORT_RANGE]) {
		const struct ifla_vxlan_port_range *r
			= RTA_DATA(tb[IFLA_VXLAN_PORT_RANGE]);
		if (is_json_context()) {
			open_json_object("port_range");
			print_uint(PRINT_JSON, "low", NULL, ntohs(r->low));
			print_uint(PRINT_JSON, "high", NULL, ntohs(r->high));
			close_json_object();
		} else {
			fprintf(f, "srcport %u %u ",
				ntohs(r->low), ntohs(r->high));
		}
	}

	if (tb[IFLA_VXLAN_PORT])
		print_uint(PRINT_ANY,
			   "port",
			   "dstport %u ",
			   rta_getattr_be16(tb[IFLA_VXLAN_PORT]));

	if (tb[IFLA_VXLAN_LEARNING]) {
		__u8 learning = rta_getattr_u8(tb[IFLA_VXLAN_LEARNING]);

		print_bool(PRINT_JSON, "learning", NULL, learning);
		if (!learning)
			print_bool(PRINT_FP, NULL, "nolearning ", true);
	}

	if (tb[IFLA_VXLAN_PROXY] && rta_getattr_u8(tb[IFLA_VXLAN_PROXY]))
		print_bool(PRINT_ANY, "proxy", "proxy ", true);

	if (tb[IFLA_VXLAN_RSC] && rta_getattr_u8(tb[IFLA_VXLAN_RSC]))
		print_bool(PRINT_ANY, "rsc", "rsc ", true);

	if (tb[IFLA_VXLAN_L2MISS] && rta_getattr_u8(tb[IFLA_VXLAN_L2MISS]))
		print_bool(PRINT_ANY, "l2miss", "l2miss ", true);

	if (tb[IFLA_VXLAN_L3MISS] && rta_getattr_u8(tb[IFLA_VXLAN_L3MISS]))
		print_bool(PRINT_ANY, "l3miss", "l3miss ", true);

	if (tb[IFLA_VXLAN_TOS])
		tos = rta_getattr_u8(tb[IFLA_VXLAN_TOS]);
	if (tos) {
		if (is_json_context() || tos != 1)
			print_0xhex(PRINT_ANY, "tos", "tos %#llx ", tos);
		else
			print_string(PRINT_FP, NULL, "tos %s ", "inherit");
	}

	if (tb[IFLA_VXLAN_TTL_INHERIT] &&
	    rta_getattr_u8(tb[IFLA_VXLAN_TTL_INHERIT])) {
		print_string(PRINT_FP, NULL, "ttl %s ", "inherit");
	} else if (tb[IFLA_VXLAN_TTL]) {
		ttl = rta_getattr_u8(tb[IFLA_VXLAN_TTL]);
		if (is_json_context() || ttl)
			print_uint(PRINT_ANY, "ttl", "ttl %u ", ttl);
		else
			print_string(PRINT_FP, NULL, "ttl %s ", "auto");
	}

	if (tb[IFLA_VXLAN_DF]) {
		enum ifla_vxlan_df df = rta_getattr_u8(tb[IFLA_VXLAN_DF]);

		if (df == VXLAN_DF_UNSET)
			print_string(PRINT_JSON, "df", "df %s ", "unset");
		else if (df == VXLAN_DF_SET)
			print_string(PRINT_ANY, "df", "df %s ", "set");
		else if (df == VXLAN_DF_INHERIT)
			print_string(PRINT_ANY, "df", "df %s ", "inherit");
	}

	if (tb[IFLA_VXLAN_LABEL]) {
		__u32 label = rta_getattr_u32(tb[IFLA_VXLAN_LABEL]);

		if (label)
			print_0xhex(PRINT_ANY, "label",
				    "flowlabel %#llx ", ntohl(label));
	}

	if (tb[IFLA_VXLAN_AGEING]) {
		__u32 age = rta_getattr_u32(tb[IFLA_VXLAN_AGEING]);

		if (age == 0)
			print_uint(PRINT_ANY, "ageing", "ageing none ", 0);
		else
			print_uint(PRINT_ANY, "ageing", "ageing %u ", age);
	}

	if (tb[IFLA_VXLAN_LIMIT] &&
	    ((maxaddr = rta_getattr_u32(tb[IFLA_VXLAN_LIMIT])) != 0))
		print_uint(PRINT_ANY, "limit", "maxaddr %u ", maxaddr);

	if (tb[IFLA_VXLAN_UDP_CSUM]) {
		__u8 udp_csum = rta_getattr_u8(tb[IFLA_VXLAN_UDP_CSUM]);

		if (is_json_context()) {
			print_bool(PRINT_ANY, "udp_csum", NULL, udp_csum);
		} else {
			if (!udp_csum)
				fputs("no", f);
			fputs("udpcsum ", f);
		}
	}

	if (tb[IFLA_VXLAN_UDP_ZERO_CSUM6_TX]) {
		__u8 csum6 = rta_getattr_u8(tb[IFLA_VXLAN_UDP_ZERO_CSUM6_TX]);

		if (is_json_context()) {
			print_bool(PRINT_ANY,
				   "udp_zero_csum6_tx", NULL, csum6);
		} else {
			if (!csum6)
				fputs("no", f);
			fputs("udp6zerocsumtx ", f);
		}
	}

	if (tb[IFLA_VXLAN_UDP_ZERO_CSUM6_RX]) {
		__u8 csum6 = rta_getattr_u8(tb[IFLA_VXLAN_UDP_ZERO_CSUM6_RX]);

		if (is_json_context()) {
			print_bool(PRINT_ANY,
				   "udp_zero_csum6_rx",
				   NULL,
				   csum6);
		} else {
			if (!csum6)
				fputs("no", f);
			fputs("udp6zerocsumrx ", f);
		}
	}

	if (tb[IFLA_VXLAN_REMCSUM_TX] &&
	    rta_getattr_u8(tb[IFLA_VXLAN_REMCSUM_TX]))
		print_bool(PRINT_ANY, "remcsum_tx", "remcsumtx ", true);

	if (tb[IFLA_VXLAN_REMCSUM_RX] &&
	    rta_getattr_u8(tb[IFLA_VXLAN_REMCSUM_RX]))
		print_bool(PRINT_ANY, "remcsum_rx", "remcsumrx ", true);

	if (tb[IFLA_VXLAN_GBP])
		print_bool(PRINT_ANY, "gbp", "gbp ", true);
	if (tb[IFLA_VXLAN_GPE])
		print_bool(PRINT_ANY, "gpe", "gpe ", true);
}

static void vxlan_print_help(struct link_util *lu, int argc, char **argv,
			     FILE *f)
{
	print_explain(f);
}

struct link_util vxlan_link_util = {
	.id		= "vxlan",
	.maxattr	= IFLA_VXLAN_MAX,
	.parse_opt	= vxlan_parse_opt,
	.print_opt	= vxlan_print_opt,
	.print_help	= vxlan_print_help,
};
