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
 * tr471_spdsvc object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_TR471_SPDSVC_H_
#define _RDPA_AG_TR471_SPDSVC_H_

/** \addtogroup tr471_spdsvc
 * @{
 */


/** Get tr471_spdsvc type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a tr471_spdsvc object.
 * \return tr471_spdsvc type handle
 */
bdmf_type_handle rdpa_tr471_spdsvc_drv(void);

/* tr471_spdsvc: Attribute types */
typedef enum {
    rdpa_tr471_spdsvc_attr_tx_start = 0, /* tx_start : RW : aggregate tr471_spdsvc_tx_start(rdpa_tr471_spdsvc_tx_start_t) : start Tx test */
    rdpa_tr471_spdsvc_attr_is_tx_test_in_progress = 1, /* is_tx_test_in_progress : RW : bool : check if Tx test in progress */
    rdpa_tr471_spdsvc_attr_tx_done = 2, /* tx_done : W : number : Tx Test is done */
    rdpa_tr471_spdsvc_attr_rx_pkt_id = 3, /* rx_pkt_id : RW : aggregate tr471_spdsvc_rx_pkt_id(rdpa_tr471_spdsvc_rx_pkt_id_t) : Rx Packet Indentification Information */
    rdpa_tr471_spdsvc_attr_tx_complete_cb = 4, /* tx_complete_cb : RI : pointer : callback function upon Tx test completion */
} rdpa_tr471_spdsvc_attr_types;

extern int (*f_rdpa_tr471_spdsvc_get)(bdmf_object_handle *pmo);

/** Get tr471_spdsvc object.

 * This function returns tr471_spdsvc object instance.
 * \param[out] tr471_spdsvc_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_tr471_spdsvc_get(bdmf_object_handle *tr471_spdsvc_obj);

/** Get tr471_spdsvc/tx_start attribute.
 *
 * Get start Tx test.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[out]  tx_start_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tr471_spdsvc_tx_start_get(bdmf_object_handle mo_, rdpa_tr471_spdsvc_tx_start_t * tx_start_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tr471_spdsvc_attr_tx_start, tx_start_, sizeof(*tx_start_));
}


/** Set tr471_spdsvc/tx_start attribute.
 *
 * Set start Tx test.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[in]   tx_start_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tr471_spdsvc_tx_start_set(bdmf_object_handle mo_, const rdpa_tr471_spdsvc_tx_start_t * tx_start_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tr471_spdsvc_attr_tx_start, tx_start_, sizeof(*tx_start_));
}


/** Get tr471_spdsvc/is_tx_test_in_progress attribute.
 *
 * Get check if Tx test in progress.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[out]  is_tx_test_in_progress_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tr471_spdsvc_is_tx_test_in_progress_get(bdmf_object_handle mo_, bdmf_boolean *is_tx_test_in_progress_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_tr471_spdsvc_attr_is_tx_test_in_progress, &_nn_);
    *is_tx_test_in_progress_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set tr471_spdsvc/is_tx_test_in_progress attribute.
 *
 * Set check if Tx test in progress.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[in]   is_tx_test_in_progress_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tr471_spdsvc_is_tx_test_in_progress_set(bdmf_object_handle mo_, bdmf_boolean is_tx_test_in_progress_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tr471_spdsvc_attr_is_tx_test_in_progress, is_tx_test_in_progress_);
}


/** Set tr471_spdsvc/tx_done attribute.
 *
 * Set Tx Test is done.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[in]   tx_done_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tr471_spdsvc_tx_done_set(bdmf_object_handle mo_, bdmf_number tx_done_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tr471_spdsvc_attr_tx_done, tx_done_);
}


/** Get tr471_spdsvc/rx_pkt_id attribute.
 *
 * Get Rx Packet Indentification Information.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[out]  rx_pkt_id_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tr471_spdsvc_rx_pkt_id_get(bdmf_object_handle mo_, rdpa_tr471_spdsvc_rx_pkt_id_t * rx_pkt_id_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tr471_spdsvc_attr_rx_pkt_id, rx_pkt_id_, sizeof(*rx_pkt_id_));
}


/** Set tr471_spdsvc/rx_pkt_id attribute.
 *
 * Set Rx Packet Indentification Information.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[in]   rx_pkt_id_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tr471_spdsvc_rx_pkt_id_set(bdmf_object_handle mo_, const rdpa_tr471_spdsvc_rx_pkt_id_t * rx_pkt_id_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tr471_spdsvc_attr_rx_pkt_id, rx_pkt_id_, sizeof(*rx_pkt_id_));
}


/** Get tr471_spdsvc/tx_complete_cb attribute.
 *
 * Get callback function upon Tx test completion.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[out]  tx_complete_cb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tr471_spdsvc_tx_complete_cb_get(bdmf_object_handle mo_, void * *tx_complete_cb_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_tr471_spdsvc_attr_tx_complete_cb, &_nn_);
    *tx_complete_cb_ = (void *)(long)_nn_;
    return _rc_;
}


/** Set tr471_spdsvc/tx_complete_cb attribute.
 *
 * Set callback function upon Tx test completion.
 * \param[in]   mo_ tr471_spdsvc object handle or mattr transaction handle
 * \param[in]   tx_complete_cb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tr471_spdsvc_tx_complete_cb_set(bdmf_object_handle mo_, void * tx_complete_cb_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tr471_spdsvc_attr_tx_complete_cb, (long)tx_complete_cb_);
}

/** @} end of tr471_spdsvc Doxygen group */




#endif /* _RDPA_AG_TR471_SPDSVC_H_ */
