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
    rdpa_port_attr_name = 0, /* name : MKRW : string : Port name */
    rdpa_port_attr_index = 1, /* index : MRI : number : Port index */
    rdpa_port_attr_is_wan = 2, /* is_wan : RW : bool : Is WAN */
    rdpa_port_attr_type = 3, /* type : RI : enum : port type */
    rdpa_port_attr_handle = 4, /* handle : RI : number : enet handle */
    rdpa_port_attr_speed = 5, /* speed : RI : enum : Active Ethernet Speed */
    rdpa_port_attr_cfg = 6, /* cfg : RW : aggregate port_dp(rdpa_port_dp_cfg_t) : Logical port configuration */
    rdpa_port_attr_tm_cfg = 7, /* tm_cfg : RW : aggregate port_tm(rdpa_port_tm_cfg_t) : TM configuration */
    rdpa_port_attr_def_flow = 8, /* def_flow : RW : aggregate classification_result(rdpa_ic_result_t ) : DS default flow classification. Wifi: last default flow w */
    rdpa_port_attr_stat = 9, /* stat : RW : aggregate port_stat(rdpa_port_stat_t) : Port statistics */
    rdpa_port_attr_flow_control = 12, /* flow_control : RW : aggregate port_flow_control(rdpa_port_flow_ctrl_t) : The port flow control */
    rdpa_port_attr_ingress_rate_limit = 13, /* ingress_rate_limit : RW : aggregate port_ingress_rate_limit(rdpa_port_ingress_rate_limit_t) : Port ingress rate limiting */
    rdpa_port_attr_rl_overhead = 14, /* rl_overhead : RW : number : Rate limit overhead - will be added to packet len before rate limit */
    rdpa_port_attr_mirror_cfg = 15, /* mirror_cfg : RW : aggregate port_mirror_cfg(rdpa_port_mirror_cfg_t) : Port mirroring configuration */
    rdpa_port_attr_bufmng_cfg = 16, /* bufmng_cfg : RW : aggregate port_bufmng_cfg(rdpa_port_bufmng_cfg_t) : Port buffer management configuration */
    rdpa_port_attr_vlan_isolation = 17, /* vlan_isolation : RW : aggregate vlan_isolation(rdpa_port_vlan_isolation_t) : LAN port VLAN isolation control */
    rdpa_port_attr_loopback = 18, /* loopback : RW : aggregate port_loopback_conf(rdpa_port_loopback_t) : Port loopbacks */
    rdpa_port_attr_max_pkt_size = 19, /* max_pkt_size : RW : number : Port maximal packet size */
    rdpa_port_attr_cpu_obj = 20, /* cpu_obj : RW : object : CPU object for exception/forwarding-through-cpu packets */
    rdpa_port_attr_pbit_to_queue = 21, /* pbit_to_queue : R : object : Handle to linked pbit to queue object */
    rdpa_port_attr_cpu_meter = 22, /* cpu_meter : RWF : number[(rdpa_port_meter_t)] : Index of per-port CPU meter */
    rdpa_port_attr_ingress_filter = 23, /* ingress_filter : RWF : aggregate[(rdpa_filter)] filter_ctrl(rdpa_filter_ctrl_t) : Ingress filter configuration per Port object */
    rdpa_port_attr_protocol_filters = 24, /* protocol_filters : RW : enum_mask : Protocol Filters define allowed traffic type */
    rdpa_port_attr_enable = 25, /* enable : RW : bool : Enable object */
    rdpa_port_attr_is_empty = 26, /* is_empty : R : bool : check if PORT is empty  */
    rdpa_port_attr_uninit = 27, /* uninit : W : bool : Port uninit: disable port and flush port related packets */
    rdpa_port_attr_mac = 28, /* mac : RW : ether_addr : PORT MAC address  */
    rdpa_port_attr_pkt_size_stat_en = 29, /* pkt_size_stat_en : RW : bool : Enable debug statistics */
    rdpa_port_attr_pbit_to_dp_map = 30, /* pbit_to_dp_map : RWF : enum[] : Pbit to Discard Priority map for Ingress QoS (low/high) */
    rdpa_port_attr_options = 31, /* options : RW : number : reserved */
    rdpa_port_attr_pfc_tx_enable = 32, /* pfc_tx_enable : RW : bool : reserved */
    rdpa_port_attr_pfc_tx_timer = 33, /* pfc_tx_timer : RWF : number[] : reserved */
} rdpa_port_attr_types;

