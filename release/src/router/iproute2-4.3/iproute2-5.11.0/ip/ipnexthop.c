// SPDX-License-Identifier: GPL-2.0
/*
 * ip nexthop
 *
 * Copyright (c) 2017-19 David Ahern <dsahern@gmail.com>
 */

#include <linux/nexthop.h>
#include <stdio.h>
#include <string.h>
#include <rt_names.h>
#include <errno.h>

#include "utils.h"
#include "ip_common.h"

static struct {
	unsigned int flushed;
	unsigned int groups;
	unsigned int ifindex;
	unsigned int master;
	unsigned int proto;
	unsigned int fdb;
} filter;

enum {
	IPNH_LIST,
	IPNH_FLUSH,
};

#define RTM_NHA(h)  ((struct rtattr *)(((char *)(h)) + \
			NLMSG_ALIGN(sizeof(struct nhmsg))))

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip nexthop { list | flush } [ protocol ID ] SELECTOR\n"
		"       ip nexthop { add | replace } id ID NH [ protocol ID ]\n"
		"       ip nexthop { get | del } id ID\n"
		"SELECTOR := [ id ID ] [ dev DEV ] [ vrf NAME ] [ master DEV ]\n"
		"            [ groups ] [ fdb ]\n"
		"NH := { blackhole | [ via ADDRESS ] [ dev DEV ] [ onlink ]\n"
		"        [ encap ENCAPTYPE ENCAPHDR ] | group GROUP [ fdb ] }\n"
		"GROUP := [ <id[,weight]>/<id[,weight]>/... ]\n"
		"ENCAPTYPE := [ mpls ]\n"
		"ENCAPHDR := [ MPLSLABEL ]\n");
	exit(-1);
}

static int nh_dump_filter(struct nlmsghdr *nlh, int reqlen)
{
	int err;

	if (filter.ifindex) {
		err = addattr32(nlh, reqlen, NHA_OIF, filter.ifindex);
		if (err)
			return err;
	}

	if (filter.groups) {
		err = addattr_l(nlh, reqlen, NHA_GROUPS, NULL, 0);
		if (err)
			return err;
	}

	if (filter.master) {
		err = addattr32(nlh, reqlen, NHA_MASTER, filter.master);
		if (err)
			return err;
	}

	if (filter.fdb) {
		err = addattr_l(nlh, reqlen, NHA_FDB, NULL, 0);
		if (err)
			return err;
	}

	return 0;
}

static struct rtnl_handle rth_del = { .fd = -1 };

static int delete_nexthop(__u32 id)
{
	struct {
		struct nlmsghdr	n;
		struct nhmsg	nhm;
		char		buf[64];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nhmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_DELNEXTHOP,
		.nhm.nh_family = AF_UNSPEC,
	};

	req.n.nlmsg_seq = ++rth_del.seq;

	addattr32(&req.n, sizeof(req), NHA_ID, id);

	if (rtnl_talk(&rth_del, &req.n, NULL) < 0)
		return -1;
	return 0;
}

static int flush_nexthop(struct nlmsghdr *nlh, void *arg)
{
	struct nhmsg *nhm = NLMSG_DATA(nlh);
	struct rtattr *tb[NHA_MAX+1];
	__u32 id = 0;
	int len;

	len = nlh->nlmsg_len - NLMSG_SPACE(sizeof(*nhm));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (filter.proto && nhm->nh_protocol != filter.proto)
		return 0;

	parse_rtattr(tb, NHA_MAX, RTM_NHA(nhm), len);
	if (tb[NHA_ID])
		id = rta_getattr_u32(tb[NHA_ID]);

	if (id && !delete_nexthop(id))
		filter.flushed++;

	return 0;
}

