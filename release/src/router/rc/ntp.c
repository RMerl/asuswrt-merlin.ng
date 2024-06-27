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
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
 
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <stdarg.h>

#define SECONDS_TO_WAIT 3
#define NTP_RETRY_INTERVAL 30

static char server[256];
static int sig_cur = -1;
static int server_idx = 0;

#if 1 // try simultaneously
#define DEFAULT_NTP_SERVER "pool.ntp.org"
static char* server_list[] = {
	DEFAULT_NTP_SERVER,
	"0.pool.ntp.org",
	"1.pool.ntp.org",
	"2.pool.ntp.org",
	"3.pool.ntp.org",
	"time.google.com",
	"time.nist.gov",
	NULL
};
#endif

static void ntp_service()
{
	static int first_sync = 1;

	if (first_sync) {
		first_sync = 0;

		setup_timezone();

		nvram_set("reload_svc_radio", "1");
		nvram_set("svc_ready", "1");
#ifndef RTCONFIG_QCA
		timecheck();
#endif

#ifdef RTCONFIG_DNSPRIVACY
		if (nvram_get_int("dnspriv_enable"))
			notify_rc("restart_stubby");
#endif
#ifdef RTCONFIG_DNSSEC
		if (nvram_get_int("dnssec_enable"))
			kill_pidfile_s("/var/run/dnsmasq.pid", SIGINT);
#endif
#ifdef RTCONFIG_DISK_MONITOR
		notify_rc("restart_diskmon");
#endif
#ifdef RTCONFIG_UUPLUGIN
		exec_uu();
#endif
	}
}

static void set_alarm()
{
	struct tm local;
	time_t now;
	int diff_sec;
	unsigned int sec;

#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2))
	if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
		ipv6_enabled() &&
                nvram_match(ipv6_nvname("ipv6_only"), "1") &&
		!nvram_get_int("ntp_ready") && f_exists("/tmp/ntpdated")) {
		nvram_set_int("ntp_ready", 1);
		ntp_service();
#ifdef RTCONFIG_CFGSYNC
		if (pidof("cfg_server") >= 0)
			kill_pidfile_s("/var/run/cfg_server.pid", SIGUSR1);
		if (pidof("cfg_client") >= 0)
			kill_pidfile_s("/var/run/cfg_client.pid", SIGUSR1);
#endif
	}
#endif
	if (nvram_get_int("ntp_ready"))
	{
		/* ntp sync every hour when time_zone set as "DST" */
		if (strstr(nvram_safe_get("time_zone_x"), "DST")) {
			time(&now);
			localtime_r(&now, &local);
//			dbg("%s: %d-%d-%d, %d:%d:%d dst:%d\n", __FUNCTION__, local.tm_year+1900, local.tm_mon+1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec, local.tm_isdst);
			/* every hour */
			if ((local.tm_min != 0) || (local.tm_sec != 0)) {
				/* compensate for the alarm(SECONDS_TO_WAIT) */
				diff_sec = (3600 - SECONDS_TO_WAIT) - (local.tm_min * 60 + local.tm_sec);
				if (diff_sec == 0)
					diff_sec = 3600;
				else if (diff_sec < 0)
					diff_sec = 3600 - diff_sec;
				else if (diff_sec <= SECONDS_TO_WAIT)
					diff_sec += 3600;
//				dbg("diff_sec: %d \n", diff_sec);
				sec = diff_sec;
			}
			else
				sec = 3600 - SECONDS_TO_WAIT;
		}
		else	/* every 12 hours */
			sec = 12 * 3600 - SECONDS_TO_WAIT;
	}
	else
		sec = NTP_RETRY_INTERVAL - SECONDS_TO_WAIT;

	//cprintf("## %s 4: sec(%u)\n", __func__, sec);
	alarm(sec);
}

static void catch_sig(int sig)
{
	sig_cur = sig;

	if (sig == SIGTSTP)
	{
		ntp_service();
	}
	else if (sig == SIGTERM)
	{
		remove("/var/run/ntp.pid");
		exit(0);
	}
	else if (sig == SIGCHLD)
	{
		chld_reap(sig);
	}
}

