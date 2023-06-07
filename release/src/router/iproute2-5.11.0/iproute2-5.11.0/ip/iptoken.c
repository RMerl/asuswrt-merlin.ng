/*
 * iptoken.c    "ip token"
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Daniel Borkmann, <borkmann@redhat.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/if.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

extern struct rtnl_handle rth;

struct rtnl_dump_args {
	FILE *fp;
	int ifindex;
};

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: ip token [ list | set | del | get ] [ TOKEN ] [ dev DEV ]\n");
	exit(-1);
}

static int print_token(struct nlmsghdr *n, void *arg)
{
	struct rtnl_dump_args *args = arg;
	FILE *fp = args->fp;
	int ifindex = args->ifindex;
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[IFLA_MAX + 1];
	struct rtattr *ltb[IFLA_INET6_MAX + 1];

	if (n->nlmsg_type != RTM_NEWLINK)
		return -1;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	if (ifi->ifi_family != AF_INET6)
		return 0;
	if (ifi->ifi_index == 0)
		return 0;
	if (ifindex > 0 && ifi->ifi_index != ifindex)
		return 0;
	if (ifi->ifi_flags & (IFF_LOOPBACK | IFF_NOARP))
		return 0;

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
	if (!tb[IFLA_PROTINFO])
		return -1;

	parse_rtattr_nested(ltb, IFLA_INET6_MAX, tb[IFLA_PROTINFO]);
	if (!ltb[IFLA_INET6_TOKEN]) {
		fprintf(stderr, "Seems there's no support for IPv6 token!\n");
		return -1;
	}

	open_json_object(NULL);
	print_string(PRINT_FP, NULL, "token ", NULL);
	print_color_string(PRINT_ANY,
			   ifa_family_color(ifi->ifi_family),
			   "token", "%s",
			   format_host_rta(ifi->ifi_family, ltb[IFLA_INET6_TOKEN]));
	print_string(PRINT_FP, NULL, " dev ", NULL);
	print_color_string(PRINT_ANY, COLOR_IFNAME,
			   "ifname", "%s\n",
			   ll_index_to_name(ifi->ifi_index));
	close_json_object();
	fflush(fp);

	return 0;
}

static int iptoken_list(int argc, char **argv)
{
	int af = AF_INET6;
	struct rtnl_dump_args da = { .fp = stdout };

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if ((da.ifindex = ll_name_to_index(*argv)) == 0)
				invarg("dev is invalid\n", *argv);
			break;
		}
		argc--; argv++;
	}

	if (rtnl_linkdump_req(&rth, af) < 0) {
		perror("Cannot send dump request");
		return -1;
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, print_token, &da) < 0) {
		delete_json_obj();
		fprintf(stderr, "Dump terminated\n");
		return -1;
	}
	delete_json_obj();

	return 0;
}

static int iptoken_set(int argc, char **argv, bool delete)
{
	struct {
		struct nlmsghdr n;
		struct ifinfomsg ifi;
		char buf[512];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_SETLINK,
		.ifi.ifi_family = AF_INET6,
	};
	struct rtattr *afs, *afs6;
	bool have_token = delete, have_dev = false;
	inet_prefix addr = { .bytelen = 16, };

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (!have_dev) {
				if ((req.ifi.ifi_index =
				     ll_name_to_index(*argv)) == 0)
					invarg("dev is invalid\n", *argv);
				have_dev = true;
			}
		} else {
			if (matches(*argv, "help") == 0)
				usage();
			if (!have_token) {
				get_prefix(&addr, *argv, req.ifi.ifi_family);
				have_token = true;
			}
		}
		argc--; argv++;
	}

	if (!have_token) {
		fprintf(stderr, "Not enough information: token is required.\n");
		return -1;
	}
	if (!have_dev) {
		fprintf(stderr, "Not enough information: \"dev\" argument is required.\n");
		return -1;
	}

	afs = addattr_nest(&req.n, sizeof(req), IFLA_AF_SPEC);
	afs6 = addattr_nest(&req.n, sizeof(req), AF_INET6);
	addattr_l(&req.n, sizeof(req), IFLA_INET6_TOKEN,
		  &addr.data, addr.bytelen);
	addattr_nest_end(&req.n, afs6);
	addattr_nest_end(&req.n, afs);

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

int do_iptoken(int argc, char **argv)
{
	ll_init_map(&rth);

	if (argc < 1) {
		return iptoken_list(0, NULL);
	} else if (matches(argv[0], "list") == 0 ||
		   matches(argv[0], "lst") == 0 ||
		   matches(argv[0], "show") == 0) {
		return iptoken_list(argc - 1, argv + 1);
	} else if (matches(argv[0], "set") == 0 ||
		   matches(argv[0], "add") == 0) {
		return iptoken_set(argc - 1, argv + 1, false);
	} else if (matches(argv[0], "delete") == 0) {
		return iptoken_set(argc - 1, argv + 1, true);
	} else if (matches(argv[0], "get") == 0) {
		return iptoken_list(argc - 1, argv + 1);
	} else if (matches(argv[0], "help") == 0)
		usage();

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip token help\".\n", *argv);
	exit(-1);
}
