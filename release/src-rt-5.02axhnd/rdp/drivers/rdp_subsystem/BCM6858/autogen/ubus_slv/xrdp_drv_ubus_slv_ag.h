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

#ifndef _XRDP_DRV_UBUS_SLV_AG_H_
#define _XRDP_DRV_UBUS_SLV_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

bdmf_error_t ag_drv_ubus_slv_vpb_base_set(uint32_t base);
bdmf_error_t ag_drv_ubus_slv_vpb_base_get(uint32_t *base);
bdmf_error_t ag_drv_ubus_slv_vpb_mask_set(uint32_t mask);
bdmf_error_t ag_drv_ubus_slv_vpb_mask_get(uint32_t *mask);
bdmf_error_t ag_drv_ubus_slv_apb_base_set(uint32_t base);
bdmf_error_t ag_drv_ubus_slv_apb_base_get(uint32_t *base);
bdmf_error_t ag_drv_ubus_slv_apb_mask_set(uint32_t mask);
bdmf_error_t ag_drv_ubus_slv_apb_mask_get(uint32_t *mask);
bdmf_error_t ag_drv_ubus_slv_dqm_base_set(uint32_t base);
bdmf_error_t ag_drv_ubus_slv_dqm_base_get(uint32_t *base);
bdmf_error_t ag_drv_ubus_slv_dqm_mask_set(uint32_t mask);
bdmf_error_t ag_drv_ubus_slv_dqm_mask_get(uint32_t *mask);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_isr_set(uint32_t ist);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_isr_get(uint32_t *ist);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_itr_get(uint32_t *ist);
bdmf_error_t ag_drv_ubus_slv_profiling_cfg_set(bdmf_boolean counter_enable, bdmf_boolean profiling_start, bdmf_boolean manual_stop_mode, bdmf_boolean do_manual_stop);
bdmf_error_t ag_drv_ubus_slv_profiling_cfg_get(bdmf_boolean *counter_enable, bdmf_boolean *profiling_start, bdmf_boolean *manual_stop_mode, bdmf_boolean *do_manual_stop);
bdmf_error_t ag_drv_ubus_slv_profiling_status_get(bdmf_boolean *profiling_on, uint32_t *cycles_counter);
bdmf_error_t ag_drv_ubus_slv_profiling_counter_get(uint32_t *val);
bdmf_error_t ag_drv_ubus_slv_profiling_start_value_get(uint32_t *val);
bdmf_error_t ag_drv_ubus_slv_profiling_stop_value_get(uint32_t *val);
bdmf_error_t ag_drv_ubus_slv_profiling_cycle_num_set(uint32_t profiling_cycles_num);
bdmf_error_t ag_drv_ubus_slv_profiling_cycle_num_get(uint32_t *profiling_cycles_num);

#ifdef USE_BDMF_SHELL
enum
{
    cli_ubus_slv_vpb_base,
    cli_ubus_slv_vpb_mask,
    cli_ubus_slv_apb_base,
    cli_ubus_slv_apb_mask,
    cli_ubus_slv_dqm_base,
    cli_ubus_slv_dqm_mask,
    cli_ubus_slv_rnr_intr_ctrl_isr,
    cli_ubus_slv_rnr_intr_ctrl_ism,
    cli_ubus_slv_rnr_intr_ctrl_ier,
    cli_ubus_slv_rnr_intr_ctrl_itr,
    cli_ubus_slv_profiling_cfg,
    cli_ubus_slv_profiling_status,
    cli_ubus_slv_profiling_counter,
    cli_ubus_slv_profiling_start_value,
    cli_ubus_slv_profiling_stop_value,
    cli_ubus_slv_profiling_cycle_num,
};

int bcm_ubus_slv_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_ubus_slv_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

