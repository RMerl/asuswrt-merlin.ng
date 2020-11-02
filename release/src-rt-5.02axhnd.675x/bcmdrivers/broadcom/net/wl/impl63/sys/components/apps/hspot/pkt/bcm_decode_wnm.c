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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "802.11.h"
#include "trace.h"
#include "bcm_hspot.h"
#include "bcm_decode_wnm.h"

/* decode WNM-notification request for subscription remediation */
int bcm_decode_wnm_subscription_remediation(
	bcm_decode_t *pkt, bcm_decode_wnm_subscription_remediation_t *wnm)
{
	uint8 byte, len;
	uint8 oui[WFA_OUI_LEN];

	WL_PRPKT("packet for WNM subscription remediation decoding",
		bcm_decode_buf(pkt), bcm_decode_buf_length(pkt));

	memset(wnm, 0, sizeof(*wnm));

	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_ACTION_CAT_WNM) {
		WL_ERROR(("WNM action category\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_WNM_ACTION_NOTFCTN_REQ) {
		WL_ERROR(("WNM notifcation request\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &wnm->dialogToken)) {
		WL_ERROR(("dialog token\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != HSPOT_WNM_TYPE) {
		WL_ERROR(("WNM type\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_MNG_VS_ID) {
		WL_ERROR(("vendor specific ID\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &len) || len < 6) {
		WL_ERROR(("length\n"));
		return FALSE;
	}
	if (len > bcm_decode_remaining(pkt)) {
		WL_ERROR(("length exceeds packet %d > %d\n",
			len, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (!bcm_decode_bytes(pkt, WFA_OUI_LEN, oui) ||
		memcmp(oui, WFA_OUI, WFA_OUI_LEN) != 0) {
		WL_ERROR(("WFA OUI\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) ||
		byte != HSPOT_WNM_SUBSCRIPTION_REMEDIATION) {
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &wnm->urlLength) ||
		wnm->urlLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("URL length\n"));
		return FALSE;
	}
	if (wnm->urlLength > 0) {
		if (!bcm_decode_bytes(pkt, wnm->urlLength, (uint8 *)wnm->url)) {
			WL_ERROR(("URL\n"));
			return FALSE;
		}
	}
	wnm->url[wnm->urlLength] = 0;
	if (bcm_decode_remaining(pkt) > 0 &&
		bcm_decode_byte(pkt, &wnm->serverMethod)) {
	}

	return TRUE;
}

/* decode WNM-notification request for deauthentication imminent */
int bcm_decode_wnm_deauthentication_imminent(bcm_decode_t *pkt,
	bcm_decode_wnm_deauthentication_imminent_t *wnm)
{
	uint8 byte, len;
	uint8 oui[WFA_OUI_LEN];

	WL_PRPKT("packet for WNM deauthentication imminent decoding",
		bcm_decode_buf(pkt), bcm_decode_buf_length(pkt));

	memset(wnm, 0, sizeof(*wnm));

	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_ACTION_CAT_WNM) {
		WL_ERROR(("WNM action category\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_WNM_ACTION_NOTFCTN_REQ) {
		WL_ERROR(("WNM notifcation request\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &wnm->dialogToken)) {
		WL_ERROR(("dialog token\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != HSPOT_WNM_TYPE) {
		WL_ERROR(("WNM type\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_MNG_VS_ID) {
		WL_ERROR(("vendor specific ID\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &len) || len < 8) {
		WL_ERROR(("length\n"));
		return FALSE;
	}
	if (len > bcm_decode_remaining(pkt)) {
		WL_ERROR(("length exceeds packet %d > %d\n",
			len, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (!bcm_decode_bytes(pkt, WFA_OUI_LEN, oui) ||
		memcmp(oui, WFA_OUI, WFA_OUI_LEN) != 0) {
		WL_ERROR(("WFA OUI\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) ||
		byte != HSPOT_WNM_DEAUTHENTICATION_IMMINENT) {
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &wnm->reason)) {
		WL_ERROR(("deauth reason\n"));
		return FALSE;
	}
	if (!bcm_decode_le16(pkt, &wnm->reauthDelay)) {
		WL_ERROR(("reauth delay\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &wnm->urlLength) ||
		wnm->urlLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("URL length\n"));
		return FALSE;
	}
	if (wnm->urlLength > 0) {
		if (!bcm_decode_bytes(pkt, wnm->urlLength, (uint8 *)wnm->url)) {
			WL_ERROR(("URL\n"));
			return FALSE;
		}
	}
	wnm->url[wnm->urlLength] = 0;

	return TRUE;
}
