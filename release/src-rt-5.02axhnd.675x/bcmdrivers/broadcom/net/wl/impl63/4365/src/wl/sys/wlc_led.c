/*
 * Customer-defined whiz band LED maintenance for Broadcom 802.11 Networking Driver.
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
 * $Id: wlc_led.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlDriverLED]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmdevs.h>
#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>

#include <wl_export.h>
#include <wlc_led.h>
#include <wlc_bmac.h>

#define LED_TIME		200	/* 200ms wlc_led_timer() period */
#define LED_NUMDEFAULT		3	/* # of default LEDs */

/* LED blink modes */
#define NUM_BLINK_RATES	4
#define LED_BLINKSLOW	0
#define LED_BLINKMED	1
#define LED_BLINKFAST	2
#define LED_BLINKCUSTOM	3

/* Default blink timings in ms for each mode */
#define LED_SLOW_ON	500
#define LED_SLOW_OFF	500
#define LED_MED_ON	80
#define LED_MED_OFF	20
#define LED_FAST_ON	40
#define LED_FAST_OFF	40
#define LED_CUSTOM_ON	200
#define LED_CUSTOM_OFF	100

/* LED timer counts for 300 sec */
#define LED_300_SEC_COUNT 300

struct led {
	uint32	pin;		/* gpio pin# == led index# */
	uint32	behavior;	/* led behavior of this pin */
	bool	activehi;	/* polarity */
	uint16	msec_on;	/* milliseconds on */
	uint16	msec_off;	/* milliseconds off */
	bool 	blink_sync; /* to synchronize blinking */
};

struct led_info {
	wlc_info_t		*wlc;
	wlc_pub_t	*pub;

	struct led	led[WL_LED_NUMGPIO];	/* led fanciness */
	uint32		gpioout_cache;		/* cache the gpio pin values */
	uint32		gpiomask_cache;		/* cache the gpio mask */
	uint16		led_on[NUM_BLINK_RATES];   /* on blink rate settings */
	uint16		led_off[NUM_BLINK_RATES]; /* Off blink rate settings */
	bool		activity;			/* recent tx/rx data frame activity */
	bool		activity_led_enab;	/* recent tx/rx data frame activity */
	struct wl_timer *led_timer;		/* periodic led timer */
	bool		led_timer_set;		/* Whether led_timer is armed or not */
	bool		led_blink_run;		/* blink timer is running */
	bool		up;			/* TRUE = led timer is running */
	uint32		timestamp;		/* timestamp for periodic blinking */
	bool		periodic_state;		/* TRUE = blinking; FALSE = OFF */
	uint32		timer_count;		/* LED timer counts */
	uint32		blink_pins;
};

enum {
	IOV_LED_BLINK_SLOW,	/* configure the led blink rates */
	IOV_LED_BLINK_MED,	/* configure the led blink rates */
	IOV_LED_BLINK_FAST,	/* configure the led blink rates */
	IOV_LEDS,			/* enable/disable leds */
	IOV_LED_BLINK_CUSTOM,	/* configure the led blink rates */
	IOV_LED_BEHAVIOR,
	IOV_LED_BLINK_SYNC
};

static const bcm_iovar_t led_iovars[] = {
	{"ledbh", IOV_LED_BEHAVIOR,
	(0), IOVT_BUFFER, sizeof(wl_led_info_t)
	},
	{"led_blinkcustom", IOV_LED_BLINK_CUSTOM,
	(0), IOVT_UINT32, 0
	},
	{"led_blinkslow", IOV_LED_BLINK_SLOW,
	(0), IOVT_UINT32, 0
	},
	{"led_blinkmed", IOV_LED_BLINK_MED,
	(0), IOVT_UINT32, 0
	},
	{"led_blinkfast", IOV_LED_BLINK_FAST,
	(0), IOVT_UINT32, 0
	},
	{"leds", IOV_LEDS,
	(0), IOVT_UINT32, 0
	},
	{"led_blink_sync", IOV_LED_BLINK_SYNC,
	(0), IOVT_BOOL, 0
	},
	{NULL, 0, 0, 0, 0 }
};

/* blink models */
static void wlc_led_blink(led_info_t *li, struct led *led, uint8 speed);
static void wlc_led_blinkoff(led_info_t *li, struct led *led);

