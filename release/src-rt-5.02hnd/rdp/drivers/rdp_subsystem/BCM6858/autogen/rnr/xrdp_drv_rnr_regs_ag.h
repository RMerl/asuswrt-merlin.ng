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

#ifndef _XRDP_DRV_RNR_REGS_AG_H_
#define _XRDP_DRV_RNR_REGS_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* trace_write_pnt: Trace_write_pointer - Trace write pointer                                     */
/* idle_no_active_task: IDLE_no_active_task - No active task                                      */
/* curr_thread_num: CURR_thread_num - Current thread num                                          */
/* profiling_active: Profiling_active - Status of profiling ON/OFF                                */
/* trace_fifo_overrun: TRACE_FIFO_OVERRUN - Sticky bit, indicating trace event FIFO overrun. Clea */
/*                     red by writing bit [31] of PROFILING_CFG_1 register                        */
/**************************************************************************************************/
typedef struct
{
    uint16_t trace_write_pnt;
    bdmf_boolean idle_no_active_task;
    uint8_t curr_thread_num;
    bdmf_boolean profiling_active;
    bdmf_boolean trace_fifo_overrun;
} rnr_regs_profiling_sts;


/**************************************************************************************************/
/* trace_wraparound: TRACE_WRAPAROUND - Wraparound when writing trace buffer                      */
/* trace_mode: TRACE_MODE - Select all tasks or single task mode                                  */
/* trace_disable_idle_in: TRACE_DISABLE_IDLE_IN - Select whether to log IDLE in context swap even */
/*                        ts                                                                      */
/* trace_disable_wakeup_log: TRACE_DISABLE_WAKEUP_LOG - Enable/disable logging of scheduler event */
/*                           s (wakeups). Relevant only for single task mode                      */
/* trace_task: TRACE_TASK - Select task for single task operation                                 */
/* idle_counter_source_sel: IDLE_COUNTER_SOURCE_SEL - Select mode for IDLE counter                */
/* trace_reset_event_fifo: TRACE_RESET_EVENT_FIFO - Apply software reset to event FIFO            */
/* trace_clear_fifo_overrun: TRACE_CLEAR_FIFO_OVERRUN - Write 1 to clear event FIFO overrun stick */
/*                           y bit                                                                */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean trace_wraparound;
    bdmf_boolean trace_mode;
    bdmf_boolean trace_disable_idle_in;
    bdmf_boolean trace_disable_wakeup_log;
    uint8_t trace_task;
    bdmf_boolean idle_counter_source_sel;
    bdmf_boolean trace_reset_event_fifo;
    bdmf_boolean trace_clear_fifo_overrun;
} rnr_regs_trace_config;


/**************************************************************************************************/
/* ld_stall_cnt: LD_STALL_CNT - Count load stalls                                                 */
/* acc_stall_cnt: ACC_STALL_CNT - Count accelerator stalls                                        */
/* ldio_stall_cnt: LDIO_STALL_CNT - Count load io stalls                                          */
/* store_stall_cnt: STORE_STALL_CNT - Count store stalls                                          */
/* idle_cnt: IDLE - IDLE countReserved                                                            */
/* jmp_taken_predicted_untaken_cnt: UNTAKEN_JMP_CNT - Counts jumps with prediction miss, when pre */
/*                                  diction was dont jump                                         */
/* jmp_untaken_predicted_taken_cnt: TAKEN_JMP_CNT - Counts jumps with prediction miss, when predi */
/*                                  ction was jump                                                */
/**************************************************************************************************/
typedef struct
{
    uint16_t ld_stall_cnt;
    uint16_t acc_stall_cnt;
    uint16_t ldio_stall_cnt;
    uint16_t store_stall_cnt;
    uint32_t idle_cnt;
    uint16_t jmp_taken_predicted_untaken_cnt;
    uint16_t jmp_untaken_predicted_taken_cnt;
} rnr_regs_rnr_core_cntrs;


