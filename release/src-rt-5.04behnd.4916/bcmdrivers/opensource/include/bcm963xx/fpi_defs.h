#ifndef __FPI_COMMON_H_INCLUDED__
#define __FPI_COMMON_H_INCLUDED__
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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

typedef enum {
	fpi_mode_l2_bridge,
	fpi_mode_l2,
	fpi_mode_l3l4,
	fpi_mode_fallback,	/* L3L4 mode first, then L2 bridge */
	fpi_mode_num
} fpi_mode_t;

typedef struct {
	/* device name and pointer have to be the first 2 fields */
	char ingress_device_name[IFNAMSIZ];
	uint64_t ingress_device_ptr;
	uint8_t dst_mac[ETH_ALEN];
	uint8_t vtag_num;		/* 2-bit value */
	uint8_t packet_priority;	/* 6-bit value */
} fpi_l2_bridge_key_t;

typedef struct {
	/* device name and pointer have to be the first 2 fields */
	char ingress_device_name[IFNAMSIZ];
	uint64_t ingress_device_ptr;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dst_mac[ETH_ALEN];
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
		fpi_l2_bridge_key_t l2_bridge_key;
		fpi_l2_key_t l2_key;
		fpi_l3l4_key_t l3l4_key;
	};
} fpi_key_t;

typedef union {
	uint16_t half;
	struct {
		/* following the same order as defined of "pktfwd_key" in
		 * bcm_pktfwd.h */
		uint16_t endpoint : 12;
		uint16_t incarnation : 2;
		uint16_t domain : 2;
	};
} fpi_wfd_nic_key_t;

typedef struct {
	char egress_device_name[IFNAMSIZ];	/* name is used for userspace */
	uint64_t egress_device_ptr;
	uint8_t wl_dst_type;
#define FPI_WL_DST_TYPE_WFD_NIC		0
#define FPI_WL_DST_TYPE_FIRST		FPI_WL_DST_TYPE_WFD_NIC
#define FPI_WL_DST_TYPE_WFD_DHD		1
#define FPI_WL_DST_TYPE_WDO_DIRECT	2
#define FPI_WL_DST_TYPE_LAST		FPI_WL_DST_TYPE_WDO_DIRECT
#define FPI_WL_DST_TYPE_INVALID		(FPI_WL_DST_TYPE_LAST + 1)
	uint8_t radio_idx;	/* 2-bit value, use for all modes */
	uint8_t user_priority;	/* 3-bit value, use for all modes */
	uint16_t flowring_id;	/* 10-bit value, use for WFD_DHD and WDO-DIR */
	fpi_wfd_nic_key_t wfd_nic_key;
	struct {
		uint16_t vtag_check : 1;
		uint16_t unsed : 3;
		uint16_t vtag_value : 12;
#define FPI_VLAN_TAG_MAX	0x0fff
	};

	/* when both vlan_8021q_prepend and vlan_8021q_remove are set,
	 * then remove will be done before prepend to achieve replace */
	struct {
		uint8_t vlan_8021q_prepend : 1;	/* 1-bit value */
		uint8_t vlan_8021q_remove : 1;	/* 1-bit value */
		uint8_t napt_enable : 1;	/* 1-bit value */
		uint8_t unused1 : 5;
	};
	uint32_t vlan_8021q_hdr;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dst_mac[ETH_ALEN];
	/* IP address fields should be host order */
	uint32_t src_ip[4];
	uint32_t dst_ip[4];
	uint16_t src_port;
	uint16_t dst_port;
	uint8_t egress_priority;	/* 1-bit for WLAN, 3-bit for Queue of
					 * other interface types */
	uint16_t max_ingress_packet_size;
} fpi_context_t;

typedef struct {
	fpi_key_t key;
	fpi_context_t context;
} fpi_flow_t;

typedef struct {
	uint32_t pkt;
	uint64_t byte;
} fpi_stat_t;

#endif /* __FPI_COMMON_H_INCLUDED__ */
