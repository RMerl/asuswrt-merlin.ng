/*
 * Decode functions which provides decoding of P2P attributes
 * as defined in P2P specification.
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

#ifndef _BCM_DECODE_P2P_H_
#define _BCM_DECODE_P2P_H_

#include "typedefs.h"
#include "bcm_decode.h"

typedef struct {
#ifndef BCMDRIVER	/* not used in dongle */
	int statusLength;
	uint8 *statusBuffer;
	int minorReasonCodeLength;
	uint8 *minorReasonCodeBuffer;
#endif	/* BCMDRIVER */
	int capabilityLength;
	uint8 *capabilityBuffer;
#ifndef BCMDRIVER	/* not used in dongle */
	int deviceIdLength;
	uint8 *deviceIdBuffer;
	int groupOwnerIntentLength;
	uint8 *groupOwnerIntentBuffer;
	int configurationTimeoutLength;
	uint8 *configurationTimeoutBuffer;
	int listenChannelLength;
	uint8 *listenChannelBuffer;
	int groupBssidLength;
	uint8 *groupBssidBuffer;
	int extendedListenTimingLength;
	uint8 *extendedListenTimingBuffer;
	int intendedInterfaceAddressLength;
	uint8 *intendedInterfaceAddressBuffer;
	int manageabilityLength;
	uint8 *manageabilityBuffer;
	int channelListLength;
	uint8 *channelListBuffer;
	int noticeOfAbsenceLength;
	uint8 *noticeOfAbsenseBuffer;
#endif	/* BCMDRIVER */
	int deviceInfoLength;
	uint8 *deviceInfoBuffer;
#ifndef BCMDRIVER	/* not used in dongle */
	int groupInfoLength;
	uint8 *groupInfoBuffer;
	int groupIdLength;
	uint8 *groupIdBuffer;
	int interfaceLength;
	uint8 *interfaceBuffer;
	int operatingChannelLength;
	uint8 *operatingChannelBuffer;
	int invitationFlagsLength;
	uint8 *invitationFlagsBuffer;
	int serviceHashLength;
	uint8 *serviceHashBuffer;
	int sessionLength;
	uint8 *sessionBuffer;
	int connectCapLength;
	uint8 *connectCapBuffer;
	int advertiseIdLength;
	uint8 *advertiseIdBuffer;
#endif	/* BCMDRIVER */
	int advertiseServiceLength;
	uint8 *advertiseServiceBuffer;
#ifndef BCMDRIVER	/* not used in dongle */
	int sessionIdLength;
	uint8 *sessionIdBuffer;
	int featureCapLength;
	uint8 *featureCapBuffer;
	int persistentGroupLength;
	uint8 *persistentGroupBuffer;
#endif	/* BCMDRIVER */
} bcm_decode_p2p_t;

/* decode P2P */
int bcm_decode_p2p(bcm_decode_t *pkt, bcm_decode_p2p_t *wfd);

typedef uint8 bcm_decode_p2p_device_type_t[8];
#define BCM_DECODE_P2P_MAX_SECONDARY_DEVICE_TYPE	4
#define BCM_DECODE_P2P_MAX_DEVICE_NAME			32

typedef struct
{
	struct ether_addr deviceAddress;
	uint16 configMethods;
	bcm_decode_p2p_device_type_t primaryType;
	uint8 numSecondaryType;
	bcm_decode_p2p_device_type_t secondaryType[BCM_DECODE_P2P_MAX_SECONDARY_DEVICE_TYPE];
	uint8 deviceName[BCM_DECODE_P2P_MAX_DEVICE_NAME + 1];
} bcm_decode_p2p_device_info_t;

/* decode device info */
int bcm_decode_p2p_device_info(bcm_decode_t *pkt, bcm_decode_p2p_device_info_t *device);

/* print decoded device info */
void bcm_decode_p2p_device_info_print(bcm_decode_p2p_device_info_t *device);

typedef struct
{
	uint8 device;
	uint8 group;
} bcm_decode_p2p_capability_t;

/* decode capability */
int bcm_decode_p2p_capability(bcm_decode_t *pkt, bcm_decode_p2p_capability_t *capability);

/* print decoded capability */
void bcm_decode_p2p_capability_print(bcm_decode_p2p_capability_t *capability);

#endif /* _BCM_DECODE_P2P_H_ */
