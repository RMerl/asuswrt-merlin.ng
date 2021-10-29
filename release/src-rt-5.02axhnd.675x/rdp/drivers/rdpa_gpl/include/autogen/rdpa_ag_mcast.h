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
 * mcast object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_MCAST_H_
#define _RDPA_AG_MCAST_H_

/** \addtogroup mcast
 * @{
 */


/** Get mcast type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a mcast object.
 * \return mcast type handle
 */
bdmf_type_handle rdpa_mcast_drv(void);

/* mcast: Attribute types */
typedef enum {
    rdpa_mcast_attr_nflows = 0, /* nflows : R : number : Number of configured Multicast flows */
    rdpa_mcast_attr_flow_idx_pool_ptr = 1, /* flow_idx_pool_ptr : RI : pointer : Flow ID Pool Virtual Address */
    rdpa_mcast_attr_flow_disp_pool_ptr = 2, /* flow_disp_pool_ptr : RI : pointer : Flow Display Pool Virtual Address */
    rdpa_mcast_attr_flow = 3, /* flow : RWADF : aggregate[] mcast_flow(rdpa_mcast_flow_t) : Multicast flow entry */
    rdpa_mcast_attr_flow_stat = 4, /* flow_stat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : Multicast flow entry statistics */
} rdpa_mcast_attr_types;

extern int (*f_rdpa_mcast_get)(bdmf_object_handle *pmo);

/** Get mcast object.

 * This function returns mcast object instance.
 * \param[out] mcast_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_mcast_get(bdmf_object_handle *mcast_obj);

/** Get mcast/nflows attribute.
 *
 * Get Number of configured Multicast flows.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[out]  nflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_nflows_get(bdmf_object_handle mo_, bdmf_number *nflows_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_mcast_attr_nflows, &_nn_);
    *nflows_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get mcast/flow_idx_pool_ptr attribute.
 *
 * Get Flow ID Pool Virtual Address.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[out]  flow_idx_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_idx_pool_ptr_get(bdmf_object_handle mo_, void * *flow_idx_pool_ptr_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_mcast_attr_flow_idx_pool_ptr, &_nn_);
    *flow_idx_pool_ptr_ = (void *)(long)_nn_;
    return _rc_;
}


/** Set mcast/flow_idx_pool_ptr attribute.
 *
 * Set Flow ID Pool Virtual Address.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[in]   flow_idx_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_idx_pool_ptr_set(bdmf_object_handle mo_, void * flow_idx_pool_ptr_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_mcast_attr_flow_idx_pool_ptr, (long)flow_idx_pool_ptr_);
}


/** Get mcast/flow_disp_pool_ptr attribute.
 *
 * Get Flow Display Pool Virtual Address.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[out]  flow_disp_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_disp_pool_ptr_get(bdmf_object_handle mo_, void * *flow_disp_pool_ptr_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_mcast_attr_flow_disp_pool_ptr, &_nn_);
    *flow_disp_pool_ptr_ = (void *)(long)_nn_;
    return _rc_;
}


/** Set mcast/flow_disp_pool_ptr attribute.
 *
 * Set Flow Display Pool Virtual Address.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[in]   flow_disp_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_disp_pool_ptr_set(bdmf_object_handle mo_, void * flow_disp_pool_ptr_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_mcast_attr_flow_disp_pool_ptr, (long)flow_disp_pool_ptr_);
}


/** Get mcast/flow attribute entry.
 *
 * Get Multicast flow entry.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_mcast_flow_t * flow_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_mcast_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Set mcast/flow attribute entry.
 *
 * Set Multicast flow entry.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_mcast_flow_t * flow_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_mcast_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Add mcast/flow attribute entry.
 *
 * Add Multicast flow entry.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_mcast_flow_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_mcast_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Delete mcast/flow attribute entry.
 *
 * Delete Multicast flow entry.
 * \param[in]   mo_ mcast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_mcast_attr_flow, (bdmf_index)ai_);
}


/** Find mcast/flow attribute entry.
 *
 * Find Multicast flow entry.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_mcast_flow_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_mcast_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Get mcast/flow_stat attribute entry.
 *
 * Get Multicast flow entry statistics.
 * \param[in]   mo_ mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_mcast_flow_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * flow_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_mcast_attr_flow_stat, (bdmf_index)ai_, flow_stat_, sizeof(*flow_stat_));
}

/** @} end of mcast Doxygen group */




#endif /* _RDPA_AG_MCAST_H_ */
