/*
 * NPHY Core module implementation
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
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_rstr.h>

#include <phy_n_ana.h>
#include <phy_n_radio.h>
#include <phy_n_tbl.h>
#include <phy_n_tpc.h>
#include <phy_n_radar.h>
#include <phy_n_calmgr.h>
#include <phy_n_noise.h>
#include <phy_n_antdiv.h>
#include <phy_n_temp.h>
#include <phy_n_rssi.h>
#include <phy_n_papdcal.h>
#include <phy_n_vcocal.h>

#include "phy_type.h"
#include "phy_type_n.h"
#include "phy_type_n_iovt.h"
#include "phy_type_n_ioct.h"
#include "phy_shared.h"
#include <phy_n.h>

#include <phy_utils_radio.h>
#include <phy_utils_var.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_n.h>
#include <wlc_phyreg_n.h>
/* TODO: all these are going away... > */
#endif // endif

#define PHY_TXPWR_MIN_NPHY	8	/* for nphy devices */

/* local functions */
static int phy_n_attach_ext(phy_info_t *pi, int bandtype);
static int phy_n_register_impl(phy_info_t *pi, phy_type_info_t *ti, int bandtype);
static void phy_n_unregister_impl(phy_info_t *pi, phy_type_info_t *ti);
static void phy_n_reset_impl(phy_info_t *pi, phy_type_info_t *ti);
static int phy_n_init_impl(phy_info_t *pi, phy_type_info_t *ti);
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)
static int phy_n_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b);
#else
#define	phy_n_dump_phyregs	NULL
#endif // endif

/* attach/detach */
phy_type_info_t *
BCMATTACHFN(phy_n_attach)(phy_info_t *pi, int bandtype)
{
	phy_n_info_t *ni;
	phy_type_fns_t fns;
	uint32 idcode;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* Extend phy_attach() here to initialize NPHY specific stuff */
	if (phy_n_attach_ext(pi, bandtype) != BCME_OK) {
		PHY_ERROR(("%s: phy_n_attach_ext failed\n", __FUNCTION__));
		return NULL;
	}

	/* read idcode */
	idcode = phy_n_radio_query_idcode(pi);
	PHY_TRACE(("%s: idcode 0x%08x\n", __FUNCTION__, idcode));
	/* parse idcode */
	phy_utils_parse_idcode(pi, idcode);
	/* override radiover */
	if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 4) ||
	    /* 4324B0, 4324B2, 43242/43243 */
	    CHIPID_4324X_MEDIA_FAMILY(pi))
		pi->pubpi.radiover = RADIOVER(pi->pubpi.radiover);
	/* validate radio id */
	if (phy_utils_valid_radio(pi) != BCME_OK) {
		PHY_ERROR(("%s: phy_utils_valid_radio failed\n", __FUNCTION__));
		return NULL;
	}

	/* TODO: move the acphy attach code to here... */
	if (wlc_phy_attach_nphy(pi) == FALSE) {
		PHY_ERROR(("%s: wlc_phy_attach_nphy failed\n", __FUNCTION__));
		return NULL;
	}
	ni = pi->u.pi_nphy;
	ni->pi = pi;

	/* register PHY type implementation entry points */
	bzero(&fns, sizeof(fns));
	fns.reg_impl = phy_n_register_impl;
	fns.unreg_impl = phy_n_unregister_impl;
	fns.reset_impl = phy_n_reset_impl;
	fns.reg_iovt = phy_n_register_iovt;
	fns.reg_ioct = phy_n_register_ioct;
	fns.init_impl = phy_n_init_impl;
	fns.dump_phyregs = phy_n_dump_phyregs;
	fns.ti = (phy_type_info_t *)ni;

	phy_register_impl(pi, &fns);

	return (phy_type_info_t *)ni;
}

void
BCMATTACHFN(phy_n_detach)(phy_type_info_t *ti)
{
	phy_n_info_t *ni = (phy_n_info_t *)ti;
	phy_info_t *pi = ni->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_mfree(pi, ni, sizeof(phy_n_info_t));
}

static int
BCMATTACHFN(phy_n_attach_ext)(phy_info_t *pi, int bandtype)
{
	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	pi->pubpi.phy_corenum = PHY_CORE_NUM_2;

	pi->aci_exit_check_period = 15;

#ifdef N2WOWL
	/* Reduce phyrxchain to 1 to save power in WOWL mode */
	if (CHIPID(pi->sh->chip) == BCM43237_CHIP_ID) {
		pi->sh->phyrxchain = 0x1;
	}
#endif /* N2WOWL */

	/* minimum reliable txpwr target is 8 dBm/mimo, 9dBm/lcn40, 10 dbm/legacy  */
	pi->min_txpower =
	        (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_mintxpower, PHY_TXPWR_MIN_NPHY);

	pi->phynoise_polling = FALSE;

	return BCME_OK;
}

/* initialize s/w and/or h/w shared by different modules */
static int
WLBANDINITFN(phy_n_init_impl)(phy_info_t *pi, phy_type_info_t *ti)
{
	/* Reset gain_boost on band-change */
	pi->nphy_gain_boost = TRUE;

	return BCME_OK;
}

/* Register/unregister NPHY specific implementations to their commons.
 * Used to configure features/modules implemented for NPHY.
 */
