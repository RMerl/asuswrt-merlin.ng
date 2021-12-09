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
 * filter object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_FILTER_H_
#define _RDPA_AG_FILTER_H_

/** \addtogroup filter
 * @{
 */


/** Get filter type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a filter object.
 * \return filter type handle
 */
bdmf_type_handle rdpa_filter_drv(void);

/* filter: Attribute types */
typedef enum {
    rdpa_filter_attr_global_cfg = 0, /* global_cfg : RW : aggregate filter_global_cfg(rdpa_filter_global_cfg_t) : Global configuration */
    rdpa_filter_attr_etype_udef = 1, /* etype_udef : RWF : number[] : Ether-Type, User-Defined filter */
    rdpa_filter_attr_oui_val = 2, /* oui_val : RWF : number[(filter_oui_val_key)] : MAC Address OUI filter, Value */
    rdpa_filter_attr_tpid_vals = 3, /* tpid_vals : RW : aggregate filter_tpid_vals(rdpa_filter_tpid_vals_t) : TPID filter, Values */
    rdpa_filter_attr_stats = 4, /* stats : RWF : number[(filter_stats_key)] : Drop statistics */
} rdpa_filter_attr_types;

extern int (*f_rdpa_filter_get)(bdmf_object_handle *pmo);

/** Get filter object.

 * This function returns filter object instance.
 * \param[out] filter_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_filter_get(bdmf_object_handle *filter_obj);

/** Get filter/global_cfg attribute.
 *
 * Get Global configuration.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[out]  global_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_global_cfg_get(bdmf_object_handle mo_, rdpa_filter_global_cfg_t * global_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_filter_attr_global_cfg, global_cfg_, sizeof(*global_cfg_));
}


/** Set filter/global_cfg attribute.
 *
 * Set Global configuration.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   global_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_global_cfg_set(bdmf_object_handle mo_, const rdpa_filter_global_cfg_t * global_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_filter_attr_global_cfg, global_cfg_, sizeof(*global_cfg_));
}


/** Get filter/etype_udef attribute entry.
 *
 * Get Ether-Type, User-Defined filter.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  etype_udef_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_etype_udef_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *etype_udef_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_filter_attr_etype_udef, (bdmf_index)ai_, &_nn_);
    *etype_udef_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set filter/etype_udef attribute entry.
 *
 * Set Ether-Type, User-Defined filter.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   etype_udef_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_etype_udef_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number etype_udef_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_filter_attr_etype_udef, (bdmf_index)ai_, etype_udef_);
}


/** Get next filter/etype_udef attribute entry.
 *
 * Get next Ether-Type, User-Defined filter.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_etype_udef_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_filter_attr_etype_udef, (bdmf_index *)ai_);
}


/** Get filter/oui_val attribute entry.
 *
 * Get MAC Address OUI filter, Value.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  oui_val_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_oui_val_get(bdmf_object_handle mo_, rdpa_filter_oui_val_key_t * ai_, bdmf_number *oui_val_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_filter_attr_oui_val, (bdmf_index)ai_, &_nn_);
    *oui_val_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set filter/oui_val attribute entry.
 *
 * Set MAC Address OUI filter, Value.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   oui_val_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_oui_val_set(bdmf_object_handle mo_, rdpa_filter_oui_val_key_t * ai_, bdmf_number oui_val_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_filter_attr_oui_val, (bdmf_index)ai_, oui_val_);
}


/** Get next filter/oui_val attribute entry.
 *
 * Get next MAC Address OUI filter, Value.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_oui_val_get_next(bdmf_object_handle mo_, rdpa_filter_oui_val_key_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_filter_attr_oui_val, (bdmf_index *)ai_);
}


/** Get filter/tpid_vals attribute.
 *
 * Get TPID filter, Values.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[out]  tpid_vals_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_tpid_vals_get(bdmf_object_handle mo_, rdpa_filter_tpid_vals_t * tpid_vals_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_filter_attr_tpid_vals, tpid_vals_, sizeof(*tpid_vals_));
}


/** Set filter/tpid_vals attribute.
 *
 * Set TPID filter, Values.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   tpid_vals_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_tpid_vals_set(bdmf_object_handle mo_, const rdpa_filter_tpid_vals_t * tpid_vals_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_filter_attr_tpid_vals, tpid_vals_, sizeof(*tpid_vals_));
}


/** Get filter/stats attribute entry.
 *
 * Get Drop statistics.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_stats_get(bdmf_object_handle mo_, rdpa_filter_stats_key_t * ai_, bdmf_number *stats_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_filter_attr_stats, (bdmf_index)ai_, &_nn_);
    *stats_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set filter/stats attribute entry.
 *
 * Set Drop statistics.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_stats_set(bdmf_object_handle mo_, rdpa_filter_stats_key_t * ai_, bdmf_number stats_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_filter_attr_stats, (bdmf_index)ai_, stats_);
}


/** Get next filter/stats attribute entry.
 *
 * Get next Drop statistics.
 * \param[in]   mo_ filter object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_filter_stats_get_next(bdmf_object_handle mo_, rdpa_filter_stats_key_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_filter_attr_stats, (bdmf_index *)ai_);
}

/** @} end of filter Doxygen group */




#endif /* _RDPA_AG_FILTER_H_ */
