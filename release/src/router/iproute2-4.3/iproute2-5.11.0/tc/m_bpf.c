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

#include <linux/bpf.h>
#include <linux/tc_act/tc_bpf.h>

#include "utils.h"

#include "tc_util.h"
#include "bpf_util.h"

static const enum bpf_prog_type bpf_type = BPF_PROG_TYPE_SCHED_ACT;

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... bpf ... [ index INDEX ]\n"
		"\n"
		"BPF use case:\n"
		" bytecode BPF_BYTECODE\n"
		" bytecode-file FILE\n"
		"\n"
		"eBPF use case:\n"
		" object-file FILE [ section ACT_NAME ] [ export UDS_FILE ]"
		" [ verbose ]\n"
		" object-pinned FILE\n"
		"\n"
		"Where BPF_BYTECODE := \'s,c t f k,c t f k,c t f k,...\'\n"
		"c,t,f,k and s are decimals; s denotes number of 4-tuples\n"
		"\n"
		"Where FILE points to a file containing the BPF_BYTECODE string,\n"
		"an ELF file containing eBPF map definitions and bytecode, or a\n"
		"pinned eBPF program.\n"
		"\n"
		"Where ACT_NAME refers to the section name containing the\n"
		"action (default \'%s\').\n"
		"\n"
		"Where UDS_FILE points to a unix domain socket file in order\n"
		"to hand off control of all created eBPF maps to an agent.\n"
		"\n"
		"Where optionally INDEX points to an existing action, or\n"
		"explicitly specifies an action index upon creation.\n",
		bpf_prog_to_default_section(bpf_type));
}

static void bpf_cbpf_cb(void *nl, const struct sock_filter *ops, int ops_len)
{
	addattr16(nl, MAX_MSG, TCA_ACT_BPF_OPS_LEN, ops_len);
	addattr_l(nl, MAX_MSG, TCA_ACT_BPF_OPS, ops,
		  ops_len * sizeof(struct sock_filter));
}

static void bpf_ebpf_cb(void *nl, int fd, const char *annotation)
{
	addattr32(nl, MAX_MSG, TCA_ACT_BPF_FD, fd);
	addattrstrz(nl, MAX_MSG, TCA_ACT_BPF_NAME, annotation);
}

static const struct bpf_cfg_ops bpf_cb_ops = {
	.cbpf_cb = bpf_cbpf_cb,
	.ebpf_cb = bpf_ebpf_cb,
};

static int bpf_parse_opt(struct action_util *a, int *ptr_argc, char ***ptr_argv,
			 int tca_id, struct nlmsghdr *n)
{
	const char *bpf_obj = NULL, *bpf_uds_name = NULL;
	struct tc_act_bpf parm = {};
	struct bpf_cfg_in cfg = {};
	bool seen_run = false;
	struct rtattr *tail;
	int argc, ret = 0;
	char **argv;

	argv = *ptr_argv;
	argc = *ptr_argc;

	if (matches(*argv, "bpf") != 0)
		return -1;

	NEXT_ARG();

	tail = addattr_nest(n, MAX_MSG, tca_id);

	while (argc > 0) {
		if (matches(*argv, "run") == 0) {
			NEXT_ARG();

			if (seen_run)
				duparg("run", *argv);
opt_bpf:
			seen_run = true;
			cfg.type = bpf_type;
			cfg.argc = argc;
			cfg.argv = argv;

			if (bpf_parse_and_load_common(&cfg, &bpf_cb_ops, n))
				return -1;

			argc = cfg.argc;
			argv = cfg.argv;

			bpf_obj = cfg.object;
			bpf_uds_name = cfg.uds;
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else if (matches(*argv, "index") == 0) {
			break;
		} else {
			if (!seen_run)
				goto opt_bpf;
			break;
		}

		NEXT_ARG_FWD();
	}

	parse_action_control_dflt(&argc, &argv, &parm.action,
				  false, TC_ACT_PIPE);

	if (argc) {
		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&parm.index, *argv, 10)) {
				fprintf(stderr, "bpf: Illegal \"index\"\n");
				return -1;
			}

			NEXT_ARG_FWD();
		}
	}

	addattr_l(n, MAX_MSG, TCA_ACT_BPF_PARMS, &parm, sizeof(parm));
	addattr_nest_end(n, tail);

	if (bpf_uds_name)
		ret = bpf_send_map_fds(bpf_uds_name, bpf_obj);

	*ptr_argc = argc;
	*ptr_argv = argv;

	return ret;
}

static int bpf_print_opt(struct action_util *au, FILE *f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_ACT_BPF_MAX + 1];
	struct tc_act_bpf *parm;
	int d_ok = 0;

	print_string(PRINT_ANY, "kind", "%s ", "bpf");
	if (arg == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_ACT_BPF_MAX, arg);

	if (!tb[TCA_ACT_BPF_PARMS]) {
		fprintf(stderr, "Missing bpf parameters\n");
		return -1;
	}

	parm = RTA_DATA(tb[TCA_ACT_BPF_PARMS]);

	if (tb[TCA_ACT_BPF_NAME])
		print_string(PRINT_ANY, "bpf_name", "%s ",
			     rta_getattr_str(tb[TCA_ACT_BPF_NAME]));
	if (tb[TCA_ACT_BPF_OPS] && tb[TCA_ACT_BPF_OPS_LEN]) {
		bpf_print_ops(tb[TCA_ACT_BPF_OPS],
			      rta_getattr_u16(tb[TCA_ACT_BPF_OPS_LEN]));
		print_string(PRINT_FP, NULL, "%s", " ");
	}

	if (tb[TCA_ACT_BPF_ID])
		d_ok = bpf_dump_prog_info(f,
					  rta_getattr_u32(tb[TCA_ACT_BPF_ID]));
	if (!d_ok && tb[TCA_ACT_BPF_TAG]) {
		SPRINT_BUF(b);

		print_string(PRINT_ANY, "tag", "tag %s ",
			     hexstring_n2a(RTA_DATA(tb[TCA_ACT_BPF_TAG]),
			     RTA_PAYLOAD(tb[TCA_ACT_BPF_TAG]),
			     b, sizeof(b)));
	}

	print_action_control(f, "default-action ", parm->action, _SL_);
	print_uint(PRINT_ANY, "index", "\t index %u", parm->index);
	print_int(PRINT_ANY, "ref", " ref %d", parm->refcnt);
	print_int(PRINT_ANY, "bind", " bind %d", parm->bindcnt);

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
	.id		= "bpf",
	.parse_aopt	= bpf_parse_opt,
	.print_aopt	= bpf_print_opt,
};
