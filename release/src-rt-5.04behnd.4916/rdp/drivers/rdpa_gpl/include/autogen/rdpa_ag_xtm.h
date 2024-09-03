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
 * xtm object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_XTM_H_
#define _RDPA_AG_XTM_H_

/** \addtogroup xtm
 * @{
 */


/** Get xtm type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtm object.
 * \return xtm type handle
 */
bdmf_type_handle rdpa_xtm_drv(void);

/* xtm: Attribute types */
typedef enum {
    rdpa_xtm_attr_index = 0, /* index : R : number : XTM Index */
} rdpa_xtm_attr_types;

extern int (*f_rdpa_xtm_get)(bdmf_object_handle *pmo);

/** Get xtm object.

 * This function returns xtm object instance.
 * \param[out] xtm_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_xtm_get(bdmf_object_handle *xtm_obj);

/** Get xtm/index attribute.
 *
 * Get XTM Index.
 * \param[in]   mo_ xtm object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtm_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtm_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}

/** @} end of xtm Doxygen group */




#endif /* _RDPA_AG_XTM_H_ */
