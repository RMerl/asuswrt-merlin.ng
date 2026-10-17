/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
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

#ifndef SRDS_API_TYPES_H
#define SRDS_API_TYPES_H

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
