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

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "hotspot.h"
#include "wapp_cmm.h"

struct hotspot;
struct wifi_app;
struct wapp_req;

struct wapp_drv_ops {
	void * (*drv_inf_init)(struct wifi_app *wapp, const int opmode, const int drv_mode);
	int (*drv_inf_exit)(struct wifi_app *wapp);
	int (*drv_wifi_version)(void *drv_dta, const char *ifname,
							char *ver, size_t *len);
	int (*drv_wapp_version_check)(void *drv_dta, const char *ifname);
	
	int (*drv_wapp_req) (void *drv_data, const char *ifname, struct wapp_req *req);
	int (*drv_send_btm_req)(void *drv_data, const char *ifname,
							const u8 *peer_sta_addr, const char *btm_req,
							size_t btm_req_len);
	int (*drv_send_btm_query)(void *drv_data, const char *ifname,
							  const char *peer_sta_addr, const char *btm_query,
							  size_t btm_query_len);
	int (*drv_send_btm_rsp)(void *drv_data, const char *ifname,
							const u8 *peer_sta_addr, const char *btm_rsp,
							size_t btm_rsp_len);
	int (*drv_send_reduced_nr_list)(void *drv_data, const char *ifname,
							const char *reduced_nr_list,
							size_t reduced_nr_list_len);
	int (*drv_wapp_param_setting)(void *drv_data, const char *ifname, u32 param, u32 value);
	int (*drv_send_anqp_req)(void *drv_data, const char *ifname,
							 const char *peer_sta_addr,const char *anqp_req,
							 size_t anqp_req_len);
	int (*drv_send_anqp_rsp)(void *drv_data, const char *ifname,
							 const u8 *peer_sta_addr,const char *anqp_rsp,
							 size_t anqp_rsp_len);
	int (*drv_set_interworking)(void *drv_data, const char *ifname, char *enable, size_t len);
	int (*drv_send_wnm_notify_req)(void *drv_data, const char *ifname,
							const char *peer_sta_addr, const char *btm_req, size_t btm_req_len, int type);
	int (*drv_set_ie)(void *drv_data, const char *ifname, char *ie,
					  size_t ie_len);
	int (*drv_wps_pbc_trigger)(void *drv_dta, const char *ifname,
							char *ver, size_t *len);
#ifdef MAP_R2
	int (*drv_ch_scan_req)(void *drv_dta, const char *ifname,
							const char *ch_scan_req, size_t len);
#ifdef DFS_CAC_R2
	int (*drv_cac_req)(void *drv_data, const char *ifname, u32 param, u32 value);
	int (*drv_set_monitor_ch_assign)(void *drv_data, const char *ifname, char *msg,
					  size_t msg_len);
	int (*drv_get_ch_avail_list)(void *drv_data, const char *ifname, char *msg,
				  unsigned int msg_len);
#endif
#endif
	int (*drv_get_misc_cap)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_ht_cap)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_vht_cap)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_he_cap)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_chan_list)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_nop_channels)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_op_class)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_bss_info)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_get_ap_metrics)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_off_ch_scan_req)(void *drv_dta, const char *ifname,
							const char *ch_scan_req, size_t len);
	int (*drv_get_chip_id)(void *drv_data, const char *ifname, char *buf, 
								  size_t *buf_len);
	int (*drv_set_ssid)(void *drv_data, const char *ifname, char *buf);
	int (*drv_set_psk)(void *drv_data, const char *ifname, char *buf);
	int (*send_action)(void *drv_data, const char *ifname, unsigned int chan,
                                      unsigned int wait_time,
                                      const u8 *dst, const u8 *src,
                                      const u8 *bssid,
                                      const u8 *data, size_t data_len, u16 seq_no,
                                      int no_cck);
	int (*drv_cancel_roc)(void *drv_data, const char *ifname);
	int (*drv_start_roc)(void *drv_data, const char *ifname, unsigned int freq,
                                      unsigned int wait_time);
#ifdef DPP_SUPPORT
	int (*drv_set_pmk)(void *drv_data, const char *ifname,
                     const u8 *pmk, size_t pmk_len, const u8 *pmkid,
                     const u8 *aa, const u8 *spa, int session_timeout,
                     int akmp);
#endif /*DPP_SUPPORT*/
};

struct hotspot_drv_ops {
	int (*drv_test)(void *drv_data, const char *ifname);
	
	int (*drv_hotspot_onoff)(void *drv_data, const char *ifname,
                             int enable, int event_trigger, int event_type);
	
	int (*drv_hs_param_setting)(void *drv_data, const char *ifname, u32 param, u32 value);

	int (*drv_ipv4_proxy_arp_list)(void *drv_data, const char *ifname,
							  	   char *proxy_arp_list, size_t *proxy_arp_list_len);
	
	int (*drv_ipv6_proxy_arp_list)(void *drv_data, const char *ifname,
							  	   char *proxy_arp_list, size_t *proxy_arp_list_len);
	int (*drv_validate_security_type)(void *drv_data, const char *ifname);
	int (*drv_reset_resource)(void *drv_data, const char *ifname);
	int (*drv_get_bssid)(void *drv_data, const char *ifname, char *bssid, size_t *bssid_len);
	int (*drv_get_osu_ssid)(void *drv_data, const char *ifname, char *ssid, size_t *ssid_len);
	int (*drv_set_osu_asan)(void *drv_data, const char *ifname, char *enable, size_t len);
	
	int (*drv_send_qosmap_configure)(void *drv_data, const char *ifname,
							const char *peer_sta_addr, const char *qosmap, size_t qosmap_len);
	int (*drv_set_bss_load)(void *drv_data, const char *ifname, char *buf, size_t len);
	
};

struct mbo_drv_ops {
	int (*drv_mbo_param_setting)(void *drv_data, const char *ifname, u32 param, u32 value);
};

#endif /* __DRIVER_H__ */
