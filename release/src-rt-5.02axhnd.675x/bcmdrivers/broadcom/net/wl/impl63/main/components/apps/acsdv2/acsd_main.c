/*
 * ACS deamon (Linux)
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
 * $Id: acsd_main.c 785391 2020-03-24 03:49:37Z $
 */

#include <ethernet.h>
#include <bcmeth.h>
#include <bcmevent.h>
#include <802.11.h>
#include <common_utils.h>

#include "acsd_svr.h"
#include "acs_dfsr.h"

#include <signal.h>

#define ACS_CHAN_BW_160MHZ		160
#define ACS_CHAN_BW_80MHZ		80
#define ACS_CHAN_BW_40MHZ		40
#define ACS_CHAN_BW_20MHZ		20

#define ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_ADJACENT		(5)	/* adjacent channel */
#define ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_NONADJACENT_FCC	(17)	/* non-adjacent in FCC */
#define ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_NONADJACENT_ETSI	(30)	/* non-adjacent in ETSI */
#define ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_WEATHER_ETSI	(5)	/* EU weather channel */

char chanspecbuf[32];

static int acs_chspec_to_bw_mhz(chanspec_t ch);
static bool acs_are_channels_5g_adjacent(chanspec_t ch0, chanspec_t ch1);
static int acs_get_blanking_threshold();
static bool acs_is_2g_blanked_by_5g();

acsd_wksp_t *d_info;
static bool acsd_running = TRUE;

static void
acsd_term_hdlr(int sig)
{
	acsd_running = FALSE;
	return;
}

/* open a UDP packet to event dispatcher for receiving/sending data */
static int
acsd_open_eventfd()
{
	int reuse = 1;
	struct sockaddr_in sockaddr;
	int fd = ACSD_DFLT_FD;

	/* open loopback socket to communicate with event dispatcher */
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(EAPD_WKSP_DCS_UDP_SPORT);

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		ACSD_ERROR("Unable to create loopback socket\n");
		goto exit1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		ACSD_ERROR("Unable to setsockopt to loopback socket %d.\n", fd);
		goto exit1;
	}

	if (bind(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		ACSD_ERROR("Unable to bind to loopback socket %d\n", fd);
		goto exit1;
	}

	ACSD_INFO("opened loopback socket %d\n", fd);
	d_info->event_fd = fd;

	return ACSD_OK;

	/* error handling */
exit1:
	if (fd != ACSD_DFLT_FD) {
		close(fd);
	}
	return errno;
}

static int
acsd_svr_socket_init(unsigned int port)
{
	int reuse = 1;
	struct sockaddr_in s_sock;

	d_info->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (d_info->listen_fd < 0) {
		ACSD_ERROR("Socket open failed: %s\n", strerror(errno));
		return ACSD_FAIL;
	}

	if (setsockopt(d_info->listen_fd, SOL_SOCKET, SO_REUSEADDR,
		(char*)&reuse, sizeof(reuse)) < 0) {
		ACSD_ERROR("Unable to setsockopt to server socket %d.\n", d_info->listen_fd);
		return ACSD_FAIL;
	}

	memset(&s_sock, 0, sizeof(struct sockaddr_in));
	s_sock.sin_family = AF_INET;
	s_sock.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	s_sock.sin_port = htons(port);

	if (bind(d_info->listen_fd, (struct sockaddr *)&s_sock,
		sizeof(struct sockaddr_in)) < 0) {
		ACSD_ERROR("Socket bind failed: %s\n", strerror(errno));
		return ACSD_FAIL;
	}

	if (listen(d_info->listen_fd, 5) < 0) {
		ACSD_ERROR("Socket listen failed: %s\n", strerror(errno));
		return ACSD_FAIL;
	}

	return ACSD_OK;
}

static void
acsd_close_listenfd()
{
	/* close event dispatcher socket */
	if (d_info->listen_fd != ACSD_DFLT_FD) {
		ACSD_INFO("listenfd: close  socket %d\n", d_info->listen_fd);
		close(d_info->listen_fd);
		d_info->event_fd = ACSD_DFLT_FD;
	}
	return;
}

static void
acsd_close_eventfd()
{
	/* close event dispatcher socket */
	if (d_info->event_fd != ACSD_DFLT_FD) {
		ACSD_INFO("eventfd: close loopback socket %d\n", d_info->event_fd);
		close(d_info->event_fd);
		d_info->event_fd = ACSD_DFLT_FD;
	}
	return;
}

static int
acsd_validate_wlpvt_message(int bytes, uint8 *dpkt)
{
	bcm_event_t *pvt_data;

	/* the message should be at least the header to even look at it */
	if (bytes < sizeof(bcm_event_t) + 2) {
		ACSD_ERROR("Invalid length of message\n");
		goto error_exit;
	}
	pvt_data  = (bcm_event_t *)dpkt;
	if (ntohs(pvt_data->bcm_hdr.subtype) != BCMILCP_SUBTYPE_VENDOR_LONG) {
		ACSD_ERROR("%s: not vendor specifictype\n",
		       pvt_data->event.ifname);
		goto error_exit;
	}
	if (pvt_data->bcm_hdr.version != BCMILCP_BCM_SUBTYPEHDR_VERSION) {
		ACSD_ERROR("%s: subtype header version mismatch\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	if (ntohs(pvt_data->bcm_hdr.length) < BCMILCP_BCM_SUBTYPEHDR_MINLENGTH) {
		ACSD_ERROR("%s: subtype hdr length not even minimum\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	if (bcmp(&pvt_data->bcm_hdr.oui[0], BRCM_OUI, DOT11_OUI_LEN) != 0) {
		ACSD_ERROR("%s: acsd_validate_wlpvt_message: not BRCM OUI\n",
			pvt_data->event.ifname);
		goto error_exit;
	}
	/* check for wl dcs message types */
	switch (ntohs(pvt_data->bcm_hdr.usr_subtype)) {
		case BCMILCP_BCM_SUBTYPE_EVENT:

			ACSD_INFO("subtype: event\n");
			break;
		default:
			goto error_exit;
			break;
	}
	return ACSD_OK; /* good packet may be this is destined to us */
error_exit:
	return ACSD_FAIL;
}

/*
 * Receives and processes the commands from client
 * o Wait for connection from client
 * o Process the command and respond back to client
 * o close connection with client
 */
static int
acsd_proc_client_req(void)
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
			ACSD_ERROR("accept failed: errno: %d - %s\n", errno, strerror(errno));
			return -1;
		}
	}
	d_info->conn_fd = fd;

	if (!d_info->cmd_buf)
		d_info->cmd_buf = acsd_malloc(ACSD_BUFSIZE_4K);

	buf = d_info->cmd_buf;

	/* get command from client */
	if ((rcount = sread(d_info->conn_fd, buf, ACSD_BUFSIZE_4K)) < 0) {
		ACSD_ERROR("Failed reading message from client: %s\n", strerror(errno));
		ret = -1;
		goto done;
	}

	/* reqeust is small string. */
	if (rcount == ACSD_BUFSIZE_4K) {
		ACSD_ERROR("Client Req too large\n");
		ret = -1;
		goto done;
	}
	buf[rcount] = '\0';

	acsd_proc_cmd(d_info, buf, rcount, &resp_size);

	if (swrite(d_info->conn_fd, buf, resp_size) < 0) {
		ACSD_ERROR("Failed sending message: %s\n", strerror(errno));
		ret = -1;
		goto done;
	}

done:
	close(d_info->conn_fd);
	d_info->conn_fd = -1;
	return ret;
}

/* Check if stay in current channel long enough */
bool
chanim_record_chan_dwell(acs_chaninfo_t *c_info, chanim_info_t *ch_info)
{
	uint8 cur_idx = chanim_mark(ch_info).record_idx;
	uint8 start_idx;
	chanim_acs_record_t *start_record;
	time_t now = uptime();

	/* negative value implies infinite dwell time; negative implies infinity */
	if (c_info->acs_chan_dwell_time < 0) {
		return FALSE;
	}

	start_idx = MODSUB(cur_idx, 1, ACS_CHANIM_ACS_RECORD);
	start_record = &ch_info->record[start_idx];
	if (now - start_record->timestamp > c_info->acs_chan_dwell_time)
		return TRUE;
	return FALSE;
}

#ifdef ZDFS_2G

