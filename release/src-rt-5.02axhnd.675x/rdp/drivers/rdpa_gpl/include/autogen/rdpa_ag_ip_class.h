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
 * ip_class object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_IP_CLASS_H_
#define _RDPA_AG_IP_CLASS_H_

/** \addtogroup ip_class
 * @{
 */


/** Get ip_class type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an ip_class object.
 * \return ip_class type handle
 */
bdmf_type_handle rdpa_ip_class_drv(void);

/* ip_class: Attribute types */
typedef enum {
    rdpa_ip_class_attr_nflows = 0, /* nflows : R : number : number of configured IP flows (5-tuple, 6-tuple, or 3-tuple) */
    rdpa_ip_class_attr_flow_idx_pool_ptr = 1, /* flow_idx_pool_ptr : RI : pointer : Flow ID Pool Virtual Address */
    rdpa_ip_class_attr_flow = 2, /* flow : RWADF : aggregate[] ip_flow_info(rdpa_ip_flow_info_t) : 5-tuple based IP flow entry */
    rdpa_ip_class_attr_flow_stat = 3, /* flow_stat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : 5-tuple based IP flow entry statistics */
    rdpa_ip_class_attr_flush = 4, /* flush : W : bool : Flush flows */
    rdpa_ip_class_attr_l4_filter = 5, /* l4_filter : RWF : aggregate[(rdpa_l4_filter_index)] l4_filter_cfg(rdpa_l4_filter_cfg_t) : L4 filter configuration */
    rdpa_ip_class_attr_l4_filter_stat = 6, /* l4_filter_stat : RF : number[(rdpa_l4_filter_index)] : L4 filter statistics */
    rdpa_ip_class_attr_routed_mac = 7, /* routed_mac : RWF : ether_addr[] : Router MAC address */
    rdpa_ip_class_attr_fc_bypass = 8, /* fc_bypass : RW : enum_mask : FlowCache Bypass Modes */
    rdpa_ip_class_attr_key_type = 9, /* key_type : RW : enum : IP class key type */
    rdpa_ip_class_attr_pathstat = 10, /* pathstat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : Ip class path entry statistics */
    rdpa_ip_class_attr_tcp_ack_prio = 11, /* tcp_ack_prio : RW : bool : TCP pure ACK prioritization (common for L3 and L2) */
} rdpa_ip_class_attr_types;

extern int (*f_rdpa_ip_class_get)(bdmf_object_handle *pmo);