extern int (*f_rdpa_port_get)(const char * name_, bdmf_object_handle *pmo);

/** Get port object by key.

 * This function returns port object instance by key.
 * \param[in] name_    Object key
 * \param[out] port_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_port_get(const char * name_, bdmf_object_handle *port_obj);

/** Get port/name attribute.
 *
 * Get Port name.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  name_ Attribute value
 * \param[in]   size_ buffer size
 * \return number of bytes read >=0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_name_get(bdmf_object_handle mo_, char * name_, uint32_t size_)
{
    return bdmf_attr_get_as_string(mo_, rdpa_port_attr_name, name_, size_);
}


/** Set port/name attribute.
 *
 * Set Port name.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   name_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_name_set(bdmf_object_handle mo_, const char * name_)
{
    return bdmf_attr_set_as_string(mo_, rdpa_port_attr_name, name_);
}


/** Get port/index attribute.
 *
 * Get Port index.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
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
static inline int rdpa_port_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_index, index_);
}


/** Get port/is_wan attribute.
 *
 * Get Is WAN.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  is_wan_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_is_wan_get(bdmf_object_handle mo_, bdmf_boolean *is_wan_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_is_wan, &_nn_);
    *is_wan_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set port/is_wan attribute.
 *
 * Set Is WAN.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   is_wan_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_is_wan_set(bdmf_object_handle mo_, bdmf_boolean is_wan_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_is_wan, is_wan_);
}


/** Get port/type attribute.
 *
 * Get port type.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_type_get(bdmf_object_handle mo_, rdpa_port_type *type_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_type, &_nn_);
    *type_ = (rdpa_port_type)_nn_;
    return _rc_;
}


/** Set port/type attribute.
 *
 * Set port type.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_type_set(bdmf_object_handle mo_, rdpa_port_type type_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_type, type_);
}


/** Get port/handle attribute.
 *
 * Get enet handle.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  handle_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_handle_get(bdmf_object_handle mo_, bdmf_number *handle_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_handle, &_nn_);
    *handle_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/handle attribute.
 *
 * Set enet handle.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   handle_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_handle_set(bdmf_object_handle mo_, bdmf_number handle_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_handle, handle_);
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


/** Get port/rl_overhead attribute.
 *
 * Get Rate limit overhead - will be added to packet len before rate limit.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  rl_overhead_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_rl_overhead_get(bdmf_object_handle mo_, bdmf_number *rl_overhead_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_rl_overhead, &_nn_);
    *rl_overhead_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/rl_overhead attribute.
 *
 * Set Rate limit overhead - will be added to packet len before rate limit.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   rl_overhead_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_rl_overhead_set(bdmf_object_handle mo_, bdmf_number rl_overhead_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_rl_overhead, rl_overhead_);
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


/** Get port/bufmng_cfg attribute.
 *
 * Get Port buffer management configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  bufmng_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_bufmng_cfg_get(bdmf_object_handle mo_, rdpa_port_bufmng_cfg_t * bufmng_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_port_attr_bufmng_cfg, bufmng_cfg_, sizeof(*bufmng_cfg_));
}


/** Set port/bufmng_cfg attribute.
 *
 * Set Port buffer management configuration.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   bufmng_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_bufmng_cfg_set(bdmf_object_handle mo_, const rdpa_port_bufmng_cfg_t * bufmng_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_port_attr_bufmng_cfg, bufmng_cfg_, sizeof(*bufmng_cfg_));
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


/** Get port/max_pkt_size attribute.
 *
 * Get Port maximal packet size.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  max_pkt_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_max_pkt_size_get(bdmf_object_handle mo_, bdmf_number *max_pkt_size_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_max_pkt_size, &_nn_);
    *max_pkt_size_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/max_pkt_size attribute.
 *
 * Set Port maximal packet size.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   max_pkt_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_max_pkt_size_set(bdmf_object_handle mo_, bdmf_number max_pkt_size_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_max_pkt_size, max_pkt_size_);
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


/** Get port/pbit_to_queue attribute.
 *
 * Get Handle to linked pbit to queue object.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  pbit_to_queue_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pbit_to_queue_get(bdmf_object_handle mo_, bdmf_object_handle *pbit_to_queue_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_pbit_to_queue, &_nn_);
    *pbit_to_queue_ = (bdmf_object_handle)(long)_nn_;
    return _rc_;
}


/** Get port/cpu_meter attribute entry.
 *
 * Get Index of per-port CPU meter.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  cpu_meter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cpu_meter_get(bdmf_object_handle mo_, rdpa_port_meter_t ai_, bdmf_number *cpu_meter_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_port_attr_cpu_meter, (bdmf_index)ai_, &_nn_);
    *cpu_meter_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/cpu_meter attribute entry.
 *
 * Set Index of per-port CPU meter.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   cpu_meter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_cpu_meter_set(bdmf_object_handle mo_, rdpa_port_meter_t ai_, bdmf_number cpu_meter_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_port_attr_cpu_meter, (bdmf_index)ai_, cpu_meter_);
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


/** Set port/uninit attribute.
 *
 * Set Port uninit: disable port and flush port related packets.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   uninit_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_port_uninit_set(bdmf_object_handle mo_, bdmf_boolean uninit_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_uninit, uninit_);
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


/** Get port/pbit_to_dp_map attribute entry.
 *
 * Get Pbit to Discard Priority map for Ingress QoS (low/high).
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pbit_to_dp_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pbit_to_dp_map_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_discard_prty *pbit_to_dp_map_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_port_attr_pbit_to_dp_map, (bdmf_index)ai_, &_nn_);
    *pbit_to_dp_map_ = (rdpa_discard_prty)_nn_;
    return _rc_;
}


/** Set port/pbit_to_dp_map attribute entry.
 *
 * Set Pbit to Discard Priority map for Ingress QoS (low/high).
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   pbit_to_dp_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pbit_to_dp_map_set(bdmf_object_handle mo_, bdmf_index ai_, rdpa_discard_prty pbit_to_dp_map_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_port_attr_pbit_to_dp_map, (bdmf_index)ai_, pbit_to_dp_map_);
}


/** Get port/options attribute.
 *
 * Get reserved.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  options_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_options_get(bdmf_object_handle mo_, bdmf_number *options_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_options, &_nn_);
    *options_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/options attribute.
 *
 * Set reserved.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   options_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_options_set(bdmf_object_handle mo_, bdmf_number options_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_options, options_);
}


/** Get port/pfc_tx_enable attribute.
 *
 * Get reserved.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[out]  pfc_tx_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pfc_tx_enable_get(bdmf_object_handle mo_, bdmf_boolean *pfc_tx_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_port_attr_pfc_tx_enable, &_nn_);
    *pfc_tx_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set port/pfc_tx_enable attribute.
 *
 * Set reserved.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   pfc_tx_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pfc_tx_enable_set(bdmf_object_handle mo_, bdmf_boolean pfc_tx_enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_port_attr_pfc_tx_enable, pfc_tx_enable_);
}


/** Get port/pfc_tx_timer attribute entry.
 *
 * Get reserved.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pfc_tx_timer_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pfc_tx_timer_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *pfc_tx_timer_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_port_attr_pfc_tx_timer, (bdmf_index)ai_, &_nn_);
    *pfc_tx_timer_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set port/pfc_tx_timer attribute entry.
 *
 * Set reserved.
 * \param[in]   mo_ port object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   pfc_tx_timer_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_port_pfc_tx_timer_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number pfc_tx_timer_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_port_attr_pfc_tx_timer, (bdmf_index)ai_, pfc_tx_timer_);
}

/** @} end of port Doxygen group */




#endif /* _RDPA_AG_PORT_H_ */
