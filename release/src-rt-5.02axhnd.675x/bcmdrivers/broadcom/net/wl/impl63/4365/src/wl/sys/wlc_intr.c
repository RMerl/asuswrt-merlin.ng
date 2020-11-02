/*
 * Common (OS-independent) portion of
 * Broadcom 802.11abgn Networking Device Driver
 *
 * @file
 * @brief
 * Interrupt/dpc handlers of common driver. Shared by BMAC driver, High driver,
 * and Full driver.
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
 * $Id: wlc_intr.c 735533 2017-12-11 06:09:19Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc_pio.h>
#include <wlc_rm.h>

/* BMAC_NOTE: a WLC_HIGH compile include of wlc.h adds in more structures and type
 * dependencies. Need to include these to files to allow a clean include of wlc.h
 * with WLC_HIGH defined.
 * At some point we may be able to skip the include of wlc.h and instead just
 * define a stub wlc_info and band struct to allow rpc calls to get the rpc handle.
 */
#include <wlc.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_tx.h>
#include <wlc_mbss.h>
#include <wlc_macdbg.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif
#ifdef BCM_OL_DEV
#include <bcm_ol_msg.h>
#endif // endif
#ifdef WLC_HIGH
#include <wlc_ap.h>
#include <wl_export.h>
#include <wlc_extlog.h>
#include <wlc_11h.h>
#include <wlc_quiet.h>
#include <wlc_hrt.h>
#ifdef WLWNM_AP
#include <wlc_wnm.h>
#endif /* WLWNM_AP */
#endif /* WLC_HIGH */
#ifdef WLDURATION
#include <wlc_duration.h>
#endif // endif
#ifdef	WLAIBSS
#include <wlc_aibss.h>
#endif /* WLAIBSS */
#ifdef WLAWDL
#include <wlc_awdl.h>
#endif // endif

#ifdef WL_BCNTRIM
#include <wlc_bcntrim.h>
#endif // endif

#ifdef WL_AIR_IQ
#include <wlc_airiq.h>
#endif /*  WL_AIR_IQ */

#ifdef WLC_HIGH
#ifndef WLC_PHYTXERR_THRESH
#define WLC_PHYTXERR_THRESH 20
#endif // endif
uint wlc_phytxerr_thresh = WLC_PHYTXERR_THRESH;
#endif /* WLC_HIGH */

#ifdef BCMDBG
static const bcm_bit_desc_t int_flags[] = {
	{MI_MACSSPNDD,	"MACSSPNDD"},
	{MI_BCNTPL,	"BCNTPL"},
	{MI_TBTT,	"TBTT"},
	{MI_BCNSUCCESS,	"BCNSUCCESS"},
	{MI_BCNCANCLD,	"BCNCANCLD"},
	{MI_ATIMWINEND,	"ATIMWINEND"},
	{MI_PMQ,	"PMQ"},
	{MI_ALTTFS,     "MI_ALTTFS"},
	{MI_NSPECGEN_1,	"NSPECGEN_1"},
	{MI_MACTXERR,	"MACTXERR"},
	{MI_PMQERR,	"PMQERR"},
	{MI_PHYTXERR,	"PHYTXERR"},
	{MI_PME,	"PME"},
	{MI_GP0,	"GP0"},
	{MI_GP1,	"GP1"},
	{MI_DMAINT,	"DMAINT"},
	{MI_TXSTOP,	"TXSTOP"},
	{MI_CCA,	"CCA"},
	{MI_BG_NOISE,	"BG_NOISE"},
	{MI_DTIM_TBTT,	"DTIM_TBTT"},
	{MI_PRQ,	"PRQ"},
	{MI_PWRUP,	"PWRUP"},
	{MI_BT_RFACT_STUCK,	"BT_RFACT_STUCK"},
	{MI_BT_PRED_REQ,	"BT_PRED_REQ"},
	{MI_RFDISABLE,	"RFDISABLE"},
	{MI_TFS,	"TFS"},
#ifdef WL_PSMX
	{MI_PSMX,	"PSMX"},
#else
	{MI_PHYCHANGED,	"PHYCHANGED"},
#endif // endif
	{MI_TO,		"TO"},
	{MI_P2P,	"P2P"},
	{MI_HWACI_NOTIFY, "HWACI_NOTIFY"},
	{0, NULL}
};
#endif /* BCMDBG */

#ifndef WLC_HIGH_ONLY
static uint8 wlc_process_per_fifo_intr(wlc_info_t *wlc, bool bounded, wlc_dpc_info_t *dpc);
#if !defined(DMA_TX_FREE)
static uint32 wlc_get_fifo_interrupt(wlc_info_t *wlc, uint8 FIFO);
#endif // endif
#endif /* !WLC_HIGH_ONLY */

#ifdef BCM_OL_DEV
#define WLC_DNGL_OL_NOISE_SET(val)	(ppcie_shared->rssi_info.noise_avg = val)
/* #define WLC_DNGL_OL_NOISE_GET()	(ppcie_shared->.rssi_info.noise_avg) */
#endif // endif

