/*
 * Test harness for encoding and decoding Hotspot2.0 ANQP packets.
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
#include <sys/mman.h>
#include <trace.h>
#include <bcm_encode_hspot_anqp.h>
#include <bcm_decode_hspot_anqp.h>
#include <test_bcm_enc_dec_hspot_anqp.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */

#define BUFFER_SIZE		256
static uint8 *buffer;
static bcm_encode_t *enc;

#define LANG_ENG	"eng"
#define LANG_CHI	"chi"
#define LANG_JPN	"jpn"

/* ------------- Setup and Teardown - Fixtures --------------- */

void hspot_anqp_setup(void)
{
	/* Creating a mmap for global variables "enc" & "buffer" */
	enc = (bcm_encode_t *)mmap(NULL, sizeof(*enc), PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	buffer = (uint8 *)mmap(NULL, BUFFER_SIZE * 4, PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (enc == MAP_FAILED || buffer == MAP_FAILED) {
		printf("Setup function for encode & decode hspot anqp failed\n");
		exit(-1);
	}
}

void hspot_anqp_teardown(void)
{
	/* munmap for global variables "enc" & "buffer" */
	int enc_unmap, buffer_unmap;

	enc_unmap = munmap(enc, sizeof(*enc));
	buffer_unmap = munmap(buffer, BUFFER_SIZE * 4);

	if (enc_unmap != 0 || buffer_unmap != 0) {
		printf("Teardown function for encode & decode hspot anqp failed\n");
		exit(-1);
	}
}

/* ----------------------UNIT TESTS------------------------ */

START_TEST(testOsuProviderList)
{
	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;
		uint8 iconBuf[BUFFER_SIZE];
		bcm_encode_t icon;
		uint8 osuBuf[BUFFER_SIZE];
		uint8 desc1Buf[BUFFER_SIZE];
		bcm_encode_t desc1;
		uint8 desc2Buf[BUFFER_SIZE];
		bcm_encode_t desc2;
		bcm_encode_t osu;
		uint8 soap = HSPOT_OSU_METHOD_SOAP_XML;
		uint8 omadm = HSPOT_OSU_METHOD_OMA_DM;

		ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE * 4, buffer),
			"bcm_encode_init failed");

		ck_assert_msg(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name,
			strlen(LANG_ENG), LANG_ENG, 6, "myname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name,
			strlen(LANG_CHI), LANG_CHI, 6, "yrname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		ck_assert_msg(bcm_encode_init(&icon, BUFFER_SIZE, iconBuf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_icon_metadata(&icon,
			1, 2, LANG_ENG,
			4, (uint8 *)"text", 13, (uint8 *)"iconfile1.txt"),
			"bcm_encode_hspot_anqp_icon_metadata failed");
		ck_assert_msg(bcm_encode_hspot_anqp_icon_metadata(&icon,
			3, 4, LANG_CHI,
			4, (uint8 *)"text", 13, (uint8 *)"iconfile2.txt"),
			"bcm_encode_hspot_anqp_icon_metadata failed");

		ck_assert_msg(bcm_encode_init(&desc1, BUFFER_SIZE, desc1Buf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&desc1,
			strlen(LANG_ENG), LANG_ENG, 12, "SOAP-XML OSU"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		ck_assert_msg(bcm_encode_init(&desc2, BUFFER_SIZE, desc2Buf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&desc2,
			strlen(LANG_ENG), LANG_ENG, 10, "OMA-DM OSU"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		ck_assert_msg(bcm_encode_init(&osu, BUFFER_SIZE, osuBuf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_osu_provider(&osu,
			bcm_encode_length(&name), bcm_encode_buf(&name),
			6, (uint8 *)"myuri1",
			1, &soap,
			bcm_encode_length(&icon), bcm_encode_buf(&icon),
			15, (uint8 *)"myprovider1.com",
			bcm_encode_length(&desc1), bcm_encode_buf(&desc1)),
			"bcm_encode_hspot_anqp_osu_provider failed");
		ck_assert_msg(bcm_encode_hspot_anqp_osu_provider(&osu,
			bcm_encode_length(&name), bcm_encode_buf(&name),
			6, (uint8 *)"myuri2",
			1, &omadm,
			bcm_encode_length(&icon), bcm_encode_buf(&icon),
			0, 0,
			bcm_encode_length(&desc2), bcm_encode_buf(&desc2)),
			"bcm_encode_hspot_anqp_osu_provider failed");

		ck_assert_msg(bcm_encode_hspot_anqp_osu_provider_list(enc,
			8, (uint8 *)"OSU SSID", 2,
			bcm_encode_length(&osu), bcm_encode_buf(&osu)),
			"bcm_encode_hspot_anqp_osu_provider_list failed");
		WL_PRPKT("hotspot OSU provider list",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}
	{
		bcm_decode_t dec;
		bcm_decode_hspot_anqp_t hspot;
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_osu_provider_list_t list;
		bcm_decode_hspot_anqp_osu_provider_t *p;
		bcm_decode_hspot_anqp_name_duple_t *duple;

		ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
			bcm_encode_buf(enc)), "bcm_decode_init failed");
		WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

		ck_assert_msg(bcm_decode_hspot_anqp(&dec, TRUE, &hspot) == 1,
			"bcm_decode_hspot_anqp failed");

		ck_assert_msg(bcm_decode_init(&ie, hspot.onlineSignupProvidersLength,
			hspot.onlineSignupProvidersBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_osu_provider_list(&ie, &list),
			"bcm_decode_hspot_anqp_osu_provider_list failed");

		ck_assert_msg(list.osuProviderCount == 2, "invalid data");

		ck_assert_msg(list.osuProvider[0].name.numName == 2, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[0].lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[0].name, "myname") == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[1].lang, LANG_CHI) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[1].name, "yrname") == 0, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].nai, "myprovider1.com") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].uri, "myuri1") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[0].methodLength == 1, "invalid data");
		ck_assert_msg(list.osuProvider[0].method[0] == HSPOT_OSU_METHOD_SOAP_XML, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadataCount == 2, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[0].width == 1, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[0].height == 2, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[0].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[0].filename,
			"iconfile1.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[1].width == 3, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[1].height == 4, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[1].lang, LANG_CHI) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[1].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[1].filename,
			"iconfile2.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[0].desc.numName == 1, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].desc.duple[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].desc.duple[0].name, "SOAP-XML OSU") == 0,
			"invalid data");

		ck_assert_msg(list.osuProvider[1].name.numName == 2, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[0].lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[0].name, "myname") == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[1].lang, LANG_CHI) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[1].name, "yrname") == 0, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].nai, "") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].uri, "myuri2") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[1].methodLength == 1, "invalid data");
		ck_assert_msg(list.osuProvider[1].method[0] == HSPOT_OSU_METHOD_OMA_DM, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadataCount == 2, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[0].width == 1, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[0].height == 2, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[0].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[0].filename,
			"iconfile1.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[1].width == 3, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[1].height == 4, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[1].lang, LANG_CHI) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[1].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[1].filename,
			"iconfile2.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[1].desc.numName == 1, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].desc.duple[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].desc.duple[0].name, "OMA-DM OSU") == 0,
			"invalid data");

		ck_assert_msg(bcm_decode_hspot_anqp_find_osu_ssid_provider(&list, 8, "OSU SSID",
			strlen(LANG_ENG), LANG_ENG, 6, "myname",
			TRUE, HSPOT_OSU_METHOD_SOAP_XML, &duple),
			"bcm_decode_hspot_anqp_find_osu_ssid_provider failed");
		ck_assert_msg(strcmp(duple->lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(duple->name, "myname") == 0, "invalid data");
		ck_assert_msg(!bcm_decode_hspot_anqp_find_osu_ssid_provider(&list, 8, "OSU SSID",
			strlen(LANG_JPN), LANG_JPN, 6, "myname",
			TRUE, HSPOT_OSU_METHOD_SOAP_XML, &duple),
			"bcm_decode_hspot_anqp_find_osu_ssid_provider failed");
		ck_assert_msg(bcm_decode_hspot_anqp_find_osu_ssid_provider(&list, 12, "missing SSID",
			strlen(LANG_ENG), LANG_ENG, 6, "myname",
			TRUE, HSPOT_OSU_METHOD_SOAP_XML, &duple) == 0,
			"bcm_decode_hspot_anqp_find_osu_ssid_provider failed");
		ck_assert_msg(strcmp(duple->lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(duple->name, "myname") == 0, "invalid data");

		/* find any provider */
		ck_assert_msg((p = bcm_decode_hspot_anqp_find_osu_provider(&list,
			0, 0, 0, 0, FALSE, 0, &duple)) != 0,
			"bcm_decode_hspot_anqp_find_osu_provider failed");
		if (p != 0) {
		ck_assert_msg(strcmp(duple->lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(duple->name, "myname") == 0, "invalid data");
		ck_assert_msg(strcmp(p->name.duple[0].lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(p->name.duple[0].name, "myname") == 0, "invalid data");
		ck_assert_msg(strcmp(p->name.duple[1].lang, LANG_CHI) == 0, "invalid data");
		ck_assert_msg(strcmp(p->name.duple[1].name, "yrname") == 0, "invalid data");
		ck_assert_msg(strcmp((const char *)p->nai, "myprovider1.com") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)p->uri, "myuri1") == 0, "invalid data");
		ck_assert_msg(p->methodLength == 1, "invalid data");
		ck_assert_msg(p->method[0] == HSPOT_OSU_METHOD_SOAP_XML, "invalid data");
		ck_assert_msg(p->iconMetadataCount == 2, "invalid data");
		ck_assert_msg(p->iconMetadata[0].width == 1, "invalid data");
		ck_assert_msg(p->iconMetadata[0].height == 2, "invalid data");
		ck_assert_msg(strcmp((const char *)p->iconMetadata[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)p->iconMetadata[0].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)p->iconMetadata[0].filename,
			"iconfile1.txt") == 0, "invalid data");
		ck_assert_msg(p->iconMetadata[1].width == 3, "invalid data");
		ck_assert_msg(p->iconMetadata[1].height == 4, "invalid data");
		ck_assert_msg(strcmp((const char *)p->iconMetadata[1].lang, LANG_CHI) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)p->iconMetadata[1].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)p->iconMetadata[1].filename,
			"iconfile2.txt") == 0, "invalid data");
		ck_assert_msg(p->desc.numName == 1, "invalid data");
		ck_assert_msg(strcmp(p->desc.duple[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp(p->desc.duple[0].name, "SOAP-XML OSU") == 0,
			"invalid data");
		}
	}
}
END_TEST

