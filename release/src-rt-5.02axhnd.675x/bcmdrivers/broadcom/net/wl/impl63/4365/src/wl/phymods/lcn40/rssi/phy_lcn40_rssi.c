/*
 * LCN40PHY RSSI Compute module implementation
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
#include <bcmendian.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rssi.h"
#include <phy_lcn40.h>
#include <phy_lcn40_rssi.h>

#include <phy_utils_reg.h>
#include <wlc_phyreg_lcn40.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn40.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_lcn40_rssi_info {
	phy_info_t *pi;
	phy_lcn40_info_t *lcn40i;
	phy_rssi_info_t *ri;
};

/* local functions */
static void phy_lcn40_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);

/* register phy type specific functions */
phy_lcn40_rssi_info_t *
BCMATTACHFN(phy_lcn40_rssi_register_impl)(phy_info_t *pi, phy_lcn40_info_t *lcn40i,
	phy_rssi_info_t *ri)
{
	phy_lcn40_rssi_info_t *info;
	phy_type_rssi_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn40_rssi_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn40_rssi_info_t));
	info->pi = pi;
	info->lcn40i = lcn40i;
	info->ri = ri;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.compute = phy_lcn40_rssi_compute;
	fns.ctx = info;

	phy_rssi_register_impl(ri, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn40_rssi_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn40_rssi_unregister_impl)(phy_lcn40_rssi_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_rssi_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_rssi_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_lcn40_rssi_info_t));
}

/* calculate rssi */
#define LCN40_RXSTAT0_BRD_ATTN	12
#define LCN40_RXSTAT0_ACITBL_IDX_MSB	11
#define LCN40_RX_GAIN_INDEX_MASK	0x7F00
#define LCN40_RX_GAIN_INDEX_SHIFT	8
#define LCN40_QDB_MASK	0x3
#define LCN40_QDB_SHIFT	2
#define LCN40_BIT1_QDB_POS	10
#define LCN40_BIT0_QDB_POS	13