#ifndef WLC_HIGH_ONLY
/* second-level interrupt processing
 *   Return TRUE if another dpc needs to be re-scheduled. FALSE otherwise.
 *   Param 'bounded' indicates if applicable loops should be bounded.
 *   Param 'dpc' returns info such as how many packets have been received/processed.
 */
bool BCMFASTPATH
wlc_dpc(wlc_info_t *wlc, bool bounded, wlc_dpc_info_t *dpc)
#else /* WLC_HIGH_ONLY */
/*
 * This fn has all the high level dpc processing from wlc_dpc.
 * POLICY: no macinstatus change, no bounding loop.
 *         All dpc bounding should be handled in BMAC dpc, like txstatus and rxint
 */
void
wlc_high_dpc(wlc_info_t *wlc, uint32 macintstatus)
#endif /* WLC_HIGH_ONLY */
{
#ifndef WLC_HIGH_ONLY
	uint32 macintstatus;
#if defined(WL_PSMX) && defined(WL_AIR_IQ)
	uint32 macintstatus_x;
#endif // endif
#endif /* WLC_HIGH_ONLY */
	wlc_hw_info_t *wlc_hw = wlc->hw;
	d11regs_t *regs = wlc->regs;
#ifndef WLC_HIGH_ONLY
	bool fatal = FALSE;
#endif // endif

#ifndef WLC_HIGH_ONLY
#ifdef WLC_HIGH
#ifndef MACOSX
	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return FALSE;
	}
#endif /* MACOSX */
#endif /* WLC_HIGH */

	/* grab and clear the saved software intstatus bits */
	macintstatus = wlc_hw->macintstatus;
	if (macintstatus & MI_HWACI_NOTIFY) {
		wlc_phy_hwaci_mitigate_intr(wlc_hw->band->pi);
	}
	wlc_hw->macintstatus = 0;
#endif /* !WLC_HIGH_ONLY */

	WLDURATION_ENTER(wlc, DUR_DPC);

#ifdef BCMDBG
	if (WL_TRACE_ON()) {
		char flagstr[128];

#ifdef WLC_HIGH
		uint unit = wlc->pub->unit;
#else
		uint unit = wlc_hw->unit;
#endif // endif
		bcm_format_flags(int_flags, macintstatus, flagstr, sizeof(flagstr));
		WL_PRINT(("wl%d: %s: macintstatus 0x%x %s\n",
			unit, __FUNCTION__, macintstatus, flagstr));
	}
#endif /* BCMDBG */

#ifndef WLC_HIGH_ONLY
#ifdef WLAWDL
	if (AWDL_ENAB(wlc->pub) && macintstatus & MI_P2P) {
		wlc_p2p_bmac_int_proc(wlc_hw);
		macintstatus &= ~MI_P2P;
	}
#endif // endif
#ifdef STA
	if (macintstatus & MI_BT_PRED_REQ)
		wlc_bmac_btc_update_predictor(wlc_hw);
#endif /* STA */
#endif /* !WLC_HIGH_ONLY */

#ifdef WLC_HIGH
	/* TBTT indication */
	/* ucode only gives either TBTT or DTIM_TBTT, not both */
	if (macintstatus & (MI_TBTT | MI_DTIM_TBTT)) {
#ifdef RADIO_PWRSAVE
		/* Enter power save mode */
		wlc_radio_pwrsave_enter_mode(wlc, ((macintstatus & MI_DTIM_TBTT) != 0));
#endif /* RADIO_PWRSAVE */
#ifdef MBSS
		if (MBSS_ENAB(wlc->pub)) {
			if (MBSS_ENAB16(wlc->pub)) {
				(void)wlc_ap_mbss16_tbtt(wlc, macintstatus);
			} else {
#ifdef WLC_LOW
				/* MBSS4 is only supported for monolithic, non split builds */
				(void)wlc_ap_mbss4_tbtt(wlc, macintstatus);
#else
				/* If this is a split build, we should not be configured for
				 * MBSS4
				 */
				ASSERT(MBSS_ENAB16(wlc->pub));
#endif /* WLC_LOW */
			}
		}
#endif /* MBSS */
		wlc_tbtt(wlc, regs);
	}

#ifdef WLWNM_AP
	if (macintstatus & MI_TTTT) {
		if (AP_ENAB(wlc->pub) && WLWNM_ENAB(wlc->pub)) {
			/* Process TIMBC sendout frame indication */
			wlc_wnm_tttt(wlc);
		}
	}
#endif /* WLWNM_AP */

#ifdef WL_BCNTRIM
	if (WLC_BCNTRIM_ENAB(wlc->pub)) {
		if (macintstatus & MI_BCNTRIM_RX) {
			wlc_bcntrim_recv_process_partial_beacon(wlc->bcntrim);
			macintstatus &= ~MI_BCNTRIM_RX;
		}
	}
