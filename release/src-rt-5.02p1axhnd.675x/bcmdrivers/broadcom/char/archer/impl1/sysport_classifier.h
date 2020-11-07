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
* File Name  : sysport_classifier.h
*
* Description: This file contains the System Port Classifier Definitions
*
*******************************************************************************
*/

#ifndef __SYSPORT_CLASSIFIER_H__
#define __SYSPORT_CLASSIFIER_H__

#include "cmdlist_api.h"

#define SYSPORT_TUPLE_HASH_WIDTH         16 // bits

#define SYSPORT_UCAST_FLOW_HASH_WIDTH    14 // less than, or equal to SYSPORT_TUPLE_HASH_WIDTH
#define SYSPORT_UCAST_FLOW_HASH_MASK     ((1 << SYSPORT_UCAST_FLOW_HASH_WIDTH) - 1)
#define SYSPORT_UCAST_FLOW_HASH_SHIFT    (SYSPORT_TUPLE_HASH_WIDTH - SYSPORT_UCAST_FLOW_HASH_WIDTH)
#define SYSPORT_UCAST_FLOW_HASH_LSB_MASK ((1 << SYSPORT_UCAST_FLOW_HASH_SHIFT) - 1)
#define SYSPORT_UCAST_FLOW_MAX           (1 << SYSPORT_UCAST_FLOW_HASH_WIDTH)
#define SYSPORT_UCAST_FLOW_BUCKET_SIZE   4
#define SYSPORT_UCAST_FLOW_INDEX_MAX     (SYSPORT_UCAST_FLOW_MAX * SYSPORT_UCAST_FLOW_BUCKET_SIZE)

#define SYSPORT_MCAST_FLOW_HASH_WIDTH    12 /* SYSPORT_UCAST_FLOW_HASH_WIDTH -
                                               LOG2(SYSPORT_MCAST_FLOW_BUCKET_SIZE / SYSPORT_UCAST_FLOW_BUCKET_SIZE) */
#define SYSPORT_MCAST_FLOW_HASH_SHIFT    (SYSPORT_TUPLE_HASH_WIDTH - SYSPORT_MCAST_FLOW_HASH_WIDTH)
#define SYSPORT_MCAST_FLOW_HASH_LSB_MASK ((1 << SYSPORT_MCAST_FLOW_HASH_SHIFT) - 1)
#define SYSPORT_MCAST_FLOW_MAX           (1 << SYSPORT_MCAST_FLOW_HASH_WIDTH)
#define SYSPORT_MCAST_FLOW_BUCKET_SIZE   16 // larger than, or equal to SYSPORT_UCAST_FLOW_BUCKET_SIZE
#define SYSPORT_MCAST_FLOW_INDEX_MAX     (SYSPORT_MCAST_FLOW_MAX * SYSPORT_MCAST_FLOW_BUCKET_SIZE)

#if defined(CONFIG_BCM963178)
#define SYSPORT_FLOW_LAN_PORTS_MAX       6
#define SYSPORT_FLOW_WLAN_PORTS_MAX      2
#endif
#if defined(CONFIG_BCM947622)
#if defined(CONFIG_BCM_HND_EAP)
/* Some EAP customers have designs w/ 6 LAN ports. */
#define SYSPORT_FLOW_LAN_PORTS_MAX       6
#define SYSPORT_FLOW_WLAN_PORTS_MAX      3
#else
#define SYSPORT_FLOW_LAN_PORTS_MAX       5
#define SYSPORT_FLOW_WLAN_PORTS_MAX      3
#endif
#endif
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM94908)
#define SYSPORT_FLOW_LAN_PORTS_MAX       5
#define SYSPORT_FLOW_WLAN_PORTS_MAX      3
#endif
#define SYSPORT_FLOW_WLAN_PORT_0         SYSPORT_FLOW_LAN_PORTS_MAX
#define SYSPORT_FLOW_EGRESS_QUEUES_MAX   8
#define SYSPORT_FLOW_CMDLIST_LENGTH_16_MAX  7
#define SYSPORT_FLOW_WLAN_SSID_MAX       9
#define SYSPORT_FLOW_WLAN_QUEUES_MAX     2

