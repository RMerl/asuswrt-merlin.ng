/*
 * Encode functions which provides encoding of information elements
 * as defined in 802.11.
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

#ifndef _BCM_ENCODE_IE_H_
#define _BCM_ENCODE_IE_H_

#include "typedefs.h"
#include "bcm_encode.h"
#include "bcm_hspot.h"

/* encode hotspot 2.0 indication */
int bcm_encode_ie_hotspot_indication(bcm_encode_t *pkt, uint8 hotspotConfig);

/* encode hotspot 2.0 indication release2 */
int bcm_encode_ie_hotspot_indication2(bcm_encode_t *pkt,
	int isDgafDisabled, uint8 releaseNumber,
	int isPpsMoIdPresent, uint16 ppsMoId,
	int isAnqpDomainIdPresent, uint16 anqpDomainId);

/* encode interworking */
int bcm_encode_ie_interworking(bcm_encode_t *pkt, uint8 accessNetworkType,
	int isInternet, int isAsra, int isEsr, int isUesa,
	int isVenue, uint8 venueGroup, uint8 venueType, struct ether_addr *hessid);

/* encode advertisement protocol tuple */
int bcm_encode_ie_advertisement_protocol_tuple(bcm_encode_t *pkt,
	int isPamebi, uint8 qResponseLimit, uint8 protocolId);

/* encode advertisement protocol */
int bcm_encode_ie_advertisement_protocol_from_tuple(bcm_encode_t *pkt, uint8 len, uint8 *data);

/* encode roaming consortium */
int bcm_encode_ie_roaming_consortium(bcm_encode_t *pkt, uint8 numAnqpOi,
	uint8 oi1Len, uint8 *oi1, uint8 oi2Len, uint8 *oi2,
	uint8 oi3Len, uint8 *oi3);

/* encode extended capabilities */
int bcm_encode_ie_extended_capabilities(bcm_encode_t *pkt, uint32 cap);

/* encode advertisement protocol */
int bcm_encode_ie_advertisement_protocol(bcm_encode_t *pkt,
	uint8 pamebi, uint8 qRspLimit, uint8 id);

/* encode qbss load */
int bcm_encode_ie_bss_load(bcm_encode_t *pkt, uint16 stationCount,
	uint8 channelUtilization, uint16 availableAdmissionCapacity);

#endif /* _BCM_ENCODE_IE_H_ */
