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

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint16_t trace_write_pnt;
    bdmf_boolean idle_no_active_task;
    uint8_t curr_thread_num;
    bdmf_boolean profiling_active;
    bdmf_boolean trace_fifo_overrun;
    uint8_t single_mode_profiling_status;
} rnr_regs_profiling_sts;

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
    bdmf_boolean profiling_window_mode;
    uint8_t single_mode_start_option;
    uint8_t single_mode_stop_option;
    bdmf_boolean window_manual_start;
    bdmf_boolean window_manual_stop;
    bdmf_boolean tracer_enable;
    bdmf_boolean profiling_window_reset;
    bdmf_boolean profiling_window_enable;
    bdmf_boolean trace_reset_event_fifo;
    bdmf_boolean trace_clear_fifo_overrun;
    bdmf_boolean trigger_on_second;
    uint16_t pc_start;
    uint32_t pc_stop_or_cycle_count;
} rnr_regs_trace_config;

typedef struct
{
    uint32_t total_stall_cnt;
    uint16_t stall_on_alu_b_full_cnt;
    uint16_t stall_on_alu_a_full_cnt;
    uint16_t stall_on_jmpreg;
    uint16_t stall_on_memio_full_cnt;
    uint16_t stall_on_waw_cnt;
    uint16_t stall_on_super_cmd;
    uint16_t stall_on_super_cmd_when_full;
    uint16_t stall_on_cs_cnt;
    uint32_t active_cycles_cnt;
    uint16_t stall_on_jmp_full_cnt;
    uint16_t stall_on_skip_jmp_cnt;
    uint32_t exec_counter;
    uint32_t idle_cnt;
    uint16_t jmp_taken_predicted_untaken_cnt;
    uint16_t jmp_untaken_predicted_taken_cnt;
} rnr_regs_rnr_core_cntrs;

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

typedef struct
{
    bdmf_boolean disable_dma_old_flow_control;
    bdmf_boolean test_fit_fail;
    bdmf_boolean zero_data_mem;
    bdmf_boolean zero_context_mem;
    bdmf_boolean zero_data_mem_done;
    bdmf_boolean zero_context_mem_done;
    bdmf_boolean chicken_disable_skip_jmp;
    bdmf_boolean chicken_disable_alu_load_balancing;
    uint8_t gdma_desc_offset;
    bdmf_boolean bbtx_tcam_dest_sel;
    bdmf_boolean bbtx_hash_dest_sel;
    bdmf_boolean bbtx_natc_dest_sel;
    bdmf_boolean bbtx_cnpl_dest_sel;
    bdmf_boolean gdma_gdesc_buffer_size;
    bdmf_boolean chicken_enable_old_unique_id_mode;
    bdmf_boolean chicken_enable_dma_old_mode;
    bdmf_boolean prevent_cs_till_stores_done;
} rnr_regs_cfg_gen_cfg;

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
    bdmf_boolean enable_breakpoint_on_fit_fail;
} rnr_regs_cfg_bkpt_cfg;

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
    bdmf_boolean reset_data_bkpt;
} rnr_regs_cfg_data_bkpt_cfg;


