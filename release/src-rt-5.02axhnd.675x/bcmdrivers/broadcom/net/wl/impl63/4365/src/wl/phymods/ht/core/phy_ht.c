/*
 * HTPHY Core module implementation
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

#include <phy_ht_ana.h>
#include <phy_ht_radio.h>
#include <phy_ht_tbl.h>
#include <phy_ht_tpc.h>
#include <phy_ht_radar.h>
#include <phy_ht_calmgr.h>
#include <phy_ht_noise.h>
#include <phy_ht_temp.h>
#include <phy_ht_rssi.h>
#include <phy_ht_txiqlocal.h>
#include <phy_ht_rxiqcal.h>
#include <phy_ht_papdcal.h>
#include <phy_ht_vcocal.h>

#include "phy_type.h"
#include "phy_type_ht.h"
#include "phy_type_ht_iovt.h"
#include "phy_type_ht_ioct.h"
#include "phy_shared.h"
#include <phy_ht.h>

#include <phy_utils_radio.h>
#include <phy_utils_var.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_ht.h>
#include "wlc_phyreg_ht.h"
/* TODO: all these are going away... > */
#endif // endif

#define PHY_TXPWR_MIN_HTPHY	8	/* for htphy devices */

/* local functions */
static int phy_ht_attach_ext(phy_info_t *pi, int bandtype);
static int phy_ht_register_impl(phy_info_t *pi, phy_type_info_t *ti, int bandtype);
static void phy_ht_unregister_impl(phy_info_t *pi, phy_type_info_t *ti);
static void phy_ht_reset_impl(phy_info_t *pi, phy_type_info_t *ti);
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)
static int phy_ht_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b);
#else
#define	phy_ht_dump_phyregs	NULL
#endif // endif

/* attach/detach */
phy_type_info_t *
BCMATTACHFN(phy_ht_attach)(phy_info_t *pi, int bandtype)
{
	phy_ht_info_t *hti;
	phy_type_fns_t fns;
	uint32 idcode;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* FIXME: must do this prior to calling wlc_phy_attach_htphy()
	 * in which 'phy_corenum' is used...
	 */
	if (phy_ht_attach_ext(pi, bandtype) != BCME_OK) {
		PHY_ERROR(("%s: phy_ht_attach_ext failed\n", __FUNCTION__));
		return NULL;
	}

	/* read idcode */
	idcode = phy_ht_radio_query_idcode(pi);
	PHY_TRACE(("%s: idcode 0x%08x\n", __FUNCTION__, idcode));
	/* parse idcode */
	phy_utils_parse_idcode(pi, idcode);
	/* validate radio id */
	if (phy_utils_valid_radio(pi) != BCME_OK) {
		PHY_ERROR(("%s: phy_utils_valid_radio failed\n", __FUNCTION__));
		return NULL;
	}

	/* TODO: move the htphy attach code to here... */
	if (wlc_phy_attach_htphy(pi) == FALSE) {
		PHY_ERROR(("%s: wlc_phy_attach_htphy failed\n", __FUNCTION__));
		return NULL;
	}
	hti = pi->u.pi_htphy;
	hti->pi = pi;

	/* register PHY type implementation entry points */
	bzero(&fns, sizeof(fns));
	fns.reg_impl = phy_ht_register_impl;
	fns.unreg_impl = phy_ht_unregister_impl;
	fns.reset_impl = phy_ht_reset_impl;
	fns.reg_iovt = phy_ht_register_iovt;
	fns.reg_ioct = phy_ht_register_ioct;
	fns.dump_phyregs = phy_ht_dump_phyregs;
	fns.ti = (phy_type_info_t *)hti;

	phy_register_impl(pi, &fns);

	return (phy_type_info_t *)hti;
}

void
BCMATTACHFN(phy_ht_detach)(phy_type_info_t *ti)
{
	phy_ht_info_t *hti = (phy_ht_info_t *)ti;
	phy_info_t *pi = hti->pi;

	phy_mfree(pi, hti, sizeof(phy_ht_info_t));
}

