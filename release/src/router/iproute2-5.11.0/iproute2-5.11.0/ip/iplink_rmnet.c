/* SPDX-License-Identifier: GPL-2.0 */
/*
 * iplink_rmnet.c	RMNET device support
 *
 * Authors:     Daniele Palmas <dnlplm@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "ip_common.h"

static void print_explain(FILE *f)
{
	fprintf(f,
		"Usage: ... rmnet mux_id MUXID\n"
		"\n"
		"MUXID := 1-254\n"
	);
}

static void explain(void)
{
	print_explain(stderr);
}

static int rmnet_parse_opt(struct link_util *lu, int argc, char **argv,
			   struct nlmsghdr *n)
{
	__u16 mux_id;

	while (argc > 0) {
		if (matches(*argv, "mux_id") == 0) {
			NEXT_ARG();
			if (get_u16(&mux_id, *argv, 0))
				invarg("mux_id is invalid", *argv);
			addattr16(n, 1024, IFLA_RMNET_MUX_ID, mux_id);
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "rmnet: unknown command \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	return 0;
}

static void rmnet_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	if (!tb)
		return;

	if (!tb[IFLA_RMNET_MUX_ID] ||
	    RTA_PAYLOAD(tb[IFLA_RMNET_MUX_ID]) < sizeof(__u16))
		return;

	print_uint(PRINT_ANY,
		   "mux_id",
		   "mux_id %u ",
		   rta_getattr_u16(tb[IFLA_RMNET_MUX_ID]));
}

static void rmnet_print_help(struct link_util *lu, int argc, char **argv,
			     FILE *f)
{
	print_explain(f);
}

struct link_util rmnet_link_util = {
	.id		= "rmnet",
	.maxattr	= IFLA_RMNET_MAX,
	.parse_opt	= rmnet_parse_opt,
	.print_opt	= rmnet_print_opt,
	.print_help	= rmnet_print_help,
};
