/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Get mdb table with netlink
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>
#include <linux/if_ether.h>
#include <string.h>
#include <arpa/inet.h>

#include "libnetlink.h"
#include "br_common.h"
#include "rt_names.h"
#include "utils.h"
#include "json_print.h"

#ifndef MDBA_RTA
#define MDBA_RTA(r) \
	((struct rtattr *)(((char *)(r)) + NLMSG_ALIGN(sizeof(struct br_port_msg))))
#endif

static unsigned int filter_index, filter_vlan;

static void usage(void)
{
	fprintf(stderr,
		"Usage: bridge mdb { add | del } dev DEV port PORT grp GROUP [src SOURCE] [permanent | temp] [vid VID]\n"
		"       bridge mdb {show} [ dev DEV ] [ vid VID ]\n");
	exit(-1);
}

static bool is_temp_mcast_rtr(__u8 type)
{
	return type == MDB_RTR_TYPE_TEMP_QUERY || type == MDB_RTR_TYPE_TEMP;
}

static const char *format_timer(__u32 ticks, int align)
{
	struct timeval tv;
	static char tbuf[32];

	__jiffies_to_tv(&tv, ticks);
	if (align)
		snprintf(tbuf, sizeof(tbuf), "%4lu.%.2lu",
			 (unsigned long)tv.tv_sec,
			 (unsigned long)tv.tv_usec / 10000);
	else
		snprintf(tbuf, sizeof(tbuf), "%lu.%.2lu",
			 (unsigned long)tv.tv_sec,
			 (unsigned long)tv.tv_usec / 10000);

	return tbuf;
}

static void __print_router_port_stats(FILE *f, struct rtattr *pattr)
{
	struct rtattr *tb[MDBA_ROUTER_PATTR_MAX + 1];

	parse_rtattr(tb, MDBA_ROUTER_PATTR_MAX, MDB_RTR_RTA(RTA_DATA(pattr)),
		     RTA_PAYLOAD(pattr) - RTA_ALIGN(sizeof(uint32_t)));

	if (tb[MDBA_ROUTER_PATTR_TIMER]) {
		__u32 timer = rta_getattr_u32(tb[MDBA_ROUTER_PATTR_TIMER]);

		print_string(PRINT_ANY, "timer", " %s",
			     format_timer(timer, 1));
	}

	if (tb[MDBA_ROUTER_PATTR_TYPE]) {
		__u8 type = rta_getattr_u8(tb[MDBA_ROUTER_PATTR_TYPE]);

		print_string(PRINT_ANY, "type", " %s",
			     is_temp_mcast_rtr(type) ? "temp" : "permanent");
	}
}

static void br_print_router_ports(FILE *f, struct rtattr *attr,
				  const char *brifname)
{
	int rem = RTA_PAYLOAD(attr);
	struct rtattr *i;

	if (is_json_context())
		open_json_array(PRINT_JSON, brifname);
	else if (!show_stats)
		fprintf(f, "router ports on %s: ", brifname);

	for (i = RTA_DATA(attr); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		uint32_t *port_ifindex = RTA_DATA(i);
		const char *port_ifname = ll_index_to_name(*port_ifindex);

		if (is_json_context()) {
			open_json_object(NULL);
			print_string(PRINT_JSON, "port", NULL, port_ifname);

			if (show_stats)
				__print_router_port_stats(f, i);
			close_json_object();
		} else if (show_stats) {
			fprintf(f, "router ports on %s: %s",
				brifname, port_ifname);

			__print_router_port_stats(f, i);
			fprintf(f, "\n");
		} else {
			fprintf(f, "%s ", port_ifname);
		}
	}

	if (!show_stats)
		print_nl();

	close_json_array(PRINT_JSON, NULL);
}

static void print_src_entry(struct rtattr *src_attr, int af, const char *sep)
{
	struct rtattr *stb[MDBA_MDB_SRCATTR_MAX + 1];
	SPRINT_BUF(abuf);
	const char *addr;
	__u32 timer_val;

	parse_rtattr_nested(stb, MDBA_MDB_SRCATTR_MAX, src_attr);
	if (!stb[MDBA_MDB_SRCATTR_ADDRESS] || !stb[MDBA_MDB_SRCATTR_TIMER])
		return;

	addr = inet_ntop(af, RTA_DATA(stb[MDBA_MDB_SRCATTR_ADDRESS]), abuf,
			 sizeof(abuf));
	if (!addr)
		return;
	timer_val = rta_getattr_u32(stb[MDBA_MDB_SRCATTR_TIMER]);

	open_json_object(NULL);
	print_string(PRINT_FP, NULL, "%s", sep);
	print_color_string(PRINT_ANY, ifa_family_color(af),
			   "address", "%s", addr);
	print_string(PRINT_ANY, "timer", "/%s", format_timer(timer_val, 0));
	close_json_object();
}

