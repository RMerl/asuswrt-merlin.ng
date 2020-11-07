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
/* trace_task: TRACE_TASK - Select task for single task operation (tracer)                        */
/* idle_counter_source_sel: IDLE_COUNTER_SOURCE_SEL - Select mode for IDLE counter                */
/* counters_selected_task_mode: COUNTERS_SELECTED_TASK_MODE - Enable single selected task mode (c */
/*                              ounters)                                                          */
/* counters_task: COUNTERS_TASK - Select task for single task operation (counters)                */
/* trace_reset_event_fifo: TRACE_RESET_EVENT_FIFO - Apply software reset to event FIFO            */
/* trace_clear_fifo_overrun: TRACE_CLEAR_FIFO_OVERRUN - Write 1 to clear event FIFO overrun stick */
/*                           y bit                                                                */
/* en_prof_on_selected_pc: EN_PROF_ON_SELECTED_PC - Enable profiling window between start PC and  */
/*                         end PC                                                                 */
/* trigger_on_second: TRIGGER_ON_SECOND - Dont start profiling window when encountering start PC  */
/*                    for first time                                                              */
/* pc_start: PC_START - Configure PC value to start profiling                                     */
/* pc_stop: PC_STOP - Configure PC value to stop profiling                                        */
/* disable_nops_and_uncond: DISABLE_NOPS_AND_UNCOND - For executed commands counter, dont count N */
/*                          OPs and UNCONDITIONAL JUMP commands.                                  */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean trace_wraparound;
    bdmf_boolean trace_mode;
    bdmf_boolean trace_disable_idle_in;
    bdmf_boolean trace_disable_wakeup_log;
    uint8_t trace_task;
    bdmf_boolean idle_counter_source_sel;
    bdmf_boolean counters_selected_task_mode;
    uint8_t counters_task;
    bdmf_boolean trace_reset_event_fifo;
    bdmf_boolean trace_clear_fifo_overrun;
    bdmf_boolean en_prof_on_selected_pc;
    bdmf_boolean trigger_on_second;
    uint16_t pc_start;
    uint16_t pc_stop;
    bdmf_boolean disable_nops_and_uncond;
} rnr_regs_trace_config;


/**************************************************************************************************/
/* stall_on_jmp_full_cnt: STALL_ON_JMP_FULL_CNT - Count stalls due to branch FIFO full            */
/* total_stall_cnt: TOTAL_STALL_CNT - Count total stall cycles in profiling window                */
/* stall_on_alu_b_full_cnt: STALL_ON_ALU_B_FULL_CNT - Count stalls due to ALU B FIFO full         */
/* stall_on_alu_a_full_cnt: STALL_ON_ALU_A_FULL_CNT - Count stalls due to ALU A FIFO full         */
/* stall_on_jmpreg: STALL_ON_JMPREG - Count stalls due to jump on address from register           */
/* stall_on_memio_full_cnt: STALL_ON_MEMIO_FULL_CNT - Count stalls due to MEMIO FIFO full         */
/* stall_on_waw_cnt: STALL_ON_WAW_CNT - Count stalls due to WAW on conditional                    */
/* active_cycles_cnt: ACTIVE_CYCLES_CNT - Count active cycles of a task                           */
/* exec_counter: EXEC_COUNTER - Count executed commands                                           */
/* idle_cnt: IDLE - IDLE countReserved                                                            */
/* jmp_taken_predicted_untaken_cnt: UNTAKEN_JMP_CNT - Counts jumps with prediction miss, when pre */
/*                                  diction was dont jump                                         */
/* jmp_untaken_predicted_taken_cnt: TAKEN_JMP_CNT - Counts jumps with prediction miss, when predi */
/*                                  ction was jump                                                */
/**************************************************************************************************/
typedef struct
{
    uint16_t stall_on_jmp_full_cnt;
    uint16_t total_stall_cnt;
    uint16_t stall_on_alu_b_full_cnt;
    uint16_t stall_on_alu_a_full_cnt;
    uint16_t stall_on_jmpreg;
    uint16_t stall_on_memio_full_cnt;
    uint16_t stall_on_waw_cnt;
    uint16_t active_cycles_cnt;
    uint32_t exec_counter;
    uint32_t idle_cnt;
    uint16_t jmp_taken_predicted_untaken_cnt;
    uint16_t jmp_untaken_predicted_taken_cnt;
} rnr_regs_rnr_core_cntrs;


