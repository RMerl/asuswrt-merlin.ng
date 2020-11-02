/*
 * Decoding of WNM packets.
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

#ifndef _BCM_DECODE_WNM_H_
#define _BCM_DECODE_WNM_H_

#include "typedefs.h"
#include "bcm_decode.h"
#include "bcm_hspot.h"

#define BCM_DECODE_WNM_MAX_SERVER_URL_LENGTH	255
typedef struct
{
	uint8 dialogToken;
	uint8 urlLength;
	char url[BCM_DECODE_WNM_MAX_SERVER_URL_LENGTH + 1];	/* null terminated */
	uint8 serverMethod;
} bcm_decode_wnm_subscription_remediation_t;

/* decode WNM-notification request for subscription remediation */
int bcm_decode_wnm_subscription_remediation(bcm_decode_t *pkt,
	bcm_decode_wnm_subscription_remediation_t *wnm);

#define BCM_DECODE_WNM_MAX_REASON_URL_LENGTH	255
typedef struct
{
	uint8 dialogToken;
	uint8 reason;
	uint16 reauthDelay;
	uint8 urlLength;
	char url[BCM_DECODE_WNM_MAX_REASON_URL_LENGTH + 1];	/* null terminated */
} bcm_decode_wnm_deauthentication_imminent_t;

/* decode WNM-notification request for deauthentication imminent */
int bcm_decode_wnm_deauthentication_imminent(bcm_decode_t *pkt,
	bcm_decode_wnm_deauthentication_imminent_t *wnm);

#endif /* _BCM_DECODE_WNM_H_ */
