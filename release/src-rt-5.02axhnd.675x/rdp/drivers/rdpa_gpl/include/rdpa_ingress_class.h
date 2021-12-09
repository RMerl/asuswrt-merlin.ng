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

#ifndef _RDPA_INGRESS_CLASS_H_
#define _RDPA_INGRESS_CLASS_H_

#include "bdmf_interface.h"
#include "rdpa_ingress_class_basic.h"
#include "rdpa_types.h"

/** \defgroup ingress_class Ingress Classification
 *  Ingress classifier supports up to 16 classifiers per direction
 *
 * @{
 */
#define RDPA_IC_MAX_PRIORITY 63

/** ACL classifier mode */
typedef enum {
    RDPA_ACL_MODE_BLACK, /**< Black list, matched packets are dropped */
    RDPA_ACL_MODE_WHITE, /**< White list, unmatched packets are dropped */
    RDPA_ACL_MODE_NONE
} rdpa_acl_mode;

/** Classifier configuration */
typedef struct {
    rdpa_ic_type type; /**<Classification type - ACL/flow/QoS/IP_flow */
    uint32_t field_mask; /**< Fields used for classification. A combination of rdpa_ic_fields. */
    uint32_t prty;  /**< Defines the priority of classifier inside rdpa_ic_value . value between 0 - 64. */
    rdpa_acl_mode acl_mode; /**< Black/White list. relevant only for cfg type acl. */
    rdpa_ports port_mask; /**< LAN ports mask, - reserved for future use. */
    rdpa_ic_gen_rule_cfg_t gen_rule_cfg1; /**< First generic rule configuration, if available. */
    rdpa_ic_gen_rule_cfg_t gen_rule_cfg2; /**< Second generic rule configuration, if available. */
    rdpa_filter_location_t generic_filter_location; /**< All Missed traffic \XRDP_LIMITED */
} rdpa_ic_cfg_t;

/** Ingress classification info (key + result).\n
 */
typedef struct 
{
    rdpa_ic_key_t  key; /**< Ingress classification key */
    rdpa_ic_result_t  result; /**< Ingress classification result */
} rdpa_ic_info_t;

/** Ingress classification per port key */
typedef struct
{
    bdmf_index flow; /**< Index of flow to add VLAN action to */
    rdpa_if port;  /**< Egress port */
} rdpa_port_action_key_t;

/** Ingress classification per port action */
typedef struct
{
    bdmf_object_handle vlan_action; /**< VLAN action object */
    bdmf_boolean drop; /**< Drop action - true/false */
} rdpa_port_action_t;

/** Bitmask of actions */
typedef enum
{
    /** Forward through service queue */
    rdpa_ic_action_service_q = (1 << rdpa_ic_act_service_q), /**< service queue bitmask*/

    /** Send packet copy to CPU */
    rdpa_ic_action_cpu_mirroring = (1 << rdpa_ic_act_cpu_mirroring), /**< cpu mirroring bitmask*/

    /** TTL action */
    rdpa_ic_action_ttl = (1 << rdpa_ic_act_ttl), /**< ttl action bitmask*/
}
rdpa_ic_action_vector_t;

/** @} end of ingress_classification Doxygen group. */

#endif /* _RDPA_INGRESS_CLASS_H_ */
