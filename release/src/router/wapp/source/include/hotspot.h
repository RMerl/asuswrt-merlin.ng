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

#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

//#include <net/if.h>
#include <stdint.h>
#include "types.h"
#include "os.h"
#include "list.h"
#include "util.h"
#include "driver.h"
#include "event.h"
#include "debug.h"
#include "eloop.h"
#include "rt_nl_copy.h"
#include "ctrl_iface_unix.h"
#include "ieee80211v_defs.h"
#include <sys/socket.h>
//#include <netpacket/packet.h>

#include <net/ethernet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "rt_config.h"
//#include "mbo.h"

#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define HOTSPOT_VERSION "2.0.0"

#define DEFAULT_IFNAME "ra0"

enum {
	IPV4,
	IPV6,
};


struct hs_peer_entry {
	struct dl_list list;
	u8 peer_mac_addr[6];
};

#define LOCATION_IE_LEN 256

struct hotspot {
	/* driver interface operation */
	const struct hotspot_drv_ops *drv_ops;

	/* event operation */
	const struct hotspot_event_ops *event_ops;


	/* hotspot peer entry list */
	struct dl_list hs_peer_list;	
	
	u8 version;
	char civic_IE[LOCATION_IE_LEN];
	char lci_IE[LOCATION_IE_LEN];
	char public_id_uri[LOCATION_IE_LEN];
	u16	civic_IE_len;
	u16	lci_IE_len;
	u16	public_id_uri_len;		
};
struct location_IE {	
	u16 type;
	u8 len;
	char location_buf[0];
};

int anqp_init(struct wifi_app *wapp, struct hotspot_event_ops *event_ops, int version);
void hotspot_run(struct wifi_app *wapp);
int hotspot_deinit(struct wifi_app *wapp);
int hotspot_onoff(struct wifi_app *wapp, const char *ifname, int enable, 
					int event_trigger, int event_type);

size_t hotspot_calc_btm_req_len(struct wifi_app *wapp,
							 	struct wapp_conf *conf);

int hotspot_collect_btm_req(struct wifi_app *wapp,
							struct wapp_conf *conf,
							const u8 *peer_addr,
							char *btm_req);



int hotspot_set_sta_all_ies(struct wifi_app *wapp, const char *iface);
int hotspot_set_sta_ifaces_all_ies(struct wifi_app *wapp);
int hotspot_set_ap_all_ies(struct wifi_app *wapp, const char *iface);
int hotspot_set_ap_ifaces_all_ies(struct wifi_app *wapp);
int hotspot_clear_all_ap_ies(struct wifi_app *wapp);
int hotspot_onoff_all(struct wifi_app *wapp, int enable);

int hotspot_ipv4_proxy_arp_list(struct wifi_app *wapp, const char *ifname,
						   		char *reply, size_t *reply_len);

int hotspot_ipv6_proxy_arp_list(struct wifi_app *wapp, const char *ifname,
						   		char *reply, size_t *reply_len);

int hotspot_icmpv4_deny(struct wifi_app *wapp, const char *ifname,
						u8 deny, char *reply, size_t *reply_len);

int hotspot_send_qosmap_configure(struct wifi_app *wapp, const char *iface,
						const char *peer_mac_addr,
						const char *qosmap,
						size_t qosmap_len);

int hotspot_set_osu_asan(struct wifi_app *wapp, const char *iface,
						char *enable,
						size_t len);

int hotspot_ap_reload(struct wifi_app *wapp,
					  const char *iface);

int hotspot_reset_all_ap_resource(struct wifi_app *wapp);

int hotspot_get_legacy_osu_ssid(struct wifi_app *wapp, struct wapp_conf *conf);

int hotspot_set_bss_load(struct wifi_app *wapp, struct wapp_conf *conf);

int hotspot_get_bssid(struct wifi_app *wapp, struct wapp_conf *conf);

int hotspot_collect_hs_anqp_rsp(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf);

int hotspot_collect_nai_home_realm_anqp_rsp(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf);
int hotspot_collect_icon_binary_file(struct wifi_app *wapp,
							struct wapp_conf *conf, char *buf);
size_t hotspot_calc_nai_home_realm_anqp_rsp_len(struct wifi_app *wapp,
													   struct wapp_conf *conf,
													   size_t nai_home_realm_anqp_req_len,
													   const char *curpos);
size_t hotspot_calc_icon_binary_file_len(struct wifi_app *wapp,
													   struct wapp_conf *conf,
													   size_t hs_anqp_req_len,
													   const char *icon_request);
size_t hotspot_calc_hs_anqp_rsp_len(struct wifi_app *wapp,
									struct wapp_conf *conf,
									size_t hs_anqp_req_len,
									const char *curpos);

int hotspot_init_param(struct wifi_app *wapp, struct wapp_conf *conf);

/* Hotspot operation mode */
#define OPMODE_STA 0
#define OPMODE_AP  1

/* Hotspot driver mode */
#define RA_WEXT 	0
#define RA_NETLINK  1

struct _ipv6_addr {
	union {
		u8 ipv6Addr8[16];
		u16 ipv6Addr16[8];
		u32 ipv6Addr32[4]; 
	}addr;
#define ipv6_addr addr.ipv6Addr8
#define ipv6_addr16 addr.ipv6Addr16
#define ipv6_addr32 addr.ipv6Addr32
};

#endif /* __HOTSPOT_H__ */
