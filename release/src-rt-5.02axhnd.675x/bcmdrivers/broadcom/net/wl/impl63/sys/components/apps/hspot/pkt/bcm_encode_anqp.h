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

#ifndef _BCM_ENCODE_ANQP_H_
#define _BCM_ENCODE_ANQP_H_

#include "typedefs.h"
#include "bcm_encode.h"

/* encode ANQP query list */
int bcm_encode_anqp_query_list(bcm_encode_t *pkt, uint16 queryLen, uint16 *queryId);

/* encode ANQP capability list */
int bcm_encode_anqp_capability_list(bcm_encode_t *pkt, uint16 capLen, uint16 *capList,
	uint16 vendorLen, uint8 *vendorList);

/* encode venue name duple */
int bcm_encode_anqp_venue_duple(bcm_encode_t *pkt, uint8 langLen, char *lang,
	uint8 nameLen, char *name);

/* encode venue name */
int bcm_encode_anqp_venue_name(bcm_encode_t *pkt, uint8 group, uint8 type,
	uint16 nameLen, uint8 *name);

/* encode network authentication unit */
int bcm_encode_anqp_network_authentication_unit(bcm_encode_t *pkt, uint8 type,
	uint16 urlLen, char *url);

/* encode network authentication type */
int bcm_encode_anqp_network_authentication_type(bcm_encode_t *pkt, uint16 len, uint8 *data);

/* encode OI duple */
int bcm_encode_anqp_oi_duple(bcm_encode_t *pkt, uint8 oiLen, uint8 *oi);

/* encode roaming consortium */
int bcm_encode_anqp_roaming_consortium(bcm_encode_t *pkt, uint16 len, uint8 *data);

/* encode IP address type availability */
int bcm_encode_anqp_ip_type_availability(bcm_encode_t *pkt, uint8 ipv6, uint8 ipv4);

/* encode authentication parameter subfield */
int bcm_encode_anqp_authentication_subfield(bcm_encode_t *pkt,
	uint8 id, uint8 authLen, uint8 *auth);

/* encode EAP method subfield */
int bcm_encode_anqp_eap_method_subfield(bcm_encode_t *pkt, uint8 method,
	uint8 authCount, uint8 authLen, uint8 *auth);

/* encode EAP method */
int bcm_encode_anqp_eap_method(bcm_encode_t *pkt, uint8 method,
	uint8 authCount, uint16 authLen, uint8 *auth);

/* encode NAI realm data */
int bcm_encode_anqp_nai_realm_data(bcm_encode_t *pkt, uint8 encoding,
	uint8 realmLen, uint8 *realm,
	uint8 eapCount, uint16 eapLen, uint8 *eap);

/* encode NAI realm */
int bcm_encode_anqp_nai_realm(bcm_encode_t *pkt,
	uint16 realmCount, uint16 len, uint8 *data);

/* encode PLMN */
int bcm_encode_anqp_plmn(bcm_encode_t *pkt, char *mcc, char *mnc);

/* encode 3GPP cellular network */
int bcm_encode_anqp_3gpp_cellular_network(bcm_encode_t *pkt,
	uint8 plmnCount, uint16 plmnLen, uint8 *plmnData);

/* encode domain name */
int bcm_encode_anqp_domain_name(bcm_encode_t *pkt, uint8 nameLen, char *name);

/* encode domain name list */
int bcm_encode_anqp_domain_name_list(bcm_encode_t *pkt, uint16 domainLen, uint8 *domain);

/* encode WFA service discovery */
int bcm_encode_anqp_wfa_service_discovery(bcm_encode_t *pkt,
	uint16 serviceUpdateIndicator, uint16 dataLen, uint8 *data);

/* encode ANQP query request vendor specific TLV */
int bcm_encode_anqp_query_request_vendor_specific_tlv(bcm_encode_t *pkt,
	uint8 serviceProtocolType, uint8 serviceTransactionId,
	uint16 queryLen, uint8 *queryData);

/* encode ANQP query response vendor specific TLV */
int bcm_encode_anqp_query_response_vendor_specific_tlv(bcm_encode_t *pkt,
	uint8 serviceProtocolType, uint8 serviceTransactionId, uint8 statusCode,
	int isNumService, uint8 numService,
	uint16 queryLen, uint8 *queryData);

/* encode WFDS request */
int bcm_encode_anqp_wfds_request(bcm_encode_t *pkt,
	uint8 serviceNameLen, uint8 *serviceName,
	uint8 serviceInfoReqLen, uint8 *serviceInfoReq);

/* encode WFDS response */
int bcm_encode_anqp_wfds_response(bcm_encode_t *pkt,
	uint32 advertisementId, uint16 configMethod,
	uint8 serviceNameLen, uint8 *serviceName,
	uint8 serviceStatus,
	uint16 serviceInfoLen, uint8 *serviceInfo);

/* encode AP LCI */
int bcm_encode_anqp_lci(bcm_encode_t *pkt, uint16 len, uint8 *data);

/* encode AP Civic Location */
int bcm_encode_anqp_civic(bcm_encode_t *pkt, uint16 len, uint8 *data);

/* encode AP Location Public Identifier URI/FQDN */
int bcm_encode_anqp_ap_location_public_id_uri(bcm_encode_t *pkt,
	uint16 len, uint8 *data);

/* encode Public Identifier URI/FQDN subelement */
int bcm_encode_hspot_anqp_ap_location_public_id_uri_data(bcm_encode_t *pkt,
	uint8 encoding, uint8 nameLen, char *name);

/* encode anqp neighbor report */
int bcm_encode_anqp_neighbor_report(bcm_encode_t *pkt, uint16 len, uint8 *data);
#endif /* _BCM_ENCODE_ANQP_H_ */
