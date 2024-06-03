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


#ifndef _XRDP_DRV_PSRAM_AG_H_
#define _XRDP_DRV_PSRAM_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint32_t arb_comb_banks_val;
    uint32_t arb_comb4_val;
    uint32_t arb_comb_val;
    uint32_t arb_arb_val;
    uint32_t arb_req_val;
} psram_pm_counters_arb;

typedef struct
{
    bdmf_boolean bwcen;
    bdmf_boolean cbwcen;
    uint32_t tw;
    bdmf_boolean cl0men;
    bdmf_boolean cl1men;
    bdmf_boolean cl2men;
    bdmf_boolean cl3men;
    bdmf_boolean cl4men;
    bdmf_boolean cl5men;
    bdmf_boolean cl6men;
} psram_pm_counters_muen;

typedef struct
{
    bdmf_boolean perm_en;
    bdmf_boolean comb_en;
    bdmf_boolean comb_full;
} psram_cfg_ctrl;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} psram_configurations_clk_gate_cntrl;

typedef struct
{
    uint32_t pm_counters_max_time[7];
} psram_pm_counters_max_time;

typedef struct
{
    uint32_t pm_counters_acc_time[7];
} psram_pm_counters_acc_time;

typedef struct
{
    uint32_t pm_counters_acc_req[7];
} psram_pm_counters_acc_req;

typedef struct
{
    uint32_t pm_counters_last_acc_time[7];
} psram_pm_counters_last_acc_time;

typedef struct
{
    uint32_t pm_counters_last_acc_req[7];
} psram_pm_counters_last_acc_req;

typedef struct
{
    uint32_t pm_counters_bw_wr_cnt[7];
} psram_pm_counters_bw_wr_cnt;

typedef struct
{
    uint32_t pm_counters_bw_rd_cnt[7];
} psram_pm_counters_bw_rd_cnt;

typedef struct
{
    uint32_t pm_counters_bw_wr_cnt_last[7];
} psram_pm_counters_bw_wr_cnt_last;

typedef struct
{
    uint32_t pm_counters_bw_rd_cnt_last[7];
} psram_pm_counters_bw_rd_cnt_last;


