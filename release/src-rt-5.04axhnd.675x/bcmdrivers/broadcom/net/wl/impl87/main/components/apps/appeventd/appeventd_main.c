/*
 * Wireless Application Event Service
 * appeventd deamon (Linux)
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#include <802.11.h>
#include "appeventd.h"

#ifdef BCM_BSD
#include "appeventd_bsd.h"
#endif /* BCM_BSD */

#ifdef BCM_WBD
#include "appeventd_wbd.h"
#endif /* BCM_WBD */

int appeventd_debug_level = APPEVENTD_DEBUG_ERROR;

static void
appeventd_hexdump_ascii(const char *title, char *buf, int len)
{
	int i;
	if (!(appeventd_debug_level & APPEVENTD_DEBUG_DETAIL))
		return;

	printf("%s[%d]:", title, len);
	for (i = 0; i < len; i++) {
		if ((i&0xf) == 0)
			printf("\n%04x: ", i);
		printf("%02X ", buf[i]);
	}
	printf("\n\n");
}

#ifdef BCM_BSD
static void appeventd_parse_bsd_sta_stats(char *pkt, int len)
{
	app_event_bsd_sta_t *sta;
	int ulen = sizeof(app_event_bsd_sta_t);
	int	count = 0;

	while (len >= ulen) {
		count++;
		sta = (app_event_bsd_sta_t *)pkt;
		APPEVENTD_INFO("BSD-STA: index=%d, MAC="MACF", "
			"src=%s, dst=%s, steer_cnt=%d, "
			"steer_fail_cnt=%d, tx_rate=%d, "
			"at_ratio=%d, rssi=%d, "
			"steer_flags=0x%x\n",
			count, ETHER_TO_MACF(sta->addr),
			sta->src_ifname, sta->dst_ifname,
			sta->steer_cnt,	sta->steer_fail_cnt,
			sta->tx_rate, sta->at_ratio, sta->rssi,
			sta->steer_flags);
		len -= ulen;
		pkt += ulen;
	}
}

static void appeventd_parse_bsd_radio_stats(char *pkt, int len)
{
	app_event_bsd_radio_t *radio;
	int ulen = sizeof(app_event_bsd_radio_t);
	int	count = 0;

	while (len >= ulen) {
		count++;
		radio = (app_event_bsd_radio_t *)pkt;
		APPEVENTD_INFO("BSD-RADIO: index=%d, ifname=%s, "
			"bw_util=%d, throughput=%d\n",
			count, radio->ifname,
			radio->bw_util, radio->throughput);
		len -= ulen;
		pkt += ulen;
	}
}

static void appeventd_parse_bsd_sta_radio_stats(char *pkt, int len)
{
	int ulen = sizeof(app_event_bsd_sta_t);

	/* one sta */
	appeventd_parse_bsd_sta_stats(pkt, ulen);

	/* several radios */
	pkt += ulen;
	len -= ulen;
	appeventd_parse_bsd_radio_stats(pkt, len);
}
#endif /* BCM_BSD */

#ifdef BCM_WBD
/* Parse the weak/strong client event data received from wbd. */
static void
appeventd_parse_wbd_weak_strong_client_data(char *pkt, int len, int event_id)
{
	app_event_wbd_weak_sta_t *weak_sta = NULL;
	int ulen = sizeof(*weak_sta);

	while (len >= ulen) {
		weak_sta = (app_event_wbd_weak_sta_t *)pkt;
		APPEVENTD_INFO("%s Sta: Ifname = %s STA MAC = "MACF" RSSI = %d"
			" Tx-Failures = %d Tx-Rate = %f\n",
			((event_id == APP_E_WBD_MASTER_WEAK_CLIENT) ||
			(event_id == APP_E_WBD_SLAVE_WEAK_CLIENT)) ?
			"WBD-Weak" : "WBD-Strong",
			weak_sta->ifname, ETHER_TO_MACF(weak_sta->sta_addr), weak_sta->rssi,
			weak_sta->tx_failures, weak_sta->tx_rate);
		pkt += ulen;
		len -= ulen;
	}
}

