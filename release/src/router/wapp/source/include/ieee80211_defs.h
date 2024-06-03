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
 
#ifndef __IEEE80211_DEFS_H__
#define __IEEE80211_DEFS_H__

#include "types.h"
#include "list.h"

enum dot11u_status_code {
	ADVERTISEMENT_PROTOCOL_NOT_SUPPORTED = 59,
	UNSPECIFIED_FAILURE = 60,
	RESPONSE_NOT_RECEIVED_FROM_SERVER = 61,
	TIMEOUT = 62,
	QUERY_RESPONSE_TOO_LARGE = 63,
	REQUEST_REFUSED_HOME_NETWORK_NOT_SUPPORT = 64,
	SERVER_UNREACHABLE = 65,
	REQUEST_REFUSED_PERMISSIONS_RECEIVED_FROM_SSPN = 67,
	REQUEST_REFUSED_AP_NOT_SUPPORT_UNAUTH_ACCESS = 68,
	TRANSMISSION_FAILURE = 79,	
};


/* ANQP Info ID definitions */
enum {
	ANQP_QUERY_LIST = 256,
	ANQP_CAPABILITY,
	VENUE_NAME_INFO,
	EMERGENCY_CALL_NUMBER_INFO,
	NETWORK_AUTH_TYPE_INFO,
	ROAMING_CONSORTIUM_LIST,
	IP_ADDRESS_TYPE_AVAILABILITY_INFO,
	NAI_REALM_LIST,
	ThirdGPP_CELLULAR_NETWORK_INFO,
	AP_GEOSPATIAL_LOCATION,
	AP_CIVIC_LOCATION,
	AP_LOCATION_PUBLIC_IDENTIFIER_URI,
	DOMAIN_NAME_LIST,
	EMERGENCY_ALERT_IDENTIFIER_URI,
	EMERGENCY_NAI = 271,
	NEIGHBOR_REPORT = 272,
	VENUE_URL = 277,
	ADVICE_OF_CHARGE = 278,
	ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST = 56797,
};

/* HS2.0 ANQP Element subtype definition */
enum {
	HS_QUERY_LIST = 1,
	HS_CAPABILITY,
	OPERATOR_FRIENDLY_NAME,
	WAN_METRICS,
	CONNECTION_CAPABILITY,
	NAI_HOME_REALM_QUERY,
	OPERATING_CLASS,
	OSU_PROVIDE_LIST,
	ANONYMOUS_NAI,
	ICON_REQUEST,
	ICON_BINARY_FILE,
	ICON_METADATA,
	OSU_PROVIDERS_NAI_LIST,
};

/* MBO Spec r0.0.27 */
/* MBO ANQP Element subtype definition */
enum {
	MBO_QUERY_LIST = 1,
	MBO_CDCP = 2,
};

/* EAP method types */
enum {
	EAP_TLS = 13,
	EAP_SIM = 18,
	EAP_TTLS = 21,
	EAP_AKA = 23,
};

/* Download status code */
enum {
	SUCCESS = 0,
	FILE_NOT_FOUND = 1,
	UNSPECIFIC_FILE_ERROR = 2,
};

/* Authentication parameter types */
enum {
	EXPANDED_EAP_METHOD = 1,
	NON_EAP_INNER_AUTH_TYPE,
	INNER_AUTH_EAP_METHOD_TYPE,
	EXPANDED_INNER_EAP_METHOD,
	CREDENTIAL_TYPE,
	TUNNELED_EAP_METHOD_CREDENTIAL_TYPE,
	VENDOR_SPECIFIC = 221,
};

struct anqp_frame {
	u16 info_id;
	u16 length;
	char variable[0];
} __attribute__ ((packed));


struct hs_anqp_frame {
	u16 info_id;
	u16 length;
	u8 oi[3];
	u8 type;
	u8 subtype;
	u8 reserved;
	u8 variable[0];
} __attribute__ ((packed));

struct mbo_anqp_frame {
	u16 info_id;
	u16 length;
	char oi[3];
	u8 type;
	u8 subtype;	
	u8 variable[0];
} __attribute__ ((packed));


#define IE_NEIGHBOR_REPORT		52
#define IE_INTERWORKING			107
#define IE_ADVERTISEMENT_PROTO	108
#define IE_QOS_MAP_SET			110
#define IE_ROAMING_CONSORTIUM	111
#define IE_REDUCED_NEIGHBOR_REPORT		201 
#define IE_HS2_INDICATION		221

#define WFA_TIA_HS 	0x11
#define WFA_TIA_MBO	0x12

struct interworking_element {
	u8 eid;
	u8 length;
	u8 access_network_options;
	/*
 	 * Following are Venue info and HESSID
 	 */
	char variable[0];
} __attribute__ ((packed));

struct roaming_consortium_info_element {
	u8 eid;
	u8 length;
	u8 num_anqp_oi;
	u8 oi1_oi2_length;
	/*
 	 * Following are OI#1, OI#2(optional), and OI#3(optional)
 	 */
	char variable[0];
} __attribute__ ((packed));

struct hotspot2dot0_indication_element {
	u8 eid;
	u8 length;
	char oi[3]; /* 0x50 6F 9A */
	u8 type;
	u8 hotspot_conf;
	u16 anqp_domain_id;
} __attribute__ ((packed));

