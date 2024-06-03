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

#ifndef _WDEV_H_
#define _WDEV_H_

#include <stdint.h>
//##include <if.h>
#include "types.h"
#include "list.h"
#include "wapp_cmm.h"
#ifdef MAP_R2
#include "off_ch_scan.h"
#endif
#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN	6
#endif

#define CLIENT_TABLE_SIZE 256
#define MAX_BEACON_REPORT_LEN 1024

struct wifi_app;

// TODO: alighn with driver
typedef enum {
	WAPP_DEV_TYPE_AP = 1,
	WAPP_DEV_TYPE_STA,
	WAPP_DEV_TYPE_APCLI = 128,
	WAPP_DEV_UNEXP,
} WAPP_DEV_TYPE;

struct sec_info {
	char auth[32];
	char encryp[32];
	char psphr[256];
};

struct wapp_dev {
	struct	dl_list list;
	const	struct wdev_ops *ops;
	struct sec_info sec;
	void	*p_dev;
	u32		ifindex;
	u8		mac_addr[MAC_ADDR_LEN];
	char	ifname[IFNAMSIZ];
	u8		dev_type;
	u8		wireless_mode; /* ex: a/b/g/n/ac */
	struct wapp_radio *radio;
#ifdef MAP_SUPPORT
	u8		i_am_fh_bss;   /* from map_vendor_extension in wsc_config */
	u8		i_am_bh_bss;
#ifdef HOSTAPD_MAP_SUPPORT	
	unsigned char bh_profile_id;
	BOOLEAN i_need_hostapd_reload;
#endif /* HOSTAPD_MAP_SUPPORT */
	struct os_time cli_list_last_update_time;
#endif /* MAP_SUPPORT */
	int scan_cookie;
	struct wapp_wsc_scan_info wsc_scan_info;
	BOOLEAN wps_triggered;
	unsigned char bh_connect_priority;
	int arp_sock;
	u8 valid;
#ifdef KV_API_SUPPORT
	BOOLEAN rrm_enable;
	BOOLEAN wnm_enbale;
#endif /* KV_API_SUPPORT */
#ifdef MAP_R2
	u32		cac_not_required;
	u8		cac_method;
	u8		synA;
	u8		synB;
#endif
	BOOLEAN wapp_triggered_scan;
#ifdef DPP_SUPPORT
       struct dpp_config *config;
       UCHAR connection_tries;
#endif /* CONFIG_DPP */
};

struct GNU_PACKED wapp_block_sta {
	u8 mac_addr[MAC_ADDR_LEN];
	u16 valid_period;
};

#define BLOCK_LIST_NUM      128

struct GNU_PACKED ap_dev {	
	/*struct wireless_client *client_table;*/
	u8				num_of_clients; //valid client number in client_table
	u16				num_of_assoc_cli;  //associated client number in client_table
	u8				max_num_of_cli;  // driver supported max number of client
	u8				max_num_of_bss;
	u8				num_of_bss;
	u8				max_num_of_block_cli;
	u8				num_of_block_cli;
	u8				rssi;	// from air monitor
	u8				isActive;
	wdev_ht_cap		ht_cap;
	wdev_vht_cap	vht_cap;
	wdev_bss_info	bss_info;
	wdev_chn_info	ch_info;
	wdev_op_class_info	op_class;
	wdev_ap_metric		ap_metrics;
	wdev_tx_power		pwr;
	wdev_steer_sta		str_sta;
	wapp_bssload_info	bssload;
	wapp_mnt_info 		mnt;
	struct wapp_sta		**client_table;
	struct wapp_block_sta	block_table[BLOCK_LIST_NUM];
	wdev_he_cap	he_cap;
#ifdef MAP_R2
	wdev_extended_ap_metric ext_ap_metrics;
#endif
#ifdef ACL_CTRL
	u8				num_of_acl_cli;
#endif
};

struct wdev_ops {
	int (*wdev_del)(struct wifi_app	*wapp, struct wapp_dev	*wdev);
};

struct wapp_dev* wapp_dev_list_lookup_by_mac_and_type(
	struct wifi_app *wapp, 
	const u8 *mac_addr, 
	const u8 wdev_type);


struct wapp_dev* wapp_dev_list_lookup_by_ifindex(
	struct wifi_app *wapp,
	const u32 ifindex);

struct wapp_dev* wapp_dev_list_lookup_by_ifname(
	struct wifi_app *wapp,
	const char *ifname);

struct wapp_dev* wapp_dev_list_lookup_by_radio(
	struct wifi_app *wapp,
	char* ra_identifier);

int wapp_dev_create(
	struct wifi_app *wapp,
	char 			*iface,
	u32				if_idx,
	u8				*mac_addr);

int wdev_ap_create(
	struct wifi_app *wapp,
	wapp_dev_info	*dev_info);

int wdev_sta_create(
	struct wifi_app *wapp,
	wapp_dev_info	*dev_info);

