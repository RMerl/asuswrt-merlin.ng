/*
 * Decode functions which provides decoding of GAS packets as defined in 802.11u.
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
#include "bcm_decode_ie.h"
#include "bcm_decode_gas.h"

static void printGasFrame(bcm_decode_gas_t *gasDecode)
{
	WL_P2PO(("decoded GAS frame:\n"));
	WL_P2PO(("   category = %d\n", gasDecode->category));
	WL_P2PO(("   action = %d (%s)\n", gasDecode->action,
		gasDecode->action == GAS_REQUEST_ACTION_FRAME ? "GAS REQUEST" :
		gasDecode->action == GAS_RESPONSE_ACTION_FRAME ? "GAS RESPONSE" :
		gasDecode->action == GAS_COMEBACK_REQUEST_ACTION_FRAME ?
			"GAS COMEBACK REQUEST" :
		gasDecode->action == GAS_COMEBACK_RESPONSE_ACTION_FRAME ?
			"GAS COMEBACK RESPONSE" :
		"?"));
	WL_P2PO(("   dialog token = %d\n", gasDecode->dialogToken));
	switch (gasDecode->action)
	{
	case GAS_REQUEST_ACTION_FRAME:
		WL_P2PO(("   advertisement protocol ID = %d\n",
			gasDecode->request.apie.protocolId));
		WL_PRPKT("   request",
			gasDecode->request.req, gasDecode->request.reqLen);
		break;
	case GAS_RESPONSE_ACTION_FRAME:
		WL_P2PO(("   status code = %d\n",
			gasDecode->response.statusCode));
		WL_P2PO(("   comeback delay = 0x%04x\n",
			gasDecode->response.comebackDelay));
		WL_P2PO(("   advertisement protocol ID = %d\n",
			gasDecode->response.apie.protocolId));
		WL_PRPKT("   response",
			gasDecode->response.rsp, gasDecode->response.rspLen);
		break;
	case GAS_COMEBACK_REQUEST_ACTION_FRAME:
		/* nothing */
		break;
	case GAS_COMEBACK_RESPONSE_ACTION_FRAME:
		WL_P2PO(("   status code = %d\n",
			gasDecode->comebackResponse.statusCode));
		WL_P2PO(("   fragment ID = 0x%02x\n",
			gasDecode->comebackResponse.fragmentId));
		WL_P2PO(("   comeback delay = 0x%04x\n",
			gasDecode->comebackResponse.comebackDelay));
		WL_P2PO(("   advertisement protocol ID = %d\n",
			gasDecode->comebackResponse.apie.protocolId));
		WL_PRPKT("   response",
			gasDecode->comebackResponse.rsp, gasDecode->comebackResponse.rspLen);
		break;
	default:
		break;
	}
}

static int decodeAdvertisementProtocol(bcm_decode_t *pkt,
	bcm_decode_ie_adv_proto_tuple_t *ap)
{
	uint8 ie, len;

	if (!bcm_decode_byte(pkt, &ie) || ie != DOT11_MNG_ADVERTISEMENT_ID) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &len) || len != 2) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_ie_advertisement_protocol_tuple(pkt, ap)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	return TRUE;
}

static int decodeReqRsp(bcm_decode_t *pkt, uint16 *rLen, uint8 **r)
{
	if (!bcm_decode_le16(pkt, rLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (bcm_decode_remaining(pkt) < *rLen) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	*r = bcm_decode_current_ptr(pkt);
	bcm_decode_offset_set(pkt, bcm_decode_offset(pkt) + *rLen);

	return TRUE;
}

int bcm_decode_gas(bcm_decode_t *pkt, bcm_decode_gas_t *gasDecode)
{

	memset(gasDecode, 0, sizeof(*gasDecode));

	if (bcm_decode_remaining(pkt) < 3) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	(void)bcm_decode_byte(pkt, &gasDecode->category);
	(void)bcm_decode_byte(pkt, &gasDecode->action);
	(void)bcm_decode_byte(pkt, &gasDecode->dialogToken);

	if (gasDecode->category != DOT11_ACTION_CAT_PUBLIC &&
		gasDecode->category != DOT11_ACTION_CAT_PDPA) {
		WL_P2PO(("invalid category %d\n", gasDecode->category));
		return FALSE;
	}

	switch (gasDecode->action)
	{
	case GAS_REQUEST_ACTION_FRAME:
		if (!decodeAdvertisementProtocol(pkt, &gasDecode->request.apie)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!decodeReqRsp(pkt, &gasDecode->request.reqLen, &gasDecode->request.req)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		break;
	case GAS_RESPONSE_ACTION_FRAME:
		if (!bcm_decode_le16(pkt, &gasDecode->response.statusCode)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!bcm_decode_le16(pkt, &gasDecode->response.comebackDelay)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!decodeAdvertisementProtocol(pkt, &gasDecode->response.apie)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!decodeReqRsp(pkt, &gasDecode->response.rspLen, &gasDecode->response.rsp)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		break;
	case GAS_COMEBACK_REQUEST_ACTION_FRAME:
		/* nothing */
		break;
	case GAS_COMEBACK_RESPONSE_ACTION_FRAME:
		if (!bcm_decode_le16(pkt, &gasDecode->comebackResponse.statusCode)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!bcm_decode_byte(pkt, &gasDecode->comebackResponse.fragmentId)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!bcm_decode_le16(pkt, &gasDecode->comebackResponse.comebackDelay)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!decodeAdvertisementProtocol(pkt, &gasDecode->comebackResponse.apie)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (!decodeReqRsp(pkt, &gasDecode->comebackResponse.rspLen,
			&gasDecode->comebackResponse.rsp)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		break;
	default:
		WL_P2PO(("invalid action %d\n", gasDecode->action));
		return FALSE;
		break;
	}

	printGasFrame(gasDecode);
	return TRUE;
}
