/*
 * DPP functionality shared between hostapd and wpa_supplicant
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See BSD_LICENSE for more details.
 */

#ifndef DPP_H
#define DPP_H

#include "util.h"
#include <openssl/x509.h>

#include "list.h"
#include "crypto/sha256.h"
#include "wapp_cmm_type.h"

struct crypto_ecdh;
struct wapp_ip_addr;
struct dpp_global;

#define DPP_HDR_LEN (4 + 2) /* OUI, OUI Type, Crypto Suite, DPP frame type */
#define DPP_TCP_PORT 7871
#define MAX_SET_BSS_INFO_NUM 26

#define PMKID_LEN 16
#define PMK_LEN 32
#define PMK_LEN_MAX 64

#ifndef BIT
#define BIT(n)                          ((UINT32) 1 << (n))
#endif

#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_FT_PSK BIT(6)
#define WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define WPA_KEY_MGMT_SAE BIT(10)
#define WPA_KEY_MGMT_FT_SAE BIT(11)
#define WPA_KEY_MGMT_DPP BIT(23)


struct wapp_radio;
/**
 * enum mfp_options - Management frame protection (IEEE 802.11w) options
 */
enum mfp_options {
	NO_MGMT_FRAME_PROTECTION = 0,
	MGMT_FRAME_PROTECTION_OPTIONAL = 1,
	MGMT_FRAME_PROTECTION_REQUIRED = 2,
};


enum dpp_public_action_frame_type {
	DPP_PA_AUTHENTICATION_REQ = 0,
	DPP_PA_AUTHENTICATION_RESP = 1,
	DPP_PA_AUTHENTICATION_CONF = 2,
	DPP_PA_PEER_DISCOVERY_REQ = 5,
	DPP_PA_PEER_DISCOVERY_RESP = 6,
	DPP_PA_PKEX_EXCHANGE_REQ = 7,
	DPP_PA_PKEX_EXCHANGE_RESP = 8,
	DPP_PA_PKEX_COMMIT_REVEAL_REQ = 9,
	DPP_PA_PKEX_COMMIT_REVEAL_RESP = 10,
	DPP_PA_CONFIGURATION_RESULT = 11,
};

enum dpp_attribute_id {
	DPP_ATTR_STATUS = 0x1000,
	DPP_ATTR_I_BOOTSTRAP_KEY_HASH = 0x1001,
	DPP_ATTR_R_BOOTSTRAP_KEY_HASH = 0x1002,
	DPP_ATTR_I_PROTOCOL_KEY = 0x1003,
	DPP_ATTR_WRAPPED_DATA = 0x1004,
	DPP_ATTR_I_NONCE = 0x1005,
	DPP_ATTR_I_CAPABILITIES = 0x1006,
	DPP_ATTR_R_NONCE = 0x1007,
	DPP_ATTR_R_CAPABILITIES = 0x1008,
	DPP_ATTR_R_PROTOCOL_KEY = 0x1009,
	DPP_ATTR_I_AUTH_TAG = 0x100A,
	DPP_ATTR_R_AUTH_TAG = 0x100B,
	DPP_ATTR_CONFIG_OBJ = 0x100C,
	DPP_ATTR_CONNECTOR = 0x100D,
	DPP_ATTR_CONFIG_ATTR_OBJ = 0x100E,
	DPP_ATTR_BOOTSTRAP_KEY = 0x100F,
	DPP_ATTR_OWN_NET_NK_HASH = 0x1011,
	DPP_ATTR_FINITE_CYCLIC_GROUP = 0x1012,
	DPP_ATTR_ENCRYPTED_KEY = 0x1013,
	DPP_ATTR_ENROLLEE_NONCE = 0x1014,
	DPP_ATTR_CODE_IDENTIFIER = 0x1015,
	DPP_ATTR_TRANSACTION_ID = 0x1016,
	DPP_ATTR_BOOTSTRAP_INFO = 0x1017,
	DPP_ATTR_CHANNEL = 0x1018,
	DPP_ATTR_PROTOCOL_VERSION = 0x1019,
	DPP_ATTR_ENVELOPED_DATA = 0x101A,
};

