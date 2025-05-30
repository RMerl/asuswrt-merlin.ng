/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 *
 *    Copyright (c) 2013 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */


#ifndef _RDPA_TYPES_H_
#define _RDPA_TYPES_H_

/** \defgroup types Commonly used types and constants
 * @{
 */

#include <bdmf_data_types.h>

/** Traffic direction */
typedef enum
{
    rdpa_dir_ds,        /**< Downstream */
    rdpa_dir_us         /**< Upstream */
} rdpa_traffic_dir;

typedef int16_t sched_id_t; /* sched_id is mapped to scheduler index and type. A negative value indicates invalid sched_id */

/** Direction + index, Underlying structure for rdpa_dir_index aggregate */
typedef struct
{
    rdpa_traffic_dir dir;       /** Traffic direction */
    bdmf_index index;           /** Index */
} rdpa_dir_index_t;

/** PPPoE header */
typedef struct
{
    uint32_t session;   /**< PPPoE session ID */
} rdpa_pppoe_t;

/** PBIT */
typedef uint8_t rdpa_pbit;

/** DSCP */
typedef uint8_t rdpa_dscp;

/** DSCP pbit dei */
typedef struct
{
    uint8_t pbit;   /**< pbit */
    uint8_t dei;    /**< pbit */
} rdpa_pbit_dei_t;

/** Priority */
typedef uint8_t rdpa_prty;

/** TCONT index */
typedef uint8_t rdpa_tcont;

/** Dual Stack lite tunnel ID */
/* this enum is the only modification needed in order to add more than one tunnel */
typedef enum
{
    rdpa_ds_lite_tunnel_0,
    rdpa_ds_lite_max_tunnel_id = rdpa_ds_lite_tunnel_0
} rdpa_ds_lite_tunnel_id;

/** GEM flow ID (internal index) */
typedef uint16_t rdpa_gem;

/** Ethernet/GEM flow ID */
typedef uint16_t rdpa_flow;

#define GBE_WAN_FLOW_ID 0 /* not used by xDSL WAN Mode. flows 1-15 are used by DSL WAN modes for 16 channels in multiples upto 255.
                           *  Any other values would collide with the DSL WAN flows.
                           *  values 0x0, 0x10, 0x20... are unused. */


/** UNASSIGNED value */
#define RDPA_VALUE_UNASSIGNED ((unsigned)-1)
#define RDPA_VALUE_UNMATCHED RDPA_VALUE_UNASSIGNED

/** ANY value */
#define RDPA_VALUE_ANY ((unsigned)-2)

/** VID/PBIT/DEI */
#ifdef G9991_COMMON
#define RDPA_MAX_VID 4095
#else
#define RDPA_MAX_VID 4094
#endif
#define RDPA_MAX_PBIT 7
#define RDPA_MAX_DEI  1
#define RDPA_VID_MASK 0xFFF
#if !defined(RDP_UFC)
#define RDPA_VLAN_TCI_MASK 0xFFFF
#else
#define RDPA_VLAN_TCI_MASK 0xFFFFFFFF
#endif
/** Layer-4 PROTOCOL definitions */
#define RDPA_INVALID_PROTOCOL (0xFF)

#define TC_TO_QUEUE_US_TABLE_INDEX 1

/** TPID Detect.
 *  A set of TPID values, both pre- and user-defined,
 *  used to apply tagging classification rules on incoming
 *  traffic. Accordingly, every packet is classified as Single
 *  or Double tagged. */
typedef enum
{
    rdpa_tpid_detect_0x8100, /**< Pre-Defined, 0x8100 */
    rdpa_tpid_detect_0x88A8, /**< Pre-Defined, 0x88A8 */
    rdpa_tpid_detect_0x9100, /**< Pre-Defined, 0x9100 */
    rdpa_tpid_detect_0x9200, /**< Pre-Defined, 0x9200 */

    rdpa_tpid_detect_udef_1, /**< User-Defined, #1    */
    rdpa_tpid_detect_udef_2, /**< User-Defined, #2    */

    rdpa_tpid_detect__num_of
} rdpa_tpid_detect_t;


