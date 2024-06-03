/*
 * hostapd / DPP integration
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See BSD_LICENSE for more details.
 */

/* This is a merged file for hostapd and wpa_supplicant dpp state mahine code
 * wapp and wdev are introduced and state machines are changed for handling
 * for AP, STA, enrollee, configurator role.
 */
#include "os.h"
#include "util.h"
#include "eloop.h"
#include "wapp_cmm.h"
#include "dpp.h"
#include "gas.h"
#include "dpp_wdev.h"
#include "gas_server.h"
#include "utils/ip_addr.h"
#include "gas_query.h"
#include "utils/wpabuf.h"
#include "utils/base64.h"
#include "utils/wpa_debug.h"
#include "map.h"
#ifdef OPENWRT_SUPPORT
#include <libdatconf.h>
#endif



#define DEFAULT_5GH_IFACE      "wlan0"
#define DEFAULT_5GL_IFACE      "rai0"
#define DEFAULT_2G_IFACE      "ra0"
#define MAX_CONN_TRIES           3
#ifdef OPENWRT_SUPPORT
static int dpp_save_config(struct wifi_app *wapp, const char *param, const char *value, char *ifname);
static void dpp_fetch_dpp_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth, struct kvc_context *dat_ctx);
static void dpp_fetch_psk_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth, struct kvc_context *dat_ctx);
#endif
static void wapp_dpp_reply_wait_timeout(void *eloop_ctx, void *timeout_ctx);
static void wapp_dpp_auth_success(struct wifi_app *wapp, int initiator, struct wapp_dev *wdev);
static void wapp_dpp_init_timeout(void *eloop_ctx, void *timeout_ctx);
static int wapp_dpp_auth_init_next(struct wifi_app *wapp, struct dpp_authentication *auth);

static const u8 broadcast[MAC_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static int wapp_dpp_process_config(struct wifi_app *wapp,
				    struct dpp_authentication *auth);

static void wapp_dpp_cancel_timeouts(struct wifi_app *wapp, struct dpp_authentication *auth);
void
wapp_dpp_tx_pkex_status(struct wifi_app *wapp,
			unsigned int chan, const u8 *dst,
			const u8 *src, const u8 *bssid,
			const u8 *data, size_t data_len,
			int ok);
static void wapp_dpp_auth_resp_retry_timeout(void *eloop_ctx, void *timeout_ctx);
static void wapp_dpp_auth_conf_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx);
static void wapp_dpp_auth_resp_retry(struct wifi_app *wapp, struct dpp_authentication *auth);


/* Use a hardcoded Transaction ID 1 in Peer Discovery frames since there is only
 * a single transaction in progress at any point in time. */
static const u8 TRANSACTION_ID = 1;

void dpp_auth_list_deinit(struct wifi_app *wapp);

/* dpp auth helper functions */
struct dpp_authentication *wapp_dpp_get_first_auth(struct wifi_app *wapp)
{
	struct dpp_authentication *auth = dl_list_first(&wapp->dpp->dpp_auth_list, struct dpp_authentication,
							list);

	return auth;
}

struct dpp_authentication *wapp_dpp_get_last_auth(struct wifi_app *wapp)
{
	struct dpp_authentication *auth, *auth_tmp, *auth_tmp2 = NULL;

	dl_list_for_each_safe(auth, auth_tmp, &(wapp->dpp->dpp_auth_list),
			struct dpp_authentication, list) {
		auth_tmp2 = auth;
	}

	if (!auth_tmp2) {
		return NULL;
	}
	auth = auth_tmp2;
	return auth;
}

struct dpp_authentication *wapp_dpp_get_auth_from_peer_mac(struct wifi_app *wapp, u8 *addr)
{
	struct dpp_authentication *auth;

	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (os_memcmp(auth->peer_mac_addr, addr, ETH_ALEN) == 0)
			return auth;
	}

	return NULL;
}

struct dpp_authentication *wapp_get_next_auth(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	struct dpp_authentication *auth1 = (struct dpp_authentication *)((&(auth->list))->next);
	return auth1;
	//return dl_list_entry(&(auth->list)->next, struct dpp_authentication, list);
}

int wapp_dpp_auth_list_insert(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	struct dpp_authentication *auth_iter;
	dl_list_for_each(auth_iter, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (os_memcmp(auth->peer_mac_addr, auth_iter->peer_mac_addr, ETH_ALEN) == 0) {
			DBGPRINT(RT_DEBUG_ERROR, "another auth exchange from same mac address\n");
			dl_list_add_tail(&wapp->dpp->dpp_auth_list, &auth->list);
			return -1;
		}
	}
	dl_list_add_tail(&wapp->dpp->dpp_auth_list, &auth->list);
	auth->wdev->radio->ongoing_dpp_cnt++;
	return 0;
}

void wapp_dpp_auth_list_remove(struct dpp_authentication *auth)
{
	dl_list_del(&auth->list);
	auth->wdev->radio->ongoing_dpp_cnt--;
}

void wapp_dpp_auth_list_remove_by_mac(struct wifi_app *wapp, char *addr)
{
	struct dpp_authentication *auth;

	dl_list_for_each(auth, &wapp->dpp->dpp_auth_list, struct dpp_authentication, list) {
		if (os_memcmp(auth->peer_mac_addr, addr, ETH_ALEN) == 0)
			return dl_list_del(&auth->list);
	}
}

struct dpp_tx_status *wapp_dpp_get_status_info_from_sq(struct wifi_app *wapp, u16 seq_no)
{
	struct dpp_tx_status *tx_status;

	dl_list_for_each(tx_status, &wapp->dpp->dpp_txstatus_pending_list, struct dpp_tx_status, list) {
		if (seq_no == tx_status->seq_no)
			return tx_status;
	}

	return NULL;
}

void wapp_dpp_txstatus_list_insert(struct wifi_app *wapp, struct dpp_tx_status *tx_status)
{
	dl_list_add_tail(&wapp->dpp->dpp_txstatus_pending_list, &tx_status->list);
}

void wapp_dpp_tx_status_list_remove(struct dpp_tx_status *tx_status)
{
	dl_list_del(&tx_status->list);
}

int wapp_get_band_from_chan(unsigned int chan)
{
	// TODO kapil check for triband
	if (chan <= 14)
		return BAND_24G;
	else
		return BAND_5G;
}

int wapp_set_pmk(struct wifi_app *wapp, const u8 *addr,
                        const u8 *pmk, size_t pmk_len, const u8 *pmkid,
                        int session_timeout, int akmp, struct wapp_dev *wdev)
{
	if (!wdev) {
		return -1;
	}
        if (!wapp->drv_ops || !wapp->drv_ops->drv_set_pmk)
                return 0;
        return wapp->drv_ops->drv_set_pmk(wapp->drv_data, wdev->ifname, pmk, pmk_len, pmkid,
                                 wdev->mac_addr, addr, session_timeout, akmp);
}

int wapp_insert_dpp_tx_status_list(struct wifi_app *wapp, struct wapp_dev *wdev,
					const u8 *dst, const u8 *data)
{
	struct dpp_tx_status *status;
	/* go to type */
	data++;

	/* Insert into auth status list if da is not broadcast */
	if (os_memcmp(dst, broadcast, MAC_ADDR_LEN) == 0) {
		return 0;
	}
	wapp->dpp->dpp_frame_seq_no++;
	if(wapp->dpp->dpp_frame_seq_no > 0xfff)
		wapp->dpp->dpp_frame_seq_no = 1011; //Reset Sequence Number

	status = os_zalloc(sizeof(*status));

	os_memcpy(status->dst, dst, MAC_ADDR_LEN);
	status->seq_no = wapp->dpp->dpp_frame_seq_no;
	status->wdev = wdev;
	os_get_time(&status->sent_time);

	if (*data == 0xa || *data == 0xb)
		status->is_gas_frame = 1;

	/* TODO add a timer to delete it if we don't get the result, or use periodic */
	wapp_dpp_txstatus_list_insert(wapp, status);

	return 0;
}

int wapp_drv_send_action(struct wifi_app *wapp, struct wapp_dev *wdev, unsigned int chan,
		unsigned int wait, const u8 *dst, const u8 *data,
		size_t len)
{
	const u8 *bssid;

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "wdev is null\n");
		return -1;
	}

	if (chan == 0) {
		chan = wdev->radio->op_ch;
	}
	if (!wapp->drv_ops || !wapp->drv_ops->send_action)
		return 0;

	/* All the DPP frames are sent in non connected state
	 * Set BSSID as always wildcard */
	bssid = broadcast;

	/* Add this for tx status list */
	wapp_insert_dpp_tx_status_list(wapp, wdev, dst, data);

	wpa_printf(MSG_DEBUG,
			"DPP: Sending out dpp frame to "MACSTR " seq_no =%u, chan=%u\n",
			MAC2STR(dst), wapp->dpp->dpp_frame_seq_no, chan);
	return wapp->drv_ops->send_action(wapp->drv_data, wdev->ifname, chan, wait, dst,
			wdev->mac_addr, bssid, data, len, wapp->dpp->dpp_frame_seq_no, 0);
}

int wapp_cancel_remain_on_channel(
        struct wifi_app *wapp, struct wapp_dev *wdev)
{

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "no send action waiting\n");
		return -1;
	}

	if (wapp->drv_ops->drv_cancel_roc)
	        return wapp->drv_ops->drv_cancel_roc(wapp->drv_data, wdev->ifname);

	return -1;
}

/**
 * wapp_dpp_qr_code - Parse and add DPP bootstrapping info from a QR Code
 * @wapp: Pointer to wifi_app
 * @cmd: DPP URI read from a QR Code
 * Returns: Identifier of the stored info or -1 on failure
 */
int wapp_dpp_qr_code(struct wifi_app *wapp, const char *cmd)
{
	struct dpp_bootstrap_info *bi;
	struct dpp_authentication *auth;

	bi = dpp_add_qr_code(wapp->dpp, cmd);
	if (!bi)
		return -1;

	auth = wapp_dpp_get_first_auth(wapp);

	while (auth) {
		if (auth && auth->response_pending &&
				dpp_notify_new_qr_code(auth, bi) == 1) {
			wpa_printf(MSG_DEBUG,
					"DPP: Sending out pending authentication response");
			wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
				" chan=%u type=%d", MAC2STR(auth->peer_mac_addr), auth->curr_chan,
				DPP_PA_AUTHENTICATION_RESP);
			wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0,
 					auth->peer_mac_addr,
					wpabuf_head(auth->resp_msg),
					wpabuf_len(auth->resp_msg));
			auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
		}
		auth = wapp_get_next_auth(wapp, auth); 

		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth); 
	}

	return bi->id;
}

void wapp_dpp_tx_status(struct wifi_app *wapp, const u8 *dst,
			   const u8 *data, size_t data_len, int ok)
{
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)dst);
	struct wapp_dev *wdev = auth->wdev;

	wpa_printf(MSG_DEBUG, "DPP: TX status: dst=" MACSTR " ok=%d",
		   MAC2STR(dst), ok);

	if (!auth) {
		wpa_printf(MSG_ERROR,
			   "DPP: Ignore TX status since there is no ongoing authentication exchange");
		return;
	}

#ifdef CONFIG_DPP2
	if (auth->connect_on_tx_status) {
		wpa_printf(MSG_ERROR,
			   "DPP: Complete exchange on configuration result");
		dpp_auth_deinit(auth);
		return;
	}
#endif /* CONFIG_DPP2 */

	if (!ok)
	{
		eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,wapp, auth);
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	}

	if (auth->remove_on_tx_status) {
		wpa_printf(MSG_ERROR,
			   "DPP: Terminate authentication exchange due to an earlier error");
		eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
 		eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,
				     wapp, auth);
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		dpp_auth_deinit(auth);
		return;
	}

	if (auth->dpp_auth_success_on_ack_rx && ok)
		wapp_dpp_auth_success(wapp, 1, wdev);

	if (!is_broadcast_ether_addr(dst) && !ok) {
		wpa_printf(MSG_ERROR,
			   "DPP: Unicast DPP Action frame was not ACKed");
		if (auth->waiting_auth_resp) {
			/* In case of DPP Authentication Request frame, move to
			 * the next channel immediately. */
			wapp_cancel_remain_on_channel(wapp, auth->wdev);
			wapp_dpp_auth_init_next(wapp, auth);
			return;
		}
		if (auth->waiting_auth_conf) {
			wapp_dpp_auth_resp_retry(wapp, auth);
			return;
		}
	}

	if (!is_broadcast_ether_addr(dst) && auth->waiting_auth_resp && ok) {
		/* Allow timeout handling to stop iteration if no response is
		 * received from a peer that has ACKed a request. */
		auth->auth_req_ack = 1;
	}

	if (auth->dpp_auth_success_on_ack_rx)
		auth->dpp_auth_success_on_ack_rx = 0;
}

static void wapp_dpp_cancel_timeouts(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,wapp, auth);
	eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
	eloop_cancel_timeout(wapp_dpp_auth_resp_retry_timeout, wapp, auth);
#ifdef CONFIG_DPP2
	eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout, wapp, auth);
#endif /* CONFIG_DPP2 */
}

static void wapp_dpp_reply_wait_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;
	unsigned int chan;
	struct os_time now, diff;  
	unsigned int wait_time, diff_ms;

	if (!auth || !auth->waiting_auth_resp)
		return;

	wait_time = wapp->dpp->dpp_resp_wait_time ?
		wapp->dpp->dpp_resp_wait_time : 2000;
	os_get_time(&now);
	os_time_sub(&now, &wapp->dpp->dpp_last_init, &diff);
	diff_ms = diff.sec * 1000 + diff.usec / 1000;
	wpa_printf(MSG_DEBUG,
		   "DPP: Reply wait timeout - wait_time=%u diff_ms=%u",
		   wait_time, diff_ms);

	if (auth->auth_req_ack && diff_ms >= wait_time) {
		/* Peer ACK'ed Authentication Request frame, but did not reply
		 * with Authentication Response frame within two seconds. */
		wpa_printf(MSG_ERROR,
			   "DPP: No response received from responder - stopping initiation attempt");
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		wapp_dpp_listen_stop(wapp, auth->wdev);
		/* Cacelling all timouts if ACK for unicast auth request received */
                wapp_dpp_cancel_timeouts(wapp,auth);
		dpp_auth_deinit(auth);
		return;
	}

	if (diff_ms >= wait_time) {
		/* Authentication Request frame was not ACK'ed and no reply
		 * was receiving within two seconds. */
		wpa_printf(MSG_INFO1,
			   "DPP: Continue Initiator channel iteration");
		wapp_cancel_remain_on_channel(wapp, auth->wdev);
		wapp_dpp_listen_stop(wapp, auth->wdev);
		wapp_dpp_auth_init_next(wapp, auth);
		return;
	}

	/* wait rest of the time on home channel
	 * and reuse this API to send on next channel */
	wait_time -= diff_ms;

	chan = auth->curr_chan;
	if (auth->neg_chan > 0) {
		chan = auth->neg_chan;
		wapp_cancel_remain_on_channel(wapp, auth->wdev);

		wpa_printf(MSG_DEBUG,
				"DPP: Continue reply wait on channel %u for %u ms",
				chan, wait_time);

		eloop_register_timeout(wait_time / 1000, (wait_time % 1000) * 1000,
				wapp_dpp_reply_wait_timeout, wapp, auth);
	}
}


