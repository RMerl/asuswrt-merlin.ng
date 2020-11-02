/*
 * Encode functions which provides encoding of Hotspot2.0 ANQP packets
 * as defined in Hotspot2.0 specification.
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
#include "bcm_encode_hspot_anqp.h"

static void encodeHspotAnqpHeader(bcm_encode_t *pkt, uint16 len, uint8 subtype)
{
	bcm_encode_le16(pkt, ANQP_ID_VENDOR_SPECIFIC_LIST);
	bcm_encode_le16(pkt, HSPOT_LENGTH_OVERHEAD + len);
	bcm_encode_bytes(pkt, WFA_OUI_LEN, (uint8 *)WFA_OUI);
	bcm_encode_byte(pkt, HSPOT_ANQP_OUI_TYPE);
	bcm_encode_byte(pkt, subtype);
	bcm_encode_byte(pkt, 0);		/* reserved */
}

/* encode query list */
int bcm_encode_hspot_anqp_query_list(bcm_encode_t *pkt, uint16 queryLen, uint8 *query)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, queryLen, HSPOT_SUBTYPE_QUERY_LIST);
	if (queryLen > 0) {
		bcm_encode_bytes(pkt, queryLen, query);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode capability list */
int bcm_encode_hspot_anqp_capability_list(bcm_encode_t *pkt, uint16 capLen, uint8 *cap)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, capLen, HSPOT_SUBTYPE_CAPABILITY_LIST);
	if (capLen > 0) {
		bcm_encode_bytes(pkt, capLen, cap);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode operator friendly name */
int bcm_encode_hspot_anqp_operator_name_duple(bcm_encode_t *pkt, uint8 langLen, char *lang,
	uint8 nameLen, char *name)
{
	int initLen = bcm_encode_length(pkt);
	int len = langLen <= VENUE_LANGUAGE_CODE_SIZE ? langLen : VENUE_LANGUAGE_CODE_SIZE;

	bcm_encode_byte(pkt, VENUE_LANGUAGE_CODE_SIZE + nameLen);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, (uint8 *)lang);
	}
	while (bcm_encode_length(pkt) - initLen < VENUE_LANGUAGE_CODE_SIZE + 1) {
		bcm_encode_byte(pkt, 0);
	}
	if (nameLen > 0) {
		bcm_encode_bytes(pkt, nameLen, (uint8 *)name);
	}

	return bcm_encode_length(pkt) - initLen;
}

int bcm_encode_hspot_anqp_operator_friendly_name(bcm_encode_t *pkt,
	uint16 nameLen, uint8 *name)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, nameLen, HSPOT_SUBTYPE_OPERATOR_FRIENDLY_NAME);
	if (nameLen > 0) {
		bcm_encode_bytes(pkt, nameLen, name);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode WAN metrics */
int bcm_encode_hspot_anqp_wan_metrics(bcm_encode_t *pkt,
	uint8 linkStatus, uint8 symmetricLink,
	uint8 atCapacity, uint32 dlinkSpeed, uint32 ulinkSpeed,
	uint8 dlinkLoad, uint8 ulinkLoad, uint16 lmd)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, 13, HSPOT_SUBTYPE_WAN_METRICS);
	bcm_encode_byte(pkt,
		linkStatus << HSPOT_WAN_LINK_STATUS_SHIFT |
		symmetricLink << HSPOT_WAN_SYMMETRIC_LINK_SHIFT |
		atCapacity << HSPOT_WAN_AT_CAPACITY_SHIFT);
	bcm_encode_le32(pkt, dlinkSpeed);
	bcm_encode_le32(pkt, ulinkSpeed);
	bcm_encode_byte(pkt, dlinkLoad);
	bcm_encode_byte(pkt, ulinkLoad);
	bcm_encode_le16(pkt, lmd);

	return bcm_encode_length(pkt) - initLen;
}

/* encode connection capability */
int bcm_encode_hspot_anqp_proto_port_tuple(bcm_encode_t *pkt,
	uint8 ipProtocol, uint16 portNumber, uint8 status)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, ipProtocol);
	bcm_encode_le16(pkt, portNumber);
	bcm_encode_byte(pkt, status);

	return bcm_encode_length(pkt) - initLen;
}

int bcm_encode_hspot_anqp_connection_capability(bcm_encode_t *pkt, uint16 capLen, uint8 *cap)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, capLen, HSPOT_SUBTYPE_CONNECTION_CAPABILITY);
	if (capLen > 0) {
		bcm_encode_bytes(pkt, capLen, cap);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode NAI home realm query */
int bcm_encode_hspot_anqp_nai_home_realm_name(bcm_encode_t *pkt, uint8 encoding,
	uint8 nameLen, char *name)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, encoding);
	bcm_encode_byte(pkt, nameLen);
	if (nameLen > 0) {
		bcm_encode_bytes(pkt, nameLen, (uint8 *)name);
	}

	return bcm_encode_length(pkt) - initLen;
}

