/*
 * PHY Core module implementation
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
#include <phy_rstr.h>
#include <phy_api.h>
#include "phy_iovt.h"
#include "phy_ioct.h"
#include "phy_type.h"
#include "phy_type_disp.h"
#include <phy_cmn.h>
#include <phy_init.h>
#include <phy_wd.h>
#include <phy_ana.h>
#include <phy_ana_api.h>
#include <phy_radio.h>
#include <phy_radio_api.h>
#include <phy_tbl.h>
#include <phy_tpc.h>
#include <phy_radar.h>
#include <phy_antdiv.h>
#include <phy_noise.h>
#include <phy_temp.h>
#include <phy_rssi.h>
#include <phy_btcx.h>
#include <phy_txiqlocal.h>
#include <phy_rxiqcal.h>
#include <phy_papdcal.h>
#include <phy_vcocal.h>
#include <phy_chanmgr.h>
#include <phy_cache.h>
#include <phy_calmgr.h>
#include <phy_chanmgr_notif.h>
#include <phy_fcbs.h>
#include <phy_lpc.h>
#include <phy.h>

#include <phy_utils_var.h>

#ifndef ALL_NEW_PHY_MOD
/* TODO: remove these lines... */
#include <wlc_phy_int.h>
#include <wlc_phy_hal.h>
#endif // endif

#define PHY_TXPWR_MIN		9	/* default min tx power */

#define PHY_WREG_LIMIT	24	/* number of consecutive phy register write before a readback */
#define PHY_WREG_LIMIT_VENDOR 1	/* num of consec phy reg write before a readback for vendor */

/* local functions */
static void wlc_phy_srom_attach(phy_info_t *pi, int bandtype);
static void wlc_phy_std_params_attach(phy_info_t *pi);
static int _phy_init(phy_init_ctx_t *ctx);
static void phy_register_dumps(phy_info_t *pi);
static void phy_init_done(phy_info_t *pi);

