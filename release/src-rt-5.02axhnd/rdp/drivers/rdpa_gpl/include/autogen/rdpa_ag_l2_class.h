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
 * l2_class object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_L2_CLASS_H_
#define _RDPA_AG_L2_CLASS_H_

/** \addtogroup l2_class
 * @{
 */


/** Get l2_class type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a l2_class object.
 * \return l2_class type handle
 */
bdmf_type_handle rdpa_l2_class_drv(void);

/* l2_class: Attribute types */
typedef enum {
    rdpa_l2_class_attr_key_exclude_fields = 0, /* key_exclude_fields : RW : enum_mask : List of fields to exclude from basic L2 class key */
    rdpa_l2_class_attr_nflows = 1, /* nflows : R : number : number of configured L2 flows */
    rdpa_l2_class_attr_flow = 2, /* flow : RWADF : aggregate[] l2_flow_info(rdpa_l2_flow_info_t) : L2 flow entry */
    rdpa_l2_class_attr_flow_stat = 3, /* flow_stat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : L2 flow entry statistics */
    rdpa_l2_class_attr_flush = 4, /* flush : W : bool : Flush flows */
    rdpa_l2_class_attr_pathstat = 5, /* pathstat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : L2 class path entry statistics */
} rdpa_l2_class_attr_types;

extern int (*f_rdpa_l2_class_get)(bdmf_object_handle *pmo);

/** Get l2_class object.

 * This function returns l2_class object instance.
 * \param[out] l2_class_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_l2_class_get(bdmf_object_handle *l2_class_obj);

/** Get l2_class/key_exclude_fields attribute.
 *
 * Get List of fields to exclude from basic L2 class key.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[out]  key_exclude_fields_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_l2_class_key_exclude_fields_get(bdmf_object_handle mo_, rdpa_l2_flow_key_exclude_fields_t *key_exclude_fields_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_l2_class_attr_key_exclude_fields, &_nn_);
    *key_exclude_fields_ = (rdpa_l2_flow_key_exclude_fields_t)_nn_;
    return _rc_;
}


/** Set l2_class/key_exclude_fields attribute.
 *
 * Set List of fields to exclude from basic L2 class key.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in]   key_exclude_fields_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_l2_class_key_exclude_fields_set(bdmf_object_handle mo_, rdpa_l2_flow_key_exclude_fields_t key_exclude_fields_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_l2_class_attr_key_exclude_fields, key_exclude_fields_);
}


/** Get l2_class/nflows attribute.
 *
 * Get number of configured L2 flows.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[out]  nflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_l2_class_nflows_get(bdmf_object_handle mo_, bdmf_number *nflows_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_l2_class_attr_nflows, &_nn_);
    *nflows_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get l2_class/flow attribute entry.
 *
 * Get L2 flow entry.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_l2_flow_info_t * flow_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_l2_class_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Set l2_class/flow attribute entry.
 *
 * Set L2 flow entry.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_l2_flow_info_t * flow_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_l2_class_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Add l2_class/flow attribute entry.
 *
 * Add L2 flow entry.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_l2_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_l2_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Delete l2_class/flow attribute entry.
 *
 * Delete L2 flow entry.
 * \param[in]   mo_ l2_class object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_l2_class_attr_flow, (bdmf_index)ai_);
}


/** Get next l2_class/flow attribute entry.
 *
 * Get next L2 flow entry.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_l2_class_attr_flow, (bdmf_index *)ai_);
}


/** Find l2_class/flow attribute entry.
 *
 * Find L2 flow entry.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_l2_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_l2_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Get l2_class/flow_stat attribute entry.
 *
 * Get L2 flow entry statistics.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * flow_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_l2_class_attr_flow_stat, (bdmf_index)ai_, flow_stat_, sizeof(*flow_stat_));
}


/** Get next l2_class/flow_stat attribute entry.
 *
 * Get next L2 flow entry statistics.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_flow_stat_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_l2_class_attr_flow_stat, (bdmf_index *)ai_);
}


/** Set l2_class/flush attribute.
 *
 * Set Flush flows.
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_l2_class_flush_set(bdmf_object_handle mo_, bdmf_boolean flush_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_l2_class_attr_flush, flush_);
}


/** Get l2_class/pathstat attribute entry.
 *
 * Get L2 class path entry statistics.

 * \deprecated This function has been deprecated.
 *
 * \param[in]   mo_ l2_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pathstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_class_pathstat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * pathstat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_l2_class_attr_pathstat, (bdmf_index)ai_, pathstat_, sizeof(*pathstat_));
}

/** @} end of l2_class Doxygen group */




#endif /* _RDPA_AG_L2_CLASS_H_ */
