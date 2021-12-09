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
 * cpu object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_CPU_H_
#define _RDPA_AG_CPU_H_

/** \addtogroup cpu
 * @{
 */


/** Get cpu type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a cpu object.
 * \return cpu type handle
 */
bdmf_type_handle rdpa_cpu_drv(void);

/* cpu: Attribute types */
typedef enum {
    rdpa_cpu_attr_index = 0, /* index : KRI : enum : CPU interface */
    rdpa_cpu_attr_num_queues = 1, /* num_queues : RI : number : Number of receive queues */
    rdpa_cpu_attr_rxq_cfg = 2, /* rxq_cfg : RWF : aggregate[] cpu_rxq_cfg(rdpa_cpu_rxq_cfg_t) : Receive queue configuration */
    rdpa_cpu_attr_rxq_flush = 3, /* rxq_flush : W : bool[] : Flush receive queue */
    rdpa_cpu_attr_rxq_stat = 4, /* rxq_stat : RWF : aggregate[] cpu_rx_stat(rdpa_cpu_rx_stat_t) : Rx statistics */
    rdpa_cpu_attr_meter_cfg = 5, /* meter_cfg : RWF : aggregate[(rdpa_dir_index)] cpu_meter_cfg(rdpa_cpu_meter_cfg_t) : CPU meter configuration */
    rdpa_cpu_attr_meter_stat = 6, /* meter_stat : RWF : number[(rdpa_dir_index)] : CPU meter drop counter */
    rdpa_cpu_attr_reason_cfg = 7, /* reason_cfg : RWF : aggregate[(cpu_reason_index)] cpu_reason_cfg(rdpa_cpu_reason_cfg_t) : Trap reason configuration */
    rdpa_cpu_attr_reason_stat = 8, /* reason_stat : RWF : number[(cpu_reason_index)] : Per trap reason statistics */
    rdpa_cpu_attr_reason_stat_external_cb = 9, /* reason_stat_external_cb : RW : pointer : Reason statistics external callback */
    rdpa_cpu_attr_int_connect = 10, /* int_connect : W : bool : Connect interrupts */
    rdpa_cpu_attr_tc_to_rxq = 16, /* tc_to_rxq : RWF : number[] : TC to CPU RX queue mapping */
} rdpa_cpu_attr_types;

extern int (*f_rdpa_cpu_get)(rdpa_cpu_port index_, bdmf_object_handle *pmo);

