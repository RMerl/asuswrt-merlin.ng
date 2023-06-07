/*
 * nametable.h	TIPC nametable functionality.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#ifndef _TIPC_NAMETABLE_H
#define _TIPC_NAMETABLE_H

extern int help_flag;

void cmd_nametable_help(struct cmdl *cmdl);
int cmd_nametable(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
		  void *data);

#endif
