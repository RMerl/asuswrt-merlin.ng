/*
 * Common (OS-independent) portion of
 * Broadcom 802.11 Networking Device Driver
 *
 * VHT support
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
 * $Id: wlc_vht.h 684190 2017-02-10 18:37:57Z $
 */

#ifndef _wlc_vht_h_
#define _wlc_vht_h_
#ifdef WL11AC

#include "osl.h"
#include "typedefs.h"
#include "bcmwifi_channels.h"
#include "proto/802.11.h"
#include "wlc_types.h"
#include "wlc_bsscfg.h"

/* Macro Definitions */
#define VHT_AP_DOWNGRADE_BACKOFF	50

#define VHT_OPER_MODE_STATE_PENDING(cfg)		(cfg->opermode_info ?	\
	(cfg->opermode_info->state != VHT_OP_MODE_NOT_PENDING)	\
	: VHT_OP_MODE_NOT_PENDING)

/* module entries */
extern wlc_vht_info_t *wlc_vht_attach(wlc_info_t *wlc);
extern void wlc_vht_detach(wlc_vht_info_t *vhti);
extern void wlc_vht_init_defaults(wlc_vht_info_t *vhti);

/* Update tx and rx mcs maps */
extern void wlc_vht_update_mcs_cap_ext(wlc_vht_info_t *vhti);
extern void wlc_vht_update_mcs_cap(wlc_vht_info_t *vhti);
extern void wlc_vht_update_defaults(wlc_vht_info_t *vhti);

/* IE mgmt */
extern vht_cap_ie_t * wlc_read_vht_cap_ie(wlc_vht_info_t *vhti, uint8 *tlvs, int tlvs_len,
	vht_cap_ie_t* cap_ie);
extern vht_op_ie_t * wlc_read_vht_op_ie(wlc_vht_info_t *vhti, uint8 *tlvs, int tlvs_len,
	vht_op_ie_t* op_ie);
extern uint8 *wlc_read_vht_features_ie(wlc_vht_info_t *vhti,  uint8 *tlvs,
	int tlvs_len, uint8 *rate_mask, int *prop_tlv_len, wlc_bss_info_t *bi);

/* return valid vht chanspec given the op_ie and ht_chanspec */
extern chanspec_t wlc_vht_chanspec(wlc_vht_info_t *vhti, vht_op_ie_t *op_ie,
	chanspec_t ht_chanspec, bool oper_mode_enab, uint8 oper_mode, uint8 ht_ccfs2);
extern void wlc_vht_upd_rate_mcsmap(wlc_vht_info_t *vhti, struct scb *scb);
extern void wlc_vht_update_sgi_rx(wlc_vht_info_t *vhti, uint int_val);
extern void wlc_vht_update_ampdu_cap(wlc_vht_info_t *vhti, uint8 vht_rx_factor);

/* get/set current cap info this node uses */
extern void wlc_vht_set_ldpc_cap(wlc_vht_info_t *vhti, bool enab);

extern void wlc_vht_set_tx_stbc_cap(wlc_vht_info_t *vhti, bool enab);

extern void wlc_vht_set_rx_stbc_cap(wlc_vht_info_t *vhti, int val);

extern uint8 wlc_vht_get_ext_nss_bw_sup(wlc_vht_info_t *vhti);
extern void wlc_vht_set_ext_nss_bw_sup(wlc_vht_info_t *vhti, uint8 val);

extern uint32 wlc_vht_get_cap_info(wlc_vht_info_t *vhti);

/* per SCB functions */
extern void wlc_vht_bcn_scb_upd(wlc_vht_info_t *vhti, int band, struct scb *scb,
	ht_cap_ie_t *ht_cap, vht_cap_ie_t *vht_cap, vht_op_ie_t *vht_op,
	uint8 vht_ratemask);
extern void wlc_vht_update_scb_state(wlc_vht_info_t *vhti, int band, struct scb *scb,
	ht_cap_ie_t *cap_ie, vht_cap_ie_t *vht_cap_ie, vht_op_ie_t *vht_op_ie,
	uint8 vht_ratemask);

/* perform per scb oper mode changes */
extern void wlc_vht_update_scb_oper_mode(wlc_vht_info_t *vhti, struct scb *scb,
	uint8 mode);

/* is oper_mode enabled for this scb */
bool wlc_vht_is_omn_enabled(wlc_vht_info_t *vhti, struct scb *scb);

/* disable scb oper mode */
void wlc_vht_disable_scb_oper_mode(wlc_vht_info_t *vhti, struct scb *scb);

/* get the ratemask corresponding to the scb */
extern uint8 wlc_vht_get_scb_ratemask(wlc_vht_info_t *vhti, struct scb *scb);

/* get the vhtflags corresponding to the scb */
extern uint16 wlc_vht_get_scb_flags(wlc_vht_info_t *vhti, struct scb *scb);
extern uint16 wlc_vht_get_scb_rxmcsmap_sds(wlc_vht_info_t *vhti, struct scb *scb);

/* get oper mode */
extern bool wlc_vht_get_scb_opermode_enab(wlc_vht_info_t *vhti, struct scb *scb);
extern bool wlc_vht_set_scb_opermode_enab(wlc_vht_info_t *vhti, struct scb *scb, bool set);
extern uint8 wlc_vht_get_scb_opermode(wlc_vht_info_t *vhti, struct scb *scb);
extern uint8 wlc_vht_get_scb_ratemask_per_band(wlc_vht_info_t *vhti, struct scb *scb);

