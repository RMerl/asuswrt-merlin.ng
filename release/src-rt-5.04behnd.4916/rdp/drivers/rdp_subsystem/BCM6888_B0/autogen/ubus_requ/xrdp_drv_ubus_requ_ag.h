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
