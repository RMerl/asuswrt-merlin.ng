/**
 * @file
 * @brief
 * WLC LTE Coex module API definition
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_ltecx.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * LTE Band 40 (TDD) and Band 7 (FDD) have close proximity to WLAN 2.4GHz ISM band. Depending on
 * WLAN and LTE channel, if WLAN and LTE operating frequency are close to each other then this can
 * create mutual interference (since WLAN and MWS* are collocated). This interference leads to
 * degraded performance in both the technologies.
 *
 * The effect of interference can be mitigated by (time) sharing the Air-time between WLAN and LTE
 * Modem. LTE's Tx may impact WLAN Rx and WLAN's Tx may impact LTE's Rx. Hence, the key to good
 * coexistence solution is to avoid simultaneous WLAN_Rx+LTE_Tx or WLAN_Tx+LTE_Rx operations. This
 * is possible by sharing LTE and WLAN medium usage information with each other. The relevant
 * information from LTE Modem to WLAN is LTE frame configuration, upcoming UE (User Equipment) DL
 * (Downlink or Rx)/UL (Uplink or Tx) allocation time, DRX (LTE Inactivity) pattern. WLAN shares
 * WLAN-Priority with LTE. Since LTE has higher priority, WLAN has to use LTE's information and try
 * to fit its Tx/Rx into available LTE free window. LTE may have to defer its Tx/Rx operation for a
 * while when WLAN is in critical situation (indicated by asserting WLAN-Priority), e.g. more than
 * threshold number of BCNs are lost or WLAN Tx/Rx data rate has dropped below threshold, WLAN is in
 * association phase, etc.
 *
 * The Coexistence information between LTE and WLAN can be shared either by ERCX or by UART (BT-SIG
 * or WCI-2) interface.
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <bcmwifi_channels.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_btcx.h>
#include <wlc_scan.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <wlc_ap.h>
#include <wlc_stf.h>
#include <wlc_ampdu.h>
#include <wlc_ampdu_rx.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#include <wlc_hw_priv.h>
#include <wlc_ltecx.h>
#include <wlc_rsdb.h>

#ifdef BCMLTECOEX

enum {
	IOV_LTECX_MWS_COEX_BITMAP,
	IOV_LTECX_MWS_WLANRX_PROT,
	IOV_LTECX_WCI2_TXIND,
	IOV_LTECX_WCI2_CONFIG,
	IOV_LTECX_MWS_PARAMS,
	IOV_LTECX_MWS_COEX_BAUDRATE,
	IOV_LTECX_MWS_SCANJOIN_PROT,
	IOV_LTECX_MWS_LTETX_ADV_PROT,
	IOV_LTECX_MWS_LTERX_PROT,
	IOV_LTECX_MWS_ELNA_RSSI_THRESH,
	IOV_LTECX_MWS_IM3_PROT,
	IOV_LTECX_MWS_WIFI_SENSITIVITY,
	IOV_LTECX_MWS_DEBUG_MSG,
	IOV_LTECX_MWS_DEBUG_MODE,
	IOV_LTECX_WCI2_LOOPBACK
};

/* LTE coex iovars */
static const bcm_iovar_t ltecx_iovars[] = {
	{"mws_coex_bitmap", IOV_LTECX_MWS_COEX_BITMAP,
	(IOVF_RSDB_SET), IOVT_UINT16, 0
	},
	{"mws_wlanrx_prot", IOV_LTECX_MWS_WLANRX_PROT,
	(IOVF_RSDB_SET), IOVT_UINT8, 0
	},
	{"wci2_txind", IOV_LTECX_WCI2_TXIND,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"mws_params", IOV_LTECX_MWS_PARAMS,
	(IOVF_RSDB_SET),
	IOVT_BUFFER, sizeof(mws_params_t)
	},
	{"wci2_config", IOV_LTECX_WCI2_CONFIG,
	(IOVF_RSDB_SET),
	IOVT_BUFFER, sizeof(wci2_config_t)
	},
	{"mws_baudrate", IOV_LTECX_MWS_COEX_BAUDRATE,
	(IOVF_RSDB_SET | IOVF_SET_DOWN), IOVT_UINT8, 0
	},
	{"mws_scanjoin_prot", IOV_LTECX_MWS_SCANJOIN_PROT,
	(IOVF_RSDB_SET), IOVT_UINT16, 0
	},
	{"mws_ltetx_adv", IOV_LTECX_MWS_LTETX_ADV_PROT,
	(IOVF_RSDB_SET), IOVT_UINT16, 0
	},
	{"mws_lterx_prot", IOV_LTECX_MWS_LTERX_PROT,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"mws_im3_prot", IOV_LTECX_MWS_IM3_PROT,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"mws_wifi_sensitivity", IOV_LTECX_MWS_WIFI_SENSITIVITY,
	(IOVF_RSDB_SET), IOVT_INT16, 0
	},
	{"mws_debug_msg", IOV_LTECX_MWS_DEBUG_MSG,
	(IOVF_SET_UP | IOVF_GET_UP),
	IOVT_BUFFER, sizeof(mws_wci2_msg_t)
	},
	{"mws_debug_mode", IOV_LTECX_MWS_DEBUG_MODE,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_UINT16, 0
	},
	{"mws_elna_rssi_thresh", IOV_LTECX_MWS_ELNA_RSSI_THRESH,
	(IOVF_RSDB_SET), IOVT_INT16, 0
	},
	{"wci2_loopback", IOV_LTECX_WCI2_LOOPBACK,
	(IOVF_SET_UP | IOVF_GET_UP),
	IOVT_BUFFER, sizeof(wci2_loopback_t)
	},
	{NULL, 0, 0, 0, 0}
};

static int wlc_ltecx_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static void wlc_ltecx_watchdog(void *arg);
static void wlc_ltecx_init_getvar_array(char* vars, const char* name,
	void* dest_array, uint dest_size, ltecx_arr_datatype_t dest_type);
static void wlc_ltecx_handle_joinproc(void *ctx, bss_assoc_state_data_t *evt_data);
static void wlc_ltecx_assoc_in_prog(wlc_ltecx_info_t *ltecx, int val);

