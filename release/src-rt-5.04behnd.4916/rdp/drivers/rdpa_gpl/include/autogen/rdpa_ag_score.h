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
 * score object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_SCORE_H_
#define _RDPA_AG_SCORE_H_

/** \addtogroup score
 * @{
 */


/** Get score type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a score object.
 * \return score type handle
 */
bdmf_type_handle rdpa_score_drv(void);

/* score: Attribute types */
typedef enum {
    rdpa_score_attr_nstreams = 0, /* nstreams : R : number : number of enabled streams */
    rdpa_score_attr_stream = 1, /* stream : RWADF : aggregate[] score_stream_info(rdpa_score_stream_info_t) : SCORE stream entry */
    rdpa_score_attr_nprofiles = 2, /* nprofiles : R : number : number of configured profiles */
    rdpa_score_attr_profile = 3, /* profile : RWADF : aggregate[] score_profile_cfg(rdpa_score_profile_cfg_t) : SCORE profile entry */
    rdpa_score_attr_nsdrs = 4, /* nsdrs : R : number : number of configured SDRs */
    rdpa_score_attr_sdr = 5, /* sdr : RWADF : aggregate[] score_sdr_info(rdpa_score_sdr_info_t) : Score Data Ring entry */
    rdpa_score_attr_score_msg_cb = 6, /* score_msg_cb : RI : pointer : callback function for score message */
    rdpa_score_attr_score_irq_coalescing_num = 7, /* score_irq_coalescing_num : RW : number : number of reports to gather per interrupt */
    rdpa_score_attr_score_irq_coalescing_timeout = 8, /* score_irq_coalescing_timeout : RI : number : timeout for pending reports in ms */
    rdpa_score_attr_score_enable = 9, /* score_enable : RW : bool : global enable SCORE */
    rdpa_score_attr_qstat_cfg = 10, /* qstat_cfg : W : number : Qstat configure */
} rdpa_score_attr_types;

extern int (*f_rdpa_score_get)(bdmf_object_handle *pmo);