/** Forwarding action */
typedef enum
{
    rdpa_forward_action_none          = 0,    /** ACL */
    rdpa_forward_action_forward       = 1,    /**< Forward */
    rdpa_forward_action_host          = 2,    /**< Trap to the host */
    rdpa_forward_action_drop          = 4,    /**< Discard */
    rdpa_forward_action_flood         = 8,    /**< Flood, for DA lookup only */
    rdpa_forward_action_skip          = 16,   /**< Skip - used for generic filter for increment counter action only \XRDP_LIMITED */
    rdpa_forward_action_drop_low_pri  = 32    /**< drop_low_pri - drop only if ingress filters pass \XRDP_LIMITED */
} rdpa_forward_action;

/** Filtering action */
typedef enum {
    rdpa_filter_action_allow,   /**< Allow through */
    rdpa_filter_action_deny     /**< Block packet */
} rdpa_filter_action;

/** QoS mapping method */
typedef enum
{
    rdpa_qos_method_flow,        /**< Flow-based QoS mapping */
    rdpa_qos_method_pbit       /**< Pbit-based QoS mapping */
} rdpa_qos_method;

/** Allow frame types */
typedef enum
{
    rdpa_port_allow_any,        /**< Allow tagged and untagged frames */
    rdpa_port_allow_tagged,     /**< Allow tagged frames only */
    rdpa_port_allow_untagged,   /**< Allow untagged frames only */
} rdpa_port_frame_allow;

/** Forwarding mode */
typedef enum
{
    rdpa_forwarding_mode_pkt,   /**< Packet-based forwarding */
    rdpa_forwarding_mode_flow,  /**< Flow-based forwarding */
} rdpa_forwarding_mode;

/** DS Ethernet flow classification mode */
typedef enum
{
    rdpa_classify_mode_pkt,     /**< Packet-based classification */
    rdpa_classify_mode_flow,    /**< Flow-based classification */
} rdpa_classify_mode;

/** Discard priority */
typedef enum
{
    rdpa_discard_prty_low,      /**< Low priority for Ingress QoS: traffic dropped under high ingress congestion */
    rdpa_discard_prty_high      /**< High priority for Ingress QoS: traffic passed under high ingress congresion */
} rdpa_discard_prty;

/** Ingress Qos priority */
typedef enum
{
    rdpa_iq_priority_low,      /**< Low priority for Ingress QoS: traffic dropped under high ingress congestion */
    rdpa_iq_priority_high      /**< High priority for Ingress QoS: traffic passed under high ingress congresion */
} rdpa_iq_priority;

/** Flow destination */
typedef enum
{
    rdpa_flow_dest_none,        /**< Not set */

    rdpa_flow_dest_iptv,        /**< IPTV */
    rdpa_flow_dest_eth,         /**< Flow */
    rdpa_flow_dest_omci,        /**< OMCI */

    rdpa_flow_dest__num_of,     /* Number of values in rdpa_flow_destination enum */
} rdpa_flow_destination;

typedef enum
{
    rdpa_port_type_none,         /**< Not configured */
    rdpa_port_gpon,              /**< GPON */
    rdpa_port_xgpon,             /**< XGPON */
    rdpa_port_epon,              /**< EPON */
    rdpa_port_xepon,             /**< XEPON */
    rdpa_port_dsl,               /**< xDSL */
    rdpa_port_epon_ae,           /**< Active Ethernet */
    rdpa_port_emac,              /**< emac */
    rdpa_port_wlan,              /**< wlan */
    rdpa_port_gdx,               /**< gdx */
    rdpa_port_cpu,               /**< cpu */
    rdpa_port_sid,               /**< sid */
    rdpa_port_ctrl_sid,          /**< control sid */
    rdpa_port_sys_port,          /**< DPU - system port */
    rdpa_port_bond,              /**< bond */
    rdpa_port_sf2_emac,          /**< SF2 emac */
    rdpa_vport_sq,               /**< virtual service queue, cant create phisical port with this type */
    rdpa_port_type__num_of
} rdpa_port_type;

#define IS_RDPA_WAN_TYPE_EPON(_type) ((_type == rdpa_port_epon) || (_type == rdpa_port_xepon))
#define IS_RDPA_WAN_TYPE_GPON(_type) ((_type == rdpa_port_gpon) || (_type == rdpa_port_xgpon))

