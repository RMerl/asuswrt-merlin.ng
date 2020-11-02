/*
 * Power Management Mode PM_FAST (PM2) functions
 *
 *   Copyright 2020 Broadcom
 *
 *   This program is the proprietary software of Broadcom and/or
 *   its licensors, and may only be used, duplicated, modified or distributed
 *   pursuant to the terms and conditions of a separate, written license
 *   agreement executed between you and Broadcom (an "Authorized License").
 *   Except as set forth in an Authorized License, Broadcom grants no license
 *   (express or implied), right to use, or waiver of any kind with respect to
 *   the Software, and Broadcom expressly reserves all rights in and to the
 *   Software and all intellectual property rights therein.  IF YOU HAVE NO
 *   AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *   WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *   THE SOFTWARE.
 *
 *   Except as expressly set forth in the Authorized License,
 *
 *   1. This program, including its structure, sequence and organization,
 *   constitutes the valuable trade secrets of Broadcom, and you shall use
 *   all reasonable efforts to protect the confidentiality thereof, and to
 *   use this information only in connection with your use of Broadcom
 *   integrated circuit products.
 *
 *   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *   REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *   OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *   DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *   NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *   ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *   CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *   OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *   BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *   SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *   IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *   IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *   ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *   OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *   NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *   $Id$
 */

/**
 * @file
 * @brief
 * Twiki: [WlDriverPowerSave]
 */

#include <wlc_cfg.h>
#ifdef STA

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <proto/bcmip.h>
#include <proto/wpa.h>
#include <proto/vlan.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#if defined(WLTEST) || defined(BCMDBG_DUMP)
#include <bcmnvram.h>
#endif // endif
#include <wlioctl.h>
#include <epivers.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX) || defined(EXT_STA) || defined(STA) || \
	defined(LINUX_CRYPTO)
#include <proto/eapol.h>
#endif // endif
#include <bcmwpa.h>
#ifdef BCMCCX
#include <bcmcrypto/ccx.h>
#endif /* BCMCCX */
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_cca.h>
#include <wlc_interfere.h>
#include <wlc_bsscfg.h>
#include <wlc_vndr_ie_list.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_led.h>
#include <wlc_frmutil.h>
#include <wlc_stf.h>

#ifdef WLNAR
#include <wlc_nar.h>
#endif // endif
#ifdef WLAMSDU
#include <wlc_amsdu.h>
#endif // endif
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#include <wlc_ampdu_rx.h>
#include <wlc_ampdu_cmn.h>
#endif // endif
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif // endif
#ifdef WLDLS
#include <wlc_dls.h>
#endif // endif
#ifdef WLBSSLOAD
#include <wlc_bssload.h>
#endif // endif
#ifdef L2_FILTER
#include <wlc_l2_filter.h>
#endif // endif
#ifdef WLMCNX
#include <wlc_mcnx.h>
#include <wlc_tbtt.h>
#endif // endif
#ifdef WLP2P
#include <wlc_p2p.h>
#endif // endif
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#include <wlc_bta.h>
#endif // endif
#include <wlc_scb_ratesel.h>
#ifdef WL_LPC
#include <wlc_scb_powersel.h>
#endif /* WL_LPC */
#include <wlc_event.h>
#ifdef WOWL
#include <wlc_wowl.h>
#endif // endif
#ifdef WOWLPF
#include <wlc_wowlpf.h>
#endif // endif
#include <wlc_seq_cmds.h>
#ifdef WLOTA_EN
#include <wlc_ota_test.h>
#endif /* WLOTA_EN */
#ifdef WLDIAG
#include <wlc_diag.h>
#endif // endif
#include <wl_export.h>
#include "d11ucode.h"
#if defined(BCMSUP_PSK) || defined(BCMCCX)
#include <wlc_sup.h>
#endif // endif
#if defined(BCMAUTH_PSK)
#include <wlc_auth.h>
#endif // endif
#ifdef BCMSDIO
#include <bcmsdh.h>
#endif // endif
#ifdef WET
#include <wlc_wet.h>
#endif // endif
#ifdef WMF
#include <wlc_wmf.h>
#endif // endif
#ifdef PSTA
#include <wlc_psta.h>
#endif /* PSTA */
#if defined(BCMNVRAMW) || defined(WLTEST)
#include <bcmotp.h>
#endif // endif
#ifdef BCMCCMP
#include <bcmcrypto/aes.h>
#endif // endif
#include <wlc_rm.h>
#ifdef BCMCCX
#include <wlc_ccx.h>
#endif // endif
#include "wlc_cac.h"
#include <wlc_ap.h>
#ifdef AP
#include <wlc_apcs.h>
#endif // endif
#include <wlc_scan.h>
#ifdef WL11K
#include <wlc_rrm.h>
#endif /* WL11K */
#ifdef WLWNM
#include <wlc_wnm.h>
#endif // endif
#ifdef WLC_HIGH_ONLY
#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>
#include <bcm_xdr.h>
#include <wlc_rpc.h>
#include <wlc_rpctx.h>
#endif /* WLC_HIGH_ONLY */
#if defined(RWL_DONGLE) || defined(UART_REFLECTOR)
#include <rwl_shared.h>
#include <rwl_uart.h>
#endif /* RWL_DONGLE || UART_REFLECTOR */
#include <wlc_extlog.h>
#include <wlc_assoc.h>
#if defined(RWL_WIFI) || defined(WIFI_REFLECTOR)
#include <wlc_rwl.h>
#endif // endif
#ifdef WLC_HOSTOID
#include <wlc_hostoid.h>
#endif /* WLC_HOSTOID */
#ifdef WLPFN
#include <wl_pfn.h>
#endif // endif
#ifdef STA
#include <wlc_wpa.h>
#endif /* STA */
#if defined(PROP_TXSTATUS)
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif // endif
#include <wlc_lq.h>
#include <wlc_11h.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_dfs.h>
#include <wlc_11d.h>
#include <wlc_cntry.h>
#include <bcm_mpool_pub.h>
#include <wlc_utils.h>
#include <wlc_hrt.h>
#include <wlc_prot.h>
#include <wlc_prot_g.h>
#define _inc_wlc_prot_n_preamble_	/* include static INLINE uint8 wlc_prot_n_preamble() */
#include <wlc_prot_n.h>
#include <wlc_11u.h>
#include <wlc_probresp.h>
#ifdef WLTOEHW
#include <wlc_tso.h>
#endif /* WLTOEHW */
#ifdef WL11AC
#include <wlc_vht.h>
#endif // endif
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)
#include <wlc_wapi.h>
#endif // endif
#include <wlc_pcb.h>
#include <wlc_txc.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif
#ifdef MFP
#include <wlc_mfp.h>
#endif // endif
#include <wlc_macfltr.h>
#include <wlc_addrmatch.h>
#include <wlc_bmon.h>
#ifdef WL_RELMCAST
#include "wlc_relmcast.h"
#endif // endif
#include <wlc_btcx.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#include <wlc_akm_ie.h>
#include <wlc_ht.h>
#if defined(P2PO) || defined(ANQPO)
#include <wl_gas.h>
#endif // endif
#ifdef P2PO
#include <wl_p2po_disc.h>
#endif // endif
#ifdef ANQPO
#include <wl_anqpo.h>
#endif // endif
#include <wlc_hs20.h>