/** Get cpu object by key.

 * This function returns cpu object instance by key.
 * \param[in] index_    Object key
 * \param[out] cpu_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_cpu_get(rdpa_cpu_port index_, bdmf_object_handle *cpu_obj);

/** Get cpu/index attribute.
 *
 * Get CPU interface.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[out]  index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_index_get(bdmf_object_handle mo_, rdpa_cpu_port *index_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_cpu_attr_index, &_nn_);
    *index_ = (rdpa_cpu_port)_nn_;
    return _rc_;
}


/** Set cpu/index attribute.
 *
 * Set CPU interface.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   index_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_index_set(bdmf_object_handle mo_, rdpa_cpu_port index_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_cpu_attr_index, index_);
}


/** Get cpu/num_queues attribute.
 *
 * Get Number of receive queues.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[out]  num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_num_queues_get(bdmf_object_handle mo_, bdmf_number *num_queues_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_cpu_attr_num_queues, &_nn_);
    *num_queues_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set cpu/num_queues attribute.
 *
 * Set Number of receive queues.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   num_queues_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_num_queues_set(bdmf_object_handle mo_, bdmf_number num_queues_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_cpu_attr_num_queues, num_queues_);
}


/** Get cpu/rxq_cfg attribute entry.
 *
 * Get Receive queue configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  rxq_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_rxq_cfg_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_cpu_rxq_cfg_t * rxq_cfg_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_cpu_attr_rxq_cfg, (bdmf_index)ai_, rxq_cfg_, sizeof(*rxq_cfg_));
}


/** Set cpu/rxq_cfg attribute entry.
 *
 * Set Receive queue configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   rxq_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_rxq_cfg_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_cpu_rxq_cfg_t * rxq_cfg_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_cpu_attr_rxq_cfg, (bdmf_index)ai_, rxq_cfg_, sizeof(*rxq_cfg_));
}


/** Set cpu/rxq_flush attribute entry.
 *
 * Set Flush receive queue.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   rxq_flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_rxq_flush_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean rxq_flush_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_cpu_attr_rxq_flush, (bdmf_index)ai_, rxq_flush_);
}


/** Get cpu/rxq_stat attribute entry.
 *
 * Get Rx statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  rxq_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_rxq_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_cpu_rx_stat_t * rxq_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_cpu_attr_rxq_stat, (bdmf_index)ai_, rxq_stat_, sizeof(*rxq_stat_));
}


/** Set cpu/rxq_stat attribute entry.
 *
 * Set Rx statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   rxq_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_rxq_stat_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_cpu_rx_stat_t * rxq_stat_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_cpu_attr_rxq_stat, (bdmf_index)ai_, rxq_stat_, sizeof(*rxq_stat_));
}


/** Get cpu/meter_cfg attribute entry.
 *
 * Get CPU meter configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  meter_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_cfg_get(bdmf_object_handle mo_, rdpa_dir_index_t * ai_, rdpa_cpu_meter_cfg_t * meter_cfg_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_cpu_attr_meter_cfg, (bdmf_index)ai_, meter_cfg_, sizeof(*meter_cfg_));
}


/** Set cpu/meter_cfg attribute entry.
 *
 * Set CPU meter configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   meter_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_cfg_set(bdmf_object_handle mo_, rdpa_dir_index_t * ai_, const rdpa_cpu_meter_cfg_t * meter_cfg_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_cpu_attr_meter_cfg, (bdmf_index)ai_, meter_cfg_, sizeof(*meter_cfg_));
}


/** Get next cpu/meter_cfg attribute entry.
 *
 * Get next CPU meter configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_cfg_get_next(bdmf_object_handle mo_, rdpa_dir_index_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_cpu_attr_meter_cfg, (bdmf_index *)ai_);
}


/** Get cpu/meter_stat attribute entry.
 *
 * Get CPU meter drop counter.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  meter_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_stat_get(bdmf_object_handle mo_, rdpa_dir_index_t * ai_, bdmf_number *meter_stat_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_cpu_attr_meter_stat, (bdmf_index)ai_, &_nn_);
    *meter_stat_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set cpu/meter_stat attribute entry.
 *
 * Set CPU meter drop counter.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   meter_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_stat_set(bdmf_object_handle mo_, rdpa_dir_index_t * ai_, bdmf_number meter_stat_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_cpu_attr_meter_stat, (bdmf_index)ai_, meter_stat_);
}


/** Get next cpu/meter_stat attribute entry.
 *
 * Get next CPU meter drop counter.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_meter_stat_get_next(bdmf_object_handle mo_, rdpa_dir_index_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_cpu_attr_meter_stat, (bdmf_index *)ai_);
}


/** Get cpu/reason_cfg attribute entry.
 *
 * Get Trap reason configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  reason_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_cfg_get(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, rdpa_cpu_reason_cfg_t * reason_cfg_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_cpu_attr_reason_cfg, (bdmf_index)ai_, reason_cfg_, sizeof(*reason_cfg_));
}


/** Set cpu/reason_cfg attribute entry.
 *
 * Set Trap reason configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   reason_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_cfg_set(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, const rdpa_cpu_reason_cfg_t * reason_cfg_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_cpu_attr_reason_cfg, (bdmf_index)ai_, reason_cfg_, sizeof(*reason_cfg_));
}


/** Get next cpu/reason_cfg attribute entry.
 *
 * Get next Trap reason configuration.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_cfg_get_next(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_cpu_attr_reason_cfg, (bdmf_index *)ai_);
}


/** Get cpu/reason_stat attribute entry.
 *
 * Get Per trap reason statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  reason_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_get(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, bdmf_number *reason_stat_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_cpu_attr_reason_stat, (bdmf_index)ai_, &_nn_);
    *reason_stat_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set cpu/reason_stat attribute entry.
 *
 * Set Per trap reason statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   reason_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_set(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_, bdmf_number reason_stat_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_cpu_attr_reason_stat, (bdmf_index)ai_, reason_stat_);
}


/** Get next cpu/reason_stat attribute entry.
 *
 * Get next Per trap reason statistics.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_get_next(bdmf_object_handle mo_, rdpa_cpu_reason_index_t * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_cpu_attr_reason_stat, (bdmf_index *)ai_);
}


/** Get cpu/reason_stat_external_cb attribute.
 *
 * Get Reason statistics external callback.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[out]  reason_stat_external_cb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_external_cb_get(bdmf_object_handle mo_, void * *reason_stat_external_cb_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_cpu_attr_reason_stat_external_cb, &_nn_);
    *reason_stat_external_cb_ = (void *)(long)_nn_;
    return _rc_;
}


/** Set cpu/reason_stat_external_cb attribute.
 *
 * Set Reason statistics external callback.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   reason_stat_external_cb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_reason_stat_external_cb_set(bdmf_object_handle mo_, void * reason_stat_external_cb_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_cpu_attr_reason_stat_external_cb, (long)reason_stat_external_cb_);
}


/** Set cpu/int_connect attribute.
 *
 * Set Connect interrupts.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   int_connect_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_cpu_int_connect_set(bdmf_object_handle mo_, bdmf_boolean int_connect_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_cpu_attr_int_connect, int_connect_);
}


/** Get cpu/tc_to_rxq attribute entry.
 *
 * Get TC to CPU RX queue mapping.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  tc_to_rxq_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_tc_to_rxq_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *tc_to_rxq_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_cpu_attr_tc_to_rxq, (bdmf_index)ai_, &_nn_);
    *tc_to_rxq_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set cpu/tc_to_rxq attribute entry.
 *
 * Set TC to CPU RX queue mapping.
 * \param[in]   mo_ cpu object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   tc_to_rxq_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_cpu_tc_to_rxq_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number tc_to_rxq_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_cpu_attr_tc_to_rxq, (bdmf_index)ai_, tc_to_rxq_);
}

/** @} end of cpu Doxygen group */




#endif /* _RDPA_AG_CPU_H_ */