#define RDPA_PORT_CPU_HANDLE 0xFEFEFEFE

/** Link speed */
typedef enum
{
    rdpa_speed_none,            /**< Not configured */
    rdpa_speed_100m,            /**< Speed of 100 Mega */
    rdpa_speed_1g,              /**< Speed of 1 Giga */
    rdpa_speed_2_5g,            /**< Speed of 2.5 Giga */
    rdpa_speed_5g,              /**< Speed of 5 Giga */
    rdpa_speed_10g,             /**< Speed of 10 Giga */
} rdpa_speed_type;

/** Simple statistics */
typedef struct
{
    uint32_t packets;           /**< Packets */
/*TODO 64 bit */
    uint64_t bytes;             /**< Bytes */
} rdpa_stat_t;

static const rdpa_stat_t RDPA_STAT_NULL = {0, 0};

/** Stat type */
typedef enum
{
    rdpa_stat_packets_only = 0, /* default */
    rdpa_stat_packets_and_bytes = 1,
} rdpa_stat_type;

/** Generic 1-way statistics */
typedef struct
{
    rdpa_stat_t passed;         /**< Passed statistics */
    rdpa_stat_t discarded;      /**< Discarded statistics */
} rdpa_stat_1way_t;

/** Tx+Rx statistics */
typedef struct
{
    rdpa_stat_1way_t tx;        /**< Transmit statistics */
    rdpa_stat_1way_t rx;        /**< Receive statistics */
} rdpa_stat_tx_rx_t;

/** Tx+Rx statistics for passed packets + bytes  */
typedef struct
{
    rdpa_stat_t tx;             /**< Transmit statistics */
    rdpa_stat_t rx;             /**< Receive statistics */
} rdpa_stat_tx_rx_valid_t;

#define RDPA_VPORT_FIRST 0

/** EMAC id */
typedef enum
{
    rdpa_emac0,             /**< EMAC0 */
    rdpa_emac1,             /**< EMAC1 */
    rdpa_emac2,             /**< EMAC2 */
    rdpa_emac3,             /**< EMAC3 */
    rdpa_emac4,             /**< EMAC4 */
    rdpa_emac5,             /**< EMAC5 */
    rdpa_emac6,             /**< EMAC6 */
    rdpa_emac7,             /**< EMAC7 */
    rdpa_emac8,             /**< EMAC8 */
    rdpa_emac9,             /**< EMAC9 */
    rdpa_emac10,            /**< EMAC10 */
#if defined(BCM6888) || defined(BCM6837)
    rdpa_emac11,            /**< EMAC11 */
    rdpa_emac12,            /**< EMAC12 */
#if defined(BCM6888)
    rdpa_emac13,            /**< EMAC13 */
    rdpa_emac14,            /**< EMAC14 */
    rdpa_emac15,            /**< EMAC15 */
#endif
#endif
    rdpa_emac__num_of,      /* Max number of EMACs */
    rdpa_emac_none,      /**< Indicates virtual port */
} rdpa_emac;

/** EMAC mode */
typedef enum
{
    rdpa_emac_mode_sgmii,   /**< SGMII */
    rdpa_emac_mode_hisgmii, /**< HISGMII */
    rdpa_emac_mode_qsgmii,  /**< QSGMII */
    rdpa_emac_mode_ss_smii, /**< SS SMII */
    rdpa_emac_mode_rgmii,   /**< RGMII */
    rdpa_emac_mode_mii,     /**< MII */
    rdpa_emac_mode_tmii,    /**< TMII */

    rdpa_emac_mode__num_of, /* Number of EMAC modes */
} rdpa_emac_mode;


/** EMAC rates */
typedef enum
{
    rdpa_emac_rate_10m,     /**< 10 Mbps */
    rdpa_emac_rate_100m,    /**< 100 Mbps */
    rdpa_emac_rate_1g,      /**< 1 Gbps */
    rdpa_emac_rate_2_5g,    /**< 2.5 Gbps */

    rdpa_emac_rate__num_of, /* Number of rates */
} rdpa_emac_rate;

