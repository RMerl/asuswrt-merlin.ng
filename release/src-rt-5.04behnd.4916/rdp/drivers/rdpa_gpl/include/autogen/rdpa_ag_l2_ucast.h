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
 * l2_ucast object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_L2_UCAST_H_
#define _RDPA_AG_L2_UCAST_H_

/** \addtogroup l2_ucast
 * @{
 */


/** Get l2_ucast type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a l2_ucast object.
 * \return l2_ucast type handle
 */
bdmf_type_handle rdpa_l2_ucast_drv(void);

/* l2_ucast: Attribute types */
typedef enum {
    rdpa_l2_ucast_attr_nflows = 0, /* nflows : R : number : number of configured L2 flows */
    rdpa_l2_ucast_attr_flow_idx_pool_ptr = 1, /* flow_idx_pool_ptr : RI : pointer : Flow ID Pool Virtual Address */
    rdpa_l2_ucast_attr_flow_disp_pool_ptr = 2, /* flow_disp_pool_ptr : RI : pointer : Flow Display Pool Virtual Address */
    rdpa_l2_ucast_attr_flow = 3, /* flow : RWADF : aggregate[] l2_flow_info(rdpa_l2_flow_info_t) : l2_ucast flow entry */
    rdpa_l2_ucast_attr_flow_stat = 4, /* flow_stat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : l2_ucast flow entry statistics */
    rdpa_l2_ucast_attr_pathstat = 5, /* pathstat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : l2_ucast path entry statistics */
} rdpa_l2_ucast_attr_types;

extern int (*f_rdpa_l2_ucast_get)(bdmf_object_handle *pmo);

/** Get l2_ucast object.

 * This function returns l2_ucast object instance.
 * \param[out] l2_ucast_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_l2_ucast_get(bdmf_object_handle *l2_ucast_obj);

/** Get l2_ucast/nflows attribute.
 *
 * Get number of configured L2 flows.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[out]  nflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_l2_ucast_nflows_get(bdmf_object_handle mo_, bdmf_number *nflows_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_l2_ucast_attr_nflows, &_nn_);
    *nflows_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get l2_ucast/flow_idx_pool_ptr attribute.
 *
 * Get Flow ID Pool Virtual Address.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[out]  flow_idx_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_idx_pool_ptr_get(bdmf_object_handle mo_, void * *flow_idx_pool_ptr_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_l2_ucast_attr_flow_idx_pool_ptr, &_nn_);
    *flow_idx_pool_ptr_ = (void *)(long)_nn_;
    return _rc_;
}


/** Set l2_ucast/flow_idx_pool_ptr attribute.
 *
 * Set Flow ID Pool Virtual Address.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in]   flow_idx_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_idx_pool_ptr_set(bdmf_object_handle mo_, void * flow_idx_pool_ptr_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_l2_ucast_attr_flow_idx_pool_ptr, (long)flow_idx_pool_ptr_);
}


/** Get l2_ucast/flow_disp_pool_ptr attribute.
 *
 * Get Flow Display Pool Virtual Address.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[out]  flow_disp_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_disp_pool_ptr_get(bdmf_object_handle mo_, void * *flow_disp_pool_ptr_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_l2_ucast_attr_flow_disp_pool_ptr, &_nn_);
    *flow_disp_pool_ptr_ = (void *)(long)_nn_;
    return _rc_;
}


/** Set l2_ucast/flow_disp_pool_ptr attribute.
 *
 * Set Flow Display Pool Virtual Address.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in]   flow_disp_pool_ptr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_disp_pool_ptr_set(bdmf_object_handle mo_, void * flow_disp_pool_ptr_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_l2_ucast_attr_flow_disp_pool_ptr, (long)flow_disp_pool_ptr_);
}


/** Get l2_ucast/flow attribute entry.
 *
 * Get l2_ucast flow entry.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_l2_flow_info_t * flow_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_l2_ucast_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Set l2_ucast/flow attribute entry.
 *
 * Set l2_ucast flow entry.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_l2_flow_info_t * flow_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_l2_ucast_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Add l2_ucast/flow attribute entry.
 *
 * Add l2_ucast flow entry.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_l2_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_l2_ucast_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Delete l2_ucast/flow attribute entry.
 *
 * Delete l2_ucast flow entry.
 * \param[in]   mo_ l2_ucast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_l2_ucast_attr_flow, (bdmf_index)ai_);
}


/** Get next l2_ucast/flow attribute entry.
 *
 * Get next l2_ucast flow entry.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_l2_ucast_attr_flow, (bdmf_index *)ai_);
}


/** Find l2_ucast/flow attribute entry.
 *
 * Find l2_ucast flow entry.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_l2_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_l2_ucast_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Get l2_ucast/flow_stat attribute entry.
 *
 * Get l2_ucast flow entry statistics.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * flow_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_l2_ucast_attr_flow_stat, (bdmf_index)ai_, flow_stat_, sizeof(*flow_stat_));
}


/** Get next l2_ucast/flow_stat attribute entry.
 *
 * Get next l2_ucast flow entry statistics.
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_flow_stat_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_l2_ucast_attr_flow_stat, (bdmf_index *)ai_);
}


/** Get l2_ucast/pathstat attribute entry.
 *
 * Get l2_ucast path entry statistics.

 * \deprecated This function has been deprecated.
 *
 * \param[in]   mo_ l2_ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pathstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_l2_ucast_pathstat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * pathstat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_l2_ucast_attr_pathstat, (bdmf_index)ai_, pathstat_, sizeof(*pathstat_));
}

/** @} end of l2_ucast Doxygen group */




#endif /* _RDPA_AG_L2_UCAST_H_ */
