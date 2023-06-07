/*
 * m_connmark.c		Connection tracking marking import
 *
 * Copyright (c) 2011 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "tc_util.h"
#include <linux/tc_act/tc_connmark.h>

static void
explain(void)
{
	fprintf(stderr,
		"Usage: ... connmark [zone ZONE] [CONTROL] [index <INDEX>]\n"
		"where :\n"
		"\tZONE is the conntrack zone\n"
		"\tCONTROL := reclassify | pipe | drop | continue | ok |\n"
		"\t           goto chain <CHAIN_INDEX>\n");
}

static void
usage(void)
{
	explain();
	exit(-1);
}

static int
parse_connmark(struct action_util *a, int *argc_p, char ***argv_p, int tca_id,
	      struct nlmsghdr *n)
{
	struct tc_connmark sel = {};
	char **argv = *argv_p;
	int argc = *argc_p;
	int ok = 0;
	struct rtattr *tail;

	while (argc > 0) {
		if (matches(*argv, "connmark") == 0) {
			ok = 1;
			argc--;
			argv++;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}

	}

	if (!ok) {
		explain();
		return -1;
	}

	if (argc) {
		if (matches(*argv, "zone") == 0) {
			NEXT_ARG();
			if (get_u16(&sel.zone, *argv, 10)) {
				fprintf(stderr, "connmark: Illegal \"zone\"\n");
				return -1;
			}
			argc--;
			argv++;
		}
	}

	parse_action_control_dflt(&argc, &argv, &sel.action, false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.index, *argv, 10)) {
				fprintf(stderr, "connmark: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
		}
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_CONNMARK_PARMS, &sel, sizeof(sel));
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_connmark(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_CONNMARK_MAX + 1];
	struct tc_connmark *ci;

	print_string(PRINT_ANY, "kind", "%s ", "connmark");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_CONNMARK_MAX, arg);
	if (tb[TCA_CONNMARK_PARMS] == NULL) {
		fprintf(stderr, "Missing connmark parameters\n");
		return -1;
	}

	ci = RTA_DATA(tb[TCA_CONNMARK_PARMS]);

	print_uint(PRINT_ANY, "zone", "zone %u", ci->zone);
	print_action_control(f, " ", ci->action, "");

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", ci->index);
	print_int(PRINT_ANY, "ref", " ref %d", ci->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", ci->bindcnt);

	if (show_stats) {
		if (tb[TCA_CONNMARK_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_CONNMARK_TM]);

			print_tm(f, tm);
		}
	}
	print_nl();

	return 0;
}

struct action_util connmark_action_util = {
	.id = "connmark",
	.parse_aopt = parse_connmark,
	.print_aopt = print_connmark,
};
