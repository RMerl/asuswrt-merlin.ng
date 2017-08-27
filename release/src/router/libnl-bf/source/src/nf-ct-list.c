/*
 * src/nf-ct-list.c     List Conntrack Entries
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/ct.h>

static void print_usage(void)
{
	printf(
	"Usage: nf-ct-list [OPTION]... [CONNTRACK ENTRY]\n"
	"\n"
	"Options\n"
	" -f, --format=TYPE     Output format { brief | details | stats }\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	"Conntrack Selection\n"
	" -i, --id=NUM            Identifier\n"
	" -p, --proto=PROTOCOL    Protocol\n"
	"     --tcp-state=STATE   TCP connection state\n"
	"     --orig-src=ADDR     Original source address\n"
	"     --orig-sport=PORT   Original source port\n"
	"     --orig-dst=ADDR     Original destination address\n"
	"     --orig-dport=PORT   Original destination port\n"
	"     --reply-src=ADDR    Reply source address\n"
	"     --reply-sport=PORT  Reply source port\n"
	"     --reply-dst=ADDR    Reply destination address\n"
	"     --reply-dport=PORT  Reply destination port\n"
	" -F, --family=FAMILY     Address family\n"
	"     --mark=NUM          Mark value\n"
	"     --timeout=NUM       Timeout value\n"
	"     --refcnt=NUM        Use counter value\n"
	"     --flags             Flags\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *ct_cache;
	struct nfnl_ct *ct;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
 
 	ct = nl_cli_ct_alloc();
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_MARK = 257,
			ARG_TCP_STATE = 258,
			ARG_ORIG_SRC,
			ARG_ORIG_SPORT,
			ARG_ORIG_DST,
			ARG_ORIG_DPORT,
			ARG_REPLY_SRC,
			ARG_REPLY_SPORT,
			ARG_REPLY_DST,
			ARG_REPLY_DPORT,
			ARG_TIMEOUT,
			ARG_REFCNT,
			ARG_FLAGS,
		};
		static struct option long_opts[] = {
			{ "format", 1, 0, 'f' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "id", 1, 0, 'i' },
			{ "proto", 1, 0, 'p' },
			{ "tcp-state", 1, 0, ARG_TCP_STATE },
			{ "orig-src", 1, 0, ARG_ORIG_SRC },
			{ "orig-sport", 1, 0, ARG_ORIG_SPORT },
			{ "orig-dst", 1, 0, ARG_ORIG_DST },
			{ "orig-dport", 1, 0, ARG_ORIG_DPORT },
			{ "reply-src", 1, 0, ARG_REPLY_SRC },
			{ "reply-sport", 1, 0, ARG_REPLY_SPORT },
			{ "reply-dst", 1, 0, ARG_REPLY_DST },
			{ "reply-dport", 1, 0, ARG_REPLY_DPORT },
			{ "family", 1, 0, 'F' },
			{ "mark", 1, 0, ARG_MARK },
			{ "timeout", 1, 0, ARG_TIMEOUT },
			{ "refcnt", 1, 0, ARG_REFCNT },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "46f:hvi:p:F:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case '?': exit(NLE_INVAL);
		case '4': nfnl_ct_set_family(ct, AF_INET); break;
		case '6': nfnl_ct_set_family(ct, AF_INET6); break;
		case 'f': params.dp_type = nl_cli_parse_dumptype(optarg); break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'i': nl_cli_ct_parse_id(ct, optarg); break;
		case 'p': nl_cli_ct_parse_protocol(ct, optarg); break;
		case ARG_TCP_STATE: nl_cli_ct_parse_tcp_state(ct, optarg); break;
		case ARG_ORIG_SRC: nl_cli_ct_parse_src(ct, 0, optarg); break;
		case ARG_ORIG_SPORT: nl_cli_ct_parse_src_port(ct, 0, optarg); break;
		case ARG_ORIG_DST: nl_cli_ct_parse_dst(ct, 0, optarg); break;
		case ARG_ORIG_DPORT: nl_cli_ct_parse_dst_port(ct, 0, optarg); break;
		case ARG_REPLY_SRC: nl_cli_ct_parse_src(ct, 1, optarg); break;
		case ARG_REPLY_SPORT: nl_cli_ct_parse_src_port(ct, 1, optarg); break;
		case ARG_REPLY_DST: nl_cli_ct_parse_dst(ct, 1, optarg); break;
		case ARG_REPLY_DPORT: nl_cli_ct_parse_dst_port(ct, 1, optarg); break;
		case 'F': nl_cli_ct_parse_family(ct, optarg); break;
		case ARG_MARK: nl_cli_ct_parse_mark(ct, optarg); break;
		case ARG_TIMEOUT: nl_cli_ct_parse_timeout(ct, optarg); break;
		case ARG_REFCNT: nl_cli_ct_parse_use(ct, optarg); break;
		case ARG_FLAGS: nl_cli_ct_parse_status(ct, optarg); break;
		}
 	}

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_NETFILTER);
	ct_cache = nl_cli_ct_alloc_cache(sock);

	nl_cache_dump_filter(ct_cache, &params, OBJ_CAST(ct));

	return 0;
}