/**************************************************************************************************/
/* int0_sts: Interrupt_0_status - While any of this field bits is set interrupt line 0 is set. SW */
/*            can write '1' to clear any bit. Write of '0' is ignored.                            */
/* int1_sts: Interrupt_1_status - While any of this field bits is set interrupt line 0 is set. SW */
/*            can write '1' to clear any bit. Write of '0' is ignored.                            */
/* int2_sts: Interrupt2_status - While this bit is set interrupt line 2 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* int3_sts: Interrupt3_status - While this bit is set interrupt line 3 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* int4_sts: Interrupt4_status - While this bit is set interrupt line 4 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* int5_sts: Interrupt5_status - While this bit is set interrupt line 5 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* int6_sts: Interrupt6_status - While this bit is set interrupt line 6 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* int7_sts: Interrupt7_status - While this bit is set interrupt line 6 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* int8_sts: Interrupt8_status - While this bit is set interrupt line 8 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* int9_sts: Interrupt9_status - While this bit is set interrupt line 9 is set. SW can write '1'  */
/*           to clear any bit. Write of '0' is ignored.                                           */
/* fit_fail_sts: Fit_fail_status -                                                                */
/**************************************************************************************************/
typedef struct
{
    uint8_t int0_sts;
    uint8_t int1_sts;
    bdmf_boolean int2_sts;
    bdmf_boolean int3_sts;
    bdmf_boolean int4_sts;
    bdmf_boolean int5_sts;
    bdmf_boolean int6_sts;
    bdmf_boolean int7_sts;
    bdmf_boolean int8_sts;
    bdmf_boolean int9_sts;
    bdmf_boolean fit_fail_sts;
} rnr_regs_cfg_int_ctrl;


/**************************************************************************************************/
/* int0_mask: Interrupt_0_mask - Mask INT0 causes                                                 */
/* int1_mask: Interrupt_1_mask - INT1 mask cause                                                  */
/* int2_mask: Interrupt_2_mask - INT2 mask cause                                                  */
/* int3_mask: Interrupt_3_mask - INT3 mask cause                                                  */
/* int4_mask: Interrupt_4_mask - INT4 mask cause                                                  */
/* int5_mask: Interrupt_5_mask - INT5 mask cause                                                  */
/* int6_mask: Interrupt_6_mask - INT6 mask cause                                                  */
/* int7_mask: Inerrupt_7_mask - INT7 mask cause                                                   */
/* int8_mask: Interrupt_8_mask - INT8 mask cause                                                  */
/* int9_mask: Interrupt_9_mask - INT9 mask cause                                                  */
/**************************************************************************************************/
typedef struct
{
    uint8_t int0_mask;
    uint8_t int1_mask;
    bdmf_boolean int2_mask;
    bdmf_boolean int3_mask;
    bdmf_boolean int4_mask;
    bdmf_boolean int5_mask;
    bdmf_boolean int6_mask;
    bdmf_boolean int7_mask;
    bdmf_boolean int8_mask;
    bdmf_boolean int9_mask;
} rnr_regs_cfg_int_mask;


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

