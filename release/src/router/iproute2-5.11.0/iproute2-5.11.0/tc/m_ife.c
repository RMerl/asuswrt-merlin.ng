/*
 * m_ife.c	IFE actions module
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
#include <linux/tc_act/tc_ife.h>

static void ife_explain(void)
{
	fprintf(stderr,
		"Usage:... ife {decode|encode} [{ALLOW|USE} ATTR] [dst DMAC] [src SMAC] [type TYPE] [CONTROL] [index INDEX]\n"
		"\tALLOW := Encode direction. Allows encoding specified metadata\n"
		"\t\t e.g \"allow mark\"\n"
		"\tUSE := Encode direction. Enforce Static encoding of specified metadata\n"
		"\t\t e.g \"use mark 0x12\"\n"
		"\tATTR := mark (32-bit), prio (32-bit), tcindex (16-bit)\n"
		"\tDMAC := 6 byte Destination MAC address to encode\n"
		"\tSMAC := optional 6 byte Source MAC address to encode\n"
		"\tTYPE := optional 16 bit ethertype to encode\n"
		"\tCONTROL := reclassify|pipe|drop|continue|ok\n"
		"\tINDEX := optional IFE table index value used\n"
		"encode is used for sending IFE packets\n"
		"decode is used for receiving IFE packets\n");
}

static void ife_usage(void)
{
	ife_explain();
	exit(-1);
}

static int parse_ife(struct action_util *a, int *argc_p, char ***argv_p,
		     int tca_id, struct nlmsghdr *n)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	int ok = 0;
	struct tc_ife p = { 0 };
	struct rtattr *tail;
	struct rtattr *tail2;
	char dbuf[ETH_ALEN];
	char sbuf[ETH_ALEN];
	__u16 ife_type = 0;
	int user_type = 0;
	__u32 ife_prio = 0;
	__u32 ife_prio_v = 0;
	__u32 ife_mark = 0;
	__u32 ife_mark_v = 0;
	__u16 ife_tcindex = 0;
	__u16 ife_tcindex_v = 0;
	char *daddr = NULL;
	char *saddr = NULL;

	if (argc <= 0)
		return -1;

	while (argc > 0) {
		if (matches(*argv, "ife") == 0) {
			NEXT_ARG();
			continue;
		} else if (matches(*argv, "decode") == 0) {
			p.flags = IFE_DECODE; /* readability aid */
			ok++;
		} else if (matches(*argv, "encode") == 0) {
			p.flags = IFE_ENCODE;
			ok++;
		} else if (matches(*argv, "allow") == 0) {
			NEXT_ARG();
			if (matches(*argv, "mark") == 0) {
				ife_mark = IFE_META_SKBMARK;
			} else if (matches(*argv, "prio") == 0) {
				ife_prio = IFE_META_PRIO;
			} else if (matches(*argv, "tcindex") == 0) {
				ife_tcindex = IFE_META_TCINDEX;
			} else {
				invarg("Illegal meta define", *argv);
			}
		} else if (matches(*argv, "use") == 0) {
			NEXT_ARG();
			if (matches(*argv, "mark") == 0) {
				NEXT_ARG();
				if (get_u32(&ife_mark_v, *argv, 0))
					invarg("ife mark val is invalid",
					       *argv);
			} else if (matches(*argv, "prio") == 0) {
				NEXT_ARG();
				if (get_u32(&ife_prio_v, *argv, 0))
					invarg("ife prio val is invalid",
					       *argv);
			} else if (matches(*argv, "tcindex") == 0) {
				NEXT_ARG();
				if (get_u16(&ife_tcindex_v, *argv, 0))
					invarg("ife tcindex val is invalid",
					       *argv);
			} else {
				invarg("Illegal meta use type", *argv);
			}
		} else if (matches(*argv, "type") == 0) {
			NEXT_ARG();
			if (get_u16(&ife_type, *argv, 0))
				invarg("ife type is invalid", *argv);
			fprintf(stderr, "IFE type 0x%04X\n", ife_type);
			user_type = 1;
		} else if (matches(*argv, "dst") == 0) {
			NEXT_ARG();
			daddr = *argv;
			if (sscanf(daddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				   dbuf, dbuf + 1, dbuf + 2,
				   dbuf + 3, dbuf + 4, dbuf + 5) != 6) {
				invarg("Invalid mac address", *argv);
			}
			fprintf(stderr, "dst MAC address <%s>\n", daddr);

		} else if (matches(*argv, "src") == 0) {
			NEXT_ARG();
			saddr = *argv;
			if (sscanf(saddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				   sbuf, sbuf + 1, sbuf + 2,
				   sbuf + 3, sbuf + 4, sbuf + 5) != 6) {
				invarg("Invalid mac address", *argv);
			}
			fprintf(stderr, "src MAC address <%s>\n", saddr);
		} else if (matches(*argv, "help") == 0) {
			ife_usage();
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
				fprintf(stderr, "ife: Illegal \"index\"\n");
				return -1;
			}
			ok++;
			argc--;
			argv++;
		}
	}

	if (!ok) {
		fprintf(stderr, "IFE requires decode/encode specified\n");
		ife_usage();
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_IFE_PARMS, &p, sizeof(p));

	if (!(p.flags & IFE_ENCODE))
		goto skip_encode;

	if (daddr)
		addattr_l(n, MAX_MSG, TCA_IFE_DMAC, dbuf, ETH_ALEN);
	if (user_type)
		addattr_l(n, MAX_MSG, TCA_IFE_TYPE, &ife_type, 2);
	else
		fprintf(stderr, "IFE type 0x%04X\n", ETH_P_IFE);
	if (saddr)
		addattr_l(n, MAX_MSG, TCA_IFE_SMAC, sbuf, ETH_ALEN);

	tail2 = addattr_nest(n, MAX_MSG, TCA_IFE_METALST);
	if (ife_mark || ife_mark_v) {
		if (ife_mark_v)
			addattr_l(n, MAX_MSG, IFE_META_SKBMARK, &ife_mark_v, 4);
		else
			addattr_l(n, MAX_MSG, IFE_META_SKBMARK, NULL, 0);
	}
	if (ife_prio || ife_prio_v) {
		if (ife_prio_v)
			addattr_l(n, MAX_MSG, IFE_META_PRIO, &ife_prio_v, 4);
		else
			addattr_l(n, MAX_MSG, IFE_META_PRIO, NULL, 0);
	}
	if (ife_tcindex || ife_tcindex_v) {
		if (ife_tcindex_v)
			addattr_l(n, MAX_MSG, IFE_META_TCINDEX, &ife_tcindex_v,
				  2);
		else
			addattr_l(n, MAX_MSG, IFE_META_TCINDEX, NULL, 0);
	}

	addattr_nest_end(n, tail2);

