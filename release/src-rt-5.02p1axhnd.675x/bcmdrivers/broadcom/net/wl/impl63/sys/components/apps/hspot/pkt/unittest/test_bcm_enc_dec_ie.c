/*
 * Test harness for encoding and decoding information elements.
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
#include <bcm_encode_ie.h>
#include <bcm_decode_ie.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */
#define BUFFER_SIZE		256
static uint8 *buffer;
static bcm_encode_t *enc;

/* ------------- Setup and Teardown - testEncodeIe --------------- */

void testEncIe_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testEncodeIe failed\n");
		exit(-1);
	}
}

void testEncDecIe_teardown(void)
{
	/* Release the memory */
	free(enc);
	free(buffer);
}

/* ------------- Setup - testDecodeIe --------------- */

void testDecIe_setup(void)
{
	/* Initializing global pointers "enc" & "buffer" */
	uint8 adBuffer[BUFFER_SIZE];
	bcm_encode_t ad;

	enc = (bcm_encode_t *)malloc(sizeof(*enc));
	buffer = (uint8 *)malloc(BUFFER_SIZE);

	if (enc == NULL || buffer == NULL) {
		printf("Setup function for testDecodeIe failed\n");
		exit(-1);
	}

	bcm_encode_init(enc, BUFFER_SIZE, buffer);
	bcm_encode_ie_hotspot_indication2(enc, TRUE, HSPOT_RELEASE_2, TRUE, 0x1234, TRUE, 0x5678);
	bcm_encode_ie_interworking(enc, IW_ANT_WILDCARD_NETWORK, FALSE, FALSE, FALSE, FALSE,
		TRUE, 0, 0, (struct ether_addr *)"\xff\xff\xff\xff\xff\xff");
	bcm_encode_init(&ad, sizeof(adBuffer), adBuffer);
	bcm_encode_ie_advertisement_protocol_tuple(&ad, ADVP_PAME_BI_DEPENDENT, 0xff,
		ADVP_ANQP_PROTOCOL_ID);
	bcm_encode_ie_advertisement_protocol_from_tuple(enc, bcm_encode_length(&ad),
		bcm_encode_buf(&ad));
	bcm_encode_ie_roaming_consortium(enc, 0xff, 3, (uint8 *)"\x00\x11\x22", 4,
		(uint8 *)"\x55\x66\x77\x88", 4, (uint8 *)"\xaa\xbb\xcc\xdd");
	bcm_encode_ie_extended_capabilities(enc, 0x80000000);
}

/* ----------------------UNIT TESTS------------------------ */

