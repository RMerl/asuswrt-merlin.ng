/* Common (OS-independent) portion of
 * Broadcom 802.11 offload Driver
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
 * $Id: wlc_eventlog_ol.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [EventLogging] [SoftwareFeatureEventLogging]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <proto/802.11.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <proto/802.3.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/bcmarp.h>
#include <bcm_ol_msg.h>
#include <wlc_dngl_ol.h>
#include <wlc_wowlol.h>
#include <wlc_bcnol.h>
#include <wl_export.h>
#include <wlc_eventlog_ol.h>

struct	wlc_dngl_ol_eventlog_info {
	bool			enabled; 	/* Flag to indicate EVENTLOG enabled */
	ol_el_buf_t		eb;		/* Event Buffer */
	wlc_dngl_ol_info_t	*wlc_dngl_ol;	/* Back pinter to our parent */
};

#ifdef __HAS_EVENTLOG_REPORT__
/*
 * We will need this when we can send
 * message to host when host wakes up
 * to send messages up to the host
 */
static void wlc_dngl_ol_eventlog_report(wlc_dngl_ol_eventlog_info_t *eventlog_ol,
	uint8 num, uint8 totalnum, uint16 buflen, uint8 *buf);

static void
wlc_dngl_ol_eventlog_report(wlc_dngl_ol_eventlog_info_t *eventlog_ol,
	uint8 num, uint8 totalnum, uint16 buflen, uint8 *buf)
{
	olmsg_eventlog_notification	*notification;
	void				*p;
	uint16				req_size = sizeof(olmsg_eventlog_notification) + buflen;

	if (!eventlog_ol) {
		return;
	}

	if (eventlog_ol->enabled == 0) {
		return;
	}
	/* Get a packet to send to the host */
	p = pktpool_get(eventlog_ol->wlc_dngl_ol->shared_msgpool);

	if (p == NULL) {
		WL_ERROR(("Failed to get packet\n"));
		return;
	}

	/* Prepare the payload of the done msg to be sent to the host */
	PKTSETLEN(eventlog_ol->wlc_dngl_ol->osh, p, req_size);

	notification = (olmsg_eventlog_notification *)PKTDATA(eventlog_ol->wlc_dngl_ol->osh, p);
	bzero(notification, req_size);

	/* Send up the info to the host */
	notification->hdr.type = BCM_OL_EL_REPORT;
	notification->hdr.seq = 0;
	notification->hdr.len = req_size - sizeof(olmsg_header);
	notification->num = num;
	notification->totalnum = totalnum;
	notification->buflen = buflen;
	memcpy(notification->buf, buf, buflen);

	wl_msgup(eventlog_ol->wlc_dngl_ol->wlc->wl, wlc_dngl_ol->osh, p);
	return;
}
#endif /* __HAS_EVENTLOG_REPORT__ */

wlc_dngl_ol_eventlog_info_t *
wlc_dngl_ol_eventlog_attach(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	wlc_dngl_ol_eventlog_info_t	*eventlog_ol;
	/* wlc_ol_eventlog_t		*buf; */

	WL_ERROR(("===========> %s: Called\n", __FUNCTION__));
	eventlog_ol = (wlc_dngl_ol_eventlog_info_t *)MALLOC(wlc_dngl_ol->osh,
		sizeof(wlc_dngl_ol_eventlog_info_t));
	if (!eventlog_ol) {
		WL_ERROR(("eventlog_ol malloc failed: %s\n", __FUNCTION__));
		return NULL;
	}
	bzero(eventlog_ol, sizeof(wlc_dngl_ol_eventlog_info_t));

	eventlog_ol->wlc_dngl_ol = wlc_dngl_ol;

	/*
	 * We have to write this address to SHM so
	 * that Host can also access it as it access
	 * event logs
	 */
	RXOESHARED(wlc_dngl_ol)->eventlog_addr = (uint32)&eventlog_ol->eb;
	return eventlog_ol;
}

void
wlc_dngl_ol_eventlog_send_proc(wlc_dngl_ol_eventlog_info_t *eventlog_ol, int cmd)
{
	if (!eventlog_ol) {
		return;
	}
	WL_ERROR(("===========> %s: Called\n", __FUNCTION__));
	if (cmd == BCM_OL_EL_START) {
		eventlog_ol->eb.write_pos = 0; /* clear pointer in buffer */
		bzero(eventlog_ol->eb.event_buffer, WLC_EL_MAX_EVENTS * sizeof(wlc_ol_eventlog_t));
		eventlog_ol->eb.count = 0;
		eventlog_ol->eb.write_pos = 0;
		eventlog_ol->eb.read_pos = 0;

		eventlog_ol->enabled = 1; /* TURN ON */
		WL_ERROR(("Event Logging started: %s\n", __FUNCTION__));
	} else if (cmd == BCM_OL_EL_SEND_REPORT) {
		eventlog_ol->enabled = 0;
		WL_ERROR(("Event Logging Read: %s\n", __FUNCTION__));
	} else {
		WL_ERROR(("BAD COMMAND: %s\n", __FUNCTION__));
	}
}

void
wlc_dngl_ol_eventlog_handler(wlc_dngl_ol_eventlog_info_t *eventlog_ol,
	uint32			event,
	void			*event_data)
{
	if (!eventlog_ol) {
		return;
	}

	switch (event) {
		case BCM_OL_E_WOWL_START:
			wlc_dngl_ol_eventlog_send_proc(eventlog_ol, BCM_OL_EL_START);
			break;
	}
}

void
wlc_dngl_ol_eventlog_write(wlc_dngl_ol_eventlog_info_t *eventlog_ol, uint8 type, uint32 data)
{
	ol_el_buf_t			*eb;		/* Event Buffer */
	uint16				cp;

	if (!eventlog_ol) {
		return;
	}

	if (eventlog_ol->enabled == 0) {
		return;
	}

	if (eventlog_ol->wlc_dngl_ol->wowl_cfg.wowl_enabled == FALSE) {
		/* Somebody tried to log in WAKE mode - reject */
		return;
	}

	WL_TRACE(("%s: Called for Type: %d Data: %08x\n", __FUNCTION__, type, data));

	eb = &(eventlog_ol->eb);

	/*
	 * See if we have room to store event
	 * if we don't have space then delete
	 * oldest event from the queue
	 */
	if (WLC_EL_FULL(eb)) {
		WLC_EL_INC_READ_POS(eb);
	}

	cp = eventlog_ol->eb.write_pos;
	eventlog_ol->eb.event_buffer[cp].event_type = type;
	eventlog_ol->eb.event_buffer[cp].event_data = data;
	eventlog_ol->eb.event_buffer[cp].event_time = OSL_SYSUPTIME();

	WLC_EL_INC_WRITE_POS(eb);
}
