/*
 * Testing tool for EAPOL-Key Supplicant/Authenticator routines
 * Copyright (c) 2006-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "utils/eloop.h"
#include "rsn_supp/wpa.h"
#include "ap/wpa_auth.h"

struct wpa {
	enum { AUTH, SUPP } test_peer;
	enum { READ, WRITE } test_oper;
	FILE *f;
	int wpa1;

	u8 auth_addr[ETH_ALEN];
	u8 supp_addr[ETH_ALEN];
	u8 psk[PMK_LEN];

	/* from authenticator */
	u8 *auth_eapol;
	size_t auth_eapol_len;

	/* from supplicant */
	u8 *supp_eapol;
	size_t supp_eapol_len;

	struct wpa_sm *supp;
	struct wpa_authenticator *auth_group;
	struct wpa_state_machine *auth;

	u8 supp_ie[80];
	size_t supp_ie_len;

	int key_request_done;
	int key_request_done1;
	int auth_sent;
};

const struct wpa_driver_ops *const wpa_drivers[] = { NULL };

static int auth_read_msg(struct wpa *wpa);
static void supp_eapol_key_request(void *eloop_data, void *user_ctx);

static void usage(void) {
	wpa_printf(MSG_INFO,
		   "usage: test-eapol <auth/supp> <read/write> <file>");
	exit(-1);
}

static void write_msg(FILE *f, const u8 *msg, size_t msg_len)
{
	u8 len[2];

	wpa_printf(MSG_DEBUG, "TEST: Write message to file (msg_len=%u)",
		   (unsigned int) msg_len);
	WPA_PUT_BE16(len, msg_len);
	fwrite(len, 2, 1, f);
	fwrite(msg, msg_len, 1, f);
}

static u8 * read_msg(FILE *f, size_t *ret_len)
{
	u8 len[2];
	u16 msg_len;
	u8 *msg;

	if (fread(len, 2, 1, f) != 1) {
		wpa_printf(MSG_ERROR, "TEST-ERROR: Could not read msg len");
		eloop_terminate();
		return NULL;
	}
	msg_len = WPA_GET_BE16(len);

	msg = os_malloc(msg_len);
	if (!msg)
		return NULL;
	if (msg_len > 0 && fread(msg, msg_len, 1, f) != 1) {
		wpa_printf(MSG_ERROR, "TEST-ERROR: Truncated msg (msg_len=%u)",
			   msg_len);
		os_free(msg);
		eloop_terminate();
		return NULL;
	}
	wpa_hexdump(MSG_DEBUG, "TEST: Read message from file", msg, msg_len);

	*ret_len = msg_len;
	return msg;
}

static int supp_get_bssid(void *ctx, u8 *bssid)
{
	struct wpa *wpa = ctx;
	wpa_printf(MSG_DEBUG, "SUPP: %s", __func__);
	os_memcpy(bssid, wpa->auth_addr, ETH_ALEN);
	return 0;
}

static void supp_set_state(void *ctx, enum wpa_states state)
{
	wpa_printf(MSG_DEBUG, "SUPP: %s(state=%d)", __func__, state);
}

static void auth_eapol_rx(void *eloop_data, void *user_ctx)
{
	struct wpa *wpa = eloop_data;

	wpa_printf(MSG_DEBUG, "AUTH: RX EAPOL frame");
	wpa->auth_sent = 0;
	wpa_receive(wpa->auth_group, wpa->auth, wpa->supp_eapol,
		    wpa->supp_eapol_len);
	if (!wpa->auth_sent && wpa->test_peer == SUPP &&
	    wpa->test_oper == READ) {
		/* Speed up process by not going through retransmit timeout */
		wpa_printf(MSG_DEBUG,
			   "AUTH: No response was sent - process next message");
		auth_read_msg(wpa);
	}
	if (wpa->wpa1 && wpa->key_request_done && !wpa->key_request_done1) {
		wpa->key_request_done1 = 1;
		eloop_register_timeout(0, 0, supp_eapol_key_request,
				       wpa, NULL);
	}

}

