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
 * ucast object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_UCAST_H_
#define _RDPA_AG_UCAST_H_

/** \addtogroup ucast
 * @{
 */


/** Get ucast type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an ucast object.
 * \return ucast type handle
 */
bdmf_type_handle rdpa_ucast_drv(void);

/* ucast: Attribute types */
typedef enum {
    rdpa_ucast_attr_nflows = 0, /* nflows : R : number/4 : number of configured 5-tuple based IP flows */
    rdpa_ucast_attr_flow = 1, /* flow : RWADF : aggregate/158[16512] ip_flow_info(rdpa_ip_flow_info_t) : 5-tuple based IP flow entry */
    rdpa_ucast_attr_flow_stat = 2, /* flow_stat : RF : aggregate/8[16512] rdpa_stat(rdpa_stat_t) : 5-tuple based IP flow entry statistics */
    rdpa_ucast_attr_ip_addresses_table = 3, /* ip_addresses_table : RWADF : aggregate/44[4] ip_addresses_table(rdpa_ip_addresses_table_t) : IP Addresses Table Entry */
    rdpa_ucast_attr_ds_wan_udp_filter = 4, /* ds_wan_udp_filter : RWADF : aggregate/16[32] ds_wan_udp_filter_entry(rdpa_ds_wan_udp_filter_t) : DS WAN UDP Filter (Drop on hi */
    rdpa_ucast_attr_ipv4_host_address_table = 5, /* ipv4_host_address_table : RWADF : aggregate/6[8] ipv4_host_address_table_entry(rdpa_ipv4_host_address_table_t) : IPv4 Host Add */
    rdpa_ucast_attr_ipv6_host_address_table = 6, /* ipv6_host_address_table : RWADF : aggregate/18[8] ipv6_host_address_table_entry(rdpa_ipv6_host_address_table_t) : IPv6 Host Ad */
    rdpa_ucast_attr_host_mac_address_table = 7, /* host_mac_address_table : RWADF : aggregate/8[8] host_mac_address_table_entry(rdpa_host_mac_address_table_t) : Host MAC Address */
    rdpa_ucast_attr_fc_accel_mode_table = 8, /* fc_accel_mode_table : RW : aggregate/4 fc_accel_mode_entry(rdpa_fc_accel_mode_t) : Flow Cache Acceleration Mode (Layer3, Layer */
} rdpa_ucast_attr_types;

extern int (*f_rdpa_ucast_get)(bdmf_object_handle *pmo);

