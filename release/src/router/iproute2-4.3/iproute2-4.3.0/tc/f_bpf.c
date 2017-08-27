/*
 * f_bpf.c	BPF-based Classifier
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Daniel Borkmann <dborkman@redhat.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <linux/filter.h>
#include <linux/if.h>

#include "utils.h"
#include "tc_util.h"
#include "tc_bpf.h"

static const enum bpf_prog_type bpf_type = BPF_PROG_TYPE_SCHED_CLS;

static void explain(void)
{
	fprintf(stderr, "Usage: ... bpf ...\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "BPF use case:\n");
	fprintf(stderr, " bytecode BPF_BYTECODE\n");
	fprintf(stderr, " bytecode-file FILE\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "eBPF use case:\n");
	fprintf(stderr, " object-file FILE [ section CLS_NAME ] [ export UDS_FILE ]");
	fprintf(stderr, " [ verbose ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Common remaining options:\n");
	fprintf(stderr, " [ action ACTION_SPEC ]\n");
	fprintf(stderr, " [ classid CLASSID ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where BPF_BYTECODE := \'s,c t f k,c t f k,c t f k,...\'\n");
	fprintf(stderr, "c,t,f,k and s are decimals; s denotes number of 4-tuples\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where FILE points to a file containing the BPF_BYTECODE string,\n");
	fprintf(stderr, "an ELF file containing eBPF map definitions and bytecode.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where CLS_NAME refers to the section name containing the\n");
	fprintf(stderr, "classifier (default \'%s\').\n", bpf_default_section(bpf_type));
	fprintf(stderr, "\n");
	fprintf(stderr, "Where UDS_FILE points to a unix domain socket file in order\n");
	fprintf(stderr, "to hand off control of all created eBPF maps to an agent.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "ACTION_SPEC := ... look at individual actions\n");
	fprintf(stderr, "NOTE: CLASSID is parsed as hexadecimal input.\n");
}

static int bpf_parse_opt(struct filter_util *qu, char *handle,
			 int argc, char **argv, struct nlmsghdr *n)
{
	struct tcmsg *t = NLMSG_DATA(n);
	const char *bpf_uds_name = NULL;
	const char *bpf_sec_name = NULL;
	char *bpf_obj = NULL;
	struct rtattr *tail;
	bool seen_run = false;
	long h = 0;
	int ret = 0;

	if (argc == 0)
		return 0;

	if (handle) {
		h = strtol(handle, NULL, 0);
		if (h == LONG_MIN || h == LONG_MAX) {
			fprintf(stderr, "Illegal handle \"%s\", must be "
				"numeric.\n", handle);
			return -1;
		}
	}

	t->tcm_handle = h;

	tail = (struct rtattr *)(((void *)n) + NLMSG_ALIGN(n->nlmsg_len));
	addattr_l(n, MAX_MSG, TCA_OPTIONS, NULL, 0);

	while (argc > 0) {
		if (matches(*argv, "run") == 0) {
			struct sock_filter bpf_ops[BPF_MAXINSNS];
			bool from_file, ebpf, bpf_verbose;
			int ret;

			NEXT_ARG();
opt_bpf:
			bpf_sec_name = bpf_default_section(bpf_type);
			bpf_verbose = false;
			ebpf = false;
			seen_run = true;

			if (strcmp(*argv, "bytecode-file") == 0 ||
			    strcmp(*argv, "bcf") == 0) {
				from_file = true;
			} else if (strcmp(*argv, "bytecode") == 0 ||
				   strcmp(*argv, "bc") == 0) {
				from_file = false;
			} else if (strcmp(*argv, "object-file") == 0 ||
				   strcmp(*argv, "obj") == 0) {
				ebpf = true;
			} else {
				fprintf(stderr, "What is \"%s\"?\n", *argv);
				explain();
				return -1;
			}

			NEXT_ARG();
			if (ebpf) {
				bpf_uds_name = getenv(BPF_ENV_UDS);
				bpf_obj = *argv;
				NEXT_ARG();

				if (strcmp(*argv, "section") == 0 ||
				    strcmp(*argv, "sec") == 0) {
					NEXT_ARG();
					bpf_sec_name = *argv;
					NEXT_ARG();
				}
				if (!bpf_uds_name &&
				    (strcmp(*argv, "export") == 0 ||
				     strcmp(*argv, "exp") == 0)) {
					NEXT_ARG();
					bpf_uds_name = *argv;
					NEXT_ARG();
				}
				if (strcmp(*argv, "verbose") == 0 ||
				    strcmp(*argv, "verb") == 0) {
					bpf_verbose = true;
					NEXT_ARG();
				}

				PREV_ARG();
			}

			ret = ebpf ? bpf_open_object(bpf_obj, bpf_type, bpf_sec_name,
						     bpf_verbose) :
				     bpf_parse_ops(argc, argv, bpf_ops, from_file);
			if (ret < 0) {
				fprintf(stderr, "%s\n", ebpf ?
					"Could not load object" :
					"Illegal \"bytecode\"");
				return -1;
			}

			if (ebpf) {
				char bpf_name[256];

				bpf_obj = basename(bpf_obj);

				snprintf(bpf_name, sizeof(bpf_name), "%s:[%s]",
					 bpf_obj, bpf_sec_name);

				addattr32(n, MAX_MSG, TCA_BPF_FD, ret);
				addattrstrz(n, MAX_MSG, TCA_BPF_NAME, bpf_name);
			} else {
				addattr16(n, MAX_MSG, TCA_BPF_OPS_LEN, ret);
				addattr_l(n, MAX_MSG, TCA_BPF_OPS, &bpf_ops,
					  ret * sizeof(struct sock_filter));
			}
		} else if (matches(*argv, "classid") == 0 ||
			   strcmp(*argv, "flowid") == 0) {
			unsigned int handle;

			NEXT_ARG();
			if (get_tc_classid(&handle, *argv)) {
				fprintf(stderr, "Illegal \"classid\"\n");
				return -1;
			}
			addattr_l(n, MAX_MSG, TCA_BPF_CLASSID, &handle, 4);
		} else if (matches(*argv, "action") == 0) {
			NEXT_ARG();
			if (parse_action(&argc, &argv, TCA_BPF_ACT, n)) {
				fprintf(stderr, "Illegal \"action\"\n");
				return -1;
			}
			continue;
		} else if (matches(*argv, "police") == 0) {
			NEXT_ARG();
			if (parse_police(&argc, &argv, TCA_BPF_POLICE, n)) {
				fprintf(stderr, "Illegal \"police\"\n");
				return -1;
			}
			continue;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			if (!seen_run)
				goto opt_bpf;

			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--;
		argv++;
	}

	tail->rta_len = (((void *)n) + n->nlmsg_len) - (void *)tail;

	if (bpf_uds_name)
		ret = bpf_send_map_fds(bpf_uds_name, bpf_obj);

	return ret;
}

static int bpf_print_opt(struct filter_util *qu, FILE *f,
			 struct rtattr *opt, __u32 handle)
{
	struct rtattr *tb[TCA_BPF_MAX + 1];

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_BPF_MAX, opt);

	if (handle)
		fprintf(f, "handle 0x%x ", handle);

	if (tb[TCA_BPF_CLASSID]) {
		SPRINT_BUF(b1);
		fprintf(f, "flowid %s ",
			sprint_tc_classid(rta_getattr_u32(tb[TCA_BPF_CLASSID]), b1));
	}

	if (tb[TCA_BPF_NAME])
		fprintf(f, "%s ", rta_getattr_str(tb[TCA_BPF_NAME]));
	else if (tb[TCA_BPF_FD])
		fprintf(f, "pfd %u ", rta_getattr_u32(tb[TCA_BPF_FD]));

	if (tb[TCA_BPF_OPS] && tb[TCA_BPF_OPS_LEN]) {
		bpf_print_ops(f, tb[TCA_BPF_OPS],
			      rta_getattr_u16(tb[TCA_BPF_OPS_LEN]));
		fprintf(f, "\n");
	}

	if (tb[TCA_BPF_POLICE]) {
		fprintf(f, "\n");
		tc_print_police(f, tb[TCA_BPF_POLICE]);
	}

	if (tb[TCA_BPF_ACT]) {
		tc_print_action(f, tb[TCA_BPF_ACT]);
	}

	return 0;
}

struct filter_util bpf_filter_util = {
	.id = "bpf",
	.parse_fopt = bpf_parse_opt,
	.print_fopt = bpf_print_opt,
};