START_TEST(testEncodeHspotAnqp)
{
	uint8 data[8];
	int i;

	for (i = 0; i < 8; i++)
		data[i] = i;

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE * 4, buffer), "bcm_encode_init failed");

	ck_assert_msg(bcm_encode_hspot_anqp_query_list(enc, 8, data),
		"bcm_encode_hspot_anqp_query_list failed");
	WL_PRPKT("hotspot query list",
		bcm_encode_buf(enc), bcm_encode_length(enc));

	ck_assert_msg(bcm_encode_hspot_anqp_capability_list(enc, 8, data),
		"bcm_encode_hspot_anqp_capability_list failed");
	WL_PRPKT("hotspot capability list",
		bcm_encode_buf(enc), bcm_encode_length(enc));

	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;

		ck_assert_msg(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");

		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name,
			strlen(LANG_ENG), LANG_ENG, 6, "myname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name,
			strlen(LANG_ENG), LANG_ENG, 10, "helloworld"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name,
			strlen(LANG_JPN), LANG_JPN, 6, "yrname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		ck_assert_msg(bcm_encode_hspot_anqp_operator_friendly_name(enc,
			bcm_encode_length(&name), bcm_encode_buf(&name)),
			"bcm_encode_hspot_anqp_operator_friendly_name failed");
		WL_PRPKT("hotspot operator friendly name",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}

	ck_assert_msg(bcm_encode_hspot_anqp_wan_metrics(enc,
		HSPOT_WAN_LINK_TEST, HSPOT_WAN_SYMMETRIC_LINK, HSPOT_WAN_AT_CAPACITY,
		0x12345678, 0x11223344, 0xaa, 0xbb, 0xcdef),
		"bcm_encode_hspot_anqp_capability_list failed");
	WL_PRPKT("hotspot WAN metrics",
		bcm_encode_buf(enc), bcm_encode_length(enc));

	{
		uint8 capBuf[BUFFER_SIZE];
		bcm_encode_t cap;

		ck_assert_msg(bcm_encode_init(&cap, BUFFER_SIZE, capBuf), "bcm_encode_init failed");

		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 1, 0, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 20, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 22, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 80, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 443, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 1723, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 5060, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 500, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 5060, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 4500, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");

		ck_assert_msg(bcm_encode_hspot_anqp_connection_capability(enc,
			bcm_encode_length(&cap), bcm_encode_buf(&cap)),
			"bcm_encode_hspot_anqp_connection_capability failed");
		WL_PRPKT("hotspot connection capability",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}

	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;

		ck_assert_msg(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");

		ck_assert_msg(bcm_encode_hspot_anqp_nai_home_realm_name(&name, 0, 5, "hello"),
			"bcm_encode_hspot_anqp_nai_home_realm_name failed");
		ck_assert_msg(bcm_encode_hspot_anqp_nai_home_realm_name(&name, 1, 5, "world"),
			"bcm_encode_hspot_anqp_nai_home_realm_name failed");

		ck_assert_msg(pktEncodeHspotAnqpNaiHomeRealmQuery(enc, 2,
			bcm_encode_length(&name), bcm_encode_buf(&name)),
			"pktEncodeHspotAnqpNaiHomeRealmQuery failed");
		WL_PRPKT("hotspot NAI home realm query",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}

	{
		/* Testing range of operating classes */
		uint8 opClass [35] = {80, 81, 82, 83, 84, 94, 95, 96, 101, 102, 103, 104, 105, 106,
			107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
		122, 123, 124, 125, 126, 127};

		ck_assert_msg(bcm_encode_hspot_anqp_operating_class_indication(enc, 35, opClass),
			"bcm_encode_hspot_anqp_operating_class_indication failed");
		WL_PRPKT("hotspot operating class indication",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}

	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;
		uint8 iconBuf[BUFFER_SIZE];
		bcm_encode_t icon;
		uint8 osuBuf[BUFFER_SIZE];
		uint8 desc1Buf[BUFFER_SIZE];
		bcm_encode_t desc1;
		uint8 desc2Buf[BUFFER_SIZE];
		bcm_encode_t desc2;
		bcm_encode_t osu;
		uint8 soap = HSPOT_OSU_METHOD_SOAP_XML;
		uint8 omadm = HSPOT_OSU_METHOD_OMA_DM;

		ck_assert_msg(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name,
			strlen(LANG_ENG), LANG_ENG, 6, "myname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name,
			strlen(LANG_CHI), LANG_CHI, 6, "yrname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		ck_assert_msg(bcm_encode_init(&icon, BUFFER_SIZE, iconBuf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_icon_metadata(&icon,
			1, 2, LANG_ENG,
			4, (uint8 *)"text", 13, (uint8 *)"iconfile1.txt"),
			"bcm_encode_hspot_anqp_icon_metadata failed");
		ck_assert_msg(bcm_encode_hspot_anqp_icon_metadata(&icon,
			3, 4, LANG_CHI,
			4, (uint8 *)"text", 13, (uint8 *)"iconfile2.txt"),
			"bcm_encode_hspot_anqp_icon_metadata failed");

		ck_assert_msg(bcm_encode_init(&desc1, BUFFER_SIZE, desc1Buf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&desc1,
			strlen(LANG_ENG), LANG_ENG, 12, "SOAP-XML OSU"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		ck_assert_msg(bcm_encode_init(&desc2, BUFFER_SIZE, desc2Buf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&desc2,
			strlen(LANG_ENG), LANG_ENG, 10, "OMA-DM OSU"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		ck_assert_msg(bcm_encode_init(&osu, BUFFER_SIZE, osuBuf),
			"bcm_encode_init failed");
		ck_assert_msg(bcm_encode_hspot_anqp_osu_provider(&osu,
			bcm_encode_length(&name), bcm_encode_buf(&name),
			6, (uint8 *)"myuri1",
			1, &soap,
			bcm_encode_length(&icon), bcm_encode_buf(&icon),
			15, (uint8 *)"myprovider1.com",
			bcm_encode_length(&desc1), bcm_encode_buf(&desc1)),
			"bcm_encode_hspot_anqp_osu_provider failed");
		ck_assert_msg(bcm_encode_hspot_anqp_osu_provider(&osu,
			bcm_encode_length(&name), bcm_encode_buf(&name),
			6, (uint8 *)"myuri2",
			1, &omadm,
			bcm_encode_length(&icon), bcm_encode_buf(&icon),
			0, 0,
			bcm_encode_length(&desc2), bcm_encode_buf(&desc2)),
			"bcm_encode_hspot_anqp_osu_provider failed");

		ck_assert_msg(bcm_encode_hspot_anqp_osu_provider_list(enc,
			8, (uint8 *)"OSU SSID", 2,
			bcm_encode_length(&osu), bcm_encode_buf(&osu)),
			"bcm_encode_hspot_anqp_osu_provider_list failed");
		WL_PRPKT("hotspot OSU provider list",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}

	{
		ck_assert_msg(bcm_encode_hspot_anqp_anonymous_nai(enc, 13, (uint8 *)"anonymous.com"),
			"bcm_encode_hspot_anqp_anonymous_nai failed");
		WL_PRPKT("hotspot anonymous NAI",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}

	{
		ck_assert_msg(bcm_encode_hspot_anqp_icon_request(enc, 12, (uint8 *)"iconfile.txt"),
			"bcm_encode_hspot_anqp_icon_request failed");
		WL_PRPKT("hotspot icon request",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}

	{
		ck_assert_msg(bcm_encode_hspot_anqp_icon_binary_file(enc, HSPOT_ICON_STATUS_SUCCESS,
			4, (uint8 *)"text", 14, (uint8 *)"iconbinarydata"),
			"bcm_encode_hspot_anqp_icon_binary_file failed");
		WL_PRPKT("hotspot icon binary file",
			bcm_encode_buf(enc), bcm_encode_length(enc));
	}
}
END_TEST

START_TEST(testDecodeHspotAnqp)
{
	bcm_decode_t dec;
	bcm_decode_hspot_anqp_t hspot;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	ck_assert_msg(bcm_decode_hspot_anqp(&dec, TRUE, &hspot) == 11, "bcm_decode_hspot_anqp failed");

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_query_list_t queryList;

		ck_assert_msg(bcm_decode_init(&ie, hspot.queryListLength,
			hspot.queryListBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_query_list(&ie, &queryList),
			"bcm_decode_hspot_anqp_query_list failed");
		ck_assert_msg(queryList.queryLen == 8, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_capability_list_t capList;

		ck_assert_msg(bcm_decode_init(&ie, hspot.capabilityListLength,
			hspot.capabilityListBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_capability_list(&ie, &capList),
			"bcm_decode_hspot_anqp_capability_list failed");
		ck_assert_msg(capList.capLen == 8, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_wan_metrics_t wanMetrics;

		ck_assert_msg(bcm_decode_init(&ie, hspot.wanMetricsLength,
			hspot.wanMetricsBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_wan_metrics(&ie, &wanMetrics),
			"pktHspotAnqpDecodeWanMetrics failed");
		ck_assert_msg(wanMetrics.linkStatus == HSPOT_WAN_LINK_TEST, "invalid data");
		ck_assert_msg(wanMetrics.symmetricLink == HSPOT_WAN_SYMMETRIC_LINK, "invalid data");
		ck_assert_msg(wanMetrics.atCapacity == HSPOT_WAN_AT_CAPACITY, "invalid data");
		ck_assert_msg(wanMetrics.dlinkSpeed == 0x12345678, "invalid data");
		ck_assert_msg(wanMetrics.ulinkSpeed == 0x11223344, "invalid data");
		ck_assert_msg(wanMetrics.dlinkLoad == 0xaa, "invalid data");
		ck_assert_msg(wanMetrics.ulinkLoad == 0xbb, "invalid data");
		ck_assert_msg(wanMetrics.lmd == 0xcdef, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_operator_friendly_name_t op;

		ck_assert_msg(bcm_decode_init(&ie, hspot.operatorFriendlyNameLength,
			hspot.operatorFriendlyNameBuffer), "bcm_decode_init failed");

		ck_assert_msg(bcm_decode_hspot_anqp_operator_friendly_name(&ie, &op),
			"bcm_decode_hspot_anqp_operator_friendly_name failed");
		ck_assert_msg(op.numName == 3, "invalid data");

		ck_assert_msg(op.duple[0].langLen == strlen(LANG_ENG), "invalid data");
		ck_assert_msg(strcmp(op.duple[0].lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(op.duple[0].nameLen == 6, "invalid data");
		ck_assert_msg(strcmp(op.duple[0].name, "myname") == 0, "invalid data");

		ck_assert_msg(op.duple[1].langLen == strlen(LANG_ENG), "invalid data");
		ck_assert_msg(strcmp(op.duple[1].lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(op.duple[1].nameLen == 10, "invalid data");
		ck_assert_msg(strcmp(op.duple[1].name, "helloworld") == 0, "invalid data");

		ck_assert_msg(op.duple[2].langLen == strlen(LANG_JPN), "invalid data");
		ck_assert_msg(strcmp(op.duple[2].lang, LANG_JPN) == 0, "invalid data");
		ck_assert_msg(op.duple[2].nameLen == 6, "invalid data");
		ck_assert_msg(strcmp(op.duple[2].name, "yrname") == 0, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_connection_capability_t cap;

		ck_assert_msg(bcm_decode_init(&ie, hspot.connectionCapabilityLength,
			hspot.connectionCapabilityBuffer), "bcm_decode_init failed");

		ck_assert_msg(bcm_decode_hspot_anqp_connection_capability(&ie, &cap),
			"pktDecodeHspotAnqpAnqpConnectionCapability failed");
		ck_assert_msg(cap.numConnectCap == 10, "invalid data");

		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 1, 0),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 6, 20),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 6, 22),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 6, 80),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 6, 443),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 6, 1723),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 6, 5060),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 17, 500),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 17, 5060),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(bcm_decode_hspot_anqp_is_connection_capability(&cap, 17, 4500),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
		ck_assert_msg(!bcm_decode_hspot_anqp_is_connection_capability(&cap, 100, 100),
			"bcm_decode_hspot_anqp_is_connection_capability failed");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_nai_home_realm_query_t realm;

		ck_assert_msg(bcm_decode_init(&ie, hspot.naiHomeRealmQueryLength,
			hspot.naiHomeRealmQueryBuffer), "bcm_decode_init failed");

		ck_assert_msg(bcm_decode_hspot_anqp_nai_home_realm_query(&ie, &realm),
			"bcm_decode_hspot_anqp_nai_home_realm_query failed");
		ck_assert_msg(realm.count == 2, "invalid data");

		ck_assert_msg(realm.data[0].encoding == 0, "invalid data");
		ck_assert_msg(realm.data[0].nameLen == 5, "invalid data");
		ck_assert_msg(strcmp(realm.data[0].name, "hello") == 0, "invalid data");

		ck_assert_msg(realm.data[1].encoding == 1, "invalid data");
		ck_assert_msg(realm.data[1].nameLen == 5, "invalid data");
		ck_assert_msg(strcmp(realm.data[1].name, "world") == 0, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_operating_class_indication_t opClassList;

		ck_assert_msg(bcm_decode_init(&ie, hspot.opClassIndicationLength,
			hspot.opClassIndicationBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_operating_class_indication(&ie, &opClassList),
			"pktDecodeHspotOperatingClassIndication failed");
		ck_assert_msg(opClassList.opClassLen == 35, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_osu_provider_list_t list;

		ck_assert_msg(bcm_decode_init(&ie, hspot.onlineSignupProvidersLength,
			hspot.onlineSignupProvidersBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_osu_provider_list(&ie, &list),
			"bcm_decode_hspot_anqp_osu_provider_list failed");

		ck_assert_msg(list.osuProviderCount == 2, "invalid data");

		ck_assert_msg(list.osuProvider[0].name.numName == 2, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[0].lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[0].name, "myname") == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[1].lang, LANG_CHI) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].name.duple[1].name, "yrname") == 0, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].nai, "myprovider1.com") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].uri, "myuri1") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[0].methodLength == 1, "invalid data");
		ck_assert_msg(list.osuProvider[0].method[0] == HSPOT_OSU_METHOD_SOAP_XML, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadataCount == 2, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[0].width == 1, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[0].height == 2, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[0].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[0].filename,
			"iconfile1.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[1].width == 3, "invalid data");
		ck_assert_msg(list.osuProvider[0].iconMetadata[1].height == 4, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[1].lang, LANG_CHI) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[1].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[0].iconMetadata[1].filename,
			"iconfile2.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[0].desc.numName == 1, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].desc.duple[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp(list.osuProvider[0].desc.duple[0].name, "SOAP-XML OSU") == 0,
			"invalid data");

		ck_assert_msg(list.osuProvider[1].name.numName == 2, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[0].lang, LANG_ENG) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[0].name, "myname") == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[1].lang, LANG_CHI) == 0, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].name.duple[1].name, "yrname") == 0, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].nai, "") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].uri, "myuri2") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[1].methodLength == 1, "invalid data");
		ck_assert_msg(list.osuProvider[1].method[0] == HSPOT_OSU_METHOD_OMA_DM, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadataCount == 2, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[0].width == 1, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[0].height == 2, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[0].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[0].filename,
			"iconfile1.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[1].width == 3, "invalid data");
		ck_assert_msg(list.osuProvider[1].iconMetadata[1].height == 4, "invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[1].lang, LANG_CHI) == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[1].type, "text") == 0,
			"invalid data");
		ck_assert_msg(strcmp((const char *)list.osuProvider[1].iconMetadata[1].filename,
			"iconfile2.txt") == 0, "invalid data");
		ck_assert_msg(list.osuProvider[1].desc.numName == 1, "invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].desc.duple[0].lang, LANG_ENG) == 0,
			"invalid data");
		ck_assert_msg(strcmp(list.osuProvider[1].desc.duple[0].name, "OMA-DM OSU") == 0,
			"invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_anonymous_nai_t anonymous;

		ck_assert_msg(bcm_decode_init(&ie, hspot.anonymousNaiLength,
			hspot.anonymousNaiBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_anonymous_nai(&ie, &anonymous),
			"bcm_decode_hspot_anqp_anonymous_nai failed");
		ck_assert_msg(strcmp(anonymous.nai, "anonymous.com") == 0, "invalid data");
		ck_assert_msg(anonymous.naiLen == 13, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_icon_request_t request;

		ck_assert_msg(bcm_decode_init(&ie, hspot.iconRequestLength,
			hspot.iconRequestBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_icon_request(&ie, &request),
			"bcm_decode_hspot_anqp_icon_request failed");
		ck_assert_msg(strcmp(request.filename, "iconfile.txt") == 0, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_icon_binary_file_t icon;

		ck_assert_msg(bcm_decode_init(&ie, hspot.iconBinaryFileLength,
			hspot.iconBinaryFileBuffer), "bcm_decode_init failed");
		ck_assert_msg(bcm_decode_hspot_anqp_icon_binary_file(&ie, &icon),
			"bcm_decode_hspot_anqp_icon_binary_file failed");
		ck_assert_msg(icon.status == HSPOT_ICON_STATUS_SUCCESS, "invalid data");
		ck_assert_msg(strcmp((const char *)icon.type, "text") == 0, "invalid data");
		ck_assert_msg(strcmp((char *)icon.binary, "iconbinarydata") == 0, "invalid data");
	}
}
END_TEST

START_TEST(testDecodeCorruptLength)
{
	bcm_decode_t dec;
	bcm_decode_hspot_anqp_t hspot;
	uint8 *lenPtr;
	uint8 save;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	lenPtr = &bcm_decode_buf(&dec)[2];
	save = *lenPtr;
	*lenPtr += 1;
	ck_assert_msg(bcm_decode_hspot_anqp(&dec, TRUE, &hspot) == 1, "bcm_decode_hspot_anqp failed");
	*lenPtr = 0x02;
	ck_assert_msg(bcm_decode_hspot_anqp(&dec, TRUE, &hspot) == 0, "bcm_decode_hspot_anqp failed");
	*lenPtr = save;
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *bcm_enc_dec_hspot_anqp_suite(void)
{
	Suite *s = suite_create("bcm_enc_dec_hspot_anqp_suite");

	/* Encode & Decode QOS Map */
	TCase *tc_enc_dec_hspot_anqp = tcase_create("Test Case");
	tcase_add_unchecked_fixture(tc_enc_dec_hspot_anqp, hspot_anqp_setup, hspot_anqp_teardown);

	tcase_add_test(tc_enc_dec_hspot_anqp, testOsuProviderList);
	tcase_add_test(tc_enc_dec_hspot_anqp, testEncodeHspotAnqp);
	tcase_add_test(tc_enc_dec_hspot_anqp, testDecodeHspotAnqp);
	tcase_add_test(tc_enc_dec_hspot_anqp, testDecodeCorruptLength);

	suite_add_tcase(s, tc_enc_dec_hspot_anqp);

	return s;
}
