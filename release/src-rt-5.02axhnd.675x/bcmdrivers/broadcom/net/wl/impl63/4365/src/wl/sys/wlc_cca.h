/*
 * 802.11h CCA stats module header file
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
 * $Id: wlc_cca.h 709556 2017-07-07 17:58:36Z $
*/

/**
 * Clear Channel Assessment is an 802.11 std term.
 * It assists in channel (re)selection and interference mitigation.
 */

#ifndef _wlc_cca_h_
#define _wlc_cca_h_

#define CCA_CHANNELS_NUM 39

#ifdef DONGLEBUILD
#define CCA_POOL_MAX		CCA_CHANNELS_NUM	/* trimmed to fit dongle */
#else /* DONGLEBUILD */
#ifdef ISID_STATS
#define CCA_POOL_MAX		450	/* bigger pool for interference samples */
#else
#define CCA_POOL_MAX		300
#endif // endif
#endif /* DONGLEBUILD */
#define CCA_FREE_BUF		0xffff

typedef uint16 cca_idx_t;
typedef struct {
	chanspec_t chanspec;
#ifndef DONGLEBUILD
	cca_idx_t  secs[MAX_CCA_SECS];
#endif /* DONGLEBUILD */
} cca_congest_channel_t;

typedef struct {
	uint32 duration;	/* millisecs spent sampling this channel */
	uint32 congest_ibss;	/* millisecs in our bss (presumably this traffic will */
				/*  move if cur bss moves channels) */
	uint32 congest_obss;	/* traffic not in our bss */
	uint32 interference;	/* millisecs detecting a non 802.11 interferer. */
	uint32 timestamp;	/* second timestamp */
#ifdef ISID_STATS
	uint32 crsglitch;	/* crs glitchs */
	uint32 badplcp;		/* num bad plcp */
	uint32 bphy_crsglitch;	/* bphy  crs glitchs */
	uint32 bphy_badplcp;		/* num bphy bad plcp */
#endif /* ISID_STATS */
	uint32 txnode[8];
	uint32 rxnode[8];
	uint32 xxobss;
} wlc_congest_t;

typedef struct {
	chanspec_t chanspec;	/* Which channel? */
	uint8 num_secs;		/* How many secs worth of data */
	wlc_congest_t  secs[1];	/* Data */
} wlc_congest_channel_req_t;

extern void cca_stats_upd(wlc_info_t *wlc, int calculate);
extern void cca_stats_tsf_upd(wlc_info_t *wlc);
extern cca_info_t *wlc_cca_attach(wlc_info_t *wlc);
extern void wlc_cca_detach(cca_info_t *cca);
extern int cca_query_stats(wlc_info_t *wlc, chanspec_t chanspec, int nsecs,
	wlc_congest_channel_req_t *stats_results, int buflen);
extern chanspec_t wlc_cca_get_chanspec(wlc_info_t *wlc, int index);
extern int cca_send_event(wlc_info_t *wlc, bool forced);

#endif /* _wlc_cca_h_ */