/* led behavior function prototypes */
static void wlc_led(led_info_t *li, uint32 mask, uint32 val, bool activehi);
static void wlc_led_null(led_info_t *li, struct led *led);
static void wlc_led_on(led_info_t *li, struct led *led);
static void wlc_led_off(led_info_t *li, struct led *led);
static void wlc_led_activity(led_info_t *li, struct led *led);
static void wlc_led_a_radio(led_info_t *li, struct led *led);
static void wlc_led_b_radio(led_info_t *li, struct led *led);
static void wlc_led_bgmode(led_info_t *li, struct led *led);
static void wlc_led_wi1(led_info_t *li, struct led *led);
static void wlc_led_wi2(led_info_t *li, struct led *led);
static void wlc_led_wi3(led_info_t *li, struct led *led);
static void wlc_led_assoc(led_info_t *li, struct led *led);
static void wlc_led_assocact(led_info_t *li, struct led *led);
static void wlc_led_wi4(led_info_t *li, struct led *led);
static void wlc_led_wi5(led_info_t *li, struct led *led);
static void wlc_led_blinkslow(led_info_t *li, struct led *led);
static void wlc_led_blinkmed(led_info_t *li, struct led *led);
static void wlc_led_blinkfast(led_info_t *li, struct led *led);
static void wlc_led_blinkcustom(led_info_t *li, struct led *led);
static void wlc_led_blinkperiodic(led_info_t *li, struct led *led);
static void wlc_led_assoc_with_security(led_info_t *li, struct led *led);
static void wlc_led_wi6(led_info_t *li, struct led *led);
static void wlc_led_wi7(led_info_t *li, struct led *led);
static void wlc_led_activity_enab_upd(led_info_t *li);
static void wlc_led_timer(void *arg);

#ifdef BCMDBG
static int wlc_led_dump(led_info_t *li, struct bcmstrbuf *b);
#endif // endif

/* led iovar functionality */
static int wlc_led_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);

/* led watchdog functionality */
static void wlc_led_watchdog(void *hdl);

static INLINE void wlc_gled_rate(led_info_t *ledh, uint32 *blinkrate, uint8 speed);
static INLINE void wlc_sled_rate(led_info_t *ledh, uint32 *blinkrate, uint8 speed);

typedef void (*ledfunc_t)(led_info_t *li, struct led *led);

/* use dynamic initialization in wlc_led_attach() for some compiler */
static ledfunc_t ledfunc[WL_LED_NUMBEHAVIOR];

/* accessor function for ROMming this file */
static ledfunc_t* wlc_led_get_ledfunc(void)
{
	return ledfunc;
}

led_info_t *
BCMATTACHFN(wlc_led_attach)(wlc_info_t *wlc)
{
	led_info_t *li;
	struct led *led = NULL;
	int i;
	char name[32];
	const char *var;
	uint val;
	bool override = FALSE;
	wlc_pub_t *pub = wlc->pub;
	ledfunc_t *lf_array = wlc_led_get_ledfunc();

	WL_TRACE(("wl: wlc_led_attach\n"));

	/* init static function array */
	lf_array[0] = wlc_led_null;
	lf_array[1] = wlc_led_on;
	lf_array[2] = wlc_led_activity;
	lf_array[3] = wlc_led_on;
	lf_array[4] = wlc_led_a_radio;
	lf_array[5] = wlc_led_b_radio;
	lf_array[6] = wlc_led_bgmode;
	lf_array[7] = wlc_led_wi1;
	lf_array[8] = wlc_led_wi2;
	lf_array[9] = wlc_led_wi3;
	lf_array[10] = wlc_led_assoc;
	lf_array[11] = wlc_led_null;
	lf_array[12] = wlc_led_assocact;
	lf_array[13] = wlc_led_wi4;
	lf_array[14] = wlc_led_wi5;
	lf_array[15] = wlc_led_blinkslow;
	lf_array[16] = wlc_led_blinkmed;
	lf_array[17] = wlc_led_blinkfast;
	lf_array[18] = wlc_led_blinkcustom;
	lf_array[19] = wlc_led_blinkperiodic;
	lf_array[20] = wlc_led_assoc_with_security;
	lf_array[21] = wlc_led_off;
	lf_array[22] = wlc_led_wi6;
	lf_array[23] = wlc_led_wi7;

	if ((li = (led_info_t *)MALLOCZ(pub->osh, sizeof(led_info_t))) == NULL) {
		WL_ERROR(("wlc_led_attach: out of memory, malloced %d bytes", MALLOCED(pub->osh)));
		goto fail;
	}
	li->wlc = (void*) wlc;
	li->pub = pub;

	/* init led behaviors */
	ASSERT(LED_NUMDEFAULT <= WL_LED_NUMGPIO);

	/* 43224A0 hack - GPIO4 is forced to WL_LED_RADIO behavior */
	if (((CHIPID(li->pub->sih->chip) == BCM43224_CHIP_ID) ||
		(CHIPID(li->pub->sih->chip) == BCM43421_CHIP_ID)) &&
		CHIPREV(li->pub->sih->chiprev) == 0 &&
		&li->led[4] != NULL) {
		WL_ERROR(("wl%d: %s: 43224A0 override - setting radio on/off behavior to GPIO4, "
		          "all other GPIOs disabled\n", pub->unit, __FUNCTION__));
		li->led[4].behavior = WL_LED_RADIO;
		li->led[4].pin = 4;
		li->led[4].activehi = FALSE;
		override = TRUE;
	}

	if (!override) {
		led = &li->led[0];
		led->pin = 0;

		/* For Win8 remove WL_LED_ACTIVITY by default to save on led blink timers */
#if defined(NDIS) && (NDISVER >= 0x0630)
		led->behavior = WL_LED_OFF;
#else
		led->behavior = WL_LED_ACTIVITY;
#endif /* defined(NDIS) && (NDISVER >= 0x0630) */

		led->activehi = TRUE;

		led++;
		led->pin = 1;
		led->behavior = WL_LED_BRADIO;
		led->activehi = TRUE;

		led++;
		led->pin = 2;
		led->behavior = WL_LED_ARADIO;
		led->activehi = TRUE;

		for (i = LED_NUMDEFAULT; i < WL_LED_NUMGPIO; i ++) {
			led ++;
			led->pin = i;
			led->behavior = WL_LED_OFF;
			led->activehi = TRUE;
		}
	}
	/* hack for HP branded bcm94306mp cards */
	if (li->pub->sih->boardvendor == VENDOR_HP_COMPAQ)
		li->led[0].behavior = WL_LED_RADIO;

	/* look for led gpio/behavior nvram overrides */
	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		led = &li->led[i];

		led->blink_sync = TRUE;

		snprintf(name, sizeof(name), "ledbh%d", i);

		if ((var = getvar(pub->vars, name)) == NULL) {
			snprintf(name, sizeof(name), "wl0gpio%d", i);
			if ((var = getvar(pub->vars, name)) == NULL) {
				continue;
			}
		}

		val = bcm_strtoul(var, NULL, 0);

		/* silently ignore old card srom garbage */
		if ((val & WL_LED_BEH_MASK) >= WL_LED_NUMBEHAVIOR)
			continue;

		if (!override) {
			led->pin = i;	/* gpio pin# == led index# */
			led->behavior = val & WL_LED_BEH_MASK;
			led->activehi = (val & WL_LED_AL_MASK)? FALSE : TRUE;
		}
	}