/* agg parameters */
extern uint16 wlc_vht_get_scb_amsdu_mtu_pref(wlc_vht_info_t *vhti, struct scb *scb);
extern uint8 wlc_vht_get_scb_ampdu_max_exp(wlc_vht_info_t *vhti, struct scb *scb);
extern void wlc_vht_upd_txbf_cap(wlc_vht_info_t *vhti, uint8 bfr, uint8 bfe, uint32 *cap);
extern void wlc_vht_upd_txbf_virtif_cap(wlc_vht_info_t *vhti, uint8 bfr, uint8 bfe, uint32 *cap);
extern bool wlc_vht_prep_rate_info(wlc_vht_info_t *vhti, wlc_d11rxhdr_t *wrxh,
	uint8 *plcp, ratespec_t rspec, struct wl_rxsts *sts);

/* ht/vht operating mode (11ac) */
extern void wlc_frameaction_vht(wlc_vht_info_t *vhti, uint action_id, struct scb *scb,
struct dot11_management_header *hdr, uint8 *body, int body_len);
extern void wlc_vht_oper_mode_timer(void *arg);
extern void wlc_vht_bss_tbtt(wlc_vht_info_t *vhti, wlc_bsscfg_t *cfg);
extern void wlc_vht_perform_sta_upgrade_downgrade(wlc_vht_info_t *vhti,
	wlc_bsscfg_t *bsscfg);
extern void wlc_vht_perform_ap_upgrade_downgrade(wlc_vht_info_t *vhti,
	wlc_bsscfg_t *bsscfg, chanspec_t chanspec);

extern int wlc_vht_handle_oper_mode_notif_request(wlc_vht_info_t *vhti,
	wlc_bsscfg_t *bsscfg, uint8 oper_mode, uint8 enabled);
extern uint8 wlc_vht_derive_opermode(wlc_vht_info_t *vhti, chanspec_t chanspec,
	uint8 rxstreams);
extern bool wlc_vht_is_downgrade(uint8 oper_mode_old, uint8 oper_mode_new);
extern void wlc_vht_pm_pending_complete(wlc_vht_info_t *vhti, wlc_bsscfg_t *cfg);
extern void wlc_vht_resume_downgrade(wlc_vht_info_t *vhti, wlc_bsscfg_t *cfg);
extern uint32 wlc_vht_get_bw_from_opermode(uint8 oper_mode, vht_op_chan_width_t width);
extern int wlc_vht_change_sta_oper_mode(wlc_vht_info_t *vhti, wlc_bsscfg_t *bsscfg,
	uint8 oper_mode, uint8 enabled);
extern bool wlc_vht_change_ap_oper_mode(wlc_vht_info_t *vhti, wlc_bsscfg_t *bsscfg,
	uint8 oper_mode, uint8 enabled);
extern chanspec_t wlc_vht_find_downgrade_chanspec(wlc_bsscfg_t *cfg,
	uint8 oper_mode_new, uint8 oper_mode_old);
extern chanspec_t wlc_vht_find_upgrade_chanspec(wlc_vht_info_t *vhti, wlc_bsscfg_t *cfg,
	uint8 oper_mode_new, uint8 oper_mode_old);
extern uint8 wlc_vht_get_opermode_from_chspec(wlc_vht_info_t *vhti, chanspec_t chanspec);
extern void wlc_send_action_vht_oper_mode(wlc_vht_info_t *vhti, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *ea, uint8 oper_mode_new);
extern uint16 wlc_vht_get_rx_mcsmap(wlc_vht_info_t *vhti);

#ifdef WLTXMONITOR
extern INLINE void
wlc_vht_txmon_htflags(uint16 phytxctl, uint16 phytxctl1, uint8 *plcp,
	uint16 chan_bw, uint16 chan_band, uint16 chan_num, ratespec_t *rspec,
	struct wl_txsts *sts);

extern INLINE void
wlc_vht_txmon_chspec(uint16 phytxctl, uint16 phytxctl1,
	uint16 chan_band, uint16 chan_num,
	struct wl_txsts *sts, uint16 *chan_bw);
#endif /* WLTXMONITOR */
extern uint8 wlc_vht_get_gid(uint8 *plcp);

#else
/* empty macros to avoid having to use WL11AC flags everywhere */
#define wlc_vht_get_scb_amsdu_mtu_pref(x, y) 0
#define wlc_vht_update_ampdu_cap(x, y)
#define wlc_vht_update_sgi_rx(x, y)
#define wlc_frameaction_vht(a, b, c, d, e, f)
#define wlc_vht_bcn_scb_upd(a, b, c, d, e, f, g)
#define wlc_vht_chanspec(a, b, c, d, e, f)
#define wlc_vht_set_ldpc_cap(x, y)
#define wlc_vht_init_defaults(x)
#define wlc_vht_set_rx_stbc_cap(x, y)
#define wlc_vht_get_ext_nss_bw_sup(x)
#define wlc_vht_set_ext_nss_bw_sup(x, y)
#define wlc_vht_update_mcs_cap(x)
#define wlc_vht_get_cap_info(x) 0
#define wlc_vht_update_mcs_cap(x)
#define wlc_read_vht_cap_ie(a, b, c, d) (NULL)
#define wlc_read_vht_cap_ie(a, b, c, d) (NULL)
#define wlc_read_vht_features_ie(a, b, c, d, e, f) (NULL)
#define wlc_vht_prep_rate_info(a, b, c, d, e) (FALSE)
#define VHT_OPER_MODE_STATE_PENDING(wlc)		FALSE
#define wlc_vht_bss_tbtt(a, b)
#define wlc_vht_pm_pending_complete(a, b)
#endif /* WL11AC */
#endif /* _wlc_vht_h_ */
