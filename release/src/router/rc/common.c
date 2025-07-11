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
 * $Id: common_ex.c,v 1.3 2007/03/29 06:02:23 shinjung Exp $
 */

#include "rc.h"

#include <stdlib.h>
#include <stdio.h>
#include <net/if_arp.h>
#include <time.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <stdarg.h>
#include <arpa/inet.h>	// oleg patch
#include <string.h>	// oleg patch
#include <bcmdevs.h>
#include <wlutils.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <shared.h>
#if !defined(__GLIBC__) && !defined(__UCLIBC__) /* musl */
#include <sys/syscall.h>
#endif

#ifdef RTCONFIG_RALINK
#include <ralink.h>
#endif

#ifdef RTCONFIG_QCA
#include <qca.h>
#endif

#include <mtd.h>
#include <limits.h>

void update_lan_status(int);

/* remove space in the end of string */
char *trim_r(char *str)
{
	int i;

	if (!str || !*str)
		return str;
	
	i = strlen(str) - 1;
	while ((i >= 0) && (str[i] == ' ' || str[i] == '\n' || str[i] == '\r'))
		str[i--] = 0;

	return str;
}

/* convert mac address format from XXXXXXXXXXXX to XX:XX:XX:XX:XX:XX */
char *conv_mac(char *mac, char *buf)
{
	int i, j;

	if (strlen(mac)==0) 
	{
		buf[0] = 0;
	}
	else
	{
		j=0;	
		for (i=0; i<12; i++)
		{		
			if (i!=0&&i%2==0) buf[j++] = ':';
			buf[j++] = mac[i];
		}
		buf[j] = 0;	// oleg patch
	}
	//buf[j] = 0;

	_dprintf("mac: %s\n", buf);

	return (buf);
}

// 2010.09 James. {
/* convert mac address format from XX:XX:XX:XX:XX:XX to XXXXXXXXXXXX */
char *conv_mac2(char *mac, char *buf)
{
	int i,j;

	if(strlen(mac) != 17)
		buf[0] = 0;
	else{
		for(i = 0, j = 0; i < 17; ++i){
			if(i%3 != 2){
				buf[j] = mac[i];
				++j;
			}

			buf[j] = 0;
		}
	}

	return(buf);
}

/* convert mac address format from XXXXXXXXXXXX to XX:XX:XX:XX:XX:XX */
char *mac_conv(char *mac_name, int idx, char *buf)
{
	char mac[32], name[32];
	int i, j;

	if(idx != -1)
		snprintf(name, sizeof(name), "%s%d", mac_name, idx);
	else
		snprintf(name, sizeof(name), "%s", mac_name);

	snprintf(mac, sizeof(mac), "%s", nvram_safe_get(name));

	if(strlen(mac) <= 0 || strlen(mac) != 12)
		buf[0] = 0;
	else{
		for(i = 0, j = 0; i < 12; ++i){
			if(i != 0 && i%2 == 0)
				buf[j++] = ':';

			buf[j++] = mac[i];
		}

		buf[j] = 0;	// oleg patch
	}

	_dprintf("mac: %s\n", buf);

	return (buf);
}

// 2010.09 James. {
/* convert mac address format from XX:XX:XX:XX:XX:XX to XXXXXXXXXXXX */
char *mac_conv2(char *mac_name, int idx, char *buf)
{
	char mac[32], name[32];
	int i, j;

	if(idx != -1)
		snprintf(name, sizeof(name), "%s%d", mac_name, idx);
	else
		snprintf(name, sizeof(name), "%s", mac_name);

	snprintf(mac, sizeof(mac), "%s", nvram_safe_get(name));

	if(strlen(mac) <= 0 || strlen(mac) != 17)
		buf[0] = 0;
	else{
		for(i = 0, j = 0; i < 17; ++i){
			if(i%3 != 2)
				buf[j++] = mac[i];

			buf[j] = 0;
		}
	}

	return(buf);
}
// 2010.09 James. }

//#if 0
void wan_netmask_check(void)
{
	unsigned int ip, gw, nm, lip, lnm;

	if (nvram_match("wan0_proto", "static") ||
	    //nvram_match("wan0_proto", "pptp"))
	    nvram_match("wan0_proto", "pptp") || nvram_match("wan0_proto", "l2tp"))	// oleg patch
	{
		ip = inet_addr(nvram_safe_get("wan_ipaddr"));
		gw = inet_addr(nvram_safe_get("wan_gateway"));
		nm = inet_addr(nvram_safe_get("wan_netmask"));

		lip = inet_addr(nvram_safe_get("lan_ipaddr"));
		lnm = inet_addr(nvram_safe_get("lan_netmask"));

		_dprintf("ip : %x %x %x\n", ip, gw, nm);

		if (ip==0x0 && (nvram_match("wan0_proto", "pptp") || nvram_match("wan0_proto", "l2tp")))	// oleg patch
			return;

		if (ip==0x0 || (ip&lnm)==(lip&lnm))
		{
			nvram_set("wan_ipaddr", "1.1.1.1");
			nvram_set("wan_netmask", "255.0.0.0");	
			nvram_set("wan0_ipaddr", nvram_safe_get("wan_ipaddr"));
			nvram_set("wan0_netmask", nvram_safe_get("wan_netmask"));
		}

		// check netmask here
		if (gw==0 || gw==0xffffffff || (ip&nm)==(gw&nm))
		{
			nvram_set("wan0_netmask", nvram_safe_get("wan_netmask"));
		}
		else
		{		
			for (nm=0xffffffff;nm!=0;nm=(nm>>8))
			{
				if ((ip&nm)==(gw&nm)) break;
			}

			_dprintf("nm: %x\n", nm);

			if (nm==0xffffffff) nvram_set("wan0_netmask", "255.255.255.255");
			else if (nm==0xffffff) nvram_set("wan0_netmask", "255.255.255.0");
			else if (nm==0xffff) nvram_set("wan0_netmask", "255.255.0.0");
			else if (nm==0xff) nvram_set("wan0_netmask", "255.0.0.0");
			else nvram_set("wan0_netmask", "0.0.0.0");
		}

		nvram_set("wanx_ipaddr", nvram_safe_get("wan0_ipaddr"));	// oleg patch, he suggests to mark the following 3 lines
		nvram_set("wanx_netmask", nvram_safe_get("wan0_netmask"));
		nvram_set("wanx_gateway", nvram_safe_get("wan0_gateway"));
	}
}

/*
 * wanmessage
 *
 */
void wanmessage(char *fmt, ...)
{
	va_list args;
	char buf[512];

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	nvram_set("wan_reason_t", buf);
	va_end(args);
}

int _pppstatus(const char *statusfile)
{
	char status[128];

	if (!statusfile || f_read_string(statusfile, status, sizeof(status)) <= 0)
		return WAN_STOPPED_REASON_NONE;

	if (strstr(status, "No response from ISP") != NULL)
		return WAN_STOPPED_REASON_PPP_NO_RESPONSE;
	else if (strstr(status, "Peer not responding") != NULL)
		return WAN_STOPPED_REASON_NONE; /* Connection appears to be disconnected */
	else if (strstr(status, "Failed to authenticate ourselves to peer") != NULL ||
		 strstr(status, "Authentication failed") != NULL)
		return WAN_STOPPED_REASON_PPP_AUTH_FAIL;
	else if (strstr(status, "Link inactive") != NULL)
		return WAN_STOPPED_REASON_PPP_LACK_ACTIVITY;

	return WAN_STOPPED_REASON_NONE;
}

int pppstatus(int unit)
{
	char statusfile[sizeof("/var/run/ppp-wanXXXXXXXXXX.status")];

	snprintf(statusfile, sizeof(statusfile), "/var/run/ppp-wan%d.status", unit);
	return _pppstatus(statusfile);
}

void usage_exit(const char *cmd, const char *help)
{
	fprintf(stderr, "Usage: %s %s\n", cmd, help);
	exit(1);
}

#ifndef ct_modprobe
#ifdef LINUX26
#define ct_modprobe(mod, args...) ({ \
		modprobe("nf_conntrack_"mod, ## args); \
		modprobe("nf_nat_"mod); \
})
#else
#define ct_modprobe(mod, args...) ({ \
		modprobe("ip_conntrack_"mod, ## args); \
		modprobe("ip_nat_"mod, ## args); \
})
#endif
#endif

#ifndef ct_modprobe_r
#ifdef LINUX26
#define ct_modprobe_r(mod) ({ \
	modprobe_r("nf_nat_"mod); \
	modprobe_r("nf_conntrack_"mod); \
})
#else
#define ct_modprobe_r(mod) ({ \
	modprobe_r("ip_nat_"mod); \
	modprobe_r("ip_conntrack_"mod); \
})
#endif
#endif

