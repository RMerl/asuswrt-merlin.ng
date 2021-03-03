/*
 * link_vti.c	VTI driver module
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Herbert Xu <herbert@gondor.apana.org.au>
 *          Saurabh Mohan <saurabh.mohan@vyatta.com> Modified link_gre.c for VTI
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

static void vti_print_help(struct link_util *lu, int argc, char **argv, FILE *f)
{
	fprintf(f,
		"Usage: ... %-4s	[ remote ADDR ]\n"
		"		[ local ADDR ]\n"
		"		[ [i|o]key KEY ]\n"
		"		[ dev PHYS_DEV ]\n"
		"		[ fwmark MARK ]\n"
		"\n"
		"Where:	ADDR := { IP_ADDRESS }\n"
		"	KEY  := { DOTTED_QUAD | NUMBER }\n"
		"	MARK := { 0x0..0xffffffff }\n",
		lu->id);
}

static int vti_parse_opt(struct link_util *lu, int argc, char **argv,
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
	struct rtattr *vtiinfo[IFLA_VTI_MAX + 1];
	__be32 ikey = 0;
	__be32 okey = 0;
	inet_prefix saddr, daddr;
	unsigned int link = 0;
	__u32 fwmark = 0;
	int len;

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

		parse_rtattr_nested(vtiinfo, IFLA_VTI_MAX,
				    linkinfo[IFLA_INFO_DATA]);

		rta = vtiinfo[IFLA_VTI_LOCAL];
		if (rta && get_addr_rta(&saddr, rta, AF_INET))
			goto get_failed;

		rta = vtiinfo[IFLA_VTI_REMOTE];
		if (rta && get_addr_rta(&daddr, rta, AF_INET))
			goto get_failed;

		if (vtiinfo[IFLA_VTI_IKEY])
			ikey = rta_getattr_u32(vtiinfo[IFLA_VTI_IKEY]);

		if (vtiinfo[IFLA_VTI_OKEY])
			okey = rta_getattr_u32(vtiinfo[IFLA_VTI_OKEY]);

		if (vtiinfo[IFLA_VTI_LINK])
			link = rta_getattr_u8(vtiinfo[IFLA_VTI_LINK]);

		if (vtiinfo[IFLA_VTI_FWMARK])
			fwmark = rta_getattr_u32(vtiinfo[IFLA_VTI_FWMARK]);

		free(answer);
	}

	while (argc > 0) {
		if (!matches(*argv, "key")) {
			NEXT_ARG();
			ikey = okey = tnl_parse_key("key", *argv);
		} else if (!matches(*argv, "ikey")) {
			NEXT_ARG();
			ikey = tnl_parse_key("ikey", *argv);
		} else if (!matches(*argv, "okey")) {
			NEXT_ARG();
			okey = tnl_parse_key("okey", *argv);
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
		} else if (strcmp(*argv, "fwmark") == 0) {
			NEXT_ARG();
			if (get_u32(&fwmark, *argv, 0))
				invarg("invalid fwmark\n", *argv);
		} else {
			vti_print_help(lu, argc, argv, stderr);
			return -1;
		}
		argc--; argv++;
	}

	addattr32(n, 1024, IFLA_VTI_IKEY, ikey);
	addattr32(n, 1024, IFLA_VTI_OKEY, okey);
	if (is_addrtype_inet_not_unspec(&saddr))
		addattr_l(n, 1024, IFLA_VTI_LOCAL, saddr.data, saddr.bytelen);
	if (is_addrtype_inet_not_unspec(&daddr))
		addattr_l(n, 1024, IFLA_VTI_REMOTE, daddr.data, daddr.bytelen);
	addattr32(n, 1024, IFLA_VTI_FWMARK, fwmark);
	if (link)
		addattr32(n, 1024, IFLA_VTI_LINK, link);

	return 0;
}

static void vti_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	char s2[64];

	if (!tb)
		return;

	tnl_print_endpoint("remote", tb[IFLA_VTI_REMOTE], AF_INET);
	tnl_print_endpoint("local", tb[IFLA_VTI_LOCAL], AF_INET);

	if (tb[IFLA_VTI_LINK]) {
		__u32 link = rta_getattr_u32(tb[IFLA_VTI_LINK]);

		if (link) {
			print_string(PRINT_ANY, "link", "dev %s ",
				     ll_index_to_name(link));
		}
	}

	if (tb[IFLA_VTI_IKEY]) {
		struct rtattr *rta = tb[IFLA_VTI_IKEY];
		__u32 key = rta_getattr_u32(rta);

		if (key && inet_ntop(AF_INET, RTA_DATA(rta), s2, sizeof(s2)))
			print_string(PRINT_ANY, "ikey", "ikey %s ", s2);
	}

	if (tb[IFLA_VTI_OKEY]) {
		struct rtattr *rta = tb[IFLA_VTI_OKEY];
		__u32 key = rta_getattr_u32(rta);

		if (key && inet_ntop(AF_INET, RTA_DATA(rta), s2, sizeof(s2)))
			print_string(PRINT_ANY, "okey", "okey %s ", s2);
	}

	if (tb[IFLA_VTI_FWMARK]) {
		__u32 fwmark = rta_getattr_u32(tb[IFLA_VTI_FWMARK]);

		if (fwmark) {
			print_0xhex(PRINT_ANY,
				    "fwmark", "fwmark %#llx ", fwmark);
		}
	}
}

struct link_util vti_link_util = {
	.id = "vti",
	.maxattr = IFLA_VTI_MAX,
	.parse_opt = vti_parse_opt,
	.print_opt = vti_print_opt,
	.print_help = vti_print_help,
};
