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
 * pbit_to_queue object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_PBIT_TO_QUEUE_H_
#define _RDPA_AG_PBIT_TO_QUEUE_H_

/** \addtogroup pbit_to_queue
 * @{
 */


/** Get pbit_to_queue type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a pbit_to_queue object.
 * \return pbit_to_queue type handle
 */
bdmf_type_handle rdpa_pbit_to_queue_drv(void);

/* pbit_to_queue: Attribute types */
typedef enum {
    rdpa_pbit_to_queue_attr_table = 0, /* table : KRI : number : Table index */
    rdpa_pbit_to_queue_attr_pbit_map = 1, /* pbit_map : RWF : number[] : Priority array */
} rdpa_pbit_to_queue_attr_types;

extern int (*f_rdpa_pbit_to_queue_get)(bdmf_number table_, bdmf_object_handle *pmo);

/** Get pbit_to_queue object by key.

 * This function returns pbit_to_queue object instance by key.
 * \param[in] table_    Object key
 * \param[out] pbit_to_queue_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_pbit_to_queue_get(bdmf_number table_, bdmf_object_handle *pbit_to_queue_obj);

/** Get pbit_to_queue/table attribute.
 *
 * Get Table index.
 * \param[in]   mo_ pbit_to_queue object handle or mattr transaction handle
 * \param[out]  table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_pbit_to_queue_table_get(bdmf_object_handle mo_, bdmf_number *table_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attr_get_as_num(mo_, rdpa_pbit_to_queue_attr_table, &_nn_);
    *table_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set pbit_to_queue/table attribute.
 *
 * Set Table index.
 * \param[in]   mo_ pbit_to_queue object handle or mattr transaction handle
 * \param[in]   table_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_pbit_to_queue_table_set(bdmf_object_handle mo_, bdmf_number table_)
{
    return bdmf_attr_set_as_num(mo_, rdpa_pbit_to_queue_attr_table, table_);
}


/** Get pbit_to_queue/pbit_map attribute entry.
 *
 * Get Priority array.
 * \param[in]   mo_ pbit_to_queue object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  pbit_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_pbit_to_queue_pbit_map_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number *pbit_map_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_pbit_to_queue_attr_pbit_map, (bdmf_index)ai_, &_nn_);
    *pbit_map_ = (bdmf_number)_nn_;
    return _rc_;
}


/** Set pbit_to_queue/pbit_map attribute entry.
 *
 * Set Priority array.
 * \param[in]   mo_ pbit_to_queue object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   pbit_map_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_pbit_to_queue_pbit_map_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_number pbit_map_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_pbit_to_queue_attr_pbit_map, (bdmf_index)ai_, pbit_map_);
}

/** @} end of pbit_to_queue Doxygen group */




#endif /* _RDPA_AG_PBIT_TO_QUEUE_H_ */
