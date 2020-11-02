/*
 * Common (OS-independent) portion of
 * Broadcom 802.11 Networking Device Driver
 *
 * Beamforming support
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
 * $Id: wlc_txbf.h 674261 2016-12-07 20:28:42Z $
 */

#ifndef _wlc_txbf_h_
#define _wlc_txbf_h_

#define TXBF_RATE_OFDM_DFT	0	/* disable txbf for all OFDM rate */
#define TXBF_RATE_MCS_DFT	0xFF	/* enable mcs 0-7 */
#define TXBF_RATE_VHT_DFT	0x3FF	/* enable vht 0-9 */

#define TXBF_N_DFT_BFE_CAP (HT_CAP_TXBF_CAP_NDP_RX | \
	(HT_CAP_TXBF_FB_TYPE_IMMEDIATE << HT_CAP_TXBF_CAP_EXPLICIT_C_FB_SHIFT) | \
	(2 << HT_CAP_TXBF_CAP_C_BFR_ANT_SHIFT) | (2 << HT_CAP_TXBF_CAP_CHAN_ESTIM_SHIFT))

#define TXBF_N_BFE_CAP_MASK	(HT_CAP_TXBF_CAP_NDP_RX | \
		HT_CAP_TXBF_CAP_EXPLICIT_C_FB_MASK)

#define TXBF_N_DFT_BFR_CAP (HT_CAP_TXBF_CAP_NDP_TX | HT_CAP_TXBF_CAP_EXPLICIT_C_STEERING)

#define TXBF_N_DFT_CAP ((TXBF_N_DFT_BFE_CAP) | (TXBF_N_DFT_BFR_CAP))

#define TXBF_N_SUPPORTED_BFR(txbf_cap) (((txbf_cap) & \
		(TXBF_N_DFT_BFR_CAP)) == (TXBF_N_DFT_BFR_CAP))
#define TXBF_N_SUPPORTED_BFE(txbf_cap) (((txbf_cap) & (TXBF_N_BFE_CAP_MASK)) == \
		(HT_CAP_TXBF_CAP_NDP_RX | \
		(HT_CAP_TXBF_FB_TYPE_IMMEDIATE << HT_CAP_TXBF_CAP_EXPLICIT_C_FB_SHIFT)))
/* invalid shm index */
#define BF_SHM_IDX_INV 0xFF
#define WLC_TXBF_FLAG_IMPBF 1
#define TXBF_MAX_LINK  8
#define TXBF_MAX_LINK_EXT 8
#define TXBF_MU_MAX_LINKS 8
#define TXBF_MAX_SU_USRIDX_GE64	12	/* max su BFI idx block */

#ifdef WL_BEAMFORMING
#define TXBF_ACTIVE(wlc) (TXBF_ENAB((wlc)->pub) && ((wlc)->txbf->mode) &&((wlc)->txbf->active))
extern bool wlc_txbf_mutx_enabled(wlc_txbf_info_t *txbf);
extern bool wlc_txbf_murx_capable(wlc_txbf_info_t *txbf);
#else
#define TXBF_ACTIVE(wlc) (0)
#define wlc_txbf_mutx_enabled(x) FALSE
#define wlc_txbf_murx_capable(x) FALSE
#endif /* WL_BEAMFORMING */

#ifdef WL_PSMX
extern void wlc_txbf_mutimer_update(wlc_txbf_info_t *txbf, bool force_disable);
#endif // endif

#define TXBF_SU_BFR_CAP 0x01
#define TXBF_MU_BFR_CAP 0x02

#define TXBF_SU_BFE_CAP 0x01
#define TXBF_MU_BFE_CAP 0x02

#define	IS_LEGACY_IMPBF_SUP(corerev) (D11REV_IS(corerev, 42) || D11REV_IS(corerev, 49))