#endif // endif

	/* Process probe request FIFO */
	if (macintstatus & MI_PRQ) {
#ifdef MBSS
#if defined(WLC_HIGH) && defined(WLC_LOW)
		if (MBSS_ENAB(wlc->pub)) {
			bool bound = bounded;

			/* Only used by the soft PRQ */
			ASSERT(MBSS_ENAB4(wlc->pub));
			if (!MBSS_ENAB16(wlc->pub) &&
			    wlc_prq_process(wlc, bound)) {
				wlc_hw->macintstatus |= MI_PRQ;
			}
		}
#endif /* WLC_HIGH && WLC_LOW */
#else
		ASSERT(!"PRQ Interrupt in non-MBSS");
#endif /* MBSS */
	}

	/* BCN template is available */
	/* ZZZ: Use AP_ACTIVE ? */
	if (macintstatus & MI_BCNTPL) {
		if (AP_ENAB(wlc->pub) && (!APSTA_ENAB(wlc->pub) || wlc->aps_associated)) {
			WL_APSTA_BCN(("wl%d: wlc_dpc: template -> wlc_update_beacon()\n",
			              wlc->pub->unit));
			wlc_update_beacon(wlc);
		}
	}
#endif /* WLC_HIGH */

#ifndef WLC_HIGH_ONLY

#ifdef BCM_OL_DEV
	/* ATIM tx status on shm */
	if (macintstatus & MI_ALTTFS) {
		if (wlc_bmac_txstatus_shm(wlc_hw, bounded, &fatal))
			wlc_hw->macintstatus |= MI_ALTTFS;
	}
	/*
	 * ************** FIX ME LATER XXX ***************
	 * MI_BG_NOISE interrupt is not enabled by uCode yet!
	 * Once we get this interrupt working then remove commented
	 * if (macintstatus & MI_BG_NOISE)  condition. For now we
	 * calculate Noise for every interrupt. Also, when uCode
	 * enables this interrupt then we need to disable it when we
	 * enter WoWL mode. Re-enabling is not required when system
	 * comes out of WoWL mode as ARM firmware will get reloaded.
	 * ************** FIX ME LATER XXX ***************
	 */
	/* if (macintstatus & MI_BG_NOISE) */ {
		int8 noise_avg = 0;

		/* wlc_phy_noise_sample_intr(wlc_hw->band->pi); */
		noise_avg = wlc_phy_noise_avg(wlc_hw->band->pi);

		WL_TRACE(("******* wl%d: Noise calculated %d\n", wlc_hw->unit, noise_avg));

		WLC_DNGL_OL_NOISE_SET(noise_avg);
	}
#endif /* BCM_OL_DEV */

	/* PMQ entry addition */
	if (macintstatus & MI_PMQ) {
#ifdef AP
		if (wlc_bmac_processpmq(wlc_hw, bounded))
			wlc_hw->macintstatus |= MI_PMQ;
#endif // endif
	}

	/* tx status */
	if (macintstatus & MI_TFS) {
		WLDURATION_ENTER(wlc, DUR_DPC_TXSTATUS);
		if (wlc_bmac_txstatus(wlc_hw, bounded, &fatal)) {
			wlc_hw->macintstatus |= MI_TFS;
		}
		WLDURATION_EXIT(wlc, DUR_DPC_TXSTATUS);
#ifdef WLC_HIGH
		if (fatal) {
			WL_ERROR(("wl%d: %s HAMMERING fatal txs err\n",
				wlc_hw->unit, __FUNCTION__));
			wlc_fatal_error(wlc);
			goto exit;
		}
#endif // endif
	}

	/* ATIM window end */
	if (macintstatus & MI_ATIMWINEND) {
#ifdef	WLAIBSS
		if (AIBSS_ENAB(wlc->pub)) {
			wlc_aibss_atim_window_end(wlc);
		}
#endif /* WLAIBSS */

		/*
		 * OR QValid bits into MACCOMMAND:
		 * - DirFrmQValid set after scrub at TBTT
		 * - BcMcFrmQValid set after BcMc ATIM tx status processed
		 * [XXX - post BcMc ATIM at queue front by swapping]
		 */
		W_REG(wlc->osh, &regs->maccommand, wlc->qvalid);
		wlc->qvalid = 0;
	}

	/* phy tx error */
	if (macintstatus & MI_PHYTXERR) {
		/* WAR for PR37973/37974: PHY RESETS can cause the first byte of a subsequent Tx
		 * frame to be bogus, causing txphy errors - So send a dummy frame as the first
		 * frame after reset
		 */
		if (D11REV_IS(wlc_hw->corerev, 11) || D11REV_IS(wlc_hw->corerev, 12))
			WL_INFORM(("wl%d: PHYTX error\n", wlc_hw->unit));
		/* WAR for 4366C0 Atlas2, series of PHYTX error seen when 5G low and 2.4G are
		 * transmitting and/or receiving simultaneously
		 */
		else if (D11REV_IS(wlc_hw->corerev, 65) && IS_MUTEPHYTXERROR())
			WL_INFORM(("wl%d: PHYTX error\n", wlc_hw->unit));
		else if (D11REV_GE(wlc_hw->corerev, 40))
			WL_PRINT(("wl%d: PHYTX error\n", wlc_hw->unit));
		else
			WL_ERROR(("wl%d: PHYTX error\n", wlc_hw->unit));
		WLCNTINCR(wlc->pub->_cnt->txphyerr);
		wlc_hw->phyerr_cnt++;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#ifdef WLC_HIGH
		wlc_dump_phytxerr(wlc, 0);
#endif /* WLC_HIGH */
#endif /* BCMDBG || BCMDBG_DUMP */
	} else
		wlc_hw->phyerr_cnt = 0;

