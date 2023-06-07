/*
 * q_choke.c		CHOKE.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Stephen Hemminger <shemminger@vyatta.com>
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

#include "tc_red.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... choke limit PACKETS bandwidth KBPS [ecn]\n"
		"		 [ min PACKETS ] [ max PACKETS ] [ burst PACKETS ]\n");
}

static int choke_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			   struct nlmsghdr *n, const char *dev)
{
	struct tc_red_qopt opt = {};
	unsigned int burst = 0;
	unsigned int avpkt = 1000;
	double probability = 0.02;
	unsigned int rate = 0;
	int ecn_ok = 0;
	int wlog;
	__u8 sbuf[256];
	__u32 max_P;
	struct rtattr *tail;

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_unsigned(&opt.limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "bandwidth") == 0) {
			NEXT_ARG();
			if (strchr(*argv, '%')) {
				if (get_percent_rate(&rate, *argv, dev)) {
					fprintf(stderr, "Illegal \"bandwidth\"\n");
					return -1;
				}
			} else if (get_rate(&rate, *argv)) {
				fprintf(stderr, "Illegal \"bandwidth\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "ecn") == 0) {
			ecn_ok = 1;
		} else if (strcmp(*argv, "min") == 0) {
			NEXT_ARG();
			if (get_unsigned(&opt.qth_min, *argv, 0)) {
				fprintf(stderr, "Illegal \"min\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "max") == 0) {
			NEXT_ARG();
			if (get_unsigned(&opt.qth_max, *argv, 0)) {
				fprintf(stderr, "Illegal \"max\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "burst") == 0) {
			NEXT_ARG();
			if (get_unsigned(&burst, *argv, 0)) {
				fprintf(stderr, "Illegal \"burst\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "avpkt") == 0) {
			NEXT_ARG();
			if (get_size(&avpkt, *argv)) {
				fprintf(stderr, "Illegal \"avpkt\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "probability") == 0) {
			NEXT_ARG();
			if (sscanf(*argv, "%lg", &probability) != 1) {
				fprintf(stderr, "Illegal \"probability\"\n");
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

	if (!rate || !opt.limit) {
		fprintf(stderr, "Required parameter (bandwidth, limit) is missing\n");
		return -1;
	}

	/* Compute default min/max thresholds based on
	   Sally Floyd's recommendations:
	   http://www.icir.org/floyd/REDparameters.txt
	*/
	if (!opt.qth_max)
		opt.qth_max = opt.limit / 4;
	if (!opt.qth_min)
		opt.qth_min = opt.qth_max / 3;
	if (!burst)
		burst = (2 * opt.qth_min + opt.qth_max) / 3;

	if (opt.qth_max > opt.limit) {
		fprintf(stderr, "\"max\" is larger than \"limit\"\n");
		return -1;
	}

	if (opt.qth_min >= opt.qth_max) {
		fprintf(stderr, "\"min\" is not smaller than \"max\"\n");
		return -1;
	}

	wlog = tc_red_eval_ewma(opt.qth_min*avpkt, burst, avpkt);
	if (wlog < 0) {
		fprintf(stderr, "CHOKE: failed to calculate EWMA constant.\n");
		return -1;
	}
	if (wlog >= 10)
		fprintf(stderr, "CHOKE: WARNING. Burst %d seems to be too large.\n", burst);
	opt.Wlog = wlog;

	wlog = tc_red_eval_P(opt.qth_min*avpkt, opt.qth_max*avpkt, probability);
	if (wlog < 0) {
		fprintf(stderr, "CHOKE: failed to calculate probability.\n");
		return -1;
	}
	opt.Plog = wlog;

	wlog = tc_red_eval_idle_damping(opt.Wlog, avpkt, rate, sbuf);
	if (wlog < 0) {
		fprintf(stderr, "CHOKE: failed to calculate idle damping table.\n");
		return -1;
	}
	opt.Scell_log = wlog;
	if (ecn_ok)
		opt.flags |= TC_RED_ECN;

	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	addattr_l(n, 1024, TCA_CHOKE_PARMS, &opt, sizeof(opt));
	addattr_l(n, 1024, TCA_CHOKE_STAB, sbuf, 256);
	max_P = probability * pow(2, 32);
	addattr_l(n, 1024, TCA_CHOKE_MAX_P, &max_P, sizeof(max_P));
	addattr_nest_end(n, tail);
	return 0;
}

static int choke_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_CHOKE_MAX+1];
	const struct tc_red_qopt *qopt;
	__u32 max_P = 0;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_CHOKE_MAX, opt);

	if (tb[TCA_CHOKE_PARMS] == NULL)
		return -1;
	qopt = RTA_DATA(tb[TCA_CHOKE_PARMS]);
	if (RTA_PAYLOAD(tb[TCA_CHOKE_PARMS])  < sizeof(*qopt))
		return -1;
	if (tb[TCA_CHOKE_MAX_P] &&
	    RTA_PAYLOAD(tb[TCA_CHOKE_MAX_P]) >= sizeof(__u32))
		max_P = rta_getattr_u32(tb[TCA_CHOKE_MAX_P]);

	print_uint(PRINT_ANY, "limit", "limit %up ", qopt->limit);
	print_uint(PRINT_ANY, "min", "min %up ", qopt->qth_min);
	print_uint(PRINT_ANY, "max", "max %up ", qopt->qth_max);

	tc_red_print_flags(qopt->flags);

	if (show_details) {
		print_uint(PRINT_ANY, "ewma", "ewma %u ", qopt->Wlog);

		if (max_P)
			print_float(PRINT_ANY, "probability",
				    "probability %lg ", max_P / pow(2, 32));
		else
			print_uint(PRINT_ANY, "Plog", "Plog %u ", qopt->Plog);

		print_uint(PRINT_ANY, "Scell_log", "Scell_log %u",
			   qopt->Scell_log);
	}
	return 0;
}

static int choke_print_xstats(struct qdisc_util *qu, FILE *f,
			      struct rtattr *xstats)
{
	struct tc_choke_xstats *st;

	if (xstats == NULL)
		return 0;

	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;

	st = RTA_DATA(xstats);

	print_uint(PRINT_ANY, "marked", "  marked %u", st->marked);
	print_uint(PRINT_ANY, "early", " early %u", st->early);
	print_uint(PRINT_ANY, "pdrop", " pdrop %u", st->pdrop);
	print_uint(PRINT_ANY, "other", " other %u", st->other);
	print_uint(PRINT_ANY, "matched", " matched %u", st->matched);

	return 0;

}

struct qdisc_util choke_qdisc_util = {
	.id		= "choke",
	.parse_qopt	= choke_parse_opt,
	.print_qopt	= choke_print_opt,
	.print_xstats	= choke_print_xstats,
};
