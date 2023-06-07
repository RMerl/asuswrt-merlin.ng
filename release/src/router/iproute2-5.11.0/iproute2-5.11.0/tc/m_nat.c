/*
 * m_nat.c		NAT module
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Herbert Xu <herbert@gondor.apana.org.au>
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
#include <linux/tc_act/tc_nat.h>

static void
explain(void)
{
	fprintf(stderr, "Usage: ... nat NAT\n"
			"NAT := DIRECTION OLD NEW\n"
			"DIRECTION := { ingress | egress }\n"
			"OLD := PREFIX\n"
			"NEW := ADDRESS\n");
}

static void
usage(void)
{
	explain();
	exit(-1);
}

static int
parse_nat_args(int *argc_p, char ***argv_p, struct tc_nat *sel)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	inet_prefix addr;

	if (argc <= 0)
		return -1;

	if (matches(*argv, "egress") == 0)
		sel->flags |= TCA_NAT_FLAG_EGRESS;
	else if (matches(*argv, "ingress") != 0)
		goto bad_val;

	NEXT_ARG();

	if (get_prefix_1(&addr, *argv, AF_INET))
		goto bad_val;

	sel->old_addr = addr.data[0];
	sel->mask = htonl(~0u << (32 - addr.bitlen));

	NEXT_ARG();

	if (get_prefix_1(&addr, *argv, AF_INET))
		goto bad_val;

	sel->new_addr = addr.data[0];

	argc--;
	argv++;

	*argc_p = argc;
	*argv_p = argv;
	return 0;

bad_val:
	return -1;
}

static int
parse_nat(struct action_util *a, int *argc_p, char ***argv_p, int tca_id, struct nlmsghdr *n)
{
	struct tc_nat sel = {};

	int argc = *argc_p;
	char **argv = *argv_p;
	int ok = 0;
	struct rtattr *tail;

	while (argc > 0) {
		if (matches(*argv, "nat") == 0) {
			NEXT_ARG();
			if (parse_nat_args(&argc, &argv, &sel)) {
				fprintf(stderr, "Illegal nat construct (%s)\n",
					*argv);
				explain();
				return -1;
			}
			ok++;
			continue;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}

	}

	if (!ok) {
		explain();
		return -1;
	}

	parse_action_control_dflt(&argc, &argv, &sel.action, false, TC_ACT_OK);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.index, *argv, 10)) {
				fprintf(stderr, "Nat: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
		}
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_NAT_PARMS, &sel, sizeof(sel));
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int
print_nat(struct action_util *au, FILE * f, struct rtattr *arg)
{
	struct tc_nat *sel;
	struct rtattr *tb[TCA_NAT_MAX + 1];
	SPRINT_BUF(buf1);
	SPRINT_BUF(buf2);
	int len;

	print_string(PRINT_ANY, "type", " %s ", "nat");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NAT_MAX, arg);

	if (tb[TCA_NAT_PARMS] == NULL) {
		fprintf(stderr, "Missing nat parameters\n");
		return -1;
	}
	sel = RTA_DATA(tb[TCA_NAT_PARMS]);

	len = ffs(sel->mask);
	len = len ? 33 - len : 0;

	print_string(PRINT_ANY, "direction", "%s",
		     sel->flags & TCA_NAT_FLAG_EGRESS ? "egress" : "ingress");

	snprintf(buf2, sizeof(buf2), "%s/%d",
		 format_host_r(AF_INET, 4, &sel->old_addr, buf1, sizeof(buf1)),
		 len);
	print_string(PRINT_ANY, "old_addr", " %s", buf2);
	print_string(PRINT_ANY, "new_addr", " %s",
		     format_host_r(AF_INET, 4, &sel->new_addr, buf1, sizeof(buf1)));

	print_action_control(f, " ", sel->action, "");
	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", sel->index);
	print_int(PRINT_ANY, "ref", " ref %d", sel->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", sel->bindcnt);

	if (show_stats) {
		if (tb[TCA_NAT_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_NAT_TM]);

			print_tm(f, tm);
		}
	}

	print_nl();

	return 0;
}

struct action_util nat_action_util = {
	.id = "nat",
	.parse_aopt = parse_nat,
	.print_aopt = print_nat,
};
