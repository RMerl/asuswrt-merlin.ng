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
 * Copyright 2014, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include <shared.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ary) (sizeof(ary) / sizeof((ary)[0]))
#endif

#define SYS_PWMCHIP	"/sys/class/pwm/pwmchip0"

#if defined(RTCONFIG_PWMX1_GPIOX3_RGBLED)
#if defined(RTAX59U)
#define PWMDEV_NO	(1)
#else
#error Define PWM device number!
#endif
#endif

#if defined(RTAX59U) || defined(PRTAX57_GO)
#define PWM_LOW_ACTIVE	"1"
#else
#define PWM_LOW_ACTIVE	"0"
#endif

struct pwm_conf {
	char type[16];
	char config[16];
};

struct gpio_mapping {
	unsigned int bitmask;
	char gpio_nv[16];
};

struct led_mapping {
	unsigned int bitmask;
	unsigned int led_id;
};

static void set_pwm(int pidx, unsigned int color)
{
	char pwmdev[sizeof(SYS_PWMCHIP"/pwmXXX")];
	int pwmX = -1, i = 0;
#if defined(RTCONFIG_PWMX1_GPIOX3_RGBLED)
	struct pwm_conf p[] = {
		{		    "0",       "0-0-0"},	/* on */
		{		    "3", "0xf-250-250"},	/* off:0.5s, on:0.5s */
		{		    "3", "0xf-125-375"},	/* off:0.25s, on:0.75s */
		{		    "3",   "0xf-63-63"},	/* off:0.125s, on:0.125s */
		{"1-128-"PWM_LOW_ACTIVE, "0x3-625-625"}		/* breathing */
	};
	struct gpio_mapping g[] = {
		{RGBLED_WLED, "xxx"}				/* fake */
	};
#elif defined(RTCONFIG_PWMX3_RGBLED)
	struct pwm_conf p[] = {
#if defined(RTCONFIG_SOC_MT7981)
		{    "2-128-"PWM_LOW_ACTIVE, "0x3-192-192"},	/* on */
		{"3-128,500-"PWM_LOW_ACTIVE, "0x3-195-195"},	/* off:0.5s, on:0.5s */
		{"3-128,750-"PWM_LOW_ACTIVE, "0x3-195-195"},	/* off:0.25s, on:0.75s */
		{"3-128,500-"PWM_LOW_ACTIVE,   "0x3-49-49"},	/* off:0.125s, on:0.125s */
		{    "1-128-"PWM_LOW_ACTIVE, "0x3-240-240"}	/* breathing */
#else
		{    "2-128-"PWM_LOW_ACTIVE, "0x3-500-500"},	/* on */
		{"3-128,500-"PWM_LOW_ACTIVE, "0x3-508-508"},	/* off:0.5s, on:0.5s */
		{"3-128,750-"PWM_LOW_ACTIVE, "0x3-508-508"},	/* off:0.25s, on:0.75s */
		{"3-128,500-"PWM_LOW_ACTIVE, "0x3-127-127"},	/* off:0.125s, on:0.125s */
		{    "1-128-"PWM_LOW_ACTIVE, "0x3-625-625"}	/* breathing */

#endif
	};
	struct gpio_mapping g[] = {
		{RGBLED_RLED, "led_red_gpio"},
		{RGBLED_GLED, "led_green_gpio"},
		{RGBLED_BLED, "led_blue_gpio"}
	};
	int rgb[3] = { 0 };

	/* for color mixing */
	if (nvram_invmatch("ledg_scheme", "")) {
		char nv[16], buf[64];
		char *ptr = buf, *str = NULL;
		int ledg_scheme = nvram_get_int("ledg_scheme");

		/* parse nvram ledg_rgbXXX */
		snprintf(nv, sizeof(nv), "ledg_rgb%d", ledg_scheme);
		strcpy(buf, nvram_safe_get(nv));
		while ((str = strsep(&ptr, ","))) {
			rgb[i] = atoi(str);
			if (++i > ARRAY_SIZE(rgb))
				break;
		}
	}
#endif

	for (i = 0; i < ARRAY_SIZE(g); i++) {
#if defined(RTCONFIG_PWMX1_GPIOX3_RGBLED)
		pwmX = PWMDEV_NO;
#elif defined(RTCONFIG_PWMX3_RGBLED)
		pwmX = (nvram_get_int(g[i].gpio_nv) & 0xff) - GPIO_PWM_DEFSHIFT;
		if (pwmX < 0)
			continue;
#endif

		snprintf(pwmdev, sizeof(pwmdev), "%s/pwm%d", SYS_PWMCHIP, pwmX);
		if (!d_exists(pwmdev))
			doSystem("echo %d > %s/export", pwmX, SYS_PWMCHIP);
		doSystem("echo 0 > %s/mm_enable", pwmdev);
#if defined(RTCONFIG_PWMX1_GPIOX3_RGBLED)
		if (!strcmp(p[pidx].type, "0"))
#elif defined(RTCONFIG_PWMX3_RGBLED)
		if (!(g[i].bitmask & color) && strstr(PWM_LOW_ACTIVE, "0"))  /* for high-active, turn off LED */
#endif
			continue;

#if defined(RTCONFIG_PWMX3_RGBLED)
		/* for color mixing */
		if (rgb[i] > 0 && rgb[i] < 128) {
			char buf[16];
			char *sptr = p[pidx].type;
			char *dptr = buf;

			/* re-generate p[pidx].type content */
			memset(buf, 0, sizeof(buf));
			strncpy(dptr, sptr, 2);
			dptr += 2;
			sptr += 5;
			dptr += sprintf(dptr, "%d", rgb[i]);
			if (*sptr != '\0')
				strncpy(dptr, sptr, strlen(sptr));
			memcpy(p[pidx].type, buf, sizeof(p[pidx].type));
		}
#endif

#if defined(RTCONFIG_PWMX3_RGBLED)
		if (!(g[i].bitmask & color)) {
			/* for low-active, turn off LED */
			doSystem("echo 2-0-1 > %s/mm_type", pwmdev);
			doSystem("echo 0x3-500-500 > %s/mm_config", pwmdev);
		}
		else
#endif
		{
			doSystem("echo %s > %s/mm_type", p[pidx].type, pwmdev);
			doSystem("echo %s > %s/mm_config", p[pidx].config, pwmdev);
		}
		doSystem("echo 1 > %s/mm_enable", pwmdev);
	}
}

