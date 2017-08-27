/*
 * src/nl-classid-lookup.c     Lookup classid
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>

static void print_usage(void)
{
	printf(
"Usage: nl-classid-lookup [OPTIONS]... NAME\n"
"\n"
"OPTIONS\n"
" -h, --help                Show this help text.\n"
" -v, --version             Show versioning information.\n"
" -r, --reverse             Do a reverse lookup, i.e. classid to name.\n"
"     --raw                 Print the raw classid, not pretty printed.\n"
"\n"
"EXAMPLE\n"
"   $ nl-classid-lookup low_latency\n"
"   $ nl-classid-lookup -r 1:12\n"
"\n"
	);
	exit(0);
}

int main(int argc, char *argv[])
{
	uint32_t classid;
	char *name;
	int err, reverse = 0, raw = 0;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_RAW = 257,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "reverse", 0, 0, 'r' },
			{ "raw", 0, 0, ARG_RAW },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "hvr", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'r': reverse = 1; break;
		case ARG_RAW: raw = 1; break;
		}
 	}

	if (optind >= argc)
		print_usage();

	name = argv[optind++];

	/*
	 * We use rtnl_tc_str2handle() even while doing a reverse lookup. This
	 * allows for name -> name lookups. This is intentional, it does not
	 * do any harm and avoids duplicating a lot of code.
	 */
	if ((err = rtnl_tc_str2handle(name, &classid)) < 0)
		nl_cli_fatal(err, "Unable to lookup classid \"%s\": %s",
			     name, nl_geterror(err));

	if (reverse) {
		char buf[64];
		printf("%s\n", rtnl_tc_handle2str(classid, buf, sizeof(buf)));
	} else if (raw)
		printf("%#x\n", classid);
	else
		printf("%x:%x\n", TC_H_MAJ(classid) >> 16, TC_H_MIN(classid));

	return 0;
}