static const char BCMATTACHDATA(rstr_ltecx)[]                    = "ltecx";
static const char BCMATTACHDATA(rstr_ltecx_rssi_thresh_lmt)[]    = "ltecx_rssi_thresh_lmt";
static const char BCMATTACHDATA(rstr_ltecx_20mhz_mode)[]         = "ltecx_20mhz_mode";
static const char BCMATTACHDATA(rstr_ltecx_10mhz_mode)[]         = "ltecx_10mhz_mode";
static const char BCMATTACHDATA(rstr_ltecx_20mhz_2390_rssi_th)[] = "ltecx_20mhz_2390_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_20mhz_2385_rssi_th)[] = "ltecx_20mhz_2385_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_20mhz_2380_rssi_th)[] = "ltecx_20mhz_2380_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_20mhz_2375_rssi_th)[] = "ltecx_20mhz_2375_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_20mhz_2370_rssi_th)[] = "ltecx_20mhz_2370_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_10mhz_2395_rssi_th)[] = "ltecx_10mhz_2395_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_10mhz_2390_rssi_th)[] = "ltecx_10mhz_2390_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_10mhz_2385_rssi_th)[] = "ltecx_10mhz_2385_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_10mhz_2380_rssi_th)[] = "ltecx_10mhz_2380_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_10mhz_2375_rssi_th)[] = "ltecx_10mhz_2375_rssi_th";
static const char BCMATTACHDATA(rstr_ltecx_10mhz_2370_rssi_th)[] = "ltecx_10mhz_2370_rssi_th";
static const char BCMATTACHDATA(rstr_ltecxmux)[]                 = "ltecxmux";
static const char BCMATTACHDATA(rstr_ltecxpadnum)[]              = "ltecxpadnum";
static const char BCMATTACHDATA(rstr_ltecxfnsel)[]               = "ltecxfnsel";
static const char BCMATTACHDATA(rstr_ltecxgcigpio)[]             = "ltecxgcigpio";

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_ltecx_info_t *
BCMATTACHFN(wlc_ltecx_attach)(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	wlc_ltecx_info_t *ltecx;

	if ((ltecx = (wlc_ltecx_info_t *)
		MALLOCZ(wlc->osh, sizeof(wlc_ltecx_info_t))) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	/* Initialize the back-pointer */
	ltecx->wlc = wlc;

	/* register module */
	if (wlc_module_register(wlc->pub, ltecx_iovars, rstr_ltecx, ltecx, wlc_ltecx_doiovar,
		wlc_ltecx_watchdog, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* assoc join-start/done callback */
	if (wlc_bss_assoc_state_register(wlc, (bss_assoc_state_fn_t)wlc_ltecx_handle_joinproc,
		ltecx) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (getvar(wlc_hw->vars, rstr_ltecx_rssi_thresh_lmt) != NULL) {
			ltecx->ltecx_rssi_thresh_lmt_nvram =
				(uint8)getintvar(wlc_hw->vars, rstr_ltecx_rssi_thresh_lmt);
	}
	else {
			ltecx->ltecx_rssi_thresh_lmt_nvram = LTE_RSSI_THRESH_LMT;
	}
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_20mhz_mode,
		ltecx->ltecx_20mhz_modes, LTECX_NVRAM_PARAM_MAX, C_LTECX_DATA_TYPE_UINT32);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_10mhz_mode,
		ltecx->ltecx_10mhz_modes, LTECX_NVRAM_PARAM_MAX, C_LTECX_DATA_TYPE_UINT32);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_20mhz_2390_rssi_th,
		ltecx->ltecx_rssi_thresh_20mhz[LTECX_NVRAM_20M_RSSI_2390],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_20mhz_2385_rssi_th,
		ltecx->ltecx_rssi_thresh_20mhz[LTECX_NVRAM_20M_RSSI_2385],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_20mhz_2380_rssi_th,
		ltecx->ltecx_rssi_thresh_20mhz[LTECX_NVRAM_20M_RSSI_2380],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_20mhz_2375_rssi_th,
		ltecx->ltecx_rssi_thresh_20mhz[LTECX_NVRAM_20M_RSSI_2375],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_20mhz_2370_rssi_th,
		ltecx->ltecx_rssi_thresh_20mhz[LTECX_NVRAM_20M_RSSI_2370],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_10mhz_2395_rssi_th,
		ltecx->ltecx_rssi_thresh_10mhz[LTECX_NVRAM_10M_RSSI_2395],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_10mhz_2390_rssi_th,
		ltecx->ltecx_rssi_thresh_10mhz[LTECX_NVRAM_10M_RSSI_2390],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_10mhz_2385_rssi_th,
		ltecx->ltecx_rssi_thresh_10mhz[LTECX_NVRAM_10M_RSSI_2385],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_10mhz_2380_rssi_th,
		ltecx->ltecx_rssi_thresh_10mhz[LTECX_NVRAM_10M_RSSI_2380],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_10mhz_2375_rssi_th,
		ltecx->ltecx_rssi_thresh_10mhz[LTECX_NVRAM_10M_RSSI_2375],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);
	wlc_ltecx_init_getvar_array(wlc_hw->vars, rstr_ltecx_10mhz_2370_rssi_th,
		ltecx->ltecx_rssi_thresh_10mhz[LTECX_NVRAM_10M_RSSI_2370],
		LTECX_NVRAM_MAX_CHANNELS, C_LTECX_DATA_TYPE_INT16);

	if (getvar(wlc_hw->vars, rstr_ltecxmux) != NULL) {
		ltecx->ltecxmux = (uint32)getintvar(wlc_hw->vars, rstr_ltecxmux);
	}
	if (getvar(wlc_hw->vars, rstr_ltecxpadnum) != NULL) {
		ltecx->ltecxpadnum = (uint32)getintvar(wlc_hw->vars, rstr_ltecxpadnum);
	}
	if (getvar(wlc_hw->vars, rstr_ltecxfnsel) != NULL) {
		ltecx->ltecxfnsel = (uint32)getintvar(wlc_hw->vars, rstr_ltecxfnsel);
	}
	if (getvar(wlc_hw->vars, rstr_ltecxgcigpio) != NULL) {
		ltecx->ltecxgcigpio = (uint32)getintvar(wlc_hw->vars, rstr_ltecxgcigpio);
	}

	/* LTECX disabled from boardflags or Invalid/missing NVRAM parameter */
	if (((wlc_hw->boardflags & BFL_LTECOEX) == 0) || (ltecx->ltecxpadnum == 0) ||
		(ltecx->ltecxfnsel == 0) || (ltecx->ltecxgcigpio == 0)) {
		wlc->pub->_ltecx = FALSE;
	} else {
		wlc->pub->_ltecx = TRUE;
	}
	wlc->pub->_ltecxgci = (wlc_hw->sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT) &&
		(wlc_hw->machwcap & MCAP_BTCX);
	return ltecx;

fail:
	wlc_ltecx_detach(ltecx);
	return NULL;
}

static void
wlc_ltecx_init_getvar_array(char* vars, const char* name,
	void* dest_array, uint dest_size, ltecx_arr_datatype_t dest_type)
{
	int i;
	int array_size;

	if (getvar(vars, name) != NULL) {
		array_size = (uint32)getintvararraysize(vars, name);

		/* limit the initialization to the size of the dest array */
		array_size = MIN(array_size, dest_size);

		if (dest_type == C_LTECX_DATA_TYPE_INT16)	{
			/* initialzie the destination array with the intvar values */
			for (i = 0; i < array_size; i++) {
				((int16*)dest_array)[i] = (int16)getintvararray(vars, name, i);
			}
		} else {
			/* initialzie the destination array with the intvar values */
			for (i = 0; i < array_size; i++) {
				((uint32*)dest_array)[i] = (uint32)getintvararray(vars, name, i);
			}
		}
	}
}

void
BCMATTACHFN(wlc_ltecx_detach)(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc;

	if (ltecx == NULL)
		return;

	wlc = ltecx->wlc;
	wlc->pub->_ltecx = FALSE;
	wlc_bss_assoc_state_unregister(wlc, wlc_ltecx_handle_joinproc, ltecx);
	wlc_module_unregister(wlc->pub, rstr_ltecx, ltecx);
	MFREE(wlc->osh, ltecx, sizeof(wlc_ltecx_info_t));
}

void
BCMINITFN(wlc_ltecx_init)(wlc_ltecx_info_t *ltecx)
{
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint32 ltecx_mode;
	uint16 ltecx_hflags;
	/* cache the pointer to the LTECX shm block, which won't change after coreinit */
	ltecx->ltecx_shm_addr = 2 * wlc_bmac_read_shm(wlc_hw, M_LTECX_BLK_PTR);

	/* Enable LTE coex in ucode */
	if (wlc_hw->up) {
		wlc_bmac_suspend_mac_and_wait(wlc_hw);
	}
	/* Clear history */
	ltecx->ltecx_enabled		= 0;
	ltecx->mws_wlanrx_prot_prev	= 0;
	ltecx->mws_lterx_prot_prev	= 0;
	ltecx->ltetx_adv_prev		= 0;
	ltecx->adv_tout_prev		= 0;
	ltecx->scanjoin_prot_prev	= 0;
	ltecx->mws_ltecx_txind_prev	= 0;
	ltecx->mws_wlan_rx_ack_prev	= 0;
	ltecx->mws_rx_aggr_off		= 0;
	ltecx->mws_wifi_sensi_prev	= 0;
	ltecx->mws_im3_prot_prev	= 0;
	/* If INVALID, set baud_rate to default */
	if (ltecx->baud_rate == LTECX_WCI2_INVALID_BAUD) {
		ltecx->baud_rate = LTECX_WCI2_DEFAULT_BAUD;
	}

	/* update look-ahead, baud rate and tx_indication from NVRAM variable */
	if (ltecx->ltecx_flags) {
		ltecx->ltetx_adv = (ltecx->ltecx_flags
					    & LTECX_LOOKAHEAD_MASK)>> LTECX_LOOKAHEAD_SHIFT;
		ltecx->baud_rate = (ltecx->ltecx_flags
					    & LTECX_BAUDRATE_MASK)>> LTECX_BAUDRATE_SHIFT;
		ltecx->mws_ltecx_txind = (ltecx->ltecx_flags
						  & LTECX_TX_IND_MASK)>> LTECX_TX_IND_SHIFT;
	}

	wlc_ltecx_update_all_states(ltecx);
	if (wlc_hw->up) {
		wlc_bmac_enable_mac(wlc_hw);
	}

	ltecx_hflags = wlc_bmac_read_shm(wlc_hw, ltecx->ltecx_shm_addr
		+ M_LTECX_HOST_FLAGS);

	if (BCMLTECOEXGCI_ENAB(ltecx->wlc->pub))
	{
		ltecx_mode = LTECX_EXTRACT_MUX(ltecx->ltecxmux, LTECX_MUX_MODE_IDX);
		if (ltecx_mode == LTECX_MUX_MODE_GPIO) {
			/* Enable LTECX ERCX interface. For RSDB Init from Core 0 only */
			if (si_coreunit(wlc_hw->sih) == 0) {
				si_ercx_init(wlc_hw->sih, ltecx->ltecxmux,
					ltecx->ltecxpadnum, ltecx->ltecxfnsel, ltecx->ltecxgcigpio);
			}
			ltecx_hflags |= (1 << C_LTECX_HOST_INTERFACE);
		} else {
			/* Enable LTECX WCI-2 UART interface. For RSDB Init from Core 0 only */
			if (si_coreunit(wlc_hw->sih) == 0) {
				si_wci2_init(wlc_hw->sih, ltecx->baud_rate,
					ltecx->ltecxmux, ltecx->ltecxpadnum,
					ltecx->ltecxfnsel, ltecx->ltecxgcigpio);
			}
			ltecx_hflags &= ~(1 << C_LTECX_HOST_INTERFACE);
		}
	}
	else {
		/* Enable LTECX ERCX interface. For RSDB Init from Core 0 only */
		if (si_coreunit(wlc_hw->sih) == 0) {
			si_ercx_init(wlc_hw->sih, ltecx->ltecxmux,
				ltecx->ltecxpadnum, ltecx->ltecxfnsel, ltecx->ltecxgcigpio);
		}
		ltecx_hflags |= (1 << C_LTECX_HOST_INTERFACE);
	}
	/* Configure Interface in ucode */
	wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr
		+ M_LTECX_HOST_FLAGS, ltecx_hflags);

}