static int
BCMATTACHFN(phy_n_register_impl)(phy_info_t *pi, phy_type_info_t *ti, int bandtype)
{
	phy_n_info_t *ni = (phy_n_info_t *)ti;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* Register with ANAcore control module */
	if (pi->anai != NULL &&
	    (ni->anai = phy_n_ana_register_impl(pi, ni, pi->anai)) == NULL) {
		PHY_ERROR(("%s: phy_n_ana_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with RADIO control module */
	if (pi->radioi != NULL &&
	    (ni->radioi = phy_n_radio_register_impl(pi, ni, pi->radioi)) == NULL) {
		PHY_ERROR(("%s: phy_n_radio_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with PHYTableInit module */
	if (pi->tbli != NULL &&
	    (ni->tbli = phy_n_tbl_register_impl(pi, ni, pi->tbli)) == NULL) {
		PHY_ERROR(("%s: phy_n_tbl_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with TxPowerCtrl module */
	if (pi->tpci != NULL &&
	    (ni->tpci = phy_n_tpc_register_impl(pi, ni, pi->tpci)) == NULL) {
		PHY_ERROR(("%s: phy_n_tpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

#if defined(AP) && defined(RADAR)
	/* Register with RadarDetect module */
	if (pi->radari != NULL &&
	    (ni->radari = phy_n_radar_register_impl(pi, ni, pi->radari)) == NULL) {
		PHY_ERROR(("%s: phy_n_radar_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register with MPhaseCAL module */
	if (pi->calmgri != NULL &&
	    (ni->calmgri = phy_n_calmgr_register_impl(pi, ni, pi->calmgri)) == NULL) {
		PHY_ERROR(("%s: phy_n_calmgr_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with ANTennaDIVersity module */
	if (pi->antdivi != NULL &&
	    (ni->antdivi = phy_n_antdiv_register_impl(pi, ni, pi->antdivi)) == NULL) {
		PHY_ERROR(("%s: phy_n_antdiv_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

#ifndef WLC_DISABLE_ACI
	/* Register with INTerFerence module */
	if (pi->noisei != NULL &&
	    (ni->noisei = phy_n_noise_register_impl(pi, ni, pi->noisei)) == NULL) {
		PHY_ERROR(("%s: phy_n_noise_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register with TEMPerature sense module */
	if (pi->tempi != NULL &&
	    (ni->tempi = phy_n_temp_register_impl(pi, ni, pi->tempi)) == NULL) {
		PHY_ERROR(("%s: phy_n_temp_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with RSSICompute module */
	if (pi->rssii != NULL &&
	    (ni->rssii = phy_n_rssi_register_impl(pi, ni, pi->rssii)) == NULL) {
		PHY_ERROR(("%s: phy_n_rssi_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with TxIQLOCal module */
	if (pi->txiqlocali != NULL &&
		(ni->txiqlocali =
		phy_n_txiqlocal_register_impl(pi, ni, pi->txiqlocali)) == NULL) {
		PHY_ERROR(("%s: phy_n_txiqlocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with RxIQCal module */
	if (pi->rxiqcali != NULL &&
		(ni->rxiqcali = phy_n_rxiqcal_register_impl(pi, ni, pi->rxiqcali)) == NULL) {
		PHY_ERROR(("%s: phy_n_rxiqcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with PAPDCal module */
	if (pi->papdcali != NULL &&
		(ni->papdcali = phy_n_papdcal_register_impl(pi, ni, pi->papdcali)) == NULL) {
		PHY_ERROR(("%s: phy_n_papdcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with VCOCal module */
	if (pi->vcocali != NULL &&
		(ni->vcocali = phy_n_vcocal_register_impl(pi, ni, pi->vcocali)) == NULL) {
		PHY_ERROR(("%s: phy_n_vcocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* ...Add your module registration here... */

	return BCME_OK;
fail:
	return BCME_ERROR;
}

static void
BCMATTACHFN(phy_n_unregister_impl)(phy_info_t *pi, phy_type_info_t *ti)
{
	phy_n_info_t *ni = (phy_n_info_t *)ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Unregister from VCO Cal module */
	if (ni->vcocali != NULL)
		phy_n_vcocal_unregister_impl(ni->vcocali);

	/* Unregister from PAPD Cal module */
	if (ni->papdcali != NULL)
		phy_n_papdcal_unregister_impl(ni->papdcali);

	/* Unregister from RXIQ Cal module */
	if (ni->rxiqcali != NULL)
		phy_n_rxiqcal_unregister_impl(ni->rxiqcali);

	/* Unregister from TXIQLO Cal module */
	if (ni->txiqlocali != NULL)
		phy_n_txiqlocal_unregister_impl(ni->txiqlocali);

	/* Unregister from RSSICompute module */
	if (ni->rssii != NULL)
		phy_n_rssi_unregister_impl(ni->rssii);

	/* Unregister from TEMPerature sense module */
	if (ni->tempi != NULL)
		phy_n_temp_unregister_impl(ni->tempi);

#ifndef WLC_DISABLE_ACI
	/* Unregister from INTerFerence module */
	if (ni->noisei != NULL)
		phy_n_noise_unregister_impl(ni->noisei);
#endif // endif

	/* Unregister from ANTennaDIVersity module */
	if (ni->antdivi != NULL)
		phy_n_antdiv_unregister_impl(ni->antdivi);

	/* Unregister from MPhaseCAL module */
	if (ni->calmgri != NULL)
		phy_n_calmgr_unregister_impl(ni->calmgri);

#if defined(AP) && defined(RADAR)
	/* Unregister from RadarDetect module */
	if (ni->radari != NULL)
		phy_n_radar_unregister_impl(ni->radari);
#endif // endif

	/* Unregister from TxPowerCtrl module */
	if (ni->tpci != NULL)
		phy_n_tpc_unregister_impl(ni->tpci);

	/* Unregister from PHYTableInit module */
	if (ni->tbli != NULL)
		phy_n_tbl_unregister_impl(ni->tbli);

	/* Unregister from RADIO control module */
	if (ni->radioi != NULL)
		phy_n_radio_unregister_impl(ni->radioi);

	/* Unregister from ANAcore control module */
	if (ni->anai != NULL)
		phy_n_ana_unregister_impl(ni->anai);

	/* ...Add your module registration here... */
}

/* reset implementation (s/w) */
static void
phy_n_reset_impl(phy_info_t *pi, phy_type_info_t *ti)
{
	phy_n_info_t *ni = (phy_n_info_t *)ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!(NREV_GE(pi->pubpi.phy_rev, 3)))
		pi->phy_spuravoid = SPURAVOID_DISABLE;

	ni->nphy_papd_skip = 0;
	ni->nphy_papd_epsilon_offset[0] = 0xf588;
	ni->nphy_papd_epsilon_offset[1] = 0xf588;
	ni->nphy_txpwr_idx_2G[0] = 128;
	ni->nphy_txpwr_idx_2G[1] = 128;
	ni->nphy_txpwr_idx_5G[0] = 128;
	ni->nphy_txpwr_idx_5G[1] = 128;
	ni->nphy_txpwrindex[0].index_internal = 40;
	ni->nphy_txpwrindex[1].index_internal = 40;
	ni->nphy_pabias = 0; /* default means no override */
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(DBG_PHY_IOV)
static phy_regs_t bphy8_regs[] = {
	{ 0,    9 },            /* 0 - 8 */
	{ 0xa,	5 },		/* 0xa - 0xe */
	{ 0x10,	0x2a },		/* 0x10 - 0x39 */
	{ 0x3d,	0x19 },		/* 0x3d - 0x55 */
	{ 0x58,	0xa },		/* 0x58 - 0x61 */
	{ 0x63,	0x12 },		/* 0x63 - 0x74 */
	{ 0,	0 }
};

static phy_regs_t nphy3_bphy_regs[] = {
	{ 1,	1 },		/* 1 */
	{ 4,	5 },		/* 4 - 8 */
	{ 0xa,	1 },		/* 0xa */
	{ 0xe,	1 },		/* 0xe */
	{ 0x10,	4 },		/* 0x10 - 0x13 */
	{ 0x18,	2 },		/* 0x18 - 0x19 */
	{ 0x20,	22 },		/* 0x20 - 0x35 */
	{ 0x38,	2 },		/* 0x38 - 0x39 */
	{ 0x40,	14 },		/* 0x40 - 0x4d */
	{ 0x4f,	7 },		/* 0x4f - 0x55 */
	{ 0x5b,	5 },		/* 0x5b - 0x5f */
	{ 0x63,	1 },		/* 0x63 */
	{ 0x67,	12 },		/* 0x67 - 0x72 */
	{ 0x75,	1 },		/* 0x75 */
	{ 0,	0 }
};

static phy_regs_t nphy2_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 -  */
	{ 0x005,	0x001 },	/* 0x005 -  */
	{ 0x007,	0x001 },	/* 0x007 -  */
	{ 0x009,	0x001 },	/* 0x009 -  */
	{ 0x00b,	0x002 },	/* 0x00b -  0x00c */
	{ 0x00e,	0x002 },	/* 0x00e -  */
	{ 0x018,	0x045 },	/* 0x018 -  */
	{ 0x060,	0x019 },	/* 0x060 -  */
	{ 0x07a,	0x00c },	/* 0x07a -  */
	{ 0x08b,	0x002 },	/* 0x08b -  */
	{ 0x090,	0x00e },	/* 0x090 -  */
	{ 0x0a0,	0x02c },	/* 0x0a0 -  */
	{ 0x0d5,	0x002 },	/* 0x0d5 -  */
	{ 0x0d9,	0x005 },	/* 0x0d9 -  */
	{ 0x0ea,	0x026 },	/* 0x0ea -  */
	{ 0x111,	0x002 },	/* 0x111 -  */
	{ 0x114,	0x002 },	/* 0x114 -  */
	{ 0x118,	0x002 },	/* 0x118 -  */
	{ 0x11c,	0x004 },	/* 0x11c -  */
	{ 0x122,	0x010 },	/* 0x122 -  */
	{ 0x134,	0x009 },	/* 0x134 -  */
	{ 0x13f,	0x001 },	/* 0x13f -  */
	{ 0x141,	0x00e },	/* 0x141 -  */
	{ 0x150,	0x001 },	/* 0x150 -  */
	{ 0x152,	0x002 },	/* 0x152 -  */
	{ 0x156,	0x009 },	/* 0x156 -  */
	{ 0x160,	0x00c },	/* 0x160 -  */
	{ 0x16f,	0x04f },	/* 0x16f -  */
	{ 0x1c4,	0x04b },	/* 0x1c4 -  */
	{ 0x210,	0x00b },	/* 0x210 -  */
	{ 0x21d,	0x022 },	/* 0x21d -  */
	{ 0,	0 }
};
static phy_regs_t nphy3_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 -  */
	{ 0x004,	0x002 },	/* 0x004 -  */
	{ 0x007,	0x003 },	/* 0x007 -  */
	{ 0x00b,	0x002 },	/* 0x00b -  0x00c */
	{ 0x00e,	0x002 },	/* 0x00e -  */
	{ 0x018,	0x008 },	/* 0x018 -  */
	{ 0x025,	0x011 },	/* 0x025 -  */
	{ 0x03b,	0x022 },	/* 0x03b -  */
	{ 0x060,	0x018 },	/* 0x060 -  */
	{ 0x08b,	0x002 },	/* 0x08b -  */
	{ 0x090,	0x001 },	/* 0x090 -  */
	{ 0x095,	0x009 },	/* 0x095 -  */
	{ 0x0a0,	0x001 },	/* 0x0a0 -  */
	{ 0x0ae,	0x01e },	/* 0x0ae -  */
	{ 0x0d5,	0x002 },	/* 0x0d5 -  */
	{ 0x0d9,	0x005 },	/* 0x0d9 -  */
	{ 0x0e0,	0x005 },	/* 0x0e0 -  */
	{ 0x0ea,	0x002 },	/* 0x0ea -  */
	{ 0x0ed,	0x001 },	/* 0x0ed -  */
	{ 0x0ee,	0x00a },	/* 0x0ee -  */
	{ 0x104,	0x009 },	/* 0x104 -  */
	{ 0x111,	0x001 },	/* 0x111 -  */
	{ 0x114,	0x002 },	/* 0x114 -  */
	{ 0x118,	0x002 },	/* 0x118 -  */
	{ 0x11c,	0x002 },	/* 0x11c -  */
	{ 0x122,	0x010 },	/* 0x122 -  */
	{ 0x134,	0x009 },	/* 0x134 -  */
	{ 0x13f,	0x001 },	/* 0x13f -  */
	{ 0x144,	0x00a },	/* 0x144 -  */
	{ 0x150,	0x001 },	/* 0x150 -  */
	{ 0x152,	0x003 },	/* 0x152 -  */
	{ 0x156,	0x009 },	/* 0x156 -  */
	{ 0x160,	0x00c },	/* 0x160 -  */
	{ 0x16f,	0x04f },	/* 0x16f -  */
	{ 0x1c4,	0x03c },	/* 0x1c4 -  */
	{ 0x204,	0x004 },	/* 0x204 -  */
	{ 0x20a,	0x005 },	/* 0x20a -  */
	{ 0x210,	0x00b },	/* 0x210 -  */
	{ 0x21d,	0x019 },	/* 0x21d -  */
	{ 0x23e,	0x005 },	/* 0x23e -  */
	{ 0x245,	0x008 },	/* 0x245 -  */
	{ 0x250,	0x086 },	/* 0x250 -  */
	{ 0x20,		0x005 },	/* 0x20 -  */
	{ 0x36,		0x005 },	/* 0x36 -  */
	{ 0x78,		0x001 },	/* 0x78 -  */
	{ 0x7a,		0x001 },	/* 0x7a -  */
	{ 0x7b,		0x00b },	/* 0x7b -  */
	{ 0x8f,		0x001 },	/* 0x8f -  */
	{ 0x91,		0x004 },	/* 0x91 -  */
	{ 0xa1,		0x00d },	/* 0xa1 -  */
	{ 0xcc,		0x004 },	/* 0xcc -  */
	{ 0xe5,		0x005 },	/* 0xe5 -  */
	{ 0xec,		0x001 },	/* 0xec -  */
	{ 0xf8,		0x00c },	/* 0xf8 -  */
	{ 0x10d,	0x003 },	/* 0x10d -  */
	{ 0x112,	0x001 },	/* 0x112 -  */
	{ 0x14e,	0x001 },	/* 0x14e -  */
	{ 0x200,	0x004 },	/* 0x200 -  */
	{ 0x208,	0x002 },	/* 0x208 -  */
	{ 0x236,	0x008 },	/* 0x236 -  */
	{ 0x243,	0x002 },	/* 0x243 -  */
	{ 0,	0 }
};

static phy_regs_t nphy5_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 -  */
	{ 0x004,	0x002 },	/* 0x004 -  */
	{ 0x007,	0x003 },	/* 0x007 -  */
	{ 0x00b,	0x002 },	/* 0x00b -  0x00c */
	{ 0x00e,	0x002 },	/* 0x00e -  */
	{ 0x018,	0x008 },	/* 0x018 -  */
	{ 0x025,	0x011 },	/* 0x025 -  */
	{ 0x03b,	0x022 },	/* 0x03b -  */
	{ 0x060,	0x018 },	/* 0x060 -  */
	{ 0x08b,	0x002 },	/* 0x08b -  */
	{ 0x090,	0x001 },	/* 0x090 -  */
	{ 0x095,	0x009 },	/* 0x095 -  */
	{ 0x0a0,	0x001 },	/* 0x0a0 -  */
	{ 0x0ae,	0x01e },	/* 0x0ae -  */
	{ 0x0d5,	0x002 },	/* 0x0d5 -  */
	{ 0x0d9,	0x005 },	/* 0x0d9 -  */
	{ 0x0e0,	0x005 },	/* 0x0e0 -  */
	{ 0x0ea,	0x002 },	/* 0x0ea -  */
	{ 0x0ed,	0x001 },	/* 0x0ed -  */
	{ 0x0ee,	0x00a },	/* 0x0ee -  */
	{ 0x104,	0x009 },	/* 0x104 -  */
	{ 0x111,	0x001 },	/* 0x111 -  */
	{ 0x114,	0x002 },	/* 0x114 -  */
	{ 0x118,	0x002 },	/* 0x118 -  */
	{ 0x11c,	0x002 },	/* 0x11c -  */
	{ 0x122,	0x010 },	/* 0x122 -  */
	{ 0x134,	0x009 },	/* 0x134 -  */
	{ 0x13f,	0x001 },	/* 0x13f -  */
	{ 0x144,	0x00a },	/* 0x144 -  */
	{ 0x150,	0x001 },	/* 0x150 -  */
	{ 0x152,	0x003 },	/* 0x152 -  */
	{ 0x156,	0x009 },	/* 0x156 -  */
	{ 0x160,	0x00c },	/* 0x160 -  */
	{ 0x16f,	0x04f },	/* 0x16f -  */
	{ 0x1c4,	0x03c },	/* 0x1c4 -  */
	{ 0x204,	0x004 },	/* 0x204 -  */
	{ 0x20a,	0x005 },	/* 0x20a -  */
	{ 0x210,	0x00b },	/* 0x210 -  */
	{ 0x21d,	0x019 },	/* 0x21d -  */
	{ 0x23e,	0x005 },	/* 0x23e -  */
	{ 0x245,	0x008 },	/* 0x245 -  */
	{ 0x250,	0x086 },	/* 0x250 -  */
	{ 0x20,		0x005 },	/* 0x20 -  */
	{ 0x36,		0x005 },	/* 0x36 -  */
	{ 0x78,		0x001 },	/* 0x78 -  */
	{ 0x7a,		0x001 },	/* 0x7a -  */
	{ 0x7b,		0x005 },	/* 0x7b -  */
	{ 0x8f,		0x001 },	/* 0x8f -  */
	{ 0x91,		0x002 },	/* 0x91 -  */
	{ 0xa1,		0x007 },	/* 0xa1 -  */
	{ 0xaa,		0x002 },	/* 0xaa -  */
	{ 0xcc,		0x002 },	/* 0xcc -  */
	{ 0xe5,		0x005 },	/* 0xe5 -  */
	{ 0xec,		0x001 },	/* 0xec -  */
	{ 0xf8,		0x004 },	/* 0xf8 -  */
	{ 0x100,	0x002 },	/* 0x100 -  */
	{ 0x10d,	0x003 },	/* 0x10d -  */
	{ 0x112,	0x001 },	/* 0x112 -  */
	{ 0x200,	0x004 },	/* 0x200 -  */
	{ 0x208,	0x002 },	/* 0x208 -  */
	{ 0x236,	0x008 },	/* 0x236 -  */
	{ 0x243,	0x002 },	/* 0x243 -  */
	{ 0,	0 }
};

static phy_regs_t nphy7_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 -  */
	{ 0x004,	0x002 },	/* 0x004 -  */
	{ 0x007,	0x003 },	/* 0x007 -  */
	{ 0x00c,	0x001 },	/* 0x00c -  */
	{ 0x00e,	0x002 },	/* 0x00e -  */
	{ 0x018,	0x008 },	/* 0x018 -  */
	{ 0x025,	0x011 },	/* 0x025 -  */
	{ 0x03b,	0x022 },	/* 0x03b -  */
	{ 0x060,	0x018 },	/* 0x060 -  */
	{ 0x08b,	0x002 },	/* 0x08b -  */
	{ 0x090,	0x001 },	/* 0x090 -  */
	{ 0x095,	0x009 },	/* 0x095 -  */
	{ 0x0a0,	0x001 },	/* 0x0a0 -  */
	{ 0x0ae,	0x01e },	/* 0x0ae -  */
	{ 0x0d5,	0x002 },	/* 0x0d5 -  */
	{ 0x0d9,	0x005 },	/* 0x0d9 -  */
	{ 0x0e0,	0x005 },	/* 0x0e0 -  */
	{ 0x0ea,	0x002 },	/* 0x0ea -  */
	{ 0x0ed,	0x001 },	/* 0x0ed -  */
	{ 0x0ee,	0x00a },	/* 0x0ee -  */
	{ 0x104,	0x009 },	/* 0x104 -  */
	{ 0x111,	0x001 },	/* 0x111 -  */
	{ 0x114,	0x002 },	/* 0x114 -  */
	{ 0x118,	0x002 },	/* 0x118 -  */
	{ 0x11c,	0x002 },	/* 0x11c -  */
	{ 0x122,	0x010 },	/* 0x122 -  */
	{ 0x134,	0x009 },	/* 0x134 -  */
	{ 0x13f,	0x001 },	/* 0x13f -  */
	{ 0x144,	0x00a },	/* 0x144 -  */
	{ 0x152,	0x003 },	/* 0x152 -  */
	{ 0x156,	0x009 },	/* 0x156 -  */
	{ 0x160,	0x00c },	/* 0x160 -  */
	{ 0x16f,	0x04f },	/* 0x16f -  */
	{ 0x1c4,	0x03c },	/* 0x1c4 -  */
	{ 0x204,	0x004 },	/* 0x204 -  */
	{ 0x20a,	0x005 },	/* 0x20a -  */
	{ 0x210,	0x00b },	/* 0x210 -  */
	{ 0x21d,	0x019 },	/* 0x21d -  */
	{ 0x23e,	0x005 },	/* 0x23e -  */
	{ 0x246,	0x007 },	/* 0x245 -  */
	{ 0x250,	0x057 },	/* 0x250 -  */
	{ 0x2b1,	0x025 },
	{ 0x2d6,	0x001 },
	{ 0x2d8,	0x00f },
	{ 0x2e8,	0x003 },
	{ 0x2f0,	0x007 },
	{ 0x2f8,	0x007 },
	{ 0x300,	0x004 },
	{ 0x30b,	0x003 },
	{ 0x310,	0x004 },
	{ 0x318,	0x004 },
	{ 0x320,	0x007 },
	{ 0x32f,	0x001 },
	{ 0x330,	0x004 },
	{ 0x335,	0x001 },
	{ 0x338,	0x008 },
	{ 0x34a,	0x001 },
	{ 0x350,	0x007 },
	{ 0x00b,	0x001 },
	{ 0x20,		0x005 },	/* 0x20 -  */
	{ 0x36,		0x005 },	/* 0x36 -  */
	{ 0x78,		0x002 },	/* 0x78 -  */
	{ 0x7a,		0x001 },	/* 0x7a -  */
	{ 0x7b,		0x005 },	/* 0x7b -  */
	{ 0x8f,		0x001 },	/* 0x8f -  */
	{ 0x91,		0x002 },	/* 0x91 -  */
	{ 0xa1,		0x007 },	/* 0xa1 -  */
	{ 0xaa,		0x002 },	/* 0xaa -  */
	{ 0xcc,		0x002 },	/* 0xcc -  */
	{ 0xe5,		0x005 },	/* 0xe5 -  */
	{ 0xec,		0x001 },	/* 0xec -  */
	{ 0xf8,		0x004 },	/* 0xf8 -  */
	{ 0x100,	0x002 },	/* 0x100 -  */
	{ 0x112,	0x001 },	/* 0x112 -  */
	{ 0x150,	0x001 },
	{ 0x200,	0x004 },	/* 0x200 -  */
	{ 0x208,	0x002 },	/* 0x208 -  */
	{ 0x236,	0x008 },	/* 0x236 -  */
	{ 0x243,	0x003 },	/* 0x243 -  */
	{ 0x2a7,	0x00a },
	{ 0x2ff,	0x001 },
	{ 0x304,	0x007 },
	{ 0x340,	0x00a },
	{ 0x357,	0x001 },
	{ 0,	0 }
};

/* 6362B0 (rev8) crashes when accessing a non-existing table address, so skip PHY regs 0x73-74 */
static phy_regs_t nphy8_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 -  */
	{ 0x004,	0x002 },	/* 0x004 -  */
	{ 0x007,	0x003 },	/* 0x007 -  */
	{ 0x00c,	0x001 },	/* 0x00c -  */
	{ 0x00e,	0x002 },	/* 0x00e -  */
	{ 0x018,	0x008 },	/* 0x018 -  */
	{ 0x025,	0x001 },	/* 0x025 -  */
	{ 0x029,	0x00d },	/* 0x029 -  */
	{ 0x03b,	0x001 },	/* 0x03b -  */
	{ 0x03f,	0x01e },	/* 0x03f -  */
	{ 0x060,	0x013 },	/* 0x060 -  */
	{ 0x075,	0x001 },	/* 0x075 -  */
	{ 0x077,	0x001 },	/* 0x077 -  */
	{ 0x08b,	0x002 },	/* 0x08b -  */
	{ 0x090,	0x001 },	/* 0x090 -  */
	{ 0x095,	0x001 },	/* 0x095 -  */
	{ 0x097,	0x001 },	/* 0x097 -  */
	{ 0x09a,	0x004 },	/* 0x09a -  */
	{ 0x0a0,	0x001 },	/* 0x0a0 -  */
	{ 0x0ae,	0x00c },	/* 0x0ae -  */
	{ 0x0bc,	0x010 },	/* 0x0bc -  */
	{ 0x0d5,	0x002 },	/* 0x0d5 -  */
	{ 0x0d9,	0x005 },	/* 0x0d9 -  */
	{ 0x0e0,	0x005 },	/* 0x0e0 -  */
	{ 0x0ea,	0x002 },	/* 0x0ea -  */
	{ 0x0ed,	0x001 },	/* 0x0ed -  */
	{ 0x0ee,	0x00a },	/* 0x0ee -  */
	{ 0x109,	0x004 },	/* 0x109 -  */
	{ 0x111,	0x001 },	/* 0x111 -  */
	{ 0x114,	0x002 },	/* 0x114 -  */
	{ 0x118,	0x002 },	/* 0x118 -  */
	{ 0x11c,	0x002 },	/* 0x11c -  */
	{ 0x122,	0x010 },	/* 0x122 -  */
	{ 0x134,	0x009 },	/* 0x134 -  */
	{ 0x13f,	0x001 },	/* 0x13f -  */
	{ 0x144,	0x00a },	/* 0x144 -  */
	{ 0x152,	0x003 },	/* 0x152 -  */
	{ 0x156,	0x009 },	/* 0x156 -  */
	{ 0x160,	0x00c },	/* 0x160 -  */
	{ 0x16f,	0x004 },	/* 0x16f -  */
	{ 0x175,	0x008 },	/* 0x175 -  */
	{ 0x17e,	0x040 },	/* 0x17e -  */
	{ 0x1c4,	0x03c },	/* 0x1c4 -  */
	{ 0x204,	0x004 },	/* 0x204 -  */
	{ 0x20a,	0x005 },	/* 0x20a -  */
	{ 0x210,	0x006 },	/* 0x210 -  */
	{ 0x217,	0x001 },	/* 0x217 -  */
	{ 0x219,	0x002 },	/* 0x219 -  */
	{ 0x21d,	0x019 },	/* 0x21d -  */
	{ 0x23e,	0x005 },	/* 0x23e -  */
	{ 0x246,	0x007 },	/* 0x245 -  */
	{ 0x250,	0x057 },	/* 0x250 -  */
	{ 0x2b1,	0x025 },
	{ 0x2d6,	0x001 },
	{ 0x2d8,	0x00f },
	{ 0x2e8,	0x004 },
	{ 0x2f0,	0x007 },
	{ 0x2f8,	0x007 },
	{ 0x300,	0x004 },
	{ 0x30b,	0x003 },
	{ 0x310,	0x004 },
	{ 0x318,	0x004 },
	{ 0x320,	0x007 },
	{ 0x32f,	0x001 },
	{ 0x330,	0x004 },
	{ 0x335,	0x001 },
	{ 0x338,	0x008 },
	{ 0x34a,	0x001 },
	{ 0x350,	0x007 },
	{ 0x358,	0x002 },        /* 0x358 -  */
	{ 0x360,	0x003 },        /* 0x360 -  */
	{ 0x00b,	0x001 },
	{ 0x20,		0x005 },	/* 0x20 -  */
	{ 0x36,		0x005 },	/* 0x36 -  */
	{ 0x78,		0x002 },	/* 0x78 -  */
	{ 0x7a,		0x001 },	/* 0x7a -  */
	{ 0x7b,		0x005 },	/* 0x7b -  */
	{ 0x8f,		0x001 },	/* 0x8f -  */
	{ 0x91,		0x002 },	/* 0x91 -  */
	{ 0xa1,		0x007 },	/* 0xa1 -  */
	{ 0xaa,		0x002 },	/* 0xaa -  */
	{ 0xcc,		0x002 },	/* 0xcc -  */
	{ 0xe5,		0x005 },	/* 0xe5 -  */
	{ 0xec,		0x001 },	/* 0xec -  */
	{ 0xf8,		0x004 },	/* 0xf8 -  */
	{ 0x100,	0x002 },	/* 0x100 -  */
	{ 0x112,	0x001 },	/* 0x112 -  */
	{ 0x150,	0x001 },
	{ 0x200,	0x004 },	/* 0x200 -  */
	{ 0x208,	0x002 },	/* 0x208 -  */
	{ 0x236,	0x008 },	/* 0x236 -  */
	{ 0x243,	0x003 },	/* 0x243 -  */
	{ 0x2a7,	0x00a },
	{ 0x2ff,	0x001 },
	{ 0x304,	0x007 },
	{ 0x340,	0x00a },
	{ 0x357,	0x001 },
	{ 0,	0 }
};

/* 53572/43217 (rev17) */
static phy_regs_t nphy17_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 -  */
	{ 0x004,	0x002 },	/* 0x004 -  */
	{ 0x007,	0x003 },	/* 0x007 -  */
	{ 0x00c,	0x001 },	/* 0x00c -  */
	{ 0x00e,	0x002 },	/* 0x00e -  */
	{ 0x018,	0x008 },	/* 0x018 -  */
	{ 0x025,	0x001 },	/* 0x025 -  */
	{ 0x029,	0x00d },	/* 0x029 -  */
	{ 0x03b,	0x001 },	/* 0x03b -  */
	{ 0x03f,	0x00a },	/* 0x03f -  0x048 */
	{ 0x053,	0x00a },	/* 0x053 -  0x05c */
	{ 0x060,	0x016 },	/* 0x060 -  0x075 */
	{ 0x077,	0x001 },	/* 0x077 -  */
	{ 0x08b,	0x002 },	/* 0x08b -  */
	{ 0x090,	0x001 },	/* 0x090 -  */
	{ 0x095,	0x002 },	/* 0x095 -  0x096 */
	{ 0x097,	0x001 },	/* 0x097 -  */
	{ 0x09a,	0x007 },	/* 0x09a -  0x0a0 */
	{ 0x0ae,	0x00a },	/* 0x0ae -  0x0b7 */
	{ 0x0bd,	0x00f },	/* 0x0bd -  0x0cb */
	{ 0x0d5,	0x002 },	/* 0x0d5 -  */
	{ 0x0dc,	0x002 },	/* 0x0dc -  0x0dd */
	{ 0x0e0,	0x005 },	/* 0x0e0 -  */
	{ 0x0ed,	0x001 },	/* 0x0ed -  */
	{ 0x0ee,	0x00a },	/* 0x0ee -  */
	{ 0x111,	0x001 },	/* 0x111 -  */
	{ 0x114,	0x002 },	/* 0x114 -  */
	{ 0x118,	0x002 },	/* 0x118 -  */
	{ 0x11c,	0x002 },	/* 0x11c -  */
	{ 0x122,	0x010 },	/* 0x122 -  */
	{ 0x134,	0x009 },	/* 0x134 -  */
	{ 0x13f,	0x001 },	/* 0x13f -  */
	{ 0x144,	0x00a },	/* 0x144 -  */
	{ 0x152,	0x003 },	/* 0x152 -  */
	{ 0x157,	0x008 },	/* 0x157 -  0x15e */
	{ 0x160,	0x00c },	/* 0x160 -  */
	{ 0x16f,	0x002 },	/* 0x16f -  0x0170 */
	{ 0x17a,	0x003 },	/* 0x17a -  0x17c */
	{ 0x17e,	0x001 },	/* 0x17e -  0x17e */
	{ 0x183,	0x001 },	/* 0x183 -  0x183 */
	{ 0x186,	0x038 },	/* 0x186 -  0x1bd */
	{ 0x1c4,	0x033 },	/* 0x1c4 -  0x1f6 */
	{ 0x1ff,	0x001 },	/* 0x1ff -  0x1ff */
	{ 0x204,	0x001 },	/* 0x204 -  0x204 */
	{ 0x20a,	0x005 },	/* 0x20a -  */
	{ 0x210,	0x006 },	/* 0x210 -  */
	{ 0x217,	0x001 },	/* 0x217 -  */
	{ 0x219,	0x002 },	/* 0x219 -  */
	{ 0x21d,	0x019 },	/* 0x21d -  */
	{ 0x23e,	0x005 },	/* 0x23e -  */
	{ 0x246,	0x007 },	/* 0x245 -  */
	{ 0x250,	0x00d },	/* 0x250 -  0x25c */
	{ 0x267,	0x026 },	/* 0x267 -  0x28c */
	{ 0x294,	0x013 },	/* 0x294 -  0x2a6 */
	{ 0x2b1,	0x025 },
	{ 0x2d6,	0x002 },    /* 0x2d6 -  0x2d7 */
	{ 0x2d8,	0x010 },    /* 0x2d8 -  0x2e7 */
	{ 0x2e8,	0x004 },
	{ 0x2f0,	0x007 },
	{ 0x2f8,	0x007 },
	{ 0x300,	0x004 },
	{ 0x30b,	0x003 },
	{ 0x310,	0x004 },
	{ 0x318,	0x004 },
	{ 0x320,	0x007 },
	{ 0x32f,	0x001 },
	{ 0x330,	0x004 },
	{ 0x335,	0x001 },
	{ 0x338,	0x008 },
	{ 0x34a,	0x001 },
	{ 0x350,	0x007 },
	{ 0x358,	0x003 },        /* 0x358 -  0x35a */
	{ 0x360,	0x050 },        /* 0x360 -  0x3af */
	{ 0x3b1,	0x001 },        /* 0x3b1 -  0x3b1 */
	{ 0xc73,	0x006 },        /* 0xc73 -  0xc78 */
	{ 0x00b,	0x001 },
	{ 0x20,		0x005 },	/* 0x20 -  */
	{ 0x36,		0x005 },	/* 0x36 -  */
	{ 0x78,		0x002 },	/* 0x78 -  */
	{ 0x7a,		0x001 },	/* 0x7a -  */
	{ 0x7b,		0x005 },	/* 0x7b -  */
	{ 0x8f,		0x001 },	/* 0x8f -  */
	{ 0x91,		0x002 },	/* 0x91 -  */
	{ 0xa1,		0x007 },	/* 0xa1 -  */
	{ 0xaa,		0x002 },	/* 0xaa -  */
	{ 0xcc,		0x002 },	/* 0xcc -  */
	{ 0xe5,		0x005 },	/* 0xe5 -  */
	{ 0xec,		0x001 },	/* 0xec -  */
	{ 0xf8,		0x004 },	/* 0xf8 -  */
	{ 0x100,	0x002 },	/* 0x100 -  */
	{ 0x112,	0x001 },	/* 0x112 -  */
	{ 0x150,	0x001 },
	{ 0x200,	0x004 },	/* 0x200 -  */
	{ 0x208,	0x002 },	/* 0x208 -  */
	{ 0x236,	0x008 },	/* 0x236 -  */
	{ 0x243,	0x003 },	/* 0x243 -  */
	{ 0x2a7,	0x00a },
	{ 0x2ff,	0x001 },
	{ 0x304,	0x007 },
	{ 0x340,	0x00a },
	{ 0x357,	0x001 },
	{ 0,	0 }
};

static int
phy_n_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b)
{
	phy_regs_t *rl;

	wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			rl = nphy3_bphy_regs;
		} else {
			rl = bphy8_regs;
		}
		phy_dump_phyregs(pi, "bphy", rl, NPHY_TO_BPHY_OFF, b);
	}

	if (NREV_IS(pi->pubpi.phy_rev, 17)) {
		rl = nphy17_regs;
	} else if (NREV_GE(pi->pubpi.phy_rev, 8)) {
		rl = nphy8_regs;
	} else if (NREV_IS(pi->pubpi.phy_rev, 7)) {
		rl = nphy7_regs;
	} else if (NREV_GE(pi->pubpi.phy_rev, 5)) {
		rl = nphy5_regs;
	} else if (NREV_GE(pi->pubpi.phy_rev, 3)) {
		rl = nphy3_regs;
	} else {
		rl = nphy2_regs;
	}
	phy_dump_phyregs(pi, "nphy", rl, 0, b);

	wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

	return BCME_OK;
}
#endif // endif
#endif /* BCMDBG || BCMDBG_DUMP */
