/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __WPS_H__
#define  __WPS_H__

/* Authentication types */
#define WSC_AUTHTYPE_OPEN        0x0001
#define WSC_AUTHTYPE_WPAPSK      0x0002
#define WSC_AUTHTYPE_SHARED      0x0004
#define WSC_AUTHTYPE_WPA         0x0008
#define WSC_AUTHTYPE_WPA2        0x0010
#define WSC_AUTHTYPE_WPA2PSK     0x0020
#ifdef MAP_SUPPORT		/*Add WPA3 support with MAP_R1*/ 
#define WSC_AUTHTYPE_SAE	 0x0040
#endif
#define WSC_AUTHTYPE_WPANONE     0x0080

/* Encryption type */
#define WSC_ENCRTYPE_NONE    0x0001
#define WSC_ENCRTYPE_WEP     0x0002
#define WSC_ENCRTYPE_TKIP    0x0004
#define WSC_ENCRTYPE_AES     0x0008

int wapp_trigger_wsc_pbc_exec(struct wifi_app *wapp,
	struct wapp_dev *wdev);
void *wps_ctrl_run_cli_wps(struct wifi_app *wapp,
	struct wapp_dev *wdev);
void wps_ctrl_run_ap_wps();
void wps_ctrl_process_scan_results(struct wifi_app *wapp);
char *WscGetAuthTypeStr(
	unsigned short authFlag);
char *WscGetEncryTypeStr(unsigned short encryFlag);
void stop_con_cli_wps(struct wifi_app *wapp, struct wapp_dev *wdev);
void wdev_process_wsc_scan_comp(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
void wdev_handle_wsc_eapol_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
void wdev_handle_wsc_eapol_end_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
void wdev_handle_scan_complete_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void map_get_scan_result(void *eloop_ctx, void *timeout_ctx);
void stop_con_ap_wps(struct wifi_app *wapp, struct wapp_dev *wdev);
void write_backhaul_configs(struct wifi_app *wapp, wsc_apcli_config_msg *bh_configs_msg);
void write_backhaul_configs_all(struct wifi_app *wapp,wsc_apcli_config_msg *bh_configs_msg, struct map_radio_identifier *ra_identifier);
void write_configs(struct wifi_app *wapp, wsc_apcli_config *apcli_config, int i, char *ra_match);
int driver_wext_get_set_uuid(void *drv_data,
	const char *ifname, char *uuid, BOOLEAN set);
int driver_wext_set_ssid(void *drv_data,
	const char *ifname, char *ssid);
int driver_wext_set_psk(void *drv_data,
	const char *ifname, char *psk);

void wapp_reset_scan_states(struct wifi_app *wapp);
void wapp_soft_reset_scan_states(struct wifi_app *wapp);
void map_config_state_check(void *eloop_data, void *user_ctx);
int driver_wext_get_wsc_profiles(void *drv_data, const char *ifname, char *wsc_profile_data,int *length);
//#ifdef MAP_R2
void wapp_reset_map_params(struct wifi_app *wapp, struct wapp_dev *wdev);
void wps_ctrl_run_ap_wps(struct wifi_app *wapp);

//#endif

#ifdef KV_API_SUPPORT
int driver_rrm_onoff(void *drv_data, const char *ifname, int onoff);
int driver_rrm_send_bcn_req_param(void *drv_data, const char *ifname, const char *bcn_req_param, u32 param_len);

int driver_wnm_onoff(void * drv_data, const char *ifname, u8 onoff);
int driver_wnm_btm_onoff(void * drv_data, const char *ifname, int onoff);
int driver_wnm_send_btm_req_param(void *drv_data, const char *ifname, const char *btm_req_param, u32 param_len);
int driver_wnm_send_btm_req_raw(void *drv_data, const char *ifname, const char *btm_req_raw, u32 param_len);
int wapp_send_btm_req_11kv_api(struct wifi_app *wapp, const char *ifname, const u8 *peer_mac_addr, const char *btm_req, size_t btm_req_len);
#endif /* KV_API_SUPPORT */
#endif
