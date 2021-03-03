// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
/* Copyright (C) 2019 Netronome Systems, Inc. */

#include <linux/if_ether.h>
#include <linux/tc_act/tc_mpls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "rt_names.h"
#include "tc_util.h"

static const char * const action_names[] = {
	[TCA_MPLS_ACT_POP] = "pop",
	[TCA_MPLS_ACT_PUSH] = "push",
	[TCA_MPLS_ACT_MODIFY] = "modify",
	[TCA_MPLS_ACT_DEC_TTL] = "dec_ttl",
	[TCA_MPLS_ACT_MAC_PUSH] = "mac_push",
};

static void explain(void)
{
	fprintf(stderr,
		"Usage: mpls pop [ protocol MPLS_PROTO ] [CONTROL]\n"
		"       mpls push [ protocol MPLS_PROTO ] [ label MPLS_LABEL ] [ tc MPLS_TC ]\n"
		"                 [ ttl MPLS_TTL ] [ bos MPLS_BOS ] [CONTROL]\n"
		"       mpls mac_push [ protocol MPLS_PROTO ] [ label MPLS_LABEL ] [ tc MPLS_TC ]\n"
		"                     [ ttl MPLS_TTL ] [ bos MPLS_BOS ] [CONTROL]\n"
		"       mpls modify [ label MPLS_LABEL ] [ tc MPLS_TC ] [ ttl MPLS_TTL ]\n"
		"                   [ bos MPLS_BOS ] [CONTROL]\n"
		"           for pop, MPLS_PROTO is next header of packet - e.g. ip or mpls_uc\n"
		"           for push and mac_push, MPLS_PROTO is one of mpls_uc or mpls_mc\n"
		"               with default: mpls_uc\n"
		"       CONTROL := reclassify | pipe | drop | continue | pass |\n"
		"                  goto chain <CHAIN_INDEX>\n");
}

static void usage(void)
{
	explain();
	exit(-1);
}

static bool can_modify_mpls_fields(unsigned int action)
{
	return action == TCA_MPLS_ACT_PUSH || action == TCA_MPLS_ACT_MAC_PUSH ||
		action == TCA_MPLS_ACT_MODIFY;
}

static bool can_set_ethtype(unsigned int action)
{
	return action == TCA_MPLS_ACT_PUSH || action == TCA_MPLS_ACT_MAC_PUSH ||
		action == TCA_MPLS_ACT_POP;
}

static bool is_valid_label(__u32 label)
{
	return label <= 0xfffff;
}

static bool check_double_action(unsigned int action, const char *arg)
{
	if (!action)
		return false;

	fprintf(stderr,
		"Error: got \"%s\" but action already set to \"%s\"\n",
		arg, action_names[action]);
	explain();
	return true;
}

