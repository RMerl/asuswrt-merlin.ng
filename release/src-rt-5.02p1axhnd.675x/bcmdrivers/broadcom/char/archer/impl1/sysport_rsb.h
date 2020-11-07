/*
  <:copyright-BRCM:2017:proprietary:standard

  Copyright (c) 2017 Broadcom 
  All Rights Reserved

  This program is the proprietary software of Broadcom and/or its
  licensors, and may only be used, duplicated, modified or distributed pursuant
  to the terms and conditions of a separate, written license agreement executed
  between you and Broadcom (an "Authorized License").  Except as set forth in
  an Authorized License, Broadcom grants no license (express or implied), right
  to use, or waiver of any kind with respect to the Software, and Broadcom
  expressly reserves all rights in and to the Software and all intellectual
  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

  Except as expressly set forth in the Authorized License,

  1. This program, including its structure, sequence and organization,
  constitutes the valuable trade secrets of Broadcom, and you shall use
  all reasonable efforts to protect the confidentiality thereof, and to
  use this information only in connection with your use of Broadcom
  integrated circuit products.

  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
  ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
  FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
  COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
  PERFORMANCE OF THE SOFTWARE.

  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
  ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
  WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
  IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
  OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
  SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
  SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
  LIMITED REMEDY.
  :> 
*/

/*
*******************************************************************************
* File Name  : sysport_rsb.h
*
* Description: This file contains the System Port RSB structure definition.
*
*******************************************************************************
*/

#ifndef __SYSPORT_RSB_H__
#define __SYSPORT_RSB_H__

/******************************************
 * RSB Definitions (little-endian format)
 ******************************************/

#define SYSPORT_RSB_VLAN_TAGS_MAX   7

typedef enum {
    SYSPORT_RSB_FLOW_TYPE_UNKNOWN = 0,
    SYSPORT_RSB_FLOW_TYPE_UCAST_L3,
    SYSPORT_RSB_FLOW_TYPE_UCAST_L2,
    SYSPORT_RSB_FLOW_TYPE_MCAST,
    SYSPORT_RSB_FLOW_TYPE_MAX
} sysport_rsb_flow_type_t;

typedef enum {
    SYSPORT_RSB_PHY_ETH_LAN = 0,
    SYSPORT_RSB_PHY_ETH_WAN,
    SYSPORT_RSB_PHY_DSL,
    SYSPORT_RSB_PHY_WLAN, // Only used for egress_phy
    SYSPORT_RSB_PHY_MAX
} sysport_rsb_phy_t;

typedef union {
    struct {
        uint8_t ingress_port : 3; // Ethernet LAN source port
        uint8_t ingress_phy : 2; // Source PHY, defined in sysport_rsb_phy_t
        uint8_t flow_type : 2; // Flow Type, defined in sysport_rsb_flow_type_t
        uint8_t valid : 1; // Hash Table = Valid Entry; RSB = Tuple was built successfully
    };
    uint8_t u8;
} sysport_rsb_flow_header_t;

// L3 Flow Classification Tuple
// - Destination MAC Address == Host MAC Address
// - In case of 4in6 and 6in4 tunnels, always generate from the innermost header
typedef struct {
    uint8_t ip_tos; // IPv4 ToS / IPv6 TC
    uint8_t reserved;
    uint8_t ip_protocol; // IP protocol Number (IPv4/IPv6)
    sysport_rsb_flow_header_t header;
    uint32_t ip_dst_addr; // Destination IPv4 Address / CRC32 of Destination IPv6 Address
    uint32_t ip_src_addr; // Source IPv4 Address / CRC32 of Source IPv6 Address
    union {
        struct {
            uint16_t dst_port; // TCP/UDP Destination port
            uint16_t src_port; // TCP/UDP Source port
        };
        uint32_t u32;
    } l4_ports;
} sysport_rsb_flow_ucast_l3_t; // 16 bytes

// L2 Flow Classification Tuple
// - Destination MAC Address != Host MAC Address
typedef struct {
    uint16_t ethertype;
    uint8_t ip_tos; // IPv4 ToS / IPv6 TC
    sysport_rsb_flow_header_t header;
    uint32_t dst_mac_crc32; // CRC32 of Destination MAC Address
    uint32_t src_mac_crc32; // CRC32 of Source MAC Address (optional)
    uint32_t vlan_tag_crc32; // CRC32 of the first two vlan tags (all fields)
} sysport_rsb_flow_ucast_l2_t; // 16 bytes

// Multicast Flow Classification Tuple
// - Multicast Flows are always identified based on ip_dst_addr range, NOT based on Destination MAC Address
typedef struct {
    uint8_t nbr_of_vlans; // Number of VLAN Tags (SYSPORT_RSB_VLAN_TAGS_MAX)
    uint8_t reserved;
    uint8_t ip_protocol; // IP protocol Number (IPv4/IPv6)
    sysport_rsb_flow_header_t header;
    uint32_t ip_dst_addr; // Destination IPv4 Address / CRC32 of Destination IPv6 Address
    uint32_t ip_src_addr; // Source IPv4 Address / CRC32 of Source IPv6 Address
    union {
        struct {
            uint32_t inner_vlan_id : 12; // VLAN ID of inner VLAN Tag (zero if nbr_of_vlans <= 1)
            uint32_t outer_vlan_id : 12; // VLAN ID of outer VLAN Tag (zero if nbr_of_vlans == 0)
            uint32_t reserved      : 8;
        };
        uint32_t u32;
    } vlan;
} sysport_rsb_flow_mcast_t; // 16 bytes

