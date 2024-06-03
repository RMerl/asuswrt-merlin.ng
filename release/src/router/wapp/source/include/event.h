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
 
#ifndef __EVENT_H__
#define __EVENT_H__

#include "wapp_cmm.h"
#include "hotspot.h"
#include "mbo.h"
#include "oce.h"

struct wifi_app;
struct hotspot;
struct mbo_cfg;
struct wapp_event;


struct wifi_app_event_ops {
	int (*event_get_mbo_neighbor_report)(struct wifi_app *wapp,
									   const char *ifname,
								       const char *neighbor_list_req,
									   size_t neighbor_list_len);

	void (*event_handle)(struct wifi_app *wapp, struct wapp_event *event);

	int (*event_btm_rsp)(struct wifi_app *wapp,
					 const char *ifname,
					 const u8 *peer_mac_addr,
					 const char *btm_rsp,
					 size_t btm_rsp_len);

	int (*event_btm_req)(struct wifi_app *wapp,
			const char *ifname,
			const u8 *peer_mac_addr,
			const char *btm_req,
			size_t btm_req_len);

	int (*event_btm_query)(struct wifi_app *wapp,
						   const char *ifname,
						   const u8 *peer_mac_addr,
						   const char *btm_query,
						   size_t btm_query_len);

	int (*event_anqp_req)(struct wifi_app *wapp,
							   const char *iface,
							   const u8 *peer_mac_addr,
							   char *anqp_req,
							   size_t anqp_req_len);

	int (*event_anqp_rsp)(struct wifi_app *wapp,
						  const char *ifname,
						  const u8 *peer_mac_addr,
						  int status,
						  const char *anqp_rsp,
						  size_t anqp_rsp_len);
	int (*event_offch_info)(struct wifi_app *wapp,
							u8 * buf);
#ifdef MAP_R2
	int (*event_wnm_notify)(struct wifi_app *wapp,
						 const char *iface,
						 const u8 *peer_mac_addr,
						 const char *wnm_req,
						 size_t wnm_req_len);
#endif
};

struct hotspot_event_ops {
	int (*event_test)(struct wifi_app *wapp);

	int (*event_hs_onoff)(struct wifi_app *wapp,
						 const char *ifname,
						 int enable);

	int (*event_proxy_arp)(struct wifi_app *wapp,
						   const int ifindex,
						   u8 ip_type,
						   u8 from_ds,
						   const char *source_mac_addr,
						   const char *source_ip_addr,
						   const char *target_mac_addr,
						   const char *target_ip_addr,
						   unsigned char IsDAD);

	int (*event_ap_reload)(struct wifi_app *wapp,
						   const char *ifname);

	int (*event_get_location_IE)(struct wifi_app *wapp,
						   char *location);

};


struct mbo_event_ops {
	int (*get_neighbor_report)(struct hotspot *hs,
						   char *pMboMsg);
	int (*sta_update)(struct wifi_app *wapp, char *pMboMsg, u8 msg_type);
};

struct oce_event_ops {
	int (*sta_update)(struct wifi_app *wapp, char *pOcoMsg, u8 msg_type);
};


enum {
	HS_ON_OFF_BASE,
	HS_AP_RELOAD,
};

enum {
	EVENT_TRIGGER_OFF,
	EVENT_TRIGGER_ON,
};

#endif /* __EVENT_H__ */
