/*
 * WLC RSDB API definition
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
 * $Id: wlc_rsdb.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_rsdb_h_
#define _wlc_rsdb_h_
#ifdef WLRSDB
enum wlc_rsdb_modes {
	WLC_RSDB_MODE_AUTO = AUTO,
	WLC_RSDB_MODE_2X2,
	WLC_RSDB_MODE_RSDB,
	WLC_RSDB_MODE_80P80,
	WLC_RSDB_MODE_MAX
};

#define WLC_RSDB_DUAL_MAC_MODE(mode)	((mode) == WLC_RSDB_MODE_RSDB)
#define WLC_RSDB_SINGLE_MAC_MODE(mode)	(((mode) == WLC_RSDB_MODE_2X2) ||	\
	((mode) == WLC_RSDB_MODE_80P80))
#define WLC_RSDB_GET_PRIMARY_WLC(wlc)	((wlc)->cmn->wlc[0])

#define SCB_MOVE /* Added flag temporarily to enable scb move during bsscfg clone */
#define WLC_RSDB_CURR_MODE(wlc) WLC_RSDB_EXTRACT_MODE((wlc)->cmn->rsdb_mode)
extern int wlc_rsdb_assoc_mode_change(wlc_bsscfg_t **cfg, wlc_bss_info_t *bi);
extern int wlc_rsdb_change_mode(wlc_info_t *wlc, int8 to_mode);
#ifdef WL_MODESW
extern uint8 wlc_rsdb_downgrade_wlc(wlc_info_t *wlc);
extern uint8 wlc_rsdb_upgrade_wlc(wlc_info_t *wlc);
#endif // endif
int wlc_rsdb_get_wlcs(wlc_info_t *wlc, wlc_info_t **wlc_2g, wlc_info_t **wlc_5g);
wlc_info_t * wlc_rsdb_get_other_wlc(wlc_info_t *wlc);
wlc_info_t* wlc_rsdb_find_wlc_for_chanspec(wlc_info_t *wlc, chanspec_t chanspec);
wlc_bsscfg_t* wlc_rsdb_cfg_for_chanspec(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chanspec);
void wlc_rsdb_update_wlcif(wlc_info_t *wlc, wlc_bsscfg_t *from, wlc_bsscfg_t *to);
int wlc_rsdb_join_prep_wlc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *SSID, int len,
	wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len);
wlc_bsscfg_t*
wlc_rsdb_bsscfg_clone(wlc_info_t *from_wlc, wlc_info_t *to_wlc, wlc_bsscfg_t *cfg, int *ret);
int wlc_rsdb_attach(wlc_info_t* wlc);
void wlc_rsdb_detach(wlc_info_t* wlc);
bool wlc_rsdb_update_active(wlc_info_t *wlc, bool *old_state);
extern uint16 wlc_rsdb_mode(void *hdl);
bool wlc_rsdb_chkiovar(const bcm_iovar_t  *vi_ptr, uint32 actid, int32 wlc_indx);
#else /* WLRSDB */
#define wlc_rsdb_mode(hdl) (PHYMODE_MIMO)
#endif /* WLRSDB */

#ifdef RSDB_DFS_SCAN
extern int wlc_rsdb_dfs_scan_prep(wlc_info_t * wlc);
#endif // endif

#if defined(WLRSDB) && !defined(WL_DUALNIC_RSDB)
void wlc_rsdb_bmc_smac_template(void *wlc, int tplbuf, uint32 bufsize);
extern void wlc_rsdb_set_phymode(void *hdl, uint32 phymode);
#else
#define wlc_rsdb_bmc_smac_template(hdl, tplbuf, bufsize)  do {} while (0)
#define wlc_rsdb_set_phymode(a, b) do {} while (0)
#endif /* defined(WLRSDB) && !defined(WL_DUALNIC_RSDB) */
#endif /* _wlc_rsdb_h_ */
