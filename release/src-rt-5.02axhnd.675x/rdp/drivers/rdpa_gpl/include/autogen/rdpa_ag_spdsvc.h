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
 * spdsvc object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_SPDSVC_H_
#define _RDPA_AG_SPDSVC_H_

/** \addtogroup spdsvc
 * @{
 */


/** Get spdsvc type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a spdsvc object.
 * \return spdsvc type handle
 */
bdmf_type_handle rdpa_spdsvc_drv(void);

/* spdsvc: Attribute types */
typedef enum {
    rdpa_spdsvc_attr_generator = 0, /* generator : RWADF : aggregate[] spdsvc_generator(rdpa_spdsvc_generator_t) : Traffic Generator */
    rdpa_spdsvc_attr_analyzer = 1, /* analyzer : RWADF : aggregate[] spdsvc_analyzer(rdpa_spdsvc_analyzer_t) : Traffic Analyzer */
    rdpa_spdsvc_attr_result = 2, /* result : R : aggregate spdsvc_result(rdpa_spdsvc_result_t) : Test Results */
} rdpa_spdsvc_attr_types;

extern int (*f_rdpa_spdsvc_get)(bdmf_object_handle *pmo);

/** Get spdsvc object.

 * This function returns spdsvc object instance.
 * \param[out] spdsvc_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_spdsvc_get(bdmf_object_handle *spdsvc_obj);

/** Get spdsvc/generator attribute.
 *
 * Get Traffic Generator.
 * \param[in]   mo_ spdsvc object handle or mattr transaction handle
 * \param[out]  generator_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_spdsvc_generator_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_spdsvc_generator_t * generator_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_spdsvc_attr_generator, generator_, sizeof(*generator_));
}


/** Add spdsvc/generator attribute.
 *
 * Add Traffic Generator.
 * \param[in]   mo_ spdsvc object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   generator_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_spdsvc_generator_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_spdsvc_generator_t * generator_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_spdsvc_attr_generator, (bdmf_index *)ai_, generator_, sizeof(*generator_));
    return rc;
}


/** Delete spdsvc/generator attribute entry.
 *
 * Delete Traffic Generator.
 * \param[in]   mo_ spdsvc object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_spdsvc_generator_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_spdsvc_attr_generator, (bdmf_index)ai_);
}


/** Get spdsvc/analyzer attribute.
 *
 * Get Traffic Analyzer.
 * \param[in]   mo_ spdsvc object handle or mattr transaction handle
 * \param[out]  analyzer_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_spdsvc_analyzer_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_spdsvc_analyzer_t * analyzer_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_spdsvc_attr_analyzer, analyzer_, sizeof(*analyzer_));
}


/** Add spdsvc/analyzer attribute.
 *
 * Add Traffic Analyzer.
 * \param[in]   mo_ spdsvc object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   analyzer_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_spdsvc_analyzer_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_spdsvc_analyzer_t * analyzer_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_spdsvc_attr_analyzer, (bdmf_index *)ai_, analyzer_, sizeof(*analyzer_));
    return rc;
}


/** Delete spdsvc/analyzer attribute entry.
 *
 * Delete Traffic Analyzer.
 * \param[in]   mo_ spdsvc object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_spdsvc_analyzer_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_spdsvc_attr_analyzer, (bdmf_index)ai_);
}


/** Get spdsvc/result attribute.
 *
 * Get Test Results.
 * \param[in]   mo_ spdsvc object handle or mattr transaction handle
 * \param[out]  result_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_spdsvc_result_get(bdmf_object_handle mo_, rdpa_spdsvc_result_t * result_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_spdsvc_attr_result, result_, sizeof(*result_));
}

/** @} end of spdsvc Doxygen group */




#endif /* _RDPA_AG_SPDSVC_H_ */