static void supp_eapol_rx(void *eloop_data, void *user_ctx)
{
	struct wpa *wpa = eloop_data;

	wpa_printf(MSG_DEBUG, "SUPP: RX EAPOL frame");
	wpa_sm_rx_eapol(wpa->supp, wpa->auth_addr, wpa->auth_eapol,
			wpa->auth_eapol_len);
}

static int supp_read_msg(struct wpa *wpa)
{
	os_free(wpa->auth_eapol);
	wpa->auth_eapol = read_msg(wpa->f, &wpa->auth_eapol_len);
	if (!wpa->auth_eapol)
		return -1;
	eloop_register_timeout(0, 0, supp_eapol_rx, wpa, NULL);
	return 0;
}

static int supp_ether_send(void *ctx, const u8 *dest, u16 proto, const u8 *buf,
			   size_t len)
{
	struct wpa *wpa = ctx;

	wpa_printf(MSG_DEBUG, "SUPP: %s(dest=" MACSTR " proto=0x%04x "
		   "len=%lu)",
		   __func__, MAC2STR(dest), proto, (unsigned long) len);

	if (wpa->test_peer == SUPP && wpa->test_oper == WRITE)
		write_msg(wpa->f, buf, len);

	if (wpa->test_peer == AUTH && wpa->test_oper == READ)
		return supp_read_msg(wpa);

	os_free(wpa->supp_eapol);
	wpa->supp_eapol = os_malloc(len);
	if (!wpa->supp_eapol)
		return -1;
	os_memcpy(wpa->supp_eapol, buf, len);
	wpa->supp_eapol_len = len;
	eloop_register_timeout(0, 0, auth_eapol_rx, wpa, NULL);

	return 0;
}

static u8 * supp_alloc_eapol(void *ctx, u8 type, const void *data,
			     u16 data_len, size_t *msg_len, void **data_pos)
{
	struct ieee802_1x_hdr *hdr;

	wpa_printf(MSG_DEBUG, "SUPP: %s(type=%d data_len=%d)",
		   __func__, type, data_len);

	*msg_len = sizeof(*hdr) + data_len;
	hdr = os_malloc(*msg_len);
	if (hdr == NULL)
		return NULL;

	hdr->version = 2;
	hdr->type = type;
	hdr->length = host_to_be16(data_len);

	if (data)
		os_memcpy(hdr + 1, data, data_len);
	else
		os_memset(hdr + 1, 0, data_len);

	if (data_pos)
		*data_pos = hdr + 1;

	return (u8 *) hdr;
}

static int supp_get_beacon_ie(void *ctx)
{
	struct wpa *wpa = ctx;
	const u8 *ie;
	size_t ielen;

	wpa_printf(MSG_DEBUG, "SUPP: %s", __func__);

	ie = wpa_auth_get_wpa_ie(wpa->auth_group, &ielen);
	if (ie == NULL || ielen < 1)
		return -1;
	if (ie[0] == WLAN_EID_RSN)
		return wpa_sm_set_ap_rsn_ie(wpa->supp, ie, 2 + ie[1]);
	return wpa_sm_set_ap_wpa_ie(wpa->supp, ie, 2 + ie[1]);
}

static int supp_set_key(void *ctx, enum wpa_alg alg,
			const u8 *addr, int key_idx, int set_tx,
			const u8 *seq, size_t seq_len,
			const u8 *key, size_t key_len)
{
	wpa_printf(MSG_DEBUG, "SUPP: %s(alg=%d addr=" MACSTR " key_idx=%d "
		   "set_tx=%d)",
		   __func__, alg, MAC2STR(addr), key_idx, set_tx);
	wpa_hexdump(MSG_DEBUG, "SUPP: set_key - seq", seq, seq_len);
	wpa_hexdump(MSG_DEBUG, "SUPP: set_key - key", key, key_len);
	return 0;
}

static int supp_mlme_setprotection(void *ctx, const u8 *addr,
				   int protection_type, int key_type)
{
	wpa_printf(MSG_DEBUG, "SUPP: %s(addr=" MACSTR " protection_type=%d "
		   "key_type=%d)",
		   __func__, MAC2STR(addr), protection_type, key_type);
	return 0;
}

