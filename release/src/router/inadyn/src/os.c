/*
 * Copyright (C) 2003-2004  Narcis Ilisei
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2017  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <libgen.h>		/* dirname() */
#include <sys/stat.h>
#include "log.h"
#include "cache.h"

static void *param = NULL;


/**
 * Execute shell script on successful update.
 * @cmd:  Full path to script or command to run
 * @ip:   IP address to set as %INADYN_IP env. variable
 * @name: String to set as %INADYN_HOSTNAME env. variable
 *
 * If inadyn has been started with the --iface=IFNAME command line
 * option the IFNAME is sent to the script as %INADYN_IFACE.
 *
 * Returns:
 * Posix %OK(0), or %RC_OS_FORK_FAILURE on vfork() failure
 */
int os_shell_execute(char *cmd, char *ip, char *name)
{
	int rc = 0;
	int child;

	child = vfork();
	switch (child) {
	case 0:
		setenv("INADYN_IP", ip, 1);
		setenv("INADYN_HOSTNAME", name, 1);
		if (iface)
			setenv("INADYN_IFACE", iface, 1);
		execl("/bin/sh", "sh", "-c", cmd, (char *)0);
		exit(1);
		break;

	case -1:
		rc = RC_OS_FORK_FAILURE;
	default:
		break;
	}

	return rc;
}

/**
 * unix_signal_handler - Signal handler
 * @signo: Signal number
 *
 * Handler for registered/known signals. Most others will terminate the
 * daemon.
 *
 * NOTE:
 * Since printf() is one of the possible back-ends of logit(), and
 * printf() is not one of the safe syscalls to be used, according to
 * POSIX signal(7). The calls are commented, since they are most likely
 * also only needed for debugging.
 */
static void unix_signal_handler(int signo)
{
	ddns_t *ctx = (ddns_t *)param;

	if (ctx == NULL)
		return;

	switch (signo) {
	case SIGHUP:
		ctx->cmd = CMD_RESTART;
		break;

	case SIGINT:
	case SIGTERM:
		ctx->cmd = CMD_STOP;
		break;

	case SIGUSR1:
		ctx->cmd = CMD_FORCED_UPDATE;
		break;

	case SIGUSR2:
		ctx->cmd = CMD_CHECK_NOW;
		break;

	default:
		break;
	}
}

/**
 * Install signal handler for signals HUP, INT, TERM and USR1
 *
 * Also block exactly the handled signals, only for the duration
 * of the handler.  All other signals are left alone.
 */
int os_install_signal_handler(void *ctx)
{
	int rc = 0;
	static int installed = 0;
	struct sigaction sa;

	if (!installed) {
		sa.sa_flags   = 0;
		sa.sa_handler = unix_signal_handler;

		rc = sigemptyset(&sa.sa_mask) ||
			sigaddset(&sa.sa_mask, SIGHUP)  ||
			sigaddset(&sa.sa_mask, SIGINT)  ||
			sigaddset(&sa.sa_mask, SIGTERM) ||
			sigaddset(&sa.sa_mask, SIGUSR1) ||
			sigaddset(&sa.sa_mask, SIGUSR2) ||
			sigaction(SIGHUP, &sa, NULL)    ||
			sigaction(SIGINT, &sa, NULL)    ||
			sigaction(SIGUSR1, &sa, NULL)   ||
			sigaction(SIGUSR2, &sa, NULL)   ||
			sigaction(SIGTERM, &sa, NULL);

		installed = 1;
	}

	if (rc) {
		logit(LOG_WARNING, "Failed installing signal handler: %s", strerror(errno));
		return RC_OS_INSTALL_SIGHANDLER_FAILED;
	}

	param = ctx;
	return 0;
}

/*
 * Check file system permissions
 *
 * Create pid and cache file repository, make sure we can write to it.  If
 * we are restarted we cannot otherwise make sure we've not already updated
 * the IP -- and the user will be locked-out of their DDNS server provider
 * for excessive updates.
 */
int os_check_perms(void)
{
	/* Create files with permissions 0644 */
	umask(S_IWGRP | S_IWOTH);

	if ((mkpath(cache_dir, 0755) && errno != EEXIST) || access(cache_dir, W_OK)) {
		logit(LOG_ERR, "No write permission to %s, aborting.", cache_dir);
		logit(LOG_ERR, "Cannot guarantee DDNS server won't lock you out for excessive updates.");
		return RC_FILE_IO_ACCESS_ERROR;
	}

	if (chown(cache_dir, uid, gid)) {
		logit(LOG_ERR, "Not allowed to change owner of %s, aborting.", cache_dir);
		return RC_FILE_IO_ACCESS_ERROR;
	}

	if (pidfile_name && pidfile_name[0] == '/') {
		char *pidfile_dir;

		if (!access(pidfile_name, F_OK)) {
			logit(LOG_ERR, "PID file %s already exists, %s already running?", pidfile_name, prognm);
			return RC_PIDFILE_EXISTS_ALREADY;
		}

		pidfile_dir = dirname(strdupa(pidfile_name));
		if (access(pidfile_dir, F_OK)) {
			if (mkpath(pidfile_dir, 0755) && errno != EEXIST) {
				logit(LOG_ERR, "No write permission to %s, aborting.", pidfile_dir);
				return RC_FILE_IO_ACCESS_ERROR;
			}

			if (chown(pidfile_dir, uid, gid)) {
				logit(LOG_ERR, "Not allowed to change owner of %s, aborting.", pidfile_dir);
				return RC_FILE_IO_ACCESS_ERROR;
			}
		}
	}

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