/** EMAC configuration */
typedef struct
{
    char loopback;      /**< 1 = line loopback */
    rdpa_emac_rate rate;       /**< EMAC rate */
    char generate_crc;  /**< 1 = generate CRC */
    char full_duplex;   /**< 1 = full duplex */
    char pad_short;     /**< 1 = pad short frames */
    char allow_too_long;/**< 1 = allow long frames */
    char check_length;  /**< 1 = check frame length */
    uint32_t preamble_length;   /**< Preamble length */
    uint32_t back2back_gap;     /**< Back2Back inter-packet gap */
    uint32_t non_back2back_gap; /**< Non Back2Back inter-packet gap */
    uint32_t min_interframe_gap; /**< Min inter-frame gap */
    char rx_flow_control;/**< 1 = enable RX flow control */
    char tx_flow_control;/**< 1 = enable TX flow control */
} rdpa_emac_cfg_t;

/** RX RMON counters.
 * Underlying type for emac_rx_stat aggregate type.
 */
typedef struct
{
    uint32_t byte;              /**< Receive Byte Counter */
    uint32_t packet;            /**< Receive Packet Counter */
    uint32_t frame_64;          /**< Receive 64 Byte Frame Counter */
    uint32_t frame_65_127;      /**< Receive 65 to 127 Byte Frame Counter */
    uint32_t frame_128_255;     /**< Receive 128 to 255 Byte Frame Counter */
    uint32_t frame_256_511;     /**< Receive 256 to 511 Byte Frame Counter */
    uint32_t frame_512_1023;    /**< Receive 512 to 1023 Byte Frame Counter */
    uint32_t frame_1024_1518;   /**< Receive 1024 to 1518 Byte Frame Counter */
    uint32_t frame_1519_mtu;    /**< Receive 1519 to MTU Frame Counter */
    uint32_t multicast_packet;  /**< Receive Multicast Packet */
    uint32_t broadcast_packet;  /**< Receive Broadcast Packet */
    uint32_t unicast_packet;    /**< Receive Unicast Packet */
    uint32_t alignment_error;   /**< Receive Alignment error */
    uint32_t frame_length_error;/**< Receive Frame Length Error Counter */
    uint32_t code_error;        /**< Receive Code Error Counter */
    uint32_t carrier_sense_error;/**< Receive Carrier sense error */
    uint32_t fcs_error;         /**< Receive FCS Error Counter */
    uint32_t control_frame;     /**< Receive Control Frame Counter */
    uint32_t pause_control_frame;/**< Receive Pause Control Frame */
    uint32_t unknown_opcode;    /**< Receive Unknown opcode */
    uint32_t undersize_packet;  /**< Receive Undersize Packet */
    uint32_t oversize_packet;   /**< Receive Oversize Packet */
    uint32_t fragments;         /**< Receive Fragments */
    uint32_t jabber;            /**< Receive Jabber counter */
    uint32_t overflow;          /**< Receive Overflow counter */
} rdpa_emac_rx_stat_t;

/** Tx RMON counters.
 * Underlying type for emac_tx_stat aggregate type.
 */
typedef struct
{
    uint32_t byte;              /**< Transmit Byte Counter */
    uint32_t packet;            /**< Transmit Packet Counter */
    uint32_t frame_64;          /**< Transmit 64 Byte Frame Counter */
    uint32_t frame_65_127;      /**< Transmit 65 to 127 Byte Frame Counter */
    uint32_t frame_128_255;     /**< Transmit 128 to 255 Byte Frame Counter */
    uint32_t frame_256_511;     /**< Transmit 256 to 511 Byte Frame Counter */
    uint32_t frame_512_1023;    /**< Transmit 512 to 1023 Byte Frame Counter */
    uint32_t frame_1024_1518;   /**< Transmit 1024 to 1518 Byte Frame Counter */
    uint32_t frame_1519_mtu;    /**< Transmit 1519 to MTU Frame Counter */
    uint32_t fcs_error;         /**< Transmit FCS Error */
    uint32_t multicast_packet;  /**< Transmit Multicast Packet */
    uint32_t broadcast_packet;  /**< Transmit Broadcast Packet */
    uint32_t unicast_packet;    /**< Transmit Unicast Packet */
    uint32_t excessive_collision; /**< Transmit Excessive collision counter */
    uint32_t late_collision;    /**< Transmit Late collision counter */
    uint32_t single_collision;  /**< Transmit Single collision frame counter */
    uint32_t multiple_collision;/**< Transmit Multiple collision frame counter */
    uint32_t total_collision;   /**< Transmit Total Collision Counter */
    uint32_t pause_control_frame; /**< Transmit PAUSE Control Frame */
    uint32_t deferral_packet;   /**< Transmit Deferral Packet */
    uint32_t excessive_deferral_packet; /**< Transmit Excessive Deferral Packet */
    uint32_t jabber_frame;      /**< Transmit Jabber Frame */
    uint32_t control_frame;     /**< Transmit Control Frame */
    uint32_t oversize_frame;    /**< Transmit Oversize Frame counter */
    uint32_t undersize_frame;   /**< Transmit Undersize Frame */
    uint32_t fragments_frame;   /**< Transmit Fragments Frame counter */
    uint32_t error;             /**< Transmission errors*/
    uint32_t underrun;          /**< Transmission underrun */
} rdpa_emac_tx_stat_t;

