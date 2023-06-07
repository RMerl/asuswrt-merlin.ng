/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Get/set/delete fdb table with netlink
 *
 * TODO: merge/replace this with ip neighbour
 *
 * Authors:	Stephen Hemminger <shemminger@vyatta.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>
#include <linux/if_ether.h>
#include <linux/neighbour.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#include "json_print.h"
#include "libnetlink.h"
#include "br_common.h"
#include "rt_names.h"
#include "utils.h"

static unsigned int filter_index, filter_dynamic, filter_master,
	filter_state, filter_vlan;

static void usage(void)
{
	fprintf(stderr,
		"Usage: bridge fdb { add | append | del | replace } ADDR dev DEV\n"
		"              [ self ] [ master ] [ use ] [ router ] [ extern_learn ]\n"
		"              [ sticky ] [ local | static | dynamic ] [ vlan VID ]\n"
		"              { [ dst IPADDR ] [ port PORT] [ vni VNI ] | [ nhid NHID ] }\n"
		"	       [ via DEV ] [ src_vni VNI ]\n"
		"       bridge fdb [ show [ br BRDEV ] [ brport DEV ] [ vlan VID ]\n"
		"              [ state STATE ] [ dynamic ] ]\n"
		"       bridge fdb get [ to ] LLADDR [ br BRDEV ] { brport | dev } DEV\n"
		"              [ vlan VID ] [ vni VNI ] [ self ] [ master ] [ dynamic ]\n");
	exit(-1);
}

static const char *state_n2a(unsigned int s)
{
	static char buf[32];

	if (s & NUD_PERMANENT)
		return "permanent";

	if (s & NUD_NOARP)
		return "static";

	if (s & NUD_STALE)
		return "stale";

	if (s & NUD_REACHABLE)
		return "";

	if (is_json_context())
		sprintf(buf, "%#x", s);
	else
		sprintf(buf, "state=%#x", s);
	return buf;
}

static int state_a2n(unsigned int *s, const char *arg)
{
	if (matches(arg, "permanent") == 0)
		*s = NUD_PERMANENT;
	else if (matches(arg, "static") == 0 || matches(arg, "temp") == 0)
		*s = NUD_NOARP;
	else if (matches(arg, "stale") == 0)
		*s = NUD_STALE;
	else if (matches(arg, "reachable") == 0 || matches(arg, "dynamic") == 0)
		*s = NUD_REACHABLE;
	else if (strcmp(arg, "all") == 0)
		*s = ~0;
	else if (get_unsigned(s, arg, 0))
		return -1;

	return 0;
}

static void fdb_print_flags(FILE *fp, unsigned int flags)
{
	open_json_array(PRINT_JSON,
			is_json_context() ?  "flags" : "");

	if (flags & NTF_SELF)
		print_string(PRINT_ANY, NULL, "%s ", "self");

	if (flags & NTF_ROUTER)
		print_string(PRINT_ANY, NULL, "%s ", "router");

	if (flags & NTF_EXT_LEARNED)
		print_string(PRINT_ANY, NULL, "%s ", "extern_learn");

	if (flags & NTF_OFFLOADED)
		print_string(PRINT_ANY, NULL, "%s ", "offload");

	if (flags & NTF_MASTER)
		print_string(PRINT_ANY, NULL, "%s ", "master");

	if (flags & NTF_STICKY)
		print_string(PRINT_ANY, NULL, "%s ", "sticky");

	close_json_array(PRINT_JSON, NULL);
}

static void fdb_print_stats(FILE *fp, const struct nda_cacheinfo *ci)
{
	static int hz;

	if (!hz)
		hz = get_user_hz();

	if (is_json_context()) {
		print_uint(PRINT_JSON, "used", NULL,
				 ci->ndm_used / hz);
		print_uint(PRINT_JSON, "updated", NULL,
				ci->ndm_updated / hz);
	} else {
		fprintf(fp, "used %d/%d ", ci->ndm_used / hz,
					ci->ndm_updated / hz);

	}
}

