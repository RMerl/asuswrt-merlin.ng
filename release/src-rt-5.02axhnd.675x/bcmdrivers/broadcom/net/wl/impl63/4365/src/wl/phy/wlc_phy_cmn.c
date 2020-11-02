/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11 Networking Device Driver.
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
 * $Id: wlc_phy_cmn.c 766421 2018-08-01 10:53:48Z $
 */

/* XXX WARNING: phy structure has been changed, read this first
 *
 * This is top level of PHY common module for WLAN chips. Major components are
 *         macro, typedef, enum, structure, global variable
 *         common PHY utilities: register read/write, table read/write
 *         common PHY operations: attach/detach/reset/init/down/up
 *         chanspec
 *         tx power control
 *         calibration: tx/rx, aci, PAPD etc
 *         radar
 *         user control: ioctl, iovar, dump
 * ================================================================================================
 * 2009 March: in TOT, the old monolithic wlc_phy.c was split into this common module and several
 *   submodule to serve different PHY and radio.
 *   To get old cvs history before split: cvs annorate -r 1.2084 wlc_phy_cmn.c
 *   Refer to http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/WlDriverModulePhy for details
 *   The PHY module architecture is below
 *         wlc_phy.h(public interface)
 *         --------------------------------------------
 *         wlc_phy_cmn.c (common, shared PHY submodule)
 *         --------------------------------------------
 *         wlc_phy_int.h(private interface)
 *         --------------------------------------------
 *         wlc_phy_n.c(PHY type specific code)
 *
 * ================================================================================================
 * Naming convention is
 *         wlc_phy_submodule_noun_verb_<phytype>
 *         i.e. wlc_phy_radar_detect_run
 *              wlc_phy_radar_detect_run_nphy
 * compile flags:
 *     BCMINTERNAL is for internal code, not available to any external customer release
 *     BCMDBG is for debug code, available to customer in external debug build.
 *     WLTEST is needed for manufacturing feature code
 *     BCMINITFN() will make the code reclaimable (after driver up)
 *
 * MERGE: manual merge(instead of cvs auto merge) is required for any old branch code
 *        test your change on NPHY(4321, 4322)
 *
 * Good practice:
 *  Avoid using macros due to lack of type checking, encapsulation and code replication
 *  reduce using ifdef except for NCONF for code size optimization
 *  Define/enum constant, hard numbers are difficult to understand and maintain
 *  Sharing code as much as possible
 *  write simple/clean code(fewer if-else, goto), think of scalability
 * ================================================================================================
 * TODO:
 *        shared more common flow
 *        unify iovar/ioctl
 *        optimize code
*/

#include <wlc_cfg.h>
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <bcmutils.h>
#include <d11.h>

/* *********************************************** */
#include <phy_dbg.h>
#include <phy_tpc.h>
#include <phy_noise.h>
#include <phy_antdiv_api.h>
#include <phy_radar.h>
#include <phy_temp.h>

#include <phy_lcn_rssi.h>
#include <phy_lcn40_rssi.h>
#include <phy_ac_tpc.h>
/* *********************************************** */

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phyreg_n.h>
#include <wlc_phyreg_ht.h>
#include <wlc_phyreg_ac.h>
#include <wlc_phyreg_lcn.h>
#include <wlc_phyreg_lcn40.h>
#include <wlc_phytbl_n.h>
#include <wlc_phytbl_ht.h>
#include <wlc_phytbl_ac.h>
#include <wlc_phytbl_20691.h>
#include <wlc_phy_radio.h>
#include <wlc_phy_lcn.h>
#include <wlc_phy_lcn40.h>
#include <wlc_phy_n.h>
#include <wlc_phy_ht.h>
#if (ACCONF != 0) || (ACCONF2 != 0)
#include <wlc_phy_ac.h>
#endif /* (ACCONF != 0) || (ACCONF2 != 0) */
#include <bcmwifi_channels.h>
#include <bcmotp.h>

#if     defined(BCM_OL_DEV)
int8 wlc_phy_noise_sample_acphy(wlc_phy_t *pih);
#endif /* defined(BCM_OL_DEV) */

#ifdef  WLOFFLD
#include <wlc_phy_shim.h>
extern int8 wlc_ol_noise_avg_offload(void *wlc);
#endif // endif
#ifdef WLSRVSDB
#include <saverestore.h>
#endif // endif

#ifdef WLNOKIA_NVMEM
#include <wlc_phy_noknvmem.h>
#endif /* WLNOKIA_NVMEM */

#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_utils_status.h>
#include <phy_utils_reg.h>
#include <phy_utils_channel.h>
#include <phy_utils_api.h>
#include <phy_ac_info.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  macro, typedef, enum, structure, global variable		*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
#define RSSI_IQEST_DEBUG 0
#define RSSI_CORR_EN 1

#if defined(WLTEST)
#define BFECONFIGREF_FORCEVAL    0x9
#define BFMCON_FORCEVAL          0x8c03
#define BFMCON_RELEASEVAL        0x8c1d
#define REFRESH_THR_FORCEVAL     0xffff
#define REFRESH_THR_RELEASEVAL   0x186a
#define BFRIDX_POS_FORCEVAL      0x100
#define BFRIDX_POS_RELEASEVAL    0x0
#endif // endif

/* channel info structure */
typedef struct _chan_info_basic {
	uint16	chan;		/* channel number */
	uint16	freq;		/* in Mhz */
} chan_info_basic_t;

uint16 ltrn_list[PHY_LTRN_LIST_LEN] = {
	0x18f9, 0x0d01, 0x00e4, 0xdef4, 0x06f1, 0x0ffc,
	0xfa27, 0x1dff, 0x10f0, 0x0918, 0xf20a, 0xe010,
	0x1417, 0x1104, 0xf114, 0xf2fa, 0xf7db, 0xe2fc,
	0xe1fb, 0x13ee, 0xff0d, 0xe91c, 0x171a, 0x0318,
	0xda00, 0x03e8, 0x17e6, 0xe9e4, 0xfff3, 0x1312,
	0xe105, 0xe204, 0xf725, 0xf206, 0xf1ec, 0x11fc,
	0x14e9, 0xe0f0, 0xf2f6, 0x09e8, 0x1010, 0x1d01,
	0xfad9, 0x0f04, 0x060f, 0xde0c, 0x001c, 0x0dff,
	0x1807, 0xf61a, 0xe40e, 0x0f16, 0x05f9, 0x18ec,
	0x0a1b, 0xff1e, 0x2600, 0xffe2, 0x0ae5, 0x1814,
	0x0507, 0x0fea, 0xe4f2, 0xf6e6
};

/* Decode OFDM PLCP SIGNAL field RATE sub-field bits 0:2 (labeled R1-R3) into
 * 802.11 MAC rate in 500kbps units
 *
 * Table from 802.11-2012, sec 18.3.4.2.
 */
const uint8 ofdm_rate_lookup[] = {
	DOT11_RATE_48M, /* 8: 48Mbps */
	DOT11_RATE_24M, /* 9: 24Mbps */
	DOT11_RATE_12M, /* A: 12Mbps */
	DOT11_RATE_6M,  /* B:  6Mbps */
	DOT11_RATE_54M, /* C: 54Mbps */
	DOT11_RATE_36M, /* D: 36Mbps */
	DOT11_RATE_18M, /* E: 18Mbps */
	DOT11_RATE_9M   /* F:  9Mbps */
};

#ifdef LP_P2P_SOFTAP
uint8 pwr_lvl_qdB[LCNPHY_TX_PWR_CTRL_MACLUT_LEN];
#endif /* LP_P2P_SOFTAP */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  local prototype						*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* %%%%%% major operations */

/* %%%%%% Calibration, ACI, noise/rssi measurement */
static void wlc_phy_noise_calc(phy_info_t *pi, uint32 *cmplx_pwr, int8 *pwr_ant,
                               uint8 extra_gain_1dB);
static void wlc_phy_noise_calc_fine_resln(phy_info_t *pi, uint32 *cmplx_pwr, int16 *pwr_ant,
                                          uint8 extra_gain_1dB, int16 *tot_gain);

static uint8 wlc_phy_calc_extra_init_gain(phy_info_t *pi, uint8 extra_gain_3dB,
                                        rxgain_t rxgain[]);

#ifndef WLC_DISABLE_ACI
static void wlc_phy_noisemode_upd(phy_info_t *pi);
#if defined(RXDESENS_EN)
static bool wlc_phy_interference(phy_info_t *pi, int wanted_mode, bool init);
#endif // endif
static int8 wlc_phy_cmn_noisemode_glitch_chk_adj(phy_info_t *pi, uint16 glitch_badplcp_sum_ma,
	noise_thresholds_t *thresholds);
static void wlc_phy_cmn_noise_limit_desense(phy_info_t *pi);
#if defined(WLTEST) || defined(WL_PHYACIARGS)
static int  wlc_phy_aci_args(phy_info_t *pi, wl_aci_args_t *params, bool get, int len);
#endif // endif
static void wlc_phy_aci_update_ma(phy_info_t *pi);
void wlc_phy_aci_noise_reset_nphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc);
#endif /* Compiling out ACI code for 4324 */
static int wlc_phy_set_interference_override_mode(phy_info_t* pi, int val);

void wlc_phy_aci_noise_reset_htphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc);

static void wlc_phy_cal_perical_mphase_schedule(phy_info_t *pi, uint delay);
void wlc_phy_rxgainctrl_set_gaintbls_acphy_tiny(phy_info_t *pi, uint8 core,
	uint16 gain_tblid, uint16 gainbits_tblid);
void wlc_phy_rxgainctrl_gainctrl_acphy_tiny(phy_info_t *pi, uint8 init_desense);

/* %%%%%% power control */
static void wlc_phy_txpwr_srom_convert_mcs_offset(uint32 po, uint8 offset, uint8 max_pwr,
	ppr_ht_mcs_rateset_t* mcs, int8 mcs7_15_offset);
static void wlc_phy_txpower_recalc_target(phy_info_t *pi, ppr_t *txpwr_reg, ppr_t *txpwr_targets);
#ifdef WL_MU_TX
static void wlc_phy_offload_ppr_to_svmp(phy_info_t *pi, ppr_t* tx_power_offset, int16 floor_pwr);
#endif // endif
#ifdef WLTXPWR_CACHE
static void wlc_phy_txpower_retrieve_cached_target(phy_info_t *pi);
#if defined(WLC_LOW_ONLY)
static void wlc_phy_pwr_cache_reserve(phy_info_t *pi, chanspec_t chanspec);
#endif /* WLC_LOW_ONLY */
#endif /* WLTXPWR_CACHE */
static void wlc_phy_txpower_reg_limit_calc(phy_info_t *pi, ppr_t *txpwr, chanspec_t chanspec,
	ppr_t *txpwr_limit);
static bool wlc_phy_cal_txpower_recalc_sw(phy_info_t *pi);

/* %%%%%% testing */
#if defined(BCMDBG) || defined(WLTEST)
static int wlc_phy_test_carrier_suppress(phy_info_t *pi, int channel);
static int wlc_phy_test_freq_accuracy(phy_info_t *pi, int channel);
static int wlc_phy_test_evm(phy_info_t *pi, int channel, uint rate, int txpwr);
#endif // endif

/* channel info structure */
static uint32 wlc_phy_rx_iq_est(phy_info_t *pi, uint8 samples, uint8 antsel, uint8 resolution,
	uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
	uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type, int16 *iqest);

static int wlc_phy_iovar_dispatch_old(phy_info_t *pi, uint32 actionid, void *p, void *a, int vsize,
	int32 int_val, bool bool_val);

static int wlc_phy_iovars_nphy(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);
static int wlc_phy_iovars_acphy(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);
static int wlc_phy_iovars_lcncmnphy(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);
static int wlc_phy_iovars_phy_specific(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);

static int wlc_phy_iovars_aci(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);
static int wlc_phy_iovars_rssi(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);
static int wlc_phy_iovars_calib(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);
static int wlc_phy_iovars_generic(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);
static int wlc_phy_iovars_txpwrctl(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);

#ifdef PREASSOC_PWRCTRL
static void wlc_phy_store_tx_pwrctrl_setting(phy_info_t *pi, chanspec_t previous_chanspec);
#endif // endif

static void wlc_phy_btc_adjust(phy_info_t *pi);
#if defined(PHYCAL_CACHING) && defined(BCMDBG)
static void wlc_phydump_chanctx(phy_info_t *phi, struct bcmstrbuf *b);
#endif // endif
#if !defined(EFI)
static void ratmodel_paparams_fix64(ratmodel_paparams_t* rsd, int m);
#if PHY_TSSI_CAL_DBG_EN
static void print_int64(int64 *a);
#endif // endif
static uint16 tssi_cal_sweep(phy_info_t *pi);
int wlc_phy_tssi_cal(phy_info_t *pi);
#else
#define wlc_phy_tssi_cal(a)	do {} while (0)
#endif /* #if !defined(EFI) */

static int8 wlc_phy_channel_gain_adjust(phy_info_t *pi);

#ifdef ENABLE_FCBS
bool wlc_phy_hw_fcbs_init(wlc_phy_t *ppi, int chanidx);
bool wlc_phy_hw_fcbs_init_chanidx(wlc_phy_t *ppi, int chanidx);
int wlc_phy_hw_fcbs(wlc_phy_t *ppi, int chanidx, bool set);
#endif /* ENABLE_FCBS */
extern void wlc_phy_init_test_acphy(phy_info_t *pi);

/* Modularise and clean up attach functions */
#if ((ACCONF != 0) || (ACCONF2 != 0) || (NCONF != 0) || (HTCONF != 0) || (LCN40CONF != \
	0))
int wlc_phy_adjust_ed_thres(phy_info_t *pi, int32 *assert_thresh_dbm, bool set_threshold);
#endif /* ((ACCONF != 0) || (ACCONF2 != 0) || (NCONF != 0) || (HTCONF != 0) || (LCN40CONF != 0)) */

#if (defined(WLTEST) || defined(WLPKTENG))
bool wlc_phy_isperratedpden(wlc_phy_t *ppi);
void wlc_phy_perratedpdset(wlc_phy_t *ppi, bool enable);
#endif // endif

/* XXXX FIXME: The below macro PHY_TXFIFO_END_BLK_REV35 is a mac
* specific value ( equal to xmtfifo_sz )
* shouldn't be really defined here - Refer to PR 103560
*/
#define PHY_TXFIFO_END_BLK_REV35	(0x7900 >> 2)

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  function implementation                                     */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* returns a pointer to per interface instance data */
shared_phy_t *
BCMATTACHFN(wlc_phy_shared_attach)(shared_phy_params_t *shp)
{
	shared_phy_t *sh;
	int ref_count = 0;

#ifdef EVENT_LOG_COMPILE
	/* First thing to do.. initialize the PHY_ERROR tag's attributes. */
	/* This is the attach function for the PHY component. */
	event_log_tag_start(EVENT_LOG_TAG_PHY_ERROR, EVENT_LOG_SET_ERROR,
		EVENT_LOG_TAG_FLAG_LOG|EVENT_LOG_TAG_FLAG_PRINT);
#endif // endif

	/* allocate wlc_info_t state structure */
	if ((sh = (shared_phy_t*) MALLOC(shp->osh, sizeof(shared_phy_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			shp->unit, __FUNCTION__, MALLOCED(shp->osh)));
		goto fail;
	}
	bzero((char*)sh, sizeof(shared_phy_t));

	/* OBJECT REGISTRY: check if shared key has value already stored */
	sh->sromi = (phy_srom_info_t *)wlapi_obj_registry_get(shp->physhim, OBJR_PHY_CMN_SROM_INFO);
	if (sh->sromi == NULL) {
		if ((sh->sromi = (phy_srom_info_t *)MALLOC(shp->osh,
			sizeof(phy_srom_info_t))) == NULL) {

			PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				shp->unit, __FUNCTION__, MALLOCED(shp->osh)));
			goto fail;
		}
		bzero(sh->sromi, sizeof(phy_srom_info_t));

		/* OBJECT REGISTRY: We are the first instance, store value for key */
		wlapi_obj_registry_set(shp->physhim, OBJR_PHY_CMN_SROM_INFO, sh->sromi);
	}
	/* OBJECT REGISTRY: Reference the stored value in both instances */
	ref_count = wlapi_obj_registry_ref(shp->physhim, OBJR_PHY_CMN_SROM_INFO);
	ASSERT(ref_count <= MAX_RSDB_MAC_NUM);
	BCM_REFERENCE(ref_count);

	sh->osh		= shp->osh;
	sh->sih		= shp->sih;
	sh->physhim	= shp->physhim;
	sh->unit	= shp->unit;
	sh->corerev	= shp->corerev;
	sh->vid		= shp->vid;
	sh->did		= shp->did;
	sh->chip	= shp->chip;
	sh->chiprev	= shp->chiprev;
	sh->chippkg	= shp->chippkg;
	sh->sromrev	= shp->sromrev;
	sh->boardtype	= shp->boardtype;
	sh->boardrev	= shp->boardrev;
	sh->boardvendor	= shp->boardvendor;
	sh->boardflags	= shp->boardflags;
	sh->boardflags2	= shp->boardflags2;
	sh->boardflags4	= shp->boardflags4;
	sh->bustype	= shp->bustype;
	sh->buscorerev	= shp->buscorerev;

	/* create our timers */
	sh->fast_timer	= PHY_SW_TIMER_FAST;
	sh->slow_timer	= PHY_SW_TIMER_SLOW;
	sh->glacial_timer = PHY_SW_TIMER_GLACIAL;

	/* reset cal scheduler */
	sh->scheduled_cal_time = 0;

	/* ACI mitigation mode is auto by default */
	sh->interference_mode = WLAN_AUTO;
	/* sh->snr_mode = SNR_ANT_MERGE_MAX; */
	sh->rssi_mode = RSSI_ANT_MERGE_MAX;

	return sh;
fail:
	wlc_phy_shared_detach(sh);
	return NULL;
}

void
BCMATTACHFN(wlc_phy_shared_detach)(shared_phy_t *sh)
{
	if (sh != NULL) {
		/* phy_head must have been all detached */
		if (sh->phy_head) {
			PHY_ERROR(("wl%d: %s non NULL phy_head\n", sh->unit, __FUNCTION__));
			ASSERT(!sh->phy_head);
		}
		if (sh->sromi != NULL) {
			if (wlapi_obj_registry_unref(sh->physhim, OBJR_PHY_CMN_SROM_INFO) == 0) {
				wlapi_obj_registry_set(sh->physhim, OBJR_PHY_CMN_SROM_INFO, NULL);
				MFREE(sh->osh, sh->sromi, sizeof(phy_srom_info_t));
			}
		}
		MFREE(sh->osh, sh, sizeof(shared_phy_t));
	}
}

static const char BCMATTACHDATA(rstr_interference)[] = "interference";
static const char BCMATTACHDATA(rstr_txpwrbckof)[] = "txpwrbckof";
static const char BCMATTACHDATA(rstr_tssilimucod)[] = "tssilimucod";
static const char BCMATTACHDATA(rstr_rssicorrnorm)[] = "rssicorrnorm";
static const char BCMATTACHDATA(rstr_rssicorratten)[] = "rssicorratten";
static const char BCMATTACHDATA(rstr_rssicorrnorm5g)[] = "rssicorrnorm5g";
static const char BCMATTACHDATA(rstr_rssicorratten5g)[] = "rssicorratten5g";
static const char BCMATTACHDATA(rstr_rssicorrperrg2g)[] = "rssicorrperrg2g";
static const char BCMATTACHDATA(rstr_rssicorrperrg5g)[] = "rssicorrperrg5g";
static const char BCMATTACHDATA(rstr_5g_cga)[] = "5g_cga";
static const char BCMATTACHDATA(rstr_2g_cga)[] = "2g_cga";
static const char BCMATTACHDATA(rstr_phycal)[] = "phycal";
static const char BCMATTACHDATA(rstr_phycal_tempdelta)[] = "phycal_tempdelta";
static const char BCMATTACHDATA(rstr_mintxpower)[] = "mintxpower";
#if defined(RXDESENS_EN)
static const char BCMATTACHDATA(rstr_phyrxdesens)[] = "phyrxdesens";
#endif // endif

/*
 * Read the phy calibration temperature delta parameters from NVRAM.
 */
void
BCMATTACHFN(wlc_phy_read_tempdelta_settings)(phy_info_t *pi, int maxtempdelta)
{
	/* Read the temperature delta from NVRAM */
	pi->phycal_tempdelta = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_phycal_tempdelta, 0);

	/* if nvram entry is bogus */
	if (pi->phycal_tempdelta == 0) {
	    if (ISACPHY(pi))
		pi->phycal_tempdelta_default = ACPHY_DEFAULT_CAL_TEMPDELTA;
	    else
		pi->phycal_tempdelta_default = pi->phycal_tempdelta;
	}

	/* Range check, disable if incorrect configuration parameter */
	/* Preserve default, in case someone wants to use it. */
	if (pi->phycal_tempdelta > maxtempdelta)
		pi->phycal_tempdelta = pi->phycal_tempdelta_default;
	else
		pi->phycal_tempdelta_default = pi->phycal_tempdelta;
}

/* Break a lengthy algorithm into smaller pieces using 0-length timer */
void
wlc_phy_timercb_phycal(phy_info_t *pi)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint delay_val = pi->phy_cal_delay;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = NULL;
	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif

	/* Increase delay between phases to be longer than 2 video frames interval 16.7*2 */
#if !defined(PHYCAL_SPLIT_4324x) && !defined(WFD_PHY_LL)
	if (CHIPID(pi->sh->chip) == BCM43237_CHIP_ID)
#endif // endif
		delay_val = 40;

	if (PHY_PERICAL_MPHASE_PENDING(pi)) {

		PHY_CAL(("wlc_phy_timercb_phycal: phase_id %d\n", pi->cal_info->cal_phase_id));

		/* XXX phy_init can be called after "wl out"(due to bandwidth switch.
		 * but scheduling calibration seems inappropriate. test cals after "wl out"
		 * can invoke the wlc_phy_cal_perical_nphy_run() directly if desired.
		 */
		if (!pi->sh->up) {
			wlc_phy_cal_perical_mphase_reset(pi);
			return;
		}

		if (SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || PHY_MUTED(pi)) {
			/* delay percal until scan completed */
			PHY_CAL(("wlc_phy_timercb_phycal: scan in progress, delay 1 sec\n"));
			delay_val = 1000;	/* delay 1 sec */
			/* PHYCAL_CACHING does not interact with mphase */
#if defined(PHYCAL_CACHING)
			if (!ctx)
#endif // endif
				wlc_phy_cal_perical_mphase_restart(pi);
		} else {
			if (ISNPHY(pi)) {
				wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_AUTO);
			} else if (ISHTPHY(pi)) {
				/* pick up the search type from what the scheduler requested
				 * (INITIAL or INCREMENTAL) and call the calibration
				 */
				wlc_phy_cals_htphy(pi, pi->cal_info->cal_searchmode);
			} else if (ISACPHY(pi)) {
				wlc_phy_cals_acphy(pi, pi->cal_info->cal_searchmode);
			} else {
				ASSERT(0); /* other phys not expected here */
			}
		}

		if (ISNPHY(pi)) {
			if (!(pi_nphy->ntd_papdcal_dcs == TRUE &&
				pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_RXCAL))
				wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, delay_val, 0);
			else {
				pi_nphy->ntd_papdcal_dcs = FALSE;
				wlc_phy_cal_perical_mphase_reset(pi);
			}
		} else {
			if (!pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_IDLE) {
				wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, delay_val, 0);
			} else {
				wlc_phy_cal_perical_mphase_reset(pi);
			}
		}
		return;
	}

	PHY_CAL(("wlc_phy_timercb_phycal: mphase phycal is done\n"));
}

void
WLBANDINITFN(wlc_phy_por_inform)(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->phy_init_por = TRUE;
}

void
wlc_phy_btclock_war(wlc_phy_t *ppi, bool enable)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->btclock_tune = enable;
}

void
wlc_phy_preamble_override_set(wlc_phy_t *ppi, int8 val)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if ((ISACPHY(pi)) && (val != WLC_N_PREAMBLE_MIXEDMODE)) {
		PHY_ERROR(("wl%d:%s: AC Phy: Ignore request to set preamble mode %d\n",
			pi->sh->unit, __FUNCTION__, val));
		return;
	}

	pi->n_preamble_override = val;
}

int8
wlc_phy_preamble_override_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->n_preamble_override;
}

/* increase the threshold to avoid false edcrs detection, non-11n only */
void
wlc_phy_edcrs_lock(wlc_phy_t *pih, bool lock)
{
	phy_info_t *pi = (phy_info_t *)pih;
	pi->edcrs_threshold_lock = lock;

	/* assertion: -59dB, deassertion: -67dB */
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh0, 0x46b)
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh1, 0x46b)
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh0, 0x3c0)
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh1, 0x3c0)
	PHY_REG_LIST_EXECUTE(pi);
}

void
wlc_phy_initcal_enable(wlc_phy_t *pih, bool initcal)
{
	phy_info_t *pi = (phy_info_t *)pih;
	if (ISNPHY(pi))
		pi->u.pi_nphy->do_initcal = initcal;
}

void
wlc_phy_hw_clk_state_upd(wlc_phy_t *pih, bool newstate)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!pi || !pi->sh)
		return;

	pi->sh->clk = newstate;
}

void
wlc_phy_hw_state_upd(wlc_phy_t *pih, bool newstate)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!pi || !pi->sh)
		return;

	pi->sh->up = newstate;
}

#ifdef WFD_PHY_LL
void
wlc_phy_wfdll_chan_active(wlc_phy_t * ppi, bool chan_active)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISNPHY(pi) || ISACPHY(pi))
		pi->wfd_ll_chan_active = chan_active;
}
#endif /* WFD_PHY_LL */

/*
 * Do one-time phy initializations and calibration.
 *
 * Note: no register accesses allowed; we have not yet waited for PLL
 * since the last corereset.
 */
void
BCMINITFN(wlc_phy_cal_init)(wlc_phy_t *pih)
{
	int i;
	phy_info_t *pi = (phy_info_t *)pih;
	initfn_t cal_init = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

#ifndef BCM_OL_DEV
	ASSERT((R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC) == 0);
#endif /* BCM_OL_DEV */

	if (!pi->initialized) {
		/* glitch counter init */
		/* detection is called only if high glitches are observed */
		pi->interf->aci.glitch_ma = ACI_INIT_MA;
		pi->interf->aci.glitch_ma_previous = ACI_INIT_MA;
		pi->interf->aci.pre_glitch_cnt = 0;
		if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3) &&
		     NREV_LE(pi->pubpi.phy_rev, 15)) || ISHTPHY(pi) || ISACPHY(pi)) {
			pi->interf->aci.ma_total = PHY_NOISE_MA_WINDOW_SZ * ACI_INIT_MA;
		} else {
			pi->interf->aci.ma_total = MA_WINDOW_SZ * ACI_INIT_MA;
		}
		for (i = 0; i < MA_WINDOW_SZ; i++)
			pi->interf->aci.ma_list[i] = ACI_INIT_MA;

		for (i = 0; i < PHY_NOISE_MA_WINDOW_SZ; i++) {
			pi->interf->noise.ofdm_glitch_ma_list[i] = PHY_NOISE_GLITCH_INIT_MA;
			pi->interf->noise.ofdm_badplcp_ma_list[i] =
				PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
			pi->interf->noise.bphy_glitch_ma_list[i] = PHY_NOISE_GLITCH_INIT_MA;
			pi->interf->noise.bphy_badplcp_ma_list[i] =
				PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		}

		pi->interf->noise.ofdm_glitch_ma = PHY_NOISE_GLITCH_INIT_MA;
		pi->interf->noise.bphy_glitch_ma = PHY_NOISE_GLITCH_INIT_MA;
		pi->interf->noise.ofdm_glitch_ma_previous = PHY_NOISE_GLITCH_INIT_MA;
		pi->interf->noise.bphy_glitch_ma_previous = PHY_NOISE_GLITCH_INIT_MA;
		pi->interf->noise.bphy_pre_glitch_cnt = 0;
		pi->interf->noise.ofdm_ma_total = PHY_NOISE_GLITCH_INIT_MA * PHY_NOISE_MA_WINDOW_SZ;
		pi->interf->noise.bphy_ma_total = PHY_NOISE_GLITCH_INIT_MA * PHY_NOISE_MA_WINDOW_SZ;

		pi->interf->badplcp_ma = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf->badplcp_ma_previous = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3) &&
		     NREV_LE(pi->pubpi.phy_rev, 15)) || ISHTPHY(pi) || ISACPHY(pi)) {
			pi->interf->badplcp_ma_total = PHY_NOISE_GLITCH_INIT_MA_BADPlCP *
			        PHY_NOISE_MA_WINDOW_SZ;
		} else {
			pi->interf->badplcp_ma_total = PHY_NOISE_GLITCH_INIT_MA_BADPlCP *
			        MA_WINDOW_SZ;
		}
		pi->interf->pre_badplcp_cnt = 0;
		pi->interf->bphy_pre_badplcp_cnt = 0;

		pi->interf->noise.ofdm_badplcp_ma = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf->noise.bphy_badplcp_ma = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;

		pi->interf->noise.ofdm_badplcp_ma_previous = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf->noise.bphy_badplcp_ma_previous = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf->noise.ofdm_badplcp_ma_total =
			PHY_NOISE_GLITCH_INIT_MA_BADPlCP * PHY_NOISE_MA_WINDOW_SZ;
		pi->interf->noise.bphy_badplcp_ma_total =
			PHY_NOISE_GLITCH_INIT_MA_BADPlCP * PHY_NOISE_MA_WINDOW_SZ;

		pi->interf->noise.ofdm_badplcp_ma_index = 0;
		pi->interf->noise.bphy_badplcp_ma_index = 0;

		pi->interf->cca_stats_func_called = FALSE;
		pi->interf->cca_stats_total_glitch = 0;
		pi->interf->cca_stats_bphy_glitch = 0;
		pi->interf->cca_stats_total_badplcp = 0;
		pi->interf->cca_stats_bphy_badplcp = 0;
		pi->interf->cca_stats_mbsstime = 0;

		cal_init = pi->pi_fptr.calinit;
		if (cal_init)
			(*cal_init)(pi);

		/* XXX: It would be cleaner to actually call the
		 * initializations here rather than as side-effects
		 * of the phy_init's, but each one needs to be
		 * reviewed for requirements.
		 */
		pi->initialized = TRUE;
	}
}

#ifndef WLC_DISABLE_ACI
#if defined(WLTEST) || defined(WL_PHYACIARGS)
static int
wlc_phy_aci_args(phy_info_t *pi, wl_aci_args_t *params, bool get, int len)
{
	bool nphy_aciarg;

	if (len == WL_ACI_ARGS_LEGACY_LENGTH)
		nphy_aciarg = FALSE;
	else if (len == sizeof(wl_aci_args_t))
		nphy_aciarg = TRUE;
	else
		return BCME_BUFTOOSHORT;

	if (get) {
		params->enter_aci_thresh = pi->interf->aci.enter_thresh;
		params->exit_aci_thresh = pi->interf->aci.exit_thresh;
		params->usec_spin = pi->interf->aci.usec_spintime;
		params->glitch_delay = pi->interf->aci.glitch_delay;
	} else {
		if (params->enter_aci_thresh > 0)
			pi->interf->aci.enter_thresh = params->enter_aci_thresh;
		if (params->exit_aci_thresh > 0)
			pi->interf->aci.exit_thresh = params->exit_aci_thresh;
		if (params->usec_spin > 0)
			pi->interf->aci.usec_spintime = params->usec_spin;
		if (params->glitch_delay > 0)
			pi->interf->aci.glitch_delay = params->glitch_delay;
	}

	if (nphy_aciarg) {
		if (get) {
			params->nphy_adcpwr_enter_thresh = pi->interf->aci.nphy.adcpwr_enter_thresh;
			params->nphy_adcpwr_exit_thresh = pi->interf->aci.nphy.adcpwr_exit_thresh;
			params->nphy_repeat_ctr = pi->interf->aci.nphy.detect_repeat_ctr;
			params->nphy_num_samples = pi->interf->aci.nphy.detect_num_samples;
			params->nphy_undetect_window_sz = pi->interf->aci.nphy.undetect_window_sz;
			params->nphy_b_energy_lo_aci = pi->interf->aci.nphy.b_energy_lo_aci;
			params->nphy_b_energy_md_aci = pi->interf->aci.nphy.b_energy_md_aci;
			params->nphy_b_energy_hi_aci = pi->interf->aci.nphy.b_energy_hi_aci;

			params->nphy_noise_noassoc_glitch_th_up =
				pi->interf->noise.nphy_noise_noassoc_glitch_th_up;
			params->nphy_noise_noassoc_glitch_th_dn =
				pi->interf->noise.nphy_noise_noassoc_glitch_th_dn;
			params->nphy_noise_assoc_glitch_th_up =
				pi->interf->noise.nphy_noise_assoc_glitch_th_up;
			params->nphy_noise_assoc_glitch_th_dn =
				pi->interf->noise.nphy_noise_assoc_glitch_th_dn;
			params->nphy_noise_assoc_aci_glitch_th_up =
				pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up;
			params->nphy_noise_assoc_aci_glitch_th_dn =
				pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn;
			params->nphy_noise_assoc_enter_th =
				pi->interf->noise.nphy_noise_assoc_enter_th;
			params->nphy_noise_noassoc_enter_th =
				pi->interf->noise.nphy_noise_noassoc_enter_th;
			params->nphy_noise_assoc_rx_glitch_badplcp_enter_th=
				pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th;
			params->nphy_noise_noassoc_crsidx_incr=
				pi->interf->noise.nphy_noise_noassoc_crsidx_incr;
			params->nphy_noise_assoc_crsidx_incr=
				pi->interf->noise.nphy_noise_assoc_crsidx_incr;
			params->nphy_noise_crsidx_decr=
				pi->interf->noise.nphy_noise_crsidx_decr;

		} else {
			pi->interf->aci.nphy.adcpwr_enter_thresh = params->nphy_adcpwr_enter_thresh;
			pi->interf->aci.nphy.adcpwr_exit_thresh = params->nphy_adcpwr_exit_thresh;
			pi->interf->aci.nphy.detect_repeat_ctr = params->nphy_repeat_ctr;
			pi->interf->aci.nphy.detect_num_samples = params->nphy_num_samples;
			pi->interf->aci.nphy.undetect_window_sz =
				MIN(params->nphy_undetect_window_sz,
				ACI_MAX_UNDETECT_WINDOW_SZ);
			pi->interf->aci.nphy.b_energy_lo_aci = params->nphy_b_energy_lo_aci;
			pi->interf->aci.nphy.b_energy_md_aci = params->nphy_b_energy_md_aci;
			pi->interf->aci.nphy.b_energy_hi_aci = params->nphy_b_energy_hi_aci;

			pi->interf->noise.nphy_noise_noassoc_glitch_th_up =
				params->nphy_noise_noassoc_glitch_th_up;
			pi->interf->noise.nphy_noise_noassoc_glitch_th_dn =
				params->nphy_noise_noassoc_glitch_th_dn;
			pi->interf->noise.nphy_noise_assoc_glitch_th_up =
				params->nphy_noise_assoc_glitch_th_up;
			pi->interf->noise.nphy_noise_assoc_glitch_th_dn =
				params->nphy_noise_assoc_glitch_th_dn;
			pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up =
				params->nphy_noise_assoc_aci_glitch_th_up;
			pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn =
				params->nphy_noise_assoc_aci_glitch_th_dn;
			pi->interf->noise.nphy_noise_assoc_enter_th =
				params->nphy_noise_assoc_enter_th;
			pi->interf->noise.nphy_noise_noassoc_enter_th =
				params->nphy_noise_noassoc_enter_th;
			pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th =
				params->nphy_noise_assoc_rx_glitch_badplcp_enter_th;
			pi->interf->noise.nphy_noise_noassoc_crsidx_incr =
				params->nphy_noise_noassoc_crsidx_incr;
			pi->interf->noise.nphy_noise_assoc_crsidx_incr =
				params->nphy_noise_assoc_crsidx_incr;
			pi->interf->noise.nphy_noise_crsidx_decr =
				params->nphy_noise_crsidx_decr;
		}
	}

	return BCME_OK;
}
#endif // endif
#endif /* Compiling out ACI code for 4324 */

static void wlc_phy_table_lock_lcnphy(phy_info_t *pi);
static void wlc_phy_table_unlock_lcnphy(phy_info_t *pi);

void
wlc_phy_table_read_lcnphy(phy_info_t *pi, const lcnphytbl_info_t *ptbl_info)
{
	uint    idx;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint32  u32temp;

	uint8  *ptbl_8b    = (uint8  *)(uintptr)ptbl_info->tbl_ptr;
	uint16 *ptbl_16b   = (uint16 *)(uintptr)ptbl_info->tbl_ptr;
	uint32 *ptbl_32b   = (uint32 *)(uintptr)ptbl_info->tbl_ptr;

	uint16 tblAddr = LCNPHY_TableAddress;
	uint16 tblDataHi = LCNPHY_TabledataHi;
	uint16 tblDatalo = LCNPHY_TabledataLo;

	ASSERT((ptbl_info->tbl_phywidth == 8) || (ptbl_info->tbl_phywidth == 16) ||
		(ptbl_info->tbl_phywidth == 32));
	ASSERT((ptbl_info->tbl_width == 8) || (ptbl_info->tbl_width == 16) ||
		(ptbl_info->tbl_width == 32));

	wlc_phy_table_lock_lcnphy(pi);

	phy_utils_write_phyreg(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {

		/* get the element from phy according to the phy table element
		 * address space width
		 */
		if (ptbl_info->tbl_phywidth == 32) {
			/* phy width is 32-bit */
			u32temp  =  phy_utils_read_phyreg(pi, tblDatalo);
			u32temp |= (phy_utils_read_phyreg(pi, tblDataHi) << 16);
		} else if (ptbl_info->tbl_phywidth == 16) {
			/* phy width is 16-bit */
			u32temp  =  phy_utils_read_phyreg(pi, tblDatalo);
		} else {
			/* phy width is 8-bit */
			u32temp   =  (uint8)phy_utils_read_phyreg(pi, tblDatalo);
		}

		/* put the element into the table according to the table element width
		 * Note that phy table width is some times more than necessary while
		 * table width is always optimal.
		 */
		if (ptbl_info->tbl_width == 32) {
			/* tbl width is 32-bit */
			ptbl_32b[idx]  =  u32temp;
		} else if (ptbl_info->tbl_width == 16) {
			/* tbl width is 16-bit */
			ptbl_16b[idx]  =  (uint16)u32temp;
		} else {
			/* tbl width is 8-bit */
			ptbl_8b[idx]   =  (uint8)u32temp;
		}
	}
	wlc_phy_table_unlock_lcnphy(pi);
}

/* prevent simultaneous phy table access by driver and ucode */
static void
wlc_phy_table_lock_lcnphy(phy_info_t *pi)
{
	uint32 mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);

	if (mc & MCTL_EN_MAC) {
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
		(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
		OSL_DELAY(5);
	}
}

static void
wlc_phy_table_unlock_lcnphy(phy_info_t *pi)
{
	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);
}

uint
wlc_phy_init_radio_regs_allbands(phy_info_t *pi, radio_20xx_regs_t *radioregs)
{
	uint i;

	for (i = 0; radioregs[i].address != 0xffff; i++) {
		if (radioregs[i].do_init) {
			phy_utils_write_radioreg(pi, radioregs[i].address,
			                (uint16)radioregs[i].init);
		}
	}

	return i;
}
uint
wlc_phy_init_radio_regs_allbands_20671(phy_info_t *pi, radio_20671_regs_t *radioregs)
{
	uint i;

	for (i = 0; radioregs[i].address != 0xffff; i++) {
		if (radioregs[i].do_init) {
			phy_utils_write_radioreg(pi, radioregs[i].address,
			                (uint16)radioregs[i].init);
		}
	}

	return i;
}
uint
wlc_phy_init_radio_prefregs_allbands(phy_info_t *pi, radio_20xx_prefregs_t *radioregs)
{
	uint i;

	for (i = 0; radioregs[i].address != 0xffff; i++) {
		phy_utils_write_radioreg(pi, radioregs[i].address,
		                (uint16)radioregs[i].init);

	}

	return i;
}

uint
wlc_phy_init_radio_regs(phy_info_t *pi, radio_regs_t *radioregs, uint16 core_offset)
{
	uint i;
	uint count = 0;

	for (i = 0; radioregs[i].address != 0xffff; i++) {
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (radioregs[i].do_init_a) {
				phy_utils_write_radioreg(pi, radioregs[i].address | core_offset,
					(uint16)radioregs[i].init_a);
				if (ISNPHY(pi) && (++count % 4 == 0))
					WLC_PHY_WAR_PR51571(pi);
			}
		} else
#endif /* BAND5G */
		{
			if (radioregs[i].do_init_g) {
				phy_utils_write_radioreg(pi, radioregs[i].address | core_offset,
					(uint16)radioregs[i].init_g);
				if (ISNPHY(pi) && (++count % 4 == 0))
					WLC_PHY_WAR_PR51571(pi);
			}
		}
	}

	return i;
}

void
wlc_phy_do_dummy_tx(phy_info_t *pi, bool ofdm, bool pa_on)
{
#define	DUMMY_PKT_LEN	20 /* Dummy packet's length */
	d11regs_t *regs = pi->regs;
	int	i, count;
	uint8	ofdmpkt[DUMMY_PKT_LEN] = {
		0xcc, 0x01, 0x02, 0x00, 0x00, 0x00, 0xd4, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
	};
	uint8	cckpkt[DUMMY_PKT_LEN] = {
		0x6e, 0x84, 0x0b, 0x00, 0x00, 0x00, 0xd4, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
	};
	uint32 *dummypkt;

	ASSERT((R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC) == 0);

	dummypkt = (uint32 *)(ofdm ? ofdmpkt : cckpkt);
	wlapi_bmac_write_template_ram(pi->sh->physhim, 0, DUMMY_PKT_LEN, dummypkt);

	/* set up the TXE transfer */

	W_REG(pi->sh->osh, &regs->PHYREF_XMTSEL, 0);
	/* Assign the WEP to the transmit path */
	if (D11REV_GE(pi->sh->corerev, 11))
		W_REG(pi->sh->osh, &regs->PHYREF_WEPCTL, 0x100);
	else
		W_REG(pi->sh->osh, &regs->PHYREF_WEPCTL, 0);

	/* Set/clear OFDM bit in PHY control word */
	W_REG(pi->sh->osh, &regs->txe_phyctl, (ofdm ? 1 : 0) | PHY_TXC_ANT_0);
	if (ISNPHY(pi) || ISHTPHY(pi) || ISLCNCOMMONPHY(pi)) {
		ASSERT(ofdm);
		W_REG(pi->sh->osh, &regs->txe_phyctl1, 0x1A02);
	}

	W_REG(pi->sh->osh, &regs->txe_wm_0, 0);		/* No substitutions */
	W_REG(pi->sh->osh, &regs->txe_wm_1, 0);

	/* Set transmission from the TEMPLATE where we loaded the frame */
	if (D11REV_IS(pi->sh->corerev, 35)) {
		/* XXX PR 103560: For 4334Bx only
		 * XXX    Template region not
		 * XXX    preserved during save/restore
		 */
		W_REG(pi->sh->osh, &regs->PHYREF_XMTTPLATETXPTR, PHY_TXFIFO_END_BLK_REV35);
	} else
		W_REG(pi->sh->osh, &regs->PHYREF_XMTTPLATETXPTR, 0);
	W_REG(pi->sh->osh, &regs->PHYREF_XMTTXCNT, DUMMY_PKT_LEN);

	/* Set Template as source, length specified as a count and destination
	 * as Serializer also set "gen_eof"
	 */
	W_REG(pi->sh->osh, &regs->PHYREF_XMTSEL, ((8 << 8) | (1 << 5) | (1 << 2) | 2));

	/* Instruct the MAC to not calculate FCS, we'll supply a bogus one */
	W_REG(pi->sh->osh, &regs->txe_ctl, 0);

	if (!pa_on) {
		if (ISNPHY(pi))
			wlc_phy_pa_override_nphy(pi, OFF);
		else if (ISHTPHY(pi)) {
			wlc_phy_pa_override_htphy(pi, OFF);
		}
	}

	/* Start transmission and wait until sendframe goes away */
	/* Set TX_NOW in AUX along with MK_CTLWRD */
	if (ISNPHY(pi) || ISHTPHY(pi) || ISLCNCOMMONPHY(pi))
		W_REG(pi->sh->osh, &regs->txe_aux, 0xD0);
	else
		W_REG(pi->sh->osh, &regs->txe_aux, ((1 << 5) | (1 << 4)));

	(void)R_REG(pi->sh->osh, &regs->txe_aux);

	/* Wait for 10 x ack time, enlarge it for vsim of QT */
	i = 0;
	count = ofdm ? 30 : 250;

#ifndef BCMQT_CPU
	if (ISSIM_ENAB(pi->sh->sih)) {
		count *= 100;
	}
#endif // endif
	/* wait for txframe to be zero */
	while ((i++ < count) && (R_REG(pi->sh->osh, &regs->txe_status) & (1 << 7))) {
		OSL_DELAY(10);
	}
	if (i >= count)
		PHY_ERROR(("wl%d: %s: Waited %d uS for %s txframe\n",
		          pi->sh->unit, __FUNCTION__, 10 * i, (ofdm ? "ofdm" : "cck")));

	/* Wait for the mac to finish (this is 10x what is supposed to take) */
	i = 0;
	/* wait for txemend */
	while ((i++ < 10) && ((R_REG(pi->sh->osh, &regs->txe_status) & (1 << 10)) == 0)) {
		OSL_DELAY(10);
	}
	if (i >= 10)
		PHY_ERROR(("wl%d: %s: Waited %d uS for txemend\n",
		          pi->sh->unit, __FUNCTION__, 10 * i));

	/* Wait for the phy to finish */
	i = 0;
	/* wait for txcrs */
	while ((i++ < 500) && ((R_REG(pi->sh->osh, &regs->PHYREF_IFSSTAT) & (1 << 8)))) {
		OSL_DELAY(10);
	}
	if (i >= 500)
		PHY_ERROR(("wl%d: %s: Waited %d uS for txcrs\n",
		          pi->sh->unit, __FUNCTION__, 10 * i));
	if (!pa_on) {
		if (ISNPHY(pi))
			wlc_phy_pa_override_nphy(pi, ON);
		else if (ISHTPHY(pi)) {
			wlc_phy_pa_override_htphy(pi, ON);
		}
	}
}

static void
wlc_phy_scanroam_cache_cal(phy_info_t *pi, bool set)
{
	if (ISHTPHY(pi)) {
		wlc_phy_scanroam_cache_cal_htphy(pi, set);
	}
#if !defined(PHYCAL_CACHING)
	else if (ISACPHY(pi)) {
		wlc_phy_scanroam_cache_cal_acphy(pi, set);
	}
#endif /* !defined(PHYCAL_CACHING) */
}

void
wlc_phy_hold_upd(wlc_phy_t *pih, mbool id, bool set)
{
	phy_info_t *pi = (phy_info_t *)pih;
	ASSERT(id);

	PHY_TRACE(("%s: id %d val %d old pi->measure_hold 0%x\n", __FUNCTION__, id, set,
		pi->measure_hold));

	PHY_CAL(("wl%d: %s: %s %s flag\n", pi->sh->unit, __FUNCTION__,
		set ? "SET" : "CLR",
		(id == PHY_HOLD_FOR_ASSOC) ? "ASSOC" :
		((id == PHY_HOLD_FOR_SCAN) ? "SCAN" :
		((id == PHY_HOLD_FOR_SCAN) ? "SCAN" :
		((id == PHY_HOLD_FOR_RM) ? "RM" :
		((id == PHY_HOLD_FOR_PLT) ? "PLT" :
		((id == PHY_HOLD_FOR_MUTE) ? "MUTE" :
		((id == PHY_HOLD_FOR_PKT_ENG) ? "PKTENG" :
		((id == PHY_HOLD_FOR_TOF) ? "TOF" :
		((id == PHY_HOLD_FOR_NOT_ASSOC) ? "NOT-ASSOC" : ""))))))))));

	if (set) {
		mboolset(pi->measure_hold, id);
	} else {
		mboolclr(pi->measure_hold, id);
	}
	if (id & PHY_HOLD_FOR_SCAN) {
		if (ISACPHY(pi)) {
			/* If scanning dont cache values only cache during cals */
			if (!set)
				wlc_phy_scanroam_cache_cal(pi, set);
			/* Dont change behavior for other PHYs */
		} else {
			wlc_phy_scanroam_cache_cal(pi, set);
		}
	}
	return;
}

bool
wlc_phy_ismuted(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	return PHY_MUTED(pi);
}

void
wlc_phy_mute_upd(wlc_phy_t *pih, bool mute, mbool flags)
{
	phy_info_t *pi = (phy_info_t *)pih;

	PHY_TRACE(("wlc_phy_mute_upd: flags 0x%x mute %d\n", flags, mute));

	if (mute) {
		mboolset(pi->measure_hold, PHY_HOLD_FOR_MUTE);
	} else {
		mboolclr(pi->measure_hold, PHY_HOLD_FOR_MUTE);
	}

	/* check if need to schedule a phy cal */
	if (!mute && (flags & PHY_MUTE_FOR_PREISM)) {
		pi->cal_info->last_cal_time = (pi->sh->now > pi->sh->glacial_timer) ?
			(pi->sh->now - pi->sh->glacial_timer) : 0;
	}
	return;
}

void
wlc_phy_clear_tssi(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)) {
		/* NPHY/HTPHY doesn't use sw or ucode powercontrol */
		return;
	} else {
		wlapi_bmac_write_shm(pi->sh->physhim, M_B_TSSI_0, NULL_TSSI_W);
		wlapi_bmac_write_shm(pi->sh->physhim, M_B_TSSI_1, NULL_TSSI_W);
		wlapi_bmac_write_shm(pi->sh->physhim, M_G_TSSI_0, NULL_TSSI_W);
		wlapi_bmac_write_shm(pi->sh->physhim, M_G_TSSI_1, NULL_TSSI_W);
	}
}

static bool
wlc_phy_cal_txpower_recalc_sw(phy_info_t *pi)
{
	bool ret = TRUE;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* NPHY/HTPHY/ACPHY doesn't ever use SW power control */
	if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi))
		return FALSE;

	if (ISLCNCOMMONPHY(pi))
		return FALSE;

	/* No need to do anything if the hw is doing pwrctrl for us */
	if (pi->hwpwrctrl) {

		/* Do nothing if radio pwr is being overridden */
		if (pi->radiopwr_override != RADIOPWR_OVERRIDE_DEF)
			return (ret);

		pi->hwpwr_txcur = wlapi_bmac_read_shm(pi->sh->physhim, M_TXPWR_CUR);
		return (ret);
	}

	return (ret);
}

void
wlc_phy_chanspec_shm_set(phy_info_t *pi, chanspec_t chanspec)
{
	uint16 curchannel;

	/* Update ucode channel value */
	if (D11REV_LT(pi->sh->corerev, 40)) {
		/* d11 rev < 40: compose a channel info value */
		curchannel = CHSPEC_CHANNEL(chanspec);
#ifdef BAND5G
		if (CHSPEC_IS5G(chanspec))
			curchannel |= D11_CURCHANNEL_5G;
#endif /* BAND5G */
		if (CHSPEC_IS40(chanspec))
			curchannel |= D11_CURCHANNEL_40;
	} else {
		/* d11 rev >= 40: store the chanspec */
		curchannel = chanspec;
	}

	PHY_TRACE(("wl%d: %s: M_CURCHANNEL %x\n", pi->sh->unit, __FUNCTION__, curchannel));
	wlapi_bmac_write_shm(pi->sh->physhim, M_CURCHANNEL, curchannel);
}

void
wlc_phy_chanspec_radio_set(wlc_phy_t *ppi, chanspec_t newch)
{
	phy_info_t *pi = (phy_info_t*)ppi;
#ifdef PREASSOC_PWRCTRL
	/* XXX this block of code
	   should be called before radio channel
	   is changed to the new channel
	*/
	if ((pi->radio_chanspec != newch) && ISACPHY(pi) && pi->sh->up) {
		wlc_phy_store_tx_pwrctrl_setting(pi, pi->radio_chanspec);
		phy_ac_tpc_shortwindow_upd(pi, TRUE);
	} else {
		pi->channel_short_window = TRUE;
	}
#endif // endif

	pi->radio_chanspec = newch;
}

int32
wlc_phy_min_txpwr_limit_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return (int32)pi->min_txpower;
}

void
wlc_phy_set_filt_war(wlc_phy_t *ppi, bool filt_war)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISHTPHY(pi))
		wlc_phy_set_filt_war_htphy(pi, filt_war);
}

bool
wlc_phy_get_filt_war(wlc_phy_t *ppi)
{
	bool ret = FALSE;
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISHTPHY(pi))
		ret = wlc_phy_get_filt_war_htphy(pi);
	return ret;
}

#if defined(WLTXPWR_CACHE) && defined(WLC_LOW_ONLY)
/**
 * Reserves an entry for the caller supplied chanspec in the tx power (ppr) cache, if not already
 * reserved.
 */
static void
wlc_phy_pwr_cache_reserve(phy_info_t *pi, chanspec_t chanspec)
{
	static chanspec_t last_chanspec = 0;
	if (wlc_phy_txpwr_cache_is_cached(pi->txpwr_cache, chanspec) != TRUE) {
		int result;
		chanspec_t kill_chan = 0;

		if (last_chanspec != 0)
			kill_chan = wlc_phy_txpwr_cache_find_other_cached_chanspec(pi->txpwr_cache,
				last_chanspec);
		if (kill_chan != 0) {
			if (pi->tx_power_offset == wlc_phy_get_cached_pwr(pi->txpwr_cache,
				kill_chan, TXPWR_CACHE_POWER_OFFSETS)) {
				/* remove reference, as we're about to delete this */
				pi->tx_power_offset = NULL;
			}
			wlc_phy_txpwr_cache_clear(pi->sh->osh, pi->txpwr_cache, kill_chan);
		}
		result = wlc_phy_txpwr_setup_entry(pi->txpwr_cache, chanspec); /* in wlc_ppr.c */
		ASSERT(result == BCME_OK);
	}
	last_chanspec = chanspec;
}
#endif /* WLTXPWR_CACHE WLC_LOW_ONLY */

#if defined(BCMTSTAMPEDLOGS)
void
phy_log(phy_info_t *pi, const char* str, uint32 p1, uint32 p2)
{
	/* Read a timestamp from the TSF timer register */
	uint32 tstamp = R_REG(pi->sh->osh, &pi->regs->tsf_timerlow);

	/* Store the timestamp and the log message in the log buffer */
	bcmtslog(tstamp, str, p1, p2);
}
#endif // endif

void
wlc_phy_chanspec_set(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	chansetfn_t chanspec_set = NULL;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = NULL;
	ctx = wlc_phy_get_chanctx(pi, chanspec);
#endif // endif
	if (!SCAN_RM_IN_PROGRESS(pi) &&	(pi->home_chanspec != chanspec))
		pi->home_chanspec = chanspec;

	PHY_TRACE(("wl%d: %s: chanspec %x\n", pi->sh->unit, __FUNCTION__, chanspec));

	ASSERT(!wf_chspec_malformed(chanspec));

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);

#ifdef WLSRVSDB
	pi->srvsdb_state->prev_chanspec = chanspec;
#endif /* WLSRVSDB */

#if defined(WLTXPWR_CACHE) && defined(WLC_LOW_ONLY)
	/* For split MAC driver, for faster channel switching, reserve an entry for this channel */
	wlc_phy_pwr_cache_reserve(pi, chanspec);
#endif /* WLTXPWR_CACHE && WLC_LOW_ONLY */

	/* Update ucode channel value */
	wlc_phy_chanspec_shm_set(pi, chanspec);

	chanspec_set = pi->pi_fptr.chanset;

#if defined(AP) && defined(RADAR)
	/* indicate first time radar detection */
	if (pi->radari != NULL)
		phy_radar_first_indicator_set(pi->radari);
#endif // endif
	/* Update interference mode for ACPHY, as now init is not called on band/bw change */
	if ((!SCAN_RM_IN_PROGRESS(pi)) && (ISACPHY(pi) ||
		(ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 4)))) {
		if (pi->sh->interference_mode_override == TRUE) {
			pi->sh->interference_mode = CHSPEC_IS2G(chanspec) ?
			        pi->sh->interference_mode_2G_override :
			        pi->sh->interference_mode_5G_override;
		} else {
			pi->sh->interference_mode = CHSPEC_IS2G(chanspec) ?
			        pi->sh->interference_mode_2G :
			        pi->sh->interference_mode_5G;
		}
	}

#if defined(PHYCAL_CACHING)
	/* If a channel context exists, retrieve the multi-phase info from there, else use
	 * the default one
	 */
	/* A context has to be explicitly created */
	if (ctx)
		pi->cal_info = &ctx->cal_info;
	else
		pi->cal_info = pi->def_cal_info;
#endif // endif

	ASSERT(pi->cal_info);

#ifdef ENABLE_FCBS
	if (IS_FCBS(pi)) {
		if (chanspec_set) {
			int chanidx;
			fcbsinitprefn_t prefcbsinit;

			if (wlc_phy_is_fcbs_chan(pi, chanspec, &chanidx) &&
				!(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi))) {
				wlc_phy_fcbs((wlc_phy_t*)pi, chanidx, 1);
				/* Need to set this to indicate that we have switched to new chan
				 * not needed for SW-based FCBS ???
				 */
				pi->phy_fcbs->FCBS_INPROG = 1;
				wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);
			} else {
				/* The conditioning here matches that below for FCBS init. Hence, if
				 * there is a pending FCBS init on this channel and if HW_FCBS, then
				 * we want to setup the channel-indicator bit, etc. appropriately
				 * before we call phy_specific chanspec_set
				 */
				if (wlc_phy_is_fcbs_pending(pi, chanspec, &chanidx) &&
				!(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi))) {
					if ((prefcbsinit = pi->pi_fptr.prefcbsinit))
						(*prefcbsinit)(pi, chanidx);
				}

				(*chanspec_set)(pi, chanspec);

				/* Now that we are on new channel, check for a pending request */
				if (wlc_phy_is_fcbs_pending(pi, chanspec, &chanidx) &&
				!(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi))) {
					wlc_phy_fcbs_init((wlc_phy_t*)pi, chanidx);
				} else {
					chanidx = wlc_phy_channelindicator_obtain_acphy(pi);
					pi->phy_fcbs->initialized[chanidx] = FALSE;
				}
			}
		}
	} else {
		if (chanspec_set)
			(*chanspec_set)(pi, chanspec);
	}
#else
	if (chanspec_set)
		(*chanspec_set)(pi, chanspec);
#endif /* ENABLE_FCBS */
	if (ISACPHY(pi)) {
		wlc_phy_preempt(pi, ((pi->sh->interference_mode & ACPHY_ACI_PREEMPTION) != 0));
	}

#if defined(PHYCAL_CACHING)
	/* Switched the context so restart a pending MPHASE cal, else clear the state */
	if (ctx) {
		if (wlc_phy_cal_cache_restore(pi) == BCME_ERROR) {
			PHY_CAL(("%s cal cache restore on chanspec 0x%x Failed\n",
				__FUNCTION__, pi->radio_chanspec));
		}

		if (CHIPID(pi->sh->chip) != BCM43237_CHIP_ID) {
			/* Calibrate if now > last_cal_time + glacial */
			if (PHY_PERICAL_MPHASE_PENDING(pi)) {
				PHY_CAL(("%s: Restarting calibration for 0x%x phase %d\n",
					__FUNCTION__, chanspec, pi->cal_info->cal_phase_id));
				/* Delete any existing timer just in case */
				wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
				wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
			} else if ((pi->phy_cal_mode != PHY_PERICAL_DISABLE) &&
				(pi->phy_cal_mode != PHY_PERICAL_MANUAL) &&
				((pi->sh->now - pi->cal_info->last_cal_time) >=
				pi->sh->glacial_timer)) {
				wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_PERICAL_WATCHDOG);
			}
		} else {
			if (PHY_PERICAL_MPHASE_PENDING(pi))
				wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
		}
	}
#endif /* PHYCAL_CACHING */

	wlapi_update_bt_chanspec(pi->sh->physhim, chanspec,
		SCAN_INPROG_PHY(pi), RM_INPROG_PHY(pi));
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
}

/* don't use this directly. use wlc_get_band_range whenever possible */
extern int  wlc_phy_chanspec_freq2bandrange_lcnphy(uint);
int
wlc_phy_chanspec_freq2bandrange_lcnphy(uint freq)
{
	int range = -1;
	if (freq < 2500)
		range = WL_CHAN_FREQ_RANGE_2G;
#ifdef BAND5G
	else if (freq <= 5320)
		range = WL_CHAN_FREQ_RANGE_5GL;
	else if (freq <= 5700)
		range = WL_CHAN_FREQ_RANGE_5GM;
	else
		range = WL_CHAN_FREQ_RANGE_5GH;
#endif /* BAND5G */

	return range;
}

int
wlc_phy_chanspec_bandrange_get(phy_info_t *pi, chanspec_t chanspec)
{
	int range = -1;
	uint channel = CHSPEC_CHANNEL(chanspec);
	uint freq = phy_utils_channel2freq(channel);

	if (ISNPHY(pi)) {
		range = wlc_phy_get_chan_freq_range_nphy(pi, channel);
	} else if (ISHTPHY(pi)) {
		range = wlc_phy_get_chan_freq_range_htphy(pi, channel);
	} else if (ISACPHY(pi)) {
	  if (SROMREV(pi->sh->sromrev) < 12) {
	    range = wlc_phy_get_chan_freq_range_acphy(pi, chanspec);
	  } else {
	    range = wlc_phy_get_chan_freq_range_srom12_acphy(pi, chanspec);
	  }
	} else if (ISLCNCOMMONPHY(pi)) {
		range = wlc_phy_chanspec_freq2bandrange_lcnphy(freq);
	} else
		ASSERT(0);

	return range;
}

bool
wlc_phy_is_txbfcal(wlc_phy_t *ppi)
{

	uint8  subband_idx;
	uint8  chans[NUM_CHANS_IN_CHAN_BONDING];
	uint16 rpcal_val, rpcal_val1 = 0;
	bool   is_caled;
	phy_info_t *pi = (phy_info_t*)ppi;

	if (!ISACPHY(pi))
		return FALSE;

	if (ACMAJORREV_33(pi->pubpi.phy_rev) && PHY_AS_80P80(pi, pi->radio_chanspec)) {
		wlc_phy_get_chan_freq_range_80p80_acphy(pi, pi->radio_chanspec, chans);
		subband_idx = chans[0];
		PHY_INFORM(("wl%d: %s: FIXME for 80P80\n", pi->sh->unit, __FUNCTION__));
		// FIXME - core0/1: chans[0], core2/3 chans[1]
	} else {
		subband_idx = wlc_phy_get_chan_freq_range_acphy(pi, 0);
	}

	switch (subband_idx) {
	case WL_CHAN_FREQ_RANGE_2G:
		rpcal_val = pi->sromi->rpcal2g;
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal2gcore3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
		rpcal_val = pi->sromi->rpcal5gb0;
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb0core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
		rpcal_val = pi->sromi->rpcal5gb1;
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb1core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
		rpcal_val = pi->sromi->rpcal5gb2;
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb2core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		rpcal_val = pi->sromi->rpcal5gb3;
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb3core3;
		}
		break;
	default:
		PHY_ERROR(("wl%d: %s: Invalid chan_freq_range %d\n",
		           pi->sh->unit, __FUNCTION__, subband_idx));
		rpcal_val = pi->sromi->rpcal2g;
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal2gcore3;
		}
		break;
	}

	is_caled = (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) ?
	        !(rpcal_val == 0 && rpcal_val1 == 0) : (rpcal_val != 0);

	return is_caled;
}

bool
wlc_phy_is_smthen(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISACPHY(pi)) {
		return pi->u.pi_acphy->acphy_enable_smth;
	} else {
		return FALSE;
	}
}

void
wlc_phy_chanspec_ch14_widefilter_set(wlc_phy_t *ppi, bool wide_filter)
{

}

/* %%%%%% txpower */

/* user txpower limit: in qdbm units with override flag */
int
#ifdef WLTXPWR1_SIGNED
wlc_phy_txpower_get(wlc_phy_t *ppi, int8 *qdbm, bool *override)
#else
wlc_phy_txpower_get(wlc_phy_t *ppi, uint *qdbm, bool *override)
#endif // endif
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ASSERT(qdbm != NULL);

	*qdbm = pi->tx_user_target;

	if (pi->openlp_tx_power_on) {
	  if (pi->txpwrnegative)
		*qdbm = (-1 * pi->openlp_tx_power_min) | WL_TXPWR_NEG;
	  else
		*qdbm = pi->openlp_tx_power_min;
	}

	if (override != NULL)
		*override = pi->txpwroverride;
	return (0);
}

/* user txpower limit: in qdbm units with override flag */
int
#ifdef WLTXPWR1_SIGNED
wlc_phy_txpower_set(wlc_phy_t *ppi, int8 qdbm, bool override, ppr_t *reg_pwr)
#else
wlc_phy_txpower_set(wlc_phy_t *ppi, uint qdbm, bool override, ppr_t *reg_pwr)
#endif // endif
{
	uint8 tx_pwr_ctrl_state;
	phy_info_t *pi = (phy_info_t *)ppi;
	if (qdbm > 127)
		return 5;

	/* No way for user to set maxpower on individual rates yet.
	 * Same max power is used for all rates
	 */
#ifdef WLTXPWR1_SIGNED
	pi->tx_user_target = qdbm;
#else
	pi->tx_user_target = (uint8)qdbm;
#endif // endif

	/* Restrict external builds to 100% Tx Power */
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WL_EXPORT_TXPOWER)
	pi->txpwroverride = override;
	pi->txpwroverrideset = override;
#else
	pi->txpwroverride = FALSE;
#endif // endif

	if (pi->sh->up) {
		if (SCAN_INPROG_PHY(pi)) {
			PHY_TXPWR(("wl%d: Scan in progress, skipping txpower control\n",
				pi->sh->unit));
		} else {
			bool suspend;

			suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) &
			            MCTL_EN_MAC);

			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);

			if (ISACPHY(pi))
				tx_pwr_ctrl_state = pi->txpwrctrl;
			else
				tx_pwr_ctrl_state = pi->nphy_txpwrctrl;

			if (ISNPHY(pi)) {
				/* Switch off power control to save previuos index */
				wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);
			} else if (ISACPHY(pi)) {
				wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
			}

			wlc_phy_txpower_recalc_target(pi, reg_pwr, NULL);
			wlc_phy_cal_txpower_recalc_sw(pi);

			if (ISNPHY(pi)) {
				/* Restore power control back */
				wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
			} else if (ISACPHY(pi)) {
				wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);
			}

			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);
		}
	}
	return (0);
}

/* user txpower limit: in qdbm units with override flag */
int
wlc_phy_neg_txpower_set(wlc_phy_t *ppi, uint qdbm)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (pi->sh->up) {
		if (SCAN_INPROG_PHY(pi)) {
			PHY_TXPWR(("wl%d: Scan in progress, skipping txpower control\n",
				pi->sh->unit));
		} else {
			bool suspend;
			suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);

			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);

			pi->openlp_tx_power_min = -1*qdbm;
			pi->txpwrnegative = 1;
			pi->txpwroverride = 1;

			phy_tpc_recalc_tgt(pi->tpci);

			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);

		}
	}
	return (0);
}

/* get sromlimit per rate for given channel. Routine does not account for ant gain */
void
wlc_phy_txpower_sromlimit(wlc_phy_t *ppi, chanspec_t chanspec, uint8 *min_pwr,
    ppr_t *max_pwr, uint8 core)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint channel = CHSPEC_CHANNEL(chanspec);

	if (!pi)
		return;

	if (min_pwr)
		*min_pwr = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
	if (max_pwr) {
		if (ISLCNPHY(pi)) {
			wlc_phy_txpower_sromlimit_get_lcnphy(pi, channel, max_pwr, core);
		} else if (ISLCN40PHY(pi)) {
			wlc_phy_txpower_sromlimit_get_lcn40phy(pi, channel, max_pwr, core);
		} else if (ISACPHY(pi)) {
			if (SROMREV(pi->sh->sromrev) < 12) {
			  wlc_phy_txpower_sromlimit_get_acphy(pi, chanspec, max_pwr, core);
			} else {
			  wlc_phy_txpower_sromlimit_get_srom12_acphy(pi, chanspec, max_pwr, core);
			}
		} else  if (ISHTPHY(pi)) {
			wlc_phy_txpower_sromlimit_get_htphy(pi, chanspec, max_pwr, core);
		} else if (ISNPHY(pi)) {
			wlc_phy_txpower_sromlimit_get_nphy(pi, channel, max_pwr, core);
		} else {
			ppr_set_cmn_val(max_pwr, (int8)WLC_TXPWR_MAX);
		}
	}
}

uint8
wlc_phy_txpower_get_target_min(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	uint8 core;
	uint8 tx_pwr_min = WLC_TXPWR_MAX;

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		tx_pwr_min = MIN(tx_pwr_min, pi->tx_power_min_per_core[core]);
	}

	return tx_pwr_min;
}

uint8
wlc_phy_txpower_get_target_max(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	uint8 core;
	uint8 tx_pwr_max = 0;

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		tx_pwr_max = MAX(tx_pwr_max, pi->tx_power_max_per_core[core]);
	}

	return tx_pwr_max;
}

static int8
wlc_phy_channel_gain_adjust(phy_info_t *pi)
{
	int8 pwr_correction = 0;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		pwr_correction = pi->phy_cga_2g[channel-1];
	}
#ifdef BAND5G
	else {
		uint freq = phy_utils_channel2freq(channel);
		uint8 i;
		uint16 chan_info_phy_cga_5g[24] = {
			5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320,
			5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640,
			5660, 5680, 5700, 5745, 5765, 5785, 5805, 5825,
		};

		for (i = 0; i < ARRAYSIZE(chan_info_phy_cga_5g); i++)
			if (freq <= chan_info_phy_cga_5g[i]) {
				pwr_correction = pi->phy_cga_5g[i];
				break;
			}
	}
#endif /* BAND5G */
	return pwr_correction;
}

#ifdef WLTXPWR_CACHE
/* Retrieve the cached ppr targets and pass them to the hardware function. */
static void
wlc_phy_txpower_retrieve_cached_target(phy_info_t *pi)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		pi->tx_power_max_per_core[core] =
			wlc_phy_get_cached_pwr_max(pi->txpwr_cache, pi->radio_chanspec, core);
		pi->tx_power_min_per_core[core] =
			wlc_phy_get_cached_pwr_min(pi->txpwr_cache, pi->radio_chanspec, core);
		pi->openlp_tx_power_min = pi->tx_power_min_per_core[core];
	}
	pi->txpwrnegative = 0;

#ifdef WL_SARLIMIT
	wlc_phy_sar_limit_set((wlc_phy_t*)pi,
		wlc_phy_get_cached_sar_lims(pi->txpwr_cache, pi->radio_chanspec));
#endif // endif

	phy_tpc_recalc_tgt(pi->tpci);
}
#endif /* WLTXPWR_CACHE */

/* Recalc target power all phys.  This internal/static function needs to be called whenever
 * the chanspec or TX power values (user target, regulatory limits or SROM/HW limits) change.
 * This happens thorough calls to the PHY public API.
 */
static void
wlc_phy_txpower_recalc_target(phy_info_t *pi, ppr_t *txpwr_reg, ppr_t *txpwr_targets)
{
	int8 tx_pwr_max = 0;
	int8 tx_pwr_min = 255;
	chanspec_t chspec;
	uint8 mintxpwr = 0;

	ppr_t *srom_max_txpwr;
	ppr_t *tx_pwr_target;

	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
	int8 cga;
	uint8 core;
#ifdef WL_MU_TX
	uint8 bwtype;
	int8 floor_pwr = 127;
	ppr_vht_mcs_rateset_t srom_bl_pwr;
#endif /* WL_MU_TX */
#if (ACCONF || ACCONF2) && defined(WLC_HIGH) && (defined(WLTEST) || defined(WLPKTENG))
	int8 openloop_pwrctrl_delta;
	bool mac_enabled = FALSE;
#endif // endif
	bool band5g_4319_flag = ((BOARDTYPE(pi->sh->boardtype) == BCM94319SDELNA6L_SSID) ||
	    (BOARDTYPE(pi->sh->boardtype) == BCM94319SDNA_SSID)) &&
		CHSPEC_IS5G(pi->radio_chanspec);

	ppr_t *reg_txpwr_limit;

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
#ifdef WLTXPWR_CACHE
	if ((pi->tx_power_offset != NULL) && (!wlc_phy_is_pwr_cached(pi->txpwr_cache,
		TXPWR_CACHE_POWER_OFFSETS, pi->tx_power_offset))) {
			ppr_delete(pi->sh->osh, pi->tx_power_offset);
	}

	if ((!pi->txpwroverride) && ((pi->tx_power_offset = wlc_phy_get_cached_pwr(pi->txpwr_cache,
		pi->radio_chanspec, TXPWR_CACHE_POWER_OFFSETS)) != NULL) &&
		(txpwr_targets == NULL)) {
		wlc_phy_txpower_retrieve_cached_target(pi);
		PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
		return;
	}
	pi->tx_power_offset = NULL;
#endif /* WLTXPWR_CACHE */

	if ((reg_txpwr_limit = ppr_create(pi->sh->osh,
		PPR_CHSPEC_BW(pi->radio_chanspec))) == NULL) {
		PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
		return;
	}

	if (txpwr_reg != NULL)
		wlc_phy_txpower_reg_limit_calc(pi, txpwr_reg, pi->radio_chanspec, reg_txpwr_limit);

	chspec = pi->radio_chanspec;

	if ((srom_max_txpwr = ppr_create(pi->sh->osh, PPR_CHSPEC_BW(chspec))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		     __FUNCTION__, MALLOCED(pi->sh->osh)));
		ppr_delete(pi->sh->osh, reg_txpwr_limit);
		PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
		return;
	}

	/* Combine regulatory limit, SROM/HW/board limit for each rate.  */

	FOREACH_CORE(pi, core) {
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WL_EXPORT_TXPOWER)
		/* Only allow tx power override for internal or test builds. */
		if (!pi->txpwroverride)
#endif // endif
		{
			/* Get board/hw limit */
			wlc_phy_txpower_sromlimit((wlc_phy_t *)pi, chspec,
				&mintxpwr, srom_max_txpwr, core);

#ifdef WL_MU_TX
			if (CHSPEC_IS20(chspec)) {
				bwtype = WL_TX_BW_20;
			} else if (CHSPEC_IS40(chspec)) {
				bwtype = WL_TX_BW_40;
			} else if (CHSPEC_IS80(chspec)) {
				bwtype = WL_TX_BW_80;
			} else {
				bwtype = 0xff;
			}

			/* get the txpwr corresponds to c9s1 board limit as evm floor txpwr */
			ppr_get_vht_mcs(srom_max_txpwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_NONE,
				WL_TX_NSS_1, &srom_bl_pwr);
			floor_pwr = (floor_pwr > srom_bl_pwr.pwr[9]) ?
				srom_bl_pwr.pwr[9]: floor_pwr;
#endif /* WL_MU_TX */

			/* Adjust board limits based on environmental conditions */
			if (ISLCN40PHY(pi)) {
					wlc_lcn40phy_apply_cond_chg(pi->u.pi_lcn40phy,
						srom_max_txpwr);
			}
			if (ISNPHY(pi) && CHIPID_4324X_EPA_FAMILY(pi)) {
					wlc_nphy_apply_cond_chg(pi, srom_max_txpwr);
			}

			/* Choose minimum of provided regulatory and board/hw limits */
			ppr_compare_min(srom_max_txpwr, reg_txpwr_limit);

			/* Subtract 4 (1.0db) for 4313(IPA) as we are doing PA trimming
			 * otherwise subtract 6 (1.5 db)
			 * to ensure we don't go over
			 * the limit given a noisy power detector.  The result
			 * is now a target, not a limit.
			 */
			if (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)) {
				wlc_lcnphy_modify_max_txpower(pi, srom_max_txpwr);
			}
			else {
				ppr_minus_cmn_val(srom_max_txpwr, pi->tx_pwr_backoff);
			}
		}
	}

	ppr_delete(pi->sh->osh, reg_txpwr_limit);

	if ((tx_pwr_target = ppr_create(pi->sh->osh, PPR_CHSPEC_BW(chspec))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		     __FUNCTION__, MALLOCED(pi->sh->osh)));
		ppr_delete(pi->sh->osh, srom_max_txpwr);
		PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
		return;
	}

	if ((pi->tx_power_offset != NULL) &&
	    (ppr_get_ch_bw(pi->tx_power_offset) != PPR_CHSPEC_BW(chspec))) {
		ppr_delete(pi->sh->osh, pi->tx_power_offset);
		pi->tx_power_offset = NULL;
	}
	if (pi->tx_power_offset == NULL) {
		if ((pi->tx_power_offset = ppr_create(pi->sh->osh, PPR_CHSPEC_BW(chspec))) ==
			NULL) {
			PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
				__FUNCTION__, MALLOCED(pi->sh->osh)));
			ppr_delete(pi->sh->osh, srom_max_txpwr);
			ppr_delete(pi->sh->osh, tx_pwr_target);
			PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
			return;
		}
	}

	ppr_clear(pi->tx_power_offset);

	cga = wlc_phy_channel_gain_adjust(pi);

	/* Combine user target, regulatory limit, SROM/HW/board limit and power
	 * percentage to get a tx power target for each rate.
	 */
	FOREACH_CORE(pi, core) {
		/* The user target is the starting point for determining the transmit
		 * power.  If pi->txoverride is true, then use the user target as the
		 * tx power target.
		 */
		ppr_set_cmn_val(tx_pwr_target, pi->tx_user_target);

#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WL_EXPORT_TXPOWER)
		/* Only allow tx power override for internal or test builds. */
		if (!pi->txpwroverride)
#endif // endif
		{

			/* Choose least of user and now combined regulatory/hw targets */
			ppr_compare_min(tx_pwr_target, srom_max_txpwr);

			/* Board specific fix reduction */

			/* Apply power output percentage */
			if (pi->txpwr_percent < 100)
				ppr_multiply_percentage(tx_pwr_target, pi->txpwr_percent);

			/* Apply power output degrade */
			if (pi->txpwr_degrade != 0)
				ppr_minus_cmn_val(tx_pwr_target, pi->txpwr_degrade);

#ifdef WL_SARLIMIT
			if (ISHTPHY(pi)) {
				ppr_apply_max(tx_pwr_target, pi->sarlimit[core]);
				/* find each core TP of all rates */
				pi->txpwr_max_percore[core] = ppr_get_max(tx_pwr_target);
			}
#endif /* WL_SARLIMIT */

			/* Enforce min power and save result as power target.
			 * LCNPHY references off the minimum so this is not appropriate for it.
			 */
			if (!ISLCNPHY(pi)) {
				if (ISACPHY(pi))
				{
					mintxpwr = wlc_phy_txpwrctrl_update_minpwr_acphy(pi);
#if defined(WLTXPWR1_SIGNED)
					/*
					 * for signed builds, only avoid forcing
					 * off if user val is below min
					 */
					if (pi->tx_user_target > mintxpwr)
#endif // endif
					/*
					 * Rates that cannot be sent because hardware cannot whisper
					 * that soft need to be WL_RATE_DISABLED.
					 */
					ppr_force_disabled(tx_pwr_target, mintxpwr);
				}
				else {
					ppr_apply_min(tx_pwr_target, mintxpwr);
				}
			}
			/* Channel Gain Adjustment */
			if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
				ppr_minus_cmn_val(tx_pwr_target, cga);
			}
		}

		if (ISHTPHY(pi)) {
			ppr_apply_max(tx_pwr_target, wlc_phy_txpwr_max_est_pwr_get_htphy(pi));
		}

		tx_pwr_max = ppr_get_max(tx_pwr_target);

#if defined(WLTXPWR1_SIGNED)
		if (tx_pwr_max < (pi->min_txpower * WLC_TXPWR_DB_FACTOR)) {
			tx_pwr_max = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
		}
#endif // endif
		if (ISHTPHY(pi) || ISACPHY(pi))
			tx_pwr_min = ppr_get_min(tx_pwr_target, mintxpwr);
		else
			tx_pwr_min = ppr_get_min(tx_pwr_target, WL_RATE_DISABLED);
#ifdef WL_SARLIMIT
		if (ISHTPHY(pi)) {
			tx_pwr_max = MIN(tx_pwr_max, pi->txpwr_max_percore[core]);
		}
#endif // endif
		/* Now calculate the tx_power_offset and update the hardware... */
		pi->tx_power_max_per_core[core] = tx_pwr_max;
		pi->tx_power_min_per_core[core] = tx_pwr_min;

#ifdef WLTXPWR_CACHE
		if (wlc_phy_txpwr_cache_is_cached(pi->txpwr_cache, pi->radio_chanspec) == TRUE) {
			wlc_phy_set_cached_pwr_min(pi->txpwr_cache, pi->radio_chanspec, core,
				tx_pwr_min);
			wlc_phy_set_cached_pwr_max(pi->txpwr_cache, pi->radio_chanspec, core,
				tx_pwr_max);
		}
#endif // endif
		pi->openlp_tx_power_min = tx_pwr_min;
		pi->txpwrnegative = 0;

		/* just to make sure */
		ASSERT(tx_pwr_min != 0);

		PHY_NONE(("wl%d: %s: min %d max %d\n", pi->sh->unit, __FUNCTION__,
		    tx_pwr_min, tx_pwr_max));

		/* determinate the txpower offset by either of the following 2 methods:
		 * txpower_offset = txpower_max - txpower_target OR
		 * txpower_offset = txpower_target - txpower_min
		 */

		if (core == PHY_CORE_0) {
			pi->curpower_display_core = PHY_CORE_0;

			if (txpwr_targets != NULL)
				ppr_copy_struct(tx_pwr_target, txpwr_targets);
		}

		if (!pi->hwpwrctrl || ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi) ||
		      band5g_4319_flag) {
			ppr_cmn_val_minus(tx_pwr_target, pi->tx_power_max_per_core[core]);
		} else {
			ppr_minus_cmn_val(tx_pwr_target, pi->tx_power_min_per_core[core]);
		}

		ppr_compare_max(pi->tx_power_offset, tx_pwr_target);

		if (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID))
			wlc_lcnphy_modify_rate_power_offsets(pi);

		if (ISLCNPHY(pi) || ISLCN40PHY(pi) || (ISNPHY(pi) && CHIPID_4324X_EPA_FAMILY(pi)) ||
			TINY_RADIO(pi)) {
		/* CCK Pwr Index Convergence Correction */
			ppr_dsss_rateset_t dsss;
			uint rate;
			int32 temp;
			ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
			for (rate = 0; rate < WL_RATESET_SZ_DSSS; rate++) {
				if ((CHIPID_4324X_EPA_FAMILY(pi) &&
					(pi->sromi->bphy_sm_fix_opt == 1)) || TINY_RADIO(pi)) {
					temp = (int32)(dsss.pwr[rate]);
					temp += pi->sromi->cckPwrIdxCorr;
					dsss.pwr[rate] = (int8)((uint8)temp);
				} else {
					temp = (int32)(-dsss.pwr[rate]);
					temp -= pi_lcn->cckPwrIdxCorr;
					dsss.pwr[rate] = (int8)((uint8)(-temp));
				}
			}
			ppr_set_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
		}

	}	/* CORE loop */

#ifdef WLTXPWR_CACHE
	if (wlc_phy_txpwr_cache_is_cached(pi->txpwr_cache, pi->radio_chanspec) == TRUE) {
		wlc_phy_set_cached_pwr(pi->sh->osh, pi->txpwr_cache, pi->radio_chanspec,
			TXPWR_CACHE_POWER_OFFSETS, pi->tx_power_offset);
	}
#endif // endif
	/*
	 * PHY_ERROR(("#####The final power offset limit########\n"));
	 * ppr_mcs_printf(pi->tx_power_offset);
	 */
	ppr_delete(pi->sh->osh, tx_pwr_target);
	ppr_delete(pi->sh->osh, srom_max_txpwr);
#if (ACCONF || ACCONF2) && defined(WLC_HIGH) && (defined(WLTEST) || defined(WLPKTENG))
	/* for 4360A/B0, when targetPwr is out of the tssi visibility range,
	 * force the power offset to be the delta between the lower bound of visibility
	 * range and the targetPwr
	 */
	if (pi->txpwroverrideset) {
		if (ISACPHY(pi) && (ACMAJORREV_0(pi->pubpi.phy_rev) ||
		    ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev))) {

			openloop_pwrctrl_delta = wlc_phy_tssivisible_thresh_acphy(pi) -
				pi->tx_user_target;

			if (openloop_pwrctrl_delta > 0) {
				if (R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC)
					mac_enabled = TRUE;
				ppr_set_cmn_val(pi->tx_power_offset, 0);
				phy_tpc_recalc_tgt(pi->tpci);

				/* Stop PKTENG if already running.. */
				if (!mac_enabled)
					wlapi_enable_mac(pi->sh->physhim);
				wlapi_bmac_pkteng(pi->sh->physhim, 0, 0);
				OSL_DELAY(100);

				/* Turn ON Power Control */
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
				wlc_phy_txpwrctrl_enable_acphy(pi, 1);

				FOREACH_CORE(pi, core) {
					pi->tx_power_max_per_core[core]
						= wlc_phy_tssivisible_thresh_acphy(pi) & 0xff;
					/* Set TX Power here for PKTENG */
					wlc_phy_txpwrctrl_set_target_acphy
						(pi, pi->tx_power_max_per_core[core], core);
				}
				wlapi_enable_mac(pi->sh->physhim);

				/* Start PKTENG to settle TX power Control */
				wlapi_bmac_pkteng(pi->sh->physhim, 1, 100);
				OSL_DELAY(1000);
				if (!mac_enabled)
					wlapi_suspend_mac_and_wait(pi->sh->physhim);
				OSL_DELAY(100);

				/* Toggle Power Control to save off base index */
				wlc_phy_txpwrctrl_enable_acphy(pi, 0);
				ppr_set_cmn_val(pi->tx_power_offset, openloop_pwrctrl_delta);
#ifdef WLTXPWR_CACHE
				wlc_phy_txpwr_cache_invalidate(pi->txpwr_cache);
#endif  /* WLTXPWR_CACHE */
#ifdef WLC_HIGH_ONLY
				wlc_bmac_phy_txpwr_cache_invalidate(wlc->hw);
#endif // endif

				PHY_NONE(("###offset: %d targetPwr %d###\n",
					openloop_pwrctrl_delta,
					pi->tx_power_max_per_core[0]));
			}
		}
		pi->txpwroverrideset = FALSE;
	}
#endif // endif

	/* Don't call the hardware specifics if we were just trying to retrieve the target powers */
	if (txpwr_targets == NULL) {
		phy_tpc_recalc_tgt(pi->tpci);
	}

#ifdef WL_MU_TX
	if (ISACPHY(pi) && (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))) {
		/* '-6' (1/4dB step) stands for 1.5dB backoff for txpwrctrl error */
		floor_pwr = tx_pwr_max - (floor_pwr - 6);
		floor_pwr = (floor_pwr >= 0)? floor_pwr: 0;
		wlc_phy_offload_ppr_to_svmp(pi, pi->tx_power_offset, (int16) floor_pwr);
	}
#endif /* WL_MU_TX */

	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
}

#ifdef WL_MU_TX
void wlc_phy_offload_ppr_to_svmp(phy_info_t *pi, ppr_t* tx_power_offset, int16 floor_pwr)
{

	uint8  k, m, n, bwtype, stall_val, num_set;
	int16  pwr0, pwr1, min_boff;
	/* additional backoff of steered frm pwr with nss_tot = 1, 2, 3, 4 and sounding pwr */
	int16  stpwr_boffs[5] = {0, 0, 4, 6, 0};
	uint32 mem_id;
	uint32 txbf_ppr_buff[8], svmp_addr = 0xe90;
	ppr_vht_mcs_rateset_t pwr_backoff;
	wl_tx_mode_t tx_mode[5] = {WL_TX_MODE_TXBF, WL_TX_MODE_TXBF, WL_TX_MODE_TXBF,
		WL_TX_MODE_TXBF, WL_TX_MODE_NONE};
	wl_tx_nss_t nss[5] = {WL_TX_NSS_1, WL_TX_NSS_2, WL_TX_NSS_3, WL_TX_NSS_4, WL_TX_NSS_4};
#if defined(WL_PSMX)
	int8 ndp_pwroffs[D11_MU_NDPPWR_MAXMCS+1] = {0};
	int ndppwr_num = D11_MU_NDPPWR_MAXMCS+1;
	uint16 ndp_pwr_tbl;
#endif /* WL_PSMX */
	uint16 txchain_cnt;
	uint8 txchain, rxchain;
	bool mac_enabled = FALSE;

	if (!ISACPHY(pi)) {
		return;
	}

	if (R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC)
		mac_enabled = TRUE;

	if (mac_enabled)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* disable stall */
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	BCM_REFERENCE(txbf_ppr_buff);

	wlc_phy_stf_chain_get_valid(pi, &txchain, &rxchain);
	txchain_cnt = PHY_BITSCNT(txchain);
	switch (txchain_cnt) {
	case 3:
		nss[3] = WL_TX_NSS_3;
		nss[4] = WL_TX_NSS_3;
		txchain_cnt = WL_TX_CHAINS_3;
		break;
	case 4:
	case 2:
	case 1:
	default:
		/* If 4 TX chains, set txchains according to the enum. For 1 or 2 chain
		 * configurations, this code is a don't care anyway.
		 */
		txchain_cnt = WL_TX_CHAINS_4;
		break;
	}
	mem_id = 4;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		num_set = 1;
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		num_set = 2;
	} else if (CHSPEC_IS80(pi->radio_chanspec)) {
		num_set = 3;
	} else {
		num_set = 0;
	}

	for (k = 0; k < num_set; k++) {
		// bwtype: "20IN20", "40IN40", "80IN80", "20IN40", "20IN80", "40IN80"
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			bwtype = (k == 0)? 0: 0xff;
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			bwtype = (k == 0)? 3: ((k == 1)? 1: 0xff);
		} else if (CHSPEC_IS80(pi->radio_chanspec)) {
			bwtype = (k == 0)? 4: ((k == 1)? 5: ((k == 2)? 2: 0xff));
		} else {
			bwtype = 0xff;
		}

		/* loop through steered frame 1ss, 2ss, 3ss, 4ss followed by openloop 4ss */
		for (n = 0; n < 5; n++) {
			min_boff = (n == 0)? 0: (floor_pwr + stpwr_boffs[n]);
			ppr_get_vht_mcs(tx_power_offset, bwtype, nss[n], tx_mode[n],
				txchain_cnt, &pwr_backoff);
			for (m = 0; m < 8; m++) {
				if (2*m < WL_RATESET_SZ_VHT_MCS_P) {
					/* temp WAR to use PPR of "non-TXBF c[8-11]s4"
					 * for corresponding TXBF rates
					 */
					if (m == 4 && ((txchain_cnt == WL_TX_CHAINS_4 && n == 3) ||
					(txchain_cnt == WL_TX_CHAINS_3 && (n == 2 || n == 3)))) {
						ppr_get_vht_mcs(tx_power_offset, bwtype, nss[4],
							tx_mode[4], txchain_cnt, &pwr_backoff);
					}
					/* only apply power bounding for mcs >= 8 || ndp */
					if (m >= 4 || n == 4) {
						pwr0 = (pwr_backoff.pwr[2*m] < min_boff) ?
							min_boff: pwr_backoff.pwr[2*m];
						pwr1 = (pwr_backoff.pwr[2*m+1] < min_boff)?
							min_boff: pwr_backoff.pwr[2*m+1];
					} else {
						pwr0 = pwr_backoff.pwr[2*m];
						pwr1 = pwr_backoff.pwr[2*m+1];
					}
					/* convert to 2's complement format */
					pwr0 += (pwr0 >= 0)? 0: 256;
					pwr1 += (pwr1 >= 0)? 0: 256;
					txbf_ppr_buff[m] = (pwr0 & 0xFF) | ((pwr1 & 0xFF) << 16);
				} else {
					txbf_ppr_buff[m] = 0;
				}
				if (bwtype == 0xff)
					txbf_ppr_buff[m] = 0;
			}

			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 8,
					svmp_addr, 32, &txbf_ppr_buff[0]);
			svmp_addr += 8;
#if defined(WL_PSMX)
			/* Save the MU NDP pwroffs table */
			ndppwr_num = MIN(D11_MU_NDPPWR_MAXMCS+1, WL_RATESET_SZ_VHT_MCS);
			if (bwtype <= 2 && n == 4) {
				memcpy((uint8 *)ndp_pwroffs, (uint8 *)pwr_backoff.pwr, ndppwr_num);
			}
#endif /* WL_PSMX */
		}
	}

#if defined(WL_PSMX)
	if (D11REV_GE(pi->sh->corerev, 64)) {
		ndp_pwr_tbl = wlapi_bmac_read_shmx(pi->sh->physhim, MX_NDPPWR_PTR);
		for (m = 0; m < ndppwr_num; m += 2) {
			/* lower byte for even mcs , higher byte for odd mcs */
			pwr0 = ndp_pwroffs[m] >= 0 ? ndp_pwroffs[m]: ndp_pwroffs[m] + 256;
			pwr1 = ndp_pwroffs[m+1] >= 0 ? ndp_pwroffs[m+1]: ndp_pwroffs[m+1] + 256;
			pwr0 = (pwr0 >> 1) & 0x7F;
			pwr1 = (pwr1 >> 1) & 0x7F;
			wlapi_bmac_write_shmx(pi->sh->physhim, 2 * ndp_pwr_tbl + m,
					pwr0 | (pwr1 << 8));
		}
	}
#endif /* WL_PSMX */

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);

	if (mac_enabled)
		wlapi_enable_mac(pi->sh->physhim);
}
#endif /* WL_MU_TX */

#ifdef BCMDBG
void
wlc_phy_txpower_limits_dump(ppr_t* txpwr, bool ishtphy)
{
	int i;
	char fraction[4][4] = {"   ", ".25", ".5 ", ".75"};
	ppr_dsss_rateset_t dsss_limits;
	ppr_ofdm_rateset_t ofdm_limits;
	ppr_ht_mcs_rateset_t mcs_limits;

	printf("CCK                  ");
	ppr_get_dsss(txpwr, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss_limits);
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++) {
		printf(" %2d%s", dsss_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[dsss_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz OFDM SISO      ");
	ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz OFDM CDD       ");
	ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_CDD, WL_TX_CHAINS_2, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "20MHz 1 Nsts to 1 Tx " : "20MHz MCS 0-7 SISO   ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "20MHz 1 Nsts to 2 Tx " : "20MHz MCS 0-7 CDD    ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	if (ishtphy) {
		printf("20MHz 1 Nsts to 3 Tx ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
			&mcs_limits);
	} else {
		printf("20MHz MCS 0-7 STBC   ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_2,
			&mcs_limits);
	}
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "20MHz 2 Nsts to 2 Tx " : "20MHz MCS 8-15 SDM   ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	if (ishtphy) {
		printf("20MHz 2 Nsts to 3 Tx ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("20MHz 3 Nsts to 3 Tx ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_3, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");
	}

	printf("40MHz OFDM SISO      ");
	ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("40MHz OFDM CDD       ");
	ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_CDD, WL_TX_CHAINS_2, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 1 Nsts to 1 Tx " : "40MHz MCS 0-7 SISO   ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 1 Nsts to 2 Tx " : "40MHz MCS 0-7 CDD    ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 1 Nsts to 3 Tx " : "40MHz MCS 0-7 CDD    ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 2 Nsts to 2 Tx " : "40MHz MCS8-15 SDM    ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	if (ishtphy) {
		printf("40MHz 2 Nsts to 3 Tx ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("40MHz 3 Nsts to 3 Tx ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_3, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");
	}

	if (!ishtphy)
		return;

	printf("20MHz UL CCK         ");
	ppr_get_dsss(txpwr, WL_TX_BW_20IN40, WL_TX_CHAINS_1, &dsss_limits);
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++) {
		printf(" %2d%s", dsss_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[dsss_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz UL OFDM SISO   ");
	ppr_get_ofdm(txpwr, WL_TX_BW_20IN40, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz UL OFDM CDD    ");
	ppr_get_ofdm(txpwr, WL_TX_BW_20IN40, WL_TX_MODE_CDD, WL_TX_CHAINS_2, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 1 Nsts to 1 Tx  ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 1 Nsts to 2 Tx  ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 1 Nsts to 3 Tx  ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 2 Nsts to 2 Tx  ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 2 Nsts to 3 Tx  ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 3 Nsts to 3 Tx  ");
	ppr_get_ht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_3, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
		&mcs_limits);
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");
}

#endif /* BCMDBG */

/* Translates the regulatory power limit array into an array of length TXP_NUM_RATES,
 * which can match the board limit array obtained using the SROM. Moreover, since the NPHY chips
 * currently do not distinguish between Legacy OFDM and MCS0-7, the SISO and CDD regulatory power
 * limits of these rates need to be combined carefully.
 * This internal/static function needs to be called whenever the chanspec or regulatory TX power
 * limits change.
 */
static void
wlc_phy_txpower_reg_limit_calc(phy_info_t *pi, ppr_t *txpwr, chanspec_t chanspec,
	ppr_t *txpwr_limit)
{
	uint k, i;
	ppr_ofdm_rateset_t ofdm_limits;
	ppr_ht_mcs_rateset_t mcs_limits;

	ppr_copy_struct(txpwr, txpwr_limit);

#if defined(WLPROPRIETARY_11N_RATES)
	ppr_get_ht_mcs(txpwr_limit, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
		WL_TX_CHAINS_1, &mcs_limits);
	if (mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_87] == WL_RATE_DISABLED) {
		mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_87] =
			mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_7];
		mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_88] =
			mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_7];
		ppr_set_ht_mcs(txpwr_limit, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
	}
#endif // endif

	/* Obtain the regulatory limits for Legacy OFDM and HT-OFDM 11n rates in NPHY chips */
	if (ISNPHY(pi)) {
		wl_tx_bw_t bw = WL_TX_BW_20;
		wl_tx_mode_t mode = WL_TX_MODE_NONE;
		wl_tx_chains_t chains = WL_TX_CHAINS_1;
		/* If NPHY is enabled, then use min of OFDM and MCS_20_SISO values as the regulatory
		 * limit for SISO Legacy OFDM and MCS0-7 rates. Similarly, for 40 MHz SIS0 Legacy
		 * OFDM  and MCS0-7 rates as well as for 20 MHz and 40 MHz CDD Legacy OFDM and
		 * MCS0-7 rates. This is because the current hardware implementation uses common
		 * powers for the 8 Legacy ofdm and 8 mcs0-7 rates, i.e. they share the same power
		 * table. The power table is populated based on the constellation, coding rate, and
		 * transmission mode (SISO/CDD/STBC/SDM). Therefore, care must be taken to match the
		 * constellation and coding rates of the Legacy OFDM and MCS0-7 rates since the 8
		 * Legacy OFDM rates and the 8 MCS0-7 rates do not have a 1-1 correspondence in
		 * these parameters.
		 */

		/* Regulatory limits for Legacy OFDM rates 20 and 40 MHz, SISO and CDD. The
		 * regulatory limits for the corresponding MCS0-7 20 and 40 MHz, SISO and
		 * CDD rates should also be mapped into Legacy OFDM limits and the minimum
		 * of the two limits should be taken for each rate.
		 */
		/* Regulatory limits for MCS0-7 rates 20 and 40 MHz, SISO and CDD. The
		 * regulatory limits for the corresponding Legacy OFDM 20 and 40 MHz, SISO and
		 * CDD rates should also be mapped into MCS0-7 limits and the minimum
		 * of the two limits should be taken for each rate.
		 */
		for (k = 0; k < 6; k++) {
			ppr_ofdm_rateset_t ofdm_from_mcs_limits;
			ppr_ht_mcs_rateset_t mcs_from_ofdm_limits;

			switch (k) {
			case 0:
				/* 20 MHz Legacy OFDM SISO */
				bw = WL_TX_BW_20;
				mode = WL_TX_MODE_NONE;
				chains = WL_TX_CHAINS_1;
				break;
			case 1:
				/* 20 MHz Legacy OFDM CDD */
				bw = WL_TX_BW_20;
				mode = WL_TX_MODE_CDD;
				chains = WL_TX_CHAINS_2;
				break;
			case 2:
				/* 40 MHz Legacy OFDM SISO */
				bw = WL_TX_BW_40;
				mode = WL_TX_MODE_NONE;
				chains = WL_TX_CHAINS_1;
				break;
			case 3:
				/* 40 MHz Legacy OFDM CDD */
				bw = WL_TX_BW_40;
				mode = WL_TX_MODE_CDD;
				chains = WL_TX_CHAINS_2;
				break;
			case 4:
				/* case 4: 20in40 MHz Legacy OFDM SISO */
				bw = WL_TX_BW_20IN40;
				mode = WL_TX_MODE_NONE;
				chains = WL_TX_CHAINS_1;
				break;
			case 5:
				/* 20in40 legacy ofdm cdd */
				bw = WL_TX_BW_20IN40;
				mode = WL_TX_MODE_CDD;
				chains = WL_TX_CHAINS_2;
				break;
			}
			ppr_get_ht_mcs(txpwr, bw, WL_TX_NSS_1, mode, chains, &mcs_limits);
			ppr_get_ofdm(txpwr, bw, mode, chains, &ofdm_limits);

			wlc_phy_copy_mcs_to_ofdm_powers(&mcs_limits, &ofdm_from_mcs_limits);

			wlc_phy_copy_ofdm_to_mcs_powers(&ofdm_limits, &mcs_from_ofdm_limits);

			for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
				if (ofdm_from_mcs_limits.pwr[i] == WL_RATE_DISABLED)
					continue;
				ofdm_limits.pwr[i] = MIN(ofdm_limits.pwr[i],
					ofdm_from_mcs_limits.pwr[i]);
			}

			for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
				if (mcs_from_ofdm_limits.pwr[i] == WL_RATE_DISABLED)
					continue;
				mcs_limits.pwr[i] = MIN(mcs_limits.pwr[i],
					mcs_from_ofdm_limits.pwr[i]);
			}

			ppr_set_ofdm(txpwr_limit, bw, mode, chains, &ofdm_limits);

			ppr_set_ht_mcs(txpwr_limit, bw, WL_TX_NSS_1, mode, chains, &mcs_limits);

		}
	}
}

/* Map Legacy OFDM powers-per-rate to MCS 0-7 powers-per-rate by matching the
 * constellation and coding rate of the corresponding Legacy OFDM and MCS rates. The power
 * of 9 Mbps Legacy OFDM is used for MCS-0 (same as 6 Mbps power) since no equivalent
 * of 9 Mbps exists in the 11n standard in terms of constellation and coding rate.
 */

void
wlc_phy_copy_ofdm_to_mcs_powers(ppr_ofdm_rateset_t* ofdm_limits, ppr_ht_mcs_rateset_t* mcs_limits)
{
	uint rate1;
	uint rate2;

	for (rate1 = 0, rate2 = 1; rate1 < WL_RATESET_SZ_OFDM-1; rate1++, rate2++) {
		mcs_limits->pwr[rate1] = ofdm_limits->pwr[rate2];
	}
	mcs_limits->pwr[rate1] = mcs_limits->pwr[rate1 - 1];
}

/* Map MCS 0-7 powers-per-rate to Legacy OFDM powers-per-rate by matching the
 * constellation and coding rate of the corresponding Legacy OFDM and MCS rates. The power
 * of 9 Mbps Legacy OFDM is set to the power of MCS-0 (same as 6 Mbps power) since no equivalent
 * of 9 Mbps exists in the 11n standard in terms of constellation and coding rate.
 */

void
wlc_phy_copy_mcs_to_ofdm_powers(ppr_ht_mcs_rateset_t* mcs_limits, ppr_ofdm_rateset_t* ofdm_limits)
{
	uint rate1;
	uint rate2;

	ofdm_limits->pwr[0] = mcs_limits->pwr[0];
	for (rate1 = 1, rate2 = 0; rate1 < WL_RATESET_SZ_OFDM; rate1++, rate2++) {
		ofdm_limits->pwr[rate1] = mcs_limits->pwr[rate2];
	}
}

void
wlc_phy_txpwr_percent_set(wlc_phy_t *ppi, uint8 txpwr_percent)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->txpwr_percent = txpwr_percent;
}

void
wlc_phy_txpwr_degrade(wlc_phy_t *ppi, uint8 txpwr_degrade)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->txpwr_degrade = txpwr_degrade;
}

void
BCMATTACHFN(wlc_phy_machwcap_set)(wlc_phy_t *ppi, uint32 machwcap)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->sh->machwcap = machwcap;
}

void
wlc_phy_runbist_config(wlc_phy_t *ppi, bool start_end)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	uint16 rxc;
	rxc = 0;

	if (start_end == ON) {
		if (!ISNPHY(pi) || ISHTPHY(pi))
			return;

		/* Keep pktproc in reset during bist run */
		if (NREV_IS(pi->pubpi.phy_rev, 3) || NREV_IS(pi->pubpi.phy_rev, 4)) {
			rxc = phy_utils_read_phyreg(pi, NPHY_RxControl);
			phy_utils_write_phyreg(pi, NPHY_RxControl,
			      NPHY_RxControl_dbgpktprocReset_MASK | rxc);
		}
	} else {
		if (NREV_IS(pi->pubpi.phy_rev, 3) || NREV_IS(pi->pubpi.phy_rev, 4)) {
			phy_utils_write_phyreg(pi, NPHY_RxControl, rxc);
		}

		wlc_phy_por_inform(ppi);
	}
}

/* Set tx power limits */
/* BMAC_NOTE: this only needs a chanspec so that it can choose which 20/40 limits
 * to save in phy state. Would not need this if we ether saved all the limits and
 * applied them only when we were on the correct channel, or we restricted this fn
 * to be called only when on the correct channel.
 */
/* FTODO make sure driver functions are calling this version */
void
wlc_phy_txpower_limit_set(wlc_phy_t *ppi, ppr_t *txpwr, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)ppi;
#ifdef TXPWR_TIMING
	int time1, time2;
	time1 = hnd_time_us();
#endif // endif
	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phy_txpower_recalc_target(pi, txpwr, NULL);
	wlc_phy_cal_txpower_recalc_sw(pi);
	wlapi_enable_mac(pi->sh->physhim);
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);

#ifdef TXPWR_TIMING
	time2 = hnd_time_us();
	wlc_phy_txpower_limit_set_time = time2 - time1;
#endif // endif
}

void
wlc_phy_ofdm_rateset_war(wlc_phy_t *pih, bool war)
{
	phy_info_t *pi = (phy_info_t*)pih;

	pi->ofdm_rateset_war = war;
}

void
wlc_phy_bf_preempt_enable(wlc_phy_t *pih, bool bf_preempt)
{
}

bool
wlc_phy_txpower_hw_ctrl_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISNPHY(pi)) {
		return pi->nphy_txpwrctrl;
	} else if (ISHTPHY(pi) || ISACPHY(pi)) {
		return pi->txpwrctrl;
	} else {
		return pi->hwpwrctrl;
	}
}

/* XXX MUST REVIEW MUST REVIEW MUST REVIEW; why is old NPHY off different than
 * restore logic in the off case???? johnvb
 *
 * Old "off" code did NOTHING to turn power control off other then set a
 * structure variable (probably a bug) but looks like it might have been
 * trying to do this (dead) code to turn it off.
 *
 * OFF:
 *		phy_utils_and_phyreg(pi, NPHY_TxPwrCtrlCmd,
 * 			(uint16) ~NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK);
 * 		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);
 *
 * Old "restore" code had this logic to turn power control OFF or ON:
 *
 * OFF:
 *		-- FIXME, to restore previous pwrindex
 *		wlc_phy_txpwr_fixpower_nphy(pi);
 * ON:
 * 		-- turn on power control --
 *		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_ON);
 *
 * What is the correct way to do this?
 *
 * Do we need to set power index's/save power index's?
 *
 * We are trying to implement a single set function (not a save and restore).
 * This might mean we need to internally keep some state information
 * including whether we have any information yet.  Of course this information
 * would need to be initialized at some appropriate place.
 *
 * When would that logically be?  (phy attach, init, etc)
 *
 */
void
wlc_phy_txpower_hw_ctrl_set(wlc_phy_t *ppi, bool hwpwrctrl)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	bool cur_hwpwrctrl = pi->hwpwrctrl;
	bool suspend;

	/* validate if hardware power control is capable */
	if (!pi->hwpwrctrl_capable) {
		PHY_ERROR(("wl%d: hwpwrctrl not capable\n", pi->sh->unit));
		return;
	}

	PHY_INFORM(("wl%d: setting the hwpwrctrl to %d\n", pi->sh->unit, hwpwrctrl));
	pi->hwpwrctrl = hwpwrctrl;
	pi->nphy_txpwrctrl = hwpwrctrl;
	pi->txpwrctrl = hwpwrctrl;

	/* if power control mode is changed, propagate it */
	if (ISNPHY(pi)) {
		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* turn on/off power control */
		wlc_phy_txpwrctrl_enable_nphy(pi, pi->nphy_txpwrctrl);
		if (pi->nphy_txpwrctrl == PHY_TPC_HW_OFF) {
			wlc_phy_txpwr_fixpower_nphy(pi);
		} else {
			/* restore the starting txpwr index */
			phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlCmd,
				NPHY_TxPwrCtrlCmd_pwrIndex_init_MASK, pi->saved_txpwr_idx);
		}

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	} else if (ISHTPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* turn on/off power control */
		wlc_phy_txpwrctrl_enable_htphy(pi, pi->txpwrctrl);

		wlapi_enable_mac(pi->sh->physhim);
	} else if (hwpwrctrl != cur_hwpwrctrl) {
		/* save, change and restore tx power control */

		if (ISLCNCOMMONPHY(pi)) {
			return;

		}
	}
}

#ifdef BCM_OL_DEV
void
wlc_phy_sarlimit_write(wlc_phy_t *pih, uint32 sar)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint core;

	if (ISACPHY(pi)) {
		FOREACH_CORE(pi, core) {
			pi->sarlimit[core] = sar & 0xff;
			sar >>= 8;
		}
		wlc_phy_set_sarlimit_acphy(pi);
	}
}

void
wlc_phy_curpwr_write(wlc_phy_t *pih, uint8 curpwr)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint core;

	if (ISACPHY(pi)) {
		FOREACH_CORE(pi, core) {
			pi->tx_power_max_per_core[core] = curpwr;
			wlc_phy_txpwrctrl_set_target_acphy(pi,
				pi->tx_power_max_per_core[core], core);
			curpwr >>= 8;
		}
	}
}
#endif /* BCM_OL_DEV */

#ifdef WL_SARLIMIT
static void
wlc_phy_sarlimit_set_int(phy_info_t *pi, int8 *sar)
{
	uint core;

	FOREACH_CORE(pi, core) {
		pi->sarlimit[core] =
			MAX((sar[core] - pi->tx_pwr_backoff), pi->min_txpower);
	}
	if (ISACPHY(pi) && pi->sh->clk) {
		wlc_phy_set_sarlimit_acphy(pi);
	}
}
#endif /* WL_SARLIMIT */

void
wlc_phy_txpower_ipa_upd(phy_info_t *pi)
{
	/* this should be expanded to work with all new PHY capable of iPA */
	if (ISACPHY(pi)) {
		pi->ipa2g_on = ((pi->sromi->epagain2g == 2) || (pi->sromi->extpagain2g == 2));
		pi->ipa5g_on = ((pi->sromi->epagain5g == 2) || (pi->sromi->extpagain5g == 2));
	} else if (NREV_GE(pi->pubpi.phy_rev, 3)) {
		pi->ipa2g_on = (pi->fem2g->extpagain == 2);
		pi->ipa5g_on = (pi->fem5g->extpagain == 2);
	} else {
		pi->ipa2g_on = FALSE;
		pi->ipa5g_on = FALSE;
	}
	PHY_INFORM(("wlc_phy_txpower_ipa_upd: ipa 2g %d, 5g %d\n", pi->ipa2g_on, pi->ipa5g_on));
}

void
wlc_phy_txpower_get_current(wlc_phy_t *ppi, ppr_t *reg_pwr, phy_tx_power_t *power)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint8 min_pwr, core;

	if (ISNPHY(pi)) {
		power->rf_cores = 2;
		power->flags |= (WL_TX_POWER_F_MIMO);
		if (pi->nphy_txpwrctrl == PHY_TPC_HW_ON)
			power->flags |= (WL_TX_POWER_F_ENABLED | WL_TX_POWER_F_HW);
	} else if (ISHTPHY(pi)) {
		power->rf_cores = PHYCORENUM(pi->pubpi.phy_corenum);
		power->flags |= (WL_TX_POWER_F_MIMO);
		if (pi->txpwrctrl == PHY_TPC_HW_ON)
			power->flags |= (WL_TX_POWER_F_ENABLED | WL_TX_POWER_F_HW);
	} else if (ISACPHY(pi)) {
		power->rf_cores = PHYCORENUM(pi->pubpi.phy_corenum);
		power->flags |= (WL_TX_POWER_F_MIMO);
		if (pi->txpwrctrl == PHY_TPC_HW_ON)
			power->flags |= (WL_TX_POWER_F_ENABLED | WL_TX_POWER_F_HW);
	}
	else if (ISLCNCOMMONPHY(pi)) {
		power->rf_cores = 1;
		power->flags |= (WL_TX_POWER_F_SISO);
		if (pi->radiopwr_override == RADIOPWR_OVERRIDE_DEF)
			power->flags |= WL_TX_POWER_F_ENABLED;
		if (pi->hwpwrctrl)
			power->flags |= WL_TX_POWER_F_HW;
	} else {
		power->rf_cores = 1;
		if (pi->radiopwr_override == RADIOPWR_OVERRIDE_DEF)
			power->flags |= WL_TX_POWER_F_ENABLED;
		if (pi->hwpwrctrl)
			power->flags |= WL_TX_POWER_F_HW;
	}

	{
		ppr_t *txpwr_srom;

		if ((txpwr_srom = ppr_create(pi->sh->osh, PPR_CHSPEC_BW(pi->radio_chanspec))) !=
			NULL) {
			wlc_phy_txpower_sromlimit(ppi, pi->radio_chanspec, &min_pwr, txpwr_srom, 0);

			ppr_copy_struct(txpwr_srom, power->ppr_board_limits);

			ppr_delete(pi->sh->osh, txpwr_srom);
		}
	}
	/* reuse txpwr for target */
	wlc_phy_txpower_recalc_target(pi, reg_pwr, power->ppr_target_powers);

	power->display_core = pi->curpower_display_core;

	/* fill the est_Pout, max target power, and rate index corresponding to the max
	 * target power fields
	 */

	wlc_phy_get_est_pout(ppi, power->est_Pout, power->est_Pout_act, &power->est_Pout_cck);

	if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)) {
		/* Store the maximum target power among all rates */
		FOREACH_CORE(pi, core) {
			power->tx_power_max[core] = pi->tx_power_max_per_core[core];
		}
	} else if (pi->hwpwrctrl && pi->sh->up) {
		/* If hw (ucode) based, read the hw based estimate in realtime */
		phy_utils_phyreg_enter(pi);
		if (ISLCNPHY(pi)) {
			/* Store the maximum target power among all rates */
			power->tx_power_max[0] = pi->tx_power_max_per_core[0];
			power->tx_power_max[1] = pi->tx_power_max_per_core[0];

			if (wlc_phy_tpc_isenabled_lcnphy(pi))
				power->flags |= (WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
			else
				power->flags &= ~(WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);

			wlc_lcnphy_get_tssi(pi, (int8*)&power->est_Pout[0],
				(int8*)&power->est_Pout_cck);
		} else if (ISLCN40PHY(pi)) {
			/* Store the maximum target power among all rates */
			power->tx_power_max[0] = pi->tx_power_max_per_core[0];
			power->tx_power_max[1] = pi->tx_power_max_per_core[0];
			if (pi->pi_fptr.ishwtxpwrctrl && pi->pi_fptr.ishwtxpwrctrl(pi))
				power->flags |= (WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
			else
				power->flags &= ~(WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
		}
		phy_utils_phyreg_exit(pi);
	}
#ifdef WL_SARLIMIT
	FOREACH_CORE(pi, core) {
		if (ISACPHY(pi))
			power->SARLIMIT[core] = pi->sarlimit[core];
		else
			power->SARLIMIT[core] = WLC_TXPWR_MAX;
	}
#endif // endif
}

void wlc_phy_get_tssi_sens_min(wlc_phy_t *ppi, int8 *tssiSensMinPwr)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISACPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_get_tssisens_min_acphy(pi, tssiSensMinPwr);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void wlc_phy_get_est_pout(wlc_phy_t *ppi, uint8* est_Pout, uint8* est_Pout_adj, uint8* est_Pout_cck)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	*est_Pout_cck = 0;
	/* fill the est_Pout array */
	if (ISNPHY(pi)) {
		uint32 est_pout;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
			est_pout = wlc_phy_txpower_est_power_lcnxn_rev3(pi);
		else
			est_pout = wlc_phy_txpower_est_power_nphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);

		/* Store the adjusted  estimated powers */
		est_Pout_adj[0] = (est_pout >> 8) & 0xff;
		est_Pout_adj[1] = est_pout & 0xff;

		/* Store the actual estimated powers without adjustment */
		est_Pout[0] = est_pout >> 24;
		est_Pout[1] = (est_pout >> 16) & 0xff;

		/* if invalid, return 0 */
		if (est_Pout[0] == 0x80)
			est_Pout[0] = 0;
		if (est_Pout[1] == 0x80)
			est_Pout[1] = 0;

		/* if invalid, return 0 */
		if (est_Pout_adj[0] == 0x80)
			est_Pout_adj[0] = 0;
		if (est_Pout_adj[1] == 0x80)
			est_Pout_adj[1] = 0;

	} else if (ISHTPHY(pi)) {
		/* Get power estimates */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_txpwr_est_pwr_htphy(pi, est_Pout, est_Pout_adj);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);

	} else if (ISACPHY(pi)) {
		/* Get power estimates */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_txpwr_est_pwr_acphy(pi, est_Pout, est_Pout_adj);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);

	} else if (!pi->hwpwrctrl) {
	} else if (pi->sh->up) {
		if (ISLCNPHY(pi)) {
			wlc_lcnphy_get_tssi(pi, (int8*)&est_Pout[0], (int8*)est_Pout_cck);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_get_tssi(pi, (int8*)&est_Pout[0], (int8*)est_Pout_cck);
			est_Pout_adj[0] = est_Pout[0];
		}
		phy_utils_phyreg_exit(pi);
	}
}

#if defined(BCMDBG) || defined(WLTEST) || defined(BCMCCX)
/* Return the current instantaneous est. power
 * For swpwr ctrl it's based on current TSSI value (as opposed to average)
 * Mainly used by mfg.
 */
static void
wlc_phy_txpower_get_instant(phy_info_t *pi, void *pwr)
{
	tx_inst_power_t *power = (tx_inst_power_t *)pwr;
	/* If sw power control, grab the instant value based on current TSSI Only
	 * If hw based, read the hw based estimate in realtime
	 */
	if (ISLCNPHY(pi)) {
		if (!pi->hwpwrctrl)
			return;

		wlc_lcnphy_get_tssi(pi, (int8*)&power->txpwr_est_Pout_gofdm,
			(int8*)&power->txpwr_est_Pout[0]);
		power->txpwr_est_Pout[1] = power->txpwr_est_Pout_gofdm;

	} else if (ISLCN40PHY(pi)) {
		if (!pi->hwpwrctrl)
			return;

		wlc_lcn40phy_get_tssi(pi, (int8*)&power->txpwr_est_Pout_gofdm,
			(int8*)&power->txpwr_est_Pout[0]);
		power->txpwr_est_Pout[1] = power->txpwr_est_Pout_gofdm;

	}

}
#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_test_init(phy_info_t *pi, int channel, bool txpkt)
{
	if (channel > MAXCHANNEL)
		return BCME_OUTOFRANGECHAN;

	wlc_phy_chanspec_set((wlc_phy_t*)pi, CH20MHZ_CHSPEC(channel));

	/* stop any requests from the stack and prevent subsequent thread */
	pi->phytest_on = TRUE;

	if (ISLCNPHY(pi)) {

		wlc_phy_init_test_lcnphy(pi);

	} else if (ISLCN40PHY(pi)) {

		wlc_phy_init_test_lcn40phy(pi);

	} else if (ISACPHY(pi)) {
		wlc_phy_init_test_acphy(pi);
	}

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	return 0;
}

int
wlc_phy_test_stop(phy_info_t *pi)
{
	if (pi->phytest_on == FALSE)
		return 0;

	/* stop phytest mode */
	pi->phytest_on = FALSE;

	/* For NPHY, phytest register needs to be accessed only via phy and not directly */
	if (ISNPHY(pi)) {
		/* XXX PR39573: clean up 2G regs. For 5G, bypass
		 *  cleanup, assuming active phytest doesn't come from bphy
		 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_and_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), 0xfc00);
			if (NREV_GE(pi->pubpi.phy_rev, 3))
				/* BPHY_DDFS_ENABLE is removed in mimophy rev 3 */
			    phy_utils_write_phyreg(pi, NPHY_bphytestcontrol, 0x0);
			else
				phy_utils_write_phyreg(pi, NPHY_TO_BPHY_OFF + BPHY_DDFS_ENABLE, 0);
		}
	} else if (ISHTPHY(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_AND_RAW_ENTRY(HTPHY_TO_BPHY_OFF + BPHY_TEST, 0xfc00)
				PHY_REG_WRITE_RAW_ENTRY(HTPHY_bphytestcontrol, 0x0)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else if (ISACPHY(pi)) {
#define ACPHY_TO_BPHY_OFF       0x3A1
#define ACPHY_BPHY_TEST         0x08
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_AND_RAW_ENTRY(ACPHY_TO_BPHY_OFF + ACPHY_BPHY_TEST, 0xfc00)
				PHY_REG_WRITE_RAW_ENTRY(ACPHY_bphytestcontrol(pi->pubpi.phy_rev),
					0x0)
			PHY_REG_LIST_EXECUTE(pi);
		}
		wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
	} else if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
		PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
	} else {
		AND_REG(pi->sh->osh, &pi->regs->phytest, 0xfc00);
		phy_utils_write_phyreg(pi, BPHY_DDFS_ENABLE, 0);
	}

	return 0;
}

/*
 * Rate is number of 500 Kb units.
 */
static int
wlc_phy_test_evm(phy_info_t *pi, int channel, uint rate, int txpwr)
{
	d11regs_t *regs = pi->regs;
	uint16 reg = 0;
	int bcmerror = 0;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		if (ISNPHY(pi)) {
			phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST),
			              pi->evm_phytest);
		} else if (ISLCNPHY(pi)) {
			phy_utils_write_phyreg(pi, LCNPHY_bphyTest, pi->evm_phytest);
			phy_utils_write_phyreg(pi, LCNPHY_ClkEnCtrl, 0);
			wlc_lcnphy_tx_pu(pi, 0);
		} else if (ISLCN40PHY(pi)) {
			phy_utils_write_phyreg(pi, LCN40PHY_bphyTest, pi->evm_phytest);
			phy_utils_write_phyreg(pi, LCN40PHY_ClkEnCtrl, 0);
			wlc_lcn40phy_tx_pu(pi, 0);
		} else 	if (ISHTPHY(pi))
			wlc_phy_bphy_testpattern_htphy(pi, HTPHY_TESTPATTERN_BPHY_EVM, reg, FALSE);
		 else
			W_REG(pi->sh->osh, &regs->phytest, pi->evm_phytest);

		pi->evm_phytest = 0;

		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL) {
			W_REG(pi->sh->osh, &pi->regs->psm_gpio_out, pi->evm_o);
			W_REG(pi->sh->osh, &pi->regs->psm_gpio_oe, pi->evm_oe);
			OSL_DELAY(1000);
		}
		return 0;
	}

	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL) {
		PHY_INFORM(("wl%d: %s: PACTRL boardflag set, clearing gpio 0x%04x\n",
			pi->sh->unit, __FUNCTION__, BOARD_GPIO_PACTRL));
		/* Store initial values */
		pi->evm_o = R_REG(pi->sh->osh, &pi->regs->psm_gpio_out);
		pi->evm_oe = R_REG(pi->sh->osh, &pi->regs->psm_gpio_oe);
		AND_REG(pi->sh->osh, &regs->psm_gpio_out, ~BOARD_GPIO_PACTRL);
		OR_REG(pi->sh->osh, &regs->psm_gpio_oe, BOARD_GPIO_PACTRL);
		OSL_DELAY(1000);
	}

	if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
		return bcmerror;

	reg = TST_TXTEST_RATE_2MBPS;
	switch (rate) {
	case 2:
		reg = TST_TXTEST_RATE_1MBPS;
		break;
	case 4:
		reg = TST_TXTEST_RATE_2MBPS;
		break;
	case 11:
		reg = TST_TXTEST_RATE_5_5MBPS;
		break;
	case 22:
		reg = TST_TXTEST_RATE_11MBPS;
		break;
	}
	reg = (reg << TST_TXTEST_RATE_SHIFT) & TST_TXTEST_RATE;

	PHY_INFORM(("wlc_evm: rate = %d, reg = 0x%x\n", rate, reg));

	/* Save original contents */
	if (pi->evm_phytest == 0 && !ISHTPHY(pi)) {
		if (ISNPHY(pi)) {
			pi->evm_phytest = phy_utils_read_phyreg(pi,
			                               (NPHY_TO_BPHY_OFF + BPHY_TEST));
		} else if (ISLCNPHY(pi)) {
			pi->evm_phytest = phy_utils_read_phyreg(pi, LCNPHY_bphyTest);
			phy_utils_write_phyreg(pi, LCNPHY_ClkEnCtrl, 0xffff);
		} else if (ISLCN40PHY(pi)) {
			pi->evm_phytest = phy_utils_read_phyreg(pi, LCN40PHY_bphyTest);
			phy_utils_write_phyreg(pi, LCN40PHY_ClkEnCtrl, 0xffff);
		} else
			pi->evm_phytest = R_REG(pi->sh->osh, &regs->phytest);
	}

	/* Set EVM test mode */
	if (ISHTPHY(pi)) {
		wlc_phy_bphy_testpattern_htphy(pi, NPHY_TESTPATTERN_BPHY_EVM, reg, TRUE);
	} else if (ISNPHY(pi)) {
		phy_utils_and_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST),
		            ~(TST_TXTEST_ENABLE|TST_TXTEST_RATE|TST_TXTEST_PHASE));
		phy_utils_or_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), TST_TXTEST_ENABLE | reg);
	} else if (ISLCNPHY(pi)) {
		wlc_lcnphy_tx_pu(pi, 1);
		phy_utils_or_phyreg(pi, LCNPHY_bphyTest, 0x128);
	} else if ISLCN40PHY(pi) {
		wlc_lcn40phy_tx_pu(pi, 1);
		phy_utils_or_phyreg(pi, LCN40PHY_bphyTest, 0x128);
	} else {
		AND_REG(pi->sh->osh, &regs->phytest,
		        ~(TST_TXTEST_ENABLE|TST_TXTEST_RATE|TST_TXTEST_PHASE));
		OR_REG(pi->sh->osh, &regs->phytest, TST_TXTEST_ENABLE | reg);
	}
	return 0;
}

static int
wlc_phy_test_carrier_suppress(phy_info_t *pi, int channel)
{
	d11regs_t *regs = pi->regs;
	int bcmerror = 0;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		if (ISNPHY(pi)) {
			phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST),
			              pi->car_sup_phytest);
		} else if (ISLCNPHY(pi)) {
			/* release the gpio controls from cc */
			wlc_lcnphy_epa_switch(pi, 0);
			/* Disable carrier suppression */
			wlc_lcnphy_crsuprs(pi, channel);
		} else if (ISLCN40PHY(pi)) {
			/* Disable carrier suppression */
			wlc_lcn40phy_crsuprs(pi, channel);
		} else 	if (ISHTPHY(pi)) {
			wlc_phy_bphy_testpattern_htphy(pi, HTPHY_TESTPATTERN_BPHY_RFCS, 0, FALSE);
		} else
			W_REG(pi->sh->osh, &regs->phytest, pi->car_sup_phytest);

		pi->car_sup_phytest = 0;
		return 0;
	}

	if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
		return bcmerror;

	/* Save original contents */
	if (pi->car_sup_phytest == 0 && !ISHTPHY(pi)) {
	        if (ISNPHY(pi)) {
			pi->car_sup_phytest = phy_utils_read_phyreg(pi,
			                                   (NPHY_TO_BPHY_OFF + BPHY_TEST));
		} else if (ISLCNPHY(pi)) {
			pi->car_sup_phytest = phy_utils_read_phyreg(pi, LCNPHY_bphyTest);
		} else
			pi->car_sup_phytest = R_REG(pi->sh->osh, &regs->phytest);
	}

	/* set carrier suppression test mode */
	if (ISHTPHY(pi)) {
		wlc_phy_bphy_testpattern_htphy(pi, HTPHY_TESTPATTERN_BPHY_RFCS, 0, TRUE);
	} else if (ISNPHY(pi)) {
		PHY_REG_LIST_START
			PHY_REG_AND_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_TEST, 0xfc00)
			PHY_REG_OR_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_TEST, 0x0228)
		PHY_REG_LIST_EXECUTE(pi);
	} else if (ISLCNPHY(pi)) {
		/* get the gpio controls to cc */
		wlc_lcnphy_epa_switch(pi, 1);
		wlc_lcnphy_crsuprs(pi, channel);
	} else {
		AND_REG(pi->sh->osh, &regs->phytest, 0xfc00);
		OR_REG(pi->sh->osh, &regs->phytest, 0x0228);
	}

	return 0;
}

static int
wlc_phy_test_freq_accuracy(phy_info_t *pi, int channel)
{
	int bcmerror = 0;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means this is a request to end the test */
	if (channel == 0) {
		/* Restore original values */
		if (ISNPHY(pi)) {
			if ((bcmerror = wlc_phy_freq_accuracy_nphy(pi, channel)) != BCME_OK)
				return bcmerror;
		} else if (ISHTPHY(pi)) {
			if ((bcmerror = wlc_phy_freq_accuracy_htphy(pi, channel)) != BCME_OK)
				return bcmerror;
		} else if (ISACPHY(pi)) {
			if ((bcmerror = wlc_phy_freq_accuracy_acphy(pi, channel)) != BCME_OK)
				return bcmerror;
		} else if (ISLCNPHY(pi)) {
			wlc_lcnphy_stop_tx_tone(pi);
			if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
				wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_TEMPBASED);
			else
				wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_HW);
			wlc_lcnphy_epa_switch(pi, 0);
		} else if (ISLCN40PHY(pi)) {
			/* For lcn40, restore the 24dB scaler */
			PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_enable, 1);
			PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_coeff, 64);
			wlc_lcn40phy_set_bbmult(pi, 64);
			wlc_lcn40phy_stop_tx_tone(pi);
			pi->pi_fptr.settxpwrctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);
		}

		return 0;
	}

	if ((bcmerror = wlc_phy_test_init(pi, channel, FALSE)))
		return bcmerror;

	if (ISNPHY(pi)) {
		if ((bcmerror = wlc_phy_freq_accuracy_nphy(pi, channel)) != BCME_OK)
			return bcmerror;
	} else if (ISACPHY(pi)) {
		if ((bcmerror = wlc_phy_freq_accuracy_acphy(pi, channel)) != BCME_OK)
			return bcmerror;
	} else if (ISHTPHY(pi)) {
		if ((bcmerror = wlc_phy_freq_accuracy_htphy(pi, channel)) != BCME_OK)
			return bcmerror;
	} else if (ISLCNPHY(pi)) {
		/* get the gpio controls to cc */
		wlc_lcnphy_epa_switch(pi, 1);
		wlc_lcnphy_start_tx_tone(pi, 0, 112, 0);
		wlc_lcnphy_set_tx_pwr_by_index(pi, (int)94);
	} else if (ISLCN40PHY(pi)) {
		/* For lcn40, need to scale up tx tone by 24 dB */
		PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_enable, 1);
		PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_coeff, 255);
		wlc_lcn40phy_set_bbmult(pi, 255);
		wlc_lcn40phy_start_tx_tone(pi, 0, 112, 0);
		pi->pi_fptr.settxpwrbyindex(pi, (int)94);
	}

	return 0;
}

#endif // endif

void
wlc_phy_antsel_type_set(wlc_phy_t *ppi, uint8 antsel_type)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->antsel_type = antsel_type;

	/* initialize flag to init HW Rx antsel if the board supports it */
	if ((pi->antsel_type == ANTSEL_2x3_HWRX) || (pi->antsel_type == ANTSEL_1x2_HWRX))
		pi->nphy_enable_hw_antsel = TRUE;
	else
		pi->nphy_enable_hw_antsel = FALSE;
}

bool
wlc_phy_test_ison(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return (pi->phytest_on);
}

void
wlc_phy_interference_set(wlc_phy_t *pih, bool init)
{
	int wanted_mode;
	phy_info_t *pi = (phy_info_t *)pih;

	if (!(ISNPHY(pi) || ISHTPHY(pi) || (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)))
		return;

	if (pi->sh->interference_mode_override == TRUE) {
		/* keep the same values */
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (pi->sh->interference_mode_5G_override == 0 ||
				pi->sh->interference_mode_5G_override == 1) {
				wanted_mode = pi->sh->interference_mode_5G_override;
			} else {
				wanted_mode = 0;
			}
		} else
#endif /* BAND5G */
		{
			wanted_mode = pi->sh->interference_mode_2G_override;
		}
	} else {

#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			wanted_mode = pi->sh->interference_mode_5G;
		} else
#endif /* BAND5G */
		{
			wanted_mode = pi->sh->interference_mode_2G;
		}
	}

	if (CHSPEC_CHANNEL(pi->radio_chanspec) != pi->interf->curr_home_channel) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifndef WLC_DISABLE_ACI
		phy_noise_set_mode(pi->noisei, wanted_mode, init);
#endif // endif
		pi->sh->interference_mode = wanted_mode;

		wlapi_enable_mac(pi->sh->physhim);
	}
}

#ifndef WLC_DISABLE_ACI
/* %%%%%% interference */
#if defined(RXDESENS_EN)
static bool
wlc_phy_interference(phy_info_t *pi, int wanted_mode, bool init)
{
	if (init) {
		pi->interference_mode_crs_time = 0;
		pi->crsglitch_prev = 0;
		if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) {
			/* clear out all the state */
			wlc_phy_noisemode_reset_nphy(pi);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				wlc_phy_acimode_reset_nphy(pi);
			}
		} else if (ISHTPHY(pi)) {
			/* clear out all the state */
			wlc_phy_noisemode_reset_htphy(pi);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				wlc_phy_acimode_reset_htphy(pi);
			}
		} else if (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)) {
			wlc_lcnphy_aci_init(pi);
		} else if (ISLCN40PHY(pi) && (CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)) {
			wlc_lcn40phy_aci_init(pi);
		}
	}

	/* NPHY 5G, supported for NON_WLAN and INTERFERE_NONE only */
	if (((ISNPHY(pi) || ISHTPHY(pi)) &&
		(CHSPEC_IS2G(pi->radio_chanspec) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
	        (wanted_mode == NON_WLAN || wanted_mode == INTERFERE_NONE))))) {
		if (wanted_mode == INTERFERE_NONE) {	/* disable */
			if (ISHTPHY(pi)) {
				/* XXX
				 * PR114196: Set MF thresholds back to default for mode 0
				 */
				wlc_phy_noise_raise_MFthresh_htphy(pi, FALSE);
			}

			switch (pi->cur_interference_mode) {
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
			case WLAN_MANUAL:
			        if (ISNPHY(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_nphy(pi, FALSE,
						PHY_ACI_PWR_NOTPRESENT);
				} else if (ISHTPHY(pi) &&
					CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi, FALSE,
						PHY_ACI_PWR_NOTPRESENT);
				}
				pi->aci_state &= ~ACI_ACTIVE;
				break;
			case NON_WLAN:
				if (ISNPHY(pi)) {
					if (NREV_GE(pi->pubpi.phy_rev, 3) &&
						CHSPEC_IS2G(pi->radio_chanspec)) {
						wlc_phy_acimode_set_nphy(pi,
							FALSE,
							PHY_ACI_PWR_NOTPRESENT);
						pi->aci_state &= ~ACI_ACTIVE;
					}
				} else if (ISHTPHY(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi,
						FALSE,
						PHY_ACI_PWR_NOTPRESENT);
					pi->aci_state &= ~ACI_ACTIVE;
				}
				break;
			}
		} else {	/* Enable */
			if (ISHTPHY(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
				/* XXX
				 * PR114196: Set high MF threshold for all RSSIs in 2G if
				 * interference mode is not 0
				 */
				wlc_phy_noise_raise_MFthresh_htphy(pi, TRUE);
			}

			switch (wanted_mode) {
			case NON_WLAN:
				if (ISNPHY(pi)) {
					if (!NREV_GE(pi->pubpi.phy_rev, 3)) {
						PHY_ERROR(("NON_WLAN not supported for NPHY\n"));
					}
				}
				break;
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
				/* fall through */
				if (((pi->aci_state & ACI_ACTIVE) != 0) ||
					ISNPHY(pi) || ISHTPHY(pi))
					break;
				/* FALLTHRU */
			case WLAN_MANUAL:
				if  (ISNPHY(pi)) {
					int aci_pwr = CHIPID_4324X_MEDIA_FAMILY(pi) ?
					    PHY_ACI_PWR_MED : PHY_ACI_PWR_HIGH;
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						wlc_phy_acimode_set_nphy(pi, TRUE, aci_pwr);
					}
				} else if (ISHTPHY(pi)) {
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						wlc_phy_acimode_set_htphy(pi, TRUE,
							PHY_ACI_PWR_HIGH);
					}
				}
			}
		}
	}

	/* Depending on interfernece modes, do whatever needs to be done */
	if (ISACPHY(pi)) {
		if (wanted_mode == INTERFERE_NONE) {
			wlc_phy_desense_aci_reset_params_acphy(pi, TRUE, TRUE, TRUE);
			wlc_phy_hwaci_setup_acphy(pi, FALSE, FALSE);
			wlc_phy_aci_w2nb_setup_acphy(pi, FALSE);
			wlc_phy_preempt(pi, FALSE);
		}

		/* Nothing needs to be done for ACPHY_ACI_GLITCHBASED_DESENSE */

		/* Switch on( & init) the required rssi settings */
		if ((pi->sh->interference_mode & ACPHY_ACI_HWACI_PKTGAINLMT) != 0)
			wlc_phy_hwaci_setup_acphy(pi, TRUE, TRUE);
		if ((pi->sh->interference_mode & ACPHY_ACI_W2NB_PKTGAINLMT) != 0)
			wlc_phy_aci_w2nb_setup_acphy(pi, TRUE);
		if ((pi->sh->interference_mode & ACPHY_ACI_PREEMPTION) != 0)
		        wlc_phy_preempt(pi, TRUE);
	}

	if (ISLCN40PHY(pi) && (CHIPID(pi->sh->chip) == BCM43143_CHIP_ID)) {
		wlc_lcn40phy_rev6_aci(pi, wanted_mode);
	}

	pi->cur_interference_mode = wanted_mode;
	return TRUE;
}
#endif /* RXDESENS_EN */
/* %%%%%% interference */
/* update aci rx carrier sense glitch moving average */
static void
wlc_phy_aci_update_ma(phy_info_t *pi)
{

	int32 delta = 0;
	int32 bphy_delta = 0;
	int32 ofdm_delta = 0;
	int32 badplcp_delta = 0;
	int32 bphy_badplcp_delta = 0;
	int32 ofdm_badplcp_delta = 0;

	if ((pi->interf->cca_stats_func_called == FALSE) || pi->interf->cca_stats_mbsstime <= 0) {
	  uint16 cur_glitch_cnt;
	  uint16 bphy_cur_glitch_cnt = 0;
	  uint16 cur_badplcp_cnt = 0;
	  uint16 bphy_cur_badplcp_cnt = 0;

#ifdef WLSRVSDB
	  uint8 bank_offset = 0;
	  uint8 vsdb_switch_failed = 0;
	  uint8 vsdb_split_cntr = 0;

	  if (CHSPEC_CHANNEL(pi->radio_chanspec) == pi->srvsdb_state->sr_vsdb_channels[0]) {
	    bank_offset = 0;
	  } else if (CHSPEC_CHANNEL(pi->radio_chanspec) == pi->srvsdb_state->sr_vsdb_channels[1]) {
	    bank_offset = 1;
	  }
	  /* Assume vsdb switch failed, if no switches were recorded for both the channels */
	  vsdb_switch_failed = !(pi->srvsdb_state->num_chan_switch[0] &
	  pi->srvsdb_state->num_chan_switch[1]);

	  /* use split counter for each channel if vsdb is active and vsdb switch was successfull */
	  /* else use last 1 sec delta counter for current channel for calculations */
	  vsdb_split_cntr = (!vsdb_switch_failed) && (pi->srvsdb_state->srvsdb_active);

#endif /* WLSRVSDB */
	  /* determine delta number of rxcrs glitches */
		cur_glitch_cnt =
			wlapi_bmac_read_shm(pi->sh->physhim, MACSTAT_ADDR(MCSTOFF_RXCRSGLITCH));
	  delta = cur_glitch_cnt - pi->interf->aci.pre_glitch_cnt;
	  pi->interf->aci.pre_glitch_cnt = cur_glitch_cnt;

#ifdef WLSRVSDB
	  if (vsdb_split_cntr) {
	    delta =  pi->srvsdb_state->sum_delta_crsglitch[bank_offset];
	  }
#endif /* WLSRVSDB */
	  if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi) || ISACPHY(pi)) {

	    /* compute the rxbadplcp  */
		cur_badplcp_cnt = wlapi_bmac_read_shm(pi->sh->physhim,
			MACSTAT_ADDR(MCSTOFF_RXBADPLCP));
	    badplcp_delta = cur_badplcp_cnt - pi->interf->pre_badplcp_cnt;
	    pi->interf->pre_badplcp_cnt = cur_badplcp_cnt;

#ifdef WLSRVSDB
	    if (vsdb_split_cntr) {
	      badplcp_delta = pi->srvsdb_state->sum_delta_prev_badplcp[bank_offset];
	    }
#endif /* WLSRVSDB */
	    /* determine delta number of bphy rx crs glitches */
		bphy_cur_glitch_cnt = wlapi_bmac_read_shm(pi->sh->physhim,
			MACSTAT_ADDR(MCSTOFF_BPHYGLITCH));
	    bphy_delta = bphy_cur_glitch_cnt - pi->interf->noise.bphy_pre_glitch_cnt;
	    pi->interf->noise.bphy_pre_glitch_cnt = bphy_cur_glitch_cnt;

#ifdef WLSRVSDB
	    if (vsdb_split_cntr) {
	      bphy_delta = pi->srvsdb_state->sum_delta_bphy_crsglitch[bank_offset];
	    }
#endif /* WLSRVSDB */
	    if (CHSPEC_IS2G(pi->radio_chanspec)) {
	      /* ofdm glitches is what we will be using */
	      ofdm_delta = delta - bphy_delta;
	    } else {
	      ofdm_delta = delta;
	    }

	    /* compute bphy rxbadplcp */
			bphy_cur_badplcp_cnt = wlapi_bmac_read_shm(pi->sh->physhim,
				MACSTAT_ADDR(MCSTOFF_BPHY_BADPLCP));
	    bphy_badplcp_delta = bphy_cur_badplcp_cnt -
	      pi->interf->bphy_pre_badplcp_cnt;
	    pi->interf->bphy_pre_badplcp_cnt = bphy_cur_badplcp_cnt;

#ifdef WLSRVSDB
	    if (vsdb_split_cntr) {
	      bphy_badplcp_delta = pi->srvsdb_state->sum_delta_prev_bphy_badplcp[bank_offset];
	    }

#endif /* WLSRVSDB */

	    /* ofdm bad plcps is what we will be using */
	    if (CHSPEC_IS2G(pi->radio_chanspec)) {
	      ofdm_badplcp_delta = badplcp_delta - bphy_badplcp_delta;
	    } else {
	      ofdm_badplcp_delta = badplcp_delta;
	    }
	  }

	  if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi)) {
	    /* if we aren't suppose to update yet, don't */
	    if (pi->interf->scanroamtimer != 0) {
	      return;
	    }

	  }
	} else {
		pi->interf->cca_stats_func_called = FALSE;
		/* Normalizing the statistics per second */
		delta = pi->interf->cca_stats_total_glitch * 1000 /
		        pi->interf->cca_stats_mbsstime;
		bphy_delta = pi->interf->cca_stats_bphy_glitch * 1000 /
		        pi->interf->cca_stats_mbsstime;
		ofdm_delta = delta - bphy_delta;
		badplcp_delta = pi->interf->cca_stats_total_badplcp * 1000 /
		        pi->interf->cca_stats_mbsstime;
		bphy_badplcp_delta = pi->interf->cca_stats_bphy_badplcp * 1000 /
		        pi->interf->cca_stats_mbsstime;
		ofdm_badplcp_delta = badplcp_delta - bphy_badplcp_delta;
	}

	if (delta >= 0) {
		/* evict old value */
		pi->interf->aci.ma_total -= pi->interf->aci.ma_list[pi->interf->aci.ma_index];

		/* admit new value */
		pi->interf->aci.ma_total += (uint16) delta;
		pi->interf->aci.glitch_ma_previous = pi->interf->aci.glitch_ma;
		if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3) &&
		     NREV_LE(pi->pubpi.phy_rev, 15)) || ISHTPHY(pi) || ISACPHY(pi)) {
			pi->interf->aci.glitch_ma = pi->interf->aci.ma_total /
			        PHY_NOISE_MA_WINDOW_SZ;
		} else {
			pi->interf->aci.glitch_ma = pi->interf->aci.ma_total / MA_WINDOW_SZ;
		}

		pi->interf->aci.ma_list[pi->interf->aci.ma_index++] = (uint16) delta;
		if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3) &&
		     NREV_LE(pi->pubpi.phy_rev, 15)) || ISHTPHY(pi) || ISACPHY(pi)) {
			if (pi->interf->aci.ma_index >= PHY_NOISE_MA_WINDOW_SZ)
				pi->interf->aci.ma_index = 0;
		} else {
			if (pi->interf->aci.ma_index >= MA_WINDOW_SZ)
				pi->interf->aci.ma_index = 0;
		}
	}

	if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi) || ISACPHY(pi)) {
		if (badplcp_delta >= 0) {
			pi->interf->badplcp_ma_total -=
				pi->interf->badplcp_ma_list[pi->interf->badplcp_ma_index];
			pi->interf->badplcp_ma_total += (uint16) badplcp_delta;
			pi->interf->badplcp_ma_previous = pi->interf->badplcp_ma;

			if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3) &&
			     NREV_LE(pi->pubpi.phy_rev, 15)) || ISHTPHY(pi) || ISACPHY(pi)) {
				pi->interf->badplcp_ma =
				        pi->interf->badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;
			} else {
				pi->interf->badplcp_ma = pi->interf->badplcp_ma_total/MA_WINDOW_SZ;
			}

			pi->interf->badplcp_ma_list[pi->interf->badplcp_ma_index++] =
				(uint16) badplcp_delta;
			if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3) &&
			     NREV_LE(pi->pubpi.phy_rev, 15)) || ISHTPHY(pi) || ISACPHY(pi)) {
				if (pi->interf->badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
					pi->interf->badplcp_ma_index = 0;
			} else {
				if (pi->interf->badplcp_ma_index >= MA_WINDOW_SZ)
					pi->interf->badplcp_ma_index = 0;
			}
		}

		/* XXX
		 * OFDM GLITCHES
		 * evict old value, admit new value, compute new ma, readjust ma window
		 */
		if ((CHSPEC_IS5G(pi->radio_chanspec) && (ofdm_delta >= 0)) ||
			(CHSPEC_IS2G(pi->radio_chanspec) && (delta >= 0) && (bphy_delta >= 0))) {
			pi->interf->noise.ofdm_ma_total -= pi->interf->noise.
					ofdm_glitch_ma_list[pi->interf->noise.ofdm_ma_index];
			pi->interf->noise.ofdm_ma_total += (uint16) ofdm_delta;
			pi->interf->noise.ofdm_glitch_ma_previous =
				pi->interf->noise.ofdm_glitch_ma;
			pi->interf->noise.ofdm_glitch_ma =
				pi->interf->noise.ofdm_ma_total / PHY_NOISE_MA_WINDOW_SZ;
			pi->interf->noise.ofdm_glitch_ma_list[pi->interf->noise.ofdm_ma_index++] =
				(uint16) ofdm_delta;
			if (pi->interf->noise.ofdm_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
				pi->interf->noise.ofdm_ma_index = 0;
		}

		/* XXX
		 * BPHY GLITCHES
		 * evict old value, admit new value, compute new ma, readjust ma window
		 */
		if (bphy_delta >= 0) {
			pi->interf->noise.bphy_ma_total -= pi->interf->noise.
					bphy_glitch_ma_list[pi->interf->noise.bphy_ma_index];
			pi->interf->noise.bphy_ma_total += (uint16) bphy_delta;
			pi->interf->noise.bphy_glitch_ma_previous =
				pi->interf->noise.bphy_glitch_ma;
			pi->interf->noise.bphy_glitch_ma =
				pi->interf->noise.bphy_ma_total / PHY_NOISE_MA_WINDOW_SZ;
			pi->interf->noise.bphy_glitch_ma_list[pi->interf->noise.bphy_ma_index++] =
				(uint16) bphy_delta;
			if (pi->interf->noise.bphy_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
				pi->interf->noise.bphy_ma_index = 0;
		}

		/* XXX
		 * OFDM BADPLCP
		 * evict old value, admit new value, compute new ma, readjust ma window
		 */
		if ((CHSPEC_IS5G(pi->radio_chanspec) && (ofdm_badplcp_delta >= 0)) ||
			(CHSPEC_IS2G(pi->radio_chanspec) && (badplcp_delta >= 0) &&
			(bphy_badplcp_delta >= 0))) {
			pi->interf->noise.ofdm_badplcp_ma_total -= pi->interf->noise.
				ofdm_badplcp_ma_list[pi->interf->noise.ofdm_badplcp_ma_index];
			pi->interf->noise.ofdm_badplcp_ma_total += (uint16) ofdm_badplcp_delta;
			pi->interf->noise.ofdm_badplcp_ma_previous =
				pi->interf->noise.ofdm_badplcp_ma;
			pi->interf->noise.ofdm_badplcp_ma =
				pi->interf->noise.ofdm_badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;
			pi->interf->noise.ofdm_badplcp_ma_list
				[pi->interf->noise.ofdm_badplcp_ma_index++] =
				(uint16) ofdm_badplcp_delta;
			if (pi->interf->noise.ofdm_badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
				pi->interf->noise.ofdm_badplcp_ma_index = 0;
		}

		/* XXX
		 * BPHY BADPLCP
		 * evict old value, admit new value, compute new ma, readjust ma window
		 */
		if (bphy_badplcp_delta >= 0) {
			pi->interf->noise.bphy_badplcp_ma_total -= pi->interf->noise.
				bphy_badplcp_ma_list[pi->interf->noise.bphy_badplcp_ma_index];
			pi->interf->noise.bphy_badplcp_ma_total += (uint16) bphy_badplcp_delta;
			pi->interf->noise.bphy_badplcp_ma_previous = pi->interf->noise.
				bphy_badplcp_ma;
			pi->interf->noise.bphy_badplcp_ma =
				pi->interf->noise.bphy_badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;
			pi->interf->noise.bphy_badplcp_ma_list
					[pi->interf->noise.bphy_badplcp_ma_index++] =
					(uint16) bphy_badplcp_delta;
			if (pi->interf->noise.bphy_badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
				pi->interf->noise.bphy_badplcp_ma_index = 0;
		}
	}

	if (((ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 16) ||
	    (NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 5)))) || ISHTPHY(pi)) &&
	    (pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
	     pi->sh->interference_mode == NON_WLAN)) {
		PHY_ACI(("wlc_phy_aci_update_ma: ACI= %s, rxglitch_ma= %d,"
			" badplcp_ma= %d, ofdm_glitch_ma= %d, bphy_glitch_ma=%d,"
			" ofdm_badplcp_ma= %d, bphy_badplcp_ma=%d, crsminpwr index= %d,"
			" init gain= 0x%x, channel= %d\n",
			(pi->aci_state & ACI_ACTIVE) ? "Active" : "Inactive",
			pi->interf->aci.glitch_ma,
			pi->interf->badplcp_ma,
			pi->interf->noise.ofdm_glitch_ma,
			pi->interf->noise.bphy_glitch_ma,
			pi->interf->noise.ofdm_badplcp_ma,
			pi->interf->noise.bphy_badplcp_ma,
			pi->interf->crsminpwr_index,
			pi->interf->init_gain_core1, CHSPEC_CHANNEL(pi->radio_chanspec)));
	} else {
		PHY_ACI(("wlc_phy_aci_update_ma: ave glitch %d, ACI is %s, delta is %d\n",
		pi->interf->aci.glitch_ma,
		(pi->aci_state & ACI_ACTIVE) ? "Active" : "Inactive", delta));
	}
#ifdef WLSRVSDB
	/* Clear out cumulatiove cntrs after 1 sec */
	/* reset both chan info becuase its a fresh start after every 1 sec */
	if (pi->srvsdb_state->srvsdb_active) {
		bzero(pi->srvsdb_state->num_chan_switch, 2 * sizeof(uint8));
		bzero(pi->srvsdb_state->sum_delta_crsglitch, 2 * sizeof(uint32));
		bzero(pi->srvsdb_state->sum_delta_bphy_crsglitch, 2 * sizeof(uint32));
		bzero(pi->srvsdb_state->sum_delta_prev_badplcp, 2 * sizeof(uint32));
		bzero(pi->srvsdb_state->sum_delta_prev_bphy_badplcp, 2 * sizeof(uint32));
	}
#endif /* WLSRVSDB */
}

void
wlc_phy_aci_upd(phy_info_t *pi)
{
	bool desense_gt_4;
	int glit_plcp_sum;
#ifdef WLSRVSDB
	uint8 offset = 0;
	uint8 vsdb_switch_failed = 0;

	if (pi->srvsdb_state->srvsdb_active) {
		/* Assume vsdb switch failure, if no chan switches were recorded in last 1 sec */
		vsdb_switch_failed = !(pi->srvsdb_state->num_chan_switch[0] &
			pi->srvsdb_state->num_chan_switch[1]);
		if (CHSPEC_CHANNEL(pi->radio_chanspec) ==
			pi->srvsdb_state->sr_vsdb_channels[0]) {
			offset = 0;
		} else if (CHSPEC_CHANNEL(pi->radio_chanspec) ==
			pi->srvsdb_state->sr_vsdb_channels[1]) {
			offset = 1;
		}

		/* return if vsdb switching was active and time spent in current channel */
		/* is less than 1 sec */
		/* If vsdb switching had failed, it could be in a  deadlock */
		/* situation because of noise/aci */
		/* So continue with aci mitigation even though delta timers show less than 1 sec */

		if ((pi->srvsdb_state->sum_delta_timer[offset] < (1000 * 1000)) &&
			!vsdb_switch_failed) {

			bzero(pi->srvsdb_state->num_chan_switch, 2 * sizeof(uint8));
			return;
		}
		PHY_INFORM(("Enter ACI mitigation for chan %x  since %d ms of time has expired\n",
			pi->srvsdb_state->sr_vsdb_channels[offset],
			pi->srvsdb_state->sum_delta_timer[offset]/1000));

		/* reset the timers after an effective 1 sec duration in the channel */
		bzero(pi->srvsdb_state->sum_delta_timer, 2 * sizeof(uint32));

		/* If enetering aci mitigation scheme, we need a save of */
		/* previous pi structure while doing switch */
		pi->srvsdb_state->swbkp_snapshot_valid[offset] = 0;
	}
#endif /* WLSRVSDB */
	wlc_phy_aci_update_ma(pi);

	if (ISACPHY(pi)) {
		if (!(ACPHY_ENABLE_FCBS_HWACI(pi)) || ACPHY_HWACI_WITH_DESENSE_ENG(pi)) {
			if ((pi->sh->interference_mode & ACPHY_ACI_GLITCHBASED_DESENSE) != 0) {
				wlc_phy_desense_aci_engine_acphy(pi);
			}
		}
		if (!ACPHY_ENABLE_FCBS_HWACI(pi)) {
			if ((pi->sh->interference_mode & (ACPHY_ACI_HWACI_PKTGAINLMT |
				ACPHY_ACI_W2NB_PKTGAINLMT)) != 0) {
				wlc_phy_hwaci_engine_acphy(pi);
			}
		}
	} else {
		switch (pi->sh->interference_mode) {
		case NON_WLAN:
			/* NON_WLAN NPHY */
			if (ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 16) ||
			    (NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 5)))) {
				/* run noise mitigation only */
				wlc_phy_noisemode_upd_nphy(pi);
			} else if (ISHTPHY(pi)) {
				/* run noise mitigation only */
				wlc_phy_noisemode_upd_htphy(pi);
			} else if (ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 6) &&
				NREV_LE(pi->pubpi.phy_rev, 15))) {
				wlc_phy_noisemode_upd(pi);
			}
			break;
		case WLAN_AUTO:
			if (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)) {
				wlc_lcnphy_aci_noise_measure(pi);
			} else if (ISLCN40PHY(pi) && ((CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43340_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43341_CHIP_ID))) {
				wlc_lcn40phy_aci_upd(pi);
			}
			else if (ISNPHY(pi) || ISHTPHY(pi)) {
				if (ASSOC_INPROG_PHY(pi))
					break;
#ifdef NOISE_CAL_LCNXNPHY
				if ((NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
					NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))) {
						wlc_phy_aci_noise_measure_nphy(pi, TRUE);
				}
				else
#endif // endif
				{
					/* 5G band not supported yet */
					if (CHSPEC_IS5G(pi->radio_chanspec))
						break;

					if (PUB_NOT_ASSOC(pi)) {
						/* not associated:  do not run aci routines */
						break;
					}
					PHY_ACI(("Interf Mode 3,"
						" pi->interf->aci.glitch_ma = %d\n",
						pi->interf->aci.glitch_ma));

					/* Attempt to enter ACI mode if not already active */
					/* only run this code if associated */
					if (!(pi->aci_state & ACI_ACTIVE)) {
						if ((pi->sh->now  % NPHY_ACI_CHECK_PERIOD) == 0) {

							if ((pi->interf->aci.glitch_ma +
								pi->interf->badplcp_ma) >=
								pi->interf->aci.enter_thresh) {
								if (ISNPHY(pi)) {
								wlc_phy_acimode_upd_nphy(pi);
								} else if (ISHTPHY(pi)) {
								wlc_phy_acimode_upd_htphy(pi);
								}
							}
						}
					} else {
						if (((pi->sh->now - pi->aci_start_time) %
							pi->aci_exit_check_period) == 0) {
							if (ISNPHY(pi)) {
								wlc_phy_acimode_upd_nphy(pi);
							} else if (ISHTPHY(pi)) {
								wlc_phy_acimode_upd_htphy(pi);
							}
						}
					}
				}
			}
			break;
		case WLAN_AUTO_W_NOISE:
			if (ISNPHY(pi) || ISHTPHY(pi)) {
#ifdef NOISE_CAL_LCNXNPHY
				if ((NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
					NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))) {
						wlc_phy_aci_noise_measure_nphy(pi, TRUE);
				}
				else
#endif // endif
				{
					/* 5G band not supported yet */
					if (CHSPEC_IS5G(pi->radio_chanspec))
						break;

					/* only do this for 4322 and future revs */
					if (ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 16) ||
					    (NREV_GE(pi->pubpi.phy_rev, 3) &&
					     NREV_LE(pi->pubpi.phy_rev, 5)))) {
						/* Attempt to enter ACI mode if
						 * not already active
						 */
						wlc_phy_aci_noise_upd_nphy(pi);
					} else if (ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 6) &&
						NREV_LE(pi->pubpi.phy_rev, 15))) {
					  PHY_ACI(("Interf Mode 4\n"));
					  desense_gt_4 = (pi->interf->noise.ofdm_desense >= 4 ||
					      pi->interf->noise.bphy_desense >= 4);
					  glit_plcp_sum = pi->interf->aci.glitch_ma +
					        pi->interf->badplcp_ma;
					  if (!(pi->aci_state & ACI_ACTIVE)) {
					    if ((pi->sh->now  % NPHY_ACI_CHECK_PERIOD) == 0) {
					      if ((glit_plcp_sum >=
					        pi->interf->aci.enter_thresh) || desense_gt_4) {
						if (ISNPHY(pi)) {
						  wlc_phy_acimode_upd_nphy(pi);
						}
					      }
					    }
					  } else {
					    if (((pi->sh->now - pi->aci_start_time) %
						 pi->aci_exit_check_period) == 0) {
					      if (ISNPHY(pi)) {
						wlc_phy_acimode_upd_nphy(pi);
					      }
					    }
					  }

					  wlc_phy_noisemode_upd(pi);

					} else if (ISHTPHY(pi)) {
						wlc_phy_aci_noise_upd_htphy(pi);
					}
				}
			} else if (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)) {
				wlc_lcnphy_aci_noise_measure(pi);
			} else if (ISLCN40PHY(pi) && ((CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43340_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43341_CHIP_ID))) {
				wlc_lcn40phy_aci_upd(pi);
			}
			break;

		default:
			break;
	}
	}
}

void
wlc_phy_noisemode_upd(phy_info_t *pi)
{

	int8 bphy_desense_delta = 0, ofdm_desense_delta = 0;
	uint16 glitch_badplcp_sum;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) != pi->interf->curr_home_channel)
		return;

	if (pi->interf->scanroamtimer != 0) {
		/* have not updated moving averages */
		return;
	}

	/* BPHY desense. Sets  pi->interf->noise.bphy_desense in side the func */
	/* Should be run only if associated */
	if (!PUB_NOT_ASSOC(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {

		glitch_badplcp_sum = pi->interf->noise.bphy_glitch_ma +
			pi->interf->noise.bphy_badplcp_ma;

		bphy_desense_delta = wlc_phy_cmn_noisemode_glitch_chk_adj(pi, glitch_badplcp_sum,
			&pi->interf->noise.bphy_thres);

		pi->interf->noise.bphy_desense += bphy_desense_delta;
	} else {
		pi->interf->noise.bphy_desense = 0;
	}

	glitch_badplcp_sum = pi->interf->noise.ofdm_glitch_ma + pi->interf->noise.ofdm_badplcp_ma;
	/* OFDM desense. Sets  pi->interf->noise.ofdm_desense in side the func */
	ofdm_desense_delta = wlc_phy_cmn_noisemode_glitch_chk_adj(pi, glitch_badplcp_sum,
		&pi->interf->noise.ofdm_thres);
	pi->interf->noise.ofdm_desense += ofdm_desense_delta;

	wlc_phy_cmn_noise_limit_desense(pi);

	/* XXX This function utilizes bphy_desense, ofdm_desense
	  * and RSSI (if setting exact OFDM and BPHY settings is not possible).
	  * This function finds out the best settings for BPHY (0x2e6, 0xc33, IG)
	  * and OFDM(0x280 and IG), that can best set the desense.
	  * Also, call this function if some desense is required.
	  * if both OFDM, BPHY desense == 0, then never mind calling the function.
	  * call the lower function only if there is any new desense to be done.
	  */

	if (bphy_desense_delta || ofdm_desense_delta) {
		if (ISNPHY(pi)) {
			wlc_phy_bphy_ofdm_noise_hw_set_nphy(pi);
		}
	}

	PHY_ACI(("PHY_CMN:  {GL_MA, BP_MA} OFDM {%d, %d},"
	         " BPHY {%d, %d}, Desense - (OFDM, BPHY) = {%d, %d} MAX_Desense {%d, %d},"
	         "channel is %d rssi = %d\n",
	         pi->interf->noise.ofdm_glitch_ma, pi->interf->noise.ofdm_badplcp_ma,
	         pi->interf->noise.bphy_glitch_ma, pi->interf->noise.bphy_badplcp_ma,
	         pi->interf->noise.ofdm_desense, pi->interf->noise.bphy_desense,
	         pi->interf->noise.max_poss_ofdm_desense, pi->interf->noise.max_poss_bphy_desense,
	         CHSPEC_CHANNEL(pi->radio_chanspec), pi->interf->rssi));

}

static int8
wlc_phy_cmn_noisemode_glitch_chk_adj(phy_info_t *pi, uint16 total_glitch_badplcp,
	noise_thresholds_t *thresholds)
{
	int8 desense_delta = 0;

	if (total_glitch_badplcp > thresholds->glitch_badplcp_low_th) {
		/* glitch count is high, could be due to inband noise */
		thresholds->high_detect_total++;
		thresholds->low_detect_total = 0;
	} else {
		/* glitch count not high */
		thresholds->high_detect_total = 0;
		thresholds->low_detect_total++;
	}

	if (thresholds->high_detect_total >= thresholds->high_detect_thresh) {
		/* we have more than glitch_th_up bphy
		 * glitches in a row. so, let's try raising the
		 * inband noise immunity
		 */
		if (total_glitch_badplcp < thresholds->glitch_badplcp_high_th) {
			/* Desense by less */
			desense_delta = thresholds->desense_lo_step;
		} else {
			/* Desense by more */
			desense_delta = thresholds->desense_hi_step;
		}
		thresholds->high_detect_total = 0;
	} else if (thresholds->low_detect_total > 0) {
		/* check to see if we can lower noise immunity */
		uint16 low_detect_total, undesense_wait, undesense_window;

		low_detect_total = thresholds->low_detect_total;
		undesense_wait = thresholds->undesense_wait;
		undesense_window = thresholds->undesense_window;

		/* Reduce the wait time to undesense if glitch count has been low for longer */
		while (undesense_wait > 1) {
			if (low_detect_total <=  undesense_window) {
				break;
			}
			low_detect_total -= undesense_window;
			/* Halve the wait time */
			undesense_wait /= 2;
		}

		if ((low_detect_total % undesense_wait) == 0) {
			/* Undesense */
			desense_delta = -1;
		}
	}
	PHY_ACI(("In %s: recomended desense = %d glitch_ma + badplcp_ma = %d,"
		"th_lo = %d, th_hi = %d \n", __FUNCTION__, desense_delta,
		total_glitch_badplcp, thresholds->glitch_badplcp_low_th,
		thresholds->glitch_badplcp_high_th));
	return desense_delta;
}

void
wlc_phy_cmn_noise_limit_desense(phy_info_t *pi)
{
	if (pi->interf->rssi != 0) {
		int8 max_desense_rssi = PHY_NOISE_DESENSE_RSSI_MAX;
		int8 desense_margin = PHY_NOISE_DESENSE_RSSI_MARGIN;

		int8 max_desense_dBm, bphy_desense_dB, ofdm_desense_dB;

		pi->interf->noise.bphy_desense = MAX(pi->interf->noise.bphy_desense, 0);
		pi->interf->noise.ofdm_desense = MAX(pi->interf->noise.ofdm_desense, 0);

		max_desense_dBm = MIN(pi->interf->rssi, max_desense_rssi) - desense_margin;

		bphy_desense_dB = max_desense_dBm - pi->interf->bphy_min_sensitivity;
		ofdm_desense_dB = max_desense_dBm - pi->interf->ofdm_min_sensitivity;

		bphy_desense_dB = MAX(bphy_desense_dB, 0);
		ofdm_desense_dB = MAX(ofdm_desense_dB, 0);

		pi->interf->noise.max_poss_bphy_desense = bphy_desense_dB;
		pi->interf->noise.max_poss_ofdm_desense = ofdm_desense_dB;

		pi->interf->noise.bphy_desense = MIN(bphy_desense_dB,
			pi->interf->noise.bphy_desense);
		pi->interf->noise.ofdm_desense = MIN(ofdm_desense_dB,
			pi->interf->noise.ofdm_desense);
	} else {
		pi->interf->noise.max_poss_bphy_desense = 0;
		pi->interf->noise.max_poss_ofdm_desense = 0;

		pi->interf->noise.bphy_desense = 0;
		pi->interf->noise.ofdm_desense = 0;
	}
}

void
wlc_phy_interf_rssi_update(wlc_phy_t *pih, chanspec_t chanspec, int8 leastRSSI)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if ((NREV_GE(pi->pubpi.phy_rev, 6) && ISNPHY(pi)) || ISHTPHY(pi)) {
	    if ((CHSPEC_CHANNEL(chanspec) == pi->interf->curr_home_channel)) {
	        pi->u.pi_nphy->intf_rssi_avg = leastRSSI;
	        pi->interf->rssi = leastRSSI;
	    }
	}

	/* Doing interference update here */
	if (ISACPHY(pi))
		wlc_phy_desense_aci_upd_chan_stats_acphy(pi, chanspec, leastRSSI);
}

void
wlc_phy_interf_chan_stats_update(wlc_phy_t *ppi, chanspec_t chanspec, uint32 crsglitch,
	uint32 bphy_crsglitch, uint32 badplcp, uint32 bphy_badplcp,
	uint8 txop, uint32 mbsstime)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	wlc_phy_desense_aci_upd_txop_acphy(pi, chanspec, txop);
	/* Doing interference update of chan stats here  */

	if (pi->interf->curr_home_channel == (CHSPEC_CHANNEL(chanspec))) {
		pi->interf->cca_stats_func_called = TRUE;
		pi->interf->cca_stats_total_glitch = crsglitch;
		pi->interf->cca_stats_bphy_glitch = bphy_crsglitch;
		pi->interf->cca_stats_total_badplcp = badplcp;
		pi->interf->cca_stats_bphy_badplcp = bphy_badplcp;
		pi->interf->cca_stats_mbsstime = mbsstime;
	}
}

#endif /* Compiling out ACI code for 4324 */

void
wlc_phy_acimode_noisemode_reset(wlc_phy_t *pih, chanspec_t chanspec,
	bool clear_aci_state, bool clear_noise_state, bool disassoc)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint channel = CHSPEC_CHANNEL(chanspec);

	pi->interf->cca_stats_func_called = FALSE;

	if (!((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi)))
		return;

	if (pi->sh->interference_mode_override == TRUE)
		return;

	PHY_TRACE(("%s: CurCh %d HomeCh %d disassoc %d\n",
		__FUNCTION__, channel,
		pi->interf->curr_home_channel, disassoc));

	if ((disassoc) ||
		((channel != pi->interf->curr_home_channel) &&
		(disassoc == FALSE))) {
		/* not home channel... reset */
		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
#ifndef WLC_DISABLE_ACI
			wlc_phy_aci_noise_reset_nphy(pi, channel,
				clear_aci_state, clear_noise_state, disassoc);
#endif /* Compiling out ACI code for 4324 */
		} else if (ISHTPHY(pi)) {
			wlc_phy_aci_noise_reset_htphy(pi, channel,
				clear_aci_state, clear_noise_state, disassoc);
		}
	}
}

static void
wlc_phy_noise_calc(phy_info_t *pi, uint32 *cmplx_pwr, int8 *pwr_ant, uint8 extra_gain_1dB)
{
	int8 cmplx_pwr_dbm[PHY_CORE_MAX];
	uint8 i;
	uint16 gain;

	bzero((uint8 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));
	ASSERT(PHYCORENUM(pi->pubpi.phy_corenum) <= PHY_CORE_MAX);

	gain = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_BLKS+0xC);
	PHY_INFORM(("--> RXGAIN: %d\n", gain));

	phy_utils_computedB(cmplx_pwr, cmplx_pwr_dbm, PHYCORENUM(pi->pubpi.phy_corenum));
	FOREACH_CORE(pi, i) {
	  if (ISACPHY(pi)) {
		/* init gain is fixed to -65 for acphy,
		 *-37 = 10*log10((0.4/512)*(0.4/512)*(16)*(1/50.0))
		 */

		/* knoise support, read gain value from shm, by Peyush */
		int16 assumed_gain;

		if (BFCTL(pi->u.pi_acphy) == 2) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_2G);
			} else {
			  assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_5G);
			}
		} else {
			assumed_gain = (int16)(ACPHY_NOISE_INITGAIN);
		}

		assumed_gain += extra_gain_1dB;

		if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
		  cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM_10BIT - assumed_gain));
		} else {
		  cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM - assumed_gain));
		}
		pi->u.pi_acphy->phy_noise_all_core[i] =  cmplx_pwr_dbm[i] + (assumed_gain);
	  }
	  else
	if (CHIPID_4324X_EPA_FAMILY(pi)) {
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			gain = pi_nphy->iqestgain;
		 cmplx_pwr_dbm[i] += (int8) (NPHY_NOISE_SAMPLEPWR_TO_DBM - gain);
	} else if (NREV_GE(pi->pubpi.phy_rev, 3)) {
		 cmplx_pwr_dbm[i] += (int8) (PHY_NOISE_OFFSETFACT_4322 - gain);
	}  else if (NREV_LT(pi->pubpi.phy_rev, 3))
			/* assume init gain 70 dB, 128 maps to 1V so
			 * 10*log10(128^2*2/128/128/50)+30=16 dBm
			 * WARNING: if the nphy init gain is ever changed,
			 * this formula needs to be updated
			*/
			cmplx_pwr_dbm[i] += (int8)(16 - (15) * 3 - (70 + extra_gain_1dB));
#if defined(WLTEST)
		else if (ISLCNPHY(pi) && LCNREV_GE(pi->pubpi.phy_rev, 2)) {
			int16 noise_offset_fact;
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			wlc_phy_get_noiseoffset_lcnphy(pi, &noise_offset_fact);
			cmplx_pwr_dbm[i] +=
				(int8)(noise_offset_fact - pi_lcn->rxpath_gain);
		} else if (ISLCN40PHY(pi)) {
			int16 noise_offset_fact;
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			wlc_phy_get_noiseoffset_lcn40phy(pi, &noise_offset_fact);
			cmplx_pwr_dbm[i] +=
				(int8) ((noise_offset_fact - (pi_lcn->rxpath_gain >> 2)));
		}
#endif // endif
		pwr_ant[i] = cmplx_pwr_dbm[i];
		PHY_INFORM(("wlc_phy_noise_calc_phy: pwr_ant[%d] = %d\n", i, pwr_ant[i]));
	}

	PHY_INFORM(("%s: samples %d ant %d\n", __FUNCTION__, pi->phy_rxiq_samps,
		pi->phy_rxiq_antsel));
}

static void
wlc_phy_noise_calc_fine_resln(phy_info_t *pi, uint32 *cmplx_pwr, int16 *pwr_ant,
                              uint8 extra_gain_1dB, int16 *tot_gain)
{
	int16 cmplx_pwr_dbm[PHY_CORE_MAX];
	uint8 i;
	uint16 gain;
	int8 offset[] = {0, 0};

	/* lookup table for computing the dB contribution from the first
	 * 4 bits after MSB (most significant NONZERO bit) in cmplx_pwr[core]
	 * (entries in multiples of 0.25dB):
	 */
	uint8 dB_LUT[] = {0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 8, 9,
		10, 10, 11, 11};
	uint8 LUT_correction[] = {13, 12, 12, 13, 16, 20, 25,
		5, 12, 19, 2, 11, 20, 5, 15, 1};

	bzero((uint16 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));
	ASSERT(PHYCORENUM(pi->pubpi.phy_corenum) <= PHY_CORE_MAX);

	/* Convert sample-power to dB scale: */
	FOREACH_CORE(pi, i) {
		uint8 shift_ct, lsb, msb_loc;
		uint8 msb2345 = 0x0;
		uint32 tmp;
		tmp = cmplx_pwr[i];
		shift_ct = msb_loc = 0;
		while (tmp != 0) {
			tmp = tmp >> 1;
			shift_ct++;
			lsb = (uint8)(tmp & 1);
			if (lsb == 1)
				msb_loc = shift_ct;
		}

		/* Store first 4 bits after MSB: */
		if (msb_loc <= 4) {
			msb2345 = (cmplx_pwr[i] << (4-msb_loc)) & 0xf;
		} else {
			/* Need to first round cmplx_pwr to 5 MSBs: */
			tmp = cmplx_pwr[i] + (1U << (msb_loc-5));
			/* Check if MSB has shifted in the process: */
			if (tmp & (1U << (msb_loc+1))) {
				msb_loc++;
			}
			msb2345 = (tmp >> (msb_loc-4)) & 0xf;
		}

		/* Power in 0.25 dB steps: */
		cmplx_pwr_dbm[i] = ((3*msb_loc) << 2) + dB_LUT[msb2345];

		/* Apply a possible +0.25dB (1 step) correction depending
		 * on MSB location in cmplx_pwr[core]:
		 */
		cmplx_pwr_dbm[i] += (int16)((msb_loc >= LUT_correction[msb2345]) ? 1 : 0);
	}

	if (ISHTPHY(pi)) {
		int16 assumed_gain;
		wlc_phy_get_rxiqest_gain_htphy(pi, &assumed_gain);
		assumed_gain += extra_gain_1dB;

		FOREACH_CORE(pi, i) {
			/* Convert to analog input power at ADC and then
			 * backoff applied gain to get antenna input power:
			 */

			/* XXX
			 * FIXME:
			 * Need to have better way of arriving at assumed_gain
			 * than having #define's for different boards
			 */

			/* scale conversion factor by 4 as power is in 0.25dB steps */
			cmplx_pwr_dbm[i] += (int16) ((HTPHY_NOISE_SAMPLEPWR_TO_DBM -
			                              assumed_gain) << 2);
			pwr_ant[i] = cmplx_pwr_dbm[i];
			PHY_INFORM(("wlc_phy_noise_calc_fine_resln: pwr_ant[%d] = %d\n",
				i, pwr_ant[i]));
		}
	} else if (ISACPHY(pi)) {
		int16 assumed_gain;

		FOREACH_CORE(pi, i) {
			assumed_gain = tot_gain[i];

			if (assumed_gain == 0) {
				if (BFCTL(pi->u.pi_acphy) == 2) {
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_2G);
					} else {
						assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_5G);
					}
				} else {
					assumed_gain = (int16)(ACPHY_NOISE_INITGAIN);
				}
			}

			assumed_gain += extra_gain_1dB;

			/* Convert to analog input power at ADC and then
			 * backoff applied gain to get antenna input power:
			 */

			/* XXX
			 * FIXME:
			 * Need to have better way of arriving at assumed_gain
			 * than having #define's for different boards
			 */

			/* scale conversion factor by 4 as power is in 0.25dB steps */
			if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
			  cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM_10BIT -
			                              assumed_gain) << 2);
			} else {
			  cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM -
			                              assumed_gain) << 2);
			}
			pwr_ant[i] = cmplx_pwr_dbm[i];
			PHY_RXIQ(("In %s: pwr_ant[%d] = %d\n", __FUNCTION__,
				i, pwr_ant[i]));
		}

#if defined(WLTEST)
	}
	else if (ISLCNPHY(pi) && LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		int16 noise_offset_fact;
		int8 freq_offset_fact;
		phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
		wlc_phy_get_noiseoffset_lcnphy(pi, &noise_offset_fact);
		FOREACH_CORE(pi, i) {
			cmplx_pwr_dbm[i] +=
				(int16) ((noise_offset_fact - pi_lcn->rxpath_gain) << 2);
			pwr_ant[i] = cmplx_pwr_dbm[i];
			/* Frequency based variation correction */
			wlc_lcnphy_get_lna_freq_correction(pi, &freq_offset_fact);
			pwr_ant[i] = pwr_ant[i] - freq_offset_fact;
		}
	} else if (ISLCN40PHY(pi)) {
		int16 noise_offset_fact;
		int8 freq_offset_fact;
		phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
		wlc_phy_get_noiseoffset_lcn40phy(pi, &noise_offset_fact);
		FOREACH_CORE(pi, i) {
			cmplx_pwr_dbm[i] +=
				(int16) ((noise_offset_fact << 2) - pi_lcn->rxpath_gain);
			pwr_ant[i] = cmplx_pwr_dbm[i];
			/* Frequency based variation correction */
			wlc_lcn40phy_get_lna_freq_correction(pi, &freq_offset_fact);
			pwr_ant[i] = pwr_ant[i] - freq_offset_fact;
		}
#endif /* #if defined(WLTEST) */
	} else if (ISNPHY(pi) &&
		(NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 6))) {
		FOREACH_CORE(pi, i) {
			/* Convert to analog input power at ADC and then
			 * backoff applied gain to get antenna input power:
			 */
			cmplx_pwr_dbm[i] += (int16) ((NPHY_NOISE_SAMPLEPWR_TO_DBM -
				NPHY_NOISE_INITGAIN) << 2);
			/* scale conversion factor by 4 as power is in 0.25dB steps */
			pwr_ant[i] = cmplx_pwr_dbm[i];
			PHY_INFORM(("wlc_phy_noise_calc_fine_resln: pwr_ant[%d] = %d\n",
				i, pwr_ant[i]));
		}
	} else if (ISNPHY(pi) && CHIPID_4324X_EPA_FAMILY(pi)) {
			/* Adding fine resolution option for 4324B1,B3,B5 */
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			gain = pi_nphy->iqestgain;
			wlc_phy_rxgain_index_offset(pi, offset);
		FOREACH_CORE(pi, i) {
			cmplx_pwr_dbm[i] += (int16) ((NPHY_NOISE_SAMPLEPWR_TO_DBM - gain) << 2);
			cmplx_pwr_dbm[i] -= (offset[i]);
			pwr_ant[i] = cmplx_pwr_dbm[i];
			PHY_INFORM(("wlc_phy_noise_calc_fine_resln: pwr_ant[%d] = %d\n",
				i, pwr_ant[i]));
		}
	} else {
		FOREACH_CORE(pi, i) {
			/* assume init gain 70 dB, 128 maps to 1V so
			 * 10*log10(128^2*2/128/128/50)+30=16 dBm
			 *  WARNING: if the nphy init gain is ever changed,
			 * this formula needs to be updated
			 */
			cmplx_pwr_dbm[i] += ((int16)(16 << 2) - (int16)((15 << 2)*3)
			                     - (int16)((70 + extra_gain_1dB) << 2));
			pwr_ant[i] = cmplx_pwr_dbm[i];
		}
	}

}

static void
wlc_phy_noise_save(phy_info_t *pi, int8 *noise_dbm_ant, int8 *max_noise_dbm)
{
	uint8 i;

	FOREACH_CORE(pi, i) {
		/* save noise per core */
		pi->phy_noise_win[i][pi->phy_noise_index] = noise_dbm_ant[i];

		/* save the MAX for all cores */
		if (noise_dbm_ant[i] > *max_noise_dbm)
			*max_noise_dbm = noise_dbm_ant[i];
	}
	pi->phy_noise_index = MODINC_POW2(pi->phy_noise_index, PHY_NOISE_WINDOW_SZ);

	pi->sh->phy_noise_window[pi->sh->phy_noise_index] = *max_noise_dbm;
	pi->sh->phy_noise_index = MODINC_POW2(pi->sh->phy_noise_index, MA_WINDOW_SZ);
}

static uint8 wlc_phy_calc_extra_init_gain(phy_info_t *pi, uint8 extra_gain_3dB,
                                         rxgain_t rxgain[])
{
	uint16 init_gain_code[4];
	uint8 core, MAX_DVGA, MAX_LPF, MAX_MIX;
	uint8 dvga, mix, lpf0, lpf1;
	uint8 dvga_inc, lpf0_inc, lpf1_inc;
	uint8 max_inc, gain_ticks = extra_gain_3dB;

	if (ISHTPHY(pi)) {
		MAX_DVGA = 4; MAX_LPF = 10; MAX_MIX = 4;
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 3, 0x106, 16, &init_gain_code);

		/* Find if the requested gain increase is possible */
		FOREACH_CORE(pi, core) {
			dvga = 0;
			mix = (init_gain_code[core] >> 4) & 0xf;
			lpf0 = (init_gain_code[core] >> 8) & 0xf;
			lpf1 = (init_gain_code[core] >> 12) & 0xf;
			max_inc = MAX(0, MAX_DVGA - dvga) + MAX(0, MAX_LPF - lpf0 - lpf1) +
			        MAX(0, MAX_MIX - mix);
			gain_ticks = MIN(gain_ticks, max_inc);
		}
		if (gain_ticks != extra_gain_3dB) {
			PHY_INFORM(("%s: Unable to find enough extra gain. Using extra_gain = %d\n",
			            __FUNCTION__, 3 * gain_ticks));
		}

		/* Do nothing if no gain increase is required/possible */
		if (gain_ticks == 0) {
			return gain_ticks;
		}

		/* Find the mix, lpf0, lpf1 gains required for extra INITgain */
		FOREACH_CORE(pi, core) {
			uint8 gain_inc = gain_ticks;

			dvga = 0;
			mix = (init_gain_code[core] >> 4) & 0xf;
			lpf0 = (init_gain_code[core] >> 8) & 0xf;
			lpf1 = (init_gain_code[core] >> 12) & 0xf;

			dvga_inc = MIN((uint8) MAX(0, MAX_DVGA - dvga), gain_inc);
			dvga += dvga_inc;
			gain_inc -= dvga_inc;

			lpf1_inc = MIN((uint8) MAX(0, MAX_LPF - lpf1 - lpf0), gain_inc);
			lpf1 += lpf1_inc;
			gain_inc -= lpf1_inc;

			lpf0_inc = MIN((uint8) MAX(0, MAX_LPF - lpf1 - lpf0), gain_inc);
			lpf0 += lpf0_inc;
			gain_inc -= lpf0_inc;

			mix += MIN((uint8) MAX(0, MAX_MIX - mix), gain_inc);

			rxgain[core].lna1 = init_gain_code[core] & 0x3;
			rxgain[core].lna2 = (init_gain_code[core] >> 2) & 0x3;
			rxgain[core].mix  = mix;
			rxgain[core].lpf0 = lpf0;
			rxgain[core].lpf1 = lpf1;
			rxgain[core].dvga = dvga;
		}
	}
	else if (ISACPHY(pi)) {
		gain_ticks = wlc_phy_calc_extra_init_gain_acphy(pi, extra_gain_3dB, rxgain);
	} else {
		PHY_ERROR(("%s: Extra INITgain not supported\n", __FUNCTION__));
		return 0;
	}

	return gain_ticks;
}

static uint32
wlc_phy_rx_iq_est(phy_info_t *pi, uint8 samples, uint8 antsel, uint8 resolution, uint8 lpf_hpc,
                  uint8 dig_lpf, uint8 gain_correct, uint8 extra_gain_3dB, uint8 wait_for_crs,
                  uint8 force_gain_type, int16 *iqest)
{
	phy_iq_est_t est[PHY_CORE_MAX];
	uint32 cmplx_pwr[PHY_CORE_MAX];
	int8 noise_dbm_ant[PHY_CORE_MAX];
	int16	tot_gain[PHY_CORE_MAX];
	int16 noise_dBm_ant_fine[PHY_CORE_MAX];
	uint16 log_num_samps, num_samps;
	uint8 wait_time = 32;
	uint8 chans[NUM_CHANS_IN_CHAN_BONDING];
	bool sampling_in_progress = (pi->phynoise_state != 0);
	uint8 i, extra_gain_1dB = 0;
	uint32 result = 0;

	if ((sampling_in_progress) && (!ISLCNPHY(pi))) {
		PHY_ERROR(("%s: sampling_in_progress\n", __FUNCTION__));
		return 0;
	}

	/* Extra INITgain is supported only for HTPHY currently */
	if (extra_gain_3dB > 0) {
	  if (!ISHTPHY(pi)&&!ISACPHY(pi)) {
			extra_gain_3dB = 0;
			PHY_ERROR(("%s: Extra INITgain not supported for this phy.\n",
			           __FUNCTION__));
		}
	}

	pi->phynoise_state |= PHY_NOISE_STATE_MON;
	/* choose num_samps to be some power of 2 */
	log_num_samps = samples;
	num_samps = 1 << log_num_samps;

	bzero((uint8 *)est, sizeof(est));
	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));
	bzero((uint16 *)noise_dBm_ant_fine, sizeof(noise_dBm_ant_fine));
	bzero((int16 *)tot_gain, sizeof(tot_gain));

	/* get IQ power measurements */
	if (ISNPHY(pi)) {
		uint16  phy_clip_state[2];
		uint16 clip_off[] = {0xffff, 0xffff};
		uint16 classif_state = 0;

		if (NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 6)) {
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);
			if (lpf_hpc) {
				/* Override the LPF high pass corners to their
				 * lowest values (0x1)
				 */
				wlc_phy_lpf_hpc_override_nphy(pi, TRUE);
			}
		} else {
			classif_state = wlc_phy_classifier_nphy(pi, 0, 0);
			wlc_phy_classifier_nphy(pi, 3, 0);
			wlc_phy_clip_det_nphy(pi, 0, &phy_clip_state[0]);
			wlc_phy_clip_det_nphy(pi, 1, clip_off);
		}

		/* get IQ power measurements */
		wlc_phy_rx_iq_est_nphy(pi, est, num_samps, wait_time, wait_for_crs);

		if (NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 6)) {
			if (lpf_hpc) {
				/* Restore LPF high pass corners to their original values */
				wlc_phy_lpf_hpc_override_nphy(pi, FALSE);
			}
			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
		} else {
			wlc_phy_clip_det_nphy(pi, 1, &phy_clip_state[0]);
			/* restore classifier settings and reenable MAC ASAP */
			wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
				classif_state);
		}
	} else if (ISHTPHY(pi)) {
		rxgain_t rxgain[PHY_CORE_MAX];
		rxgain_ovrd_t rxgain_ovrd[PHY_CORE_MAX];

		wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
		if (lpf_hpc) {
			/* Override the LPF high pass corners to their lowest values (0x1) */
			wlc_phy_lpf_hpc_override_htphy(pi, TRUE);
		}

		/* Overide the digital LPF */
		if (dig_lpf) {
			wlc_phy_dig_lpf_override_htphy(pi, dig_lpf);
		}

		/* Increase INITgain if requested */
		if (extra_gain_3dB > 0) {
			extra_gain_3dB = wlc_phy_calc_extra_init_gain(pi, extra_gain_3dB, rxgain);

			/* Override higher INITgain if possible */
			if (extra_gain_3dB > 0) {
				wlc_phy_rfctrl_override_rxgain_htphy(pi, 0, rxgain, rxgain_ovrd);
			}
		}

		/* get IQ power measurements */
		wlc_phy_rx_iq_est_htphy(pi, est, num_samps, wait_time, wait_for_crs);

		/* Disable the overrides if they were set */
		if (extra_gain_3dB > 0) {
			wlc_phy_rfctrl_override_rxgain_htphy(pi, 1, NULL, rxgain_ovrd);
		}

		if (lpf_hpc) {
			/* Restore LPF high pass corners to their original values */
			wlc_phy_lpf_hpc_override_htphy(pi, FALSE);
		}

		/* Restore the digital LPF */
		if (dig_lpf) {
			wlc_phy_dig_lpf_override_htphy(pi, 0);
		}
		wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
	} else if (ISACPHY(pi)) {
		rxgain_t rxgain[PHY_CORE_MAX];
		rxgain_ovrd_t rxgain_ovrd[PHY_CORE_MAX];
		phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

		wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

		if (force_gain_type != 0) {
			wlc_phy_get_rxgain_acphy(pi, rxgain, tot_gain, force_gain_type);
			wlc_phy_rfctrl_override_rxgain_acphy(pi, 0, rxgain, rxgain_ovrd);
			PHY_RXIQ(("In %s: For gain type = %d | Total gain being applied = %d \n",
				__FUNCTION__, force_gain_type, tot_gain[0]));
		} else {
			PHY_RXIQ(("In %s: For gain type = %d | Total gain being applied = %d \n",
				__FUNCTION__, force_gain_type, ACPHY_NOISE_INITGAIN));
		}

		if (lpf_hpc) {
			/* Override the LPF high pass corners to their lowest values (0x1) */
			phy_ac_lpf_hpc_override(pi_ac, TRUE);
		}
		/* Overide the digital LPF */
		if (!TINY_RADIO(pi) && dig_lpf) {
			wlc_phy_dig_lpf_override_acphy(pi, dig_lpf);
		}

		/* Increase INITgain if requested */
		if (extra_gain_3dB > 0) {
			extra_gain_3dB = wlc_phy_calc_extra_init_gain(pi, extra_gain_3dB, rxgain);

			/* Override higher INITgain if possible */
			if (extra_gain_3dB > 0) {
				wlc_phy_rfctrl_override_rxgain_acphy(pi, 0, rxgain, rxgain_ovrd);
			}
		}

		/* get IQ power measurements */
		wlc_phy_rx_iq_est_acphy(pi, est, num_samps, wait_time, wait_for_crs, FALSE);
		/* Disable the overrides if they were set */
		if (extra_gain_3dB > 0) {
			wlc_phy_rfctrl_override_rxgain_acphy(pi, 1, NULL, rxgain_ovrd);
		}
		if (lpf_hpc) {
			/* Restore LPF high pass corners to their original values */
			phy_ac_lpf_hpc_override(pi_ac, FALSE);
		}
		/* Restore the digital LPF */
		if (!TINY_RADIO(pi) && dig_lpf) {
			wlc_phy_dig_lpf_override_acphy(pi, 0);
		}

		if (force_gain_type != 0) {
			if ((force_gain_type == 4) ||
				((force_gain_type == 3) && (pi_ac->mdgain_trtx_allowed))) {
				wlc_phy_get_rxgain_acphy(pi, rxgain, tot_gain, 6);
			}
			wlc_phy_rfctrl_override_rxgain_acphy(pi, 1, NULL, rxgain_ovrd);
		}

		wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
	} else if (ISLCNPHY(pi)) {
#if defined(WLTEST)
		uint8 save_antsel;
		phy_antdiv_get_rx(pi, &save_antsel);
		if (antsel <= 1)
			phy_antdiv_set_rx(pi, antsel);
		wlc_lcnphy_rx_power(pi, num_samps, wait_time, wait_for_crs, est);
		phy_antdiv_set_rx(pi, save_antsel);
#else
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		return 0;
#endif // endif
	} else if (ISLCN40PHY(pi)) {
#if defined(WLTEST)
		uint8 save_antsel;
		phy_antdiv_get_rx(pi, &save_antsel);
		if (antsel <= 1)
			phy_antdiv_set_rx(pi, antsel);
		wlc_lcn40phy_rx_power(pi, num_samps, wait_time, wait_for_crs, est);
		phy_antdiv_set_rx(pi, save_antsel);
#else
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		return 0;
#endif // endif
	}

	/* sum I and Q powers for each core, average over num_samps with rounding */
	ASSERT(PHYCORENUM(pi->pubpi.phy_corenum) <= PHY_CORE_MAX);
	FOREACH_CORE(pi, i) {
		cmplx_pwr[i] = ((est[i].i_pwr + est[i].q_pwr) +
			(1U << (log_num_samps-1))) >> log_num_samps;
	}

	/* convert in 1dB gain for gain adjustment */
	extra_gain_1dB = 3 * extra_gain_3dB;

	if (resolution == 0) {
		/* pi->phy_noise_win per antenna is updated inside */
	  wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, extra_gain_1dB);

		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;

		for (i = PHYCORENUM(pi->pubpi.phy_corenum); i >= 1; i--)
			result = (result << 8) | (noise_dbm_ant[i-1] & 0xff);

#if CORE4 >= 4
		for (i = 0; i < PHYCORENUM(pi->pubpi.phy_corenum); i++)
		    iqest[i] = noise_dbm_ant[i];
#endif // endif
		return result;

	}
	else if (resolution == 1) {
		/* Reports power in finer resolution than 1 dB (currently 0.25 dB) */

		if (((ISHTPHY(pi)) && (PHYCORENUM(pi->pubpi.phy_corenum) == 3)) ||
			(ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 3) &&
			NREV_LE(pi->pubpi.phy_rev, 6))) || CHIPID_4324X_EPA_FAMILY(pi) ||
			(ISLCNPHY(pi) && LCNREV_GE(pi->pubpi.phy_rev, 2)) ||
			(ISLCN40PHY(pi))||(ISACPHY(pi)))  {

			phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
			int16 noisefloor;

			if ((gain_correct == 3) || (gain_correct == 4)) {
				int8 subband_idx, core, bw_idx, tmp_range, ant;
				acphy_rssioffset_t *pi_ac_rssioffset =
				        &pi_ac->sromi->rssioffset;
				/* If rssi_cal_rev is FALSE, then rssi_gain_delta is in 1 dB steps
				 * So, this has to be adj in the tot_gain which is in 1 dB steps.
				 * Can't apply 0.25 dB steps as we can't apply comp with tot_gain
				 * So, have to apply it with tempsense comp which is in 0.25dB steps
				 */
				if (ISACPHY(pi) && (pi->u.pi_acphy->rssi_cal_rev == FALSE)) {
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						bw_idx = (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
					} else {
						bw_idx = (CHSPEC_IS80(pi->radio_chanspec) ||
							PHY_AS_80P80(pi, pi->radio_chanspec))
							? 2 : (CHSPEC_IS160(pi->radio_chanspec)) ? 3
							: (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
					}

					/* Apply nvram based offset: */
					FOREACH_CORE(pi, core) {
						ant = phy_get_rsdbbrd_corenum(pi, core);
						if (CHSPEC_IS2G(pi->radio_chanspec)) {
						  tot_gain[core] -=
						    pi_ac_rssioffset
						    ->rssi_corr_gain_delta_2g[ant]
						    [ACPHY_GAIN_DELTA_ELNA_ON][bw_idx];
						  PHY_RXIQ(("In %s: rssi_gain_delta_offset for"
							    " core %d = %d (in dB) \n",
							    __FUNCTION__, core, pi_ac_rssioffset
							    ->rssi_corr_gain_delta_2g[ant]
							    [ACPHY_GAIN_DELTA_ELNA_ON][bw_idx]));
						} else {
						tmp_range =  wlc_phy_get_chan_freq_range_acphy(pi,
								pi->radio_chanspec);
						  if ((tmp_range > 0) && (tmp_range < 5)) {
						    subband_idx = tmp_range -1;
						    tot_gain[core]  -=
						      pi_ac_rssioffset
						      ->rssi_corr_gain_delta_5g[ant][0][bw_idx]
						      [subband_idx];
						    PHY_RXIQ(("In %s: rssi_gain_delta_offset for"
							      " core %d = %d (in dB) \n",
							      __FUNCTION__, core, pi_ac_rssioffset
							      ->rssi_corr_gain_delta_5g[ant]
							      [ACPHY_GAIN_DELTA_ELNA_ON][bw_idx]
							      [subband_idx]));
						  }
						}
					}
				}
			}

			wlc_phy_noise_calc_fine_resln(pi, cmplx_pwr, noise_dBm_ant_fine,
			                              extra_gain_1dB, tot_gain);

			/* This peice of code will be executed
			 *only if resolution is 1, => qdBm steps.
			 */
			if ((ISACPHY(pi)) && ((gain_correct == 4) || ((pi->u.pi_acphy->rssi_cal_rev
			     == TRUE) && (pi->u.pi_acphy->rxgaincal_rssical == TRUE)))) {
				int16	rssi_gain_delta_qdBm[PHY_CORE_MAX];
				int16 rssi_temp_delta_qdBm, curr_temp, gain_temp_slope = 0;
				FOREACH_CORE(pi, i) {
				  rssi_gain_delta_qdBm[i] = 0;
				}

				if ((gain_correct == 4 && ISACPHY(pi))) {
					/* This is absolute temp gain compensation
					 * This has to be applied only for Rudimentary AGC.
					 * For this, -g 4 option has to be given.
					 * For iqest cali and veri, we do not apply this
					 * absolute gain temp compensation.
					 */
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						gain_temp_slope = pi_ac->sromi->rxgain_tempadj_2g;
						/* 57 = 5.7 dB change in gain
						 * for 100 degrees change
						 */
					} else {
					int8 tmp_range;
					if (ACMAJORREV_33(pi->pubpi.phy_rev) &&
							PHY_AS_80P80(pi, pi->radio_chanspec)) {
						wlc_phy_get_chan_freq_range_80p80_acphy(pi,
							pi->radio_chanspec, chans);
						tmp_range = chans[0];
						// FIXME - core0/1: chans[0], core2/3 chans[1]
					} else {
						tmp_range = wlc_phy_get_chan_freq_range_acphy(pi,
						   pi->radio_chanspec);
					}

						if (tmp_range == 2) {
							gain_temp_slope =
							  pi_ac->sromi->rxgain_tempadj_5gl;
						} else if (tmp_range == 3) {
							gain_temp_slope =
							  pi_ac->sromi->rxgain_tempadj_5gml;
						} else if (tmp_range == 4) {
							gain_temp_slope =
							  pi_ac->sromi->rxgain_tempadj_5gmu;
						} else if (tmp_range == 5) {
							gain_temp_slope =
							  pi_ac->sromi->rxgain_tempadj_5gh;
						}

					}
					curr_temp = wlc_phy_tempsense_acphy(pi);

					if (curr_temp >= 0) {
						rssi_temp_delta_qdBm = (curr_temp *
							gain_temp_slope * 2 + 250)/500;
					} else {
						rssi_temp_delta_qdBm = (curr_temp *
							gain_temp_slope * 2 - 250)/500;
					}
				} else {
					/* SO, if it is here, -g 4 option was not provided.
					 * And both rssi_cal_rev and rxgaincal_rssical were TRUE.
					 * So, apply gain_cal_temp based copensation for tone
					 * calibration and verification. Hopefully for calibration
					 * the coeff are 0's and thus no compensation is applied.
					 */
					wlc_phy_tempsense_acphy(pi);
					wlc_phy_upd_gain_wrt_gain_cal_temp_phy(pi,
					  &rssi_temp_delta_qdBm);
				}

				if ((pi->u.pi_acphy->rssi_cal_rev == TRUE) &&
				    (pi->u.pi_acphy->rxgaincal_rssical == TRUE)) {
					int8 subband_idx, core, bw_idx, ant;
					acphy_rssioffset_t *pi_ac_rssioffset =
					  &pi_ac->sromi->rssioffset;
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						bw_idx = (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
					} else {
						bw_idx = (CHSPEC_IS80(pi->radio_chanspec) ||
							PHY_AS_80P80(pi, pi->radio_chanspec))
							? 2 : (CHSPEC_IS160(pi->radio_chanspec)) ? 3
							: (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
					}

					/* Apply nvram based offset: */
					FOREACH_CORE(pi, core) {
						ant = phy_get_rsdbbrd_corenum(pi, core);
						subband_idx =
						  wlc_phy_rssi_get_chan_freq_range_acphy(pi, core);
						if (CHSPEC_IS2G(pi->radio_chanspec)) {
						  rssi_gain_delta_qdBm[core] =
						    pi_ac_rssioffset
						    ->rssi_corr_gain_delta_2g_sub[ant]
						    [ACPHY_GAIN_DELTA_ELNA_ON][bw_idx][subband_idx];
						} else {
						  if ((subband_idx >= 0) && (subband_idx < 5)) {
						    rssi_gain_delta_qdBm[core] =
						      pi_ac_rssioffset
						      ->rssi_corr_gain_delta_5g_sub
						      [ant][ACPHY_GAIN_DELTA_ELNA_ON][bw_idx]
						      [subband_idx];
						  }
						}
					}
				} else {
					/* False part i.e. gain delta in 1 dB step is being applied
					 *  in if (gain_correct == 3 && gain_correct == 4)
					 */
					FOREACH_CORE(pi, i) {
						rssi_gain_delta_qdBm[i] = 0;
					}
				}
				FOREACH_CORE(pi, i) {
				  noise_dBm_ant_fine[i] += rssi_temp_delta_qdBm +
				    rssi_gain_delta_qdBm[i];
				  PHY_RXIQ(("In %s: | Core %d | temp_delta_qdBm = %d (qdB)"
					    "| gain_delta_qdBm = %d (qdB) |"
					    "RXIQEST = %d (qdB)\n", __FUNCTION__, i,
					    rssi_temp_delta_qdBm, rssi_gain_delta_qdBm[i],
					    noise_dBm_ant_fine[i]));
				}
			}

			if ((gain_correct == 1) || (gain_correct == 2)) {
				int16 gainerr[PHY_CORE_MAX];
				int16 gain_err_temp_adj;
				wlc_phy_get_rxgainerr_phy(pi, gainerr);

				/* make and apply temperature correction */
				if (ISHTPHY(pi)) {
					/* Read and (implicitly) store current temperature */
					wlc_phy_tempsense_htphy(pi);
				} else if (ISACPHY(pi)) {
					/* Read and (implicitly) store current temperature */
					wlc_phy_tempsense_acphy(pi);
				}
				wlc_phy_upd_gain_wrt_temp_phy(pi, &gain_err_temp_adj);

				FOREACH_CORE(pi, i) {
					/* gainerr is in 0.5dB units;
					 * need to convert to 0.25dB units
					 */
				    if (gain_correct == 1) {
				    gainerr[i] = gainerr[i] << 1;
					/* Apply gain correction */
					noise_dBm_ant_fine[i] -= gainerr[i];
					}
					noise_dBm_ant_fine[i] += gain_err_temp_adj;
				}
			}

			if (ISLCNPHY(pi))
				noisefloor = 4*LCNPHY_NOISE_FLOOR_20M;
			if (ISLCN40PHY(pi))
				noisefloor = (CHSPEC_IS40(pi->radio_chanspec))?
					4*LCN40PHY_NOISE_FLOOR_40M : 4*LCN40PHY_NOISE_FLOOR_20M;
			else if (ISACPHY(pi)) {
				if (CHSPEC_IS40(pi->radio_chanspec))
					noisefloor = 4*ACPHY_NOISE_FLOOR_40M;
				else if (CHSPEC_IS20(pi->radio_chanspec))
					noisefloor = 4*ACPHY_NOISE_FLOOR_20M;
				else
					noisefloor = 4*ACPHY_NOISE_FLOOR_80M;
			} else
				noisefloor = (CHSPEC_IS40(pi->radio_chanspec))?
					4*HTPHY_NOISE_FLOOR_40M : 4*HTPHY_NOISE_FLOOR_20M;

			/* DO NOT do flooring of estimate if the Chip is 4350 AND gain correct is 0.
			 * In other words,
			 * DO flooring for ALL chips other than 4350 AND
			 * DO flooring for 4350 if gain correct is done - ie, -g is 1/2/3/4.
			 */
			if (!(ACMAJORREV_2(pi->pubpi.phy_rev) &&
				ACMINORREV_1(pi) && (gain_correct == 0))) {
				FOREACH_CORE(pi, i) {
					if (noise_dBm_ant_fine[i] < noisefloor) {
					        noise_dBm_ant_fine[i] = noisefloor;
					}
				}
			}

#if CORE4 >= 4
			for (i = 0; i < PHYCORENUM(pi->pubpi.phy_corenum); i++) {
			    iqest[i] = noise_dBm_ant_fine[i];
			  }
#endif // endif

			for (i = PHYCORENUM(pi->pubpi.phy_corenum); i >= 1; i--) {
				result = (result << 10) | (noise_dBm_ant_fine[i-1] & 0x3ff);
			}

			pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
			return result;

		}
		else {
			PHY_ERROR(("%s: Fine-resolution reporting not supported\n", __FUNCTION__));
			pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
			return 0;
		}
	}

	pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
	return 0;
}

bool
wlc_phy_get_rxgainerr_phy(phy_info_t *pi, int16 *gainerr)
{
	/*
	 * Retrieves rxgain error (read from srom) for current channel;
	 * Returns TRUE if no gainerror was written to SROM, FALSE otherwise
	 */
	uint8 core;
	bool srom_isempty;

	if (ISLCNPHY(pi)) {
#if defined(WLTEST)
		wlc_phy_get_rxgainerr_lcnphy(pi, gainerr);
#endif // endif
		return 1;
	} else if (ISLCN40PHY(pi)) {
#if defined(WLTEST)
		wlc_phy_get_rxgainerr_lcn40phy(pi, gainerr);
#endif // endif
		return 1;
	} else if (!((ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 3) &&
		NREV_LE(pi->pubpi.phy_rev, 6))) || ISHTPHY(pi) || ISACPHY(pi))) {
		return 0;
	} else {
#ifdef BAND5G
		if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
			/* 5G */
			if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 48) {
				/* 5G-low: channels 36 through 48 */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->rxgainerr_5gl[core];
				}
				srom_isempty = pi->rxgainerr5gl_isempty;
			} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 64) {
				/* 5G-mid: channels 52 through 64 */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->rxgainerr_5gm[core];
				}
				srom_isempty = pi->rxgainerr5gm_isempty;
			} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 128) {
				/* 5G-high: channels 100 through 128 */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->rxgainerr_5gh[core];
				}
				srom_isempty = pi->rxgainerr5gh_isempty;
			} else {
				/* 5G-upper: channels 132 and above */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->rxgainerr_5gu[core];
				}
				srom_isempty = pi->rxgainerr5gu_isempty;
			}
		} else
#endif /* BAND5G */
		{
			/* 2G */
			FOREACH_CORE(pi, core) {
				gainerr[core] = (int16) pi->rxgainerr_2g[core];
			}
			srom_isempty = pi->rxgainerr2g_isempty;
		}
	}
	return srom_isempty;
}

#ifdef NOISE_CAL_LCNXNPHY
void wlc_phy_noise_trigger_ucode(phy_info_t *pi)
{
	/* ucode assumes these shm locations start with 0
	 * and ucode will not touch them in case of sampling expires
	 */
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP0, 0);
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP1, 0);
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP2, 0);
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP3, 0);
	if (ISHTPHY(pi)|ISACPHY(pi)) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP4, 0);
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP5, 0);
	}

	W_REG(pi->sh->osh, &pi->regs->maccommand, MCMD_BG_NOISE);

}
#endif /* NOISE_CAL_LCNXNPHY */
/* This is the unified entry point for all phy noise sampling for all PHYs.
 *   It support both sw polling and ucode interrupt noise sampling.
 *	??? how to ensure coexistence
 *	XXX need to trace PHY_NOISE_SAMPLE_CQRM case, which may be recursive
 *
 *   Polling will finish atomically. For interrupt, wlc_phy_noise_sample_intr will collect results.
 *   Both methods can coexist and requests can be concurrent. But at any given time,
 *     only one can sampling can be in progress to avoid disturbing the PHY IQest engine.
 *   wlc_phy_noise_cb is called to propagate results to all consumers, where
 *   the phynoise_state flags and channel matching ensure sampling results are not misused.
 */
void
wlc_phy_noise_sample_request(phy_info_t *pi, uint8 reason, uint8 ch)
{
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	bool sampling_in_progress = (pi->phynoise_state != 0);
	bool wait_for_intr = TRUE;

	PHY_NONE(("wlc_phy_noise_sample_request: state %d reason %d, channel %d\n",
	          pi->phynoise_state, reason, ch));

	/* This is needed to make sure that the crsmin cal happens
	*   even if sampling is in progress
	*/
	if (ISACPHY(pi)) {
		pi->u.pi_acphy->trigger_crsmin_cal = FALSE;
		if (reason == PHY_NOISE_STATE_CRSMINCAL && sampling_in_progress) {
			pi->u.pi_acphy->trigger_crsmin_cal = TRUE;
		}
	}

	/* since polling is atomic, sampling_in_progress equals to interrupt sampling ongoing
	 *  In these collision cases, always yield and wait interrupt to finish, where the results
	 *  maybe be sharable if channel matches in common callback progressing.
	 */
	if (sampling_in_progress)
		return;

	switch (reason) {
	case PHY_NOISE_SAMPLE_MON:

		pi->phynoise_chan_watchdog = ch;
		pi->phynoise_state |= PHY_NOISE_STATE_MON;

		break;

	case PHY_NOISE_SAMPLE_EXTERNAL:

		pi->phynoise_state |= PHY_NOISE_STATE_EXTERNAL;
		break;

	case PHY_NOISE_SAMPLE_CRSMINCAL:

		if (ISACPHY(pi)) {
			pi->phynoise_state |= PHY_NOISE_STATE_CRSMINCAL;
			break;
		}

	default:
		ASSERT(0);
		break;
	}

	/* start test, save the timestamp to recover in case ucode gets stuck */
	pi->phynoise_now = pi->sh->now;

	/* Fixed noise, don't need to do the real measurement */
	if (pi->phy_fixed_noise) {
		if (ISNPHY(pi) || ISHTPHY(pi)) {
			uint8 i;
			FOREACH_CORE(pi, i) {
				pi->phy_noise_win[i][pi->phy_noise_index] =
					PHY_NOISE_FIXED_VAL_NPHY;
			}
			pi->phy_noise_index = MODINC_POW2(pi->phy_noise_index,
				PHY_NOISE_WINDOW_SZ);
			/* legacy noise is the max of antennas */
			noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
		} else {
			/* all other PHY */
			noise_dbm = PHY_NOISE_FIXED_VAL;
		}

		wait_for_intr = FALSE;
		goto done;
	}

	if (ISLCN40PHY(pi)) {
		wlc_lcn40phy_noise_measure_start(pi, FALSE);
	} else if (ISLCNPHY(pi)) {
		/* Trigger noise cal but don't adjust anything */
		wlc_lcnphy_noise_measure_start(pi, FALSE);
	} else if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)) {
		if (!pi->phynoise_polling || (reason == PHY_NOISE_SAMPLE_EXTERNAL) ||
			(reason == PHY_NOISE_SAMPLE_CRSMINCAL)) {
			/* ucode assumes these shm locations start with 0
			 * and ucode will not touch them in case of sampling expires
			 */
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP0, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP1, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP2, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP3, 0);

			if (ISHTPHY(pi) || ISACPHY(pi)) {
				wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP4, 0);
				wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP5, 0);
			}

			W_REG(pi->sh->osh, &pi->regs->maccommand, MCMD_BG_NOISE);

		} else {
			/* polling mode */
			phy_iq_est_t est[PHY_CORE_MAX];
			uint32 cmplx_pwr[PHY_CORE_MAX];
			int8 noise_dbm_ant[PHY_CORE_MAX];
			uint16 log_num_samps, num_samps, classif_state = 0;
			uint8 wait_time = 32;
			uint8 wait_crs = 0;
			uint8 i;

			bzero((uint8 *)est, sizeof(est));
			bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
			bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));

			/* choose num_samps to be some power of 2 */
			log_num_samps = PHY_NOISE_SAMPLE_LOG_NUM_NPHY;
			num_samps = 1 << log_num_samps;

			/* suspend MAC, get classifier settings, turn it off
			 * get IQ power measurements
			 * restore classifier settings and reenable MAC ASAP
			*/
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

			if (ISNPHY(pi)) {
				classif_state = wlc_phy_classifier_nphy(pi, 0, 0);
				wlc_phy_classifier_nphy(pi, 3, 0);
				wlc_phy_rx_iq_est_nphy(pi, est, num_samps, wait_time, wait_crs);
				wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
					classif_state);
			} else if (ISHTPHY(pi)) {
				classif_state = wlc_phy_classifier_htphy(pi, 0, 0);
				wlc_phy_classifier_htphy(pi, 3, 0);
				wlc_phy_rx_iq_est_htphy(pi, est, num_samps, wait_time, wait_crs);
				wlc_phy_classifier_htphy(pi,
					HTPHY_ClassifierCtrl_classifierSel_MASK, classif_state);
			} else if (ISACPHY(pi)) {
			  classif_state = wlc_phy_classifier_acphy(pi, 0, 0);
			  wlc_phy_classifier_acphy(pi, ACPHY_ClassifierCtrl_classifierSel_MASK, 0);
			  wlc_phy_rx_iq_est_acphy(pi, est, num_samps, wait_time, wait_crs, FALSE);
				wlc_phy_classifier_acphy(pi,
					ACPHY_ClassifierCtrl_classifierSel_MASK, classif_state);
			} else {
				ASSERT(0);
			}

			wlapi_enable_mac(pi->sh->physhim);

			/* sum I and Q powers for each core, average over num_samps */
			FOREACH_CORE(pi, i)
				cmplx_pwr[i] = (est[i].i_pwr + est[i].q_pwr) >> log_num_samps;

			/* pi->phy_noise_win per antenna is updated inside */
			wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, 0);
			wlc_phy_noise_save(pi, noise_dbm_ant, &noise_dbm);
			wait_for_intr = FALSE;
		}
	} else {
		PHY_TRACE(("Fallthru nophy\n"));
		/* No phy ?? */
		return;
	}

done:
	/* if no interrupt scheduled, populate noise results now */
	if (!wait_for_intr)
		wlc_phy_noise_cb(pi, ch, noise_dbm);
}

void
wlc_phy_noise_sample_request_external(wlc_phy_t *pih)
{
	uint8  channel;

	channel = CHSPEC_CHANNEL(phy_utils_get_chanspec((phy_info_t *)pih));

	wlc_phy_noise_sample_request((phy_info_t *)pih, PHY_NOISE_SAMPLE_EXTERNAL, channel);
}

void
wlc_phy_noise_sample_request_crsmincal(wlc_phy_t *pih)
{
	uint8  channel;

	channel = CHSPEC_CHANNEL(phy_utils_get_chanspec((phy_info_t *)pih));

	wlc_phy_noise_sample_request((phy_info_t *)pih, PHY_NOISE_SAMPLE_CRSMINCAL, channel);
}

void
wlc_phy_noise_cb(phy_info_t *pi, uint8 channel, int8 noise_dbm)
{
	if (!pi->phynoise_state)
		return;

	PHY_NONE(("wlc_phy_noise_cb: state %d noise %d channel %d\n",
	          pi->phynoise_state, noise_dbm, channel));
	if (pi->phynoise_state & PHY_NOISE_STATE_MON) {
		if (pi->phynoise_chan_watchdog == channel) {
			pi->sh->phy_noise_window[pi->sh->phy_noise_index] = noise_dbm;
			pi->sh->phy_noise_index = MODINC(pi->sh->phy_noise_index, MA_WINDOW_SZ);
		}
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
	}

	if (pi->phynoise_state & PHY_NOISE_STATE_EXTERNAL) {
		pi->phynoise_state &= ~PHY_NOISE_STATE_EXTERNAL;
		wlapi_noise_cb(pi->sh->physhim, channel, noise_dbm);
	}

	if (ISACPHY(pi)) {
		if (pi->phynoise_state & PHY_NOISE_STATE_CRSMINCAL)
			pi->phynoise_state &= ~PHY_NOISE_STATE_CRSMINCAL;
	}
}

int8
wlc_phy_noise_read_shmem(phy_info_t *pi)
{
	uint32 cmplx_pwr[PHY_CORE_MAX];
	int8 noise_dbm_ant[PHY_CORE_MAX];
	uint16 lo, hi;
	uint32 cmplx_pwr_tot = 0;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	uint8 core;
	uint8 noise_during_crs = 0;
	uint8 noise_during_lte = 0;

	ASSERT(PHYCORENUM(pi->pubpi.phy_corenum) <= PHY_CORE_MAX);
	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));

	/* read SHM, reuse old corerev PWRIND since we are tight on SHM pool */
	FOREACH_CORE(pi, core) {
	        lo = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(2*core));
		hi = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(2*core+1));
		if (ISACPHY(pi)) {
		  noise_during_crs =
		    (wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP1) >> 15) & 0x1;
		  noise_during_lte =
		    (wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP1) >> 14) & 0x1;
		  if (core == 0) {
		    hi = hi & ~(3<<14); /* Clear bit 14 and 15 */
		  }
		}
		cmplx_pwr[core] = (hi << 16) + lo;
		cmplx_pwr_tot += cmplx_pwr[core];
		if (cmplx_pwr[core] == 0) {
			noise_dbm_ant[core] = PHY_NOISE_FIXED_VAL_NPHY;
		} else
			cmplx_pwr[core] >>= PHY_NOISE_SAMPLE_LOG_NUM_UCODE;
	}

#if PHY_CORE_MAX == 4
	/* FIX ME: this is temp WAR for the 4th core crsmin cal */
	cmplx_pwr[3] = cmplx_pwr[1];

#endif // endif
	if (cmplx_pwr_tot == 0)
	  PHY_INFORM(("wlc_phy_noise_sample_nphy_compute: timeout in ucode\n"));
	else {
#ifdef NOISE_CAL_LCNXNPHY
		if ((NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
			NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 2)) &&
			ISNPHY(pi)) {
			wlc_phy_noisepwr_nphy(pi, cmplx_pwr);
		}
#endif // endif
		wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, 0);
		/* Putting check here to see if crsmin cal needs to be triggered */
		if (ISACPHY(pi)) {
			if ((pi->phynoise_state == PHY_NOISE_STATE_CRSMINCAL) ||
			pi->u.pi_acphy->trigger_crsmin_cal) {
				/* Don't condition on noise_during_crs, if we do then in jammer
				   cases, noise_during_crs will always be high
				*/
				wlc_phy_crs_min_pwr_cal_acphy(pi, PHY_CRS_RUN_AUTO);
			}
		}
	}

	if (ISACPHY(pi)) {
	  if ((!noise_during_lte) && (!noise_during_crs)) {
		  wlc_phy_noise_save(pi, noise_dbm_ant, &noise_dbm);
		} else {
		  noise_dbm	= noise_dbm_ant[0];
		}
	} else {
		  wlc_phy_noise_save(pi, noise_dbm_ant,	&noise_dbm);
	}
	/* legacy noise is the max of antennas */
	return noise_dbm;

}
#ifndef WL_ACI_FCBS
#define HWACI_OFDM_DESENSE		9
#define HWACI_BPHY_DESENSE		12
#define HWACI_LNA1_DESENSE		1
#define HWACI_LNA2_DESENSE      2
#define HWACI_CLIP_INIT_DESENSE	0
#define HWACI_CLIP_HIGH_DESENSE 0
#define HWACI_CLIP_MED_DESENSE	0
#define HWACI_CLIP_LO_DESENSE	0
#endif /* WL_ACI_FCBS */

/* ucode detected ACI, apply mitigation settings */
void
wlc_phy_hwaci_mitigate_intr(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

#ifdef WL_ACI_FCBS
	if (ISACPHY(pi) && !ACPHY_ENABLE_FCBS_HWACI(pi))
#else
	if (ISACPHY(pi))
#endif // endif
	{

#ifndef WL_ACI_FCBS
		phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
		uint16 m_hwaci_st_offset =
			((wlapi_bmac_read_shm(pi->sh->physhim, M_UCODE_MACSTAT1_PTR) + 0x20) << 1);

		pi_ac->hw_aci_status = wlapi_bmac_read_shm(pi->sh->physhim, m_hwaci_st_offset) & 1;
		/* printf("HWACI Reading M_HWACI_ST = %d \n", pi_ac->hw_aci_status); */

#ifndef WLC_DISABLE_ACI
/*
	if (pi_ac->btcx_hybrid_mode_simul_rx.mode) {
			wlc_phy_hwaci_mitigate_acphy(pi, FALSE);
		}
*/

		/*
		 * Currently this feature is enabled in ucode for CoreRevIds 47 and 48. In the
		 * driver it is functional only for MajorRev3 (4345b1) and not for 4350c2
		 * (Phyrev 14).
		 */
		if ((ACMAJORREV_3(pi->pubpi.phy_rev) ||
			ACMAJORREV_4(pi->pubpi.phy_rev)) &&
			(pi->sh->interference_mode & ACPHY_HWACI_MITIGATION)) {
			if (pi_ac->aci == NULL) {
				pi_ac->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi,
					pi->radio_chanspec, TRUE);
			}

		    if (pi_ac->hw_aci_status == 0) {
				pi_ac->aci->desense.analog_gain_desense_ofdm = 0;
				pi_ac->aci->desense.analog_gain_desense_bphy = 0;
				pi_ac->aci->desense.lna1_tbl_desense = 0;
				pi_ac->aci->desense.lna2_tbl_desense = 0;
				pi_ac->aci->desense.clipgain_desense[0] = 0;
				pi_ac->aci->desense.clipgain_desense[1] = 0;
				pi_ac->aci->desense.clipgain_desense[2] = 0;
				pi_ac->aci->desense.clipgain_desense[3] = 0;
				wlc_phy_desense_apply_acphy(pi, TRUE);
		    } else {
				pi_ac->aci->desense.analog_gain_desense_ofdm = HWACI_OFDM_DESENSE;
				pi_ac->aci->desense.analog_gain_desense_bphy = HWACI_BPHY_DESENSE;
				pi_ac->aci->desense.lna1_tbl_desense = HWACI_LNA1_DESENSE;
				if (PHY_ILNA(pi))
					pi_ac->aci->desense.lna2_tbl_desense = HWACI_LNA2_DESENSE;
				else
					pi_ac->aci->desense.lna2_tbl_desense = 0;
				pi_ac->aci->desense.clipgain_desense[0] = HWACI_CLIP_INIT_DESENSE;
				pi_ac->aci->desense.clipgain_desense[1] = HWACI_CLIP_HIGH_DESENSE;
				pi_ac->aci->desense.clipgain_desense[2] = HWACI_CLIP_MED_DESENSE;
				pi_ac->aci->desense.clipgain_desense[3] = HWACI_CLIP_LO_DESENSE;
				wlc_phy_desense_apply_acphy(pi, TRUE);
		    }
		} else if ((ACMAJORREV_32(pi->pubpi.phy_rev)) &&
			((pi->sh->interference_mode & ACPHY_HWACI_MITIGATION) != 0)) {
			wlc_phy_hwaci_mitigation_acphy_tiny(pi, -1);
		}
#endif /* WLC_DISABLE_ACI */
#endif /* !WL_ACI_FCBS */

	}
}

/* ucode finished phy noise measurement and raised interrupt */
void
wlc_phy_noise_sample_intr(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint16 jssi_aux;
	uint8 channel = 0;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	if (ISLCN40PHY(pi)) {
	  wlc_lcn40phy_noise_measure(pi);
	} else if (ISLCNPHY(pi)) {
	  wlc_lcnphy_noise_measure(pi);
	} else if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)) {
		/* copy of the M_CURCHANNEL, just channel number plus 2G/5G flag */
		jssi_aux = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_AUX);
		channel = jssi_aux & D11_CURCHANNEL_MAX;
		noise_dbm = wlc_phy_noise_read_shmem(pi);
			} else {
		PHY_ERROR(("%s, unsupported phy type\n", __FUNCTION__));
		ASSERT(0);
	}

	if (!ISLCNCOMMONPHY(pi))
	  {
	    /* rssi dbm computed, invoke all callbacks */
	    wlc_phy_noise_cb(pi, channel, noise_dbm);
	  }
}

int8
wlc_phy_noise_avg(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int tot = 0;
	int i = 0;

#ifdef BCM_OL_DEV
	if (ISACPHY(pi)) {
		int j = 0;
		PHY_TRACE(("%s: Offload collecting noise sample\n", __FUNCTION__));
		wlc_phy_noise_sample_acphy(pih);
		for (i = 0; i < MA_WINDOW_SZ; i++) {
			if (pi->sh->phy_noise_window[i] != 0) {
				tot += pi->sh->phy_noise_window[i];
				++j;
			}
		}
		if (j) {
			tot /= j;
		}
		PHY_TRACE(("%s: Offload noise value %d\n", __FUNCTION__, tot));
		return (int8)tot;
	}
#endif /* BCM_OL_DEV */

#ifdef WLOFFLD
	void *wlc = wlapi_get_wlc_info(pi->sh->physhim);
	tot =  wlc_ol_noise_avg_offload(wlc); /* Read it from shared memory */
	if (tot != 0) {
		PHY_TRACE(("%s: Got noise value (%d) form offload engine\n",
			__FUNCTION__, tot));
		return (int8)tot;
	} else {
		/*
		* May be the offload ARM engine has not started yet. Get the actual sampled
		* value and fall thro' to perform average calculation as usual.
		* previously it was returning PHY_NOISE_FIXED_VAL_NPHY
		*/
		if (ISACPHY(pi)) {
			int8 current_noise_dbm;
			PHY_TRACE(("%s: No noise value found in shared memory, collecting sample\n",
			__FUNCTION__));
			/*
			* Since there is no noise average available in offload
			* there is a possibility that noise samples are filled
			* with garbage values, so fill up the entire array NOW
			*/
			current_noise_dbm = wlc_phy_noise_sample_acphy(pih);
			for (i = 0; i < MA_WINDOW_SZ; i++) {
				pi->sh->phy_noise_window[i] = current_noise_dbm;
			}
		} /* ISACPHY */
	} /* else */
#endif /* WLOFFLD */

	if (ISLCN40PHY(pi)) {
		phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
		int8 gain_adjust = 0;
		if (!pi->sh->clk)
			return 0;
		if (pi_lcn40->noise_iqest_en) {
			uint16 lcn40phyregs_shm_addr =
				2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
			int16 noise_dBm_ucode, noise_dBm;
			wlc_lcn40phy_trigger_noise_iqest(pi);
			noise_dBm_ucode = (int16)wlapi_bmac_read_shm(pi->sh->physhim,
				lcn40phyregs_shm_addr + M_NOISE_IQPWR_DB);
			if (noise_dBm_ucode)
				gain_adjust = wlc_lcn40phy_get_noise_iqest_gainadjust(pi);
			noise_dBm = noise_dBm_ucode + gain_adjust;
			tot = noise_dBm;
			PHY_INFORM(("noise_dBm=%d, noise_iq=%d, noise_dBm_ucode=%d\n",
				noise_dBm, (int16)wlapi_bmac_read_shm(pi->sh->physhim,
				lcn40phyregs_shm_addr + M_NOISE_IQPWR), noise_dBm_ucode));
		}
	} else {
		for (i = 0; i < MA_WINDOW_SZ; i++)
			tot += pi->sh->phy_noise_window[i];

		tot /= MA_WINDOW_SZ;
	}
	return (int8)tot;
}

int8
wlc_phy_noise_avg_per_antenna(wlc_phy_t *pih, int coreidx)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint8 i, idx;
	int32 tot = 0;
	int8 result = 0;

	if (!pi->sh->up)
		return 0;

	/* checking coreidx to prevent overrunning
	 * phy_noise_win array
	 */
	if (((uint)coreidx) >= PHY_CORE_MAX)
		return 0;

	IF_ACTV_CORE(pi, pi->sh->phyrxchain, coreidx) {
		tot = 0;
		idx = pi->phy_noise_index;
		for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
			tot += pi->phy_noise_win[coreidx][idx];
			idx = MODINC_POW2(idx, PHY_NOISE_WINDOW_SZ);
		}

		result = (int8)(tot/PHY_NOISE_WINDOW_SZ);
	}

	return result;
}

int8
wlc_phy_noise_lte_avg(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int tot = 0;

	if (ISLCN40PHY(pi)) {
		phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
		int8 gain_adjust = 0;
		if (!pi->sh->clk)
			return 0;
		if (pi_lcn40->noise_iqest_en) {
			uint16 lcn40phyregs_shm_addr =
				2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
			int16 noise_dBm_ucode, noise_dBm;
			wlc_lcn40phy_trigger_noise_iqest(pi);
			noise_dBm_ucode = (int16)wlapi_bmac_read_shm(pi->sh->physhim,
				lcn40phyregs_shm_addr + M_NOISE_LTE_IQPWR_DB);
			if (noise_dBm_ucode)
				gain_adjust = wlc_lcn40phy_get_noise_iqest_gainadjust(pi);
			noise_dBm = noise_dBm_ucode + gain_adjust;
			tot = noise_dBm;
			PHY_INFORM(("noise_dBm=%d, noise_iq=%d, noise_dBm_ucode=%d\n",
				noise_dBm, (int16)wlapi_bmac_read_shm(pi->sh->physhim,
				lcn40phyregs_shm_addr + M_NOISE_IQPWR), noise_dBm_ucode));
		}
	}
	return (int8)tot;
}

#define LCN40_RXSTAT0_BRD_ATTN	12
#define LCN40_RXSTAT0_ACITBL_IDX_MSB	11
#define LCN40_RX_GAIN_INDEX_MASK	0x7F00
#define LCN40_RX_GAIN_INDEX_SHIFT	8
#define LCN40_QDB_MASK	0x3
#define LCN40_QDB_SHIFT	2
#define LCN40_BIT1_QDB_POS	10
#define LCN40_BIT0_QDB_POS	13

/* Increase the loop bandwidth of the PLL in the demodulator.
 * Note that although this allows the demodulator to track the
 * received carrier frequency over a wider bandwidth, it may
 * cause the Rx sensitivity to decrease
 */
void
wlc_phy_freqtrack_start(wlc_phy_t *pih)
{
}

/* Restore the loop bandwidth of the PLL in the demodulator to the original value */
void
wlc_phy_freqtrack_end(wlc_phy_t *pih)
{
}

void
wlc_phy_set_deaf(wlc_phy_t *ppi, bool user_flag)
{
	phy_info_t *pi;
	pi = (phy_info_t*)ppi;

	if (ISLCNPHY(pi))
		wlc_lcnphy_deaf_mode(pi, TRUE);
	else if (ISLCN40PHY(pi)) {
		/* Before being deaf, Force digi_gain to 0dB */
		uint16 save_digi_gain_ovr_val;
		phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

		save_digi_gain_ovr_val =
			PHY_REG_READ(pi, LCN40PHY, radioCtrl, digi_gain_ovr_val);
		pi_lcn40->save_digi_gain_ovr =
			PHY_REG_READ(pi, LCN40PHY, radioCtrl, digi_gain_ovr);
		/* Force digi_gain to 0dB */

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr_val, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, radioCtrl, digi_gain_ovr, 1)

			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 0)
			PHY_REG_MOD_ENTRY(LCN40PHY, agcControl4, c_agc_fsm_en, 1)
		PHY_REG_LIST_EXECUTE(pi);
		OSL_DELAY(2);
		PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0);

		wlc_lcn40phy_deaf_mode(pi, TRUE);

		PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr_val,
			save_digi_gain_ovr_val);
	} else if (ISNPHY(pi))
		wlc_nphy_deaf_mode(pi, TRUE);
	else if (ISHTPHY(pi))
		wlc_phy_deaf_htphy(pi, TRUE);
	else if (ISACPHY(pi))
		wlc_phy_deaf_acphy(pi, TRUE);
	else {
		PHY_ERROR(("%s: Not yet supported\n", __FUNCTION__));
		ASSERT(0);
	}
}

void
wlc_phy_clear_deaf(wlc_phy_t  *ppi, bool user_flag)
{
	phy_info_t *pi;
	pi = (phy_info_t*)ppi;

	if (ISLCNPHY(pi))
		wlc_lcnphy_deaf_mode(pi, FALSE);
	else if (ISLCN40PHY(pi)) {
		phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

		PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr,
			pi_lcn40->save_digi_gain_ovr);
		wlc_lcn40phy_deaf_mode(pi, FALSE);
		/* Setting the saved value to default just to be safe */
		pi_lcn40->save_digi_gain_ovr = 0;
	}
	else if (ISNPHY(pi))
		wlc_nphy_deaf_mode(pi, FALSE);
	else if (ISHTPHY(pi))
		wlc_phy_deaf_htphy(pi, FALSE);
	else if (ISACPHY(pi))
		wlc_phy_deaf_acphy(pi, FALSE);
	else {
		PHY_ERROR(("%s: Not yet supported\n", __FUNCTION__));
		ASSERT(0);
	}
}

#if defined(WLTEST) || defined(AP)
static int
wlc_phy_iovar_perical_config(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr,	bool set)
{
	int err = BCME_OK;

	if (!set) {
		if (!ISNPHY(pi) && !ISHTPHY(pi) && !ISACPHY(pi) && !ISLCNPHY(pi))
			/* supported for n, ht, ac and lcn phy only */
			return BCME_UNSUPPORTED;

		*ret_int_ptr =  pi->phy_cal_mode;
	} else {
		if (!ISNPHY(pi) && !ISHTPHY(pi) && !ISACPHY(pi) && !ISLCNPHY(pi))
			/* supported for n, ht, ac and lcn phy only */
			return BCME_UNSUPPORTED;

		if (int_val == 0) {
			pi->phy_cal_mode = PHY_PERICAL_DISABLE;
		} else if (int_val == 1) {
			pi->phy_cal_mode = PHY_PERICAL_SPHASE;
		} else if (int_val == 2) {
			pi->phy_cal_mode = PHY_PERICAL_MPHASE;
		} else if (int_val == 3) {
			/* this mode is to disable normal periodic cal paths
			 *  only manual trigger(nphy_forcecal) can run it
			 */
			pi->phy_cal_mode = PHY_PERICAL_MANUAL;
		} else {
			err = BCME_RANGE;
			goto end;
		}
		wlc_phy_cal_perical_mphase_reset(pi);
	}
end:
	return err;
}
#endif // endif

#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX) || defined(ATE_BUILD) || \
	defined(BCMDBG_TEMPSENSE)
static int
wlc_phy_iovar_tempsense(phy_info_t *pi, int32 *ret_int_ptr)
{
	int err = BCME_OK;
	int32 int_val;

	*ret_int_ptr = 0;
	if (ISNPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		*ret_int_ptr = (int32)wlc_phy_tempsense_nphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	} else if (ISHTPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		*ret_int_ptr = (int32)wlc_phy_tempsense_htphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	} else if (ISACPHY(pi)) {
		/* No need to call suspend_mac and phyreg_enter since it
		* is done inside wlc_phy_tempsense_acphy
		*/
	  if (pi->radio_is_on)
	    *ret_int_ptr = (int32)wlc_phy_tempsense_acphy(pi);
	  else
	    err = BCME_RADIOOFF;
	} else if (ISLCNPHY(pi)) {
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			int_val = (int32)wlc_lcnphy_tempsense(pi, 1);
		else
			int_val = wlc_lcnphy_tempsense_degree(pi, 1);
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else if (ISLCN40PHY(pi)) {
		int_val = wlc_lcn40phy_tempsense(pi, TEMPER_VBAT_TRIGGER_NEW_MEAS);
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else
		err = BCME_UNSUPPORTED;

	return err;
}

#endif	/* BCMDBG || WLTEST || MACOSX || ATE_BUILD || BCMDBG_TEMPSENSE */

#if defined(WLTEST)
static int
wlc_phy_iovar_idletssi_reg(phy_info_t *pi, int32 *ret_int_ptr, int32 int_val, bool set)
{
	int err = BCME_OK;
	uint32 tmp;
	uint16 idle_tssi[NPHY_CORE_NUM];
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (ISLCN40PHY(pi))
		*ret_int_ptr = wlc_lcn40phy_idle_tssi_reg_iovar(pi, int_val, set, &err);
	else if (ISHTPHY(pi))
		*ret_int_ptr = wlc_phy_idletssi_get_htphy(pi);
	else if (ISNPHY(pi)) {
		if (!(CHIP_4324_B0(pi) || CHIP_4324_B4(pi))) {
			wlc_phy_lcnxn_rx2tx_stallwindow_nphy(pi, 1);
			wlc_phy_txpwrctrl_idle_tssi_nphy(pi);
			wlc_phy_lcnxn_rx2tx_stallwindow_nphy(pi, 0);
		}
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			idle_tssi[0] = (uint16)pi_nphy->nphy_pwrctrl_info[0].idle_tssi_2g;
			idle_tssi[1] = (uint16)pi_nphy->nphy_pwrctrl_info[1].idle_tssi_2g;
		} else {
			idle_tssi[0] = (uint16)pi_nphy->nphy_pwrctrl_info[0].idle_tssi_5g;
			idle_tssi[1] = (uint16)pi_nphy->nphy_pwrctrl_info[1].idle_tssi_5g;
		}
		tmp = (idle_tssi[1] << 16) | idle_tssi[0];
		*ret_int_ptr = tmp;
	}

	return err;
}

static int
wlc_phy_iovar_avgtssi_reg(phy_info_t *pi, int32 *ret_int_ptr)
{
	int err = BCME_OK;
	if (ISLCN40PHY(pi))
		*ret_int_ptr = wlc_lcn40phy_avg_tssi_reg_iovar(pi);
	else if (ISNPHY(pi)) {
		*ret_int_ptr = wlc_nphy_tssi_read_iovar(pi);
	}
	return err;
}

static int
wlc_phy_pkteng_get_gainindex(phy_info_t *pi, int32 *gain_idx)
{
	int i;

	if (D11REV_LT(pi->sh->corerev, 11))
		return BCME_UNSUPPORTED;

	if (!pi->sh->up) {
		return BCME_NOTUP;
	}

	PHY_INFORM(("wlc_phy_pkteng_get_gainindex Called\n"));

	if (ISLCNCOMMONPHY(pi)) {
		uint8 gidx[4];
		uint16 rssi_addr[4];

		uint16 lcnphyregs_shm_addr =
			2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);

		rssi_addr[0] = lcnphyregs_shm_addr + M_LCN_RSSI_0;
		rssi_addr[1] = lcnphyregs_shm_addr + M_LCN_RSSI_1;
		rssi_addr[2] = lcnphyregs_shm_addr + M_LCN_RSSI_2;
		rssi_addr[3] = lcnphyregs_shm_addr + M_LCN_RSSI_3;

		for (i = 0; i < 4; i++) {
			gidx[i] =
				(wlapi_bmac_read_shm(pi->sh->physhim, rssi_addr[i])
				& LCN40_RX_GAIN_INDEX_MASK) >> LCN40_RX_GAIN_INDEX_SHIFT;

		}
		*gain_idx = (int32)gidx[0];
	}

	return BCME_OK;
}

void
wlc_phy_pkteng_rxstats_update(wlc_phy_t *ppi, uint8 statidx)
{
	phy_info_t *pi;
	pi = (phy_info_t*)ppi;

	if (ISACPHY(pi) && ((pi->measure_hold & PHY_HOLD_FOR_PKT_ENG)))
		pi->u.pi_acphy->rxstats[statidx] += 1;
}

static int
wlc_phy_pkteng_stats_get(phy_info_t *pi, void *a, int alen)
{
	wl_pkteng_stats_t stats;
	uint16 rxstats_base;
	uint16 hi, lo;
	int i;

	if (D11REV_LT(pi->sh->corerev, 11))
		return BCME_UNSUPPORTED;

	if (!pi->sh->up) {
		return BCME_NOTUP;
	}

	PHY_INFORM(("Pkteng Stats Called\n"));

	/* Read with guard against carry */
	do {
		hi = wlapi_bmac_read_shm(pi->sh->physhim, M_PKTENG_FRMCNT_HI);
		lo = wlapi_bmac_read_shm(pi->sh->physhim, M_PKTENG_FRMCNT_LO);
	} while (hi != wlapi_bmac_read_shm(pi->sh->physhim, M_PKTENG_FRMCNT_HI));

	stats.lostfrmcnt = (hi << 16) | lo;
	stats.rssi_qdb = 0;

	if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)) {
		if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 4) &&
			(!CHIPID_4324X_MEDIA_FAMILY(pi))) {
			int16 rxpwr0, rxpwr1;
			rxpwr0 = R_REG(pi->sh->osh, &pi->regs->rssi) & 0xff;
			rxpwr1 = (R_REG(pi->sh->osh, &pi->regs->rssi) >> 8) & 0xff;
			/* Sign extend */
			if (rxpwr0 > 127)
				rxpwr0 -= 256;
			if (rxpwr1 > 127)
				rxpwr1 -= 256;
			stats.rssi = wlc_phy_swrssi_compute_nphy(pi, &rxpwr0, &rxpwr1);
		} else if (ISACPHY(pi)) {
		  #ifdef WL11AC
			 stats.rssi = (R_REG(pi->sh->osh,
			 &pi->regs->u_rcv.d11regs.rxe_phyrs_2) >> 8) & 0xff;
			if (stats.rssi > 127)
				stats.rssi -= 256;
		  #endif

			if (ACMAJORREV_2(pi->pubpi.phy_rev) &&
				(ACMINORREV_1(pi) || ACMINORREV_3(pi))) {
				stats.rssi = pi->u.pi_acphy->last_rssi;
			}
		} else {
			stats.rssi = R_REG(pi->sh->osh, &pi->regs->rssi) & 0xff;
			if (stats.rssi > 127)
				stats.rssi -= 256;
		}
		stats.snr = stats.rssi - PHY_NOISE_FIXED_VAL_NPHY;
	} else if (ISLCNCOMMONPHY(pi)) {
		int16 rssi_lcn[4];
		int16 snr_a_lcn[4];
		int16 snr_b_lcn[4];
		uint8 gidx[4];
		int8 snr[4];
		int8 snr_lcn[4];
		phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
		phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
		uint16 rssi_addr[4], snr_a_addr[4], snr_b_addr[4];
		uint16 lcnphyregs_shm_addr =
			2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR);
		int16 rxpath_gain;
		uint16 board_atten = 0;
		int16 rssi_lcn40_qdb[4];
		uint16 rssi_qdb_addr[4];
		uint8 gain_resp[4];
		uint8 gidx_calc[4];

#if RSSI_IQEST_DEBUG
		uint16 rssi_iqpwr;
		int16 rssi_iqpwr_dB;
		int wait_count = 0;
		uint16 board_atten_dbg;

		while (wlapi_bmac_read_shm(pi->sh->physhim, lcnphyregs_shm_addr + M_RSSI_LOCK)) {
			/* Check for timeout */
			if (wait_count > 100) { /* 1 ms */
				PHY_ERROR(("wl%d: %s: Unable to get RSSI_LOCK\n",
					pi->sh->unit, __FUNCTION__));
				goto iqest_rssi_skip;
			}
			OSL_DELAY(10);
			wait_count++;
		}
		wlapi_bmac_write_shm(pi->sh->physhim, lcnphyregs_shm_addr + M_RSSI_LOCK, 1);
		rssi_iqpwr = wlapi_bmac_read_shm(pi->sh->physhim,
			lcnphyregs_shm_addr + M_RSSI_IQPWR_DBG);
		rssi_iqpwr_dB = wlapi_bmac_read_shm(pi->sh->physhim,
			lcnphyregs_shm_addr + M_RSSI_IQPWR_DB_DBG);
		board_atten_dbg = wlapi_bmac_read_shm(pi->sh->physhim,
			lcnphyregs_shm_addr + M_RSSI_BOARDATTEN_DBG);
		wlapi_bmac_write_shm(pi->sh->physhim, lcnphyregs_shm_addr + M_RSSI_LOCK, 0);
		printf("iqpwr = %d, iqpwr_dB = %d, board_atten = %d\n",
			rssi_iqpwr, rssi_iqpwr_dB, board_atten_dbg);
iqest_rssi_skip:
#endif /* RSSI_IQEST_DEBUG */

		rssi_addr[0] = lcnphyregs_shm_addr + M_LCN_RSSI_0;
		rssi_addr[1] = lcnphyregs_shm_addr + M_LCN_RSSI_1;
		rssi_addr[2] = lcnphyregs_shm_addr + M_LCN_RSSI_2;
		rssi_addr[3] = lcnphyregs_shm_addr + M_LCN_RSSI_3;

		rssi_qdb_addr[0] = lcnphyregs_shm_addr + M_LCN40_RSSI_QDB_0;
		rssi_qdb_addr[1] = lcnphyregs_shm_addr + M_LCN40_RSSI_QDB_1;
		rssi_qdb_addr[2] = lcnphyregs_shm_addr + M_LCN40_RSSI_QDB_2;
		rssi_qdb_addr[3] = lcnphyregs_shm_addr + M_LCN40_RSSI_QDB_3;

		stats.rssi = 0;

		for (i = 0; i < 4; i++) {
			rssi_lcn[i] = (int8)(wlapi_bmac_read_shm(pi->sh->physhim,
				rssi_addr[i]) & 0xFF);
			snr_lcn[i] = (int8)((wlapi_bmac_read_shm(pi->sh->physhim,
				rssi_addr[i]) >> 8) & 0xFF);
			BCM_REFERENCE(snr_lcn[i]);
			gidx[i] =
				(wlapi_bmac_read_shm(pi->sh->physhim, rssi_addr[i])
				& LCN40_RX_GAIN_INDEX_MASK) >> LCN40_RX_GAIN_INDEX_SHIFT;

			if (ISLCNPHY(pi)) {
				rssi_lcn[i] +=
				        phy_lcn_get_pkt_rssi_gain_index_offset(gidx[i]);
				if ((wlapi_bmac_read_shm(pi->sh->physhim, rssi_addr[i])) & 0x8000) {
					rssi_lcn[i] = rssi_lcn[i] +
						pi->rssi_corr_boardatten;
				} else
					rssi_lcn[i] = rssi_lcn[i] + pi->rssi_corr_normal;

				rssi_lcn[i] = rssi_lcn[i] << 2;
			} else {
				int8 *corr2g;
#ifdef BAND5G
				int8 *corr5g;
#endif /* BAND5G */
				int8 *corrperrg;
				int8 po_reg;
				int16 po_nom;
				uint8 tr_iso = 0;

				if (pi_lcn40->rssi_iqest_en) {
					board_atten = wlapi_bmac_read_shm(pi->sh->physhim,
					rssi_addr[i]) >> 15;

					gain_resp[i] =
						wlapi_bmac_read_shm(pi->sh->physhim,
						rssi_qdb_addr[i]) >> 2;
					gidx_calc[i] = ((gain_resp[i] + 18)* 85) >> 8;
					if (gidx[i] > 37)
						gidx_calc[i] = gidx_calc[i] + 38;
					if (CHSPEC_IS2G(pi->radio_chanspec))
						tr_iso = pi_lcn->lcnphy_tr_isolation_mid;
#ifdef BAND5G
					else
						tr_iso = pi_lcn->triso5g[0];
#endif // endif
					if (board_atten) {
						gidx_calc[i] = gidx_calc[i] + tr_iso;
					}
					gidx[i] = gidx_calc[i];

					rxpath_gain =
						wlc_lcn40phy_get_rxpath_gain_by_index(pi,
						gidx[i], board_atten);
					PHY_INFORM(("iqdB= %d, boardattn= %d, rxpath_gain= %d, "
						"gidx = %d, rssi_iqest_gain_adj = %d\n",
						rssi_lcn[i], board_atten, rxpath_gain, gidx[i],
						pi_lcn40->rssi_iqest_gain_adj));
					rssi_lcn40_qdb[i] =
						wlapi_bmac_read_shm(pi->sh->physhim,
						rssi_qdb_addr[i]) & 0x3;
					rssi_lcn[i] = (rssi_lcn[i] << LCN40_QDB_SHIFT)
						+ rssi_lcn40_qdb[i] - rxpath_gain +
						(pi_lcn40->rssi_iqest_gain_adj
						<< LCN40_QDB_SHIFT);
				}

#if RSSI_CORR_EN
			if (!pi_lcn40->rssi_iqest_en) {
				/* JSSI adjustment wrt power offset */
				if (CHSPEC_IS20(pi->radio_chanspec))
					po_reg =
					PHY_REG_READ(pi, LCN40PHY,
					SignalBlockConfigTable6_new,
					crssignalblk_input_pwr_offset_db);
				else
					po_reg =
					PHY_REG_READ(pi, LCN40PHY,
					SignalBlockConfigTable5_new,
					crssignalblk_input_pwr_offset_db_40mhz);

				switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
				case WL_CHAN_FREQ_RANGE_2G:
					if (CHSPEC_IS20(pi->radio_chanspec))
						po_nom = pi_lcn->noise.nvram_input_pwr_offset_2g;
					else
						po_nom = pi_lcn->noise.nvram_input_pwr_offset_40_2g;
					break;
			#ifdef BAND5G
				case WL_CHAN_FREQ_RANGE_5GL:
					/* 5 GHz low */
					if (CHSPEC_IS20(pi->radio_chanspec))
						po_nom = pi_lcn->noise.nvram_input_pwr_offset_5g[0];
					else
						po_nom =
						pi_lcn->noise.nvram_input_pwr_offset_40_5g[0];
					break;
				case WL_CHAN_FREQ_RANGE_5GM:
					/* 5 GHz middle */
					if (CHSPEC_IS20(pi->radio_chanspec))
						po_nom = pi_lcn->noise.nvram_input_pwr_offset_5g[1];
					else
						po_nom =
						pi_lcn->noise.nvram_input_pwr_offset_40_5g[1];
					break;
				case WL_CHAN_FREQ_RANGE_5GH:
					/* 5 GHz high */
					if (CHSPEC_IS20(pi->radio_chanspec))
						po_nom = pi_lcn->noise.nvram_input_pwr_offset_5g[2];
					else
						po_nom =
						pi_lcn->noise.nvram_input_pwr_offset_40_5g[2];
					break;
			#endif /* BAND5G */
				default:
					po_nom = po_reg;
					break;
				}

				rssi_lcn[i] += ((po_nom - po_reg) << LCN40_QDB_SHIFT);

				/* RSSI adjustment and Adding the JSSI range specific corrections */
				#ifdef BAND5G
				if (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec) !=
					WL_CHAN_FREQ_RANGE_2G) {
						if (((rssi_lcn[i] >> LCN40_QDB_SHIFT) < -60) &&
							((gidx[i] > 0) && (gidx[i] < 38)))
					    rssi_lcn[i] +=
					      (phy_lcn40_get_pkt_rssi_gain_index_offset_5g(gidx[i])
					            << LCN40_QDB_SHIFT);
						corrperrg = pi->rssi_corr_perrg_5g;
					} else
				#endif /* BAND5G */
					{
						if (((rssi_lcn[i] >> LCN40_QDB_SHIFT) < -60) &&
							((gidx[i] > 0) && (gidx[i] < 38)))
					    rssi_lcn[i] +=
					      (phy_lcn40_get_pkt_rssi_gain_index_offset_2g(gidx[i])
					            << LCN40_QDB_SHIFT);
						corrperrg = pi->rssi_corr_perrg_2g;
					}

					if ((rssi_lcn[i] << LCN40_QDB_SHIFT) <= corrperrg[0])
						rssi_lcn[i] += (corrperrg[2] << LCN40_QDB_SHIFT);
					else if ((rssi_lcn[i] << LCN40_QDB_SHIFT) <= corrperrg[1])
						rssi_lcn[i] += (corrperrg[3] << LCN40_QDB_SHIFT);
					else
						rssi_lcn[i] += (corrperrg[4] << LCN40_QDB_SHIFT);

					corr2g = &(pi->rssi_corr_normal);
#ifdef BAND5G
					corr5g = &(pi->rssi_corr_normal_5g[0]);
#endif /* BAND5G */

				switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
						case WL_CHAN_FREQ_RANGE_2G:
							rssi_lcn[i] = rssi_lcn[i] +
								(*corr2g  << LCN40_QDB_SHIFT);
							break;
					#ifdef BAND5G
						case WL_CHAN_FREQ_RANGE_5GL:
							/* 5 GHz low */
							rssi_lcn[i] = rssi_lcn[i] +
							(corr5g[0] << LCN40_QDB_SHIFT);
							break;

						case WL_CHAN_FREQ_RANGE_5GM:
							/* 5 GHz middle */
							rssi_lcn[i] = rssi_lcn[i] +
							(corr5g[1] << LCN40_QDB_SHIFT);
							break;

						case WL_CHAN_FREQ_RANGE_5GH:
							/* 5 GHz high */
							rssi_lcn[i] = rssi_lcn[i] +
							(corr5g[2] << LCN40_QDB_SHIFT);
							break;
					#endif /* BAND5G */
						default:
							rssi_lcn[i] = rssi_lcn[i] + 0;
							break;
					}
				}

				/* Temp sense based correction */
				rssi_lcn[i] += wlc_lcn40phy_rssi_tempcorr(pi, 0);
				if (pi_lcn40->rssi_iqest_en)
					rssi_lcn[i] +=
					wlc_lcn40phy_iqest_rssi_tempcorr(pi, 0, board_atten);

#endif /* RSSI_CORR_EN */
			}
			stats.rssi += rssi_lcn[i];
		}

		stats.rssi = stats.rssi >> 2;

		/* temperature compensation */
		stats.rssi = stats.rssi + (pi_lcn->lcnphy_pkteng_rssi_slope << LCN40_QDB_SHIFT);

		/* convert into dB and save qdB portion */
		stats.rssi_qdb = stats.rssi & LCN40_QDB_MASK;
		stats.rssi = stats.rssi >> LCN40_QDB_SHIFT;

		/* SNR */
		snr_a_addr[0] = lcnphyregs_shm_addr + M_LCN_SNR_A_0;
		snr_a_addr[1] = lcnphyregs_shm_addr + M_LCN_SNR_A_1;
		snr_a_addr[2] = lcnphyregs_shm_addr + M_LCN_SNR_A_2;
		snr_a_addr[3] = lcnphyregs_shm_addr + M_LCN_SNR_A_3;

		snr_b_addr[0] = lcnphyregs_shm_addr + M_LCN_SNR_B_0;
		snr_b_addr[1] = lcnphyregs_shm_addr + M_LCN_SNR_B_1;
		snr_b_addr[2] = lcnphyregs_shm_addr + M_LCN_SNR_B_2;
		snr_b_addr[3] = lcnphyregs_shm_addr + M_LCN_SNR_B_3;

		stats.snr = 0;
		for (i = 0; i < 4; i++) {
			snr_a_lcn[i] = wlapi_bmac_read_shm(pi->sh->physhim, snr_a_addr[i]);
			snr_b_lcn[i] = wlapi_bmac_read_shm(pi->sh->physhim, snr_b_addr[i]);
			snr[i] = ((snr_a_lcn[i] - snr_b_lcn[i])* 3) >> 5;
			if (snr[i] > 31)
				snr[i] = 31;
			stats.snr += snr[i];
			PHY_INFORM(("i = %d, gidx = %d, snr = %d, snr_lcn = %d\n",
				i, phy_lcn_get_pkt_rssi_gain_index_offset(gidx[i]),
				snr[i], snr_lcn[i]));
		}
		stats.snr = stats.snr >> 2;

#if RSSI_IQEST_DEBUG
	stats.rssi = rssi_iqpwr_dB;
	stats.lostfrmcnt = rssi_iqpwr;
	stats.snr = board_atten_dbg;
#endif // endif
	} else {
		/* Not available */
		stats.rssi = stats.snr = 0;
	}

#if defined(WLNOKIA_NVMEM)
	/* Nokia NVMEM spec specifies the rssi offsets */
	stats.rssi = wlc_phy_upd_rssi_offset(pi, (int8)stats.rssi, pi->radio_chanspec);
#endif // endif

	/* rx pkt stats */
	if (ISACPHY(pi)) {
		for (i = 0; i <= NUM_80211_RATES; i++)
			stats.rxpktcnt[i] = pi->u.pi_acphy->rxstats[i];
	} else {
		rxstats_base = wlapi_bmac_read_shm(pi->sh->physhim, M_RXSTATS_BLK_PTR);
		for (i = 0; i <= NUM_80211_RATES; i++) {
			stats.rxpktcnt[i] =
				wlapi_bmac_read_shm(pi->sh->physhim, 2*(rxstats_base+i));
		}
	}
	bcopy(&stats, a,
		(sizeof(wl_pkteng_stats_t) < (uint)alen) ? sizeof(wl_pkteng_stats_t) : (uint)alen);

	return BCME_OK;
}

static int
wlc_phy_iovar_idletssi(phy_info_t *pi, int32 *ret_int_ptr, bool type)
{
	/* no argument or type = 1 will do full tx_pwr_ctrl_init */
	/* type = 0 will do just idle_tssi_est */
	int err = BCME_OK;
	if (ISLCNPHY(pi))
		*ret_int_ptr = wlc_lcnphy_idle_tssi_est_iovar(pi, type);
	else if (ISLCN40PHY(pi))
		*ret_int_ptr = wlc_lcn40phy_idle_tssi_est_iovar(pi, type);
	else
		*ret_int_ptr = 0;

	return err;
}

static int
wlc_phy_iovar_bbmult_get(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr)
{
	int err = BCME_OK;

	if (ISNPHY(pi))
		wlc_phy_get_bbmult_nphy(pi, ret_int_ptr);
	else
		err = BCME_UNSUPPORTED;

	return err;
}

static int
wlc_phy_iovar_bbmult_set(phy_info_t *pi, void *p)
{
	int err = BCME_OK;
	uint16 bbmult[PHY_CORE_NUM_2] = { 0 };
	uint8 m0, m1;

	bcopy(p, bbmult, PHY_CORE_NUM_2 * sizeof(uint16));

	if (ISNPHY(pi)) {
		m0 = (uint8)(bbmult[0] & 0xff);
		m1 = (uint8)(bbmult[1] & 0xff);
		wlc_phy_set_bbmult_nphy(pi, m0, m1);
	} else
		err = BCME_UNSUPPORTED;

	return err;
}

static int
wlc_phy_iovar_vbatsense(phy_info_t *pi, int32 *ret_int_ptr)
{
	int err = BCME_OK;
	int32 int_val;

	if (ISLCNPHY(pi)) {
		int_val = wlc_lcnphy_vbatsense(pi, 1);
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else if (ISLCN40PHY(pi)) {
		int_val = wlc_lcn40phy_vbatsense(pi, TEMPER_VBAT_TRIGGER_NEW_MEAS);
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else if (ISNPHY(pi)) {
		int_val = (int32)(wlc_phy_vbat_from_statusbyte_nphy_rev19(pi));
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else
		err = BCME_UNSUPPORTED;

	return err;
}
#endif // endif

#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV) || \
	defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG) || defined(ATE_BUILD)
static int
wlc_phy_iovar_forcecal(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set)
{
	int err = BCME_OK;
	void *a;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx;
#endif /* PHYCAL_CACHING */

	a = (int32*)ret_int_ptr;

	if (!pi->sh->up)
		return BCME_NOTUP;

	if (ISHTPHY(pi)) {
		uint8 mphase = 0, searchmode = 0;

		/* for get with no argument, assume 0x00 */
		if (!set)
			int_val = 0x00;

		/* upper nibble: 0 = sphase,  1 = mphase */
		mphase = (((uint8) int_val) & 0xf0) >> 4;

		/* 0 = RESTART, 1 = REFINE, for Tx-iq/lo-cal */
		searchmode = ((uint8) int_val) & 0xf;

		PHY_CAL(("wlc_phy_iovar_forcecal (mphase = %d, refine = %d)\n",
			mphase, searchmode));

		/* call sphase or schedule mphase cal */
		wlc_phy_cal_perical_mphase_reset(pi);
		if (mphase) {
			pi->cal_info->cal_searchmode = searchmode;
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
		} else {
			wlc_phy_cals_htphy(pi, searchmode);
		}
	} else if (ISACPHY(pi)) {
		uint8 mphase = FALSE;
		uint8 searchmode = PHY_CAL_SEARCHMODE_RESTART;

		/* for get with no argument, assume 0x00 */
		if (!set)
			int_val = 0x00;

		/* only values in range [0-3] are valids */
		if (int_val > 3)
			return BCME_BADARG;

		/* 3 is mphase, anything else is single phase */
		if (int_val == 3) {
			mphase = TRUE;
		}
		else {
			/* Single phase, using 2 means sphase partial */
			if (int_val == 2)
				searchmode = PHY_CAL_SEARCHMODE_REFINE;
		}

		PHY_CAL(("wlc_phy_iovar_forcecal (mphase = %d, refine = %d)\n",
			mphase, searchmode));

		/* call sphase or schedule mphase cal */
		wlc_phy_cal_perical_mphase_reset(pi);
		if (mphase) {
			pi->cal_info->cal_searchmode = searchmode;
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
		} else {
			wlc_phy_cals_acphy(pi, searchmode);
		}
	} else if (ISNPHY(pi)) {
		/* for get with no argument, assume 0x00 */
		if (!set)
			int_val = PHY_PERICAL_AUTO;

		if ((int_val == PHY_PERICAL_PARTIAL) ||
		    (int_val == PHY_PERICAL_AUTO) ||
		    (int_val == PHY_PERICAL_FULL)) {
			wlc_phy_cal_perical_mphase_reset(pi);
			pi->u.pi_nphy->cal_type_override = (uint8)int_val;
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
#ifdef WLOTA_EN
		} else if (int_val == PHY_FULLCAL_SPHASE) {
			wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_FULLCAL_SPHASE);
#endif /* WLOTA_EN */
		} else
			err = BCME_RANGE;

		/* phy_forcecal will trigger noisecal */
		pi->trigger_noisecal = TRUE;

	} else if (ISLCNCOMMONPHY(pi) && pi->pi_fptr.calibmodes) {
#if defined(PHYCAL_CACHING)
		ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
		if (ctx) {
			ctx->valid = FALSE;
		}
		/* null ctx is invalid by definition */
#else
		phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
		pi_lcn->lcnphy_full_cal_channel = 0;
#endif /* PHYCAL_CACHING */
		if (!set)
			*ret_int_ptr = 0;

		pi->pi_fptr.calibmodes(pi, PHY_FULLCAL);
		int_val = 0;
		bcopy(&int_val, a, vsize);
	}

	return err;
}

#ifndef ATE_BUILD
static int
wlc_phy_iovar_forcecal_obt(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set)
{
	int err = BCME_OK;
	uint8 wait_ctr = 0;
	int val = 1;
	if (ISNPHY(pi)) {
			wlc_phy_cal_perical_mphase_reset(pi);

			pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_INIT;
			pi->trigger_noisecal = TRUE;

			while (wait_ctr < 50) {
				val = ((pi->cal_info->cal_phase_id !=
					MPHASE_CAL_STATE_IDLE)? 1 : 0);
				if (val == 0) {
					err = BCME_OK;
					break;
				}
				else {
					wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_FULL);
					wait_ctr++;
				}
			}
			if (wait_ctr >= 50) {
				return BCME_ERROR;
			}

		}

	return err;
}

/* JIRA: SWWLAN-32606, RB: 12975: function to do only Noise cal & read crsmin power of
 * core 0 & core 1
*/
static int
wlc_phy_iovar_forcecal_noise(phy_info_t *pi, int32 int_val, void *a, int vsize, bool set)
{
	int err = BCME_OK;
	uint8 wait_ctr = 0;
	int val = 1;
	uint16 crsmin[4];

	if (ISNPHY(pi)) {
		if (!set) {
			crsmin[0] = phy_utils_read_phyreg(pi, NPHY_crsminpowerl0);
			crsmin[1] = phy_utils_read_phyreg(pi, NPHY_crsminpoweru0);
			crsmin[2] = phy_utils_read_phyreg(pi, NPHY_crsminpowerl0_core1);
			crsmin[3] = phy_utils_read_phyreg(pi, NPHY_crsminpoweru0_core1);
			bcopy(crsmin, a, sizeof(uint16)*4);
		}
		else {
			wlc_phy_cal_perical_mphase_reset(pi);

			pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_NOISECAL;
			pi->trigger_noisecal = TRUE;

			while (wait_ctr < 50) {
				val = ((pi->cal_info->cal_phase_id !=
				MPHASE_CAL_STATE_IDLE)? 1 : 0);

				if (val == 0) {
					err = BCME_OK;
					break;
				}
				else {
					wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_PARTIAL);
					wait_ctr++;
				}
			}

			if (wait_ctr >= 50) {
				return BCME_ERROR;
			}
		}
	}
	return err;
}
#endif /* !ATE_BUILD */
#endif // endif

#if defined(WLTEST) || defined(DBG_PHY_IOV)
static int
wlc_phy_dynamic_ml(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set)
{
	int err = BCME_OK;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;
	else if (NREV_LT(pi->pubpi.phy_rev, LCNXN_BASEREV + 2))
		return BCME_UNSUPPORTED;

	if (!set) {
		if (ISNPHY(pi)) {
			wlc_phy_dynamic_ml_get(pi);
			*ret_int_ptr = pi->nphy_ml_type;
		}

	} else {
		if ((int_val > 4) || (int_val < 0))
			return BCME_RANGE;
		wlc_phy_dynamic_ml_set(pi, int_val);
	}
	return err;
}

/* aci_nams : ACI Non Assoc Mode Sanity */
static int
wlc_phy_aci_nams(phy_info_t *pi, int32 int_val,	int32 *ret_int_ptr, int vsize, bool set)
{
	int err = BCME_OK;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;
	else if NREV_LT(pi->pubpi.phy_rev, LCNXN_BASEREV + 2)
		return BCME_UNSUPPORTED;

	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = pi->aci_nams;
		}

	} else {
		if ((int_val > 1) || (int_val < 0))
			return BCME_RANGE;
		pi->aci_nams = (uint8)int_val;
	}
	return err;
}
#endif // endif

static int
wlc_phy_iovar_oclscd(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	int err = BCME_OK;
	uint8 coremask;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	coremask = ((phy_utils_read_phyreg(pi, NPHY_CoreConfig) & NPHY_CoreConfig_CoreMask_MASK)
		>> NPHY_CoreConfig_CoreMask_SHIFT);

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;
	else if (NREV_LT(pi->pubpi.phy_rev, LCNXN_BASEREV + 2))
		return BCME_UNSUPPORTED;

	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = pi->nphy_oclscd;
		}

	} else {
		if (ISNPHY(pi)) {

			if ((int_val > 3) || (int_val < 0))
				return BCME_RANGE;

			if (int_val == 2)
				return BCME_BADARG;

			if ((coremask < 3) && (int_val != 0))
				return BCME_NORESOURCE;

			pi->nphy_oclscd = (uint8)int_val;
			/* suspend mac */
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

			wlc_phy_set_oclscd_nphy(pi);

			/* resume mac */
			wlapi_enable_mac(pi->sh->physhim);
		}
	}
	return err;
}

/* Debug functionality. Is called via an iovar. */
static int
wlc_phy_iovar_prog_lnldo2(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	int err = BCME_OK;
	uint8 lnldo2_val = 0;
	uint32 reg_value = 0;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;
	else if (!CHIPID_4324X_IPA_FAMILY(pi))
		return BCME_UNSUPPORTED;

	if (!set) {
		/* READ */
		wlc_si_pmu_regcontrol_access(pi, 5, &reg_value, 0);
		*ret_int_ptr = (int32)((reg_value & 0xff) >> 1);
	} else {
		/* WRITE */
		lnldo2_val = (uint8)(int_val & 0xff);
		*ret_int_ptr = wlc_phy_lnldo2_war_nphy(pi, 1, lnldo2_val);
	}
	return err;
}

#if defined(WLTEST) || defined(MACOSX)
static void
wlc_phy_iovar_set_deaf(phy_info_t *pi, int32 int_val)
{
	if (int_val) {
		wlc_phy_set_deaf((wlc_phy_t *) pi, TRUE);
	} else {
		wlc_phy_clear_deaf((wlc_phy_t *) pi, TRUE);
	}
}

static int
wlc_phy_iovar_get_deaf(phy_info_t *pi, int32 *ret_int_ptr)
{
	if (ISHTPHY(pi)) {
		*ret_int_ptr = (int32)wlc_phy_get_deaf_htphy(pi);
		return BCME_OK;
	} else if (ISNPHY(pi)) {
		*ret_int_ptr = (int32)wlc_phy_get_deaf_nphy(pi);
		return BCME_OK;
	} else if (ISACPHY(pi)) {
	        *ret_int_ptr = (int32)wlc_phy_get_deaf_acphy(pi);
		return BCME_OK;
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif // endif
#if defined(WLTEST) || defined(ATE_BUILD)
static int
wlc_phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	int err = BCME_OK;

	if (!set) {
		if (ISACPHY(pi) || ISHTPHY(pi)) {
			*ret_int_ptr = pi->txpwrctrl;
		} else if (ISNPHY(pi)) {
			*ret_int_ptr = pi->nphy_txpwrctrl;
		} else if (ISLCNPHY(pi)) {
			*ret_int_ptr = wlc_phy_tpc_iovar_isenabled_lcnphy(pi);
		} else if (pi->pi_fptr.ishwtxpwrctrl) {
			*ret_int_ptr = pi->pi_fptr.ishwtxpwrctrl(pi);
		}

	} else {
		if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)) {
			if ((int_val != PHY_TPC_HW_OFF) && (int_val != PHY_TPC_HW_ON)) {
				err = BCME_RANGE;
				goto end;
			}

			pi->nphy_txpwrctrl = (uint8)int_val;
			pi->txpwrctrl = (uint8)int_val;

			/* if not up, we are done */
			if (!pi->sh->up)
				goto end;

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);
			if (ISNPHY(pi))
				wlc_phy_txpwrctrl_enable_nphy(pi, (uint8) int_val);
			else if (ISHTPHY(pi))
				wlc_phy_txpwrctrl_enable_htphy(pi, (uint8) int_val);
			else if (ISACPHY(pi))
				wlc_phy_txpwrctrl_enable_acphy(pi, (uint8) int_val);
			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);

		} else if (ISLCNPHY(pi)) {
			wlc_lcnphy_iovar_txpwrctrl(pi, int_val, ret_int_ptr);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_iovar_txpwrctrl(pi, int_val, ret_int_ptr);
		}
	}

end:
	return err;
}

static int
wlc_phy_iovar_txpwrindex_get(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr)
{
	int err = BCME_OK;

	if (ISNPHY(pi)) {

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
			(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
			OSL_DELAY(1);
		}

		*ret_int_ptr = wlc_phy_txpwr_idx_get_nphy(pi);

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);

	} else if (ISHTPHY(pi)) {
		*ret_int_ptr = wlc_phy_txpwr_idx_get_htphy(pi);
	} else if (ISACPHY(pi)) {
		*ret_int_ptr = wlc_phy_txpwr_idx_get_acphy(pi);
	} else if (ISLCNPHY(pi))
		*ret_int_ptr = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	else if (ISLCN40PHY(pi))
		*ret_int_ptr = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

	return err;
}

static int
wlc_phy_iovar_txpwrindex_set(phy_info_t *pi, void *p)
{
	int err = BCME_OK;
	uint32 txpwridx[PHY_CORE_MAX] = { 0x30 };
	int8 idx, core;
	int8 siso_int_val;
	phy_info_nphy_t *pi_nphy = NULL;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif

	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	bcopy(p, txpwridx, PHY_CORE_MAX * sizeof(uint32));
	siso_int_val = (int8)(txpwridx[0] & 0xff);

	if (ISNPHY(pi)) {
		FOREACH_CORE(pi, core) {
			idx = (int8)(txpwridx[core] & 0xff);
			pi_nphy->nphy_txpwrindex[core].index_internal = idx;
			wlc_phy_store_txindex_nphy(pi);
			wlc_phy_txpwr_index_nphy(pi, (1 << core), idx, TRUE);
		}
	} else if (ISHTPHY(pi)) {
		FOREACH_CORE(pi, core) {
			idx = (int8)(txpwridx[core] & 0xff);
			wlc_phy_txpwr_by_index_htphy(pi, (1 << core), idx);
		}
	} else if (ISACPHY(pi)) {
		FOREACH_CORE(pi, core) {
			idx = (int8)(txpwridx[core] & 0xff);
			/* XXX: wlc_phy_txpwrctrl_enable_acphy can be moved into
			 * wlc_phy_txpwr_by_index_acphy
			 */
			wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
			wlc_phy_txpwr_by_index_acphy(pi, (1 << core), idx);
		}
	} else if (ISLCNCOMMONPHY(pi)) {
#if defined(PHYCAL_CACHING)
		err = wlc_iovar_txpwrindex_set_lcncommon(pi, siso_int_val, ctx);
#else
		err = wlc_iovar_txpwrindex_set_lcncommon(pi, siso_int_val);
#endif // endif
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	return err;
}
#endif // endif

#if defined(WLTEST)
static int
wlc_phy_iovar_txrx_chain(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, bool set)
{
	int err = BCME_OK;

	if (ISHTPHY(pi))
		return BCME_UNSUPPORTED;

	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = (int)pi->nphy_txrx_chain;
		}
	} else {
		if (ISNPHY(pi)) {
			if ((int_val != AUTO) && (int_val != WLC_N_TXRX_CHAIN0) &&
				(int_val != WLC_N_TXRX_CHAIN1)) {
				err = BCME_RANGE;
				goto end;
			}

			if (pi->nphy_txrx_chain != (int8)int_val) {
				pi->nphy_txrx_chain = (int8)int_val;
				if (pi->sh->up) {
					wlapi_suspend_mac_and_wait(pi->sh->physhim);
					phy_utils_phyreg_enter(pi);
					wlc_phy_stf_chain_upd_nphy(pi);
					wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);
					phy_utils_phyreg_exit(pi);
					wlapi_enable_mac(pi->sh->physhim);
				}
			}
		}
	}
end:
	return err;
}

static void
wlc_phy_iovar_bphy_testpattern(phy_info_t *pi, uint8 testpattern, bool enable)
{
	bool existing_enable = FALSE;

	/* WL out check */
	if (pi->sh->up) {
		PHY_ERROR(("wl%d: %s: This function needs to be called after 'wl out'\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	/* confirm band is locked to 2G */
	if (!CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_ERROR(("wl%d: %s: Band needs to be locked to 2G (b)\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	if (NREV_LT(pi->pubpi.phy_rev, 2) || ISHTPHY(pi)) {
		PHY_ERROR(("wl%d: %s: This function is supported only for NPHY PHY_REV > 1\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	if (testpattern == NPHY_TESTPATTERN_BPHY_EVM) {    /* CW CCK for EVM testing */
		existing_enable = (bool) pi->phy_bphy_evm;
	} else if (testpattern == NPHY_TESTPATTERN_BPHY_RFCS) { /* RFCS testpattern */
		existing_enable = (bool) pi->phy_bphy_rfcs;
	} else {
		PHY_ERROR(("Testpattern needs to be between [0 (BPHY_EVM), 1 (BPHY_RFCS)]\n"));
		ASSERT(0);
	}

	if (ISNPHY(pi)) {
		wlc_phy_bphy_testpattern_nphy(pi, testpattern, enable, existing_enable);
	} else {
		PHY_ERROR(("support yet to be added\n"));
		ASSERT(0);
	}

	/* Return state of testpattern enables */
	if (testpattern == NPHY_TESTPATTERN_BPHY_EVM) {    /* CW CCK for EVM testing */
		pi->phy_bphy_evm = enable;
	} else if (testpattern == NPHY_TESTPATTERN_BPHY_RFCS) { /* RFCS testpattern */
		pi->phy_bphy_rfcs = enable;
	}
}

static void
wlc_phy_iovar_scraminit(phy_info_t *pi, int8 scraminit)
{
	pi->phy_scraminit = (int8)scraminit;
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (ISNPHY(pi)) {
		wlc_phy_test_scraminit_nphy(pi, scraminit);
	} else if (ISHTPHY(pi)) {
		wlc_phy_test_scraminit_htphy(pi, scraminit);
	} else if (ISACPHY(pi)) {
		wlc_phy_test_scraminit_acphy(pi, scraminit);
	} else {
		PHY_ERROR(("support yet to be added\n"));
		ASSERT(0);
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

static void
wlc_phy_iovar_force_rfseq(phy_info_t *pi, uint8 int_val)
{
	phy_utils_phyreg_enter(pi);
	if (ISNPHY(pi)) {
		wlc_phy_force_rfseq_nphy(pi, int_val);
	} else if (ISHTPHY(pi)) {
		wlc_phy_force_rfseq_htphy(pi, int_val);
	}
	phy_utils_phyreg_exit(pi);
}

static void
wlc_phy_iovar_tx_tone(phy_info_t *pi, int32 int_val)
{
	pi->phy_tx_tone_freq = (int32) int_val;

	if (pi->phy_tx_tone_freq == 0) {
		if (ISNPHY(pi)) {
			/* Restore back PAPD settings after stopping the tone */
			if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV))
				wlc_phy_papd_enable_nphy(pi, TRUE);
			/* FIXME4324. Dont know why only 4324 hangs if mac is not suspended */
			if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_stopplayback_nphy(pi);
			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
			if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
				wlapi_enable_mac(pi->sh->physhim);
		} else if (ISACPHY(pi)) {
			wlc_phy_stopplayback_acphy(pi);
			wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
			wlapi_enable_mac(pi->sh->physhim);
		} else if (ISHTPHY(pi)) {
			wlc_phy_stopplayback_htphy(pi);
			wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
		} else if (ISLCNPHY(pi)) {
			wlc_lcnphy_stop_tx_tone(pi);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_stop_tx_tone(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
	} else {
		if (ISNPHY(pi)) {
			/* use 151 since that should correspond to nominal tx output power */
			/* Can not play tone with papd bit enabled */
			if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV))
				wlc_phy_papd_enable_nphy(pi, FALSE);
			if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);
			wlc_phy_tx_tone_nphy(pi, (uint32)int_val, 151, 0, 0, TRUE); /* play tone */
			if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
				wlapi_enable_mac(pi->sh->physhim);
		} else if (ISACPHY(pi)) {
			pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);
			wlc_phy_tx_tone_acphy(pi, (int32)int_val, 151, 0, 0, TRUE);
		} else if (ISHTPHY(pi)) {
			wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
			wlc_phy_tx_tone_htphy(pi, (uint32)int_val, 151, 0, 0, TRUE); /* play tone */
		} else if (ISLCNPHY(pi)) {
			pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
			wlc_lcnphy_set_tx_tone_and_gain_idx(pi);
		} else if (ISLCN40PHY(pi)) {
			pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_lcn40phy_set_tx_tone_and_gain_idx(pi);
		}
	}
}

static void
wlc_phy_iovar_tx_tone_hz(phy_info_t *pi, int32 int_val)
{
	pi->phy_tx_tone_freq = (int32) int_val;

	if (ISLCNPHY(pi)) {
		wlc_lcnphy_set_tx_tone_and_gain_idx(pi);
	} else if (ISLCN40PHY(pi)) {
		wlc_lcn40phy_set_tx_tone_and_gain_idx(pi);
	}
}

static void
wlc_phy_iovar_tx_tone_stop(phy_info_t *pi)
{
	if (ISLCNPHY(pi)) {
		wlc_lcnphy_stop_tx_tone(pi);
	} else if (ISLCN40PHY(pi)) {
		wlc_lcn40phy_stop_tx_tone(pi);
	}
}

static int16
wlc_phy_iovar_test_tssi(phy_info_t *pi, uint8 val, uint8 pwroffset)
{
	int16 tssi = 0;
	if (ISNPHY(pi)) {
		tssi = (int16) wlc_phy_test_tssi_nphy(pi, val, pwroffset);
	} else if (ISHTPHY(pi)) {
		tssi = (int16) wlc_phy_test_tssi_htphy(pi, val, pwroffset);
	} else if (ISACPHY(pi)) {
		tssi = (int16) wlc_phy_test_tssi_acphy(pi, val, pwroffset);
	}
	return tssi;
}

static int16
wlc_phy_iovar_test_idletssi(phy_info_t *pi, uint8 val)
{
	int16 idletssi = INVALID_IDLETSSI_VAL;
	if (ISACPHY(pi)) {
		idletssi = (int16) wlc_phy_test_idletssi_acphy(pi, val);
	}
	return idletssi;
}

static int16
wlc_phy_iovar_setrptbl(phy_info_t *pi)
{
	if (ISACPHY(pi) && (!ACMAJORREV_1(pi->pubpi.phy_rev))) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_populate_recipcoeffs_acphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		return 0;
	}

	return BCME_UNSUPPORTED;
}

static int16
wlc_phy_iovar_forceimpbf(phy_info_t *pi)
{
	if (ISACPHY(pi) && (!ACMAJORREV_1(pi->pubpi.phy_rev))) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		phy_utils_write_phyreg(pi, ACPHY_BfeConfigReg0(pi->pubpi.phy_rev),
		                       BFECONFIGREF_FORCEVAL);

		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			WRITE_PHYREG(pi, BfeMuConfigReg1, 0x1000);
			WRITE_PHYREG(pi, BfeMuConfigReg2, 0x2000);
			WRITE_PHYREG(pi, BfeMuConfigReg3, 0x1000);
		}

		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		return 0;
	}

	return BCME_UNSUPPORTED;
}

static int16
wlc_phy_iovar_forcesteer(phy_info_t *pi, uint8 enable)
{
#if (ACCONF || ACCONF2) && defined(WL_BEAMFORMING)
	uint16 bfmcon_val      = 0;
	uint16 bfridx_pos_val  = 0;
	uint16 refresh_thr_val = 0;
	uint16 bfcfg_blk, shm_base, addr1, addr2, bfrctl = 0;

	if (ISACPHY(pi) && (!ACMAJORREV_1(pi->pubpi.phy_rev))) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		bfmcon_val      = enable ? BFMCON_FORCEVAL      : BFMCON_RELEASEVAL;
		bfridx_pos_val  = enable ? BFRIDX_POS_FORCEVAL  : BFRIDX_POS_RELEASEVAL;
		refresh_thr_val = enable ? REFRESH_THR_FORCEVAL : REFRESH_THR_RELEASEVAL;

		bfcfg_blk = wlapi_bmac_read_shm(pi->sh->physhim, M_BFCFG_PTR);
		shm_base = wlapi_bmac_read_shm(pi->sh->physhim, (bfcfg_blk << 1));

		/* NDP streams */
		if (PHY_BITSCNT(pi->sh->phytxchain) == 4) {
			/* 4 streams */
			bfrctl = (2 << C_BFI_BFRCTL_POS_NSTS_SHIFT);
		} else if (PHY_BITSCNT(pi->sh->phytxchain) == 3) {
			/* 3 streams */
			bfrctl = (1 << C_BFI_BFRCTL_POS_NSTS_SHIFT);
		} else if (PHY_BITSCNT(pi->sh->phytxchain) == 2) {
			/* 2 streams */
			bfrctl = 0;
		}
		bfrctl |= (pi->sh->phytxchain << C_BFI_BFRCTL_POS_BFM_SHIFT);
		wlapi_bmac_write_shm(pi->sh->physhim, shm_addr(shm_base, C_BFI_BFRCTL_POS), bfrctl);

		addr1 = shm_addr(bfcfg_blk, C_BFI_REFRESH_THR_OFFSET);
		addr2 = shm_addr(shm_base, C_BFI_BFRIDX_POS);
		phy_utils_write_phyreg(pi, ACPHY_BfmCon(pi->pubpi.phy_rev), bfmcon_val);

		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			uint32 tmpaddr = 0x1000;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX,
			1, 0x1000, 32, &tmpaddr);
			MOD_PHYREG(pi, BfeMuConfigReg0, useTxbfIndexAddr, 1);
		}

		wlapi_bmac_write_shm(pi->sh->physhim, addr1, refresh_thr_val);
		wlapi_bmac_write_shm(pi->sh->physhim, addr2, bfridx_pos_val);

		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		return 0;
	}
#endif /* (ACCONF || ACCONF2) && WL_BEAMFORMING */

	return BCME_UNSUPPORTED;
}

static void
wlc_phy_iovar_rxcore_enable(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (set) {
		if (ISNPHY(pi)) {
			wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, (uint8) int_val, 0);
		} else if (ISHTPHY(pi)) {
			wlc_phy_rxcore_setstate_htphy((wlc_phy_t *)pi, (uint8) int_val);
		} else if (ISACPHY(pi)) {
			wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi, (uint8) int_val);
		}
	} else {
		if (ISNPHY(pi)) {
			*ret_int_ptr =  (uint32)wlc_phy_rxcore_getstate_nphy((wlc_phy_t *)pi);
		} else if (ISHTPHY(pi)) {
			*ret_int_ptr =  (uint32)wlc_phy_rxcore_getstate_htphy((wlc_phy_t *)pi);
		} else if (ISACPHY(pi)) {
			*ret_int_ptr =  (uint32)wlc_phy_rxcore_getstate_acphy((wlc_phy_t *)pi);
		}
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

#endif // endif

static int
wlc_phy_iovar_set_btc_restage_rxgain(phy_info_t *pi, int32 set_val)
{
	int err = BCME_OK;

	if (ISHTPHY(pi) && (IS_X29B_BOARDTYPE(pi) ||
	                    IS_X29D_BOARDTYPE(pi) || IS_X33_BOARDTYPE(pi))) {
		phy_info_htphy_t *pi_ht = (phy_info_htphy_t *)pi->u.pi_htphy;

		if ((set_val < 0) || (set_val > 1)) {
			return BCME_RANGE;
		}
		if (SCAN_RM_IN_PROGRESS(pi)) {
			return BCME_NOTREADY;
		}
		if (IS_X29B_BOARDTYPE(pi)) {
			/* XXX
			 * Ensure no other code is tampering with init gain
			 */
			if ((pi->sh->interference_mode != INTERFERE_NONE) &&
			     (pi->sh->interference_mode != NON_WLAN)) {
				return BCME_UNSUPPORTED;
			}
		}
		if ((IS_X29D_BOARDTYPE(pi) || IS_X33_BOARDTYPE(pi)) &&
		    !CHSPEC_IS2G(pi->radio_chanspec)) {
			return BCME_BADBAND;
		}

		if (((set_val == 0) && pi_ht->btc_restage_rxgain) ||
		    ((set_val == 1) && !pi_ht->btc_restage_rxgain)) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_btc_restage_rxgain_htphy(pi, (bool)set_val);
			wlapi_enable_mac(pi->sh->physhim);
		}
	} else if (ISACPHY(pi)) {
#ifndef WLC_DISABLE_ACI
		phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
#endif /* !WLC_DISABLE_ACI */
		if ((set_val < 0) || (set_val > 7)) {
			return BCME_RANGE;
		}
		if (SCAN_RM_IN_PROGRESS(pi)) {
			return BCME_NOTREADY;
		}
#ifndef WLC_DISABLE_ACI
		if (((set_val == 0) && (pi_ac->btc_mode != 0)) ||
		    ((set_val != 0) && (pi_ac->btc_mode != set_val))) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_desense_btcoex_acphy(pi, set_val);
			wlapi_enable_mac(pi->sh->physhim);
		}
#endif /* !WLC_DISABLE_ACI */
	} else if (CHIPID_4324X_MEDIA_FAMILY(pi)) {

		const uint8 num_dev_thres = 2;	/* Make configurable later */
		uint16 num_bt_devices;

		/* pi->bt_shm_addr is set by phy_init */
		if (pi->bt_shm_addr == 0) {
			return BCME_NOTUP;
		}

		/* Read number of BT task and desense configuration for SHM */
		num_bt_devices  = wlapi_bmac_read_shm(pi->sh->physhim,
			pi->bt_shm_addr + M_BTCX_NUM_TASKS);

		/* If more than num_dev_thres (two) tasks, no interference override
		 * and not already in manual ACI override mode
		 */
		if ((num_dev_thres > 0) && (num_bt_devices >= num_dev_thres) &&
			(!pi->sh->interference_mode_override) &&
			(pi->sh->interference_mode != WLAN_MANUAL)) {

			wlc_phy_set_interference_override_mode(pi, WLAN_MANUAL);

		} else if ((num_bt_devices < num_dev_thres) &&
			(pi->sh->interference_mode_override)) {

			wlc_phy_set_interference_override_mode(pi, INTERFERE_OVRRIDE_OFF);
		}
		/* Return "not BCME_OK" so that this function will be called every time
		 * wlc_btcx_watchdog is called
		 */
		err = BCME_BUSY;

	} else {
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovar_get_btc_restage_rxgain(phy_info_t *pi, int32 *ret_val)
{
	if (ISHTPHY(pi) && (IS_X29B_BOARDTYPE(pi) || IS_X29D_BOARDTYPE(pi) ||
	                    IS_X33_BOARDTYPE(pi))) {
		if ((IS_X29D_BOARDTYPE(pi) || IS_X33_BOARDTYPE(pi)) &&
		    !CHSPEC_IS2G(pi->radio_chanspec)) {
			return BCME_BADBAND;
		}
		*ret_val = (int32)pi->u.pi_htphy->btc_restage_rxgain;
		return BCME_OK;
	} else if (ISACPHY(pi)) {
	  *ret_val = (int32)pi->u.pi_acphy->btc_mode;
		return BCME_OK;
	} else
		return BCME_UNSUPPORTED;
}

static int
wlc_phy_iovar_set_dssf(phy_info_t *pi, int32 set_val)
{
	if (ISACPHY(pi) && PHY_ILNA(pi)) {
	  phy_utils_write_phyreg(pi, ACPHY_DSSF_C_CTRL(pi->pubpi.phy_rev), (uint16) set_val);

		return BCME_OK;
	}

	return BCME_UNSUPPORTED;
}

static int
wlc_phy_iovar_get_dssf(phy_info_t *pi, int32 *ret_val)
{
	if (ISACPHY(pi) && PHY_ILNA(pi)) {
		*ret_val = (int32) phy_utils_read_phyreg(pi, ACPHY_DSSF_C_CTRL(pi->pubpi.phy_rev));

		return BCME_OK;
	}

	return BCME_UNSUPPORTED;
}

static int
wlc_phy_iovar_set_lesi(phy_info_t *pi, int32 set_val)
{

	if (!pi->lesi_mode) {
		if (ISACPHY(pi)) {
			MOD_PHYREG(pi, lesi_control, lesiFstrEn, (uint16) set_val);
			MOD_PHYREG(pi, lesi_control, lesiCrsEn, (uint16) set_val);

			/* chippkg bit-2: LESI supported, 0: TRUE ; 1: FALSE */
			if (ACMAJORREV_33(pi->pubpi.phy_rev) && (pi->sh->chippkg & 0x4) == 0) {
				pi->u.pi_acphy->lesi = (int8) set_val;
			}

			if (set_val > 0) {
				wlc_phy_lesi_acphy(pi, TRUE, 0);
			} else {
				wlc_phy_lesi_acphy(pi, FALSE, 0);
			}
			return BCME_OK;
		}
	}
	return BCME_UNSUPPORTED;
}

static int
wlc_phy_iovar_get_lesi(phy_info_t *pi, int32 *ret_val)
{
	if (ISACPHY(pi)) {
	  if ((READ_PHYREG(pi, lesi_control) & 0x3) == 0x3) {
	    *ret_val = pi->u.pi_acphy->lesi;
	  } else {
	    *ret_val = (int32) (READ_PHYREG(pi, lesi_control) & 0x1);
	  }
	  return BCME_OK;
	}
	return BCME_UNSUPPORTED;
}

/* handler for iovar modules */
typedef int (*iovar_module_t)(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);

iovar_module_t iovar_module_list[] = {
	wlc_phy_iovars_generic,
	wlc_phy_iovars_aci,
	wlc_phy_iovars_rssi,
	wlc_phy_iovars_calib,
	wlc_phy_iovars_txpwrctl,
	wlc_phy_iovars_phy_specific,
#ifdef SAMPLE_COLLECT
	phy_iovars_sample_collect,
#endif // endif
	NULL
};

/* Dispatch phy iovars */
int
wlc_phy_iovar_dispatch(phy_info_t *pi, uint32 actionid, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;
	iovar_module_t *module;
	module = iovar_module_list;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	BCM_REFERENCE(bool_val);

	do {
		err = (*module)(pi, actionid, -1, p, plen, a, alen, vsize);
		++module;
	} while ((*module != NULL) && (err == BCME_UNSUPPORTED));

	if (err == BCME_UNSUPPORTED)
		err = wlc_phy_iovar_dispatch_old(pi, actionid, p, a, vsize, int_val, bool_val);

	return err;
}

/* OLD PHYTYPE specific iovars, to be phased out gradually,
 * do NOT add more. Instead, share, expand or create unified ones in wlc_phy_iovar_dispatch
 * XXX to reorganize and consolidate
 */
static int
wlc_phy_iovar_dispatch_old(phy_info_t *pi, uint32 actionid, void *p, void *a, int vsize,
	int32 int_val, bool bool_val)
{
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	phy_info_nphy_t *pi_nphy;

	pi_nphy = pi->u.pi_nphy;
	BCM_REFERENCE(pi_nphy);
	BCM_REFERENCE(ret_int_ptr);

	switch (actionid) {
#if NCONF
#if defined(BCMDBG)
	case IOV_SVAL(IOV_NPHY_INITGAIN):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_setinitgain_nphy(pi, (uint16) int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_SVAL(IOV_NPHY_HPVGA1GAIN):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_sethpf1gaintbl_nphy(pi, (int8) int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_SVAL(IOV_NPHY_TX_TEMP_TONE): {
		uint16 orig_BBConfig;
		uint16 m0m1;
		nphy_txgains_t target_gain;

		if ((uint32)int_val > 0) {
			pi->phy_tx_tone_freq = (uint32) int_val;
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

			/* Save the bbmult values,since it gets overwritten by mimophy_tx_tone() */
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

			/* Disable the re-sampler (in case we are in spur avoidance mode) */
			orig_BBConfig = phy_utils_read_phyreg(pi, NPHY_BBConfig);
			phy_utils_mod_phyreg(pi, NPHY_BBConfig,
			                     NPHY_BBConfig_resample_clk160_MASK, 0);

			/* read current tx gain and use as target_gain */
			wlc_phy_get_tx_gain_nphy(pi, &target_gain);

			PHY_ERROR(("Tx gain core 0: target gain: ipa = %d,"
			         " pad = %d, pga = %d, txgm = %d, txlpf = %d\n",
			         target_gain.ipa[0], target_gain.pad[0], target_gain.pga[0],
			         target_gain.txgm[0], target_gain.txlpf[0]));

			PHY_ERROR(("Tx gain core 1: target gain: ipa = %d,"
			         " pad = %d, pga = %d, txgm = %d, txlpf = %d\n",
			         target_gain.ipa[0], target_gain.pad[1], target_gain.pga[1],
			         target_gain.txgm[1], target_gain.txlpf[1]));

			/* play a tone for 10 secs and then stop it and return */
			wlc_phy_tx_tone_nphy(pi, (uint32)int_val, 250, 0, 0, FALSE);

			/* Now restore the original bbmult values */
			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m0m1);

			OSL_DELAY(10000000);
			wlc_phy_stopplayback_nphy(pi);

			/* Restore the state of the re-sampler
			   (in case we are in spur avoidance mode)
			*/
			phy_utils_write_phyreg(pi, NPHY_BBConfig, orig_BBConfig);

			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		break;
	}
	case IOV_SVAL(IOV_NPHY_CAL_RESET):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_cal_reset_nphy(pi, (uint32) int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_EST_TONEPWR):
	case IOV_GVAL(IOV_PHY_EST_TONEPWR): {
		int32 dBm_power[2];
		uint16 orig_BBConfig;
		uint16 m0m1;

		if (ISNPHY(pi)) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);

			/* Save the bbmult values, since it gets overwritten
			   by mimophy_tx_tone()
			*/
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

			/* Disable the re-sampler (in case we are in spur avoidance mode) */
			orig_BBConfig = phy_utils_read_phyreg(pi, NPHY_BBConfig);
			phy_utils_mod_phyreg(pi, NPHY_BBConfig,
			                     NPHY_BBConfig_resample_clk160_MASK, 0);
			pi->phy_tx_tone_freq = (uint32) 4000;

			/* play a tone for 10 secs */
			wlc_phy_tx_tone_nphy(pi, (uint32)4000, 250, 0, 0, FALSE);

			/* Now restore the original bbmult values */
			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m0m1);

			OSL_DELAY(10000000);
			wlc_phy_est_tonepwr_nphy(pi, dBm_power, 128);
			wlc_phy_stopplayback_nphy(pi);

			/* Restore the state of the re-sampler
			   (in case we are in spur avoidance mode)
			*/
			phy_utils_write_phyreg(pi, NPHY_BBConfig, orig_BBConfig);

			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);

			int_val = dBm_power[0]/4;
			bcopy(&int_val, a, vsize);
			break;
		} else {
			err = BCME_UNSUPPORTED;
			break;
		}
	}

	case IOV_GVAL(IOV_NPHY_RFSEQ_TXGAIN): {
		uint16 rfseq_tx_gain[2];
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, rfseq_tx_gain);
		int_val = (((uint32) rfseq_tx_gain[1] << 16) | ((uint32) rfseq_tx_gain[0]));
		bcopy(&int_val, a, vsize);
		break;
	}

	case IOV_SVAL(IOV_PHY_SPURAVOID):
		if ((int_val != SPURAVOID_DISABLE) && (int_val != SPURAVOID_AUTO) &&
		    (int_val != SPURAVOID_FORCEON) && (int_val != SPURAVOID_FORCEON2)) {
			err = BCME_RANGE;
			break;
		}

		pi->phy_spuravoid = (int8)int_val;
		break;

	case IOV_GVAL(IOV_PHY_SPURAVOID):
		int_val = pi->phy_spuravoid;
		bcopy(&int_val, a, vsize);
		break;
#endif /* defined(BCMDBG) */

#if defined(WLTEST)
	case IOV_GVAL(IOV_NPHY_CCK_PWR_OFFSET):
		if (ISNPHY(pi)) {
			int_val =  pi_nphy->nphy_cck_pwr_err_adjust;
			bcopy(&int_val, a, vsize);
		}
		break;
	case IOV_GVAL(IOV_NPHY_CAL_SANITY):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		*ret_int_ptr = (uint32)wlc_phy_cal_sanity_nphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_BPHY_EVM):
	case IOV_GVAL(IOV_PHY_BPHY_EVM):

		*ret_int_ptr = pi->phy_bphy_evm;
		break;

	case IOV_SVAL(IOV_NPHY_BPHY_EVM):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_EVM, (bool) int_val);
		break;

	case IOV_GVAL(IOV_NPHY_BPHY_RFCS):
		*ret_int_ptr = pi->phy_bphy_rfcs;
		break;

	case IOV_SVAL(IOV_NPHY_BPHY_RFCS):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_RFCS, (bool) int_val);
		break;

	case IOV_GVAL(IOV_NPHY_SCRAMINIT):
		*ret_int_ptr = pi->phy_scraminit;
		break;

	case IOV_SVAL(IOV_NPHY_SCRAMINIT):
		wlc_phy_iovar_scraminit(pi, pi->phy_scraminit);
		break;

	case IOV_SVAL(IOV_NPHY_RFSEQ):
		wlc_phy_iovar_force_rfseq(pi, (uint8)int_val);
		break;

	case IOV_GVAL(IOV_NPHY_TXIQLOCAL): {
		nphy_txgains_t target_gain;
		uint8 tx_pwr_ctrl_state;
		if (ISNPHY(pi)) {

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);

			/* read current tx gain and use as target_gain */
			wlc_phy_get_tx_gain_nphy(pi, &target_gain);
			tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
			wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

			/* want outer (0,1) ants so T/R works properly for CB2 2x3 switch, */
			if (pi->antsel_type == ANTSEL_2x3) {
				wlc_phy_antsel_init_nphy((wlc_phy_t *)pi, TRUE);
			}

			err = wlc_phy_cal_txiqlo_nphy(pi, target_gain, TRUE, FALSE);
			if (err)
				break;
			wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		*ret_int_ptr = 0;
		break;
	}
	case IOV_SVAL(IOV_NPHY_RXIQCAL): {
		nphy_txgains_t target_gain;
		uint8 tx_pwr_ctrl_state;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		/* read current tx gain and use as target_gain */
		wlc_phy_get_tx_gain_nphy(pi, &target_gain);
		tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);
#ifdef RXIQCAL_FW_WAR
		if (wlc_phy_cal_rxiq_nphy_fw_war(pi, target_gain, 0, (bool)int_val, 0x3) != BCME_OK)
#else
		if (wlc_phy_cal_rxiq_nphy(pi, target_gain, 0, (bool)int_val, 0x3) != BCME_OK)
#endif // endif
		{
			break;
		}
		wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		int_val = 0;
		bcopy(&int_val, a, vsize);
		break;
	}
	case IOV_GVAL(IOV_NPHY_RXCALPARAMS):
		if (ISNPHY(pi)) {
			*ret_int_ptr = pi_nphy->nphy_rxcalparams;
		}
		break;

	case IOV_SVAL(IOV_NPHY_RXCALPARAMS):
		if (ISNPHY(pi)) {
			pi_nphy->nphy_rxcalparams = (uint32)int_val;
		}
		break;

	case IOV_GVAL(IOV_NPHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_NPHY_RSSISEL):
		*ret_int_ptr = pi->nphy_rssisel;
		break;

	case IOV_SVAL(IOV_NPHY_RSSISEL):
		pi->nphy_rssisel = (uint8)int_val;

		if (!pi->sh->up)
			break;

		if (pi->nphy_rssisel < 0) {
			phy_utils_phyreg_enter(pi);
			wlc_phy_rssisel_nphy(pi, RADIO_MIMO_CORESEL_OFF, 0);
			phy_utils_phyreg_exit(pi);
		} else {
			int32 rssi_buf[4];
			phy_utils_phyreg_enter(pi);
			wlc_phy_poll_rssi_nphy(pi, (uint8)int_val, rssi_buf, 1);
			phy_utils_phyreg_exit(pi);
		}
		break;

	case IOV_GVAL(IOV_NPHY_RSSICAL): {
		/* if down, return the value, if up, run the cal */
		if (!pi->sh->up) {
			int_val = pi->nphy_rssical;
			bcopy(&int_val, a, vsize);
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		/* run rssi cal */
		wlc_phy_rssi_cal_nphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		int_val = pi->nphy_rssical;
		bcopy(&int_val, a, vsize);
		break;
	}

	case IOV_SVAL(IOV_NPHY_RSSICAL): {
		pi->nphy_rssical = bool_val;
		break;
	}

	case IOV_GVAL(IOV_NPHY_GPIOSEL):
	case IOV_GVAL(IOV_PHY_GPIOSEL):
		*ret_int_ptr = pi->phy_gpiosel;
		break;

	case IOV_SVAL(IOV_NPHY_GPIOSEL):
	case IOV_SVAL(IOV_PHY_GPIOSEL):
		pi->phy_gpiosel = (uint16) int_val;

		if (!pi->sh->up)
			break;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		if (ISNPHY(pi))
			wlc_phy_gpiosel_nphy(pi, (uint16)int_val);
		else if (ISHTPHY(pi))
			wlc_phy_gpiosel_htphy(pi, (uint16)int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_TX_TONE):
		*ret_int_ptr = pi->phy_tx_tone_freq;
		break;

	case IOV_SVAL(IOV_NPHY_TX_TONE):
		wlc_phy_iovar_tx_tone(pi, (uint32)int_val);
		break;

	case IOV_SVAL(IOV_NPHY_GAIN_BOOST):
		pi->nphy_gain_boost = bool_val;
		break;

	case IOV_GVAL(IOV_NPHY_GAIN_BOOST):
		*ret_int_ptr = (int32)pi->nphy_gain_boost;
		break;

	case IOV_SVAL(IOV_NPHY_ELNA_GAIN_CONFIG):
		pi->nphy_elna_gain_config = (int_val != 0) ? TRUE : FALSE;
		break;

	case IOV_GVAL(IOV_NPHY_ELNA_GAIN_CONFIG):
		*ret_int_ptr = (int32)pi->nphy_elna_gain_config;
		break;

	case IOV_GVAL(IOV_NPHY_TEST_TSSI):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 0);
		break;

	case IOV_GVAL(IOV_NPHY_TEST_TSSI_OFFS):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 12);
		break;

#ifdef BAND5G
	case IOV_SVAL(IOV_NPHY_5G_PWRGAIN):
		pi->phy_5g_pwrgain = bool_val;
		break;

	case IOV_GVAL(IOV_NPHY_5G_PWRGAIN):
		*ret_int_ptr = (int32)pi->phy_5g_pwrgain;
		break;
#endif /* BAND5G */

	case IOV_GVAL(IOV_NPHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_NPHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;

#ifndef WLC_DISABLE_ACI
	case IOV_GVAL(IOV_NPHY_ACI_SCAN):
		if (SCAN_INPROG_PHY(pi)) {
			PHY_ERROR(("Scan in Progress, can execute %s\n", __FUNCTION__));
			*ret_int_ptr = -1;
		} else {
			if (pi->cur_interference_mode == INTERFERE_NONE) {
				PHY_ERROR(("interference mode is off\n"));
				*ret_int_ptr = -1;
				break;
			}

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			*ret_int_ptr = wlc_phy_aci_scan_nphy(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		break;
#endif /* Compiling out ACI code for 4324 */
	case IOV_SVAL(IOV_NPHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_NPHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_PAPDCALTYPE):
		if (ISNPHY(pi))
			pi_nphy->nphy_papd_cal_type = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PAPDCAL):
		if (ISNPHY(pi))
			pi_nphy->nphy_force_papd_cal = TRUE;
		int_val = 0;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_NPHY_SKIPPAPD):
		if ((int_val != 0) && (int_val != 1)) {
			err = BCME_RANGE;
			break;
		}
		if (ISNPHY(pi))
			pi_nphy->nphy_papd_skip = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PAPDCALINDEX):
		if (ISNPHY(pi)) {
			*ret_int_ptr = (pi_nphy->nphy_papd_cal_gain_index[0] << 8) |
				pi_nphy->nphy_papd_cal_gain_index[1];
		}
		break;

	case IOV_SVAL(IOV_NPHY_CALTXGAIN): {
		uint8 tx_pwr_ctrl_state;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		if (ISNPHY(pi)) {
			pi_nphy->nphy_cal_orig_pwr_idx[0] =
			        (uint8) ((phy_utils_read_phyreg(pi,
			                  NPHY_Core0TxPwrCtrlStatus) >> 8) & 0x7f);
			pi_nphy->nphy_cal_orig_pwr_idx[1] =
				(uint8) ((phy_utils_read_phyreg(pi,
			                  NPHY_Core1TxPwrCtrlStatus) >> 8) & 0x7f);
		}

		tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

		wlc_phy_cal_txgainctrl_nphy(pi, int_val, TRUE);

		wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);

		break;
	}

	case IOV_GVAL(IOV_NPHY_VCOCAL):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phy_radio205x_vcocal_nphy(pi);
		wlapi_enable_mac(pi->sh->physhim);
		*ret_int_ptr = 0;
		break;

	case IOV_GVAL(IOV_NPHY_TBLDUMP_MINIDX):
		*ret_int_ptr = (int32)pi->nphy_tbldump_minidx;
		break;

	case IOV_SVAL(IOV_NPHY_TBLDUMP_MINIDX):
		pi->nphy_tbldump_minidx = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_TBLDUMP_MAXIDX):
		*ret_int_ptr = (int32)pi->nphy_tbldump_maxidx;
		break;

	case IOV_SVAL(IOV_NPHY_TBLDUMP_MAXIDX):
		pi->nphy_tbldump_maxidx = (int8) int_val;
		break;

	case IOV_SVAL(IOV_NPHY_PHYREG_SKIPDUMP):
		if (pi->nphy_phyreg_skipcnt < 127) {
			pi->nphy_phyreg_skipaddr[pi->nphy_phyreg_skipcnt++] = (uint) int_val;
		}
		break;

	case IOV_GVAL(IOV_NPHY_PHYREG_SKIPDUMP):
		*ret_int_ptr = (pi->nphy_phyreg_skipcnt > 0) ?
			(int32) pi->nphy_phyreg_skipaddr[pi->nphy_phyreg_skipcnt-1] : 0;
		break;

	case IOV_SVAL(IOV_NPHY_PHYREG_SKIPCNT):
		pi->nphy_phyreg_skipcnt = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PHYREG_SKIPCNT):
		*ret_int_ptr = (int32)pi->nphy_phyreg_skipcnt;
		break;
#endif // endif
#endif /* NCONF */

#if defined(WLTEST)

#if LCNCONF
	case IOV_GVAL(IOV_LCNPHY_PAPDEPSTBL):
	{
		lcnphytbl_info_t tab;
		uint32 papdepstbl[PHY_PAPD_EPS_TBL_SIZE_LCNPHY];

		/* Preset PAPD eps table */
		tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
		tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_offset = 0;
		tab.tbl_width = 32;
		tab.tbl_phywidth = 32;
		tab.tbl_ptr = &papdepstbl[0];

		/* read the table */
		wlc_phy_table_read_lcnphy(pi, &tab);
		bcopy(&papdepstbl[0], a, PHY_PAPD_EPS_TBL_SIZE_LCNPHY*sizeof(uint32));
	}
	break;
#endif /* LCNCONF */
#endif // endif

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

uint32
wlc_phy_cap_get(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint32	cap = 0;

	switch (pi->pubpi.phy_type) {
	case PHY_TYPE_N:
		cap |= PHY_CAP_40MHZ;
		if (NREV_GE(pi->pubpi.phy_rev, 3))
			cap |= (PHY_CAP_SGI | PHY_CAP_STBC);
		break;

	case PHY_TYPE_HT:
		cap |= (PHY_CAP_40MHZ | PHY_CAP_SGI | PHY_CAP_STBC | PHY_CAP_LDPC);
		break;

#if (ACCONF || ACCONF2)
	case PHY_TYPE_AC:
		cap |= wlc_phy_ac_caps(pi);
		break;
#endif /* ACCONF || ACCONF2 */

	case PHY_TYPE_LCN:
		cap |= PHY_CAP_SGI;
		break;
	case PHY_TYPE_LCN40:
		cap |= (PHY_CAP_SGI | PHY_CAP_40MHZ | PHY_CAP_STBC);
		break;

	default:
		break;
	}
	return cap;
}

/* %%%%%% IOCTL */
int
wlc_phy_ioctl_dispatch(phy_info_t *pi, int cmd, int len, void *arg, bool *ta_ok)
{
	wlc_phy_t *pih = (wlc_phy_t *)pi;
	int bcmerror = 0;
	int val, *pval;
	bool bool_val;
	uint8 max_aci_mode;

	(void)pih;

	/* default argument is generic integer */
	pval = (int*)arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	else
		val = 0;

	/* bool conversion to avoid duplication below */
	bool_val = (val != 0);
	BCM_REFERENCE(bool_val);

	switch (cmd) {
	case WLC_RESTART:
		break;
	default:
		if ((arg == NULL) || (len <= 0)) {
			PHY_ERROR(("wl%d: %s: Command %d needs arguments\n",
			          pi->sh->unit, __FUNCTION__, cmd));
			return BCME_BADARG;
		}
		break;
	}

	switch (cmd) {

	case WLC_GET_PHY_NOISE:
		ASSERT(pval != NULL);
		*pval = wlc_phy_noise_avg(pih);
		break;

	case WLC_RESTART:
		/* Reset calibration results to uninitialized state in order to
		 * trigger recalibration next time wlc_init() is called.
		 */
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
		phy_type_reset_impl(pi->typei);
		break;

#if defined(BCMDBG)|| defined(WLTEST) || defined(DBG_PHY_IOV)
	case WLC_GET_RADIOREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}
		ASSERT(pval != NULL);

		phy_utils_phyreg_enter(pi);
		phy_utils_radioreg_enter(pi);
		if (val == RADIO_IDCODE)
			*pval = phy_radio_query_idcode(pi->radioi);
		else
			*pval = phy_utils_read_radioreg(pi, (uint16)val);
		phy_utils_radioreg_exit(pi);
		phy_utils_phyreg_exit(pi);
		break;

	case WLC_SET_RADIOREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		phy_utils_phyreg_enter(pi);
		phy_utils_radioreg_enter(pi);
		phy_utils_write_radioreg(pi, (uint16)val, (uint16)(val >> NBITS(uint16)));
		phy_utils_radioreg_exit(pi);
		phy_utils_phyreg_exit(pi);
		break;
#endif // endif

#if defined(BCMDBG)
	case WLC_GET_TX_PATH_PWR:

		*pval = (phy_utils_read_radioreg(pi, RADIO_2050_PU_OVR) & 0x84) ? 1 : 0;
		break;

	case WLC_SET_TX_PATH_PWR:

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		phy_utils_phyreg_enter(pi);
		phy_utils_radioreg_enter(pi);
		if (bool_val) {
			/* Enable overrides */
			phy_utils_write_radioreg(pi, RADIO_2050_PU_OVR,
				0x84 | (phy_utils_read_radioreg(pi, RADIO_2050_PU_OVR) &
				0xf7));
		} else {
			/* Disable overrides */
			phy_utils_write_radioreg(pi, RADIO_2050_PU_OVR,
				phy_utils_read_radioreg(pi, RADIO_2050_PU_OVR) & ~0x84);
		}
		phy_utils_radioreg_exit(pi);
		phy_utils_phyreg_exit(pi);
		break;
#endif /* BCMDBG */

#if defined(BCMDBG) || defined(WLTEST) || defined(WLMEDIA_N2DBG) || \
	defined(DBG_PHY_IOV)
	case WLC_GET_PHYREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		phy_utils_phyreg_enter(pi);

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
			(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
			OSL_DELAY(1);
		}

		ASSERT(pval != NULL);
		*pval = phy_utils_read_phyreg(pi, (uint16)val);

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);

		phy_utils_phyreg_exit(pi);
		break;

	case WLC_SET_PHYREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		phy_utils_phyreg_enter(pi);
		phy_utils_write_phyreg(pi, (uint16)val, (uint16)(val >> NBITS(uint16)));
		phy_utils_phyreg_exit(pi);
		break;
#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_TSSI: {

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		ASSERT(pval != NULL);
		*pval = 0;
		switch (pi->pubpi.phy_type) {
		case PHY_TYPE_LCN:
			PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
			CASECHECK(PHYTYPE, PHY_TYPE_LCN);
			{
				int8 ofdm_pwr = 0, cck_pwr = 0;

				wlc_lcnphy_get_tssi(pi, &ofdm_pwr, &cck_pwr);
				*pval =  ((uint16)ofdm_pwr << 8) | (uint16)cck_pwr;
				break;
			}
		case PHY_TYPE_LCN40:
			PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
			CASECHECK(PHYTYPE, PHY_TYPE_LCN40);
			{
				int8 ofdm_pwr = 0, cck_pwr = 0;

				wlc_lcn40phy_get_tssi(pi, &ofdm_pwr, &cck_pwr);
				*pval =  ((uint16)ofdm_pwr << 8) | (uint16)cck_pwr;
				break;
			}
		case PHY_TYPE_N:
			CASECHECK(PHYTYPE, PHY_TYPE_N);
			{
			*pval = (phy_utils_read_phyreg(pi, NPHY_TSSIBiasVal1) & 0xff) << 8;
			*pval |= (phy_utils_read_phyreg(pi, NPHY_TSSIBiasVal2) & 0xff);
			break;
			}
		}

		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;
	}

	case WLC_GET_ATTEN: {
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

	case WLC_SET_ATTEN: {
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

	case WLC_GET_PWRIDX:
		bcmerror = BCME_UNSUPPORTED;
		break;

	case WLC_SET_PWRIDX:	/* set A band radio/baseband power index */
		bcmerror = BCME_UNSUPPORTED;
		break;

	case WLC_LONGTRAIN:
		{
		longtrnfn_t long_train_fn = NULL;

		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		long_train_fn = pi->pi_fptr.longtrn;
		if (long_train_fn)
			bcmerror = (*long_train_fn)(pi, val);
		else
			PHY_ERROR(("WLC_LONGTRAIN: unsupported phy type\n"));

			break;
		}

	case WLC_EVM:
		ASSERT(arg != NULL);
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		bcmerror = wlc_phy_test_evm(pi, val, *(((uint *)arg) + 1), *(((int *)arg) + 2));
		break;

	case WLC_FREQ_ACCURACY:
		/* SSLPNCONF transmits a few frames before running PAPD Calibration
		 * it does papd calibration each time it enters a new channel
		 * We cannot be down for this reason
		 */
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		bcmerror = wlc_phy_test_freq_accuracy(pi, val);
		break;

	case WLC_CARRIER_SUPPRESS:
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
		bcmerror = wlc_phy_test_carrier_suppress(pi, val);
		break;
#endif // endif

#ifndef WLC_DISABLE_ACI
#if defined(WLTEST) || defined(WL_PHYACIARGS)
	case WLC_GET_ACI_ARGS:
		ASSERT(arg != NULL);
		bcmerror = wlc_phy_aci_args(pi, arg, TRUE, len);
		break;

	case WLC_SET_ACI_ARGS:
		ASSERT(arg != NULL);
		bcmerror = wlc_phy_aci_args(pi, arg, FALSE, len);
		break;

#endif // endif
#endif /* Compiling out ACI code for 4324 */

	case WLC_GET_INTERFERENCE_MODE:
		ASSERT(pval != NULL);
		*pval = pi->sh->interference_mode;
		if (pi->aci_state & ACI_ACTIVE) {
			*pval |= AUTO_ACTIVE;
			*pval |= (pi->aci_active_pwr_level << 4);
		}
		break;

	case WLC_SET_INTERFERENCE_MODE:
		max_aci_mode = ISACPHY(pi) ? ACPHY_ACI_MAX_MODE : WLAN_AUTO_W_NOISE;
		if (val < INTERFERE_NONE || val > max_aci_mode) {
			bcmerror = BCME_RANGE;
			break;
		}

		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))
			val &= (ACPHY_ACI_GLITCHBASED_DESENSE |
					ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION);

		if (pi->sh->interference_mode == val)
			break;
		/* push to sw state */
		pi->sh->interference_mode = val;

		if (!pi->sh->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifndef WLC_DISABLE_ACI
		/* turn interference mode to off before entering another mode */
		if (val != INTERFERE_NONE)
			phy_noise_set_mode(pi->noisei, INTERFERE_NONE, TRUE);

#if defined(RXDESENS_EN)
		if (ISNPHY(pi))
			wlc_nphy_set_rxdesens((wlc_phy_t *)pi, 0);
#endif // endif
		if (phy_noise_set_mode(pi->noisei, pi->sh->interference_mode, TRUE) != BCME_OK)
			bcmerror = BCME_BADOPTION;
#endif /* !defined(WLC_DISABLE_ACI) */

		wlapi_enable_mac(pi->sh->physhim);
		break;

	case WLC_GET_INTERFERENCE_OVERRIDE_MODE:
		if (!(ISACPHY(pi) || ISNPHY(pi) || ISHTPHY(pi) || (ISLCNPHY(pi) &&
			(CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)))) {
			break;
		}

		if (ISNPHY(pi) && !NREV_GE(pi->pubpi.phy_rev, 3)) {
			break;
		}

		ASSERT(pval != NULL);
		if (pi->sh->interference_mode_override == FALSE) {
			*pval = INTERFERE_OVRRIDE_OFF;
		} else {
			*pval = pi->sh->interference_mode;
		}
		break;

	case WLC_SET_INTERFERENCE_OVERRIDE_MODE:
		max_aci_mode = ISACPHY(pi) ? ACPHY_ACI_MAX_MODE : WLAN_AUTO_W_NOISE;
		if (!(ISACPHY(pi) || ISNPHY(pi) || ISHTPHY(pi)	|| (ISLCNPHY(pi) &&
			(CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)))) {
			break;
		}

		if (ISNPHY(pi) && !NREV_GE(pi->pubpi.phy_rev, 3)) {
			break;
		}

		if (val < INTERFERE_OVRRIDE_OFF || val > max_aci_mode) {
			bcmerror = BCME_RANGE;
			break;
		}

		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))
			val &= (ACPHY_ACI_GLITCHBASED_DESENSE |
					ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION);

		bcmerror = wlc_phy_set_interference_override_mode(pi, val);

		break;

	default:
		bcmerror = BCME_UNSUPPORTED;
	}

	return bcmerror;
}

/* Implements core functionality of WLC_SET_INTERFERENCE_OVERRIDE_MODE */
static int
wlc_phy_set_interference_override_mode(phy_info_t* pi, int val)
{
	int bcmerror = 0;

	if (val == INTERFERE_OVRRIDE_OFF) {
		/* this is a reset */
		pi->sh->interference_mode_override = FALSE;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pi->sh->interference_mode =
				pi->sh->interference_mode_2G;
		} else {
			pi->sh->interference_mode =
				pi->sh->interference_mode_5G;
		}
	} else {

		pi->sh->interference_mode_override = TRUE;

		/* push to sw state */
		if (ISACPHY(pi)) {
			pi->sh->interference_mode_2G_override = val;
			pi->sh->interference_mode_5G_override = val;
			pi->sh->interference_mode = CHSPEC_IS2G(pi->radio_chanspec) ?
				pi->sh->interference_mode_2G_override :
				pi->sh->interference_mode_5G_override;
		} else if (ISHTPHY(pi) ||
		          (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) ||
		          (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID))) {
			pi->sh->interference_mode_2G_override = val;
			pi->sh->interference_mode_5G_override = val;
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				/* for 2G, all values 0 thru 4 are valid */
				pi->sh->interference_mode =
					pi->sh->interference_mode_2G_override;
			} else {
				/* for 5G, only values 0 and 1 are valid options */
				if (val == 0 || val == 1) {
					pi->sh->interference_mode =
						pi->sh->interference_mode_5G_override;
				} else {
					/* default 5G interference value to 0 */
					pi->sh->interference_mode = 0;
				}
			}
		} else {
			pi->sh->interference_mode = val;
		}
	}

	if (!pi->sh->up)
		return BCME_NOTUP;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifndef WLC_DISABLE_ACI
	/* turn interference mode to off before entering another mode */
	if (val != INTERFERE_NONE)
		phy_noise_set_mode(pi->noisei, INTERFERE_NONE, TRUE);

	if (phy_noise_set_mode(pi->noisei, pi->sh->interference_mode, TRUE) != BCME_OK)
		bcmerror = BCME_BADOPTION;
#endif // endif

	wlapi_enable_mac(pi->sh->physhim);

	return bcmerror;
}

/* WARNING: check ISSIM_ENAB() before doing any radio related calibrations */
int32
wlc_phy_watchdog(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	/* abort if call should be blocked (e.g. not in home channel) */
	if ((SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || ASSOC_INPROG_PHY(pi)))
		return BCME_OK;

	if (wlapi_bmac_btc_mode_get(pi->sh->physhim))
		wlc_phy_btc_adjust(pi);

	/* PHY specific watchdog callbacks */
	if (pi->pi_fptr.phywatchdog)
		(*pi->pi_fptr.phywatchdog)(pi);

	/* PHY calibration is suppressed until this counter becomes 0 */
	if (pi->cal_info->cal_suppress_count > 0)
		pi->cal_info->cal_suppress_count--;

	return BCME_OK;
}

void
wlc_phy_tx_pwr_limit_check(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	if (!pi->ucode_tssi_limit_en)
		return;

	if (ISLCNPHY(pi))
	  wlc_lcnphy_tx_pwr_limit_check(pi);
}

/* adjust phy setting based on bt states */
static void
wlc_phy_btc_adjust(phy_info_t *pi)
{
	bool btactive = FALSE;
	uint16 btperiod = 0;

	wlapi_bmac_btc_period_get(pi->sh->physhim, &btperiod, &btactive);

	if (btactive != pi->bt_active) {
		if (pi->pi_fptr.phybtcadjust) {
			(*pi->pi_fptr.phybtcadjust)(pi, btactive);
		}
	}

	pi->bt_period = btperiod;
	pi->bt_active = btactive;
}

void
wlc_phy_BSSinit(wlc_phy_t *pih, bool bonlyap, int noise)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint i;
	uint core;

	if (bonlyap) {
		/* PR43338 WAR, we have generic CCK reception problem with ACI of OFDM.
		 * should narrow RC filter and turn off OFDM classification if we are
		 * associate to B ONLY AP
		 */
	}

	/* watchdog idle phy noise */
	for (i = 0; i < MA_WINDOW_SZ; i++) {
		pi->sh->phy_noise_window[i] = (int8)(noise & 0xff);
	}
	pi->sh->phy_noise_index = 0;

	if ((pi->sh->interference_mode == WLAN_AUTO) &&
	     (pi->aci_state & ACI_ACTIVE)) {
		/* Reset the clock to check again after the moving average buffer has filled
		 */
		pi->aci_start_time = pi->sh->now + MA_WINDOW_SZ;
	}

	for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
		FOREACH_CORE(pi, core)
			pi->phy_noise_win[core][i] = PHY_NOISE_FIXED_VAL_NPHY;
	}
	pi->phy_noise_index = 0;
}

/* Convert epsilon table value to complex number */
void
wlc_phy_papd_decode_epsilon(uint32 epsilon, int32 *eps_real, int32 *eps_imag)
{
	if ((*eps_imag = (epsilon>>13)) > 0xfff)
		*eps_imag -= 0x2000; /* Sign extend */
	if ((*eps_real = (epsilon & 0x1fff)) > 0xfff)
		*eps_real -= 0x2000; /* Sign extend */
}

#if defined(PHYCAL_CACHING)
int
wlc_phy_cal_cache_init(wlc_phy_t *ppi)
{
	return 0;
}

void
wlc_phy_cal_cache_deinit(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx = pi->phy_calcache;

	while (ctx) {
		pi->phy_calcache = ctx->next;
		MFREE(pi->sh->osh, ctx,
		      sizeof(ch_calcache_t));
		ctx = pi->phy_calcache;
	}

	pi->phy_calcache = NULL;

	/* No more per-channel contexts, switch in the default one */
	pi->cal_info = pi->def_cal_info;
	if (pi->cal_info != NULL) {
		/* Reset the parameters */
		pi->cal_info->last_cal_temp = -50;
		pi->cal_info->last_cal_time = 0;
		pi->cal_info->last_temp_cal_time = 0;
	}
}
#endif /* defined(PHYCAL_CACHING) */

#if defined(PHYCAL_CACHING) || defined(PHYCAL_CACHE_SMALL)
void
wlc_phy_cal_cache_set(wlc_phy_t *ppi, bool state)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->phy_calcache_on = state;
	if (!state)
		wlc_phy_cal_cache_deinit(ppi);
}

bool
wlc_phy_cal_cache_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->phy_calcache_on;
}
#endif /* defined(PHYCAL_CACHING) || defined(PHYCALCACHE_SMALL) */

void
wlc_phy_cal_mode(wlc_phy_t *ppi, uint mode)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (pi->pi_fptr.calibmodes)
		pi->pi_fptr.calibmodes(pi, mode);
}

void
wlc_phy_set_glacial_timer(wlc_phy_t *ppi, uint period)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->sh->glacial_timer = period;
}

void
wlc_phy_cal_perical_mphase_reset(phy_info_t *pi)
{
	phy_cal_info_t *cal_info = pi->cal_info;

#if defined(PHYCAL_CACHING)
	PHY_CAL(("wlc_phy_cal_perical_mphase_reset chanspec 0x%x ctx: %p\n",
		pi->radio_chanspec, wlc_phy_get_chanctx(pi, pi->radio_chanspec)));
#else
	PHY_CAL(("wlc_phy_cal_perical_mphase_reset\n"));
#endif // endif

	if (pi->phycal_timer)
		wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);

	if (ISNPHY(pi))
		pi->u.pi_nphy->cal_type_override = PHY_PERICAL_AUTO;

	/* resets cal engine */
	cal_info->cal_phase_id = MPHASE_CAL_STATE_IDLE;

	cal_info->txcal_cmdidx = 0; /* needed in nphy only */
}

/* wrapper function for multi phase perical schedule */
void
wlc_phy_mphase_cal_schedule(wlc_phy_t *pih, uint delay_val)
{
	phy_info_t *pi = (phy_info_t*)pih;

	if (!ISNPHY(pi) && !ISHTPHY(pi) && !ISACPHY(pi))
		return;

	if (PHY_PERICAL_MPHASE_PENDING(pi)) {
		wlc_phy_cal_perical_mphase_reset(pi);
	}

	pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_RESTART;
	/* schedule mphase cal */
	wlc_phy_cal_perical_mphase_schedule(pi, delay_val);
}

static void
wlc_phy_cal_perical_mphase_schedule(phy_info_t *pi, uint delay_val)
{
	/* for manual mode, let it run */
	if ((pi->phy_cal_mode != PHY_PERICAL_MPHASE) &&
	    (pi->phy_cal_mode != PHY_PERICAL_MANUAL))
		return;

	PHY_CAL(("wlc_phy_cal_perical_mphase_schedule\n"));

	/* use timer to wait for clean context since this
	 * may be called in the middle of nphy_init
	 */
	wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);

	pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_INIT;
	wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, delay_val, 0);
}

/* policy entry */
void
wlc_phy_cal_perical(wlc_phy_t *pih, uint8 reason)
{
	int16 current_temp = 0, delta_temp = 0;
	uint delta_time = 0;
	bool  suppress_cal = FALSE;

	phy_info_t *pi = (phy_info_t*)pih;

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = NULL;
	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif

	/* reset to default */
	pi->cal_info->cal_suppress_count = 0;

	/* do only init noisecal or trigger noisecal when STA
	 * joins to an AP (also trigger noisecal if AP roams)
	 */
	pi->trigger_noisecal = TRUE;

	/* reset periodic noise poll flag to avoid
	 * race-around condition with cal triggers
	 * between watchdog and others which could
	 * potentially cause sw corruption.
	 */
	pi->capture_periodic_noisestats = FALSE;

	if (!ISNPHY(pi) && !ISHTPHY(pi) && !ISACPHY(pi))
		return;

	if ((pi->phy_cal_mode == PHY_PERICAL_DISABLE) ||
	    (pi->phy_cal_mode == PHY_PERICAL_MANUAL))
		return;

	/* NPHY_IPA : disable PAPD cal for following calibration at least 4322A1? */

	PHY_CAL(("wlc_phy_cal_perical: reason %d chanspec 0x%x\n", reason,
	         pi->radio_chanspec));

	/* Update the Tx power per channel on ACPHY for 2GHz channels */
#ifdef POWPERCHANNL
		if (ISACPHY(pi))
			wlc_phy_tx_target_pwr_per_channel_decide_run_acphy(pi);
#endif /* POWPERCHANNL */

	/* perical is enabled : Either single phase only, or mphase is allowed
	 * Dispatch to s-phase or m-phase based on reasons
	 */
	switch (reason) {

	case PHY_PERICAL_DRIVERUP:	/* always single phase ? */
		break;

	case PHY_PERICAL_PHYINIT:	/* always multi phase */
		if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
#if defined(PHYCAL_CACHING)
			if (ctx)
			{
				/* Switched context so restart a pending MPHASE cal or
				 * restore stored calibration
				 */
				ASSERT(ctx->chanspec == pi->radio_chanspec);

				/* If it was pending last time, just restart it */
				if (PHY_PERICAL_MPHASE_PENDING(pi)) {
					/* Delete any existing timer just in case */
					PHY_CAL(("%s: Restarting calibration for 0x%x phase %d\n",
						__FUNCTION__, ctx->chanspec,
						pi->cal_info->cal_phase_id));
					wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
					wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
				} else if (wlc_phy_cal_cache_restore(pi) != BCME_ERROR) {
					break;
				}
			}
			else
#endif /* PHYCAL_CACHING */
			{
				if (PHY_PERICAL_MPHASE_PENDING(pi))
					wlc_phy_cal_perical_mphase_reset(pi);

				pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_RESTART;

				/* schedule mphase cal */
				wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_INIT_DELAY);
			}
		}
		break;

	case PHY_PERICAL_JOIN_BSS:
	case PHY_PERICAL_START_IBSS:
	case PHY_PERICAL_UP_BSS:
	case PHY_PERICAL_PHYMODE_SWITCH:

		/* These must run in single phase to ensure clean Tx/Rx
		 * performance so the auto-rate fast-start is promising
		 */

		if ((pi->phy_cal_mode == PHY_PERICAL_MPHASE) && PHY_PERICAL_MPHASE_PENDING(pi))
			wlc_phy_cal_perical_mphase_reset(pi);

		/* Always do idle TSSI measurement at the end of NPHY cal
		 * while starting/joining a BSS/IBSS
		 */
		pi->first_cal_after_assoc = TRUE;

		if (ISNPHY(pi))
			pi->u.pi_nphy->cal_type_override = PHY_PERICAL_FULL; /* not used in htphy */

		/* force in-line rx/tx cal before join.  Multiphase cal
		 * could cause loss of packets while mac is suspended
		 * XXX PR63371
		 * wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_ASSOC_DELAY);
		 */

		/* Update last cal temp to current tempsense reading */
		if (pi->phycal_tempdelta) {
			if (ISNPHY(pi))
				pi->cal_info->last_cal_temp = wlc_phy_tempsense_nphy(pi);
			else if (ISHTPHY(pi))
				pi->cal_info->last_cal_temp = wlc_phy_tempsense_htphy(pi);
			else if (ISACPHY(pi))
				pi->cal_info->last_cal_temp = wlc_phy_tempsense_acphy(pi);
		}

		/* Attempt cal cache restore if ctx is valid */
#if defined(PHYCAL_CACHING)
		if (reason == PHY_PERICAL_PHYMODE_SWITCH) {
			wlc_phy_invalidate_chanctx((wlc_phy_t *)pi, pi->radio_chanspec);
		}

		if (ctx)
		{
			PHY_CAL(("wl%d: %s: Attempting to restore cals on JOIN...\n",
				pi->sh->unit, __FUNCTION__));

			if (wlc_phy_cal_cache_restore(pi) == BCME_ERROR) {
				if (ISNPHY(pi))
					wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_FULL);
				else if (ISHTPHY(pi))
					wlc_phy_cals_htphy(pi, PHY_CAL_SEARCHMODE_RESTART);
				else if (ISACPHY(pi))
					wlc_phy_cals_acphy(pi, PHY_CAL_SEARCHMODE_RESTART);
			}
		}
		else
#endif /* PHYCAL_CACHING */
		{
			if (ISNPHY(pi))
				wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_FULL);
			else if (ISHTPHY(pi))
				wlc_phy_cals_htphy(pi, PHY_CAL_SEARCHMODE_RESTART);
			else if (ISACPHY(pi))
				wlc_phy_cals_acphy(pi, PHY_CAL_SEARCHMODE_RESTART);
		}
		break;

	case PHY_PERICAL_WATCHDOG:

		if (PUB_NOT_ASSOC(pi) && ISACPHY(pi))
			return;

		/* Disable periodic noisecal trigger */
		pi->trigger_noisecal = FALSE;

		/* XXX Caputre noise stats at the expiry of watchdog
		 * glacial timer and update the reference crsminpwrs
		 * (which will be relative to the glacial timer).
		 * This will be used in the ACI scheme for comparision
		 * in lieu of baseline crsminpwr values
		 * For more info refer PR110454
		 */
		if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
			pi->capture_periodic_noisestats = TRUE;
		else
			pi->capture_periodic_noisestats = FALSE;

		PHY_CAL(("%s: %sPHY phycal_tempdelta=%d\n", __FUNCTION__,
			(ISNPHY(pi)) ? "N": (ISHTPHY(pi) ? "HT" : (ISACPHY(pi) ? "AC" : "some")),
			pi->phycal_tempdelta));

		if (pi->phycal_tempdelta && (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi))) {

			int cal_chanspec = 0;

			if (ISNPHY(pi)) {
				current_temp = wlc_phy_tempsense_nphy(pi);
				cal_chanspec = pi->cal_info->u.ncal.txiqlocal_chanspec;
			} else if (ISHTPHY(pi)) {
				current_temp = wlc_phy_tempsense_htphy(pi);
				cal_chanspec = pi->cal_info->u.htcal.chanspec;
			} else if (ISACPHY(pi)) {
				if (pi->sh->now - pi->cal_info->last_temp_cal_time >=
					pi->sh->glacial_timer) {
					pi->cal_info->last_temp_cal_time = pi->sh->now;
					current_temp = wlc_phy_tempsense_acphy(pi);
				} else {
					current_temp =  pi->cal_info->last_cal_temp;
				}
				cal_chanspec = pi->cal_info->u.accal.chanspec;
			}

			delta_temp = ((current_temp > pi->cal_info->last_cal_temp) ?
				(current_temp - pi->cal_info->last_cal_temp) :
				(pi->cal_info->last_cal_temp - current_temp));

			/* Only do WATCHDOG triggered (periodic) calibration if
			 * the channel hasn't changed and if the temperature delta
			 * is above the specified threshold
			 */
			PHY_CAL(("%sPHY temp is %d, delta %d, cal_delta %d, chanspec %04x/%04x\n",
				(ISNPHY(pi)) ? "N": (ISHTPHY(pi) ? "HT" :
				(ISACPHY(pi) ? "AC" : "some")),
				current_temp, delta_temp, pi->phycal_tempdelta,
				cal_chanspec, pi->radio_chanspec));

			delta_time = pi->sh->now - pi->cal_info->last_cal_time;

			/* cal_period = 0 implies only temperature based cals */
			if ((delta_temp < pi->phycal_tempdelta) &&
				(((delta_time < pi->cal_period) &&
				(pi->cal_period > 0)) || (pi->cal_period == 0)) &&
				(cal_chanspec == pi->radio_chanspec)) {

				suppress_cal = TRUE;
				pi->cal_info->cal_suppress_count = pi->sh->glacial_timer;
				PHY_CAL(("Suppressing calibration.\n"));

			} else {
				pi->cal_info->last_cal_temp = current_temp;
			}
		}

		if (!suppress_cal) {
			/* if mphase is allowed, do it, otherwise, fall back to single phase */
			if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
				/* only schedule if it's not in progress */
				if (!PHY_PERICAL_MPHASE_PENDING(pi)) {
					pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_REFINE;
					wlc_phy_cal_perical_mphase_schedule(pi,
						PHY_PERICAL_WDOG_DELAY);
				}
			} else if (pi->phy_cal_mode == PHY_PERICAL_SPHASE) {
				if (ISNPHY(pi))
					wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_AUTO);
				else if (ISHTPHY(pi))
					wlc_phy_cals_htphy(pi, PHY_CAL_SEARCHMODE_RESTART);
				else if (ISACPHY(pi))
					wlc_phy_cals_acphy(pi, PHY_CAL_SEARCHMODE_RESTART);
			} else {
				ASSERT(0);
			}
		}
		break;

	case PHY_PERICAL_DCS:

		/* Only applicable for NPHYs */
		ASSERT(ISNPHY(pi));

		if (ISNPHY(pi)) {
			if (PHY_PERICAL_MPHASE_PENDING(pi)) {
				wlc_phy_cal_perical_mphase_reset(pi);

				if (pi->phycal_tempdelta) {
					current_temp = wlc_phy_tempsense_nphy(pi);
					pi->cal_info->last_cal_temp = current_temp;
				}
			} else if (pi->phycal_tempdelta) {

				current_temp = wlc_phy_tempsense_nphy(pi);

				delta_temp = ((current_temp > pi->cal_info->last_cal_temp) ?
					(current_temp - pi->cal_info->last_cal_temp) :
					(pi->cal_info->last_cal_temp - current_temp));

				if ((delta_temp < (int16)pi->phycal_tempdelta)) {
					suppress_cal = TRUE;
				} else {
					pi->cal_info->last_cal_temp = current_temp;
				}
			}

			if (suppress_cal) {
				wlc_phy_txpwr_papd_cal_nphy_dcs(pi);
			} else {
				/* only mphase is allowed */
				if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
					pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_REFINE;
					wlc_phy_cal_perical_mphase_schedule(pi,
						PHY_PERICAL_WDOG_DELAY);
				} else {
					ASSERT(0);
				}
			}
		}
		break;

	default:
		ASSERT(0);
		break;
	}
}

void
wlc_phy_cal_perical_mphase_restart(phy_info_t *pi)
{
	PHY_CAL(("wlc_phy_cal_perical_mphase_restart\n"));
	pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_INIT;
	pi->cal_info->txcal_cmdidx = 0;
}

#ifdef WL_SARLIMIT
void
wlc_phy_sar_limit_set(wlc_phy_t *pih, uint32 int_val)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint core;
	int8 sar[PHY_MAX_CORES];

	FOREACH_CORE(pi, core) {
		sar[core] = (int8)(((int_val) >> (core * 8)) & 0xff);
	}
	/* internal */
	wlc_phy_sarlimit_set_int(pi, sar);
}
#endif /* WL_SARLIMIT */

#ifdef WL_SAR_SIMPLE_CONTROL
void
wlc_phy_sar_limit_set_percore(wlc_phy_t *pih, uint32 uint_val)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint core;

	FOREACH_CORE(pi, core) {
		if (((uint_val) >> (core * SAR_VAL_LENG)) & SAR_ACTIVEFLAG_MASK) {
			pi->sarlimit[core] =
				(int8)(((uint_val) >> (core * SAR_VAL_LENG)) & SAR_VAL_MASK);
		} else {
			pi->sarlimit[core] = WLC_TXPWR_MAX;
		}
	}

	if (ISACPHY(pi) && pi->sh->clk) {
		wlc_phy_set_sarlimit_acphy(pi);
	}
}
#endif /* WL_SAR_SIMPLE_CONTROL */

void
wlc_phy_stf_chain_init(wlc_phy_t *pih, uint8 txchain, uint8 rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;

	pi->sh->hw_phytxchain = txchain;
	pi->sh->hw_phyrxchain = rxchain;
	if (pi->sromi->sr13_en_sw_txrxchain_mask) {
		pi->sh->phytxchain = txchain & pi->sromi->sw_txchain_mask;
		pi->sh->phyrxchain = rxchain & pi->sromi->sw_rxchain_mask;
	} else {
		pi->sh->phytxchain = txchain;
		pi->sh->phyrxchain = rxchain;
	}
}

void
wlc_phy_stf_chain_set(wlc_phy_t *pih, uint8 txchain, uint8 rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;

	PHY_TRACE(("wlc_phy_stf_chain_set, new phy chain tx %d, rx %d", txchain, rxchain));

	pi->sh->phytxchain = txchain;

	if (ISNPHY(pi)) {
		if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV))
			wlc_phy_rxcore_setstate_nphy(pih, rxchain, 1);
		else
			wlc_phy_rxcore_setstate_nphy(pih, rxchain, 0);

	} else if (ISHTPHY(pi)) {
		wlc_phy_rxcore_setstate_htphy(pih, rxchain);
	} else if (ISACPHY(pi)) {
		wlc_phy_rxcore_setstate_acphy(pih, rxchain);
	}
}

void
wlc_phy_stf_chain_get(wlc_phy_t *pih, uint8 *txchain, uint8 *rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;

	*txchain = pi->sh->phytxchain;
	*rxchain = pi->sh->phyrxchain;
}

void
wlc_phy_stf_chain_get_valid(phy_info_t *pi, uint8 *txchain, uint8 *rxchain)
{
	*txchain = pi->sh->hw_phytxchain;
	*rxchain = pi->sh->hw_phyrxchain;
	if (pi->sromi->sr13_en_sw_txrxchain_mask) {
		*txchain &= pi->sromi->sw_txchain_mask;
		*rxchain &= pi->sromi->sw_rxchain_mask;
	}
}

uint8
wlc_phy_stf_chain_active_get(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	ASSERT(pi->tempi != NULL);
	return phy_temp_throttle(pi->tempi);
}

int8
wlc_phy_stf_ssmode_get(wlc_phy_t *pih, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)pih;
	int8* mcs_limits_1;
	int8* mcs_limits_2;

	PHY_TRACE(("wl%d: %s: chanspec %x\n", pi->sh->unit, __FUNCTION__, chanspec));

	/* criteria to choose stf mode */

	/* the "+3dbm (12 0.25db units)" is to account for the fact that with CDD, tx occurs
	 * on both chains
	 */
	if (CHSPEC_IS40(chanspec)) {
		mcs_limits_1 = &pi->b40_1x1mcs0;
		mcs_limits_2 = &pi->b40_1x2cdd_mcs0;
	} else {
		mcs_limits_1 = &pi->b20_1x1mcs0;
		mcs_limits_2 = &pi->b20_1x2cdd_mcs0;
	}
	if (*mcs_limits_1 > *mcs_limits_2 + 12)
		return PHY_TXC1_MODE_SISO;
	else
		return PHY_TXC1_MODE_CDD;
}

#ifdef WLTEST
void
wlc_phy_boardflag_upd(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if ((ISNPHY(pi)) && NREV_GE(pi->pubpi.phy_rev, 3)) {
		/* PR 64616 fix: Move nphy_aband_spurwar_en from nphy_init to phy_attach.
		   wlc_phy_chanspec_nphy_setup uses this flag and is called before nphy_init,
		   and thus we need to initialize this flag before
		   calling wlc_phy_chanspec_nphy_setup
		*/
		/* Check if A-band spur WAR should be enabled for this board */
		if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_SPUR_WAR) {
			PHY_ERROR(("%s: aband_spurwar on\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_aband_spurwar_en = TRUE;
		} else {
			PHY_ERROR(("%s: aband_spurwar off\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_aband_spurwar_en = FALSE;
		}
	}

	if ((ISNPHY(pi)) && NREV_GE(pi->pubpi.phy_rev, 6)) {
		/* Check if extra G-band spur WAR for 40 MHz channels 3 through 10
		 * should be enabled for this board
		 */
		if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_2G_SPUR_WAR) {
			PHY_ERROR(("%s: gband_spurwar2 on\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_gband_spurwar2_en = TRUE;
		} else {
			PHY_ERROR(("%s: gband_spurwar2 off\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_gband_spurwar2_en = FALSE;
		}
	}

	pi->nphy_txpwrctrl = PHY_TPC_HW_OFF;
	pi->txpwrctrl = PHY_TPC_HW_OFF;
	pi->phy_5g_pwrgain = FALSE;

	if (pi->sh->boardvendor == VENDOR_APPLE &&
	    (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))) {

		pi->nphy_txpwrctrl =  PHY_TPC_HW_ON;
		pi->phy_5g_pwrgain = TRUE;

	} else if ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_TXPWRCTRL_EN) &&
		NREV_GE(pi->pubpi.phy_rev, 2) && (pi->sh->sromrev >= 4)) {

		pi->nphy_txpwrctrl = PHY_TPC_HW_ON;

	} else if ((pi->sh->sromrev >= 4) &&
		(BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_5G_PWRGAIN)) {
		pi->phy_5g_pwrgain = TRUE;
	}
}
#endif /* WLTEST */
#if defined(WLTEST)
/*  FA009736 - PD Test Failure WAR */
void
wlc_phy_resetcntrl_regwrite(wlc_phy_t *pih)
{
#if LCNCONF
	phy_info_t *pi = (phy_info_t *)pih;
	/* enable rfseqSoftReset bit */
	phy_utils_write_phyreg(pi, LCNPHY_resetCtrl, 0x088);
	OSL_DELAY(5);
	/* disable rfseqSoftReset bit and write default value 0x80  */
	phy_utils_write_phyreg(pi, LCNPHY_resetCtrl, 0x080);
	OSL_DELAY(5);
	wlc_lcnphy_4313war(pi);
#endif /* LCNCONF */
}

int
wlc_phy_set_po_htphy(phy_info_t *pi, wl_po_t *inpo)
{
	int err = BCME_OK;

	if ((inpo->band > WL_CHAN_FREQ_RANGE_5G_BAND3))
		err  = BCME_BADARG;
	else
	{
		if (inpo->band == WL_CHAN_FREQ_RANGE_2G)
		{
			pi->ppr.sr9.cckbw202gpo = inpo->cckpo;
			pi->ppr.sr9.cckbw20ul2gpo = inpo->cckpo;
		}
		pi->ppr.sr9.ofdm[inpo->band].bw20 = inpo->ofdmpo;
		pi->ppr.sr9.ofdm[inpo->band].bw20ul = inpo->ofdmpo;
		pi->ppr.sr9.ofdm[inpo->band].bw40 = 0;
		pi->ppr.sr9.mcs[inpo->band].bw20 =
			(inpo->mcspo[1] << 16) | inpo->mcspo[0];
		pi->ppr.sr9.mcs[inpo->band].bw20ul =
			(inpo->mcspo[3] << 16) | inpo->mcspo[2];
		pi->ppr.sr9.mcs[inpo->band].bw40 =
			(inpo->mcspo[5] << 16) | inpo->mcspo[4];
	}
	return err;
}

int
wlc_phy_get_po_htphy(phy_info_t *pi, wl_po_t *outpo)
{
	int err = BCME_OK;

	if ((outpo->band > WL_CHAN_FREQ_RANGE_5G_BAND3))
		err  = BCME_BADARG;
	else
	{
		if (outpo->band == WL_CHAN_FREQ_RANGE_2G)
			outpo->cckpo = pi->ppr.sr9.cckbw202gpo;

		outpo->ofdmpo = pi->ppr.sr9.ofdm[outpo->band].bw20;
		outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs[outpo->band].bw20;
		outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs[outpo->band].bw20 >>16);
		outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs[outpo->band].bw20ul;
		outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs[outpo->band].bw20ul >>16);
		outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs[outpo->band].bw40;
		outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs[outpo->band].bw40 >>16);
	}

	return err;
}

#endif // endif
/* %%%%%% dump */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

int
wlc_phydump_phycal(phy_info_t *pi, struct bcmstrbuf *b)
{

	if (!pi->sh->up)
		return BCME_NOTUP;

	if (!(ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)))
		return BCME_UNSUPPORTED;

	/* for HTPHY, branch out to htphy phycal dump routine */
	if (ISACPHY(pi)) {
		wlc_phy_cal_dump_acphy(pi, b);
	} else if (ISHTPHY(pi)) {
		wlc_phy_cal_dump_htphy(pi, b);
	} else if (ISNPHY(pi)) {
		wlc_phy_cal_dump_nphy(pi, b);
	}

#if defined(PHYCAL_CACHING) && defined(BCMDBG)
	{
	  if (!ISNPHY(pi) && !ISHTPHY(pi) && !ISACPHY(pi))
			return BCME_OK;
	  wlc_phydump_chanctx(pi, b);
	}
#endif // endif

	return BCME_OK;
}

int
wlc_phydump_papd(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint32 val, i, j;
	int32 eps_real, eps_imag;

	eps_real = eps_imag = 0;

	if (!pi->sh->up)
		return BCME_NOTUP;

	if (!(ISNPHY(pi) || ISLCNCOMMONPHY(pi)))
		return BCME_UNSUPPORTED;

	/*
	* XXX PR41476 WAR: Prevent MAC from accessing PHY registers while the host is
	* For Corerev 11 and 12, make sure that
	*       * either the MAC is disabled, or
	*       * MCTL_PHYLOCK is set that ucode checks before reading phyreg
	* This prevents collision for phy read access between Host and Ucode
	*/

	if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
		(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
		OSL_DELAY(1);
	}

	if (ISNPHY(pi))
	{
		bcm_bprintf(b, "papd eps table:\n [core 0]\t\t[core 1] \n");
		for (j = 0; j < 64; j++) {
			for (i = 0; i < 2; i++) {
				wlc_phy_table_read_nphy(pi, ((i == 0) ? NPHY_TBL_ID_EPSILONTBL0 :
					NPHY_TBL_ID_EPSILONTBL1), 1, j, 32, &val);
				wlc_phy_papd_decode_epsilon(val, &eps_real, &eps_imag);
				bcm_bprintf(b, "{%d\t%d}\t\t", eps_real, eps_imag);
			}
		bcm_bprintf(b, "\n");
		}
		bcm_bprintf(b, "\n\n");
	}
	else if (ISLCNPHY(pi))
	{
		wlc_lcnphy_read_papdepstbl(pi, b);

	} else if (ISLCN40PHY(pi))
		wlc_lcn40phy_read_papdepstbl(pi, b);

	if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK, 0);

	return BCME_OK;
}

int
wlc_phydump_state(phy_info_t *pi, struct bcmstrbuf *b)
{
#ifndef PPR_API
	char fraction[4][4] = {"  ", ".25", ".5", ".75"};
	int i;
	bool isphyhtcap = ISPHY_HT_CAP(pi);
	uint offset;

#define QDB_FRAC(x)	(x) / WLC_TXPWR_DB_FACTOR, fraction[(x) % WLC_TXPWR_DB_FACTOR]

	bcm_bprintf(b, "phy_type %d phy_rev %d ana_rev %d radioid 0x%x radiorev 0x%x\n",
	               pi->pubpi.phy_type, pi->pubpi.phy_rev, pi->pubpi.ana_rev,
	               pi->pubpi.radioid, pi->pubpi.radiorev);

	bcm_bprintf(b, "hw_power_control %d\n",
	               pi->hwpwrctrl);

	bcm_bprintf(b, "Power targets: ");
	/* CCK Power/Rate */
	bcm_bprintf(b, "\n\tCCK: ");
	for (i = 0; i < WL_NUM_RATES_CCK; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_CCK]));
	/* OFDM Power/Rate */
	bcm_bprintf(b, "\n\tOFDM 20MHz SISO: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM]));
	bcm_bprintf(b, "\n\tOFDM 20MHz CDD: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM_20_CDD]));
	/* 20MHz MCS Power/Rate */
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS0-7 20MHz 1 Tx: " : "\n\tMCS0-7 20MHz SISO: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_20_S1x1 : TXP_FIRST_MCS_20_SISO;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS0-7 20MHz 2 Tx: " : "\n\tMCS0-7 20MHz CDD: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_20_S1x2 : TXP_FIRST_MCS_20_CDD;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS0-7 20MHz 3 Tx: " : "\n\tMCS0-7 20MHz STBC: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_20_S1x3 : TXP_FIRST_MCS_20_STBC;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS8-15 20MHz 2 Tx: " : "\n\tMCS0-7 20MHz SDM: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_20_S2x2 : TXP_FIRST_MCS_20_SDM;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS8-15 20MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && isphyhtcap; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_20_S2x3]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS16-23 20MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && isphyhtcap; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_20_S3x3]));

	/* 40MHz OFDM Power/Rate */
	bcm_bprintf(b, "\n\tOFDM 40MHz SISO: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM_40_SISO]));
	bcm_bprintf(b, "\n\tOFDM 40MHz CDD: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM_40_CDD]));
	/* 40MHz MCS Power/Rate */
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS0-7 40MHz 1 Tx: " : "\n\tMCS0-7 40MHz SISO: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_40_S1x1 : TXP_FIRST_MCS_40_SISO;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS0-7 40MHz 2 Tx: " : "\n\tMCS0-7 40MHz CDD: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_40_S1x2 : TXP_FIRST_MCS_40_CDD;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS0-7 40MHz 3 Tx: " : "\n\tMCS0-7 40MHz STBC: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_40_S1x3 : TXP_FIRST_MCS_40_STBC;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS8-15 40MHz 2 Tx: " : "\n\tMCS0-7 40MHz SDM: ");
	offset = isphyhtcap ? TXP_FIRST_MCS_40_S2x2 : TXP_FIRST_MCS_40_SDM;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS8-15 40MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && isphyhtcap; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_40_S2x3]));
	bcm_bprintf(b, isphyhtcap ? "\n\tMCS16-23 40MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && isphyhtcap; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_40_S3x3]));

	/* MCS 32 Power */
	bcm_bprintf(b, "\n\tMCS32: %2d%s\n\n", QDB_FRAC(pi->tx_power_target[i]));

	if (isphyhtcap)
		goto next;

	/* CCK Power/Rate */
#if HTCONF
	bcm_bprintf(b, "\n\tCCK 20UL: ");
	for (i = 0; i < WL_NUM_RATES_CCK; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_CCK]));

	/* 20 in 40MHz OFDM Power/Rate */
	bcm_bprintf(b, "\n\tOFDM 20UL SISO: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_OFDM]));
	bcm_bprintf(b, "\n\tOFDM 40MHz CDD: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_OFDM_CDD]));

	/* 20 in 40MHz MCS Power/Rate */
	bcm_bprintf(b, "\n\tMCS0-7 20UL 1 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_S1x1]));
	bcm_bprintf(b, "\n\tMCS0-7 20UL 2 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_S1x2]));
	bcm_bprintf(b, "\n\tMCS0-7 20UL 3 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_S1x3]));
	bcm_bprintf(b, "\n\tMCS8-15 20UL 2 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_LAST_20UL_S2x2]));
	bcm_bprintf(b, "\n\tMCS8-15 20UL 3 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && isphyhtcap; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_LAST_20UL_S2x3]));
	bcm_bprintf(b, "\n\tMCS16-23 20UL 3 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && isphyhtcap; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_LAST_20UL_S3x3]));
#endif /* HTCONF */

next:
	if (ISNPHY(pi)) {
		bcm_bprintf(b, "antsel_type %d\n", pi->antsel_type);
		bcm_bprintf(b, "ipa2g %d ipa5g %d\n", pi->ipa2g_on, pi->ipa5g_on);

	} else if (ISLCNPHY(pi) || ISLCN40PHY(pi) || ISHTPHY(pi)) {
		return;
	}

	bcm_bprintf(b, "\ninterference_mode %d intf_crs %d\n",
		pi->sh->interference_mode, pi->interference_mode_crs);
#endif /* !PPR_API */

	return BCME_OK;
}

int
wlc_phydump_lnagain(phy_info_t *pi, struct bcmstrbuf *b)
{
	int core;
	uint16 lnagains[2][4];
	uint16 mingain[2];
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

	/* Now, read the gain table */
	for (core = 0; core < 2; core++) {
		wlc_phy_table_read_nphy(pi, core, 4, 8, 16, &lnagains[core][0]);
	}

	mingain[0] =
		(phy_utils_read_phyreg(pi, NPHY_Core1MinMaxGain) &
		NPHY_CoreMinMaxGain_minGainValue_MASK) >>
		NPHY_CoreMinMaxGain_minGainValue_SHIFT;
	mingain[1] =
		(phy_utils_read_phyreg(pi, NPHY_Core2MinMaxGain) &
		NPHY_CoreMinMaxGain_minGainValue_MASK) >>
		NPHY_CoreMinMaxGain_minGainValue_SHIFT;

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	bcm_bprintf(b, "Core 0: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
		lnagains[0][0], lnagains[0][1], lnagains[0][2], lnagains[0][3]);
	bcm_bprintf(b, "Core 1: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n\n",
		lnagains[1][0], lnagains[1][1], lnagains[1][2], lnagains[1][3]);
	bcm_bprintf(b, "Min Gain: Core 0=0x%02x,   Core 1=0x%02x\n\n",
		mingain[0], mingain[1]);

	return BCME_OK;
}

int
wlc_phydump_initgain(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint8 ctr;
	uint16 regval[2], tblregval[4];
	uint16 lna_gain[2], hpvga1_gain[2], hpvga2_gain[2];
	uint16 tbl_lna_gain[4], tbl_hpvga1_gain[4], tbl_hpvga2_gain[4];
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

	regval[0] = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCode);
	regval[1] = phy_utils_read_phyreg(pi, NPHY_Core2InitGainCode);

	wlc_phy_table_read_nphy(pi, 7, PHYCORENUM(pi->pubpi.phy_corenum), 0x106, 16, tblregval);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	lna_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initLnaIndex_MASK) >>
		NPHY_CoreInitGainCode_initLnaIndex_SHIFT;
	hpvga1_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initHpvga1Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga1Index_SHIFT;
	hpvga2_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initHpvga2Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga2Index_SHIFT;

	lna_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initLnaIndex_MASK) >>
		NPHY_CoreInitGainCode_initLnaIndex_SHIFT;
	hpvga1_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initHpvga1Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga1Index_SHIFT;
	hpvga2_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initHpvga2Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga2Index_SHIFT;

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_lna_gain[ctr] = (tblregval[ctr] >> 2) & 0x3;
	}

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_hpvga1_gain[ctr] = (tblregval[ctr] >> 4) & 0xf;
	}

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_hpvga2_gain[ctr] = (tblregval[ctr] >> 8) & 0x1f;
	}

	bcm_bprintf(b, "Core 0 INIT gain: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
		hpvga2_gain[0], hpvga1_gain[0], lna_gain[0]);
	bcm_bprintf(b, "Core 1 INIT gain: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
		hpvga2_gain[1], hpvga1_gain[1], lna_gain[1]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "INIT gain table:\n");
	bcm_bprintf(b, "----------------\n");
	for (ctr = 0; ctr < 4; ctr++) {
		bcm_bprintf(b, "Core %d: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
			ctr, tbl_hpvga2_gain[ctr], tbl_hpvga1_gain[ctr], tbl_lna_gain[ctr]);
	}

	return BCME_OK;
}

int
wlc_phydump_hpf1tbl(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint8 ctr, core;
	uint16 gain[2][NPHY_MAX_HPVGA1_INDEX+1];
	uint16 gainbits[2][NPHY_MAX_HPVGA1_INDEX+1];
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

	/* Read from the HPVGA1 gaintable */
	wlc_phy_table_read_nphy(pi, 0, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gain[0][0]);
	wlc_phy_table_read_nphy(pi, 1, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gain[1][0]);
	wlc_phy_table_read_nphy(pi, 2, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gainbits[0][0]);
	wlc_phy_table_read_nphy(pi, 3, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gainbits[1][0]);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	for (core = 0; core < 2; core++) {
		bcm_bprintf(b, "Core %d gain: ", core);
		for (ctr = 0; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
			bcm_bprintf(b, "%2d ", gain[core][ctr]);
		}
		bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, "\n");
	for (core = 0; core < 2; core++) {
		bcm_bprintf(b, "Core %d gainbits: ", core);
		for (ctr = 0; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
			bcm_bprintf(b, "%2d ", gainbits[core][ctr]);
		}
		bcm_bprintf(b, "\n");
	}

	return BCME_OK;
}

int
wlc_phydump_chanest(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint16 num_rx, num_sts, num_tones;
	uint16 k, r, t, fftk;
	uint32 ch;
	uint16 ch_re_ma, ch_im_ma;
	uint8  ch_re_si, ch_im_si;
	int16  ch_re, ch_im;
	int8   ch_exp;

	if (!(ISHTPHY(pi) || ISACPHY(pi) || ISNPHY(pi)))
		return BCME_UNSUPPORTED;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* Go deaf to prevent PHY channel writes while doing reads */
	if (ISNPHY(pi))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);
	else if (ISHTPHY(pi))
		wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
	else if (ISACPHY(pi))
		wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	num_rx = (uint8)PHYCORENUM(pi->pubpi.phy_corenum);
	num_sts = 4;

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		num_tones = 128;
#ifdef CHSPEC_IS80
	} else if (CHSPEC_IS80(pi->radio_chanspec)) {
		num_tones = 256;
#endif /* CHSPEC_IS80 */
	} else {
		num_tones = 64;
	}

	for (k = 0; k < num_tones; k++) {
		for (r = 0; r < num_rx; r++) {
			for (t = 0; t < num_sts; t++) {
				if (ISNPHY(pi)) {
					wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_CHANEST, 1,
					                         t*128 + k, 32, &ch);
				} else if (ISHTPHY(pi)) {
					wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_CHANEST(r), 1,
					                         t*128 + k, 32, &ch);
				} else if (ISACPHY(pi)) {
					wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_CHANEST(r), 1,
					                         t*256 + k, 32, &ch);
				}
				ch_re_ma  = ((ch >> 18) & 0x7ff);
				ch_re_si  = ((ch >> 29) & 0x001);
				ch_im_ma  = ((ch >>  6) & 0x7ff);
				ch_im_si  = ((ch >> 17) & 0x001);
				ch_exp    = ((int8)((ch << 2) & 0xfc)) >> 2;

				ch_re = (ch_re_si > 0) ? -ch_re_ma : ch_re_ma;
				ch_im = (ch_im_si > 0) ? -ch_im_ma : ch_im_ma;

				fftk = ((k < num_tones/2) ? (k + num_tones/2) : (k - num_tones/2));

				bcm_bprintf(b, "chan(%d,%d,%d)=(%d+i*%d)*2^%d;\n",
				            r+1, t+1, fftk+1, ch_re, ch_im, ch_exp);
			}
		}
	}

	/* Return from deaf */
	if (ISNPHY(pi))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
	else if (ISHTPHY(pi))
		wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
	else if (ISACPHY(pi))
		wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}

#ifdef ENABLE_FCBS
int
wlc_phydump_fcbs(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint shmem_radioreg, shmem_phytbl16, shmem_phytbl32;
	uint shmem_phyreg, shmem_bphyctrl, shmem_cache_ptr;
	int bwidx[2];
	int len, cache_consumed;
	char *bwstr [ ] = {"", "20MHz", "40MHz"};

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	shmem_radioreg = (pi->phy_fcbs->shmem_radioreg != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_radioreg) : 0;

	shmem_phytbl16 = (pi->phy_fcbs->shmem_phytbl16 != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_phytbl16) : 0;

	shmem_phytbl32 = (pi->phy_fcbs->shmem_phytbl32 != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_phytbl32) : 0;

	shmem_phyreg = (pi->phy_fcbs->shmem_phyreg != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_phyreg) : 0;

	shmem_bphyctrl = (pi->phy_fcbs->shmem_bphyctrl != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_bphyctrl) : 0;

	shmem_cache_ptr = (pi->phy_fcbs->shmem_cache_ptr != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr) : 0;

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	bcm_bprintf(b, "FCBS h/w addresses:\n");
	bcm_bprintf(b, "-------------------\n");
	bcm_bprintf(b, "RAM start address      = 0x%04x\n", pi->phy_fcbs->cache_startaddr);
	bcm_bprintf(b, "SHMEM radio address    = 0x%04x\n", pi->phy_fcbs->shmem_radioreg);
	bcm_bprintf(b, "SHMEM phytbl16 address = 0x%04x\n", pi->phy_fcbs->shmem_phytbl16);
	bcm_bprintf(b, "SHMEM phytbl32 address = 0x%04x\n", pi->phy_fcbs->shmem_phytbl32);
	bcm_bprintf(b, "SHMEM phyreg address   = 0x%04x\n", pi->phy_fcbs->shmem_phyreg);
	bcm_bprintf(b, "SHMEM bphyctrl address = 0x%04x\n", pi->phy_fcbs->shmem_bphyctrl);
	bcm_bprintf(b, "SHMEM cacheptr address = 0x%04x\n\n", pi->phy_fcbs->shmem_cache_ptr);

	bcm_bprintf(b, "FCBS shmem values:\n");
	bcm_bprintf(b, "------------------\n");
	bcm_bprintf(b, "radioreg  = %d\n", shmem_radioreg);
	bcm_bprintf(b, "phytbl16  = %d\n", shmem_phytbl16);
	bcm_bprintf(b, "phytbl32  = %d\n", shmem_phytbl32);
	bcm_bprintf(b, "phyreg    = %d\n", shmem_phyreg);
	bcm_bprintf(b, "bphyctrl  = 0x%04x\n", shmem_bphyctrl);
	bcm_bprintf(b, "cacheptr  = 0x%04x\n\n", shmem_cache_ptr);

	cache_consumed = 0;
	bcm_bprintf(b, "FCBS cache consumption:\n");
	bcm_bprintf(b, "-----------------------\n");
	len = pi->phy_fcbs->num_radio_regs * 2 * sizeof(uint16);
	bcm_bprintf(b, "Radio reg cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "   CHAN_B = %d bytes\n", len);
	cache_consumed += len;

	len = pi->phy_fcbs->phytbl16_buflen;
	if (((len/4) * 4) != len) {
		len += 2;
	}
	bcm_bprintf(b, "PHYTBL16 cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "   CHAN_B = %d bytes\n", len);
	cache_consumed += len;

	len = pi->phy_fcbs->phytbl32_buflen;
	bcm_bprintf(b, "PHYTBL32 cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "   CHAN_B = %d bytes\n", len);
	cache_consumed += len;

	len = pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_A];
	bcm_bprintf(b, "PHY reg cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	len = pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_B];
	bcm_bprintf(b, "   CHAN_B = %d bytes\n\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "Total cache used by FCBS = %d bytes\n\n", cache_consumed);

	bcm_bprintf(b, "FCBS reg and table entry count:\n");
	bcm_bprintf(b, "-------------------------------\n");
	bcm_bprintf(b, "Radio regs       = %d\n", pi->phy_fcbs->num_radio_regs);
	bcm_bprintf(b, "PHYTBL16 entries = %d\n", pi->phy_fcbs->phytbl16_entries);
	bcm_bprintf(b, "PHYTBL32 entries = %d\n", pi->phy_fcbs->phytbl32_entries);
	bcm_bprintf(b, "PHY regs         = %d\n", pi->phy_fcbs->num_phy_regs);
	bcm_bprintf(b, "BPHY regs:\n");
	bcm_bprintf(b, "   CHAN_A = %d\n", pi->phy_fcbs->num_bphy_regs[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = %d\n\n", pi->phy_fcbs->num_bphy_regs[FCBS_CHAN_B]);

	bcm_bprintf(b, "FCBS cache offsets:\n");
	bcm_bprintf(b, "-------------------\n");
	bcm_bprintf(b, "Channel cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "Radio reg cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->radioreg_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->radioreg_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "PHYTBL16 cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->phytbl16_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->phytbl16_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "PHYTBL32 cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->phytbl32_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->phytbl32_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "PHY reg cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->phyreg_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->phyreg_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "BPHY reg cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->bphyreg_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n\n", pi->phy_fcbs->bphyreg_cache_offset[FCBS_CHAN_B]);

	if (pi->phy_fcbs->initialized[FCBS_CHAN_A]) {
		if (CHSPEC_IS40(pi->phy_fcbs->chanspec[FCBS_CHAN_A])) {
			bwidx[0] = 2;
		} else {
			bwidx[0] = 1;
		}
	} else {
		bwidx[0] = 0;
	}

	if (pi->phy_fcbs->initialized[FCBS_CHAN_B]) {
		if (CHSPEC_IS40(pi->phy_fcbs->chanspec[FCBS_CHAN_B])) {
			bwidx[1] = 2;
		} else {
			bwidx[1] = 1;
		}
	} else {
		bwidx[1] = 0;
	}
	bcm_bprintf(b, "FCBS driver internal:\n");
	bcm_bprintf(b, "---------------------\n");
	bcm_bprintf(b, "Initialized   : CHAN_A=%3d, CHAN_B=%3d\n",
	    pi->phy_fcbs->initialized[FCBS_CHAN_A], pi->phy_fcbs->initialized[FCBS_CHAN_B]);
	bcm_bprintf(b, "Channels      : CHAN_A=%3d, CHAN_B=%3d\n",
	    CHSPEC_CHANNEL(pi->phy_fcbs->chanspec[FCBS_CHAN_A]),
	    CHSPEC_CHANNEL(pi->phy_fcbs->chanspec[FCBS_CHAN_B]));
	bcm_bprintf(b, "Bandwidth     : CHAN_A=%s, CHAN_B=%s\n", bwstr[bwidx[0]], bwstr[bwidx[1]]);
	bcm_bprintf(b, "Channel index : %d\n", pi->phy_fcbs->curr_fcbs_chan);
	bcm_bprintf(b, "Switch count  : %d\n", pi->phy_fcbs->switch_count);
	bcm_bprintf(b, "Load regs/tbls: %d\n", pi->phy_fcbs->load_regs_tbls);

	return BCME_OK;
}
#endif /* ENABLE_FCBS */

int
wlc_phydump_txv0(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint16 k;
	uint32 tbl_val;
	uint8 stall_val;

	if (!ISACPHY(pi))
		return BCME_UNSUPPORTED;

	phy_utils_phyreg_enter(pi);

	/* disable stall */
	stall_val = (phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev)) &
	             ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev))
	        >> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev);
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
	            ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
	            1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));

	/* id=16, depth=1952 words for usr 0, width=32 */
	for (k = 0; k < 1952; k++) {
		wlc_phy_table_read_acphy(pi, 16, 1, k, 32, &tbl_val);
		bcm_bprintf(b, "0x%08x\n", tbl_val);
	}

	/* restore stall value */
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
	            ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
	            stall_val << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));

	phy_utils_phyreg_exit(pi);

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef WLTEST
int
wlc_phydump_ch4rpcal(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint16 num_tones;
	uint16 k, r, fftk, fft_size;
	uint32 ch, tbl_id;
	uint16 ch_re_ma, ch_im_ma;
	uint8  ch_re_si, ch_im_si;
	int16  ch_re, ch_im;
	uint16 *tone_idx_tbl;
	uint16 tone_idx_20[52]  = {
		4,  5,  6,  7,  8,  9, 10, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 31,
		33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 44, 45, 46,
		47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60};

	uint16 tone_idx_40[108] = {
		6,  7,  8,  9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
		25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62,
		66, 67, 68, 69, 70, 71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 83, 84,
		85, 86, 87, 88, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
		103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
		118, 119, 120, 121, 122};

#ifdef CHSPEC_IS80
	uint16 tone_idx_80[234] = {
		6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		24, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61,
		62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 90, 91, 92, 93, 94, 95, 96, 97, 98,
		99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
		114, 115, 116, 118, 119, 120, 121, 122, 123, 124, 125, 126, 130, 131, 132,
		133, 134, 135, 136, 137, 138, 140, 141, 142, 143, 144, 145, 146, 147, 148,
		149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
		164, 165, 166, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
		180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
		195, 196, 197, 198, 199, 200, 201, 202, 204, 205, 206, 207, 208, 209, 210,
		211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225,
		226, 227, 228, 229, 230, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
		242, 243, 244, 245, 246, 247, 248, 249, 250};
#endif /* CHSPEC_IS80 */

	if (!(ISHTPHY(pi) || ISACPHY(pi) || ISNPHY(pi)))
		return BCME_UNSUPPORTED;

	if (!pi->sh->up)
		return BCME_NOTUP;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* Go deaf to prevent PHY channel writes while doing reads */
	if (ISNPHY(pi))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);
	else if (ISHTPHY(pi))
		wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
	else if (ISACPHY(pi))
		wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		num_tones = 108;
		fft_size = 128;
		tone_idx_tbl = tone_idx_40;
#ifdef CHSPEC_IS80
	} else if (CHSPEC_IS80(pi->radio_chanspec)) {
		num_tones = 234;
		fft_size = 256;
		tone_idx_tbl = tone_idx_80;
#endif /* CHSPEC_IS80 */
	} else {
		num_tones = 52;
		fft_size = 64;
		tone_idx_tbl = tone_idx_20;
	}

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, r) {
		for (k = 0; k < num_tones; k++) {
			fftk = tone_idx_tbl[k];
			fftk = (fftk < fft_size/2) ? (fftk + fft_size/2) : (fftk - fft_size/2);

			if (ISNPHY(pi)) {
				tbl_id = NPHY_TBL_ID_CHANEST;
				wlc_phy_table_read_nphy(pi, tbl_id, 1, fftk, 32, &ch);
			} else if (ISHTPHY(pi)) {
				tbl_id = HTPHY_TBL_ID_CHANEST(r);
				wlc_phy_table_read_htphy(pi, tbl_id, 1, fftk, 32, &ch);
			} else if (ISACPHY(pi)) {
				tbl_id = ACPHY_TBL_ID_CHANEST(r);
				wlc_phy_table_read_acphy(pi, tbl_id, 1, fftk, 32, &ch);
			}
			if (ACMAJORREV_0(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev) ||
			ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
				ch_re_ma  = ((ch >> 18) & 0x7ff);
				ch_re_si  = ((ch >> 29) & 0x001);
				ch_im_ma  = ((ch >>  6) & 0x7ff);
				ch_im_si  = ((ch >> 17) & 0x001);
			} else {
				ch_re_ma  = ((ch >> 14) & 0xff);
				ch_re_si  = ((ch >> 22) & 0x01);
				ch_im_ma  = ((ch >>  5) & 0xff);
				ch_im_si  = ((ch >> 13) & 0x01);
			}
			ch_re = (ch_re_si > 0) ? -ch_re_ma : ch_re_ma;
			ch_im = (ch_im_si > 0) ? -ch_im_ma : ch_im_ma;

			bcm_bprintf(b, "%d\n%d\n", ch_re, ch_im);
		}
	}

	/* Return from deaf */
	if (ISNPHY(pi))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
	else if (ISHTPHY(pi))
		wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
	else if (ISACPHY(pi))
		wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}
#endif /* WLTEST */

#if defined(DBG_BCN_LOSS)
int
wlc_phydump_phycal_rx_min(phy_info_t *pi, struct bcmstrbuf *b)
{
	nphy_iq_comp_t rxcal_coeffs;
	int time_elapsed;
	phy_info_nphy_t *pi_nphy = NULL;

	if (!pi->sh->up) {
		PHY_ERROR(("wl%d: %s: Not up, cannot dump \n", pi->sh->unit, __FUNCTION__));
		return BCME_NOTUP;
	}

	if (ISACPHY(pi)) {
		PHY_ERROR(("wl%d: %s: AC Phy not yet supported\n", pi->sh->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
	else if (ISHTPHY(pi)) {
		wlc_phy_cal_dump_htphy_rx_min(pi, b);
	}
	else if (ISNPHY(pi)) {
		pi_nphy = pi->u.pi_nphy;
		if (!pi_nphy) {
			PHY_ERROR(("wl%d: %s: NPhy null, cannot dump \n",
				pi->sh->unit, __FUNCTION__));
			return BCME_UNSUPPORTED;
		}

		time_elapsed = pi->sh->now - pi->cal_info->last_cal_time;
		if (time_elapsed < 0)
			time_elapsed = 0;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		if (pi_nphy->phyhang_avoid)
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

		/* Read Rx calibration co-efficients */
		wlc_phy_rx_iq_coeffs_nphy(pi, 0, &rxcal_coeffs);

		if (pi_nphy->phyhang_avoid)
			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

		/* reg access is done, enable the mac */
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);

		bcm_bprintf(b, "time since last cal: %d (sec), mphase_cal_id: %d\n\n",
			time_elapsed, pi->cal_info->cal_phase_id);

		bcm_bprintf(b, "rx cal  a0=%d, b0=%d, a1=%d, b1=%d\n\n",
			rxcal_coeffs.a0, rxcal_coeffs.b0, rxcal_coeffs.a1, rxcal_coeffs.b1);
	}

	return BCME_OK;
}
#endif /* DBG_BCN_LOSS */

void
wlc_beacon_loss_war_lcnxn(wlc_phy_t *ppi, uint8 enable)
{
	 phy_info_t *pi = (phy_info_t*)ppi;
		phy_utils_mod_phyreg(pi, NPHY_reset_cca_frame_cond_ctrl_1,
		NPHY_reset_cca_frame_cond_ctrl_1_resetCCA_frame_cond_en_MASK,
		enable << NPHY_reset_cca_frame_cond_ctrl_1_resetCCA_frame_cond_en_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_EngCtrl1,
		NPHY_EngCtrl1_resetBphyEn_MASK,
		enable << NPHY_EngCtrl1_resetBphyEn_SHIFT);
}

const uint8 *
BCMRAMFN(wlc_phy_get_ofdm_rate_lookup)(void)
{
	return ofdm_rate_lookup;
}

/* LCNCONF */
void
wlc_lcnphy_epa_switch(phy_info_t *pi, bool mode)
{
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM)) {
		if (mode) {
			uint16 txant = 0;
			txant = wlapi_bmac_get_txant(pi->sh->physhim);
			if (txant == 1) {
				PHY_REG_MOD(pi, LCNPHY, RFOverrideVal0, ant_selp_ovr_val, 1);
				PHY_REG_MOD(pi, LCNPHY, RFOverride0, ant_selp_ovr, 1);
			}
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpiocontrol),
				~0x0, 0x0);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioout),
				0x40, 0x40);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioouten),
				0x40, 0x40);
		} else {
			PHY_REG_MOD(pi, LCNPHY, RFOverride0, ant_selp_ovr, 0);
			PHY_REG_MOD(pi, LCNPHY, RFOverrideVal0, ant_selp_ovr_val, 0);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioout),
				0x40, 0x00);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioouten),
				0x40, 0x0);
			si_corereg(pi->sh->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, gpiocontrol), ~0x0, 0x40);
		}
	}
}

bool
wlc_phy_get_tempsense_degree(wlc_phy_t *ppi, int8 *pval)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISLCNPHY(pi))
		*pval = wlc_lcnphy_tempsense_degree(pi, 0);
	else
		return FALSE;
	return TRUE;
}

void
wlc_phy_ldpc_override_set(wlc_phy_t *ppi, bool ldpc)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISHTPHY(pi))
		wlc_phy_update_rxldpc_htphy(pi, ldpc);
	if (ISACPHY(pi))
		wlc_phy_update_rxldpc_acphy(pi, ldpc);
	return;
}

void
wlc_phy_tkip_rifs_war(wlc_phy_t *ppi, uint8 rifs)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISNPHY(pi))
		wlc_phy_nphy_tkip_rifs_war(pi, rifs);
}

void
wlc_phy_get_pwrdet_offsets(phy_info_t *pi, int8 *cckoffset, int8 *ofdmoffset)
{
	*cckoffset = 0;
	*ofdmoffset = 0;
#ifdef WLNOKIA_NVMEM
	if (ISLCNPHY(pi))
		wlc_phy_noknvmem_get_pwrdet_offsets(pi, cckoffset, ofdmoffset);
#endif /* WLNOKIA_NVMEM */
}

/* update the cck power detector offset */
int8
wlc_phy_upd_rssi_offset(phy_info_t *pi, int8 rssi, chanspec_t chanspec)
{

#ifdef WLNOKIA_NVMEM
	if (rssi != WLC_RSSI_INVALID)
		rssi = wlc_phy_noknvmem_modify_rssi(pi, rssi, chanspec);
#endif // endif
	return rssi;
}

bool
wlc_phy_txpower_ipa_ison(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISNPHY(pi))
		return (wlc_phy_n_txpower_ipa_ison(pi));
	else
		return 0;
}

int
BCMRAMFN(wlc_phy_create_chanctx)(wlc_phy_t *ppi, chanspec_t chanspec)
{
#ifdef PHYCAL_CACHING
	ch_calcache_t *ctx;
	phy_info_t *pi = (phy_info_t *)ppi;

	/* Check for existing */
	if (wlc_phy_reuse_chanctx(ppi, chanspec) != 0)
		return BCME_OK;

#ifdef DONGLE_MAX_CAL_CACHE
	if (DONGLE_MAX_CAL_CACHE <= pi->phy_calcache_num) {
		/* Already max num ctx exist, reuse oldest */
		ctx = wlc_phy_get_chanctx_oldest(pi);
		ASSERT(ctx);
		wlc_phy_reinit_chanctx(pi, ctx,  pi->radio_chanspec);
		PHY_INFORM(("%s:max num ctx exist reuse oldest \n", __FUNCTION__));
		return BCME_OK;
	}
#endif // endif

	if (!(ctx = (ch_calcache_t *)MALLOC(pi->sh->osh, sizeof(ch_calcache_t)))) {
		PHY_ERROR(("%s: out of memory %d\n", __FUNCTION__, MALLOCED(pi->sh->osh)));
		return BCME_NOMEM;
	}
	bzero(ctx, sizeof(ch_calcache_t));

	ctx->chanspec = chanspec;
	ctx->creation_time = pi->sh->now;
	ctx->cal_info.last_cal_temp = -50;
	ctx->cal_info.txcal_numcmds = pi->def_cal_info->txcal_numcmds;
	ctx->in_use = TRUE;
#ifdef WL_AIR_IQ
	if (phy_get_phymode(pi) == PHYMODE_3x3_1x1) {
		ctx->chanspec_sc = pi->radio_chanspec_sc;
	} else {
		ctx->chanspec_sc = 0;
	}
#endif /* WL_AIR_IQ */

	/* Add it to the list */
	ctx->next = pi->phy_calcache;

	/* For the first context, switch out the default context */
	if (pi->phy_calcache == NULL &&
	    (pi->radio_chanspec == chanspec))
		pi->cal_info = &ctx->cal_info;
	pi->phy_calcache = ctx;
	pi->phy_calcache_num++;

	PHY_INFORM(("wl%d: %s ctx %d created for Ch %d\n",
		PI_INSTANCE(pi), __FUNCTION__,
		pi->phy_calcache_num,
		CHSPEC_CHANNEL(chanspec)));
#endif /* PHYCAL_CACHING */
	return BCME_OK;
}

void
BCMRAMFN(wlc_phy_destroy_chanctx)(wlc_phy_t *ppi, chanspec_t chanspec)
{
#ifdef PHYCAL_CACHING
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx;

	ctx = wlc_phy_get_chanctx(pi, chanspec);
	if (ctx) {
		ctx->valid = FALSE;
		ctx->in_use = FALSE;
	}

	PHY_INFORM(("wl%d: %s for Ch %d\n",
		PI_INSTANCE(pi), __FUNCTION__,
		CHSPEC_CHANNEL(chanspec)));
#endif /* PHYCAL_CACHING */
}

#if defined(PHYCAL_CACHING)
int
wlc_phy_reinit_chanctx(phy_info_t *pi, ch_calcache_t *ctx, chanspec_t chanspec)
{
	ASSERT(ctx);
	ctx->valid = FALSE;
	ctx->chanspec = chanspec;
	ctx->creation_time = pi->sh->now;
	ctx->cal_info.last_cal_time = 0;
	ctx->cal_info.last_papd_cal_time = 0;
	ctx->cal_info.last_cal_temp = -50;
	ctx->in_use = TRUE;
#ifdef WL_AIR_IQ
	if (phy_get_phymode(pi) == PHYMODE_3x3_1x1) {
		ctx->chanspec_sc = pi->radio_chanspec_sc;
	} else {
		ctx->chanspec_sc = 0;
	}
#endif /* WL_AIR_IQ */
	return 0;
}

int
wlc_phy_invalidate_chanctx(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx = pi->phy_calcache;

	while (ctx) {
		if (ctx->chanspec == chanspec) {
			ctx->valid = FALSE;
			ctx->cal_info.last_cal_time = 0;
			ctx->cal_info.last_papd_cal_time = 0;
			ctx->cal_info.last_cal_temp = -50;
			return 0;
		}
		ctx = ctx->next;
	}

	return 0;
}
/*   This function will try and reuse the existing ctx:
	return 0 --> couldn't find any ctx
	return 1 --> channel ctx exist
	return 2 --> grabbed an invalid ctx
*/
int
wlc_phy_reuse_chanctx(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx = pi->phy_calcache;

	/* Check for existing */
	if (wlc_phy_get_chanctx(pi, chanspec)) {
		PHY_INFORM(("wl%d: %s | using existing chanctx for Ch %d\n",
			PI_INSTANCE(pi), __FUNCTION__,
			CHSPEC_CHANNEL(chanspec)));
		return 1;
	}

	/* Check if there are any invalid entries and use them */
	while (ctx) {
		if (!ctx->in_use)
		{
			wlc_phy_reinit_chanctx(pi, ctx, chanspec);
			PHY_INFORM(("wl%d: %s | grabbed an unused chanctx\n",
				PI_INSTANCE(pi), __FUNCTION__));
			return 2;
		}
		ctx = ctx->next;
	}

	PHY_INFORM(("wl%d: %s | couldn't find any ctx for Ch %d\n",
		PI_INSTANCE(pi), __FUNCTION__,
		CHSPEC_CHANNEL(chanspec)));

	return BCME_OK;
}

void
wlc_phy_update_chctx_glacial_time(wlc_phy_t *ppi, chanspec_t chanspec)
{
	ch_calcache_t *ctx;
	phy_info_t *pi = (phy_info_t *)ppi;
	if ((ctx = wlc_phy_get_chanctx((phy_info_t *)ppi, chanspec)))
		ctx->cal_info.last_cal_time = pi->sh->now - pi->sh->glacial_timer;
}

ch_calcache_t *
wlc_phy_get_chanctx_oldest(phy_info_t *phi)
{
	ch_calcache_t *ctx = phi->phy_calcache;
	ch_calcache_t *ctx_lo_ctime = ctx;
	while (ctx->next) {
		ctx = ctx->next;
		if (ctx_lo_ctime->creation_time > ctx->creation_time)
			ctx_lo_ctime = ctx;
	}
	return ctx_lo_ctime;
}

ch_calcache_t *
wlc_phy_get_chanctx(phy_info_t *phi, chanspec_t chanspec)
{
	ch_calcache_t *ctx = phi->phy_calcache;
	while (ctx) {
#ifdef WL_AIR_IQ
		/* matching rules for 3x3+1 phy mode:
		 * - 3x3 band == +1 band, equivalent to 4x4 phymode.
		 * - 3x3 band != +1 band, cache
		 * 4x4 mode: chanspec_sc must be 0.
		 */
		PHY_TRACE(("%s: mode:%d ctx chanspec 0x%x (%d) radio 0x%x %d scan 0x%x %d\n",
					__FUNCTION__, phy_get_phymode(phi),
					ctx->chanspec, CHSPEC_CHANNEL(ctx->chanspec),
					phi->radio_chanspec, CHSPEC_CHANNEL(phi->radio_chanspec),
					phi->radio_chanspec_sc, CHSPEC_CHANNEL(phi->radio_chanspec_sc)));
		if (phy_get_phymode(phi) == PHYMODE_3x3_1x1) {
			if (ctx->chanspec == phi->radio_chanspec) {
				if (ctx->chanspec_sc == 0) {
					if (CHSPEC_BAND(phi->radio_chanspec) ==
						CHSPEC_BAND(phi->radio_chanspec_sc)) {
						/* Re-use 4x4 context if chanspec_sc and
						 * chanspec bands are equal.
						 */
						return ctx;
					}
				} else { /* +1 chanspec, not 4x4 */
					if (CHSPEC_BAND(ctx->chanspec_sc) == CHSPEC_BAND(phi->radio_chanspec_sc)) {
						return ctx;
					}
				}
			}
		} else if (ctx->chanspec == chanspec && ctx->chanspec_sc == 0) {
			/* Handles MIMO PHY mode (0) */
			return ctx;
		}
#else
		if (ctx->chanspec == chanspec)
			return ctx;
#endif /* WL_AIR_IQ */
		ctx = ctx->next;
	}
	return NULL;
}

bool
wlc_phy_chan_iscached(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx;
	bool ret = 0;

	ctx = wlc_phy_get_chanctx(pi, chanspec);

	if (ctx != NULL)
		if (ctx->valid)
			ret = 1;

	return ret;
}

void wlc_phy_get_all_cached_ctx(wlc_phy_t *ppi, chanspec_t *chanlist)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx = pi->phy_calcache;
	int i = 0;

	while (ctx) {
		*(chanlist+i) = ctx->chanspec;
		i++;
		ctx = ctx->next;
	}
}

void
wlc_phy_get_cachedchans(wlc_phy_t *ppi, chanspec_t *chanlist)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx = pi->phy_calcache;
	chanlist[0] = 0;

	while (ctx) {
		if (ctx->valid) {
			chanlist[0] += 1;
			chanlist[chanlist[0]] = ctx->chanspec;
		}
		ctx = ctx->next;
	}
}

int8
wlc_phy_get_max_cachedchans(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISLCN40PHY(pi))
		return (wlc_lcn40phy_max_cachedchans(pi));

	return (-1);
}

uint32
wlc_phy_get_current_cachedchans(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi) {
#if defined(PHYCAL_CACHING)|| defined(WL_MODESW)
		return pi->phy_calcache_num;
#endif // endif
	}
	return 0;
}

#if defined(PHYCAL_CACHING) && defined(BCMDBG)
static void
wlc_phydump_chanctx(phy_info_t *phi, struct bcmstrbuf *b)
{
	ch_calcache_t *ctx = phi->phy_calcache;

	if (phi->HW_FCBS) {
		return;
	}

	bcm_bprintf(b, "Current chanspec: 0x%x\n", phi->radio_chanspec);
	while (ctx) {
			bcm_bprintf(b, "%sContext found for chanspec: 0x%x\n",
			            (ctx->valid)? "Valid ":"",
			            ctx->chanspec);
		if (ISNPHY(phi)) {
			wlc_phydump_cal_cache_nphy(phi, ctx, b);
		} else if (ISHTPHY(phi)) {
			wlc_phydump_cal_cache_htphy(phi, ctx, b);
		} else if (ISACPHY(phi)) {
			wlc_phydump_cal_cache_acphy(phi, ctx, b);
		}

		ctx = ctx->next;
	}
}
#endif /* PHYCAL_CACHING && BCMDBG  */

void
wlc_phy_cal_cache(wlc_phy_t *ppi)
{
	if (ISACPHY((phy_info_t*)ppi)) {
		wlc_phy_cal_cache_acphy(ppi);
	}
}

int
wlc_phy_cal_cache_restore(phy_info_t *pi)
{
	if (ISNPHY(pi)) {
		return (wlc_phy_cal_cache_restore_nphy(pi));
	} else if (ISHTPHY(pi)) {
		return (wlc_phy_cal_cache_restore_htphy(pi));
	} else if (ISACPHY(pi)) {
		return (wlc_phy_cal_cache_restore_acphy(pi));
	} else {
		return 0;
	}
}

int
wlc_phy_cal_cache_return(wlc_phy_t *ppi)
{
	return wlc_phy_cal_cache_restore((phy_info_t *)ppi);
}
#endif /* PHYCAL_CACHING */

int
wlc_phy_txpower_core_offset_set(wlc_phy_t *ppi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	int err = BCME_UNSUPPORTED;

	if (pi->pi_fptr.txcorepwroffsetset)
		err = (*pi->pi_fptr.txcorepwroffsetset)(pi, offsets);

	return err;
}

int
wlc_phy_txpower_core_offset_get(wlc_phy_t *ppi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	int err = BCME_UNSUPPORTED;

	if (pi->pi_fptr.txcorepwroffsetget)
		err = (*pi->pi_fptr.txcorepwroffsetget)(pi, offsets);

	return err;
}

#ifndef PPR_API
static void
BCMNMIATTACHFN(wlc_phy_txpwr_srom9_convert)(phy_info_t *pi, uint8 *srom_max, uint32 pwr_offset,
	uint8 tmp_max_pwr, uint8 rate_start, uint8 rate_end, bool shift)
{
	uint8 rate;
	uint8 nibble;

	if (pi->sh->sromrev < 9) {
		ASSERT(0 && "SROMREV < 9");
		return;
	}

	for (rate = rate_start; rate <= rate_end; rate++) {
		nibble = (uint8)(pwr_offset & 0xf);
		if (shift)
			pwr_offset >>= 4;
		/* nibble info indicates offset in 0.5dB units convert to 0.25dB */
		srom_max[rate] = tmp_max_pwr - (nibble << 1);
	}
}
void
BCMNMIATTACHFN(wlc_phy_txpwr_apply_srom9)(phy_info_t *pi)
{

	srom_pwrdet_t	*pwrdet  = pi->pwrdet;
	uint8 tmp_max_pwr = 0;
	uint8 *tx_srom_max_rate = NULL;
	uint32 ppr_offsets[PWR_OFFSET_SIZE];
	uint32 pwr_offsets;
	uint rate_cnt, rate;
	int band_num;
	bool shift;

	for (band_num = 0; band_num < NUMSUBBANDS(pi); band_num++) {
		bzero((uint8 *)ppr_offsets, PWR_OFFSET_SIZE * sizeof(uint32));
		if (ISNPHY(pi)) {
			/* find MIN of 2  cores, board limits */
			tmp_max_pwr = MIN(pwrdet->max_pwr[0][band_num],
				pwrdet->max_pwr[1][band_num]);
		}
		if (ISPHY_HT_CAP(pi)) {
			tmp_max_pwr =
				MIN(pwrdet->max_pwr[0][band_num], pwrdet->max_pwr[1][band_num]);
			tmp_max_pwr =
				MIN(tmp_max_pwr, pwrdet->max_pwr[2][band_num]);
		}

		ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm[band_num].bw20;
		ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm[band_num].bw20ul;
		ppr_offsets[OFDM_40DUP_PO] = pi->ppr.sr9.ofdm[band_num].bw40;
		ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs[band_num].bw20;
		ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs[band_num].bw20ul;
		ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs[band_num].bw40;
		tx_srom_max_rate = (uint8*)(&(pi->tx_srom_max_rate[band_num][0]));

		switch (band_num) {
			case WL_CHAN_FREQ_RANGE_2G:
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po & 0xf);
				wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
					pi->ppr.sr9.cckbw202gpo, tmp_max_pwr,
					TXP_FIRST_CCK, TXP_LAST_CCK, TRUE);
				wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
					pi->ppr.sr9.cckbw202gpo, tmp_max_pwr,
					TXP_FIRST_CCK_CDD_S1x2, TXP_LAST_CCK_CDD_S1x2, TRUE);
				wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
					pi->ppr.sr9.cckbw202gpo, tmp_max_pwr,
					TXP_FIRST_CCK_CDD_S1x3, TXP_LAST_CCK_CDD_S1x3, TRUE);
				wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
					pi->ppr.sr9.cckbw20ul2gpo, tmp_max_pwr,
					TXP_FIRST_20UL_CCK, TXP_LAST_20UL_CCK, TRUE);
				wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
					pi->ppr.sr9.cckbw20ul2gpo, tmp_max_pwr,
					TXP_FIRST_CCK_20U_CDD_S1x2,
					TXP_LAST_CCK_20U_CDD_S1x2, TRUE);
				wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
					pi->ppr.sr9.cckbw20ul2gpo, tmp_max_pwr,
					TXP_FIRST_CCK_20U_CDD_S1x3,
					TXP_LAST_CCK_20U_CDD_S1x3, TRUE);
				break;
#ifdef BAND5G
			case WL_CHAN_FREQ_RANGE_5G_BAND0:
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 4) & 0xf;
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND1:
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 8) & 0xf;
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND2:
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 12) & 0xf;
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND3:
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 12) & 0xf;
				break;
#endif /* BAND5G */
			default:
				break;
		}

		for (rate = TXP_FIRST_CCK; rate < TXP_NUM_RATES; rate += rate_cnt) {
			shift = TRUE;
			pwr_offsets = 0;
			switch (rate) {
				case TXP_FIRST_CCK:
				case TXP_FIRST_20UL_CCK:
				case TXP_FIRST_CCK_CDD_S1x2:
				case TXP_FIRST_CCK_CDD_S1x3:
				case TXP_FIRST_CCK_20U_CDD_S1x2:
				case TXP_FIRST_CCK_20U_CDD_S1x3:
					rate_cnt = WL_NUM_RATES_CCK;
					continue;
				case TXP_FIRST_OFDM:
				case TXP_FIRST_OFDM_20_CDD:
					pwr_offsets = ppr_offsets[OFDM_20_PO];
					rate_cnt = WL_NUM_RATES_OFDM;
					break;
				case TXP_FIRST_MCS_20_S1x1:
				case TXP_FIRST_MCS_20_S1x2:
				case TXP_FIRST_MCS_20_S1x3:
				case TXP_FIRST_MCS_20_S2x2:
				case TXP_FIRST_MCS_20_S2x3:
				case TXP_FIRST_MCS_20_S3x3:
					pwr_offsets = ppr_offsets[MCS_20_PO];
					rate_cnt = WL_NUM_RATES_MCS_1STREAM;
					break;
				case TXP_FIRST_OFDM_40_SISO:
				case TXP_FIRST_OFDM_40_CDD:
					pwr_offsets = ppr_offsets[OFDM_40DUP_PO];
					rate_cnt = WL_NUM_RATES_OFDM;
					break;
				case TXP_FIRST_MCS_40_S1x1:
				case TXP_FIRST_MCS_40_S1x2:
				case TXP_FIRST_MCS_40_S1x3:
				case TXP_FIRST_MCS_40_S2x2:
				case TXP_FIRST_MCS_40_S2x3:
				case TXP_FIRST_MCS_40_S3x3:
					pwr_offsets = ppr_offsets[MCS_40_PO];
					rate_cnt = WL_NUM_RATES_MCS_1STREAM;
					break;
				case TXP_MCS_32:
					pwr_offsets = ppr_offsets[MCS32_PO];
					rate_cnt = WL_NUM_RATES_MCS32;
					break;
				case TXP_FIRST_20UL_OFDM:
				case TXP_FIRST_20UL_OFDM_CDD:
					pwr_offsets = ppr_offsets[OFDM_20UL_PO];
					rate_cnt = WL_NUM_RATES_OFDM;
					break;
				case TXP_FIRST_20UL_S1x1:
				case TXP_FIRST_20UL_S1x2:
				case TXP_FIRST_20UL_S1x3:
				case TXP_FIRST_20UL_S2x2:
				case TXP_FIRST_20UL_S2x3:
				case TXP_FIRST_20UL_S3x3:
					pwr_offsets = ppr_offsets[MCS_20UL_PO];
					rate_cnt = WL_NUM_RATES_MCS_1STREAM;
					break;
				default:
					PHY_ERROR(("Invalid rate %d\n", rate));
					rate_cnt = 1;
					ASSERT(0);
					break;
			}
			wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate, pwr_offsets,
				tmp_max_pwr, (uint8)rate, (uint8)rate+rate_cnt-1, shift);
		}
	}
}

#endif /* !PPR_API */

#ifdef PPR_API
/* add 1MSB to represent 5bit-width ppr value, for mcs8 and mcs9 only */
void
wlc_phy_txpwr_ppr_bit_ext_mcs8and9(ppr_vht_mcs_rateset_t* vht, uint8 msb)
{
	/* this added 1MSB is the 4th bit, so left shift 4 bits
	 * then left shift 1 more bit since boardlimit is 0.5dB format
	 */
	vht->pwr[8] -= ((msb & 0x1) << 4) << 1;
	vht->pwr[9] -= ((msb & 0x2) << 3) << 1;
}

/* add 1MSB to represent 5bit-width ppr value, for mcs10 and mcs11 only */
void
wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(ppr_vht_mcs_rateset_t* vht, uint8 msb)
{
	/* this added 1MSB is the 4th bit, so left shift 4 bits
	 * then left shift 1 more bit since boardlimit is 0.5dB format
	 * bit 0-3 for mcs8-11
	 */
	vht->pwr[8]  -= ((msb & 0x1) << 4) << 1;
	vht->pwr[9]  -= ((msb & 0x2) << 3) << 1;
	if (sizeof(*vht) > 10) {
		vht->pwr[10] -= ((msb & 0x4) << 2) << 1;
		vht->pwr[11] -= ((msb & 0x8) << 1) << 1;
	}
}

/* for CCK case, 4 rates only */
void
wlc_phy_txpwr_srom_convert_cck(uint16 po, uint8 max_pwr, ppr_dsss_rateset_t *dsss)
{
	uint8 i;
	/* Extract offsets for 4 CCK rates, convert from .5 to .25 dbm units. */
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++) {
		dsss->pwr[i] = max_pwr - ((po & 0xf) * 2);
		po >>= 4;
	}
}

/* for OFDM cases, 8 rates only */
void
wlc_phy_txpwr_srom_convert_ofdm(uint32 po, uint8 max_pwr, ppr_ofdm_rateset_t *ofdm)
{
	uint8 i;
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		ofdm->pwr[i] = max_pwr - ((po & 0xf) * 2);
		po >>= 4;
	}
}

/* for MCS20&40_2G case, 10 rates only */
static void
wlc_phy_txpwr_srom11_convert_mcs_2g(uint32 po, uint8 nibble,
         uint8 tmp_max_pwr, ppr_vht_mcs_rateset_t* vht) {
	uint8 i;
	int8 offset;
	offset = (nibble + 8)%16 - 8;

	for (i = 0; i < WL_RATESET_SZ_VHT_MCS; i++) {
		if ((i == 1)||(i == 2)) {
			vht->pwr[i] = vht->pwr[0];
		} else {
			vht->pwr[i] = tmp_max_pwr - ((po & 0xf)<<1);
			po = po >> 4;
		}
	}
	vht->pwr[1] -= (offset << 1);
	vht->pwr[2] = vht->pwr[1];
}

#ifdef WL11AC
/* for MCS10/11 cases, 2 rates only */
static void
wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_2g(uint16 po, chanspec_t chanspec,
         uint8 tmp_max_pwr, ppr_vht_mcs_rateset_t* vht) {

	if (!(sizeof(*vht) > 10)) {
		PHY_ERROR(("%s: should not call me this file without VHT MCS10/11 supported!\n",
			__FUNCTION__));
		return;
	}

	if (CHSPEC_IS20(chanspec)) {
		vht->pwr[10] = tmp_max_pwr - ((po & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 4) & 0xf) << 1);
	} else if (CHSPEC_IS40(chanspec)) {
		vht->pwr[10] = tmp_max_pwr - (((po >> 8) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 12) & 0xf) << 1);
	}
}
#endif /* WL11AC */

/* for 2G Legacy 40Dup mode, providing the base pwr */
static void
wlc_phy_txpwr_srom11_convert_ofdm_2g_dup40(uint32 po, uint8 nibble,
         uint8 tmp_max_pwr, ppr_ofdm_rateset_t* ofdm) {
	uint8 i;
	int8 offset;
	offset = (nibble + 8)%16 - 8;
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		if ((i == 1)||(i == 2)||(i == 3)) {
			ofdm->pwr[i] = ofdm->pwr[0];
		} else {
			ofdm->pwr[i] = tmp_max_pwr - ((po & 0xf) <<1);
			po = po >> 4;
		}
	}
	ofdm->pwr[2] -= (offset << 1);
	ofdm->pwr[3] = ofdm->pwr[2];
}

/* for ofdm20in40_2G case, 8 rates only */
static void
wlc_phy_txpwr_srom11_convert_ofdm_offset(ppr_ofdm_rateset_t* po,
                                         uint8 nibble2, ppr_ofdm_rateset_t* ofdm)
{
	uint8 i;
	int8 offsetL, offsetH;
	offsetL = ((nibble2 & 0xf) + 8)%16 - 8;
	offsetH = (((nibble2>>4) & 0xf) + 8)%16 - 8;
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		if (i < 6)
			ofdm->pwr[i] = po->pwr[i] + (offsetL << 1);
		else
			ofdm->pwr[i] = po->pwr[i] + (offsetH << 1);
	}
}

/* for mcs20in40_2G case, 10 rates only */
static void
wlc_phy_txpwr_srom11_convert_mcs_offset(ppr_vht_mcs_rateset_t* po,
                                        uint8 nibble2, ppr_vht_mcs_rateset_t* vht)
{
	uint8 i;
	int8 offsetL, offsetH;
	offsetL = ((nibble2 & 0xf) + 8)%16 - 8;
	offsetH = (((nibble2>>4) & 0xf) + 8)%16 - 8;
	for (i = 0; i < sizeof(*vht); i++) {
		if (i < 5)
			vht->pwr[i] = po->pwr[i] + (offsetL << 1);
		else
			vht->pwr[i] = po->pwr[i] + (offsetH << 1);
	}
}

#ifdef BAND5G
/* for ofdm20_5G case, 8 rates only */
static void
wlc_phy_txpwr_srom11_convert_ofdm_5g(uint32 po, uint8 nibble,
         uint8 tmp_max_pwr, ppr_ofdm_rateset_t* ofdm) {
	uint8 i;
	int8 offset;
	offset = (nibble + 8)%16 - 8;
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		if ((i == 1)||(i == 2)||(i == 3)) {
			ofdm->pwr[i] = ofdm->pwr[0];
		} else {
			ofdm->pwr[i] = tmp_max_pwr - ((po & 0xf) <<1);
			po = po >> 4;
		}
	}
	ofdm->pwr[2] -= (offset << 1);
	ofdm->pwr[3] = ofdm->pwr[2];
}

/* for MCS20&40_5G case, 10 rates only */
static void
wlc_phy_txpwr_srom11_convert_mcs_5g(uint32 po, uint8 nibble,
         uint8 tmp_max_pwr, ppr_vht_mcs_rateset_t* vht) {
	uint8 i;
	int8 offset;
	offset = (nibble + 8)%16 - 8;
	for (i = 0; i < sizeof(*vht); i++) {
		if ((i == 1)||(i == 2)) {
			vht->pwr[i] = vht->pwr[0];
		} else {
			vht->pwr[i] = tmp_max_pwr - ((po & 0xf)<<1);
			po = po >> 4;
		}
	}
	vht->pwr[1] -= (offset << 1);
	vht->pwr[2] = vht->pwr[1];
}

#endif /* BAND5G */

#ifdef WL11AC
/* for MCS10/11 cases, 2 rates only */
static void
wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_5g(uint32 po, uint8 pkt_bw,
         uint8 tmp_max_pwr, ppr_vht_mcs_rateset_t* vht) {

	if (!(sizeof(*vht) > 10)) {
		PHY_ERROR(("%s: should not call me this file without VHT MCS10/11 supported!\n",
				__FUNCTION__));
		return;
	}

	if (pkt_bw == WL_PKT_BW_20) {
		vht->pwr[10] = tmp_max_pwr - ((po & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 4) & 0xf) << 1);
	} else if (pkt_bw == WL_PKT_BW_40) {
		vht->pwr[10] = tmp_max_pwr - (((po >> 8) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 12) & 0xf) << 1);
	} else if (pkt_bw == WL_PKT_BW_80) {
		vht->pwr[10] = tmp_max_pwr - (((po >> 16) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 20) & 0xf) << 1);
	} else if (pkt_bw == WL_PKT_BW_160) {
		vht->pwr[10] = tmp_max_pwr - (((po >> 24) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 28) & 0xf) << 1);
	} else {
		PHY_ERROR(("%s: Invalid pkt_bw = %d!\n", __FUNCTION__, pkt_bw));
	}
}
#endif /* WL11AC */

static void
wlc_phy_ppr_set_dsss(ppr_t* tx_srom_max_pwr, uint8 bwtype,
          ppr_dsss_rateset_t* pwr_offsets, phy_info_t *pi) {
	uint8 chain;
	for (chain = WL_TX_CHAINS_1; chain <= PHYCORENUM(pi->pubpi.phy_corenum); chain++)
		/* for 2g_dsss: S1x1, S1x2, S1x3, s1x4 */
		ppr_set_dsss(tx_srom_max_pwr, bwtype, chain,
		      (const ppr_dsss_rateset_t*)pwr_offsets);
}

static void
wlc_phy_ppr_set_ofdm(ppr_t* tx_srom_max_pwr, uint8 bwtype,
          ppr_ofdm_rateset_t* pwr_offsets, phy_info_t *pi) {

	uint8 chain;
	ppr_set_ofdm(tx_srom_max_pwr, bwtype, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
	       (const ppr_ofdm_rateset_t*)pwr_offsets);
	BCM_REFERENCE(chain);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		for (chain = WL_TX_CHAINS_2; chain <= PHYCORENUM(pi->pubpi.phy_corenum); chain++) {
			ppr_set_ofdm(tx_srom_max_pwr, bwtype, WL_TX_MODE_CDD, chain,
				(const ppr_ofdm_rateset_t*)pwr_offsets);
#ifdef WL_BEAMFORMING
			/* Add TXBF */
			ppr_set_ofdm(tx_srom_max_pwr, bwtype, WL_TX_MODE_TXBF, chain,
				(const ppr_ofdm_rateset_t*)pwr_offsets);
#endif // endif
		}
	}
}

static void
wlc_phy_ppr_set_ht_mcs(ppr_t* tx_srom_max_pwr, uint8 bwtype,
         ppr_ht_mcs_rateset_t* pwr_offsets, phy_info_t *pi) {
	ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_NONE,
		WL_TX_CHAINS_1, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		/* for ht_S1x2_CDD */
		ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_CDD,
			WL_TX_CHAINS_2, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		/* for ht_S2x2_STBC */
		ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_STBC,
			WL_TX_CHAINS_2, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		/* for ht_S2x2_SDM */
		ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_NONE,
			WL_TX_CHAINS_2, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
			/* for ht_S1x3_CDD */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
			/* for ht_S2x3_STBC */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
			/* for ht_S2x3_SDM */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
			/* for ht_S3x3_SDM */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_3, WL_TX_MODE_NONE,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		}
	}
}

static void
wlc_phy_ppr_set_mcs(ppr_t* tx_srom_max_pwr, uint8 bwtype,
          ppr_vht_mcs_rateset_t* pwr_offsets, phy_info_t *pi) {
		int8 tmp_mcs8, tmp_mcs9;

	uint8 phy_core_num = PHYCORENUM(pi->pubpi.phy_corenum);
	if (ACMAJORREV_33(pi->pubpi.phy_rev) &&
		PHY_AS_80P80(pi, pi->radio_chanspec)) {
		phy_core_num = phy_core_num >> 1;
	}

	ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_NONE,
		WL_TX_CHAINS_1, (const ppr_vht_mcs_rateset_t*)pwr_offsets);

	if (phy_core_num > 1) {
		/* for vht_S1x2_CDD */
		ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_CDD,
			WL_TX_CHAINS_2, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
		/* for vht_S2x2_STBC */
		ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_STBC,
			WL_TX_CHAINS_2, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
		/* for vht_S2x2_SDM */
		ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_NONE,
			WL_TX_CHAINS_2, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
		/* for vht_S1x2_TXBF */
		ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_TXBF,
			WL_TX_CHAINS_2, (const ppr_vht_mcs_rateset_t*)pwr_offsets);

		tmp_mcs8 = pwr_offsets->pwr[8];
		tmp_mcs9 = pwr_offsets->pwr[9];
		pwr_offsets->pwr[8] = WL_RATE_DISABLED;
		pwr_offsets->pwr[9] = WL_RATE_DISABLED;
		/* for vht_S2x2_TXBF */
		/* VHT8SS2_TXBF0 and VHT9SS2_TXBF0 are invalid */
		ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_TXBF,
			WL_TX_CHAINS_2, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
		pwr_offsets->pwr[8] = tmp_mcs8;
		pwr_offsets->pwr[9] = tmp_mcs9;
		if (phy_core_num > 2) {
			/* for vht_S1x3_CDD */
			ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_3, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
			/* for vht_S2x3_STBC */
			ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_3, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
			/* for vht_S2x3_SDM */
			ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_3, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
			/* for vht_S3x3_SDM */
			ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_3, WL_TX_MODE_NONE,
				WL_TX_CHAINS_3, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
#if defined(WL_BEAMFORMING) && !defined(WLTXBF_DISABLED)
			/* for vht_S1x3_TXBF */
			ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_TXBF,
				WL_TX_CHAINS_3, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
			/* for vht_S2x3_TXBF */
			ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_TXBF,
				WL_TX_CHAINS_3, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
			tmp_mcs8 = pwr_offsets->pwr[8];
			tmp_mcs9 = pwr_offsets->pwr[9];
			pwr_offsets->pwr[8] = WL_RATE_DISABLED;
			pwr_offsets->pwr[9] = WL_RATE_DISABLED;
			/* for vht_S3x3_TXBF */
			/* VHT8SS3_TXBF0 and VHT9SS3_TXBF0 are invalid */
			ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_3, WL_TX_MODE_TXBF,
				WL_TX_CHAINS_3, (const ppr_vht_mcs_rateset_t*)pwr_offsets);
			pwr_offsets->pwr[8] = tmp_mcs8;
			pwr_offsets->pwr[9] = tmp_mcs9;
#endif /* WL_BEAMFORMING && !WLTXBF_DISABLED */

			if (phy_core_num > 3) {
				/* for vht_S1x4_CDD */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
				/* for vht_S2x4_STBC */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
				/* for vht_S2x4_SDM */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
				/* for vht_S3x4_SDM */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
				/* for vht_S4x4_SDM */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_4,
					WL_TX_MODE_NONE, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
#if defined(WL_BEAMFORMING) && !defined(WLTXBF_DISABLED)
				/* for vht_S1x4_TXBF */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1,
					WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
				/* for vht_S2x4_TXBF */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2,
					WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
				/* for vht_S3x4_TXBF */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_3,
					WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
				/* for vht_S4x4_TXBF */
				ppr_set_vht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_4,
					WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
					(const ppr_vht_mcs_rateset_t*)pwr_offsets);
#endif /* WL_BEAMFORMING && !WLTXBF_DISABLED */
			}
		}
	}
	BCM_REFERENCE(tmp_mcs8);
	BCM_REFERENCE(tmp_mcs9);
}

void
ppr_dsss_printf(ppr_t *p)
{
	int chain, bitN;
	ppr_dsss_rateset_t dsss_boardlimits;

	for (chain = WL_TX_CHAINS_1; chain <= WL_TX_CHAINS_3; chain++) {
		ppr_get_dsss(p, WL_TX_BW_20, chain, &dsss_boardlimits);
		PHY_ERROR(("--------DSSS-BW_20-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_DSSS; bitN++)
			PHY_ERROR(("max-pwr = %d\n", dsss_boardlimits.pwr[bitN]));

		ppr_get_dsss(p, WL_TX_BW_20IN40, chain, &dsss_boardlimits);
		PHY_ERROR(("--------DSSS-BW_20IN40-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_DSSS; bitN++)
			PHY_ERROR(("max-pwr = %d\n", dsss_boardlimits.pwr[bitN]));

	}
}

void
ppr_ofdm_printf(ppr_t *p)
{

	int chain, bitN;
	ppr_ofdm_rateset_t ofdm_boardlimits;
	wl_tx_mode_t mode = WL_TX_MODE_NONE;

	for (chain = WL_TX_CHAINS_1; chain <= WL_TX_CHAINS_3; chain++) {
		ppr_get_ofdm(p, WL_TX_BW_20, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_20-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));

		ppr_get_ofdm(p, WL_TX_BW_20IN40, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_20IN40-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));

		ppr_get_ofdm(p, WL_TX_BW_20IN80, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_20IN80-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));

		ppr_get_ofdm(p, WL_TX_BW_40, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_DUP40-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));
		mode = WL_TX_MODE_CDD;
	}
}

void
ppr_mcs_printf(ppr_t* tx_srom_max_pwr)
{

	int bitN, bwtype;
	ppr_vht_mcs_rateset_t mcs_boardlimits;
#if defined(BCMDBG)
	char* bw[6] = { "20IN20", "40IN40", "80IN80", "20IN40", "20IN80", "40IN80" };
#endif // endif
	for (bwtype = 0; bwtype < 6; bwtype++) {
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S1x1-----\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S1x2_CDD */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S1x2-CDD-----\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S1x3_CDD */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 1, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S1x3-CDD-----\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x2_STBC */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_STBC, WL_TX_CHAINS_2,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x2-STBC------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x3_STBC */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_STBC, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x3-STBC------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x2_SDM */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x2-SDM------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x3_SDM */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x3-SDM------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S3x3_SDM */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 3, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S3x3-SDM------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
	}
}

static uint8
wlc_phy_make_byte(uint16 nibbleH, uint16 nibbleL)
{
	return (uint8) (((nibbleH & 0xf) << 4) | (nibbleL & 0xf));
}

void
wlc_phy_txpwr_apply_srom11(phy_info_t *pi, uint8 band, chanspec_t chanspec,
                           uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr)
{
	uint8 nibbles;
	uint8 ppr_bit_ext, msb;
	const struct srom11_ppr *sr11 = &pi->ppr.sr11;

	if (!ISACPHY(pi))
		return;

	ppr_bit_ext = BF3_PPR_BIT_EXT(pi->u.pi_acphy);

	if (CHSPEC_IS2G(chanspec))
	{
		ppr_ofdm_rateset_t	ofdm20_offset_2g;
		ppr_vht_mcs_rateset_t	mcs20_offset_2g;

		/* 2G - OFDM_20 */
		wlc_phy_txpwr_srom_convert_ofdm(sr11->ofdm_2g.bw20, tmp_max_pwr, &ofdm20_offset_2g);

		/* 2G - MCS_20 */
		nibbles = (sr11->offset_2g >> 8) & 0xf;   /* 2LSB is needed */
		wlc_phy_txpwr_srom11_convert_mcs_2g(sr11->mcs_2g.bw20, nibbles,
		        tmp_max_pwr, &mcs20_offset_2g);

		if (ppr_bit_ext) {
			/* msb: bit 1 for mcs9, bit 0 for mcs8
			 * sb40and80hr5glpo, nib3 is 2G
			 * bit13 and bit12 are 2g-20MHz: mcs9,mcs8
			 */
			msb = (sr11->offset_40in80_h[0] >> 12) & 0x3;
			wlc_phy_txpwr_ppr_bit_ext_mcs8and9(&mcs20_offset_2g, msb);
		}

		if (CHSPEC_IS20(chanspec)) {
			ppr_dsss_rateset_t	cck20_offset;

			/* 2G - CCK */
			wlc_phy_txpwr_srom_convert_cck(sr11->cck.bw20, tmp_max_pwr, &cck20_offset);

			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20, &cck20_offset, pi);
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, &ofdm20_offset_2g, pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr,  WL_TX_BW_20, &mcs20_offset_2g, pi);
		} else if (CHSPEC_IS40(chanspec)) {
			ppr_dsss_rateset_t	cck20in40_offset = {{0, }};
			ppr_ofdm_rateset_t	ofdm20in40_offset_2g = {{0, }};
			ppr_ofdm_rateset_t	ofdmdup40_offset_2g = {{0, }};
			ppr_ofdm_rateset_t	ofdm40_offset_2g = {{0, }};
			ppr_vht_mcs_rateset_t	mcs40_offset_2g = {{0, }};
			ppr_vht_mcs_rateset_t	mcs20in40_offset_2g = {{0, }};

			/* 2G - CCK */
			wlc_phy_txpwr_srom_convert_cck(sr11->cck.bw20in40,
			        tmp_max_pwr, &cck20in40_offset);

			/* 2G - MCS_40 */
			nibbles = (sr11->offset_2g >> 12) & 0xf;   /* 3LSB is needed */
			wlc_phy_txpwr_srom11_convert_mcs_2g(sr11->mcs_2g.bw40, nibbles,
			        tmp_max_pwr, &mcs40_offset_2g);

			if (ppr_bit_ext) {
				/* msb: bit 1 for mcs9, bit 0 for mcs8
				 * sb40and80hr5glpo, nib3 is 2G
				 * bit15 and bit14 are 2g-40MHz: mcs9,mcs8
				 */
				msb = (sr11->offset_40in80_h[0] >> 14) & 0x3;
				wlc_phy_txpwr_ppr_bit_ext_mcs8and9(&mcs40_offset_2g, msb);
			}
			/* this is used for 2g_ofdm_dup40 mode,
			 * remapping mcs40_offset_2g to ofdm40_offset_2g as the basis for dup
			 */
			wlc_phy_txpwr_srom11_convert_ofdm_2g_dup40(sr11->mcs_2g.bw40,
			        nibbles, tmp_max_pwr, &ofdm40_offset_2g);

			/* 2G - OFDM_20IN40 */
			nibbles = wlc_phy_make_byte(sr11->offset_20in40_h, sr11->offset_20in40_l);
			wlc_phy_txpwr_srom11_convert_ofdm_offset(&ofdm20_offset_2g, nibbles,
				&ofdm20in40_offset_2g);

			/* 2G - MCS_20IN40 */
			wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs20_offset_2g, nibbles,
			        &mcs20in40_offset_2g);

			/* 2G OFDM_DUP40 */
			nibbles = wlc_phy_make_byte(sr11->offset_dup_h, sr11->offset_dup_l);
			wlc_phy_txpwr_srom11_convert_ofdm_offset(&ofdm40_offset_2g, nibbles,
			        &ofdmdup40_offset_2g);

			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40, &ofdmdup40_offset_2g,
				pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr,  WL_TX_BW_40, &mcs40_offset_2g, pi);

			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &cck20in40_offset, pi);
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &ofdm20in40_offset_2g, pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr,  WL_TX_BW_20IN40,
			                     &mcs20in40_offset_2g, pi);
		}
	}

#ifdef BAND5G
	else if (CHSPEC_IS5G(chanspec)) {
		uint8 band5g = band - 1;
		int bitN = (band == 1) ? 4 : ((band == 2) ? 8 : 12);
		ppr_ofdm_rateset_t	ofdm20_offset_5g;
		ppr_vht_mcs_rateset_t	mcs20_offset_5g;

		/* 5G 11agnac_20IN20 */
		nibbles = sr11->offset_5g[band5g] & 0xf;		/* 0LSB */
		wlc_phy_txpwr_srom11_convert_ofdm_5g(sr11->ofdm_5g.bw20[band5g],
		        nibbles, tmp_max_pwr, &ofdm20_offset_5g);
		wlc_phy_txpwr_srom11_convert_mcs_5g(sr11->ofdm_5g.bw20[band5g],
		        nibbles, tmp_max_pwr, &mcs20_offset_5g);

		if (ppr_bit_ext) {
			/* msb: bit 1 for mcs9, bit 0 for mcs8
			 * sb40and80hr5glpo, nib2 and nib1 is 5G-low
			 * sb40and80hr5gmpo, nib2 and nib1 is 5G-mid
			 * sb40and80hr5ghpo, nib2 and nib1 is 5G-high
			 * bit5 and bit4 are 5g-20MHz: mcs9,mcs8
			 */
			msb = (sr11->offset_40in80_h[band5g] >> 4) & 0x3;
			wlc_phy_txpwr_ppr_bit_ext_mcs8and9(&mcs20_offset_5g, msb);
		}

		if (CHSPEC_IS20(chanspec)) {
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, &ofdm20_offset_5g, pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_20, &mcs20_offset_5g, pi);
		} else {
			ppr_ofdm_rateset_t	ofdm40_offset_5g = {{0, }};
			ppr_vht_mcs_rateset_t	mcs40_offset_5g = {{0, }};

			/* 5G 11nac 40IN40 */
			nibbles = (sr11->offset_5g[band5g] >> 4) & 0xf; /* 1LSB */
			wlc_phy_txpwr_srom11_convert_mcs_5g(sr11->mcs_5g.bw40[band5g],
			        nibbles, tmp_max_pwr, &mcs40_offset_5g);

			if (ppr_bit_ext) {
				/* msb: bit 1 for mcs9, bit 0 for mcs8
				 * sb40and80hr5glpo, nib2 and nib1 is 5G-low
				 * sb40and80hr5gmpo, nib2 and nib1 is 5G-mid
				 * sb40and80hr5ghpo, nib2 and nib1 is 5G-high
				 * bit7andbit6 are 5g-40MHz: mcs9, mcs8
				 */
				msb = (sr11->offset_40in80_h[band5g] >> 6) & 0x3;
				wlc_phy_txpwr_ppr_bit_ext_mcs8and9(&mcs40_offset_5g, msb);
			}

			/* same for ofdm 5g dup40 in 40MHz and dup80 in 80MHz */
			wlc_phy_txpwr_srom11_convert_ofdm_5g(sr11->mcs_5g.bw40[band5g],
			        nibbles, tmp_max_pwr, &ofdm40_offset_5g);

			if (CHSPEC_IS40(chanspec)) {
				ppr_ofdm_rateset_t	ofdm20in40_offset_5g;
				ppr_ofdm_rateset_t	ofdmdup40_offset_5g;
				ppr_vht_mcs_rateset_t	mcs20in40_offset_5g;

				/* 5G 11agnac_20IN40 */
				nibbles = wlc_phy_make_byte(sr11->offset_20in40_h >> bitN,
				                            sr11->offset_20in40_l >> bitN);
				wlc_phy_txpwr_srom11_convert_ofdm_offset(&ofdm20_offset_5g,
				        nibbles, &ofdm20in40_offset_5g);
				wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs20_offset_5g,
				        nibbles, &mcs20in40_offset_5g);

				/* 5G ofdm_DUP40 */
				nibbles = wlc_phy_make_byte(sr11->offset_dup_h >> bitN,
				                            sr11->offset_dup_l >> bitN);
				wlc_phy_txpwr_srom11_convert_ofdm_offset((ppr_ofdm_rateset_t*)
				      &ofdm40_offset_5g, nibbles, &ofdmdup40_offset_5g);

				wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40,
				                     &ofdmdup40_offset_5g, pi);
				wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_40,
				                     &mcs40_offset_5g, pi);

				wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40,
				                     &ofdm20in40_offset_5g, pi);
				wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_20IN40,
				                     &mcs20in40_offset_5g, pi);

#ifdef WL11AC
			} else if (CHSPEC_IS80(chanspec)) {
				ppr_ofdm_rateset_t	ofdm20in80_offset_5g = {{0, }};
				ppr_ofdm_rateset_t	ofdm80_offset_5g = {{0, }};
				ppr_ofdm_rateset_t	ofdmdup80_offset_5g = {{0, }};
				ppr_ofdm_rateset_t	ofdmquad80_offset_5g = {{0, }};
				ppr_vht_mcs_rateset_t	mcs80_offset_5g = {{0, }};
				ppr_vht_mcs_rateset_t	mcs20in80_offset_5g = {{0, }};
				ppr_vht_mcs_rateset_t	mcs40in80_offset_5g = {{0, }};

				/* 5G 11nac 80IN80 */
				nibbles = (sr11->offset_5g[band5g] >> 8) & 0xf; /* 2LSB */
				wlc_phy_txpwr_srom11_convert_mcs_5g(sr11->mcs_5g.bw80[band5g],
				        nibbles, tmp_max_pwr, &mcs80_offset_5g);
				wlc_phy_txpwr_srom11_convert_ofdm_5g(sr11->mcs_5g.bw80[band5g],
				        nibbles, tmp_max_pwr, &ofdm80_offset_5g);

				if (ppr_bit_ext) {
					/* msb: bit 1 for mcs9, bit 0 for mcs8
					 * sb40and80hr5glpo, nib2 and nib1 is 5G-low
					 * sb40and80hr5gmpo, nib2 and nib1 is 5G-mid
					 * sb40and80hr5ghpo, nib2 and nib1 is 5G-high
					 * bit9andbit8 are 5g-80MHz: mcs9,mcs8
					 */
					msb = (sr11->offset_40in80_h[band5g] >> 8) & 0x3;
					wlc_phy_txpwr_ppr_bit_ext_mcs8and9(&mcs80_offset_5g, msb);
				}

				/* 5G ofdm_QUAD80, 80in80 */
				nibbles = wlc_phy_make_byte(sr11->offset_dup_h >> bitN,
				                            sr11->offset_dup_l >> bitN);
				wlc_phy_txpwr_srom11_convert_ofdm_offset((ppr_ofdm_rateset_t*)
				        &ofdm80_offset_5g, nibbles, &ofdmquad80_offset_5g);

				/* 5G ofdm_DUP40in80 */
				wlc_phy_txpwr_srom11_convert_ofdm_offset((ppr_ofdm_rateset_t*)
			            &ofdm40_offset_5g, nibbles, &ofdmdup80_offset_5g);

				/* 5G 11agnac_20Ul/20LU/20UU/20LL */
				/* 8 for 20LU/20UL subband  */
				nibbles = wlc_phy_make_byte(sr11->offset_20in80_h[band5g],
				                            sr11->offset_20in80_l[band5g]);
				wlc_phy_txpwr_srom11_convert_ofdm_offset(
				        &ofdm20_offset_5g, nibbles, &ofdm20in80_offset_5g);
				wlc_phy_txpwr_srom11_convert_mcs_offset(
				        &mcs20_offset_5g, nibbles, &mcs20in80_offset_5g);

				if ((CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_UU) ||
					(CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_LL)) {
					/* for 20UU/20LL subband = offset + 20UL/20LU */
					/* 8 for 20LL/20UU subband  */
					nibbles = wlc_phy_make_byte(
							sr11->offset_20in80_h[band5g] >> 2,
							sr11->offset_20in80_l[band5g] >> 2);
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
					    &ofdm20in80_offset_5g, nibbles, &ofdm20in80_offset_5g);
					wlc_phy_txpwr_srom11_convert_mcs_offset(
					    &mcs20in80_offset_5g, nibbles, &mcs20in80_offset_5g);
				}

				/* 5G 11nac_40IN80 */
				nibbles = wlc_phy_make_byte(sr11->offset_40in80_h[band5g],
				                            sr11->offset_40in80_l[band5g]);
				wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs40_offset_5g,
					nibbles, &mcs40in80_offset_5g);

				/* for 80IN80MHz OFDM or OFDMQUAD80 */
				wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_80,
				                     &ofdmquad80_offset_5g, pi);
				/* for 80IN80MHz HT */
				wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_80,
				                     &mcs80_offset_5g, pi);
				/* for ofdm_20IN80: S1x1, S1x2, S1x3 */
				wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN80,
				                     &ofdm20in80_offset_5g, pi);
				/* for 20IN80MHz HT */
				wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_20IN80,
				                     &mcs20in80_offset_5g, pi);
				/* for 40IN80MHz HT */
				wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_40IN80,
				                     &mcs40in80_offset_5g, pi);

				/* for ofdm_40IN80: S1x1, S1x2, S1x3 */
				wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40IN80,
				                     &ofdmdup80_offset_5g, pi);
#endif /* WL11AC */

			}
		}
	}
#endif /* BAND5G */
}

#ifdef WL11AC
void
wlc_phy_txpwr_apply_srom13(phy_info_t *pi, uint8 band, chanspec_t chanspec,
                           uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr)
{
	uint8 nibbles;
	uint8 ppr_bit_ext, msb;
	uint32 ofdm_po;

	const struct srom13_ppr *sr13 = &pi->ppr.sr13;

	if (!ISACPHY(pi))
		return;

	ppr_bit_ext = 1;

	if (CHSPEC_IS2G(chanspec)) {
		ppr_ofdm_rateset_t	ofdm20_offset_2g;
		ppr_vht_mcs_rateset_t	mcs20_offset_2g;

		/* 2G - OFDM_20 */
		ofdm_po = (uint32)(sr13->ofdm_2g.bw20);

		wlc_phy_txpwr_srom_convert_ofdm(ofdm_po, tmp_max_pwr, &ofdm20_offset_2g);

		/* 2G - MCS_20 */
		nibbles = (sr13->offset_2g >> 8) & 0xf;   /* 2LSB is needed */
		wlc_phy_txpwr_srom11_convert_mcs_2g(sr13->mcs_2g.bw20, nibbles,
		        tmp_max_pwr, &mcs20_offset_2g);
		wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_2g(sr13->pp1024qam2g,
		        chanspec, tmp_max_pwr, &mcs20_offset_2g);
		if (ppr_bit_ext) {
			/* msb: bit 3->0 for mcs11->mcs8 */
			msb = ((sr13->ppmcsexp[3] & 0x1) << 3) |
					((sr13->ppmcsexp[2] & 0x1) << 2) |
					((sr13->ppmcsexp[1] & 0x1) << 1) |
					(sr13->ppmcsexp[0] & 0x1);
			wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(&mcs20_offset_2g, msb);
		}

#ifdef WL11ULB
		if (CHSPEC_IS20(chanspec) || CHSPEC_ISLE20(chanspec)) {
#else
		if (CHSPEC_IS20(chanspec)) {
#endif // endif
			ppr_dsss_rateset_t	cck20_offset;

			/* 2G - CCK */
			wlc_phy_txpwr_srom_convert_cck(sr13->cck.bw20, tmp_max_pwr, &cck20_offset);

			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20, &cck20_offset, pi);
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, &ofdm20_offset_2g, pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr,  WL_TX_BW_20, &mcs20_offset_2g, pi);
		} else if (CHSPEC_IS40(chanspec)) {
			ppr_dsss_rateset_t	cck20in40_offset;
			ppr_ofdm_rateset_t	ofdm20in40_offset_2g;
			ppr_ofdm_rateset_t	ofdmdup40_offset_2g;
			ppr_ofdm_rateset_t	ofdm40_offset_2g;
			ppr_vht_mcs_rateset_t	mcs40_offset_2g;
			ppr_vht_mcs_rateset_t	mcs20in40_offset_2g;

			/* 2G - CCK */
			wlc_phy_txpwr_srom_convert_cck(sr13->cck.bw20in40,
			        tmp_max_pwr, &cck20in40_offset);

			/* 2G - MCS_40 */
			nibbles = (sr13->offset_2g >> 12) & 0xf;   /* 3LSB is needed */
			wlc_phy_txpwr_srom11_convert_mcs_2g(sr13->mcs_2g.bw40, nibbles,
			        tmp_max_pwr, &mcs40_offset_2g);
			wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_2g(sr13->pp1024qam2g,
				chanspec, tmp_max_pwr, &mcs40_offset_2g);
			if (ppr_bit_ext) {
				/* msb: bit 3 for mcs11, bit 0 for mcs8 */
				msb = (((sr13->ppmcsexp[3]>>8) & 0x1) << 3) |
					(((sr13->ppmcsexp[2]>>8) & 0x1) << 2) |
					(((sr13->ppmcsexp[1]>>8) & 0x1) << 1) |
					((sr13->ppmcsexp[0]>>8) & 0x1);
				wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(&mcs40_offset_2g, msb);
			}
			/* this is used for 2g_ofdm_dup40 mode,
			 * remapping mcs40_offset_2g to ofdm40_offset_2g as the basis for dup
			 */
			wlc_phy_txpwr_srom11_convert_ofdm_2g_dup40(sr13->mcs_2g.bw40,
			        nibbles, tmp_max_pwr, &ofdm40_offset_2g);

			/* 2G - OFDM_20IN40 */
			nibbles = wlc_phy_make_byte(sr13->offset_20in40_h, sr13->offset_20in40_l);
			wlc_phy_txpwr_srom11_convert_ofdm_offset(&ofdm20_offset_2g, nibbles,
				&ofdm20in40_offset_2g);

			/* 2G - MCS_20IN40 */
			wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs20_offset_2g, nibbles,
			        &mcs20in40_offset_2g);

			/* 2G OFDM_DUP40 */
			nibbles = wlc_phy_make_byte(sr13->offset_dup_h, sr13->offset_dup_l);
			wlc_phy_txpwr_srom11_convert_ofdm_offset(&ofdm40_offset_2g, nibbles,
			        &ofdmdup40_offset_2g);

			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40, &ofdmdup40_offset_2g,
				pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr,  WL_TX_BW_40, &mcs40_offset_2g, pi);

			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &cck20in40_offset, pi);
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &ofdm20in40_offset_2g, pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr,  WL_TX_BW_20IN40,
			                     &mcs20in40_offset_2g, pi);
		}
	}

#ifdef BAND5G
	else if (CHSPEC_IS5G(chanspec)) {
		uint8 band5g = band - 1;
		int bitN = (band == 1) ? 4 : ((band == 2) ? 8 : 12);
		ppr_ofdm_rateset_t	ofdm20_offset_5g;
		ppr_vht_mcs_rateset_t	mcs20_offset_5g;

		/* 5G 11agnac_20IN20 */
		nibbles = sr13->offset_5g[band5g] & 0xf;		/* 0LSB */
		wlc_phy_txpwr_srom11_convert_ofdm_5g(sr13->ofdm_5g.bw20[band5g],
		        nibbles, tmp_max_pwr, &ofdm20_offset_5g);
		wlc_phy_txpwr_srom11_convert_mcs_5g(sr13->ofdm_5g.bw20[band5g],
		        nibbles, tmp_max_pwr, &mcs20_offset_5g);

		wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_5g(sr13->pp1024qam5g[band5g],
			WL_PKT_BW_20, tmp_max_pwr, &mcs20_offset_5g);

		if (ppr_bit_ext) {
			/* msb: bit 3 for mcs11, bit 0 for mcs8 */
			msb = (((sr13->ppmcsexp[3]>>(band5g+1)) & 0x1) << 3) |
				(((sr13->ppmcsexp[2]>>(band5g+1)) & 0x1) << 2) |
				(((sr13->ppmcsexp[1]>>(band5g+1)) & 0x1) << 1) |
				((sr13->ppmcsexp[0]>>(band5g+1)) & 0x1);
			wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(&mcs20_offset_5g, msb);
		}

#ifdef WL11ULB
		if (CHSPEC_IS20(chanspec) || CHSPEC_ISLE20(chanspec)) {
#else
		if (CHSPEC_IS20(chanspec)) {
#endif // endif
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, &ofdm20_offset_5g, pi);
			wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_20, &mcs20_offset_5g, pi);
		} else {
			ppr_ofdm_rateset_t	ofdm40_offset_5g;
			ppr_vht_mcs_rateset_t	mcs40_offset_5g;

			/* 5G 11nac 40IN40 */
			nibbles = (sr13->offset_5g[band5g] >> 4) & 0xf; /* 1LSB */
			wlc_phy_txpwr_srom11_convert_mcs_5g(sr13->mcs_5g.bw40[band5g],
			        nibbles, tmp_max_pwr, &mcs40_offset_5g);

			wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_5g(
				sr13->pp1024qam5g[band5g],
				WL_PKT_BW_40, tmp_max_pwr, &mcs40_offset_5g);

			if (ppr_bit_ext) {
				/* msb: bit 3 for mcs11, bit 0 for mcs8 */
				msb = (((sr13->ppmcsexp[3]>>(band5g+9)) & 0x1) << 3) |
					(((sr13->ppmcsexp[2]>>(band5g+9)) & 0x1) << 2) |
					(((sr13->ppmcsexp[1]>>(band5g+9)) & 0x1) << 1) |
					((sr13->ppmcsexp[0]>>(band5g+9)) & 0x1);
				wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(&mcs40_offset_5g, msb);
			}

			/* same for ofdm 5g dup40 in 40MHz and dup80 in 80MHz */
			wlc_phy_txpwr_srom11_convert_ofdm_5g(sr13->mcs_5g.bw40[band5g],
			        nibbles, tmp_max_pwr, &ofdm40_offset_5g);

			if (CHSPEC_IS40(chanspec)) {
				ppr_ofdm_rateset_t	ofdm20in40_offset_5g;
				ppr_ofdm_rateset_t	ofdmdup40_offset_5g;
				ppr_vht_mcs_rateset_t	mcs20in40_offset_5g;

				/* 5G 11agnac_20IN40 */
				nibbles = wlc_phy_make_byte(sr13->offset_20in40_h >> bitN,
				                            sr13->offset_20in40_l >> bitN);
				wlc_phy_txpwr_srom11_convert_ofdm_offset(&ofdm20_offset_5g,
				        nibbles, &ofdm20in40_offset_5g);
				wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs20_offset_5g,
				        nibbles, &mcs20in40_offset_5g);

				/* 5G ofdm_DUP40 */
				nibbles = wlc_phy_make_byte(sr13->offset_dup_h >> bitN,
				                            sr13->offset_dup_l >> bitN);
				wlc_phy_txpwr_srom11_convert_ofdm_offset((ppr_ofdm_rateset_t*)
				      &ofdm40_offset_5g, nibbles, &ofdmdup40_offset_5g);

				wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40,
				                     &ofdmdup40_offset_5g, pi);
				wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_40,
				                     &mcs40_offset_5g, pi);

				wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40,
				                     &ofdm20in40_offset_5g, pi);
				wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_20IN40,
				                     &mcs20in40_offset_5g, pi);

			} else {
				ppr_ofdm_rateset_t      ofdm80_offset_5g;
				ppr_vht_mcs_rateset_t   mcs80_offset_5g;

				/* 5G 11nac 80IN80 */
				nibbles = (sr13->offset_5g[band5g] >> 8) & 0xf; /* 2LSB */
				wlc_phy_txpwr_srom11_convert_ofdm_5g(sr13->mcs_5g.bw80[band5g],
					nibbles, tmp_max_pwr, &ofdm80_offset_5g);
				wlc_phy_txpwr_srom11_convert_mcs_5g(sr13->mcs_5g.bw80[band5g],
					nibbles, tmp_max_pwr, &mcs80_offset_5g);

				wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_5g(
					sr13->pp1024qam5g[band5g],
					WL_PKT_BW_80, tmp_max_pwr, &mcs80_offset_5g);

				if (ppr_bit_ext) {
					/* msb: bit 3 for mcs11, bit 0 for mcs8 */
					msb = (((sr13->ppmcsexp[3]>>(band5g+17)) & 0x1) << 3) |
						(((sr13->ppmcsexp[2]>>(band5g+17)) & 0x1) << 2) |
						(((sr13->ppmcsexp[1]>>(band5g+17)) & 0x1) << 1) |
						((sr13->ppmcsexp[0]>>(band5g+17)) & 0x1);
					wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(&mcs80_offset_5g,
						msb);
				}

				if (CHSPEC_IS80(chanspec)) {
					ppr_ofdm_rateset_t	ofdm20in80_offset_5g;
					ppr_ofdm_rateset_t	ofdmdup80_offset_5g;
					ppr_ofdm_rateset_t	ofdmquad80_offset_5g;
					ppr_vht_mcs_rateset_t	mcs20in80_offset_5g;
					ppr_vht_mcs_rateset_t	mcs40in80_offset_5g;

					/* 5G ofdm_QUAD80, 80in80 */
					nibbles = wlc_phy_make_byte(sr13->offset_dup_h >> bitN,
						sr13->offset_dup_l >> bitN);
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
						(ppr_ofdm_rateset_t*)&ofdm80_offset_5g,
						nibbles, &ofdmquad80_offset_5g);

					/* 5G ofdm_DUP40in80 */
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
						(ppr_ofdm_rateset_t*)&ofdm40_offset_5g,
						nibbles, &ofdmdup80_offset_5g);

					/* 5G 11agnac_20Ul/20LU/20UU/20LL */
					/* 8 for 20LU/20UL subband  */
					nibbles = wlc_phy_make_byte(sr13->offset_20in80_h[band5g],
							sr13->offset_20in80_l[band5g]);
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
						&ofdm20_offset_5g, nibbles, &ofdm20in80_offset_5g);
					wlc_phy_txpwr_srom11_convert_mcs_offset(
						&mcs20_offset_5g, nibbles, &mcs20in80_offset_5g);

					if ((CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_UU) ||
						(CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_LL))
					{
						/* for 20UU/20LL subband = offset + 20UL/20LU */
						/* 8 for 20LL/20UU subband  */
						nibbles = wlc_phy_make_byte(
							sr13->offset_20in80_h[band5g] >> 2,
							sr13->offset_20in80_l[band5g] >> 2);
						wlc_phy_txpwr_srom11_convert_ofdm_offset(
						&ofdm20in80_offset_5g, nibbles,
						&ofdm20in80_offset_5g);
						wlc_phy_txpwr_srom11_convert_mcs_offset(
							&mcs20in80_offset_5g, nibbles,
							&mcs20in80_offset_5g);
					}

					/* 5G 11nac_40IN80 */
					nibbles = wlc_phy_make_byte(sr13->offset_40in80_h[band5g],
							sr13->offset_40in80_l[band5g]);
					wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs40_offset_5g,
						nibbles, &mcs40in80_offset_5g);

					/* for 80IN80MHz OFDM or OFDMQUAD80 */
					wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_80,
							&ofdmquad80_offset_5g, pi);
					/* for 80IN80MHz HT */
					wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_80,
							&mcs80_offset_5g, pi);
					/* for ofdm_20IN80: S1x1, S1x2, S1x3 */
					wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN80,
							&ofdm20in80_offset_5g, pi);
					/* for 20IN80MHz HT */
					wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_20IN80,
						&mcs20in80_offset_5g, pi);
					/* for 40IN80MHz HT */
					wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_40IN80,
						&mcs40in80_offset_5g, pi);

					/* for ofdm_40IN80: S1x1, S1x2, S1x3 */
					wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40IN80,
						&ofdmdup80_offset_5g, pi);
				} else if (CHSPEC_IS160(chanspec)) {
					ppr_ofdm_rateset_t      ofdm160_offset_5g;
					ppr_ofdm_rateset_t      ofdm20in160_offset_5g;
					ppr_ofdm_rateset_t      ofdmdup160_offset_5g;
					ppr_ofdm_rateset_t      ofdmquad160_offset_5g;
					ppr_ofdm_rateset_t      ofdmoct160_offset_5g;
					ppr_vht_mcs_rateset_t   mcs160_offset_5g;
					ppr_vht_mcs_rateset_t   mcs20in160_offset_5g;
					ppr_vht_mcs_rateset_t   mcs40in160_offset_5g;
					ppr_vht_mcs_rateset_t   mcs80in160_offset_5g;

					/* 5G 11nac 160IN160 */
					/* 12 for  bandwidth 160 */
					nibbles = (sr13->offset_5g[band5g] >> 12) & 0xf;
					wlc_phy_txpwr_srom11_convert_ofdm_5g(
						pi->pprsr13_mcs_5g_bw160[band5g],
						nibbles, tmp_max_pwr, &ofdm160_offset_5g);
					wlc_phy_txpwr_srom11_convert_mcs_5g(
						pi->pprsr13_mcs_5g_bw160[band5g],
						nibbles, tmp_max_pwr, &mcs160_offset_5g);

					wlc_phy_txpwr_srom13_ext_1024qam_convert_mcs_5g(
						sr13->pp1024qam5g[band5g],
						WL_PKT_BW_160, tmp_max_pwr, &mcs160_offset_5g);

					if (ppr_bit_ext) {
						/* msb: bit 3 for mcs11, bit 0 for mcs8 */
						msb = (((sr13->ppmcsexp[3]>>(band5g+25)) &
							0x1) << 3) |
							(((sr13->ppmcsexp[2]>>(band5g+25)) &
							0x1) << 2) |
							(((sr13->ppmcsexp[1]>>(band5g+25)) &
							0x1) << 1) |
							((sr13->ppmcsexp[0]>>(band5g+25)) &
							0x1);
						wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(
							&mcs160_offset_5g,
							msb);
					}

					/* 5G ofdm_OCT160, 160in160 */
					nibbles = wlc_phy_make_byte(sr13->offset_dup_h >> bitN,
						sr13->offset_dup_l >> bitN);
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
						(ppr_ofdm_rateset_t*)&ofdm160_offset_5g,
						nibbles, &ofdmoct160_offset_5g);

					/* 5G ofdm_QUAD80in160 */
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
						(ppr_ofdm_rateset_t*)&ofdm80_offset_5g,
						nibbles, &ofdmquad160_offset_5g);

					/* 5G ofdm_DUP40in160 */
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
						(ppr_ofdm_rateset_t*)&ofdm40_offset_5g,
						nibbles, &ofdmdup160_offset_5g);

					/* 5G 11agnac_20IN160 */
					nibbles = wlc_phy_make_byte(
						sr13->offset_20in80_h[band5g] >> 4,
						sr13->offset_20in80_l[band5g] >> 4);
					wlc_phy_txpwr_srom11_convert_ofdm_offset(
						&ofdm20_offset_5g, nibbles,
						&ofdm20in160_offset_5g);
					wlc_phy_txpwr_srom11_convert_mcs_offset(
						&mcs20_offset_5g, nibbles, &mcs20in160_offset_5g);
					if ((CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_UUU) ||
						(CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_LLL))
					{
						/* 12 for 20LLL/20UUU subband w.r.t 20in160 sb */
						nibbles = wlc_phy_make_byte(
							sr13->offset_20in80_h[band5g] >> 12,
							sr13->offset_20in80_l[band5g] >> 12);
						wlc_phy_txpwr_srom11_convert_ofdm_offset(
							&ofdm20in160_offset_5g, nibbles,
							&ofdm20in160_offset_5g);
						wlc_phy_txpwr_srom11_convert_mcs_offset(
							&mcs20in160_offset_5g, nibbles,
							&mcs20in160_offset_5g);
					}

					/* 5G 11nac_40IN160 */
					nibbles = wlc_phy_make_byte(
							sr13->offset_40in80_h[band5g] >> 4,
							sr13->offset_40in80_l[band5g] >> 4);
					wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs40_offset_5g,
							nibbles, &mcs40in160_offset_5g);
					if ((CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_UU) ||
						(CHSPEC_CTL_SB(chanspec) == WL_CHANSPEC_CTL_SB_LL))
					{
						/* 12 for 40LL/40UU subband w.r.t to 40LU/UL */
						nibbles = wlc_phy_make_byte(
							sr13->offset_40in80_h[band5g] >> 12,
							sr13->offset_40in80_l[band5g] >> 12);
						wlc_phy_txpwr_srom11_convert_mcs_offset(
							&mcs40in160_offset_5g,
							nibbles, &mcs40in160_offset_5g);
					}

					/* 5G 11nac_80IN160 */
					nibbles = wlc_phy_make_byte(
						sr13->offset_40in80_h[band5g] >> 8,
						sr13->offset_40in80_l[band5g] >> 8);
					wlc_phy_txpwr_srom11_convert_mcs_offset(&mcs80_offset_5g,
						nibbles, &mcs80in160_offset_5g);

					/* for 160IN160MHz OFDM or OFDMQUAD160 */
					wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_160,
						&ofdmoct160_offset_5g, pi);
					/* for ofdm_20IN160: S1x1, S1x2, S1x3, S1x4 */
					wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN160,
						&ofdm20in160_offset_5g, pi);
					/* for ofdm_40IN160: S1x1, S1x2, S1x3, S1x4 */
					wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40IN160,
						&ofdmdup160_offset_5g, pi);
					/* for ofdm_80IN160: S1x1, S1x2, S1x3, S1x4 */
					wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_80IN160,
						&ofdmquad160_offset_5g, pi);

					/* for 160IN160MHz HT */
					wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_160,
						&mcs160_offset_5g, pi);
					/* for 20IN160MHz HT */
					wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_20IN160,
						&mcs20in160_offset_5g, pi);
					/* for 40IN160MHz HT */
					wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_40IN160,
						&mcs40in160_offset_5g, pi);
					/* for 80IN160MHz HT */
					wlc_phy_ppr_set_mcs(tx_srom_max_pwr, WL_TX_BW_80IN160,
						&mcs80in160_offset_5g, pi);
				}
			}
		}
	}
#endif /* BAND5G */

}

#endif /* WL11AC */

uint8
wlc_phy_get_band_from_channel(phy_info_t *pi, uint channel)
{
	/* NOTE: At present this function has only been validated for
	 * LCN and LCN40 phys. It may apply to others, but
	 * that is left as an exercise to the reader.
	 */
	uint8 band = 0;

#ifdef BAND5G
	if ((channel >= FIRST_LOW_5G_CHAN_SSLPNPHY) &&
		(channel <= LAST_LOW_5G_CHAN_SSLPNPHY)) {
		band = WL_CHAN_FREQ_RANGE_5GL;
	} else if ((channel >= FIRST_MID_5G_CHAN_SSLPNPHY) &&
		(channel <= LAST_MID_5G_CHAN_SSLPNPHY)) {
		band = WL_CHAN_FREQ_RANGE_5GM;
	} else if ((channel >= FIRST_HIGH_5G_CHAN_SSLPNPHY) &&
		(channel <= LAST_HIGH_5G_CHAN_SSLPNPHY)) {
		band = WL_CHAN_FREQ_RANGE_5GH;
	} else
#endif /* BAND5G */
	if (channel <= CH_MAX_2G_CHANNEL) {
		band = WL_CHAN_FREQ_RANGE_2G;
	} else {
		PHY_ERROR(("%s: invalid channel %d\n", __FUNCTION__, channel));
		ASSERT(0);
	}

	return band;
}
#endif /* PPR_API */

static const char BCMATTACHDATA(rstr_cckbw202gpo)[] = "cckbw202gpo";
static const char BCMATTACHDATA(rstr_cckbw20ul2gpo)[] = "cckbw20ul2gpo";
static const char BCMATTACHDATA(rstr_legofdmbw202gpo)[] = "legofdmbw202gpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul2gpo)[] = "legofdmbw20ul2gpo";
static const char BCMATTACHDATA(rstr_legofdmbw205glpo)[] = "legofdmbw205glpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5glpo)[] = "legofdmbw20ul5glpo";
static const char BCMATTACHDATA(rstr_legofdmbw205gmpo)[] = "legofdmbw205gmpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5gmpo)[] = "legofdmbw20ul5gmpo";
static const char BCMATTACHDATA(rstr_legofdmbw205ghpo)[] = "legofdmbw205ghpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5ghpo)[] = "legofdmbw20ul5ghpo";
static const char BCMATTACHDATA(rstr_mcsbw202gpo)[] = "mcsbw202gpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul2gpo)[] = "mcsbw20ul2gpo";
static const char BCMATTACHDATA(rstr_mcsbw402gpo)[] = "mcsbw402gpo";
static const char BCMATTACHDATA(rstr_mcsbw205glpo)[] = "mcsbw205glpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5glpo)[] = "mcsbw20ul5glpo";
static const char BCMATTACHDATA(rstr_mcsbw405glpo)[] = "mcsbw405glpo";
static const char BCMATTACHDATA(rstr_mcsbw205gmpo)[] = "mcsbw205gmpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gmpo)[] = "mcsbw20ul5gmpo";
static const char BCMATTACHDATA(rstr_mcsbw405gmpo)[] = "mcsbw405gmpo";
static const char BCMATTACHDATA(rstr_mcsbw205ghpo)[] = "mcsbw205ghpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5ghpo)[] = "mcsbw20ul5ghpo";
static const char BCMATTACHDATA(rstr_mcsbw405ghpo)[] = "mcsbw405ghpo";
static const char BCMATTACHDATA(rstr_legofdm40duppo)[] = "legofdm40duppo";
static const char BCMATTACHDATA(rstr_ofdmlrbw202gpo)[] = "ofdmlrbw202gpo";
static const char BCMATTACHDATA(rstr_dot11agofdmhrbw202gpo)[] = "dot11agofdmhrbw202gpo";
static const char BCMATTACHDATA(rstr_sb20in40lrpo)[] = "sb20in40lrpo";
static const char BCMATTACHDATA(rstr_sb20in40hrpo)[] = "sb20in40hrpo";
static const char BCMATTACHDATA(rstr_dot11agduphrpo)[] = "dot11agduphrpo";
static const char BCMATTACHDATA(rstr_dot11agduplrpo)[] = "dot11agduplrpo";
static const char BCMATTACHDATA(rstr_mcslr5glpo)[] = "mcslr5glpo";
static const char BCMATTACHDATA(rstr_mcslr5gmpo)[] = "mcslr5gmpo";
static const char BCMATTACHDATA(rstr_mcslr5ghpo)[] = "mcslr5ghpo";
static const char BCMATTACHDATA(rstr_mcsbw805glpo)[] = "mcsbw805glpo";
static const char BCMATTACHDATA(rstr_mcsbw805gmpo)[] = "mcsbw805gmpo";
static const char BCMATTACHDATA(rstr_mcsbw805ghpo)[] = "mcsbw805ghpo";
static const char BCMATTACHDATA(rstr_sb20in80and160lr5glpo)[] = "sb20in80and160lr5glpo";
static const char BCMATTACHDATA(rstr_sb20in80and160hr5glpo)[] = "sb20in80and160hr5glpo";
static const char BCMATTACHDATA(rstr_sb20in80and160lr5gmpo)[] = "sb20in80and160lr5gmpo";
static const char BCMATTACHDATA(rstr_sb20in80and160hr5gmpo)[] = "sb20in80and160hr5gmpo";
static const char BCMATTACHDATA(rstr_sb20in80and160lr5ghpo)[] = "sb20in80and160lr5ghpo";
static const char BCMATTACHDATA(rstr_sb20in80and160hr5ghpo)[] = "sb20in80and160hr5ghpo";
static const char BCMATTACHDATA(rstr_sb40and80lr5glpo)[] = "sb40and80lr5glpo";
static const char BCMATTACHDATA(rstr_sb40and80hr5glpo)[] = "sb40and80hr5glpo";
static const char BCMATTACHDATA(rstr_sb40and80lr5gmpo)[] = "sb40and80lr5gmpo";
static const char BCMATTACHDATA(rstr_sb40and80hr5gmpo)[] = "sb40and80hr5gmpo";
static const char BCMATTACHDATA(rstr_sb40and80lr5ghpo)[] = "sb40and80lr5ghpo";
static const char BCMATTACHDATA(rstr_sb40and80hr5ghpo)[] = "sb40and80hr5ghpo";
static const char BCMATTACHDATA(rstr_mcsbw1605glpo)[] = "mcsbw1605glpo";
static const char BCMATTACHDATA(rstr_mcsbw1605gmpo)[] = "mcsbw1605gmpo";
static const char BCMATTACHDATA(rstr_mcsbw1605ghpo)[] = "mcsbw1605ghpo";

static const char BCMATTACHDATA(rstr_mcsbw205gx1po)[] = "mcsbw205gx1po";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gx1po)[] = "mcsbw20ul5gx1po";
static const char BCMATTACHDATA(rstr_mcsbw405gx1po)[] = "mcsbw405gx1po";
static const char BCMATTACHDATA(rstr_mcsbw205gx2po)[] = "mcsbw205gx2po";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gx2po)[] = "mcsbw20ul5gx2po";
static const char BCMATTACHDATA(rstr_mcsbw405gx2po)[] = "mcsbw405gx2po";
static const char BCMATTACHDATA(rstr_mcslr5gx1po)[] = "mcslr5gx1po";
static const char BCMATTACHDATA(rstr_mcslr5gx2po)[] = "mcslr5gx2po";
static const char BCMATTACHDATA(rstr_mcsbw805gx1po)[] = "mcsbw805gx1po";
static const char BCMATTACHDATA(rstr_mcsbw805gx2po)[] = "mcsbw805gx2po";
static const char BCMATTACHDATA(rstr_mcsbw1605gx1po)[] = "mcsbw1605gx1po";
static const char BCMATTACHDATA(rstr_mcsbw1605gx2po)[] = "mcsbw1605gx2po";
static const char BCMATTACHDATA(rstr_sb20in80and160lr5gx1po)[] = "sb20in80and160lr5gx1po";
static const char BCMATTACHDATA(rstr_sb20in80and160hr5gx1po)[] = "sb20in80and160hr5gx1po";
static const char BCMATTACHDATA(rstr_sb20in80and160lr5gx2po)[] = "sb20in80and160lr5gx2po";
static const char BCMATTACHDATA(rstr_sb20in80and160hr5gx2po)[] = "sb20in80and160hr5gx2po";
static const char BCMATTACHDATA(rstr_sb40and80lr5gx1po)[] = "sb40and80lr5gx1po";
static const char BCMATTACHDATA(rstr_sb40and80hr5gx1po)[] = "sb40and80hr5gx1po";
static const char BCMATTACHDATA(rstr_sb40and80lr5gx2po)[] = "sb40and80lr5gx2po";
static const char BCMATTACHDATA(rstr_sb40and80hr5gx2po)[] = "sb40and80hr5gx2po";

static const char BCMATTACHDATA(rstr_mcs1024qam2gpo)[] = "mcs1024qam2gpo";
static const char BCMATTACHDATA(rstr_mcs1024qam5glpo)[] = "mcs1024qam5glpo";
static const char BCMATTACHDATA(rstr_mcs1024qam5gmpo)[] = "mcs1024qam5gmpo";
static const char BCMATTACHDATA(rstr_mcs1024qam5ghpo)[] = "mcs1024qam5ghpo";
static const char BCMATTACHDATA(rstr_mcs1024qam5gx1po)[] = "mcs1024qam5gx1po";
static const char BCMATTACHDATA(rstr_mcs1024qam5gx2po)[] = "mcs1024qam5gx2po";

static const char BCMATTACHDATA(rstr_mcs8poexp)[] = "mcs8poexp";
static const char BCMATTACHDATA(rstr_mcs9poexp)[] = "mcs9poexp";
static const char BCMATTACHDATA(rstr_mcs10poexp)[] = "mcs10poexp";
static const char BCMATTACHDATA(rstr_mcs11poexp)[] = "mcs11poexp";

void
wlc_phy_txpwr_srom_convert_mcs(uint32 po, uint8 max_pwr, ppr_ht_mcs_rateset_t *mcs)
{
	wlc_phy_txpwr_srom_convert_mcs_offset(po, 0, max_pwr, mcs, 0);
}

void
wlc_phy_txpwr_srom_convert_mcs_offset(uint32 po,
	uint8 offset, uint8 max_pwr, ppr_ht_mcs_rateset_t* mcs, int8 mcs7_15_offset)
{
	uint8 i;
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		mcs->pwr[i] = max_pwr - ((po & 0xF) * 2) - (offset * 2);
		po >>= 4;
	}

	mcs->pwr[WL_RATESET_SZ_HT_MCS - 1] -= mcs7_15_offset;
}

static void
wlc_phy_txpwr_srom8_sub_ofdm(ppr_ofdm_rateset_t* ofdm, uint8 ppr_offset)
{
	uint8 i;
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		ofdm->pwr[i] = ofdm->pwr[i] - (ppr_offset << 1);
	}
}

static void
wlc_phy_txpwr_srom8_sub_mcs(ppr_ht_mcs_rateset_t* mcs, uint8 ppr_offset)
{
	uint8 i;
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		mcs->pwr[i] = mcs->pwr[i] - (ppr_offset << 1);
	}
}

static void
wlc_phy_ppr_set_dsss_srom8(ppr_t* tx_srom_max_pwr, uint8 bwtype,
          ppr_dsss_rateset_t* pwr_offsets)
{
	uint8 chain;
	for (chain = WL_TX_CHAINS_1; chain <= WL_TX_CHAINS_2; chain++)
		/* for 2g_dsss_20IN20: S1x1, S1x2 */
		ppr_set_dsss(tx_srom_max_pwr, bwtype, chain,
			(const ppr_dsss_rateset_t*)pwr_offsets);
}

static void
wlc_phy_ppr_set_ofdm_srom8(ppr_t* tx_srom_max_pwr, uint8 bwtype, wl_tx_mode_t mode,
          wl_tx_chains_t tx_chain, ppr_ofdm_rateset_t* pwr_offsets)
{
	ppr_set_ofdm(tx_srom_max_pwr, bwtype, mode, tx_chain,
		(const ppr_ofdm_rateset_t*)pwr_offsets);

}

static void
wlc_phy_ppr_set_mcs_srom8(ppr_t* tx_srom_max_pwr, uint8 bwtype, wl_tx_nss_t Nss,
	wl_tx_mode_t mode, wl_tx_chains_t tx_chains, ppr_ht_mcs_rateset_t* pwr_offsets)
{
	ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, Nss, mode,
		tx_chains, (const ppr_ht_mcs_rateset_t*)pwr_offsets);

}

void
wlc_phy_txpwr_apply_srom8(phy_info_t *pi, uint8 band,
	uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr)
{

	chanspec_t chanspec = pi->radio_chanspec;
	uint8 tmp_bw40po = 0, tmp_cddpo = 0, tmp_stbcpo = 0;
	uint32 tmp_mcs_word = 0;

	ppr_dsss_rateset_t cck20_offset;
	ppr_ofdm_rateset_t ofdm20_offset_siso, ofdm20_offset_cdd,
		ofdm40_offset_siso, ofdm40_offset_cdd;

	ppr_ht_mcs_rateset_t mcs20_offset_siso, mcs20_offset_cdd, mcs20_offset_stbc,
		mcs20_offset_sdm, mcs40_offset_siso, mcs40_offset_cdd, mcs40_offset_stbc,
		mcs40_offset_sdm;

	if (band == 0) {	/* 2G case */

		/* ----------------2G--------------------- */
		/* 2G - CCK */
		wlc_phy_txpwr_srom_convert_cck(pi->ppr.sr8.cck2gpo,
		        tmp_max_pwr,  &cck20_offset);

		if (CHSPEC_IS20(chanspec)) {

			/* for 2g_dsss_20IN20: S1x1, S1x2 */
			wlc_phy_ppr_set_dsss_srom8(tx_srom_max_pwr, WL_TX_BW_20, &cck20_offset);
		}
		if (CHSPEC_IS40(chanspec)) {

			/* for 2g_dsss_20IN40: S1x1, S1x2 */
			wlc_phy_ppr_set_dsss_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40, &cck20_offset);
		}

	}

	if (CHSPEC_IS20(chanspec)) {

		/* 2G - OFDM_20 */
		wlc_phy_txpwr_srom_convert_ofdm(pi->ppr.sr8.ofdm[band], tmp_max_pwr,
			&ofdm20_offset_siso);

		/* for ofdm_20IN20: S1x1   */
		wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &ofdm20_offset_siso);

		/* for 20MHz Mapping Legacy OFDM SISo to MCS0-7 SISO */
		wlc_phy_copy_ofdm_to_mcs_powers(&ofdm20_offset_siso, &mcs20_offset_siso);

		/* for mcs_20IN20 SISO: S1x1  */
		wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_1,
			WL_TX_MODE_NONE, 1, &mcs20_offset_siso);

		/* 2G - MCS_CDD  */
		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			/* Apply power-offset specified by the cddpo SROM field to rates sent
			 * in the CDD STF mode
			 */
			tmp_cddpo = pi->ppr.sr8.cdd[band];
		}
		else {
			tmp_cddpo = 0;
		}

		tmp_mcs_word = (pi->ppr.sr8.mcs[band][1] << 16)|(pi->ppr.sr8.mcs[band][0]);
		wlc_phy_txpwr_srom_convert_mcs_offset(tmp_mcs_word, tmp_cddpo, tmp_max_pwr,
			&mcs20_offset_cdd, 0);

		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
			/* for mcs_20IN20 CDD: S1x1, S1x2  */
			wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_1,
				WL_TX_MODE_CDD, WL_TX_CHAINS_2, &mcs20_offset_cdd);
		}

		/* for 20MHz Mapping MCS0-7 CDD to Legacy OFDM CDD  */
		wlc_phy_copy_mcs_to_ofdm_powers(&mcs20_offset_cdd, &ofdm20_offset_cdd);

		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
			/* for ofdm_20IN20 CDD:  S1x2  */
			wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &ofdm20_offset_cdd);
		}

		/* STBC 20 MHz */
		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			/* Apply power-offset specified by the cddpo SROM field to rates sent
			 * in the CDD STF mode
			 */
			tmp_stbcpo = pi->ppr.sr8.stbc[band];
		}
		else {
			tmp_stbcpo = 0;
		}

		tmp_mcs_word = (pi->ppr.sr8.mcs[band][1] << 16)|(pi->ppr.sr8.mcs[band][0]);
		wlc_phy_txpwr_srom_convert_mcs_offset(tmp_mcs_word, tmp_stbcpo, tmp_max_pwr,
			&mcs20_offset_stbc, 0);

		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
			/* for mcs_20IN20 STBC: S2x2  */
			wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_2,
				WL_TX_MODE_STBC, WL_TX_CHAINS_2, &mcs20_offset_stbc);
		}

		/* SDM 20 MHz */
		tmp_mcs_word = (pi->ppr.sr8.mcs[band][3] << 16)|(pi->ppr.sr8.mcs[band][2]);
		wlc_phy_txpwr_srom_convert_mcs_offset(tmp_mcs_word, 0, tmp_max_pwr,
			&mcs20_offset_sdm, 0);

		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
			/* for mcs_20IN20 SDM: S2x2  */
			wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_2,
				WL_TX_MODE_NONE, WL_TX_CHAINS_2, &mcs20_offset_sdm);
		}
	}

	/* 40 MHz PPRs */
	/* For nphy_rev>=5, re-interpret the mcs[2g,5g,5gl,5gh]po4-7 SROM fields to be the
	 * power-offsets for 40 MHz mcs0-15 w.r.t the max power. For nphy_rev<5, 40 MHz
	 * mcs0-15, use the same power offsets as for 20 MHz mcs0-15.
	 * The bw402gpo field is further used to implement an additional uniform power
	 * back-off for all 40 MHz OFDM rates.
	 */
	if (CHSPEC_IS40(chanspec)) {
		if (NPHY_IS_SROM_REINTERPRET) {
			int8 mcs7_15_offset = 0;

			/* Hack for LCNXNPHY  rev 0 */
			/* 2 channels are looking 2 dB off with respect to evm and SM performance */
			/* Dropping the srom powers only for those 2 channels */
			if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV)) {
				uint channel = CHSPEC_CHANNEL(pi->radio_chanspec);
				if (channel == 151)  {
					mcs7_15_offset = 4;
				}
			}

			/* MCS 40 SISO */
			tmp_bw40po = pi->ppr.sr8.bw40[band];

			tmp_mcs_word = (pi->ppr.sr8.mcs[band][5] << 16)|(pi->ppr.sr8.mcs[band][4]);
			wlc_phy_txpwr_srom_convert_mcs_offset(tmp_mcs_word, tmp_bw40po, tmp_max_pwr,
				&mcs40_offset_siso, mcs7_15_offset);

			/* for mcs_40 SISO: S1x1  */
			wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_1,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs40_offset_siso);

			/* for mcs_20in40 SISO: S1x1  */
			wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40, WL_TX_NSS_1,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs40_offset_siso);

			/* for 40MHz Mapping MCS0-7 SISO to Legacy OFDM SISO	*/
			wlc_phy_copy_mcs_to_ofdm_powers(&mcs40_offset_siso, &ofdm40_offset_siso);

			/* for ofdm_40 SISO: S1x1, */
			wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_MODE_NONE,
				WL_TX_CHAINS_1, &ofdm40_offset_siso);

			/* for ofdm_20IN40 SISO: S1x1, */
			wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm40_offset_siso);

			/* MCS 40 CDD */
			tmp_cddpo = pi->ppr.sr8.cdd[band];

			tmp_mcs_word = (pi->ppr.sr8.mcs[band][5] << 16)|(pi->ppr.sr8.mcs[band][4]);
			wlc_phy_txpwr_srom_convert_mcs_offset(tmp_mcs_word,
				(tmp_bw40po + tmp_cddpo), tmp_max_pwr, &mcs40_offset_cdd,
				mcs7_15_offset);

			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				/* for mcs_40 CDD: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_2, &mcs40_offset_cdd);

				/* for mcs_20in40 CDD: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
					&mcs40_offset_cdd);
			}
			/* for 40MHz Mapping MCS0-7 CDD to Legacy OFDM CDD	*/
			wlc_phy_copy_mcs_to_ofdm_powers(&mcs40_offset_cdd, &ofdm40_offset_cdd);

			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				/* for ofdm_40 CDD: S1x2, */
				wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_40,
					WL_TX_MODE_CDD,	WL_TX_CHAINS_2, &ofdm40_offset_cdd);

				/* for ofdm_20IN40 CDD: S1x2, */
				wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_MODE_CDD, WL_TX_CHAINS_2, &ofdm40_offset_cdd);

				/* MCS 40 STBC */
				tmp_stbcpo = pi->ppr.sr8.stbc[band];

				tmp_mcs_word = (pi->ppr.sr8.mcs[band][5] << 16)|
					(pi->ppr.sr8.mcs[band][4]);
				wlc_phy_txpwr_srom_convert_mcs_offset(tmp_mcs_word,
					(tmp_bw40po + tmp_stbcpo), tmp_max_pwr, &mcs40_offset_stbc,
					mcs7_15_offset);

				/* for mcs_40 STBC: S2x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_2, &mcs40_offset_stbc);

				/* for mcs_20in40 STBC: S2x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_2,
					&mcs40_offset_stbc);

				/* MCS 40 SDM */
				tmp_mcs_word = (pi->ppr.sr8.mcs[band][7] << 16)|
					(pi->ppr.sr8.mcs[band][6]);
				wlc_phy_txpwr_srom_convert_mcs_offset(tmp_mcs_word, tmp_bw40po,
					tmp_max_pwr, &mcs40_offset_sdm, mcs7_15_offset);

				/* for mcs_40 SDM: S2x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_2, &mcs40_offset_sdm);

				/* for mcs_20in40 SDM: S2x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
					&mcs40_offset_sdm);
			}

		}
		else {
			/* For nphy_rev<5, set the powers of the 40MHz rates sames as the
			 * 20 MHz rates + bw40po
			 */

			tmp_bw40po = pi->ppr.sr8.bw40[band];
			wlc_phy_txpwr_srom8_sub_ofdm(&ofdm20_offset_siso, tmp_bw40po);
			wlc_phy_txpwr_srom8_sub_ofdm(&ofdm20_offset_cdd, tmp_bw40po);

			wlc_phy_txpwr_srom8_sub_mcs(&mcs20_offset_siso, tmp_bw40po);
			wlc_phy_txpwr_srom8_sub_mcs(&mcs20_offset_cdd, tmp_bw40po);
			wlc_phy_txpwr_srom8_sub_mcs(&mcs20_offset_stbc, tmp_bw40po);
			wlc_phy_txpwr_srom8_sub_mcs(&mcs20_offset_sdm, tmp_bw40po);

			/* for mcs_40 SISO: S1x1  */
			wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_1,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs20_offset_siso);

			/* for mcs_20in40 SISO: S1x1  */
			wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40, WL_TX_NSS_1,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs20_offset_siso);

			/* for ofdm_40 SISO: S1x1, */
			wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_MODE_NONE,
				WL_TX_CHAINS_1, &ofdm20_offset_siso);

			/* for ofdm_20IN40 SISO: S1x1, */
			wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm20_offset_siso);

			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				/* for mcs_40 CDD: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_2, &mcs20_offset_cdd);

				/* for mcs_20in40 CDD: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
					&mcs20_offset_cdd);

				/* for ofdm_40 CDD: S1x2, */
				wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_40,
					WL_TX_MODE_CDD,	WL_TX_CHAINS_2, &ofdm20_offset_cdd);

				/* for ofdm_20in40 CDD: S1x2, */
				wlc_phy_ppr_set_ofdm_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_MODE_CDD,	WL_TX_CHAINS_2, &ofdm20_offset_cdd);

				/* for mcs_40 STBC: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_2, &mcs20_offset_stbc);

				/* for mcs_20in40 STBC: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_2,
					&mcs20_offset_stbc);

				/* for mcs_40 SDM: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_2, &mcs20_offset_sdm);

				/* for mcs_20in40 SDM: S1x2  */
				wlc_phy_ppr_set_mcs_srom8(tx_srom_max_pwr, WL_TX_BW_20IN40,
					WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
					&mcs20_offset_sdm);
			}

		}
	}

}

#ifdef BAND5G
static int8
wlc_phy_txpwr40Moffset_srom_convert(uint8 offset)
{
	/* XXX
	 * This function checks to see if the 40MHz offset
	 * programmed into srom is
	 * in the correcr range and if the values are programmed
	 * right, it converts a 3 bit 2 complement
	 * number to decimal that is used to offset the estpower
	 * in the estpower look up table
	 */
	if (offset == 0xf)
		return 0;
	else if (offset > 0x7) {
		PHY_ERROR(("ILLEGAL 40MHZ PWRCTRL OFFSET VALUE, APPLYING 0 OFFSET \n"));
		return 0;
	} else {
		if (offset < 4)
			return offset;
		else
			return (-8+offset);
	}
}
#endif /* BAND5G */

static const char BCMATTACHDATA(rstr_bw40po)[] = "bw40po";
static const char BCMATTACHDATA(rstr_cddpo)[] = "cddpo";
static const char BCMATTACHDATA(rstr_stbcpo)[] = "stbcpo";
static const char BCMATTACHDATA(rstr_bwduppo)[] = "bwduppo";
static const char BCMATTACHDATA(rstr_txpid2ga0)[] = "txpid2ga0";
static const char BCMATTACHDATA(rstr_txpid2ga1)[] = "txpid2ga1";
static const char BCMATTACHDATA(rstr_maxp2ga0)[] = "maxp2ga0";
static const char BCMATTACHDATA(rstr_maxp2ga1)[] = "maxp2ga1";
static const char BCMATTACHDATA(rstr_maxp2ga2)[] = "maxp2ga2";
static const char BCMATTACHDATA(rstr_pa2gw0a2)[] = "pa2gw0a2";
static const char BCMATTACHDATA(rstr_pa2gw1a2)[] = "pa2gw1a2";
static const char BCMATTACHDATA(rstr_pa2gw2a2)[] = "pa2gw2a2";
static const char BCMATTACHDATA(rstr_maxp5gla2)[] = "maxp5gla2";
static const char BCMATTACHDATA(rstr_pa5glw0a2)[] = "pa5glw0a2";
static const char BCMATTACHDATA(rstr_pa5glw1a2)[] = "pa5glw1a2";
static const char BCMATTACHDATA(rstr_pa5glw2a2)[] = "pa5glw2a2";
static const char BCMATTACHDATA(rstr_maxp5ga2)[] = "maxp5ga2";
static const char BCMATTACHDATA(rstr_pa5gw0a2)[] = "pa5gw0a2";
static const char BCMATTACHDATA(rstr_pa5gw1a2)[] = "pa5gw1a2";
static const char BCMATTACHDATA(rstr_pa5gw2a2)[] = "pa5gw2a2";
static const char BCMATTACHDATA(rstr_maxp5gha2)[] = "maxp5gha2";
static const char BCMATTACHDATA(rstr_pa5ghw0a2)[] = "pa5ghw0a2";
static const char BCMATTACHDATA(rstr_pa5ghw1a2)[] = "pa5ghw1a2";
static const char BCMATTACHDATA(rstr_pa5ghw2a2)[] = "pa5ghw2a2";
static const char BCMATTACHDATA(rstr_pa2gw0a0)[] = "pa2gw0a0";
static const char BCMATTACHDATA(rstr_pa2gw0a1)[] = "pa2gw0a1";
static const char BCMATTACHDATA(rstr_pa2gw1a0)[] = "pa2gw1a0";
static const char BCMATTACHDATA(rstr_pa2gw1a1)[] = "pa2gw1a1";
static const char BCMATTACHDATA(rstr_pa2gw2a0)[] = "pa2gw2a0";
static const char BCMATTACHDATA(rstr_pa2gw2a1)[] = "pa2gw2a1";
static const char BCMATTACHDATA(rstr_itt2ga0)[] = "itt2ga0";
static const char BCMATTACHDATA(rstr_itt2ga1)[] = "itt2ga1";
static const char BCMATTACHDATA(rstr_cck2gpo)[] = "cck2gpo";
static const char BCMATTACHDATA(rstr_ofdm2gpo)[] = "ofdm2gpo";
static const char BCMATTACHDATA(rstr_mcs2gpo0)[] = "mcs2gpo0";
static const char BCMATTACHDATA(rstr_mcs2gpo1)[] = "mcs2gpo1";
static const char BCMATTACHDATA(rstr_mcs2gpo2)[] = "mcs2gpo2";
static const char BCMATTACHDATA(rstr_mcs2gpo3)[] = "mcs2gpo3";
static const char BCMATTACHDATA(rstr_mcs2gpo4)[] = "mcs2gpo4";
static const char BCMATTACHDATA(rstr_mcs2gpo5)[] = "mcs2gpo5";
static const char BCMATTACHDATA(rstr_mcs2gpo6)[] = "mcs2gpo6";
static const char BCMATTACHDATA(rstr_mcs2gpo7)[] = "mcs2gpo7";
static const char BCMATTACHDATA(rstr_txpid5gla0)[] = "txpid5gla0";
static const char BCMATTACHDATA(rstr_txpid5gla1)[] = "txpid5gla1";
static const char BCMATTACHDATA(rstr_maxp5gla0)[] = "maxp5gla0";
static const char BCMATTACHDATA(rstr_maxp5gla1)[] = "maxp5gla1";
static const char BCMATTACHDATA(rstr_pa5glw0a0)[] = "pa5glw0a0";
static const char BCMATTACHDATA(rstr_pa5glw0a1)[] = "pa5glw0a1";
static const char BCMATTACHDATA(rstr_pa5glw1a0)[] = "pa5glw1a0";
static const char BCMATTACHDATA(rstr_pa5glw1a1)[] = "pa5glw1a1";
static const char BCMATTACHDATA(rstr_pa5glw2a0)[] = "pa5glw2a0";
static const char BCMATTACHDATA(rstr_pa5glw2a1)[] = "pa5glw2a1";
static const char BCMATTACHDATA(rstr_ofdm5glpo)[] = "ofdm5glpo";
static const char BCMATTACHDATA(rstr_mcs5glpo0)[] = "mcs5glpo0";
static const char BCMATTACHDATA(rstr_mcs5glpo1)[] = "mcs5glpo1";
static const char BCMATTACHDATA(rstr_mcs5glpo2)[] = "mcs5glpo2";
static const char BCMATTACHDATA(rstr_mcs5glpo3)[] = "mcs5glpo3";
static const char BCMATTACHDATA(rstr_mcs5glpo4)[] = "mcs5glpo4";
static const char BCMATTACHDATA(rstr_mcs5glpo5)[] = "mcs5glpo5";
static const char BCMATTACHDATA(rstr_mcs5glpo6)[] = "mcs5glpo6";
static const char BCMATTACHDATA(rstr_mcs5glpo7)[] = "mcs5glpo7";
static const char BCMATTACHDATA(rstr_txpid5ga0)[] = "txpid5ga0";
static const char BCMATTACHDATA(rstr_txpid5ga1)[] = "txpid5ga1";
static const char BCMATTACHDATA(rstr_maxp5ga0)[] = "maxp5ga0";
static const char BCMATTACHDATA(rstr_maxp5ga1)[] = "maxp5ga1";
static const char BCMATTACHDATA(rstr_pa5gw0a0)[] = "pa5gw0a0";
static const char BCMATTACHDATA(rstr_pa5gw0a1)[] = "pa5gw0a1";
static const char BCMATTACHDATA(rstr_pa5gw1a0)[] = "pa5gw1a0";
static const char BCMATTACHDATA(rstr_pa5gw1a1)[] = "pa5gw1a1";
static const char BCMATTACHDATA(rstr_pa5gw2a0)[] = "pa5gw2a0";
static const char BCMATTACHDATA(rstr_pa5gw2a1)[] = "pa5gw2a1";
static const char BCMATTACHDATA(rstr_itt5ga0)[] = "itt5ga0";
static const char BCMATTACHDATA(rstr_itt5ga1)[] = "itt5ga1";
static const char BCMATTACHDATA(rstr_ofdm5gpo)[] = "ofdm5gpo";
static const char BCMATTACHDATA(rstr_mcs5gpo0)[] = "mcs5gpo0";
static const char BCMATTACHDATA(rstr_mcs5gpo1)[] = "mcs5gpo1";
static const char BCMATTACHDATA(rstr_mcs5gpo2)[] = "mcs5gpo2";
static const char BCMATTACHDATA(rstr_mcs5gpo3)[] = "mcs5gpo3";
static const char BCMATTACHDATA(rstr_mcs5gpo4)[] = "mcs5gpo4";
static const char BCMATTACHDATA(rstr_mcs5gpo5)[] = "mcs5gpo5";
static const char BCMATTACHDATA(rstr_mcs5gpo6)[] = "mcs5gpo6";
static const char BCMATTACHDATA(rstr_mcs5gpo7)[] = "mcs5gpo7";
static const char BCMATTACHDATA(rstr_txpid5gha0)[] = "txpid5gha0";
static const char BCMATTACHDATA(rstr_txpid5gha1)[] = "txpid5gha1";
static const char BCMATTACHDATA(rstr_maxp5gha0)[] = "maxp5gha0";
static const char BCMATTACHDATA(rstr_maxp5gha1)[] = "maxp5gha1";
static const char BCMATTACHDATA(rstr_pa5ghw0a0)[] = "pa5ghw0a0";
static const char BCMATTACHDATA(rstr_pa5ghw0a1)[] = "pa5ghw0a1";
static const char BCMATTACHDATA(rstr_pa5ghw1a0)[] = "pa5ghw1a0";
static const char BCMATTACHDATA(rstr_pa5ghw1a1)[] = "pa5ghw1a1";
static const char BCMATTACHDATA(rstr_pa5ghw2a0)[] = "pa5ghw2a0";
static const char BCMATTACHDATA(rstr_pa5ghw2a1)[] = "pa5ghw2a1";
static const char BCMATTACHDATA(rstr_ofdm5ghpo)[] = "ofdm5ghpo";
static const char BCMATTACHDATA(rstr_mcs5ghpo0)[] = "mcs5ghpo0";
static const char BCMATTACHDATA(rstr_mcs5ghpo1)[] = "mcs5ghpo1";
static const char BCMATTACHDATA(rstr_mcs5ghpo2)[] = "mcs5ghpo2";
static const char BCMATTACHDATA(rstr_mcs5ghpo3)[] = "mcs5ghpo3";
static const char BCMATTACHDATA(rstr_mcs5ghpo4)[] = "mcs5ghpo4";
static const char BCMATTACHDATA(rstr_mcs5ghpo5)[] = "mcs5ghpo5";
static const char BCMATTACHDATA(rstr_mcs5ghpo6)[] = "mcs5ghpo6";
static const char BCMATTACHDATA(rstr_mcs5ghpo7)[] = "mcs5ghpo7";

static void
BCMATTACHFN(wlc_phy_txpwr_srom8_read_ppr)(phy_info_t *pi)
{
		uint16 bw40po, cddpo, stbcpo, bwduppo;
		int band_num;
		phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
		srom_pwrdet_t	*pwrdet  = pi->pwrdet;

		/* Read bw40po value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		bw40po = (uint16)PHY_GETINTVAR(pi, rstr_bw40po);
		pi->ppr.sr8.bw40[WL_CHAN_FREQ_RANGE_2G] = bw40po & 0xf;
#ifdef BAND5G
		pi->ppr.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND0] = (bw40po & 0xf0) >> 4;
		pi->ppr.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND1] = (bw40po & 0xf00) >> 8;
		pi->ppr.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND2] = (bw40po & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND3] =
				(bw40po & 0xf000) >> 12;
#endif /* BAND5G */
		/* Read cddpo value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		cddpo = (uint16)PHY_GETINTVAR(pi, rstr_cddpo);
		pi->ppr.sr8.cdd[WL_CHAN_FREQ_RANGE_2G] = cddpo & 0xf;
#ifdef BAND5G
		pi->ppr.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND0]  = (cddpo & 0xf0) >> 4;
		pi->ppr.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND1]  = (cddpo & 0xf00) >> 8;
		pi->ppr.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND2]  = (cddpo & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND3]  = (cddpo & 0xf000) >> 12;
#endif /* BAND5G */

		/* Read stbcpo value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		stbcpo = (uint16)PHY_GETINTVAR(pi, rstr_stbcpo);
		pi->ppr.sr8.stbc[WL_CHAN_FREQ_RANGE_2G] = stbcpo & 0xf;
#ifdef BAND5G
		pi->ppr.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND0]  = (stbcpo & 0xf0) >> 4;
		pi->ppr.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND1]  = (stbcpo & 0xf00) >> 8;
		pi->ppr.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND2]  = (stbcpo & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND3]  = (stbcpo & 0xf000) >> 12;
#endif /* BAND5G */

		/* Read bwduppo value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		bwduppo = (uint16)PHY_GETINTVAR(pi, rstr_bwduppo);
		pi->ppr.sr8.bwdup[WL_CHAN_FREQ_RANGE_2G] = bwduppo & 0xf;
#ifdef BAND5G
		pi->ppr.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND0]  = (bwduppo & 0xf0) >> 4;
		pi->ppr.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND1]  = (bwduppo & 0xf00) >> 8;
		pi->ppr.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND2]  = (bwduppo & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND3]  = (bwduppo & 0xf000) >> 12;
#endif /* BAND5G */

		for (band_num = 0; band_num < NUMSUBBANDS(pi); band_num++) {
			switch (band_num) {
				case WL_CHAN_FREQ_RANGE_2G:
					/* 2G band */
					if (ISNPHY(pi)) {
						pi_nphy->nphy_txpid2g[PHY_CORE_0]  =
						(uint8)PHY_GETINTVAR(pi, rstr_txpid2ga0);
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
						idle_targ_2g =
						(int8)PHY_GETINTVAR(pi, rstr_itt2ga0);
					}
					pwrdet->max_pwr[PHY_CORE_0][band_num] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a0);

					if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
						if (ISNPHY(pi)) {
							pi_nphy->nphy_txpid2g[PHY_CORE_1]  =
							(uint8)PHY_GETINTVAR(pi, rstr_txpid2ga1);
							pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
							idle_targ_2g =
							(int8)PHY_GETINTVAR(pi, rstr_itt2ga1);
						}
						pwrdet->max_pwr[PHY_CORE_1][band_num] =
							(int8)PHY_GETINTVAR(pi, rstr_maxp2ga1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a1);
					}
					if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
						pwrdet->max_pwr[PHY_CORE_2][band_num] =
							(int8)PHY_GETINTVAR(pi, rstr_maxp2ga2);
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a2);
					}

					/* 2G CCK */
					pi->ppr.sr8.cck2gpo =
						(uint16)PHY_GETINTVAR(pi, rstr_cck2gpo);

					/* 2G ofdm2gpo power offsets */
					pi->ppr.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm2gpo);

					/* 2G mcs2gpo power offsets */
					pi->ppr.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo0);
					pi->ppr.sr8.mcs[band_num][1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo1);
					pi->ppr.sr8.mcs[band_num][2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo2);
					pi->ppr.sr8.mcs[band_num][3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo3);
					pi->ppr.sr8.mcs[band_num][4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo4);
					pi->ppr.sr8.mcs[band_num][5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo5);
					pi->ppr.sr8.mcs[band_num][6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo6);
					pi->ppr.sr8.mcs[band_num][7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo7);
					break;
#ifdef BAND5G
				case WL_CHAN_FREQ_RANGE_5G_BAND0:
					/* 5G lowband */
					if (ISNPHY(pi)) {
						pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num] =
						(uint8)PHY_GETINTVAR(pi, rstr_txpid5gla0);
					}
					pwrdet->max_pwr[PHY_CORE_0][band_num]  =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gla0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a0);

					if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
						if (ISNPHY(pi)) {
							pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
							= (uint8)PHY_GETINTVAR(pi, rstr_txpid5gla1);
						}
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a1);
						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gla1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a1);
					}
					if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a2);
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gla2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a2);
					}

					if (ISNPHY(pi)) {
						pi_nphy->nphy_pwrctrl_info[0].
						idle_targ_5g[band_num] = 0;
						pi_nphy->nphy_pwrctrl_info[1].
						idle_targ_5g[band_num] = 0;
					}

					/* 5G lowband ofdm5glpo power offsets */
					pi->ppr.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5glpo);

					/* 5G lowband mcs5glpo power offsets */
					pi->ppr.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo0);
					pi->ppr.sr8.mcs[band_num] [1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo1);
					pi->ppr.sr8.mcs[band_num] [2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo2);
					pi->ppr.sr8.mcs[band_num] [3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo3);
					pi->ppr.sr8.mcs[band_num] [4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo4);
					pi->ppr.sr8.mcs[band_num] [5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo5);
					pi->ppr.sr8.mcs[band_num] [6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo6);
					pi->ppr.sr8.mcs[band_num] [7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo7);
					break;
				case WL_CHAN_FREQ_RANGE_5G_BAND1:
					/* 5G band, mid */
					if (ISNPHY(pi)) {
						pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num]  =
						(uint8)PHY_GETINTVAR(pi, rstr_txpid5ga0);
					}
					pwrdet->max_pwr[PHY_CORE_0][band_num] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a0);

					if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
						if (ISNPHY(pi)) {
							pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
							= (uint8)PHY_GETINTVAR(pi, rstr_txpid5ga1);
						}
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a1);
						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5ga1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a1);
					}
					if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a2);
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5ga2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a2);
					}

					if (ISNPHY(pi)) {
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
						idle_targ_5g[band_num] =
						(int8)PHY_GETINTVAR(pi, rstr_itt5ga0);
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
						idle_targ_5g[band_num] =
						(int8)PHY_GETINTVAR(pi, rstr_itt5ga1);
					}

					/* 5G midband ofdm5gpo power offsets */
					pi->ppr.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5gpo);

					/* 5G midband mcs5gpo power offsets */
					pi->ppr.sr8.mcs[band_num] [0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo0);
					pi->ppr.sr8.mcs[band_num] [1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo1);
					pi->ppr.sr8.mcs[band_num] [2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo2);
					pi->ppr.sr8.mcs[band_num] [3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo3);
					pi->ppr.sr8.mcs[band_num] [4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo4);
					pi->ppr.sr8.mcs[band_num] [5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo5);
					pi->ppr.sr8.mcs[band_num] [6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo6);
					pi->ppr.sr8.mcs[band_num] [7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo7);
					break;
				case WL_CHAN_FREQ_RANGE_5G_BAND2:
					/* 5G highband */
					if (ISNPHY(pi)) {
						pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num] =
						(uint8)PHY_GETINTVAR(pi, rstr_txpid5gha0);
					}
					pwrdet->max_pwr[PHY_CORE_0][band_num]  =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);

					if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
						if (ISNPHY(pi)) {
							pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
							= (uint8)PHY_GETINTVAR(pi, rstr_txpid5gha1);
						}
						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
					}
					if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
					}

					if (ISNPHY(pi)) {
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
						idle_targ_5g[band_num] = 0;
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
						idle_targ_5g[band_num] = 0;
					}

					/* 5G highband ofdm5ghpo power offsets */
					pi->ppr.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5ghpo);

					/* 5G highband mcs5ghpo power offsets */
					pi->ppr.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo0);
					pi->ppr.sr8.mcs[band_num][1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo1);
					pi->ppr.sr8.mcs[band_num][2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo2);
					pi->ppr.sr8.mcs[band_num][3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo3);
					pi->ppr.sr8.mcs[band_num][4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo4);
					pi->ppr.sr8.mcs[band_num][5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo5);
					pi->ppr.sr8.mcs[band_num][6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo6);
					pi->ppr.sr8.mcs[band_num][7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo7);
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND3:
					/* 5G highband */
					if (ISNPHY(pi)) {
						pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num] =
						(uint8)PHY_GETINTVAR(pi, rstr_txpid5gha0);
					}
					pwrdet->max_pwr[PHY_CORE_0][band_num]  =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);

					if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
						if (ISNPHY(pi)) {
							pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
							= (uint8)PHY_GETINTVAR(pi, rstr_txpid5gha1);
						}
						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
					}
					if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
					}

					if (ISNPHY(pi)) {
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
						idle_targ_5g[band_num] = 0;
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
						idle_targ_5g[band_num] = 0;
					}

					/* 5G highband ofdm5ghpo power offsets */
					pi->ppr.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5ghpo);

					/* 5G highband mcs5ghpo power offsets */
					pi->ppr.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo0);
					pi->ppr.sr8.mcs[band_num][1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo1);
					pi->ppr.sr8.mcs[band_num][2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo2);
					pi->ppr.sr8.mcs[band_num][3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo3);
					pi->ppr.sr8.mcs[band_num][4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo4);
					pi->ppr.sr8.mcs[band_num][5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo5);
					pi->ppr.sr8.mcs[band_num][6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo6);
					pi->ppr.sr8.mcs[band_num][7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo7);
					break;
#endif /* BAND5G */
				}
			}

		/* Finished reading from SROM, calculate and apply powers */
}

static void
BCMATTACHFN(wlc_phy_txpwr_srom9_read_ppr)(phy_info_t *pi)
{

	/* Read and interpret the power-offset parameters from the SROM for each band/subband */
	if (pi->sh->sromrev >= 9) {
		int i, j;

		PHY_INFORM(("Get SROM 9 Power Offset per rate\n"));
		/* 2G CCK */
		pi->ppr.sr9.cckbw202gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
		pi->ppr.sr9.cckbw20ul2gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw20ul2gpo);
		/* 2G OFDM power offsets */
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw202gpo);
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul2gpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul2gpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);

#ifdef BAND5G
		/* 5G power offsets */
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205glpo);
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5glpo);
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gmpo);
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gmpo);
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205ghpo);
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5ghpo);

		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5glpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND0].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gmpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND1].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5ghpo);
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND2].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
		{
			pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205ghpo);
			pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5ghpo);

			pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);
			pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5ghpo);
			pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND3].bw40 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);
		}
#endif /* BAND5G */

		/* 40 Dups */
		pi->ppr.sr9.ofdm40duppo = (uint16)PHY_GETINTVAR(pi, rstr_legofdm40duppo);
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw40 =
			pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul;
		for (i = 0; i < NUMSUBBANDS(pi); i++) {
			uint32 nibble, dup40_offset = 0;
			nibble = pi->ppr.sr9.ofdm40duppo & 0xf;
			for (j = 0; j < WL_NUM_RATES_OFDM; j++) {
				dup40_offset |= nibble;
				nibble <<= 4;
			}
			if (i == 0)
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw40 =
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul +
				dup40_offset;
#ifdef BAND5G
			else if (i == 1)
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw40 =
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20ul +
				dup40_offset;
			else if (i == 2)
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw40 =
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20ul +
				dup40_offset;
			else if (i == 3)
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw40 =
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20ul +
				dup40_offset;
			else if (i == 4)
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw40 =
				pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20ul +
				dup40_offset;

#endif /* BAND5G */
		}
	}

	PHY_INFORM(("CCK: 0x%04x 0x%04x\n", pi->ppr.sr9.cckbw202gpo, pi->ppr.sr9.cckbw202gpo));
	PHY_INFORM(("OFDM: 0x%08x 0x%08x 0x%02x\n",
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20,
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul,
		pi->ppr.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw40));
	PHY_INFORM(("MCS: 0x%08x 0x%08x 0x%08x\n",
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20,
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20ul,
		pi->ppr.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw40));
}

static void
BCMATTACHFN(wlc_phy_txpwr_srom11_read_ppr)(phy_info_t *pi)
{

	/* Read and interpret the power-offset parameters from the SROM for each band/subband */
	if (pi->sh->sromrev >= 11) {
		uint16 _tmp;
		uint8 nibble, nibble01;

		PHY_INFORM(("Get SROM 11 Power Offset per rate\n"));
		/* --------------2G------------------- */
		/* 2G CCK */
		pi->ppr.sr11.cck.bw20 			=
		                (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
		pi->ppr.sr11.cck.bw20in40 		=
		                (uint16)PHY_GETINTVAR(pi, rstr_cckbw20ul2gpo);

		pi->ppr.sr11.offset_2g			=
		                (uint16)PHY_GETINTVAR(pi, rstr_ofdmlrbw202gpo);
		/* 2G OFDM_20 */
		_tmp 		= (uint16)PHY_GETINTVAR(pi, rstr_dot11agofdmhrbw202gpo);
		nibble 		= pi->ppr.sr11.offset_2g & 0xf;
		nibble01 	= (nibble<<4)|nibble;
		nibble 		= (pi->ppr.sr11.offset_2g>>4)& 0xf;
		pi->ppr.sr11.ofdm_2g.bw20 		=
		                (((nibble<<8)|(nibble<<12))|(nibble01))&0xffff;
		pi->ppr.sr11.ofdm_2g.bw20 		|=
		                (_tmp << 16);
		/* 2G MCS_20 */
		pi->ppr.sr11.mcs_2g.bw20 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
		/* 2G MCS_40 */
		pi->ppr.sr11.mcs_2g.bw40 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);

		pi->ppr.sr11.offset_20in40_l 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in40lrpo);
		pi->ppr.sr11.offset_20in40_h 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in40hrpo);

		pi->ppr.sr11.offset_dup_h 		=
		                (uint16)PHY_GETINTVAR(pi, rstr_dot11agduphrpo);
		pi->ppr.sr11.offset_dup_l 		=
		                (uint16)PHY_GETINTVAR(pi, rstr_dot11agduplrpo);

#ifdef BAND5G
		/* ---------------5G--------------- */
		/* 5G 11agnac_20IN20 */
		pi->ppr.sr11.ofdm_5g.bw20[0] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
		pi->ppr.sr11.ofdm_5g.bw20[1] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
		pi->ppr.sr11.ofdm_5g.bw20[2] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);

		pi->ppr.sr11.offset_5g[0]			=
		                (uint16)PHY_GETINTVAR(pi, rstr_mcslr5glpo);
		pi->ppr.sr11.offset_5g[1] 			=
		                (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gmpo);
		pi->ppr.sr11.offset_5g[2] 			=
		                (uint16)PHY_GETINTVAR(pi, rstr_mcslr5ghpo);

		/* 5G 11nac 40IN40 */
		pi->ppr.sr11.mcs_5g.bw40[0] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
		pi->ppr.sr11.mcs_5g.bw40[1] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
		pi->ppr.sr11.mcs_5g.bw40[2] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);

		/* 5G 11nac 80IN80 */
		pi->ppr.sr11.mcs_5g.bw80[0] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805glpo);
		pi->ppr.sr11.mcs_5g.bw80[1] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gmpo);
		pi->ppr.sr11.mcs_5g.bw80[2] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805ghpo);
		pi->ppr.sr11.mcs_5g.bw80[3] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gx1po);
		pi->ppr.sr11.mcs_5g.bw80[4] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gx2po);

		/* 5G 11agnac_20Ul/20LU */
		pi->ppr.sr11.offset_20in80_l[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5glpo);
		pi->ppr.sr11.offset_20in80_h[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5glpo);
		pi->ppr.sr11.offset_20in80_l[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gmpo);
		pi->ppr.sr11.offset_20in80_h[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gmpo);
		pi->ppr.sr11.offset_20in80_l[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5ghpo);
		pi->ppr.sr11.offset_20in80_h[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5ghpo);

		/* 5G 11nac_40IN80 */
		pi->ppr.sr11.offset_40in80_l[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5glpo);
		pi->ppr.sr11.offset_40in80_h[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5glpo);
		pi->ppr.sr11.offset_40in80_l[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gmpo);
		pi->ppr.sr11.offset_40in80_h[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gmpo);
		pi->ppr.sr11.offset_40in80_l[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5ghpo);
		pi->ppr.sr11.offset_40in80_h[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5ghpo);

#endif /* BAND5G */
		if (0) {
			/* printf srom value for verification */
			PHY_ERROR(("		cckbw202gpo=%x\n", pi->ppr.sr11.cck.bw20));
			PHY_ERROR(("		cckbw20ul2gpo=%x\n", pi->ppr.sr11.cck.bw20in40));
			PHY_ERROR(("		ofdmlrbw202gpo=%x\n", pi->ppr.sr11.offset_2g));
			PHY_ERROR(("		dot11agofdmhrbw202gpo=%x\n", _tmp));
			PHY_ERROR(("		mcsbw202gpo=%x\n", pi->ppr.sr11.mcs_2g.bw20));
			PHY_ERROR(("		mcsbw402gpo=%x\n", pi->ppr.sr11.mcs_2g.bw40));
			PHY_ERROR(("		sb20in40lrpo=%x\n", pi->ppr.sr11.offset_20in40_l));
			PHY_ERROR(("		sb20in40hrpo=%x\n", pi->ppr.sr11.offset_20in40_h));
			PHY_ERROR(("		dot11agduphrpo=%x\n", pi->ppr.sr11.offset_dup_h));
			PHY_ERROR(("		dot11agduplrpo=%x\n", pi->ppr.sr11.offset_dup_l));
			PHY_ERROR(("		mcsbw205glpo=%x\n", pi->ppr.sr11.ofdm_5g.bw20[0]));
			PHY_ERROR(("		mcsbw205gmpo=%x\n", pi->ppr.sr11.ofdm_5g.bw20[1]));
			PHY_ERROR(("		mcsbw205ghpo=%x\n", pi->ppr.sr11.ofdm_5g.bw20[2]));
			PHY_ERROR(("		mcslr5glpo=%x\n", pi->ppr.sr11.offset_5g[0]));
			PHY_ERROR(("		mcslr5gmpo=%x\n", pi->ppr.sr11.offset_5g[1]));
			PHY_ERROR(("		mcslr5ghpo=%x\n", pi->ppr.sr11.offset_5g[2]));
			PHY_ERROR(("		mcsbw405glpo=%x\n", pi->ppr.sr11.mcs_5g.bw40[0]));
			PHY_ERROR(("		mcsbw405gmpo=%x\n", pi->ppr.sr11.mcs_5g.bw40[1]));
			PHY_ERROR(("		mcsbw405ghpo=%x\n", pi->ppr.sr11.mcs_5g.bw40[2]));
			PHY_ERROR(("		mcsbw805glpo=%x\n", pi->ppr.sr11.mcs_5g.bw80[0]));
			PHY_ERROR(("		mcsbw805gmpo=%x\n", pi->ppr.sr11.mcs_5g.bw80[1]));
			PHY_ERROR(("		mcsbw805ghpo=%x\n", pi->ppr.sr11.mcs_5g.bw80[2]));
			PHY_ERROR(("		sb20in80and160lr5glpo=%x\n",
			                    pi->ppr.sr11.offset_20in80_l[0]));
			PHY_ERROR(("		sb20in80and160hr5glpo=%x\n",
			                    pi->ppr.sr11.offset_20in80_h[0]));
			PHY_ERROR(("		sb20in80and160lr5gmpo=%x\n",
			                    pi->ppr.sr11.offset_20in80_l[1]));
			PHY_ERROR(("		sb20in80and160hr5gmpo=%x\n",
			                    pi->ppr.sr11.offset_20in80_h[1]));
			PHY_ERROR(("		sb20in80and160lr5ghpo=%x\n",
			                    pi->ppr.sr11.offset_20in80_l[2]));
			PHY_ERROR(("		sb20in80and160hr5ghpo=%x\n",
			                    pi->ppr.sr11.offset_20in80_h[2]));
			PHY_ERROR(("		sb40and80lr5glpo=%x\n",
			                    pi->ppr.sr11.offset_40in80_l[0]));
			PHY_ERROR(("		sb40and80hr5glpo=%x\n",
			                    pi->ppr.sr11.offset_40in80_h[0]));
			PHY_ERROR(("		sb40and80lr5gmpo=%x\n",
			                    pi->ppr.sr11.offset_40in80_l[1]));
			PHY_ERROR(("		sb40and80hr5gmpo=%x\n",
			                    pi->ppr.sr11.offset_40in80_h[1]));
			PHY_ERROR(("		sb40and80lr5ghpo=%x\n",
			                    pi->ppr.sr11.offset_40in80_l[2]));
			PHY_ERROR(("		sb40and80hr5ghpo=%x\n",
			                    pi->ppr.sr11.offset_40in80_h[2]));
		}
	}
}

#ifdef WL11AC
static void
BCMATTACHFN(wlc_phy_txpwr_srom13_read_ppr)(phy_info_t *pi)
{
	uint8 nibble, nibble01;

	if (!(SROMREV(pi->sh->sromrev) < 13)) {
	    /* Read and interpret the power-offset parameters from the SROM for each band/subband */
	    ASSERT(pi->sh->sromrev >= 13);

	    PHY_INFORM(("Get SROM 13 Power Offset per rate\n"));
	    /* --------------2G------------------- */
	    /* 2G CCK */
	    pi->ppr.sr13.cck.bw20 = (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
	    pi->ppr.sr13.cck.bw20in40 = (uint16)PHY_GETINTVAR(pi, rstr_cckbw20ul2gpo);

	    pi->ppr.sr13.offset_2g = (uint16)PHY_GETINTVAR(pi, rstr_ofdmlrbw202gpo);
	    /* 2G OFDM_20 */
	    nibble      = pi->ppr.sr13.offset_2g & 0xf;
	    nibble01    = (nibble<<4)|nibble;
	    nibble      = (pi->ppr.sr13.offset_2g>>4)& 0xf;
	    pi->ppr.sr13.ofdm_2g.bw20 = (((nibble<<8)|(nibble<<12))|(nibble01))&0xffff;
		pi->ppr.sr13.ofdm_2g.bw20 |=
				(((uint16)PHY_GETINTVAR(pi, rstr_dot11agofdmhrbw202gpo)) << 16);
	    /* 2G MCS_20 */
	    pi->ppr.sr13.mcs_2g.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
	    /* 2G MCS_40 */
	    pi->ppr.sr13.mcs_2g.bw40 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);

	    pi->ppr.sr13.offset_20in40_l = (uint16)PHY_GETINTVAR(pi, rstr_sb20in40lrpo);
	    pi->ppr.sr13.offset_20in40_h = (uint16)PHY_GETINTVAR(pi, rstr_sb20in40hrpo);

	    pi->ppr.sr13.offset_dup_h = (uint16)PHY_GETINTVAR(pi, rstr_dot11agduphrpo);
	    pi->ppr.sr13.offset_dup_l = (uint16)PHY_GETINTVAR(pi, rstr_dot11agduplrpo);

	    pi->ppr.sr13.pp1024qam2g = (uint16)PHY_GETINTVAR(pi, rstr_mcs1024qam2gpo);

#ifdef BAND5G
	    /* ---------------5G--------------- */
	    /* 5G 11agnac_20IN20 */
	    pi->ppr.sr13.ofdm_5g.bw20[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
	    pi->ppr.sr13.ofdm_5g.bw20[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
	    pi->ppr.sr13.ofdm_5g.bw20[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);
	    pi->ppr.sr13.ofdm_5g.bw20[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gx1po);
	    pi->ppr.sr13.ofdm_5g.bw20[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gx2po);

	    pi->ppr.sr13.offset_5g[0]	= (uint16)PHY_GETINTVAR(pi, rstr_mcslr5glpo);
	    pi->ppr.sr13.offset_5g[1] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gmpo);
	    pi->ppr.sr13.offset_5g[2] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5ghpo);
	    pi->ppr.sr13.offset_5g[3] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gx1po);
	    pi->ppr.sr13.offset_5g[4] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gx2po);

	    /* 5G 11nac 40IN40 */
	    pi->ppr.sr13.mcs_5g.bw40[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
	    pi->ppr.sr13.mcs_5g.bw40[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
	    pi->ppr.sr13.mcs_5g.bw40[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);
	    pi->ppr.sr13.mcs_5g.bw40[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gx1po);
	    pi->ppr.sr13.mcs_5g.bw40[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gx2po);

	    /* 5G 11nac 80IN80 */
	    pi->ppr.sr13.mcs_5g.bw80[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805glpo);
	    pi->ppr.sr13.mcs_5g.bw80[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gmpo);
	    pi->ppr.sr13.mcs_5g.bw80[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805ghpo);
	    pi->ppr.sr13.mcs_5g.bw80[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gx1po);
	    pi->ppr.sr13.mcs_5g.bw80[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gx2po);

	    pi->ppr.sr13.offset_20in80_l[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5glpo);
	    pi->ppr.sr13.offset_20in80_h[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5glpo);
	    pi->ppr.sr13.offset_20in80_l[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gmpo);
	    pi->ppr.sr13.offset_20in80_h[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gmpo);
	    pi->ppr.sr13.offset_20in80_l[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5ghpo);
	    pi->ppr.sr13.offset_20in80_h[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5ghpo);
	    pi->ppr.sr13.offset_20in80_l[3] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gx1po);
	    pi->ppr.sr13.offset_20in80_h[3] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gx1po);
	    pi->ppr.sr13.offset_20in80_l[4] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gx2po);
	    pi->ppr.sr13.offset_20in80_h[4] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gx2po);

	    pi->ppr.sr13.offset_40in80_l[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5glpo);
	    pi->ppr.sr13.offset_40in80_h[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5glpo);
	    pi->ppr.sr13.offset_40in80_l[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gmpo);
	    pi->ppr.sr13.offset_40in80_h[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gmpo);
	    pi->ppr.sr13.offset_40in80_l[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5ghpo);
	    pi->ppr.sr13.offset_40in80_h[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5ghpo);
	    pi->ppr.sr13.offset_40in80_l[3] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gx1po);
	    pi->ppr.sr13.offset_40in80_h[3] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gx1po);
	    pi->ppr.sr13.offset_40in80_l[4] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gx2po);
	    pi->ppr.sr13.offset_40in80_h[4] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gx2po);

		/* 5G 11nac 160IN160 */
		pi->pprsr13_mcs_5g_bw160[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw1605glpo);
		pi->pprsr13_mcs_5g_bw160[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw1605gmpo);
		pi->pprsr13_mcs_5g_bw160[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw1605ghpo);
		pi->pprsr13_mcs_5g_bw160[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw1605gx1po);
		pi->pprsr13_mcs_5g_bw160[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw1605gx2po);

	    /* extension fields in SROM 13 */
	    pi->ppr.sr13.pp1024qam5g[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5glpo);
	    pi->ppr.sr13.pp1024qam5g[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5gmpo);
	    pi->ppr.sr13.pp1024qam5g[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5ghpo);
	    pi->ppr.sr13.pp1024qam5g[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5gx1po);
	    pi->ppr.sr13.pp1024qam5g[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5gx2po);

	    pi->ppr.sr13.ppmcsexp[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcs8poexp);
	    pi->ppr.sr13.ppmcsexp[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcs9poexp);
	    pi->ppr.sr13.ppmcsexp[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcs10poexp);
	    pi->ppr.sr13.ppmcsexp[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcs11poexp);

#endif /* BAND5G */
	}
}
#endif /* WL11AC */

static void
BCMATTACHFN(wlc_phy_txpwr_srom12_read_ppr)(phy_info_t *pi)
{
	if (!(SROMREV(pi->sh->sromrev) < 12)) {
	    /* Read and interpret the power-offset parameters from the SROM for each band/subband */
	    uint8 nibble, nibble01;
	    ASSERT(pi->sh->sromrev >= 12);

	    PHY_INFORM(("Get SROM 12 Power Offset per rate\n"));
	    /* --------------2G------------------- */
	    /* 2G CCK */
	    pi->ppr.sr11.cck.bw20 = (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
	    pi->ppr.sr11.cck.bw20in40 = (uint16)PHY_GETINTVAR(pi, rstr_cckbw20ul2gpo);

	    pi->ppr.sr11.offset_2g = (uint16)PHY_GETINTVAR(pi, rstr_ofdmlrbw202gpo);
	    /* 2G OFDM_20 */
	    nibble 		= pi->ppr.sr11.offset_2g & 0xf;
	    nibble01 	= (nibble<<4)|nibble;
	    nibble 		= (pi->ppr.sr11.offset_2g>>4)& 0xf;
	    pi->ppr.sr11.ofdm_2g.bw20 = (((nibble<<8)|(nibble<<12))|(nibble01))&0xffff;
	    pi->ppr.sr11.ofdm_2g.bw20 		|=
	     (((uint16)PHY_GETINTVAR(pi, rstr_dot11agofdmhrbw202gpo)) << 16);
	    /* 2G MCS_20 */
	    pi->ppr.sr11.mcs_2g.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
	    /* 2G MCS_40 */
	    pi->ppr.sr11.mcs_2g.bw40 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);

	    pi->ppr.sr11.offset_20in40_l = (uint16)PHY_GETINTVAR(pi, rstr_sb20in40lrpo);
	    pi->ppr.sr11.offset_20in40_h = (uint16)PHY_GETINTVAR(pi, rstr_sb20in40hrpo);

	    pi->ppr.sr11.offset_dup_h = (uint16)PHY_GETINTVAR(pi, rstr_dot11agduphrpo);
	    pi->ppr.sr11.offset_dup_l = (uint16)PHY_GETINTVAR(pi, rstr_dot11agduplrpo);

#ifdef BAND5G
	    /* ---------------5G--------------- */
	    /* 5G 11agnac_20IN20 */
	    pi->ppr.sr11.ofdm_5g.bw20[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
	    pi->ppr.sr11.ofdm_5g.bw20[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
	    pi->ppr.sr11.ofdm_5g.bw20[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);
	    pi->ppr.sr11.ofdm_5g.bw20[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gx1po);
	    pi->ppr.sr11.ofdm_5g.bw20[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gx2po);

	    pi->ppr.sr11.offset_5g[0]	= (uint16)PHY_GETINTVAR(pi, rstr_mcslr5glpo);
	    pi->ppr.sr11.offset_5g[1] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gmpo);
	    pi->ppr.sr11.offset_5g[2] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5ghpo);
	    pi->ppr.sr11.offset_5g[3] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gx1po);
	    pi->ppr.sr11.offset_5g[4] = (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gx2po);

	    /* 5G 11nac 40IN40 */
	    pi->ppr.sr11.mcs_5g.bw40[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
	    pi->ppr.sr11.mcs_5g.bw40[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
	    pi->ppr.sr11.mcs_5g.bw40[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);
	    pi->ppr.sr11.mcs_5g.bw40[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gx1po);
	    pi->ppr.sr11.mcs_5g.bw40[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gx2po);

	    /* 5G 11nac 80IN80 */
	    pi->ppr.sr11.mcs_5g.bw80[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805glpo);
	    pi->ppr.sr11.mcs_5g.bw80[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gmpo);
	    pi->ppr.sr11.mcs_5g.bw80[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805ghpo);
	    pi->ppr.sr11.mcs_5g.bw80[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gx1po);
	    pi->ppr.sr11.mcs_5g.bw80[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gx2po);

	    pi->ppr.sr11.offset_20in80_l[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5glpo);
	    pi->ppr.sr11.offset_20in80_h[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5glpo);
	    pi->ppr.sr11.offset_20in80_l[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gmpo);
	    pi->ppr.sr11.offset_20in80_h[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gmpo);
	    pi->ppr.sr11.offset_20in80_l[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5ghpo);
	    pi->ppr.sr11.offset_20in80_h[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5ghpo);
	    pi->ppr.sr11.offset_20in80_l[3] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gx1po);
	    pi->ppr.sr11.offset_20in80_h[3] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gx1po);
	    pi->ppr.sr11.offset_20in80_l[4] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gx2po);
	    pi->ppr.sr11.offset_20in80_h[4] =
	     (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gx2po);

	    pi->ppr.sr11.offset_40in80_l[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5glpo);
	    pi->ppr.sr11.offset_40in80_h[0] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5glpo);
	    pi->ppr.sr11.offset_40in80_l[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gmpo);
	    pi->ppr.sr11.offset_40in80_h[1] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gmpo);
	    pi->ppr.sr11.offset_40in80_l[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5ghpo);
	    pi->ppr.sr11.offset_40in80_h[2] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5ghpo);
	    pi->ppr.sr11.offset_40in80_l[3] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gx1po);
	    pi->ppr.sr11.offset_40in80_h[3] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gx1po);
	    pi->ppr.sr11.offset_40in80_l[4] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gx2po);
	    pi->ppr.sr11.offset_40in80_h[4] = (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gx2po);

#endif /* BAND5G */
	}
}

static void
wlc_phy_txpwr_srom9_convert(phy_info_t *pi, int8 *srom_max,
                                            uint32 pwr_offset, uint8 tmp_max_pwr, uint8 rate_cnt)
{
	uint8 rate;
	uint8 nibble;

	if (pi->sh->sromrev < 9) {
		ASSERT(0 && "SROMREV < 9");
		return;
	}

	for (rate = 0; rate < rate_cnt; rate++) {
		nibble = (uint8)(pwr_offset & 0xf);
		pwr_offset >>= 4;
		/* nibble info indicates offset in 0.5dB units convert to 0.25dB */
		srom_max[rate] = (int8)(tmp_max_pwr - (nibble << 1));
		/* XXX
		  printf("****** srom_max[rate] = %d nibble = %d \n", srom_max[rate], nibble << 1);
		*/
	}
}

void
wlc_phy_txpwr_apply_srom9(phy_info_t *pi, uint8 band_num, chanspec_t chanspec,
	uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr)
{
	srom_pwrdet_t *pwrdet  = pi->pwrdet;
	ppr_dsss_rateset_t cck20_offset_ppr_api, cck20in40_offset_ppr_ppr_api;

	ppr_ofdm_rateset_t ofdm20_offset_ppr_api;
	ppr_ofdm_rateset_t ofdm20in40_offset_ppr_api;
	ppr_ofdm_rateset_t ofdmdup40_offset_ppr_api;

	ppr_ht_mcs_rateset_t mcs20_offset_ppr_api;
	ppr_ht_mcs_rateset_t mcs40_offset_ppr_api;
	ppr_ht_mcs_rateset_t mcs20in40_offset_ppr_api;

	ASSERT(tx_srom_max_pwr);

	tmp_max_pwr = pwrdet->max_pwr[0][band_num];

	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[1][band_num]);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[2][band_num]);

	switch (band_num) {
	case WL_CHAN_FREQ_RANGE_2G:
		/* XXX
		   printf("wlc_phy_txpwr_apply_srom9:
		   pi->ppr.sr9.cckbw202gpo = %d ************ \n",pi->ppr.sr9.cckbw202gpo);
		   printf("wlc_phy_txpwr_apply_srom9: pi->ppr.sr9.cckbw20ul2gpo
		   = %d*************** \n",pi->ppr.sr9.cckbw20ul2gpo);
		*/
		if (CHSPEC_IS20(chanspec)) {
			wlc_phy_txpwr_srom9_convert(pi, cck20_offset_ppr_api.pwr,
			                            pi->ppr.sr9.cckbw202gpo, tmp_max_pwr,
			                            WL_RATESET_SZ_DSSS);
			/* populating tx_srom_max_pwr = pi->tx_srom_max_pwr[band]
			   structure
			*/
			/* for 2g_dsss_20IN20: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20,
			                     &cck20_offset_ppr_api, pi);
		}
		else if (CHSPEC_IS40(chanspec)) {
			wlc_phy_txpwr_srom9_convert(pi, cck20in40_offset_ppr_ppr_api.pwr,
			                            pi->ppr.sr9.cckbw20ul2gpo, tmp_max_pwr,
			                            WL_RATESET_SZ_DSSS);
			/* for 2g_dsss_20IN40: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &cck20in40_offset_ppr_ppr_api, pi);
		}
		/* Fall through to set OFDM and .11n rates for 2.4GHz band */
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		/* OFDM srom conversion */
		/* ofdm_20IN20: S1x1, S1x2, S1x3 */
		/*  pwr_offsets = pi->ppr.sr9.ofdm[band_num].bw20; */
		/* XXX
		   printf("wlc_phy_txpwr_apply_srom9: pi->ppr.sr9.ofdm[band_num].bw20 =
		   %d ************ \n",pi->ppr.sr9.ofdm[band_num].bw20);
		   printf("wlc_phy_txpwr_apply_srom9:  pi->ppr.sr9.ofdm[band_num].bw20ul =
		   %d*************** \n", pi->ppr.sr9.ofdm[band_num].bw20ul);
		   printf("wlc_phy_txpwr_apply_srom9:  pi->ppr.sr9.ofdm[band_num].bw40 =
		   %d*************** \n", pi->ppr.sr9.ofdm[band_num].bw40);
		*/
		if (CHSPEC_IS20(chanspec)) {
			wlc_phy_txpwr_srom9_convert(pi, ofdm20_offset_ppr_api.pwr,
			                            pi->ppr.sr9.ofdm[band_num].bw20,
			                            tmp_max_pwr, WL_RATESET_SZ_OFDM);
			/* ofdm_20IN20: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, &ofdm20_offset_ppr_api,
				pi);
			/* HT srom conversion  */
			/*  20MHz HT */
			/* rate_cnt = WL_RATESET_SZ_HT_MCS; */
			/* pwr_offsets = pi->ppr.sr9.mcs[band_num].bw20; */
			wlc_phy_txpwr_srom9_convert(pi, mcs20_offset_ppr_api.pwr,
			                            pi->ppr.sr9.mcs[band_num].bw20,
			                            tmp_max_pwr, WL_RATESET_SZ_HT_MCS);
			/* 20MHz HT  */
			wlc_phy_ppr_set_ht_mcs(tx_srom_max_pwr,
			                       WL_TX_BW_20, &mcs20_offset_ppr_api, pi);
		}
		else if (CHSPEC_IS40(chanspec)) {
			/* * ofdm 20 in 40 */
			/* pwr_offsets = pi->ppr.sr9.ofdm[band_num].bw20ul; */
			wlc_phy_txpwr_srom9_convert(pi, ofdm20in40_offset_ppr_api.pwr,
			                            pi->ppr.sr9.ofdm[band_num].bw20ul,
			                            tmp_max_pwr, WL_RATESET_SZ_OFDM);
			/*  ofdm dup */
			/*  pwr_offsets = pi->ppr.sr9.ofdm[band_num].bw40; */
			wlc_phy_txpwr_srom9_convert(pi, ofdmdup40_offset_ppr_api.pwr,
			                            pi->ppr.sr9.ofdm[band_num].bw40,
			                            tmp_max_pwr, WL_RATESET_SZ_OFDM);
			/* ofdm_20IN40: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &ofdm20in40_offset_ppr_api, pi);
			/* ofdm DUP: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40,
			                     &ofdmdup40_offset_ppr_api, pi);

			/* XXX
			  printf("wlc_phy_txpwr_apply_srom9:  pi->ppr.sr9.mcs[band_num].bw20 =
			  %d ************ \n", pi->ppr.sr9.mcs[band_num].bw20);
			  printf("wlc_phy_txpwr_apply_srom9:  pi->ppr.sr9.mcs[band_num].bw20ul =
			  %d*************** \n", pi->ppr.sr9.ofdm[band_num].bw20ul);
			  printf("wlc_phy_txpwr_apply_srom9:  pi->ppr.sr9.mcs[band_num].bw40 =
			  %d*************** \n", pi->ppr.sr9.mcs[band_num].bw40);
			*/

			/* 40MHz HT  */

			/* pwr_offsets = pi->ppr.sr9.mcs[band_num].bw20ul; */
			wlc_phy_txpwr_srom9_convert(pi, mcs20in40_offset_ppr_api.pwr,
			                            pi->ppr.sr9.mcs[band_num].bw20ul,
			                            tmp_max_pwr, WL_RATESET_SZ_HT_MCS);
			/* 20IN40MHz HT */
			/* pwr_offsets = pi->ppr.sr9.mcs[band_num].bw40; */
			wlc_phy_txpwr_srom9_convert(pi, mcs40_offset_ppr_api.pwr,
			                            pi->ppr.sr9.mcs[band_num].bw40,
			                            tmp_max_pwr, WL_RATESET_SZ_HT_MCS);
			/* 40MHz HT */
			wlc_phy_ppr_set_ht_mcs(tx_srom_max_pwr,
			                       WL_TX_BW_40, &mcs40_offset_ppr_api, pi);
			/* 20IN40MHz HT */
			wlc_phy_ppr_set_ht_mcs(tx_srom_max_pwr,
			                       WL_TX_BW_20IN40, &mcs20in40_offset_ppr_api, pi);
		}
			break;
		default:
			break;
		}
		/* XXX
		  printf("wlc_phy_txpwr_apply_srom9: *******************
		  band = %d ******************** \n", band_num);
		  ppr_dsss_printf(tx_srom_max_pwr);
		  ppr_ofdm_printf(tx_srom_max_pwr);
		  ppr_mcs_ht_printf(tx_srom_max_pwr);
		*/
}

void
wlc_phy_txpwr_apply_srom_5g_subband(int8 max_pwr_ref, ppr_t *tx_srom_max_pwr,
	uint32 ofdm_20_offsets, uint32 mcs_20_offsets, uint32 mcs_40_offsets)
{
	ppr_ofdm_rateset_t ppr_ofdm;
	ppr_ht_mcs_rateset_t ppr_mcs;
	uint32 offset_mcs, last_offset_mcs;

	/* 5G-hi - OFDM_20 */
	wlc_phy_txpwr_srom_convert_ofdm(ofdm_20_offsets, max_pwr_ref, &ppr_ofdm);
	ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ppr_ofdm);

	/* 5G-hi - MCS_20 */
	offset_mcs = mcs_20_offsets;
	wlc_phy_txpwr_srom_convert_mcs(offset_mcs, max_pwr_ref, &ppr_mcs);
	ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&ppr_mcs);

	/* 5G-hi - MCS_40 */
	last_offset_mcs = offset_mcs;
	offset_mcs = mcs_40_offsets;
	if (!offset_mcs)
		offset_mcs = last_offset_mcs;

	wlc_phy_txpwr_srom_convert_mcs(offset_mcs, max_pwr_ref, &ppr_mcs);
	ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&ppr_mcs);
	/* Infer 20in40 MCS from this limit */
	ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_NONE,
		WL_TX_CHAINS_1, &ppr_mcs);
	/* Infer 20in40 OFDM from this limit */
	ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		(ppr_ofdm_rateset_t*)&ppr_mcs);
	/* Infer 40MHz OFDM from this limit */
	ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		(ppr_ofdm_rateset_t*)&ppr_mcs);
}

static const char BCMATTACHDATA(rstr_elna2g)[] = "elna2g";
static const char BCMATTACHDATA(rstr_elna5g)[] = "elna5g";
static const char BCMATTACHDATA(rstr_antswitch)[] = "antswitch";
static const char BCMATTACHDATA(rstr_aa2g)[] = "aa2g";
static const char BCMATTACHDATA(rstr_aa5g)[] = "aa5g";
static const char BCMATTACHDATA(rstr_tssipos2g)[] = "tssipos2g";
static const char BCMATTACHDATA(rstr_extpagain2g)[] = "extpagain2g";
static const char BCMATTACHDATA(rstr_pdetrange2g)[] = "pdetrange2g";
static const char BCMATTACHDATA(rstr_triso2g)[] = "triso2g";
static const char BCMATTACHDATA(rstr_antswctl2g)[] = "antswctl2g";
static const char BCMATTACHDATA(rstr_tssipos5g)[] = "tssipos5g";
static const char BCMATTACHDATA(rstr_extpagain5g)[] = "extpagain5g";
static const char BCMATTACHDATA(rstr_pdetrange5g)[] = "pdetrange5g";
static const char BCMATTACHDATA(rstr_triso5g)[] = "triso5g";
static const char BCMATTACHDATA(rstr_antswctl5g)[] = "antswctl5g";
static const char BCMATTACHDATA(rstr_pcieingress_war)[] = "pcieingress_war";
static const char BCMATTACHDATA(rstr_pa2gw0a3)[] = "pa2gw0a3";
static const char BCMATTACHDATA(rstr_pa2gw1a3)[] = "pa2gw1a3";
static const char BCMATTACHDATA(rstr_pa2gw2a3)[] = "pa2gw2a3";
static const char BCMATTACHDATA(rstr_pa2ga0)[] = "pa2ga0";
static const char BCMATTACHDATA(rstr_pa2ga1)[] = "pa2ga1";
static const char BCMATTACHDATA(rstr_pa2ga2)[] = "pa2ga2";
static const char BCMATTACHDATA(rstr_pa2ga3)[] = "pa2ga3";
static const char BCMATTACHDATA(rstr_pa5ga0)[] = "pa5ga0";
static const char BCMATTACHDATA(rstr_pa5ga1)[] = "pa5ga1";
static const char BCMATTACHDATA(rstr_pa5ga2)[] = "pa5ga2";
static const char BCMATTACHDATA(rstr_pa5ga3)[] = "pa5ga3";
static const char BCMATTACHDATA(rstr_pa5gbw4080a0)[] = "pa5gbw4080a0";
static const char BCMATTACHDATA(rstr_pa5gbw4080a1)[] = "pa5gbw4080a1";
static const char BCMATTACHDATA(rstr_pa2gccka0)[] = "pa2gccka0";
static const char BCMATTACHDATA(rstr_pa2gccka1)[] = "pa2gccka1";
static const char BCMATTACHDATA(rstr_pa2gbw40a0)[] = "pa2gbw40a0";
static const char BCMATTACHDATA(rstr_pa5gbw40a0)[] = "pa5gbw40a0";
static const char BCMATTACHDATA(rstr_pa5gbw80a0)[] = "pa5gbw80a0";
static const char BCMATTACHDATA(rstr_tssifloor2g)[] = "tssifloor2g";
static const char BCMATTACHDATA(rstr_tssifloor5g)[] = "tssifloor5g";
static const char BCMATTACHDATA(rstr_tempoffset)[] = "tempoffset";
static const char BCMATTACHDATA(rstr_tempoption)[] = "tempsense_option";
#ifdef POWPERCHANNL
static const char BCMATTACHDATA(rstr_PowOffs2GTNA0)[] = "powoffs2gtna0";
static const char BCMATTACHDATA(rstr_PowOffs2GTNA1)[] = "powoffs2gtna1";
static const char BCMATTACHDATA(rstr_PowOffs2GTNA2)[] = "powoffs2gtna2";
static const char BCMATTACHDATA(rstr_PowOffs2GTLA0)[] = "powoffs2gtla0";
static const char BCMATTACHDATA(rstr_PowOffs2GTLA1)[] = "powoffs2gtla1";
static const char BCMATTACHDATA(rstr_PowOffs2GTLA2)[] = "powoffs2gtla2";
static const char BCMATTACHDATA(rstr_PowOffs2GTHA0)[] = "powoffs2gtha0";
static const char BCMATTACHDATA(rstr_PowOffs2GTHA1)[] = "powoffs2gtha1";
static const char BCMATTACHDATA(rstr_PowOffs2GTHA2)[] = "powoffs2gtha2";
static const char BCMATTACHDATA(rstr_PowOffsTempRange)[] = "powoffstemprange";
#endif /* POWPERCHANNL */
static const char BCMATTACHDATA(rstr_pdoffset2g40mvalid)[] = "pdoffset2g40mvalid";
static const char BCMATTACHDATA(rstr_pdoffset40ma0)[]      = "pdoffset40ma0";
static const char BCMATTACHDATA(rstr_pdoffset80ma0)[]      = "pdoffset80ma0";
static const char BCMATTACHDATA(rstr_pdoffset2g40ma0)[]    = "pdoffset2g40ma0";
static const char BCMATTACHDATA(rstr_pdoffsetcckma0)[]     = "pdoffsetcckma0";
static const char BCMATTACHDATA(rstr_cckpwroffset0)[]      = "cckpwroffset0";
static const char BCMATTACHDATA(rstr_cckPwrIdxCorr)[]      = "cckPwrIdxCorr";
static const char BCMATTACHDATA(rstr_pdoffset40ma1)[]      = "pdoffset40ma1";
static const char BCMATTACHDATA(rstr_pdoffset80ma1)[]      = "pdoffset80ma1";
static const char BCMATTACHDATA(rstr_pdoffset2g40ma1)[]    = "pdoffset2g40ma1";
static const char BCMATTACHDATA(rstr_pdoffsetcckma1)[]     = "pdoffsetcckma1";
static const char BCMATTACHDATA(rstr_cckpwroffset1)[]      = "cckpwroffset1";
static const char BCMATTACHDATA(rstr_pdoffset40ma2)[]      = "pdoffset40ma2";
static const char BCMATTACHDATA(rstr_pdoffset80ma2)[]      = "pdoffset80ma2";
static const char BCMATTACHDATA(rstr_pdoffset2g40ma2)[]    = "pdoffset2g40ma2";
static const char BCMATTACHDATA(rstr_pdoffsetcckma2)[]     = "pdoffsetcckma2";
static const char BCMATTACHDATA(rstr_cckpwroffset2)[]      = "cckpwroffset2";

static const char BCMATTACHDATA(rstr_pa2g40a0)[] = "pa2g40a0";
static const char BCMATTACHDATA(rstr_pa2g40a1)[] = "pa2g40a1";
static const char BCMATTACHDATA(rstr_pa2g40a2)[] = "pa2g40a2";
static const char BCMATTACHDATA(rstr_pa2g40a3)[] = "pa2g40a3";
static const char BCMATTACHDATA(rstr_pa5g40a0)[] = "pa5g40a0";
static const char BCMATTACHDATA(rstr_pa5g40a1)[] = "pa5g40a1";
static const char BCMATTACHDATA(rstr_pa5g40a2)[] = "pa5g40a2";
static const char BCMATTACHDATA(rstr_pa5g40a3)[] = "pa5g40a3";
static const char BCMATTACHDATA(rstr_pa5g80a0)[] = "pa5g80a0";
static const char BCMATTACHDATA(rstr_pa5g80a1)[] = "pa5g80a1";
static const char BCMATTACHDATA(rstr_pa5g80a2)[] = "pa5g80a2";
static const char BCMATTACHDATA(rstr_pa5g80a3)[] = "pa5g80a3";
static const char BCMATTACHDATA(rstr_maxp2gb0a0)[] = "maxp2ga0";
static const char BCMATTACHDATA(rstr_maxp2gb0a1)[] = "maxp2ga1";
static const char BCMATTACHDATA(rstr_maxp2gb0a2)[] = "maxp2ga2";
static const char BCMATTACHDATA(rstr_maxp2gb0a3)[] = "maxp2ga3";
static const char BCMATTACHDATA(rstr_maxp5gb0a0)[] = "maxp5gb0a0";
static const char BCMATTACHDATA(rstr_maxp5gb0a1)[] = "maxp5gb0a1";
static const char BCMATTACHDATA(rstr_maxp5gb0a2)[] = "maxp5gb0a2";
static const char BCMATTACHDATA(rstr_maxp5gb0a3)[] = "maxp5gb0a3";
static const char BCMATTACHDATA(rstr_maxp5gb1a0)[] = "maxp5gb1a0";
static const char BCMATTACHDATA(rstr_maxp5gb1a1)[] = "maxp5gb1a1";
static const char BCMATTACHDATA(rstr_maxp5gb1a2)[] = "maxp5gb1a2";
static const char BCMATTACHDATA(rstr_maxp5gb1a3)[] = "maxp5gb1a3";
static const char BCMATTACHDATA(rstr_maxp5gb2a0)[] = "maxp5gb2a0";
static const char BCMATTACHDATA(rstr_maxp5gb2a1)[] = "maxp5gb2a1";
static const char BCMATTACHDATA(rstr_maxp5gb2a2)[] = "maxp5gb2a2";
static const char BCMATTACHDATA(rstr_maxp5gb2a3)[] = "maxp5gb2a3";
static const char BCMATTACHDATA(rstr_maxp5gb3a0)[] = "maxp5gb3a0";
static const char BCMATTACHDATA(rstr_maxp5gb3a1)[] = "maxp5gb3a1";
static const char BCMATTACHDATA(rstr_maxp5gb3a2)[] = "maxp5gb3a2";
static const char BCMATTACHDATA(rstr_maxp5gb3a3)[] = "maxp5gb3a3";
static const char BCMATTACHDATA(rstr_maxp5gb4a0)[] = "maxp5gb4a0";
static const char BCMATTACHDATA(rstr_maxp5gb4a1)[] = "maxp5gb4a1";
static const char BCMATTACHDATA(rstr_maxp5gb4a2)[] = "maxp5gb4a2";
static const char BCMATTACHDATA(rstr_maxp5gb4a3)[] = "maxp5gb4a3";
static const char BCMATTACHDATA(rstr_pdoffset2gcck)[]       = "pdoffsetcck";
static const char BCMATTACHDATA(rstr_pdoffset2gcck20m)[]    = "pdoffsetcck20m";
static const char BCMATTACHDATA(rstr_pdoffset20in40m5gb0)[] = "pdoffset20in40m5gb0";
static const char BCMATTACHDATA(rstr_pdoffset20in40m5gb1)[] = "pdoffset20in40m5gb1";
static const char BCMATTACHDATA(rstr_pdoffset20in40m5gb2)[] = "pdoffset20in40m5gb2";
static const char BCMATTACHDATA(rstr_pdoffset20in40m5gb3)[] = "pdoffset20in40m5gb3";
static const char BCMATTACHDATA(rstr_pdoffset20in40m5gb4)[] = "pdoffset20in40m5gb4";
static const char BCMATTACHDATA(rstr_pdoffset20in80m5gb0)[] = "pdoffset20in80m5gb0";
static const char BCMATTACHDATA(rstr_pdoffset20in80m5gb1)[] = "pdoffset20in80m5gb1";
static const char BCMATTACHDATA(rstr_pdoffset20in80m5gb2)[] = "pdoffset20in80m5gb2";
static const char BCMATTACHDATA(rstr_pdoffset20in80m5gb3)[] = "pdoffset20in80m5gb3";
static const char BCMATTACHDATA(rstr_pdoffset20in80m5gb4)[] = "pdoffset20in80m5gb4";
static const char BCMATTACHDATA(rstr_pdoffset40in80m5gb0)[] = "pdoffset40in80m5gb0";
static const char BCMATTACHDATA(rstr_pdoffset40in80m5gb1)[] = "pdoffset40in80m5gb1";
static const char BCMATTACHDATA(rstr_pdoffset40in80m5gb2)[] = "pdoffset40in80m5gb2";
static const char BCMATTACHDATA(rstr_pdoffset40in80m5gb3)[] = "pdoffset40in80m5gb3";
static const char BCMATTACHDATA(rstr_pdoffset40in80m5gb4)[] = "pdoffset40in80m5gb4";
static const char BCMATTACHDATA(rstr_pdoffset20in40m5gcore3)[]   = "pdoffset20in40m5gcore3";
static const char BCMATTACHDATA(rstr_pdoffset20in40m5gcore3_1)[] = "pdoffset20in40m5gcore3_1";
static const char BCMATTACHDATA(rstr_pdoffset20in80m5gcore3)[]   = "pdoffset20in80m5gcore3";
static const char BCMATTACHDATA(rstr_pdoffset20in80m5gcore3_1)[] = "pdoffset20in80m5gcore3_1";
static const char BCMATTACHDATA(rstr_pdoffset40in80m5gcore3)[]   = "pdoffset40in80m5gcore3";
static const char BCMATTACHDATA(rstr_pdoffset40in80m5gcore3_1)[] = "pdoffset40in80m5gcore3_1";
static const char BCMATTACHDATA(rstr_pdoffset20in40m2g)[]        = "pdoffset20in40m2g";
static const char BCMATTACHDATA(rstr_pdoffset20in40m2gcore3)[]   = "pdoffset20in40m2gcore3";

bool
BCMATTACHFN(wlc_phy_txpwr_srom11_read)(phy_info_t *pi)
{
	srom11_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 b, maxval, ant = 1;
	bool update_rsdb_core1_params = FALSE;
#ifdef POWPERCHANNL
	uint8 ch;
#endif /* POWPERCHANNL */
	ASSERT(pi->sh->sromrev >= 11);

	if (!ISACPHY(pi)) {
		return FALSE;
	}

	maxval = CH_2G_GROUP + CH_5G_4BAND;

	if (phy_get_phymode(pi) == PHYMODE_RSDB)
	{
		/* update pi[0] to hold pwrdet params for all cores */
		/* This is required for mimo operation */
		if (phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0)
		{
			pi->pubpi.phy_corenum <<= 1;
		}
		else if (phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1)
		{
			/* Incase of RSDB Core1 PA PD MAXPwr Params needs to be
			 * updated from Core1 nvram params
			 */
			ant = 0;
			update_rsdb_core1_params = TRUE;
		}
	}
	/* read pwrdet params for each band/subband */
	for (b = 0; b < maxval; b++) {
		switch (b) {
			case WL_CHAN_FREQ_RANGE_2G: /* 0 */
			/* 2G band */
				pwrdet->max_pwr[0][b]	=
					(int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
				pwrdet->pwrdet_a1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 0);
				pwrdet->pwrdet_b0[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 1);
				pwrdet->pwrdet_b1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 2);
				pwrdet->tssifloor[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_tssifloor2g, 0);
#ifdef POWPERCHANNL
				/* power per channel and Temp */
				pwrdet->max_pwr_SROM2G[0] = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);

				PHY_TXPWR(("wl%d: %s: Loading max = %d \n",
					pi->sh->unit, __FUNCTION__,
					pwrdet->max_pwr_SROM2G[0]));
				for (ch = 0; ch < CH20MHz_NUM_2G; ch++) {
					pwrdet->PwrOffsets2GNormTemp[0][ch] =
						(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
							pi, rstr_PowOffs2GTNA0, ch, 0);
					pwrdet->PwrOffsets2GLowTemp[0][ch] =
						(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
							pi, rstr_PowOffs2GTLA0, ch, 0);
					pwrdet->PwrOffsets2GHighTemp[0][ch] =
						(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
							pi, rstr_PowOffs2GTHA0, ch, 0);

					PHY_TXPWR(("Core=0 Ch=%d Offset: Norm=%d Low=%d High=%d\n",
						ch, pwrdet->PwrOffsets2GNormTemp[0][ch],
						pwrdet->PwrOffsets2GLowTemp[0][ch],
						pwrdet->PwrOffsets2GHighTemp[0][ch]));
				}
				pwrdet->Low2NormTemp =
					(int16)PHY_GETINTVAR_ARRAY_DEFAULT(
						pi, rstr_PowOffsTempRange, 0, 0xff);
				pwrdet->High2NormTemp =
					(int16)PHY_GETINTVAR_ARRAY_DEFAULT(
						pi, rstr_PowOffsTempRange, 1, 0xff);

				PHY_TXPWR((" Low Temp Limit=%d  High Temp Limit=%d \n",
					pwrdet->Low2NormTemp,
					pwrdet->High2NormTemp));
#endif  /* POWPERCHANNL */
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(update_rsdb_core1_params == TRUE)) {
					pwrdet->max_pwr[ant][b]	=
						(int8)PHY_GETINTVAR(pi, rstr_maxp2ga1);
#ifdef POWPERCHANNL
					pwrdet->max_pwr_SROM2G[ant] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp2ga1);
					for (ch = 0; ch < CH20MHz_NUM_2G; ch++) {
						pwrdet->PwrOffsets2GNormTemp[ant][ch] =
							(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
								pi, rstr_PowOffs2GTNA1, ch, 0);
						pwrdet->PwrOffsets2GLowTemp[ant][ch] =
							(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
								pi, rstr_PowOffs2GTLA1, ch, 0);
						pwrdet->PwrOffsets2GHighTemp[ant][ch] =
							(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
								pi, rstr_PowOffs2GTHA1, ch, 0);

						PHY_TXPWR(("Core=1 Ch=%d Offset:", ch));
						PHY_TXPWR(("Norm=%d Low=%d High=%d\n",
							pwrdet->PwrOffsets2GNormTemp[ant][ch],
							pwrdet->PwrOffsets2GLowTemp[ant][ch],
							pwrdet->PwrOffsets2GHighTemp[ant][ch]));
					}
#endif  /* POWPERCHANNL */
				}
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(pi->u.pi_acphy->srom_tworangetssi2g &&
					ACMAJORREV_1(pi->pubpi.phy_rev)) ||
					(update_rsdb_core1_params == TRUE)) {
					pwrdet->pwrdet_a1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga1, 0);
					pwrdet->pwrdet_b0[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga1, 1);
					pwrdet->pwrdet_b1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga1, 2);
				}
				if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
					pwrdet->max_pwr[2][b]	=
						(int8)PHY_GETINTVAR(pi, rstr_maxp2ga2);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga2, 0);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga2, 1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga2, 2);
#ifdef POWPERCHANNL
					pwrdet->max_pwr_SROM2G[2] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp2ga2);
					for (ch = 0; ch < CH20MHz_NUM_2G; ch++) {
						pwrdet->PwrOffsets2GNormTemp[2][ch] =
							(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
								pi, rstr_PowOffs2GTNA2, ch, 0);
						pwrdet->PwrOffsets2GLowTemp[2][ch] =
							(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
								pi, rstr_PowOffs2GTLA2, ch, 0);
						pwrdet->PwrOffsets2GHighTemp[2][ch] =
							(int8)PHY_GETINTVAR_ARRAY_DEFAULT(
								pi, rstr_PowOffs2GTHA2, ch, 0);
					}
					/* input range limit for power per channel */
					wlc_phy_tx_target_pwr_per_channel_limit_acphy(pi);
#endif  /* POWPERCHANNL */
				}
#ifdef POWPERCHANNL
				/* input range limit for power per channel */
				wlc_phy_tx_target_pwr_per_channel_limit_acphy(pi);
#endif  /* POWPERCHANNL */
				if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
					(ACMAJORREV_1(pi->pubpi.phy_rev) ||
					(TINY_RADIO(pi) && !ACMAJORREV_4(pi->pubpi.phy_rev)))) {
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka0, 0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka0, 1);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka0, 2);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gbw40a0, 0);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gbw40a0, 1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gbw40a0, 2);

				}
				if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
					(((ACMAJORREV_2(pi->pubpi.phy_rev)) && (PHY_IPA(pi))) ||
					(ACMAJORREV_4(pi->pubpi.phy_rev)))) {
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka0, 0);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka0, 1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka0, 2);
					pwrdet->pwrdet_a1[2+ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka1, 0);
					pwrdet->pwrdet_b0[2+ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka1, 1);
					pwrdet->pwrdet_b1[2+ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2gccka1, 2);
				}
				break;
#ifdef BAND5G
			case WL_CHAN_FREQ_RANGE_5G_BAND0: /* 1 */
				pwrdet->max_pwr[0][b]	=
					(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga0, 0);
				pwrdet->pwrdet_a1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 0);
				pwrdet->pwrdet_b0[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 1);
				pwrdet->pwrdet_b1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 2);
				pwrdet->tssifloor[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_tssifloor5g, 0);
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(update_rsdb_core1_params == TRUE)) {
					pwrdet->max_pwr[ant][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga1, 0);
				}
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(pi->u.pi_acphy->srom_tworangetssi5g &&
					ACMAJORREV_1(pi->pubpi.phy_rev)) ||
					(update_rsdb_core1_params == TRUE))  {
					pwrdet->pwrdet_a1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 0);
					pwrdet->pwrdet_b0[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 1);
					pwrdet->pwrdet_b1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 2);
				}
				if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
					pwrdet->max_pwr[2][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga2, 0);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 0);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 2);
				}
				if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
					(ACMAJORREV_1(pi->pubpi.phy_rev) ||
					(TINY_RADIO(pi) && !ACMAJORREV_4(pi->pubpi.phy_rev)))) {
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 1);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 2);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 0);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 2);
				}
				if ((ACMAJORREV_2(pi->pubpi.phy_rev) ||
					ACMAJORREV_4(pi->pubpi.phy_rev)) &&
					BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
					pwrdet->pwrdet_a1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 0);
					pwrdet->pwrdet_b0[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 1);
					pwrdet->pwrdet_b1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 2);
					pwrdet->pwrdet_a1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 0);
					pwrdet->pwrdet_b0[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 1);
					pwrdet->pwrdet_b1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 2);
				}
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND1: /* 2 */
				pwrdet->max_pwr[0][b]	=
					(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga0, 1);
				pwrdet->pwrdet_a1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 3);
				pwrdet->pwrdet_b0[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 4);
				pwrdet->pwrdet_b1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 5);
				pwrdet->tssifloor[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_tssifloor5g, 1);
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(update_rsdb_core1_params == TRUE))	{
					pwrdet->max_pwr[ant][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga1, 1);
				}
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(pi->u.pi_acphy->srom_tworangetssi5g &&
					ACMAJORREV_1(pi->pubpi.phy_rev)) ||
					(update_rsdb_core1_params == TRUE)) {
					pwrdet->pwrdet_a1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 3);
					pwrdet->pwrdet_b0[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 4);
					pwrdet->pwrdet_b1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 5);
				}
				if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
					pwrdet->max_pwr[2][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga2, 1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 3);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 4);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 5);
				}

				if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
					(ACMAJORREV_1(pi->pubpi.phy_rev) ||
					(TINY_RADIO(pi) && !ACMAJORREV_4(pi->pubpi.phy_rev)))) {
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 3);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 4);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 5);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 3);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 4);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 5);
				}

				if ((ACMAJORREV_2(pi->pubpi.phy_rev) ||
					ACMAJORREV_4(pi->pubpi.phy_rev)) &&
					BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
					pwrdet->pwrdet_a1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 3);
					pwrdet->pwrdet_b0[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 4);
					pwrdet->pwrdet_b1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 5);
					pwrdet->pwrdet_a1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 3);
					pwrdet->pwrdet_b0[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 4);
					pwrdet->pwrdet_b1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 5);
				}
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND2: /* 3 */
				pwrdet->max_pwr[0][b]	=
					(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga0, 2);
				pwrdet->pwrdet_a1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 6);
				pwrdet->pwrdet_b0[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 7);
				pwrdet->pwrdet_b1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 8);
				pwrdet->tssifloor[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_tssifloor5g, 2);
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
				(update_rsdb_core1_params == TRUE)) {
					pwrdet->max_pwr[ant][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga1, 2);
				}
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(pi->u.pi_acphy->srom_tworangetssi5g &&
					ACMAJORREV_1(pi->pubpi.phy_rev)) ||
					(update_rsdb_core1_params == TRUE)) {
					pwrdet->pwrdet_a1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 6);
					pwrdet->pwrdet_b0[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 7);
					pwrdet->pwrdet_b1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 8);
				}
				if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
					pwrdet->max_pwr[2][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga2, 2);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 6);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 7);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 8);
				}

				if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
					(ACMAJORREV_1(pi->pubpi.phy_rev) ||
					(TINY_RADIO(pi) && !ACMAJORREV_4(pi->pubpi.phy_rev)))) {
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 6);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 7);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 8);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 6);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 7);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 8);
				}

				if ((ACMAJORREV_2(pi->pubpi.phy_rev) ||
					ACMAJORREV_4(pi->pubpi.phy_rev)) &&
					BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
					pwrdet->pwrdet_a1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 6);
					pwrdet->pwrdet_b0[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 7);
					pwrdet->pwrdet_b1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 8);
					pwrdet->pwrdet_a1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 6);
					pwrdet->pwrdet_b0[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 7);
					pwrdet->pwrdet_b1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 8);
				}
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND3: /* 4 */
				pwrdet->max_pwr[0][b]	=
					(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga0, 3);
				pwrdet->pwrdet_a1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 9);
				pwrdet->pwrdet_b0[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 10);
				pwrdet->pwrdet_b1[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, 11);
				pwrdet->tssifloor[0][b] =
					(int16)PHY_GETINTVAR_ARRAY(pi, rstr_tssifloor5g, 3);
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
				(update_rsdb_core1_params == TRUE)) {
					pwrdet->max_pwr[ant][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga1, 3);
				}
				if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
					(pi->u.pi_acphy->srom_tworangetssi5g &&
					ACMAJORREV_1(pi->pubpi.phy_rev)) ||
					(update_rsdb_core1_params == TRUE)) {
					pwrdet->pwrdet_a1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 9);
					pwrdet->pwrdet_b0[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 10);
					pwrdet->pwrdet_b1[ant][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, 11);
				}
				if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
					pwrdet->max_pwr[2][b]	=
						(int8)PHY_GETINTVAR_ARRAY(pi, rstr_maxp5ga2, 3);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 9);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 10);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, 11);
				}

				if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
					(ACMAJORREV_1(pi->pubpi.phy_rev) ||
					(TINY_RADIO(pi) && !ACMAJORREV_4(pi->pubpi.phy_rev)))) {
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 9);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 10);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw40a0, 11);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 9);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 10);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw80a0, 11);
				}

				if ((ACMAJORREV_2(pi->pubpi.phy_rev) ||
					ACMAJORREV_4(pi->pubpi.phy_rev)) &&
					BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
					pwrdet->pwrdet_a1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 9);
					pwrdet->pwrdet_b0[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 10);
					pwrdet->pwrdet_b1[2][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a0, 11);
					pwrdet->pwrdet_a1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 9);
					pwrdet->pwrdet_b0[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 10);
					pwrdet->pwrdet_b1[2+ant][b] =
					    (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5gbw4080a1, 11);
				}
				break;
#endif /* BAND5G */
			default:
				break;
		}
	}
	wlc_phy_txpwr_srom11_read_ppr(pi);

	/* read out power detect offset values */
	pwrdet->pdoffset2g40_flag = (uint8)PHY_GETINTVAR(pi, rstr_pdoffset2g40mvalid);
	pwrdet->pdoffset40[0] = (uint16)PHY_GETINTVAR(pi, rstr_pdoffset40ma0);
	pwrdet->pdoffset80[0] = (uint16)PHY_GETINTVAR(pi, rstr_pdoffset80ma0);
	pwrdet->pdoffset2g40[0] = (uint8)PHY_GETINTVAR(pi, rstr_pdoffset2g40ma0);
	pwrdet->pdoffsetcck[0] = (uint8)PHY_GETINTVAR(pi, rstr_pdoffsetcckma0);
	pi->cckpwroffset[0] = (int8)PHY_GETINTVAR(pi, rstr_cckpwroffset0);
	pi->sh->cckPwrIdxCorr = (int8) PHY_GETINTVAR(pi, rstr_cckPwrIdxCorr);
	if ((PHYCORENUM(pi->pubpi.phy_corenum) > 1) ||
		(update_rsdb_core1_params == TRUE)) {
		pwrdet->pdoffset40[ant] = (uint16)PHY_GETINTVAR(pi, rstr_pdoffset40ma1);
		pwrdet->pdoffset80[ant] = (uint16)PHY_GETINTVAR(pi, rstr_pdoffset80ma1);
		pwrdet->pdoffset2g40[ant] = (uint8)PHY_GETINTVAR(pi, rstr_pdoffset2g40ma1);
		pwrdet->pdoffsetcck[ant] = (uint8)PHY_GETINTVAR(pi, rstr_pdoffsetcckma1);
		pi->cckpwroffset[ant] = (int8)PHY_GETINTVAR(pi, rstr_cckpwroffset1);
	}
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
		pwrdet->pdoffset40[2] = (uint16)PHY_GETINTVAR(pi, rstr_pdoffset40ma2);
		pwrdet->pdoffset80[2] = (uint16)PHY_GETINTVAR(pi, rstr_pdoffset80ma2);
		pwrdet->pdoffset2g40[2] = (uint8)PHY_GETINTVAR(pi, rstr_pdoffset2g40ma2);
		pwrdet->pdoffsetcck[2] = (uint8)PHY_GETINTVAR(pi, rstr_pdoffsetcckma2);
		pi->cckpwroffset[2] = (int8)PHY_GETINTVAR(pi, rstr_cckpwroffset2);
	}

	if ((phy_get_phymode(pi) == PHYMODE_RSDB) &&
	(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0))
	{
		pi->pubpi.phy_corenum >>= 1;
	}

	pi->phy_tempsense_offset = (int8)PHY_GETINTVAR(pi, rstr_tempoffset);
	if (pi->phy_tempsense_offset == -1) {
		pi->phy_tempsense_offset = 0;
	} else if (pi->phy_tempsense_offset != 0) {
		if (pi->phy_tempsense_offset >
			(ACPHY_SROM_TEMPSHIFT + ACPHY_SROM_MAXTEMPOFFSET)) {
			pi->phy_tempsense_offset = ACPHY_SROM_MAXTEMPOFFSET;
		} else if (pi->phy_tempsense_offset < (ACPHY_SROM_TEMPSHIFT +
			ACPHY_SROM_MINTEMPOFFSET)) {
			pi->phy_tempsense_offset = ACPHY_SROM_MINTEMPOFFSET;
		} else {
			pi->phy_tempsense_offset -= ACPHY_SROM_TEMPSHIFT;
		}
	}

	/* For ACPHY, if the SROM contains a bogus value, then tempdelta
	 * will default to ACPHY_DEFAULT_CAL_TEMPDELTA. If the SROM contains
	 * a valid value, then the default will be overwritten with this value
	 */
	wlc_phy_read_tempdelta_settings(pi, ACPHY_CAL_MAXTEMPDELTA);

	return TRUE;
}
#define PDOFFSET(pi, nvramstrng, core) ((uint16)(PHY_GETINTVAR((pi), \
	(nvramstrng)) >> (5 * (core))) & 0x1f)
bool
BCMATTACHFN(wlc_phy_txpwr_srom12_read)(phy_info_t *pi)
{
	srom12_pwrdet_t *pwrdet = pi->pwrdet_ac;
	uint8 b = 0, core;
	if (!(SROMREV(pi->sh->sromrev) < 12)) {
#ifdef BAND5G
	    uint8 i = 0, j = 0, maxval = 0;
	    maxval = CH_5G_5BAND * 4; /* PAparams per subband for particular bandwidth = 4 */
#endif // endif

	    ASSERT(pi->sh->sromrev >= 12);

	    if (!ISACPHY(pi)) {
		return FALSE;
	    }

	    /* read pwrdet params for each band/subband/bandwidth */
	    /* 2G_20MHz */
	    pwrdet->pwrdet_a[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 0);
	    pwrdet->pwrdet_b[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 1);
	    pwrdet->pwrdet_c[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 2);
	    pwrdet->pwrdet_d[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 3);
	    /* 2G_40MHz */
	    pwrdet->pwrdet_a_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a0, 0);
	    pwrdet->pwrdet_b_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a0, 1);
	    pwrdet->pwrdet_c_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a0, 2);
	    pwrdet->pwrdet_d_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a0, 3);

	    if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		/* 2G_20MHz */
		pwrdet->pwrdet_a[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga1, 0);
		pwrdet->pwrdet_b[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga1, 1);
		pwrdet->pwrdet_c[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga1, 2);
		pwrdet->pwrdet_d[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga1, 3);
		/* 2G_40MHz */
		pwrdet->pwrdet_a_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a1, 0);
		pwrdet->pwrdet_b_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a1, 1);
		pwrdet->pwrdet_c_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a1, 2);
		pwrdet->pwrdet_d_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a1, 3);
	    }
	    if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
		/* 2G_20MHz */
		pwrdet->pwrdet_a[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga2, 0);
		pwrdet->pwrdet_b[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga2, 1);
		pwrdet->pwrdet_c[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga2, 2);
		pwrdet->pwrdet_d[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga2, 3);
		/* 2G_40MHz */
		pwrdet->pwrdet_a_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a2, 0);
		pwrdet->pwrdet_b_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a2, 1);
		pwrdet->pwrdet_c_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a2, 2);
		pwrdet->pwrdet_d_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a2, 3);
	    }
	    if (PHYCORENUM(pi->pubpi.phy_corenum) > 3) {
		/* 2G_20MHz */
		pwrdet->pwrdet_a[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga3, 0);
		pwrdet->pwrdet_b[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga3, 1);
		pwrdet->pwrdet_c[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga3, 2);
		pwrdet->pwrdet_d[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga3, 3);
		/* 2G_40MHz */
		pwrdet->pwrdet_a_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a3, 0);
		pwrdet->pwrdet_b_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a3, 1);
		pwrdet->pwrdet_c_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a3, 2);
		pwrdet->pwrdet_d_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2g40a3, 3);
	    }

	    ++b;

#ifdef BAND5G
	    for (i = 0; i < maxval; i = i + 4) {
		j = b - 1; /* 5G 80 MHz index starts from 0 */
		/* 5G_BANDS_20MHz */
		pwrdet->pwrdet_a[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, i);
		pwrdet->pwrdet_b[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, i+1);
		pwrdet->pwrdet_c[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, i+2);
		pwrdet->pwrdet_d[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga0, i+3);
		/* 5G_BANDS_40MHz */
		pwrdet->pwrdet_a_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a0, i);
		pwrdet->pwrdet_b_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a0, i+1);
		pwrdet->pwrdet_c_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a0, i+2);
		pwrdet->pwrdet_d_40[0][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a0, i+3);
		/* 5G_BANDS_80MHz */
		pwrdet->pwrdet_a_80[0][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a0, i);
		pwrdet->pwrdet_b_80[0][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a0, i+1);
		pwrdet->pwrdet_c_80[0][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a0, i+2);
		pwrdet->pwrdet_d_80[0][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a0, i+3);

		if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		    /* 5G_BANDS_20MHz */
		    pwrdet->pwrdet_a[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, i);
		    pwrdet->pwrdet_b[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, i+1);
		    pwrdet->pwrdet_c[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, i+2);
		    pwrdet->pwrdet_d[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga1, i+3);
		    /* 5G_BANDS_40MHz */
		    pwrdet->pwrdet_a_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a1, i);
		    pwrdet->pwrdet_b_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a1, i+1);
		    pwrdet->pwrdet_c_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a1, i+2);
		    pwrdet->pwrdet_d_40[1][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a1, i+3);
		    /* 5G_BANDS_80MHz */
		    pwrdet->pwrdet_a_80[1][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a1, i);
		    pwrdet->pwrdet_b_80[1][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a1, i+1);
		    pwrdet->pwrdet_c_80[1][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a1, i+2);
		    pwrdet->pwrdet_d_80[1][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a1, i+3);
		}

		if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
		    /* 5G_BANDS_20MHz */
		    pwrdet->pwrdet_a[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, i);
		    pwrdet->pwrdet_b[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, i+1);
		    pwrdet->pwrdet_c[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, i+2);
		    pwrdet->pwrdet_d[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga2, i+3);
		    /* 5G_BANDS_40MHz */
		    pwrdet->pwrdet_a_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a2, i);
		    pwrdet->pwrdet_b_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a2, i+1);
		    pwrdet->pwrdet_c_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a2, i+2);
		    pwrdet->pwrdet_d_40[2][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a2, i+3);
		    /* 5G_BANDS_80MHz */
		    pwrdet->pwrdet_a_80[2][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a2, i);
		    pwrdet->pwrdet_b_80[2][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a2, i+1);
		    pwrdet->pwrdet_c_80[2][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a2, i+2);
		    pwrdet->pwrdet_d_80[2][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a2, i+3);
		}
		if (PHYCORENUM(pi->pubpi.phy_corenum) > 3) {
		    /* 5G_BANDS_20MHz */
		    pwrdet->pwrdet_a[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga3, i);
		    pwrdet->pwrdet_b[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga3, i+1);
		    pwrdet->pwrdet_c[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga3, i+2);
		    pwrdet->pwrdet_d[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5ga3, i+3);
		    /* 5G_BANDS_40MHz */
		    pwrdet->pwrdet_a_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a3, i);
		    pwrdet->pwrdet_b_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a3, i+1);
		    pwrdet->pwrdet_c_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a3, i+2);
		    pwrdet->pwrdet_d_40[3][b] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g40a3, i+3);
		    /* 5G_BANDS_80MHz */
		    pwrdet->pwrdet_a_80[3][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a3, i);
		    pwrdet->pwrdet_b_80[3][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a3, i+1);
		    pwrdet->pwrdet_c_80[3][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a3, i+2);
		    pwrdet->pwrdet_d_80[3][j] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa5g80a3, i+3);
		}
		++b;
	    }

	    i = 0;
	    pwrdet->max_pwr[0][i++] = (int8)PHY_GETINTVAR(pi, rstr_maxp2gb0a0);
	    pwrdet->max_pwr[0][i++] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb0a0);
	    pwrdet->max_pwr[0][i++] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb1a0);
	    pwrdet->max_pwr[0][i++] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb2a0);
	    pwrdet->max_pwr[0][i++] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb3a0);
	    pwrdet->max_pwr[0][i++] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb4a0);

	    if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
		i = 0;
		pwrdet->max_pwr[1][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp2gb0a1);
		pwrdet->max_pwr[1][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb0a1);
		pwrdet->max_pwr[1][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb1a1);
		pwrdet->max_pwr[1][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb2a1);
		pwrdet->max_pwr[1][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb3a1);
		pwrdet->max_pwr[1][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb4a1);
	    }

	    if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
		i = 0;
		pwrdet->max_pwr[2][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp2gb0a2);
		pwrdet->max_pwr[2][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb0a2);
		pwrdet->max_pwr[2][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb1a2);
		pwrdet->max_pwr[2][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb2a2);
		pwrdet->max_pwr[2][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb3a2);
		pwrdet->max_pwr[2][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb4a2);
	    }
	    if (PHYCORENUM(pi->pubpi.phy_corenum) > 3) {
		i = 0;
		pwrdet->max_pwr[3][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp2gb0a3);
		pwrdet->max_pwr[3][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb0a3);
		pwrdet->max_pwr[3][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb1a3);
		pwrdet->max_pwr[3][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb2a3);
		pwrdet->max_pwr[3][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb3a3);
		pwrdet->max_pwr[3][i++]  = (int8)PHY_GETINTVAR(pi, rstr_maxp5gb4a3);
	    }
#endif /* BAND5G */
	}

	if (SROMREV(pi->sh->sromrev) < 13) {
		wlc_phy_txpwr_srom12_read_ppr(pi);
	}
#ifdef WL11AC
	else {
		wlc_phy_txpwr_srom13_read_ppr(pi);
	}
#endif /* WL11AC */

	if (!(SROMREV(pi->sh->sromrev) < 12)) {
	    /* read out power detect offset values */
	    FOREACH_CORE(pi, core) {
		if (core < 3) {
		    pwrdet->pdoffsetcck[core]    = PDOFFSET(pi, rstr_pdoffset2gcck,    core);
		    pwrdet->pdoffsetcck20m[core] = PDOFFSET(pi, rstr_pdoffset2gcck20m, core);
		    pwrdet->pdoffset20in40[core][0] = PDOFFSET(pi, rstr_pdoffset20in40m2g,   core);
		    pwrdet->pdoffset20in40[core][1] = PDOFFSET(pi, rstr_pdoffset20in40m5gb0, core);
		    pwrdet->pdoffset20in40[core][2] = PDOFFSET(pi, rstr_pdoffset20in40m5gb1, core);
		    pwrdet->pdoffset20in40[core][3] = PDOFFSET(pi, rstr_pdoffset20in40m5gb2, core);
		    pwrdet->pdoffset20in40[core][4] = PDOFFSET(pi, rstr_pdoffset20in40m5gb3, core);
		    pwrdet->pdoffset20in40[core][5] = PDOFFSET(pi, rstr_pdoffset20in40m5gb4, core);
		    pwrdet->pdoffset20in80[core][1] = PDOFFSET(pi, rstr_pdoffset20in80m5gb0, core);
		    pwrdet->pdoffset20in80[core][2] = PDOFFSET(pi, rstr_pdoffset20in80m5gb1, core);
		    pwrdet->pdoffset20in80[core][3] = PDOFFSET(pi, rstr_pdoffset20in80m5gb2, core);
		    pwrdet->pdoffset20in80[core][4] = PDOFFSET(pi, rstr_pdoffset20in80m5gb3, core);
		    pwrdet->pdoffset20in80[core][5] = PDOFFSET(pi, rstr_pdoffset20in80m5gb4, core);
		    pwrdet->pdoffset40in80[core][1] = PDOFFSET(pi, rstr_pdoffset40in80m5gb0, core);
		    pwrdet->pdoffset40in80[core][2] = PDOFFSET(pi, rstr_pdoffset40in80m5gb1, core);
		    pwrdet->pdoffset40in80[core][3] = PDOFFSET(pi, rstr_pdoffset40in80m5gb2, core);
		    pwrdet->pdoffset40in80[core][4] = PDOFFSET(pi, rstr_pdoffset40in80m5gb3, core);
		    pwrdet->pdoffset40in80[core][5] = PDOFFSET(pi, rstr_pdoffset40in80m5gb4, core);
		} else {
		   pwrdet->pdoffsetcck[core]    = PDOFFSET(pi, rstr_pdoffset20in40m2gcore3, 1);
		   pwrdet->pdoffsetcck20m[core] = PDOFFSET(pi, rstr_pdoffset20in40m2gcore3, 2);
		   pwrdet->pdoffset20in40[core][0] = PDOFFSET(pi, rstr_pdoffset20in40m2gcore3, 0);
		   pwrdet->pdoffset20in40[core][1] = PDOFFSET(pi, rstr_pdoffset20in40m5gcore3, 0);
		   pwrdet->pdoffset20in40[core][2] = PDOFFSET(pi, rstr_pdoffset20in40m5gcore3, 1);
		   pwrdet->pdoffset20in40[core][3] = PDOFFSET(pi, rstr_pdoffset20in40m5gcore3, 2);
		   pwrdet->pdoffset20in40[core][4] = PDOFFSET(pi, rstr_pdoffset20in40m5gcore3_1, 0);
		   pwrdet->pdoffset20in40[core][5] = PDOFFSET(pi, rstr_pdoffset20in40m5gcore3_1, 1);
		   pwrdet->pdoffset20in80[core][1] = PDOFFSET(pi, rstr_pdoffset20in80m5gcore3, 0);
		   pwrdet->pdoffset20in80[core][2] = PDOFFSET(pi, rstr_pdoffset20in80m5gcore3, 1);
		   pwrdet->pdoffset20in80[core][3] = PDOFFSET(pi, rstr_pdoffset20in80m5gcore3, 2);
		   pwrdet->pdoffset20in80[core][4] = PDOFFSET(pi, rstr_pdoffset20in80m5gcore3_1, 0);
		   pwrdet->pdoffset20in80[core][5] = PDOFFSET(pi, rstr_pdoffset20in80m5gcore3_1, 1);
		   pwrdet->pdoffset40in80[core][1] = PDOFFSET(pi, rstr_pdoffset40in80m5gcore3, 0);
		   pwrdet->pdoffset40in80[core][2] = PDOFFSET(pi, rstr_pdoffset40in80m5gcore3, 1);
		   pwrdet->pdoffset40in80[core][3] = PDOFFSET(pi, rstr_pdoffset40in80m5gcore3, 2);
		   pwrdet->pdoffset40in80[core][4] = PDOFFSET(pi, rstr_pdoffset40in80m5gcore3_1, 0);
		   pwrdet->pdoffset40in80[core][5] = PDOFFSET(pi, rstr_pdoffset40in80m5gcore3_1, 1);
		}
	    }

	    pi->phy_tempsense_option = (uint8) PHY_GETINTVAR(pi, rstr_tempoption);
	    pi->phy_tempsense_offset = (int8)PHY_GETINTVAR(pi, rstr_tempoffset);
	    if (pi->phy_tempsense_offset < 0) {
		pi->phy_tempsense_offset = 0;
	    } else if (pi->phy_tempsense_offset != 0) {
		if (pi->phy_tempsense_offset >
		    (ACPHY_SROM_TEMPSHIFT + ACPHY_SROM_MAXTEMPOFFSET)) {
		    pi->phy_tempsense_offset = ACPHY_SROM_MAXTEMPOFFSET;
		} else if (pi->phy_tempsense_offset < (ACPHY_SROM_TEMPSHIFT +
		                                       ACPHY_SROM_MINTEMPOFFSET)) {
		    pi->phy_tempsense_offset = ACPHY_SROM_MINTEMPOFFSET;
		} else {
		    pi->phy_tempsense_offset -= ACPHY_SROM_TEMPSHIFT;
		}
	    }

	    /* For ACPHY, if the SROM contains a bogus value, then tempdelta
	     * will default to ACPHY_DEFAULT_CAL_TEMPDELTA. If the SROM contains
	     * a valid value, then the default will be overwritten with this value
	     */
	    wlc_phy_read_tempdelta_settings(pi, ACPHY_CAL_MAXTEMPDELTA);
	}
	return TRUE;
}

bool
BCMATTACHFN(wlc_phy_txpwr_srom8_read)(phy_info_t *pi)
{

	/* read in antenna-related config */
	pi->antswitch = (uint8) PHY_GETINTVAR(pi, rstr_antswitch);
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, rstr_aa2g);

#ifdef BAND5G
	pi->aa5g = (uint8) PHY_GETINTVAR(pi, rstr_aa5g);
#endif /* BAND5G */

	/* read in FEM stuff */
	pi->fem2g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos2g);
	pi->fem2g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain2g);
	pi->fem2g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange2g);
	pi->fem2g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso2g);
	pi->fem2g->antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl2g);

#ifdef BAND5G
	pi->fem5g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos5g);
	pi->fem5g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain5g);
	pi->fem5g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange5g);
	pi->fem5g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso5g);

	/* If antswctl5g entry exists, use it.
	 * Fallback to antswctl2g value if 5g entry does not exist.
	 * Previous code used 2g value only, thus...
	 * this is a WAR for any legacy NVRAMs that only had a 2g entry.
	 */
	pi->fem5g->antswctrllut = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_antswctl5g,
		PHY_GETINTVAR(pi, rstr_antswctl2g));
#endif /* BAND5G */

	if (PHY_GETVAR(pi, rstr_elna2g)) {
		/* extlnagain2g entry exists, so use it. */
		if (ISNPHY(pi)) {
			pi->u.pi_nphy->elna2g = (uint8)PHY_GETINTVAR(pi, rstr_elna2g);
		}
	}
#ifdef BAND5G
	if (PHY_GETVAR(pi, rstr_elna5g)) {
		/* extlnagain5g entry exists, so use it. */
		if (ISNPHY(pi)) {
			pi->u.pi_nphy->elna5g = (uint8)PHY_GETINTVAR(pi, rstr_elna5g);
		}
	}
#endif /* BAND5G */

	/* srom_fem2/5g.extpagain changed */
	wlc_phy_txpower_ipa_upd(pi);

	pi->phy_tempsense_offset = (int8)PHY_GETINTVAR(pi, rstr_tempoffset);
	if (pi->phy_tempsense_offset != 0) {
		if (pi->phy_tempsense_offset >
			(NPHY_SROM_TEMPSHIFT + NPHY_SROM_MAXTEMPOFFSET)) {
			pi->phy_tempsense_offset = NPHY_SROM_MAXTEMPOFFSET;
		} else if (pi->phy_tempsense_offset < (NPHY_SROM_TEMPSHIFT +
			NPHY_SROM_MINTEMPOFFSET)) {
			pi->phy_tempsense_offset = NPHY_SROM_MINTEMPOFFSET;
		} else {
			pi->phy_tempsense_offset -= NPHY_SROM_TEMPSHIFT;
		}
	}

	wlc_phy_read_tempdelta_settings(pi, NPHY_CAL_MAXTEMPDELTA);

	/* Power per Rate */
	wlc_phy_txpwr_srom8_read_ppr(pi);

	return TRUE;

}

static const char BCMATTACHDATA(rstr_maxp5ga3)[] = "maxp5ga3";
static const char BCMATTACHDATA(rstr_maxp5gla3)[] = "maxp5gla3";
static const char BCMATTACHDATA(rstr_pa5gw0a3)[] = "pa5gw0a3";
static const char BCMATTACHDATA(rstr_pa5glw0a3)[] = "pa5glw0a3";
static const char BCMATTACHDATA(rstr_pa5gw1a3)[] = "pa5gw1a3";
static const char BCMATTACHDATA(rstr_pa5glw1a3)[] = "pa5glw1a3";
static const char BCMATTACHDATA(rstr_pa5gw2a3)[] = "pa5gw2a3";
static const char BCMATTACHDATA(rstr_pa5glw2a3)[] = "pa5glw2a3";
static const char BCMATTACHDATA(rstr_maxp5gha3)[] = "maxp5gha3";
static const char BCMATTACHDATA(rstr_pa5ghw0a3)[] = "pa5ghw0a3";
static const char BCMATTACHDATA(rstr_pa5ghw1a3)[] = "pa5ghw1a3";
static const char BCMATTACHDATA(rstr_pa5ghw2a3)[] = "pa5ghw2a3";

/* */
bool
BCMATTACHFN(wlc_phy_txpwr_srom9_read)(phy_info_t *pi)
{
	srom_pwrdet_t	*pwrdet  = pi->pwrdet;
#ifdef BAND5G
	uint32 offset_40MHz[PHY_MAX_CORES] = {0};
#endif /* BAND5G */
	int b;

	if (PHY_GETVAR(pi, rstr_elna2g)) {
		/* extlnagain2g entry exists, so use it. */
		if (ISNPHY(pi)) {
			pi->u.pi_nphy->elna2g = (uint8)PHY_GETINTVAR(pi, rstr_elna2g);
		}
	}
#ifdef BAND5G
	if (PHY_GETVAR(pi, rstr_elna5g)) {
		/* extlnagain5g entry exists, so use it. */
		if (ISNPHY(pi)) {
			pi->u.pi_nphy->elna5g = (uint8)PHY_GETINTVAR(pi, rstr_elna5g);
		}
	}
#endif /* BAND5G */

	/* read in antenna-related config */
	pi->antswitch = (uint8) PHY_GETINTVAR(pi, rstr_antswitch);
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, rstr_aa2g);
#ifdef BAND5G
	pi->aa5g = (uint8) PHY_GETINTVAR(pi, rstr_aa5g);
#endif /* BAND5G */

	/* read in FEM stuff */
	pi->fem2g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos2g);
	pi->fem2g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain2g);
	pi->fem2g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange2g);
	pi->fem2g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso2g);
	pi->fem2g->antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl2g);

#ifdef BAND5G
	pi->fem5g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos5g);
	pi->fem5g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain5g);
	pi->fem5g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange5g);
	pi->fem5g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso5g);
	pi->fem5g->antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl5g);
#endif /* BAND5G */

	/* srom_fem2/5g.extpagain changed */
	if (ISNPHY(pi))
		wlc_phy_txpower_ipa_upd(pi);

#ifdef BAND5G
	offset_40MHz[PHY_CORE_0] = PHY_GETINTVAR(pi, rstr_pa2gw0a3);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		offset_40MHz[PHY_CORE_1] = PHY_GETINTVAR(pi, rstr_pa2gw1a3);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		offset_40MHz[PHY_CORE_2] = PHY_GETINTVAR(pi, rstr_pa2gw2a3);
#endif /* BAND5G */

	/* read pwrdet params for each band/subband */
	for (b = 0; b < NUMSUBBANDS(pi); b++) {
		switch (b) {
		case WL_CHAN_FREQ_RANGE_2G: /* 0 */
			/* 2G band */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw2a0);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = 0;

			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp2ga1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a1);
				pwrdet->pwr_offset40[PHY_CORE_1][b] = 0;
			}
			if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp2ga2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a2);
				pwrdet->pwr_offset40[PHY_CORE_2][b] = 0;
			}
			break;
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5G_BAND0: /* 1 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gla0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw2a0);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = wlc_phy_txpwr40Moffset_srom_convert(
				(offset_40MHz[0] & PWROFFSET40_MASK_0)
					>> PWROFFSET40_SHIFT_0);
			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gla1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a1);
				pwrdet->pwr_offset40[PHY_CORE_1][b] =
					wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[1] & PWROFFSET40_MASK_0)
						>> PWROFFSET40_SHIFT_0);
			}
			if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gla2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a2);
				pwrdet->pwr_offset40[PHY_CORE_2][b] =
					wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[2] & PWROFFSET40_MASK_0)
						>> PWROFFSET40_SHIFT_0);
			}
			break;

		case WL_CHAN_FREQ_RANGE_5G_BAND1: /* 2 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw2a0);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = wlc_phy_txpwr40Moffset_srom_convert(
				(offset_40MHz[0] & PWROFFSET40_MASK_1) >> PWROFFSET40_SHIFT_1);
			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5ga1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a1);
				pwrdet->pwr_offset40[PHY_CORE_1][b] =
					wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[1] & PWROFFSET40_MASK_1)
						>> PWROFFSET40_SHIFT_1);
			}

			if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5ga2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a2);
				pwrdet->pwr_offset40[PHY_CORE_2][b] =
					wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[2] & PWROFFSET40_MASK_1)
						>> PWROFFSET40_SHIFT_1);
			}
			break;

		case WL_CHAN_FREQ_RANGE_5G_BAND2: /* 3 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);
			pwrdet->pwr_offset40[0][b] = wlc_phy_txpwr40Moffset_srom_convert(
				(offset_40MHz[PHY_CORE_0] & PWROFFSET40_MASK_2) >>
					PWROFFSET40_SHIFT_2);
			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
				pwrdet->pwr_offset40[1][b] = wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[PHY_CORE_1] & PWROFFSET40_MASK_2) >>
						PWROFFSET40_SHIFT_2);
			}

			if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
				pwrdet->pwr_offset40[2][b] =
					wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[PHY_CORE_2] & PWROFFSET40_MASK_2)
						>> PWROFFSET40_SHIFT_2);
			}
			break;

		case WL_CHAN_FREQ_RANGE_5G_BAND3: /* 4 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga3);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw0a3);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw1a3);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw2a3);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = wlc_phy_txpwr40Moffset_srom_convert(
				(offset_40MHz[0] & PWROFFSET40_MASK_3) >> PWROFFSET40_SHIFT_3);
			if (PHYCORENUM(pi->pubpi.phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gla3);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a3);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a3);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a3);
				pwrdet->pwr_offset40[PHY_CORE_1][b] =
					wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[1] & PWROFFSET40_MASK_3)
						>> PWROFFSET40_SHIFT_3);
			}

			if (PHYCORENUM(pi->pubpi.phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gha3);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a3);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a3);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a3);
				pwrdet->pwr_offset40[PHY_CORE_2][b] =
					wlc_phy_txpwr40Moffset_srom_convert(
					(offset_40MHz[2] & PWROFFSET40_MASK_3)
						>> PWROFFSET40_SHIFT_3);
			}
			break;
#endif /* BAND5G */
		}
	}
	wlc_phy_txpwr_srom9_read_ppr(pi);
	return TRUE;
}

void
wlc_phy_antsel_init(wlc_phy_t *ppi, bool lut_init)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISNPHY(pi))
		wlc_phy_antsel_init_nphy(ppi, lut_init);
}

#define BTCX_FLUSH_WAIT_MAX_MS  500
void
wlc_btcx_override_enable(phy_info_t *pi)
{
	/* This is required only for 2G operation. No BTCX in 5G */
	if ((pi->sh->machwcap & MCAP_BTCX) &&
		CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC));
		wlapi_coex_flush_a2dp_buffers(pi->sh->physhim);

		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, &pi->regs->PHYREF_BTCX_CTRL, BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force WLAN antenna and priority */
		OR_REG(pi->sh->osh, &pi->regs->PHYREF_BTCX_TRANS_CTRL,
			BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL);

		/* SWWLAN-30288 For 4324x ucode is not setting clb_rf_sw_ctrl_mask with value
		 * needed for WLAN to  have control whenever BT->WLAN priority switch happens,
		 * so forcing it use correct value
		 */
		if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3)) {
			pi->saved_clb_sw_ctrl_mask = phy_utils_read_phyreg(pi,
			NPHY_REV19_clb_rf_sw_ctrl_mask_ctrl);
			phy_utils_write_phyreg(pi, NPHY_REV19_clb_rf_sw_ctrl_mask_ctrl,
				LCNXN_SWCTRL_MASK_DEFAULT);
			}
	}
}

void
wlc_phy_btcx_override_disable(phy_info_t *pi)
{
	if ((pi->sh->machwcap & MCAP_BTCX) &&
		CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC));

		/* SWWLAN-30288 For 4324x ucode is not setting clb_rf_sw_ctrl_mask with value
		 * needed for WLAN to have control whenever BT->WLAN priority switch happens
		 * so it was forced to wlan prefered value, so restoring the correct value
		 * for BT to have control
		 */
		if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3))
			phy_utils_write_phyreg(pi, NPHY_REV19_clb_rf_sw_ctrl_mask_ctrl,
				pi->saved_clb_sw_ctrl_mask);

		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, &pi->regs->PHYREF_BTCX_CTRL, BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force BT priority */
		AND_REG(pi->sh->osh, &pi->regs->PHYREF_BTCX_TRANS_CTRL,
			~(BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL));
	}
}

bool
wlc_phy_no_cal_possible(phy_info_t *pi)
{
	return (SCAN_RM_IN_PROGRESS(pi));
}

#if !defined(EFI)

#if PHY_TSSI_CAL_DBG_EN
static void
print_int64(int64 *a)
{
	void *llp = a;
	uint32 *lp_low = (uint32 *)llp;
	uint32 *lp_high = lp_low + 1;
	printf("0x%08x%08x ", *lp_high, *lp_low);
}
#endif // endif

#if PHY_TSSI_CAL_DBG_EN
/*
 * matrix print
 * dimensions a (m x n)
 * name - matrix name
 */
static void
mat_print(int64 *a, int m, int n, const char *name)
{
	int i, j;

	printf("\n%s\n", name);
	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++)
			print_int64(a + (i * n) + j);
		printf("\n");
	}
}
#else /* PHY_TSSI_CAL_DBG_EN */
static void
mat_print(int64 *a, int m, int n, const char *name)
{
}
#endif /* PHY_TSSI_CAL_DBG_EN */

/* ================================================================
function [b0 b1 a1] = ratmodel_paparams_fix64(n, P)

%This is the algorithm used for curve fitting to get PA Params
%n: Adjusted TSSI values
%P: Power in qdBm

q1 = 4;
n = reshape(n, length(n), 1);
P = (reshape(P, length(P), 1)*q1);
P = round(P);

rho = ones(length(n), 3)*q1;
rho(:,2) = n*q1;
rho(:,3) = -n.*P;
rho = (rho./q1);
rho = round(rho);

C1 = rho' * rho;

a11 = C1(1,1); a12 = C1(1,2); a13 = C1(1,3);
a21 = C1(2,1); a22 = C1(2,2); a23 = C1(2,3);
a31 = C1(3,1); a32 = C1(3,2); a33 = C1(3,3);

C2_calc = [a22*a33 - a32*a23  a13*a32 - a12*a33  a12*a23 - a13*a22
		a23*a31 - a21*a33  a11*a33 - a13*a31  a13*a21 - a11*a23
		a21*a32 - a31*a22  a12*a31 - a11*a32  a11*a22 - a12*a21];

det_C1 = a11*a22*a33 + a12*a23*a31 + a13*a21*a32
		- a11*a23*a32 - a12*a21*a33 - a13*a22*a31;

C3 = C2_calc * rho';

C4 = C3 * P

C4 = round(C4./q1)

C = C4./det_C1;

b0=round(2^8*C(1)) ;
b1=round(2^12*C(2));
a1=round(2^15*C(3));

return;
*/
static void
ratmodel_paparams_fix64(ratmodel_paparams_t* rsd, int m)
{
	int i, j, n, q;
	int64 *a, temp;

	phy_utils_mat_rho((int64 *)(&rsd->n), (int64 *)(&rsd->p),
		(int64 *)(&rsd->rho), m);
	mat_print((int64 *)(&rsd->rho), m, 3, "rho");

	phy_utils_mat_transpose((int64 *)(&rsd->rho),
		(int64 *)(&rsd->rho_t), m, 3);
	mat_print((int64 *)(&rsd->rho_t), 3, m, "rho_t");

	phy_utils_mat_mult((int64*)(&rsd->rho_t), (int64 *)(&rsd->rho),
		(int64*)(&rsd->c1), 3, m, 3);
	mat_print((int64 *)(&rsd->c1), 3, 3, "c1");

	phy_utils_mat_inv_prod_det((int64 *)(&rsd->c1),
		(int64 *)(&rsd->c2_calc));
	mat_print((int64 *)(&rsd->c2_calc), 3, 3, "c2_calc");

	phy_utils_mat_det((int64 *)(&rsd->c1), (int64 *)(&rsd->det_c1));

#if PHY_TSSI_CAL_DBG_EN
	printf("\ndet_c1 = ");
	print_int64(&rsd->det_c1);
	printf("\n");
#endif // endif

	phy_utils_mat_mult((int64*)(&rsd->c2_calc), (int64 *)(&rsd->rho_t),
		(int64*)(&rsd->c3), 3, 3, m);
	mat_print((int64 *)(&rsd->c3), 3, m, "c3");

	phy_utils_mat_mult((int64*)(&rsd->c3), (int64 *)(&rsd->p),
		(int64*)(&rsd->c4), 3, m, 1);

	m = 3; n = 1; q = 2;
	a = (int64*)(&rsd->c4);
	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++) {
			temp = *(a + (i * n) + j);
			temp = (temp + (int64)(1 << (q-1)));
			temp = temp >> q;
			*(a + (i * n) + j) = temp;
		}

	mat_print((int64 *)(&rsd->c4), 3, 1, "c4");
}

int
wlc_phy_tssi_cal(phy_info_t *pi)
{
	uint16 count;
	count = tssi_cal_sweep(pi);
	ratmodel_paparams_fix64(&pi->ptssi_cal->rsd, count);
	return 0;
}

static uint16
tssi_cal_sweep(phy_info_t *pi)
{

	uint16 i, k = 0;

	uint16 count = 0;
	int8 *des_pwr = NULL;
	uint8 *adj_tssi = NULL;
	int *sort_pwr = NULL, avg_pwr;
	uint8 *sort_pwr_cnt = NULL;
	int16 MIN_PWR = 32; /* 8dBm */
	int16 MAX_PWR = 72; /* 18dBm */

	int64* tssi = pi->ptssi_cal->rsd.n;
	int64* pwr = pi->ptssi_cal->rsd.p;

	des_pwr = (int8*)MALLOC(pi->sh->osh, sizeof(int8)* 80 * MAX_NUM_ANCHORS);
	if (des_pwr == NULL)
		goto cleanup;

	adj_tssi = (uint8*)MALLOC(pi->sh->osh, sizeof(uint8) * 80 * MAX_NUM_ANCHORS);
	if (adj_tssi == NULL)
		goto cleanup;

	sort_pwr = (int*)MALLOC(pi->sh->osh, sizeof(int)*128);
	if (sort_pwr == NULL)
		goto cleanup;

	sort_pwr_cnt = (uint8*)MALLOC(pi->sh->osh, sizeof(uint8)*128);
	if (sort_pwr_cnt == NULL)
		goto cleanup;

	if (pi->pi_fptr.tssicalsweep)
		count = (*pi->pi_fptr.tssicalsweep)(pi, des_pwr, adj_tssi);
	else
		goto cleanup;

	for (i = 0; i < 128; i++) {
		sort_pwr[i] = 0xffffffff;
		sort_pwr_cnt[i] = 0;
	}

	for (i = 0; i < count; i++) {
		if (sort_pwr[adj_tssi[i]] == 0xffffffff)
			sort_pwr[adj_tssi[i]] = des_pwr[i];
		else {
			sort_pwr[adj_tssi[i]] += des_pwr[i];
		}
		sort_pwr_cnt[adj_tssi[i]]++;
	}

	k = 0;
	for (i = 0; i < 128; i++) {
		if (sort_pwr[i] != 0xffffffff) {
			avg_pwr =  sort_pwr[i]/sort_pwr_cnt[i];
			if ((avg_pwr >= MIN_PWR) && (avg_pwr <= MAX_PWR)) {
				tssi[k] = (int64) i;
				pwr[k] =  (int64) avg_pwr;
				k++;
			}
		}
	}

#if PHY_TSSI_CAL_DBG_EN
	printf("TSSI\tPWR, k = %d\n", k);
	for (i = 0; i < k; i++) {
		print_int64(&tssi[i]);
		printf("\t\t");
		print_int64(&pwr[i]);
		printf("\n");
	}
#endif // endif

cleanup:
	if (des_pwr)
		MFREE(pi->sh->osh, des_pwr, sizeof(int8) * 80 * MAX_NUM_ANCHORS);
	if (adj_tssi)
		MFREE(pi->sh->osh, adj_tssi, sizeof(uint8) * 80 * MAX_NUM_ANCHORS);
	if (sort_pwr)
		MFREE(pi->sh->osh, sort_pwr, sizeof(int)*128);
	if (sort_pwr_cnt)
		MFREE(pi->sh->osh, sort_pwr_cnt, sizeof(uint8)*128);

	return k;
}

#endif /* #if !defined(EFI) */

/* a simple implementation of gcd(greatest common divisor)
 * assuming argument 1 is bigger than argument 2, both of them
 * are positive numbers.
 */
uint32
wlc_phy_gcd(uint32 bigger, uint32 smaller)
{
	uint32 remainder;

	do {
		remainder = bigger % smaller;
		if (remainder) {
			bigger = smaller;
			smaller = remainder;
		} else {
			return smaller;
		}
	} while (TRUE);
}

#if defined(LCNCONF) || defined(LCN40CONF)
void
wlc_phy_get_paparams_for_band(phy_info_t *pi, int32 *a1, int32 *b0, int32 *b1)
{
	/* On lcnphy, estPwrLuts0/1 table entries are in S6.3 format */
	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
	case WL_CHAN_FREQ_RANGE_2G:
			/* 2.4 GHz */
			ASSERT((pi->txpa_2g[0] != -1) && (pi->txpa_2g[1] != -1) &&
				(pi->txpa_2g[2] != -1));
			*b0 = pi->txpa_2g[0];
			*b1 = pi->txpa_2g[1];
			*a1 = pi->txpa_2g[2];
			break;
#ifdef BAND5G
	case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			ASSERT((pi->txpa_5g_low[0] != -1) &&
				(pi->txpa_5g_low[1] != -1) &&
				(pi->txpa_5g_low[2] != -1));
			*b0 = pi->txpa_5g_low[0];
			*b1 = pi->txpa_5g_low[1];
			*a1 = pi->txpa_5g_low[2];
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			ASSERT((pi->txpa_5g_mid[0] != -1) &&
				(pi->txpa_5g_mid[1] != -1) &&
				(pi->txpa_5g_mid[2] != -1));
			*b0 = pi->txpa_5g_mid[0];
			*b1 = pi->txpa_5g_mid[1];
			*a1 = pi->txpa_5g_mid[2];
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			ASSERT((pi->txpa_5g_hi[0] != -1) &&
				(pi->txpa_5g_hi[1] != -1) &&
				(pi->txpa_5g_hi[2] != -1));
			*b0 = pi->txpa_5g_hi[0];
			*b1 = pi->txpa_5g_hi[1];
			*a1 = pi->txpa_5g_hi[2];
			break;
#endif /* BAND5G */
		default:
			ASSERT(FALSE);
			break;
	}
	return;
}
#endif /* lcn40 || lcn */

/* --------------------------------------------- */
/* this will evetually be moved to lcncommon.c */
phy_info_lcnphy_t *
wlc_phy_getlcnphy_common(phy_info_t *pi)
{
	if (ISLCNPHY(pi))
		return pi->u.pi_lcnphy;
	else if (ISLCN40PHY(pi))
		return (phy_info_lcnphy_t *)pi->u.pi_lcn40phy;
	else {
		ASSERT(FALSE);
		return NULL;
	}
}

uint16
wlc_txpwrctrl_lcncommon(phy_info_t *pi)
{
	if (ISLCNPHY(pi))
		return LCNPHY_TX_PWR_CTRL_HW;
	else if (ISLCN40PHY(pi))
		return LCN40PHY_TX_PWR_CTRL_HW;
	else {
		ASSERT(FALSE);
		return 0;
	}
}

#ifdef ENABLE_FCBS

/* Fast Channel/Band Switch (FCBS) engine functions */

/* Function prototype */
void wlc_phy_fcbs_read_regs_tbls(phy_info_t *pi, int chanidx, chanspec_t chanspec);

bool wlc_phy_is_fcbs_chan(phy_info_t *pi, chanspec_t chanspec, int *chanidx_ptr)
{
	int chanidx;
	bool retval = FALSE;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (pi->phy_fcbs->initialized[chanidx]) {
			if (pi->phy_fcbs->chanspec[chanidx] == chanspec) {
				*chanidx_ptr = chanidx;
				retval = TRUE;
				break;
			}
		}
	}

	return retval;
}

bool wlc_phy_is_fcbs_pending(phy_info_t *pi, chanspec_t chanspec, int *chanidx_ptr)
{
	int chanidx;
	bool retval = FALSE;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (!pi->phy_fcbs->initialized[chanidx]) {
			if ((pi->phy_fcbs->chanspec[chanidx] == chanspec) ||
				((pi->phy_fcbs->chanspec[chanidx] == 0xFFFF))) {
				*chanidx_ptr = chanidx;
				retval = TRUE;
				break;
			}
		}
	}

	return retval;
}

void wlc_phy_fcbs_exit(wlc_phy_t *ppi)
{
	int chanidx;
	phy_info_t *pi = (phy_info_t*)ppi;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		pi->phy_fcbs->chanspec[chanidx] = 0;
		pi->phy_fcbs->initialized[chanidx] = FALSE;
	}
}

bool wlc_phy_fcbs_uninit(wlc_phy_t *ppi, chanspec_t chanspec)
{
	int chanidx;
	phy_info_t *pi = (phy_info_t*)ppi;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (pi->phy_fcbs->chanspec[chanidx] == chanspec) {
			pi->phy_fcbs->chanspec[chanidx] = 0;
			pi->phy_fcbs->initialized[chanidx] = FALSE;
			PHY_INFORM(("%s: Uninitting channel %d on idx %d\n",
				__FUNCTION__, CHSPEC_CHANNEL(chanspec), chanidx));

			return TRUE;
		}
	}
	PHY_ERROR(("%s: Cannot uninit unused fcbs context!! 0x%x, %d\n",
		__FUNCTION__, chanspec, CHSPEC_CHANNEL(chanspec)));

	return FALSE;
}

void wlc_phy_fcbs_read_regs_tbls(phy_info_t *pi, int chanidx, chanspec_t chanspec)
{
	/* TBD */
}

/* using chanspec of 0 cancels an arm request */
/* Might call mac_suspend() */
bool wlc_phy_fcbs_arm(wlc_phy_t *ppi, chanspec_t chanspec, int chanidx)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));
	ASSERT(!wf_chspec_malformed(chanspec) || chanspec == 0 || chanspec == 0xff);

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (pi->phy_fcbs->chanspec[chanidx] == chanspec) {
			PHY_ERROR(("%s: channel %d is already armed.\n",
				__FUNCTION__, CHSPEC_CHANNEL(chanspec)));
			return TRUE;
		}
	}

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
			if ((!pi->phy_fcbs->chanspec[chanidx]))
				break;
	}

	if (chanidx >= MAX_FCBS_CHANS) {
		PHY_ERROR(("%s: ERROR: No Free Entry!!\n", __FUNCTION__));
		return FALSE;
	}

	/* If we lucky enough to be on channel, call init directly */
	if ((chanspec == pi->radio_chanspec) && IS_FCBS(pi) &&
	    !(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi)) &&
		(chanidx == wlc_phy_channelindicator_obtain_acphy(pi))) {
		PHY_ERROR(("%s: Already on channel %d, call fcbs_init immediatly\n",
			__FUNCTION__, CHSPEC_CHANNEL(pi->radio_chanspec)));
		pi->phy_fcbs->chanspec[chanidx] = chanspec;
		return wlc_phy_fcbs_init(ppi, chanidx);
	} else {
		PHY_INFORM(("%s: Arming channel %d\n", __FUNCTION__, CHSPEC_CHANNEL(chanspec)));
		pi->phy_fcbs->chanspec[chanidx] = chanspec;
		pi->phy_fcbs->initialized[chanidx] = FALSE;
		return TRUE;
	}
}

bool wlc_phy_hw_fcbs_init(wlc_phy_t *ppi, int chanidx)
{
	fcbsinitfn_t fcbs_init = NULL;
	phy_info_t *pi = (phy_info_t*)ppi;

	if (chanidx >= MAX_FCBS_CHANS) {
		PHY_ERROR(("%s: ERROR: Out of empty contexts!!\n", __FUNCTION__));
		return FALSE;
	}

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

	PHY_INFORM(("%s: Initting slot %d, channel %d\n",
		__FUNCTION__, chanidx, CHSPEC_CHANNEL(pi->radio_chanspec)));

	pi->phy_fcbs->chanspec[chanidx] = pi->radio_chanspec;

	/* Get the pointer to the PHY specfic FCBS init function */
	fcbs_init = pi->pi_fptr.fcbsinit;

	if (!fcbs_init) {
		PHY_ERROR(("%s: Missing fcbsinit pointer\n", __FUNCTION__));
		pi->phy_fcbs->initialized[chanidx] = FALSE;
		ASSERT(fcbs_init);
		return TRUE;
	}
	/* phy-specific fcbs_init function needs to just initialize the pointers to the
	 * lists of regs/tbls that need to be saved into the FCBS TBL
	 */
	(*fcbs_init)(pi, chanidx, pi->radio_chanspec);
	/* actual reading and saving of all the required regs/btls is done in this fctn */
	pi->phy_fcbs->initialized[chanidx] =
		wlc_phy_hw_fcbs_init_chanidx((wlc_phy_t*)pi, chanidx);

	return TRUE;
}

bool wlc_phy_hw_fcbs_init_chanidx(wlc_phy_t *ppi, int chanidx)
{
	/* compiling and writing all FCBS data */

	uint16 *p_fcbs_tbl_data;
	fcbs_radioreg_core_list_entry *radioreg_list_ptr;
	uint16 *reg_list_ptr;
	fcbs_phytbl_list_entry *tbl_list_ptr;
	phy_info_t *pi = (phy_info_t*)ppi;
	int length;
	uint8 stall_val;

	p_fcbs_tbl_data = pi->phy_fcbs->hw_fcbs_tbl_data;
	length = 0;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	for (radioreg_list_ptr = pi->phy_fcbs->fcbs_radioreg_list;
			radioreg_list_ptr->regaddr != 0xFFFF; radioreg_list_ptr++) {
		*p_fcbs_tbl_data = (radioreg_list_ptr->regaddr) |
			((radioreg_list_ptr->core_info & 0x7) << FCBS_TBL_RADIOREG_CORE_SHIFT) |
			FCBS_TBL_INST_INDICATOR;
		p_fcbs_tbl_data += (chanidx + 1);
		*p_fcbs_tbl_data = phy_utils_read_radioreg(pi, radioreg_list_ptr->regaddr);
		p_fcbs_tbl_data += (MAX_FCBS_CHANS - chanidx);
	}
	*p_fcbs_tbl_data = 0xFFFF;
	p_fcbs_tbl_data++;

	for (reg_list_ptr = pi->phy_fcbs->fcbs_phyreg_list;
			*reg_list_ptr != 0xFFFF; reg_list_ptr++) {
		*p_fcbs_tbl_data = *reg_list_ptr | FCBS_TBL_INST_INDICATOR;
		p_fcbs_tbl_data += (chanidx + 1);
		*p_fcbs_tbl_data = phy_utils_read_phyreg(pi, *reg_list_ptr);
		p_fcbs_tbl_data += (MAX_FCBS_CHANS - chanidx);
	}
	*p_fcbs_tbl_data = 0xFFFF;
	p_fcbs_tbl_data++;

	if (!pi->phy_fcbs->FCBS_ucode) {
		for (tbl_list_ptr = pi->phy_fcbs->fcbs_phytbl16_list;
		    tbl_list_ptr->tbl_id != 0xFFFF; tbl_list_ptr++) {
			int num_entries;
			int tbl_idx;
			uint16 fcbs_phytbl_copy[20];

			num_entries = tbl_list_ptr->num_entries;
			/* Setup the information tuple */
			*p_fcbs_tbl_data++ = tbl_list_ptr->tbl_id | FCBS_TBL_INST_INDICATOR;
			*p_fcbs_tbl_data++ = tbl_list_ptr->tbl_offset;
			*p_fcbs_tbl_data++ = tbl_list_ptr->tbl_offset +
				tbl_list_ptr->num_entries - 1;

			stall_val = (phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev)) &
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev))
				>> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
				1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));
			wlc_phy_table_read_acphy(pi, tbl_list_ptr->tbl_id,
			     tbl_list_ptr->num_entries, tbl_list_ptr->tbl_offset,
			     16, fcbs_phytbl_copy);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
				stall_val <<
				ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));

			/* valA and valB are interleved and we can align the 1st element correctly
			 * and then increment by 2 to write the subsequent entries of the same chan
			 */
			p_fcbs_tbl_data += chanidx;
			for (tbl_idx = 0; tbl_idx < num_entries; tbl_idx++) {
				*p_fcbs_tbl_data = fcbs_phytbl_copy[tbl_idx];
				p_fcbs_tbl_data += MAX_FCBS_CHANS;
			}
			p_fcbs_tbl_data -= chanidx;
			/* Setup the information tuple */
		}
	} else {
		uint16 * p_phytbl16_buf;
		int offset, len, phytbl_idx;
		uint16 num_entries;

		p_phytbl16_buf = pi->phy_fcbs->phytbl16_buf[chanidx];
		phytbl_idx = 0;
		/* radio regs are being handle through HW FCBS  */
		if (chanidx == FCBS_CHAN_A) {
			pi->phy_fcbs->chan_cache_offset[chanidx] =
			    pi->phy_fcbs->cache_startaddr;
		}
		offset = pi->phy_fcbs->chan_cache_offset[chanidx];
		len = 0;
		pi->phy_fcbs->radioreg_cache_offset[chanidx] = offset;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_radioreg, (uint16)len);
		PHY_FCBS(("radio reg buf: start offset=%d, len=%d\n", offset, len));

		/* Store the 16-bit PHY table entries in the FCBS cache */
		offset += len;
		len = 0;
		for (tbl_list_ptr = pi->phy_fcbs->fcbs_phytbl16_list;
		    tbl_list_ptr->tbl_id != 0xFFFF; tbl_list_ptr++) {
			/* Setup the information tuple */
			p_phytbl16_buf[phytbl_idx++] = (tbl_list_ptr->tbl_id << 10) |
			    tbl_list_ptr->tbl_offset;
			p_phytbl16_buf[phytbl_idx++] = tbl_list_ptr->num_entries;
			num_entries = tbl_list_ptr->num_entries;

			stall_val = (phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev))
				& ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev))
				>> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
				1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));
			wlc_phy_table_read_acphy(pi, tbl_list_ptr->tbl_id,
			     tbl_list_ptr->num_entries, tbl_list_ptr->tbl_offset,
			     16, p_phytbl16_buf + phytbl_idx);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
				stall_val <<
				ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));

			phytbl_idx += tbl_list_ptr->num_entries;

			/* if we don't have even number of entries pad
			 * the buffer by 16-bits of zeros
			 */
			if (((num_entries/2) * 2) != num_entries) {
				p_phytbl16_buf[phytbl_idx++] = 0;
				num_entries += 1;
			}

			len += 4 + (num_entries * 2);
		}

		pi->phy_fcbs->phytbl16_cache_offset[chanidx] = offset;
		/* buffer has to end at 4-byte boundary in the RAM */
		if (((len/4) * 4) != len) {
			len += 2;
		}
		wlapi_bmac_write_template_ram(pi->sh->physhim,
		    offset, len, p_phytbl16_buf);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl16, (uint16)len);
		PHY_FCBS(("phytbl16: start offset = %d, len = %d\n", offset, len));

		/* 32-bit PHY tables entries handled through HW FCBS TBL */
		offset += len;
		len = 0;
		pi->phy_fcbs->phytbl32_cache_offset[chanidx] = offset;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl32, (uint16)len);
		PHY_FCBS(("phytbl32: start offset = %d, len = %d\n", offset, len));

		/* PHY regs handled through HW FCBS TBL */
		offset += len;
		len = 0;
		pi->phy_fcbs->phyreg_cache_offset[chanidx] = offset;

		pi->phy_fcbs->phyreg_buflen[chanidx] = len;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phyreg, (uint16)len);
		PHY_FCBS(("PHY reg buf: start offset=%d, len = %d\n", offset, len));

		if (chanidx == FCBS_CHAN_A) {
			/* Now that we have finished storing the
			 * cache for CHAN_A, we know the starting
			 * cache address for CHAN_B
			 */
			pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_B] = offset + len;
		}

		/* Clear the cache pointer in case it had a non-zero value
		 * before the init
		 */
		wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr, 0);

	}

	*p_fcbs_tbl_data = 0xFFFF;
	p_fcbs_tbl_data++;
	/* length gets number of array entries */
	length = (int)(p_fcbs_tbl_data - pi->phy_fcbs->hw_fcbs_tbl_data);
	stall_val = (phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev)) &
		ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev))
		>> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev);
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
		ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
		1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH,
	                         length, 0, 16, pi->phy_fcbs->hw_fcbs_tbl_data);
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi.phy_rev),
		ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi.phy_rev),
		stall_val << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi.phy_rev));
	/* FCBS has been written into FCBS tbl */

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
	return TRUE;
}

/* Might call mac_suspend() */
/* NOTE: chanidx isn't used anymore, leaving it for now to try it out */

bool wlc_phy_fcbs_init(wlc_phy_t *ppi, int chanidx)
{
	int offset, bphy_offset;
	int len, bphy_len;
	fcbsinitfn_t fcbs_init = NULL;
	phy_info_t *pi = (phy_info_t*)ppi;

#if defined(FCBS_GPIO_PROFILE)
	uint32 gpio_mask_val = 0x10000; /* CCTRL4331_BT_SHD0_ON_GPIO4 */
#endif // endif
	if (IS_FCBS(pi)) {
		return wlc_phy_hw_fcbs_init((wlc_phy_t*)pi, chanidx);
	}

#if defined(FCBS_GPIO_PROFILE)
	/* Use a GPIO to measure the FCBS time on an oscilloscope */
	if (chanidx == FCBS_CHAN_A) {
		/* Enable the GPIO */
		si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		           gpio_mask_val, 0);
	}
#endif /* FCBS_GPIO_PROFILE */

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

#ifdef LATER
	/* Chan A has to be initialized before Chan B */
	/* Why ?? */
	if ((chanidx == FCBS_CHAN_B) && (!pi->phy_fcbs->initialized[FCBS_CHAN_A])) {
		PHY_ERROR(("%s: Error: Chan A has to be initialized before Chan B\n",
			__FUNCTION__));
		return FALSE;
	}
#endif /* LATER */

	PHY_INFORM(("%s: Initting slot %d, channel %d\n",
		__FUNCTION__, chanidx, CHSPEC_CHANNEL(pi->radio_chanspec)));

	pi->phy_fcbs->chanspec[chanidx] = pi->radio_chanspec;

	/* Get the pointer to the PHY specfic FCBS init function */
	fcbs_init = pi->pi_fptr.fcbsinit;

	if (!fcbs_init) {
		PHY_ERROR(("%s: Missing fcbsinit pointer\n", __FUNCTION__));
		pi->phy_fcbs->initialized[chanidx] = FALSE;
		ASSERT(fcbs_init);
		return TRUE;
	}

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	phy_utils_phyreg_enter(pi);

	/* Clear the flag which is used by the PHY specific FCBS init code to indicate
	   if the FCBS engine should load the PHY tables and PHY/Radio registers
	*/
	pi->phy_fcbs->load_regs_tbls = FALSE;

	/* Call the PHY specific initialization function. This will update the phy_fcbs
	   fields with the starting h/w address of the on-chip RAM and the shmem locations
	   used by the driver to specify to the ucode, the various offsets within the
	   FCBS cache

	   This function will also set the pointers to the memory buffers used to
	   read the PHY tables and PHY/radio regs. It may also (optionally) load these
	   buffers with the contents of the PHY tables and PHY/Radio regs.
	*/
	pi->phy_fcbs->initialized[chanidx] = (*fcbs_init)(pi, chanidx, pi->radio_chanspec);

	phy_utils_phyreg_exit(pi);

	if (pi->phy_fcbs->initialized[chanidx] == TRUE) {
		if (chanidx == FCBS_CHAN_A) {
			/* Reset the BPHY update bit. This is used by
			 * the ucode to determine if BPHY registers
			 * need to be updated during FCBS
			 */
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, 0);

			pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_A] =
			    pi->phy_fcbs->cache_startaddr;

			/* This keeps a count of how many times we did a FCBS */
			pi->phy_fcbs->switch_count = 0;
		}

		if (pi->phy_fcbs->load_regs_tbls) {
			/* The PHY specific FCBS init routine wants
			 * the FCBS engine to read the PHY tables
			 * and PHY/Radio regs
			 */
			wlc_phy_fcbs_read_regs_tbls(pi, chanidx,
			    pi->radio_chanspec);
		}

		pi->phy_fcbs->curr_fcbs_chan = chanidx;

		/* Store the radio registers in the FCBS cache */
		offset = pi->phy_fcbs->chan_cache_offset[chanidx];
		/* Each cache entry contains an address and value */
		len = pi->phy_fcbs->num_radio_regs * 2 * sizeof(uint16);
		pi->phy_fcbs->radioreg_cache_offset[chanidx] = offset;
		wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
		    len, pi->phy_fcbs->radioreg_buf[chanidx]);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_radioreg, (uint16)len);
		PHY_FCBS(("radio reg buf: start offset=%d, len=%d\n", offset, len));

		/* Store the 16-bit PHY table entries in the FCBS cache */
		offset += len;
		len = pi->phy_fcbs->phytbl16_buflen;
		pi->phy_fcbs->phytbl16_cache_offset[chanidx] = offset;
		/* buffer has to end at 4-byte boundary in the RAM */
		if (((len/4) * 4) != len) {
			len += 2;
		}
		wlapi_bmac_write_template_ram(pi->sh->physhim,
		    offset, len, pi->phy_fcbs->phytbl16_buf[chanidx]);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl16, (uint16)len);
		PHY_FCBS(("phytbl16: start offset = %d, len = %d\n", offset, len));

		/* Store the 32-bit PHY table entries in the FCBS cache */
		offset += len;
		len = pi->phy_fcbs->phytbl32_buflen;
		pi->phy_fcbs->phytbl32_cache_offset[chanidx] = offset;
		wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
		    len, pi->phy_fcbs->phytbl32_buf[chanidx]);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl32, (uint16)len);
		PHY_FCBS(("phytbl32: start offset = %d, len = %d\n", offset, len));

		/* Store the PHY registers in the FCBS cache */
		offset += len;
		/* Each cache entry contains an address and value */
		len = pi->phy_fcbs->num_phy_regs * 2 * sizeof(uint16);
		pi->phy_fcbs->phyreg_cache_offset[chanidx] = offset;
		wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
		    len, pi->phy_fcbs->phyreg_buf[chanidx]);

		if (pi->phy_fcbs->num_bphy_regs[chanidx] > 0) {
			/* Store BPHY registers in the cache */
			bphy_offset = offset + len;
			bphy_len = pi->phy_fcbs->num_bphy_regs[chanidx] * sizeof(uint32);
			pi->phy_fcbs->bphyreg_cache_offset[chanidx] = bphy_offset;
			wlapi_bmac_write_template_ram(pi->sh->physhim, bphy_offset,
			    bphy_len, pi->phy_fcbs->bphyreg_buf[chanidx]);
			len += bphy_len;
		}

		pi->phy_fcbs->phyreg_buflen[chanidx] = len;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phyreg, (uint16)len);
		PHY_FCBS(("PHY reg buf: start offset=%d, len = %d\n", offset, len));

		if (chanidx == FCBS_CHAN_A) {
			/* Now that we have finished storing the
			 * cache for CHAN_A, we know the starting
			 * cache address for CHAN_B
			 */
			pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_B] = offset + len;
		}

		/* Clear the cache pointer in case it had a non-zero value
		 * before the init
		 */
		wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr, 0);

		wlapi_enable_mac(pi->sh->physhim);
	}
	return TRUE;
}

#define FCBS_MAX_ITERS 200
int wlc_phy_hw_fcbs(wlc_phy_t *ppi, int chanidx, bool set)
{
	uint16 ptr;
	int i;
	fcbspostfn_t post_fcbs = NULL;
	fcbsprefn_t pre_fcbs = NULL;
	fcbsfn_t fcbs = NULL;
	phy_info_t *pi = (phy_info_t*)ppi;

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));
	if (!pi->phy_fcbs->initialized[chanidx]) {
		return -1;
	}
	if (!set) {
		return pi->phy_fcbs->curr_fcbs_chan;
	}
	pi->phy_fcbs->curr_fcbs_chan = chanidx;

	/* About to start FCBS. Call the PHY specific pre-FCBS function, if any */
	if ((pre_fcbs = pi->pi_fptr.prefcbs)) {
		(*pre_fcbs)(pi, chanidx);
	}

	if ((fcbs = pi->pi_fptr.fcbs)) {
		(*fcbs)(pi, chanidx);
	}
	/* If we are currently in the 2G band and we are switching to 5G, tell
	   the ucode to turn the BPHY core off
	*/
	if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[pi->phy_fcbs->curr_fcbs_chan])) {
		if (CHSPEC_IS5G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_OFF);
		}
	} else {
		/* We are currently in 5G. If we are switching to the 2G band
		   tell the ucode to turn the BPHY core ON
		*/
		if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_ON);
		}
	}

	/* If one of the FCBS channel is in the 2G band and the other is in the
	   5G band, then the length of the PHY register cache will be different
	   due to the BPHY register values. In this case update the shmem
	   location with the appropriate value for the channel that we are
	   switching to
	*/
	if (pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_A] !=
		pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_B]) {
		wlapi_bmac_write_shm(pi->sh->physhim,
			pi->phy_fcbs->shmem_phyreg, (uint16)(pi->phy_fcbs->phyreg_buflen[chanidx]));
	}

	/* Now tell the ucode which cache (CHAN_A or CHAN_B) it should use */
	wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr,
	    (uint16) (pi->phy_fcbs->chan_cache_offset[chanidx]));

	/*
	Wait for ucode to write register tables:
	Using 4331 Rev B0
	Measuring 162 usecs from poking cache_ptr until it reads back 0.
	Each wlapi_bmac_read_shm() call takes 1.8 usecs.
	*/
	OSL_DELAY(100);
	ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	for (i = 0; ptr != 0 && i < FCBS_MAX_ITERS; i++) {
		OSL_DELAY(1);
		ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	}
	if (i >= FCBS_MAX_ITERS) {
		PHY_ERROR(("%s: Failed to complete ucode channel switch\n", __FUNCTION__));
	}
	ASSERT(i < FCBS_MAX_ITERS);

	pi->phy_fcbs->switch_count++;

	/* FCBS is done. Call the PHY specific post-FCBS function, if any */
	if ((post_fcbs = pi->pi_fptr.postfcbs)) {
		(*post_fcbs)(pi, chanidx);
	}

	return pi->phy_fcbs->curr_fcbs_chan;
}

int wlc_phy_fcbs(wlc_phy_t *ppi, int chanidx, bool set)
{
	fcbspostfn_t post_fcbs = NULL;
	fcbsprefn_t pre_fcbs = NULL;
	uint16 ptr;
	int i;
	phy_info_t *pi = (phy_info_t*)ppi;
#if defined(FCBS_CPU_PROFILE)
	unsigned long tick1, tick2, tick_diff;
#endif /* FCBS_CPU_PROFILE */

	if (IS_FCBS(pi)) {
		return wlc_phy_hw_fcbs((wlc_phy_t*)pi, chanidx, set);
	}
	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

	if (!pi->phy_fcbs->initialized[chanidx]) {
		return -1;
	}

	if (!set) {
		return pi->phy_fcbs->curr_fcbs_chan;
	}

#if defined(FCBS_GPIO_PROFILE)
	/* Assert GPIO 4 before switching */
	si_gpioout(pi->sh->sih, (1 << 4), (1 << 4), GPIO_DRV_PRIORITY);
	si_gpioouten(pi->sh->sih, (1 << 4), (1 << 4), GPIO_DRV_PRIORITY);
#endif /* FCBS_GPIO_PROFILE */

	/* If we are currently in the 2G band and we are switching to 5G, tell
	   the ucode to turn the BPHY core off
	*/
	if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[pi->phy_fcbs->curr_fcbs_chan])) {
		if (CHSPEC_IS5G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_OFF);
		}
	} else {
		/* We are currently in 5G. If we are switching to the 2G band
		   tell the ucode to turn the BPHY core ON
		*/
		if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_ON);
		}
	}

	pi->phy_fcbs->curr_fcbs_chan = chanidx;

	/* If one of the FCBS channel is in the 2G band and the other is in the
	   5G band, then the length of the PHY register cache will be different
	   due to the BPHY register values. In this case update the shmem
	   location with the appropriate value for the channel that we are
	   switching to
	*/
	if (pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_A] !=
		pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_B]) {
		wlapi_bmac_write_shm(pi->sh->physhim,
			pi->phy_fcbs->shmem_phyreg, (uint16)(pi->phy_fcbs->phyreg_buflen[chanidx]));
	}

	/* About to start FCBS. Call the PHY specific pre-FCBS function, if any */
	if ((pre_fcbs = pi->pi_fptr.prefcbs)) {
		(*pre_fcbs)(pi, chanidx);
	}

	/* Now tell the ucode which cache (CHAN_A or CHAN_B) it should use */
	wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr,
	    (uint16) (pi->phy_fcbs->chan_cache_offset[chanidx] >> 2));

	pi->phy_fcbs->switch_count++;

#if defined(FCBS_GPIO_PROFILE)
	/* De-assert GPIO 4 after switching */
	si_gpioout(pi->sh->sih, (1 << 4), 0, GPIO_DRV_PRIORITY);
	si_gpioouten(pi->sh->sih, (1 << 4), (1 << 4),
	    GPIO_DRV_PRIORITY);
#endif /* FCBS_GPIO_PROFILE */

#if defined(FCBS_CPU_PROFILE)
	OSL_GETCYCLES(tick1);
#endif /* FCBS_CPU_PROFILE */

	/*
	Wait for ucode to write register tables:
	Using 4331 Rev B0
	Measuring 162 usecs from poking cache_ptr until it reads back 0.
	Each wlapi_bmac_read_shm() call takes 1.8 usecs.
	*/
	OSL_DELAY(100);
	ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	for (i = 0; ptr != 0 && i < FCBS_MAX_ITERS; i++) {
		OSL_DELAY(1);
		ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	}
#if defined(FCBS_CPU_PROFILE)
	OSL_GETCYCLES(tick2);
	tick_diff = tick2 - tick1;
	PHY_ERROR(("%s: completed chan %d, %ld usecs\n",
		__FUNCTION__, CHSPEC_CHANNEL(pi->phy_fcbs->chanspec[chanidx]),
		tick_diff/3000)); /* 3 Ghz cpu */
#endif /* FCBS_CPU_PROFILE */
	if (i >= FCBS_MAX_ITERS) {
		PHY_ERROR(("%s: Failed to complete ucode channel switch\n", __FUNCTION__));
	}
	ASSERT(i < FCBS_MAX_ITERS);

	/* FCBS is done. Call the PHY specific post-FCBS function, if any */
	if ((post_fcbs = pi->pi_fptr.postfcbs)) {
		(*post_fcbs)(pi, chanidx);
	}

	return pi->phy_fcbs->curr_fcbs_chan;
}
#endif /* ENABLE_FCBS */

void
wlc_phy_get_SROMnoiselvl_phy(phy_info_t *pi, int8 *noiselvl)
{
	/* Returns noise level (read from srom) for current channel */
	uint8 core;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 14) {
		/* 2G */
		FOREACH_CORE(pi, core) {
			noiselvl[core] = pi->noiselvl_2g[core];
		}
	} else {
#ifdef BAND5G
		/* 5G */
		if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 48) {
			/* 5G-low: channels 36 through 48 */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->noiselvl_5gl[core];
			}
		} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 64) {
			/* 5G-mid: channels 52 through 64 */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->noiselvl_5gm[core];
			}
		} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 128) {
			/* 5G-high: channels 100 through 128 */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->noiselvl_5gh[core];
			}
		} else {
			/* 5G-upper: channels 132 and above */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->noiselvl_5gu[core];
			}
		}
#endif /* BAND5G */
	}
}

#define PHY_TEMPSENSE_MIN 0
#define PHY_TEMPSENSE_MAX 105
void
wlc_phy_upd_gain_wrt_temp_phy(phy_info_t *pi, int16 *gain_err_temp_adj)
{
	*gain_err_temp_adj = 0;

	/* now, adjust for temperature */
	if ((ISNPHY(pi) &&
	     (NREV_GE(pi->pubpi.phy_rev, 3) &&
	      NREV_LE(pi->pubpi.phy_rev, 6))) || ISHTPHY(pi) || ISACPHY(pi)) {
		/* read in the temperature */
		int16 temp_diff, curr_temp = 0, gain_temp_slope = 0;

		if (ISNPHY(pi)) {
			/* make sure mac is suspended before calling tempsense */
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			curr_temp = wlc_phy_tempsense_nphy(pi);
			wlapi_enable_mac(pi->sh->physhim);
		} else if (ISHTPHY(pi)) {
			curr_temp = pi->u.pi_htphy->current_temperature;
		} else if (ISACPHY(pi)) {
		  curr_temp = pi->u.pi_acphy->current_temperature;
		}

		curr_temp = MIN(MAX(curr_temp, PHY_TEMPSENSE_MIN), PHY_TEMPSENSE_MAX);

		/* check that non programmed SROM for cal temp are not changed */
		if (pi->srom_rawtempsense != 255) {
			temp_diff = curr_temp - pi->srom_rawtempsense;
		} else {
			temp_diff = 0;
		}

		/* adjust gain based on the temperature difference now vs. calibration time:
		 * make gain diff rounded to nearest 0.25 dbm, where 1 tick is 0.25 dbm
		 */
		if (ISNPHY(pi)) {
			gain_temp_slope = CHSPEC_IS2G(pi->radio_chanspec) ?
			        NPHY_GAIN_VS_TEMP_SLOPE_2G : NPHY_GAIN_VS_TEMP_SLOPE_5G;
		} else if (ISHTPHY(pi)) {
			gain_temp_slope = CHSPEC_IS2G(pi->radio_chanspec) ?
			        HTPHY_GAIN_VS_TEMP_SLOPE_2G : HTPHY_GAIN_VS_TEMP_SLOPE_5G;
		} else if (ISACPHY(pi)) {
			gain_temp_slope = CHSPEC_IS2G(pi->radio_chanspec) ?
			        ACPHY_GAIN_VS_TEMP_SLOPE_2G : ACPHY_GAIN_VS_TEMP_SLOPE_5G;
		}

		if (temp_diff >= 0) {
			*gain_err_temp_adj = (temp_diff * gain_temp_slope*2 + 25)/50;
		} else {
			*gain_err_temp_adj = (temp_diff * gain_temp_slope*2 - 25)/50;
		}
	}

#ifdef WLTEST
	if (ISLCN40PHY(pi)) {
		*gain_err_temp_adj  = wlc_lcn40phy_rxgaincal_tempadj(pi);
	}
#endif // endif

}

void
wlc_phy_upd_gain_wrt_gain_cal_temp_phy(phy_info_t *pi, int16 *gain_err_temp_adj)
{

	/* now, adjust for temperature */

	/* read in the temperature */
	int16 temp_diff, curr_temp = 0, gain_temp_slope = 0;

	*gain_err_temp_adj = 0;

	curr_temp = pi->u.pi_acphy->current_temperature;

	curr_temp = MIN(MAX(curr_temp, PHY_TEMPSENSE_MIN), PHY_TEMPSENSE_MAX);

	/* check that non programmed SROM for cal temp are not changed */
	if (pi->srom_gain_cal_temp != 255) {
		temp_diff = curr_temp - pi->srom_gain_cal_temp;
	} else {
		temp_diff = 0;
	}

	/* adjust gain based on the temperature difference now vs. calibration time:
	* make gain diff rounded to nearest 0.25 dbm, where 1 tick is 0.25 dbm
	*/

	gain_temp_slope = CHSPEC_IS2G(pi->radio_chanspec) ?
	    ACPHY_GAIN_VS_TEMP_SLOPE_2G : ACPHY_GAIN_VS_TEMP_SLOPE_5G;

	if (temp_diff >= 0) {
		*gain_err_temp_adj = (temp_diff * gain_temp_slope*2 + 25)/50;
	} else {
		*gain_err_temp_adj = (temp_diff * gain_temp_slope*2 - 25)/50;
	}

}

#ifdef WLTEST
void
wlc_phy_pkteng_boostackpwr(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISLCN40PHY(pi) && (pi->boostackpwr == 1)) {
		pi->pi_fptr.settxpwrbyindex(pi, (int)0);
	}
}
#endif /* ifdef WLTEST */

#ifdef LP_P2P_SOFTAP
uint8
BCMATTACHFN(wlc_lcnphy_get_index)(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
		phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
		int8 offset = pi_lcn->pwr_offset_val;
		uint8 i;

		if (offset) {
			/* Search for the offset */
			for (i = 0;
				(i < LCNPHY_TX_PWR_CTRL_MACLUT_LEN) && (-offset >= pwr_lvl_qdB[i]);
				i++);
			return --i;
		}
	}

	/* For other PHYs return 0: Nominal power */
	return 0;
}
#endif /* LP_P2P_SOFTAP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
void
wlc_acphy_txerr_dump(uint16 PhyErr)
{
	char flagstr[128];
	const bcm_bit_desc_t attr_flags_phy_err[] = {
		{ACPHY_TxError_NDPError_MASK(0), "NDPError"},
		{ACPHY_TxError_RsvdBitError_MASK(0), "RsvdBitError"},
		{ACPHY_TxError_illegal_frame_type_MASK(0), "Illegal frame type"},
		{ACPHY_TxError_COMBUnsupport_MASK(0), "COMBUnsupport"},
		{ACPHY_TxError_BWUnsupport_MASK(0), "BWUnsupport"},
		{ACPHY_TxError_txInCal_MASK(0), "txInCal_MASK"},
		{ACPHY_TxError_send_frame_low_MASK(0), "send_frame_low"},
		{ACPHY_TxError_lengthmismatch_short_MASK(0), "lengthmismatch_short"},
		{ACPHY_TxError_lengthmismatch_long_MASK(0), "lengthmismatch_long"},
		{ACPHY_TxError_invalidRate_MASK(0), "invalidRate_MASK"},
		{ACPHY_TxError_unsupportedmcs_MASK(0), "unsupported mcs"},
		{ACPHY_TxError_send_frame_low_MASK(0), "send_frame_low"},
		{0, NULL}
	};
	if (PhyErr) {
		bcm_format_flags(attr_flags_phy_err, PhyErr, flagstr, 128);
		printf("Tx PhyErr 0x%04x (%s)\n", PhyErr, flagstr);
	}
}

void
wlc_htphy_txerr_dump(uint16 PhyErr)
{
	char flagstr[128];
	const bcm_bit_desc_t attr_flags_phy_err[] = {
		{HTPHY_TxError_NDPError_MASK, "NDPError"},
		{HTPHY_TxError_illegal_frame_type_MASK, "Illegal frame type"},
		{HTPHY_TxError_COMBUnsupport_MASK, "COMBUnsupport"},
		{HTPHY_TxError_BWUnsupport_MASK, "BWUnsupport"},
		{HTPHY_TxError_txInCal_MASK, "txInCal_MASK"},
		{HTPHY_TxError_send_frame_low_MASK, "send_frame_low"},
		{HTPHY_TxError_lengthmismatch_short_MASK, "lengthmismatch_short"},
		{HTPHY_TxError_lengthmismatch_long_MASK, "lengthmismatch_long"},
		{HTPHY_TxError_invalidRate_MASK, "invalidRate_MASK"},
		{0, NULL}
	};
	if (PhyErr) {
		bcm_format_flags(attr_flags_phy_err, PhyErr, flagstr, 128);
		printf("Tx PhyErr 0x%04x (%s)\n", PhyErr, flagstr);
	}
}
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef WL_LPC
uint8
wlc_phy_lpc_getminidx(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi->pi_fptr.lpcgetminidx)
		return (*pi->pi_fptr.lpcgetminidx)();
	return 0;
}

uint8
wlc_phy_lpc_getoffset(wlc_phy_t *ppi, uint8 index)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi->pi_fptr.lpcgetpwros)
		return (*pi->pi_fptr.lpcgetpwros)(index);
	return 0;
}

uint8
wlc_phy_lpc_get_txcpwrval(wlc_phy_t *ppi, uint16 phytxctrlword)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi->pi_fptr.lpcgettxcpwrval)
		return (*pi->pi_fptr.lpcgettxcpwrval)(phytxctrlword);
	return 0;
}

void
wlc_phy_lpc_set_txcpwrval(wlc_phy_t *ppi, uint16 *phytxctrlword, uint8 txcpwrval)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi->pi_fptr.lpcsettxcpwrval)
		(*pi->pi_fptr.lpcsettxcpwrval)(phytxctrlword, txcpwrval);
	return;
}

void
wlc_phy_lpc_algo_set(wlc_phy_t *ppi, bool enable)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->lpc_algo = enable;
	if (pi->pi_fptr.lpcsetmode)
		(*pi->pi_fptr.lpcsetmode)(pi, enable);
	return;
}

#ifdef WL_LPC_DEBUG
uint8 *
wlc_phy_lpc_get_pwrlevelptr(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi->pi_fptr.lpcgetpwrlevelptr)
		return (*pi->pi_fptr.lpcgetpwrlevelptr)();
	return 0;
}
#endif // endif
#endif /* WL_LPC */

#if defined(WLC_LOWPOWER_BEACON_MODE)
void
wlc_phy_lowpower_beacon_mode(wlc_phy_t *pih, int lowpower_beacon_mode)
{
	phy_info_t *pi = (phy_info_t *)pih;
	if (pi->pi_fptr.lowpowerbeaconmode)
		pi->pi_fptr.lowpowerbeaconmode(pi, lowpower_beacon_mode);
}
#endif /* WLC_LOWPOWER_BEACON_MODE */

/* block bbpll change if ILP cal is in progress */
void
wlc_phy_block_bbpll_change(wlc_phy_t *pih, bool block, bool going_down)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (block) {
		pi->block_for_slowcal = TRUE;
	} else {
		pi->block_for_slowcal = FALSE;

		if (pi->blocked_freq_for_slowcal && ISACPHY(pi) && !going_down) {
			phy_ac_set_spurmode(pi->u.pi_acphy->rxspuri,
				pi->blocked_freq_for_slowcal);
		} else if (pi->blocked_freq_for_slowcal && ISNPHY(pi) && !going_down) {
			wlc_phy_set_spurmode_nphy(pi, pi->blocked_freq_for_slowcal);
		}
	}

}

#ifdef WLTXPWR_CACHE
void wlc_phy_clear_tx_power_offset(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

/*	if ((pi->tx_power_offset != NULL) &&
		(!wlc_phy_is_pwr_cached(pi->txpwr_cache, TXPWR_CACHE_POWER_OFFSETS,
		pi->tx_power_offset))) {
			ppr_delete(pi->sh->osh, pi->tx_power_offset);
	}
*/
	pi->tx_power_offset = NULL;
}
#endif /* WLTXPWR_CACHE */

void
wlc_phy_btcx_wlan_critical_enter(phy_info_t *pi)
{
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL, MHF1_WLAN_CRITICAL, WLC_BAND_2G);
}

void
wlc_phy_btcx_wlan_critical_exit(phy_info_t *pi)
{
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL, 0, WLC_BAND_2G);
}

void
wlc_phy_trigger_cals_for_btc_adjust(phy_info_t *pi)
{
	wlc_phy_cal_perical_mphase_reset(pi);
	if (ISNPHY(pi)) {
		pi->u.pi_nphy->cal_type_override = PHY_PERICAL_FULL;
	}
	wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
}

void
wlc_phy_set_femctrl_bt_wlan_ovrd(wlc_phy_t *pih, int8 state)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (ISACPHY(pi))
	  {
	    pi->u.pi_acphy->bt_sw_state = state;
	    wlc_phy_set_femctrl_bt_wlan_ovrd_acphy(pi, state);
	  }
}

int8
wlc_phy_get_femctrl_bt_wlan_ovrd(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int8 state = AUTO;

	if (ISACPHY(pi))
		state = wlc_phy_get_femctrl_bt_wlan_ovrd_acphy(pi);

	return state;
}

#ifdef PREASSOC_PWRCTRL
void
wlc_phy_store_tx_pwrctrl_setting(phy_info_t *pi, chanspec_t previous_chanspec)
{
	if (!pi->sh->up)
		return;
	if (ISACPHY(pi)) {
		wlc_phy_store_tx_pwrctrl_setting_acphy(pi, previous_chanspec);
	}
}
#endif /* PREASSOC_PWRCTRL */

int wlc_phy_tssivisible_thresh(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int visi_thresh_qdbm;

	if (ISACPHY(pi)) visi_thresh_qdbm = wlc_phy_tssivisible_thresh_acphy(pi);
	else visi_thresh_qdbm = -128;
	return visi_thresh_qdbm;
}

#ifdef WLSRVSDB
#ifdef WFD_PHY_LL
int
wlc_phy_sr_vsdb_invalidate_active(wlc_phy_t * ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	if (pi->srvsdb_state->sr_vsdb_channels[0] ==
	    CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec)) {
		pi->srvsdb_state->swbkp_snapshot_valid[0] = 0;
		return 0;
	}
	else if (pi->srvsdb_state->sr_vsdb_channels[1] ==
	         CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec)) {
		pi->srvsdb_state->swbkp_snapshot_valid[1] = 0;
		return 1;
	}

	return -1;
}
#endif /* WFD_PHY_LL */

void
wlc_phy_sr_vsdb_reset(wlc_phy_t * ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->srvsdb_state->swbkp_snapshot_valid[0] = 0;
	pi->srvsdb_state->swbkp_snapshot_valid[1] = 0;
	pi->srvsdb_state->sr_vsdb_bank_valid[0] = FALSE;
	pi->srvsdb_state->sr_vsdb_bank_valid[1] = FALSE;
	pi->srvsdb_state->prev_chanspec = CHANNEL_UNDEFINED;
	pi->srvsdb_state->vsdb_trig_cnt = 0;
}

/** reset SR VSDB hw */
void
wlc_phy_reset_srvsdb_engine(wlc_phy_t *ppi)
{
	uint origidx;
	chipcregs_t *cc;
	si_t *sih;
	phy_info_t *pi = (phy_info_t*)ppi;

	sih = (si_t*)pi->sh->sih;

	origidx = si_coreidx(sih);

	/* setcore to chipcmn */
	cc = si_setcoreidx(sih, SI_CC_IDX);

	/* Jira-SWWLAN-28550: A fix of Sr related changes from B0 to B1 created a problem for VSDB
	For VSDB, Bit 9 of PMU chip control 2 gates off the clock to the SR engine, in VSDB
	mode this Bit is always SET.
	Fix: RESET the Bit Before SRVSDB, and SET it back after SRVSDB, this is done presently
	only for 4324B4 chips
	*/
	if (CHIP_4324_B4(pi)) {
		uint32 temp, temp1;
		W_REG(si_osh(sih), &cc->chipcontrol_addr, 2);
		temp = R_REG(si_osh(sih), &cc->chipcontrol_data);
		temp1 = temp & 0xFFFFFDFF;
		W_REG(si_osh(sih), &cc->chipcontrol_data, temp1);
	}

	/* Reset 2nd bit */
	sr_chipcontrol(sih, 0x4, 0x0);
	/* Set 2nd bit */
	sr_chipcontrol(sih, 0x4, 0x4);

	/* Jira-SWWLAN-28550: A fix of Sr related changes from B0 to B1 created a problem for VSDB
	For VSDB, Bit 9 of PMU chip control 2 gates off the clock to the SR engine, in VSDB
	mode this Bit is always SET.
	Fix: RESET the Bit Before SRVSDB, and SET it back after SRVSDB, this is done presently
	only for 4324B4 chips
	*/
	if (CHIP_4324_B4(pi)) {
		uint32 temp, temp1;
		W_REG(si_osh(sih), &cc->chipcontrol_addr, 2);
		temp = R_REG(si_osh(sih), &cc->chipcontrol_data);
		temp1 = temp | 0x200;
		W_REG(si_osh(sih), &cc->chipcontrol_data, temp1);
	}

	/* Set core orig core */
	si_setcoreidx(sih, origidx);

}

void
wlc_phy_force_vsdb_chans(wlc_phy_t *ppi, uint16* vsdb_chans, uint8 set)
{
	uint16 chans[2];
	phy_info_t *pi = (phy_info_t*)ppi;

	if (set) {
		pi->srvsdb_state->force_vsdb = 1;

		bcopy(vsdb_chans, chans, 2*sizeof(uint16));

		wlc_phy_attach_srvsdb_module(ppi, chans[0], chans[1]);

		pi->srvsdb_state->prev_chanspec = chans[0];
		pi->srvsdb_state->force_vsdb_chans[0] = chans[0];
		pi->srvsdb_state->force_vsdb_chans[1] = chans[1];

	} else {
		wlc_phy_detach_srvsdb_module(ppi);

		/* Reset force vsdb chans */
		pi->srvsdb_state->force_vsdb_chans[0] = 0;
		pi->srvsdb_state->force_vsdb_chans[1] = 0;
		pi->srvsdb_state->force_vsdb = 0;
	}
}

void
wlc_phy_detach_srvsdb_module(wlc_phy_t *ppi)
{
	uint8 i;
	phy_info_t *pi = (phy_info_t*)ppi;

	/* Disable the flags */
	wlc_phy_sr_vsdb_reset(ppi);

	for (i = 0; i < SR_MEMORY_BANK; i++) {
		if (pi->vsdb_bkp[i] != NULL) {
			if (pi->vsdb_bkp[i]->pi_nphy != NULL) {
				MFREE(pi->sh->osh, pi->vsdb_bkp[i]->pi_nphy,
					sizeof(phy_info_nphy_t));

				pi->vsdb_bkp[i]->pi_nphy = NULL;
			}

			if (pi->vsdb_bkp[i]->tx_power_offset != NULL)
				ppr_delete(pi->sh->osh, pi->vsdb_bkp[i]->tx_power_offset);
			MFREE(pi->sh->osh, pi->vsdb_bkp[i], sizeof(vsdb_backup_t));
			pi->vsdb_bkp[i] = NULL;
			PHY_INFORM(("de allocate %d of mem \n", (sizeof(vsdb_backup_t) +
				sizeof(phy_info_nphy_t))));
		}
	}
	pi->srvsdb_state->sr_vsdb_channels[0] = 0;
	pi->srvsdb_state->sr_vsdb_channels[1] = 0;
	pi->srvsdb_state->srvsdb_active = 0;

	pi->srvsdb_state->acimode_noisemode_reset_done[0] = FALSE;
	pi->srvsdb_state->acimode_noisemode_reset_done[1] = FALSE;

	/* srvsdb switch status */
	pi->srvsdb_state->switch_successful = FALSE;

	/* Timers */
	bzero(pi->srvsdb_state->prev_timer, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_timer, 2 * sizeof(uint32));

	/* counter for no of switch iterations */
	bzero(pi->srvsdb_state->num_chan_switch, 2 * sizeof(uint8));

	/* crsglitch */
	bzero(pi->srvsdb_state->prev_crsglitch_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_crsglitch, 2 * sizeof(uint32));
	/* bphy_crsglitch */
	bzero(pi->srvsdb_state->prev_bphy_rxcrsglitch_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_bphy_crsglitch, 2 * sizeof(uint32));
	/* badplcp */
	bzero(pi->srvsdb_state->prev_badplcp_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_prev_badplcp, 2 * sizeof(uint32));
	/* bphy_badplcp */
	bzero(pi->srvsdb_state->prev_bphy_badplcp_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_prev_bphy_badplcp, 2 * sizeof(uint32));
}

/**
 * Despite the 'attach' in its name: is not meant to be called in the 'attach' phase.
 * Returns TRUE on success. Caller supplied arguments chan0 and chan1 may not reside in the same
 * band.
 */
uint8
wlc_phy_attach_srvsdb_module(wlc_phy_t *ppi, chanspec_t chan0, chanspec_t chan1)
{

	uint8 i;
	phy_info_t *pi = (phy_info_t*)ppi;

	/* Detach allready existing structire */
	wlc_phy_detach_srvsdb_module(ppi);

	/* reset srvsdb enigne */
	wlc_phy_reset_srvsdb_engine(ppi);

	/* Alloc mem for sw backup structures */
	for (i = 0; i < SR_MEMORY_BANK; i++) {
		if ((pi->vsdb_bkp[i] = (vsdb_backup_t*)MALLOC(pi->sh->osh,
			sizeof(vsdb_backup_t))) == NULL) {

			PHY_ERROR(("SRVSDB: MEM alloc fail for vsdb_backup_t\n"));
			ASSERT(0);
			return FALSE;
		}

		bzero(pi->vsdb_bkp[i], sizeof(vsdb_backup_t));

		if ((pi->vsdb_bkp[i]->pi_nphy = (phy_info_nphy_t *)MALLOC(pi->sh->osh,
			sizeof(phy_info_nphy_t))) == NULL) {

			PHY_ERROR(("SRVSDB: MEM alloc fail for phy_info_nphy_t\n"));
			ASSERT(0);
			MFREE(pi->sh->osh, pi->vsdb_bkp[i], sizeof(vsdb_backup_t));
			return FALSE;
		}
		bzero(pi->vsdb_bkp[i]->pi_nphy, sizeof(phy_info_nphy_t));

		PHY_INFORM(("allocate %d of mem \n", (sizeof(vsdb_backup_t) +
			sizeof(phy_info_nphy_t))));
	}

	pi->srvsdb_state->sr_vsdb_channels[0] = CHSPEC_CHANNEL(chan0);
	pi->srvsdb_state->sr_vsdb_channels[1] = CHSPEC_CHANNEL(chan1);
	pi->srvsdb_state->srvsdb_active = 1;

	return TRUE;
}
#endif /* WLSRVSDB */

/**
 * Reduce channel switch time by attempting to use hardware acceleration.
 */
uint8
wlc_set_chanspec_sr_vsdb(wlc_phy_t *ppi, chanspec_t chanspec, uint8 *last_chan_saved)
{
	uint8 switched = FALSE;

#ifdef WLSRVSDB
	phy_info_t *pi = (phy_info_t*)ppi;

	switched = wlc_set_chanspec_sr_vsdb_nphy(pi, chanspec, last_chan_saved);

#if defined(WLTXPWR_CACHE) && defined(WLC_LOW_ONLY)
	if (switched) {
		/*
		 * For split MAC driver, for faster channel switching, reserve an entry for the new
		 * channel.
		 */
		wlc_phy_pwr_cache_reserve(pi, chanspec);
	}
#endif /* WLTXPWR_CACHE && WLC_LOW_ONLY */
#endif /* WLSRVSDB */
	return switched;
}

void wlc_phy_clear_match_tx_offset(wlc_phy_t *ppi, ppr_t *pprptr)
{
	if (ppi != NULL) {
		phy_info_t *pi = (phy_info_t*)ppi;
		if (pprptr == pi->tx_power_offset)
			pi->tx_power_offset = NULL;
	}
}

#if defined(ATE_BUILD) || defined(BCMDBG) || defined(BCM_MACDBG)
void
wlc_phy_gpiosel_disable(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	/* Regs. to enable / disable PHY GPIOs */
	uint16 reg_gpioOutEn_lo = ISACPHY(pi) ?
		ACPHY_gpioLoOutEn(pi->pubpi.phy_rev) : HTPHY_gpioLoOutEn;
	uint16 reg_gpioOutEn_hi = ISACPHY(pi) ?
		ACPHY_gpioHiOutEn(pi->pubpi.phy_rev) : HTPHY_gpioHiOutEn;

#ifdef ATE_BUILD
	if (ISACPHY(pi) && ACMAJORREV_3(pi->pubpi.phy_rev)) {
#endif /* ATE_BUILD */
		phy_utils_phyreg_enter(pi);
		phy_utils_write_phyreg(pi, reg_gpioOutEn_lo, 0);
		phy_utils_write_phyreg(pi, reg_gpioOutEn_hi, 0);
		phy_utils_phyreg_exit(pi);
#ifdef ATE_BUILD
	} else {
		/* Not supported for other PHYs yet */
		ASSERT(FALSE);
	}
#endif /* ATE_BUILD */

}
#endif // endif

#if defined(RXDESENS_EN)
/* Main wrapper for dynamic phy rxdesens */
void
wlc_phy_rxdesens(wlc_phy_t *ppi, bool densens_en)
{
	phy_info_t *pi = (phy_info_t *)ppi;
#ifdef WLSRVSDB
	bool invalidate_swbkp = FALSE;
#endif // endif

	if (CHSPEC_IS5G(pi->radio_chanspec))
		return;

	if (pi->sromi->phyrxdesens == 0)
		return;

	if (densens_en) {
#ifndef WLC_DISABLE_ACI
		if (pi->sh->interference_mode != INTERFERE_NONE) {
			/* disable interference */
			wlc_phy_interference(pi, INTERFERE_NONE, TRUE);
			pi->sromi->saved_interference_mode = pi->sh->interference_mode;
			pi->sh->interference_mode = INTERFERE_NONE;
#ifdef WLSRVSDB
			invalidate_swbkp = TRUE;
#endif // endif
		}
#endif /* !defined(WLC_DISABLE_ACI) */
		/* apply densens */
		wlc_nphy_set_rxdesens((wlc_phy_t *)pi, pi->sromi->phyrxdesens);
		pi->u.pi_nphy->ntd_rxdesens_active = TRUE;
		PHY_INFORM(("%s: applied desens on Ch %d |"
			" phyreg 0x21 %x | 0x283: %x | 0x4aa: %x\n",
			__FUNCTION__, CHSPEC_CHANNEL(pi->radio_chanspec),
			phy_utils_read_phyreg(pi, 0x21), phy_utils_read_phyreg(pi, 0x283),
			phy_utils_read_phyreg(pi, 0x4aa)));
	} else {
		/* remove desens */
		wlc_nphy_set_rxdesens((wlc_phy_t *)pi, 0);
		pi->u.pi_nphy->ntd_rxdesens_active = FALSE;
		PHY_INFORM(("%s: removed desens on Ch %d |"
			" phyreg 0x21: %x | 0x283: %x | 0x4aa: %x\n",
			__FUNCTION__, CHSPEC_CHANNEL(pi->radio_chanspec),
			phy_utils_read_phyreg(pi, 0x21), phy_utils_read_phyreg(pi, 0x283),
			phy_utils_read_phyreg(pi, 0x4aa)));
#ifndef WLC_DISABLE_ACI
		/* enable interference */
		pi->sh->interference_mode = pi->sromi->saved_interference_mode;
		wlc_phy_interference(pi, pi->sh->interference_mode, TRUE);
#ifdef WLSRVSDB
		invalidate_swbkp = TRUE;
#endif // endif
#endif /* !defined(WLC_DISABLE_ACI) */
	}

#ifdef WLSRVSDB
	if (invalidate_swbkp) {
		/* invalidate swbkp to update per cfg phyrxdesens & interference_mode */
		if (pi->srvsdb_state->sr_vsdb_channels[0] ==
			CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec)) {

			pi->srvsdb_state->swbkp_snapshot_valid[0] = 0;
			PHY_INFORM(("invalidating swbkp for Ch %d\n",
				CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec)));

		} else if (pi->srvsdb_state->sr_vsdb_channels[1] ==
			CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec)) {

			pi->srvsdb_state->swbkp_snapshot_valid[1] = 0;
			PHY_INFORM(("invalidating swbkp for Ch %d\n",
				CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec)));
		}
	}
#endif /* defined(WLSRVSDB) */
}
#ifdef WLSRVSDB
/* Returns a flag to defer dynamic rxdesens if srvsdb engine
 * is not ready or SW backup is not taken.
 */
bool
wlc_phy_rxdesens_defer(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	int i = 0;
	bool defer = FALSE;

	for (i = 0; i < 2; i++) {
		if (pi->srvsdb_state->srvsdb_active &&
			!pi->srvsdb_state->swbkp_snapshot_valid[i]) {
			defer = TRUE;
			break;
		}
	}
	return defer;
}
#endif /* WLSRVSDB */
#endif /* RXDESENS_EN */

#if defined(WLMCHAN)
/* Returns a flag to reset aci and noise mitigation states (in vsdb mode) when back in home chanspec
 * to ensure corruption free sw backup and hardware state save/restore.
 */
bool
wlc_phy_acimode_noisemode_reset_allowed(wlc_phy_t *ppi, uint16 home_channel)
{
	bool allow = FALSE;
#if !defined(WLC_DISABLE_ACI) && defined(WLSRVSDB)
	phy_info_t *pi = (phy_info_t *)ppi;

	int i = 0;
	for (i = 0; i < 2; i++)
		if ((pi->srvsdb_state->sr_vsdb_channels[i] == home_channel) &&
			(!pi->srvsdb_state->swbkp_snapshot_valid[i]) &&
			(!pi->srvsdb_state->acimode_noisemode_reset_done[i])) {
			allow = TRUE;
			pi->srvsdb_state->acimode_noisemode_reset_done[i] = TRUE;
			break;
		}
#endif // endif
	return allow;
}

#ifdef WLSRVSDB
bool
wlc_phy_srvsdb_switch_status(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return pi->srvsdb_state->switch_successful;
}
#endif /* WLSRVSDB */
#endif /* defined(WLMCHAN) */

#ifdef ATE_BUILD
void
wlc_phy_gpaio(wlc_phy_t *ppi, wl_gpaio_option_t option, int core)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi->pi_fptr.gpaioconfigptr) {
		return (*pi->pi_fptr.gpaioconfigptr)(pi, option, core);
	}
	return;
}
#endif // endif

#ifdef	WL_DYNAMIC_TEMPSENSE
#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_temperature_override(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	return  pi->tempsense_override;
}
#endif /* BCMDBG || WLTEST */

int
wlc_phy_temperature_threshold(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	return pi->txcore_temp->disable_temp;
}

/*
 * Move this to wlc_phy_cmn.c and do it based on PHY
 * This function DOES NOT calculate temperature and it also DOES NOT
 * read any register from MAC and/or RADIO. This function returns
 * last recorded temperature that is stored in phy_info_t.
 * There are few types of phy that DO NOT record last temperature
 * but derives it based on various other mechnism. This function
 * returns BCME_RANGE for such type of PHY. So that the code would
 * continue as per old fashioned tempsense mechanism at every 10S.
 */
int
wlc_phy_current_temperature(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int ct = BCME_RANGE;

	if (pi->txcore_temp->heatedup) {
		return BCME_RANGE; /* Indicate we are already heated up */
	}
	if (ISHTPHY(pi))
		ct = pi->u.pi_htphy->current_temperature;
	if (ISACPHY(pi))
		ct = pi->u.pi_acphy->current_temperature;

	if (ct >= (pi->txcore_temp->disable_temp))
		return BCME_RANGE;
	return ct;
}
#endif /* WL_DYNAMIC_TEMPSENSE */

#ifdef WLC_TXDIVERSITY
/*
 * Returns Oclscd Enable status
 */
uint8
wlc_phy_get_oclscdenable_status(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	return pi->nphy_oclscd;
}
#endif /* WLC_TXDIVERSITY */

void
wlc_phy_update_link_rssi(wlc_phy_t *pih, int8 rssi)
{
	phy_info_t *pi = (phy_info_t*)pih;

	pi->interf->link_rssi = rssi;
}

void
wlc_phy_watchdog_override(phy_info_t *pi, bool val)
{
	pi->phywatchdog_override = val;
}

#if ((ACCONF != 0) || (ACCONF2 != 0) || (NCONF != 0) || (HTCONF != 0) || (LCN40CONF != \
	0))

void wlc_dynamic_ed_thresh_enable(phy_info_t *pi,bool set_dynamic_ed_th)
{
	pi->dynamic_ed_thresh_enable = set_dynamic_ed_th;
	return;
}
void
wlc_dynamic_ed_th_overwrite_mon_win(phy_info_t *pi, int val){
	pi->const_ed_monitor_window = val;
	return;
}
void
wlc_dynamic_ed_th_overwrite_sed_dis(phy_info_t *pi, int val){
	pi->const_sed_disable = val;
	return;
}
void
wlc_dynamic_ed_th_overwrite_sed_high(phy_info_t *pi, int val){
	pi->const_sed_upper_bound = val;
	return;
}
void
wlc_dynamic_ed_th_overwrite_sed_low(phy_info_t *pi, int val){
	pi->const_sed_lower_bound = val;
	return;
}
void
wlc_dynamic_ed_th_overwrite_th_high(phy_info_t *pi, int val){
	pi->const_ed_th_high = val;
	return;
}
void
wlc_dynamic_ed_th_overwrite_th_low(phy_info_t *pi, int val){
	pi->const_ed_th_low = val;
	return;
}
void
wlc_dynamic_ed_th_overwrite_step_inc(phy_info_t *pi, int val){
	pi->const_ed_inc_step = val;
	return;
}
void
wlc_dynamic_ed_th_overwrite_step_dec(phy_info_t *pi,int val){
	pi->const_ed_dec_step = val;
	return;
}

void
wlc_dynamic_ed_th_overwrite_acphy(phy_info_t *pi,int val){
	pi->acphy_for_dynamic_ed = val;
	return;
}

int
wlc_phy_adjust_ed_thres(phy_info_t *pi, int32 *assert_thresh_dbm, bool set_threshold)
{
	if (ISACPHY(pi))
	        wlc_phy_adjust_ed_thres_acphy(pi, assert_thresh_dbm, set_threshold);
	else if (ISNPHY(pi))
		wlc_phy_adjust_ed_thres_nphy(pi, assert_thresh_dbm, set_threshold);
	else if (ISHTPHY(pi))
		wlc_phy_adjust_ed_thres_htphy(pi, assert_thresh_dbm, set_threshold);
	else if (ISLCN40PHY(pi))
		wlc_phy_adjust_ed_thres_lcn40phy(pi, assert_thresh_dbm, set_threshold);
	else
		return BCME_UNSUPPORTED;

	return BCME_OK;
}
#endif /* ((ACCONF != 0) || (ACCONF2 != 0) || (NCONF != 0) || (HTCONF != 0) || (LCN40CONF != 0)) */

static int
wlc_phy_iovars_phy_specific(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen,
	void *a, int alen, int vsize)
{
	int err = BCME_OK;

	if (ISACPHY(pi))
		err = wlc_phy_iovars_acphy(pi, actionid, type, p, plen, a, alen, vsize);
	else if (ISLCNCOMMONPHY(pi))
		err = wlc_phy_iovars_lcncmnphy(pi, actionid, type, p, plen, a, alen, vsize);
	else if (ISNPHY(pi))
		err = wlc_phy_iovars_nphy(pi, actionid, type, p, plen, a, alen, vsize);
	else
		err = BCME_UNSUPPORTED;

	return err;
}

static int
wlc_phy_iovars_nphy(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	BCM_REFERENCE(*ret_int_ptr);
	BCM_REFERENCE(bool_val);

	switch (actionid) {
	case IOV_GVAL(IOV_PHY_OCLSCDENABLE):
		err = wlc_phy_iovar_oclscd(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_OCLSCDENABLE):
		err = wlc_phy_iovar_oclscd(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_LNLDO2):
		wlc_phy_iovar_prog_lnldo2(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_LNLDO2):
		err = wlc_phy_iovar_prog_lnldo2(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

#if defined(WLTEST) || defined(DBG_PHY_IOV)
	case IOV_GVAL(IOV_PHY_DYN_ML):
		err =  wlc_phy_dynamic_ml(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_DYN_ML):
		err =  wlc_phy_dynamic_ml(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_ACI_NAMS):
		err =  wlc_phy_aci_nams(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_ACI_NAMS):
		err =  wlc_phy_aci_nams(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;
#endif // endif
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovars_lcncmnphy(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	BCM_REFERENCE(*ret_int_ptr);

	switch (actionid) {
#if defined(WLTEST)
	case IOV_SVAL(IOV_PHY_AUXPGA):
			if (ISLCNCOMMONPHY(pi)) {
				phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
				uint8 *aug_pga_val = (uint8*)p;
				pi_lcn->lcnphy_rssi_vf = aug_pga_val[0];
				pi_lcn->lcnphy_rssi_vc = aug_pga_val[1];
				pi_lcn->lcnphy_rssi_gs = aug_pga_val[2];
#ifdef BAND5G
				pi_lcn->rssismf5g = aug_pga_val[3];
				pi_lcn->rssismc5g = aug_pga_val[4];
				pi_lcn->rssisav5g = aug_pga_val[5];
#endif // endif
			}
		break;

	case IOV_GVAL(IOV_PHY_AUXPGA):
			if (ISLCNCOMMONPHY(pi)) {
				phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
				uint8 *aug_pga_val = (uint8*)a;
				aug_pga_val[0] = pi_lcn->lcnphy_rssi_vf;
				aug_pga_val[1] = pi_lcn->lcnphy_rssi_vc;
				aug_pga_val[2] = pi_lcn->lcnphy_rssi_gs;
#ifdef BAND5G
				aug_pga_val[3] = (uint8) pi_lcn->rssismf5g;
				aug_pga_val[4] = (uint8) pi_lcn->rssismc5g;
				aug_pga_val[5] = (uint8) pi_lcn->rssisav5g;
#endif // endif
			}
		break;
#endif // endif
#if defined(WLTEST)
#if defined(LCNCONF) || defined(LCN40CONF)
	case IOV_GVAL(IOV_LCNPHY_RXIQGAIN):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->rxpath_gain;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_RXIQGSPOWER):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->rxpath_gainselect_power;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_RXIQPOWER):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->rxpath_final_power;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_RXIQSTATUS):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->rxpath_status;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_RXIQSTEPS):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->rxpath_steps;
		}
		break;
	case IOV_SVAL(IOV_LCNPHY_RXIQSTEPS):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			pi_lcn->rxpath_steps = (uint8)int_val;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_TSSI_MAXPWR):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->tssi_maxpwr_limit;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_TSSI_MINPWR):
		{
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->tssi_minpwr_limit;
		}
		break;
#endif /* #if defined(LCNCONF) || defined(LCN40CONF) */
#if LCN40CONF
	case IOV_GVAL(IOV_LCNPHY_TXPWRCLAMP_DIS):
		if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->txpwr_clamp_dis;
		}
		break;
	case IOV_SVAL(IOV_LCNPHY_TXPWRCLAMP_DIS):
		if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			pi_lcn->txpwr_clamp_dis = (uint8)int_val;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_TXPWRCLAMP_OFDM):
		if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->target_pwr_ofdm_max;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_TXPWRCLAMP_CCK):
		if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
			*ret_int_ptr = (int32)pi_lcn->target_pwr_cck_max;
		}
		break;
#endif /* #if LCN40CONF */
#endif /* #if defined(WLTEST) */
#if defined(BCMDBG)
	case IOV_GVAL(IOV_LCNPHY_CWTXPWRCTRL):
		if (ISLCNPHY(pi))
			wlc_lcnphy_iovar_cw_tx_pwr_ctrl(pi, int_val, ret_int_ptr, FALSE);
		break;
	case IOV_SVAL(IOV_LCNPHY_CWTXPWRCTRL):
		if (ISLCNPHY(pi))
			wlc_lcnphy_iovar_cw_tx_pwr_ctrl(pi, int_val, ret_int_ptr, TRUE);
		break;
#endif // endif
	case IOV_SVAL(IOV_PHY_CRS_WAR):
		if (ISLCN40PHY(pi)) {
			pi->u.pi_lcn40phy->phycrs_war_en = (bool)int_val;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_PHY_CRS_WAR):
		if (ISLCN40PHY(pi)) {
			*ret_int_ptr = (int32) pi->u.pi_lcn40phy->phycrs_war_en;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovars_acphy(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	BCM_REFERENCE(*ret_int_ptr);

	switch (actionid) {
#if defined(BCMDBG) || defined(WLTEST) || defined(WLMEDIA_CALDBG) || \
	defined(PHYCAL_CHNG_CS) || defined(WLMEDIA_FLAMES)
	case IOV_GVAL(IOV_HIRSSI_PERIOD):
		*ret_int_ptr = (int32)pi->u.pi_acphy->hirssi_period;
		break;
	case IOV_SVAL(IOV_HIRSSI_PERIOD):
		pi->u.pi_acphy->hirssi_period = (int_val <= 0) ? PHY_SW_HIRSSI_PERIOD  :
		(uint16)int_val;
		break;
	case IOV_GVAL(IOV_HIRSSI_EN):
		*ret_int_ptr = (int32)pi->u.pi_acphy->hirssi_en;
		break;
	case IOV_SVAL(IOV_HIRSSI_EN):
		pi->u.pi_acphy->hirssi_en = (int_val == 0) ? FALSE : TRUE;
		if (ISACPHY(pi) && PHY_SW_HIRSSI_UCODE_CAP(pi)) {
			wlc_phy_hirssi_elnabypass_init_acphy(pi);
			if (!pi->u.pi_acphy->hirssi_en)
				wlc_phy_hirssi_elnabypass_apply_acphy(pi);
		}
		break;
	case IOV_GVAL(IOV_HIRSSI_BYP_RSSI):
		*ret_int_ptr = (int32)pi->u.pi_acphy->hirssi_byp_rssi;
		break;
	case IOV_SVAL(IOV_HIRSSI_BYP_RSSI):
		pi->u.pi_acphy->hirssi_byp_rssi = (int8)int_val;
		if (ISACPHY(pi) && PHY_SW_HIRSSI_UCODE_CAP(pi))
			wlc_phy_hirssi_elnabypass_init_acphy(pi);
		break;
	case IOV_GVAL(IOV_HIRSSI_RES_RSSI):
		*ret_int_ptr = (int32)pi->u.pi_acphy->hirssi_res_rssi;
		break;
	case IOV_SVAL(IOV_HIRSSI_RES_RSSI):
		pi->u.pi_acphy->hirssi_res_rssi = (int8)int_val;
		if (ISACPHY(pi) && PHY_SW_HIRSSI_UCODE_CAP(pi))
			wlc_phy_hirssi_elnabypass_init_acphy(pi);
		break;
	case IOV_GVAL(IOV_HIRSSI_BYP_CNT):
		*ret_int_ptr = (int32)pi->u.pi_acphy->hirssi_byp_cnt;
		break;
	case IOV_SVAL(IOV_HIRSSI_BYP_CNT):
		pi->u.pi_acphy->hirssi_byp_cnt = (int_val == -1) ? PHY_SW_HIRSSI_W1_BYP_CNT :
		(uint16)int_val;
		if (ISACPHY(pi) && PHY_SW_HIRSSI_UCODE_CAP(pi))
			wlc_phy_hirssi_elnabypass_init_acphy(pi);
		break;
	case IOV_GVAL(IOV_HIRSSI_RES_CNT):
	        *ret_int_ptr = (int32)pi->u.pi_acphy->hirssi_res_cnt;
		break;
	case IOV_SVAL(IOV_HIRSSI_RES_CNT):
		pi->u.pi_acphy->hirssi_res_cnt = (int_val == -1) ? PHY_SW_HIRSSI_W1_RES_CNT :
		(uint16)int_val;
		if (ISACPHY(pi) && PHY_SW_HIRSSI_UCODE_CAP(pi))
			wlc_phy_hirssi_elnabypass_init_acphy(pi);
		break;
	case IOV_GVAL(IOV_HIRSSI_STATUS):
		*ret_int_ptr = 0;
		if (ISACPHY(pi) && PHY_SW_HIRSSI_UCODE_CAP(pi))
			*ret_int_ptr = (int32) wlc_phy_hirssi_elnabypass_status_acphy(pi);
		break;
#endif /*  BCMDBG || WLTEST || WLMEDIA_CALDBG || PHYCAL_CHNG_CS || WLMEDIA_FLAMES */
#if defined(WLTEST)
	case IOV_SVAL(IOV_RPCALVARS): {
		const wl_rpcal_t *rpcal = p;
		if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			PHY_ERROR(("Number of TX Chain has to be > 1!\n"));
			err = BCME_UNSUPPORTED;
		} else {
			pi->sromi->rpcal2g = rpcal[WL_CHAN_FREQ_RANGE_2G].update ?
			rpcal[WL_CHAN_FREQ_RANGE_2G].value : pi->sromi->rpcal2g;
			pi->sromi->rpcal5gb0 = rpcal[WL_CHAN_FREQ_RANGE_5G_BAND0].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND0].value : pi->sromi->rpcal5gb0;
			pi->sromi->rpcal5gb1 = rpcal[WL_CHAN_FREQ_RANGE_5G_BAND1].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND1].value : pi->sromi->rpcal5gb1;
			pi->sromi->rpcal5gb2 = rpcal[WL_CHAN_FREQ_RANGE_5G_BAND2].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND2].value : pi->sromi->rpcal5gb2;
			pi->sromi->rpcal5gb3 = rpcal[WL_CHAN_FREQ_RANGE_5G_BAND3].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND3].value : pi->sromi->rpcal5gb3;

			if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
				pi->sromi->rpcal2gcore3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_2G].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_2G].value :
				pi->sromi->rpcal2gcore3;
				pi->sromi->rpcal5gb0core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND0].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND0].value :
				pi->sromi->rpcal5gb0core3;
				pi->sromi->rpcal5gb1core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND1].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND1].value :
				pi->sromi->rpcal5gb1core3;
				pi->sromi->rpcal5gb2core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND2].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND2].value :
				pi->sromi->rpcal5gb2core3;
				pi->sromi->rpcal5gb3core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND3].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND3].value :
				pi->sromi->rpcal5gb3core3;
			}
		}
		break;
	}

	case IOV_GVAL(IOV_RPCALVARS): {
		wl_rpcal_t *rpcal = a;
		if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			PHY_ERROR(("Number of TX Chain has to be > 1!\n"));
			err = BCME_UNSUPPORTED;
		} else {
			rpcal[WL_CHAN_FREQ_RANGE_2G].value = pi->sromi->rpcal2g;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND0].value = pi->sromi->rpcal5gb0;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND1].value = pi->sromi->rpcal5gb1;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND2].value = pi->sromi->rpcal5gb2;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND3].value = pi->sromi->rpcal5gb3;
			if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_2G].value =
				pi->sromi->rpcal2gcore3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND0].value =
				pi->sromi->rpcal5gb0core3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND1].value =
				pi->sromi->rpcal5gb1core3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND2].value =
				pi->sromi->rpcal5gb2core3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND3].value =
				pi->sromi->rpcal5gb3core3;
			}
		}
		break;
	}

	case IOV_GVAL(IOV_PHY_VCOCAL):
	case IOV_SVAL(IOV_PHY_VCOCAL):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phy_force_vcocal_acphy(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;
#endif // endif
#if (defined(BCMDBG) || defined(BCMDBG_DUMP))
	case IOV_SVAL(IOV_PHY_FORCE_GAINLEVEL):
	{
		wlc_phy_force_gainlevel_acphy(pi, (int16) int_val);
		break;
	}
#endif /* BCMDBG */
#if defined(WLTEST)
	case IOV_SVAL(IOV_PHY_FORCE_SPURMODE):
	{
		if (int_val == -1)
			pi->acphy_spuravoid_mode_override = 0;
		else
			pi->acphy_spuravoid_mode_override = 1;

		phy_ac_force_spurmode(pi->u.pi_acphy->rxspuri, (int16)int_val);
		break;
	}
	case IOV_GVAL(IOV_PHY_FORCE_SPURMODE):
	{
	  if (pi->acphy_spuravoid_mode_override == 1)
			*ret_int_ptr = pi->acphy_spuravoid_mode;
		else
			*ret_int_ptr = -1;
		break;
	}
#endif /* WLTEST */
#if defined(BCMDBG)
	case IOV_SVAL(IOV_PHY_FORCE_FDIQI):
	{
		wlc_phy_force_fdiqi_acphy(pi, (uint16) int_val);
		break;
	}
#endif /* BCMDBG */

	case IOV_SVAL(IOV_PHY_FORCE_CRSMIN):
	{
	    wlc_phy_force_crsmin_acphy(pi, p);
		break;
	}
	case IOV_SVAL(IOV_PHY_FORCE_LESISCALE):
	{
	    wlc_phy_force_lesiscale_acphy(pi, p);
		break;
	}
#if defined(BCMDBG)
	case IOV_SVAL(IOV_PHY_BTCOEX_DESENSE):
	{
	  wlc_phy_desense_btcoex_acphy(pi, int_val);
		break;
	}
#endif /* BCMDBG */
#ifndef WLC_DISABLE_ACI
#endif /* WLC_DISABLE_ACI */
	case IOV_SVAL(IOV_EDCRS):
	{

		if (int_val == 0)
		{
			W_REG(pi->sh->osh, &pi->regs->PHYREF_IFS_CTL_SEL_PRICRS, 0x000F);
		}
		else
		{
			W_REG(pi->sh->osh, &pi->regs->PHYREF_IFS_CTL_SEL_PRICRS, 0x0F0F);
		}
		break;
	}
	case IOV_SVAL(IOV_LP_MODE):
	{
		if ((int_val > 3) || (int_val < 1)) {
			PHY_ERROR(("LP MODE %d is not supported \n", (uint16)int_val));
		} else {
			wlc_phy_lp_mode(pi, (int8) int_val);
		}
		break;
	}
	case IOV_GVAL(IOV_LP_MODE):
	{
		if (ISACPHY(pi))
			*ret_int_ptr = pi->u.pi_acphy->acphy_lp_status;
		break;

	}
	case IOV_SVAL(IOV_LP_VCO_2G):
	{
		if ((int_val != 0) && (int_val != 1)) {
			PHY_ERROR(("LP MODE %d is not supported \n", (uint16)int_val));
		} else {
			wlc_phy_force_lpvco_2G(pi, (int8) int_val);
		}
		break;
	}
	case IOV_GVAL(IOV_LP_VCO_2G):
	{
		if (ISACPHY(pi))
			*ret_int_ptr = pi->u.pi_acphy->acphy_force_lpvco_2G;
		break;
	}
	case IOV_SVAL(IOV_SMTH):
	{
		if ((ACMAJORREV_1(pi->pubpi.phy_rev) &&
		     ACMINORREV_2(pi)) || ACMAJORREV_3(pi->pubpi.phy_rev) ||
		    ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			if (((int_val > SMTH_FOR_TXBF) || (int_val < SMTH_DISABLE)) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
				PHY_ERROR(("Smth %d is not supported \n", (uint16)int_val));
			} else {
				wlc_phy_smth(pi, (int8) int_val, SMTH_NODUMP);
			}
		} else {
			PHY_ERROR(("Smth is not supported for this chip \n"));
		}
		break;
	}
	case IOV_GVAL(IOV_SMTH):
	{
	  if ((ACMAJORREV_1(pi->pubpi.phy_rev) && ACMINORREV_2(pi)) ||
	      ACMAJORREV_33(pi->pubpi.phy_rev)) {
			if (ISACPHY(pi))
				*ret_int_ptr = pi->u.pi_acphy->acphy_enable_smth;
		} else {
			PHY_ERROR(("Smth is not supported for this chip \n"));
		}
		break;
	}
#if defined(WLTEST)
	case IOV_SVAL(IOV_RADIO_PD):
	{
		if (CHIPID(pi->sh->chip) == BCM4335_CHIP_ID) {
			if (int_val == 1) {
				pi->u.pi_acphy->acphy_4335_radio_pd_status = 1;
				wlc_phy_radio2069_pwrdwn_seq(pi);
			} else if (int_val == 0) {
				wlc_phy_radio2069_pwrup_seq(pi);
				pi->u.pi_acphy->acphy_4335_radio_pd_status = 0;
			} else {
				PHY_ERROR(("RADIO PD %d is not supported \n", (uint16)int_val));
			}
		} else {
			PHY_ERROR(("RADIO PD is not supported for this chip \n"));
		}
		break;
	}
	case IOV_GVAL(IOV_RADIO_PD):
	{
	    *ret_int_ptr = pi->u.pi_acphy->acphy_4335_radio_pd_status;
		break;
	}
#endif /* WLTEST */
	case IOV_GVAL(IOV_EDCRS):
	{
		if (R_REG(pi->sh->osh, &pi->regs->PHYREF_IFS_CTL_SEL_PRICRS) == 0x000F)
		{
			*ret_int_ptr = 0;
		}
		else if (R_REG(pi->sh->osh, &pi->regs->PHYREF_IFS_CTL_SEL_PRICRS) == 0x0F0F)
		{
			*ret_int_ptr = 1;
		}
		break;
	}
	case IOV_GVAL(IOV_PHY_AFE_OVERRIDE):
		*ret_int_ptr = (int32)pi->afe_override;
		break;
	case IOV_SVAL(IOV_PHY_AFE_OVERRIDE):
		pi->afe_override = (uint8)int_val;
		break;
#if defined(VASIP_HW_SUPPORT) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
	/* SVMP_SAMPLE_COLLECT */
	case IOV_GVAL(IOV_SVMP_SAMPLE_COLLECT):
		if (ISACPHY(pi) &&
			(ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)))
		{
			wl_svmp_sampcol_t svmp_sampcol_param;
			phy_ac_svmp_sampcol_params_get(pi, &svmp_sampcol_param);
			bcopy(&svmp_sampcol_param, a, sizeof(wl_svmp_sampcol_t));
		}
		break;
	case IOV_SVAL(IOV_SVMP_SAMPLE_COLLECT):
		if (ISACPHY(pi) &&
			(ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)))
		{
			wl_svmp_sampcol_t svmp_sampcol_param;
			bcopy(p, &svmp_sampcol_param, sizeof(wl_svmp_sampcol_t));
			if (svmp_sampcol_param.version != WL_SVMP_SAMPCOL_PARAMS_VERSION) {
				err = BCME_BADARG;
				break;
			}
			phy_ac_svmp_sampcol_params_set(pi, &svmp_sampcol_param);
		}
		break;
#endif /* defined(VASIP_HW_SUPPORT) && (defined(BCMDBG) || defined(BCMDBG_DUMP)) */
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovars_calib(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	BCM_REFERENCE(bool_val);
	BCM_REFERENCE(*ret_int_ptr);

	switch (actionid) {
#if defined(WLTEST) || defined(ATE_BUILD)
	case IOV_GVAL(IOV_PHY_TXIQCC):
	{
		int32 iqccValues[4];
		uint16 valuea = 0;
		uint16 valueb = 0;
		uint16 valuea1 = 0;
		uint16 valueb1 = 0;
		if (!(ISNPHY(pi))) {
			txiqccgetfn_t txiqcc_fn = pi->pi_fptr.txiqccget;
			if (txiqcc_fn) {
				(*txiqcc_fn)(pi, &valuea, &valueb);
				iqccValues[0] = valuea;
				iqccValues[1] = valueb;
				bcopy(iqccValues, a, 2*sizeof(int32));
			}
		} else {
			txiqccmimogetfn_t txiqcc_fn = pi->pi_fptr.txiqccmimoget;
			if (txiqcc_fn) {
				(*txiqcc_fn)(pi, &valuea, &valueb, &valuea1, &valueb1);
				iqccValues[0] = valuea;
				iqccValues[1] = valueb;
				iqccValues[2] = valuea1;
				iqccValues[3] = valueb1;
				bcopy(iqccValues, a, 4*sizeof(int32));
			}
		}
		break;
	}
	case IOV_SVAL(IOV_PHY_TXIQCC):
	{
		int32 iqccValues[4];
		uint16 valuea, valueb, valuea1, valueb1;
		if (!(ISNPHY(pi))) {
			txiqccsetfn_t txiqcc_fn = pi->pi_fptr.txiqccset;
			if (txiqcc_fn) {
				bcopy(p, iqccValues, 2*sizeof(int32));
				valuea = (uint16)(iqccValues[0]);
				valueb = (uint16)(iqccValues[1]);
				(*txiqcc_fn)(pi, valuea, valueb);
			}
		} else {
			txiqccmimosetfn_t txiqcc_fn = pi->pi_fptr.txiqccmimoset;
			if (txiqcc_fn) {
				bcopy(p, iqccValues, 4*sizeof(int32));
				valuea = (uint16)(iqccValues[0]);
				valueb = (uint16)(iqccValues[1]);
				valuea1 = (uint16)(iqccValues[2]);
				valueb1 = (uint16)(iqccValues[3]);
				(*txiqcc_fn)(pi, valuea, valueb, valuea1, valueb1);
			}
		}
		break;
	}
	case IOV_GVAL(IOV_PHY_TXLOCC):
	{
		uint16 di0dq0;
		uint16 di1dq1;
		uint8 *loccValues = a;

		if (!(ISNPHY(pi))) {
			txloccgetfn_t txlocc_fn = pi->pi_fptr.txloccget;
			radioloftgetfn_t radio_loft_fn = pi->pi_fptr.radioloftget;
			if ((txlocc_fn) && (radio_loft_fn))
			{
				/* copy the 6 bytes to a */
				di0dq0 = (*txlocc_fn)(pi);
				loccValues[0] = (uint8)(di0dq0 >> 8);
				loccValues[1] = (uint8)(di0dq0 & 0xff);
				(*radio_loft_fn)(pi, &loccValues[2], &loccValues[3],
					&loccValues[4], &loccValues[5]);
			}
		} else {
			txloccmimogetfn_t txlocc_fn = pi->pi_fptr.txloccmimoget;
			radioloftmimogetfn_t radio_loft_fn = pi->pi_fptr.radioloftmimoget;

			if ((txlocc_fn) && (radio_loft_fn))
			{
				/* copy the 6 bytes to a */
				(*txlocc_fn)(pi, &di0dq0, &di1dq1);
				loccValues[0] = (uint8)(di0dq0 >> 8);
				loccValues[1] = (uint8)(di0dq0 & 0xff);
				loccValues[6] = (uint8)(di1dq1 >> 8);
				loccValues[7] = (uint8)(di1dq1 & 0xff);
				(*radio_loft_fn)(pi, &loccValues[2], &loccValues[3],
					&loccValues[4], &loccValues[5], &loccValues[8],
					&loccValues[9], &loccValues[10], &loccValues[11]);
			}
		}
		break;
	}
	case IOV_SVAL(IOV_PHY_TXLOCC):
	{
		/* copy 6 bytes from a to radio */
		uint16 di0dq0, di1dq1;
		uint8 *loccValues = p;

		if (!(ISNPHY(pi))) {
			di0dq0 = ((uint16)loccValues[0] << 8) | loccValues[1];
			if (pi->pi_fptr.txloccset && pi->pi_fptr.radioloftset) {
				pi->pi_fptr.txloccset(pi, di0dq0);
				pi->pi_fptr.radioloftset(pi, loccValues[2],
					loccValues[3], loccValues[4], loccValues[5]);
			} else
			return BCME_UNSUPPORTED;
		} else {
			di0dq0 = ((uint16)loccValues[0] << 8) | loccValues[1];
			di1dq1 = ((uint16)loccValues[6] << 8) | loccValues[7];
			if (pi->pi_fptr.txloccmimoset && pi->pi_fptr.radioloftmimoset) {
				pi->pi_fptr.txloccmimoset(pi, di0dq0, di1dq1);
				pi->pi_fptr.radioloftmimoset(pi, loccValues[2],
					loccValues[3], loccValues[4], loccValues[5],
					loccValues[8], loccValues[9], loccValues[10],
					loccValues[11]);
			}
		}
		break;
	}
#endif // endif

#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX) || defined(ATE_BUILD) || \
	defined(BCMDBG_TEMPSENSE)
	case IOV_GVAL(IOV_PHY_TEMPSENSE):
		err = wlc_phy_iovar_tempsense(pi, ret_int_ptr);
		break;
#endif /* BCMDBG || WLTEST || MACOSX || ATE_BUILD || BCMDBG_TEMPSENSE */
#if defined(WLTEST) || defined(WLMEDIA_CALDBG)
	case IOV_GVAL(IOV_PHY_CAL_DISABLE):
		*ret_int_ptr = (int32)pi->disable_percal;
		break;

	case IOV_SVAL(IOV_PHY_CAL_DISABLE):
		pi->disable_percal = bool_val;
		break;
#endif // endif

#if defined(WLTEST)
	case IOV_GVAL(IOV_PHY_IDLETSSI):
		wlc_phy_iovar_idletssi(pi, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_PHY_IDLETSSI):
		wlc_phy_iovar_idletssi(pi, ret_int_ptr, bool_val);
		break;

	case IOV_GVAL(IOV_PHY_VBATSENSE):
		wlc_phy_iovar_vbatsense(pi, ret_int_ptr);
		break;

	case IOV_GVAL(IOV_PHY_IDLETSSI_REG):
		if (!pi->sh->clk)
			err = BCME_NOCLK;
		else
			err = wlc_phy_iovar_idletssi_reg(pi, ret_int_ptr, int_val, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_IDLETSSI_REG):
		if (!pi->sh->clk)
			err = BCME_NOCLK;
		else
			err = wlc_phy_iovar_idletssi_reg(pi, ret_int_ptr, int_val, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_AVGTSSI_REG):
		if (!pi->sh->clk)
			err = BCME_NOCLK;
		else
			wlc_phy_iovar_avgtssi_reg(pi, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_PHY_RESETCCA):
		if (ISNPHY(pi)) {
			wlc_phy_resetcca_nphy(pi);
		}
		else if (ISACPHY(pi)) {
			bool macSuspended;
			/* check if MAC already suspended */
			macSuspended = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
			if (!macSuspended) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			}
			wlc_phy_resetcca_acphy(pi);
			if (!macSuspended)
				wlapi_enable_mac(pi->sh->physhim);
		}
		break;
	case IOV_GVAL(IOV_PHY_PACALIDX0):

		if (ISLCNPHY(pi)) {
			uint32 papd_cal_idx;
			papd_cal_idx = 0;
			papd_cal_idx = (uint32) (pi->u.pi_lcnphy)->papd_lut0_cal_idx;
			bcopy(&papd_cal_idx, a, sizeof(uint32));
		} else if (ISACPHY(pi)) {
			int32 papd_cal_idx;
			papd_cal_idx = (int32) (pi->u.pi_acphy)->papd_lut0_cal_idx;
			bcopy(&papd_cal_idx, a, sizeof(int32));
		}

		break;
	case IOV_GVAL(IOV_PHY_PACALIDX1):

		if (ISLCNPHY(pi)) {
			uint32 papd_cal_idx;
			papd_cal_idx = 0;
			papd_cal_idx = (uint32) (pi->u.pi_lcnphy)->papd_lut1_cal_idx;
			bcopy(&papd_cal_idx, a, sizeof(uint32));
		} else if (ISACPHY(pi)) {
			uint32 papd_cal_idx;
			papd_cal_idx = (int32) (pi->u.pi_acphy)->papd_lut1_cal_idx;
			bcopy(&papd_cal_idx, a, sizeof(int32));
		}

		break;

	case IOV_SVAL(IOV_PHY_IQLOCALIDX):
		if (ISLCNPHY(pi)) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				OSL_DELAY(1000);
				(pi->u.pi_lcnphy)->iqlocalidx_2g = (int8) *ret_int_ptr;
				(pi->u.pi_lcnphy)->iqlocalidx2goffs = 0;
			}
#ifdef BAND5G
			else {
				OSL_DELAY(1000);
				(pi->u.pi_lcnphy)->iqlocalidx_5g = (int8) *ret_int_ptr;
				(pi->u.pi_lcnphy)->iqlocalidx5goffs = 0;
			}
#endif /* BAND5G */
		}
		break;

	case IOV_SVAL(IOV_PHY_PACALIDX):
		if (ISLCNPHY(pi)) {
				OSL_DELAY(1000);
				(pi->u.pi_lcnphy)->pacalidx = (int8) *ret_int_ptr;
		} else if (ISACPHY(pi)) {
				(pi->u.pi_acphy)->pacalidx_iovar = (int8) *ret_int_ptr;
		}
		break;

	case IOV_GVAL(IOV_PHYCAL_TEMPDELTA):
		*ret_int_ptr = (int32)pi->phycal_tempdelta;
		break;

	case IOV_SVAL(IOV_PHYCAL_TEMPDELTA):
		if (int_val == -1)
			pi->phycal_tempdelta = pi->phycal_tempdelta_default;
		else
			pi->phycal_tempdelta = (uint8)int_val;
		break;
#endif // endif
	case IOV_SVAL(IOV_PHY_SROM_TEMPSENSE):
	{
		pi->srom_rawtempsense = (int16)int_val;
		break;
	}

	case IOV_GVAL(IOV_PHY_SROM_TEMPSENSE):
	{
		*ret_int_ptr = pi->srom_rawtempsense;
		break;
	}
	case IOV_SVAL(IOV_PHY_RXGAIN_RSSI):
	{
		pi->u.pi_acphy->rxgaincal_rssical = (bool)int_val;
		break;
	}

	case IOV_GVAL(IOV_PHY_RXGAIN_RSSI):
	{
		*ret_int_ptr = pi->u.pi_acphy->rxgaincal_rssical;
		break;
	}
	case IOV_SVAL(IOV_PHY_GAIN_CAL_TEMP):
	{
		pi->srom_gain_cal_temp  = (int16)int_val;
		break;
	}
	case IOV_GVAL(IOV_PHY_GAIN_CAL_TEMP):
	{
		*ret_int_ptr = pi->srom_gain_cal_temp;
		break;
	}
	case IOV_SVAL(IOV_PHY_RSSI_CAL_REV):
	{
		pi->u.pi_acphy->rssi_cal_rev = (bool)int_val;
		break;
	}

	case IOV_GVAL(IOV_PHY_RSSI_CAL_REV):
	{
		*ret_int_ptr = pi->u.pi_acphy->rssi_cal_rev;
		break;
	}
	case IOV_SVAL(IOV_PHY_RUD_AGC_ENABLE):
	{
		pi->u.pi_acphy->rud_agc_enable = (bool)int_val;
		break;
	}

	case IOV_GVAL(IOV_PHY_RUD_AGC_ENABLE):
	{
		*ret_int_ptr = pi->u.pi_acphy->rud_agc_enable;
		break;
	}

#ifdef PHYMON
	case IOV_GVAL(IOV_PHYCAL_STATE): {
		if (alen < (int)sizeof(wl_phycal_state_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (ISNPHY(pi))
			err = wlc_phycal_state_nphy(pi, a, alen);
		else
			err = BCME_UNSUPPORTED;

		break;
	}
#endif /* PHYMON */
#if defined(WLTEST) || defined(AP)
	case IOV_GVAL(IOV_PHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, TRUE);
		break;
#endif // endif

	case IOV_GVAL(IOV_PHY_PERICAL_DELAY):
		*ret_int_ptr = (int32)pi->phy_cal_delay;
		break;

	case IOV_SVAL(IOV_PHY_PERICAL_DELAY):
		if ((int_val >= PHY_PERICAL_DELAY_MIN) && (int_val <= PHY_PERICAL_DELAY_MAX))
			pi->phy_cal_delay = (uint16)int_val;
		else
			err = BCME_RANGE;
		break;

	case IOV_GVAL(IOV_PHY_PAPD_DEBUG):
		break;

	case IOV_GVAL(IOV_NOISE_MEASURE):
#if LCNCONF
		if (ISLCNPHY(pi))
		  wlc_lcnphy_noise_measure_start(pi, TRUE);
#endif // endif
		int_val = 0;
		bcopy(&int_val, a, sizeof(int_val));
		break;
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovars_txpwrctl(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	BCM_REFERENCE(*ret_int_ptr);
	BCM_REFERENCE(bool_val);

	switch (actionid) {
#if defined(BCMDBG) || defined(WLTEST) || defined(BCMCCX)
	case IOV_GVAL(IOV_TXINSTPWR):
		phy_utils_phyreg_enter(pi);
		/* Return the current instantaneous est. power
		 * For swpwr ctrl it's based on current TSSI value (as opposed to average)
		 */
		wlc_phy_txpower_get_instant(pi, a);
		phy_utils_phyreg_exit(pi);
		break;
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
	case IOV_SVAL(IOV_TSSICAL_START_IDX):
	case IOV_SVAL(IOV_TSSICAL_START):
		if (ISLCNPHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			uint16 curr_anchor;
			if (!pi->ptssi_cal) {
				pi->ptssi_cal = (tssi_cal_info_t *)MALLOC(pi->sh->osh,
					sizeof(tssi_cal_info_t));
				if (pi->ptssi_cal == NULL) {
					PHY_ERROR(("wl%d: %s: MALLOC failure\n",
						pi->sh->unit, __FUNCTION__));
					err = BCME_UNSUPPORTED;
					break;
				}
				bzero((char *)pi->ptssi_cal, sizeof(tssi_cal_info_t));
			}
			curr_anchor = pi->ptssi_cal->curr_anchor;
			wlc_lcnphy_clear_tx_power_offsets(pi);
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, cckPwrOffset,
				pi_lcn->cckPwrOffset);
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmd, txPwrCtrl_en, 1);

			if (actionid == IOV_SVAL(IOV_TSSICAL_START_IDX))
				bcopy(p, &(pi->ptssi_cal->anchor_txidx[curr_anchor]),
					sizeof(uint16));
			else
				bcopy(p, &(pi->ptssi_cal->target_pwr_qdBm[curr_anchor]),
					sizeof(int));

			pi->ptssi_cal->paparams_calc_done = 0;
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_TSSICAL_POWER):
		if (ISLCNPHY(pi)) {
			uint16 curr_anchor;
			if (!pi->ptssi_cal) {
				err = BCME_UNSUPPORTED;
				break;
			}
			curr_anchor = pi->ptssi_cal->curr_anchor;
			bcopy(p, &(pi->ptssi_cal->measured_pwr_qdBm[curr_anchor]), sizeof(int));
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_TSSICAL_POWER):
		if (ISLCNPHY(pi)) {
			uint16 anchor_var[3];
			uint16 curr_anchor;
			if (!pi->ptssi_cal) {
				err = BCME_UNSUPPORTED;
				break;
			}
			curr_anchor = pi->ptssi_cal->curr_anchor;
			pi->ptssi_cal->anchor_txidx[curr_anchor] =
				wlc_lcnphy_get_current_tx_pwr_idx(pi);
			pi->ptssi_cal->anchor_tssi[curr_anchor] =
				PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
			pi->ptssi_cal->anchor_bbmult[curr_anchor] =
				wlc_lcnphy_get_bbmult_from_index(pi,
				pi->ptssi_cal->anchor_txidx[curr_anchor]);

			anchor_var[0] = pi->ptssi_cal->anchor_bbmult[curr_anchor];
			anchor_var[1] = pi->ptssi_cal->anchor_txidx[curr_anchor];
			anchor_var[2] = pi->ptssi_cal->anchor_tssi[curr_anchor];

			bcopy(anchor_var, a, 3*sizeof(uint16));

			pi->ptssi_cal->curr_anchor++;
			if (pi->ptssi_cal->curr_anchor >= MAX_NUM_ANCHORS)
				pi->ptssi_cal->curr_anchor = 0;

		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_TSSICAL_PARAMS):
		if (ISLCNPHY(pi)) {
			uint16 num_anchor;
			if (!pi->ptssi_cal) {
				err = BCME_UNSUPPORTED;
				break;
			}
			if ((!pi->ptssi_cal->paparams_calc_done) &&
				(!pi->ptssi_cal->paparams_calc_in_progress)) {

				if (pi->ptssi_cal->anchor_bbmult[0]) { /* Atleast One Anchor */

					pi->ptssi_cal->paparams_calc_in_progress = 1;
					wlc_phy_tssi_cal(pi);

					pi->ptssi_cal->paparams_new[0] =
						pi->ptssi_cal->rsd.c4[0][0];
					pi->ptssi_cal->paparams_new[1] =
						pi->ptssi_cal->rsd.c4[1][0];
					pi->ptssi_cal->paparams_new[2] =
						pi->ptssi_cal->rsd.c4[2][0];
					pi->ptssi_cal->paparams_new[3] =
						pi->ptssi_cal->rsd.det_c1;
				}
				else {
					pi->ptssi_cal->paparams_new[0] = 1;
					pi->ptssi_cal->paparams_new[1] = 1;
					pi->ptssi_cal->paparams_new[2] = 1;
					pi->ptssi_cal->paparams_new[3] = 1;
				}

				pi->ptssi_cal->paparams_calc_done = 1;
			}

			if (pi->ptssi_cal->paparams_calc_done) {
				bcopy(pi->ptssi_cal->paparams_new, a, 4*sizeof(int64));
				pi->ptssi_cal->paparams_calc_in_progress = 0;
				pi->ptssi_cal->curr_anchor = 0;
				for (num_anchor = 0; num_anchor < MAX_NUM_ANCHORS; num_anchor++)
					pi->ptssi_cal->anchor_bbmult[num_anchor] = 0;
			}
		} else
			err = BCME_UNSUPPORTED;
		break;
	case IOV_SVAL(IOV_PHY_TSSITXDELAY):
		if (ISLCNPHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			pi_lcn->lcnphy_tssical_txdelay = (uint32)int_val;
		}
		break;

	case IOV_GVAL(IOV_PHY_TSSITXDELAY):
		if (ISLCNPHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			int_val = pi_lcn->lcnphy_tssical_txdelay;
			bcopy(&int_val, a, sizeof(int_val));
		}
		break;
#endif /* BCMDBG || WLTEST */

#if defined(WLTEST) || defined(ATE_BUILD)
	case IOV_GVAL(IOV_PHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_PHY_TXPWRINDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_iovar_txpwrindex_set(pi, p);
		break;

	case IOV_GVAL(IOV_PHY_TXPWRINDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		wlc_phy_iovar_txpwrindex_get(pi, int_val, bool_val, ret_int_ptr);
		break;
#endif // endif
#if defined(WLTEST)
	case IOV_GVAL(IOV_PATRIM):
		if (ISACPHY(pi))
			wlc_phy_iovar_patrim_acphy(pi, ret_int_ptr);
		else
			*ret_int_ptr = 0;
	break;

	case IOV_GVAL(IOV_PAVARS): {
		uint16 *outpa = a;
		uint16 inpa[WL_PHY_PAVARS_LEN];
		uint j = 3;	/* PA parameters start from offset 3 */

		bcopy(p, inpa, sizeof(inpa));

		outpa[0] = inpa[0]; /* Phy type */
		outpa[1] = inpa[1]; /* Band range */
		outpa[2] = inpa[2]; /* Chain */

		if (ISHTPHY(pi)) {
			if (inpa[0] != PHY_TYPE_HT) {
				outpa[0] = PHY_TYPE_NULL;
				break;
			}

			if (inpa[2] >= PHYCORENUM(pi->pubpi.phy_corenum))
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
			case WL_CHAN_FREQ_RANGE_5G_BAND0:
			case WL_CHAN_FREQ_RANGE_5G_BAND1:
			case WL_CHAN_FREQ_RANGE_5G_BAND2:
			case WL_CHAN_FREQ_RANGE_5G_BAND3:
				wlc_phy_pavars_get_htphy(pi, &outpa[j], inpa[1], inpa[2]);
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			}
		} else if (ISNPHY(pi)) {
			srom_pwrdet_t	*pwrdet  = pi->pwrdet;

			if (inpa[0] != PHY_TYPE_N) {
				outpa[0] = PHY_TYPE_NULL;
				break;
			}
			outpa[j++] = pwrdet->pwrdet_a1[inpa[2]][inpa[1]];	/* a1 */
			outpa[j++] = pwrdet->pwrdet_b0[inpa[2]][inpa[1]];	/* b0 */
			outpa[j++] = pwrdet->pwrdet_b1[inpa[2]][inpa[1]];	/* b1 */
		} else if (ISLCNCOMMONPHY(pi)) {
			if (((inpa[0] != PHY_TYPE_LCN) && (inpa[0] != PHY_TYPE_LCN40))) {
				outpa[0] = PHY_TYPE_NULL;
				break;
			}

			if (inpa[2] != 0)
				return BCME_BADARG;
#if defined(LCNCONF)
			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
				outpa[j++] = pi->txpa_2g[0];		/* b0 */
				outpa[j++] = pi->txpa_2g[1];		/* b1 */
				outpa[j++] = pi->txpa_2g[2];		/* a1 */
				break;
#ifdef BAND5G
			case WL_CHAN_FREQ_RANGE_5GL:
				outpa[j++] = pi->txpa_5g_low[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_low[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_low[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				outpa[j++] = pi->txpa_5g_mid[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_mid[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_mid[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				outpa[j++] = pi->txpa_5g_hi[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_hi[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_hi[2];	/* a1 */
				break;
#endif /* BAND5G */
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[0]));
				break;
			}
#else
			return BCME_UNSUPPORTED;
#endif /* older PHYs */
		} else if (ISACPHY(pi)) {
			int chain = inpa[2];
			int freq_range;
			int num_paparams = PHY_CORE_MAX;

#ifdef WL_CHAN_FREQ_RANGE_5G_4BAND
			int n;
#endif // endif
			freq_range = inpa[1];

			if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
				(ACMAJORREV_1(pi->pubpi.phy_rev) ||
				ACMAJORREV_3(pi->pubpi.phy_rev))) {
				num_paparams = 3;
			} else if ((ACMAJORREV_2(pi->pubpi.phy_rev) ||
				(ACMAJORREV_4(pi->pubpi.phy_rev))) &&
				BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
				num_paparams = 4;
			} else if ((pi->u.pi_acphy->srom_tworangetssi2g) &&
			 (inpa[1] == WL_CHAN_FREQ_RANGE_2G) && pi->ipa2g_on &&
				(ACMAJORREV_1(pi->pubpi.phy_rev))) {
				num_paparams = 2;
			} else if ((pi->u.pi_acphy->srom_tworangetssi5g) &&
			 (inpa[1] != WL_CHAN_FREQ_RANGE_2G) && pi->ipa5g_on &&
				(ACMAJORREV_1(pi->pubpi.phy_rev))) {
				num_paparams = 2;
			}
			if (inpa[0] != PHY_TYPE_AC) {
				PHY_ERROR(("Wrong phy type %d\n", inpa[0]));
				outpa[0] = PHY_TYPE_NULL;
				break;
			}
			if (chain > (num_paparams - 1)) {
				PHY_ERROR(("Wrong chain number %d\n", chain));
				outpa[0] = PHY_TYPE_NULL;
				break;
			}

			if (SROMREV(pi->sh->sromrev) >= 12) {
			    srom12_pwrdet_t *pwrdet = pi->pwrdet_ac;
			    switch (freq_range) {
			    case WL_CHAN_FREQ_RANGE_2G:
				outpa[j++] = pwrdet->pwrdet_a[chain][freq_range];
				outpa[j++] = pwrdet->pwrdet_b[chain][freq_range];
				outpa[j++] = pwrdet->pwrdet_c[chain][freq_range];
				outpa[j++] = pwrdet->pwrdet_d[chain][freq_range];
				break;
			    case WL_CHAN_FREQ_RANGE_2G_40:
				outpa[j++] = pwrdet->pwrdet_a_40[chain][freq_range-6];
				outpa[j++] = pwrdet->pwrdet_b_40[chain][freq_range-6];
				outpa[j++] = pwrdet->pwrdet_c_40[chain][freq_range-6];
				outpa[j++] = pwrdet->pwrdet_d_40[chain][freq_range-6];
				break;
				/* allow compile in branches without 4BAND definition */
#ifdef WL_CHAN_FREQ_RANGE_5G_4BAND
			    case WL_CHAN_FREQ_RANGE_5G_BAND4:
				outpa[j++] = pwrdet->pwrdet_a[chain][freq_range];
				outpa[j++] = pwrdet->pwrdet_b[chain][freq_range];
				outpa[j++] = pwrdet->pwrdet_c[chain][freq_range];
				outpa[j++] = pwrdet->pwrdet_d[chain][freq_range];
				break;
			    case WL_CHAN_FREQ_RANGE_5G_BAND0:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3:
				if (ACMAJORREV_2(pi->pubpi.phy_rev) ||
				    ACMAJORREV_5(pi->pubpi.phy_rev)) {
				    outpa[j++] = pwrdet->pwrdet_a[chain][freq_range];
				    outpa[j++] = pwrdet->pwrdet_b[chain][freq_range];
				    outpa[j++] = pwrdet->pwrdet_c[chain][freq_range];
				    outpa[j++] = pwrdet->pwrdet_d[chain][freq_range];
				} else {
				    PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				}
				break;
			    case WL_CHAN_FREQ_RANGE_5G_BAND0_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND4_40:
				outpa[j++] = pwrdet->pwrdet_a_40[chain][freq_range-6];
				outpa[j++] = pwrdet->pwrdet_b_40[chain][freq_range-6];
				outpa[j++] = pwrdet->pwrdet_c_40[chain][freq_range-6];
				outpa[j++] = pwrdet->pwrdet_d_40[chain][freq_range-6];
				break;
			    case WL_CHAN_FREQ_RANGE_5G_BAND0_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND4_80:
				outpa[j++] = pwrdet->pwrdet_a_80[chain][freq_range-12];
				outpa[j++] = pwrdet->pwrdet_b_80[chain][freq_range-12];
				outpa[j++] = pwrdet->pwrdet_c_80[chain][freq_range-12];
				outpa[j++] = pwrdet->pwrdet_d_80[chain][freq_range-12];
				break;
			    case WL_CHAN_FREQ_RANGE_5G_5BAND:
				for (n = 1; n <= 5; n++) {
				    outpa[j++] = pwrdet->pwrdet_a[chain][n];
				    outpa[j++] = pwrdet->pwrdet_b[chain][n];
				    outpa[j++] = pwrdet->pwrdet_c[chain][n];
				    outpa[j++] = pwrdet->pwrdet_d[chain][n];
				}
				break;
			    case WL_CHAN_FREQ_RANGE_5G_5BAND_40:
				for (n = 1; n <= 5; n++) {
				    outpa[j++] = pwrdet->pwrdet_a_40[chain][n];
				    outpa[j++] = pwrdet->pwrdet_b_40[chain][n];
				    outpa[j++] = pwrdet->pwrdet_c_40[chain][n];
				    outpa[j++] = pwrdet->pwrdet_d_40[chain][n];
				}
				break;
			    case WL_CHAN_FREQ_RANGE_5G_5BAND_80:
				for (n = 0; n <= 4; n++) {
				    outpa[j++] = pwrdet->pwrdet_a_80[chain][n];
				    outpa[j++] = pwrdet->pwrdet_b_80[chain][n];
				    outpa[j++] = pwrdet->pwrdet_c_80[chain][n];
				    outpa[j++] = pwrdet->pwrdet_d_80[chain][n];
				}
				break;
#endif /* WL_CHAN_FREQ_RANGE_5G_4BAND */
			    default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			    }
			} else {
			    srom11_pwrdet_t *pwrdet11 = pi->pwrdet_ac;
			    switch (freq_range) {
			    case WL_CHAN_FREQ_RANGE_2G:
				outpa[j++] = pwrdet11->pwrdet_a1[chain][freq_range];
				outpa[j++] = pwrdet11->pwrdet_b0[chain][freq_range];
				outpa[j++] = pwrdet11->pwrdet_b1[chain][freq_range];
				break;
				/* allow compile in branches without 4BAND definition */
#ifdef WL_CHAN_FREQ_RANGE_5G_4BAND
			    case WL_CHAN_FREQ_RANGE_5G_4BAND:
				for (n = 1; n <= 4; n ++) {
				    outpa[j++] = pwrdet11->pwrdet_a1[chain][n];
				    outpa[j++] = pwrdet11->pwrdet_b0[chain][n];
				    outpa[j++] = pwrdet11->pwrdet_b1[chain][n];
				}
				break;
			    case WL_CHAN_FREQ_RANGE_5G_BAND0:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3:
				if (ACMAJORREV_2(pi->pubpi.phy_rev) ||
				    ACMAJORREV_5(pi->pubpi.phy_rev)) {
				    outpa[j++] = pwrdet11->pwrdet_a1[chain][freq_range];
				    outpa[j++] = pwrdet11->pwrdet_b0[chain][freq_range];
				    outpa[j++] = pwrdet11->pwrdet_b1[chain][freq_range];
				} else {
				    PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				}
				break;
			    default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			    }
#endif /* WL_CHAN_FREQ_RANGE_5G_4BAND */

			}
		} else {
		    PHY_ERROR(("Unsupported PHY type!\n"));
		    err = BCME_UNSUPPORTED;
		}
	}
	    break;

	case IOV_SVAL(IOV_PAVARS): {
		uint16 inpa[WL_PHY_PAVARS_LEN];
		uint j = 3;	/* PA parameters start from offset 3 */
		int chain;
		int freq_range;
		int num_paparams;
		bcopy(p, inpa, sizeof(inpa));
		if (ISHTPHY(pi)) {
			if (inpa[0] != PHY_TYPE_HT) {
				break;
			}

			if (inpa[2] >= PHYCORENUM(pi->pubpi.phy_corenum))
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
			case WL_CHAN_FREQ_RANGE_5G_BAND0:
			case WL_CHAN_FREQ_RANGE_5G_BAND1:
			case WL_CHAN_FREQ_RANGE_5G_BAND2:
			case WL_CHAN_FREQ_RANGE_5G_BAND3:
				wlc_phy_pavars_set_htphy(pi, &inpa[j], inpa[1], inpa[2]);
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			}
		} else if (ISNPHY(pi)) {
			srom_pwrdet_t	*pwrdet  = pi->pwrdet;

			if (inpa[0] != PHY_TYPE_N)
				break;

			if (inpa[2] > 1)
				return BCME_BADARG;

			pwrdet->pwrdet_a1[inpa[2]][inpa[1]] = inpa[j++];
			pwrdet->pwrdet_b0[inpa[2]][inpa[1]] = inpa[j++];
			pwrdet->pwrdet_b1[inpa[2]][inpa[1]] = inpa[j++];

		} else if (ISLCNCOMMONPHY(pi)) {
			if ((inpa[0] != PHY_TYPE_LCN) && (inpa[0] != PHY_TYPE_LCN40))
				break;

			if (inpa[2] != 0)
				return BCME_BADARG;
#if defined(LCNCONF)
			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
				pi->txpa_2g[0] = inpa[j++];	/* b0 */
				pi->txpa_2g[1] = inpa[j++];	/* b1 */
				pi->txpa_2g[2] = inpa[j++];	/* a1 */
				break;
#ifdef BAND5G
			case WL_CHAN_FREQ_RANGE_5GL:
				pi->txpa_5g_low[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_low[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_low[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				pi->txpa_5g_mid[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_mid[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_mid[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				pi->txpa_5g_hi[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_hi[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_hi[2] = inpa[j++];	/* a1 */
				break;
#endif /* BAND5G */
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[0]));
				break;
			}
#else
			return BCME_UNSUPPORTED;
#endif /* Older PHYs */
		} else if (ISACPHY(pi)) {
#ifdef WL_CHAN_FREQ_RANGE_5G_4BAND
			int n;
#endif // endif
			chain = inpa[2];
			freq_range = inpa[1];
			num_paparams = PHY_CORE_MAX;

			if ((BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) &&
				(ACMAJORREV_1(pi->pubpi.phy_rev) ||
				ACMAJORREV_3(pi->pubpi.phy_rev))) {
				num_paparams = 3;
			} else if ((ACMAJORREV_2(pi->pubpi.phy_rev) ||
				ACMAJORREV_4(pi->pubpi.phy_rev)) &&
				BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) {
				num_paparams = 4;
			} else if ((pi->u.pi_acphy->srom_tworangetssi2g) &&
			 (inpa[1] == WL_CHAN_FREQ_RANGE_2G) && pi->ipa2g_on &&
				(ACMAJORREV_1(pi->pubpi.phy_rev))) {
				num_paparams = 2;
			} else if ((pi->u.pi_acphy->srom_tworangetssi5g) &&
			 (inpa[1] != WL_CHAN_FREQ_RANGE_2G) && pi->ipa5g_on &&
				(ACMAJORREV_1(pi->pubpi.phy_rev))) {
				num_paparams = 2;
			}

			if (inpa[0] != PHY_TYPE_AC) {
				PHY_ERROR(("Wrong phy type %d\n", inpa[0]));
				break;
			}

			if (chain > (num_paparams - 1)) {
				PHY_ERROR(("Wrong chain number %d\n", chain));
				break;
			}

			if (SROMREV(pi->sh->sromrev) >= 12) {
			    srom12_pwrdet_t *pwrdet = pi->pwrdet_ac;
			    switch (freq_range) {
			    case WL_CHAN_FREQ_RANGE_2G:
				pwrdet->pwrdet_a[chain][freq_range] = inpa[j++];
				pwrdet->pwrdet_b[chain][freq_range] = inpa[j++];
				pwrdet->pwrdet_c[chain][freq_range] = inpa[j++];
				pwrdet->pwrdet_d[chain][freq_range] = inpa[j++];
				break;
			    case WL_CHAN_FREQ_RANGE_2G_40:
				pwrdet->pwrdet_a_40[chain][freq_range-6] = inpa[j++];
				pwrdet->pwrdet_b_40[chain][freq_range-6] = inpa[j++];
				pwrdet->pwrdet_c_40[chain][freq_range-6] = inpa[j++];
				pwrdet->pwrdet_d_40[chain][freq_range-6] = inpa[j++];
				break;
				/* allow compile in branches without 4BAND definition */
#ifdef WL_CHAN_FREQ_RANGE_5G_4BAND
			    case WL_CHAN_FREQ_RANGE_5G_BAND4:
				pwrdet->pwrdet_a[chain][freq_range] = inpa[j++];
				pwrdet->pwrdet_b[chain][freq_range] = inpa[j++];
				pwrdet->pwrdet_c[chain][freq_range] = inpa[j++];
				pwrdet->pwrdet_d[chain][freq_range] = inpa[j++];
				break;

			    case WL_CHAN_FREQ_RANGE_5G_BAND0:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3:
				if (ACMAJORREV_2(pi->pubpi.phy_rev) ||
				    ACMAJORREV_5(pi->pubpi.phy_rev)) {
				    pwrdet->pwrdet_a[chain][freq_range] = inpa[j++];
				    pwrdet->pwrdet_b[chain][freq_range] = inpa[j++];
				    pwrdet->pwrdet_c[chain][freq_range] = inpa[j++];
				    pwrdet->pwrdet_d[chain][freq_range] = inpa[j++];
				} else {
				    PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				}
				break;
			    case WL_CHAN_FREQ_RANGE_5G_BAND0_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3_40:
			    case WL_CHAN_FREQ_RANGE_5G_BAND4_40:
				pwrdet->pwrdet_a_40[chain][freq_range-6] = inpa[j++];
				pwrdet->pwrdet_b_40[chain][freq_range-6] = inpa[j++];
				pwrdet->pwrdet_c_40[chain][freq_range-6] = inpa[j++];
				pwrdet->pwrdet_d_40[chain][freq_range-6] = inpa[j++];
				break;
			    case WL_CHAN_FREQ_RANGE_5G_BAND0_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3_80:
			    case WL_CHAN_FREQ_RANGE_5G_BAND4_80:
				pwrdet->pwrdet_a_80[chain][freq_range-12] = inpa[j++];
				pwrdet->pwrdet_b_80[chain][freq_range-12] = inpa[j++];
				pwrdet->pwrdet_c_80[chain][freq_range-12] = inpa[j++];
				pwrdet->pwrdet_d_80[chain][freq_range-12] = inpa[j++];
				break;
			    case WL_CHAN_FREQ_RANGE_5G_5BAND:
				for (n = 1; n <= 5; n++) {
				    pwrdet->pwrdet_a[chain][n] = inpa[j++];
				    pwrdet->pwrdet_b[chain][n] = inpa[j++];
				    pwrdet->pwrdet_c[chain][n] = inpa[j++];
				    pwrdet->pwrdet_d[chain][n] = inpa[j++];
				}
				break;
			    case WL_CHAN_FREQ_RANGE_5G_5BAND_40:
				for (n = 1; n <= 5; n++) {
				    pwrdet->pwrdet_a_40[chain][n] = inpa[j++];
				    pwrdet->pwrdet_b_40[chain][n] = inpa[j++];
				    pwrdet->pwrdet_c_40[chain][n] = inpa[j++];
				    pwrdet->pwrdet_d_40[chain][n] = inpa[j++];
				}
				break;
			    case WL_CHAN_FREQ_RANGE_5G_5BAND_80:
				for (n = 0; n <= 4; n++) {
				    pwrdet->pwrdet_a_80[chain][n] = inpa[j++];
				    pwrdet->pwrdet_b_80[chain][n] = inpa[j++];
				    pwrdet->pwrdet_c_80[chain][n] = inpa[j++];
				    pwrdet->pwrdet_d_80[chain][n] = inpa[j++];
				}
				break;
#endif /* WL_CHAN_FREQ_RANGE_5G_4BAND */
			    default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			    }
			} else {
			    srom11_pwrdet_t *pwrdet11 = pi->pwrdet_ac;
			    switch (freq_range) {
			    case WL_CHAN_FREQ_RANGE_2G:
				pwrdet11->pwrdet_a1[chain][freq_range] = inpa[j++];
				pwrdet11->pwrdet_b0[chain][freq_range] = inpa[j++];
				pwrdet11->pwrdet_b1[chain][freq_range] = inpa[j++];
				break;
				/* allow compile in branches without 4BAND definition */
#ifdef WL_CHAN_FREQ_RANGE_5G_4BAND
			    case WL_CHAN_FREQ_RANGE_5G_4BAND:
				for (n = 1; n <= 4; n ++) {
				    pwrdet11->pwrdet_a1[chain][n] = inpa[j++];
				    pwrdet11->pwrdet_b0[chain][n] = inpa[j++];
				    pwrdet11->pwrdet_b1[chain][n] = inpa[j++];
				}
				break;

			    case WL_CHAN_FREQ_RANGE_5G_BAND0:
			    case WL_CHAN_FREQ_RANGE_5G_BAND1:
			    case WL_CHAN_FREQ_RANGE_5G_BAND2:
			    case WL_CHAN_FREQ_RANGE_5G_BAND3:
				if (ACMAJORREV_2(pi->pubpi.phy_rev) ||
				    ACMAJORREV_5(pi->pubpi.phy_rev)) {
				    pwrdet11->pwrdet_a1[chain][freq_range] = inpa[j++];
				    pwrdet11->pwrdet_b0[chain][freq_range] = inpa[j++];
				    pwrdet11->pwrdet_b1[chain][freq_range] = inpa[j++];
				} else {
				    PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				}
				break;
#endif /* WL_CHAN_FREQ_RANGE_5G_4BAND */
			    default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			    }
			}
		} else {
		    PHY_ERROR(("Unsupported PHY type!\n"));
		    err = BCME_UNSUPPORTED;
		}
	}
	    break;
	case IOV_GVAL(IOV_PAVARS2): {
			wl_pavars2_t *invar = (wl_pavars2_t *)p;
			wl_pavars2_t *outvar = (wl_pavars2_t *)a;
			uint16 *outpa = outvar->inpa;
			uint j = 0; /* PA parameters start from offset */

			if (invar->ver	!= WL_PHY_PAVAR_VER) {
				PHY_ERROR(("Incompatible version; use %d expected version %d\n",
					invar->ver, WL_PHY_PAVAR_VER));
				return BCME_BADARG;
			}

			outvar->ver = WL_PHY_PAVAR_VER;
			outvar->len = sizeof(wl_pavars2_t);
			if (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)
				== invar->bandrange)
				outvar->inuse = 1;
			else
				outvar->inuse = 0;

#ifdef BAND5G
			if (pi->sromi->subband5Gver == PHY_SUBBAND_5BAND) {
				if ((invar->bandrange == WL_CHAN_FREQ_RANGE_5GL) ||
					(invar->bandrange == WL_CHAN_FREQ_RANGE_5GM) ||
					(invar->bandrange == WL_CHAN_FREQ_RANGE_5GH)) {
					outvar->phy_type = PHY_TYPE_NULL;
					break;
				}
			}

			if (pi->sromi->subband5Gver == PHY_SUBBAND_3BAND_JAPAN) {
				if ((invar->bandrange == WL_CHAN_FREQ_RANGE_5GLL_5BAND) ||
					(invar->bandrange == WL_CHAN_FREQ_RANGE_5GLH_5BAND) ||
					(invar->bandrange == WL_CHAN_FREQ_RANGE_5GML_5BAND) ||
					(invar->bandrange == WL_CHAN_FREQ_RANGE_5GMH_5BAND) ||
					(invar->bandrange == WL_CHAN_FREQ_RANGE_5GH_5BAND)) {
					outvar->phy_type = PHY_TYPE_NULL;
					break;
				}
			}
#endif /* BAND5G */

			if (ISHTPHY(pi)) {
				if (invar->phy_type != PHY_TYPE_HT) {
					outvar->phy_type = PHY_TYPE_NULL;
					break;
				}

				if (invar->chain >= PHYCORENUM(pi->pubpi.phy_corenum))
					return BCME_BADARG;

				switch (invar->bandrange) {
				case WL_CHAN_FREQ_RANGE_2G:
				case WL_CHAN_FREQ_RANGE_5GL:
				case WL_CHAN_FREQ_RANGE_5GM:
				case WL_CHAN_FREQ_RANGE_5GH:
				case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
				case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
				case WL_CHAN_FREQ_RANGE_5GML_5BAND:
				case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
				case WL_CHAN_FREQ_RANGE_5GH_5BAND:
					wlc_phy_pavars_get_htphy(pi, &outpa[j], invar->bandrange,
						invar->chain);
					break;
				default:
					PHY_ERROR(("bandrange %d is out of scope\n",
						invar->bandrange));
					break;
				}
			} else if (ISLCNCOMMONPHY(pi)) {
				if ((invar->phy_type != PHY_TYPE_LCN) &&
					(invar->phy_type != PHY_TYPE_LCN40)) {
					outvar->phy_type = PHY_TYPE_NULL;
					break;
				}

				if (invar->chain != 0)
					return BCME_BADARG;

				switch (invar->bandrange) {
				case WL_CHAN_FREQ_RANGE_2G:
					outpa[j++] = pi->txpa_2g[0];		/* b0 */
					outpa[j++] = pi->txpa_2g[1];		/* b1 */
					outpa[j++] = pi->txpa_2g[2];		/* a1 */
					break;
#ifdef BAND5G
				case WL_CHAN_FREQ_RANGE_5GL:
					outpa[j++] = pi->txpa_5g_low[0];	/* b0 */
					outpa[j++] = pi->txpa_5g_low[1];	/* b1 */
					outpa[j++] = pi->txpa_5g_low[2];	/* a1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GM:
					outpa[j++] = pi->txpa_5g_mid[0];	/* b0 */
					outpa[j++] = pi->txpa_5g_mid[1];	/* b1 */
					outpa[j++] = pi->txpa_5g_mid[2];	/* a1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GH:
					outpa[j++] = pi->txpa_5g_hi[0]; /* b0 */
					outpa[j++] = pi->txpa_5g_hi[1]; /* b1 */
					outpa[j++] = pi->txpa_5g_hi[2]; /* a1 */
					break;
#endif /* BAND5G */
				default:
					PHY_ERROR(("bandrange %d is out of scope\n",
						invar->bandrange));
					break;
				}
			} else {
				PHY_ERROR(("Unsupported PHY type!\n"));
				err = BCME_UNSUPPORTED;
			}
		}
		break;

		case IOV_SVAL(IOV_PAVARS2): {
			wl_pavars2_t *invar = (wl_pavars2_t *)p;
			uint16 *inpa = invar->inpa;
			uint j = 0; /* PA parameters start from offset */

			if (invar->ver	!= WL_PHY_PAVAR_VER) {
				PHY_ERROR(("Incompatible version; use %d expected version %d\n",
					invar->ver, WL_PHY_PAVAR_VER));
				return BCME_BADARG;
			}

			if (ISHTPHY(pi)) {
				if (invar->phy_type != PHY_TYPE_HT) {
					break;
				}

				if (invar->chain >= PHYCORENUM(pi->pubpi.phy_corenum))
					return BCME_BADARG;

				switch (invar->bandrange) {
				case WL_CHAN_FREQ_RANGE_2G:
				case WL_CHAN_FREQ_RANGE_5GL:
				case WL_CHAN_FREQ_RANGE_5GM:
				case WL_CHAN_FREQ_RANGE_5GH:
				case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
				case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
				case WL_CHAN_FREQ_RANGE_5GML_5BAND:
				case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
				case WL_CHAN_FREQ_RANGE_5GH_5BAND:
					if (invar->bandrange < (CH_2G_GROUP + CH_5G_4BAND)) {
						wlc_phy_pavars_set_htphy(pi, &inpa[j],
							invar->bandrange, invar->chain);
					} else {
						err = BCME_RANGE;
						PHY_ERROR(("bandrange %d is out of scope\n",
							invar->bandrange));
					}
					break;
				default:
					err = BCME_RANGE;
					PHY_ERROR(("bandrange %d is out of scope\n",
						invar->bandrange));
					break;
				}
			} else if (ISLCNCOMMONPHY(pi)) {
				if ((invar->phy_type != PHY_TYPE_LCN) &&
					(invar->phy_type != PHY_TYPE_LCN40))
					break;

				if (invar->chain != 0)
					return BCME_BADARG;

				switch (invar->bandrange) {
				case WL_CHAN_FREQ_RANGE_2G:
					pi->txpa_2g[0] = inpa[j++]; /* b0 */
					pi->txpa_2g[1] = inpa[j++]; /* b1 */
					pi->txpa_2g[2] = inpa[j++]; /* a1 */
					break;
#ifdef BAND5G
				case WL_CHAN_FREQ_RANGE_5GL:
					pi->txpa_5g_low[0] = inpa[j++]; /* b0 */
					pi->txpa_5g_low[1] = inpa[j++]; /* b1 */
					pi->txpa_5g_low[2] = inpa[j++]; /* a1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GM:
					pi->txpa_5g_mid[0] = inpa[j++]; /* b0 */
					pi->txpa_5g_mid[1] = inpa[j++]; /* b1 */
					pi->txpa_5g_mid[2] = inpa[j++]; /* a1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GH:
					pi->txpa_5g_hi[0] = inpa[j++];	/* b0 */
					pi->txpa_5g_hi[1] = inpa[j++];	/* b1 */
					pi->txpa_5g_hi[2] = inpa[j++];	/* a1 */
					break;
#endif /* BAND5G */
				default:
					PHY_ERROR(("bandrange %d is out of scope\n",
						invar->bandrange));
					break;
				}
			} else {
				PHY_ERROR(("Unsupported PHY type!\n"));
				err = BCME_UNSUPPORTED;
			}
		}
		break;

	case IOV_GVAL(IOV_POVARS): {
		wl_po_t tmppo;

		/* tmppo has the input phy_type and band */
		bcopy(p, &tmppo, sizeof(wl_po_t));
		if (ISHTPHY(pi)) {
			if ((tmppo.phy_type != PHY_TYPE_HT) && (tmppo.phy_type != PHY_TYPE_N))  {
				tmppo.phy_type = PHY_TYPE_NULL;
				break;
			}

			err = wlc_phy_get_po_htphy(pi, &tmppo);
			if (!err)
				bcopy(&tmppo, a, sizeof(wl_po_t));
			break;
		} else if (ISNPHY(pi)) {
			if (tmppo.phy_type != PHY_TYPE_N)  {
				tmppo.phy_type = PHY_TYPE_NULL;
				break;
			}

			/* Power offsets variables depend on the SROM revision */
			if (NREV_GE(pi->pubpi.phy_rev, 8) && (pi->sh->sromrev >= 9)) {
				err = wlc_phy_get_po_htphy(pi, &tmppo);

			} else {
				switch (tmppo.band) {
				case WL_CHAN_FREQ_RANGE_2G:
					tmppo.cckpo = pi->ppr.sr8.cck2gpo;
					tmppo.ofdmpo = pi->ppr.sr8.ofdm[tmppo.band];
					bcopy(&pi->ppr.sr8.mcs[tmppo.band][0], &tmppo.mcspo,
						8*sizeof(uint16));
					break;
#ifdef BAND5G
				case WL_CHAN_FREQ_RANGE_5G_BAND0:
					tmppo.ofdmpo = pi->ppr.sr8.ofdm[tmppo.band];
					bcopy(&pi->ppr.sr8.mcs[tmppo.band], &tmppo.mcspo,
						8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND1:
					tmppo.ofdmpo = pi->ppr.sr8.ofdm[tmppo.band];
					bcopy(&pi->ppr.sr8.mcs[tmppo.band], &tmppo.mcspo,
						8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND2:
					tmppo.ofdmpo = pi->ppr.sr8.ofdm[tmppo.band];
					bcopy(&pi->ppr.sr8.mcs[tmppo.band], &tmppo.mcspo,
						8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND3:
					tmppo.ofdmpo = pi->ppr.sr8.ofdm[tmppo.band];
					bcopy(&pi->ppr.sr8.mcs[tmppo.band], &tmppo.mcspo,
						8*sizeof(uint16));
					break;
#endif /* BAND5G */
				default:
					PHY_ERROR(("bandrange %d is out of scope\n", tmppo.band));
					err = BCME_BADARG;
					break;
				}
			}

			if (!err)
				bcopy(&tmppo, a, sizeof(wl_po_t));
		} else if (ISLCNCOMMONPHY(pi)) {
			if ((tmppo.phy_type != PHY_TYPE_LCN) &&
				(tmppo.phy_type != PHY_TYPE_LCN40)) {
				tmppo.phy_type = PHY_TYPE_NULL;
				break;
			}

			switch (tmppo.band) {
			case WL_CHAN_FREQ_RANGE_2G:
				tmppo.cckpo = pi->ppr.sr8.cck2gpo;
				tmppo.ofdmpo = pi->ppr.sr8.ofdm[tmppo.band];
				bcopy(&pi->ppr.sr8.mcs[tmppo.band], &tmppo.mcspo, 8*sizeof(uint16));

				break;

			default:
				PHY_ERROR(("bandrange %d is out of scope\n", tmppo.band));
				err = BCME_BADARG;
				break;
			}

			if (!err)
				bcopy(&tmppo, a, sizeof(wl_po_t));
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_SVAL(IOV_POVARS): {
		wl_po_t inpo;

		bcopy(p, &inpo, sizeof(wl_po_t));

		if (ISHTPHY(pi)) {
			if ((inpo.phy_type == PHY_TYPE_HT) || (inpo.phy_type == PHY_TYPE_N))
				err = wlc_phy_set_po_htphy(pi, &inpo);
			break;
		} else if (ISNPHY(pi)) {
			if (inpo.phy_type != PHY_TYPE_N)
				break;

			/* Power offsets variables depend on the SROM revision */
			if (NREV_GE(pi->pubpi.phy_rev, 8) && (pi->sh->sromrev >= 9)) {
				err = wlc_phy_set_po_htphy(pi, &inpo);

			} else {

				switch (inpo.band) {
				case WL_CHAN_FREQ_RANGE_2G:
					pi->ppr.sr8.cck2gpo = inpo.cckpo;
					pi->ppr.sr8.ofdm[inpo.band]  = inpo.ofdmpo;
					bcopy(inpo.mcspo, &(pi->ppr.sr8.mcs[inpo.band][0]),
						8*sizeof(uint16));
					break;
#ifdef BAND5G
				case WL_CHAN_FREQ_RANGE_5G_BAND0:
					pi->ppr.sr8.ofdm[inpo.band] = inpo.ofdmpo;
					bcopy(inpo.mcspo, &(pi->ppr.sr8.mcs[inpo.band][0]),
						8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND1:
					pi->ppr.sr8.ofdm[inpo.band] = inpo.ofdmpo;
					bcopy(inpo.mcspo, &(pi->ppr.sr8.mcs[inpo.band][0]),
						8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND2:
					pi->ppr.sr8.ofdm[inpo.band] = inpo.ofdmpo;
					bcopy(inpo.mcspo, &(pi->ppr.sr8.mcs[inpo.band][0]),
						8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND3:
					pi->ppr.sr8.ofdm[inpo.band] = inpo.ofdmpo;
					bcopy(inpo.mcspo, &(pi->ppr.sr8.mcs[inpo.band][0]),
						8*sizeof(uint16));
					break;
#endif /* BAND5G */
				default:
					PHY_ERROR(("bandrange %d is out of scope\n", inpo.band));
					err = BCME_BADARG;
					break;
				}

			}

		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;
#endif // endif

	case IOV_GVAL(IOV_SROM_REV): {
			*ret_int_ptr = pi->sh->sromrev;
	}
	break;

#ifdef WLTEST
	case IOV_GVAL(IOV_PHY_MAXP): {
		if (ISNPHY(pi)) {
			srom_pwrdet_t	*pwrdet  = pi->pwrdet;
			uint8*	maxp = (uint8*)a;

			maxp[0] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_2G];
			maxp[1] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_2G];
			maxp[2] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND0];
			maxp[3] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND0];
			maxp[4] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND1];
			maxp[5] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND1];
			maxp[6] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND2];
			maxp[7] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND2];
			if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			{
				maxp[8] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND3];
				maxp[9] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND3];
			}
		}
		break;
	}
	case IOV_SVAL(IOV_PHY_MAXP): {
		if (ISNPHY(pi)) {
			uint8*	maxp = (uint8*)p;
			srom_pwrdet_t	*pwrdet  = pi->pwrdet;

			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_2G] = maxp[0];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_2G] = maxp[1];
			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND0] = maxp[2];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND0] = maxp[3];
			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND1] = maxp[4];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND1] = maxp[5];
			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND2] = maxp[6];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND2] = maxp[7];
			if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			{
				pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND3] = maxp[8];
				pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND3] = maxp[9];
			}
		}
		break;
	}
#endif /* WLTEST */
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovars_rssi(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	BCM_REFERENCE(*ret_int_ptr);
	BCM_REFERENCE(*pi_ac);

	switch (actionid) {
#if defined(WLTEST)
	case IOV_GVAL(IOV_UNMOD_RSSI):
	{
		int32 input_power_db = 0;
		rxsigpwrfn_t rx_sig_pwr_fn = pi->pi_fptr.rxsigpwr;

		PHY_INFORM(("UNMOD RSSI Called\n"));

		if (!rx_sig_pwr_fn)
			return BCME_UNSUPPORTED;	/* lpphy and sslnphy support only for now */

		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}

		input_power_db = (*rx_sig_pwr_fn)(pi, -1);

#if defined(WLNOKIA_NVMEM)
		input_power_db = wlc_phy_upd_rssi_offset(pi,
			(int8)input_power_db, pi->radio_chanspec);
#endif // endif

		*ret_int_ptr = input_power_db;
		break;
	}
#endif // endif
#if (defined(NCONF) || defined(LCN40CONF)) && defined(WLTEST) || ACCONF || ACCONF2
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_CAL_TEMP):
		if (ISLCN40PHY(pi)) {
			phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
			*ret_int_ptr = (int32)pi_lcn40->gain_cal_temp;
		} else if (CHIPID_4324X_EPA_FAMILY(pi)) {
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			*ret_int_ptr = (int32)pi_nphy->gain_cal_temp;
		}
		break;

	case IOV_SVAL(IOV_PHY_RSSI_GAIN_CAL_TEMP):
		if (ISLCN40PHY(pi)) {
			phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
			pi_lcn40->gain_cal_temp = (int8)int_val;
		} else if (CHIPID_4324X_EPA_FAMILY(pi)) {
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			pi_nphy->gain_cal_temp = (int8)int_val;
		}
		break;
	case IOV_SVAL(IOV_PHY_RSSI_CAL_FREQ_GRP_2G):
	{
		int8 i;
		uint8 *nvramValues = p;

		for (i = 0; i < 14; i++) {
			pi_ac->sromi->rssi_cal_freq_grp[i] =
				nvramValues[i];
		}
		break;
	}
	case IOV_GVAL(IOV_PHY_RSSI_CAL_FREQ_GRP_2G):
	{
		int8 i;
		uint8 *nvramValues = a;

		for (i = 0; i < 14; i++) {
		 nvramValues[i] =
		   pi_ac->sromi->rssi_cal_freq_grp[i];
		}

		break;
	}
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB0):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB1):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB2):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB3):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB4):
		if (ISACPHY(pi)) {
			acphy_rssioffset_t *pi_ac_rssioffset =
			  &pi_ac->sromi->rssioffset;
			int8 *deltaValues = p;
			uint8 core = deltaValues[0];
			uint8 gain_idx, bw_idx, subband_idx;

			if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB0)) {
				subband_idx = 0;
			} else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB1)) {
				subband_idx = 1;
			} else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB2)) {
				subband_idx = 2;
			} else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB3)) {
				subband_idx = 3;
			} else {
				subband_idx = 4;
			}

			for (bw_idx = 0; bw_idx < ACPHY_NUM_BW_2G; bw_idx++) {
				for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_2G_PARAMS_EXT;
					 gain_idx++) {
					pi_ac_rssioffset->rssi_corr_gain_delta_2g_sub[core]
							[gain_idx][bw_idx][subband_idx]
							= deltaValues[gain_idx + 4*bw_idx + 1];
				}
			}
			wlc_phy_set_trloss_reg_acphy(pi, core);

		}
		break;

	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB0):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB1):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB2):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB3):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB4):
		if (ISACPHY(pi)) {
		acphy_rssioffset_t *pi_ac_rssioffset =
		&pi_ac->sromi->rssioffset;
		int8 *deltaValues = a;
		uint8 core, ant;
		uint8 gain_idx, bw_idx, core_idx = 0, subband_idx;

		if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB0)) {
			subband_idx = 0;
		} else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB1)) {
			subband_idx = 1;
		} else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB2)) {
			subband_idx = 2;
		} else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB3)) {
			subband_idx = 3;
		} else {
			subband_idx = 4;
		}

		FOREACH_CORE(pi, core) {
			ant = phy_get_rsdbbrd_corenum(pi, core);
			deltaValues[9*core_idx] = ant;
			for (bw_idx = 0; bw_idx < ACPHY_NUM_BW_2G; bw_idx++) {
				for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_2G_PARAMS_EXT;
					gain_idx++) {
					deltaValues[gain_idx + 4*bw_idx + 9*core_idx +1 ]=
					  pi_ac_rssioffset->rssi_corr_gain_delta_2g_sub[ant]
					  [gain_idx][bw_idx][subband_idx];

				}
			}
			core_idx++;
		}

		deltaValues[9*core_idx] = -1;
		}
		break;

	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2G):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GHH):
		if (ISLCN40PHY(pi)) {
			phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
			int8 *deltaValues = p;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2G))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2g;
			else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2gh;
			else
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2ghh;

			for (i = 0; i < LCN40PHY_GAIN_DELTA_2G_PARAMS; i++)
				rssi_gain_delta[i] = deltaValues[i];
		} else if (CHIPID_4324X_EPA_FAMILY(pi)) {
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			int8 *deltaValues = p;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2G))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_2g;
			else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_2gh;
			else
				rssi_gain_delta = pi_nphy->rssi_gain_delta_2ghh;

			for (i = 0; i < NPHY_GAIN_DELTA_2G_PARAMS; i++)
				rssi_gain_delta[i] = deltaValues[i];
		} else if (ISACPHY(pi)) {
			acphy_rssioffset_t *pi_ac_rssioffset =
			        &pi_ac->sromi->rssioffset;
			int8 *deltaValues = p;
			uint8 core = deltaValues[0];
			uint8 gain_idx, bw_idx;

			if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2G)) {
				for (bw_idx = 0; bw_idx < ACPHY_NUM_BW_2G; bw_idx++) {
					for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_2G_PARAMS;
					     gain_idx++) {
						pi_ac_rssioffset->rssi_corr_gain_delta_2g[core]
						        [gain_idx][bw_idx]
						        = deltaValues[gain_idx + 2*bw_idx + 1];
					}
				}
				wlc_phy_set_trloss_reg_acphy(pi, core);

			} else {
				PHY_ERROR(("Unsupported RSSI_GAIN_DELTA_2G type!\n"));
				err = BCME_UNSUPPORTED;
			}
		}
		break;

	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2G):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GHH):
		if (ISLCN40PHY(pi)) {
			phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
			int8 *deltaValues = a;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2G))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2g;
			else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2gh;
			else
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_2ghh;

			for (i = 0; i < LCN40PHY_GAIN_DELTA_2G_PARAMS; i++)
				deltaValues[i] = rssi_gain_delta[i];
		} else if (CHIPID_4324X_EPA_FAMILY(pi)) {
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			int8 *deltaValues = a;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2G))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_2g;
			else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_2gh;
			else
				rssi_gain_delta = pi_nphy->rssi_gain_delta_2ghh;

			for (i = 0; i < NPHY_GAIN_DELTA_2G_PARAMS; i++)
				deltaValues[i] = rssi_gain_delta[i];
		} else if (ISACPHY(pi)) {
			acphy_rssioffset_t *pi_ac_rssioffset =
				&pi_ac->sromi->rssioffset;
			int8 *deltaValues = a;
			uint8 core, ant;
			uint8 gain_idx, bw_idx, core_idx = 0;

			if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2G)) {
			   FOREACH_CORE(pi, core) {
			      ant = phy_get_rsdbbrd_corenum(pi, core);
			      deltaValues[5*core_idx] = ant;
			      for (bw_idx = 0; bw_idx < ACPHY_NUM_BW_2G; bw_idx++) {
			         for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_2G_PARAMS;
				      gain_idx++) {
			            deltaValues[gain_idx + 2*bw_idx + 5*core_idx +1 ]=
			              pi_ac_rssioffset->rssi_corr_gain_delta_2g[ant]
				            [gain_idx][bw_idx];
			         }
			      }
			      core_idx++;
			   }
			   /* set core to -1 after the last valid entry */
			   deltaValues[core_idx*5] = -1;

			   for (bw_idx = 0; bw_idx < core_idx*5; bw_idx++) {
				printf("%d ", deltaValues[bw_idx]);
			   }
			   printf("\n");
			} else {
				PHY_ERROR(("Unsupported RSSI_GAIN_DELTA_2G type!\n"));
				err = BCME_UNSUPPORTED;
			}
		}
		break;

	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU):
	case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GH):
		if (ISLCN40PHY(pi)) {
			phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
			int8 *deltaValues = p;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gl;
			else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gml;
			else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gmu;
			else
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gh;

			for (i = 0; i < LCN40PHY_GAIN_DELTA_5G_PARAMS; i++)
				rssi_gain_delta[i] = deltaValues[i];
		} else if (CHIPID_4324X_EPA_FAMILY(pi)) {
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			int8 *deltaValues = p;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gl;
			else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gml;
			else if (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gmu;
			else
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gh;

			for (i = 0; i < NPHY_GAIN_DELTA_5G_PARAMS; i++)
				rssi_gain_delta[i] = deltaValues[i];
		} else if (ISACPHY(pi) && (pi->u.pi_acphy->rssi_cal_rev == FALSE)) {
			acphy_rssioffset_t *pi_ac_rssioffset =
				&pi_ac->sromi->rssioffset;
			int8 *deltaValues = p;
			uint8 core = deltaValues[0];
			uint8 gain_idx, bw_idx, subband_idx;

			subband_idx = (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL)) ? 0:
			        (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML)) ? 1:
			        (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU)) ? 2:3;
			for (bw_idx = 0; bw_idx < ACPHY_NUM_BW; bw_idx++) {
				for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_5G_PARAMS;
				     gain_idx++) {
					pi_ac_rssioffset->rssi_corr_gain_delta_5g
						[core][gain_idx][bw_idx][subband_idx]
						= deltaValues[gain_idx + 2*bw_idx + 1];
				}
			}
			wlc_phy_set_trloss_reg_acphy(pi, core);

			for (bw_idx = 0; bw_idx < ACPHY_NUM_BW; bw_idx++) {
				for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_5G_PARAMS;
				     gain_idx++) {
					printf("%d ", pi_ac_rssioffset->rssi_corr_gain_delta_5g
						[core][gain_idx][bw_idx][subband_idx]);
				}
			}
			printf("\n");
		} else if (ISACPHY(pi) && (pi->u.pi_acphy->rssi_cal_rev == TRUE)) {
			acphy_rssioffset_t *pi_ac_rssioffset =
				&pi_ac->sromi->rssioffset;
			int8 *deltaValues = p;
			uint8 core = deltaValues[0];
			uint8 gain_idx, bw_idx, subband_idx;

			subband_idx = (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL)) ? 0:
			        (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML)) ? 1:
			        (actionid == IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU)) ? 2:3;
			for (bw_idx = 0; bw_idx < ACPHY_NUM_BW; bw_idx++) {
				for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_5G_PARAMS_EXT;
				     gain_idx++) {
					pi_ac_rssioffset->rssi_corr_gain_delta_5g_sub
						[core][gain_idx][bw_idx][subband_idx]
						= deltaValues[gain_idx + 4*bw_idx + 1];
				}
			}
			wlc_phy_set_trloss_reg_acphy(pi, core);

			for (bw_idx = 0; bw_idx < ACPHY_NUM_BW; bw_idx++) {
				for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_5G_PARAMS_EXT;
				     gain_idx++) {
					printf("%d ", pi_ac_rssioffset->rssi_corr_gain_delta_5g_sub
						[core][gain_idx][bw_idx][subband_idx]);
				}
			}
			printf("\n");
		}

		break;

	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU):
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GH):
		if (ISLCN40PHY(pi)) {
			phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
			int8 *deltaValues = a;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gl;
			else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gml;
			else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU))
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gmu;
			else
				rssi_gain_delta = pi_lcn40->rssi_gain_delta_5gh;

			for (i = 0; i < LCN40PHY_GAIN_DELTA_5G_PARAMS; i++)
				deltaValues[i] = rssi_gain_delta[i];
		} else if (CHIPID_4324X_EPA_FAMILY(pi)) {
			phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
			int8 *deltaValues = a;
			uint8 i;
			int8 *rssi_gain_delta;
			if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gl;
			else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gml;
			else if (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU))
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gmu;
			else
				rssi_gain_delta = pi_nphy->rssi_gain_delta_5gh;

			for (i = 0; i < NPHY_GAIN_DELTA_5G_PARAMS; i++)
				deltaValues[i] = rssi_gain_delta[i];
		} else if (ISACPHY(pi) && (pi->u.pi_acphy->rssi_cal_rev == FALSE)) {
			acphy_rssioffset_t *pi_ac_rssioffset =
				&pi_ac->sromi->rssioffset;
			int8 *deltaValues = a;
			uint8 core, ant, core_idx = 0;
			uint8 gain_idx, bw_idx, subband_idx;

			subband_idx = (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL)) ? 0:
			        (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML)) ? 1:
			        (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU)) ? 2:3;

			FOREACH_CORE(pi, core) {
				ant = phy_get_rsdbbrd_corenum(pi, core);
				deltaValues[7*core_idx] =  ant;
				for (bw_idx = 0; bw_idx < ACPHY_NUM_BW; bw_idx++) {
					for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_5G_PARAMS;
					     gain_idx++) {
						deltaValues[gain_idx + 2*bw_idx + 7*core_idx +1]=
							pi_ac_rssioffset->rssi_corr_gain_delta_5g
							[ant][gain_idx][bw_idx][subband_idx];
					}
				}
				core_idx++;
			}
			deltaValues[core_idx*7] = -1;
		} else if (ISACPHY(pi) && (pi->u.pi_acphy->rssi_cal_rev == TRUE)) {
			acphy_rssioffset_t *pi_ac_rssioffset =
				&pi_ac->sromi->rssioffset;
			int8 *deltaValues = a;
			uint8 core, core_idx = 0;
			uint8 gain_idx, bw_idx, subband_idx;

			subband_idx = (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL)) ? 0:
			        (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML)) ? 1:
			        (actionid == IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU)) ? 2:3;

			FOREACH_CORE(pi, core) {
				deltaValues[13*core_idx] = core;
				for (bw_idx = 0; bw_idx < ACPHY_NUM_BW; bw_idx++) {
				  for (gain_idx = 0; gain_idx < ACPHY_GAIN_DELTA_5G_PARAMS_EXT;
				       gain_idx++) {
				    deltaValues[gain_idx + 4*bw_idx + 13*core_idx +1 ]=
				      pi_ac_rssioffset->rssi_corr_gain_delta_5g_sub
				      [core][gain_idx][bw_idx][subband_idx];
				  }
				}
				core_idx++;
			}
			/* set core to -1 after the last valid entry */
			deltaValues[core_idx*13] = -1;

		}
		break;
	case IOV_SVAL(IOV_PHY_RXGAINERR_2G):
		if (ISACPHY(pi)) {
			uint8 core;
			int8 *deltaValues = p;
			FOREACH_CORE(pi, core) {
				pi->rxgainerr_2g[core] = deltaValues[core];
			}
			pi->rxgainerr2g_isempty = FALSE;
		}
		break;

	case IOV_GVAL(IOV_PHY_RXGAINERR_2G):
		if (ISACPHY(pi)) {
			int8 *deltaValues = a;
			uint8 core;
			FOREACH_CORE(pi, core) {
			  deltaValues[core] = pi->rxgainerr_2g[core];
			}
		}
		break;
	case IOV_SVAL(IOV_PHY_RXGAINERR_5GL):
		if (ISACPHY(pi)) {
			uint8 core;
			int8 *deltaValues = p;
			FOREACH_CORE(pi, core) {
				pi->rxgainerr_5gl[core] = deltaValues[core];
			}
			pi->rxgainerr5gl_isempty = FALSE;
		}
		break;

	case IOV_GVAL(IOV_PHY_RXGAINERR_5GL):
		if (ISACPHY(pi)) {
			int8 *deltaValues = a;
			uint8 core;
			FOREACH_CORE(pi, core) {
			  deltaValues[core] = pi->rxgainerr_5gl[core];
			}
		}
		break;

	case IOV_SVAL(IOV_PHY_RXGAINERR_5GM):
		if (ISACPHY(pi)) {
			uint8 core;
			int8 *deltaValues = p;
			FOREACH_CORE(pi, core) {
				pi->rxgainerr_5gm[core] = deltaValues[core];
			}
			pi->rxgainerr5gm_isempty = FALSE;
		}
		break;

	case IOV_GVAL(IOV_PHY_RXGAINERR_5GM):
		if (ISACPHY(pi)) {
			int8 *deltaValues = a;
			uint8 core;
			FOREACH_CORE(pi, core) {
			  deltaValues[core] = pi->rxgainerr_5gm[core];
			}
		}
		break;

	case IOV_SVAL(IOV_PHY_RXGAINERR_5GH):
		if (ISACPHY(pi)) {
			uint8 core;
			int8 *deltaValues = p;
			FOREACH_CORE(pi, core) {
				pi->rxgainerr_5gh[core] = deltaValues[core];
			}
			pi->rxgainerr5gh_isempty = FALSE;
		}
		break;

	case IOV_GVAL(IOV_PHY_RXGAINERR_5GH):
		if (ISACPHY(pi)) {
			int8 *deltaValues = a;
			uint8 core;
			FOREACH_CORE(pi, core) {
			  deltaValues[core] = pi->rxgainerr_5gh[core];
			}
		}
		break;

	case IOV_SVAL(IOV_PHY_RXGAINERR_5GU):
		if (ISACPHY(pi)) {
			uint8 core;
			int8 *deltaValues = p;
			FOREACH_CORE(pi, core) {
				pi->rxgainerr_5gu[core] = deltaValues[core];
			}
			pi->rxgainerr5gu_isempty = FALSE;
		}
		break;

	case IOV_GVAL(IOV_PHY_RXGAINERR_5GU):
		if (ISACPHY(pi)) {
			int8 *deltaValues = a;
			uint8 core;

			FOREACH_CORE(pi, core) {
			  deltaValues[core] = pi->rxgainerr_5gu[core];
			}
		}
		break;

#endif /* NCONF || LCN40CONF || ACCONF || ACCONF2 */
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovars_aci(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	int err = BCME_OK;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (actionid) {
#if defined(WLTEST)
	case IOV_SVAL(IOV_ACI_EXIT_CHECK_PERIOD):
		if (int_val == 0)
			err = BCME_RANGE;
		else
			pi->aci_exit_check_period = int_val;
		break;

	case IOV_GVAL(IOV_ACI_EXIT_CHECK_PERIOD):
		int_val = pi->aci_exit_check_period;
		bcopy(&int_val, a, vsize);
		break;

#endif // endif
#if defined(WLTEST)
	case IOV_SVAL(IOV_PHY_GLITCHK):
		pi->tunings[0] = (uint16)int_val;
		break;

	case IOV_SVAL(IOV_PHY_NOISE_UP):
		pi->tunings[1] = (uint16)int_val;
		break;

	case IOV_SVAL(IOV_PHY_NOISE_DWN):
		pi->tunings[2] = (uint16)int_val;
		break;
#endif /* #if defined(WLTEST) */
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static int
wlc_phy_iovars_generic(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	BCM_REFERENCE(*ret_int_ptr);
	BCM_REFERENCE(bool_val);

	switch (actionid) {

#if defined(BCMDBG) || defined(WLTEST)
	case IOV_GVAL(IOV_FAST_TIMER):
		*ret_int_ptr = (int32)pi->sh->fast_timer;
		break;

	case IOV_SVAL(IOV_FAST_TIMER):
		pi->sh->fast_timer = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_SLOW_TIMER):
		*ret_int_ptr = (int32)pi->sh->slow_timer;
		break;

	case IOV_SVAL(IOV_SLOW_TIMER):
		pi->sh->slow_timer = (uint32)int_val;
		break;

#endif /* BCMDBG || WLTEST */
#if defined(BCMDBG) || defined(WLTEST) || defined(WLMEDIA_CALDBG) || \
	defined(PHYCAL_CHNG_CS) || defined(WLMEDIA_FLAMES)
	case IOV_GVAL(IOV_GLACIAL_TIMER):
		*ret_int_ptr = (int32)pi->sh->glacial_timer;
		break;

	case IOV_SVAL(IOV_GLACIAL_TIMER):
		pi->sh->glacial_timer = (uint32)int_val;
		break;
#endif // endif
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV) || \
	defined(MACOSX) || defined(DBG_PHY_IOV)

	case IOV_GVAL(IOV_PHY_WATCHDOG):
		*ret_int_ptr = (int32)pi->phywatchdog_override;
		break;

	case IOV_SVAL(IOV_PHY_WATCHDOG):
		wlc_phy_watchdog_override(pi, bool_val);
		break;
#endif // endif
	case IOV_GVAL(IOV_CAL_PERIOD):
	        *ret_int_ptr = (int32)pi->cal_period;
	        break;

	case IOV_SVAL(IOV_CAL_PERIOD):
	        pi->cal_period = (uint32)int_val;
	        break;
#if defined(WLTEST)
#ifdef BAND5G
	case IOV_GVAL(IOV_PHY_CGA_5G):
		/* Pass on existing channel based offset into wl */
		bcopy(pi->phy_cga_5g, a, 24*sizeof(int8));
		break;
#endif /* BAND5G */
	case IOV_GVAL(IOV_PHY_CGA_2G):
		/* Pass on existing channel based offset into wl */
		bcopy(pi->phy_cga_2g, a, 14*sizeof(int8));
		break;

	case IOV_GVAL(IOV_PHYHAL_MSG):
		*ret_int_ptr = get_phyhal_msg_level();
		break;

	case IOV_SVAL(IOV_PHYHAL_MSG):
		set_phyhal_msg_level(int_val);
		break;

	case IOV_SVAL(IOV_PHY_FIXED_NOISE):
		pi->phy_fixed_noise = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_FIXED_NOISE):
		int_val = (int32)pi->phy_fixed_noise;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_PHYNOISE_POLL):
		*ret_int_ptr = (int32)pi->phynoise_polling;
		break;

	case IOV_SVAL(IOV_PHYNOISE_POLL):
		pi->phynoise_polling = bool_val;
		break;

	case IOV_GVAL(IOV_CARRIER_SUPPRESS):
		if (!ISLCNPHY(pi))
			err = BCME_UNSUPPORTED; /* lcnphy for now */
		*ret_int_ptr = (pi->carrier_suppr_disable == 0);
		break;

	case IOV_SVAL(IOV_CARRIER_SUPPRESS):
	{
		initfn_t carr_suppr_fn = pi->pi_fptr.carrsuppr;
		if (carr_suppr_fn) {
			pi->carrier_suppr_disable = bool_val;
			if (pi->carrier_suppr_disable) {
				(*carr_suppr_fn)(pi);
			}
			PHY_INFORM(("Carrier Suppress Called\n"));
		} else
			err = BCME_UNSUPPORTED;
		break;
	}

	case IOV_GVAL(IOV_PKTENG_STATS):
	  wlc_phy_pkteng_stats_get(pi, a, alen);
		break;
#ifdef BAND5G
	case IOV_GVAL(IOV_PHY_SUBBAND5GVER):
		/* Retrieve 5G subband version */
		int_val = (uint8)(pi->sromi->subband5Gver);
		bcopy(&int_val, a, vsize);
		break;
#endif /* BAND5G */
	case IOV_GVAL(IOV_PHY_TXRX_CHAIN):
	        wlc_phy_iovar_txrx_chain(pi, int_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_TXRX_CHAIN):
		err = wlc_phy_iovar_txrx_chain(pi, int_val, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_PHY_BPHY_EVM):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_EVM, (bool) int_val);
		break;

	case IOV_GVAL(IOV_PHY_BPHY_RFCS):
		*ret_int_ptr = pi->phy_bphy_rfcs;
		break;

	case IOV_SVAL(IOV_PHY_BPHY_RFCS):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_RFCS, (bool) int_val);
		break;

	case IOV_GVAL(IOV_PHY_SCRAMINIT):
		*ret_int_ptr = pi->phy_scraminit;
		break;

	case IOV_SVAL(IOV_PHY_SCRAMINIT):
		wlc_phy_iovar_scraminit(pi, (uint8)int_val);
		break;

	case IOV_SVAL(IOV_PHY_RFSEQ):
		wlc_phy_iovar_force_rfseq(pi, (uint8)int_val);
		break;

	case IOV_GVAL(IOV_PHY_TX_TONE):
	case IOV_GVAL(IOV_PHY_TX_TONE_HZ):
		*ret_int_ptr = pi->phy_tx_tone_freq;
		break;

	case IOV_SVAL(IOV_PHY_TX_TONE_STOP):
		wlc_phy_iovar_tx_tone_stop(pi);
		break;

	case IOV_SVAL(IOV_PHY_TX_TONE):
		wlc_phy_iovar_tx_tone(pi, (int32)int_val);
		break;

	case IOV_SVAL(IOV_PHY_TX_TONE_HZ):
		wlc_phy_iovar_tx_tone_hz(pi, (int32)int_val);
		break;

	case IOV_GVAL(IOV_PHY_TEST_TSSI):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 0);
		break;

	case IOV_GVAL(IOV_PHY_TEST_TSSI_OFFS):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 12);
		break;

	case IOV_GVAL(IOV_PHY_TEST_IDLETSSI):
		*((uint*)a) = wlc_phy_iovar_test_idletssi(pi, (uint8)int_val);
		break;

	case IOV_SVAL(IOV_PHY_SETRPTBL):
		wlc_phy_iovar_setrptbl(pi);
		break;

	case IOV_SVAL(IOV_PHY_FORCEIMPBF):
		wlc_phy_iovar_forceimpbf(pi);
		break;

	case IOV_SVAL(IOV_PHY_FORCESTEER):
		wlc_phy_iovar_forcesteer(pi, (uint8)int_val);
		break;
#ifdef BAND5G
	case IOV_SVAL(IOV_PHY_5G_PWRGAIN):
		pi->phy_5g_pwrgain = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_5G_PWRGAIN):
		*ret_int_ptr = (int32)pi->phy_5g_pwrgain;
		break;
#endif /* BAND5G */

	case IOV_SVAL(IOV_PHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_GVAL(IOV_PHY_ACTIVECAL):
		*ret_int_ptr = (int32)((pi->cal_info->cal_phase_id !=
			MPHASE_CAL_STATE_IDLE)? 1 : 0);
		break;

	case IOV_SVAL(IOV_PHY_BBMULT):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_iovar_bbmult_set(pi, p);
		break;

	case IOV_GVAL(IOV_PHY_BBMULT):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		wlc_phy_iovar_bbmult_get(pi, int_val, bool_val, ret_int_ptr);
		break;

#if defined(WLC_LOWPOWER_BEACON_MODE)
	case IOV_GVAL(IOV_PHY_LOWPOWER_BEACON_MODE):
		if (ISLCN40PHY(pi)) {
			*ret_int_ptr = (pi->u.pi_lcn40phy)->lowpower_beacon_mode;
		}
		break;

	case IOV_SVAL(IOV_PHY_LOWPOWER_BEACON_MODE):
		wlc_phy_lowpower_beacon_mode(pih, int_val);
		break;
#endif /* WLC_LOWPOWER_BEACON_MODE */
#endif // endif
#if defined(WLTEST) || defined(WLMEDIA_CALDBG)
	case IOV_GVAL(IOV_PKTENG_GAININDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_pkteng_get_gainindex(pi, ret_int_ptr);
		break;

#endif // endif
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV) || \
	defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG) || defined(ATE_BUILD)
	case IOV_GVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;

	case IOV_SVAL(IOV_PAPD_EN_WAR):
		wlapi_bmac_write_shm(pi->sh->physhim, M_PAPDOFF_MCS, (uint16)int_val);
		break;

	case IOV_GVAL(IOV_PAPD_EN_WAR):
		*ret_int_ptr = wlapi_bmac_read_shm(pi->sh->physhim, M_PAPDOFF_MCS);
		break;

#ifndef ATE_BUILD
	case IOV_SVAL(IOV_PHY_SKIPPAPD):
		if ((int_val != 0) && (int_val != 1)) {
			err = BCME_RANGE;
			break;
		}
		if (ISACPHY(pi))
			pi->u.pi_acphy->acphy_papd_skip = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_PHY_SKIPPAPD):
		if (ISACPHY(pi))
		        *ret_int_ptr = pi->u.pi_acphy->acphy_papd_skip;
		break;

	case IOV_GVAL(IOV_PHY_FORCECAL_OBT):
		err = wlc_phy_iovar_forcecal_obt(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_FORCECAL_OBT):
		err = wlc_phy_iovar_forcecal_obt(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_GVAL(IOV_PHY_FORCECAL_NOISE): /* Get crsminpwr for core 0 & core 1 */
		err = wlc_phy_iovar_forcecal_noise(pi, int_val, a, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_FORCECAL_NOISE): /* do only Noise Cal */
		err = wlc_phy_iovar_forcecal_noise(pi, int_val, a, vsize, TRUE);
		break;
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WLTEST) || defined(MACOSX)
	case IOV_SVAL(IOV_PHY_DEAF):
		wlc_phy_iovar_set_deaf(pi, int_val);
		break;
	case IOV_GVAL(IOV_PHY_DEAF):
		err = wlc_phy_iovar_get_deaf(pi, ret_int_ptr);
		break;
#endif // endif
#ifdef WLTEST
	case IOV_GVAL(IOV_PHY_FEM2G): {
		bcopy(&pi->fem2g, a, sizeof(srom_fem_t));
		break;
	}

	case IOV_SVAL(IOV_PHY_FEM2G): {
		bcopy(p, &pi->fem2g, sizeof(srom_fem_t));
		/* srom_fem2g->extpagain changed after attach time */
		wlc_phy_txpower_ipa_upd(pi);
		break;
	}

#ifdef BAND5G
	case IOV_GVAL(IOV_PHY_FEM5G): {
		bcopy(&pi->fem5g, a, sizeof(srom_fem_t));
		break;
	}

	case IOV_SVAL(IOV_PHY_FEM5G): {
		bcopy(p, &pi->fem5g, sizeof(srom_fem_t));
		/* srom_fem5g->extpagain changed after attach time */
		wlc_phy_txpower_ipa_upd(pi);
		break;
	}
#endif /* BAND5G */
#endif /* WLTEST */
	case IOV_GVAL(IOV_PHY_RXIQ_EST):
	{
		bool suspend;
		bool low_pwr = FALSE;
		uint16 r;
		int temp_dBm;
#if CORE4 >= 4
		int16 tmp;
#endif // endif
		int16 iqest[PHY_MAX_CORES] = {0};

		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}

		/* make sure bt-prisel is on WLAN side */
		wlc_phy_btcx_wlan_critical_enter(pi);

		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}

		phy_utils_phyreg_enter(pi);

		/* For 4350 Olympic program, -i 0 should behave exactly same as -i 1
		 * So, if there is force gain type is 0, then make it 1 for 4350
		 */
		if ((pi->u.pi_acphy->rud_agc_enable == TRUE) &&
			(pi->phy_rxiq_force_gain_type == 0) &&
			(pi->phy_rxiq_extra_gain_3dB == 0)) {
			pi->phy_rxiq_force_gain_type = 1;
		}
		/* get IQ power measurements */
		*ret_int_ptr = wlc_phy_rx_iq_est(pi, pi->phy_rxiq_samps, pi->phy_rxiq_antsel,
		                                 pi->phy_rxiq_resln, pi->phy_rxiq_lpfhpc,
		                                 pi->phy_rxiq_diglpf,
		                                 pi->phy_rxiq_gain_correct,
		                                 pi->phy_rxiq_extra_gain_3dB, 0,
				pi->phy_rxiq_force_gain_type, iqest);

		if ((pi->u.pi_acphy->rud_agc_enable == TRUE) &&
			(pi->phy_rxiq_force_gain_type == 1) && (pi->phy_rxiq_resln == 1)) {
			FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, r) {
				temp_dBm = *ret_int_ptr;
				temp_dBm = (temp_dBm >> (10*r)) & 0x3ff;
				temp_dBm = ((int16)(temp_dBm << 6)) >> 6; /* sign extension */
				if ((temp_dBm >> 2) < -82) {
					low_pwr = TRUE;
				}
				PHY_RXIQ(("In %s: | For core %d | iqest_dBm = %d"
					  " \n", __FUNCTION__, r, (temp_dBm >> 2)));
			}
			if (low_pwr) {
				pi->phy_rxiq_force_gain_type = 9;
				*ret_int_ptr = wlc_phy_rx_iq_est(pi, pi->phy_rxiq_samps,
					pi->phy_rxiq_antsel,
					pi->phy_rxiq_resln, pi->phy_rxiq_lpfhpc,
					pi->phy_rxiq_diglpf, pi->phy_rxiq_gain_correct,
					pi->phy_rxiq_extra_gain_3dB, 0,
					pi->phy_rxiq_force_gain_type, iqest);
			}
		}
#if CORE4 >= 4
		if  (pi->phy_rxiq_resln) {
		  FOREACH_CORE(pi, r) {
		    tmp = (iqest[r]) & 0x3ff;
		    tmp = ((int16)(tmp << 6)) >> 6; /* sign extension */
		    iqest[r] = tmp;
		  }
		} else {
		  FOREACH_CORE(pi, r) {
		    tmp = (iqest[r]) & 0xff;
		    tmp = ((int16)(tmp << 8)) >> 8; /* sign extension */
		    iqest[r] = tmp;
		  }
		}

		bcopy(iqest, a, PHY_MAX_CORES*sizeof(int16));
#endif /* if CORE4>= 4 */
		phy_utils_phyreg_exit(pi);

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		wlc_phy_btcx_wlan_critical_exit(pi);
		break;
	}

	case IOV_SVAL(IOV_PHY_RXIQ_EST):

		{
			uint8 samples, antenna, resolution, lpf_hpc, dig_lpf;
			uint8 gain_correct, extra_gain_3dB, force_gain_type;

#if CORE4 >= 4
			int16 iqest_core[PHY_MAX_CORES];
			bcopy(p, iqest_core, sizeof(iqest_core));
			int_val = (int32)(iqest_core[1]<<16) + (int32)iqest_core[0];
#endif // endif
			extra_gain_3dB = (int_val >> 28) & 0xf;
			gain_correct = (int_val >> 24) & 0xf;
			lpf_hpc = (int_val >> 20) & 0x3;
			dig_lpf = (int_val >> 22) & 0x3;
			resolution = (int_val >> 16) & 0xf;
			samples = (int_val >> 8) & 0xff;
			antenna = int_val & 0xf;
			force_gain_type = (int_val >> 4) & 0xf;
#if defined(WLTEST)
			if (ISLCNCOMMONPHY(pi)) {
				uint8 index, elna, index_valid;
				phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
				antenna = int_val & 0x7f;
				elna =  (int_val >> 20) & 0x1;
				index = (((int_val >> 28) & 0xF) << 3) | ((int_val >> 21) & 0x7);
				index_valid = (int_val >> 7) & 0x1;
				if (index_valid)
					pi_lcn->rxpath_index = index;
				else
					pi_lcn->rxpath_index = 0xFF;
				pi_lcn->rxpath_elna = elna;
			}
#endif // endif
		       if (CHIPID_4324X_EPA_FAMILY(pi)) {
				uint8 index;
				phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
				index = (((int_val >> 28) & 0xF) << 3) | ((int_val >> 21) & 0x7);
				pi_nphy->gainindex = index;
			}

			if (gain_correct > 4) {
				err = BCME_RANGE;
				break;
			}

			if (!ISLCNCOMMONPHY(pi) && !(CHIPID_4324X_EPA_FAMILY(pi))) {
				if ((lpf_hpc != 0) && (lpf_hpc != 1)) {
					err = BCME_RANGE;
					break;
				}
				if (dig_lpf > 2) {
					err = BCME_RANGE;
					break;
					}
			}

			if ((resolution != 0) && (resolution != 1)) {
				err = BCME_RANGE;
				break;
			}

			if (samples < 10 || samples > 15) {
				err = BCME_RANGE;
				break;
			}

			/* Limit max number of samples to 2^14 since Lcnphy RXIQ Estimator
			 * takes too much and variable time for more than that.
			*/
			if (ISLCNCOMMONPHY(pi)) {
				samples = MIN(14, samples);
			}
			if (!(CHIPID_4324X_EPA_FAMILY(pi))) {
				if ((antenna != ANT_RX_DIV_FORCE_0) &&
					(antenna != ANT_RX_DIV_FORCE_1) &&
					(antenna != ANT_RX_DIV_DEF)) {
						err = BCME_RANGE;
						break;
				}
			}
			pi->phy_rxiq_samps = samples;
			pi->phy_rxiq_antsel = antenna;
			pi->phy_rxiq_resln = resolution;
			pi->phy_rxiq_lpfhpc = lpf_hpc;
			pi->phy_rxiq_diglpf = dig_lpf;
			pi->phy_rxiq_gain_correct = gain_correct;
			pi->phy_rxiq_extra_gain_3dB = extra_gain_3dB;
			pi->phy_rxiq_force_gain_type = force_gain_type;
		}
		break;

	case IOV_GVAL(IOV_PHYNOISE_SROM):
		if (ISHTPHY(pi) || ISACPHY(pi) ||
		(ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3))) {

			int8 noiselvl[PHY_CORE_MAX];
			uint8 core;
			uint32 pkd_noise = 0;
			if (!pi->sh->up) {
				err = BCME_NOTUP;
				break;
			}
			wlc_phy_get_SROMnoiselvl_phy(pi, noiselvl);
			for (core = PHYCORENUM(pi->pubpi.phy_corenum); core >= 1; core--) {
				pkd_noise = (pkd_noise << 8) | (uint8)(noiselvl[core-1]);
			}
			*ret_int_ptr = pkd_noise;
		} else if (ISLCNPHY(pi)) {
#if defined(WLTEST)
			int8 noiselvl;
			uint32 pkd_noise = 0;
			if (!pi->sh->up) {
				err = BCME_NOTUP;
				break;
			}
			wlc_phy_get_SROMnoiselvl_lcnphy(pi, &noiselvl);
			pkd_noise  = (uint8)noiselvl;
			*ret_int_ptr = pkd_noise;
#else
			return BCME_UNSUPPORTED;
#endif // endif
		} else {
			return BCME_UNSUPPORTED;        /* only htphy supported for now */
		}
		break;

	case IOV_GVAL(IOV_NUM_STREAM):
		if (ISNPHY(pi)) {
			int_val = 2;
		} else if (ISHTPHY(pi)) {
			int_val = 3;
		} else if (ISLCNPHY(pi)) {
			int_val = 1;
		} else {
			int_val = -1;
		}
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_BAND_RANGE):
		int_val = wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec);
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_MIN_TXPOWER):
		pi->min_txpower = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_MIN_TXPOWER):
		int_val = pi->min_txpower;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_ANT_DIV_SW_CORE0):
	{
		if (ISACPHY(pi)) {
			if ((int_val > 2) || (int_val < 0)) {
				PHY_ERROR(("Value %d is not supported \n", (uint16)int_val));
			} else {
				wlc_ant_div_sw_control(pi, (int8) int_val, 0);
			}
		} else {
			PHY_ERROR(("IOVAR is not supported for this chip \n"));
		}
		break;
	}

	case IOV_GVAL(IOV_ANT_DIV_SW_CORE0):
	{
		if (ISACPHY(pi)) {
				*ret_int_ptr = pi->u.pi_acphy->ant_swOvr_state_core0;
		} else {
			PHY_ERROR(("IOVAR is not supported for this chip \n"));
		}
		break;
	}
	case IOV_SVAL(IOV_ANT_DIV_SW_CORE1):
	{
		if (ISACPHY(pi)) {
			if ((int_val > 2) || (int_val < 0)) {
				PHY_ERROR(("Value %d is not supported \n", (uint16)int_val));
			} else {
				wlc_ant_div_sw_control(pi, (int8) int_val, 1);
			}
		} else {
			PHY_ERROR(("IOVAR is not supported for this chip \n"));
		}
		break;
	}

	case IOV_GVAL(IOV_ANT_DIV_SW_CORE1):
	{
		if (ISACPHY(pi)) {
			*ret_int_ptr = pi->u.pi_acphy->ant_swOvr_state_core1;
		} else {
			PHY_ERROR(("IOVAR is not supported for this chip \n"));
		}
		break;
	}

#if defined(WLTEST)
	case IOV_GVAL(IOV_TSSIVISI_THRESH):
		int_val = wlc_phy_tssivisible_thresh((wlc_phy_t *)pi);
		bcopy(&int_val, a, sizeof(int_val));
		break;
#endif // endif

#if defined(MACOSX)
	case IOV_GVAL(IOV_PHYWREG_LIMIT):
		int_val = pi->phy_wreg_limit;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_PHYWREG_LIMIT):
		pi->phy_wreg_limit = (uint8)int_val;
		break;
#endif // endif
	case IOV_GVAL(IOV_PHY_MUTED):
		*ret_int_ptr = PHY_MUTED(pi) ? 1 : 0;
		break;

#ifdef WLMEDIA_TXFILTER_OVERRIDE
	case IOV_GVAL(IOV_PHY_TXFILTER_SM_OVERRIDE):
		int_val = pi->sromi->txfilter_sm_override;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_PHY_TXFILTER_SM_OVERRIDE):
		if (int_val < WLC_TXFILTER_OVERRIDE_DISABLED ||
			int_val > WLC_TXFILTER_OVERRIDE_ENABLED) {
			err = BCME_RANGE;
		} else {
			pi->sromi->txfilter_sm_override = int_val;
		}

		break;
#endif /* WLMEDIA_TXFILTER_OVERRIDE */
#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG) || defined(RXDESENS_EN)
	case IOV_GVAL(IOV_PHY_RXDESENS):
		if (ISNPHY(pi))
			err = wlc_nphy_get_rxdesens((wlc_phy_t *)pi, ret_int_ptr);
		else if (ISACPHY(pi) && !ACPHY_ENABLE_FCBS_HWACI(pi) &&
		         pi->u.pi_acphy->total_desense->forced) {
			*ret_int_ptr = (int32)pi->u.pi_acphy->total_desense->ofdm_desense;
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_PHY_RXDESENS):
	{
		if (ISNPHY(pi) && (pi->sh->interference_mode == INTERFERE_NONE))
			err = wlc_nphy_set_rxdesens((wlc_phy_t *)pi, int_val);
		else
#if ACCONF || ACCONF2
		  if (ISACPHY(pi) && !(ACPHY_ENABLE_FCBS_HWACI(pi))) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
				if (int_val <= 0) {
					/* disable phy_rxdesens and restore
					 * default interference mode
					 */
					int8 negative = -1;
					pi->u.pi_acphy->total_desense->forced = FALSE;

					pi->sh->interference_mode_override = FALSE;
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						pi->sh->interference_mode =
							pi->sh->interference_mode_2G;
					} else {
						pi->sh->interference_mode =
							pi->sh->interference_mode_5G;
					}

#ifndef WLC_DISABLE_ACI
					/* turn off interference mode
					 * before entering another mode
					 */
					if (pi->sh->interference_mode != INTERFERE_NONE)
						wlc_phy_interference(pi, INTERFERE_NONE, TRUE);

					if (!wlc_phy_interference
						(pi, pi->sh->interference_mode, TRUE))
						err = BCME_BADOPTION;
#endif /* !defined(WLC_DISABLE_ACI) */

					/* restore crsmincal automode, and force crsmincal */
					wlc_phy_force_crsmin_acphy(pi, &negative);
					wlc_phy_force_lesiscale_acphy(pi, &negative);
				} else {
					/* enable phy_rxdesens and disable interference mode
					* through override mode
					*/
					pi->sh->interference_mode_override = TRUE;
					pi->sh->interference_mode_2G_override = INTERFERE_NONE;
					pi->sh->interference_mode_5G_override = INTERFERE_NONE;
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						pi->sh->interference_mode =
							pi->sh->interference_mode_2G_override;
					} else {
						pi->sh->interference_mode =
							pi->sh->interference_mode_5G_override;
					}
					wlc_phy_interference(pi, INTERFERE_NONE, TRUE);

					/* disable crsmincal */
					pi->u.pi_acphy->crsmincal_enable = FALSE;
					pi->u.pi_acphy->lesiscalecal_enable = FALSE;
					/* apply desense */
					pi->u.pi_acphy->total_desense->forced = TRUE;
					pi->u.pi_acphy->total_desense->ofdm_desense =
					  (uint8)int_val;
					pi->u.pi_acphy->total_desense->bphy_desense =
					  (uint8)int_val;
					wlc_phy_desense_apply_acphy(pi, TRUE);

				}
				wlapi_enable_mac(pi->sh->physhim);

			} else
#endif /* ACCONF || ACCONF2 */
				err = BCME_UNSUPPORTED;
		break;
	}
#endif /* defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG) || defined(RXDESENS_EN) */
#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG)
	case IOV_GVAL(IOV_NTD_GDS_LOWTXPWR):
		if (ISNPHY(pi))
			err = wlc_nphy_get_lowtxpwr((wlc_phy_t *)pi, ret_int_ptr);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_NTD_GDS_LOWTXPWR):
		if (ISNPHY(pi))
			err = wlc_nphy_set_lowtxpwr((wlc_phy_t *)pi, int_val);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PAPDCAL_INDEXDELTA):
		*ret_int_ptr = (int32)pi->papdcal_indexdelta;
		break;

	case IOV_SVAL(IOV_PAPDCAL_INDEXDELTA):
		if (int_val == -1)
			pi->papdcal_indexdelta = pi->papdcal_indexdelta_default;
		else
			pi->papdcal_indexdelta = (uint8)int_val;
		break;

#endif /* defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG) */
	case IOV_GVAL(IOV_PHY_RXANTSEL):
		if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 7))
			*ret_int_ptr = pi->nphy_enable_hw_antsel ? 1 : 0;
		break;

	case IOV_SVAL(IOV_PHY_RXANTSEL):
		if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 7)) {
			pi->nphy_enable_hw_antsel = bool_val;
			/* make sure driver is up (so clks are on) before writing to PHY regs */
			if (pi->sh->up) {
				wlc_phy_init_hw_antsel(pi);
			}
		}
		break;
#ifdef ENABLE_FCBS
	case IOV_SVAL(IOV_PHY_FCBSINIT):
		if (ISHTPHY(pi)) {
			if ((int_val >= FCBS_CHAN_A) && (int_val <= FCBS_CHAN_B)) {
				wlc_phy_fcbs_init((wlc_phy_t*)pi, int_val);
			} else {
				err = BCME_RANGE;
			}
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_SVAL(IOV_PHY_FCBS):
		if (ISACPHY(pi)) {
			pi->FCBS = (bool)int_val;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_PHY_FCBS):
		if (ISACPHY(pi)) {
			*ret_int_ptr = pi->FCBS;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_PHY_FCBSARM):
		if (ISACPHY(pi)) {
			wlc_phy_fcbs_arm((wlc_phy_t*)pi, 0xFFFF, 0);
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_PHY_FCBSEXIT):
		if (ISACPHY(pi)) {
			 wlc_phy_fcbs_exit((wlc_phy_t*)pi);
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
#endif /* ENABLE_FCBS */
#if ((ACCONF != 0) || (ACCONF2 != 0) || (NCONF != 0) || (HTCONF != 0) || (LCN40CONF != \
	0))
	case IOV_SVAL(IOV_ED_THRESH):
		if (pi->dynamic_ed_thresh_enable == 0) {
			err = wlc_phy_adjust_ed_thres(pi, &int_val, TRUE);
			pi->initial_ed_thresh = int_val;
		}
		break;
	case IOV_GVAL(IOV_ED_THRESH):
		err = wlc_phy_adjust_ed_thres(pi, ret_int_ptr, FALSE);
		break;
	case IOV_SVAL(IOV_DYNAMIC_ED_THRESH_EN):
		if(int_val==0) {
			wlc_dynamic_ed_thresh_enable(pi, FALSE);
		}else{
			wlc_dynamic_ed_thresh_enable(pi, TRUE);
		}
		break;
	case IOV_GVAL(IOV_DYNAMIC_ED_THRESH_EN):
		*ret_int_ptr = pi->dynamic_ed_thresh_enable;
		break;
	case IOV_SVAL(IOV_DYNAMIC_ED_THRESH_ACPHY_NVRAM):
		wlc_dynamic_ed_th_overwrite_acphy(pi, int_val);
		break;
	case IOV_GVAL(IOV_DYNAMIC_ED_THRESH_ACPHY_NVRAM):
		*ret_int_ptr = pi->acphy_for_dynamic_ed;
		break;
	case IOV_GVAL(IOV_DED_SETUP): {
		dynamic_ed_setup_t *setup = (dynamic_ed_setup_t *)a;
		setup->ed_monitor_window = pi->const_ed_monitor_window;
		setup->sed_dis = pi->const_sed_disable;
		setup->sed_upper_bound = pi->const_sed_upper_bound;
		setup->sed_lower_bound = pi->const_sed_lower_bound;
		setup->ed_th_high = pi->const_ed_th_high;
		setup->ed_th_low = pi->const_ed_th_low;
		setup->ed_inc_step = pi->const_ed_inc_step;
		setup->ed_dec_step = pi->const_ed_dec_step;
		break;
	}
	case IOV_SVAL(IOV_DED_SETUP): {
		dynamic_ed_setup_t *setup = (dynamic_ed_setup_t *)a;
		if (setup->ed_monitor_window > 0){
			wlc_dynamic_ed_th_overwrite_mon_win(pi, setup->ed_monitor_window);
		}
		if (setup->sed_dis <=DYN_ED_MAX_SED){
			wlc_dynamic_ed_th_overwrite_sed_dis(pi, setup->sed_dis);
		}
		if (setup->sed_upper_bound >= pi->const_sed_lower_bound && setup->sed_upper_bound <= DYN_ED_MAX_SED){
			wlc_dynamic_ed_th_overwrite_sed_high(pi, setup->sed_upper_bound);
		}
		if (setup->sed_lower_bound <= pi->const_sed_upper_bound && setup->sed_lower_bound >= DYN_ED_MIN_SED){
			wlc_dynamic_ed_th_overwrite_sed_low(pi, setup->sed_lower_bound);
		}
		if (setup->ed_th_high >= pi->const_ed_th_low && setup->ed_th_high <= DYN_ED_MAX_ACC_TH){
			wlc_dynamic_ed_th_overwrite_th_high(pi, setup->ed_th_high);
		}
		if (setup->ed_th_low >= DYN_ED_MIN_ACC_TH && setup->ed_th_low <= pi->const_ed_th_high){
			wlc_dynamic_ed_th_overwrite_th_low(pi, setup->ed_th_low);
		}
		if (setup->ed_inc_step > 0){
			wlc_dynamic_ed_th_overwrite_step_inc(pi, setup->ed_inc_step);
		}
		if (setup->ed_dec_step > 0){
			wlc_dynamic_ed_th_overwrite_step_dec(pi, setup->ed_dec_step);
		}

		break;
	}

#endif /* ACCONF || ACCONF2 || NCONF || HTCONF || LCN40CONF */
	case IOV_GVAL(IOV_PHY_BTC_RESTAGE_RXGAIN):
		err = wlc_phy_iovar_get_btc_restage_rxgain(pi, ret_int_ptr);
		break;
	case IOV_SVAL(IOV_PHY_BTC_RESTAGE_RXGAIN):
		err = wlc_phy_iovar_set_btc_restage_rxgain(pi, int_val);
		break;
	case IOV_GVAL(IOV_PHY_DSSF):
	        err = wlc_phy_iovar_get_dssf(pi, ret_int_ptr);
		break;
	case IOV_SVAL(IOV_PHY_DSSF):
		err = wlc_phy_iovar_set_dssf(pi, int_val);
		break;
	case IOV_GVAL(IOV_PHY_LESI):
	        err = wlc_phy_iovar_get_lesi(pi, ret_int_ptr);
		break;
	case IOV_SVAL(IOV_PHY_LESI):
		err = wlc_phy_iovar_set_lesi(pi, int_val);
		break;
#ifdef WL_SARLIMIT
	case IOV_SVAL(IOV_PHY_SAR_LIMIT):
	{
		wlc_phy_sar_limit_set((wlc_phy_t*)pi, (uint32)int_val);
		break;
	}

	case IOV_GVAL(IOV_PHY_TXPWR_CORE):
	{
		uint core;
		uint32 sar = 0;
		uint8 tmp;

		FOREACH_CORE(pi, core) {
			if (pi->txpwr_max_percore_override[core] != 0)
				tmp = pi->txpwr_max_percore_override[core];
			else
				tmp = pi->txpwr_max_percore[core];

			sar |= (tmp << (core * 8));
		}
		*ret_int_ptr = (int32)sar;
		break;
	}
#if defined(WLTEST) || defined(BCMDBG)
	case IOV_SVAL(IOV_PHY_TXPWR_CORE):
	{
		uint core;

		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		} else if (!ISHTPHY(pi)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		FOREACH_CORE(pi, core) {
			pi->txpwr_max_percore_override[core] =
				(uint8)(((uint32)int_val) >> (core * 8) & 0x7f);
			if (pi->txpwr_max_percore_override[core] != 0) {
				pi->txpwr_max_percore_override[core] =
					MIN(WLC_TXPWR_MAX,
					MAX(pi->txpwr_max_percore_override[core],
					pi->min_txpower));
			}
		}
		phy_tpc_recalc_tgt(pi->tpci);
		break;
	}
#endif /* WLTEST || BCMDBG */
#endif /* WL_SARLIMIT */

	case IOV_GVAL(IOV_PHY_TXSWCTRLMAP): {
		/* Getter mode, return the previously set value. */
		if (pi->pi_fptr.txswctrlmapgetptr) {
			*ret_int_ptr = (int32) pi->pi_fptr.txswctrlmapgetptr(pi);
		} else {
			/* Not implemented for this phy. */
			err = BCME_UNSUPPORTED;
			PHY_ERROR(("Command not supported for this phy\n"));
		}
		break;
	}
	case IOV_SVAL(IOV_PHY_TXSWCTRLMAP): {
		if (pi->pi_fptr.txswctrlmapsetptr) {
			if (!((int_val >= AUTO) && (int_val <= PAMODE_HI_EFF))) {
				PHY_ERROR(("Value out of range\n"));
				err = BCME_RANGE;
				break;
			}
			/* Setter mode, sets the value. */
			pi->pi_fptr.txswctrlmapsetptr(pi, (int8)int_val);
		} else {
			/* Not implemented for this phy. */
			err = BCME_UNSUPPORTED;
			PHY_ERROR(("Command not supported for this phy\n"));
		}
		break;
	}

#if defined(WFD_PHY_LL)
	case IOV_SVAL(IOV_PHY_WFD_LL_ENABLE):
		if ((int_val < 0) || (int_val > 2)) {
			err = BCME_RANGE;
			break;
		}
		if (ISNPHY(pi) || ISACPHY(pi)) {
			/* Force the channel to be active */
			pi->wfd_ll_chan_active_force =  (int_val == 2) ?TRUE : FALSE;

			pi->wfd_ll_enable_pending = (uint8)int_val;
			if (!PHY_PERICAL_MPHASE_PENDING(pi)) {
				/* Apply it since there is no CAL in progress */
				pi->wfd_ll_enable = (uint8)int_val;
				if (!int_val) {
					/* Force a watchdog CAL when disabling WFD optimization
					 * As PADP CAL has not been executed since a long time
					 * a PADP CAL is executed at the next watchdog timeout
					 */
					 pi->cal_info->last_cal_time = 0;
				}
			}
		}
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PHY_WFD_LL_ENABLE):
		if (ISNPHY(pi) || ISACPHY(pi)) {
			*ret_int_ptr = pi->wfd_ll_enable;
		}
		else err = BCME_UNSUPPORTED;
		break;
#endif /* WFD_PHY_LL */

#if defined(WLTEST) || defined(BCMDBG)

		case IOV_SVAL(IOV_PHY_ENABLE_EPA_DPD_2G):
		case IOV_SVAL(IOV_PHY_ENABLE_EPA_DPD_5G):
		{
			if (pi->pi_fptr.epadpdsetptr) {
				if ((int_val < 0) || (int_val > 1)) {
					err = BCME_RANGE;
					PHY_ERROR(("Value out of range\n"));
					break;
				}
				pi->pi_fptr.epadpdsetptr(pi, (uint8)int_val,
					(actionid == IOV_SVAL(IOV_PHY_ENABLE_EPA_DPD_2G)));
			} else {
				/* Not implemented for this phy. */
				err = BCME_UNSUPPORTED;
				PHY_ERROR(("Command not supported for this phy\n"));
			}
			break;
		}

		case IOV_GVAL(IOV_PHY_EPACAL2GMASK): {
			*ret_int_ptr = (uint32)pi->epacal2g_mask;
			break;
		}

		case IOV_SVAL(IOV_PHY_EPACAL2GMASK): {
			pi->epacal2g_mask = (uint16)int_val;
			break;
		}

		case IOV_GVAL(IOV_PHYMODE): {
			err = wlc_phy_get_val_phymode(pi, ret_int_ptr);
			break;
		}
		case IOV_SVAL(IOV_PHYMODE): {
			err = wlc_phy_set_val_phymode(pi, int_val);
			break;
		}
		case IOV_GVAL(IOV_SC_CHAN):
			err = wlc_phy_get_val_sc_chspec(pi, ret_int_ptr);
			break;

		case IOV_SVAL(IOV_SC_CHAN): {
			err = wlc_phy_set_val_sc_chspec(pi, int_val);
			break;
		}
		case IOV_GVAL(IOV_PHY_VCORE): {
			err = wlc_phy_get_val_phy_vcore(pi, ret_int_ptr);
			break;
		}
#endif /* defined(WLTEST) || defined(BCMDBG) */

	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}
#ifdef WL_PROXDETECT
#ifdef WL_PROXD_SEQ
/* Configure phy appropiately for RTT measurements */
int
wlc_phy_tof(wlc_phy_t *ppi, bool enter, bool tx, bool hw_adj, bool seq_en, int core)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi)) {
		wlc_phy_hold_upd(ppi, PHY_HOLD_FOR_TOF, enter);
		return wlc_phy_tof_acphy(pi, enter, tx, hw_adj, seq_en, core);
	}
	return BCME_UNSUPPORTED;
}
#else
/* Configure phy appropiately for RTT measurements */
int
wlc_phy_tof(wlc_phy_t *ppi, bool enter, bool hw_adj)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi)) {
		wlc_phy_hold_upd(ppi, PHY_HOLD_FOR_TOF, enter);
		wlc_phy_tof_acphy(pi, enter, FALSE, hw_adj, FALSE, 0);

		return BCME_OK;
	}
	return BCME_UNSUPPORTED;
}
#endif /* WL_PROXD_SEQ */

/* Get channel frequency response for deriving 11v rx timestamp */
int
wlc_phy_chan_freq_response(wlc_phy_t *ppi, int len, int nbits,
	int32* Hr, int32* Hi, uint32* Hraw)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi))
		return wlc_phy_chan_freq_response_acphy(pi, len, nbits, (cint32 *)Hr, Hraw);

	return BCME_UNSUPPORTED;
}
/* Get mag sqrd channel impulse response(from channel smoothing hw) to derive 11v rx timestamp */
int
wlc_phy_chan_mag_sqr_impulse_response(wlc_phy_t *ppi, int frame_type,
	int len, int offset, int nbits, int32* h, int* pgd, uint32* hraw, uint16 tof_shm_ptr)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi))
		return wlc_phy_chan_mag_sqr_impulse_response_acphy(pi,
			frame_type, len, offset, nbits, h, pgd, hraw, tof_shm_ptr);

	return BCME_UNSUPPORTED;
}
#ifdef WL_PROXD_SEQ
/* Extract information from status bytes of last rxd frame */
int wlc_phy_tof_info(wlc_phy_t *ppi, int* p_frame_type, int* p_frame_bw, int* p_cfo, int8* p_rssi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi)) {
		return wlc_phy_tof_info_acphy(pi, p_frame_type, p_frame_bw, p_cfo, p_rssi);
	}
	return BCME_UNSUPPORTED;
}
#else
/* Extract information from status bytes of last rxd frame */
int wlc_phy_tof_info(wlc_phy_t *ppi, int* p_frame_type, int* p_frame_bw, int8* p_rssi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	int cfo;

	ASSERT(pi != NULL);
	if (ISACPHY(pi))
		return wlc_phy_tof_info_acphy(pi, p_frame_type, p_frame_bw, &cfo, p_rssi);

	return BCME_UNSUPPORTED;
}
#endif /* WL_PROXD_SEQ */

/* Get timestamps from ranging sequence */
int
wlc_phy_seq_ts(wlc_phy_t *ppi, int n, void* p_buffer, int tx, int cfo, int adj, void* pparams,
	int32* p_ts, int32* p_seq_len, uint32* p_raw)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi)) {
		return wlc_phy_seq_ts_acphy(pi, n, (cint32*)p_buffer, tx, cfo, adj, pparams, p_ts,
			p_seq_len, p_raw);
	}
	return BCME_UNSUPPORTED;
}

/* Do any phy specific setup needed for each command */
void wlc_phy_tof_cmd(wlc_phy_t *ppi, bool seq)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi))
		wlc_phy_tof_cmd_acphy(pi, seq);
}

#ifdef WL_PROXD_SEQ
/* Get TOF K value for initiator and target */
int
wlc_phy_tof_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 *kip, uint32 *ktp, bool seq_en)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi))
		return wlc_phy_tof_kvalue_acphy(pi, chanspec, kip, ktp, seq_en);

	return -1;
}
#else
/* Get TOF K value for initiator and target */
int
wlc_phy_tof_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 *kip, uint32 *ktp)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	if (ISACPHY(pi))
		return wlc_phy_tof_kvalue_acphy(pi, chanspec, kip, ktp, FALSE);

	return -1;
}
#endif /* WL_PROXD_SEQ */
#endif /* WL_PROXDETECT */

#if (defined(WLTEST) || defined(WLPKTENG))
bool
wlc_phy_isperratedpden(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISACPHY(pi))
		return (*pi->pi_fptr.isperratedpdenptr)(pi);
	return FALSE;
}

void
wlc_phy_perratedpdset(wlc_phy_t *ppi, bool enable)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISACPHY(pi))
		(*pi->pi_fptr.perratedpdsetptr)(pi, enable);
}
#endif // endif

#ifdef WLTXPWR_CACHE
tx_pwr_cache_entry_t* wlc_phy_get_txpwr_cache(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return (tx_pwr_cache_entry_t*)pi->txpwr_cache;
}

#if !defined(WLC_LOW_ONLY) && !defined(WLTXPWR_CACHE_PHY_ONLY)

void wlc_phy_set_txpwr_cache(wlc_phy_t *ppi, tx_pwr_cache_entry_t* cacheptr)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->txpwr_cache = cacheptr;
}

#endif // endif

#endif	/* WLTXPWR_CACHE */

uint8
wlc_phy_get_txpwr_backoff(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->tx_pwr_backoff;

}

void
wlc_phy_get_txpwr_min(wlc_phy_t *ppi, uint8 *min_pwr)
{
	/* wlc_phy_get_thresh_acphy() returns OLPC threshold when OLPC
	 * is on otherwise returns tssivisible.
	 * XXX Why tssivisible is used as min TxPwr instead of SROM min
	 * limit only for ACPHY.
	 */
#if defined(PHYCAL_CACHING)
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISACPHY(pi))
		*min_pwr = wlc_phy_get_thresh_acphy(pi);
#endif /* PHYCAL_CACHING */

}

/*
 * if bfe capable then return max no. of streams that sta can receive in a VHT
 * NDP minus 1.
 */
uint8
wlc_phy_get_bfe_ndp_recvstreams(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISACPHY(pi)) {
		/* AC major 4 and 32 can recv 3 */
		if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev))
			return 3;
		else
			return 2;
	} else {
		ASSERT(0);
		return 0;
	}
}

uint32
wlc_phy_get_cal_dur(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	return pi->cal_dur;
}

/* Update BFR spatial expansion NumTx */
void wlc_phy_upd_bfr_exp_ntx(wlc_phy_t *pih, uint8 enable)
{
	phy_info_t *pi = (phy_info_t*)pih;
	if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, BfrMuConfigReg0, bfr_config_spatialExpansion, enable);
		MOD_PHYREG(pi, BfrMuConfigReg0, bfr_config_phyNumTx,
			PHY_BITSCNT(pi->sh->phytxchain) - 1);
	}
}
