/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
