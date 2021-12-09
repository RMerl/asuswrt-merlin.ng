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
 * tcpspdtest object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_TCPSPDTEST_H_
#define _RDPA_AG_TCPSPDTEST_H_

/** \addtogroup tcpspdtest
 * @{
 */


/** Get tcpspdtest type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a tcpspdtest object.
 * \return tcpspdtest type handle
 */
bdmf_type_handle rdpa_tcpspdtest_drv(void);

/* tcpspdtest: Attribute types */
typedef enum {
    rdpa_tcpspdtest_attr_engine_global_info = 0, /* engine_global_info : RW : aggregate tcpspdtest_engine_global_info(rdpa_tcpspdtest_engine_global_info_t) : Tcp speed test engin */
    rdpa_tcpspdtest_attr_engine_conn_info = 1, /* engine_conn_info : RWF : aggregate[] tcpspdtest_engine_conn_info(rdpa_tcpspdtest_engine_conn_info_t) : Tcp speed test engine c */
    rdpa_tcpspdtest_attr_engine_tcb = 2, /* engine_tcb : RWF : aggregate[] tcpspdtest_engine_tcb(rdpa_tcpspdtest_engine_tcb_t) : Tcp speed test engine TCB */
    rdpa_tcpspdtest_attr_engine_ref_pkt_hdr = 3, /* engine_ref_pkt_hdr : RWF : aggregate[] tcpspdtest_engine_ref_pkt_hdr(rdpa_tcpspdtest_engine_ref_pkt_hdr_t) : Tcp speed test en */
    rdpa_tcpspdtest_attr_engine_pkt_drop = 4, /* engine_pkt_drop : RWF : aggregate[] tcpspdtest_engine_pkt_drop(rdpa_tcpspdtest_engine_pkt_drop_t) : Tcp speed test engine pack */
    rdpa_tcpspdtest_attr_engine_ref_pkt = 5, /* engine_ref_pkt : RWF : aggregate[] spdtest_ref_pkt(rdpa_spdtest_ref_pkt_t) : Tcp speed test engine reference packet (upload) */
    rdpa_tcpspdtest_attr_engine_ref_pkts_clr = 6, /* engine_ref_pkts_clr : W : bool : Clear all tcp speed test engine reference packet (end of the test) */
    rdpa_tcpspdtest_attr_tx_start = 7, /* tx_start : W : aggregate tcpspdtest_tx_info(rdpa_tcpspdtest_tx_info_t) : Start packets transmission (upload) */
    rdpa_tcpspdtest_attr_tx_stop = 8, /* tx_stop : W : aggregate tcpspdtest_tx_info(rdpa_tcpspdtest_tx_info_t) : Stop packets transmission (upload) */
    rdpa_tcpspdtest_attr_is_on = 10, /* is_on : RW : bool : TCP speed test mode (on/off) */
} rdpa_tcpspdtest_attr_types;

extern int (*f_rdpa_tcpspdtest_get)(bdmf_object_handle *pmo);

