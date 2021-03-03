/*
 * tc_qdisc.c		"tc qdisc".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *		J Hadi Salim: Extension to ingress
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include "utils.h"
#include "tc_util.h"
#include "tc_common.h"

static int usage(void)
{
	fprintf(stderr,
		"Usage: tc qdisc [ add | del | replace | change | show ] dev STRING\n"
		"       [ handle QHANDLE ] [ root | ingress | clsact | parent CLASSID ]\n"
		"       [ estimator INTERVAL TIME_CONSTANT ]\n"
		"       [ stab [ help | STAB_OPTIONS] ]\n"
		"       [ ingress_block BLOCK_INDEX ] [ egress_block BLOCK_INDEX ]\n"
		"       [ [ QDISC_KIND ] [ help | OPTIONS ] ]\n"
		"\n"
		"       tc qdisc { show | list } [ dev STRING ] [ QDISC_ID ] [ invisible ]\n"
		"Where:\n"
		"QDISC_KIND := { [p|b]fifo | tbf | prio | cbq | red | etc. }\n"
		"OPTIONS := ... try tc qdisc add <desired QDISC_KIND> help\n"
		"STAB_OPTIONS := ... try tc qdisc add stab help\n"
		"QDISC_ID := { root | ingress | handle QHANDLE | parent CLASSID }\n");
	return -1;
}

static int tc_qdisc_modify(int cmd, unsigned int flags, int argc, char **argv)
{
	struct qdisc_util *q = NULL;
	struct tc_estimator est = {};
	struct {
		struct tc_sizespec	szopts;
		__u16			*data;
	} stab = {};
	char  d[IFNAMSIZ] = {};
	char  k[FILTER_NAMESZ] = {};
	struct {
		struct nlmsghdr	n;
		struct tcmsg		t;
		char			buf[TCA_BUF_MAX];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.t.tcm_family = AF_UNSPEC,
	};
	__u32 ingress_block = 0;
	__u32 egress_block = 0;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (d[0])
				duparg("dev", *argv);
			strncpy(d, *argv, sizeof(d)-1);
		} else if (strcmp(*argv, "handle") == 0) {
			__u32 handle;

			if (req.t.tcm_handle)
				duparg("handle", *argv);
			NEXT_ARG();
			if (get_qdisc_handle(&handle, *argv))
				invarg("invalid qdisc ID", *argv);
			req.t.tcm_handle = handle;
		} else if (strcmp(*argv, "root") == 0) {
			if (req.t.tcm_parent) {
				fprintf(stderr, "Error: \"root\" is duplicate parent ID\n");
				return -1;
			}
			req.t.tcm_parent = TC_H_ROOT;
		} else if (strcmp(*argv, "clsact") == 0) {
			if (req.t.tcm_parent) {
				fprintf(stderr, "Error: \"clsact\" is a duplicate parent ID\n");
				return -1;
			}
			req.t.tcm_parent = TC_H_CLSACT;
			strncpy(k, "clsact", sizeof(k) - 1);
			q = get_qdisc_kind(k);
			req.t.tcm_handle = TC_H_MAKE(TC_H_CLSACT, 0);
			NEXT_ARG_FWD();
			break;
		} else if (strcmp(*argv, "ingress") == 0) {
			if (req.t.tcm_parent) {
				fprintf(stderr, "Error: \"ingress\" is a duplicate parent ID\n");
				return -1;
			}
			req.t.tcm_parent = TC_H_INGRESS;
			strncpy(k, "ingress", sizeof(k) - 1);
			q = get_qdisc_kind(k);
			req.t.tcm_handle = TC_H_MAKE(TC_H_INGRESS, 0);
			NEXT_ARG_FWD();
			break;
		} else if (strcmp(*argv, "parent") == 0) {
			__u32 handle;

			NEXT_ARG();
			if (req.t.tcm_parent)
				duparg("parent", *argv);
			if (get_tc_classid(&handle, *argv))
				invarg("invalid parent ID", *argv);
			req.t.tcm_parent = handle;
		} else if (matches(*argv, "estimator") == 0) {
			if (parse_estimator(&argc, &argv, &est))
				return -1;
		} else if (matches(*argv, "stab") == 0) {
			if (parse_size_table(&argc, &argv, &stab.szopts) < 0)
				return -1;
			continue;
		} else if (matches(*argv, "ingress_block") == 0) {
			NEXT_ARG();
			if (get_u32(&ingress_block, *argv, 0) || !ingress_block)
				invarg("invalid ingress block index value", *argv);
		} else if (matches(*argv, "egress_block") == 0) {
			NEXT_ARG();
			if (get_u32(&egress_block, *argv, 0) || !egress_block)
				invarg("invalid egress block index value", *argv);
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			strncpy(k, *argv, sizeof(k)-1);

			q = get_qdisc_kind(k);
			argc--; argv++;
			break;
		}
		argc--; argv++;
	}

	if (k[0])
		addattr_l(&req.n, sizeof(req), TCA_KIND, k, strlen(k)+1);
	if (est.ewma_log)
		addattr_l(&req.n, sizeof(req), TCA_RATE, &est, sizeof(est));

	if (ingress_block)
		addattr32(&req.n, sizeof(req),
			  TCA_INGRESS_BLOCK, ingress_block);
	if (egress_block)
		addattr32(&req.n, sizeof(req),
			  TCA_EGRESS_BLOCK, egress_block);

	if (q) {
		if (q->parse_qopt) {
			if (q->parse_qopt(q, argc, argv, &req.n, d))
				return 1;
		} else if (argc) {
			fprintf(stderr, "qdisc '%s' does not support option parsing\n", k);
			return -1;
		}
	} else {
		if (argc) {
			if (matches(*argv, "help") == 0)
				usage();

			fprintf(stderr, "Garbage instead of arguments \"%s ...\". Try \"tc qdisc help\".\n", *argv);
			return -1;
		}
	}

	if (check_size_table_opts(&stab.szopts)) {
		struct rtattr *tail;

		if (tc_calc_size_table(&stab.szopts, &stab.data) < 0) {
			fprintf(stderr, "failed to calculate size table.\n");
			return -1;
		}

		tail = addattr_nest(&req.n, sizeof(req), TCA_STAB);
		addattr_l(&req.n, sizeof(req), TCA_STAB_BASE, &stab.szopts,
			  sizeof(stab.szopts));
		if (stab.data)
			addattr_l(&req.n, sizeof(req), TCA_STAB_DATA, stab.data,
				  stab.szopts.tsize * sizeof(__u16));
		addattr_nest_end(&req.n, tail);
		if (stab.data)
			free(stab.data);
	}

	if (d[0])  {
		int idx;

		ll_init_map(&rth);

		idx = ll_name_to_index(d);
		if (!idx)
			return -nodev(d);
		req.t.tcm_ifindex = idx;
	}

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return 2;

	return 0;
}

static int filter_ifindex;
static __u32 filter_parent;
static __u32 filter_handle;

int print_qdisc(struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;
	struct tcmsg *t = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[TCA_MAX+1];
	struct qdisc_util *q;
	char abuf[256];

	if (n->nlmsg_type != RTM_NEWQDISC && n->nlmsg_type != RTM_DELQDISC) {
		fprintf(stderr, "Not a qdisc\n");
		return 0;
	}
	len -= NLMSG_LENGTH(sizeof(*t));
	if (len < 0) {
		fprintf(stderr, "Wrong len %d\n", len);
		return -1;
	}

	if (filter_ifindex && filter_ifindex != t->tcm_ifindex)
		return 0;

	if (filter_handle && filter_handle != t->tcm_handle)
		return 0;

	if (filter_parent && filter_parent != t->tcm_parent)
		return 0;

	parse_rtattr_flags(tb, TCA_MAX, TCA_RTA(t), len, NLA_F_NESTED);

	if (tb[TCA_KIND] == NULL) {
		fprintf(stderr, "print_qdisc: NULL kind\n");
		return -1;
	}

	open_json_object(NULL);

	if (n->nlmsg_type == RTM_DELQDISC)
		print_bool(PRINT_ANY, "deleted", "deleted ", true);

	if (n->nlmsg_type == RTM_NEWQDISC &&
			(n->nlmsg_flags & NLM_F_CREATE) &&
			(n->nlmsg_flags & NLM_F_REPLACE))
		print_bool(PRINT_ANY, "replaced", "replaced ", true);

	if (n->nlmsg_type == RTM_NEWQDISC &&
			(n->nlmsg_flags & NLM_F_CREATE) &&
			(n->nlmsg_flags & NLM_F_EXCL))
		print_bool(PRINT_ANY, "added", "added ", true);

	print_string(PRINT_ANY, "kind", "qdisc %s",
		     rta_getattr_str(tb[TCA_KIND]));
	sprintf(abuf, "%x:", t->tcm_handle >> 16);
	print_string(PRINT_ANY, "handle", " %s", abuf);
	if (show_raw) {
		sprintf(abuf, "[%08x]", t->tcm_handle);
		print_string(PRINT_FP, NULL, "%s", abuf);
	}
	print_string(PRINT_FP, NULL, " ", NULL);

	if (filter_ifindex == 0)
		print_devname(PRINT_ANY, t->tcm_ifindex);

	if (t->tcm_parent == TC_H_ROOT)
		print_bool(PRINT_ANY, "root", "root ", true);
	else if (t->tcm_parent) {
		print_tc_classid(abuf, sizeof(abuf), t->tcm_parent);
		print_string(PRINT_ANY, "parent", "parent %s ", abuf);
	}

	if (t->tcm_info != 1)
		print_uint(PRINT_ANY, "refcnt", "refcnt %u ", t->tcm_info);

	if (tb[TCA_HW_OFFLOAD] &&
	    (rta_getattr_u8(tb[TCA_HW_OFFLOAD])))
		print_bool(PRINT_ANY, "offloaded", "offloaded ", true);

	if (tb[TCA_INGRESS_BLOCK] &&
	    RTA_PAYLOAD(tb[TCA_INGRESS_BLOCK]) >= sizeof(__u32)) {
		__u32 block = rta_getattr_u32(tb[TCA_INGRESS_BLOCK]);

		if (block)
			print_uint(PRINT_ANY, "ingress_block",
				   "ingress_block %u ", block);
	}

	if (tb[TCA_EGRESS_BLOCK] &&
	    RTA_PAYLOAD(tb[TCA_EGRESS_BLOCK]) >= sizeof(__u32)) {
		__u32 block = rta_getattr_u32(tb[TCA_EGRESS_BLOCK]);

		if (block)
			print_uint(PRINT_ANY, "egress_block",
				   "egress_block %u ", block);
	}

	/* pfifo_fast is generic enough to warrant the hardcoding --JHS */
	if (strcmp("pfifo_fast", RTA_DATA(tb[TCA_KIND])) == 0)
		q = get_qdisc_kind("prio");
	else
		q = get_qdisc_kind(RTA_DATA(tb[TCA_KIND]));

	open_json_object("options");
	if (tb[TCA_OPTIONS]) {
		if (q)
			q->print_qopt(q, fp, tb[TCA_OPTIONS]);
		else
			fprintf(stderr, "Cannot parse qdisc parameters\n");
	}
	close_json_object();

	print_nl();

	if (show_details && tb[TCA_STAB]) {
		print_size_table(fp, " ", tb[TCA_STAB]);
		print_nl();
	}

	if (show_stats) {
		struct rtattr *xstats = NULL;

		if (tb[TCA_STATS] || tb[TCA_STATS2] || tb[TCA_XSTATS]) {
			print_tcstats_attr(fp, tb, " ", &xstats);
			print_nl();
		}

		if (q && xstats && q->print_xstats) {
			q->print_xstats(q, fp, xstats);
			print_nl();
		}
	}
	close_json_object();
	fflush(fp);
	return 0;
}