#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif // endif
#include <wlc_pm.h>

#ifdef WL_EXCESS_PMWAKE
#ifndef WL_PWRSTATS
#error WL_PWRSTATS is required for WL_EXCESS_PMWAKE
#endif /* !WL_PWRSTATS */
#endif /* WL_EXCESS_PMWAKE */

#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif /* WL_PWRSTATS */

#ifdef WL_EXCESS_PMWAKE
static void wlc_get_ucode_dbg(wlc_info_t *wlc, wl_pmalert_ucode_dbg_t *ud);
#endif /* WL_EXCESS_PMWAKE */

/* Start the PM2 return to sleep timer */
void
wlc_pm2_sleep_ret_timer_start(wlc_bsscfg_t *cfg)
{
	wlc_pm_st_t *pm = cfg->pm;

#ifdef WL_PWRSTATS
	if (PWRSTATS_ENAB(cfg->wlc->pub)) {
		wlc_pwrstats_frts_start(cfg->wlc->pwrstats);
	}
#endif /* WL_PWRSTATS */
	/* the new timeout will supersede the old one so delete old one first */
	wlc_hrt_del_timeout(pm->pm2_ret_timer);
	wlc_hrt_add_timeout(pm->pm2_ret_timer,
	                    WLC_PM2_TICK_GP(pm->pm2_sleep_ret_time_left),
	                    wlc_pm2_sleep_ret_timeout_cb, (void *)cfg);
}

/* Stop the PM2 tick timer */
void
wlc_pm2_sleep_ret_timeout_cb(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;

	if (cfg->pm->PM == PM_FAST) {
		wlc_pm2_sleep_ret_timeout(cfg);
	}
}

void
wlc_pm2_sleep_ret_timer_stop(wlc_bsscfg_t *cfg)
{
	wlc_pm_st_t *pm = cfg->pm;

#ifdef WL_PWRSTATS
	if (PWRSTATS_ENAB(cfg->wlc->pub)) {
		wlc_pwrstats_frts_end(cfg->wlc->pwrstats);
	}
#endif /* WL_PWRSTATS */

	if (pm->pm2_ret_timer != NULL)
		wlc_hrt_del_timeout(pm->pm2_ret_timer);

	WL_RTDC2(cfg->wlc, "wlc_pm2_sleep_ret_timer_stop", 0, 0);
}

