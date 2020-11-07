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

#ifndef _WAN_PRBS_AG_H_
#define _WAN_PRBS_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    uint8_t en_timer_mode;
    uint8_t en_timeout;
    uint8_t mode_sel;
    bdmf_boolean err_cnt_burst_mode;
    uint8_t lock_cnt;
    uint8_t ool_cnt;
    bdmf_boolean inv;
    bdmf_boolean sig_prbs_status_clr;
} wan_prbs_wan_prbs_chk_ctrl_0;

bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_0_set(const wan_prbs_wan_prbs_chk_ctrl_0 *wan_prbs_chk_ctrl_0);
bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_0_get(wan_prbs_wan_prbs_chk_ctrl_0 *wan_prbs_chk_ctrl_0);
bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_1_set(bdmf_boolean prbs_chk_en, uint8_t prbs_chk_mode, uint32_t prbs_timer_val);
bdmf_error_t ag_drv_wan_prbs_wan_prbs_chk_ctrl_1_get(bdmf_boolean *prbs_chk_en, uint8_t *prbs_chk_mode, uint32_t *prbs_timer_val);
bdmf_error_t ag_drv_wan_prbs_wan_prbs_status_0_get(bdmf_boolean *lock_lost_lh, uint32_t *err_cnt);
bdmf_error_t ag_drv_wan_prbs_wan_prbs_status_1_get(bdmf_boolean *any_err, bdmf_boolean *lock);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_wan_prbs_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