static void supp_cancel_auth_timeout(void *ctx)
{
	wpa_printf(MSG_DEBUG, "SUPP: %s", __func__);
}

static void * supp_get_network_ctx(void *ctx)
{
	return (void *) 1;
}

static void supp_deauthenticate(void *ctx, u16 reason_code)
{
	wpa_printf(MSG_DEBUG, "SUPP: %s(%d)", __func__, reason_code);
}

static enum wpa_states supp_get_state(void *ctx)
{
	return WPA_COMPLETED;
}

static int supp_init(struct wpa *wpa)
{
	struct wpa_sm_ctx *ctx = os_zalloc(sizeof(*ctx));

	if (!ctx)
		return -1;

	ctx->ctx = wpa;
	ctx->msg_ctx = wpa;
	ctx->set_state = supp_set_state;
	ctx->get_bssid = supp_get_bssid;
	ctx->ether_send = supp_ether_send;
	ctx->get_beacon_ie = supp_get_beacon_ie;
	ctx->alloc_eapol = supp_alloc_eapol;
	ctx->set_key = supp_set_key;
	ctx->mlme_setprotection = supp_mlme_setprotection;
	ctx->cancel_auth_timeout = supp_cancel_auth_timeout;
	ctx->get_network_ctx = supp_get_network_ctx;
	ctx->deauthenticate = supp_deauthenticate;
	ctx->get_state = supp_get_state;
	wpa->supp = wpa_sm_init(ctx);
	if (!wpa->supp) {
		wpa_printf(MSG_DEBUG, "SUPP: wpa_sm_init() failed");
		return -1;
	}

	wpa_sm_set_own_addr(wpa->supp, wpa->supp_addr);
	if (wpa->wpa1) {
		wpa_sm_set_param(wpa->supp, WPA_PARAM_RSN_ENABLED, 0);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_PROTO, WPA_PROTO_WPA);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_PAIRWISE,
				 WPA_CIPHER_TKIP);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_GROUP, WPA_CIPHER_TKIP);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_KEY_MGMT,
				 WPA_KEY_MGMT_PSK);
	} else {
		wpa_sm_set_param(wpa->supp, WPA_PARAM_RSN_ENABLED, 1);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_PROTO, WPA_PROTO_RSN);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_PAIRWISE,
				 WPA_CIPHER_CCMP);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_GROUP, WPA_CIPHER_CCMP);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_KEY_MGMT,
				 WPA_KEY_MGMT_PSK);
		wpa_sm_set_param(wpa->supp, WPA_PARAM_MFP,
				 MGMT_FRAME_PROTECTION_OPTIONAL);
	}
	wpa_sm_set_pmk(wpa->supp, wpa->psk, PMK_LEN, NULL, NULL);

	wpa->supp_ie_len = sizeof(wpa->supp_ie);
	if (wpa_sm_set_assoc_wpa_ie_default(wpa->supp, wpa->supp_ie,
					    &wpa->supp_ie_len) < 0) {
		wpa_printf(MSG_DEBUG, "SUPP: wpa_sm_set_assoc_wpa_ie_default()"
			   " failed");
		return -1;
	}

	wpa_sm_notify_assoc(wpa->supp, wpa->auth_addr);

	return 0;
}

static void auth_logger(void *ctx, const u8 *addr, logger_level level,
			const char *txt)
{
	if (addr)
		wpa_printf(MSG_DEBUG, "AUTH: " MACSTR " - %s",
			   MAC2STR(addr), txt);
	else
		wpa_printf(MSG_DEBUG, "AUTH: %s", txt);
}

static int auth_read_msg(struct wpa *wpa)
{
	os_free(wpa->supp_eapol);
	wpa->supp_eapol = read_msg(wpa->f, &wpa->supp_eapol_len);
	if (!wpa->supp_eapol)
		return -1;
	eloop_register_timeout(0, 0, auth_eapol_rx, wpa, NULL);
	return 0;
}

