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
 * capwap object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_CAPWAP_H_
#define _RDPA_AG_CAPWAP_H_

/** \addtogroup capwap
 * @{
 */


/** Get capwap type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a capwap object.
 * \return capwap type handle
 */
bdmf_type_handle rdpa_capwap_drv(void);

/* capwap: Attribute types */
typedef enum {
    rdpa_capwap_attr_clear_stats = 0, /* clear_stats : W : bool : CAPWAP clear statistics */
    rdpa_capwap_attr_cfg = 1, /* cfg : RW : aggregate capwap_configuration(rdpa_capwap_cfg_t) : CAPWAP configuration */
} rdpa_capwap_attr_types;

extern int (*f_rdpa_capwap_get)(bdmf_object_handle *pmo);

/** Get capwap object.

 * This function returns capwap object instance.
 * \param[out] capwap_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_capwap_get(bdmf_object_handle *capwap_obj);

/** Set capwap/clear_stats attribute.
 *
 * Set CAPWAP clear statistics.
 * \param[in]   mo_ capwap object handle or mattr transaction handle
 * \param[in]   clear_stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_clear_stats_set(bdmf_object_handle mo_, bdmf_boolean clear_stats_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_capwap_attr_clear_stats, clear_stats_);
}


/** Get capwap/cfg attribute.
 *
 * Get CAPWAP configuration.
 * \param[in]   mo_ capwap object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_cfg_get(bdmf_object_handle mo_, rdpa_capwap_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_capwap_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set capwap/cfg attribute.
 *
 * Set CAPWAP configuration.
 * \param[in]   mo_ capwap object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_cfg_set(bdmf_object_handle mo_, const rdpa_capwap_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_capwap_attr_cfg, cfg_, sizeof(*cfg_));
}

/** @} end of capwap Doxygen group */


/** \addtogroup capwap_reassembly
 * @{
 */


/** Get capwap_reassembly type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a capwap_reassembly object.
 * \return capwap_reassembly type handle
 */
bdmf_type_handle rdpa_capwap_reassembly_drv(void);

/* capwap_reassembly: Attribute types */
typedef enum {
    rdpa_capwap_reassembly_attr_enable = 0, /* enable : RW : bool : CAPWAP enable/disable processing */
    rdpa_capwap_reassembly_attr_cfg = 1, /* cfg : RW : aggregate capwap_reassembly_configuration(rdpa_capwap_reassembly_cfg_t) : CAPWAP reassembly configuration */
    rdpa_capwap_reassembly_attr_stats = 2, /* stats : R : aggregate capwap_reassembly_statistics(rdpa_capwap_reassembly_stats_t) : CAPWAP reassembly statistics */
    rdpa_capwap_reassembly_attr_active_contexts = 3, /* active_contexts : R : aggregate capwap_reassembly_active_contexts(rdpa_capwap_reassembly_contexts_t) : Active CAPWAP context e */
} rdpa_capwap_reassembly_attr_types;

extern int (*f_rdpa_capwap_reassembly_get)(bdmf_object_handle *pmo);

