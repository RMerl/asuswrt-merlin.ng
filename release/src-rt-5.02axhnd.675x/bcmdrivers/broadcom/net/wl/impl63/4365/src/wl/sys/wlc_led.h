/*
 * Required functions exported by the wlc_led.c
 * to common (os-independent) driver code.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_led.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_led_h_
#define _wlc_led_h_

#ifdef WLLED

#define	LED_BLINK_TIME		10	/* 10ms wlc_led_blink_timer() period */
/* PMU override bit starting point for the GPIO line controlling the LED */
#define PMU_CCA1_OVERRIDE_BIT_GPIO0	16

struct bmac_led {
	uint	pin;		/* gpio pin# == led index# */
	bool	pin_ledbh;	/* gpio pin is defined by nvram ledbh */
	bool	activehi;	/* led behavior of this pin */
	uint32	msec_on;	/* milliseconds or timer ticks on */
	uint32	msec_off;	/* milliseconds or timer ticks off */
#if OSL_SYSUPTIME_SUPPORT
	uint32	timestamp;	/* OSL_SYSUPTIME of the last action */
	bool	next_state;	/*  transitioning from on->off or off->on? */
	bool 	restart;	/* start the LED blinking from the beginning of ON cycle */
#else
	int32	blinkmsec;	/* total number of on/off ticks */
#endif // endif
};

struct bmac_led_info {
	void		*wlc_hw;
	struct bmac_led	led[WL_LED_NUMGPIO];	/* led fanciness */
	uint32		gpioout_cache;		/* cache the gpio pin values */
	uint32		gpiomask_cache;		/* cache the gpio mask */
	uint		led_blink_time;		/* timer blink interval */
	struct wl_timer *led_blink_timer;	/* led_blink_time duration (ms) led blink timer */
	bool 		blink_start;
	bool		blink_adjust;
};

#define WLACTINCR(a)		((a)++) /* Increment by 1 */
extern led_info_t *wlc_led_attach(wlc_info_t *wlc);
extern int wlc_led_detach(led_info_t *ledh);
extern void wlc_led_init(led_info_t *ledh);
extern void wlc_led_deinit(led_info_t *ledh);
extern int wlc_led_event(led_info_t *ledh);
extern int wlc_led_set(led_info_t *ledh, wl_led_info_t *ed);
extern void wlc_led_radioset(led_info_t *ledh, bool led_state);
extern void wlc_led_activityset(led_info_t *ledh, bool led_state);
extern void wlc_led_up(wlc_info_t *wlc);
extern uint wlc_led_down(wlc_info_t *wlc);
extern uint wlc_led_start_activity_timer(led_info_t *ledh);
extern uint wlc_led_stop_activity_timer(led_info_t *ledh);

#else
#define WLACTINCR(a)
#define wlc_led_attach(a, b)				(led_info_t *)0x0dadbeef
static INLINE int wlc_led_detach(led_info_t *ledh)	{ return 0; }
#define wlc_led_init(a)					do {} while (0)
#define wlc_led_deinit(a)				do {} while (0)
#define wlc_led_event(a)				do {} while (0)
#define wlc_led_set(a, b)				do {} while (0)
#define wlc_led_radioset(a, b)				do {} while (0)
#define wlc_led_activityset(a, b)			do {} while (0)
#define wlc_led_up(a)					do {} while (0)
#define wlc_led_down(a)					0
#endif /* WLLED */

#endif	/* _wlc_led_h_ */