bdmf_error_t ag_drv_rnr_regs_rnr_enable_set(uint8_t rnr_id, bdmf_boolean en);
bdmf_error_t ag_drv_rnr_regs_rnr_enable_get(uint8_t rnr_id, bdmf_boolean *en);
bdmf_error_t ag_drv_rnr_regs_dma_illegal_set(uint8_t rnr_id, bdmf_boolean dma_illegal_status);
bdmf_error_t ag_drv_rnr_regs_dma_illegal_get(uint8_t rnr_id, bdmf_boolean *dma_illegal_status);
bdmf_error_t ag_drv_rnr_regs_prediction_overrun_set(uint8_t rnr_id, bdmf_boolean prediction_overrun_status);
bdmf_error_t ag_drv_rnr_regs_prediction_overrun_get(uint8_t rnr_id, bdmf_boolean *prediction_overrun_status);
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
bdmf_error_t ag_drv_rnr_regs_cfg_int_ctrl_set(uint8_t rnr_id, const rnr_regs_cfg_int_ctrl *cfg_int_ctrl);
bdmf_error_t ag_drv_rnr_regs_cfg_int_ctrl_get(uint8_t rnr_id, rnr_regs_cfg_int_ctrl *cfg_int_ctrl);
bdmf_error_t ag_drv_rnr_regs_cfg_int_mask_set(uint8_t rnr_id, const rnr_regs_cfg_int_mask *cfg_int_mask);
bdmf_error_t ag_drv_rnr_regs_cfg_int_mask_get(uint8_t rnr_id, rnr_regs_cfg_int_mask *cfg_int_mask);
bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_set(uint8_t rnr_id, bdmf_boolean disable_dma_old_flow_control, bdmf_boolean test_fit_fail);
bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_get(uint8_t rnr_id, bdmf_boolean *disable_dma_old_flow_control, bdmf_boolean *test_fit_fail);
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
bdmf_error_t ag_drv_rnr_regs_cfg_ext_acc_cfg_set(uint8_t rnr_id, uint16_t addr_base, uint8_t addr_step, uint8_t start_thread);
bdmf_error_t ag_drv_rnr_regs_cfg_ext_acc_cfg_get(uint8_t rnr_id, uint16_t *addr_base, uint8_t *addr_step, uint8_t *start_thread);
bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_set(uint8_t rnr_id, uint16_t trace_base_addr, uint16_t trace_max_addr);
bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_get(uint8_t rnr_id, uint16_t *trace_base_addr, uint16_t *trace_max_addr);
bdmf_error_t ag_drv_rnr_regs_cfg_profiling_counter_get(uint8_t rnr_id, uint32_t *val);

#ifdef USE_BDMF_SHELL
enum
{
    cli_rnr_regs_rnr_enable,
    cli_rnr_regs_dma_illegal,
    cli_rnr_regs_prediction_overrun,
    cli_rnr_regs_rnr_freq,
    cli_rnr_regs_cam_stop_val,
    cli_rnr_regs_profiling_sts,
    cli_rnr_regs_is_trace_fifo_overrun,
    cli_rnr_regs_trace_config,
    cli_rnr_regs_reset_trace_fifo,
    cli_rnr_regs_clear_trace_fifo_overrun,
    cli_rnr_regs_rnr_core_cntrs,
    cli_rnr_regs_cfg_cpu_wakeup,
    cli_rnr_regs_cfg_int_ctrl,
    cli_rnr_regs_cfg_int_mask,
    cli_rnr_regs_cfg_gen_cfg,
    cli_rnr_regs_cfg_ddr_cfg,
    cli_rnr_regs_cfg_psram_cfg,
    cli_rnr_regs_cfg_ramrd_range_mask_cfg,
    cli_rnr_regs_cfg_sch_cfg,
    cli_rnr_regs_cfg_bkpt_cfg,
    cli_rnr_regs_cfg_bkpt_imm,
    cli_rnr_regs_cfg_bkpt_sts,
    cli_rnr_regs_cfg_pc_sts,
    cli_rnr_regs_cfg_ext_acc_cfg,
    cli_rnr_regs_cfg_profiling_cfg_0,
    cli_rnr_regs_cfg_profiling_counter,
};

int bcm_rnr_regs_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_rnr_regs_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