/**********************************************************************************************************************
 * arb_comb_banks_val: 
 *     value
 * arb_comb4_val: 
 *     value
 * arb_comb_val: 
 *     value
 * arb_arb_val: 
 *     value
 * arb_req_val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_arb_get(psram_pm_counters_arb *pm_counters_arb);

/**********************************************************************************************************************
 * last_acc_cnt_rd: 
 *     Number of double words that were written to the DDR per client
 * last_acc_cnt_wr: 
 *     Number of double words that were written to the DDR per client
 * acc_cnt_rd: 
 *     Number of double words that were written to the DDR per client
 * acc_cnt_wr: 
 *     Number of double words that were written to the DDR per client
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_cnt_acc_get(uint32_t *last_acc_cnt_rd, uint32_t *last_acc_cnt_wr, uint32_t *acc_cnt_rd, uint32_t *acc_cnt_wr);

/**********************************************************************************************************************
 * bwcen: 
 *     start of new monitoring session. zeroes counters on rise.
 * cbwcen: 
 *     if this enabled - when the bw period reaches its limit - the counters are reset.
 * tw: 
 *     measure time window in clock cycles
 * cl0men: 
 *     enable monitor for client 0
 *     
 * cl1men: 
 *     enable monitor for client 1
 *     
 * cl2men: 
 *     enable monitor for client 2
 *     
 * cl3men: 
 *     enable monitor for client 3
 *     
 * cl4men: 
 *     enable monitor for client 4
 *     
 * cl5men: 
 *     enable monitor for client 5
 * cl6men: 
 *     enable monitor for client 6
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_muen_set(const psram_pm_counters_muen *pm_counters_muen);
bdmf_error_t ag_drv_psram_pm_counters_muen_get(psram_pm_counters_muen *pm_counters_muen);

/**********************************************************************************************************************
 * perm_en: 
 *     1: enable memory banks permutations
 *     0: disable
 * comb_en: 
 *     1: enable memory banks combinations
 *     0: disable
 * comb_full: 
 *     1: enable full combinations(also on same 4-banks)
 *     0: disable full combinations(allow only on opposite 4-banks)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_cfg_ctrl_set(const psram_cfg_ctrl *cfg_ctrl);
bdmf_error_t ag_drv_psram_cfg_ctrl_get(psram_cfg_ctrl *cfg_ctrl);

/**********************************************************************************************************************
 * val: 
 *     programmable seed
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_configurations_scrm_seed_set(uint32_t val);
bdmf_error_t ag_drv_psram_configurations_scrm_seed_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     programmable seed
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_configurations_scrm_addr_set(uint32_t val);
bdmf_error_t ag_drv_psram_configurations_scrm_addr_get(uint32_t *val);

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
bdmf_error_t ag_drv_psram_configurations_clk_gate_cntrl_set(const psram_configurations_clk_gate_cntrl *configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_psram_configurations_clk_gate_cntrl_get(psram_configurations_clk_gate_cntrl *configurations_clk_gate_cntrl);

/**********************************************************************************************************************
 * max: 
 *     max wait time
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_max_time_get(uint32_t zero, psram_pm_counters_max_time *pm_counters_max_time);

/**********************************************************************************************************************
 * max: 
 *     max wait time
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_acc_time_get(uint32_t zero, psram_pm_counters_acc_time *pm_counters_acc_time);

/**********************************************************************************************************************
 * req: 
 *     accumulated number of served requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_acc_req_get(uint32_t zero, psram_pm_counters_acc_req *pm_counters_acc_req);

/**********************************************************************************************************************
 * time: 
 *     accumulated wait time
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_last_acc_time_get(uint32_t zero, psram_pm_counters_last_acc_time *pm_counters_last_acc_time);

/**********************************************************************************************************************
 * req: 
 *     accumulated number of served requests
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_last_acc_req_get(uint32_t zero, psram_pm_counters_last_acc_req *pm_counters_last_acc_req);

/**********************************************************************************************************************
 * cnt: 
 *     Number of double words that were written to the DDR per client
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_get(uint32_t zero, psram_pm_counters_bw_wr_cnt *pm_counters_bw_wr_cnt);

/**********************************************************************************************************************
 * cnt: 
 *     Number of double words that were written to the DDR per client
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_get(uint32_t zero, psram_pm_counters_bw_rd_cnt *pm_counters_bw_rd_cnt);

/**********************************************************************************************************************
 * cnt: 
 *     Number of double words that were written to the DDR per client
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_last_get(uint32_t zero, psram_pm_counters_bw_wr_cnt_last *pm_counters_bw_wr_cnt_last);

/**********************************************************************************************************************
 * cnt: 
 *     Number of double words that were written to the DDR per client
 **********************************************************************************************************************/
bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_last_get(uint32_t zero, psram_pm_counters_bw_rd_cnt_last *pm_counters_bw_rd_cnt_last);

#ifdef USE_BDMF_SHELL
enum
{
    cli_psram_pm_counters_arb,
    cli_psram_pm_counters_cnt_acc,
    cli_psram_pm_counters_muen,
    cli_psram_cfg_ctrl,
    cli_psram_configurations_scrm_seed,
    cli_psram_configurations_scrm_addr,
    cli_psram_configurations_clk_gate_cntrl,
    cli_psram_pm_counters_max_time,
    cli_psram_pm_counters_acc_time,
    cli_psram_pm_counters_acc_req,
    cli_psram_pm_counters_last_acc_time,
    cli_psram_pm_counters_last_acc_req,
    cli_psram_pm_counters_bw_wr_cnt,
    cli_psram_pm_counters_bw_rd_cnt,
    cli_psram_pm_counters_bw_wr_cnt_last,
    cli_psram_pm_counters_bw_rd_cnt_last,
};

int bcm_psram_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_psram_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
