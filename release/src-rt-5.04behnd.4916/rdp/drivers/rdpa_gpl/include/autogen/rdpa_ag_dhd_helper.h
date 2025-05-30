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
 * dhd_helper object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_DHD_HELPER_H_
#define _RDPA_AG_DHD_HELPER_H_

/** \addtogroup dhd_helper
 * @{
 */


/** Get dhd_helper type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a dhd_helper object.
 * \return dhd_helper type handle
 */
bdmf_type_handle rdpa_dhd_helper_drv(void);

/* dhd_helper: Attribute types */
typedef enum {
    rdpa_dhd_helper_attr_radio_idx = 0, /* radio_idx : KRI : number : Radio Index */
    rdpa_dhd_helper_attr_init_cfg = 1, /* init_cfg : MRI : aggregate dhd_init_config(rdpa_dhd_init_cfg_t) : Initial DHD Configuration */
    rdpa_dhd_helper_attr_flush = 2, /* flush : W : number : Flush FlowRing */
    rdpa_dhd_helper_attr_flow_ring_enable = 3, /* flow_ring_enable : W : bool[] : Enable/Disable FlowRing */
    rdpa_dhd_helper_attr_rx_post_init = 4, /* rx_post_init : W : bool : RX Post init: allocate and push RX Post descriptors to Dongle */
    rdpa_dhd_helper_attr_ssid_tx_dropped_packets = 5, /* ssid_tx_dropped_packets : RF : number[] : SSID Dropped Packets */
    rdpa_dhd_helper_attr_aggregation_size = 6, /* aggregation_size : RWF : number[] : Number of packets to be aggregated on host side per AC */
    rdpa_dhd_helper_attr_aggregation_timeout = 7, /* aggregation_timeout : RWF : number[] : Aggregation expiration timeout on host side per AC */
    rdpa_dhd_helper_attr_aggregation_timer = 8, /* aggregation_timer : RW : number : Global Aggregation expiration timer on host (old implementation, keeping it for backward com */
    rdpa_dhd_helper_attr_int_connect = 9, /* int_connect : W : bool : Connect interrupts */
    rdpa_dhd_helper_attr_rx_post_uninit = 10, /* rx_post_uninit : W : bool : RX Post uninit: free the buffers allocated in RX Post descriptors to Dongle */
    rdpa_dhd_helper_attr_tx_complete_send2host = 11, /* tx_complete_send2host : RW : bool : Global flag: Tx Complete HOST_BUFFER type send to DHD (0 - don't send, 1 - send) */
    rdpa_dhd_helper_attr_cpu_data = 12, /* cpu_data : RW : aggregate dhd_cpu_data(rdpa_dhd_cpu_data_t) : DHD CPU Resources data */
    rdpa_dhd_helper_attr_rx_post_reinit = 14, /* rx_post_reinit : W : bool : RX Post reinit: Refill RX Post ring */
    rdpa_dhd_helper_attr_int_coalescing_amount = 15, /* int_coalescing_amount : RW : number : Amount of interrupt to coalesce (0 - disable) */
    rdpa_dhd_helper_attr_int_coalescing_timeout = 16, /* int_coalescing_timeout : RW : number : Coalescing timeout in msec */
    rdpa_dhd_helper_attr_codel_enable = 17, /* codel_enable : RW : bool : CoDel management switch */
    rdpa_dhd_helper_attr_flow_ring_update = 18, /* flow_ring_update : W : number : Update FlowRing */
} rdpa_dhd_helper_attr_types;

extern int (*f_rdpa_dhd_helper_get)(bdmf_number radio_idx_, bdmf_object_handle *pmo);

