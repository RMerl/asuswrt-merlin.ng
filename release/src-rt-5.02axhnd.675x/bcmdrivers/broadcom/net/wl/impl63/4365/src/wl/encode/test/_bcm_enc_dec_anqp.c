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
#include "proto/802.11.h"
#include "proto/p2p.h"
#include "test.h"
#include "trace.h"
#include "bcm_encode_anqp.h"
#include "bcm_decode_anqp.h"
#include "bcm_encode_hspot_anqp.h"
#include "bcm_decode_hspot_anqp.h"

TEST_DECLARE();

#define NO_IE_APPEND	0

#define BUFFER_SIZE		1024
static uint8 buffer[BUFFER_SIZE];
static bcm_encode_t enc;

/* --------------------------------------------------------------- */

static void testEncodeQueryList(void)
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

	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_query_list(&enc, sizeof(query) / sizeof(uint16), query),
	"bcm_encode_anqp_query_list failed");

	WL_PRPKT("testEncodeQueryList",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeQueryList(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_query_list_t queryList;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 1, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.anqpQueryListLength,
		anqp.anqpQueryListBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_query_list(&ie, &queryList), "bcm_decode_anqp_query_list failed");
	TEST(queryList.queryLen == 7, "invalid data");
	TEST(queryList.queryId[0] == ANQP_ID_VENUE_NAME_INFO, "invalid data");
	TEST(queryList.queryId[1] == ANQP_ID_NETWORK_AUTHENTICATION_TYPE_INFO, "invalid data");
	TEST(queryList.queryId[2] == ANQP_ID_ROAMING_CONSORTIUM_LIST, "invalid data");
	TEST(queryList.queryId[3] == ANQP_ID_IP_ADDRESS_TYPE_AVAILABILITY_INFO, "invalid data");
	TEST(queryList.queryId[4] == ANQP_ID_NAI_REALM_LIST, "invalid data");
	TEST(queryList.queryId[5] == ANQP_ID_G3PP_CELLULAR_NETWORK_INFO, "invalid data");
	TEST(queryList.queryId[6] == ANQP_ID_DOMAIN_NAME_LIST, "invalid data");
}

static void testEncodeCapabilityList(void)
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

	TEST(bcm_encode_init(&vendor, BUFFER_SIZE, buf), "bcm_encode_init failed");
	TEST(bcm_encode_hspot_anqp_capability_list(&vendor,
	sizeof(vendorQuery) / sizeof(uint8), vendorQuery),
	"bcm_encode_hspot_anqp_capability_list failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_capability_list(&enc, sizeof(query) / sizeof(uint16), query,
	bcm_encode_length(&vendor), bcm_encode_buf(&vendor)),
	"bcm_encode_anqp_capability_list failed");

	WL_PRPKT("testEncodeCapabilityList",
	bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeCapabilityList(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_capability_list_t capList;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 2, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.anqpCapabilityListLength,
		anqp.anqpCapabilityListBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_capability_list(&ie, &capList),
		"bcm_decode_anqp_capability_list failed");
	TEST(capList.capLen == 6, "invalid data");
	TEST(capList.capId[0] == ANQP_ID_VENUE_NAME_INFO, "invalid data");
	TEST(capList.capId[1] == ANQP_ID_ROAMING_CONSORTIUM_LIST, "invalid data");
	TEST(capList.capId[2] == ANQP_ID_NAI_REALM_LIST, "invalid data");
	TEST(capList.capId[3] == ANQP_ID_G3PP_CELLULAR_NETWORK_INFO, "invalid data");
	TEST(capList.capId[4] == ANQP_ID_DOMAIN_NAME_LIST, "invalid data");
	TEST(capList.capId[5] == ANQP_ID_EMERGENCY_NAI, "invalid data");
	TEST(capList.hspotCapList.capLen == 3, "invalid data");
	TEST(capList.hspotCapList.capId[0] == 1, "invalid data");
	TEST(capList.hspotCapList.capId[1] == 2, "invalid data");
	TEST(capList.hspotCapList.capId[2] == 3, "invalid data");
}