#ifdef BCMDBG
	wlc_dump_register(pub, "led", (dump_fn_t)wlc_led_dump, (void *)li);
#endif // endif
	/* init LED default blink rates */
	li->led_on[LED_BLINKSLOW] = LED_SLOW_ON;
	li->led_on[LED_BLINKMED] = LED_MED_ON;
	li->led_on[LED_BLINKFAST] = LED_FAST_ON;
	li->led_on[LED_BLINKCUSTOM] = LED_CUSTOM_ON;
	li->led_off[LED_BLINKSLOW] = LED_SLOW_OFF;
	li->led_off[LED_BLINKMED] = LED_MED_OFF;
	li->led_off[LED_BLINKFAST] = LED_FAST_OFF;
	li->led_off[LED_BLINKCUSTOM] = LED_CUSTOM_OFF;

	/* register module */
	if (wlc_module_register(pub, led_iovars, "led", li, wlc_led_doiovar,
	                        wlc_led_watchdog, NULL, NULL)) {
		WL_ERROR(("wl%d: led wlc_module_register() failed\n", pub->unit));
		goto fail;
	}
	if (!(li->led_timer = wl_init_timer(wlc->wl, wlc_led_timer, wlc, "led"))) {
		WL_ERROR(("wl%d: wlc_led_attach: wl_init_timer for led_timer failed\n",
			wlc->pub->unit));
		goto fail;
	}

	wlc_led_activity_enab_upd(li);

	return li;

fail:
	if (li) {
		MFREE(li->pub->osh, li, sizeof(led_info_t));
	}
	return NULL;
}

int
BCMATTACHFN(wlc_led_detach)(led_info_t *li)
{
	wlc_info_t *wlc;
	int callbacks = 0;

	WL_TRACE(("wl: %s: li = %p\n", __FUNCTION__, li));

	if (li == NULL)
		return callbacks;

	wlc = (wlc_info_t*) li->wlc;
	wlc_module_unregister(li->pub, "led", li);

	if (li->led_timer) {
		if (!wl_del_timer(wlc->wl, li->led_timer))
			callbacks++;
		wl_free_timer(wlc->wl, li->led_timer);
		li->led_timer = NULL;
	}

	MFREE(li->pub->osh, li, sizeof(led_info_t));

	return callbacks;
}

void
BCMINITFN(wlc_led_init)(led_info_t *li)
{
	uint32 mask = 0;
	int i;
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;

	li->gpiomask_cache = 0;
	li->gpioout_cache = 0;

	/* build gpio mask and turn LED off  */
	for (i = 0; i < WL_LED_NUMGPIO; i++)
		if ((li->led[i].behavior) && (li->led[i].behavior != WL_LED_INACTIVE)) {
			mask |= (1 << li->led[i].pin);
		}

	li->gpiomask_cache = mask;

	/* Initialize mask for BMAC LED GPIOs */
	wlc_bmac_led_hw_mask_init(wlc->hw, mask);
}