/* PM2 tick timer timeout handler */
void
wlc_pm2_sleep_ret_timeout(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	uint wake_time;
	uint last_wake_time;
	uint pm2_sleep_ret_time = pm->pm2_sleep_ret_time_left;
	uint time_left;

	/* Ignore timeout if it occurs just as we exited PM2 (race condition). */
	if (pm->PM != PM_FAST) {
		WL_RTDC(cfg->wlc, "wlc_pm2_timeout: not PM2", 0, 0);
		return;
	}

	/* Ignore timeout if wl is no longer up.  wlc_hrt functions cannot do
	 * register reads when no clock is provided to the core.
	 */
	if (!wlc->pub->up) {
		WL_RTDC(cfg->wlc, "wlc_pm2_timeout: not up", 0, 0);
		return;
	}

	/* Do not pass &pm->pm2_last_wake_time directly to wlc_hrt_getdelta().
	 * wlc_hrt_getdelta() has a side effect of modifying the timestamp
	 * pointed to by its 2nd parameter.
	 */
	last_wake_time = pm->pm2_last_wake_time;
	wake_time = wlc_hrt_getdelta(wlc->hrti, &last_wake_time);
	wake_time = wake_time/1024;

	time_left = pm2_sleep_ret_time - wake_time;

	/* 5 ms is to account for delays to avoid re-fires for small time */
	/* The check for > pm2_sleep_ret_time is for the case of wraparound */
	if (time_left <= 5 || time_left > pm2_sleep_ret_time) {
		/* Re-initialize back for next cycle */
		pm->pm2_sleep_ret_time_left = pm->pm2_sleep_ret_time;
#ifdef WL_PWRSTATS
		if (PWRSTATS_ENAB(wlc->pub)) {
			wlc_pwrstats_frts_end(cfg->wlc->pwrstats);
		}
#endif /* WL_PWRSTATS */
		wlc_pm2_enter_ps(cfg);
	}
	else {
		/* Restart the PM2 tick timer for the remaining wake time */
		pm->pm2_sleep_ret_time_left = time_left;
		wlc_pm2_sleep_ret_timer_start(cfg);

		if (SCAN_IN_PROGRESS(wlc->scan) && wlc_roam_scan_islazy(wlc, cfg, TRUE)) {
			if (wlc_lazy_roam_scan_suspend(wlc, cfg)) {
				/* Postpone background roam scan in presence of any activity
				 * Wait for the next power save mode enter check
				 */
				WL_SRSCAN(("Extend background roam scan home time by %dms "
					"(wake=%dms)", 2 * pm->pm2_sleep_ret_time, wake_time));
				WL_ASSOC(("Extend background roam scan home time by %dms "
					"(wake=%dms)", 2 * pm->pm2_sleep_ret_time, wake_time));
				wlc_scan_timer_update(wlc->scan, 2 * pm->pm2_sleep_ret_time);
			} else {
				/* Cancel background roam scan in presence of any activity */
				WL_SRSCAN(("Cancel background roam scan (wake=%dms)", wake_time));
				WL_ASSOC(("Cancel background roam scan (wake=%dms)", wake_time));
				wlc_assoc_abort(cfg);
			}
		}
	}
}

void
wlc_pm2_ret_upd_last_wake_time(wlc_bsscfg_t *cfg, uint32* tsf_l)
{
	if (!cfg)
		return;
	if (!cfg->pm)
		return;

	if (tsf_l) {
		cfg->pm->pm2_last_wake_time = *tsf_l;
	} else {
		/* Note: for BMAC drivers, calling wlc_hrt_gettime() here
		 * results in a very slow read across the bus.  This reduces
		 * throughput significantly if called from the data path.
		 */
		cfg->pm->pm2_last_wake_time = 0;
	}
}

/* Try enter PS mode in power mgmt mode PM_FAST.
 * If entered PS mode, stop the PM2 timers.
 */
void
wlc_pm2_enter_ps(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;
	uint txpktcnt = 0;

	/* IBSS PS may have pkts pending for a peer which is not reachable, ignore
	 * pktcnt to go back to sleep in PM2
	 */
	if (!AIBSS_ENAB(wlc->pub) || !BSSCFG_IBSS(cfg)) {
		txpktcnt = wlc_txpktcnt(wlc);
	}

	/* Do not enter PS mode if:
	 * a) a scan is in progress, or
	 * b) there are pending tx packets and the Receive Throttle feature is
	 *    not enabled.
	 *    (Not entering PS mode for the OFF part of the receive
	 *    throttle duty cycle for any reason will defeat the main purpose
	 *    of the receive throttle feature - heat reduction.)
	 * c) we are no longer in PM2
	 * d) we are no longer associated to an AP
	 * e) PMenabledModuleId is non-zero, meaning another module put us in PS already
	 */
	if ((!PM2_RCV_DUR_ENAB(cfg) && (txpktcnt > 0)) ||
	    pm->PM != PM_FAST ||
	    !cfg->associated ||
	    pm->PM_override ||
	    pm->PMenabledModuleId) {

		WL_RTDC(wlc, "wlc_pm2_enter_ps: no PS, scan=%u txcnt=%u",
			SCAN_IN_PROGRESS(wlc->scan), wlc_txpktcnt(wlc));
		WL_RTDC(wlc, "    stas=%u AS_IN_PROGRESS=%d",
			wlc->stas_associated, AS_IN_PROGRESS(wlc));
		WL_RTDC(wlc, "    PM=%u", pm->PM, 0);
	}
	else if (AS_IN_PROGRESS(wlc) || SCAN_IN_PROGRESS(wlc->scan)) {
		WL_RTDC(wlc, "wlc_pm2_enter_ps: no PS, scan=%u AS_IN_PROGRESS=%d",
			SCAN_IN_PROGRESS(wlc->scan), AS_IN_PROGRESS(wlc));

		if (wlc_roam_scan_islazy(wlc, cfg, TRUE)) {
			/* Continue with scan in absence of any activity */
			wlc_scan_timer_update(wlc->scan, 0);
		}
	}
	else {
		/* Start entering PS mode */
		wlc_set_pmstate(cfg, TRUE);
		wlc_dfrts_reset_counters(cfg);
	}

	/* If we succeeded in starting to enter PS mode or
	 * if another module already put us in PS mode.
	 */
	if (pm->PMpending || pm->PMenabledModuleId) {
		WL_RTDC(wlc, "wlc_pm2_enter_ps: succeeded", 0, 0);

		/* Enter the Receive Throttle feature state where we are in the
		 * transition between the ON and OFF parts of the duty cycle.  In this
		 * state we are waiting for a PM-indicated ACK to complete entering PS
		 * mode.
		 */
#if defined(WL_PM2_RCV_DUR_LIMIT)
		if (PM2_RCV_DUR_ENAB(cfg)) {
			/* non-zero pm->PMenabledModuleId means another module has already
			 * put us in PS mode.  This means that we won't get PM-indicated
			 * ACK because we're already in PS mode.  Thus, proceed to the next
			 * state, which is PM2RD_WAIT_BCN.
			 */
			if (pm->PMenabledModuleId) {
				pm->pm2_rcv_state = PM2RD_WAIT_BCN;
				WL_RTDC(wlc, "pm2_rcv_state=PM2RD_WAIT_BCN", 0, 0);
			}
			else {
				pm->pm2_rcv_state = PM2RD_WAIT_RTS_ACK;
				WL_RTDC(wlc, "pm2_rcv_state=PM2RD_WAIT_RTS_ACK", 0, 0);
			}
			wlc_pm2_rcv_timer_stop(cfg);
		}
#endif /* WL_PM2_RCV_DUR_LIMIT */

		/* Stop the PM2 tick timer */
		wlc_pm2_sleep_ret_timer_stop(cfg);
	}
	/* else we did not start entering PS mode */
	else {
		/* Restart the return to sleep timer to try again later */
		WL_RTDC2(wlc, "wlc_pm2_enter_ps: restart timer", 0, 0);
		wlc_pm2_sleep_ret_timer_start(cfg);

#ifdef BCMDBG
		WL_RTDC(wlc, "wlc_pm2_enter_ps: enter PS failed, PMep=%02u AW=%02u",
			(pm->PMenabled ? 10 : 0) | pm->PMpending,
			(PS_ALLOWED(cfg) ? 10 : 0) | STAY_AWAKE(wlc));
		WL_RTDC(wlc, "    bss=%u assoc=%u", cfg->BSS, cfg->associated);
		WL_RTDC(wlc, "    dptpend=%u portopen=%u",
			FALSE, WLC_PORTOPEN(cfg));
#endif /* BCMDBG */
	}
}

