/*
 * tc_monitor.c		"tc monitor".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Jamal Hadi Salim
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "rt_names.h"
#include "utils.h"
#include "tc_util.h"
#include "tc_common.h"


static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: tc [-timestamp [-tshort] monitor\n");
	exit(-1);
}


static int accept_tcmsg(struct rtnl_ctrl_data *ctrl,
			struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;

	if (timestamp)
		print_timestamp(fp);

	if (n->nlmsg_type == RTM_NEWTFILTER ||
	    n->nlmsg_type == RTM_DELTFILTER ||
	    n->nlmsg_type == RTM_NEWCHAIN ||
	    n->nlmsg_type == RTM_DELCHAIN) {
		print_filter(n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWTCLASS || n->nlmsg_type == RTM_DELTCLASS) {
		print_class(n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWQDISC || n->nlmsg_type == RTM_DELQDISC) {
		print_qdisc(n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_GETACTION || n->nlmsg_type == RTM_NEWACTION ||
	    n->nlmsg_type == RTM_DELACTION) {
		print_action(n, arg);
		return 0;
	}
	if (n->nlmsg_type != NLMSG_ERROR && n->nlmsg_type != NLMSG_NOOP &&
	    n->nlmsg_type != NLMSG_DONE) {
		fprintf(fp, "Unknown message: length %08d type %08x flags %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
	}
	return 0;
}

int do_tcmonitor(int argc, char **argv)
{
	struct rtnl_handle rth;
	char *file = NULL;
	unsigned int groups = nl_mgrp(RTNLGRP_TC);

	while (argc > 0) {
		if (matches(*argv, "file") == 0) {
			NEXT_ARG();
			file = *argv;
		} else {
			if (matches(*argv, "help") == 0) {
				usage();
			} else {
				fprintf(stderr, "Argument \"%s\" is unknown, try \"tc monitor help\".\n", *argv);
				exit(-1);
			}
		}
		argc--;	argv++;
	}

	if (file) {
		FILE *fp = fopen(file, "r");
		int ret;

		if (fp == NULL) {
			perror("Cannot fopen");
			exit(-1);
		}

		ret = rtnl_from_file(fp, accept_tcmsg, stdout);
		fclose(fp);
		return ret;
	}

	if (rtnl_open(&rth, groups) < 0)
		exit(1);

	ll_init_map(&rth);

	if (rtnl_listen(&rth, accept_tcmsg, (void *)stdout) < 0) {
		rtnl_close(&rth);
		exit(2);
	}

	rtnl_close(&rth);
	exit(0);
}