enum dpp_status_error {
	DPP_STATUS_OK = 0,
	DPP_STATUS_NOT_COMPATIBLE = 1,
	DPP_STATUS_AUTH_FAILURE = 2,
	DPP_STATUS_UNWRAP_FAILURE = 3,
	DPP_STATUS_BAD_GROUP = 4,
	DPP_STATUS_CONFIGURE_FAILURE = 5,
	DPP_STATUS_RESPONSE_PENDING = 6,
	DPP_STATUS_INVALID_CONNECTOR = 7,
	DPP_STATUS_NO_MATCH = 8,
	DPP_STATUS_CONFIG_REJECTED = 9,
};

#define DPP_CAPAB_ENROLLEE BIT(0)
#define DPP_CAPAB_CONFIGURATOR BIT(1)
#define DPP_CAPAB_ROLE_MASK (BIT(0) | BIT(1))

#define DPP_BOOTSTRAP_MAX_FREQ 30
#define DPP_MAX_NONCE_LEN 32
#define DPP_MAX_HASH_LEN 64
#define DPP_MAX_SHARED_SECRET_LEN 66

struct wapp_bss_config {
        char *dpp_connector;
        struct wpabuf *dpp_netaccesskey;
        unsigned int dpp_netaccesskey_expiry;
        struct wpabuf *dpp_csign;
};

struct set_config_bss_info{
	unsigned char mac[ETH_ALEN];
	char oper_class[4];
	char ssid[33];
	unsigned short authmode;
	unsigned short encryptype;
	char key[65];
	unsigned char wfa_vendor_extension;
	unsigned char hidden_ssid;
	/* local */
	unsigned char operating_chan;
	unsigned char is_used;
};

struct wapp_ip_addr {
	int af; /* AF_INET / AF_INET6 */
	union {
		struct in_addr v4;
		u8 max_len[16];
	} u;
};

struct dpp_connection {
	struct dl_list list;
	struct dpp_controller *ctrl;
	struct dpp_relay_controller *relay;
	struct dpp_global *global;
	struct dpp_authentication *auth;
	int sock;
	u8 mac_addr[ETH_ALEN];
	unsigned int chan;
	u8 msg_len[4];
	size_t msg_len_octets;
	struct wpabuf *msg;
	struct wpabuf *msg_out;
	size_t msg_out_pos;
	u8 is_map_connection;
	unsigned int read_eloop:1;
	unsigned int write_eloop:1;
	unsigned int on_tcp_tx_complete_gas_done:1;
	unsigned int on_tcp_tx_complete_remove:1;
	unsigned int on_tcp_tx_complete_auth_ok:1;
};

/* Remote Controller */
struct dpp_relay_controller {
	struct dl_list list;
	struct dpp_global *global;
	u8 pkhash[SHA256_MAC_LEN];
	struct wapp_ip_addr ipaddr;
	void *cb_ctx;
	void (*tx)(void *ctx, const u8 *addr, unsigned int chan, const u8 *msg,
		   size_t len);
	void (*gas_resp_tx)(void *ctx, const u8 *addr, u8 dialog_token,
			    int prot, struct wpabuf *buf);
	struct dl_list conn; /* struct dpp_connection */
};

/* Local Controller */
struct dpp_controller {
	struct dpp_global *global;
	u8 allowed_roles;
	int qr_mutual;
	int sock;
	struct dl_list conn; /* struct dpp_connection */
	char *configurator_params;
};

struct dpp_global {
	void *msg_ctx;
	struct dl_list bootstrap; /* struct dpp_bootstrap_info */
	struct dl_list configurator; /* struct dpp_configurator */

        struct gas_query *gas_query_ctx;

	unsigned int off_channel_chan;

