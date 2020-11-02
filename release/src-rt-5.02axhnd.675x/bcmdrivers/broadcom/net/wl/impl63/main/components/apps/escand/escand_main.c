/*
 * ESCAN Daemon (Linux)
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
 * $Id: escand_main.c 769837 2018-11-28 06:31:29Z $
 */

#include <ethernet.h>
#include <bcmeth.h>
#include <bcmevent.h>
#include <802.11.h>
#include <common_utils.h>

#include "escand_svr.h"

#include <signal.h>

escand_wksp_t *d_info;
static bool escand_running = TRUE;

static void
escand_term_hdlr(int sig)
{
	escand_running = FALSE;
	return;
}

/* open a UDP packet to event dispatcher for receiving/sending data */
static int
escand_open_eventfd()
{
	int reuse = 1;
	struct sockaddr_in sockaddr;
	int fd = ESCAND_DFLT_FD;

	/* open loopback socket to communicate with event dispatcher */
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(EAPD_WKSP_DCS_UDP_SPORT);

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		ESCAND_ERROR("Unable to create loopback socket\n");
		goto exit1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		ESCAND_ERROR("Unable to setsockopt to loopback socket %d.\n", fd);
		goto exit1;
	}

	if (bind(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		ESCAND_ERROR("Unable to bind to loopback socket %d\n", fd);
		goto exit1;
	}

	ESCAND_INFO("opened loopback socket %d\n", fd);
	d_info->event_fd = fd;

	return ESCAND_OK;

	/* error handling */
exit1:
	if (fd != ESCAND_DFLT_FD) {
		close(fd);
	}
	return errno;
}

static int
escand_svr_socket_init(unsigned int port)
{
	int reuse = 1;
	struct sockaddr_in s_sock;

	d_info->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (d_info->listen_fd < 0) {
		ESCAND_ERROR("Socket open failed: %s\n", strerror(errno));
		return ESCAND_FAIL;
	}

	if (setsockopt(d_info->listen_fd, SOL_SOCKET, SO_REUSEADDR,
		(char*)&reuse, sizeof(reuse)) < 0) {
		ESCAND_ERROR("Unable to setsockopt to server socket %d.\n", d_info->listen_fd);
		return ESCAND_FAIL;
	}

	memset(&s_sock, 0, sizeof(struct sockaddr_in));
	s_sock.sin_family = AF_INET;
	s_sock.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	s_sock.sin_port = htons(port);

	if (bind(d_info->listen_fd, (struct sockaddr *)&s_sock,
		sizeof(struct sockaddr_in)) < 0) {
		ESCAND_ERROR("Socket bind failed: %s\n", strerror(errno));
		return ESCAND_FAIL;
	}

	if (listen(d_info->listen_fd, 5) < 0) {
		ESCAND_ERROR("Socket listen failed: %s\n", strerror(errno));
		return ESCAND_FAIL;
	}

	return ESCAND_OK;
}

static void
escand_close_listenfd()
{
	/* close event dispatcher socket */
	if (d_info->listen_fd != ESCAND_DFLT_FD) {
		ESCAND_INFO("listenfd: close  socket %d\n", d_info->listen_fd);
		close(d_info->listen_fd);
		d_info->event_fd = ESCAND_DFLT_FD;
	}
	return;
}

static void
escand_close_eventfd()
{
	/* close event dispatcher socket */
	if (d_info->event_fd != ESCAND_DFLT_FD) {
		ESCAND_INFO("eventfd: close loopback socket %d\n", d_info->event_fd);
		close(d_info->event_fd);
		d_info->event_fd = ESCAND_DFLT_FD;
	}
	return;
}

