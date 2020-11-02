/*
 * P2P discovery state machine.
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

#ifndef _BCM_P2P_DISCOVERY_H_
#define _BCM_P2P_DISCOVERY_H_

typedef struct bcm_p2p_discovery bcm_p2p_discovery_t;

/* Opaque driver handle type. In dongle this is struct wlc_info_t, representing
 * the driver. On linux host this is struct ifreq, representing the primary OS
 * interface for a driver instance. To specify a virtual interface this should
 * be used together with a bsscfg index.
 */
struct bcm_p2p_discovery_wl_drv_hdl;

/* initialize P2P discovery */
int bcm_p2p_discovery_initialize(void);

/* deinitialize P2P discovery */
int bcm_p2p_discovery_deinitialize(void);

/* create P2P discovery */
bcm_p2p_discovery_t *bcm_p2p_discovery_create(
	struct bcm_p2p_discovery_wl_drv_hdl *drv, uint16 listenChannel);

/* destroy P2P discovery */
int bcm_p2p_discovery_destroy(bcm_p2p_discovery_t *disc);

/* reset P2P discovery */
int bcm_p2p_discovery_reset(bcm_p2p_discovery_t *disc);

/* start P2P discovery */
int bcm_p2p_discovery_start_discovery(bcm_p2p_discovery_t *disc);

/* start P2P extended listen */
/* for continuous listen set on=non-zero (e.g. 5000), off=0 */
int bcm_p2p_discovery_start_ext_listen(bcm_p2p_discovery_t *disc,
	uint16 listenOnTimeout, uint16 listenOffTimeout);

/* get bsscfg index of P2P discovery interface */
/* bsscfg index is valid only after started */
int bcm_p2p_discovery_get_bsscfg_index(bcm_p2p_discovery_t *disc);

/* wlan event handler */
void bcm_p2p_discovery_process_wlan_event(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length);

#endif /* _BCM_P2P_DISCOVERY_H_ */