/** Get tcpspdtest object.

 * This function returns tcpspdtest object instance.
 * \param[out] tcpspdtest_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_tcpspdtest_get(bdmf_object_handle *tcpspdtest_obj);

/** Get tcpspdtest/engine_global_info attribute.
 *
 * Get Tcp speed test engine global info.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[out]  engine_global_info_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_global_info_get(bdmf_object_handle mo_, rdpa_tcpspdtest_engine_global_info_t * engine_global_info_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_global_info, engine_global_info_, sizeof(*engine_global_info_));
}


/** Set tcpspdtest/engine_global_info attribute.
 *
 * Set Tcp speed test engine global info.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   engine_global_info_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_global_info_set(bdmf_object_handle mo_, const rdpa_tcpspdtest_engine_global_info_t * engine_global_info_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_global_info, engine_global_info_, sizeof(*engine_global_info_));
}


/** Get tcpspdtest/engine_conn_info attribute entry.
 *
 * Get Tcp speed test engine connection info.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  engine_conn_info_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_conn_info_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_tcpspdtest_engine_conn_info_t * engine_conn_info_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_conn_info, (bdmf_index)ai_, engine_conn_info_, sizeof(*engine_conn_info_));
}


/** Set tcpspdtest/engine_conn_info attribute entry.
 *
 * Set Tcp speed test engine connection info.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   engine_conn_info_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_conn_info_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_tcpspdtest_engine_conn_info_t * engine_conn_info_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_conn_info, (bdmf_index)ai_, engine_conn_info_, sizeof(*engine_conn_info_));
}


/** Get tcpspdtest/engine_tcb attribute entry.
 *
 * Get Tcp speed test engine TCB.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  engine_tcb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_tcb_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_tcpspdtest_engine_tcb_t * engine_tcb_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_tcb, (bdmf_index)ai_, engine_tcb_, sizeof(*engine_tcb_));
}


/** Set tcpspdtest/engine_tcb attribute entry.
 *
 * Set Tcp speed test engine TCB.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   engine_tcb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_tcb_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_tcpspdtest_engine_tcb_t * engine_tcb_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_tcb, (bdmf_index)ai_, engine_tcb_, sizeof(*engine_tcb_));
}


/** Get tcpspdtest/engine_ref_pkt_hdr attribute entry.
 *
 * Get Tcp speed test engine reference packet header.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  engine_ref_pkt_hdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_ref_pkt_hdr_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_tcpspdtest_engine_ref_pkt_hdr_t * engine_ref_pkt_hdr_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_ref_pkt_hdr, (bdmf_index)ai_, engine_ref_pkt_hdr_, sizeof(*engine_ref_pkt_hdr_));
}


/** Set tcpspdtest/engine_ref_pkt_hdr attribute entry.
 *
 * Set Tcp speed test engine reference packet header.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   engine_ref_pkt_hdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_ref_pkt_hdr_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_tcpspdtest_engine_ref_pkt_hdr_t * engine_ref_pkt_hdr_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_ref_pkt_hdr, (bdmf_index)ai_, engine_ref_pkt_hdr_, sizeof(*engine_ref_pkt_hdr_));
}


/** Get tcpspdtest/engine_pkt_drop attribute entry.
 *
 * Get Tcp speed test engine packet drop.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  engine_pkt_drop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_pkt_drop_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_tcpspdtest_engine_pkt_drop_t * engine_pkt_drop_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_pkt_drop, (bdmf_index)ai_, engine_pkt_drop_, sizeof(*engine_pkt_drop_));
}


/** Set tcpspdtest/engine_pkt_drop attribute entry.
 *
 * Set Tcp speed test engine packet drop.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   engine_pkt_drop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_pkt_drop_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_tcpspdtest_engine_pkt_drop_t * engine_pkt_drop_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_pkt_drop, (bdmf_index)ai_, engine_pkt_drop_, sizeof(*engine_pkt_drop_));
}


/** Get tcpspdtest/engine_ref_pkt attribute entry.
 *
 * Get Tcp speed test engine reference packet (upload).
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  engine_ref_pkt_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_ref_pkt_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_spdtest_ref_pkt_t * engine_ref_pkt_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_ref_pkt, (bdmf_index)ai_, engine_ref_pkt_, sizeof(*engine_ref_pkt_));
}


/** Set tcpspdtest/engine_ref_pkt attribute entry.
 *
 * Set Tcp speed test engine reference packet (upload).
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   engine_ref_pkt_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_ref_pkt_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_spdtest_ref_pkt_t * engine_ref_pkt_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_ref_pkt, (bdmf_index)ai_, engine_ref_pkt_, sizeof(*engine_ref_pkt_));
}


/** Invoke tcpspdtest/engine_ref_pkts_clr attribute.
 *
 * Invoke Clear all tcp speed test engine reference packet (end of the test).
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_engine_ref_pkts_clr(bdmf_object_handle mo_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tcpspdtest_attr_engine_ref_pkts_clr, 1);
}


/** Set tcpspdtest/tx_start attribute.
 *
 * Set Start packets transmission (upload).
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   tx_start_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_tx_start_set(bdmf_object_handle mo_, const rdpa_tcpspdtest_tx_info_t * tx_start_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tcpspdtest_attr_tx_start, tx_start_, sizeof(*tx_start_));
}


/** Set tcpspdtest/tx_stop attribute.
 *
 * Set Stop packets transmission (upload).
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   tx_stop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_tx_stop_set(bdmf_object_handle mo_, const rdpa_tcpspdtest_tx_info_t * tx_stop_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tcpspdtest_attr_tx_stop, tx_stop_, sizeof(*tx_stop_));
}


/** Get tcpspdtest/is_on attribute.
 *
 * Get TCP speed test mode (on/off).
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[out]  is_on_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_is_on_get(bdmf_object_handle mo_, bdmf_boolean *is_on_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_tcpspdtest_attr_is_on, &_nn_);
    *is_on_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set tcpspdtest/is_on attribute.
 *
 * Set TCP speed test mode (on/off).
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   is_on_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tcpspdtest_is_on_set(bdmf_object_handle mo_, bdmf_boolean is_on_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tcpspdtest_attr_is_on, is_on_);
}

/** @} end of tcpspdtest Doxygen group */




#endif /* _RDPA_AG_TCPSPDTEST_H_ */
