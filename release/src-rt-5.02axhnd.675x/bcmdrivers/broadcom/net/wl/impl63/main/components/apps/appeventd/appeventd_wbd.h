/*
 * Wireless Application Event Service
 * appeventd-wbd header file
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
 * $Id$
 */

#ifndef _appeventd_wbd_h_
#define _appeventd_wbd_h_

/* WiFi application event WBD status. */
#define APP_E_WBD_STATUS_SUCCESS	1

#ifndef IFNAMSIZ
#define IFNAMSIZ			16	/* Ifname size. */
#endif // endif

#define RESP_BUFSIZE			32	/* Resp buffer size. */

/* MAP init status */
typedef enum {
	MAP_INIT_START = 1,
	MAP_INIT_END = 2,
} map_init_status_t;

/* MAP APP type */
typedef enum {
	MAP_APPTYPE_MASTER = 1,
	MAP_APPTYPE_SLAVE = 2,
} map_apptype_t;

/* MAP init Start/End data. */
typedef struct app_event_wbd_map_init {
	struct ether_addr device_id;	/* Device AL MAC */
	map_init_status_t status;	/* Start = 1, end = 2 */
	map_apptype_t app_id;		/* Master = 1, slave = 2 */
} app_event_wbd_map_init_t;

/* Weak sta event data. */
typedef struct app_event_wbd_weak_sta {
	char ifname[IFNAMSIZ];		/* Interface name. */
	struct ether_addr sta_addr;	/* STA mac addr. */
	int rssi;			/* STA rssi. */
	uint32 tx_failures;		/* Tx fail count. */
	float tx_rate;			/* Tx rate. */
} app_event_wbd_weak_sta_t;

/* Steer sta event data. */
typedef struct app_event_wbd_steer_sta {
	char ifname[IFNAMSIZ];		/* Interface name. */
	struct ether_addr sta_addr;	/* STA mac addr. */
	struct ether_addr dst_addr;	/* Destination slave bssid. */
	int src_rssi;			/* STA rssi at source ap. */
	int dst_rssi;			/* STA rssi at target ap. */
} app_event_wbd_steer_sta_t;

/* Steer response event data. */
typedef struct app_event_wbd_steer_resp {
	char ifname[IFNAMSIZ];		/* Interface name. */
	struct ether_addr sta_addr;	/* STA mac addr. */
	int resp_code;			/* Steer response code. */
	char resp[RESP_BUFSIZE];	/* Steer response. */
} app_event_wbd_steer_resp_t;

/* Sta stats event data. */
typedef struct app_event_wbd_sta_stats {
	struct ether_addr sta_addr;	/* STA mac addr. */
	int rssi;			/* STA rssi. */
	float tx_rate;			/* STA tx-rate. */
} app_event_wbd_sta_stats_t;

/* Steer complete event data. */
typedef struct app_event_wbd_steer_complete {
	char ifname[IFNAMSIZ];		/* Interface name. */
	struct ether_addr sta_addr;	/* STA mac addr */
	struct ether_addr src_addr;	/* Source slave bssid. */
	struct ether_addr dst_addr;	/* Destination slave bssid. */
	app_event_wbd_sta_stats_t sta_stats;	/* STA stats after steering completion. */
} app_event_wbd_steer_complete_t;
#endif /* _appeventd_wbd_h_ */
