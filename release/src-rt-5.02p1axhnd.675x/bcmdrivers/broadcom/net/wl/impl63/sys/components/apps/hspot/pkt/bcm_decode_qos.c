/*
 * Decoding of QoS packets.
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
#include "bcm_decode_qos.h"

/* decode QoS map */
int bcm_decode_qos_map(bcm_decode_t *pkt, bcm_decode_qos_map_t *qos)
{
	uint8 byte;
	uint8 len, except_len;
	int i;

	WL_PRPKT("packet for QoS map decoding",
		bcm_decode_buf(pkt), bcm_decode_buf_length(pkt));

	memset(qos, 0, sizeof(*qos));

	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_ACTION_CAT_QOS) {
		WL_ERROR(("QoS action category\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_QOS_ACTION_QOS_MAP) {
		WL_ERROR(("QoS map\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &byte) || byte != DOT11_MNG_QOS_MAP_ID) {
		WL_ERROR(("QoS ID\n"));
		return FALSE;
	}
	if (!bcm_decode_byte(pkt, &len) || len < 16 || ((len % 2) == 1)) {
		WL_ERROR(("length\n"));
		return FALSE;
	}
	if (len > bcm_decode_remaining(pkt)) {
		WL_ERROR(("length exceeds packet %d > %d\n",
			len, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	except_len = len - 16;
	for (i = 0; i < except_len / 2; i++) {
		if (!bcm_decode_byte(pkt, &qos->except[i].dscp) ||
			!bcm_decode_byte(pkt, &qos->except[i].up)) {
			WL_ERROR(("DSCP exception\n"));
			return FALSE;
		}
	}
	qos->exceptCount = i;
	for (i = 0; i < 16 / 2; i++) {
		if (!bcm_decode_byte(pkt, &qos->up[i].low) ||
			!bcm_decode_byte(pkt, &qos->up[i].high)) {
			WL_ERROR(("DSCP range\n"));
			return FALSE;
		}
	}

	return TRUE;
}
