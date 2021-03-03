/*
 * link_gre.c	gre driver module
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Herbert Xu <herbert@gondor.apana.org.au>
 *
 */

#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <linux/ip.h>
#include <linux/if_tunnel.h>
#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "tunnel.h"

static bool gre_is_erspan(struct link_util *lu)
{
	return !strcmp(lu->id, "erspan");
}

static void gre_print_help(struct link_util *lu, int argc, char **argv, FILE *f)
{
	bool is_erspan = gre_is_erspan(lu);

	fprintf(f,
		"Usage: ... %-9s	[ remote ADDR ]\n"
		"			[ local ADDR ]\n"
		"			[ [no][i|o]seq ]\n"
		"			[ [i|o]key KEY | no[i|o]key ]\n"
		"			[ [no][i|o]csum ]\n"
		"			[ ttl TTL ]\n"
		"			[ tos TOS ]\n"
		"			[ [no]pmtudisc ]\n"
		"			[ [no]ignore-df ]\n"
		"			[ dev PHYS_DEV ]\n"
		"			[ fwmark MARK ]\n"
		"			[ external ]\n"
		"			[ noencap ]\n"
		"			[ encap { fou | gue | none } ]\n"
		"			[ encap-sport PORT ]\n"
		"			[ encap-dport PORT ]\n"
		"			[ [no]encap-csum ]\n"
		"			[ [no]encap-csum6 ]\n"
		"			[ [no]encap-remcsum ]\n", lu->id);
	if (is_erspan)
		fprintf(f,
			"			[ erspan_ver version ]\n"
			"			[ erspan IDX ]\n"
			"			[ erspan_dir { ingress | egress } ]\n"
			"			[ erspan_hwid hwid ]\n");
	fprintf(f,
		"\n"
		"Where:	ADDR := { IP_ADDRESS | any }\n"
		"	TOS  := { NUMBER | inherit }\n"
		"	TTL  := { 1..255 | inherit }\n"
		"	KEY  := { DOTTED_QUAD | NUMBER }\n"
		"	MARK := { 0x0..0xffffffff }\n");
}

static int gre_parse_opt(struct link_util *lu, int argc, char **argv,
			 struct nlmsghdr *n)
{
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct {
		struct nlmsghdr n;
		struct ifinfomsg i;
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(*ifi)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_GETLINK,
		.i.ifi_family = preferred_family,
		.i.ifi_index = ifi->ifi_index,
	};
	struct nlmsghdr *answer;
	struct rtattr *tb[IFLA_MAX + 1];
	struct rtattr *linkinfo[IFLA_INFO_MAX+1];
	struct rtattr *greinfo[IFLA_GRE_MAX + 1];
	int len;
	__u16 iflags = 0;
	__u16 oflags = 0;
	__be32 ikey = 0;
	__be32 okey = 0;
	inet_prefix saddr, daddr;
	__u8 pmtudisc = 1;
	__u8 ignore_df = 0;
	__u8 tos = 0;
	__u8 ttl = 0;
	__u32 link = 0;
	__u16 encaptype = 0;
	__u16 encapflags = 0;
	__u16 encapsport = 0;
	__u16 encapdport = 0;
	__u8 metadata = 0;
	__u32 fwmark = 0;
	bool is_erspan = gre_is_erspan(lu);
	__u32 erspan_idx = 0;
	__u8 erspan_ver = 1;
	__u8 erspan_dir = 0;
	__u16 erspan_hwid = 0;

	inet_prefix_reset(&saddr);
	inet_prefix_reset(&daddr);