static int auth_send_eapol(void *ctx, const u8 *addr, const u8 *data,
			   size_t data_len, int encrypt)
{
	struct wpa *wpa = ctx;

	wpa_printf(MSG_DEBUG, "AUTH: %s(addr=" MACSTR " data_len=%lu "
		   "encrypt=%d)",
		   __func__, MAC2STR(addr), (unsigned long) data_len, encrypt);
	wpa->auth_sent = 1;

	if (wpa->test_peer == AUTH && wpa->test_oper == WRITE)
		write_msg(wpa->f, data, data_len);

	if (wpa->test_peer == SUPP && wpa->test_oper == READ)
		return auth_read_msg(wpa);

	os_free(wpa->auth_eapol);
	wpa->auth_eapol = os_malloc(data_len);
	if (!wpa->auth_eapol)
		return -1;
	os_memcpy(wpa->auth_eapol, data, data_len);
	wpa->auth_eapol_len = data_len;
	eloop_register_timeout(0, 0, supp_eapol_rx, wpa, NULL);

	return 0;
}

static const u8 * auth_get_psk(void *ctx, const u8 *addr,
			       const u8 *p2p_dev_addr, const u8 *prev_psk,
			       size_t *psk_len, int *vlan_id)
{
	struct wpa *wpa = ctx;

	wpa_printf(MSG_DEBUG, "AUTH: %s (addr=" MACSTR " prev_psk=%p)",
		   __func__, MAC2STR(addr), prev_psk);
	if (vlan_id)
		*vlan_id = 0;
	if (psk_len)
		*psk_len = PMK_LEN;
	if (prev_psk)
		return NULL;
	return wpa->psk;
}

static void supp_eapol_key_request(void *eloop_data, void *user_ctx)
{
	struct wpa *wpa = eloop_data;

	wpa_printf(MSG_DEBUG, "SUPP: EAPOL-Key Request trigger");
	if (wpa->test_peer == SUPP && wpa->test_oper == READ) {
		if (!eloop_is_timeout_registered(auth_eapol_rx, wpa, NULL))
			auth_read_msg(wpa);
	} else {
		wpa_sm_key_request(wpa->supp, 0, 1);
	}
}

static int auth_set_key(void *ctx, int vlan_id, enum wpa_alg alg,
			const u8 *addr, int idx, u8 *key,
			size_t key_len)
{
	struct wpa *wpa = ctx;

	wpa_printf(MSG_DEBUG, "AUTH: %s (vlan_id=%d alg=%d idx=%d key_len=%d)",
		   __func__, vlan_id, alg, idx, (int) key_len);
	if (addr)
		wpa_printf(MSG_DEBUG, "AUTH: addr=" MACSTR, MAC2STR(addr));

	if (alg != WPA_ALG_NONE && idx == 0 && key_len > 0 &&
	    !wpa->key_request_done) {
		wpa_printf(MSG_DEBUG, "Test EAPOL-Key Request");
		wpa->key_request_done = 1;
		if (!wpa->wpa1)
			eloop_register_timeout(0, 0, supp_eapol_key_request,
					       wpa, NULL);
	}

	return 0;
}

static int auth_init_group(struct wpa *wpa)
{
	struct wpa_auth_config conf;
	struct wpa_auth_callbacks cb;

	wpa_printf(MSG_DEBUG, "AUTH: Initializing group state machine");

	os_memset(&conf, 0, sizeof(conf));
	if (wpa->wpa1) {
		conf.wpa = 1;
		conf.wpa_key_mgmt = WPA_KEY_MGMT_PSK;
		conf.wpa_pairwise = WPA_CIPHER_TKIP;
		conf.wpa_group = WPA_CIPHER_TKIP;
	} else {
		conf.wpa = 2;
		conf.wpa_key_mgmt = WPA_KEY_MGMT_PSK;
		conf.wpa_pairwise = WPA_CIPHER_CCMP;
		conf.rsn_pairwise = WPA_CIPHER_CCMP;
		conf.wpa_group = WPA_CIPHER_CCMP;
		conf.ieee80211w = 2;
		conf.group_mgmt_cipher = WPA_CIPHER_AES_128_CMAC;
	}
	conf.eapol_version = 2;
	conf.wpa_group_update_count = 4;
	conf.wpa_pairwise_update_count = 4;

	os_memset(&cb, 0, sizeof(cb));
	cb.logger = auth_logger;
	cb.send_eapol = auth_send_eapol;
	cb.get_psk = auth_get_psk;
	cb.set_key = auth_set_key,

	wpa->auth_group = wpa_init(wpa->auth_addr, &conf, &cb, wpa);
	if (!wpa->auth_group) {
		wpa_printf(MSG_DEBUG, "AUTH: wpa_init() failed");
		return -1;
	}

	return 0;
}