/** Get dhd_helper object by key.

 * This function returns dhd_helper object instance by key.
 * \param[in] radio_idx_    Object key
 * \param[out] dhd_helper_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_dhd_helper_get(bdmf_number radio_idx_, bdmf_object_handle *dhd_helper_obj);

/** Get dhd_helper/radio_idx attribute.
 *
 * Get Radio Index.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  radio_idx_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_radio_idx_get(bdmf_object_handle mo_, bdmf_number *radio_idx_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dhd_helper_attr_radio_idx, &_nn_);
    *radio_idx_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set dhd_helper/radio_idx attribute.
 *
 * Set Radio Index.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   radio_idx_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_radio_idx_set(bdmf_object_handle mo_, bdmf_number radio_idx_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_radio_idx, radio_idx_);
}


/** Get dhd_helper/init_cfg attribute.
 *
 * Get Initial DHD Configuration.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  init_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_init_cfg_get(bdmf_object_handle mo_, rdpa_dhd_init_cfg_t * init_cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_dhd_helper_attr_init_cfg, init_cfg_, sizeof(*init_cfg_));
}


/** Set dhd_helper/init_cfg attribute.
 *
 * Set Initial DHD Configuration.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   init_cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_init_cfg_set(bdmf_object_handle mo_, const rdpa_dhd_init_cfg_t * init_cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_dhd_helper_attr_init_cfg, init_cfg_, sizeof(*init_cfg_));
}


/** Set dhd_helper/flush attribute.
 *
 * Set Flush FlowRing.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_flush_set(bdmf_object_handle mo_, bdmf_number flush_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_flush, flush_);
}


/** Set dhd_helper/flow_ring_enable attribute entry.
 *
 * Set Enable/Disable FlowRing.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   flow_ring_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_flow_ring_enable_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean flow_ring_enable_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_dhd_helper_attr_flow_ring_enable, (bdmf_index)ai_, flow_ring_enable_);
}


/** Invoke dhd_helper/rx_post_init attribute.
 *
 * Invoke RX Post init: allocate and push RX Post descriptors to Dongle.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_rx_post_init(bdmf_object_handle mo_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_rx_post_init, 1);
}


/** Get dhd_helper/ssid_tx_dropped_packets attribute entry.
 *
 * Get SSID Dropped Packets.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ssid_tx_dropped_packets_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_ssid_tx_dropped_packets_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *ssid_tx_dropped_packets_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_dhd_helper_attr_ssid_tx_dropped_packets, (bdmf_index)ai_, &_nn_);
    *ssid_tx_dropped_packets_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get dhd_helper/aggregation_size attribute entry.
 *
 * Get Number of packets to be aggregated on host side per AC.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  aggregation_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_aggregation_size_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *aggregation_size_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_dhd_helper_attr_aggregation_size, (bdmf_index)ai_, &_nn_);
    *aggregation_size_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set dhd_helper/aggregation_size attribute entry.
 *
 * Set Number of packets to be aggregated on host side per AC.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   aggregation_size_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_aggregation_size_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number aggregation_size_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_dhd_helper_attr_aggregation_size, (bdmf_index)ai_, aggregation_size_);
}


/** Get dhd_helper/aggregation_timeout attribute entry.
 *
 * Get Aggregation expiration timeout on host side per AC.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  aggregation_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_aggregation_timeout_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *aggregation_timeout_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_dhd_helper_attr_aggregation_timeout, (bdmf_index)ai_, &_nn_);
    *aggregation_timeout_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set dhd_helper/aggregation_timeout attribute entry.
 *
 * Set Aggregation expiration timeout on host side per AC.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   aggregation_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_aggregation_timeout_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number aggregation_timeout_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_dhd_helper_attr_aggregation_timeout, (bdmf_index)ai_, aggregation_timeout_);
}


/** Get dhd_helper/aggregation_timer attribute.
 *
 * Get Global Aggregation expiration timer on host (old implementation, keeping it for backward compatibility).
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  aggregation_timer_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_aggregation_timer_get(bdmf_object_handle mo_, bdmf_number *aggregation_timer_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dhd_helper_attr_aggregation_timer, &_nn_);
    *aggregation_timer_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set dhd_helper/aggregation_timer attribute.
 *
 * Set Global Aggregation expiration timer on host (old implementation, keeping it for backward compatibility).
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   aggregation_timer_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_aggregation_timer_set(bdmf_object_handle mo_, bdmf_number aggregation_timer_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_aggregation_timer, aggregation_timer_);
}


/** Set dhd_helper/int_connect attribute.
 *
 * Set Connect interrupts.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   int_connect_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_int_connect_set(bdmf_object_handle mo_, bdmf_boolean int_connect_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_int_connect, int_connect_);
}


/** Invoke dhd_helper/rx_post_uninit attribute.
 *
 * Invoke RX Post uninit: free the buffers allocated in RX Post descriptors to Dongle.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_rx_post_uninit(bdmf_object_handle mo_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_rx_post_uninit, 1);
}


/** Get dhd_helper/tx_complete_send2host attribute.
 *
 * Get Global flag: Tx Complete HOST_BUFFER type send to DHD (0 - don't send, 1 - send).
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  tx_complete_send2host_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_tx_complete_send2host_get(bdmf_object_handle mo_, bdmf_boolean *tx_complete_send2host_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dhd_helper_attr_tx_complete_send2host, &_nn_);
    *tx_complete_send2host_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set dhd_helper/tx_complete_send2host attribute.
 *
 * Set Global flag: Tx Complete HOST_BUFFER type send to DHD (0 - don't send, 1 - send).
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   tx_complete_send2host_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_tx_complete_send2host_set(bdmf_object_handle mo_, bdmf_boolean tx_complete_send2host_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_tx_complete_send2host, tx_complete_send2host_);
}


/** Get dhd_helper/cpu_data attribute.
 *
 * Get DHD CPU Resources data.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  cpu_data_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_cpu_data_get(bdmf_object_handle mo_, rdpa_dhd_cpu_data_t * cpu_data_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_dhd_helper_attr_cpu_data, cpu_data_, sizeof(*cpu_data_));
}


/** Set dhd_helper/cpu_data attribute.
 *
 * Set DHD CPU Resources data.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   cpu_data_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_cpu_data_set(bdmf_object_handle mo_, const rdpa_dhd_cpu_data_t * cpu_data_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_dhd_helper_attr_cpu_data, cpu_data_, sizeof(*cpu_data_));
}


/** Invoke dhd_helper/rx_post_reinit attribute.
 *
 * Invoke RX Post reinit: Refill RX Post ring.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_rx_post_reinit(bdmf_object_handle mo_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_rx_post_reinit, 1);
}


/** Get dhd_helper/int_coalescing_amount attribute.
 *
 * Get Amount of interrupt to coalesce (0 - disable).
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  int_coalescing_amount_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_int_coalescing_amount_get(bdmf_object_handle mo_, bdmf_number *int_coalescing_amount_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dhd_helper_attr_int_coalescing_amount, &_nn_);
    *int_coalescing_amount_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set dhd_helper/int_coalescing_amount attribute.
 *
 * Set Amount of interrupt to coalesce (0 - disable).
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   int_coalescing_amount_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_int_coalescing_amount_set(bdmf_object_handle mo_, bdmf_number int_coalescing_amount_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_int_coalescing_amount, int_coalescing_amount_);
}


/** Get dhd_helper/int_coalescing_timeout attribute.
 *
 * Get Coalescing timeout in msec.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  int_coalescing_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_int_coalescing_timeout_get(bdmf_object_handle mo_, bdmf_number *int_coalescing_timeout_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dhd_helper_attr_int_coalescing_timeout, &_nn_);
    *int_coalescing_timeout_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set dhd_helper/int_coalescing_timeout attribute.
 *
 * Set Coalescing timeout in msec.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   int_coalescing_timeout_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_int_coalescing_timeout_set(bdmf_object_handle mo_, bdmf_number int_coalescing_timeout_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_int_coalescing_timeout, int_coalescing_timeout_);
}


/** Get dhd_helper/codel_enable attribute.
 *
 * Get CoDel management switch.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[out]  codel_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_codel_enable_get(bdmf_object_handle mo_, bdmf_boolean *codel_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_dhd_helper_attr_codel_enable, &_nn_);
    *codel_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set dhd_helper/codel_enable attribute.
 *
 * Set CoDel management switch.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   codel_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_dhd_helper_codel_enable_set(bdmf_object_handle mo_, bdmf_boolean codel_enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_codel_enable, codel_enable_);
}


/** Set dhd_helper/flow_ring_update attribute.
 *
 * Set Update FlowRing.
 * \param[in]   mo_ dhd_helper object handle or mattr transaction handle
 * \param[in]   flow_ring_update_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_dhd_helper_flow_ring_update_set(bdmf_object_handle mo_, bdmf_number flow_ring_update_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_dhd_helper_attr_flow_ring_update, flow_ring_update_);
}

/** @} end of dhd_helper Doxygen group */




#endif /* _RDPA_AG_DHD_HELPER_H_ */
