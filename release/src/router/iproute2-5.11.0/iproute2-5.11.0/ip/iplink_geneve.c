/*
 * iplink_geneve.c	GENEVE device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     John W. Linville <linville@tuxdriver.com>
 */

#include <stdio.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

#define GENEVE_ATTRSET(attrs, type) (((attrs) & (1L << (type))) != 0)

static void print_explain(FILE *f)
{
	fprintf(f,
		"Usage: ... geneve id VNI\n"
		"		remote ADDR\n"
		"		[ ttl TTL ]\n"
		"		[ tos TOS ]\n"
		"		[ df DF ]\n"
		"		[ flowlabel LABEL ]\n"
		"		[ dstport PORT ]\n"
		"		[ [no]external ]\n"
		"		[ [no]udpcsum ]\n"
		"		[ [no]udp6zerocsumtx ]\n"
		"		[ [no]udp6zerocsumrx ]\n"
		"\n"
		"Where:	VNI   := 0-16777215\n"
		"	ADDR  := IP_ADDRESS\n"
		"	TOS   := { NUMBER | inherit }\n"
		"	TTL   := { 1..255 | auto | inherit }\n"
		"	DF    := { unset | set | inherit }\n"
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
	if (!GENEVE_ATTRSET(*attrs, type)) {
		*attrs |= (1L << type);
		return;
	}
	duparg2(key, argv);
}

static int geneve_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{
	inet_prefix daddr;
	__u32 vni = 0;
	__u32 label = 0;
	__u8 ttl = 0;
	__u8 tos = 0;
	__u16 dstport = 0;
	bool metadata = 0;
	__u8 udpcsum = 0;
	__u8 udp6zerocsumtx = 0;
	__u8 udp6zerocsumrx = 0;
	__u64 attrs = 0;
	bool set_op = (n->nlmsg_type == RTM_NEWLINK &&
		       !(n->nlmsg_flags & NLM_F_CREATE));

	inet_prefix_reset(&daddr);

	while (argc > 0) {
		if (!matches(*argv, "id") ||
		    !matches(*argv, "vni")) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_GENEVE_ID, "id", *argv);
			if (get_u32(&vni, *argv, 0) ||
			    vni >= 1u << 24)
				invarg("invalid id", *argv);
		} else if (!matches(*argv, "remote")) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_GENEVE_REMOTE, "remote",
				     *argv);
			get_addr(&daddr, *argv, AF_UNSPEC);
			if (!is_addrtype_inet_not_multi(&daddr))
				invarg("invalid remote address", *argv);
		} else if (!matches(*argv, "ttl") ||
			   !matches(*argv, "hoplimit")) {
			unsigned int uval;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_GENEVE_TTL, "ttl", *argv);
			if (strcmp(*argv, "inherit") == 0) {
				addattr8(n, 1024, IFLA_GENEVE_TTL_INHERIT, 1);
			} else if (strcmp(*argv, "auto") != 0) {
				if (get_unsigned(&uval, *argv, 0))
					invarg("invalid TTL", *argv);
				if (uval > 255)
					invarg("TTL must be <= 255", *argv);
				ttl = uval;
			}
		} else if (!matches(*argv, "tos") ||
			   !matches(*argv, "dsfield")) {
			__u32 uval;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_GENEVE_TOS, "tos", *argv);
			if (strcmp(*argv, "inherit") != 0) {
				if (rtnl_dsfield_a2n(&uval, *argv))
					invarg("bad TOS value", *argv);
				tos = uval;
			} else
				tos = 1;
		} else if (!matches(*argv, "df")) {
			enum ifla_geneve_df df;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_GENEVE_DF, "df", *argv);
			if (strcmp(*argv, "unset") == 0)
				df = GENEVE_DF_UNSET;
			else if (strcmp(*argv, "set") == 0)
				df = GENEVE_DF_SET;
			else if (strcmp(*argv, "inherit") == 0)
				df = GENEVE_DF_INHERIT;
			else
				invarg("DF must be 'unset', 'set' or 'inherit'",
				       *argv);

			addattr8(n, 1024, IFLA_GENEVE_DF, df);
		} else if (!matches(*argv, "label") ||
			   !matches(*argv, "flowlabel")) {
			__u32 uval;

			NEXT_ARG();
			check_duparg(&attrs, IFLA_GENEVE_LABEL, "flowlabel",
				     *argv);
			if (get_u32(&uval, *argv, 0) ||
			    (uval & ~LABEL_MAX_MASK))
				invarg("invalid flowlabel", *argv);
			label = htonl(uval);
		} else if (!matches(*argv, "dstport")) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_GENEVE_PORT, "dstport",
				     *argv);
			if (get_u16(&dstport, *argv, 0))
				invarg("dstport", *argv);
		} else if (!matches(*argv, "external")) {
			check_duparg(&attrs, IFLA_GENEVE_COLLECT_METADATA,
				     *argv, *argv);
			metadata = true;
		} else if (!matches(*argv, "noexternal")) {
			check_duparg(&attrs, IFLA_GENEVE_COLLECT_METADATA,
				     *argv, *argv);
			metadata = false;
		} else if (!matches(*argv, "udpcsum")) {
			check_duparg(&attrs, IFLA_GENEVE_UDP_CSUM, *argv,
				     *argv);
			udpcsum = 1;
		} else if (!matches(*argv, "noudpcsum")) {
			check_duparg(&attrs, IFLA_GENEVE_UDP_CSUM, *argv,
				     *argv);
			udpcsum = 0;
		} else if (!matches(*argv, "udp6zerocsumtx")) {
			check_duparg(&attrs, IFLA_GENEVE_UDP_ZERO_CSUM6_TX,
				     *argv, *argv);
			udp6zerocsumtx = 1;
		} else if (!matches(*argv, "noudp6zerocsumtx")) {
			check_duparg(&attrs, IFLA_GENEVE_UDP_ZERO_CSUM6_TX,
				     *argv, *argv);
			udp6zerocsumtx = 0;
		} else if (!matches(*argv, "udp6zerocsumrx")) {
			check_duparg(&attrs, IFLA_GENEVE_UDP_ZERO_CSUM6_RX,
				     *argv, *argv);
			udp6zerocsumrx = 1;
		} else if (!matches(*argv, "noudp6zerocsumrx")) {
			check_duparg(&attrs, IFLA_GENEVE_UDP_ZERO_CSUM6_RX,
				     *argv, *argv);
			udp6zerocsumrx = 0;
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "geneve: unknown command \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	if (metadata && GENEVE_ATTRSET(attrs, IFLA_GENEVE_ID)) {
		fprintf(stderr, "geneve: both 'external' and vni cannot be specified\n");
		return -1;
	}

	if (!metadata) {
		/* parameter checking make sense only for full geneve tunnels */
		if (!GENEVE_ATTRSET(attrs, IFLA_GENEVE_ID)) {
			fprintf(stderr, "geneve: missing virtual network identifier\n");
			return -1;
		}

		/* If we are modifying the geneve device, then we only need the
		 * ID (VNI) to identify the geneve device, and we do not need
		 * the remote IP.
		 */
		if (!set_op && !is_addrtype_inet(&daddr)) {
			fprintf(stderr, "geneve: remote link partner not specified\n");
			return -1;
		}
	}

	addattr32(n, 1024, IFLA_GENEVE_ID, vni);
	if (is_addrtype_inet(&daddr)) {
		int type = (daddr.family == AF_INET) ? IFLA_GENEVE_REMOTE :
						       IFLA_GENEVE_REMOTE6;
		addattr_l(n, 1024, type, daddr.data, daddr.bytelen);
	}
	if (!set_op || GENEVE_ATTRSET(attrs, IFLA_GENEVE_LABEL))
		addattr32(n, 1024, IFLA_GENEVE_LABEL, label);
	if (!set_op || GENEVE_ATTRSET(attrs, IFLA_GENEVE_TTL))
		addattr8(n, 1024, IFLA_GENEVE_TTL, ttl);
	if (!set_op || GENEVE_ATTRSET(attrs, IFLA_GENEVE_TOS))
		addattr8(n, 1024, IFLA_GENEVE_TOS, tos);
	if (dstport)
		addattr16(n, 1024, IFLA_GENEVE_PORT, htons(dstport));
	if (metadata)
		addattr(n, 1024, IFLA_GENEVE_COLLECT_METADATA);
	if (GENEVE_ATTRSET(attrs, IFLA_GENEVE_UDP_CSUM))
		addattr8(n, 1024, IFLA_GENEVE_UDP_CSUM, udpcsum);
	if (GENEVE_ATTRSET(attrs, IFLA_GENEVE_UDP_ZERO_CSUM6_TX))
		addattr8(n, 1024, IFLA_GENEVE_UDP_ZERO_CSUM6_TX, udp6zerocsumtx);
	if (GENEVE_ATTRSET(attrs, IFLA_GENEVE_UDP_ZERO_CSUM6_RX))
		addattr8(n, 1024, IFLA_GENEVE_UDP_ZERO_CSUM6_RX, udp6zerocsumrx);

	return 0;
}