static void wapp_dpp_init_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if (!auth)
		return;
	wpa_printf(MSG_DEBUG, "DPP: Retry initiation after timeout");
	wapp_dpp_auth_init_next(wapp, auth);
}


static int wapp_dpp_auth_init_next(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	const u8 *dst;
	unsigned int wait_time, max_wait_time, chan, max_tries, used;
	struct os_time now, diff; 

	if (!auth)
		return -1;

	if (auth->chan_idx == 0)
		os_get_time(&wapp->dpp->dpp_init_iter_start);

	if (auth->chan_idx >= auth->num_chan) {
		auth->num_chan_iters++;
		if (wapp->dpp->dpp_init_max_tries)
			max_tries = wapp->dpp->dpp_init_max_tries;
		else
			max_tries = 5; 
		if (auth->num_chan_iters >= max_tries || auth->auth_req_ack) {
			wpa_printf(MSG_INFO1,
				   "DPP: No response received from responder - stopping initiation attempt");
			eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,
					     wapp, auth);
			wapp_cancel_remain_on_channel(wapp, auth->wdev);
			dpp_auth_deinit(auth);
			return -1;
		}
		auth->chan_idx = 0;
		eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
		if (wapp->dpp->dpp_init_retry_time)
			wait_time = wapp->dpp->dpp_init_retry_time;
		else
			wait_time = 10000;
		os_get_time(&now);
		os_time_sub(&now, &wapp->dpp->dpp_init_iter_start, &diff);
		used = diff.sec * 1000 + diff.usec / 1000;
		if (used > wait_time)
			wait_time = 0;
		else
			wait_time -= used;
		wpa_printf(MSG_DEBUG, "DPP: Next init attempt in %u ms",
			   wait_time);
		eloop_register_timeout(wait_time / 1000,
				       (wait_time % 1000) * 1000,
				       wapp_dpp_init_timeout, wapp,
				       auth);
		return 0;
	}
	chan = auth->chan[auth->chan_idx++];
	auth->curr_chan = chan;

	if (is_zero_ether_addr(auth->peer_bi->mac_addr))
		dst = broadcast;
	else
		dst = auth->peer_bi->mac_addr;
	auth->dpp_auth_success_on_ack_rx = 0; 
	eloop_cancel_timeout(wapp_dpp_reply_wait_timeout, wapp, auth);
	wait_time = wapp->dpp->max_remain_on_chan;
	max_wait_time = wapp->dpp->dpp_resp_wait_time ?
		wapp->dpp->dpp_resp_wait_time : 2000;
	if (wait_time > max_wait_time)
		wait_time = max_wait_time;
	wait_time += 10; /* give the driver some extra time to complete */
	if (auth->neg_chan > 0 && chan != auth->neg_chan) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Initiate on %u and move to neg_chan %u for response",
			   chan, auth->neg_chan);
		wait_time = 100;
	}
	eloop_register_timeout(wait_time / 1000, (wait_time % 1000) * 1000,
			       wapp_dpp_reply_wait_timeout, wapp, auth);
	wait_time -= 10;
	auth->auth_req_ack = 0;
	os_get_time(&wapp->dpp->dpp_last_init);
	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(dst), auth->curr_chan,
		DPP_PA_AUTHENTICATION_REQ);
	auth->current_state = DPP_STATE_AUTH_RESP_WAITING;
	return wapp_drv_send_action(wapp, auth->wdev, chan, wait_time,
 				       dst,
				       wpabuf_head(auth->req_msg),
				       wpabuf_len(auth->req_msg));
}

int wapp_dpp_auth_init(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd)
{
	const char *pos;
	struct dpp_bootstrap_info *peer_bi, *own_bi = NULL;
	u8 allowed_roles = wapp->dpp->dpp_allowed_roles;
	unsigned int neg_chan = 0;
#ifdef CONFIG_DPP2
        int tcp = 0, map = 0;
	int tcp_port = DPP_TCP_PORT;
	struct wapp_ip_addr ipaddr;
	char *addr, *token1;
	u8 agnt_alid[MAC_ADDR_LEN], i = 0;
#endif /* CONFIG_DPP2 */
	struct dpp_authentication *auth;

	pos = os_strstr(cmd, " peer=");
	if (!pos)
		return -1;
	pos += 6;
	peer_bi = dpp_bootstrap_get_id(wapp->dpp, atoi(pos));
	if (!peer_bi) {
		wpa_printf(MSG_INFO1,
			   "DPP: Could not find bootstrapping info for the identified peer");
		return -1;
	}

	pos = os_strstr(cmd, " own=");
	if (pos) {
		pos += 5;
		own_bi = dpp_bootstrap_get_id(wapp->dpp,
					      atoi(pos));
		if (!own_bi) {
			wpa_printf(MSG_INFO1,
				   "DPP: Could not find bootstrapping info for the identified local entry");
			return -1;
		}

		if (peer_bi->curve != own_bi->curve) {
			wpa_printf(MSG_ERROR,
				   "DPP: Mismatching curves in bootstrapping info (peer=%s own=%s)",
				   peer_bi->curve->name, own_bi->curve->name);
			return -1;
		}
	}

#ifdef CONFIG_DPP2
	pos = os_strstr(cmd, " tcp_port=");
	if (pos) {
		pos += 10;
		tcp_port = atoi(pos);
	}

	addr = get_param(cmd, " tcp_addr=");
	if (addr) {
		int res;

		res = wapp_parse_ip_addr(addr, &ipaddr);
		os_free(addr);
		if (res)
			return -1;
		tcp = 1;
	}
	addr = get_param(cmd, " almac=");
	if (addr) {
		token1 = strtok(addr, ":");

		while (token1 != NULL) {
			AtoH(token1, (char *) &agnt_alid[i], 1);
			i++;
			if (i >= MAC_ADDR_LEN)
				break;
			token1 = strtok(NULL, ":");
		}
		os_free(addr);
		map = 1;
	}
#endif /* CONFIG_DPP2 */

	pos = os_strstr(cmd, " role=");
	if (pos) {
		pos += 6;
		if (os_strncmp(pos, "configurator", 12) == 0)
			allowed_roles = DPP_CAPAB_CONFIGURATOR;
		else if (os_strncmp(pos, "enrollee", 8) == 0)
			allowed_roles = DPP_CAPAB_ENROLLEE;
		else if (os_strncmp(pos, "either", 6) == 0)
			allowed_roles = DPP_CAPAB_CONFIGURATOR |
				DPP_CAPAB_ENROLLEE;
		else
			goto fail;
	}

	pos = os_strstr(cmd, " neg_chan=");
	if (pos)
		neg_chan = atoi(pos + 10);
	/* Multiple auth are allowed in case of configurator but in same channel
	 */
	if (allowed_roles != DPP_CAPAB_CONFIGURATOR) {
		auth = wapp_dpp_get_first_auth(wapp);
		if (auth) {
			eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
			eloop_cancel_timeout(wapp_dpp_reply_wait_timeout,
				     wapp, auth);
			wapp_cancel_remain_on_channel(wapp, auth->wdev);
			dpp_auth_deinit(auth);
		}
	} else {
		if (wdev->radio->ongoing_dpp_cnt) {
			wpa_printf(MSG_ERROR,
				   "already %d dpp session ongoing on radio, return", wdev->radio->ongoing_dpp_cnt);
			goto fail;
		}
	} 

	auth = dpp_auth_init(wapp, wdev, peer_bi, own_bi,
				       allowed_roles, neg_chan);
	if (!auth)
		goto fail;
	auth->wdev = wdev;
	
	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
		goto fail;
	}
	if (dpp_set_configurator(wapp->dpp, wapp,
				 auth, cmd) < 0) {
		dpp_auth_deinit(auth);
		goto fail;
	}

	auth->neg_chan = neg_chan;

	if (!is_zero_ether_addr(peer_bi->mac_addr))
		os_memcpy(auth->peer_mac_addr, peer_bi->mac_addr,
			  MAC_ADDR_LEN);

#ifdef CONFIG_DPP2
        if (tcp)
                return dpp_tcp_init(wapp->dpp, auth, &ipaddr, tcp_port);
	else if (map)
                return dpp_map_init(wapp->dpp, auth, agnt_alid);
#endif /* CONFIG_DPP2 */
	return wapp_dpp_auth_init_next(wapp, auth);
fail:
	return -1;
}

void wapp_dpp_auth_conf_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	wpa_printf(MSG_INFO1,
		   "DPP: Timeout while waiting for Authentication Confirm");
	wapp_cancel_remain_on_channel(wapp, auth->wdev);
	wapp_dpp_listen_stop(wapp, auth->wdev);
	dpp_auth_deinit(auth);
	return;
}

static void wapp_dpp_rx_auth_req(struct wifi_app *wapp,  struct wapp_dev *wdev,
				 const u8 *src, const u8 *hdr, const u8 *buf,
				 size_t len, unsigned int chan)
{
	const u8 *r_bootstrap, *i_bootstrap;
	u16 r_bootstrap_len, i_bootstrap_len;
	struct dpp_bootstrap_info *own_bi = NULL, *peer_bi = NULL;
	struct dpp_authentication *auth = NULL;

	if (!wapp->dpp){
		wpa_printf(MSG_ERROR,
			"Invalid DPP instance");
		return;
	}

	wpa_printf(MSG_INFO1, "DPP: Authentication Request from " MACSTR,
		   MAC2STR(src));

	r_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR,
			"Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return;
	}
	wpa_hexdump(MSG_MSGDUMP, "DPP: Responder Bootstrapping Key Hash",
		    r_bootstrap, r_bootstrap_len);

	i_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_I_BOOTSTRAP_KEY_HASH,
				   &i_bootstrap_len);
	if (!i_bootstrap || i_bootstrap_len != SHA256_MAC_LEN) {
		wpa_printf(MSG_ERROR,
			"Missing or invalid required Initiator Bootstrapping Key Hash attribute");
		return;
	}
	wpa_hexdump(MSG_MSGDUMP, "DPP: Initiator Bootstrapping Key Hash",
		    i_bootstrap, i_bootstrap_len);

	/* Try to find own and peer bootstrapping key matches based on the
	 * received hash values */
	dpp_bootstrap_find_pair(wapp->dpp, i_bootstrap,
				r_bootstrap, &own_bi, &peer_bi);
#ifdef CONFIG_DPP2
	if (!own_bi) {
		if (dpp_relay_rx_action(wapp->dpp,
					src, hdr, buf, len, chan, i_bootstrap,
					r_bootstrap) == 0)
			return;
	}
#endif /* CONFIG_DPP2 */
	if (!own_bi) {
		wpa_printf(MSG_INFO1,
			"No matching own bootstrapping key found - ignore message");
		return;
	}

	/* Get first instance, if not NULL and enrollee mode, return */
	if (wapp_dpp_get_first_auth(wapp) && (wapp->dpp->dpp_allowed_roles != DPP_CAPAB_CONFIGURATOR)) {
		wpa_printf(MSG_ERROR,
			"Already in DPP authentication exchange - ignore new one");
		return;
	}
	
	auth = dpp_auth_req_rx(wapp, wapp->dpp->dpp_allowed_roles,
					 wapp->dpp->dpp_qr_mutual,
					 peer_bi, own_bi, chan, hdr, buf, len);
	/* add in list */
	if (!auth) {
		wpa_printf(MSG_DEBUG, "DPP: No response generated");
		return;
	}
	auth->dpp_auth_success_on_ack_rx = 0;
	auth->wdev = wdev;

	if ((auth->curr_chan != chan) && wdev->radio->ongoing_dpp_cnt) {
		/* dpp ongoing on one channel, can't go on another channel */
		wpa_printf(MSG_ERROR, "DPP: dpp ongoing on one channel, reject");
		auth->wdev = wdev;
		dpp_auth_deinit(auth);
		return;
	} else {
		if (auth->curr_chan != chan) {
			/* Since we are initiator, move to responder's channel */
			wapp_cancel_remain_on_channel(wapp, auth->wdev);  
			wdev_set_quick_ch(wapp, auth->wdev, auth->curr_chan);
		}
	}

	os_memcpy(auth->peer_mac_addr, src, ETH_ALEN);
	if(wapp_dpp_auth_list_insert(wapp, auth) < 0) {
		dpp_auth_deinit(auth);
		return;
	}

	if (dpp_set_configurator(wapp->dpp, wapp,
				 auth,
				 " ") < 0) {
		dpp_auth_deinit(auth);
		return;
	}

	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), auth->curr_chan,
		DPP_PA_AUTHENTICATION_RESP);

	auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
	wapp_drv_send_action(wapp, wdev, auth->curr_chan, 0,
				src, wpabuf_head(auth->resp_msg),
				wpabuf_len(auth->resp_msg));
	if(!wapp->dpp->dpp_qr_mutual)
	{
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	}
}

int wapp_dpp_handle_config_obj(struct wifi_app *wapp,
					  struct dpp_authentication *auth)
{
	wpa_printf(MSG_INFO1, DPP_EVENT_CONF_RECEIVED);
	wpa_printf(MSG_INFO1, DPP_EVENT_CONFOBJ_AKM "%s",
		dpp_akm_str(auth->akm));
	if (auth->ssid_len)
		wpa_printf(MSG_INFO1, DPP_EVENT_CONFOBJ_SSID "%s",
			os_ssid_txt(auth->ssid, auth->ssid_len));
	if (auth->connector) {
		/* TODO: Save the Connector and consider using a command
		 * to fetch the value instead of sending an event with
		 * it. The Connector could end up being larger than what
		 * most clients are ready to receive as an event
		 * message. */
		wpa_printf(MSG_INFO1, DPP_EVENT_CONNECTOR "%s",
			auth->connector);
	} else if (auth->passphrase[0]) {
		char hex[64 * 2 + 1];
		os_snprintf_hex(hex, sizeof(hex),
				 (const u8 *) auth->passphrase,
				 os_strlen(auth->passphrase));
		wpa_printf(MSG_INFO1, DPP_EVENT_CONFOBJ_PASS "%s",
			hex);
	} else if (auth->psk_set) {
		char hex[PMK_LEN * 2 + 1];
		os_snprintf_hex(hex, sizeof(hex), auth->psk, PMK_LEN);
		wpa_printf(MSG_INFO1, DPP_EVENT_CONFOBJ_PSK "%s",
			hex);
	}
	if (auth->c_sign_key) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wpabuf_len(auth->c_sign_key) + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
					 wpabuf_head(auth->c_sign_key),
					 wpabuf_len(auth->c_sign_key));
			wpa_printf(MSG_INFO1,
				DPP_EVENT_C_SIGN_KEY "%s", hex);
			os_free(hex);
		}
	}
	if (auth->net_access_key) {
		char *hex;
		size_t hexlen;

		hexlen = 2 * wpabuf_len(auth->net_access_key) + 1;
		hex = os_malloc(hexlen);
		if (hex) {
			os_snprintf_hex(hex, hexlen,
					 wpabuf_head(auth->net_access_key),
					 wpabuf_len(auth->net_access_key));
			if (auth->net_access_key_expiry)
				wpa_printf(MSG_INFO1,
					DPP_EVENT_NET_ACCESS_KEY "%s %lu", hex,
					(unsigned long)
					auth->net_access_key_expiry);
			else
				wpa_printf(MSG_INFO1,
					DPP_EVENT_NET_ACCESS_KEY "%s", hex);
			os_free(hex);
		}
	}
	/* Saving the configuration here */
	dpp_save_config_to_file(wapp, auth);
	return wapp_dpp_process_config(wapp, auth);
}

