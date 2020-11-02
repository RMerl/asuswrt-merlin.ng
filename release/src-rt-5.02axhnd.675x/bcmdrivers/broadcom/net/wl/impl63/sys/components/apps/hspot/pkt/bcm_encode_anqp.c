/*
 * Encode functions which provides encoding of ANQP packets as defined in 802.11u.
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
#include "bcm_encode_anqp.h"

/* encode ANQP query list */
int bcm_encode_anqp_query_list(bcm_encode_t *pkt, uint16 queryLen, uint16 *queryId)
{
	int initLen = bcm_encode_length(pkt);
	int i;

	bcm_encode_le16(pkt, ANQP_ID_QUERY_LIST);
	bcm_encode_le16(pkt, queryLen * sizeof(uint16));
	for (i = 0; i < queryLen; i++)
		bcm_encode_le16(pkt, queryId[i]);

	return bcm_encode_length(pkt) - initLen;
}

/* encode ANQP capability list */
int bcm_encode_anqp_capability_list(bcm_encode_t *pkt, uint16 capLen, uint16 *capList,
	uint16 vendorLen, uint8 *vendorList)
{
	int initLen = bcm_encode_length(pkt);
	int i;

	bcm_encode_le16(pkt, ANQP_ID_CAPABILITY_LIST);
	bcm_encode_le16(pkt, capLen * sizeof(uint16) + vendorLen);
	for (i = 0; i < capLen; i++) {
		bcm_encode_le16(pkt, capList[i]);
	}
	if (vendorLen > 0) {
		bcm_encode_bytes(pkt, vendorLen, vendorList);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode venue name duple */
int bcm_encode_anqp_venue_duple(bcm_encode_t *pkt, uint8 langLen, char *lang,
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

/* encode venue name */
int bcm_encode_anqp_venue_name(bcm_encode_t *pkt, uint8 group, uint8 type,
	uint16 nameLen, uint8 *name)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_VENUE_NAME_INFO);
	bcm_encode_le16(pkt, nameLen + 2);
	bcm_encode_byte(pkt, group);
	bcm_encode_byte(pkt, type);
	if (nameLen > 0) {
		bcm_encode_bytes(pkt, nameLen, name);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode network authentication unit */
int bcm_encode_anqp_network_authentication_unit(bcm_encode_t *pkt, uint8 type,
	uint16 urlLen, char *url)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, type);
	bcm_encode_le16(pkt, urlLen);
	if (urlLen > 0) {
		bcm_encode_bytes(pkt, urlLen, (uint8 *)url);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode network authentication type */
int bcm_encode_anqp_network_authentication_type(bcm_encode_t *pkt, uint16 len, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_NETWORK_AUTHENTICATION_TYPE_INFO);
	bcm_encode_le16(pkt, len);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, data);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode OI duple */
int bcm_encode_anqp_oi_duple(bcm_encode_t *pkt, uint8 oiLen, uint8 *oi)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, oiLen);
	if (oiLen) {
		bcm_encode_bytes(pkt, oiLen, oi);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode roaming consortium */
int bcm_encode_anqp_roaming_consortium(bcm_encode_t *pkt, uint16 len, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_ROAMING_CONSORTIUM_LIST);
	bcm_encode_le16(pkt, len);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, data);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode IP address type availability */
int bcm_encode_anqp_ip_type_availability(bcm_encode_t *pkt, uint8 ipv6, uint8 ipv4)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_IP_ADDRESS_TYPE_AVAILABILITY_INFO);
	bcm_encode_le16(pkt, 1);
	bcm_encode_byte(pkt, ((ipv4 << IPA_IPV4_SHIFT) & IPA_IPV4_MASK) |
		((ipv6 << IPA_IPV6_SHIFT) & IPA_IPV6_MASK));

	return bcm_encode_length(pkt) - initLen;
}

