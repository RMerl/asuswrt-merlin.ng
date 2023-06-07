/* SPDX-License-Identifier: GPL-2.0 */
/*
 * q_log.c		plug scheduler
 *
 * Copyright (C) 2019	Paolo Abeni <pabeni@redhat.com>
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
	fprintf(stderr, "Usage: ... plug [block | release | release_indefinite | limit NUMBER]\n");
}

static int plug_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			  struct nlmsghdr *n, const char *dev)
{
	struct tc_plug_qopt opt = {};
	int ok = 0;

	while (argc > 0) {
		if (strcmp(*argv, "block") == 0) {
			opt.action = TCQ_PLUG_BUFFER;
			ok++;
		} else if (strcmp(*argv, "release") == 0) {
			opt.action = TCQ_PLUG_RELEASE_ONE;
			ok++;
		} else if (strcmp(*argv, "release_indefinite") == 0) {
			opt.action = TCQ_PLUG_RELEASE_INDEFINITE;
			ok++;
		} else if (strcmp(*argv, "limit") == 0) {
			opt.action = TCQ_PLUG_LIMIT;
			NEXT_ARG();
			if (get_size(&opt.limit, *argv)) {
				fprintf(stderr, "Illegal value for \"limit\": \"%s\"\n", *argv);
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "%s: unknown parameter \"%s\"\n", qu->id, *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (ok)
		addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
	return 0;
}

static int plug_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	/* dummy implementation as sch_plug does not implement a dump op */
	return 0;
}


struct qdisc_util plug_qdisc_util = {
	.id = "plug",
	.parse_qopt = plug_parse_opt,
	.print_qopt = plug_print_opt,
};