#ifdef WLC_HIGH
	if (WLCISSSLPNPHY(wlc_hw->band) && (wlc_hw->phyerr_cnt >= wlc_phytxerr_thresh)) {
		WL_ERROR(("wl%d: phyerr thresh exceeded HAMMERING!\n", wlc_hw->unit));
		wlc_fatal_error(wlc);
	}
#endif /* WLC_HIGH */

#ifdef DMATXRC
	/* Opportunistically reclaim tx buffers */
	if (macintstatus & MI_DMATX) {
		/* Reclaim more pkts */
		if (DMATXRC_ENAB(wlc->pub))
			wlc_dmatx_reclaim(wlc_hw);
	}
#endif // endif

#if defined(WL_PSMX) && defined(WL_AIR_IQ)
	if ((macintstatus & MI_PSMX) && (D11REV_GE(wlc_hw->corerev, 65))) {
		macintstatus_x = GET_MACINTSTATUS_X(wlc->osh, regs);
		if (macintstatus_x & MIX_AIRIQ) {
			SET_MACINTSTATUS_X(wlc->osh, regs, MIX_AIRIQ);
			/* Air-IQ data to handle */
			wlc_airiq_vasip_fft_dpc(wlc);
		}
	}
#endif /* WL_PSMX && WL_AIR_IQ */

#ifdef WLRXOV
	if (macintstatus & MI_RXOV) {
		if (WLRXOV_ENAB(wlc->pub))
			wlc_rxov_int(wlc);
	}
#endif // endif

	/* received data or control frame, MI_DMAINT is indication of RX_FIFO interrupt */
	if (macintstatus & MI_DMAINT) {
#ifdef WLC_HIGH
		if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
		    wlc_hw->dma_lpbk) {
			wlc_recover_pkts(wlc_hw->wlc, TX_DATA_FIFO);
			dma_rxfill(wlc_hw->di[RX_FIFO]);
		} else
#endif /* WLC_HIGH */
		{
		WLDURATION_ENTER(wlc, DUR_DPC_RXFIFO);
		if (BCMSPLITRX_ENAB()) {
			if (wlc_process_per_fifo_intr(wlc, bounded, dpc)) {
				wlc_hw->macintstatus |= MI_DMAINT;
			}
		} else {
			if (wlc_bmac_recv(wlc_hw, RX_FIFO, bounded, dpc)) {
				wlc_hw->macintstatus |= MI_DMAINT;
			}
		}
		WLDURATION_EXIT(wlc, DUR_DPC_RXFIFO);
		}
	}
#endif /* !WLC_HIGH_ONLY */

#ifdef WLC_HIGH
	/* TX FIFO suspend/flush completion */
	if (macintstatus & MI_TXSTOP) {
		if (wlc_bmac_tx_fifo_suspended(wlc_hw, TX_DATA_FIFO)) {
			wlc_txstop_intr(wlc);
		}
	}
#endif /* WLC_HIGH */

#ifndef WLC_HIGH_ONLY
	/* noise sample collected */
	if (macintstatus & MI_BG_NOISE) {
		WL_NONE(("wl%d: got background noise samples\n", wlc_hw->unit));
		wlc_phy_noise_sample_intr(wlc_hw->band->pi);
	}

#if defined(STA) && defined(WLRM)
	if (WLRM_ENAB(wlc->pub) && macintstatus & MI_CCA) { /* CCA measurement complete */
		WL_INFORM(("wl%d: CCA measurement interrupt\n", wlc_hw->unit));
		wlc_bmac_rm_cca_int(wlc_hw);
	}
#endif // endif
#endif /* !WLC_HIGH_ONLY */

#ifdef WLC_HIGH
	if (macintstatus & MI_GP0) {
		WL_PRINT(("wl%d: PSM microcode watchdog fired at %d (seconds). Resetting.\n",
			wlc->pub->unit, wlc->pub->now));
		/* BMAC_NOTE: maybe make this a WLC_HIGH_API ? */
		wlc_dump_ucode_fatal(wlc, PSM_FATAL_PSMWD);
		if (!((CHIPID(wlc->pub->sih->chip) == BCM4321_CHIP_ID) &&
		      (CHIPREV(wlc->pub->sih->chiprev) == 0))) {
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
				0, R_REG(wlc->osh, &regs->psmdebug), "psmdebug");
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
				0, R_REG(wlc->osh, &regs->phydebug), "phydebug");
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
				0, R_REG(wlc->osh, &regs->psm_brc), "psm_brc");
			ASSERT(!"PSM Watchdog");
		}

		WLCNTINCR(wlc->pub->_cnt->psmwds);

