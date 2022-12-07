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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>

static void wps_pbc(int sig)
{
	int unit = nvram_get_int("wps_band_x");
#ifdef CONFIG_BCMWL5
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	if (!nvram_match(strcat_r(prefix, "mode", tmp), "ap")
#ifdef RTCONFIG_AMAS
		&& !(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1"))
#endif
	) {
		nvram_set_int("wps_enr_hw", 1);
		notify_rc("start_wps_enr");
	}
	else
#endif
	{
#ifdef RTCONFIG_QCA_PLC2
//		do_plc_pushbutton(6);	//star PLC join procedure
		killall("detect_plc", SIGUSR1);
		if(nvram_match("wps_enable", "1"))
#endif
		//start_wps_pbc(0);
		start_wps_pbc(unit);
	}
}

static void wpsaide_exit(int sig)
{
	remove("/var/run/wpsaide.pid");
	exit(0);
}

int
wpsaide_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;

	/* write pid */
	if ((fp=fopen("/var/run/wpsaide.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGTSTP);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);
	signal(SIGTERM, wpsaide_exit);
	signal(SIGTSTP, wps_pbc);

	/* listen for replies */
	while (1)
	{
		pause();
	}

	return 0;
}
