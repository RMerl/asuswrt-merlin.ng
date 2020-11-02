/*
 * WLC RSDB API Implementation
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
 * $Id: wlc_rsdb.c 768542 2018-10-17 09:25:19Z $
 */

/**
 * @file
 * @brief
 * Real Simultaneous Dual Band operation
 */

/**
 * @file
 * @brief
 * XXX Twiki: [RealSimultaneousDualBand]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

#ifdef WLRSDB

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <wl_dbg.h>
#include <wlc_types.h>
#include <siutils.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_rsdb.h>
#include <wl_export.h>
#include <wlc_scan.h>
#include <wlc_assoc.h>
#include <wlc_rrm.h>
#include <wlc_stf.h>
#include <wlc_bmac.h>
#include <phy_api.h>
#include <wlc_scb.h>
#include <wlc_vht.h>
#include <wlc_ht.h>
#include <wlc_bmac.h>
#include <wlc_scb_ratesel.h>
#include <wlc_txc.h>
#include <wlc_txbf.h>
#ifdef WL_MODESW
#include <wlc_modesw.h>
#endif // endif
#include <wlc_dfs.h>

#ifdef BCMDBG
#define WLRSDB_DBG(x) printf x
#else
#define WLRSDB_DBG(x)
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST)
static int wlc_rsdb_dump(void *wlc, struct bcmstrbuf *b);
#endif // endif

#ifdef WL_MODESW
void wlc_rsdb_opmode_change_cb(void *ctx, wlc_modesw_notif_cb_data_t *notif_data);
#endif // endif

static int swmode2phymode[] = {PHYMODE_MIMO, PHYMODE_RSDB, PHYMODE_80P80, -1};
#define SWMODE2PHYMODE(x) swmode2phymode[(x)]
#define PHYMODE2SWMODE(x) phymode2swmode(x)
#define UNIT(ptr)	((ptr)->pub->unit)

/* iovar table */
enum {
	IOV_RSDB_MODE = 0,		/* Possible Values: -1(auto), 0(2x2), 1(rsdb), 2(80p80) */
	IOV_RSDB_LAST
};

static const bcm_iovar_t rsdb_iovars[] = {
	{"rsdb_mode", IOV_RSDB_MODE, 0, IOVT_BUFFER, sizeof(wl_config_t)},
	{NULL, 0, 0, 0, 0}
};

#ifdef WLRSDB_DVT
static const char *rsdb_gbl_iovars[] = {"mpc", "apsta", "country", "event_msgs"};
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_ASSERT) || defined(BCMDBGASSERT_LOG) || \
	(defined(BCMDBG_TRAP) && !defined(DONGLEBUILD))
static int phymode2swmode(int16 mode)
{
	uint8 i = 0;
	while (swmode2phymode[i] != -1) {
		if (swmode2phymode[i] == mode)
			return i;
		i++;
	}
	return -1;
}
#endif // endif

int
wlc_rsdb_assoc_mode_change(wlc_bsscfg_t **pcfg, wlc_bss_info_t *bi)
{
	wlc_bsscfg_t *cfg = *pcfg;
	wlc_info_t *wlc = cfg->wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_info_t *to_wlc = NULL;
	wlc_info_t *from_wlc = wlc;
	wlc_info_t *other_wlc;
	to_wlc = wlc_rsdb_find_wlc_for_chanspec(from_wlc, bi->chanspec);
	other_wlc = wlc_rsdb_get_other_wlc(to_wlc);
	if (as->state != AS_MODE_SWITCH_FAILED) {
		/* Check if need to switch to MIMO mode.
		 * This is done if we are connecting in
		 * primary WLC and secondary WLC has
		 * no active connection.
		 */
		if ((to_wlc == to_wlc->cmn->wlc[0]) &&
			(!other_wlc->stas_connected)) {
			/* Check if we need to MOVE the cfg across wlc */
			if (to_wlc != from_wlc) {
				cfg =  wlc_rsdb_cfg_for_chanspec(from_wlc,
					cfg, bi->chanspec);
				wlc = to_wlc;
				ASSERT(wlc == cfg->wlc);
				as = cfg->assoc;
				wlc->as_info->assoc_req[0] = cfg;
				wlc_rsdb_join_prep_wlc(wlc, cfg, cfg->SSID,
					cfg->SSID_len, cfg->scan_params,
					cfg->assoc_params,
					cfg->assoc_params_len);
				/* XXX sync h/w and s/w for old wlc
				 * to avoid wake_ok assert.
				 */
				wlc_set_wake_ctrl(from_wlc);
			}
#ifdef WL_MODESW
			/* Switch to MIMO */
			if (WLC_MODESW_ENAB(wlc->pub)) {
				if (wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK) {
					if (WLC_RSDB_DUAL_MAC_MODE(WLC_RSDB_CURR_MODE(wlc)) &&
						wlc_bss_get_mimo_cap(bi)) {
						WL_MODE_SWITCH(("wl%d.%d: Wait for the mode"
							"switch %p bi_index = %d\n",
							WLCWLUNIT(cfg->wlc), cfg->_idx,
							cfg,
							wlc->as_info->join_targets_last));
						/* Increment the BI index back to original so
						 * that it can be used while re-entering wlc_join
						 * attempt function after completing mode switch.
							 */
						wl_add_timer(wlc->wl, as->timer,
							MSW_MODE_SWITCH_TIMEOUT, FALSE);
						wlc_assoc_change_state(cfg,
							AS_MODE_SWITCH_START);
						if (WLC_RSDB_SINGLE_MAC_MODE(
							WLC_RSDB_CURR_MODE(wlc))) {
							wlc_assoc_change_state(cfg, AS_SCAN);
						}
						else {
							wlc->as_info->join_targets_last++;
							return BCME_NOTREADY;
						}
					}
				}
			}
#endif /* WL_MODESW */
		}
		else {
			WL_ASSOC(("RSDB:wlc mismatch,making join attempt to"
				"wl%d.%d from wl%d.%d\n",
				to_wlc->pub->unit, cfg->_idx, wlc->pub->unit, cfg->_idx));
			/* Make sure that all the active BSSCFG's in the from_wlc
			 * are downgraded to 1x1 80Mhz operation in case they were
			 * operating at 2x2 or 80p80 mode.
			 */
#ifdef WL_MODESW
			 /* Switch to RSDB */
			if (WLC_MODESW_ENAB(wlc->pub)) {
				if ((wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK) &&
					(!WLC_RSDB_DUAL_MAC_MODE(WLC_RSDB_CURR_MODE(wlc))) &&
					(!MCHAN_ACTIVE(wlc->pub))) {
					WL_MODE_SWITCH(("wl%d.%d: Wait for the mode"
						"switch %p bi_index = %d\n",
						WLCWLUNIT(cfg->wlc), cfg->_idx,
						cfg,
						wlc->as_info->join_targets_last));
					/* Increment the BI index back to original so
					 * that it can be used while re-entering wlc_join
					 * attempt function after completing mode switch.
					 */
					wl_add_timer(wlc->wl, as->timer,
						MSW_MODE_SWITCH_TIMEOUT, FALSE);
					wlc_assoc_change_state(cfg,
						AS_MODE_SWITCH_START);

					if (WLC_RSDB_DUAL_MAC_MODE(WLC_RSDB_CURR_MODE(wlc))) {
						wlc_assoc_change_state(cfg, AS_SCAN);
					}
					else {
						wlc->as_info->join_targets_last++;
						return BCME_NOTREADY;
					}
				}
			}
#endif /* WL_MODESW */
			if (to_wlc != from_wlc) {
				cfg =  wlc_rsdb_cfg_for_chanspec(from_wlc,
					cfg, bi->chanspec);
				wlc = to_wlc;
				ASSERT(wlc == cfg->wlc);
				as = cfg->assoc;
				wlc->as_info->assoc_req[0] = cfg;
				wlc_rsdb_join_prep_wlc(wlc, cfg, cfg->SSID,
					cfg->SSID_len, cfg->scan_params,
					cfg->assoc_params,
					cfg->assoc_params_len);
				/* XXX sync h/w and s/w for old wlc
				 * to avoid wake_ok assert.
				 */
				wlc_set_wake_ctrl(from_wlc);
			}
		}
	}
	*pcfg = cfg;
	return BCME_OK;
}