static int
wlc_ltecx_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_ltecx_info_t *ltecx = (wlc_ltecx_info_t *)ctx;
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	int err = 0;
	BCM_REFERENCE(wlc);

	if (p_len >= (int)sizeof(int_val)) {
		bcopy(params, &int_val, sizeof(int_val));
	}

	if (p_len >= (int)sizeof(int_val) * 2) {
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));
	}

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
	case IOV_GVAL(IOV_LTECX_MWS_COEX_BITMAP):
		*ret_int_ptr = (int32) ltecx->ltecx_chmap;
	    break;
	case IOV_SVAL(IOV_LTECX_MWS_COEX_BITMAP):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			uint16 bitmap = int_val;
			ltecx->ltecx_chmap = bitmap;
			/* Enable LTE coex in ucode */
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_check_chmap(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_WLANRX_PROT):
		*ret_int_ptr = (int32) ltecx->mws_wlanrx_prot;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_WLANRX_PROT):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			uint8 val = (uint8) int_val;
			if (val > C_LTECX_MWS_WLANRX_PROT_AUTO) {
				err = BCME_BADARG;
				break;
			}

			ltecx->mws_wlanrx_prot = val;
			/* Enable wlan rx protection in ucode */
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_set_wlanrx_prot(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_WCI2_TXIND):
		*ret_int_ptr = (int32) ltecx->mws_ltecx_txind;
		break;
	case IOV_SVAL(IOV_LTECX_WCI2_TXIND):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			bool w = int_val;
			ltecx->mws_ltecx_txind = w;
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltetx_indication(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_WCI2_CONFIG):
		bcopy(&ltecx->wci2_config, (char *)arg,
		sizeof(wci2_config_t));
		break;
	case IOV_SVAL(IOV_LTECX_WCI2_CONFIG):
		bcopy((char *)params, &ltecx->wci2_config,
		sizeof(wci2_config_t));
		break;
	case IOV_GVAL(IOV_LTECX_MWS_PARAMS):
		bcopy(&ltecx->mws_params, (char *)arg,
		sizeof(mws_params_t));
		break;
	case IOV_SVAL(IOV_LTECX_MWS_PARAMS):
		bcopy((char *)params, &ltecx->mws_params,
		sizeof(mws_params_t));
		break;
	case IOV_GVAL(IOV_LTECX_MWS_COEX_BAUDRATE):
		*ret_int_ptr = (int32) ltecx->baud_rate;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_COEX_BAUDRATE):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			uint8 baudrate = int_val;
			if ((baudrate == 2) || (baudrate == 3) || (baudrate == 4)) {
				ltecx->baud_rate = baudrate;
			} else {
				err = BCME_BADARG;
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_SCANJOIN_PROT):
		*ret_int_ptr = (int32) ltecx->scanjoin_prot;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_SCANJOIN_PROT):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			uint16 w = int_val;
			ltecx->scanjoin_prot = w;
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_scanjoin_prot(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_LTETX_ADV_PROT):
		*ret_int_ptr = (int32) ltecx->ltetx_adv;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_LTETX_ADV_PROT):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			uint16 lookahead_dur = int_val;
			ltecx->ltetx_adv = lookahead_dur;
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_update_ltetx_adv(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_LTERX_PROT):
		*ret_int_ptr = (int32) ltecx->mws_lterx_prot;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_LTERX_PROT):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			bool w = int_val;
			ltecx->mws_lterx_prot = w;
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_update_lterx_prot(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_ELNA_RSSI_THRESH):
		*ret_int_ptr = ltecx->mws_elna_rssi_thresh;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_ELNA_RSSI_THRESH):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			int16 rssi_thresh = int_val;
			if (rssi_thresh > 0) {
				err = BCME_BADARG;
				break;
			}
			ltecx->mws_elna_rssi_thresh = rssi_thresh;
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_update_wl_rssi_thresh(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_IM3_PROT):
		*ret_int_ptr =  (int32) ltecx->mws_im3_prot;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_IM3_PROT):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			bool w = int_val;
			ltecx->mws_im3_prot = w;
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_update_im3_prot(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_WIFI_SENSITIVITY):
		*ret_int_ptr = (int32) ltecx->mws_ltecx_wifi_sensitivity;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_WIFI_SENSITIVITY):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			int16 wifi_sensitivity = int_val;
			if (wifi_sensitivity > 0) {
				err = BCME_BADARG;
				break;
			}
			ltecx->mws_ltecx_wifi_sensitivity = wifi_sensitivity;
			if (wlc_hw->up) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			wlc_ltecx_wifi_sensitivity(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}

		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_DEBUG_MSG):
		bcopy(&ltecx->mws_wci2_msg, (char *)arg,
		sizeof(mws_wci2_msg_t));
		break;

	case IOV_SVAL(IOV_LTECX_MWS_DEBUG_MSG):
		bcopy((char *)params, &ltecx->mws_wci2_msg,
		sizeof(mws_wci2_msg_t));

		if (!wlc_hw->up) {
			err = BCME_NOTUP;
			break;
		} else {
			wlc_bmac_suspend_mac_and_wait(wlc_hw);
		}
		wlc_ltecx_update_debug_msg(ltecx);
		if (wlc_hw->up) {
			wlc_bmac_enable_mac(wlc_hw);
		}
		break;
	case IOV_GVAL(IOV_LTECX_MWS_DEBUG_MODE):
		*ret_int_ptr =  ltecx->mws_debug_mode;
		break;
	case IOV_SVAL(IOV_LTECX_MWS_DEBUG_MODE):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			uint16 w = int_val;
			if (!wlc_hw->up) {
				err = BCME_NOTUP;
				break;
			} else {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			ltecx->mws_debug_mode = w;
			wlc_ltecx_update_debug_mode(ltecx);
			if (wlc_hw->up) {
				wlc_bmac_enable_mac(wlc_hw);
			}
		}
		break;
	case IOV_GVAL(IOV_LTECX_WCI2_LOOPBACK):
	{
		wci2_loopback_rsp_t rsp;
		rsp.nbytes_rx = (int)wlc_bmac_read_shm(wlc_hw, ltecx->ltecx_shm_addr +
			M_LTECX_WCI2_LPBK_NBYTES_RX);
		rsp.nbytes_tx = (int)wlc_bmac_read_shm(wlc_hw, ltecx->ltecx_shm_addr +
			M_LTECX_WCI2_LPBK_NBYTES_TX);
		rsp.nbytes_err = (int)wlc_bmac_read_shm(wlc_hw, ltecx->ltecx_shm_addr +
			M_LTECX_WCI2_LPBK_NBYTES_ERR);
		bcopy(&rsp, (char *)arg, sizeof(wci2_loopback_rsp_t));
	}
		break;
	case IOV_SVAL(IOV_LTECX_WCI2_LOOPBACK):
		if (!BCMLTECOEX_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
		} else {
			wci2_loopback_t wci2_loopback;
			uint16 w = 0;
			bcopy((char *)params, (char *)(&wci2_loopback), sizeof(wci2_loopback));
			if (wci2_loopback.loopback_type) {
				/* prevent setting of incompatible loopback mode */
				w = wlc_bmac_read_shm(wlc_hw,
					ltecx->ltecx_shm_addr + M_LTECX_FLAGS);
				if ((w & LTECX_FLAGS_LPBK_MASK) &&
					((w & LTECX_FLAGS_LPBK_MASK) !=
					wci2_loopback.loopback_type)) {
					err = BCME_BADARG;
					break;
				}
				w |= wci2_loopback.loopback_type;
				if (wci2_loopback.loopback_type != LTECX_FLAGS_LPBK_OFF) {
					/* Reuse CRTI code to start test */
					ltecx->mws_debug_mode = 1;
					/* Init counters */
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_WCI2_LPBK_NBYTES_RX,
						htol16(0));
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_WCI2_LPBK_NBYTES_TX,
						htol16(0));
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_WCI2_LPBK_NBYTES_ERR,
						htol16(0));
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_CRTI_MSG,
						htol16(0));
					/* CRTI_REPEATS=0 presumes Olympic Rx loopback test */
					/* Initialized for Olympic Tx loopback test further below */
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_CRTI_REPEATS,
						htol16(0));
					/* Suppress scans during lpbk */
					wlc_hw->wlc->scan->state |= SCAN_STATE_SUPPRESS;
				}
				if (wci2_loopback.loopback_type == LTECX_FLAGS_LPBKSRC_MASK) {
					/* Set bit15 of CRTI_MSG to distinguish Olympic Tx
					 * loopback test from RIM test
					 */
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_CRTI_MSG,
						0x8000 | wci2_loopback.packet);
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_CRTI_INTERVAL,
						20); /* TODO: hardcoded for now */
					wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
						M_LTECX_CRTI_REPEATS,
						htol16(wci2_loopback.repeat_ct));
				}
			}
			else {
				/* Lpbk disabled. Reenable scans */
				wlc_hw->wlc->scan->state &= ~SCAN_STATE_SUPPRESS;
				/* CRTI ucode used to start test - now stop test */
				ltecx->mws_debug_mode = 0;
			}
			wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr + M_LTECX_FLAGS, w);
		}
	    break;

	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static void
