/* SPDX-License-Identifier: GPL-2.0 */
/*
 * m_ctinfo.c		netfilter ctinfo mark action
 *
 * Copyright (c) 2019 Kevin Darbyshire-Bryant <ldir@darbyshire-bryant.me.uk>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "tc_util.h"
#include <linux/tc_act/tc_ctinfo.h>

static void
explain(void)
{
	fprintf(stderr,
		"Usage: ... ctinfo [dscp mask [statemask]] [cpmark [mask]] [zone ZONE] [CONTROL] [index <INDEX>]\n"
		"where :\n"
		"\tdscp   MASK bitmask location of stored DSCP\n"
		"\t       STATEMASK bitmask to determine conditional restoring\n"
		"\tcpmark MASK mask applied to mark on restoration\n"
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
parse_ctinfo(struct action_util *a, int *argc_p, char ***argv_p, int tca_id,
	     struct nlmsghdr *n)
{
	unsigned int cpmarkmask = 0, dscpmask = 0, dscpstatemask = 0;
	struct tc_ctinfo sel = {};
	unsigned short zone = 0;
	char **argv = *argv_p;
	struct rtattr *tail;
	int argc = *argc_p;
	int ok = 0;
	__u8 i;

	while (argc > 0) {
		if (matches(*argv, "ctinfo") == 0) {
			ok = 1;
			NEXT_ARG_FWD();
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
		if (matches(*argv, "dscp") == 0) {
			NEXT_ARG();
			if (get_u32(&dscpmask, *argv, 0)) {
				fprintf(stderr,
					"ctinfo: Illegal dscp \"mask\"\n");
				return -1;
			}
			if (NEXT_ARG_OK()) {
				NEXT_ARG_FWD();
				if (!get_u32(&dscpstatemask, *argv, 0))
					NEXT_ARG_FWD(); /* was a statemask */
			} else {
				NEXT_ARG_FWD();
			}
		}
	}

	/* cpmark has optional mask parameter, so the next arg might not  */
	/* exist, or it might be the next option, or it may actually be a */
	/* 32bit mask */
	if (argc) {
		if (matches(*argv, "cpmark") == 0) {
			cpmarkmask = ~0;
			if (NEXT_ARG_OK()) {
				NEXT_ARG_FWD();
				if (!get_u32(&cpmarkmask, *argv, 0))
					NEXT_ARG_FWD(); /* was a mask */
			} else {
				NEXT_ARG_FWD();
			}
		}
	}

	if (argc) {
		if (matches(*argv, "zone") == 0) {
			NEXT_ARG();
			if (get_u16(&zone, *argv, 10)) {
				fprintf(stderr, "ctinfo: Illegal \"zone\"\n");
				return -1;
			}
			NEXT_ARG_FWD();
		}
	}

	parse_action_control_dflt(&argc, &argv, &sel.action,
				  false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.index, *argv, 10)) {
				fprintf(stderr, "ctinfo: Illegal \"index\"\n");
				return -1;
			}
			NEXT_ARG_FWD();
		}
	}

	if (dscpmask & dscpstatemask) {
		fprintf(stderr,
			"ctinfo: dscp mask & statemask must NOT overlap\n");
		return -1;
	}

	i = ffs(dscpmask);
	if (i && ((~0 & (dscpmask >> (i - 1))) != 0x3f)) {
		fprintf(stderr,
			"ctinfo: dscp mask must be 6 contiguous bits long\n");
		return -1;
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_CTINFO_ACT, &sel, sizeof(sel));
	addattr16(n, MAX_MSG, TCA_CTINFO_ZONE, zone);

	if (dscpmask)
		addattr32(n, MAX_MSG,
			  TCA_CTINFO_PARMS_DSCP_MASK, dscpmask);

	if (dscpstatemask)
		addattr32(n, MAX_MSG,
			  TCA_CTINFO_PARMS_DSCP_STATEMASK, dscpstatemask);

	if (cpmarkmask)
		addattr32(n, MAX_MSG,
			  TCA_CTINFO_PARMS_CPMARK_MASK, cpmarkmask);

	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static void print_ctinfo_stats(FILE *f, struct rtattr *tb[TCA_CTINFO_MAX + 1])
{
	struct tcf_t *tm;

	if (tb[TCA_CTINFO_TM]) {
		tm = RTA_DATA(tb[TCA_CTINFO_TM]);

		print_tm(f, tm);
	}

	if (tb[TCA_CTINFO_STATS_DSCP_SET])
		print_lluint(PRINT_ANY, "dscpset", " DSCP set %llu",
			     rta_getattr_u64(tb[TCA_CTINFO_STATS_DSCP_SET]));
	if (tb[TCA_CTINFO_STATS_DSCP_ERROR])
		print_lluint(PRINT_ANY, "dscperror", " error %llu",
			     rta_getattr_u64(tb[TCA_CTINFO_STATS_DSCP_ERROR]));

	if (tb[TCA_CTINFO_STATS_CPMARK_SET])
		print_lluint(PRINT_ANY, "cpmarkset", " CPMARK set %llu",
			     rta_getattr_u64(tb[TCA_CTINFO_STATS_CPMARK_SET]));
}