#if defined(WL_PM2_RCV_DUR_LIMIT)
/* Reset the PM2 receive throttle duty cycle feature */
void
wlc_pm2_rcv_reset(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;

	/* The Receive Throttle feature overrides the TBTT wakeup interval
	 * specified by bcn_li_bcn and bcn_li_dtim.  So we must restore the
	 * wakeup interval to match bcn_li_bcn and bcn_li_dtim again.
	 */
	wlc_bcn_li_upd(wlc);

	/* Stop the receive throttle timer */
	wlc_pm2_rcv_timer_stop(cfg);
	/* Next beacon should start the timer */

	pm->pm2_rcv_state = PM2RD_IDLE;
	WL_RTDC(wlc, "pm2_rcv_state=PM2RD_IDLE", 0, 0);
}

/* Start the PM2 receive throttle duty cycle timer.
 *
 * This timer should be started at the beginning of the ON part of the
 * PM2 receive duty cycle.
 * When this timer expires, the OFF part of the duty cycle begins.
 * At the end of the current beacon period, the duty cycle ends.
 * (The end of the current beacon period is indicated by the next TBTT
 * wakeup.  This function adjusts the TBTT wake up interval to ensure
 * the wakeup interval is the same as the beacon interval.)
 */
void
wlc_pm2_rcv_timer_start(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;

	 /* Start the PM2 receive duration timer */
	 /* the new timeout will supersede the old one so delete old one first */
	wlc_hrt_del_timeout(pm->pm2_rcv_timer);
	wlc_hrt_add_timeout(pm->pm2_rcv_timer,
	                    WLC_PM2_TICK_GP(pm->pm2_rcv_time),
	                    wlc_pm2_rcv_timeout_cb, (void *)cfg);

	pm->pm2_rcv_state = PM2RD_WAIT_TMO;
	WL_RTDC(wlc, "pm2_rcv_state=PM2RD_WAIT_TMO", 0, 0);

	/* Ensure we will get our next TBTT wake up interrupt at the next beacon:
	 * update the beacon listen interval in shared memory to instruct ucode to
	 * wake up to listen to every beacon, as if bcn_li_bcn == 1 and
	 * bcn_li_dtim == 0.
	 *
	 * This is needed because the wakeup interval varies between 1 and DTIM
	 * beacon intervals depending on recent tx/rx activity.
	 */
	wlc_write_shm(wlc, M_BCN_LI, (0 /* bcn_li_dtim */  << 8) | 1 /* bcn_li_bcn */);
}

/* Stop the PM2 tick timer */
void
wlc_pm2_rcv_timeout_cb(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;

	if (cfg->pm->PM == PM_FAST) {
		wlc_pm2_rcv_timeout(cfg);
	}
}

void
wlc_pm2_rcv_timer_stop(wlc_bsscfg_t *cfg)
{
	wlc_pm_st_t *pm = cfg->pm;

	if (pm->pm2_rcv_timer != NULL)
		wlc_hrt_del_timeout(pm->pm2_rcv_timer);

	WL_RTDC2(cfg->wlc, "wlc_pm2_rcv_timer_stop", 0, 0);
}

/* PM2 tick timer timeout handler */
void
wlc_pm2_rcv_timeout(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	wlc_pm_st_t *pm = cfg->pm;

	(void)wlc;

	/* Ignore timeout if it occurs just as we exited PM2 (race condition). */
	if (pm->PM != PM_FAST) {
		WL_RTDC(wlc, "wlc_pm2_rcv_timeout: not PM2", 0, 0);
		return;
	}

	/* Decrement the Receive Throttle countdown timers.
	 * If timer reaches 0, enter PS mode and stop the PM2 timer.
	 */
	if (PM2_RCV_DUR_ENAB(cfg)) {
		WL_RTDC(wlc, "wlc_pm2_rcv_timeout: RTDC enter ps", 0, 0);
		wlc_pm2_enter_ps(cfg);
		return;
	}

	/* Let the next beacon start next timer again */
}
#endif /* WL_PM2_RCV_DUR_LIMIT */

