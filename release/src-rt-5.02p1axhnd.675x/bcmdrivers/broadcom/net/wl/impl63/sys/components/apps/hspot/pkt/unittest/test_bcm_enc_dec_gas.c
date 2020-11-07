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
#include <802.11.h>
#include <trace.h>
#include <bcm_encode_gas.h>
#include <bcm_decode_gas.h>
#include <bcmwifi_channels.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */

#define BUFFER_SIZE		256
static uint8 *buffer;
static bcm_encode_t *enc;

/* ------------- Setup and Teardown - Fixtures --------------- */

void gas_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
}

void gas_teardown(void)
{
	/* Release the memory */
	free(enc);
	free(buffer);
}

/* ----------------------UNIT TESTS------------------------ */

START_TEST(testPktGasRequest)
{
	bcm_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 req[128];
	int i;
	bcm_decode_gas_t gasDecode;

	for (i = 0; i < 128; i++)
		req[i] = i;

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_gas_request(enc, 0x11, 4, apie, 128, req),
		"bcm_encode_gas_request failed");

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_gas(&dec, &gasDecode), "bcm_decode_gas failed");
	ck_assert_msg(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	ck_assert_msg(gasDecode.action == GAS_REQUEST_ACTION_FRAME, "decode failed");
	ck_assert_msg(gasDecode.dialogToken == 0x11, "decode failed");
	ck_assert_msg(gasDecode.request.apie.protocolId == 0, "decode failed");
	ck_assert_msg(gasDecode.request.reqLen == 128, "decode failed");
}
END_TEST

START_TEST(testPktGasResponse)
{
	bcm_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 rsp[128];
	int i;
	bcm_decode_gas_t gasDecode;

	for (i = 0; i < 128; i++)
		rsp[i] = i;

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");

	ck_assert_msg(bcm_encode_gas_response(enc, 0x11, 0x1234, 0x5678, 4, apie, 128, rsp),
		"bcm_encode_gas_response failed");

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_gas(&dec, &gasDecode), "bcm_decode_gas failed");
	ck_assert_msg(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	ck_assert_msg(gasDecode.action == GAS_RESPONSE_ACTION_FRAME, "decode failed");
	ck_assert_msg(gasDecode.dialogToken == 0x11, "decode failed");
	ck_assert_msg(gasDecode.response.statusCode == 0x1234, "decode failed");
	ck_assert_msg(gasDecode.response.comebackDelay == 0x5678, "decode failed");
	ck_assert_msg(gasDecode.response.apie.protocolId == 0, "decode failed");
	ck_assert_msg(gasDecode.response.rspLen == 128, "decode failed");
}
END_TEST

START_TEST(testPktGasComebackRequest)
{
	bcm_decode_t dec;
	bcm_decode_gas_t gasDecode;

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");

	ck_assert_msg(bcm_encode_gas_comeback_request(enc, 0x11),
		"bcm_encode_gas_comeback_request failed");

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_gas(&dec, &gasDecode), "bcm_decode_gas failed");
	ck_assert_msg(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	ck_assert_msg(gasDecode.action == GAS_COMEBACK_REQUEST_ACTION_FRAME, "decode failed");
	ck_assert_msg(gasDecode.dialogToken == 0x11, "decode failed");
}
END_TEST

START_TEST(testPktGasComebackResponse)
{
	bcm_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 rsp[128];
	int i;
	bcm_decode_gas_t gasDecode;

	for (i = 0; i < 128; i++)
		rsp[i] = i;

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");

	ck_assert_msg(bcm_encode_gas_comeback_response(enc, 0x11, 0x1234, 0xaa, 0x5678,
		4, apie, 128, rsp), "bcm_encode_gas_comeback_response failed");

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_gas(&dec, &gasDecode), "bcm_decode_gas failed");
	ck_assert_msg(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	ck_assert_msg(gasDecode.action == GAS_COMEBACK_RESPONSE_ACTION_FRAME, "decode failed");
	ck_assert_msg(gasDecode.dialogToken == 0x11, "decode failed");
	ck_assert_msg(gasDecode.comebackResponse.statusCode == 0x1234, "decode failed");
	ck_assert_msg(gasDecode.comebackResponse.fragmentId == 0xaa, "decode failed");
	ck_assert_msg(gasDecode.comebackResponse.comebackDelay == 0x5678, "decode failed");
	ck_assert_msg(gasDecode.comebackResponse.apie.protocolId == 0, "decode failed");
	ck_assert_msg(gasDecode.comebackResponse.rspLen == 128, "decode failed");
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *bcm_enc_dec_gas_suite(void)
{
	Suite *s = suite_create("bcm_enc_dec_gas_suite");

	/* GAS request & Response */
	TCase *tc_enc_dec_gas = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_enc_dec_gas, gas_setup, gas_teardown);

	tcase_add_test(tc_enc_dec_gas, testPktGasRequest);
	tcase_add_test(tc_enc_dec_gas, testPktGasResponse);

	tcase_add_test(tc_enc_dec_gas, testPktGasComebackRequest);
	tcase_add_test(tc_enc_dec_gas, testPktGasComebackResponse);

	suite_add_tcase(s, tc_enc_dec_gas);
	return s;
}