/** Get capwap_reassembly object.

 * This function returns capwap_reassembly object instance.
 * \param[out] capwap_reassembly_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_capwap_reassembly_get(bdmf_object_handle *capwap_reassembly_obj);

/** Get capwap_reassembly/enable attribute.
 *
 * Get CAPWAP enable/disable processing.
 * \param[in]   mo_ capwap_reassembly object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_reassembly_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_capwap_reassembly_attr_enable, &_nn_);
    *enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set capwap_reassembly/enable attribute.
 *
 * Set CAPWAP enable/disable processing.
 * \param[in]   mo_ capwap_reassembly object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_reassembly_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_capwap_reassembly_attr_enable, enable_);
}


/** Get capwap_reassembly/cfg attribute.
 *
 * Get CAPWAP reassembly configuration.
 * \param[in]   mo_ capwap_reassembly object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_reassembly_cfg_get(bdmf_object_handle mo_, rdpa_capwap_reassembly_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_capwap_reassembly_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set capwap_reassembly/cfg attribute.
 *
 * Set CAPWAP reassembly configuration.
 * \param[in]   mo_ capwap_reassembly object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_reassembly_cfg_set(bdmf_object_handle mo_, const rdpa_capwap_reassembly_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_capwap_reassembly_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get capwap_reassembly/stats attribute.
 *
 * Get CAPWAP reassembly statistics.
 * \param[in]   mo_ capwap_reassembly object handle or mattr transaction handle
 * \param[out]  stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_capwap_reassembly_stats_get(bdmf_object_handle mo_, rdpa_capwap_reassembly_stats_t * stats_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_capwap_reassembly_attr_stats, stats_, sizeof(*stats_));
}


/** Get capwap_reassembly/active_contexts attribute.
 *
 * Get Active CAPWAP context entries.
 * \param[in]   mo_ capwap_reassembly object handle or mattr transaction handle
 * \param[out]  active_contexts_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_reassembly_active_contexts_get(bdmf_object_handle mo_, rdpa_capwap_reassembly_contexts_t * active_contexts_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_capwap_reassembly_attr_active_contexts, active_contexts_, sizeof(*active_contexts_));
}

/** @} end of capwap_reassembly Doxygen group */


/** \addtogroup capwap_fragmentation
 * @{
 */


/** Get capwap_fragmentation type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a capwap_fragmentation object.
 * \return capwap_fragmentation type handle
 */
bdmf_type_handle rdpa_capwap_fragmentation_drv(void);

/* capwap_fragmentation: Attribute types */
typedef enum {
    rdpa_capwap_fragmentation_attr_enable = 0, /* enable : RW : bool : CAPWAP enable/disable processing */
    rdpa_capwap_fragmentation_attr_cfg = 1, /* cfg : RW : aggregate capwap_fragmentation_configuration(rdpa_capwap_fragmentation_cfg_t) : CAPWAP fragmentation configuration */
    rdpa_capwap_fragmentation_attr_stats = 2, /* stats : R : aggregate capwap_fragmentation_statistics(rdpa_capwap_fragmentation_stats_t) : CAPWAP fragmentation statistics */
} rdpa_capwap_fragmentation_attr_types;

extern int (*f_rdpa_capwap_fragmentation_get)(bdmf_object_handle *pmo);

/** Get capwap_fragmentation object.

 * This function returns capwap_fragmentation object instance.
 * \param[out] capwap_fragmentation_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_capwap_fragmentation_get(bdmf_object_handle *capwap_fragmentation_obj);

/** Get capwap_fragmentation/enable attribute.
 *
 * Get CAPWAP enable/disable processing.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_capwap_fragmentation_attr_enable, &_nn_);
    *enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set capwap_fragmentation/enable attribute.
 *
 * Set CAPWAP enable/disable processing.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_capwap_fragmentation_attr_enable, enable_);
}


/** Get capwap_fragmentation/cfg attribute.
 *
 * Get CAPWAP fragmentation configuration.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_cfg_get(bdmf_object_handle mo_, rdpa_capwap_fragmentation_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_capwap_fragmentation_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set capwap_fragmentation/cfg attribute.
 *
 * Set CAPWAP fragmentation configuration.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_capwap_fragmentation_cfg_set(bdmf_object_handle mo_, const rdpa_capwap_fragmentation_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_capwap_fragmentation_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get capwap_fragmentation/stats attribute.
 *
 * Get CAPWAP fragmentation statistics.
 * \param[in]   mo_ capwap_fragmentation object handle or mattr transaction handle
 * \param[out]  stats_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_capwap_fragmentation_stats_get(bdmf_object_handle mo_, rdpa_capwap_fragmentation_stats_t * stats_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_capwap_fragmentation_attr_stats, stats_, sizeof(*stats_));
}

/** @} end of capwap_fragmentation Doxygen group */




#endif /* _RDPA_AG_CAPWAP_H_ */
