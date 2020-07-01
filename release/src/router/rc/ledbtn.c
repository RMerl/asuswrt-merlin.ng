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
 * Copyright 2020, ASUSTeK Inc.
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
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <shared.h>
#include <shutils.h>
#include <rc.h>

#define LEDG_WAIT	20

static struct itimerval itv;
static int btn_led_pressed = 0;
static int btn_led_count = 0;
static int LED_status_old = -1;
static int LED_status = -1;
static int LED_status_changed = 0;
static int LED_status_first = 1;
static int LED_status_on = -1;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void ledbtn_exit(int sig)
{
	alarmtimer(0, 0);

	remove("/var/run/ledbtn.pid");
	exit(0);
}

static void ledbtn_init()
{
}

static void ledbtn_alarmtimer()
{
	alarmtimer(0, 100 * 1000); 	/* 0.1 second */
}

static void ledbtn(int sig)
{
	LED_status = 0;
	int val = button_pressed(BTN_LED);
	if (val) {
		if (!btn_led_pressed)
		{
			btn_led_pressed = 1;
			btn_led_count = 0;

			int ledg_scheme = nvram_get_int("ledg_scheme");

			if ((ledg_scheme >= LEDG_SCHEME_BLINKING) || (ledg_scheme == LEDG_SCHEME_OFF)) {
				if (ledg_scheme == LEDG_SCHEME_OFF)
					LED_status = 1;
				ledg_scheme = LEDG_SCHEME_WATER_FLOW;
			} else {
				ledg_scheme = (ledg_scheme + 1) % (LEDG_SCHEME_MAX - 2);
				if (ledg_scheme == LEDG_SCHEME_OFF)
					ledg_scheme = LEDG_SCHEME_GRADIENT;
			}

			nvram_set_int("ledg_scheme", ledg_scheme);
			nvram_commit();

			dbg("switch effect\n");
			kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);
		}
		else if (++btn_led_count > LEDG_WAIT)
		{
			dbg("You can release LED button now!\n");
			btn_led_pressed = 2;
		}
	} else {
		if (btn_led_pressed == 2)
		{
			nvram_set_int("ledg_scheme", LEDG_SCHEME_OFF);
			nvram_commit();

			dbg("turn off cled\n");
			kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);

			LED_status = 1;
		}

		btn_led_count = 0;
		btn_led_pressed = 0;
	}

	if (!nvram_get_int("AllLED") && LED_status_first)
	{
		LED_status_first = 0;
		LED_status_changed = 1;
		LED_status_on = 0;
	}
	else if (LED_status &&
	    (LED_status != LED_status_old))
	{
		LED_status_changed = 1;
		if (LED_status_first)
		{
			LED_status_first = 0;
			LED_status_on = 0;
		}
		else
			LED_status_on = 1 - LED_status_on;
	}
	else
		LED_status_changed = 0;

	if (LED_status_changed)
	{
		TRACE_PT("button BTN_LED pressed\n");
		if (LED_status && (LED_status != LED_status_old)) {
			if (LED_status_on)
				nvram_set_int("AllLED", 1);
			else
				nvram_set_int("AllLED", 0);
			nvram_commit();
		}

		if (LED_status_on)
			setAllLedNormal();
		else
			setAllLedOff();
	}
}

int
ledbtn_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;

	/* write pid */
	if ((fp = fopen("/var/run/ledbtn.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGALRM, ledbtn);
	signal(SIGTERM, ledbtn_exit);

	ledbtn_init();
	ledbtn_alarmtimer();

	/* Most of time it goes to sleep */
	while (1)
	{
		pause();
	}

	return 0;
}
