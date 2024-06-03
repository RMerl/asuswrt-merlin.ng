/*
 * WPA Supplicant / main() function for UNIX like OSes and MinGW
 * Copyright (c) 2003-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  main
 *
 *  Abstract:
 *  main() function
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     Derived from WPA Supplicant / main()
 * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif /* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "wapp_usr_intf_ctrl.h"
#include "client_db.h"
#include "mapd_i.h"
#include "wapp_if.h"
#ifdef SUPPORT_MULTI_AP
char g_map_cfg_path[MAX_FILE_PATH_LENGTH];
#ifdef MAP_R2
char g_map_1905_cfg_path[MAX_FILE_PATH_LENGTH];
#define _1905_MAP_CFG_FILE "/etc/map/1905d.cfg"
#endif
#endif
char g_mapd_strng_path[MAX_FILE_PATH_LENGTH];


static void usage(void)
{
#if 0
	printf("%s\n\n%s\n"
	       "usage:\n"
	       "  mapd [-BddhKLqq"
#ifdef CONFIG_DEBUG_SYSLOG
	       "s"
#endif /* CONFIG_DEBUG_SYSLOG */
	       "t"
	       "vW] [-P<pid file>] "
	       "[-g<global ctrl>] \\\n"
	       "        [-G<group>] \\\n"
	       "        -i<ifname> -c<config file> [-C<ctrl>] [-D<driver>] "
	       "[-p<driver_param>] \\\n"
	       "        [-b<br_ifname>] [-e<entropy file>]"
#ifdef CONFIG_DEBUG_FILE
	       " [-f<debug file>]"
#endif /* CONFIG_DEBUG_FILE */
	       " \\\n"
	       "        [-o<override driver>] [-O<override ctrl>] \\\n"
	       "        [-N -i<ifname> -c<conf> [-C<ctrl>] "
	       "[-D<driver>] \\\n"
	       "        [-p<driver_param>] [-b<br_ifname>] [-I<config file>] "
	       "...]\n"
	       "\n"
	       "drivers:\n",

#ifndef CONFIG_NO_STDOUT_DEBUG
	printf("options:\n"
	       "  -b = optional bridge interface name\n"
	       "  -B = run daemon in the background\n"
	       "  -c = Configuration file\n"
	       "  -C = ctrl_interface parameter (only used if -c is not)\n"
	       "  -d = increase debugging verbosity (-dd even more)\n"
	       "  -D = driver name (can be multiple drivers: nl80211,wext)\n"
	       "  -e = entropy file\n"
#ifdef CONFIG_DEBUG_FILE
	       "  -f = log output to debug file instead of stdout\n"
#endif /* CONFIG_DEBUG_FILE */
	       "  -g = global ctrl_interface\n"
	       "  -G = global ctrl_interface group\n"
	       "  -h = show this help text\n"
	       "  -i = interface name\n"
	       "  -I = additional configuration file\n"
	       "  -K = include keys (passwords, etc.) in debug output\n"
	       "  -N = start describing new interface\n"
	       "  -o = override driver parameter for new interfaces\n"
	       "  -O = override ctrl_interface parameter for new interfaces\n"
	       "  -p = driver parameters\n"
	       "  -P = PID file\n"
	       "  -q = decrease debugging verbosity (-qq even less)\n"
#ifdef CONFIG_DEBUG_SYSLOG
	       "  -s = log output to syslog instead of stdout\n"
#endif /* CONFIG_DEBUG_SYSLOG */
	       "  -t = include timestamp in debug messages\n"
#ifdef CONFIG_DEBUG_LINUX_TRACING
	       "  -T = record to Linux tracing in addition to logging\n"
	       "       (records all messages regardless of debug verbosity)\n"
#endif /* CONFIG_DEBUG_LINUX_TRACING */
	       "  -v = show version\n"
	       "  -W = wait for a control interface monitor before starting\n");

	printf("example:\n"
	       "  mapd -c/etc/clientDB.txt\n");
#endif /* CONFIG_NO_STDOUT_DEBUG */
#endif
}


static void mapd_fd_workaround(int start)
{
#ifdef __linux__
	static int fd[3] = { -1, -1, -1 };
	int i;
	/* When started from pcmcia-cs scripts, mapd might start with
	 * fd 0, 1, and 2 closed. This will cause some issues because many
	 * places in mapd are still printing out to stdout. As a
	 * workaround, make sure that fd's 0, 1, and 2 are not used for other
	 * sockets. */
	if (start) {
		for (i = 0; i < 3; i++) {
			fd[i] = open("/dev/null", O_RDWR);
			if (fd[i] > 2) {
				close(fd[i]);
				fd[i] = -1;
				break;
			}
		}
	} else {
		for (i = 0; i < 3; i++) {
			if (fd[i] >= 0) {
				close(fd[i]);
				fd[i] = -1;
			}
		}
	}
#endif /* __linux__ */
}