static void print_mdb_entry(FILE *f, int ifindex, const struct br_mdb_entry *e,
			    struct nlmsghdr *n, struct rtattr **tb)
{
	const void *grp, *src;
	const char *addr;
	SPRINT_BUF(abuf);
	const char *dev;
	int af;

	if (filter_vlan && e->vid != filter_vlan)
		return;

	if (!e->addr.proto) {
		af = AF_PACKET;
		grp = &e->addr.u.mac_addr;
	} else if (e->addr.proto == htons(ETH_P_IP)) {
		af = AF_INET;
		grp = &e->addr.u.ip4;
	} else {
		af = AF_INET6;
		grp = &e->addr.u.ip6;
	}
	dev = ll_index_to_name(ifindex);

	open_json_object(NULL);

	print_int(PRINT_JSON, "index", NULL, ifindex);
	print_color_string(PRINT_ANY, COLOR_IFNAME, "dev", "dev %s", dev);
	print_string(PRINT_ANY, "port", " port %s",
		     ll_index_to_name(e->ifindex));

	/* The ETH_ALEN argument is ignored for all cases but AF_PACKET */
	addr = rt_addr_n2a_r(af, ETH_ALEN, grp, abuf, sizeof(abuf));
	if (!addr)
		return;

	print_color_string(PRINT_ANY, ifa_family_color(af),
			    "grp", " grp %s", addr);

	if (tb && tb[MDBA_MDB_EATTR_SOURCE]) {
		src = (const void *)RTA_DATA(tb[MDBA_MDB_EATTR_SOURCE]);
		print_color_string(PRINT_ANY, ifa_family_color(af),
				   "src", " src %s",
				   inet_ntop(af, src, abuf, sizeof(abuf)));
	}
	print_string(PRINT_ANY, "state", " %s",
			   (e->state & MDB_PERMANENT) ? "permanent" : "temp");
	if (show_details && tb) {
		if (tb[MDBA_MDB_EATTR_GROUP_MODE]) {
			__u8 mode = rta_getattr_u8(tb[MDBA_MDB_EATTR_GROUP_MODE]);

			print_string(PRINT_ANY, "filter_mode", " filter_mode %s",
				     mode == MCAST_INCLUDE ? "include" :
							     "exclude");
		}
		if (tb[MDBA_MDB_EATTR_SRC_LIST]) {
			struct rtattr *i, *attr = tb[MDBA_MDB_EATTR_SRC_LIST];
			const char *sep = " ";
			int rem;

			open_json_array(PRINT_ANY, is_json_context() ?
								"source_list" :
								" source_list");
			rem = RTA_PAYLOAD(attr);
			for (i = RTA_DATA(attr); RTA_OK(i, rem);
			     i = RTA_NEXT(i, rem)) {
				print_src_entry(i, af, sep);
				sep = ",";
			}
			close_json_array(PRINT_JSON, NULL);
		}
		if (tb[MDBA_MDB_EATTR_RTPROT]) {
			__u8 rtprot = rta_getattr_u8(tb[MDBA_MDB_EATTR_RTPROT]);
			SPRINT_BUF(rtb);

			print_string(PRINT_ANY, "protocol", " proto %s ",
				     rtnl_rtprot_n2a(rtprot, rtb, sizeof(rtb)));
		}
	}

	open_json_array(PRINT_JSON, "flags");
	if (e->flags & MDB_FLAGS_OFFLOAD)
		print_string(PRINT_ANY, NULL, " %s", "offload");
	if (e->flags & MDB_FLAGS_FAST_LEAVE)
		print_string(PRINT_ANY, NULL, " %s", "fast_leave");
	if (e->flags & MDB_FLAGS_STAR_EXCL)
		print_string(PRINT_ANY, NULL, " %s", "added_by_star_ex");
	if (e->flags & MDB_FLAGS_BLOCKED)
		print_string(PRINT_ANY, NULL, " %s", "blocked");
	close_json_array(PRINT_JSON, NULL);

	if (e->vid)
		print_uint(PRINT_ANY, "vid", " vid %u", e->vid);

	if (show_stats && tb && tb[MDBA_MDB_EATTR_TIMER]) {
		__u32 timer = rta_getattr_u32(tb[MDBA_MDB_EATTR_TIMER]);

		print_string(PRINT_ANY, "timer", " %s",
			     format_timer(timer, 1));
	}

	print_nl();
	close_json_object();
}

static void br_print_mdb_entry(FILE *f, int ifindex, struct rtattr *attr,
			       struct nlmsghdr *n)
{
	struct rtattr *etb[MDBA_MDB_EATTR_MAX + 1];
	struct br_mdb_entry *e;
	struct rtattr *i;
	int rem;