void
BCMUNINITFN(wlc_led_deinit)(led_info_t *li)
{
	wlc_info_t *wlc = (wlc_info_t*) li->wlc;

	wlc_bmac_led_hw_deinit(wlc->hw, li->gpiomask_cache);
}

/* led ioctl */
int
wlc_led_set(led_info_t *li, wl_led_info_t *ed)
{
	if (ed->index >= WL_LED_NUMGPIO)
		return -1;

	li->led[ed->index].behavior = ed->behavior;
	li->led[ed->index].activehi = ed->activehi;

	wlc_bmac_led_set(((wlc_info_t*)(li->wlc))->hw, ed->index, ed->activehi);

	/* Update if any of ACTIVITY LED was set */
	wlc_led_activity_enab_upd(li);

	return 0;
}

/* this update can be called regardless driver is "up" or "down" */
int
wlc_led_event(led_info_t *li)
{
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;
	struct led *led;
	int i;
	int callbacks = 0;
	bool blink = FALSE;
	ledfunc_t *lf_array = wlc_led_get_ledfunc();

	/* call each pin behavior function */
	for (i = 0; i < WL_LED_NUMGPIO; i++)
		if ((led = &li->led[i])->behavior)
			lf_array[led->behavior](li, led);

	/* Update the Generic state of Blink timer is being used..or should be used... */
	if (!li->up && !wlc_down_for_mpc(wlc))
		blink = FALSE;
	else {
		for (i = 0; i < WL_LED_NUMGPIO; i++) {
			if (li->led[i].msec_on || li->led[i].msec_off) {
				blink = TRUE;
				break;
			}
		}
	}
	if (blink && !li->led_blink_run) {
		wlc_bmac_led_blink_event(wlc->hw, TRUE);
		li->led_blink_run = TRUE;
	} else if (!blink && li->led_blink_run) {
		callbacks += wlc_bmac_led_blink_event(wlc->hw, FALSE);
		li->led_blink_run = FALSE;
	}
	return callbacks;
}

static void
wlc_led_watchdog(void *hdl)
{
	led_info_t *li = (led_info_t *)hdl;

	/* decrease timer_count if it > 0 */
	if (li->timer_count)
		li->timer_count--;

	wlc_led_event(li);
}

static void
wlc_led_timer(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	led_info_t *li = wlc->ledh;

	if (!wlc->pub->up) {
		return;
	}

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

	wlc_led_event(li);

	if (!li->activity)
		wlc_led_stop_activity_timer(li);

	li->activity = FALSE;
}

#ifdef BCMDBG
static int
wlc_led_dump(led_info_t *li, struct bcmstrbuf *b)
{
	uint i;

	bcm_bprintf(b, "led:  ");
	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		bcm_bprintf(b, "%d%s=%d ", li->led[i].pin,
			(li->led[i].activehi ? "" : "_l"), li->led[i].behavior);
	}
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "led blink_sync:  ");
	for (i = 0; i < WL_LED_NUMGPIO; i++)
		bcm_bprintf(b, "%d=%d ", li->led[i].pin, li->led[i].blink_sync);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "ledblinkslowon %d ledblinkslowoff %d\n",
		li->led_on[LED_BLINKSLOW], li->led_off[LED_BLINKSLOW]);
	bcm_bprintf(b, "ledblinkmedon %d ledblinkmedoff %d\n",
		li->led_on[LED_BLINKMED], li->led_off[LED_BLINKMED]);
	bcm_bprintf(b, "ledblinkfaston %d ledblinkfastoff %d\n",
		li->led_on[LED_BLINKFAST], li->led_off[LED_BLINKFAST]);
	bcm_bprintf(b, "ledblinkcustomon %d ledblinkcustomoff %d\n",
		li->led_on[LED_BLINKCUSTOM], li->led_off[LED_BLINKCUSTOM]);
	bcm_bprintf(b, "blink_pins 0x%x\n", li->blink_pins);

	return 0;
}

#endif /* BCMDBG */

static void
wlc_led_blink(led_info_t *li, struct led *led, uint8 speed)
{
	wlc_info_t *wlc = (wlc_info_t*)li->wlc;

	if (led->blink_sync)
		li->blink_pins |= (0x1 << led->pin);

	/* continue only if there is a change in the On/Off Blink Rate */
	if (led->msec_on == li->led_on[speed] && led->msec_off == li->led_off[speed])
		return;

	led->msec_on = li->led_on[speed];
	led->msec_off = li->led_off[speed];

	wlc_bmac_led_blink(wlc->hw, led->pin, led->msec_on, led->msec_off);

	if (li->blink_pins)
		wlc_bmac_blink_sync(wlc->hw, li->blink_pins);

}

static INLINE void
wlc_gled_rate(led_info_t *li, uint32 *blinkrate, uint8 speed)
{
	*blinkrate = 0;
	*blinkrate |= ((li->led_on[speed] & 0xffff) <<16);
	*blinkrate |= (li->led_off[speed] & 0xffff);
}

