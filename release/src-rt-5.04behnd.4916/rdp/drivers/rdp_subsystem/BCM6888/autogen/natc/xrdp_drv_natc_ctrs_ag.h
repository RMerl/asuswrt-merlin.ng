/*
   Copyright (c) 2015 Broadcom
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


#ifndef _XRDP_DRV_NATC_CTRS_AG_H_
#define _XRDP_DRV_NATC_CTRS_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint32_t cache_hit_count;
    uint32_t cache_miss_count;
    uint32_t ddr_request_count;
    uint32_t ddr_evict_count;
    uint32_t ddr_block_count;
} natc_ctrs_natc_ctrs;


/**********************************************************************************************************************
 * cache_hit_count: 
 *     32-bit total cache hit count value for statistics collection
 * cache_miss_count: 
 *     32-bit total cache miss count value for statistics collection
 * ddr_request_count: 
 *     32-bit total DDR request count value for statistics collection
 * ddr_evict_count: 
 *     32-bit total DDR evict count value for statistics collection.
 *     It does not include the flush command evict count.
 * ddr_block_count: 
 *     32-bit total DDR blocked access count value for statistics collection
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_ctrs_natc_ctrs_set(const natc_ctrs_natc_ctrs *natc_ctrs);
bdmf_error_t ag_drv_natc_ctrs_natc_ctrs_get(natc_ctrs_natc_ctrs *natc_ctrs);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_ctrs_natc_ctrs,
};

int bcm_natc_ctrs_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_ctrs_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
