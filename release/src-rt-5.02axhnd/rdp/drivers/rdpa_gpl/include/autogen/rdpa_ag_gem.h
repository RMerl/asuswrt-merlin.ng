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
 * gem object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_GEM_H_
#define _RDPA_AG_GEM_H_

/** \addtogroup gem
 * @{
 */


/** Get gem type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a gem object.
 * \return gem type handle
 */
bdmf_type_handle rdpa_gem_drv(void);

/* gem: Attribute types */
typedef enum {
    rdpa_gem_attr_index = 0, /* index : KRI : number : GEM index */
    rdpa_gem_attr_gem_port = 1, /* gem_port : MRI : number : GEM port ID */
    rdpa_gem_attr_flow_type = 2, /* flow_type : RI : enum : GEM flow type */
    rdpa_gem_attr_ds_def_flow = 3, /* ds_def_flow : RW : aggregate classification_result(rdpa_ic_result_t ) : Downstream Default flow configuration */
    rdpa_gem_attr_port_action = 4, /* port_action : RWF : aggregate[(rdpa_if)] gem_port_action(rdpa_gem_port_action_t) : Per port Vlan Action configuration */
    rdpa_gem_attr_us_cfg = 5, /* us_cfg : RW : aggregate gem_us_cfg(rdpa_gem_flow_us_cfg_t) : Upstream GEM configuration */
    rdpa_gem_attr_ds_cfg = 6, /* ds_cfg : RW : aggregate ds_cfg(rdpa_gem_flow_ds_cfg_t) : downstream GEM configuration */
    rdpa_gem_attr_stat = 7, /* stat : RW : aggregate gem_stat(rdpa_gem_stat_t) : GEM statistics */
} rdpa_gem_attr_types;

extern int (*f_rdpa_gem_get)(bdmf_number index_, bdmf_object_handle *pmo);

/** Get gem object by key.

 * This function returns gem object instance by key.
 * \param[in] index_    Object key
 * \param[out] gem_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_gem_get(bdmf_number index_, bdmf_object_handle *gem_obj);

/** Get gem/index attribute.
 *
 * Get GEM index.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_gem_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_gem_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set gem/index attribute.
 *
 * Set GEM index.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_gem_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_gem_attr_index, index_);
}


/** Get gem/gem_port attribute.
 *
 * Get GEM port ID.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[out]  gem_port_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_gem_gem_port_get(bdmf_object_handle mo_, bdmf_number *gem_port_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_gem_attr_gem_port, &_nn_);
    *gem_port_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set gem/gem_port attribute.
 *
 * Set GEM port ID.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   gem_port_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_gem_gem_port_set(bdmf_object_handle mo_, bdmf_number gem_port_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_gem_attr_gem_port, gem_port_);
}


/** Get gem/flow_type attribute.
 *
 * Get GEM flow type.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[out]  flow_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_gem_flow_type_get(bdmf_object_handle mo_, rdpa_gem_flow_type *flow_type_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_gem_attr_flow_type, &_nn_);
    *flow_type_ = (rdpa_gem_flow_type)_nn_;
    return _rc_;
}


/** Set gem/flow_type attribute.
 *
 * Set GEM flow type.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   flow_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_gem_flow_type_set(bdmf_object_handle mo_, rdpa_gem_flow_type flow_type_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_gem_attr_flow_type, flow_type_);
}


/** Get gem/ds_def_flow attribute.
 *
 * Get Downstream Default flow configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[out]  ds_def_flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_ds_def_flow_get(bdmf_object_handle mo_, rdpa_ic_result_t  * ds_def_flow_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_gem_attr_ds_def_flow, ds_def_flow_, sizeof(*ds_def_flow_));
}


/** Set gem/ds_def_flow attribute.
 *
 * Set Downstream Default flow configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   ds_def_flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_ds_def_flow_set(bdmf_object_handle mo_, const rdpa_ic_result_t  * ds_def_flow_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_gem_attr_ds_def_flow, ds_def_flow_, sizeof(*ds_def_flow_));
}


/** Get gem/port_action attribute entry.
 *
 * Get Per port Vlan Action configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  port_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_port_action_get(bdmf_object_handle mo_, rdpa_if ai_, rdpa_gem_port_action_t * port_action_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_gem_attr_port_action, (bdmf_index)ai_, port_action_, sizeof(*port_action_));
}


/** Set gem/port_action attribute entry.
 *
 * Set Per port Vlan Action configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   port_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_port_action_set(bdmf_object_handle mo_, rdpa_if ai_, const rdpa_gem_port_action_t * port_action_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_gem_attr_port_action, (bdmf_index)ai_, port_action_, sizeof(*port_action_));
}


/** Get gem/us_cfg attribute.
 *
 * Get Upstream GEM configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[out]  us_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_us_cfg_get(bdmf_object_handle mo_, rdpa_gem_flow_us_cfg_t * us_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_gem_attr_us_cfg, us_cfg_, sizeof(*us_cfg_));
}


/** Set gem/us_cfg attribute.
 *
 * Set Upstream GEM configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   us_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_us_cfg_set(bdmf_object_handle mo_, const rdpa_gem_flow_us_cfg_t * us_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_gem_attr_us_cfg, us_cfg_, sizeof(*us_cfg_));
}


/** Get gem/ds_cfg attribute.
 *
 * Get downstream GEM configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[out]  ds_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_ds_cfg_get(bdmf_object_handle mo_, rdpa_gem_flow_ds_cfg_t * ds_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_gem_attr_ds_cfg, ds_cfg_, sizeof(*ds_cfg_));
}


/** Set gem/ds_cfg attribute.
 *
 * Set downstream GEM configuration.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   ds_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_ds_cfg_set(bdmf_object_handle mo_, const rdpa_gem_flow_ds_cfg_t * ds_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_gem_attr_ds_cfg, ds_cfg_, sizeof(*ds_cfg_));
}


/** Get gem/stat attribute.
 *
 * Get GEM statistics.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_stat_get(bdmf_object_handle mo_, rdpa_gem_stat_t * stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_gem_attr_stat, stat_, sizeof(*stat_));
}


/** Set gem/stat attribute.
 *
 * Set GEM statistics.
 * \param[in]   mo_ gem object handle or mattr transaction handle
 * \param[in]   stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_gem_stat_set(bdmf_object_handle mo_, const rdpa_gem_stat_t * stat_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_gem_attr_stat, stat_, sizeof(*stat_));
}

/** @} end of gem Doxygen group */




#endif /* _RDPA_AG_GEM_H_ */
