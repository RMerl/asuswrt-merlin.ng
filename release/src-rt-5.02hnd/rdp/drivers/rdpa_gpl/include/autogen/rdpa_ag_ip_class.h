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
    rdpa_ip_class_attr_nflows = 0, /* nflows : R : number/4 : number of configured 5-tuple based IP flows */
    rdpa_ip_class_attr_flow = 1, /* flow : RWADF : aggregate/209[65536] ip_flow_info(rdpa_ip_flow_info_t) : 5-tuple based IP flow entry */
    rdpa_ip_class_attr_flow_stat = 2, /* flow_stat : RF : aggregate/8[65536] rdpa_stat(rdpa_stat_t) : 5-tuple based IP flow entry statistics */
    rdpa_ip_class_attr_flow_status = 3, /* flow_status : RF : bool/1[65536] : 5-tuple based IP flow entry status (true for active) */
    rdpa_ip_class_attr_flush = 4, /* flush : W : bool/1 : Flush flows */
    rdpa_ip_class_attr_l4_filter = 5, /* l4_filter : RWF : aggregate/5[9(rdpa_l4_filter_index)] l4_filter_cfg(rdpa_l4_filter_cfg_t) : L4 filter configuration */
    rdpa_ip_class_attr_l4_filter_stat = 6, /* l4_filter_stat : RF : number/4[9(rdpa_l4_filter_index)] : L4 filter statistics */
    rdpa_ip_class_attr_routed_mac = 7, /* routed_mac : RWF : ether_addr/6[5] : Router MAC address */
    rdpa_ip_class_attr_fc_bypass = 8, /* fc_bypass : RW : enum_mask/4 : FlowCache Bypass Modes */
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
 * Get number of configured 5-tuple based IP flows.
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


/** Get ip_class/flow_status attribute entry.
 *
 * Get 5-tuple based IP flow entry status (true for active).
 * \param[in]   mo_ ip_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_status_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ip_class_flow_status_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean *flow_status_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_ip_class_attr_flow_status, (bdmf_index)ai_, &_nn_);
    *flow_status_ = (bdmf_boolean)_nn_;
    return _rc_;
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

/** @} end of ip_class Doxygen group */




#endif /* _RDPA_AG_IP_CLASS_H_ */