static void BCMFASTPATH
phy_lcn40_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh)
{
	phy_lcn40_rssi_info_t *info = (phy_lcn40_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_lcn40_info_t *lcn40i = info->lcn40i;
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	int rssi = rxh->PhyRxStatus_1 & PRXS1_JSSI_MASK;
	uint16 board_atten = (rxh->PhyRxStatus_0 >> LCN40_RXSTAT0_BRD_ATTN) & 0x1;
	uint8 gidx = (rxh->PhyRxStatus_2 & 0xFC00) >> 10;
	int8 *corr2g;
#ifdef BAND5G
	int8 *corr5g;
#endif // endif
	int8 *corrperrg;
	int8 po_reg;
	int16 po_nom = 0;
	int16 rxpath_gain;
	uint8 aci_tbl = 0;
	int8 rssi_qdb = 0;
	bool jssi_based_rssi = FALSE;
	uint8 tr_iso = 0;

	/* For bcm4334 do not use packets that were received when prisel_clr flag is set */
	if ((pi->sh->corerev == 35) && (rxh->RxStatus2 & RXS_PHYRXST_PRISEL_CLR)) {
		/* RXS_PHYRXST_PRISEL_CLR set, rssi_qdB = 0x0 => Use JSSI */
		/* RXS_PHYRXST_PRISEL_CLR set, rssi_qdB = 0x3 => WLC_RSSI_INVALID */
		if (lcn40i->rssi_iqest_en && lcn40i->rssi_iqest_jssi_en) {
			rssi_qdb = (rxh->PhyRxStatus_0
			            >> LCN40_BIT0_QDB_POS) & 0x1;
			rssi_qdb |= ((rxh->PhyRxStatus_0
			              & (1 << LCN40_BIT1_QDB_POS))
			             >> (LCN40_BIT1_QDB_POS - 1));
			if (!rssi_qdb) {
				jssi_based_rssi = TRUE;
			}
		}
		else {
			rssi = WLC_RSSI_INVALID;
			wrxh->do_rssi_ma = 1; /* skip calc rssi MA */
			goto end;
		}
	}

	wrxh->do_rssi_ma = 0;

	if (rssi > 127)
		rssi -= 256;

	aci_tbl = (rxh->PhyRxStatus_0 >> LCN40_RXSTAT0_ACITBL_IDX_MSB) & 0x1;
	if (aci_tbl)
		gidx = gidx + (1 << 6);

	if (lcn40i->rssi_iqest_en) {
		if (CHSPEC_IS2G(pi->radio_chanspec))
			tr_iso = lcn40i->lcnphycommon.lcnphy_tr_isolation_mid;
#ifdef BAND5G
		else
			tr_iso = lcn40i->lcnphycommon.triso5g[0];
#endif // endif
		if (board_atten) {
			gidx = gidx + tr_iso;
		}
	}

	if (lcn40i->rssi_iqest_en && (!jssi_based_rssi)) {
		int rssi_in_qdb;

		rxpath_gain = wlc_lcn40phy_get_rxpath_gain_by_index(pi, gidx, board_atten);
		rssi_qdb = (rxh->PhyRxStatus_0 >> LCN40_BIT0_QDB_POS) & 0x1;
		rssi_qdb |= ((rxh->PhyRxStatus_0 & (1 << LCN40_BIT1_QDB_POS))
		             >> (LCN40_BIT1_QDB_POS - 1));
		rssi_in_qdb = (rssi << LCN40_QDB_SHIFT) + rssi_qdb - rxpath_gain +
		        (lcn40i->rssi_iqest_gain_adj << LCN40_QDB_SHIFT);
		rssi = (rssi_in_qdb >> LCN40_QDB_SHIFT);
		rssi_qdb = rssi_in_qdb & LCN40_QDB_MASK;
		PHY_INFORM(("rssidB= %d, rssi_qdB= %d, rssi_in_qdB= %d"
		            "boardattn= %d, rxpath_gain= %d, "
		            "gidx = %d, gain_adj = %d\n",
		            rssi, rssi_qdb, rssi_in_qdb, board_atten,
		            rxpath_gain, gidx,
		            lcn40i->rssi_iqest_gain_adj));
	}

	if ((!lcn40i->rssi_iqest_en) || jssi_based_rssi) {
		/* JSSI adjustment wrt power offset */
		if (CHSPEC_IS20(pi->radio_chanspec))
			po_reg = PHY_REG_READ(pi, LCN40PHY, SignalBlockConfigTable6_new,
			                      crssignalblk_input_pwr_offset_db);
		else
			po_reg = PHY_REG_READ(pi, LCN40PHY, SignalBlockConfigTable5_new,
			                      crssignalblk_input_pwr_offset_db_40mhz);
		switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			if (CHSPEC_IS20(pi->radio_chanspec))
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_2g;
			else
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_40_2g;
			break;
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			if (CHSPEC_IS20(pi->radio_chanspec))
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_5g[0];
			else
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_40_5g[0];
			break;
		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			if (CHSPEC_IS20(pi->radio_chanspec))
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_5g[1];
			else
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_40_5g[1];
			break;
		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			if (CHSPEC_IS20(pi->radio_chanspec))
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_5g[2];
			else
				po_nom = lcn40i->lcnphycommon.noise.nvram_input_pwr_offset_40_5g[2];
			break;
#endif /* BAND5G */
		default:
			po_nom = po_reg;
			break;
		}

		rssi += (po_nom - po_reg);

		/* RSSI adjustment and Adding the JSSI range specific corrections */
#ifdef BAND5G
		if (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec) !=
		    WL_CHAN_FREQ_RANGE_2G) {
			if ((rssi < -60) && ((gidx > 0) && (gidx <= 37)))
				rssi += phy_lcn40_get_pkt_rssi_gain_index_offset_5g(gidx);
			corrperrg = pi->rssi_corr_perrg_5g;
		} else
#endif /* BAND5G */
		{
			if ((rssi < -60) && ((gidx > 0) && (gidx <= 37)))
				rssi += phy_lcn40_get_pkt_rssi_gain_index_offset_2g(gidx);
			corrperrg = pi->rssi_corr_perrg_2g;
		}

		if (rssi <= corrperrg[0])
			rssi += corrperrg[2];
		else if (rssi <= corrperrg[1])
			rssi += corrperrg[3];
		else
			rssi += corrperrg[4];

		corr2g = &(pi->rssi_corr_normal);
#ifdef BAND5G
		corr5g = &(pi->rssi_corr_normal_5g[0]);
#endif /* BAND5G */

		switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			rssi += *corr2g;
			break;
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			rssi += corr5g[0];
			break;
		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			rssi += corr5g[1];
			break;
		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			rssi += corr5g[2];
			break;
#endif /* BAND5G */
		default:
			rssi += 0;
			break;
		}
	}

	/* Temp sense based correction */
	rssi = (rssi << LCN40_QDB_SHIFT) + rssi_qdb;
	rssi += wlc_lcn40phy_rssi_tempcorr(pi, 0);
	if (lcn40i->rssi_iqest_en && !jssi_based_rssi)
		rssi += wlc_lcn40phy_iqest_rssi_tempcorr(pi, 0, board_atten);

	rssi_qdb = rssi & LCN40_QDB_MASK;
	rssi = (rssi >> LCN40_QDB_SHIFT);

	/* temperature compensation */
	rssi = rssi + lcn40i->lcnphycommon.lcnphy_pkteng_rssi_slope;

	/* Sign extend */
	if (rssi > 127)
		rssi -= 256;

	wrxh->rxpwr[0] = (int8)rssi;

	if (rssi > MAX_VALID_RSSI)
		rssi = MAX_VALID_RSSI;

