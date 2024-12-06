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
 * xtmflow object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_XTMFLOW_H_
#define _RDPA_AG_XTMFLOW_H_

/** \addtogroup xtmflow
 * @{
 */


/** Get xtmflow type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtmflow object.
 * \return xtmflow type handle
 */
bdmf_type_handle rdpa_xtmflow_drv(void);

/* xtmflow: Attribute types */
typedef enum {
    rdpa_xtmflow_attr_index = 0, /* index : KRI : number : xtmflow index */
    rdpa_xtmflow_attr_hdr_type = 1, /* hdr_type : MRI : number : xtmflow hdr_type */
    rdpa_xtmflow_attr_fstat = 2, /* fstat : MRI : number : xtmflow fstat */
    rdpa_xtmflow_attr_us_cfg = 3, /* us_cfg : RW : aggregate xtmflow_us_cfg(rdpa_xtmflow_us_cfg_t) : US configuration */
    rdpa_xtmflow_attr_stat = 4, /* stat : RW : aggregate xtmflow_stat(rdpa_xtmflow_stat_t) : xtmflow statistics */
    rdpa_xtmflow_attr_ptmBonding = 5, /* ptmBonding : MRI : number : xtmflow ptmBonding */
} rdpa_xtmflow_attr_types;

extern int (*f_rdpa_xtmflow_get)(bdmf_number index_, bdmf_object_handle *pmo);

/** Get xtmflow object by key.

 * This function returns xtmflow object instance by key.
 * \param[in] index_    Object key
 * \param[out] xtmflow_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_xtmflow_get(bdmf_number index_, bdmf_object_handle *xtmflow_obj);

/** Get xtmflow/index attribute.
 *
 * Get xtmflow index.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmflow_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set xtmflow/index attribute.
 *
 * Set xtmflow index.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmflow_attr_index, index_);
}


/** Get xtmflow/hdr_type attribute.
 *
 * Get xtmflow hdr_type.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  hdr_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_hdr_type_get(bdmf_object_handle mo_, bdmf_number *hdr_type_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmflow_attr_hdr_type, &_nn_);
    *hdr_type_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set xtmflow/hdr_type attribute.
 *
 * Set xtmflow hdr_type.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   hdr_type_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_hdr_type_set(bdmf_object_handle mo_, bdmf_number hdr_type_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmflow_attr_hdr_type, hdr_type_);
}


/** Get xtmflow/fstat attribute.
 *
 * Get xtmflow fstat.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  fstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_fstat_get(bdmf_object_handle mo_, bdmf_number *fstat_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmflow_attr_fstat, &_nn_);
    *fstat_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set xtmflow/fstat attribute.
 *
 * Set xtmflow fstat.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   fstat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_fstat_set(bdmf_object_handle mo_, bdmf_number fstat_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmflow_attr_fstat, fstat_);
}


/** Get xtmflow/us_cfg attribute.
 *
 * Get US configuration.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  us_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_us_cfg_get(bdmf_object_handle mo_, rdpa_xtmflow_us_cfg_t * us_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_xtmflow_attr_us_cfg, us_cfg_, sizeof(*us_cfg_));
}


/** Set xtmflow/us_cfg attribute.
 *
 * Set US configuration.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   us_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_us_cfg_set(bdmf_object_handle mo_, const rdpa_xtmflow_us_cfg_t * us_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_xtmflow_attr_us_cfg, us_cfg_, sizeof(*us_cfg_));
}


/** Get xtmflow/stat attribute.
 *
 * Get xtmflow statistics.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_stat_get(bdmf_object_handle mo_, rdpa_xtmflow_stat_t * stat_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_xtmflow_attr_stat, stat_, sizeof(*stat_));
}


/** Set xtmflow/stat attribute.
 *
 * Set xtmflow statistics.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmflow_stat_set(bdmf_object_handle mo_, const rdpa_xtmflow_stat_t * stat_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_xtmflow_attr_stat, stat_, sizeof(*stat_));
}


/** Get xtmflow/ptmBonding attribute.
 *
 * Get xtmflow ptmBonding.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[out]  ptmBonding_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_ptmBonding_get(bdmf_object_handle mo_, bdmf_number *ptmBonding_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmflow_attr_ptmBonding, &_nn_);
    *ptmBonding_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set xtmflow/ptmBonding attribute.
 *
 * Set xtmflow ptmBonding.
 * \param[in]   mo_ xtmflow object handle or mattr transaction handle
 * \param[in]   ptmBonding_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmflow_ptmBonding_set(bdmf_object_handle mo_, bdmf_number ptmBonding_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmflow_attr_ptmBonding, ptmBonding_);
}

/** @} end of xtmflow Doxygen group */




#endif /* _RDPA_AG_XTMFLOW_H_ */
