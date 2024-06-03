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


#ifndef _RDPA_SYSTEM_H_
#define _RDPA_SYSTEM_H_

#include "rdpa_types.h"

/** \defgroup system System-level Configuration
 * The System object is a root object of RDPA object hierarchy.
 * Therefore, the system object must be created first, before any other
 * RDPA object.
 *
 * Once created, system object performs initialization and initial configuration
 * based on configuration profile and other object attributes.
 * @{
 */

#define RDPA_FW_VER_LEN 128 /**< Length of firmware version string */

#define RDPA_MAX_PORTS 32

/** RDPA sw version struct */
typedef struct
{
    uint8_t rdpa_version_major; /**< Major */
    uint8_t rdpa_version_minor; /**< Minor */
    uint8_t rdpa_version_branch; /**< Branch */
    uint32_t rdpa_version_sw_revision; /**< RDP */
    char rdpa_version_firmware_revision[RDPA_FW_VER_LEN]; /**< Firmware */
} rdpa_sw_version_t;

/** VLAN switching methods. */
typedef enum
{
    rdpa_vlan_aware_switching,      /**< VLAN aware switching */
    rdpa_mac_based_switching,       /**< MAC based switching */
    rdpa_switching_none,            /**< MAC based switching */
} rdpa_vlan_switching;

/** External switch type configuration. */
typedef enum
{
    rdpa_brcm_hdr_opcode_0, /**< 4 bytes long */
    rdpa_brcm_hdr_opcode_1, /**< 8 bytes long */
    rdpa_brcm_fttdp,        /**< FTTdp */
    rdpa_brcm_none
} rdpa_ext_sw_type;

/** Select ingress or egress packet based mapping for downstream (default is egress). */
typedef enum
{
    rdpa_egress_pbit,       /**< Force egress packet based mapping for downstream  */
    rdpa_ingress_pbit,      /**< Force ingress packet based mapping for downstream \XRDP_LIMITED */
} rdpa_qos_mapping_mode_t;

/** External switch configuration. */
typedef struct
{
    bdmf_boolean enabled;       /**< Toggle external switch */
    rdpa_emac emac_id;          /**< External switch EMAC ID. Ignored in XRDP (use emac_id in rdpa_port object instead) */
    rdpa_ext_sw_type type;      /**< External switch port identification type */
} rdpa_runner_ext_sw_cfg_t;

/** Debug feature: redirect packets to CPU */
typedef enum
{
    rdpa_rx_redirect_to_cpu_disabled,       /**< Redirection of all incoming packets to CPU is disabled */
    rdpa_rx_redirect_to_cpu_all,            /**< Redirect all packets to CPU */
} rdpa_rx_redirect_cpu_t;

#define RDPA_DP_MAX_TABLES        2  /*< One drop precedence table per direction. */

/** Drop eligibility configuration parameters, combination of PBIT and DEI used to define packet drop eligibility. */
typedef struct
{
    rdpa_traffic_dir dir;   /**< Configure the traffic direction */
    uint16_t reserved;      /* Compiler padding workaround */
    rdpa_pbit pbit;         /**< PBIT value */
    uint8_t dei;            /**< Drop Eligible Indicator value */
} rdpa_dp_key_t;

/** IPTV Table Size \n
  * The IPTV table size, can be changed only on BCM6846/BCM6856 platforms. Default value 256 \n
  * For other platforms IPTV table size is fixed, for details refer to \ref appendixes "Runner Resources Table"
  * 
  * Changing IPTV table size alters the MAC table size as well. 
  * -# If IPTV table size of 256 entries is selected the MAC table size will be 1024 entries
  * -# If IPTV table size of 1024 entries is selected the MAC table size will be 512 entries
  */ 
typedef enum
{
    rdpa_table_256_entries,  /**< table size - 256 entries */
    rdpa_table_1024_entries, /**< table size - 1024 entries */
} rdpa_iptv_table_size;

