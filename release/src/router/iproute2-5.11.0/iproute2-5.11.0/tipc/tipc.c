/*
 * tipc.	TIPC utility frontend.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include "bearer.h"
#include "link.h"
#include "nametable.h"
#include "socket.h"
#include "media.h"
#include "node.h"
#include "peer.h"
#include "cmdl.h"
#include "utils.h"

int help_flag;
int json;

static void about(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Transparent Inter-Process Communication Protocol\n"
		"Usage: %s [OPTIONS] COMMAND [ARGS] ...\n"
		"\n"
		"Options:\n"
		" -h, --help \t\tPrint help for last given command\n"
		" -j, --json \t\tJson format printouts\n"
		" -p, --pretty \t\tpretty print\n"
		"\n"
		"Commands:\n"
		" bearer                - Show or modify bearers\n"
		" link                  - Show or modify links\n"
		" media                 - Show or modify media\n"
		" nametable             - Show nametable\n"
		" node                  - Show or modify node related parameters\n"
		" peer                  - Peer related operations\n"
		" socket                - Show sockets\n",
		cmdl->argv[0]);
}

int main(int argc, char *argv[])
{
	int i;
	int res;
	struct cmdl cmdl;
	const struct cmd cmd = {"tipc", NULL, about};
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"json", no_argument, 0, 'j'},
		{"pretty", no_argument, 0, 'p'},
		{0, 0, 0, 0}
	};
	const struct cmd cmds[] = {
		{ "bearer",	cmd_bearer,	cmd_bearer_help},
		{ "link",	cmd_link,	cmd_link_help},
		{ "media",	cmd_media,	cmd_media_help},
		{ "nametable",	cmd_nametable,	cmd_nametable_help},
		{ "node",	cmd_node,	cmd_node_help},
		{ "peer",	cmd_peer,	cmd_peer_help},
		{ "socket",	cmd_socket,	cmd_socket_help},
		{ NULL }
	};

	do {
		int option_index = 0;

		i = getopt_long(argc, argv, "hjp", long_options, &option_index);

		switch (i) {
		case 'h':
			/*
			 * We want the help for the last command, so we flag
			 * here in order to print later.
			 */
			help_flag = 1;
			break;
		case 'j':
			/*
			 * Enable json format printouts
			 */
			json = 1;
			break;
		case 'p':
			/*
			 * Enable json pretty output
			 */
			pretty = 1;
			break;
		case -1:
			/* End of options */
			break;
		default:
			/* Invalid option, error msg is printed by getopts */
			return 1;
		}
	} while (i != -1);

	cmdl.optind = optind;
	cmdl.argc = argc;
	cmdl.argv = argv;

	if ((res = run_cmd(NULL, &cmd, cmds, &cmdl, NULL)) != 0)
		return 1;

	return 0;
}
