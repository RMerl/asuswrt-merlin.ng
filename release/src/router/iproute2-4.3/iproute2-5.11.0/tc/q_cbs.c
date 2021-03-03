/*
 * q_cbs.c		CBS.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Vinicius Costa Gomes <vinicius.gomes@intel.com>
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
		"Usage: ... cbs hicredit BYTES locredit BYTES sendslope BPS idleslope BPS\n"
		"	   [offload 0|1]\n");
}

static void explain1(const char *arg, const char *val)
{
	fprintf(stderr, "cbs: illegal value for \"%s\": \"%s\"\n", arg, val);
}

static int cbs_parse_opt(struct qdisc_util *qu, int argc,
			 char **argv, struct nlmsghdr *n, const char *dev)
{
	struct tc_cbs_qopt opt = {};
	struct rtattr *tail;

	while (argc > 0) {
		if (matches(*argv, "offload") == 0) {
			NEXT_ARG();
			if (opt.offload) {
				fprintf(stderr, "cbs: duplicate \"offload\" specification\n");
				return -1;
			}
			if (get_u8(&opt.offload, *argv, 0)) {
				explain1("offload", *argv);
				return -1;
			}
		} else if (matches(*argv, "hicredit") == 0) {
			NEXT_ARG();
			if (opt.hicredit) {
				fprintf(stderr, "cbs: duplicate \"hicredit\" specification\n");
				return -1;
			}
			if (get_s32(&opt.hicredit, *argv, 0)) {
				explain1("hicredit", *argv);
				return -1;
			}
		} else if (matches(*argv, "locredit") == 0) {
			NEXT_ARG();
			if (opt.locredit) {
				fprintf(stderr, "cbs: duplicate \"locredit\" specification\n");
				return -1;
			}
			if (get_s32(&opt.locredit, *argv, 0)) {
				explain1("locredit", *argv);
				return -1;
			}
		} else if (matches(*argv, "sendslope") == 0) {
			NEXT_ARG();
			if (opt.sendslope) {
				fprintf(stderr, "cbs: duplicate \"sendslope\" specification\n");
				return -1;
			}
			if (get_s32(&opt.sendslope, *argv, 0)) {
				explain1("sendslope", *argv);
				return -1;
			}
		} else if (matches(*argv, "idleslope") == 0) {
			NEXT_ARG();
			if (opt.idleslope) {
				fprintf(stderr, "cbs: duplicate \"idleslope\" specification\n");
				return -1;
			}
			if (get_s32(&opt.idleslope, *argv, 0)) {
				explain1("idleslope", *argv);
				return -1;
			}
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "cbs: unknown parameter \"%s\"\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	tail = addattr_nest(n, 1024, TCA_OPTIONS);
	addattr_l(n, 2024, TCA_CBS_PARMS, &opt, sizeof(opt));
	addattr_nest_end(n, tail);
	return 0;
}

static int cbs_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_CBS_MAX+1];
	struct tc_cbs_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_CBS_MAX, opt);

	if (tb[TCA_CBS_PARMS] == NULL)
		return -1;

	qopt = RTA_DATA(tb[TCA_CBS_PARMS]);
	if (RTA_PAYLOAD(tb[TCA_CBS_PARMS])  < sizeof(*qopt))
		return -1;

	print_int(PRINT_ANY, "hicredit", "hicredit %d ", qopt->hicredit);
	print_int(PRINT_ANY, "locredit", "locredit %d ", qopt->locredit);
	print_int(PRINT_ANY, "sendslope", "sendslope %d ", qopt->sendslope);
	print_int(PRINT_ANY, "idleslope", "idleslope %d ", qopt->idleslope);
	print_int(PRINT_ANY, "offload", "offload %d ", qopt->offload);

	return 0;
}

struct qdisc_util cbs_qdisc_util = {
	.id		= "cbs",
	.parse_qopt	= cbs_parse_opt,
	.print_qopt	= cbs_print_opt,
};