#ifdef WL_MODESW
uint8
wlc_rsdb_upgrade_wlc(wlc_info_t *wlc)
{
	int idx = 0;
	wlc_bsscfg_t *bsscfg;
	bool mode_switch_sched = FALSE, is160_8080 = FALSE;
	uint8 new_oper_mode, curr_oper_mode, bw, nss;
	uint8 chip_mode = WLC_RSDB_CURR_MODE(wlc);

	if ((wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK) &&
		(chip_mode == WLC_RSDB_MODE_2X2)) {
		WL_MODE_SWITCH(("wl%d: Not possible to upgrade... mode = %x\n",
			WLCWLUNIT(wlc), chip_mode));
	    return BCME_OK;
	}

	FOREACH_BSS(wlc, idx, bsscfg) {
		if (WLC_BSS_CONNECTED(bsscfg)) {
			/* If associated, then the Opmode is derived from the chanspec */
			curr_oper_mode = wlc_modesw_derive_opermode(wlc->modesw,
				bsscfg->current_bss->chanspec, bsscfg, wlc->stf->op_rxstreams);
			bw = DOT11_OPER_MODE_CHANNEL_WIDTH(curr_oper_mode);
			nss = DOT11_OPER_MODE_RXNSS(curr_oper_mode);
			is160_8080 = DOT11_OPER_MODE_160_8080(curr_oper_mode);

			if ((nss == 1) && (wlc->stf->hw_rxchain_cap > 1))	{
				/* Use the Rx max streams for both Rx and Tx */
				new_oper_mode = DOT11_D8_OPER_MODE(0,
				WLC_BITSCNT(wlc->stf->hw_rxchain_cap), 0, is160_8080, bw);
			}
			else {
				new_oper_mode = DOT11_D8_OPER_MODE(0, 3, 0, is160_8080,
					DOT11_OPER_MODE_80MHZ);
			}
			wlc_modesw_handle_oper_mode_notif_request(wlc->modesw, bsscfg,
				new_oper_mode, TRUE, 0);
			mode_switch_sched = TRUE;
		}
		else {
			/* Use the Rx max streams for both Rx and Tx */
			new_oper_mode = DOT11_D8_OPER_MODE(0,
				WLC_BITSCNT(wlc->stf->hw_rxchain_cap), 0, is160_8080,
				DOT11_OPER_MODE_80MHZ);
			wlc_modesw_handle_oper_mode_notif_request(wlc->modesw, bsscfg,
				new_oper_mode, TRUE, 0);
		}
	}
	if (!mode_switch_sched) {
		WL_MODE_SWITCH(("wl%d: No Operating mode triggered\n",
			WLCWLUNIT(wlc)));
		wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_2X2);
		wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_AUTO);
	}
	return BCME_OK;
}

uint8
wlc_rsdb_downgrade_wlc(wlc_info_t *wlc)
{
	int idx = 0;
	wlc_bsscfg_t *bsscfg;
	uint8 new_oper_mode, curr_oper_mode, bw, nss;
	uint8 chip_mode = WLC_RSDB_CURR_MODE(wlc);
	bool mode_switch_sched = FALSE, is160_8080 = FALSE;

	if ((wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK) &&
		(chip_mode == WLC_RSDB_MODE_RSDB)) {
		WL_MODE_SWITCH(("wl%d: Not possible to Downgrade... mode = %x\n",
			WLCWLUNIT(wlc), chip_mode));
		return BCME_OK;
	}

	FOREACH_BSS(wlc, idx, bsscfg)
	{
		if (WLC_BSS_CONNECTED(bsscfg)) {
			curr_oper_mode = wlc_modesw_derive_opermode(wlc->modesw,
				bsscfg->current_bss->chanspec, bsscfg, wlc->stf->op_rxstreams);
			bw = DOT11_OPER_MODE_CHANNEL_WIDTH(curr_oper_mode);
			nss = DOT11_OPER_MODE_RXNSS(curr_oper_mode);
			is160_8080 = DOT11_OPER_MODE_160_8080(curr_oper_mode);
			if (nss != 1)	{
				new_oper_mode = DOT11_D8_OPER_MODE(0, 1, 0, is160_8080, bw);
			}
			else {
				new_oper_mode = DOT11_D8_OPER_MODE(0, 1, 0, is160_8080,
						DOT11_OPER_MODE_80MHZ);
			}
			wlc_modesw_handle_oper_mode_notif_request(wlc->modesw, bsscfg,
				new_oper_mode, TRUE, 0);
			mode_switch_sched = TRUE;
		}
		else {
			new_oper_mode = DOT11_D8_OPER_MODE(0, 1, 0, is160_8080,
					DOT11_OPER_MODE_80MHZ);
			wlc_modesw_handle_oper_mode_notif_request(wlc->modesw, bsscfg,
				new_oper_mode, TRUE, 0);
		}
	}
	if (!mode_switch_sched) {
		wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_RSDB);
		wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_AUTO);
	}
	return BCME_OK;
}
#endif /* WL_MODESW */