void wapp_dpp_config_req_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	wpa_printf(MSG_DEBUG,
		   "DPP: Timeout while waiting for Configuration Request");
	wapp_cancel_remain_on_channel(wapp, auth->wdev);
	wapp_dpp_listen_stop(wapp, auth->wdev);
	dpp_auth_deinit(auth);
	return;
}

static void wapp_dpp_rx_auth_resp(struct wifi_app *wapp, const u8 *src,
				     const u8 *hdr, const u8 *buf, size_t len,
				     unsigned int chan)
{
	/* Since we can't be initiator in more than one case
	 * We should be able to handle multiple enrollee in responder mode
	 * only. In initiator mode, its not possible to loop thorugh the channels.
	 */
	/* Food for thought: Can we become initiator in #radio times at the same time
	 * but do we know the channels of each enrollee in that case?
	 */
	struct dpp_authentication *auth = wapp_dpp_get_last_auth(wapp);
	struct wpabuf *msg;

	wpa_printf(MSG_INFO1, "DPP: Authentication Response from " MACSTR,
		   MAC2STR(src));

	if (!auth) {
		wpa_printf(MSG_ERROR,
			   "DPP: No DPP Authentication in progress - drop");
		return;
	}

	if (auth->current_state != DPP_STATE_AUTH_RESP_WAITING) {
		wpa_printf(MSG_ERROR,
			   "DPP: Incorrect auth state=%d", auth->current_state);
		//dpp_auth_deinit(auth);
		return;
	}
	if (!is_zero_ether_addr(auth->peer_mac_addr) &&
	    os_memcmp(src, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
		wpa_printf(MSG_ERROR, "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
		dpp_auth_deinit(auth);
		return;
	}

	wapp_dpp_cancel_timeouts(wapp,auth);

	/* It may be possible that we got the response in different channel
	 * Ideally in that case, timer in the driver may still be running
	 * cancel that timer, new channel info will be stored in auth packet
	 * and channel switch should be taken care by that.
	 */
	wapp_cancel_remain_on_channel(wapp, auth->wdev);
	if (auth->curr_chan != chan && auth->neg_chan == chan) {
		wpa_printf(MSG_INFO1,
			   "DPP: Responder accepted request for different negotiation channel");
		auth->curr_chan = chan;
	}
	if (auth->wdev && auth->wdev->radio->op_ch != chan) {
		/* Since we are initiator, move to responder's channel */
		wdev_set_quick_ch(wapp, auth->wdev, chan);
	}
	msg = dpp_auth_resp_rx(auth, hdr, buf, len);
	if (!msg) {
		if (auth->auth_resp_status == DPP_STATUS_RESPONSE_PENDING) {
			wpa_printf(MSG_INFO1, "DPP: Wait for full response");
			return;
		}
		wpa_printf(MSG_ERROR, "DPP: No confirm generated");
		dpp_auth_deinit(auth);
		return;
	}
	os_memcpy(auth->peer_mac_addr, src, MAC_ADDR_LEN);

	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), auth->curr_chan,
		DPP_PA_AUTHENTICATION_CONF);

	if (wapp->dpp->dpp_configurator_supported)
		auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
	else
		auth->current_state = DPP_STATE_CONFIG_RSP_WAITING;

	wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));

	if (wapp->dpp->dpp_configurator_supported)
	{
		eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_config_req_wait_timeout, wapp, auth);
	}
	wpabuf_free(msg);
	auth->dpp_auth_success_on_ack_rx = 1;
}

static void wapp_dpp_rx_auth_conf(struct wifi_app *wapp, const u8 *src,
				     const u8 *hdr, const u8 *buf, size_t len)
{
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)src);
	struct wapp_dev *wdev = auth->wdev;

	wpa_printf(MSG_INFO1, "DPP: Authentication Confirmation from " MACSTR,
		   MAC2STR(src));
	
	if (!auth || (auth->auth_success)) {
		wpa_printf(MSG_ERROR,
			   "DPP: No DPP Authentication in progress - drop");
		return;
	}

	if (auth->current_state != DPP_STATE_AUTH_CONF_WAITING) {
		wpa_printf(MSG_ERROR,
			   "DPP: Incorrect auth state=%d", auth->current_state);
		return;
	}

	wapp_dpp_cancel_timeouts(wapp,auth);

	if (os_memcmp(src, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
		wpa_printf(MSG_ERROR, "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
		return;
	}

	if (dpp_auth_conf_rx(auth, hdr, buf, len) < 0) {
		wpa_printf(MSG_ERROR, "DPP: Authentication failed");
		return;
	}

	if (wapp->dpp->dpp_configurator_supported)
		auth->current_state = DPP_STATE_CONFIG_REQ_WAITING;
	else
		auth->current_state = DPP_STATE_CONFIG_RSP_WAITING;

	if (wapp->dpp->dpp_configurator_supported)
	{
		eloop_cancel_timeout(wapp_dpp_config_req_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_config_req_wait_timeout, wapp, auth);
	}

	wapp_dpp_auth_success(wapp, 0, wdev);
}

#ifdef CONFIG_DPP2
static void wapp_dpp_config_result_wait_timeout(void *eloop_ctx,
						   void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if (!auth || !auth->waiting_conf_result)
		return;

	wpa_printf(MSG_DEBUG,
		   "DPP: Timeout while waiting for Configuration Result");
	dpp_auth_deinit(auth);
}


static void wapp_dpp_rx_conf_result(struct wifi_app *wapp, const u8 *src,
				       const u8 *hdr, const u8 *buf, size_t len)
{
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, src);
	enum dpp_status_error status;

	wpa_printf(MSG_DEBUG, "DPP: Configuration Result from " MACSTR,
		   MAC2STR(src));

	if (!auth || !auth->waiting_conf_result) {
		wpa_printf(MSG_DEBUG,
			   "DPP: No DPP Configuration waiting for result - drop");
		return;
	}

	if (auth->current_state != DPP_STATE_CONFIG_RESULT_WAITING) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Incorrect auth state=%d", auth->current_state);
		return;
	}
	if (os_memcmp(src, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
		wpa_printf(MSG_DEBUG, "DPP: MAC address mismatch (expected "
			   MACSTR ") - drop", MAC2STR(auth->peer_mac_addr));
		return;
	}

	status = dpp_conf_result_rx(auth, hdr, buf, len);

	wapp_cancel_remain_on_channel(wapp, wdev);
	wapp_dpp_listen_stop(wapp, auth->wdev);
	if (status == DPP_STATUS_OK)
		wpa_printf(MSG_INFO1, DPP_EVENT_CONF_SENT);
	else
		wpa_printf(MSG_INFO1, DPP_EVENT_CONF_FAILED);
	dpp_auth_deinit(auth);
	eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout, wapp,
			     auth);
}

#endif /* CONFIG_DPP2 */

static void wapp_dpp_send_peer_disc_resp(struct wifi_app *wapp,
					 struct wapp_dev *wdev,
					    const u8 *src, unsigned int chan,
					    u8 trans_id,
					    enum dpp_status_error status)
{
	struct wpabuf *msg;

	msg = dpp_alloc_msg(DPP_PA_PEER_DISCOVERY_RESP,
			    5 + 5 + 4 + os_strlen(wdev->config->dpp_connector));
	if (!msg)
		return;

	/* Transaction ID */
	wpabuf_put_le16(msg, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, trans_id);

	/* DPP Status */
	wpabuf_put_le16(msg, DPP_ATTR_STATUS);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, status);

	/* DPP Connector */
	if (status == DPP_STATUS_OK) {
		wpabuf_put_le16(msg, DPP_ATTR_CONNECTOR);
		wpabuf_put_le16(msg, os_strlen(wdev->config->dpp_connector));
		wpabuf_put_str(msg, wdev->config->dpp_connector);
	}

	wpa_printf(MSG_INFO1, "DPP: Send Peer Discovery Response to " MACSTR
		   " status=%d", MAC2STR(src), status);

	wapp_drv_send_action(wapp, wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);
}

static void wapp_dpp_rx_peer_disc_req(struct wifi_app *wapp,
					struct wapp_dev *wdev,
					 const u8 *src,
					 const u8 *buf, size_t len,
					 unsigned int chan)
{
	const u8 *connector, *trans_id;
	u16 connector_len, trans_id_len;
	struct os_time now;
	struct dpp_introduction intro;
	os_time_t expire;
	int expiration;
	enum dpp_status_error res;

	wpa_printf(MSG_INFO1, "DPP: Peer Discovery Request from " MACSTR,
		   MAC2STR(src));

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "wdev is null, return \n");
		return;
	}

	if (!wdev->config || !wdev->config->dpp_connector ||
		!wdev->config->dpp_netaccesskey || !wdev->config->dpp_csign) {
		wpa_printf(MSG_ERROR, "DPP: No own Connector/keys set");
		return;
	}

	os_get_time(&now);

	if (wdev->config->dpp_netaccesskey_expiry &&
	    (os_time_t) wdev->config->dpp_netaccesskey_expiry < now.sec) {
		wpa_printf(MSG_ERROR, "DPP: Own netAccessKey expired");
		return;
	}

	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
			       &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		wpa_printf(MSG_ERROR,
			   "DPP: Peer did not include Transaction ID");
		return;
	}

	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		wpa_printf(MSG_ERROR,
			   "DPP: Peer did not include its Connector");
		return;
	}

	res = dpp_peer_intro(&intro, wdev->config->dpp_connector,
			     wdev->config->dpp_netaccesskey,
			     wdev->config->dpp_netaccesskey_len,
			     wdev->config->dpp_csign,
			     wdev->config->dpp_csign_len,
			     connector, connector_len, &expire);
	if (res == 255) {
		wpa_printf(MSG_ERROR,
			   "DPP: Network Introduction protocol resulted in internal failure (peer "
			   MACSTR ")", MAC2STR(src));
		return;
	}
	if (res != DPP_STATUS_OK) {
		wpa_printf(MSG_ERROR,
			   "DPP: Network Introduction protocol resulted in failure (peer "
			   MACSTR " status %d)", MAC2STR(src), res);
		wapp_dpp_send_peer_disc_resp(wapp, wdev, src, chan, trans_id[0],
						res);
		return;
	}

	if (!expire || (os_time_t) wdev->config->dpp_netaccesskey_expiry < expire)
		expire = wdev->config->dpp_netaccesskey_expiry;
	if (expire)
		expiration = expire - now.sec;
	else
		expiration = 0;

	if (wapp_set_pmk(wapp, src, intro.pmk, intro.pmk_len,
				intro.pmkid, expiration,
				WPA_KEY_MGMT_DPP, wdev) < 0) {
		wpa_printf(MSG_ERROR, "DPP: Failed to add PMKSA cache entry");
		return;
	}

	wapp_dpp_send_peer_disc_resp(wapp, wdev, src, chan, trans_id[0],
					DPP_STATUS_OK);
}

static void wapp_dpp_rx_peer_disc_resp(struct wifi_app *wapp,
					struct wapp_dev *wdev,
				       const u8 *src,
				       const u8 *buf, size_t len)
{
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "wdev is null, return \n");
		return;
	}
	struct dpp_config *config = wdev->config;
	const u8 *connector, *trans_id, *status;
	u16 connector_len, trans_id_len, status_len;
	struct dpp_introduction intro;
	//struct rsn_pmksa_cache_entry *entry;
	struct os_time now;
	struct os_time rnow;
	os_time_t expiry;
	unsigned int seconds;
	enum dpp_status_error res;
	int expiration;

	wpa_printf(MSG_INFO1, "DPP: Peer Discovery Response from " MACSTR,
		   MAC2STR(src));

	if (!config || !config->dpp_connector || !config->dpp_netaccesskey ||
	    !config->dpp_csign) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Profile not found for network introduction");
		return;
	}
	trans_id = dpp_get_attr(buf, len, DPP_ATTR_TRANSACTION_ID,
			       &trans_id_len);
	if (!trans_id || trans_id_len != 1) {
		wpa_printf(MSG_ERROR,
			   "DPP: Peer did not include Transaction ID");
		wpa_printf(MSG_ERROR, DPP_EVENT_INTRO "peer=" MACSTR
			" fail=missing_transaction_id", MAC2STR(src));
		goto fail;
	}
	if (trans_id[0] != TRANSACTION_ID) {
		wpa_printf(MSG_ERROR,
			   "DPP: Ignore frame with unexpected Transaction ID %u",
			   trans_id[0]);
		wpa_printf(MSG_ERROR, DPP_EVENT_INTRO "peer=" MACSTR
			" fail=transaction_id_mismatch", MAC2STR(src));
		goto fail;
	}

	status = dpp_get_attr(buf, len, DPP_ATTR_STATUS, &status_len);
	if (!status || status_len != 1) {
		wpa_printf(MSG_ERROR, "DPP: Peer did not include Status");
		wpa_printf(MSG_ERROR, DPP_EVENT_INTRO "peer=" MACSTR
			" fail=missing_status", MAC2STR(src));
		goto fail;
	}
	if (status[0] != DPP_STATUS_OK) {
		wpa_printf(MSG_ERROR,
			   "DPP: Peer rejected network introduction: Status %u",
			   status[0]);
		wpa_printf(MSG_ERROR, DPP_EVENT_INTRO "peer=" MACSTR
			" status=%u", MAC2STR(src), status[0]);
		goto fail;
	}

	connector = dpp_get_attr(buf, len, DPP_ATTR_CONNECTOR, &connector_len);
	if (!connector) {
		wpa_printf(MSG_ERROR,
			   "DPP: Peer did not include its Connector");
		wpa_printf(MSG_ERROR, DPP_EVENT_INTRO "peer=" MACSTR
			" fail=missing_connector", MAC2STR(src));
		goto fail;
	}

	res = dpp_peer_intro(&intro, config->dpp_connector,
			     config->dpp_netaccesskey,
			     config->dpp_netaccesskey_len,
			     config->dpp_csign,
			     config->dpp_csign_len,
			     connector, connector_len, &expiry);
	if (res != DPP_STATUS_OK) {
		wpa_printf(MSG_INFO1,
			   "DPP: Network Introduction protocol resulted in failure");
		wpa_printf(MSG_ERROR, DPP_EVENT_INTRO "peer=" MACSTR
			" fail=peer_connector_validation_failed", MAC2STR(src));
		goto fail;
	}
	if (expiry) {
		os_get_time(&now);
		seconds = expiry - now.sec;
	} else {
		seconds = 86400 * 7;
	}
	os_get_time(&rnow);
	expiration = rnow.sec + seconds;
	
	wpa_printf(MSG_DEBUG, DPP_EVENT_INTRO "peer=" MACSTR
		" status=%u", MAC2STR(src), status[0]);


	wpa_printf(MSG_INFO1,
		   "DPP: Try connection again after successful network introduction");
	if (wapp_set_pmk(wapp, src, intro.pmk, intro.pmk_len,
				intro.pmkid, expiration,
				WPA_KEY_MGMT_DPP, wdev) < 0) {
		wpa_printf(MSG_ERROR, "DPP: Failed to add PMKSA cache entry");
		return;
	}
	wdev_enable_apcli_iface(wapp, wdev, 1);

	return;
