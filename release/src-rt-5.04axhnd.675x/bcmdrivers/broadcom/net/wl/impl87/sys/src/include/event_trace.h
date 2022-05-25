/*
 * Trace log blocks sent over HBUS
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: event_trace.h 787288 2020-05-25 16:56:02Z $
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

#ifndef LINUX_POSTMOGRIFY_REMOVAL

#define ETHER_ADDR_PACK_LOW(addr)  (((addr)->octet[3])<<24 | ((addr)->octet[2])<<16 | \
	((addr)->octet[1])<<8 | ((addr)->octet[0]))
#define ETHER_ADDR_PACK_HI(addr)   (((addr)->octet[5])<<8 | ((addr)->octet[4]))
#define SSID_PACK(addr)   (((uint8)(addr)[0])<<24 | ((uint8)(addr)[1])<<16 | \
	((uint8)(addr)[2])<<8 | ((uint8)(addr)[3]))

#endif  /* LINUX_POSTMOGRIFY_REMOVAL */
#endif	/* _WL_DIAG_H */