/* encode authentication parameter subfield */
int bcm_encode_anqp_authentication_subfield(bcm_encode_t *pkt,
	uint8 id, uint8 authLen, uint8 *auth)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, id);
	bcm_encode_byte(pkt, authLen);
	if (authLen > 0) {
		bcm_encode_bytes(pkt, authLen, auth);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode EAP method subfield */
int bcm_encode_anqp_eap_method_subfield(bcm_encode_t *pkt, uint8 method,
	uint8 authCount, uint8 authLen, uint8 *auth)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, authLen + 2);
	bcm_encode_byte(pkt, method);
	bcm_encode_byte(pkt, authCount);
	if (authLen > 0) {
		bcm_encode_bytes(pkt, authLen, auth);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode EAP method */
int bcm_encode_anqp_eap_method(bcm_encode_t *pkt, uint8 method,
	uint8 authCount, uint16 authLen, uint8 *auth)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, method);
	bcm_encode_byte(pkt, authCount);
	if (authLen > 0) {
		bcm_encode_bytes(pkt, authLen, auth);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode NAI realm data */
int bcm_encode_anqp_nai_realm_data(bcm_encode_t *pkt, uint8 encoding,
	uint8 realmLen, uint8 *realm,
	uint8 eapCount, uint16 eapLen, uint8 *eap)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, 3 + realmLen + eapLen);
	bcm_encode_byte(pkt, encoding);
	bcm_encode_byte(pkt, realmLen);
	if (realmLen > 0) {
		bcm_encode_bytes(pkt, realmLen, realm);
	}
	bcm_encode_byte(pkt, eapCount);
	if (eapLen > 0) {
		bcm_encode_bytes(pkt, eapLen, eap);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode NAI realm */
int bcm_encode_anqp_nai_realm(bcm_encode_t *pkt, uint16 realmCount,
	uint16 len, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_NAI_REALM_LIST);
	bcm_encode_le16(pkt, len + 2);
	bcm_encode_le16(pkt, realmCount);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, data);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode PLMN */
int bcm_encode_anqp_plmn(bcm_encode_t *pkt, char *mcc, char *mnc)
{
	int initLen = bcm_encode_length(pkt);
	int i;

	if (!(strlen(mcc) == 3 && (strlen(mnc) == 3 || strlen(mnc) == 2)))
		return 0;

	for (i = 0; i < (int)strlen(mcc); i++) {
		if (!isdigit(mcc[i]))
			return 0;
	}

	for (i = 0; i < (int)strlen(mnc); i++) {
		if (!isdigit(mnc[i]))
			return 0;
	}

	/* mcc digit 2 | mcc digit 1 */
	bcm_encode_byte(pkt, (mcc[1] - '0') << 4 | (mcc[0] - '0'));
	if (strlen(mnc) == 2)
		/* mnc digit 3 | mcc digit 3 */
		bcm_encode_byte(pkt, 0x0f << 4 | (mcc[2] - '0'));
	else
		/* mnc digit 3 | mcc digit 3 */
		bcm_encode_byte(pkt, (mnc[2] - '0') << 4 | (mcc[2] - '0'));
	/* mnc digit 2 | mnc digit 1 */
	bcm_encode_byte(pkt, (mnc[1] - '0') << 4 | (mnc[0] - '0'));

	return bcm_encode_length(pkt) - initLen;
}

