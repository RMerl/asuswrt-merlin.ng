// SPDX-License-Identifier: GPL-2.0-only
/*
 * Flow Queue PIE
 *
 * Copyright (C) 2019 Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 * Copyright (C) 2019 Sachin D. Patil <sdp.sachin@gmail.com>
 * Copyright (C) 2019 V. Saicharan <vsaicharan1998@gmail.com>
 * Copyright (C) 2019 Mohit Bhasi <mohitbhasi1998@gmail.com>
 * Copyright (C) 2019 Leslie Monis <lesliemonis@gmail.com>
 * Copyright (C) 2019 Gautam Ramakrishnan <gautamramk@gmail.com>
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
		"Usage: ... fq_pie [ limit PACKETS ] [ flows NUMBER ]\n"
		"                  [ target TIME ] [ tupdate TIME ]\n"
		"                  [ alpha NUMBER ] [ beta NUMBER ]\n"
		"                  [ quantum BYTES ] [ memory_limit BYTES ]\n"
		"                  [ ecn_prob PERCENTAGE ] [ [no]ecn ]\n"
		"                  [ [no]bytemode ] [ [no_]dq_rate_estimator ]\n");
}

#define ALPHA_MAX 32
#define BETA_MAX 32

static int fq_pie_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			    struct nlmsghdr *n, const char *dev)
{
	unsigned int limit = 0;
	unsigned int flows = 0;
	unsigned int target = 0;
	unsigned int tupdate = 0;
	unsigned int alpha = 0;
	unsigned int beta = 0;
	unsigned int quantum = 0;
	unsigned int memory_limit = 0;
	unsigned int ecn_prob = 0;
	int ecn = -1;
	int bytemode = -1;
	int dq_rate_estimator = -1;
	struct rtattr *tail;

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_unsigned(&limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "flows") == 0) {
			NEXT_ARG();
			if (get_unsigned(&flows, *argv, 0)) {
				fprintf(stderr, "Illegal \"flows\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "target") == 0) {
			NEXT_ARG();
			if (get_time(&target, *argv)) {
				fprintf(stderr, "Illegal \"target\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "tupdate") == 0) {
			NEXT_ARG();
			if (get_time(&tupdate, *argv)) {
				fprintf(stderr, "Illegal \"tupdate\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "alpha") == 0) {
			NEXT_ARG();
			if (get_unsigned(&alpha, *argv, 0) ||
			    alpha > ALPHA_MAX) {
				fprintf(stderr, "Illegal \"alpha\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "beta") == 0) {
			NEXT_ARG();
			if (get_unsigned(&beta, *argv, 0) ||
			    beta > BETA_MAX) {
				fprintf(stderr, "Illegal \"beta\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_size(&quantum, *argv)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "memory_limit") == 0) {
			NEXT_ARG();
			if (get_size(&memory_limit, *argv)) {
				fprintf(stderr, "Illegal \"memory_limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "ecn_prob") == 0) {
			NEXT_ARG();
			if (get_unsigned(&ecn_prob, *argv, 0) ||
			    ecn_prob >= 100) {
				fprintf(stderr, "Illegal \"ecn_prob\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "ecn") == 0) {
			ecn = 1;
		} else if (strcmp(*argv, "noecn") == 0) {
			ecn = 0;
		} else if (strcmp(*argv, "bytemode") == 0) {
			bytemode = 1;
		} else if (strcmp(*argv, "nobytemode") == 0) {
			bytemode = 0;
		} else if (strcmp(*argv, "dq_rate_estimator") == 0) {
			dq_rate_estimator = 1;
		} else if (strcmp(*argv, "no_dq_rate_estimator") == 0) {
			dq_rate_estimator = 0;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}

		argc--;
		argv++;
	}

	tail = addattr_nest(n, 1024, TCA_OPTIONS | NLA_F_NESTED);
	if (limit)
		addattr_l(n, 1024, TCA_FQ_PIE_LIMIT, &limit, sizeof(limit));
	if (flows)
		addattr_l(n, 1024, TCA_FQ_PIE_FLOWS, &flows, sizeof(flows));
	if (target)
		addattr_l(n, 1024, TCA_FQ_PIE_TARGET, &target, sizeof(target));
	if (tupdate)
		addattr_l(n, 1024, TCA_FQ_PIE_TUPDATE, &tupdate,
			  sizeof(tupdate));
	if (alpha)
		addattr_l(n, 1024, TCA_FQ_PIE_ALPHA, &alpha, sizeof(alpha));
	if (beta)
		addattr_l(n, 1024, TCA_FQ_PIE_BETA, &beta, sizeof(beta));
	if (quantum)
		addattr_l(n, 1024, TCA_FQ_PIE_QUANTUM, &quantum,
			  sizeof(quantum));
	if (memory_limit)
		addattr_l(n, 1024, TCA_FQ_PIE_MEMORY_LIMIT, &memory_limit,
			  sizeof(memory_limit));
	if (ecn_prob)
		addattr_l(n, 1024, TCA_FQ_PIE_ECN_PROB, &ecn_prob,
			  sizeof(ecn_prob));
	if (ecn != -1)
		addattr_l(n, 1024, TCA_FQ_PIE_ECN, &ecn, sizeof(ecn));
	if (bytemode != -1)
		addattr_l(n, 1024, TCA_FQ_PIE_BYTEMODE, &bytemode,
			  sizeof(bytemode));
	if (dq_rate_estimator != -1)
		addattr_l(n, 1024, TCA_FQ_PIE_DQ_RATE_ESTIMATOR,
			  &dq_rate_estimator, sizeof(dq_rate_estimator));
	addattr_nest_end(n, tail);

	return 0;
}

static int fq_pie_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_FQ_PIE_MAX + 1];
	unsigned int limit = 0;
	unsigned int flows = 0;
	unsigned int target = 0;
	unsigned int tupdate = 0;
	unsigned int alpha = 0;
	unsigned int beta = 0;
	unsigned int quantum = 0;
	unsigned int memory_limit = 0;
	unsigned int ecn_prob = 0;
	int ecn = -1;
	int bytemode = -1;
	int dq_rate_estimator = -1;

	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_FQ_PIE_MAX, opt);

	if (tb[TCA_FQ_PIE_LIMIT] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_LIMIT]) >= sizeof(__u32)) {
		limit = rta_getattr_u32(tb[TCA_FQ_PIE_LIMIT]);
		print_uint(PRINT_ANY, "limit", "limit %up ", limit);
	}
	if (tb[TCA_FQ_PIE_FLOWS] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_FLOWS]) >= sizeof(__u32)) {
		flows = rta_getattr_u32(tb[TCA_FQ_PIE_FLOWS]);
		print_uint(PRINT_ANY, "flows", "flows %u ", flows);
	}
	if (tb[TCA_FQ_PIE_TARGET] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_TARGET]) >= sizeof(__u32)) {
		target = rta_getattr_u32(tb[TCA_FQ_PIE_TARGET]);
		print_uint(PRINT_JSON, "target", NULL, target);
		print_string(PRINT_FP, NULL, "target %s ",
			     sprint_time(target, b1));
	}
	if (tb[TCA_FQ_PIE_TUPDATE] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_TUPDATE]) >= sizeof(__u32)) {
		tupdate = rta_getattr_u32(tb[TCA_FQ_PIE_TUPDATE]);
		print_uint(PRINT_JSON, "tupdate", NULL, tupdate);
		print_string(PRINT_FP, NULL, "tupdate %s ",
			     sprint_time(tupdate, b1));
	}
	if (tb[TCA_FQ_PIE_ALPHA] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_ALPHA]) >= sizeof(__u32)) {
		alpha = rta_getattr_u32(tb[TCA_FQ_PIE_ALPHA]);
		print_uint(PRINT_ANY, "alpha", "alpha %u ", alpha);
	}
	if (tb[TCA_FQ_PIE_BETA] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_BETA]) >= sizeof(__u32)) {
		beta = rta_getattr_u32(tb[TCA_FQ_PIE_BETA]);
		print_uint(PRINT_ANY, "beta", "beta %u ", beta);
	}
	if (tb[TCA_FQ_PIE_QUANTUM] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_QUANTUM]) >= sizeof(__u32)) {
		quantum = rta_getattr_u32(tb[TCA_FQ_PIE_QUANTUM]);
		print_size(PRINT_ANY, "quantum", "quantum %s ", quantum);
	}
	if (tb[TCA_FQ_PIE_MEMORY_LIMIT] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_MEMORY_LIMIT]) >= sizeof(__u32)) {
		memory_limit = rta_getattr_u32(tb[TCA_FQ_PIE_MEMORY_LIMIT]);
		print_size(PRINT_ANY, "memory_limit", "memory_limit %s ",
			   memory_limit);
	}
	if (tb[TCA_FQ_PIE_ECN_PROB] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_ECN_PROB]) >= sizeof(__u32)) {
		ecn_prob = rta_getattr_u32(tb[TCA_FQ_PIE_ECN_PROB]);
		print_uint(PRINT_ANY, "ecn_prob", "ecn_prob %u ", ecn_prob);
	}
	if (tb[TCA_FQ_PIE_ECN] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_ECN]) >= sizeof(__u32)) {
		ecn = rta_getattr_u32(tb[TCA_FQ_PIE_ECN]);
		if (ecn)
			print_bool(PRINT_ANY, "ecn", "ecn ", true);
	}
	if (tb[TCA_FQ_PIE_BYTEMODE] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_BYTEMODE]) >= sizeof(__u32)) {
		bytemode = rta_getattr_u32(tb[TCA_FQ_PIE_BYTEMODE]);
		if (bytemode)
			print_bool(PRINT_ANY, "bytemode", "bytemode ", true);
	}
	if (tb[TCA_FQ_PIE_DQ_RATE_ESTIMATOR] &&
	    RTA_PAYLOAD(tb[TCA_FQ_PIE_DQ_RATE_ESTIMATOR]) >= sizeof(__u32)) {
		dq_rate_estimator =
			rta_getattr_u32(tb[TCA_FQ_PIE_DQ_RATE_ESTIMATOR]);
		if (dq_rate_estimator)
			print_bool(PRINT_ANY, "dq_rate_estimator",
				   "dq_rate_estimator ", true);
	}

	return 0;
}

static int fq_pie_print_xstats(struct qdisc_util *qu, FILE *f,
			       struct rtattr *xstats)
{
	struct tc_fq_pie_xstats _st = {}, *st;

	if (xstats == NULL)
		return 0;

	st = RTA_DATA(xstats);
	if (RTA_PAYLOAD(xstats) < sizeof(*st)) {
		memcpy(&_st, st, RTA_PAYLOAD(xstats));
		st = &_st;
	}

	print_uint(PRINT_ANY, "pkts_in", "  pkts_in %u",
		   st->packets_in);
	print_uint(PRINT_ANY, "overlimit", " overlimit %u",
		   st->overlimit);
	print_uint(PRINT_ANY, "overmemory", " overmemory %u",
		   st->overmemory);
	print_uint(PRINT_ANY, "dropped", " dropped %u",
		   st->dropped);
	print_uint(PRINT_ANY, "ecn_mark", " ecn_mark %u",
		   st->ecn_mark);
	print_nl();
	print_uint(PRINT_ANY, "new_flow_count", "  new_flow_count %u",
		   st->new_flow_count);
	print_uint(PRINT_ANY, "new_flows_len", " new_flows_len %u",
		   st->new_flows_len);
	print_uint(PRINT_ANY, "old_flows_len", " old_flows_len %u",
		   st->old_flows_len);
	print_uint(PRINT_ANY, "memory_used", " memory_used %u",
		   st->memory_usage);

	return 0;

}

struct qdisc_util fq_pie_qdisc_util = {
	.id		= "fq_pie",
	.parse_qopt	= fq_pie_parse_opt,
	.print_qopt	= fq_pie_print_opt,
	.print_xstats	= fq_pie_print_xstats,
};
