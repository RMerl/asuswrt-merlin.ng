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
 * port object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_PORT_H_
#define _RDPA_AG_PORT_H_

/** \addtogroup port
 * @{
 */


/** Get port type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a port object.
 * \return port type handle
 */
bdmf_type_handle rdpa_port_drv(void);

/* port: Attribute types */
typedef enum {
    rdpa_port_attr_index = 0, /* index : MKRI : enum : Port index */
    rdpa_port_attr_wan_type = 1, /* wan_type : RI : enum : Wan type */
    rdpa_port_attr_speed = 2, /* speed : RI : enum : Active Ethernet Speed */
    rdpa_port_attr_cfg = 3, /* cfg : RW : aggregate port_dp(rdpa_port_dp_cfg_t) : Logical port configuration */
    rdpa_port_attr_tm_cfg = 4, /* tm_cfg : RW : aggregate port_tm(rdpa_port_tm_cfg_t) : TM configuration */
    rdpa_port_attr_sa_limit = 5, /* sa_limit : RW : aggregate port_sa_limit(rdpa_port_sa_limit_t) : SA limit configuration */
    rdpa_port_attr_def_flow = 6, /* def_flow : RW : aggregate classification_result(rdpa_ic_result_t ) : DS default flow classification. Wifi: last default flow w */
    rdpa_port_attr_stat = 7, /* stat : RW : aggregate port_stat(rdpa_port_stat_t) : Port statistics */
    rdpa_port_attr_flow_control = 10, /* flow_control : RW : aggregate port_flow_control(rdpa_port_flow_ctrl_t) : The port flow control */
    rdpa_port_attr_ingress_rate_limit = 11, /* ingress_rate_limit : RW : aggregate port_ingress_rate_limit(rdpa_port_ingress_rate_limit_t) : Port ingress rate limiting */
    rdpa_port_attr_mirror_cfg = 12, /* mirror_cfg : RW : aggregate port_mirror_cfg(rdpa_port_mirror_cfg_t) : Port mirroring configuration */
    rdpa_port_attr_vlan_isolation = 13, /* vlan_isolation : RW : aggregate vlan_isolation(rdpa_port_vlan_isolation_t) : LAN port VLAN isolation control */
    rdpa_port_attr_transparent = 14, /* transparent : RW : bool : LAN port transparent control */
    rdpa_port_attr_loopback = 15, /* loopback : RW : aggregate port_loopback_conf(rdpa_port_loopback_t) : Port loopbacks */
    rdpa_port_attr_mtu_size = 16, /* mtu_size : RW : number : Port MTU */
    rdpa_port_attr_cpu_obj = 17, /* cpu_obj : RW : object : CPU object for exception/forwarding-through-cpu packets */
    rdpa_port_attr_cpu_meter = 18, /* cpu_meter : RW : number : Index of per-port CPU meter */
    rdpa_port_attr_ingress_filter = 19, /* ingress_filter : RWF : aggregate[(rdpa_filter)] filter_ctrl(rdpa_filter_ctrl_t) : Ingress filter configuration per Port object */
    rdpa_port_attr_protocol_filters = 20, /* protocol_filters : RW : enum_mask : Protocol Filters define allowed traffic type */
    rdpa_port_attr_enable = 21, /* enable : RW : bool : Enable object */
    rdpa_port_attr_is_empty = 22, /* is_empty : R : bool : check if PORT is empty  */
    rdpa_port_attr_mac = 23, /* mac : RW : ether_addr : PORT MAC address  */
    rdpa_port_attr_pkt_size_stat_en = 24, /* pkt_size_stat_en : RW : bool : Enable debug statistics */
} rdpa_port_attr_types;

extern int (*f_rdpa_port_get)(rdpa_if index_, bdmf_object_handle *pmo);

