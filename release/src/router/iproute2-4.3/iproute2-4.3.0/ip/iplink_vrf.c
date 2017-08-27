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

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

static void vrf_explain(FILE *f)
{
	fprintf(f, "Usage: ... vrf table TABLEID \n");
}

static void explain(void)
{
	vrf_explain(stderr);
}

static int table_arg(void)
{
	fprintf(stderr,"Error: argument of \"table\" must be 0-32767 and currently unused\n");
	return -1;
}

static int vrf_parse_opt(struct link_util *lu, int argc, char **argv,
			    struct nlmsghdr *n)
{
	while (argc > 0) {
		if (matches(*argv, "table") == 0) {
			__u32 table;

			NEXT_ARG();

			table = atoi(*argv);
			if (table > 32767)
				return table_arg();
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
		fprintf(f, "table %u ", rta_getattr_u32(tb[IFLA_VRF_TABLE]));
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
