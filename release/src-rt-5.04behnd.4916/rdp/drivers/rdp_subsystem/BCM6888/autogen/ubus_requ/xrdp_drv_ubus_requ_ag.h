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


#ifndef _XRDP_DRV_UBUS_REQU_AG_H_
#define _XRDP_DRV_UBUS_REQU_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * en: 
 *     bridge enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_en_set(uint8_t ubus_requ_id, bdmf_boolean en);
bdmf_error_t ag_drv_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_en_get(uint8_t ubus_requ_id, bdmf_boolean *en);

/**********************************************************************************************************************
 * cmd_space: 
 *     command space indication that controls the ARdy signal.
 *     
 *     Once the HSPACE indication is lower than CMD_SPACE the ARdy will be deasserted
 * data_space: 
 *     data space indication that controls the ARdy signal.
 *     
 *     Once the DSPACE indication is lower than DATA_SPACE the ARdy will be deasserted
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_hyst_ctrl_set(uint8_t ubus_requ_id, uint16_t cmd_space, uint16_t data_space);
bdmf_error_t ag_drv_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_hyst_ctrl_get(uint8_t ubus_requ_id, uint16_t *cmd_space, uint16_t *data_space);

/**********************************************************************************************************************
 * hp_en: 
 *     enables the hp mechanism
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_hp_set(uint8_t ubus_requ_id, bdmf_boolean hp_en);
bdmf_error_t ag_drv_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_hp_get(uint8_t ubus_requ_id, bdmf_boolean *hp_en);

#ifdef USE_BDMF_SHELL
enum
{
    cli_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_en,
    cli_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_hyst_ctrl,
    cli_ubus_requ_xrdp_ubus_requester_xrdp_ubus_rqstr_hp,
};

int bcm_ubus_requ_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_ubus_requ_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
