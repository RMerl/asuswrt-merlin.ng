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
 * epon object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_EPON_H_
#define _RDPA_AG_EPON_H_

/** \addtogroup epon
 * @{
 */


/** Get epon type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an epon object.
 * \return epon type handle
 */
bdmf_type_handle rdpa_epon_drv(void);

/* epon: Attribute types */
typedef enum {
    rdpa_epon_attr_rate = 0, /* mode : RW : enum/4 : epon oam: downstream/upstream rate */
    rdpa_epon_attr_mode = 1, /* mode : RW : enum/4 : epon oam: ctc/dpoe/bcm */
    rdpa_epon_attr_init = 2, /* init : W : bool/1 : one time EPON stack init */
    rdpa_epon_attr_max_link_count = 3, /* max_link_count : RW : number/1 : how many links can be used */
    rdpa_epon_attr_registered_links = 4, /* registered_links : R : number/1 : how many links are registered */
    rdpa_epon_attr_laser_tx_mode = 5, /* laser_tx_mode : RW : enum/4 : Set the laser TX mode */
    rdpa_epon_attr_laser_rx_enable = 6, /* laser_rx_enable : RW : bool/1 : Set the Rx laser enable */
    rdpa_epon_attr_holdover = 7, /* holdover : RW : aggregate/8 epon_holdover(rdpa_epon_holdover_t) : EPON holdover properties */
    rdpa_epon_attr_los_threshold = 8, /* los_threshold : RW : aggregate/4 epon_los(rdpa_epon_los_t) : PON and Gate LOS time threshold        before any action is taken */
    rdpa_epon_attr_link_enable = 9, /* link_enable : RWF : bool/1[8] : Enable epon link */
    rdpa_epon_attr_link_flush = 10, /* link_flush : W : bool[] : Flush epon link */
    rdpa_epon_attr_link_mpcp_state = 11, /* link_mpcp_state : RF : enum/4[8] : Get the link MPCP regitration state */
    rdpa_epon_attr_mcast_link = 12, /* mcast_link : RWF : aggregate/5[8] epon_mcast_link(rdpa_epon_mcast_link_t) : EPON multicast link properties */
    rdpa_epon_attr_fec_enable = 13, /* fec_enable : RWF : aggregate/2[8] epon_fec_enable(rdpa_epon_fec_enable_t) : enable EPON MAC FEC for us/ds or both */
    rdpa_epon_attr_burst_cap_map = 14, /* burst_cap_map : RWF : aggregate/16[8] burst_cap_per_priority(rdpa_epon_burst_cap_per_priority_t) : all links burst cap per pri */
} rdpa_epon_attr_types;

extern int (*f_rdpa_epon_get)(bdmf_object_handle *pmo);