/* 
 * The various child job starting functions:
 * _eval()
 *	Start the child. If ppid param is NULL, wait until the child exits.
 *	Otherwise, store the child's pid in ppid and return immediately.
 * eval()
 *	Call _eval with a NULL ppid, to wait for the child to exit.
 * xstart()
 *	Call _eval with a garbage ppid (to not wait), then return.
 * runuserfile
 *	Execute each executable in a directory that has the specified extention.
 *	Call _eval with a ppid (to not wait), then check every second for the child's pid.
 *	After wtime seconds or when the child has exited, return.
 *	If any such filename has an '&' character in it, then do *not* wait at
 *	all for the child to exit, regardless of the wtime.
 */

int _xstart(const char *cmd, ...)
{
	va_list ap;
	char *argv[16];
	int argc;
	int pid;

	argv[0] = (char *)cmd;
	argc = 1;
	va_start(ap, cmd);
	while ((argv[argc++] = va_arg(ap, char *)) != NULL) {
		//
	}
	va_end(ap);

	return _eval(argv, NULL, 0, &pid);
}

static int endswith(const char *str, char *cmp)
{
	int cmp_len, str_len, i;

	cmp_len = strlen(cmp);
	str_len = strlen(str);
	if (cmp_len > str_len)
		return 0;
	for (i = 0; i < cmp_len; i++) {
		if (str[(str_len - 1) - i] != cmp[(cmp_len - 1) - i])
			return 0;
	}
	return 1;
}

static void execute_with_maxwait(char *const argv[], int wtime)
{
	pid_t pid;

	if (_eval(argv, NULL, 0, &pid) != 0)
		pid = -1;
	else {
		while (wtime-- > 0) {
			waitpid(pid, NULL, WNOHANG);	/* Reap the zombie if it has terminated. */
			if (kill(pid, 0) != 0) break;
			sleep(1);
		}
		_dprintf("%s killdon: errno: %d pid %d\n", argv[0], errno, pid);
	}
}

/* This is a bit ugly. Why didn't they allow another parameter to filter???? */
static char *filter_extension;
static int endswith_filter(const struct dirent *entry)
{
	return endswith(entry->d_name, filter_extension);
}

/* If the filename has an '&' character in it, don't wait at all. */
void run_userfile(char *folder, char *extension, const char *arg1, int wtime)
{
	unsigned char buf[PATH_MAX + 1];
	char *argv[] = { (char *)buf, (char *)arg1, NULL };
	struct dirent **namelist;
	int i, n;

	/* Do them in sorted order. */
	filter_extension = extension;
	n = scandir(folder, &namelist, endswith_filter, alphasort);
	if (n >= 0) {
		for (i = 0; i < n; ++i) {
			sprintf((char *) buf, "%s/%s", folder, namelist[i]->d_name);
			execute_with_maxwait(argv,
				strchr(namelist[i]->d_name, '&') ? 0 : wtime);
			free(namelist[i]);
		}
		free(namelist);
	}
}

/* Run user-supplied script(s), with 1 argument.
 * Return when the script(s) have finished,
 * or after wtime seconds, even if they aren't finished.
 *
 * Extract NAME from nvram variable named as "script_NAME".
 *
 * The sole exception to the nvram item naming rule is sesx.
 * That one is "sesx_script" rather than "script_sesx", due
 * to historical accident.
 *
 * The other exception is time-scheduled commands.
 * These have names that start with "sch_".
 * No directories are searched for corresponding user scripts.
 *
 * Execute in this order:
 *	nvram item: nv (run as a /bin/sh script)
 *		(unless nv starts with a dot)
 *	All files with a suffix of ".NAME" in these directories:
 *	/etc/config/
 *	/jffs/etc/config/
 *	/opt/etc/config/
 *	/mmc/etc/config/
 *	/tmp/config/
 */
/*
At this time, the names/events are:
   (Unless otherwise noted, there are no parameters.  Otherwise, one parameter).
   sesx		SES/AOSS Button custom script.  Param: ??
   brau		"bridge/auto" button pushed.  Param: mode (bridge/auto/etc)
   fire		When firewall service has been started or re-started.
   shut		At system shutdown, just before wan/lan/usb/etc. are stopped.
   init		At system startup, just before wan/lan/usb/etc. are started.
		The root filesystem and /jffs are mounted, but not any USB devices.
   usbmount	After an auto-mounted USB drive is mounted.
   usbumount	Before an auto-mounted USB drive is unmounted.
   usbhotplug	When any USB device is attached or removed.
   wanup	After WAN has come up.
   autostop	When a USB partition gets un-mounted.  Param: the mount-point (directory).
		If unmounted from the GUI, the directory is still mounted and accessible.
		If the USB drive was unplugged, it is still mounted but not accessible.

User scripts -- no directories are searched.  One parameter.
   autorun	When a USB disk partition gets auto-mounted. Param: the mount-point (directory).
		But not if the partition was already mounted.
		Only the files in that directory will be run.
*/
void run_nvscript(const char *nv, const char *arg1, int wtime)
{
	FILE *f;
	char script[PATH_MAX];
	char s[PATH_MAX + 1];
	char *argv[] = { s, (char *)arg1, NULL };
	int check_dirs = 1;

	if (nv[0] == '.') {
		strcpy(s, nv);
	}
	else {
		snprintf(script, sizeof(script), "%s", nvram_safe_get(nv));

		if(strlen(script) > 0){
			snprintf(s, sizeof(s), "/tmp/%s.sh", nv);
			if ((f = fopen(s, "w")) != NULL) {
				fputs("#!/bin/sh\n", f);
				fputs(script, f);
				fputs("\n", f);
				fclose(f);
				chmod(s, 0700);
				chdir("/tmp");

				_dprintf("Running: '%s %s'\n", argv[0], argv[1]? argv[1]: "");
				execute_with_maxwait(argv, wtime);
				chdir("/");
			}
		}

		snprintf(s, sizeof(s), ".%s", nv);
		if (strncmp("sch_c", nv, 5) == 0) {
			check_dirs = 0;
		}
		else if (strncmp("sesx_", nv, 5) == 0) {
			s[5] = 0;
		}
		else if (strncmp("script_", nv, 7) == 0) {
			strcpy(&s[1], &nv[7]);
		}
	}

	if (nvram_match("userfiles_disable", "1")) {
		// backdoor to disable user scripts execution
		check_dirs = 0;
	}

	if ((check_dirs) && strcmp(s, ".") != 0) {
		_dprintf("checking for user scripts: '%s'\n", s);
		run_userfile("/etc/config", s, arg1, wtime);
		run_userfile("/jffs/etc/config", s, arg1, wtime);
		run_userfile("/opt/etc/config", s, arg1, wtime);
		run_userfile("/mmc/etc/config", s, arg1, wtime);
		run_userfile("/tmp/config", s, arg1, wtime);
	}
}

static void write_ct_timeout(const char *type, const char *name, unsigned int val)
{
	unsigned char buf[128];
	char v[16];

	sprintf((char *) buf, "/proc/sys/net/%s_%s_timeout%s%s",
		d_exists("/proc/sys/net/ipv4/netfilter") ? "ipv4/netfilter/ip_conntrack" : "netfilter/nf_conntrack",
		type, (name && name[0]) ? "_" : "", name ? name : "");
	sprintf(v, "%u", val);

	f_write_string((const char *) buf, v, 0, 0);
}

#ifndef write_tcp_timeout
#define write_tcp_timeout(name, val) write_ct_timeout("tcp", name, val)
#endif

#ifndef write_udp_timeout
#define write_udp_timeout(name, val) write_ct_timeout("udp", name, val)
#endif

static unsigned int read_ct_timeout(const char *type, const char *name)
{
	unsigned char buf[128];
	unsigned int val = 0;
	char v[16];

	sprintf((char *) buf, "/proc/sys/net/%s_%s_timeout%s%s",
		d_exists("/proc/sys/net/ipv4/netfilter") ? "ipv4/netfilter/ip_conntrack" : "netfilter/nf_conntrack",
		type, (name && name[0]) ? "_" : "", name ? name : "");
	if (f_read_string((const char *) buf, v, sizeof(v)) > 0)
		val = atoi(v);

	return val;
}

#ifndef read_tcp_timeout
#define read_tcp_timeout(name) read_ct_timeout("tcp", name)
#endif

#ifndef read_udp_timeout
#define read_udp_timeout(name) read_ct_timeout("udp", name)
#endif


void setup_ftp_conntrack(int port)
{
	char ports[32];

	if(port>0&&port!=21)
		sprintf(ports, "ports=21,%d", port);
	else sprintf(ports, "ports=21");

	if(!nvram_match("ftp_ports", ports))
	{
		ct_modprobe_r("ftp");
		ct_modprobe("ftp", ports);
		nvram_set("ftp_ports", ports);
	}
}

