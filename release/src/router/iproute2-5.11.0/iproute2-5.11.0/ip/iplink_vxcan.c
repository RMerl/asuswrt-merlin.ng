/*
 * iplink_vxcan.c	vxcan device support (Virtual CAN Tunnel)
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Author:	Oliver Hartkopp <socketcan@hartkopp.net>
 * Based on:	link_veth.c from Pavel Emelianov <xemul@openvz.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <linux/can/vxcan.h>

#include "utils.h"
#include "ip_common.h"

static void print_usage(FILE *f)
{
	printf("Usage: ip link <options> type vxcan [peer <options>]\n"
	       "To get <options> type 'ip link add help'\n");
}

static void usage(void)
{
	print_usage(stderr);
}

static int vxcan_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{
	char *type = NULL;
	int err;
	struct rtattr *data;
	struct ifinfomsg *ifm, *peer_ifm;
	unsigned int ifi_flags, ifi_change, ifi_index;

	if (strcmp(argv[0], "peer") != 0) {
		usage();
		return -1;
	}

	ifm = NLMSG_DATA(n);
	ifi_flags = ifm->ifi_flags;
	ifi_change = ifm->ifi_change;
	ifi_index = ifm->ifi_index;
	ifm->ifi_flags = 0;
	ifm->ifi_change = 0;
	ifm->ifi_index = 0;

	data = addattr_nest(n, 1024, VXCAN_INFO_PEER);

	n->nlmsg_len += sizeof(struct ifinfomsg);

	err = iplink_parse(argc - 1, argv + 1, (struct iplink_req *)n, &type);
	if (err < 0)
		return err;

	if (type)
		duparg("type", argv[err]);

	peer_ifm = RTA_DATA(data);
	peer_ifm->ifi_index = ifm->ifi_index;
	peer_ifm->ifi_flags = ifm->ifi_flags;
	peer_ifm->ifi_change = ifm->ifi_change;
	ifm->ifi_flags = ifi_flags;
	ifm->ifi_change = ifi_change;
	ifm->ifi_index = ifi_index;

	addattr_nest_end(n, data);
	return argc - 1 - err;
}

static void vxcan_print_help(struct link_util *lu, int argc, char **argv,
	FILE *f)
{
	print_usage(f);
}

struct link_util vxcan_link_util = {
	.id = "vxcan",
	.parse_opt = vxcan_parse_opt,
	.print_help = vxcan_print_help,
};
