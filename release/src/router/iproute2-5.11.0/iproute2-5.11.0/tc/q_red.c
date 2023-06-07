/*
 * q_red.c		RED.
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
#include "tc_qevent.h"

#include "tc_red.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... red	limit BYTES [min BYTES] [max BYTES] avpkt BYTES [burst PACKETS]\n"
		"		[adaptive] [probability PROBABILITY] [bandwidth KBPS]\n"
		"		[ecn] [harddrop] [nodrop]\n"
		"		[qevent early_drop block IDX] [qevent mark block IDX]\n");
}

#define RED_SUPPORTED_FLAGS (TC_RED_HISTORIC_FLAGS | TC_RED_NODROP)

static struct qevent_plain qe_early_drop = {};
static struct qevent_plain qe_mark = {};
static struct qevent_util qevents[] = {
	QEVENT("early_drop", plain, &qe_early_drop, TCA_RED_EARLY_DROP_BLOCK),
	QEVENT("mark", plain, &qe_mark, TCA_RED_MARK_BLOCK),
	{},
};

static int red_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			 struct nlmsghdr *n, const char *dev)
{
	struct nla_bitfield32 flags_bf = {
		.selector = RED_SUPPORTED_FLAGS,
	};
	struct tc_red_qopt opt = {};
	unsigned int burst = 0;
	unsigned int avpkt = 0;
	double probability = 0.02;
	unsigned int rate = 0;
	int parm;
	__u8 sbuf[256];
	__u32 max_P;
	struct rtattr *tail;

	qevents_init(qevents);

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&opt.limit, *argv)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "min") == 0) {
			NEXT_ARG();
			if (get_size(&opt.qth_min, *argv)) {
				fprintf(stderr, "Illegal \"min\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "max") == 0) {
			NEXT_ARG();
			if (get_size(&opt.qth_max, *argv)) {
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
			flags_bf.value |= TC_RED_ECN;
		} else if (strcmp(*argv, "harddrop") == 0) {
			flags_bf.value |= TC_RED_HARDDROP;
		} else if (strcmp(*argv, "nodrop") == 0) {
			flags_bf.value |= TC_RED_NODROP;
		} else if (strcmp(*argv, "adaptative") == 0) {
			flags_bf.value |= TC_RED_ADAPTATIVE;
		} else if (strcmp(*argv, "adaptive") == 0) {
			flags_bf.value |= TC_RED_ADAPTATIVE;
		} else if (matches(*argv, "qevent") == 0) {
			NEXT_ARG();
			if (qevent_parse(qevents, &argc, &argv))
				return -1;
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

	if (!opt.limit || !avpkt) {
		fprintf(stderr, "RED: Required parameter (limit, avpkt) is missing\n");
		return -1;
	}
	/* Compute default min/max thresholds based on
	 * Sally Floyd's recommendations:
	 * http://www.icir.org/floyd/REDparameters.txt
	 */
	if (!opt.qth_max)
		opt.qth_max = opt.qth_min ? opt.qth_min * 3 : opt.limit / 4;
	if (!opt.qth_min)
		opt.qth_min = opt.qth_max / 3;
	if (!burst)
		burst = (2 * opt.qth_min + opt.qth_max) / (3 * avpkt);
	if (!rate) {
		get_rate(&rate, "10Mbit");
		fprintf(stderr, "RED: set bandwidth to 10Mbit\n");
	}
	if ((parm = tc_red_eval_ewma(opt.qth_min, burst, avpkt)) < 0) {
		fprintf(stderr, "RED: failed to calculate EWMA constant.\n");
		return -1;
	}
	if (parm >= 10)
		fprintf(stderr, "RED: WARNING. Burst %u seems to be too large.\n", burst);
	opt.Wlog = parm;
	if ((parm = tc_red_eval_P(opt.qth_min, opt.qth_max, probability)) < 0) {
		fprintf(stderr, "RED: failed to calculate probability.\n");
		return -1;
	}
	opt.Plog = parm;
	if ((parm = tc_red_eval_idle_damping(opt.Wlog, avpkt, rate, sbuf)) < 0) {
		fprintf(stderr, "RED: failed to calculate idle damping table.\n");
		return -1;
	}
	opt.Scell_log = parm;

	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	addattr_l(n, 1024, TCA_RED_PARMS, &opt, sizeof(opt));
	addattr_l(n, 1024, TCA_RED_STAB, sbuf, 256);
	max_P = probability * pow(2, 32);
	addattr_l(n, 1024, TCA_RED_MAX_P, &max_P, sizeof(max_P));
	addattr_l(n, 1024, TCA_RED_FLAGS, &flags_bf, sizeof(flags_bf));
	if (qevents_dump(qevents, n))
		return -1;
	addattr_nest_end(n, tail);
	return 0;
}

static int red_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_RED_MAX + 1];
	struct nla_bitfield32 *flags_bf;
	struct tc_red_qopt *qopt;
	__u32 max_P = 0;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_RED_MAX, opt);

	if (tb[TCA_RED_PARMS] == NULL)
		return -1;
	qopt = RTA_DATA(tb[TCA_RED_PARMS]);
	if (RTA_PAYLOAD(tb[TCA_RED_PARMS])  < sizeof(*qopt))
		return -1;

	if (tb[TCA_RED_MAX_P] &&
	    RTA_PAYLOAD(tb[TCA_RED_MAX_P]) >= sizeof(__u32))
		max_P = rta_getattr_u32(tb[TCA_RED_MAX_P]);

	if (tb[TCA_RED_FLAGS] &&
	    RTA_PAYLOAD(tb[TCA_RED_FLAGS]) >= sizeof(*flags_bf)) {
		flags_bf = RTA_DATA(tb[TCA_RED_FLAGS]);
		qopt->flags = flags_bf->value;
	}

	print_size(PRINT_ANY, "limit", "limit %s ", qopt->limit);
	print_size(PRINT_ANY, "min", "min %s ", qopt->qth_min);
	print_size(PRINT_ANY, "max", "max %s ", qopt->qth_max);

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

	qevents_init(qevents);
	if (qevents_read(qevents, tb))
		return -1;
	qevents_print(qevents, f);
	return 0;
}

static int red_print_xstats(struct qdisc_util *qu, FILE *f, struct rtattr *xstats)
{
#ifdef TC_RED_ECN
	struct tc_red_xstats *st;

	if (xstats == NULL)
		return 0;

	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;

	st = RTA_DATA(xstats);
	print_uint(PRINT_ANY, "marked", "  marked %u ", st->marked);
	print_uint(PRINT_ANY, "early", "early %u ", st->early);
	print_uint(PRINT_ANY, "pdrop", "pdrop %u ", st->pdrop);
	print_uint(PRINT_ANY, "other", "other %u ", st->other);
#endif
	return 0;
}

static int red_has_block(struct qdisc_util *qu, struct rtattr *opt, __u32 block_idx, bool *p_has)
{
	struct rtattr *tb[TCA_RED_MAX + 1];

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_RED_MAX, opt);

	qevents_init(qevents);
	if (qevents_read(qevents, tb))
		return -1;

	*p_has = qevents_have_block(qevents, block_idx);
	return 0;
}

struct qdisc_util red_qdisc_util = {
	.id		= "red",
	.parse_qopt	= red_parse_opt,
	.print_qopt	= red_print_opt,
	.print_xstats	= red_print_xstats,
	.has_block	= red_has_block,
};
