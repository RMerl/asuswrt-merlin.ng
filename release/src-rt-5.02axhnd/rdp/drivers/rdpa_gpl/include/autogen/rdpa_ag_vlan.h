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
 * vlan object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_VLAN_H_
#define _RDPA_AG_VLAN_H_

/** \addtogroup vlan
 * @{
 */


/** Get vlan type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a vlan object.
 * \return vlan type handle
 */
bdmf_type_handle rdpa_vlan_drv(void);

/* vlan: Attribute types */
typedef enum {
    rdpa_vlan_attr_name = 0, /* name : KRI : string : unique container name */
    rdpa_vlan_attr_vid_enable = 1, /* vid_enable : RWF : bool[] : VID enabled */
    rdpa_vlan_attr_ingress_filter = 2, /* ingress_filter : RWF : aggregate[(rdpa_filter)] filter_ctrl(rdpa_filter_ctrl_t) : Ingress filter configuration per VLAN object */
    rdpa_vlan_attr_mac_lookup_cfg = 3, /* mac_lookup_cfg : RW : aggregate mac_lookup_cfg(rdpa_mac_lookup_cfg_t) : SA/DA MAC lookup configuration */
    rdpa_vlan_attr_protocol_filters = 4, /* protocol_filters : RW : enum_mask : Protocol Filters define allowed traffic type */
    rdpa_vlan_attr_discard_prty = 5, /* discard_prty : RW : enum : Discard priority */
    rdpa_vlan_attr_stat = 6, /* stat : RW : aggregate rdpa_stat_tx_rx_valid(rdpa_stat_tx_rx_valid_t) : vlan statistics */
} rdpa_vlan_attr_types;

extern int (*f_rdpa_vlan_get)(const char * name_, bdmf_object_handle *pmo);

/** Get vlan object by key.

 * This function returns vlan object instance by key.
 * \param[in] name_    Object key
 * \param[out] vlan_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_vlan_get(const char * name_, bdmf_object_handle *vlan_obj);

/** Get vlan/name attribute.
 *
 * Get unique container name.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[out]  name_ Attribute value
 * \param[in]   size_ buffer size
 * \return number of bytes read >=0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_name_get(bdmf_object_handle mo_, char * name_, uint32_t size_)
{
    return bdmf_attr_get_as_string(mo_, rdpa_vlan_attr_name, name_, size_);
}


/** Set vlan/name attribute.
 *
 * Set unique container name.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   name_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_name_set(bdmf_object_handle mo_, const char * name_)
{
    return bdmf_attr_set_as_string(mo_, rdpa_vlan_attr_name, name_);
}


/** Get vlan/vid_enable attribute entry.
 *
 * Get VID enabled.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  vid_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_vid_enable_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean *vid_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_vlan_attr_vid_enable, (bdmf_index)ai_, &_nn_);
    *vid_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set vlan/vid_enable attribute entry.
 *
 * Set VID enabled.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   vid_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_vid_enable_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean vid_enable_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_vlan_attr_vid_enable, (bdmf_index)ai_, vid_enable_);
}


/** Get vlan/ingress_filter attribute entry.
 *
 * Get Ingress filter configuration per VLAN object.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ingress_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_ingress_filter_get(bdmf_object_handle mo_, rdpa_filter ai_, rdpa_filter_ctrl_t * ingress_filter_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_vlan_attr_ingress_filter, (bdmf_index)ai_, ingress_filter_, sizeof(*ingress_filter_));
}


/** Set vlan/ingress_filter attribute entry.
 *
 * Set Ingress filter configuration per VLAN object.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   ingress_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_ingress_filter_set(bdmf_object_handle mo_, rdpa_filter ai_, const rdpa_filter_ctrl_t * ingress_filter_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_vlan_attr_ingress_filter, (bdmf_index)ai_, ingress_filter_, sizeof(*ingress_filter_));
}


/** Get next vlan/ingress_filter attribute entry.
 *
 * Get next Ingress filter configuration per VLAN object.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_ingress_filter_get_next(bdmf_object_handle mo_, rdpa_filter * ai_)
{
    int rc;
    bdmf_index _ai_tmp_ = *ai_;
    rc = bdmf_attrelem_get_next(mo_, rdpa_vlan_attr_ingress_filter, (bdmf_index *)&_ai_tmp_);
    *ai_ = (rdpa_filter)_ai_tmp_;
    return rc;
}


/** Get vlan/mac_lookup_cfg attribute.
 *
 * Get SA/DA MAC lookup configuration.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[out]  mac_lookup_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_mac_lookup_cfg_get(bdmf_object_handle mo_, rdpa_mac_lookup_cfg_t * mac_lookup_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_vlan_attr_mac_lookup_cfg, mac_lookup_cfg_, sizeof(*mac_lookup_cfg_));
}


/** Set vlan/mac_lookup_cfg attribute.
 *
 * Set SA/DA MAC lookup configuration.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   mac_lookup_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_mac_lookup_cfg_set(bdmf_object_handle mo_, const rdpa_mac_lookup_cfg_t * mac_lookup_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_vlan_attr_mac_lookup_cfg, mac_lookup_cfg_, sizeof(*mac_lookup_cfg_));
}


/** Get vlan/protocol_filters attribute.
 *
 * Get Protocol Filters define allowed traffic type.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[out]  protocol_filters_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_protocol_filters_get(bdmf_object_handle mo_, rdpa_proto_filters_mask_t *protocol_filters_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_vlan_attr_protocol_filters, &_nn_);
    *protocol_filters_ = (rdpa_proto_filters_mask_t)_nn_;
    return _rc_;
}


/** Set vlan/protocol_filters attribute.
 *
 * Set Protocol Filters define allowed traffic type.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   protocol_filters_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_protocol_filters_set(bdmf_object_handle mo_, rdpa_proto_filters_mask_t protocol_filters_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_vlan_attr_protocol_filters, protocol_filters_);
}


/** Get vlan/discard_prty attribute.
 *
 * Get Discard priority.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[out]  discard_prty_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_discard_prty_get(bdmf_object_handle mo_, rdpa_discard_prty *discard_prty_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_vlan_attr_discard_prty, &_nn_);
    *discard_prty_ = (rdpa_discard_prty)_nn_;
    return _rc_;
}


/** Set vlan/discard_prty attribute.
 *
 * Set Discard priority.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   discard_prty_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_discard_prty_set(bdmf_object_handle mo_, rdpa_discard_prty discard_prty_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_vlan_attr_discard_prty, discard_prty_);
}


/** Get vlan/stat attribute.
 *
 * Get vlan statistics.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_stat_get(bdmf_object_handle mo_, rdpa_stat_tx_rx_valid_t * stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_vlan_attr_stat, stat_, sizeof(*stat_));
}


/** Set vlan/stat attribute.
 *
 * Set vlan statistics.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_stat_set(bdmf_object_handle mo_, const rdpa_stat_tx_rx_valid_t * stat_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_vlan_attr_stat, stat_, sizeof(*stat_));
}

/** @} end of vlan Doxygen group */




#endif /* _RDPA_AG_VLAN_H_ */