/** Emac statistics */
typedef struct
{
    rdpa_emac_rx_stat_t rx; /**< Emac Receive Statistics */
    rdpa_emac_tx_stat_t tx; /**< Emac Transmit Statistics */
} rdpa_emac_stat_t;

/** RDPA EMAC mask
 * A combination of \ref rdpa_emac_id constants.
 */
typedef unsigned int rdpa_emacs;

/** RDPA emac mask.
 * \param[in] __emac EMAC
 * \return EMAC - Mask representation
 */
static inline rdpa_emacs rdpa_emac_id(rdpa_emac __emac)
{
    return 1 << (__emac);
}

/** RDPA port mask
 * A combination of \ref rdpa_if_id constants.
 */
typedef uint64_t rdpa_ports __attribute__((aligned(8)));

/** RDPA interface (port) mask.
 * Can be combined in rdpa_ports mask to specify multiple ports in the same operation.
 * \param[in] __if Interface
 * \return Interface - Mask representation
 */

#if defined(CONFIG_BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
#define WL_NUM_OF_SSID_PER_UNIT        16
#else
#define WL_NUM_OF_SSID_PER_UNIT        8
#endif /* CONFIG_BCM_PON */

/** All EMACs */
#if !defined(XRDP) && !defined(BCM_XRDP)
#define RDPA_PORT_ALL_EMACS \
    (rdpa_emac_id(rdpa_emac0) | rdpa_emac_id(rdpa_emac1) | rdpa_emac_id(rdpa_emac2) | \
        rdpa_emac_id(rdpa_emac3) | rdpa_emac_id(rdpa_emac4) | rdpa_emac_id(rdpa_emac5))
#else
#ifdef BCM6888
#define RDPA_PORT_ALL_EMACS \
    (rdpa_emac_id(rdpa_emac0) | rdpa_emac_id(rdpa_emac1) | rdpa_emac_id(rdpa_emac2) | \
        rdpa_emac_id(rdpa_emac3) | rdpa_emac_id(rdpa_emac4) | rdpa_emac_id(rdpa_emac5) |\
        rdpa_emac_id(rdpa_emac6) | rdpa_emac_id(rdpa_emac7) | rdpa_emac_id(rdpa_emac8) |\
        rdpa_emac_id(rdpa_emac9) | rdpa_emac_id(rdpa_emac10) | rdpa_emac_id(rdpa_emac11) |\
        rdpa_emac_id(rdpa_emac12) | rdpa_emac_id(rdpa_emac13) | rdpa_emac_id(rdpa_emac14) |\
        rdpa_emac_id(rdpa_emac15))
#elif defined(BCM4912) || defined(CHIP_4912) || defined(BCM6813) || defined(CHIP_6813)
#define RDPA_PORT_ALL_EMACS \
    (rdpa_emac_id(rdpa_emac0) | rdpa_emac_id(rdpa_emac1) | rdpa_emac_id(rdpa_emac2) | \
        rdpa_emac_id(rdpa_emac3) | rdpa_emac_id(rdpa_emac4) | rdpa_emac_id(rdpa_emac5) |\
        rdpa_emac_id(rdpa_emac6) | rdpa_emac_id(rdpa_emac7) | rdpa_emac_id(rdpa_emac8) |\
        rdpa_emac_id(rdpa_emac9) | rdpa_emac_id(rdpa_emac10))