int
wlc_rsdb_change_mode(wlc_info_t *wlc, int8 to_mode)
{
	int err = BCME_OK;
	int idx;
	wlc_cmn_info_t *wlc_cmn;
	wlc_info_t *wlc_iter;

	/* Make sure sw mode and phy mode are in sync */
	ASSERT(PHYMODE2SWMODE(phy_get_phymode((phy_info_t *)WLC_PI(wlc)))
		== WLC_RSDB_CURR_MODE(wlc));

	/* Check if the current mode and the new mode are the same */
	if (to_mode == wlc->cmn->rsdb_mode)
		return err;

	/* Check if the current mode is auto and the new mode is also auto */
	if ((to_mode == WLC_RSDB_MODE_AUTO) && ((wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK)
		== WLC_RSDB_MODE_AUTO_MASK)) {
		return err;
	}

	/* Check if the current mode is auto + ZZZ and the new mode is ZZZ */
	if ((wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK) &&
		(WLC_RSDB_CURR_MODE(wlc) == to_mode)) {
		wlc->cmn->rsdb_mode = to_mode;
		return err;
	}

	/* TODO: Check if mode can be changed */

	if (to_mode == WLC_RSDB_MODE_AUTO) {
		wlc->cmn->rsdb_mode |= WLC_RSDB_MODE_AUTO_MASK;
	} else {
		wlc->cmn->rsdb_mode = to_mode;
	}

	/* In Automode, currently we are not changing the actual mode. So, no need to update
	   anything. If any actual mode change happens, the following if condition need to be
	   re-looked at
	 */
	if (to_mode != WLC_RSDB_MODE_AUTO) {
		wlc_cmn = wlc->cmn;
		if (WLC_RSDB_EXTRACT_MODE(to_mode) == WLC_RSDB_MODE_RSDB) {
			/* For RSDB case, apply changes on all wlcs
			 */
			FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
				phy_set_phymode((phy_info_t *)WLC_PI(wlc_iter),
					(uint16)SWMODE2PHYMODE(WLC_RSDB_EXTRACT_MODE(to_mode)));
				wlc_stf_phy_chain_calc_set(wlc_iter);
			}
		} else {
			/* For non RSDB cases, apply changes only on wlc0
			 */
			/* TODO: In case a connection in wlc[1] needs to upgrade to 2x2 or 80p80,
			   then move bsscfg first, and call upgrade in wlc[0] context
			 */
			phy_set_phymode((phy_info_t *)WLC_PI(wlc_cmn->wlc[0]),
				(uint16)SWMODE2PHYMODE(WLC_RSDB_EXTRACT_MODE(to_mode)));
			wlc_stf_phy_chain_calc_set(wlc_cmn->wlc[0]);
		}
	}

	/* TODO : Do further mode switch related calls */

	/* TODO: Update rsdb_mode, in case of AUTO */
	return err;
}

static int
wlc_rsdb_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;
	int32 int_val = 0;
	int err = 0;

	switch (actionid) {
		case IOV_GVAL(IOV_RSDB_MODE): {
			wl_config_t cfg;

			cfg.config = (wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK) ? AUTO : 0;
			cfg.status = WLC_RSDB_CURR_MODE(wlc);
			memcpy(a, &cfg, sizeof(wl_config_t));
			break;
		}

		case IOV_SVAL(IOV_RSDB_MODE): {
			wl_config_t cfg;

			if (plen >= (int)sizeof(int_val)) {
				bcopy(p, &cfg, sizeof(wl_config_t));
				int_val = cfg.config;
			} else {
				err = BCME_BADARG;
				break;
			}

			if ((int_val < WLC_RSDB_MODE_AUTO) || (int_val >= WLC_RSDB_MODE_MAX)) {
				err = BCME_BADARG;
				break;
			}
			err = wlc_rsdb_change_mode(wlc, int_val);
			break;
		}

		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST)
static int
wlc_rsdb_dump(void *w, struct bcmstrbuf *b)
{
	wlc_info_t *wlc_iter;
	wlc_bsscfg_t *bsscfg;
	wlc_info_t *wlc = (wlc_info_t*)w;
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;

	int idx = 0, i;
	uint8 active = 0;
	int mode = wlc->cmn->rsdb_mode;
	char *automode = "";

	bcm_bprintf(b, "RSDB active:%d\n", RSDB_ACTIVE(wlc->pub));
	automode = (mode & WLC_RSDB_MODE_AUTO_MASK) ? "RSDB Mode: Auto, " : "";
	bcm_bprintf(b, "%sCurrent RSDB Mode : %d\n", automode, WLC_RSDB_EXTRACT_MODE(mode));
	bcm_bprintf(b, "RSDB PHY mode:%d\n", phy_get_phymode((phy_info_t *)WLC_PI(wlc)));
	bcm_bprintf(b, "RSDB HW mode:0x%x\n",
		(si_core_cflags(wlc->pub->sih, 0, 0) & SICF_PHYMODE) >> SICF_PHYMODE_SHIFT);

	bcm_bprintf(b, "active WLCs:");
	FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
		if (wlc_iter->pub->associated) {
			bcm_bprintf(b, "wlc[%d] ", idx);
			active++;
		}
	}
	if (active)
		bcm_bprintf(b, "(%d)\n", active);
	else
		bcm_bprintf(b, "none\n");

	FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
		bcm_bprintf(b, "wlc:%d\n", idx);
		FOREACH_BSS(wlc_iter, i, bsscfg) {
			char ifname[32];
			char perm[32];
			char ssidbuf[SSID_FMT_BUF_LEN];

			bcm_bprintf(b, "\n");
			wlc_format_ssid(ssidbuf, bsscfg->SSID, bsscfg->SSID_len);
			strncpy(ifname, wl_ifname(wlc_iter->wl, bsscfg->wlcif->wlif),
				sizeof(ifname));
			ifname[sizeof(ifname) - 1] = '\0';
			bcm_bprintf(b, "BSSCFG %d: \"%s\"\n", WLC_BSSCFG_IDX(bsscfg), ssidbuf);

			bcm_bprintf(b, "%s enable %d up %d \"%s\"\n",
				BSSCFG_AP(bsscfg)?"AP":"STA",
				bsscfg->enable, bsscfg->up, ifname);
			bcm_bprintf(b, "current_bss->BSSID %s\n",
				bcm_ether_ntoa(&bsscfg->current_bss->BSSID, (char*)perm));

			wlc_format_ssid(ssidbuf, bsscfg->current_bss->SSID,
				bsscfg->current_bss->SSID_len);
			bcm_bprintf(b, "current_bss->SSID \"%s\"\n", ssidbuf);
#ifdef STA
			if (BSSCFG_STA(bsscfg))
				bcm_bprintf(b, "assoc_state %d\n", bsscfg->assoc->state);

#endif /* STA */
		} /* FOREACH_BSS */
		bcm_bprintf(b, "\n");