/* extension to phy_init */
static int
BCMATTACHFN(phy_ht_attach_ext)(phy_info_t *pi, int bandtype)
{
	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	pi->pubpi.phy_corenum = PHY_CORE_NUM_3;

	pi->aci_exit_check_period = 15;

	/* minimum reliable txpwr target is 8 dBm/mimo, 9dBm/lcn40, 10 dbm/legacy  */
	pi->min_txpower = PHY_TXPWR_MIN_HTPHY;

	pi->phynoise_polling = FALSE;

	return BCME_OK;
}

/* Register/unregister HTPHY specific implementations to their commons.
 * Used to configure features/modules implemented for HTPHY.
 */
int
BCMATTACHFN(phy_ht_register_impl)(phy_info_t *pi, phy_type_info_t *ti, int bandtype)
{
	phy_ht_info_t *hti = (phy_ht_info_t *)ti;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* Register with ANAcore control module */
	if (pi->anai != NULL &&
	    (hti->anai = phy_ht_ana_register_impl(pi, hti, pi->anai)) == NULL) {
		PHY_ERROR(("%s: phy_ht_ana_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with RADIO control module */
	if (pi->radioi != NULL &&
	    (hti->radioi = phy_ht_radio_register_impl(pi, hti, pi->radioi)) == NULL) {
		PHY_ERROR(("%s: phy_ht_radio_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with PHYTableInit module */
	if (pi->tbli != NULL &&
	    (hti->tbli = phy_ht_tbl_register_impl(pi, hti, pi->tbli)) == NULL) {
		PHY_ERROR(("%s: phy_ht_tbl_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with TxPowerCtrl module */
	if (pi->tpci != NULL &&
	    (hti->tpci = phy_ht_tpc_register_impl(pi, hti, pi->tpci)) == NULL) {
		PHY_ERROR(("%s: phy_ht_tpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

#if defined(AP) && defined(RADAR)
	/* Register with RadarDetect module */
	if (pi->radari != NULL &&
	    (hti->radari = phy_ht_radar_register_impl(pi, hti, pi->radari)) == NULL) {
		PHY_ERROR(("%s: phy_ht_radar_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register with MPhaseCAL module */
	if (pi->calmgri != NULL &&
	    (hti->calmgri = phy_ht_calmgr_register_impl(pi, hti, pi->calmgri)) == NULL) {
		PHY_ERROR(("%s: phy_ht_calmgr_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

#ifndef WLC_DISABLE_ACI
	/* Register with INTerFerence module */
	if (pi->noisei != NULL &&
	    (hti->noisei = phy_ht_noise_register_impl(pi, hti, pi->noisei)) == NULL) {
		PHY_ERROR(("%s: phy_ht_noise_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register with TEMPerature sense module */
	if (pi->tempi != NULL &&
	    (hti->tempi = phy_ht_temp_register_impl(pi, hti, pi->tempi)) == NULL) {
		PHY_ERROR(("%s: phy_ht_temp_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with RSSICompute module */
	if (pi->rssii != NULL &&
	    (hti->rssii = phy_ht_rssi_register_impl(pi, hti, pi->rssii)) == NULL) {
		PHY_ERROR(("%s: phy_ht_rssi_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with TxIQLOCal module */
	if (pi->txiqlocali != NULL &&
		(hti->txiqlocali =
		phy_ht_txiqlocal_register_impl(pi, hti, pi->txiqlocali)) == NULL) {
		PHY_ERROR(("%s: phy_ht_txiqlocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with RxIQCal module */
	if (pi->rxiqcali != NULL &&
		(hti->rxiqcali = phy_ht_rxiqcal_register_impl(pi, hti, pi->rxiqcali)) == NULL) {
		PHY_ERROR(("%s: phy_ht_rxiqcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with PAPDCal module */
	if (pi->papdcali != NULL &&
		(hti->papdcali = phy_ht_papdcal_register_impl(pi, hti, pi->papdcali)) == NULL) {
		PHY_ERROR(("%s: phy_ht_papdcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with VCOCal module */
	if (pi->vcocali != NULL &&
		(hti->vcocali = phy_ht_vcocal_register_impl(pi, hti, pi->vcocali)) == NULL) {
		PHY_ERROR(("%s: phy_ht_vcocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* ...Add your module registration here... */

	return BCME_OK;

fail:
	return BCME_ERROR;
}

void
BCMATTACHFN(phy_ht_unregister_impl)(phy_info_t *pi, phy_type_info_t *ti)
{
	phy_ht_info_t *hti = (phy_ht_info_t *)ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Unregister from VCO Cal module */
	if (hti->vcocali != NULL)
		phy_ht_vcocal_unregister_impl(hti->vcocali);

	/* Unregister from PAPD Cal module */
	if (hti->papdcali != NULL)
		phy_ht_papdcal_unregister_impl(hti->papdcali);

	/* Unregister from RXIQ Cal module */
	if (hti->rxiqcali != NULL)
		phy_ht_rxiqcal_unregister_impl(hti->rxiqcali);

	/* Unregister from TXIQLO Cal module */
	if (hti->txiqlocali != NULL)
		phy_ht_txiqlocal_unregister_impl(hti->txiqlocali);

	/* Unregister from RSSICompute module */
	if (hti->rssii != NULL)
		phy_ht_rssi_unregister_impl(hti->rssii);

	/* Unregister from TEMPerature sense module */
	if (hti->tempi != NULL)
		phy_ht_temp_unregister_impl(hti->tempi);

#ifndef WLC_DISABLE_ACI
	/* Unregister from INTerFerence module */
	if (hti->noisei != NULL)
		phy_ht_noise_unregister_impl(hti->noisei);
#endif // endif

	/* Unregister from MPhaseCAL module */
	if (hti->calmgri != NULL)
		phy_ht_calmgr_unregister_impl(hti->calmgri);

#if defined(AP) && defined(RADAR)
	/* Unregister from RadarDetect module */
	if (hti->radari != NULL)
		phy_ht_radar_unregister_impl(hti->radari);
#endif // endif

	/* Unregister from TxPowerCtrl module */
	if (hti->tpci != NULL)
		phy_ht_tpc_unregister_impl(hti->tpci);

	/* Unregister from PHYTableInit module */
	if (hti->tbli != NULL)
		phy_ht_tbl_unregister_impl(hti->tbli);

	/* Unregister from RADIO control module */
	if (hti->radioi != NULL)
		phy_ht_radio_unregister_impl(hti->radioi);

	/* Unregister from ANAcore control module */
	if (hti->anai != NULL)
		phy_ht_ana_unregister_impl(hti->anai);

	/* ...Add your module registration here... */
}

/* reset implementation (s/w) */
void
phy_ht_reset_impl(phy_info_t *pi, phy_type_info_t *ti)
{
	phy_ht_info_t *hti = (phy_ht_info_t *)ti;
	uint i;

	PHY_TRACE(("%s\n", __FUNCTION__));

	for (i = 0; i < PHY_CORE_MAX; i++)
		hti->txpwrindex[i] = 40;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(DBG_PHY_IOV)
static phy_regs_t htphy0_bphy_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 - 0x001 */
	{ 0x004,	0x005 },	/* 0x004 - 0x008 */
	{ 0x00a,	0x001 },	/* 0x00a - 0x00a */
	{ 0x010,	0x004 },	/* 0x010 - 0x013 */
	{ 0x018,	0x002 },	/* 0x018 - 0x019 */
	{ 0x020,	0x016 },	/* 0x020 - 0x035 */
	{ 0x038,	0x002 },	/* 0x038 - 0x039 */
	{ 0x03d,	0x001 },	/* 0x03d - 0x03d */
	{ 0x040,	0x00e },	/* 0x040 - 0x04d */
	{ 0x04f,	0x007 },	/* 0x04f - 0x055 */
	{ 0x05b,	0x005 },	/* 0x05b - 0x05f */
	{ 0x063,	0x001 },	/* 0x063 - 0x063 */
	{ 0x067,	0x00c },	/* 0x067 - 0x072 */
	{ 0,	0 }
};

static phy_regs_t htphy0_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 - 0x001 */
	{ 0x004,	0x002 },	/* 0x004 - 0x005 */
	{ 0x007,	0x003 },	/* 0x007 - 0x009 */
	{ 0x00b,	0x002 },	/* 0x00b - 0x00c */
	{ 0x020,	0x004 },	/* 0x020 - 0x023 */
	{ 0x044,	0x019 },	/* 0x044 - 0x05c */
	{ 0x060,	0x016 },	/* 0x060 - 0x075 */
	{ 0x077,	0x001 },	/* 0x077 - 0x077 */
	{ 0x08b,	0x003 },	/* 0x08b - 0x08d */
	{ 0x090,	0x001 },	/* 0x090 - 0x090 */
	{ 0x095,	0x001 },	/* 0x095 - 0x095 */
	{ 0x097,	0x001 },	/* 0x097 - 0x097 */
	{ 0x09a,	0x008 },	/* 0x09a - 0x0a1 */
	{ 0x0ae,	0x00f },	/* 0x0ae - 0x0bc */
	{ 0x0be,	0x00e },	/* 0x0be - 0x0cb */
	{ 0x0d6,	0x001 },	/* 0x0d6 - 0x0d6 */
	{ 0x0d9,	0x005 },	/* 0x0d9 - 0x0dd */
	{ 0x0e0,	0x007 },	/* 0x0e0 - 0x0e6 */
	{ 0x0ed,	0x00b },	/* 0x0ed - 0x0f7 */
	{ 0x109,	0x004 },	/* 0x109 - 0x10c */
	{ 0x111,	0x001 },	/* 0x111 - 0x111 */
	{ 0x114,	0x003 },	/* 0x114 - 0x116 */
	{ 0x118,	0x003 },	/* 0x118 - 0x11a */
	{ 0x11c,	0x002 },	/* 0x11c - 0x11d */
	{ 0x122,	0x010 },	/* 0x122 - 0x131 */
	{ 0x134,	0x00c },	/* 0x134 - 0x13f */
	{ 0x144,	0x001 },	/* 0x144 - 0x144 */
	{ 0x14d,	0x003 },	/* 0x14d - 0x14f */
	{ 0x154,	0x001 },	/* 0x154 - 0x154 */
	{ 0x157,	0x00a },	/* 0x157 - 0x160 */
	{ 0x171,	0x002 },	/* 0x171 - 0x172 */
	{ 0x175,	0x007 },	/* 0x175 - 0x17b */
	{ 0x17e,	0x040 },	/* 0x17e - 0x1bd */
	{ 0x1c8,	0x004 },	/* 0x1c8 - 0x1cb */
	{ 0x1cd,	0x033 },	/* 0x1cd - 0x1ff */
	{ 0x204,	0x004 },	/* 0x204 - 0x207 */
	{ 0x20a,	0x001 },	/* 0x20a - 0x20a */
	{ 0x20c,	0x002 },	/* 0x20c - 0x20d */
	{ 0x214,	0x002 },	/* 0x214 - 0x215 */
	{ 0x217,	0x001 },	/* 0x217 - 0x217 */
	{ 0x219,	0x003 },	/* 0x219 - 0x21b */
	{ 0x21d,	0x019 },	/* 0x21d - 0x235 */
	{ 0x23e,	0x005 },	/* 0x23e - 0x242 */
	{ 0x245,	0x003 },	/* 0x245 - 0x247 */
	{ 0x24c,	0x001 },	/* 0x24c - 0x24c */
	{ 0x259,	0x030 },	/* 0x259 - 0x288 */
	{ 0x28c,	0x022 },	/* 0x28c - 0x2ad */
	{ 0x2b1,	0x045 },	/* 0x2b1 - 0x2f5 */
	{ 0x308,	0x006 },	/* 0x308 - 0x30d */
	{ 0x310,	0x004 },	/* 0x310 - 0x313 */
	{ 0x318,	0x004 },	/* 0x318 - 0x31b */
	{ 0x320,	0x007 },	/* 0x320 - 0x326 */
	{ 0x32f,	0x001 },	/* 0x32f - 0x32f */
	{ 0x34a,	0x001 },	/* 0x34a - 0x34a */
	{ 0x358,	0x002 },	/* 0x358 - 0x359 */
	{ 0x400,	0x009 },	/* 0x400 - 0x408 */
	{ 0x40b,	0x012 },	/* 0x40b - 0x41c */
	{ 0x420,	0x009 },	/* 0x420 - 0x428 */
	{ 0x440,	0x009 },	/* 0x440 - 0x448 */
	{ 0x44b,	0x012 },	/* 0x44b - 0x45c */
	{ 0x460,	0x009 },	/* 0x460 - 0x468 */
	{ 0x480,	0x009 },	/* 0x480 - 0x488 */
	{ 0x48b,	0x012 },	/* 0x48b - 0x49c */
	{ 0x4a0,	0x009 },	/* 0x4a0 - 0x4a8 */
	{ 0x500,	0x003 },	/* 0x500 - 0x502 */
	{ 0x510,	0x009 },	/* 0x510 - 0x518 */
	{ 0x520,	0x003 },	/* 0x520 - 0x522 */
	{ 0x530,	0x006 },	/* 0x530 - 0x535 */
	{ 0x540,	0x00f },	/* 0x540 - 0x54e */
	{ 0x550,	0x002 },	/* 0x550 - 0x551 */
	{ 0x800,	0x002 },	/* 0x800 - 0x801 */
	{ 0x803,	0x004 },	/* 0x803 - 0x806 */
	{ 0x808,	0x002 },	/* 0x808 - 0x809 */
	{ 0x810,	0x003 },	/* 0x810 - 0x812 */
	{ 0x820,	0x003 },	/* 0x820 - 0x822 */
	{ 0x824,	0x003 },	/* 0x824 - 0x826 */
	{ 0x828,	0x001 },	/* 0x828 - 0x828 */
	{ 0x830,	0x00c },	/* 0x830 - 0x83b */
	{ 0x840,	0x00d },	/* 0x840 - 0x84c */
	{ 0x860,	0x00d },	/* 0x860 - 0x86c */
	{ 0x880,	0x00d },	/* 0x880 - 0x88c */
	{ 0x900,	0x006 },	/* 0x900 - 0x905 */
	{ 0x908,	0x001 },	/* 0x908 - 0x908 */
	{ 0x910,	0x003 },	/* 0x910 - 0x912 */
	{ 0x914,	0x003 },	/* 0x914 - 0x916 */
	{ 0x918,	0x003 },	/* 0x918 - 0x91a */
	{ 0x94f,	0x004 },	/* 0x94f - 0x952 */
	{ 0x955,	0x007 },	/* 0x955 - 0x95b */
	{ 0x960,	0x003 },	/* 0x960 - 0x962 */
	{ 0x964,	0x008 },	/* 0x964 - 0x96b */
	{ 0x970,	0x003 },	/* 0x970 - 0x972 */
	{ 0x980,	0x00f },	/* 0x980 - 0x98e */
	{ 0,	0 }
};

static phy_regs_t htphy1_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 - 0x001 */
	{ 0x004,	0x002 },	/* 0x004 - 0x005 */
	{ 0x007,	0x003 },	/* 0x007 - 0x009 */
	{ 0x00b,	0x002 },	/* 0x00b - 0x00c */
	{ 0x020,	0x004 },	/* 0x020 - 0x023 */
	{ 0x044,	0x005 },	/* 0x044 - 0x048 */
	{ 0x053,	0x00a },	/* 0x053 - 0x05c */
	{ 0x060,	0x016 },	/* 0x060 - 0x075 */
	{ 0x077,	0x001 },	/* 0x077 - 0x077 */
	{ 0x08b,	0x003 },	/* 0x08b - 0x08d */
	{ 0x090,	0x001 },	/* 0x090 - 0x090 */
	{ 0x095,	0x001 },	/* 0x095 - 0x095 */
	{ 0x097,	0x001 },	/* 0x097 - 0x097 */
	{ 0x09a,	0x008 },	/* 0x09a - 0x0a1 */
	{ 0x0ae,	0x004 },	/* 0x0ae - 0x0b1 */
	{ 0x0b4,	0x002 },	/* 0x0b4 - 0x0b5 */
	{ 0x0b9,	0x004 },	/* 0x0b9 - 0x0bc */
	{ 0x0be,	0x00e },	/* 0x0be - 0x0cb */
	{ 0x0d6,	0x001 },	/* 0x0d6 - 0x0d6 */
	{ 0x0d9,	0x002 },	/* 0x0d9 - 0x0da */
	{ 0x0dc,	0x002 },	/* 0x0dc - 0x0dd */
	{ 0x0e0,	0x007 },	/* 0x0e0 - 0x0e6 */
	{ 0x0ed,	0x00a },	/* 0x0ed - 0x0f6 */
	{ 0x109,	0x004 },	/* 0x109 - 0x10c */
	{ 0x111,	0x001 },	/* 0x111 - 0x111 */
	{ 0x114,	0x003 },	/* 0x114 - 0x116 */
	{ 0x118,	0x003 },	/* 0x118 - 0x11a */
	{ 0x11c,	0x002 },	/* 0x11c - 0x11d */
	{ 0x122,	0x010 },	/* 0x122 - 0x131 */
	{ 0x134,	0x00c },	/* 0x134 - 0x13f */
	{ 0x144,	0x001 },	/* 0x144 - 0x144 */
	{ 0x14d,	0x003 },	/* 0x14d - 0x14f */
	{ 0x154,	0x001 },	/* 0x154 - 0x154 */
	{ 0x157,	0x00a },	/* 0x157 - 0x160 */
	{ 0x171,	0x002 },	/* 0x171 - 0x172 */
	{ 0x175,	0x007 },	/* 0x175 - 0x17b */
	{ 0x17e,	0x001 },	/* 0x17e - 0x17e */
	{ 0x183,	0x001 },	/* 0x183 - 0x183 */
	{ 0x186,	0x038 },	/* 0x186 - 0x1bd */
	{ 0x1c8,	0x003 },	/* 0x1c8 - 0x1ca */
	{ 0x1cd,	0x011 },	/* 0x1cd - 0x1dd */
	{ 0x1df,	0x001 },	/* 0x1df - 0x1df */
	{ 0x1e1,	0x001 },	/* 0x1e1 - 0x1e1 */
	{ 0x1e5,	0x012 },	/* 0x1e5 - 0x1f6 */
	{ 0x1f9,	0x001 },	/* 0x1f9 - 0x1f9 */
	{ 0x1fb,	0x005 },	/* 0x1fb - 0x1ff */
	{ 0x204,	0x001 },	/* 0x204 - 0x204 */
	{ 0x20a,	0x001 },	/* 0x20a - 0x20a */
	{ 0x20c,	0x002 },	/* 0x20c - 0x20d */
	{ 0x214,	0x002 },	/* 0x214 - 0x215 */
	{ 0x217,	0x001 },	/* 0x217 - 0x217 */
	{ 0x219,	0x003 },	/* 0x219 - 0x21b */
	{ 0x21d,	0x007 },	/* 0x21d - 0x223 */
	{ 0x228,	0x00e },	/* 0x228 - 0x235 */
	{ 0x23e,	0x005 },	/* 0x23e - 0x242 */
	{ 0x245,	0x003 },	/* 0x245 - 0x247 */
	{ 0x24c,	0x001 },	/* 0x24c - 0x24c */
	{ 0x259,	0x004 },	/* 0x259 - 0x25c */
	{ 0x267,	0x01f },	/* 0x267 - 0x285 */
	{ 0x293,	0x001 },	/* 0x293 - 0x293 */
	{ 0x295,	0x019 },	/* 0x295 - 0x2ad */
	{ 0x2b1,	0x03b },	/* 0x2b1 - 0x2eb */
	{ 0x2ed,	0x009 },	/* 0x2ed - 0x2f5 */
	{ 0x308,	0x006 },	/* 0x308 - 0x30d */
	{ 0x310,	0x004 },	/* 0x310 - 0x313 */
	{ 0x318,	0x004 },	/* 0x318 - 0x31b */
	{ 0x320,	0x007 },	/* 0x320 - 0x326 */
	{ 0x32f,	0x001 },	/* 0x32f - 0x32f */
	{ 0x34a,	0x001 },	/* 0x34a - 0x34a */
	{ 0x358,	0x002 },	/* 0x358 - 0x359 */
	{ 0x400,	0x008 },	/* 0x400 - 0x407 */
	{ 0x40b,	0x012 },	/* 0x40b - 0x41c */
	{ 0x420,	0x009 },	/* 0x420 - 0x428 */
	{ 0x440,	0x008 },	/* 0x440 - 0x447 */
	{ 0x44b,	0x012 },	/* 0x44b - 0x45c */
	{ 0x460,	0x009 },	/* 0x460 - 0x468 */
	{ 0x480,	0x008 },	/* 0x480 - 0x487 */
	{ 0x48b,	0x012 },	/* 0x48b - 0x49c */
	{ 0x4a0,	0x009 },	/* 0x4a0 - 0x4a8 */
	{ 0x500,	0x003 },	/* 0x500 - 0x502 */
	{ 0x510,	0x009 },	/* 0x510 - 0x518 */
	{ 0x520,	0x003 },	/* 0x520 - 0x522 */
	{ 0x530,	0x006 },	/* 0x530 - 0x535 */
	{ 0x540,	0x00f },	/* 0x540 - 0x54e */
	{ 0x550,	0x002 },	/* 0x550 - 0x551 */
	{ 0x800,	0x002 },	/* 0x800 - 0x801 */
	{ 0x803,	0x004 },	/* 0x803 - 0x806 */
	{ 0x808,	0x002 },	/* 0x808 - 0x809 */
	{ 0x810,	0x003 },	/* 0x810 - 0x812 */
	{ 0x820,	0x003 },	/* 0x820 - 0x822 */
	{ 0x824,	0x003 },	/* 0x824 - 0x826 */
	{ 0x828,	0x001 },	/* 0x828 - 0x828 */
	{ 0x830,	0x00c },	/* 0x830 - 0x83b */
	{ 0x840,	0x00d },	/* 0x840 - 0x84c */
	{ 0x860,	0x00d },	/* 0x860 - 0x86c */
	{ 0x880,	0x00d },	/* 0x880 - 0x88c */
	{ 0x900,	0x006 },	/* 0x900 - 0x905 */
	{ 0x908,	0x001 },	/* 0x908 - 0x908 */
	{ 0x910,	0x003 },	/* 0x910 - 0x912 */
	{ 0x914,	0x003 },	/* 0x914 - 0x916 */
	{ 0x918,	0x003 },	/* 0x918 - 0x91a */
	{ 0x94f,	0x004 },	/* 0x94f - 0x952 */
	{ 0x955,	0x007 },	/* 0x955 - 0x95b */
	{ 0x960,	0x003 },	/* 0x960 - 0x962 */
	{ 0x964,	0x008 },	/* 0x964 - 0x96b */
	{ 0x970,	0x003 },	/* 0x970 - 0x972 */
	{ 0x980,	0x016 },	/* 0x980 - 0x995 */
	{ 0x9a0,	0x005 },	/* 0x9a0 - 0x9a4 */
	{ 0,	0 }
};

static int
phy_ht_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b)
{
	phy_regs_t *rl;

	wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		rl = htphy0_bphy_regs;
		phy_dump_phyregs(pi, "bphy", rl, HTPHY_TO_BPHY_OFF, b);
	}
	if (HTREV_GE(pi->pubpi.phy_rev, 1)) {
		rl = htphy1_regs;
	} else {
		rl = htphy0_regs;
	}

	phy_dump_phyregs(pi, "htphy", rl, 0, b);

	wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);

	return BCME_OK;
}
#endif // endif
#endif /* BCMDBG || BCMDBG_DUMP */