extern wlc_txbf_info_t *wlc_txbf_attach(wlc_info_t *wlc);
extern void wlc_txbf_detach(wlc_txbf_info_t *txbf);
extern void wlc_txbf_init(wlc_txbf_info_t *txbf);
extern void wlc_txbf_delete_link(wlc_txbf_info_t *txbf, struct scb *scb);
extern int wlc_txbf_init_link(wlc_txbf_info_t *txbf, struct scb *scb);
extern void wlc_txbf_txchain_set(wlc_txbf_info_t *txbf);
extern void wlc_txbf_txchain_upd(wlc_txbf_info_t *txbf);
extern void wlc_txbf_rxchain_upd(wlc_txbf_info_t *txbf);
extern void wlc_txfbf_update_amt_idx(wlc_txbf_info_t *txbf, int idx, const struct ether_addr *addr);
extern void wlc_txbf_pkteng_tx_start(wlc_txbf_info_t *txbf, struct scb *scb);
extern void wlc_txbf_pkteng_tx_stop(wlc_txbf_info_t *txbf, struct scb *scb);
extern bool wlc_txbf_sel(wlc_txbf_info_t *txbf, ratespec_t rspec, struct scb *scb, uint8 *shm_index,
	txpwr204080_t* txpwrs);
extern bool wlc_txbf_bfen(wlc_txbf_info_t *txbf, struct scb *scb, ratespec_t rspec,
        txpwr204080_t* txpwrs, bool is_imp);
extern void wlc_txbf_fix_rspec_plcp(wlc_txbf_info_t *txbf, ratespec_t *prspec, uint8 *plcp,
wl_tx_chains_t txbf_chains);
extern void wlc_txbf_clear_init_pending(wlc_txbf_info_t *txbf, struct scb *scb);
extern void wlc_txbf_set_init_pending(wlc_txbf_info_t *txbf, struct scb *scb);
extern bool wlc_txbf_is_init_pending(wlc_txbf_info_t *txbf, struct scb *scb);
extern void wlc_txbf_txpower_target_max_upd(wlc_txbf_info_t *txbf, int8 max_txpwr_limit);
extern void wlc_txbf_applied2ovr_upd(wlc_txbf_info_t *txbf, bool bfen);
extern void wlc_txbf_vht_upd_bfr_bfe_cap(wlc_txbf_info_t *txbf, wlc_bsscfg_t *cfg, uint32 *cap);
extern void wlc_txbf_ht_upd_bfr_bfe_cap(wlc_txbf_info_t *txbf, wlc_bsscfg_t *cfg, uint32 *cap);
extern void wlc_txbf_upd(wlc_txbf_info_t *txbf);
extern void wlc_txbf_impbf_upd(wlc_txbf_info_t *txbf);
extern uint8 wlc_txbf_get_applied2ovr(wlc_txbf_info_t *txbf);
extern void wlc_txbf_imp_txstatus(wlc_txbf_info_t *txbf, struct scb *scb, tx_status_t *txs);
extern uint8 wlc_txbf_get_bfe_sts_cap(wlc_txbf_info_t *txbf, struct scb *scb);
#ifdef WL_MUPKTENG
extern int wlc_txbf_mupkteng_addsta(wlc_txbf_info_t *txbf, struct scb *scb, int idx, int rxchain);
extern int wlc_txbf_mupkteng_clrsta(wlc_txbf_info_t *txbf, struct scb *scb);
#endif // endif
extern bool wlc_txbf_bfmspexp_enable(wlc_txbf_info_t *txbf);
extern bool wlc_txbf_bfrspexp_enable(wlc_txbf_info_t *txbf);
extern uint8 wlc_txbf_get_bfi_idx(wlc_txbf_info_t *txbf, struct scb *scb);
#ifdef WL_PSMX
extern uint8 wlc_txbf_get_mubfi_idx(wlc_txbf_info_t *txbf, struct scb *scb);
extern int wlc_txbf_mu_cap_stas(wlc_txbf_info_t *txbf);
extern uint8 wlc_txbf_get_free_su_bfr_links(wlc_txbf_info_t *txbf);
extern void wlc_txbf_scb_ps_notify(wlc_txbf_info_t *txbf, struct scb *scb, bool ps_on);
#else
#define wlc_txbf_get_mubfi_idx(a, b) BF_SHM_IDX_INV
#define wlc_txbf_scb_ps_notify(a, b, c) do {} while (0)
#endif // endif
extern void wlc_txbf_scb_state_upd(wlc_txbf_info_t *txbf, struct scb *scb, uint cap_info);
extern void wlc_txbf_link_upd(wlc_txbf_info_t *txbf, struct scb *scb);
extern int wlc_txbf_mu_max_links_upd(wlc_txbf_info_t *txbf, uint8 num);
extern bool wlc_txbf_max_mu_link_limit(wlc_txbf_info_t *txbf, wlc_bsscfg_t *bsscfg);
extern void wlc_txbf_chanspec_upd(wlc_txbf_info_t *txbf);
#endif /* _wlc_txbf_h_ */
