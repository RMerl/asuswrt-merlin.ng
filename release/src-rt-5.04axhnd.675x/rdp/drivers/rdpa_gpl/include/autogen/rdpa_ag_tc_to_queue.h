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
 * tc_to_queue object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_TC_TO_QUEUE_H_
#define _RDPA_AG_TC_TO_QUEUE_H_

/** \addtogroup tc_to_queue
 * @{
 */


/** Get tc_to_queue type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a tc_to_queue object.
 * \return tc_to_queue type handle
 */
bdmf_type_handle rdpa_tc_to_queue_drv(void);

/* tc_to_queue: Attribute types */
typedef enum {
    rdpa_tc_to_queue_attr_table = 0, /* table : KRI : number : Table index */
    rdpa_tc_to_queue_attr_dir = 1, /* dir : MKRI : enum : Traffic Direction */
    rdpa_tc_to_queue_attr_tc_map = 2, /* tc_map : RWF : number[] : TC to queue array */
} rdpa_tc_to_queue_attr_types;

/** tc_to_queue object key. */
typedef struct {
    bdmf_number table; /**< tc_to_queue: Table index */
    rdpa_traffic_dir dir; /**< tc_to_queue: Traffic Direction */
} rdpa_tc_to_queue_key_t;


extern int (*f_rdpa_tc_to_queue_get)(const rdpa_tc_to_queue_key_t * key_, bdmf_object_handle *pmo);

/** Get tc_to_queue object by key.

 * This function returns tc_to_queue object instance by key.
 * \param[in] key_    Object key
 * \param[out] tc_to_queue_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_tc_to_queue_get(const rdpa_tc_to_queue_key_t * key_, bdmf_object_handle *tc_to_queue_obj);

/** Get tc_to_queue/table attribute.
 *
 * Get Table index.
 * \param[in]   mo_ tc_to_queue object handle or mattr transaction handle
 * \param[out]  table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tc_to_queue_table_get(bdmf_object_handle mo_, bdmf_number *table_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_tc_to_queue_attr_table, &_nn_);
    *table_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set tc_to_queue/table attribute.
 *
 * Set Table index.
 * \param[in]   mo_ tc_to_queue object handle or mattr transaction handle
 * \param[in]   table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tc_to_queue_table_set(bdmf_object_handle mo_, bdmf_number table_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tc_to_queue_attr_table, table_);
}


/** Get tc_to_queue/dir attribute.
 *
 * Get Traffic Direction.
 * \param[in]   mo_ tc_to_queue object handle or mattr transaction handle
 * \param[out]  dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tc_to_queue_dir_get(bdmf_object_handle mo_, rdpa_traffic_dir *dir_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_tc_to_queue_attr_dir, &_nn_);
    *dir_ = (rdpa_traffic_dir)_nn_;
    return _rc_;
}


/** Set tc_to_queue/dir attribute.
 *
 * Set Traffic Direction.
 * \param[in]   mo_ tc_to_queue object handle or mattr transaction handle
 * \param[in]   dir_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_tc_to_queue_dir_set(bdmf_object_handle mo_, rdpa_traffic_dir dir_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_tc_to_queue_attr_dir, dir_);
}


/** Get tc_to_queue/tc_map attribute entry.
 *
 * Get TC to queue array.
 * \param[in]   mo_ tc_to_queue object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  tc_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tc_to_queue_tc_map_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *tc_map_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_tc_to_queue_attr_tc_map, (bdmf_index)ai_, &_nn_);
    *tc_map_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set tc_to_queue/tc_map attribute entry.
 *
 * Set TC to queue array.
 * \param[in]   mo_ tc_to_queue object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   tc_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tc_to_queue_tc_map_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number tc_map_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_tc_to_queue_attr_tc_map, (bdmf_index)ai_, tc_map_);
}


/** Get next tc_to_queue/tc_map attribute entry.
 *
 * Get next TC to queue array.
 * \param[in]   mo_ tc_to_queue object handle or mattr transaction handle
 * \param[in,out]   ai_ Attribute array index
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_tc_to_queue_tc_map_get_next(bdmf_object_handle mo_, bdmf_index * ai_)
{
    return bdmf_attrelem_get_next(mo_, rdpa_tc_to_queue_attr_tc_map, (bdmf_index *)ai_);
}

/** @} end of tc_to_queue Doxygen group */




#endif /* _RDPA_AG_TC_TO_QUEUE_H_ */
