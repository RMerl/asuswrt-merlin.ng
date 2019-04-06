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
 *
 *
 * Copyright 2019 Eric Sauvageau.
 *
 */

#include <string.h>
#include <rc.h>
#include <bcmnvram.h>
#include <shutils.h>

#define NTPD_PIDFILE "/var/run/ntpd.pid"

int start_ntpd(void)
{
	char *ntpd_argv[] = { "/usr/sbin/ntpd",
		"-I", nvram_safe_get("lan_ifname"),
		"-l", "-t",
		"-p", "pool.ntp.org",
		NULL, NULL,		/* -p second_server */
		NULL };
	int ret, index = 7;
	pid_t pid;

	if (!nvram_get_int("ntpd_enable"))
		return 0;

	//if (!nvram_get_int("ntp_ready"))
	//	return;

	if (getpid() != 1) {
		notify_rc("start_ntpd");
		return 0;
	}

	if (!nvram_match("ntp_server0", ""))
		ntpd_argv[index - 1] = nvram_safe_get("ntp_server0");
	if (!nvram_match("ntp_server1", "")) {
		ntpd_argv[index++] = "-p";
		ntpd_argv[index++] = nvram_safe_get("ntp_server1");
	}

	ret = _eval(ntpd_argv, NULL, 0, &pid);
	if (ret == 0)
		logmessage("ntpd", "Started ntpd");

	return ret;
}

void stop_ntpd(void)
{
	if (getpid() != 1) {
		notify_rc("stop_ntpd");
		return;
	}

	if (f_exists(NTPD_PIDFILE)) {
		kill_pidfile_tk(NTPD_PIDFILE);
		logmessage("ntpd", "Stopped ntpd");
	}
}