fail:
	os_memset(&intro, 0, sizeof(intro));
}

static void
wapp_dpp_rx_pkex_exchange_req(struct wifi_app *wapp, struct wapp_dev *wdev,
				const u8 *src,
				 const u8 *buf, size_t len,
				 unsigned int chan)
{
	struct wpabuf *msg;

	if (!wdev) {
		return;
	}

	wpa_printf(MSG_DEBUG, "DPP: PKEX Exchange Request from " MACSTR,
		   MAC2STR(src));

	/* TODO: Support multiple PKEX codes by iterating over all the enabled
	 * values here */

	if (!wapp->dpp->dpp_pkex_code || !wapp->dpp->dpp_pkex_bi) {
		wpa_printf(MSG_DEBUG,
			   "DPP: No PKEX code configured - ignore request");
		return;
	}

	if (wapp->dpp->dpp_pkex) {
		/* TODO: Support parallel operations */
		wpa_printf(MSG_DEBUG,
			   "DPP: Already in PKEX session - ignore new request");
		return;
	}

	wapp->dpp->dpp_pkex = dpp_pkex_rx_exchange_req(wapp,
						  wapp->dpp->dpp_pkex_bi,
						  wdev->mac_addr, src,
						  wapp->dpp->dpp_pkex_identifier,
						  wapp->dpp->dpp_pkex_code,
						  buf, len);
	if (!wapp->dpp->dpp_pkex) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Failed to process the request - ignore it");
		return;
	}

	msg = wapp->dpp->dpp_pkex->exchange_resp;
	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan,
		DPP_PA_PKEX_EXCHANGE_RESP);
	wapp_drv_send_action(wapp, wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	if (wapp->dpp->dpp_pkex->failed) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Terminate PKEX exchange due to an earlier error");
		if (wapp->dpp->dpp_pkex->t > wapp->dpp->dpp_pkex->own_bi->pkex_t)
			wapp->dpp->dpp_pkex->own_bi->pkex_t = wapp->dpp->dpp_pkex->t;
		dpp_pkex_free(wapp->dpp->dpp_pkex);
		wapp->dpp->dpp_pkex = NULL;
	}
}


static void
wapp_dpp_rx_pkex_exchange_resp(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
				  const u8 *buf, size_t len, unsigned int chan)
{
	struct wpabuf *msg;

	wpa_printf(MSG_DEBUG, "DPP: PKEX Exchange Response from " MACSTR,
		   MAC2STR(src));

	/* TODO: Support multiple PKEX codes by iterating over all the enabled
	 * values here */

	if (!wapp->dpp->dpp_pkex || !wapp->dpp->dpp_pkex->initiator ||
	    wapp->dpp->dpp_pkex->exchange_done) {
		wpa_printf(MSG_DEBUG, "DPP: No matching PKEX session");
		return;
	}

	msg = dpp_pkex_rx_exchange_resp(wapp->dpp->dpp_pkex, src, buf, len);
	if (!msg) {
		wpa_printf(MSG_DEBUG, "DPP: Failed to process the response");
		return;
	}

	wpa_printf(MSG_DEBUG, "DPP: Send PKEX Commit-Reveal Request to " MACSTR,
		   MAC2STR(src));

	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan,
		DPP_PA_PKEX_COMMIT_REVEAL_REQ);
	wapp_drv_send_action(wapp, wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);
}


static void
wapp_dpp_rx_pkex_commit_reveal_req(struct wifi_app *wapp, const u8 *src,
				      const u8 *hdr, const u8 *buf, size_t len,
				      unsigned int chan)
{
	struct wpabuf *msg;
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;
	struct dpp_bootstrap_info *bi;

	wpa_printf(MSG_DEBUG, "DPP: PKEX Commit-Reveal Request from " MACSTR,
		   MAC2STR(src));

	if (!pkex || pkex->initiator || !pkex->exchange_done) {
		wpa_printf(MSG_DEBUG, "DPP: No matching PKEX session");
		return;
	}

	msg = dpp_pkex_rx_commit_reveal_req(pkex, hdr, buf, len);
	if (!msg) {
		wpa_printf(MSG_DEBUG, "DPP: Failed to process the request");
		if (wapp->dpp->dpp_pkex->failed) {
			wpa_printf(MSG_DEBUG, "DPP: Terminate PKEX exchange");
			if (wapp->dpp->dpp_pkex->t > wapp->dpp->dpp_pkex->own_bi->pkex_t)
				wapp->dpp->dpp_pkex->own_bi->pkex_t =
					wapp->dpp->dpp_pkex->t;
			dpp_pkex_free(wapp->dpp->dpp_pkex);
			wapp->dpp->dpp_pkex = NULL;
		}
		return;
	}

	wpa_printf(MSG_DEBUG, "DPP: Send PKEX Commit-Reveal Response to "
		   MACSTR, MAC2STR(src));

	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan,
		DPP_PA_PKEX_COMMIT_REVEAL_RESP);
	wapp_drv_send_action(wapp, wapp->dpp->dpp_pkex->wdev, chan, 0, src,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);

	bi = dpp_pkex_finish(wapp->dpp, pkex, src, chan);
	if (!bi)
		return;
	wapp->dpp->dpp_pkex = NULL;
}


static void
wapp_dpp_rx_pkex_commit_reveal_resp(struct wifi_app *wapp, const u8 *src,
				       const u8 *hdr, const u8 *buf, size_t len,
				       unsigned int chan)
{
	int res;
	struct dpp_bootstrap_info *bi;
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;
	char cmd[500];

	wpa_printf(MSG_DEBUG, "DPP: PKEX Commit-Reveal Response from " MACSTR,
		   MAC2STR(src));

	if (!pkex || !pkex->initiator || !pkex->exchange_done) {
		wpa_printf(MSG_DEBUG, "DPP: No matching PKEX session");
		return;
	}

	res = dpp_pkex_rx_commit_reveal_resp(pkex, hdr, buf, len);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, "DPP: Failed to process the response");
		return;
	}

	bi = dpp_pkex_finish(wapp->dpp, pkex, src, chan);
	if (!bi)
		return;
	wapp->dpp->dpp_pkex = NULL;

	os_snprintf(cmd, sizeof(cmd), " peer=%u %s",
		    bi->id,
		    wapp->dpp->dpp_pkex_auth_cmd ? wapp->dpp->dpp_pkex_auth_cmd : "");
	wpa_printf(MSG_DEBUG,
		   "DPP: Start authentication after PKEX with parameters: %s",
		   cmd);
	if (wapp_dpp_auth_init(wapp, pkex->wdev, cmd) < 0) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Authentication initialization failed");
		return;
	}
}


void wapp_dpp_rx_action(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
			   const u8 *buf, size_t len, unsigned int chan)
{
	u8 crypto_suite;
	enum dpp_public_action_frame_type type;
	const u8 *hdr,*wlan_hdr;
	u8 da[MAC_ADDR_LEN];
	unsigned int pkex_t;

	if (len < DPP_HDR_LEN)
		return;

	wlan_hdr = buf;
	os_memcpy(da, &wlan_hdr[4], MAC_ADDR_LEN);
	/* skipping the wlan header part */
	buf += 24 + 2;
	len -= (24 - 2);
		
	if (WPA_GET_BE24(buf) != OUI_WFA || buf[3] != DPP_OUI_TYPE)
		return;

	hdr = buf;
	buf += 4;
	len -= 4;
	crypto_suite = *buf++;
	type = *buf++;
	len -= 2;

	wpa_printf(MSG_DEBUG,
		   "DPP: Received DPP Public Action frame crypto suite %u type %d from "
		   MACSTR " chan=%u",
		   crypto_suite, type, MAC2STR(src), chan);
	if (crypto_suite != 1) {
		wpa_printf(MSG_ERROR, "DPP: Unsupported crypto suite %u",
			   crypto_suite);
		wpa_printf(MSG_ERROR, DPP_EVENT_RX "src=" MACSTR
			" chan=%u type=%d ignore=unsupported-crypto-suite",
			MAC2STR(src), chan, type);
		return;
	}

	// TODO kapil default interfaces will be three/two based on radio cnt
	if (is_broadcast_ether_addr(da) && (type == DPP_PA_AUTHENTICATION_REQ)){
		if (IS_MAP_CH_5GH(chan))
			wdev = wapp->dpp->default_5gh_iface;
		else if (IS_MAP_CH_5GL(chan))
			wdev = wapp->dpp->default_5gl_iface;	
		else if (IS_MAP_CH_24G(chan))
			wdev = wapp->dpp->default_2g_iface;				
		else
			return;
	}
	wpa_hexdump(MSG_MSGDUMP, "DPP: Received message attributes", buf, len);
	if (dpp_check_attrs(buf, len) < 0) {
		wpa_printf(MSG_DEBUG, DPP_EVENT_RX "src=" MACSTR
			" chan=%u type=%d ignore=invalid-attributes",
			MAC2STR(src), chan, type);
		return;
	}
	wpa_printf(MSG_INFO1, DPP_EVENT_RX "src=" MACSTR
		" chan=%u type=%d", MAC2STR(src), chan, type);

#ifdef CONFIG_DPP2
	if (dpp_relay_rx_action(wapp->dpp,
				src, hdr, buf, len, chan, NULL, NULL) == 0)
		return;
#endif /* CONFIG_DPP2 */
	switch (type) {
	case DPP_PA_AUTHENTICATION_REQ:
		wapp_dpp_rx_auth_req(wapp, wdev, src, hdr, buf, len, chan);
		break;
	case DPP_PA_AUTHENTICATION_RESP:
		wapp_dpp_rx_auth_resp(wapp, src, hdr, buf, len, chan);
		break;
	case DPP_PA_AUTHENTICATION_CONF:
		wapp_dpp_rx_auth_conf(wapp, src, hdr, buf, len);
		break;
	case DPP_PA_PEER_DISCOVERY_REQ:
		wapp_dpp_rx_peer_disc_req(wapp, wdev, src, buf, len, chan);
		break;
	case DPP_PA_PEER_DISCOVERY_RESP:
		wapp_dpp_rx_peer_disc_resp(wapp, wdev, src, buf, len);
		break;
	case DPP_PA_PKEX_EXCHANGE_REQ:
		wapp_dpp_rx_pkex_exchange_req(wapp, wdev, src, buf, len, chan);
		break;
	case DPP_PA_PKEX_EXCHANGE_RESP:
		wapp_dpp_rx_pkex_exchange_resp(wapp, wdev, src, buf, len, chan);
		break;
	case DPP_PA_PKEX_COMMIT_REVEAL_REQ:
		wapp_dpp_rx_pkex_commit_reveal_req(wapp, src, hdr, buf, len,
						      chan);
		break;
	case DPP_PA_PKEX_COMMIT_REVEAL_RESP:
		wapp_dpp_rx_pkex_commit_reveal_resp(wapp, src, hdr, buf, len,
						       chan);
		break;
#ifdef CONFIG_DPP2
	case DPP_PA_CONFIGURATION_RESULT:
		wapp_dpp_rx_conf_result(wapp, src, hdr, buf, len);
		break;
#endif /* CONFIG_DPP2 */
	default:
		wpa_printf(MSG_DEBUG,
			   "DPP: Ignored unsupported frame subtype %d", type);
		break;
	}

	if (wapp->dpp->dpp_pkex)
		pkex_t = wapp->dpp->dpp_pkex->t;
	else if (wapp->dpp->dpp_pkex_bi)
		pkex_t = wapp->dpp->dpp_pkex_bi->pkex_t;
	else
		pkex_t = 0;
	if (pkex_t >= PKEX_COUNTER_T_LIMIT) {
		wpa_printf(MSG_INFO1, DPP_EVENT_PKEX_T_LIMIT "id=0");
		wapp_dpp_pkex_remove(wapp, "*");
	}
}

struct wpabuf *
wapp_dpp_gas_req_handler(void *ctx, const u8 *sa,
			    const u8 *query, size_t query_len,
				u8 *data, size_t data_len)
{
	struct wifi_app *wapp = (struct wifi_app *)ctx;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)sa);
	struct wpabuf *resp;

	wapp_dpp_cancel_timeouts(wapp,auth);
	wpa_printf(MSG_INFO1, "DPP: GAS request from " MACSTR, MAC2STR(sa));
	if (!auth || !auth->auth_success ||
	    os_memcmp(sa, auth->peer_mac_addr, MAC_ADDR_LEN) != 0) {
#ifdef CONFIG_DPP2
		if (dpp_relay_rx_gas_req(wapp->dpp, sa, data,
				     data_len) == 0) {
			/* Response will be forwarded once received over TCP */
			return 0;
		}
#endif /* CONFIG_DPP2 */
		wpa_printf(MSG_ERROR, "DPP: No matching exchange in progress");
		return NULL;
	}

	if (auth->current_state != DPP_STATE_CONFIG_REQ_WAITING) {
		wpa_printf(MSG_ERROR,
			   "DPP: Incorrect auth state=%d", auth->current_state);
		dpp_auth_deinit(auth);
		return NULL;
	}
	wpa_hexdump(MSG_DEBUG,
		    "DPP: Received Configuration Request (GAS Query Request)",
		    query, query_len);
	wpa_printf(MSG_DEBUG, DPP_EVENT_CONF_REQ_RX "src=" MACSTR,
		MAC2STR(sa));
	resp = dpp_conf_req_rx(auth, query, query_len);
	if (!resp)
		wpa_printf(MSG_INFO1, DPP_EVENT_CONF_FAILED);

	return resp;
}


void wapp_dpp_gas_status_handler(void *ctx, u8 *dst, int ok)
{
	struct wifi_app *wapp = (struct wifi_app *)ctx;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, dst);

	if (!auth)
		return;

	wpa_printf(MSG_INFO1, "DPP: Configuration exchange completed (ok=%d)",
		   ok);
	wapp_dpp_cancel_timeouts(wapp,auth);
