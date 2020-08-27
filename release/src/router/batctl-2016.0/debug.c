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


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "debugfs.h"
#include "functions.h"
#include "sys.h"

const struct debug_table_data batctl_debug_tables[BATCTL_TABLE_NUM] = {
	{
		.opt_long = "neighbors",
		.opt_short = "n",
		.debugfs_name = "neighbors",
		.header_lines = 2,
	},
	{
		.opt_long = "originators",
		.opt_short = "o",
		.debugfs_name = "originators",
		.header_lines = 2,
	},
	{
		.opt_long = "gateways",
		.opt_short = "gwl",
		.debugfs_name = "gateways",
		.header_lines = 1,
	},
	{
		.opt_long = "translocal",
		.opt_short = "tl",
		.debugfs_name = "transtable_local",
		.header_lines = 2,
	},
	{
		.opt_long = "transglobal",
		.opt_short = "tg",
		.debugfs_name = "transtable_global",
		.header_lines = 2,
	},
	{
		.opt_long = "claimtable",
		.opt_short = "cl",
		.debugfs_name = "bla_claim_table",
		.header_lines = 2,
	},
	{
		.opt_long = "backbonetable",
		.opt_short = "bbt",
		.debugfs_name = "bla_backbone_table",
		.header_lines = 2,
	},
	{
		.opt_long = "dat_cache",
		.opt_short = "dc",
		.debugfs_name = "dat_cache",
		.header_lines = 2,
	},
	{
		.opt_long = "nc_nodes",
		.opt_short = "nn",
		.debugfs_name = "nc_nodes",
		.header_lines = 0,
	},
};

static void debug_table_usage(int debug_table)
{
	fprintf(stderr, "Usage: batctl [options] %s|%s [parameters]\n",
	       batctl_debug_tables[debug_table].opt_long, batctl_debug_tables[debug_table].opt_short);
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, " \t -n don't replace mac addresses with bat-host names\n");
	fprintf(stderr, " \t -H don't show the header\n");
	fprintf(stderr, " \t -w [interval] watch mode - refresh the table continuously\n");

	if (debug_table == BATCTL_TABLE_ORIGINATORS) {
		fprintf(stderr, " \t -t timeout interval - don't print originators not seen for x.y seconds \n");
		fprintf(stderr, " \t -i [interface] - show multiif originator table for a specific interface\n");
	}

	if (debug_table == BATCTL_TABLE_TRANSLOCAL ||
	    debug_table == BATCTL_TABLE_TRANSGLOBAL) {
		fprintf(stderr, " \t -u|-m print unicast or multicast mac addresses only\n");
	}
}