void
wlc_dfrts_reset_counters(wlc_bsscfg_t *bsscfg)
{
	WL_RTDC(bsscfg->wlc, "dfrts_reset", 0, 0);
	bsscfg->pm->dfrts_rx_pkts = 0;
	bsscfg->pm->dfrts_tx_pkts = 0;
	bsscfg->pm->dfrts_rx_bytes = 0;
	bsscfg->pm->dfrts_tx_bytes = 0;
	bsscfg->pm->dfrts_reached_threshold = FALSE;
	bsscfg->pm->pm2_sleep_ret_time_left = bsscfg->pm->pm2_sleep_ret_time;
}

/* Look at the updated tx/rx counters and restart the PM2 return to sleep
 * timer if needed.  This applies to both the FRTS and DFRTS features.
 */
void
wlc_update_sleep_ret(wlc_bsscfg_t *bsscfg, bool inc_rx, bool inc_tx,
	uint rxbytes, uint txbytes)
{
	wlc_pm_st_t *pm = bsscfg->pm;
	bool first_bcn_sleep_tmr_rx = FALSE;
	uint sleep_ret_time;

	if (pm->PM != PM_FAST || pm->PMenabled || !bsscfg->associated)
		return;

	/* Check if this is the first rx pkt since starting the sleep return
	 * timer for pm2_bcn_sleep_ret_time.
	 */
	if (pm->pm2_rx_pkts_since_bcn == 1 && inc_rx &&
	    pm->pm2_bcn_sleep_ret_time != 0 &&
	    pm->pm2_sleep_ret_time_left == pm->pm2_bcn_sleep_ret_time)
		first_bcn_sleep_tmr_rx = TRUE;

	/* Optimization to bypass all the DFRTS threshold checks if this PM
	 * cycle has already passed the threshold for switching to the high
	 * DFRTS timeout.
	 */
	if (pm->dfrts_reached_threshold) {
		WL_RTDC(bsscfg->wlc, "dfrts_upd: already reached threshold", 0, 0);
		goto restart_tmr;
	}

	/* If Dynamic FRTS is not enabled */
	if (pm->dfrts_logic == WL_DFRTS_LOGIC_OFF) {
		if (first_bcn_sleep_tmr_rx) {
			WL_RTDC(bsscfg->wlc, "frts_upd: bcnsr -> frts", 0, 0);
			/* restart the sleep ret timer for the normal FRTS time */
			goto restart_tmr;
		} else {
			/* extend the sleep return time for this wake period */
			pm->pm2_sleep_ret_time_left = MAX(pm->pm2_sleep_ret_time_left,
				pm->pm2_sleep_ret_time);
			WL_RTDC(bsscfg->wlc, "frts_upd: timeleft=%u",
				pm->pm2_sleep_ret_time_left, 0);
			return;
		}
	}

	if (inc_rx)
		pm->dfrts_rx_pkts++;
	if (inc_tx)
		pm->dfrts_tx_pkts++;
	pm->dfrts_rx_bytes += rxbytes;
	pm->dfrts_tx_bytes += txbytes;

	if (pm->dfrts_logic == WL_DFRTS_LOGIC_AND) {
		/* do the AND of all conditions whose algo enable bits are 1 */
		if (pm->dfrts_rx_pkts >= pm->dfrts_rx_pkts_threshold &&
		    pm->dfrts_tx_pkts >= pm->dfrts_tx_pkts_threshold &&
		    pm->dfrts_tx_pkts + pm->dfrts_rx_pkts >=
			pm->dfrts_txrx_pkts_threshold &&
		    pm->dfrts_rx_bytes >= pm->dfrts_rx_bytes_threshold &&
		    pm->dfrts_tx_bytes >= pm->dfrts_tx_bytes_threshold &&
		    pm->dfrts_tx_bytes + pm->dfrts_rx_bytes >=
			pm->dfrts_txrx_bytes_threshold) {
			pm->dfrts_reached_threshold = TRUE;
			WL_RTDC(bsscfg->wlc, "frts_upd: dfrts lo ->hi, AND", 0, 0);
		}
	}
	else if (pm->dfrts_logic == WL_DFRTS_LOGIC_OR) {
		/* do the OR of all conditions whose algo enable bits are 1 */
		if ((pm->dfrts_rx_pkts_threshold > 0 &&
			pm->dfrts_rx_pkts >= pm->dfrts_rx_pkts_threshold) ||
		    (pm->dfrts_tx_pkts_threshold > 0 &&
			pm->dfrts_tx_pkts >= pm->dfrts_tx_pkts_threshold) ||
		    (pm->dfrts_txrx_pkts_threshold > 0 &&
			(pm->dfrts_tx_pkts + pm->dfrts_rx_pkts) >=
			pm->dfrts_txrx_pkts_threshold) ||
		    (pm->dfrts_rx_bytes_threshold > 0 &&
			pm->dfrts_rx_bytes >= pm->dfrts_rx_bytes_threshold) ||
		    (pm->dfrts_tx_bytes_threshold > 0 &&
			pm->dfrts_tx_bytes >= pm->dfrts_tx_bytes_threshold) ||
		    (pm->dfrts_txrx_bytes_threshold > 0 &&
			(pm->dfrts_tx_bytes + pm->dfrts_rx_bytes) >=
			pm->dfrts_txrx_bytes_threshold)) {
			pm->dfrts_reached_threshold = TRUE;
			WL_RTDC(bsscfg->wlc, "frts_upd: dfrts lo ->hi, OR", 0, 0);
		}
	}

restart_tmr:
	/* extend the sleep return time for this wake period */
	if (pm->dfrts_reached_threshold) {
		sleep_ret_time = pm->dfrts_high_ms;
	} else {
		sleep_ret_time = pm->pm2_sleep_ret_time;
	}

	/* If this is the first rx during pm2_bcn_sleep_ret_time or
	 * (pm2_bcn_sleep_ret_time is not in effect and we are trying
	 * to shrink the sleep return time, ie. dfrts hi < dfrts lo)
	 */
	if (first_bcn_sleep_tmr_rx ||
		(sleep_ret_time < pm->pm2_sleep_ret_time_left &&
		pm->pm2_sleep_ret_time_left != pm->pm2_bcn_sleep_ret_time)) {

		/* Override the sleep return time for this wake period */
		pm->pm2_sleep_ret_time_left = sleep_ret_time;
		WL_RTDC(bsscfg->wlc, "frts_upd: %d -> frts %d",
			first_bcn_sleep_tmr_rx ? 1 : 0,
			pm->dfrts_reached_threshold ? 1 : 0);

		/* Immediately restart the sleep return timer */
		WL_RTDC(bsscfg->wlc, "frts_upd: restart timer for %u",
			pm->pm2_sleep_ret_time_left, 0);
#ifdef WL_PWRSTATS
		if (PWRSTATS_ENAB(bsscfg->wlc->pub)) {
			wlc_pwrstats_set_frts_data(bsscfg->wlc->pwrstats, TRUE);
		}
#endif /* WL_PWRSTATS */
		wlc_pm2_sleep_ret_timer_start(bsscfg);
	} else {
		/* Extend the sleep return time for this wake period */
		pm->pm2_sleep_ret_time_left = MAX(pm->pm2_sleep_ret_time_left,
			sleep_ret_time);

		/*
		 * Do not restart the sleep return timer here.
		 * Let wlc_pm2_sleep_ret_timeout() restart the timer using
		 * the modified pm->pm2_sleep_ret_time_left.
		 */
		WL_RTDC(bsscfg->wlc, "frts_upd:%d timeleft=%u",
			(pm->pm2_bcn_sleep_ret_time != 0 &&
			pm->pm2_sleep_ret_time_left == pm->pm2_bcn_sleep_ret_time &&
			pm->pm2_rx_pkts_since_bcn == 0 &&
			pm->dfrts_reached_threshold) ? 1 : 0,
			pm->pm2_sleep_ret_time_left);
	}

	WL_RTDC(bsscfg->wlc, "dfrts_upd: rxp=%u txp=%u",
		pm->dfrts_rx_pkts, pm->dfrts_tx_pkts);
	WL_RTDC(bsscfg->wlc, "           rxb=%u txb=%u",
		pm->dfrts_rx_bytes, pm->dfrts_tx_bytes);
}
#ifdef WL_EXCESS_PMWAKE
static void
wlc_create_pmalert_data(wlc_info_t *wlc, uint32 reason)
{
	wl_pmalert_t *pm_alert_data;
	uint8 *data;
	wl_pmalert_fixed_t *fixed;

	uint32 pmalert_len = sizeof(wl_pmalert_t) - 1 + sizeof(wl_pmalert_fixed_t) +
		sizeof(wl_pmalert_pmstate_t) + 2 * sizeof(wl_pmalert_event_dur_t) +
		2 * sizeof(uint32) * (WLC_PMD_EVENT_MAX - 1) +
		sizeof(wlc_pm_debug_t) * (WLC_STA_AWAKE_STATES_MAX - 1) +
		sizeof(wl_pmalert_ucode_dbg_t);

	pm_alert_data = MALLOC(wlc->osh, pmalert_len);

	if (pm_alert_data == NULL) {
		/* Send the event without event data */
		wlc_generate_pm_alert_event(wlc, reason, NULL, 0);
		return;
	}
	memset(pm_alert_data, 0, pmalert_len);
	data = pm_alert_data->data;

	fixed = (wl_pmalert_fixed_t *)data;
	fixed->type = WL_PMALERT_FIXED;
	fixed->len = sizeof(wl_pmalert_fixed_t);
	fixed->prev_pm_dur = wlc->excess_pm_last_pmdur;
	fixed->pm_dur = wlc_get_accum_pmdur(wlc);

	fixed->curr_time = OSL_SYSUPTIME();
	fixed->prev_stats_time = wlc->excess_pm_last_osltime;

	fixed->cal_dur = wlc_phy_get_cal_dur(WLC_PI(wlc));
	fixed->prev_cal_dur = wlc->excess_pmwake->last_cal_dur;
	fixed->prev_frts_dur = wlc->excess_pmwake->last_frts_dur;
#ifdef WLPFN
	fixed->prev_mpc_dur = wlc->excess_pm_last_mpcdur;
	fixed->mpc_dur = wlc_get_mpc_dur(wlc);
#endif // endif
	fixed->hw_macc = R_REG(wlc->osh, &wlc->regs->maccontrol);
#ifdef WLC_LOW
	fixed->sw_macc = wlc->hw->maccontrol;
#endif /* WLC_LOW */
	if (PWRSTATS_ENAB(wlc->pub)) {
		data = wlc_pwrstats_fill_pmalert(wlc, data);
	}
	wlc_get_ucode_dbg(wlc, (wl_pmalert_ucode_dbg_t *)data);
	wlc_generate_pm_alert_event(wlc, reason, pm_alert_data,
		pmalert_len);
	MFREE(wlc->osh, pm_alert_data, pmalert_len);
}

