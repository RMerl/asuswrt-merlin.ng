/* SPDX-License-Identifier: GPL-2.0 */
/* q_hhf.c		Heavy-Hitter Filter (HHF)
 *
 * Copyright (C) 2013 Terry Lam <vtlam@google.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... hhf	[ limit PACKETS ] [ quantum BYTES]\n"
		"		[ hh_limit NUMBER ]\n"
		"		[ reset_timeout TIME ]\n"
		"		[ admit_bytes BYTES ]\n"
		"		[ evict_timeout TIME ]\n"
		"		[ non_hh_weight NUMBER ]\n");
}

static int hhf_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			 struct nlmsghdr *n, const char *dev)
{
	unsigned int limit = 0;
	unsigned int quantum = 0;
	unsigned int hh_limit = 0;
	unsigned int reset_timeout = 0;
	unsigned int admit_bytes = 0;
	unsigned int evict_timeout = 0;
	unsigned int non_hh_weight = 0;
	struct rtattr *tail;

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_unsigned(&limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_unsigned(&quantum, *argv, 0)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "hh_limit") == 0) {
			NEXT_ARG();
			if (get_unsigned(&hh_limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"hh_limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "reset_timeout") == 0) {
			NEXT_ARG();
			if (get_time(&reset_timeout, *argv)) {
				fprintf(stderr, "Illegal \"reset_timeout\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "admit_bytes") == 0) {
			NEXT_ARG();
			if (get_unsigned(&admit_bytes, *argv, 0)) {
				fprintf(stderr, "Illegal \"admit_bytes\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "evict_timeout") == 0) {
			NEXT_ARG();
			if (get_time(&evict_timeout, *argv)) {
				fprintf(stderr, "Illegal \"evict_timeout\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "non_hh_weight") == 0) {
			NEXT_ARG();
			if (get_unsigned(&non_hh_weight, *argv, 0)) {
				fprintf(stderr, "Illegal \"non_hh_weight\"\n");
				return -1;
			}
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

	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	if (limit)
		addattr_l(n, 1024, TCA_HHF_BACKLOG_LIMIT, &limit,
			  sizeof(limit));
	if (quantum)
		addattr_l(n, 1024, TCA_HHF_QUANTUM, &quantum, sizeof(quantum));
	if (hh_limit)
		addattr_l(n, 1024, TCA_HHF_HH_FLOWS_LIMIT, &hh_limit,
			  sizeof(hh_limit));
	if (reset_timeout)
		addattr_l(n, 1024, TCA_HHF_RESET_TIMEOUT, &reset_timeout,
			  sizeof(reset_timeout));
	if (admit_bytes)
		addattr_l(n, 1024, TCA_HHF_ADMIT_BYTES, &admit_bytes,
			  sizeof(admit_bytes));
	if (evict_timeout)
		addattr_l(n, 1024, TCA_HHF_EVICT_TIMEOUT, &evict_timeout,
			  sizeof(evict_timeout));
	if (non_hh_weight)
		addattr_l(n, 1024, TCA_HHF_NON_HH_WEIGHT, &non_hh_weight,
			  sizeof(non_hh_weight));
	addattr_nest_end(n, tail);
	return 0;
}

static int hhf_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_HHF_MAX + 1];
	unsigned int limit;
	unsigned int quantum;
	unsigned int hh_limit;
	unsigned int reset_timeout;
	unsigned int admit_bytes;
	unsigned int evict_timeout;
	unsigned int non_hh_weight;

	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_HHF_MAX, opt);

	if (tb[TCA_HHF_BACKLOG_LIMIT] &&
	    RTA_PAYLOAD(tb[TCA_HHF_BACKLOG_LIMIT]) >= sizeof(__u32)) {
		limit = rta_getattr_u32(tb[TCA_HHF_BACKLOG_LIMIT]);
		print_uint(PRINT_ANY, "limit", "limit %up ", limit);
	}
	if (tb[TCA_HHF_QUANTUM] &&
	    RTA_PAYLOAD(tb[TCA_HHF_QUANTUM]) >= sizeof(__u32)) {
		quantum = rta_getattr_u32(tb[TCA_HHF_QUANTUM]);
		print_size(PRINT_ANY, "quantum", "quantum %s ", quantum);
	}
	if (tb[TCA_HHF_HH_FLOWS_LIMIT] &&
	    RTA_PAYLOAD(tb[TCA_HHF_HH_FLOWS_LIMIT]) >= sizeof(__u32)) {
		hh_limit = rta_getattr_u32(tb[TCA_HHF_HH_FLOWS_LIMIT]);
		print_uint(PRINT_ANY, "hh_limit", "hh_limit %u ", hh_limit);
	}
	if (tb[TCA_HHF_RESET_TIMEOUT] &&
	    RTA_PAYLOAD(tb[TCA_HHF_RESET_TIMEOUT]) >= sizeof(__u32)) {
		reset_timeout = rta_getattr_u32(tb[TCA_HHF_RESET_TIMEOUT]);
		print_uint(PRINT_JSON, "reset_timeout", NULL, reset_timeout);
		print_string(PRINT_FP, NULL, "reset_timeout %s ",
			     sprint_time(reset_timeout, b1));
	}
	if (tb[TCA_HHF_ADMIT_BYTES] &&
	    RTA_PAYLOAD(tb[TCA_HHF_ADMIT_BYTES]) >= sizeof(__u32)) {
		admit_bytes = rta_getattr_u32(tb[TCA_HHF_ADMIT_BYTES]);
		print_size(PRINT_ANY, "admit_bytes", "admit_bytes %s ",
			   admit_bytes);
	}
	if (tb[TCA_HHF_EVICT_TIMEOUT] &&
	    RTA_PAYLOAD(tb[TCA_HHF_EVICT_TIMEOUT]) >= sizeof(__u32)) {
		evict_timeout = rta_getattr_u32(tb[TCA_HHF_EVICT_TIMEOUT]);
		print_uint(PRINT_JSON, "evict_timeout", NULL, evict_timeout);
		print_string(PRINT_FP, NULL, "evict_timeout %s ",
			     sprint_time(evict_timeout, b1));
	}
	if (tb[TCA_HHF_NON_HH_WEIGHT] &&
	    RTA_PAYLOAD(tb[TCA_HHF_NON_HH_WEIGHT]) >= sizeof(__u32)) {
		non_hh_weight = rta_getattr_u32(tb[TCA_HHF_NON_HH_WEIGHT]);
		print_uint(PRINT_ANY, "non_hh_weight", "non_hh_weight %u ",
			   non_hh_weight);
	}
	return 0;
}

static int hhf_print_xstats(struct qdisc_util *qu, FILE *f,
			    struct rtattr *xstats)
{
	struct tc_hhf_xstats *st;

	if (xstats == NULL)
		return 0;

	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;

	st = RTA_DATA(xstats);

	print_uint(PRINT_ANY, "drop_overlimit", "  drop_overlimit %u",
		   st->drop_overlimit);
	print_uint(PRINT_ANY, "hh_overlimit", " hh_overlimit %u",
		   st->hh_overlimit);
	print_uint(PRINT_ANY, "tot_hh", " tot_hh %u", st->hh_tot_count);
	print_uint(PRINT_ANY, "cur_hh", " cur_hh %u", st->hh_cur_count);

	return 0;
}

struct qdisc_util hhf_qdisc_util = {
	.id		= "hhf",
	.parse_qopt	= hhf_parse_opt,
	.print_qopt	= hhf_print_opt,
	.print_xstats	= hhf_print_xstats,
};
