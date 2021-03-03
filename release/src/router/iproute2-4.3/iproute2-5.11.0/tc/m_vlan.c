/*
 * m_vlan.c		vlan manipulation module
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@resnulli.us>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/if_ether.h>
#include "utils.h"
#include "rt_names.h"
#include "tc_util.h"
#include <linux/tc_act/tc_vlan.h>

static const char * const action_names[] = {
	[TCA_VLAN_ACT_POP] = "pop",
	[TCA_VLAN_ACT_PUSH] = "push",
	[TCA_VLAN_ACT_MODIFY] = "modify",
	[TCA_VLAN_ACT_POP_ETH] = "pop_eth",
	[TCA_VLAN_ACT_PUSH_ETH] = "push_eth",
};

static void explain(void)
{
	fprintf(stderr,
		"Usage: vlan pop [CONTROL]\n"
		"       vlan push [ protocol VLANPROTO ] id VLANID [ priority VLANPRIO ] [CONTROL]\n"
		"       vlan modify [ protocol VLANPROTO ] id VLANID [ priority VLANPRIO ] [CONTROL]\n"
		"       vlan pop_eth [CONTROL]\n"
		"       vlan push_eth dst_mac LLADDR src_mac LLADDR [CONTROL]\n"
		"       VLANPROTO is one of 802.1Q or 802.1AD\n"
		"            with default: 802.1Q\n"
		"       CONTROL := reclassify | pipe | drop | continue | pass |\n"
		"                  goto chain <CHAIN_INDEX>\n");
}

static void usage(void)
{
	explain();
	exit(-1);
}

static bool has_push_attribs(int action)
{
	return action == TCA_VLAN_ACT_PUSH || action == TCA_VLAN_ACT_MODIFY;
}

static void unexpected(const char *arg)
{
	fprintf(stderr,
		"unexpected \"%s\" - action already specified\n",
		arg);
	explain();
}

static int parse_vlan(struct action_util *a, int *argc_p, char ***argv_p,
		      int tca_id, struct nlmsghdr *n)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	struct rtattr *tail;
	int action = 0;
	char dst_mac[ETH_ALEN] = {};
	int dst_mac_set = 0;
	char src_mac[ETH_ALEN] = {};
	int src_mac_set = 0;
	__u16 id;
	int id_set = 0;
	__u16 proto;
	int proto_set = 0;
	__u8 prio;
	int prio_set = 0;
	struct tc_vlan parm = {};

	if (matches(*argv, "vlan") != 0)
		return -1;

	NEXT_ARG();

	while (argc > 0) {
		if (matches(*argv, "pop") == 0) {
			if (action) {
				unexpected(*argv);
				return -1;
			}
			action = TCA_VLAN_ACT_POP;
		} else if (matches(*argv, "push") == 0) {
			if (action) {
				unexpected(*argv);
				return -1;
			}
			action = TCA_VLAN_ACT_PUSH;
		} else if (matches(*argv, "modify") == 0) {
			if (action) {
				unexpected(*argv);
				return -1;
			}
			action = TCA_VLAN_ACT_MODIFY;
		} else if (matches(*argv, "pop_eth") == 0) {
			if (action) {
				unexpected(*argv);
				return -1;
			}
			action = TCA_VLAN_ACT_POP_ETH;
		} else if (matches(*argv, "push_eth") == 0) {
			if (action) {
				unexpected(*argv);
				return -1;
			}
			action = TCA_VLAN_ACT_PUSH_ETH;
		} else if (matches(*argv, "id") == 0) {
			if (!has_push_attribs(action))
				invarg("only valid for push/modify", *argv);

			NEXT_ARG();
			if (get_u16(&id, *argv, 0))
				invarg("id is invalid", *argv);
			id_set = 1;
		} else if (matches(*argv, "protocol") == 0) {
			if (!has_push_attribs(action))
				invarg("only valid for push/modify", *argv);

			NEXT_ARG();
			if (ll_proto_a2n(&proto, *argv))
				invarg("protocol is invalid", *argv);
			proto_set = 1;
		} else if (matches(*argv, "priority") == 0) {
			if (!has_push_attribs(action))
				invarg("only valid for push/modify", *argv);

			NEXT_ARG();
			if (get_u8(&prio, *argv, 0) || (prio & ~0x7))
				invarg("prio is invalid", *argv);
			prio_set = 1;
		} else if (matches(*argv, "dst_mac") == 0) {
			if (action != TCA_VLAN_ACT_PUSH_ETH)
				invarg("only valid for push_eth", *argv);

			NEXT_ARG();
			if (ll_addr_a2n(dst_mac, sizeof(dst_mac), *argv) < 0)
				invarg("dst_mac is invalid", *argv);
			dst_mac_set = 1;
		} else if (matches(*argv, "src_mac") == 0) {
			if (action != TCA_VLAN_ACT_PUSH_ETH)
				invarg("only valid for push_eth", *argv);

			NEXT_ARG();
			if (ll_addr_a2n(src_mac, sizeof(src_mac), *argv) < 0)
				invarg("src_mac is invalid", *argv);
			src_mac_set = 1;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			break;
		}
		argc--;
		argv++;
	}

	parse_action_control_dflt(&argc, &argv, &parm.action,
				  false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&parm.index, *argv, 10)) {
				fprintf(stderr, "vlan: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
		}
	}

	if (has_push_attribs(action) && !id_set) {
		fprintf(stderr, "id needs to be set for %s\n",
			action_names[action]);
		explain();
		return -1;
	}

	if (action == TCA_VLAN_ACT_PUSH_ETH) {
		if (!dst_mac_set) {
			fprintf(stderr, "dst_mac needs to be set for %s\n",
				action_names[action]);
			explain();
			return -1;
		} else if (!src_mac_set) {
			fprintf(stderr, "src_mac needs to be set for %s\n",
				action_names[action]);
			explain();
			return -1;
		}
	}

	parm.v_action = action;
	tail = addattr_nest(n, MAX_MSG, tca_id);
	addattr_l(n, MAX_MSG, TCA_VLAN_PARMS, &parm, sizeof(parm));
	if (id_set)
		addattr_l(n, MAX_MSG, TCA_VLAN_PUSH_VLAN_ID, &id, 2);
	if (proto_set) {
		if (proto != htons(ETH_P_8021Q) &&
		    proto != htons(ETH_P_8021AD)) {
			fprintf(stderr, "protocol not supported\n");
			explain();
			return -1;
		}

		addattr_l(n, MAX_MSG, TCA_VLAN_PUSH_VLAN_PROTOCOL, &proto, 2);
	}
	if (prio_set)
		addattr8(n, MAX_MSG, TCA_VLAN_PUSH_VLAN_PRIORITY, prio);
	if (dst_mac_set)
		addattr_l(n, MAX_MSG, TCA_VLAN_PUSH_ETH_DST, dst_mac,
			  sizeof(dst_mac));
	if (src_mac_set)
		addattr_l(n, MAX_MSG, TCA_VLAN_PUSH_ETH_SRC, src_mac,
			  sizeof(src_mac));

	addattr_nest_end(n, tail);

	*argc_p = argc;
	*argv_p = argv;
	return 0;
}

static int print_vlan(struct action_util *au, FILE *f, struct rtattr *arg)
{
	SPRINT_BUF(b1);
	struct rtattr *tb[TCA_VLAN_MAX + 1];
	__u16 val;
	struct tc_vlan *parm;

	print_string(PRINT_ANY, "kind", "%s ", "vlan");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_VLAN_MAX, arg);

	if (!tb[TCA_VLAN_PARMS]) {
		fprintf(stderr, "Missing vlan parameters\n");
		return -1;
	}
	parm = RTA_DATA(tb[TCA_VLAN_PARMS]);

	print_string(PRINT_ANY, "vlan_action", " %s",
		     action_names[parm->v_action]);

	switch (parm->v_action) {
	case TCA_VLAN_ACT_PUSH:
	case TCA_VLAN_ACT_MODIFY:
		if (tb[TCA_VLAN_PUSH_VLAN_ID]) {
			val = rta_getattr_u16(tb[TCA_VLAN_PUSH_VLAN_ID]);
			print_uint(PRINT_ANY, "id", " id %u", val);
		}
		if (tb[TCA_VLAN_PUSH_VLAN_PROTOCOL]) {
			__u16 proto;

			proto = rta_getattr_u16(tb[TCA_VLAN_PUSH_VLAN_PROTOCOL]);
			print_string(PRINT_ANY, "protocol", " protocol %s",
				     ll_proto_n2a(proto, b1, sizeof(b1)));
		}
		if (tb[TCA_VLAN_PUSH_VLAN_PRIORITY]) {
			val = rta_getattr_u8(tb[TCA_VLAN_PUSH_VLAN_PRIORITY]);
			print_uint(PRINT_ANY, "priority", " priority %u", val);
		}
		break;
	case TCA_VLAN_ACT_PUSH_ETH:
		if (tb[TCA_VLAN_PUSH_ETH_DST] &&
		    RTA_PAYLOAD(tb[TCA_VLAN_PUSH_ETH_DST]) == ETH_ALEN) {
			ll_addr_n2a(RTA_DATA(tb[TCA_VLAN_PUSH_ETH_DST]),
				    ETH_ALEN, 0, b1, sizeof(b1));
			print_string(PRINT_ANY, "dst_mac", " dst_mac %s", b1);
		}
		if (tb[TCA_VLAN_PUSH_ETH_SRC &&
		       RTA_PAYLOAD(tb[TCA_VLAN_PUSH_ETH_SRC]) == ETH_ALEN]) {
			ll_addr_n2a(RTA_DATA(tb[TCA_VLAN_PUSH_ETH_SRC]),
				    ETH_ALEN, 0, b1, sizeof(b1));
			print_string(PRINT_ANY, "src_mac", " src_mac %s", b1);
		}
	}
	print_action_control(f, " ", parm->action, "");

	print_nl();
	print_uint(PRINT_ANY, "index", "\t index %u", parm->index);
	print_int(PRINT_ANY, "ref", " ref %d", parm->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", parm->bindcnt);

	if (show_stats) {
		if (tb[TCA_VLAN_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_VLAN_TM]);

			print_tm(f, tm);
		}
	}

	print_nl();

	return 0;
}

struct action_util vlan_action_util = {
	.id = "vlan",
	.parse_aopt = parse_vlan,
	.print_aopt = print_vlan,
};