/**************************************************************************************************/
/* bkpt_0_en: BKPT_0_EN - Enable breakpoint 0                                                     */
/* bkpt_0_use_thread: BKPT_0_USE_THREAD - Enable breakpoint for given thread only                 */
/* bkpt_1_en: BKPT_1_EN - Enable breakpoint 1                                                     */
/* bkpt_1_use_thread: BKPT_1_USE_THREAD - Enable breakpoint for given thread only                 */
/* bkpt_2_en: BKPT_2_EN - Enable breakpoint 2                                                     */
/* bkpt_2_use_thread: BKPT_2_USE_THREAD - Enable breakpoint for given thread only                 */
/* bkpt_3_en: BKPT_3_EN - Enable breakpoint 3                                                     */
/* bkpt_3_use_thread: BKPT_3_USE_THREAD - Enable breakpoint for given thread only                 */
/* bkpt_4_en: BKPT_4_EN - Enable breakpoint 4                                                     */
/* bkpt_4_use_thread: BKPT_4_USE_THREAD - Enable breakpoint for given thread only                 */
/* bkpt_5_en: BKPT_5_EN - Enable breakpoint 5                                                     */
/* bkpt_5_use_thread: BKPT_5_USE_THREAD - Enable breakpoint for given thread only                 */
/* bkpt_6_en: BKPT_6_EN - Enable breakpoint 6                                                     */
/* bkpt_6_use_thread: BKPT_6_USE_THREAD - Enable breakpoint for given thread only                 */
/* bkpt_7_en: BKPT_7_EN - Enable breakpoint 7                                                     */
/* bkpt_7_use_thread: BKPT_7_USE_THREAD - Enable breakpoint for given thread only                 */
/* step_mode: STEP_MODE - Configure step mode                                                     */
/* new_flags_val: NEW_FLAGS_VAL - Value for new flags                                             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bkpt_0_en;
    bdmf_boolean bkpt_0_use_thread;
    bdmf_boolean bkpt_1_en;
    bdmf_boolean bkpt_1_use_thread;
    bdmf_boolean bkpt_2_en;
    bdmf_boolean bkpt_2_use_thread;
    bdmf_boolean bkpt_3_en;
    bdmf_boolean bkpt_3_use_thread;
    bdmf_boolean bkpt_4_en;
    bdmf_boolean bkpt_4_use_thread;
    bdmf_boolean bkpt_5_en;
    bdmf_boolean bkpt_5_use_thread;
    bdmf_boolean bkpt_6_en;
    bdmf_boolean bkpt_6_use_thread;
    bdmf_boolean bkpt_7_en;
    bdmf_boolean bkpt_7_use_thread;
    bdmf_boolean step_mode;
    uint8_t new_flags_val;
} rnr_regs_cfg_bkpt_cfg;


/**************************************************************************************************/
/* dbg_sel: Debug_bus_select - Control bits for the debug design output.                          */
/* main_dis_per_sched: Main_Disable_Periodic_Scheduling - Disables the scheduler to upgrade perio */
/*                     dically it's selection.When this bit is set the scheduler can change it's  */
/*                     thread selection only if a context  switch occurs.For debug only           */
/* pico_dis_per_sched: Pico_Disable_Periodic_Scheduling - Disables the scheduler to upgrade perio */
/*                     dically it's selection.When this bit is set the scheduler can change it's  */
/*                     thread selection only if a context  switch occurs.For debug only           */
/* main_fw_self_is_sync: main_fw_self_is_sync - fw self wakeup is sync                            */
/* pico_fw_self_is_sync: pico_fw_self_is_sync - fw self wakeup is sync                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t dbg_sel;
    bdmf_boolean main_dis_per_sched;
    bdmf_boolean pico_dis_per_sched;
    bdmf_boolean main_fw_self_is_sync;
    bdmf_boolean pico_fw_self_is_sync;
} rnr_regs_dbg_design_dbg_ctrl;

bdmf_error_t ag_drv_rnr_regs_rnr_enable_set(uint8_t rnr_id, bdmf_boolean en);
bdmf_error_t ag_drv_rnr_regs_rnr_enable_get(uint8_t rnr_id, bdmf_boolean *en);
bdmf_error_t ag_drv_rnr_regs_dma_illegal_set(uint8_t rnr_id, bdmf_boolean dma_illegal_status);
bdmf_error_t ag_drv_rnr_regs_dma_illegal_get(uint8_t rnr_id, bdmf_boolean *dma_illegal_status);
bdmf_error_t ag_drv_rnr_regs_rnr_freq_set(uint8_t rnr_id, uint16_t micro_sec_val);
bdmf_error_t ag_drv_rnr_regs_rnr_freq_get(uint8_t rnr_id, uint16_t *micro_sec_val);
bdmf_error_t ag_drv_rnr_regs_cam_stop_val_set(uint8_t rnr_id, uint16_t stop_value);
bdmf_error_t ag_drv_rnr_regs_cam_stop_val_get(uint8_t rnr_id, uint16_t *stop_value);
bdmf_error_t ag_drv_rnr_regs_profiling_sts_get(uint8_t rnr_id, rnr_regs_profiling_sts *profiling_sts);
bdmf_error_t ag_drv_rnr_regs_is_trace_fifo_overrun_get(uint8_t rnr_id, bdmf_boolean *trace_fifo_overrun);
bdmf_error_t ag_drv_rnr_regs_trace_config_set(uint8_t rnr_id, const rnr_regs_trace_config *trace_config);
bdmf_error_t ag_drv_rnr_regs_trace_config_get(uint8_t rnr_id, rnr_regs_trace_config *trace_config);
bdmf_error_t ag_drv_rnr_regs_reset_trace_fifo_set(uint8_t rnr_id, bdmf_boolean trace_reset_event_fifo);
bdmf_error_t ag_drv_rnr_regs_reset_trace_fifo_get(uint8_t rnr_id, bdmf_boolean *trace_reset_event_fifo);
bdmf_error_t ag_drv_rnr_regs_clear_trace_fifo_overrun_set(uint8_t rnr_id, bdmf_boolean trace_clear_fifo_overrun);
bdmf_error_t ag_drv_rnr_regs_clear_trace_fifo_overrun_get(uint8_t rnr_id, bdmf_boolean *trace_clear_fifo_overrun);
bdmf_error_t ag_drv_rnr_regs_rnr_core_cntrs_get(uint8_t rnr_id, rnr_regs_rnr_core_cntrs *rnr_core_cntrs);
bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_set(uint8_t rnr_id, uint8_t thread_num);
bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_get(uint8_t rnr_id, uint8_t *thread_num);
bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, uint8_t dma_static_offset);
bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, uint8_t *dma_static_offset);
bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, uint8_t dma_static_offset);
bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, uint8_t *dma_static_offset);
bdmf_error_t ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(uint8_t rnr_id, uint16_t mask0, uint16_t mask1);
bdmf_error_t ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(uint8_t rnr_id, uint16_t *mask0, uint16_t *mask1);
bdmf_error_t ag_drv_rnr_regs_cfg_sch_cfg_set(uint8_t rnr_id, uint8_t scheduler_mode);
bdmf_error_t ag_drv_rnr_regs_cfg_sch_cfg_get(uint8_t rnr_id, uint8_t *scheduler_mode);
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_bkpt_cfg *cfg_bkpt_cfg);
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_cfg_get(uint8_t rnr_id, rnr_regs_cfg_bkpt_cfg *cfg_bkpt_cfg);
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_imm_set(uint8_t rnr_id, bdmf_boolean enable);
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_imm_get(uint8_t rnr_id, bdmf_boolean *enable);
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_sts_get(uint8_t rnr_id, uint16_t *bkpt_addr, bdmf_boolean *active);
bdmf_error_t ag_drv_rnr_regs_cfg_pc_sts_get(uint8_t rnr_id, uint16_t *current_pc_addr, uint16_t *pc_ret);
bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_set(uint8_t rnr_id, uint16_t trace_base_addr, uint16_t trace_max_addr);
bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_get(uint8_t rnr_id, uint16_t *trace_base_addr, uint16_t *trace_max_addr);
bdmf_error_t ag_drv_rnr_regs_dbg_design_dbg_ctrl_set(uint8_t rnr_id, const rnr_regs_dbg_design_dbg_ctrl *dbg_design_dbg_ctrl);
bdmf_error_t ag_drv_rnr_regs_dbg_design_dbg_ctrl_get(uint8_t rnr_id, rnr_regs_dbg_design_dbg_ctrl *dbg_design_dbg_ctrl);
bdmf_error_t ag_drv_rnr_regs_dbg_design_dbg_data_get(uint8_t rnr_id, uint32_t *dbg_data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_rnr_regs_rnr_enable,
    cli_rnr_regs_dma_illegal,
    cli_rnr_regs_rnr_freq,
    cli_rnr_regs_cam_stop_val,
    cli_rnr_regs_profiling_sts,
    cli_rnr_regs_is_trace_fifo_overrun,
    cli_rnr_regs_trace_config,
    cli_rnr_regs_reset_trace_fifo,
    cli_rnr_regs_clear_trace_fifo_overrun,
    cli_rnr_regs_rnr_core_cntrs,
    cli_rnr_regs_cfg_cpu_wakeup,
    cli_rnr_regs_cfg_ddr_cfg,
    cli_rnr_regs_cfg_psram_cfg,
    cli_rnr_regs_cfg_ramrd_range_mask_cfg,
    cli_rnr_regs_cfg_sch_cfg,
    cli_rnr_regs_cfg_bkpt_cfg,
    cli_rnr_regs_cfg_bkpt_imm,
    cli_rnr_regs_cfg_bkpt_sts,
    cli_rnr_regs_cfg_pc_sts,
    cli_rnr_regs_cfg_profiling_cfg_0,
    cli_rnr_regs_dbg_design_dbg_ctrl,
    cli_rnr_regs_dbg_design_dbg_data,
};

int bcm_rnr_regs_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_rnr_regs_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