#ifndef BCMNODOWN
		if (g_assert_type == 3) {
			/* bring the driver down without resetting hardware */
			wlc->down_override = TRUE;
			wlc->psm_watchdog_debug = TRUE;
			wlc_out(wlc);

			/* halt the PSM */
			wlc_bmac_mctrl(wlc_hw, MCTL_PSM_RUN, 0);
			wlc->psm_watchdog_debug = FALSE;
			goto out;
		}
		else
#endif /* BCMNODOWN */
		{
#if !defined(WLC_HOSTPMAC)
			WL_ERROR(("wl%d: %s HAMMERING: MI_GP0 set\n",
				wlc->pub->unit, __FUNCTION__));
			/* big hammer */
			wlc_fatal_error(wlc);
#endif // endif
		}
	}
#ifdef WAR4360_UCODE
	if (wlc_hw->need_reinit) {
		/* big hammer */
		WL_ERROR(("wl%d: %s: need to reinit() %d, Big Hammer\n",
			wlc->pub->unit, __FUNCTION__, wlc_hw->need_reinit));
		wlc->pub->_cnt->reinitreason[REINITREASONIDX(wlc->hw->need_reinit)]++;
		wlc_hw->need_reinit = 0;
		wlc_fatal_error(wlc);
	}
#endif /* WAR4360_UCODE */
	/* gptimer timeout */
	if (macintstatus & MI_TO) {
		wlc_hrt_isr(wlc->hrti);
	}

#ifdef STA
	if (macintstatus & MI_RFDISABLE) {
		wlc_rfd_intr(wlc);
	}
#endif /* STA */
#endif /* WLC_HIGH */

#ifndef WLC_HIGH_ONLY
	if (macintstatus & MI_P2P)
		wlc_p2p_bmac_int_proc(wlc_hw);
#endif /* !WLC_HIGH_ONLY */

#ifdef WLC_HIGH
	/* send any enq'd tx packets. Just makes sure to jump start tx */
	if (WLC_TXQ_OCCUPIED(wlc)) {
		wlc_send_q(wlc, wlc->active_queue);
	}

#ifdef WLC_HIGH_ONLY
#else
	ASSERT(wlc_ps_check(wlc));
#endif /* WLC_HIGH_ONLY */
#endif	/* WLC_HIGH */

#ifdef WLC_LOW_ONLY
#define MACINTMASK_BMAC (MI_TFS | MI_ATIMWINEND | MI_PHYTXERR | MI_DMAINT | MI_BG_NOISE | \
			 MI_CCA | MI_GP0 | MI_PMQ | MI_P2P | MI_ALTTFS | MI_HWACI_NOTIFY | \
			 0)
	macintstatus &= ~MACINTMASK_BMAC;
	if (macintstatus != 0) {
		wlc_high_dpc(wlc, macintstatus);
	}
#endif /* WLC_LOW_ONLY */

#if defined(BCMDBG) && !defined(WLC_LOW_ONLY)
	wlc_update_perf_stats(wlc, WLC_PERF_STATS_DPC);
#endif // endif

#ifdef WLOFFLD
	if (WLOFFLD_CAP(wlc))
		wlc_ol_dpc(wlc->ol);
#endif // endif

#if defined(WLC_HIGH) && !defined(BCMNODOWN)
out:
	;
#endif /* WLC_HIGH && !BCMNODOWN */

#ifndef WLC_HIGH_ONLY
	/* make sure the bound indication and the implementation are in sync */
	ASSERT(bounded == TRUE || wlc_hw->macintstatus == 0);

#ifdef WLC_HIGH
exit:
#endif // endif
	WLDURATION_EXIT(wlc, DUR_DPC);
	/* it isn't done and needs to be resched if macintstatus is non-zero */
	return (wlc_hw->macintstatus != 0);
#endif /* !WLC_HIGH_ONLY */
}

#ifndef WLC_HIGH_ONLY
/*
 * Read and clear macintmask and macintstatus and intstatus registers.
 * This routine should be called with interrupts off
 * Return:
 *   -1 if DEVICEREMOVED(wlc) evaluates to TRUE;
 *   0 if the interrupt is not for us, or we are in some special cases;
 *   device interrupt status bits otherwise.
 */
static INLINE uint32
wlc_intstatus(wlc_info_t *wlc, bool in_isr)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	d11regs_t *regs = wlc_hw->regs;
	uint32 macintstatus, mask;
	uint32 intstatus_rxfifo, intstatus_txsfifo;
	osl_t *osh;

	osh = wlc_hw->osh;

	/* macintstatus includes a DMA interrupt summary bit */
	macintstatus = GET_MACINTSTATUS(osh, regs);

	WL_TRACE(("wl%d: macintstatus: 0x%x\n", wlc_hw->unit, macintstatus));

#ifndef MACOSX
	/* detect cardbus removed, in power down(suspend) and in reset */
	if (DEVICEREMOVED(wlc))
		return -1;
#endif // endif

	/* DEVICEREMOVED succeeds even when the core is still resetting,
	 * handle that case here.
	 */
	if (macintstatus == 0xffffffff) {
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		return 0;
	}

	mask = (in_isr ? wlc_hw->macintmask : wlc_hw->defmacintmask);

	mask |= wlc_hw->delayedintmask;

	/* defer unsolicited interrupts */
	macintstatus &= mask;

	/* if not for us */
	if (macintstatus == 0)
		return 0;

