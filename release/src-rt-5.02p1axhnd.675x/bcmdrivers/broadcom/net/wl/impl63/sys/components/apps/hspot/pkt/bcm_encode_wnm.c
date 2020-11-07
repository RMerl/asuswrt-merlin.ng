/*
 * Encoding of WNM packets.
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
#include "bcm_hspot.h"
#include "bcm_encode_wnm.h"

/* encode WNM-notification request for subscription remediation */
int bcm_encode_wnm_subscription_remediation(bcm_encode_t *pkt,
	uint8 dialogToken, uint8 urlLen, char *url, uint8 serverMethod)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_WNM);
	bcm_encode_byte(pkt, DOT11_WNM_ACTION_NOTFCTN_REQ);
	bcm_encode_byte(pkt, dialogToken);
	bcm_encode_byte(pkt, HSPOT_WNM_TYPE);
	bcm_encode_byte(pkt, DOT11_MNG_VS_ID);
	if (urlLen > 0) {
		bcm_encode_byte(pkt, 6 + urlLen);
	}
	else
		bcm_encode_byte(pkt, 5);
	bcm_encode_bytes(pkt, WFA_OUI_LEN, (uint8 *)WFA_OUI);
	bcm_encode_byte(pkt, HSPOT_WNM_SUBSCRIPTION_REMEDIATION);
	bcm_encode_byte(pkt, urlLen);
	if (urlLen > 0) {
		bcm_encode_bytes(pkt, urlLen, (uint8 *)url);
		bcm_encode_byte(pkt, serverMethod);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode WNM-notification request for deauthentication imminent */
int bcm_encode_wnm_deauthentication_imminent(bcm_encode_t *pkt,
	uint8 dialogToken, uint8 reason, uint16 reauthDelay, uint8 urlLen, char *url)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_WNM);
	bcm_encode_byte(pkt, DOT11_WNM_ACTION_NOTFCTN_REQ);
	bcm_encode_byte(pkt, dialogToken);
	bcm_encode_byte(pkt, HSPOT_WNM_TYPE);
	bcm_encode_byte(pkt, DOT11_MNG_VS_ID);
	bcm_encode_byte(pkt, 8 + urlLen);
	bcm_encode_bytes(pkt, WFA_OUI_LEN, (uint8 *)WFA_OUI);
	bcm_encode_byte(pkt, HSPOT_WNM_DEAUTHENTICATION_IMMINENT);
	bcm_encode_byte(pkt, reason);
	bcm_encode_le16(pkt, reauthDelay);
	bcm_encode_byte(pkt, urlLen);
	if (urlLen > 0) {
		bcm_encode_bytes(pkt, urlLen, (uint8 *)url);
	}

	return bcm_encode_length(pkt) - initLen;
}
