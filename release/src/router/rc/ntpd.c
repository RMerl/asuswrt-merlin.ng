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

void start_ntpd(void) {
	FILE *fp;
	char server[32], server2[32];
	int parm = 5;
	pid_t pid;

	char *argv[] = {"/usr/sbin/ntpd",
			"-I",
			nvram_safe_get("lan_ifname"),
			"-l",
			"-p",
			NULL,
			NULL,	// second -p
			NULL,	// second server
			NULL};

	if (!nvram_get_int("ntpd_enable")) return;

	//if (!nvram_get_int("ntp_ready")) return;

	strlcpy(server, nvram_safe_get("ntp_server0"), sizeof(server));
	strlcpy(server2, nvram_safe_get("ntp_server1"), sizeof(server2));

	if (!*server)
		strcpy(server, "pool.ntp.org");

	argv[parm++] = server;

	if (*server2) {
		argv[parm++] = "-p";
		argv[parm++] = server2;
	}

	_eval(argv, NULL, 0, &pid);

	if (pid)
		logmessage("ntpd", "Started ntpd");
}


void stop_ntpd(void) {
	if (f_exists(NTPD_PIDFILE)) {
		kill_pidfile_tk(NTPD_PIDFILE);
		logmessage("ntpd", "Stopped ntpd");
	}
}