static void set_color(unsigned int color)
{
#if defined(RTCONFIG_PWMX1_GPIOX3_RGBLED)
	struct led_mapping l[] = {
		{RGBLED_BLED, LED_BLUE},
		{RGBLED_GLED, LED_GREEN},
		{RGBLED_RLED, LED_RED},
		{RGBLED_WLED, LED_WHITE}
	};
#elif defined(RTCONFIG_PWMX3_RGBLED)
	struct led_mapping l[] = {
		{RGBLED_WLED, LED_WHITE}
	};
#endif
	int i;

	for (i = 0; i < ARRAY_SIZE(l); i++) {
		if (l[i].bitmask & color)
			led_control(l[i].led_id, LED_ON);
		else
			led_control(l[i].led_id, LED_OFF);
	}
}

void set_rgbled(unsigned int mode)
{
	unsigned int cmask = RGBLED_COLOR_MESK, bmask = RGBLED_BLINK_MESK;
	unsigned int c = mode & cmask;
	unsigned int b = mode & bmask;
	int pidx = 0;

	if ((c == RGBLED_CONNECTED || c == RGBLED_ETH_BACKHAUL)
	  && b == 0
	  && nvram_match("AllLED", "0")
	)
		c = 0;

	switch (b) {
	case RGBLED_ATE_MODE:
		pidx = 0;
		break;
	case RGBLED_SBLINK:
		pidx = 1;
		break;
	case RGBLED_3ON1OFF:
		pidx = 2;
		break;
	case RGBLED_FBLINK:
		pidx = 3;
		break;
	case RGBLED_BREATHING:
		pidx = 4;
		break;
	default:
		;
	}

	if (b != RGBLED_ATE_MODE)
		set_color(c);
	set_pwm(pidx, c);
}
