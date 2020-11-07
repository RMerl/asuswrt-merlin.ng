/*
 * Test harness for dispatcher.
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
#include "tmr.h"

/* request handler */
typedef void (*requestHandlerT)(void *context,
	int reqLength, uint8 *reqData, uint8 *rspData);

#define BUF_SIZE	32

typedef struct {
	requestHandlerT handler;
	uint8 data[BUF_SIZE];
} reqT;

TEST_DECLARE();

static void *testContext = (void *)0xabcd1234;
static tmrT *timer;

/* --------------------------------------------------------------- */

static void processReqEvent(void *context,
	int reqLength, uint8 *reqData, uint8 *rspData)
{
	reqT *req = (reqT *)reqData;

	TEST(context == testContext, "context incorrect");
	TEST(reqLength == sizeof(reqT), "request length incorrect");

	if (rspData != 0) {
		memcpy(rspData, req->data, sizeof(req->data));
	}
}

static void processWlanEvent(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length)
{
#if !TRACE_ENABLED
	(void)eventType;
#endif // endif
	(void)context;
	(void)wlEvent;
	(void)data;
	(void)length;

	TRACE(TRACE_DEBUG, "event_type=%d\n", eventType);
}

static void
timeout(void *arg)
{
	(void)arg;
	TRACE(TRACE_DEBUG, "timeout callback\n");
}

static void testdsp(void)
{
	int i;
	reqT req;
	uint8 rsp[BUF_SIZE];

	req.handler = processReqEvent;
	for (i = 0; i < BUF_SIZE; i++)
		req.data[i] = i;

	TEST(dspSubscribe(dsp(), 0, processWlanEvent),
		"dspSubscribe failed");

	timer = tmrCreate(dsp(), timeout, 0, "test");
	tmrStart(timer, 5 * 1000, FALSE);

	for (i = 0; i < 10; i++)
		TEST(dspRequest(dsp(), testContext, sizeof(req), (uint8 *)&req),
			"dspData failed");

	for (i = 0; i < 10; i++) {
		memset(req.data, i, sizeof(req.data));
		memset(rsp, 0, sizeof(rsp));
		TRACE_HEX_DUMP(TRACE_DEBUG, "req bufffer", sizeof(req.data), req.data);
		TEST(dspRequestSynch(dsp(), testContext,
			sizeof(req), (uint8 *)&req, rsp), "dspDataSync failed");
		TRACE_HEX_DUMP(TRACE_DEBUG, "rsp bufffer", sizeof(rsp), rsp);
		TEST(memcmp(req.data, rsp, sizeof(rsp)) == 0, "response data incorrect");
	}

	sleep(1);

	TEST(dspUnsubscribe(dsp(), processWlanEvent),
		"dspUnsubscribe failed");
	dspFree();
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_ALL);
	TEST_INITIALIZE();

	testdsp();

	TEST_FINALIZE();
	return 0;
}