static INLINE void
wlc_sled_rate(led_info_t *li, uint32 *blinkrate, uint8 speed)
{
	li->led_on[speed] = (*blinkrate >>16) & 0xffff;
	li->led_off[speed] = *blinkrate & 0xffff;
}

static void
wlc_led_blinkoff(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc = (wlc_info_t*)li->wlc;

	if (led->msec_on || led->msec_off) {
		led->msec_on = led->msec_off = 0;
		if (led->blink_sync) {
			if (li->blink_pins & (0x1 << led->pin))
				li->blink_pins &= ~(0x1 << led->pin);
		}

		wlc_bmac_led_blink(wlc->hw, led->pin, 0, 0);
	}
}

/* turn gpio bits on or off */
static void
wlc_led(led_info_t *li, uint32 mask, uint32 val, bool activehi)
{
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;

	wlc_bmac_led(wlc->hw, mask, val, activehi);
}

static void
wlc_led_null(led_info_t *li, struct led *led)
{
}

static void
wlc_led_on(led_info_t *li, struct led *led)
{
	uint mask, val;
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;

	mask = 1 << led->pin;
	val = mask;

	/* Handle both on and Off cases */
	if (!li->up && !wlc_down_for_mpc(wlc))
		val = 0;

	wlc_led(li, mask, val, led->activehi);
}

static void
wlc_led_off(led_info_t *li, struct led *led)
{
	wlc_led(li, (1 << led->pin), 0, led->activehi);
}

static void
wlc_led_activity(led_info_t *li, struct led *led)
{
	if (!li->up) {
		/* make sure li->activity is clear since wlc_led_timer may be stopped in wlc_down */
		li->activity = FALSE;
	}

	if (li->activity)
		wlc_led_blink(li, led, LED_BLINKMED);
	else {
		wlc_led_blinkoff(li, led);
		wlc_led_off(li, led);
	}
}

static void
wlc_led_a_radio(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;
	uint bits;
	bool a_radio;

	a_radio = FALSE;
	if ((NBANDS_PUB(li->pub) == 1) || (wlc->bandlocked == TRUE)) {
		if (BAND_5G(wlc->band->bandtype))
			a_radio = TRUE;
	} else {
		a_radio = TRUE;
	}

	bits = a_radio? (1 << led->pin) : 0;

	/* when down, but not due to MPC only, turn off radio LED */
	if (!li->up && !wlc_down_for_mpc(wlc))
		bits = 0;

	wlc_led(li, (1 << led->pin), bits, led->activehi);
}

static void
wlc_led_b_radio(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;
	uint bits;
	bool b_radio;

	b_radio = FALSE;
	if ((NBANDS_PUB(li->pub) == 1) || (wlc->bandlocked == TRUE)) {
		if (BAND_2G(wlc->band->bandtype))
			b_radio = TRUE;
	} else {
		b_radio = TRUE;
	}

	bits = b_radio? (1 << led->pin) : 0;

	/* when down, but not due to MPC only, turn off radio LED */
	if (!li->up && !wlc_down_for_mpc(wlc))
		bits = 0;

	wlc_led(li, (1 << led->pin), bits, led->activehi);
}

static void
wlc_led_bgmode(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
	uint bits;

	if (AP_ENAB(li->pub))
		bits = wlc->band->gmode ? (1 << led->pin) : 0;
	else
		bits = (cfg->associated && cfg->BSS && wlc->band->gmode &&
			wlc_rateset_isofdm(cfg->current_bss->rateset.count,
		                           cfg->current_bss->rateset.rates)) ?
			(1 << led->pin) : 0;

	wlc_led(li, bits, bits, led->activehi);
}

static void
wlc_led_wi1(led_info_t *li, struct led *led)
{
	/* blink if activity, otherwise on */
	if (li->activity)
		wlc_led_blink(li, led, LED_BLINKMED);
	else {
		wlc_led_blinkoff(li, led);
		wlc_led_on(li, led);
	}
}

/* 11g behavior */
static void
wlc_led_wi2(led_info_t *li, struct led *led)
{
	if (li->pub->associated || AP_ENAB(li->pub))
		if (li->activity)
			wlc_led_blink(li, led, LED_BLINKFAST);
		else {
			wlc_led_blinkoff(li, led);
			wlc_led_on(li, led);
		}
	else
		wlc_led_blink(li, led, LED_BLINKSLOW);
}

/* 11b behavior */
static void
wlc_led_wi3(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc = (wlc_info_t *)li->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (WLCISAPHY(wlc->band))
		return;

	if (cfg->associated &&
	    !wlc_rateset_isofdm(cfg->current_bss->rateset.count,
	                        cfg->current_bss->rateset.rates))
		if (li->activity)
			wlc_led_blink(li, led, LED_BLINKFAST);
		else {
			wlc_led_blinkoff(li, led);
			wlc_led_on(li, led);
		}
	else
		wlc_led_blink(li, led, LED_BLINKSLOW);
}