	if (!(n->nlmsg_flags & NLM_F_CREATE)) {
		const struct rtattr *rta;

		if (rtnl_talk(&rth, &req.n, &answer) < 0) {
get_failed:
			fprintf(stderr,
				"Failed to get existing tunnel info.\n");
			return -1;
		}

		len = answer->nlmsg_len;
		len -= NLMSG_LENGTH(sizeof(*ifi));
		if (len < 0)
			goto get_failed;

		parse_rtattr(tb, IFLA_MAX, IFLA_RTA(NLMSG_DATA(answer)), len);

		if (!tb[IFLA_LINKINFO])
			goto get_failed;

		parse_rtattr_nested(linkinfo, IFLA_INFO_MAX, tb[IFLA_LINKINFO]);

		if (!linkinfo[IFLA_INFO_DATA])
			goto get_failed;

		parse_rtattr_nested(greinfo, IFLA_GRE_MAX,
				    linkinfo[IFLA_INFO_DATA]);

		rta = greinfo[IFLA_GRE_LOCAL];
		if (rta && get_addr_rta(&saddr, rta, AF_INET))
			goto get_failed;

		rta = greinfo[IFLA_GRE_REMOTE];
		if (rta && get_addr_rta(&daddr, rta, AF_INET))
			goto get_failed;

		if (greinfo[IFLA_GRE_IKEY])
			ikey = rta_getattr_u32(greinfo[IFLA_GRE_IKEY]);

		if (greinfo[IFLA_GRE_OKEY])
			okey = rta_getattr_u32(greinfo[IFLA_GRE_OKEY]);

		if (greinfo[IFLA_GRE_IFLAGS])
			iflags = rta_getattr_u16(greinfo[IFLA_GRE_IFLAGS]);

		if (greinfo[IFLA_GRE_OFLAGS])
			oflags = rta_getattr_u16(greinfo[IFLA_GRE_OFLAGS]);

		if (greinfo[IFLA_GRE_PMTUDISC])
			pmtudisc = rta_getattr_u8(
				greinfo[IFLA_GRE_PMTUDISC]);

		if (greinfo[IFLA_GRE_IGNORE_DF])
			ignore_df =
				!!rta_getattr_u8(greinfo[IFLA_GRE_IGNORE_DF]);

		if (greinfo[IFLA_GRE_TOS])
			tos = rta_getattr_u8(greinfo[IFLA_GRE_TOS]);

		if (greinfo[IFLA_GRE_TTL])
			ttl = rta_getattr_u8(greinfo[IFLA_GRE_TTL]);

		if (greinfo[IFLA_GRE_LINK])
			link = rta_getattr_u32(greinfo[IFLA_GRE_LINK]);

		if (greinfo[IFLA_GRE_ENCAP_TYPE])
			encaptype = rta_getattr_u16(greinfo[IFLA_GRE_ENCAP_TYPE]);

		if (greinfo[IFLA_GRE_ENCAP_FLAGS])
			encapflags = rta_getattr_u16(greinfo[IFLA_GRE_ENCAP_FLAGS]);

		if (greinfo[IFLA_GRE_ENCAP_SPORT])
			encapsport = rta_getattr_u16(greinfo[IFLA_GRE_ENCAP_SPORT]);

		if (greinfo[IFLA_GRE_ENCAP_DPORT])
			encapdport = rta_getattr_u16(greinfo[IFLA_GRE_ENCAP_DPORT]);

		if (greinfo[IFLA_GRE_COLLECT_METADATA])
			metadata = 1;

		if (greinfo[IFLA_GRE_FWMARK])
			fwmark = rta_getattr_u32(greinfo[IFLA_GRE_FWMARK]);

		if (greinfo[IFLA_GRE_ERSPAN_INDEX])
			erspan_idx = rta_getattr_u32(greinfo[IFLA_GRE_ERSPAN_INDEX]);

		if (greinfo[IFLA_GRE_ERSPAN_VER])
			erspan_ver = rta_getattr_u8(greinfo[IFLA_GRE_ERSPAN_VER]);

		if (greinfo[IFLA_GRE_ERSPAN_DIR])
			erspan_dir = rta_getattr_u8(greinfo[IFLA_GRE_ERSPAN_DIR]);

		if (greinfo[IFLA_GRE_ERSPAN_HWID])
			erspan_hwid = rta_getattr_u16(greinfo[IFLA_GRE_ERSPAN_HWID]);

		free(answer);
	}