/* call this at start of ZDFS_2G CAC */
void
acs_update_tx_dur_secs_start()
{
	int i;
	acs_chaninfo_t **ci_5g_all = d_info->acs_info->ci_5g_all;

	for (i = 0; i < ACSD_MAX_INTERFACES && ci_5g_all[i]; i++) {
		ci_5g_all[i]->tx_dur_secs_start = acs_get_tx_dur_secs(ci_5g_all[i]);
	}

	d_info->acs_info->ci_zdfs_2g->zdfs_2g_start_tick = d_info->ticks;
}

/* call this at end of ZDFS_2G CAC */
void
acs_update_tx_dur_secs_end()
{
	int i;
	acs_chaninfo_t **ci_5g_all = d_info->acs_info->ci_5g_all;

	for (i = 0; i < ACSD_MAX_INTERFACES && ci_5g_all[i]; i++) {
		ci_5g_all[i]->tx_dur_secs_end = acs_get_tx_dur_secs(ci_5g_all[i]);
	}

	d_info->acs_info->ci_zdfs_2g->zdfs_2g_end_tick = d_info->ticks;
}

/* call this with chanspec to get respective bandwidth in MHz */
static int
acs_chspec_to_bw_mhz(chanspec_t ch)
{

	uint16 bw_flag = CHSPEC_BW(ch);
	int bw_mhz = 0;

	if (bw_flag == WL_CHANSPEC_BW_160) {
		bw_mhz = ACS_CHAN_BW_160MHZ;
	} else if (bw_flag == WL_CHANSPEC_BW_80) {
		bw_mhz = ACS_CHAN_BW_80MHZ;
	} else if (bw_flag == WL_CHANSPEC_BW_40) {
		bw_mhz = ACS_CHAN_BW_40MHZ;
	} else if (bw_flag == WL_CHANSPEC_BW_20) {
		bw_mhz = ACS_CHAN_BW_20MHZ;
	}

	return bw_mhz;
}

/* given two 5GHz chanspecs, returns if the two are adjacent/overlapping (TRUE) or
 * non-adjacent/separated (FALSE) in spectrum
 */
static bool
acs_are_channels_5g_adjacent(chanspec_t ch0, chanspec_t ch1)
{
	int cent0 = CHSPEC_CHANNEL(ch0), bw_mhz0 = acs_chspec_to_bw_mhz(ch0);
	int cent1 = CHSPEC_CHANNEL(ch1), bw_mhz1 = acs_chspec_to_bw_mhz(ch1);
	int bw_num_ch0 = bw_mhz0 / 5; /* channel numbers are per 5MHz */
	int bw_num_ch1 = bw_mhz1 / 5;

	if (CHSPEC_IS2G(ch0) || CHSPEC_IS2G(ch1)) {
		return FALSE;
	}

	return (ABS(cent0 - cent1) <= ((bw_num_ch0 + bw_num_ch1)/2));
}

/* Returns the ZDFS blanking threshold percent applicable based on
 *  - the channel being cleared (using ZDFS_2G) and
 *  - channels used on 5GHz interface(s)
 *  For multi 5GHz case (5GL, 5GH), even if one of those is operating on an adjacent or weather
 *  radar channel, lower thresholds are returned.
 */
static int
acs_get_blanking_threshold()
{
	acs_chaninfo_t *ci_zdfs_2g = d_info->acs_info->ci_zdfs_2g;
	acs_chaninfo_t *ci_zdfs_5g = ci_zdfs_2g->ci_5g;
	acs_chaninfo_t **ci_5g_all = d_info->acs_info->ci_5g_all;
	chanspec_t ch_zdfs_2g = ci_zdfs_2g->acs_bgdfs->next_scan_chan;
	bool is_etsi = ci_zdfs_5g->country_is_edcrs_eu;
	bool is_dfs_weather = acs_is_dfs_weather_chanspec(ci_zdfs_5g, ch_zdfs_2g);
	bool is_adj = FALSE;
	int i;

	for (i = 0; i < ACSD_MAX_INTERFACES && ci_5g_all[i]; i++) {
		if (acs_are_channels_5g_adjacent(ch_zdfs_2g, ci_5g_all[i]->cur_chspec)) {
			is_adj = TRUE;
			break;
		}
	}

	if (is_adj) {
		return ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_ADJACENT;
	}
	if (is_etsi) {
		if (is_dfs_weather) {
			return ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_WEATHER_ETSI;
		} else {
			return ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_NONADJACENT_ETSI;
		}
	} else {
		return ACS_ZDFS_BLANKING_THRESHOLD_PERCENT_NONADJACENT_FCC;
	}
}

/* Returns TRUE if traffic from 5GHz interface(s), appear to exceed the blanking thresholds.
 * Limitations:
 *  - Does a pessimistic sum of traffic where there are multiple 5GHz interfaces (as traffic
 *    duration overlap) can not be determined reliably.
 *  - Applies common low threshold even if just one of the 5GHz interfaces is operating on
 *    an adjacent/weather radar channel.
 */
static bool
acs_is_2g_blanked_by_5g()
{
	int i, total = 0, duration, percent = 0, threshold = 100;
	acs_chaninfo_t **ci_5g_all = d_info->acs_info->ci_5g_all;
	acs_chaninfo_t *ci_zdfs_2g = d_info->acs_info->ci_zdfs_2g;
	chanspec_t ch_zdfs_2g = ci_zdfs_2g->acs_bgdfs->next_scan_chan;
	bool ret = FALSE;

	for (i = 0; i < ACSD_MAX_INTERFACES && ci_5g_all[i]; i++) {
		total += ci_5g_all[i]->tx_dur_secs_end - ci_5g_all[i]->tx_dur_secs_start;
	}

	duration = ci_zdfs_2g->zdfs_2g_end_tick - ci_zdfs_2g->zdfs_2g_start_tick;
	if (duration > 0) {
		percent = (total * 100) / duration;
	}

	percent = (percent < 0 ? (0) : (percent > 100 ? 100 : percent));

	threshold = acs_get_blanking_threshold();

	ret = (percent > threshold);

	ACSD_INFO("%s: %s retd: %d, ch: 0x%4x, per:%d th:%d\n",
			ci_zdfs_2g->name, __func__, ret, ch_zdfs_2g, percent, threshold);

	return ret;
}

/* returns the c_info corresponding to a zdfs 2g interface if available else NULL */
acs_chaninfo_t*
acs_get_zdfs_2g_ci()
{
	return d_info->acs_info->ci_zdfs_2g;
}

#endif /* ZDFS_2G */
/* This function compares the exclude_ifname with ifnames and
 * increment the excl_cnt if it matches
 */
int
acs_compare_iface_list(char *exclude_ifname)
{
	int i, excl_cnt = 0;
	if (exclude_ifname != NULL) {
		for (i = 0; i < 16 && d_info->acs_info->exclude_ifnames[i] != NULL; i++) {
			if (!strncmp(d_info->acs_info->exclude_ifnames[i], exclude_ifname,
					strlen(exclude_ifname))) {
				excl_cnt++;
			}
		}
	}
	return excl_cnt;
}

