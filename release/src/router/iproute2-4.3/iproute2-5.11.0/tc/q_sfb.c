/*
 * q_sfb.c	Stochastic Fair Blue.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Juliusz Chroboczek <jch@pps.jussieu.fr>
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

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... sfb [ rehash SECS ] [ db SECS ]\n"
		"	    [ limit PACKETS ] [ max PACKETS ] [ target PACKETS ]\n"
		"	    [ increment FLOAT ] [ decrement FLOAT ]\n"
		"	    [ penalty_rate PPS ] [ penalty_burst PACKETS ]\n");
}

static int get_prob(__u32 *val, const char *arg)
{
	double d;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	d = strtod(arg, &ptr);
	if (!ptr || ptr == arg || d < 0.0 || d > 1.0)
		return -1;
	*val = (__u32)(d * SFB_MAX_PROB + 0.5);
	return 0;
}

static int sfb_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			 struct nlmsghdr *n, const char *dev)
{
	struct tc_sfb_qopt opt = {
		.rehash_interval = 600*1000,
		.warmup_time = 60*1000,
		.penalty_rate = 10,
		.penalty_burst = 20,
		.increment = (SFB_MAX_PROB + 1000) / 2000,
		.decrement = (SFB_MAX_PROB + 10000) / 20000,
	};
	struct rtattr *tail;

	while (argc > 0) {
	    if (strcmp(*argv, "rehash") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.rehash_interval, *argv, 0)) {
				fprintf(stderr, "Illegal \"rehash\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "db") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.warmup_time, *argv, 0)) {
				fprintf(stderr, "Illegal \"db\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.limit, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "max") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.max, *argv, 0)) {
				fprintf(stderr, "Illegal \"max\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "target") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.bin_size, *argv, 0)) {
				fprintf(stderr, "Illegal \"target\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "increment") == 0) {
			NEXT_ARG();
			if (get_prob(&opt.increment, *argv)) {
				fprintf(stderr, "Illegal \"increment\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "decrement") == 0) {
			NEXT_ARG();
			if (get_prob(&opt.decrement, *argv)) {
				fprintf(stderr, "Illegal \"decrement\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "penalty_rate") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.penalty_rate, *argv, 0)) {
				fprintf(stderr, "Illegal \"penalty_rate\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "penalty_burst") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.penalty_burst, *argv, 0)) {
				fprintf(stderr, "Illegal \"penalty_burst\"\n");
				return -1;
			}
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (opt.max == 0) {
		if (opt.bin_size >= 1)
			opt.max = (opt.bin_size * 5 + 1) / 4;
		else
			opt.max = 25;
	}
	if (opt.bin_size == 0)
		opt.bin_size = (opt.max * 4 + 3) / 5;

	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	addattr_l(n, 1024, TCA_SFB_PARMS, &opt, sizeof(opt));
	addattr_nest_end(n, tail);
	return 0;
}

static int sfb_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[__TCA_SFB_MAX];
	struct tc_sfb_qopt *qopt;

	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_SFB_MAX, opt);
	if (tb[TCA_SFB_PARMS] == NULL)
		return -1;
	qopt = RTA_DATA(tb[TCA_SFB_PARMS]);
	if (RTA_PAYLOAD(tb[TCA_SFB_PARMS]) < sizeof(*qopt))
		return -1;

	print_uint(PRINT_JSON, "rehash", NULL, qopt->rehash_interval * 1000);
	print_string(PRINT_FP, NULL, "rehash %s ",
		     sprint_time(qopt->rehash_interval * 1000, b1));

	print_uint(PRINT_JSON, "db", NULL, qopt->warmup_time * 1000);
	print_string(PRINT_FP, NULL, "db %s ",
		     sprint_time(qopt->warmup_time * 1000, b1));

	print_uint(PRINT_ANY, "limit", "limit %up ", qopt->limit);
	print_uint(PRINT_ANY, "max", "max %up ", qopt->max);
	print_uint(PRINT_ANY, "target", "target %up ", qopt->bin_size);

	print_float(PRINT_ANY, "increment", "increment %lg ",
		    (double)qopt->increment / SFB_MAX_PROB);
	print_float(PRINT_ANY, "decrement", "decrement %lg ",
		    (double)qopt->decrement / SFB_MAX_PROB);

	print_uint(PRINT_ANY, "penalty_rate", "penalty_rate %upps ",
		   qopt->penalty_rate);
	print_uint(PRINT_ANY, "penalty_burst", "penalty_burst %up ",
		   qopt->penalty_burst);

	return 0;
}

static int sfb_print_xstats(struct qdisc_util *qu, FILE *f,
			    struct rtattr *xstats)
{
	struct tc_sfb_xstats *st;

	if (xstats == NULL)
		return 0;

	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;

	st = RTA_DATA(xstats);

	print_uint(PRINT_ANY, "earlydrop", "  earlydrop %u", st->earlydrop);
	print_uint(PRINT_ANY, "penaltydrop", " penaltydrop %u",
		   st->penaltydrop);
	print_uint(PRINT_ANY, "bucketdrop", " bucketdrop %u", st->bucketdrop);
	print_uint(PRINT_ANY, "queuedrop", " queuedrop %u", st->queuedrop);
	print_uint(PRINT_ANY, "childdrop", " childdrop %u", st->childdrop);
	print_uint(PRINT_ANY, "marked", " marked %u", st->marked);
	print_nl();
	print_uint(PRINT_ANY, "maxqlen", "  maxqlen %u", st->maxqlen);

	print_float(PRINT_ANY, "maxprob", " maxprob %lg",
		    (double)st->maxprob / SFB_MAX_PROB);
	print_float(PRINT_ANY, "avgprob", " avgprob %lg",
		    (double)st->avgprob / SFB_MAX_PROB);

	return 0;
}

struct qdisc_util sfb_qdisc_util = {
	.id		= "sfb",
	.parse_qopt	= sfb_parse_opt,
	.print_qopt	= sfb_print_opt,
	.print_xstats	= sfb_print_xstats,
};