#ifdef STA
		bcm_bprintf(b, "AS_IN_PROGRESS() %d stas_associated %d\n", AS_IN_PROGRESS(wlc_iter),
			wlc_iter->stas_associated);
#endif /* STA */

		bcm_bprintf(b, "aps_associated %d\n", wlc_iter->aps_associated);
	} /* FOREACH_WLC */
	return 0;
}
#endif /* #if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined (WLTEST) */

#ifdef WL_MODESW
void
wlc_rsdb_opmode_change_cb(void *ctx, wlc_modesw_notif_cb_data_t *notif_data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_bsscfg_t *icfg = NULL, *join_pend_cfg = NULL;
	uint8 new_rxnss, old_rxnss;
	int idx;
	ASSERT(wlc);
	ASSERT(notif_data);
	WL_MODE_SWITCH(("MODESW Callback status = %d oper_mode = %x signal = %d\n",
		notif_data->status, notif_data->opmode, notif_data->signal));
	bsscfg = notif_data->cfg;
	switch (notif_data->signal) {
		case MODESW_DN_AP_COMPLETE:
		case MODESW_DN_STA_COMPLETE:
		case MODESW_UP_STA_COMPLETE:
		{
			/* Assoc state machine transition */
			if (RSDB_ENAB(wlc->pub) &&
				(wlc->cmn->rsdb_mode & WLC_RSDB_MODE_AUTO_MASK)) {
				FOREACH_BSS(wlc, idx, icfg) {
					if (icfg->assoc->state == AS_MODE_SWITCH_START) {
						join_pend_cfg = icfg;
					}
				}
				if (join_pend_cfg) {
					wlc_assoc_change_state(join_pend_cfg,
						AS_MODE_SWITCH_COMPLETE);
				}
			}
		}
		break;
		case MODESW_PHY_DN_COMPLETE:
			WL_MODE_SWITCH(("wl%d: Change chip mode to RSDB(%d) by cfg = %d\n",
				WLCWLUNIT(wlc), WLC_RSDB_MODE_RSDB, bsscfg->_idx));
			wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_RSDB);
			wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_AUTO);
		break;
		case MODESW_PHY_UP_START:
			old_rxnss = DOT11_OPER_MODE_RXNSS(bsscfg->oper_mode);
			new_rxnss = DOT11_OPER_MODE_RXNSS(notif_data->opmode);
			if (old_rxnss != new_rxnss) {
				WL_MODE_SWITCH(("wl%d: Change chip mode to MIMO(%d) by cfg = %d\n",
					WLCWLUNIT(wlc), WLC_RSDB_MODE_2X2, bsscfg->_idx));
				wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_2X2);
			} else {
				WL_MODE_SWITCH(("wl%d: Change chip mode to 80p80(%d) by cfg = %d\n",
					WLCWLUNIT(wlc), WLC_RSDB_MODE_80P80, bsscfg->_idx));
				wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_80P80);
			}
			wlc_rsdb_change_mode(wlc, WLC_RSDB_MODE_AUTO);
		break;
	}

#if defined(RSDB_DFS_SCAN) && defined(WLDFS)
	wlc_dfs_handle_modeswitch(wlc->dfs, notif_data->signal);
#endif // endif
	return;
}
#endif /* WL_MODESW */

int
BCMATTACHFN(wlc_rsdb_attach)(wlc_info_t* wlc)
{
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST)
	if (wlc_dump_register(wlc->pub, "rsdb", wlc_rsdb_dump, (void*)wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
	}
#endif // endif
	if (wlc_module_register(wlc->pub, rsdb_iovars, "rsdb", wlc, wlc_rsdb_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: stf wlc_rsdb_iovar_attach failed\n", wlc->pub->unit));
		return -1;
	}
#ifdef WL_MODESW
		if (wlc->modesw) {
			wlc_modesw_notif_cb_register(wlc->modesw, wlc_rsdb_opmode_change_cb, wlc);
		}
#endif /* WL_MODESW */
	return 0;
}

void
BCMATTACHFN(wlc_rsdb_detach)(wlc_info_t* wlc)
{
#ifdef WL_MODESW
		if (wlc->modesw) {
			wlc_modesw_notif_cb_unregister(wlc->modesw, wlc_rsdb_opmode_change_cb,
				wlc->modesw);
		}
#endif /* WL_MODESW */
	wlc_module_unregister(wlc->pub, "rsdb", wlc);
}

/* API to get both wlc pointers given single wlc.
 * API return wlc_2g and wlc_5g assuming preferred
 * band for that wlc
 */
int
wlc_rsdb_get_wlcs(wlc_info_t *wlc, wlc_info_t **wlc_2g, wlc_info_t **wlc_5g)
{
	wlc_cmn_info_t* wlc_cmn;
	wlc_info_t *wlc_iter;
	int idx;
	int chspec;

	WLRSDB_DBG(("wl%d:%s:%d\n", wlc->pub->unit, __FUNCTION__, __LINE__));
	wlc_cmn = wlc->cmn;
	/* Simple Init */
	*wlc_2g = wlc_cmn->wlc[0];
	*wlc_5g = wlc_cmn->wlc[1];

#ifdef RSDB_BAND_AFFINITY
	BCM_REFERENCE(chspec);
	/* overwrite based on band affinity */
	FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
		if (wlc_iter->rsdb_band_affinity == BAND_2G_INDEX) {
			*wlc_2g = wlc_iter;
		} else {
			*wlc_5g = wlc_iter;
			ASSERT(wlc->rsdb_band_affinity == BAND_5G_INDEX);
		}
	}
#else
	BCM_REFERENCE(idx);
	BCM_REFERENCE(wlc_iter);
	/* overwrite based on current association */
	if (wlc_iovar_op(wlc, "chanspec",
		NULL, 0, &chspec, sizeof(int), IOV_GET, NULL) == BCME_OK) {
		if (CHSPEC_IS5G(chspec)) {
			*wlc_2g = wlc_rsdb_get_other_wlc((*wlc_5g = wlc));
		}
		else {
			*wlc_5g = wlc_rsdb_get_other_wlc((*wlc_2g = wlc));
		}
	}
#endif /* RSDB_BAND_AFFINITY */
	ASSERT(wlc_2g != wlc_5g && wlc_2g && wlc_5g);

	return BCME_OK;
}

