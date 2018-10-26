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

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* bypass_clk_gate: BYPASS_CLOCK_GATE - If set to 1b1 will disable the clock gate logic such to a */
/*                  lways enable the clock                                                        */
/* timer_val: TIMER_VALUE - For how long should the clock stay active once all conditions for clo */
/*            ck disable are met.                                                                 */
/* keep_alive_en: KEEP_ALIVE_ENABLE - Enables the keep alive logic which will periodically enable */
/*                 the clock to assure that no deadlock of clock being removed completely will oc */
/*                cur                                                                             */
/* keep_alive_intrvl: KEEP_ALIVE_INTERVAL - If the KEEP alive option is enabled the field will de */
/*                    termine for how many cycles should the clock be active                      */
/* keep_alive_cyc: KEEP_ALIVE_CYCLE - If the KEEP alive option is enabled this field will determi */
/*                 ne for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTE */
/*                 RVAL)So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} bac_if_bacif_block_bacif_configurations_clk_gate_cntrl;

bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_set(uint8_t bacif_id, uint8_t thr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr_get(uint8_t bacif_id, uint8_t *thr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_set(uint8_t bacif_id, bdmf_boolean en, uint8_t id, uint16_t addr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_dec_rout_ovride_get(uint8_t bacif_id, bdmf_boolean *en, uint8_t *id, uint16_t *addr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(uint8_t bacif_id, const bac_if_bacif_block_bacif_configurations_clk_gate_cntrl *bacif_block_bacif_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(uint8_t bacif_id, bac_if_bacif_block_bacif_configurations_clk_gate_cntrl *bacif_block_bacif_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_ingfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_cmdfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_rsltfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_egfifo_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_fifos_rpprmarr_get(uint8_t bacif_id, uint8_t fifo_idx, uint32_t *entry, bdmf_boolean *val);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_ing_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_cmd_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_cmd_cnt_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_eng_rslt_cnt_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_rslt_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_egr_f_cnt_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_cmdlng_c_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_of_c_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_err_params_uf_c_get(uint8_t bacif_id, uint32_t *cntr);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_set(uint8_t bacif_id, bdmf_boolean rd_clr, bdmf_boolean wrap);
bdmf_error_t ag_drv_bac_if_bacif_block_bacif_pm_counters_gen_cfg_get(uint8_t bacif_id, bdmf_boolean *rd_clr, bdmf_boolean *wrap);

#ifdef USE_BDMF_SHELL
enum
{
    cli_bac_if_bacif_block_bacif_configurations_rslt_f_full_thr,
    cli_bac_if_bacif_block_bacif_configurations_dec_rout_ovride,
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
};

int bcm_bac_if_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bac_if_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