#ifdef CONFIG_DPP2
	if (ok && auth->peer_version >= 2 &&
	    auth->conf_resp_status == DPP_STATUS_OK) {
		wpa_printf(MSG_DEBUG, "DPP: Wait for Configuration Result");
		auth->waiting_conf_result = 1;
		eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout,
				     wapp, auth);
		eloop_register_timeout(2, 0, //kapil to do
				       wapp_dpp_config_result_wait_timeout,
				       wapp, auth);
		return;
	}
#endif /* CONFIG_DPP2 */
	wapp_cancel_remain_on_channel(wapp, auth->wdev);

	if (ok)
		wpa_printf(MSG_INFO1, DPP_EVENT_CONF_SENT);
	else
		wpa_printf(MSG_INFO1, DPP_EVENT_CONF_FAILED);
	dpp_auth_deinit(auth);
}

int wapp_dpp_configurator_sign(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd)
{
	struct dpp_authentication *auth;
	int ret = -1;
	char *curve = NULL;
	int ap = 1;

	auth = os_zalloc(sizeof(*auth));
	if (!auth)
		return -1;

	curve = get_param(cmd, " curve=");

	if (wdev->dev_type == WAPP_DEV_TYPE_STA)
		ap = 0;

	if (dpp_set_configurator(wapp->dpp, wapp,
				 auth, cmd) == 0 &&
	    dpp_configurator_own_config(auth, curve, ap) == 0) {
		auth->wdev = wdev;
		wapp_dpp_handle_config_obj(wapp, auth);
		ret = 0;
	}

	dpp_auth_deinit(auth);
	os_free(curve);

	return ret;
}

#ifdef CONFIG_DPP2

static void wapp_dpp_relay_tx(void *ctx, struct wapp_dev *wdev,
				const u8 *addr, unsigned int chan,
				 const u8 *msg, size_t len)
{
	struct wifi_app *wapp = ctx;
	u8 *buf;

	wpa_printf(MSG_DEBUG, "DPP: Send action frame dst=" MACSTR " chan=%u",
		   MAC2STR(addr), chan);
	buf = os_malloc(2 + len);
	if (!buf)
		return;
	buf[0] = WLAN_ACTION_PUBLIC;
	buf[1] = WLAN_PA_VENDOR_SPECIFIC;
	os_memcpy(buf + 2, msg, len);
	wapp_drv_send_action(wapp, wdev, chan, 0, addr, buf, 2 + len);
	os_free(buf);
}


static void wapp_dpp_relay_gas_resp_tx(void *ctx, const u8 *addr,
					  u8 dialog_token, int prot,
					  struct wpabuf *buf)
{
	struct wifi_app *wapp = ctx;

	gas_serv_req_dpp_processing(wapp->dpp->gas_server, wdev, addr, dialog_token, prot, buf);
}

#endif /* CONFIG_DPP2 */

static void wapp_gas_server_tx(void *ctx, struct wapp_dev *wdev,
				int chan, const u8 *da,
                               struct wpabuf *buf, unsigned int wait_time)
{
        struct wifi_app *wapp = ctx;

	wapp_drv_send_action(wapp, wdev, chan, wait_time,
			       da,
			       wpabuf_head(buf),
			       wpabuf_len(buf));
}

int wapp_dpp_add_controllers(struct wifi_app *wapp)
{
#ifdef CONFIG_DPP2
	struct dpp_relay_config config;

	os_memset(&config, 0, sizeof(config));
	config.cb_ctx = wapp;
	config.tx = wapp_dpp_relay_tx;
	config.gas_resp_tx = wapp_dpp_relay_gas_resp_tx;

#if 0
	config.ipaddr = &wapp->controller_conf.ipaddr;
	config.pkhash = wapp->controller_conf.pkhash;
#endif
	if (dpp_relay_add_controller(wapp->dpp,
				     &config) < 0)
		return -1;
#endif /* CONFIG_DPP2 */

	return 0;
}

int wapp_dpp_gas_server_init(struct wifi_app *wapp)
{
	wapp->dpp->gas_server = gas_server_init(wapp, wapp_gas_server_tx);
	u8 adv_proto_id[7];

	adv_proto_id[0] = WLAN_EID_VENDOR_SPECIFIC;
	adv_proto_id[1] = 5;
	WPA_PUT_BE24(&adv_proto_id[2], OUI_WFA);
	adv_proto_id[5] = DPP_OUI_TYPE;
	adv_proto_id[6] = 0x01;
	if (gas_server_register(wapp->dpp->gas_server, adv_proto_id,
				sizeof(adv_proto_id), wapp_dpp_gas_req_handler,
				wapp_dpp_gas_status_handler, wapp) < 0)
		return -1;
	return 0;
}

void wapp_ap_dpp_deinit(struct wifi_app *wapp)
{
	if (!wapp->dpp->dpp_init_done)
		return;
	dpp_auth_list_deinit(wapp);
	wapp_dpp_pkex_remove(wapp, "*");
	wapp->dpp->dpp_pkex = NULL;
	os_free(wapp->dpp->dpp_configurator_params);
	wapp->dpp->dpp_configurator_params = NULL;
}

#ifdef CONFIG_DPP2
int wapp_dpp_controller_start(struct wifi_app *wapp, const char *cmd)
{
        struct dpp_controller_config config;
        const char *pos;

        os_memset(&config, 0, sizeof(config));
        if (cmd) {
                pos = os_strstr(cmd, " tcp_port=");
                if (pos) {
                        pos += 10;
                        config.tcp_port = atoi(pos);
                }
        }
        config.configurator_params = wapp->dpp->dpp_configurator_params;
        return dpp_controller_start(wapp->dpp, &config);
}
#endif

int wapp_dpp_pkex_add(struct wifi_app *wapp,  struct wapp_dev *wdev, const char *cmd)
{
	struct dpp_bootstrap_info *own_bi;
	const char *pos, *end;
	unsigned int wait_time;

	pos = os_strstr(cmd, " own=");
	if (!pos)
		return -1;
	pos += 5;
	own_bi = dpp_bootstrap_get_id(wapp->dpp, atoi(pos));
	if (!own_bi) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Identified bootstrap info not found");
		return -1;
	}
	if (own_bi->type != DPP_BOOTSTRAP_PKEX) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Identified bootstrap info not for PKEX");
		return -1;
	}
	wapp->dpp->dpp_pkex_bi = own_bi;
	own_bi->pkex_t = 0; /* clear pending errors on new code */

	os_free(wapp->dpp->dpp_pkex_identifier);
	wapp->dpp->dpp_pkex_identifier = NULL;

	pos = os_strstr(cmd, " identifier=");
	if (pos) {
		pos += 12;
		end = os_strchr(pos, ' ');
		if (!end)
			return -1;
		wapp->dpp->dpp_pkex_identifier = os_malloc(end - pos + 1);
		if (!wapp->dpp->dpp_pkex_identifier)
			return -1;
		os_memcpy(wapp->dpp->dpp_pkex_identifier, pos, end - pos);
		wapp->dpp->dpp_pkex_identifier[end - pos] = '\0';
	}

	pos = os_strstr(cmd, " code=");
	if (!pos)
		return -1;
	os_free(wapp->dpp->dpp_pkex_code);
	wapp->dpp->dpp_pkex_code = os_strdup(pos + 6);
	if (!wapp->dpp->dpp_pkex_code)
		return -1;

	if (os_strstr(cmd, " init=1")) {
		struct dpp_pkex *pkex;
		struct wpabuf *msg;

		wpa_printf(MSG_DEBUG, "DPP: Initiating PKEX");
		dpp_pkex_free(wapp->dpp->dpp_pkex);
		wapp->dpp->dpp_pkex = dpp_pkex_init(wapp, own_bi, wdev->mac_addr,
						wapp->dpp->dpp_pkex_identifier,
						wapp->dpp->dpp_pkex_code);
		pkex = wapp->dpp->dpp_pkex;
		if (!pkex)
			return -1;

		msg = pkex->exchange_req;
		wait_time = wapp->dpp->max_remain_on_chan;
		if (wait_time > 2000)
			wait_time = 2000;
		pkex->chan = 2437;
		pkex->wdev = wdev;
		wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
			" chan=%u type=%d",
			MAC2STR(broadcast), pkex->chan,
			DPP_PA_PKEX_EXCHANGE_REQ);
		wapp_drv_send_action(wapp, wdev, pkex->chan, wait_time,
 					broadcast,
					wpabuf_head(msg),
					wpabuf_len(msg));

#if 0
		offchannel_send_action(wapp, wdev, pkex->chan, broadcast,
				       wdev->mac_addr, broadcast,
				       wpabuf_head(msg), wpabuf_len(msg),
				       wait_time, wapp_dpp_tx_pkex_status, 0);
#endif
		if (wait_time == 0)
			wait_time = 2000;
		pkex->exch_req_wait_time = wait_time;
		pkex->exch_req_tries = 1;
	}

	/* TODO: Support multiple PKEX info entries */

	os_free(wapp->dpp->dpp_pkex_auth_cmd);
	wapp->dpp->dpp_pkex_auth_cmd = os_strdup(cmd);

	return 1;
}


int wapp_dpp_pkex_remove(struct wifi_app *wapp, const char *id)
{
	unsigned int id_val;

	if (os_strcmp(id, "*") == 0) {
		id_val = 0;
	} else {
		id_val = atoi(id);
		if (id_val == 0)
			return -1;
	}

	if ((id_val != 0 && id_val != 1) || !wapp->dpp->dpp_pkex_code)
		return -1;

	/* TODO: Support multiple PKEX entries */
	os_free(wapp->dpp->dpp_pkex_code);
	wapp->dpp->dpp_pkex_code = NULL;
	os_free(wapp->dpp->dpp_pkex_identifier);
	wapp->dpp->dpp_pkex_identifier = NULL;
	os_free(wapp->dpp->dpp_pkex_auth_cmd);
	wapp->dpp->dpp_pkex_auth_cmd = NULL;
	wapp->dpp->dpp_pkex_bi = NULL;
	/* TODO: Remove dpp_pkex only if it is for the identified PKEX code */
	dpp_pkex_free(wapp->dpp->dpp_pkex);
	wapp->dpp->dpp_pkex = NULL;
	return 0;
}


void wapp_dpp_stop(struct wifi_app *wapp)
{
	dpp_auth_list_deinit(wapp);
	dpp_pkex_free(wapp->dpp->dpp_pkex);
	wapp->dpp->dpp_pkex = NULL;
	if (wapp->dpp->dpp_gas_client && wapp->dpp->dpp_gas_dialog_token >= 0)
		gas_query_stop(wapp->dpp->gas_query_ctx, wapp->dpp->dpp_gas_dialog_token);
}

static void wapp_dpp_auth_resp_retry_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_authentication *auth = timeout_ctx;

	if (!auth || !auth->resp_msg)
		return;

	wpa_printf(MSG_INFO1,
		   "DPP: Retry Authentication Response after timeout");
	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR
		" chan=%u type=%d",
		MAC2STR(auth->peer_mac_addr), auth->curr_chan,
		DPP_PA_AUTHENTICATION_RESP);
	auth->current_state = DPP_STATE_AUTH_CONF_WAITING;
	wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 500, auth->peer_mac_addr,
				wpabuf_head(auth->resp_msg), wpabuf_len(auth->resp_msg));

	if(!wapp->dpp->dpp_qr_mutual)
	{
		eloop_cancel_timeout(wapp_dpp_auth_conf_wait_timeout, wapp, auth);
		eloop_register_timeout(DPP_AUTH_WAIT_TIMEOUT, 0, wapp_dpp_auth_conf_wait_timeout, wapp, auth);
	}
}

static void wapp_dpp_auth_resp_retry(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	unsigned int wait_time, max_tries;

	if (!auth || !auth->resp_msg)
		return;

	if (wapp->dpp->dpp_resp_max_tries)
		max_tries = wapp->dpp->dpp_resp_max_tries;
	else
		max_tries = 5;
	auth->auth_resp_tries++;
	if (auth->auth_resp_tries >= max_tries) {
		wpa_printf(MSG_INFO1, "DPP: No confirm received from initiator - stopping exchange");
		dpp_auth_deinit(auth);
		return;
	}

	if (wapp->dpp->dpp_resp_retry_time)
		wait_time = wapp->dpp->dpp_resp_retry_time;
	else
		wait_time = 1000;
	wpa_printf(MSG_DEBUG,
		   "DPP: Schedule retransmission of Authentication Response frame in %u ms",
		wait_time);
	eloop_cancel_timeout(wapp_dpp_auth_resp_retry_timeout, wapp, auth);
	eloop_register_timeout(wait_time / 1000,
			       (wait_time % 1000) * 1000,
			       wapp_dpp_auth_resp_retry_timeout, wapp, auth);
}

int wpa_start_roc(struct wifi_app *wapp, unsigned int chan, struct wapp_radio *radio, unsigned int wait_time)
{
	struct wapp_dev *target_wdev = NULL, *wdev = NULL;
	struct dl_list *dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->radio == radio && wdev->dev_type == WAPP_DEV_TYPE_STA) {
			target_wdev = wdev;
			break;
		}
	}

	if (target_wdev)
        	wapp->drv_ops->drv_start_roc(wapp->drv_data, wdev->ifname, chan, wait_time);
	return 0;
}

static int wapp_dpp_listen_start(struct wifi_app *wapp, struct wapp_dev *wdev,
		unsigned int channel)
{
	if (!((IS_MAP_CH_24G(channel) && IS_MAP_CH_24G(wdev->radio->op_ch)) ||
	      (IS_MAP_CH_5GL(channel) && IS_MAP_CH_5GL(wdev->radio->op_ch)) ||
	      (IS_MAP_CH_5GH(channel) && IS_MAP_CH_5GH(wdev->radio->op_ch)))) {
		wpa_printf(MSG_DEBUG, "Invalid channel %d interface combo",
			   channel);
		return -1;
	}
	wdev_set_quick_ch(wapp, wdev, channel);

	/* TODO should these also be band specific?? */
	wapp->dpp->dpp_listen_chan = channel;
	wapp->dpp->off_channel_chan = channel;

	return 0;
}

int wapp_dpp_listen(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd)
{
	int chan;

	chan = atoi(cmd);
	if (chan <= 0)
		return -1;

	wapp->dpp->dpp_qr_mutual = os_strstr(cmd, " qr=mutual") != NULL;

	if(IS_MAP_CH_24G(wdev->radio->op_ch))
		wapp->dpp->default_2g_iface = wdev;
	else if(IS_MAP_CH_5GL(wdev->radio->op_ch))
		wapp->dpp->default_5gh_iface = wdev;
	else if(IS_MAP_CH_5GH(wdev->radio->op_ch))
		wapp->dpp->default_5gl_iface = wdev;
	else
		return -1;

	if (wapp->dpp->dpp_listen_chan == (unsigned int) chan) {
		wpa_printf(MSG_DEBUG, "DPP: Already listening on %u MHz",
			   chan);
		return 0;
	}

	return wapp_dpp_listen_start(wapp, wdev, chan);
}