        int dpp_init_done;
	struct wapp_bss_config *conf;
	struct dl_list dpp_auth_list; /* struct dpp_authentication */
        unsigned int dpp_pending_listen_chan;
        unsigned int dpp_listen_chan;
        u8 dpp_allowed_roles;
        int dpp_qr_mutual;
        int dpp_in_response_listen;
        int dpp_gas_client;
        int dpp_gas_dialog_token;
        struct dpp_pkex *dpp_pkex;
        struct dpp_bootstrap_info *dpp_pkex_bi;
        char *dpp_pkex_code;
        char *dpp_pkex_identifier;
        char *dpp_pkex_auth_cmd;
        char *dpp_configurator_params;
        struct os_time dpp_last_init;  
        struct os_time dpp_init_iter_start;
        struct gas_server *gas_server;
        unsigned int dpp_init_max_tries;
        unsigned int dpp_init_retry_time;
        unsigned int dpp_resp_wait_time;
        unsigned int dpp_resp_max_tries;
        unsigned int dpp_resp_retry_time;
	unsigned long max_remain_on_chan;
	/* should be mapped to correct wdev */
	unsigned int is_map;
	struct set_config_bss_info bss_config[MAX_SET_BSS_INFO_NUM];
	UCHAR bss_config_num;
	char dpp_private_key[512];
	UCHAR dpp_configurator_supported;
	char curve_name[32];
	struct dpp_configuration *conf_ap;
	struct dpp_configuration *conf_sta;
	u16 dpp_frame_seq_no;
	struct dl_list dpp_txstatus_pending_list; /* struct dpp_tx_status */
	struct wapp_dev *default_5gh_iface;
	struct wapp_dev *default_5gl_iface;
	struct wapp_dev *default_2g_iface;
	u8 dpp_max_connection_tries;

#ifdef CONFIG_DPP2
	struct dl_list controllers; /* struct dpp_relay_controller */
	struct dpp_controller *controller;
	struct dl_list tcp_init; /* struct dpp_connection */
	void *cb_ctx;
	int (*process_conf_obj)(void *ctx, struct dpp_authentication *auth);
#endif /* CONFIG_DPP2 */
};

struct bss_info_scan_result {
	struct dl_list list;
	struct scan_bss_info bss;
};



struct dpp_curve_params {
	const char *name;
	size_t hash_len;
	size_t aes_siv_key_len;
	size_t nonce_len;
	size_t prime_len;
	const char *jwk_crv;
	u16 ike_group;
	const char *jws_alg;
};

enum dpp_bootstrap_type {
	DPP_BOOTSTRAP_QR_CODE,
	DPP_BOOTSTRAP_PKEX,
};

struct dpp_bootstrap_info {
	struct dl_list list;
	unsigned int id;
	enum dpp_bootstrap_type type;
	char *uri;
	u8 mac_addr[ETH_ALEN];
	char *info;
	unsigned int chan[DPP_BOOTSTRAP_MAX_FREQ];
	unsigned int num_chan;
	int own;
	EVP_PKEY *pubkey;
	u8 pubkey_hash[SHA256_MAC_LEN];
	const struct dpp_curve_params *curve;
	unsigned int pkex_t; /* number of failures before dpp_pkex
			      * instantiation */
};

#define PKEX_COUNTER_T_LIMIT 5

struct dpp_pkex {
	void *msg_ctx;
	unsigned int initiator:1;
	unsigned int exchange_done:1;
	unsigned int failed:1;
	struct dpp_bootstrap_info *own_bi;
	u8 own_mac[ETH_ALEN];
	u8 peer_mac[ETH_ALEN];
	char *identifier;
	char *code;
	EVP_PKEY *x;
	EVP_PKEY *y;
	u8 Mx[DPP_MAX_SHARED_SECRET_LEN];
	u8 Nx[DPP_MAX_SHARED_SECRET_LEN];
	u8 z[DPP_MAX_HASH_LEN];
	EVP_PKEY *peer_bootstrap_key;
	struct wpabuf *exchange_req;
	struct wpabuf *exchange_resp;
	unsigned int t; /* number of failures on code use */
	unsigned int exch_req_wait_time;
	unsigned int exch_req_tries;
	unsigned int chan;
	struct wapp_dev *wdev;
};

