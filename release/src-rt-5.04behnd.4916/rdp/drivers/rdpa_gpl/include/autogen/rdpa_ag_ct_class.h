// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2, as published by
// the Free Software Foundation (the "GPL").
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// 
// A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
// writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
// 
// :>
/*
 * ct_class object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_CT_CLASS_H_
#define _RDPA_AG_CT_CLASS_H_

/** \addtogroup ct_class
 * @{
 */


/** Get ct_class type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a ct_class object.
 * \return ct_class type handle
 */
bdmf_type_handle rdpa_ct_class_drv(void);

/* ct_class: Attribute types */
typedef enum {
    rdpa_ct_class_attr_nflows = 0, /* nflows : R : number : number of configured CT entries */
    rdpa_ct_class_attr_flow = 1, /* flow : RWADF : aggregate[] ct_entry_info(rdpa_ct_entry_info_t) : 5-tuple based CT entry */
    rdpa_ct_class_attr_flush = 2, /* flush : W : bool : Flush entries */
    rdpa_ct_class_attr_ct_miss_counter = 3, /* ct_miss_counter : RW : number : CT miss counter */
    rdpa_ct_class_attr_ratelimit_drop_counter = 4, /* ratelimit_drop_counter : RW : number : Ratelimit drop counter */
    rdpa_ct_class_attr_ct_stat = 5, /* ct_stat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : 5-tuple CT entry hit statistics */
    rdpa_ct_class_attr_enable = 6, /* enable : RW : bool : Enable Firewall on WAN ports; when enabled, TCP/UDP traffic from WAN to LAN not matching CT entries will  */
    rdpa_ct_class_attr_enable_ct_expect = 7, /* enable_ct_expect : RW : bool : Enable CT_Expected lookup */
} rdpa_ct_class_attr_types;

extern int (*f_rdpa_ct_class_get)(bdmf_object_handle *pmo);

/** Get ct_class object.

 * This function returns ct_class object instance.
 * \param[out] ct_class_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_ct_class_get(bdmf_object_handle *ct_class_obj);

/** Get ct_class/nflows attribute.
 *
 * Get number of configured CT entries.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[out]  nflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_nflows_get(bdmf_object_handle mo_, bdmf_number *nflows_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ct_class_attr_nflows, &_nn_);
    *nflows_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get ct_class/flow attribute entry.
 *
 * Get 5-tuple based CT entry.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ct_class_flow_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ct_entry_info_t * flow_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ct_class_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Add ct_class/flow attribute entry.
 *
 * Add 5-tuple based CT entry.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ct_class_flow_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ct_entry_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ct_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Delete ct_class/flow attribute entry.
 *
 * Delete 5-tuple based CT entry.
 * \param[in]   mo_ ct_class object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ct_class_flow_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ct_class_attr_flow, (bdmf_index)ai_);
}


/** Get next ct_class/flow attribute entry.
 *
 * Get next 5-tuple based CT entry.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ct_class_flow_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_ct_class_attr_flow, (bdmf_index *)ai_);
}


/** Find ct_class/flow attribute entry.
 *
 * Find 5-tuple based CT entry.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ct_class_flow_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_ct_entry_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_ct_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Set ct_class/flush attribute.
 *
 * Set Flush entries.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_flush_set(bdmf_object_handle mo_, bdmf_boolean flush_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ct_class_attr_flush, flush_);
}


/** Get ct_class/ct_miss_counter attribute.
 *
 * Get CT miss counter.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[out]  ct_miss_counter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_ct_miss_counter_get(bdmf_object_handle mo_, bdmf_number *ct_miss_counter_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ct_class_attr_ct_miss_counter, &_nn_);
    *ct_miss_counter_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get ct_class/ratelimit_drop_counter attribute.
 *
 * Get Ratelimit drop counter.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[out]  ratelimit_drop_counter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_ratelimit_drop_counter_get(bdmf_object_handle mo_, bdmf_number *ratelimit_drop_counter_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ct_class_attr_ratelimit_drop_counter, &_nn_);
    *ratelimit_drop_counter_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get ct_class/ct_stat attribute entry.
 *
 * Get 5-tuple CT entry hit statistics.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ct_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ct_class_ct_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * ct_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ct_class_attr_ct_stat, (bdmf_index)ai_, ct_stat_, sizeof(*ct_stat_));
}


/** Get next ct_class/ct_stat attribute entry.
 *
 * Get next 5-tuple CT entry hit statistics.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ct_class_ct_stat_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_ct_class_attr_ct_stat, (bdmf_index *)ai_);
}


/** Get ct_class/enable attribute.
 *
 * Get Enable Firewall on WAN ports; when enabled, TCP/UDP traffic from WAN to LAN not matching CT entries will be dropped.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ct_class_attr_enable, &_nn_);
    *enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set ct_class/enable attribute.
 *
 * Set Enable Firewall on WAN ports; when enabled, TCP/UDP traffic from WAN to LAN not matching CT entries will be dropped.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ct_class_attr_enable, enable_);
}


/** Get ct_class/enable_ct_expect attribute.
 *
 * Get Enable CT_Expected lookup.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[out]  enable_ct_expect_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_enable_ct_expect_get(bdmf_object_handle mo_, bdmf_boolean *enable_ct_expect_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ct_class_attr_enable_ct_expect, &_nn_);
    *enable_ct_expect_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set ct_class/enable_ct_expect attribute.
 *
 * Set Enable CT_Expected lookup.
 * \param[in]   mo_ ct_class object handle or mattr transaction handle
 * \param[in]   enable_ct_expect_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ct_class_enable_ct_expect_set(bdmf_object_handle mo_, bdmf_boolean enable_ct_expect_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ct_class_attr_enable_ct_expect, enable_ct_expect_);
}

/** @} end of ct_class Doxygen group */




#endif /* _RDPA_AG_CT_CLASS_H_ */