#if defined(CONFIG_BCM_ARCHER_SIM)
#define SYSPORT_FLOW_PORTS_MAX           4
#else
#define SYSPORT_FLOW_PORTS_MAX           (SYSPORT_FLOW_LAN_PORTS_MAX + SYSPORT_FLOW_WLAN_PORTS_MAX)
#endif

#define SYSPORT_IP_ADDR_TABLE_SIZE       4
#define SYSPORT_IP_ADDR_TABLE_INVALID    SYSPORT_IP_ADDR_TABLE_SIZE

#define SYSPORT_CMDLIST_INDEX_INVALID    0xFFFF

#define SYSPORT_CLASSIFIER_PATHSTAT_MAX  64

typedef struct {
    uint32_t packets;
    uint32_t bytes;
} sysport_classifier_pathstat_t;

typedef struct {
    union {
        struct {
            uint32_t ip_src_addr; // Source IPv4 Address / CRC32 of Source IPv6 Address
            uint32_t ip_dst_addr; // Destination IPv4 Address / CRC32 of Destination IPv6 Address
        };
        uint64_t u64;
    };
    int ref_count;
} sysport_ip_addr_table_t;

// The System Port SPE requires command lists to be stored back to back in DDR
typedef struct {
    uint32_t cmdlist[CMDLIST_CMD_LIST_SIZE_MAX_32];
} sysport_cmdlist_table_t;

typedef struct {
    uint32_t ref_count;
    uint8_t cmdlist_length; // see CMDLIST_CMD_LIST_SIZE_MAX
    uint8_t cmdlist_length_16;
} sysport_cmdlist_ctrl_table_t;

typedef union {
    struct {
#if defined(CONFIG_CPU_BIG_ENDIAN)
        uint32_t fhw_reserved : 2;
        uint32_t reserved     : 8;
        uint32_t flow_type    : 2;  // sysport_rsb_flow_type_t
        uint32_t bucket_index : 4;  // MAX(SYSPORT_UCAST_FLOW_BUCKET_SIZE, SYSPORT_MCAST_FLOW_BUCKET_SIZE)
        uint32_t table_index  : 16; // MAX(SYSPORT_UCAST_FLOW_MAX, SYSPORT_MCAST_FLOW_MAX)
#elif defined(CONFIG_CPU_LITTLE_ENDIAN) || defined(CONFIG_ARM)
        uint32_t table_index  : 16; // MAX(SYSPORT_UCAST_FLOW_MAX, SYSPORT_MCAST_FLOW_MAX)
        uint32_t bucket_index : 4;  // MAX(SYSPORT_UCAST_FLOW_BUCKET_SIZE, SYSPORT_MCAST_FLOW_BUCKET_SIZE)
        uint32_t flow_type    : 2;  // sysport_rsb_flow_type_t
        uint32_t reserved     : 8;
        uint32_t fhw_reserved : 2;
#endif
    };
    uint32_t u32;
} sysport_flow_key_t;

typedef union {
    struct {
        uint32_t packets;
        uint32_t bytes;
    };
    uint64_t u64;
} sysport_classifier_flow_stats_t;

typedef union {
    struct {
        struct {
            uint16_t cmdlist_length_16 : 3;  // 0 = 128 bytes, see SYSPORT_FLOW_CMDLIST_LENGTH_16_MAX
            uint16_t egress_queue      : 4;  // SYSPORT_FLOW_EGRESS_QUEUES_MAX
            uint16_t wlan_ssid_vector  : 9;  // WLAN MCAST Only, see SYSPORT_FLOW_WLAN_SSID_MAX
        };
        uint16_t cmdlist_index;
    };
    uint32_t u32;
} sysport_classifier_flow_port_t;

typedef int (*sysport_classifier_dev_xmit) (void *buffer_p, void *dev_p);

