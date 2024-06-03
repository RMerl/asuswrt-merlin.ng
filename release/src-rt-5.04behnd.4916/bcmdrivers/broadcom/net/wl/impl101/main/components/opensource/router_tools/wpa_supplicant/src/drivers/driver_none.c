/*
 * Driver interface for RADIUS server or WPS ER only (no driver)
 * Copyright (c) 2008, Atheros Communications
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "driver.h"

struct none_driver_data {
	struct hostapd_data *hapd;
	void *ctx;
};

static void * none_driver_hapd_init(struct hostapd_data *hapd,
				    struct wpa_init_params *params)
{
	struct none_driver_data *drv;

	drv = os_zalloc(sizeof(struct none_driver_data));
	if (drv == NULL) {
		wpa_printf(MSG_ERROR, "Could not allocate memory for none "
			   "driver data");
		return NULL;
	}
	drv->hapd = hapd;

	return drv;
}

static void none_driver_hapd_deinit(void *priv)
{
	struct none_driver_data *drv = priv;

	os_free(drv);
}

static void * none_driver_init(void *ctx, const char *ifname)
{
	struct none_driver_data *drv;

	drv = os_zalloc(sizeof(struct none_driver_data));
	if (drv == NULL) {
		wpa_printf(MSG_ERROR, "Could not allocate memory for none "
			   "driver data");
		return NULL;
	}
	drv->ctx = ctx;

	return drv;
}

static void none_driver_deinit(void *priv)
{
	struct none_driver_data *drv = priv;

	os_free(drv);
}

#ifdef CONFIG_DRIVER_BRCM_MAP
static int none_driver_wpa_get_ssid(void *priv, u8 *ssid)
{
	struct none_driver_data *drv = priv;

	return wpa_supplicant_driver_none_get_ssid(drv->ctx, ssid);
}

static int none_driver_wpa_get_bssid(void *priv, u8 *bssid)
{
	struct none_driver_data *drv = priv;

	return wpa_supplicant_driver_none_get_bssid(drv->ctx, bssid);;
}

static int none_driver_wpa_set_key(void *priv, struct wpa_driver_set_key_params *params)
{
	struct none_driver_data *drv = priv;

	wpa_printf(MSG_DEBUG, "%s: alg=%d addr=%p key_idx=%d "
		"set_tx=%d seq_len=%lu key_len=%lu key_flag=0x%x",
		__func__, params->alg, params->addr, params->key_idx,
		params->set_tx, (unsigned long) params->seq_len,
		(unsigned long) params->key_len, params->key_flag);

	if ((params->addr == NULL) || (params->key == NULL) || (params->key_len == 0)) {
		return 0;
	}

	if (params->key_flag & KEY_FLAG_PAIRWISE) {
		wpa_printf(MSG_INFO, "WPA: MAP PTK for MAP Device " MACSTR, MAC2STR(params->addr));
		return wpa_supplicant_driver_none_send_general_msg_on_ctrl_socket(drv->ctx,
			params->addr, "MAP-1905-PTK", params->key, params->key_len);
	} else if ((params->key_flag & KEY_FLAG_GROUP) && (params->key_idx < 2)) {
		wpa_printf(MSG_INFO, "WPA: MAP GTK");
		return wpa_supplicant_driver_none_send_general_msg_on_ctrl_socket(drv->ctx,
			params->addr, "MAP-1905-GTK", params->key, params->key_len);
	}

	return 0;
}

static int none_driver_wpa_send_action(void *priv, unsigned int freq,
	unsigned int wait, const u8 *dst, const u8 *src, const u8 *bssid,
	const u8 *data, size_t data_len, int no_cck)
{
	struct none_driver_data *drv = priv;

	return wpa_supplicant_driver_none_send_general_msg_on_ctrl_socket(drv->ctx, dst,
		"WPA-ACTION-FRAME", data, data_len);
}
#endif	/* CONFIG_DRIVER_BRCM_MAP */

const struct wpa_driver_ops wpa_driver_none_ops = {
	.name = "none",
	.desc = "no driver (RADIUS server/WPS ER)",
	.hapd_init = none_driver_hapd_init,
	.hapd_deinit = none_driver_hapd_deinit,
	.init = none_driver_init,
	.deinit = none_driver_deinit,
#ifdef CONFIG_DRIVER_BRCM_MAP
	.get_bssid = none_driver_wpa_get_bssid,
	.get_ssid = none_driver_wpa_get_ssid,
	.set_key = none_driver_wpa_set_key,
	.send_action = none_driver_wpa_send_action,
#endif	/* CONFIG_DRIVER_BRCM_MAP */
};