/* encode 3GPP cellular network */
int bcm_encode_anqp_3gpp_cellular_network(bcm_encode_t *pkt,
	uint8 plmnCount, uint16 plmnLen, uint8 *plmnData)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_G3PP_CELLULAR_NETWORK_INFO);
	bcm_encode_le16(pkt, 5 + plmnLen);
	bcm_encode_byte(pkt, G3PP_GUD_VERSION);
	bcm_encode_byte(pkt, 3 + plmnLen);
	bcm_encode_byte(pkt, G3PP_PLMN_LIST_IE);
	bcm_encode_byte(pkt, 1 + plmnLen);
	bcm_encode_byte(pkt, plmnCount);
	if (plmnLen > 0) {
		bcm_encode_bytes(pkt, plmnLen, plmnData);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode domain name */
int bcm_encode_anqp_domain_name(bcm_encode_t *pkt, uint8 nameLen, char *name)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, nameLen);
	if (nameLen > 0) {
		bcm_encode_bytes(pkt, nameLen, (uint8 *)name);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode domain name list */
int bcm_encode_anqp_domain_name_list(bcm_encode_t *pkt, uint16 domainLen, uint8 *domain)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_DOMAIN_NAME_LIST);
	bcm_encode_le16(pkt, domainLen);
	if (domainLen > 0) {
		bcm_encode_bytes(pkt, domainLen, domain);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode WFA service discovery */
int bcm_encode_anqp_wfa_service_discovery(bcm_encode_t *pkt,
	uint16 serviceUpdateIndicator, uint16 dataLen, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_VENDOR_SPECIFIC_LIST);
	bcm_encode_le16(pkt, WFA_OUI_LEN + 1 + 2 + dataLen);
	bcm_encode_bytes(pkt, WFA_OUI_LEN, (uint8 *)WFA_OUI);
	bcm_encode_byte(pkt, ANQP_OUI_SUBTYPE);
	bcm_encode_le16(pkt, serviceUpdateIndicator);
	if (dataLen) {
		bcm_encode_bytes(pkt, dataLen, data);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode ANQP query vendor specific */
static int bcm_encode_anqp_query_vendor_specific_tlv(bcm_encode_t *pkt,
	uint8 serviceProtocolType, uint8 serviceTransactionId,
	int isStatusCode, uint8 statusCode,
	int isNumService, uint8 numService,
	uint16 queryLen, uint8 *queryData)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, 2 + (isStatusCode ? 1 : 0) +
		(isNumService ? 1 : 0) + queryLen);
	bcm_encode_byte(pkt, serviceProtocolType);
	bcm_encode_byte(pkt, serviceTransactionId);
	if (isStatusCode) {
		bcm_encode_byte(pkt, statusCode);
	}
	if (isNumService) {
		bcm_encode_byte(pkt, numService);
	}
	if (queryLen > 0) {
		bcm_encode_bytes(pkt, queryLen, queryData);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode ANQP query request vendor specific TLV */
int bcm_encode_anqp_query_request_vendor_specific_tlv(bcm_encode_t *pkt,
	uint8 serviceProtocolType, uint8 serviceTransactionId,
	uint16 queryLen, uint8 *queryData)
{
	return bcm_encode_anqp_query_vendor_specific_tlv(pkt,
		serviceProtocolType, serviceTransactionId,
		FALSE, 0, FALSE, 0, queryLen, queryData);
}

/* encode ANQP query response vendor specific TLV */
int bcm_encode_anqp_query_response_vendor_specific_tlv(bcm_encode_t *pkt,
	uint8 serviceProtocolType, uint8 serviceTransactionId, uint8 statusCode,
	int isNumService, uint8 numService,
	uint16 queryLen, uint8 *queryData)
{
	return bcm_encode_anqp_query_vendor_specific_tlv(pkt,
		serviceProtocolType, serviceTransactionId,
		TRUE, statusCode, isNumService, numService,
		queryLen, queryData);
}

/* encode WFDS request */
int bcm_encode_anqp_wfds_request(bcm_encode_t *pkt,
	uint8 serviceNameLen, uint8 *serviceName,
	uint8 serviceInfoReqLen, uint8 *serviceInfoReq)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, serviceNameLen);
	if (serviceNameLen > 0) {
		bcm_encode_bytes(pkt, serviceNameLen, serviceName);
	}
	bcm_encode_byte(pkt, serviceInfoReqLen);
	if (serviceInfoReqLen > 0) {
		bcm_encode_bytes(pkt, serviceInfoReqLen, serviceInfoReq);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode WFDS response */
int bcm_encode_anqp_wfds_response(bcm_encode_t *pkt,
	uint32 advertisementId, uint16 configMethod,
	uint8 serviceNameLen, uint8 *serviceName,
	uint8 serviceStatus,
	uint16 serviceInfoLen, uint8 *serviceInfo)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le32(pkt, advertisementId);
	bcm_encode_le16(pkt, configMethod);
	bcm_encode_byte(pkt, serviceNameLen);
	if (serviceNameLen > 0) {
		bcm_encode_bytes(pkt, serviceNameLen, serviceName);
	}
	bcm_encode_byte(pkt, serviceStatus);
	bcm_encode_le16(pkt, serviceInfoLen);
	if (serviceInfoLen > 0) {
		bcm_encode_bytes(pkt, serviceInfoLen, serviceInfo);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode AP LCI */
int bcm_encode_anqp_lci(bcm_encode_t *pkt, uint16 len, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_AP_GEOSPATIAL_LOCATION);
	bcm_encode_le16(pkt, len);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, data);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode AP Civic Location */
int bcm_encode_anqp_civic(bcm_encode_t *pkt, uint16 len, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_AP_CIVIC_LOCATION);
	bcm_encode_le16(pkt, len);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, data);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode AP Location Public Identifier URI/FQDN */
int bcm_encode_anqp_ap_location_public_id_uri(bcm_encode_t *pkt,
	uint16 len, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_AP_LOCATION_PUBLIC_ID_URI);
	bcm_encode_le16(pkt, len);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, data);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode Public Identifier URI/FQDN subelement */
int bcm_encode_hspot_anqp_ap_location_public_id_uri_data(bcm_encode_t *pkt,
	uint8 encoding, uint8 nameLen, char *name)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, PUBLIC_ID_URI_FQDN_SE_ID);
	bcm_encode_byte(pkt, nameLen + 1); /* include encoding in len */
	bcm_encode_byte(pkt, encoding);
	if (nameLen > 0) {
		bcm_encode_bytes(pkt, nameLen, (uint8 *)name);
	}

	return bcm_encode_length(pkt) - initLen;
}
/* encode neighbor report info */
int bcm_encode_anqp_neighbor_report(bcm_encode_t *pkt, uint16 len, uint8 *data)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_le16(pkt, ANQP_ID_NEIGHBOR_REPORT);
	bcm_encode_le16(pkt, len);
	if (len > 0) {
		bcm_encode_bytes(pkt, len, data);
	}

	return bcm_encode_length(pkt) - initLen;
}
