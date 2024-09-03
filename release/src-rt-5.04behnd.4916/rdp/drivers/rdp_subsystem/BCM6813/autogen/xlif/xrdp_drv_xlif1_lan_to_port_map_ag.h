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

#ifndef _XRDP_DRV_XLIF1_LAN_TO_PORT_MAP_AG_H_
#define _XRDP_DRV_XLIF1_LAN_TO_PORT_MAP_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

bdmf_error_t ag_drv_xlif1_lan_to_port_map_port_map_set(uint8_t channel_id, uint8_t lan0, uint8_t lan1, uint8_t lan2, uint8_t lan3);
bdmf_error_t ag_drv_xlif1_lan_to_port_map_port_map_get(uint8_t channel_id, uint8_t *lan0, uint8_t *lan1, uint8_t *lan2, uint8_t *lan3);

#ifdef USE_BDMF_SHELL
enum
{
    cli_xlif1_lan_to_port_map_port_map,
};

int bcm_xlif1_lan_to_port_map_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_xlif1_lan_to_port_map_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

