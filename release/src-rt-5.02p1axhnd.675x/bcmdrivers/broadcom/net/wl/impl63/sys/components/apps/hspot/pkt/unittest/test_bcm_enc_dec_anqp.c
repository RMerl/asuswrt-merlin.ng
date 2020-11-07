/*
 * Test harness for encoding and decoding 802.11u ANQP packets.
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
#include <802.11.h>
#include <p2p.h>
#include <trace.h>
#include <bcm_encode_anqp.h>
#include <bcm_decode_anqp.h>
#include <bcm_encode_hspot_anqp.h>
#include <bcm_decode_hspot_anqp.h>
#include <test_bcm_enc_dec_anqp.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */

#define NO_IE_APPEND	0

#define BUFFER_SIZE		1024
static uint8 *buffer;
static bcm_encode_t *enc;

/* ------------- Setup and Teardown - Fixtures --------------- */

void anqp_setup(void)
{
	/* Creating a mmap for global variables "enc" & "buffer" */
	enc = (bcm_encode_t *)mmap(NULL, sizeof(*enc), PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	buffer = (uint8 *)mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (enc == MAP_FAILED || buffer == MAP_FAILED) {
		printf("Setup function for encode & decode anqp failed\n");
		exit(-1);
	}
}

void anqp_teardown(void)
{
	/* munmap for global variables "enc" & "buffer" */
	int enc_unmap, buffer_unmap;

	enc_unmap = munmap(enc, sizeof(*enc));
	buffer_unmap = munmap(buffer, BUFFER_SIZE);

	if (enc_unmap != 0 || buffer_unmap != 0) {
		printf("Teardown function for encode & decode anqp failed\n");
		exit(-1);
	}
}

/* ----------------------UNIT TESTS------------------------ */

START_TEST(testEncodeQueryList)
{
	uint16 query[] = {
		ANQP_ID_VENUE_NAME_INFO,
		ANQP_ID_NETWORK_AUTHENTICATION_TYPE_INFO,
		ANQP_ID_ROAMING_CONSORTIUM_LIST,
		ANQP_ID_IP_ADDRESS_TYPE_AVAILABILITY_INFO,
		ANQP_ID_NAI_REALM_LIST,
		ANQP_ID_G3PP_CELLULAR_NETWORK_INFO,
		ANQP_ID_DOMAIN_NAME_LIST,
	};

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_query_list(enc, sizeof(query) / sizeof(uint16), query),
	"bcm_encode_anqp_query_list failed");

	WL_PRPKT("testEncodeQueryList",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeQueryList)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_query_list_t queryList;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 1, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.anqpQueryListLength,
		anqp.anqpQueryListBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_query_list(&ie, &queryList),
		"bcm_decode_anqp_query_list failed");
	ck_assert_msg(queryList.queryLen == 7, "invalid data");
	ck_assert_msg(queryList.queryId[0] == ANQP_ID_VENUE_NAME_INFO, "invalid data");
	ck_assert_msg(queryList.queryId[1] == ANQP_ID_NETWORK_AUTHENTICATION_TYPE_INFO,
		"invalid data");
	ck_assert_msg(queryList.queryId[2] == ANQP_ID_ROAMING_CONSORTIUM_LIST, "invalid data");
	ck_assert_msg(queryList.queryId[3] == ANQP_ID_IP_ADDRESS_TYPE_AVAILABILITY_INFO,
		"invalid data");
	ck_assert_msg(queryList.queryId[4] == ANQP_ID_NAI_REALM_LIST, "invalid data");
	ck_assert_msg(queryList.queryId[5] == ANQP_ID_G3PP_CELLULAR_NETWORK_INFO, "invalid data");
	ck_assert_msg(queryList.queryId[6] == ANQP_ID_DOMAIN_NAME_LIST, "invalid data");
}
END_TEST

START_TEST(testEncodeCapabilityList)
{
	uint8 buf[BUFFER_SIZE];
	bcm_encode_t vendor;
	uint8 vendorQuery[] = {1, 2, 3};
	uint16 query[] = {
		ANQP_ID_VENUE_NAME_INFO,
		ANQP_ID_ROAMING_CONSORTIUM_LIST,
		ANQP_ID_NAI_REALM_LIST,
		ANQP_ID_G3PP_CELLULAR_NETWORK_INFO,
		ANQP_ID_DOMAIN_NAME_LIST,
		ANQP_ID_EMERGENCY_NAI};

	ck_assert_msg(bcm_encode_init(&vendor, BUFFER_SIZE, buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_hspot_anqp_capability_list(&vendor,
	sizeof(vendorQuery) / sizeof(uint8), vendorQuery),
	"bcm_encode_hspot_anqp_capability_list failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_capability_list(enc, sizeof(query) / sizeof(uint16), query,
	bcm_encode_length(&vendor), bcm_encode_buf(&vendor)),
	"bcm_encode_anqp_capability_list failed");

	WL_PRPKT("testEncodeCapabilityList",
	bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeCapabilityList)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_capability_list_t capList;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 2, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.anqpCapabilityListLength,
		anqp.anqpCapabilityListBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_capability_list(&ie, &capList),
		"bcm_decode_anqp_capability_list failed");
	ck_assert_msg(capList.capLen == 6, "invalid data");
	ck_assert_msg(capList.capId[0] == ANQP_ID_VENUE_NAME_INFO, "invalid data");
	ck_assert_msg(capList.capId[1] == ANQP_ID_ROAMING_CONSORTIUM_LIST, "invalid data");
	ck_assert_msg(capList.capId[2] == ANQP_ID_NAI_REALM_LIST, "invalid data");
	ck_assert_msg(capList.capId[3] == ANQP_ID_G3PP_CELLULAR_NETWORK_INFO, "invalid data");
	ck_assert_msg(capList.capId[4] == ANQP_ID_DOMAIN_NAME_LIST, "invalid data");
	ck_assert_msg(capList.capId[5] == ANQP_ID_EMERGENCY_NAI, "invalid data");
	ck_assert_msg(capList.hspotCapList.capLen == 3, "invalid data");
	ck_assert_msg(capList.hspotCapList.capId[0] == 1, "invalid data");
	ck_assert_msg(capList.hspotCapList.capId[1] == 2, "invalid data");
	ck_assert_msg(capList.hspotCapList.capId[2] == 3, "invalid data");
}
END_TEST

