/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "libnetlink.h"
#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

#define BAREUDP_ATTRSET(attrs, type) (((attrs) & (1L << (type))) != 0)

static void print_explain(FILE *f)
{
	fprintf(f,
		"Usage: ... bareudp dstport PORT\n"
		"		ethertype PROTO\n"
		"		[ srcportmin PORT ]\n"
		"		[ [no]multiproto ]\n"
		"\n"
		"Where:	PORT  := UDP_PORT\n"
		"	PROTO := ETHERTYPE\n"
		"\n"
		"Note: ETHERTYPE can be given as number or as protocol name (\"ipv4\", \"ipv6\",\n"
		"      \"mpls_uc\", etc.).\n"
	);
}

static void explain(void)
{
	print_explain(stderr);
}

static void check_duparg(__u64 *attrs, int type, const char *key,
			 const char *argv)
{
	if (!BAREUDP_ATTRSET(*attrs, type)) {
		*attrs |= (1L << type);
		return;
	}
	duparg2(key, argv);
}

static int bareudp_parse_opt(struct link_util *lu, int argc, char **argv,
			     struct nlmsghdr *n)
{
	bool multiproto = false;
	__u16 srcportmin = 0;
	__be16 ethertype = 0;
	__be16 dstport = 0;
	__u64 attrs = 0;

	while (argc > 0) {
		if (matches(*argv, "dstport") == 0) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_BAREUDP_PORT, "dstport",
				     *argv);
			if (get_be16(&dstport, *argv, 0))
				invarg("dstport", *argv);
		} else if (matches(*argv, "ethertype") == 0)  {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_BAREUDP_ETHERTYPE,
				     "ethertype", *argv);
			if (ll_proto_a2n(&ethertype, *argv))
				invarg("ethertype", *argv);
		} else if (matches(*argv, "srcportmin") == 0) {
			NEXT_ARG();
			check_duparg(&attrs, IFLA_BAREUDP_SRCPORT_MIN,
				     "srcportmin", *argv);
			if (get_u16(&srcportmin, *argv, 0))
				invarg("srcportmin", *argv);
		} else if (matches(*argv, "multiproto") == 0) {
			check_duparg(&attrs, IFLA_BAREUDP_MULTIPROTO_MODE,
				     *argv, *argv);
			multiproto = true;
		} else if (matches(*argv, "nomultiproto") == 0) {
			check_duparg(&attrs, IFLA_BAREUDP_MULTIPROTO_MODE,
				     *argv, *argv);
			multiproto = false;
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "bareudp: unknown command \"%s\"?\n",
				*argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	if (!BAREUDP_ATTRSET(attrs, IFLA_BAREUDP_PORT))
		missarg("dstport");
	if (!BAREUDP_ATTRSET(attrs, IFLA_BAREUDP_ETHERTYPE))
		missarg("ethertype");

	addattr16(n, 1024, IFLA_BAREUDP_PORT, dstport);
	addattr16(n, 1024, IFLA_BAREUDP_ETHERTYPE, ethertype);
	if (BAREUDP_ATTRSET(attrs, IFLA_BAREUDP_SRCPORT_MIN))
		addattr16(n, 1024, IFLA_BAREUDP_SRCPORT_MIN, srcportmin);
	if (multiproto)
		addattr(n, 1024, IFLA_BAREUDP_MULTIPROTO_MODE);

	return 0;
}

static void bareudp_print_opt(struct link_util *lu, FILE *f,
			      struct rtattr *tb[])
{
	if (!tb)
		return;

	if (tb[IFLA_BAREUDP_PORT])
		print_uint(PRINT_ANY, "dstport", "dstport %u ",
			   rta_getattr_be16(tb[IFLA_BAREUDP_PORT]));

	if (tb[IFLA_BAREUDP_ETHERTYPE]) {
		struct rtattr *attr = tb[IFLA_BAREUDP_ETHERTYPE];
		SPRINT_BUF(ethertype);

		print_string(PRINT_ANY, "ethertype", "ethertype %s ",
			     ll_proto_n2a(rta_getattr_u16(attr),
					  ethertype, sizeof(ethertype)));
	}

	if (tb[IFLA_BAREUDP_SRCPORT_MIN])
		print_uint(PRINT_ANY, "srcportmin", "srcportmin %u ",
			   rta_getattr_u16(tb[IFLA_BAREUDP_SRCPORT_MIN]));

	if (tb[IFLA_BAREUDP_MULTIPROTO_MODE])
		print_bool(PRINT_ANY, "multiproto", "multiproto ", true);
	else
		print_bool(PRINT_ANY, "multiproto", "nomultiproto ", false);
}

static void bareudp_print_help(struct link_util *lu, int argc, char **argv,
			       FILE *f)
{
	print_explain(f);
}

struct link_util bareudp_link_util = {
	.id		= "bareudp",
	.maxattr	= IFLA_BAREUDP_MAX,
	.parse_opt	= bareudp_parse_opt,
	.print_opt	= bareudp_print_opt,
	.print_help	= bareudp_print_help,
};
