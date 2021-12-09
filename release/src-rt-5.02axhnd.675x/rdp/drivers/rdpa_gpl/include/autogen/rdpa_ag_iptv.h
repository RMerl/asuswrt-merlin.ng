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
 * iptv object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_IPTV_H_
#define _RDPA_AG_IPTV_H_

/** \addtogroup iptv
 * @{
 */


/** Get iptv type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an iptv object.
 * \return iptv type handle
 */
bdmf_type_handle rdpa_iptv_drv(void);

/* iptv: Attribute types */
typedef enum {
    rdpa_iptv_attr_lookup_method = 0, /* lookup_method : RW : enum : IPTV Lookup Method */
    rdpa_iptv_attr_mcast_prefix_filter = 1, /* mcast_prefix_filter : RW : enum :  Multicast Prefix Filtering Method */
    rdpa_iptv_attr_lookup_miss_action = 2, /* lookup_miss_action : RW : enum : Multicast iptv lookup miss action */
    rdpa_iptv_attr_iptv_stat = 3, /* iptv_stat : RW : aggregate iptv_stat(rdpa_iptv_stat_t) : IPTV global statistics */
    rdpa_iptv_attr_channel_request = 4, /* channel_request : WAD : aggregate iptv_channel_request(rdpa_iptv_channel_request_t) : Request to view the channel (reflecting  */
    rdpa_iptv_attr_channel = 5, /* channel : RF : aggregate[] iptv_channel(rdpa_iptv_channel_t) : IPTV channels table */
    rdpa_iptv_attr_channel_pm_stats = 6, /* channel_pm_stats : RF : aggregate[] rdpa_stat(rdpa_stat_t) : IPTV channels Performance Monitoring statistics */
    rdpa_iptv_attr_flush = 7, /* flush : W : bool : Flush IPTV table (remove all configured channels) */
} rdpa_iptv_attr_types;

extern int (*f_rdpa_iptv_get)(bdmf_object_handle *pmo);