/** egress_tm drop counter type
 *  drop counter uses "rdpa_stat_t" struct
 *  counter has three mode: \n
 *  -# drop (default)     [packet, bytes] \n
 *  -# queue watermark    [packet, bytes]\n
 *  -# drop according to color (Supported only on BCM6846/BCM6856 platforms.) \n
 *                        [green packets, yellow packets]
 */
typedef enum
{
    rdpa_drop_counter_packet,    /**< egress_tm counter - drop [packet,bytes] */
    rdpa_counter_watermark,      /**< egress_tm counter - queue watermark [packet,bytes] */
    rdpa_drop_counter_color,     /**< egress_tm counter - green,yellow [packet,packet] */
} rdpa_drop_counter_mode;


/** RDPA initial system configuration.
 * This is underlying structure of system aggregate.
 */
typedef struct
{
    /** Profile-specific configuration */
    uint32_t enabled_emac; /**< backward mode - enabled EMAC bitmask*/
    rdpa_usxgmiim_cfg_t usxgmiim_port_cfg; /**< USXGMII-M mode port count */
    rdpa_vlan_switching switching_mode; /**< VLAN switching working mode */
    rdpa_operation_mode operation_mode;  /**< System operational mode */
    rdpa_runner_ext_sw_cfg_t runner_ext_sw_cfg; /**< Runner configuration when external switch is connected */
    bdmf_boolean us_ddr_queue_enable; /**< WAN TX queue DDR offload enable. Not supported in XRDP (value ignored) */
    rdpa_iptv_table_size iptv_table_size; /**< 256(default) / 1024 */
    uint32_t dhd_offload_bitmask;          /**< DHD offloaf configuration bitmask */
} rdpa_system_init_cfg_t;

/** RDPA QM configuration.
 * This is underlying structure of system aggregate.
 */
typedef struct
{
    uint16_t number_of_ds_queues; /**< define the number of queue for DS queues */
    uint16_t number_of_us_queues; /**< define the number of queue for US queues */
    uint16_t number_of_service_queues; /**< define the number of queue for SERVICE queues */
    uint16_t number_of_queue_tm_core_0; /**< number of queue used by TM Core#0 - Read Only \XRDP_LIMITED*/
    uint16_t number_of_queue_tm_core_1; /**< number of queue used by TM Core#1 - Read Only \XRDP_LIMITED*/
    uint16_t max_dynamic_queues_num; /**< Max dyamic queues number - Read Only \XRDP_LIMITED*/
 } rdpa_qm_cfg_t;
 
/** RDPA counter configuration.
 * This is underlying structure of system aggregate.
 */
typedef struct
{
    bdmf_boolean vlan_stats_enable;     /**< enable vlan counters.
                                        When enabled, reduces amount of available Ingress Classifier rule counters to
                                        half of total amount of Ingress Classifier rules \XRDP_LIMITED  */
    uint32_t shared_counters;           /**< quantity of shared counters - read only */
} rdpa_counter_cfg_t;


/** User Group configuration.
 */
typedef struct
{
    uint8_t ds_packet_buffer_alloc;    /**< define the percent of packet buffers for DS budget(0-100) */
    uint8_t us_packet_buffer_alloc;    /**< define the percent of packet buffers for US budget(0-100) */
    uint8_t wlan_cpu_packet_buffer_alloc;  /**< define the percent of packet buffers for WLAN/CPU budget (0-100) */
} rdpa_user_group_alloc_cfg_t;

typedef struct
{
    uint8_t wlan_group_rsv_threshold;  /**< define the percent of packet buffers for wlan group (0-100) */
    uint8_t wired_group_rsv_threshold; /**< define the percent of reserved packet buffers for phy group (0-100) */
    uint8_t BE1_group_rsv_threshold;  /**< define the percent of reserved packet buffers for BE1 group (0-100) */
    uint8_t BE2_group_rsv_threshold;  /**< define the percent of reserved packet buffers for BE2 group (0-100) */
} rdpa_group_rsv_thresholds_cfg_t;