wlc_ltecx_watchdog(void *arg)
{
	wlc_ltecx_info_t *ltecx = (wlc_ltecx_info_t *)arg;
	wlc_info_t *wlc = ltecx->wlc;
	BCM_REFERENCE(wlc);
	if (BCMLTECOEX_ENAB(wlc->pub)) {
		wlc_ltecx_update_all_states(ltecx);

		uint16 re_enable_rxaggr_off;
		if (BT3P_HW_COEX(wlc) && CHSPEC_IS2G(wlc->chanspec)) {
			int btc_mode = wlc_btc_mode_get(wlc);
			if (wlc_btc_mode_not_parallel(btc_mode)) {
				/* Make sure STA is on the home channel to avoid changing AMPDU
				 * state during scanning
				 */
				if (AMPDU_ENAB(wlc->pub) && !SCAN_IN_PROGRESS(wlc->scan) &&
					wlc->pub->associated) {
					if (wlc_ltecx_turnoff_rx_aggr(ltecx)) {
						wlc_ampdu_agg_state_update_rx_all(wlc, OFF);
						ltecx->mws_rx_aggr_off = TRUE;
					}
					if (wlc_ltecx_turnoff_tx_aggr(ltecx)) {
						wlc_ampdu_agg_state_update_tx_all(wlc, OFF);
					}
					if (!wlc_btc_turnoff_aggr(wlc)) {
						if ((!wlc_ltecx_turnoff_rx_aggr(ltecx)	&&
							ltecx->mws_rx_aggr_off) &&
							!wlc_btc_active(wlc)) {
							re_enable_rxaggr_off = wlc_bmac_read_shm(
								wlc->hw, ltecx->ltecx_shm_addr
								+ M_LTECX_RX_REAGGR);
							if (!re_enable_rxaggr_off) {
								/* Resume Rx aggregation per
								  * SWWLAN-32809
								  */
								wlc_ampdu_agg_state_update_rx_all(
									wlc, ON);
								ltecx->mws_rx_aggr_off = FALSE;
							}
						}
						if (!wlc_ltecx_turnoff_tx_aggr(ltecx)) {
							wlc_ampdu_agg_state_update_tx_all(wlc, ON);
						}
					}
				}
			} else {
				if (wlc_ltecx_turnoff_rx_aggr(ltecx)) {
					wlc_ampdu_agg_state_update_rx_all(wlc, OFF);
					ltecx->mws_rx_aggr_off = TRUE;
				} else if (ltecx->mws_rx_aggr_off &&
					!wlc_btc_active(wlc)) {
					re_enable_rxaggr_off = wlc_bmac_read_shm(wlc->hw,
						ltecx->ltecx_shm_addr + M_LTECX_RX_REAGGR);
					if (!re_enable_rxaggr_off) {
						/* Resume Rx aggregation per SWWLAN-32809 */
						wlc_ampdu_agg_state_update_rx_all(wlc, ON);
						ltecx->mws_rx_aggr_off = FALSE;
					}
				}
				/* Don't resume tx aggr while LTECX is active */
				if (wlc_ltecx_turnoff_tx_aggr(ltecx)) {
					wlc_ampdu_agg_state_update_tx_all(wlc, OFF);
				} else {
					wlc_ampdu_agg_state_update_tx_all(wlc, ON);
				}
			}
		}
	}
}

