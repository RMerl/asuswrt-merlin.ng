/*
 * src/nf-exp-delete.c     Delete an expectation
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 * Copyright (c) 2012 Rich Fought <rich.fought@watchguard.com>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/exp.h>

static int quiet = 0;

static void print_usage(void)
{
	printf(
	"Usage: nf-exp-list [OPTION]... [CONNTRACK ENTRY]\n"
	"\n"
	"Options\n"
	"     --replace             Replace the address if it exists.\n"
	" -q, --quiet               Do not print informal notifications.\n"
	" -h, --help                Show this help\n"
	" -v, --version             Show versioning information\n"
	"\n"
	"Expectation Selection\n"
	" -i, --id=NUM                Identifier\n"
	"     --expect-proto=PROTOCOL Expectation protocol\n"
	"     --expect-src=ADDR       Expectation source address\n"
	"     --expect-sport=PORT     Expectation source port\n"
	"     --expect-dst=ADDR       Expectation destination address\n"
	"     --expect-dport=PORT     Expectation destination port\n"
	"     --master-proto=PROTOCOL Master conntrack protocol\n"
	"     --master-src=ADDR       Master conntrack source address\n"
	"     --master-sport=PORT     Master conntrack source port\n"
	"     --master-dst=ADDR       Master conntrack destination address\n"
	"     --master-dport=PORT     Master conntrack destination port\n"
	"     --mask-proto=PROTOCOL   Mask protocol\n"
	"     --mask-src=ADDR         Mask source address\n"
	"     --mask-sport=PORT       Mask source port\n"
	"     --mask-dst=ADDR         Mask destination address\n"
	"     --mask-dport=PORT       Mask destination port\n"
	" -F, --family=FAMILY         Address family\n"
	"     --timeout=NUM           Timeout value\n"
	"     --helper=STRING         Helper Name\n"
	"     --flags                 Flags\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nfnl_exp *exp;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err, nlflags = 0;

 	exp = nl_cli_exp_alloc();
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_MARK = 270,
			ARG_TCP_STATE = 271,
			ARG_EXPECT_PROTO,
			ARG_EXPECT_SRC,
			ARG_EXPECT_SPORT,
			ARG_EXPECT_DST,
			ARG_EXPECT_DPORT,
			ARG_MASTER_PROTO,
			ARG_MASTER_SRC,
			ARG_MASTER_SPORT,
			ARG_MASTER_DST,
			ARG_MASTER_DPORT,
 			ARG_MASK_PROTO,
 			ARG_MASK_SRC,
			ARG_MASK_SPORT,
			ARG_MASK_DST,
			ARG_MASK_DPORT,
			ARG_TIMEOUT,
 			ARG_HELPER_NAME,
			ARG_FLAGS,
		};
		static struct option long_opts[] = {
 			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "id", 1, 0, 'i' },
			{ "expect-proto", 1, 0, ARG_EXPECT_PROTO },
			{ "expect-src", 1, 0, ARG_EXPECT_SRC },
			{ "expect-sport", 1, 0, ARG_EXPECT_SPORT },
			{ "expect-dst", 1, 0, ARG_EXPECT_DST },
			{ "expect-dport", 1, 0, ARG_EXPECT_DPORT },
			{ "master-proto", 1, 0, ARG_MASTER_PROTO },
			{ "master-src", 1, 0, ARG_MASTER_SRC },
			{ "master-sport", 1, 0, ARG_MASTER_SPORT },
			{ "master-dst", 1, 0, ARG_MASTER_DST },
			{ "master-dport", 1, 0, ARG_MASTER_DPORT },
			{ "mask-proto", 1, 0, ARG_MASK_PROTO },
 			{ "mask-src", 1, 0, ARG_MASK_SRC },
 			{ "mask-sport", 1, 0, ARG_MASK_SPORT },
 			{ "mask-dst", 1, 0, ARG_MASK_DST },
 			{ "mask-dport", 1, 0, ARG_MASK_DPORT },
			{ "family", 1, 0, 'F' },
			{ "timeout", 1, 0, ARG_TIMEOUT },
			{ "helper", 1, 0, ARG_HELPER_NAME },
 			{ "flags", 1, 0, ARG_FLAGS},
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "46f:hvi:p:F:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case '?': exit(NLE_INVAL);
		case 'q': quiet = 1; break;
		case '4': nfnl_exp_set_family(exp, AF_INET); break;
		case '6': nfnl_exp_set_family(exp, AF_INET6); break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'i': nl_cli_exp_parse_id(exp, optarg); break;
		case ARG_EXPECT_PROTO: nl_cli_exp_parse_l4protonum(exp, NFNL_EXP_TUPLE_EXPECT, optarg); break;
		case ARG_EXPECT_SRC: nl_cli_exp_parse_src(exp, NFNL_EXP_TUPLE_EXPECT, optarg); break;
		case ARG_EXPECT_SPORT: nl_cli_exp_parse_src_port(exp, NFNL_EXP_TUPLE_EXPECT, optarg); break;
		case ARG_EXPECT_DST: nl_cli_exp_parse_dst(exp, NFNL_EXP_TUPLE_EXPECT, optarg); break;
		case ARG_EXPECT_DPORT: nl_cli_exp_parse_dst_port(exp, NFNL_EXP_TUPLE_EXPECT, optarg); break;
		case ARG_MASTER_PROTO: nl_cli_exp_parse_l4protonum(exp, NFNL_EXP_TUPLE_MASTER, optarg); break;
		case ARG_MASTER_SRC: nl_cli_exp_parse_src(exp, NFNL_EXP_TUPLE_MASTER, optarg); break;
		case ARG_MASTER_SPORT: nl_cli_exp_parse_src_port(exp, NFNL_EXP_TUPLE_MASTER, optarg); break;
		case ARG_MASTER_DST: nl_cli_exp_parse_dst(exp, NFNL_EXP_TUPLE_MASTER, optarg); break;
		case ARG_MASTER_DPORT: nl_cli_exp_parse_dst_port(exp, NFNL_EXP_TUPLE_MASTER, optarg); break;
		case ARG_MASK_PROTO: nl_cli_exp_parse_l4protonum(exp, NFNL_EXP_TUPLE_MASK, optarg); break;
		case ARG_MASK_SRC: nl_cli_exp_parse_src(exp, NFNL_EXP_TUPLE_MASK, optarg); break;
		case ARG_MASK_SPORT: nl_cli_exp_parse_src_port(exp, NFNL_EXP_TUPLE_MASK, optarg); break;
		case ARG_MASK_DST: nl_cli_exp_parse_dst(exp, NFNL_EXP_TUPLE_MASK, optarg); break;
 		case ARG_MASK_DPORT: nl_cli_exp_parse_dst_port(exp, NFNL_EXP_TUPLE_MASK, optarg); break;
		case 'F': nl_cli_exp_parse_family(exp, optarg); break;
		case ARG_TIMEOUT: nl_cli_exp_parse_timeout(exp, optarg); break;
		case ARG_HELPER_NAME: nl_cli_exp_parse_helper_name(exp, optarg); break;
		case ARG_FLAGS: nl_cli_exp_parse_flags(exp, optarg); break;
		}
 	}

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_NETFILTER);

	if ((err = nfnl_exp_del(sock, exp, nlflags)) < 0)
		nl_cli_fatal(err, "Unable to delete expectation: %s", nl_geterror(err));

	if (!quiet) {
		printf("Deleted ");
		nl_object_dump(OBJ_CAST(exp), &params);
	}

	return 0;
}
