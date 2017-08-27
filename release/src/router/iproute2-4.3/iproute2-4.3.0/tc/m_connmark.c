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
	fprintf(stderr, "Usage: ... connmark [ZONE] [BRANCH] [index <INDEX>]\n");
	fprintf(stderr, "where :\n"
		"\tZONE is the conntrack zone\n"
		"\tBRANCH := reclassify|pipe|drop|continue|ok\n");
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
				fprintf(stderr, "simple: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
		}
	}

	sel.action = TC_ACT_PIPE;
	if (argc) {
		if (matches(*argv, "reclassify") == 0) {
			sel.action = TC_ACT_RECLASSIFY;
			argc--;
			argv++;
		} else if (matches(*argv, "pipe") == 0) {
			sel.action = TC_ACT_PIPE;
			argc--;
			argv++;
		} else if (matches(*argv, "drop") == 0 ||
			   matches(*argv, "shot") == 0) {
			sel.action = TC_ACT_SHOT;
			argc--;
			argv++;
		} else if (matches(*argv, "continue") == 0) {
			sel.action = TC_ACT_UNSPEC;
			argc--;
			argv++;
		} else if (matches(*argv, "pass") == 0) {
			sel.action = TC_ACT_OK;
			argc--;
			argv++;
		}
	}

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.index, *argv, 10)) {
				fprintf(stderr, "simple: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
		}
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, MAX_MSG, tca_id, NULL, 0);
	addattr_l(n, MAX_MSG, TCA_CONNMARK_PARMS, &sel, sizeof(sel));
	tail->rta_len = (char *)NLMSG_TAIL(n) - (char *)tail;

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_connmark(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_CONNMARK_MAX + 1];
	struct tc_connmark *ci;

	if (arg == NULL)
		return -1;

	parse_rtattr_nested(tb, TCA_CONNMARK_MAX, arg);
	if (tb[TCA_CONNMARK_PARMS] == NULL) {
		fprintf(f, "[NULL connmark parameters]");
		return -1;
	}

	ci = RTA_DATA(tb[TCA_CONNMARK_PARMS]);

	fprintf(f, " connmark zone %d\n", ci->zone);
	fprintf(f, "\t index %d ref %d bind %d", ci->index,
		ci->refcnt, ci->bindcnt);

	if (show_stats) {
		if (tb[TCA_CONNMARK_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_CONNMARK_TM]);
			print_tm(f, tm);
		}
	}
	fprintf(f, "\n");

	return 0;
}

struct action_util connmark_action_util = {
	.id = "connmark",
	.parse_aopt = parse_connmark,
	.print_aopt = print_connmark,
};