/* attach/detach the PHY Core module to the system. */
phy_info_t *
BCMATTACHFN(phy_module_attach)(shared_phy_t *sh, void *regs, int bandtype, char *vars)
{
	phy_info_t *pi;
	uint32 sflags = 0;
	uint phyversion;
	osl_t *osh = sh->osh;

	PHY_TRACE(("wl: %s(%p, %d, %p)\n", __FUNCTION__, regs, bandtype, sh));

	if (D11REV_IS(sh->corerev, 4))
		sflags = SISF_2G_PHY | SISF_5G_PHY;
	else
		sflags = si_core_sflags(sh->sih, 0, 0);

	if (BAND_5G(bandtype)) {
		if ((sflags & (SISF_5G_PHY | SISF_DB_PHY)) == 0) {
			PHY_ERROR(("wl%d: %s: No phy available for 5G\n",
			          sh->unit, __FUNCTION__));
			return NULL;
		}
	}

	/* Figure out if we have a phy for the requested band and attach to it */
	if ((sflags & SISF_DB_PHY) && (pi = sh->phy_head)) {
		pi->vars = vars;
		/* For the second band in dualband phys, load the band specific
		 * NVRAM parameters
		  * The second condition excludes UNO3 inorder to
		  * keep the device id as 0x4360 (dual band).
		  * Purely to be backward compatible to previous UNO3 NVRAM file.
		  *
		 */
		if (ISLCNPHY(pi) &&
		    !(pi->sh->boardtype == 0x0551 && CHIPID(pi->sh->chip) == BCM4330_CHIP_ID) &&
		    phy_tpc_read_srom(pi->tpci, bandtype) != BCME_OK) {
			PHY_ERROR(("%s: phy_tpc_read_srom failed\n", __FUNCTION__));
			pi->refcnt++;
			goto err;
		}
		/* For the second band in dualband phys, just bring the core back out of reset */
		wlapi_bmac_corereset(pi->sh->physhim, pi->pubpi.coreflags);

		pi->refcnt++;
		goto exit;
	}

	/* ONLY common PI is allocated. pi->u.pi_xphy is not available yet */
	if ((pi = (phy_info_t *)MALLOC(osh, sizeof(phy_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
		return NULL;
	}
	bzero((char *)pi, sizeof(phy_info_t));

	if ((pi->pwrdet_ac = MALLOCZ(sh->osh, (SROMREV(sh->sromrev) >= 12 ? sizeof(srom12_pwrdet_t)
					       : sizeof(srom11_pwrdet_t)))) == NULL) {
	    PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", sh->unit,
	       __FUNCTION__, MALLOCED(sh->osh)));
	    goto err;
	}

	if ((pi->interf = MALLOCZ(sh->osh, sizeof(*pi->interf))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced interf %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->pwrdet = MALLOCZ(sh->osh, sizeof(*pi->pwrdet))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced pwrdet %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->txcore_temp = MALLOCZ(sh->osh, sizeof(*pi->txcore_temp))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced txcore_temp %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->def_cal_info = MALLOCZ(sh->osh, sizeof(*pi->def_cal_info))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced def_cal_info %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

#ifdef ENABLE_FCBS
	if ((pi->phy_fcbs = MALLOCZ(sh->osh, sizeof(*pi->phy_fcbs))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced phy_fcbs %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}
#endif /* ENABLE_FCBS */

	if ((pi->fem2g = MALLOCZ(sh->osh, sizeof(*pi->fem2g))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced fem2g %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->fem5g = MALLOCZ(sh->osh, sizeof(*pi->fem5g))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced fem5g %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

#ifdef WLSRVSDB
	if ((pi->srvsdb_state = MALLOCZ(sh->osh, sizeof(*pi->srvsdb_state))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced srvsdb_state %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}
#endif /* WLSRVSDB */

#if defined(WLTEST)
	if ((pi->nphy_phyreg_skipaddr =
		MALLOCZ(sh->osh, sizeof(*pi->nphy_phyreg_skipaddr) * 128)) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced nphy_phyreg_skipaddr %d bytes\n",
			sh->unit, __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}
#endif // endif

	pi->regs = (d11regs_t *)regs;
	pi->sh = sh;
	pi->vars = vars;

	/* point pi->sromi to phy_sh->sromi */
	pi->sromi = pi->sh->sromi;

	/* Good phy, increase refcnt and put it in list */
	pi->refcnt++;
	pi->next = pi->sh->phy_head;
	sh->phy_head = pi;

	/* set init power on reset to TRUE */
	pi->phy_init_por = TRUE;

#ifdef WLNOKIA_NVMEM
	pi->noknvmem = wlc_phy_noknvmem_attach(osh, pi);
	if (pi->noknvmem == NULL) {
		PHY_ERROR(("wl%d: %s: wlc_phy_noknvmem_attach failed \n", sh->unit, __FUNCTION__));
		goto err;
	}
#endif /* WLNOKIA_NVMEM */

	if ((pi->sh->boardvendor == VENDOR_APPLE) &&
	    (pi->sh->boardtype == 0x0093)) {
		pi->phy_wreg_limit = PHY_WREG_LIMIT_VENDOR;
	}
	else {
		pi->phy_wreg_limit = PHY_WREG_LIMIT;
	}
	if (BAND_2G(bandtype) && (sflags & SISF_2G_PHY)) {
		/* Set the sflags gmode indicator */
		pi->pubpi.coreflags = SICF_GMODE;
	}

	/* get the phy type & revision */
	/* Note: corereset seems to be required to get the phyversion read correctly */
	wlapi_bmac_corereset(pi->sh->physhim, pi->pubpi.coreflags);
	phyversion = R_REG(osh, &pi->regs->phyversion);
	pi->pubpi.phy_type = PHY_TYPE(phyversion);
	pi->pubpi.phy_rev = phyversion & PV_PV_MASK;

	/* Read the fabid */
	pi->fabid = si_fabid(GENERIC_PHY_INFO(pi)->sih);

	if (((pi->sh->chip == BCM43235_CHIP_ID) ||
	     (pi->sh->chip == BCM43236_CHIP_ID) ||
	     (pi->sh->chip == BCM43238_CHIP_ID) ||
	     (pi->sh->chip == BCM43234_CHIP_ID)) &&
	    ((pi->sh->chiprev == 2) || (pi->sh->chiprev == 3))) {
		pi->pubpi.phy_rev = 9;
	}

	/* LCNXN */
	if (pi->pubpi.phy_type == PHY_TYPE_LCNXN) {
		pi->pubpi.phy_type = PHY_TYPE_N;
		pi->pubpi.phy_rev += LCNXN_BASEREV;
	}

	/* Default to 1 core. Each PHY specific attach should initialize it
	 * to PHY/chip specific.
	 */
	pi->pubpi.phy_corenum = PHY_CORE_NUM_1;
	pi->pubpi.ana_rev = (phyversion & PV_AV_MASK) >> PV_AV_SHIFT;

	if (!VALID_PHYTYPE(pi)) {
		PHY_ERROR(("wl%d: %s: invalid phy_type %d\n",
		          sh->unit, __FUNCTION__, pi->pubpi.phy_type));
		goto err;
	}

	/* default channel and channel bandwidth is 20 MHZ */
	pi->bw = WL_CHANSPEC_BW_20;
	pi->radio_chanspec = BAND_2G(bandtype) ? CH20MHZ_CHSPEC(1) : CH20MHZ_CHSPEC(36);
	pi->radio_chanspec_sc = BAND_2G(bandtype) ? CH20MHZ_CHSPEC(1) : CH20MHZ_CHSPEC(36);

	/* attach nvram driven variables */
	wlc_phy_srom_attach(pi, bandtype);

	/* update standard configuration params to defaults */
	wlc_phy_std_params_attach(pi);

	/* ######## Attach process start ######## */

	/* ======== Attach infrastructure services ======== */

	/* Attach dump registry module - MUST BE THE FIRST! */
	if ((pi->dumpi = phy_dump_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_dump_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach PHY Common info */
	if ((pi->cmni = phy_cmn_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_cmn_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach PHY type specific implementation dispatch info */
	if ((pi->typei = phy_type_disp_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_type_disp_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach INIT control module */
	if ((pi->initi = phy_init_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_init_attach failed\n", __FUNCTION__));
		goto err;
	}
#ifdef NEW_PHY_CAL_ARCH
	/* Attach Channel Manager Nofitication module */
	if ((pi->chanmgr_notifi = phy_chanmgr_notif_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_chanmgr_notif_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach CACHE module */
	if ((pi->cachei = phy_cache_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_cache_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif
	/* Attach WATCHDOG module */
	if ((pi->wdi = phy_wd_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_wd_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach CALibrationManaGeR module */
	if ((pi->calmgri = phy_calmgr_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_calmgr_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* ======== Attach PHY specific layer ======== */

	/* Attach PHY Core type specific implementation */
	if (pi->typei != NULL &&
	    (*(phy_type_info_t **)(uintptr)&pi->u =
	     phy_type_attach(pi->typei, bandtype)) == NULL) {
		PHY_ERROR(("%s: phy_type_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* ======== Attach modules' common layer ======== */

	/* Attach ANAcore control module */
	if ((pi->anai = phy_ana_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_ana_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RADIO control module */
	if ((pi->radioi = phy_radio_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_radio_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach PHYTableInit module */
	if ((pi->tbli = phy_tbl_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_tbl_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach TxPowerCtrl module */
	if ((pi->tpci = phy_tpc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_tpc_attach failed\n", __FUNCTION__));
		goto err;
	}

#if defined(AP) && defined(RADAR)
	/* Attach RadarDetect module */
	if ((pi->radari = phy_radar_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_radar_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif

	/* Attach ANTennaDIVersity module */
	if ((pi->antdivi = phy_antdiv_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_antdiv_attach failed\n", __FUNCTION__));
		goto err;
	}

#ifndef WLC_DISABLE_ACI
	/* Attach NOISE module */
	if ((pi->noisei = phy_noise_attach(pi, bandtype)) == NULL) {
		PHY_ERROR(("%s: phy_noise_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif

	/* Attach TEMPerature sense module */
	if ((pi->tempi = phy_temp_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_temp_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RSSICompute module */
	if ((pi->rssii = phy_rssi_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rssi_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach BlueToothCoExistence module */
	if ((pi->btcxi = phy_btcx_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_btcx_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach TxIQLOCal module */
	if ((pi->txiqlocali = phy_txiqlocal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_txiqlocal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RxIQCal module */
	if ((pi->rxiqcali = phy_rxiqcal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rxiqcal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach PAPDCal module */
	if ((pi->papdcali = phy_papdcal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_papdcal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach VCOCal module */
	if ((pi->vcocali = phy_vcocal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_vcocal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach CHannelManaGeR module */
	if ((pi->chanmgri = phy_chanmgr_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_chanmgr_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach FCBS module */
	if ((pi->fcbsi = phy_fcbs_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_fcbs_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach LPC module */
	if ((pi->lpci = phy_lpc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_lpc_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach MISC module */
	if ((pi->misci = phy_misc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_misc_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach TSSI Cal module */
	if ((pi->tssicali = phy_tssical_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_tssi_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RXGCRS module */
	if ((pi->rxgcrsi = phy_rxgcrs_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rxgcrs_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RXSPUR module */
	if ((pi->rxspuri = phy_rxspur_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rxspur_attach failed\n", __FUNCTION__));
		goto err;
	}

#ifdef SAMPLE_COLLECT
	/* Attach sample collect module */
	if ((pi->sampi = phy_samp_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_samp_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* SAMPLE_COLLECT */

#ifdef WL_MU_RX
	/* Attach MU-MIMO module */
	if ((pi->mui = phy_mu_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_mu_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif  /* WL_MU_RX */
	/* ...Attach other modules... */

	/* ======== Attach modules' PHY specific layer ======== */

	/* Register PHY type implementation layer to common layer */
	if (pi->typei != NULL &&
	    phy_type_register_impl(pi->typei, bandtype) != BCME_OK) {
		PHY_ERROR(("%s: phy_type_register_impl failed\n", __FUNCTION__));
		goto err;
	}

#ifdef WLTXPWR_CACHE
	pi->txpwr_cache = wlc_phy_txpwr_cache_create(pi->sh->osh);
#endif // endif

	/* ######## Attach process end ######## */

	/* register reset fn */
	if (phy_init_add_init_fn(pi->initi, _phy_init, pi, PHY_INIT_PHYIMPL) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto err;
	}

	/* Make a public copy of the attach time constant phy attributes */
	bcopy(&pi->pubpi, &pi->pubpi_ro, sizeof(wlc_phy_t));

	/* register dump functions */
	phy_register_dumps(pi);

exit:
	/* Mark that they are not longer available so we can error/assert.  Use a pointer
	 * to self as a flag.
	 */
	pi->vars = (char *)&pi->vars;
	return pi;

err:
	phy_module_detach(pi);
	return NULL;
}

static void
BCMATTACHFN(wlc_phy_srom_attach)(phy_info_t *pi, int bandtype)
{
	int i = 0;

	pi->ucode_tssi_limit_en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssilimucod, 1);
	pi->rssi_corr_normal = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssicorrnorm, 0);
	pi->rssi_corr_boardatten = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssicorratten, 7);

	/* Re-init the interference value based on the nvram variables */
	if (PHY_GETVAR(pi, rstr_interference) != NULL) {
		pi->sh->interference_mode_2G = (int)PHY_GETINTVAR(pi, rstr_interference);
		pi->sh->interference_mode_5G = (int)PHY_GETINTVAR(pi, rstr_interference);

		if (BAND_2G(bandtype))
			pi->sh->interference_mode = pi->sh->interference_mode_2G;
		else
			pi->sh->interference_mode = pi->sh->interference_mode_5G;
	}

#if defined(RXDESENS_EN)
	/* phyrxdesens in db for SS SPC production */
	pi->sh->phyrxdesens = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_phyrxdesens, 0);
#endif // endif

#ifdef BAND5G
	for (i = 0; i < 3; i++) {
		pi->rssi_corr_normal_5g[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssicorrnorm5g, i, 0);
		pi->rssi_corr_boardatten_5g[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssicorratten5g, i, 0);
	}
#endif /* BAND5G */
	/* The below parameters are used to adjust JSSI based range */
	pi->rssi_corr_perrg_2g[0] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
		rstr_rssicorrperrg2g, 0, -150);
	pi->rssi_corr_perrg_2g[1] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
		rstr_rssicorrperrg2g, 1, -150);
#ifdef BAND5G
	pi->rssi_corr_perrg_5g[0] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
		rstr_rssicorrperrg5g, 0, -150);
	pi->rssi_corr_perrg_5g[1] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
		rstr_rssicorrperrg5g, 1, -150);
#endif /* BAND5G */
	for (i = 2; i < 5; i++) {
		pi->rssi_corr_perrg_2g[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssicorrperrg2g, i, 0);
#ifdef BAND5G
		pi->rssi_corr_perrg_5g[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_rssicorrperrg5g, i, 0);
#endif /* BAND5G */
	}

	for (i = 0; i < 14; i++)
		pi->phy_cga_2g[i] = (int8)PHY_GETINTVAR_ARRAY(pi, rstr_2g_cga, i);
#ifdef BAND5G
	for (i = 0; i < 24; i++)
		pi->phy_cga_5g[i] = (int8)PHY_GETINTVAR_ARRAY(pi, rstr_5g_cga, i);
#endif /* BAND5G */

	pi->min_txpower = PHY_TXPWR_MIN;
	pi->tx_pwr_backoff = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_txpwrbckof, 6);

	pi->phy_tempsense_offset = 0;
	/* Read default temp delta setting. */
	wlc_phy_read_tempdelta_settings(pi, NPHY_CAL_MAXTEMPDELTA);
}

static void
BCMATTACHFN(wlc_phy_std_params_attach)(phy_info_t *pi)
{
	/* set default rx iq est antenna/samples */
	pi->phy_rxiq_samps = PHY_NOISE_SAMPLE_LOG_NUM_NPHY;
	pi->phy_rxiq_antsel = ANT_RX_DIV_DEF;

	/* initialize SROM "isempty" flags for rxgainerror */
	pi->rxgainerr2g_isempty = FALSE;
	pi->rxgainerr5gl_isempty = FALSE;
	pi->rxgainerr5gm_isempty = FALSE;
	pi->rxgainerr5gh_isempty = FALSE;
	pi->rxgainerr5gu_isempty = FALSE;

	/* Do not enable the PHY watchdog for ATE */
#ifndef ATE_BUILD
	pi->phywatchdog_override = TRUE;
#endif // endif
	/* pi->phy_rx_diglpf_default_coeffs are not set yet */
	pi->phy_rx_diglpf_default_coeffs_valid = FALSE;

	/* Enable both cores by default */
	pi->sh->phyrxchain = 0x3;

#ifdef N2WOWL
	/* Reduce phyrxchain to 1 to save power in WOWL mode */
	if (CHIPID(pi->sh->chip) == BCM43237_CHIP_ID) {
		pi->sh->phyrxchain = 0x1;
	}
#endif /* N2WOWL */

#if defined(WLTEST)
	/* Initialize to invalid index values */
	pi->nphy_tbldump_minidx = -1;
	pi->nphy_tbldump_maxidx = -1;
	pi->nphy_phyreg_skipcnt = 0;
#endif // endif

	/* This is the temperature at which the last PHYCAL was done.
	 * Initialize to a very low value.
	 */
	pi->def_cal_info->last_cal_temp = -50;
	pi->def_cal_info->cal_suppress_count = 0;

	/* default, PHY type overrides if interrupt based noise measurement isn't supported */
	pi->phynoise_polling = TRUE;

	/* still need to have this information hanging around, even for OPT version */
	pi->tx_power_offset = NULL;

	/* initialize our txpwr limit to a large value until we know what band/channel
	 * we settle on in wlc_up() set the txpwr user override to the max
	 */
	pi->tx_user_target = WLC_TXPWR_MAX;

#ifdef WL_SARLIMIT
	memset(pi->sarlimit, WLC_TXPWR_MAX, PHY_MAX_CORES);
#endif /* WL_SARLIMIT */

	/* default radio power */
	pi->radiopwr_override = RADIOPWR_OVERRIDE_DEF;

	/* Assign the default cal info */
	pi->cal_info = pi->def_cal_info;
	pi->cal_info->cal_suppress_count = 0;

	/* set default power output percentage to 100 percent */
	pi->txpwr_percent = 100;

	/* this will get the value from the SROM */
	pi->papdcal_indexdelta = 4;
	pi->papdcal_indexdelta_default = 4;
	pi->txpwr_degrade = 0;
}

void
BCMATTACHFN(phy_module_detach)(phy_info_t *pi)
{
	PHY_TRACE(("wl: %s: pi = %p\n", __FUNCTION__, pi));

	if (pi == NULL)
		return;

	ASSERT(pi->refcnt > 0);

	if (--pi->refcnt)
		return;

	/* ======== Detach modules' PHY specific layer ======== */

#ifdef SAMPLE_COLLECT
	/* Detach SAMPle collect module */
	if (pi->sampi != NULL)
		phy_samp_detach(pi->sampi);
#endif /* SAMPLE_COLLECT */

	if (pi->pwrdet_ac != NULL) {
	    MFREE(pi->sh->osh, pi->pwrdet_ac, (SROMREV(pi->sh->sromrev) >= 12 ?
	       sizeof(srom12_pwrdet_t) : sizeof(srom11_pwrdet_t)));
	    pi->pwrdet_ac = NULL;
	}

	/* Detach RXSPUR module */
	if (pi->rxspuri != NULL)
		phy_rxspur_detach(pi->rxspuri);

	/* Detach RXGCRS module */
	if (pi->rxgcrsi != NULL)
		phy_rxgcrs_detach(pi->rxgcrsi);

	/* Detach TSSI Cal module */
	if (pi->tssicali != NULL)
		phy_tssical_detach(pi->tssicali);

	/* Unregister PHY type implementations from common - MUST BE THE FIRST! */
	if (pi->typei != NULL)
		phy_type_unregister_impl(pi->typei);

	/* ======== Detach modules' common layer ======== */

	/* Detach misc module */
	if (pi->misci != NULL)
		phy_misc_detach(pi->misci);

	/* Detach LPC module */
	if (pi->lpci != NULL)
		phy_lpc_detach(pi->lpci);

	/* Detach FCBS module */
	if (pi->fcbsi != NULL)
		phy_fcbs_detach(pi->fcbsi);

	/* Detach CHannelManaGeR module */
	if (pi->chanmgri != NULL)
		phy_chanmgr_detach(pi->chanmgri);

	/* Detach VCO Cal module */
	if (pi->vcocali != NULL)
		phy_vcocal_detach(pi->vcocali);

	/* Detach PAPD Cal module */
	if (pi->papdcali != NULL)
		phy_papdcal_detach(pi->papdcali);

	/* Detach RXIQ Cal module */
	if (pi->rxiqcali != NULL)
		phy_rxiqcal_detach(pi->rxiqcali);

	/* Detach TXIQLO Cal module */
	if (pi->txiqlocali != NULL)
		phy_txiqlocal_detach(pi->txiqlocali);

	/* Detach BlueToothCoExistence module */
	if (pi->btcxi != NULL)
		phy_btcx_detach(pi->btcxi);

	/* Detach RSSICompute module */
	if (pi->rssii != NULL)
		phy_rssi_detach(pi->rssii);

	/* Detach TEMPerature sense module */
	if (pi->tempi != NULL)
		phy_temp_detach(pi->tempi);

#ifndef WLC_DISABLE_ACI
	/* Detach INTerFerence module */
	if (pi->noisei != NULL)
		phy_noise_detach(pi->noisei);
#endif // endif

	/* Detach ANTennaDIVersity module */
	if (pi->antdivi != NULL)
		phy_antdiv_detach(pi->antdivi);

#if defined(AP) && defined(RADAR)
	/* Detach RadarDetect module */
	if (pi->radari != NULL)
		phy_radar_detach(pi->radari);
#endif // endif

	/* Detach TxPowerCtrl module */
	if (pi->tpci != NULL)
		phy_tpc_detach(pi->tpci);

	/* Detach PHYTableInit module */
	if (pi->tbli != NULL)
		phy_tbl_detach(pi->tbli);

	/* Detach RADIO control module */
	if (pi->radioi != NULL)
		phy_radio_detach(pi->radioi);

	/* Detach ANAcore control module */
	if (pi->anai != NULL)
		phy_ana_detach(pi->anai);

#ifdef WL_MU_RX
	/* Detach MU-MIMO module */
	if (pi->mui != NULL)
		phy_mu_detach(pi->mui);
#endif /* WL_MU_RX */
	/* ...Detach other modules... */

	/* ======== Detach infrastructure services ======== */

	/* Detach PHY type implementation layer from common layer */
	if (pi->typei != NULL &&
	    *(phy_type_info_t **)(uintptr)&pi->u != NULL)
		phy_type_detach(pi->typei, *(phy_type_info_t **)(uintptr)&pi->u);

	/* Detach CALibrationManaGeR module */
	if (pi->calmgri != NULL)
		phy_calmgr_detach(pi->calmgri);

	/* Detach watchdog module */
	if (pi->wdi != NULL)
		phy_wd_detach(pi->wdi);
#ifdef NEW_PHY_CAL_ARCH
	/* Detach CACHE module */
	if (pi->cachei != NULL)
		phy_cache_detach(pi->cachei);

	/* Detach CHannelManaGeR Notification module */
	if (pi->chanmgr_notifi != NULL)
		phy_chanmgr_notif_detach(pi->chanmgr_notifi);
#endif // endif
	/* Detach INIT control module */
	if (pi->initi != NULL)
		phy_init_detach(pi->initi);

	/* Detach PHY type implementation dispatch info */
	if (pi->typei != NULL)
		phy_type_disp_detach(pi->typei);

	/* Detach PHY Common info */
	if (pi->cmni != NULL)
		phy_cmn_detach(pi->cmni);

	/* Detach dump registry - MUST BE THE LAST */
	if (pi->dumpi != NULL)
		phy_dump_detach(pi->dumpi);

/* *********************************************** */

	if (pi->interf != NULL) {
	    MFREE(pi->sh->osh, pi->interf, sizeof(*pi->interf));
		pi->interf = NULL;
	}

	if (pi->pwrdet != NULL) {
	    MFREE(pi->sh->osh, pi->pwrdet, sizeof(*pi->pwrdet));
		pi->pwrdet = NULL;
	}

	if (pi->txcore_temp != NULL) {
	    MFREE(pi->sh->osh, pi->txcore_temp, sizeof(*pi->txcore_temp));
		pi->txcore_temp = NULL;
	}

#ifdef ENABLE_FCBS
	if (pi->phy_fcbs != NULL) {
	    MFREE(pi->sh->osh, pi->phy_fcbs, sizeof(*pi->phy_fcbs));
		pi->phy_fcbs = NULL;
	}
#endif /* ENABLE_FCBS */

	if (pi->fem2g != NULL) {
	    MFREE(pi->sh->osh, pi->fem2g, sizeof(*pi->fem2g));
		pi->fem2g = NULL;
	}

	if (pi->fem5g != NULL) {
	    MFREE(pi->sh->osh, pi->fem5g, sizeof(*pi->fem5g));
		pi->fem5g = NULL;
	}

#ifdef WLSRVSDB
	if (pi->srvsdb_state != NULL) {
	    MFREE(pi->sh->osh, pi->srvsdb_state, sizeof(*pi->srvsdb_state));
		pi->srvsdb_state = NULL;
	}
#endif /* WLSRVSDB */

#if defined(WLTEST)
	if (pi->nphy_phyreg_skipaddr != NULL) {
		MFREE(pi->sh->osh, pi->nphy_phyreg_skipaddr,
			sizeof(*pi->nphy_phyreg_skipaddr) * 128);
		pi->nphy_phyreg_skipaddr = NULL;
	}
#endif // endif

#if defined(PHYCAL_CACHING)
	pi->phy_calcache_on = FALSE;
	wlc_phy_cal_cache_deinit((wlc_phy_t *)pi);
#endif // endif

	if (pi->def_cal_info != NULL) {
	    MFREE(pi->sh->osh, pi->def_cal_info, sizeof(*pi->def_cal_info));
		pi->def_cal_info = NULL;
	}

	/* Quick-n-dirty remove from list */
	if (pi->sh->phy_head == pi)
		pi->sh->phy_head = pi->next;
	else if (pi->sh->phy_head->next == pi)
		pi->sh->phy_head->next = NULL;
	else
		ASSERT(0);

#ifdef WLTXPWR_CACHE
	if (pi->tx_power_offset != NULL)
		wlc_phy_clear_tx_power_offset((wlc_phy_t *)pi);
#if defined(WLC_LOW_ONLY) || defined(WLTXPWR_CACHE_PHY_ONLY)
	if (pi->txpwr_cache != NULL)
		wlc_phy_txpwr_cache_close(pi->sh->osh, pi->txpwr_cache);
#endif // endif
#else
	if (pi->tx_power_offset != NULL)
		ppr_delete(pi->sh->osh, pi->tx_power_offset);
#endif	/* WLTXPWR_CACHE */

	MFREE(pi->sh->osh, pi, sizeof(phy_info_t));
}

/* Register all iovar tables to/from system */
int
BCMATTACHFN(phy_register_iovt_all)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	int err;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Register all common layer's iovar tables/handlers */
	if ((err = phy_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PHY type implementation layer's iovar tables/handlers */
	if ((err = phy_type_register_iovt(pi->typei, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_type_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

/* Register all ioctl tables to/from system */
int
BCMATTACHFN(phy_register_ioct_all)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	int err;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Register all common layer's ioctl tables/handlers */
	if ((err = phy_register_ioct(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_register_ioct failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PHY type implementation layer's ioctl tables/handlers */
	if ((err = phy_type_register_ioct(pi->typei, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_type_register_ioct failed\n", __FUNCTION__));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

/* band specific init */
void
WLBANDINITFN(phy_bsinit)(phy_info_t *pi, chanspec_t chanspec, bool forced)
{
	/* if chanswitch path, skip phy_init for D11REV > 40 */
	if (!ISACPHY(pi) || forced)
		phy_init(pi, chanspec);
}

/* band width init */
void
WLBANDINITFN(phy_bwinit)(phy_info_t *pi, chanspec_t chanspec)
{
	if (!ISACPHY(pi))
		phy_init(pi, chanspec);
}

/* Init/deinit the PHY h/w. */
void
WLBANDINITFN(phy_init)(phy_info_t *pi, chanspec_t chanspec)
{
	uint32	mc;
#ifdef BCMDBG
	char chbuf[CHANSPEC_STR_LEN];
#endif // endif

	ASSERT(pi != NULL);

	PHY_TRACE(("wl%d: %s chanspec %s\n", pi->sh->unit, __FUNCTION__,
		wf_chspec_ntoa(chanspec, chbuf)));

	/* skip if this function is called recursively(e.g. when bw is changed) */
	if (pi->init_in_progress)
		return;

	pi->last_radio_chanspec = pi->radio_chanspec;

	pi->init_in_progress = TRUE;
	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);
	pi->phynoise_state = 0;

	/* Update ucode channel value */
	wlc_phy_chanspec_shm_set(pi, chanspec);

	mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);
	if ((mc & MCTL_EN_MAC) != 0) {
		if (mc == 0xffffffff)
			PHY_ERROR(("wl%d: %s: chip is dead !!!\n", pi->sh->unit, __FUNCTION__));
		else
			PHY_ERROR(("wl%d: %s: MAC running! mc=0x%x\n",
			          pi->sh->unit, __FUNCTION__, mc));
		ASSERT((const char*)"wlc_phy_init: Called with the MAC running!" == NULL);
	}

	/* clear during init. To be set by higher level wlc code */
	pi->cur_interference_mode = INTERFERE_NONE;

	/* init PUB_NOT_ASSOC */
	if (!(pi->measure_hold & PHY_HOLD_FOR_SCAN) &&
	    !(pi->interf->aci.nphy.detection_in_progress)) {
#ifdef WLSRVSDB
		if (!pi->srvsdb_state->srvsdb_active)
			pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
#else
		pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
#endif // endif
	}

	/* check D11 is running on Fast Clock */
	if (D11REV_GE(pi->sh->corerev, 5)) {
		ASSERT(si_core_sflags(pi->sh->sih, 0, 0) & SISF_FCLKA);
	}

	/* ######## Init process start ######## */

	/* ======== Common inits ======== */

	/* Init each feature/module including s/w and h/w */
	if (phy_init_invoke_init_fns(pi->initi) != BCME_OK) {
		PHY_ERROR(("wl%d: %s: phy_init_invoke_init_fns failed for phy_type %d, rev %d\n",
		          pi->sh->unit, __FUNCTION__, pi->pubpi.phy_type, pi->pubpi.phy_rev));
		return;
	}

	/* ======== Special inits ======== */

	/* ^^^Add other special init calls here^^^ */

	/* ######## Init process end ######## */

	/* Indicate a power on reset isn't needed for future phy init's */
	pi->phy_init_por = FALSE;

	pi->init_in_progress = FALSE;

	/* clear flag */
	phy_init_done(pi);

	pi->bt_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_BTCX_BLK_PTR);
}

/* driver up/init processing */
static int
WLBANDINITFN(_phy_init)(phy_init_ctx_t *ctx)
{
	phy_info_t *pi = (phy_info_t *)ctx;

	return phy_type_init_impl(pi->typei);
}

int
BCMUNINITFN(phy_down)(phy_info_t *pi)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

#ifndef BCMNODOWN
	/* all activate phytest should have been stopped */
	ASSERT(pi->phytest_on == FALSE);

	/* ^^^Add other special down calls here^^^ */

	/* ======== Common down ======== */

	phy_init_invoke_down_fns(pi->initi);
#endif /* !BCMNODOWN */

	return callbacks;
}

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
static int
_phy_dump_phyregs(phy_info_t *pi, struct bcmstrbuf *b)
{
	if (!pi->sh->clk)
		return BCME_NOCLK;

	return phy_type_dump_phyregs(pi->typei, b);

}

/* dump phyregs listed in 'reglist' */
void
phy_dump_phyregs(phy_info_t *pi, const char *str,
	const phy_regs_t *reglist, uint16 off, struct bcmstrbuf *b)
{
	uint16 addr, val = 0, num;
#if defined(WLTEST)
	uint16 i = 0;
	bool skip;
#endif // endif

	if (reglist == NULL)
		return;

	bcm_bprintf(b, "----- %06s -----\n", str);
	bcm_bprintf(b, "Add Value\n");

	while ((num = reglist->num) > 0) {
#if defined(WLTEST)
		skip = FALSE;

		for (i = 0; i < pi->nphy_phyreg_skipcnt; i++) {
			if (pi->nphy_phyreg_skipaddr[i] == reglist->base) {
				skip = TRUE;
				break;
			}
		}

		if (skip) {
			reglist++;
			continue;
		}
#endif // endif

		for (addr = reglist->base + off; num && b->size > 0; addr++, num--) {
			/* XXX PR41476 WAR: Prevent MAC from accessing PHY registers while the
			 * host is
			 */
			if (D11REV_IS(pi->sh->corerev, 11) ||
			    D11REV_IS(pi->sh->corerev, 12)) {
				wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
				(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
				OSL_DELAY(1);
			}

			val = phy_type_read_phyreg(pi->typei, addr);

			if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
				wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);

			if (PHY_INFORM_ON() && si_taclear(pi->sh->sih, FALSE)) {
				PHY_INFORM(("%s: TA reading phy reg %s:0x%x\n",
				           __FUNCTION__, str, addr));
				bcm_bprintf(b, "%03x tabort\n", addr);
			} else
				bcm_bprintf(b, "%03x %04x\n", addr, val);
		}
		reglist++;
	}
}
#endif // endif

static void
BCMATTACHFN(phy_register_dumps)(phy_info_t *pi)
{
#if defined(BCMDBG_PHYDUMP)
	phy_dbg_add_dump_fn(pi, "phyreg", (phy_dump_fn_t)_phy_dump_phyregs, pi);
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	phy_dbg_add_dump_fn(pi, "phycal", (phy_dump_fn_t)wlc_phydump_phycal, pi);
	phy_dbg_add_dump_fn(pi, "phypapd", (phy_dump_fn_t)wlc_phydump_papd, pi);
	phy_dbg_add_dump_fn(pi, "phystate", (phy_dump_fn_t)wlc_phydump_state, pi);
	phy_dbg_add_dump_fn(pi, "phylnagain", (phy_dump_fn_t)wlc_phydump_lnagain, pi);
	phy_dbg_add_dump_fn(pi, "phyinitgain", (phy_dump_fn_t)wlc_phydump_initgain, pi);
	phy_dbg_add_dump_fn(pi, "phyhpf1tbl", (phy_dump_fn_t)wlc_phydump_hpf1tbl, pi);
	phy_dbg_add_dump_fn(pi, "phychanest", (phy_dump_fn_t)wlc_phydump_chanest, pi);
#ifdef ENABLE_FCBS
	phy_dbg_add_dump_fn(pi, "phyfcbs", (phy_dump_fn_t)wlc_phydump_fcbs, pi);
#endif /* ENABLE_FCBS */
	phy_dbg_add_dump_fn(pi, "phytxv0", (phy_dump_fn_t)wlc_phydump_txv0, pi);
#if defined(DBG_BCN_LOSS)
	phy_dbg_add_dump_fn(pi, "phycalrxmin", (phy_dump_fn_t)wlc_phydump_phycal_rx_min, pi);
#endif // endif
#endif /* BCMDBG || BCMDBG_DUMP */

#if defined(WLTEST)
	phy_dbg_add_dump_fn(pi, "phych4rpcal", (phy_dump_fn_t)wlc_phydump_ch4rpcal, pi);
#endif /* WLTEST */

}

/* PHYMODE switch requires a clean phy initialization,
 * set a flag to indicate phyinit is pending
 */
bool
phy_init_pending(phy_info_t *pi)
{
	ASSERT(pi != NULL);
	return pi->phyinit_pending;
}

/* clear flag upon phyinit */
static void
phy_init_done(phy_info_t *pi)
{
	ASSERT(pi != NULL);
	pi->phyinit_pending = FALSE;
}

mbool
phy_get_measure_hold_status(phy_info_t *pi)
{
	return pi->measure_hold;
}

void
phy_set_measure_hold_status(phy_info_t *pi, mbool set)
{
	pi->measure_hold = set;
}
