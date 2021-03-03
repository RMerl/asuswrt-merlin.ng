/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... clsact\n");
}

static int clsact_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			    struct nlmsghdr *n, const char *dev)
{
	if (argc > 0) {
		fprintf(stderr, "What is \"%s\"?\n", *argv);
		explain();
		return -1;
	}

	return 0;
}

static int clsact_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	return 0;
}

struct qdisc_util clsact_qdisc_util = {
	.id		= "clsact",
	.parse_qopt	= clsact_parse_opt,
	.print_qopt	= clsact_print_opt,
};
