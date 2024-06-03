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
 
#ifndef __DRIVER_WEXT_H__
#define __DRIVER_WEXT_H__

#include "types.h"

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

/* Ralink defined OIDs */
#define RT_PRIV_IOCTL			(SIOCIWFIRSTPRIV + 0x01)
#define OID_GET_SET_TOGGLE					0x8000
#if 0
#define OID_802_11_WIFI_VER					0x0887
#define OID_802_11_HS_TEST      			0x0888
#define OID_802_11_HS_IE        			0x0889
#define OID_802_11_HS_ANQP_REQ  			0x088a
#define OID_802_11_HS_ANQP_RSP  			0x088b
#define OID_802_11_HS_ONOFF     			0x088c
#define OID_802_11_HS_IPCTYPE				0x088d
#define OID_802_11_HS_PARAM_SETTING			0x088e
#define RT_OID_WE_VERSION_COMPILED			0x0622
#define OID_802_11_WNM_BTM_REQ				0x0900
#define OID_802_11_WNM_BTM_QUERY			0x0901
#define OID_802_11_WNM_BTM_RSP				0x0902
#define OID_802_11_WNM_PROXY_ARP			0x0903
#define OID_802_11_WNM_IPV4_PROXY_ARP_LIST	0x0904
#define OID_802_11_WNM_IPV6_PROXY_ARP_LIST	0x0905
#define OID_802_11_SECURITY_TYPE			0x0906
#define OID_802_11_HS_RESET_RESOURCE		0x0907
#define OID_802_11_HS_AP_RELOAD				0x0908
#define OID_802_11_HS_BSSID					0x0909
#else
#define RT_OID_WE_VERSION_COMPILED              0x0622
#define OID_WAPP_EVENT							0x0647
#define OID_802_11_COEXISTENCE					0x0530

#define OID_MTK_CHIP_ID							0x068A
#define OID_802_11_WIFI_VER                     0x0920
#define OID_802_11_WAPP_SUPPORT_VER             0x0921
#define OID_802_11_WAPP_IE                      0x0922
#define OID_802_11_HS_ANQP_REQ                  0x0923
#define OID_802_11_HS_ANQP_RSP                  0x0924
#define OID_802_11_HS_ONOFF                     0x0925
#define OID_802_11_WAPP_PARAM_SETTING           0x0927
#define OID_802_11_WNM_BTM_REQ                  0x0928
#define OID_802_11_WNM_BTM_QUERY                0x0929
#define OID_802_11_WNM_BTM_RSP                  0x093a
#define OID_802_11_WNM_PROXY_ARP                0x093b
#define OID_802_11_WNM_IPV4_PROXY_ARP_LIST      0x093c
#define OID_802_11_WNM_IPV6_PROXY_ARP_LIST      0x093d
#define OID_802_11_SECURITY_TYPE                0x093e
#define OID_802_11_HS_RESET_RESOURCE            0x093f
#define OID_802_11_HS_AP_RELOAD                 0x0940
#define OID_802_11_HS_BSSID                     0x0941
#define OID_802_11_HS_OSU_SSID                  0x0942
#define OID_802_11_HS_SASN_ENABLE               0x0943
#define OID_802_11_WNM_NOTIFY_REQ               0x0944
#define OID_802_11_QOSMAP_CONFIGURE             0x0945
#define OID_802_11_GET_STA_HSINFO				0x0946
#define OID_802_11_BSS_LOAD						0x0947
#define OID_802_11_HS_LOCATION_DRV_INFORM_IE	0x0948
#define OID_802_11_INTERWORKING_ENABLE			0x0949

#define OID_BNDSTRG_MSG							0x0950
#define OID_BNDSTRG_GET_NVRAM					0x0951
#define OID_BNDSTRG_SET_NVRAM					0x0952

