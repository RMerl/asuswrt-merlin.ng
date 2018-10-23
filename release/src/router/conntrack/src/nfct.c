/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <errno.h>

#include <libmnl/libmnl.h>

#include "linux_list.h"
#include "nfct.h"

static int nfct_cmd_version(int argc, char *argv[]);
static int nfct_cmd_help(int argc, char *argv[]);

static void usage(char *argv[])
{
	fprintf(stderr, "Usage: %s command subsystem [parameters]...\n",
		argv[0]);
}

void nfct_perror(const char *msg)
{
	if (errno == 0) {
		fprintf(stderr, "nfct v%s: %s\n", VERSION, msg);
	} else {
		fprintf(stderr, "nfct v%s: %s: %s\n",
			VERSION, msg, strerror(errno));
	}
}

static LIST_HEAD(nfct_extension_list);

void nfct_extension_register(struct nfct_extension *ext)
{
	list_add(&ext->head, &nfct_extension_list);
}

static struct nfct_extension *nfct_extension_lookup(int type)
{
	struct nfct_extension *ext;

	list_for_each_entry(ext, &nfct_extension_list, head) {
		if (ext->type == type)
			return ext;
	}
	return NULL;
}

static const char *nfct_cmd_array[NFCT_CMD_MAX] = {
	[NFCT_CMD_LIST]		= "list",
	[NFCT_CMD_ADD]		= "add",
	[NFCT_CMD_DELETE]	= "delete",
	[NFCT_CMD_GET]		= "get",
	[NFCT_CMD_FLUSH]	= "flush",
	[NFCT_CMD_DISABLE]	= "disable",
	[NFCT_CMD_DEFAULT_SET]	= "default-set",
	[NFCT_CMD_DEFAULT_GET]	= "default-get",
};

static int nfct_cmd_parse(const char *cmdstr)
{
	int i;

	for (i = 1; i < NFCT_CMD_MAX; i++) {
		if (strncmp(nfct_cmd_array[i], cmdstr, strlen(cmdstr)) == 0)
			return i;
	}
	return -1;
}

static int nfct_cmd_error(char *argv[])
{
	fprintf(stderr, "nfct v%s: Unknown command: %s\n", VERSION, argv[1]);
	usage(argv);

	return EXIT_FAILURE;
}

static const char *nfct_subsys_array[NFCT_SUBSYS_MAX] = {
	[NFCT_SUBSYS_TIMEOUT]	= "timeout",
	[NFCT_SUBSYS_HELPER]	= "helper",
	[NFCT_SUBSYS_VERSION]	= "version",
	[NFCT_SUBSYS_HELP]	= "help",
};

static int nfct_subsys_parse(const char *cmdstr)
{
	int i;

	for (i = 1; i < NFCT_SUBSYS_MAX; i++) {
		if (strncmp(nfct_subsys_array[i], cmdstr, strlen(cmdstr)) == 0)
			return i;
	}
	return -1;
}

static int nfct_subsys_error(char *argv[])
{
	fprintf(stderr, "nfct v%s: Unknown subsystem: %s\n", VERSION, argv[1]);
	usage(argv);

	return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
	int subsys, cmd, ret = 0;
	struct nfct_extension *ext;
	struct mnl_socket *nl;

	if (argc < 3) {
		usage(argv);
		exit(EXIT_FAILURE);
	}

	cmd = nfct_cmd_parse(argv[1]);
	if (cmd < 0) {
		/* Workaround not to break backward compatibility and to get
		 * the syntax in sync with nft. Old nfct versions allow to
		 * specify the subsystem before the command.
		 */
		subsys = nfct_subsys_parse(argv[1]);
		if (subsys < 0)
			return nfct_subsys_error(argv);

		cmd = nfct_cmd_parse(argv[2]);
		if (cmd < 0)
			return nfct_cmd_error(argv);
	} else {
		subsys = nfct_subsys_parse(argv[2]);
		if (subsys < 0)
			return nfct_subsys_error(argv);
	}

	switch (subsys) {
	case NFCT_SUBSYS_VERSION:
		ret = nfct_cmd_version(argc, argv);
		break;
	case NFCT_SUBSYS_HELP:
		ret = nfct_cmd_help(argc, argv);
		break;
	default:
		ext = nfct_extension_lookup(subsys);
		if (ext == NULL) {
			fprintf(stderr, "nfct v%s: subsystem %s not supported\n",
				VERSION, argv[1]);
			return EXIT_FAILURE;
		}

		nl = nfct_mnl_open();
		if (nl == NULL) {
			nfct_perror("cannot open netlink");
			return -1;
		}

		ret = ext->parse_params(nl, argc, argv, cmd);
		mnl_socket_close(nl);
		break;
	}
	return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

static const char version_msg[] =
	"nfct v%s: utility for the Netfilter's Connection Tracking System\n"
	"Copyright (C) 2012 Pablo Neira Ayuso <pablo@netfilter.org>\n"
	"This program comes with ABSOLUTELY NO WARRANTY.\n"
	"This is free software, and you are welcome to redistribute it under "
	"certain \nconditions; see LICENSE file distributed in this package "
	"for details.\n";

static int nfct_cmd_version(int argc, char *argv[])
{
	printf(version_msg, VERSION);
	return 0;
}

static const char help_msg[] =
	"nfct v%s: utility for the Netfilter's Connection Tracking System\n"
	"Usage: %s command [parameters]...\n\n"
	"Subsystem:\n"
	"  helper\t\tAllows to configure user-space helper\n"
	"  timeout\t\tAllows definition of fine-grain timeout policies\n"
	"  version\t\tDisplay version and disclaimer\n"
	"  help\t\t\tDisplay this help message\n"
	"Commands:\n"
	"  list [reset]\t\tList the accounting object table (and reset)\n"
	"  add object-name\tAdd new accounting object to table\n"
	"  delete object-name\tDelete existing accounting object\n"
	"  get object-name\tGet existing accounting object\n"
	"  disable\t\tDisable queueing packets to userspace for helper inspection\n"
	"  default-set\t\tSet default timeouts\n"
	"  default-get\t\tGet default timeouts\n"
	"  flush\t\t\tFlush accounting object table\n";

static int nfct_cmd_help(int argc, char *argv[])
{
	printf(help_msg, VERSION, argv[0]);
	return 0;
}

int nfct_mnl_talk(struct mnl_socket *nl, struct nlmsghdr *nlh,
		  uint32_t seq, uint32_t portid,
		  int (*cb)(const struct nlmsghdr *nlh, void *data),
		  void *data)
{
	int ret;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
		return -1;

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, cb, data);
		if (ret <= 0)
			break;

		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1)
		return -1;

	return 0;
}

struct mnl_socket *nfct_mnl_open(void)
{
	struct mnl_socket *nl;

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL)
		return NULL;

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
		return NULL;

	return nl;
}
