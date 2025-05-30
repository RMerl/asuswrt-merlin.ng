/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