/* On txfail event, change the channel only if it crosses dwell time period */
static int
acs_channel_trigger(acs_chaninfo_t *c_info, char *ifname, unsigned char *addr)
{
	int ret = 0, bw = 0;
	bool chan_least_dwell = FALSE;
	time_t now = uptime();

	chan_least_dwell = chanim_record_chan_dwell(c_info,
			c_info->chanim_info);

	if (!chan_least_dwell) {
		ACSD_5G("%s: chan_least_dwell is FALSE\n", c_info->name);
		return ret;
	}

	if (!c_info->traffic_thresh) {
		if (acs_update_rssi(c_info, addr)) {
			ACSD_ERROR("intf:%s not able to update the rssi"
				"value\n", c_info->name);
			return ret;
		}
		if (c_info->acs_ignore_txfail_on_far_sta) {
			if (c_info->sta_status & ACS_STA_EXIST_FAR) {
				ACSD_5G(" intf: %s Ignoring txfail for the far "
					"sta %d\n", c_info->name,
					c_info->sta_status);
			return ret;
			}
		}
	}

	if (c_info->acs_txop_thresh) {
		if (c_info->txop_score >= c_info->acs_txop_thresh) {
			ACSD_5G("Ignoring txfail if txop score is"
				"greater than threshold ifnmae :%s"
				" txop score : %d and txop threshold: %d\n",
				c_info->name, c_info->txop_score,
				c_info->acs_txop_thresh);
			return ret;
		}
	}

	if (c_info->country_is_edcrs_eu) {
		c_info->txfail_event_timeout = d_info->ticks + TXFAIL_EVENT_TIMEOUT_ETSI;
	} else {
		c_info->txfail_event_timeout = d_info->ticks + TXFAIL_EVENT_TIMEOUT_FCC;
	}

	if (acsd_trigger_dfsr_check(c_info)) {
		ACSD_DFSR("%s: trigger DFS reentry...\n", c_info->name);
		acs_dfsr_set(ACS_DFSR_CTX(c_info),
			c_info->cur_chspec, __FUNCTION__);
		return ret;
	}

	if (!acsd_need_chan_switch(c_info)) {
		ACSD_5G("%s: No channel switch...\n", c_info->name);
		return ret;
	}

	if (c_info->cur_is_dfs) {
		acs_get_best_dfs_forced_chspec(c_info);
		c_info->selected_chspec = acs_adjust_ctrl_chan(
			c_info,	c_info->dfs_forced_chspec);
		if (!acs_channel_compare(c_info, c_info->cur_chspec, c_info->selected_chspec)) {
			return FALSE;
		}
		ACSD_5G("%s: Select 0x%x\n", c_info->name,
			c_info->selected_chspec);
	} else {
		c_info->switch_reason = APCS_TXFAIL;
		acs_select_chspec(c_info);
	}

	if (!acs_bgdfs_attempt_on_txfail(c_info)) {
		bw = acs_derive_bw_from_given_chspec(c_info);
		if (ACS_11H(c_info) && !c_info->cur_is_dfs && (c_info->acs_bgdfs == NULL) &&
				(c_info->country_is_edcrs_eu ||
				!acs_nondfs_chan_check_for_bgdfs_or_dfsr_trigger(c_info, bw))) {
			acs_dfsr_set_reentry_type(ACS_DFSR_CTX(c_info), DFS_REENTRY_IMMEDIATE);
			c_info->switch_reason = APCS_DFS_REENTRY;
			if (!acs_select_chspec(c_info)) {
				return ret;
			}
		}

		if (CHSPEC_CHANNEL(c_info->selected_chspec) ==
				CHSPEC_CHANNEL(c_info->recent_prev_chspec)) {
			if (now - c_info->acs_prev_chan_at < 2 * c_info->acs_chan_dwell_time) {
				ACSD_INFO("%s: staying on same channel because of prev_chanspec"
						"dwell restrictions\n", c_info->name);
				return ret;
			}
		}

		if (c_info->fallback_to_primary && CHSPEC_CHANNEL(c_info->selected_chspec) ==
				CHSPEC_CHANNEL(c_info->cur_chspec)) {
			ACSD_INFO("%s: Seleted = cur 0x%x avoid CSA\n",
					c_info->name, c_info->cur_chspec);
			return ret;
		}

		ACSD_PRINT("%s received event: MAC tx failures (exhaustion of 802.11 retries) exceeding threshold(s)\n", c_info->name);

		ACSD_INFO("%s Performing CSA on chspec 0x%x\n",
				c_info->name, c_info->selected_chspec);
		if (!(ret = acs_csa_handle_request(c_info))) {
			c_info->recent_prev_chspec = c_info->cur_chspec;
			c_info->acs_prev_chan_at = uptime();
			if (!c_info->traffic_thresh) {
				acs_intfer_config_txfail(c_info);
			}
			chanim_upd_acs_record(c_info->chanim_info,
				c_info->selected_chspec, APCS_TXFAIL);
		}
	}
	return ret;
}

/* set chanspec_info for all 5GHz interfaces supporting 11h */
static int
acsd_set_5g_chanspec_info(chanspec_t chspec, uint8 bmp, acs_chan_info_bmp_t bmp_type)
{
	acs_chaninfo_t* c_info;
	int i, err = BCME_OK;

	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		c_info = d_info->acs_info->chan_info[i];

		if ((!c_info) || (c_info->mode == ACS_MODE_DISABLE)) {
			continue;
		}
		/* skip if 2G or not 11h */
		if (BAND_2G(c_info->rs_info.band_type) || !ACS_11H(c_info)) {
			continue;
		}
		err = acs_set_chanspec_info(c_info, chspec, bmp, bmp_type);
		ACSD_INFO("%s: set chanspec_info returned %d for ch:0x%04x", c_info->name,
				err, chspec);
	}

	return err;
}

