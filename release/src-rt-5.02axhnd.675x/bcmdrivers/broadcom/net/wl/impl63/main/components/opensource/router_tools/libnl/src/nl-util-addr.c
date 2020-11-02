/*
 * src/nl-util-addr.c     Address Helper
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>

int main(int argc, char *argv[])
{
	int err;
	char host[256];
	struct nl_addr *a;

	if (argc < 2) {
		fprintf(stderr, "Usage: nl-util-addr <address>\n");
		return -1;
	}

	a = nl_cli_addr_parse(argv[1], AF_UNSPEC);
	err = nl_addr_resolve(a, host, sizeof(host));
	if (err != 0)
		nl_cli_fatal(err, "Unable to resolve address \"%s\": %s",
		      argv[1], nl_geterror(err));

	printf("%s\n", host);

	return 0;
}