typedef struct
{
    uint8_t wlan_group_max_threshold;  /**< define the percent of max packet buffers for wlan group (0-100) */
    uint8_t wired_group_max_threshold; /**< define the percent of max packet buffers for phy group (0-100) */
    uint8_t BE1_group_max_threshold;  /**< define the percent of max packet buffers for BE1 group (0-100) */
    uint8_t BE2_group_max_threshold;  /**< define the percent of max packet buffers for BE2 group (0-100) */
} rdpa_group_max_thresholds_cfg_t;

/** thresholds for fpm reservation
 */
typedef struct
{
    uint8_t high_prio_buf_threshold;   /**< define the percent of packet buffers for high priority packets (0-100) */
    uint8_t min_buf_rsv_threshold;     /**< define the percent of packet buffers reserved if UG is congested (0-100) */
} rdpa_rsv_thresholds_cfg_t;

/** RDPA WLAN Buffer Reservation configuration.
 * This is underlying structure of system aggregate.
 */
typedef struct
{
    uint8_t high_prio_buf_threshold;   /**< define the percent (from wlan budget) for high priority (0-100) */
    uint8_t excl_prio_buf_threshold;   /**< define the percent (from wlan budget) for exclusive priority (0-100) */
 } rdpa_wlan_buffer_rsv_cfg_t;

/** RDPA Packet Buffer Reservation configuration.
 * This is underlying structure of system aggregate.
 */
typedef struct
{
    rdpa_user_group_alloc_cfg_t ug_allocation;         /**<  define the packet buffer distribution between user groups (DS, US, WLAN) */
    rdpa_rsv_thresholds_cfg_t ds_prio_rsv_thrs;        /**<  define DS reservation thresholds */
    rdpa_rsv_thresholds_cfg_t us_prio_rsv_thrs;        /**<  define US reservation thresholds*/
    rdpa_wlan_buffer_rsv_cfg_t wlan_prio_rsv_thrs;     /**<  define WLAN reservation thresholds*/
    rdpa_group_rsv_thresholds_cfg_t bufmng_rsv_thrs;   /**<  define buffer management group reserved thresholds */
    rdpa_group_max_thresholds_cfg_t bufmng_max_thrs;   /**<  define buffer management group max thresholds */
} rdpa_packet_buffer_cfg_t;
 
/** RDPA system configuration that can be changed in runtime.
 * This is the underlying structure of system aggregate.
 */
typedef struct
{
    bdmf_boolean car_mode; /**< Is CAR mode enabled/disabled */
    int headroom_size; /**< Min skb headroom size. Ignored in XRDP */
    int def_max_pkt_size; /**< Default Max packet size (can be fine-tuned per port) */
    uint16_t inner_tpid; /**< Inner TPID (For single-tag VLAN action commands). Ignored in XRDP */
    uint16_t outer_tpid; /**< Outer TPID (For double-tag VLAN action commands). Ignored in XRDP */
    uint16_t add_always_tpid; /**< 'Add Always' TPID (For 'Add Always' VLAN action commands). Ignored in XRDP */
    bdmf_boolean ic_dbg_stats; /**< Enable Ingress class debug statistics */
    bdmf_boolean force_dscp_to_pbit_us; /**< Force DSCP to Pbit mapping for upstream */
    bdmf_boolean force_dscp_to_pbit_ds; /**< Force DSCP to Pbit mapping for downstream */
    rdpa_qos_mapping_mode_t qos_mapping_mode; /**< Select ingress or egress packet based mapping for downstream (default is egress) \XRDP_LIMITED  */
    uint32_t options;          /**< global reserved flag */
    rdpa_rx_redirect_cpu_t cpu_redirect_mode;  /* CPU redirection mode (debug) */
    rdpa_drop_counter_mode counter_type; /**<  [packet,byte] / [green,yellow] / [queue watermark] \XRDP_LIMITED */
    bdmf_boolean buffer_cong_mgmt; /**< Enable/Disable dynamic buffer congestion management_mode */
    bdmf_boolean force_dscp_remark; /**< Force DSCP remark */
#if defined(RDP_UFC)
    bdmf_boolean disable_non_accel_sq; /**< Disable service queues for non-accelerated packets */
#endif
} rdpa_system_cfg_t;