typedef struct {
    sysport_classifier_flow_stats_t stats;

    union {
        struct {
            uint32_t reserved_0          : 18 - SYSPORT_FLOW_PORTS_MAX;
            uint32_t pathstat_index      : 6;  // SYSPORT_CLASSIFIER_PATHSTAT_MAX
            uint32_t drop_profile        : 1;  // Drop Profile Index, ARCHER_DROP_PROFILE_MAX
            uint32_t dpi_queue           : 5;
            uint32_t egress_phy          : 2;  // sysport_rsb_phy_t
            uint32_t egress_port_or_mask : SYSPORT_FLOW_PORTS_MAX;  // UCAST=Port, MCAST=Port Mask
        };
        uint32_t u32_0;
    };

    union {
        struct {
            uint32_t drop           : 1;
            uint32_t mtu            : 11;
            uint32_t is_routed      : 1;
            uint32_t check_ip_df    : 1; // For MAP-T US packets, check IPv4 DF flag
            uint32_t expected_ip_df : 1;
            uint32_t check_tos      : 1;
            uint32_t expected_tos   : 8;
            uint32_t ip_addr_index  : 3; // SYSPORT_IP_ADDR_TABLE_INVALID
            uint32_t wlan_priority  : 4; // WLAN MCAST Only, shared across all ports
            uint32_t tcp_pure_ack   : 1;
        };
        uint32_t u32_1;
    };

    union {
        sysport_classifier_flow_port_t port[SYSPORT_FLOW_PORTS_MAX];
        wlFlowInf_t wfd; // WLAN UCAST Only, fist WLAN Port is SYSPORT_FLOW_WLAN_PORT_0
    };

#if defined(CONFIG_BCM_ARCHER_SIM)
    sysport_classifier_dev_xmit dev_xmit;
    void *tx_dev_p;
#endif
} sysport_classifier_flow_context_t;

typedef struct {
    sysport_rsb_flow_tuple_t tuple;
    sysport_classifier_flow_context_t context;
} sysport_classifier_flow_t;

typedef struct {
    int found;
    int shown;
} sysport_classifier_flow_dump_stats_t;

typedef struct {
    // Input
    sysport_rsb_flow_type_t flow_type;
    int max;
    // Output
    sysport_classifier_flow_dump_stats_t stats[SYSPORT_RSB_FLOW_TYPE_MAX];
} sysport_classifier_flow_dump_t;

#define SYSPORT_CLASSIFIER_UCAST_L2_IP_TOS_UNKNOWN  0xFF

#define SYSPORT_CLASSIFIER_OVERWRITE_INVALID        (-1)

#define SYSPORT_CLASSIFIER_OVERWRITE_IS_VALID(_rsb_overwrite_data) \
    (_rsb_overwrite_data != SYSPORT_CLASSIFIER_OVERWRITE_INVALID)

typedef struct {
    struct {
        int ingress_port;
        int valid;
    } header;
    struct {
        int ip_tos;
    } ucast_l2;
} sysport_classifier_rsb_overwrite_data_t;

typedef struct {
    sysport_classifier_rsb_overwrite_data_t parser;
    sysport_classifier_rsb_overwrite_data_t flow;
} sysport_classifier_rsb_overwrite_t;

static inline void sysport_classifier_rsb_overwrite_init(sysport_classifier_rsb_overwrite_t *rsb_overwrite_p)
{
    memset(rsb_overwrite_p, 0xFF, sizeof(sysport_classifier_rsb_overwrite_t));
}

static inline int sysport_classifier_port_mask_to_port(int port_mask)
{
    int port = ffs(port_mask);

    return port - 1;
}

