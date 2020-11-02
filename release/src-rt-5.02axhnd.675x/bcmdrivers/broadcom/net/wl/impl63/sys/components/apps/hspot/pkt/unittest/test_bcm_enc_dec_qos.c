/*
 * Test harness for encoding and decoding QoS packets
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
#include <bcmip.h>
#include <trace.h>
#include <bcm_encode_qos.h>
#include <bcm_decode_qos.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */
#define BUFFER_SIZE		512
static uint8 *buffer;
static bcm_encode_t *enc;

static uint8 dscpToUp(uint8 dscp, uint8 *qosMapIe)
{
	uint8 *ptr = qosMapIe;
	uint8 len;

	if (ptr != 0 &&
		ptr[0] == DOT11_MNG_QOS_MAP_ID && (len = ptr[1]) >= 16) {
		int i;

		ptr += 2;

		/* check dscp exceptions */
		for (; len > 16; len -= 2) {
			if (dscp == ptr[0])
				return ptr[1];
			ptr += 2;
		}

		/* check dscp/up ranges */
		for (i = 0; i < 16; i += 2) {
			uint8 low = ptr[i];
			uint high = ptr[i + 1];
			if (low == 255 && high == 255)
				continue;
			if (dscp >= low && dscp <= high)
				return (i / 2);
		}
	}

	return dscp >> IPV4_TOS_PREC_SHIFT;
}

/* ------------ Setup and Teardown - testEncodeQosMap ------------- */

void testEncQos_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
}

void Qos_teardown(void)
{
	/* Release the memory */
	free(enc);
	free(buffer);
}

/* ------------ Setup - testDecodeQosMap ------------- */

void testDecQos_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
	bcm_encode_init(enc, BUFFER_SIZE, buffer);
	bcm_encode_qos_map(enc, 4, (uint8 *)"\x35\x02\x16\x06", 8, 15, 0, 7, 255, 255,
		16, 31, 32, 39, 255, 255, 40, 47, 255, 255);
}

/* ------------ Setup - testDecodeQosMap1 ------------- */

void testDecQos1_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
	bcm_encode_init(enc, BUFFER_SIZE, buffer);
	bcm_encode_qos_map(enc, 0, 0, 8, 15, 0, 7, 255, 255, 16, 31,
		32, 39, 255, 255, 40, 47, 48, 63);
}

/* ----------------------UNIT TESTS------------------------ */

START_TEST(testEncodeQosMap)
{
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_qos_map(enc, 4, (uint8 *)"\x35\x02\x16\x06",
		8, 15, 0, 7, 255, 255, 16, 31, 32, 39, 255, 255, 40, 47, 255, 255),
		"bcm_encode_qos_map failed");
	WL_PRPKT("encoded packet", bcm_encode_buf(enc), bcm_encode_length(enc));

	ck_assert_msg(dscpToUp(53, bcm_encode_buf(enc) + 2) == 2,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(22, bcm_encode_buf(enc) + 2) == 6,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(10, bcm_encode_buf(enc) + 2) == 0,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(0, bcm_encode_buf(enc) + 2) == 1,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(18, bcm_encode_buf(enc) + 2) == 3,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(26, bcm_encode_buf(enc) + 2) == 3,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(34, bcm_encode_buf(enc) + 2) == 4,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(46, bcm_encode_buf(enc) + 2) == 6,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(47, bcm_encode_buf(enc) + 2) == 6,
		"dscpToUp failed");
	ck_assert_msg(dscpToUp(48, bcm_encode_buf(enc) + 2)
		== (48 >> IPV4_TOS_PREC_SHIFT), "dscpToUp failed");
}
END_TEST