START_TEST(testEncodeVenueName)
{
	uint8 buf[BUFFER_SIZE];
	bcm_encode_t duple;

	ck_assert_msg(bcm_encode_init(&duple, BUFFER_SIZE, buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_venue_duple(&duple, 2, "EN", 6, "myname"),
		"bcm_encode_anqp_venue_duple failed");
	ck_assert_msg(bcm_encode_anqp_venue_duple(&duple, 2, "FR", 10, "helloworld"),
		"bcm_encode_anqp_venue_duple failed");
	ck_assert_msg(bcm_encode_anqp_venue_duple(&duple, 5, "JAPAN", 6, "yrname"),
		"pktEncodeAnqpOperatorNameDuple failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_venue_name(enc, VENUE_BUSINESS, 7,
		bcm_encode_length(&duple), bcm_encode_buf(&duple)),
		"bcm_encode_anqp_venue_name failed");

	WL_PRPKT("testEncodeVenueName",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeVenueName)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_venue_name_t venueName;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 3, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.venueNameInfoLength,
		anqp.venueNameInfoBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_venue_name(&ie, &venueName),
		"bcm_decode_anqp_venue_name failed");
	ck_assert_msg(venueName.group == VENUE_BUSINESS, "invalid data");
	ck_assert_msg(venueName.type == 7, "invalid data");
	ck_assert_msg(venueName.numVenueName == 3, "invalid data");
	ck_assert_msg(strcmp(venueName.venueName[0].lang, "EN") == 0, "invalid data");
	ck_assert_msg(strcmp(venueName.venueName[0].name, "myname") == 0, "invalid data");
	ck_assert_msg(strcmp(venueName.venueName[1].lang, "FR") == 0, "invalid data");
	ck_assert_msg(strcmp(venueName.venueName[1].name, "helloworld") == 0, "invalid data");
	ck_assert_msg(strcmp(venueName.venueName[2].lang, "JAP") == 0, "invalid data");
	ck_assert_msg(strcmp(venueName.venueName[2].name, "yrname") == 0, "invalid data");
}
END_TEST

START_TEST(testEncodeNetworkAuthenticationType)
{
	uint8 buf[BUFFER_SIZE];
	bcm_encode_t network;

	ck_assert_msg(bcm_encode_init(&network, BUFFER_SIZE, buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_network_authentication_unit(&network,
		NATI_ONLINE_ENROLLMENT_SUPPORTED, 5, "myurl"),
		"bcm_encode_anqp_network_authentication_unit failed");
	ck_assert_msg(bcm_encode_anqp_network_authentication_unit(&network,
		NATI_ONLINE_ENROLLMENT_SUPPORTED, 5, "yrurl"),
		"bcm_encode_anqp_network_authentication_unit failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_network_authentication_type(enc,
		bcm_encode_length(&network), bcm_encode_buf(&network)),
		"bcm_encode_anqp_network_authentication_type failed");

	WL_PRPKT("testEncodeNetworkAuthenticationType",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeNetworkAuthenticationType)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_network_authentication_type_t auth;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 4, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.networkAuthenticationTypeInfoLength,
		anqp.networkAuthenticationTypeInfoBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_network_authentication_type(&ie, &auth),
		"bcm_decode_anqp_network_authentication_type failed");
	ck_assert_msg(auth.numAuthenticationType == 2, "invalid data");
	ck_assert_msg(auth.unit[0].type == NATI_ONLINE_ENROLLMENT_SUPPORTED, "invalid data");
	ck_assert_msg(strcmp((char *)auth.unit[0].url, "myurl") == 0, "invalid data");
	ck_assert_msg(auth.unit[1].type == NATI_ONLINE_ENROLLMENT_SUPPORTED, "invalid data");
	ck_assert_msg(strcmp((char *)auth.unit[1].url, "yrurl") == 0, "invalid data");
}
END_TEST

