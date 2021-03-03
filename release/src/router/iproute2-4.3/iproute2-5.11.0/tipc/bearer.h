/*
 * bearer.h	TIPC bearer functionality.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#ifndef _TIPC_BEARER_H
#define _TIPC_BEARER_H

#include "cmdl.h"

extern int help_flag;

int cmd_bearer(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl, void *data);
void cmd_bearer_help(struct cmdl *cmdl);

void print_bearer_media(void);
int cmd_get_unique_bearer_name(const struct cmd *cmd, struct cmdl *cmdl,
			       struct opt *opts, char *bname,
			       const struct tipc_sup_media *sup_media);
#endif
