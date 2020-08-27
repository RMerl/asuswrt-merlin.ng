/*
 * This program is sm_free software; you can redistribute it and/or
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
 * Copyright 2012, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <shared.h>
#include <json.h>
#include <pthread.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <rc.h>
#ifdef RTCONFIG_CFGSYNC
#include <cfg_string.h>
#endif
#include "rmd.h"

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_SW_HW_AUTH
#include <auth_common.h>
#define APP_ID  "33716237"
#define APP_KEY "g2hkhuig238789ajkhc"
#endif
#endif

static int thread_term = 0;

static struct itimerval itv;
static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void rmd_watchdog(int sig)
{
	//TODO
}

static void rmd_exit(int sig)
{
	thread_term = 1;
	alarmtimer(0, 0);

	RMD_INFO("rmd exit...\n");
	remove("/var/run/rmd.pid");
	exit(0);
}

static void rmd_ipc_receive(int sockfd)
{
	int length = 0;
	char buf[2048];
	json_object *rootObj = NULL, *reNumObj = NULL, *sta5gObj = NULL;

	memset(buf, 0, sizeof(buf));
	if ((length = read(sockfd, buf, sizeof(buf))) <= 0)
	{
		RMD_DBG("ipc read socket error!\n");
		return;
	}
  
	rootObj = json_tokener_parse(buf);
	RMD_DBG("IPC Receive: %s <<< RCV EVENT >>>\n", buf);

	json_object_object_get_ex(rootObj, CFG_STR_STA5G, &sta5gObj);
	json_object_object_get_ex(rootObj, CFG_STR_RE_NUMBER, &reNumObj);

	//TODO
	RMD_DBG("re number(%d), 5g sta mac(%s)\n",
	json_object_get_int(reNumObj), json_object_get_string(sta5gObj));

	json_object_put(rootObj);
}

static int rmd_start_ipc_socket(void)
{
	struct sockaddr_un addr;
	int sockfd, newsockfd;

	if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		RMD_INFO("ipc create socket error!\n");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, RMD_IPC_SOCKET_PATH, sizeof(addr.sun_path)-1);

	unlink(RMD_IPC_SOCKET_PATH);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		RMD_INFO("ipc bind socket error!\n");
		exit(-1);
	}

	if (listen(sockfd, RMD_IPC_MAX_CONNECTION) == -1) {
		RMD_INFO("ipc listen socket error!\n");
		exit(-1);
	}

	while (!thread_term) {
		RMD_INFO("ipc accept socket...\n");
		if ( (newsockfd = accept(sockfd, NULL, NULL)) == -1) {
			RMD_INFO("ipc accept socket error!\n");
			continue;
		}

		rmd_ipc_receive(newsockfd);
		close(newsockfd);
	}

	return 0;
}

void rmd_ipc_socket_thread(void)
{
	pthread_t thread;
	pthread_attr_t attr;

	RMD_DBG("Start ipc socket thread.\n");

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread,NULL,(void *)&rmd_start_ipc_socket,NULL);
	pthread_attr_destroy(&attr);
}

void rmd_init()
{
	rmd_ipc_socket_thread();
}

int rmd_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;
	char *val;

#if defined(RTCONFIG_SW_HW_AUTH) && defined(RTCONFIG_AMAS)
	time_t timestamp = time(NULL);
	char in_buf[48];
	char out_buf[65];
	char hw_out_buf[65];
	char *hw_auth_code = NULL;

	// initial
	memset(in_buf, 0, sizeof(in_buf));
	memset(out_buf, 0, sizeof(out_buf));
	memset(hw_out_buf, 0, sizeof(hw_out_buf));

	// use timestamp + APP_KEY to get auth_code
	snprintf(in_buf, sizeof(in_buf)-1, "%ld|%s", timestamp, APP_KEY);

	hw_auth_code = hw_auth_check(APP_ID, get_auth_code(in_buf, out_buf, sizeof(out_buf)), timestamp, hw_out_buf, sizeof(hw_out_buf));

	// use timestamp + APP_KEY + APP_ID to get auth_code
	snprintf(in_buf, sizeof(in_buf)-1, "%ld|%s|%s", timestamp, APP_KEY, APP_ID);

	// if check fail, return
	if (strcmp(hw_auth_code, get_auth_code(in_buf, out_buf, sizeof(out_buf)))) {
		return 0;
	}
#endif

	/* write pid */
	if ((fp = fopen("/var/run/rmd.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	rmd_init();

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGALRM, rmd_watchdog);
	signal(SIGTERM, rmd_exit);

	alarmtimer(NORMAL_PERIOD, 0);

	/* Most of time it goes to sleep */
	while (1)
	{
		val = nvram_safe_get("rmd_msglevel");
		if (strcmp(val, ""))
			msglevel = strtoul(val, NULL, 0);

		pause();
	}

	return 0;
}
