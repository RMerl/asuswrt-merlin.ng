/*
 * dcs.c
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
 * $Id: dcs.c 736533 2017-12-15 13:52:50Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmtimer.h>
#include <bcmendian.h>
#include <shutils.h>
#include <bcmendian.h>
#include <wlutils.h>

#include "acsd_svr.h"

int
dcs_parse_actframe(dot11_action_wifi_vendor_specific_t *actfrm, wl_bcmdcs_data_t *dcs_data)
{
	uint8 cat;
	uint32 reason;
	chanspec_t chanspec;

	if (!actfrm)
		return ACSD_ERR_NO_FRM;

	cat = actfrm->category;
	ACSD_INFO("recved action frames, category: %d\n", actfrm->category);
	if (cat != DOT11_ACTION_CAT_VS)
		return ACSD_ERR_NOT_ACTVS;

	if ((actfrm->OUI[0] != BCM_ACTION_OUI_BYTE0) ||
		(actfrm->OUI[1] != BCM_ACTION_OUI_BYTE1) ||
		(actfrm->OUI[2] != BCM_ACTION_OUI_BYTE2) ||
		(actfrm->type != BCM_ACTION_RFAWARE)) {
		ACSD_INFO("recved VS action frame, but not DCS request\n");
		return ACSD_ERR_NOT_DCSREQ;
	}

	bcopy(&actfrm->data[0], &reason, sizeof(uint32));
	dcs_data->reason = ltoh32(reason);
	bcopy(&actfrm->data[4], (uint8*)&chanspec, sizeof(chanspec_t));
	dcs_data->chspec = ltoh16(chanspec);

	ACSD_INFO("dcs_data: reason: %d, chanspec: 0x%4x\n", dcs_data->reason,
		(uint16)dcs_data->chspec);

	return ACSD_OK;
}