void setup_udp_timeout(int connflag)
{
	unsigned int v[10];
	char p[32];
	char buf[70];

	if (connflag
#ifdef RTCONFIG_WIRELESSREPEATER
			&& sw_mode()!=SW_MODE_REPEATER
#endif
	) {
		snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_udp_timeout"));
		if (strcmp(p,"0 0") && sscanf(p, "%u%u", &v[0], &v[1]) == 2) {
			write_udp_timeout(NULL, v[0]);
			write_udp_timeout("stream", v[1]);
		}
		else {
			v[0] = read_udp_timeout(NULL);
			v[1] = read_udp_timeout("stream");
			snprintf(buf, sizeof(buf), "%u %u", v[0], v[1]);
			nvram_set("ct_udp_timeout", buf);
		}
	}
	else {
		write_udp_timeout(NULL, 1);
		write_udp_timeout("stream", 6);
	}
}

int scan_icmp_unreplied_conntrack()
{
	FILE *fp = fopen( "/proc/net/nf_conntrack", "r" );
	char buff[1024], ipv[16], ipv_num[16], protocol[16], protocol_num[16];
	int found = 0;

	if (fp == NULL) {
		perror( "openning /proc/net/nf_conntrack" );
		return -1;
	}

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		sscanf(buff, "%15s %15s %15s %15s", ipv, ipv_num, protocol, protocol_num);

		if (memcmp(protocol, "icmp", 4) || !strstr(buff, "UNREPLIED")) continue;

//		dbG("\n%s %s %s %s\n", ipv, ipv_num, protocol, protocol_num);

		found++;
		break;
	}

	fclose(fp);

//	if (!found)
//		dbG("No matching conntrack found\n");

	return found;
}

void setup_ct_timeout(int connflag)
{
	unsigned int v[10];
	char p[32];
	char buf[70];
	int i;

	if (connflag
#ifdef RTCONFIG_WIRELESSREPEATER
			&& sw_mode()!=SW_MODE_REPEATER
#endif
	) {
		snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_timeout"));
		if (strcmp(p,"0 0") && sscanf(p, "%u%u", &v[0], &v[1]) == 2) {
//			write_ct_timeout("generic", NULL, v[0]);
			write_ct_timeout("icmp", NULL, v[1]);
		}
		else {
			v[0] = read_ct_timeout("generic", NULL);
			v[1] = read_ct_timeout("icmp", NULL);

			snprintf(buf, sizeof(buf), "%u %u", v[0], v[1]);
			nvram_set("ct_timeout", buf);
		}
	}
	else {
		for (i = 0; i < 3; i++)
		{
			if (scan_icmp_unreplied_conntrack() > 0)
			{
//				write_ct_timeout("generic", NULL, 0);
#if defined(RTCONFIG_DUALWAN)
				if(nvram_get_int("wandog_enable"))
					write_ct_timeout("icmp", NULL, 1);
				else
#endif
					write_ct_timeout("icmp", NULL, 0);
				sleep(2);
			}
		}
	}
}

void setup_conntrack(void)
{
	unsigned int v[10];
	char p[100];
	char buf[100];
	int i;

#ifdef RTCONFIG_CONCURRENTREPEATER
	return;		/* don't need it for concurrent repeater */
#endif

	snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_tcp_timeout"));
	if (strcmp(p,"0 0 0 0 0 0 0 0 0 0") && sscanf(p, "%u%u%u%u%u%u%u%u%u%u",
		&v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9]) == 10) {	// lightly verify
		write_tcp_timeout("established", v[1]);
		write_tcp_timeout("syn_sent", v[2]);
		write_tcp_timeout("syn_recv", v[3]);
		write_tcp_timeout("fin_wait", v[4]);
		write_tcp_timeout("time_wait", v[5]);
		write_tcp_timeout("close", v[6]);
		write_tcp_timeout("close_wait", v[7]);
		write_tcp_timeout("last_ack", v[8]);
	}
	else {
		v[1] = read_tcp_timeout("established");
		v[2] = read_tcp_timeout("syn_sent");
		v[3] = read_tcp_timeout("syn_recv");
		v[4] = read_tcp_timeout("fin_wait");
		v[5] = read_tcp_timeout("time_wait");
		v[6] = read_tcp_timeout("close");
		v[7] = read_tcp_timeout("close_wait");
		v[8] = read_tcp_timeout("last_ack");
		snprintf(buf, sizeof(buf), "0 %u %u %u %u %u %u %u %u 0",
			v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
			nvram_set("ct_tcp_timeout", buf);
	}

	setup_udp_timeout(FALSE);

	snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_timeout"));
	if (strcmp(p,"0 0") && sscanf(p, "%u%u", &v[0], &v[1]) == 2) {
//		write_ct_timeout("generic", NULL, v[0]);
		write_ct_timeout("icmp", NULL, v[1]);
	}
	else {
		v[0] = read_ct_timeout("generic", NULL);
		v[1] = read_ct_timeout("icmp", NULL);
		snprintf(buf, sizeof(buf), "%u %u", v[0], v[1]);
		nvram_set("ct_timeout", buf);
	}

#ifdef LINUX26
	snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_hashsize"));
	i = atoi(p);
	if (i >= 127) {
		f_write_string("/sys/module/nf_conntrack/parameters/hashsize", p, 0, 0);
	}
	else if (f_read_string("/sys/module/nf_conntrack/parameters/hashsize", buf, sizeof(buf)) > 0) {
		if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
		if (atoi(buf) > 0) nvram_set("ct_hashsize", buf);
	}
#endif
#ifdef LINUX26
	snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_max"));
	i = atoi(p);
	if (i >= 128) {
		f_write_string("/proc/sys/net/nf_conntrack_max", p, 0, 0);
	}
	else if (f_read_string("/proc/sys/net/nf_conntrack_max", buf, sizeof(buf)) > 0) {
		if (atoi(buf) > 0) nvram_set("ct_max", buf);
	}
#else
	snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_max"));
	i = atoi(p);
	if (i >= 128) {
		f_write_string("/proc/sys/net/ipv4/netfilter/ip_conntrack_max", p, 0, 0);
	}
	else if (f_read_string("/proc/sys/net/ipv4/netfilter/ip_conntrack_max", buf, sizeof(buf)) > 0) {
		if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
		if (atoi(buf) > 0) nvram_set("ct_max", buf);
	}
#endif
#ifdef LINUX26
	if (f_exists("/proc/sys/net/netfilter/nf_conntrack_expect_max")) {
		snprintf(p, sizeof(p), "%s", nvram_safe_get("ct_expect_max"));
		i = atoi(p);
		if (i >= 1) {
			f_write_string("/proc/sys/net/netfilter/nf_conntrack_expect_max", p, 0, 0);
		}
		else if (f_read_string("/proc/sys/net/netfilter/nf_conntrack_expect_max", buf, sizeof(buf)) > 0) {
			if (atoi(buf) > 0) nvram_set("ct_expect_max", buf);
		}
	}
#endif
#if 0
	if (!nvram_match("nf_rtsp", "0")) {
		ct_modprobe("rtsp");
	}
	else {
		ct_modprobe_r("rtsp");
	}

	if (!nvram_match("nf_h323", "0")) {
		ct_modprobe("h323");
	}
	else {
		ct_modprobe_r("h323");
	}

#ifdef LINUX26
	if (!nvram_match("nf_sip", "0")) {
		ct_modprobe("sip");
	}
	else {
		ct_modprobe_r("sip");
	}
#endif
#endif
	// !!TB - FTP Server
#ifdef RTCONFIG_FTP
	i = nvram_get_int("ftp_port");
	if (nvram_match("ftp_enable", "1") && (i > 0) && (i != 21))
	{
		char ports[32];

		snprintf(ports, sizeof(ports), "ports=21,%d", i);
		ct_modprobe("ftp", ports);
	}
	else 
#endif
	if (!nvram_match("nf_ftp", "0")
#ifdef RTCONFIG_FTP
		|| nvram_match("ftp_enable", "1")	// !!TB - FTP Server
#endif
		) {
		ct_modprobe("ftp");
	}
	else {
		ct_modprobe_r("ftp");
	}

	if (!nvram_match("nf_pptp", "0")) {
		ct_modprobe("proto_gre");
		ct_modprobe("pptp");
	}
	else {
		ct_modprobe_r("pptp");
		ct_modprobe_r("proto_gre");
	}
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916) || defined(RTCONFIG_MT798X)
	f_write_string("/proc/sys/net/netfilter/nf_conntrack_helper", "1", 0, 0);
#endif

#ifdef RTCONFIG_BCMARM
	/* mark only out of window RST segments as INVALID */
	f_write_string("/proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal", "1", 0, 0);
#endif
	modprobe("xt_conntrack");
	modprobe("xt_mscs");
	modprobe("xt_scs");
	modprobe("xt_mac");
	modprobe("xt_mac_extend");
	modprobe("xt_mark");
	modprobe("xt_mark_extend");

}

