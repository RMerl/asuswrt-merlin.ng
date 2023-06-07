/*
 * peer.h	TIPC peer functionality.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#ifndef _TIPC_PEER_H
#define _TIPC_PEER_H

extern int help_flag;

int cmd_peer(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
	     void *data);
void cmd_peer_help(struct cmdl *cmdl);

#endif
