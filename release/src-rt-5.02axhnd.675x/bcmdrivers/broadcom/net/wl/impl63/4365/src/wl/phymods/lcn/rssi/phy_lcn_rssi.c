/*
 * LCNPHY RSSI Compute module implementation
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
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rssi.h"
#include <phy_lcn.h>
#include <phy_lcn_rssi.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_lcn_rssi_info {
	phy_info_t *pi;
	phy_lcn_info_t *lcni;
	phy_rssi_info_t *ri;
};

/* local functions */
static void phy_lcn_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);

/* register phy type specific functions */
phy_lcn_rssi_info_t *
BCMATTACHFN(phy_lcn_rssi_register_impl)(phy_info_t *pi, phy_lcn_info_t *lcni, phy_rssi_info_t *ri)
{
	phy_lcn_rssi_info_t *info;
	phy_type_rssi_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn_rssi_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn_rssi_info_t));
	info->pi = pi;
	info->lcni = lcni;
	info->ri = ri;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.compute = phy_lcn_rssi_compute;
	fns.ctx = info;

	phy_rssi_register_impl(ri, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn_rssi_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn_rssi_unregister_impl)(phy_lcn_rssi_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_rssi_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_rssi_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_lcn_rssi_info_t));
}

/* calculate rssi */
static void BCMFASTPATH
phy_lcn_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh)
{
	phy_lcn_rssi_info_t *info = (phy_lcn_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_lcn_info_t *lcni = info->lcni;
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	int rssi = rxh->PhyRxStatus_1 & PRXS1_JSSI_MASK;
	uint16 board_atten = (rxh->PhyRxStatus_0 >> 11) & 0x1;
	uint8 gidx = (rxh->PhyRxStatus_2 & 0xFC00) >> 10;
	int8 ant = (int8)((rxh->PhyRxStatus_0 >> 14) & 0x1);

	if (rssi > 127)
		rssi -= 256;

	/* RSSI adjustment */
	rssi = rssi + phy_lcn_get_pkt_rssi_gain_index_offset(gidx);
	if (board_atten)
		rssi = rssi + pi->rssi_corr_boardatten;
	else
		rssi = rssi + pi->rssi_corr_normal;

	/* temperature compensation */
	rssi = rssi + lcni->lcnphy_pkteng_rssi_slope;

	/* Sign extend */
	if (rssi > 127)
		rssi -= 256;

	wrxh->rxpwr[0] = (int8)rssi;

	wrxh->rssi = (int8)rssi;
	wrxh->rssi_qdb = 0;
	wrxh->do_rssi_ma = 0;

	PHY_TRACE(("%s: rssi %d\n", __FUNCTION__, (int8)rssi));

	wlc_phy_lcn_updatemac_rssi(pi, (int8)rssi, ant);
}

static const int8 lcnphy_gain_index_offset_for_pkt_rssi[] = {
	8,	/* 0 */
	8,	/* 1 */
	8,	/* 2 */
	8,	/* 3 */
	8,	/* 4 */
	8,	/* 5 */
	8,	/* 6 */
	9,	/* 7 */
	10,	/* 8 */
	8,	/* 9 */
	8,	/* 10 */
	7,	/* 11 */
	7,	/* 12 */
	1,	/* 13 */
	2,	/* 14 */
	2,	/* 15 */
	2,	/* 16 */
	2,	/* 17 */
	2,	/* 18 */
	2,	/* 19 */
	2,	/* 20 */
	2,	/* 21 */
	2,	/* 22 */
	2,	/* 23 */
	2,	/* 24 */
	2,	/* 25 */
	2,	/* 26 */
	2,	/* 27 */
	2,	/* 28 */
	2,	/* 29 */
	2,	/* 30 */
	2,	/* 31 */
	1,	/* 32 */
	1,	/* 33 */
	0,	/* 34 */
	0,	/* 35 */
	0,	/* 36 */
	0	/* 37 */
};

int8 *phy_lcn_pkt_rssi_gain_index_offset = (int8 *)lcnphy_gain_index_offset_for_pkt_rssi;
