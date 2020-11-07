/*
 * Decode functions which provides decoding of ANQP packets as defined in 802.11u.
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
#include "802.11.h"
#include "trace.h"
#include "bcm_decode_anqp.h"

static void printAnqpDecode(bcm_decode_anqp_t *anqp)
{
	WL_P2PO(("decoded ANQP IEs:\n"));

#ifndef BCM_DECODE_NO_ANQP
	if (anqp->anqpQueryListBuffer) {
		WL_PRPKT("   ANQP_QUERY_LIST",
			anqp->anqpQueryListBuffer, anqp->anqpQueryListLength);
	}
	if (anqp->anqpCapabilityListBuffer) {
		WL_PRPKT("   ANQP_CAPABILITY_LIST",
			anqp->anqpCapabilityListBuffer, anqp->anqpCapabilityListLength);
	}
	if (anqp->venueNameInfoBuffer) {
		WL_PRPKT("   VENUE_NAME_INFO",
			anqp->venueNameInfoBuffer, anqp->venueNameInfoLength);
	}
	if (anqp->emergencyCallNumberInfoBuffer) {
		WL_PRPKT("   EMERGENCY_CALL_NUMBER_INFO",
			anqp->emergencyCallNumberInfoBuffer,
			anqp->emergencyCallNumberInfoLength);
	}
	if (anqp->networkAuthenticationTypeInfoBuffer) {
		WL_PRPKT("   NETWORK_AUTHENTICATION_TYPE_INFO",
			anqp->networkAuthenticationTypeInfoBuffer,
			anqp->networkAuthenticationTypeInfoLength);
	}
	if (anqp->roamingConsortiumListBuffer) {
		WL_PRPKT("   ROAMING_CONSORTIUM_LIST",
			anqp->roamingConsortiumListBuffer,
			anqp->roamingConsortiumListLength);
	}
	if (anqp->ipAddressTypeAvailabilityInfoBuffer) {
		WL_PRPKT("   IP_ADDRESS_TYPE_AVAILABILITY_INFO",
			anqp->ipAddressTypeAvailabilityInfoBuffer,
			anqp->ipAddressTypeAvailabilityInfoLength);
	}
	if (anqp->naiRealmListBuffer) {
		WL_PRPKT("   NAI_REALM_LIST",
			anqp->naiRealmListBuffer, anqp->naiRealmListLength);
	}
	if (anqp->g3ppCellularNetworkInfoBuffer) {
		WL_PRPKT("   G3PP_CELLULAR_NETWORK_INFO",
			anqp->g3ppCellularNetworkInfoBuffer,
			anqp->g3ppCellularNetworkInfoLength);
	}
	if (anqp->apGeospatialLocationBuffer) {
		WL_PRPKT("   AP_GEOSPATIAL_LOCATION",
			anqp->apGeospatialLocationBuffer, anqp->apGeospatialLocationLength);
	}
	if (anqp->apCivicLocationBuffer) {
		WL_PRPKT("   AP_CIVIC_LOCATION",
			anqp->apCivicLocationBuffer, anqp->apCivicLocationLength);
	}
	if (anqp->apLocationPublicIdUriBuffer) {
		WL_PRPKT("   AP_LOCATION_PUBLIC_ID_URI",
			anqp->apLocationPublicIdUriBuffer,
			anqp->apLocationPublicIdUriLength);
	}
	if (anqp->domainNameListBuffer) {
		WL_PRPKT("   DOMAIN_NAME_LIST",
			anqp->domainNameListBuffer, anqp->domainNameListLength);
	}
	if (anqp->emergencyAlertIdUriBuffer) {
		WL_PRPKT("   EMERGENCY_ALERT_ID_URI",
			anqp->emergencyAlertIdUriBuffer, anqp->emergencyAlertIdUriLength);
	}
	if (anqp->emergencyNaiBuffer) {
		WL_PRPKT("   EMERGENCY_NAI",
			anqp->emergencyNaiBuffer, anqp->emergencyNaiLength);
	}
	if (anqp->anqpVendorSpecificListBuffer) {
		WL_PRPKT("   ANQP_VENDOR_SPECIFIC_LIST",
			anqp->anqpVendorSpecificListBuffer,
			anqp->anqpVendorSpecificListLength);
	}
	if (anqp->wfaServiceDiscoveryBuffer) {
		WL_PRPKT("   WFA service discovery",
			anqp->wfaServiceDiscoveryBuffer,
			anqp->wfaServiceDiscoveryLength);
	}

#endif	/* BCM_DECODE_NO_ANQP */

#ifndef BCM_DECODE_NO_HOTSPOT_ANQP
	bcm_decode_hspot_anqp_print(&anqp->hspot);
#endif	/* BCM_DECODE_NO_HOTSPOT_ANQP */
}