void setup_pt_conntrack(void)
{
	if (nvram_match("fw_pt_rtsp", "1")) {
		ct_modprobe("rtsp", "ports=554,8554");
	}
	else {
		ct_modprobe_r("rtsp");
	}

	if (nvram_match("fw_pt_h323", "1")) {
		ct_modprobe("h323");
	}
	else {
		ct_modprobe_r("h323");
	}

#ifdef LINUX26
#if defined(BRTAC828)
	if (!nvram_match("fw_pt_sip", "0")) {
		if (nvram_get_int("fw_pt_sip_mode") == 1) {
			ct_modprobe_r("sip");
			ct_modprobe("cisco_sip");
		} else {
			ct_modprobe_r("cisco_sip");
			ct_modprobe("sip");
		}
	}
	else {
		ct_modprobe_r("sip");
		ct_modprobe_r("cisco_sip");
	}
#else
	if (nvram_match("fw_pt_sip", "1")) {
		ct_modprobe("sip");
	}
	else {
		ct_modprobe_r("sip");
	}
#endif
#endif
}

void remove_conntrack(void)
{
	ct_modprobe_r("pptp");
	ct_modprobe_r("ftp");
	ct_modprobe_r("rtsp");
	ct_modprobe_r("h323");
#ifdef LINUX26
	ct_modprobe_r("sip");
#if defined(BRTAC828)
	ct_modprobe_r("cisco_sip");
#endif
#endif
}

void inc_mac(char *mac, int plus)
{
	unsigned char m[6];
	int i;

	for (i = 0; i < 6; i++)
		m[i] = (unsigned char) strtol(mac + (3 * i), (char **)NULL, 16);
	while (plus != 0) {
		for (i = 5; i >= 3; --i) {
			m[i] += (plus < 0) ? -1 : 1;
			if (m[i] != 0) break;	// continue if rolled over
		}
		plus += (plus < 0) ? 1 : -1;
	}
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
		m[0], m[1], m[2], m[3], m[4], m[5]);
}

void set_mac(const char *ifname, const char *nvname, int plus)
{
	int sfd;
	struct ifreq ifr;
	int up;
	int j;
	char et_hwaddr[32];

	if ((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		_dprintf("%s: %s %d\n", ifname, __FUNCTION__, __LINE__);
		return;
	}

	strcpy(ifr.ifr_name, ifname);

	up = 0;
	if (ioctl(sfd, SIOCGIFFLAGS, &ifr) == 0) {
		if ((up = ifr.ifr_flags & IFF_UP) != 0) {
			ifr.ifr_flags &= ~IFF_UP;
			if (ioctl(sfd, SIOCSIFFLAGS, &ifr) != 0) {
				_dprintf("%s: %s %d\n", ifname, __FUNCTION__, __LINE__);
			}
		}
	}
	else {
		_dprintf("%s: %s %d\n", ifname, __FUNCTION__, __LINE__);
	}

	snprintf(et_hwaddr, sizeof(et_hwaddr), "%s", get_lan_hwaddr());

	if (!ether_atoe(nvram_safe_get(nvname), (unsigned char *)&ifr.ifr_hwaddr.sa_data)) {
		if (!ether_atoe(et_hwaddr, (unsigned char *)&ifr.ifr_hwaddr.sa_data)) {

			// goofy et0macaddr, make something up
			nvram_set("lan_hwaddr", "00:01:23:45:67:89");
			nvram_set(get_lan_mac_name(), "00:01:23:45:67:89");

			ifr.ifr_hwaddr.sa_data[0] = 0;
			ifr.ifr_hwaddr.sa_data[1] = 0x01;
			ifr.ifr_hwaddr.sa_data[2] = 0x23;
			ifr.ifr_hwaddr.sa_data[3] = 0x45;
			ifr.ifr_hwaddr.sa_data[4] = 0x67;
			ifr.ifr_hwaddr.sa_data[5] = 0x89;
		}

		while (plus-- > 0) {
			for (j = 5; j >= 3; --j) {
				ifr.ifr_hwaddr.sa_data[j]++;
				if (ifr.ifr_hwaddr.sa_data[j] != 0) break;	// continue if rolled over
			}
		}
	}

	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	if (ioctl(sfd, SIOCSIFHWADDR, &ifr) == -1) {
		_dprintf("Error setting %s address\n", ifname);
	}

	if (up) {
		if (ioctl(sfd, SIOCGIFFLAGS, &ifr) == 0) {
			ifr.ifr_flags |= IFF_UP|IFF_RUNNING;
			if (ioctl(sfd, SIOCSIFFLAGS, &ifr) == -1) {
				_dprintf("%s: %s %d\n", ifname, __FUNCTION__, __LINE__);
			}
		}
		else {
			_dprintf("%s: %s %d\n", ifname, __FUNCTION__, __LINE__);
		}
	}

	close(sfd);
}

/*
const char *default_wanif(void)
{
	return ((strtoul(nvram_safe_get("boardflags"), NULL, 0) & BFL_ENETVLAN) ||
		(check_hw_type() == HW_BCM4712)) ? "vlan1" : "eth1";
}
*/

/*
const char *default_wlif(void)
{
	switch (check_hw_type()) {
	case HW_BCM4702:
	case HW_BCM4704_BCM5325F:
	case HW_BCM4704_BCM5325F_EWC:
		return "eth2";
	}
	return "eth1";

}
*/

void simple_unlock(const char *name)
{
	char fn[256];

	snprintf(fn, sizeof(fn), "/var/lock/%s.lock", name);
	f_write(fn, NULL, 0, 0, 0600);
}

void simple_lock(const char *name)
{
	int n;
	char fn[256];

	n = 5 + (getpid() % 10);
	snprintf(fn, sizeof(fn), "/var/lock/%s.lock", name);
	while (unlink(fn) != 0) {
		if (--n == 0) {
			syslog(LOG_DEBUG, "Breaking %s", fn);
			break;
		}
		sleep(1);
	}
}

void killall_tk(const char *name)
{
	int n;

	if (killall(name, SIGTERM) == 0) {
		n = 10;
		while ((killall(name, 0) == 0) && (n-- > 0)) {
			_dprintf("%s: waiting name=%s n=%d\n", __FUNCTION__, name, n);
			usleep(100 * 1000);
		}
		if (n < 0) {
			n = 10;
			while ((killall(name, SIGKILL) == 0) && (n-- > 0)) {
				_dprintf("%s: SIGKILL name=%s n=%d\n", __FUNCTION__, name, n);
				usleep(100 * 1000);
			}
		}
	}
}

void kill_pid_tk(pid_t pid)
{
	int n;

	if (pid <= 1 && pid >= -1)
		return;

	if (kill(pid, SIGTERM) == 0) {
		n = 10;
		while ((kill(pid, 0) == 0) && (n-- > 0)) {
			_dprintf("%s: waiting pid=%d n=%d\n", __FUNCTION__, pid, n);
			usleep(100 * 1000);
		}
		if (n < 0) {
			n = 10;
			while ((kill(pid, SIGKILL) == 0) && (n-- > 0)) {
				_dprintf("%s: SIGKILL pid=%d n=%d\n", __FUNCTION__, pid, n);
				usleep(100 * 1000);
			}
		}
	}
}

void kill_pidfile_tk(const char *pidfile)
{
	FILE *fp;
	char buf[256];
	pid_t pid = 0;

	if ((fp = fopen(pidfile, "r")) != NULL) {
		if (fgets(buf, sizeof(buf), fp) != NULL)
			pid = strtoul(buf, NULL, 0);
		fclose(fp);
		kill_pid_tk(pid);
	}
}

void kill_pidfile_tk_g(const char *pidfile)
{
	FILE *fp;
	char buf[256];
	pid_t pid = 0;
	pid_t pgid;

	if ((fp = fopen(pidfile, "r")) != NULL) {
		if (fgets(buf, sizeof(buf), fp) != NULL)
			pid = strtoul(buf, NULL, 0);
		fclose(fp);
		pgid = getpgid(pid);
		kill_pid_tk(-pgid);
	}
}

long fappend(FILE *out, const char *fname)
{
	FILE *in;
	char buf[1024];
	int n;
	long r;

	if ((in = fopen(fname, "r")) == NULL) return -1;
	r = 0;
	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, n, out) != n) {
			r = -1;
			break;
		}
		else {
			r += n;
		}
	}
	fclose(in);
	return r;
}

long fappend_file(const char *path, const char *fname)
{
	FILE *f;
	int r = -1;

	if (f_exists(fname) && (f = fopen(path, "a")) != NULL) {
		r = fappend(f, fname);
		fclose(f);
	}
	return r;
}

/* uclibc accepts any of following:
 * [STD][Offset][DST], where
 * [STD] ~ any [a-zA-Z]
 * [DST] ~ any [a-zA-Z] */