START_TEST(testEncodeRoamingConsortium)
{
	uint8 buf[BUFFER_SIZE];
	bcm_encode_t oi;

	ck_assert_msg(bcm_encode_init(&oi, BUFFER_SIZE, buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_oi_duple(&oi, 4, (uint8 *)"\x00\x11\x22\x33"),
		"bcm_encode_anqp_oi_duple failed");
	ck_assert_msg(bcm_encode_anqp_oi_duple(&oi, 3, (uint8 *)"\x12\x34\x56"),
		"bcm_encode_anqp_oi_duple failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_roaming_consortium(enc,
		bcm_encode_length(&oi), bcm_encode_buf(&oi)),
		"bcm_encode_anqp_roaming_consortium failed");

	WL_PRPKT("testEncodeRoamingConsortium",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeRoamingConsortium)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_roaming_consortium_t roam;
	bcm_decode_anqp_oi_duple_t oi1 = {3, "\x11\x22\x33"};
	bcm_decode_anqp_oi_duple_t oi2 = {3, "\x12\x34\x56"};

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 5, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.roamingConsortiumListLength,
		anqp.roamingConsortiumListBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_roaming_consortium(&ie, &roam),
		"bcm_decode_anqp_roaming_consortium failed");
	ck_assert_msg(roam.numOi == 2, "invalid data");
	ck_assert_msg(memcmp((char *)roam.oi[0].oi, "\x00\x11\x22\x33",
		roam.oi[0].oiLen) == 0, "invalid data");
	ck_assert_msg(memcmp((char *)roam.oi[1].oi, "\x12\x34\x56",
		roam.oi[1].oiLen) == 0, "invalid data");

	ck_assert_msg(bcm_decode_anqp_is_roaming_consortium(&roam, &oi1) == FALSE,
		"bcm_decode_anqp_is_roaming_consortium failed");
	ck_assert_msg(bcm_decode_anqp_is_roaming_consortium(&roam, &oi2) == TRUE,
		"bcm_decode_anqp_is_roaming_consortium failed");
}
END_TEST

START_TEST(testEncodeIpAddressType)
{
#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_ip_type_availability(enc,
		IPA_IPV6_AVAILABLE, IPA_IPV4_PORT_RESTRICT_SINGLE_NAT),
		"bcm_encode_anqp_ip_type_availability failed");

	WL_PRPKT("testEncodeIpAddressType",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeIpAddressType)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_ip_type_t type;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 6, "bcm_decode_anqp failed");
	ck_assert_msg(bcm_decode_init(&ie, anqp.ipAddressTypeAvailabilityInfoLength,
		anqp.ipAddressTypeAvailabilityInfoBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_ip_type_availability(&ie, &type),
		"bcm_decode_anqp_ip_type_availability failed");
	ck_assert_msg(type.ipv6 == IPA_IPV6_AVAILABLE, "invalid data");
	ck_assert_msg(type.ipv4 == IPA_IPV4_PORT_RESTRICT_SINGLE_NAT, "invalid data");
}
END_TEST