#define SYSPORT_CLASSIFIER_ERROR_INVALID           -1  // Invalid argument
#define SYSPORT_CLASSIFIER_ERROR_CMDLIST_INIT      -2  // Command List Table is Full
#define SYSPORT_CLASSIFIER_ERROR_CMDLIST_FULL      -3  // Command List Table is Full
#define SYSPORT_CLASSIFIER_ERROR_CMDLIST_INTERNAL  -4  // Command List Internal Error
#define SYSPORT_CLASSIFIER_ERROR_FLOW_INIT         -5  // Flow Table is Full
#define SYSPORT_CLASSIFIER_ERROR_FLOW_FULL         -6  // Flow Table is Full
#define SYSPORT_CLASSIFIER_ERROR_FLOW_NOT_FOUND    -7  // Flow Table is Full
#define SYSPORT_CLASSIFIER_ERROR_IPADDR_FULL       -8  // IP Address Table is Full

#define SYSPORT_CLASSIFIER_ERROR_OVERFLOW(_ret)                    \
    ( SYSPORT_CLASSIFIER_ERROR_CMDLIST_FULL == (_ret) ||           \
      SYSPORT_CLASSIFIER_ERROR_FLOW_FULL == (_ret) ||              \
      SYSPORT_CLASSIFIER_ERROR_IPADDR_FULL == (_ret) )

int sysport_classifier_flow_find(sysport_classifier_flow_t *user_flow_p,
                                 sysport_flow_key_t *flow_key_p,
                                 sysport_classifier_rsb_overwrite_t *rsb_overwrite_p);

int sysport_classifier_flow_get(sysport_flow_key_t flow_key,
                                sysport_classifier_flow_t *user_flow_p);

int sysport_classifier_flow_create(sysport_classifier_flow_t *user_flow_p,
                                   sysport_rsb_flow_tuple_info_t *user_flow_info_p,
                                   uint32_t *cmdlist_p, int cmdlist_length,
                                   sysport_flow_key_t *flow_key_p,
                                   sysport_classifier_rsb_overwrite_t *rsb_overwrite_p);

int sysport_classifier_flow_port_get(sysport_flow_key_t flow_key, int egress_port,
                                     sysport_classifier_flow_port_t *user_port_p);

int sysport_classifier_flow_port_set(sysport_flow_key_t flow_key, int egress_port,
                                     sysport_classifier_flow_port_t *user_port_p);

int sysport_classifier_flow_port_add(sysport_flow_key_t flow_key, int egress_port,
                                     sysport_classifier_flow_port_t *user_port_p,
                                     int mtu, uint32_t *cmdlist_p, int cmdlist_length);

int sysport_classifier_flow_port_delete(sysport_flow_key_t flow_key, int egress_port);

int sysport_classifier_flow_dpi_queue_set(sysport_flow_key_t flow_key, unsigned int dpi_queue);

int sysport_classifier_flow_dpi_priority_set(sysport_flow_key_t flow_key, int flow_id, unsigned int priority);

int sysport_classifier_flow_delete(sysport_flow_key_t flow_key);

int sysport_classifier_flow_delete_all(void);

int sysport_classifier_flow_stats_get(sysport_flow_key_t flow_key,
                                      sysport_classifier_flow_stats_t *stats_p);

int sysport_classifier_flow_stats_reset(sysport_flow_key_t flow_key);

int sysport_classifier_pathstat_get(unsigned int pathstat_index,
                                    sysport_classifier_pathstat_t *pathstat_p);

int sysport_classifier_ip_addr_table_add(sysport_ip_addr_table_t *ip_addr_p,
                                         int *ip_addr_index_p);

int sysport_classifier_ip_addr_table_delete(int ip_addr_index);

void sysport_classifier_flow_dump(sysport_classifier_flow_t *flow_p,
                                  sysport_rsb_flow_tuple_info_t *flow_info_p);

void sysport_classifier_flow_table_dump(sysport_classifier_flow_dump_t *dump_p);

void sysport_classifier_packet_dump(void *packet_p, int packet_length, int has_brcmtag);

void sysport_classifier_tcp_pure_ack_enable(int enable);

int __init sysport_classifier_construct(void);
void __exit sysport_classifier_destruct(void);

#endif  /* __SYSPORT_CLASSIFIER_H__ */