	rem = RTA_PAYLOAD(attr);
	for (i = RTA_DATA(attr); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		e = RTA_DATA(i);
		parse_rtattr_flags(etb, MDBA_MDB_EATTR_MAX, MDB_RTA(RTA_DATA(i)),
				   RTA_PAYLOAD(i) - RTA_ALIGN(sizeof(*e)),
				   NLA_F_NESTED);
		print_mdb_entry(f, ifindex, e, n, etb);
	}
}

static void print_mdb_entries(FILE *fp, struct nlmsghdr *n,
			      int ifindex,  struct rtattr *mdb)
{
	int rem = RTA_PAYLOAD(mdb);
	struct rtattr *i;

	for (i = RTA_DATA(mdb); RTA_OK(i, rem); i = RTA_NEXT(i, rem))
		br_print_mdb_entry(fp, ifindex, i, n);
}

static void print_router_entries(FILE *fp, struct nlmsghdr *n,
				 int ifindex, struct rtattr *router)
{
	const char *brifname = ll_index_to_name(ifindex);

	if (n->nlmsg_type == RTM_GETMDB) {
		if (show_details)
			br_print_router_ports(fp, router, brifname);
	} else {
		struct rtattr *i = RTA_DATA(router);
		uint32_t *port_ifindex = RTA_DATA(i);
		const char *port_name = ll_index_to_name(*port_ifindex);

		if (is_json_context()) {
			open_json_array(PRINT_JSON, brifname);
			open_json_object(NULL);

			print_string(PRINT_JSON, "port", NULL,
				     port_name);
			close_json_object();
			close_json_array(PRINT_JSON, NULL);
		} else {
			fprintf(fp, "router port dev %s master %s\n",
				port_name, brifname);
		}
	}
}

