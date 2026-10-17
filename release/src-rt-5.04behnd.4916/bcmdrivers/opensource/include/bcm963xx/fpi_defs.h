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

typedef enum {
	ESP_IGNORE = 0,
	ESP_US_FWD_UDP,
	ESP_US_FWD,
	ESP_DS_UDP,
	ESP_DS,
	ESP_US_UDP,
	ESP_US,
	ESP_FLOWTYPE_MAX
} fpi_esp_flow_type_t;

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
	uint32_t esp_flow_type; /* fpi_esp_flow_type_t */
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

typedef enum {
	fpi_grehdr_mode_none,
	fpi_grehdr_mode_ipv4,
	fpi_grehdr_mode_ipv6,
	fpi_grehdr_mode_num
} fpi_grehdr_mode_t;

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
		uint16_t gre_prepend : 2;	/* 2-bit value.
						 * fpi_grehdr_mode_t.
						 */
		uint16_t gre_remove : 2;	/* 2-bit value.
						 * fpi_grehdr_mode_t.
						 */
		uint16_t outer_l2hdr_insert_mode : 2;	/* 2-bit value.
							 * fpi_l2hdr_mode_t.
							 */
		uint16_t outer_l2hdr_remove_mode : 2;	/* 2-bit value.
							 * fpi_l2hdr_mode_t.
							 */
		uint16_t eth_mac_valid_en : 1;	/* 1-bit value */
		uint16_t ip_frag_en : 1;	/* 1-bit value */
		uint16_t ignore_ip_df : 1;	/* 1-bit, ignore the DF bit
						 * in the IPv4 header in incoming packet */
	};
	uint32_t vlan_8021q_hdr;

	/* ETH MAC addresses used for validation */
	uint8_t src_mac_valid[ETH_ALEN];
	uint8_t dst_mac_valid[ETH_ALEN];

	uint8_t src_mac[ETH_ALEN];
	uint8_t dst_mac[ETH_ALEN];
	/* IP address fields should be host order */
	uint32_t src_ip[4];
	uint32_t dst_ip[4];
	uint8_t dscp;		/* 6-bit value */
#define FPI_DSCP_MAX	0x3f
	uint16_t src_port;
	uint16_t dst_port;
	uint32_t esp_spi;
	uint8_t ipsec_tun_id;
	uint32_t gre_hdr;
	uint8_t outer_l2hdr[ETH_HLEN + VLAN_HLEN * 2];
	uint8_t encap_ip_hdr[40];	/* up to 40 byte for outer IPv6 Hdr */
	uint8_t egress_priority;	/* 3-bit for Queue of non-WLAN
					 * interface types; 1-bit for WLAN
					 * interface congestion control */
	uint16_t max_ingress_packet_size;
	uint16_t max_ip_packet_size;	/* mainly used for IP fragmentation */
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

typedef struct {
	struct {
		uint32_t active;
		uint32_t activates;
		uint32_t deactivates;
		uint32_t failures_activate;
		uint32_t failures_deactivate;
	} flow;
	struct {
		uint32_t active;
		uint32_t activates;
		uint32_t deactivates;
		uint32_t failures_activate;
		uint32_t failures_deactivate;
	} ipsec_tun;
	struct {
		uint32_t packets_received;
		uint32_t fragments_transmitted;
		uint32_t no_buffer_drop;
	} ip_frag;
	struct {
		uint32_t fragments_received;
		uint32_t packets_reassembled;
		uint32_t too_many_fragments_trap;
		uint32_t packet_length_too_big_trap;
		uint32_t buffer_type_mismatch_trap;
		uint32_t no_buffer_trap;
		uint32_t no_available_ctx_trap;
		uint32_t flow_miss_trap;
		uint32_t timeout_trap;
	} ip_reasm;
	struct {
		uint32_t outresrc;
		uint32_t dupl;
		uint32_t reject;
		uint32_t invalid;
		uint32_t notfound;
		uint32_t wlnotfound;
		uint32_t misc;
	} err;
} fpi_global_stat_t;

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

typedef struct {
	int fwd;  /* number of fwd / udp flows associated with the IPsec tunnel */
	int crypto; /* number of crypto flows associated with the IPsec tunnel */
} fpi_ipsec_flows_t;

typedef struct {
	char tun_dev_name[IFNAMSIZ]; /* name is used for userspace */
	uint64_t tun_device_ptr;
	uint32_t esp_spi;
	uint32_t new_spi;
	uint8_t is_us;
	uint8_t is_ipv6;
} fpi_ipsec_tunnel_t;

#endif /* __FPI_COMMON_H_INCLUDED__ */
