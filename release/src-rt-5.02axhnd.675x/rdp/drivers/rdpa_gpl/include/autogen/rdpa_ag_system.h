// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// Unless you and Broadcom execute a separate written software license
// agreement governing use of this software, this software is licensed
// to you under the terms of the GNU General Public License version 2
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give
//    you permission to link this software with independent modules, and
//    to copy and distribute the resulting executable under terms of your
//    choice, provided that you also meet, for each linked independent
//    module, the terms and conditions of the license of that module.
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications
//    of the software.
// 
// Not withstanding the above, under no circumstances may you combine
// this software in any way with any other Broadcom software provided
// under a license other than the GPL, without Broadcom's express prior
// written consent.
// 
// :>
/*
 * system object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_SYSTEM_H_
#define _RDPA_AG_SYSTEM_H_

/** \addtogroup system
 * @{
 */


/** Get system type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a system object.
 * \return system type handle
 */
bdmf_type_handle rdpa_system_drv(void);

/* system: Attribute types */
typedef enum {
    rdpa_system_attr_init_cfg = 0, /* init_cfg : MRI : aggregate system_init_config(rdpa_system_init_cfg_t) : Initial System Configuration */
    rdpa_system_attr_cfg = 1, /* cfg : RW : aggregate system_config(rdpa_system_cfg_t) : System Configuration that can be changes in run-time */
    rdpa_system_attr_sw_version = 2, /* sw_version : R : aggregate sw_version(rdpa_sw_version_t) : Software version */
    rdpa_system_attr_clock_gate = 3, /* clock_gate : RW : bool : Enable/Disable clock auto-gating feature */
    rdpa_system_attr_drop_precedence = 6, /* drop_precedence : RWF : bool[(dp_key)] : Drop precedence flow entry */
    rdpa_system_attr_tpid_detect = 7, /* tpid_detect : RWF : aggregate[(rdpa_tpid_detect_t)] system_tpid_detect_cfg(rdpa_tpid_detect_cfg_t) : TPID Detect */
    rdpa_system_attr_tod = 8, /* tod : R : aggregate system_tod(rdpa_system_tod_t) : Time Of Day */
    rdpa_system_attr_cpu_reason_to_tc = 9, /* cpu_reason_to_tc : RWF : number[(rdpa_cpu_reason)] : CPU Reason to TC global configuration */
    rdpa_system_attr_ipv4_host_address_table = 10, /* ipv4_host_address_table : RWADF : ipv4[] : IPv4 Host Address Table Entry */
    rdpa_system_attr_ipv6_host_address_table = 11, /* ipv6_host_address_table : RWADF : ipv6[] : IPv6 Host Address Table Entry */
    rdpa_system_attr_qm_cfg = 12, /* qm_cfg : RI : aggregate system_qm_config(rdpa_qm_cfg_t) : Configuration for dynamic Queue allocation */
    rdpa_system_attr_packet_buffer_cfg = 13, /* packet_buffer_cfg : RW : aggregate system_packet_buffer_cfg(rdpa_packet_buffer_cfg_t) : FPM packet buffer configuration for Us */
    rdpa_system_attr_high_prio_tc_threshold = 14, /* high_prio_tc_threshold : RI : enum : TC threshold for high priority traffic (TC7 - highest TC) */
    rdpa_system_attr_counter_cfg = 15, /* counter_cfg : RI : aggregate system_counter_cfg(rdpa_counter_cfg_t) : Define the counter configuration and get available count */
    rdpa_system_attr_fpm_isr_delay_timer_period = 16, /* fpm_isr_delay_timer_period : RW : number : FPM ISR delay timer period (msec) */
    rdpa_system_attr_natc_counter = 17, /* natc_counter : RWF : aggregate[] natc_cntr(rdpa_natc_cntr_t) : NAT Cache counters */
    rdpa_system_attr_ih_cong_threshold = 18, /* ih_cong_threshold : RW : number : IH Buffer threshold for congestion state (congested when IH buffer is below the configured v */
    rdpa_system_attr_ingress_congestion_ctrl = 19, /* ingress_congestion_ctrl : RW : bool : Enable/Disable ingress congestion control */
    rdpa_system_attr_system_resources = 20, /* system_resources : R : aggregate system_resources(rdpa_system_resources_t) : Return system resources */
} rdpa_system_attr_types;

extern int (*f_rdpa_system_get)(bdmf_object_handle *pmo);

