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

#ifndef _RDPA_L2_UCAST_H_
#define _RDPA_L2_UCAST_H_

#include <bdmf_interface.h>
#include "rdpa_cpu.h"
#include "rdpa_egress_tm.h"
#include "rdpa_cmd_list.h"

/** \defgroup L2 flow Flow Classification
 * L2 flows are used for fast L2 bridging.\n
 * The classifier identifies L2 flows using L2 key\n
 * { dst_mac, src_mac, vtag[2], eth_type, vtag_num }.\n
 * @{
 */

#if defined(XRDP)
#define RDPA_UCAST_MAX_PATHS 64
#define RDPA_L2_KEY_EXCLUDE_FIELDS rdpa_l2_flow_key_exclude_dei_field
#else
#define RDPA_UCAST_MAX_PATHS 64
#endif

#include "rdpa_l2_common.h"

/** L2 flow classifaction result.\n
 * Each result determines L2 header manipulation, forwarding decision and QoS mapping information.\n
 */
typedef struct {
    rdpa_if egress_if;                                     /**< RDPA Egress Interface */
    uint32_t queue_id;                                     /**< Egress queue id */
    uint8_t service_queue_id;                              /**< Service queue id */
    int wan_flow;                                          /**< DSL ATM/PTM US channel */
    int wan_flow_mode;                                     /**< DSL single/bonded */
    uint8_t is_routed;                                     /**< 1: Routed Flow; 0: Bridged Flow */
    uint8_t is_l2_accel;                                   /**< 1: L2 acceleratd Flow; 0: L3 accelerated Flow */
    uint8_t is_hit_trap;                                   /**< 1: Trap to cpu; 0: forwarding */
    uint8_t tc;                                            /**< 6-bit traffic class value */
    uint8_t is_wred_high_prio;                             /**< 1: WRED High Priority class, 0: WRED Low Priority class */
    uint8_t is_ingqos_high_prio;                           /**< 1: Ingress QoS High Priority, 0: Low Priority */
    uint8_t drop;                                          /**< 1: Drop packets; 0: Forward packets */
    uint16_t mtu;                                          /**< Egress Port MTU */
    uint8_t is_tos_mangle;                                 /**< 1: Mangle ToS; 0: No Mangle ToS */
    uint8_t tos;                                           /**< mangled TX ToS value */
    uint8_t lag_port;                                      /**< Runner Egress LAG Port */
    union {
        uint32_t wl_metadata;                              /**< WL metadata */
        rdpa_wfd_t wfd;
        rdpa_rnr_t rnr;
    };
    uint8_t cpu_reason;                                    /**< CPU_rx trap reason */
    uint8_t pathstat_idx;                                  /**< Pathstat counter index */
    uint8_t cmd_list_length;                               /**< Command List Length, in bytes */
    uint32_t cmd_list[RDPA_CMD_LIST_UCAST_LIST_SIZE_32];   /**< Command List */
} rdpa_l2_flow_result_t;

/** L2 flow classifaction info (key + result).\n
 */
typedef struct {
    uint32_t  hw_flow_id;            /**< L2 flow HW flow ID */
    rdpa_l2_flow_key_t key;          /**< tuple based L2 flow key */
    rdpa_l2_flow_result_t result;    /**< tuple based L2 flow result */
} rdpa_l2_flow_info_t;

/** @} end of l2_ucast Doxygen group. */

#endif /* _RDPA_L2_UCAST_H_ */