void
wlc_generate_pm_alert_event(wlc_info_t *wlc, uint32 reason, void *data, uint32 datalen)
{
	uint8 temp[MIN_PM_ALERT_LEN];
	wl_pmalert_t *pm_alert = (wl_pmalert_t *) data;
	if (pm_alert == NULL) {
		pm_alert = (wl_pmalert_t *)temp;
		datalen = MIN_PM_ALERT_LEN;
	}
	pm_alert->version = WL_PM_ALERT_VERSION;
	pm_alert->length = datalen;
	pm_alert->reasons = reason;
	wlc_mac_event(wlc, WLC_E_EXCESS_PM_WAKE_EVENT,
		NULL, 0, 0, 0, pm_alert, datalen);
}

uint32
wlc_get_pfn_ms(wlc_info_t *wlc)
{
	uint32 curr_pfn_scan_time = 0;

	/* Calculate awake time due to active pfn scan */
	if (wlc_scan_inprog(wlc)) {
#if defined(WLPFN)
		if (WLPFN_ENAB(wlc->pub)) {
			if (wl_pfn_scan_in_progress(wlc->pfn)) {
				curr_pfn_scan_time = wlc_get_curr_scan_time(wlc);
			}
		}
#endif /* WLPFN */
	}
	return wlc->excess_pmwake->pfn_scan_ms + curr_pfn_scan_time;
}

