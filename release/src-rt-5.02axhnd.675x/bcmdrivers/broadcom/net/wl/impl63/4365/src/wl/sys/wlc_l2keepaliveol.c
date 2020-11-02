/*
 * Broadcom 802.11 L2 keepalive offload Driver
 *
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
 * $Id: wlc_l2keepaliveol.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * It implements two main functionalities in wowl mode:
 * 1. Periodically send out a null or qos-null frame with a priority(in case of qos-null)
 *    and time period configured by OS.
 * 2. Delete a null/qos null frame received by ARM.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [L2KeepaliveOffload]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <bcm_ol_msg.h>
#include <wl_export.h>
#include <wlc_dngl_ol.h>
#include <wlc_l2keepaliveol.h>

/* L2 keepalive offload private info structure */
struct wlc_dngl_ol_l2keepalive_info {
	wlc_dngl_ol_info_t *wlc_dngl_ol;

	bool enabled;

	struct wl_timer	*l2keepalive_timer;

	/* Received message from Host to enable the l2 keepalive offload */
	uint16 period_ms;

	uint8 prio;

	uint8 flags;
};

static void
wlc_dngl_ol_l2keepalive_timer(void *arg)
{
	wlc_dngl_ol_info_t *wlc_dngl_ol = (wlc_dngl_ol_info_t *)arg;
	wlc_dngl_ol_l2keepalive_info_t * l2keepalive_info = wlc_dngl_ol->l2keepalive_ol;
	WL_INFORM(("wlc_dngl_ol_l2keepalive_timer fired %p\n", wlc_dngl_ol));

	if (l2keepalive_info->flags & BCM_OL_KEEPALIVE_PERIODIC_TX) {
		WL_INFORM(("Send Null data frame \n"));
		wlc_dngl_ol_sendnulldata(wlc_dngl_ol, -1);
	}
	else if (l2keepalive_info->flags & BCM_OL_KEEPALIVE_PERIODIC_TX_QOS) {
		WL_INFORM(("Send QOS Null data frame \n"));
		wlc_dngl_ol_sendnulldata(wlc_dngl_ol, l2keepalive_info->prio);
	}
}

/* L2 keepalive attach */
wlc_dngl_ol_l2keepalive_info_t *
wlc_dngl_ol_l2keepalive_attach(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	wlc_dngl_ol_l2keepalive_info_t *l2keepalive_ol;
	wlc_info_t	*wlc = wlc_dngl_ol->wlc;

	l2keepalive_ol =
		(wlc_dngl_ol_l2keepalive_info_t *)MALLOC(wlc_dngl_ol->osh,
		sizeof(wlc_dngl_ol_l2keepalive_info_t));
	if (!l2keepalive_ol) {
		WL_ERROR((" l2keepalive_ol malloc failed: %s\n", __FUNCTION__));
		return NULL;
	}
	WL_ERROR(("wlc_dngl_ol_l2keepalive_attach\n"));

	bzero(l2keepalive_ol, sizeof(wlc_dngl_ol_l2keepalive_info_t));
	l2keepalive_ol->wlc_dngl_ol = wlc_dngl_ol;

	if ((l2keepalive_ol->l2keepalive_timer =
	     wl_init_timer(wlc->wl,
		wlc_dngl_ol_l2keepalive_timer,
		wlc_dngl_ol,
		"l2keepalive_ol")) == NULL) {
		WL_ERROR(("wl %s: wl_init_timer() failed.\n", __FUNCTION__));
		return NULL;
	}

	WL_ERROR(("wlc_dngl_ol_l2keepalive_attach : timer initialized\n"));
	return l2keepalive_ol;
}

/* L2 keep alive message handling */
void wlc_dngl_ol_l2keepalive_send_proc(wlc_dngl_ol_l2keepalive_info_t *l2keepalive_ol,
	void *buf, int len)
{
	uchar *pktdata;
	olmsg_header *msg_hdr;
	olmsg_l2keepalive_enable_t *l2keepalive_enable;
	wlc_dngl_ol_info_t *wlc_dngl_ol;

	if (!l2keepalive_ol)
		return;

	wlc_dngl_ol = l2keepalive_ol->wlc_dngl_ol;
	pktdata = (uint8 *)buf;
	msg_hdr = (olmsg_header *)pktdata;

	switch (msg_hdr->type) {
		case BCM_OL_L2KEEPALIVE_ENABLE:
			WL_ERROR(("BCM_OL_L2KEEPALIVE_ENABLE\n"));
			l2keepalive_enable = (olmsg_l2keepalive_enable_t *)pktdata;
			l2keepalive_ol->period_ms = l2keepalive_enable->period_ms;
			l2keepalive_ol->prio = l2keepalive_enable->prio;
			l2keepalive_ol->flags = l2keepalive_enable->flags;
			l2keepalive_ol->enabled = TRUE;

WL_ERROR(("l2keepalive \n"));
WL_ERROR(("l2keepalive flags %d\n", wlc_dngl_ol->l2keepalive_ol->flags));
WL_ERROR(("l2keepalive prio %d\n", wlc_dngl_ol->l2keepalive_ol->prio));
WL_ERROR(("l2keepalive period_ms %d\n", wlc_dngl_ol->l2keepalive_ol->period_ms));
			break;
		default:
			WL_ERROR(("%s: INVALID message type:%d\n", __FILE__, msg_hdr->type));
			break;
	}
}

/* Start periodic keepalive timer */
void wlc_l2_keepalive_timer_start(wlc_dngl_ol_info_t *wlc_dngl_ol, bool restart)
{
	wlc_info_t *wlc = wlc_dngl_ol->wlc;
	wlc_dngl_ol_l2keepalive_info_t *l2keepalive_ol = wlc_dngl_ol->l2keepalive_ol;

	if (!l2keepalive_ol ||
		!(l2keepalive_ol->enabled) ||
		!(wlc_dngl_ol->wowl_cfg.wowl_enabled) ||
		(!(l2keepalive_ol->flags & BCM_OL_KEEPALIVE_PERIODIC_TX_QOS) &&
		!(l2keepalive_ol->flags & BCM_OL_KEEPALIVE_PERIODIC_TX)))
		return;

	if (restart) {
		WL_ERROR(("deleting l2keepalive_timer\n"));
		wl_del_timer(wlc->wl, l2keepalive_ol->l2keepalive_timer);
	}

	wl_add_timer(wlc->wl, l2keepalive_ol->l2keepalive_timer, l2keepalive_ol->period_ms, TRUE);
}

void
wlc_l2_keepalive_event_handler(wlc_dngl_ol_info_t *wlc_dngl_ol, uint32 event, void *event_data)
{
	switch (event) {
		case BCM_OL_E_WOWL_COMPLETE:
			wlc_l2_keepalive_timer_start(wlc_dngl_ol, FALSE);
			break;
	}
}

uint8 wlc_l2_keepalive_get_flags(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	if (!wlc_dngl_ol ||
		!(wlc_dngl_ol->l2keepalive_ol) ||
		!(wlc_dngl_ol->l2keepalive_ol->enabled))
		return 0;
	else
		return wlc_dngl_ol->l2keepalive_ol->flags;
}
