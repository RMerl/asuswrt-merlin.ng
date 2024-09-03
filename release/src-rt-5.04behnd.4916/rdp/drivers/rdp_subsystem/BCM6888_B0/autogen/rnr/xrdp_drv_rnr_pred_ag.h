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


#ifndef _XRDP_DRV_RNR_PRED_AG_H_
#define _XRDP_DRV_RNR_PRED_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * pred_mem: 
 *     MEM_PRED_MAIN
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_pred_mem_entry_set(uint8_t rnr_id, uint32_t word_index, uint16_t pred_mem);
bdmf_error_t ag_drv_rnr_pred_mem_entry_get(uint8_t rnr_id, uint32_t word_index, uint16_t *pred_mem);

#ifdef USE_BDMF_SHELL
enum
{
    cli_rnr_pred_mem_entry,
};

int bcm_rnr_pred_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_rnr_pred_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