int ntp_main(int argc, char *argv[])
{
	FILE *fp;
	pid_t pid;
	char *args[] = {"ntpclient", "-h", server, "-i", "3", "-l", "-s", NULL};
#if defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2)
	pid_t pid__ntpdate;
	char *args_ntpdate[] = { "ntpdate", "2.pool.ntp.org", NULL };
#endif
	int ret;

	if (nvram_get_int("no_ntp"))
		return 0;

	strlcpy(server, nvram_safe_get("ntp_server0"), sizeof(server));

	fp = fopen("/var/run/ntp.pid", "w");
	if (fp == NULL)
		exit(0);
	fprintf(fp, "%d", getpid());
	fclose(fp);

	dbg("starting ntp...\n");

	signal(SIGTSTP, catch_sig);
	signal(SIGALRM, catch_sig);
	signal(SIGTERM, catch_sig);
//	signal(SIGCHLD, chld_reap);
	signal(SIGCHLD, catch_sig);

#if defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2)
	unlink("/tmp/ntpdated");
#endif
	nvram_set("ntp_ready", "0");
#ifdef RTCONFIG_QTN
	nvram_set("qtn_ntp_ready", "0");
#endif
	nvram_set("svc_ready", "0");

	while (1)
	{
		if (sig_cur == SIGTSTP)
			;
		else if (is_router_mode() &&
			(nvram_get_int("link_internet") != 2))
		{
			alarm(SECONDS_TO_WAIT);
		}
		else if ((repeater_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
				|| psr_mode() || mediabridge_mode()
#elif defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA)
				|| mediabridge_mode()
#endif
#ifdef RTCONFIG_DPSTA
				|| (dpsta_mode() && nvram_get_int("re_mode") == 0)
#endif
				|| (rp_mode() && nvram_get_int("re_mode") == 0)
			 ) && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED)
		{
			alarm(SECONDS_TO_WAIT);
		}
		else if (sig_cur == SIGCHLD && nvram_get_int("ntp_ready") != 0 )
		{ //handle the delayed ntpclient process
			set_alarm();
		}
		else
		{
			stop_ntpc();

			nvram_set("ntp_server_tried", server);
			if (nvram_match("ntp_ready", "0") || nvram_match("ntp_debug", "1") ||
				!strstr(nvram_safe_get("time_zone_x"), "DST")) {
				logmessage("ntp", "start NTP update");
			}

		if (is_router_mode()) {	// try simultaneously
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2))
			if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
				ipv6_enabled() &&
				nvram_match(ipv6_nvname("ipv6_only"), "1")) {
				ret = doSystem("nslookup %s &> /dev/null", args_ntpdate[1]);
				if (ret == 0)
				_eval(args_ntpdate, NULL, 0, &pid__ntpdate);
			} else
#endif
			if(strcmp(server, DEFAULT_NTP_SERVER)) //customer setting
			{
				ret = doSystem("nslookup %s &> /dev/null", server);
				if (ret == 0)
				_eval(args, NULL, 0, &pid);
				strlcpy(server, DEFAULT_NTP_SERVER, sizeof(server));
			}
			else
			{
				server_idx = 0;
				while(server_list[server_idx])
				{
					strlcpy(server, server_list[server_idx], sizeof(server));
					ret = doSystem("nslookup %s &> /dev/null", server_list[server_idx]);
					if (ret == 0)
					_eval(args, NULL, 0, &pid);
					server_idx++;
				}

				strlcpy(server, nvram_safe_get("ntp_server0"), sizeof(server));
			}
		} else {
			ret = doSystem("nslookup %s &> /dev/null", server);
			if (ret == 0)
			_eval(args, NULL, 0, &pid);

			if (strlen(nvram_safe_get("ntp_server0")))
				strlcpy(server, nvram_safe_get("ntp_server0"), sizeof (server));
			else if (strlen(nvram_safe_get("ntp_server1")))
				strlcpy(server, nvram_safe_get("ntp_server1"), sizeof(server));
			else
				strlcpy(server, "", sizeof(server));
			args[2] = server;
		}
			sleep(SECONDS_TO_WAIT);
			set_alarm();
		}

		pause();
	}
}