#define CONVERT_TZ_TO_GMT_DST
#ifdef CONVERT_TZ_TO_GMT_DST
int gettzoffset(char *tzstr, char *tzstr1, int size1)
{
	char offstr[32];
	char *tzptr = tzstr;
	char *offptr = offstr;
	int ret = 0;
	int dst = 0;

	memset(offstr, 0, sizeof(offstr));
	for ( ; *tzptr; tzptr++) {
		if (*tzptr=='-'||*tzptr=='+'||*tzptr==':'||isdigit(*tzptr)) {
			*offptr++ = *tzptr;
			ret = 1;
		} else if (ret) {
			dst = isalpha(*tzptr);
			break;
		}
	}

	if (ret)
		snprintf(tzstr1, size1, "GMT%s%s", offstr, dst ? "DST" : "");
	return ret;
}
#endif


#ifdef HND_ROUTER
#define LOCALTIME_FILE "/etc/localtime"
#define ZONEINFO_PATH "/rom/usr/share/zoneinfo/"
typedef struct zoneinfo {
	char *tz_name;
	char *timezone;
}zoneinfo_t;

const zoneinfo_t tz_list[] = {
	{"UTC12",	"Etc/GMT+12"},		// (GMT-12:00) Eniwetok, Kwajalein
	{"UTC11",	"US/Samoa"},		// (GMT-11:00) Midway Island, Samoa
	{"UTC10",	"US/Hawaii"},		// (GMT-10:00) Hawaii
       	{"NAST9DST",	"US/Alaska"},		// (GMT-09:00) Alaska
        {"PST8DST",	"US/Pacific"},		// (GMT-08:00) Pacific Time (US & Canada)
        {"MST7DST_1",	"US/Mountain"},		// (GMT-07:00) Mountain Time (US & Canada)
        {"MST7_2",	"US/Arizona"},		// (GMT-07:00) Arizona
	{"MST7_3",	"America/Mazatlan"},	// (GMT-07:00) La Paz, Mazatlan //MST7DST_3
        {"CST6_2",	"Canada/Saskatchewan"},	// (GMT-06:00) Saskatchewan
        {"CST6_3",	"Mexico/General"},	// (GMT-06:00) Guadalajara, Mexico City
        {"CST6_3_1",	"America/Monterrey"},	// (GMT-06:00) Monterreyi, Chihuahua
        {"UTC6DST",	"US/Central"},		// (GMT-06:00) Central Time (US & Canada)
        {"EST5DST",	"US/Eastern"},		// (GMT-05:00) Eastern Time (US & Canada)
        {"UTC5_1",	"US/East-Indiana"},	// (GMT-05:00) Indiana (East)    US Eastern
       	{"UTC5_2",	"America/Bogota"},	// (GMT-05:00) Bogota, Lima, Quito   SA Pacific
	{"AST4DST",     "Canada/Atlantic"},     // (GMT-04:00) Atlantic Time (Canada)
	{"UTC4_1",      "America/Manaus"},      // (GMT-04:00) La Paz
	{"UTC4_2",	"America/Caracas"},	// (GMT-04:00) Caracas
        {"UTC4DST_2",	"America/Santiago"},	// (GMT-04:00) Santiago
        {"NST3.30DST",	"Canada/Newfoundland"},	// (GMT-03:30) Newfoundland
        {"EBST3",	"America/Araguaina"},	// (GMT-03:00) Brasilia //EBST3DST_1
		{"UTC3DST",     "America/Saint-Pierre-et-Miquelon"},    // (GMT-03:00) Saint Pierre	//UTC2DST
		{"UTC3",	"America/Araguaina"},	// (GMT-03:00) Buenos Aires, Georgetown
        {"UTC2",	"Atlantic/South_Georgia"},	// (GMT-02:00) South Georgia
		{"UTC2DST_1",  "America/Godthab"}, // (GMT-02:00) Greenland    //UTC2_1 //EBST3DST_2
        {"EUT1DST",     "Atlantic/Azores"},	// (GMT-01:00) Azores
        {"UTC1",        "Atlantic/Cape_Verde"},	// (GMT-01:00) Cape Verde Is.
        {"GMT0",        "GMT"},			// (GMT+00:00) Greenwich Mean Time
        {"GMT0DST_1",   "Europe/Dublin"},	// (GMT+00:00) Dublin, Edinburg, Lisbon, London
        {"GMT0DST_2",   "Africa/Casablanca"},	// (GMT+00:00) Casablanca
        {"GMT0_2",      "Africa/Monrovia"},	// (GMT+00:00) Monrovia
        {"UTC-1DST_1",  "Europe/Belgrade"},	// (GMT+01:00) Belgrade, Bratislava, Budapest
        {"UTC-1DST_1_1","Europe/Ljubljana"},	// (GMT+01:00) Ljubljana, Prague
        {"UTC-1_2",     "Europe/Sarajevo"},	// (GMT+01:00) Sarajevo, Skopje
        {"UTC-1DST_2",  "Europe/Warsaw"},	// (GMT+01:00) Warsaw, Zagreb
	{"MET-1DST",    "Europe/Copenhagen"},	// (GMT+01:00) Copenhagen, Stockholm, Oslo
        {"MET-1DST_1",  "Europe/Madrid"},	// (GMT+01:00) Madrid, Paris
        {"MEZ-1DST",    "Europe/Amsterdam"},	// (GMT+01:00) Amsterdam, Berlin, Brussels
        {"MEZ-1DST_1",  "Europe/Rome"},		// (GMT+01:00) Rome, Vienna, Bern
        {"UTC-1_3",     "Africa/Lagos"},	// (GMT+01:00) West Central Africa
        {"UTC-2DST",    "Europe/Vilnius"},	// (GMT+02:00) Vilnus, Bucharest, sofija
        {"UTC-2DST_3",  "Europe/Helsinki"},	// (GMT+02:00) Helsiki
        {"EST-2",	"Africa/Cairo"},	// (GMT+02:00) Cairo
        {"UTC-2DST_4",  "Europe/Riga"},		// (GMT+02:00) Riga, Tallinn
        {"UTC-2DST_2",  "Europe/Athens"},	// (GMT+02:00) Athens
        {"IST-2DST",    "Asia/Jerusalem"},	// (GMT+02:00) Jerusalem
        {"EET-2DST",    "Europe/Kiev"},		// (GMT+02:00) Kiev
        {"UTC-2_1",     "Europe/Kaliningrad"},	// (GMT+02:00) Kaliningrad
        {"SAST-2",      "Africa/Harare"},	// (GMT+02:00) Harare
        {"UTC-3_1",     "Asia/Kuwait"},		// (GMT+03:00) Kuwait, Riyadh
        {"UTC-3_2",     "Africa/Nairobi"},	// (GMT+03:00) Nairobi
        {"UTC-3_3",     "Europe/Minsk"},	// (GMT+03:00) Minsk
        {"UTC-3_4",     "Europe/Moscow"},	// (GMT+03:00) Moscow, St. Petersburg
	{"UTC-3_5",     "Europe/Volgograd"},    // (GMT+03:00) Volgograd        //UTC-4_7
        {"IST-3",       "Asia/Baghdad"},	// (GMT+03:00) Baghdad
        {"UTC-3_6",     "Asia/Istanbul"},	// (GMT+03:00) Istanbul
        {"UTC-3.30",	"Asia/Tehran"},		// (GMT+03:00) Tehran       //UTC-3.30DST 
        {"UTC-4_1",     "Asia/Muscat"},		// (GMT+04:00) Abu Dhabi, Muscat
        {"UTC-4_5",     "Europe/Samara"},	// (GMT+04:00) Izhevsk, Samara
	{"UTC-4_4",     "Asia/Tbilisi"},	// (GMT+04:00) Tbilisi, Yerevan
        {"UTC-4_6",	"Asia/Baku"},		// (GMT+04:00) Baku
        {"UTC-4.30",    "Asia/Kabul"},		// (GMT+04:30) Kabul
        {"UTC-5",       "Asia/Karachi"},	// (GMT+05:00) Islamabad, Karachi, Tashkent
        {"UTC-5_1",     "Asia/Yekaterinburg"},	// (GMT+05:00) Yekaterinburg
		{"UTC-5_2",       "Asia/Almaty"},     // (GMT+05:00) Almaty, Astana	//RFT-6
        {"UTC-5.30_2",  "Asia/Kolkata"},	// (GMT+05:00) Kolkata, Chennai
        {"UTC-5.30_1",  "Asia/Calcutta"},	// (GMT+05:30) Mumbai, New Delhi
        {"UTC-5.30",    "Asia/Calcutta"},	// (GMT+05:30) Sri Jayawardenepura
	{"UTC-5.45",    "Asia/Kathmandu"},	// (GMT+05:45) Kathmandu
        {"UTC-6",       "Asia/Dhaka"},		// (GMT+06:00) Dhaka
        {"UTC-6_2",     "Asia/Novosibirsk"},	// (GMT+06:00) Novosibirsk
        {"UTC-6.30",    "Asia/Yangon"},		// (GMT+06:30) Yangon
        {"UTC-7",       "Asia/Bangkok"},	// (GMT+07:00) Bangkok, Hanoi, Jakarta
        {"UTC-7_2",     "Asia/Krasnoyarsk"},	// (GMT+07:00) Krasnoyarsk
        {"CST-8",       "Asia/Shanghai"},	// (GMT+08:00) Beijing, Hong Kong 
        {"CST-8_1",     "Asia/Chongqing"},	// (GMT+08:00) Chongqing, Urumqi
        {"SST-8",       "Asia/Kuala_Lumpur"},	// (GMT+08:00) Kuala_Lumpur, Singapore
        {"CCT-8",       "Asia/Taipei"},		// (GMT+08:00) Taipei
        {"WAS-8",       "Australia/Perth"},	// (GMT+08:00) Perth
        {"UTC-8",       "Asia/Irkutsk"},	// (GMT+08:00) Ulaan Baatar
        {"UTC-8_1",     "Asia/Irkutsk"},	// (GMT+08:00) Irkutsk
        {"UTC-9_1",     "Asia/Seoul"},		// (GMT+09:00) Seoul
        {"UTC-9_3",     "Asia/Yakutsk"},	// (GMT+09:00) Yakutsk
        {"JST-9",       "Asia/Tokyo"},		// (GMT+09:00) Osaka, Sapporo, Tokyo
        {"CST-9.30",    "Australia/Darwin"},	// (GMT+09:30) Darwin
        {"UTC-9.30DST", "Australia/Adelaide"},	// (GMT+09:30) Adelaide
        {"UTC-10DST_1", "Australia/Canberra"},	// (GMT+10:00) Canberra, Melbourne, Sydney
        {"UTC-10_2",    "Australia/Brisbane"},	// (GMT+10:00) Brisbane"
        {"UTC-10_4",    "Asia/Vladivostok"},	// (GMT+10:00) Vladivostok
        {"UTC-11_4",    "Asia/Magadan"},	// (GMT+10:00) Asia/Magadan
        {"TST-10TDT",   "Australia/Hobart"},	// (GMT+10:00) Australia/Hobart
        {"UTC-10_6",    "Pacific/Guam"},	// (GMT+10:00) Guam, Port Moresby
        {"UTC-11",      "Pacific/Noumea"},	// (GMT+11:00) Solomon Is.
        {"UTC-11_1",    "Pacific/Noumea"},	// (GMT+11:00) New Caledonia
        {"UTC-11_3",    "Asia/Srednekolymsk"},	// (GMT+11:00) Chokurdakh, Srednekolymsk
	{"UTC-12",      "Pacific/Majuro"},	// (GMT+12:00) Marshall IS.
	{"UTC-12_3",	"Pacific/Fiji"},	// (GMT+12:00) Fiji
	{"UTC-12_2",	"Asia/Anadyr"},		// (GMT+12:00) Anadyr, Petropavlovsk-Kamchatsky
	{"NZST-12DST",  "Pacific/Auckland"},	// (GMT+12:00) Auckland, Wellington
	{"UTC-13",      "Pacific/Tongatapu"},	// (GMT+13:00) Nuku'alofa
	{ NULL }
};
#endif

