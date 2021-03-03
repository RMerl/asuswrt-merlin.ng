/*
 * ipmroute.c		"ip mroute".
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
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>

#include <rt_names.h>
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip mroute show [ [ to ] PREFIX ] [ from PREFIX ] [ iif DEVICE ]\n"
	"			[ table TABLE_ID ]\n"
	"TABLE_ID := [ local | main | default | all | NUMBER ]\n"
#if 0
	"Usage: ip mroute [ add | del ] DESTINATION from SOURCE [ iif DEVICE ] [ oif DEVICE ]\n"
#endif
	);
	exit(-1);
}

static struct rtfilter {
	int tb;
	int af;
	int iif;
	inet_prefix mdst;
	inet_prefix msrc;
} filter;

int print_mroute(struct nlmsghdr *n, void *arg)
{
	struct rtmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[RTA_MAX+1];
	FILE *fp = arg;
	const char *src, *dst;
	SPRINT_BUF(b1);
	SPRINT_BUF(b2);
	__u32 table;
	int iif = 0;
	int family;

	if ((n->nlmsg_type != RTM_NEWROUTE &&
	     n->nlmsg_type != RTM_DELROUTE)) {
		fprintf(stderr, "Not a multicast route: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return 0;
	}
	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (r->rtm_type != RTN_MULTICAST) {
		fprintf(stderr,
			"Non multicast route received, kernel does support IP multicast?\n");
		return -1;
	}

	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
	table = rtm_get_table(r, tb);

	if (filter.tb > 0 && filter.tb != table)
		return 0;

	if (tb[RTA_IIF])
		iif = rta_getattr_u32(tb[RTA_IIF]);
	if (filter.iif && filter.iif != iif)
		return 0;

	if (filter.af && filter.af != r->rtm_family)
		return 0;

	if (inet_addr_match_rta(&filter.mdst, tb[RTA_DST]))
		return 0;

	if (inet_addr_match_rta(&filter.msrc, tb[RTA_SRC]))
		return 0;

	family = get_real_family(r->rtm_type, r->rtm_family);

	open_json_object(NULL);
	if (n->nlmsg_type == RTM_DELROUTE)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	if (tb[RTA_SRC])
		src = rt_addr_n2a_r(family, RTA_PAYLOAD(tb[RTA_SRC]),
				    RTA_DATA(tb[RTA_SRC]), b1, sizeof(b1));
	else
		src = "unknown";

	if (tb[RTA_DST])
		dst = rt_addr_n2a_r(family, RTA_PAYLOAD(tb[RTA_DST]),
				    RTA_DATA(tb[RTA_DST]), b2, sizeof(b2));
	else
		dst = "unknown";

	if (is_json_context()) {
		print_string(PRINT_JSON, "src", NULL, src);
		print_string(PRINT_JSON, "dst", NULL, dst);
	} else {
		char obuf[256];

		snprintf(obuf, sizeof(obuf), "(%s,%s)", src, dst);
		print_string(PRINT_FP, NULL,
			     "%-32s Iif: ", obuf);
	}

	if (iif)
		print_color_string(PRINT_ANY, COLOR_IFNAME,
				   "iif", "%-10s ", ll_index_to_name(iif));
	else
		print_string(PRINT_ANY,"iif", "%s ", "unresolved");

	if (tb[RTA_MULTIPATH]) {
		struct rtnexthop *nh = RTA_DATA(tb[RTA_MULTIPATH]);
		int first = 1;

		open_json_array(PRINT_JSON, "multipath");
		len = RTA_PAYLOAD(tb[RTA_MULTIPATH]);

		for (;;) {
			if (len < sizeof(*nh))
				break;
			if (nh->rtnh_len > len)
				break;

			open_json_object(NULL);
			if (first) {
				print_string(PRINT_FP, NULL, "Oifs: ", NULL);
				first = 0;
			}

			print_color_string(PRINT_ANY, COLOR_IFNAME,
					   "oif", "%s", ll_index_to_name(nh->rtnh_ifindex));

			if (nh->rtnh_hops > 1)
				print_uint(PRINT_ANY,
					   "ttl", "(ttl %u) ", nh->rtnh_hops);
			else
				print_string(PRINT_FP, NULL, " ", NULL);

			close_json_object();
			len -= NLMSG_ALIGN(nh->rtnh_len);
			nh = RTNH_NEXT(nh);
		}
		close_json_array(PRINT_JSON, NULL);
	}

	print_string(PRINT_ANY, "state", " State: %s",
		     (r->rtm_flags & RTNH_F_UNRESOLVED) ? "unresolved" : "resolved");

	if (r->rtm_flags & RTNH_F_OFFLOAD)
		print_null(PRINT_ANY, "offload", " offload", NULL);

	if (show_stats && tb[RTA_MFC_STATS]) {
		struct rta_mfc_stats *mfcs = RTA_DATA(tb[RTA_MFC_STATS]);

		print_nl();
		print_u64(PRINT_ANY, "packets", "  %"PRIu64" packets,",
			   mfcs->mfcs_packets);
		print_u64(PRINT_ANY, "bytes", " %"PRIu64" bytes", mfcs->mfcs_bytes);

		if (mfcs->mfcs_wrong_if)
			print_u64(PRINT_ANY, "wrong_if",
				   ", %"PRIu64" arrived on wrong iif.",
				   mfcs->mfcs_wrong_if);
	}

	if (show_stats && tb[RTA_EXPIRES]) {
		struct timeval tv;
		double age;

		__jiffies_to_tv(&tv, rta_getattr_u64(tb[RTA_EXPIRES]));
		age = tv.tv_sec;
		age += tv.tv_usec / 1000000.;
		print_float(PRINT_ANY, "expires",
			    ", Age %.2f", age);
	}

	if (table && (table != RT_TABLE_MAIN || show_details > 0) && !filter.tb)
		print_string(PRINT_ANY, "table", " Table: %s",
			     rtnl_rttable_n2a(table, b1, sizeof(b1)));

	print_string(PRINT_FP, NULL, "\n", NULL);
	close_json_object();
	fflush(fp);
	return 0;
}

void ipmroute_reset_filter(int ifindex)
{
	memset(&filter, 0, sizeof(filter));
	filter.mdst.bitlen = -1;
	filter.msrc.bitlen = -1;
	filter.iif = ifindex;
}

static int iproute_dump_filter(struct nlmsghdr *nlh, int reqlen)
{
	int err;

	if (filter.tb) {
		err = addattr32(nlh, reqlen, RTA_TABLE, filter.tb);
		if (err)
			return err;
	}

	return 0;
}

static int mroute_list(int argc, char **argv)
{
	char *id = NULL;
	int family = preferred_family;

	ipmroute_reset_filter(0);
	if (family == AF_INET || family == AF_UNSPEC) {
		family = RTNL_FAMILY_IPMR;
		filter.af = RTNL_FAMILY_IPMR;
		filter.tb = RT_TABLE_DEFAULT;  /* for backward compatibility */
	} else if (family == AF_INET6) {
		family = RTNL_FAMILY_IP6MR;
		filter.af = RTNL_FAMILY_IP6MR;
	} else {
		/* family does not have multicast routing */
		return 0;
	}

	filter.msrc.family = filter.mdst.family = family;

	while (argc > 0) {
		if (matches(*argv, "table") == 0) {
			__u32 tid;

			NEXT_ARG();
			if (rtnl_rttable_a2n(&tid, *argv)) {
				if (strcmp(*argv, "all") == 0) {
					filter.tb = 0;
				} else if (strcmp(*argv, "help") == 0) {
					usage();
				} else {
					invarg("table id value is invalid\n", *argv);
				}
			} else
				filter.tb = tid;
		} else if (strcmp(*argv, "iif") == 0) {
			NEXT_ARG();
			id = *argv;
		} else if (matches(*argv, "from") == 0) {
			NEXT_ARG();
			if (get_prefix(&filter.msrc, *argv, family))
				invarg("from value is invalid\n", *argv);
		} else {
			if (strcmp(*argv, "to") == 0) {
				NEXT_ARG();
			}
			if (matches(*argv, "help") == 0)
				usage();
			if (get_prefix(&filter.mdst, *argv, family))
				invarg("to value is invalid\n", *argv);
		}
		argc--; argv++;
	}

	ll_init_map(&rth);

	if (id)  {
		int idx;

		idx = ll_name_to_index(id);
		if (!idx)
			return nodev(id);
		filter.iif = idx;
	}

	if (rtnl_routedump_req(&rth, filter.af, iproute_dump_filter) < 0) {
		perror("Cannot send dump request");
		return 1;
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, print_mroute, stdout) < 0) {
		delete_json_obj();
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	delete_json_obj();

	return 0;
}

int do_multiroute(int argc, char **argv)
{
	if (argc < 1)
		return mroute_list(0, NULL);
#if 0
	if (matches(*argv, "add") == 0)
		return mroute_modify(RTM_NEWADDR, argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return mroute_modify(RTM_DELADDR, argc-1, argv+1);
	if (matches(*argv, "get") == 0)
		return mroute_get(argc-1, argv+1);
#endif
	if (matches(*argv, "list") == 0 || matches(*argv, "show") == 0
	    || matches(*argv, "lst") == 0)
		return mroute_list(argc-1, argv+1);
	if (matches(*argv, "help") == 0)
		usage();
	fprintf(stderr, "Command \"%s\" is unknown, try \"ip mroute help\".\n", *argv);
	exit(-1);
}
