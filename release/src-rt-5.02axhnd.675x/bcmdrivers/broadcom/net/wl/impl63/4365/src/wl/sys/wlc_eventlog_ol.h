/*
 * Common (OS-independent) portion of
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
 * $Id: wlc_eventlog_ol.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef	__WLC_OL_EVENTLOG__

#define	WLC_EL_MAX_EVENTS	64

typedef struct wlc_ol_eventlog {
	uint32	event_time;	/* timestamp in Milliseconds resolution */
	uint32	event_data;	/* Data */
	uint8	event_type;	/* enumerated upto 256 types */
} wlc_ol_eventlog_t;

typedef	struct ol_el_buf {
	uint16			count;			/* Indicate buffer is free or not */
	uint16			write_pos;		/* Current write pointer in buffer */
	uint16			read_pos;		/* Current read pointer in buffer */
	wlc_ol_eventlog_t	event_buffer[WLC_EL_MAX_EVENTS]; /* Log buffer */
} ol_el_buf_t;

enum {
	WLC_EL_BEACON_LOST,
	WLC_EL_BEACON_IE_CHANGED,
	WLC_EL_BEACON_RSSI_THRESHOLD,
	WLC_EL_RADIO_HW_DISABLED,
	WLC_EL_UNASSOC,
	WLC_EL_DEAUTH,
	WLC_EL_DISASSOC,
	WLC_EL_SCAN_BEGIN,
	WLC_EL_SCAN_END,
	WLC_EL_PREFSSID_FOUND,
	WLC_EL_CSA,
	WLC_EL_PME_ASSERTED,
#ifdef BCMDBG
	WLC_EL_TEST = 254, /* Dummy test event, useful only for testing internal builds */
#endif /* BCMDBG */
	WLC_EL_LAST
};

#define	WLC_EL_INC_READ_POS(eb) {\
	if (eb->read_pos < WLC_EL_MAX_EVENTS)\
		eb->read_pos++; \
	else\
		eb->read_pos = 0; \
	eb->count--; \
	}

#define	WLC_EL_INC_WRITE_POS(eb) {\
	if (eb->write_pos < WLC_EL_MAX_EVENTS)\
		eb->write_pos++; \
	else\
		eb->write_pos = 0; \
	eb->count++; \
	}

#define	WLC_EL_EMPTY(eb) ((eb->count == 0) ? 1 : 0)
#define	WLC_EL_FULL(eb) ((eb->count == WLC_EL_MAX_EVENTS) ? 1 : 0)

#ifdef BCM_OL_DEV
wlc_dngl_ol_eventlog_info_t *wlc_dngl_ol_eventlog_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
void	wlc_dngl_ol_eventlog_write(wlc_dngl_ol_eventlog_info_t *context, uint8 type, uint32 data);
void	wlc_dngl_ol_eventlog_send_proc(wlc_dngl_ol_eventlog_info_t *eventlog_ol, int cmd);
void	wlc_dngl_ol_eventlog_handler(wlc_dngl_ol_eventlog_info_t *eventlog_ol, uint32 event,
	void *event_data);
#endif // endif

#endif	/* #ifndef __WLC_OL_EVENTLOG__ */
