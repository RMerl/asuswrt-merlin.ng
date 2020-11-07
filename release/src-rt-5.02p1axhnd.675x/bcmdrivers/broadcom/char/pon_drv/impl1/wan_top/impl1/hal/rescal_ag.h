/*
   Copyright (c) 2015 Broadcom Corporation
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

#ifndef _BCM6858_RESCAL_AG_H_
#define _BCM6858_RESCAL_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif

/**************************************************************************************************/
/* wan_rescal_done:  - Connects to o_done.                                                        */
/* wan_rescal_pon:  - Connects to o_pon.                                                          */
/* wan_rescal_prev_comp_cnt:  - Connects to o_prev_comp_cnt.                                      */
/* wan_rescal_ctrl_dfs:  - Connects to o_rescal_ctrl_dfs.                                         */
/* wan_rescal_state:  - Connects to o_rescal_state.                                               */
/* wan_rescal_comp:  - Connects to o_rescalcomp.                                                  */
/* wan_rescal_valid:  - Connects to o_valid.                                                      */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean wan_rescal_done;
    uint8_t wan_rescal_pon;
    uint8_t wan_rescal_prev_comp_cnt;
    uint16_t wan_rescal_ctrl_dfs;
    uint8_t wan_rescal_state;
    bdmf_boolean wan_rescal_comp;
    bdmf_boolean wan_rescal_valid;
} rescal_status_0;

bdmf_error_t ag_drv_rescal_cfg_set(bdmf_boolean cfg_wan_rescal_rstb, bdmf_boolean cfg_wan_rescal_diag_on, bdmf_boolean cfg_wan_rescal_pwrdn, uint16_t cfg_wan_rescal_ctrl);
bdmf_error_t ag_drv_rescal_cfg_get(bdmf_boolean *cfg_wan_rescal_rstb, bdmf_boolean *cfg_wan_rescal_diag_on, bdmf_boolean *cfg_wan_rescal_pwrdn, uint16_t *cfg_wan_rescal_ctrl);
bdmf_error_t ag_drv_rescal_status_0_get(rescal_status_0 *status_0);
bdmf_error_t ag_drv_rescal_status_1_get(uint8_t *wan_rescal_curr_comp_cnt);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_rescal_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