static void
wlc_led_assoc(led_info_t *li, struct led *led)
{
	bool associated;

	if (AP_ENAB(li->pub))
		return;

	associated = li->pub->associated;

	if (!li->up) {
		if (WOWL_ACTIVE(li->pub) ||
			(ASSOC_RECREATE_ENAB(li->pub) &&
			(((wlc_info_t*)(li->wlc))->cfg->flags & WLC_BSSCFG_PRESERVE))) {
			return;
		}
		else {
			ASSERT(!li->pub->associated);
			associated = FALSE;
		}
	}

	if (associated) {
		wlc_led_blinkoff(li, led);
		wlc_led_on(li, led);
	} else
		wlc_led_blink(li, led, LED_BLINKSLOW);
}

/* On while associated; fast blink for activity */
static void
wlc_led_assocact(led_info_t *li, struct led *led)
{
	bool associated;

	if (AP_ENAB(li->pub))
		return;

	associated = li->pub->associated;

	if (!li->up) {
		if (WOWL_ACTIVE(li->pub) ||
			(ASSOC_RECREATE_ENAB(li->pub) &&
			(((wlc_info_t*)(li->wlc))->cfg->flags & WLC_BSSCFG_PRESERVE))) {
			return;
		}
		else {
			ASSERT(!li->pub->associated);
			associated = FALSE;
		}
	}

	if (associated) {
		if (li->activity)
			wlc_led_blink(li, led, LED_BLINKMED);
		else {
			wlc_led_blinkoff(li, led);
			wlc_led_on(li, led);
		}
	} else
		wlc_led_off(li, led);
}

/* 1. Led off when not plug or in power save mode 		*/
/* 2. slow blinking when scanning (before connect to AP) 	*/
/* 3. Solid but will blink when sending or receiving data.	*/
/* 5G band */
static void
wlc_led_wi4(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc;
	bool radio_on;

	wlc = (wlc_info_t *)li->wlc;
	radio_on = BAND_5G(wlc->band->bandtype) ? TRUE : FALSE;

	/* when down, turn off radio LED */
	if (!li->up) {
		wlc_led_blinkoff(li, led);
		wlc_led_off(li, led);
	} else {
		if (li->pub->associated && radio_on) {
			if (li->activity)
				wlc_led_blink(li, led, LED_BLINKFAST);
			else {
				wlc_led_blinkoff(li, led);
				wlc_led_on(li, led);
			}
		 } else if  (li->pub->associated && !radio_on) {
			wlc_led_blinkoff(li, led);
			wlc_led_off(li, led);
		} else if (!(li->pub->associated))
			wlc_led_blink(li, led, LED_BLINKSLOW);
	}
}

/* 2.4G band */
static void
wlc_led_wi5(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc;
	bool radio_on;

	wlc = (wlc_info_t *)li->wlc;
	radio_on = BAND_2G(wlc->band->bandtype) ? TRUE : FALSE;

	/* when down, turn off radio LED */
	if (!li->up) {
		wlc_led_blinkoff(li, led);
		wlc_led(li, (1 << led->pin), (1 << led->pin), led->activehi);
	}
	else {
		if (li->pub->associated && radio_on) {
			if (li->activity)
				wlc_led_blink(li, led, LED_BLINKFAST);
			else {
				wlc_led_blinkoff(li, led);
				wlc_led_off(li, led);
			}
		}
		else if  (li->pub->associated && !radio_on) {
			wlc_led_blinkoff(li, led);
			wlc_led_on(li, led);
		}
		else if (!(li->pub->associated))
			wlc_led_blink(li, led, LED_BLINKSLOW);
	}
}

static void
wlc_led_blinkslow(led_info_t *li, struct led *led)
{
	wlc_led_blink(li, led, LED_BLINKSLOW);
}

static void
wlc_led_blinkmed(led_info_t *li, struct led *led)
{
	wlc_led_blink(li, led, LED_BLINKMED);
}

static void
wlc_led_blinkfast(led_info_t *li, struct led *led)
{
	wlc_led_blink(li, led, LED_BLINKFAST);
}

static void
wlc_led_blinkcustom(led_info_t *li, struct led *led)
{
	wlc_led_blink(li, led, LED_BLINKCUSTOM);
}

/* Blink periodic is based on the 200ms LED timer */
#define LEDPED_ON	1000
#define LEDPED_OFF	400

