/*
 * Codel - The Controlled-Delay Active Queue Management algorithm
 *
 *  Copyright (C) 2011-2012 Kathleen Nichols <nichols@pollere.com>
 *  Copyright (C) 2011-2012 Van Jacobson <van@pollere.com>
 *  Copyright (C) 2012 Michael D. Taht <dave.taht@bufferbloat.net>
 *  Copyright (C) 2012,2015 Eric Dumazet <edumazet@google.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, provided that this notice is retained in full, this
 * software may be distributed under the terms of the GNU General
 * Public License ("GPL") version 2, in which case the provisions of the
 * GPL apply INSTEAD OF those given above.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
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
		"Usage: ... codel [ limit PACKETS ] [ target TIME ]\n"
		"		 [ interval TIME ] [ ecn | noecn ]\n"
		"		 [ ce_threshold TIME ]\n");
}

static int codel_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			   struct nlmsghdr *n, const char *dev)
{
	unsigned int limit = 0;
	unsigned int target = 0;
	unsigned int interval = 0;
	unsigned int ce_threshold = ~0U;
	int ecn = -1;
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
		} else if (strcmp(*argv, "ce_threshold") == 0) {
			NEXT_ARG();
			if (get_time(&ce_threshold, *argv)) {
				fprintf(stderr, "Illegal \"ce_threshold\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "interval") == 0) {
			NEXT_ARG();
			if (get_time(&interval, *argv)) {
				fprintf(stderr, "Illegal \"interval\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "ecn") == 0) {
			ecn = 1;
		} else if (strcmp(*argv, "noecn") == 0) {
			ecn = 0;
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
		addattr_l(n, 1024, TCA_CODEL_LIMIT, &limit, sizeof(limit));
	if (interval)
		addattr_l(n, 1024, TCA_CODEL_INTERVAL, &interval, sizeof(interval));
	if (target)
		addattr_l(n, 1024, TCA_CODEL_TARGET, &target, sizeof(target));
	if (ecn != -1)
		addattr_l(n, 1024, TCA_CODEL_ECN, &ecn, sizeof(ecn));
	if (ce_threshold != ~0U)
		addattr_l(n, 1024, TCA_CODEL_CE_THRESHOLD,
			  &ce_threshold, sizeof(ce_threshold));

	addattr_nest_end(n, tail);
	return 0;
}

static int codel_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_CODEL_MAX + 1];
	unsigned int limit;
	unsigned int interval;
	unsigned int target;
	unsigned int ecn;
	unsigned int ce_threshold;

	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_CODEL_MAX, opt);

	if (tb[TCA_CODEL_LIMIT] &&
	    RTA_PAYLOAD(tb[TCA_CODEL_LIMIT]) >= sizeof(__u32)) {
		limit = rta_getattr_u32(tb[TCA_CODEL_LIMIT]);
		print_uint(PRINT_ANY, "limit", "limit %up ", limit);
	}
	if (tb[TCA_CODEL_TARGET] &&
	    RTA_PAYLOAD(tb[TCA_CODEL_TARGET]) >= sizeof(__u32)) {
		target = rta_getattr_u32(tb[TCA_CODEL_TARGET]);
		print_uint(PRINT_JSON, "target", NULL, target);
		print_string(PRINT_FP, NULL, "target %s ",
			     sprint_time(target, b1));
	}
	if (tb[TCA_CODEL_CE_THRESHOLD] &&
	    RTA_PAYLOAD(tb[TCA_CODEL_CE_THRESHOLD]) >= sizeof(__u32)) {
		ce_threshold = rta_getattr_u32(tb[TCA_CODEL_CE_THRESHOLD]);
		print_uint(PRINT_JSON, "ce_threshold", NULL, ce_threshold);
		print_string(PRINT_FP, NULL, "ce_threshold %s ",
			     sprint_time(ce_threshold, b1));
	}
	if (tb[TCA_CODEL_INTERVAL] &&
	    RTA_PAYLOAD(tb[TCA_CODEL_INTERVAL]) >= sizeof(__u32)) {
		interval = rta_getattr_u32(tb[TCA_CODEL_INTERVAL]);
		print_uint(PRINT_JSON, "interval", NULL, interval);
		print_string(PRINT_FP, NULL, "interval %s ",
			     sprint_time(interval, b1));
	}
	if (tb[TCA_CODEL_ECN] &&
	    RTA_PAYLOAD(tb[TCA_CODEL_ECN]) >= sizeof(__u32)) {
		ecn = rta_getattr_u32(tb[TCA_CODEL_ECN]);
		if (ecn)
			print_bool(PRINT_ANY, "ecn", "ecn ", true);
	}

	return 0;
}

static int codel_print_xstats(struct qdisc_util *qu, FILE *f,
			      struct rtattr *xstats)
{
	struct tc_codel_xstats _st = {}, *st;

	SPRINT_BUF(b1);

	if (xstats == NULL)
		return 0;

	st = RTA_DATA(xstats);
	if (RTA_PAYLOAD(xstats) < sizeof(*st)) {
		memcpy(&_st, st, RTA_PAYLOAD(xstats));
		st = &_st;
	}

	print_uint(PRINT_ANY, "count", "  count %u", st->count);
	print_uint(PRINT_ANY, "lastcount", " lastcount %u", st->lastcount);
	print_uint(PRINT_JSON, "ldelay", NULL, st->ldelay);
	print_string(PRINT_FP, NULL, " ldelay %s", sprint_time(st->ldelay, b1));

	if (st->dropping)
		print_bool(PRINT_ANY, "dropping", " dropping", true);

	print_int(PRINT_JSON, "drop_next", NULL, st->drop_next);
	if (st->drop_next < 0)
		print_string(PRINT_FP, NULL, " drop_next -%s",
			     sprint_time(-st->drop_next, b1));
	else
		print_string(PRINT_FP, NULL, " drop_next %s",
			     sprint_time(st->drop_next, b1));

	print_nl();
	print_uint(PRINT_ANY, "maxpacket", "  maxpacket %u", st->maxpacket);
	print_uint(PRINT_ANY, "ecn_mark", " ecn_mark %u", st->ecn_mark);
	print_uint(PRINT_ANY, "drop_overlimit", " drop_overlimit %u",
		   st->drop_overlimit);

	if (st->ce_mark)
		print_uint(PRINT_ANY, "ce_mark", " ce_mark %u", st->ce_mark);

	return 0;

}

struct qdisc_util codel_qdisc_util = {
	.id		= "codel",
	.parse_qopt	= codel_parse_opt,
	.print_qopt	= codel_print_opt,
	.print_xstats	= codel_print_xstats,
};