/** Get epon object.

 * This function returns epon object instance.
 * \param[out] epon_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_epon_get(bdmf_object_handle *epon_obj);

/** Get epon/rate attribute.
 *
 * Get epon rate: downstream/upstream rate.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  rate_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_rate_get(bdmf_object_handle mo_, rdpa_epon_rate *rate_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_epon_attr_rate, &_nn_);
    *rate_ = (rdpa_epon_rate)_nn_;
    return _rc_;
}


/** Set epon/rate attribute.
 *
 * Set epon rate: downstream/upstream rate.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   rate_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_rate_set(bdmf_object_handle mo_, rdpa_epon_rate rate_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_epon_attr_rate, rate_);
}


/** Get epon/mode attribute.
 *
 * Get epon oam: ctc/dpoe/bcm.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_mode_get(bdmf_object_handle mo_, rdpa_epon_mode *mode_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_epon_attr_mode, &_nn_);
    *mode_ = (rdpa_epon_mode)_nn_;
    return _rc_;
}


/** Set epon/mode attribute.
 *
 * Set epon oam: ctc/dpoe/bcm.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_mode_set(bdmf_object_handle mo_, rdpa_epon_mode mode_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_epon_attr_mode, mode_);
}


/** Set epon/init attribute.
 *
 * Set one time EPON stack init.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   init_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_init_set(bdmf_object_handle mo_, bdmf_boolean init_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_epon_attr_init, init_);
}


/** Get epon/max_link_count attribute.
 *
 * Get how many links can be used.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  max_link_count_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_max_link_count_get(bdmf_object_handle mo_, bdmf_number *max_link_count_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_epon_attr_max_link_count, &_nn_);
    *max_link_count_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set epon/max_link_count attribute.
 *
 * Set how many links can be used.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   max_link_count_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_max_link_count_set(bdmf_object_handle mo_, bdmf_number max_link_count_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_epon_attr_max_link_count, max_link_count_);
}


/** Get epon/registered_links attribute.
 *
 * Get how many links are registered.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  registered_links_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_registered_links_get(bdmf_object_handle mo_, bdmf_number *registered_links_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_epon_attr_registered_links, &_nn_);
    *registered_links_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Get epon/laser_tx_mode attribute.
 *
 * Get Set the laser TX mode.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  laser_tx_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_laser_tx_mode_get(bdmf_object_handle mo_, rdpa_epon_laser_tx_mode *laser_tx_mode_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_epon_attr_laser_tx_mode, &_nn_);
    *laser_tx_mode_ = (rdpa_epon_laser_tx_mode)_nn_;
    return _rc_;
}


/** Set epon/laser_tx_mode attribute.
 *
 * Set Set the laser TX mode.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   laser_tx_mode_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_laser_tx_mode_set(bdmf_object_handle mo_, rdpa_epon_laser_tx_mode laser_tx_mode_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_epon_attr_laser_tx_mode, laser_tx_mode_);
}


/** Get epon/laser_rx_enable attribute.
 *
 * Get Set the Rx laser enable.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  laser_rx_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_laser_rx_enable_get(bdmf_object_handle mo_, bdmf_boolean *laser_rx_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_epon_attr_laser_rx_enable, &_nn_);
    *laser_rx_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set epon/laser_rx_enable attribute.
 *
 * Set Set the Rx laser enable.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   laser_rx_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_laser_rx_enable_set(bdmf_object_handle mo_, bdmf_boolean laser_rx_enable_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_epon_attr_laser_rx_enable, laser_rx_enable_);
}


/** Get epon/holdover attribute.
 *
 * Get EPON holdover properties.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  holdover_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_holdover_get(bdmf_object_handle mo_, rdpa_epon_holdover_t * holdover_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_epon_attr_holdover, holdover_, sizeof(*holdover_));
}


/** Set epon/holdover attribute.
 *
 * Set EPON holdover properties.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   holdover_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_holdover_set(bdmf_object_handle mo_, const rdpa_epon_holdover_t * holdover_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_epon_attr_holdover, holdover_, sizeof(*holdover_));
}


/** Get epon/los_threshold attribute.
 *
 * Get PON and Gate LOS time threshold        before any action is taken. Time unit is [milliseconds].
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[out]  los_threshold_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_los_threshold_get(bdmf_object_handle mo_, rdpa_epon_los_t * los_threshold_)
{
    return bdmf_attr_get_as_buf(mo_, rdpa_epon_attr_los_threshold, los_threshold_, sizeof(*los_threshold_));
}


/** Set epon/los_threshold attribute.
 *
 * Set PON and Gate LOS time threshold        before any action is taken. Time unit is [milliseconds].
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   los_threshold_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_los_threshold_set(bdmf_object_handle mo_, const rdpa_epon_los_t * los_threshold_)
{
    return bdmf_attr_set_as_buf(mo_, rdpa_epon_attr_los_threshold, los_threshold_, sizeof(*los_threshold_));
}


/** Get epon/link_enable attribute entry.
 *
 * Get Enable epon link.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  link_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_link_enable_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean *link_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_epon_attr_link_enable, (bdmf_index)ai_, &_nn_);
    *link_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set epon/link_enable attribute entry.
 *
 * Set Enable epon link.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   link_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_link_enable_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean link_enable_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_epon_attr_link_enable, (bdmf_index)ai_, link_enable_);
}


/** Set epon/link_flush attribute entry.
 *
 * Set Flush epon link.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   link_flush_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_link_flush_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean link_flush_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_epon_attr_link_flush, (bdmf_index)ai_, link_flush_);
}


/** Get epon/link_mpcp_state attribute entry.
 *
 * Get Get the link MPCP regitration state.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  link_mpcp_state_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_link_mpcp_state_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_epon_link_mpcp_state *link_mpcp_state_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_epon_attr_link_mpcp_state, (bdmf_index)ai_, &_nn_);
    *link_mpcp_state_ = (rdpa_epon_link_mpcp_state)_nn_;
    return _rc_;
}


/** Get epon/mcast_link attribute entry.
 *
 * Get EPON multicast link properties.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  mcast_link_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_mcast_link_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_epon_mcast_link_t * mcast_link_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_epon_attr_mcast_link, (bdmf_index)ai_, mcast_link_, sizeof(*mcast_link_));
}


/** Set epon/mcast_link attribute entry.
 *
 * Set EPON multicast link properties.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   mcast_link_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_mcast_link_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_epon_mcast_link_t * mcast_link_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_epon_attr_mcast_link, (bdmf_index)ai_, mcast_link_, sizeof(*mcast_link_));
}


/** Get epon/fec_enable attribute entry.
 *
 * Get enable EPON MAC FEC for us/ds or both.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  fec_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_fec_enable_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_epon_fec_enable_t * fec_enable_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_epon_attr_fec_enable, (bdmf_index)ai_, fec_enable_, sizeof(*fec_enable_));
}


/** Set epon/fec_enable attribute entry.
 *
 * Set enable EPON MAC FEC for us/ds or both.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   fec_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_fec_enable_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_epon_fec_enable_t * fec_enable_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_epon_attr_fec_enable, (bdmf_index)ai_, fec_enable_, sizeof(*fec_enable_));
}


/** Get epon/burst_cap_map attribute entry.
 *
 * Get all links burst cap per priority level             the index is the link number.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  burst_cap_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_burst_cap_map_get(bdmf_object_handle mo_, bdmf_index ai_, rdpa_epon_burst_cap_per_priority_t * burst_cap_map_)
{
    return bdmf_attrelem_get_as_buf(mo_, rdpa_epon_attr_burst_cap_map, (bdmf_index)ai_, burst_cap_map_, sizeof(*burst_cap_map_));
}


/** Set epon/burst_cap_map attribute entry.
 *
 * Set all links burst cap per priority level             the index is the link number.
 * \param[in]   mo_ epon object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   burst_cap_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_epon_burst_cap_map_set(bdmf_object_handle mo_, bdmf_index ai_, const rdpa_epon_burst_cap_per_priority_t * burst_cap_map_)
{
    return bdmf_attrelem_set_as_buf(mo_, rdpa_epon_attr_burst_cap_map, (bdmf_index)ai_, burst_cap_map_, sizeof(*burst_cap_map_));
}

/** @} end of epon Doxygen group */




#endif /* _RDPA_AG_EPON_H_ */
