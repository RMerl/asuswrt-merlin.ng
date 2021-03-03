/*
 * ipnetconf.c		"ip netconf".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Nicolas Dichtel, <nicolas.dichtel@6wind.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

static struct {
	int family;
	int ifindex;
} filter;

static const char * const rp_filter_names[] = {
	"off", "strict", "loose"
};

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: ip netconf show [ dev STRING ]\n");
	exit(-1);
}

static struct rtattr *netconf_rta(struct netconfmsg *ncm)
{
	return (struct rtattr *)((char *)ncm
				 + NLMSG_ALIGN(sizeof(struct netconfmsg)));
}

int print_netconf(struct rtnl_ctrl_data *ctrl, struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;
	struct netconfmsg *ncm = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[NETCONFA_MAX+1];
	int ifindex = 0;

	if (n->nlmsg_type == NLMSG_ERROR)
		return -1;

	if (n->nlmsg_type != RTM_NEWNETCONF &&
	    n->nlmsg_type != RTM_DELNETCONF) {
		fprintf(stderr, "Not a netconf message: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);

		return -1;
	}
	len -= NLMSG_SPACE(sizeof(*ncm));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (filter.family && filter.family != ncm->ncm_family)
		return 0;

	parse_rtattr(tb, NETCONFA_MAX, netconf_rta(ncm),
		     NLMSG_PAYLOAD(n, sizeof(*ncm)));

	if (tb[NETCONFA_IFINDEX])
		ifindex = rta_getattr_u32(tb[NETCONFA_IFINDEX]);

	if (filter.ifindex && filter.ifindex != ifindex)
		return 0;

	open_json_object(NULL);
	if (n->nlmsg_type == RTM_DELNETCONF)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	print_string(PRINT_ANY, "family",
		     "%s ", family_name(ncm->ncm_family));

	if (tb[NETCONFA_IFINDEX]) {
		const char *dev;

		switch (ifindex) {
		case NETCONFA_IFINDEX_ALL:
			dev = "all";
			break;
		case NETCONFA_IFINDEX_DEFAULT:
			dev = "default";
			break;
		default:
			dev = ll_index_to_name(ifindex);
			break;
		}
		print_color_string(PRINT_ANY, COLOR_IFNAME,
				   "interface", "%s ", dev);
	}

	if (tb[NETCONFA_FORWARDING])
		print_on_off(PRINT_ANY, "forwarding", "forwarding %s ",
			     rta_getattr_u32(tb[NETCONFA_FORWARDING]));

	if (tb[NETCONFA_RP_FILTER]) {
		__u32 rp_filter = rta_getattr_u32(tb[NETCONFA_RP_FILTER]);

		if (rp_filter < ARRAY_SIZE(rp_filter_names))
			print_string(PRINT_ANY, "rp_filter",
				     "rp_filter %s ",
				     rp_filter_names[rp_filter]);
		else
			print_uint(PRINT_ANY, "rp_filter",
				   "rp_filter %u ", rp_filter);
	}

	if (tb[NETCONFA_MC_FORWARDING])
		print_on_off(PRINT_ANY, "mc_forwarding", "mc_forwarding %s ",
			     rta_getattr_u32(tb[NETCONFA_MC_FORWARDING]));

	if (tb[NETCONFA_PROXY_NEIGH])
		print_on_off(PRINT_ANY, "proxy_neigh", "proxy_neigh %s ",
			     rta_getattr_u32(tb[NETCONFA_PROXY_NEIGH]));

	if (tb[NETCONFA_IGNORE_ROUTES_WITH_LINKDOWN])
		print_on_off(PRINT_ANY, "ignore_routes_with_linkdown",
			     "ignore_routes_with_linkdown %s ",
			     rta_getattr_u32(tb[NETCONFA_IGNORE_ROUTES_WITH_LINKDOWN]));

	if (tb[NETCONFA_INPUT])
		print_on_off(PRINT_ANY, "input", "input %s ",
			     rta_getattr_u32(tb[NETCONFA_INPUT]));

	close_json_object();
	print_string(PRINT_FP, NULL, "\n", NULL);
	fflush(fp);
	return 0;
}

static int print_netconf2(struct nlmsghdr *n, void *arg)
{
	return print_netconf(NULL, n, arg);
}

void ipnetconf_reset_filter(int ifindex)
{
	memset(&filter, 0, sizeof(filter));
	filter.ifindex = ifindex;
}

static int do_show(int argc, char **argv)
{
	struct {
		struct nlmsghdr		n;
		struct netconfmsg	ncm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct netconfmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
		.n.nlmsg_type = RTM_GETNETCONF,
	};

	ipnetconf_reset_filter(0);
	filter.family = preferred_family;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			filter.ifindex = ll_name_to_index(*argv);
			if (filter.ifindex <= 0) {
				fprintf(stderr,
					"Device \"%s\" does not exist.\n",
					*argv);
				return -1;
			}
		}
		argv++; argc--;
	}

	ll_init_map(&rth);

	if (filter.ifindex && filter.family != AF_UNSPEC) {
		req.ncm.ncm_family = filter.family;
		addattr_l(&req.n, sizeof(req), NETCONFA_IFINDEX,
			  &filter.ifindex, sizeof(filter.ifindex));

		if (rtnl_send(&rth, &req.n, req.n.nlmsg_len) < 0) {
			perror("Can not send request");
			exit(1);
		}
		rtnl_listen(&rth, print_netconf, stdout);
	} else {
		rth.flags = RTNL_HANDLE_F_SUPPRESS_NLERR;
dump:
		if (rtnl_netconfdump_req(&rth, filter.family) < 0) {
			perror("Cannot send dump request");
			exit(1);
		}

		new_json_obj(json);
		if (rtnl_dump_filter(&rth, print_netconf2, stdout) < 0) {
			/* kernel does not support netconf dump on AF_UNSPEC;
			 * fall back to requesting by family
			 */
			if (errno == EOPNOTSUPP &&
			    filter.family == AF_UNSPEC) {
				filter.family = AF_INET;
				goto dump;
			}
			perror("RTNETLINK answers");
			fprintf(stderr, "Dump terminated\n");
			exit(1);
		}
		delete_json_obj();
		if (preferred_family == AF_UNSPEC && filter.family == AF_INET) {
			preferred_family = AF_INET6;
			filter.family = AF_INET6;
			goto dump;
		}
	}
	return 0;
}

int do_ipnetconf(int argc, char **argv)
{
	if (argc > 0) {
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return do_show(argc-1, argv+1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return do_show(0, NULL);

	fprintf(stderr,
		"Command \"%s\" is unknown, try \"ip netconf help\".\n",
		*argv);
	exit(-1);
}