/* Parse the steer start event data received from wbd. */
static void
appeventd_parse_wbd_steer_start_data(char *pkt, int len)
{
	app_event_wbd_steer_sta_t *steer_sta = NULL;
	int ulen = sizeof(*steer_sta);

	while (len >= ulen) {
		steer_sta = (app_event_wbd_steer_sta_t *)pkt;
		APPEVENTD_INFO("WBD-Steer Sta: Ifname = %s STA MAC = "MACF" RSSI = %d"
			" Target BSSID = "MACF" tbss rssi = %d\n", steer_sta->ifname,
			ETHER_TO_MACF(steer_sta->sta_addr), steer_sta->src_rssi,
			ETHER_TO_MACF(steer_sta->dst_addr), steer_sta->dst_rssi);
		pkt += ulen;
		len -= ulen;
	}
}

/* Parse the steer resp event data received from wbd. */
static void
appeventd_parse_wbd_steer_resp_data(char *pkt, int len)
{
	app_event_wbd_steer_resp_t *steer_resp = NULL;
	int ulen = sizeof(*steer_resp);

	while (len >= ulen) {
		steer_resp = (app_event_wbd_steer_resp_t *)pkt;
		APPEVENTD_INFO("WBD-Steer Resp: Ifname = %s STA MAC = "MACF""
			" Resp code = %d Steer Resp = %s\n", steer_resp->ifname,
			ETHER_TO_MACF(steer_resp->sta_addr), steer_resp->resp_code,
			steer_resp->resp);
		pkt += ulen;
		len -= ulen;
	}
}

/* Parse the steer end event data received from wbd. */
static void
appeventd_parse_wbd_steer_end_data(char *pkt, int len)
{
	app_event_wbd_steer_complete_t *steer_complete = NULL;
	int ulen = sizeof(*steer_complete);

	while (len >= ulen) {
		steer_complete = (app_event_wbd_steer_complete_t*)pkt;
		APPEVENTD_INFO("WBD-Steer End: Ifname = %s STA MAC = "MACF""
			" Source BSS = "MACF" Target BSS = "MACF" RSSI = %d TxRate = %f\n",
			steer_complete->ifname,
			ETHER_TO_MACF(steer_complete->sta_addr),
			ETHER_TO_MACF(steer_complete->src_addr),
			ETHER_TO_MACF(steer_complete->dst_addr),
			steer_complete->sta_stats.rssi,
			steer_complete->sta_stats.tx_rate);
		pkt += ulen;
		len -= ulen;
	}
}

/* Parse the map init start/end event data received from wbd. */
static void
appeventd_parse_wbd_map_init_data(char *pkt, int len)
{
	app_event_wbd_map_init_t *map_init = NULL;
	int ulen = sizeof(*map_init);

	while (len >= ulen) {
		map_init = (app_event_wbd_map_init_t *)pkt;
		APPEVENTD_INFO("WBD-Map Init: APP_type: %s  Device MAC = "MACF""
			" Map Init:[%s]\n",(map_init->app_id == MAP_APPTYPE_MASTER) ? "MASTER" :
			"SLAVE", ETHER_TO_MACF(map_init->device_id),
			(map_init->status == MAP_INIT_START) ? "Start" : "End");
		pkt += ulen;
		len -= ulen;
	}
}

/* Parse the controller data received from wbd */
static void
appeventd_parse_wbd_controller_data(char *pkt, int len)
{
	app_event_wbd_controller_data_t *controller_data = NULL;
	int ulen = sizeof(*controller_data);

	if (len < ulen) {
		APPEVENTD_ERROR("WBD-Length mismatch for Another Controller Detected event "
			"rcvd len %d expected len %d\n", len, ulen);
		return;
	}

	controller_data = (app_event_wbd_controller_data_t *)pkt;
	APPEVENTD_INFO("WBD-Controller Detected: Another controller device found with "
		"AL MAC = "MACF"\n\n", ETHER_TO_MACF(controller_data->device_id));
}
#endif /* BCM_WBD */

