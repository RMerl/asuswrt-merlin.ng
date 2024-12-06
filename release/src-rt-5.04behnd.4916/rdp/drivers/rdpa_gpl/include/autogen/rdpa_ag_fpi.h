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
 * fpi object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_FPI_H_
#define _RDPA_AG_FPI_H_

/** \addtogroup fpi
 * @{
 */


/** Get fpi type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a fpi object.
 * \return fpi type handle
 */
bdmf_type_handle rdpa_fpi_drv(void);

/* fpi: Attribute types */
typedef enum {
    rdpa_fpi_attr_mode = 0, /* mode : RW : enum : Flow Provisioning Interface Mode */
    rdpa_fpi_attr_default_priority = 1, /* default_priority : RW : number : Flow Provisioning Interface Default Priority */
    rdpa_fpi_attr_special_gre = 2, /* special_gre : RW : bool : Flow Provisioning Interface to support a special GRE */
    rdpa_fpi_attr_l2lkp_on_ethertype = 3, /* l2lkp_on_ethertype : RW : bool : Flow Provisioning Interface to force L2 Lookup when configured EtherType is seen */
    rdpa_fpi_attr_ethertype_for_l2lkp = 4, /* ethertype_for_l2lkp : RW : number : Flow Provisioning Interface. EtherType to force L2 Lookup */
    rdpa_fpi_attr_lkp_enable = 5, /* lkp_enable : RW : bool : Hardware Flow Lookup Enable */
} rdpa_fpi_attr_types;

extern int (*f_rdpa_fpi_get)(bdmf_object_handle *pmo);

/** Get fpi object.

 * This function returns fpi object instance.
 * \param[out] fpi_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_fpi_get(bdmf_object_handle *fpi_obj);

/** Get fpi/mode attribute.
 *
 * Get Flow Provisioning Interface Mode.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[out]  mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_mode_get(bdmf_object_handle mo_, rdpa_fpi_mode_t *mode_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_fpi_attr_mode, &_nn_);
    *mode_ = (rdpa_fpi_mode_t)_nn_;
    return _rc_;
}


/** Set fpi/mode attribute.
 *
 * Set Flow Provisioning Interface Mode.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[in]   mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_mode_set(bdmf_object_handle mo_, rdpa_fpi_mode_t mode_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_fpi_attr_mode, mode_);
}


/** Get fpi/default_priority attribute.
 *
 * Get Flow Provisioning Interface Default Priority.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[out]  default_priority_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_default_priority_get(bdmf_object_handle mo_, bdmf_number *default_priority_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_fpi_attr_default_priority, &_nn_);
    *default_priority_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set fpi/default_priority attribute.
 *
 * Set Flow Provisioning Interface Default Priority.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[in]   default_priority_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_default_priority_set(bdmf_object_handle mo_, bdmf_number default_priority_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_fpi_attr_default_priority, default_priority_);
}


/** Get fpi/special_gre attribute.
 *
 * Get Flow Provisioning Interface to support a special GRE.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[out]  special_gre_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_special_gre_get(bdmf_object_handle mo_, bdmf_boolean *special_gre_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_fpi_attr_special_gre, &_nn_);
    *special_gre_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set fpi/special_gre attribute.
 *
 * Set Flow Provisioning Interface to support a special GRE.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[in]   special_gre_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_special_gre_set(bdmf_object_handle mo_, bdmf_boolean special_gre_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_fpi_attr_special_gre, special_gre_);
}


/** Get fpi/l2lkp_on_ethertype attribute.
 *
 * Get Flow Provisioning Interface to force L2 Lookup when configured EtherType is seen.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[out]  l2lkp_on_ethertype_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_l2lkp_on_ethertype_get(bdmf_object_handle mo_, bdmf_boolean *l2lkp_on_ethertype_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_fpi_attr_l2lkp_on_ethertype, &_nn_);
    *l2lkp_on_ethertype_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set fpi/l2lkp_on_ethertype attribute.
 *
 * Set Flow Provisioning Interface to force L2 Lookup when configured EtherType is seen.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[in]   l2lkp_on_ethertype_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_l2lkp_on_ethertype_set(bdmf_object_handle mo_, bdmf_boolean l2lkp_on_ethertype_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_fpi_attr_l2lkp_on_ethertype, l2lkp_on_ethertype_);
}


/** Get fpi/ethertype_for_l2lkp attribute.
 *
 * Get Flow Provisioning Interface. EtherType to force L2 Lookup.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[out]  ethertype_for_l2lkp_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_ethertype_for_l2lkp_get(bdmf_object_handle mo_, bdmf_number *ethertype_for_l2lkp_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_fpi_attr_ethertype_for_l2lkp, &_nn_);
    *ethertype_for_l2lkp_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set fpi/ethertype_for_l2lkp attribute.
 *
 * Set Flow Provisioning Interface. EtherType to force L2 Lookup.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[in]   ethertype_for_l2lkp_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_ethertype_for_l2lkp_set(bdmf_object_handle mo_, bdmf_number ethertype_for_l2lkp_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_fpi_attr_ethertype_for_l2lkp, ethertype_for_l2lkp_);
}


/** Get fpi/lkp_enable attribute.
 *
 * Get Hardware Flow Lookup Enable.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[out]  lkp_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_lkp_enable_get(bdmf_object_handle mo_, bdmf_boolean *lkp_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_fpi_attr_lkp_enable, &_nn_);
    *lkp_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set fpi/lkp_enable attribute.
 *
 * Set Hardware Flow Lookup Enable.
 * \param[in]   mo_ fpi object handle or mattr transaction handle
 * \param[in]   lkp_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_fpi_lkp_enable_set(bdmf_object_handle mo_, bdmf_boolean lkp_enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_fpi_attr_lkp_enable, lkp_enable_);
}

/** @} end of fpi Doxygen group */




#endif /* _RDPA_AG_FPI_H_ */