static int auth_init(struct wpa *wpa)
{
	if (wpa->test_peer == AUTH && wpa->test_oper == READ)
		return supp_read_msg(wpa);

	wpa->auth = wpa_auth_sta_init(wpa->auth_group, wpa->supp_addr, NULL);
	if (!wpa->auth) {
		wpa_printf(MSG_DEBUG, "AUTH: wpa_auth_sta_init() failed");
		return -1;
	}

	if (wpa_validate_wpa_ie(wpa->auth_group, wpa->auth, 2412, wpa->supp_ie,
				wpa->supp_ie_len, NULL, 0, NULL, 0, NULL, 0) !=
	    WPA_IE_OK) {
		wpa_printf(MSG_DEBUG, "AUTH: wpa_validate_wpa_ie() failed");
		return -1;
	}

	wpa_auth_sm_event(wpa->auth, WPA_ASSOC);

	wpa_auth_sta_associated(wpa->auth_group, wpa->auth);

	return 0;
}

static void deinit(struct wpa *wpa)
{
	wpa_auth_sta_deinit(wpa->auth);
	wpa_sm_deinit(wpa->supp);
	wpa_deinit(wpa->auth_group);
	os_free(wpa->auth_eapol);
	wpa->auth_eapol = NULL;
	os_free(wpa->supp_eapol);
	wpa->supp_eapol = NULL;
}

int main(int argc, char *argv[])
{
	const char *file;
	int ret;
	struct wpa wpa;

	if (os_program_init())
		return -1;

	wpa_debug_level = 0;
	wpa_debug_show_keys = 1;
	os_memset(&wpa, 0, sizeof(wpa));

	if (argc < 4)
		usage();

	if (os_strcmp(argv[1], "auth") == 0) {
		wpa.test_peer = AUTH;
	} else if (os_strcmp(argv[1], "auth1") == 0) {
		wpa.test_peer = AUTH;
		wpa.wpa1 = 1;
	} else if (os_strcmp(argv[1], "supp") == 0) {
		wpa.test_peer = SUPP;
	} else if (os_strcmp(argv[1], "supp1") == 0) {
		wpa.test_peer = SUPP;
		wpa.wpa1 = 1;
	} else {
		usage();
	}

	if (os_strcmp(argv[2], "read") == 0)
		wpa.test_oper = READ;
	else if (os_strcmp(argv[2], "write") == 0)
		wpa.test_oper = WRITE;
	else
		usage();

	file = argv[3];

	wpa.f = fopen(file, wpa.test_oper == READ ? "r" : "w");
	if (!wpa.f)
		return -1;

	os_memset(wpa.auth_addr, 0x12, ETH_ALEN);
	os_memset(wpa.supp_addr, 0x32, ETH_ALEN);
	os_memset(wpa.psk, 0x44, PMK_LEN);

	if (eloop_init()) {
		wpa_printf(MSG_ERROR, "Failed to initialize event loop");
		goto fail;
	}

	if (auth_init_group(&wpa) < 0)
		goto fail;

	if (supp_init(&wpa) < 0)
		goto fail;

	if (auth_init(&wpa) < 0)
		goto fail;

	wpa_printf(MSG_DEBUG, "Starting eloop");
	eloop_run();
	wpa_printf(MSG_DEBUG, "eloop done");

	ret = 0;
fail:
	deinit(&wpa);
	fclose(wpa.f);

	eloop_destroy();

	os_program_deinit();

	return ret;
}