void
wlc_ltecx_update_all_states(wlc_ltecx_info_t *ltecx)
{
	/* update ltecx parameters based on nvram and lte freq */
	wlc_ltecx_update_status(ltecx);
	/* enable/disable ltecx based on channel map */
	wlc_ltecx_check_chmap(ltecx);
	/* update elna status based on rssi_thresh
	 * First update the elna status and
	 * update the other flags
	 */
	wlc_ltecx_update_wl_rssi_thresh(ltecx);
	/* update protection type */
	wlc_ltecx_set_wlanrx_prot(ltecx);
	/* Allow WLAN TX during LTE TX based on eLNA bypass status */
	wlc_ltecx_update_wlanrx_ack(ltecx);
	/* update look ahead and protection
	 * advance duration
	 */
	wlc_ltecx_update_ltetx_adv(ltecx);
	/* update lterx protection type */
	wlc_ltecx_update_lterx_prot(ltecx);
	/* update im3 protection type */
	wlc_ltecx_update_im3_prot(ltecx);
	/* update scanjoin protection */
	wlc_ltecx_scanjoin_prot(ltecx);
	/* update ltetx indication */
	wlc_ltetx_indication(ltecx);
	/* Check RSSI with WIFI Sensitivity */
	wlc_ltecx_wifi_sensitivity(ltecx);
	wlc_ltecx_update_debug_mode(ltecx);
}

void
wlc_ltecx_check_chmap(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 ltecx_hflags, ltecx_state;
	bool ltecx_en;

	if (!si_iscoreup(wlc_hw->sih)) {
		return;
	}

	ltecx_en = 0;	/* IMP: Initialize to disable */
	/* Decide if ltecx algo needs to be enabled */
	if (CHSPEC_IS2G(wlc->chanspec)) {
		/* enable ltecx algo as per ltecx_chmap */
		chanspec_t chan = CHSPEC_CHANNEL(wlc->chanspec);
		if (ltecx->ltecx_chmap & (1 << (chan - 1))) {
			ltecx_en	= 1;
		}
	}

	if (ltecx->ltecx_shm_addr == NULL)
		return;

	ltecx_state = wlc_bmac_read_shm(wlc_hw,
		ltecx->ltecx_shm_addr + M_LTECX_STATE);

	/* Update ucode ltecx flags */
	if (ltecx->ltecx_enabled != ltecx_en)	{
		if (ltecx->ltecx_shm_addr) {
			ltecx_hflags = wlc_bmac_read_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
			if (ltecx_en == 1) {
				ltecx_hflags |= (1<<C_LTECX_HOST_COEX_EN);
			}
			else  {
				ltecx_hflags &= ~(1<<C_LTECX_HOST_COEX_EN);
				ltecx_state  |= (1<<C_LTECX_ST_IDLE);
				wlc_bmac_write_shm(wlc_hw,
					ltecx->ltecx_shm_addr + M_LTECX_STATE, ltecx_state);
			}
			wlc_bmac_write_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS, ltecx_hflags);
			ltecx->ltecx_enabled = ltecx_en;
		}
	}
	ltecx->ltecx_idle = (ltecx_state >> C_LTECX_ST_IDLE) & 0x1;
}

void
wlc_ltecx_set_wlanrx_prot(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint8 prot;
	uint16 shm_val;
	if (!si_iscoreup(wlc_hw->sih))
		return;

	prot = ltecx->mws_wlanrx_prot;
	/* Set protection to NONE if !2G, or 2G but eLNA bypass */
	if (!CHSPEC_IS2G(wlc->chanspec) || (ltecx->mws_elna_bypass)) {
		prot = C_LTECX_MWS_WLANRX_PROT_NONE;
	}
	if (prot != ltecx->mws_wlanrx_prot_prev)	{
		if (ltecx->ltecx_shm_addr) {
			shm_val = wlc_bmac_read_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
			/* Clear all protection type bits  */
			shm_val = (shm_val &
				~((1 << C_LTECX_HOST_PROT_TYPE_NONE_TMP)
				|(1 << C_LTECX_HOST_PROT_TYPE_PM_CTS)
				|(1 << C_LTECX_HOST_PROT_TYPE_AUTO)));
			/* Set appropriate protection type bit */
			if (prot == C_LTECX_MWS_WLANRX_PROT_NONE) {
				shm_val = shm_val | (1 << C_LTECX_HOST_PROT_TYPE_NONE_TMP);
			} else if (prot == C_LTECX_MWS_WLANRX_PROT_CTS) {
				shm_val = shm_val | (1 << C_LTECX_HOST_PROT_TYPE_PM_CTS);
			} else if (prot == C_LTECX_MWS_WLANRX_PROT_PM) {
				shm_val = shm_val | (0 << C_LTECX_HOST_PROT_TYPE_PM_CTS);
			} else if (prot == C_LTECX_MWS_WLANRX_PROT_AUTO) {
				shm_val = shm_val | (1 << C_LTECX_HOST_PROT_TYPE_AUTO);
			}
			wlc_bmac_write_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS, shm_val);
			ltecx->mws_wlanrx_prot_prev = prot;
		}
	}
}

void
wlc_ltecx_update_ltetx_adv(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 adv_tout = 0;
	uint16 prot_type;

	if (!si_iscoreup(wlc_hw->sih)) {
		return;
	}

	/* Update ltetx_adv and adv_tout for 2G band only */
	if (CHSPEC_IS2G(wlc->chanspec))	{
		if (ltecx->ltecx_shm_addr)	{
			if (ltecx->ltetx_adv != ltecx->ltetx_adv_prev)	{
				wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
					M_LTECX_TX_LOOKAHEAD_DUR, ltecx->ltetx_adv);
				ltecx->ltetx_adv_prev = ltecx->ltetx_adv;
			}
			/* NOTE: C_LTECX_HOST_PROT_TYPE_CTS may be changed by ucode in AUTO mode */
			prot_type = wlc_bmac_read_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
		    if (prot_type & (1 << C_LTECX_HOST_PROT_TYPE_CTS)) {
				if (ltecx->ltetx_adv >= 500) {
				    adv_tout = ltecx->ltetx_adv - 500;
				} else {
				    adv_tout = ltecx->ltetx_adv;
				}
			} else {
			    if (ltecx->ltetx_adv >= 800) {
					adv_tout = ltecx->ltetx_adv - 800;
				} else {
					adv_tout = ltecx->ltetx_adv;
				}
			}
			if (adv_tout != ltecx->adv_tout_prev)	{
				wlc_bmac_write_shm(wlc_hw,
					ltecx->ltecx_shm_addr + M_LTECX_PROT_ADV_TIME, adv_tout);
				ltecx->adv_tout_prev = adv_tout;
			}
		}
	}
}

void
wlc_ltecx_update_lterx_prot(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 ltecx_hflags;
	bool lterx_prot;

	if (!si_iscoreup (wlc_hw->sih)) {
		return;
	}

	lterx_prot = 0;
	if (CHSPEC_IS2G(wlc->chanspec))	{
		lterx_prot	= ltecx->mws_lterx_prot;
	}
	if (lterx_prot != ltecx->mws_lterx_prot_prev)	{
		if (ltecx->ltecx_shm_addr)	{
			ltecx_hflags = wlc_bmac_read_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
			if (lterx_prot == 0) {
				ltecx_hflags |= (1 << C_LTECX_HOST_TX_ALWAYS);
			} else {
				ltecx_hflags &= ~(1 << C_LTECX_HOST_TX_ALWAYS);
			}
			wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr
				+ M_LTECX_HOST_FLAGS, ltecx_hflags);
			ltecx->mws_lterx_prot_prev = lterx_prot;
		}
	}
}

