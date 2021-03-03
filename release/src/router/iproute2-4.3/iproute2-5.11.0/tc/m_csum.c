/*
 * m_csum.c	checksum updating action
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors: Gregoire Baron <baronchon@n7mm.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/tc_act/tc_csum.h>

#include "utils.h"
#include "tc_util.h"

static void
explain(void)
{
	fprintf(stderr, "Usage: ... csum <UPDATE>\n"
			"Where: UPDATE := <TARGET> [<UPDATE>]\n"
			"       TARGET := { ip4h | icmp | igmp | tcp | udp | udplite | sctp | <SWEETS> }\n"
			"       SWEETS := { and | or | \'+\' }\n");
}

static void
usage(void)
{
	explain();
	exit(-1);
}

static int
parse_csum_args(int *argc_p, char ***argv_p, struct tc_csum *sel)
{
	int argc = *argc_p;
	char **argv = *argv_p;

	if (argc <= 0)
		return -1;

	while (argc > 0) {
		if ((matches(*argv, "iph") == 0) ||
		    (matches(*argv, "ip4h") == 0) ||
		    (matches(*argv, "ipv4h") == 0))
			sel->update_flags |= TCA_CSUM_UPDATE_FLAG_IPV4HDR;

		else if (matches(*argv, "icmp") == 0)
			sel->update_flags |= TCA_CSUM_UPDATE_FLAG_ICMP;

		else if (matches(*argv, "igmp") == 0)
			sel->update_flags |= TCA_CSUM_UPDATE_FLAG_IGMP;

		else if (matches(*argv, "tcp") == 0)
			sel->update_flags |= TCA_CSUM_UPDATE_FLAG_TCP;

		else if (matches(*argv, "udp") == 0)
			sel->update_flags |= TCA_CSUM_UPDATE_FLAG_UDP;

		else if (matches(*argv, "udplite") == 0)
			sel->update_flags |= TCA_CSUM_UPDATE_FLAG_UDPLITE;

		else if (matches(*argv, "sctp") == 0)
			sel->update_flags |= TCA_CSUM_UPDATE_FLAG_SCTP;

		else if ((matches(*argv, "and") == 0) ||
			 (matches(*argv, "or") == 0) ||
			 (matches(*argv, "+") == 0))
			; /* just ignore: ... csum iph and tcp or udp */
		else
			break;
		argc--;
		argv++;
	}

	*argc_p = argc;
	*argv_p = argv;

	return 0;
}

static int
parse_csum(struct action_util *a, int *argc_p,
	   char ***argv_p, int tca_id, struct nlmsghdr *n)
{
	struct tc_csum sel = {};

	int argc = *argc_p;
	char **argv = *argv_p;
	int ok = 0;
	struct rtattr *tail;

	while (argc > 0) {
		if (matches(*argv, "csum") == 0) {
			NEXT_ARG();
			if (parse_csum_args(&argc, &argv, &sel)) {
				fprintf(stderr, "Illegal csum construct (%s)\n",
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

	if (sel.update_flags == 0) {
		fprintf(stderr, "Illegal csum construct, empty <UPDATE> list\n");
		return -1;
	}

	parse_action_control_dflt(&argc, &argv, &sel.action, false, TC_ACT_OK);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&sel.index, *argv, 10)) {
				fprintf(stderr, "Illegal \"index\" (%s) <csum>\n",
					*argv);
				return -1;
			}
			argc--;
			argv++;
		}
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_CSUM_PARMS, &sel, sizeof(sel));
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;

	return 0;
}

static int
print_csum(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct tc_csum *sel;

	struct rtattr *tb[TCA_CSUM_MAX + 1];

	char *uflag_1 = "";
	char *uflag_2 = "";
	char *uflag_3 = "";
	char *uflag_4 = "";
	char *uflag_5 = "";
	char *uflag_6 = "";
	char *uflag_7 = "";
	SPRINT_BUF(buf);

	int uflag_count = 0;

	print_string(PRINT_ANY, "kind", "%s ", "csum");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_CSUM_MAX, arg);

	if (tb[TCA_CSUM_PARMS] == NULL) {
		fprintf(stderr, "Missing csum parameters\n");
		return -1;
	}
	sel = RTA_DATA(tb[TCA_CSUM_PARMS]);

	if (sel->update_flags & TCA_CSUM_UPDATE_FLAG_IPV4HDR) {
		uflag_1 = "iph";
		uflag_count++;
	}
	#define CSUM_UFLAG_BUFFER(flag_buffer, flag_value, flag_string)	\
		do {							\
			if (sel->update_flags & flag_value) {		\
				flag_buffer = uflag_count > 0 ?		\
					", " flag_string : flag_string; \
				uflag_count++;				\
			}						\
		} while (0)
	CSUM_UFLAG_BUFFER(uflag_2, TCA_CSUM_UPDATE_FLAG_ICMP, "icmp");
	CSUM_UFLAG_BUFFER(uflag_3, TCA_CSUM_UPDATE_FLAG_IGMP, "igmp");
	CSUM_UFLAG_BUFFER(uflag_4, TCA_CSUM_UPDATE_FLAG_TCP, "tcp");
	CSUM_UFLAG_BUFFER(uflag_5, TCA_CSUM_UPDATE_FLAG_UDP, "udp");
	CSUM_UFLAG_BUFFER(uflag_6, TCA_CSUM_UPDATE_FLAG_UDPLITE, "udplite");
	CSUM_UFLAG_BUFFER(uflag_7, TCA_CSUM_UPDATE_FLAG_SCTP, "sctp");
	if (!uflag_count) {
		uflag_1 = "?empty";
	}

	snprintf(buf, sizeof(buf), "%s%s%s%s%s%s%s",
		 uflag_1, uflag_2, uflag_3,
		 uflag_4, uflag_5, uflag_6, uflag_7);
	print_string(PRINT_ANY, "csum", "(%s) ", buf);

	print_action_control(f, "action ", sel->action, _SL_);
	print_uint(PRINT_ANY, "index", "\tindex %u", sel->index);
	print_int(PRINT_ANY, "ref", " ref %d", sel->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", sel->bindcnt);

	if (show_stats) {
		if (tb[TCA_CSUM_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_CSUM_TM]);

			print_tm(f, tm);
		}
	}
	print_nl();

	return 0;
}

struct action_util csum_action_util = {
	.id = "csum",
	.parse_aopt = parse_csum,
	.print_aopt = print_csum,
};
