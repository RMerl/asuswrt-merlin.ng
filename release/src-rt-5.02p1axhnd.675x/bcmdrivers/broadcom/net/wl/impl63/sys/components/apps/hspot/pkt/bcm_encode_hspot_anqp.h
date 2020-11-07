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

#ifndef _BCM_ENCODE_HSPOT_ANQP_H_
#define _BCM_ENCODE_HSPOT_ANQP_H_

#include "typedefs.h"
#include "bcm_encode.h"
#include "bcm_hspot.h"

/* encode query list */
int bcm_encode_hspot_anqp_query_list(bcm_encode_t *pkt, uint16 queryLen, uint8 *query);

/* encode capability list */
int bcm_encode_hspot_anqp_capability_list(bcm_encode_t *pkt, uint16 capLen, uint8 *cap);

/* encode operator friendly name */
int bcm_encode_hspot_anqp_operator_name_duple(bcm_encode_t *pkt, uint8 langLen, char *lang,
	uint8 nameLen, char *name);
int bcm_encode_hspot_anqp_operator_friendly_name(bcm_encode_t *pkt, uint16 nameLen, uint8 *name);

/* encode WAN metrics */
int bcm_encode_hspot_anqp_wan_metrics(bcm_encode_t *pkt, uint8 linkStatus, uint8 symmetricLink,
	uint8 atCapacity, uint32 dlinkSpeed, uint32 ulinkSpeed,
	uint8 dlinkLoad, uint8 ulinkLoad, uint16 lmd);

/* encode connection capability */
int bcm_encode_hspot_anqp_proto_port_tuple(bcm_encode_t *pkt,
	uint8 ipProtocol, uint16 portNumber, uint8 status);
int bcm_encode_hspot_anqp_connection_capability(bcm_encode_t *pkt, uint16 capLen, uint8 *cap);

/* encode NAI home realm query */
int bcm_encode_hspot_anqp_nai_home_realm_name(bcm_encode_t *pkt, uint8 encoding,
	uint8 nameLen, char *name);
int pktEncodeHspotAnqpNaiHomeRealmQuery(bcm_encode_t *pkt, uint8 count,
	uint16 nameLen, uint8 *name);

/* encode operating class indication */
int bcm_encode_hspot_anqp_operating_class_indication(bcm_encode_t *pkt,
	uint16 opClassLen, uint8 *opClass);

/* encode icon metadata */
int bcm_encode_hspot_anqp_icon_metadata(bcm_encode_t *pkt,
	uint16 width, uint16 height, char *lang,
	uint8 typeLength, uint8 *type, uint8 filenameLength, uint8 *filename);
/* encode OSU provider */
int bcm_encode_hspot_anqp_osu_provider(bcm_encode_t *pkt,
	uint16 nameLength, uint8 *name, uint8 uriLength, uint8 *uri,
	uint8 methodLength, uint8 *method, uint16 iconLength, uint8 *icon,
	uint8 naiLength, uint8 *nai, uint16 descLength, uint8 *desc);
/* encode OSU provider list */
int bcm_encode_hspot_anqp_osu_provider_list(bcm_encode_t *pkt,
	uint8 osuSsidLength, uint8 *osuSsid,
	uint8 numOsuProvider, uint16 providerLength, uint8 *provider);

/* encode anonymous NAI */
int bcm_encode_hspot_anqp_anonymous_nai(bcm_encode_t *pkt, uint16 length, uint8 *nai);

/* encode icon request */
int bcm_encode_hspot_anqp_icon_request(bcm_encode_t *pkt, uint16 length, uint8 *filename);

/* encode icon binary file */
int bcm_encode_hspot_anqp_icon_binary_file(bcm_encode_t *pkt,
	uint8 status, uint8 typeLength, uint8 *type, uint16 length, uint8 *data);

#endif /* _BCM_ENCODE_HSPOT_ANQP_H_ */
