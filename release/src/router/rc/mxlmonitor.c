/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2023, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <shared.h>
#include <net/ethernet.h>
#include <gsw_device.h>
#include <gsw.h>

#ifdef GSBE18000
#define PORT_MAX		7
#else
#define PORT_MAX		4
#endif
#define NORMAL_PERIOD		3

static int debug;
static struct itimerval itv;

typedef struct {
	uint32 count;
	struct ether_addr ea[256];
} mactable;

mactable Port_mactable[4];

void ebt_broute_rule_per_port(mactable *pPort_mactable, int add, int port)
{
	int i;
	char eabuf[18];
	char mark[7];

	if (pPort_mactable->count)
	for (i = 0; i < pPort_mactable->count && i < 256; i++) {
		ether_etoa((void *)&pPort_mactable->ea[i], eabuf);
		sprintf(mark, "0x000%x", port*2+1);
		if (debug) dbg("%s dst mac %s mark %s\n", add ? "add" : "del", eabuf, mark);
		eval("ebtables", "-t", "broute", add ? "-A" : "-D", "BROUTING", "-d", eabuf, "-j", "mark", "--mark-or", mark);
		eval("ebtables", "-t", "filter", add ? "-A" : "-D", "OUTPUT", "-d", eabuf, "-j", "mark", "--mark-or", mark);
	}
}

int mxl_read_mactable(int port, mactable *m)
{
	GSW_Device_t *gsw_dev;
	GSW_return_t ret;
	GSW_MAC_tableRead_t MAC_tableRead = { 0 };
	int count = 0;
	int i;
	char eabuf[18];

	if ((port < 0))
		return 0;

	memset(&MAC_tableRead, 0, sizeof(GSW_MAC_tableRead_t));
	MAC_tableRead.bInitial = 1;

	api_gsw_get_links("");
	gsw_dev = gsw_get_struc(0, 0);

	for (;;) {
		ret = GSW_MAC_TableEntryRead(gsw_dev, &MAC_tableRead);
		if (ret < 0)
			return ret;

		if (MAC_tableRead.bLast == 1)
			break;

		if (checkValidMAC_Address(MAC_tableRead.nMAC)) {
			if ((MAC_tableRead.nAgeTimer == 0) && (MAC_tableRead.bStaticEntry == 0))
				continue;
		}

		if (MAC_tableRead.nPortId == (port + 1)) {
			memcpy(&m->ea[count], &MAC_tableRead.nMAC, sizeof(struct ether_addr));
			count++;
		}

		memset(&MAC_tableRead, 0x0, sizeof(GSW_MAC_tableRead_t));
	}

	if (count)
		m->count = count;

	if (debug) dbg("mactable of port %d\n", port);
	for (i = 0; i < count; i++) {
		if (debug) dbg("%d %s\n", i, ether_etoa((void *) &m->ea[i], eabuf));
	}

	return 0;
}

int check_mactable()
{
	int ret = 0;
	int fd;
	int port;
	int idx;
	mactable Port_mactable_local;
	int lock = file_lock("mxlmonitor");
	int change = 0;

	fd = open("/dev/mxlswitch", O_RDONLY);
	if (fd < 0)
	{
		file_unlock(lock);
		return -1;
	}
	else
	{
#ifdef GSBE18000
		for (port = 0; port <= PORT_MAX; port++) {
			if ((port == 3) || (port > 4)) continue;
#else
		for (port = 1; port <= PORT_MAX; port++) {
#endif
			memset(&Port_mactable_local, 0, sizeof(mactable));
			mxl_read_mactable(port, &Port_mactable_local);
			idx = port;
#ifdef RTBE82M
			idx--;
#endif
#ifdef GSBE18000
			if (port == 4) idx = 3;
#endif
			if (memcmp(&Port_mactable_local, &Port_mactable[idx], sizeof(mactable))) {
				change++;
				ebt_broute_rule_per_port(&Port_mactable[idx], 0, idx);
				ebt_broute_rule_per_port(&Port_mactable_local, 1, idx);
				memcpy(&Port_mactable[idx], &Port_mactable_local, sizeof(mactable));
			}
		}
	}

	close(fd);
	if (change) eval("fc", "flush", "--if", "eth1");
	file_unlock(lock);
	return ret;
}

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void mxlmonitor_exit(int sig)
{
	int port;

	alarmtimer(0, 0);

	for (port = 0; port < 4; port++)
		ebt_broute_rule_per_port(&Port_mactable[port], 0, port);

	remove("/var/run/mxlmonitor.pid");
	exit(0);
}

static void mxlmonitor_init()
{
	int port;

	for (port = 0; port < 4; port++)
		memset(&Port_mactable[port], 0, sizeof(mactable));
}

static void mxlmonitor_alarmtimer()
{
	check_mactable();

	alarmtimer(NORMAL_PERIOD, 0);
}

int
mxlmonitor_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;

	if (nvram_get_int("stop_mxlmonitor"))
		return 0;

	/* write pid */
	if ((fp = fopen("/var/run/mxlmonitor.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	debug = nvram_get_int("mxlmonitor_debug");

#if defined(RTBE82M) || defined(GSBE18000)
	system("ethswctl -c pause -p 5 -v 2");
	if (!is_router_mode() && !re_mode()) {
		system("brctl delif br0 eth0");
		system("brctl addif br0 eth0");
	}
#endif

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGKILL);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGALRM, mxlmonitor_alarmtimer);
	signal(SIGTERM, mxlmonitor_exit);
	signal(SIGKILL, mxlmonitor_exit);

	mxlmonitor_init();
	mxlmonitor_alarmtimer();

	/* Most of time it goes to sleep */
	while (1)
	{
		pause();
	}

	return 0;
}
