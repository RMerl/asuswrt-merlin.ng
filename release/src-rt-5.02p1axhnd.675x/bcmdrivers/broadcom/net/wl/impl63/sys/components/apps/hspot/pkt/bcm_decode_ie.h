/*
 * Decode functions which provides decoding of information elements
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

#ifndef _BCM_DECODE_IE_H_
#define _BCM_DECODE_IE_H_

#include "typedefs.h"
#include "wlioctl.h"
#include "bcm_decode.h"
#include "bcm_decode_p2p.h"
#include "bcm_hspot.h"

#define BCM_DECODE_MAX_IE_FRAGMENTS	8

typedef struct {
	int dsLength;
	uint8 *ds;
#ifndef BCMDRIVER	/* not used in dongle */
	int bssLoadLength;
	uint8 *bssLoad;
	int timeAdvertisementLength;
	uint8 *timeAdvertisement;
	int timeZoneLength;
	uint8 *timeZone;
	int interworkingLength;
	uint8 *interworking;
	int advertisementProtocolLength;
	uint8 *advertisementProtocol;
	int expeditedBandwidthRequestLength;
	uint8 *expeditedBandwidthRequest;
	int qosMapSetLength;
	uint8 *qosMapSet;
	int roamingConsortiumLength;
	uint8 *roamingConsortium;
	int emergencyAlertLength;
	uint8 *emergencyAlert;
	int extendedCapabilityLength;
	uint8 *extendedCapability;
	int rsnInfoLength;
	uint8 *rsnInfo;
#endif	/* BCMDRIVER */

	/* vendor specific */
	int hotspotIndicationLength;
	uint8 *hotspotIndication;
	int osenIeLength;
	uint8 *osenIe;
	int wpsIeLength;
	uint8 *wpsIe;
	struct {
		int p2pIeLength;
		uint8 *p2pIe;
	} p2p[BCM_DECODE_MAX_IE_FRAGMENTS];
} bcm_decode_ie_t;

/* decode vendor IE */
int bcm_decode_ie(bcm_decode_t *pkt, bcm_decode_ie_t *ie);

/* calculate length of all P2P IEs */
int bcm_decode_ie_get_p2p_ie_length(bcm_decode_t *pkt, bcm_decode_ie_t *ie);

/* copy concatenated P2P IEs into buf */
/* buf must be of size returned by bcm_decode_ie_get_p2p_ie_length() */
uint8 *bcm_decode_ie_get_p2p_ie(bcm_decode_t *pkt, bcm_decode_ie_t *ie, uint8 *buf);

/* decode hotspot 2.0 indication */
int bcm_decode_ie_hotspot_indication(bcm_decode_t *pkt, uint8 *hotspotConfig);

typedef struct
{
	int isDgafDisabled;
	uint8 releaseNumber;
	int isPpsMoIdPresent;
	uint16 ppsMoId;
	int isAnqpDomainIdPresent;
	uint16 anqpDomainId;
} bcm_decode_hotspot_indication_t;

/* decode hotspot 2.0 indication release2 */
int bcm_decode_ie_hotspot_indication2(bcm_decode_t *pkt, bcm_decode_hotspot_indication_t *hotspot);

/* decode OSEN */
int bcm_decode_ie_osen(bcm_decode_t *pkt);

typedef struct
{
	uint8 accessNetworkType;
	int isInternet;
	int isAsra;
	int isEsr;
	int isUesa;
	int isVenue;
	uint8 venueGroup;
	uint8 venueType;
	int isHessid;
	struct ether_addr hessid;
} bcm_decode_interworking_t;

/* decode interworking */
int bcm_decode_ie_interworking(bcm_decode_t *pkt, bcm_decode_interworking_t *interworking);

typedef struct
{
	uint8 queryResponseLimit;
	int isPamebi;
	uint8 protocolId;
} bcm_decode_ie_adv_proto_tuple_t;

/* decode advertisement protocol tuple */
int bcm_decode_ie_advertisement_protocol_tuple(bcm_decode_t *pkt,
	bcm_decode_ie_adv_proto_tuple_t *tuple);

