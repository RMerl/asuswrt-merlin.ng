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
    rdpa_tcpspdtest_attr_engine_conn_info = 0, /* engine_conn_info : RW : aggregate tcpspdtest_engine_conn_info(rdpa_tcpspdtest_engine_conn_info_t) : Tcp Speed Test Engine CONN */
    rdpa_tcpspdtest_attr_engine_tcb = 1, /* engine_tcb : RW : aggregate tcpspdtest_engine_tcb(rdpa_tcpspdtest_engine_tcb_t) : Tcp Speed Test Engine TCB */
    rdpa_tcpspdtest_attr_engine_ref_pkt_hdr = 2, /* engine_ref_pkt_hdr : RW : aggregate tcpspdtest_engine_ref_pkt_hdr(rdpa_tcpspdtest_engine_ref_pkt_hdr_t) : Tcp Speed Test Engin */
    rdpa_tcpspdtest_attr_engine_pkt_drop = 3, /* engine_pkt_drop : RW : aggregate tcpspdtest_engine_pkt_drop(rdpa_tcpspdtest_engine_pkt_drop_t) : Tcp Speed Test Engine Packet  */
} rdpa_tcpspdtest_attr_types;

extern int (*f_rdpa_tcpspdtest_get)(bdmf_object_handle *pmo);

/** Get tcpspdtest object.

 * This function returns tcpspdtest object instance.
 * \param[out] tcpspdtest_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_tcpspdtest_get(bdmf_object_handle *tcpspdtest_obj);

/** Get tcpspdtest/engine_conn_info attribute.
 *
 * Get Tcp Speed Test Engine CONN_INFO.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[out]  engine_conn_info_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_conn_info_get(bdmf_object_handle mo_, rdpa_tcpspdtest_engine_conn_info_t * engine_conn_info_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_conn_info, engine_conn_info_, sizeof(*engine_conn_info_));
}


/** Set tcpspdtest/engine_conn_info attribute.
 *
 * Set Tcp Speed Test Engine CONN_INFO.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   engine_conn_info_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_conn_info_set(bdmf_object_handle mo_, const rdpa_tcpspdtest_engine_conn_info_t * engine_conn_info_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_conn_info, engine_conn_info_, sizeof(*engine_conn_info_));
}


/** Get tcpspdtest/engine_tcb attribute.
 *
 * Get Tcp Speed Test Engine TCB.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[out]  engine_tcb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_tcb_get(bdmf_object_handle mo_, rdpa_tcpspdtest_engine_tcb_t * engine_tcb_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_tcb, engine_tcb_, sizeof(*engine_tcb_));
}


/** Set tcpspdtest/engine_tcb attribute.
 *
 * Set Tcp Speed Test Engine TCB.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   engine_tcb_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_tcb_set(bdmf_object_handle mo_, const rdpa_tcpspdtest_engine_tcb_t * engine_tcb_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_tcb, engine_tcb_, sizeof(*engine_tcb_));
}


/** Get tcpspdtest/engine_ref_pkt_hdr attribute.
 *
 * Get Tcp Speed Test Engine Reference Packet Header.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[out]  engine_ref_pkt_hdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_ref_pkt_hdr_get(bdmf_object_handle mo_, rdpa_tcpspdtest_engine_ref_pkt_hdr_t * engine_ref_pkt_hdr_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_ref_pkt_hdr, engine_ref_pkt_hdr_, sizeof(*engine_ref_pkt_hdr_));
}


/** Set tcpspdtest/engine_ref_pkt_hdr attribute.
 *
 * Set Tcp Speed Test Engine Reference Packet Header.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   engine_ref_pkt_hdr_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_ref_pkt_hdr_set(bdmf_object_handle mo_, const rdpa_tcpspdtest_engine_ref_pkt_hdr_t * engine_ref_pkt_hdr_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_ref_pkt_hdr, engine_ref_pkt_hdr_, sizeof(*engine_ref_pkt_hdr_));
}


/** Get tcpspdtest/engine_pkt_drop attribute.
 *
 * Get Tcp Speed Test Engine Packet Drop.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[out]  engine_pkt_drop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_pkt_drop_get(bdmf_object_handle mo_, rdpa_tcpspdtest_engine_pkt_drop_t * engine_pkt_drop_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_tcpspdtest_attr_engine_pkt_drop, engine_pkt_drop_, sizeof(*engine_pkt_drop_));
}


/** Set tcpspdtest/engine_pkt_drop attribute.
 *
 * Set Tcp Speed Test Engine Packet Drop.
 * \param[in]   mo_ tcpspdtest object handle or mattr transaction handle
 * \param[in]   engine_pkt_drop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tcpspdtest_engine_pkt_drop_set(bdmf_object_handle mo_, const rdpa_tcpspdtest_engine_pkt_drop_t * engine_pkt_drop_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_tcpspdtest_attr_engine_pkt_drop, engine_pkt_drop_, sizeof(*engine_pkt_drop_));
}

/** @} end of tcpspdtest Doxygen group */




#endif /* _RDPA_AG_TCPSPDTEST_H_ */