/** Get iptv object.

 * This function returns iptv object instance.
 * \param[out] iptv_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_iptv_get(bdmf_object_handle *iptv_obj);

/** Get iptv/lookup_method attribute.
 *
 * Get IPTV Lookup Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  lookup_method_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_lookup_method_get(bdmf_object_handle mo_, rdpa_iptv_lookup_method *lookup_method_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_iptv_attr_lookup_method, &_nn_);
    *lookup_method_ = (rdpa_iptv_lookup_method)_nn_;
    return _rc_;
}


/** Set iptv/lookup_method attribute.
 *
 * Set IPTV Lookup Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   lookup_method_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_lookup_method_set(bdmf_object_handle mo_, rdpa_iptv_lookup_method lookup_method_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_iptv_attr_lookup_method, lookup_method_);
}


/** Get iptv/mcast_prefix_filter attribute.
 *
 * Get  Multicast Prefix Filtering Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  mcast_prefix_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_mcast_prefix_filter_get(bdmf_object_handle mo_, rdpa_mcast_filter_method *mcast_prefix_filter_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_iptv_attr_mcast_prefix_filter, &_nn_);
    *mcast_prefix_filter_ = (rdpa_mcast_filter_method)_nn_;
    return _rc_;
}


/** Set iptv/mcast_prefix_filter attribute.
 *
 * Set  Multicast Prefix Filtering Method.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   mcast_prefix_filter_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_mcast_prefix_filter_set(bdmf_object_handle mo_, rdpa_mcast_filter_method mcast_prefix_filter_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_iptv_attr_mcast_prefix_filter, mcast_prefix_filter_);
}


/** Get iptv/lookup_miss_action attribute.
 *
 * Get Multicast iptv lookup miss action.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  lookup_miss_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_lookup_miss_action_get(bdmf_object_handle mo_, rdpa_forward_action *lookup_miss_action_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_iptv_attr_lookup_miss_action, &_nn_);
    *lookup_miss_action_ = (rdpa_forward_action)_nn_;
    return _rc_;
}


/** Set iptv/lookup_miss_action attribute.
 *
 * Set Multicast iptv lookup miss action.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   lookup_miss_action_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_lookup_miss_action_set(bdmf_object_handle mo_, rdpa_forward_action lookup_miss_action_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_iptv_attr_lookup_miss_action, lookup_miss_action_);
}


/** Get iptv/iptv_stat attribute.
 *
 * Get IPTV global statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[out]  iptv_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_iptv_stat_get(bdmf_object_handle mo_, rdpa_iptv_stat_t * iptv_stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_iptv_attr_iptv_stat, iptv_stat_, sizeof(*iptv_stat_));
}


/** Set iptv/iptv_stat attribute.
 *
 * Set IPTV global statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   iptv_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_iptv_stat_set(bdmf_object_handle mo_, const rdpa_iptv_stat_t * iptv_stat_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_iptv_attr_iptv_stat, iptv_stat_, sizeof(*iptv_stat_));
}


/** Set iptv/channel_request attribute.
 *
 * Set Request to view the channel (reflecting IGMP JOIN/LEAVE membership reports).
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   channel_request_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_request_set(bdmf_object_handle mo_, const rdpa_iptv_channel_request_t * channel_request_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_iptv_attr_channel_request, channel_request_, sizeof(*channel_request_));
}


/** Add iptv/channel_request attribute.
 *
 * Add Request to view the channel (reflecting IGMP JOIN/LEAVE membership reports).
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   channel_request_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_request_add(bdmf_object_handle mo_, rdpa_channel_req_key_t * ai_, const rdpa_iptv_channel_request_t * channel_request_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_iptv_attr_channel_request, (bdmf_index *)ai_, channel_request_, sizeof(*channel_request_));
    return rc;
}


/** Delete iptv/channel_request attribute entry.
 *
 * Delete Request to view the channel (reflecting IGMP JOIN/LEAVE membership reports).
 * \param[in]   mo_ iptv object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_request_delete(bdmf_object_handle mo_, const rdpa_channel_req_key_t * ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_iptv_attr_channel_request, (bdmf_index)ai_);
}


/** Get iptv/channel attribute entry.
 *
 * Get IPTV channels table.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  channel_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_channel_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_iptv_channel_t * channel_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_iptv_attr_channel, (bdmf_index)ai_, channel_, sizeof(*channel_));
}


/** Get next iptv/channel attribute entry.
 *
 * Get next IPTV channels table.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_channel_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_iptv_attr_channel, (bdmf_index *)ai_);
}


/** Find iptv/channel attribute entry.
 *
 * Find IPTV channels table.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   channel_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_channel_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_iptv_channel_t * channel_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_iptv_attr_channel, (bdmf_index *)ai_, channel_, sizeof(*channel_));
    return rc;
}


/** Get iptv/channel_pm_stats attribute entry.
 *
 * Get IPTV channels Performance Monitoring statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  channel_pm_stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_pm_stats_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * channel_pm_stats_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_iptv_attr_channel_pm_stats, (bdmf_index)ai_, channel_pm_stats_, sizeof(*channel_pm_stats_));
}


/** Get next iptv/channel_pm_stats attribute entry.
 *
 * Get next IPTV channels Performance Monitoring statistics.
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_iptv_channel_pm_stats_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_iptv_attr_channel_pm_stats, (bdmf_index *)ai_);
}


/** Set iptv/flush attribute.
 *
 * Set Flush IPTV table (remove all configured channels).
 * \param[in]   mo_ iptv object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_iptv_flush_set(bdmf_object_handle mo_, bdmf_boolean flush_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_iptv_attr_flush, flush_);
}

/** @} end of iptv Doxygen group */




#endif /* _RDPA_AG_IPTV_H_ */
