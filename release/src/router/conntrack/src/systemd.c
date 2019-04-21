/*
 * (C) 2015 by Arturo Borrero Gonzalez <arturo@debian.org>
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

#include "systemd.h"
#include "conntrackd.h"
#include "alarm.h"
#include "log.h"
#include <systemd/sd-daemon.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

static struct alarm_block	sd_watchdog;
static uint64_t			sd_watchdog_interval;

static void sd_ct_watchdog_alarm(struct alarm_block *a, void *data)
{
	sd_notify(0, "WATCHDOG=1");
	add_alarm(&sd_watchdog, 0, sd_watchdog_interval);
}

void sd_ct_watchdog_init(void)
{
	int ret;

	if (CONFIG(systemd) == 0)
		return;

	ret = sd_watchdog_enabled(0, &sd_watchdog_interval);
	if (ret < 0) {
		dlog(LOG_WARNING, "failed to get watchdog details from "
		     "systemd: %s", strerror(-ret));
		return;
	} else if (ret == 0) {
		/* no watchdog required */
		return;
	}

	/* from man page, recommended interval is half of set by admin */
	sd_watchdog_interval = sd_watchdog_interval / 2;

	init_alarm(&sd_watchdog, &sd_watchdog_interval, sd_ct_watchdog_alarm);
	add_alarm(&sd_watchdog, 0, sd_watchdog_interval);
}

void sd_ct_init(void)
{
	if (CONFIG(systemd) == 0)
		return;

	sd_notify(0, "READY=1");
}

void sd_ct_mainpid(pid_t pid)
{
	if (CONFIG(systemd) == 0)
		return;

	sd_notifyf(0, "MAINPID=%d", pid);
}

void sd_ct_stop(void)
{
	if (CONFIG(systemd) == 0)
		return;

	sd_notify(0, "STOPPING=1");
}
