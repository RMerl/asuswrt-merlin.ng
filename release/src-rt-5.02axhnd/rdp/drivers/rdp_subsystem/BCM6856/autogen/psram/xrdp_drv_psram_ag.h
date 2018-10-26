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

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* arb_comb_banks_val: value - value                                                              */
/* arb_comb4_val: value - value                                                                   */
/* arb_comb_val: value - value                                                                    */
/* arb_arb_val: value - value                                                                     */
/* arb_req_val: value - value                                                                     */
/**************************************************************************************************/
typedef struct
{
    uint32_t arb_comb_banks_val;
    uint32_t arb_comb4_val;
    uint32_t arb_comb_val;
    uint32_t arb_arb_val;
    uint32_t arb_req_val;
} psram_pm_counters_arb;


/**************************************************************************************************/
/* bwcen: pm_bw_check_en - start of new monitoring session. zeroes counters on rise.              */
/* cbwcen: cyclic_bw_check_en - if this enabled - when the bw period reaches its limit - the coun */
/*         ters are reset.                                                                        */
/* tw: time_window - measure time window in clock cycles                                          */
/* cl0men: cl0_measure_enable - enable monitor for client 0                                       */
/* cl1men: cl1_measure_enable - enable monitor for client 1                                       */
/* cl2men: cl2_measure_enable - enable monitor for client 2                                       */
/* cl3men: cl3_measure_enable - enable monitor for client 3                                       */
/* cl4men: cl4_measure_enable - enable monitor for client 4                                       */
/* cl5men: cl5_measure_enable - enable monitor for client 5                                       */
/* cl6men: cl6_measure_enable - enable monitor for client 6                                       */
/**************************************************************************************************/
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


/**************************************************************************************************/
/* perm_en: permutations_enable - 1: enable memory banks permutations0: disable                   */
/* comb_en: combinations_enable - 1: enable memory banks combinations0: disable                   */
/* comb_full: combinations_full - 1: enable full combinations(also on same 4-banks)0: disable ful */
/*            l combinations(allow only on opposite 4-banks)                                      */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean perm_en;
    bdmf_boolean comb_en;
    bdmf_boolean comb_full;
} psram_cfg_ctrl;


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
} psram_configurations_clk_gate_cntrl;


/**************************************************************************************************/
/* max: max_time - max wait time                                                                  */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_max_time[7];
} psram_pm_counters_max_time;


/**************************************************************************************************/
/* max: max_time - max wait time                                                                  */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_acc_time[7];
} psram_pm_counters_acc_time;


/**************************************************************************************************/
/* req: number_of_requests - accumulated number of served requests                                */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_acc_req[7];
} psram_pm_counters_acc_req;


/**************************************************************************************************/
/* time: accumulated_time - accumulated wait time                                                 */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_last_acc_time[7];
} psram_pm_counters_last_acc_time;


/**************************************************************************************************/
/* req: Number_of_requests - accumulated number of served requests                                */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_last_acc_req[7];
} psram_pm_counters_last_acc_req;


/**************************************************************************************************/
/* cnt: Double_word_count - Number of double words that were written to the DDR per client        */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_bw_wr_cnt[7];
} psram_pm_counters_bw_wr_cnt;


/**************************************************************************************************/
/* cnt: Double_word_count - Number of double words that were written to the DDR per client        */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_bw_rd_cnt[7];
} psram_pm_counters_bw_rd_cnt;


/**************************************************************************************************/
/* cnt: Double_word_count - Number of double words that were written to the DDR per client        */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_bw_wr_cnt_last[7];
} psram_pm_counters_bw_wr_cnt_last;


/**************************************************************************************************/
/* cnt: Double_word_count - Number of double words that were written to the DDR per client        */
/**************************************************************************************************/
typedef struct
{
    uint32_t pm_counters_bw_rd_cnt_last[7];
} psram_pm_counters_bw_rd_cnt_last;

bdmf_error_t ag_drv_psram_pm_counters_arb_get(psram_pm_counters_arb *pm_counters_arb);
bdmf_error_t ag_drv_psram_pm_counters_cnt_acc_get(uint32_t *last_acc_cnt_rd, uint32_t *last_acc_cnt_wr, uint32_t *acc_cnt_rd, uint32_t *acc_cnt_wr);
bdmf_error_t ag_drv_psram_pm_counters_muen_set(const psram_pm_counters_muen *pm_counters_muen);
bdmf_error_t ag_drv_psram_pm_counters_muen_get(psram_pm_counters_muen *pm_counters_muen);
bdmf_error_t ag_drv_psram_cfg_ctrl_set(const psram_cfg_ctrl *cfg_ctrl);
bdmf_error_t ag_drv_psram_cfg_ctrl_get(psram_cfg_ctrl *cfg_ctrl);
bdmf_error_t ag_drv_psram_configurations_clk_gate_cntrl_set(const psram_configurations_clk_gate_cntrl *configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_psram_configurations_clk_gate_cntrl_get(psram_configurations_clk_gate_cntrl *configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_psram_pm_counters_max_time_get(uint32_t zero, psram_pm_counters_max_time *pm_counters_max_time);
bdmf_error_t ag_drv_psram_pm_counters_acc_time_get(uint32_t zero, psram_pm_counters_acc_time *pm_counters_acc_time);
bdmf_error_t ag_drv_psram_pm_counters_acc_req_get(uint32_t zero, psram_pm_counters_acc_req *pm_counters_acc_req);
bdmf_error_t ag_drv_psram_pm_counters_last_acc_time_get(uint32_t zero, psram_pm_counters_last_acc_time *pm_counters_last_acc_time);
bdmf_error_t ag_drv_psram_pm_counters_last_acc_req_get(uint32_t zero, psram_pm_counters_last_acc_req *pm_counters_last_acc_req);
bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_get(uint32_t zero, psram_pm_counters_bw_wr_cnt *pm_counters_bw_wr_cnt);
bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_get(uint32_t zero, psram_pm_counters_bw_rd_cnt *pm_counters_bw_rd_cnt);
bdmf_error_t ag_drv_psram_pm_counters_bw_wr_cnt_last_get(uint32_t zero, psram_pm_counters_bw_wr_cnt_last *pm_counters_bw_wr_cnt_last);
bdmf_error_t ag_drv_psram_pm_counters_bw_rd_cnt_last_get(uint32_t zero, psram_pm_counters_bw_rd_cnt_last *pm_counters_bw_rd_cnt_last);

#ifdef USE_BDMF_SHELL
enum
{
    cli_psram_pm_counters_arb,
    cli_psram_pm_counters_cnt_acc,
    cli_psram_pm_counters_muen,
    cli_psram_cfg_ctrl,
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
bdmfmon_handle_t ag_drv_psram_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

