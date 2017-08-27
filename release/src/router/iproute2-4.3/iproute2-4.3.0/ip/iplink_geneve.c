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

static void print_explain(FILE *f)
{
	fprintf(f, "Usage: ... geneve id VNI remote ADDR\n");
	fprintf(f, "                 [ ttl TTL ] [ tos TOS ]\n");
	fprintf(f, "\n");
	fprintf(f, "Where: VNI  := 0-16777215\n");
	fprintf(f, "       ADDR := IP_ADDRESS\n");
	fprintf(f, "       TOS  := { NUMBER | inherit }\n");
	fprintf(f, "       TTL  := { 1..255 | inherit }\n");
}

static void explain(void)
{
	print_explain(stderr);
}

static int geneve_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{
	__u32 vni = 0;
	int vni_set = 0;
	__u32 daddr = 0;
	struct in6_addr daddr6 = IN6ADDR_ANY_INIT;
	__u8 ttl = 0;
	__u8 tos = 0;

	while (argc > 0) {
		if (!matches(*argv, "id") ||
		    !matches(*argv, "vni")) {
			NEXT_ARG();
			if (get_u32(&vni, *argv, 0) ||
			    vni >= 1u << 24)
				invarg("invalid id", *argv);
			vni_set = 1;
		} else if (!matches(*argv, "remote")) {
			NEXT_ARG();
			if (!inet_get_addr(*argv, &daddr, &daddr6)) {
				fprintf(stderr, "Invalid address \"%s\"\n", *argv);
				return -1;
			}
			if (IN_MULTICAST(ntohl(daddr)))
				invarg("invalid remote address", *argv);
		} else if (!matches(*argv, "ttl") ||
			   !matches(*argv, "hoplimit")) {
			unsigned uval;

			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0) {
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
			if (strcmp(*argv, "inherit") != 0) {
				if (rtnl_dsfield_a2n(&uval, *argv))
					invarg("bad TOS value", *argv);
				tos = uval;
			} else
				tos = 1;
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

	if (!vni_set) {
		fprintf(stderr, "geneve: missing virtual network identifier\n");
		return -1;
	}

	if (!daddr) {
		fprintf(stderr, "geneve: remove link partner not specified\n");
		return -1;
	}
	if (memcmp(&daddr6, &in6addr_any, sizeof(daddr6)) != 0) {
		fprintf(stderr, "geneve: remove link over IPv6 not supported\n");
		return -1;
	}

	addattr32(n, 1024, IFLA_GENEVE_ID, vni);
	if (daddr)
		addattr_l(n, 1024, IFLA_GENEVE_REMOTE, &daddr, 4);
	addattr8(n, 1024, IFLA_GENEVE_TTL, ttl);
	addattr8(n, 1024, IFLA_GENEVE_TOS, tos);

	return 0;
}

static void geneve_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	__u32 vni;
	char s1[1024];
	__u8 tos;

	if (!tb)
		return;

	if (!tb[IFLA_GENEVE_ID] ||
	    RTA_PAYLOAD(tb[IFLA_GENEVE_ID]) < sizeof(__u32))
		return;

	vni = rta_getattr_u32(tb[IFLA_GENEVE_ID]);
	fprintf(f, "id %u ", vni);

	if (tb[IFLA_GENEVE_REMOTE]) {
		__be32 addr = rta_getattr_u32(tb[IFLA_GENEVE_REMOTE]);
		if (addr)
			fprintf(f, "remote %s ",
				format_host(AF_INET, 4, &addr, s1, sizeof(s1)));
	}

	if (tb[IFLA_GENEVE_TTL]) {
		__u8 ttl = rta_getattr_u8(tb[IFLA_GENEVE_TTL]);
		if (ttl)
			fprintf(f, "ttl %d ", ttl);
	}

	if (tb[IFLA_GENEVE_TOS] &&
	    (tos = rta_getattr_u8(tb[IFLA_GENEVE_TOS]))) {
		if (tos == 1)
			fprintf(f, "tos inherit ");
		else
			fprintf(f, "tos %#x ", tos);
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
