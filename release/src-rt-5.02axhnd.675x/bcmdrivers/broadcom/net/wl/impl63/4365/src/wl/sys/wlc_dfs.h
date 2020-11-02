/*
 * 802.11h DFS module header file
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
 * $Id: wlc_dfs.h 740945 2018-01-15 08:44:07Z $
 */

#ifndef _wlc_dfs_h_
#define _wlc_dfs_h_

#if defined(WLRSDB) || defined(BGDFS)
#include <wlc_modesw.h>
#endif /* WLRSDB || BGDFS */

/* check if core revision supports background DFS scan core */
#define DFS_HAS_BACKGROUND_SCAN_CORE(wlc) D11REV_GE((wlc)->pub->corerev, 0x40)

/* radar subband information is available only for 4365/4366 corerev 0x40 (<=b1) and 0x41 (c0) */
#define DFS_HAS_SUBBAND_INFO(wlc) (D11REV_IS(wlc->pub->corerev, 0x40) || \
		D11REV_IS(wlc->pub->corerev, 0x41))

#ifdef WLDFS

/* module */
extern wlc_dfs_info_t *wlc_dfs_attach(wlc_info_t *wlc);
extern void wlc_dfs_detach(wlc_dfs_info_t *dfs);

/* others */
extern void wlc_set_dfs_cacstate(wlc_dfs_info_t *dfs, int state);
extern chanspec_t wlc_dfs_sel_chspec(wlc_dfs_info_t *dfs, bool force);
extern void wlc_dfs_reset_all(wlc_dfs_info_t *dfs);
extern int wlc_dfs_set_radar(wlc_dfs_info_t *dfs, int radar, uint subband);
extern bool wlc_valid_dfs_chanspec(wlc_info_t *wlc, chanspec_t chspec);
extern uint wlc_dfs_get_cactime_ms(wlc_dfs_info_t *dfs);
extern bool wlc_cac_is_clr_chanspec(wlc_dfs_info_t *dfs, chanspec_t chspec);
extern void wlc_dfs_send_action_frame_complete(wlc_info_t *wlc, uint txstauts, void *arg);
extern bool wlc_dfs_monitor_mode(wlc_dfs_info_t *dfs);
#ifdef WL_DFS_WAVE_MODE
extern bool wlc_dfs_test_mode(wlc_dfs_info_t *dfs);
#else
#define wlc_dfs_test_mode(dfs) FALSE
#endif /* WL_DFS_WAVE_MODE */

/* accessors */
extern uint32 wlc_dfs_get_radar(wlc_dfs_info_t *dfs);

extern uint32 wlc_dfs_get_chan_info(wlc_dfs_info_t *dfs, uint channel);

#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
extern int wlc_dfs_scan_in_progress(wlc_dfs_info_t *dfs);
extern void wlc_dfs_scan_abort(wlc_dfs_info_t *dfs);
extern void wlc_dfs_handle_modeswitch(wlc_dfs_info_t *dfs, uint new_state);
extern int wlc_dfs_scan(wlc_dfs_info_t *dfs, wl_chan_switch_t *csa);
#else
#define wlc_dfs_scan_in_progress(dfs) 0
#define wlc_dfs_scan_abort(dfs) do {} while (0)
#define wlc_dfs_handle_modeswitch(dfs, new_state) do {} while (0)
#define wlc_dfs_scan(dfs, csa) 0
#endif /* RSDB_DFS_SCAN || BGDFS */

#if defined(BGDFS) && defined(WL_MODESW)
extern void wlc_dfs_opmode_change_cb(void *ctx, wlc_modesw_notif_cb_data_t *notif_data);
#endif /* BGDFS && WL_MODESW */

#else /* !WLDFS */

#define wlc_dfs_attach(wlc) NULL
#define wlc_dfs_detach(dfs) do {} while (0)

#define wlc_set_dfs_cacstate(dfs, state) do {} while (0)
#define wlc_dfs_sel_chspec(dfs, force) 0
#define wlc_dfs_reset_all(dfs) do {} while (0)
#define wlc_dfs_set_radar(dfs, radar, subband)  BCME_UNSUPPORTED
#define wlc_valid_dfs_chanspec(wlc, chspec) (TRUE)
#define wlc_dfs_send_action_frame_complete(wlc, txstauts, arg) do {} while (0)

#define wlc_dfs_get_radar(dfs) 0
#define wlc_dfs_get_chan_info(dfs, channel) 0

#define wlc_dfs_scan_in_progress(dfs) 0
#define wlc_dfs_scan_abort(dfs) do {} while (0)
#define wlc_dfs_handle_modeswitch(dfs, new_state) do {} while (0)
#define wlc_dfs_scan(dfs, csa) 0

#define wlc_dfs_opmode_change_cb(ctx, notif_data) do {} while (0)
#define wlc_dfs_monitor_mode(dfs) (FALSE)
#define wlc_dfs_test_mode(dfs) FALSE

#endif /* !WLDFS */

#endif /* _wlc_dfs_h_ */
