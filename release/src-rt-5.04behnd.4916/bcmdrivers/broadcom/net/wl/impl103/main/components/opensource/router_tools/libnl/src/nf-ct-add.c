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

static int quiet = 0;

static void print_usage(void)
{
	printf(
	"Usage: nf-ct-add [OPTION]... [CONNTRACK ENTRY]\n"
	"\n"
	"Options\n"
	" -q, --quiet           Do not print informal notifications.\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	"Conntrack Selection\n"
	" -p, --proto=PROTOCOL    Protocol\n"
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
	"     --status            Bitset representing status of connection.\n"
	"     --zone=NUM          Zone value\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nfnl_ct *ct;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err, nlflags = NLM_F_CREATE;

	ct = nl_cli_ct_alloc();

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_ORIG_SRC = 257,
			ARG_ORIG_SPORT = 258,
			ARG_ORIG_DST,
			ARG_ORIG_DPORT,
			ARG_REPLY_SRC,
			ARG_REPLY_SPORT,
			ARG_REPLY_DST,
			ARG_REPLY_DPORT,
			ARG_MARK,
			ARG_TIMEOUT,
			ARG_STATUS,
			ARG_ZONE,
		};
		static struct option long_opts[] = {
			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "proto", 1, 0, 'p' },
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
			{ "status", 1, 0, ARG_STATUS },
			{ "zone", 1, 0, ARG_ZONE },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "46q:hv:p:F:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case '?': exit(NLE_INVAL);
		case 'q': quiet = 1; break;
		case '4': nfnl_ct_set_family(ct, AF_INET); break;
		case '6': nfnl_ct_set_family(ct, AF_INET6); break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'p': nl_cli_ct_parse_protocol(ct, optarg); break;
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
		case ARG_STATUS: nl_cli_ct_parse_status(ct, optarg); break;
		case ARG_ZONE: nl_cli_ct_parse_zone(ct, optarg); break;
		}
	}

	if (!quiet) {
		printf("Adding ");
		nl_object_dump(OBJ_CAST(ct), &params);
	}

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_NETFILTER);

	if ((err = nfnl_ct_add(sock, ct, nlflags)) < 0)
		nl_cli_fatal(err, "Unable to add conntrack: %s", nl_geterror(err));

	if (!quiet) {
		printf("Added ");
		nl_object_dump(OBJ_CAST(ct), &params);
	}

	return 0;
}