int print_fdb(struct nlmsghdr *n, void *arg)
{
	FILE *fp = arg;
	struct ndmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[NDA_MAX+1];
	__u16 vid = 0;

	if (n->nlmsg_type != RTM_NEWNEIGH && n->nlmsg_type != RTM_DELNEIGH) {
		fprintf(stderr, "Not RTM_NEWNEIGH: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return 0;
	}

	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (r->ndm_family != AF_BRIDGE)
		return 0;

	if (filter_index && filter_index != r->ndm_ifindex)
		return 0;

	if (filter_state && !(r->ndm_state & filter_state))
		return 0;

	parse_rtattr(tb, NDA_MAX, NDA_RTA(r),
		     n->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	if (tb[NDA_VLAN])
		vid = rta_getattr_u16(tb[NDA_VLAN]);

	if (filter_vlan && filter_vlan != vid)
		return 0;

	if (filter_dynamic && (r->ndm_state & NUD_PERMANENT))
		return 0;

	open_json_object(NULL);
	if (n->nlmsg_type == RTM_DELNEIGH)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	if (tb[NDA_LLADDR]) {
		const char *lladdr;
		SPRINT_BUF(b1);

		lladdr = ll_addr_n2a(RTA_DATA(tb[NDA_LLADDR]),
				     RTA_PAYLOAD(tb[NDA_LLADDR]),
				     ll_index_to_type(r->ndm_ifindex),
				     b1, sizeof(b1));

		print_color_string(PRINT_ANY, COLOR_MAC,
				   "mac", "%s ", lladdr);
	}

	if (!filter_index && r->ndm_ifindex)
		print_color_string(PRINT_ANY, COLOR_IFNAME,
				   "ifname", "dev %s ",
				   ll_index_to_name(r->ndm_ifindex));

	if (tb[NDA_DST]) {
		int family = AF_INET;
		const char *dst;

		if (RTA_PAYLOAD(tb[NDA_DST]) == sizeof(struct in6_addr))
			family = AF_INET6;

		dst = format_host(family,
				  RTA_PAYLOAD(tb[NDA_DST]),
				  RTA_DATA(tb[NDA_DST]));

		print_color_string(PRINT_ANY,
				   ifa_family_color(family),
				    "dst", "dst %s ", dst);
	}

	if (vid)
		print_uint(PRINT_ANY,
				 "vlan", "vlan %hu ", vid);

	if (tb[NDA_PORT])
		print_uint(PRINT_ANY,
				 "port", "port %u ",
				 rta_getattr_be16(tb[NDA_PORT]));

	if (tb[NDA_VNI])
		print_uint(PRINT_ANY,
				 "vni", "vni %u ",
				 rta_getattr_u32(tb[NDA_VNI]));

	if (tb[NDA_SRC_VNI])
		print_uint(PRINT_ANY,
				 "src_vni", "src_vni %u ",
				rta_getattr_u32(tb[NDA_SRC_VNI]));

	if (tb[NDA_IFINDEX]) {
		unsigned int ifindex = rta_getattr_u32(tb[NDA_IFINDEX]);

		if (tb[NDA_LINK_NETNSID])
			print_uint(PRINT_ANY,
					 "viaIfIndex", "via ifindex %u ",
					 ifindex);
		else
			print_string(PRINT_ANY,
					   "viaIf", "via %s ",
					   ll_index_to_name(ifindex));
	}

	if (tb[NDA_NH_ID])
		print_uint(PRINT_ANY, "nhid", "nhid %u ",
			   rta_getattr_u32(tb[NDA_NH_ID]));

	if (tb[NDA_LINK_NETNSID])
		print_uint(PRINT_ANY,
				 "linkNetNsId", "link-netnsid %d ",
				 rta_getattr_u32(tb[NDA_LINK_NETNSID]));

	if (show_stats && tb[NDA_CACHEINFO])
		fdb_print_stats(fp, RTA_DATA(tb[NDA_CACHEINFO]));

	fdb_print_flags(fp, r->ndm_flags);


	if (tb[NDA_MASTER])
		print_string(PRINT_ANY, "master", "master %s ",
			     ll_index_to_name(rta_getattr_u32(tb[NDA_MASTER])));

	print_string(PRINT_ANY, "state", "%s\n",
			   state_n2a(r->ndm_state));
	close_json_object();
	fflush(fp);
	return 0;
}

static int fdb_linkdump_filter(struct nlmsghdr *nlh, int reqlen)
{
	int err;

	if (filter_index) {
		struct ifinfomsg *ifm = NLMSG_DATA(nlh);

		ifm->ifi_index = filter_index;
	}

	if (filter_master) {
		err = addattr32(nlh, reqlen, IFLA_MASTER, filter_master);
		if (err)
			return err;
	}

	return 0;
}

static int fdb_dump_filter(struct nlmsghdr *nlh, int reqlen)
{
	int err;

	if (filter_index) {
		struct ndmsg *ndm = NLMSG_DATA(nlh);

		ndm->ndm_ifindex = filter_index;
	}

	if (filter_master) {
		err = addattr32(nlh, reqlen, NDA_MASTER, filter_master);
		if (err)
			return err;
	}

	return 0;
}

static int fdb_show(int argc, char **argv)
{
	char *filter_dev = NULL;
	char *br = NULL;
	int rc;

	while (argc > 0) {
		if ((strcmp(*argv, "brport") == 0) || strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			filter_dev = *argv;
		} else if (strcmp(*argv, "br") == 0) {
			NEXT_ARG();
			br = *argv;
		} else if (strcmp(*argv, "vlan") == 0) {
			NEXT_ARG();
			if (filter_vlan)
				duparg("vlan", *argv);
			filter_vlan = atoi(*argv);
		} else if (strcmp(*argv, "state") == 0) {
			unsigned int state;

			NEXT_ARG();
			if (state_a2n(&state, *argv))
				invarg("invalid state", *argv);
			filter_state |= state;
		} else if (strcmp(*argv, "dynamic") == 0) {
			filter_dynamic = 1;
		} else {
			if (matches(*argv, "help") == 0)
				usage();
		}
		argc--; argv++;
	}

	if (br) {
		int br_ifindex = ll_name_to_index(br);

		if (br_ifindex == 0) {
			fprintf(stderr, "Cannot find bridge device \"%s\"\n", br);
			return -1;
		}
		filter_master = br_ifindex;
	}

	/*we'll keep around filter_dev for older kernels */
	if (filter_dev) {
		filter_index = ll_name_to_index(filter_dev);
		if (!filter_index)
			return nodev(filter_dev);
	}

	if (rth.flags & RTNL_HANDLE_F_STRICT_CHK)
		rc = rtnl_neighdump_req(&rth, PF_BRIDGE, fdb_dump_filter);
	else
		rc = rtnl_fdb_linkdump_req_filter_fn(&rth, fdb_linkdump_filter);
	if (rc < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, print_fdb, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	delete_json_obj();
	fflush(stdout);

	return 0;
}

static int fdb_modify(int cmd, int flags, int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct ndmsg		ndm;
		char			buf[256];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.ndm.ndm_family = PF_BRIDGE,
		.ndm.ndm_state = NUD_NOARP,
	};
	char *addr = NULL;
	char *d = NULL;
	char abuf[ETH_ALEN];
	int dst_ok = 0;
	inet_prefix dst;
	unsigned long port = 0;
	unsigned long vni = ~0;
	unsigned long src_vni = ~0;
	unsigned int via = 0;
	char *endptr;
	short vid = -1;
	__u32 nhid = 0;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			d = *argv;
		} else if (strcmp(*argv, "dst") == 0) {
			NEXT_ARG();
			if (dst_ok)
				duparg2("dst", *argv);
			get_addr(&dst, *argv, preferred_family);
			dst_ok = 1;
		} else if (strcmp(*argv, "nhid") == 0) {
			NEXT_ARG();
			if (get_u32(&nhid, *argv, 0))
				invarg("\"id\" value is invalid\n", *argv);
		} else if (strcmp(*argv, "port") == 0) {

			NEXT_ARG();
			port = strtoul(*argv, &endptr, 0);
			if (endptr && *endptr) {
				struct servent *pse;

				pse = getservbyname(*argv, "udp");
				if (!pse)
					invarg("invalid port\n", *argv);
				port = ntohs(pse->s_port);
			} else if (port > 0xffff)
				invarg("invalid port\n", *argv);
		} else if (strcmp(*argv, "vni") == 0) {
			NEXT_ARG();
			vni = strtoul(*argv, &endptr, 0);
			if ((endptr && *endptr) ||
			    (vni >> 24) || vni == ULONG_MAX)
				invarg("invalid VNI\n", *argv);
		} else if (strcmp(*argv, "src_vni") == 0) {
			NEXT_ARG();
			src_vni = strtoul(*argv, &endptr, 0);
			if ((endptr && *endptr) ||
			    (src_vni >> 24) || src_vni == ULONG_MAX)
				invarg("invalid src VNI\n", *argv);
		} else if (strcmp(*argv, "via") == 0) {
			NEXT_ARG();
			via = ll_name_to_index(*argv);
			if (!via)
				exit(nodev(*argv));
		} else if (strcmp(*argv, "self") == 0) {
			req.ndm.ndm_flags |= NTF_SELF;
		} else if (matches(*argv, "master") == 0) {
			req.ndm.ndm_flags |= NTF_MASTER;
		} else if (matches(*argv, "router") == 0) {
			req.ndm.ndm_flags |= NTF_ROUTER;
		} else if (matches(*argv, "local") == 0 ||
			   matches(*argv, "permanent") == 0) {
			req.ndm.ndm_state |= NUD_PERMANENT;
		} else if (matches(*argv, "temp") == 0 ||
			   matches(*argv, "static") == 0) {
			req.ndm.ndm_state |= NUD_REACHABLE;
		} else if (matches(*argv, "dynamic") == 0) {
			req.ndm.ndm_state |= NUD_REACHABLE;
			req.ndm.ndm_state &= ~NUD_NOARP;
		} else if (matches(*argv, "vlan") == 0) {
			if (vid >= 0)
				duparg2("vlan", *argv);
			NEXT_ARG();
			vid = atoi(*argv);
		} else if (matches(*argv, "use") == 0) {
			req.ndm.ndm_flags |= NTF_USE;
		} else if (matches(*argv, "extern_learn") == 0) {
			req.ndm.ndm_flags |= NTF_EXT_LEARNED;
		} else if (matches(*argv, "sticky") == 0) {
			req.ndm.ndm_flags |= NTF_STICKY;
		} else {
			if (strcmp(*argv, "to") == 0)
				NEXT_ARG();

			if (matches(*argv, "help") == 0)
				usage();
			if (addr)
				duparg2("to", *argv);
			addr = *argv;
		}
		argc--; argv++;
	}

	if (d == NULL || addr == NULL) {
		fprintf(stderr, "Device and address are required arguments.\n");
		return -1;
	}

	if (nhid && (dst_ok || port || vni != ~0)) {
		fprintf(stderr, "dst, port, vni are mutually exclusive with nhid\n");
		return -1;
	}

	/* Assume self */
	if (!(req.ndm.ndm_flags&(NTF_SELF|NTF_MASTER)))
		req.ndm.ndm_flags |= NTF_SELF;

	/* Assume permanent */
	if (!(req.ndm.ndm_state&(NUD_PERMANENT|NUD_REACHABLE)))
		req.ndm.ndm_state |= NUD_PERMANENT;

	if (sscanf(addr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		   abuf, abuf+1, abuf+2,
		   abuf+3, abuf+4, abuf+5) != 6) {
		fprintf(stderr, "Invalid mac address %s\n", addr);
		return -1;
	}

	addattr_l(&req.n, sizeof(req), NDA_LLADDR, abuf, ETH_ALEN);
	if (dst_ok)
		addattr_l(&req.n, sizeof(req), NDA_DST, &dst.data, dst.bytelen);

	if (vid >= 0)
		addattr16(&req.n, sizeof(req), NDA_VLAN, vid);
	if (nhid > 0)
		addattr32(&req.n, sizeof(req), NDA_NH_ID, nhid);

	if (port) {
		unsigned short dport;

		dport = htons((unsigned short)port);
		addattr16(&req.n, sizeof(req), NDA_PORT, dport);
	}
	if (vni != ~0)
		addattr32(&req.n, sizeof(req), NDA_VNI, vni);
	if (src_vni != ~0)
		addattr32(&req.n, sizeof(req), NDA_SRC_VNI, src_vni);
	if (via)
		addattr32(&req.n, sizeof(req), NDA_IFINDEX, via);

	req.ndm.ndm_ifindex = ll_name_to_index(d);
	if (!req.ndm.ndm_ifindex)
		return nodev(d);

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -1;

	return 0;
}

