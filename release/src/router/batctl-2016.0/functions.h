/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#ifndef _BATCTL_FUNCTIONS_H
#define _BATCTL_FUNCTIONS_H

#include <net/ethernet.h>
#include <stddef.h>


#define ETH_STR_LEN 17
#define BATMAN_ADV_TAG "batman-adv:"

/* return time delta from start to end in milliseconds */
void start_timer(void);
double end_timer(void);
char *ether_ntoa_long(const struct ether_addr *addr);
char *get_name_by_macaddr(struct ether_addr *mac_addr, int read_opt);
char *get_name_by_macstr(char *mac_str, int read_opt);
int file_exists(const char *fpath);
int read_file(const char *dir, const char *path, int read_opt,
	      float orig_timeout, float watch_interval, size_t header_lines);
int write_file(const char *dir, const char *fname, const char *arg1,
	       const char *arg2);
struct ether_addr *translate_mac(char *mesh_iface, struct ether_addr *mac);
struct ether_addr *resolve_mac(const char *asc);
int vlan_get_link(const char *ifname, char **parent);

extern char *line_ptr;

enum {
	NO_FLAGS = 0x00,
	CONT_READ = 0x01,
	CLR_CONT_READ = 0x02,
	USE_BAT_HOSTS = 0x04,
	LOG_MODE = 0x08,
	USE_READ_BUFF = 0x10,
	SILENCE_ERRORS = 0x20,
	NO_OLD_ORIGS = 0x40,
	COMPAT_FILTER = 0x80,
	SKIP_HEADER = 0x100,
	UNICAST_ONLY = 0x200,
	MULTICAST_ONLY = 0x400,
};

#endif