uint32
wlc_get_roam_ms(wlc_info_t *wlc)
{
	return wlc->excess_pmwake->roam_ms + wlc_curr_roam_scan_time(wlc) +
		wlc_pwrstats_curr_connect_time(wlc->pwrstats);
}

static void
wlc_get_ucode_dbg(wlc_info_t *wlc, wl_pmalert_ucode_dbg_t *ud)
{
	uint32 i;
	d11regs_t *regs = wlc->regs;
	ud->type = WL_PMALERT_UCODE_DBG;
	ud->len = sizeof(wl_pmalert_ucode_dbg_t);
	ud->macctrl = R_REG(wlc->osh, &wlc->regs->maccontrol);
#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		ud->m_p2p_hps = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_HPS);
		for (i = 0; i < MAX_P2P_BSS_DTIM_PRD; i++)
			ud->m_p2p_bss_dtim_prd[i] =
				wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS_DTIM_PRD(i));
	}
#endif /* WLMCNX */
	for (i = 0; i < 20; i++) {
		ud->psmdebug[i] = R_REG(wlc->osh, &regs->psmdebug);
		ud->phydebug[i] = R_REG(wlc->osh, &regs->phydebug);
	}
	ud->psm_brc = R_REG(wlc->osh, &regs->psm_brc);
	ud->ifsstat = R_REG(wlc->osh, &regs->u.d11regs.ifsstat);
}

/* reset epm dur at start of new pm period */
void
wlc_reset_epm_dur(wlc_info_t *wlc)
{
	wlc_excess_pm_wake_info_t *epm = wlc->excess_pmwake;
	wlc->excess_pm_last_osltime = OSL_SYSUPTIME();
#ifdef WLPFN
	wlc->excess_pm_last_mpcdur = wlc_get_mpc_dur(wlc);
#endif // endif
	wlc->excess_pm_last_pmdur = wlc_get_accum_pmdur(wlc);
	if (wlc->clk) {
		/* Note the roam and pfn ts for next pm_wake period */
		epm->last_pm_prd_roam_ts = wlc_get_roam_ms(wlc);
		epm->last_pm_prd_pfn_ts = wlc_get_pfn_ms(wlc);
	}
	wlc_pwrstats_frts_checkpoint(wlc->pwrstats);
	epm->last_frts_dur = wlc_pwrstats_get_frts_data_dur(wlc->pwrstats);
	epm->last_cal_dur = wlc_phy_get_cal_dur(WLC_PI(wlc));
	wlc_pwrstats_copy_event_wake_dur(epm->pp_start_event_dur, wlc->pwrstats);
}

void
wlc_reset_epm_ca(wlc_info_t *wlc)
{
	wlc_excess_pm_wake_info_t *epm = wlc->excess_pmwake;
	epm->last_ca_pmdur = wlc_get_accum_pmdur(wlc);
	epm->last_ca_osl_time = OSL_SYSUPTIME();
	epm->ca_txrxpkts = 0;
	wlc_pwrstats_copy_event_wake_dur(epm->ca_start_event_dur, wlc->pwrstats);
}