enum dpp_akm {
	DPP_AKM_UNKNOWN,
	DPP_AKM_DPP,
	DPP_AKM_PSK,
	DPP_AKM_SAE,
	DPP_AKM_PSK_SAE,
	DPP_AKM_SAE_DPP,
	DPP_AKM_PSK_SAE_DPP,
};

struct dpp_configuration {
	u8 ssid[32];
	size_t ssid_len;
	enum dpp_akm akm;

	/* For DPP configuration (connector) */
	os_time_t netaccesskey_expiry;

	/* TODO: groups */
	char *group_id;

	/* For legacy configuration */
	char *passphrase;
	u8 psk[32];
	int psk_set;
};

struct peer_radio_info {
	u8 identifier[6];
	u8 is_bh_sta_supported;
	u8 max_bss;
	u8 operating_chan;
};

enum dpp_auth_state {
	DPP_STATE_DEINIT,
	DPP_STATE_AUTH_RESP_WAITING,
	DPP_STATE_AUTH_CONF_WAITING,
	DPP_STATE_CONFIG_REQ_WAITING,
	DPP_STATE_CONFIG_RSP_WAITING,
	DPP_STATE_CONFIG_RESULT_WAITING,
	DPP_STATE_CONFIG_DONE,
};

struct dpp_tx_status {
	struct dl_list list;
	u8 dst[MAC_ADDR_LEN];
	struct wapp_dev *wdev;
	u16 seq_no;
	struct os_time sent_time;
	u8 is_gas_frame;
};

struct dpp_authentication {
	struct dl_list list;
	void *msg_ctx;
	u8 peer_version;
	const struct dpp_curve_params *curve;
	struct dpp_bootstrap_info *peer_bi;
	struct dpp_bootstrap_info *own_bi;
	struct dpp_bootstrap_info *tmp_own_bi;
	u8 waiting_pubkey_hash[SHA256_MAC_LEN];
	int response_pending;
	enum dpp_status_error auth_resp_status;
	enum dpp_status_error conf_resp_status;
	u8 peer_mac_addr[ETH_ALEN];
	u8 i_nonce[DPP_MAX_NONCE_LEN];
	u8 r_nonce[DPP_MAX_NONCE_LEN];
	u8 e_nonce[DPP_MAX_NONCE_LEN];
	u8 i_capab;
	u8 r_capab;
	EVP_PKEY *own_protocol_key;
	EVP_PKEY *peer_protocol_key;
	struct wpabuf *req_msg;
	struct wpabuf *resp_msg;
	/* Intersection of possible frequencies for initiating DPP
	 * Authentication exchange */
	unsigned int chan[DPP_BOOTSTRAP_MAX_FREQ];
	unsigned int num_chan, chan_idx;
	unsigned int curr_chan;
	unsigned int neg_chan;
	unsigned int num_chan_iters;
	size_t secret_len;
	u8 Mx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Mx_len;
	u8 Nx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Nx_len;
	u8 Lx[DPP_MAX_SHARED_SECRET_LEN];
	size_t Lx_len;
	u8 k1[DPP_MAX_HASH_LEN];
	u8 k2[DPP_MAX_HASH_LEN];
	u8 ke[DPP_MAX_HASH_LEN];
	int initiator;
	int waiting_auth_resp;
	int waiting_auth_conf;
	int auth_req_ack;
	unsigned int auth_resp_tries;
	u8 allowed_roles;
	int configurator;
	int remove_on_tx_status;
	int connect_on_tx_status;
	int waiting_conf_result;
	int auth_success;
	struct wpabuf *conf_req;
	const struct wpabuf *conf_resp; /* owned by GAS server */
	struct dpp_configuration *conf_ap;
	struct dpp_configuration *conf_sta;
	struct dpp_configurator *conf;
	char *connector; /* received signedConnector */
	u8 ssid[SSID_MAX_LEN];
	u8 ssid_len;
	char passphrase[64];
	u8 psk[PMK_LEN];
	int psk_set;
	enum dpp_akm akm;
	struct wpabuf *net_access_key;
	os_time_t net_access_key_expiry;
	struct wpabuf *c_sign_key;
	struct peer_radio_info radio[3];
	u8 bss_index;
	enum dpp_auth_state current_state;
	int dpp_auth_success_on_ack_rx;
	struct wapp_dev *wdev;
};

