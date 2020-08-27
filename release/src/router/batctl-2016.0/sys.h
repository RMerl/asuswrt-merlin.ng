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

#ifndef _BATCTL_SYS_H
#define _BATCTL_SYS_H

#include "main.h"

#define SYS_BATIF_PATH_FMT	"/sys/class/net/%s/mesh/"
#define SYS_LOG_LEVEL		"log_level"
#define SYS_LOG			"log"
#define SYS_GW_MODE		"gw_mode"
#define SYS_GW_SEL		"gw_sel_class"
#define SYS_GW_BW		"gw_bandwidth"
#define SYS_IFACE_PATH		"/sys/class/net"
#define SYS_IFACE_DIR		SYS_IFACE_PATH"/%s/"
#define SYS_MESH_IFACE_FMT	SYS_IFACE_PATH"/%s/batman_adv/mesh_iface"
#define SYS_IFACE_STATUS_FMT	SYS_IFACE_PATH"/%s/batman_adv/iface_status"
#define SYS_VLAN_PATH		SYS_IFACE_PATH"/%s/mesh/vlan%d/"
#define SYS_ROUTING_ALGO_FMT	SYS_IFACE_PATH"/%s/mesh/routing_algo"
#define SYS_SELECTED_RA_PATH	"/sys/module/batman_adv/parameters/routing_algo"
#define VLAN_ID_MAX_LEN		4

enum batctl_settings_list {
	BATCTL_SETTINGS_ORIG_INTERVAL,
	BATCTL_SETTINGS_AP_ISOLATION,
	BATCTL_SETTINGS_BLA,
	BATCTL_SETTINGS_DAT,
	BATCTL_SETTINGS_AGGREGATION,
	BATCTL_SETTINGS_BONDING,
	BATCTL_SETTINGS_FRAGMENTATION,
	BATCTL_SETTINGS_NETWORK_CODING,
	BATCTL_SETTINGS_ISOLATION_MARK,
	BATCTL_SETTINGS_MULTICAST_MODE,
	BATCTL_SETTINGS_NUM,
};

enum gw_modes {
	GW_MODE_OFF,
	GW_MODE_CLIENT,
	GW_MODE_SERVER,
};

struct settings_data {
	const char opt_long[OPT_LONG_MAX_LEN];
	const char opt_short[OPT_SHORT_MAX_LEN];
	const char sysfs_name[SETTINGS_PATH_MAX_LEN];
	const char **params;
};

extern const char *sysfs_param_enable[];
extern const char *sysfs_param_server[];
extern const struct settings_data batctl_settings[BATCTL_SETTINGS_NUM];

int interface(char *mesh_iface, int argc, char **argv);
int handle_loglevel(char *mesh_iface, int argc, char **argv);
int handle_sys_setting(char *mesh_iface, int setting, int argc, char **argv);
int handle_gw_setting(char *mesh_iface, int argc, char **argv);
int handle_ra_setting(int argc, char **argv);
int check_mesh_iface(char *mesh_iface);
int check_mesh_iface_ownership(char *mesh_iface, char *hard_iface);

#endif
