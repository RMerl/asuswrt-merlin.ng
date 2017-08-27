/*
 * m_bpf.c	BPF based action module
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@resnulli.us>
 *              Daniel Borkmann <daniel@iogearbox.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <linux/bpf.h>
#include <linux/tc_act/tc_bpf.h>

#include "utils.h"
#include "rt_names.h"
#include "tc_util.h"
#include "tc_bpf.h"

static const enum bpf_prog_type bpf_type = BPF_PROG_TYPE_SCHED_ACT;

static void explain(void)
{
	fprintf(stderr, "Usage: ... bpf ... [ index INDEX ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "BPF use case:\n");
	fprintf(stderr, " bytecode BPF_BYTECODE\n");
	fprintf(stderr, " bytecode-file FILE\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "eBPF use case:\n");
	fprintf(stderr, " object-file FILE [ section ACT_NAME ] [ export UDS_FILE ]");
	fprintf(stderr, " [ verbose ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where BPF_BYTECODE := \'s,c t f k,c t f k,c t f k,...\'\n");
	fprintf(stderr, "c,t,f,k and s are decimals; s denotes number of 4-tuples\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where FILE points to a file containing the BPF_BYTECODE string,\n");
	fprintf(stderr, "an ELF file containing eBPF map definitions and bytecode.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where ACT_NAME refers to the section name containing the\n");
	fprintf(stderr, "action (default \'%s\').\n", bpf_default_section(bpf_type));
	fprintf(stderr, "\n");
	fprintf(stderr, "Where UDS_FILE points to a unix domain socket file in order\n");
	fprintf(stderr, "to hand off control of all created eBPF maps to an agent.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where optionally INDEX points to an existing action, or\n");
	fprintf(stderr, "explicitly specifies an action index upon creation.\n");
}

static void usage(void)
{
	explain();
	exit(-1);
}

static int parse_bpf(struct action_util *a, int *argc_p, char ***argv_p,
		     int tca_id, struct nlmsghdr *n)
{
	char **argv = *argv_p, bpf_name[256];
	struct rtattr *tail;
	struct tc_act_bpf parm = { 0 };
	struct sock_filter bpf_ops[BPF_MAXINSNS];
	bool ebpf_fill = false, bpf_fill = false;
	bool ebpf = false, seen_run = false;
	const char *bpf_uds_name = NULL;
	const char *bpf_sec_name = NULL;
	char *bpf_obj = NULL;
	int argc = *argc_p, ret = 0;
	__u16 bpf_len = 0;
	__u32 bpf_fd = 0;

	if (matches(*argv, "bpf") != 0)
		return -1;

	NEXT_ARG();

	while (argc > 0) {
		if (matches(*argv, "run") == 0) {
			bool from_file, bpf_verbose;
			int ret;

			NEXT_ARG();
opt_bpf:
			bpf_sec_name = bpf_default_section(bpf_type);
			bpf_verbose = false;
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
				fprintf(stderr, "unexpected \"%s\"\n", *argv);
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
				bpf_obj = basename(bpf_obj);

				snprintf(bpf_name, sizeof(bpf_name), "%s:[%s]",
					 bpf_obj, bpf_sec_name);

				bpf_fd = ret;
				ebpf_fill = true;
			} else {
				bpf_len = ret;
				bpf_fill = true;
			}
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else if (matches(*argv, "index") == 0) {
			break;
		} else {
			if (!seen_run)
				goto opt_bpf;
			break;
		}
		argc--;
		argv++;
	}

	parm.action = TC_ACT_PIPE;
	if (argc) {
		if (matches(*argv, "reclassify") == 0) {
			parm.action = TC_ACT_RECLASSIFY;
			argc--;
			argv++;
		} else if (matches(*argv, "pipe") == 0) {
			parm.action = TC_ACT_PIPE;
			argc--;
			argv++;
		} else if (matches(*argv, "drop") == 0 ||
			   matches(*argv, "shot") == 0) {
			parm.action = TC_ACT_SHOT;
			argc--;
			argv++;
		} else if (matches(*argv, "continue") == 0) {
			parm.action = TC_ACT_UNSPEC;
			argc--;
			argv++;
		} else if (matches(*argv, "pass") == 0) {
			parm.action = TC_ACT_OK;
			argc--;
			argv++;
		}
	}

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&parm.index, *argv, 10)) {
				fprintf(stderr, "bpf: Illegal \"index\"\n");
				return -1;
			}
			argc--;
			argv++;
		}
	}

	tail = NLMSG_TAIL(n);

	addattr_l(n, MAX_MSG, tca_id, NULL, 0);
	addattr_l(n, MAX_MSG, TCA_ACT_BPF_PARMS, &parm, sizeof(parm));

	if (ebpf_fill) {
		addattr32(n, MAX_MSG, TCA_ACT_BPF_FD, bpf_fd);
		addattrstrz(n, MAX_MSG, TCA_ACT_BPF_NAME, bpf_name);
	} else if (bpf_fill) {
		addattr16(n, MAX_MSG, TCA_ACT_BPF_OPS_LEN, bpf_len);
		addattr_l(n, MAX_MSG, TCA_ACT_BPF_OPS, &bpf_ops,
			  bpf_len * sizeof(struct sock_filter));
	}

	tail->rta_len = (char *)NLMSG_TAIL(n) - (char *)tail;

	*argc_p = argc;
	*argv_p = argv;

	if (bpf_uds_name)
		ret = bpf_send_map_fds(bpf_uds_name, bpf_obj);

	return ret;
}

static int print_bpf(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_ACT_BPF_MAX + 1];
	struct tc_act_bpf *parm;
	SPRINT_BUF(action_buf);

	if (arg == NULL)
		return -1;

	parse_rtattr_nested(tb, TCA_ACT_BPF_MAX, arg);

	if (!tb[TCA_ACT_BPF_PARMS]) {
		fprintf(f, "[NULL bpf parameters]");
		return -1;
	}

	parm = RTA_DATA(tb[TCA_ACT_BPF_PARMS]);

	fprintf(f, "bpf ");

	if (tb[TCA_ACT_BPF_NAME])
		fprintf(f, "%s ", rta_getattr_str(tb[TCA_ACT_BPF_NAME]));
	else if (tb[TCA_ACT_BPF_FD])
		fprintf(f, "pfd %u ", rta_getattr_u32(tb[TCA_ACT_BPF_FD]));

	if (tb[TCA_ACT_BPF_OPS] && tb[TCA_ACT_BPF_OPS_LEN]) {
		bpf_print_ops(f, tb[TCA_ACT_BPF_OPS],
			      rta_getattr_u16(tb[TCA_ACT_BPF_OPS_LEN]));
		fprintf(f, " ");
	}

	fprintf(f, "default-action %s\n", action_n2a(parm->action, action_buf,
		sizeof(action_buf)));
	fprintf(f, "\tindex %d ref %d bind %d", parm->index, parm->refcnt,
		parm->bindcnt);

	if (show_stats) {
		if (tb[TCA_ACT_BPF_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_ACT_BPF_TM]);
			print_tm(f, tm);
		}
	}

	fprintf(f, "\n ");

	return 0;
}

struct action_util bpf_action_util = {
	.id = "bpf",
	.parse_aopt = parse_bpf,
	.print_aopt = print_bpf,
};
