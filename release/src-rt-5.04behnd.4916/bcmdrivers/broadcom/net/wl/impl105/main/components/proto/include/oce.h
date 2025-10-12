/*
 * WFA OCE Specification (Optimized Connectivity Expirience)
 * fundamental types and constants
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: oce.h 827466 2023-07-13 08:18:17Z $
 */

#ifndef _OCE_H_
#define _OCE_H_

#include <mbo_oce.h>

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* OCE ATTR related macros */
#define OCE_ATTR_ID_OFF			0
#define OCE_ATTR_LEN_OFF		1
#define OCE_ATTR_DATA_OFF		2

#define OCE_ATTR_ID_LEN			1	/* Attr ID field length */
#define OCE_ATTR_LEN_LEN		1	/* Attr Length field length */
#define OCE_ATTR_HDR_LEN		2	/* ID + 1-byte length field */

/* OCE attributes as defined in the OCE spec */
enum {
	OCE_ATTR_OCE_CAPABILITY_INDICATION = 101,
	OCE_ATTR_RSSI_BASED_ASSOC_REJECTION = 102,
	OCE_ATTR_REDUCED_WAN_METRICS = 103,
	OCE_ATTR_RNR_COMPLETENESS = 104,
	OCE_ATTR_PROBE_SUPPRESS_BSSID = 105,
	OCE_ATTR_PROBE_SUPPRESS_SSID = 106,
	OCE_ATTR_TRANSMIT_PWR = 107,
	OCE_ATTR_IP_SUBNET = 108
};

/* 4.2.1 OCE Capability Indication Attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_oce_cap_ind_attr_s {
	/* Attribute ID - 101 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Carries information about OCE release */
	/* along with other indicators */
	uint8 oce_control;
} BWL_POST_PACKED_STRUCT wifi_oce_cap_ind_attr_t;

#define OCE_CAP_INDICATION_ATTR_SIZE (sizeof(wifi_oce_cap_ind_attr_t))

/* OCE Capability Indication Control Field Values */
#define OCE_RELEASE		0x1	/* bit 0 - bit 2 reserved */
#define STA_CFON		0x08	/* bit 3 */
#define OCE_11b_ONLY_AP_PRESENT	0X10	/* bit 4 */
#define HLP_ENABLED		0X20	/* bit 5 */
#define NON_OCE_AP_PRESENT	0X40	/* bit 6 */

/* 4.2.2 OCE RSSI-Based Association Rejection Attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_oce_rssi_assoc_rej_attr_s {
	/* Attribute ID - 102 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* The difference in dB acceptable by AP */
	uint8 delta_rssi;
	/* The time period in seconds for which the AP will not accept */
	/* any subsequent association */
	uint8 retry_delay;
} BWL_POST_PACKED_STRUCT wifi_oce_rssi_assoc_rej_attr_t;

#define OCE_RSSI_ASSOC_REJ_ATTR_SIZE (sizeof(wifi_oce_rssi_assoc_rej_attr_t))

#define OCE_ASSOC_REJECT_RC_INSUFFICIENT_RSSI	34

/* 4.2.3 OCE Reduced WAN Metrics Attribute */
typedef BWL_PRE_PACKED_STRUCT struct  wifi_oce_reduced_wan_metrics_attr_s {
	/* Attribute ID - 103 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Available Capacity bitmap */
	uint8 avail_capacity;
} BWL_POST_PACKED_STRUCT wifi_oce_reduced_wan_metrics_attr_t;

#define OCE_REDUCED_WAN_METR_ATTR_SIZE (sizeof(wifi_oce_reduced_wan_metrics_attr_t))

/* 4.2.4 OCE RNR Completeness Attribute */
typedef BWL_PRE_PACKED_STRUCT struct  wifi_oce_rnr_completeness_attr_s {
	/* Attribute ID - 104 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Variable short ssid list */
	uint8 short_ssid_list[];
} BWL_POST_PACKED_STRUCT wifi_oce_rnr_completeness_attr_t;

#define OCE_RNR_COMPLETENESS_ATTR_SIZE (sizeof(wifi_oce_rnr_completeness_attr_t))

/* 4.2.5 Probe Suppression BSSID Attribute */
typedef BWL_PRE_PACKED_STRUCT struct  wifi_oce_probe_suppress_bssid_attr_s {
	/* Attribute ID - 105 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Variable bssid list */
	uint8 bssid_list[];
} BWL_POST_PACKED_STRUCT wifi_oce_probe_suppress_bssid_attr_t;

#define OCE_PROBE_SUPPRESS_BSSID_ATTR_SIZE (sizeof(wifi_oce_probe_suppress_bssid_attr_t))

/* 4.2.6 Probe Suppression SSID Attribute */
typedef BWL_PRE_PACKED_STRUCT struct  wifi_oce_probe_suppress_ssid_attr_s {
	/* Attribute ID - 106 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* Variable ssid list */
	uint8 ssid_list[];
} BWL_POST_PACKED_STRUCT wifi_oce_probe_suppress_ssid_attr_t;

#define OCE_PROBE_SUPPRESS_SSID_ATTR_SIZE (sizeof(wifi_oce_probe_suppress_ssid_attr_t))
#define SHORT_SSID_LEN		4U

/* 4.2.7 Transmit power Attribute */
typedef BWL_PRE_PACKED_STRUCT struct wifi_oce_transmit_pwr_attr_s {
	/* Attribute ID - 107 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* transmit power at the output of antenna */
	int8 transmit_pwr;
} BWL_POST_PACKED_STRUCT wifi_oce_transmit_pwr_attr_t;
#define OCE_TRANSMIT_PWR_ATTR_SIZE (sizeof(wifi_oce_transmit_pwr_attr_t))

/* 4.2.8 IP subnet identifier attribute */
typedef BWL_PRE_PACKED_STRUCT struct  wifi_oce_ip_subnet_identifier_attr_s {
	/* Attribute ID - 108 */
	uint8 id;
	/* Length of the following fields in the attribute */
	uint8 len;
	/* unique 6 bytes identifier */
	uint8 ip_subnet_identifier[6];
} BWL_POST_PACKED_STRUCT wifi_oce_ip_subnet_identifier_attr_t;
#define OCE_IP_SUBNET_ATTR_SIZE (sizeof(wifi_oce_ip_subnet_identifier_attr_t))

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* __OCE_H__ */