static int tc_qdisc_list(int argc, char **argv)
{
	struct {
		struct nlmsghdr n;
		struct tcmsg t;
		char buf[256];
	} req = {
		.n.nlmsg_type = RTM_GETQDISC,
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcmsg)),
		.t.tcm_family = AF_UNSPEC,
	};

	char d[IFNAMSIZ] = {};
	bool dump_invisible = false;
	__u32 handle;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			strncpy(d, *argv, sizeof(d)-1);
		} else if (strcmp(*argv, "root") == 0) {
			if (filter_parent)
				invarg("parent is already specified", *argv);
			else if (filter_handle)
				invarg("handle is already specified", *argv);
			filter_parent = TC_H_ROOT;
		} else if (strcmp(*argv, "ingress") == 0 ||
				strcmp(*argv, "clsact") == 0) {
			if (filter_parent)
				invarg("parent is already specified", *argv);
			else if (filter_handle)
				invarg("handle is already specified", *argv);
			filter_parent = TC_H_INGRESS;
		} else if (matches(*argv, "parent") == 0) {
			if (filter_parent)
				invarg("parent is already specified", *argv);
			else if (filter_handle)
				invarg("handle is already specified", *argv);
			NEXT_ARG();
			if (get_tc_classid(&handle, *argv))
				invarg("invalid parent ID", *argv);
			filter_parent = handle;
		} else if (matches(*argv, "handle") == 0) {
			if (filter_parent)
				invarg("parent is already specified", *argv);
			else if (filter_handle)
				invarg("handle is already specified", *argv);
			NEXT_ARG();
			if (get_qdisc_handle(&handle, *argv))
				invarg("invalid handle ID", *argv);
			filter_handle = handle;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else if (strcmp(*argv, "invisible") == 0) {
			dump_invisible = true;
		} else {
			fprintf(stderr, "What is \"%s\"? Try \"tc qdisc help\".\n", *argv);
			return -1;
		}

		argc--; argv++;
	}

	ll_init_map(&rth);

	if (d[0]) {
		req.t.tcm_ifindex = ll_name_to_index(d);
		if (!req.t.tcm_ifindex)
			return -nodev(d);
		filter_ifindex = req.t.tcm_ifindex;
	}

	if (dump_invisible) {
		addattr(&req.n, 256, TCA_DUMP_INVISIBLE);
	}

	if (rtnl_dump_request_n(&rth, &req.n) < 0) {
		perror("Cannot send request");
		return 1;
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, print_qdisc, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return 1;
	}
	delete_json_obj();

	return 0;
}

