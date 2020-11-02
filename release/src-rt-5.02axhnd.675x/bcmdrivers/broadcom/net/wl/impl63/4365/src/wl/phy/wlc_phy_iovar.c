/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abg
 * PHY iovar processing of Broadcom BCM43XX 802.11abg
 * Networking Device Driver.
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
 * $Id: wlc_phy_iovar.c 769957 2018-11-30 09:42:57Z $
 */

/*
 * This file contains high portion PHY iovar processing and table.
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLC_HIGH
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wl_dbg.h>
#include <wlc.h>
#include <bcmwifi_channels.h>

const bcm_iovar_t phy_iovars[] = {
	/* OLD, PHYTYPE specific iovars, to phase out, use unified ones at the end of this array */
#if NCONF	/* move some to internal ?? */
#if defined(BCMDBG)
	{"nphy_initgain", IOV_NPHY_INITGAIN,
	IOVF_SET_UP, IOVT_UINT16, 0
	},
	{"nphy_hpv1gain", IOV_NPHY_HPVGA1GAIN,
	IOVF_SET_UP, IOVT_INT8, 0
	},
	{"nphy_tx_temp_tone", IOV_NPHY_TX_TEMP_TONE,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
	{"nphy_cal_reset", IOV_NPHY_CAL_RESET,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
	{"nphy_est_tonepwr", IOV_NPHY_EST_TONEPWR,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"nphy_rfseq_txgain", IOV_NPHY_RFSEQ_TXGAIN,
	IOVF_GET_UP, IOVT_INT32, 0
	},
#endif /* BCMDBG */
#endif /* NCONF */

#if defined(WLTEST)
#if NCONF
	{"nphy_rssisel", IOV_NPHY_RSSISEL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_txiqlocal", IOV_NPHY_TXIQLOCAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_rxiqcal", IOV_NPHY_RXIQCAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_rssical", IOV_NPHY_RSSICAL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_gain_boost", IOV_NPHY_GAIN_BOOST,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_spuravoid", IOV_PHY_SPURAVOID,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_cckpwr_offset", IOV_NPHY_CCK_PWR_OFFSET,
	(IOVF_SET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_papdcal", IOV_NPHY_PAPDCAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_pacaltype", IOV_NPHY_PAPDCALTYPE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_skippapd", IOV_NPHY_SKIPPAPD,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_pacalindex", IOV_NPHY_PAPDCALINDEX,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"nphy_aci_scan", IOV_NPHY_ACI_SCAN,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_tbldump_minidx", IOV_NPHY_TBLDUMP_MINIDX,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_tbldump_maxidx", IOV_NPHY_TBLDUMP_MAXIDX,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_phyreg_skipdump", IOV_NPHY_PHYREG_SKIPDUMP,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"nphy_phyreg_skipcount", IOV_NPHY_PHYREG_SKIPCNT,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_caltxgain", IOV_NPHY_CALTXGAIN,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_cal_sanity", IOV_NPHY_CAL_SANITY,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
#endif /* NCONF */

#if LCNCONF
	{"lcnphy_ldovolt", IOV_LCNPHY_LDOVOLT,
	(IOVF_SET_UP), IOVT_UINT32, 0
	},
	{"lcnphy_papdepstbl", IOV_LCNPHY_PAPDEPSTBL,
	(IOVF_GET_UP | IOVF_MFG), IOVT_BUFFER, 0
	},
#endif // endif
#endif // endif

	/* ==========================================
	 * unified phy iovar, independent of PHYTYPE
	 * ==========================================
	 */
#if defined(BCMDBG) || defined(WLTEST)
	{"fast_timer", IOV_FAST_TIMER,
	(IOVF_NTRL | IOVF_MFG), IOVT_UINT32, 0
	},
	{"slow_timer", IOV_SLOW_TIMER,
	(IOVF_NTRL | IOVF_MFG), IOVT_UINT32, 0
	},
#endif /* BCMDBG || WLTEST */
	{"cal_period", IOV_CAL_PERIOD,
	0, IOVT_UINT32, 0
	},
#if defined(BCMDBG) || defined(WLTEST) || defined(WLMEDIA_CALDBG) || \
	defined(PHYCAL_CHNG_CS) || defined(WLMEDIA_FLAMES)
	{"glacial_timer", IOV_GLACIAL_TIMER,
	IOVF_NTRL, IOVT_UINT32, 0
	},
	{"pkteng_gainindex", IOV_PKTENG_GAININDEX,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"hirssi_period", IOV_HIRSSI_PERIOD,
	(IOVF_MFG), IOVT_INT16, 0
	},
	{"hirssi_en", IOV_HIRSSI_EN,
	0, IOVT_UINT8, 0
	},
	{"hirssi_byp_rssi", IOV_HIRSSI_BYP_RSSI,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"hirssi_res_rssi", IOV_HIRSSI_RES_RSSI,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"hirssi_byp_w1cnt", IOV_HIRSSI_BYP_CNT,
	(IOVF_NTRL | IOVF_MFG), IOVT_UINT16, 0
	},
	{"hirssi_res_w1cnt", IOV_HIRSSI_RES_CNT,
	(IOVF_NTRL | IOVF_MFG), IOVT_UINT16, 0
	},
	{"hirssi_status", IOV_HIRSSI_STATUS,
	0, IOVT_UINT8, 0
	},
#endif /* BCMDBG || WLTEST || WLMEDIA_CALDBG || PHYCAL_CHNG_CS || WLMEDIA_FLAMES */

#if defined(WLTEST)
	{"txinstpwr", IOV_TXINSTPWR,
	(IOVF_GET_CLK | IOVF_GET_BAND | IOVF_MFG), IOVT_BUFFER, sizeof(tx_inst_power_t)
	},
#endif // endif

#ifdef SAMPLE_COLLECT
	{"sample_collect", IOV_PHY_SAMPLE_COLLECT,
	(IOVF_GET_CLK), IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
	{"sample_data", IOV_PHY_SAMPLE_DATA,
	(IOVF_GET_CLK), IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
	{"sample_collect_gainadj", IOV_PHY_SAMPLE_COLLECT_GAIN_ADJUST,
	0, IOVT_INT8, 0
	},
	{"mac_triggered_sample_collect", IOV_PHY_MAC_TRIGGERED_SAMPLE_COLLECT,
	0, IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
	{"mac_triggered_sample_data", IOV_PHY_MAC_TRIGGERED_SAMPLE_DATA,
	0, IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
	{"sample_collect_gainidx", IOV_PHY_SAMPLE_COLLECT_GAIN_INDEX,
	0, IOVT_UINT8, 0
	},
	{"iq_metric_data", IOV_IQ_IMBALANCE_METRIC_DATA,
	(IOVF_GET_DOWN | IOVF_GET_CLK), IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
	{"iq_metric", IOV_IQ_IMBALANCE_METRIC,
	(IOVF_GET_DOWN | IOVF_GET_CLK), IOVT_BUFFER, 0
	},
	{"iq_metric_pass", IOV_IQ_IMBALANCE_METRIC_PASS,
	(IOVF_GET_DOWN | IOVF_GET_CLK), IOVT_BUFFER, 0
	},
#endif /* SAMPLE COLLECT */
#if defined(MACOSX)
	{"phywreg_limit", IOV_PHYWREG_LIMIT,
	0, IOVT_UINT32, IOVT_UINT32
	},
#endif // endif
	{"phy_muted", IOV_PHY_MUTED,
	0, IOVT_UINT8, 0
	},
	{"sromrev", IOV_SROM_REV,
	(IOVF_SET_DOWN), IOVT_UINT8, 0
	},
#ifdef WLTEST
	{"fem2g", IOV_PHY_FEM2G,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"fem5g", IOV_PHY_FEM5G,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"maxpower", IOV_PHY_MAXP,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 0
	},
#endif /* WLTEST */
#if defined(WLTEST) || defined(WLMEDIA_CALDBG)
	{"phy_cal_disable", IOV_PHY_CAL_DISABLE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
#endif // endif
#if defined(WLTEST)
	{"phymsglevel", IOV_PHYHAL_MSG,
	(0), IOVT_UINT32, 0
	},
	{"phy_cga_5g", IOV_PHY_CGA_5G,
	IOVF_SET_UP, IOVT_BUFFER, 24*sizeof(int8)
	},
	{"phy_cga_2g", IOV_PHY_CGA_2G,
	IOVF_SET_UP, IOVT_BUFFER, 14*sizeof(int8)
	},
#endif // endif
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV) || \
	defined(MACOSX) || defined(DBG_PHY_IOV)
	{"phy_watchdog", IOV_PHY_WATCHDOG,
	(IOVF_MFG), IOVT_UINT8, 0
	},
#endif // endif
#if defined(WLTEST)
	{"phy_tssi", IOV_PHY_AVGTSSI_REG,
	(IOVF_GET_UP), IOVT_BUFFER, 0
	},
	{"phy_idletssi", IOV_PHY_IDLETSSI_REG,
	(IOVF_GET_UP | IOVF_SET_UP), IOVT_BUFFER, 0
	},
	{"phy_fixed_noise", IOV_PHY_FIXED_NOISE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phynoise_polling", IOV_PHYNOISE_POLL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"carrier_suppress", IOV_CARRIER_SUPPRESS,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"unmod_rssi", IOV_UNMOD_RSSI,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"pkteng_stats", IOV_PKTENG_STATS,
	(IOVF_GET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(wl_pkteng_stats_t)
	},
	{"aci_exit_check_period", IOV_ACI_EXIT_CHECK_PERIOD,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"pavars", IOV_PAVARS,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, WL_PHY_PAVARS_LEN * sizeof(uint16)
	},
	{"phy_auxpga", IOV_PHY_AUXPGA,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 6*sizeof(uint8)
	},
	{"patrim", IOV_PATRIM,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"povars", IOV_POVARS,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, sizeof(wl_po_t)
	},
	{"rpcalvars", IOV_RPCALVARS,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 2*WL_NUM_RPCALVARS * sizeof(wl_rpcal_t)
	},
#endif // endif
#if defined(WLTEST) || defined(DBG_PHY_IOV)
	{"phy_dynamic_ml", IOV_PHY_DYN_ML,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"aci_nams", IOV_PHY_ACI_NAMS,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
#endif // endif
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV) || \
	defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG) || defined(ATE_BUILD)
	{"phy_forcecal", IOV_PHY_FORCECAL,
#ifdef WFD_PHY_LL_DEBUG
	IOVF_SET_UP, IOVT_UINT8, 0
#else
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
#endif /* WFD_PHY_LL_DEBUG */
	},
#ifndef ATE_BUILD
	{"phy_forcecal_obt", IOV_PHY_FORCECAL_OBT,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_forcecal_noise", IOV_PHY_FORCECAL_NOISE,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(uint16)
	},
	{"phy_skippapd", IOV_PHY_SKIPPAPD,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT8, 0
	},
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WLTEST)
	{"phy_papd_en_war", IOV_PAPD_EN_WAR,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
#endif // endif
#if defined(WLTEST)
	{"phy_vcocal", IOV_PHY_VCOCAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
#endif // endif
#if defined(WLTEST)
	{"phy_glitchthrsh", IOV_PHY_GLITCHK,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_noise_up", IOV_PHY_NOISE_UP,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_noise_dwn", IOV_PHY_NOISE_DWN,
	(IOVF_MFG), IOVT_UINT8, 0
	},
#endif /* #if defined(WLTEST) */
#if defined(WLTEST)
	{"tssical_start_idx", IOV_TSSICAL_START_IDX,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(int)
	},
	{"tssical_start", IOV_TSSICAL_START,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(int)
	},
	{"tssical_power", IOV_TSSICAL_POWER,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(int)
	},
	{"tssical_params", IOV_TSSICAL_PARAMS,
	(IOVF_GET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int64)
	},
	{"phy_resetcca", IOV_PHY_RESETCCA,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"tssical_txdelay", IOV_PHY_TSSITXDELAY,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_UINT32, 0
	},
#endif // endif
#if defined(WLTEST) || defined(MACOSX)
	{"phy_deaf", IOV_PHY_DEAF,
	(IOVF_GET_UP | IOVF_SET_UP), IOVT_UINT8, 0
	},
#endif // endif
#if defined(WLTEST) || defined(ATE_BUILD)
	{"phy_txpwrctrl", IOV_PHY_TXPWRCTRL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_txpwrindex", IOV_PHY_TXPWRINDEX,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"phy_txiqcc", IOV_PHY_TXIQCC,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER,  2*sizeof(int32)
	},
	{"phy_txlocc", IOV_PHY_TXLOCC,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 6
	},
#endif // endif
#if defined(WLTEST)
	{"phy_bbmult", IOV_PHY_BBMULT,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"phy_txrx_chain", IOV_PHY_TXRX_CHAIN,
	(0), IOVT_INT8, 0
	},
	{"phy_bphy_evm", IOV_PHY_BPHY_EVM,
	(IOVF_SET_DOWN | IOVF_SET_BAND | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_bphy_rfcs", IOV_PHY_BPHY_RFCS,
	(IOVF_SET_DOWN | IOVF_SET_BAND | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_enrxcore", IOV_PHY_ENABLERXCORE,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_est_tonepwr", IOV_PHY_EST_TONEPWR,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"phy_gpiosel", IOV_PHY_GPIOSEL,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"phy_5g_pwrgain", IOV_PHY_5G_PWRGAIN,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_rfseq", IOV_PHY_RFSEQ,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_scraminit", IOV_PHY_SCRAMINIT,
	(IOVF_SET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_test_tssi", IOV_PHY_TEST_TSSI,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_test_tssi_offs", IOV_PHY_TEST_TSSI_OFFS,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_test_idletssi", IOV_PHY_TEST_IDLETSSI,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_tx_tone", IOV_PHY_TX_TONE,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_tx_tone_hz", IOV_PHY_TX_TONE_HZ,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_tx_tone_stop", IOV_PHY_TX_TONE_STOP,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_activecal", IOV_PHY_ACTIVECAL,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"phy_pacalidx0", IOV_PHY_PACALIDX0,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_pacalidx1", IOV_PHY_PACALIDX1,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_iqlocalidx", IOV_PHY_IQLOCALIDX,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_pacalidx", IOV_PHY_PACALIDX,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_setrptbl", IOV_PHY_SETRPTBL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_VOID, 0
	},
	{"phy_forceimpbf", IOV_PHY_FORCEIMPBF,
	(IOVF_SET_UP | IOVF_MFG), IOVT_VOID, 0
	},
	{"phy_forcesteer", IOV_PHY_FORCESTEER,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
#if defined(WLC_LOWPOWER_BEACON_MODE)
	{"phy_lowpower_beacon_mode", IOV_PHY_LOWPOWER_BEACON_MODE,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_lowpower_beacon_mode", IOV_PHY_LOWPOWER_BEACON_MODE,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
#endif /* WLC_LOWPOWER_BEACON_MODE */
#endif // endif
#ifdef PHYMON
	{"phycal_state", IOV_PHYCAL_STATE,
	IOVF_GET_UP, IOVT_UINT32, 0,
	},
#endif /* PHYMON */
#if defined(WLTEST) || defined(AP)
	{"phy_percal", IOV_PHY_PERICAL,
	(0), IOVT_UINT8, 0
	},
#endif // endif
	{"phy_percal_delay", IOV_PHY_PERICAL_DELAY,
	(0), IOVT_UINT16, 0
	},
	{"phy_force_crsmin", IOV_PHY_FORCE_CRSMIN,
	IOVF_SET_UP, IOVT_BUFFER, 4*sizeof(int8)
	},
	{"phy_force_lesiscale", IOV_PHY_FORCE_LESISCALE,
	IOVF_SET_UP, IOVT_BUFFER, 4*sizeof(int8)
	},
#if (ACCONF || ACCONF2) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
	{"phy_force_gainlevel", IOV_PHY_FORCE_GAINLEVEL,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
#endif // endif
#if (ACCONF || ACCONF2) && defined(WLTEST)
	{"phy_force_spurmode", IOV_PHY_FORCE_SPURMODE,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
#endif // endif
#if defined(BCMDBG)
	{"phy_btcoex_desense", IOV_PHY_BTCOEX_DESENSE,
	IOVF_SET_UP, IOVT_INT32, 0
	},
#endif // endif
#if (defined(BCMDBG) || defined(BCMDBG_DUMP))
	{"phy_force_fdiqi", IOV_PHY_FORCE_FDIQI,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
#endif // endif
#if CORE4 >= 4
	{"phy_rxiqest", IOV_PHY_RXIQ_EST,
	IOVF_SET_UP, IOVT_BUFFER, 4*sizeof(int16)
	},
#else
	{"phy_rxiqest", IOV_PHY_RXIQ_EST,
	IOVF_SET_UP, IOVT_UINT32, IOVT_UINT32
	},
#endif /* defined(4CORE) */
#if ACCONF || ACCONF2
	{"edcrs", IOV_EDCRS,
	(IOVF_SET_UP|IOVF_GET_UP), IOVT_UINT8, 0
	},
	{"lp_mode", IOV_LP_MODE,
	(IOVF_SET_UP|IOVF_GET_UP), IOVT_UINT8, 0
	},
	{"lp_vco_2g", IOV_LP_VCO_2G,
	(IOVF_SET_UP|IOVF_GET_UP), IOVT_UINT8, 0
	},
	{"smth_enable", IOV_SMTH,
	(IOVF_SET_UP|IOVF_GET_UP), IOVT_UINT8, 0
	},
	{"radio_pd", IOV_RADIO_PD,
	(IOVF_SET_UP|IOVF_GET_UP), IOVT_UINT8, 0
	},
#endif /* If defined ACPHY */
	{"phynoise_srom", IOV_PHYNOISE_SROM,
	IOVF_GET_UP, IOVT_UINT32, 0
	},
	{"num_stream", IOV_NUM_STREAM,
	(0), IOVT_INT32, 0
	},
	{"band_range", IOV_BAND_RANGE,
	0, IOVT_INT8, 0
	},
	{"subband5gver", IOV_PHY_SUBBAND5GVER,
	0, IOVT_INT8, 0
	},
	{"min_txpower", IOV_MIN_TXPOWER,
	0, IOVT_UINT32, 0
	},
	{"ant_diversity_sw_core0", IOV_ANT_DIV_SW_CORE0,
	(IOVF_SET_UP|IOVF_GET_UP), IOVT_UINT8, 0
	},
	{"ant_diversity_sw_core1", IOV_ANT_DIV_SW_CORE1,
	(IOVF_SET_UP|IOVF_GET_UP), IOVT_UINT8, 0
	},
#if defined(WLTEST)
	{"tssivisi_thresh", IOV_TSSIVISI_THRESH,
	0, IOVT_UINT32, 0
	},
#endif // endif
#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX) || defined(ATE_BUILD) || \
	defined(BCMDBG_TEMPSENSE)
	{"phy_tempsense", IOV_PHY_TEMPSENSE,
	IOVF_GET_UP, IOVT_INT16, 0
	},
#endif /* BCMDBG || WLTEST || MACOSX || ATE_BUILD || BCMDBG_TEMPSENSE */
#if defined(BCMDBG) || defined(WLTEST)
	{"phy_vbatsense", IOV_PHY_VBATSENSE,
	IOVF_GET_UP, IOVT_INT32, 0
	},
#if defined(LCNCONF) || defined(LCN40CONF)
	{"lcnphy_rxiqgain", IOV_LCNPHY_RXIQGAIN,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lcnphy_rxiqgspower", IOV_LCNPHY_RXIQGSPOWER,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lcnphy_rxiqpower", IOV_LCNPHY_RXIQPOWER,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lcnphy_rxiqstatus", IOV_LCNPHY_RXIQSTATUS,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lcnphy_rxiqsteps", IOV_LCNPHY_RXIQSTEPS,
	IOVF_SET_UP, IOVT_UINT8, 0
	},
	{"lcnphy_tssimaxpwr", IOV_LCNPHY_TSSI_MAXPWR,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lcnphy_tssiminpwr", IOV_LCNPHY_TSSI_MINPWR,
	IOVF_GET_UP, IOVT_INT32, 0
	},
#endif /* #if defined(LCNCONF) || defined(LCN40CONF) */
#if LCN40CONF
	{"lcnphy_txclampdis", IOV_LCNPHY_TXPWRCLAMP_DIS,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"lcnphy_txclampofdm", IOV_LCNPHY_TXPWRCLAMP_OFDM,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lcnphy_txclampcck", IOV_LCNPHY_TXPWRCLAMP_CCK,
	IOVF_GET_UP, IOVT_INT32, 0
	},
#endif /* #if LCN40CONF */
#if ACCONF || ACCONF2
	{"rssi_cal_freq_grp_2g", IOV_PHY_RSSI_CAL_FREQ_GRP_2G,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 7*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2g", IOV_PHY_RSSI_GAIN_DELTA_2G,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int8)
	},
	{"phy_rxgainerr_2g", IOV_PHY_RXGAINERR_2G,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int8)
	},
#endif // endif
#if defined(NCONF) || defined(LCN40CONF) || ACCONF || ACCONF2
	{"phy_rssi_gain_delta_2gb0", IOV_PHY_RSSI_GAIN_DELTA_2GB0,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 8*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2gb1", IOV_PHY_RSSI_GAIN_DELTA_2GB1,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 8*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2gb2", IOV_PHY_RSSI_GAIN_DELTA_2GB2,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 8*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2gb3", IOV_PHY_RSSI_GAIN_DELTA_2GB3,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 8*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2gb4", IOV_PHY_RSSI_GAIN_DELTA_2GB4,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 8*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2g", IOV_PHY_RSSI_GAIN_DELTA_2G,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 18*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2gh", IOV_PHY_RSSI_GAIN_DELTA_2GH,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 18*sizeof(int8)
	},
	{"phy_rssi_gain_delta_2ghh", IOV_PHY_RSSI_GAIN_DELTA_2GHH,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 18*sizeof(int8)
	},
#endif /* #if defined(NCONF) || defined(LCN40CONF) */
#if defined(NCONF) || defined(LCN40CONF) || ACCONF || ACCONF2
	{"phy_rssi_gain_delta_5gl", IOV_PHY_RSSI_GAIN_DELTA_5GL,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 6*sizeof(int8)
	},
	{"phy_rssi_gain_delta_5gml", IOV_PHY_RSSI_GAIN_DELTA_5GML,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 6*sizeof(int8)
	},
	{"phy_rssi_gain_delta_5gmu", IOV_PHY_RSSI_GAIN_DELTA_5GMU,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 6*sizeof(int8)
	},
	{"phy_rssi_gain_delta_5gh", IOV_PHY_RSSI_GAIN_DELTA_5GH,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 6*sizeof(int8)
	},
	{"phy_rssi_gain_cal_temp", IOV_PHY_RSSI_GAIN_CAL_TEMP,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_INT32, 0
	},
	{"phy_rxgainerr_5gl", IOV_PHY_RXGAINERR_5GL,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int8)
	},
	{"phy_rxgainerr_5gm", IOV_PHY_RXGAINERR_5GM,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int8)
	},
	{"phy_rxgainerr_5gh", IOV_PHY_RXGAINERR_5GH,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int8)
	},
	{"phy_rxgainerr_5gu", IOV_PHY_RXGAINERR_5GU,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int8)
	},
#endif /* #if defined(NCONF) || defined(LCN40CONF) */
	{"phycal_tempdelta", IOV_PHYCAL_TEMPDELTA,
	(IOVF_MFG), IOVT_UINT8, 0
	},
#endif /* #if defined(BCMDBG) || defined(WLTEST) */
#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG) || defined(RXDESENS_EN)
	{"phy_rxdesens", IOV_PHY_RXDESENS,
	IOVF_GET_UP, IOVT_INT32, 0
	},
#endif // endif
#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG)
	{"ntd_gds_lowtxpwr", IOV_NTD_GDS_LOWTXPWR,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"papdcal_indexdelta", IOV_PAPDCAL_INDEXDELTA,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
#endif // endif
#if defined(BCMDBG)
#if LCNCONF
	{"lcnphy_cwtxpwrctrl", IOV_LCNPHY_CWTXPWRCTRL,
	IOVF_MFG, IOVT_UINT8, 0
	},
#endif /* #if LCNCONF */
#endif // endif
	{"phy_oclscdenable", IOV_PHY_OCLSCDENABLE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"lnldo2", IOV_LNLDO2,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_rxantsel", IOV_PHY_RXANTSEL,
	(0), IOVT_UINT8, 0
	},
#ifdef ENABLE_FCBS
	{"phy_fcbs_init", IOV_PHY_FCBSINIT,
	(IOVF_SET_UP), IOVT_INT8, 0
	},
	{"phy_fcbs", IOV_PHY_FCBS,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_UINT8, 0
	},
	{"phy_fcbs_arm", IOV_PHY_FCBSARM,
	(IOVF_SET_UP), IOVT_UINT8, 0
	},
	{"phy_fcbs_exit", IOV_PHY_FCBSEXIT,
	(IOVF_SET_UP), IOVT_UINT8, 0
	},
#endif /* ENABLE_FCBS */
	{"phy_crs_war", IOV_PHY_CRS_WAR,
	(0), IOVT_INT8, 0
	},
	{"subband_idx", IOV_BAND_RANGE,
	0, IOVT_INT8, 0
	},
	{"pavars2", IOV_PAVARS2,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, sizeof(wl_pavars2_t)
	},
	{"phy_btc_restage_rxgain", IOV_PHY_BTC_RESTAGE_RXGAIN,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
	{"phy_dssf", IOV_PHY_DSSF,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
	{"phy_lesi", IOV_PHY_LESI,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_UINT32, 0
	},
	{"dy_ed_setup", IOV_DED_SETUP,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_BUFFER, 0
	},
	{"phy_ed_thresh", IOV_ED_THRESH,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_INT32, 0
	},
	{"dy_ed_thresh", IOV_DYNAMIC_ED_THRESH_EN,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_INT32, 0
	},
	/* added for debug purposes. Now one can overwrite the initialization */
	{"dy_ed_thresh_acphy", IOV_DYNAMIC_ED_THRESH_ACPHY_NVRAM,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_INT32, 0
	},

#ifdef WL_SARLIMIT
	{"phy_sarlimit", IOV_PHY_SAR_LIMIT,
	0, IOVT_UINT32, 0
	},
	{"phy_txpwr_core", IOV_PHY_TXPWR_CORE,
	0, IOVT_UINT32, 0
	},
#endif /* WL_SARLIMIT */
	{"phy_txswctrlmap", IOV_PHY_TXSWCTRLMAP,
	0, IOVT_INT8, 0
	},
#ifdef WFD_PHY_LL
	{"phy_wfd_ll_enable", IOV_PHY_WFD_LL_ENABLE,
	0, IOVT_UINT8, 0
	},
#endif /* WFD_PHY_LL */
	{"phy_sromtempsense", IOV_PHY_SROM_TEMPSENSE,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT16, 0
	},
	{"rxg_rssi", IOV_PHY_RXGAIN_RSSI,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT16, 0
	},
	{"rssi_cal_rev", IOV_PHY_RSSI_CAL_REV,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT16, 0
	},
	{"rud_agc_enable", IOV_PHY_RUD_AGC_ENABLE,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT16, 0
	},
	{"gain_cal_temp", IOV_PHY_GAIN_CAL_TEMP,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT16, 0
	},
#if defined(WLTEST) || defined(BCMDBG)
	{"phy_enable_epa_dpd_2g", IOV_PHY_ENABLE_EPA_DPD_2G,
	IOVF_SET_UP, IOVT_INT8, 0
	},
	{"phy_enable_epa_dpd_5g", IOV_PHY_ENABLE_EPA_DPD_5G,
	IOVF_SET_UP, IOVT_INT8, 0
	},
	{"phy_epacal2gmask", IOV_PHY_EPACAL2GMASK,
	0, IOVT_INT16, 0
	},
	{"phymode", IOV_PHYMODE,
	0, IOVT_UINT16, 0
	},
	{"sc_chan", IOV_SC_CHAN,
	0, IOVT_UINT16, 0
	},
#endif /* defined(WLTEST) || defined(BCMDBG) */
	{"phy_vcore", IOV_PHY_VCORE,
	0, IOVT_UINT16, 0
	},
#if ACCONF || ACCONF2
	{"phy_afeoverride", IOV_PHY_AFE_OVERRIDE,
	(0), IOVT_UINT8, 0
	},
#endif // endif
#ifdef WLULB_PKTENG_DBG
	{"ulb_pkteng_mode", IOV_ULB_PKTENG_MODE,
	(IOVF_SET_DOWN), IOVT_UINT8, 0
	},
#endif /* WLULB_PKTENG_DBG */
#if defined(VASIP_HW_SUPPORT) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
	{"svmp_sampcol", IOV_SVMP_SAMPLE_COLLECT,
	(0), IOVT_BUFFER, sizeof(wl_svmp_sampcol_t)
	},
#endif /* defined(VASIP_HW_SUPPORT) && (defined(BCMDBG) || defined(BCMDBG_DUMP)) */
	/* terminating element, only add new before this */
	{NULL, 0, 0, 0, 0 }
};
#endif /* WLC_HIGH */

#ifdef WLC_LOW
#include <typedefs.h>
#include <wlc_phy_int.h>

static int
phy_legacy_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize)
{
	return wlc_phy_iovar_dispatch((phy_info_t *)ctx, aid, p, plen, a, (int)alen, (int)vsize);
}
#endif /* WLC_LOW */

#ifdef WLC_HIGH_ONLY
#include <bcm_xdr.h>

static bool
phy_legacy_pack_iov(wlc_info_t *wlc, uint32 aid, void *p, uint p_len, bcm_xdr_buf_t *b)
{
	int err;

	BCM_REFERENCE(err);

	/* Decide the buffer is 16-bit or 32-bit buffer */
	switch (IOV_ID(aid)) {
	case IOV_PKTENG_STATS:
	case IOV_POVARS:
		p_len &= ~3;
		err = bcm_xdr_pack_uint32(b, p_len);
		ASSERT(!err);
		err = bcm_xdr_pack_uint32_vec(b, p_len, p);
		ASSERT(!err);
		return TRUE;
	case IOV_PAVARS:
		p_len &= ~1;
		err = bcm_xdr_pack_uint32(b, p_len);
		ASSERT(!err);
		err = bcm_xdr_pack_uint16_vec(b, p_len, p);
		ASSERT(!err);
		return TRUE;
	}
	return FALSE;
}

static bool
phy_legacy_unpack_iov(wlc_info_t *wlc, uint32 aid, bcm_xdr_buf_t *b, void *a, uint a_len)
{
	/* Dealing with all the structures/special cases */
	switch (aid) {
	case IOV_GVAL(IOV_PHY_SAMPLE_COLLECT):
		WL_ERROR(("%s: nphy_sample_collect need endianess conversion code\n",
		          __FUNCTION__));
		break;
	case IOV_GVAL(IOV_PHY_SAMPLE_DATA):
		WL_ERROR(("%s: nphy_sample_data need endianess conversion code\n",
		          __FUNCTION__));
		break;
	}
	return FALSE;
}
#endif /* WLC_HIGH_ONLY */

/* register iovar table to the system */
#include <phy_api.h>

#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>

int phy_legacy_register_iovt(phy_info_t *pi, wlc_iocv_info_t *ii);

int
BCMATTACHFN(phy_legacy_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_iovars,
	                   phy_legacy_pack_iov, phy_legacy_unpack_iov,
	                   phy_legacy_doiovar, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
