/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Get/set/delete bridge with netlink
 *
 * Authors:	Stephen Hemminger <shemminger@vyatta.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>

#include "version.h"
#include "utils.h"
#include "br_common.h"
#include "namespace.h"
#include "color.h"

struct rtnl_handle rth = { .fd = -1 };
int preferred_family = AF_UNSPEC;
int oneline;
int show_stats;
int show_details;
static int color;
int compress_vlans;
int json;
int timestamp;
static const char *batch_file;
int force;

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
"Usage: bridge [ OPTIONS ] OBJECT { COMMAND | help }\n"
"       bridge [ -force ] -batch filename\n"
"where	OBJECT := { link | fdb | mdb | vlan | monitor }\n"
"	OPTIONS := { -V[ersion] | -s[tatistics] | -d[etails] |\n"
"		     -o[neline] | -t[imestamp] | -n[etns] name |\n"
"		     -c[ompressvlans] -color -p[retty] -j[son] }\n");
	exit(-1);
}

static int do_help(int argc, char **argv)
{
	usage();
}


static const struct cmd {
	const char *cmd;
	int (*func)(int argc, char **argv);
} cmds[] = {
	{ "link",	do_link },
	{ "fdb",	do_fdb },
	{ "mdb",	do_mdb },
	{ "vlan",	do_vlan },
	{ "monitor",	do_monitor },
	{ "help",	do_help },
	{ 0 }
};

static int do_cmd(const char *argv0, int argc, char **argv)
{
	const struct cmd *c;

	for (c = cmds; c->cmd; ++c) {
		if (matches(argv0, c->cmd) == 0)
			return c->func(argc-1, argv+1);
	}

	fprintf(stderr,
		"Object \"%s\" is unknown, try \"bridge help\".\n", argv0);
	return -1;
}

static int br_batch_cmd(int argc, char *argv[], void *data)
{
	return do_cmd(argv[0], argc, argv);
}

static int batch(const char *name)
{
	int ret;

	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		return EXIT_FAILURE;
	}

	rtnl_set_strict_dump(&rth);

	ret = do_batch(name, force, br_batch_cmd, NULL);

	rtnl_close(&rth);
	return ret;
}

int
main(int argc, char **argv)
{
	while (argc > 1) {
		const char *opt = argv[1];

		if (strcmp(opt, "--") == 0) {
			argc--; argv++;
			break;
		}
		if (opt[0] != '-')
			break;
		if (opt[1] == '-')
			opt++;

		if (matches(opt, "-help") == 0) {
			usage();
		} else if (matches(opt, "-Version") == 0) {
			printf("bridge utility, %s\n", version);
			exit(0);
		} else if (matches(opt, "-stats") == 0 ||
			   matches(opt, "-statistics") == 0) {
			++show_stats;
		} else if (matches(opt, "-details") == 0) {
			++show_details;
		} else if (matches(opt, "-oneline") == 0) {
			++oneline;
		} else if (matches(opt, "-timestamp") == 0) {
			++timestamp;
		} else if (matches(opt, "-family") == 0) {
			argc--;
			argv++;
			if (argc <= 1)
				usage();
			if (strcmp(argv[1], "inet") == 0)
				preferred_family = AF_INET;
			else if (strcmp(argv[1], "inet6") == 0)
				preferred_family = AF_INET6;
			else if (strcmp(argv[1], "help") == 0)
				usage();
			else
				invarg("invalid protocol family", argv[1]);
		} else if (strcmp(opt, "-4") == 0) {
			preferred_family = AF_INET;
		} else if (strcmp(opt, "-6") == 0) {
			preferred_family = AF_INET6;
		} else if (matches(opt, "-netns") == 0) {
			NEXT_ARG();
			if (netns_switch(argv[1]))
				exit(-1);
		} else if (matches(opt, "-compressvlans") == 0) {
			++compress_vlans;
		} else if (matches_color(opt, &color)) {
		} else if (matches(opt, "-force") == 0) {
			++force;
		} else if (matches(opt, "-json") == 0) {
			++json;
		} else if (matches(opt, "-pretty") == 0) {
			++pretty;
		} else if (matches(opt, "-batch") == 0) {
			argc--;
			argv++;
			if (argc <= 1)
				usage();
			batch_file = argv[1];
		} else {
			fprintf(stderr,
				"Option \"%s\" is unknown, try \"bridge help\".\n",
				opt);
			exit(-1);
		}
		argc--;	argv++;
	}

	_SL_ = oneline ? "\\" : "\n";

	check_enable_color(color, json);

	if (batch_file)
		return batch(batch_file);

	if (rtnl_open(&rth, 0) < 0)
		exit(1);

	rtnl_set_strict_dump(&rth);

	if (argc > 1)
		return do_cmd(argv[1], argc-1, argv+1);

	rtnl_close(&rth);
	usage();
}