/** System-level drop statistics. */
#if defined(XRDP) || defined(BCM_XRDP)

typedef struct
{
    uint32_t protocol_us_ipv4;                  /**< Drop due to IPV4 protocol filter  */
    uint32_t protocol_us_ipv6;                  /**< Drop due to IPV6 protocol filter */
    uint32_t protocol_us_pppoe;                 /**< Drop due to PPPOE protocol filter */
    uint32_t protocol_us_non_ip;                /**< Drop due to none IP protocol filter */
    uint32_t mirror_us_no_sbpm;                 /**< Drop due to no SBPM for mirroring */
    uint32_t mirror_us_no_dispatch_token;       /**< Drop due to no dispatcher token for mirroring */
} rdpa_system_us_stat_t;

typedef struct
{
    uint32_t connection_action;                 /**< Connection action == drop */
    uint32_t cpu_rx_ring_congestion;            /**< CPU RX Ring congestion */
    uint32_t cpu_rx_qm_queue_drop_norm;         /**< Drop in QM for CPU RX normal queue*/
    uint32_t cpu_rx_qm_queue_drop_excl;         /**< Drop in QM for CPU RX exclusive queue*/
    uint32_t cpu_recycle_ring_congestion;       /**< CPU Recycle Ring congestion */
    uint32_t cpu_tx_copy_no_fpm;                /**< Drop due to no FPM when CPU TX copy */
    uint32_t cpu_tx_copy_no_sbpm;               /**< Drop due to no SBPM when CPU TX copy */
    uint32_t flow_drop_action;                  /**< Flow action = drop */
    uint32_t rx_mirror_cpu_mcast_exception;     /**< Drop due to RX mirroring or CPU/WLAN MCAST exception */
    uint32_t cpu_rx_meter_drop;                 /**< Counts total packets number dropped by cpu rx meters */
    uint32_t ingress_resources_congestion;      /**< Drop due to XRDP resources congestion ingress */
    uint32_t egress_resources_congestion;       /**< Drop due to XRDP resources congestion egress */
    uint32_t ingress_isolation_drop;            /**< Ingress VLAN isolation drop */
    uint32_t egress_isolation_drop;             /**< Egress VLAN isolation drop */
    uint32_t disabled_tx_flow;                  /**< TX flow is not defined */
    uint32_t cpu_rx_tc_to_rxq_map;              /**< Drop due to CPU RX TC to RXQ map */
    uint32_t cpu_rx_vport_to_cpu_obj_map;       /**< Drop due to CPU RX vport to CPU object map */
    uint32_t da_lookup_miss;                    /**< Drop due to Bridge ARL miss on DA MAC lookup */
    uint32_t sa_lookup_miss;                    /**< Drop due to Bridge ARL miss on SA MAC lookup */
    uint32_t bridge_fw_eligability;             /**< Drop due to ingress and egress ports don't belong to the same bridge */
    uint32_t da_lookup_match_drop;              /**< Drop due to DA lookup miss no matched entry */
    uint32_t sa_lookup_match_drop;              /**< Drop due to SA lookup miss no matched entry */
    uint32_t cpu_tx_disabled_q_drop;            /**< Drop due to disabled queue */
    uint32_t ingress_rate_limit_drop;           /**< Drop when ingress_rate limit emac port exceeded */
    uint32_t loopback_drop;                     /**< Drop when packet in this flow is not loopbacked */
    uint32_t undefined_queue_drop;              /**< Drop flow control on emac port */
    uint32_t qm_wred_drop;                      /**< Total drops from all queues due to WRED */
    uint32_t qm_fpm_cong_pool_drop[4];          /**< Drop due to FPM pool priority thresholds violation, counts per buffer's size pools (x1, x2, x4,x8) */
    uint32_t qm_fpm_cong;                       /**< Drop due to FPM congestion */
    uint32_t qm_fpm_grp_drop[32];               /**< Drop due to QM bufmng counter drop. Per qm drop counter */
    uint32_t qm_ddr_pd_cong_drop;               /**< Drop due to DDR PD congestion */
    uint32_t qm_ddr_byte_cong_drop;             /**< Drop due to DDR byte congestion */
    uint32_t qm_pd_cong_drop;                   /**< Drop due to PD congestion */
    uint32_t qm_psram_egress_cong;              /**< Drop due to PSSRAM egress congestion */
#if defined(BCM_PON_XRDP) || defined(RDP_UFC)
    uint32_t tx_mirroring_drop_no_credit;       /**< Drop tx mirrored packet due to no dispatcher token for mirroring */
    uint32_t tx_mirroring_drop_sbpm;            /**< Drop tx mirrored packet due to packet in sbpm */
    uint32_t tx_mirroring_drop_abs;             /**< Drop tx mirrored packet due to packet in abs */
#endif
#if !defined(BCM_DSL_XRDP)
    uint32_t tunnel_no_sbpm_drop;               /**< Drop US tunnel packet, no SBPM packet */
#endif
#if defined(RDP_UFC)
    uint32_t rx_error_drop;                     /**< Drop rx packet due to error */
    uint32_t packet_modify_drop;                /**< Drop packet due to failure in FW packet modification */
    uint32_t padding_short_packet_drop;         /**< Drop packet due to sbpm alloc failure for padding short packet */
    uint32_t tx_mirroring_exception;            /**< Drop tx mirrored packet due to inner invalid conditions packet not in FPM, etc */
#endif
    uint32_t cpu_tx_ingress_packets;            /**< Total packets sent to runner via CPU Tx ingress method */
    uint32_t cpu_tx_egress_packets;             /**< Total packets sent to runner via CPU Tx egress method */
#if !defined(BCM_DSL_XRDP)
    uint32_t flush_egress_queue_packets;        /**< Total packets which were flushed */
#endif
    uint32_t mcast_fpm_to_sbpm_drop;            /**< Multicast drop due to lack of SBPMs */
    uint32_t aqm_drop;                          /**< Total PDs droped by AQM */
    uint32_t dos_attack_drop;                   /**< Drop due to one of dos attack reasons */
} rdpa_system_common_stat_t;

