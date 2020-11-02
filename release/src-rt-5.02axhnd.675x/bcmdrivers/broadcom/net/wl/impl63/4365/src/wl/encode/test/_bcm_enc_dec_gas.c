/*
 * Test harness for encoding and decoding 802.11u GAS packets.
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
#include "proto/802.11.h"
#include "test.h"
#include "trace.h"
#include "pktEncodeGas.h"
#include "pktDecodeGas.h"

TEST_DECLARE();

#define BUFFER_SIZE		256
static uint8 buffer[BUFFER_SIZE];
static bcm_pkt_encode_t enc;

/* --------------------------------------------------------------- */

static void testPktGasRequest(void)
{
	bcm_pkt_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 req[128];
	int i;
	pktGasDecodeT gasDecode;

	for (i = 0; i < 128; i++)
		req[i] = i;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasRequest(&enc, 0x11, 4, apie, 128, req),
		"pktEncodeGasRequest failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_REQUEST_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
	TEST(gasDecode.request.apie.protocolId == 0, "decode failed");
	TEST(gasDecode.request.reqLen == 128, "decode failed");
}

static void testPktGasResponse(void)
{
	bcm_pkt_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 rsp[128];
	int i;
	pktGasDecodeT gasDecode;

	for (i = 0; i < 128; i++)
		rsp[i] = i;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasResponse(&enc, 0x11, 0x1234, 0x5678, 4, apie, 128, rsp),
		"pktEncodeGasResponse failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_RESPONSE_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
	TEST(gasDecode.response.statusCode == 0x1234, "decode failed");
	TEST(gasDecode.response.comebackDelay == 0x5678, "decode failed");
	TEST(gasDecode.response.apie.protocolId == 0, "decode failed");
	TEST(gasDecode.response.rspLen == 128, "decode failed");
}

static void testPktGasComebackRequest(void)
{
	bcm_pkt_decode_t dec;
	pktGasDecodeT gasDecode;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasComebackRequest(&enc, 0x11),
		"pktEncodeGasComebackRequest failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_COMEBACK_REQUEST_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
}

static void testPktGasComebackResponse(void)
{
	bcm_pkt_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 rsp[128];
	int i;
	pktGasDecodeT gasDecode;

	for (i = 0; i < 128; i++)
		rsp[i] = i;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasComebackResponse(&enc, 0x11, 0x1234, 0xaa, 0x5678, 4, apie, 128, rsp),
		"pktEncodeGasComebackResponse failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_COMEBACK_RESPONSE_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
	TEST(gasDecode.comebackResponse.statusCode == 0x1234, "decode failed");
	TEST(gasDecode.comebackResponse.fragmentId == 0xaa, "decode failed");
	TEST(gasDecode.comebackResponse.comebackDelay == 0x5678, "decode failed");
	TEST(gasDecode.comebackResponse.apie.protocolId == 0, "decode failed");
	TEST(gasDecode.comebackResponse.rspLen == 128, "decode failed");
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_ALL);
	TEST_INITIALIZE();

	testPktGasRequest();
	testPktGasResponse();
	testPktGasComebackRequest();
	testPktGasComebackResponse();

	TEST_FINALIZE();
	return 0;
}
