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
 * wlan_mcast object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_WLAN_MCAST_H_
#define _RDPA_AG_WLAN_MCAST_H_

/** \addtogroup wlan_mcast
 * @{
 */


/** Get wlan_mcast type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a wlan_mcast object.
 * \return wlan_mcast type handle
 */
bdmf_type_handle rdpa_wlan_mcast_drv(void);

/* wlan_mcast: Attribute types */
typedef enum {
    rdpa_wlan_mcast_attr_fwd_table_entries = 0, /* fwd_table_entries : R : number : Number of configured Forwarding Table entries */
    rdpa_wlan_mcast_attr_fwd_table = 1, /* fwd_table : RWADF : aggregate[] fwd_table_entry(rdpa_wlan_mcast_fwd_table_t) : Forwarding Table */
    rdpa_wlan_mcast_attr_dhd_station_entries = 2, /* dhd_station_entries : R : number : Number of configured DHD Stations */
    rdpa_wlan_mcast_attr_dhd_station = 3, /* dhd_station : RWADF : aggregate[] dhd_station_entry(rdpa_wlan_mcast_dhd_station_t) : DHD Station Table */
    rdpa_wlan_mcast_attr_ssid_mac_address_entries = 4, /* ssid_mac_address_entries : R : number : Number of configured SSID MAC Addresses */
    rdpa_wlan_mcast_attr_ssid_mac_address = 5, /* ssid_mac_address : RWADF : aggregate[] ssid_mac_address_entry(rdpa_wlan_mcast_ssid_mac_address_t) : SSID MAC Address Table */
    rdpa_wlan_mcast_attr_ssid_stats = 6, /* ssid_stats : RF : aggregate[] ssid_stats_entry(rdpa_wlan_mcast_ssid_stats_t) : SSID Statistics */
} rdpa_wlan_mcast_attr_types;

extern int (*f_rdpa_wlan_mcast_get)(bdmf_object_handle *pmo);

/** Get wlan_mcast object.

 * This function returns wlan_mcast object instance.
 * \param[out] wlan_mcast_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_wlan_mcast_get(bdmf_object_handle *wlan_mcast_obj);

/** Get wlan_mcast/fwd_table_entries attribute.
 *
 * Get Number of configured Forwarding Table entries.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[out]  fwd_table_entries_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_wlan_mcast_fwd_table_entries_get(bdmf_object_handle mo_, bdmf_number *fwd_table_entries_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_wlan_mcast_attr_fwd_table_entries, &_nn_);
    *fwd_table_entries_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get wlan_mcast/fwd_table attribute entry.
 *
 * Get Forwarding Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  fwd_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_fwd_table_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_wlan_mcast_fwd_table_t * fwd_table_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_wlan_mcast_attr_fwd_table, (bdmf_index)ai_, fwd_table_, sizeof(*fwd_table_));
}


/** Set wlan_mcast/fwd_table attribute entry.
 *
 * Set Forwarding Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   fwd_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_fwd_table_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_wlan_mcast_fwd_table_t * fwd_table_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_wlan_mcast_attr_fwd_table, (bdmf_index)ai_, fwd_table_, sizeof(*fwd_table_));
}


/** Add wlan_mcast/fwd_table attribute entry.
 *
 * Add Forwarding Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   fwd_table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_fwd_table_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_wlan_mcast_fwd_table_t * fwd_table_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_wlan_mcast_attr_fwd_table, (bdmf_index *)ai_, fwd_table_, sizeof(*fwd_table_));
    return rc;
}


/** Delete wlan_mcast/fwd_table attribute entry.
 *
 * Delete Forwarding Table.
 * \param[in]   mo_ wlan_mcast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_fwd_table_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_wlan_mcast_attr_fwd_table, (bdmf_index)ai_);
}


/** Get wlan_mcast/dhd_station_entries attribute.
 *
 * Get Number of configured DHD Stations.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[out]  dhd_station_entries_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_wlan_mcast_dhd_station_entries_get(bdmf_object_handle mo_, bdmf_number *dhd_station_entries_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_wlan_mcast_attr_dhd_station_entries, &_nn_);
    *dhd_station_entries_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get wlan_mcast/dhd_station attribute entry.
 *
 * Get DHD Station Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  dhd_station_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_dhd_station_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_wlan_mcast_dhd_station_t * dhd_station_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_wlan_mcast_attr_dhd_station, (bdmf_index)ai_, dhd_station_, sizeof(*dhd_station_));
}


/** Add wlan_mcast/dhd_station attribute entry.
 *
 * Add DHD Station Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   dhd_station_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_dhd_station_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_wlan_mcast_dhd_station_t * dhd_station_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_wlan_mcast_attr_dhd_station, (bdmf_index *)ai_, dhd_station_, sizeof(*dhd_station_));
    return rc;
}


/** Delete wlan_mcast/dhd_station attribute entry.
 *
 * Delete DHD Station Table.
 * \param[in]   mo_ wlan_mcast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_dhd_station_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_wlan_mcast_attr_dhd_station, (bdmf_index)ai_);
}


/** Find wlan_mcast/dhd_station attribute entry.
 *
 * Find DHD Station Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   dhd_station_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_dhd_station_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_wlan_mcast_dhd_station_t * dhd_station_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_wlan_mcast_attr_dhd_station, (bdmf_index *)ai_, dhd_station_, sizeof(*dhd_station_));
    return rc;
}


/** Get wlan_mcast/ssid_mac_address_entries attribute.
 *
 * Get Number of configured SSID MAC Addresses.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[out]  ssid_mac_address_entries_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_wlan_mcast_ssid_mac_address_entries_get(bdmf_object_handle mo_, bdmf_number *ssid_mac_address_entries_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_wlan_mcast_attr_ssid_mac_address_entries, &_nn_);
    *ssid_mac_address_entries_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get wlan_mcast/ssid_mac_address attribute entry.
 *
 * Get SSID MAC Address Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ssid_mac_address_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_ssid_mac_address_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_wlan_mcast_ssid_mac_address_t * ssid_mac_address_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_wlan_mcast_attr_ssid_mac_address, (bdmf_index)ai_, ssid_mac_address_, sizeof(*ssid_mac_address_));
}


/** Add wlan_mcast/ssid_mac_address attribute entry.
 *
 * Add SSID MAC Address Table.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   ssid_mac_address_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_ssid_mac_address_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_wlan_mcast_ssid_mac_address_t * ssid_mac_address_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_wlan_mcast_attr_ssid_mac_address, (bdmf_index *)ai_, ssid_mac_address_, sizeof(*ssid_mac_address_));
    return rc;
}


/** Delete wlan_mcast/ssid_mac_address attribute entry.
 *
 * Delete SSID MAC Address Table.
 * \param[in]   mo_ wlan_mcast object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_ssid_mac_address_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_wlan_mcast_attr_ssid_mac_address, (bdmf_index)ai_);
}


/** Get wlan_mcast/ssid_stats attribute entry.
 *
 * Get SSID Statistics.
 * \param[in]   mo_ wlan_mcast object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ssid_stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_wlan_mcast_ssid_stats_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_wlan_mcast_ssid_stats_t * ssid_stats_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_wlan_mcast_attr_ssid_stats, (bdmf_index)ai_, ssid_stats_, sizeof(*ssid_stats_));
}

/** @} end of wlan_mcast Doxygen group */




#endif /* _RDPA_AG_WLAN_MCAST_H_ */