static int __parse_mdb_nlmsg(struct nlmsghdr *n, struct rtattr **tb)
{
	struct br_port_msg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;

	if (n->nlmsg_type != RTM_GETMDB &&
	    n->nlmsg_type != RTM_NEWMDB &&
	    n->nlmsg_type != RTM_DELMDB) {
		fprintf(stderr,
			"Not RTM_GETMDB, RTM_NEWMDB or RTM_DELMDB: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);

		return 0;
	}

	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (filter_index && filter_index != r->ifindex)
		return 0;

	parse_rtattr(tb, MDBA_MAX, MDBA_RTA(r), n->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	return 1;
}

static int print_mdbs(struct nlmsghdr *n, void *arg)
{
	struct br_port_msg *r = NLMSG_DATA(n);
	struct rtattr *tb[MDBA_MAX+1];
	FILE *fp = arg;
	int ret;

	ret = __parse_mdb_nlmsg(n, tb);
	if (ret != 1)
		return ret;

	if (tb[MDBA_MDB])
		print_mdb_entries(fp, n, r->ifindex, tb[MDBA_MDB]);

	return 0;
}

static int print_rtrs(struct nlmsghdr *n, void *arg)
{
	struct br_port_msg *r = NLMSG_DATA(n);
	struct rtattr *tb[MDBA_MAX+1];
	FILE *fp = arg;
	int ret;

	ret = __parse_mdb_nlmsg(n, tb);
	if (ret != 1)
		return ret;

	if (tb[MDBA_ROUTER])
		print_router_entries(fp, n, r->ifindex, tb[MDBA_ROUTER]);

	return 0;
}

int print_mdb_mon(struct nlmsghdr *n, void *arg)
{
	struct br_port_msg *r = NLMSG_DATA(n);
	struct rtattr *tb[MDBA_MAX+1];
	FILE *fp = arg;
	int ret;

	ret = __parse_mdb_nlmsg(n, tb);
	if (ret != 1)
		return ret;

	if (n->nlmsg_type == RTM_DELMDB)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	if (tb[MDBA_MDB])
		print_mdb_entries(fp, n, r->ifindex, tb[MDBA_MDB]);

	if (tb[MDBA_ROUTER])
		print_router_entries(fp, n, r->ifindex, tb[MDBA_ROUTER]);

	return 0;
}

static int mdb_show(int argc, char **argv)
{
	char *filter_dev = NULL;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (filter_dev)
				duparg("dev", *argv);
			filter_dev = *argv;
		} else if (strcmp(*argv, "vid") == 0) {
			NEXT_ARG();
			if (filter_vlan)
				duparg("vid", *argv);
			filter_vlan = atoi(*argv);
		}
		argc--; argv++;
	}

	if (filter_dev) {
		filter_index = ll_name_to_index(filter_dev);
		if (!filter_index)
			return nodev(filter_dev);
	}

	new_json_obj(json);
	open_json_object(NULL);

	/* get mdb entries */
	if (rtnl_mdbdump_req(&rth, PF_BRIDGE) < 0) {
		perror("Cannot send dump request");
		return -1;
	}

	open_json_array(PRINT_JSON, "mdb");
	if (rtnl_dump_filter(&rth, print_mdbs, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return -1;
	}
	close_json_array(PRINT_JSON, NULL);

	/* get router ports */
	if (rtnl_mdbdump_req(&rth, PF_BRIDGE) < 0) {
		perror("Cannot send dump request");
		return -1;
	}

	open_json_object("router");
	if (rtnl_dump_filter(&rth, print_rtrs, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return -1;
	}
	close_json_object();

	close_json_object();
	delete_json_obj();
	fflush(stdout);

	return 0;
}

static int mdb_parse_grp(const char *grp, struct br_mdb_entry *e)
{
	if (inet_pton(AF_INET, grp, &e->addr.u.ip4)) {
		e->addr.proto = htons(ETH_P_IP);
		return 0;
	}
	if (inet_pton(AF_INET6, grp, &e->addr.u.ip6)) {
		e->addr.proto = htons(ETH_P_IPV6);
		return 0;
	}
	if (ll_addr_a2n((char *)e->addr.u.mac_addr, sizeof(e->addr.u.mac_addr),
			grp) == ETH_ALEN) {
		e->addr.proto = 0;
		return 0;
	}

	return -1;
}

static int mdb_modify(int cmd, int flags, int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct br_port_msg	bpm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct br_port_msg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.bpm.family = PF_BRIDGE,
	};
	char *d = NULL, *p = NULL, *grp = NULL, *src = NULL;
	struct br_mdb_entry entry = {};
	short vid = 0;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			d = *argv;
		} else if (strcmp(*argv, "grp") == 0) {
			NEXT_ARG();
			grp = *argv;
		} else if (strcmp(*argv, "port") == 0) {
			NEXT_ARG();
			p = *argv;
		} else if (strcmp(*argv, "permanent") == 0) {
			if (cmd == RTM_NEWMDB)
				entry.state |= MDB_PERMANENT;
		} else if (strcmp(*argv, "temp") == 0) {
			;/* nothing */
		} else if (strcmp(*argv, "vid") == 0) {
			NEXT_ARG();
			vid = atoi(*argv);
		} else if (strcmp(*argv, "src") == 0) {
			NEXT_ARG();
			src = *argv;
		} else {
			if (matches(*argv, "help") == 0)
				usage();
		}
		argc--; argv++;
	}

	if (d == NULL || grp == NULL || p == NULL) {
		fprintf(stderr, "Device, group address and port name are required arguments.\n");
		return -1;
	}

	req.bpm.ifindex = ll_name_to_index(d);
	if (!req.bpm.ifindex)
		return nodev(d);

	entry.ifindex = ll_name_to_index(p);
	if (!entry.ifindex)
		return nodev(p);

	if (mdb_parse_grp(grp, &entry)) {
		fprintf(stderr, "Invalid address \"%s\"\n", grp);
		return -1;
	}

	entry.vid = vid;
	addattr_l(&req.n, sizeof(req), MDBA_SET_ENTRY, &entry, sizeof(entry));
	if (src) {
		struct rtattr *nest = addattr_nest(&req.n, sizeof(req),
						   MDBA_SET_ENTRY_ATTRS);
		struct in6_addr src_ip6;
		__be32 src_ip4;

		nest->rta_type |= NLA_F_NESTED;
		if (!inet_pton(AF_INET, src, &src_ip4)) {
			if (!inet_pton(AF_INET6, src, &src_ip6)) {
				fprintf(stderr, "Invalid source address \"%s\"\n", src);
				return -1;
			}
			addattr_l(&req.n, sizeof(req), MDBE_ATTR_SOURCE, &src_ip6, sizeof(src_ip6));
		} else {
			addattr32(&req.n, sizeof(req), MDBE_ATTR_SOURCE, src_ip4);
		}
		addattr_nest_end(&req.n, nest);
	}

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -1;

	return 0;
}

int do_mdb(int argc, char **argv)
{
	ll_init_map(&rth);

	if (argc > 0) {
		if (matches(*argv, "add") == 0)
			return mdb_modify(RTM_NEWMDB, NLM_F_CREATE|NLM_F_EXCL, argc-1, argv+1);
		if (matches(*argv, "delete") == 0)
			return mdb_modify(RTM_DELMDB, 0, argc-1, argv+1);

		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return mdb_show(argc-1, argv+1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return mdb_show(0, NULL);

	fprintf(stderr, "Command \"%s\" is unknown, try \"bridge mdb help\".\n", *argv);
	exit(-1);
}
