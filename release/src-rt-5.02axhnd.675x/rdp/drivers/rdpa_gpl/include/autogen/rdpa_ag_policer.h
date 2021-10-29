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
 * policer object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_POLICER_H_
#define _RDPA_AG_POLICER_H_

/** \addtogroup policer
 * @{
 */


/** Get policer type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a policer object.
 * \return policer type handle
 */
bdmf_type_handle rdpa_policer_drv(void);

/* policer: Attribute types */
typedef enum {
    rdpa_policer_attr_dir = 0, /* dir : MKRI : enum : Traffic Direction */
    rdpa_policer_attr_index = 1, /* index : KRI : number : Policer Index */
    rdpa_policer_attr_cfg = 2, /* cfg : RW : aggregate tm_policer_cfg(rdpa_tm_policer_cfg_t) : Configuration */
    rdpa_policer_attr_stat = 3, /* stat : RW : aggregate tm_policer_stat(rdpa_tm_policer_stat_t) : Statistics */
} rdpa_policer_attr_types;

/** policer object key. */
typedef struct {
    rdpa_traffic_dir dir; /**< policer: Traffic Direction */
    bdmf_number index; /**< policer: Policer Index */
} rdpa_policer_key_t;


extern int (*f_rdpa_policer_get)(const rdpa_policer_key_t * key_, bdmf_object_handle *pmo);

/** Get policer object by key.

 * This function returns policer object instance by key.
 * \param[in] key_    Object key
 * \param[out] policer_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_policer_get(const rdpa_policer_key_t * key_, bdmf_object_handle *policer_obj);

/** Get policer/dir attribute.
 *
 * Get Traffic Direction.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[out]  dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_policer_dir_get(bdmf_object_handle mo_, rdpa_traffic_dir *dir_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_policer_attr_dir, &_nn_);
    *dir_ = (rdpa_traffic_dir)_nn_;
    return _rc_;
}


/** Set policer/dir attribute.
 *
 * Set Traffic Direction.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[in]   dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_policer_dir_set(bdmf_object_handle mo_, rdpa_traffic_dir dir_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_policer_attr_dir, dir_);
}


/** Get policer/index attribute.
 *
 * Get Policer Index.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_policer_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_policer_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set policer/index attribute.
 *
 * Set Policer Index.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_policer_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_policer_attr_index, index_);
}


/** Get policer/cfg attribute.
 *
 * Get Configuration.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_policer_cfg_get(bdmf_object_handle mo_, rdpa_tm_policer_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_policer_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set policer/cfg attribute.
 *
 * Set Configuration.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_policer_cfg_set(bdmf_object_handle mo_, const rdpa_tm_policer_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_policer_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get policer/stat attribute.
 *
 * Get Statistics.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_policer_stat_get(bdmf_object_handle mo_, rdpa_tm_policer_stat_t * stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_policer_attr_stat, stat_, sizeof(*stat_));
}


/** Set policer/stat attribute.
 *
 * Set Statistics.
 * \param[in]   mo_ policer object handle or mattr transaction handle
 * \param[in]   stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_policer_stat_set(bdmf_object_handle mo_, const rdpa_tm_policer_stat_t * stat_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_policer_attr_stat, stat_, sizeof(*stat_));
}

/** @} end of policer Doxygen group */




#endif /* _RDPA_AG_POLICER_H_ */
