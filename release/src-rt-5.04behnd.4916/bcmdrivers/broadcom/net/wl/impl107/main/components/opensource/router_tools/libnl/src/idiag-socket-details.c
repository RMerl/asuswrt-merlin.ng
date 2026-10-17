/*
 * src/idiag-socket-details.c     List socket details
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation version 2 of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#include <netlink/cli/utils.h>
#include <netlink/idiag/idiagnl.h>
#include <netlink/idiag/msg.h>
#include <linux/netlink.h>

static void print_usage(void)
{
	printf(
"Usage: idiag-socket-details [OPTION]\n"
"\n"
"Options\n"
"     --summary		    Show socket detail summary.\n"
"     --details             Show socket details on multiple lines.\n"
"     --stats               Show full socket statistics.\n"
" -h, --help                Show this help.\n"
" -v, --version             Show versioning information.\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *idiag_cache;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_nl_cb = NULL,
		.dp_fd = stdout,
	};
	int err = 0;

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_INET_DIAG);
	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_SUMMARY = 257,
			ARG_DETAILS = 258,
			ARG_STATS = 259,
			ARG_FAMILY,
		};
		static struct option long_opts[] = {
			{ "details", 0, 0, ARG_DETAILS },
			{ "summary", 0, 0, ARG_SUMMARY },
			{ "stats", 0, 0, ARG_STATS},
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "hv", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case '?': exit(NLE_INVAL);
		case ARG_SUMMARY: params.dp_type = NL_DUMP_LINE; break;
		case ARG_DETAILS: params.dp_type = NL_DUMP_DETAILS; break;
		case ARG_STATS:   params.dp_type = NL_DUMP_STATS; break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		}
	}

	if ((err = idiagnl_msg_alloc_cache(sock, AF_INET, IDIAG_SS_ALL,
					&idiag_cache))) {
		nl_cli_fatal(err, "Unable to allocate idiag msg cache: %s",
				nl_geterror(err));
	}

	nl_cache_mngt_provide(idiag_cache);

	nl_cache_dump_filter(idiag_cache, &params, NULL);

	nl_cache_mngt_unprovide(idiag_cache);
	nl_cache_free(idiag_cache);
	nl_socket_free(sock);

	return 0;
}