void
wlc_ltecx_scanjoin_prot(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 scanjoin_prot, ltecx_hflags;

	if (!si_iscoreup(wlc_hw->sih))
		return;

	scanjoin_prot = 0;
	if (CHSPEC_IS2G(wlc->chanspec)) {
		scanjoin_prot = ltecx->scanjoin_prot;
	}
	if (scanjoin_prot != ltecx->scanjoin_prot_prev)	{
		if (ltecx->ltecx_shm_addr)	{
			ltecx_hflags = wlc_bmac_read_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
			if (scanjoin_prot == 0)
				ltecx_hflags &= ~(1 << C_LTECX_HOST_SCANJOIN_PROT);
			else
				ltecx_hflags |= (1 << C_LTECX_HOST_SCANJOIN_PROT);
			wlc_bmac_write_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS, ltecx_hflags);
			ltecx->scanjoin_prot_prev = scanjoin_prot;
		}
	}
}

void
wlc_ltetx_indication(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 ltecx_hflags;
	bool ltecx_txind;

	if (!si_iscoreup(wlc_hw->sih)) {
		return;
	}

	ltecx_txind = 0;
	if (CHSPEC_IS2G(wlc->chanspec)) {
		ltecx_txind = ltecx->mws_ltecx_txind;
	}
	if (ltecx_txind != ltecx->mws_ltecx_txind_prev)	{
		if (ltecx->ltecx_shm_addr)	{
			ltecx_hflags = wlc_bmac_read_shm(wlc_hw, ltecx->ltecx_shm_addr
				+ M_LTECX_HOST_FLAGS);
			if (ltecx_txind == 0) {
				ltecx_hflags &= ~(1 << C_LTECX_HOST_TXIND);
			} else {
				ltecx_hflags |= (1 << C_LTECX_HOST_TXIND);
			}
			wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr
					+ M_LTECX_HOST_FLAGS, ltecx_hflags);
			ltecx->mws_ltecx_txind_prev = ltecx_txind;
		}
	}
}

bool
wlc_ltecx_get_lte_status(wlc_ltecx_info_t *ltecx)
{
	if (ltecx->ltecx_enabled && !ltecx->ltecx_idle) {
		return TRUE;
	} else {
		return FALSE;
	}
}

bool
wlc_ltecx_turnoff_rx_aggr(wlc_ltecx_info_t *ltecx)
{
	/* Turn Off Rx Aggr if LTECX is active and not in eLNA bypass mode */
	return (wlc_ltecx_get_lte_status(ltecx) &&
		!ltecx->mws_elna_bypass);
}

bool
wlc_ltecx_turnoff_tx_aggr(wlc_ltecx_info_t *ltecx)
{
	return (wlc_ltecx_get_lte_status(ltecx));
}

bool
wlc_ltecx_get_lte_map(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	if (CHSPEC_IS2G(wlc->chanspec) && (BCMLTECOEX_ENAB(wlc->pub))) {
		chanspec_t chan = CHSPEC_CHANNEL(wlc->chanspec);
		if ((ltecx->ltecx_enabled) &&
			(ltecx->ltecx_chmap & (1 << (chan - 1)))) {
			return TRUE;
		}
	}
	return FALSE;
}

void
wlc_ltecx_update_wl_rssi_thresh(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	int16 ltecx_rssi_thresh;
	uint8 thresh_lmt;

	if (!si_iscoreup (wlc_hw->sih)) {
		return;
	}

	if (CHSPEC_IS2G(wlc->chanspec)) {
		if (ltecx->ltecx_shm_addr && wlc_ltecx_get_lte_status(wlc->ltecx) &&
			(wlc->cfg->link->rssi != WLC_RSSI_INVALID)) {
			ltecx_rssi_thresh = ltecx->mws_elna_rssi_thresh;
			/* mws_ltecx_rssi_thresh_lmt will be zero the very first time
			 * to make the init time decision if we are entering the hysterisis from
			 * lower RSSI side or higher RSSI side (w.r.t. RSSI thresh)
			 */
			thresh_lmt = ltecx->mws_ltecx_rssi_thresh_lmt;
			if (wlc->cfg->link->rssi >= (int) (ltecx_rssi_thresh + thresh_lmt)) {
				ltecx->mws_elna_bypass = TRUE;
				ltecx->mws_ltecx_rssi_thresh_lmt =
					ltecx->ltecx_rssi_thresh_lmt_nvram;
			} else if (wlc->cfg->link->rssi < (int) (ltecx_rssi_thresh - thresh_lmt)) {
				ltecx->mws_elna_bypass = FALSE;
				ltecx->mws_ltecx_rssi_thresh_lmt =
					ltecx->ltecx_rssi_thresh_lmt_nvram;
			}
		} else {
			ltecx->mws_elna_bypass = FALSE;
		}
	}
}

void
wlc_ltecx_update_wlanrx_ack(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 ltecx_hflags;
	bool wlan_rx_ack;

	if (!si_iscoreup (wlc_hw->sih)) {
		return;
	}

	wlan_rx_ack = 0;
	if (CHSPEC_IS2G(wlc->chanspec))   {
		wlan_rx_ack  = ltecx->mws_elna_bypass;
	}
	if (wlan_rx_ack != ltecx->mws_wlan_rx_ack_prev)    {
		if (ltecx->ltecx_shm_addr)   {
			ltecx_hflags = wlc_bmac_read_shm(wlc_hw,
			ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
			if (wlan_rx_ack == 1) {
				ltecx_hflags |= (1 << C_LTECX_HOST_RX_ACK);
			} else {
				ltecx_hflags &= ~(1 << C_LTECX_HOST_RX_ACK);
			}
			wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr
				+ M_LTECX_HOST_FLAGS, ltecx_hflags);
			ltecx->mws_wlan_rx_ack_prev = wlan_rx_ack;
		}
	}
}

int
wlc_ltecx_chk_elna_bypass_mode(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	if (CHSPEC_IS2G(wlc->chanspec) && (BCMLTECOEX_ENAB(wlc->pub))) {
		chanspec_t chan = CHSPEC_CHANNEL(wlc->chanspec);
		if ((ltecx->ltecx_enabled) &&
			(ltecx->ltecx_chmap & (1 << (chan - 1)))) {
			if (ltecx->mws_elna_bypass == TRUE) {
				return 1;
			} else {
				return 0;
			}
		}
	}
	return 0;
}

