/*
 * Copyright (C) 2009-2016  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
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



#ifndef _BATCTL_BAT_HOSTS_H
#define _BATCTL_BAT_HOSTS_H

#include <net/ethernet.h>

#define HOST_NAME_MAX_LEN 50
#define CONF_DIR_LEN 256


struct bat_host {
	struct ether_addr mac_addr;
	char name[HOST_NAME_MAX_LEN];
} __attribute__((packed));

void bat_hosts_init(int read_opt);
struct bat_host *bat_hosts_find_by_name(char *name);
struct bat_host *bat_hosts_find_by_mac(char *mac);
void bat_hosts_free(void);

#endif
