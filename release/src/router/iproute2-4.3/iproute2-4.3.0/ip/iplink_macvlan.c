/*
 * iplink_macvlan.c	macvlan/macvtap device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Patrick McHardy <kaber@trash.net>
 *		Arnd Bergmann <arnd@arndb.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_link.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

#define pfx_err(lu, ...) {               \
	fprintf(stderr, "%s: ", lu->id); \
	fprintf(stderr, __VA_ARGS__);    \
	fprintf(stderr, "\n");           \
}

static void print_explain(struct link_util *lu, FILE *f)
{
	fprintf(f,
		"Usage: ... %s mode { private | vepa | bridge | passthru [nopromisc] }\n",
		lu->id
	);
}

static void explain(struct link_util *lu)
{
	print_explain(lu, stderr);
}

static int mode_arg(const char *arg)
{
        fprintf(stderr, "Error: argument of \"mode\" must be \"private\", "
		"\"vepa\", \"bridge\" or \"passthru\", not \"%s\"\n", arg);
        return -1;
}

static int macvlan_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{
	__u32 mode = 0;
	__u16 flags = 0;

	while (argc > 0) {
		if (matches(*argv, "mode") == 0) {
			NEXT_ARG();

			if (strcmp(*argv, "private") == 0)
				mode = MACVLAN_MODE_PRIVATE;
			else if (strcmp(*argv, "vepa") == 0)
				mode = MACVLAN_MODE_VEPA;
			else if (strcmp(*argv, "bridge") == 0)
				mode = MACVLAN_MODE_BRIDGE;
			else if (strcmp(*argv, "passthru") == 0)
				mode = MACVLAN_MODE_PASSTHRU;
			else
				return mode_arg(*argv);
		} else if (matches(*argv, "nopromisc") == 0) {
			flags |= MACVLAN_FLAG_NOPROMISC;
		} else if (matches(*argv, "help") == 0) {
			explain(lu);
			return -1;
		} else {
			pfx_err(lu, "unknown option \"%s\"?", *argv);
			explain(lu);
			return -1;
		}
		argc--, argv++;
	}

	if (mode)
		addattr32(n, 1024, IFLA_MACVLAN_MODE, mode);

	if (flags) {
		if (flags & MACVLAN_FLAG_NOPROMISC &&
		    mode != MACVLAN_MODE_PASSTHRU) {
			pfx_err(lu, "nopromisc flag only valid in passthru mode");
			explain(lu);
			return -1;
		}
		addattr16(n, 1024, IFLA_MACVLAN_FLAGS, flags);
	}
	return 0;
}

static void macvlan_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	__u32 mode;
	__u16 flags;

	if (!tb)
		return;

	if (!tb[IFLA_MACVLAN_MODE] ||
	    RTA_PAYLOAD(tb[IFLA_MACVLAN_MODE]) < sizeof(__u32))
		return;

	mode = rta_getattr_u32(tb[IFLA_MACVLAN_MODE]);
	fprintf(f, " mode %s ",
		  mode == MACVLAN_MODE_PRIVATE ? "private"
		: mode == MACVLAN_MODE_VEPA    ? "vepa"
		: mode == MACVLAN_MODE_BRIDGE  ? "bridge"
		: mode == MACVLAN_MODE_PASSTHRU  ? "passthru"
		:				 "unknown");

	if (!tb[IFLA_MACVLAN_FLAGS] ||
	    RTA_PAYLOAD(tb[IFLA_MACVLAN_FLAGS]) < sizeof(__u16))
		return;

	flags = rta_getattr_u16(tb[IFLA_MACVLAN_FLAGS]);
	if (flags & MACVLAN_FLAG_NOPROMISC)
		fprintf(f, "nopromisc ");
}

static void macvlan_print_help(struct link_util *lu, int argc, char **argv,
	FILE *f)
{
	print_explain(lu, f);
}

struct link_util macvlan_link_util = {
	.id		= "macvlan",
	.maxattr	= IFLA_MACVLAN_MAX,
	.parse_opt	= macvlan_parse_opt,
	.print_opt	= macvlan_print_opt,
	.print_help	= macvlan_print_help,
};

struct link_util macvtap_link_util = {
	.id		= "macvtap",
	.maxattr	= IFLA_MACVLAN_MAX,
	.parse_opt	= macvlan_parse_opt,
	.print_opt	= macvlan_print_opt,
	.print_help	= macvlan_print_help,
};