void
wlc_ltecx_update_status(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	int i, ch, lte_freq_half_bw = 0, freq, coex_chmap = 0;
	uint8 wlanrx_prot_min_ch = 0, lterx_prot_min_ch = 0, scanjoin_prot_min_ch = 0;
	uint8 freq_index = 0, index;
	uint32 wlanrx_prot_info = 0, lterx_prot_info = 0, scan_join_prot_info = 0;
	chanspec_t chan;

	/* No processing required if !2G */
	if (!CHSPEC_IS2G(wlc->chanspec)) {
		return;
	}
	chan = CHSPEC_CHANNEL(wlc->chanspec);

	if (!ltecx->ltecx_10mhz_modes[LTECX_NVRAM_WLANRX_PROT] &&
		!ltecx->ltecx_20mhz_modes[LTECX_NVRAM_WLANRX_PROT] &&
		!ltecx->ltecx_10mhz_modes[LTECX_NVRAM_LTERX_PROT] &&
		!ltecx->ltecx_20mhz_modes[LTECX_NVRAM_LTERX_PROT]) {
		return;
	}
	if ((ltecx->mws_params.mws_tx_channel_bw !=
		ltecx->lte_channel_bw_prev)||
		(ltecx->mws_params.mws_tx_center_freq !=
		ltecx->lte_center_freq_prev)) {
		ltecx->lte_channel_bw_prev =
			ltecx->mws_params.mws_tx_channel_bw;
		ltecx->lte_center_freq_prev =
			ltecx->mws_params.mws_tx_center_freq;

		if (ltecx->mws_params.mws_tx_channel_bw == LTE_CHANNEL_BW_20MHZ)	{
			wlanrx_prot_info =
				ltecx->ltecx_20mhz_modes[LTECX_NVRAM_WLANRX_PROT];
			lterx_prot_info =
				ltecx->ltecx_20mhz_modes[LTECX_NVRAM_LTERX_PROT];
			scan_join_prot_info =
				ltecx->ltecx_20mhz_modes[LTECX_NVRAM_SCANJOIN_PROT];
			lte_freq_half_bw = LTE_20MHZ_INIT_STEP;
		} else if (ltecx->mws_params.mws_tx_channel_bw == LTE_CHANNEL_BW_10MHZ) {
			wlanrx_prot_info =
				ltecx->ltecx_10mhz_modes[LTECX_NVRAM_WLANRX_PROT];
			lterx_prot_info =
				ltecx->ltecx_10mhz_modes[LTECX_NVRAM_LTERX_PROT];
			scan_join_prot_info =
				ltecx->ltecx_10mhz_modes[LTECX_NVRAM_SCANJOIN_PROT];
			lte_freq_half_bw = LTE_10MHZ_INIT_STEP;
		} else {
			return;
		}
		for (freq = (LTE_BAND40_MAX_FREQ - lte_freq_half_bw);
			freq > LTE_BAND40_MIN_FREQ;
			freq = freq- LTE_FREQ_STEP_SIZE) {
			if ((ltecx->mws_params.mws_tx_center_freq +
				LTE_MAX_FREQ_DEVIATION) >= freq) {
				break;
			}
			freq_index ++;
		}
		if (freq_index < LTE_FREQ_STEP_MAX) {
			wlanrx_prot_min_ch =  (wlanrx_prot_info >>
				(freq_index * LTECX_NVRAM_GET_PROT_MASK)) & LTECX_MIN_CH_MASK;
			lterx_prot_min_ch =  (lterx_prot_info >>
				(freq_index * LTECX_NVRAM_GET_PROT_MASK)) & LTECX_MIN_CH_MASK;
			scanjoin_prot_min_ch =  (scan_join_prot_info >>
				(freq_index * LTECX_NVRAM_GET_PROT_MASK)) & LTECX_MIN_CH_MASK;
		}
		ltecx->mws_wlanrx_prot_min_ch = wlanrx_prot_min_ch;
		ltecx->mws_lterx_prot_min_ch = lterx_prot_min_ch;
		ltecx->mws_scanjoin_prot_min_ch = scanjoin_prot_min_ch;
		ltecx->mws_lte_freq_index = freq_index;

		ch = (wlanrx_prot_min_ch >= lterx_prot_min_ch)
					? wlanrx_prot_min_ch: lterx_prot_min_ch;
		for (i = 0; i < ch; i++) {
			coex_chmap |= (1<<i);
		}
		/* update coex_bitmap */
		ltecx->ltecx_chmap = coex_chmap;
	}

	wlanrx_prot_min_ch = ltecx->mws_wlanrx_prot_min_ch;
	lterx_prot_min_ch = ltecx->mws_lterx_prot_min_ch;
	scanjoin_prot_min_ch = ltecx->mws_scanjoin_prot_min_ch;

	/* update wlanrx protection */
	if (wlanrx_prot_min_ch && chan <= wlanrx_prot_min_ch) {
		ltecx->mws_wlanrx_prot = 1;
	} else {
		ltecx->mws_wlanrx_prot = 0;
	}
	/* update lterx protection */
	if (lterx_prot_min_ch && chan <= lterx_prot_min_ch) {
		ltecx->mws_lterx_prot = 1;
	} else {
		ltecx->mws_lterx_prot = 0;
	}
	/* update scanjoin protection */
	if (scanjoin_prot_min_ch && chan <= scanjoin_prot_min_ch) {
		ltecx->scanjoin_prot = 1;
	} else {
		ltecx->scanjoin_prot = 0;
	}
	/* update wl rssi threshold */
	index = ltecx->mws_lte_freq_index;
	if (ltecx->lte_channel_bw_prev == LTE_CHANNEL_BW_20MHZ) {
		if (index < LTECX_NVRAM_RSSI_THRESH_20MHZ) {
			ltecx->mws_elna_rssi_thresh =
				ltecx->ltecx_rssi_thresh_20mhz[index][chan-1];
		} else {
			ltecx->mws_elna_rssi_thresh = 0;
		}
	} else if (ltecx->lte_channel_bw_prev == LTE_CHANNEL_BW_10MHZ) {
		if (index < LTECX_NVRAM_RSSI_THRESH_10MHZ) {
			ltecx->mws_elna_rssi_thresh =
				ltecx->ltecx_rssi_thresh_10mhz[index][chan-1];
		} else {
			ltecx->mws_elna_rssi_thresh = 0;
		}
	}
}

void
wlc_ltecx_update_im3_prot(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 ltecx_hflags = 0;

	if (!si_iscoreup (wlc_hw->sih)) {
		return;
	}

	if (CHSPEC_IS2G(wlc->chanspec) && (ltecx->ltecx_shm_addr) &&
		(ltecx->mws_im3_prot != ltecx->mws_im3_prot_prev)) {
		ltecx_hflags = wlc_bmac_read_shm(wlc_hw,
			ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
		if (ltecx->mws_im3_prot) {
			ltecx_hflags |= (1 << C_LTECX_HOST_PROT_TXRX);
		} else {
			ltecx_hflags &= ~(1 << C_LTECX_HOST_PROT_TXRX);
		}
		wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr
			+ M_LTECX_HOST_FLAGS, ltecx_hflags);
		ltecx->mws_im3_prot_prev = ltecx->mws_im3_prot;
	}
}

void
wlc_ltecx_wifi_sensitivity(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	int ltecx_hflags;
	int16 mws_wifi_sensi = 0;

	if (!si_iscoreup (wlc_hw->sih)) {
		return;
	}

	if (CHSPEC_IS2G(wlc->chanspec)) {
		if (ltecx->ltecx_shm_addr && ltecx->mws_wlanrx_prot) {
			if (wlc->cfg->link->rssi > (int) ltecx->mws_ltecx_wifi_sensitivity) {
				mws_wifi_sensi = 1;
			} else {
				mws_wifi_sensi = 0;
			}

			if (mws_wifi_sensi !=
				ltecx->mws_wifi_sensi_prev) {
				ltecx_hflags = wlc_bmac_read_shm(wlc_hw,
				ltecx->ltecx_shm_addr + M_LTECX_HOST_FLAGS);
				if (mws_wifi_sensi) {
					ltecx_hflags |= (1 << C_LTECX_HOST_PROT_TYPE_NONE_TMP);
				} else {
					ltecx_hflags &= ~(1 << C_LTECX_HOST_PROT_TYPE_NONE_TMP);
				}
				wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr
				+ M_LTECX_HOST_FLAGS, ltecx_hflags);
				ltecx->mws_wifi_sensi_prev =
					mws_wifi_sensi;
			}
		}
	}
}

void
wlc_ltecx_update_debug_msg(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;

	if (!si_iscoreup(wlc_hw->sih)) {
		return;
	}

	if (CHSPEC_IS2G(wlc->chanspec) && (ltecx->ltecx_shm_addr)) {
		wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
			M_LTECX_CRTI_MSG, ltecx->mws_wci2_msg.mws_wci2_data);
		wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
			M_LTECX_CRTI_INTERVAL, ltecx->mws_wci2_msg.mws_wci2_interval);
		wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr +
			M_LTECX_CRTI_REPEATS, ltecx->mws_wci2_msg.mws_wci2_repeat);
	}
}

