/*
 * Test harness for WLAN functions.
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
 * $Id:$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "trace.h"
#include "test.h"
#include "dsp.h"
#include "wlu_api.h"
#include "wlan.h"

TEST_DECLARE();

static void wlanEventCallback(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length)
{
	TEST((int)context == 0x12345678, "invalid context");
	if (eventType == WLC_E_ESCAN_RESULT) {
		if (wlEvent->status == WLC_E_STATUS_SUCCESS) {
			TRACE(TRACE_DEBUG,
				"WLC_E_ESCAN_RESULT - WLC_E_STATUS_SUCCESS %d\n", length);
		}
		else if (wlEvent->status == WLC_E_STATUS_PARTIAL) {
			wl_escan_result_t *escan_data = (wl_escan_result_t*)data;

			TRACE(TRACE_DEBUG,
				"WLC_E_ESCAN_RESULT - WLC_E_STATUS_PARTIAL\n");

			if (length >= sizeof(*escan_data)) {
				wl_bss_info_t *bi = &escan_data->bss_info[0];
				dump_bss_info(bi);
			}
		}
	}
}

/* --------------------------------------------------------------- */

static void testIfName(void)
{
	wlanT *wlan;

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(strcmp(wlanIfName(wlan), "wlan0") == 0, "wlanIfName failed");
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testEtherAddr(void)
{
	wlanT *wlan;
	struct ether_addr addr;

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(wlanEtherAddr(wlan, &addr), "wlanEtherAddr failed");
	TRACE_MAC_ADDR(TRACE_PRINTF, "ether addr", &addr);
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testEventMsg(void)
{
	wlanT *wlan;

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(wlanDisableEventMsg(wlan, WLC_E_ACTION_FRAME_RX), "wlanEnableEventMsg failed");
	TEST(wlanEnableEventMsg(wlan, WLC_E_ACTION_FRAME_RX), "wlanEnableEventMsg failed");
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testVendorIe(void)
{
	wlanT *wlan;
	uint8 ie[] = "\x50\x6F\x9A\x10\x01\x02\x03";

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(wlanAddVendorIe(wlan, VNDR_IE_PRBREQ_FLAG, sizeof(ie), ie),
		"wlanAddVendorIe failed");
	TEST(wlanDeleteVendorIe(wlan, VNDR_IE_PRBREQ_FLAG, sizeof(ie), ie),
		"wlanDeleteVendorIe failed");
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testEscan(void)
{
	wlanT *wlan;

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(wlanStartEscan(wlan, TRUE, -1, -1, -1), "wlanStartEscan failed");
	sleep(1);
	TEST(wlanStopScan(wlan), "wlanStopScan failed");
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testBssTransitionQuery(void)
{
	wlanT *wlan;

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(wlanBssTransitionQuery(wlan), "wlanBssTransitionQuery failed");
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testActionFrame(void)
{
	wlanT *wlan;
	struct ether_addr da = {{0x00, 0x11, 0x11, 0x11, 0x11, 0x11}};
	char data[256];
	int i;

	data[DOT11_ACTION_CAT_OFF] = DOT11_ACTION_CAT_PUBLIC;
	data[DOT11_ACTION_ACT_OFF] = GAS_REQUEST_ACTION_FRAME;
	for (i = DOT11_ACTION_ACT_OFF + 1; i < (int)sizeof(data); i++)
		data[i] = i;

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(wlanActionFrame(wlan, (int)data, 1, 500, &da, &da, sizeof(data), (uint8 *)data),
		"wlanActionFrame faile");
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testAssociationStatus(void)
{
	wlanT *wlan;
	int isAssociated;
	char buffer[1024];

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	TEST(wlanAssociationStatus(wlan, &isAssociated, sizeof(buffer), (wl_bss_info_t *)buffer),
		"wlanAssociationStatus failed");
	if (isAssociated) {
		dump_bss_info((wl_bss_info_t *)buffer);
	}
	else {
		printf("not associated\n");
	}
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}

static void testWnm(void)
{
	wlanT *wlan;
	int wMask, rMask;

	wlan = wlanCreate();
	TEST(wlan != 0, "wlanCreate failed");
	for (wMask = 0; wMask < 0x10; wMask++) {
		TEST(wlanWnm(wlan, wMask), "wlanWnm failed");
		TEST(wlanWnmGet(wlan, &rMask), "wlanWnmGet failed");
		TEST(wMask == rMask, "invalid data");
	}
	TEST(wlanDestroy(wlan), "wlanDestroy failed");
}
int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_ALL);
	TEST_INITIALIZE();

	wlanSubscribeEvent((void *)0x12345678, wlanEventCallback);
	wlanInitialize();

	testIfName();
	testEtherAddr();
	testEventMsg();
	testVendorIe();
	testEscan();
	testBssTransitionQuery();
	testActionFrame();
	testAssociationStatus();
	testWnm();

	wlFree();
	dspFree();

	wlanDeinitialize();
	wlanUnsubscribeEvent(wlanEventCallback);

	TEST_FINALIZE();
	return 0;
}
