// SPDX-License-Identifier: GPL-2.0
/*
 * link_xfrm.c	Virtual XFRM Interface driver module
 *
 * Authors:	Matt Ellison <matt@arroyo.io>
 */

#include <string.h>
#include <linux/if_link.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "tunnel.h"

static void xfrm_print_help(struct link_util *lu, int argc, char **argv,
			    FILE *f)
{
	fprintf(f,
		"Usage: ... %-4s dev [ PHYS_DEV ] [ if_id IF-ID ]\n"
		"\n"
		"Where: IF-ID := { 0x0..0xffffffff }\n",
		lu->id);
}

static int xfrm_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{
	unsigned int link = 0;
	__u32 if_id = 0;

	while (argc > 0) {
		if (!matches(*argv, "dev")) {
			NEXT_ARG();
			link = ll_name_to_index(*argv);
			if (!link)
				exit(nodev(*argv));
		} else if (!matches(*argv, "if_id")) {
			NEXT_ARG();
			if (get_u32(&if_id, *argv, 0))
				invarg("if_id value is invalid", *argv);
			else
				addattr32(n, 1024, IFLA_XFRM_IF_ID, if_id);
		} else {
			xfrm_print_help(lu, argc, argv, stderr);
			return -1;
		}
		argc--; argv++;
	}

	if (link)
		addattr32(n, 1024, IFLA_XFRM_LINK, link);

	return 0;
}

static void xfrm_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{

	if (!tb)
		return;

	if (tb[IFLA_XFRM_IF_ID]) {
		__u32 id = rta_getattr_u32(tb[IFLA_XFRM_IF_ID]);

		print_0xhex(PRINT_ANY, "if_id", "if_id %#llx ", id);

	}

}

struct link_util xfrm_link_util = {
	.id = "xfrm",
	.maxattr = IFLA_XFRM_MAX,
	.parse_opt = xfrm_parse_opt,
	.print_opt = xfrm_print_opt,
	.print_help = xfrm_print_help,
};