/* decode ANQP */
int bcm_decode_anqp(bcm_decode_t *pkt, bcm_decode_anqp_t *anqp)
{
	int nextIeOffset = 0;
	int ieCount = 0;

	WL_PRPKT("packet for ANQP decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(anqp, 0, sizeof(*anqp));

	while (nextIeOffset < bcm_decode_buf_length(pkt)) {
		uint16 id;
		uint16 length;
		int dataLength;
		uint8 *dataPtr;

		bcm_decode_offset_set(pkt, nextIeOffset);
		WL_TRACE(("decoding offset 0x%x\n", bcm_decode_offset(pkt)));

		/* minimum ID and length */
		if (bcm_decode_remaining(pkt) < 4) {
			WL_P2PO(("ID and length too short\n"));
			break;
		}

		(void)bcm_decode_le16(pkt, &id);
		(void)bcm_decode_le16(pkt, &length);

		/* check length */
		if (length > bcm_decode_remaining(pkt)) {
			WL_P2PO(("length exceeds packet %d > %d\n",
				length, bcm_decode_remaining(pkt)));
			break;
		}
		nextIeOffset = bcm_decode_offset(pkt) + length;

		/* data */
		dataLength = length;
		dataPtr = bcm_decode_current_ptr(pkt);

		switch (id)
		{
#ifndef BCM_DECODE_NO_ANQP
		case ANQP_ID_QUERY_LIST:
			anqp->anqpQueryListLength = dataLength;
			anqp->anqpQueryListBuffer = dataPtr;
			break;
		case ANQP_ID_CAPABILITY_LIST:
			anqp->anqpCapabilityListLength = dataLength;
			anqp->anqpCapabilityListBuffer = dataPtr;
			break;
		case ANQP_ID_VENUE_NAME_INFO:
			anqp->venueNameInfoLength = dataLength;
			anqp->venueNameInfoBuffer = dataPtr;
			break;
		case ANQP_ID_EMERGENCY_CALL_NUMBER_INFO:
			anqp->emergencyCallNumberInfoLength = dataLength;
			anqp->emergencyCallNumberInfoBuffer = dataPtr;
			break;
		case ANQP_ID_NETWORK_AUTHENTICATION_TYPE_INFO:
			anqp->networkAuthenticationTypeInfoLength = dataLength;
			anqp->networkAuthenticationTypeInfoBuffer = dataPtr;
			break;
		case ANQP_ID_ROAMING_CONSORTIUM_LIST:
			anqp->roamingConsortiumListLength = dataLength;
			anqp->roamingConsortiumListBuffer = dataPtr;
			break;
		case ANQP_ID_IP_ADDRESS_TYPE_AVAILABILITY_INFO:
			anqp->ipAddressTypeAvailabilityInfoLength = dataLength;
			anqp->ipAddressTypeAvailabilityInfoBuffer = dataPtr;
			break;
		case ANQP_ID_NAI_REALM_LIST:
			anqp->naiRealmListLength = dataLength;
			anqp->naiRealmListBuffer = dataPtr;
			break;
		case ANQP_ID_G3PP_CELLULAR_NETWORK_INFO:
			anqp->g3ppCellularNetworkInfoLength = dataLength;
			anqp->g3ppCellularNetworkInfoBuffer = dataPtr;
			break;
		case ANQP_ID_AP_GEOSPATIAL_LOCATION:
			anqp->apGeospatialLocationLength = dataLength;
			anqp->apGeospatialLocationBuffer = dataPtr;
			break;
		case ANQP_ID_AP_CIVIC_LOCATION:
			anqp->apCivicLocationLength = dataLength;
			anqp->apCivicLocationBuffer = dataPtr;
			break;
		case ANQP_ID_AP_LOCATION_PUBLIC_ID_URI:
			anqp->apLocationPublicIdUriLength = dataLength;
			anqp->apLocationPublicIdUriBuffer = dataPtr;
			break;
		case ANQP_ID_DOMAIN_NAME_LIST:
			anqp->domainNameListLength = dataLength;
			anqp->domainNameListBuffer = dataPtr;
			break;
		case ANQP_ID_EMERGENCY_ALERT_ID_URI:
			anqp->emergencyAlertIdUriLength = dataLength;
			anqp->emergencyAlertIdUriBuffer = dataPtr;
			break;
		case ANQP_ID_EMERGENCY_NAI:
			anqp->emergencyNaiLength = dataLength;
			anqp->emergencyNaiBuffer = dataPtr;
			break;
#endif	/* BCM_DECODE_NO_ANQP */
		case ANQP_ID_VENDOR_SPECIFIC_LIST:
		{
			bcm_decode_t vs;
			uint16 sui;

			bcm_decode_init(&vs, dataLength, dataPtr);
			if (bcm_decode_anqp_wfa_service_discovery(&vs, &sui)) {
				anqp->wfaServiceDiscoveryLength = dataLength;
				anqp->wfaServiceDiscoveryBuffer = dataPtr;
				break;
			}

#ifndef BCM_DECODE_NO_HOTSPOT_ANQP
			/* include ID and length */
			bcm_decode_init(&vs, dataLength + 4, dataPtr - 4);
			/* hotspot decode */
			if (bcm_decode_hspot_anqp(&vs, FALSE, &anqp->hspot) == 0)
#endif	/* BCM_DECODE_NO_HOTSPOT_ANQP */
			{
				/* not decoded */
				anqp->anqpVendorSpecificListLength = dataLength;
				anqp->anqpVendorSpecificListBuffer = dataPtr;
			}
		}
			break;
		default:
			WL_P2PO(("invalid ID %d\n", id));
			continue;
			break;
		}

		/* count IEs decoded */
		ieCount++;
	}

	if (ieCount > 0)
		printAnqpDecode(anqp);

	return ieCount;
}

static char *ieName(uint16 id)
{
	char *str;

	switch (id) {
	case ANQP_ID_QUERY_LIST:
		str = "ANQP_ID_QUERY_LIST";
		break;
	case ANQP_ID_CAPABILITY_LIST:
		str = "ANQP_ID_CAPABILITY_LIST";
		break;
	case ANQP_ID_VENUE_NAME_INFO:
		str = "ANQP_ID_VENUE_NAME_INFO";
		break;
	case ANQP_ID_EMERGENCY_CALL_NUMBER_INFO:
		str = "ANQP_ID_EMERGENCY_CALL_NUMBER_INFO";
		break;
	case ANQP_ID_NETWORK_AUTHENTICATION_TYPE_INFO:
		str = "ANQP_ID_NETWORK_AUTHENTICATION_TYPE_INFO";
		break;
	case ANQP_ID_ROAMING_CONSORTIUM_LIST:
		str = "ANQP_ID_ROAMING_CONSORTIUM_LIST";
		break;
	case ANQP_ID_IP_ADDRESS_TYPE_AVAILABILITY_INFO:
		str = "ANQP_ID_IP_ADDRESS_TYPE_AVAILABILITY_INFO";
		break;
	case ANQP_ID_NAI_REALM_LIST:
		str = "ANQP_ID_NAI_REALM_LIST";
		break;
	case ANQP_ID_G3PP_CELLULAR_NETWORK_INFO:
		str = "ANQP_ID_G3PP_CELLULAR_NETWORK_INFO";
		break;
	case ANQP_ID_AP_GEOSPATIAL_LOCATION:
		str = "ANQP_ID_AP_GEOSPATIAL_LOCATION";
		break;
	case ANQP_ID_AP_CIVIC_LOCATION:
		str = "ANQP_ID_AP_CIVIC_LOCATION";
		break;
	case ANQP_ID_AP_LOCATION_PUBLIC_ID_URI:
		str = "ANQP_ID_AP_LOCATION_PUBLIC_ID_URI";
		break;
	case ANQP_ID_DOMAIN_NAME_LIST:
		str = "ANQP_ID_DOMAIN_NAME_LIST";
		break;
	case ANQP_ID_EMERGENCY_ALERT_ID_URI:
		str = "ANQP_ID_EMERGENCY_ALERT_ID_URI";
		break;
	case ANQP_ID_EMERGENCY_NAI:
		str = "ANQP_ID_EMERGENCY_NAI";
		break;
	case ANQP_ID_VENDOR_SPECIFIC_LIST:
		str = "ANQP_ID_VENDOR_SPECIFIC_LIST";
		break;
	default:
		str = "unknown";
		break;
	}

	return str;
}

/* decode ANQP query list */
int bcm_decode_anqp_query_list(bcm_decode_t *pkt, bcm_decode_anqp_query_list_t *queryList)
{
	int count, i;

	WL_PRPKT("packet for ANQP query list decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(queryList, 0, sizeof(*queryList));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	count =  bcm_decode_remaining(pkt) / 2;
	for (i = 0; i < count; i++) {
		if (i >= BCM_DECODE_ANQP_MAX_LIST_SIZE) {
			WL_ERROR(("truncating query list to %d\n",
				BCM_DECODE_ANQP_MAX_LIST_SIZE));
			return FALSE;
		}
		(void)bcm_decode_le16(pkt, &queryList->queryId[queryList->queryLen++]);
	}

	queryList->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded ANQP query list */
void bcm_decode_anqp_query_list_print(bcm_decode_anqp_query_list_t *queryList)
{
	int i;

	if (!queryList->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP query list:\n"));
	WL_PRINT(("query count = %d\n", queryList->queryLen));

	for (i = 0; i < queryList->queryLen; i++) {
		WL_PRINT(("   %s (%d)\n",
			ieName(queryList->queryId[i]), queryList->queryId[i]));
	}
}

/* decode ANQP vendor specific list */
int bcm_decode_anqp_vendor_specific_list(bcm_decode_t *pkt,
	bcm_decode_anqp_vendor_list_t *vendorList)
{
	WL_PRPKT("packet for ANQP vendor list decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(vendorList, 0, sizeof(*vendorList));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	if (bcm_decode_remaining(pkt) > BCM_DECODE_ANQP_MAX_LIST_SIZE) {
		WL_ERROR(("list size %d > %d\n",
			bcm_decode_remaining(pkt), BCM_DECODE_ANQP_MAX_LIST_SIZE));
		return FALSE;
	}

	vendorList->vendorLen = bcm_decode_remaining(pkt);

	(void)bcm_decode_bytes(pkt, vendorList->vendorLen, vendorList->vendorData);

	vendorList->isDecodeValid = TRUE;
	return TRUE;
}

/* decode ANQP capability list */
int bcm_decode_anqp_capability_list(bcm_decode_t *pkt, bcm_decode_anqp_capability_list_t *capList)
{
	WL_PRPKT("packet for ANQP capability list decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(capList, 0, sizeof(*capList));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	while (bcm_decode_remaining(pkt) >= 2) {
		uint16 id;

		if (capList->capLen >= BCM_DECODE_ANQP_MAX_LIST_SIZE) {
			WL_ERROR(("truncating capability list to %d\n",
				BCM_DECODE_ANQP_MAX_LIST_SIZE));
			return FALSE;
		}
		(void)bcm_decode_le16(pkt, &id);

		if (id != ANQP_ID_VENDOR_SPECIFIC_LIST) {
			capList->capId[capList->capLen++] = id;
		}
		else {
			uint16 len;
			bcm_decode_t vs;
			bcm_decode_hspot_anqp_t hspot;
			bcm_decode_t cap;

			if (!bcm_decode_le16(pkt, &len)) {
				WL_ERROR(("decode error\n"));
				return FALSE;
			}

			if (len > bcm_decode_remaining(pkt)) {
				WL_ERROR(("decode error\n"));
				return FALSE;
			}

			/* include ID and length */
			bcm_decode_init(&vs, len + 4,
				bcm_decode_current_ptr(pkt) - 4);
			/* decode hotspot capability */
			if (bcm_decode_hspot_anqp(&vs, TRUE, &hspot)) {
				if (hspot.capabilityListLength > BCM_DECODE_ANQP_MAX_LIST_SIZE) {
					WL_ERROR(("list size %d > %d\n",
						hspot.capabilityListLength,
						BCM_DECODE_ANQP_MAX_LIST_SIZE));
					return FALSE;
				}
				bcm_decode_init(&cap, hspot.capabilityListLength,
					hspot.capabilityListBuffer);
				if (!bcm_decode_hspot_anqp_capability_list(
					&cap, &capList->hspotCapList)) {
					return FALSE;
				}
			}

			/* advance packet pointer */
			bcm_decode_offset_set(pkt, bcm_decode_offset(pkt) + len);
		}
	}

	capList->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded ANQP capability list */
void bcm_decode_anqp_capability_list_print(bcm_decode_anqp_capability_list_t *capList)
{
	int i;

	if (!capList->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP capability list:\n"));
	WL_PRINT(("capability count = %d\n", capList->capLen));

	for (i = 0; i < capList->capLen; i++) {
		WL_PRINT(("   %s (%d)\n",
			ieName(capList->capId[i]), capList->capId[i]));
	}
}

/* decode venue name duple */
static int pktDecodeAnqpVenueNameDuple(bcm_decode_t *pkt, bcm_decode_anqp_venue_name_duple_t *duple)
{
	uint8 len;

	if (!bcm_decode_byte(pkt, &len)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, VENUE_LANGUAGE_CODE_SIZE, (uint8 *)duple->lang)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	duple->lang[VENUE_LANGUAGE_CODE_SIZE] = 0;
	duple->langLen = strlen(duple->lang);

	if (len - VENUE_LANGUAGE_CODE_SIZE > bcm_decode_remaining(pkt)) {
		WL_ERROR(("packet too short %d > %d\n",
		len - VENUE_LANGUAGE_CODE_SIZE, len - VENUE_LANGUAGE_CODE_SIZE));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, len - VENUE_LANGUAGE_CODE_SIZE, (uint8 *)duple->name)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	duple->name[len - VENUE_LANGUAGE_CODE_SIZE] = 0;
	duple->nameLen = strlen(duple->name);

	return TRUE;
}

int bcm_decode_anqp_venue_name(bcm_decode_t *pkt, bcm_decode_anqp_venue_name_t *venueName)
{
	WL_PRPKT("packet for ANQP venue name decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(venueName, 0, sizeof(*venueName));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	if (!bcm_decode_byte(pkt, &venueName->group)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &venueName->type)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	while (bcm_decode_remaining(pkt) > 0 &&
		venueName->numVenueName < BCM_DECODE_ANQP_MAX_VENUE_NAME) {
		if (!pktDecodeAnqpVenueNameDuple(pkt,
			&venueName->venueName[venueName->numVenueName])) {
			return FALSE;
		}
		else {
			venueName->numVenueName++;
		}
	}

	venueName->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded venue name */
void bcm_decode_anqp_venue_name_print(bcm_decode_anqp_venue_name_t *venueName)
{
	int i;

	if (!venueName->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP venue name:\n"));
	WL_PRINT(("   group: %d\n", venueName->group));
	WL_PRINT(("   type: %d\n", venueName->type));
	for (i = 0; i < venueName->numVenueName; i++) {
		WL_PRINT(("   language: %s\n", venueName->venueName[i].lang));
		WL_PRUSR("   venue name",
			(uint8 *)venueName->venueName[i].name, venueName->venueName[i].nameLen);
	}
}

/* decode network authentication unit */
static int pktDecodeAnqpNetworkAuthenticationUnit(bcm_decode_t *pkt,
	bcm_decode_anqp_network_authentication_unit_t *unit)
{

	if (!bcm_decode_byte(pkt, &unit->type)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_le16(pkt, &unit->urlLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (unit->urlLen > BCM_DECODE_ANQP_MAX_URL_LENGTH) {
		WL_ERROR(("URL length %d > %d\n", unit->urlLen, BCM_DECODE_ANQP_MAX_URL_LENGTH));
		return FALSE;
	}

	if (unit->urlLen && !bcm_decode_bytes(pkt, unit->urlLen, unit->url)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	unit->url[unit->urlLen] = 0;

	return TRUE;
}

/* decode network authentication type */
int bcm_decode_anqp_network_authentication_type(bcm_decode_t *pkt,
	bcm_decode_anqp_network_authentication_type_t *auth)
{
	WL_PRPKT("packet for ANQP network authentication type decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(auth, 0, sizeof(*auth));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	while (bcm_decode_remaining(pkt) > 0 &&
		auth->numAuthenticationType < BCM_DECODE_ANQP_MAX_AUTHENTICATION_UNIT) {
		if (!pktDecodeAnqpNetworkAuthenticationUnit(pkt,
			&auth->unit[auth->numAuthenticationType])) {
			return FALSE;
		}
		else {
			auth->numAuthenticationType++;
		}
	}

	auth->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded network authentication type */
void bcm_decode_anqp_network_authentication_type_print(
	bcm_decode_anqp_network_authentication_type_t *auth)
{
	int i;

	if (!auth->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP network authentication type:\n"));
	WL_PRINT(("   count: %d\n", auth->numAuthenticationType));
	for (i = 0; i < auth->numAuthenticationType; i++) {
		WL_PRINT(("   type: %d  url:%s\n",
			auth->unit[i].type, auth->unit[i].url));
	}
}

/* search decoded network authentication type for online enrollment support */
int bcm_decode_anqp_is_online_enrollment_support(
	bcm_decode_anqp_network_authentication_type_t *auth)
{
	int i;

	if (!auth->isDecodeValid)
		return FALSE;

	for (i = 0; i < auth->numAuthenticationType; i++) {
		if (auth->unit[i].type == NATI_ONLINE_ENROLLMENT_SUPPORTED) {
			return TRUE;
		}
	}

	return FALSE;
}

/* decode OI duple */
static int pktDecodeAnqpOiDuple(bcm_decode_t *pkt, bcm_decode_anqp_oi_duple_t *oi)
{

	if (!bcm_decode_byte(pkt, &oi->oiLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (oi->oiLen > BCM_DECODE_ANQP_MAX_OI_LENGTH) {
		WL_ERROR(("OI length %d > %d\n", oi->oiLen, BCM_DECODE_ANQP_MAX_OI_LENGTH));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, oi->oiLen, oi->oi)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	return TRUE;
}

/* decode roaming consortium */
int bcm_decode_anqp_roaming_consortium(bcm_decode_t *pkt,
	bcm_decode_anqp_roaming_consortium_t *roam)
{
	WL_PRPKT("packet for ANQP roaming consortium decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(roam, 0, sizeof(*roam));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	while (bcm_decode_remaining(pkt) > 0 &&
		roam->numOi < BCM_DECODE_ANQP_MAX_OI) {
		if (!pktDecodeAnqpOiDuple(pkt,
			&roam->oi[roam->numOi])) {
			return FALSE;
		}
		else {
			roam->numOi++;
		}
	}

	roam->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded roaming consortium */
void bcm_decode_anqp_roaming_consortium_print(bcm_decode_anqp_roaming_consortium_t *roam)
{
	int i;

	if (!roam->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP roaming consortium:\n"));
	WL_PRINT(("   count: %d\n", roam->numOi));
	for (i = 0; i < roam->numOi; i++) {
		WL_PRUSR("   OI",
			roam->oi[i].oi, roam->oi[i].oiLen);
	}
}

/* search decoded roaming consortium for a match */
int bcm_decode_anqp_is_roaming_consortium(bcm_decode_anqp_roaming_consortium_t *roam,
	bcm_decode_anqp_oi_duple_t *oi)
{
	int i;

	if (!roam->isDecodeValid)
		return FALSE;

	for (i = 0; i < roam->numOi; i++) {
		bcm_decode_anqp_oi_duple_t *r = &roam->oi[i];
		if (oi->oiLen == r->oiLen) {
			if (memcmp(oi->oi, r->oi, oi->oiLen) == 0)
				return TRUE;
		}
	}

	return FALSE;
}

/* decode IP address type availability */
int bcm_decode_anqp_ip_type_availability(bcm_decode_t *pkt, bcm_decode_anqp_ip_type_t *ip)
{
	uint8 type;

	WL_PRPKT("packet for IP type availability decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(ip, 0, sizeof(*ip));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	if (!bcm_decode_byte(pkt, &type)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	ip->ipv6 = (type & IPA_IPV6_MASK) >> IPA_IPV6_SHIFT;
	ip->ipv4 = (type & IPA_IPV4_MASK) >> IPA_IPV4_SHIFT;

	ip->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded IP address type availability */
void bcm_decode_anqp_ip_type_availability_print(bcm_decode_anqp_ip_type_t *ip)
{
	char *str;

	if (!ip->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP IP type availability:\n"));

	switch (ip->ipv6)
	{
		case IPA_IPV6_NOT_AVAILABLE:
			str = "address type not available";
			break;
		case IPA_IPV6_AVAILABLE:
			str = "address type available";
			break;
		case IPA_IPV6_UNKNOWN_AVAILABILITY:
			str = "availability of the address type not known";
			break;
		default:
			str = "unknown";
			break;
	}
	WL_PRINT(("IPv6: %d   %s\n", ip->ipv6, str));

	switch (ip->ipv4)
	{
		case IPA_IPV4_NOT_AVAILABLE:
			str = "address type not available";
			break;
		case IPA_IPV4_PUBLIC:
			str = "public IPv4 adress available";
			break;
		case IPA_IPV4_PORT_RESTRICT:
			str = "port-restricted IPv4 address available";
			break;
		case IPA_IPV4_SINGLE_NAT:
			str = "single NATed private IPv4 address available";
			break;
		case IPA_IPV4_DOUBLE_NAT:
			str = "double NATed private IPv4 address available";
			break;
		case IPA_IPV4_PORT_RESTRICT_SINGLE_NAT:
			str = "port-restricted IPv4 address and single "
			"NATed IPv4 address available";
			break;
		case IPA_IPV4_PORT_RESTRICT_DOUBLE_NAT:
			str = "port-restricted IPv4 address and double "
			"NATed IPv4 address available";
			break;
		case IPA_IPV4_UNKNOWN_AVAILABILITY:
			str = "availability of the address type is not known";
			break;
		default:
			str = "unknown";
			break;
	}
	WL_PRINT(("IPv4: %d   %s\n", ip->ipv4, str));
}

/* decode authentication parameter */
static int pktDecodeAnqpAuthentication(bcm_decode_t *pkt, bcm_decode_anqp_auth_t *auth)
{

	if (!bcm_decode_byte(pkt, &auth->id)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &auth->len)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (auth->len > BCM_DECODE_ANQP_MAX_AUTH_PARAM) {
		WL_ERROR(("authentication parameter %d > %d\n",
			auth->len, BCM_DECODE_ANQP_MAX_AUTH_PARAM));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, auth->len, auth->value)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	return TRUE;
}

/* decode EAP method */
static int pktDecodeAnqpEapMethod(bcm_decode_t *pkt, bcm_decode_anqp_eap_method_t *eap)
{
	uint8 len;
	int i;

	if (!bcm_decode_byte(pkt, &len)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (len > bcm_decode_remaining(pkt)) {
		WL_ERROR(("packet too short %d > %d\n",
			len, bcm_decode_remaining(pkt)));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &eap->eapMethod)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &eap->authCount)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (eap->authCount > BCM_DECODE_ANQP_MAX_AUTH) {
		WL_ERROR(("auth count %d > %d\n",
			eap->authCount, BCM_DECODE_ANQP_MAX_AUTH));
		return FALSE;
	}

	for (i = 0; i < eap->authCount; i++) {
		if (!pktDecodeAnqpAuthentication(pkt, &eap->auth[i]))
			return FALSE;
	}

	return TRUE;
}

/* decode NAI realm data */
static int pktDecodeAnqpNaiRealmData(bcm_decode_t *pkt, bcm_decode_anqp_nai_realm_data_t *data)
{
	uint16 len;
	int i;

	if (!bcm_decode_le16(pkt, &len)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (len > bcm_decode_remaining(pkt)) {
		WL_ERROR(("packet too short %d > %d\n",
			len, bcm_decode_remaining(pkt)));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &data->encoding)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &data->realmLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, data->realmLen, data->realm)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	data->realm[data->realmLen] = 0;

	if (!bcm_decode_byte(pkt, &data->eapCount)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (data->eapCount > BCM_DECODE_ANQP_MAX_EAP_METHOD) {
		WL_ERROR(("EAP count %d > %d\n",
			data->eapCount, BCM_DECODE_ANQP_MAX_EAP_METHOD));
		return FALSE;
	}

	for (i = 0; i < data->eapCount; i++) {
		if (!pktDecodeAnqpEapMethod(pkt, &data->eap[i]))
			return FALSE;
	}

	return TRUE;
}

/* decode NAI realm */
int bcm_decode_anqp_nai_realm(bcm_decode_t *pkt, bcm_decode_anqp_nai_realm_list_t *realm)
{
	int i;

	WL_PRPKT("packet for NAI realm decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(realm, 0, sizeof(*realm));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	if (!bcm_decode_le16(pkt, &realm->realmCount)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
}

	if (realm->realmCount > BCM_DECODE_ANQP_MAX_REALM) {
		WL_ERROR(("realm count %d > %d\n",
			realm->realmCount, BCM_DECODE_ANQP_MAX_REALM));
		return FALSE;
	}

	for (i = 0; i < realm->realmCount; i++) {
		if (!pktDecodeAnqpNaiRealmData(pkt, &realm->realm[i]))
			return FALSE;
	}

	realm->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded NAI realm */
void bcm_decode_anqp_nai_realm_print(bcm_decode_anqp_nai_realm_list_t *realm)
{
	int i, j, k;

	if (!realm->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP NAI realm list:\n"));
	WL_PRINT(("realm count = %d\n", realm->realmCount));

	for (i = 0; i < realm->realmCount; i++) {
		bcm_decode_anqp_nai_realm_data_t *data = &realm->realm[i];
		WL_PRINT(("\n"));
		WL_PRINT(("%s:\n", data->realm));
		WL_PRINT(("encoding = %d\n", data->encoding));
		WL_PRINT(("EAP count = %d\n", data->eapCount));

		for (j = 0; j < data->eapCount; j++) {
			bcm_decode_anqp_eap_method_t *eap = &data->eap[j];
			char *eapStr;

			switch (eap->eapMethod) {
			case REALM_EAP_TLS:
				eapStr = "EAP-TLS";
				break;
			case REALM_EAP_SIM:
				eapStr = "EAP-SIM";
				break;
			case REALM_EAP_TTLS:
				eapStr = "EAP-TTLS";
				break;
			case REALM_EAP_AKA:
				eapStr = "EAP-AKA";
				break;
			case REALM_EAP_AKAP:
				eapStr = "EAP-AKA'";
				break;
			default:
				eapStr = "unknown";
				break;
			}
			WL_PRINT(("   EAP method = %s (%d)\n", eapStr, eap->eapMethod));
			WL_PRINT(("   authentication count = %d\n", eap->authCount));

			for (k = 0; k < eap->authCount; k++) {
				bcm_decode_anqp_auth_t *auth = &eap->auth[k];
				char *authStr;
				char *paramStr;

				switch (auth->id) {
				case REALM_EXPANDED_EAP:
					authStr = "expanded EAP method";
					break;
				case REALM_NON_EAP_INNER_AUTHENTICATION:
					authStr = "non-EAP inner authentication";
					break;
				case REALM_INNER_AUTHENTICATION_EAP:
					authStr = "inner authentication EAP method";
					break;
				case REALM_EXPANDED_INNER_EAP:
					authStr = "expanded inner EAP method";
					break;
				case REALM_CREDENTIAL:
					authStr = "credential";
					break;
				case REALM_TUNNELED_EAP_CREDENTIAL:
					authStr = "tunneled EAP method credential";
					break;
				case REALM_VENDOR_SPECIFIC_EAP:
					authStr = "vendor specific";
					break;
				default:
					authStr = "unknown";
					break;
				}
				WL_PRINT(("      authentication = %s (%d)\n",
					authStr, auth->id));

				if (auth->id == REALM_NON_EAP_INNER_AUTHENTICATION) {
					switch (auth->value[0]) {
					case REALM_PAP:
						paramStr = "PAP";
						break;
					case REALM_CHAP:
						paramStr = "CHAP";
						break;
					case REALM_MSCHAP:
						paramStr = "MSCHAP";
						break;
					case REALM_MSCHAPV2:
						paramStr = "MSCHAPV2";
						break;
					default:
						paramStr = "unknown";
						break;
					}
					WL_PRINT(("      %s (%d)\n",
						paramStr, auth->value[0]));
				}
				else if (auth->id == REALM_CREDENTIAL ||
					auth->id == REALM_TUNNELED_EAP_CREDENTIAL) {
					switch (auth->value[0]) {
					case REALM_SIM:
						paramStr = "SIM";
						break;
					case REALM_USIM:
						paramStr = "USIM";
						break;
					case REALM_NFC:
						paramStr = "NFC";
						break;
					case REALM_HARDWARE_TOKEN:
						paramStr = "hardware token";
						break;
					case REALM_SOFTOKEN:
						paramStr = "softoken";
						break;
					case REALM_CERTIFICATE:
						paramStr = "certificate";
						break;
					case REALM_USERNAME_PASSWORD:
						paramStr = "username/password";
						break;
					case REALM_SERVER_SIDE:
						paramStr = "server side";
						break;
					default:
						paramStr = "unknown";
						break;
					}
					WL_PRINT(("      %s (%d)\n",
						paramStr, auth->value[0]));
				}
				else {
					if (auth->len > 0) {
						int i;

						WL_PRINT(("      "));
						for (i = 0; i < auth->len; i++)
							WL_PRINT(("%02x ",
								auth->value[i]));
					}
				}
			}
		}
	}
}

/* search the decoded NAI realm for a match */
int bcm_decode_anqp_is_realm(bcm_decode_anqp_nai_realm_list_t *realmList,
	char *realmName, uint8 eapMethod, uint8 credential)
{
	int i, j, k;
	bcm_decode_anqp_nai_realm_data_t *data;
	bcm_decode_anqp_eap_method_t *eap;

	if (!realmList->isDecodeValid)
		return FALSE;

	/* search for realm name */
	for (i = 0; i < realmList->realmCount; i++) {
		uint8 realm[BCM_DECODE_ANQP_MAX_REALM_LENGTH + 1];
		int argc = 0;
		char *argv[16], *token;
		int isRealmFound = FALSE;

		data = &realmList->realm[i];

		/* the realm may have multiple entries delimited by ';' */
		/* copy and convert realm to argc/argv format */
		strncpy((char *)realm, (char *)data->realm, sizeof(realm));
#ifndef BCMDRIVER
		while ((argc < (int)(sizeof(argv) / sizeof(char *) - 1)) &&
		     ((token = strtok(argc ? NULL : (char *)realm, ";")) != NULL)) {
#else
		while ((argc < (int)(sizeof(argv) / sizeof(char *) - 1)) &&
		     ((token = bcmstrtok((char **)&realm, ";", 0)) != NULL)) {
#endif // endif
			argv[argc++] = token;
		}
		argv[argc] = NULL;

		/* compare each realm entry */
		for (j = 0; j < argc; j++) {
			if (realmName != 0 &&
				strcmp(argv[j], realmName) == 0) {
				isRealmFound = TRUE;
				break;
			}
		}

		if (!isRealmFound)
			continue;

		/* search for EAP method */
		for (j = 0; j < data->eapCount; j++) {
			eap = &data->eap[j];
			if (eap->eapMethod == eapMethod) {
				/* seach for credential */
				for (k = 0; k < eap->authCount; k++) {
					bcm_decode_anqp_auth_t *auth = &eap->auth[k];
					if (auth->id == REALM_CREDENTIAL &&
						auth->value[0] == credential) {
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

/* decode PLMN */
static int pktDecodeAnqpPlmn(bcm_decode_t *pkt, bcm_decode_anqp_plmn_t *plmn)
{
	uint8 octet1, octet2, octet3;

	if (!bcm_decode_byte(pkt, &octet1)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &octet2)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &octet3)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	plmn->mcc[0] = (octet1 & 0x0f) + '0';
	plmn->mcc[1] = (octet1 >> 4) + '0';
	plmn->mcc[2] = (octet2 & 0x0f) + '0';
	plmn->mcc[3] = 0;

	plmn->mnc[0] = (octet3 & 0x0f) + '0';
	plmn->mnc[1] = (octet3 >> 4) + '0';
	if ((octet2 & 0xf0) == 0xf0)
		plmn->mnc[2] = 0;
	else
		plmn->mnc[2] = (octet2 >> 4) + '0';
	plmn->mnc[3] = 0;

	return TRUE;
}

/* decode 3GPP cellular network */
int bcm_decode_anqp_3gpp_cellular_network(bcm_decode_t *pkt,
	bcm_decode_anqp_3gpp_cellular_network_t *g3pp)
{
	uint8 byte;
	uint8 count;
	int i;

	WL_PRPKT("packet for 3GPP cellular network",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(g3pp, 0, sizeof(*g3pp));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	if (!bcm_decode_byte(pkt, &byte)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (byte != G3PP_GUD_VERSION) {
		WL_ERROR(("3GPP PLMN GUD version %d != %d\n",
			byte, G3PP_GUD_VERSION));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &byte)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (byte > bcm_decode_remaining(pkt)) {
		WL_ERROR(("UDHL too short %d > %d\n",
			byte, bcm_decode_remaining(pkt)));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &byte)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (byte != G3PP_PLMN_LIST_IE) {
		WL_ERROR(("3GPP PLMN IE %d != %d\n",
			byte, G3PP_PLMN_LIST_IE));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &byte)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (byte > bcm_decode_remaining(pkt)) {
		WL_ERROR(("PLMN length too short %d > %d\n",
			byte, bcm_decode_remaining(pkt)));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &count)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (count > BCM_DECODE_ANQP_MAX_PLMN) {
		WL_ERROR(("PLMN count %d > %d\n",
			count, BCM_DECODE_ANQP_MAX_PLMN));
		return FALSE;
	}

	if (count * 3 > bcm_decode_remaining(pkt)) {
		WL_ERROR(("packet too short %d > %d\n",
			count * 3, bcm_decode_remaining(pkt)));
		return FALSE;
	}

	for (i = 0; i < count; i++)
		pktDecodeAnqpPlmn(pkt, &g3pp->plmn[i]);

	g3pp->plmnCount = count;
	g3pp->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded 3GPP cellular network */
void bcm_decode_anqp_3gpp_cellular_network_print(bcm_decode_anqp_3gpp_cellular_network_t *g3pp)
{
	int i;

	if (!g3pp->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP 3GPP cellular network:\n"));
	WL_PRINT(("   PLMN count = %d\n", g3pp->plmnCount));
	for (i = 0; i < g3pp->plmnCount; i++)
		WL_PRINT(("   MCC=%s MNC=%s\n", g3pp->plmn[i].mcc, g3pp->plmn[i].mnc));
}

/* search the decoded 3GPP cellular network for a match */
int bcm_decode_anqp_is_3gpp(bcm_decode_anqp_3gpp_cellular_network_t *g3pp,
	bcm_decode_anqp_plmn_t *plmn)
{
	int i;

	if (!g3pp->isDecodeValid)
		return FALSE;

	for (i = 0; i < g3pp->plmnCount; i++) {
		if (strcmp(plmn->mcc, g3pp->plmn[i].mcc) == 0 &&
			strcmp(plmn->mnc, g3pp->plmn[i].mnc) == 0)
			return TRUE;
	}

	return FALSE;
}

/* decode domain name */
static int pktDecodeAnqpDomainName(bcm_decode_t *pkt, bcm_decode_anqp_domain_name_t *domain)
{

	if (!bcm_decode_byte(pkt, &domain->len)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (domain->len > bcm_decode_remaining(pkt)) {
		WL_ERROR(("packet too short %d > %d\n",
		domain->len, bcm_decode_remaining(pkt)));
		return FALSE;
	}

	if (domain->len > BCM_DECODE_ANQP_MAX_DOMAIN_NAME_SIZE) {
		WL_ERROR(("domain name size %d > %d\n",
		domain->len, BCM_DECODE_ANQP_MAX_DOMAIN_NAME_SIZE));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, domain->len, (uint8 *)domain->name)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	domain->name[domain->len] = 0;

	return TRUE;
}

/* decode domain name list */
int bcm_decode_anqp_domain_name_list(bcm_decode_t *pkt,
	bcm_decode_anqp_domain_name_list_t *list)
{
	WL_PRPKT("packet for ANQP domain name list",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(list, 0, sizeof(*list));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	while (bcm_decode_remaining(pkt) > 0 &&
		list->numDomain < BCM_DECODE_ANQP_MAX_DOMAIN) {
		if (!pktDecodeAnqpDomainName(pkt,
			&list->domain[list->numDomain])) {
			return FALSE;
		}
		else {
			list->numDomain++;
		}
	}

	list->isDecodeValid = TRUE;
	return TRUE;
}

/* print decoded domain name list */
void bcm_decode_anqp_domain_name_list_print(bcm_decode_anqp_domain_name_list_t *list)
{
	int i;

	if (!list->isDecodeValid)
		return;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded ANQP domain name:\n"));
	WL_PRINT(("   count = %d\n", list->numDomain));
	for (i = 0; i < list->numDomain; i++)
		WL_PRINT(("   %s\n", list->domain[i].name));
}

/* search the decoded domain name list for a match */
int bcm_decode_anqp_is_domain_name(bcm_decode_anqp_domain_name_list_t *list,
	char *domain, int isSubdomain)
{
	int i;

	if (!list->isDecodeValid)
		return FALSE;

	if (domain != 0) {
		for (i = 0; i < list->numDomain; i++) {
			if (isSubdomain) {
				char *p;
				if ((p = strstr(list->domain[i].name, domain)) != 0) {
					if (p == list->domain[i].name) {
						/* exact match */
						return TRUE;
					}
					else {
						char *q = p - 1;
						/* previous char must be dot and
						 * suffix match
						 */
						if (*q == '.' &&
							strcmp(p, domain) == 0) {
							return TRUE;
						}
					}
				}
			}
			else {
				/* full domain (exact) match */
				if (strcmp(domain, list->domain[i].name) == 0)
					return TRUE;
			}
		}
	}

	return FALSE;
}

/* decode WFA service discovery */
int bcm_decode_anqp_wfa_service_discovery(bcm_decode_t *pkt, uint16 *serviceUpdateIndicator)
{
	uint8 oui[WFA_OUI_LEN];
	uint8 ouiSubtype;

	WL_PRPKT("packet for ANQP query vendor specific",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	/* check OUI */
	if (!bcm_decode_bytes(pkt, WFA_OUI_LEN, oui) ||
		memcmp(oui, WFA_OUI, WFA_OUI_LEN) != 0) {
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &ouiSubtype) || ouiSubtype != ANQP_OUI_SUBTYPE) {
		return FALSE;
	}
	if (!bcm_decode_le16(pkt, serviceUpdateIndicator)) {
		return FALSE;
	}
	return TRUE;
}

static int pktDecodeAnqpQueryVendorSpecificTlv(bcm_decode_t *pkt,
	uint8 *serviceProtocolType, uint8 *serviceTransactionId,
	uint8 *statusCode, uint8 *numService,
	uint16 *dataLen, uint8 **data)
{
	uint16 length;

	if (!bcm_decode_le16(pkt, &length)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (length > bcm_decode_remaining(pkt)) {
		WL_ERROR(("length exceeds packet %d > %d\n",
			length, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, serviceProtocolType)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, serviceTransactionId)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (statusCode != 0) {
		if (!bcm_decode_byte(pkt, statusCode)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
	}
	if (numService != 0) {
		if (!bcm_decode_byte(pkt, numService)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
	}
	*dataLen = length - (2 + (statusCode != 0 ? 1 : 0) +
		(numService != 0 ? 1 : 0));
	*data = bcm_decode_current_ptr(pkt);
	bcm_decode_offset_set(pkt, bcm_decode_offset(pkt) + *dataLen);
	return TRUE;
}

/* decode query request vendor specific TLV */
int bcm_decode_anqp_query_request_vendor_specific_tlv(bcm_decode_t *pkt,
	bcm_decode_anqp_query_request_vendor_specific_tlv_t *request)
{
	WL_PRPKT("packet for ANQP query request vendor specific TLV",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(request, 0, sizeof(*request));
	return pktDecodeAnqpQueryVendorSpecificTlv(pkt,
		&request->serviceProtocolType,
		&request->serviceTransactionId, 0, 0,
		&request->dataLen, &request->data);
}

/* decode query response vendor specific TLV */
int bcm_decode_anqp_query_response_vendor_specific_tlv(bcm_decode_t *pkt, int isNumService,
	bcm_decode_anqp_query_response_vendor_specific_tlv_t *response)
{
	WL_PRPKT("packet for ANQP query response vendor specific TLV",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(response, 0, sizeof(*response));
	return pktDecodeAnqpQueryVendorSpecificTlv(pkt,
		&response->serviceProtocolType,
		&response->serviceTransactionId, &response->statusCode,
		isNumService ? &response->numService : 0,
		&response->dataLen, &response->data);
}

/* decode WFDS request */
int bcm_decode_anqp_wfds_request(bcm_decode_t *pkt,
	bcm_decode_anqp_wfds_request_t *request)
{
	WL_PRPKT("packet for WFDS request",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(request, 0, sizeof(*request));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	if (!bcm_decode_byte(pkt, &request->serviceNameLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (request->serviceNameLen > bcm_decode_remaining(pkt)) {
		WL_ERROR(("service name length exceeds packet %d > %d\n",
			request->serviceNameLen, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	request->serviceName = bcm_decode_current_ptr(pkt);
	bcm_decode_offset_set(pkt, bcm_decode_offset(pkt) + request->serviceNameLen);

	if (!bcm_decode_byte(pkt, &request->serviceInfoReqLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (request->serviceInfoReqLen > bcm_decode_remaining(pkt)) {
		WL_ERROR(("service request info length exceeds packet %d > %d\n",
			request->serviceInfoReqLen, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	request->serviceInfoReq = bcm_decode_current_ptr(pkt);
	bcm_decode_offset_set(pkt, bcm_decode_offset(pkt) + request->serviceInfoReqLen);

	return TRUE;
}

/* decode WFDS response */
int bcm_decode_anqp_wfds_response(bcm_decode_t *pkt,
	bcm_decode_anqp_wfds_response_t *response)
{
	WL_PRPKT("packet for WFDS response",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(response, 0, sizeof(*response));

	if (bcm_decode_is_zero_length(pkt))
		return TRUE;

	if (!bcm_decode_le32(pkt, &response->advertisementId)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (!bcm_decode_le16(pkt, &response->configMethod)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &response->serviceNameLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (response->serviceNameLen > bcm_decode_remaining(pkt)) {
		WL_ERROR(("service name length exceeds packet %d > %d\n",
			response->serviceNameLen, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	response->serviceName = bcm_decode_current_ptr(pkt);
	bcm_decode_offset_set(pkt, bcm_decode_offset(pkt) + response->serviceNameLen);

	if (!bcm_decode_byte(pkt, &response->serviceStatus)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_le16(pkt, &response->serviceInfoLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (response->serviceInfoLen > bcm_decode_remaining(pkt)) {
		WL_ERROR(("service info length exceeds packet %d > %d\n",
			response->serviceInfoLen, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	response->serviceInfo = bcm_decode_current_ptr(pkt);
	bcm_decode_offset_set(pkt, bcm_decode_offset(pkt) + response->serviceInfoLen);

	return TRUE;
}