void time_zone_x_mapping(void)
{
	FILE *fp;
	char tmpstr[32];
	char *ptr;
	int len;

#ifdef HND_ROUTER
	int idx;
	char cmd[128];
	const zoneinfo_t *pzlist = tz_list;

	if(pzlist) {
		for (idx = 0; pzlist[idx].tz_name; idx++) {
			if (nvram_match("time_zone", pzlist[idx].tz_name)) {
				snprintf(cmd, sizeof(cmd), "ln -s %s%s %s",
						ZONEINFO_PATH,
						pzlist[idx].timezone,
						LOCALTIME_FILE);
				unlink(LOCALTIME_FILE);
				system(cmd);
				break;
			}
		}
	}
#endif

	/* pre mapping because time_zone area changed*/
	if (nvram_match("time_zone", "UTC-3.30DST")){
		nvram_set("time_zone", "UTC-3.30");               /*Tehran*/
                nvram_set("time_zone_dst", "0");
	}else if (nvram_match("time_zone", "MST7DST_3")){
		nvram_set("time_zone", "MST7");               /*Mazatlan*/
		nvram_set("time_zone_dst", "0");
	}else if (nvram_match("time_zone", "RFT-6")){
        nvram_set("time_zone", "UTC-5_2");               /*Almaty, Astana*/
    }else if (nvram_match("time_zone", "EBST3DST_2") || nvram_match("time_zone", "UTC2_1")){
		nvram_set("time_zone", "UTC2DST_1");		/*Greenland*/
		nvram_set("time_zone_dst", "1");
	}else if (nvram_match("time_zone", "UTC-12DST")){             /*Fiji*/
                nvram_set("time_zone", "UTC-12_3");
                nvram_set("time_zone_dst", "0");
	}else if (nvram_match("time_zone", "CST6DST_3")){		/*Guadalajara, Mexico City*/
		nvram_set("time_zone", "CST6_3");
		nvram_set("time_zone_dst", "0");
	}else if (nvram_match("time_zone", "CST6DST_3_1")){	/*Monterrey*/
                nvram_set("time_zone", "CST6_3_1");
		nvram_set("time_zone_dst", "0");
	}
	/*}else if (nvram_match("time_zone", "KST-9KDT"))
		nvram_set("time_zone", "UCT-9_1");
	else if (nvram_match("time_zone", "RFT-9RFTDST"))
		nvram_set("time_zone", "UCT-9_2");
	else if (nvram_match("time_zone", "UTC-2DST_1"))	//Minsk
		nvram_set("time_zone", "UTC-3_3");
	else if (nvram_match("time_zone", "UTC-4_2"))		//Moscow, St. Petersburg
		nvram_set("time_zone", "UTC-3_4");
	else if (nvram_match("time_zone", "UTC-4_3"))		//Volgograd
		nvram_set("time_zone", "UTC-3_5");
	else if (nvram_match("time_zone", "UTC-6_1"))		//Yekaterinburg
		nvram_set("time_zone", "UTC-5_1");
	else if (nvram_match("time_zone", "UTC-7_1"))		//Novosibirsk
		nvram_set("time_zone", "UTC-6_2");
	else if (nvram_match("time_zone", "CST-8_2"))		//Krasnoyarsk
		nvram_set("time_zone", "CST-7_2");
	else if (nvram_match("time_zone", "UTC-9_2"))		//Irkutsk
		nvram_set("time_zone", "UTC-8_1");
	else if (nvram_match("time_zone", "UTC-10_3"))		//Yakutsk
		nvram_set("time_zone", "UTC-9_3");
	else if (nvram_match("time_zone", "UTC-11_2"))		//Vladivostok
		nvram_set("time_zone", "UTC-10_4");*/	
	else if (nvram_match("time_zone", "UTC-12_1"))          /*Magadan*/
		nvram_set("time_zone", "UTC-10_6");
	else if (nvram_match("time_zone", "UTC4.30"))		/*Caracas*/
                nvram_set("time_zone", "UTC4_2");
	else if (nvram_match("time_zone", "NORO2DST")){           /*South Georgia*/
                nvram_set("time_zone", "UTC2");
		nvram_set("time_zone_dst", "0");
	}
	else if (nvram_match("time_zone", "UTC-1_2")){           /*Sarajevo, Skopje*/
                nvram_set("time_zone", "UTC-1DST_1_2");
                nvram_set("time_zone_dst", "1");
        }
	else if (nvram_match("time_zone", "EST-2DST")){		/*Cairo*/
		nvram_set("time_zone", "EST-2");
		nvram_set("time_zone_dst", "0");
	}
	else if (nvram_match("time_zone", "UTC-4DST_2")){	/*Baku*/
		nvram_set("time_zone", "UTC-4_6");
		nvram_set("time_zone_dst", "0");
	}
	else if (nvram_match("time_zone", "UTC-10_5")){		/*Magadan*/
		nvram_set("time_zone", "UTC-11_4");
	}
	else if (nvram_match("time_zone", "EBST3DST_1")){	/*Brasilia*/
		nvram_set("time_zone", "EBST3");
		nvram_set("time_zone_dst", "0");
	}
	else if (nvram_match("time_zone", "UTC-4_7")){  /*Volgograd*/
		nvram_set("time_zone", "UTC-3_5");
	}	
	else if (nvram_match("time_zone", "UTC-6_2")){  /*Novosibirsk*/
		nvram_set("time_zone", "UTC-7_3");
	}
	else if (nvram_match("time_zone", "UTC2DST")){  /*Saint-Pierre-et-Miquelon*/
		nvram_set("time_zone", "UTC3DST");
	}
	else if (nvram_match("time_zone", "JST")){	/* convert JST to JST-9 */
		nvram_set("time_zone", "JST-9");
	}
	else if (nvram_match("time_zone", "UTC4DST_3")){  /* convert UTC4DST_3 to UTC3_1 */
        nvram_set("time_zone", "UTC3_1");
    }


	snprintf(tmpstr, sizeof(tmpstr), "%s", nvram_safe_get("time_zone"));
	/* replace . with : */
	while ((ptr=strchr(tmpstr, '.'))!=NULL) *ptr = ':';
	/* terminate string at first instance of '_', if there is one */
	ptr = strchr(tmpstr, '_');
	if(ptr) 
        	*ptr = '\0';

	/* check time_zone_dst for daylight saving */
	if (nvram_get_int("time_zone_dst")){
		/* append time zone dst offset */
		len = strnlen(tmpstr, sizeof(tmpstr));
		snprintf(tmpstr + len, sizeof(tmpstr) - len, ",%s", nvram_safe_get("time_zone_dstoff"));
	}
#ifdef CONVERT_TZ_TO_GMT_DST
	else	gettzoffset(tmpstr, tmpstr, sizeof(tmpstr));
#endif

	nvram_set("time_zone_x", tmpstr);

	if ((fp = fopen("/etc/TZ", "w")) != NULL) {
		fprintf(fp, "%s\n", tmpstr);
		fclose(fp);
	}
#ifdef RTCONFIG_AVOID_TZ_ENV
	/* add a "/tmp/TZ" as a hard link of /etc/TZ in process that run chroot() */
	if ((fp = fopen("/tmp/TZ", "w")) != NULL) {
		fprintf(fp, "%s\n", tmpstr);
		fclose(fp);
	}
#endif
}