START_TEST(testEncodeNaiRealm)
{
	uint8 credential = REALM_SIM;
	int numAuth = 2;
	uint8 authBuf[BUFFER_SIZE];
	bcm_encode_t auth;
	int numEap = 3;
	uint8 eapBuf[BUFFER_SIZE];
	bcm_encode_t eap;
	int numRealm = 2;
	uint8 realmBuf[BUFFER_SIZE];
	bcm_encode_t realm;
	int i;

	ck_assert_msg(bcm_encode_init(&auth, BUFFER_SIZE, authBuf), "bcm_encode_init failed");
	for (i = 0; i < numAuth; i++) {
		ck_assert_msg(bcm_encode_anqp_authentication_subfield(&auth,
			REALM_CREDENTIAL, sizeof(credential), &credential),
			"bcm_encode_anqp_authentication_subfield failed");
	}

	ck_assert_msg(bcm_encode_init(&eap, BUFFER_SIZE, eapBuf), "bcm_encode_init failed");
	for (i = 0; i < numEap; i++) {
		ck_assert_msg(bcm_encode_anqp_eap_method_subfield(&eap, REALM_EAP_SIM,
			numAuth, bcm_encode_length(&auth), bcm_encode_buf(&auth)),
			"bcm_encode_anqp_eap_method_subfield failed");
	}

	ck_assert_msg(bcm_encode_init(&realm, BUFFER_SIZE, realmBuf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_nai_realm_data(&realm, REALM_ENCODING_RFC4282,
		31, (uint8 *)"helloworld;myworld.com;test.com", numEap,
		bcm_encode_length(&eap), bcm_encode_buf(&eap)),
		"bcm_encode_anqp_nai_realm_data failed");
	ck_assert_msg(bcm_encode_anqp_nai_realm_data(&realm, REALM_ENCODING_RFC4282,
		11, (uint8 *)"hotspot.com", numEap,
		bcm_encode_length(&eap), bcm_encode_buf(&eap)),
		"bcm_encode_anqp_nai_realm_data failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_nai_realm(enc, numRealm,
		bcm_encode_length(&realm), bcm_encode_buf(&realm)),
		"bcm_encode_anqp_nai_realm failed");

	WL_PRPKT("testEncodeNaiRealm",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeNaiRealm)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_nai_realm_list_t realm;
	int i, j, k;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 7, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.naiRealmListLength,
		anqp.naiRealmListBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_nai_realm(&ie, &realm),
		"bcm_decode_anqp_nai_realm failed");

	ck_assert_msg(realm.realmCount == 2, "invalid data");

	for (i = 0; i < realm.realmCount; i++) {
		ck_assert_msg(realm.realm[i].encoding == 0, "invalid data");
		if (i == 0)
			ck_assert_msg(strcmp((char *)realm.realm[i].realm,
				"helloworld;myworld.com;test.com") == 0, "invalid data");
		else
			ck_assert_msg(strcmp((char *)realm.realm[i].realm,
				"hotspot.com") == 0, "invalid data");
		ck_assert_msg(realm.realm[i].eapCount == 3, "invalid data");
		for (j = 0; j < realm.realm[i].eapCount; j++) {
			ck_assert_msg(realm.realm[i].eap[j].eapMethod == REALM_EAP_SIM,
				"invalid data");
			ck_assert_msg(realm.realm[i].eap[j].authCount == 2, "invalid data");
			for (k = 0; k < realm.realm[i].eap[j].authCount; k++) {
				ck_assert_msg(realm.realm[i].eap[j].auth[k].id ==
					REALM_CREDENTIAL, "invalid data");
				ck_assert_msg(realm.realm[i].eap[j].auth[k].value[0] ==
					REALM_SIM, "invalid data");
			}
		}
	}

	ck_assert_msg(bcm_decode_anqp_is_realm(&realm, "hotspot.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	ck_assert_msg(bcm_decode_anqp_is_realm(&realm, "helloworld", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	ck_assert_msg(bcm_decode_anqp_is_realm(&realm, "myworld.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	ck_assert_msg(bcm_decode_anqp_is_realm(&realm, "test.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	ck_assert_msg(!bcm_decode_anqp_is_realm(&realm, "missing.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
}
END_TEST

START_TEST(testEncode3GppCellularNetwork)
{
	uint8 plmnBuf[BUFFER_SIZE];
	bcm_encode_t plmn;

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

	ck_assert_msg(bcm_encode_init(&plmn, BUFFER_SIZE, plmnBuf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_plmn(&plmn, "310", "026"), "bcm_encode_anqp_plmn failed");
	ck_assert_msg(bcm_encode_anqp_plmn(&plmn, "208", "00"), "bcm_encode_anqp_plmn failed");
	ck_assert_msg(bcm_encode_anqp_plmn(&plmn, "208", "01"), "bcm_encode_anqp_plmn failed");
	ck_assert_msg(bcm_encode_anqp_plmn(&plmn, "208", "02"), "bcm_encode_anqp_plmn failed");
	ck_assert_msg(bcm_encode_anqp_plmn(&plmn, "450", "02"), "bcm_encode_anqp_plmn failed");
	ck_assert_msg(bcm_encode_anqp_plmn(&plmn, "450", "04"), "bcm_encode_anqp_plmn failed");

	ck_assert_msg(bcm_encode_anqp_3gpp_cellular_network(enc, 6,
		bcm_encode_length(&plmn), bcm_encode_buf(&plmn)),
		"bcm_encode_anqp_3gpp_cellular_network failed");

	WL_PRPKT("testEncode3GppCellularNetwork",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecode3GppCellularNetwork)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_3gpp_cellular_network_t g3pp;
	bcm_decode_anqp_plmn_t plmn;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 8, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.g3ppCellularNetworkInfoLength,
		anqp.g3ppCellularNetworkInfoBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_3gpp_cellular_network(&ie, &g3pp),
		"bcm_decode_anqp_3gpp_cellular_network failed");
	ck_assert_msg(g3pp.plmnCount == 6, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[0].mcc, "310") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[0].mnc, "026") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[1].mcc, "208") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[1].mnc, "00") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[2].mcc, "208") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[2].mnc, "01") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[3].mcc, "208") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[3].mnc, "02") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[4].mcc, "450") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[4].mnc, "02") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[5].mcc, "450") == 0, "invalid data");
	ck_assert_msg(strcmp(g3pp.plmn[5].mnc, "04") == 0, "invalid data");

	strncpy(plmn.mcc, "310", sizeof(plmn.mcc));
	strncpy(plmn.mnc, "026", sizeof(plmn.mnc));
	ck_assert_msg(bcm_decode_anqp_is_3gpp(&g3pp, &plmn), "bcm_decode_anqp_is_3gpp failed");

	strncpy(plmn.mcc, "208", sizeof(plmn.mcc));
	strncpy(plmn.mnc, "02", sizeof(plmn.mnc));
	ck_assert_msg(bcm_decode_anqp_is_3gpp(&g3pp, &plmn), "bcm_decode_anqp_is_3gpp failed");

	strncpy(plmn.mnc, "03", sizeof(plmn.mnc));
	ck_assert_msg(bcm_decode_anqp_is_3gpp(&g3pp, &plmn) == FALSE,
		"bcm_decode_anqp_is_3gpp failed");
}
END_TEST

START_TEST(testEncodeDomainNameList)
{
	uint8 nameBuf[BUFFER_SIZE];
	bcm_encode_t name;

	ck_assert_msg(bcm_encode_init(&name, BUFFER_SIZE, nameBuf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_domain_name(&name, 17, "my.helloworld.com"),
		"bcm_encode_anqp_domain_name failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_domain_name_list(enc,
		bcm_encode_length(&name), bcm_encode_buf(&name)),
		"bcm_encode_anqp_domain_name_list failed");

	WL_PRPKT("testEncodeDomainNameList",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeDomainNameList)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_domain_name_list_t list;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 9, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.domainNameListLength,
		anqp.domainNameListBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_domain_name_list(&ie, &list),
		"bcm_decode_anqp_domain_name_list failed");
	ck_assert_msg(list.numDomain == 1, "invalid data");
	ck_assert_msg(strcmp(list.domain[0].name, "my.helloworld.com") == 0, "invalid data");

	ck_assert_msg(bcm_decode_anqp_is_domain_name(&list, "my.helloworld.com", FALSE),
		"bcm_decode_anqp_is_domain_name failed");
	ck_assert_msg(!bcm_decode_anqp_is_domain_name(&list, "world", TRUE),
		"bcm_decode_anqp_is_domain_name failed");
	ck_assert_msg(!bcm_decode_anqp_is_domain_name(&list, "hello", TRUE),
		"bcm_decode_anqp_is_domain_name failed");
	ck_assert_msg(bcm_decode_anqp_is_domain_name(&list, "helloworld.com", TRUE),
		"bcm_decode_anqp_is_domain_name failed");
	ck_assert_msg(!bcm_decode_anqp_is_domain_name(&list, "nomatch", FALSE),
		"bcm_decode_anqp_is_domain_name failed");
	ck_assert_msg(!bcm_decode_anqp_is_domain_name(&list, "nomatch", TRUE),
		"bcm_decode_anqp_is_domain_name failed");
}
END_TEST

