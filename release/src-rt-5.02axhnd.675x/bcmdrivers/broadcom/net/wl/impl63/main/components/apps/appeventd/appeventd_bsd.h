/*
 * Wireless Application Event Service
 * appeventd-bsd shared header file
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id:  $
 */

#ifndef _appeventd_bsd_h_
#define _appeventd_bsd_h_

/* WiFi Application Event BSD status */
#define APP_E_BSD_STATUS_STEER_START  1
#define APP_E_BSD_STATUS_STEER_SUCC   2
#define APP_E_BSD_STATUS_STEER_FAIL   3
#define APP_E_BSD_STATUS_QUERY_STA    4
#define APP_E_BSD_STATUS_QUERY_RADIO  5

#define BSD_APPEVENT_BUFSIZE 2000

/* WiFi Application Event BSD data structure */
typedef struct app_event_bsd_sta {
	struct	ether_addr addr; /* STA mac addr */
	char	src_ifname[IFNAMSIZ];
	char	dst_ifname[IFNAMSIZ];
	int	steer_cnt;
	int	steer_fail_cnt;
	uint32	tx_rate;
	uint32	at_ratio;         /* airtime ratio */
	int	rssi;
	uint32	steer_flags;
} app_event_bsd_sta_t;

typedef struct app_event_bsd_radio {
	char	ifname[IFNAMSIZ];
	int	bw_util;
	uint32	throughput;
} app_event_bsd_radio_t;

#endif /* _appeventd_bsd_h_ */
