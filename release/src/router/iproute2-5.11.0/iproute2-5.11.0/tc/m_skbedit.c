/*
 * m_skbedit.c		SKB Editing module
 *
 * Copyright (c) 2008, Intel Corporation.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 *
 * Authors:	Alexander Duyck <alexander.h.duyck@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "tc_util.h"
#include <linux/tc_act/tc_skbedit.h>
#include <linux/if_packet.h>

static void explain(void)
{
	fprintf(stderr, "Usage: ... skbedit <[QM] [PM] [MM] [PT] [IF]>\n"
		"QM = queue_mapping QUEUE_MAPPING\n"
		"PM = priority PRIORITY\n"
		"MM = mark MARK[/MASK]\n"
		"PT = ptype PACKETYPE\n"
		"IF = inheritdsfield\n"
		"PACKETYPE = is one of:\n"
		"  host, otherhost, broadcast, multicast\n"
		"QUEUE_MAPPING = device transmit queue to use\n"
		"PRIORITY = classID to assign to priority field\n"
		"MARK = firewall mark to set\n"
		"MASK = mask applied to firewall mark (0xffffffff by default)\n"
		"note: inheritdsfield maps DS field to skb->priority\n");
}

static void
usage(void)
{
	explain();
	exit(-1);
}

static int
parse_skbedit(struct action_util *a, int *argc_p, char ***argv_p, int tca_id,
	      struct nlmsghdr *n)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	int ok = 0;
	struct rtattr *tail;
	unsigned int tmp;
	__u16 queue_mapping, ptype;
	__u32 flags = 0, priority, mark, mask;
	__u64 pure_flags = 0;
	struct tc_skbedit sel = { 0 };

	if (matches(*argv, "skbedit") != 0)
		return -1;

	NEXT_ARG();

	while (argc > 0) {
		if (matches(*argv, "queue_mapping") == 0) {
			flags |= SKBEDIT_F_QUEUE_MAPPING;
			NEXT_ARG();
			if (get_unsigned(&tmp, *argv, 10) || tmp > 65535) {
				fprintf(stderr, "Illegal queue_mapping\n");
				return -1;
			}
			queue_mapping = tmp;
			ok++;
		} else if (matches(*argv, "priority") == 0) {
			flags |= SKBEDIT_F_PRIORITY;
			NEXT_ARG();
			if (get_tc_classid(&priority, *argv)) {
				fprintf(stderr, "Illegal priority\n");
				return -1;
			}
			ok++;
		} else if (matches(*argv, "mark") == 0) {
			char *slash;

			NEXT_ARG();
			slash = strchr(*argv, '/');
			if (slash)
				*slash = '\0';

			flags |= SKBEDIT_F_MARK;
			if (get_u32(&mark, *argv, 0)) {
				fprintf(stderr, "Illegal mark\n");
				return -1;
			}

			if (slash) {
				if (get_u32(&mask, slash + 1, 0)) {
					fprintf(stderr, "Illegal mask\n");
					return -1;
				}
				flags |= SKBEDIT_F_MASK;
			}
			ok++;
		} else if (matches(*argv, "ptype") == 0) {

			NEXT_ARG();
			if (matches(*argv, "host") == 0) {
				ptype = PACKET_HOST;
			} else if (matches(*argv, "broadcast") == 0) {
				ptype = PACKET_BROADCAST;
			} else if (matches(*argv, "multicast") == 0) {
				ptype = PACKET_MULTICAST;
			} else if (matches(*argv, "otherhost") == 0) {
				ptype = PACKET_OTHERHOST;
			} else {
				fprintf(stderr, "Illegal ptype (%s)\n",
					*argv);
				return -1;
			}
			flags |= SKBEDIT_F_PTYPE;
			ok++;
		} else if (matches(*argv, "inheritdsfield") == 0) {
			pure_flags |= SKBEDIT_F_INHERITDSFIELD;
			ok++;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}
		argc--;
		argv++;
	}

	parse_action_control_dflt(&argc, &argv, &sel.action,
				  false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.index, *argv, 10)) {
				fprintf(stderr, "skbedit: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
			ok++;
		}
	}

	if (!ok) {
		explain();
		return -1;
	}


	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_SKBEDIT_PARMS, &sel, sizeof(sel));
	if (flags & SKBEDIT_F_QUEUE_MAPPING)
		addattr_l(n, MAX_MSG, TCA_SKBEDIT_QUEUE_MAPPING,
			  &queue_mapping, sizeof(queue_mapping));
	if (flags & SKBEDIT_F_PRIORITY)
		addattr_l(n, MAX_MSG, TCA_SKBEDIT_PRIORITY,
			  &priority, sizeof(priority));
	if (flags & SKBEDIT_F_MARK)
		addattr_l(n, MAX_MSG, TCA_SKBEDIT_MARK,
			  &mark, sizeof(mark));
	if (flags & SKBEDIT_F_MASK)
		addattr_l(n, MAX_MSG, TCA_SKBEDIT_MASK,
			  &mask, sizeof(mask));
	if (flags & SKBEDIT_F_PTYPE)
		addattr_l(n, MAX_MSG, TCA_SKBEDIT_PTYPE,
			  &ptype, sizeof(ptype));
	if (pure_flags != 0)
		addattr64(n, MAX_MSG, TCA_SKBEDIT_FLAGS, pure_flags);
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_skbedit(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_SKBEDIT_MAX + 1];

	SPRINT_BUF(b1);
	__u32 priority;
	__u16 ptype;
	struct tc_skbedit *p;

	print_string(PRINT_ANY, "kind", "%s ", "skbedit");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_SKBEDIT_MAX, arg);

	if (tb[TCA_SKBEDIT_PARMS] == NULL) {
		fprintf(stderr, "Missing skbedit parameters\n");
		return -1;
	}
	p = RTA_DATA(tb[TCA_SKBEDIT_PARMS]);

	if (tb[TCA_SKBEDIT_QUEUE_MAPPING] != NULL) {
		print_uint(PRINT_ANY, "queue_mapping", "queue_mapping %u",
			   rta_getattr_u16(tb[TCA_SKBEDIT_QUEUE_MAPPING]));
	}
	if (tb[TCA_SKBEDIT_PRIORITY] != NULL) {
		priority = rta_getattr_u32(tb[TCA_SKBEDIT_PRIORITY]);
		print_string(PRINT_ANY, "priority", " priority %s",
			     sprint_tc_classid(priority, b1));
	}
	if (tb[TCA_SKBEDIT_MARK] != NULL) {
		print_uint(PRINT_ANY, "mark", " mark %u",
			   rta_getattr_u32(tb[TCA_SKBEDIT_MARK]));
	}
	if (tb[TCA_SKBEDIT_MASK]) {
		print_hex(PRINT_ANY, "mask", "/%#x",
			  rta_getattr_u32(tb[TCA_SKBEDIT_MASK]));
	}
	if (tb[TCA_SKBEDIT_PTYPE] != NULL) {
		ptype = rta_getattr_u16(tb[TCA_SKBEDIT_PTYPE]);
		if (ptype == PACKET_HOST)
			print_string(PRINT_ANY, "ptype", " ptype %s", "host");
		else if (ptype == PACKET_BROADCAST)
			print_string(PRINT_ANY, "ptype", " ptype %s",
				     "broadcast");
		else if (ptype == PACKET_MULTICAST)
			print_string(PRINT_ANY, "ptype", " ptype %s",
				     "multicast");
		else if (ptype == PACKET_OTHERHOST)
			print_string(PRINT_ANY, "ptype", " ptype %s",
				     "otherhost");
		else
			print_uint(PRINT_ANY, "ptype", " ptype %u", ptype);
	}
	if (tb[TCA_SKBEDIT_FLAGS] != NULL) {
		__u64 flags = rta_getattr_u64(tb[TCA_SKBEDIT_FLAGS]);

		if (flags & SKBEDIT_F_INHERITDSFIELD)
			print_null(PRINT_ANY, "inheritdsfield", " %s",
				     "inheritdsfield");
	}

	print_action_control(f, " ", p->action, "");

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", p->index);
	print_int(PRINT_ANY, "ref", " ref %d", p->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", p->bindcnt);

	if (show_stats) {
		if (tb[TCA_SKBEDIT_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_SKBEDIT_TM]);

			print_tm(f, tm);
		}
	}

	print_nl();

	return 0;
}

struct action_util skbedit_action_util = {
	.id = "skbedit",
	.parse_aopt = parse_skbedit,
	.print_aopt = print_skbedit,
};