START_TEST(testDecodeQosMap)
{
	bcm_decode_t dec;
	bcm_decode_qos_map_t qos;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	ck_assert_msg(bcm_decode_qos_map(&dec, &qos),
		"bcm_decode_qos_map failed");
	ck_assert_msg(qos.exceptCount == 2, "invalid data");
	ck_assert_msg(qos.except[0].dscp == 0x35, "invalid data");
	ck_assert_msg(qos.except[0].up == 0x02, "invalid data");
	ck_assert_msg(qos.except[1].dscp == 0x16, "invalid data");
	ck_assert_msg(qos.except[1].up == 0x06, "invalid data");
	ck_assert_msg(qos.up[0].low == 8, "invalid data");
	ck_assert_msg(qos.up[0].high == 15, "invalid data");
	ck_assert_msg(qos.up[1].low == 0, "invalid data");
	ck_assert_msg(qos.up[1].high == 7, "invalid data");
	ck_assert_msg(qos.up[2].low == 255, "invalid data");
	ck_assert_msg(qos.up[2].high == 255, "invalid data");
	ck_assert_msg(qos.up[3].low == 16, "invalid data");
	ck_assert_msg(qos.up[3].high == 31, "invalid data");
	ck_assert_msg(qos.up[4].low == 32, "invalid data");
	ck_assert_msg(qos.up[4].high == 39, "invalid data");
	ck_assert_msg(qos.up[5].low == 255, "invalid data");
	ck_assert_msg(qos.up[5].high == 255, "invalid data");
	ck_assert_msg(qos.up[6].low == 40, "invalid data");
	ck_assert_msg(qos.up[6].high == 47, "invalid data");
	ck_assert_msg(qos.up[7].low == 255, "invalid data");
	ck_assert_msg(qos.up[7].high == 255, "invalid data");
}
END_TEST

START_TEST(testEncodeQosMap1)
{
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_qos_map(enc, 0, 0,
		8, 15, 0, 7, 255, 255, 16, 31, 32, 39, 255, 255, 40, 47, 48, 63),
		"bcm_encode_qos_map failed");
	WL_PRPKT("encoded packet", bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeQosMap1)
{
	bcm_decode_t dec;
	bcm_decode_qos_map_t qos;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	ck_assert_msg(bcm_decode_qos_map(&dec, &qos),
		"bcm_decode_qos_map failed");
	ck_assert_msg(qos.exceptCount == 0, "invalid data");
	ck_assert_msg(qos.up[0].low == 8, "invalid data");
	ck_assert_msg(qos.up[0].high == 15, "invalid data");
	ck_assert_msg(qos.up[1].low == 0, "invalid data");
	ck_assert_msg(qos.up[1].high == 7, "invalid data");
	ck_assert_msg(qos.up[2].low == 255, "invalid data");
	ck_assert_msg(qos.up[2].high == 255, "invalid data");
	ck_assert_msg(qos.up[3].low == 16, "invalid data");
	ck_assert_msg(qos.up[3].high == 31, "invalid data");
	ck_assert_msg(qos.up[4].low == 32, "invalid data");
	ck_assert_msg(qos.up[4].high == 39, "invalid data");
	ck_assert_msg(qos.up[5].low == 255, "invalid data");
	ck_assert_msg(qos.up[5].high == 255, "invalid data");
	ck_assert_msg(qos.up[6].low == 40, "invalid data");
	ck_assert_msg(qos.up[6].high == 47, "invalid data");
	ck_assert_msg(qos.up[7].low == 48, "invalid data");
	ck_assert_msg(qos.up[7].high == 63, "invalid data");
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *bcm_enc_dec_qos_suite(void)
{
	Suite *s = suite_create("bcm_enc_dec_qos_suite");

	/* Encode & Decode QOS Map */
	TCase *tc_EncQos = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_EncQos, testEncQos_setup, Qos_teardown);
	tcase_add_test(tc_EncQos, testEncodeQosMap);
	tcase_add_test(tc_EncQos, testEncodeQosMap1);

	TCase *tc_DecQos = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_DecQos, testDecQos_setup, Qos_teardown);
	tcase_add_test(tc_DecQos, testDecodeQosMap);

	TCase *tc_DecQos1 = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_DecQos1, testDecQos1_setup, Qos_teardown);
	tcase_add_test(tc_DecQos1, testDecodeQosMap1);

	suite_add_tcase(s, tc_EncQos);
	suite_add_tcase(s, tc_DecQos);
	suite_add_tcase(s, tc_DecQos1);

	return s;
}
