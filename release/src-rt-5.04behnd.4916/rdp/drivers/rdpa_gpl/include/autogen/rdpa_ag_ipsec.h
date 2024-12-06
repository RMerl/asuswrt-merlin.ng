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
 * ipsec object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_IPSEC_H_
#define _RDPA_AG_IPSEC_H_

/** \addtogroup ipsec
 * @{
 */


/** Get ipsec type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an ipsec object.
 * \return ipsec type handle
 */
bdmf_type_handle rdpa_ipsec_drv(void);

/* ipsec: Attribute types */
typedef enum {
    rdpa_ipsec_attr_sa_table_ddr_addr = 0, /* sa_table_ddr_addr : MRI : number : ddr sa table base address */
    rdpa_ipsec_attr_sa_entry_size = 1, /* sa_entry_size : MRI : number : sa table entry size */
    rdpa_ipsec_attr_sa_desc_ds = 2, /* sa_desc_ds : RWF : aggregate[] sa_desc(rdpa_sa_desc_t) : Runner Downstream SA Descriptor */
    rdpa_ipsec_attr_sa_desc_us = 3, /* sa_desc_us : RWF : aggregate[] sa_desc(rdpa_sa_desc_t) : Runner Upstream SA Descriptor */
    rdpa_ipsec_attr_sa_desc_cam_tbl_ds = 4, /* sa_desc_cam_tbl_ds : MRIF : number[] : Runner Downstream SA Descriptor CAM Table */
    rdpa_ipsec_attr_sa_desc_cam_tbl_us = 5, /* sa_desc_cam_tbl_us : MRIF : number[] : Runner Upstream SA Descriptor CAM Table */
} rdpa_ipsec_attr_types;

extern int (*f_rdpa_ipsec_get)(bdmf_object_handle *pmo);

/** Get ipsec object.

 * This function returns ipsec object instance.
 * \param[out] ipsec_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_ipsec_get(bdmf_object_handle *ipsec_obj);

/** Get ipsec/sa_table_ddr_addr attribute.
 *
 * Get ddr sa table base address.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[out]  sa_table_ddr_addr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_table_ddr_addr_get(bdmf_object_handle mo_, bdmf_number *sa_table_ddr_addr_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ipsec_attr_sa_table_ddr_addr, &_nn_);
    *sa_table_ddr_addr_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set ipsec/sa_table_ddr_addr attribute.
 *
 * Set ddr sa table base address.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   sa_table_ddr_addr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_table_ddr_addr_set(bdmf_object_handle mo_, bdmf_number sa_table_ddr_addr_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ipsec_attr_sa_table_ddr_addr, sa_table_ddr_addr_);
}


/** Get ipsec/sa_entry_size attribute.
 *
 * Get sa table entry size.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[out]  sa_entry_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_entry_size_get(bdmf_object_handle mo_, bdmf_number *sa_entry_size_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_ipsec_attr_sa_entry_size, &_nn_);
    *sa_entry_size_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set ipsec/sa_entry_size attribute.
 *
 * Set sa table entry size.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   sa_entry_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_entry_size_set(bdmf_object_handle mo_, bdmf_number sa_entry_size_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_ipsec_attr_sa_entry_size, sa_entry_size_);
}


/** Get ipsec/sa_desc_ds attribute entry.
 *
 * Get Runner Downstream SA Descriptor.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  sa_desc_ds_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_ds_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_sa_desc_t * sa_desc_ds_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ipsec_attr_sa_desc_ds, (bdmf_index)ai_, sa_desc_ds_, sizeof(*sa_desc_ds_));
}


/** Set ipsec/sa_desc_ds attribute entry.
 *
 * Set Runner Downstream SA Descriptor.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   sa_desc_ds_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_ds_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_sa_desc_t * sa_desc_ds_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ipsec_attr_sa_desc_ds, (bdmf_index)ai_, sa_desc_ds_, sizeof(*sa_desc_ds_));
}


/** Get ipsec/sa_desc_us attribute entry.
 *
 * Get Runner Upstream SA Descriptor.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  sa_desc_us_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_us_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_sa_desc_t * sa_desc_us_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_ipsec_attr_sa_desc_us, (bdmf_index)ai_, sa_desc_us_, sizeof(*sa_desc_us_));
}


/** Set ipsec/sa_desc_us attribute entry.
 *
 * Set Runner Upstream SA Descriptor.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   sa_desc_us_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_us_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_sa_desc_t * sa_desc_us_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_ipsec_attr_sa_desc_us, (bdmf_index)ai_, sa_desc_us_, sizeof(*sa_desc_us_));
}


/** Get ipsec/sa_desc_cam_tbl_ds attribute entry.
 *
 * Get Runner Downstream SA Descriptor CAM Table.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  sa_desc_cam_tbl_ds_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_cam_tbl_ds_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *sa_desc_cam_tbl_ds_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_ipsec_attr_sa_desc_cam_tbl_ds, (bdmf_index)ai_, &_nn_);
    *sa_desc_cam_tbl_ds_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set ipsec/sa_desc_cam_tbl_ds attribute entry.
 *
 * Set Runner Downstream SA Descriptor CAM Table.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   sa_desc_cam_tbl_ds_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_cam_tbl_ds_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number sa_desc_cam_tbl_ds_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_ipsec_attr_sa_desc_cam_tbl_ds, (bdmf_index)ai_, sa_desc_cam_tbl_ds_);
}


/** Get ipsec/sa_desc_cam_tbl_us attribute entry.
 *
 * Get Runner Upstream SA Descriptor CAM Table.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  sa_desc_cam_tbl_us_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_cam_tbl_us_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *sa_desc_cam_tbl_us_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_ipsec_attr_sa_desc_cam_tbl_us, (bdmf_index)ai_, &_nn_);
    *sa_desc_cam_tbl_us_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set ipsec/sa_desc_cam_tbl_us attribute entry.
 *
 * Set Runner Upstream SA Descriptor CAM Table.
 * \param[in]   mo_ ipsec object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   sa_desc_cam_tbl_us_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_ipsec_sa_desc_cam_tbl_us_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number sa_desc_cam_tbl_us_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_ipsec_attr_sa_desc_cam_tbl_us, (bdmf_index)ai_, sa_desc_cam_tbl_us_);
}

/** @} end of ipsec Doxygen group */




#endif /* _RDPA_AG_IPSEC_H_ */
