/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>

static void check_watchdog_exit(int sig)
{
	exit(0);
}

extern int g_upgrade;
void check_watchdog()
{
	int ret;
	struct stat sb;
	time_t now;
	static unsigned long ts_check = 0;

	if (g_reboot || g_upgrade)
		return;

	if (ate_factory_mode())
		return;

	ret = stat("/tmp/watchdog_heartbeat", &sb);
	time(&now);

	if (ts_check && ((now - ts_check) < 60))
		return;
	else
		ts_check = now;

	if ((ret != -1) && ((sb.st_mode & S_IFMT) == S_IFREG)) {
		if ((now - sb.st_ctime) > 300) {
			logmessage("check_watchdog", "[%s] restart watchdog for no heartbeat\n", __FUNCTION__);
			notify_rc("restart_watchdog");
		}
	}
}

static struct itimerval itv;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

int
check_watchdog_main(int argc, char *argv[])
{
	sigset_t sigs_to_catch;

	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);
	signal(SIGTERM, check_watchdog_exit);
	signal(SIGALRM, check_watchdog);

	alarmtimer(30, 0);

	while (1)
	{
		pause();
	}

	return 0;
}