/* API to get the other wlc given one wlc */
wlc_info_t *
wlc_rsdb_get_other_wlc(wlc_info_t *wlc)
{
	wlc_cmn_info_t* wlc_cmn;
	wlc_info_t *to_wlc;
	int idx;

	WLRSDB_DBG(("wl%d:%s:%d\n", wlc->pub->unit, __FUNCTION__, __LINE__));
	wlc_cmn = wlc->cmn;
	FOREACH_WLC(wlc_cmn, idx, to_wlc) {
		if (wlc != to_wlc)
			return to_wlc;
	}
	return NULL;
}

/* Finds the WLC for RSDB usage based on chanspec
 *	Return any free / un-associated WLC. Give preference to the incoming wlc to
 * avoid un-necessary movement. Return associated wlc if oparating chanspec
 * matches.
 */
wlc_info_t*
wlc_rsdb_find_wlc_for_chanspec(wlc_info_t *wlc, chanspec_t chanspec)
{
	int idx;
	wlc_info_t* iwlc = NULL;
	wlc_info_t* to_wlc = NULL;
	int wlc_busy = 0;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	WLRSDB_DBG(("%s: Incoming wlc:%d Request wlc For chspec:%s \n", __FUNCTION__,
		wlc->pub->unit, wf_chspec_ntoa_ex(chanspec, chanbuf)));

	FOREACH_WLC(wlc->cmn, idx, iwlc) {
		wlc_busy = 0;
		if (iwlc->pub->associated) {
			int bss_idx;
			wlc_bsscfg_t *cfg;
			FOREACH_AS_BSS(iwlc, bss_idx, cfg) {
				if (cfg->assoc->state != AS_SCAN) { /* cfg not in Roam */
					wlc_busy = 1;
					if (CHSPEC_BAND(cfg->current_bss->chanspec)
						== CHSPEC_BAND(chanspec)) {
						/* Band matched with as'd wlc. MCHAN. */
						WLRSDB_DBG(("Return MCNX wlc:%d For chspec:%s\n",
						iwlc->pub->unit,
						wf_chspec_ntoa_ex(chanspec, chanbuf)));
						return iwlc;
					}
				}
			} /* FOREACH_AS_BSS */
		}
		if (!wlc_busy) {
			if (to_wlc == NULL)
				to_wlc = iwlc;
			else if (wlc == iwlc)
				/* Return the incoming wlc if no other wlc has active connection
				 * on the requested chanspec band.
				 */
				to_wlc = iwlc;
		}
	} /* FOREACH_WLC */
	WLRSDB_DBG(("Return wlc:%d For chspec:%s\n", to_wlc->pub->unit,
		wf_chspec_ntoa_ex(chanspec, chanbuf)));

	return to_wlc;
}

/* RSDB bsscfg allocation based on WLC band allocation
 * For RSDB, this involves creating a new cfg in the target WLC
 * Typically, called from JOIN/ROAM-attempt, AP-up
*/
wlc_bsscfg_t* wlc_rsdb_cfg_for_chanspec(wlc_info_t *wlc, wlc_bsscfg_t *cfg, chanspec_t chanspec)
{
	wlc_info_t *to_wlc = wlc_rsdb_find_wlc_for_chanspec(wlc, chanspec);
	if (to_wlc != wlc) {
		wlc_bsscfg_t *to_cfg = wlc_rsdb_bsscfg_clone(wlc, to_wlc, cfg, NULL);
		return to_cfg;
	}
	return cfg;
}

#ifdef SCB_MOVE
/*
*This functions allocates and sets up the scb parameters
* required to create a reassoc after bsscfg move
*/
static void
wlc_rsdb_scb_reassoc(wlc_info_t *to_wlc, wlc_bsscfg_t *to_cfg)
{
	/* Do a SCB reassoc only if the input cfg is an associated STA */
	if (WLC_BSS_CONNECTED(to_cfg) && !BSSCFG_IBSS(to_cfg)) {
		struct scb *to_scb;
		to_cfg->up = TRUE;

		/* TODO: Security parameters copy bettwen SCB */
		to_scb = wlc_scblookupband(to_wlc, to_cfg,
		&to_cfg->current_bss->BSSID, CHSPEC_WLCBANDUNIT(to_cfg->current_bss->chanspec));

		/* Setting the state parameters for SCB */
		if (to_scb) {
			to_scb->state = AUTHENTICATED|ASSOCIATED;
			to_scb->flags |= SCB_MYAP;
		}

		/* join re-creates assumes cfg is associated */
		wlc_sta_assoc_upd(to_cfg, TRUE);
		to_cfg->assoc->type = AS_RECREATE;
		wlc_join_recreate(to_wlc, to_cfg);
	}
}

/*
* This functions copies the cfg association parameters from
*  source bsscfg to destination bsscfg
* It copies current bss, target bss and chanspec variables
* from the source to destination bsscfg
*/

static void
wlc_rsdb_clone_assoc(wlc_bsscfg_t *from_cfg, wlc_bsscfg_t *to_cfg)
{
	/* copy assoc information only if the input cfg is an associated STA */
	if (WLC_BSS_CONNECTED(to_cfg) && !BSSCFG_IBSS(from_cfg)) {
		/* Deep copy for associated */
		bcopy(from_cfg->target_bss, to_cfg->target_bss, sizeof(wlc_bss_info_t));
		to_cfg->target_bss->bcn_prb = NULL;
		to_cfg->target_bss->bcn_prb_len = 0;

		bcopy(from_cfg->current_bss, to_cfg->current_bss, sizeof(wlc_bss_info_t));
		to_cfg->current_bss->bcn_prb = NULL;
		to_cfg->current_bss->bcn_prb_len = 0;

		to_cfg->up = from_cfg->up;
		to_cfg->associated = from_cfg->associated;
		bcopy(from_cfg->BSSID.octet, to_cfg->BSSID.octet, sizeof(from_cfg->BSSID.octet));
		/* copy the RSSI value to avoid roam after move */
		to_cfg->roam->prev_rssi = from_cfg->roam->prev_rssi;
	}
}

