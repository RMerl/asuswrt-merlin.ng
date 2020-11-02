/*
 * Support WiFi-Direct discovery state machine in the driver
 * for the P2P ofload (p2po).
 * See bcm_p2p_disc and wlc_p2po for the APIs.
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
 * $Id: wl_p2po_disc.h 708017 2017-06-29 14:11:45Z $
 */

/**
 * XXX Apple specific feature
 * Twiki: [P2PBonjour]
 */

#ifndef _wl_disc_h_
#define _wl_disc_h_

#include <wlc_cfg.h>
#include <d11.h>
#include <wlc_types.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <proto/bcmevent.h>
#include <siutils.h>
#include <wlc_pub.h>
#include <osl.h>
#include <wlc.h>
#include <wl_export.h>
#include <wl_tmr.h>

typedef struct wl_disc_info wl_disc_info_t;

/*
 * Initialize disc private context.
 * Returns a pointer to the disc private context, NULL on failure.
 */
extern wl_disc_info_t *wl_disc_attach(wlc_info_t *wlc);

/* Cleanup disc private context */
extern void wl_disc_detach(wl_disc_info_t *disc);

/* get device bsscfg index */
#define wl_p2p_dev(wlc, bsscfgIndex) \
	wl_disc_get_p2p_devcfg_idx((wlc), (bsscfgIndex))
extern int32 wl_disc_get_p2p_devcfg_idx(void *w, int32 *idx);

/* set p2p discovery state */
#define wl_p2p_state(wlc, state, chspec, dwell) \
	wl_disc_p2p_state((wlc), (state), (chspec), (dwell))
extern int wl_disc_p2p_state(void *w,
	uint8 state, chanspec_t chspec, uint16 dwell);

/* do p2p scan */
#define wl_p2p_scan(wlc, sync_id, is_active, num_probes, \
		active_dwell_time, passive_dwell_time, home_time, \
		num_channels, channels, flags) \
	wl_disc_p2p_scan((wlc), (sync_id), (is_active), (num_probes), \
		(active_dwell_time), (passive_dwell_time), (int)(home_time), \
		(num_channels), (channels), (flags))
extern int wl_disc_p2p_scan(void *w, uint16 sync_id, int is_active,
	int num_probes, int active_dwell_time, int passive_dwell_time,
	int home_time, int num_channels, uint16 *channels, uint8 flags);

/* get discovery bsscfg */
extern wlc_bsscfg_t * wl_disc_bsscfg(wl_disc_info_t *disc);

#endif /* _wl_disc_h_ */