static int parse_mpls(struct action_util *a, int *argc_p, char ***argv_p,
		      int tca_id, struct nlmsghdr *n)
{
	struct tc_mpls parm = {};
	__u32 label = 0xffffffff;
	unsigned int action = 0;
	char **argv = *argv_p;
	struct rtattr *tail;
	int argc = *argc_p;
	__u16 proto = 0;
	__u8 bos = 0xff;
	__u8 tc = 0xff;
	__u8 ttl = 0;

	if (matches(*argv, "mpls") != 0)
		return -1;

	NEXT_ARG();

	while (argc > 0) {
		if (matches(*argv, "pop") == 0) {
			if (check_double_action(action, *argv))
				return -1;
			action = TCA_MPLS_ACT_POP;
		} else if (matches(*argv, "push") == 0) {
			if (check_double_action(action, *argv))
				return -1;
			action = TCA_MPLS_ACT_PUSH;
		} else if (matches(*argv, "modify") == 0) {
			if (check_double_action(action, *argv))
				return -1;
			action = TCA_MPLS_ACT_MODIFY;
		} else if (matches(*argv, "mac_push") == 0) {
			if (check_double_action(action, *argv))
				return -1;
			action = TCA_MPLS_ACT_MAC_PUSH;
		} else if (matches(*argv, "dec_ttl") == 0) {
			if (check_double_action(action, *argv))
				return -1;
			action = TCA_MPLS_ACT_DEC_TTL;
		} else if (matches(*argv, "label") == 0) {
			if (!can_modify_mpls_fields(action))
				invarg("only valid for push, mac_push and modify",
				       *argv);
			NEXT_ARG();
			if (get_u32(&label, *argv, 0) || !is_valid_label(label))
				invarg("label must be <=0xFFFFF", *argv);
		} else if (matches(*argv, "tc") == 0) {
			if (!can_modify_mpls_fields(action))
				invarg("only valid for push, mac_push and modify",
				       *argv);
			NEXT_ARG();
			if (get_u8(&tc, *argv, 0) || (tc & ~0x7))
				invarg("tc field is 3 bits max", *argv);
		} else if (matches(*argv, "ttl") == 0) {
			if (!can_modify_mpls_fields(action))
				invarg("only valid for push, mac_push and modify",
				       *argv);
			NEXT_ARG();
			if (get_u8(&ttl, *argv, 0) || !ttl)
				invarg("ttl must be >0 and <=255", *argv);
		} else if (matches(*argv, "bos") == 0) {
			if (!can_modify_mpls_fields(action))
				invarg("only valid for push, mac_push and modify",
				       *argv);
			NEXT_ARG();
			if (get_u8(&bos, *argv, 0) || (bos & ~0x1))
				invarg("bos must be 0 or 1", *argv);
		} else if (matches(*argv, "protocol") == 0) {
			if (!can_set_ethtype(action))
				invarg("only valid for push, mac_push and pop",
				       *argv);
			NEXT_ARG();
			if (ll_proto_a2n(&proto, *argv))
				invarg("protocol is invalid", *argv);
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}

		NEXT_ARG_FWD();
	}

	if (!action)
		incomplete_command();

	parse_action_control_dflt(&argc, &argv, &parm.action,
				  false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&parm.index, *argv, 10))
				invarg("illegal index", *argv);
			NEXT_ARG_FWD();
		}
	}

	if (action == TCA_MPLS_ACT_PUSH && label == 0xffffffff)
		missarg("label");

	if ((action == TCA_MPLS_ACT_PUSH || action == TCA_MPLS_ACT_MAC_PUSH) &&
	    proto &&
	    proto != htons(ETH_P_MPLS_UC) && proto != htons(ETH_P_MPLS_MC)) {
		fprintf(stderr,
			"invalid %spush protocol \"0x%04x\" - use mpls_(uc|mc)\n",
			action == TCA_MPLS_ACT_MAC_PUSH ? "mac_" : "",
			ntohs(proto));
		return -1;
	}

	if (action == TCA_MPLS_ACT_POP && !proto)
		missarg("protocol");

	parm.m_action = action;
	tail = addattr_nest(n, MAX_MSG, tca_id | NLA_F_NESTED);
	addattr_l(n, MAX_MSG, TCA_MPLS_PARMS, &parm, sizeof(parm));
	if (label != 0xffffffff)
		addattr_l(n, MAX_MSG, TCA_MPLS_LABEL, &label, sizeof(label));
	if (proto)
		addattr_l(n, MAX_MSG, TCA_MPLS_PROTO, &proto, sizeof(proto));
	if (tc != 0xff)
		addattr8(n, MAX_MSG, TCA_MPLS_TC, tc);
	if (ttl)
		addattr8(n, MAX_MSG, TCA_MPLS_TTL, ttl);
	if (bos != 0xff)
		addattr8(n, MAX_MSG, TCA_MPLS_BOS, bos);
	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_mpls(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_MPLS_MAX + 1];
	struct tc_mpls *parm;
	SPRINT_BUF(b1);
	__u32 val;

	print_string(PRINT_ANY, "kind", "%s ", "mpls");
	if (!arg)
		return 0;

	parse_rtattr_nested(tb, TCA_MPLS_MAX, arg);

	if (!tb[TCA_MPLS_PARMS]) {
		fprintf(stderr, "[NULL mpls parameters]\n");
		return -1;
	}
	parm = RTA_DATA(tb[TCA_MPLS_PARMS]);

	print_string(PRINT_ANY, "mpls_action", " %s",
		     action_names[parm->m_action]);

	switch (parm->m_action) {
	case TCA_MPLS_ACT_POP:
		if (tb[TCA_MPLS_PROTO]) {
			__u16 proto;

			proto = rta_getattr_u16(tb[TCA_MPLS_PROTO]);
			print_string(PRINT_ANY, "protocol", " protocol %s",
				     ll_proto_n2a(proto, b1, sizeof(b1)));
		}
		break;
	case TCA_MPLS_ACT_PUSH:
	case TCA_MPLS_ACT_MAC_PUSH:
		if (tb[TCA_MPLS_PROTO]) {
			__u16 proto;

			proto = rta_getattr_u16(tb[TCA_MPLS_PROTO]);
			print_string(PRINT_ANY, "protocol", " protocol %s",
				     ll_proto_n2a(proto, b1, sizeof(b1)));
		}
		/* Fallthrough */
	case TCA_MPLS_ACT_MODIFY:
		if (tb[TCA_MPLS_LABEL]) {
			val = rta_getattr_u32(tb[TCA_MPLS_LABEL]);
			print_uint(PRINT_ANY, "label", " label %u", val);
		}
		if (tb[TCA_MPLS_TC]) {
			val = rta_getattr_u8(tb[TCA_MPLS_TC]);
			print_uint(PRINT_ANY, "tc", " tc %u", val);
		}
		if (tb[TCA_MPLS_BOS]) {
			val = rta_getattr_u8(tb[TCA_MPLS_BOS]);
			print_uint(PRINT_ANY, "bos", " bos %u", val);
		}
		if (tb[TCA_MPLS_TTL]) {
			val = rta_getattr_u8(tb[TCA_MPLS_TTL]);
			print_uint(PRINT_ANY, "ttl", " ttl %u", val);
		}
		break;
	}
	print_action_control(f, " ", parm->action, "");

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", parm->index);
	print_int(PRINT_ANY, "ref", " ref %d", parm->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", parm->bindcnt);

	if (show_stats) {
		if (tb[TCA_MPLS_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_MPLS_TM]);

			print_tm(f, tm);
		}
	}

	print_nl();

	return 0;
}

struct action_util mpls_action_util = {
	.id = "mpls",
	.parse_aopt = parse_mpls,
	.print_aopt = print_mpls,
};