int main(int argc, char *argv[])
{
	int c;
	int exitcode = -1;
	struct mapd_params params;
	struct mapd_global *global;
	os_memset(&params, 0, sizeof(params));
	params.mapd_debug_level = MSG_ERROR;
#ifdef SUPPORT_MULTI_AP
	params.Certification = 0;
#endif
	mapd_fd_workaround(1);
#ifdef SUPPORT_MULTI_AP
	os_memcpy(g_map_cfg_path, "/etc/map/mapd_cfg", os_strlen("/etc/map/mapd_cfg"));
#ifdef MAP_R2
	os_memcpy(g_map_1905_cfg_path, _1905_MAP_CFG_FILE, os_strlen(_1905_MAP_CFG_FILE));
#endif
#endif
	os_memcpy(g_mapd_strng_path, "/etc/mapd_strng.conf", os_strlen("/etc/mapd_strng.conf"));
	for (;;) {
		c = getopt(argc, argv,
#ifdef SUPPORT_MULTI_AP
			   "b:Bc:C:D:de:f:g:G:hi:I:KLMm:No:O:p:P:qsTtuvW");
#else
			   "b:Bc:C:D:de:f:g:hi:KLMm:No:O:p:P:qsTtuvW");
#endif
		if (c < 0)
			break;
		switch (c) {
		case 'B':
			params.daemonize++;
			break;
#ifdef CONFIG_CLIENT_DB_FILE
		case 'c':
			params.clientDBname = optarg;
			break;
#endif
		case 'C':
			params.core_dump++;
			break;
		case 'd':
			params.mapd_debug_level--;
			break;
#ifdef CONFIG_DEBUG_FILE
		case 'f':
			params.mapd_debug_file_path = optarg;
			break;
#endif /* CONFIG_DEBUG_FILE */
		case 'g':
			params.ctrl_interface = optarg;
			break;
		case 'h':
			usage();
			exitcode = 0;
			goto out;
		case 'P':
			os_free(params.pid_file);
			params.pid_file = os_rel2abs_path(optarg);
			break;
		case 'q':
			params.mapd_debug_level++;
			break;
#ifdef CONFIG_DEBUG_SYSLOG
		case 's':
			params.mapd_debug_syslog++;
			break;
#endif /* CONFIG_DEBUG_SYSLOG */
		case 't':
			params.mapd_debug_timestamp++;
			break;
		case 'v':
			always("%s\n", "TEST");
			exitcode = 0;
			goto out;
		case 'M':
#ifdef SUPPORT_MULTI_AP
			params.Certification = 1;
#endif
			params.mapd_debug_level = MSG_ERROR;
			break;
#ifdef SUPPORT_MULTI_AP
		case 'I':
			if(os_strlen(optarg)< MAX_FILE_PATH_LENGTH)
			{
				os_memset(g_map_cfg_path,0,sizeof(g_map_cfg_path));
				os_memcpy(g_map_cfg_path, optarg, os_strlen(optarg));
			}
			break;
#ifdef MAP_R2
		case 'K':
			if(os_strlen(optarg) < MAX_FILE_PATH_LENGTH) {
				os_memset(g_map_1905_cfg_path, 0, sizeof(g_map_1905_cfg_path));
				os_memcpy(g_map_1905_cfg_path, optarg, os_strlen(g_map_1905_cfg_path));
			}
			break;
#endif
#endif
		case 'O':
			if(os_strlen(optarg)< MAX_FILE_PATH_LENGTH)
			{
				os_memset(g_mapd_strng_path,0,sizeof(g_mapd_strng_path));
				os_memcpy(g_mapd_strng_path, optarg, os_strlen(optarg));
			}
			break;
		default:
			usage();
			exitcode = 0;
			goto out;
		}
	}

	exitcode = 0;
	global = mapd_init(&params);
	if (global == NULL) {
		mapd_printf(MSG_ERROR, "Failed to initialize mapd");
		exitcode = -1;
		goto out;
	} else {
		mapd_printf(MSG_INFO, "Successfully initialized "
			   "mapd");
	}

	if (exitcode == 0) {
	    /* Ready to GO */
		exitcode = mapd_run(global);
    }

	mapd_deinit(global);

out:
	mapd_fd_workaround(0);
	os_free(params.pid_file);

	os_program_deinit();

	return exitcode;
}