#else
#define RDPA_PORT_ALL_EMACS \
    (rdpa_emac_id(rdpa_emac0) | rdpa_emac_id(rdpa_emac1) | rdpa_emac_id(rdpa_emac2) | \
        rdpa_emac_id(rdpa_emac3) | rdpa_emac_id(rdpa_emac4) | rdpa_emac_id(rdpa_emac5) |\
        rdpa_emac_id(rdpa_emac6) | rdpa_emac_id(rdpa_emac7))
#endif        
#endif


/** System operation mode */
typedef enum
{
    rdpa_method_prv,          /**< Used to configure system in Provision mode */
    rdpa_method_fc,           /**< Used to configure system in Flow Cache mode */
} rdpa_operation_mode;

/** EPON mode */
typedef enum
{
    rdpa_epon_none,            /**< not EPON mode */
    rdpa_epon_ctc,             /**< CTC OAM mode */
    rdpa_epon_cuc,             /**< CUC OAM mode */
    rdpa_epon_dpoe,            /**< DPOE OAM mode */
    rdpa_epon_bcm,             /**< BCM OAM mode */
    rdpa_epon_ctc_dyn,         /**< CTC OAM dynamic mode  */
    rdpa_epon_cuc_dyn,         /**< CUC OAM dynamic mode  */
    rdpa_epon_last,
} rdpa_epon_mode;

/** Packet offset type */
typedef enum
{
    RDPA_OFFSET_L2, /**< Offset of L2 header */
    RDPA_OFFSET_L3, /**< Offset of L3 header */
    RDPA_OFFSET_L4, /**< Offset of L4 header */
} rdpa_offset_t;

/* BPM buffer size */
typedef enum
{
    RDPA_BPM_BUFFER_2K = 2048,
    RDPA_BPM_BUFFER_2_5K = 2560,
    RDPA_BPM_BUFFER_4K = 4096,
    RDPA_BPM_BUFFER_16K = 16384,
} rdpa_bpm_buffer_size_t;

/** RDP FPM resources */
typedef struct
{
    uint32_t fpm_pool_memory_size; /**< FPM pool memory size */
    uint32_t fpm_buf_size; /**< FPM buffer size */
    uint32_t hardware_supported_total_fpm_tokens; /**< Total number of FPM tokens supported by the hardware */
    uint32_t configured_total_fpm_tokens; /**< Total number of FPM tokens available to the system */
    uint32_t fpm_pool_configuration; /**< FPM pools multipliers */
} rdp_fpm_resources_t;

typedef enum  port_speed_e
{
    PORT_1_GB ,
    PORT_2_5_GB ,
    PORT_5_GB ,
    PORT_10_GB ,
} port_speed_e;

typedef struct
{
    bdmf_boolean port_enable;/**< define if port enable */
    port_speed_e port_speed; /**< define port speed (relevant only if port is enable) */
} ports_profile_t;

/** WiFi Acceleration type */
typedef enum
{
    RDPA_WL_ACCEL_NONE = 0,     /**< Acceleration disabled */
    RDPA_WL_ACCEL_WFD,          /**< WFD Acceleration type */
    RDPA_WL_ACCEL_DHD_OFFLOAD   /**< DHD Offload Acceleration type */
} rdpa_wl_accel_t;

/** CPU ring type */
typedef enum {
    rdpa_ring_data = 0,         /**< Data ring */
    rdpa_ring_recycle = 1,      /**< Recycle ring */
    rdpa_ring_feed = 2,         /**< Feed ring */
    rdpa_ring_cpu_tx = 3,       /**< Cpu tx PD ring */
} rdpa_ring_type_t;

typedef enum {
    rdpa_proto_filter_ipv4,
    rdpa_proto_filter_ipv6,
    rdpa_proto_filter_pppoe,
    rdpa_proto_filter_non_ip,
    rdpa_proto_filter_any,
    rdpa_proto_filter_last = rdpa_proto_filter_any,
} rdpa_proto_filter_t;