void wapp_dpp_listen_stop(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	wapp->dpp->dpp_in_response_listen = 0;
	if (!wapp->dpp->dpp_listen_chan)
		return;

	wpa_printf(MSG_DEBUG, "DPP: Stop listen on %u MHz",
		   wapp->dpp->dpp_listen_chan);
	wapp_cancel_remain_on_channel(wapp, wdev);
	wapp->dpp->dpp_listen_chan = 0;
}

static struct dpp_config * wapp_dpp_add_network(struct wifi_app *wapp,
						struct wapp_dev *wdev,
					      struct dpp_authentication *auth)
{
	struct dpp_config *wdev_config = os_zalloc(sizeof(*wdev_config));

	wdev_config->ssid = os_malloc(auth->ssid_len);
	if (!wdev_config->ssid)
		goto fail;
	os_memcpy(wdev_config->ssid, auth->ssid, auth->ssid_len);
	wdev_config->ssid_len = auth->ssid_len;

	if (auth->connector) {
		wdev_config->key_mgmt = WPA_KEY_MGMT_DPP;
		wdev_config->ieee80211w = MGMT_FRAME_PROTECTION_REQUIRED;
		wdev_config->dpp_connector = os_strdup(auth->connector);
		if (!wdev_config->dpp_connector)
			goto fail;
	}

	if (auth->c_sign_key) {
		wdev_config->dpp_csign = os_malloc(wpabuf_len(auth->c_sign_key));
		if (!wdev_config->dpp_csign)
			goto fail;
		os_memcpy(wdev_config->dpp_csign, wpabuf_head(auth->c_sign_key),
			  wpabuf_len(auth->c_sign_key));
		wdev_config->dpp_csign_len = wpabuf_len(auth->c_sign_key);
	}

	if (auth->net_access_key) {
		wdev_config->dpp_netaccesskey =
			os_malloc(wpabuf_len(auth->net_access_key));
		if (!wdev_config->dpp_netaccesskey)
			goto fail;
		os_memcpy(wdev_config->dpp_netaccesskey,
			  wpabuf_head(auth->net_access_key),
			  wpabuf_len(auth->net_access_key));
		wdev_config->dpp_netaccesskey_len = wpabuf_len(auth->net_access_key);
		wdev_config->dpp_netaccesskey_expiry = auth->net_access_key_expiry;
	}

	if (!auth->connector || dpp_akm_psk(auth->akm) ||
	    dpp_akm_sae(auth->akm)) {
		if (!auth->connector)
			wdev_config->key_mgmt = 0;
		if (dpp_akm_psk(auth->akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_PSK |
				WPA_KEY_MGMT_PSK_SHA256 | WPA_KEY_MGMT_FT_PSK;
		if (dpp_akm_sae(auth->akm))
			wdev_config->key_mgmt |= WPA_KEY_MGMT_SAE |
				WPA_KEY_MGMT_FT_SAE;
		wdev_config->ieee80211w = MGMT_FRAME_PROTECTION_OPTIONAL;
#if 0 // Kapil not needed atm
		if (auth->passphrase[0]) {
			if (wpa_config_set_quoted(ssid, "psk",
						  auth->passphrase) < 0)
				goto fail;
			wpa_config_update_psk(ssid);
			ssid->export_keys = 1;
		} else
#endif
		{
			wdev_config->psk_set = auth->psk_set;
			os_memcpy(wdev_config->psk, auth->psk, PMK_LEN);
		}
	}

	if (wdev && wdev->config)
		os_free(wdev->config);

	/* Set this config in ongoing wdev */
	wdev->config = wdev_config;
	return wdev_config;
fail:
	return NULL;
}

static int wapp_wdev_dpp_scan(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	wapp_issue_scan_request(wapp, wdev);
	wdev->scan_cookie = random();
	wdev->connection_tries++;
	//eloop_register_timeout(10, 0, map_get_scan_result, wapp, wdev);  //TODO check the timeout clearing
	return 0;
}

static int wapp_dpp_process_config(struct wifi_app *wapp,
				    struct dpp_authentication *auth)
{
	struct dpp_config *config;

	// TODO kapil, this will be modified for MAP-R2, DPP-R2
	struct wapp_dev *wdev =  auth->wdev;
	config = wapp_dpp_add_network(wapp, wdev, auth);
	if (config->psk_set) {
		/* MAP turnkey feature, send the event to mapd */
	}
	/* Need to see how this can be broken if possible for best AP selection */
	/* food for Thought: current design of best AP selection itself is wrong, scan should be triggered 
	   from wapp and wapp should ask for best AP out of matched bss */

	/* set config parameters as per interface */
	wdev_set_dpp_akm(wapp, wdev, auth->akm);
	wdev_enable_pmf(wapp,wdev);

	if (auth->passphrase[0]) {
		wdev_set_psk(wapp, wdev, auth->passphrase);
	}
	wdev_set_ssid(wapp, wdev, (char *)auth->ssid);
	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		// TODO add more dpp mixed modes
		if(dpp_akm_dpp(auth->akm))
			wapp_wdev_dpp_scan(wapp, wdev);
		else
			wdev_enable_apcli_iface(wapp, wdev, 1);
	}
	return 0;
}

static void wapp_dpp_gas_resp_cb(void *ctx, const u8 *addr, u8 dialog_token,
				 enum gas_query_result result,
				 const struct wpabuf *adv_proto,
				 const struct wpabuf *resp, u16 status_code)
{
	struct wifi_app *wapp = ctx;
	const u8 *pos;
	struct dpp_authentication *auth = wapp_dpp_get_auth_from_peer_mac(wapp, (u8 *)addr);
	enum dpp_status_error status = DPP_STATUS_CONFIG_REJECTED;

	wapp->dpp->dpp_gas_dialog_token = -1;

	if (!auth || !auth->auth_success) {
		wpa_printf(MSG_ERROR, "DPP: No matching exchange in progress");
		return;
	}

	wpa_printf(MSG_INFO1, "DPP: GAS response from " MACSTR, MAC2STR(addr));
	if (auth->current_state != DPP_STATE_CONFIG_RSP_WAITING) {
		wpa_printf(MSG_ERROR,
			   "DPP: Incorrect auth state=%d", auth->current_state);
		goto fail;
	}
	if (result != GAS_QUERY_SUCCESS ||
	    !resp || status_code != WLAN_STATUS_SUCCESS) {
		wpa_printf(MSG_ERROR, "DPP: GAS query did not succeed");
		goto fail;
	}

	wpa_hexdump_buf(MSG_DEBUG, "DPP: Configuration Response adv_proto",
			adv_proto);
	wpa_hexdump_buf(MSG_DEBUG, "DPP: Configuration Response (GAS response)",
			resp);

	if (wpabuf_len(adv_proto) != 10 ||
	    !(pos = wpabuf_head(adv_proto)) ||
	    pos[0] != WLAN_EID_ADV_PROTO ||
	    pos[1] != 8 ||
	    pos[3] != WLAN_EID_VENDOR_SPECIFIC ||
	    pos[4] != 5 ||
	    WPA_GET_BE24(&pos[5]) != OUI_WFA ||
	    pos[8] != 0x1a ||
	    pos[9] != 1) {
		wpa_printf(MSG_ERROR,
			   "DPP: Not a DPP Advertisement Protocol ID");
		goto fail;
	}

	if (dpp_conf_resp_rx(auth, resp) < 0) {
		wpa_printf(MSG_ERROR, "DPP: Configuration attempt failed");
		goto fail;
	}
	status = DPP_STATUS_OK;
fail:
	if (status != DPP_STATUS_OK)
		wpa_printf(MSG_INFO1, DPP_EVENT_CONF_FAILED);
#ifdef CONFIG_DPP2
	if (auth->peer_version >= 2 &&
	    auth->conf_resp_status == DPP_STATUS_OK) {
		struct wpabuf *msg;

		wpa_printf(MSG_DEBUG, "DPP: Send DPP Configuration Result");
		msg = dpp_build_conf_result(auth, status);
		if (!msg)
			goto fail2;

                wpa_printf(MSG_INFO1,
                        DPP_EVENT_TX "dst=" MACSTR " chan=%u type=%d",
                        MAC2STR(addr), auth->curr_chan,
                        DPP_PA_CONFIGURATION_RESULT);
		auth->current_state = DPP_STATE_CONFIG_RESULT_WAITING;
		wapp_drv_send_action(wapp, auth->wdev, auth->curr_chan, 500,
					addr, wpabuf_head(msg), wpabuf_len(msg));
                wpabuf_free(msg);

		/* This exchange will be terminated in the TX status handler */
		return;
	}
fail2:
#endif /* CONFIG_DPP2 */
	dpp_auth_deinit(auth);
}

int append_map_version_tlv(unsigned char *pkt, unsigned char version)
{
	unsigned char *temp_buf = NULL;
	unsigned short total_length = 0;

	temp_buf = pkt;
	total_length = 4;
	(*temp_buf) = MULTI_AP_VERSION_TYPE;
	*(unsigned short *)(temp_buf+1) = cpu2be16(total_length-3);
	*(temp_buf+3) = version;

	return total_length;
}

unsigned short append_supported_service_tlv(
		unsigned char *pkt, unsigned char service)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;

	*temp_buf = SUPPORTED_SERVICE_TLV_TYPE;
	temp_buf +=1;
	/**
	 * need communicate with WAPP to get supported service
	 * or just write hard code to Multi-AP Agent
	 */
	if (service == 0) {
		*(unsigned short *)(temp_buf) = cpu2be16(SUPPORTED_SERVICE_LENGTH);
		temp_buf += 2;
		*temp_buf++ = 1;
		*temp_buf = SERVICE_CONTROLLER;
		total_length = SUPPORTED_SERVICE_LENGTH + 3;
	} else if (service == 1) {
		*(unsigned short *)(temp_buf) = cpu2be16(SUPPORTED_SERVICE_LENGTH);
		temp_buf += 2;
		*temp_buf++ = 1;
		*temp_buf = SERVICE_AGENT;
		total_length = SUPPORTED_SERVICE_LENGTH + 3;
	} else if (service == 2) {
		*(unsigned short *)(temp_buf) = cpu2be16(SUPPORTED_SERVICE2_LENGTH);
		temp_buf += 2;
		*temp_buf++ = 2;
		*temp_buf++ = SERVICE_CONTROLLER;
		*temp_buf = SERVICE_AGENT;
		total_length = SUPPORTED_SERVICE2_LENGTH + 3;
	} else {
		printf("unvalid service\n");
	}

	return total_length;
}

unsigned short append_ap_radio_basic_capability_tlv(unsigned char *pkt,
		struct ap_radio_basic_cap *bcap)
{
	unsigned short total_length = 0, tmp_len = 0, i;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = AP_RADIO_BASIC_CAPABILITY_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, bcap->identifier, MAC_ADDR_LEN);
	temp_buf += MAC_ADDR_LEN;
	total_length += MAC_ADDR_LEN;

	*temp_buf = bcap->max_bss_num;
	temp_buf += 1;
	total_length += 1;

	temp_buf[0] = bcap->op_class_num;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < bcap->op_class_num; i++)
	{
		tmp_len = 3 + bcap->opcap[i].non_operch_num;
		memcpy(temp_buf, &bcap->opcap[i], tmp_len);
		temp_buf += tmp_len;
		total_length += tmp_len;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

unsigned short append_backhaul_sta_radio_caps_tlv(unsigned char *pkt,
		struct radio_bhsta_caps *cap)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = BACKHAUL_STATION_RADIO_CAP_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, cap->identifier, MAC_ADDR_LEN);
	temp_buf += MAC_ADDR_LEN;
	total_length += MAC_ADDR_LEN;

	if (!cap->is_sta_mac) {
		*temp_buf = 0x1;
		total_length += 1;
	} else {
		memcpy(temp_buf, cap->sta_mac, MAC_ADDR_LEN);
		temp_buf += MAC_ADDR_LEN;
		total_length += MAC_ADDR_LEN;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

unsigned short append_ap_radio_advanced_capability_tlv(unsigned char *pkt,
		struct radio_adv_caps *cap)
{
	unsigned short total_length = 0;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = AP_RADIO_ADVANCE_CAP_TLV;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	memcpy(temp_buf, cap->identifier, MAC_ADDR_LEN);
	temp_buf += MAC_ADDR_LEN;
	total_length += MAC_ADDR_LEN;

	*temp_buf = cap->ts_rules_support;
	temp_buf += 1;
	total_length += 1;

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}


unsigned short append_r2_ap_caps(unsigned char *pkt, struct r2_ap_caps *caps)
{
	unsigned short total_length = 0, i;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = R2_CAP_TLV_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	*(unsigned short *)(temp_buf) = cpu2be16(caps->max_sp_rule_cnt);
	temp_buf += 2;
	total_length += 2;

	*(unsigned short *)(temp_buf) = cpu2be16(caps->max_adv_sp_rule_cnt);
	temp_buf += 2;
	total_length += 2;

	*(unsigned short *)(temp_buf) = cpu2be16(caps->max_destination_addr);
	temp_buf += 2;
	total_length += 2;

	*temp_buf = caps->byte_count_unit;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = caps->max_vid;
	temp_buf += 1;
	total_length += 1;

	*temp_buf = caps->eth_edge_iface_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->eth_edge_iface_cnt; i++)
	{
		memcpy(temp_buf, caps->addr[i], MAC_ADDR_LEN);
		temp_buf += MAC_ADDR_LEN;
		total_length += MAC_ADDR_LEN;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

unsigned short append_akm_suite_capability(unsigned char *pkt,
		struct akm_suite_caps *caps)
{
	unsigned short total_length = 0, i;
	unsigned char *temp_buf;

	temp_buf = pkt;
	*temp_buf = AKM_SUITE_TLV_TYPE;
	temp_buf += 1;
	total_length += 1;

	temp_buf += 2;
	total_length += 2;

	*temp_buf = caps->bhsta_akm_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->bhsta_akm_cnt; i++)
	{
		memcpy(temp_buf, caps->bhsta_akm[i].oui, 3);
		temp_buf += 3;
		total_length += 3;
		*temp_buf = caps->bhsta_akm[i].oui_type;
		temp_buf += 1;
		total_length += 1;
	}

	*temp_buf = caps->bhap_akm_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->bhap_akm_cnt; i++)
	{
		memcpy(temp_buf, caps->bhap_akm[i].oui, 3);
		temp_buf += 3;
		total_length += 3;
		*temp_buf = caps->bhap_akm[i].oui_type;
		temp_buf += 1;
		total_length += 1;
	}

	*temp_buf = caps->fhap_akm_cnt;
	temp_buf += 1;
	total_length += 1;

	for (i = 0; i < caps->fhap_akm_cnt; i++)
	{
		memcpy(temp_buf, caps->fhap_akm[i].oui, 3);
		temp_buf += 3;
		total_length += 3;
		*temp_buf = caps->fhap_akm[i].oui_type;
		temp_buf += 1;
		total_length += 1;
	}

	/*calculate totoal length & fill into the length field*/
	*(unsigned short *)(pkt+1) = cpu2be16(total_length - 3);

	return total_length;
}

