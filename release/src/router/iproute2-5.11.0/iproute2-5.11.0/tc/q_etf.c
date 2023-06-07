/*
 * q_etf.c		Earliest TxTime First (ETF).
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Vinicius Costa Gomes <vinicius.gomes@intel.com>
 *		Jesus Sanchez-Palencia <jesus.sanchez-palencia@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

#define CLOCKID_INVALID (-1)
static const struct static_clockid {
	const char *name;
	clockid_t clockid;
} clockids_sysv[] = {
	{ "REALTIME", CLOCK_REALTIME },
	{ "TAI", CLOCK_TAI },
	{ "BOOTTIME", CLOCK_BOOTTIME },
	{ "MONOTONIC", CLOCK_MONOTONIC },
	{ NULL }
};

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... etf delta NANOS clockid CLOCKID [offload] [deadline_mode]\n"
		"CLOCKID must be a valid SYS-V id (i.e. CLOCK_TAI)\n");
}

static void explain1(const char *arg, const char *val)
{
	fprintf(stderr, "etf: illegal value for \"%s\": \"%s\"\n", arg, val);
}

static void explain_clockid(const char *val)
{
	fprintf(stderr,
		"etf: illegal value for \"clockid\": \"%s\".\n"
		"It must be a valid SYS-V id (i.e. CLOCK_TAI)\n",
		val);
}

static int get_clockid(__s32 *val, const char *arg)
{
	const struct static_clockid *c;

	/* Drop the CLOCK_ prefix if that is being used. */
	if (strcasestr(arg, "CLOCK_") != NULL)
		arg += sizeof("CLOCK_") - 1;

	for (c = clockids_sysv; c->name; c++) {
		if (strcasecmp(c->name, arg) == 0) {
			*val = c->clockid;

			return 0;
		}
	}

	return -1;
}

static const char* get_clock_name(clockid_t clockid)
{
	const struct static_clockid *c;

	for (c = clockids_sysv; c->name; c++) {
		if (clockid == c->clockid)
			return c->name;
	}

	return "invalid";
}

static int etf_parse_opt(struct qdisc_util *qu, int argc,
			 char **argv, struct nlmsghdr *n, const char *dev)
{
	struct tc_etf_qopt opt = {
		.clockid = CLOCKID_INVALID,
	};
	struct rtattr *tail;

	while (argc > 0) {
		if (matches(*argv, "offload") == 0) {
			if (opt.flags & TC_ETF_OFFLOAD_ON) {
				fprintf(stderr, "etf: duplicate \"offload\" specification\n");
				return -1;
			}

			opt.flags |= TC_ETF_OFFLOAD_ON;
		} else if (matches(*argv, "deadline_mode") == 0) {
			if (opt.flags & TC_ETF_DEADLINE_MODE_ON) {
				fprintf(stderr, "etf: duplicate \"deadline_mode\" specification\n");
				return -1;
			}

			opt.flags |= TC_ETF_DEADLINE_MODE_ON;
		} else if (matches(*argv, "delta") == 0) {
			NEXT_ARG();
			if (opt.delta) {
				fprintf(stderr, "etf: duplicate \"delta\" specification\n");
				return -1;
			}
			if (get_s32(&opt.delta, *argv, 0)) {
				explain1("delta", *argv);
				return -1;
			}
		} else if (matches(*argv, "clockid") == 0) {
			NEXT_ARG();
			if (opt.clockid != CLOCKID_INVALID) {
				fprintf(stderr, "etf: duplicate \"clockid\" specification\n");
				return -1;
			}
			if (get_clockid(&opt.clockid, *argv)) {
				explain_clockid(*argv);
				return -1;
			}
		} else if (strcmp(*argv, "skip_sock_check") == 0) {
			if (opt.flags & TC_ETF_SKIP_SOCK_CHECK) {
				fprintf(stderr, "etf: duplicate \"skip_sock_check\" specification\n");
				return -1;
			}

			opt.flags |= TC_ETF_SKIP_SOCK_CHECK;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "etf: unknown parameter \"%s\"\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 2024, TCA_ETF_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;
	return 0;
}

static int etf_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_ETF_MAX+1];
	struct tc_etf_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_ETF_MAX, opt);

	if (tb[TCA_ETF_PARMS] == NULL)
		return -1;

	qopt = RTA_DATA(tb[TCA_ETF_PARMS]);
	if (RTA_PAYLOAD(tb[TCA_ETF_PARMS])  < sizeof(*qopt))
		return -1;

	print_string(PRINT_ANY, "clockid", "clockid %s ",
		     get_clock_name(qopt->clockid));

	print_uint(PRINT_ANY, "delta", "delta %d ", qopt->delta);
	print_string(PRINT_ANY, "offload", "offload %s ",
				(qopt->flags & TC_ETF_OFFLOAD_ON) ? "on" : "off");
	print_string(PRINT_ANY, "deadline_mode", "deadline_mode %s ",
				(qopt->flags & TC_ETF_DEADLINE_MODE_ON) ? "on" : "off");
	print_string(PRINT_ANY, "skip_sock_check", "skip_sock_check %s",
				(qopt->flags & TC_ETF_SKIP_SOCK_CHECK) ? "on" : "off");

	return 0;
}

struct qdisc_util etf_qdisc_util = {
	.id		= "etf",
	.parse_qopt	= etf_parse_opt,
	.print_qopt	= etf_print_opt,
};