static int print_ctinfo(struct action_util *au, FILE *f, struct rtattr *arg)
{
	unsigned int cpmarkmask = ~0, dscpmask = 0, dscpstatemask = 0;
	struct rtattr *tb[TCA_CTINFO_MAX + 1];
	unsigned short zone = 0;
	struct tc_ctinfo *ci;

	print_string(PRINT_ANY, "kind", "%s ", "ctinfo");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_CTINFO_MAX, arg);
	if (!tb[TCA_CTINFO_ACT]) {
		print_string(PRINT_FP, NULL, "%s",
			     "[NULL ctinfo action parameters]");
		return -1;
	}

	ci = RTA_DATA(tb[TCA_CTINFO_ACT]);

	if (tb[TCA_CTINFO_PARMS_DSCP_MASK]) {
		if (RTA_PAYLOAD(tb[TCA_CTINFO_PARMS_DSCP_MASK]) >=
		    sizeof(__u32))
			dscpmask = rta_getattr_u32(
					tb[TCA_CTINFO_PARMS_DSCP_MASK]);
		else
			print_string(PRINT_FP, NULL, "%s",
				     "[invalid dscp mask parameter]");
	}

	if (tb[TCA_CTINFO_PARMS_DSCP_STATEMASK]) {
		if (RTA_PAYLOAD(tb[TCA_CTINFO_PARMS_DSCP_STATEMASK]) >=
		    sizeof(__u32))
			dscpstatemask = rta_getattr_u32(
					tb[TCA_CTINFO_PARMS_DSCP_STATEMASK]);
		else
			print_string(PRINT_FP, NULL, "%s",
				     "[invalid dscp statemask parameter]");
	}

	if (tb[TCA_CTINFO_PARMS_CPMARK_MASK]) {
		if (RTA_PAYLOAD(tb[TCA_CTINFO_PARMS_CPMARK_MASK]) >=
		    sizeof(__u32))
			cpmarkmask = rta_getattr_u32(
					tb[TCA_CTINFO_PARMS_CPMARK_MASK]);
		else
			print_string(PRINT_FP, NULL, "%s",
				     "[invalid cpmark mask parameter]");
	}

	if (tb[TCA_CTINFO_ZONE] && RTA_PAYLOAD(tb[TCA_CTINFO_ZONE]) >=
	    sizeof(__u16))
		zone = rta_getattr_u16(tb[TCA_CTINFO_ZONE]);

	print_hu(PRINT_ANY, "zone", "zone %u", zone);
	print_action_control(f, " ", ci->action, "");

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", ci->index);
	print_int(PRINT_ANY, "ref", " ref %d", ci->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", ci->bindcnt);

	if (tb[TCA_CTINFO_PARMS_DSCP_MASK]) {
		print_0xhex(PRINT_ANY, "dscpmask", " dscp %#010llx", dscpmask);
		print_0xhex(PRINT_ANY, "dscpstatemask", " %#010llx",
			    dscpstatemask);
	}

	if (tb[TCA_CTINFO_PARMS_CPMARK_MASK])
		print_0xhex(PRINT_ANY, "cpmark", " cpmark %#010llx",
			    cpmarkmask);

	if (show_stats)
		print_ctinfo_stats(f, tb);

	print_nl();

	return 0;
}

struct action_util ctinfo_action_util = {
	.id = "ctinfo",
	.parse_aopt = parse_ctinfo,
	.print_aopt = print_ctinfo,
};
