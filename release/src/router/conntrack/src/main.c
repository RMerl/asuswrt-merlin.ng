/*
 * (C) 2006-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Vyatta Inc. <http://www.vyatta.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "conntrackd.h"
#include "log.h"
#include "helper.h"
#include "systemd.h"
#include "resync.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

struct ct_general_state st;
struct ct_state state;

static const char usage_general_commands[] =
	"General commands:\n"
	"  -d, run in daemon mode\n"
	"  -C [configfile], configuration file path\n"
	"  -v, display conntrackd version\n"
	"  -h, display this help information\n";

static const char usage_client_commands[] =
	"Client mode commands:\n"
	"  -c [ct|expect], commit external cache to conntrack table\n"
	"  -f [internal|external], flush internal and external cache\n"
	"  -F [ct|expect], flush kernel conntrack table\n"
	"  -i [ct|expect], display content of the internal cache\n"
	"  -e [ct|expect], display the content of the external cache\n"
	"  -k, kill conntrack daemon\n"
	"  -s  [network|cache|runtime|link|rsqueue|queue|ct|expect], "
		"dump statistics\n"
	"  -R [ct|expect], resync with kernel conntrack table\n"
	"  -n, request resync with other node (only FT-FW and NOTRACK modes)\n"
	"  -B, force a bulk send to other replica firewalls\n"
	"  -x, dump cache in XML format (requires -i or -e)\n"
	"  -t, reset the kernel timeout (see PurgeTimeout clause)\n";

static void
show_usage(char *progname)
{
	fprintf(stdout, "Connection tracking userspace daemon v%s\n", VERSION);
	fprintf(stdout, "Usage: %s [commands] [options]\n\n", progname);
	fprintf(stdout, "%s\n", usage_general_commands);
	fprintf(stdout, "%s\n", usage_client_commands);
}

static void
show_version(void)
{
	fprintf(stdout, "Connection tracking userspace daemon v%s. ", VERSION);
	fprintf(stdout, "Licensed under GPLv2.\n");
	fprintf(stdout, "(C) 2006-2009 Pablo Neira Ayuso ");
	fprintf(stdout, "<pablo@netfilter.org>\n");
}

static void
set_operation_mode(int *current, int want, char *argv[])
{
	if (*current == NOT_SET) {
		*current = want;
		return;
	}
	if (*current != want) {
		show_usage(argv[0]);
		dlog(LOG_ERR, "Invalid parameters");
		exit(EXIT_FAILURE);
	}
}

static int
set_action_by_table(int i, int argc, char *argv[],
		    int ct_action, int exp_action, int dfl_action, int *action)
{
	if (i+1 < argc && argv[i+1][0] != '-') {
		if (strncmp(argv[i+1], "ct", strlen(argv[i+1])) == 0) {
			*action = ct_action;
			i++;
		} else if (strncmp(argv[i+1], "expect",
						strlen(argv[i+1])) == 0) {
			*action = exp_action;
			i++;
		}
	} else
		*action = dfl_action;

	return i;
}

static void
do_chdir(const char *d)
{
	if (chdir(d))
		dlog(LOG_WARNING, "Cannot change current directory to %s: %s",
		     d, strerror(errno));
}

int main(int argc, char *argv[])
{
	int ret, i, action = -1;
	char config_file[PATH_MAX] = {};
	int type = 0;
	struct utsname u;
	int version, major, minor;

	/* Check kernel version: it must be >= 2.6.18 */
	if (uname(&u) == -1) {
		dlog(LOG_ERR, "Can't retrieve kernel version via uname()");
		exit(EXIT_FAILURE);
	}
	sscanf(u.release, "%d.%d.%d", &version, &major, &minor);
	if (version < 2 && major < 6 && minor < 18) {
		dlog(LOG_ERR, "Linux kernel version must be >= 2.6.18");
		exit(EXIT_FAILURE);
	}

	for (i=1; i<argc; i++) {
		switch(argv[i][1]) {
		case 'd':
			set_operation_mode(&type, DAEMON, argv);
			CONFIG(running_mode) = DAEMON;
			break;
		case 'c':
			set_operation_mode(&type, REQUEST, argv);
			i = set_action_by_table(i, argc, argv,
						CT_COMMIT, EXP_COMMIT,
						ALL_COMMIT, &action);
			break;
		case 'i':
			set_operation_mode(&type, REQUEST, argv);
			i = set_action_by_table(i, argc, argv,
						CT_DUMP_INTERNAL,
						EXP_DUMP_INTERNAL,
						CT_DUMP_INTERNAL, &action);
			break;
		case 'e':
			set_operation_mode(&type, REQUEST, argv);
			i = set_action_by_table(i, argc, argv,
						CT_DUMP_EXTERNAL,
						EXP_DUMP_EXTERNAL,
						CT_DUMP_EXTERNAL, &action);
			break;
		case 'C':
			if (++i < argc) {
				strncpy(config_file, argv[i], PATH_MAX);
				if (strlen(argv[i]) >= PATH_MAX){
					config_file[PATH_MAX-1]='\0';
					dlog(LOG_WARNING, "Path to config file"
					     " to long. Cutting it down to %d"
					     " characters", PATH_MAX);
				}
				break;
			}
			show_usage(argv[0]);
			dlog(LOG_ERR, "Missing config filename");
			break;
		case 'F':
			set_operation_mode(&type, REQUEST, argv);
			i = set_action_by_table(i, argc, argv,
						CT_FLUSH_MASTER,
						EXP_FLUSH_MASTER,
						ALL_FLUSH_MASTER, &action);
			break;
		case 'f':
			set_operation_mode(&type, REQUEST, argv);
			if (i+1 < argc && argv[i+1][0] != '-') {
				if (strncmp(argv[i+1], "internal",
					    strlen(argv[i+1])) == 0) {
					action = CT_FLUSH_INT_CACHE;
					i++;
				} else if (strncmp(argv[i+1], "external",
						 strlen(argv[i+1])) == 0) {
					action = CT_FLUSH_EXT_CACHE;
					i++;
				} else {
					dlog(LOG_ERR, "unknown parameter `%s' "
					     "for option `-f'", argv[i + 1]);
					exit(EXIT_FAILURE);
				}
			} else {
				/* default to general flushing */
				action = ALL_FLUSH_CACHE;
			}
			break;
		case 'R':
			set_operation_mode(&type, REQUEST, argv);
			i = set_action_by_table(i, argc, argv,
						CT_RESYNC_MASTER,
						EXP_RESYNC_MASTER,
						ALL_RESYNC_MASTER, &action);
			break;
		case 'B':
			set_operation_mode(&type, REQUEST, argv);
			action = SEND_BULK;
			break;
		case 't':
			set_operation_mode(&type, REQUEST, argv);
			action = RESET_TIMERS;
			break;
		case 'k':
			set_operation_mode(&type, REQUEST, argv);
			action = KILL;
			break;
		case 's':
			set_operation_mode(&type, REQUEST, argv);
			/* we've got a parameter */
			if (i+1 < argc && argv[i+1][0] != '-') {
				if (strncmp(argv[i+1], "network",
					    strlen(argv[i+1])) == 0) {
					action = STATS_NETWORK;
					i++;
				} else if (strncmp(argv[i+1], "cache",
						 strlen(argv[i+1])) == 0) {
					action = STATS_CACHE;
					i++;
				} else if (strncmp(argv[i+1], "runtime",
						 strlen(argv[i+1])) == 0) {
					action = STATS_RUNTIME;
					i++;
				} else if (strncmp(argv[i+1], "multicast",
						 strlen(argv[i+1])) == 0) {
					dlog(LOG_WARNING, "use `link' "
					     "instead of `multicast' as "
					     "parameter.");
					action = STATS_LINK;
					i++;
				} else if (strncmp(argv[i+1], "link",
						 strlen(argv[i+1])) == 0) {
					action = STATS_LINK;
					i++;
				} else if (strncmp(argv[i+1], "rsqueue",
						strlen(argv[i+1])) == 0) {
					action = STATS_RSQUEUE;
					i++;
				} else if (strncmp(argv[i+1], "process",
						 strlen(argv[i+1])) == 0) {
					action = STATS_PROCESS;
					i++;
				} else if (strncmp(argv[i+1], "queue",
						strlen(argv[i+1])) == 0) {
					action = STATS_QUEUE;
					i++;
				} else if (strncmp(argv[i+1], "ct",
						strlen(argv[i+1])) == 0) {
					action = STATS;
					i++;
				} else if (strncmp(argv[i+1], "expect",
						strlen(argv[i+1])) == 0) {
					action = EXP_STATS;
					i++;
				} else {
					dlog(LOG_ERR, "unknown parameter `%s' "
					     "for option `-s'", argv[i + 1]);
					exit(EXIT_FAILURE);
				}
			} else {
				/* default to general statistics */
				action = STATS;
			}
			break;
		case 'n':
			set_operation_mode(&type, REQUEST, argv);
			action = REQUEST_DUMP;
			break;
		case 'x':
			if (action == CT_DUMP_INTERNAL)
				action = CT_DUMP_INT_XML;
			else if (action == CT_DUMP_EXTERNAL)
				action = CT_DUMP_EXT_XML;
			else if (action == EXP_DUMP_INTERNAL)
				action = EXP_DUMP_INT_XML;
			else if (action == EXP_DUMP_EXTERNAL)
				action = EXP_DUMP_EXT_XML;
			else {
				show_usage(argv[0]);
				dlog(LOG_ERR,  "Invalid parameters");
				exit(EXIT_FAILURE);

			}
			break;
		case 'v':
			show_version();
			exit(EXIT_SUCCESS);
		case 'h':
			show_usage(argv[0]);
			exit(EXIT_SUCCESS);
		default:
			show_usage(argv[0]);
			dlog(LOG_ERR, "Unknown option: %s", argv[i]);
			return 0;
			break;
		}
	}

	if (!config_file[0])
		strcpy(config_file, DEFAULT_CONFIGFILE);

	umask(0177);

	if ((ret = init_config(config_file)) == -1) {
		dlog(LOG_ERR, "can't open config file `%s'", config_file);
		exit(EXIT_FAILURE);
	}

	/*
	 * Evaluate configuration
	 */
	if (evaluate() == -1) {
		dlog(LOG_ERR, "conntrackd cannot start, please review your "
		     "configuration");
		exit(EXIT_FAILURE);
	}

	if (type == REQUEST) {
		if (do_local_request(action, &conf.local, local_step) == -1) {
			dlog(LOG_ERR, "can't connect: is conntrackd "
			     "running? appropriate permissions?");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}

	/*
	 * Setting up logging
	 */
	if (init_log() == -1)
		exit(EXIT_FAILURE);

	/*
	 * lock file
	 */
	ret = open(CONFIG(lockfile), O_CREAT | O_EXCL | O_TRUNC, 0600);
	if (ret == -1) {
		dlog(LOG_ERR, "lockfile `%s' exists, perhaps conntrackd"
		     " already running?", CONFIG(lockfile));
		exit(EXIT_FAILURE);
	}
	close(ret);

	/*
	 * initialization process
	 */

	if (init() == -1) {
		dlog(LOG_ERR, "conntrackd cannot start, please review your "
		     "configuration");
		close_log();
		unlink(CONFIG(lockfile));
		exit(EXIT_FAILURE);
	}

	do_chdir("/");
	close(STDIN_FILENO);

	sd_ct_watchdog_init();

	/* Daemonize conntrackd */
	if (type == DAEMON) {
		pid_t pid;

		if ((pid = fork()) == -1) {
			dlog(LOG_ERR, "fork has failed: %s", strerror(errno));
			close_log();
			unlink(CONFIG(lockfile));
			exit(EXIT_FAILURE);
		} else if (pid) {
			sd_ct_mainpid(pid);
			exit(EXIT_SUCCESS);
		}

		setsid();

		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		dlog(LOG_NOTICE, "-- starting in daemon mode --");
	} else
		dlog(LOG_NOTICE, "-- starting in console mode --");

	sd_ct_init();
	resync_at_startup();

	/*
	 * run main process
	 */
	select_main_loop();
	return 0;
}
