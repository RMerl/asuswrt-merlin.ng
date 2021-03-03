/*
 * f_matchall.c		Match-all Classifier
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Jiri Pirko <jiri@mellanox.com>, Yotam Gigi <yotamg@mellanox.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/if.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... matchall [skip_sw | skip_hw]\n"
		"                 [ action ACTION_SPEC ] [ classid CLASSID ]\n"
		"\n"
		"Where: SELECTOR := SAMPLE SAMPLE ...\n"
		"       FILTERID := X:Y:Z\n"
		"       ACTION_SPEC := ... look at individual actions\n"
		"\n"
		"NOTE: CLASSID is parsed as hexadecimal input.\n");
}

static int matchall_parse_opt(struct filter_util *qu, char *handle,
			   int argc, char **argv, struct nlmsghdr *n)
{
	struct tcmsg *t = NLMSG_DATA(n);
	struct rtattr *tail;
	__u32 flags = 0;
	long h = 0;

	if (handle) {
		h = strtol(handle, NULL, 0);
		if (h == LONG_MIN || h == LONG_MAX) {
			fprintf(stderr, "Illegal handle \"%s\", must be numeric.\n",
			    handle);
			return -1;
		}
	}
	t->tcm_handle = h;

	if (argc == 0)
		return 0;

	tail = (struct rtattr *)(((void *)n)+NLMSG_ALIGN(n->nlmsg_len));
	addattr_l(n, MAX_MSG, TCA_OPTIONS, NULL, 0);

	while (argc > 0) {
		if (matches(*argv, "classid") == 0 ||
			   strcmp(*argv, "flowid") == 0) {
			unsigned int handle;

			NEXT_ARG();
			if (get_tc_classid(&handle, *argv)) {
				fprintf(stderr, "Illegal \"classid\"\n");
				return -1;
			}
			addattr_l(n, MAX_MSG, TCA_MATCHALL_CLASSID, &handle, 4);
		} else if (matches(*argv, "action") == 0) {
			NEXT_ARG();
			if (parse_action(&argc, &argv, TCA_MATCHALL_ACT, n)) {
				fprintf(stderr, "Illegal \"action\"\n");
				return -1;
			}
			continue;

		} else if (strcmp(*argv, "skip_hw") == 0) {
			NEXT_ARG();
			flags |= TCA_CLS_FLAGS_SKIP_HW;
			continue;
		} else if (strcmp(*argv, "skip_sw") == 0) {
			NEXT_ARG();
			flags |= TCA_CLS_FLAGS_SKIP_SW;
			continue;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (flags) {
		if (!(flags ^ (TCA_CLS_FLAGS_SKIP_HW |
			       TCA_CLS_FLAGS_SKIP_SW))) {
			fprintf(stderr,
				"skip_hw and skip_sw are mutually exclusive\n");
			return -1;
		}
		addattr_l(n, MAX_MSG, TCA_MATCHALL_FLAGS, &flags, 4);
	}

	tail->rta_len = (((void *)n)+n->nlmsg_len) - (void *)tail;
	return 0;
}

static int matchall_print_opt(struct filter_util *qu, FILE *f,
			   struct rtattr *opt, __u32 handle)
{
	struct rtattr *tb[TCA_MATCHALL_MAX+1];
	struct tc_matchall_pcnt *pf = NULL;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_MATCHALL_MAX, opt);

	if (handle)
		print_uint(PRINT_ANY, "handle", "handle 0x%x ", handle);

	if (tb[TCA_MATCHALL_CLASSID]) {
		SPRINT_BUF(b1);
		print_string(PRINT_ANY, "flowid", "flowid %s ",
			sprint_tc_classid(rta_getattr_u32(tb[TCA_MATCHALL_CLASSID]), b1));
	}

	if (tb[TCA_MATCHALL_FLAGS]) {
		__u32 flags = rta_getattr_u32(tb[TCA_MATCHALL_FLAGS]);

		if (flags & TCA_CLS_FLAGS_SKIP_HW)
			print_bool(PRINT_ANY, "skip_hw", "\n  skip_hw", true);
		if (flags & TCA_CLS_FLAGS_SKIP_SW)
			print_bool(PRINT_ANY, "skip_sw", "\n  skip_sw", true);

		if (flags & TCA_CLS_FLAGS_IN_HW)
			print_bool(PRINT_ANY, "in_hw", "\n  in_hw", true);
		else if (flags & TCA_CLS_FLAGS_NOT_IN_HW)
			print_bool(PRINT_ANY, "not_in_hw", "\n  not_in_hw", true);
	}

	if (tb[TCA_MATCHALL_PCNT]) {
		if (RTA_PAYLOAD(tb[TCA_MATCHALL_PCNT])  < sizeof(*pf)) {
			print_string(PRINT_FP, NULL, "Broken perf counters\n", NULL);
			return -1;
		}
		pf = RTA_DATA(tb[TCA_MATCHALL_PCNT]);
	}

	if (show_stats && NULL != pf)
		print_u64(PRINT_ANY, "rule_hit", " (rule hit %llu)",
			(unsigned long long) pf->rhit);


	if (tb[TCA_MATCHALL_ACT])
		tc_print_action(f, tb[TCA_MATCHALL_ACT], 0);

	return 0;
}

struct filter_util matchall_filter_util = {
	.id = "matchall",
	.parse_fopt = matchall_parse_opt,
	.print_fopt = matchall_print_opt,
};