START_TEST(testEncodeIe)
{
	uint8 adBuffer[BUFFER_SIZE];
	bcm_encode_t ad;

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");

	ck_assert_msg(bcm_encode_ie_hotspot_indication2(enc, TRUE, HSPOT_RELEASE_2,
		TRUE, 0x1234, TRUE, 0x5678),
		"bcm_encode_ie_hotspot_indication2 failed");

	ck_assert_msg(bcm_encode_ie_interworking(enc,
		IW_ANT_WILDCARD_NETWORK, FALSE, FALSE, FALSE, FALSE,
		TRUE, 0, 0, (struct ether_addr *)"\xff\xff\xff\xff\xff\xff"),
		"bcm_encode_ie_interworking failed");

	ck_assert_msg(bcm_encode_init(&ad, sizeof(adBuffer), adBuffer),
		"bcm_encode_init failed");
	ck_assert_msg(bcm_encode_ie_advertisement_protocol_tuple(&ad,
		ADVP_PAME_BI_DEPENDENT, 0xff, ADVP_ANQP_PROTOCOL_ID),
		"bcm_encode_ie_advertisement_protocol_tuple failed");
	ck_assert_msg(bcm_encode_ie_advertisement_protocol_from_tuple(enc,
		bcm_encode_length(&ad), bcm_encode_buf(&ad)),
		"bcm_encode_ie_advertisement_protocol_from_tuple failed");

	ck_assert_msg(bcm_encode_ie_roaming_consortium(enc, 0xff, 3, (uint8 *)"\x00\x11\x22",
		4, (uint8 *)"\x55\x66\x77\x88", 4, (uint8 *)"\xaa\xbb\xcc\xdd"),
		"bcm_encode_ie_roaming_consortium failed");

	ck_assert_msg(bcm_encode_ie_extended_capabilities(enc, 0x80000000),
		"bcm_encode_ie_extended_capabilities failed");

	WL_PRPKT("encoded IEs", bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeIe)
{
	bcm_decode_t dec;
	bcm_decode_ie_t ie;
	bcm_decode_t dec1;
	bcm_decode_hotspot_indication_t hotspot;
	bcm_decode_interworking_t interworking;
	bcm_decode_advertisement_protocol_t advertise;
	bcm_decode_roaming_consortium_t roam;
	uint32 cap;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	ck_assert_msg(bcm_decode_ie(&dec, &ie) == 5, "bcm_decode_ie failed");

	ck_assert_msg(bcm_decode_init(&dec1, ie.hotspotIndicationLength,
		ie.hotspotIndication), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_ie_hotspot_indication2(&dec1, &hotspot),
		"bcm_decode_ie_hotspot_indication2 failed");
	ck_assert_msg(hotspot.isDgafDisabled == TRUE, "invalid data");
	ck_assert_msg(hotspot.releaseNumber == HSPOT_RELEASE_2, "invalid data");
	ck_assert_msg(hotspot.isPpsMoIdPresent == TRUE, "invalid data");
	ck_assert_msg(hotspot.ppsMoId == 0x1234, "invalid data");
	ck_assert_msg(hotspot.isAnqpDomainIdPresent == TRUE, "invalid data");
	ck_assert_msg(hotspot.anqpDomainId == 0x5678, "invalid data");

	ck_assert_msg(bcm_decode_init(&dec1, ie.interworkingLength,
		ie.interworking), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_ie_interworking(&dec1, &interworking),
		"bcm_decode_ie_interworking failed");
	ck_assert_msg(interworking.accessNetworkType == IW_ANT_WILDCARD_NETWORK, "invalid data");
	ck_assert_msg(interworking.isInternet == FALSE, "invalid data");
	ck_assert_msg(interworking.isAsra == FALSE, "invalid data");
	ck_assert_msg(interworking.isEsr == FALSE, "invalid data");
	ck_assert_msg(interworking.isUesa == FALSE, "invalid data");
	ck_assert_msg(interworking.isVenue == TRUE, "invalid data");
	ck_assert_msg(interworking.venueGroup == 0, "invalid data");
	ck_assert_msg(interworking.venueType == 0, "invalid data");
	ck_assert_msg(interworking.isHessid == TRUE, "invalid data");
	ck_assert_msg(memcmp(&interworking.hessid, "\xff\xff\xff\xff\xff\xff",
		sizeof(interworking.hessid)) == 0, "invalid data");

	ck_assert_msg(bcm_decode_init(&dec1, ie.advertisementProtocolLength,
		ie.advertisementProtocol), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_ie_advertisement_protocol(&dec1, &advertise),
		"bcm_decode_ie_advertisement_protocol failed");
	ck_assert_msg(advertise.count == 1, "invalid data");
	ck_assert_msg(advertise.protocol[0].queryResponseLimit == 0x7f, "invalid data");
	ck_assert_msg(advertise.protocol[0].isPamebi == FALSE, "invalid data");
	ck_assert_msg(advertise.protocol[0].protocolId == ADVP_ANQP_PROTOCOL_ID, "invalid data");

	ck_assert_msg(bcm_decode_init(&dec1, ie.roamingConsortiumLength,
		ie.roamingConsortium), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_ie_roaming_consortium(&dec1, &roam),
		"pktDecodeRoamingConsortium failed");
	ck_assert_msg(roam.anqpOiCount == 0xff, "invalid data");
	ck_assert_msg(roam.count == 3, "invalid data");
	ck_assert_msg(memcmp(roam.oi[0].data, "\x00\x11\x22", roam.oi[0].length) == 0,
		"invalid data");
	ck_assert_msg(memcmp(roam.oi[1].data, "\x55\x66\x77\x88", roam.oi[1].length) == 0,
		"invalid data");
	ck_assert_msg(memcmp(roam.oi[2].data, "\xaa\xbb\xcc\xdd", roam.oi[2].length) == 0,
		"invalid data");

	ck_assert_msg(bcm_decode_init(&dec1, ie.extendedCapabilityLength,
		ie.extendedCapability), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_ie_extended_capabilities(&dec1, &cap),
		"bcm_decode_ie_extended_capabilities failed");
	ck_assert_msg(cap == 0x80000000, "invalid data");
}
END_TEST

START_TEST(testDecodeCorruptLengthIe)
{
	bcm_decode_t dec;
	bcm_decode_ie_t ie;
	uint8 *lenPtr;
	uint8 save;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	lenPtr = &bcm_decode_buf(&dec)[1];
	save = *lenPtr;
	*lenPtr += 1;
	ck_assert_msg(bcm_decode_ie(&dec, &ie) == 1, "bcm_decode_ie failed");
	*lenPtr = 0xff;
	ck_assert_msg(bcm_decode_ie(&dec, &ie) == 0, "bcm_decode_ie failed");
	*lenPtr = save;
}
END_TEST

Suite *bcm_enc_dec_ie_suite(void)
{
	Suite *s = suite_create("bcm_enc_dec_ie_suite");

	/* Encode & Decode IE */
	TCase *tc_EncIe = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_EncIe, testEncIe_setup, testEncDecIe_teardown);
	tcase_add_test(tc_EncIe, testEncodeIe);

	TCase *tc_DecIe = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_DecIe, testDecIe_setup, testEncDecIe_teardown);
	tcase_add_test(tc_DecIe, testDecodeIe);
	tcase_add_test(tc_DecIe, testDecodeCorruptLengthIe);

	suite_add_tcase(s, tc_EncIe);
	suite_add_tcase(s, tc_DecIe);

	return s;
}