START_TEST(testEncodeQueryVendorSpecific)
{
#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	ck_assert_msg(bcm_encode_anqp_wfa_service_discovery(enc, 3,
		10, (uint8 *)"helloworld"),
		"bcm_encode_anqp_wfa_service_discovery failed");

	WL_PRPKT("testEncodeQueryVendorSpecific",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeQueryVendorSpecific)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	uint16 serviceUpdateIndicator;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 10, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.wfaServiceDiscoveryLength,
		anqp.wfaServiceDiscoveryBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_wfa_service_discovery(&ie, &serviceUpdateIndicator),
		"bcm_decode_anqp_wfa_service_discovery failed");
	ck_assert_msg(serviceUpdateIndicator == 3, "invalid data");
	ck_assert_msg(bcm_decode_remaining(&ie) == 10, "invalid data");
	if (bcm_decode_current_ptr(&ie) != 0) {
		ck_assert_msg(memcmp(bcm_decode_current_ptr(&ie), "helloworld", 10) == 0,
			"invalid data");
	}
}
END_TEST

START_TEST(testEncodeQueryRequestVendorSpecific)
{
	uint8 queryBuf[BUFFER_SIZE];
	bcm_encode_t query;

	ck_assert_msg(bcm_encode_init(&query, BUFFER_SIZE, queryBuf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_query_request_vendor_specific_tlv(&query,
		SVC_RPOTYPE_UPNP, 1, 12, (uint8 *)"queryrequest"),
		"bcm_encode_anqp_query_request_vendor_specific_tlv failed");
	ck_assert_msg(bcm_encode_anqp_query_request_vendor_specific_tlv(&query,
		SVC_RPOTYPE_BONJOUR, 2, 12, (uint8 *)"queryrequest"),
		"bcm_encode_anqp_query_request_vendor_specific_tlv failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

	ck_assert_msg(bcm_encode_anqp_wfa_service_discovery(enc, 0,
		bcm_encode_length(&query), bcm_encode_buf(&query)),
		"bcm_encode_anqp_wfa_service_discovery failed");

	WL_PRPKT("testEncodeQueryRequestVendorSpecific",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeQueryRequestVendorSpecific)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	uint16 serviceUpdateIndicator;
	bcm_decode_anqp_query_request_vendor_specific_tlv_t request;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 11, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.wfaServiceDiscoveryLength,
		anqp.wfaServiceDiscoveryBuffer), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp_wfa_service_discovery(&ie, &serviceUpdateIndicator),
		"bcm_decode_anqp_wfa_service_discovery failed");
	ck_assert_msg(serviceUpdateIndicator == 0, "invalid data");

	ck_assert_msg(bcm_decode_anqp_query_request_vendor_specific_tlv(&ie, &request),
		"bcm_decode_anqp_query_request_vendor_specific_tlv failed");
	ck_assert_msg(request.serviceProtocolType == SVC_RPOTYPE_UPNP, "invalid data");
	ck_assert_msg(request.serviceTransactionId == 1, "invalid data");
	ck_assert_msg(request.dataLen == 12, "invalid data");
	if (request.data != 0) {
		ck_assert_msg(memcmp(request.data, "queryrequest", 12) == 0, "invalid data");
	}

	ck_assert_msg(bcm_decode_anqp_query_request_vendor_specific_tlv(&ie, &request),
		"bcm_decode_anqp_query_request_vendor_specific_tlv failed");
	ck_assert_msg(request.serviceProtocolType == SVC_RPOTYPE_BONJOUR, "invalid data");
	ck_assert_msg(request.serviceTransactionId == 2, "invalid data");
	ck_assert_msg(request.dataLen == 12, "invalid data");
	if (request.data != 0) {
		ck_assert_msg(memcmp(request.data, "queryrequest", 12) == 0, "invalid data");
	}
}
END_TEST

