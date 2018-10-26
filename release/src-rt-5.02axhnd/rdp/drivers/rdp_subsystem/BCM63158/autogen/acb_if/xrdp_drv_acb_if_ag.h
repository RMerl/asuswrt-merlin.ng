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

#ifndef _XRDP_DRV_ACB_IF_AG_H_
#define _XRDP_DRV_ACB_IF_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

bdmf_error_t ag_drv_acb_if_conf_set(bdmf_boolean crc_add, uint8_t val_loc);
bdmf_error_t ag_drv_acb_if_conf_get(bdmf_boolean *crc_add, uint8_t *val_loc);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_type_get(uint32_t idx, uint32_t *val);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_imp_get(uint32_t idx, uint32_t *val);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_agg_get(uint32_t idx, uint32_t *val);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_buffs_get(uint32_t idx, uint32_t *val);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_set(bdmf_boolean rd_clr, bdmf_boolean wrap);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_get(bdmf_boolean *rd_clr, bdmf_boolean *wrap);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_set(uint8_t vs);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_get(uint8_t *vs);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_dbgbus_get(uint32_t *val);
bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_stat_get(uint32_t idx, uint32_t *val);

#ifdef USE_BDMF_SHELL
enum
{
    cli_acb_if_conf,
    cli_acb_if_acbif_block_acbif_pm_counters_cmd_type,
    cli_acb_if_acbif_block_acbif_pm_counters_cmd_imp,
    cli_acb_if_acbif_block_acbif_pm_counters_agg,
    cli_acb_if_acbif_block_acbif_pm_counters_buffs,
    cli_acb_if_acbif_block_acbif_pm_counters_gen_cfg,
    cli_acb_if_acbif_block_acbif_debug_dbgsel,
    cli_acb_if_acbif_block_acbif_debug_dbgbus,
    cli_acb_if_acbif_block_acbif_debug_stat,
};

int bcm_acb_if_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_acb_if_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

