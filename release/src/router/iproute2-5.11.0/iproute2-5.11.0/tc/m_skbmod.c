/*
 * m_skbmod.c	skb modifier action module
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:  J Hadi Salim (jhs@mojatatu.com)
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
#include <linux/netdevice.h>

#include "rt_names.h"
#include "utils.h"
#include "tc_util.h"
#include <linux/tc_act/tc_skbmod.h>

static void skbmod_explain(void)
{
	fprintf(stderr,
		"Usage:... skbmod {[set <SETTABLE>] [swap <SWAPABLE>]} [CONTROL] [index INDEX]\n"
		"where SETTABLE is: [dmac DMAC] [smac SMAC] [etype ETYPE]\n"
		"where SWAPABLE is: \"mac\" to swap mac addresses\n"
		"note: \"swap mac\" is done after any outstanding D/SMAC change\n"
		"\tDMAC := 6 byte Destination MAC address\n"
		"\tSMAC := optional 6 byte Source MAC address\n"
		"\tETYPE := optional 16 bit ethertype\n"
		"\tCONTROL := reclassify | pipe | drop | continue | ok |\n"
		"\t           goto chain <CHAIN_INDEX>\n"
		"\tINDEX := skbmod index value to use\n");
}

static void skbmod_usage(void)
{
	skbmod_explain();
	exit(-1);
}

static int parse_skbmod(struct action_util *a, int *argc_p, char ***argv_p,
			int tca_id, struct nlmsghdr *n)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	int ok = 0;
	struct tc_skbmod p;
	struct rtattr *tail;
	char dbuf[ETH_ALEN];
	char sbuf[ETH_ALEN];
	__u16 skbmod_etype = 0;
	char *daddr = NULL;
	char *saddr = NULL;

	memset(&p, 0, sizeof(p));

	if (argc <= 0)
		return -1;

	while (argc > 0) {
		if (matches(*argv, "skbmod") == 0) {
			NEXT_ARG();
			continue;
		} else if (matches(*argv, "swap") == 0) {
			NEXT_ARG();
			continue;
		} else if (matches(*argv, "mac") == 0) {
			p.flags |= SKBMOD_F_SWAPMAC;
			ok += 1;
		} else if (matches(*argv, "set") == 0) {
			NEXT_ARG();
			continue;
		} else if (matches(*argv, "etype") == 0) {
			NEXT_ARG();
			if (get_u16(&skbmod_etype, *argv, 0))
				invarg("ethertype is invalid", *argv);
			fprintf(stderr, "skbmod etype 0x%x\n", skbmod_etype);
			p.flags |= SKBMOD_F_ETYPE;
			ok += 1;
		} else if (matches(*argv, "dmac") == 0) {
			NEXT_ARG();
			daddr = *argv;
			if (sscanf(daddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				   dbuf, dbuf + 1, dbuf + 2,
				   dbuf + 3, dbuf + 4, dbuf + 5) != 6) {
				fprintf(stderr, "Invalid dst mac address %s\n",
					daddr);
				return -1;
			}
			p.flags |= SKBMOD_F_DMAC;
			fprintf(stderr, "dst MAC address <%s>\n", daddr);
			ok += 1;

		} else if (matches(*argv, "smac") == 0) {
			NEXT_ARG();
			saddr = *argv;
			if (sscanf(saddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				   sbuf, sbuf + 1, sbuf + 2,
				   sbuf + 3, sbuf + 4, sbuf + 5) != 6) {
				fprintf(stderr, "Invalid smac address %s\n",
					saddr);
				return -1;
			}
			p.flags |= SKBMOD_F_SMAC;
			fprintf(stderr, "src MAC address <%s>\n", saddr);
			ok += 1;
		} else if (matches(*argv, "help") == 0) {
			skbmod_usage();
		} else {
			break;
		}

		argc--;
		argv++;
	}

	parse_action_control_dflt(&argc, &argv, &p.action, false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&p.index, *argv, 0)) {
				fprintf(stderr, "skbmod: Illegal \"index\"\n");
				return -1;
			}
			ok++;
			argc--;
			argv++;
		}
	}

	if (!ok) {
		fprintf(stderr, "skbmod requires at least one option\n");
		skbmod_usage();
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_SKBMOD_PARMS, &p, sizeof(p));

	if (daddr)
		addattr_l(n, MAX_MSG, TCA_SKBMOD_DMAC, dbuf, ETH_ALEN);
	if (skbmod_etype)
		addattr16(n, MAX_MSG, TCA_SKBMOD_ETYPE, skbmod_etype);
	if (saddr)
		addattr_l(n, MAX_MSG, TCA_SKBMOD_SMAC, sbuf, ETH_ALEN);

	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_skbmod(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct tc_skbmod *p;
	struct rtattr *tb[TCA_SKBMOD_MAX + 1];
	__u16 skbmod_etype = 0;
	int has_optional = 0;
	SPRINT_BUF(b1);
	SPRINT_BUF(b2);

	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_SKBMOD_MAX, arg);

	if (tb[TCA_SKBMOD_PARMS] == NULL) {
		fprintf(stderr, "Missing skbmod parameters\n");
		return -1;
	}

	p = RTA_DATA(tb[TCA_SKBMOD_PARMS]);

	fprintf(f, "skbmod ");
	print_action_control(f, "", p->action, " ");

	if (tb[TCA_SKBMOD_ETYPE]) {
		skbmod_etype = rta_getattr_u16(tb[TCA_SKBMOD_ETYPE]);
		has_optional = 1;
		fprintf(f, "set etype 0x%X ", skbmod_etype);
	}

	if (has_optional)
		fprintf(f, "\n\t ");

	if (tb[TCA_SKBMOD_DMAC]) {
		has_optional = 1;
		fprintf(f, "set dmac %s ",
			ll_addr_n2a(RTA_DATA(tb[TCA_SKBMOD_DMAC]),
				    RTA_PAYLOAD(tb[TCA_SKBMOD_DMAC]), 0, b1,
				    sizeof(b1)));

	}

	if (tb[TCA_SKBMOD_SMAC]) {
		has_optional = 1;
		fprintf(f, "set smac %s ",
			ll_addr_n2a(RTA_DATA(tb[TCA_SKBMOD_SMAC]),
				    RTA_PAYLOAD(tb[TCA_SKBMOD_SMAC]), 0, b2,
				    sizeof(b2)));
	}

	if (p->flags & SKBMOD_F_SWAPMAC)
		fprintf(f, "swap mac ");

	fprintf(f, "\n\t index %u ref %d bind %d", p->index, p->refcnt,
		p->bindcnt);
	if (show_stats) {
		if (tb[TCA_SKBMOD_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_SKBMOD_TM]);

			print_tm(f, tm);
		}
	}

	fprintf(f, "\n");

	return 0;
}

struct action_util skbmod_action_util = {
	.id = "skbmod",
	.parse_aopt = parse_skbmod,
	.print_aopt = print_skbmod,
};