/* Get the timezone from NVRAM and set the timezone in the kernel
 * and export the TZ variable 
 */
void
setup_timezone(void)
{
#ifndef RC_BUILDTIME
#define RC_BUILDTIME	1704067200	// Jan  1 00:00:00 GMT 2024
#endif
	time_t now;
	struct timeval tv = { RC_BUILDTIME, 0 };
	struct timeval *tvp = NULL;

	/* Export TZ variable for the time libraries to 
	 * use.
	 */
	time_zone_x_mapping();
#ifndef RTCONFIG_AVOID_TZ_ENV
	setenv("TZ", nvram_get("time_zone_x"), 1);
#endif

	time(&now);

	struct tm gm, local;
	struct timezone tz;

	/* Update kernel timezone */
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);
	gm.tm_isdst = local.tm_isdst;
	memset(&tz, 0, sizeof(tz));	// On Linux, with glibc, the setting of the tz_dsttime field of struct timezone has never been used by settimeofday() or gettimeofday().
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;

#if !defined(__GLIBC__) && !defined(__UCLIBC__) /* musl */
	if (syscall(SYS_settimeofday, NULL, &tz))
		_dprintf("[%s]: can't set kernel time zone!!!!\n");
#elif defined(RTCONFIG_HND_ROUTER_BE_4916)
	if (syscall(__NR_settimeofday, NULL, &tz))
		_dprintf("[%s]: can't set kernel time zone!!!!\n");
#endif

	/* Setup sane start time */
	if (now < RC_BUILDTIME) {
		struct sysinfo info;

		sysinfo(&info);
		tv.tv_sec += info.uptime;
		tvp = &tv;
	}

#if defined(RTCONFIG_HND_ROUTER_BE_4916)
	if (tvp)
		settimeofday(tvp, NULL);
#else
	settimeofday(tvp, &tz);
#endif
}

int get_meminfo_item(const char *name)
{
	FILE *fp;
	char memdata[256] = {0};
	int mem = 0;

	if (!name || *name == '\0')
		return -1;

	if ((fp = fopen("/proc/meminfo", "r")) != NULL) {
		/* get one memory parameter specified by the name */
		while (fgets(memdata, 255, fp) != NULL) {
			if (strstr(memdata, name) != NULL) {
				sscanf(memdata, "%*s %d kB", &mem);
				break;
			}
		}
		fclose(fp);
	}

	return mem;
}

#ifdef RTCONFIG_SHP
void restart_lfp()
{
	char v[32];

	if(nvram_get_int("lfp_disable")==0) {
		sprintf(v, "%x", inet_addr(nvram_safe_get("lan_ipaddr")));
		f_write_string("/proc/net/lfpctrl", v, 0, 0);
	}
	else {
		f_write_string("/proc/net/lfpctrl", "", 0, 0);
	}
}
#endif

#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_WIRELESSREPEATER)
int setup_dnsmq(int mode)
{
	char v[32];
	char tmp[32];

	if(mode) {
		// setup ebtables
		eval("ebtables", "-F");
		eval("ebtables", "-t", "broute", "-F");
		eval("ebtables", "-t", "broute", "-I", "BROUTING", "-d", "00:E0:11:22:33:44", "-j", "redirect", "--redirect-target", "DROP");
	}
	else {
		eval("ebtables", "-F");
		eval("ebtables", "-t", "broute", "-F");
#ifdef CONFIG_BCMWL5
		if (is_ure(nvram_get_int("wlc_band")))
#endif
		eval("ebtables", "-I", "FORWARD", "-i", nvram_safe_get(wlc_nvname("ifname")), "-j", "DROP");
	}	
	
	eval("iptables", "-t", "nat", "-F", "PREROUTING");
	eval("iptables", "-t", "nat", "-I", "PREROUTING", "-p", "udp", "--dport", "53",
		"-j", "DNAT", "--to-destination", strcat_r(nvram_safe_get("lan_ipaddr"), ":18018", tmp));

	if(mode) {
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		if (!is_psta(nvram_get_int("wlc_band")) && !is_psr(nvram_get_int("wlc_band")))
#endif
		{
			snprintf(tmp, sizeof(tmp), "%s:%d", nvram_safe_get("lan_ipaddr"), nvram_get_int("http_lanport") ? : 80);
			eval("iptables", "-t", "nat", "-I", "PREROUTING", "-p", "tcp", "-d", "10.0.0.1", "--dport", "80",
				"-j", "DNAT", "--to-destination", tmp);
		}
	
		//sprintf(v, "%x my.%s", inet_addr("10.0.0.1"), get_productid());
		sprintf(v, "%x %s", inet_addr(nvram_safe_get("lan_ipaddr")), DUT_DOMAIN_NAME);
		f_write_string("/proc/net/dnsmqctrl", v, 0, 0);
	}
	else {
		// setup ebtables and iptables
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		if (!is_psta(nvram_get_int("wlc_band")) && !is_psr(nvram_get_int("wlc_band")))
#endif
		eval("iptables", "-t", "nat", "-I", "PREROUTING", "-p", "tcp", "-d", "10.0.0.1", "--dport", "80",
			"-j", "DNAT", "--to-destination", strcat_r(nvram_safe_get("lan_ipaddr"), ":18017", tmp));
	
		f_write_string("/proc/net/dnsmqctrl", "", 0, 0);
	}

	return 0;
}
#endif


int
is_invalid_char_for_volname(char c)
{
	int ret = 0;

	if (c < 0x20)
		ret = 1;
#if 0
	else if (c >= 0x21 && c <= 0x2c)
		ret = 1;
#else	/* allow '+' */
	else if (c >= 0x21 && c <= 0x2a)	/* !"#$%&'()* */
		ret = 1;
	else if (c == 0x2c)			/* , */
		ret = 1;
#endif
	else if (c >= 0x2e && c <= 0x2f)
		ret = 1;
	else if (c >= 0x3a && c <= 0x40)
		ret = 1;
	else if (c >= 0x5b && c <= 0x5e)
		ret = 1;
	else if (c == 0x60)
		ret = 1;
	else if (c >= 0x7b)
		ret = 1;
	return ret;
}

int
is_valid_volname(const char *name)
{
	int len, i;

	if (!name)
		return 0;

	len = strlen(name);
	for (i = 0; i < len ; i++) {
		if (is_invalid_char_for_volname(name[i])) {
			len = 0;
			break;
		}
	}
	return len;
}


