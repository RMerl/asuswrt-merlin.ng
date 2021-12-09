/*
    <:copyright-BRCM:2014-2016:DUAL/GPL:standard
    
       Copyright (c) 2014-2016 Broadcom 
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

#ifndef _RDD_DEFS_H
#define _RDD_DEFS_H

#include "rdpa_types.h"
#include "rdpa_ip_class_basic.h"
#include "rdpa_cpu_basic.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_map_auto.h"
#if defined(BCM63158)
#include "rdd_data_structures_auto.h"
#endif

#if defined(LINUX_KERNEL) || defined(__KERNEL__)
#include <linux/kallsyms.h>
#endif

#define ADDRESS_OF(image, task_name) image##_##task_name

#define RDD_LAYER2_HEADER_MINIMUM_LENGTH                        14
#define RDD_MAX_WAN_FLOW                                        255
#define RDD_NUM_OF_WAN_FLOWS                                    256
#define RDD_LOG2_NUM_OF_WAN_FLOWS                               8
#define RX_FLOW_CONTEXTS_NUMBER                                 (256 + 64)
#define TX_FLOW_CONTEXTS_NUMBER                                 RX_FLOW_CONTEXTS_NUMBER
#define IPTV_CTX_ENTRY_IDX_NULL                                 0x0
#define RDD_PACKET_HEADROOM_OFFSET                              18
#define RDD_CONTEXT_TABLE_SIZE                                  (64 * 1024)

#define THREAD_WAKEUP_REQUEST(x)                                (((x) << 4) + 1)

#define RDD_PHYS_PORT_WAN_PON rdpa_emac__num_of

typedef uint32_t rdd_vport_id_t;
typedef uint32_t rdd_emac_id_vector_t;
typedef unsigned long long rdd_vport_vector_t;
typedef uint32_t rdd_wan_channel_id_t;

typedef enum
{
    rdd_wan_gpon  = 0,
    rdd_wan_epon  = 1,
    rdd_wan_ae    = 2,
}
rdd_wan_mode_t;

typedef enum
{
    RDD_TX_QUEUE_0 = 0,
    RDD_TX_QUEUE_1 = 1,
    RDD_TX_QUEUE_2 = 2,
    RDD_TX_QUEUE_3 = 3,
    RDD_TX_QUEUE_4 = 4,
    RDD_TX_QUEUE_5 = 5,
    RDD_TX_QUEUE_6 = 6,
    RDD_TX_QUEUE_7 = 7,
    RDD_TX_QUEUE_LAST = 7,
    RDD_TX_QUEUE_NUMBER = RDD_TX_QUEUE_LAST + 1,
} rdd_tx_queue_id_t;

typedef struct
{
    uint32_t  rate;
    uint32_t  limit;
} rdd_rate_limit_params_t;

typedef struct
{
    uint32_t sustain_budget;
    rdd_rate_limit_params_t peak_budget;
    uint32_t peak_weight;
} rdd_rate_cntrl_params_t;

typedef enum
{
    RDD_RATE_LIMITER_PORT_0 = 0,
    RDD_RATE_LIMITER_PORT_1 = 1,
    RDD_RATE_LIMITER_PORT_2 = 2,
    RDD_RATE_LIMITER_PORT_3 = 3,
    RDD_RATE_LIMITER_PORT_4 = 4,
    RDD_RATE_LIMITER_PORT_5 = 5,
    RDD_RATE_LIMITER_PORT_6 = 6,
    RDD_RATE_LIMITER_SERVICE_QUEUE_0 = 6,
    RDD_RATE_LIMITER_PORT_7 = 7,
    RDD_RATE_LIMITER_PORT_8 = 8,
    RDD_RATE_LIMITER_PORT_9 = 9,
    RDD_RATE_LIMITER_PORT_10 = 10,
    RDD_RATE_LIMITER_PORT_11 = 11,
    RDD_RATE_LIMITER_PORT_12 = 12,
    RDD_RATE_LIMITER_PORT_13 = 13,
    RDD_RATE_LIMITER_SERVICE_QUEUE_7 = 13,
    RDD_RATE_LIMITER_PORT_14 = 14,
    RDD_RATE_LIMITER_SERVICE_QUEUE_OVERALL = 14,
    RDD_RATE_LIMITER_PORT_15 = 15,
    RDD_RATE_LIMITER_PORT_LAST = 15,
    RDD_RATE_LIMITER_DISABLED = 16,
} rdd_rate_limiter_t;

typedef enum
{
    RDD_QUEUE_PROFILE_0 = 0,
    RDD_QUEUE_PROFILE_1,
    RDD_QUEUE_PROFILE_2,
    RDD_QUEUE_PROFILE_3,
    RDD_QUEUE_PROFILE_4,
    RDD_QUEUE_PROFILE_5,
    RDD_QUEUE_PROFILE_6,
    RDD_QUEUE_PROFILE_7,
    RDD_QUEUE_PROFILE_DISABLED = 8,
} rdd_queue_profile_id_t;

typedef enum
{
    RDD_TPID_ID_0 = 0,
    RDD_TPID_ID_1,
    RDD_TPID_ID_2,
    RDD_TPID_ID_3,
    RDD_TPID_ID_4,
    RDD_TPID_ID_5,
    RDD_TPID_ID_6,
    RDD_TPID_ID_7,
} rdd_tpid_id_t;
/* alias for backward compatibility */
typedef rdd_tpid_id_t rdd_tpid_id;

