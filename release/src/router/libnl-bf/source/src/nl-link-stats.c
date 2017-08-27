/*
 * src/nl-link-stats.c     Retrieve link statistics
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>

static void print_usage(void)
{
	printf(
	"Usage: nl-link-stats [OPTION]... [LINK] [ListOfStats]\n"
	"\n"
	"Options\n"
	" -l, --list            List available statistic names\n"
	" -h, --help            Show this help\n"
	" -v, --version         Show versioning information\n"
	"\n"
	"Link Options\n"
	" -n, --name=NAME	link name\n"
	" -i, --index=NUM       interface index\n"
	);
	exit(0);
}

static void list_stat_names(void)
{
	char buf[64];
	int i;

	for (i = 0; i < RTNL_LINK_STATS_MAX; i++)
		printf("%s\n", rtnl_link_stat2str(i, buf, sizeof(buf)));

	exit(0);
}

static int gargc;

static void dump_stat(struct rtnl_link *link, int id)
{
	uint64_t st = rtnl_link_get_stat(link, id);
	char buf[64];

	printf("%s.%s %" PRIu64 "\n", rtnl_link_get_name(link),
	       rtnl_link_stat2str(id, buf, sizeof(buf)), st);
}

static void dump_stats(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	char **argv = arg;

	if (optind >= gargc) {
		int i;

		for (i = 0; i < RTNL_LINK_STATS_MAX; i++)
			dump_stat(link, i);
	} else {
		while (optind < gargc) {
			int id = rtnl_link_str2stat(argv[optind]);

			if (id < 0)
				fprintf(stderr, "Warning: Unknown statistic "
					"\"%s\"\n", argv[optind]);
			else
				dump_stat(link, id);

			optind++;
		}
	}
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *link_cache;
	struct rtnl_link *link;

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	link = nl_cli_link_alloc();

	for (;;) {
		int c, optidx = 0;
		static struct option long_opts[] = {
			{ "list", 0, 0, 'l' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "name", 1, 0, 'n' },
			{ "index", 1, 0, 'i' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "lhvn:i:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'l': list_stat_names(); break;
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'n': nl_cli_link_parse_name(link, optarg); break;
		case 'i': nl_cli_link_parse_ifindex(link, optarg); break;
		}
	}

	gargc = argc;
	nl_cache_foreach_filter(link_cache, OBJ_CAST(link), dump_stats, argv);

	return 0;
}