struct dpp_configurator {
	struct dl_list list;
	unsigned int id;
	int own;
	EVP_PKEY *csign;
	char *kid;
	const struct dpp_curve_params *curve;
};

struct dpp_introduction {
	u8 pmkid[PMKID_LEN];
	u8 pmk[PMK_LEN_MAX];
	size_t pmk_len;
};

struct dpp_relay_config {
	const struct wapp_ip_addr *ipaddr;
	const u8 *pkhash;

	void *cb_ctx;
	void (*tx)(void *ctx, const u8 *addr, unsigned int chan, const u8 *msg,
		   size_t len);
	void (*gas_resp_tx)(void *ctx, const u8 *addr, u8 dialog_token, int prot,
			    struct wpabuf *buf);
};

struct dpp_controller_config {
	const char *configurator_params;
	int tcp_port;
	u8 is_map_controller;
	u8 al_mac[6];
};

struct pmk_cache {
	u8 pmk;
	u8 pmkid;
	u8 bssid;
        int key_mgmt;
};

struct dpp_config {
        u8 *ssid;
        size_t ssid_len;

        enum mfp_options ieee80211w;
        int psk_set;
	u8 psk[32];
        int key_mgmt;

        char *dpp_connector;

        u8 *dpp_netaccesskey;
        size_t dpp_netaccesskey_len;
        unsigned int dpp_netaccesskey_expiry;

        u8 *dpp_csign;
        size_t dpp_csign_len;
	struct pmk_cache pmk_list[16];
};

void dpp_bootstrap_info_free(struct dpp_bootstrap_info *info);
const char * dpp_bootstrap_type_txt(enum dpp_bootstrap_type type);
int dpp_bootstrap_key_hash(struct dpp_bootstrap_info *bi);
int dpp_parse_uri_chan_list(struct dpp_bootstrap_info *bi,
			    const char *chan_list);
int dpp_parse_uri_mac(struct dpp_bootstrap_info *bi, const char *mac);
int dpp_parse_uri_info(struct dpp_bootstrap_info *bi, const char *info);
struct dpp_bootstrap_info * dpp_parse_qr_code(const char *uri);
char * dpp_keygen(struct dpp_bootstrap_info *bi, const char *curve,
		  const u8 *privkey, size_t privkey_len);
struct dpp_authentication * dpp_auth_init(void *msg_ctx,
					  struct wapp_dev *wdev,
					  struct dpp_bootstrap_info *peer_bi,
					  struct dpp_bootstrap_info *own_bi,
					  u8 dpp_allowed_roles,
					  unsigned int neg_chan);

struct dpp_authentication *
dpp_auth_req_rx(void *msg_ctx, u8 dpp_allowed_roles, int qr_mutual,
		struct dpp_bootstrap_info *peer_bi,
		struct dpp_bootstrap_info *own_bi,
		unsigned int chan, const u8 *hdr, const u8 *attr_start,
		size_t attr_len);
struct wpabuf *
dpp_auth_resp_rx(struct dpp_authentication *auth, const u8 *hdr,
		 const u8 *attr_start, size_t attr_len);
struct wpabuf * dpp_build_conf_req(struct dpp_authentication *auth,
				   const char *json);
int dpp_auth_conf_rx(struct dpp_authentication *auth, const u8 *hdr,
		     const u8 *attr_start, size_t attr_len);
