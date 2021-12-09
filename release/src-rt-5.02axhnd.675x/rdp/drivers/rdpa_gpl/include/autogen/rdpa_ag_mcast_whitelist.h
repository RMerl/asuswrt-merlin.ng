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
 * mcast_whitelist object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_MCAST_WHITELIST_H_
#define _RDPA_AG_MCAST_WHITELIST_H_

/** \addtogroup mcast_whitelist
 * @{
 */


/** Get mcast_whitelist type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a mcast_whitelist object.
 * \return mcast_whitelist type handle
 */
bdmf_type_handle rdpa_mcast_whitelist_drv(void);

/* mcast_whitelist: Attribute types */
typedef enum {
    rdpa_mcast_whitelist_attr_entry = 0, /* entry : RWADF : aggregate[] mcast_whitelist(rdpa_mcast_whitelist_t) : Multicast whitelist entry */
    rdpa_mcast_whitelist_attr_port_enable = 1, /* port_enable : RWF : bool[(rdpa_if)] : Multicast whitelist port enable */
    rdpa_mcast_whitelist_attr_stat = 2, /* stat : RF : aggregate[] rdpa_stat(rdpa_stat_t) : Multicast whitelist statistics */
    rdpa_mcast_whitelist_attr_global_stat = 3, /* global_stat : R : aggregate mcast_whitelist_stat(rdpa_mcast_whitelist_stat_t) : Multicast whitelist global statistics */
} rdpa_mcast_whitelist_attr_types;

extern int (*f_rdpa_mcast_whitelist_get)(bdmf_object_handle *pmo);

/** Get mcast_whitelist object.

 * This function returns mcast_whitelist object instance.
 * \param[out] mcast_whitelist_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_mcast_whitelist_get(bdmf_object_handle *mcast_whitelist_obj);

/** Get mcast_whitelist/entry attribute entry.
 *
 * Get Multicast whitelist entry.
 * \param[in]   mo_ mcast_whitelist object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  entry_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_entry_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_mcast_whitelist_t * entry_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_mcast_whitelist_attr_entry, (bdmf_index)ai_, entry_, sizeof(*entry_));
}


/** Add mcast_whitelist/entry attribute entry.
 *
 * Add Multicast whitelist entry.
 * \param[in]   mo_ mcast_whitelist object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   entry_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_entry_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_mcast_whitelist_t * entry_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_mcast_whitelist_attr_entry, (bdmf_index *)ai_, entry_, sizeof(*entry_));
    return rc;
}


/** Delete mcast_whitelist/entry attribute entry.
 *
 * Delete Multicast whitelist entry.
 * \param[in]   mo_ mcast_whitelist object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_entry_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_mcast_whitelist_attr_entry, (bdmf_index)ai_);
}


/** Find mcast_whitelist/entry attribute entry.
 *
 * Find Multicast whitelist entry.
 * \param[in]   mo_ mcast_whitelist object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   entry_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_entry_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_mcast_whitelist_t * entry_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_mcast_whitelist_attr_entry, (bdmf_index *)ai_, entry_, sizeof(*entry_));
    return rc;
}


/** Get mcast_whitelist/port_enable attribute entry.
 *
 * Get Multicast whitelist port enable.
 * \param[in]   mo_ mcast_whitelist object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  port_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_port_enable_get(bdmf_object_handle mo_, rdpa_if ai_, bdmf_boolean *port_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_mcast_whitelist_attr_port_enable, (bdmf_index)ai_, &_nn_);
    *port_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set mcast_whitelist/port_enable attribute entry.
 *
 * Set Multicast whitelist port enable.
 * \param[in]   mo_ mcast_whitelist object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   port_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_port_enable_set(bdmf_object_handle mo_, rdpa_if ai_, bdmf_boolean port_enable_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_mcast_whitelist_attr_port_enable, (bdmf_index)ai_, port_enable_);
}


/** Get mcast_whitelist/stat attribute entry.
 *
 * Get Multicast whitelist statistics.
 * \param[in]   mo_ mcast_whitelist object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_stat_t * stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_mcast_whitelist_attr_stat, (bdmf_index)ai_, stat_, sizeof(*stat_));
}


/** Get mcast_whitelist/global_stat attribute.
 *
 * Get Multicast whitelist global statistics.
 * \param[in]   mo_ mcast_whitelist object handle or mattr transaction handle
 * \param[out]  global_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_mcast_whitelist_global_stat_get(bdmf_object_handle mo_, rdpa_mcast_whitelist_stat_t * global_stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_mcast_whitelist_attr_global_stat, global_stat_, sizeof(*global_stat_));
}

/** @} end of mcast_whitelist Doxygen group */




#endif /* _RDPA_AG_MCAST_WHITELIST_H_ */
