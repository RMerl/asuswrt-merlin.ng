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

#include "bdmf_dev.h"
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

#define RDPA_FW_VER_LEN 32 /**< Length of firmware version string */
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

/** External switch configuration. */
typedef struct
{
    bdmf_boolean enabled;       /**< Toggle external switch */
    rdpa_emac emac_id;          /**< External switch EMAC ID. Ignored in XRDP (use emac_id in rdpa_port object instead) */
    rdpa_ext_sw_type type;      /**< External switch port identification type */
} rdpa_runner_ext_sw_cfg_t;

#define RDPA_DP_MAX_TABLES        2  /*< One drop precedence table per direction. */

/** Drop eligibility configuration parameters, combination of PBIT and DEI used to define packet drop eligibility. */
typedef struct
{
    rdpa_traffic_dir dir;   /**< Configure the traffic direction */
    rdpa_pbit pbit;         /**< PBIT value */
    uint8_t dei;            /**< Drop Eligible Indicator value */
} rdpa_dp_key_t;

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
} rdpa_system_init_cfg_t;

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
    uint32_t options;          /**< global reserved flag */
} rdpa_system_cfg_t;

/** Time Of Day. */
typedef struct {
    uint16_t sec_ms;    /**< Seconds, MS bits   */
    uint32_t sec_ls;    /**< Seconds, LS bits   */

    uint32_t nsec;      /**< Nanoseconds        */
} rdpa_system_tod_t;

/** TPID Detect: Configuration. */
typedef struct
{
    uint16_t val_udef; /**< TPID Value, User-Defined */

    bdmf_boolean otag_en; /**< Outer tag, Enabled Detection flag */
    bdmf_boolean itag_en; /**< Inner tag, Enabled Detection flag */
    bdmf_boolean triple_en; /**< Triple tag (most inner tag), Enabled Detection flag */
} rdpa_tpid_detect_cfg_t;

/** @} end of system doxygen group */

#endif /* _RDPA_SYSTEM_H_ */
