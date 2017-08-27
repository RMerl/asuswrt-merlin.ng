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
 * egress_tm object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_EGRESS_TM_H_
#define _RDPA_AG_EGRESS_TM_H_

/** \addtogroup egress_tm
 * @{
 */


/** Get egress_tm type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an egress_tm object.
 * \return egress_tm type handle
 */
bdmf_type_handle rdpa_egress_tm_drv(void);

/* egress_tm: Attribute types */
typedef enum {
    rdpa_egress_tm_attr_dir = 0, /* dir : MKRI : enum/4 : Traffic Direction */
    rdpa_egress_tm_attr_index = 1, /* index : KRI : number/4 : Egress-TM Index */
    rdpa_egress_tm_attr_level = 2, /* level : RI : enum/4 : Egress-TM Next Level */
    rdpa_egress_tm_attr_mode = 3, /* mode : RI : enum/4 : Scheduler Operating Mode */
    rdpa_egress_tm_attr_overall_rl = 4, /* overall_rl : RI : bool/1 : Overall Rate Limiter */
    rdpa_egress_tm_attr_service_queue = 5, /* service_queue : RI : aggregate/1 service_queue_cfg(rdpa_tm_service_queue_t) : Service Queue Parameters Configuration */
    rdpa_egress_tm_attr_enable = 6, /* enable : RW : bool/1 : Enable object */
    rdpa_egress_tm_attr_rl = 7, /* rl : RW : aggregate/12 tm_rl_cfg(rdpa_tm_rl_cfg_t) : Rate Configuration */
    rdpa_egress_tm_attr_rl_rate_mode = 8, /* rl_rate_mode : RI : enum/4 : Subsidiary Rate Limiter Rate Mode */
    rdpa_egress_tm_attr_num_queues = 9, /* num_queues : RI : number/1 : Number of Queues */
    rdpa_egress_tm_attr_num_sp_elements = 10, /* num_sp_elements : RW : enum/4 : Number of SP Scheduling Elements for SP_WRR Scheduling Mode */
    rdpa_egress_tm_attr_queue_cfg = 11, /* queue_cfg : RWF : aggregate/45[8] tm_queue_cfg(rdpa_tm_queue_cfg_t) : Queue Parameters Configuration */
    rdpa_egress_tm_attr_queue_flush = 12, /* queue_flush : W : bool/1[8(tm_queue_index)] : Flush Egress Queue */
    rdpa_egress_tm_attr_queue_statistics = 13, /* queue_statistics : RF : aggregate/16[8(tm_queue_index)] rdpa_stat_1way(rdpa_stat_1way_t) : Dropped Service Queue Statistics */
    rdpa_egress_tm_attr_queue_stat = 14, /* queue_stat : RWF : aggregate/16[8(tm_queue_index)] rdpa_stat_1way(rdpa_stat_1way_t) : Retrieve Egress Queue Statistics */
    rdpa_egress_tm_attr_subsidiary = 15, /* subsidiary : RWF : object/4[32] : Next Level Egress-TM */
    rdpa_egress_tm_attr_weight = 16, /* weight : RW : number/4 : Weight for WRR scheduling (0 for unset) */
} rdpa_egress_tm_attr_types;

/** egress_tm object key. */
typedef struct {
    rdpa_traffic_dir dir; /**< egress_tm: Traffic Direction */
    bdmf_number index; /**< egress_tm: Egress-TM Index */
} rdpa_egress_tm_key_t;


extern int (*f_rdpa_egress_tm_get)(const rdpa_egress_tm_key_t * key_, bdmf_object_handle *pmo);

