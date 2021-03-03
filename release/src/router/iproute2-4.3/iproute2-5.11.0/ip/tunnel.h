/*
 * Copyright (C)2006 USAGI/WIDE Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 */
/*
 * Author:
 *	Masahide NAKAMURA @USAGI
 */
#ifndef __TUNNEL_H__
#define __TUNNEL_H__ 1

#include <stdbool.h>
#include <linux/types.h>

struct rtattr;
struct ifinfomsg;

extern struct rtnl_handle rth;

struct tnl_print_nlmsg_info {
	const struct ifinfomsg *ifi;
	const void *p1;
	void *p2;

	void (*init)(const struct tnl_print_nlmsg_info *info);
	bool (*match)(const struct tnl_print_nlmsg_info *info);
	void (*print)(const void *t);
};

int do_tunnels_list(struct tnl_print_nlmsg_info *info);

const char *tnl_strproto(__u8 proto);

int tnl_get_ioctl(const char *basedev, void *p);
int tnl_add_ioctl(int cmd, const char *basedev, const char *name, void *p);
int tnl_del_ioctl(const char *basedev, const char *name, void *p);
int tnl_prl_ioctl(int cmd, const char *name, void *p);
int tnl_6rd_ioctl(int cmd, const char *name, void *p);
int tnl_ioctl_get_6rd(const char *name, void *p);
__be32 tnl_parse_key(const char *name, const char *key);
void tnl_print_encap(struct rtattr *tb[],
		     int encap_type, int encap_flags,
		     int encap_sport, int encap_dport);
void tnl_print_endpoint(const char *name,
			const struct rtattr *rta, int family);
void tnl_print_gre_flags(__u8 proto,
			 __be16 i_flags, __be16 o_flags,
			 __be32 i_key, __be32 o_key);

#endif