int handle_debug_table(char *mesh_iface, int debug_table, int argc, char **argv)
{
	int optchar, read_opt = USE_BAT_HOSTS;
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;
	char *orig_iface = NULL;
	float orig_timeout = 0.0f;
	float watch_interval = 1;
	opterr = 0;

	while ((optchar = getopt(argc, argv, "hnw:t:Humi:")) != -1) {
		switch (optchar) {
		case 'h':
			debug_table_usage(debug_table);
			return EXIT_SUCCESS;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			break;
		case 'w':
			read_opt |= CLR_CONT_READ;
			if (optarg[0] == '-') {
				optind--;
				break;
			}

			if (!sscanf(optarg, "%f", &watch_interval)) {
				fprintf(stderr, "Error - provided argument of '-%c' is not a number\n", optchar);
				return EXIT_FAILURE;
			}
			break;
		case 't':
			if (debug_table != BATCTL_TABLE_ORIGINATORS) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(debug_table);
				return EXIT_FAILURE;
			}

			read_opt |= NO_OLD_ORIGS;
			if (!sscanf(optarg, "%f", &orig_timeout)) {
				fprintf(stderr, "Error - provided argument of '-%c' is not a number\n", optchar);
				return EXIT_FAILURE;
			}
			break;
		case 'H':
			read_opt |= SKIP_HEADER;
			break;
		case 'u':
			if (debug_table != BATCTL_TABLE_TRANSLOCAL &&
			    debug_table != BATCTL_TABLE_TRANSGLOBAL) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(debug_table);
				return EXIT_FAILURE;
			}

			read_opt |= UNICAST_ONLY;
			break;
		case 'm':
			if (debug_table != BATCTL_TABLE_TRANSLOCAL &&
			    debug_table != BATCTL_TABLE_TRANSGLOBAL) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(debug_table);
				return EXIT_FAILURE;
			}

			read_opt |= MULTICAST_ONLY;
			break;
		case 'i':
			if (debug_table != BATCTL_TABLE_ORIGINATORS) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(debug_table);
				return EXIT_FAILURE;
			}

			if (check_mesh_iface_ownership(mesh_iface, optarg) != EXIT_SUCCESS)
				return EXIT_FAILURE;

			orig_iface = optarg;
			break;
		case '?':
			if (optopt == 't') {
				fprintf(stderr, "Error - option '-t' needs a number as argument\n");
			} else if (optopt == 'i') {
				fprintf(stderr, "Error - option '-i' needs an interface as argument\n");
			} else if (optopt == 'w') {
				read_opt |= CLR_CONT_READ;
				break;
			}
			else
				fprintf(stderr, "Error - unrecognised option: '-%c'\n", optopt);

			return EXIT_FAILURE;
		default:
			debug_table_usage(debug_table);
			return EXIT_FAILURE;
		}
	}

	if (read_opt & UNICAST_ONLY && read_opt & MULTICAST_ONLY) {
		fprintf(stderr, "Error - '-u' and '-m' are exclusive options\n");
		debug_table_usage(debug_table);
		return EXIT_FAILURE;
	}

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		fprintf(stderr, "Error - can't mount or find debugfs\n");
		return EXIT_FAILURE;
	}

	if (orig_iface)
		debugfs_make_path(DEBUG_BATIF_PATH_FMT "/", orig_iface, full_path, sizeof(full_path));
	else
		debugfs_make_path(DEBUG_BATIF_PATH_FMT "/", mesh_iface, full_path, sizeof(full_path));
	return read_file(full_path, (char *)batctl_debug_tables[debug_table].debugfs_name,
			 read_opt, orig_timeout, watch_interval,
			 batctl_debug_tables[debug_table].header_lines);
}

int print_routing_algos(void) {
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		fprintf(stderr, "Error - can't mount or find debugfs\n");
		return -1;
	}

	debugfs_make_path(DEBUG_BATIF_PATH_FMT, "", full_path, sizeof(full_path));
	return read_file(full_path, DEBUG_ROUTING_ALGOS, 0, 0, 0, 0);
}

int print_vis_info(char *mesh_iface)
{
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;
	FILE *fp;

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		fprintf(stderr, "Error - can't mount or find debugfs\n");
		return -1;
	}

	debugfs_make_path(DEBUG_BATIF_PATH_FMT "/vis_data", mesh_iface, full_path, sizeof(full_path));
	fp = fopen(full_path, "r");
	if (fp) {
		fclose(fp);
		fprintf(stderr, "Error - batctl version is newer than kernel module - the kernel module still supports\n"
				"vis, but later versions will not. The vis functionality has been moved to the userspace\n"
				"daemon ''alfred''. Please either downgrade to an older (compatible) batctl version or use alfred.\n");
	} else {
		fprintf(stderr, "Error - The installed batctl version and kernel module don't have vis support. The vis functionality\n"
				"has been moved to the userspace daemon ''alfred''.\n");
	}
	return 0;
}

static void log_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] log [parameters]\n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, " \t -n don't replace mac addresses with bat-host names\n");
}

int log_print(char *mesh_iface, int argc, char **argv)
{
	int optchar, res, read_opt = USE_BAT_HOSTS | LOG_MODE;
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;

	while ((optchar = getopt(argc, argv, "hn")) != -1) {
		switch (optchar) {
		case 'h':
			log_usage();
			return EXIT_SUCCESS;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			break;
		default:
			log_usage();
			return EXIT_FAILURE;
		}
	}

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		fprintf(stderr, "Error - can't mount or find debugfs\n");
		return EXIT_FAILURE;
	}

	debugfs_make_path(DEBUG_BATIF_PATH_FMT "/", mesh_iface, full_path, sizeof(full_path));
	res = read_file(full_path, DEBUG_LOG, read_opt, 0, 0, 0);
	return res;
}
