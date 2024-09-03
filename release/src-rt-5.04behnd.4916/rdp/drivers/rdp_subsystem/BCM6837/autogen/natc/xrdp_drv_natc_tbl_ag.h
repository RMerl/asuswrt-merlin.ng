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


#ifndef _XRDP_DRV_NATC_TBL_AG_H_
#define _XRDP_DRV_NATC_TBL_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * key_lo: 
 * key_hi: 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_tbl_key_addr_set(uint8_t tbl_idx, uint32_t key_lo, uint8_t key_hi);
bdmf_error_t ag_drv_natc_tbl_key_addr_get(uint8_t tbl_idx, uint32_t *key_lo, uint8_t *key_hi);

/**********************************************************************************************************************
 * res_lo: 
 * res_hi: 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_tbl_res_addr_set(uint8_t tbl_idx, uint32_t res_lo, uint8_t res_hi);
bdmf_error_t ag_drv_natc_tbl_res_addr_get(uint8_t tbl_idx, uint32_t *res_lo, uint8_t *res_hi);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_tbl_key_addr,
    cli_natc_tbl_res_addr,
};

int bcm_natc_tbl_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_tbl_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
