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
 * mllid object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_MLLID_H_
#define _RDPA_AG_MLLID_H_

/** \addtogroup mllid
 * @{
 */


/** Get mllid type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a mllid object.
 * \return mllid type handle
 */
bdmf_type_handle rdpa_mllid_drv(void);

/* mllid: Attribute types */
typedef enum {
    rdpa_mllid_attr_index = 0, /* index : KRI : number : MLLID index */
    rdpa_mllid_attr_flow_id = 1, /* flow_id : MRI : number : Flow index */
    rdpa_mllid_attr_enable = 2, /* enable : RW : bool : Enable MLLID service */
    rdpa_mllid_attr_stat = 3, /* stat : RW : aggregate mllid_stat(rdpa_mllid_stat_t) : MLLID statistics */
} rdpa_mllid_attr_types;

extern int (*f_rdpa_mllid_get)(bdmf_number index_, bdmf_object_handle *pmo);

/** Get mllid object by key.

 * This function returns mllid object instance by key.
 * \param[in] index_    Object key
 * \param[out] mllid_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_mllid_get(bdmf_number index_, bdmf_object_handle *mllid_obj);

/** Get mllid/index attribute.
 *
 * Get MLLID index.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mllid_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_mllid_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set mllid/index attribute.
 *
 * Set MLLID index.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mllid_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_mllid_attr_index, index_);
}


/** Get mllid/flow_id attribute.
 *
 * Get Flow index.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[out]  flow_id_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mllid_flow_id_get(bdmf_object_handle mo_, bdmf_number *flow_id_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_mllid_attr_flow_id, &_nn_);
    *flow_id_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set mllid/flow_id attribute.
 *
 * Set Flow index.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[in]   flow_id_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mllid_flow_id_set(bdmf_object_handle mo_, bdmf_number flow_id_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_mllid_attr_flow_id, flow_id_);
}


/** Get mllid/enable attribute.
 *
 * Get Enable MLLID service.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mllid_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_mllid_attr_enable, &_nn_);
    *enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set mllid/enable attribute.
 *
 * Set Enable MLLID service.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mllid_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_mllid_attr_enable, enable_);
}


/** Get mllid/stat attribute.
 *
 * Get MLLID statistics.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mllid_stat_get(bdmf_object_handle mo_, rdpa_mllid_stat_t * stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_mllid_attr_stat, stat_, sizeof(*stat_));
}


/** Set mllid/stat attribute.
 *
 * Set MLLID statistics.
 * \param[in]   mo_ mllid object handle or mattr transaction handle
 * \param[in]   stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mllid_stat_set(bdmf_object_handle mo_, const rdpa_mllid_stat_t * stat_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_mllid_attr_stat, stat_, sizeof(*stat_));
}

/** @} end of mllid Doxygen group */




#endif /* _RDPA_AG_MLLID_H_ */
