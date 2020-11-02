/*
 * wlc_taf.h
 *
 * This module contains the external definitions for the taf transmit module.
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
 * $Id$
 *
 */
#if !defined(__wlc_taf_h__)
#define __wlc_taf_h__

#ifdef WLTAF
/*
 * Module attach and detach functions. This is the tip of the iceberg, visible from the outside.
 * All the rest is hidden under the surface.
 */
extern wlc_taf_info_t *wlc_taf_attach(wlc_info_t *);
extern int wlc_taf_detach(wlc_taf_info_t *);

#define TAF_PKTTAG_NUM_BITS			14
#define TAF_PKTTAG_MAX				((1 << TAF_PKTTAG_NUM_BITS) - 1)
#define TAF_MICROSEC_NUM_BITS			16
#define TAF_MICROSEC_MAX			((1 << TAF_MICROSEC_NUM_BITS) - 1)
#define TAF_MICROSEC_TO_PKTTAG_SHIFT		(TAF_MICROSEC_NUM_BITS - TAF_PKTTAG_NUM_BITS)

#define TAF_UNITS_TO_MICROSEC(a)		((a + 1) / 2)
#define TAF_MICROSEC_TO_UNITS(a)		(a * 2)

#define TAF_MICROSEC_TO_PKTTAG(a)		((a) >> TAF_MICROSEC_TO_PKTTAG_SHIFT)
#define TAF_PKTTAG_TO_MICROSEC(a)		((a) << TAF_MICROSEC_TO_PKTTAG_SHIFT)
#define TAF_PKTTAG_TO_UNITS(a)			TAF_MICROSEC_TO_UNITS(TAF_PKTTAG_TO_MICROSEC(a))
#define TAF_UNITS_TO_PKTTAG(a)			TAF_MICROSEC_TO_PKTTAG(TAF_UNITS_TO_MICROSEC(a))

#define TAF_PKTBYTES_COEFF			4096
#define TAF_PKTBYTES_TO_TIME(len, p, b) \
	(((p) + ((len) * (b)) + (TAF_PKTBYTES_COEFF / 2)) / TAF_PKTBYTES_COEFF)
#define TAF_PKTBYTES_TO_UNITS(len, p, b) \
	TAF_MICROSEC_TO_UNITS(TAF_PKTBYTES_TO_TIME(len, p, b))

typedef struct {
	uint32  emptied;
	uint32  max_did_rel_delta;
	uint32  ready;
	uint32  release_frcount;
	uint32  release_pcount;
	uint32  release_time;
	uint32  did_rel_delta;
	uint32  did_rel_time;
} taf_scheduler_tid_stats_t;

typedef struct  taf_scheduler_public {
	bool    was_emptied;
	bool    is_ps_mode;
	uint8   index;
	uint8   actual_release;
	uint16  last_release_pkttag;
	uint32  time_limit_units;
	uint32  released_units;
	uint32  released_bytes;
	uint32  total_released_units;
	uint32  byte_rate;
	uint32  pkt_rate;
#ifdef BCMDBG
	taf_scheduler_tid_stats_t* tidstats;
#endif // endif
} taf_scheduler_public_t;

extern bool wlc_taf_enabled(wlc_taf_info_t* taf_info);
extern bool wlc_taf_rawfb(wlc_taf_info_t* taf_info);
extern uint32 wlc_taf_schedule_period(wlc_taf_info_t* taf_info, int tid);

extern uint16 wlc_taf_traffic_active(wlc_taf_info_t* taf_info, struct scb* scb);

extern bool wlc_taf_schedule(wlc_taf_info_t* taf_info,  int tid,  struct scb* scb,
                             bool force);

extern bool wlc_taf_handle_star(wlc_taf_info_t* taf_info, int tid, uint16 pkttag, uint8 index);
extern bool wlc_taf_reset_scheduling(wlc_taf_info_t* taf_info, int tid);

#endif /* WLTAF */

#endif /* __wlc_taf_h__ */