static void
appeventd_main_loop(int sock)
{
	struct timeval tv = {1, 0};    /* timed out every second */
	fd_set fdset;
	int status, fdmax;

	char buf[APPEVENTD_BUFSIZE], *pkt = buf;
	int bytes, len;

	app_event_t *app_event;

	FD_ZERO(&fdset);
	fdmax = -1;

	if (sock >= 0) {
		FD_SET(sock, &fdset);
		if (sock > fdmax)
			fdmax = sock;
	}
	else {
		APPEVENTD_ERROR("Err: wrong socket\n");
		return;
	}

	status = select(fdmax+1, &fdset, NULL, NULL, &tv);
	if ((status > 0) && FD_ISSET(sock, &fdset)) {
		if ((bytes = recv(sock, pkt, APPEVENTD_BUFSIZE, 0)) > sizeof(app_event_h)) {

			appeventd_hexdump_ascii("REVD Raw", pkt, bytes);

			app_event = (app_event_t *)(pkt);

			APPEVENTD_INFO("Received event %d, timestamp=%lu, status=%d, len=%d\n",
				app_event->h.event_id, app_event->h.timestamp,
				app_event->h.status, bytes);

			len = bytes - sizeof(app_event_h);
			pkt += sizeof(app_event_h);

			switch (app_event->h.event_id) {
#ifdef BCM_BSD
				case APP_E_BSD_STEER_START:
					/* Fall through */
				case APP_E_BSD_STEER_END:
					APPEVENTD_DEBUG("event %s: len=%d\n",
						(app_event->h.event_id == APP_E_BSD_STEER_START)?
						"APP_E_BSD_STEER_START":"APP_E_BSD_STEER_END",
						len);
					appeventd_parse_bsd_sta_radio_stats(pkt, len);
					break;
				case APP_E_BSD_STATS_QUERY:
					if (app_event->h.status == APP_E_BSD_STATUS_QUERY_STA) {
						APPEVENTD_DEBUG("APP_E_BSD_STATUS_QUERY_STA: "
							"len=%d\n", len);
						appeventd_parse_bsd_sta_stats(pkt, len);
					}
					else if (app_event->h.status ==
						APP_E_BSD_STATUS_QUERY_RADIO) {
						APPEVENTD_DEBUG("APP_E_BSD_STATUS_QUERY_RADIO: "
							"len=%d\n", len);
						appeventd_parse_bsd_radio_stats(pkt, len);
					}
					else {
						APPEVENTD_ERROR("unknown status: %d\n",
							app_event->h.status);
					}
					break;
#endif /* BCM_BSD */

#ifdef BCM_WBD
				case APP_E_WBD_MASTER_WEAK_CLIENT:
				case APP_E_WBD_SLAVE_WEAK_CLIENT:
					APPEVENTD_DEBUG("Received event %s len = %d\n",
					(app_event->h.event_id == APP_E_WBD_MASTER_WEAK_CLIENT) ?
					"APP_E_WBD_MASTER_WEAK_CLIENT" :
					"APP_E_WBD_SLAVE_WEAK_CLIENT", len);
					appeventd_parse_wbd_weak_strong_client_data(pkt, len,
						app_event->h.event_id);
					break;

				case APP_E_WBD_MASTER_STEER_START:
				case APP_E_WBD_SLAVE_STEER_START:
					APPEVENTD_DEBUG("Received event %s len = %d\n",
					(app_event->h.event_id == APP_E_WBD_MASTER_STEER_START) ?
					"APP_E_WBD_MASTER_STEER_START" :
					"APP_E_WBD_SLAVE_STEER_START", len);
					appeventd_parse_wbd_steer_start_data(pkt, len);
					break;

				case APP_E_WBD_MASTER_STEER_RESP:
				case APP_E_WBD_SLAVE_STEER_RESP:
					APPEVENTD_DEBUG("Received event %s len = %d\n",
					(app_event->h.event_id == APP_E_WBD_MASTER_STEER_RESP) ?
					"APP_E_WBD_MASTER_STEER_RESP" :
					"APP_E_WBD_SLAVE_STEER_RESP", len);
					appeventd_parse_wbd_steer_resp_data(pkt, len);
					break;

				case APP_E_WBD_MASTER_STEER_END:
					APPEVENTD_DEBUG("Received event %s len = %d\n",
						"APP_E_WBD_MASTER_STEER_END", len);
					appeventd_parse_wbd_steer_end_data(pkt, len);
					break;
				case APP_E_WBD_MASTER_MAP_INIT_START:
				case APP_E_WBD_MASTER_MAP_INIT_END:
					APPEVENTD_DEBUG("Received event %s len = %d\n",
						(app_event->h.event_id ==
						APP_E_WBD_MASTER_MAP_INIT_START) ?
						"APP_E_WBD_MASTER_MAP_INIT_START" :
						"APP_E_WBD_MASTER_MAP_INIT_END", len);
					appeventd_parse_wbd_map_init_data(pkt, len);
					break;

				case APP_E_WBD_SLAVE_MAP_INIT_END:
                                        APPEVENTD_DEBUG("Received event %s len = %d\n",
                                                 "APP_E_WBD_SLAVE_MAP_INIT_END",len);
					appeventd_parse_wbd_map_init_data(pkt, len);
                                        break;

				case APP_E_WBD_MASTER_ANOTHER_CONTROLLER_FOUND:
                                        APPEVENTD_DEBUG("Received event %s len = %d\n",
                                                 "APP_E_WBD_MASTER_ANOTHER_CONTROLLER_FOUND",len);
					appeventd_parse_wbd_controller_data(pkt, len);
					break;

				case APP_E_WBD_MASTER_STRONG_CLIENT:
				case APP_E_WBD_SLAVE_STRONG_CLIENT:
					APPEVENTD_DEBUG("Received event %s len = %d\n",
					(app_event->h.event_id == APP_E_WBD_MASTER_STRONG_CLIENT) ?
					"APP_E_WBD_MASTER_STRONG_CLIENT" :
					"APP_E_WBD_SLAVE_STRONG_CLIENT", len);
					appeventd_parse_wbd_weak_strong_client_data(pkt, len,
						app_event->h.event_id);
					break;

#endif /* BCM_WBD */

				default:
					APPEVENTD_ERROR("unknown id: %d\n", app_event->h.event_id);
					break;
			}
		}
	}
}