/** Protocol Filters mask, defines allowed protocols */
typedef enum {
    rdpa_proto_filter_ipv4_mask = (1 << rdpa_proto_filter_ipv4),     /**< Allow IPv4 traffic */
    rdpa_proto_filter_ipv6_mask = (1 << rdpa_proto_filter_ipv6),     /**< Allow IPv6 traffic */
    rdpa_proto_filter_pppoe_mask = (1 << rdpa_proto_filter_pppoe),   /**< Allow PPPoE traffic */
    rdpa_proto_filter_non_ip_mask = (1 << rdpa_proto_filter_non_ip), /**< Allow Non-IP traffic */
    rdpa_proto_filter_any_mask = (1 << rdpa_proto_filter_any),       /**< Allow any traffic */
} rdpa_proto_filter_fields;

typedef uint32_t rdpa_proto_filters_mask_t; /**< Mask of \ref rdpa_proto_filter_fields (enabled protocols) */

/** Per port CPU meters */
typedef enum {
    rdpa_port_meter_mcast, /**< Per port multicast CPU meter */
    rdpa_port_meter_bcast, /**< Per port broadcast CPU meter */
    rdpa_port_meter_unknown_da, /**< Per port unknown DA CPU meter */
    rdpa_port_meter_any, /**< Per port meter for any kind of traffic trapped to CPU */
    rdpa_port_meter_last = rdpa_port_meter_any,
    rdpa_port_meter_num,
} rdpa_port_meter_t;

/** Port ID used for TM map */
typedef enum
{
    rdpa_tm_map_port_pon,
    rdpa_tm_map_port_dsl,
    rdpa_tm_map_port_emac0,
    rdpa_tm_map_port_emac1,
    rdpa_tm_map_port_emac2,
    rdpa_tm_map_port_emac3,
    rdpa_tm_map_port_emac4,
    rdpa_tm_map_port_emac5,
    rdpa_tm_map_port_emac6,
    rdpa_tm_map_port_emac7,
#if defined(BCM6888) || defined(BCM4912) || defined(BCM6813) || defined(BCM6837)
    rdpa_tm_map_port_emac8,
    rdpa_tm_map_port_emac9,
    rdpa_tm_map_port_emac10,
#if defined(BCM6888) || defined(BCM6837)
    rdpa_tm_map_port_emac11,
    rdpa_tm_map_port_emac12,
#if defined(BCM6888)
    rdpa_tm_map_port_emac13,
    rdpa_tm_map_port_emac14,
    rdpa_tm_map_port_emac15,
#endif
#endif
#endif
#if defined(SF2_SUPPORT)
    rdpa_tm_map_port_sf2_emac0,
    rdpa_tm_map_port_sf2_emac1,
    rdpa_tm_map_port_sf2_emac2,
    rdpa_tm_map_port_sf2_emac3,
    rdpa_tm_map_port_sf2_emac4,
    rdpa_tm_map_port_sf2_emac5,
    rdpa_tm_map_port_sf2_emac6,
#endif
    rdpa_tm_map_port__num_of
} rdpa_system_tm_map_port;

/** @} end of types Doxygen group */

typedef struct
{
    int src;
    int dest;
} int2int_map_t;

static inline int int2int_map(int2int_map_t *map, int src, int last)
{
    for (; map->src != last && map->src != src; map++)
        ;
    return map->dest;
}

static inline int int2int_map_r(int2int_map_t *map, int src, int last)
{
    for (; map->src != last && map->dest != src; map++)
        ;
    return map->src;
}

typedef enum
{
    RDPA_FLOW_UNKNOWN,
    RDPA_FLOW_TUPLE_L3,
    RDPA_FLOW_TUPLE_L2,
    RDPA_FLOW_MC,
} rdpa_flow_t;