static void
wlc_led_blinkperiodic(led_info_t *li, struct led *led)
{
#ifdef OSL_SYSUPTIME_SUPPORT
	uint32 now = OSL_SYSUPTIME();

	if (li->periodic_state) {	/* blinking now, turn it off */
		if (now - li->timestamp >= LEDPED_ON) {
			wlc_led_blink(li, led, LED_BLINKCUSTOM);
			li->timestamp = now;
			li->periodic_state = FALSE;
		}
	} else {			/* OFF now, make it blink */
		if (now - li->timestamp >= LEDPED_OFF) {
			wlc_led_blinkoff(li, led);
			wlc_led_off(li, led);
			li->timestamp = now;
			li->periodic_state = TRUE;
		}
	}
#else
	wlc_led_blink(li, led, LED_BLINKCUSTOM);
#endif /* OSL_SYSUPTIME_SUPPORT */
}

static void
wlc_led_assoc_with_security(led_info_t *li, struct led *led)
{
	bool associated;

	if (AP_ENAB(li->pub))
		return;

	associated = li->pub->associated;

	if (!li->up) {
		if (WOWL_ACTIVE(li->pub) ||
			(ASSOC_RECREATE_ENAB(li->pub) &&
			(((wlc_info_t*)(li->wlc))->cfg->flags & WLC_BSSCFG_PRESERVE))) {
			return;
		}
		else {
			ASSERT(!li->pub->associated);
			associated = FALSE;
		}
	}

	if (associated) {
		wlc_led_blinkoff(li, led);
		if (!li->timer_count) {
			li->timer_count = LED_300_SEC_COUNT + 1;
			wlc_led_on(li, led);
		}
		else if (li->timer_count == 1) {
			wlc_led_off(li, led);
			led->behavior = 0;
		}
	} else {
		wlc_led_off(li, led);
		li->timer_count = 0;
	}
}

/* 1. Led off when not plug or in power save mode */
/* 2. slow blinking when scanning (before connect to AP) */
/* 3. Solid after connect to AP	*/
/* 5G band */
static void
wlc_led_wi6(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc;
	bool radio_on;

	wlc = (wlc_info_t *)li->wlc;
	radio_on = BAND_5G(wlc->band->bandtype) ? TRUE : FALSE;

	/* when down, turn off radio LED */
	if (!li->up) {
		wlc_led_blinkoff(li, led);
		wlc_led_off(li, led);
	} else {
		if (li->pub->associated && radio_on) {
			wlc_led_blinkoff(li, led);
			wlc_led_on(li, led);
		 } else if  (li->pub->associated && !radio_on) {
			wlc_led_blinkoff(li, led);
			wlc_led_off(li, led);
		} else if (!(li->pub->associated))
			wlc_led_blink(li, led, LED_BLINKSLOW);
	}
}

/* 1. Led off when not plug or in power save mode */
/* 2. slow blinking when scanning (before connect to AP) */
/* 3. Solid after connect to AP	*/
/* 2.4G band */
static void
wlc_led_wi7(led_info_t *li, struct led *led)
{
	wlc_info_t *wlc;
	bool radio_on;

	wlc = (wlc_info_t *)li->wlc;
	radio_on = BAND_2G(wlc->band->bandtype) ? TRUE : FALSE;

	/* when down, turn off radio LED */
	if (!li->up) {
		wlc_led_blinkoff(li, led);
		wlc_led_off(li, led);
	}
	else {
		if (li->pub->associated && radio_on) {
			wlc_led_blinkoff(li, led);
			wlc_led_on(li, led);

		}
		else if  (li->pub->associated && !radio_on) {
			wlc_led_blinkoff(li, led);
			wlc_led_off(li, led);
		}
		else if (!(li->pub->associated))
			wlc_led_blink(li, led, LED_BLINKSLOW);
	}
}

static void
wlc_led_activity_enab_upd(led_info_t *li)
{
	struct led *led;
	uint i;

	li->activity_led_enab = FALSE;

	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		led = &li->led[i];
		if (led->behavior == WL_LED_ACTIVITY) {
			li->activity_led_enab = TRUE;
		}
	}
}

/* turn activity LED on or off */
void
wlc_led_activityset(led_info_t *li, bool led_state)
{
	struct led *led;
	uint i;

	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		led = &li->led[i];
		if (led->behavior == WL_LED_ACTIVITY) {
			if (led_state == ON)
				wlc_led_on(li, led);
			else
				wlc_led_off(li, led);
		}
	}
}

/* turn radio LEDs on or off when switch band */
void
wlc_led_radioset(led_info_t *li, bool led_state)
{
	struct led *led;
	uint i;

	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		led = &li->led[i];
		if ((led->behavior == WL_LED_ARADIO) ||
		    (led->behavior == WL_LED_BRADIO)) {
			if (led_state == ON)
				wlc_led_on(li, led);
			else
				wlc_led_off(li, led);
		}
	}
}