struct p2p_attribute {
	u8 attribute_id;
	u16 length;
	char variable[0];
} __attribute__ ((packed));

struct p2p_info_element {
	u8 eid;
	u8 length;
	char oui[3]; /* 0x50 6F 9A */
	u8 oui_type; /* 0x09 */
	/*
 	 * Following are P2P attributes
 	 */
	char variable[0];
} __attribute__ ((packed));

struct advertisement_proto_element {
	u8 eid;
	u8 length;
	/*
 	 * Following are advertisement protocol tuple #1, #2, ..#n
 	 */	 
	char variable[0];
} __attribute__ ((packed)); 

struct qosmap_element {
	u8 eid;
	u8 length;
	u16 dscp_range[8];
	/*
 	 * dscp_exception max to 21 fields
 	 */
	char variable[0];
} __attribute__ ((packed));

struct plmn_IEI {
	u8 plmn_list_iei;
	u8 plmn_list_len;
	u8 plmn_list_num;
	char variable[0];
} __attribute__ ((packed));

struct domain_name_field {
	struct dl_list list;
	u8 length;
	char domain_name[0];
};

struct venue_name_duple {
	struct dl_list list;
	u8 length;
	char language[3];
	char venue_name[0];
};

struct operator_name_duple {
	struct dl_list list;
	u8 length;
	char language[3];
	char operator_name[0];
};

struct osu_providers_nai_duple {
	struct dl_list list;
	u8 length;
	char osu_providers_nai_list[0];
};

struct operator_icon_metadata {
	struct dl_list list;
	u8 type_len;
	u8 filename_len;
	u16 weight;
	u16 height;
	char language[3];
	char icon_buf[0];
};

struct oi_duple {
	struct dl_list list;
	u8 length;
	char oi[0];
};

struct net_auth_type_unit {
	struct dl_list list;
	u8 net_auth_type_indicator;
	u16 re_direct_URL_len;
	char re_direct_URL[0];
};

struct proto_port_tuple {
	struct dl_list list;
	u8 ip_protocol;
	u16 port;
	u8 status;
};

struct anqp_capability {
	struct dl_list list;
	u16 info_id;
};

struct anqp_hs_capability {
	struct dl_list list;
	u8 subtype;
};

struct auth_param {
	struct dl_list list;
	u8 id;
	u8 len;
	char auth_param_value[0];
};

struct eap_method {
	struct dl_list list;
	u8 len;
	u8 eap_method;
	u8 auth_param_count;
	struct dl_list auth_param_list;
};

struct nai_realm_data {
	struct dl_list list;
	u16 nai_realm_data_field_len;
	u8 nai_realm_encoding;
	u8 nai_realm_len;
	u8 eap_method_count;
	struct dl_list eap_method_list;
	char nai_realm[0];
};

struct nai_home_realm_data_query {
	struct dl_list list;
	u8 nai_home_realm_encoding;
	u8 nai_home_realm_len;
	char nai_home_realm[0];
};

struct wan_metrics {
	u8 link_status;
	u8 at_capacity;
	u32 dl_speed;
	u32 ul_speed;
	u8 dl_load;
	u8 ul_load;
	u16 lmd;
};

struct plmn {
	struct dl_list list;
	char mcc[3];
	char mnc[3];
};

struct operating_class_unit {
	struct dl_list list;
	u8 op_class;
};

struct osu_service_desc {
	struct dl_list list;
	u8 len;
	char language[3];
	char osu_service_desc_value[0];
};

struct osu_nai {
	struct dl_list list;
	u8 len;
	char osu_nai_value[0];
};

struct icon_available {
	struct dl_list list;
	u8 type_len;
	u8 filename_len;
	u16 weight;
	u16 height;
	char language[3];
	char icon_buf[0];
};

struct icon_binary {
	struct dl_list list;
	u16 filesize;
	char filename[0];
};

struct osu_method {
	struct dl_list list;
	char osu_method_value;
};

struct osu_friendly_name {
	struct dl_list list;
	u8 len;
	char language[3];
	char osu_friendly_name_value[0];
};

struct osu_providers {
	struct dl_list list;
	u16 osu_providers_list_field_len;
	u16 osu_friendly_name_len;
	struct dl_list osu_friendly_name_list;
	u8 osu_server_uri_len;
	u8 *osu_server_uri;	
	u8 osu_method_len;
	struct dl_list osu_method_list;
	u16 icon_len;
	struct dl_list icon_list;
	u8 osu_nai_len;
	struct dl_list osu_nai_list;
	u16 osu_service_len;
	struct dl_list osu_service_desc_list;
};

struct advice_of_charge_data {
	struct dl_list list;
	u8 advice_of_charge_type;
	u8 aoc_realm_encoding;
	u8 aoc_realm_len;
	char *aoc_realm;
	struct dl_list aoc_plan_tuples_list;
};

struct aoc_plan_tuple_data {
	struct dl_list list;
	u16 plan_information_len;
	char language[3];
	char currency_code[3];
	char *plan_information;
};

struct venue_url_duple {
	struct dl_list list;
	u8 url_length;
	u8 venue_number;
	char venue_url[0];
};

#define GAS_ANQP_QUERY 	  0
#define GAS_ANQP_HS_QUERY 1

#define GUD_VER1 0
#define IEI_PLMN 0

#endif /* IEEE80211_DEFS_H__ */