int dpp_get_map_tlv_blob(struct wifi_app *wapp, unsigned char *map_tvl_blob)
{
	int total_len = 0, i;
	int num_radio = 3; //TODO initialize it

	/* version */
	total_len += append_map_version_tlv(map_tvl_blob + total_len, 2);

	/* supported service */
	total_len += append_supported_service_tlv(map_tvl_blob + total_len, 1);
	/* akm suite */
	total_len += append_akm_suite_capability(map_tvl_blob + total_len, &wapp->akm_caps);
	/* radio basic caps multiple */
	for (i = 0; i < num_radio; i++) {
		/* basic radio caps tlv */
		os_memcpy(map_tvl_blob + total_len, wapp->radio[i].bcap_tlv, wapp->radio[i].bcap_len);
		total_len += wapp->radio[i].bcap_len;
		//total_len += append_ap_radio_basic_capability_tlv(map_tvl_blob + total_len, &wapp->wapp->radio[i].bcap);
	}
	/* backhaul sta radio */
	for (i = 0; i < num_radio; i++) {
		total_len += append_backhaul_sta_radio_caps_tlv(map_tvl_blob + total_len, &wapp->radio[i].bhsta_cap);
	}
	/* R2 ap caps */
	total_len += append_r2_ap_caps(map_tvl_blob + total_len, &wapp->r2_ap_cap);
	/* ap radio advaced caps multiple */
	for (i = 0; i < num_radio; i++) {
		total_len += append_ap_radio_advanced_capability_tlv(map_tvl_blob + total_len, &wapp->radio[i].adv_cap);
	}

	wpa_hexdump(0, "tlv blob", map_tvl_blob, total_len);
	return total_len;
}

static void wapp_dpp_start_gas_client(struct wifi_app *wapp)
{
	/* enrollee mode, get the first auth from auth list */
	struct dpp_authentication *auth = wapp_dpp_get_first_auth(wapp);
	struct wpabuf *buf;
	char json[300];
	unsigned char map_tvl_blob[1000];
	int res, len;
	size_t outlen;
	char is_sta = 0;

	wapp->dpp->dpp_gas_client = 1;
	if (!wapp->dpp->is_map) {
		if (auth->wdev->dev_type == WAPP_DEV_TYPE_STA)
			is_sta = 1;
		os_snprintf(json, sizeof(json),
				"{\"name\":\"Test\","
				"\"wi-fi_tech\":\"infra\","
				"\"netRole\":\"%s\"}",
				is_sta ? "sta" : "ap");
		wpa_printf(MSG_DEBUG, "DPP: GAS Config Attributes: %s", json);
	} else {
		len = dpp_get_map_tlv_blob(wapp, map_tvl_blob);
		os_snprintf(json, sizeof(json),
				"{\"name\":\"MTK\","
				"\"wi-fi_tech\":\"map\","
				"\"netRole\":\"mapAgent\","
				"\"mapTLVBlob\":\"%s\"}",
				base64_url_encode(map_tvl_blob, len, &outlen, 0));
		wpa_printf(MSG_DEBUG, "DPP: GAS Config Attributes: %s", json);
	}

	wapp_dpp_listen_stop(wapp, auth->wdev);

	buf = dpp_build_conf_req(auth, json);
	if (!buf) {
		wpa_printf(MSG_ERROR,
			   "DPP: No configuration request data available");
		return;
	}

	wpa_printf(MSG_INFO1, "DPP: GAS request to " MACSTR " (chan %u MHz)",
		   MAC2STR(auth->peer_mac_addr), auth->curr_chan);

	res = gas_query_req(wapp->dpp->gas_query_ctx, auth->wdev, auth->peer_mac_addr, auth->curr_chan,
			    1, buf, wapp_dpp_gas_resp_cb, wapp);
	if (res < 0) {
		wpa_printf(MSG_ERROR, "GAS: Failed to send Query Request");
		wpabuf_free(buf);
	} else {
		wpa_printf(MSG_DEBUG,
			   "DPP: GAS query started with dialog token %u", res);
		wapp->dpp->dpp_gas_dialog_token = res;
	}
}

static void wapp_dpp_auth_success(struct wifi_app *wapp, int initiator, struct wapp_dev *wdev)
{
	wpa_printf(MSG_DEBUG, "DPP: Authentication succeeded init=%d", initiator);
	if (!wapp->dpp->dpp_configurator_supported)
		wapp_dpp_start_gas_client(wapp);
}

#ifdef CONFIG_DPP2

static int wapp_dpp_process_conf_obj(void *ctx,
				     struct dpp_authentication *auth)
{
	struct wifi_app *wapp = ctx;

	return wapp_dpp_handle_config_obj(wapp, auth);
}

#endif /* CONFIG_DPP2 */

void dpp_auth_list_deinit(struct wifi_app *wapp)
{
	struct dpp_authentication *auth = wapp_dpp_get_first_auth(wapp);

	while (auth) {
		eloop_cancel_timeout(wapp_dpp_reply_wait_timeout, wapp, auth);
		eloop_cancel_timeout(wapp_dpp_init_timeout, wapp, auth);
#ifdef CONFIG_DPP2
		eloop_cancel_timeout(wapp_dpp_config_result_wait_timeout, wapp,
			     auth);
#endif /* CONFIG_DPP2 */
		auth = wapp_dpp_get_first_auth(wapp);
		eloop_cancel_timeout(wapp_dpp_auth_resp_retry_timeout, wapp, auth);
		wapp_dpp_listen_stop(wapp, auth->wdev);
		dpp_auth_deinit(auth);
	}
}

static int wapp_dpp_pkex_next_channel(struct wifi_app *wapp,
				      struct dpp_pkex *pkex)
{
	if (pkex->chan == 6)
		pkex->chan = 149;
	else if (pkex->chan == 149)
		pkex->chan = 44;
	else
		return -1; /* no more channels to try */

	/* Could not use this channel - try the next one */
	return wapp_dpp_pkex_next_channel(wapp, pkex);
}


static void wapp_dpp_pkex_retry_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = eloop_ctx;
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;

	if (!pkex || !pkex->exchange_req)
		return;
	if (pkex->exch_req_tries >= 5) {
		if (wapp_dpp_pkex_next_channel(wapp, pkex) < 0) {
			wpa_printf(MSG_INFO1, DPP_EVENT_FAIL
				"No response from PKEX peer");
			dpp_pkex_free(pkex);
			wapp->dpp->dpp_pkex = NULL;
			return;
		}
		pkex->exch_req_tries = 0;
	}

	pkex->exch_req_tries++;
	wpa_printf(MSG_DEBUG, "DPP: Retransmit PKEX Exchange Request (try %u)",
		   pkex->exch_req_tries);
	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR " chan=%u type=%d",
		MAC2STR(broadcast), pkex->chan, DPP_PA_PKEX_EXCHANGE_REQ);
	wapp_drv_send_action(wapp, pkex->wdev, pkex->chan, 0, broadcast,
				wpabuf_head(pkex->exchange_req),
				wpabuf_len(pkex->exchange_req));
#if 0
	offchannel_send_action(wapp, pkex->wdev, pkex->chan, broadcast,
			       pkex->wdev->mac_addr, broadcast,
			       wpabuf_head(pkex->exchange_req),
			       wpabuf_len(pkex->exchange_req),
			       pkex->exch_req_wait_time,
			       wapp_dpp_tx_pkex_status, 0);
#endif
}

void
wapp_dpp_tx_pkex_status(struct wifi_app *wapp,
			unsigned int chan, const u8 *dst,
			const u8 *src, const u8 *bssid,
			const u8 *data, size_t data_len,
			int ok)
{
	struct dpp_pkex *pkex = wapp->dpp->dpp_pkex;

	wpa_printf(MSG_INFO1, DPP_EVENT_TX_STATUS "dst=" MACSTR
		" chan=%u result=%d", MAC2STR(dst), chan, ok);

	if (!pkex) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Ignore TX status since there is no ongoing PKEX exchange");
		return;
	}

	if (pkex->failed) {
		wpa_printf(MSG_DEBUG,
			   "DPP: Terminate PKEX exchange due to an earlier error");
		if (pkex->t > pkex->own_bi->pkex_t)
			pkex->own_bi->pkex_t = pkex->t;
		dpp_pkex_free(pkex);
		wapp->dpp->dpp_pkex = NULL;
		return;
	}

	if (pkex->exch_req_wait_time && pkex->exchange_req) {
		/* Wait for PKEX Exchange Response frame and retry request if
		 * no response is seen. */
		eloop_cancel_timeout(wapp_dpp_pkex_retry_timeout, wapp, NULL);
		eloop_register_timeout(pkex->exch_req_wait_time / 1000,
				       (pkex->exch_req_wait_time % 1000) * 1000,
				       wapp_dpp_pkex_retry_timeout, wapp,
				       NULL);
	}
}

int wapp_dpp_check_connect(struct wifi_app *wapp,
			   struct wapp_dev *wdev,
			   char *bssid, char chan)
{
	struct dpp_config *config = wdev->config;
	struct os_time now;
	struct wpabuf *msg;
	unsigned int wait_time;

	if (!(config->key_mgmt & WPA_KEY_MGMT_DPP) || !bssid)
		return 0; /* Not using DPP AKM - continue */

	/* Kapil: Do we need to maintain a cache here as well? */
#if 0
	if (wpa_sm_pmksa_exists(wapp->dpp->wpa, bssid, config))
		return 0; /* PMKSA exists for DPP AKM - continue */
#endif

	if (!config->dpp_connector || !config->dpp_netaccesskey ||
	    !config->dpp_csign) {
		wpa_printf(MSG_ERROR, DPP_EVENT_MISSING_CONNECTOR
			"missing %s",
			!config->dpp_connector ? "Connector" :
			(!config->dpp_netaccesskey ? "netAccessKey" :
			 "C-sign-key"));
		return -1;
	}

	os_get_time(&now);

	if (config->dpp_netaccesskey_expiry &&
	    (os_time_t) config->dpp_netaccesskey_expiry < now.sec) {
		wpa_printf(MSG_ERROR, DPP_EVENT_MISSING_CONNECTOR
			"netAccessKey expired");
		return -1;
	}

	wpa_printf(MSG_DEBUG,
		   "DPP: Starting network introduction protocol to derive PMKSA for "
		   MACSTR, MAC2STR(bssid));

	msg = dpp_alloc_msg(DPP_PA_PEER_DISCOVERY_REQ,
			    5 + 4 + os_strlen(config->dpp_connector));
	if (!msg)
		return -1;

	/* Transaction ID */
	wpabuf_put_le16(msg, DPP_ATTR_TRANSACTION_ID);
	wpabuf_put_le16(msg, 1);
	wpabuf_put_u8(msg, TRANSACTION_ID);

	/* DPP Connector */
	wpabuf_put_le16(msg, DPP_ATTR_CONNECTOR);
	wpabuf_put_le16(msg, os_strlen(config->dpp_connector));
	wpabuf_put_str(msg, config->dpp_connector);

	/*changing the channel if not same*/
	wapp_cancel_remain_on_channel(wapp, wdev);
	wdev_set_quick_ch(wapp, wdev, chan);
	
	/* TODO: Timeout on AP response */
	wait_time = wapp->dpp->max_remain_on_chan;
	if (wait_time > 2000)
		wait_time = 2000;
	wpa_printf(MSG_INFO1, DPP_EVENT_TX "dst=" MACSTR " chan=%u type=%d",
		MAC2STR(bssid), chan, DPP_PA_PEER_DISCOVERY_REQ);
	wapp_drv_send_action(wapp, wdev, chan, 0, (u8 *)bssid,
				wpabuf_head(msg), wpabuf_len(msg));
	wpabuf_free(msg);

	return 1;
}

int wapp_handle_dpp_scan(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	struct bss_info_scan_result *scan_result, *scan_result_tmp;
	struct dpp_config *wdev_config = wdev->config;

	if (!wdev->connection_tries) {
		DBGPRINT(RT_DEBUG_ERROR, "dpp connection is not ongoing\n");
		return -1;
	}

	if (!wdev_config) {
		DBGPRINT(RT_DEBUG_ERROR, "wdev config not found\n");
		return -1;
	}

	/* parse scan results */
	dl_list_for_each_safe(scan_result, scan_result_tmp, &(wapp->scan_results_list),
								struct bss_info_scan_result, list) {

		if (os_memcmp(scan_result->bss.Ssid, wdev_config->ssid, scan_result->bss.SsidLen) == 0) {
			/* Found our match */
			/* TODO match rsn */
			/* TODO Return best out of all bss found */
			/*do not connect if wildcard ssid comes in results */
			if(scan_result->bss.SsidLen == 0)
				continue;
			wapp_dpp_check_connect(wapp, wdev, (char *)scan_result->bss.Bssid, scan_result->bss.Channel);
			return 0;
		}
	}
	/* if AP not found try again */
	if (wdev->connection_tries < wapp->dpp->dpp_max_connection_tries)
		return wapp_wdev_dpp_scan(wapp,wdev);
	else
		DBGPRINT(RT_DEBUG_ERROR, "AP not found\n");

	return -1;
}

int dpp_set_conf_akm(struct dpp_configuration *conf, char *token)
{
	if (os_strcmp(token, "psk") == 0)
		conf->akm = DPP_AKM_PSK;
	else if (os_strcmp(token, "sae") == 0)
		conf->akm = DPP_AKM_SAE;
	else if ((os_strcmp(token, "psk-sae") == 0)||
		 (os_strcmp(token, "psk+sae") == 0))
		conf->akm = DPP_AKM_PSK_SAE;
	else if ((os_strcmp(token, "sae-dpp") == 0) ||
		 (os_strcmp(token, "dpp+sae") == 0))
		conf->akm = DPP_AKM_SAE_DPP;
	else if ((os_strcmp(token, "psk-sae-dpp") == 0) ||
		 (os_strcmp(token, "dpp+psk+sae") == 0))
		conf->akm = DPP_AKM_PSK_SAE_DPP;
	else if (os_strcmp(token, "dpp") == 0)
		conf->akm = DPP_AKM_DPP;
	else {
		DBGPRINT(RT_DEBUG_ERROR, "failed to find correct akm\n");
		return -1;
	}

	return 0;
}

