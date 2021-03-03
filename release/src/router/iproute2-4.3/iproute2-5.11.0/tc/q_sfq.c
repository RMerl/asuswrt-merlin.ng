/*
 * q_sfq.c		SFQ.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
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
#include "tc_red.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... sfq	[ limit NUMBER ] [ perturb SECS ] [ quantum BYTES ]\n"
		"		[ divisor NUMBER ] [ flows NUMBER] [ depth NUMBER ]\n"
		"		[ headdrop ]\n"
		"		[ redflowlimit BYTES ] [ min BYTES ] [ max BYTES ]\n"
		"		[ avpkt BYTES ] [ burst PACKETS ] [ probability P ]\n"
		"		[ ecn ] [ harddrop ]\n");
}

static int sfq_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n, const char *dev)
{
	int ok = 0, red = 0;
	struct tc_sfq_qopt_v1 opt = {};
	unsigned int burst = 0;
	int wlog;
	unsigned int avpkt = 1000;
	double probability = 0.02;

	while (argc > 0) {
		if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_size(&opt.v0.quantum, *argv)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "perturb") == 0) {
			NEXT_ARG();
			if (get_integer(&opt.v0.perturb_period, *argv, 0)) {
				fprintf(stderr, "Illegal \"perturb\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.v0.limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
			if (opt.v0.limit < 2) {
				fprintf(stderr, "Illegal \"limit\", must be > 1\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "divisor") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.v0.divisor, *argv, 0)) {
				fprintf(stderr, "Illegal \"divisor\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "flows") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.v0.flows, *argv, 0)) {
				fprintf(stderr, "Illegal \"flows\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "depth") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.depth, *argv, 0)) {
				fprintf(stderr, "Illegal \"flows\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "headdrop") == 0) {
			opt.headdrop = 1;
			ok++;
		} else if (strcmp(*argv, "redflowlimit") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"redflowlimit\"\n");
				return -1;
			}
			red++;
		} else if (strcmp(*argv, "min") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.qth_min, *argv, 0)) {
				fprintf(stderr, "Illegal \"min\"\n");
				return -1;
			}
			red++;
		} else if (strcmp(*argv, "max") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.qth_max, *argv, 0)) {
				fprintf(stderr, "Illegal \"max\"\n");
				return -1;
			}
			red++;
		} else if (strcmp(*argv, "burst") == 0) {
			NEXT_ARG();
			if (get_unsigned(&burst, *argv, 0)) {
				fprintf(stderr, "Illegal \"burst\"\n");
				return -1;
			}
			red++;
		} else if (strcmp(*argv, "avpkt") == 0) {
			NEXT_ARG();
			if (get_size(&avpkt, *argv)) {
				fprintf(stderr, "Illegal \"avpkt\"\n");
				return -1;
			}
			red++;
		} else if (strcmp(*argv, "probability") == 0) {
			NEXT_ARG();
			if (sscanf(*argv, "%lg", &probability) != 1) {
				fprintf(stderr, "Illegal \"probability\"\n");
				return -1;
			}
			red++;
		} else if (strcmp(*argv, "ecn") == 0) {
			opt.flags |= TC_RED_ECN;
			red++;
		} else if (strcmp(*argv, "harddrop") == 0) {
			opt.flags |= TC_RED_HARDDROP;
			red++;
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
	if (red) {
		if (!opt.limit) {
			fprintf(stderr, "Required parameter (redflowlimit) is missing\n");
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
			burst = (2 * opt.qth_min + opt.qth_max) / (3 * avpkt);

		if (opt.qth_max > opt.limit) {
			fprintf(stderr, "\"max\" is larger than \"limit\"\n");
			return -1;
		}

		if (opt.qth_min >= opt.qth_max) {
			fprintf(stderr, "\"min\" is not smaller than \"max\"\n");
			return -1;
		}

		wlog = tc_red_eval_ewma(opt.qth_min, burst, avpkt);
		if (wlog < 0) {
			fprintf(stderr, "SFQ: failed to calculate EWMA constant.\n");
			return -1;
		}
		if (wlog >= 10)
			fprintf(stderr, "SFQ: WARNING. Burst %u seems to be too large.\n", burst);
		opt.Wlog = wlog;

		wlog = tc_red_eval_P(opt.qth_min, opt.qth_max, probability);
		if (wlog < 0) {
			fprintf(stderr, "SFQ: failed to calculate probability.\n");
			return -1;
		}
		opt.Plog = wlog;
		opt.max_P = probability * pow(2, 32);
	}

	if (ok || red)
		addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
	return 0;
}

static int sfq_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct tc_sfq_qopt *qopt;
	struct tc_sfq_qopt_v1 *qopt_ext = NULL;

	if (opt == NULL)
		return 0;

	if (RTA_PAYLOAD(opt)  < sizeof(*qopt))
		return -1;
	if (RTA_PAYLOAD(opt) >= sizeof(*qopt_ext))
		qopt_ext = RTA_DATA(opt);
	qopt = RTA_DATA(opt);

	print_uint(PRINT_ANY, "limit", "limit %up ", qopt->limit);
	print_size(PRINT_ANY, "quantum", "quantum %s ", qopt->quantum);

	if (qopt_ext && qopt_ext->depth)
		print_uint(PRINT_ANY, "depth", "depth %u ", qopt_ext->depth);
	if (qopt_ext && qopt_ext->headdrop)
		print_bool(PRINT_ANY, "headdrop", "headdrop ", true);
	if (show_details)
		print_uint(PRINT_ANY, "flows", "flows %u ", qopt->flows);

	print_uint(PRINT_ANY, "divisor", "divisor %u ", qopt->divisor);

	if (qopt->perturb_period)
		print_int(PRINT_ANY, "perturb", "perturb %dsec ",
			   qopt->perturb_period);
	if (qopt_ext && qopt_ext->qth_min) {
		print_uint(PRINT_ANY, "ewma", "ewma %u ", qopt_ext->Wlog);
		print_size(PRINT_ANY, "min", "min %s ", qopt_ext->qth_min);
		print_size(PRINT_ANY, "max", "max %s ", qopt_ext->qth_max);
		print_float(PRINT_ANY, "probability", "probability %lg ",
			    qopt_ext->max_P / pow(2, 32));
		tc_red_print_flags(qopt_ext->flags);
		if (show_stats) {
			print_nl();
			print_uint(PRINT_ANY, "prob_mark", "  prob_mark %u",
				   qopt_ext->stats.prob_mark);
			print_uint(PRINT_ANY, "prob_mark_head",
				   " prob_mark_head %u",
				   qopt_ext->stats.prob_mark_head);
			print_uint(PRINT_ANY, "prob_drop", " prob_drop %u",
				   qopt_ext->stats.prob_drop);
			print_nl();
			print_uint(PRINT_ANY, "forced_mark",
				   "  forced_mark %u",
				   qopt_ext->stats.forced_mark);
			print_uint(PRINT_ANY, "forced_mark_head",
				   " forced_mark_head %u",
				   qopt_ext->stats.forced_mark_head);
			print_uint(PRINT_ANY, "forced_drop", " forced_drop %u",
				   qopt_ext->stats.forced_drop);
		}
	}
	return 0;
}

static int sfq_print_xstats(struct qdisc_util *qu, FILE *f,
			    struct rtattr *xstats)
{
	struct tc_sfq_xstats *st;

	if (xstats == NULL)
		return 0;
	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;
	st = RTA_DATA(xstats);

	print_int(PRINT_ANY, "allot", "  allot %d", st->allot);

	return 0;
}

struct qdisc_util sfq_qdisc_util = {
	.id		= "sfq",
	.parse_qopt	= sfq_parse_opt,
	.print_qopt	= sfq_print_opt,
	.print_xstats	= sfq_print_xstats,
};