typedef struct
{
    uint32_t protocol_ds_ipv4;                  /**< Drop due to IPV4 protocol filter  */
    uint32_t protocol_ds_ipv6;                  /**< Drop due to IPV6 protocol filter */
    uint32_t protocol_ds_pppoe;                 /**< Drop due to PPPOE protocol filter */
    uint32_t protocol_ds_non_ip;                /**< Drop due to none IP protocol filter */
    uint32_t mirror_ds_no_sbpm;                 /**< Drop due to no SBPM for mirroring */
    uint32_t mirror_ds_no_dispatch_token;                /**< Drop due to no dispatcher token for mirroring */
} rdpa_system_ds_stat_t;

typedef struct
{
    uint32_t dos_attack_mac_sa_eq_da;    /**< DOS_ATTACK_MAC_SA_EQ_DA detected   */
    uint32_t dos_attack_ip_land;         /**< DOS_ATTACK_IP_LAND detected         */
    uint32_t dos_attack_tcp_blat;        /**< DOS_ATTACK_TCP_BLAT detected        */
    uint32_t dos_attack_udp_blat;        /**< DOS_ATTACK_UDP_BLAT detected        */
    uint32_t dos_attack_tcp_nullscan;    /**< DOS_ATTACK_TCP_NULLScan detected    */
    uint32_t dos_attack_tcp_xmasscan;    /**< DOS_ATTACK_TCP_XMASScan detected    */
    uint32_t dos_attack_tcp_synfinscan;  /**< DOS_ATTACK_TCP_SYNFINScan detected  */
    uint32_t dos_attack_tcp_synerror;    /**< DOS_ATTACK_TCP_SYNError detected    */
    uint32_t dos_attack_tcp_shorthdr;    /**< DOS_ATTACK_TCP_ShortHDR detected    */
    uint32_t dos_attack_tcp_fragerror;   /**< DOS_ATTACK_TCP_FragError detected   */
    uint32_t dos_attack_icmpv4_fragment; /**< DOS_ATTACK_ICMPv4_Fragment detected */
    uint32_t dos_attack_icmpv6_fragment; /**< DOS_ATTACK_ICMPv6_Fragment detected */
    uint32_t dos_attack_icmpv4_longping; /**< DOS_ATTACK_ICMPv4_LongPing detected */
    uint32_t dos_attack_icmpv6_longping; /**< DOS_ATTACK_ICMPv6_LongPing detected */
} rdpa_system_dos_attack_stat_t;

