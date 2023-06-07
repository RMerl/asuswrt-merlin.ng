/*
 * em_ipt.c		IPtables extensions matching Ematch
 *
 * (C) 2018 Eyal Birger <eyal.birger@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <getopt.h>

#include <linux/tc_ematch/tc_em_ipt.h>
#include <linux/pkt_cls.h>
#include <xtables.h>
#include "m_ematch.h"

static void em_ipt_print_usage(FILE *fd)
{
	fprintf(fd,
		"Usage: ipt([-6] -m MATCH_NAME [MATCH_OPTS])\n"
		"Example: 'ipt(-m policy --reqid 1 --pol ipsec --dir in)'\n");
}

static struct option original_opts[] = {
	{
		.name = "match",
		.has_arg = 1,
		.val = 'm'
	},
	{
		.name = "ipv6",
		.val = '6'
	},
	{}
};

static struct xtables_globals em_tc_ipt_globals = {
	.option_offset = 0,
	.program_name = "tc-em-ipt",
	.program_version = "0.1",
	.orig_opts = original_opts,
	.opts = original_opts,
#if (XTABLES_VERSION_CODE >= 11)
	.compat_rev = xtables_compatible_revision,
#endif
};

static struct xt_entry_match *fake_xt_entry_match(int data_size, void *data)
{
	struct xt_entry_match *m;

	m = xtables_calloc(1, XT_ALIGN(sizeof(*m)) + data_size);
	if (!m)
		return NULL;

	if (data)
		memcpy(m->data, data, data_size);

	m->u.user.match_size = data_size;
	return m;
}

static void scrub_match(struct xtables_match *match)
{
	match->mflags = 0;
	free(match->m);
	match->m = NULL;
}

/* IPv4 and IPv6 share the same hooking enumeration */
#define HOOK_PRE_ROUTING 0
#define HOOK_POST_ROUTING 4

static __u32 em_ipt_hook(struct nlmsghdr *n)
{
	struct tcmsg *t = NLMSG_DATA(n);

	if (t->tcm_parent != TC_H_ROOT &&
	    t->tcm_parent == TC_H_MAJ(TC_H_INGRESS))
		return HOOK_PRE_ROUTING;

	return HOOK_POST_ROUTING;
}

static int em_ipt_parse_eopt_argv(struct nlmsghdr *n,
				  struct tcf_ematch_hdr *hdr,
				  int argc, char **argv)
{
	struct xtables_globals tmp_tcipt_globals = em_tc_ipt_globals;
	struct xtables_match *match = NULL;
	__u8 nfproto = NFPROTO_IPV4;

	while (1) {
		struct option *opts;
		int c;

		c = getopt_long(argc, argv, "6m:", tmp_tcipt_globals.opts,
				NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'm':
			xtables_init_all(&tmp_tcipt_globals, nfproto);

			match = xtables_find_match(optarg, XTF_TRY_LOAD, NULL);
			if (!match || !match->x6_parse) {
				fprintf(stderr, " failed to find match %s\n\n",
					optarg);
				return -1;
			}

			match->m = fake_xt_entry_match(match->size, NULL);
			if (!match->m) {
				printf(" %s error\n", match->name);
				return -1;
			}

			if (match->init)
				match->init(match->m);

			opts = xtables_options_xfrm(tmp_tcipt_globals.orig_opts,
						    tmp_tcipt_globals.opts,
						    match->x6_options,
						    &match->option_offset);
			if (!opts) {
				scrub_match(match);
				return -1;
			}

			tmp_tcipt_globals.opts = opts;
			break;

		case '6':
			nfproto = NFPROTO_IPV6;
			break;

		default:
			if (!match) {
				fprintf(stderr, "failed to find match %s\n\n",
					optarg);
				return -1;

			}
			xtables_option_mpcall(c, argv, 0, match, NULL);
			break;
		}
	}

	if (!match) {
		fprintf(stderr, " failed to parse parameters (%s)\n", *argv);
		return -1;
	}

	/* check that we passed the correct parameters to the match */
	xtables_option_mfcall(match);

	addraw_l(n, MAX_MSG, hdr, sizeof(*hdr));
	addattr32(n, MAX_MSG, TCA_EM_IPT_HOOK, em_ipt_hook(n));
	addattrstrz(n, MAX_MSG, TCA_EM_IPT_MATCH_NAME, match->name);
	addattr8(n, MAX_MSG, TCA_EM_IPT_MATCH_REVISION, match->revision);
	addattr8(n, MAX_MSG, TCA_EM_IPT_NFPROTO, nfproto);
	addattr_l(n, MAX_MSG, TCA_EM_IPT_MATCH_DATA, match->m->data,
		  match->size);

	xtables_free_opts(1);

	scrub_match(match);
	return 0;
}

static int em_ipt_print_epot(FILE *fd, struct tcf_ematch_hdr *hdr, void *data,
			     int data_len)
{
	struct rtattr *tb[TCA_EM_IPT_MAX + 1];
	struct xtables_match *match;
	const char *mname;
	__u8 nfproto;

	if (parse_rtattr(tb, TCA_EM_IPT_MAX, data, data_len) < 0)
		return -1;

	nfproto = rta_getattr_u8(tb[TCA_EM_IPT_NFPROTO]);

	xtables_init_all(&em_tc_ipt_globals, nfproto);

	mname = rta_getattr_str(tb[TCA_EM_IPT_MATCH_NAME]);
	match = xtables_find_match(mname, XTF_TRY_LOAD, NULL);
	if (!match)
		return -1;

	match->m = fake_xt_entry_match(RTA_PAYLOAD(tb[TCA_EM_IPT_MATCH_DATA]),
				       RTA_DATA(tb[TCA_EM_IPT_MATCH_DATA]));
	if (!match->m)
		return -1;

	match->print(NULL, match->m, 0);

	scrub_match(match);
	return 0;
}

struct ematch_util ipt_ematch_util = {
	.kind = "ipt",
	.kind_num = TCF_EM_IPT,
	.parse_eopt_argv = em_ipt_parse_eopt_argv,
	.print_eopt = em_ipt_print_epot,
	.print_usage = em_ipt_print_usage
};
