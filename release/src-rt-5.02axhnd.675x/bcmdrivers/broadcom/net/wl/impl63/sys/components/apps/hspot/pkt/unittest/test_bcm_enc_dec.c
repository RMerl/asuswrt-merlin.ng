/*
 * Test harness for encoding and decoding base functions.
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
#include <trace.h>
#include <bcm_encode.h>
#include <bcm_decode.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */

#define BUFFER_SIZE		256
static uint8 *buffer;
static bcm_encode_t *enc;

/* ------------- Setup and Teardown - testEncode --------------- */

void testEnc_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
}

void testEncDec_teardown(void)
{
	/* Release the memory */
	free(enc);
	free(buffer);
}

/* ------------- Setup - testDecode --------------- */

void testDec_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testDecode failed\n");
		exit(-1);
	}

	bcm_encode_init(enc, BUFFER_SIZE, buffer);
	bcm_encode_be16(enc, 0x1122);
	bcm_encode_be32(enc, 0x11223344);
	bcm_encode_le16(enc, 0xaabb);
	bcm_encode_le32(enc, 0xaabbccdd);
}

/* ----------------------UNIT TESTS------------------------ */

START_TEST(testEncode)
{
	uint8 data[BUFFER_SIZE];

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");

	ck_assert_msg(bcm_encode_be16(enc, 0x1122) == 2, "bcm_encode_be16 failed");
	ck_assert_msg(bcm_encode_be32(enc, 0x11223344) == 4, "bcm_encode_be32 failed");
	ck_assert_msg(bcm_encode_le16(enc, 0xaabb) == 2, "bcm_encode_le16 failed");
	ck_assert_msg(bcm_encode_le32(enc, 0xaabbccdd) == 4, "bcm_encode_le32 failed");

	/* packet full */
	ck_assert_msg(bcm_encode_bytes(enc, BUFFER_SIZE, data) == 0, "bcm_encode_bytes failed");
	ck_assert_msg(bcm_encode_length(enc) == 12, "bcm_encode_length failed");
}
END_TEST

START_TEST(testDecode)
{
	bcm_decode_t dec;
	uint16 data16;
	uint32 data32;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	data16 = 0;
	ck_assert_msg(bcm_decode_be16(&dec, &data16) == 2, "bcm_decode_be16 failed");
	ck_assert_msg(data16 == 0x1122, "invalid data");
	data32 = 0;
	ck_assert_msg(bcm_decode_be32(&dec, &data32) == 4, "bcm_decode_be32 failed");
	ck_assert_msg(data32 == 0x11223344, "invalid data");
	data16 = 0;
	ck_assert_msg(bcm_decode_le16(&dec, &data16) == 2, "bcm_decode_le16 failed");
	ck_assert_msg(data16 == 0xaabb, "invalid data");
	data32 = 0;
	ck_assert_msg(bcm_decode_le32(&dec, &data32) == 4, "bcm_decode_le32 failed");
	ck_assert_msg(data32 == 0xaabbccdd, "invalid data");

	/* decode beyond buffer */
	ck_assert_msg(bcm_decode_be16(&dec, &data16) == 0, "bcm_decode_be16 failed");
	ck_assert_msg(bcm_decode_be32(&dec, &data32) == 0, "bcm_decode_be32 failed");
	ck_assert_msg(bcm_decode_le16(&dec, &data16) == 0, "bcm_decode_le16 failed");
	ck_assert_msg(bcm_decode_le32(&dec, &data32) == 0, "bcm_decode_le32 failed");
}
END_TEST

START_TEST(testDecodeZeroLength)
{
	bcm_decode_t dec;

	/* no decode */
	ck_assert_msg(bcm_decode_init(&dec, 0, 0), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_remaining(&dec) == 0, "bcm_decode_remaining failed");
	ck_assert_msg(bcm_decode_buf(&dec) == 0, "bcm_decode_buf failed");
	ck_assert_msg(!bcm_decode_is_zero_length(&dec), "bcm_decode_is_zero_length failed");

	/* valid zero length decode */
	ck_assert_msg(bcm_decode_init(&dec, 0, bcm_encode_buf(enc)), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_remaining(&dec) == 0, "bcm_decode_remaining failed");
	ck_assert_msg(bcm_decode_buf(&dec) != 0, "bcm_decode_buf failed");
	ck_assert_msg(bcm_decode_is_zero_length(&dec), "bcm_decode_is_zero_length failed");
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *bcm_enc_dec_suite(void)
{
	Suite *s = suite_create("bcm_enc_dec_suite");

	TCase *tc_Enc = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_Enc, testEnc_setup, testEncDec_teardown);
	tcase_add_test(tc_Enc, testEncode);

	TCase *tc_Dec = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_Dec, testDec_setup, testEncDec_teardown);
	tcase_add_test(tc_Dec, testDecode);
	tcase_add_test(tc_Dec, testDecodeZeroLength);

	suite_add_tcase(s, tc_Enc);
	suite_add_tcase(s, tc_Dec);

	return s;
}