/** Get score object.

 * This function returns score object instance.
 * \param[out] score_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_score_get(bdmf_object_handle *score_obj);

/** Get score/nstreams attribute.
 *
 * Get number of enabled streams.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  nstreams_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_nstreams_get(bdmf_object_handle mo_, bdmf_number *nstreams_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_score_attr_nstreams, &_nn_);
    *nstreams_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get score/stream attribute entry.
 *
 * Get SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_score_stream_info_t * stream_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_score_attr_stream, (bdmf_index)ai_, stream_, sizeof(*stream_));
}


/** Set score/stream attribute entry.
 *
 * Set SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_score_stream_info_t * stream_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_score_attr_stream, (bdmf_index)ai_, stream_, sizeof(*stream_));
}


/** Add score/stream attribute entry.
 *
 * Add SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_score_stream_info_t * stream_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_score_attr_stream, (bdmf_index *)ai_, stream_, sizeof(*stream_));
    return rc;
}


/** Delete score/stream attribute entry.
 *
 * Delete SCORE stream entry.
 * \param[in]   mo_ score object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_score_attr_stream, (bdmf_index)ai_);
}


/** Get next score/stream attribute entry.
 *
 * Get next SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_score_attr_stream, (bdmf_index *)ai_);
}


/** Find score/stream attribute entry.
 *
 * Find SCORE stream entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   stream_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_stream_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_score_stream_info_t * stream_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_score_attr_stream, (bdmf_index *)ai_, stream_, sizeof(*stream_));
    return rc;
}


/** Get score/nprofiles attribute.
 *
 * Get number of configured profiles.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  nprofiles_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_nprofiles_get(bdmf_object_handle mo_, bdmf_number *nprofiles_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_score_attr_nprofiles, &_nn_);
    *nprofiles_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get score/profile attribute entry.
 *
 * Get SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_score_profile_cfg_t * profile_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_score_attr_profile, (bdmf_index)ai_, profile_, sizeof(*profile_));
}


/** Set score/profile attribute entry.
 *
 * Set SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_score_profile_cfg_t * profile_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_score_attr_profile, (bdmf_index)ai_, profile_, sizeof(*profile_));
}


/** Add score/profile attribute entry.
 *
 * Add SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_score_profile_cfg_t * profile_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_score_attr_profile, (bdmf_index *)ai_, profile_, sizeof(*profile_));
    return rc;
}


/** Delete score/profile attribute entry.
 *
 * Delete SCORE profile entry.
 * \param[in]   mo_ score object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_score_attr_profile, (bdmf_index)ai_);
}


/** Get next score/profile attribute entry.
 *
 * Get next SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_score_attr_profile, (bdmf_index *)ai_);
}


/** Find score/profile attribute entry.
 *
 * Find SCORE profile entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   profile_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_profile_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_score_profile_cfg_t * profile_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_score_attr_profile, (bdmf_index *)ai_, profile_, sizeof(*profile_));
    return rc;
}


/** Get score/nsdrs attribute.
 *
 * Get number of configured SDRs.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  nsdrs_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_nsdrs_get(bdmf_object_handle mo_, bdmf_number *nsdrs_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_score_attr_nsdrs, &_nn_);
    *nsdrs_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get score/sdr attribute entry.
 *
 * Get Score Data Ring entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  sdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_sdr_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_score_sdr_info_t * sdr_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_score_attr_sdr, (bdmf_index)ai_, sdr_, sizeof(*sdr_));
}


/** Set score/sdr attribute entry.
 *
 * Set Score Data Ring entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   sdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_sdr_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_score_sdr_info_t * sdr_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_score_attr_sdr, (bdmf_index)ai_, sdr_, sizeof(*sdr_));
}


/** Add score/sdr attribute entry.
 *
 * Add Score Data Ring entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in]   sdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_sdr_add(bdmf_object_handle mo_, bdmf_index * ai_, const rdpa_score_sdr_info_t * sdr_)
{
    int rc;
    rc = bdmf_attrelem_add_as_buf(mo_, rdpa_score_attr_sdr, (bdmf_index *)ai_, sdr_, sizeof(*sdr_));
    return rc;
}


/** Delete score/sdr attribute entry.
 *
 * Delete Score Data Ring entry.
 * \param[in]   mo_ score object handle
 * \param[in]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_sdr_delete(bdmf_object_handle mo_, bdmf_index ai_)
{
    return bdmf_attrelem_delete(mo_, rdpa_score_attr_sdr, (bdmf_index)ai_);
}


/** Get next score/sdr attribute entry.
 *
 * Get next Score Data Ring entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_sdr_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_score_attr_sdr, (bdmf_index *)ai_);
}


/** Find score/sdr attribute entry.
 *
 * Find Score Data Ring entry.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \param[in,out]   sdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_sdr_find(bdmf_object_handle mo_, bdmf_index * ai_, rdpa_score_sdr_info_t * sdr_)
{
    int rc;
    rc = bdmf_attrelem_find(mo_, rdpa_score_attr_sdr, (bdmf_index *)ai_, sdr_, sizeof(*sdr_));
    return rc;
}


/** Get score/score_msg_cb attribute.
 *
 * Get callback function for score message.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  score_msg_cb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_msg_cb_get(bdmf_object_handle mo_, void * *score_msg_cb_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_score_attr_score_msg_cb, &_nn_);
    *score_msg_cb_ = (void *)(bdmf_long_t)_nn_;
    return _rc_;
}


/** Set score/score_msg_cb attribute.
 *
 * Set callback function for score message.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   score_msg_cb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_msg_cb_set(bdmf_object_handle mo_, void * score_msg_cb_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_score_attr_score_msg_cb, (bdmf_long_t)score_msg_cb_);
}


/** Get score/score_irq_coalescing_num attribute.
 *
 * Get number of reports to gather per interrupt.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  score_irq_coalescing_num_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_score_irq_coalescing_num_get(bdmf_object_handle mo_, bdmf_number *score_irq_coalescing_num_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_score_attr_score_irq_coalescing_num, &_nn_);
    *score_irq_coalescing_num_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set score/score_irq_coalescing_num attribute.
 *
 * Set number of reports to gather per interrupt.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   score_irq_coalescing_num_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_score_score_irq_coalescing_num_set(bdmf_object_handle mo_, bdmf_number score_irq_coalescing_num_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_score_attr_score_irq_coalescing_num, score_irq_coalescing_num_);
}


/** Get score/score_irq_coalescing_timeout attribute.
 *
 * Get timeout for pending reports in ms.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  score_irq_coalescing_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_irq_coalescing_timeout_get(bdmf_object_handle mo_, bdmf_number *score_irq_coalescing_timeout_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_score_attr_score_irq_coalescing_timeout, &_nn_);
    *score_irq_coalescing_timeout_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set score/score_irq_coalescing_timeout attribute.
 *
 * Set timeout for pending reports in ms.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   score_irq_coalescing_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_irq_coalescing_timeout_set(bdmf_object_handle mo_, bdmf_number score_irq_coalescing_timeout_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_score_attr_score_irq_coalescing_timeout, score_irq_coalescing_timeout_);
}


/** Get score/score_enable attribute.
 *
 * Get global enable SCORE.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[out]  score_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_enable_get(bdmf_object_handle mo_, bdmf_boolean *score_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_score_attr_score_enable, &_nn_);
    *score_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set score/score_enable attribute.
 *
 * Set global enable SCORE.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   score_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_score_enable_set(bdmf_object_handle mo_, bdmf_boolean score_enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_score_attr_score_enable, score_enable_);
}


/** Set score/qstat_cfg attribute.
 *
 * Set Qstat configure.
 * \param[in]   mo_ score object handle or mattr transaction handle
 * \param[in]   qstat_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_score_qstat_cfg_set(bdmf_object_handle mo_, bdmf_number qstat_cfg_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_score_attr_qstat_cfg, qstat_cfg_);
}

/** @} end of score Doxygen group */




#endif /* _RDPA_AG_SCORE_H_ */
