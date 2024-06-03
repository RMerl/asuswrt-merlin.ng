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


#ifndef _XRDP_DRV_UBUS_RESP_AG_H_
#define _XRDP_DRV_UBUS_RESP_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * ist: 
 *     ISR - 32bit RNR INT
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_isr_set(uint32_t ist);
bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_isr_get(uint32_t *ist);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_ism_get(uint32_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_ier_get(uint32_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_ubus_resp_rnr_intr_ctrl_itr_get(uint32_t *ist);

/**********************************************************************************************************************
 * counter_enable: 
 *     Enable free-running counter
 * profiling_start: 
 *     Start profiling window.
 * manual_stop_mode: 
 *     Enable manual stop mode
 * do_manual_stop: 
 *     Stop window now
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_profiling_cfg_set(bdmf_boolean counter_enable, bdmf_boolean profiling_start, bdmf_boolean manual_stop_mode, bdmf_boolean do_manual_stop);
bdmf_error_t ag_drv_ubus_resp_profiling_cfg_get(bdmf_boolean *counter_enable, bdmf_boolean *profiling_start, bdmf_boolean *manual_stop_mode, bdmf_boolean *do_manual_stop);

/**********************************************************************************************************************
 * profiling_on: 
 *     Profiling is currently on
 * cycles_counter: 
 *     Current value of profiling window cycles counter (bits [30:0]
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_profiling_status_get(bdmf_boolean *profiling_on, uint32_t *cycles_counter);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_profiling_counter_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_profiling_start_value_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_profiling_stop_value_get(uint32_t *val);

/**********************************************************************************************************************
 * profiling_cycles_num: 
 *     Length of profiling window in 500MHz clock cycles
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_profiling_cycle_num_set(uint32_t profiling_cycles_num);
bdmf_error_t ag_drv_ubus_resp_profiling_cycle_num_get(uint32_t *profiling_cycles_num);

/**********************************************************************************************************************
 * rchk_lock: 
 *     Range checker cfg lock bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_lock_set(bdmf_boolean rchk_lock);
bdmf_error_t ag_drv_ubus_resp_rchk_lock_get(bdmf_boolean *rchk_lock);

/**********************************************************************************************************************
 * wr_abort: 
 *     Abort write command if engine params match
 * rd_abort: 
 *     Abort read command if engine params match
 * prot: 
 *     PROT matching field
 * prot_msk: 
 *     PROT Mask field to be compared with PROT field
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_eng_ctrl_set(uint8_t idx, bdmf_boolean wr_abort, bdmf_boolean rd_abort, uint8_t prot, uint8_t prot_msk);
bdmf_error_t ag_drv_ubus_resp_rchk_eng_ctrl_get(uint8_t idx, bdmf_boolean *wr_abort, bdmf_boolean *rd_abort, uint8_t *prot, uint8_t *prot_msk);

/**********************************************************************************************************************
 * start_add: 
 *     Range checker engine start address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_eng_start_add_set(uint8_t idx, uint32_t start_add);
bdmf_error_t ag_drv_ubus_resp_rchk_eng_start_add_get(uint8_t idx, uint32_t *start_add);

/**********************************************************************************************************************
 * end_add: 
 *     Range checker engine end address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_eng_end_add_set(uint8_t idx, uint32_t end_add);
bdmf_error_t ag_drv_ubus_resp_rchk_eng_end_add_get(uint8_t idx, uint32_t *end_add);

/**********************************************************************************************************************
 * seclev_en: 
 *     Ranbge checker engine seclev enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_eng_seclev_en_set(uint8_t idx, uint32_t seclev_en);
bdmf_error_t ag_drv_ubus_resp_rchk_eng_seclev_en_get(uint8_t idx, uint32_t *seclev_en);

/**********************************************************************************************************************
 * rchk_rnr_en: 
 *     Range Checker Runner enable mapping
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_rnr_en_set(uint8_t idx, uint8_t rchk_rnr_en);
bdmf_error_t ag_drv_ubus_resp_rchk_rnr_en_get(uint8_t idx, uint8_t *rchk_rnr_en);

/**********************************************************************************************************************
 * rchk_abort_capt0: 
 *     Controls capture during abort:
 *     [31:31] - Abort indication
 *     [21:21] - Trans rwb
 *     [20:16] - runner accessed
 *     [15:13] - Trans prot
 *     [12:8]  - Trans seclev
 *     [7:0]   - Trans srcid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_abort_cpt0_get(uint32_t *rchk_abort_capt0);

/**********************************************************************************************************************
 * rchk_abort_capt1: 
 *     Address high bits [31:0] capture during abort
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_abort_cpt1_get(uint32_t *rchk_abort_capt1);

/**********************************************************************************************************************
 * rchk_abort_capt2: 
 *     Address low bits [39:32] capture during abort
 **********************************************************************************************************************/
bdmf_error_t ag_drv_ubus_resp_rchk_abort_cpt2_get(uint32_t *rchk_abort_capt2);

#ifdef USE_BDMF_SHELL
enum
{
    cli_ubus_resp_rnr_intr_ctrl_isr,
    cli_ubus_resp_rnr_intr_ctrl_ism,
    cli_ubus_resp_rnr_intr_ctrl_ier,
    cli_ubus_resp_rnr_intr_ctrl_itr,
    cli_ubus_resp_profiling_cfg,
    cli_ubus_resp_profiling_status,
    cli_ubus_resp_profiling_counter,
    cli_ubus_resp_profiling_start_value,
    cli_ubus_resp_profiling_stop_value,
    cli_ubus_resp_profiling_cycle_num,
    cli_ubus_resp_rchk_lock,
    cli_ubus_resp_rchk_eng_ctrl,
    cli_ubus_resp_rchk_eng_start_add,
    cli_ubus_resp_rchk_eng_end_add,
    cli_ubus_resp_rchk_eng_seclev_en,
    cli_ubus_resp_rchk_rnr_en,
    cli_ubus_resp_rchk_abort_cpt0,
    cli_ubus_resp_rchk_abort_cpt1,
    cli_ubus_resp_rchk_abort_cpt2,
};

int bcm_ubus_resp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_ubus_resp_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
