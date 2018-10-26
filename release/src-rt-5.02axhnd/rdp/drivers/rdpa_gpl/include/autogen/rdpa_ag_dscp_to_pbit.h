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
 * dscp_to_pbit object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_DSCP_TO_PBIT_H_
#define _RDPA_AG_DSCP_TO_PBIT_H_

/** \addtogroup dscp_to_pbit
 * @{
 */


/** Get dscp_to_pbit type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a dscp_to_pbit object.
 * \return dscp_to_pbit type handle
 */
bdmf_type_handle rdpa_dscp_to_pbit_drv(void);

/* dscp_to_pbit: Attribute types */
typedef enum {
    rdpa_dscp_to_pbit_attr_table = 0, /* table : KRI : number : Table index */
    rdpa_dscp_to_pbit_attr_qos_mapping = 1, /* qos_mapping : RI : bool : Yes : qos mapping table, no : vlan action per port table */
    rdpa_dscp_to_pbit_attr_dscp_map = 2, /* dscp_map : RWF : number[] : DSCP PBIT array */
    rdpa_dscp_to_pbit_attr_dscp_pbit_dei_map = 3, /* dscp_pbit_dei_map : RWF : aggregate[] pbit_dei(rdpa_pbit_dei_t) : DSCP PBIT/DEI array */
} rdpa_dscp_to_pbit_attr_types;

extern int (*f_rdpa_dscp_to_pbit_get)(bdmf_number table_, bdmf_object_handle *pmo);

/** Get dscp_to_pbit object by key.

 * This function returns dscp_to_pbit object instance by key.
 * \param[in] table_    Object key
 * \param[out] dscp_to_pbit_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_dscp_to_pbit_get(bdmf_number table_, bdmf_object_handle *dscp_to_pbit_obj);

/** Get dscp_to_pbit/table attribute.
 *
 * Get Table index.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[out]  table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_table_get(bdmf_object_handle mo_, bdmf_number *table_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dscp_to_pbit_attr_table, &_nn_);
    *table_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set dscp_to_pbit/table attribute.
 *
 * Set Table index.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_table_set(bdmf_object_handle mo_, bdmf_number table_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dscp_to_pbit_attr_table, table_);
}


/** Get dscp_to_pbit/qos_mapping attribute.
 *
 * Get Yes : qos mapping table, no : vlan action per port table.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[out]  qos_mapping_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_qos_mapping_get(bdmf_object_handle mo_, bdmf_boolean *qos_mapping_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dscp_to_pbit_attr_qos_mapping, &_nn_);
    *qos_mapping_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set dscp_to_pbit/qos_mapping attribute.
 *
 * Set Yes : qos mapping table, no : vlan action per port table.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   qos_mapping_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dscp_to_pbit_qos_mapping_set(bdmf_object_handle mo_, bdmf_boolean qos_mapping_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dscp_to_pbit_attr_qos_mapping, qos_mapping_);
}


/** Get dscp_to_pbit/dscp_map attribute entry.
 *
 * Get DSCP PBIT array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  dscp_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_map_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_pbit *dscp_map_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_dscp_to_pbit_attr_dscp_map, (bdmf_index)ai_, &_nn_);
    *dscp_map_ = (rdpa_pbit)_nn_;
    return _rc_;
}


/** Set dscp_to_pbit/dscp_map attribute entry.
 *
 * Set DSCP PBIT array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   dscp_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_map_set(bdmf_object_handle mo_, bdmf_index ai_, rdpa_pbit dscp_map_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_dscp_to_pbit_attr_dscp_map, (bdmf_index)ai_, dscp_map_);
}


/** Get dscp_to_pbit/dscp_pbit_dei_map attribute entry.
 *
 * Get DSCP PBIT/DEI array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  dscp_pbit_dei_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_pbit_dei_map_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_pbit_dei_t * dscp_pbit_dei_map_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_dscp_to_pbit_attr_dscp_pbit_dei_map, (bdmf_index)ai_, dscp_pbit_dei_map_, sizeof(*dscp_pbit_dei_map_));
}


/** Set dscp_to_pbit/dscp_pbit_dei_map attribute entry.
 *
 * Set DSCP PBIT/DEI array.
 * \param[in]   mo_ dscp_to_pbit object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   dscp_pbit_dei_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dscp_to_pbit_dscp_pbit_dei_map_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_pbit_dei_t * dscp_pbit_dei_map_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_dscp_to_pbit_attr_dscp_pbit_dei_map, (bdmf_index)ai_, dscp_pbit_dei_map_, sizeof(*dscp_pbit_dei_map_));
}

/** @} end of dscp_to_pbit Doxygen group */




#endif /* _RDPA_AG_DSCP_TO_PBIT_H_ */