int pktEncodeHspotAnqpNaiHomeRealmQuery(bcm_encode_t *pkt, uint8 count,
	uint16 nameLen, uint8 *name)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, nameLen + 1, HSPOT_SUBTYPE_NAI_HOME_REALM_QUERY);
	bcm_encode_byte(pkt, count);
	if (nameLen > 0) {
		bcm_encode_bytes(pkt, nameLen, name);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode operating class indication */
int bcm_encode_hspot_anqp_operating_class_indication(bcm_encode_t *pkt,
	uint16 opClassLen, uint8 *opClass)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, opClassLen, HSPOT_SUBTYPE_OPERATING_CLASS_INDICATION);
	if (opClassLen > 0) {
		bcm_encode_bytes(pkt, opClassLen, opClass);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode icon metadata */
int bcm_encode_hspot_anqp_icon_metadata(bcm_encode_t *pkt,
	uint16 width, uint16 height, char *lang,
	uint8 typeLength, uint8 *type, uint8 filenameLength, uint8 *filename)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, width);
	bcm_encode_le16(pkt, height);
	bcm_encode_bytes(pkt, VENUE_LANGUAGE_CODE_SIZE, (uint8 *)lang);
	bcm_encode_byte(pkt, typeLength);
	if (typeLength > 0) {
		bcm_encode_bytes(pkt, typeLength, type);
	}
	bcm_encode_byte(pkt, filenameLength);
	if (filenameLength > 0) {
		bcm_encode_bytes(pkt, filenameLength, filename);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode OSU provider */
int bcm_encode_hspot_anqp_osu_provider(bcm_encode_t *pkt,
	uint16 nameLength, uint8 *name, uint8 uriLength, uint8 *uri,
	uint8 methodLength, uint8 *method, uint16 iconLength, uint8 *icon,
	uint8 naiLength, uint8 *nai, uint16 descLength, uint8 *desc)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, 9 + nameLength + uriLength +
		methodLength + iconLength + naiLength + descLength);
	bcm_encode_le16(pkt, nameLength);
	if (nameLength > 0) {
		bcm_encode_bytes(pkt, nameLength, name);
	}
	bcm_encode_byte(pkt, uriLength);
	if (uriLength > 0) {
		bcm_encode_bytes(pkt, uriLength, uri);
	}
	bcm_encode_byte(pkt, methodLength);
	if (methodLength > 0) {
		bcm_encode_bytes(pkt, methodLength, method);
	}
	bcm_encode_le16(pkt, iconLength);
	if (iconLength > 0) {
		bcm_encode_bytes(pkt, iconLength, icon);
	}
	bcm_encode_byte(pkt, naiLength);
	if (naiLength > 0) {
		bcm_encode_bytes(pkt, naiLength, nai);
	}
	bcm_encode_le16(pkt, descLength);
	if (descLength > 0) {
		bcm_encode_bytes(pkt, descLength, desc);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode OSU provider list */
int bcm_encode_hspot_anqp_osu_provider_list(bcm_encode_t *pkt,
	uint8 osuSsidLength, uint8 *osuSsid,
	uint8 numOsuProvider, uint16 providerLength, uint8 *provider)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, 2 + osuSsidLength + providerLength,
		HSPOT_SUBTYPE_ONLINE_SIGNUP_PROVIDERS);
	bcm_encode_byte(pkt, osuSsidLength);
	if (osuSsidLength > 0) {
		bcm_encode_bytes(pkt, osuSsidLength, osuSsid);
	}
	bcm_encode_byte(pkt, numOsuProvider);
	if (providerLength > 0) {
		bcm_encode_bytes(pkt, providerLength, provider);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode anonymous NAI */
int bcm_encode_hspot_anqp_anonymous_nai(bcm_encode_t *pkt, uint16 length, uint8 *nai)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, 2 + length, HSPOT_SUBTYPE_ANONYMOUS_NAI);
	bcm_encode_le16(pkt, length);
	if (length > 0) {
		bcm_encode_bytes(pkt, length, nai);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode icon request */
int bcm_encode_hspot_anqp_icon_request(bcm_encode_t *pkt, uint16 length, uint8 *filename)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, length, HSPOT_SUBTYPE_ICON_REQUEST);
	if (length > 0) {
		bcm_encode_bytes(pkt, length, filename);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode icon binary file */
int bcm_encode_hspot_anqp_icon_binary_file(bcm_encode_t *pkt,
	uint8 status, uint8 typeLength, uint8 *type, uint16 length, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	encodeHspotAnqpHeader(pkt, 2 + typeLength + 2 + length, HSPOT_SUBTYPE_ICON_BINARY_FILE);
	bcm_encode_byte(pkt, status);
	bcm_encode_byte(pkt, typeLength);
	if (typeLength > 0) {
		bcm_encode_bytes(pkt, typeLength, type);
	}
	bcm_encode_le16(pkt, length);
	if (length > 0) {
		bcm_encode_bytes(pkt, length, data);
	}

	return bcm_encode_length(pkt) - initLen;
}