#else

/** US system-level drop statistics. */
typedef struct
{
    uint16_t eth_flow_action;                   /**< Flow action = drop */
    uint16_t sa_lookup_failure;                 /**< SA lookup failure */
    uint16_t da_lookup_failure;                 /**< DA lookup failure */
    uint16_t sa_action;                         /**< SA action == drop */
    uint16_t da_action;                         /**< DA action == drop */
    uint16_t forwarding_matrix_disabled;        /**< Disabled in forwarding matrix */
    uint16_t connection_action;                 /**< Connection action == drop */
    uint16_t parsing_exception;                 /**< Parsing exception */
    uint16_t parsing_error;                     /**< Parsing error */
    uint16_t local_switching_congestion;        /**< Local switching congestion */
    uint16_t vlan_switching;                    /**< VLAN switching */
    uint16_t tpid_detect;                       /**< Invalid TPID */
    uint16_t invalid_subnet_ip;                 /**< Invalid subnet */
    uint16_t acl_oui;                           /**< Dropped by OUI ACL */
    uint16_t acl;                               /**< Dropped by ACL */
} rdpa_system_us_stat_t;

/** DS system-level drop statistics */
typedef struct
{
    uint16_t eth_flow_action;                   /**< Flow action = drop */
    uint16_t sa_lookup_failure;                 /**< SA lookup failure */
    uint16_t da_lookup_failure;                 /**< DA lookup failure */
    uint16_t sa_action;                         /**< SA action == drop */
    uint16_t da_action;                         /**< DA action == drop */
    uint16_t forwarding_matrix_disabled;        /**< Disabled in forwarding matrix */
    uint16_t connection_action;                 /**< Connection action == drop */
    uint16_t policer;                           /**< Dropped by policer */
    uint16_t parsing_exception;                 /**< Parsing exception */
    uint16_t parsing_error;                     /**< Parsing error */
    uint16_t iptv_layer3;                       /**< IPTV L3 drop */
    uint16_t vlan_switching;                    /**< VLAN switching */
    uint16_t tpid_detect;                       /**< Invalid TPID */
    uint16_t dual_stack_lite_congestion;        /**< DSLite congestion */
    uint16_t invalid_subnet_ip;                 /**< Invalid subnet */
    uint16_t invalid_layer2_protocol;           /**< Invalid L2 protocol */
    uint16_t firewall;                          /**< Dropped by firewall */
    uint16_t dst_mac_non_router;                /**< DST MAC is not equal to the router's MAC */
} rdpa_system_ds_stat_t;
#endif /* !XRDP */

typedef struct
{
    uint32_t tm_pd_not_valid_id;                 /**< TM PD not valid */
    uint32_t tm_action_not_valid_id;             /**< TM action not valid */
    uint32_t epon_tm_pd_not_valid_id;           /**< Epon TM PD not valid */
    uint32_t g9991_tm_pd_not_valid_id;          /**< DPU TM PD not valid */
    uint32_t sbpm_lib_disp_cong;                /**< SBPM LIB Dispatcher Congestion */
    uint32_t bridge_flooding;                   /**< Bridge flooding */
    uint32_t ingress_congestion_low_lan;        /**< Ingress_congestion on lan */
    uint32_t ingress_congestion_low_wan;        /**< Ingress_congestion on wan */
} rdpa_debug_stat_t;

