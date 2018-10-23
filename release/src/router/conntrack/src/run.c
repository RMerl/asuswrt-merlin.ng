/*
 * (C) 2006-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
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
 *
 * Part of this code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "conntrackd.h"
#include "netlink.h"
#include "filter.h"
#include "log.h"
#include "alarm.h"
#include "fds.h"
#include "traffic_stats.h"
#include "process.h"
#include "origin.h"
#include "date.h"
#include "internal.h"
#include "systemd.h"

#include <sched.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

void killer(int signo)
{
	/* Signals are re-entrant, disable signal handling to avoid problems
	 * in case we receive SIGINT and SIGTERM in a row. This function is
	 * also called via -k from the unix socket context, we already disabled
	 * signals in that path, so don't do it.
	 */
	if (signo)
		sigprocmask(SIG_BLOCK, &STATE(block), NULL);

	local_server_destroy(&STATE(local));

	if (CONFIG(flags) & (CTD_SYNC_MODE | CTD_STATS_MODE))
		ctnl_kill();

#ifdef BUILD_CTHELPER
	if (CONFIG(flags) & CTD_HELPER)
		cthelper_kill();
#endif
	destroy_fds(STATE(fds));
	unlink(CONFIG(lockfile));
	dlog(LOG_NOTICE, "---- shutdown received ----");
	close_log();

	sd_ct_stop();
	exit(0);
}

static void child(int foo)
{
	int status, ret;

	while ((ret = waitpid(0, &status, WNOHANG)) != 0) {
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			if (errno == ECHILD)
				break;
			STATE(stats).wait_failed++;
			break;
		}
		/* delete process from list and run the callback */
		fork_process_delete(ret);

		if (!WIFSIGNALED(status))
			continue;

		switch(WTERMSIG(status)) {
		case SIGSEGV:
			dlog(LOG_ERR, "child process (pid=%u) has aborted, "
				      "received signal SIGSEGV (crashed)", ret);
			STATE(stats).child_process_failed++;
			STATE(stats).child_process_error_segfault++;
			break;
		case SIGINT:
		case SIGTERM:
		case SIGKILL:
			dlog(LOG_ERR, "child process (pid=%u) has aborted, "
				      "received termination signal (%u)",
				      ret, WTERMSIG(status));
			STATE(stats).child_process_failed++;
			STATE(stats).child_process_error_term++;
			break;
		default:
			dlog(LOG_NOTICE, "child process (pid=%u) "
					 "received signal (%u)", 
					 ret, WTERMSIG(status));
			STATE(stats).child_process_failed++;
			break;
		}
	}
}

static void uptime(char *buf, size_t bufsiz)
{
	time_t tmp;
	int updays, upminutes, uphours;
	size_t size = 0;

	time(&tmp);
	tmp = tmp - STATE(stats).daemon_start_time;
	updays = (int) tmp / (60*60*24);
	if (updays) {
		size = snprintf(buf, bufsiz, "%d day%s ",
				updays, (updays != 1) ? "s" : "");
	}
	upminutes = (int) tmp / 60;
	uphours = (upminutes / 60) % 24;
	upminutes %= 60;
	if(uphours) {
		snprintf(buf + size, bufsiz, "%d h %d min", uphours, upminutes);
	} else {
		snprintf(buf + size, bufsiz, "%d min", upminutes);
	}
}

static void dump_stats_runtime(int fd)
{
	char buf[1024], uptime_string[512];
	int size;

	uptime(uptime_string, sizeof(uptime_string));
	size = snprintf(buf, sizeof(buf),
			"daemon uptime: %s\n\n"
			"netlink stats:\n"
			"\tevents received:\t%20llu\n"
			"\tevents filtered:\t%20llu\n"
			"\tevents unknown type:\t\t%12u\n"
			"\tcatch event failed:\t\t%12u\n"
			"\tdump unknown type:\t\t%12u\n"
			"\tnetlink overrun:\t\t%12u\n"
			"\tflush kernel table:\t\t%12u\n"
			"\tresync with kernel table:\t%12u\n"
			"\tcurrent buffer size (in bytes):\t%12u\n\n"
			"runtime stats:\n"
			"\tchild process failed:\t\t%12u\n"
			"\t\tchild process segfault:\t%12u\n"
			"\t\tchild process termsig:\t%12u\n"
			"\tselect failed:\t\t\t%12u\n"
			"\twait failed:\t\t\t%12u\n"
			"\tlocal read failed:\t\t%12u\n"
			"\tlocal unknown request:\t\t%12u\n\n",
			uptime_string,
			(unsigned long long)STATE(stats).nl_events_received,
			(unsigned long long)STATE(stats).nl_events_filtered,
			STATE(stats).nl_events_unknown_type,
			STATE(stats).nl_catch_event_failed,
			STATE(stats).nl_dump_unknown_type,
			STATE(stats).nl_overrun,
			STATE(stats).nl_kernel_table_flush,
			STATE(stats).nl_kernel_table_resync,
			CONFIG(netlink_buffer_size),
			STATE(stats).child_process_failed,
			STATE(stats).child_process_error_segfault,
			STATE(stats).child_process_error_term,
			STATE(stats).select_failed,
			STATE(stats).wait_failed,
			STATE(stats).local_read_failed,
			STATE(stats).local_unknown_request);

	send(fd, buf, size, 0);
}