static int fdb_get(int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct ndmsg		ndm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_GETNEIGH,
		.ndm.ndm_family = AF_BRIDGE,
	};
	char  *d = NULL, *br = NULL;
	struct nlmsghdr *answer;
	unsigned long vni = ~0;
	char abuf[ETH_ALEN];
	int br_ifindex = 0;
	char *addr = NULL;
	short vlan = -1;
	char *endptr;

	while (argc > 0) {
		if ((strcmp(*argv, "brport") == 0) || strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			d = *argv;
		} else if (strcmp(*argv, "br") == 0) {
			NEXT_ARG();
			br = *argv;
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			d = *argv;
		} else if (strcmp(*argv, "vni") == 0) {
			NEXT_ARG();
			vni = strtoul(*argv, &endptr, 0);
			if ((endptr && *endptr) ||
			    (vni >> 24) || vni == ULONG_MAX)
				invarg("invalid VNI\n", *argv);
		} else if (strcmp(*argv, "self") == 0) {
			req.ndm.ndm_flags |= NTF_SELF;
		} else if (matches(*argv, "master") == 0) {
			req.ndm.ndm_flags |= NTF_MASTER;
		} else if (matches(*argv, "vlan") == 0) {
			if (vlan >= 0)
				duparg2("vlan", *argv);
			NEXT_ARG();
			vlan = atoi(*argv);
		} else if (matches(*argv, "dynamic") == 0) {
			filter_dynamic = 1;
		} else {
			if (strcmp(*argv, "to") == 0)
				NEXT_ARG();

			if (matches(*argv, "help") == 0)
				usage();
			if (addr)
				duparg2("to", *argv);
			addr = *argv;
		}
		argc--; argv++;
	}

	if ((d == NULL && br == NULL) || addr == NULL) {
		fprintf(stderr, "Device or master and address are required arguments.\n");
		return -1;
	}

	if (sscanf(addr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		   abuf, abuf+1, abuf+2,
		   abuf+3, abuf+4, abuf+5) != 6) {
		fprintf(stderr, "Invalid mac address %s\n", addr);
		return -1;
	}

	addattr_l(&req.n, sizeof(req), NDA_LLADDR, abuf, ETH_ALEN);

	if (vlan >= 0)
		addattr16(&req.n, sizeof(req), NDA_VLAN, vlan);

	if (vni != ~0)
		addattr32(&req.n, sizeof(req), NDA_VNI, vni);

	if (d) {
		req.ndm.ndm_ifindex = ll_name_to_index(d);
		if (!req.ndm.ndm_ifindex) {
			fprintf(stderr, "Cannot find device \"%s\"\n", d);
			return -1;
		}
	}

	if (br) {
		br_ifindex = ll_name_to_index(br);
		if (!br_ifindex) {
			fprintf(stderr, "Cannot find bridge device \"%s\"\n", br);
			return -1;
		}
		addattr32(&req.n, sizeof(req), NDA_MASTER, br_ifindex);
	}

	if (rtnl_talk(&rth, &req.n, &answer) < 0)
		return -2;

	/*
	 * Initialize a json_writer and open an array object
	 * if -json was specified.
	 */
	new_json_obj(json);
	if (print_fdb(answer, stdout) < 0) {
		fprintf(stderr, "An error :-)\n");
		return -1;
	}
	delete_json_obj();

	return 0;
}

int do_fdb(int argc, char **argv)
{
	ll_init_map(&rth);

	if (argc > 0) {
		if (matches(*argv, "add") == 0)
			return fdb_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_EXCL, argc-1, argv+1);
		if (matches(*argv, "append") == 0)
			return fdb_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_APPEND, argc-1, argv+1);
		if (matches(*argv, "replace") == 0)
			return fdb_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE, argc-1, argv+1);
		if (matches(*argv, "delete") == 0)
			return fdb_modify(RTM_DELNEIGH, 0, argc-1, argv+1);
		if (matches(*argv, "get") == 0)
			return fdb_get(argc-1, argv+1);
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return fdb_show(argc-1, argv+1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return fdb_show(0, NULL);

	fprintf(stderr, "Command \"%s\" is unknown, try \"bridge fdb help\".\n", *argv);
	exit(-1);
}
