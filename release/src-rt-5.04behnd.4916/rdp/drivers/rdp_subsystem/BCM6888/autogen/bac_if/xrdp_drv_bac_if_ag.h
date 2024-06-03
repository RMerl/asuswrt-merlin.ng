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


#ifndef _XRDP_DRV_BAC_IF_AG_H_
#define _XRDP_DRV_BAC_IF_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} bac_if_bacif_block_bacif_configurations_clk_gate_cntrl;


/**********************************************************************************************************************
 * thr: 
 *     threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_set(uint8_t bacif_id, uint8_t thr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_get(uint8_t bacif_id, uint8_t *thr);

/**********************************************************************************************************************
 * en: 
 *     en override route address
 * id: 
 *     id to override route address
 * addr: 
 *     addr to override route address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_set(uint8_t bacif_id, bdmf_boolean en, uint8_t id, uint16_t addr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_get(uint8_t bacif_id, bdmf_boolean *en, uint8_t *id, uint16_t *addr);

/**********************************************************************************************************************
 * ba: 
 *     base_address (in 8B resolution).
 * bt: 
 *     first task the base address refers to.
 * ofst: 
 *     offset jump for each task (in 8B resolution).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_prgrm_m_prm_set(uint8_t bacif_id, uint16_t ba, uint8_t bt, uint16_t ofst);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_prgrm_m_prm_get(uint8_t bacif_id, uint16_t *ba, uint8_t *bt, uint16_t *ofst);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(uint8_t bacif_id, const bac_if_bacif_block_bacif_configurations_clk_gate_cntrl *bacif_block_bacif_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(uint8_t bacif_id, bac_if_bacif_block_bacif_configurations_clk_gate_cntrl *bacif_block_bacif_configurations_clk_gate_cntrl);

/**********************************************************************************************************************
 * entry: 
 *     lower 31b of entry
 * val: 
 *     valid bit of entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_ingfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);

/**********************************************************************************************************************
 * entry: 
 *     lower 31b of entry
 * val: 
 *     valid bit of entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_cmdfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);

/**********************************************************************************************************************
 * entry: 
 *     lower 31b of entry
 * val: 
 *     valid bit of entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_rsltfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);

/**********************************************************************************************************************
 * entry: 
 *     lower 31b of entry
 * val: 
 *     valid bit of entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_egfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);

/**********************************************************************************************************************
 * entry: 
 *     lower 31b of entry
 * val: 
 *     valid bit of entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_rpprmarr_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_of_c_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     value of cntr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c_get(uint8_t bacif_id, uint32_t *cntr);

/**********************************************************************************************************************
 * rd_clr: 
 *     read clear bit
 * wrap: 
 *     read clear bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_set(uint8_t bacif_id, bdmf_boolean rd_clr, bdmf_boolean wrap);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_get(uint8_t bacif_id, bdmf_boolean *rd_clr, bdmf_boolean *wrap);

/**********************************************************************************************************************
 * val: 
 *     value of debug reg
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_debug_dbg0_get(uint8_t bacif_id, uint32_t *val);

#ifdef USE_BDMF_SHELL
enum
{
    cli_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr,
    cli_bac_if_bacif_block_bacif_configurations_dec_rout_ovride,
    cli_bac_if_bacif_block_bacif_configurations_prgrm_m_prm,
    cli_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl,
    cli_bac_if_bacif_block_bacif_fifos_ingfifo,
    cli_bac_if_bacif_block_bacif_fifos_cmdfifo,
    cli_bac_if_bacif_block_bacif_fifos_rsltfifo,
    cli_bac_if_bacif_block_bacif_fifos_egfifo,
    cli_bac_if_bacif_block_bacif_fifos_rpprmarr,
    cli_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt,
    cli_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt,
    cli_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt,
    cli_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt,
    cli_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt,
    cli_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt,
    cli_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c,
    cli_bac_if_bacif_block_bacif_pm_counters_err_params_of_c,
    cli_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c,
    cli_bac_if_bacif_block_bacif_pm_counters_gen_cfg,
    cli_bac_if_bacif_block_bacif_debug_dbg0,
};

int bcm_bac_if_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bac_if_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