typedef union {
    struct {
        union {
            struct {
                struct {
                    uint16_t reserved_16;
                    uint8_t reserved_8;
                    sysport_rsb_flow_header_t header;
                };
                uint32_t u32_1;
            };
            uint64_t u64_0;
        };
        union {
            struct {
                uint32_t u32_2;
                uint32_t u32_3;
            };
            uint64_t u64_1;
        };
    };
    sysport_rsb_flow_ucast_l3_t ucast_l3;
    sysport_rsb_flow_ucast_l2_t ucast_l2;
    sysport_rsb_flow_mcast_t mcast;
} sysport_rsb_flow_tuple_t; // 16 bytes

typedef enum {
    SYSPORT_RSB_L3_TYPE_UNKNOWN = 0,
    SYSPORT_RSB_L3_TYPE_IPV4,
    SYSPORT_RSB_L3_TYPE_IPV6,
    SYSPORT_RSB_L3_TYPE_4IN6,
    SYSPORT_RSB_L3_TYPE_6IN4,
    SYSPORT_RSB_L3_TYPE_MAX
} sysport_rsb_l3_type_t;

typedef enum {
    SYSPORT_RSB_PKT_TYPE_UCAST = 0,
    SYSPORT_RSB_PKT_TYPE_BCAST,
    SYSPORT_RSB_PKT_TYPE_MCAST_IP,
    SYSPORT_RSB_PKT_TYPE_MCAST_L2,
    SYSPORT_RSB_PKT_TYPE_MAX
} sysport_rsb_pkt_type_t;

// RSB Structure - Stored in Rx buffer 32 bytes prior to SOP
typedef struct {
    union {
        struct {
            uint32_t mlt_index     : 12; // overlaps traffic_class, reason_code, and pkt_type
            uint32_t reserved      : 20;
        };
        struct {
            uint32_t traffic_class : 3;
            uint32_t reason_code   : 7;
            uint32_t pkt_type      : 2; // sysport_rsb_pkt_type_t
            uint32_t error         : 1;
            uint32_t overflow      : 1;
            uint32_t parse_fail    : 1;
            uint32_t l4_checksum   : 1;
            uint32_t sop           : 1;
            uint32_t eop           : 1;
            uint32_t data_length   : 14;
        };
        uint32_t rx_info;
    };

    uint32_t tunnel_ip_src_addr; // Source IPv4 Address / CRC32 of Source IPv6 Address of the outermost IP Header

    union {
        struct {
            uint16_t ip_length; // IPv4 Total Length / IPv6 Payload Length
            uint8_t ip_tos; // IPv4 ToS / IPv6 TC field
            uint8_t ip_offset; // IPv4/v6 Header Offset from SOP; When 4in6 and 6in4: offset of innermost IP Header
        };
        uint32_t ip_info;
    };

    uint16_t tuple_crc16; // CRC16 of tuple
    union {
        struct {
            // Headers found during packet parsing
            // Pre-classification checks
            uint16_t tcp_rst_syn_fin     : 1; // Set if TCP RST or SYN or FIN flags are set
            uint16_t ipv4_frag           : 1; // Set if IPv4 More Fragments Flag (MF) == 1 || IPv4 Fragment Offset != 0
            uint16_t ipv4_df             : 1;
            uint16_t ttl_expired         : 1; // Set if Packet IPv4 TTL / IPv6 Hop Limit <= 1
            uint16_t ipv4_options        : 1; // Set if IPv4 IHL != 5
            uint16_t ip_version_mismatch : 1; // Set if IPv4/v6 Version does not match preceeding L2 Type
            uint16_t udp                 : 1;
            uint16_t tcp                 : 1;
            uint16_t l3_type             : 3; // Defined in sysport_rsb_l3_type_t
            uint16_t llc_snap            : 1;
            uint16_t pppoe               : 1;
            uint16_t nbr_of_vlans        : 3; // Number of VLAN Tags (SYSPORT_RSB_VLAN_TAGS_MAX)
        };
        uint16_t flags;
    };

    sysport_rsb_flow_tuple_t tuple; // 16 bytes
} sysport_rsb_t; // 32 bytes

typedef union {
    struct {
        BlogEthAddr_t dst_mac;
        BlogEthAddr_t src_mac;
        uint8_t nbr_of_vlans;
        BlogVlanHdr_t vlan[SYSPORT_RSB_VLAN_TAGS_MAX];
    } l2;
    struct {
        uint8_t is_ipv6;
        struct {
            BlogIpv6Addr_t src_addr;
            BlogIpv6Addr_t dst_addr;
        } ipv6;
    } l3;
} sysport_rsb_flow_tuple_info_t;

extern const char *sysport_rsb_flow_type_name[SYSPORT_RSB_FLOW_TYPE_MAX];
extern const char *sysport_rsb_phy_name[SYSPORT_RSB_PHY_MAX];
extern const char *sysport_rsb_l3_type_name[SYSPORT_RSB_L3_TYPE_MAX];

void sysport_rsb_mem_dump(void *p, int length);
void sysport_rsb_tuple_dump(sysport_rsb_flow_tuple_t *tuple_p,
                            sysport_rsb_flow_tuple_info_t *tuple_info_p);
void sysport_rsb_dump(sysport_rsb_t *rsb_p, sysport_rsb_flow_tuple_info_t *tuple_info_p, int mlt_enable);

#endif  /* __SYSPORT_RSB_H__ */
