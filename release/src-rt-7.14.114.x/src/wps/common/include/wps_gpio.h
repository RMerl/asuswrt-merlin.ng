/*
 * WPS GPIO Header file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_gpio.h 647117 2016-07-04 01:59:23Z $
 */

#ifndef _WPS_GPIO_H_
#define _WPS_GPIO_H_

#include <wps_hal.h>

#ifdef BCA_HNDROUTER
/* Define the WSC SM state const. This definition should be consistence */
#define WSC_PROC_IDLE		0
#define WSC_PROC_WAITING	1
#define WSC_PROC_SUCC		2
#define WSC_PROC_TIMEOUT	3
#define WSC_PROC_FAIL		4
#define WSC_PROC_M2_SENT	5
#define WSC_PROC_M7_SENT	6
#define WSC_PROC_MSG_DONE	7
#define WSC_PROC_PBC_OVERLAP	8

/* Event const definition: */
#define WSC_EVENTS_UNDEFINE	0
#define WSC_EVENTS_BTN_PRESSED	1

/* WPS SM State */
#define WSC_EVENTS_PROC_START		2
#define WSC_EVENTS_PROC_IDLE		(WSC_PROC_IDLE + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_WAITING		(WSC_PROC_WAITING + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_SUCC		(WSC_PROC_SUCC + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_TIMEOUT		(WSC_PROC_TIMEOUT + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_FAIL		(WSC_PROC_FAIL + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_M2_SENT		(WSC_PROC_M2_SENT + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_M7_SENT		(WSC_PROC_M7_SENT + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_MSG_DONE	(WSC_PROC_MSG_DONE + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_PBC_OVERLAP	(WSC_PROC_PBC_OVERLAP + WSC_EVENTS_PROC_START)

/* Led Blink */
#define WSC_LED_OFF	0
#define WSC_LED_ON	1
#define WSC_LED_BLINK	2

#define WSC_LED_OFFSET	0
#define WSC_LED_MASK	0x03L

#define WSC_STATUS_OFFSET	8
#define WSC_STATUS_MASK		0x0000ff00L

#define WSC_EVENT_OFFSET	16
#define WSC_EVENT_MASK		0x00ff0000L

#define WSC_BLINK_OFFSET	24
#define WSC_BLINK_MASK		0xff000000L
#endif /* BCA_HNDROUTER */

#define WPS_LED_FASTBLINK		100		/* fast blink period */
#define WPS_LED_MEDIUMBLINK	1000	/* medium blink period */
#define WPS_LED_SLOWBLINK		5000	/* slow blink period */

#define WPS_LONG_PRESSTIME	5			/* seconds */
#define WPS_BTNSAMPLE_PERIOD	(500 * 1000)	/* 500 ms */
#define WPS_BTN_ASSERTLVL	0
#define WPS_LED_ASSERTLVL	1
#define WPS_LED_BLINK_TIME_UNIT	100 /* 100ms (0.1 second) wps_led_blink_timer() period */

int wps_gpio_btn_init(void);
int wps_gpio_led_init(void);
void wps_gpio_btn_cleanup(void);
void wps_gpio_led_cleanup(void);
wps_btnpress_t wps_gpio_btn_pressed(void);
void wps_gpio_led_blink(wps_blinktype_t blinktype);
void wps_gpio_led_blink_timer();

#endif /* _WPS_GPIO_H_ */
