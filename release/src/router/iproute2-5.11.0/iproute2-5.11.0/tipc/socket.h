/*
 * socket.h	TIPC socket functionality.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#ifndef _TIPC_SOCKET_H
#define _TIPC_SOCKET_H

extern int help_flag;

void cmd_socket_help(struct cmdl *cmdl);
int cmd_socket(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
		  void *data);

#endif
