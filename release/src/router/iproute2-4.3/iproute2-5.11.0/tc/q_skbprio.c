/*
 * q_skbprio.c		SKB PRIORITY QUEUE.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Nishanth Devarajan, <ndev2021@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... <skbprio> [ limit NUMBER ]\n");
}

static int skbprio_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			     struct nlmsghdr *n, const char *dev)
{
	int ok = 0;
	struct tc_skbprio_qopt opt = {};

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&opt.limit, *argv)) {
				fprintf(stderr,
					"%s: Illegal \"limit\" value:\"%s\"\n",
					 qu->id, *argv);
				return -1;
			}
			ok++;
		}
		else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr,
				"%s: unknown parameter \"%s\"\n",
				qu->id, *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (ok)
		addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
	return 0;
}

static int skbprio_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct tc_skbprio_qopt *qopt;

	if (opt == NULL)
		return 0;

	if (RTA_PAYLOAD(opt)  < sizeof(*qopt))
		return -1;
	qopt = RTA_DATA(opt);

	print_uint(PRINT_ANY, "limit", "limit %u ", qopt->limit);
	return 0;
}

struct qdisc_util skbprio_qdisc_util = {
	.id = "skbprio",
	.parse_qopt = skbprio_parse_opt,
	.print_qopt = skbprio_print_opt,
};