int dpp_notify_new_qr_code(struct dpp_authentication *auth,
			   struct dpp_bootstrap_info *peer_bi);
struct dpp_configuration * dpp_configuration_alloc(const char *type);
int dpp_akm_psk(enum dpp_akm akm);
int dpp_akm_sae(enum dpp_akm akm);
int dpp_akm_legacy(enum dpp_akm akm);
int dpp_akm_dpp(enum dpp_akm akm);
int dpp_akm_ver2(enum dpp_akm akm);
int dpp_configuration_valid(const struct dpp_configuration *conf);
void dpp_configuration_free(struct dpp_configuration *conf);
int dpp_set_configurator(struct dpp_global *dpp, void *msg_ctx,
			 struct dpp_authentication *auth,
			 const char *cmd);
void dpp_auth_deinit(struct dpp_authentication *auth);
struct wpabuf *
dpp_conf_req_rx(struct dpp_authentication *auth, const u8 *attr_start,
		size_t attr_len);
int dpp_conf_resp_rx(struct dpp_authentication *auth,
		     const struct wpabuf *resp);
enum dpp_status_error dpp_conf_result_rx(struct dpp_authentication *auth,
					 const u8 *hdr,
					 const u8 *attr_start, size_t attr_len);
struct wpabuf * dpp_build_conf_result(struct dpp_authentication *auth,
				      enum dpp_status_error status);
struct wpabuf * dpp_alloc_msg(enum dpp_public_action_frame_type type,
			      size_t len);
const u8 * dpp_get_attr(const u8 *buf, size_t len, u16 req_id, u16 *ret_len);
int dpp_check_attrs(const u8 *buf, size_t len);
int dpp_key_expired(const char *timestamp, os_time_t *expiry);
const char * dpp_akm_str(enum dpp_akm akm);
int dpp_configurator_get_key(const struct dpp_configurator *conf, char *buf,
			     size_t buflen);
void dpp_configurator_free(struct dpp_configurator *conf);
struct dpp_configurator *
dpp_keygen_configurator(const char *curve, const u8 *privkey,
			size_t privkey_len);
int dpp_configurator_own_config(struct dpp_authentication *auth,
				const char *curve, int ap);
enum dpp_status_error
dpp_peer_intro(struct dpp_introduction *intro, const char *own_connector,
	       const u8 *net_access_key, size_t net_access_key_len,
	       const u8 *csign_key, size_t csign_key_len,
	       const u8 *peer_connector, size_t peer_connector_len,
	       os_time_t *expiry);
struct dpp_pkex * dpp_pkex_init(void *msg_ctx, struct dpp_bootstrap_info *bi,
				const u8 *own_mac,
				const char *identifier,
				const char *code);
struct dpp_pkex * dpp_pkex_rx_exchange_req(void *msg_ctx,
					   struct dpp_bootstrap_info *bi,
					   const u8 *own_mac,
					   const u8 *peer_mac,
					   const char *identifier,
					   const char *code,
					   const u8 *buf, size_t len);
struct wpabuf * dpp_pkex_rx_exchange_resp(struct dpp_pkex *pkex,
					  const u8 *peer_mac,
					  const u8 *buf, size_t len);
struct wpabuf * dpp_pkex_rx_commit_reveal_req(struct dpp_pkex *pkex,
					      const u8 *hdr,
					      const u8 *buf, size_t len);
int dpp_pkex_rx_commit_reveal_resp(struct dpp_pkex *pkex, const u8 *hdr,
				   const u8 *buf, size_t len);
void dpp_pkex_free(struct dpp_pkex *pkex);

char * dpp_corrupt_connector_signature(const char *connector);


struct dpp_pfs {
	struct crypto_ecdh *ecdh;
	const struct dpp_curve_params *curve;
	struct wpabuf *ie;
	struct wpabuf *secret;
};

struct dpp_pfs * dpp_pfs_init(const u8 *net_access_key,
			      size_t net_access_key_len);