static void
wlc_rsdb_copy_pending_tx_pkts(wlc_bsscfg_t *from_cfg, wlc_bsscfg_t *to_cfg)
{
	uint txpktpendtot = 0;
	wlc_info_t *from_wlc = from_cfg->wlc;
	/* Do a SCB reassoc only if the input cfg is an associated STA */
	if (WLC_BSS_CONNECTED(to_cfg) && !BSSCFG_IBSS(from_cfg)) {
		/* Block Diaasoc TX */
		from_cfg->assoc->block_disassoc_tx = TRUE;
		/* If there are pending packets on the fifo, then stop the fifo
		 * processing and re-enqueue packets
		 */
		txpktpendtot = TXPKTPENDTOT(from_wlc);

		if ((txpktpendtot > 0) && (!from_wlc->txfifo_detach_pending)) {
			from_wlc->txfifo_detach_transition_queue = from_cfg->wlcif->qi;
			from_wlc->txfifo_detach_pending = TRUE;
			/* flush the fifos and process txstatus from packets that
			 * were sent before the flush.
			 */
			wlc_bmac_tx_fifo_sync(from_wlc->hw,
			BITMAP_SYNC_ALL_TX_FIFOS, SYNCFIFO);
		}
	}
}

#endif /* SCB_MOVE */
wlc_bsscfg_t*
wlc_rsdb_bsscfg_clone(wlc_info_t *from_wlc, wlc_info_t *to_wlc, wlc_bsscfg_t *from_cfg, int *ret)
{
	int err = BCME_OK;
	int idx = from_cfg->_idx;
	wlc_bsscfg_t *to_cfg;
	bool primary_cfg_move = (wlc_bsscfg_primary(from_wlc) == from_cfg);
	int8 old_idx = -1;

	WLRSDB_DBG(("RSDB clone %s cfg[%d] from wlc[%d] to wlc[%d]\n",
		primary_cfg_move ?"primary":"virtual", idx, UNIT(from_wlc), UNIT(to_wlc)));

	/* TODO: Associated AP clone not handled
	*/
	if (BSSCFG_AP_UP(from_cfg))
		return NULL;

	/* TODO: What about the data packets in TXQ during cfg move? */

	/* process event queue */
	wlc_eventq_flush(from_wlc->eventq);

	/* set up 'to_cfg'
	* in case of secondary move - allocate a new bsscfg structure for 'to_cfg'
	* incase of primary move- set the primary bsscfg of 'to_wlc' to 'to_cfg'
	*/
	if (!primary_cfg_move) {
		to_cfg = wlc_bsscfg_alloc(to_wlc, idx,
			(from_cfg->flags | WLC_BSSCFG_RSDB_CLONE),
			&(from_cfg->cur_etheraddr),
			from_cfg->_ap);

		if (to_cfg == NULL) {
			err = BCME_NOMEM;
			if (ret)
				*ret = err;
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				from_wlc->pub->unit, __FUNCTION__, MALLOCED(from_wlc->osh)));
			return NULL;
		}
	}
	else {
		/* Movement is for primary cfg.. Swap _idx */
		to_cfg = wlc_bsscfg_primary(to_wlc);
		ASSERT(to_cfg);

		wlc_bsscfg_deinit(to_wlc, to_cfg);
		ASSERT(from_cfg->_idx != to_cfg->_idx);
		old_idx = to_cfg->_idx;
		/* Update the cfg index and init the bsscfg */
		to_cfg->_idx = idx;
		wlc_bsscfg_init(to_wlc, to_cfg);
	}

	/* transfer the cubby data of cfg to clonecfg */
	err = wlc_bsscfg_configure_from_bsscfg(from_cfg, to_cfg);

	if (!primary_cfg_move) {
		ASSERT(to_cfg->wlc == to_wlc);
		ASSERT(to_wlc->cfg != to_cfg);

		/* Add CLONE flag to skip the OS interface free during bsscfg_free */
		from_cfg->flags |= WLC_BSSCFG_RSDB_CLONE;

		/* Fixup per port layer references, ONLY if we are OS interface */
		if (!BSSCFG_HAS_NOIF(from_cfg)) {
			WLRSDB_DBG(("RSDB clone: move WLCIF from wlc[%d] to wlc[%d]\n",
			from_wlc->pub->unit, to_wlc->pub->unit));
			wlc_rsdb_update_wlcif(to_wlc, from_cfg, to_cfg);
		}

		/* XXX FIXME: a hack to let disable pass thru
		 * FIXME: Store back the from_cfg in to the allocated location index
		*/
		to_wlc->bsscfg[idx] = from_cfg;
	}
	else {
		from_cfg->assoc->state = AS_IDLE;
	}

#ifdef SCB_MOVE
	if (WLC_BSS_CONNECTED(from_cfg) && !BSSCFG_IBSS(from_cfg)) {

		/* Clone the assoc information from source bsscfg to destination bsscfg */
		wlc_rsdb_clone_assoc(from_cfg, to_cfg);

		/* Clone pending tx packets from 'from_cfg' to 'to_cfg' */
		wlc_rsdb_copy_pending_tx_pkts(from_cfg, to_cfg);

		if (!primary_cfg_move) {
			/* Disable the source config */
			wlc_bsscfg_disable(from_wlc, from_cfg);
		}
	}
#endif /* SCB_MOVE */

	/* Free/Reinit 'from_cfg'
	* In case of secondary config - Free 'from_cfg'
	* In case of primary config - deinit and then init 'from_cfg'
	*/
	if (!primary_cfg_move) {
		from_wlc->bsscfg[idx] = from_cfg;
		wlc_bsscfg_free(from_wlc, from_cfg);
	}
	else {
		/* Disable the source config */
		wlc_bsscfg_disable(from_wlc, from_cfg);

		/* Clear the stale references of cfg if any */
		wlc_bsscfg_deinit(from_wlc, from_cfg);
		/* Update the cfg index and init the bsscfg */
		from_cfg->_idx = old_idx;
		wlc_bsscfg_init(from_wlc, from_cfg);

		wlc_rsdb_update_wlcif(to_wlc, from_cfg, to_cfg);
		to_wlc->bsscfg[from_cfg->_idx] = from_cfg;
	}

	/* wlc->bsscfg[] array is shared. Update bsscfg array. */
	to_wlc->bsscfg[to_cfg->_idx] = to_cfg;

	/* If an SCB was associated to an AP in the cloned cfg recreate the association
	 *	TODO:IBSS CFG scb move, primary bsscfg scb move
	 */