void
wlc_ltecx_update_debug_mode(wlc_ltecx_info_t *ltecx)
{
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	uint16 ltecx_state;
	if (!si_iscoreup(wlc_hw->sih)) {
		return;
	}

	if (CHSPEC_IS2G(wlc->chanspec) && (ltecx->ltecx_shm_addr) &&
		(ltecx->mws_debug_mode != ltecx->mws_debug_mode_prev)) {
		ltecx->mws_debug_mode_prev = ltecx->mws_debug_mode;
		ltecx_state = wlc_bmac_read_shm(wlc_hw,
			ltecx->ltecx_shm_addr + M_LTECX_STATE);
		if (ltecx->mws_debug_mode) {
			/* Disable inbandIntMask for FrmSync, LTE_Rx and LTE_Tx
			  * Note: FrameSync, LTE Rx & LTE Tx happen to share the same REGIDX
			  * Hence a single Access is sufficient
			  */
			si_gci_indirect(wlc_hw->sih, GCI_REGIDX(GCI_LTE_FRAMESYNC_POS),
				OFFSETOF(chipcregs_t, gci_inbandeventintmask),
				((1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
				|(1 << GCI_BITOFFSET(GCI_LTE_RX_POS))
				|(1 << GCI_BITOFFSET(GCI_LTE_TX_POS))),
				((0 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
				|(0 << GCI_BITOFFSET(GCI_LTE_RX_POS))
				|(0 << GCI_BITOFFSET(GCI_LTE_TX_POS))));

			/* Enable Inband interrupt for Aux Valid bit */
			si_gci_indirect(wlc_hw->sih,
				GCI_REGIDX(GCI_LTE_AUXRXDVALID_POS),
				OFFSETOF(chipcregs_t, gci_inbandeventintmask),
				(1 << GCI_BITOFFSET(GCI_LTE_AUXRXDVALID_POS)),
				(1 << GCI_BITOFFSET(GCI_LTE_AUXRXDVALID_POS)));
			ltecx_state |= (1 << C_LTECX_ST_CRTI_DEBUG_MODE_TMP);
		} else {
			/* Enable inbandIntMask for FrmSync Only; disable LTE_Rx and LTE_Tx
			  * Note: FrameSync, LTE Rx & LTE Tx happen to share the same REGIDX
			  * Hence a single Access is sufficient
			  */
			si_gci_indirect(wlc_hw->sih,
				GCI_REGIDX(GCI_LTE_FRAMESYNC_POS),
				OFFSETOF(chipcregs_t, gci_inbandeventintmask),
				((1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
				|(1 << GCI_BITOFFSET(GCI_LTE_RX_POS))
				|(1 << GCI_BITOFFSET(GCI_LTE_TX_POS))),
				((1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
				|(0 << GCI_BITOFFSET(GCI_LTE_RX_POS))
				|(0 << GCI_BITOFFSET(GCI_LTE_TX_POS))));
			/* Disable Inband interrupt for Aux Valid bit */
			si_gci_indirect(wlc_hw->sih,
				GCI_REGIDX(GCI_LTE_AUXRXDVALID_POS),
				OFFSETOF(chipcregs_t, gci_inbandeventintmask),
				(1 << GCI_BITOFFSET(GCI_LTE_AUXRXDVALID_POS)),
				(0 << GCI_BITOFFSET(GCI_LTE_AUXRXDVALID_POS)));
			ltecx_state &= ~(1 << C_LTECX_ST_CRTI_DEBUG_MODE_TMP);
		}
		wlc_bmac_write_shm(wlc_hw,
			ltecx->ltecx_shm_addr + M_LTECX_STATE, ltecx_state);
	}
}

static void
wlc_ltecx_handle_joinproc(void *ctx, bss_assoc_state_data_t *evt_data)
{
	wlc_ltecx_info_t *ltecx = (wlc_ltecx_info_t *)ctx;

	if (evt_data) {
		if (evt_data->state == AS_JOIN_INIT) {
			wlc_ltecx_assoc_in_prog(ltecx, TRUE);
		} else if (evt_data->state == AS_JOIN_ADOPT) {
			wlc_ltecx_assoc_in_prog(ltecx, FALSE);
		} else if (evt_data->state == AS_IDLE) {
			wlc_ltecx_assoc_in_prog(ltecx, FALSE);
		}
	}
}

static void
wlc_ltecx_assoc_in_prog(wlc_ltecx_info_t *ltecx, int val)
{
	uint16 ltecxHostFlag;
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;
	if (!wlc_hw || !si_iscoreup(wlc_hw->sih)) {
		return;
	}
	if (CHSPEC_IS2G(wlc->chanspec) && (ltecx->ltecx_shm_addr)) {
		ltecxHostFlag = wlc_bmac_read_shm(wlc_hw, ltecx->ltecx_shm_addr
			+ M_LTECX_HOST_FLAGS);
		if (val == TRUE) {
			ltecxHostFlag |= (1 << C_LTECX_HOST_ASSOC_PROG);
		} else {
			ltecxHostFlag &= (~(1 << C_LTECX_HOST_ASSOC_PROG));
		}
		wlc_bmac_write_shm(wlc_hw, ltecx->ltecx_shm_addr
			+ M_LTECX_HOST_FLAGS, ltecxHostFlag);
	}
}

#ifdef WLRSDB
void
wlc_ltecx_update_coex_iomask(wlc_ltecx_info_t *ltecx)
{
#if defined(WLC_LOW)
	wlc_info_t *wlc = ltecx->wlc;
	wlc_hw_info_t *wlc_hw = ltecx->wlc->hw;

	/* Should come here only for RSDB capable devices */
	ASSERT(wlc_bmac_rsdb_cap(wlc_hw));

	if (!RSDB_ENAB(wlc->pub) ||
		(wlc_rsdb_mode(wlc) == PHYMODE_MIMO) ||
		(wlc_rsdb_mode(wlc) == PHYMODE_80P80)) {
		/* In the MIMO/80p80 set coex_io_mask to 0x3- on core 1
		 * i.e. mask both WlPrio and WlTxOn on Core 1.
		 * Unmask coex_io_mask on core 0 (0x0-)
		 */
		if (si_coreunit(wlc_hw->sih) == 0) {
			d11regs_t *sregs;

			/* Unmask coex_io_mask on core0 to 0x0- */
			AND_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.coex_io_mask,
				~((1 << COEX_IOMASK_WLPRIO_POS) | (1 << COEX_IOMASK_WLTXON_POS)));

			/* Set: core 1 */
			sregs = si_d11_switch_addrbase(wlc_hw->sih, 1);

			/* Enable MAC control IHR access on core 1 */
			OR_REG(wlc_hw->osh, &sregs->maccontrol, MCTL_IHR_EN);

			/* Mask WlPrio and WlTxOn on core 1 */
			OR_REG(wlc_hw->osh, &sregs->u.d11regs.coex_io_mask,
				((1 << COEX_IOMASK_WLPRIO_POS) | (1 << COEX_IOMASK_WLTXON_POS)));

			/* Restore: core 0 */
			si_d11_switch_addrbase(wlc_hw->sih, 0);
		}
	} else {
		wlc_cmn_info_t* wlc_cmn = ltecx->wlc->cmn;
		wlc_info_t *other_wlc;
		int idx;
		int coex_io_mask, coex_io_mask_ch;

		/* read current core's iomask */
		coex_io_mask = R_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.coex_io_mask);

		/* By default, unmask WlPrio and WlTxOn for present core */
		coex_io_mask_ch = coex_io_mask &
			~((1 << COEX_IOMASK_WLPRIO_POS)
			| (1 << COEX_IOMASK_WLTXON_POS));
		FOREACH_WLC(wlc_cmn, idx, other_wlc) {
			if (wlc != other_wlc) {
				if (CHSPEC_IS5G(wlc_hw->chanspec) &&
					CHSPEC_IS2G(other_wlc->hw->chanspec)) {
					/* mask WlPrio and WlTxOn for present core */
					coex_io_mask_ch = coex_io_mask |
						((1 << COEX_IOMASK_WLPRIO_POS)
						|(1 << COEX_IOMASK_WLTXON_POS));
				}
			}
		}

		/* update coex_io_mask if there is a change */
		if (coex_io_mask_ch != coex_io_mask) {
			W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.coex_io_mask, coex_io_mask_ch);
		}
	}
#endif /* WLC_LOW */
}
#endif /* WLRSDB */

#endif /* BCMLTECOEX */