/* listen to sockets and call handlers to process packets */
void
acsd_main_loop(struct timeval *tv)
{
	fd_set fdset;
	int width, status = 0, bytes, len;
	uint8 *pkt;
	bcm_event_t *pvt_data;
	bool chan_least_dwell = FALSE;
	wl_bcmdcs_data_t dcs_data;
	acs_chaninfo_t *c_info;
	int idx = 0;
	int err;
	uint32 escan_event_status;
	wl_escan_result_t *escan_data = NULL;
	struct escan_bss *result;
	acs_chaninfo_t *zdfs_2g_ci = NULL;

#ifdef DEBUG
	char test_cswitch_ifname[32];

	strncpy(test_cswitch_ifname, nvram_safe_get("acs_test_cs"), sizeof(test_cswitch_ifname));
	test_cswitch_ifname[sizeof(test_cswitch_ifname) - 1] = '\0';
	nvram_unset("acs_test_cs");

	if (test_cswitch_ifname[0]) {
		if ((idx = acs_idx_from_map(test_cswitch_ifname)) < 0) {
			ACSD_WARNING("cannot find the mapped entry for ifname: %s\n",
				test_cswitch_ifname);
			return;
		}
		c_info = d_info->acs_info->chan_info[idx];

		ACSD_5G("trigger Fake cswitch from:0x%x:...\n", c_info->cur_chspec);

		if (acsd_trigger_dfsr_check(c_info)) {
			ACSD_DFSR("trigger DFS reentry...\n");
			acs_dfsr_set(ACS_DFSR_CTX(c_info), c_info->cur_chspec, __FUNCTION__);
		}
		else {
			if (c_info->cur_is_dfs) {
				acs_get_best_dfs_forced_chspec(c_info);
				c_info->selected_chspec = acs_adjust_ctrl_chan(c_info,
					c_info->dfs_forced_chspec);
				ACSD_5G("Select 0x%x:...\n", c_info->selected_chspec);
			}
			else {
				c_info->switch_reason = APCS_TXFAIL;
				acs_select_chspec(c_info);
			}
			if (!(err = acs_csa_handle_request(c_info))) {
				if (!c_info->traffic_thresh) {
					acs_intfer_config_txfail(c_info);
				}
				chanim_upd_acs_record(c_info->chanim_info,
						c_info->selected_chspec, APCS_TXFAIL);
			}
		}
	}
#endif /* DEBUG */

	/* init file descriptor set */
	FD_ZERO(&d_info->fdset);
	d_info->fdmax = -1;

	/* build file descriptor set now to save time later */
	if (d_info->event_fd != ACSD_DFLT_FD) {
		FD_SET(d_info->event_fd, &d_info->fdset);
		d_info->fdmax = d_info->event_fd;
	}

	if (d_info->listen_fd != ACSD_DFLT_FD) {
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
		ACSD_ERROR("err from select: %s", strerror(errno));
		return;
	}

	if (d_info->listen_fd != ACSD_DFLT_FD && FD_ISSET(d_info->listen_fd, &fdset)) {
		d_info->stats.num_cmds++;
		acsd_proc_client_req();
	}

	/* handle brcm event */
	if (d_info->event_fd !=  ACSD_DFLT_FD && FD_ISSET(d_info->event_fd, &fdset)) {
		char *ifname = (char *)pkt;
		struct ether_header *eth_hdr = (struct ether_header *)(ifname + IFNAMSIZ);
		uint16 ether_type = 0;
		uint32 evt_type;

		ACSD_INFO("recved event from eventfd\n");

		d_info->stats.num_events++;

		if ((bytes = recv(d_info->event_fd, pkt, len, 0)) <= 0)
			return;

		ACSD_INFO("recved %d bytes from eventfd, ifname: %s\n",
				bytes, ifname);
		bytes -= IFNAMSIZ;

		if (strncmp(ifname, "wds", 3) == 0) {
			ACSD_INFO("discard all wds events\n");
			return;
		}

		if ((ether_type = ntohs(eth_hdr->ether_type) != ETHER_TYPE_BRCM)) {
			ACSD_INFO("recved ether type %x\n", ether_type);
			return;
		}

		if ((err = acsd_validate_wlpvt_message(bytes, (uint8 *)eth_hdr)))
			return;

		pvt_data = (bcm_event_t *)(ifname + IFNAMSIZ);
		evt_type = ntoh32(pvt_data->event.event_type);
		ACSD_INFO("recved brcm event, event_type: %d\n", evt_type);

		if ((evt_type == WLC_E_TXFAIL_THRESH || evt_type == WLC_E_TXFAIL_TRFTHOLD) &&
				(acs_compare_iface_list(ifname))) {
			ACSD_INFO("ignore txfail for this excluded intf %s\n", ifname);
			return;
		}

		acs_check_ifname_is_virtual(&ifname);

		if ((idx = acs_idx_from_map(ifname)) < 0) {
			ACSD_WARNING("cannot find the mapped entry for ifname: %s\n", ifname);
			return;
		}

		c_info = d_info->acs_info->chan_info[idx];

		if (c_info->mode == ACS_MODE_DISABLE && c_info->acs_boot_only) {
			ACSD_INFO("No event handling enabled. Only boot selection \n");
			return;
		}

		if (!AUTOCHANNEL(c_info) && !COEXCHECK(c_info) &&
				evt_type != WLC_E_REQ_BW_CHANGE &&
				evt_type != WLC_E_RADAR_DETECTED) {
			ACSD_INFO("Event fail ACSD not in autochannel/coex modes \n");
			return;
		}

		if ((evt_type != WLC_E_ESCAN_RESULT) && c_info->acs_escan->acs_escan_inprogress) {
			ACSD_INFO("when scan is in progress, don't allow other"
				" events for processing\n");
			return;
		}

		/* If on Fixed chspec and Obss Coex mode, allow only escan events */
		if (COEXCHECK(c_info) &&
			(evt_type != WLC_E_SCAN_COMPLETE && evt_type != WLC_E_ESCAN_RESULT)) {
			ACSD_INFO("In coex mode, discard events other than escan %d\n", evt_type);
			return;
		}

		if (evt_type == WLC_E_RADAR_DETECTED) {
			/* 5G */
			if (BAND_5G(c_info->rs_info.band_type)) {
				/* no processing of radar detected signal from 5G interfaces */
				ACSD_INFO("%s: Ignore radar event in 5G\n", c_info->name);
				return;
			}
			/* 2G */
			if (c_info->acs_zdfs_2g_ignore_radar) {
				/* since configured to ignore radar from 2g, return */
				ACSD_INFO("%s: Ignore radar event in 2G\n", c_info->name);
				return;
			}
		}

		if (c_info->acs_bgdfs != NULL && c_info->acs_bgdfs->state != BGDFS_STATE_IDLE &&
			evt_type != WLC_E_REQ_BW_CHANGE && BAND_5G(c_info->rs_info.band_type)) {
			ACSD_INFO("ACSD avoid events During BGDFS evt_type %d acs_bgdfs state %d\n",
				evt_type, c_info->acs_bgdfs->state);
			return;
		}

		if ((zdfs_2g_ci = acs_get_zdfs_2g_ci()) != NULL && zdfs_2g_ci->acs_bgdfs != NULL &&
				zdfs_2g_ci->acs_bgdfs->state != BGDFS_STATE_IDLE) {
			ACSD_INFO("%s: Ignore acsd events During ZDFS_2G and state is %d\n",
					c_info->name, zdfs_2g_ci->acs_bgdfs->state);
			return;
		}

		/* Wi-Fi Blanket Repeater needs to run in Fixed Chanspec mode, to implement this
		 * behaviour, WBD needs ACSD not to change channel, if not told to do so through CLI
		 */
		if (FIXCHSPEC(c_info) && (evt_type == WLC_E_TXFAIL_THRESH ||
				evt_type == WLC_E_TXFAIL_TRFTHOLD)) {
			ACSD_INFO("ACSD mode is FIXCHSPEC, avoid channel switch\n");
			return;
		}

		if (BAND_5G(c_info->rs_info.band_type) &&
				ACS_11H(c_info) && (c_info->acs_bgdfs == NULL) &&
				(evt_type == WLC_E_TXFAIL_TRFTHOLD ||
				evt_type == WLC_E_TXFAIL_THRESH) &&
				c_info->txfail_event_timeout > d_info->ticks) {
			ACSD_INFO("%s: Avoid back to back txfail events timeout:%u ticks:%u\n",
					c_info->name, c_info->txfail_event_timeout, d_info->ticks);
			return;
		}

		if (BAND_2G(c_info->rs_info.band_type) &&
			(evt_type == WLC_E_TXFAIL_TRFTHOLD || evt_type == WLC_E_TXFAIL_THRESH)) {
			ACSD_INFO("Avoid TXfail events when in 2G band\n");
			return;
		}

		if (c_info->ignore_txfail) {
			if (evt_type == WLC_E_TXFAIL_THRESH || evt_type == WLC_E_TXFAIL_TRFTHOLD) {
				ACSD_INFO("Avoid changing the channels for txfail events as of"
					"now\n");
				return;
			}
		}

		if (c_info->wet_enabled && acs_check_assoc_scb(c_info)) {
			ACSD_INFO("avoid channel switch when ACSD is in WET mode and scb associated\n");
			return;
		}

		d_info->stats.valid_events++;

		switch (evt_type) {
			case WLC_E_DCS_REQUEST:
			{
				dot11_action_wifi_vendor_specific_t * actfrm;
				actfrm = (dot11_action_wifi_vendor_specific_t *)(pvt_data + 1);

				if ((err = dcs_parse_actframe(actfrm, &dcs_data))) {
					ACSD_ERROR("err from dcs_parse_request: %d\n", err);
					break;
				}

				ACSD_PRINT("%s received event: DCS request\n", c_info->name);

				if ((err = dcs_handle_request(ifname, &dcs_data,
						DOT11_CSA_MODE_ADVISORY, DCS_CSA_COUNT,
						CSA_BROADCAST_ACTION_FRAME)))
					ACSD_ERROR("err from dcs_handle_req: %d\n", err);

				break;
			}
			case WLC_E_SCAN_COMPLETE:
			{
				ACSD_INFO("recved brcm event: scan complete\n");
				break;
			}
			case WLC_E_PKTDELAY_IND:
			{
				txdelay_event_t pktdelay;

				memcpy(&pktdelay, (txdelay_event_t *)(pvt_data + 1),
						sizeof(txdelay_event_t));
				/* stay in current channel more than acs_chan_dwell_time */
				chan_least_dwell = chanim_record_chan_dwell(c_info,
						c_info->chanim_info);
				if (chan_least_dwell &&
						(pktdelay.chanim_stats.chan_idle <
						 c_info->acs_ci_scan_chanim_stats)) {
					ACSD_PRINT("%s received event: tx pkt delay suddently jump\n", c_info->name);

					c_info->switch_reason = APCS_TXDLY;
					acs_select_chspec(c_info);

					if (CHSPEC_CHANNEL(c_info->selected_chspec) ==
							CHSPEC_CHANNEL(c_info->cur_chspec)) {
						ACSD_INFO("%s: Seleted = cur 0x%x avoid CSA\n",
								c_info->name, c_info->cur_chspec);
						break;
					}

					c_info->recent_prev_chspec = c_info->cur_chspec;
					c_info->acs_prev_chan_at = uptime();

					if (!acs_csa_handle_request(c_info)) {
						chanim_upd_acs_record(c_info->chanim_info,
							c_info->selected_chspec, APCS_TXDLY);
					}
				}
				break;
			}
			case WLC_E_TXFAIL_THRESH:
			{
				wl_intfer_event_t *event;
				unsigned char *addr;

				/* ensure we have the latest channel information and
				   dwell time etc
				   */
				acs_update_status(c_info);

				event = (wl_intfer_event_t *)(pvt_data + 1);
				addr = (unsigned char *)(&(pvt_data->event.addr));

				ACSD_5G("Intfer:%s Mac:%02x:%02x:%02x:%02x:%02x:%02x"
					"status = 0x%x\n", ifname, addr[0], addr[1],
					addr[2], addr[3], addr[4], addr[5], event->status);

				for (idx = 0; idx < WLINTFER_STATS_NSMPLS; idx++) {
					ACSD_5G("0x%x\t", event->txfail_histo[idx]);
				}

				ACSD_5G("\n time:%u", (uint32)uptime());
				acs_channel_trigger(c_info, ifname, addr);

				break;
			}
			case WLC_E_TXFAIL_TRFTHOLD:
			{
				wlc_traffic_thresh_event_t *event;
				unsigned char *addr;

				/* ensure we have the latest channel information and
				   dwell time etc
				   */
				acs_update_status(c_info);

				event = (wlc_traffic_thresh_event_t *)(pvt_data + 1);
				addr = (unsigned char *)(&(pvt_data->event.addr));

				ACSD_INFO("Intfer:%s Mac:%02x:%02x:%02x:%02x:%02x:%02x",
					ifname, addr[0], addr[1], addr[2], addr[3],
					addr[4], addr[5]);

				ACSD_INFO("Inter type:%d version %d length %d count %d \n",
					event->type, event->version, event->length,
					event->count);

				ACSD_5G("\n time:%u", (uint32)uptime());
				acs_channel_trigger(c_info, ifname, addr);

				break;
			}
			case WLC_E_ESCAN_RESULT:
			{
				if (!c_info->acs_escan->acs_escan_inprogress ||
						!c_info->acs_escan->acs_use_escan) {
					ACSD_INFO("ACSD Escan not triggered from ACSD\n");
					return;
				}

				escan_event_status = ntoh32(pvt_data->event.status);
				escan_data = (wl_escan_result_t*)(pvt_data + 1);

				if (escan_event_status == WLC_E_STATUS_PARTIAL) {
					wl_bss_info_t *bi = &escan_data->bss_info[0];
					wl_bss_info_t *bss;

					/* check if we've received info of same BSSID */
					for (result = c_info->acs_escan->escan_bss_head;
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
						ebss = (struct escan_bss *)acsd_malloc(
							OFFSETOF(struct escan_bss, bss)
							+ bi->length);

						if (!ebss) {
							ACSD_ERROR("can't allocate memory"
									"for escan bss");
							break;
						}

						ebss->next = NULL;
						memcpy(&ebss->bss, bi, bi->length);
						if (c_info->acs_escan->escan_bss_tail) {
							c_info->acs_escan->escan_bss_tail->next =
							ebss;
						} else {
							c_info->acs_escan->escan_bss_head =
							ebss;
						}

						c_info->acs_escan->escan_bss_tail = ebss;
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
					c_info->timestamp_acs_scan = uptime();
					if (c_info->acs_escan->scan_type == ACS_SCAN_TYPE_CS) {
						c_info->timestamp_tx_idle =
							c_info->timestamp_acs_scan;
					}
#ifdef ACS_DEBUG
					/* print scan results */
					for (result = c_info->acs_escan->escan_bss_head;
						result;	result = result->next) {
						dump_bss_info(result->bss);
					}
#endif // endif
					c_info->acs_escan->acs_escan_inprogress = FALSE;
					ACSD_INFO("Escan success!\n");
				} else {
					ACSD_ERROR("sync_id: %d, status:%d, misc."
						"error/abort\n",
						escan_data->sync_id, status);

					acs_escan_free(c_info->acs_escan->escan_bss_head);
					c_info->acs_escan->escan_bss_head = NULL;
					c_info->acs_escan->escan_bss_tail = NULL;
					c_info->acs_escan->acs_escan_inprogress = FALSE;
				}
				break;
			}
			case WLC_E_MODE_SWITCH:
			{
				wl_event_mode_switch_t *ev_ms =
					(wl_event_mode_switch_t *) (pvt_data + 1);
				wl_event_mode_switch_dyn160 *data_dyn160;
				if (ev_ms->version != WL_EVENT_MODESW_VER_1) {
					ACSD_ERROR("%s: Unsupported modesw event ver %d",
							c_info->name, ev_ms->version);
					break;
				}
				if (ev_ms->reason != WL_E_MODESW_REASON_DYN160) {
					break;
				}
				if (bytes < ev_ms->data_offset + sizeof(*data_dyn160)) {
					ACSD_ERROR("%s: invalid event data offset %d",
							c_info->name, ev_ms->data_offset);
					break;
				}
				data_dyn160 = (wl_event_mode_switch_dyn160 *)
					((uint8 *)ev_ms) + ev_ms->data_offset;
				if (data_dyn160->trigger == 0) {
					c_info->is_mu_active = FALSE;
				} else if (data_dyn160->trigger == 1) {
					c_info->is_mu_active = TRUE;
				}
				break;
			}
			/* This is used only by bw_switch_160 IOVar by FW to upgrade to 160Mhz
			 * with same primary channel Eg: Upgrade from 36/80 to 36/160.
			 * Note that prior to this, BW is downgraded to 80Mhz due to
			 * bw_switch_160 IOVar.
			 */
			case WLC_E_REQ_BW_CHANGE:
			{
				wl_event_req_bw_upgd_t *data;
				chanspec_t upgrd_chspec;

				data = (wl_event_req_bw_upgd_t *)(pvt_data + 1);

				if ((data->length != WL_EVENT_REQ_BW_UPGD_LEN) ||
					(data->version != WL_EVENT_REQ_BW_UPGD_VER_1)) {
					ACSD_ERROR("%s: Invalid BW UPGD REQ event ver %d len %d\n",
						c_info->name, data->version, data->length);
					break;
				}
				upgrd_chspec = data->upgrd_chspec;
				if (!CHSPEC_IS160(upgrd_chspec)) {
					ACSD_ERROR("%s: Not 160Mhz chanspec 0x%x in BW UPGD REQ\n",
						c_info->name, upgrd_chspec);
					break;
				}
				c_info->acs_req_bw_upgrd = TRUE;
				ACSD_INFO("%s Received REQ_BW_UPGRADE event %d for chanspec 0x%x\n",
					c_info->name, WLC_E_REQ_BW_CHANGE, upgrd_chspec);
				/* Trigger BGDFS even when ACSD is not configured in Auto. */
				if (acs_bgdfs_attempt(c_info, upgrd_chspec, FALSE) != BCME_OK) {
					ACSD_ERROR("%s dfs_ap_move Failed\n", c_info->name);
				}
				c_info->acs_req_bw_upgrd = FALSE;
				break;
			}
			case WLC_E_RADAR_DETECTED:
			{
				chanspec_t chspec;
				uint16 subband;
				wl_event_radar_detect_data_t *radar_data;
				radar_data = (wl_event_radar_detect_data_t *)(pvt_data + 1);
				chspec = dtoh16(radar_data->current_chanspec);
				subband = dtoh16(radar_data->radar_info[0].subband);
				ACSD_INFO("%s: Handling the radar event %d\n", c_info->name,
						evt_type);
				err = acsd_set_5g_chanspec_info(chspec, (uint8)(subband & 0xFFu),
						ACS_CHAN_INFO_INACTIVE);
				if (c_info->acs_bgdfs) {
					c_info->acs_bgdfs->state = BGDFS_STATE_IDLE;
					c_info->cac_mode = ACS_CAC_MODE_AUTO;
				}
				if (c_info->ci_5g && c_info->ci_5g->acs_bgdfs) {
					c_info->ci_5g->acs_bgdfs->next_scan_chan = 0;
				}
				break;
			}
			default:
				ACSD_INFO("recved event type %x\n", evt_type);
				break;
		}
	}
}

/*
 * acs_upgrade_downgrade_opermode - upgrade/downgrade by oper_mode
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Return void.
 */
static void
acs_upgrade_downgrade_opermode(acs_chaninfo_t * c_info)
{
	acs_rsi_t *rsi = &c_info->rs_info;

	if (!WL_BW_CAP_160MHZ(rsi->bw_cap)) {
		return;
	}

	if (ACS_CHINFO_IS_UNCLEAR(acs_get_chanspec_info(c_info, c_info->cur_chspec))) {
		ACSD_DEBUG("%s Chanspec 0x%x NOT cleared or CAC in Progress\n",
			c_info->name, c_info->cur_chspec);
		return;
	}

	/* Upgrade to 160Mhz by Oper_mode */
	if (ACS_11H(c_info) && c_info->is160_upgradable &&
		c_info->dyn160_enabled && ACS_OP_BW_IS_80(c_info->oper_mode)) {
		chanspec_t upgrade_chspc;

		upgrade_chspc = wf_channel2chspec(wf_chspec_ctlchan(c_info->cur_chspec),
			WL_CHANSPEC_BW_160);

		if (CHSPEC_IS160(upgrade_chspc) && !wf_chspec_malformed(upgrade_chspc) &&
			ACS_CHINFO_IS_CLEARED(acs_get_chanspec_info(c_info, upgrade_chspc))) {
			ACSD_INFO("%s Upgrading to 160 Mhz chanspec 0x%x by oper_mode \n",
				c_info->name, upgrade_chspc);
			/* Upgrade to 160Mhz done in two steps, 0x112 and then to 0x116 */
			if (c_info->oper_mode == ACS_OP_4NSS_80) {
				if (acs_set_oper_mode(c_info, ACS_OP_2NSS_80) != BCME_OK) {
					ACSD_ERROR("%s Upgrade to 160 Mhz Step 1 Failed\n",
						c_info->name);
				}
			} else if (c_info->oper_mode == ACS_OP_2NSS_80) {
				if (acs_set_oper_mode(c_info, ACS_OP_2NSS_160) != BCME_OK) {
					ACSD_ERROR("%s Upgrade to 160 Mhz Step 2 Failed\n",
						c_info->name);
				}
			}
		}
	}

	/* Downgrade to 80Mhz by Oper_mode */
	if (ACS_11H(c_info) && c_info->is160_downgradable &&
		ACS_OP_BW_IS_160_80p80(c_info->oper_mode)) {
		ACSD_INFO("%s Downgrade to 80 Mhz by oper_mode \n", c_info->name);
		/* from 160MHz to 80MHz */
		if (acs_set_oper_mode(c_info, ACS_OP_2NSS_80) != BCME_OK) {
			ACSD_ERROR("%s Downgrade to 80Mhz Failed\n", c_info->name);
		}
	}
}

static int
acsd_init(void)
{
	int err = ACSD_OK;
	uint  port = ACSD_DFLT_CLI_PORT;

	d_info = (acsd_wksp_t*)acsd_malloc(sizeof(acsd_wksp_t));

	d_info->version = ACSD_VERSION;
	d_info->fdmax = ACSD_DFLT_FD;
	d_info->event_fd = ACSD_DFLT_FD;
	d_info->listen_fd = ACSD_DFLT_FD;
	d_info->conn_fd = ACSD_DFLT_FD;
	d_info->poll_interval = ACSD_DFLT_POLL_INTERVAL;
	d_info->ticks = 0;
	d_info->cmd_buf = NULL;

	err = acsd_open_eventfd();

	if (err == ACSD_OK)
		err = acsd_svr_socket_init(port);

	return err;
}

static void
acsd_cleanup(void)
{
	if (d_info) {
		if (d_info->acs_info)
			acs_cleanup(&d_info->acs_info);
		ACS_FREE(d_info->cmd_buf);
		free(d_info);
	}
}
static int
acs_bgdfs_radar_detect(acs_chaninfo_t *c_info)
{
	wl_dfs_ap_move_status_v2_t *status_v2 = NULL;
	const char *dfs_state_str[DFS_SCAN_S_MAX] = {
		"Radar Free On Channel",
		"Radar Found On Channel",
		"Radar Scan In Progress",
		"Radar Scan Aborted",
		"RSDB Mode switch in Progress For Scan"
	};
	status_v2 = &c_info->acs_bgdfs->status;
	ACSD_INFO("%s: version=%d, move status=%d\n", c_info->name,
		status_v2->version, status_v2->move_status);
	if (status_v2->move_status == DFS_SCAN_S_RADAR_FOUND) {
		ACSD_INFO("%s: %s\n", c_info->name,
			dfs_state_str[status_v2->move_status]);
		return TRUE;
	} else {
		ACSD_INFO("%s: dfs AP move in IDLE state\n", c_info->name);
		return FALSE;
	}
}
static void
acsd_watchdog(uint ticks)
{
	int i, ret;
	acs_chaninfo_t* c_info;
	acs_bgdfs_info_t * bgdfs;

	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		c_info = d_info->acs_info->chan_info[i];

		if ((!c_info) || (c_info->mode == ACS_MODE_DISABLE) || !AUTOCHANNEL(c_info)) {
			continue;
		}
		bgdfs = c_info->acs_bgdfs;

		if (c_info->wet_enabled && acs_check_assoc_scb(c_info)) {
			ACSD_INFO("%s: do not perform scan or no channel change,"
				" when ACSD is in WET mode and scb associated\n", c_info->name);
			continue;
		}

		if (ticks % ACS_TICK_DISPLAY_INTERVAL == 0) {
			ACSD_INFO("tick:%u\n", ticks);
		}

		if (ticks % ACS_ASSOCLIST_POLL == 0)  {
			acs_update_assoc_info(c_info);
			if (c_info->dyn160_cap) {
				acs_update_dyn160_status(c_info);
			}
		}
#ifdef ZDFS_2G
		if (c_info == acs_get_zdfs_2g_ci() &&
				bgdfs->state != BGDFS_STATE_IDLE) {
			time_t now = uptime();
			wl_bcmdcs_data_t dcs_data;
			int err;
			chanspec_t last_cleared_chanspec;
			if ((ret = acs_bgdfs_get(c_info)) != BGDFS_CAP_TYPE0) {
				ACSD_ERROR("acs bgdfs get failed with %d\n", ret);
			}
			if (bgdfs->timeout < now) {
				bgdfs->state = BGDFS_STATE_IDLE;
				c_info->cac_mode = ACS_CAC_MODE_AUTO;
				acs_update_tx_dur_secs_end();
				last_cleared_chanspec = BGDFS_SUB_LAST(&c_info->acs_bgdfs->status,
						BGDFS_SUB_SCAN_CORE);
				if (last_cleared_chanspec == bgdfs->next_scan_chan &&
						!acs_is_2g_blanked_by_5g() &&
						!acs_bgdfs_radar_detect(c_info)) {
					acs_chaninfo_t *ci_5g = c_info->ci_5g;

					/* clearing the passive bit */
					ret = acs_set_chanspec_info(ci_5g,
						bgdfs->next_scan_chan, FALSE, ACS_CHAN_INFO_ACTIVE);
					if (ret != BCME_OK) {
						ACSD_ERROR("%s: Failed to clear the chanspec"
							"0x%02x \n", c_info->name,
							bgdfs->next_scan_chan);
						continue;
					}
					ACSD_INFO("%s:2g last cleared channel is 0x%x\n",
						c_info->name, bgdfs->next_scan_chan);

					/* If 2G BGDFS is not stunted, switch channel
					 * on 5G Interface by CSA
					 */
					if (!(bgdfs->bgdfs_stunted)) {
						dcs_data.reason = 0;
						dcs_data.chspec = bgdfs->next_scan_chan;
						ACSD_INFO("%s:Initiate csa on channel 0x%x\n",
							ci_5g->name, bgdfs->next_scan_chan);
						if ((err = dcs_handle_request(ci_5g->name,
								&dcs_data,
								DOT11_CSA_MODE_ADVISORY,
								ACS_CSA_COUNT,
								ci_5g->acs_dcs_csa))) {
							ACSD_ERROR("%sErr dcs_handle_request:%d\n",
								ci_5g->name, err);
						}
						ci_5g->selected_chspec = bgdfs->next_scan_chan;
						if (!(ci_5g->switch_reason_type ==
								APCS_DFS_REENTRY)) {
							chanim_upd_acs_record(ci_5g->chanim_info,
								ci_5g->selected_chspec, APCS_ZDFS);
							ci_5g->recent_prev_chspec =
								ci_5g->cur_chspec;
							ci_5g->acs_prev_chan_at =
								uptime();
							acs_update_status(c_info->ci_5g);
						}
					}
					/* Clear 5G radio's channel to be scanced next.
					 * This is the channel on which next BGDFS_2G
					 * may be attempted.
					 */
					ci_5g->acs_bgdfs->next_scan_chan = 0;
					/* Resetting it to null due to successful completion
					 * of ZDFS_2G attempt
					 */
					c_info->ci_5g = 0;
					ci_5g->ci_2g = 0;
				} else if (c_info->ci_5g->acs_bgdfs->fallback_blocking_cac &&
						!acs_bgdfs_radar_detect(c_info) &&
						c_info->acs_bgdfs->acs_bgdfs_on_txfail) {
					acs_chaninfo_t *ci_5g = c_info->ci_5g;
					if (ci_5g->txop_score > ci_5g->acs_txop_limit_hi) {
						ACSD_INFO("ifname: %s Not initiating fallback"
						" due to txop thresh being high %d compared to"
						"thresh: %d\n", ci_5g->name, ci_5g->txop_score,
						c_info->acs_txop_limit_hi);
						continue;
					}
					if (!acs_channel_compare_before_fallback(ci_5g,
							ci_5g->cur_chspec,
							bgdfs->next_scan_chan))
					{
						ACSD_INFO("ifname: %s Not initiating fallback"
						" due to thresh being low\n", ci_5g->name);
						continue;
					}
					wl_bcmdcs_data_t dcs_data;
					int err;
					ACSD_INFO("ifname: %s BGDFS Failed. Do Full MIMO CAC \n",
							c_info->name);

					if (!(bgdfs->bgdfs_stunted)) {
						dcs_data.reason = 0;
						dcs_data.chspec = bgdfs->next_scan_chan;
						if ((err = dcs_handle_request(ci_5g->name,
							&dcs_data,
							DOT11_CSA_MODE_ADVISORY,
							ACS_CSA_COUNT,
							ci_5g->acs_dcs_csa))) {
							ACSD_ERROR("%s Error dcs_handle_request:"
								"%d\n", ci_5g->name, err);
							}
						ci_5g->selected_chspec = bgdfs->next_scan_chan;
						if (!(ci_5g->switch_reason_type ==
							APCS_DFS_REENTRY)) {
						chanim_upd_acs_record(ci_5g->chanim_info,
								ci_5g->selected_chspec, APCS_ZDFS);
						ci_5g->recent_prev_chspec = ci_5g->cur_chspec;
						ci_5g->acs_prev_chan_at = uptime();
						acs_update_status(ci_5g);
						}
					}
					c_info->acs_bgdfs->acs_bgdfs_on_txfail = FALSE;
					ci_5g->acs_bgdfs->next_scan_chan = 0;
					c_info->ci_5g = 0;
					ci_5g->ci_2g = 0;
				} else {
					ACSD_INFO("%s: Not able to clear the channel 0x%x\n",
						c_info->name, bgdfs->next_scan_chan);
				}
			}
		}
#endif /* ZDFS_2G */

		if (BAND_5G(c_info->rs_info.band_type)) {
		/* BGDFS is not enabled/triggered if 160Mhz BW capable */
			if (ACS_11H_AND_BGDFS(c_info) &&
					(!c_info->is160_bwcap || c_info->bgdfs160) &&
					bgdfs->state != BGDFS_STATE_IDLE) {
				time_t now = uptime();
				bool bgdfs_scan_done = FALSE;
				if ((ticks % ACS_BGDFS_SCAN_STATUS_CHECK_INTERVAL) == 0) {
					if ((ret = acs_bgdfs_get(c_info)) != BGDFS_CAP_TYPE0) {
						ACSD_ERROR("acs bgdfs get failed with %d\n", ret);
					}
					if (bgdfs->status.move_status != DFS_SCAN_S_INPROGESS) {
						bgdfs_scan_done = TRUE;
					}
				}
				if (bgdfs_scan_done || bgdfs->timeout < now) {
					bgdfs->state = BGDFS_STATE_IDLE;
					c_info->cac_mode = ACS_CAC_MODE_AUTO;
					if (!bgdfs_scan_done &&
						(ret = acs_bgdfs_get(c_info)) != BGDFS_CAP_TYPE0) {
						ACSD_ERROR("acs bgdfs get failed with %d\n", ret);
					}
					if (bgdfs->fallback_blocking_cac &&
						bgdfs->acs_bgdfs_on_txfail &&
						((ret = acs_bgdfs_check_status(c_info, TRUE))
						 != BCME_OK)) {
					if (c_info->txop_score >
							c_info->acs_txop_limit_hi) {
						ACSD_INFO("ifname: %s Not initiating "
								"fallback due to txop "
								"thresh being high %d "
								"when compared to thresh: %d\n",
								c_info->name, c_info->txop_score,
								c_info->acs_txop_limit_hi);
						continue;
					}
					if (!acs_channel_compare_before_fallback(c_info,
							c_info->cur_chspec,
							bgdfs->next_scan_chan))
					{
						ACSD_INFO("ifname: %s Not initiating"
								" fallback due to thresh"
								"being low\n",
								c_info->name);
						continue;
					}
					ACSD_INFO("ifname:%s BGDFS Failed. Do Full MIMO"
							"CAC\n", c_info->name);
					ACSD_PRINT("%s: DCS request for failed BGDFS. Do full MIMO CAC\n", c_info->name);
					acs_csa_handle_request(c_info);
					bgdfs->acs_bgdfs_on_txfail = FALSE;
					bgdfs->next_scan_chan = 0;
					} else if (bgdfs->next_scan_chan != 0) {
						if ((ret = acs_bgdfs_check_status(c_info, FALSE))
							== BCME_OK) {
							ACSD_INFO("ifname:%s acs bgdfs ch 0x%x is"
								"radar free\n", c_info->name,
								bgdfs->next_scan_chan);
							/* Updating the selected chanspec with bgdfs
							 * scan core chanspec
							 */
							if (!c_info->country_is_edcrs_eu) {
								bgdfs->next_scan_chan =
								c_info->selected_chspec;
								chanim_upd_acs_record(
								c_info->chanim_info,
								bgdfs->next_scan_chan, APCS_ZDFS);
								c_info->recent_prev_chspec =
								c_info->cur_chspec;
								c_info->acs_prev_chan_at =
									uptime();
							}
						} else if (bgdfs->next_scan_chan != 0) {
							ACSD_INFO("ifname:%s acs bgdfs chan 0x%x"
								"is not radar free (err: %d)\n",
								c_info->name,
								bgdfs->next_scan_chan, ret);
						}
						/* reset for next try; let it recompute channel */
						bgdfs->next_scan_chan = 0;
					}
				} else {
					continue;
				}
			}
		}

		if (BAND_5G(c_info->rs_info.band_type)) { /* Update channel idle times */

			if (ACS_11H_AND_BGDFS(c_info) && (ticks %
				ACS_TRAFFIC_INFO_UPDATE_INTERVAL((c_info)->acs_bgdfs)) == 0) {

				if ((ret = acs_activity_update(c_info)) != BCME_OK) {
					ACSD_ERROR("activity update failed");
				}

				(void) acs_bgdfs_idle_check(c_info);

				/* BGDFS is not enabled/triggered if 160Mhz BW capable */
				if (ACS_11H_AND_BGDFS(c_info) &&
						(!c_info->is160_bwcap || c_info->bgdfs160) &&
						bgdfs->idle &&
						bgdfs->state == BGDFS_STATE_IDLE) {
					if (bgdfs->ahead &&
							acs_bgdfs_ahead_trigger_scan(c_info) !=
							BCME_OK) {
						ACSD_ERROR("BGDFS ahead trigger scan "
								"failed\n");
					}
				}
				/* Upgrade to 160Mhz by full MIMO CAC */
				if (ACS_11H_AND_BGDFS(c_info) && c_info->is160_upgradable &&
						!c_info->dyn160_enabled &&
						bgdfs->idle) {
					if (acs_upgrade_to160(c_info) == BCME_OK) {
						ACSD_INFO("%s acs_upgrade_to160 picked 0x%4x (%s)\n",
								c_info->name,
								c_info->selected_chspec,
								wf_chspec_ntoa(c_info->selected_chspec, chanspecbuf));
						ACSD_PRINT("%s: DCS request for successful upgrading to 160Mhz %s by full MIMO CAC\n", c_info->name, wf_chspec_ntoa(c_info->selected_chspec, chanspecbuf));
						acs_csa_handle_request(c_info);
					} else {
						ACSD_ERROR("%s acs_upgrade_to160 Failed\n",
								c_info->name);
					}
				}
			}

			if (ticks % ACS_ASSOCLIST_POLL == 0)  {
				acs_upgrade_downgrade_opermode(c_info);
			}
			acs_dfsr_activity_update(ACS_DFSR_CTX(c_info), c_info->name);
		}

		if (ticks % ACS_STATUS_POLL == 0)
			acs_update_status(c_info);

		/* Chanim_stats updates for every 1 min, but we are also updating chanim_stats
		 * if switch reason is CS or CI scan. These stats are helpful for selecting
		 * better channel.
		 */
		if ((ticks % ACS_CHANIM_POLL_MIN) == 0 ||
				c_info->switch_reason_type == APCS_CSTIMER ||
				c_info->switch_reason_type == ACS_SCAN_TYPE_CI) {
			acsd_chanim_check(ticks, c_info);
		}

		/*
		 * After completion of cs/ci scan timeout acsd does scan and try to select the
		 * best channel. While selecting best channel acsd may stay on same channel
		 * if TXOP on current channel is greater than threshold(90 for CS, 80 for CI)
		 * else acsd calculate the score for all valid channels and select the best channel,
		 * during this period c_info->txop_channel_select is set to 2
		 * (inside acs_pick_chanspec_common function). The reason for this setting is wait
		 * for 2 sec and check if txop recovers within the time limit. If it recovers no
		 * channel change is allowed and acsd stays on same channel. If it doesn't recover
		 * update the channel and acs_record in this function.
		 */
		if ((c_info->txop_channel_select && (-- c_info->txop_channel_select == 0)) ||
				(c_info->bw_upgradable && c_info->selected_chspec &&
				c_info->selected_chspec != c_info->cur_chspec)) {
			if (acs_is_dfs_chanspec(c_info, c_info->selected_chspec)) {
				ACSD_DEBUG("%s: Moving to DFS channel 0x%04x is not allowed using"
					       "CSA\n", c_info->name, c_info->selected_chspec);
				continue;
			}
			uint8 scan_type;
			time_t now = uptime();

			if (acs_check_assoc_scb(c_info)) {
				ACSD_INFO("txop channel select: no channel switch while associated\n");
				return;
			}

			if (acs_is_dfs_chanspec(c_info, c_info->selected_chspec)) {
				ACSD_DEBUG("%s: Moving to DFS channel 0x%04x is not allowed using"
					       "CSA\n", c_info->name, c_info->selected_chspec);
				return;
			}

			ACSD_DEBUG("%s: prev timestamp1 %d cur time stamp1 %d score %d\n",
				c_info->name,
				c_info->timestamp, c_info->cur_timestamp, c_info->txop_score);
			scan_type = ((c_info->switch_reason_type == ACS_SCAN_TYPE_CI) ?
				(c_info->txop_score < c_info->ci_scan_txop_limit) :
				(c_info->txop_score < c_info->acs_txop_limit));

			if (CHSPEC_CHANNEL(c_info->selected_chspec) ==
					CHSPEC_CHANNEL(c_info->recent_prev_chspec)) {
				if (now - c_info->acs_prev_chan_at <
						2 * c_info->acs_chan_dwell_time) {
					ACSD_DEBUG("%s: staying on same channel because of "
							"prev_chspec dwell time restrictions\n",
							c_info->name);
					continue;
				}
			}
			ACSD_DEBUG("%s timestamp:%us, cur_timestamp:%us, scan_type:%d, "
					"bw_upgradable:%d, selected_chspec:0x%04x, "
					"cur_chspec:0x%04x, switch_reason_type:%d\n",
					c_info->name, c_info->timestamp,
					c_info->cur_timestamp, scan_type, c_info->bw_upgradable,
					c_info->selected_chspec, c_info->cur_chspec,
					c_info->switch_reason_type);
			if ((c_info->timestamp != c_info->cur_timestamp) &&
					(scan_type || c_info->bw_upgradable)) {
				if (c_info->cur_chspec != c_info->selected_chspec) {
					int ret = 0;
					ACSD_PRINT("%s: selected_chspec is %4x (%s)\n",
						c_info->name, c_info->selected_chspec, wf_chspec_ntoa(c_info->selected_chspec, chanspecbuf));
					c_info->selected_chspec = acs_adjust_ctrl_chan(c_info,
						c_info->selected_chspec);
					ACSD_PRINT("%s: Adjusted channel spec: 0x%4x (%s)\n",
						c_info->name, c_info->selected_chspec, wf_chspec_ntoa(c_info->selected_chspec, chanspecbuf));
					ACSD_PRINT("%s: selected channel spec: 0x%4x (%s)\n",
						c_info->name, c_info->selected_chspec, wf_chspec_ntoa(c_info->selected_chspec, chanspecbuf));
					if (c_info->fallback_to_primary &&
							CHSPEC_CHANNEL(c_info->selected_chspec) ==
							CHSPEC_CHANNEL(c_info->cur_chspec)) {
						ACSD_INFO("%s selected 0x%04x & cur_chspec 0x%04x "
							"are using the same control channel\n",
							c_info->name, c_info->selected_chspec,
							c_info->cur_chspec);
						c_info->switch_reason_type = 0;
						continue;
					}
					ACSD_PRINT("%s: txop channel select: Performing CSA on chspec 0x%04x\n",
							c_info->name, c_info->selected_chspec);
					if ((ret = acs_csa_handle_request(c_info))) {
						continue;
					}
					if (c_info->switch_reason_type == APCS_CSTIMER) {
						chanim_upd_acs_record(c_info->chanim_info,
								c_info->selected_chspec,
								APCS_CSTIMER);
					} else {
						chanim_upd_acs_record(c_info->chanim_info,
							c_info->selected_chspec, APCS_TXDLY);
					}
					c_info->recent_prev_chspec = c_info->cur_chspec;
					c_info->acs_prev_chan_at = uptime();
					acs_update_status(c_info);
				}
			} else {
				c_info->selected_chspec = c_info->cur_chspec;
				ACSD_PRINT("%s: staying in current channel as txop is recovered "
					"with in time limit\n", c_info->name);
			}
			c_info->switch_reason_type = 0;
		}

		/* AUTOCHANNEL/DFSR is checked in called fn */
		acs_scan_timer_or_dfsr_check(c_info, ticks);

		if (AUTOCHANNEL(c_info) &&
			(acs_ci_scan_check(c_info, ticks) || CI_SCAN(c_info))) {
			acs_do_ci_update(ticks, c_info);
		}
	}
}

