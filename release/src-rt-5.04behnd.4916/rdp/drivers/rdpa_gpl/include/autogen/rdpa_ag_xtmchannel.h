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
 * xtmchannel object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_XTMCHANNEL_H_
#define _RDPA_AG_XTMCHANNEL_H_

/** \addtogroup xtmchannel
 * @{
 */


/** Get xtmchannel type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtmchannel object.
 * \return xtmchannel type handle
 */
bdmf_type_handle rdpa_xtmchannel_drv(void);

/* xtmchannel: Attribute types */
typedef enum {
    rdpa_xtmchannel_attr_index = 0, /* index : KRI : number : XTMCHANNEL index */
    rdpa_xtmchannel_attr_egress_tm = 1, /* egress_tm : RW : object : US scheduler object */
    rdpa_xtmchannel_attr_enable = 2, /* enable : RW : bool : Enable XTMCHANNEL */
    rdpa_xtmchannel_attr_orl_prty = 3, /* orl_prty : RW : enum : Priority for overall rate limiter */
} rdpa_xtmchannel_attr_types;

extern int (*f_rdpa_xtmchannel_get)(bdmf_number index_, bdmf_object_handle *pmo);

/** Get xtmchannel object by key.

 * This function returns xtmchannel object instance by key.
 * \param[in] index_    Object key
 * \param[out] xtmchannel_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_xtmchannel_get(bdmf_number index_, bdmf_object_handle *xtmchannel_obj);

/** Get xtmchannel/index attribute.
 *
 * Get XTMCHANNEL index.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmchannel_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmchannel_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set xtmchannel/index attribute.
 *
 * Set XTMCHANNEL index.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_xtmchannel_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmchannel_attr_index, index_);
}


/** Get xtmchannel/egress_tm attribute.
 *
 * Get US scheduler object.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  egress_tm_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_egress_tm_get(bdmf_object_handle mo_, bdmf_object_handle *egress_tm_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmchannel_attr_egress_tm, &_nn_);
    *egress_tm_ = (bdmf_object_handle)(long)_nn_;
    return _rc_;
}


/** Set xtmchannel/egress_tm attribute.
 *
 * Set US scheduler object.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   egress_tm_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_egress_tm_set(bdmf_object_handle mo_, bdmf_object_handle egress_tm_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmchannel_attr_egress_tm, (long)egress_tm_);
}


/** Get xtmchannel/enable attribute.
 *
 * Get Enable XTMCHANNEL.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmchannel_attr_enable, &_nn_);
    *enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set xtmchannel/enable attribute.
 *
 * Set Enable XTMCHANNEL.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmchannel_attr_enable, enable_);
}


/** Get xtmchannel/orl_prty attribute.
 *
 * Get Priority for overall rate limiter.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[out]  orl_prty_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_orl_prty_get(bdmf_object_handle mo_, rdpa_tm_orl_prty *orl_prty_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_xtmchannel_attr_orl_prty, &_nn_);
    *orl_prty_ = (rdpa_tm_orl_prty)_nn_;
    return _rc_;
}


/** Set xtmchannel/orl_prty attribute.
 *
 * Set Priority for overall rate limiter.
 * \param[in]   mo_ xtmchannel object handle or mattr transaction handle
 * \param[in]   orl_prty_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_xtmchannel_orl_prty_set(bdmf_object_handle mo_, rdpa_tm_orl_prty orl_prty_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_xtmchannel_attr_orl_prty, orl_prty_);
}

/** @} end of xtmchannel Doxygen group */




#endif /* _RDPA_AG_XTMCHANNEL_H_ */
