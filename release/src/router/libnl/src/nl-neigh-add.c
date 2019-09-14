/*
 * src/ nl-neigh-add.c     Add a neighbour
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/neigh.h>
#include <netlink/cli/link.h>

static int quiet = 0;

static void print_usage(void)
{
	printf(
	"Usage: nl-neigh-add [OPTION]... NEIGHBOUR\n"
	"\n"
	"Options\n"
	"     --update-only     Do not create neighbour, updates exclusively\n"
	"     --create-only     Do not update neighbour if it exists already.\n"
	" -q, --quiet           Do not print informal notifications\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	"Neighbour Options\n"
	" -a, --addr=ADDR       Destination address of neighbour\n"
	" -l, --lladdr=ADDR     Link layer address of neighbour\n"
	" -d, --dev=DEV         Device the neighbour is connected to\n"
	"     --state=STATE     Neighbour state, (default = permanent)\n"
	"\n"
	"Example\n"
	"  nl-neigh-add --create-only --addr=10.0.0.1 --dev=eth0 \\\n"
	"               --lladdr=AA:BB:CC:DD:EE:FF\n"
	);

	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct rtnl_neigh *neigh;
	struct nl_cache *link_cache;
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};
	int err, ok = 0, nlflags = NLM_F_REPLACE | NLM_F_CREATE;
 
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
 	neigh = nl_cli_neigh_alloc();
 
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_UPDATE_ONLY = 257,
			ARG_CREATE_ONLY = 258,
			ARG_STATE,
		};
		static struct option long_opts[] = {
			{ "update-only", 0, 0, ARG_UPDATE_ONLY },
			{ "create-only", 0, 0, ARG_CREATE_ONLY },
			{ "quiet", 0, 0, 'q' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "addr", 1, 0, 'a' },
			{ "lladdr", 1, 0, 'l' },
			{ "dev", 1, 0, 'd' },
			{ "state", 1, 0, ARG_STATE },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "qhva:l:d:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case ARG_UPDATE_ONLY: nlflags &= ~NLM_F_CREATE; break;
		case ARG_CREATE_ONLY: nlflags |= NLM_F_EXCL; break;
		case 'q': quiet = 1; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'a': ok++; nl_cli_neigh_parse_dst(neigh, optarg); break;
		case 'l': nl_cli_neigh_parse_lladdr(neigh, optarg); break;
		case 'd': nl_cli_neigh_parse_dev(neigh, link_cache, optarg); break;
		case ARG_STATE: nl_cli_neigh_parse_state(neigh, optarg); break;
		}
 	}

	if (!ok)
		print_usage();

	if ((err = rtnl_neigh_add(sock, neigh, nlflags)) < 0)
		nl_cli_fatal(err, "Unable to add neighbour: %s",
			     nl_geterror(err));

	if (!quiet) {
		printf("Added ");
		nl_object_dump(OBJ_CAST(neigh), &dp);
 	}

	return 0;
}