typedef enum
{
    rdpa_dos_reason_mac_sa_eq_da      = 0,    /*< MAC SA==DA */
    rdpa_dos_reason_ip_land           = 1,    /*< IPDA=IPSA in an IP(v4/v6) datagram */
    rdpa_dos_reason_tcp_blat          = 2,    /*< DPort=SPort in a TCP header */
    rdpa_dos_reason_udp_blat          = 3,    /*< DPort=SPort in a UDP header */
    rdpa_dos_reason_tcp_null_scan     = 4,    /*< Seq_Num=0 & All TCP_FLAGs=0 */
    rdpa_dos_reason_tcp_xmas_scan     = 5,    /*< Seq_Num=0 & FIN=1 & URG=1 & PSH=1 */
    rdpa_dos_reason_tcp_synfin_scan   = 6,    /*< SYN=1 & FIN=1 */
    rdpa_dos_reason_tcp_syn_error     = 7,    /*< SYN=1 & ACK=0 & SRC_Port<1024 */
    rdpa_dos_reason_tcp_short_hdr     = 8,    /*< Length of TCP header < MIN_TCP_Header_Size */
    rdpa_dos_reason_tcp_frag_error    = 9,    /*< Fragment_Offset=1 in any fragment of a fragmented IP datagram carring part of TCP data */
    rdpa_dos_reason_icmpv4_fragment   = 10,   /*< ICMPv4 protocol data unit carrier in a fragmented IPv4 datagram */
    rdpa_dos_reason_icmpv6_fragment   = 11,   /*< ICMPv6 protocol data unit carrier in a fragmented IPv6 datagram */
    rdpa_dos_reason_icmpv4_long_ping  = 12,   /*< ICMPv4 Ping(Echo Request) > MAX_ICMPv4_Size + size of IPv4 header */
    rdpa_dos_reason_icmpv6_long_ping  = 13,   /*< ICMPv6 Ping(Echo Request) > MAX_ICMPv4_Size + size of IPv6 header */

} rdpa_dos_attack_reason_t;

typedef enum
{
    rdpa_usxgmiim_1_port,
    rdpa_usxgmiim_2_port,
    rdpa_usxgmiim_4_port,
} rdpa_usxgmiim_cfg_t;

typedef enum
{
    rdpa_fpi_mode_l2,
    rdpa_fpi_mode_l3l4,
} rdpa_fpi_mode_t;

#define TM_CORE_EMAC_MAP_MAX_PORTS 15
typedef struct
{
    uint32_t tm_core_port_map_enable:1; /**< enable using PortsTm0 and PortsTm1 port map from PSP */
    uint32_t core_map:15;               /**< 0 = PortsTm0, 1 = PortsTm1, used only if tm_core_port_map_enable == 1 */
    uint32_t reserved:1;
    uint32_t enabled_map:15;            /**< port bitmap */
} rdpa_tm_core_emac_map; 

typedef struct
{
    void *sysb;             /**< Buffer pointer */
    void *data;             /**< Buffer pointer */
    uint32_t fpm_bn;        /**< Buffer number */
    uint16_t offset;        /**< Buffer offset */
    uint16_t length;        /**< Buffer length */
    uint8_t abs_flag:1;       /**< ABS/FPM */
    uint8_t sbpm_copy:1;      /**< copy to SBPM/FPM */
    uint8_t fpm_fallback:1;   /**< if no SBPM copy to FPM */
    uint8_t bufmng_cnt_id:5;  /**< buffer manager counter id */
    uint8_t fpm_pool_id;      /**< FPM pool id */
    uint16_t reserve:14;
    uint16_t is_sg_desc:1;
    uint16_t do_not_recycle:1;
} pbuf_t;

typedef union
{
    struct {
        bdmf_mac_t sa;
        bdmf_mac_t da;
        uint32_t vtag0;
        uint32_t vtag1;
        uint8_t vtag_num;
        uint16_t eth_type;
        uint8_t is_llc_snap;
        uint8_t client_idx;
    } l2;
    struct {
        bdmf_ip_t sip;
        bdmf_ip_t dip;
    } l3;
    struct {
        bdmf_ip_t sip;
        bdmf_ip_t dip;
        uint32_t vtag0;
        uint32_t vtag1;
        uint8_t vtag_num;
#if defined(BCM_DSL_RDP)
        uint8_t tos;
        uint8_t lkup_port;
#endif
        uint16_t eth_type;
    } mc;
} flow_display_info_t;

#endif /* _RDPA_TYPES_H_ */