static void geneve_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	__u32 vni;
	__u8 ttl = 0;
	__u8 tos = 0;

	if (!tb)
		return;

	if (tb[IFLA_GENEVE_COLLECT_METADATA]) {
		print_bool(PRINT_ANY, "external", "external ", true);
		return;
	}

	if (!tb[IFLA_GENEVE_ID] ||
	    RTA_PAYLOAD(tb[IFLA_GENEVE_ID]) < sizeof(__u32))
		return;

	vni = rta_getattr_u32(tb[IFLA_GENEVE_ID]);
	print_uint(PRINT_ANY, "id", "id %u ", vni);

	if (tb[IFLA_GENEVE_REMOTE]) {
		__be32 addr = rta_getattr_u32(tb[IFLA_GENEVE_REMOTE]);

		if (addr)
			print_string(PRINT_ANY,
				     "remote",
				     "remote %s ",
				     format_host(AF_INET, 4, &addr));
	} else if (tb[IFLA_GENEVE_REMOTE6]) {
		struct in6_addr addr;

		memcpy(&addr, RTA_DATA(tb[IFLA_GENEVE_REMOTE6]), sizeof(struct in6_addr));
		if (!IN6_IS_ADDR_UNSPECIFIED(&addr)) {
			if (!IN6_IS_ADDR_MULTICAST(&addr))
				print_string(PRINT_ANY,
					     "remote6",
					     "remote %s ",
					     format_host(AF_INET6,
							 sizeof(struct in6_addr),
							 &addr));
		}
	}

	if (tb[IFLA_GENEVE_TTL_INHERIT] &&
	    rta_getattr_u8(tb[IFLA_GENEVE_TTL_INHERIT])) {
		print_string(PRINT_FP, NULL, "ttl %s ", "inherit");
	} else if (tb[IFLA_GENEVE_TTL]) {
		ttl = rta_getattr_u8(tb[IFLA_GENEVE_TTL]);
		if (is_json_context() || ttl)
			print_uint(PRINT_ANY, "ttl", "ttl %u ", ttl);
		else
			print_string(PRINT_FP, NULL, "ttl %s ", "auto");
	}

	if (tb[IFLA_GENEVE_TOS])
		tos = rta_getattr_u8(tb[IFLA_GENEVE_TOS]);
	if (tos) {
		if (is_json_context() || tos != 1)
			print_0xhex(PRINT_ANY, "tos", "tos %#llx ", tos);
		else
			print_string(PRINT_FP, NULL, "tos %s ", "inherit");
	}

	if (tb[IFLA_GENEVE_DF]) {
		enum ifla_geneve_df df = rta_getattr_u8(tb[IFLA_GENEVE_DF]);

		if (df == GENEVE_DF_UNSET)
			print_string(PRINT_JSON, "df", "df %s ", "unset");
		else if (df == GENEVE_DF_SET)
			print_string(PRINT_ANY, "df", "df %s ", "set");
		else if (df == GENEVE_DF_INHERIT)
			print_string(PRINT_ANY, "df", "df %s ", "inherit");
	}

	if (tb[IFLA_GENEVE_LABEL]) {
		__u32 label = rta_getattr_u32(tb[IFLA_GENEVE_LABEL]);

		if (label)
			print_0xhex(PRINT_ANY,
				    "label", "flowlabel %#llx ",
				    ntohl(label));
	}

	if (tb[IFLA_GENEVE_PORT])
		print_uint(PRINT_ANY,
			   "port",
			   "dstport %u ",
			   rta_getattr_be16(tb[IFLA_GENEVE_PORT]));

	if (tb[IFLA_GENEVE_UDP_CSUM]) {
		if (is_json_context()) {
			print_bool(PRINT_JSON,
				   "udp_csum",
				   NULL,
				   rta_getattr_u8(tb[IFLA_GENEVE_UDP_CSUM]));
		} else {
			if (!rta_getattr_u8(tb[IFLA_GENEVE_UDP_CSUM]))
				fputs("no", f);
			fputs("udpcsum ", f);
		}
	}

	if (tb[IFLA_GENEVE_UDP_ZERO_CSUM6_TX]) {
		if (is_json_context()) {
			print_bool(PRINT_JSON,
				   "udp_zero_csum6_tx",
				   NULL,
				   rta_getattr_u8(tb[IFLA_GENEVE_UDP_ZERO_CSUM6_TX]));
		} else {
			if (!rta_getattr_u8(tb[IFLA_GENEVE_UDP_ZERO_CSUM6_TX]))
				fputs("no", f);
			fputs("udp6zerocsumtx ", f);
		}
	}

	if (tb[IFLA_GENEVE_UDP_ZERO_CSUM6_RX]) {
		if (is_json_context()) {
			print_bool(PRINT_JSON,
				   "udp_zero_csum6_rx",
				   NULL,
				   rta_getattr_u8(tb[IFLA_GENEVE_UDP_ZERO_CSUM6_RX]));
		} else {
			if (!rta_getattr_u8(tb[IFLA_GENEVE_UDP_ZERO_CSUM6_RX]))
				fputs("no", f);
			fputs("udp6zerocsumrx ", f);
		}
	}
}

static void geneve_print_help(struct link_util *lu, int argc, char **argv,
	FILE *f)
{
	print_explain(f);
}

struct link_util geneve_link_util = {
	.id		= "geneve",
	.maxattr	= IFLA_GENEVE_MAX,
	.parse_opt	= geneve_parse_opt,
	.print_opt	= geneve_print_opt,
	.print_help	= geneve_print_help,
};