/* service main entry */
int
main(int argc, char *argv[])
{
	int err = ACSD_OK;
	struct timeval tv;
	char *val;
#if !defined(DEBUG)
	int daemonize = 1;
#endif // endif

	val = nvram_safe_get("acsd_disable");
	if (!strcmp(val, "1")) {
		ACSD_ERROR("Not Staring ACSD as acsd_disable nvram is set\n");
		return err;
	}

	val = nvram_safe_get("acsd_debug_level");
	if (strcmp(val, ""))
		acsd_debug_level = strtoul(val, NULL, 0);

	argv[0] = "acsd2";
	ACSD_INFO("acsd start...\n");

	if (argc > 1) {
	    if (strcmp(argv[1], "-F") == 0) {
#if !defined(DEBUG)
		daemonize = 0;
#endif // endif
	    } else {
		ACSD_ERROR("Unknown argument\n");
		goto cleanup;
	    }
	}

	nvram_unset("obd_allow_scan");

	val = nvram_safe_get("acs_ifnames");
	if (!strcmp(val, "")) {
		ACSD_ERROR("No interface specified, exiting...");
		return err;
	}

	if ((err = acsd_init()))
		goto cleanup;

	acs_init_run(&d_info->acs_info);

#if !defined(DEBUG)
	if (daemonize) {
		if (daemon(1, 1) == -1) {
			ACSD_ERROR("err from daemonize.\n");
			goto cleanup;
		}
	}
#endif // endif
	tv.tv_sec = d_info->poll_interval;
	tv.tv_usec = 0;

	/* Provide necessary info to debug_monitor for service restart */
	dm_register_app_restart_info(getpid(), 1, argv, NULL);

	/* establish a handler to handle SIGTERM. */
	signal(SIGTERM, acsd_term_hdlr);
	acsd_running = TRUE;
	while (acsd_running) {
		/* Don't change channel when WPS is in the processing,
		 * to avoid WPS fails
		 */
		if (ACS_WPS_RUNNING) {
			sleep_ms(1000);
			continue;
		}

		if (tv.tv_sec == 0 && tv.tv_usec == 0) {
			d_info->ticks ++;
			tv.tv_sec = d_info->poll_interval;
			tv.tv_usec = 0;
			ACSD_DEBUG("ticks: %d\n", d_info->ticks);
			acsd_watchdog(d_info->ticks);
		}
		acsd_main_loop(&tv);
	}
cleanup:
	acsd_close_eventfd();
	acsd_close_listenfd();
	acsd_cleanup();
	return err;
}