static int
escand_validate_wlpvt_message(int bytes, uint8 *dpkt)
{
	bcm_event_t *pvt_data;

	/* the message should be at least the header to even look at it */
	if (bytes < sizeof(bcm_event_t) + 2) {
		ESCAND_ERROR("Invalid length of message\n");
		goto error_exit;
	}
	pvt_data  = (bcm_event_t *)dpkt;
	if (ntohs(pvt_data->bcm_hdr.subtype) != BCMILCP_SUBTYPE_VENDOR_LONG) {
		ESCAND_ERROR("%s: not vendor specifictype\n",
		       pvt_data->event.ifname);
		goto error_exit;
	}
	if (pvt_data->bcm_hdr.version != BCMILCP_BCM_SUBTYPEHDR_VERSION) {
		ESCAND_ERROR("%s: subtype header version mismatch\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	if (ntohs(pvt_data->bcm_hdr.length) < BCMILCP_BCM_SUBTYPEHDR_MINLENGTH) {
		ESCAND_ERROR("%s: subtype hdr length not even minimum\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	if (bcmp(&pvt_data->bcm_hdr.oui[0], BRCM_OUI, DOT11_OUI_LEN) != 0) {
		ESCAND_ERROR("%s: escand_validate_wlpvt_message: not BRCM OUI\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	/* check for wl dcs message types */
	switch (ntohs(pvt_data->bcm_hdr.usr_subtype)) {
		case BCMILCP_BCM_SUBTYPE_EVENT:

			ESCAND_INFO("subtype: event\n");
			break;
		default:
			goto error_exit;
			break;
	}
	return ESCAND_OK; /* good packet may be this is destined to us */
error_exit:
	return ESCAND_FAIL;
}

/*
 * Receives and processes the commands from client
 * o Wait for connection from client
 * o Process the command and respond back to client
 * o close connection with client
 */
static int
escand_proc_client_req(void)
{
	uint resp_size = 0;
	int rcount = 0;
	int fd = -1;
	struct sockaddr_in cliaddr;
	uint len = 0; /* need initialize here to avoid EINVAL */
	char* buf;
	int ret = 0;

	fd = accept(d_info->listen_fd, (struct sockaddr*)&cliaddr,
		&len);
	if (fd < 0) {
		if (errno == EINTR) return 0;
		else {
			ESCAND_ERROR("accept failed: errno: %d - %s\n", errno, strerror(errno));
			return -1;
		}
	}
	d_info->conn_fd = fd;

	if (!d_info->cmd_buf)
		d_info->cmd_buf = escand_malloc(ESCAND_BUFSIZE_4K);

	buf = d_info->cmd_buf;

	/* get command from client */
	if ((rcount = sread(d_info->conn_fd, buf, ESCAND_BUFSIZE_4K)) < 0) {
		ESCAND_ERROR("Failed reading message from client: %s\n", strerror(errno));
		ret = -1;
		goto done;
	}

	/* reqeust is small string. */
	if (rcount == ESCAND_BUFSIZE_4K) {
		ESCAND_ERROR("Client Req too large\n");
		ret = -1;
		goto done;
	}
	buf[rcount] = '\0';

	escand_proc_cmd(d_info, buf, rcount, &resp_size);

	if (swrite(d_info->conn_fd, buf, resp_size) < 0) {
		ESCAND_ERROR("Failed sending message: %s\n", strerror(errno));
		ret = -1;
		goto done;
	}

done:
	close(d_info->conn_fd);
	d_info->conn_fd = -1;
	return ret;
}

/* This function compares the exclude_ifname with ifnames and
 * increment the excl_cnt if it matches
 */
int
escand_compare_iface_list(char *exclude_ifname)
{
	int i, excl_cnt = 0;
	if (exclude_ifname != NULL) {
		for (i = 0; i < 16 && d_info->escand_info->exclude_ifnames[i] != NULL; i++) {
			if (!strncmp(d_info->escand_info->exclude_ifnames[i], exclude_ifname,
					strlen(exclude_ifname))) {
				excl_cnt++;
			}
		}
	}
	return excl_cnt;
}

/* listen to sockets and call handlers to process packets */
void
escand_main_loop(struct timeval *tv)
{
	fd_set fdset;
	int width, status = 0, bytes, len;
	uint8 *pkt;
	bcm_event_t *pvt_data;
#ifdef DEBUG
	wl_bcmdcs_data_t dcs_data;
#endif /* DEBUG */
	escand_chaninfo_t *c_info;
	int idx = 0;
	int err;
	uint32 escan_event_status;
	wl_escan_result_t *escan_data = NULL;
	struct escan_bss *result;

	/* init file descriptor set */
	FD_ZERO(&d_info->fdset);
	d_info->fdmax = -1;

	/* build file descriptor set now to save time later */
	if (d_info->event_fd != ESCAND_DFLT_FD) {
		FD_SET(d_info->event_fd, &d_info->fdset);
		d_info->fdmax = d_info->event_fd;
	}

	if (d_info->listen_fd != ESCAND_DFLT_FD) {
		FD_SET(d_info->listen_fd, &d_info->fdset);
		if (d_info->listen_fd > d_info->fdmax)
			d_info->fdmax = d_info->listen_fd;
	}

	pkt = d_info->packet;
	len = sizeof(d_info->packet);
	width = d_info->fdmax + 1;
	fdset = d_info->fdset;

	/* listen to data availible on all sockets */
	status = select(width, &fdset, NULL, NULL, tv);

	if ((status == -1 && errno == EINTR) || (status == 0))
		return;

	if (status <= 0) {
		ESCAND_ERROR("err from select: %s", strerror(errno));
		return;
	}

	if (d_info->listen_fd != ESCAND_DFLT_FD && FD_ISSET(d_info->listen_fd, &fdset)) {
		d_info->stats.num_cmds++;
		escand_proc_client_req();
	}

	/* handle brcm event */
	if (d_info->event_fd !=  ESCAND_DFLT_FD && FD_ISSET(d_info->event_fd, &fdset)) {
		char *ifname = (char *)pkt;
		struct ether_header *eth_hdr = (struct ether_header *)(ifname + IFNAMSIZ);
		uint16 ether_type = 0;
		uint32 evt_type;

		ESCAND_INFO("recved event from eventfd\n");

		d_info->stats.num_events++;

		if ((bytes = recv(d_info->event_fd, pkt, len, 0)) <= 0)
			return;

		ESCAND_INFO("recved %d bytes from eventfd, ifname: %s\n",
				bytes, ifname);
		bytes -= IFNAMSIZ;

		if ((ether_type = ntohs(eth_hdr->ether_type) != ETHER_TYPE_BRCM)) {
			ESCAND_INFO("recved ether type %x\n", ether_type);
			return;
		}

		if ((err = escand_validate_wlpvt_message(bytes, (uint8 *)eth_hdr)))
			return;

		pvt_data = (bcm_event_t *)(ifname + IFNAMSIZ);
		evt_type = ntoh32(pvt_data->event.event_type);
		ESCAND_INFO("recved brcm event, event_type: %d\n", evt_type);

		escand_check_ifname_is_virtual(&ifname);

		if ((idx = escand_idx_from_map(ifname)) < 0) {
			ESCAND_ERROR("cannot find the mapped entry for ifname: %s\n", ifname);
			return;
		}

		c_info = d_info->escand_info->chan_info[idx];

		if (c_info->mode == ESCAND_MODE_DISABLE && c_info->escand_boot_only) {
			ESCAND_INFO("%s: No event handling enabled. Only boot selection \n",
				c_info->name);
			return;
		}

		if ((evt_type != WLC_E_ESCAN_RESULT) && c_info->escand_escan->escand_escan_inprogress) {
			ESCAND_INFO("%s: when scan is in progress, don't allow other"
				" events for processing\n", c_info->name);
			return;
		}

		if (!escand_is_mode_check(c_info->name)) {
			ESCAND_INFO("%s: avoid scanning when ESCAND is not in AP mode\n",
				c_info->name);
			return;
		}

		d_info->stats.valid_events++;

		switch (evt_type) {
			case WLC_E_SCAN_COMPLETE:
			{
				ESCAND_INFO("%s: received brcm event: scan complete\n", c_info->name);
				break;
			}
			case WLC_E_ESCAN_RESULT:
			{
				if (!c_info->escand_escan->escand_escan_inprogress ||
						!c_info->escand_escan->escand_use_escan) {
					ESCAND_INFO("%s: Escan not triggered from ESCAND\n",
						c_info->name);
					return;
				}

				escan_event_status = ntoh32(pvt_data->event.status);
				escan_data = (wl_escan_result_t*)(pvt_data + 1);

				if (escan_event_status == WLC_E_STATUS_PARTIAL) {
					wl_bss_info_t *bi = &escan_data->bss_info[0];
					wl_bss_info_t *bss;

					/* check if we've received info of same BSSID */
					for (result = c_info->escand_escan->escan_bss_head;
							result;	result = result->next) {
						bss = result->bss;

						if (!memcmp(bi->BSSID.octet,
							bss->BSSID.octet,
							ETHER_ADDR_LEN) &&
							CHSPEC_BAND(bi->chanspec) ==
							CHSPEC_BAND(bss->chanspec) &&
							bi->SSID_len ==	bss->SSID_len &&
							! memcmp(bi->SSID, bss->SSID,
							bi->SSID_len)) {
							break;
						}
					}

					if (!result) {
						/* New BSS. Allocate memory and save it */
						struct escan_bss *ebss;
						ebss = (struct escan_bss *)escand_malloc(
							OFFSETOF(struct escan_bss, bss)
							+ bi->length);

						if (!ebss) {
							ESCAND_ERROR("can't allocate memory"
									"for escan bss");
							break;
						}

						ebss->next = NULL;
						memcpy(&ebss->bss, bi, bi->length);
						if (c_info->escand_escan->escan_bss_tail) {
							c_info->escand_escan->escan_bss_tail->next =
							ebss;
						} else {
							c_info->escand_escan->escan_bss_head =
							ebss;
						}

						c_info->escand_escan->escan_bss_tail = ebss;
					} else if (bi->RSSI != WLC_RSSI_INVALID) {
						/* We've got this BSS. Update RSSI
						   if necessary
						   */
						bool preserve_maxrssi = FALSE;
						if (((bss->flags &
							WL_BSS_FLAGS_RSSI_ONCHANNEL) ==
							(bi->flags &
							WL_BSS_FLAGS_RSSI_ONCHANNEL)) &&
							((bss->RSSI == WLC_RSSI_INVALID) ||
							(bss->RSSI < bi->RSSI))) {
							/* Preserve max RSSI if the
							   measurements are both
							   on-channel or both off-channel
							   */
							preserve_maxrssi = TRUE;
						} else if ((bi->flags &
							WL_BSS_FLAGS_RSSI_ONCHANNEL) &&
							(bss->flags &
							WL_BSS_FLAGS_RSSI_ONCHANNEL) == 0) {
							/* Preserve the on-channel RSSI
							   measurement if the
							   new measurement is off channel
							   */
							preserve_maxrssi = TRUE;
							bss->flags |=
							WL_BSS_FLAGS_RSSI_ONCHANNEL;
						}

						if (preserve_maxrssi) {
							bss->RSSI = bi->RSSI;
							bss->SNR = bi->SNR;
							bss->phy_noise = bi->phy_noise;
						}
					}
				} else if (escan_event_status == WLC_E_STATUS_SUCCESS) {
					/* Escan finished. Lets dump results */
					c_info->timestamp_escand_scan = time(NULL);
					if (c_info->escand_escan->scan_type == ESCAND_SCAN_TYPE_CS) {
						c_info->timestamp_tx_idle =
							c_info->timestamp_escand_scan;
					}
#ifdef ESCAN_DEBUG
					/* print scan results */
					for (result = c_info->escand_escan->escan_bss_head;
						result;	result = result->next) {
						dump_bss_info(result->bss);
					}
#endif /* ESCAN_DEBUG */
					c_info->escand_escan->escand_escan_inprogress = FALSE;
					ESCAND_INFO("%s: Escan success!\n", c_info->name);
				} else {
					ESCAND_ERROR("sync_id: %d, status:%d, misc."
						"error/abort\n",
						escan_data->sync_id, status);

					escand_escan_free(c_info->escand_escan->escan_bss_head);
					c_info->escand_escan->escan_bss_head = NULL;
					c_info->escand_escan->escan_bss_tail = NULL;
					c_info->escand_escan->escand_escan_inprogress = FALSE;
				}
				break;
			}
			default:
				ESCAND_INFO("%s: received event type %x\n", c_info->name, evt_type);
				break;
		}
	}
}

static int
escand_init(void)
{
	int err = ESCAND_OK;
	uint  port = ESCAND_DFLT_CLI_PORT;

	d_info = (escand_wksp_t*)escand_malloc(sizeof(escand_wksp_t));

	d_info->version = ESCAND_VERSION;
	d_info->fdmax = ESCAND_DFLT_FD;
	d_info->event_fd = ESCAND_DFLT_FD;
	d_info->listen_fd = ESCAND_DFLT_FD;
	d_info->conn_fd = ESCAND_DFLT_FD;
	d_info->poll_interval = ESCAND_DFLT_POLL_INTERVAL;
	d_info->ticks = 0;
	d_info->cmd_buf = NULL;

	err = escand_open_eventfd();

	if (err == ESCAND_OK)
		err = escand_svr_socket_init(port);

	return err;
}

static void
escand_cleanup(void)
{
	if (d_info) {
		if (d_info->escand_info)
			escan_cleanup(&d_info->escand_info);
		ESCAND_FREE(d_info->cmd_buf);
		free(d_info);
	}
}

static void
escand_watchdog(uint ticks)
{
	int i, ret, isup = 0;
	escand_chaninfo_t* c_info;

	for (i = 0; i < ESCAND_MAX_IF_NUM; i++) {
		c_info = d_info->escand_info->chan_info[i];

		if ((!c_info) || (c_info->mode == ESCAND_MODE_DISABLE)) {
			continue;
		}

		ret = escand_get_isup(c_info, &isup);
		if (ret < 0) {
			ESCAND_INFO("Couldn't get up status on %s, treat as disabled\n",
				c_info->name);
			c_info->wasdown = 1;
			continue;
		}
		if (isup == 0) {
			ESCAND_INFO("Interface %s not up, treat as disabled\n",
				c_info->name);
			c_info->wasdown = 1;
			continue;
		} else {
			if (c_info->wasdown == 1) {
				ESCAND_INFO("Interface %s was down, reload escand intf configuration\n",
					c_info->name);
				c_info->wasdown = 0;
				escand_start(c_info->name, c_info);
			}
		}

		if (!escand_is_mode_check(c_info->name)) {
			ESCAND_INFO("do not perform scan"
				" when ESCAND is not in AP mode\n");
			continue;
		}

		if (ticks % ESCAND_TICK_DISPLAY_INTERVAL == 0) {
			ESCAND_INFO("tick:%u\n", ticks);
		}

		if (ticks % ESCAND_ASSOCLIST_POLL == 0)  {
			escand_update_assoc_info(c_info);
		}

		if (ticks % ESCAND_STATUS_POLL == 0)
			escand_update_status(c_info);

		/*
		escand_scan_timer_check(c_info);
		*/

		if ((escand_ci_scan_check(c_info) || CI_SCAN(c_info))) {
			escand_do_ci_update(ticks, c_info);
		}
	}
	BCM_REFERENCE(ret);
}

/* service main entry */
int
main(int argc, char *argv[])
{
	int err = ESCAND_OK;
	struct timeval tv;
	char *val;
	int daemonize = 1;

	val = nvram_safe_get("escand_debug_level");
	if (strcmp(val, ""))
		escand_debug_level = strtoul(val, NULL, 0);

	argv[0] = "escand";
	ESCAND_PRINT("escand start...\n");

	if (argc > 1) {
	    if (strcmp(argv[1], "-F") == 0) {
		daemonize = 0;
	    } else {
		ESCAND_ERROR("Unknown argument\n");
		goto cleanup;
	    }
	}

	val = nvram_safe_get("escand_ifnames");
	if (!strcmp(val, "")) {
		ESCAND_ERROR("No interface specified, exiting...");
		return err;
	}

	if ((err = escand_init()))
		goto cleanup;

	escand_init_run(&d_info->escand_info);

#if !defined(DEBUG)
	if (daemonize) {
		if (daemon(1, 1) == -1) {
			ESCAND_ERROR("err from daemonize.\n");
			goto cleanup;
		}
	}
#endif // endif
	tv.tv_sec = d_info->poll_interval;
	tv.tv_usec = 0;

	/* Provide necessary info to debug_monitor for service restart */
	dm_register_app_restart_info(getpid(), 1, argv, NULL);

	/* establish a handler to handle SIGTERM. */
	signal(SIGTERM, escand_term_hdlr);
	escand_running = TRUE;
	while (escand_running) {
		/* Don't change channel when WPS is in the processing,
		 * to avoid WPS fails
		 */
		if (ESCAND_WPS_RUNNING) {
			sleep_ms(1000);
			continue;
		}

		if (tv.tv_sec == 0 && tv.tv_usec == 0) {
			d_info->ticks ++;
			tv.tv_sec = d_info->poll_interval;
			tv.tv_usec = 0;
			ESCAND_DEBUG("ticks: %d\n", d_info->ticks);
			escand_watchdog(d_info->ticks);
		}
		escand_main_loop(&tv);
	}
cleanup:
	escand_close_eventfd();
	escand_close_listenfd();
	escand_cleanup();
	return err;
}