/* Handling LED related iovars */
static int
wlc_led_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	led_info_t *li = (led_info_t *)hdl;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *)arg;

	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	switch (actionid) {
	case IOV_GVAL(IOV_LED_BEHAVIOR):
	{
		int ledno = ((wl_led_info_t*)params)->index;

		if (ledno >= WL_LED_NUMGPIO)
			return BCME_BADARG;

		bcopy(&li->led[ledno], arg, sizeof(wl_led_info_t)); /* not returning msec on/off */
		break;
	}

	case IOV_SVAL(IOV_LED_BEHAVIOR):
	{
		wl_led_info_t	led;

		bcopy(params, &led, sizeof(wl_led_info_t));

		if (led.index >= WL_LED_NUMGPIO ||
			(led.behavior & WL_LED_BEH_MASK) >= WL_LED_NUMBEHAVIOR)
			return BCME_BADARG;

		li->led[led.index].behavior = led.behavior;

		wlc_led_blinkoff(li, &li->led[led.index]);
		if (led.behavior == WL_LED_OFF) {
			wlc_led_off(li, &li->led[led.index]);
		}
		break;
	}

	case IOV_GVAL(IOV_LED_BLINK_CUSTOM):
	{
		wlc_gled_rate(li, (uint32*) ret_int_ptr, LED_BLINKCUSTOM);
		break;
	}

	case IOV_SVAL(IOV_LED_BLINK_CUSTOM):
	{
		wlc_sled_rate(li, (uint32*) &int_val, LED_BLINKCUSTOM);
		break;
	}

	case IOV_GVAL(IOV_LED_BLINK_SLOW):
	{
		wlc_gled_rate(li, (uint32*) ret_int_ptr, LED_BLINKSLOW);
		break;
	}

	case IOV_SVAL(IOV_LED_BLINK_SLOW):
	{
		wlc_sled_rate(li, (uint32*) &int_val, LED_BLINKSLOW);
		break;
	}

	case IOV_GVAL(IOV_LED_BLINK_MED):
	{
		wlc_gled_rate(li, (uint32*) ret_int_ptr, LED_BLINKMED);
		break;
	}

	case IOV_SVAL(IOV_LED_BLINK_MED):
	{
		wlc_sled_rate(li, (uint32*) &int_val, LED_BLINKMED);
		break;
	}
	case IOV_GVAL(IOV_LED_BLINK_FAST):
	{
		wlc_gled_rate(li, (uint32*) ret_int_ptr, LED_BLINKFAST);
		break;
	}

	case IOV_SVAL(IOV_LED_BLINK_FAST):
	{
		wlc_sled_rate(li, (uint32*) &int_val, LED_BLINKFAST);
		break;
	}

	case IOV_GVAL(IOV_LEDS):
	{
		*ret_int_ptr = li->up;
		break;
	}

	case IOV_SVAL(IOV_LEDS):
	{
		if (int_val)
			wlc_led_up(li->wlc);
		else
			wlc_led_down(li->wlc);
		break;
	}

	case IOV_GVAL(IOV_LED_BLINK_SYNC):
	{
		if (int_val >= WL_LED_NUMGPIO)
			return BCME_BADARG;

		*ret_int_ptr = li->led[int_val].blink_sync;
		break;
	}

	case IOV_SVAL(IOV_LED_BLINK_SYNC):
	{
		int32 int_val2 = 0;

		if (int_val >= WL_LED_NUMGPIO)
			return BCME_BADARG;

		if (p_len >= (int)sizeof(int_val) * 2)
			bcopy((void*)((uintptr)params + sizeof(int_val)),
				&int_val2, sizeof(int_val));
		if ((int_val2 == 0) && (li->led[int_val].blink_sync)) {
			if (li->blink_pins & (0x1 << int_val))
				li->blink_pins &= ~(0x1 << int_val);
		}
		li->led[int_val].blink_sync = (int_val2 != 0) ? TRUE : FALSE;

		break;
	}

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

uint
wlc_led_start_activity_timer(led_info_t *li)
{
	if (li->activity_led_enab == FALSE) {
		return 0;
	}

	li->activity = TRUE;
	if (li->led_timer_set == FALSE) {

		/* Schedule a periodic timer at interval of LED_TIME */
		wl_add_timer(li->wlc->wl, li->led_timer, LED_TIME, TRUE);
		li->led_timer_set = TRUE;
	}
	return 0;
}

uint
wlc_led_stop_activity_timer(led_info_t *li)
{
	uint callbacks = 0;
	if (li->led_timer_set == TRUE) {
		if (!wl_del_timer(li->wlc->wl, li->led_timer)) callbacks++;
		li->led_timer_set = FALSE;
	}
	li->activity = FALSE;
	return callbacks;
}

void
wlc_led_up(wlc_info_t *wlc)
{
	if (wlc->ledh->up)
		return;
	wlc->ledh->up = TRUE;
}

uint
wlc_led_down(wlc_info_t *wlc)
{
	uint callbacks = 0;

	if (!wlc->ledh->up)
		return 0;

	/* cancel the led timer */
	callbacks += wlc_led_stop_activity_timer(wlc->ledh);

	wlc->ledh->up = FALSE;

	/* notify all the LED users */
	callbacks += wlc_led_event(wlc->ledh);

	return callbacks;
}
