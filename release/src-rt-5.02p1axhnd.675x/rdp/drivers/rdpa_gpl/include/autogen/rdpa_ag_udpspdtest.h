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
 * udpspdtest object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_UDPSPDTEST_H_
#define _RDPA_AG_UDPSPDTEST_H_

/** \addtogroup udpspdtest
 * @{
 */


/** Get udpspdtest type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an udpspdtest object.
 * \return udpspdtest type handle
 */
bdmf_type_handle rdpa_udpspdtest_drv(void);

/* udpspdtest: Attribute types */
typedef enum {
    rdpa_udpspdtest_attr_cfg = 0, /* cfg : RW : aggregate udpspdtest_cfg(rdpa_udpspdtest_cfg_t) : UDP Speed Test configuration */
    rdpa_udpspdtest_attr_stream_stat = 1, /* stream_stat : RF : aggregate[] udpspdtest_stat(rdpa_udpspdtest_stat_t) : UDP Speed Test stream statistics */
    rdpa_udpspdtest_attr_rx_params = 2, /* rx_params : RWF : aggregate[] udpspdtest_rx_params(rdpa_udpspdtest_rx_params_t) : Stream RX parameters */
    rdpa_udpspdtest_attr_rx_start = 3, /* rx_start : W : number : Start packets receive on a stream */
    rdpa_udpspdtest_attr_rx_stop = 4, /* rx_stop : W : number : Stop packets receive on a steam */
    rdpa_udpspdtest_attr_ref_pkt = 5, /* ref_pkt : RWF : aggregate[] spdtest_ref_pkt(rdpa_spdtest_ref_pkt_t) : UDP Speed Test TX reference packet */
    rdpa_udpspdtest_attr_tx_params = 6, /* tx_params : RWF : aggregate[] udpspdtest_tx_params(rdpa_udpspdtest_tx_params_t) : Stream TX parametes */
    rdpa_udpspdtest_attr_tx_start = 7, /* tx_start : W : number : Start packets transmission on a stream */
    rdpa_udpspdtest_attr_tx_stop = 8, /* tx_stop : W : number : Stop packets transmission on a steam */
} rdpa_udpspdtest_attr_types;

extern int (*f_rdpa_udpspdtest_get)(bdmf_object_handle *pmo);

/** Get udpspdtest object.

 * This function returns udpspdtest object instance.
 * \param[out] udpspdtest_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_udpspdtest_get(bdmf_object_handle *udpspdtest_obj);

/** Get udpspdtest/cfg attribute.
 *
 * Get UDP Speed Test configuration.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[out]  cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_cfg_get(bdmf_object_handle mo_, rdpa_udpspdtest_cfg_t * cfg_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_udpspdtest_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Set udpspdtest/cfg attribute.
 *
 * Set UDP Speed Test configuration.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   cfg_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_cfg_set(bdmf_object_handle mo_, const rdpa_udpspdtest_cfg_t * cfg_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_udpspdtest_attr_cfg, cfg_, sizeof(*cfg_));
}


/** Get udpspdtest/stream_stat attribute entry.
 *
 * Get UDP Speed Test stream statistics.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  stream_stat_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_udpspdtest_stream_stat_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_udpspdtest_stat_t * stream_stat_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_udpspdtest_attr_stream_stat, (bdmf_index)ai_, stream_stat_, sizeof(*stream_stat_));
}


/** Get udpspdtest/rx_params attribute entry.
 *
 * Get Stream RX parameters.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  rx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_params_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_udpspdtest_rx_params_t * rx_params_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_udpspdtest_attr_rx_params, (bdmf_index)ai_, rx_params_, sizeof(*rx_params_));
}


/** Set udpspdtest/rx_params attribute entry.
 *
 * Set Stream RX parameters.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   rx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_params_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_udpspdtest_rx_params_t * rx_params_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_udpspdtest_attr_rx_params, (bdmf_index)ai_, rx_params_, sizeof(*rx_params_));
}


/** Set udpspdtest/rx_start attribute.
 *
 * Set Start packets receive on a stream.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   rx_start_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_start_set(bdmf_object_handle mo_, bdmf_number rx_start_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_udpspdtest_attr_rx_start, rx_start_);
}


/** Set udpspdtest/rx_stop attribute.
 *
 * Set Stop packets receive on a steam.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   rx_stop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_rx_stop_set(bdmf_object_handle mo_, bdmf_number rx_stop_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_udpspdtest_attr_rx_stop, rx_stop_);
}


/** Get udpspdtest/ref_pkt attribute entry.
 *
 * Get UDP Speed Test TX reference packet.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  ref_pkt_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_ref_pkt_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_spdtest_ref_pkt_t * ref_pkt_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_udpspdtest_attr_ref_pkt, (bdmf_index)ai_, ref_pkt_, sizeof(*ref_pkt_));
}


/** Set udpspdtest/ref_pkt attribute entry.
 *
 * Set UDP Speed Test TX reference packet.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   ref_pkt_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_ref_pkt_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_spdtest_ref_pkt_t * ref_pkt_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_udpspdtest_attr_ref_pkt, (bdmf_index)ai_, ref_pkt_, sizeof(*ref_pkt_));
}


/** Get udpspdtest/tx_params attribute entry.
 *
 * Get Stream TX parametes.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  tx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_params_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_udpspdtest_tx_params_t * tx_params_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_udpspdtest_attr_tx_params, (bdmf_index)ai_, tx_params_, sizeof(*tx_params_));
}


/** Set udpspdtest/tx_params attribute entry.
 *
 * Set Stream TX parametes.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   tx_params_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_params_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_udpspdtest_tx_params_t * tx_params_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_udpspdtest_attr_tx_params, (bdmf_index)ai_, tx_params_, sizeof(*tx_params_));
}


/** Set udpspdtest/tx_start attribute.
 *
 * Set Start packets transmission on a stream.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   tx_start_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_start_set(bdmf_object_handle mo_, bdmf_number tx_start_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_udpspdtest_attr_tx_start, tx_start_);
}


/** Set udpspdtest/tx_stop attribute.
 *
 * Set Stop packets transmission on a steam.
 * \param[in]   mo_ udpspdtest object handle or mattr transaction handle
 * \param[in]   tx_stop_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_udpspdtest_tx_stop_set(bdmf_object_handle mo_, bdmf_number tx_stop_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_udpspdtest_attr_tx_stop, tx_stop_);
}

/** @} end of udpspdtest Doxygen group */




#endif /* _RDPA_AG_UDPSPDTEST_H_ */
