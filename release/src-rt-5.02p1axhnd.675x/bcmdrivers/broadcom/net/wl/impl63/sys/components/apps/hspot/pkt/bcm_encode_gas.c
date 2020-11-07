/*
 * Encode functions which provides encoding of GAS packets as defined in 802.11u.
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
#include "bcm_encode_gas.h"

/* encode GAS request */
int bcm_encode_gas_request(bcm_encode_t *pkt, uint8 dialogToken,
	uint8 apieLen, uint8 *apie, uint16 reqLen, uint8 *req)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_PUBLIC);
	bcm_encode_byte(pkt, GAS_REQUEST_ACTION_FRAME);
	bcm_encode_byte(pkt, dialogToken);
	if (apieLen > 0) {
		bcm_encode_bytes(pkt, apieLen, apie);
	}
	bcm_encode_le16(pkt, reqLen);
	if (reqLen > 0) {
		bcm_encode_bytes(pkt, reqLen, req);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode GAS response */
int bcm_encode_gas_response(bcm_encode_t *pkt, uint8 dialogToken,
	uint16 statusCode, uint16 comebackDelay, uint8 apieLen, uint8 *apie,
	uint16 rspLen, uint8 *rsp)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_PUBLIC);
	bcm_encode_byte(pkt, GAS_RESPONSE_ACTION_FRAME);
	bcm_encode_byte(pkt, dialogToken);
	bcm_encode_le16(pkt, statusCode);
	bcm_encode_le16(pkt, comebackDelay);
	if (apieLen > 0) {
		bcm_encode_bytes(pkt, apieLen, apie);
	}
	bcm_encode_le16(pkt, rspLen);
	if (rspLen > 0) {
		bcm_encode_bytes(pkt, rspLen, rsp);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode GAS comeback request */
int bcm_encode_gas_comeback_request(bcm_encode_t *pkt, uint8 dialogToken)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_PUBLIC);
	bcm_encode_byte(pkt, GAS_COMEBACK_REQUEST_ACTION_FRAME);
	bcm_encode_byte(pkt, dialogToken);

	return bcm_encode_length(pkt) - initLen;
}

/* encode GAS response */
int bcm_encode_gas_comeback_response(bcm_encode_t *pkt, uint8 dialogToken,
	uint16 statusCode, uint8 fragmentId, uint16 comebackDelay,
	uint8 apieLen, uint8 *apie, uint16 rspLen, uint8 *rsp)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_PUBLIC);
	bcm_encode_byte(pkt, GAS_COMEBACK_RESPONSE_ACTION_FRAME);
	bcm_encode_byte(pkt, dialogToken);
	bcm_encode_le16(pkt, statusCode);
	bcm_encode_byte(pkt, fragmentId);
	bcm_encode_le16(pkt, comebackDelay);
	if (apieLen > 0) {
		bcm_encode_bytes(pkt, apieLen, apie);
	}
	bcm_encode_le16(pkt, rspLen);
	if (rspLen > 0) {
		bcm_encode_bytes(pkt, rspLen, rsp);
	}

	return bcm_encode_length(pkt) - initLen;
}
