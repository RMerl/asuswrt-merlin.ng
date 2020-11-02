/*
 * Common OS-independent driver header file for power selection
 * algorithm of Broadcom 802.11b DCF-only Networking Adapter.
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
 *
 * $Id: wlc_power_sel.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef	_WLC_POWER_SEL_H_
#define	_WLC_POWER_SEL_H_

typedef struct lcb lcb_t;

struct rate_lcb_info {
	bool	tpr_good;
	bool	tpr_good_valid;
	bool	la_good;
	bool	la_good_valid;
	uint8	tpr_thresh;
	uint32	hi_rate_kbps;
#ifdef WL_LPC_DEBUG
	uint8	tpr_val;
#endif // endif
};

/* Debug log settings */
#define LPC_MSG_INFO_VAL 0x01
#define LPC_MSG_MORE_VAL 0x02
extern uint8 lpc_msglevel;

#ifdef WL_LPC_DEBUG
#define LPC_INFO(args) \
	do {if (WL_LPC_ON() && (lpc_msglevel & LPC_MSG_INFO_VAL)) \
	printf args;} while (0)
#define LPC_MORE(args) \
	do {if (WL_LPC_ON() && (lpc_msglevel & LPC_MSG_MORE_VAL)) \
	printf args;} while (0)
#else
#define LPC_INFO(args)
#define LPC_MORE(args)
#endif // endif

extern lpc_info_t *BCMATTACHFN(wlc_lpc_attach)(wlc_info_t *wlc);
extern void BCMATTACHFN(wlc_lpc_detach)(struct lpc_info *lpci);
#ifdef BCMDBG
extern void wlc_lpc_dump_lcb(lcb_t *lcb, int32 ac, struct bcmstrbuf *b);
#endif // endif
extern bool wlc_lpc_capable_chip(wlc_info_t *wlc);
extern void wlc_lpc_init(lpc_info_t *lpci, lcb_t *state, struct scb *scb);
extern uint8 wlc_lpc_getcurrpwr(lcb_t *state);
extern void wlc_lpc_update_pwr(lcb_t *state, uint8 ac, uint16 phy_ctrl_word);
extern void wlc_lpc_store_pcw(lcb_t *state, uint16 phy_ctrl_word);
extern int wlc_lpc_lcb_sz(void);
#endif	/* _WLC_POWER_SEL_H_ */
