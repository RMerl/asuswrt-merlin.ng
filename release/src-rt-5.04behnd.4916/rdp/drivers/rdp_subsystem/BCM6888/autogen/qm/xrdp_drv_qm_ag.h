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


#ifndef _XRDP_DRV_QM_AG_H_
#define _XRDP_DRV_QM_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean ddr_byte_congestion_drop_enable;
    uint32_t ddr_bytes_lower_thr;
    uint32_t ddr_bytes_mid_thr;
    uint32_t ddr_bytes_higher_thr;
    bdmf_boolean ddr_pd_congestion_drop_enable;
    uint8_t ddr_pipe_lower_thr;
    uint8_t ddr_pipe_higher_thr;
} qm_ddr_cong_ctrl;

typedef struct
{
    uint32_t lower_thr;
    uint32_t mid_thr;
    uint32_t higher_thr;
} qm_fpm_ug_thr;

typedef struct
{
    uint16_t start_queue;
    uint16_t end_queue;
    uint16_t pd_fifo_base;
    uint8_t pd_fifo_size;
    uint16_t upd_fifo_base;
    uint8_t upd_fifo_size;
    uint8_t rnr_bb_id;
    uint8_t rnr_task;
    bdmf_boolean rnr_enable;
} qm_rnr_group_cfg;

typedef struct
{
    uint32_t min_thr0;
    bdmf_boolean flw_ctrl_en0;
    uint32_t min_thr1;
    bdmf_boolean flw_ctrl_en1;
    uint32_t max_thr0;
    uint32_t max_thr1;
    uint8_t slope_mantissa0;
    uint8_t slope_exp0;
    uint8_t slope_mantissa1;
    uint8_t slope_exp1;
} qm_wred_profile_cfg;

typedef struct
{
    bdmf_boolean fpm_prefetch_enable;
    bdmf_boolean reorder_credit_enable;
    bdmf_boolean dqm_pop_enable;
    bdmf_boolean rmt_fixed_arb_enable;
    bdmf_boolean dqm_push_fixed_arb_enable;
    bdmf_boolean aqm_clk_counter_enable;
    bdmf_boolean aqm_timestamp_counter_enable;
    bdmf_boolean aqm_timestamp_write_to_pd_enable;
} qm_enable_ctrl;

typedef struct
{
    bdmf_boolean fpm_prefetch0_sw_rst;
    bdmf_boolean fpm_prefetch1_sw_rst;
    bdmf_boolean fpm_prefetch2_sw_rst;
    bdmf_boolean fpm_prefetch3_sw_rst;
    bdmf_boolean normal_rmt_sw_rst;
    bdmf_boolean non_delayed_rmt_sw_rst;
    bdmf_boolean pre_cm_fifo_sw_rst;
    bdmf_boolean cm_rd_pd_fifo_sw_rst;
    bdmf_boolean cm_wr_pd_fifo_sw_rst;
    bdmf_boolean bb0_output_fifo_sw_rst;
    bdmf_boolean bb1_output_fifo_sw_rst;
    bdmf_boolean bb1_input_fifo_sw_rst;
    bdmf_boolean tm_fifo_ptr_sw_rst;
    bdmf_boolean non_delayed_out_fifo_sw_rst;
    bdmf_boolean bb0_egr_msg_out_fifo_sw_rst;
} qm_reset_ctrl;

typedef struct
{
    bdmf_boolean read_clear_pkts;
    bdmf_boolean read_clear_bytes;
    bdmf_boolean disable_wrap_around_pkts;
    bdmf_boolean disable_wrap_around_bytes;
    bdmf_boolean free_with_context_last_search;
    bdmf_boolean wred_disable;
    bdmf_boolean ddr_pd_congestion_disable;
    bdmf_boolean ddr_byte_congestion_disable;
    bdmf_boolean ddr_occupancy_disable;
    bdmf_boolean ddr_fpm_congestion_disable;
    bdmf_boolean fpm_ug_disable;
    bdmf_boolean queue_occupancy_ddr_copy_decision_disable;
    bdmf_boolean psram_occupancy_ddr_copy_decision_disable;
    bdmf_boolean dont_send_mc_bit_to_bbh;
    bdmf_boolean close_aggregation_on_timeout_disable;
    bdmf_boolean fpm_congestion_buf_release_mechanism_disable;
    bdmf_boolean fpm_buffer_global_res_enable;
    bdmf_boolean qm_preserve_pd_with_fpm;
    bdmf_boolean qm_residue_per_queue;
    bdmf_boolean ghost_rpt_update_after_close_agg_en;
    bdmf_boolean fpm_ug_flow_ctrl_disable;
    bdmf_boolean ddr_write_multi_slave_en;
    bdmf_boolean ddr_pd_congestion_agg_priority;
    bdmf_boolean psram_occupancy_drop_disable;
    bdmf_boolean qm_ddr_write_alignment;
    bdmf_boolean exclusive_dont_drop;
    bdmf_boolean dqmol_jira_973_fix_enable;
    bdmf_boolean gpon_dbr_ceil;
    bdmf_boolean drop_cnt_wred_drops;
    bdmf_boolean same_sec_lvl_bit_agg_en;
    bdmf_boolean glbl_egr_drop_cnt_read_clear_enable;
    bdmf_boolean glbl_egr_aqm_drop_cnt_read_clear_enable;
} qm_drop_counters_ctrl;

typedef struct
{
    bdmf_boolean fpm_pool_bp_enable;
    bdmf_boolean fpm_congestion_bp_enable;
    uint8_t fpm_force_bp_lvl;
    bdmf_boolean fpm_prefetch_granularity;
    uint8_t fpm_prefetch_min_pool_size;
    uint8_t fpm_prefetch_pending_req_limit;
    bdmf_boolean fpm_override_bb_id_en;
    uint8_t fpm_override_bb_id_value;
} qm_fpm_ctrl;

typedef struct
{
    uint16_t max_agg_bytes;
    uint8_t max_agg_pkts;
    bdmf_boolean agg_ovr_512b_en;
    uint16_t max_agg_pkt_size;
    uint16_t min_agg_pkt_size;
} qm_global_cfg_aggregation_ctrl;

typedef struct
{
    bdmf_boolean epon_line_rate;
    bdmf_boolean epon_crc_add_disable;
    bdmf_boolean mac_flow_overwrite_crc_en;
    uint8_t mac_flow_overwrite_crc;
    uint16_t fec_ipg_length;
} qm_epon_overhead_ctrl;

typedef struct
{
    bdmf_boolean egress_accumulated_cnt_pkts_read_clear_enable;
    bdmf_boolean egress_accumulated_cnt_bytes_read_clear_enable;
    bdmf_boolean agg_closure_suspend_on_bp;
    bdmf_boolean bufmng_en_or_ug_cntr;
    bdmf_boolean dqm_to_fpm_ubus_or_fpmini;
} qm_global_cfg_qm_general_ctrl2;

typedef struct
{
    uint8_t lower_thr;
    uint8_t higher_thr;
} qm_fpm_pool_thr;

typedef struct
{
    bdmf_boolean qm_dqm_pop_on_empty;
    bdmf_boolean qm_dqm_push_on_full;
    bdmf_boolean qm_cpu_pop_on_empty;
    bdmf_boolean qm_cpu_push_on_full;
    bdmf_boolean qm_normal_queue_pd_no_credit;
    bdmf_boolean qm_non_delayed_queue_pd_no_credit;
    bdmf_boolean qm_non_valid_queue;
    bdmf_boolean qm_agg_coherent_inconsistency;
    bdmf_boolean qm_force_copy_on_non_delayed;
    bdmf_boolean qm_fpm_pool_size_nonexistent;
    bdmf_boolean qm_target_mem_abs_contradiction;
    bdmf_boolean qm_1588_drop;
    bdmf_boolean qm_1588_multicast_contradiction;
    bdmf_boolean qm_byte_drop_cnt_overrun;
    bdmf_boolean qm_pkt_drop_cnt_overrun;
    bdmf_boolean qm_total_byte_cnt_underrun;
    bdmf_boolean qm_total_pkt_cnt_underrun;
    bdmf_boolean qm_fpm_ug0_underrun;
    bdmf_boolean qm_fpm_ug1_underrun;
    bdmf_boolean qm_fpm_ug2_underrun;
    bdmf_boolean qm_fpm_ug3_underrun;
    bdmf_boolean qm_timer_wraparound;
    bdmf_boolean qm_copy_plen_zero;
    bdmf_boolean qm_ingress_bb_unexpected_msg;
    bdmf_boolean qm_egress_bb_unexpected_msg;
    bdmf_boolean dqm_reached_full;
} qm_intr_ctrl_isr;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} qm_clk_gate_clk_gate_cntrl;

typedef struct
{
    uint8_t wred_profile;
    uint8_t copy_dec_profile;
    bdmf_boolean ddr_copy_disable;
    bdmf_boolean aggregation_disable;
    uint8_t fpm_ug_or_bufmng;
    bdmf_boolean exclusive_priority;
    bdmf_boolean q_802_1ae;
    bdmf_boolean sci;
    bdmf_boolean fec_enable;
    uint8_t res_profile;
    uint8_t spare_room_0;
    uint8_t spare_room_1;
    uint8_t service_queue_profile;
    uint8_t timestamp_res_profile;
} qm_q_context;


