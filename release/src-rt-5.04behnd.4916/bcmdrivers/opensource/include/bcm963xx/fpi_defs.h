#ifndef __FPI_COMMON_H_INCLUDED__
#define __FPI_COMMON_H_INCLUDED__
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *******************************************************************************
 * File Name : fpi_defs.h
 *
 *******************************************************************************
 */

#include <linux/if.h>
#include <linux/if_ether.h>

#ifndef VLAN_HLEN
#define VLAN_HLEN	4
#endif

typedef enum {
	fpi_mode_l2,	/* perform L3L4 mode lookup if DA MAC == AP MAC */
	fpi_mode_l3l4,
	fpi_mode_num
} fpi_mode_t;

typedef struct {
	/* device name and pointer have to be the first 2 fields */
	char ingress_device_name[IFNAMSIZ];
	uint64_t ingress_device_ptr;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dst_mac[ETH_ALEN];
	uint16_t eth_type;		/* in host order */
	uint8_t vtag_num;		/* 2-bit value */
	uint8_t packet_priority;	/* 6-bit value */
} fpi_l2_key_t;

typedef struct {
	char ingress_device_name[IFNAMSIZ];	/* name is used for userspace */
	uint64_t ingress_device_ptr;
	uint8_t vtag_num;		/* 2-bit value */
	uint8_t is_ipv6			/* 1-bit value */;
	/* IP address fields should be host order */
	uint32_t src_ip[4];
	uint32_t dst_ip[4];
	uint8_t l4_proto;
	uint16_t src_port;
	uint16_t dst_port;
	uint8_t esp_spi_mode;		/* 2-bit value */
#define FPI_ESP_IGNORED	0
#define FPI_ESP_IN_IP	1
#define FPI_ESP_IN_UDP	2
	uint32_t esp_spi;
	uint8_t packet_priority;	/* 6-bit value */
} fpi_l3l4_key_t;

typedef struct {
	fpi_mode_t mode;
	union {
		fpi_l2_key_t l2_key;
		fpi_l3l4_key_t l3l4_key;
	};
} fpi_key_t;

typedef enum {
	fpi_l2hdr_mode_none,
	fpi_l2hdr_mode_14b,
	fpi_l2hdr_mode_18b,
	fpi_l2hdr_mode_22b,
	fpi_l2hdr_mode_num
} fpi_l2hdr_mode_t;

typedef struct {
	char egress_device_name[IFNAMSIZ];	/* name is used for userspace */
	uint64_t egress_device_ptr;
	uint8_t wl_user_priority;	/* 3-bit value for WLAN interfaces */
	struct {
		uint16_t vtag_check : 1;
		uint16_t unsed : 3;
		uint16_t vtag_value : 12;
#define FPI_VLAN_TAG_MAX	0x0fff
	};

	/* when both vlan_8021q_prepend and vlan_8021q_remove are set,
	 * then remove will be done before prepend to achieve replace
	 */
	struct {
		uint16_t drop : 1;		/* 1-bit value */
		uint16_t vlan_8021q_prepend : 1;	/* 1-bit value */
		uint16_t vlan_8021q_remove : 1;	/* 1-bit value */
		uint16_t napt_enable : 1;	/* 1-bit value */
		uint16_t dscp_rewrite : 1;	/* 1-bit value */
		uint16_t gre_prepend : 1;	/* 1-bit value */
		uint16_t gre_remove : 1;	/* 1-bit value */
		uint16_t outer_l2hdr_insert_mode : 2;	/* 2-bit value.
							 * fpi_l2hdr_mode_t.
							 */
		uint16_t outer_l2hdr_remove_mode : 2;	/* 2-bit value.
							 * fpi_l2hdr_mode_t.
							 */
		uint16_t unused1 : 5;
	};
	uint32_t vlan_8021q_hdr;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dst_mac[ETH_ALEN];
	/* IP address fields should be host order */
	uint32_t src_ip[4];
	uint32_t dst_ip[4];
	uint8_t dscp;		/* 6-bit value */
#define FPI_DSCP_MAX	0x3f
	uint16_t src_port;
	uint16_t dst_port;
	uint32_t gre_hdr;
	uint8_t outer_l2hdr[ETH_HLEN + VLAN_HLEN * 2];
	uint8_t outer_src_mac[ETH_ALEN];	/* for L2GRE. obsolete */
	uint8_t outer_dst_mac[ETH_ALEN];	/* for L2GRE. obsolete */
	uint8_t encap_ip_hdr[40];	/* up to 40 byte for outer IPv6 Hdr */
	uint8_t egress_priority;	/* 3-bit for Queue of non-WLAN
					 * interface types; 1-bit for WLAN
					 * interface congestion control */
	uint16_t max_ingress_packet_size;
} fpi_context_t;

typedef struct {
	fpi_key_t key;
	fpi_context_t context;
} fpi_flow_t;

typedef enum {
	fpi_gre_mode_standard,
	fpi_gre_mode_proprietary,
	fpi_gre_mode_num
} fpi_gre_mode_t;

typedef struct {
	uint32_t pkt;
	uint64_t byte;
} fpi_stat_t;

typedef union {
	uint16_t half;
	struct {
		/* following the same order as defined of "pktfwd_key" in
		 * bcm_pktfwd.h
		 */
		uint16_t endpoint : 12;
		uint16_t incarnation : 1;
		uint16_t domain : 3;
	};
} fpi_wfd_nic_key_t;

typedef struct {
	uint8_t wl_dst_type;
#define FPI_WL_DST_TYPE_WFD_NIC		0
#define FPI_WL_DST_TYPE_FIRST		FPI_WL_DST_TYPE_WFD_NIC
#define FPI_WL_DST_TYPE_WFD_DHD		1
#define FPI_WL_DST_TYPE_WDO_DIRECT	2
#define FPI_WL_DST_TYPE_LAST		FPI_WL_DST_TYPE_WDO_DIRECT
#define FPI_WL_DST_TYPE_INVALID		(FPI_WL_DST_TYPE_LAST + 1)
	uint8_t radio_idx;	/* 2-bit value, use for all modes */
	uint16_t flowring_id;	/* 10-bit value, use for WFD_DHD and WDO-DIR */
	fpi_wfd_nic_key_t wfd_nic_key;
} fpi_wlan_egress_info_t;

#endif /* __FPI_COMMON_H_INCLUDED__ */