int wdev_ap_client_table_create(
	struct wifi_app *wapp,
	struct ap_dev	*ap);

int wdev_ap_client_table_release(
	struct wifi_app *wapp,
	struct ap_dev	*ap);

int wapp_client_create(
	struct wifi_app *wapp,
	struct ap_dev	*ap,
	struct wapp_sta **new_sta);

void wapp_fill_client_info(
	struct wifi_app *wapp,
	wapp_client_info *cli_info,
	struct wapp_sta *sta);

int wdev_show_wapp_sta_info(
	struct wapp_sta *cli);

struct wapp_sta*  wdev_ap_client_list_lookup_for_all_bss(
	struct wifi_app *wapp,
	const u8 *mac_addr);

struct wapp_sta*  wdev_ap_client_list_lookup(
	struct wifi_app *wapp,
	struct ap_dev *ap,
	const u8 *mac_addr);

struct wapp_block_sta* wdev_ap_block_list_lookup(
	struct wifi_app *wapp,
	struct ap_dev *ap,
	u8 *mac_addr);

int wapp_del_block_sta(
	struct ap_dev *ap,
	unsigned char *sta_addr);

int wapp_update_block_list(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct ap_dev *ap);

int wapp_add_block_sta(
	struct wifi_app *wapp,
	struct ap_dev *ap,
	u8 *sta_addr,
	u16 valid_period);

int wdev_ap_block_list_init(
	struct wifi_app *wapp,
	struct ap_dev *ap);

int wdev_ap_show_block_list(
	struct wifi_app *wapp,
	struct ap_dev *ap);

int wdev_ap_show_cli_list(
	struct wifi_app *wapp,
	struct ap_dev *ap);

int wdev_ap_show_chn_list(
	struct wifi_app *wapp,
	struct ap_dev *ap);

int wdev_ap_show_bss_info(
	struct wifi_app *wapp,
	struct ap_dev *ap);

int wdev_ap_show_ap_metric(
	struct wifi_app *wapp,
	struct ap_dev *ap);

int wdev_set_sec_and_ssid(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	struct sec_info *sec,
	char *ssid);

int wapp_issue_scan_request(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev);

int wdev_set_ch(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	int ch,
    unsigned char op_class);

int wdev_set_quick_ch(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int ch);

int wdev_set_ssid(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	char *ssid);

int wdev_set_psk(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	char *psk);

#ifdef DPP_SUPPORT
enum dpp_akm;

int wdev_set_dpp_akm(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	enum dpp_akm akm);

void wdev_enable_pmf(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev);

int wdev_enable_apcli_iface(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int enable);
#endif /*DPP_SUPPORT*/

int wdev_set_radio_onoff(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int onoff);

int wdev_ap_show_ap_info(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct ap_dev	*ap);


int wdev_ap_set_txpwr_limit(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	char pwr_limit);

void wapp_show_dev_list(
	struct wifi_app *wapp);

int wapp_client_clear(
	struct wifi_app *wapp,
	struct ap_dev	*ap,
	u8 *mac_addr);

int wdev_set_bss_role(
	struct wapp_dev *wdev,
	unsigned char map_vendor_extension);

int wdev_set_hidden_ssid(
	struct wapp_dev *wdev,
	unsigned char hidden_ssid);
int wdev_set_bss_coex(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	char enable);
int driver_wext_get_bss_coex(void *drv_data,
	const char *ifname, char *bss_coex);
#ifdef DPP_SUPPORT
struct wapp_dev* wapp_dev_list_lookup_by_radio_and_type(struct wifi_app *wapp, char* ra_identifier, const u8 wdev_type);
int driver_wext_get_dpp_frame(void *drv_data, const char *ifname, char *frame, int length);
#endif /*DPP_SUPPORT*/

void wdev_enable_apcli_pmf(struct wifi_app *wapp, struct wapp_dev *wdev);
void wdev_bh_sta_reset_default(struct wifi_app *wapp, struct wapp_dev *wdev);

int driver_wext_get_assoc_req_frame(void *drv_data, 
	const char *ifname, char *assoc_data, int length);

#ifdef MAP_R2
void map_build_and_send_ch_scan_rep(struct wifi_app *wapp);
void ts_bh_set_default_8021q(struct wapp_dev *wdev, unsigned short primary_vid, unsigned char pcp);
void ts_bh_set_all_vid(struct wapp_dev *wdev, unsigned char vlan_num, unsigned short vids[]);
void ts_fh_set_vid(struct wapp_dev *wdev, unsigned short vid);
extern struct global_oper_class oper_class[];
#ifdef DFS_CAC_R2
int driver_wext_get_cac_capability(void *drv_data, const char *ifname, char *buf, unsigned int length);
int driver_wext_get_avail_channel(void *drv_data, const char *ifname, char *buf, unsigned int length);

#endif
void wapp_set_mbo_allow_disallow(struct wifi_app *wapp);
#endif

#endif /* #ifndef _WDEV_H_ */
