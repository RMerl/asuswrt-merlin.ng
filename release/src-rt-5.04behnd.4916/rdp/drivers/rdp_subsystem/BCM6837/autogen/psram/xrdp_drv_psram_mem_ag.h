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


#ifndef _XRDP_DRV_PSRAM_MEM_AG_H_
#define _XRDP_DRV_PSRAM_MEM_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint32_t memory_data[32];
} psram_mem_memory_data;


/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_mem_memory_data_set(uint8_t psram_mem_id, uint32_t psram_enrty, const psram_mem_memory_data *memory_data);
bdmf_error_t ag_drv_psram_mem_memory_data_get(uint8_t psram_mem_id, uint32_t psram_enrty, psram_mem_memory_data *memory_data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_psram_mem_memory_data,
};

int bcm_psram_mem_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_psram_mem_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