/* excess pm constant awake */
static void
wlc_process_epm_ca(wlc_info_t *wlc, uint32 cur_time, uint32 cur_pmdur)
{
	wlc_excess_pm_wake_info_t *epm = wlc->excess_pmwake;

	if (epm->ca_thresh == 0)
		return;

	if (epm->last_ca_pmdur == cur_pmdur) {
		/* const awake thresh exceeded? */
		if (cur_time - epm->last_ca_osl_time >= (epm->ca_thresh * 1000)) {
			if (epm->ca_txrxpkts <= epm->ca_pkts_allowance) {
				/* Trigger recovery on second crossing of threshold */
				if (epm->ca_alert_pmdur_valid &&
					epm->ca_alert_pmdur == cur_pmdur) {
					uint8 pm_backup = wlc->cfg->pm->PM;
					wlc_create_pmalert_data(wlc,
						CONST_AWAKE_DUR_RECOVERY);

					wlc_fatal_error(wlc);

					/* PM mode transition */
					wlc_set_pm_mode(wlc, 0, wlc->cfg);
					wlc_set_pm_mode(wlc, pm_backup, wlc->cfg);
					epm->ca_alert_pmdur = 0;
					epm->ca_alert_pmdur_valid = 0;

				} else {
					/* 1st crossing of threshold results in alert */
					wlc_create_pmalert_data(wlc, CONST_AWAKE_DUR_ALERT);
					epm->ca_alert_pmdur = cur_pmdur;
					epm->ca_alert_pmdur_valid = 1;
				}
			} else {
				epm->ca_alert_pmdur = 0;
				epm->ca_alert_pmdur_valid = 0;
			}

			/* Reset dur to generate next event */
			wlc_reset_epm_ca(wlc);
		}
	} else {
		/* pmdur changed -> !constantly awake */
		wlc_reset_epm_ca(wlc);
		epm->ca_alert_pmdur = 0;
		epm->ca_alert_pmdur_valid = 0;
	}
}

void
wlc_check_excess_pm_awake(wlc_info_t *wlc)
{
	uint32 cur_time, delta_time;
	uint32 cur_pmdur;

	wlc_excess_pm_wake_info_t *epm = wlc->excess_pmwake;
#ifdef WLC_LOW
	bool wasup = wlc->pub->hw_up;

	if (!wasup)
		wlc_corereset(wlc, WLC_USE_COREFLAGS);
#endif /* WLC_LOW */

	cur_time = OSL_SYSUPTIME();
	cur_pmdur = wlc_get_accum_pmdur(wlc);

	if (wlc->stas_associated)
		wlc_process_epm_ca(wlc, cur_time, cur_pmdur);

	delta_time = cur_time - wlc->excess_pm_last_osltime;
	if (wlc->excess_pm_period && delta_time >= (uint32)wlc->excess_pm_period * 1000) {
		uint32 cur_dur, last_dur;

		if (wlc->stas_associated) {
			cur_dur = cur_pmdur;
			last_dur = wlc->excess_pm_last_pmdur;
		} else
#ifdef WLPFN
		{
			cur_dur = wlc_get_mpc_dur(wlc);
			last_dur = wlc->excess_pm_last_mpcdur;
		}
#else
			goto exit;
#endif /* WLPFN */
		if ((cur_dur >= last_dur) &&
			(delta_time >= (cur_dur - last_dur))) {
			uint32 idle_time = delta_time - (cur_dur - last_dur);

			/* Reduce the wake time because of pfn or roam */
			if (epm->pfn_alert_thresh)
				idle_time -= (wlc_get_pfn_ms(wlc) - epm->last_pm_prd_pfn_ts);
			if (epm->roam_alert_thresh)
				idle_time -= (wlc_get_roam_ms(wlc) - epm->last_pm_prd_roam_ts);

			/* Check idle awake time >= excess_pm_percent of sampled period */
			if (idle_time >= ((wlc->excess_pm_percent * delta_time) / 100)) {
				uint32 reason;
				if (wlc->cfg->pm->PM)
					reason = PM_DUR_EXCEEDED;
				else
					/* For mpc mode a separate timer (to trigger at
					 * excess_pm_period) is not started. So the event
					 * (reason code MPC_DUR_EXCEEDED) gets sent
					 * on next chip-wake up. So for e.g. if PFN scan
					 * is scheduled at 30-sec interval,
					 * excess_pm_period = 10sec, excess_pm_percent = 10%.
					 * If the PFN takes more than 1 sec but less than 3 sec
					 * to complete, the event would not  be sent. It did exceed
					 * the 10 percent (i.e. 1sec) if last excess_pm_period is
					 * considered but since we check it on next wakeup i.e.
					 * after 30-seconds, it did not exceed 10 percent of this
					 * interval.
					 */
					reason = MPC_DUR_EXCEEDED;

				wlc_create_pmalert_data(wlc, reason);
			}
		}
		wlc_reset_epm_dur(wlc);
	}
#ifndef WLPFN
exit:
#endif /* WLPFN */

#ifdef WLC_LOW
	if (!wasup)
		wlc_coredisable(wlc->hw);
#endif // endif
	return;
}

void
wlc_check_roam_alert_thresh(wlc_info_t *wlc)
{
	wlc_excess_pm_wake_info_t *epm = wlc->excess_pmwake;

	if (epm->roam_alert_enable) {
		uint32 roam_time_spent = wlc_get_roam_ms(wlc) - epm->roam_alert_thresh_ts;
		if (roam_time_spent > INT32_MAX) {
			WL_ERROR(("roam_time err:rm_ms %d thr_ts %d",
				wlc->excess_pmwake->roam_ms, epm->roam_alert_thresh_ts));
		}
		else if (roam_time_spent > epm->roam_alert_thresh) {
			wlc_generate_pm_alert_event(wlc, ROAM_ALERT_THRESH_EXCEEDED, NULL, 0);
			/* Disable further events */
			epm->roam_alert_enable = FALSE;
		}
	}
}
void
wlc_epm_roam_time_upd(wlc_info_t *wlc, uint32 connect_dur)
{
	wlc->excess_pmwake->roam_ms += connect_dur;
	wlc_check_roam_alert_thresh(wlc);
}
#endif /* WL_EXCESS_PMWAKE */

#endif /* STA */