	while (argc > 0) {
		if (!matches(*argv, "key")) {
			NEXT_ARG();
			iflags |= GRE_KEY;
			oflags |= GRE_KEY;
			ikey = okey = tnl_parse_key("key", *argv);
		} else if (!matches(*argv, "nokey")) {
			iflags &= ~GRE_KEY;
			oflags &= ~GRE_KEY;
			ikey = okey = 0;
		} else if (!matches(*argv, "ikey")) {
			NEXT_ARG();
			iflags |= GRE_KEY;
			ikey = tnl_parse_key("ikey", *argv);
		} else if (!matches(*argv, "noikey")) {
			iflags &= ~GRE_KEY;
			ikey = 0;
		} else if (!matches(*argv, "okey")) {
			NEXT_ARG();
			oflags |= GRE_KEY;
			okey = tnl_parse_key("okey", *argv);
		} else if (!matches(*argv, "nookey")) {
			oflags &= ~GRE_KEY;
			okey = 0;
		} else if (!matches(*argv, "seq")) {
			iflags |= GRE_SEQ;
			oflags |= GRE_SEQ;
		} else if (!matches(*argv, "noseq")) {
			iflags &= ~GRE_SEQ;
			oflags &= ~GRE_SEQ;
		} else if (!matches(*argv, "iseq")) {
			iflags |= GRE_SEQ;
		} else if (!matches(*argv, "noiseq")) {
			iflags &= ~GRE_SEQ;
		} else if (!matches(*argv, "oseq")) {
			oflags |= GRE_SEQ;
		} else if (!matches(*argv, "nooseq")) {
			oflags &= ~GRE_SEQ;
		} else if (!matches(*argv, "csum")) {
			iflags |= GRE_CSUM;
			oflags |= GRE_CSUM;
		} else if (!matches(*argv, "nocsum")) {
			iflags &= ~GRE_CSUM;
			oflags &= ~GRE_CSUM;
		} else if (!matches(*argv, "icsum")) {
			iflags |= GRE_CSUM;
		} else if (!matches(*argv, "noicsum")) {
			iflags &= ~GRE_CSUM;
		} else if (!matches(*argv, "ocsum")) {
			oflags |= GRE_CSUM;
		} else if (!matches(*argv, "noocsum")) {
			oflags &= ~GRE_CSUM;
		} else if (!matches(*argv, "nopmtudisc")) {
			pmtudisc = 0;
		} else if (!matches(*argv, "pmtudisc")) {
			pmtudisc = 1;
		} else if (!matches(*argv, "remote")) {
			NEXT_ARG();
			get_addr(&daddr, *argv, AF_INET);
		} else if (!matches(*argv, "local")) {
			NEXT_ARG();
			get_addr(&saddr, *argv, AF_INET);
		} else if (!matches(*argv, "dev")) {
			NEXT_ARG();
			link = ll_name_to_index(*argv);
			if (!link)
				exit(nodev(*argv));
		} else if (!matches(*argv, "ttl") ||
			   !matches(*argv, "hoplimit") ||
			   !matches(*argv, "hlim")) {
			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0) {
				if (get_u8(&ttl, *argv, 0))
					invarg("invalid TTL\n", *argv);
			} else
				ttl = 0;
		} else if (!matches(*argv, "tos") ||
			   !matches(*argv, "tclass") ||
			   !matches(*argv, "dsfield")) {
			__u32 uval;

			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0) {
				if (rtnl_dsfield_a2n(&uval, *argv))
					invarg("bad TOS value", *argv);
				tos = uval;
			} else
				tos = 1;
		} else if (strcmp(*argv, "noencap") == 0) {
			encaptype = TUNNEL_ENCAP_NONE;
		} else if (strcmp(*argv, "encap") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "fou") == 0)
				encaptype = TUNNEL_ENCAP_FOU;
			else if (strcmp(*argv, "gue") == 0)
				encaptype = TUNNEL_ENCAP_GUE;
			else if (strcmp(*argv, "none") == 0)
				encaptype = TUNNEL_ENCAP_NONE;
			else
				invarg("Invalid encap type.", *argv);
		} else if (strcmp(*argv, "encap-sport") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "auto") == 0)
				encapsport = 0;
			else if (get_u16(&encapsport, *argv, 0))
				invarg("Invalid source port.", *argv);
		} else if (strcmp(*argv, "encap-dport") == 0) {
			NEXT_ARG();
			if (get_u16(&encapdport, *argv, 0))
				invarg("Invalid destination port.", *argv);
		} else if (strcmp(*argv, "encap-csum") == 0) {
			encapflags |= TUNNEL_ENCAP_FLAG_CSUM;
		} else if (strcmp(*argv, "noencap-csum") == 0) {
			encapflags &= ~TUNNEL_ENCAP_FLAG_CSUM;
		} else if (strcmp(*argv, "encap-udp6-csum") == 0) {
			encapflags |= TUNNEL_ENCAP_FLAG_CSUM6;
		} else if (strcmp(*argv, "noencap-udp6-csum") == 0) {
			encapflags &= ~TUNNEL_ENCAP_FLAG_CSUM6;
		} else if (strcmp(*argv, "encap-remcsum") == 0) {
			encapflags |= TUNNEL_ENCAP_FLAG_REMCSUM;
		} else if (strcmp(*argv, "noencap-remcsum") == 0) {
			encapflags &= ~TUNNEL_ENCAP_FLAG_REMCSUM;
		} else if (strcmp(*argv, "external") == 0) {
			metadata = 1;
		} else if (strcmp(*argv, "ignore-df") == 0) {
			ignore_df = 1;
		} else if (strcmp(*argv, "noignore-df") == 0) {
			/*
			 *only the lsb is significant, use 2 for presence
			 */
			ignore_df = 2;
		} else if (strcmp(*argv, "fwmark") == 0) {
			NEXT_ARG();
			if (get_u32(&fwmark, *argv, 0))
				invarg("invalid fwmark\n", *argv);
		} else if (is_erspan && strcmp(*argv, "erspan") == 0) {
			NEXT_ARG();
			if (get_u32(&erspan_idx, *argv, 0))
				invarg("invalid erspan index\n", *argv);
			if (erspan_idx & ~((1<<20) - 1) || erspan_idx == 0)
				invarg("erspan index must be > 0 and <= 20-bit\n", *argv);
		} else if (is_erspan && strcmp(*argv, "erspan_ver") == 0) {
			NEXT_ARG();
			if (get_u8(&erspan_ver, *argv, 0))
				invarg("invalid erspan version\n", *argv);
			if (erspan_ver > 2)
				invarg("erspan version must be 0/1/2\n", *argv);
		} else if (is_erspan && strcmp(*argv, "erspan_dir") == 0) {
			NEXT_ARG();
			if (matches(*argv, "ingress") == 0)
				erspan_dir = 0;
			else if (matches(*argv, "egress") == 0)
				erspan_dir = 1;
			else
				invarg("Invalid erspan direction.", *argv);
		} else if (is_erspan && strcmp(*argv, "erspan_hwid") == 0) {
			NEXT_ARG();
			if (get_u16(&erspan_hwid, *argv, 0))
				invarg("invalid erspan hwid\n", *argv);
		} else {
			gre_print_help(lu, argc, argv, stderr);
			return -1;
		}
		argc--; argv++;
	}

	if (is_addrtype_inet_multi(&daddr)) {
		if (!ikey) {
			ikey = daddr.data[0];
			iflags |= GRE_KEY;
		}
		if (!okey) {
			okey = daddr.data[0];
			oflags |= GRE_KEY;
		}
		if (!is_addrtype_inet_not_unspec(&saddr)) {
			fprintf(stderr,
				"A broadcast tunnel requires a source address.\n");
			return -1;
		}
	}

	if (metadata) {
		addattr_l(n, 1024, IFLA_GRE_COLLECT_METADATA, NULL, 0);
		return 0;
	}

	addattr32(n, 1024, IFLA_GRE_IKEY, ikey);
	addattr32(n, 1024, IFLA_GRE_OKEY, okey);
	addattr_l(n, 1024, IFLA_GRE_IFLAGS, &iflags, 2);
	addattr_l(n, 1024, IFLA_GRE_OFLAGS, &oflags, 2);
	if (is_addrtype_inet_not_unspec(&saddr))
		addattr_l(n, 1024, IFLA_GRE_LOCAL, saddr.data, saddr.bytelen);
	if (is_addrtype_inet_not_unspec(&daddr))
		addattr_l(n, 1024, IFLA_GRE_REMOTE, daddr.data, daddr.bytelen);
	addattr_l(n, 1024, IFLA_GRE_PMTUDISC, &pmtudisc, 1);
	if (ignore_df)
		addattr8(n, 1024, IFLA_GRE_IGNORE_DF, ignore_df & 1);
	addattr_l(n, 1024, IFLA_GRE_TOS, &tos, 1);
	if (link)
		addattr32(n, 1024, IFLA_GRE_LINK, link);
	addattr_l(n, 1024, IFLA_GRE_TTL, &ttl, 1);
	addattr32(n, 1024, IFLA_GRE_FWMARK, fwmark);
	if (is_erspan) {
		addattr8(n, 1024, IFLA_GRE_ERSPAN_VER, erspan_ver);
		if (erspan_ver == 1 && erspan_idx != 0) {
			addattr32(n, 1024, IFLA_GRE_ERSPAN_INDEX, erspan_idx);
		} else if (erspan_ver == 2) {
			addattr8(n, 1024, IFLA_GRE_ERSPAN_DIR, erspan_dir);
			addattr16(n, 1024, IFLA_GRE_ERSPAN_HWID, erspan_hwid);
		}
	}
	addattr16(n, 1024, IFLA_GRE_ENCAP_TYPE, encaptype);
	addattr16(n, 1024, IFLA_GRE_ENCAP_FLAGS, encapflags);
	addattr16(n, 1024, IFLA_GRE_ENCAP_SPORT, htons(encapsport));
	addattr16(n, 1024, IFLA_GRE_ENCAP_DPORT, htons(encapdport));

	return 0;
}

