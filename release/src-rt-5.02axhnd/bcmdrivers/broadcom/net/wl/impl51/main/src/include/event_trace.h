/*
 * Trace log blocks sent over HBUS
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: event_trace.h 608346 2015-12-24 14:42:49Z $
 */

/**
 * @file
 * @brief
 * Define the trace event ID and tag ID
 */

#ifndef	_WL_DIAG_H
#define	_WL_DIAG_H

#define DIAG_MAJOR_VERSION      1	/* 4 bits */
#define DIAG_MINOR_VERSION      0	/* 4 bits */
#define DIAG_MICRO_VERSION      0	/* 4 bits */

#define DIAG_VERSION		\
	((DIAG_MICRO_VERSION&0xF) | (DIAG_MINOR_VERSION&0xF)<<4 | \
	(DIAG_MAJOR_VERSION&0xF)<<8)
					/* bit[11:8] major ver */
					/* bit[7:4] minor ver */
					/* bit[3:0] micro ver */

/* event ID for trace purpose only, to avoid the conflict with future new
* WLC_E_ , starting from 0x8000
*/
#define TRACE_FW_AUTH_STARTED			0x8000
#define TRACE_FW_ASSOC_STARTED			0x8001
#define TRACE_FW_RE_ASSOC_STARTED		0x8002
#define TRACE_G_SCAN_STARTED			0x8003
#define TRACE_ROAM_SCAN_STARTED			0x8004
#define TRACE_ROAM_SCAN_COMPLETE		0x8005
#define TRACE_FW_EAPOL_FRAME_TRANSMIT_START	0x8006
#define TRACE_FW_EAPOL_FRAME_TRANSMIT_STOP	0x8007
#define TRACE_BLOCK_ACK_NEGOTIATION_COMPLETE	0x8008	/* protocol status */
#define TRACE_BT_COEX_BT_SCO_START		0x8009
#define TRACE_BT_COEX_BT_SCO_STOP		0x800a
#define TRACE_BT_COEX_BT_SCAN_START		0x800b
#define TRACE_BT_COEX_BT_SCAN_STOP		0x800c
#define TRACE_BT_COEX_BT_HID_START		0x800d
#define TRACE_BT_COEX_BT_HID_STOP		0x800e
#define TRACE_ROAM_AUTH_STARTED			0x800f
/* Event ID for NAN, start from 0x9000 */
#define TRACE_NAN_CLUSTER_STARTED               0x9000
#define TRACE_NAN_CLUSTER_JOINED                0x9001
#define TRACE_NAN_CLUSTER_MERGED                0x9002
#define TRACE_NAN_ROLE_CHANGED                  0x9003
#define TRACE_NAN_SCAN_COMPLETE                 0x9004
#define TRACE_NAN_STATUS_CHNG                   0x9005

/* Parameters of wifi logger events are TLVs */
/* Event parameters tags are defined as: */
#define TRACE_TAG_VENDOR_SPECIFIC		0 /* take a byte stream as parameter */
#define TRACE_TAG_BSSID				1 /* takes a 6 bytes MAC address as parameter */
#define TRACE_TAG_ADDR				2 /* takes a 6 bytes MAC address as parameter */
#define TRACE_TAG_SSID				3 /* takes a 32 bytes SSID address as parameter */
#define TRACE_TAG_STATUS			4 /* takes an integer as parameter */
#define TRACE_TAG_CHANNEL_SPEC			5 /* takes one or more wifi_channel_spec as */
						  /* parameter */
#define TRACE_TAG_WAKE_LOCK_EVENT		6 /* takes a wake_lock_event struct as parameter */
#define TRACE_TAG_ADDR1				7 /* takes a 6 bytes MAC address as parameter */
#define TRACE_TAG_ADDR2				8 /* takes a 6 bytes MAC address as parameter */
#define TRACE_TAG_ADDR3				9 /* takes a 6 bytes MAC address as parameter */
#define TRACE_TAG_ADDR4				10 /* takes a 6 bytes MAC address as parameter */
#define TRACE_TAG_TSF				11 /* take a 64 bits TSF value as parameter */
#define TRACE_TAG_IE				12 /* take one or more specific 802.11 IEs */
						   /* parameter, IEs are in turn indicated in */
						   /* TLV format as per 802.11 spec */
#define TRACE_TAG_INTERFACE			13 /* take interface name as parameter */
#define TRACE_TAG_REASON_CODE			14 /* take a reason code as per 802.11 */
						   /* as parameter */
#define TRACE_TAG_RATE_MBPS			15 /* take a wifi rate in 0.5 mbps */

typedef union {
	struct {
		uint16 event:	16;
		uint16 version:	16;
	};
	uint32 t;
} wl_event_log_id_ver_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL

#define ETHER_ADDR_PACK_LOW(addr)  (((addr)->octet[3])<<24 | ((addr)->octet[2])<<16 | \
	((addr)->octet[1])<<8 | ((addr)->octet[0]))
#define ETHER_ADDR_PACK_HI(addr)   (((addr)->octet[5])<<8 | ((addr)->octet[4]))
#define SSID_PACK(addr)   (((uint8)(addr)[0])<<24 | ((uint8)(addr)[1])<<16 | \
	((uint8)(addr)[2])<<8 | ((uint8)(addr)[3]))

/* for each event id with logging data, define its logging data structure */

typedef union {
	struct {
		uint16 status:	16;
		uint16 paraset:	16;
	};
	uint32 t;
} wl_event_log_blk_ack_t;

typedef union {
	struct {
		uint8	mode:	8;
		uint8	count:	8;
		uint16    ch:	16;
	};
	uint32 t;
} wl_event_log_csa_t;

typedef union {
	struct {
		uint8  status:		1;
		uint16 notused:		15;
		uint16 frag_tx_cnt:	16;
	};
	uint32 t;
} wl_event_log_eapol_tx_t;

typedef union {
	struct {
		uint16 tag;
		uint16 length; /* length of value in bytes */
	};
	uint32 t;
} wl_event_log_tlv_hdr_t;

#if defined(WL_EVENT_LOG_COMPILE) || defined(NAN_EVENT_LOG_COMPILE)
extern event_log_top_t *event_log_top;
#endif // endif

#ifdef WL_EVENT_LOG_COMPILE

#define _WL_EVENT_LOG(tag, event, ...) \
	do {					\
		event_log_top->timestamp = OSL_SYSUPTIME(); \
		wl_event_log_id_ver_t entry = {{event, DIAG_VERSION}}; \
		EVENT_LOG(tag, "WL event", entry.t , ## __VA_ARGS__); \
	} while (0)
#define WL_EVENT_LOG(args)	_WL_EVENT_LOG args
#else
#define WL_EVENT_LOG(args)
#endif    /* WL_EVENT_LOG_COMPILE */

#ifdef NAN_EVENT_LOG_COMPILE
#define _NAN_EVENT_LOG(tag, event, ...) \
	do {				\
		event_log_top->timestamp = OSL_SYSUPTIME(); \
		wl_event_log_id_ver_t hdr = {{event, DIAG_VERSION}}; \
		EVENT_LOG(tag, "NAN event", hdr.t , ## __VA_ARGS__); \
	} while (0)
#define NAN_EVENT_LOG(args)	_NAN_EVENT_LOG args
#else
#define NAN_EVENT_LOG(args)
#endif /* NAN_EVENT_LOG_COMPILE */

#endif  /* LINUX_POSTMOGRIFY_REMOVAL */
#endif	/* _WL_DIAG_H */