int dpp_read_config_file(struct dpp_global *dpp)
{
	FILE *file;
	char buf[512], *pos, *token;
	char tmpbuf[512];
	int line = 0;
	struct wifi_app *wapp = (struct wifi_app *)dpp->msg_ctx;
	int radio_count = wapp_get_valid_radio_count(wapp);

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!dpp) {
		DBGPRINT(RT_DEBUG_ERROR, "\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);
		return WAPP_INVALID_ARG;
	}

	if (!wapp) {
                DBGPRINT(RT_DEBUG_ERROR, "\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);
                return WAPP_INVALID_ARG;
        }

	file = fopen(DPP_CFG_FILE, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open DPP cfg file (%s) fail\n", DPP_CFG_FILE);
		return WAPP_NOT_INITIALIZED;
	}

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);

	while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_strcpy(tmpbuf, pos);
		token = strtok(pos, "=");

		/* TODO initialize basic parameters */
		os_strcpy(dpp->curve_name, "prime256v1");
		if (token != NULL) {
			if (os_strcmp(token, "dpp_private_key") == 0) {
				token = strtok(NULL, "");
				os_strcpy(dpp->dpp_private_key, token);
			}  else if (os_strcmp(token, "configurator_support") == 0) {
				token = strtok(NULL, "");
				dpp->dpp_configurator_supported = atoi(token);
			/* 1 enrollee, 2 configurator, 3 both */
			}  else if (os_strcmp(token, "allowed_role") == 0) {
				token = strtok(NULL, "");
				dpp->dpp_allowed_roles = atoi(token);
			}  else if (os_strcmp(token, "dpp_interface_2g") == 0) {
				token = strtok(NULL, "");
				dpp->default_2g_iface = wapp_dev_list_lookup_by_ifname(wapp, token);
				if(dpp->default_2g_iface == NULL)
                                        continue;
				DBGPRINT(RT_DEBUG_OFF, "2g interface is: %s\n", dpp->default_2g_iface->ifname);
			}  else if (os_strcmp(token, "dpp_interface_5gl") == 0) {
				token = strtok(NULL, "");
				dpp->default_5gl_iface = wapp_dev_list_lookup_by_ifname(wapp, token);
				/* condition for DBDC */
				if(radio_count == MAX_RADIO_DBDC){
					dpp->default_5gh_iface = wapp_dev_list_lookup_by_ifname(wapp, token);
					if(dpp->default_5gh_iface == NULL)
						continue;
				}
				if(dpp->default_5gl_iface == NULL)
                                        continue;
				DBGPRINT(RT_DEBUG_OFF, "5gl interface is: %s\n", dpp->default_5gl_iface->ifname);
			}  else if ((os_strcmp(token, "dpp_interface_5gh") == 0) && (radio_count == MAP_MAX_RADIO)) {
				token = strtok(NULL, "");
				dpp->default_5gh_iface = wapp_dev_list_lookup_by_ifname(wapp, token);
				if(dpp->default_5gh_iface == NULL)
                                        continue;
				DBGPRINT(RT_DEBUG_OFF, "5gh interface is: %s\n", dpp->default_5gh_iface->ifname);
			}  else if (os_strcmp(token, "curve_name") == 0) {
				token = strtok(NULL, "");
				os_strcpy(dpp->curve_name, token);
			}  else if (os_strcmp(token, "map_support") == 0) {
				token = strtok(NULL, "");
				dpp->is_map = atoi(token);
			}  else if (os_strcmp(token, "max_conn_retries") == 0) {
				token = strtok(NULL, "");
				dpp->dpp_max_connection_tries = atoi(token);
			}  else if (os_strcmp(token, "ap_config") == 0) {
				struct dpp_configuration *conf;
				conf = os_zalloc(sizeof(*conf));
				if (!conf)
					return -1;
				dpp->conf_ap = conf;
			// TODO move inside of this if block
			}  else if (os_strcmp(token, "ap_ssid") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				os_strcpy((char *)conf->ssid, token);
				conf->ssid_len = os_strlen(token);
			}  else if (os_strcmp(token, "ap_pass") == 0) {
				struct dpp_configuration *conf;
				size_t pass_len;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				pass_len = os_strlen(token);
				conf->passphrase = os_zalloc(pass_len + 1);
				os_strcpy((char *)conf->passphrase, token);
			}  else if (os_strcmp(token, "ap_group_id") == 0) {
				struct dpp_configuration *conf;
				size_t group_id_len;
				token = strtok(NULL, "");
				conf = dpp->conf_ap;
				group_id_len = os_strlen(token);
				conf->group_id = os_zalloc(group_id_len + 1);
				os_memcpy(conf->group_id, token, group_id_len);
			}  else if (os_strcmp(token, "ap_expiry") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				conf->netaccesskey_expiry = atoi(token);
			}  else if (os_strcmp(token, "ap_akm") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_ap;
				token = strtok(NULL, "");
				dpp_set_conf_akm(conf, token);
			}  else if (os_strcmp(token, "sta_config") == 0) {
				struct dpp_configuration *conf;
				conf = os_zalloc(sizeof(*conf));
				if (!conf)
					return -1;
				dpp->conf_sta = conf;
			}  else if (os_strcmp(token, "sta_ssid") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				os_strcpy((char *)conf->ssid, token);
				conf->ssid_len = os_strlen(token);
			}  else if (os_strcmp(token, "sta_pass") == 0) {
				struct dpp_configuration *conf;
				size_t pass_len;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				pass_len = os_strlen(token);
				conf->passphrase = os_zalloc(pass_len + 1);
				os_strcpy((char *)conf->passphrase, token);
			}  else if (os_strcmp(token, "sta_group_id") == 0) {
				struct dpp_configuration *conf;
				size_t group_id_len;
				token = strtok(NULL, "");
				conf = dpp->conf_sta;
				group_id_len = os_strlen(token);
				conf->group_id = os_zalloc(group_id_len + 1);
				os_memcpy(conf->group_id, token, group_id_len);
			}  else if (os_strcmp(token, "sta_expiry") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				conf->netaccesskey_expiry = atoi(token);
			}  else if (os_strcmp(token, "sta_akm") == 0) {
				struct dpp_configuration *conf;
				conf = dpp->conf_sta;
				token = strtok(NULL, "");
				dpp_set_conf_akm(conf, token);
			}
		}
	}
	fclose(file);
	return WAPP_SUCCESS;
}

int wapp_dpp_init(struct wifi_app *wapp)
{
	struct dpp_global_config config;

	os_memset(&config, 0, sizeof(config));
	config.msg_ctx = wapp;
	config.cb_ctx = wapp;
#ifdef CONFIG_DPP2
	config.process_conf_obj = wapp_dpp_process_conf_obj;
#endif /* CONFIG_DPP2 */
	wapp->dpp = dpp_global_init(&config);
	wapp->dpp->gas_query_ctx = gas_query_init(wapp);
	wapp->dpp->dpp_allowed_roles = DPP_CAPAB_CONFIGURATOR | DPP_CAPAB_ENROLLEE;
	wapp->dpp->dpp_init_done = 1;
	wapp->dpp->max_remain_on_chan = 2000;
	dl_list_init(&wapp->dpp->dpp_txstatus_pending_list);
	wapp->dpp->dpp_frame_seq_no = 1011; //TF
	wapp->dpp->default_5gh_iface = wapp_dev_list_lookup_by_ifname(wapp,DEFAULT_5GH_IFACE);
	wapp->dpp->default_5gl_iface = wapp_dev_list_lookup_by_ifname(wapp,DEFAULT_5GL_IFACE);
	wapp->dpp->default_2g_iface = wapp_dev_list_lookup_by_ifname(wapp,DEFAULT_2G_IFACE);
	wapp->dpp->dpp_max_connection_tries = MAX_CONN_TRIES;
 
	return wapp->dpp ? 0 : -1;
}

void wapp_dpp_deinit(struct wifi_app *wapp)
{
	if (!wapp->dpp)
		return;

	dpp_global_clear(wapp->dpp);
	eloop_cancel_timeout(wapp_dpp_pkex_retry_timeout, wapp, NULL);
#ifdef CONFIG_DPP2
	//dpp_pfs_free(wapp->dpp->dpp_pfs);
	//wapp->dpp->dpp_pfs = NULL;
#endif /* CONFIG_DPP2 */
	wapp_dpp_stop(wapp);
	wapp_dpp_pkex_remove(wapp, "*");
	os_free(wapp->dpp->dpp_configurator_params);
	wapp->dpp->dpp_configurator_params = NULL;
}

static int dpp_save_config(struct wifi_app *wapp, const char *param, const char *value, char *ifname)
{
#ifdef OPENWRT_SUPPORT
	struct kvc_context *dat_ctx = NULL;
	char *ifparam;
	int ret = 0;

	os_alloc_mem(NULL, (UCHAR**)&ifparam, IFNAMSIZ + os_strlen(param));
	NdisZeroMemory(ifparam, IFNAMSIZ + os_strlen(param));
	if(ifparam == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "%s, mem alloc fail \n", __func__);
		goto out;
	}
	strcat(ifparam, ifname);
	strcat(ifparam, param);
	dat_ctx = dat_load(DPP_CFG_FILE);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", DPP_CFG_FILE);
		ret = -1;
		goto out;
	}
	ret = kvc_set(dat_ctx, (const char *)ifparam, (const char *)value);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "set param(%s) fail\n", param);
		goto out;
	}
	ret = kvc_commit(dat_ctx);
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "write param(%s) fail\n", param);
		goto out;
	}
	os_free_mem(NULL, ifparam);
out:
	if (dat_ctx)
		kvc_unload(dat_ctx);
	if (ret)
		return -1;
	else
#endif /* OPENWRT_SUPPORT */
		return 0;
}

void dpp_save_config_to_file(struct wifi_app *wapp, struct dpp_authentication *auth)
{
	char *strbuf;

	os_alloc_mem(NULL, (UCHAR**)&strbuf, 512);
	NdisZeroMemory(strbuf, 512);
	dpp_save_config(wapp, "_ssid", os_ssid_txt(auth->ssid, auth->ssid_len), auth->wdev->ifname);
	sprintf(strbuf, "%i", auth->akm);
	dpp_save_config(wapp, "_akm", strbuf, auth->wdev->ifname);
	NdisZeroMemory(strbuf, 512);

	switch (auth->akm) {
	case DPP_AKM_DPP:
	case DPP_AKM_SAE_DPP:
		dpp_save_config(wapp, "_connector", auth->connector, auth->wdev->ifname);

		os_snprintf_hex(strbuf, 512, auth->net_access_key->buf, auth->net_access_key->used);
		dpp_save_config(wapp, "_netAccessKey", strbuf, auth->wdev->ifname);
		NdisZeroMemory(strbuf, 512);

		os_snprintf_hex(strbuf, 512, auth->c_sign_key->buf, auth->c_sign_key->used);
		dpp_save_config(wapp, "_cSignKey", strbuf, auth->wdev->ifname);
		NdisZeroMemory(strbuf, 512);
		break;
	case DPP_AKM_PSK:
	case DPP_AKM_PSK_SAE:
	case DPP_AKM_PSK_SAE_DPP:
		dpp_save_config(wapp, "_passPhrase", auth->passphrase, auth->wdev->ifname);
		break;
	case DPP_AKM_UNKNOWN:
	case DPP_AKM_SAE:
		break;
	}
	os_free_mem(NULL, strbuf);
	return;
}
#ifdef OPENWRT_SUPPORT
static void dpp_fetch_dpp_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth,
				    struct kvc_context *dat_ctx)
{
	char ifparam[50];
	const char *buf;
	u8 buftemp[512];
	char cmd[100];

	auth->akm = DPP_AKM_DPP;
	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_connector");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	auth->connector = os_strdup(buf);

	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_netAccessKey");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	NdisZeroMemory(buftemp, 512);
	hexstr2bin(buf, buftemp, os_strlen(buf));
	auth->net_access_key = wpabuf_alloc(os_strlen(buf));
	NdisCopyMemory(auth->net_access_key->buf, buftemp, os_strlen(buf));
	auth->net_access_key->used = os_strlen(buf);

	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_cSignKey");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	NdisZeroMemory(buftemp, 512);
	hexstr2bin(buf, buftemp, os_strlen(buf));
	auth->c_sign_key = wpabuf_alloc(os_strlen(buf));
	NdisCopyMemory(auth->c_sign_key->buf, buftemp, os_strlen(buf));
	auth->c_sign_key->used = os_strlen(buf);

	os_memset(cmd, 0, 100);
	sprintf(cmd, "iwpriv %s set DppEnable=1;",
			wdev->ifname);
	system(cmd);
	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);

}

static void dpp_fetch_psk_akm_param(struct wapp_dev *wdev, struct dpp_authentication *auth,
				    struct kvc_context *dat_ctx)
{
	char ifparam[50];
	const char *buf;

	auth->akm = DPP_AKM_PSK;
	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_passPhrase");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	os_strcpy(auth->passphrase, buf);
}
#endif

void dpp_conf_init(struct wifi_app *wapp, wapp_dev_info *dev_info)
{
#ifdef OPENWRT_SUPPORT
	struct wapp_dev *wdev = NULL;
	struct kvc_context *dat_ctx = NULL;
	struct dpp_authentication *auth = NULL;
	char ifparam[50];
	const char *buf;
	int saved_akm;

	os_alloc_mem(NULL, (UCHAR**)&auth, sizeof(*auth));
	if (!auth)
		goto out;
	NdisZeroMemory(auth, sizeof(*auth));

	dat_ctx = dat_load(DPP_CFG_FILE);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", DPP_CFG_FILE);
		goto out;
	}

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, dev_info->ifindex);
	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_ssid");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	/* if ssid for that wdev NULL assume not configured */
	if (buf == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "saved config not found for %s\n", wdev->ifname);
		goto out;
	}
	DBGPRINT(RT_DEBUG_ERROR, "saved config found for %s\n", wdev->ifname);
	os_strlcpy((char*)auth->ssid, buf, os_strlen(buf) + 1);
	auth->ssid_len = os_strlen(buf) + 1;
	NdisZeroMemory(ifparam, 50);
	strcat(ifparam, wdev->ifname);
	strcat(ifparam, "_akm");
	buf = kvc_get(dat_ctx, (const char *)ifparam);
	saved_akm = buf[0] - '0';

	switch(saved_akm) {
		case DPP_AKM_DPP:
		case DPP_AKM_SAE_DPP:
			dpp_fetch_dpp_akm_param(wdev, auth, dat_ctx);
			break;
		case DPP_AKM_PSK:
		case DPP_AKM_PSK_SAE:
		case DPP_AKM_PSK_SAE_DPP:
			dpp_fetch_psk_akm_param(wdev, auth, dat_ctx);
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, "%s, DPP AKM specified in saved config not supported %s dpp init fail\n",
				__func__, wdev->ifname);
			goto out;
	}
	auth->wdev = wdev;
	wapp_dpp_process_config(wapp,auth);
out:
	if (auth)
		os_free_mem(NULL, auth);
	if (dat_ctx)
		kvc_unload(dat_ctx);
#endif /* OPENWRT_SUPPORT */
		return;
}