typedef struct
{
    uint32_t min_threshold;
    uint32_t max_threshold;
    uint32_t max_drop_probability;
} rdd_prio_class_thresholds_t;

typedef struct
{
    rdd_prio_class_thresholds_t high_priority_class;
    rdd_prio_class_thresholds_t low_priority_class;
    bdmf_boolean us_flow_control_mode; /* 0 for disabled, 1 for enabled */
} rdd_queue_profile_t;

typedef enum
{
    RDD_FC_FWD_ACTION_CPU = 0,
    RDD_FC_FWD_ACTION_DROP,
} rdd_fc_fwd_action_t;

#if !defined(BCM63158)
#define RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE 32
typedef struct
{
    bdmf_index conn_index;

    rdpa_fc_action_vec_t actions_vector;
    rdd_fc_fwd_action_t fwd_action; /* In use when forward action action is turned on in action vector */
    rdpa_cpu_reason trap_reason; /* CPU trap reason in case forwarding action is ::rdpa_forward_action_host
                                    and ::rdpa_fc_action_forward is set. */
    bdmf_boolean service_queue_enabled;
    bdmf_index service_queue_id;
    rdpa_qos_method	qos_method;

    bdmf_ip_family ip_version;
    uint16_t nat_port;
    bdmf_ip_t nat_ip;
    uint8_t ds_lite_hdr_index;

    uint8_t ovid_offset;
    uint8_t opbit_action;
    uint8_t ipbit_action;
    rdpa_dscp dscp_value; /* DSCP value if ::rdpa_fc_action_dscp_remark is set. */
    uint8_t ecn_value;
    uint8_t policer_id;

    uint32_t ip_checksum_delta;
    uint32_t l4_checksum_delta;
    uint8_t phy_egress_port;

    uint8_t traffic_class;
    /* For WAN/LAN egress, mutual exclusive with wl_metadata */
    uint8_t tx_flow;
    uint8_t to_lan;
    uint8_t rate_controller;

    uint8_t ssid; /* For WLAN usage (where tx_flow is cpu_vport for wlan) */
    union {
        uint32_t wl_metadata;
        rdpa_wfd_t wfd;
        rdpa_rnr_t rnr;
    };

    int8_t l2_hdr_offset;
    uint8_t l2_hdr_size;
    uint8_t l2_hdr_number_of_tags;
    uint8_t l2_header[RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE];

    uint8_t drop_eligibility;              /* drop_eligibility bit 0: 0=non drop eligible class high, 1=drop eligible low, bit 1: enabler */

    /* Statistics (read-only) */
    rdpa_stat_t valid_cnt;
} rdd_fc_context_t;
#else
typedef union
{
    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_DTS fc_ucast_flow_context_eth_xtm_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_DTS fc_ucast_flow_context_wfd_nic_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS         fc_ucast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS fc_ucast_flow_context_rnr_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS fc_ucast_flow_context_wfd_dhd_entry;
} rdd_fc_context_t;

typedef enum
{
    rdd_egress_phy_eth_lan = 0,
    rdd_egress_phy_eth_wan,
    rdd_egress_phy_gpon = rdd_egress_phy_eth_wan, /* FIXME - once multi-wan is supported ; FW only has 2 bits*/ 
    rdd_egress_phy_dsl,
    rdd_egress_phy_wlan,
    rdd_egress_phy_max
} rdd_egress_phy_t;
#endif

typedef struct
{
    rdpa_ip_flow_key_t *lookup_entry;
    rdd_fc_context_t context_entry;
    uint32_t entry_index;
} rdd_ip_flow_t;

typedef enum
{
    RDD_MAC_FWD_ACTION_FORWARD = 0,
    RDD_MAC_FWD_ACTION_DROP,
    RDD_MAC_FWD_ACTION_CPU_TRAP0,
    RDD_MAC_FWD_ACTION_CPU_TRAP1,
    RDD_MAC_FWD_ACTION_CPU_TRAP2,
    RDD_MAC_FWD_ACTION_CPU_TRAP3,
    RDD_MAC_FWD_ACTION_RATE_LIMIT,
} rdd_mac_fwd_action_t;