/** Get ucast object.

 * This function returns ucast object instance.
 * \param[out] ucast_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_ucast_get(bdmf_object_handle *ucast_obj);

/** Get ucast/nflows attribute.
 *
 * Get number of configured 5-tuple based IP flows.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[out]  nflows_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_nflows_get(bdmf_object_handle mo_, bdmf_number *nflows_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ucast_attr_nflows, &_nn_);
    *nflows_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get ucast/flow attribute entry.
 *
 * Get 5-tuple based IP flow entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ucast_flow_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ip_flow_info_t * flow_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ucast_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Set ucast/flow attribute entry.
 *
 * Set 5-tuple based IP flow entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ucast_flow_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_ip_flow_info_t * flow_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ucast_attr_flow, (bdmf_index)ai_, flow_, sizeof(*flow_));
}


/** Add ucast/flow attribute entry.
 *
 * Add 5-tuple based IP flow entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ucast_flow_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ip_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ucast_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Delete ucast/flow attribute entry.
 *
 * Delete 5-tuple based IP flow entry.
 * \param[in]   mo_ ucast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ucast_flow_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ucast_attr_flow, (bdmf_index)ai_);
}


/** Find ucast/flow attribute entry.
 *
 * Find 5-tuple based IP flow entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   flow_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ucast_flow_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_ip_flow_info_t * flow_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_ucast_attr_flow, (bdmf_index *)ai_, flow_, sizeof(*flow_));
    return rc;
}


/** Get ucast/flow_stat attribute entry.
 *
 * Get 5-tuple based IP flow entry statistics.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  flow_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ucast_flow_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * flow_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ucast_attr_flow_stat, (bdmf_index)ai_, flow_stat_, sizeof(*flow_stat_));
}


/** Get ucast/ip_addresses_table attribute entry.
 *
 * Get IP Addresses Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ip_addresses_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ip_addresses_table_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ip_addresses_table_t * ip_addresses_table_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ucast_attr_ip_addresses_table, (bdmf_index)ai_, ip_addresses_table_, sizeof(*ip_addresses_table_));
}


/** Add ucast/ip_addresses_table attribute entry.
 *
 * Add IP Addresses Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   ip_addresses_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ip_addresses_table_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ip_addresses_table_t * ip_addresses_table_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ucast_attr_ip_addresses_table, (bdmf_index *)ai_, ip_addresses_table_, sizeof(*ip_addresses_table_));
    return rc;
}


/** Delete ucast/ip_addresses_table attribute entry.
 *
 * Delete IP Addresses Table Entry.
 * \param[in]   mo_ ucast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ip_addresses_table_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ucast_attr_ip_addresses_table, (bdmf_index)ai_);
}


/** Get ucast/ds_wan_udp_filter attribute entry.
 *
 * Get DS WAN UDP Filter (Drop on hits, Pass on misses).
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ds_wan_udp_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ds_wan_udp_filter_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ds_wan_udp_filter_t * ds_wan_udp_filter_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ucast_attr_ds_wan_udp_filter, (bdmf_index)ai_, ds_wan_udp_filter_, sizeof(*ds_wan_udp_filter_));
}


/** Add ucast/ds_wan_udp_filter attribute entry.
 *
 * Add DS WAN UDP Filter (Drop on hits, Pass on misses).
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   ds_wan_udp_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ds_wan_udp_filter_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ds_wan_udp_filter_t * ds_wan_udp_filter_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ucast_attr_ds_wan_udp_filter, (bdmf_index *)ai_, ds_wan_udp_filter_, sizeof(*ds_wan_udp_filter_));
    return rc;
}


/** Delete ucast/ds_wan_udp_filter attribute entry.
 *
 * Delete DS WAN UDP Filter (Drop on hits, Pass on misses).
 * \param[in]   mo_ ucast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ds_wan_udp_filter_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ucast_attr_ds_wan_udp_filter, (bdmf_index)ai_);
}


/** Get ucast/ipv4_host_address_table attribute entry.
 *
 * Get IPv4 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ipv4_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv4_host_address_table_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ipv4_host_address_table_t * ipv4_host_address_table_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ucast_attr_ipv4_host_address_table, (bdmf_index)ai_, ipv4_host_address_table_, sizeof(*ipv4_host_address_table_));
}


/** Add ucast/ipv4_host_address_table attribute entry.
 *
 * Add IPv4 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   ipv4_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv4_host_address_table_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ipv4_host_address_table_t * ipv4_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ucast_attr_ipv4_host_address_table, (bdmf_index *)ai_, ipv4_host_address_table_, sizeof(*ipv4_host_address_table_));
    return rc;
}


/** Delete ucast/ipv4_host_address_table attribute entry.
 *
 * Delete IPv4 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv4_host_address_table_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ucast_attr_ipv4_host_address_table, (bdmf_index)ai_);
}


/** Find ucast/ipv4_host_address_table attribute entry.
 *
 * Find IPv4 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   ipv4_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv4_host_address_table_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_ipv4_host_address_table_t * ipv4_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_ucast_attr_ipv4_host_address_table, (bdmf_index *)ai_, ipv4_host_address_table_, sizeof(*ipv4_host_address_table_));
    return rc;
}


/** Get ucast/ipv6_host_address_table attribute entry.
 *
 * Get IPv6 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ipv6_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv6_host_address_table_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_ipv6_host_address_table_t * ipv6_host_address_table_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ucast_attr_ipv6_host_address_table, (bdmf_index)ai_, ipv6_host_address_table_, sizeof(*ipv6_host_address_table_));
}


/** Add ucast/ipv6_host_address_table attribute entry.
 *
 * Add IPv6 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   ipv6_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv6_host_address_table_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_ipv6_host_address_table_t * ipv6_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ucast_attr_ipv6_host_address_table, (bdmf_index *)ai_, ipv6_host_address_table_, sizeof(*ipv6_host_address_table_));
    return rc;
}


/** Delete ucast/ipv6_host_address_table attribute entry.
 *
 * Delete IPv6 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv6_host_address_table_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ucast_attr_ipv6_host_address_table, (bdmf_index)ai_);
}


/** Find ucast/ipv6_host_address_table attribute entry.
 *
 * Find IPv6 Host Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   ipv6_host_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_ipv6_host_address_table_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_ipv6_host_address_table_t * ipv6_host_address_table_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_ucast_attr_ipv6_host_address_table, (bdmf_index *)ai_, ipv6_host_address_table_, sizeof(*ipv6_host_address_table_));
    return rc;
}


/** Get ucast/host_mac_address_table attribute entry.
 *
 * Get Host MAC Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  host_mac_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_host_mac_address_table_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_host_mac_address_table_t * host_mac_address_table_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ucast_attr_host_mac_address_table, (bdmf_index)ai_, host_mac_address_table_, sizeof(*host_mac_address_table_));
}


/** Add ucast/host_mac_address_table attribute entry.
 *
 * Add Host MAC Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   host_mac_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_host_mac_address_table_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_host_mac_address_table_t * host_mac_address_table_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_ucast_attr_host_mac_address_table, (bdmf_index *)ai_, host_mac_address_table_, sizeof(*host_mac_address_table_));
    return rc;
}


/** Delete ucast/host_mac_address_table attribute entry.
 *
 * Delete Host MAC Address Table Entry.
 * \param[in]   mo_ ucast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_host_mac_address_table_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_ucast_attr_host_mac_address_table, (bdmf_index)ai_);
}


/** Find ucast/host_mac_address_table attribute entry.
 *
 * Find Host MAC Address Table Entry.
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   host_mac_address_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_host_mac_address_table_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_host_mac_address_table_t * host_mac_address_table_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_ucast_attr_host_mac_address_table, (bdmf_index *)ai_, host_mac_address_table_, sizeof(*host_mac_address_table_));
    return rc;
}


/** Get ucast/fc_accel_mode_table attribute.
 *
 * Get Flow Cache Acceleration Mode (Layer3, Layer2 & 3).
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[out]  fc_accel_mode_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_fc_accel_mode_table_get(bdmf_object_handle mo_, rdpa_fc_accel_mode_t * fc_accel_mode_table_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_ucast_attr_fc_accel_mode_table, fc_accel_mode_table_, sizeof(*fc_accel_mode_table_));
}


/** Set ucast/fc_accel_mode_table attribute.
 *
 * Set Flow Cache Acceleration Mode (Layer3, Layer2 & 3).
 * \param[in]   mo_ ucast object handle or mattr transaction handle
 * \param[in]   fc_accel_mode_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_ucast_fc_accel_mode_table_set(bdmf_object_handle mo_, const rdpa_fc_accel_mode_t * fc_accel_mode_table_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_ucast_attr_fc_accel_mode_table, fc_accel_mode_table_, sizeof(*fc_accel_mode_table_));
}

/** @} end of ucast Doxygen group */




#endif /* _RDPA_AG_UCAST_H_ */
