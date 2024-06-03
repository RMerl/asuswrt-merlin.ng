/*
 * hostapd / DPP integration
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See BSD_LICENSE for more details.
 */

#ifndef DPP_WDEV_H
#define DPP_WDEV_H

#define DPP_AUTH_WAIT_TIMEOUT 2
void wapp_dpp_rx_action(struct wifi_app *wapp, struct wapp_dev *wdev, const u8 *src,
			const u8 *buf, size_t len, unsigned int chan);
void wapp_dpp_tx_status(struct wifi_app *wapp, const u8 *dst,
			   const u8 *data, size_t data_len, int ok);
struct wpabuf *
wapp_dpp_gas_req_handler(void *ctx, const u8 *sa,
			    const u8 *query, size_t query_len,
				u8 *data, size_t data_len);
int wapp_dpp_gas_req_relay_handler(struct wifi_app *wapp, const u8 *sa,
			    const u8 *query, size_t query_len, const u8 *data, size_t data_len);
void wapp_dpp_gas_status_handler(void *ctx, u8 *dst, int ok);
int wapp_dpp_configurator_remove(struct wifi_app *wapp, const char *id);
int wapp_dpp_configurator_get_key(struct wifi_app *wapp, unsigned int id,
				     char *buf, size_t buflen);
int wapp_dpp_pkex_add(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
int wapp_dpp_pkex_remove(struct wifi_app *wapp, const char *id);
void wapp_dpp_stop(struct wifi_app *wapp);
int wapp_dpp_gas_server_init(struct wifi_app *wapp);
void wapp_ap_dpp_deinit(struct wifi_app *wapp);
int wapp_dpp_controller_start(struct wifi_app *wapp, const char *cmd);

int wapp_dpp_qr_code(struct wifi_app *wapp, const char *cmd);
int wapp_dpp_auth_init(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
int wapp_dpp_listen(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
void wapp_dpp_listen_stop(struct wifi_app *wapp, struct wapp_dev *wdev);
int wapp_dpp_configurator_sign(struct wifi_app *wapp, struct wapp_dev *wdev, const char *cmd);
int wapp_dpp_init(struct wifi_app *wapp);
void wapp_dpp_deinit(struct wifi_app *wapp);
int dpp_read_config_file(struct dpp_global *dpp);
int wapp_handle_dpp_scan(struct wifi_app *wapp, struct wapp_dev *wdev);
void wapp_dpp_config_req_wait_timeout(void *eloop_ctx, void *timeout_ctx);
void dpp_save_config_to_file(struct wifi_app *wapp, struct dpp_authentication *auth);
void dpp_conf_init(struct wifi_app *wapp, wapp_dev_info *dev_info);
#endif /* DPP_WDEV_H */