static void gre_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	char s2[64];
	__u16 iflags = 0;
	__u16 oflags = 0;
	__u8 ttl = 0;
	__u8 tos = 0;

	if (!tb)
		return;

	if (tb[IFLA_GRE_COLLECT_METADATA]) {
		print_bool(PRINT_ANY, "external", "external ", true);
		return;
	}

	tnl_print_endpoint("remote", tb[IFLA_GRE_REMOTE], AF_INET);
	tnl_print_endpoint("local", tb[IFLA_GRE_LOCAL], AF_INET);

	if (tb[IFLA_GRE_LINK]) {
		__u32 link = rta_getattr_u32(tb[IFLA_GRE_LINK]);

		if (link) {
			print_string(PRINT_ANY, "link", "dev %s ",
				     ll_index_to_name(link));
		}
	}

	if (tb[IFLA_GRE_TTL])
		ttl = rta_getattr_u8(tb[IFLA_GRE_TTL]);
	if (is_json_context() || ttl)
		print_uint(PRINT_ANY, "ttl", "ttl %u ", ttl);
	else
		print_string(PRINT_FP, NULL, "ttl %s ", "inherit");

	if (tb[IFLA_GRE_TOS])
		tos = rta_getattr_u8(tb[IFLA_GRE_TOS]);
	if (tos) {
		if (is_json_context() || tos != 1)
			print_0xhex(PRINT_ANY, "tos", "tos %#llx ", tos);
		else
			print_string(PRINT_FP, NULL, "tos %s ", "inherit");
	}

	if (tb[IFLA_GRE_PMTUDISC]) {
		if (!rta_getattr_u8(tb[IFLA_GRE_PMTUDISC]))
			print_bool(PRINT_ANY, "pmtudisc", "nopmtudisc ", false);
		else
			print_bool(PRINT_JSON, "pmtudisc", NULL, true);
	}

	if (tb[IFLA_GRE_IGNORE_DF] && rta_getattr_u8(tb[IFLA_GRE_IGNORE_DF]))
		print_bool(PRINT_ANY, "ignore_df", "ignore-df ", true);

	if (tb[IFLA_GRE_IFLAGS])
		iflags = rta_getattr_u16(tb[IFLA_GRE_IFLAGS]);

	if (tb[IFLA_GRE_OFLAGS])
		oflags = rta_getattr_u16(tb[IFLA_GRE_OFLAGS]);

	if ((iflags & GRE_KEY) && tb[IFLA_GRE_IKEY]) {
		inet_ntop(AF_INET, RTA_DATA(tb[IFLA_GRE_IKEY]), s2, sizeof(s2));
		print_string(PRINT_ANY, "ikey", "ikey %s ", s2);
	}

	if ((oflags & GRE_KEY) && tb[IFLA_GRE_OKEY]) {
		inet_ntop(AF_INET, RTA_DATA(tb[IFLA_GRE_OKEY]), s2, sizeof(s2));
		print_string(PRINT_ANY, "okey", "okey %s ", s2);
	}

	if (iflags & GRE_SEQ)
		print_bool(PRINT_ANY, "iseq", "iseq ", true);
	if (oflags & GRE_SEQ)
		print_bool(PRINT_ANY, "oseq", "oseq ", true);
	if (iflags & GRE_CSUM)
		print_bool(PRINT_ANY, "icsum", "icsum ", true);
	if (oflags & GRE_CSUM)
		print_bool(PRINT_ANY, "ocsum", "ocsum ", true);

	if (tb[IFLA_GRE_FWMARK]) {
		__u32 fwmark = rta_getattr_u32(tb[IFLA_GRE_FWMARK]);

		if (fwmark) {
			print_0xhex(PRINT_ANY,
				    "fwmark", "fwmark %#llx ", fwmark);
		}
	}

	if (tb[IFLA_GRE_ERSPAN_INDEX]) {
		__u32 erspan_idx = rta_getattr_u32(tb[IFLA_GRE_ERSPAN_INDEX]);

		print_uint(PRINT_ANY,
			   "erspan_index", "erspan_index %u ", erspan_idx);
	}

	if (tb[IFLA_GRE_ERSPAN_VER]) {
		__u8 erspan_ver = rta_getattr_u8(tb[IFLA_GRE_ERSPAN_VER]);

		print_uint(PRINT_ANY,
			   "erspan_ver", "erspan_ver %u ", erspan_ver);
	}

	if (tb[IFLA_GRE_ERSPAN_DIR]) {
		__u8 erspan_dir = rta_getattr_u8(tb[IFLA_GRE_ERSPAN_DIR]);

		if (erspan_dir == 0)
			print_string(PRINT_ANY, "erspan_dir",
				     "erspan_dir ingress ", NULL);
		else
			print_string(PRINT_ANY, "erspan_dir",
				     "erspan_dir egress ", NULL);
	}

	if (tb[IFLA_GRE_ERSPAN_HWID]) {
		__u16 erspan_hwid = rta_getattr_u16(tb[IFLA_GRE_ERSPAN_HWID]);

		print_0xhex(PRINT_ANY,
			    "erspan_hwid", "erspan_hwid %#llx ", erspan_hwid);
	}

	tnl_print_encap(tb,
			IFLA_GRE_ENCAP_TYPE,
			IFLA_GRE_ENCAP_FLAGS,
			IFLA_GRE_ENCAP_SPORT,
			IFLA_GRE_ENCAP_DPORT);
}

struct link_util gre_link_util = {
	.id = "gre",
	.maxattr = IFLA_GRE_MAX,
	.parse_opt = gre_parse_opt,
	.print_opt = gre_print_opt,
	.print_help = gre_print_help,
};

struct link_util gretap_link_util = {
	.id = "gretap",
	.maxattr = IFLA_GRE_MAX,
	.parse_opt = gre_parse_opt,
	.print_opt = gre_print_opt,
	.print_help = gre_print_help,
};

struct link_util erspan_link_util = {
	.id = "erspan",
	.maxattr = IFLA_GRE_MAX,
	.parse_opt = gre_parse_opt,
	.print_opt = gre_print_opt,
	.print_help = gre_print_help,
};
