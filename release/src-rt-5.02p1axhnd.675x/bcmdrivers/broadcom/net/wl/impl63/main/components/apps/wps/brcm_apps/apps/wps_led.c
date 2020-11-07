/*
 * WPS LED (include LAN leds control functions)
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
 * $Id: wps_led.c 766338 2018-07-31 04:55:48Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wpscommon.h>
#include <wps_hal.h>
#include <wps_led.h>
#include <wps_wps.h>
#include <tutrace.h>

extern int wps_getProcessStates();

static int wps_prevstatus = -1;

int
wps_led_wl_select_on()
{
	return wps_hal_led_wl_select_on();
}

int
wps_led_wl_select_off()
{
	return wps_hal_led_wl_select_off();
}

void
wps_led_wl_selecting(int led_id)
{
	wps_hal_led_wl_selecting(led_id);
}

void
wps_led_wl_confirmed(int led_id)
{
	wps_hal_led_wl_confirmed(led_id);
}

static int
wps_led(int status)
{
	switch (status) {
	case WPS_INIT:
		wps_hal_led_blink(WPS_BLINKTYPE_STOP);
		break;

	case WPS_ASSOCIATED:
	case WPS_SENDM2:
	case WPS_SENDM7:
	case WPS_MSGDONE:
		wps_hal_led_blink(WPS_BLINKTYPE_INPROGRESS);
		break;

	case WPS_OK:
		wps_hal_led_blink(WPS_BLINKTYPE_SUCCESS);
		break;

	case WPS_TIMEOUT:

	case WPS_MSG_ERR:
		wps_hal_led_blink(WPS_BLINKTYPE_ERROR);
		break;

	case WPS_PBCOVERLAP:
		wps_hal_led_blink(WPS_BLINKTYPE_OVERLAP);
		break;

	case WPS_FIND_PBC_AP:
	case WPS_ASSOCIATING:
		wps_hal_led_blink(WPS_BLINKTYPE_INPROGRESS);
		break;

	default:
		wps_hal_led_blink(WPS_BLINKTYPE_STOP);
		break;
	}

	return 0;
}

void
wps_led_update()
{
	int val;

	val = wps_getProcessStates();
	if (val != -1) {
		if (wps_prevstatus != val) {
			wps_prevstatus = val;
			wps_led(val);
		}
	}
}

int
wps_led_init()
{
	int ret;
	int val;

	ret = wps_hal_led_init();
	if (ret == 0) {
		/* sync wps led */
		wps_prevstatus = WPS_INIT;
		val = wps_getProcessStates();
		if (val != -1) {
			wps_prevstatus = val;
		}

		/* off all wps multi-color led */
		if (val == WPS_INIT)
			wps_hal_led_blink(WPS_BLINKTYPE_STOP_MULTI);

		/* set wps led blink */
		wps_led(wps_prevstatus);
	}

	return ret;
}

void
wps_led_deinit()
{
	/* Show WPS OK or ERROR led before deinit */
	wps_led_update();
	WpsSleepMs(500); /* 500 ms */

	wps_hal_led_deinit();
}