#ifdef SCB_MOVE
	if (WLC_BSS_CONNECTED(to_cfg) && !BSSCFG_IBSS(to_cfg)) {
		wlc_rsdb_scb_reassoc(to_wlc, to_cfg);
	}
#endif // endif
	ASSERT(to_wlc->bsscfg[idx] == from_wlc->bsscfg[idx]);
	if (ret) *ret = err;
	return to_cfg;
}

/* Call this function to update the CFG's wlcif reference
 * Typically called after JOIN-success or AP-up
*/
void wlc_rsdb_update_wlcif(wlc_info_t *wlc, wlc_bsscfg_t *from_cfg, wlc_bsscfg_t *to_cfg)
{
	wlc_info_t *from_wlc = from_cfg->wlc;
	wlc_info_t *to_wlc = to_cfg->wlc;
	int primary_cfg_move = FALSE;
	wlc_if_t * from_wlcif = from_cfg->wlcif;
	wlc_if_t * to_wlcif = to_cfg->wlcif;

	if (from_wlc == to_wlc) {
		WLRSDB_DBG(("%s: already updated \n", __FUNCTION__));
		return;
	}
	primary_cfg_move = (wlc_bsscfg_primary(from_wlc) == from_cfg);

	ASSERT(from_wlcif->type == WLC_IFTYPE_BSS);
	WLRSDB_DBG(("%s:wl%d updating wlcif[%d] cfg/idx(%p/%d->%p/%d) from wl%d to wl%d\n",
		__FUNCTION__, wlc->pub->unit, from_wlcif->index,
		from_cfg, WLC_BSSCFG_IDX(from_cfg), to_cfg, WLC_BSSCFG_IDX(to_cfg),
		from_wlc->pub->unit, to_wlc->pub->unit));

#ifdef BCMDBG_ASSERT
	if (primary_cfg_move)
		ASSERT(WLC_BSSCFG_IDX(from_cfg) != WLC_BSSCFG_IDX(to_cfg));
	else
		ASSERT(WLC_BSSCFG_IDX(from_cfg) == WLC_BSSCFG_IDX(to_cfg));
#endif // endif

	if (primary_cfg_move) {
		uint8 tmp_idx;
		tmp_idx = from_wlcif->index;
		from_wlcif->index = to_wlcif->index;
		to_wlcif->index = tmp_idx;
		memcpy(&to_wlcif->_cnt, &from_wlcif->_cnt, sizeof(wl_if_stats_t));
	}
	/* update host interface reference for wlcif */
	wl_update_if(from_wlc->wl, to_wlc->wl, from_wlcif->wlif, to_wlcif);
	if (primary_cfg_move) {
		/* New cfg uses the existing host interface */
		to_wlcif->wlif = from_wlcif->wlif;
		/* We can nolonger reference the wlif, in the old cfg  */
		from_wlcif->wlif = NULL;
	}

}

/* Call this function to update rsdb_active status
* currently called from sta_assoc_upd() and ap_up_upd()
*/
bool
wlc_rsdb_update_active(wlc_info_t *wlc, bool *old_state)
{
	uint8 active = 0;
	int idx;
	wlc_cmn_info_t *wlc_cmn = wlc->cmn;
	wlc_info_t *wlc_iter;

	if (old_state)
		*old_state = wlc->pub->cmn->_rsdb_active;

	wlc->pub->cmn->_rsdb_active = FALSE;

	FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
		if (wlc_iter && wlc_iter->pub && wlc_iter->pub->associated) {
			active++;
		}
	}
	if (active > 1)
		wlc->pub->cmn->_rsdb_active = TRUE;

	return wlc->pub->cmn->_rsdb_active;
}

int
wlc_rsdb_join_prep_wlc(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *SSID, int len,
	wl_join_scan_params_t *scan_params,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len)
{
	wlc_assoc_t *as = bsscfg->assoc;

	WLRSDB_DBG(("wl%d.%d %s Join Prep on SSID %s\n", wlc->pub->unit, bsscfg->_idx,
		__FUNCTION__, (char *)SSID));

	ASSERT(bsscfg != NULL);
	ASSERT(BSSCFG_STA(bsscfg));
	ASSERT(wlc->pub->corerev >= 15);
	ASSERT(as->state == AS_SCAN);
	ASSERT(bsscfg->associated == 0);
	BCM_REFERENCE(as);

	wlc_bsscfg_enable(wlc, bsscfg);
	wlc_set_mac(bsscfg);

#ifdef WLRM
	if (wlc_rminprog(wlc)) {
		WL_INFORM(("wl%d: association request while radio "
				   "measurement is in progress, aborting measurement\n",
				   wlc->pub->unit));
		wlc_rm_stop(wlc);
	}
#endif /* WLRM */

#ifdef WL11K
		if (wlc_rrm_inprog(wlc)) {
			WL_INFORM(("wl%d: association request while radio "
				   "measurement is in progress, aborting RRM\n",
				   wlc->pub->unit));
			wlc_rrm_stop(wlc);
		}
#endif /* WL11K */

	wlc_set_wake_ctrl(wlc);

	/* save SSID and assoc params for later use in a different context and retry */
	wlc_bsscfg_SSID_set(bsscfg, SSID, len);
	wlc_bsscfg_assoc_params_set(wlc, bsscfg, assoc_params, assoc_params_len);

#ifdef WME
	wlc_wme_initparams_sta(wlc, &bsscfg->wme->wme_param_ie);
#endif // endif
	wlc_join_start_prep(wlc, bsscfg);

	wlc_assoc_init(bsscfg, AS_ASSOCIATION);
	WLRSDB_DBG(("wl%d.%d %s Join Prep Done\n", UNIT(wlc), bsscfg->_idx, SSID));

	return BCME_OK;
}