START_TEST(testEncodeQueryResponseVendorSpecific)
{
	uint8 queryBuf[BUFFER_SIZE];
	bcm_encode_t query;

	ck_assert_msg(bcm_encode_init(&query, BUFFER_SIZE, queryBuf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_query_response_vendor_specific_tlv(&query,
		SVC_RPOTYPE_UPNP, 1, 0, FALSE, 0, 13, (uint8 *)"queryresponse"),
		"bcm_encode_anqp_query_response_vendor_specific_tlv failed");
	ck_assert_msg(bcm_encode_anqp_query_response_vendor_specific_tlv(&query,
		SVC_RPOTYPE_BONJOUR, 2, 0, FALSE, 0, 13, (uint8 *)"queryresponse"),
		"bcm_encode_anqp_query_response_vendor_specific_tlv failed");

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

	ck_assert_msg(bcm_encode_anqp_wfa_service_discovery(enc, 0,
		bcm_encode_length(&query), bcm_encode_buf(&query)),
		"bcm_encode_anqp_wfa_service_discovery failed");

	WL_PRPKT("testEncodeQueryResponseVendorSpecific",
		bcm_encode_buf(enc), bcm_encode_length(enc));
}
END_TEST

START_TEST(testDecodeQueryResponseVendorSpecific)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	uint16 serviceUpdateIndicator;
	bcm_decode_anqp_query_response_vendor_specific_tlv_t response;

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 12, "bcm_decode_anqp failed");

	ck_assert_msg(bcm_decode_init(&ie, anqp.wfaServiceDiscoveryLength,
		anqp.wfaServiceDiscoveryBuffer), "bcm_decode_init failed");

	ck_assert_msg(bcm_decode_anqp_wfa_service_discovery(&ie, &serviceUpdateIndicator),
		"bcm_decode_anqp_wfa_service_discovery failed");
	ck_assert_msg(serviceUpdateIndicator == 0, "invalid data");

	ck_assert_msg(bcm_decode_anqp_query_response_vendor_specific_tlv(&ie, FALSE, &response),
		"bcm_decode_anqp_query_response_vendor_specific_tlv failed");
	ck_assert_msg(response.serviceProtocolType == SVC_RPOTYPE_UPNP, "invalid data");
	ck_assert_msg(response.serviceTransactionId == 1, "invalid data");
	ck_assert_msg(response.statusCode == 0, "invalid data");
	ck_assert_msg(response.dataLen == 13, "invalid data");
	if (response.data != 0) {
		ck_assert_msg(memcmp(response.data, "queryresponse", 13) == 0, "invalid data");
	}

	ck_assert_msg(bcm_decode_anqp_query_response_vendor_specific_tlv(&ie, FALSE, &response),
		"bcm_decode_anqp_query_response_vendor_specific_tlv failed");
	ck_assert_msg(response.serviceProtocolType == SVC_RPOTYPE_BONJOUR, "invalid data");
	ck_assert_msg(response.serviceTransactionId == 2, "invalid data");
	ck_assert_msg(response.statusCode == 0, "invalid data");
	ck_assert_msg(response.dataLen == 13, "invalid data");
	if (response.data != 0) {
		ck_assert_msg(memcmp(response.data, "queryresponse", 13) == 0, "invalid data");
	}
}
END_TEST

START_TEST(testEncodeHspotAnqp1)
{
	uint8 data[8];
	int i;

	for (i = 0; i < 8; i++)
		data[i] = i;

#if NO_IE_APPEND
	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

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

		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name, 2, "EN", 6, "myname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name, 2, "FR", 10, "helloworld"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		ck_assert_msg(bcm_encode_hspot_anqp_operator_name_duple(&name, 5, "JAPAN", 6, "yrname"),
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
}
END_TEST

START_TEST(testEmpty3GppCellularNetwork)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_3gpp_cellular_network_t g3pp;

	ck_assert_msg(bcm_encode_init(enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_3gpp_cellular_network(enc, 0, 0, 0),
		"bcm_encode_anqp_3gpp_cellular_network failed");
	WL_PRPKT("testEncode3GppCellularNetwork",
		bcm_encode_buf(enc), bcm_encode_length(enc));

	ck_assert_msg(bcm_decode_init(&dec, bcm_encode_length(enc),
		bcm_encode_buf(enc)), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp(&dec, &anqp) == 1, "bcm_decode_anqp failed");
	ck_assert_msg(bcm_decode_init(&ie, anqp.g3ppCellularNetworkInfoLength,
		anqp.g3ppCellularNetworkInfoBuffer), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp_3gpp_cellular_network(&ie, &g3pp),
		"bcm_decode_anqp_3gpp_cellular_network failed");
}
END_TEST