#ifdef WLOFFLD
	/* Disable PCIE MB interrupt also until DPC is run */
	if (WLOFFLD_CAP(wlc))
		wlc_ol_enable_intrs(wlc->ol, FALSE);
#endif // endif
	/* interrupts are already turned off for CFE build
	 * Caution: For CFE Turning off the interrupts again has some undesired
	 * consequences
	 */
#if !defined(_CFE_)
	/* turn off the interrupts */
	/*
	 * WAR for PR 39712
	 * clear the software/hardware intmask, before clearing macintstatus
	 */
	SET_MACINTMASK(osh, regs, 0);
#ifndef BCMSDIO
	(void)GET_MACINTMASK(osh, regs);	/* sync readback */
#endif // endif
	wlc_hw->macintmask = 0;
#endif /* !defined(_CFE_) */

#ifdef WLC_HIGH
	/* BMAC_NOTE: this code should not be in the ISR code, it should be in DPC.
	 * We can not modify wlc state except for very restricted items.
	 */
	if ((macintstatus & MI_BT_RFACT_STUCK) && D11REV_LT(wlc_hw->corerev, 15)) {
		/* disable BT Coexist and WAR detection when stuck pin is detected
		 * This has to be done before clearing the macintstatus
		 */
		WL_ERROR(("Disabling BTC mode (BT_RFACT stuck detected)\n"));
		wlc_hw->btc->stuck_detected = TRUE;
		wlc_bmac_btc_mode_set(wlc_hw, WL_BTC_DISABLE);
		wlc_bmac_btc_stuck_war50943(wlc_hw, FALSE);
	}
#endif /* WLC_HIGH */

	/* clear device interrupts */
	SET_MACINTSTATUS(osh, regs, macintstatus);

	/* MI_DMAINT is indication of non-zero intstatus */
	if (macintstatus & MI_DMAINT) {
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			intstatus_rxfifo = R_REG(osh, &regs->intctrlregs[RX_FIFO].intstatus);
			intstatus_txsfifo = R_REG(osh,
			                          &regs->intctrlregs[RX_TXSTATUS_FIFO].intstatus);
			WL_TRACE(("wl%d: intstatus_rxfifo 0x%x, intstatus_txsfifo 0x%x\n",
			          wlc_hw->unit, intstatus_rxfifo, intstatus_txsfifo));

			/* defer unsolicited interrupt hints */
			intstatus_rxfifo &= DEF_RXINTMASK;
			intstatus_txsfifo &= DEF_RXINTMASK;

			/* MI_DMAINT bit in macintstatus is indication of RX_FIFO interrupt */
			/* clear interrupt hints */
			if (intstatus_rxfifo)
				W_REG(osh,
				      &regs->intctrlregs[RX_FIFO].intstatus, intstatus_rxfifo);
			else
				macintstatus &= ~MI_DMAINT;

			/* MI_TFS bit in macintstatus is encoding of RX_TXSTATUS_FIFO interrupt */
			if (intstatus_txsfifo) {
				W_REG(osh, &regs->intctrlregs[RX_TXSTATUS_FIFO].intstatus,
				      intstatus_txsfifo);
				macintstatus |= MI_TFS;
			}
#if defined(DMA_TX_FREE)
			/* do not support DMA_TX_FREE for corerev 4 */
			ASSERT(0);
#endif // endif
		} else {
#if !defined(DMA_TX_FREE)
			/*
			 * For corerevs >= 5, only fifo interrupt enabled is I_RI in RX_FIFO.
			 * If MI_DMAINT is set, assume it is set and clear the interrupt.
			 */
			if (BCMSPLITRX_ENAB()) {
				if (wlc_get_fifo_interrupt(wlc, RX_FIFO)) {
					/* check for fif0-0 intr */
					wlc_hw->dma_rx_intstatus |= RX_INTR_FIFO_0;
					W_REG(osh, &regs->intctrlregs[RX_FIFO].intstatus,
						DEF_RXINTMASK);
				}
				if (PKT_CLASSIFY_EN(RX_FIFO1) || RXFIFO_SPLIT()) {
					if (wlc_get_fifo_interrupt(wlc, RX_FIFO1)) {
					/* check for fif0-1 intr */
					wlc_hw->dma_rx_intstatus |= RX_INTR_FIFO_1;
						W_REG(osh, &regs->intctrlregs[RX_FIFO1].intstatus,
							DEF_RXINTMASK);
					}
				}
				if (PKT_CLASSIFY_EN(RX_FIFO2)) {
					if (wlc_get_fifo_interrupt(wlc, RX_FIFO2)) {
						/* check for fif0-2 intr */
						wlc_hw->dma_rx_intstatus |= RX_INTR_FIFO_2;
						W_REG(osh, &regs->intctrlregs[RX_FIFO2].intstatus,
							DEF_RXINTMASK);
					}
				}
			} else {
				W_REG(osh, &regs->intctrlregs[RX_FIFO].intstatus,
					DEF_RXINTMASK);
			}

#ifdef DMATXRC
			if (D11REV_GE(wlc_hw->corerev, 40) && DMATXRC_ENAB(wlc->pub)) {
				if (R_REG(osh, &regs->intctrlregs[TX_DATA_FIFO].intstatus) & I_XI) {
					/* Clear it */
					W_REG(osh, &regs->intctrlregs[TX_DATA_FIFO].intstatus,
						I_XI);

					/* Trigger processing */
					macintstatus |= MI_DMATX;
				}
			}
#endif // endif

#else
			int i;

			/* Clear rx and tx interrupts */
			/* When cut through DMA enabled, we use the other indirect indaqm DMA
			 * channel registers instead of legacy intctrlregs on TX direction.
			 * The descriptor in aqm_di contains the corresponding txdata DMA
			 * descriptors information which must be filled in tx.
			 */
			if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
				for (i = 0; i < wlc_hw->nfifo_inuse; i++) {
					if (i == RX_FIFO) {
						W_REG(osh, &regs->intctrlregs[i].intstatus,
							DEF_RXINTMASK);
						dma_set_indqsel(wlc_hw->aqm_di[i], FALSE);
						W_REG(osh, &regs->indaqm.indintstatus, I_XI);
					}
					else if (wlc_hw->aqm_di[i] &&
						dma_txactive(wlc_hw->aqm_di[i])) {
						dma_set_indqsel(wlc_hw->aqm_di[idx], FALSE);
						W_REG(osh, &regs->indaqm.indintstatus, I_XI);
					}
				}
			}
			else {
				for (i = 0; i < NFIFO; i++) {
					if (i == RX_FIFO) {
						W_REG(osh, &regs->intctrlregs[i].intstatus,
							DEF_RXINTMASK | I_XI);
					}
					else if (wlc_hw->di[i] && dma_txactive(wlc_hw->di[i])) {
						W_REG(osh, &regs->intctrlregs[i].intstatus, I_XI);
					}
				}
			}
