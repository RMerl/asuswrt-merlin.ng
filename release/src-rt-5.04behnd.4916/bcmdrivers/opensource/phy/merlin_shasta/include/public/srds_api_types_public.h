/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

/***********************************************************************************
 ***********************************************************************************
 *                                                                                 *
 *  Revision      :   *
 *                                                                                 *
 *  Description   :  Common types used by Serdes API functions                     *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/

/** @file srds_api_types.h
 * Common types used by Serdes API functions
 * This file contains types which are common to all serdes cores
 */

#ifndef SRDS_API_TYPES_PUBLIC_H
#define SRDS_API_TYPES_PUBLIC_H

/* Opaque SerDes Access Struct to be defined by Upper level Software */
struct srds_access_s;
typedef struct srds_access_s srds_access_t;

/* Opaque SerDes Mutex Struct to be defined by Upper level Software */
struct srds_mutex_s;
typedef struct srds_mutex_s srdes_mutex_t;

typedef struct {
    uint32_t signature;
    uint32_t diag_mem_ram_base;
    uint32_t diag_mem_ram_size;
    uint32_t core_var_ram_base; /* Must be the same as CORE_VAR_RAM_BASE */
    uint32_t core_var_ram_size;
    uint32_t lane_var_ram_base; /* Must be the same as LANE_VAR_RAM_BASE */
    uint32_t lane_var_ram_size; /* Must be the same as LANE_VAR_RAM_SIZE */
    uint32_t trace_mem_ram_base;
    uint32_t trace_mem_ram_size;
    uint32_t micro_var_ram_base;
    uint8_t  lane_count; /* lanes per micro */
    uint8_t  trace_memory_descending_writes;
    uint8_t  micro_count;
    uint8_t  micro_var_ram_size;
    uint16_t grp_ram_size;
    uint32_t ucode_version;
} srds_info_t;
#endif