#define OID_802_11_MBO_MSG						0x0953
#define OID_NEIGHBOR_REPORT						0x0954
#define OID_802_11_OCE_REDUCED_NEIGHBOR_REPORT  			0x0969
#define OID_OFFCHANNEL_INFO						0x0955
#define OID_802_11_CURRENT_CHANNEL_INFO			0x0956
#define OID_OPERATING_INFO						0x0957
#define OID_802_11_OCE_MSG						0x0958
#ifdef MAP_R2
#define OID_DFS_ZERO_WAIT						0x0985
#endif
#define OID_802_11_CHANNELINFO					0x0999
#define OID_WSC_UUID							0x0990
#define OID_SET_SSID							0x0992
#define OID_SET_PSK								0x0993
#define OID_GET_WSC_PROFILES					0x0994
#define OID_GET_MISC_CAP 						0x0995
#define OID_GET_HT_CAP 							0x0996
#define OID_GET_VHT_CAP 						0x0997
#define OID_GET_CHAN_LIST 						0x0998
#define OID_GET_OP_CLASS 						0x0999
#define OID_GET_BSS_INFO 						0x099A
#define OID_GET_AP_METRICS 						0x099B
#define OID_GET_NOP_CHANNEL_LIST				0x099C
#define OID_GET_HE_CAP 							0x099D

#ifdef MAP_R2
#ifdef DFS_CAC_R2
#define OID_GET_CAC_CAP							0x09A0
#define OID_802_11_CAC_STOP						0x09A1
#endif
#endif


#endif
#define OID_SEND_OFFCHAN_ACTION_FRAME					0x09a3
#define OID_802_11_CANCEL_ROC						0x09a4
#define OID_802_11_START_ROC						0x09a5
#ifdef DPP_SUPPORT
#define OID_802_11_SET_PMK						0x09a6
#define OID_802_11_GET_DPP_FRAME					0x09a7
#endif /*DPP_SUPPORT*/
#define OID_GET_ASSOC_REQ_FRAME						0x099F

#define LENGTH_802_11               24
struct driver_wext_data {
	int opmode;
	char drv_mode;
	void *priv;
	struct netlink_data *netlink;
	int ioctl_sock;
	int we_version_compiled;
};

struct anqp_req_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 anqp_req_len;
	char anqp_req[0];
};

struct anqp_rsp_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u16 status;
	u32 anqp_rsp_len;
	char anqp_rsp[0];
};
#ifdef MAP_R2
struct wnn_notify_req_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 wnm_req_len;
	char wnm_req[0];
};
#endif

struct hs_onoff {
	u32 ifindex;
	u8 hs_onoff;
	u8 event_trigger;
	u8 event_type;
};

struct wapp_param_setting {
	u32 param;
	u32 value;
};
#if 1 //OPENWRT_SUPPORT
/* please check driver should be GNU_PACKED as well */
struct GNU_PACKED btm_req_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_req_len;	
	char btm_req[0];
};

struct GNU_PACKED btm_rsp_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_rsp_len;
	char btm_rsp[0];
};

struct GNU_PACKED btm_query_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_query_len;
	char btm_query[0];
};

struct GNU_PACKED neighbor_list_data {
	u32 ifindex;
	u32 neighbor_list_len;	
	char neighbor_list_req[0];
};

struct GNU_PACKED reduced_neighbor_list_data {
	u32 ifindex;
	u32 reduced_neighbor_list_len;	
	char reduced_neighbor_list_req[0];
};

#else

struct btm_req_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_req_len;	
	char btm_req[0];
};

struct btm_rsp_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_rsp_len;
	char btm_rsp[0];
};

struct btm_query_data {
	u32 ifindex;
	u8 peer_mac_addr[6];
	u32 btm_query_len;
	char btm_query[0];
};
#endif /* CONFIG_SUPPORT_OPENWRT */

struct proxy_arp_entry {
	u32 ifindex;
	u8 ip_type;
	u8 from_ds;
	u8 IsDAD;
	char source_mac_addr[6];
	char target_mac_addr[6];
	char ip_addr[0];
};

struct security_type {
	u32 ifindex;
	u8 auth_mode;
	u8 encryp_type;
};

struct proxy_arp_ipv4_unit {
	u8   target_mac_addr[6];
	u8   target_ip_addr[4];
};

struct proxy_arp_ipv6_unit {
	u8 target_mac_addr[6];
	u8 target_ip_type;
	u8 target_ip_addr[16];
};

struct wnm_req_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 type;
	u32 wnm_req_len;	
	char wnm_req[0];
};

struct qosmap_data {
	u32 ifindex;
	char peer_mac_addr[6];
	u32 qosmap_len;
	char qosmap[0];
};

#endif /* __DRIVER_WEXT_H__ */