#endif /* DMA_TX_FREE */
		}
	}

#ifdef DMATXRC
	/* Opportunistically reclaim tx buffers */
	if (macintstatus & MI_DMATX) {
		/* Reclaim pkts immediately */
		if (DMATXRC_ENAB(wlc->pub))
			wlc_dmatx_reclaim(wlc_hw);
	}
#endif // endif

	return macintstatus;
}

/* Update wlc_hw->macintstatus and wlc_hw->intstatus[]. */
/* Return TRUE if they are updated successfully. FALSE otherwise */
bool
wlc_intrsupd(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint32 macintstatus;

	ASSERT(wlc_hw->macintstatus != 0);

	/* read and clear macintstatus and intstatus registers */
	macintstatus = wlc_intstatus(wlc, FALSE);

	/* device is removed */
	if (macintstatus == 0xffffffff) {
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		return FALSE;
	}

#if defined(WLC_HIGH) && defined(MBSS)
	if (MBSS_ENAB(wlc->pub) &&
	    (macintstatus & (MI_DTIM_TBTT | MI_TBTT))) {
		/* Record latest TBTT/DTIM interrupt time for latency calc */
		wlc->mbss->last_tbtt_us = R_REG(wlc_hw->osh, &wlc_hw->regs->tsf_timerlow);
	}
#endif /* WLC_HIGH && MBSS */

	/* update interrupt status in software */
	wlc_hw->macintstatus |= macintstatus;

	return TRUE;
}

/*
 * First-level interrupt processing.
 * Return TRUE if this was our interrupt, FALSE otherwise.
 * *wantdpc will be set to TRUE if further wlc_dpc() processing is required,
 * FALSE otherwise.
 */
bool BCMFASTPATH
wlc_isr(wlc_info_t *wlc, bool *wantdpc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint32 macintstatus;

	*wantdpc = FALSE;

	if (!wlc_hw->up || !wlc_hw->macintmask)
		return (FALSE);

	/* read and clear macintstatus and intstatus registers */
	macintstatus = wlc_intstatus(wlc, TRUE);

#ifdef WLOFFLD
	if (WLOFFLD_CAP(wlc)) {
		if ((macintstatus == 0) && wlc_ol_intstatus(wlc->ol)) {
			/* Disable MAC interrupts until DPC is run */
			wlc_intrsoff(wlc);
			*wantdpc = TRUE;
			return TRUE;
		}
	}
#endif // endif

	if (macintstatus == 0xffffffff) {
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		WL_ERROR(("DEVICEREMOVED detected in the ISR code path.\n"));
		/* in rare cases, we may reach this condition as a race condition may occur */
		/* between disabling interrupts and clearing the SW macintmask */
		/* clear mac int status as there is no valid interrupt for us */
		wlc_hw->macintstatus = 0;
		/* assume this is our interrupt as before; note: want_dpc is FALSE */
		return (TRUE);
	}

	/* it is not for us */
	if (macintstatus == 0)
		return (FALSE);

	*wantdpc = TRUE;

#ifdef WLC_HIGH
#if defined(MBSS)
	/* BMAC_NOTE: This mechanism to bail out of sending beacons in
	 * wlc_ap.c:wlc_ap_sendbeacons() does not seem like a good idea across a bus with
	 * non-negligible reg-read time. The time values in the code have
	 * wlc_ap_sendbeacons() bail out if the delay between the isr time and it is >
	 * 4ms. But the check is done in wlc_ap_sendbeacons() with a reg read that might
	 * take on order of 0.3ms.  Should mbss only be supported with beacons in
	 * templates instead of beacons from host driver?
	 */
	if (MBSS_ENAB(wlc->pub) &&
	    (macintstatus & (MI_DTIM_TBTT | MI_TBTT))) {
		/* Record latest TBTT/DTIM interrupt time for latency calc */
		wlc->mbss->last_tbtt_us = R_REG(wlc_hw->osh, &wlc_hw->regs->tsf_timerlow);
	}
#endif /* MBSS */
#endif	/* WLC_HIGH */

	/* save interrupt status bits */
	ASSERT(wlc_hw->macintstatus == 0);
	wlc_hw->macintstatus = macintstatus;

#if defined(BCMDBG) && !defined(WLC_LOW_ONLY)
	wlc_update_isr_stats(wlc, macintstatus);
#endif // endif
	return (TRUE);

}

