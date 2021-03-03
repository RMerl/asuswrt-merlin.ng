/* iplink_vrf.c	VRF device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Shrijeet Mukherjee <shm@cumulusnetworks.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_link.h>
#include <errno.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

static void vrf_explain(FILE *f)
{
	fprintf(f, "Usage: ... vrf table TABLEID\n");
}

static void explain(void)
{
	vrf_explain(stderr);
}

static int vrf_parse_opt(struct link_util *lu, int argc, char **argv,
			    struct nlmsghdr *n)
{
	while (argc > 0) {
		if (matches(*argv, "table") == 0) {
			__u32 table;

			NEXT_ARG();

			if (rtnl_rttable_a2n(&table, *argv))
				invarg("invalid table ID\n", *argv);
			addattr32(n, 1024, IFLA_VRF_TABLE, table);
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "vrf: unknown option \"%s\"?\n",
				*argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	return 0;
}

static void vrf_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	if (!tb)
		return;

	if (tb[IFLA_VRF_TABLE])
		print_uint(PRINT_ANY,
			   "table",
			   "table %u ",
			   rta_getattr_u32(tb[IFLA_VRF_TABLE]));
}

static void vrf_slave_print_opt(struct link_util *lu, FILE *f,
				struct rtattr *tb[])
{
	if (!tb)
		return;

	if (tb[IFLA_VRF_PORT_TABLE]) {
		print_uint(PRINT_ANY,
			   "table",
			   "table %u ",
			   rta_getattr_u32(tb[IFLA_VRF_PORT_TABLE]));
	}
}

static void vrf_print_help(struct link_util *lu, int argc, char **argv,
			   FILE *f)
{
	vrf_explain(f);
}

struct link_util vrf_link_util = {
	.id		= "vrf",
	.maxattr	= IFLA_VRF_MAX,
	.parse_opt	= vrf_parse_opt,
	.print_opt	= vrf_print_opt,
	.print_help	= vrf_print_help,
};

struct link_util vrf_slave_link_util = {
	.id             = "vrf_slave",
	.maxattr        = IFLA_VRF_PORT_MAX,
	.print_opt	= vrf_slave_print_opt,
};

/* returns table id if name is a VRF device */
__u32 ipvrf_get_table(const char *name)
{
	struct {
		struct nlmsghdr		n;
		struct ifinfomsg	i;
		char			buf[1024];
	} req = {
		.n = {
			.nlmsg_len   = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
			.nlmsg_flags = NLM_F_REQUEST,
			.nlmsg_type  = RTM_GETLINK,
		},
		.i = {
			.ifi_family  = preferred_family,
		},
	};
	struct nlmsghdr *answer;
	struct rtattr *tb[IFLA_MAX+1];
	struct rtattr *li[IFLA_INFO_MAX+1];
	struct rtattr *vrf_attr[IFLA_VRF_MAX + 1];
	struct ifinfomsg *ifi;
	__u32 tb_id = 0;
	int len;

	addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, strlen(name) + 1);

	if (rtnl_talk_suppress_rtnl_errmsg(&rth, &req.n, &answer) < 0) {
		/* special case "default" vrf to be the main table */
		if (errno == ENODEV && !strcmp(name, "default"))
			if (rtnl_rttable_a2n(&tb_id, "main"))
				fprintf(stderr,
					"BUG: RTTable \"main\" not found.\n");

		return tb_id;
	}

	ifi = NLMSG_DATA(answer);
	len = answer->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0) {
		fprintf(stderr, "BUG: Invalid response to link query.\n");
		goto out;
	}

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);

	if (!tb[IFLA_LINKINFO])
		goto out;

	parse_rtattr_nested(li, IFLA_INFO_MAX, tb[IFLA_LINKINFO]);

	if (!li[IFLA_INFO_KIND] || !li[IFLA_INFO_DATA])
		goto out;

	if (strcmp(RTA_DATA(li[IFLA_INFO_KIND]), "vrf"))
		goto out;

	parse_rtattr_nested(vrf_attr, IFLA_VRF_MAX, li[IFLA_INFO_DATA]);
	if (vrf_attr[IFLA_VRF_TABLE])
		tb_id = rta_getattr_u32(vrf_attr[IFLA_VRF_TABLE]);

	if (!tb_id)
		fprintf(stderr, "BUG: VRF %s is missing table id\n", name);

out:
	free(answer);
	return tb_id;
}

int name_is_vrf(const char *name)
{
	struct {
		struct nlmsghdr		n;
		struct ifinfomsg	i;
		char			buf[1024];
	} req = {
		.n = {
			.nlmsg_len   = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
			.nlmsg_flags = NLM_F_REQUEST,
			.nlmsg_type  = RTM_GETLINK,
		},
		.i = {
			.ifi_family  = preferred_family,
		},
	};
	struct nlmsghdr *answer;
	struct rtattr *tb[IFLA_MAX+1];
	struct rtattr *li[IFLA_INFO_MAX+1];
	struct ifinfomsg *ifi;
	int ifindex = 0;
	int len;

	addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, strlen(name) + 1);

	if (rtnl_talk_suppress_rtnl_errmsg(&rth, &req.n, &answer) < 0)
		return 0;

	ifi = NLMSG_DATA(answer);
	len = answer->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0) {
		fprintf(stderr, "BUG: Invalid response to link query.\n");
		goto out;
	}

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);

	if (!tb[IFLA_LINKINFO])
		goto out;

	parse_rtattr_nested(li, IFLA_INFO_MAX, tb[IFLA_LINKINFO]);

	if (!li[IFLA_INFO_KIND])
		goto out;

	if (strcmp(RTA_DATA(li[IFLA_INFO_KIND]), "vrf"))
		goto out;

	ifindex = ifi->ifi_index;
out:
	free(answer);
	return ifindex;
}
