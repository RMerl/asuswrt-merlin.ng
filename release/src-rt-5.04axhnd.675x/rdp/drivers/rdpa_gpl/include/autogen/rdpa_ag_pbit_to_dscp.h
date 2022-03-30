// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2, as published by
// the Free Software Foundation (the "GPL").
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// 
// A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
// writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
// 
// :>
/*
 * pbit_to_dscp object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_PBIT_TO_DSCP_H_
#define _RDPA_AG_PBIT_TO_DSCP_H_

/** \addtogroup pbit_to_dscp
 * @{
 */


/** Get pbit_to_dscp type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a pbit_to_dscp object.
 * \return pbit_to_dscp type handle
 */
bdmf_type_handle rdpa_pbit_to_dscp_drv(void);

/* pbit_to_dscp: Attribute types */
typedef enum {
    rdpa_pbit_to_dscp_attr_pbit_map = 0, /* pbit_map : RWF : number[] : PBIT DSCP array */
    rdpa_pbit_to_dscp_attr_pbit_map_non_ip = 1, /* pbit_map_non_ip : RW : number : none IP mapping */
} rdpa_pbit_to_dscp_attr_types;

extern int (*f_rdpa_pbit_to_dscp_get)(bdmf_object_handle *pmo);

/** Get pbit_to_dscp object.

 * This function returns pbit_to_dscp object instance.
 * \param[out] pbit_to_dscp_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_pbit_to_dscp_get(bdmf_object_handle *pbit_to_dscp_obj);

/** Get pbit_to_dscp/pbit_map attribute entry.
 *
 * Get PBIT DSCP array.
 * \param[in]   mo_ pbit_to_dscp object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pbit_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_pbit_to_dscp_pbit_map_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_dscp *pbit_map_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_pbit_to_dscp_attr_pbit_map, (bdmf_index)ai_, &_nn_);
    *pbit_map_ = (rdpa_dscp)_nn_;
    return _rc_;
}


/** Set pbit_to_dscp/pbit_map attribute entry.
 *
 * Set PBIT DSCP array.
 * \param[in]   mo_ pbit_to_dscp object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   pbit_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_pbit_to_dscp_pbit_map_set(bdmf_object_handle mo_, bdmf_index ai_, rdpa_dscp pbit_map_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_pbit_to_dscp_attr_pbit_map, (bdmf_index)ai_, pbit_map_);
}


/** Get pbit_to_dscp/pbit_map_non_ip attribute.
 *
 * Get none IP mapping.
 * \param[in]   mo_ pbit_to_dscp object handle or mattr transaction handle
 * \param[out]  pbit_map_non_ip_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_pbit_to_dscp_pbit_map_non_ip_get(bdmf_object_handle mo_, bdmf_number *pbit_map_non_ip_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_pbit_to_dscp_attr_pbit_map_non_ip, &_nn_);
    *pbit_map_non_ip_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set pbit_to_dscp/pbit_map_non_ip attribute.
 *
 * Set none IP mapping.
 * \param[in]   mo_ pbit_to_dscp object handle or mattr transaction handle
 * \param[in]   pbit_map_non_ip_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_pbit_to_dscp_pbit_map_non_ip_set(bdmf_object_handle mo_, bdmf_number pbit_map_non_ip_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_pbit_to_dscp_attr_pbit_map_non_ip, pbit_map_non_ip_);
}

/** @} end of pbit_to_dscp Doxygen group */




#endif /* _RDPA_AG_PBIT_TO_DSCP_H_ */