/** Get system object.

 * This function returns system object instance.
 * \param[out] system_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_system_get(bdmf_object_handle *system_obj);

/** Get system/init_cfg attribute.
 *
 * Get Initial System Configuration.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  init_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_init_cfg_get(bdmf_object_handle mo_, rdpa_system_init_cfg_t * init_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_init_cfg, init_cfg_, sizeof(*init_cfg_));
}


/** Set system/init_cfg attribute.
 *
 * Set Initial System Configuration.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   init_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_init_cfg_set(bdmf_object_handle mo_, const rdpa_system_init_cfg_t * init_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_system_attr_init_cfg, init_cfg_, sizeof(*init_cfg_));
}


/** Get system/cfg attribute.
 *
 * Get System Configuration that can be changes in run-time.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_cfg_get(bdmf_object_handle mo_, rdpa_system_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set system/cfg attribute.
 *
 * Set System Configuration that can be changes in run-time.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_cfg_set(bdmf_object_handle mo_, const rdpa_system_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_system_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get system/sw_version attribute.
 *
 * Get Software version.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  sw_version_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_sw_version_get(bdmf_object_handle mo_, rdpa_sw_version_t * sw_version_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_sw_version, sw_version_, sizeof(*sw_version_));
}


/** Get system/clock_gate attribute.
 *
 * Get Enable/Disable clock auto-gating feature.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  clock_gate_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_clock_gate_get(bdmf_object_handle mo_, bdmf_boolean *clock_gate_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_system_attr_clock_gate, &_nn_);
    *clock_gate_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set system/clock_gate attribute.
 *
 * Set Enable/Disable clock auto-gating feature.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   clock_gate_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_clock_gate_set(bdmf_object_handle mo_, bdmf_boolean clock_gate_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_system_attr_clock_gate, clock_gate_);
}


/** Get system/drop_precedence attribute entry.
 *
 * Get Drop precedence flow entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  drop_precedence_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_drop_precedence_get(bdmf_object_handle mo_, rdpa_dp_key_t * ai_, bdmf_boolean *drop_precedence_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_system_attr_drop_precedence, (bdmf_index)ai_, &_nn_);
    *drop_precedence_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set system/drop_precedence attribute entry.
 *
 * Set Drop precedence flow entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   drop_precedence_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_drop_precedence_set(bdmf_object_handle mo_, rdpa_dp_key_t * ai_, bdmf_boolean drop_precedence_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_system_attr_drop_precedence, (bdmf_index)ai_, drop_precedence_);
}


/** Get next system/drop_precedence attribute entry.
 *
 * Get next Drop precedence flow entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_drop_precedence_get_next(bdmf_object_handle mo_, rdpa_dp_key_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_system_attr_drop_precedence, (bdmf_index *)ai_);
}


/** Get system/tpid_detect attribute entry.
 *
 * Get TPID Detect.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  tpid_detect_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_tpid_detect_get(bdmf_object_handle mo_, rdpa_tpid_detect_t ai_, rdpa_tpid_detect_cfg_t * tpid_detect_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_system_attr_tpid_detect, (bdmf_index)ai_, tpid_detect_, sizeof(*tpid_detect_));
}


/** Set system/tpid_detect attribute entry.
 *
 * Set TPID Detect.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   tpid_detect_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_tpid_detect_set(bdmf_object_handle mo_, rdpa_tpid_detect_t ai_, const rdpa_tpid_detect_cfg_t * tpid_detect_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_system_attr_tpid_detect, (bdmf_index)ai_, tpid_detect_, sizeof(*tpid_detect_));
}


/** Get system/tod attribute.
 *
 * Get Time Of Day.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  tod_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_tod_get(bdmf_object_handle mo_, rdpa_system_tod_t * tod_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_tod, tod_, sizeof(*tod_));
}


/** Get system/cpu_reason_to_tc attribute entry.
 *
 * Get CPU Reason to TC global configuration.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  cpu_reason_to_tc_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_cpu_reason_to_tc_get(bdmf_object_handle mo_, rdpa_cpu_reason ai_, bdmf_number *cpu_reason_to_tc_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_system_attr_cpu_reason_to_tc, (bdmf_index)ai_, &_nn_);
    *cpu_reason_to_tc_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set system/cpu_reason_to_tc attribute entry.
 *
 * Set CPU Reason to TC global configuration.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   cpu_reason_to_tc_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_cpu_reason_to_tc_set(bdmf_object_handle mo_, rdpa_cpu_reason ai_, bdmf_number cpu_reason_to_tc_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_system_attr_cpu_reason_to_tc, (bdmf_index)ai_, cpu_reason_to_tc_);
}


/** Get system/ipv4_host_address_table attribute entry.
 *
 * Get IPv4 Host Address Table Entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ipv4_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv4_host_address_table_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_ipv4 *ipv4_host_address_table_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_system_attr_ipv4_host_address_table, (bdmf_index)ai_, &_nn_);
    *ipv4_host_address_table_ = (bdmf_ipv4)_nn_;
    return _rc_;
}


/** Add system/ipv4_host_address_table attribute entry.
 *
 * Add IPv4 Host Address Table Entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   ipv4_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv4_host_address_table_add(bdmf_object_handle mo_, bdmf_index * ai_, bdmf_ipv4 ipv4_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_add_as_num(mo_, rdpa_system_attr_ipv4_host_address_table, (bdmf_index *)ai_, ipv4_host_address_table_);
    return rc;
}


/** Delete system/ipv4_host_address_table attribute entry.
 *
 * Delete IPv4 Host Address Table Entry.
 * \param[in]   mo_ system object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv4_host_address_table_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_system_attr_ipv4_host_address_table, (bdmf_index)ai_);
}


/** Find system/ipv4_host_address_table attribute entry.
 *
 * Find IPv4 Host Address Table Entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   ipv4_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv4_host_address_table_find(bdmf_object_handle mo_, bdmf_index * ai_, bdmf_ipv4 *ipv4_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_system_attr_ipv4_host_address_table, (bdmf_index *)ai_, ipv4_host_address_table_, sizeof(*ipv4_host_address_table_));
    return rc;
}


/** Get system/ipv6_host_address_table attribute entry.
 *
 * Get IPv6 Host Address Table Entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ipv6_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv6_host_address_table_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_ipv6_t * ipv6_host_address_table_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_system_attr_ipv6_host_address_table, (bdmf_index)ai_, ipv6_host_address_table_, sizeof(*ipv6_host_address_table_));
}


/** Add system/ipv6_host_address_table attribute entry.
 *
 * Add IPv6 Host Address Table Entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   ipv6_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv6_host_address_table_add(bdmf_object_handle mo_, bdmf_index * ai_, const bdmf_ipv6_t * ipv6_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_system_attr_ipv6_host_address_table, (bdmf_index *)ai_, ipv6_host_address_table_, sizeof(*ipv6_host_address_table_));
    return rc;
}


/** Delete system/ipv6_host_address_table attribute entry.
 *
 * Delete IPv6 Host Address Table Entry.
 * \param[in]   mo_ system object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv6_host_address_table_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_system_attr_ipv6_host_address_table, (bdmf_index)ai_);
}


/** Find system/ipv6_host_address_table attribute entry.
 *
 * Find IPv6 Host Address Table Entry.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   ipv6_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ipv6_host_address_table_find(bdmf_object_handle mo_, bdmf_index * ai_, bdmf_ipv6_t * ipv6_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_system_attr_ipv6_host_address_table, (bdmf_index *)ai_, ipv6_host_address_table_, sizeof(*ipv6_host_address_table_));
    return rc;
}


/** Get system/qm_cfg attribute.
 *
 * Get Configuration for dynamic Queue allocation.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  qm_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_qm_cfg_get(bdmf_object_handle mo_, rdpa_qm_cfg_t * qm_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_qm_cfg, qm_cfg_, sizeof(*qm_cfg_));
}


/** Set system/qm_cfg attribute.
 *
 * Set Configuration for dynamic Queue allocation.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   qm_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_qm_cfg_set(bdmf_object_handle mo_, const rdpa_qm_cfg_t * qm_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_system_attr_qm_cfg, qm_cfg_, sizeof(*qm_cfg_));
}


/** Get system/packet_buffer_cfg attribute.
 *
 * Get FPM packet buffer configuration for User Groups.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  packet_buffer_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_packet_buffer_cfg_get(bdmf_object_handle mo_, rdpa_packet_buffer_cfg_t * packet_buffer_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_packet_buffer_cfg, packet_buffer_cfg_, sizeof(*packet_buffer_cfg_));
}


/** Set system/packet_buffer_cfg attribute.
 *
 * Set FPM packet buffer configuration for User Groups.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   packet_buffer_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_packet_buffer_cfg_set(bdmf_object_handle mo_, const rdpa_packet_buffer_cfg_t * packet_buffer_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_system_attr_packet_buffer_cfg, packet_buffer_cfg_, sizeof(*packet_buffer_cfg_));
}


/** Get system/high_prio_tc_threshold attribute.
 *
 * Get TC threshold for high priority traffic (TC7 - highest TC).
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  high_prio_tc_threshold_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_high_prio_tc_threshold_get(bdmf_object_handle mo_, rdpa_cpu_tc *high_prio_tc_threshold_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_system_attr_high_prio_tc_threshold, &_nn_);
    *high_prio_tc_threshold_ = (rdpa_cpu_tc)_nn_;
    return _rc_;
}


/** Set system/high_prio_tc_threshold attribute.
 *
 * Set TC threshold for high priority traffic (TC7 - highest TC).
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   high_prio_tc_threshold_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_high_prio_tc_threshold_set(bdmf_object_handle mo_, rdpa_cpu_tc high_prio_tc_threshold_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_system_attr_high_prio_tc_threshold, high_prio_tc_threshold_);
}


/** Get system/counter_cfg attribute.
 *
 * Get Define the counter configuration and get available counters.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  counter_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_counter_cfg_get(bdmf_object_handle mo_, rdpa_counter_cfg_t * counter_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_counter_cfg, counter_cfg_, sizeof(*counter_cfg_));
}


/** Set system/counter_cfg attribute.
 *
 * Set Define the counter configuration and get available counters.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   counter_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_system_counter_cfg_set(bdmf_object_handle mo_, const rdpa_counter_cfg_t * counter_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_system_attr_counter_cfg, counter_cfg_, sizeof(*counter_cfg_));
}


/** Get system/fpm_isr_delay_timer_period attribute.
 *
 * Get FPM ISR delay timer period (msec).
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  fpm_isr_delay_timer_period_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_fpm_isr_delay_timer_period_get(bdmf_object_handle mo_, bdmf_number *fpm_isr_delay_timer_period_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_system_attr_fpm_isr_delay_timer_period, &_nn_);
    *fpm_isr_delay_timer_period_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set system/fpm_isr_delay_timer_period attribute.
 *
 * Set FPM ISR delay timer period (msec).
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   fpm_isr_delay_timer_period_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_fpm_isr_delay_timer_period_set(bdmf_object_handle mo_, bdmf_number fpm_isr_delay_timer_period_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_system_attr_fpm_isr_delay_timer_period, fpm_isr_delay_timer_period_);
}


/** Get system/natc_counter attribute entry.
 *
 * Get NAT Cache counters.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  natc_counter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_natc_counter_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_natc_cntr_t * natc_counter_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_system_attr_natc_counter, (bdmf_index)ai_, natc_counter_, sizeof(*natc_counter_));
}


/** Set system/natc_counter attribute entry.
 *
 * Set NAT Cache counters.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   natc_counter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_natc_counter_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_natc_cntr_t * natc_counter_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_system_attr_natc_counter, (bdmf_index)ai_, natc_counter_, sizeof(*natc_counter_));
}


/** Get system/ih_cong_threshold attribute.
 *
 * Get IH Buffer threshold for congestion state (congested when IH buffer is below the configured value).
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  ih_cong_threshold_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ih_cong_threshold_get(bdmf_object_handle mo_, bdmf_number *ih_cong_threshold_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_system_attr_ih_cong_threshold, &_nn_);
    *ih_cong_threshold_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set system/ih_cong_threshold attribute.
 *
 * Set IH Buffer threshold for congestion state (congested when IH buffer is below the configured value).
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ih_cong_threshold_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ih_cong_threshold_set(bdmf_object_handle mo_, bdmf_number ih_cong_threshold_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_system_attr_ih_cong_threshold, ih_cong_threshold_);
}


/** Get system/ingress_congestion_ctrl attribute.
 *
 * Get Enable/Disable ingress congestion control.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  ingress_congestion_ctrl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ingress_congestion_ctrl_get(bdmf_object_handle mo_, bdmf_boolean *ingress_congestion_ctrl_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_system_attr_ingress_congestion_ctrl, &_nn_);
    *ingress_congestion_ctrl_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set system/ingress_congestion_ctrl attribute.
 *
 * Set Enable/Disable ingress congestion control.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[in]   ingress_congestion_ctrl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_ingress_congestion_ctrl_set(bdmf_object_handle mo_, bdmf_boolean ingress_congestion_ctrl_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_system_attr_ingress_congestion_ctrl, ingress_congestion_ctrl_);
}


/** Get system/system_resources attribute.
 *
 * Get Return system resources.
 * \param[in]   mo_ system object handle or mattr transaction handle
 * \param[out]  system_resources_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_system_system_resources_get(bdmf_object_handle mo_, rdpa_system_resources_t * system_resources_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_system_attr_system_resources, system_resources_, sizeof(*system_resources_));
}

/** @} end of system Doxygen group */




#endif /* _RDPA_AG_SYSTEM_H_ */