int dpp_pfs_process(struct dpp_pfs *pfs, const u8 *peer_ie, size_t peer_ie_len);
void dpp_pfs_free(struct dpp_pfs *pfs);

struct dpp_bootstrap_info * dpp_add_qr_code(struct dpp_global *dpp,
					    const char *uri);
int dpp_bootstrap_gen_at_bootup(struct dpp_global *dpp, char *key, char *mac, char *chan);
int dpp_bootstrap_gen(struct dpp_global *dpp, const char *cmd);
struct dpp_bootstrap_info *
dpp_bootstrap_get_id(struct dpp_global *dpp, unsigned int id);
int dpp_bootstrap_remove(struct dpp_global *dpp, const char *id);
struct dpp_bootstrap_info *
dpp_pkex_finish(struct dpp_global *dpp, struct dpp_pkex *pkex, const u8 *peer,
		unsigned int chan);
const char * dpp_bootstrap_get_uri(struct dpp_global *dpp, unsigned int id);
int dpp_bootstrap_info(struct dpp_global *dpp, int id,
		       char *reply, int reply_size);
void dpp_bootstrap_find_pair(struct dpp_global *dpp, const u8 *i_bootstrap,
			     const u8 *r_bootstrap,
			     struct dpp_bootstrap_info **own_bi,
			     struct dpp_bootstrap_info **peer_bi);
int dpp_configurator_add(struct dpp_global *dpp, const char *cmd);
int dpp_configurator_remove(struct dpp_global *dpp, const char *id);
int dpp_configurator_get_key_id(struct dpp_global *dpp, unsigned int id,
				char *buf, size_t buflen);
int dpp_relay_add_controller(struct dpp_global *dpp,
			     struct dpp_relay_config *config);
int dpp_relay_rx_action(struct dpp_global *dpp, const u8 *src, const u8 *hdr,
			const u8 *buf, size_t len, unsigned int chan,
			const u8 *i_bootstrap, const u8 *r_bootstrap);
int dpp_relay_rx_gas_req(struct dpp_global *dpp, const u8 *src, const u8 *data,
			 size_t data_len);
int dpp_controller_start(struct dpp_global *dpp,
			 struct dpp_controller_config *config);
void dpp_controller_stop(struct dpp_global *dpp);
int dpp_tcp_init(struct dpp_global *dpp, struct dpp_authentication *auth,
		 const struct wapp_ip_addr *addr, int port);
int dpp_map_init(struct dpp_global *dpp, struct dpp_authentication *auth, u8 *alid);

struct dpp_global_config {
	void *msg_ctx;
	void *cb_ctx;
	int (*process_conf_obj)(void *ctx, struct dpp_authentication *auth);
};

struct dpp_global * dpp_global_init(struct dpp_global_config *config);
void dpp_global_clear(struct dpp_global *dpp);
void dpp_global_deinit(struct dpp_global *dpp);
void dpp_controller_process_rx(void *con, const u8 *pos, size_t len);
void map_process_dpp_packet(struct wifi_app *wapp, u8 *msg, int len);
int wapp_dpp_handle_config_obj(struct wifi_app *wapp,
					  struct dpp_authentication *auth);

int wapp_dpp_configurator_add(struct dpp_global *dpp);
void wdev_handle_dpp_action_frame(struct wifi_app *wapp,
	struct wapp_dev *wdev, struct wapp_dpp_action_frame *frame);
void wdev_handle_dpp_frm_tx_status(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
void wdev_get_dpp_action_frame(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void wapp_dpp_auth_list_remove(struct dpp_authentication *auth);
int wapp_drv_send_action(struct wifi_app *wapp, struct wapp_dev *wdev, unsigned int chan,
		unsigned int wait, const u8 *dst, const u8 *data,
		size_t len);
struct dpp_tx_status *wapp_dpp_get_status_info_from_sq(struct wifi_app *wapp, u16 seq_no);
#endif /* DPP_H */
