/*
 * MCHAN related header file
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
 * $Id: wlc_mchan.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_mchan_h_
#define _wlc_mchan_h_

#ifdef WLMCHAN

extern bool wlc_mchan_stago_is_disabled(mchan_info_t *mchan);
extern mchan_info_t *wlc_mchan_attach(wlc_info_t *wlc);
extern void wlc_mchan_detach(mchan_info_t *mchan);
extern uint16 wlc_mchan_get_pretbtt_time(mchan_info_t *mchan);
extern void wlc_mchan_recv_process_beacon(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, uint8 *body, int bcn_len);
extern int wlc_mchan_create_bss_chan_context(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	chanspec_t chanspec);
extern int wlc_mchan_delete_bss_chan_context(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern int wlc_mchan_update_bss_chan_context(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	chanspec_t chanspec, bool create);
extern void wlc_mchan_abs_proc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 tsf_l);
extern void wlc_mchan_psc_proc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 tsf_l);
extern void wlc_mchan_pm_pending_complete(mchan_info_t *mchan);
extern void wlc_mchan_client_noa_clear(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
#if defined(PROP_TXSTATUS)
extern int wlc_wlfc_mchan_interface_state_update(wlc_info_t *wlc,
	wlc_bsscfg_t *bsscfg,
	uint8 open_close, bool force_open);
#endif /* PROP_TXSTATUS */

extern 	wlc_bsscfg_t *wlc_mchan_get_cfg_frm_q(wlc_info_t *wlc, wlc_txq_info_t *qi);
extern 	wlc_bsscfg_t *wlc_mchan_get_other_cfg_frm_q(wlc_info_t *wlc, wlc_txq_info_t *qi);

void wlc_mchan_set_priq(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
void wlc_mchan_set_pmpending(mchan_info_t *mchan, wlc_bsscfg_t *cfg, bool pmpending);
void wlc_mchan_update_pmpending(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
bool _wlc_mchan_ovlp_chan(mchan_info_t *mchan, wlc_bsscfg_t *cfg, chanspec_t chspec,
	uint chbw);
bool wlc_mchan_ovlp_chan(mchan_info_t *mchan, wlc_bsscfg_t *cfg1, wlc_bsscfg_t *cfg2,
	uint chbw);
bool _wlc_mchan_same_chan(mchan_info_t *mchan, wlc_bsscfg_t *cfg, chanspec_t chspec);
bool wlc_mchan_shared_chanctx(mchan_info_t *mchan, wlc_bsscfg_t *cfg1, wlc_bsscfg_t *cfg2);
chanspec_t wlc_mchan_current_chanspec(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
bool wlc_mchan_has_chanctx(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
void wlc_mchan_config_go_chanspec(mchan_info_t *mchan, wlc_bsscfg_t *cfg, chanspec_t chspec);
chanspec_t wlc_mchan_configd_go_chanspec(mchan_info_t *mchan, wlc_bsscfg_t *cfg);
bool wlc_mchan_ap_tbtt_setup(wlc_info_t *wlc, wlc_bsscfg_t *ap_cfg);
#ifdef WLTDLS
extern void wlc_mchan_stop_tdls_timer(mchan_info_t *mchan);
extern void wlc_mchan_start_tdls_timer(mchan_info_t *mchan, wlc_bsscfg_t *parent,
	struct scb *scb, bool force);
#endif // endif
#else	/* stubs */
#define wlc_mchan_attach(a) (mchan_info_t *)0x0dadbeef
#define	wlc_mchan_detach(a) do {} while (0)
#endif /* WLMCHAN */

#endif /* _wlc_mchan_h_ */