static void testEncodeVenueName(void)
{
	uint8 buf[BUFFER_SIZE];
	bcm_encode_t duple;

	TEST(bcm_encode_init(&duple, BUFFER_SIZE, buf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_venue_duple(&duple, 2, "EN", 6, "myname"),
		"bcm_encode_anqp_venue_duple failed");
	TEST(bcm_encode_anqp_venue_duple(&duple, 2, "FR", 10, "helloworld"),
		"bcm_encode_anqp_venue_duple failed");
	TEST(bcm_encode_anqp_venue_duple(&duple, 5, "JAPAN", 6, "yrname"),
		"pktEncodeAnqpOperatorNameDuple failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_venue_name(&enc, VENUE_BUSINESS, 7,
		bcm_encode_length(&duple), bcm_encode_buf(&duple)),
		"bcm_encode_anqp_venue_name failed");

	WL_PRPKT("testEncodeVenueName",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeVenueName(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_venue_name_t venueName;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 3, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.venueNameInfoLength,
		anqp.venueNameInfoBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_venue_name(&ie, &venueName),
		"bcm_decode_anqp_venue_name failed");
	TEST(venueName.group == VENUE_BUSINESS, "invalid data");
	TEST(venueName.type == 7, "invalid data");
	TEST(venueName.numVenueName == 3, "invalid data");
	TEST(strcmp(venueName.venueName[0].lang, "EN") == 0, "invalid data");
	TEST(strcmp(venueName.venueName[0].name, "myname") == 0, "invalid data");
	TEST(strcmp(venueName.venueName[1].lang, "FR") == 0, "invalid data");
	TEST(strcmp(venueName.venueName[1].name, "helloworld") == 0, "invalid data");
	TEST(strcmp(venueName.venueName[2].lang, "JAP") == 0, "invalid data");
	TEST(strcmp(venueName.venueName[2].name, "yrname") == 0, "invalid data");
}

static void testEncodeNetworkAuthenticationType(void)
{
	uint8 buf[BUFFER_SIZE];
	bcm_encode_t network;

	TEST(bcm_encode_init(&network, BUFFER_SIZE, buf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_network_authentication_unit(&network,
		NATI_ONLINE_ENROLLMENT_SUPPORTED, 5, "myurl"),
		"bcm_encode_anqp_network_authentication_unit failed");
	TEST(bcm_encode_anqp_network_authentication_unit(&network,
		NATI_ONLINE_ENROLLMENT_SUPPORTED, 5, "yrurl"),
		"bcm_encode_anqp_network_authentication_unit failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_network_authentication_type(&enc,
		bcm_encode_length(&network), bcm_encode_buf(&network)),
		"bcm_encode_anqp_network_authentication_type failed");

	WL_PRPKT("testEncodeNetworkAuthenticationType",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeNetworkAuthenticationType(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_network_authentication_type_t auth;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 4, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.networkAuthenticationTypeInfoLength,
		anqp.networkAuthenticationTypeInfoBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_network_authentication_type(&ie, &auth),
		"bcm_decode_anqp_network_authentication_type failed");
	TEST(auth.numAuthenticationType == 2, "invalid data");
	TEST(auth.unit[0].type == NATI_ONLINE_ENROLLMENT_SUPPORTED, "invalid data");
	TEST(strcmp((char *)auth.unit[0].url, "myurl") == 0, "invalid data");
	TEST(auth.unit[1].type == NATI_ONLINE_ENROLLMENT_SUPPORTED, "invalid data");
	TEST(strcmp((char *)auth.unit[1].url, "yrurl") == 0, "invalid data");
}

static void testEncodeRoamingConsortium(void)
{
	uint8 buf[BUFFER_SIZE];
	bcm_encode_t oi;

	TEST(bcm_encode_init(&oi, BUFFER_SIZE, buf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_oi_duple(&oi, 4, (uint8 *)"\x00\x11\x22\x33"),
		"bcm_encode_anqp_oi_duple failed");
	TEST(bcm_encode_anqp_oi_duple(&oi, 3, (uint8 *)"\x12\x34\x56"),
		"bcm_encode_anqp_oi_duple failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_roaming_consortium(&enc,
		bcm_encode_length(&oi), bcm_encode_buf(&oi)),
		"bcm_encode_anqp_roaming_consortium failed");

	WL_PRPKT("testEncodeRoamingConsortium",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeRoamingConsortium(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_roaming_consortium_t roam;
	bcm_decode_anqp_oi_duple_t oi1 = {3, "\x11\x22\x33"};
	bcm_decode_anqp_oi_duple_t oi2 = {3, "\x12\x34\x56"};

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 5, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.roamingConsortiumListLength,
		anqp.roamingConsortiumListBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_roaming_consortium(&ie, &roam),
		"bcm_decode_anqp_roaming_consortium failed");
	TEST(roam.numOi == 2, "invalid data");
	TEST(memcmp((char *)roam.oi[0].oi, "\x00\x11\x22\x33",
		roam.oi[0].oiLen) == 0, "invalid data");
	TEST(memcmp((char *)roam.oi[1].oi, "\x12\x34\x56",
		roam.oi[1].oiLen) == 0, "invalid data");

	TEST(bcm_decode_anqp_is_roaming_consortium(&roam, &oi1) == FALSE,
		"bcm_decode_anqp_is_roaming_consortium failed");
	TEST(bcm_decode_anqp_is_roaming_consortium(&roam, &oi2) == TRUE,
		"bcm_decode_anqp_is_roaming_consortium failed");
}

static void testEncodeIpAddressType(void)
{
#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_ip_type_availability(&enc,
		IPA_IPV6_AVAILABLE, IPA_IPV4_PORT_RESTRICT_SINGLE_NAT),
		"bcm_encode_anqp_ip_type_availability failed");

	WL_PRPKT("testEncodeIpAddressType",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeIpAddressType(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_ip_type_t type;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 6, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.ipAddressTypeAvailabilityInfoLength,
		anqp.ipAddressTypeAvailabilityInfoBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_ip_type_availability(&ie, &type),
		"bcm_decode_anqp_ip_type_availability failed");
	TEST(type.ipv6 == IPA_IPV6_AVAILABLE, "invalid data");
	TEST(type.ipv4 == IPA_IPV4_PORT_RESTRICT_SINGLE_NAT, "invalid data");
}

static void testEncodeNaiRealm(void)
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

	TEST(bcm_encode_init(&auth, BUFFER_SIZE, authBuf), "bcm_encode_init failed");
	for (i = 0; i < numAuth; i++) {
		TEST(bcm_encode_anqp_authentication_subfield(&auth,
			REALM_CREDENTIAL, sizeof(credential), &credential),
			"bcm_encode_anqp_authentication_subfield failed");
	}

	TEST(bcm_encode_init(&eap, BUFFER_SIZE, eapBuf), "bcm_encode_init failed");
	for (i = 0; i < numEap; i++) {
		TEST(bcm_encode_anqp_eap_method_subfield(&eap, REALM_EAP_SIM,
			numAuth, bcm_encode_length(&auth), bcm_encode_buf(&auth)),
			"bcm_encode_anqp_eap_method_subfield failed");
	}

	TEST(bcm_encode_init(&realm, BUFFER_SIZE, realmBuf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_nai_realm_data(&realm, REALM_ENCODING_RFC4282,
		31, (uint8 *)"helloworld;myworld.com;test.com", numEap,
		bcm_encode_length(&eap), bcm_encode_buf(&eap)),
		"bcm_encode_anqp_nai_realm_data failed");
	TEST(bcm_encode_anqp_nai_realm_data(&realm, REALM_ENCODING_RFC4282,
		11, (uint8 *)"hotspot.com", numEap,
		bcm_encode_length(&eap), bcm_encode_buf(&eap)),
		"bcm_encode_anqp_nai_realm_data failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_nai_realm(&enc, numRealm,
		bcm_encode_length(&realm), bcm_encode_buf(&realm)),
		"bcm_encode_anqp_nai_realm failed");

	WL_PRPKT("testEncodeNaiRealm",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeNaiRealm(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_nai_realm_list_t realm;
	int i, j, k;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 7, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.naiRealmListLength,
		anqp.naiRealmListBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_nai_realm(&ie, &realm),
		"bcm_decode_anqp_nai_realm failed");

	TEST(realm.realmCount == 2, "invalid data");

	for (i = 0; i < realm.realmCount; i++) {
		TEST(realm.realm[i].encoding == 0, "invalid data");
		if (i == 0)
			TEST(strcmp((char *)realm.realm[i].realm,
				"helloworld;myworld.com;test.com") == 0, "invalid data");
		else
			TEST(strcmp((char *)realm.realm[i].realm,
				"hotspot.com") == 0, "invalid data");
		TEST(realm.realm[i].eapCount == 3, "invalid data");
		for (j = 0; j < realm.realm[i].eapCount; j++) {
			TEST(realm.realm[i].eap[j].eapMethod == REALM_EAP_SIM, "invalid data");
			TEST(realm.realm[i].eap[j].authCount == 2, "invalid data");
			for (k = 0; k < realm.realm[i].eap[j].authCount; k++) {
				TEST(realm.realm[i].eap[j].auth[k].id ==
					REALM_CREDENTIAL, "invalid data");
				TEST(realm.realm[i].eap[j].auth[k].value[0] ==
					REALM_SIM, "invalid data");
			}
		}
	}

	TEST(bcm_decode_anqp_is_realm(&realm, "hotspot.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	TEST(bcm_decode_anqp_is_realm(&realm, "helloworld", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	TEST(bcm_decode_anqp_is_realm(&realm, "myworld.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	TEST(bcm_decode_anqp_is_realm(&realm, "test.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
	TEST(!bcm_decode_anqp_is_realm(&realm, "missing.com", REALM_EAP_SIM, REALM_SIM),
		"bcm_decode_anqp_is_realm failed");
}

static void testEncode3GppCellularNetwork(void)
{
	uint8 plmnBuf[BUFFER_SIZE];
	bcm_encode_t plmn;

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

	TEST(bcm_encode_init(&plmn, BUFFER_SIZE, plmnBuf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_plmn(&plmn, "310", "026"), "bcm_encode_anqp_plmn failed");
	TEST(bcm_encode_anqp_plmn(&plmn, "208", "00"), "bcm_encode_anqp_plmn failed");
	TEST(bcm_encode_anqp_plmn(&plmn, "208", "01"), "bcm_encode_anqp_plmn failed");
	TEST(bcm_encode_anqp_plmn(&plmn, "208", "02"), "bcm_encode_anqp_plmn failed");
	TEST(bcm_encode_anqp_plmn(&plmn, "450", "02"), "bcm_encode_anqp_plmn failed");
	TEST(bcm_encode_anqp_plmn(&plmn, "450", "04"), "bcm_encode_anqp_plmn failed");

	TEST(bcm_encode_anqp_3gpp_cellular_network(&enc, 6,
		bcm_encode_length(&plmn), bcm_encode_buf(&plmn)),
		"bcm_encode_anqp_3gpp_cellular_network failed");

	WL_PRPKT("testEncode3GppCellularNetwork",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecode3GppCellularNetwork(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_3gpp_cellular_network_t g3pp;
	bcm_decode_anqp_plmn_t plmn;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 8, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.g3ppCellularNetworkInfoLength,
		anqp.g3ppCellularNetworkInfoBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_3gpp_cellular_network(&ie, &g3pp),
		"bcm_decode_anqp_3gpp_cellular_network failed");
	TEST(g3pp.plmnCount == 6, "invalid data");
	TEST(strcmp(g3pp.plmn[0].mcc, "310") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[0].mnc, "026") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[1].mcc, "208") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[1].mnc, "00") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[2].mcc, "208") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[2].mnc, "01") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[3].mcc, "208") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[3].mnc, "02") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[4].mcc, "450") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[4].mnc, "02") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[5].mcc, "450") == 0, "invalid data");
	TEST(strcmp(g3pp.plmn[5].mnc, "04") == 0, "invalid data");

	strncpy(plmn.mcc, "310", sizeof(plmn.mcc));
	strncpy(plmn.mnc, "026", sizeof(plmn.mnc));
	TEST(bcm_decode_anqp_is_3gpp(&g3pp, &plmn), "bcm_decode_anqp_is_3gpp failed");

	strncpy(plmn.mcc, "208", sizeof(plmn.mcc));
	strncpy(plmn.mnc, "02", sizeof(plmn.mnc));
	TEST(bcm_decode_anqp_is_3gpp(&g3pp, &plmn), "bcm_decode_anqp_is_3gpp failed");

	strncpy(plmn.mnc, "03", sizeof(plmn.mnc));
	TEST(bcm_decode_anqp_is_3gpp(&g3pp, &plmn) == FALSE, "bcm_decode_anqp_is_3gpp failed");
}

