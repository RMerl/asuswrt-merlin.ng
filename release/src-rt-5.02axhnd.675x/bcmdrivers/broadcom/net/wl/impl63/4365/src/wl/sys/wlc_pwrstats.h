/*
 * Power statistics
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
 * $Id:$
 */

/**
 * XXX
 * Collect power related stats and expose to other modules
 * Twiki: []
 */

#ifndef _wlc_pwrstats_h_
#define _wlc_pwrstats_h_

/* debugging... */
#ifdef EVENT_LOG_COMPILE
#define WL_PWRSTATS_INFO(str, p1, p2, p3, p4) \
	EVENT_LOG(EVENT_LOG_TAG_PWRSTATS_INFO, str, p1, p2, p3, p4)
#else /* EVENT_LOG_COMPILE */
#define WL_PWRSTATS_INFO(str, p1, p2, p3, p4)
#endif /* EVENT_LOG_COMPILE */
typedef struct wlc_pwrstats_info wlc_pwrstats_info_t;
extern void wlc_pwrstats_detach(void *pwrstats);
extern void *wlc_pwrstats_attach(wlc_info_t *wlc);
extern void wlc_pwrstats_wake_reason_upd(wlc_info_t *wlc, bool stay_awake);
extern void wlc_pwrstats_bcn_process(wlc_bsscfg_t *bsscfg, void *ps, uint16 seq);
extern uint8 *wlc_pwrstats_fill_pmalert(wlc_info_t *wlc, uint8 *data);
extern void wlc_pwrstats_copy_event_wake_dur(void *buf, void *ps);
extern uint32 wlc_pwrstats_connect_time_upd(void *pwrstats);
extern void wlc_pwrstats_connect_start(void *pwrstats);
extern uint32 wlc_pwrstats_curr_connect_time(void *pwrstats);
extern void wlc_pwrstats_frts_start(void *ps);
extern void wlc_pwrstats_frts_end(void *ps);
extern void wlc_pwrstats_frts_checkpoint(wlc_pwrstats_info_t *ps);
extern void wlc_pwrstats_set_frts_data(wlc_pwrstats_info_t *ps, bool isdata);
extern uint32 wlc_pwrstats_get_frts_data_dur(wlc_pwrstats_info_t *ps);
#endif /* _wlc_pwrstats_h_ */