typedef struct
{
    rdpa_system_us_stat_t us;                   /**< Upstream drop statistics */
    rdpa_system_ds_stat_t ds;                   /**< Downstream drop statistics */
#if defined(XRDP) || defined(BCM_XRDP)
    rdpa_system_common_stat_t common;           /**< Common drop statistic */
    rdpa_system_dos_attack_stat_t dos_attack;   /**< DOS Attack drop statistic */
#endif
} rdpa_system_stat_t;



/** RDPA parser configuration.
 * This is the underlying structure of system aggregate.
 */
typedef struct
{
    bdmf_boolean dos_attack_detection; /**< Is DOS ATTACK detection enabled/disabled */
} rdpa_parser_cfg_t;

/** Time Of Day. */
typedef struct {
    uint16_t sec_ms;    /**< ToD Seconds, MS bits   */
    uint32_t sec_ls;    /**< ToD Seconds, LS bits   */

    uint32_t nsec;      /**< ToD Nanoseconds        */

    uint64_t ts48_nsec; /**< Timestamp TS48 */
} rdpa_system_tod_t;

/** TPID Detect: Configuration. */
typedef struct
{
    uint16_t val_udef; /**< TPID Value, User-Defined */

    bdmf_boolean otag_en; /**< Outer tag, Enabled Detection flag */
    bdmf_boolean itag_en; /**< Inner tag, Enabled Detection flag */
    bdmf_boolean triple_en; /**< Triple tag (most inner tag), Enabled Detection flag */
} rdpa_tpid_detect_cfg_t;

#define RDPA_SYSTEM_NATC_TABLES_NUM    8

/** NAT Cache counter */
typedef struct
{
    uint32_t cache_hit_count;                   /**< NAT Cache cache hit count */
    uint32_t cache_miss_count;                  /**< NAT Cache cache miss count */
    uint32_t ddr_request_count;                 /**< NAT Cache DDR request count */
    uint32_t ddr_evict_count;                   /**< NAT Cache DDR evict count */
    uint32_t ddr_block_count;                   /**< NAT Cache DDR block count */
} rdpa_natc_cntr_t;

/** RDPA system resources */
typedef struct
{
#if defined(CONFIG_BCM_PON)  || defined(BCM_PON_XRDP) || defined(BCM_PON)
    uint32_t num_mac_entries; /**< Number of mac table entries */
#endif
#if defined(CONFIG_BCM_PON)  || defined(BCM_PON_XRDP) || defined(BCM_PON) || \
    defined(BCM63158) || defined(CHIP_63158) || defined(BCM6813) || defined(CHIP_6813)
    uint32_t num_iptv_entries; /**< Number of iptv entry table */
    uint32_t num_tcont; /**< Number of tconts */
    uint32_t num_llid; /**< Number of llids */
    uint32_t num_ds_gems; /**< Number of downstream gems */
    uint32_t num_us_gems; /**< Number of upstream gems */
    uint32_t fpm_token_size; /**< Fpm token size */
    uint8_t num_dscp2pbit_tables; /**< Number of dscp to pbit tables */
#endif
#if !defined(XRDP) 
    uint32_t num_ds_policers; /**< Number ds of policers */
    uint32_t num_us_policers; /**< Number us of policers */
#else
    uint32_t num_policers; /**< Number of policers */
#endif
} rdpa_system_resources_t;

#define RDPA_DEFAULT_MAX_TCONTS                     32

#define RDPA_SYSTEM_IPV4_HOST_ADDRESS_TABLE_SIZE    8
#define RDPA_SYSTEM_IPV6_HOST_ADDRESS_TABLE_SIZE    16

const rdp_fpm_resources_t *get_rdp_fpm_resources(void);

/** @} end of system doxygen group */

#endif /* _RDPA_SYSTEM_H_ */