end:
	wrxh->rssi = (int8)rssi;
	wrxh->rssi_qdb = rssi_qdb;

	PHY_TRACE(("%s: rssi %d\n", __FUNCTION__, (int8)rssi));
}

static const int8 lcn40phy_gain_index_offset_for_pkt_rssi_2g[] = {
	0,	/* 0 */
	0,	/* 1 */
	0,	/* 2 */
	0,	/* 3 */
	0,	/* 4 */
	0,	/* 5 */
	0,	/* 6 */
	0,	/* 7 */
	0,	/* 8 */
	0,	/* 9 */
	0,	/* 10 */
	0,	/* 11 */
	0,	/* 12 */
	0,	/* 13 */
	0,	/* 14 */
	0,	/* 15 */
	0,	/* 16 */
	0,	/* 17 */
	0,	/* 18 */
	0,	/* 19 */
	0,	/* 20 */
	0,	/* 21 */
	0,	/* 22 */
	0,	/* 23 */
	0,	/* 24 */
	1,	/* 25 */
	0,	/* 26 */
	1,	/* 27 */
	2,	/* 28 */
	2,	/* 29 */
	0,	/* 30 */
	0,	/* 31 */
	0,	/* 32 */
	0,	/* 33 */
	0,	/* 34 */
	0,	/* 35 */
	0,	/* 36 */
	0,	/* 37 */
};

/* don't ROM this variable */
int8 *phy_lcn40_pkt_rssi_gain_index_offset_2g =
        (int8 *)lcn40phy_gain_index_offset_for_pkt_rssi_2g;

#ifdef BAND5G
static const  int8 lcn40phy_gain_index_offset_for_pkt_rssi_5g[] = {
	0,	/* 0 */
	0,	/* 1 */
	0,	/* 2 */
	0,	/* 3 */
	0,	/* 4 */
	0,	/* 5 */
	0,	/* 6 */
	0,	/* 7 */
	0,	/* 8 */
	0,	/* 9 */
	0,	/* 10 */
	0,	/* 11 */
	0,	/* 12 */
	0,	/* 13 */
	0,	/* 14 */
	0,	/* 15 */
	0,	/* 16 */
	0,	/* 17 */
	0,	/* 18 */
	0,	/* 19 */
	0,	/* 20 */
	0,	/* 21 */
	0,	/* 22 */
	0,	/* 23 */
	0,	/* 24 */
	0,	/* 25 */
	2,	/* 26 */
	2,	/* 27 */
	2,	/* 28 */
	2,	/* 29 */
	0,	/* 30 */
	0,	/* 31 */
	0,	/* 32 */
	0,	/* 33 */
	0,	/* 34 */
	0,	/* 35 */
	0,	/* 36 */
	0	/* 37 */
};

/* don't ROM this variable */
int8 *phy_lcn40_pkt_rssi_gain_index_offset_5g =
        (int8 *)lcn40phy_gain_index_offset_for_pkt_rssi_5g;
#endif /* BAND5G */