void
wlc_intrson(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;

	ASSERT(wlc_hw->defmacintmask);
	wlc_hw->macintmask = wlc_hw->defmacintmask;
	SET_MACINTMASK(wlc_hw->osh, wlc_hw->regs, wlc_hw->macintmask);
#ifdef WLOFFLD
	if (WLOFFLD_CAP(wlc)) {
	/* Enable PCIE MB interrupt */
		wlc_ol_enable_intrs(wlc->ol, TRUE);
	}
#endif // endif
}

/* mask off interrupts */
#define SET_MACINTMASK_OFF(osh, regs) { \
	SET_MACINTMASK(osh, regs, 0); \
	(void)GET_MACINTMASK(osh, regs);	/* sync readback */ \
	OSL_DELAY(1); /* ensure int line is no longer driven */ \
}

uint32
wlc_intrsoff(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint32 macintmask;

	if (!wlc_hw->clk)
		return 0;

	macintmask = wlc_hw->macintmask;	/* isr can still happen */
	SET_MACINTMASK_OFF(wlc_hw->osh, wlc_hw->regs);
	wlc_hw->macintmask = 0;

	/* return previous macintmask; resolve race between us and our isr */
	return (wlc_hw->macintstatus ? 0 : macintmask);
}

/* deassert interrupt */
void
wlc_intrs_deassert(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;

	if (!wlc_hw->clk)
		return;

	SET_MACINTMASK_OFF(wlc_hw->osh, wlc_hw->regs);
}

void
wlc_intrsrestore(wlc_info_t *wlc, uint32 macintmask)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;

	if (!wlc_hw->clk)
		return;

	wlc_hw->macintmask = macintmask;
	SET_MACINTMASK(wlc_hw->osh, wlc_hw->regs, wlc_hw->macintmask);
}

/* Read per fifo interrupt and process the frame if any interrupt is present */
static uint8
wlc_process_per_fifo_intr(wlc_info_t *wlc, bool bounded, wlc_dpc_info_t *dpc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint8 fifo_status = wlc_hw->dma_rx_intstatus;
	/* reset sw interrup bit */
	wlc_hw->dma_rx_intstatus = 0;
	/* process fifo- 0 */
	if (fifo_status & RX_INTR_FIFO_0) {
		if (wlc_bmac_recv(wlc_hw, RX_FIFO, bounded, dpc)) {
			/* More frames to be processed */
			wlc_hw->dma_rx_intstatus |= RX_INTR_FIFO_0;
		}
	}
	/* process fifo- 1 */
	if (PKT_CLASSIFY_EN(RX_FIFO1) || RXFIFO_SPLIT()) {
			if (fifo_status & RX_INTR_FIFO_1) {
				if (wlc_bmac_recv(wlc_hw, RX_FIFO1, bounded, dpc)) {
				/* More frames to be processed */
				wlc_hw->dma_rx_intstatus |= RX_INTR_FIFO_1;
				}
			}
	}
	/* Process fifo-2 */
	if (PKT_CLASSIFY_EN(RX_FIFO2)) {
		if (fifo_status & RX_INTR_FIFO_2) {
			if (wlc_bmac_recv(wlc_hw, RX_FIFO2, bounded, dpc)) {
				/* More frames to be processed */
				wlc_hw->dma_rx_intstatus |= RX_INTR_FIFO_2;
			}
		}
	}
	return wlc_hw->dma_rx_intstatus;
}

#if !defined(DMA_TX_FREE)
/* Return intstaus fof fifo-x */
static uint32
wlc_get_fifo_interrupt(wlc_info_t *wlc, uint8 FIFO)
{
	d11regs_t *regs = wlc->regs;
	return (R_REG(wlc->osh, &regs->intctrlregs[FIFO].intstatus) &
		DEF_RXINTMASK);
}
#endif /* !DMA_TX_FREE */

#endif /* !WLC_HIGH_ONLY */