/** Get port object by key.

 * This function returns port object instance by key.
 * \param[in] index_    Object key
 * \param[out] port_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_port_get(rdpa_if index_, bdmf_object_handle *port_obj);

/** Get port/index attribute.
 *
 * Get Port index.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_index_get(bdmf_object_handle mo_, rdpa_if *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_index, &_nn_);
    *index_ = (rdpa_if)_nn_;
    return _rc_;
}


/** Set port/index attribute.
 *
 * Set Port index.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_index_set(bdmf_object_handle mo_, rdpa_if index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_index, index_);
}


/** Get port/wan_type attribute.
 *
 * Get Wan type.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  wan_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_wan_type_get(bdmf_object_handle mo_, rdpa_wan_type *wan_type_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_wan_type, &_nn_);
    *wan_type_ = (rdpa_wan_type)_nn_;
    return _rc_;
}


/** Set port/wan_type attribute.
 *
 * Set Wan type.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   wan_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_wan_type_set(bdmf_object_handle mo_, rdpa_wan_type wan_type_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_wan_type, wan_type_);
}


/** Get port/speed attribute.
 *
 * Get Active Ethernet Speed.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  speed_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_speed_get(bdmf_object_handle mo_, rdpa_speed_type *speed_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_speed, &_nn_);
    *speed_ = (rdpa_speed_type)_nn_;
    return _rc_;
}


/** Set port/speed attribute.
 *
 * Set Active Ethernet Speed.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   speed_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_speed_set(bdmf_object_handle mo_, rdpa_speed_type speed_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_speed, speed_);
}


/** Get port/cfg attribute.
 *
 * Get Logical port configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cfg_get(bdmf_object_handle mo_, rdpa_port_dp_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set port/cfg attribute.
 *
 * Set Logical port configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cfg_set(bdmf_object_handle mo_, const rdpa_port_dp_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get port/tm_cfg attribute.
 *
 * Get TM configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  tm_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_tm_cfg_get(bdmf_object_handle mo_, rdpa_port_tm_cfg_t * tm_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_tm_cfg, tm_cfg_, sizeof(*tm_cfg_));
}


/** Set port/tm_cfg attribute.
 *
 * Set TM configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   tm_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_tm_cfg_set(bdmf_object_handle mo_, const rdpa_port_tm_cfg_t * tm_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_tm_cfg, tm_cfg_, sizeof(*tm_cfg_));
}


/** Get port/sa_limit attribute.
 *
 * Get SA limit configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  sa_limit_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_sa_limit_get(bdmf_object_handle mo_, rdpa_port_sa_limit_t * sa_limit_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_sa_limit, sa_limit_, sizeof(*sa_limit_));
}


/** Set port/sa_limit attribute.
 *
 * Set SA limit configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   sa_limit_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_sa_limit_set(bdmf_object_handle mo_, const rdpa_port_sa_limit_t * sa_limit_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_sa_limit, sa_limit_, sizeof(*sa_limit_));
}


/** Get port/def_flow attribute.
 *
 * Get DS default flow classification. Wifi: last default flow will be applied for all wifi ports.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  def_flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_def_flow_get(bdmf_object_handle mo_, rdpa_ic_result_t  * def_flow_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_def_flow, def_flow_, sizeof(*def_flow_));
}


/** Set port/def_flow attribute.
 *
 * Set DS default flow classification. Wifi: last default flow will be applied for all wifi ports.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   def_flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_def_flow_set(bdmf_object_handle mo_, const rdpa_ic_result_t  * def_flow_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_def_flow, def_flow_, sizeof(*def_flow_));
}


/** Get port/stat attribute.
 *
 * Get Port statistics.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_stat_get(bdmf_object_handle mo_, rdpa_port_stat_t * stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_stat, stat_, sizeof(*stat_));
}


/** Set port/stat attribute.
 *
 * Set Port statistics.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_stat_set(bdmf_object_handle mo_, const rdpa_port_stat_t * stat_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_stat, stat_, sizeof(*stat_));
}


/** Get port/flow_control attribute.
 *
 * Get The port flow control.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  flow_control_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_flow_control_get(bdmf_object_handle mo_, rdpa_port_flow_ctrl_t * flow_control_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_flow_control, flow_control_, sizeof(*flow_control_));
}


/** Set port/flow_control attribute.
 *
 * Set The port flow control.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   flow_control_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_flow_control_set(bdmf_object_handle mo_, const rdpa_port_flow_ctrl_t * flow_control_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_flow_control, flow_control_, sizeof(*flow_control_));
}


/** Get port/ingress_rate_limit attribute.
 *
 * Get Port ingress rate limiting.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  ingress_rate_limit_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_ingress_rate_limit_get(bdmf_object_handle mo_, rdpa_port_ingress_rate_limit_t * ingress_rate_limit_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_ingress_rate_limit, ingress_rate_limit_, sizeof(*ingress_rate_limit_));
}


/** Set port/ingress_rate_limit attribute.
 *
 * Set Port ingress rate limiting.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ingress_rate_limit_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_ingress_rate_limit_set(bdmf_object_handle mo_, const rdpa_port_ingress_rate_limit_t * ingress_rate_limit_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_ingress_rate_limit, ingress_rate_limit_, sizeof(*ingress_rate_limit_));
}


/** Get port/mirror_cfg attribute.
 *
 * Get Port mirroring configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  mirror_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_mirror_cfg_get(bdmf_object_handle mo_, rdpa_port_mirror_cfg_t * mirror_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_mirror_cfg, mirror_cfg_, sizeof(*mirror_cfg_));
}


/** Set port/mirror_cfg attribute.
 *
 * Set Port mirroring configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   mirror_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_mirror_cfg_set(bdmf_object_handle mo_, const rdpa_port_mirror_cfg_t * mirror_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_mirror_cfg, mirror_cfg_, sizeof(*mirror_cfg_));
}


/** Get port/vlan_isolation attribute.
 *
 * Get LAN port VLAN isolation control.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  vlan_isolation_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_vlan_isolation_get(bdmf_object_handle mo_, rdpa_port_vlan_isolation_t * vlan_isolation_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_vlan_isolation, vlan_isolation_, sizeof(*vlan_isolation_));
}


/** Set port/vlan_isolation attribute.
 *
 * Set LAN port VLAN isolation control.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   vlan_isolation_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_vlan_isolation_set(bdmf_object_handle mo_, const rdpa_port_vlan_isolation_t * vlan_isolation_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_vlan_isolation, vlan_isolation_, sizeof(*vlan_isolation_));
}


/** Get port/transparent attribute.
 *
 * Get LAN port transparent control.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  transparent_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_transparent_get(bdmf_object_handle mo_, bdmf_boolean *transparent_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_transparent, &_nn_);
    *transparent_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set port/transparent attribute.
 *
 * Set LAN port transparent control.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   transparent_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_transparent_set(bdmf_object_handle mo_, bdmf_boolean transparent_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_transparent, transparent_);
}


/** Get port/loopback attribute.
 *
 * Get Port loopbacks.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  loopback_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_loopback_get(bdmf_object_handle mo_, rdpa_port_loopback_t * loopback_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_loopback, loopback_, sizeof(*loopback_));
}


/** Set port/loopback attribute.
 *
 * Set Port loopbacks.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   loopback_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_loopback_set(bdmf_object_handle mo_, const rdpa_port_loopback_t * loopback_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_loopback, loopback_, sizeof(*loopback_));
}


/** Get port/mtu_size attribute.
 *
 * Get Port MTU.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  mtu_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_mtu_size_get(bdmf_object_handle mo_, bdmf_number *mtu_size_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_mtu_size, &_nn_);
    *mtu_size_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/mtu_size attribute.
 *
 * Set Port MTU.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   mtu_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_mtu_size_set(bdmf_object_handle mo_, bdmf_number mtu_size_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_mtu_size, mtu_size_);
}


/** Get port/cpu_obj attribute.
 *
 * Get CPU object for exception/forwarding-through-cpu packets.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  cpu_obj_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cpu_obj_get(bdmf_object_handle mo_, bdmf_object_handle *cpu_obj_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_cpu_obj, &_nn_);
    *cpu_obj_ = (bdmf_object_handle)(long)_nn_;
    return _rc_;
}


/** Set port/cpu_obj attribute.
 *
 * Set CPU object for exception/forwarding-through-cpu packets.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   cpu_obj_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cpu_obj_set(bdmf_object_handle mo_, bdmf_object_handle cpu_obj_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_cpu_obj, (long)cpu_obj_);
}


/** Get port/cpu_meter attribute.
 *
 * Get Index of per-port CPU meter.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  cpu_meter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cpu_meter_get(bdmf_object_handle mo_, bdmf_number *cpu_meter_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_cpu_meter, &_nn_);
    *cpu_meter_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/cpu_meter attribute.
 *
 * Set Index of per-port CPU meter.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   cpu_meter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cpu_meter_set(bdmf_object_handle mo_, bdmf_number cpu_meter_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_cpu_meter, cpu_meter_);
}


/** Get port/ingress_filter attribute entry.
 *
 * Get Ingress filter configuration per Port object.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ingress_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_ingress_filter_get(bdmf_object_handle mo_, rdpa_filter ai_, rdpa_filter_ctrl_t * ingress_filter_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_port_attr_ingress_filter, (bdmf_index)ai_, ingress_filter_, sizeof(*ingress_filter_));
}


/** Set port/ingress_filter attribute entry.
 *
 * Set Ingress filter configuration per Port object.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   ingress_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_ingress_filter_set(bdmf_object_handle mo_, rdpa_filter ai_, const rdpa_filter_ctrl_t * ingress_filter_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_port_attr_ingress_filter, (bdmf_index)ai_, ingress_filter_, sizeof(*ingress_filter_));
}


/** Get next port/ingress_filter attribute entry.
 *
 * Get next Ingress filter configuration per Port object.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_ingress_filter_get_next(bdmf_object_handle mo_, rdpa_filter * ai_)
{
    int rc;
    bdmf_index _ai_tmp_ = *ai_;
    rc = bdmf_attrelem_get_next(mo_, rdpa_port_attr_ingress_filter, (bdmf_index *)&_ai_tmp_);
    *ai_ = (rdpa_filter)_ai_tmp_;
    return rc;
}


/** Get port/protocol_filters attribute.
 *
 * Get Protocol Filters define allowed traffic type.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  protocol_filters_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_protocol_filters_get(bdmf_object_handle mo_, rdpa_proto_filters_mask_t *protocol_filters_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_protocol_filters, &_nn_);
    *protocol_filters_ = (rdpa_proto_filters_mask_t)_nn_;
    return _rc_;
}


/** Set port/protocol_filters attribute.
 *
 * Set Protocol Filters define allowed traffic type.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   protocol_filters_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_protocol_filters_set(bdmf_object_handle mo_, rdpa_proto_filters_mask_t protocol_filters_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_protocol_filters, protocol_filters_);
}


/** Get port/enable attribute.
 *
 * Get Enable object.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_enable, &_nn_);
    *enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set port/enable attribute.
 *
 * Set Enable object.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_enable, enable_);
}


/** Get port/is_empty attribute.
 *
 * Get check if PORT is empty .
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  is_empty_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_is_empty_get(bdmf_object_handle mo_, bdmf_boolean *is_empty_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_is_empty, &_nn_);
    *is_empty_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Get port/mac attribute.
 *
 * Get PORT MAC address .
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  mac_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_mac_get(bdmf_object_handle mo_, bdmf_mac_t * mac_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_mac, mac_, sizeof(*mac_));
}


/** Set port/mac attribute.
 *
 * Set PORT MAC address .
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   mac_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_mac_set(bdmf_object_handle mo_, const bdmf_mac_t * mac_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_mac, mac_, sizeof(*mac_));
}


/** Get port/pkt_size_stat_en attribute.
 *
 * Get Enable debug statistics.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  pkt_size_stat_en_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pkt_size_stat_en_get(bdmf_object_handle mo_, bdmf_boolean *pkt_size_stat_en_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_pkt_size_stat_en, &_nn_);
    *pkt_size_stat_en_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set port/pkt_size_stat_en attribute.
 *
 * Set Enable debug statistics.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   pkt_size_stat_en_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pkt_size_stat_en_set(bdmf_object_handle mo_, bdmf_boolean pkt_size_stat_en_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_pkt_size_stat_en, pkt_size_stat_en_);
}

/** @} end of port Doxygen group */




#endif /* _RDPA_AG_PORT_H_ */
