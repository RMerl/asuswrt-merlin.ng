/*
 * ipmonitor.c		"ip monitor".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "utils.h"
#include "ip_common.h"

static void usage(void) __attribute__((noreturn));
int prefix_banner;
int listen_all_nsid;

static void usage(void)
{
	fprintf(stderr, "Usage: ip monitor [ all | LISTofOBJECTS ] [ FILE ] "
			"[ label ] [all-nsid] [dev DEVICE]\n");
	fprintf(stderr, "LISTofOBJECTS := link | address | route | mroute | prefix |\n");
	fprintf(stderr, "                 neigh | netconf | rule | nsid\n");
	fprintf(stderr, "FILE := file FILENAME\n");
	exit(-1);
}

static void print_headers(FILE *fp, char *label, struct rtnl_ctrl_data *ctrl)
{
	if (timestamp)
		print_timestamp(fp);

	if (listen_all_nsid) {
		if (ctrl == NULL || ctrl->nsid < 0)
			fprintf(fp, "[nsid current]");
		else
			fprintf(fp, "[nsid %d]", ctrl->nsid);
	}

	if (prefix_banner)
		fprintf(fp, "%s", label);
}

static int accept_msg(const struct sockaddr_nl *who,
		      struct rtnl_ctrl_data *ctrl,
		      struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE*)arg;

	if (n->nlmsg_type == RTM_NEWROUTE || n->nlmsg_type == RTM_DELROUTE) {
		struct rtmsg *r = NLMSG_DATA(n);
		int len = n->nlmsg_len - NLMSG_LENGTH(sizeof(*r));

		if (len < 0) {
			fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
			return -1;
		}

		if (r->rtm_flags & RTM_F_CLONED)
			return 0;

		if (r->rtm_family == RTNL_FAMILY_IPMR ||
		    r->rtm_family == RTNL_FAMILY_IP6MR) {
			print_headers(fp, "[MROUTE]", ctrl);
			print_mroute(who, n, arg);
			return 0;
		} else {
			print_headers(fp, "[ROUTE]", ctrl);
			print_route(who, n, arg);
			return 0;
		}
	}

	if (n->nlmsg_type == RTM_NEWLINK || n->nlmsg_type == RTM_DELLINK) {
		ll_remember_index(who, n, NULL);
		print_headers(fp, "[LINK]", ctrl);
		print_linkinfo(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWADDR || n->nlmsg_type == RTM_DELADDR) {
		print_headers(fp, "[ADDR]", ctrl);
		print_addrinfo(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWADDRLABEL || n->nlmsg_type == RTM_DELADDRLABEL) {
		print_headers(fp, "[ADDRLABEL]", ctrl);
		print_addrlabel(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWNEIGH || n->nlmsg_type == RTM_DELNEIGH ||
	    n->nlmsg_type == RTM_GETNEIGH) {
		if (preferred_family) {
			struct ndmsg *r = NLMSG_DATA(n);

			if (r->ndm_family != preferred_family)
				return 0;
		}

		print_headers(fp, "[NEIGH]", ctrl);
		print_neigh(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWPREFIX) {
		print_headers(fp, "[PREFIX]", ctrl);
		print_prefix(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWRULE || n->nlmsg_type == RTM_DELRULE) {
		print_headers(fp, "[RULE]", ctrl);
		print_rule(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWNETCONF) {
		print_headers(fp, "[NETCONF]", ctrl);
		print_netconf(who, ctrl, n, arg);
		return 0;
	}
	if (n->nlmsg_type == NLMSG_TSTAMP) {
		print_nlmsg_timestamp(fp, n);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWNSID || n->nlmsg_type == RTM_DELNSID) {
		print_headers(fp, "[NSID]", ctrl);
		print_nsid(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type != NLMSG_ERROR && n->nlmsg_type != NLMSG_NOOP &&
	    n->nlmsg_type != NLMSG_DONE) {
		fprintf(fp, "Unknown message: type=0x%08x(%d) flags=0x%08x(%d)"
			"len=0x%08x(%d)\n", n->nlmsg_type, n->nlmsg_type,
			n->nlmsg_flags, n->nlmsg_flags, n->nlmsg_len,
			n->nlmsg_len);
	}
	return 0;
}

int do_ipmonitor(int argc, char **argv)
{
	char *file = NULL;
	unsigned groups = 0;
	int llink=0;
	int laddr=0;
	int lroute=0;
	int lmroute=0;
	int lprefix=0;
	int lneigh=0;
	int lnetconf=0;
	int lrule=0;
	int lnsid=0;
	int ifindex=0;

	groups |= nl_mgrp(RTNLGRP_LINK);
	groups |= nl_mgrp(RTNLGRP_IPV4_IFADDR);
	groups |= nl_mgrp(RTNLGRP_IPV6_IFADDR);
	groups |= nl_mgrp(RTNLGRP_IPV4_ROUTE);
	groups |= nl_mgrp(RTNLGRP_IPV6_ROUTE);
	groups |= nl_mgrp(RTNLGRP_MPLS_ROUTE);
	groups |= nl_mgrp(RTNLGRP_IPV4_MROUTE);
	groups |= nl_mgrp(RTNLGRP_IPV6_MROUTE);
	groups |= nl_mgrp(RTNLGRP_IPV6_PREFIX);
	groups |= nl_mgrp(RTNLGRP_NEIGH);
	groups |= nl_mgrp(RTNLGRP_IPV4_NETCONF);
	groups |= nl_mgrp(RTNLGRP_IPV6_NETCONF);
	groups |= nl_mgrp(RTNLGRP_IPV4_RULE);
	groups |= nl_mgrp(RTNLGRP_IPV6_RULE);
	groups |= nl_mgrp(RTNLGRP_NSID);

	rtnl_close(&rth);

	while (argc > 0) {
		if (matches(*argv, "file") == 0) {
			NEXT_ARG();
			file = *argv;
		} else if (matches(*argv, "label") == 0) {
			prefix_banner = 1;
		} else if (matches(*argv, "all-nsid") == 0) {
			listen_all_nsid = 1;
		} else if (matches(*argv, "link") == 0) {
			llink=1;
			groups = 0;
		} else if (matches(*argv, "address") == 0) {
			laddr=1;
			groups = 0;
		} else if (matches(*argv, "route") == 0) {
			lroute=1;
			groups = 0;
		} else if (matches(*argv, "mroute") == 0) {
			lmroute=1;
			groups = 0;
		} else if (matches(*argv, "prefix") == 0) {
			lprefix=1;
			groups = 0;
		} else if (matches(*argv, "neigh") == 0) {
			lneigh = 1;
			groups = 0;
		} else if (matches(*argv, "netconf") == 0) {
			lnetconf = 1;
			groups = 0;
		} else if (matches(*argv, "rule") == 0) {
			lrule = 1;
			groups = 0;
		} else if (matches(*argv, "nsid") == 0) {
			lnsid = 1;
			groups = 0;
		} else if (strcmp(*argv, "all") == 0) {
			prefix_banner=1;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();

			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				invarg("Device does not exist\n", *argv);
		} else {
			fprintf(stderr, "Argument \"%s\" is unknown, try \"ip monitor help\".\n", *argv);
			exit(-1);
		}
		argc--;	argv++;
	}

	ipaddr_reset_filter(1, ifindex);
	iproute_reset_filter(ifindex);
	ipmroute_reset_filter(ifindex);
	ipneigh_reset_filter(ifindex);
	ipnetconf_reset_filter(ifindex);

	if (llink)
		groups |= nl_mgrp(RTNLGRP_LINK);
	if (laddr) {
		if (!preferred_family || preferred_family == AF_INET)
			groups |= nl_mgrp(RTNLGRP_IPV4_IFADDR);
		if (!preferred_family || preferred_family == AF_INET6)
			groups |= nl_mgrp(RTNLGRP_IPV6_IFADDR);
	}
	if (lroute) {
		if (!preferred_family || preferred_family == AF_INET)
			groups |= nl_mgrp(RTNLGRP_IPV4_ROUTE);
		if (!preferred_family || preferred_family == AF_INET6)
			groups |= nl_mgrp(RTNLGRP_IPV6_ROUTE);
		if (!preferred_family || preferred_family == AF_MPLS)
			groups |= nl_mgrp(RTNLGRP_MPLS_ROUTE);
	}
	if (lmroute) {
		if (!preferred_family || preferred_family == AF_INET)
			groups |= nl_mgrp(RTNLGRP_IPV4_MROUTE);
		if (!preferred_family || preferred_family == AF_INET6)
			groups |= nl_mgrp(RTNLGRP_IPV6_MROUTE);
	}
	if (lprefix) {
		if (!preferred_family || preferred_family == AF_INET6)
			groups |= nl_mgrp(RTNLGRP_IPV6_PREFIX);
	}
	if (lneigh) {
		groups |= nl_mgrp(RTNLGRP_NEIGH);
	}
	if (lnetconf) {
		if (!preferred_family || preferred_family == AF_INET)
			groups |= nl_mgrp(RTNLGRP_IPV4_NETCONF);
		if (!preferred_family || preferred_family == AF_INET6)
			groups |= nl_mgrp(RTNLGRP_IPV6_NETCONF);
	}
	if (lrule) {
		if (!preferred_family || preferred_family == AF_INET)
			groups |= nl_mgrp(RTNLGRP_IPV4_RULE);
		if (!preferred_family || preferred_family == AF_INET6)
			groups |= nl_mgrp(RTNLGRP_IPV6_RULE);
	}
	if (lnsid) {
		groups |= nl_mgrp(RTNLGRP_NSID);
	}
	if (file) {
		FILE *fp;
		fp = fopen(file, "r");
		if (fp == NULL) {
			perror("Cannot fopen");
			exit(-1);
		}
		return rtnl_from_file(fp, accept_msg, stdout);
	}

	if (rtnl_open(&rth, groups) < 0)
		exit(1);
	if (listen_all_nsid && rtnl_listen_all_nsid(&rth) < 0)
		exit(1);

	ll_init_map(&rth);
	netns_map_init();

	if (rtnl_listen(&rth, accept_msg, stdout) < 0)
		exit(2);

	return 0;
}
