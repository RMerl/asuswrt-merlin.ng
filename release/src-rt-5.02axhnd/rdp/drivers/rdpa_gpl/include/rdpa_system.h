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
/** RDPA sw version struct */

#define RDPA_FW_VER_LEN 128 /**< Length of firmware version string */
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
    rdpa_emac gbe_wan_emac; /**< EMAC ID */
    rdpa_vlan_switching switching_mode; /**< VLAN switching working mode */
    rdpa_ip_class_method ip_class_method;  /**< Operational mode of the IP class object */
    rdpa_runner_ext_sw_cfg_t runner_ext_sw_cfg; /**< Runner configuration when external switch is connected */
    bdmf_boolean us_ddr_queue_enable; /**< WAN TX queue DDR offload enable. Not supported in XRDP (value ignored) */
    bdmf_boolean dpu_split_scheduling_mode; /**< DPU split scheduling mode */
    rdpa_iptv_table_size iptv_table_size; /**< 256(default) / 1024 */
} rdpa_system_init_cfg_t;

/** RDPA QM configuration.
 * This is underlying structure of system aggregate.
 */
typedef struct
{
    uint16_t number_of_ds_queues; /**< define the number of queue for DS queues */
    uint16_t number_of_us_queues; /**< define the number of queue for US queues */
    uint16_t number_of_service_queues; /**< define the number of queue for SERVICE queues */
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
    uint8_t ds_packet_buffer_alloc;    /**< define the percent of packet buffers for DS (0-100) */
    uint8_t us_packet_buffer_alloc;    /**< define the percent of packet buffers for US (0-100) */
    uint8_t wlan_packet_buffer_alloc;  /**< define the percent of packet buffers for WLAN (0-100) */
} rdpa_user_group_alloc_cfg_t;

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
 } rdpa_packet_buffer_cfg_t;
 
/** RDPA system configuration that can be changed in runtime.
 * This is the underlying structure of system aggregate.
 */
typedef struct
{
    bdmf_boolean car_mode; /**< Is CAR mode enabled/disabled */
    int headroom_size; /**< Min skb headroom size. Ignored in XRDP */
    int mtu_size; /**< MTU size. Ignored in XRDP */
    uint16_t inner_tpid; /**< Inner TPID (For single-tag VLAN action commands). Ignored in XRDP */
    uint16_t outer_tpid; /**< Outer TPID (For double-tag VLAN action commands). Ignored in XRDP */
    uint16_t add_always_tpid; /**< 'Add Always' TPID (For 'Add Always' VLAN action commands). Ignored in XRDP */
    bdmf_boolean ic_dbg_stats; /**< Enable Ingress class debug statistics */
    bdmf_boolean force_dscp_to_pbit_us; /**< Force DSCP to Pbit mapping for upstream */
    bdmf_boolean force_dscp_to_pbit_ds; /**< Force DSCP to Pbit mapping for downstream */
    rdpa_qos_mapping_mode_t qos_mapping_mode; /**< Select ingress or egress packet based mapping for downstream (default is egress) \XRDP_LIMITED  */
    uint32_t options;          /**< global reserved flag */
    uint8_t rate_limit_overhead;   /**< Rate limit overhead (bytes)*/
    rdpa_rx_redirect_cpu_t cpu_redirect_mode;  /* CPU redirection mode (debug) */
    rdpa_drop_counter_mode counter_type; /**<  [packet,byte] / [green,yellow] / [queue watermark] \XRDP_LIMITED */
} rdpa_system_cfg_t;

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


typedef struct
{
#if defined(CONFIG_BCM_PON)  || defined(BCM_PON_XRDP) || defined(BCM_PON) || defined(BCM63158) || defined(CHIP_63158) 
    uint32_t num_mac_entries; /**< Number of mac table entries */
    uint32_t num_iptv_entries; /**< Number of iptv entry table */
    uint32_t num_tcont; /**< Number of tconts */
    uint32_t num_llid; /**< Number of llids */
    uint32_t num_ds_gems; /**< Number of downstream gems */
    uint32_t num_us_gems; /**< Number of upstream gems */
#endif
#if !defined(XRDP) 
    uint32_t num_ds_policers; /**< Number ds of policers */
    uint32_t num_us_policers; /**< Number us of policers */
#else
    uint32_t num_policers; /**< Number of policers */
#endif
} rdpa_system_resources_t;


#define RDPA_SYSTEM_IPV4_HOST_ADDRESS_TABLE_SIZE    8
#define RDPA_SYSTEM_IPV6_HOST_ADDRESS_TABLE_SIZE    16

/** @} end of system doxygen group */

#endif /* _RDPA_SYSTEM_H_ */