/** Get egress_tm object by key.

 * This function returns egress_tm object instance by key.
 * \param[in] key_    Object key
 * \param[out] egress_tm_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_egress_tm_get(const rdpa_egress_tm_key_t * key_, bdmf_object_handle *egress_tm_obj);

/** Get egress_tm/dir attribute.
 *
 * Get Traffic Direction.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_dir_get(bdmf_object_handle mo_, rdpa_traffic_dir *dir_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_dir, &_nn_);
    *dir_ = (rdpa_traffic_dir)_nn_;
    return _rc_;
}


/** Set egress_tm/dir attribute.
 *
 * Set Traffic Direction.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_dir_set(bdmf_object_handle mo_, rdpa_traffic_dir dir_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_dir, dir_);
}


/** Get egress_tm/index attribute.
 *
 * Get Egress-TM Index.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_index_get(bdmf_object_handle mo_, bdmf_number *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_index, &_nn_);
    *index_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set egress_tm/index attribute.
 *
 * Set Egress-TM Index.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_index_set(bdmf_object_handle mo_, bdmf_number index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_index, index_);
}


/** Get egress_tm/level attribute.
 *
 * Get Egress-TM Next Level.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  level_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_level_get(bdmf_object_handle mo_, rdpa_tm_level_type *level_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_level, &_nn_);
    *level_ = (rdpa_tm_level_type)_nn_;
    return _rc_;
}


/** Set egress_tm/level attribute.
 *
 * Set Egress-TM Next Level.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   level_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_level_set(bdmf_object_handle mo_, rdpa_tm_level_type level_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_level, level_);
}


/** Get egress_tm/mode attribute.
 *
 * Get Scheduler Operating Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_mode_get(bdmf_object_handle mo_, rdpa_tm_sched_mode *mode_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_mode, &_nn_);
    *mode_ = (rdpa_tm_sched_mode)_nn_;
    return _rc_;
}


/** Set egress_tm/mode attribute.
 *
 * Set Scheduler Operating Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_mode_set(bdmf_object_handle mo_, rdpa_tm_sched_mode mode_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_mode, mode_);
}


/** Get egress_tm/overall_rl attribute.
 *
 * Get Overall Rate Limiter.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  overall_rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_overall_rl_get(bdmf_object_handle mo_, bdmf_boolean *overall_rl_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_overall_rl, &_nn_);
    *overall_rl_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set egress_tm/overall_rl attribute.
 *
 * Set Overall Rate Limiter.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   overall_rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_overall_rl_set(bdmf_object_handle mo_, bdmf_boolean overall_rl_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_overall_rl, overall_rl_);
}


/** Get egress_tm/service_queue attribute.
 *
 * Get Service Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  service_queue_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_service_queue_get(bdmf_object_handle mo_, rdpa_tm_service_queue_t * service_queue_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_egress_tm_attr_service_queue, service_queue_, sizeof(*service_queue_));
}


/** Set egress_tm/service_queue attribute.
 *
 * Set Service Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   service_queue_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_service_queue_set(bdmf_object_handle mo_, const rdpa_tm_service_queue_t * service_queue_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_egress_tm_attr_service_queue, service_queue_, sizeof(*service_queue_));
}


/** Get egress_tm/enable attribute.
 *
 * Get Enable object.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_enable_get(bdmf_object_handle mo_, bdmf_boolean *enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_enable, &_nn_);
    *enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set egress_tm/enable attribute.
 *
 * Set Enable object.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_enable_set(bdmf_object_handle mo_, bdmf_boolean enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_enable, enable_);
}


/** Get egress_tm/rl attribute.
 *
 * Get Rate Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_rl_get(bdmf_object_handle mo_, rdpa_tm_rl_cfg_t * rl_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_egress_tm_attr_rl, rl_, sizeof(*rl_));
}


/** Set egress_tm/rl attribute.
 *
 * Set Rate Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   rl_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_rl_set(bdmf_object_handle mo_, const rdpa_tm_rl_cfg_t * rl_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_egress_tm_attr_rl, rl_, sizeof(*rl_));
}


/** Get egress_tm/rl_rate_mode attribute.
 *
 * Get Subsidiary Rate Limiter Rate Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  rl_rate_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_rl_rate_mode_get(bdmf_object_handle mo_, rdpa_tm_rl_rate_mode *rl_rate_mode_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_rl_rate_mode, &_nn_);
    *rl_rate_mode_ = (rdpa_tm_rl_rate_mode)_nn_;
    return _rc_;
}


/** Set egress_tm/rl_rate_mode attribute.
 *
 * Set Subsidiary Rate Limiter Rate Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   rl_rate_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_rl_rate_mode_set(bdmf_object_handle mo_, rdpa_tm_rl_rate_mode rl_rate_mode_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_rl_rate_mode, rl_rate_mode_);
}


/** Get egress_tm/num_queues attribute.
 *
 * Get Number of Queues.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_num_queues_get(bdmf_object_handle mo_, uint8_t *num_queues_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_num_queues, &_nn_);
    *num_queues_ = (uint8_t)_nn_;
    return _rc_;
}


/** Set egress_tm/num_queues attribute.
 *
 * Set Number of Queues.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_egress_tm_num_queues_set(bdmf_object_handle mo_, uint8_t num_queues_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_num_queues, num_queues_);
}


/** Get egress_tm/num_sp_elements attribute.
 *
 * Get Number of SP Scheduling Elements for SP_WRR Scheduling Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  num_sp_elements_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_num_sp_elements_get(bdmf_object_handle mo_, rdpa_tm_num_sp_elem *num_sp_elements_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_num_sp_elements, &_nn_);
    *num_sp_elements_ = (rdpa_tm_num_sp_elem)_nn_;
    return _rc_;
}


/** Set egress_tm/num_sp_elements attribute.
 *
 * Set Number of SP Scheduling Elements for SP_WRR Scheduling Mode.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   num_sp_elements_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_num_sp_elements_set(bdmf_object_handle mo_, rdpa_tm_num_sp_elem num_sp_elements_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_num_sp_elements, num_sp_elements_);
}


/** Get egress_tm/queue_cfg attribute entry.
 *
 * Get Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_cfg_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_tm_queue_cfg_t * queue_cfg_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_egress_tm_attr_queue_cfg, (bdmf_index)ai_, queue_cfg_, sizeof(*queue_cfg_));
}


/** Set egress_tm/queue_cfg attribute entry.
 *
 * Set Queue Parameters Configuration.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   queue_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_cfg_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_tm_queue_cfg_t * queue_cfg_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_egress_tm_attr_queue_cfg, (bdmf_index)ai_, queue_cfg_, sizeof(*queue_cfg_));
}


/** Set egress_tm/queue_flush attribute entry.
 *
 * Set Flush Egress Queue.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   queue_flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_flush_set(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, bdmf_boolean queue_flush_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_egress_tm_attr_queue_flush, (bdmf_index)ai_, queue_flush_);
}


/** Get egress_tm/queue_statistics attribute entry.
 *
 * Get Dropped Service Queue Statistics.

 * This function has been deprecated.
 *
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_statistics_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_statistics_get(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, rdpa_stat_1way_t * queue_statistics_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_egress_tm_attr_queue_statistics, (bdmf_index)ai_, queue_statistics_, sizeof(*queue_statistics_));
}


/** Get next egress_tm/queue_statistics attribute entry.
 *
 * Get next Dropped Service Queue Statistics.

 * This function has been deprecated.
 *
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_statistics_get_next(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_egress_tm_attr_queue_statistics, (bdmf_index *)ai_);
}


/** Get egress_tm/queue_stat attribute entry.
 *
 * Get Retrieve Egress Queue Statistics.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  queue_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_stat_get(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, rdpa_stat_1way_t * queue_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_egress_tm_attr_queue_stat, (bdmf_index)ai_, queue_stat_, sizeof(*queue_stat_));
}


/** Set egress_tm/queue_stat attribute entry.
 *
 * Set Retrieve Egress Queue Statistics.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   queue_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_stat_set(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_, const rdpa_stat_1way_t * queue_stat_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_egress_tm_attr_queue_stat, (bdmf_index)ai_, queue_stat_, sizeof(*queue_stat_));
}


/** Get next egress_tm/queue_stat attribute entry.
 *
 * Get next Retrieve Egress Queue Statistics.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_queue_stat_get_next(bdmf_object_handle mo_, rdpa_tm_queue_index_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_egress_tm_attr_queue_stat, (bdmf_index *)ai_);
}


/** Get egress_tm/subsidiary attribute entry.
 *
 * Get Next Level Egress-TM.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  subsidiary_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_subsidiary_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_object_handle *subsidiary_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_egress_tm_attr_subsidiary, (bdmf_index)ai_, &_nn_);
    *subsidiary_ = (bdmf_object_handle)(long)_nn_;
    return _rc_;
}


/** Set egress_tm/subsidiary attribute entry.
 *
 * Set Next Level Egress-TM.
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   subsidiary_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_subsidiary_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_object_handle subsidiary_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_egress_tm_attr_subsidiary, (bdmf_index)ai_, (long)subsidiary_);
}


/** Get egress_tm/weight attribute.
 *
 * Get Weight for WRR scheduling (0 for unset).
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[out]  weight_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_weight_get(bdmf_object_handle mo_, bdmf_number *weight_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_egress_tm_attr_weight, &_nn_);
    *weight_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set egress_tm/weight attribute.
 *
 * Set Weight for WRR scheduling (0 for unset).
 * \param[in]   mo_ egress_tm object handle or mattr transaction handle
 * \param[in]   weight_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_egress_tm_weight_set(bdmf_object_handle mo_, bdmf_number weight_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_egress_tm_attr_weight, weight_);
}

/** @} end of egress_tm Doxygen group */




#endif /* _RDPA_AG_EGRESS_TM_H_ */
