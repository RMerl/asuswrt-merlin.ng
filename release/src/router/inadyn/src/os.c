/*
 * Copyright (C) 2003-2004  Narcis Ilisei
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2020  Joachim Nilsson <troglobit@gmail.com>
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
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>		/* fopen() et al */
#include <stdlib.h>		/* atoi() */

#include "log.h"
#include "cache.h"

static void *param = NULL;


/**
 * Execute shell script on successful update.
 * @cmd:  Full path to script or command to run
 * @ip:   IP address to set as %INADYN_IP env. variable
 * @name: String to set as %INADYN_HOSTNAME env. variable
 * @event:Event name to set as %INADYN_EVENT env. variable
 * @error:Error code to set as %INADYN_ERROR env. variable
 *
 * If inadyn has been started with the --iface=IFNAME command line
 * option the IFNAME is sent to the script as %INADYN_IFACE.
 *
 * Returns:
 * Posix %OK(0), or %RC_OS_FORK_FAILURE on vfork() failure
 */
int os_shell_execute(char *cmd, char *ip, char *name, char *event, int error)
{
	int rc = 0;
	int child;
	char errbuf[11];

	child = vfork();
	switch (child) {
	case 0:
		snprintf(errbuf, sizeof(errbuf), "%d", error);
		setenv("INADYN_IP", ip, 1);
		setenv("INADYN_HOSTNAME", name, 1);
		setenv("INADYN_EVENT", event, 1);
		setenv("INADYN_ERROR", errbuf, 1);
		setenv("INADYN_ERROR_MESSAGE", error_str(error), 1);
		if (iface)
			setenv("INADYN_IFACE", iface, 1);

		execl("/bin/sh", "sh", "-c", cmd, (char *)0);
		exit(1);
		break;

	case -1:
		rc = RC_OS_FORK_FAILURE;
		break;
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

/*
 * Set SIGCHLD to 'ignore', i.e., children are automatically reaped,
 * as of POSIX.1-2001.  This is fine since we are (currently) not
 * interested in their exit status.
 */
static int os_install_child_handler(void)
{
	static int installed = 0;
	int rc = 0;

	if (!installed) {
		struct sigaction sa;

		memset(&sa, 0, sizeof(sa));
#ifdef SA_RESTART
		sa.sa_flags |= SA_RESTART;
#endif
		sa.sa_handler = SIG_IGN;

		rc = (sigemptyset(&sa.sa_mask)        ||
		      sigaddset(&sa.sa_mask, SIGCHLD) ||
		      sigaction(SIGCHLD, &sa, NULL));

		installed = 1;
	}

	if (rc) {
		logit(LOG_WARNING, "Failed installing signal handler: %s", strerror(errno));
		return RC_OS_INSTALL_SIGHANDLER_FAILED;
	}

	return 0;
}

/**
 * Install signal handler for signals HUP, INT, TERM and USR1
 *
 * Also block exactly the handled signals, only for the duration
 * of the handler.  All other signals are left alone.
 */
int os_install_signal_handler(void *ctx)
{
	static int installed = 0;
	int rc = 0;

	if (!installed) {
		struct sigaction sa;

		memset(&sa, 0, sizeof(sa));
#ifdef SA_RESTART
		sa.sa_flags |= SA_RESTART;
#endif
		sa.sa_handler = unix_signal_handler;

		rc = (sigemptyset(&sa.sa_mask)        ||
		      sigaddset(&sa.sa_mask, SIGHUP)  ||
		      sigaddset(&sa.sa_mask, SIGINT)  ||
		      sigaddset(&sa.sa_mask, SIGTERM) ||
		      sigaddset(&sa.sa_mask, SIGUSR1) ||
		      sigaddset(&sa.sa_mask, SIGUSR2) ||
		      sigaction(SIGHUP, &sa, NULL)    ||
		      sigaction(SIGINT, &sa, NULL)    ||
		      sigaction(SIGUSR1, &sa, NULL)   ||
		      sigaction(SIGUSR2, &sa, NULL)   ||
		      sigaction(SIGTERM, &sa, NULL));

		installed = 1;
	}

	if (script_exec) 
		os_install_child_handler();

	if (rc) {
		logit(LOG_WARNING, "Failed installing signal handler: %s", strerror(errno));
		return RC_OS_INSTALL_SIGHANDLER_FAILED;
	}

	param = ctx;
	return 0;
}

static int pid_alive(pid_t pid) {
	return !(kill(pid, 0) && errno == ESRCH);
}

static pid_t getpid_from_pidfile(char *pidfn)
{
	FILE *fp;
	pid_t pid = 0;
	fp = fopen(pidfn, "r");
	if (fp) {
		char buf[20];

		if (fgets(buf, sizeof(buf), fp)) {
			pid = atoi(buf);
		}

		fclose(fp);
	}

	return pid;
}

/*
 * Check file system permissions
 *
 * Try to create PID file directory and cache file repository.  Check if
 * we are allowed to write to them.  This to ensure we can both signal
 * ACK to a SIGHUP and to ensure we do not cause op to have their DDNS
 * provider lock them out for excessive updates.
 */
int os_check_perms(void)
{
	/* Create files with permissions 0644 */
	umask(S_IWGRP | S_IWOTH);

	if ((mkpath(cache_dir, 0755) && errno != EEXIST) || access(cache_dir, W_OK)) {
		logit(LOG_WARNING, "No write permission to %s: %s", cache_dir, strerror(errno));
		logit(LOG_WARNING, "Cannot guarantee DDNS server won't lock you out for excessive updates.");
	}

	/* Handle --no-pidfile case as well, check for "" */
	if (pidfile_name && pidfile_name[0]) {
		char pidfn[strlen(RUNSTATEDIR) + strlen(pidfile_name) + 6];
		char *pidfile_dir;

		if (pidfile_name[0] != '/')
			snprintf(pidfn, sizeof(pidfn), "%s/%s.pid", RUNSTATEDIR, pidfile_name);
		else
			strlcpy(pidfn, pidfile_name, sizeof(pidfn));

		if (!access(pidfn, F_OK)) {
			pid_t pid = getpid_from_pidfile(pidfn);
			if (pid > 0 && pid != getpid() && pid_alive(pid)) {
				logit(LOG_ERR, "PID file %s already exists, %s already running?",
							pidfn, prognm);
				return RC_PIDFILE_EXISTS_ALREADY;
			}
		}

		pidfile_dir = dirname(strdupa(pidfn));
		if (access(pidfile_dir, F_OK)) {
			if (mkpath(pidfile_dir, 0755) && errno != EEXIST)
				logit(LOG_ERR, "No write permission to %s, aborting.", pidfile_dir);
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