/** Get ip_class object.

 * This function returns ip_class object instance.
 * \param[out] ip_class_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_ip_class_get(bdmf_object_handle *ip_class_obj);

/** Get ip_class/nflows attribute.
 *
 * Get number of configured IP flows (5-tuple, 6-tuple, or 3-tuple).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  nflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_nflows_get(bdmf_object_handle mo_, bdmf_number *nflows_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ip_class_attr_nflows, &_nn_);
    *nflows_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get ip_class/flow_idx_pool_ptr attribute.
 *
 * Get Flow ID Pool Virtual Address.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  flow_idx_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_idx_pool_ptr_get(bdmf_object_handle mo_, void * *flow_idx_pool_ptr_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ip_class_attr_flow_idx_pool_ptr, &_nn_);
    *flow_idx_pool_ptr_ = (void *)(long)_nn_;
    return _rc_;
}


/** Set ip_class/flow_idx_pool_ptr attribute.
 *
 * Set Flow ID Pool Virtual Address.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   flow_idx_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_idx_pool_ptr_set(bdmf_object_handle mo_, void * flow_idx_pool_ptr_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ip_class_attr_flow_idx_pool_ptr, (long)flow_idx_pool_ptr_);
}


/** Get ip_class/flow attribute entry.
 *
 * Get 5-tuple based IP flow entry.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ip_flow_info_t * flow_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ip_class_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Set ip_class/flow attribute entry.
 *
 * Set 5-tuple based IP flow entry.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_ip_flow_info_t * flow_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ip_class_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Add ip_class/flow attribute entry.
 *
 * Add 5-tuple based IP flow entry.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ip_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ip_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Delete ip_class/flow attribute entry.
 *
 * Delete 5-tuple based IP flow entry.
 * \param[in]   mo_ ip_class object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ip_class_attr_flow, (bdmf_index)ai_);
}


/** Get next ip_class/flow attribute entry.
 *
 * Get next 5-tuple based IP flow entry.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_ip_class_attr_flow, (bdmf_index *)ai_);
}


/** Find ip_class/flow attribute entry.
 *
 * Find 5-tuple based IP flow entry.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_ip_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_ip_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Get ip_class/flow_stat attribute entry.
 *
 * Get 5-tuple based IP flow entry statistics.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * flow_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ip_class_attr_flow_stat, (bdmf_index)ai_, flow_stat_, sizeof(*flow_stat_));
}


/** Get next ip_class/flow_stat attribute entry.
 *
 * Get next 5-tuple based IP flow entry statistics.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_stat_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_ip_class_attr_flow_stat, (bdmf_index *)ai_);
}


/** Set ip_class/flush attribute.
 *
 * Set Flush flows.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_flush_set(bdmf_object_handle mo_, bdmf_boolean flush_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ip_class_attr_flush, flush_);
}


/** Get ip_class/l4_filter attribute entry.
 *
 * Get L4 filter configuration.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  l4_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_l4_filter_get(bdmf_object_handle mo_, rdpa_l4_filter_index ai_, rdpa_l4_filter_cfg_t * l4_filter_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ip_class_attr_l4_filter, (bdmf_index)ai_, l4_filter_, sizeof(*l4_filter_));
}


/** Set ip_class/l4_filter attribute entry.
 *
 * Set L4 filter configuration.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   l4_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_l4_filter_set(bdmf_object_handle mo_, rdpa_l4_filter_index ai_, const rdpa_l4_filter_cfg_t * l4_filter_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ip_class_attr_l4_filter, (bdmf_index)ai_, l4_filter_, sizeof(*l4_filter_));
}


/** Get ip_class/l4_filter_stat attribute entry.
 *
 * Get L4 filter statistics.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  l4_filter_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_l4_filter_stat_get(bdmf_object_handle mo_, rdpa_l4_filter_index ai_, uint32_t *l4_filter_stat_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_ip_class_attr_l4_filter_stat, (bdmf_index)ai_, &_nn_);
    *l4_filter_stat_ = (uint32_t)_nn_;
    return _rc_;
}


/** Get ip_class/routed_mac attribute entry.
 *
 * Get Router MAC address.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  routed_mac_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_routed_mac_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_mac_t * routed_mac_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ip_class_attr_routed_mac, (bdmf_index)ai_, routed_mac_, sizeof(*routed_mac_));
}


/** Set ip_class/routed_mac attribute entry.
 *
 * Set Router MAC address.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   routed_mac_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_routed_mac_set(bdmf_object_handle mo_, bdmf_index ai_, const bdmf_mac_t * routed_mac_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ip_class_attr_routed_mac, (bdmf_index)ai_, routed_mac_, sizeof(*routed_mac_));
}


/** Get ip_class/fc_bypass attribute.
 *
 * Get FlowCache Bypass Modes.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  fc_bypass_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_fc_bypass_get(bdmf_object_handle mo_, rdpa_fc_bypass *fc_bypass_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ip_class_attr_fc_bypass, &_nn_);
    *fc_bypass_ = (rdpa_fc_bypass)_nn_;
    return _rc_;
}


/** Set ip_class/fc_bypass attribute.
 *
 * Set FlowCache Bypass Modes.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   fc_bypass_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_fc_bypass_set(bdmf_object_handle mo_, rdpa_fc_bypass fc_bypass_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ip_class_attr_fc_bypass, fc_bypass_);
}


/** Get ip_class/key_type attribute.
 *
 * Get IP class key type.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  key_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_key_type_get(bdmf_object_handle mo_, rdpa_key_type *key_type_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ip_class_attr_key_type, &_nn_);
    *key_type_ = (rdpa_key_type)_nn_;
    return _rc_;
}


/** Set ip_class/key_type attribute.
 *
 * Set IP class key type.
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   key_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_key_type_set(bdmf_object_handle mo_, rdpa_key_type key_type_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ip_class_attr_key_type, key_type_);
}


/** Get ip_class/pathstat attribute entry.
 *
 * Get Ip class path entry statistics.

 * \deprecated This function has been deprecated.
 *
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pathstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_pathstat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * pathstat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ip_class_attr_pathstat, (bdmf_index)ai_, pathstat_, sizeof(*pathstat_));
}


/** Get ip_class/tcp_ack_prio attribute.
 *
 * Get TCP pure ACK prioritization (common for L3 and L2).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[out]  tcp_ack_prio_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_tcp_ack_prio_get(bdmf_object_handle mo_, bdmf_boolean *tcp_ack_prio_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ip_class_attr_tcp_ack_prio, &_nn_);
    *tcp_ack_prio_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set ip_class/tcp_ack_prio attribute.
 *
 * Set TCP pure ACK prioritization (common for L3 and L2).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   tcp_ack_prio_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ip_class_tcp_ack_prio_set(bdmf_object_handle mo_, bdmf_boolean tcp_ack_prio_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ip_class_attr_tcp_ack_prio, tcp_ack_prio_);
}

/** @} end of ip_class Doxygen group */




#endif /* _RDPA_AG_IP_CLASS_H_ */