int do_qdisc(int argc, char **argv)
{
	if (argc < 1)
		return tc_qdisc_list(0, NULL);
	if (matches(*argv, "add") == 0)
		return tc_qdisc_modify(RTM_NEWQDISC, NLM_F_EXCL|NLM_F_CREATE, argc-1, argv+1);
	if (matches(*argv, "change") == 0)
		return tc_qdisc_modify(RTM_NEWQDISC, 0, argc-1, argv+1);
	if (matches(*argv, "replace") == 0)
		return tc_qdisc_modify(RTM_NEWQDISC, NLM_F_CREATE|NLM_F_REPLACE, argc-1, argv+1);
	if (matches(*argv, "link") == 0)
		return tc_qdisc_modify(RTM_NEWQDISC, NLM_F_REPLACE, argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return tc_qdisc_modify(RTM_DELQDISC, 0,  argc-1, argv+1);
	if (matches(*argv, "list") == 0 || matches(*argv, "show") == 0
	    || matches(*argv, "lst") == 0)
		return tc_qdisc_list(argc-1, argv+1);
	if (matches(*argv, "help") == 0) {
		usage();
		return 0;
	}
	fprintf(stderr, "Command \"%s\" is unknown, try \"tc qdisc help\".\n", *argv);
	return -1;
}

struct tc_qdisc_block_exists_ctx {
	__u32 block_index;
	bool found;
};

static int tc_qdisc_block_exists_cb(struct nlmsghdr *n, void *arg)
{
	struct tc_qdisc_block_exists_ctx *ctx = arg;
	struct tcmsg *t = NLMSG_DATA(n);
	struct rtattr *tb[TCA_MAX+1];
	int len = n->nlmsg_len;
	struct qdisc_util *q;
	const char *kind;
	int err;

	if (n->nlmsg_type != RTM_NEWQDISC)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*t));
	if (len < 0)
		return -1;

	parse_rtattr_flags(tb, TCA_MAX, TCA_RTA(t), len, NLA_F_NESTED);

	if (tb[TCA_KIND] == NULL)
		return -1;

	if (tb[TCA_INGRESS_BLOCK] &&
	    RTA_PAYLOAD(tb[TCA_INGRESS_BLOCK]) >= sizeof(__u32)) {
		__u32 block = rta_getattr_u32(tb[TCA_INGRESS_BLOCK]);

		if (block == ctx->block_index)
			ctx->found = true;
	}

	if (tb[TCA_EGRESS_BLOCK] &&
	    RTA_PAYLOAD(tb[TCA_EGRESS_BLOCK]) >= sizeof(__u32)) {
		__u32 block = rta_getattr_u32(tb[TCA_EGRESS_BLOCK]);

		if (block == ctx->block_index)
			ctx->found = true;
	}

	kind = rta_getattr_str(tb[TCA_KIND]);
	q = get_qdisc_kind(kind);
	if (!q)
		return -1;
	if (q->has_block) {
		bool found = false;

		err = q->has_block(q, tb[TCA_OPTIONS], ctx->block_index, &found);
		if (err)
			return err;
		if (found)
			ctx->found = true;
	}

	return 0;
}

bool tc_qdisc_block_exists(__u32 block_index)
{
	struct tc_qdisc_block_exists_ctx ctx = { .block_index = block_index };
	struct tcmsg t = { .tcm_family = AF_UNSPEC };

	if (rtnl_dump_request(&rth, RTM_GETQDISC, &t, sizeof(t)) < 0) {
		perror("Cannot send dump request");
		return false;
	}

	if (rtnl_dump_filter(&rth, tc_qdisc_block_exists_cb, &ctx) < 0) {
		perror("Dump terminated\n");
		return false;
	}

	return ctx.found;
}