#define BCM_DECODE_IE_MAX_ADVERTISEMENT_PROTOCOL	8
typedef struct
{
	int count;
	bcm_decode_ie_adv_proto_tuple_t protocol[BCM_DECODE_IE_MAX_ADVERTISEMENT_PROTOCOL];
} bcm_decode_advertisement_protocol_t;

/* decode advertisement protocol */
int bcm_decode_ie_advertisement_protocol(bcm_decode_t *pkt,
	bcm_decode_advertisement_protocol_t *advertise);

#define BCM_DECODE_IE_MAX_IE_OI_LENGTH	15
typedef struct
{
	uint8 length;
	uint8 data[BCM_DECODE_IE_MAX_IE_OI_LENGTH];
} bcm_decode_oi_t;

#define BCM_DECODE_IE_MAX_IE_OI	3
typedef struct
{
	uint8 anqpOiCount;
	uint8 count;
	bcm_decode_oi_t oi[BCM_DECODE_IE_MAX_IE_OI];
} bcm_decode_roaming_consortium_t;

/* decode roaming consortium */
int bcm_decode_ie_roaming_consortium(bcm_decode_t *pkt, bcm_decode_roaming_consortium_t *roam);

/* decode extended capabilities */
int bcm_decode_ie_extended_capabilities(bcm_decode_t *pkt, uint32 *cap);

typedef struct
{
	uint16 year;
	uint8 month;
	uint8 day;
	uint8 hours;
	uint8 minutes;
	uint8 seconds;
	uint16 milliseconds;
	uint8 reserved;
} bcm_decode_time_t;

#define BCM_DECODE_IE_TIME_ERROR_LENGTH		5
#define BCM_DECODE_IE_TIME_UPDATE_LENGTH	1

typedef struct
{
	uint8 capabilities;
	bcm_decode_time_t timeValue;
	uint8 timeError[BCM_DECODE_IE_TIME_ERROR_LENGTH];
	uint8 timeUpdate[BCM_DECODE_IE_TIME_UPDATE_LENGTH];
} bcm_decode_time_advertisement_t;

/* decode time advertisement */
int bcm_decode_ie_time_advertisement(bcm_decode_t *pkt, bcm_decode_time_advertisement_t *time);

#define BCM_DECODE_IE_TIME_ZONE_LENGTH	255
typedef char bcm_decode_time_zone_t[BCM_DECODE_IE_TIME_ZONE_LENGTH + 1]; /* null terminated */

/* decode time zone */
int bcm_decode_ie_time_zone(bcm_decode_t *pkt, bcm_decode_time_zone_t *zone);

typedef struct
{
	uint16 stationCount;
	uint8 channelUtilization;
	uint16 availableAdmissionCapacity;
} bcm_decode_bss_load_t;

/* decode BSS load */
int bcm_decode_ie_bss_load(bcm_decode_t *pkt, bcm_decode_bss_load_t *load);

#define MAX_CIPHER_SUITE	8
#define PKMID_LENGTH		16

typedef struct
{
	uint16 version;
	uint32 groupCipherSuite;
	uint16 pairwiseCipherSuiteCount;
	uint32 pairwiseCipherSuite[MAX_CIPHER_SUITE];
	uint16 akmSuiteCount;
	uint32 akmSuite[MAX_CIPHER_SUITE];
	uint16 rsnCapabilities;
	uint16 pkmidCount;
	uint8 pkmid[MAX_CIPHER_SUITE][PKMID_LENGTH];
	uint32 groupManagementCipherSuite;
} bcm_decode_rsn_info_t;

/* decode RSN info */
int bcm_decode_ie_rsn_info(bcm_decode_t *pkt, bcm_decode_rsn_info_t *rsn);

typedef struct
{
	uint16 channel;
	int isP2P;
	int isP2PDeviceInfoDecoded;
	bcm_decode_p2p_device_info_t p2pDeviceInfo;
	int isP2PCapabilityDecoded;
	bcm_decode_p2p_capability_t p2pCapability;
	int isHotspotDecoded;
	uint8 hotspotConfig;
} bcm_decode_probe_response_t;

/* decode probe response from wl_bss_info_t */
int bcm_decode_ie_probe_response(wl_bss_info_t *bi, bcm_decode_probe_response_t *pr);

#endif /* _BCM_DECODE_IE_H_ */