/**********************************************************************************************************************
 * en: 
 *     Runner enable. When reset runner pipe is halted, instruction memory and context memory can be accessed by the
 *     CPU. The CPU can reset or set this bit
 *     The firmware can reset this bit by writing to the disable bit at the runner I/O control register.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_rnr_enable_set(uint8_t rnr_id, bdmf_boolean en);
bdmf_error_t ag_drv_rnr_regs_rnr_enable_get(uint8_t rnr_id, bdmf_boolean *en);

/**********************************************************************************************************************
 * dma_illegal_status: 
 *     Notifies about DMA illegal access (>16 cycles on UBUS). Sticky bit. cleared by writing 1 to this bit.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_dma_illegal_get(uint8_t rnr_id, bdmf_boolean *dma_illegal_status);

/**********************************************************************************************************************
 * prediction_overrun_status: 
 *     Notifies about prediction FIFO overwrite status. Sticky bit. cleared by writing 1 to this bit.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_prediction_overrun_get(uint8_t rnr_id, bdmf_boolean *prediction_overrun_status);

/**********************************************************************************************************************
 * micro_sec_val: 
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_rnr_freq_set(uint8_t rnr_id, uint16_t micro_sec_val);
bdmf_error_t ag_drv_rnr_regs_rnr_freq_get(uint8_t rnr_id, uint16_t *micro_sec_val);

/**********************************************************************************************************************
 * stop_value: 
 *     CAM operation is stopped when reaching an entry with a value matching this field.
 *     For a 32-bit or 64-bit CAM entries, this value is concatenated.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cam_stop_val_set(uint8_t rnr_id, uint16_t stop_value);
bdmf_error_t ag_drv_rnr_regs_cam_stop_val_get(uint8_t rnr_id, uint16_t *stop_value);

/**********************************************************************************************************************
 * trace_write_pnt: 
 *     Trace write pointer
 * idle_no_active_task: 
 *     No active task
 * curr_thread_num: 
 *     Current thread num
 * profiling_active: 
 *     Status of profiling ON/OFF
 * trace_fifo_overrun: 
 *     Sticky bit, indicating trace event FIFO overrun. Cleared by writing bit [31] of PROFILING_CFG_1 register
 * single_mode_profiling_status: 
 *     Shows the status of profiling window in single mode
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_profiling_sts_get(uint8_t rnr_id, rnr_regs_profiling_sts *profiling_sts);

/**********************************************************************************************************************
 * trace_fifo_overrun: 
 *     Sticky bit, indicating trace event FIFO overrun. Cleared by writing bit [31] of PROFILING_CFG_1 register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_is_trace_fifo_overrun_get(uint8_t rnr_id, bdmf_boolean *trace_fifo_overrun);

/**********************************************************************************************************************
 * trace_wraparound: 
 *     Wraparound when writing trace buffer
 * trace_mode: 
 *     Select all tasks or single task mode
 * trace_disable_idle_in: 
 *     Select whether to log IDLE in context swap events
 * trace_disable_wakeup_log: 
 *     Enable/disable logging of scheduler events (wakeups). Relevant only for single task mode
 * trace_task: 
 *     Select task for single task operation (tracer)
 * idle_counter_source_sel: 
 *     Select mode for IDLE counter
 * counters_selected_task_mode: 
 *     Enable single selected task mode (counters)
 * counters_task: 
 *     Select task for single task operation (counters)
 * profiling_window_mode: 
 *     Choose profiling window mode
 * single_mode_start_option: 
 *     Choose start window option in case of single runner profiling window mode
 * single_mode_stop_option: 
 *     Choose stop window option in case of single runner profiling window mode
 * window_manual_start: 
 *     write 1 to start profiling window manually (if appropriate option is configured in SINGLE_MODE_START_OPTION)
 * window_manual_stop: 
 *     write 1 to stop profiling window manually (if appropriate option is configured in SINGLE_MODE_START_OPTION)
 * tracer_enable: 
 *     Enable tracer logic (writing trace to memory)
 * profiling_window_reset: 
 *     write 1 to reset profiling window
 * profiling_window_enable: 
 *     Enable single profiling window. If enabled, will start looking for start condition as specified by START_OPTION
 *     field
 * trace_reset_event_fifo: 
 *     Apply software reset to event FIFO
 * trace_clear_fifo_overrun: 
 *     Write 1 to clear event FIFO overrun sticky bit
 * trigger_on_second: 
 *     Dont start profiling window when encountering start PC for first time
 * pc_start: 
 *     Configure PC value to start profiling
 * pc_stop_or_cycle_count: 
 *     Configure PC value to stop profiling, or number of cycles to count - as selected by STOP_OPTION field in
 *     PROGFILING_CFG_1 register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_trace_config_set(uint8_t rnr_id, const rnr_regs_trace_config *trace_config);
bdmf_error_t ag_drv_rnr_regs_trace_config_get(uint8_t rnr_id, rnr_regs_trace_config *trace_config);

/**********************************************************************************************************************
 * trace_reset_event_fifo: 
 *     Apply software reset to event FIFO
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_reset_trace_fifo_set(uint8_t rnr_id, bdmf_boolean trace_reset_event_fifo);
bdmf_error_t ag_drv_rnr_regs_reset_trace_fifo_get(uint8_t rnr_id, bdmf_boolean *trace_reset_event_fifo);

/**********************************************************************************************************************
 * trace_clear_fifo_overrun: 
 *     Write 1 to clear event FIFO overrun sticky bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_clear_trace_fifo_overrun_set(uint8_t rnr_id, bdmf_boolean trace_clear_fifo_overrun);
bdmf_error_t ag_drv_rnr_regs_clear_trace_fifo_overrun_get(uint8_t rnr_id, bdmf_boolean *trace_clear_fifo_overrun);

/**********************************************************************************************************************
 * total_stall_cnt: 
 *     Count total stall cycles in profiling window
 * stall_on_alu_b_full_cnt: 
 *     Count stalls due to ALU B FIFO full
 * stall_on_alu_a_full_cnt: 
 *     Count stalls due to ALU A FIFO full
 * stall_on_jmpreg: 
 *     Count stalls due to jump on address from register
 * stall_on_memio_full_cnt: 
 *     Count stalls due to MEMIO FIFO full
 * stall_on_waw_cnt: 
 *     Count stalls due to WAW on conditional
 * stall_on_super_cmd: 
 *     Count stalls due to super command
 * stall_on_super_cmd_when_full: 
 *     Count stalls due to super command full stall
 * stall_on_cs_cnt: 
 *     Count stall cycles due to CS event
 * active_cycles_cnt: 
 *     Count active cycles of a task
 * stall_on_jmp_full_cnt: 
 *     Count stalls due to branch FIFO full
 * stall_on_skip_jmp_cnt: 
 *     Count stalls due wit conditional on skip jump
 * exec_counter: 
 *     Count executed commands
 * idle_cnt: 
 *     IDLE countReserved
 * jmp_taken_predicted_untaken_cnt: 
 *     Counts jumps with prediction miss, when prediction was dont jump
 * jmp_untaken_predicted_taken_cnt: 
 *     Counts jumps with prediction miss, when prediction was jump
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_rnr_core_cntrs_get(uint8_t rnr_id, rnr_regs_rnr_core_cntrs *rnr_core_cntrs);

/**********************************************************************************************************************
 * thread_num: 
 *     The thread number to be invoked by the CPU.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_set(uint8_t rnr_id, uint8_t thread_num);
bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_get(uint8_t rnr_id, uint8_t *thread_num);

/**********************************************************************************************************************
 * int0_sts: 
 *     While any of this field bits is set interrupt line 0 is set. SW can write '1' to clear any bit. Write of '0' is
 *     ignored.
 * int1_sts: 
 *     While any of this field bits is set interrupt line 0 is set. SW can write '1' to clear any bit. Write of '0' is
 *     ignored.
 * int2_sts: 
 *     While this bit is set interrupt line 2 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * int3_sts: 
 *     While this bit is set interrupt line 3 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * int4_sts: 
 *     While this bit is set interrupt line 4 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * int5_sts: 
 *     While this bit is set interrupt line 5 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * int6_sts: 
 *     While this bit is set interrupt line 6 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * int7_sts: 
 *     While this bit is set interrupt line 6 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * int8_sts: 
 *     While this bit is set interrupt line 8 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * int9_sts: 
 *     While this bit is set interrupt line 9 is set. SW can write '1' to clear any bit. Write of '0' is ignored.
 * fit_fail_sts: 
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_int_ctrl_set(uint8_t rnr_id, const rnr_regs_cfg_int_ctrl *cfg_int_ctrl);
bdmf_error_t ag_drv_rnr_regs_cfg_int_ctrl_get(uint8_t rnr_id, rnr_regs_cfg_int_ctrl *cfg_int_ctrl);

/**********************************************************************************************************************
 * int0_mask: 
 *     Mask INT0 causes
 * int1_mask: 
 *     INT1 mask cause
 * int2_mask: 
 *     INT2 mask cause
 * int3_mask: 
 *     INT3 mask cause
 * int4_mask: 
 *     INT4 mask cause
 * int5_mask: 
 *     INT5 mask cause
 * int6_mask: 
 *     INT6 mask cause
 * int7_mask: 
 *     INT7 mask cause
 * int8_mask: 
 *     INT8 mask cause
 * int9_mask: 
 *     INT9 mask cause
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_int_mask_set(uint8_t rnr_id, const rnr_regs_cfg_int_mask *cfg_int_mask);
bdmf_error_t ag_drv_rnr_regs_cfg_int_mask_get(uint8_t rnr_id, rnr_regs_cfg_int_mask *cfg_int_mask);

/**********************************************************************************************************************
 * disable_dma_old_flow_control: 
 *     Disable DMA old flow control. When set to 1, DMA will not check read FIFO occupancy when issuing READ requests,
 *     relying instead on DMA backpressure mechanism vs read dispatcher block.
 * test_fit_fail: 
 *     set to 1 to test fit fail interrupt.
 * zero_data_mem: 
 *     Trigger self-zeroing mechanism for data memory.
 *     
 * zero_context_mem: 
 *     Trigger self-zeroing mechanism for context memory.
 *     
 * zero_data_mem_done: 
 *     Goes high when zeroing is done. Reset to 0 when config ZERO_DATA_MEM is set to 1
 * zero_context_mem_done: 
 *     Goes high when zeroing is done. Reset to 0 when config ZERO_CONTEXT_MEM is set to 1
 * chicken_disable_skip_jmp: 
 *     When set to 1, skip jump functionality is disabled
 * chicken_disable_alu_load_balancing: 
 *     When set to 1, ALU load balancing functionality is disabled
 * gdma_desc_offset: 
 *     Configure descriptor offset for GATHER DMA command
 * bbtx_tcam_dest_sel: 
 *     Select destination TCAM for Runner
 * bbtx_hash_dest_sel: 
 *     Select destination HASH for Runner
 * bbtx_natc_dest_sel: 
 *     Select destination NATC for Runner
 * bbtx_cnpl_dest_sel: 
 *     Select destination CNPL for Runner
 * gdma_gdesc_buffer_size: 
 *     0 - GDMA prefetches 32B when reading descriptors from the data memory
 *     1 - GDMA prefetches 64B when reading descriptors from the data memory
 *     
 *     if 0 is set then total number of gather descriptors per GDMA command isnt allowed to exceed 32B.
 * chicken_enable_old_unique_id_mode: 
 *     When set to 1, use old mode for unique ID assignment
 * chicken_enable_dma_old_mode: 
 *     When set to 1, use old mode for 40 bit address calculation
 * prevent_cs_till_stores_done: 
 *     When set to 1, prevents context swap until all STOREs reach meeory
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_gen_cfg *cfg_gen_cfg);
bdmf_error_t ag_drv_rnr_regs_cfg_gen_cfg_get(uint8_t rnr_id, rnr_regs_cfg_gen_cfg *cfg_gen_cfg);

/**********************************************************************************************************************
 * dma_base: 
 *     DMA base address for ADDR_CALC
 * dma_static_offset: 
 *     DMA static offset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_fpm_mini_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_static_offset);
bdmf_error_t ag_drv_rnr_regs_cfg_fpm_mini_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_static_offset);

/**********************************************************************************************************************
 * dma_base: 
 *     DMA base address for ADDR_CALC
 * dma_buf_size: 
 *     3 bits indicating buffer size
 * dma_buf_size_mode: 
 *     Determine old buffer  mode or new mode.
 *     Old mode: 128, 256, 512, 1024, 2048
 *     New mode: 128, 320, 640, 1280, 2560
 * dma_static_offset: 
 *     DMA static offset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, bdmf_boolean dma_buf_size_mode, uint8_t dma_static_offset);
bdmf_error_t ag_drv_rnr_regs_cfg_ddr_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, bdmf_boolean *dma_buf_size_mode, uint8_t *dma_static_offset);

/**********************************************************************************************************************
 * dma_base: 
 *     DMA base address for ADDR_CALC
 * dma_buf_size: 
 *     3 bits indicating buffer size
 * dma_buf_size_mode: 
 *     Determine old buffer  mode or new mode
 *     Old mode: 128, 256, 512, 1024, 2048
 *     New mode: 128, 320, 640, 1280, 2560
 * dma_static_offset: 
 *     DMA static offset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_set(uint8_t rnr_id, uint32_t dma_base, uint8_t dma_buf_size, bdmf_boolean dma_buf_size_mode, uint8_t dma_static_offset);
bdmf_error_t ag_drv_rnr_regs_cfg_psram_cfg_get(uint8_t rnr_id, uint32_t *dma_base, uint8_t *dma_buf_size, bdmf_boolean *dma_buf_size_mode, uint8_t *dma_static_offset);

/**********************************************************************************************************************
 * mask0: 
 *     Mask 0 for range serach. according to the number of 1 in the mask the cam machine can differ between the Key
 *     and TAG
 * mask1: 
 *     Mask 0 for range serach. according to the number of 1 in the mask the cam machine can differ between the Key
 *     and TAG
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_set(uint8_t rnr_id, uint16_t mask0, uint16_t mask1);
bdmf_error_t ag_drv_rnr_regs_cfg_ramrd_range_mask_cfg_get(uint8_t rnr_id, uint16_t *mask0, uint16_t *mask1);

/**********************************************************************************************************************
 * scheduler_mode: 
 *     Configure priority mode for scheduler operation
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_sch_cfg_set(uint8_t rnr_id, uint8_t scheduler_mode);
bdmf_error_t ag_drv_rnr_regs_cfg_sch_cfg_get(uint8_t rnr_id, uint8_t *scheduler_mode);

/**********************************************************************************************************************
 * bkpt_0_en: 
 *     Enable breakpoint 0
 * bkpt_0_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_1_en: 
 *     Enable breakpoint 1
 * bkpt_1_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_2_en: 
 *     Enable breakpoint 2
 * bkpt_2_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_3_en: 
 *     Enable breakpoint 3
 * bkpt_3_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_4_en: 
 *     Enable breakpoint 4
 * bkpt_4_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_5_en: 
 *     Enable breakpoint 5
 * bkpt_5_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_6_en: 
 *     Enable breakpoint 6
 * bkpt_6_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_7_en: 
 *     Enable breakpoint 7
 * bkpt_7_use_thread: 
 *     Enable breakpoint for given thread only
 * step_mode: 
 *     Configure step mode
 * new_flags_val: 
 *     Value for new flags
 * enable_breakpoint_on_fit_fail: 
 *     When set to 1, Runner will break on fit_fail
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_bkpt_cfg *cfg_bkpt_cfg);
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_cfg_get(uint8_t rnr_id, rnr_regs_cfg_bkpt_cfg *cfg_bkpt_cfg);

/**********************************************************************************************************************
 * enable: 
 *     Enable immediate breakpoint
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_imm_set(uint8_t rnr_id, bdmf_boolean enable);
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_imm_get(uint8_t rnr_id, bdmf_boolean *enable);

/**********************************************************************************************************************
 * bkpt_addr: 
 *     Breakpoint address
 * active: 
 *     Breakpoint active indication
 * data_bkpt_addr: 
 *     Data address when triggered by breakpoint
 * bkpt_reason: 
 *     Display reason for breakpoint
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_bkpt_sts_get(uint8_t rnr_id, uint16_t *bkpt_addr, bdmf_boolean *active, uint16_t *data_bkpt_addr, uint8_t *bkpt_reason);

/**********************************************************************************************************************
 * current_pc_addr: 
 *     Current program counter address
 * pc_ret: 
 *     Call stack return address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_pc_sts_get(uint8_t rnr_id, uint16_t *current_pc_addr, uint16_t *pc_ret);

/**********************************************************************************************************************
 * addr_base: 
 *     address base for calculation.
 *     See description under start_thread field
 * addr_step_0: 
 *     address step 0 for thread base address calculation. See description under start_thread field.
 * addr_step_1: 
 *     address step 1 for thread base address calculation. See description under start_thread field.
 * start_thread: 
 *     start thread for calculation.
 *     
 *     Address for external accelerator will be calculated as follows:
 *     <adjusted_thread>=<current_thread>-<start_thread>
 *     
 *     acc_thread_address = address_base + adjusted_thread*(2^step_0) + adjusted_thread*(2^step_1)
 *     
 *     Note that if step_0 or step_1 are set to 0, the operands of the equation will still be non-zero.
 *     So if total_step is required to be an exact power N of 2, then both step_0 and step_1 should be set to (N-1).
 *     For example if total step is 256 (2^8), then both step_0 and step_1 should be set to 7.
 *     
 *     In any case, the resulting address should be always aligned to 8-bytes (64-bits)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_ext_acc_cfg_set(uint8_t rnr_id, uint16_t addr_base, uint8_t addr_step_0, uint8_t addr_step_1, uint8_t start_thread);
bdmf_error_t ag_drv_rnr_regs_cfg_ext_acc_cfg_get(uint8_t rnr_id, uint16_t *addr_base, uint8_t *addr_step_0, uint8_t *addr_step_1, uint8_t *start_thread);

/**********************************************************************************************************************
 * start_addr: 
 *     If (current PC >= start_addr) AND (current PC <= stop_addr), fit fail will not be checked. START_ADDR value
 *     should be even.
 * stop_addr: 
 *     If (current PC >= start_addr) AND (current PC <= stop_addr), fit fail will not be checked. STOP_ADDR value
 *     should be even.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_fit_fail_cfg_set(uint8_t rnr_id, uint16_t start_addr, uint16_t stop_addr);
bdmf_error_t ag_drv_rnr_regs_cfg_fit_fail_cfg_get(uint8_t rnr_id, uint16_t *start_addr, uint16_t *stop_addr);

/**********************************************************************************************************************
 * bkpt_0_en: 
 *     Enable breakpoint 0
 * bkpt_0_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_1_en: 
 *     Enable breakpoint 1
 * bkpt_1_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_2_en: 
 *     Enable breakpoint 2
 * bkpt_2_use_thread: 
 *     Enable breakpoint for given thread only
 * bkpt_3_en: 
 *     Enable breakpoint 3
 * bkpt_3_use_thread: 
 *     Enable breakpoint for given thread only
 * reset_data_bkpt: 
 *     Reset data breakpoint mechanism. Write 1 and then 0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_data_bkpt_cfg_set(uint8_t rnr_id, const rnr_regs_cfg_data_bkpt_cfg *cfg_data_bkpt_cfg);
bdmf_error_t ag_drv_rnr_regs_cfg_data_bkpt_cfg_get(uint8_t rnr_id, rnr_regs_cfg_data_bkpt_cfg *cfg_data_bkpt_cfg);

/**********************************************************************************************************************
 * aqm_counter_value: 
 *     Counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_aqm_counter_val_get(uint8_t rnr_id, uint32_t *aqm_counter_value);

/**********************************************************************************************************************
 * trace_base_addr: 
 *     Base address for trace buffer
 * trace_max_addr: 
 *     Trace buffer MAX address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_set(uint8_t rnr_id, uint16_t trace_base_addr, uint16_t trace_max_addr);
bdmf_error_t ag_drv_rnr_regs_cfg_profiling_cfg_0_get(uint8_t rnr_id, uint16_t *trace_base_addr, uint16_t *trace_max_addr);

/**********************************************************************************************************************
 * val: 
 *     Current 32-bit value of counter
 **********************************************************************************************************************/
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
    cli_rnr_regs_cfg_fpm_mini_cfg,
    cli_rnr_regs_cfg_ddr_cfg,
    cli_rnr_regs_cfg_psram_cfg,
    cli_rnr_regs_cfg_ramrd_range_mask_cfg,
    cli_rnr_regs_cfg_sch_cfg,
    cli_rnr_regs_cfg_bkpt_cfg,
    cli_rnr_regs_cfg_bkpt_imm,
    cli_rnr_regs_cfg_bkpt_sts,
    cli_rnr_regs_cfg_pc_sts,
    cli_rnr_regs_cfg_ext_acc_cfg,
    cli_rnr_regs_cfg_fit_fail_cfg,
    cli_rnr_regs_cfg_data_bkpt_cfg,
    cli_rnr_regs_cfg_aqm_counter_val,
    cli_rnr_regs_cfg_profiling_cfg_0,
    cli_rnr_regs_cfg_profiling_counter,
};

int bcm_rnr_regs_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_rnr_regs_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
