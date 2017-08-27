/*
 * iplink_bridge.c	Bridge device support
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
#include <string.h>
#include <linux/if_link.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

static void print_explain(FILE *f)
{
	fprintf(f,
		"Usage: ... bridge [ forward_delay FORWARD_DELAY ]\n"
		"                  [ hello_time HELLO_TIME ]\n"
		"                  [ max_age MAX_AGE ]\n"
		"                  [ ageing_time AGEING_TIME ]\n"
		"                  [ stp_state STP_STATE ]\n"
		"                  [ priority PRIORITY ]\n"
		"                  [ vlan_filtering VLAN_FILTERING ]\n"
		"                  [ vlan_protocol VLAN_PROTOCOL ]\n"
		"\n"
		"Where: VLAN_PROTOCOL := { 802.1Q | 802.1ad }\n"
	);
}

static void explain(void)
{
	print_explain(stderr);
}

static int bridge_parse_opt(struct link_util *lu, int argc, char **argv,
			    struct nlmsghdr *n)
{
	__u32 val;

	while (argc > 0) {
		if (matches(*argv, "forward_delay") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid forward_delay", *argv);

			addattr32(n, 1024, IFLA_BR_FORWARD_DELAY, val);
		} else if (matches(*argv, "hello_time") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid hello_time", *argv);

			addattr32(n, 1024, IFLA_BR_HELLO_TIME, val);
		} else if (matches(*argv, "max_age") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid max_age", *argv);

			addattr32(n, 1024, IFLA_BR_MAX_AGE, val);
		} else if (matches(*argv, "ageing_time") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid ageing_time", *argv);

			addattr32(n, 1024, IFLA_BR_AGEING_TIME, val);
		} else if (matches(*argv, "stp_state") == 0) {
			NEXT_ARG();
			if (get_u32(&val, *argv, 0))
				invarg("invalid stp_state", *argv);

			addattr32(n, 1024, IFLA_BR_STP_STATE, val);
		} else if (matches(*argv, "priority") == 0) {
			__u16 prio;

			NEXT_ARG();
			if (get_u16(&prio, *argv, 0))
				invarg("invalid priority", *argv);

			addattr16(n, 1024, IFLA_BR_PRIORITY, prio);
		} else if (matches(*argv, "vlan_filtering") == 0) {
			__u8 vlan_filter;

			NEXT_ARG();
			if (get_u8(&vlan_filter, *argv, 0)) {
				invarg("invalid vlan_filtering", *argv);
				return -1;
			}
			addattr8(n, 1024, IFLA_BR_VLAN_FILTERING, vlan_filter);
		} else if (matches(*argv, "vlan_protocol") == 0) {
			__u16 vlan_proto;

			NEXT_ARG();
			if (ll_proto_a2n(&vlan_proto, *argv)) {
				invarg("invalid vlan_protocol", *argv);
				return -1;
			}
			addattr16(n, 1024, IFLA_BR_VLAN_PROTOCOL, vlan_proto);
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "bridge: unknown command \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	return 0;
}

static void bridge_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	if (!tb)
		return;

	if (tb[IFLA_BR_FORWARD_DELAY])
		fprintf(f, "forward_delay %u ",
			rta_getattr_u32(tb[IFLA_BR_FORWARD_DELAY]));

	if (tb[IFLA_BR_HELLO_TIME])
		fprintf(f, "hello_time %u ",
			rta_getattr_u32(tb[IFLA_BR_HELLO_TIME]));

	if (tb[IFLA_BR_MAX_AGE])
		fprintf(f, "max_age %u ",
			rta_getattr_u32(tb[IFLA_BR_MAX_AGE]));

	if (tb[IFLA_BR_AGEING_TIME])
		fprintf(f, "ageing_time %u ",
			rta_getattr_u32(tb[IFLA_BR_AGEING_TIME]));

	if (tb[IFLA_BR_STP_STATE])
		fprintf(f, "stp_state %u ",
			rta_getattr_u32(tb[IFLA_BR_STP_STATE]));

	if (tb[IFLA_BR_PRIORITY])
		fprintf(f, "priority %u ",
			rta_getattr_u16(tb[IFLA_BR_PRIORITY]));

	if (tb[IFLA_BR_VLAN_FILTERING])
		fprintf(f, "vlan_filtering %u ",
			rta_getattr_u8(tb[IFLA_BR_VLAN_FILTERING]));

	if (tb[IFLA_BR_VLAN_PROTOCOL]) {
		SPRINT_BUF(b1);

		fprintf(f, "vlan_protocol %s ",
			ll_proto_n2a(rta_getattr_u16(tb[IFLA_BR_VLAN_PROTOCOL]),
				     b1, sizeof(b1)));
	}
}

static void bridge_print_help(struct link_util *lu, int argc, char **argv,
		FILE *f)
{
	print_explain(f);
}

struct link_util bridge_link_util = {
	.id		= "bridge",
	.maxattr	= IFLA_BR_MAX,
	.parse_opt	= bridge_parse_opt,
	.print_opt	= bridge_print_opt,
	.print_help     = bridge_print_help,
};
