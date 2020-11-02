/*
 * A-MPDU (with extended Block Ack) related header file
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
 * $Id: wlc_ampdu_cmn.h 713769 2017-08-01 13:19:42Z $
*/

#ifndef _wlc_ampdu_ctl_h_
#define _wlc_ampdu_ctl_h_

extern int wlc_ampdu_init(wlc_info_t *wlc);
extern void wlc_ampdu_deinit(wlc_info_t *wlc);

extern void scb_ampdu_cleanup(wlc_info_t *wlc, struct scb *scb);
extern void scb_ampdu_cleanup_all(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
extern void wlc_frameaction_ampdu(wlc_info_t *wlc, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, int body_len);

extern void wlc_scb_ampdu_enable(wlc_info_t *wlc, struct scb *scb);
extern void wlc_scb_ampdu_disable(wlc_info_t *wlc, struct scb *scb);

#define AMPDU_MAX_SCB_TID	(NUMPRIO)	/* max tid; currently 8; future 16 */
#define AMPDU_ALL_TID_BITMAP	NBITMASK(AMPDU_MAX_SCB_TID)

#define AMPDU_MAX_MCS 31                        /* we don't deal with mcs 32 */
#define AMPDU_MAX_VHT 48			/* VHT rate 0-9 + prop 10-11, 4 streams for now */

#ifdef BCMDBG
#define AMPDUSCBCNTADD(cnt, upd) ((cnt) += (upd))
#define AMPDUSCBCNTINCR(cnt) ((cnt)++)
#else
#define AMPDUSCBCNTADD(a, b) do { } while (0)
#define AMPDUSCBCNTINCR(a)  do { } while (0)
#endif // endif

#define AMPDU_VALIDATE_TID(ampdu, tid, str) \
	if (tid >= AMPDU_MAX_SCB_TID) { \
		WL_ERROR(("wl%d: %s: invalid tid %d\n", ampdu->wlc->pub->unit, str, tid)); \
		WLCNTINCR(ampdu->cnt->rxunexp); \
		return; \
	}

/** Macro's to deal with noncontiguous range of MCS values */
#if defined(WLPROPRIETARY_11N_RATES)
#define AMPDU_N_11N_MCS		(AMPDU_MAX_MCS + 1 + WLC_11N_N_PROP_MCS)
#define MCS2IDX(mcs)	(mcs2idx(mcs))
#define NEXT_MCS(mcs) ((mcs) != AMPDU_MAX_MCS ? (mcs) + 1 : WLC_11N_FIRST_PROP_MCS) /* iterator */
#define AMPDU_HT_MCS_ARRAY_SIZE (AMPDU_N_11N_MCS + 1) /* one extra for mcs-32 */
#else
#define AMPDU_N_11N_MCS		(AMPDU_MAX_MCS + 1)
#define MCS2IDX(mcs)	(mcs)
#define AMPDU_HT_MCS_ARRAY_SIZE AMPDU_N_11N_MCS
#endif /* WLPROPRIETARY_11N_RATES */

#define AMPDU_HT_MCS_LAST_EL	(AMPDU_HT_MCS_ARRAY_SIZE - 1) /* last element used as 'common' */

extern int wlc_send_addba_req(wlc_info_t *wlc, struct scb *scb, uint8 tid, uint16 wsize,
	uint8 ba_policy, uint8 delba_timeout);
extern void *wlc_send_bar(wlc_info_t *wlc, struct scb *scb, uint8 tid,
	uint16 start_seq, uint16 cf_policy, bool enq_only, bool *blocked);
extern int wlc_send_delba(wlc_info_t *wlc, struct scb *scb, uint8 tid, uint16 initiator,
	uint16 reason);

extern void wlc_ampdu_recv_ctl(wlc_info_t *wlc, struct scb *scb, uint8 *body,
	int body_len, uint16 fk);
extern void wlc_ampdu_recv_delba(wlc_info_t *wlc, struct scb *scb,
	uint8 *body, int body_len);
extern void wlc_ampdu_recv_addba_req(wlc_info_t *wlc, struct scb *scb,
	uint8 *body, int body_len);

extern void wlc_ampdu_agg_state_update_all(wlc_info_t *wlc, bool aggr);

#if defined(WLPROPRIETARY_11N_RATES)
extern uint8 mcs2idx(uint mcs); /* maps mcs to array index for arrays[AMPDU_N_11N_MCS] */
#endif /* WLPROPRIETARY_11N_RATES */
#ifdef WL11K_ALL_MEAS
extern void wlc_ampdu_get_stats(wlc_info_t *wlc, rrm_stat_group_12_t *g12);
extern uint32 wlc_ampdu_getstat_rxampdu(wlc_info_t *wlc);
extern uint32 wlc_ampdu_getstat_rxmpdu(wlc_info_t *wlc);
extern uint32 wlc_ampdu_getstat_rxampdubyte_h(wlc_info_t *wlc);
extern uint32 wlc_ampdu_getstat_rxampdubyte_l(wlc_info_t *wlc);
extern uint32 wlc_ampdu_getstat_ampducrcfail(wlc_info_t *wlc);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
#if defined(EVENT_LOG_COMPILE)
extern int wlc_ampdu_stats_e_report(wlc_info_t *wlc, uint16 tag, uint16 type, uint16 len,
	uint32 *counters, bool ec);
extern void wlc_ampdu_stats_range(uint32 *stats, int max_counters, int *first, int *last);
#endif /* EVENT_LOG_COMPILE */
#endif /* BCMDBG || BCMDBG_DUMP || WLTEST || BCMDBG_AMPDU */

#endif /* _wlc_ampdu_ctl_h_ */