static int ipnh_flush(unsigned int all)
{
	int rc = -2;

	if (all) {
		filter.groups = 1;
		filter.ifindex = 0;
		filter.master = 0;
	}

	if (rtnl_open(&rth_del, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		return EXIT_FAILURE;
	}
again:
	if (rtnl_nexthopdump_req(&rth, preferred_family, nh_dump_filter) < 0) {
		perror("Cannot send dump request");
		goto out;
	}

	if (rtnl_dump_filter(&rth, flush_nexthop, stdout) < 0) {
		fprintf(stderr, "Dump terminated. Failed to flush nexthops\n");
		goto out;
	}

	/* if deleting all, then remove groups first */
	if (all && filter.groups) {
		filter.groups = 0;
		goto again;
	}

	rc = 0;
out:
	rtnl_close(&rth_del);
	if (!filter.flushed)
		printf("Nothing to flush\n");
	else
		printf("Flushed %d nexthops\n", filter.flushed);

	return rc;
}

static void print_nh_group(FILE *fp, const struct rtattr *grps_attr)
{
	struct nexthop_grp *nhg = RTA_DATA(grps_attr);
	int num = RTA_PAYLOAD(grps_attr) / sizeof(*nhg);
	int i;

	if (!num || num * sizeof(*nhg) != RTA_PAYLOAD(grps_attr)) {
		fprintf(fp, "<invalid nexthop group>");
		return;
	}

	open_json_array(PRINT_JSON, "group");
	print_string(PRINT_FP, NULL, "%s", "group ");
	for (i = 0; i < num; ++i) {
		open_json_object(NULL);

		if (i)
			print_string(PRINT_FP, NULL, "%s", "/");

		print_uint(PRINT_ANY, "id", "%u", nhg[i].id);
		if (nhg[i].weight)
			print_uint(PRINT_ANY, "weight", ",%u", nhg[i].weight + 1);

		close_json_object();
	}
	print_string(PRINT_FP, NULL, "%s", " ");
	close_json_array(PRINT_JSON, NULL);
}

int print_nexthop(struct nlmsghdr *n, void *arg)
{
	struct nhmsg *nhm = NLMSG_DATA(n);
	struct rtattr *tb[NHA_MAX+1];
	FILE *fp = (FILE *)arg;
	int len;

	SPRINT_BUF(b1);

	if (n->nlmsg_type != RTM_DELNEXTHOP &&
	    n->nlmsg_type != RTM_NEWNEXTHOP) {
		fprintf(stderr, "Not a nexthop: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return -1;
	}

	len = n->nlmsg_len - NLMSG_SPACE(sizeof(*nhm));
	if (len < 0) {
		close_json_object();
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (filter.proto && filter.proto != nhm->nh_protocol)
		return 0;

	parse_rtattr(tb, NHA_MAX, RTM_NHA(nhm), len);

	open_json_object(NULL);

	if (n->nlmsg_type == RTM_DELNEXTHOP)
		print_bool(PRINT_ANY, "deleted", "Deleted ", true);

	if (tb[NHA_ID])
		print_uint(PRINT_ANY, "id", "id %u ",
			   rta_getattr_u32(tb[NHA_ID]));

	if (tb[NHA_GROUP])
		print_nh_group(fp, tb[NHA_GROUP]);

	if (tb[NHA_ENCAP])
		lwt_print_encap(fp, tb[NHA_ENCAP_TYPE], tb[NHA_ENCAP]);

	if (tb[NHA_GATEWAY])
		print_rta_gateway(fp, nhm->nh_family, tb[NHA_GATEWAY]);

	if (tb[NHA_OIF])
		print_rta_if(fp, tb[NHA_OIF], "dev");

	if (nhm->nh_scope != RT_SCOPE_UNIVERSE || show_details > 0) {
		print_string(PRINT_ANY, "scope", "scope %s ",
			     rtnl_rtscope_n2a(nhm->nh_scope, b1, sizeof(b1)));
	}

	if (tb[NHA_BLACKHOLE])
		print_null(PRINT_ANY, "blackhole", "blackhole ", NULL);

	if (nhm->nh_protocol != RTPROT_UNSPEC || show_details > 0) {
		print_string(PRINT_ANY, "protocol", "proto %s ",
			     rtnl_rtprot_n2a(nhm->nh_protocol, b1, sizeof(b1)));
	}

	print_rt_flags(fp, nhm->nh_flags);

	if (tb[NHA_FDB])
		print_null(PRINT_ANY, "fdb", "fdb", NULL);

	print_string(PRINT_FP, NULL, "%s", "\n");
	close_json_object();
	fflush(fp);

	return 0;
}

static int add_nh_group_attr(struct nlmsghdr *n, int maxlen, char *argv)
{
	struct nexthop_grp *grps;
	int count = 0, i;
	char *sep, *wsep;

	if (*argv != '\0')
		count = 1;

	/* separator is '/' */
	sep = strchr(argv, '/');
	while (sep) {
		count++;
		sep = strchr(sep + 1, '/');
	}

	if (count == 0)
		return -1;

	grps = calloc(count, sizeof(*grps));
	if (!grps)
		return -1;

	for (i = 0; i < count; ++i) {
		sep = strchr(argv, '/');
		if (sep)
			*sep = '\0';

		wsep = strchr(argv, ',');
		if (wsep)
			*wsep = '\0';

		if (get_unsigned(&grps[i].id, argv, 0))
			return -1;
		if (wsep) {
			unsigned int w;

			wsep++;
			if (get_unsigned(&w, wsep, 0) || w == 0 || w > 256)
				invarg("\"weight\" is invalid\n", wsep);
			grps[i].weight = w - 1;
		}

		if (!sep)
			break;

		argv = sep + 1;
	}

	return addattr_l(n, maxlen, NHA_GROUP, grps, count * sizeof(*grps));
}

static int ipnh_modify(int cmd, unsigned int flags, int argc, char **argv)
{
	struct {
		struct nlmsghdr	n;
		struct nhmsg	nhm;
		char		buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nhmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.nhm.nh_family = preferred_family,
	};
	__u32 nh_flags = 0;

	while (argc > 0) {
		if (!strcmp(*argv, "id")) {
			__u32 id;

			NEXT_ARG();
			if (get_unsigned(&id, *argv, 0))
				invarg("invalid id value", *argv);
			addattr32(&req.n, sizeof(req), NHA_ID, id);
		} else if (!strcmp(*argv, "dev")) {
			int ifindex;

			NEXT_ARG();
			ifindex = ll_name_to_index(*argv);
			if (!ifindex)
				invarg("Device does not exist\n", *argv);
			addattr32(&req.n, sizeof(req), NHA_OIF, ifindex);
			if (req.nhm.nh_family == AF_UNSPEC)
				req.nhm.nh_family = AF_INET;
		} else if (strcmp(*argv, "via") == 0) {
			inet_prefix addr;
			int family;

			NEXT_ARG();
			family = read_family(*argv);
			if (family == AF_UNSPEC)
				family = req.nhm.nh_family;
			else
				NEXT_ARG();
			get_addr(&addr, *argv, family);
			if (req.nhm.nh_family == AF_UNSPEC)
				req.nhm.nh_family = addr.family;
			else if (req.nhm.nh_family != addr.family)
				invarg("address family mismatch\n", *argv);
			addattr_l(&req.n, sizeof(req), NHA_GATEWAY,
				  &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "encap") == 0) {
			char buf[1024];
			struct rtattr *rta = (void *)buf;

			rta->rta_type = NHA_ENCAP;
			rta->rta_len = RTA_LENGTH(0);

			lwt_parse_encap(rta, sizeof(buf), &argc, &argv,
					NHA_ENCAP, NHA_ENCAP_TYPE);

			if (rta->rta_len > RTA_LENGTH(0)) {
				addraw_l(&req.n, 1024, RTA_DATA(rta),
					 RTA_PAYLOAD(rta));
			}
		} else if (!strcmp(*argv, "blackhole")) {
			addattr_l(&req.n, sizeof(req), NHA_BLACKHOLE, NULL, 0);
			if (req.nhm.nh_family == AF_UNSPEC)
				req.nhm.nh_family = AF_INET;
		} else if (!strcmp(*argv, "fdb")) {
			addattr_l(&req.n, sizeof(req), NHA_FDB, NULL, 0);
		} else if (!strcmp(*argv, "onlink")) {
			nh_flags |= RTNH_F_ONLINK;
		} else if (!strcmp(*argv, "group")) {
			NEXT_ARG();

			if (add_nh_group_attr(&req.n, sizeof(req), *argv))
				invarg("\"group\" value is invalid\n", *argv);
		} else if (matches(*argv, "protocol") == 0) {
			__u32 prot;

			NEXT_ARG();
			if (rtnl_rtprot_a2n(&prot, *argv))
				invarg("\"protocol\" value is invalid\n", *argv);
			req.nhm.nh_protocol = prot;
		} else if (strcmp(*argv, "help") == 0) {
			usage();
		} else {
			invarg("", *argv);
		}
		argc--; argv++;
	}

	req.nhm.nh_flags = nh_flags;

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static int ipnh_get_id(__u32 id)
{
	struct {
		struct nlmsghdr	n;
		struct nhmsg	nhm;
		char		buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nhmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type  = RTM_GETNEXTHOP,
		.nhm.nh_family = preferred_family,
	};
	struct nlmsghdr *answer;

	addattr32(&req.n, sizeof(req), NHA_ID, id);

	if (rtnl_talk(&rth, &req.n, &answer) < 0)
		return -2;

	new_json_obj(json);

	if (print_nexthop(answer, (void *)stdout) < 0) {
		free(answer);
		return -1;
	}

	delete_json_obj();
	fflush(stdout);

	free(answer);

	return 0;
}

static int ipnh_list_flush(int argc, char **argv, int action)
{
	unsigned int all = (argc == 0);

	while (argc > 0) {
		if (!matches(*argv, "dev")) {
			NEXT_ARG();
			filter.ifindex = ll_name_to_index(*argv);
			if (!filter.ifindex)
				invarg("Device does not exist\n", *argv);
		} else if (!matches(*argv, "groups")) {
			filter.groups = 1;
		} else if (!matches(*argv, "master")) {
			NEXT_ARG();
			filter.master = ll_name_to_index(*argv);
			if (!filter.master)
				invarg("Device does not exist\n", *argv);
		} else if (matches(*argv, "vrf") == 0) {
			NEXT_ARG();
			if (!name_is_vrf(*argv))
				invarg("Invalid VRF\n", *argv);
			filter.master = ll_name_to_index(*argv);
			if (!filter.master)
				invarg("VRF does not exist\n", *argv);
		} else if (!strcmp(*argv, "id")) {
			__u32 id;

			NEXT_ARG();
			if (get_unsigned(&id, *argv, 0))
				invarg("invalid id value", *argv);
			return ipnh_get_id(id);
		} else if (!matches(*argv, "protocol")) {
			__u32 proto;

			NEXT_ARG();
			if (get_unsigned(&proto, *argv, 0))
				invarg("invalid protocol value", *argv);
			filter.proto = proto;
		} else if (!matches(*argv, "fdb")) {
			filter.fdb = 1;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			invarg("", *argv);
		}
		argc--; argv++;
	}

	if (action == IPNH_FLUSH)
		return ipnh_flush(all);

	if (rtnl_nexthopdump_req(&rth, preferred_family, nh_dump_filter) < 0) {
		perror("Cannot send dump request");
		return -2;
	}

	new_json_obj(json);

	if (rtnl_dump_filter(&rth, print_nexthop, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return -2;
	}

	delete_json_obj();
	fflush(stdout);

	return 0;
}

static int ipnh_get(int argc, char **argv)
{
	__u32 id = 0;

	while (argc > 0) {
		if (!strcmp(*argv, "id")) {
			NEXT_ARG();
			if (get_unsigned(&id, *argv, 0))
				invarg("invalid id value", *argv);
		} else  {
			usage();
		}
		argc--; argv++;
	}

	if (!id) {
		usage();
		return -1;
	}

	return ipnh_get_id(id);
}

int do_ipnh(int argc, char **argv)
{
	if (argc < 1)
		return ipnh_list_flush(0, NULL, IPNH_LIST);

	if (!matches(*argv, "add"))
		return ipnh_modify(RTM_NEWNEXTHOP, NLM_F_CREATE|NLM_F_EXCL,
				   argc-1, argv+1);
	if (!matches(*argv, "replace"))
		return ipnh_modify(RTM_NEWNEXTHOP, NLM_F_CREATE|NLM_F_REPLACE,
				   argc-1, argv+1);
	if (!matches(*argv, "delete"))
		return ipnh_modify(RTM_DELNEXTHOP, 0, argc-1, argv+1);

	if (!matches(*argv, "list") ||
	    !matches(*argv, "show") ||
	    !matches(*argv, "lst"))
		return ipnh_list_flush(argc-1, argv+1, IPNH_LIST);

	if (!matches(*argv, "get"))
		return ipnh_get(argc-1, argv+1);

	if (!matches(*argv, "flush"))
		return ipnh_list_flush(argc-1, argv+1, IPNH_FLUSH);

	if (!matches(*argv, "help"))
		usage();

	fprintf(stderr,
		"Command \"%s\" is unknown, try \"ip nexthop help\".\n", *argv);
	exit(-1);
}
