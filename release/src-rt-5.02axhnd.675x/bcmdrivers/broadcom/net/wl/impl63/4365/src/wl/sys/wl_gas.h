/*
 * Support bcm_gas 802.11u GAS (Generic Advertisement Service) state machine in the driver.
 * See bcm_gas for the API.
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
 * $Id: wl_gas.h 708017 2017-06-29 14:11:45Z $
 */

/**
 * XXX Twiki: [HslServiceDiscovery]
 */

#ifndef _wl_gas_h_
#define _wl_gas_h_

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
#include <wl_eventq.h>

typedef struct wl_gas_info wl_gas_info_t;

#define MAX_WLIF_NUM				(1)

/*
 * Initialize gas private context.
 * Returns a pointer to the gas private context, NULL on failure.
 */
extern wl_gas_info_t *wl_gas_attach(wlc_info_t *wlc, wl_eventq_info_t *wlevtq);

/* Cleanup gas private context */
extern void wl_gas_detach(wl_gas_info_t *gas);

/* get ethernet address */
#define wl_cur_etheraddr(wlc, idx, buf) wl_gas_get_etheraddr((wlc_info_t *)wlc, idx, buf)
extern int wl_gas_get_etheraddr(wlc_info_t *wlc, int bsscfg_idx, struct ether_addr *outbuf);

/* transmit an action frame */
#define wl_actframe wl_gas_tx_actframe
extern int wl_gas_tx_actframe(void *w, int bsscfg_idx,
	uint32 packet_id, uint32 channel, int32 dwell_time,
	struct ether_addr *BSSID, struct ether_addr *da,
	uint16 len, uint8 *data);

/* abort action frame */
#define wl_actframe_abort wl_gas_abort_actframe
extern int wl_gas_abort_actframe(void *w, int bsscfg_idx);

/* get wlcif */
#define wl_getifbybsscfgidx(wlc, bsscfgidx) \
	wl_gas_get_wlcif((wlc), (bsscfgidx))
extern struct wlc_if *wl_gas_get_wlcif(wlc_info_t *wlc, int bsscfgidx);

extern int wl_gas_start_eventq(wl_gas_info_t *gas);
extern void wl_gas_stop_eventq(wl_gas_info_t *gas);

extern void *wl_gas_malloc(void *w, size_t size);
extern void wl_gas_free(void *w, void* memblk);
#endif /* _wl_gas_h_ */
