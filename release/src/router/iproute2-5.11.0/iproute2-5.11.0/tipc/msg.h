/*
 * msg.h	Messaging (netlink) helper functions.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#ifndef _TIPC_MSG_H
#define _TIPC_MSG_H

struct nlmsghdr *msg_init(char *buf, int cmd);
int msg_doit(struct nlmsghdr *nlh, mnl_cb_t callback, void *data);
int msg_dumpit(struct nlmsghdr *nlh, mnl_cb_t callback, void *data);
int parse_attrs(const struct nlattr *attr, void *data);

#endif