/**********************************************************************************************************************
 * ddr_byte_congestion_drop_enable: 
 *     This field indicates whether crossing the DDR bytes thresholds (the number of bytes waiting to be copied to
 *     DDR) will result in dropping packets or in applying back pressure to the re-order.
 *     0 - apply back pressure
 *     1 - drop packets
 *     
 * ddr_bytes_lower_thr: 
 *     DDR copy bytes Lower Threshold.
 *     When working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:
 *     * If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.
 *     * If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high
 *     priority are dropped (only exclusive packets are not dropped).
 *     * If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are
 *     dropped.
 *     * If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.
 *     When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) >
 *     (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and
 *     DDR_BYTES_MID_THR are dont care).
 * ddr_bytes_mid_thr: 
 *     DDR copy bytes Lower Threshold.
 *     When working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:
 *     * If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.
 *     * If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high
 *     priority are dropped (only exclusive packets are not dropped).
 *     * If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are
 *     dropped.
 *     * If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.
 *     When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) >
 *     (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and
 *     DDR_BYTES_MID_THR are dont care).
 * ddr_bytes_higher_thr: 
 *     DDR copy bytes Lower Threshold.
 *     When working in packet drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:
 *     * If (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.
 *     * If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then packets in low/high
 *     priority are dropped (only exclusive packets are not dropped).
 *     * If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_THR), then packets in low priority are
 *     dropped.
 *     * If (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.
 *     When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy bytes counter) >
 *     (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR and
 *     DDR_BYTES_MID_THR are dont care).
 * ddr_pd_congestion_drop_enable: 
 *     This field indicates whether crossing the DDR Pipe thresholds will result in dropping packets or in applying
 *     back pressure to the re-order.
 *     0 - apply back pressure
 *     1 - drop packets
 *     
 * ddr_pipe_lower_thr: 
 *     DDR copy Pipe Lower Threshold.
 *     When working in packet drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:
 *     * If (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are dropped (only
 *     exclusive packets are not dropped).
 *     * If (DDR_PIPE_LOWER_THR) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low priority
 *     are dropped.
 *     * If (DDR copy pipe occupancy) <=  (DDR_PIPE_LOWER_THR), then no packets are dropped.
 *     When working in backpressure mode (DDR_PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) >
 *     (DDR_PIPE_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_LOWER_THR is dont care).
 * ddr_pipe_higher_thr: 
 *     DDR copy Pipe Lower Threshold.
 *     When working in packet drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:
 *     * If (DDR copy pipe occupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are dropped (only
 *     exclusive packets are not dropped).
 *     * If (DDR_PIPE_LOWER_THR) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low priority
 *     are dropped.
 *     * If (DDR copy pipe occupancy) <= (DDR_PIPE_LOWER_THR), then no packets are dropped.
 *     When working in backpressure mode (DDR_PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) >
 *     (DDR_PIPE_HIGHER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_LOWER_THR is dont care).
 *     IMPORTANT: recommended maximum value is 0x7B in order to avoid performance degradation when working with
 *     aggregation timeout enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_ddr_cong_ctrl_set(const qm_ddr_cong_ctrl *ddr_cong_ctrl);
bdmf_error_t ag_drv_qm_ddr_cong_ctrl_get(qm_ddr_cong_ctrl *ddr_cong_ctrl);

/**********************************************************************************************************************
 * q_not_empty: 
 *     Queue Not empty indication.
 *     This is a 1-bit indication per queue.
 *     This register consists of a batch of 32 queues.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_is_queue_not_empty_get(uint16_t q_idx, bdmf_boolean *data);

/**********************************************************************************************************************
 * pop_ready: 
 *     Queue pop ready indication.
 *     This is a 1-bit indication per queue.
 *     This register consists of a batch of 32 queues.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_is_queue_pop_ready_get(uint16_t q_idx, bdmf_boolean *data);

/**********************************************************************************************************************
 * q_full: 
 *     Queue Full indication.
 *     This is a 1-bit indication per queue.
 *     This register consists of a batch of 32 queues.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_is_queue_full_get(uint16_t q_idx, bdmf_boolean *data);

/**********************************************************************************************************************
 * lower_thr: 
 *     FPM group Lower Threshold.
 *     * If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped.
 *     * If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority
 *     are dropped (only exclusive packets are not dropped).
 *     * If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are
 *     dropped.
 *     * If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.
 * mid_thr: 
 *     FPM group Lower Threshold.
 *     * If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped.
 *     * If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority
 *     are dropped (only exclusive packets are not dropped).
 *     * If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are
 *     dropped.
 *     * If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.
 * higher_thr: 
 *     FPM group Lower Threshold.
 *     * If (FPM User Group Counter) > (FPM_GRP_HIGHER_THR), all packets in this user group are dropped.
 *     * If (FPM_GRP_MID_THR) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority
 *     are dropped (only exclusive packets are not dropped).
 *     * If (FPM_GRP_LOWER_THR) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are
 *     dropped.
 *     * If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_ug_thr_set(uint8_t ug_grp_idx, const qm_fpm_ug_thr *fpm_ug_thr);
bdmf_error_t ag_drv_qm_fpm_ug_thr_get(uint8_t ug_grp_idx, qm_fpm_ug_thr *fpm_ug_thr);

/**********************************************************************************************************************
 * start_queue: 
 *     Indicates the Queue that starts this runner group. Queues belonging to the runner group are defined by the
 *     following equation:
 *     START_QUEUE <= runner_queues <= END_QUEUE
 * end_queue: 
 *     Indicates the Queue that ends this runner group.
 *     Queues belonging to the runner group are defined by the following equation:
 *     START_QUEUE <= runner_queues <= END_QUEUE
 * pd_fifo_base: 
 *     PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_ADDR*8).
 * pd_fifo_size: 
 *     PD FIFO Size
 *     0 - 2 entries
 *     1 - 4 entries
 *     2 - 8 entries
 * upd_fifo_base: 
 *     PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_ADDR*8).
 * upd_fifo_size: 
 *     PD FIFO Size
 *     0 - 8 entries
 *     1 - 16 entries
 *     2 - 32 entries
 *     3 - 64 entries
 *     4 - 128 entries
 *     5 - 256 entries
 * rnr_bb_id: 
 *     Runner BB ID associated with this configuration.
 * rnr_task: 
 *     Runner Task number to be woken up when the update FIFO is written to.
 * rnr_enable: 
 *     Enable this runner interface
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_rnr_group_cfg_set(uint8_t rnr_idx, const qm_rnr_group_cfg *rnr_group_cfg);
bdmf_error_t ag_drv_qm_rnr_group_cfg_get(uint8_t rnr_idx, qm_rnr_group_cfg *rnr_group_cfg);

/**********************************************************************************************************************
 * data0: 
 *     Data
 * data1: 
 *     Data
 * data2: 
 *     Data
 * data3: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_set(uint8_t indirect_grp_idx, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3);

/**********************************************************************************************************************
 * data0: 
 *     Data
 * data1: 
 *     Data
 * data2: 
 *     Data
 * data3: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cpu_pd_indirect_rd_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3);

/**********************************************************************************************************************
 * context_valid: 
 *     QM ingress aggregation context valid indication.
 *     This is a 1-bit indication per queue.
 *     This register consists of a batch of 32 queues.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_aggr_context_get(uint16_t idx, uint32_t *context_valid);

/**********************************************************************************************************************
 * min_thr0: 
 *     WRED Color Min Threshold.
 *     This field represents the higher 24-bits of the queue occupancy byte threshold.
 *     byte_threshold = THR*64.
 * flw_ctrl_en0: 
 *     0 - flow control disable. regular WRED profile
 *     1 - flow control enable. no WRED drop, wake up appropriate runner task when crossed.
 * min_thr1: 
 *     WRED Color Min Threshold.
 *     This field represents the higher 24-bits of the queue occupancy byte threshold.
 *     byte_threshold = THR*64.
 * flw_ctrl_en1: 
 *     0 - flow control disable. regular WRED profile
 *     1 - flow control enable. no WRED drop, wake up appropriate runner task when crossed.
 * max_thr0: 
 *     WRED Color Max Threshold.
 *     This field represents the higher 24-bits of the queue occupancy byte threshold.
 *     byte_threshold = THR*64.
 * max_thr1: 
 *     WRED Color Max Threshold.
 *     This field represents the higher 24-bits of the queue occupancy byte threshold.
 *     byte_threshold = THR*64.
 * slope_mantissa0: 
 *     WRED Color slope mantissa.
 * slope_exp0: 
 *     WRED Color slope exponent.
 * slope_mantissa1: 
 *     WRED Color slope mantissa.
 * slope_exp1: 
 *     WRED Color slope exponent.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_wred_profile_cfg_set(uint8_t profile_idx, const qm_wred_profile_cfg *wred_profile_cfg);
bdmf_error_t ag_drv_qm_wred_profile_cfg_get(uint8_t profile_idx, qm_wred_profile_cfg *wred_profile_cfg);

/**********************************************************************************************************************
 * fpm_prefetch_enable: 
 *     FPM Prefetch Enable. Setting this bit to 1 will start filling up the FPM pool prefetch FIFOs.
 *     Seeting this bit to 0, will stop FPM prefetches.
 * reorder_credit_enable: 
 *     When this bit is set the QM will send credits to the REORDER block.
 *     Disabling this bit will stop sending credits to the reorder.
 * dqm_pop_enable: 
 *     When this bit is set the QM will pop PDs from the DQM and place them in the runner SRAM
 * rmt_fixed_arb_enable: 
 *     When this bit is set Fixed arbitration will be done in pops from the remote FIFOs (Non delayed highest
 *     priority). If this bit is cleared RR arbitration is done
 * dqm_push_fixed_arb_enable: 
 *     When this bit is set Fixed arbitration will be done in DQM pushes (CPU highest priority, then non-delayed
 *     queues and then normal queues. If this bit is cleared RR arbitration is done.
 * aqm_clk_counter_enable: 
 *     When this bit is set it enables the AQM internal global counter that counts clocks to uSec. If this bit is
 *     cleared the counter is disabled and zeroed.
 * aqm_timestamp_counter_enable: 
 *     When this bit is set it enables the AQM timestamp global counter that counts timestamp in uSec resolution. If
 *     this bit is cleared the counter is disabled and zeroed.
 * aqm_timestamp_write_to_pd_enable: 
 *     When this bit is set it enables the AQM timestamp 10bit write to PD. If this bit is cleared the timestamp is
 *     not written to PD.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_enable_ctrl_set(const qm_enable_ctrl *enable_ctrl);
bdmf_error_t ag_drv_qm_enable_ctrl_get(qm_enable_ctrl *enable_ctrl);

/**********************************************************************************************************************
 * fpm_prefetch0_sw_rst: 
 *     FPM Prefetch FIFO0 SW reset.
 * fpm_prefetch1_sw_rst: 
 *     FPM Prefetch FIFO1 SW reset.
 * fpm_prefetch2_sw_rst: 
 *     FPM Prefetch FIFO2 SW reset.
 * fpm_prefetch3_sw_rst: 
 *     FPM Prefetch FIFO3 SW reset.
 * normal_rmt_sw_rst: 
 *     Normal Remote FIFO SW reset.
 * non_delayed_rmt_sw_rst: 
 *     Non-delayed Remote FIFO SW reset.
 * pre_cm_fifo_sw_rst: 
 *     Pre Copy Machine FIFO SW reset.
 * cm_rd_pd_fifo_sw_rst: 
 *     Copy Machine RD PD FIFO SW reset.
 * cm_wr_pd_fifo_sw_rst: 
 *     Pre Copy Machine FIFO SW reset.
 * bb0_output_fifo_sw_rst: 
 *     BB0 OUTPUT FIFO SW reset.
 * bb1_output_fifo_sw_rst: 
 *     BB1 Output FIFO SW reset.
 * bb1_input_fifo_sw_rst: 
 *     BB1 Input FIFO SW reset.
 * tm_fifo_ptr_sw_rst: 
 *     TM FIFOs Pointers SW reset.
 * non_delayed_out_fifo_sw_rst: 
 *     Non delayed output FIFO Pointers SW reset.
 * bb0_egr_msg_out_fifo_sw_rst: 
 *     BB0 free messages from egress OUTPUT FIFO SW reset.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_reset_ctrl_set(const qm_reset_ctrl *reset_ctrl);
bdmf_error_t ag_drv_qm_reset_ctrl_get(qm_reset_ctrl *reset_ctrl);

/**********************************************************************************************************************
 * read_clear_pkts: 
 *     Indicates whether the Drop/max_occupancy packets counter is read clear.
 * read_clear_bytes: 
 *     Indicates whether the Drop/max_occupancy bytes counter is read clear.
 * disable_wrap_around_pkts: 
 *     This bit defines the functionality of the drop packets counter.
 *     0 - Functions as the drop packets counter
 *     1 - Functions as the max packets occupancy holder
 * disable_wrap_around_bytes: 
 *     This bit defines the functionality of the drop bytes counter.
 *     0 - Functions as the drop bytes counter
 *     1 - Functions as the max bytes occupancy holder
 * free_with_context_last_search: 
 *     Indicates The value to put in the last_search field of the SBPM free with context message
 * wred_disable: 
 *     Disables WRED influence on drop condition
 * ddr_pd_congestion_disable: 
 *     Disables DDR_PD_CONGESTION influence on drop/bp
 *     condition
 * ddr_byte_congestion_disable: 
 *     Disables DDR_BYTE_CONGESTION influence on drop/bp condition
 * ddr_occupancy_disable: 
 *     Disables DDR_OCCUPANCY influence on drop/bp condition
 * ddr_fpm_congestion_disable: 
 *     Disables DDR_FPM_CONGESTION influence on drop/bp condition
 * fpm_ug_disable: 
 *     Disables FPM_UG influence on drop condition
 * queue_occupancy_ddr_copy_decision_disable: 
 *     Disables QUEUE_OCCUPANCY_DDR_COPY_DECISION influence on copy condition
 * psram_occupancy_ddr_copy_decision_disable: 
 *     Disables PSRAM_OCCUPANCY_DDR_COPY_DECISION influence on copy condition
 * dont_send_mc_bit_to_bbh: 
 *     When set, the multicast bit of the PD will not be sent to BBH TX
 * close_aggregation_on_timeout_disable: 
 *     When set, aggregations are not closed automatically when queue open aggregation time expired.
 * fpm_congestion_buf_release_mechanism_disable: 
 *     When cleared, given that there is an FPM congestion situation and all prefetch FPM buffers are full then a min
 *     pool size buffer will be freed each 1us. This is done due to the fact that exclusive indication is received
 *     only togeter with buffer allocation reply and if this will not be done then a deadlock could occur.
 *     Setting this bit will disable this mechanism.
 * fpm_buffer_global_res_enable: 
 *     FPM over subscription mechanism.
 *     Each queue will have one out of 8 reserved byte threshold profiles. Each profile defines 8 bit threshold with
 *     512byte resolution.
 *     Once the global FPM counter pass configurable threshold the system goes to buffer reservation congestion state.
 *     In this state any PD entering a queue which passes the reserved byte threshold will be dropped.
 *     
 * qm_preserve_pd_with_fpm: 
 *     Dont drop pd with fpm allocation.
 * qm_residue_per_queue: 
 *     Updated definition:
 *     1 - Use full residue memory - reset value
 *     0 - Debug only - use half size or no residue in projects where the full residue is 32B/q
 *     
 *     
 *     OLD Definition (perior 63146A0)
 *     6878:
 *     1 for 32B/Queue
 *     0 debug - no residue
 *     
 *     Other projects:
 *     0 for 64B/Queue
 *     1 for 128B/Queue
 * ghost_rpt_update_after_close_agg_en: 
 *     Controls the timing of updating the overhead counters with packets which goes through aggregation.
 *     
 *     0 - updates when the packets enters QM
 *     1 - updates when aggregation is done.
 * fpm_ug_flow_ctrl_disable: 
 *     Disables FPM_UG influence on flow control wake up messages to FW.
 * ddr_write_multi_slave_en: 
 *     Enables to write packet transaction to multiple slave (unlimited), if disable only one ubus slave allowed.
 * ddr_pd_congestion_agg_priority: 
 *     global priority bit to aggregated PDs which go through reprocessing.
 *     
 * psram_occupancy_drop_disable: 
 *     Disables PSRAM_OCCUPANCY_DROP influence on drop condition
 * qm_ddr_write_alignment: 
 *     0 According to length
 *     1 8-byte aligned
 * exclusive_dont_drop: 
 *     Controls if the exclusive indication in PD marks the PD as dont drop or as dont drop if the fpm in exclusive
 *     state
 *     1 - global dont drop
 *     0 - FPM exclusive state dont drop
 * dqmol_jira_973_fix_enable: 
 *     chicken bit for DQMOL bug:
 *     REPIN_D is full, QSM is locked by hw_push, prefetch locks UBUS.
 *     prefetch waits to QSM to unload repin_d but hw_push locks UBUS but cant since adjusted reqout_h is full because
 *     HOL is read req(offload or FPM req).
 *     
 *     previously - EXCLUSIVE_DONT_DROP_BP_EN
 *     
 *     when set 1 backpressure will be applied when the DONT_DROP pd should be dropped.
 *     for example, 0 fpm buffers available and the PD should be copied to DDR.
 * gpon_dbr_ceil: 
 *     If the bit enable, QM round up to 4 every packet length added ghost counters.
 * drop_cnt_wred_drops: 
 *     Drop counter counts WRED drops by color per queue.
 *     In order to enable this feature the drop counter should be configured to count drops. if the drop counter is
 *     configured count max occupancy per queue, it will override WRED drops count.
 *     color 0 - is written in dropped bytes field (word0)
 *     color 1 - is written in dropped pkts field (word1)
 * same_sec_lvl_bit_agg_en: 
 *     Uses bit 126 in aggregated PD to mark aggregation where all packets have the same second level queue.
 *     If set to 0, then aggregated PDs in DRAM bit 126 is the 9th bit of the fourth second level queue
 *     if set to 1, then aggregated PDs in DRAM bit 126 is same second level queue.
 * glbl_egr_drop_cnt_read_clear_enable: 
 *     Indicates whether the QM_GLOBAL_EGRESS_DROP_COUNTER counter is read clear.
 * glbl_egr_aqm_drop_cnt_read_clear_enable: 
 *     Indicates whether the QM_GLOBAL_EGRESS_AQM_DROP_COUNTER counter is read clear.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_drop_counters_ctrl_set(const qm_drop_counters_ctrl *drop_counters_ctrl);
bdmf_error_t ag_drv_qm_drop_counters_ctrl_get(qm_drop_counters_ctrl *drop_counters_ctrl);

/**********************************************************************************************************************
 * fpm_pool_bp_enable: 
 *     This field indicates whether crossing the per pool FPM buffer prefetch FIFO occupancy thresholds will result in
 *     dropping packets or in applying back pressure to the re-order.
 *     0 - drop packets
 *     1 - apply back pressure
 * fpm_congestion_bp_enable: 
 *     This field indicates whether crossing the FPM congestion threshold will result in dropping packets or in
 *     applying back pressure to the re-order.
 *     0 - drop packets
 *     1 - apply back pressure
 * fpm_force_bp_lvl: 
 *     Min pool occupancy which forces BP even if QM is working in drop mode. The purpose of this reg is to solve
 *     cases when exclusive_dont_drop packets exist in QM + pools are shallow. BP_EN = MIN(POOL0,POOL1,POOL2,POOL3) <
 *     FPM_FORCE_BP_LVL.
 *     When set to 0, BP isnt applied. It isnt recommended since a packet of an exclusive dont drop queue can receive
 *     dirty/used FPM buffer.
 * fpm_prefetch_granularity: 
 *     FPM_PREFETCH_MIN_POOL_SIZE granularity
 *     
 *     0 - 256B
 *     1 - 320B
 * fpm_prefetch_min_pool_size: 
 *     FPM prefetch minimum pool size.
 *     The supported FPM pool sizes are derived from this value:
 *     * FPM_PREFETCH_MIN_POOL_SIZEx1
 *     * FPM_PREFETCH_MIN_POOL_SIZEx2
 *     * FPM_PREFETCH_MIN_POOL_SIZEx4
 *     * FPM_PREFETCH_MIN_POOL_SIZEx8
 *     
 *     The optional values for this field (also depend on FPM_PREFETCH_GRANULARITY value) :
 *     0 - 256Byte  or 320Byte
 *     1 - 512Byte  or 640Byte
 *     2 - 1024Byte or 1280Byte
 *     3 - 2048Byte or 2560Byte
 *     
 *     
 *     
 * fpm_prefetch_pending_req_limit: 
 *     The allowed on the fly FPM prefetch pending Alloc requests to the FPM.
 * fpm_override_bb_id_en: 
 *     Enable FPM BB ID override with non default value
 * fpm_override_bb_id_value: 
 *     Value to override the default FPM BB ID.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_ctrl_set(const qm_fpm_ctrl *fpm_ctrl);
bdmf_error_t ag_drv_qm_fpm_ctrl_get(qm_fpm_ctrl *fpm_ctrl);

/**********************************************************************************************************************
 * total_pd_thr: 
 *     If the number of PDs for a certain queue exceeds this value, then PDs will be dropped.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_set(uint32_t total_pd_thr);
bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_get(uint32_t *total_pd_thr);

/**********************************************************************************************************************
 * abs_drop_queue: 
 *     Absolute address drop queue.
 *     Absolute address PDs which are dropped will be redirected into this configured queue. FW will be responsible
 *     for reclaiming their DDR space.
 * abs_drop_queue_en: 
 *     Absolute address drop queue enable.
 *     Enables the mechanism in which absolute address PDs which are dropped are be redirected into this configured
 *     queue. FW will be responsible for reclaiming their DDR space.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_set(uint16_t abs_drop_queue, bdmf_boolean abs_drop_queue_en);
bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_get(uint16_t *abs_drop_queue, bdmf_boolean *abs_drop_queue_en);

/**********************************************************************************************************************
 * max_agg_bytes: 
 *     This field indicates the maximum number of bytes in an aggregated PD.
 * max_agg_pkts: 
 *     This field indicates the maximum number of packets in an aggregated PD
 * agg_ovr_512b_en: 
 *     This feature, when enabled, allows QM to aggregate more than 512 in each aggregation.
 *     Max PD size allowed to be added to an aggregation will still remain 512 due to limitation in the PD struct.
 *     Note that the default value once this feature is enabled in 640Byte. This needs to be configured in the
 *     MAX_AGG_BYTES.
 *     MAX_PACKET_SIZE will still be 512
 *     
 * max_agg_pkt_size: 
 *     This indicates the maximum Packet size that can be aggregated.
 *     With current PD limitation max Packet can be 512.
 *     This is true even if the AGG_OVER_512BYTE_ENABLE is set to 1b1.
 * min_agg_pkt_size: 
 *     This indicates the minimum Packet size that can be aggregated.
 *     This is for the design to understand if the current accumulated agg size has enough space for another Packet.
 *     if it hasnt, it will close the agg. if it has it will agg and leave the aggregation open.
 *     
 *     
 *     Note!!
 *     You must config this value aligned to be to 8 (e.g for 60 byte min packet configure 64)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_set(const qm_global_cfg_aggregation_ctrl *global_cfg_aggregation_ctrl);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_get(qm_global_cfg_aggregation_ctrl *global_cfg_aggregation_ctrl);

/**********************************************************************************************************************
 * agg_pool_sel_en: 
 *     Enable pool selection for aggregarion according to AGG_POOL_SEL configuration and not according to
 *     min_buffer_size config
 * agg_pool_sel: 
 *     This register sets the the size of the pool to use for Agg Packets.
 *     Since the design can now support max AGG of any size up to 640 (and even till 1024) there should be an explicit
 *     configuration to which pool to use.
 *     
 *     -> POOL0 - Use 8 buffers
 *     -> POOL1 - Use 4 buffers
 *     -> POOL2 - Use 2 buffers
 *     -> POOL3 - Use 1 buffers
 *     
 *     This configuration needs to take into account the min_buffer_size and max_agg_size in order to make sure that
 *     the correct pool is chosen for agg
 *     
 *     e.g. min buffer size is 256 and max_agg_byte is 1016. will require pool of 4 buffers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl2_set(bdmf_boolean agg_pool_sel_en, uint8_t agg_pool_sel);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl2_get(bdmf_boolean *agg_pool_sel_en, uint8_t *agg_pool_sel);

/**********************************************************************************************************************
 * fpm_base_addr: 
 *     FPM Base Address. This is the 32-bit MSBs out of the 40-bit address.
 *     Multiply this field by 256 to get the 40-bit address.
 *     Example:
 *     If desired base address is 0x0080_0000
 *     The FPM_BASE_ADDR field should be configured to: 0x0000_8000.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_base_addr_set(uint32_t fpm_base_addr);
bdmf_error_t ag_drv_qm_fpm_base_addr_get(uint32_t *fpm_base_addr);

/**********************************************************************************************************************
 * fpm_base_addr: 
 *     FPM Base Address. This is the 32-bit MSBs out of the 40-bit address.
 *     Multiply this field by 256 to get the 40-bit address.
 *     Example:
 *     If desired base address is 0x0080_0000
 *     The FPM_BASE_ADDR field should be configured to: 0x0000_8000.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(uint32_t fpm_base_addr);
bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_get(uint32_t *fpm_base_addr);

/**********************************************************************************************************************
 * ddr_sop_offset0: 
 *     DDR SOP Offset option 0
 *     
 * ddr_sop_offset1: 
 *     DDR SOP Offset option 1
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_ddr_sop_offset_set(uint16_t ddr_sop_offset0, uint16_t ddr_sop_offset1);
bdmf_error_t ag_drv_qm_ddr_sop_offset_get(uint16_t *ddr_sop_offset0, uint16_t *ddr_sop_offset1);

/**********************************************************************************************************************
 * epon_line_rate: 
 *     EPON Line Rate
 *     0 - 1G
 *     1 - 10G
 * epon_crc_add_disable: 
 *     If this bit is not set then 4-bytes will be added to the ghost reporting accumulated bytes and to the byte
 *     overhead calculation input
 * mac_flow_overwrite_crc_en: 
 *     Enables to overwrite CRC addition specified MAC FLOW in the field below.
 * mac_flow_overwrite_crc: 
 *     MAC flow ID to force disable CRC addition
 * fec_ipg_length: 
 *     FEC IPG Length
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_epon_overhead_ctrl_set(const qm_epon_overhead_ctrl *epon_overhead_ctrl);
bdmf_error_t ag_drv_qm_epon_overhead_ctrl_get(qm_epon_overhead_ctrl *epon_overhead_ctrl);

/**********************************************************************************************************************
 * addr: 
 *     ADDR
 * bbhtx_req_otf: 
 *     BBHTX_REQ_OTF
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_bbhtx_fifo_addr_set(uint8_t addr, uint8_t bbhtx_req_otf);
bdmf_error_t ag_drv_qm_global_cfg_bbhtx_fifo_addr_get(uint8_t *addr, uint8_t *bbhtx_req_otf);

/**********************************************************************************************************************
 * queue_num: 
 *     Queue num
 * flush_en: 
 *     flush queue enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_set(uint16_t queue_num, bdmf_boolean flush_en);
bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_get(uint16_t *queue_num, bdmf_boolean *flush_en);

/**********************************************************************************************************************
 * prescaler_granularity: 
 *     defines the granularity of the prescaler counter:
 *     0 = 10bits
 *     1 = 11bits
 *     2 = 12bits
 *     3 = 13bits
 *     4 = 14bits
 *     5 = 15bits
 *     6 = 16bits
 *     7 = 17bits (to support 1GHz clk)
 *     
 *     
 * aggregation_timeout_value: 
 *     Aggregation timeout value, counted in prescaler counters cycles.
 *     valid values = [1..7]
 *     0 - isnt supported
 *     
 *     
 * pd_occupancy_en: 
 *     If set, aggregation of queues with PD occupancy more than encoded on PD_OCCUPANCY_VALUE arent closed even if
 *     the timer is expired.
 *     If not set, then aggregation is closed after queues timer expires
 * pd_occupancy_value: 
 *     if PD_OCCUPANCY_EN == 1 then
 *     Aggregations of queues with more than byte_occupacny of (PD_OCCUPNACY > 0) ?2 ^ (PD_OCCUPANCY + 5):0 are not
 *     closed on timeout.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(uint8_t prescaler_granularity, uint8_t aggregation_timeout_value, bdmf_boolean pd_occupancy_en, uint8_t pd_occupancy_value);
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(uint8_t *prescaler_granularity, uint8_t *aggregation_timeout_value, bdmf_boolean *pd_occupancy_en, uint8_t *pd_occupancy_value);

/**********************************************************************************************************************
 * fpm_gbl_cnt: 
 *     FPM global counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(uint16_t fpm_gbl_cnt);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(uint16_t *fpm_gbl_cnt);

/**********************************************************************************************************************
 * ddr_headroom: 
 *     DDR headroom space
 * ddr_tailroom: 
 *     DDR tailroom in bytes
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_ddr_spare_room_set(uint16_t pair_idx, uint16_t ddr_headroom, uint16_t ddr_tailroom);
bdmf_error_t ag_drv_qm_qm_ddr_spare_room_get(uint16_t pair_idx, uint16_t *ddr_headroom, uint16_t *ddr_tailroom);

/**********************************************************************************************************************
 * dummy_profile_0: 
 *     DDR dummy spare room profile 0
 * dummy_profile_1: 
 *     DDR dummy spare room profile 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_dummy_spare_room_profile_id_set(uint8_t dummy_profile_0, uint8_t dummy_profile_1);
bdmf_error_t ag_drv_qm_global_cfg_dummy_spare_room_profile_id_get(uint8_t *dummy_profile_0, uint8_t *dummy_profile_1);

/**********************************************************************************************************************
 * tkn_reqout_h: 
 *     token reqout hspace. less then this values FSM stays at idle
 * tkn_reqout_d: 
 *     token reqout dspace. less then this values FSM stays at idle
 * offload_reqout_h: 
 *     offload reqout hspace. less then this values FSM stays at idle
 * offload_reqout_d: 
 *     offload reqout dspace. less then this values FSM stays at idle
 *     
 *     IMPORTANT: RDPDEVEL-1057
 *     The correct value should be 4 however the reset value is 5. The correct value must to be written by SW.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_dqm_ubus_ctrl_set(uint8_t tkn_reqout_h, uint8_t tkn_reqout_d, uint8_t offload_reqout_h, uint8_t offload_reqout_d);
bdmf_error_t ag_drv_qm_global_cfg_dqm_ubus_ctrl_get(uint8_t *tkn_reqout_h, uint8_t *tkn_reqout_d, uint8_t *offload_reqout_h, uint8_t *offload_reqout_d);

/**********************************************************************************************************************
 * mem_init_en: 
 *     Memory auto init enable
 * mem_sel_init: 
 *     Select which memory to AUTO INIT
 *     
 *     3b000: qm_total_valid_counter
 *     3b001: qm_drop_counter
 *     3b010: qm_dqm_valid_counter
 *     3b011: qm_epon_rpt_cnt_counter
 *     3b111: All memoires
 * mem_size_init: 
 *     What is the size of the memory (according to the Number of Qs)
 *     
 *     3b000: 96
 *     3b001: 128
 *     3b010: 160
 *     3b011: 288
 *     3b100: 448
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_set(bdmf_boolean mem_init_en, uint8_t mem_sel_init, uint8_t mem_size_init);
bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_get(bdmf_boolean *mem_init_en, uint8_t *mem_sel_init, uint8_t *mem_size_init);

/**********************************************************************************************************************
 * mem_init_done: 
 *     Memory auto init done
 *     Bit is asserted when init is done.
 *     Bit is de-asserted when init starts
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_sts_get(bdmf_boolean *mem_init_done);

/**********************************************************************************************************************
 * pool_0_num_of_tkns: 
 *     Number of tokens used for each buffer in pool0
 * pool_1_num_of_tkns: 
 *     Number of tokens used for each buffer in pool1
 * pool_2_num_of_tkns: 
 *     Number of tokens used for each buffer in pool2
 * pool_3_num_of_tkns: 
 *     Number of tokens used for each buffer in pool3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_set(uint8_t pool_0_num_of_tkns, uint8_t pool_1_num_of_tkns, uint8_t pool_2_num_of_tkns, uint8_t pool_3_num_of_tkns);
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_get(uint8_t *pool_0_num_of_tkns, uint8_t *pool_1_num_of_tkns, uint8_t *pool_2_num_of_tkns, uint8_t *pool_3_num_of_tkns);

/**********************************************************************************************************************
 * pool_0_num_of_bytes: 
 *     Number of bytes used for each buffer in pool0 - Up to 16K
 * pool_1_num_of_bytes: 
 *     Number of Bytes used for each buffer in pool1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_set(uint16_t pool_0_num_of_bytes, uint16_t pool_1_num_of_bytes);
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_get(uint16_t *pool_0_num_of_bytes, uint16_t *pool_1_num_of_bytes);

/**********************************************************************************************************************
 * pool_2_num_of_bytes: 
 *     Number of bytes used for each buffer in pool0 - Up to 16K
 * pool_3_num_of_bytes: 
 *     Number of Bytes used for each buffer in pool1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_set(uint16_t pool_2_num_of_bytes, uint16_t pool_3_num_of_bytes);
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_get(uint16_t *pool_2_num_of_bytes, uint16_t *pool_3_num_of_bytes);

/**********************************************************************************************************************
 * mc_headers_pool_sel: 
 *     This register sets the size of the pool to use for MC headers.
 *     Since the 256K FPM PD Format does not have a field indicating the pool size, there should be an explicit
 *     configuration for which pool to use.
 *     
 *     -> POOL0 - Use 8 buffers
 *     -> POOL1 - Use 4 buffers
 *     -> POOL2 - Use 2 buffers
 *     -> POOL3 - Use 1 buffers
 *     
 *     This configuration needs to take into account the min_buffer_size in order to make sure that the correct pool
 *     is chosen for multicast headers.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_mc_ctrl_set(uint8_t mc_headers_pool_sel);
bdmf_error_t ag_drv_qm_global_cfg_mc_ctrl_get(uint8_t *mc_headers_pool_sel);

/**********************************************************************************************************************
 * value: 
 *     Clock counter cycle 16bit value. default is 1000d for 1uSec @1GHz clk.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_aqm_clk_counter_cycle_set(uint16_t value);
bdmf_error_t ag_drv_qm_global_cfg_aqm_clk_counter_cycle_get(uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     Global threshold value of queue fill level (ingress occupancy in bytes) to mark the PD Push to Empty bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_aqm_push_to_empty_thr_set(uint8_t value);
bdmf_error_t ag_drv_qm_global_cfg_aqm_push_to_empty_thr_get(uint8_t *value);

/**********************************************************************************************************************
 * egress_accumulated_cnt_pkts_read_clear_enable: 
 *     Indicates whether the egress accumulated total packets counter is read clear.
 * egress_accumulated_cnt_bytes_read_clear_enable: 
 *     Indicates whether the egress accumulated total bytes counter is read clear.
 * agg_closure_suspend_on_bp: 
 *     When set, closure due to aggregation timers will be suspended when BP is applied from CM pipe.
 * bufmng_en_or_ug_cntr: 
 *     When set, QM use BUFMNG instead of user-group counters - direct interface (no BB).
 *     Old mechanism remain for backup.
 *     1= enable bufmng
 *     0= use ug counters
 *     default is 1=bufmng
 * dqm_to_fpm_ubus_or_fpmini: 
 *     1b configuration, selecting between access of dqm to fpm through ubus (old mode), and access to fpmini (new).
 *     Default is 1 (fpmini new mode).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_general_ctrl2_set(const qm_global_cfg_qm_general_ctrl2 *global_cfg_qm_general_ctrl2);
bdmf_error_t ag_drv_qm_global_cfg_qm_general_ctrl2_get(qm_global_cfg_qm_general_ctrl2 *global_cfg_qm_general_ctrl2);

/**********************************************************************************************************************
 * lower_thr: 
 *     FPM Lower Threshold.
 *     When working in packet drop mode (FPM_BP_ENABLE=0), Then:
 *     * If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high priority are dropped (only exclusive
 *     packets are not dropped).
 *     * If (FPM_LOWER_THR) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped.
 *     * If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.
 *     When working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOWER_THR), then
 *     backpressure is applied to re-order (in this case FPM_HIGHER_THR is dont care).
 * higher_thr: 
 *     FPM Higher Threshold.
 *     When working in packet drop mode (FPM_BP_ENABLE=0), Then:
 *     * If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high priority are dropped (only exclusive
 *     packets are not dropped).
 *     * If (FPM_LOWER_THR) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped.
 *     * If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.
 *     When working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOWER_THR), then
 *     backpressure is applied to re-order (in this case FPM_HIGHER_THR is dont care).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_pool_thr_set(uint8_t pool_idx, const qm_fpm_pool_thr *fpm_pool_thr);
bdmf_error_t ag_drv_qm_fpm_pool_thr_get(uint8_t pool_idx, qm_fpm_pool_thr *fpm_pool_thr);

/**********************************************************************************************************************
 * fpm_ug_cnt: 
 *     FPM user group counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_ug_cnt_set(uint8_t grp_idx, uint32_t fpm_ug_cnt);
bdmf_error_t ag_drv_qm_fpm_ug_cnt_get(uint8_t grp_idx, uint32_t *fpm_ug_cnt);

/**********************************************************************************************************************
 * qm_dqm_pop_on_empty: 
 *     HW tried to pop a PD from the DQM of an empty queue.
 * qm_dqm_push_on_full: 
 *     HW tried to pop a PD into the DQM of a full queue.
 * qm_cpu_pop_on_empty: 
 *     CPU tried to pop a PD from the DQM of an empty queue.
 * qm_cpu_push_on_full: 
 *     CPU tried to push a PD into the DQM of a full queue.
 * qm_normal_queue_pd_no_credit: 
 *     A PD arrived to the Normal queue without having any credits
 * qm_non_delayed_queue_pd_no_credit: 
 *     A PD arrived to the NON-delayed queue without having any credits
 * qm_non_valid_queue: 
 *     A PD arrived with a non valid queue number (>287)
 * qm_agg_coherent_inconsistency: 
 *     An aggregation of PDs was done in which the coherent bit of the PD differs between them (The coherent bit of
 *     the first aggregated PD was used)
 * qm_force_copy_on_non_delayed: 
 *     A PD with force copy bit set was received on the non-delayed queue (in this queue the copy machine is bypassed)
 * qm_fpm_pool_size_nonexistent: 
 *     A PD was marked to be copied, but there does not exist an FPM pool buffer large enough to hold it.
 * qm_target_mem_abs_contradiction: 
 *     A PD was marked with a target_mem=1 (located in PSRAM) and on the other hand, the absolute address indication
 *     was set.
 * qm_1588_drop: 
 *     1588 Packet is dropped when the QM PD occupancy exceeds threshold (64K)
 * qm_1588_multicast_contradiction: 
 *     A PD was marked as a 1588 and multicast together.
 * qm_byte_drop_cnt_overrun: 
 *     The byte drop counter of one of the queues reached its maximum value and a new value was pushed.
 * qm_pkt_drop_cnt_overrun: 
 *     The Packet drop counter of one of the queues reached its maximum value and a new value was pushed.
 * qm_total_byte_cnt_underrun: 
 *     The Total byte counter was decremented to a negative value.
 * qm_total_pkt_cnt_underrun: 
 *     The Total PD counter was decremented to a negative value.
 * qm_fpm_ug0_underrun: 
 *     The UG0 counter was decremented to a negative value.
 * qm_fpm_ug1_underrun: 
 *     The UG1 counter was decremented to a negative value.
 * qm_fpm_ug2_underrun: 
 *     The UG2 counter was decremented to a negative value.
 * qm_fpm_ug3_underrun: 
 *     The UG3 counter was decremented to a negative value.
 * qm_timer_wraparound: 
 *     QM aggregation timers wraps around. In this case it isnt guaranteed that the aggregation will be closed on
 *     pre-defined timeout expiration. However the aggregation should be closed eventually.
 *     
 * qm_copy_plen_zero: 
 *     Packet with length = 0 is copied to DDR. FW/SW should take care that zero length packets arent copied to DDR.
 *     
 * qm_ingress_bb_unexpected_msg: 
 *     Unexpected Message arrived at QM ingress BB.
 *     
 * qm_egress_bb_unexpected_msg: 
 *     Unexpected Message arrived at QM egress BB.
 *     
 * dqm_reached_full: 
 *     DQM reached a full condition (used for debug).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_intr_ctrl_isr_set(const qm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_qm_intr_ctrl_isr_get(qm_intr_ctrl_isr *intr_ctrl_isr);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_intr_ctrl_ism_get(uint32_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_qm_intr_ctrl_ier_get(uint32_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_qm_intr_ctrl_itr_get(uint32_t *ist);

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
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_set(const qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_get(qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);

/**********************************************************************************************************************
 * queue_num: 
 *     Queue Number
 * cmd: 
 *     Command:
 *     00 - Nothing
 *     01 - Write
 *     10 - Read
 *     11 - Read No commit (entry not popped)
 *     
 *     Will trigger a read/write from the selected RAM
 *     
 *     IMPORTANT: Read is for debug purpose only. shouldnt be used during regular QM work on the requested queue (HW
 *     pop).
 *     Popping the same queue both from CPU and HW could cause to race condition which will cause to incorrect data
 *     output. It could occur when there is only one entry in the queue which is accessed both from the CPU and the HW.
 * done: 
 *     Indicates that read/write to DQM is done
 * error: 
 *     Indicates that that an error occured (write on full or read on empty)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(uint8_t indirect_grp_idx, uint16_t queue_num, uint8_t cmd, bdmf_boolean done, bdmf_boolean error);
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(uint8_t indirect_grp_idx, uint16_t *queue_num, uint8_t *cmd, bdmf_boolean *done, bdmf_boolean *error);

/**********************************************************************************************************************
 * wred_profile: 
 *     Defines to which WRED Profile this queue belongs to.
 * copy_dec_profile: 
 *     Defines to which Copy Decision Profile this queue belongs.
 *     profile 3d7 is copy_to_ddr: always copy to DDR
 * ddr_copy_disable: 
 *     Defines this queue never to copy to DDR.
 * aggregation_disable: 
 *     Defines this queue never to aggregated PDs.
 * fpm_ug_or_bufmng: 
 *     Defines to which FPM UG or BUFMNG this queue belongs.
 *     Use BUFMNG or UG according to the config bit: BUFMNG_EN_OR_UG_CNTR in QM_GLOBAL_CFG.QM_GENERAL_CTRL2
 * exclusive_priority: 
 *     Defines this queue with exclusive priority.
 * q_802_1ae: 
 *     Defines this queue as 802.1AE for EPON packet overhead calculations.
 * sci: 
 *     Configures SCI for EPON packet overhead calculations.
 * fec_enable: 
 *     FEC enable configuration for EPON packet overhead calculations.
 * res_profile: 
 *     FPM reservation profile.
 *     Once the QM goes over global FPM reservation threshold.
 *     Queue with more bytes the defined in the profile will be dropped.
 *     Profile 0 means no drop due to FPM reservation for the queues with this profile.
 * spare_room_0: 
 *     Select 1 out of 4 headroom and tailroom pairs. if PDs SOP select is set to 0. This profile is chosen
 * spare_room_1: 
 *     Select 1 out of 4 headroom and tailroom pairs. if PDs SOP select is set to 1. This profile is chosen
 * service_queue_profile: 
 *     17-options, 16 for the service queue and an extra one for using its own queue (sort of a service-queue disable
 *     bit)
 * timestamp_res_profile: 
 *     Select which profile this queue belongs to. 1 out of 2 resolutions profiles, for 10b out of 32b timestamp
 *     resolution in the PD:
 *     0d = profile0
 *     1d = profile1
 *     2d = profile2
 *     3d = profile3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_q_context_set(uint16_t q_idx, const qm_q_context *q_context);
bdmf_error_t ag_drv_qm_q_context_get(uint16_t q_idx, qm_q_context *q_context);

/**********************************************************************************************************************
 * queue_occupancy_thr: 
 *     Queue Occupancy Threshold.
 *     When passing this threhold, packets will be copied to the DDR
 * psram_thr: 
 *     Indicates which of the two PSRAM threshold crossing indications coming from the SBPM will be used for the copy
 *     decision. when going over the chosen threshold, packets will be copied to the DDR.
 *     0 - Lower threshold
 *     1 - Higher threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_copy_decision_profile_set(uint8_t profile_idx, uint32_t queue_occupancy_thr, bdmf_boolean psram_thr);
bdmf_error_t ag_drv_qm_copy_decision_profile_get(uint8_t profile_idx, uint32_t *queue_occupancy_thr, bdmf_boolean *psram_thr);

/**********************************************************************************************************************
 * start: 
 *     AQM Timestamp resolution Profile, 4 profiles to select which 10bits of the timestamp counter 32bit will be
 *     written to the PD.
 *     Value represents the start of the 10bit offset in the 32bit:
 *     0d = [9:0], 1d = [10:1], 2d = [11:2] , 22d = [31:22]
 *     total 23 values (0d-22d).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_timestamp_res_profile_set(uint8_t profile_idx, uint8_t start);
bdmf_error_t ag_drv_qm_timestamp_res_profile_get(uint8_t profile_idx, uint8_t *start);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_egress_drop_counter_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_egress_aqm_drop_counter_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_total_valid_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_total_valid_cnt_get(uint16_t q_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_valid_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_dqm_valid_cnt_get(uint16_t q_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_drop_counter_get(uint16_t q_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_accumulated_counter_get(uint32_t q_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_epon_q_byte_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_epon_q_byte_cnt_get(uint16_t q_idx, uint32_t *data);

/**********************************************************************************************************************
 * status_bit_vector: 
 *     Status bit vector - a bit per queue indicates if the queue has been updated.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_epon_q_status_get(uint16_t q_idx, uint32_t *status_bit_vector);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_rd_data_pool0_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_rd_data_pool1_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_rd_data_pool2_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_rd_data_pool3_get(uint32_t *data);

/**********************************************************************************************************************
 * pop_pool0: 
 *     Pop FIFO entry
 * pop_pool1: 
 *     Pop FIFO entry
 * pop_pool2: 
 *     Pop FIFO entry
 * pop_pool3: 
 *     Pop FIFO entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pop_3_set(bdmf_boolean pop_pool0, bdmf_boolean pop_pool1, bdmf_boolean pop_pool2, bdmf_boolean pop_pool3);
bdmf_error_t ag_drv_qm_pop_3_get(bdmf_boolean *pop_pool0, bdmf_boolean *pop_pool1, bdmf_boolean *pop_pool2, bdmf_boolean *pop_pool3);

/**********************************************************************************************************************
 * wr_ptr: 
 *     PDFIFO WR pointers
 * rd_ptr: 
 *     PDFIFO RD pointers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pdfifo_ptr_get(uint16_t q_idx, uint8_t *wr_ptr, uint8_t *rd_ptr);

/**********************************************************************************************************************
 * wr_ptr: 
 *     UF WR pointers
 * rd_ptr: 
 *     UF RD pointers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_update_fifo_ptr_get(uint16_t fifo_idx, uint16_t *wr_ptr, uint8_t *rd_ptr);

/**********************************************************************************************************************
 * pop: 
 *     Pop FIFO entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pop_2_set(bdmf_boolean pop);
bdmf_error_t ag_drv_qm_pop_2_get(bdmf_boolean *pop);

/**********************************************************************************************************************
 * pop: 
 *     Pop FIFO entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pop_1_set(bdmf_boolean pop);
bdmf_error_t ag_drv_qm_pop_1_get(bdmf_boolean *pop);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb0_egr_msg_out_fifo_data_get(uint32_t idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_set(uint32_t idx, uint32_t data);
bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_get(uint32_t idx, uint32_t *data);

/**********************************************************************************************************************
 * en_byte: 
 *     Enable flow control on byte occupancy
 * en_ug: 
 *     Enable flow control according to byte occupancy
 * bbh_rx_bb_id: 
 *     BB ID to which Xoff/Xon is sent.
 *     Design assumption:
 * fw_port_id: 
 *     FW_ID compared to PD to decide whether to send Xoff/Xon.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_port_cfg_set(uint8_t idx, bdmf_boolean en_byte, bdmf_boolean en_ug, uint8_t bbh_rx_bb_id, uint8_t fw_port_id);
bdmf_error_t ag_drv_qm_port_cfg_get(uint8_t idx, bdmf_boolean *en_byte, bdmf_boolean *en_ug, uint8_t *bbh_rx_bb_id, uint8_t *fw_port_id);

/**********************************************************************************************************************
 * ug_en: 
 *     UG/BUFMNG participates in FC (0-31)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fc_ug_mask_ug_en_set(uint32_t ug_en);
bdmf_error_t ag_drv_qm_fc_ug_mask_ug_en_get(uint32_t *ug_en);

/**********************************************************************************************************************
 * queue_vec: 
 *     each bit represents queue.
 *     1 - fc is enabled for this queue (unmasked)
 *     0 - fc is disabled for this queue (masked)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fc_queue_mask_set(uint8_t idx, uint32_t queue_vec);
bdmf_error_t ag_drv_qm_fc_queue_mask_get(uint8_t idx, uint32_t *queue_vec);

/**********************************************************************************************************************
 * start_queue: 
 *     First queue of range1 for which FC will be applied.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fc_queue_range1_start_set(uint16_t start_queue);
bdmf_error_t ag_drv_qm_fc_queue_range1_start_get(uint16_t *start_queue);

/**********************************************************************************************************************
 * start_queue: 
 *     First queue of range1 for which FC will be applied.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fc_queue_range2_start_set(uint16_t start_queue);
bdmf_error_t ag_drv_qm_fc_queue_range2_start_get(uint16_t *start_queue);

/**********************************************************************************************************************
 * status: 
 *     1 - exceeds thr
 *     0 - below thr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dbg_get(uint32_t *status);

/**********************************************************************************************************************
 * status: 
 *     queue
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_ug_occupancy_status_get(uint32_t q_idx, uint32_t *status);

/**********************************************************************************************************************
 * status: 
 *     1 - exceeds thr
 *     0 - below thr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_queue_range1_occupancy_status_get(uint8_t idx, uint32_t *status);

/**********************************************************************************************************************
 * status: 
 *     1 - exceeds thr
 *     0 - below thr
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_queue_range2_occupancy_status_get(uint8_t idx, uint32_t *status);

/**********************************************************************************************************************
 * select: 
 *     Counter
 * enable: 
 *     Enable register controlled debug select
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_debug_sel_set(uint8_t select, bdmf_boolean enable);
bdmf_error_t ag_drv_qm_debug_sel_get(uint8_t *select, bdmf_boolean *enable);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_debug_bus_lsb_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_debug_bus_msb_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_spare_config_get(uint32_t *data);

/**********************************************************************************************************************
 * good_lvl1_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_good_lvl1_pkts_cnt_get(uint32_t *good_lvl1_pkts);

/**********************************************************************************************************************
 * good_lvl1_bytes: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_good_lvl1_bytes_cnt_get(uint32_t *good_lvl1_bytes);

/**********************************************************************************************************************
 * good_lvl2_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_good_lvl2_pkts_cnt_get(uint32_t *good_lvl2_pkts);

/**********************************************************************************************************************
 * good_lvl2_bytes: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_good_lvl2_bytes_cnt_get(uint32_t *good_lvl2_bytes);

/**********************************************************************************************************************
 * copied_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_copied_pkts_cnt_get(uint32_t *copied_pkts);

/**********************************************************************************************************************
 * copied_bytes: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_copied_bytes_cnt_get(uint32_t *copied_bytes);

/**********************************************************************************************************************
 * agg_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_agg_pkts_cnt_get(uint32_t *agg_pkts);

/**********************************************************************************************************************
 * agg_bytes: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_agg_bytes_cnt_get(uint32_t *agg_bytes);

/**********************************************************************************************************************
 * agg1_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_agg_1_pkts_cnt_get(uint32_t *agg1_pkts);

/**********************************************************************************************************************
 * agg2_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_agg_2_pkts_cnt_get(uint32_t *agg2_pkts);

/**********************************************************************************************************************
 * agg3_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_agg_3_pkts_cnt_get(uint32_t *agg3_pkts);

/**********************************************************************************************************************
 * agg4_pkts: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_agg_4_pkts_cnt_get(uint32_t *agg4_pkts);

/**********************************************************************************************************************
 * wred_drop: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_wred_drop_cnt_get(uint32_t *wred_drop);

/**********************************************************************************************************************
 * fpm_cong: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_congestion_drop_cnt_get(uint32_t *fpm_cong);

/**********************************************************************************************************************
 * ddr_pd_cong_drop: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_ddr_pd_congestion_drop_cnt_get(uint32_t *ddr_pd_cong_drop);

/**********************************************************************************************************************
 * ddr_cong_byte_drop: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_ddr_byte_congestion_drop_cnt_get(uint32_t *ddr_cong_byte_drop);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_pd_congestion_drop_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * abs_requeue: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_abs_requeue_cnt_get(uint32_t *abs_requeue);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo0_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo1_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo2_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo3_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_normal_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_non_delayed_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_non_delayed_out_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pre_cm_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_rd_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_wr_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_common_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb0_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb1_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb1_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_egress_data_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_egress_rr_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * ovr_en: 
 *     BB rout address decode Override enable
 * dest_id: 
 *     Destination ID
 * route_addr: 
 *     Route Address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb_route_ovr_set(uint8_t idx, bdmf_boolean ovr_en, uint8_t dest_id, uint16_t route_addr);
bdmf_error_t ag_drv_qm_bb_route_ovr_get(uint8_t idx, bdmf_boolean *ovr_en, uint8_t *dest_id, uint16_t *route_addr);

/**********************************************************************************************************************
 * ingress_stat: 
 *     Stat
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_ingress_stat_get(uint32_t *ingress_stat);

/**********************************************************************************************************************
 * egress_stat: 
 *     Stat
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_egress_stat_get(uint32_t *egress_stat);

/**********************************************************************************************************************
 * cm_stat: 
 *     Stat
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_stat_get(uint32_t *cm_stat);

/**********************************************************************************************************************
 * fpm_prefetch_stat: 
 *     Stat
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_prefetch_stat_get(uint32_t *fpm_prefetch_stat);

/**********************************************************************************************************************
 * connect_ack_counter: 
 *     Pending SBPM Connect ACKs counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_connect_ack_counter_get(uint8_t *connect_ack_counter);

/**********************************************************************************************************************
 * ddr_wr_reply_counter: 
 *     Pending DDR WR Replies counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_ddr_wr_reply_counter_get(uint8_t *ddr_wr_reply_counter);

/**********************************************************************************************************************
 * ddr_pipe: 
 *     Pending bytes to be copied to the DDR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_ddr_pipe_byte_counter_get(uint32_t *ddr_pipe);

/**********************************************************************************************************************
 * requeue_valid: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_abs_requeue_valid_counter_get(uint16_t *requeue_valid);

/**********************************************************************************************************************
 * pd: 
 *     PD
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_illegal_pd_capture_get(uint32_t idx, uint32_t *pd);

/**********************************************************************************************************************
 * pd: 
 *     PD
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_ingress_processed_pd_capture_get(uint32_t idx, uint32_t *pd);

/**********************************************************************************************************************
 * pd: 
 *     PD
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_cm_processed_pd_capture_get(uint32_t idx, uint32_t *pd);

/**********************************************************************************************************************
 * fpm_grp_drop: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_grp_drop_cnt_get(uint32_t idx, uint32_t *fpm_grp_drop);

/**********************************************************************************************************************
 * fpm_drop: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_pool_drop_cnt_get(uint32_t idx, uint32_t *fpm_drop);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_buffer_res_drop_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_psram_egress_cong_drp_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * status: 
 *     STATUS:
 *     
 *     Back pressure sets the relevant register per bp reason. SW should write-clear in order to unset:
 *     
 *     0x1 - fpm exclusive threshold
 *     0x2 - fpm prefetch occupancy
 *     0x4 - DDR byte on the fly byte count threshold is exceeded
 *     0x8 - PD count in copy machine is exceeded
 *     
 *     Any permutation of the above may occur. especially if the value isnt cleared every time.
 *     
 *     
 *     A write of 0xFFFF_FFFF in order at reset this indication.
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_backpressure_set(uint8_t status);
bdmf_error_t ag_drv_qm_backpressure_get(uint8_t *status);

/**********************************************************************************************************************
 * value: 
 *     AQM timestamp current 32bit counter value for debug
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_aqm_timestamp_curr_counter_get(uint32_t *value);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb0_egr_msg_out_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_residue_data_get(uint16_t idx, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_qm_ddr_cong_ctrl,
    cli_qm_is_queue_not_empty,
    cli_qm_is_queue_pop_ready,
    cli_qm_is_queue_full,
    cli_qm_fpm_ug_thr,
    cli_qm_rnr_group_cfg,
    cli_qm_cpu_pd_indirect_wr_data,
    cli_qm_cpu_pd_indirect_rd_data,
    cli_qm_aggr_context,
    cli_qm_wred_profile_cfg,
    cli_qm_enable_ctrl,
    cli_qm_reset_ctrl,
    cli_qm_drop_counters_ctrl,
    cli_qm_fpm_ctrl,
    cli_qm_qm_pd_cong_ctrl,
    cli_qm_global_cfg_abs_drop_queue,
    cli_qm_global_cfg_aggregation_ctrl,
    cli_qm_global_cfg_aggregation_ctrl2,
    cli_qm_fpm_base_addr,
    cli_qm_global_cfg_fpm_coherent_base_addr,
    cli_qm_ddr_sop_offset,
    cli_qm_epon_overhead_ctrl,
    cli_qm_global_cfg_bbhtx_fifo_addr,
    cli_qm_global_cfg_qm_egress_flush_queue,
    cli_qm_global_cfg_qm_aggregation_timer_ctrl,
    cli_qm_global_cfg_qm_fpm_ug_gbl_cnt,
    cli_qm_qm_ddr_spare_room,
    cli_qm_global_cfg_dummy_spare_room_profile_id,
    cli_qm_global_cfg_dqm_ubus_ctrl,
    cli_qm_global_cfg_mem_auto_init,
    cli_qm_global_cfg_mem_auto_init_sts,
    cli_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens,
    cli_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte,
    cli_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte,
    cli_qm_global_cfg_mc_ctrl,
    cli_qm_global_cfg_aqm_clk_counter_cycle,
    cli_qm_global_cfg_aqm_push_to_empty_thr,
    cli_qm_global_cfg_qm_general_ctrl2,
    cli_qm_fpm_pool_thr,
    cli_qm_fpm_ug_cnt,
    cli_qm_intr_ctrl_isr,
    cli_qm_intr_ctrl_ism,
    cli_qm_intr_ctrl_ier,
    cli_qm_intr_ctrl_itr,
    cli_qm_clk_gate_clk_gate_cntrl,
    cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl,
    cli_qm_q_context,
    cli_qm_copy_decision_profile,
    cli_qm_timestamp_res_profile,
    cli_qm_global_egress_drop_counter,
    cli_qm_global_egress_aqm_drop_counter,
    cli_qm_total_valid_cnt,
    cli_qm_dqm_valid_cnt,
    cli_qm_drop_counter,
    cli_qm_accumulated_counter,
    cli_qm_epon_q_byte_cnt,
    cli_qm_epon_q_status,
    cli_qm_rd_data_pool0,
    cli_qm_rd_data_pool1,
    cli_qm_rd_data_pool2,
    cli_qm_rd_data_pool3,
    cli_qm_pop_3,
    cli_qm_pdfifo_ptr,
    cli_qm_update_fifo_ptr,
    cli_qm_pop_2,
    cli_qm_pop_1,
    cli_qm_bb0_egr_msg_out_fifo_data,
    cli_qm_fpm_buffer_reservation_data,
    cli_qm_port_cfg,
    cli_qm_fc_ug_mask_ug_en,
    cli_qm_fc_queue_mask,
    cli_qm_fc_queue_range1_start,
    cli_qm_fc_queue_range2_start,
    cli_qm_dbg,
    cli_qm_ug_occupancy_status,
    cli_qm_queue_range1_occupancy_status,
    cli_qm_queue_range2_occupancy_status,
    cli_qm_debug_sel,
    cli_qm_debug_bus_lsb,
    cli_qm_debug_bus_msb,
    cli_qm_qm_spare_config,
    cli_qm_good_lvl1_pkts_cnt,
    cli_qm_good_lvl1_bytes_cnt,
    cli_qm_good_lvl2_pkts_cnt,
    cli_qm_good_lvl2_bytes_cnt,
    cli_qm_copied_pkts_cnt,
    cli_qm_copied_bytes_cnt,
    cli_qm_agg_pkts_cnt,
    cli_qm_agg_bytes_cnt,
    cli_qm_agg_1_pkts_cnt,
    cli_qm_agg_2_pkts_cnt,
    cli_qm_agg_3_pkts_cnt,
    cli_qm_agg_4_pkts_cnt,
    cli_qm_wred_drop_cnt,
    cli_qm_fpm_congestion_drop_cnt,
    cli_qm_ddr_pd_congestion_drop_cnt,
    cli_qm_ddr_byte_congestion_drop_cnt,
    cli_qm_qm_pd_congestion_drop_cnt,
    cli_qm_qm_abs_requeue_cnt,
    cli_qm_fpm_prefetch_fifo0_status,
    cli_qm_fpm_prefetch_fifo1_status,
    cli_qm_fpm_prefetch_fifo2_status,
    cli_qm_fpm_prefetch_fifo3_status,
    cli_qm_normal_rmt_fifo_status,
    cli_qm_non_delayed_rmt_fifo_status,
    cli_qm_non_delayed_out_fifo_status,
    cli_qm_pre_cm_fifo_status,
    cli_qm_cm_rd_pd_fifo_status,
    cli_qm_cm_wr_pd_fifo_status,
    cli_qm_cm_common_input_fifo_status,
    cli_qm_bb0_output_fifo_status,
    cli_qm_bb1_output_fifo_status,
    cli_qm_bb1_input_fifo_status,
    cli_qm_egress_data_fifo_status,
    cli_qm_egress_rr_fifo_status,
    cli_qm_bb_route_ovr,
    cli_qm_ingress_stat,
    cli_qm_egress_stat,
    cli_qm_cm_stat,
    cli_qm_fpm_prefetch_stat,
    cli_qm_qm_connect_ack_counter,
    cli_qm_qm_ddr_wr_reply_counter,
    cli_qm_qm_ddr_pipe_byte_counter,
    cli_qm_qm_abs_requeue_valid_counter,
    cli_qm_qm_illegal_pd_capture,
    cli_qm_qm_ingress_processed_pd_capture,
    cli_qm_qm_cm_processed_pd_capture,
    cli_qm_fpm_grp_drop_cnt,
    cli_qm_fpm_pool_drop_cnt,
    cli_qm_fpm_buffer_res_drop_cnt,
    cli_qm_psram_egress_cong_drp_cnt,
    cli_qm_backpressure,
    cli_qm_aqm_timestamp_curr_counter,
    cli_qm_bb0_egr_msg_out_fifo_status,
    cli_qm_cm_residue_data,
};

int bcm_qm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_qm_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
