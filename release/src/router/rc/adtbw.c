/*
 * Copyright 2017, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

/* Adaptive Bandwidth Control */

#include <rc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <wlioctl.h>
#include <wlutils.h>
#include "adtbw.h"

static struct itimerval itv;
static int adtbw_state = ADTBW_STATE_LEAVE;
static int adtbw_hit = 0;
static int adtbw_freeze = 0;

static void alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void adtbw_exit(int sig)
{

	alarmtimer(0, 0);
        dbG("adtbw exit...\n");
        remove("/var/run/adtbw.pid");
        exit(0);
}

static void adtbw_monitor(int sig)
{
	if(!adtbw_config()) {
		if(adtbw_hit) adtbw_hit = 0;
		return;
	}

	if(adtbw_freeze > 0) {
		adtbw_freeze -= ADTBW_TIMER;
		return;
	}

	if(adtbw_state == ADTBW_STATE_LEAVE && adtbw_enter())
	{
			adtbw_hit++;
			if(adtbw_hit >= ADTBW_HITCOUNT && adtbw_active()) {
				adtbw_state = ADTBW_STATE_ENTER;
				adtbw_hit = 0;
				adtbw_freeze = ADTBW_FREEZE_TIME;
			}
	}
	else if(adtbw_state == ADTBW_STATE_ENTER && adtbw_leave())
	{
			adtbw_hit++;
			if(adtbw_hit >= ADTBW_HITCOUNT && adtbw_restore()) {
				adtbw_state = ADTBW_STATE_LEAVE;
				adtbw_hit = 0;
				adtbw_freeze = ADTBW_FREEZE_TIME;
			}
	}
	else
	{
		adtbw_hit = 0;
	}
	
	return;	
}

void stop_adtbw()
{
	killall_tk("adtbw");
}

void start_adtbw()
{
	char *adtbw_argv[] = {"adtbw", NULL};
	pid_t pid;

	_eval(adtbw_argv, NULL, 0, &pid);
}


int adtbw_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;

	if (!adtbw_enable())
		return 0;

	if ((fp = fopen("/var/run/adtbw.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

        dbG("adtbw start...\n");

        /* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGALRM, adtbw_monitor);
	signal(SIGTERM, adtbw_exit);

	alarmtimer(ADTBW_TIMER, 0);

	while (1)
	{
		pause();
	}

	return 0;
}