static int local_handler(int fd, void *data)
{
	int ret = LOCAL_RET_OK;
	int type;

	if (read(fd, &type, sizeof(type)) <= 0) {
		STATE(stats).local_read_failed++;
		return LOCAL_RET_OK;
	}
	switch(type) {
        case KILL:
                killer(0);
                break;
	case STATS_RUNTIME:
		dump_stats_runtime(fd);
		break;
	case STATS_PROCESS:
		fork_process_dump(fd);
		break;
	}

	if (CONFIG(flags) & (CTD_SYNC_MODE | CTD_STATS_MODE))
		return ctnl_local(fd, type, data);

#ifdef BUILD_CTHELPER
	if (CONFIG(flags) & CTD_HELPER)
		return cthelper_local(fd, type, data);
#endif
	return ret;
}

/* order received via UNIX socket */
static void local_cb(void *data)
{
	do_local_server_step(&STATE(local), NULL, local_handler);
}

int evaluate(void)
{
	if (CONFIG(sync).external_cache_disable &&
	    CONFIG(commit_timeout)) {
		dlog(LOG_WARNING, "`CommitTimeout' can't be combined with "
		     "`DisableExternalCache', ignoring this option. "
		     "Fix your configuration file.");
		CONFIG(commit_timeout) = 0;
	}

	return 0;
}


static void set_scheduler(void)
{
	struct sched_param schedparam;
	int sched_type;

	if (CONFIG(sched).type == SCHED_OTHER) {
		/* default */
		schedparam.sched_priority = sched_get_priority_max(SCHED_RR);
		sched_type = SCHED_RR;
	} else {
		schedparam.sched_priority = CONFIG(sched).prio;
		sched_type = CONFIG(sched).type;
	}

	if (sched_setscheduler(0, sched_type, &schedparam) < 0)
		dlog(LOG_WARNING, "scheduler configuration failed: %s. "
		     "Likely a bug in conntrackd, please report it. "
		     "Continuing with system default scheduler.",
		     strerror(errno));
}

int
init(void)
{
	do_gettimeofday();

	set_scheduler();

	STATE(fds) = create_fds();
	if (STATE(fds) == NULL) {
		dlog(LOG_ERR, "can't create file descriptor pool");
		return -1;
	}

	/* local UNIX socket */
	if (local_server_create(&STATE(local), &CONFIG(local)) == -1) {
		dlog(LOG_ERR, "can't open unix socket!");
		return -1;
	}
	register_fd(STATE(local).fd, local_cb, NULL, STATE(fds));

	/* Signals handling */
	sigemptyset(&STATE(block));
	sigaddset(&STATE(block), SIGTERM);
	sigaddset(&STATE(block), SIGINT);
	sigaddset(&STATE(block), SIGCHLD);

	if (signal(SIGINT, killer) == SIG_ERR)
		return -1;

	if (signal(SIGTERM, killer) == SIG_ERR)
		return -1;

	/* ignore connection reset by peer */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return -1;

	if (signal(SIGCHLD, child) == SIG_ERR)
		return -1;

	/* Initialization */
	if (CONFIG(flags) & (CTD_SYNC_MODE | CTD_STATS_MODE))
		if (ctnl_init() < 0)
			return -1;

#ifdef BUILD_CTHELPER
	if (CONFIG(flags) & CTD_HELPER) {
		if (cthelper_init() < 0)
			return -1;
	}
#endif
	time(&STATE(stats).daemon_start_time);

	dlog(LOG_NOTICE, "initialization completed");

	return 0;
}