void stop_if_misc(void)
{
	DIR *dir;
	struct dirent *dirent;
	struct ifreq ifr;
	int sfd;

	if ((dir = opendir("/proc/sys/net/ipv4/conf")) != NULL) {
		while ((dirent = readdir(dir)) != NULL) {
			if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
				continue;

			if (strcmp(dirent->d_name, "all") &&
				strcmp(dirent->d_name, "default") &&
				strcmp(dirent->d_name, "lo") &&
				strncmp(dirent->d_name, "br", 2) &&
				strncmp(dirent->d_name, "eth", 3) &&
				strncmp(dirent->d_name, "ra", 2) &&
				!((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0))
			{
				strcpy(ifr.ifr_name, dirent->d_name);
				if (!ioctl(sfd, SIOCGIFFLAGS, &ifr) && (ifr.ifr_flags & IFF_UP))
					ifconfig(ifr.ifr_name, 0, NULL, NULL);

				close(sfd);
			}

		}
		closedir(dir);
	}
}

int mssid_mac_validate(const char *macaddr)
{
	unsigned char mac_binary[6];
	unsigned long long macvalue;
	char macbuf[13];

	if (!macaddr || !strlen(macaddr))
		return 0;

	ether_atoe(macaddr, mac_binary);
	sprintf(macbuf, "%02X%02X%02X%02X%02X%02X",
		mac_binary[0],
		mac_binary[1],
		mac_binary[2],
		mac_binary[3],
		mac_binary[4],
		mac_binary[5]);
	macvalue = strtoll(macbuf, (char **) NULL, 16);
#if defined(RTCONFIG_CONCURRENTREPEATER)
	if (macvalue % 2)
#else
	if (macvalue % 4)
#endif
		return 0;
	else
		return 1;
}

int rand_seed_by_time(void)
{
	time_t atime;
	static unsigned long rand_base = 0;

	time(&atime);
	srand(((unsigned long)atime + rand_base++) % ULONG_MAX);

	return rand();
}

//Andrew add
#ifdef RTCONFIG_CONNTRACK
void conntrack_check(int action)
{
	static unsigned int conntrack_times = 0;
	char cmd[80];

	if (!nvram_match("fb_conntrack_debug", "1"))
		return;

	int pid = pidof("conntrack");
	/*
	time_t now;
	struct tm *info;
	char t_str[10];

	time(&now);
	info = localtime(&now);
	strftime(t_str, 10, "%H:%M:%S", info);
	*/
	switch (action) {
	case CONNTRACK_START:
		//_dprintf("%s conntrack_check, action=[%d][START], pid=[%d]\n", t_str, CONNTRACK_START, pid);
		if (pid < 1) {
			snprintf(cmd, sizeof(cmd), "conntrack -E -p tcp -v %s &", NF_CONNTRACK_FILE);
			_dprintf("cmd=[%s]\n", cmd);
			system(cmd);
			conntrack_times = 0;
		}
		break;

	case CONNTRACK_STOP:
		//_dprintf("%s conntrack_check, action=[%d][STOP], pid=[%d]\n", t_str, CONNTRACK_STOP, pid);
		if (pid >= 1) {
			snprintf(cmd, sizeof(cmd), "kill -SIGTERM %d", pid);
			_dprintf("cmd=[%s]\n", cmd);
			system(cmd);
			conntrack_times = 0;
		}
		break;

	case CONNTRACK_ROTATE:
		//_dprintf("%s conntrack_check, action=[%d][ROTATE], pid=[%d], times=[%d]\n", t_str, CONNTRACK_ROTATE, pid, conntrack_times);
		conntrack_times++;
		if (conntrack_times < ONEDAY)
			return;

		if (pid >= 1) {
			snprintf(cmd, sizeof(cmd), "kill -SIGUSR1 %d", pid);
			_dprintf("cmd=[%s]\n", cmd);
			system(cmd);
			conntrack_times = 0;
		}
		break;
	}

	return;
}
#endif //RTCONFIG_CONNTRACK
//Andrew end

#if defined(RTCONFIG_QCA)
char *get_wpa_supplicant_pidfile(const char *ifname, char *buf, int size)
{
	if(ifname == NULL || buf == NULL || size < 24)
		return "/var/run/wifi-sta0.pid";

	snprintf(buf, size, "/var/run/wifi-%s.pid", ifname);
	return buf;
}

void kill_wifi_wpa_supplicant(int unit)
{
	int band, end;
	char buf[32], *pidfile;

	if (unit > MAX_NR_WL_IF)
		return;
	else if (unit < 0) {
		band = 0;
		end = MAX_NR_WL_IF;
	}
	else {
		band = end = unit;
	}
	for(; band < end; band++) {
		const char *ifname = get_staifname(band);
		pidfile = get_wpa_supplicant_pidfile(ifname, buf, sizeof(buf));
		kill_pidfile_tk(pidfile);
		unlink(pidfile);
		doSystem("ifconfig %s down", ifname);
	}
}
#endif	/* RTCONFIG_QCA */

void reset_corefilesize(rlim_t s)
{
	struct rlimit lim;

	getrlimit(RLIMIT_CORE, &lim);
	_dprintf("\nnow sys rlimit core:cur=%d, max=%d\n", (int)lim.rlim_cur, (int)lim.rlim_max);

	lim.rlim_cur = s;
	if (setrlimit(RLIMIT_CORE, &lim) < 0)
		_dprintf("\nreset core file size failed\n\n");
	else
		_dprintf("\nreset core file size as %d\n\n", s);
}

#define DEBUGLOG_KLOG_PATH "/jffs/debuglog_klog.tgz"
#define DEBUGLOG_CORE_PATH "/jffs/debuglog_core.tgz"
#define DEBUGLOG_SLOG_PATH "/jffs/debuglog_slog.tgz"
extern void collect_misclog(char* file_list, size_t len);
extern void delete_tmplog();
void collect_debuglog(int type)
{
	char cmd[1024] = {0};
	char path[128] = {0};
	char buf[1024] = {0};

	if (type & DEBUGLOG_KLOG) {
		strlcpy(path, "/tmp/klog.txt", sizeof(path));
		snprintf(cmd, sizeof(cmd), "dmesg -r > %s", path);
		system(cmd);
		snprintf(cmd, sizeof(cmd), "tar zcf %s %s", DEBUGLOG_KLOG_PATH, path);
		system(cmd);
		unlink(path);
	}
	if (type & DEBUGLOG_CORE) {
		strlcpy(path, "/tmp/corelist.txt", sizeof(path));
		snprintf(cmd, sizeof(cmd), "ls -l /tmp/core-* > %s", path);
		system(cmd);
		snprintf(cmd, sizeof(cmd), "tar zcf %s /tmp/core-* %s", DEBUGLOG_CORE_PATH, path);
		system(cmd);
		unlink(path);
	}
	if (type & DEBUGLOG_SLOG) {
		snprintf(cmd, sizeof(cmd), "tar zcf %s /tmp/syslog.log*", DEBUGLOG_SLOG_PATH);
		collect_misclog(buf, sizeof(buf));
		strlcat(cmd, buf, sizeof(cmd));
		system(cmd);
		delete_tmplog();
	}
}

void remove_guillemets_form_str(char *str_in,int sizeofbuf)//sizeofbuf not strlen
{
	char *tmp;
	int i,tmp_idx;
	int len;

	if(str_in == NULL)
		return;

	_dprintf("from [%s]\n",str_in);

	len = sizeofbuf;

	tmp=malloc(len+1);
	if(tmp == NULL)
	{ //make sure conn_diag will not crash
		for(i=0;i<len;i++){
			if(str_in[i]=='<' || str_in[i]=='>')
				memset(str_in,0,len);
		}
		return;
	} else {
		tmp_idx=0;
		//remove '<' or '>'
		memset(tmp,0,len+1);
		for(i=0;i<len;i++){
			if(str_in[i]!='<' && str_in[i]!='>'){
				tmp[tmp_idx] = str_in[i];
				tmp_idx++;
			}
		}
		strncpy(str_in,tmp,len);
		_dprintf("to [%s]\n",str_in);
	}
}

int check_mountpoint(char *mountpoint)
{
	FILE *procpt;
	char line[256], devname[48], mpname[48], system_type[10], mount_mode[128];
	int dummy1, dummy2;

	if ((procpt = fopen("/proc/mounts", "r")) != NULL)
	while (fgets(line, sizeof(line), procpt)) {
		memset(mpname, 0x0, sizeof(mpname));
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;

		if (!strcmp(mpname, mountpoint))
			return 1;
	}

	if (procpt)
		fclose(procpt);

	return 0;
}

extern char **environ;
void envsave(const char* path)
{
	FILE* fp;
	int i = 0;

	if (!*path)
		return;

	fp = fopen(path, "w");
	if (fp) {
		while(environ[i] != NULL) {
			fprintf(fp, "%s\n", environ[i]);
			i++;
		}
		fclose(fp);
	}
}

int remove_ip_rules(const int pref, const int v6)
{
	char tmp[256], pref_str[8], tmp2[256];
	FILE *fp;
	static const char iprule_tmp[] = "/tmp/iprule_tmp";

	// remove current default routing table
	snprintf(pref_str, sizeof(pref_str), "%d", pref);
	snprintf(tmp, sizeof(tmp), "ip %s rule show | grep %d > %s", (v6)?"-6":"-4", pref, iprule_tmp);

	system(tmp);
	fp = fopen(iprule_tmp, "r");
	if(fp)
	{
		while(fgets(tmp, sizeof(tmp), fp))
		{
			eval("ip", (v6)?"-6":"-4", "rule", "del", "priority", pref_str);
		}
		fclose(fp);
	}
	unlink(iprule_tmp);
	return 0;
}