static int
appeventd_socket_init(void)
{
	int reuse = 1;
	struct sockaddr_in sockaddr;
	int event_socket = -1;

	/* open loopback socket to communicate with apps */
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(APPS_EVENT_UDP_PORT);

	if ((event_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		APPEVENTD_ERROR("Unable to create loopback socket\n");
		return APPEVENTD_FAIL;
	}

	if (setsockopt(event_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		APPEVENTD_ERROR("Unable to setsockopt to loopback socket %d.\n", event_socket);
		goto exit1;
	}

	if (bind(event_socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		APPEVENTD_ERROR("Unable to bind to loopback socket %d\n", event_socket);
		goto exit1;
	}

	APPEVENTD_DEBUG("opened loopback socket %d\n", event_socket);
	return event_socket;

	/* error handling */
exit1:
	close(event_socket);
	return APPEVENTD_FAIL;
}

int
main(int argc, char **argv)
{
	int sock;
	char *dbg;

	/* get appeventd_msg_level from nvram */
	if ((dbg = nvram_get("appeventd_msglevel"))) {
		appeventd_debug_level = (uint)strtoul(dbg, NULL, 0);
	}

	/* UDP socket init for app event service */
	if ((sock = appeventd_socket_init()) < 0) {
		APPEVENTD_ERROR("Err: fail to init socket\n");
		return APPEVENTD_FAIL;
	}

	/* receive app event from all applications via UDP */
	while (1) {
		appeventd_main_loop(sock);
	}

	close(sock);
	return APPEVENTD_OK;
}