START_TEST(testWfdsRequest)
{
	char *serviceName1 = "org.wi-fi.wfds.print";
	char *serviceInfoReq1 = "dlna:local:all";
	char *serviceName2 = "org.wi-fi.wfds.play";
	char *serviceInfoReq2 = "dlna:local:players";
	uint8 enc1Buf[BUFFER_SIZE];
	bcm_encode_t enc1;
	uint8 enc2Buf[BUFFER_SIZE];
	bcm_encode_t enc2;
	uint8 enc3Buf[BUFFER_SIZE];
	bcm_encode_t enc3;
	bcm_decode_t dec1;
	bcm_decode_anqp_t anqp;
	bcm_decode_t dec2;
	uint16 serviceUpdateIndicator;
	bcm_decode_anqp_query_request_vendor_specific_tlv_t request;
	bcm_decode_t dec3;
	int count;

	/* encode multiple service request */
	ck_assert_msg(bcm_encode_init(&enc1, sizeof(enc1Buf), enc1Buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_wfds_request(&enc1, strlen(serviceName1), (uint8 *)serviceName1,
		strlen(serviceInfoReq1), (uint8 *)serviceInfoReq1),
		"bcm_encode_anqp_wfds_request failed");
	ck_assert_msg(bcm_encode_anqp_wfds_request(&enc1, strlen(serviceName2), (uint8 *)serviceName2,
		strlen(serviceInfoReq2), (uint8 *)serviceInfoReq2),
		"bcm_encode_anqp_wfds_request failed");

	ck_assert_msg(bcm_encode_init(&enc2, sizeof(enc2Buf), enc2Buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_query_request_vendor_specific_tlv(&enc2,
		SVC_RPOTYPE_WFDS, 1, bcm_encode_length(&enc1), bcm_encode_buf(&enc1)),
		"bcm_encode_anqp_query_request_vendor_specific_tlv failed");

	ck_assert_msg(bcm_encode_init(&enc3, sizeof(enc3Buf), enc3Buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_wfa_service_discovery(&enc3, 0x1234,
		bcm_encode_length(&enc2), bcm_encode_buf(&enc2)),
		"bcm_encode_anqp_wfa_service_discovery failed");

	WL_PRPKT("WFDS request", bcm_encode_buf(&enc3), bcm_encode_length(&enc3));

	/* decode request */
	ck_assert_msg(bcm_decode_init(&dec1, bcm_encode_length(&enc3), bcm_encode_buf(&enc3)),
		"bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp(&dec1, &anqp), "bcm_decode_anqp failed");
	ck_assert_msg(bcm_decode_init(&dec2, anqp.wfaServiceDiscoveryLength,
		anqp.wfaServiceDiscoveryBuffer), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp_wfa_service_discovery(&dec2, &serviceUpdateIndicator),
		"bcm_decode_anqp_wfa_service_discovery failed");
	ck_assert_msg(serviceUpdateIndicator == 0x1234, "invalid data");
	ck_assert_msg(bcm_decode_anqp_query_request_vendor_specific_tlv(&dec2, &request),
		"bcm_decode_anqp_query_request_vendor_specific_tlv failed");
	ck_assert_msg(request.serviceProtocolType == SVC_RPOTYPE_WFDS, "invalid data");
	ck_assert_msg(request.serviceTransactionId == 1, "invalid data");

	ck_assert_msg(bcm_decode_init(&dec3, request.dataLen, request.data),
		"bcm_decode_init failed");

	count = 0;
	while (bcm_decode_remaining(&dec3) > 0) {
		int ret;
		bcm_decode_anqp_wfds_request_t wfds;
		char *serviceName;
		char *serviceInfoReq;

		ret = bcm_decode_anqp_wfds_request(&dec3, &wfds);
		ck_assert_msg(ret, "bcm_decode_anqp_wfds_request failed");
		if (!ret) {
			break;
		}
		if (count == 0) {
			serviceName = serviceName1;
			serviceInfoReq = serviceInfoReq1;
		}
		else {
			serviceName = serviceName2;
			serviceInfoReq = serviceInfoReq2;
		}
		ck_assert_msg(wfds.serviceNameLen == strlen(serviceName), "invalid data");
		ck_assert_msg(memcmp(wfds.serviceName, serviceName, wfds.serviceNameLen) == 0,
			"invalid data");
		ck_assert_msg(wfds.serviceInfoReqLen == strlen(serviceInfoReq), "invalid data");
		ck_assert_msg(memcmp(wfds.serviceInfoReq, serviceInfoReq, wfds.serviceInfoReqLen) == 0,
			"invalid data");
		WL_PRPKT("WFDS service name", wfds.serviceName, wfds.serviceNameLen);
		WL_PRPKT("WFDS service info request", wfds.serviceInfoReq, wfds.serviceInfoReqLen);
		count++;
	}
}
END_TEST

START_TEST(testWfdsResponse)
{
	char *serviceName1 = "org.wi-fi.wfds.print";
	char *serviceInfo1 = "hello world";
	char *serviceName2 = "org.wi-fi.wfds.play";
	char *serviceInfo2 = "wonderful world";
	uint8 enc1Buf[BUFFER_SIZE];
	bcm_encode_t enc1;
	uint8 enc2Buf[BUFFER_SIZE];
	bcm_encode_t enc2;
	uint8 enc3Buf[BUFFER_SIZE];
	bcm_encode_t enc3;
	bcm_decode_t dec1;
	bcm_decode_anqp_t anqp;
	bcm_decode_t dec2;
	uint16 serviceUpdateIndicator;
	bcm_decode_anqp_query_response_vendor_specific_tlv_t response;
	bcm_decode_t dec3;
	int count;

	/* encode response */
	ck_assert_msg(bcm_encode_init(&enc1, sizeof(enc1Buf), enc1Buf), "bcm_encode_init failed");
	count = 0;
	ck_assert_msg(bcm_encode_anqp_wfds_response(&enc1, 0x11223344, 0xaabb,
		strlen(serviceName1), (uint8 *)serviceName1, 1,
		strlen(serviceInfo1), (uint8 *)serviceInfo1),
		"bcm_encode_anqp_wfds_response failed");
	count++;
	ck_assert_msg(bcm_encode_anqp_wfds_response(&enc1, 0x55667788, 0xccdd,
		strlen(serviceName2), (uint8 *)serviceName2, 1,
		strlen(serviceInfo2), (uint8 *)serviceInfo2),
		"bcm_encode_anqp_wfds_response failed");
	count++;

	ck_assert_msg(bcm_encode_init(&enc2, sizeof(enc2Buf), enc2Buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_query_response_vendor_specific_tlv(&enc2,
		SVC_RPOTYPE_WFDS, 2, 1, TRUE, count,
		bcm_encode_length(&enc1), bcm_encode_buf(&enc1)),
		"bcm_encode_anqp_query_request_vendor_specific_tlv failed");

	ck_assert_msg(bcm_encode_init(&enc3, sizeof(enc3Buf), enc3Buf), "bcm_encode_init failed");
	ck_assert_msg(bcm_encode_anqp_wfa_service_discovery(&enc3, 0x1234,
		bcm_encode_length(&enc2), bcm_encode_buf(&enc2)),
		"bcm_encode_anqp_wfa_service_discovery failed");

	WL_PRPKT("WFDS request", bcm_encode_buf(&enc3), bcm_encode_length(&enc3));

	/* decode response */
	ck_assert_msg(bcm_decode_init(&dec1, bcm_encode_length(&enc3), bcm_encode_buf(&enc3)),
		"bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp(&dec1, &anqp), "bcm_decode_anqp failed");
	ck_assert_msg(bcm_decode_init(&dec2, anqp.wfaServiceDiscoveryLength,
		anqp.wfaServiceDiscoveryBuffer), "bcm_decode_init failed");
	ck_assert_msg(bcm_decode_anqp_wfa_service_discovery(&dec2, &serviceUpdateIndicator),
		"bcm_decode_anqp_wfa_service_discovery failed");
	ck_assert_msg(serviceUpdateIndicator == 0x1234, "invalid data");
	ck_assert_msg(bcm_decode_anqp_query_response_vendor_specific_tlv(&dec2, TRUE, &response),
		"bcm_decode_anqp_query_request_vendor_specific_tlv failed");
	ck_assert_msg(response.serviceProtocolType == SVC_RPOTYPE_WFDS, "invalid data");
	ck_assert_msg(response.serviceTransactionId == 2, "invalid data");
	ck_assert_msg(response.statusCode == 1, "invalid data");
	ck_assert_msg(response.numService == count, "invalid data");

	ck_assert_msg(bcm_decode_init(&dec3, response.dataLen, response.data),
		"bcm_decode_init failed");

	count = 0;
	while (bcm_decode_remaining(&dec3) > 0) {
		int ret;
		bcm_decode_anqp_wfds_response_t wfds;
		uint32 advertisementId;
		uint16 configMethod;
		char *serviceName;
		uint8 serviceStatus;
		char *serviceInfo;

		ret = bcm_decode_anqp_wfds_response(&dec3, &wfds);
		ck_assert_msg(ret, "bcm_decode_anqp_wfds_response failed");
		if (!ret) {
			break;
		}
		if (count == 0) {
			advertisementId = 0x11223344;
			configMethod = 0xaabb;
			serviceName = serviceName1;
			serviceStatus = 1;
			serviceInfo = serviceInfo1;
		}
		else {
			advertisementId = 0x55667788;
			configMethod = 0xccdd;
			serviceName = serviceName2;
			serviceStatus = 1;
			serviceInfo = serviceInfo2;
		}
		ck_assert_msg(wfds.advertisementId == advertisementId, "invalid data");
		ck_assert_msg(wfds.configMethod == configMethod, "invalid data");
		ck_assert_msg(wfds.serviceNameLen == strlen(serviceName), "invalid data");
		ck_assert_msg(memcmp(wfds.serviceName, serviceName, wfds.serviceNameLen) == 0,
			"invalid data");
		ck_assert_msg(wfds.serviceStatus == serviceStatus, "invalid data");
		ck_assert_msg(wfds.serviceInfoLen == strlen(serviceInfo), "invalid data");
		ck_assert_msg(memcmp(wfds.serviceInfo, serviceInfo, wfds.serviceInfoLen) == 0,
			"invalid data");
		WL_PRPKT("WFDS service name", wfds.serviceName, wfds.serviceNameLen);
		WL_PRPKT("WFDS service info", wfds.serviceInfo, wfds.serviceInfoLen);
		count++;
	}
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *bcm_enc_dec_anqp_suite(void)
{
	Suite *s = suite_create("bcm_enc_dec_anqp_suite");

	/* Encode & Decode QOS Map */
	TCase *tc_enc_dec_anqp = tcase_create("Test Case");
	tcase_add_unchecked_fixture(tc_enc_dec_anqp, anqp_setup, anqp_teardown);
	tcase_add_test(tc_enc_dec_anqp, testEncodeQueryList);
	tcase_add_test(tc_enc_dec_anqp, testDecodeQueryList);

	tcase_add_test(tc_enc_dec_anqp, testEncodeCapabilityList);
	tcase_add_test(tc_enc_dec_anqp, testDecodeCapabilityList);

	tcase_add_test(tc_enc_dec_anqp, testEncodeVenueName);
	tcase_add_test(tc_enc_dec_anqp, testDecodeVenueName);

	tcase_add_test(tc_enc_dec_anqp, testEncodeNetworkAuthenticationType);
	tcase_add_test(tc_enc_dec_anqp, testDecodeNetworkAuthenticationType);

	tcase_add_test(tc_enc_dec_anqp, testEncodeRoamingConsortium);
	tcase_add_test(tc_enc_dec_anqp, testDecodeRoamingConsortium);

	tcase_add_test(tc_enc_dec_anqp, testEncodeIpAddressType);
	tcase_add_test(tc_enc_dec_anqp, testDecodeIpAddressType);

	tcase_add_test(tc_enc_dec_anqp, testEncodeNaiRealm);
	tcase_add_test(tc_enc_dec_anqp, testDecodeNaiRealm);

	tcase_add_test(tc_enc_dec_anqp, testEncode3GppCellularNetwork);
	tcase_add_test(tc_enc_dec_anqp, testDecode3GppCellularNetwork);

	tcase_add_test(tc_enc_dec_anqp, testEncodeDomainNameList);
	tcase_add_test(tc_enc_dec_anqp, testDecodeDomainNameList);

	tcase_add_test(tc_enc_dec_anqp, testEncodeQueryVendorSpecific);
	tcase_add_test(tc_enc_dec_anqp, testDecodeQueryVendorSpecific);

	tcase_add_test(tc_enc_dec_anqp, testEncodeQueryRequestVendorSpecific);
	tcase_add_test(tc_enc_dec_anqp, testDecodeQueryRequestVendorSpecific);

	tcase_add_test(tc_enc_dec_anqp, testEncodeQueryResponseVendorSpecific);
	tcase_add_test(tc_enc_dec_anqp, testDecodeQueryResponseVendorSpecific);

	tcase_add_test(tc_enc_dec_anqp, testEncodeHspotAnqp1);

	tcase_add_test(tc_enc_dec_anqp, testEmpty3GppCellularNetwork);

	tcase_add_test(tc_enc_dec_anqp, testWfdsRequest);
	tcase_add_test(tc_enc_dec_anqp, testWfdsResponse);

	suite_add_tcase(s, tc_enc_dec_anqp);

	return s;
}
