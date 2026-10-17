// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*************************************************************************************
 *************************************************************************************
 *                                                                                   *
 *  Revision      :         *
 *                                                                                   *
 *  Description   :  Common types used by Serdes API functions                       *
 *                                                                                   *
 *************************************************************************************
 *************************************************************************************/

#ifndef SRDS_API_TYPES_PUBLIC_H
#define SRDS_API_TYPES_PUBLIC_H

/*! @file
 *  @brief Common types used by Serdes API functions which are common to all cores
 */

/*! @addtogroup APITag
 * @{
 */

/*! @defgroup SrdsAPITypesTag Common Serdes Types */

/*! @addtogroup SrdsAPITypesTag
 * @{
 */

/* Opaque SerDes Access Struct to be defined by Upper level Software */
struct srds_access_s;
typedef struct srds_access_s srds_access_t;

/* Opaque SerDes Mutex Struct to be defined by Upper level Software */
struct srds_mutex_s;
typedef struct srds_mutex_s srdes_mutex_t;

/*------------------------------*/
/** Serdes Info Table Structure */
/*------------------------------*/
typedef struct {
    uint32_t signature;
    uint32_t diag_mem_ram_base;
    uint32_t diag_mem_ram_size;
    uint32_t core_var_ram_base;
    uint32_t core_var_ram_size;
    uint32_t lane_var_ram_base;
    uint32_t lane_var_ram_size;
    uint32_t lane_static_var_ram_base;
    uint32_t lane_static_var_ram_size;
    uint32_t trace_mem_ram_base;
    uint32_t trace_mem_ram_size;
    uint32_t micro_var_ram_base;
    uint32_t hw_exception_mem_ram_base;
    uint32_t ucode_version;
    uint16_t grp_ram_size;
    uint8_t  lane_count;
    uint8_t  trace_memory_descending_writes;
    uint8_t  trace_memory_concurrent_read;
    uint8_t  micro_count;
    uint8_t  micro_var_ram_size;
    uint8_t  info_table_version;
    uint8_t  silicon_version; /* Only used for UAPI variants */
    uint8_t  is_big_endian;
} srds_info_t;

/*! @} SrdsAPITypesTag */
/*! @} APITag */
#endif
