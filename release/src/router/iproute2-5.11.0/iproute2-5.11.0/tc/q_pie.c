/* Copyright (C) 2013 Cisco Systems, Inc, 2013.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Vijay Subramanian <vijaynsu@cisco.com>
 * Author: Mythili Prabhu <mysuryan@cisco.com>
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
#include <math.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... pie [ limit PACKETS ] [ target TIME ]\n"
		"               [ tupdate TIME ] [ alpha ALPHA ] [ beta BETA ]\n"
		"               [ bytemode | nobytemode ] [ ecn | noecn ]\n"
		"               [ dq_rate_estimator | no_dq_rate_estimator ]\n");
}

#define ALPHA_MAX 32
#define BETA_MAX 32

static int pie_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			 struct nlmsghdr *n, const char *dev)
{
	unsigned int limit   = 0;
	unsigned int target  = 0;
	unsigned int tupdate = 0;
	unsigned int alpha   = 0;
	unsigned int beta    = 0;
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
			    (alpha > ALPHA_MAX)) {
				fprintf(stderr, "Illegal \"alpha\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "beta") == 0) {
			NEXT_ARG();
			if (get_unsigned(&beta, *argv, 0) ||
			    (beta > BETA_MAX)) {
				fprintf(stderr, "Illegal \"beta\"\n");
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

	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	if (limit)
		addattr_l(n, 1024, TCA_PIE_LIMIT, &limit, sizeof(limit));
	if (tupdate)
		addattr_l(n, 1024, TCA_PIE_TUPDATE, &tupdate, sizeof(tupdate));
	if (target)
		addattr_l(n, 1024, TCA_PIE_TARGET, &target, sizeof(target));
	if (alpha)
		addattr_l(n, 1024, TCA_PIE_ALPHA, &alpha, sizeof(alpha));
	if (beta)
		addattr_l(n, 1024, TCA_PIE_BETA, &beta, sizeof(beta));
	if (ecn != -1)
		addattr_l(n, 1024, TCA_PIE_ECN, &ecn, sizeof(ecn));
	if (bytemode != -1)
		addattr_l(n, 1024, TCA_PIE_BYTEMODE, &bytemode,
			  sizeof(bytemode));
	if (dq_rate_estimator != -1)
		addattr_l(n, 1024, TCA_PIE_DQ_RATE_ESTIMATOR,
			  &dq_rate_estimator, sizeof(dq_rate_estimator));

	addattr_nest_end(n, tail);
	return 0;
}

static int pie_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_PIE_MAX + 1];
	unsigned int limit;
	unsigned int tupdate;
	unsigned int target;
	unsigned int alpha;
	unsigned int beta;
	unsigned int ecn;
	unsigned int bytemode;
	unsigned int dq_rate_estimator;

	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_PIE_MAX, opt);

	if (tb[TCA_PIE_LIMIT] &&
	    RTA_PAYLOAD(tb[TCA_PIE_LIMIT]) >= sizeof(__u32)) {
		limit = rta_getattr_u32(tb[TCA_PIE_LIMIT]);
		print_uint(PRINT_ANY, "limit", "limit %up ", limit);
	}
	if (tb[TCA_PIE_TARGET] &&
	    RTA_PAYLOAD(tb[TCA_PIE_TARGET]) >= sizeof(__u32)) {
		target = rta_getattr_u32(tb[TCA_PIE_TARGET]);
		print_uint(PRINT_JSON, "target", NULL, target);
		print_string(PRINT_FP, NULL, "target %s ",
			     sprint_time(target, b1));
	}
	if (tb[TCA_PIE_TUPDATE] &&
	    RTA_PAYLOAD(tb[TCA_PIE_TUPDATE]) >= sizeof(__u32)) {
		tupdate = rta_getattr_u32(tb[TCA_PIE_TUPDATE]);
		print_uint(PRINT_JSON, "tupdate", NULL, tupdate);
		print_string(PRINT_FP, NULL, "tupdate %s ",
			     sprint_time(tupdate, b1));
	}
	if (tb[TCA_PIE_ALPHA] &&
	    RTA_PAYLOAD(tb[TCA_PIE_ALPHA]) >= sizeof(__u32)) {
		alpha = rta_getattr_u32(tb[TCA_PIE_ALPHA]);
		print_uint(PRINT_ANY, "alpha", "alpha %u ", alpha);
	}
	if (tb[TCA_PIE_BETA] &&
	    RTA_PAYLOAD(tb[TCA_PIE_BETA]) >= sizeof(__u32)) {
		beta = rta_getattr_u32(tb[TCA_PIE_BETA]);
		print_uint(PRINT_ANY, "beta", "beta %u ", beta);
	}

	if (tb[TCA_PIE_ECN] && RTA_PAYLOAD(tb[TCA_PIE_ECN]) >= sizeof(__u32)) {
		ecn = rta_getattr_u32(tb[TCA_PIE_ECN]);
		if (ecn)
			print_bool(PRINT_ANY, "ecn", "ecn ", true);
	}

	if (tb[TCA_PIE_BYTEMODE] &&
	    RTA_PAYLOAD(tb[TCA_PIE_BYTEMODE]) >= sizeof(__u32)) {
		bytemode = rta_getattr_u32(tb[TCA_PIE_BYTEMODE]);
		if (bytemode)
			print_bool(PRINT_ANY, "bytemode", "bytemode ", true);
	}

	if (tb[TCA_PIE_DQ_RATE_ESTIMATOR] &&
	    RTA_PAYLOAD(tb[TCA_PIE_DQ_RATE_ESTIMATOR]) >= sizeof(__u32)) {
		dq_rate_estimator =
				rta_getattr_u32(tb[TCA_PIE_DQ_RATE_ESTIMATOR]);
		if (dq_rate_estimator)
			print_bool(PRINT_ANY, "dq_rate_estimator",
				   "dq_rate_estimator ", true);
	}

	return 0;
}

static int pie_print_xstats(struct qdisc_util *qu, FILE *f,
			    struct rtattr *xstats)
{
	struct tc_pie_xstats *st;

	SPRINT_BUF(b1);

	if (xstats == NULL)
		return 0;

	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;

	st = RTA_DATA(xstats);

	/* prob is returned as a fracion of maximum integer value */
	print_float(PRINT_ANY, "prob", "  prob %lg",
		    (double)st->prob / (double)UINT64_MAX);
	print_uint(PRINT_JSON, "delay", NULL, st->delay);
	print_string(PRINT_FP, NULL, " delay %s", sprint_time(st->delay, b1));

	if (st->dq_rate_estimating)
		print_uint(PRINT_ANY, "avg_dq_rate", " avg_dq_rate %u",
			   st->avg_dq_rate);

	print_nl();
	print_uint(PRINT_ANY, "pkts_in", "  pkts_in %u", st->packets_in);
	print_uint(PRINT_ANY, "overlimit", " overlimit %u", st->overlimit);
	print_uint(PRINT_ANY, "dropped", " dropped %u", st->dropped);
	print_uint(PRINT_ANY, "maxq", " maxq %u", st->maxq);
	print_uint(PRINT_ANY, "ecn_mark", " ecn_mark %u", st->ecn_mark);

	return 0;

}

struct qdisc_util pie_qdisc_util = {
	.id = "pie",
	.parse_qopt	= pie_parse_opt,
	.print_qopt	= pie_print_opt,
	.print_xstats	= pie_print_xstats,
};
