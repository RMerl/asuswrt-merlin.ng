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
 * ingress_class object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_INGRESS_CLASS_H_
#define _RDPA_AG_INGRESS_CLASS_H_

/** \addtogroup ingress_class
 * @{
 */


/** Get ingress_class type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an ingress_class object.
 * \return ingress_class type handle
 */
bdmf_type_handle rdpa_ingress_class_drv(void);

/* ingress_class: Attribute types */
typedef enum {
    rdpa_ingress_class_attr_dir = 0, /* dir : MKRI : enum : Traffic Direction */
    rdpa_ingress_class_attr_index = 1, /* index : KRI : number : Ingress class index */
    rdpa_ingress_class_attr_cfg = 2, /* cfg : MRI : aggregate class_configuration(rdpa_ic_cfg_t ) : Ingress class configuration */
    rdpa_ingress_class_attr_nflow = 3, /* nflow : R : number : Number of associated classification flows */
    rdpa_ingress_class_attr_flow = 4, /* flow : RWADF : aggregate[] class_info(rdpa_ic_info_t ) : Ingress class flow entry */
    rdpa_ingress_class_attr_flow_stat = 5, /* flow_stat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : Ingress class flow statistics (can be enabled in system object) */
    rdpa_ingress_class_attr_port_action = 6, /* port_action : RWF : aggregate[(port_action_key)] port_action(rdpa_port_action_t) : Per egress port action */
    rdpa_ingress_class_attr_flush = 7, /* flush : W : bool : Flush ingress class table (remove all configured flows) */
} rdpa_ingress_class_attr_types;

/** ingress_class object key. */
typedef struct {
    rdpa_traffic_dir dir; /**< ingress_class: Traffic Direction */
    bdmf_number index; /**< ingress_class: Ingress class index */
} rdpa_ingress_class_key_t;


extern int (*f_rdpa_ingress_class_get)(const rdpa_ingress_class_key_t * key_, bdmf_object_handle *pmo);

/** Get ingress_class object by key.

 * This function returns ingress_class object instance by key.
 * \param[in] key_    Object key
 * \param[out] ingress_class_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_ingress_class_get(const rdpa_ingress_class_key_t * key_, bdmf_object_handle *ingress_class_obj);

/** Get ingress_class/dir attribute.
 *
 * Get Traffic Direction.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[out]  dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_dir_get(bdmf_object_handle mo_, rdpa_traffic_dir *dir_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ingress_class_attr_dir, &_nn_);
    *dir_ = (rdpa_traffic_dir)_nn_;
    return _rc_;
}


/** Set ingress_class/dir attribute.
 *
 * Set Traffic Direction.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_dir_set(bdmf_object_handle mo_, rdpa_traffic_dir dir_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ingress_class_attr_dir, dir_);
}


/** Get ingress_class/index attribute.
 *
 * Get Ingress class index.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ingress_class_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set ingress_class/index attribute.
 *
 * Set Ingress class index.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ingress_class_attr_index, index_);
}


/** Get ingress_class/cfg attribute.
 *
 * Get Ingress class configuration.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_cfg_get(bdmf_object_handle mo_, rdpa_ic_cfg_t  * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_ingress_class_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set ingress_class/cfg attribute.
 *
 * Set Ingress class configuration.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_cfg_set(bdmf_object_handle mo_, const rdpa_ic_cfg_t  * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_ingress_class_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get ingress_class/nflow attribute.
 *
 * Get Number of associated classification flows.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[out]  nflow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_nflow_get(bdmf_object_handle mo_, bdmf_number *nflow_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ingress_class_attr_nflow, &_nn_);
    *nflow_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get ingress_class/flow attribute entry.
 *
 * Get Ingress class flow entry.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_flow_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ic_info_t  * flow_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ingress_class_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Set ingress_class/flow attribute entry.
 *
 * Set Ingress class flow entry.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_flow_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_ic_info_t  * flow_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ingress_class_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Add ingress_class/flow attribute entry.
 *
 * Add Ingress class flow entry.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_flow_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ic_info_t  * flow_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ingress_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Delete ingress_class/flow attribute entry.
 *
 * Delete Ingress class flow entry.
 * \param[in]   mo_ ingress_class object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_flow_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ingress_class_attr_flow, (bdmf_index)ai_);
}


/** Get next ingress_class/flow attribute entry.
 *
 * Get next Ingress class flow entry.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_flow_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_ingress_class_attr_flow, (bdmf_index *)ai_);
}


/** Find ingress_class/flow attribute entry.
 *
 * Find Ingress class flow entry.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_flow_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_ic_info_t  * flow_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_ingress_class_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Get ingress_class/flow_stat attribute entry.
 *
 * Get Ingress class flow statistics (can be enabled in system object).
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_flow_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * flow_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ingress_class_attr_flow_stat, (bdmf_index)ai_, flow_stat_, sizeof(*flow_stat_));
}


/** Get next ingress_class/flow_stat attribute entry.
 *
 * Get next Ingress class flow statistics (can be enabled in system object).
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ingress_class_flow_stat_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_ingress_class_attr_flow_stat, (bdmf_index *)ai_);
}


/** Get ingress_class/port_action attribute entry.
 *
 * Get Per egress port action.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  port_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_port_action_get(bdmf_object_handle mo_, rdpa_port_action_key_t * ai_, rdpa_port_action_t * port_action_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ingress_class_attr_port_action, (bdmf_index)ai_, port_action_, sizeof(*port_action_));
}


/** Set ingress_class/port_action attribute entry.
 *
 * Set Per egress port action.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   port_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_port_action_set(bdmf_object_handle mo_, rdpa_port_action_key_t * ai_, const rdpa_port_action_t * port_action_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ingress_class_attr_port_action, (bdmf_index)ai_, port_action_, sizeof(*port_action_));
}


/** Get next ingress_class/port_action attribute entry.
 *
 * Get next Per egress port action.
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_port_action_get_next(bdmf_object_handle mo_, rdpa_port_action_key_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_ingress_class_attr_port_action, (bdmf_index *)ai_);
}


/** Set ingress_class/flush attribute.
 *
 * Set Flush ingress class table (remove all configured flows).
 * \param[in]   mo_ ingress_class object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ingress_class_flush_set(bdmf_object_handle mo_, bdmf_boolean flush_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ingress_class_attr_flush, flush_);
}

/** @} end of ingress_class Doxygen group */




#endif /* _RDPA_AG_INGRESS_CLASS_H_ */
