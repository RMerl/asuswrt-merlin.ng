/*
 * iplink_bond_slave.c	Bonding slave device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@resnulli.us>
 */

#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_bonding.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

static void print_explain(FILE *f)
{
	fprintf(f, "Usage: ... bond_slave [ queue_id ID ]\n");
}

static void explain(void)
{
	print_explain(stderr);
}

static const char *slave_states[] = {
	[BOND_STATE_ACTIVE] = "ACTIVE",
	[BOND_STATE_BACKUP] = "BACKUP",
};

static void print_slave_state(FILE *f, struct rtattr *tb)
{
	unsigned int state = rta_getattr_u8(tb);

	if (state >= ARRAY_SIZE(slave_states))
		print_int(PRINT_ANY, "state_index", "state %d ", state);
	else
		print_string(PRINT_ANY,
			     "state",
			     "state %s ",
			     slave_states[state]);
}

static const char *slave_mii_status[] = {
	[BOND_LINK_UP] = "UP",
	[BOND_LINK_FAIL] = "GOING_DOWN",
	[BOND_LINK_DOWN] = "DOWN",
	[BOND_LINK_BACK] = "GOING_BACK",
};

static void print_slave_mii_status(FILE *f, struct rtattr *tb)
{
	unsigned int status = rta_getattr_u8(tb);

	if (status >= ARRAY_SIZE(slave_mii_status))
		print_int(PRINT_ANY,
			  "mii_status_index",
			  "mii_status %d ",
			  status);
	else
		print_string(PRINT_ANY,
			     "mii_status",
			     "mii_status %s ",
			     slave_mii_status[status]);
}

static void print_slave_oper_state(FILE *fp, const char *name, __u16 state)
{
	open_json_array(PRINT_ANY, name);
	print_string(PRINT_FP, NULL, " <", NULL);
#define _PF(s, str) if (state & LACP_STATE_##s) {			\
			state &= ~LACP_STATE_##s;			\
			print_string(PRINT_ANY, NULL,			\
				     state ? "%s," : "%s", str); }
	_PF(LACP_ACTIVITY, "active");
	_PF(LACP_TIMEOUT, "short_timeout");
	_PF(AGGREGATION, "aggregating");
	_PF(SYNCHRONIZATION, "in_sync");
	_PF(COLLECTING, "collecting");
	_PF(DISTRIBUTING, "distributing");
	_PF(DEFAULTED, "defaulted");
	_PF(EXPIRED, "expired");
#undef _PF
	close_json_array(PRINT_ANY, "> ");
}

static void bond_slave_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	SPRINT_BUF(b1);
	if (!tb)
		return;

	if (tb[IFLA_BOND_SLAVE_STATE])
		print_slave_state(f, tb[IFLA_BOND_SLAVE_STATE]);

	if (tb[IFLA_BOND_SLAVE_MII_STATUS])
		print_slave_mii_status(f, tb[IFLA_BOND_SLAVE_MII_STATUS]);

	if (tb[IFLA_BOND_SLAVE_LINK_FAILURE_COUNT])
		print_int(PRINT_ANY,
			  "link_failure_count",
			  "link_failure_count %d ",
			  rta_getattr_u32(tb[IFLA_BOND_SLAVE_LINK_FAILURE_COUNT]));

	if (tb[IFLA_BOND_SLAVE_PERM_HWADDR])
		print_string(PRINT_ANY,
			     "perm_hwaddr",
			     "perm_hwaddr %s ",
			     ll_addr_n2a(RTA_DATA(tb[IFLA_BOND_SLAVE_PERM_HWADDR]),
					 RTA_PAYLOAD(tb[IFLA_BOND_SLAVE_PERM_HWADDR]),
					 0, b1, sizeof(b1)));

	if (tb[IFLA_BOND_SLAVE_QUEUE_ID])
		print_int(PRINT_ANY,
			  "queue_id",
			  "queue_id %d ",
			  rta_getattr_u16(tb[IFLA_BOND_SLAVE_QUEUE_ID]));

	if (tb[IFLA_BOND_SLAVE_AD_AGGREGATOR_ID])
		print_int(PRINT_ANY,
			  "ad_aggregator_id",
			  "ad_aggregator_id %d ",
			  rta_getattr_u16(tb[IFLA_BOND_SLAVE_AD_AGGREGATOR_ID]));

	if (tb[IFLA_BOND_SLAVE_AD_ACTOR_OPER_PORT_STATE]) {
		__u8 state = rta_getattr_u8(tb[IFLA_BOND_SLAVE_AD_ACTOR_OPER_PORT_STATE]);

		print_int(PRINT_ANY,
			  "ad_actor_oper_port_state",
			  "ad_actor_oper_port_state %d ",
			  state);
		print_slave_oper_state(f, "ad_actor_oper_port_state_str", state);
	}

	if (tb[IFLA_BOND_SLAVE_AD_PARTNER_OPER_PORT_STATE]) {
		__u16 state = rta_getattr_u8(tb[IFLA_BOND_SLAVE_AD_PARTNER_OPER_PORT_STATE]);

		print_int(PRINT_ANY,
			  "ad_partner_oper_port_state",
			  "ad_partner_oper_port_state %d ",
			  state);
		print_slave_oper_state(f, "ad_partner_oper_port_state_str", state);
	}
}

static int bond_slave_parse_opt(struct link_util *lu, int argc, char **argv,
				struct nlmsghdr *n)
{
	__u16 queue_id;

	while (argc > 0) {
		if (matches(*argv, "queue_id") == 0) {
			NEXT_ARG();
			if (get_u16(&queue_id, *argv, 0))
				invarg("queue_id is invalid", *argv);
			addattr16(n, 1024, IFLA_BOND_SLAVE_QUEUE_ID, queue_id);
		} else {
			if (matches(*argv, "help") != 0)
				fprintf(stderr,
					"bond_slave: unknown option \"%s\"?\n",
					*argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	return 0;
}

static void bond_slave_print_help(struct link_util *lu, int argc, char **argv,
				  FILE *f)
{
	print_explain(f);
}

struct link_util bond_slave_link_util = {
	.id		= "bond_slave",
	.maxattr	= IFLA_BOND_SLAVE_MAX,
	.print_opt	= bond_slave_print_opt,
	.parse_opt	= bond_slave_parse_opt,
	.print_help	= bond_slave_print_help,
	.parse_ifla_xstats = bond_parse_xstats,
	.print_ifla_xstats = bond_print_xstats,
};