skip_encode:
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_ife(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct tc_ife *p;
	struct rtattr *tb[TCA_IFE_MAX + 1];
	__u16 ife_type = 0;
	__u32 mmark = 0;
	__u16 mtcindex = 0;
	__u32 mprio = 0;
	int has_optional = 0;
	SPRINT_BUF(b2);

	print_string(PRINT_ANY, "kind", "%s ", "ife");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_IFE_MAX, arg);

	if (tb[TCA_IFE_PARMS] == NULL) {
		fprintf(stderr, "Missing ife parameters\n");
		return -1;
	}
	p = RTA_DATA(tb[TCA_IFE_PARMS]);

	print_string(PRINT_ANY, "mode", "%s ",
		     p->flags & IFE_ENCODE ? "encode" : "decode");
	print_action_control(f, "action ", p->action, " ");

	if (tb[TCA_IFE_TYPE]) {
		ife_type = rta_getattr_u16(tb[TCA_IFE_TYPE]);
		has_optional = 1;
		print_0xhex(PRINT_ANY, "type", "type %#llX ", ife_type);
	}

	if (has_optional)
		print_string(PRINT_FP, NULL, "%s\t", _SL_);

	if (tb[TCA_IFE_METALST]) {
		struct rtattr *metalist[IFE_META_MAX + 1];
		int len = 0;

		parse_rtattr_nested(metalist, IFE_META_MAX,
				    tb[TCA_IFE_METALST]);

		if (metalist[IFE_META_SKBMARK]) {
			len = RTA_PAYLOAD(metalist[IFE_META_SKBMARK]);
			if (len) {
				mmark = rta_getattr_u32(metalist[IFE_META_SKBMARK]);
				print_uint(PRINT_ANY, "mark", "use mark %u ",
					   mmark);
			} else
				print_string(PRINT_ANY, "mark", "%s mark ",
					     "allow");
		}

		if (metalist[IFE_META_TCINDEX]) {
			len = RTA_PAYLOAD(metalist[IFE_META_TCINDEX]);
			if (len) {
				mtcindex =
					rta_getattr_u16(metalist[IFE_META_TCINDEX]);
				print_uint(PRINT_ANY, "tcindex",
					   "use tcindex %u ", mtcindex);
			} else
				print_string(PRINT_ANY, "tcindex",
					     "%s tcindex ", "allow");
		}

		if (metalist[IFE_META_PRIO]) {
			len = RTA_PAYLOAD(metalist[IFE_META_PRIO]);
			if (len) {
				mprio = rta_getattr_u32(metalist[IFE_META_PRIO]);
				print_uint(PRINT_ANY, "prio", "use prio %u ",
					   mprio);
			} else
				print_string(PRINT_ANY, "prio", "%s prio ",
					     "allow");
		}

	}

	if (tb[TCA_IFE_DMAC]) {
		has_optional = 1;
		print_string(PRINT_ANY, "dst", "dst %s ",
			     ll_addr_n2a(RTA_DATA(tb[TCA_IFE_DMAC]),
					 RTA_PAYLOAD(tb[TCA_IFE_DMAC]), 0, b2,
					 sizeof(b2)));
	}

	if (tb[TCA_IFE_SMAC]) {
		has_optional = 1;
		print_string(PRINT_ANY, "src", "src %s ",
			     ll_addr_n2a(RTA_DATA(tb[TCA_IFE_SMAC]),
					 RTA_PAYLOAD(tb[TCA_IFE_SMAC]), 0, b2,
					 sizeof(b2)));
	}

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", p->index);
	print_int(PRINT_ANY, "ref", " ref %d", p->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", p->bindcnt);

	if (show_stats) {
		if (tb[TCA_IFE_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_IFE_TM]);

			print_tm(f, tm);
		}
	}

	print_nl();

	return 0;
}

struct action_util ife_action_util = {
	.id = "ife",
	.parse_aopt = parse_ife,
	.print_aopt = print_ife,
};
