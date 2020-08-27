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

#ifndef _BATCTL_DEBUG_H
#define _BATCTL_DEBUG_H

#include <stddef.h>
#include "main.h"

#define DEBUG_BATIF_PATH_FMT "%s/batman_adv/%s"
#define DEBUG_TRANSTABLE_GLOBAL "transtable_global"
#define DEBUG_LOG "log"
#define DEBUG_ROUTING_ALGOS "routing_algos"

enum batctl_debug_tables {
	BATCTL_TABLE_NEIGHBORS,
	BATCTL_TABLE_ORIGINATORS,
	BATCTL_TABLE_GATEWAYS,
	BATCTL_TABLE_TRANSLOCAL,
	BATCTL_TABLE_TRANSGLOBAL,
	BATCTL_TABLE_BLA_CLAIMS,
	BATCTL_TABLE_BLA_BACKBONES,
	BATCTL_TABLE_DAT,
	BATCTL_TABLE_NETWORK_CODING_NODES,
	BATCTL_TABLE_NUM,
};

struct debug_table_data {
       const char opt_long[OPT_LONG_MAX_LEN];
       const char opt_short[OPT_SHORT_MAX_LEN];
       const char debugfs_name[DEBUG_TABLE_PATH_MAX_LEN];
       size_t header_lines;
};

extern const struct debug_table_data batctl_debug_tables[BATCTL_TABLE_NUM];

int handle_debug_table(char *mesh_iface, int debug_table, int argc, char **argv);
int log_print(char *mesh_iface, int argc, char **argv);
int print_routing_algos(void);
int print_vis_info(char *mesh_iface);

#endif