/* tm - common definitions */
#define EXPONENT_LIST_LEN       4
#define MANTISSA_LEN            14
#define EXPONENT_LEN            2
#define RL_MAX_BUCKET_SIZE      (((1 << MANTISSA_LEN) - 1) << exponent_list[EXPONENT_LIST_LEN - 1])

typedef uint8_t quantum_number_t;

typedef enum
{
    RDD_SCHED_TYPE_BASIC,
    RDD_SCHED_TYPE_COMPLEX,
} rdpa_rdd_sched_type_t;

typedef enum
{
    RDD_RL_TYPE_BASIC,
    RDD_RL_TYPE_COMPLEX,
} rdpa_rdd_rl_type_t;

typedef struct
{
    uint8_t exponent;
    uint16_t mantissa;
} rdd_rl_float_t;

/* find the first core of module_idx */
#ifdef XRDP_EMULATION
int get_runner_idx(rdp_runner_image_e module_idx);
#else
static inline int get_runner_idx(rdp_runner_image_e module_idx)
{
    int i;

    for (i = 0; i < NUM_OF_RUNNER_CORES; ++i)
        if (rdp_core_to_image_map[i] == module_idx)
            return i;

    return NUM_OF_RUNNER_CORES;
}
#endif

/* returns the i'th bit in a vector */
static inline int get_location_of_bit_in_mask(uint8_t idx, uint32_t mask)
{
    uint8_t i, count = 0;

    for (i = 0; i < 32; ++i)
    {
        if (mask & 1)
            count++;
        if (idx == count)
            return i;
        mask = mask >> 1;
    }
    return (-1);
}

/* count the number of bits in a vector */
static inline uint8_t asserted_bits_count_get(uint32_t mask)
{
    uint8_t i, count = 0;

    for (i = 0; i < 32; ++i)
    {
        if (mask & 1)
            count++;
        mask = mask >> 1;
    }
    return count;
}

static inline rdd_rl_float_t rdd_rate_limiter_get_floating_point_rep(uint32_t fixed_point, uint32_t *exponent_list)
{
    rdd_rl_float_t floating_point = {};
    uint32_t  i;

    for (i = EXPONENT_LIST_LEN - 1; i > 0; i--)
    {
        if (fixed_point > (((1 << MANTISSA_LEN) - 1) << exponent_list[i - 1]))
        {
            floating_point.exponent = i;
            break;
        }
    }
    floating_point.mantissa = fixed_point >> exponent_list[floating_point.exponent];
    if ((exponent_list[floating_point.exponent] > 0) && ((fixed_point >> (exponent_list[floating_point.exponent] - 1)) & 1))
            floating_point.mantissa++;

    return floating_point;
}

static inline uint32_t rdd_rate_to_alloc_unit(uint32_t rate, uint32_t period)
{
    return ((rate + ((1000000 / period) / 2)) / (1000000 / period));
}

typedef struct
{
    uint32_t good_tx_packet;
    uint16_t error_tx_packets_discard;
} rdd_service_queue_pm_counters_t;

typedef struct
{
    uint32_t rx_packets;
    uint32_t tx_packets;
    uint32_t rx_bytes;
    uint32_t tx_bytes;
    uint32_t crc_err;
    uint32_t rx_drop_pkt;
} rdd_vport_pm_counters_t;

typedef struct
{
    uint8_t to_lan;
    uint8_t tx_flow;                  /**< US: Gem Flow or LLID ; DS : VPORT */
    rdd_vport_id_t egress_port;
    rdpa_qos_method qos_method;
    uint8_t priority;
    rdpa_forward_action action;       /**< forward/drop/cpu */
} rdd_ic_context_t;

#if defined(USE_BDMF_SHELL) && !defined(XRDP_EMULATION)
extern uint32_t g_rdd_trace;

#define RDD_TRACE(fmt, args...) \
    do { \
        if (g_rdd_trace) \
            bdmf_trace("TRACE: %s#%d : " fmt, __FUNCTION__, __LINE__, ## args); \
    } while(0)

#if defined(LINUX_KERNEL) || defined(__KERNEL__)
#define RDD_BTRACE_FMT "TRACE: %s#%d (<-- %pS): "
#else
#define RDD_BTRACE_FMT "TRACE: %s#%d (<-- %p): "
#endif

#define RDD_BTRACE(fmt, args...) \
    do { \
        void *ra1 = __builtin_return_address(0); \
        if (g_rdd_trace) \
            bdmf_trace(RDD_BTRACE_FMT fmt, __FUNCTION__, __LINE__, (void *)ra1, ## args); \
    } while(0)

#else
#define RDD_TRACE(fmt, args...) do { } while (0)
#define RDD_BTRACE(fmt, args...) do { } while (0)
#endif

#endif


