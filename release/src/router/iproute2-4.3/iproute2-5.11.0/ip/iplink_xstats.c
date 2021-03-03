/*
 * iplink_stats.c       Extended statistics commands
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Nikolay Aleksandrov <nikolay@cumulusnetworks.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_link.h>
#include <netinet/ether.h>

#include "utils.h"
#include "ip_common.h"

static void print_explain(FILE *f)
{
	fprintf(f, "Usage: ... xstats type TYPE [ ARGS ]\n");
}

int iplink_ifla_xstats(int argc, char **argv)
{
	struct link_util *lu = NULL;
	__u32 filt_mask;

	if (!argc) {
		fprintf(stderr, "xstats: missing argument\n");
		return -1;
	}

	if (matches(*argv, "type") == 0) {
		NEXT_ARG();
		lu = get_link_kind(*argv);
		if (!lu)
			invarg("invalid type", *argv);
	} else if (matches(*argv, "help") == 0) {
		print_explain(stdout);
		return 0;
	} else {
		invarg("unknown argument", *argv);
	}

	if (!lu) {
		print_explain(stderr);
		return -1;
	}

	if (!lu->print_ifla_xstats) {
		fprintf(stderr, "xstats: link type %s doesn't support xstats\n",
			lu->id);
		return -1;
	}

	if (lu->parse_ifla_xstats &&
	    lu->parse_ifla_xstats(lu, argc-1, argv+1))
		return -1;

	if (strstr(lu->id, "_slave"))
		filt_mask = IFLA_STATS_FILTER_BIT(IFLA_STATS_LINK_XSTATS_SLAVE);
	else
		filt_mask = IFLA_STATS_FILTER_BIT(IFLA_STATS_LINK_XSTATS);

	if (rtnl_statsdump_req_filter(&rth, AF_UNSPEC, filt_mask) < 0) {
		perror("Cannont send dump request");
		return -1;
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&rth, lu->print_ifla_xstats, stdout) < 0) {
		delete_json_obj();
		fprintf(stderr, "Dump terminated\n");
		return -1;
	}
	delete_json_obj();

	return 0;
}
