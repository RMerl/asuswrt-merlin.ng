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
 * bridge object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_BRIDGE_H_
#define _RDPA_AG_BRIDGE_H_

/** \addtogroup bridge
 * @{
 */


/** Get bridge type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a bridge object.
 * \return bridge type handle
 */
bdmf_type_handle rdpa_bridge_drv(void);

/* bridge: Attribute types */
typedef enum {
    rdpa_bridge_attr_index = 0, /* index : KRI : number : Bridge index */
    rdpa_bridge_attr_config = 1, /* config : RI : aggregate bridge_cfg(rdpa_bridge_cfg_t) : Bridge configuration */
    rdpa_bridge_attr_mac = 2, /* mac : RWDF : aggregate[(fdb_key)] fdb_data(rdpa_fdb_data_t) : MAC table entry  */
    rdpa_bridge_attr_mac_status = 3, /* mac_status : RF : bool[(fdb_key)] : MAC entry status (true for active) */
    rdpa_bridge_attr_local_switch_enable = 4, /* local_switch_enable : RW : bool : Enable/Disable local switching on the bridge, when disabled, traffic from LAN to LAN interfa */
} rdpa_bridge_attr_types;

extern int (*f_rdpa_bridge_get)(bdmf_number index_, bdmf_object_handle *pmo);

/** Get bridge object by key.

 * This function returns bridge object instance by key.
 * \param[in] index_    Object key
 * \param[out] bridge_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_bridge_get(bdmf_number index_, bdmf_object_handle *bridge_obj);

/** Get bridge/index attribute.
 *
 * Get Bridge index.
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_bridge_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set bridge/index attribute.
 *
 * Set Bridge index.
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_bridge_attr_index, index_);
}


/** Get bridge/config attribute.
 *
 * Get Bridge configuration.
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[out]  config_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_config_get(bdmf_object_handle mo_, rdpa_bridge_cfg_t * config_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_bridge_attr_config, config_, sizeof(*config_));
}


/** Set bridge/config attribute.
 *
 * Set Bridge configuration.
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in]   config_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_config_set(bdmf_object_handle mo_, const rdpa_bridge_cfg_t * config_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_bridge_attr_config, config_, sizeof(*config_));
}


/** Get bridge/mac attribute entry.
 *
 * Get MAC table entry .
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  mac_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_mac_get(bdmf_object_handle mo_, rdpa_fdb_key_t * ai_, rdpa_fdb_data_t * mac_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_bridge_attr_mac, (bdmf_index)ai_, mac_, sizeof(*mac_));
}


/** Set bridge/mac attribute entry.
 *
 * Set MAC table entry .
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   mac_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_mac_set(bdmf_object_handle mo_, rdpa_fdb_key_t * ai_, const rdpa_fdb_data_t * mac_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_bridge_attr_mac, (bdmf_index)ai_, mac_, sizeof(*mac_));
}


/** Delete bridge/mac attribute entry.
 *
 * Delete MAC table entry .
 * \param[in]   mo_ bridge object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_mac_delete(bdmf_object_handle mo_, const rdpa_fdb_key_t * ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_bridge_attr_mac, (bdmf_index)ai_);
}


/** Get next bridge/mac attribute entry.
 *
 * Get next MAC table entry .
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_mac_get_next(bdmf_object_handle mo_, rdpa_fdb_key_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_bridge_attr_mac, (bdmf_index *)ai_);
}


/** Get bridge/mac_status attribute entry.
 *
 * Get MAC entry status (true for active).
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  mac_status_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_mac_status_get(bdmf_object_handle mo_, rdpa_fdb_key_t * ai_, bdmf_boolean *mac_status_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_bridge_attr_mac_status, (bdmf_index)ai_, &_nn_);
    *mac_status_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Get next bridge/mac_status attribute entry.
 *
 * Get next MAC entry status (true for active).
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_bridge_mac_status_get_next(bdmf_object_handle mo_, rdpa_fdb_key_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_bridge_attr_mac_status, (bdmf_index *)ai_);
}


/** Get bridge/local_switch_enable attribute.
 *
 * Get Enable/Disable local switching on the bridge, when disabled, traffic from LAN to LAN interface will be dropped.
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[out]  local_switch_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_bridge_local_switch_enable_get(bdmf_object_handle mo_, bdmf_boolean *local_switch_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_bridge_attr_local_switch_enable, &_nn_);
    *local_switch_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set bridge/local_switch_enable attribute.
 *
 * Set Enable/Disable local switching on the bridge, when disabled, traffic from LAN to LAN interface will be dropped.
 * \param[in]   mo_ bridge object handle or mattr transaction handle
 * \param[in]   local_switch_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_bridge_local_switch_enable_set(bdmf_object_handle mo_, bdmf_boolean local_switch_enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_bridge_attr_local_switch_enable, local_switch_enable_);
}

/** @} end of bridge Doxygen group */




#endif /* _RDPA_AG_BRIDGE_H_ */
