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
 * tunnel object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_TUNNEL_H_
#define _RDPA_AG_TUNNEL_H_

/** \addtogroup tunnel
 * @{
 */


/** Get tunnel type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a tunnel object.
 * \return tunnel type handle
 */
bdmf_type_handle rdpa_tunnel_drv(void);

/* tunnel: Attribute types */
typedef enum {
    rdpa_tunnel_attr_index = 0, /* index : KRI : number : Tunnel Index */
    rdpa_tunnel_attr_cfg = 1, /* cfg : RW : aggregate tunnel_cfg(rdpa_tunnel_cfg_t) : Tunnel Configuration */
    rdpa_tunnel_attr_ref_cnt = 2, /* ref_cnt : R : number : Reference count */
} rdpa_tunnel_attr_types;

extern int (*f_rdpa_tunnel_get)(bdmf_number index_, bdmf_object_handle *pmo);

/** Get tunnel object by key.

 * This function returns tunnel object instance by key.
 * \param[in] index_    Object key
 * \param[out] tunnel_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_tunnel_get(bdmf_number index_, bdmf_object_handle *tunnel_obj);

/** Get tunnel/index attribute.
 *
 * Get Tunnel Index.
 * \param[in]   mo_ tunnel object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tunnel_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_tunnel_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set tunnel/index attribute.
 *
 * Set Tunnel Index.
 * \param[in]   mo_ tunnel object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tunnel_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tunnel_attr_index, index_);
}


/** Get tunnel/cfg attribute.
 *
 * Get Tunnel Configuration.
 * \param[in]   mo_ tunnel object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tunnel_cfg_get(bdmf_object_handle mo_, rdpa_tunnel_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tunnel_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set tunnel/cfg attribute.
 *
 * Set Tunnel Configuration.
 * \param[in]   mo_ tunnel object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tunnel_cfg_set(bdmf_object_handle mo_, const rdpa_tunnel_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tunnel_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get tunnel/ref_cnt attribute.
 *
 * Get Reference count.
 * \param[in]   mo_ tunnel object handle or mattr transaction handle
 * \param[out]  ref_cnt_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tunnel_ref_cnt_get(bdmf_object_handle mo_, bdmf_number *ref_cnt_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_tunnel_attr_ref_cnt, &_nn_);
    *ref_cnt_ = (bdmf_number)_nn_;
    return _rc_;
}

/** @} end of tunnel Doxygen group */




#endif /* _RDPA_AG_TUNNEL_H_ */