static void testEncodeDomainNameList(void)
{
	uint8 nameBuf[BUFFER_SIZE];
	bcm_encode_t name;

	TEST(bcm_encode_init(&name, BUFFER_SIZE, nameBuf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_domain_name(&name, 10, "helloworld"),
		"bcm_encode_anqp_domain_name failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_domain_name_list(&enc,
		bcm_encode_length(&name), bcm_encode_buf(&name)),
		"bcm_encode_anqp_domain_name_list failed");

	WL_PRPKT("testEncodeDomainNameList",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeDomainNameList(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_domain_name_list_t list;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 9, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.domainNameListLength,
		anqp.domainNameListBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_domain_name_list(&ie, &list),
		"bcm_decode_anqp_domain_name_list failed");
	TEST(list.numDomain == 1, "invalid data");
	TEST(strcmp(list.domain[0].name, "helloworld") == 0, "invalid data");

	TEST(bcm_decode_anqp_is_domain_name(&list, "helloworld"),
		"bcm_decode_anqp_is_domain_name failed");
}

static void testEncodeQueryVendorSpecific(void)
{
#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif
	TEST(bcm_encode_anqp_query_vendor_specific(&enc, 3,
		10, (uint8 *)"helloworld"),
		"bcm_encode_anqp_query_vendor_specific failed");

	WL_PRPKT("testEncodeQueryVendorSpecific",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeQueryVendorSpecific(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	uint16 serviceUpdateIndicator;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 10, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.anqpVendorSpecificListLength,
		anqp.anqpVendorSpecificListBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_query_vendor_specific(&ie, &serviceUpdateIndicator),
		"bcm_decode_anqp_query_vendor_specific failed");
	TEST(serviceUpdateIndicator == 3, "invalid data");
	TEST(bcm_decode_remaining(&ie) == 10, "invalid data");
	TEST(memcmp(bcm_decode_current_ptr(&ie), "helloworld", 10) == 0, "invalid data");
}

static void testEncodeQueryRequestVendorSpecific(void)
{
	uint8 queryBuf[BUFFER_SIZE];
	bcm_encode_t query;

	TEST(bcm_encode_init(&query, BUFFER_SIZE, queryBuf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_query_request_vendor_specific_tlv(&query,
		SVC_RPOTYPE_UPNP, 1, 12, (uint8 *)"queryrequest"),
		"bcm_encode_anqp_query_request_vendor_specific_tlv failed");
	TEST(bcm_encode_anqp_query_request_vendor_specific_tlv(&query,
		SVC_RPOTYPE_BONJOUR, 2, 12, (uint8 *)"queryrequest"),
		"bcm_encode_anqp_query_request_vendor_specific_tlv failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

	TEST(bcm_encode_anqp_query_vendor_specific(&enc, 0,
		bcm_encode_length(&query), bcm_encode_buf(&query)),
		"bcm_encode_anqp_query_vendor_specific failed");

	WL_PRPKT("testEncodeQueryRequestVendorSpecific",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeQueryRequestVendorSpecific(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	uint16 serviceUpdateIndicator;
	bcm_decode_anqp_query_request_vendor_specific_tlv_t request;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 11, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.anqpVendorSpecificListLength,
		anqp.anqpVendorSpecificListBuffer), "bcm_decode_init failed");
	TEST(bcm_decode_anqp_query_vendor_specific(&ie, &serviceUpdateIndicator),
		"bcm_decode_anqp_query_vendor_specific failed");
	TEST(serviceUpdateIndicator == 0, "invalid data");

	TEST(bcm_decode_anqp_query_request_vendor_specific_tlv(&ie, &request),
		"bcm_decode_anqp_query_request_vendor_specific_tlv failed");
	TEST(request.serviceProtocolType == SVC_RPOTYPE_UPNP, "invalid data");
	TEST(request.serviceTransactionId == 1, "invalid data");
	TEST(request.queryLen == 12, "invalid data");
	TEST(memcmp(request.queryData, "queryrequest", 12) == 0, "invalid data");

	TEST(bcm_decode_anqp_query_request_vendor_specific_tlv(&ie, &request),
		"bcm_decode_anqp_query_request_vendor_specific_tlv failed");
	TEST(request.serviceProtocolType == SVC_RPOTYPE_BONJOUR, "invalid data");
	TEST(request.serviceTransactionId == 2, "invalid data");
	TEST(request.queryLen == 12, "invalid data");
	TEST(memcmp(request.queryData, "queryrequest", 12) == 0, "invalid data");

}

static void testEncodeQueryResponseVendorSpecific(void)
{
	uint8 queryBuf[BUFFER_SIZE];
	bcm_encode_t query;

	TEST(bcm_encode_init(&query, BUFFER_SIZE, queryBuf), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_query_response_vendor_specific_tlv(&query,
		SVC_RPOTYPE_UPNP, 1, 0, 13, (uint8 *)"queryresponse"),
		"bcm_encode_anqp_query_response_vendor_specific_tlv failed");
	TEST(bcm_encode_anqp_query_response_vendor_specific_tlv(&query,
		SVC_RPOTYPE_BONJOUR, 2, 0, 13, (uint8 *)"queryresponse"),
		"bcm_encode_anqp_query_response_vendor_specific_tlv failed");

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

	TEST(bcm_encode_anqp_query_vendor_specific(&enc, 0,
		bcm_encode_length(&query), bcm_encode_buf(&query)),
		"bcm_encode_anqp_query_vendor_specific failed");

	WL_PRPKT("testEncodeQueryResponseVendorSpecific",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));
}

static void testDecodeQueryResponseVendorSpecific(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	uint16 serviceUpdateIndicator;
	bcm_decode_anqp_query_response_vendor_specific_tlv_t response;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");

	TEST(bcm_decode_anqp(&dec, &anqp) == 12, "bcm_decode_anqp failed");

	TEST(bcm_decode_init(&ie, anqp.anqpVendorSpecificListLength,
		anqp.anqpVendorSpecificListBuffer), "bcm_decode_init failed");

	TEST(bcm_decode_anqp_query_vendor_specific(&ie, &serviceUpdateIndicator),
		"bcm_decode_anqp_query_vendor_specific failed");
	TEST(serviceUpdateIndicator == 0, "invalid data");

	TEST(bcm_decode_anqp_query_response_vendor_specific_tlv(&ie, &response),
		"bcm_decode_anqp_query_response_vendor_specific_tlv failed");
	TEST(response.serviceProtocolType == SVC_RPOTYPE_UPNP, "invalid data");
	TEST(response.serviceTransactionId == 1, "invalid data");
	TEST(response.statusCode == 0, "invalid data");
	TEST(response.queryLen == 13, "invalid data");
	TEST(memcmp(response.queryData, "queryresponse", 13) == 0, "invalid data");

	TEST(bcm_decode_anqp_query_response_vendor_specific_tlv(&ie, &response),
		"bcm_decode_anqp_query_response_vendor_specific_tlv failed");
	TEST(response.serviceProtocolType == SVC_RPOTYPE_BONJOUR, "invalid data");
	TEST(response.serviceTransactionId == 2, "invalid data");
	TEST(response.statusCode == 0, "invalid data");
	TEST(response.queryLen == 13, "invalid data");
	TEST(memcmp(response.queryData, "queryresponse", 13) == 0, "invalid data");
}

static void testEncodeHspotAnqp(void)
{
	uint8 data[8];
	int i;

	for (i = 0; i < 8; i++)
		data[i] = i;

#if NO_IE_APPEND
	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
#endif // endif

	TEST(bcm_encode_hspot_anqp_query_list(&enc, 8, data),
		"bcm_encode_hspot_anqp_query_list failed");
	WL_PRPKT("hotspot query list",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));

	TEST(bcm_encode_hspot_anqp_capability_list(&enc, 8, data),
		"bcm_encode_hspot_anqp_capability_list failed");
	WL_PRPKT("hotspot capability list",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));

	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;

		TEST(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");

		TEST(bcm_encode_hspot_anqp_operator_name_duple(&name, 2, "EN", 6, "myname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		TEST(bcm_encode_hspot_anqp_operator_name_duple(&name, 2, "FR", 10, "helloworld"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		TEST(bcm_encode_hspot_anqp_operator_name_duple(&name, 5, "JAPAN", 6, "yrname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		TEST(bcm_encode_hspot_anqp_operator_friendly_name(&enc,
			bcm_encode_length(&name), bcm_encode_buf(&name)),
			"bcm_encode_hspot_anqp_operator_friendly_name failed");
		WL_PRPKT("hotspot operator friendly name",
			bcm_encode_buf(&enc), bcm_encode_length(&enc));
	}

	TEST(bcm_encode_hspot_anqp_wan_metrics(&enc,
		HSPOT_WAN_LINK_TEST, HSPOT_WAN_SYMMETRIC_LINK, HSPOT_WAN_AT_CAPACITY,
		0x12345678, 0x11223344, 0xaa, 0xbb, 0xcdef),
		"bcm_encode_hspot_anqp_capability_list failed");
	WL_PRPKT("hotspot WAN metrics",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));

	{
		uint8 capBuf[BUFFER_SIZE];
		bcm_encode_t cap;

		TEST(bcm_encode_init(&cap, BUFFER_SIZE, capBuf), "bcm_encode_init failed");

		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 1, 0, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 20, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 22, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 80, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 443, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 1723, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 5060, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 500, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 5060, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 4500, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");

		TEST(bcm_encode_hspot_anqp_connection_capability(&enc,
			bcm_encode_length(&cap), bcm_encode_buf(&cap)),
			"bcm_encode_hspot_anqp_connection_capability failed");
		WL_PRPKT("hotspot connection capability",
			bcm_encode_buf(&enc), bcm_encode_length(&enc));
	}

	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;

		TEST(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");

		TEST(bcm_encode_hspot_anqp_nai_home_realm_name(&name, 0, 5, "hello"),
			"bcm_encode_hspot_anqp_nai_home_realm_name failed");
		TEST(bcm_encode_hspot_anqp_nai_home_realm_name(&name, 1, 5, "world"),
			"bcm_encode_hspot_anqp_nai_home_realm_name failed");

		TEST(pktEncodeHspotAnqpNaiHomeRealmQuery(&enc, 2,
			bcm_encode_length(&name), bcm_encode_buf(&name)),
			"pktEncodeHspotAnqpNaiHomeRealmQuery failed");
		WL_PRPKT("hotspot NAI home realm query",
			bcm_encode_buf(&enc), bcm_encode_length(&enc));
	}
}

static void testEmpty3GppCellularNetwork(void)
{
	bcm_decode_t dec;
	bcm_decode_anqp_t anqp;
	bcm_decode_t ie;
	bcm_decode_anqp_3gpp_cellular_network_t g3pp;

	TEST(bcm_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_encode_init failed");
	TEST(bcm_encode_anqp_3gpp_cellular_network(&enc, 0, 0, 0),
		"bcm_encode_anqp_3gpp_cellular_network failed");
	WL_PRPKT("testEncode3GppCellularNetwork",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");
	TEST(bcm_decode_anqp(&dec, &anqp) == 1, "bcm_decode_anqp failed");
	TEST(bcm_decode_init(&ie, anqp.g3ppCellularNetworkInfoLength,
		anqp.g3ppCellularNetworkInfoBuffer), "bcm_decode_init failed");
	TEST(bcm_decode_anqp_3gpp_cellular_network(&ie, &g3pp),
		"bcm_decode_anqp_3gpp_cellular_network failed");
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_DEBUG | TRACE_PACKET);
	TEST_INITIALIZE();

	testEncodeQueryList();
	testDecodeQueryList();

	testEncodeCapabilityList();
	testDecodeCapabilityList();

	testEncodeVenueName();
	testDecodeVenueName();

	testEncodeNetworkAuthenticationType();
	testDecodeNetworkAuthenticationType();

	testEncodeRoamingConsortium();
	testDecodeRoamingConsortium();

	testEncodeIpAddressType();
	testDecodeIpAddressType();

	testEncodeNaiRealm();
	testDecodeNaiRealm();

	testEncode3GppCellularNetwork();
	testDecode3GppCellularNetwork();

	testEncodeDomainNameList();
	testDecodeDomainNameList();

	testEncodeQueryVendorSpecific();
	testDecodeQueryVendorSpecific();

	testEncodeQueryRequestVendorSpecific();
	testDecodeQueryRequestVendorSpecific();

	testEncodeQueryResponseVendorSpecific();
	testDecodeQueryResponseVendorSpecific();

	testEncodeHspotAnqp();

	testEmpty3GppCellularNetwork();

	TEST_FINALIZE();
	return 0;
}
