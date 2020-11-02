/*
 * Test harness for encoding and decoding WNM packets
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
#include <bcm_encode_wnm.h>
#include <bcm_decode_wnm.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */
#define BUFFER_SIZE		512
static uint8 *buffer;
static bcm_encode_t *enc;

/* ------------ Setup and Teardown - testEncodeSubscriptionRemediation ------------- */

void testEncSub_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
}

void Wnm_teardown(void)
{
	/* Release the memory */
	free(enc);
	free(buffer);
}

/* ----------- Setup - testDecodeSubscriptionRemediation ------------- */

void testDecSub_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
	bcm_encode_init(enc, BUFFER_SIZE, buffer);
	bcm_encode_wnm_subscription_remediation(enc, 1, 10, "helloworld", 1);
}

/* ----------- Setup - testDecodeDeauthenticationImminent ------------ */

void testDecDeauth_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncode failed\n");
		exit(-1);
	}
	bcm_encode_init(enc, BUFFER_SIZE, buffer);
	bcm_encode_wnm_deauthentication_imminent(enc, 2, HSPOT_DEAUTH_RC_ESS_DISALLOW, 1000, 10,
		"helloworld");
}

/* ----------------------UNIT TESTS------------------------ */

START_TEST(testEncodeSubscriptionRemediation)
{
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_wnm_subscription_remediation(enc, 1, 10, "helloworld", 1),
		"bcm_encode_wnm_subscription_remediation failed");
	WL_PRPKT("encoded packet", bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeSubscriptionRemediation)
{
	bcm_decode_t dec;
	bcm_decode_wnm_subscription_remediation_t wnm;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	ck_assert_msg(bcm_decode_wnm_subscription_remediation(&dec, &wnm),
		"bcm_decode_wnm_subscription_remediation failed");
	ck_assert_msg(wnm.dialogToken == 1, "invalid data");
	ck_assert_msg(wnm.urlLength == 10, "invalid data");
	ck_assert_msg(strcmp(wnm.url, "helloworld") == 0, "invalid data");
	ck_assert_msg(wnm.serverMethod == 1, "invalid data");
}
END_TEST

START_TEST(testEncodeDeauthenticationImminent)
{
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_wnm_deauthentication_imminent(enc, 2,
		HSPOT_DEAUTH_RC_ESS_DISALLOW, 1000, 10, "helloworld"),
		"bcm_encode_wnm_deauthentication_imminent failed");
	WL_PRPKT("encoded packet", bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeDeauthenticationImminent)
{
	bcm_decode_t dec;
	bcm_decode_wnm_deauthentication_imminent_t wnm;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	ck_assert_msg(bcm_decode_wnm_deauthentication_imminent(&dec, &wnm),
		"bcm_decode_wnm_deauthentication_imminent failed");
	ck_assert_msg(wnm.dialogToken == 2, "invalid data");
	ck_assert_msg(wnm.reason == HSPOT_DEAUTH_RC_ESS_DISALLOW, "invalid data");
	ck_assert_msg(wnm.reauthDelay == 1000, "invalid data");
	ck_assert_msg(wnm.urlLength == 10, "invalid data");
	ck_assert_msg(strcmp(wnm.url, "helloworld") == 0, "invalid data");
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *bcm_enc_dec_wnm_suite(void)
{
	Suite *s = suite_create("bcm_enc_dec_wnm_suite");

	/* Encode & Decode WNM */
	TCase *tc_EncWnm = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_EncWnm, testEncSub_setup, Wnm_teardown);
	tcase_add_test(tc_EncWnm, testEncodeSubscriptionRemediation);
	tcase_add_test(tc_EncWnm, testEncodeDeauthenticationImminent);

	TCase *tc_DecSub = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_DecSub, testDecSub_setup, Wnm_teardown);
	tcase_add_test(tc_DecSub, testDecodeSubscriptionRemediation);

	TCase *tc_DecDeauth = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_DecDeauth, testDecDeauth_setup, Wnm_teardown);
	tcase_add_test(tc_DecDeauth, testDecodeDeauthenticationImminent);

	suite_add_tcase(s, tc_EncWnm);
	suite_add_tcase(s, tc_DecSub);
	suite_add_tcase(s, tc_DecDeauth);

	return s;
}