uint16
wlc_rsdb_mode(void *hdl)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;

	ASSERT(WLC_RSDB_CURR_MODE(wlc) < WLC_RSDB_MODE_MAX);

	return (uint16)SWMODE2PHYMODE(WLC_RSDB_CURR_MODE(wlc));
}

#ifndef WL_DUALNIC_RSDB
void
wlc_rsdb_set_phymode(void *hdl, uint32 phymode)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;
	wlc_pub_t	*pub;

	if (!wlc || !wlc->pub)
		return;

	pub = wlc->pub;

	WL_TRACE(("wl%d: %s\n", pub->unit, __FUNCTION__));
	ASSERT(wlc_bmac_rsdb_cap(wlc->hw));
	/* PHY Mode has to be written only in Core 0 cflags.
	 * For Core 1 override, switch to core-0 and write it.
	 */
	phymode = phymode << SICF_PHYMODE_SHIFT;
	if ((si_coreunit(pub->sih) == 0x1) &&
		(wlc_rsdb_mode(wlc) == PHYMODE_RSDB_SISO_1)) {
		si_d11_switch_addrbase(pub->sih, 0);
		/* Set up the PHY MODE for RSDB PHY 4349 */
		si_core_cflags(pub->sih, SICF_PHYMODE, phymode);
		si_d11_switch_addrbase(pub->sih, 1);
		OSL_DELAY(10);
	} else if (si_coreunit(pub->sih) == 0x0)  {
		/* Set up the PHY MODE for RSDB PHY 4349 */
		si_core_cflags(pub->sih, SICF_PHYMODE, phymode);
		OSL_DELAY(10);
	}

}

#define D11MAC_BMC_TPL_IDX	7
void
wlc_rsdb_bmc_smac_template(void *hdl, int tplbuf, uint32 doublebufsize)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;
	osl_t *osh;
	int i;
	wlc_pub_t	*pub;

	if (!wlc || !wlc->pub)
		return;

	pub = wlc->pub;
	WL_TRACE(("wl%d: %s\n", pub->unit, __FUNCTION__));
	ASSERT(wlc_bmac_rsdb_cap(wlc->hw));

	/* 4349 Template init for Core 1. Just duplicate for now.
	 * Make the code a proc later and call with different base
	 */
	osh = (osl_t *)wlc->osh;
	if ((si_coreunit(pub->sih) == 0) &&
	    (wlc_rsdb_mode(pub->wlc) == PHYMODE_RSDB)) {
		d11regs_t *sregs = si_d11_switch_addrbase(pub->sih, 1);
		OR_REG(osh, &sregs->maccontrol, MCTL_IHR_EN);
		/* init template */
		for (i = 0; i < tplbuf; i ++) {
			int end_idx;
			end_idx = tplbuf + i + 2 + doublebufsize;
			if (end_idx >= 2 * tplbuf)
				end_idx = (2 * tplbuf) - 1;
			W_REG(osh, &sregs->u.d11acregs.MSDUEntryStartIdx, tplbuf + i);
			W_REG(osh, &sregs->u.d11acregs.MSDUEntryEndIdx, end_idx);
			W_REG(osh, &sregs->u.d11acregs.MSDUEntryBufCnt,
				end_idx - (tplbuf + i) + 1);
			W_REG(osh, &sregs->u.d11acregs.PsmMSDUAccess,
			      ((1 << PsmMSDUAccess_WriteBusy_SHIFT) |
			       (i << PsmMSDUAccess_MSDUIdx_SHIFT) |
			       (D11MAC_BMC_TPL_IDX << PsmMSDUAccess_TIDSel_SHIFT)));

			SPINWAIT((R_REG(pub->osh, &sregs->u.d11acregs.PsmMSDUAccess) &&
				(1 << PsmMSDUAccess_WriteBusy_SHIFT)), 200);
			if (R_REG(pub->osh, &sregs->u.d11acregs.PsmMSDUAccess) &
			    (1 << PsmMSDUAccess_WriteBusy_SHIFT))
				{
					WL_ERROR(("wl%d: PSM MSDU init not done yet :-(\n",
					(pub->unit + 1)));
				}
		}

		OR_REG(osh, &sregs->maccontrol, ~MCTL_IHR_EN);
		si_d11_switch_addrbase(pub->sih, 0);
	}
}
#endif /* WL_DUALNIC_RSDB */

/* This function evaluates if current iovar needs to be applied on both
* cores or single core during dongle-MFGTEST & dualNIC DVT scenario
*/
bool wlc_rsdb_chkiovar(const bcm_iovar_t *vi_ptr, uint32 actid, int32 wlc_indx)
{
	bool result = FALSE;
#ifndef WLRSDB_DVT
	result = (((vi_ptr->flags) & IOVF_RSDB_SET) && (IOV_ISSET(actid)) && (wlc_indx < 0));
#else
	int i, size;
	size = ARRAYSIZE(rsdb_gbl_iovars);
	for (i = 0; i < size; i++) {
		if (!strcmp(vi_ptr->name, rsdb_gbl_iovars[i])) {
			result = TRUE;
			break;
		}
	}
#endif	/* !WLRSDB_DVT */
	return result;
}

#if defined(RSDB_DFS_SCAN) && defined(WLDFS)
int
wlc_rsdb_dfs_scan_prep(wlc_info_t * wlc)
{
	wlc_info_t *other_wlc;

	/* DFS Scan request on current wlc. Background DFS scan is possible only
	 * when device is in MIMO mode.
	 */
	if (!RSDB_ENAB(wlc->pub)) {
		return BCME_UNSUPPORTED;
	}
	other_wlc = wlc_rsdb_get_other_wlc(wlc);

	if (!other_wlc || other_wlc->pub->associated) {
		return BCME_EPERM;
	}

	/* If device is in MIMO mode, but auto mode is not enabled, return error. */
	if (WLC_RSDB_SINGLE_MAC_MODE(WLC_RSDB_CURR_MODE(wlc)) && !WLC_RSDB_IS_AUTO_MODE(wlc)) {
		return BCME_ERROR;
	}

	/* If device is in MIMO AUTO mode, do protocol mode switch. */
	if (WLC_RSDB_SINGLE_MAC_MODE(WLC_RSDB_CURR_MODE(wlc))) {
		wlc_rsdb_downgrade_wlc(wlc);
		return BCME_BUSY;
	} else {
		/* Device is in RSDB mode. Trigger DFS scan. */
		return BCME_OK;
	}
}
#endif /* RSDB_DFS_SCAN && WLDFS */

#endif /* WLRSDB */
