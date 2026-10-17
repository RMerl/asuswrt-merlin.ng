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
    uint16_t ddr_pipe_lower_thr;
    uint16_t ddr_pipe_higher_thr;
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
    uint16_t rnr_bb_id;
    uint8_t rnr_task;
    bdmf_boolean rnr_enable;
    uint16_t gsu_bb_id;
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
    bdmf_boolean ingress_pipe_arbiter_rr_mode_enable;
    bdmf_boolean cm_ddr_sop_offset_inc_enable;
    bdmf_boolean esbpm_buf_cnt_disable;
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
    bdmf_boolean bb0_ing_free_msg_fifo_sw_rst;
    bdmf_boolean dqm_pop_pd_data_fifo_sw_rst;
    bdmf_boolean dqm_ififo_egr_sw_rst;
    bdmf_boolean dqm_ofifo_egr_sw_rst;
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
    bdmf_boolean close_aggregation_on_mirror_change_en;
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
    uint16_t fpm_override_bb_id_value;
    bdmf_boolean fpm_elementry_token_sel;
} qm_fpm_ctrl;

typedef struct
{
    bdmf_boolean epon_line_rate;
    bdmf_boolean epon_crc_add_disable;
    bdmf_boolean mac_flow_overwrite_crc_en;
    uint8_t mac_flow_overwrite_crc;
    bdmf_boolean epon_8021ae;
    bdmf_boolean epon_sci;
    bdmf_boolean epon_fec_enable;
    uint16_t fec_ipg_length;
} qm_epon_overhead_ctrl;

typedef struct
{
    bdmf_boolean egress_accumulated_cnt_pkts_read_clear_enable;
    bdmf_boolean egress_accumulated_cnt_bytes_read_clear_enable;
    bdmf_boolean agg_closure_suspend_on_bp;
    bdmf_boolean bufmng_en_or_ug_cntr;
    bdmf_boolean dqm_to_fpm_ubus_or_fpmini;
    bdmf_boolean agg_closure_suspend_on_fpm_congestion_disable;
    bdmf_boolean epsram_buffer_size;
    bdmf_boolean agg_closure_suspend_on_stall;
    bdmf_boolean esbpm_prefetch_bp_en;
    bdmf_boolean esbpm_empty_bp_en;
    uint8_t esbpm_prefetch_bp_thr;
    bdmf_boolean egress_drop_per_queue_cnt_en;
    uint16_t sec_bn_fifo_usedw_bp_thr;
    bdmf_boolean force_epsram_bufmng_en;
    bdmf_boolean split_ddr_epsram_en;
    bdmf_boolean copy_to_ddr_on_qtc_dis;
    bdmf_boolean mixed_copy_queue_en;
    bdmf_boolean qm_is_upstream;
} qm_global_cfg_qm_general_ctrl2;

typedef struct
{
    uint8_t maxreq;
    bdmf_boolean enable;
    bdmf_boolean reset;
    uint16_t bbid;
    bdmf_boolean destovrd;
} qm_esbpm_ctrl;

typedef struct
{
    uint8_t lower_thr;
    uint8_t higher_thr;
} qm_fpm_pool_thr;

typedef struct
{
    bdmf_boolean qm_timer_wraparound0;
    bdmf_boolean qm_timer_wraparound1;
    bdmf_boolean qm_timer_wraparound2;
    bdmf_boolean qm_timer_wraparound3;
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
    bdmf_boolean qm_copy_plen_zero;
    bdmf_boolean qm_ingress_bb_unexpected_msg;
    bdmf_boolean qm_egress_bb_unexpected_msg;
    bdmf_boolean dqm_reached_full;
    bdmf_boolean qm_fpmini_intr;
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
    bdmf_boolean total_egress_accum_cnt_pkt;
    bdmf_boolean global_egress_drop_cnt_pkt;
    bdmf_boolean drop_ing_egr_cnt_pkt;
    bdmf_boolean fpm_grp_drop_cnt_pkt;
    bdmf_boolean qm_pd_congestion_drop_cnt_pkt;
    bdmf_boolean ddr_pd_congestion_drop_cnt_pkt;
    bdmf_boolean wred_drop_cnt_pkt;
    bdmf_boolean good_lvl2_pkts_cnt_pkt;
    bdmf_boolean good_lvl1_pkts_cnt_pkt;
    bdmf_boolean qm_total_valid_cnt_pkt;
    bdmf_boolean dqm_valid_cnt_pkt;
    bdmf_boolean psram_drop_cnt_pkt;
} qm_count_pkt_not_pd_mode_bits;

typedef struct
{
    uint8_t egr0_wrr_stat;
    uint8_t egr1_wrr_stat;
    uint8_t egr2_wrr_stat;
    uint8_t egr3_wrr_stat;
    uint8_t egr4_wrr_stat;
    uint8_t egr5_wrr_stat;
    uint8_t egr6_wrr_stat;
    uint8_t egr7_wrr_stat;
} qm_cm_egr_wrr_status;

typedef struct
{
    uint16_t queue;
    bdmf_boolean queue_all;
    uint8_t stall_type;
    bdmf_boolean stall_all;
    bdmf_boolean dbg_en;
} qm_dqm_dbg_cntr_cfg;

typedef struct
{
    uint8_t wred_profile;
    uint8_t copy_dec_profile;
    uint8_t fpm_ug_or_bufmng;
    bdmf_boolean exclusive_priority;
    uint8_t res_profile;
    uint8_t timestamp_res_profile;
    uint8_t aggregation_profile;
    bdmf_boolean ddr_copy_disable;
    bdmf_boolean aggregation_disable;
    uint8_t spare_room_1;
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
 * to_val0: 
 *     Timeout value
 * timer0: 
 *     Choose one of 4 timers (each has its own prescalar resolution)
 * to_val1: 
 *     Timeout value
 * timer1: 
 *     Choose one of 4 timers (each has its own prescalar resolution)
 * to_val2: 
 *     Timeout value
 * timer2: 
 *     Choose one of 4 timers (each has its own prescalar resolution)
 * to_val3: 
 *     Timeout value
 * timer3: 
 *     Choose one of 4 timers (each has its own prescalar resolution)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_agg_to_gsu_cfg_set(uint16_t q_idx, bdmf_boolean data);
bdmf_error_t ag_drv_qm_qm_agg_to_gsu_cfg_get(uint16_t q_idx, bdmf_boolean *data);

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
 *     0 - 4  entries
 *     1 - 8  entries
 *     2 - 16 entries
 *     3 - 32 entries
 *     4 - 64 entries
 * upd_fifo_base: 
 *     Update FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_ADDR*8).
 * upd_fifo_size: 
 *     Update FIFO Size
 *     0 - 8 entries
 *     1 - 16 entries
 *     2 - 32 entries
 *     3 - 64 entries
 *     4 - 128 entries
 *     5 - 4 entries
 *     6 - 2 entries
 *     7 - 1   entry
 * rnr_bb_id: 
 *     Runner BB ID associated with this configuration.
 * rnr_task: 
 *     Runner Task number to be woken up when the update FIFO is written to.
 * rnr_enable: 
 *     Enable this runner interface
 * gsu_bb_id: 
 *     GSU BB ID to runner group associated with this configuration.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_rnr_group_cfg_set(uint8_t rnr_idx, const qm_rnr_group_cfg *rnr_group_cfg);
bdmf_error_t ag_drv_qm_rnr_group_cfg_get(uint8_t rnr_idx, qm_rnr_group_cfg *rnr_group_cfg);

/**********************************************************************************************************************
 * data0: 
 *     Data
 *     TRIGGERS_HW
 *     
 * data1: 
 *     Data
 *     TRIGGERS_HW
 *     
 * data2: 
 *     Data
 *     TRIGGERS_HW
 *     
 * data3: 
 *     Data
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_set(uint8_t indirect_grp_idx, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3);

/**********************************************************************************************************************
 * data0: 
 *     Data
 *     TRIGGERS_HW
 *     
 * data1: 
 *     Data
 *     TRIGGERS_HW
 *     
 * data2: 
 *     Data
 *     TRIGGERS_HW
 *     
 * data3: 
 *     Data
 *     TRIGGERS_HW
 *     
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
 *     1 - flow control enable. no WRED drop.
 *     
 *     WRED Flow control is enabled/disabled per queue using this bit FLW_CTRL_EN.
 *     And also according to the WRED profile and the color value in the PD.
 *     When the FLW_CTRL_EN register is set (==1), the QM does flow control, only when the byte occupancy counter is
 *     above the configured wred_min_thr of color=1.
 *     And doesnt drop packets due to WRED.
 *     
 *     When the byte occupancy counter is above the configured wred_max_thr, then the QM will drop the packets, even
 *     if the WRED profile FLW_CTRL_EN register is set (==1).
 * min_thr1: 
 *     WRED Color Min Threshold.
 *     This field represents the higher 24-bits of the queue occupancy byte threshold.
 *     byte_threshold = THR*64.
 * flw_ctrl_en1: 
 *     0 - flow control disable. regular WRED profile
 *     1 - flow control enable. no WRED drop.
 *     
 *     WRED Flow control is enabled/disabled per queue using this bit FLW_CTRL_EN.
 *     And also according to the WRED profile and the color value in the PD.
 *     When the FLW_CTRL_EN register is set (==1), the QM does flow control, only when the byte occupancy counter is
 *     above the configured wred_min_thr of color=1.
 *     And doesnt drop packets due to WRED.
 *     
 *     When the byte occupancy counter is above the configured wred_max_thr, then the QM will drop the packets, even
 *     if the WRED profile FLW_CTRL_EN register is set (==1).
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
 *     TRIGGERS_HW
 *     
 * reorder_credit_enable: 
 *     When this bit is set the QM will send credits to the REORDER block.
 *     Disabling this bit will stop sending credits to the reorder.
 *     TRIGGERS_HW
 *     
 * dqm_pop_enable: 
 *     When this bit is set the QM will pop PDs from the DQM and place them in the runner SRAM
 *     TRIGGERS_HW
 *     
 * rmt_fixed_arb_enable: 
 *     When this bit is set Fixed arbitration will be done in pops from the remote FIFOs (Non delayed highest
 *     priority). If this bit is cleared RR arbitration is done
 *     TRIGGERS_HW
 *     
 * dqm_push_fixed_arb_enable: 
 *     When this bit is set Fixed arbitration will be done in DQM pushes (CPU highest priority, then non-delayed
 *     queues and then normal queues. If this bit is cleared RR arbitration is done.
 *     TRIGGERS_HW
 *     
 * aqm_clk_counter_enable: 
 *     When this bit is set it enables the AQM internal global counter that counts clocks to uSec. If this bit is
 *     cleared the counter is disabled and zeroed.
 *     TRIGGERS_HW
 *     
 * aqm_timestamp_counter_enable: 
 *     When this bit is set it enables the AQM timestamp global counter that counts timestamp in uSec resolution. If
 *     this bit is cleared the counter is disabled and zeroed.
 *     TRIGGERS_HW
 *     
 * aqm_timestamp_write_to_pd_enable: 
 *     When this bit is set it enables the AQM timestamp 10bit write to PD. If this bit is cleared the timestamp is
 *     not written to PD.
 *     TRIGGERS_HW
 *     
 * ingress_pipe_arbiter_rr_mode_enable: 
 *     The Ingress Pipe arbiter chooses between the AGG Timeout and the normal flow.
 *     0 - Strict mode
 *     1 - RR mode
 * cm_ddr_sop_offset_inc_enable: 
 *     Add 8B to the DDR SOP Offset field of PD type DDR_non_agg, from CM to DQM.
 *     0 - dont add
 *     1 - add 8B
 *     Default = 1
 * esbpm_buf_cnt_disable: 
 *     esbpm buffer count disable.
 *     chicken bit to use the sbpm_thr_stat_in[1:0] instead of the counter.
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
 *     N/A for CRDP in 68660A0
 * bb0_egr_msg_out_fifo_sw_rst: 
 *     BB0 free messages from egress OUTPUT FIFO SW reset.
 * bb0_ing_free_msg_fifo_sw_rst: 
 *     BB0 free messages from ingress FIFO SW reset.
 *     for ingress sbpm_free_woc OR multicast fpmini free messages after drop
 *     
 * dqm_pop_pd_data_fifo_sw_rst: 
 *     Sync dqm_qm_pop_pd_data to dqm_valid_cnt_dec_ack egress_control FIFO SW reset.
 *     
 * dqm_ififo_egr_sw_rst: 
 *     DQM egress control ififo FIFO SW reset.
 * dqm_ofifo_egr_sw_rst: 
 *     DQM egress control ofifo FIFO SW reset.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_reset_ctrl_set(const qm_reset_ctrl *reset_ctrl);
bdmf_error_t ag_drv_qm_reset_ctrl_get(qm_reset_ctrl *reset_ctrl);

/**********************************************************************************************************************
 * read_clear_pkts: 
 *     Indicates whether the Drop/max_occupancy packets counter is read clear.
 * read_clear_bytes: 
 *     In CRDP MUST be set to 0 (default).
 *     1 is not supported.
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
 *     Disables DDR_PD_CONGESTION influence on bp condition (no drop, only bp condition)
 * ddr_byte_congestion_disable: 
 *     Disables DDR_BYTE_CONGESTION influence on drop/bp condition
 *     N/A in CRDP, no byte congestion, only PD congestion
 * ddr_occupancy_disable: 
 *     Disables DDR_OCCUPANCY influence on bp condition (no drop, only bp condition fpm_preftech_bp)
 * ddr_fpm_congestion_disable: 
 *     Used for CM DDR Burst Buffer congestion drop ddr_burst_buffer_congestion
 *     (name change from ddr_fpm_congestion)
 *     Disables ddr_burst_buffer_congestion influence on drop/bp condition
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
 *     N/A in CRDP
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
 *     Updated For CRDP it is used for Lotus SM PSRAM CAM enable
 *     
 *     old xrdp definition:
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
 *     Disable only Exclusive packets drop when esbpm is empty.
 *     To disable ALL epsram occupancy drops, use the: qm_general_ctrl2.esbpm_empty_bp_en reg
 * qm_ddr_write_alignment: 
 *     0 According to length
 *     1 8-byte aligned
 * exclusive_dont_drop: 
 *     CRDP RTL supports only 1 - global dont drop.
 *     0 is N/A.
 *     
 *     Controls if the exclusive indication in PD marks the PD as dont drop or as dont drop if the fpm in exclusive
 *     state
 *     1 - global dont drop
 *     0 - FPM exclusive state dont drop
 * close_aggregation_on_mirror_change_en: 
 *     Enable bit for Close aggregation if mirror indication bit is changed.
 *     Default = enable.
 *     
 *     previously chicken bit for DQMOL bug, DQMOL_JIRA_973_FIX_ENABLE
 *     
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
 *     FPM elementry token size:
 *     If (FPM_ELEMENTRY_TOKEN_SEL[27]==0) then:
 *     0: 4KB
 *     1: 8KB
 *     2: 16KB
 *     
 *     If (FPM_ELEMENTRY_TOKEN_SEL[27]==1) then:
 *     0: 512B
 *     1: 1KB
 *     2: 2KB
 * fpm_prefetch_pending_req_limit: 
 *     The allowed on the fly FPM prefetch pending Alloc requests to the FPM.
 * fpm_override_bb_id_en: 
 *     Enable FPM BB ID override with non default value
 * fpm_override_bb_id_value: 
 *     Value to override the default FPM BB ID.
 *     
 * fpm_elementry_token_sel: 
 *     This field indicates the elementry token that is used in FPM_PREFETCH_MIN_POOL_SIZE
 *     0 - for 68660B0 - Lotus
 *     1 - for 68850A0 - Redwood
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
 * max_agg_bytes: 
 *     This field indicates the maximum number of bytes in an aggregated PD.
 * max_agg_pkts: 
 *     This field indicates the maximum number of packets in an aggregated PD
 * min_agg_pkt_size: 
 *     This indicates the minimum Packet size that can be aggregated.
 *     This is for the design to understand if the current accumulated agg size has enough space for another Packet.
 *     if it hasnt, it will close the agg. if it has it will agg and leave the aggregation open.
 *     
 *     
 *     Note!!
 *     You must config this value aligned to be to 8 (e.g for 60 byte min packet configure 64)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_set(uint16_t max_agg_bytes, uint8_t max_agg_pkts, uint16_t min_agg_pkt_size);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_get(uint16_t *max_agg_bytes, uint8_t *max_agg_pkts, uint16_t *min_agg_pkt_size);

/**********************************************************************************************************************
 * agg_pool_sel_en: 
 *     N/A for CRDP in 68660A0
 *     Enable pool selection for aggregarion according to AGG_POOL_SEL configuration and not according to
 *     min_buffer_size config
 * agg_pool_sel: 
 *     N/A for CRDP in 68660A0
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
 * agg_remaining_gap: 
 *     For the current aggregation total size:
 *     if the remaining bytes are less then this register value, considering the max agg short/long queue size, then
 *     close the aggregation.
 *     
 *     if:
 *     max_agg_short/long_size - current_agg_bytes <= agg_remaining_gap
 *     then:
 *     close the current aggregation.
 *     
 *     Size includes 8B EH,
 *     Default for 64B is 72B.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl2_set(bdmf_boolean agg_pool_sel_en, uint8_t agg_pool_sel, uint16_t agg_remaining_gap);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl2_get(bdmf_boolean *agg_pool_sel_en, uint8_t *agg_pool_sel, uint16_t *agg_remaining_gap);

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
 *     DDR SOP Offset option 0. Obsolete, not in use since 63146A0.
 *     
 * ddr_sop_offset1: 
 *     DDR SOP Offset option 1. Obsolete, not in use since 63146A0.
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
 * epon_8021ae: 
 *     If this bit is set then EPON_8021AE will be enabled in the ghost reporting
 * epon_sci: 
 *     If this bit is set then EPON_SCI will be enabled in the ghost reporting
 * epon_fec_enable: 
 *     If this bit is set then EPON_FEC will be enabled in the ghost reporting
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
 * pre_sc0: 
 *     defines the granularity of the prescaler counter:
 *     0 = 10bits
 *     1 = 11bits
 *     2 = 12bits
 *     3 = 13bits
 *     4 = 14bits
 *     5 = 15bits
 *     6 = 16bits
 *     7 = 17bits
 * pre_sc1: 
 *     defines the granularity of the prescaler counter:
 *     0 = 10bits
 *     1 = 11bits
 *     2 = 12bits
 *     3 = 13bits
 *     4 = 14bits
 *     5 = 15bits
 *     6 = 16bits
 *     7 = 17bits
 * pre_sc2: 
 *     defines the granularity of the prescaler counter:
 *     0 = 10bits
 *     1 = 11bits
 *     2 = 12bits
 *     3 = 13bits
 *     4 = 14bits
 *     5 = 15bits
 *     6 = 16bits
 *     7 = 17bits
 * pre_sc3: 
 *     defines the granularity of the prescaler counter:
 *     0 = 10bits
 *     1 = 11bits
 *     2 = 12bits
 *     3 = 13bits
 *     4 = 14bits
 *     5 = 15bits
 *     6 = 16bits
 *     7 = 17bits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_agg_to_timer_cfg_set(uint8_t pre_sc0, uint8_t pre_sc1, uint8_t pre_sc2, uint8_t pre_sc3);
bdmf_error_t ag_drv_qm_global_cfg_qm_agg_to_timer_cfg_get(uint8_t *pre_sc0, uint8_t *pre_sc1, uint8_t *pre_sc2, uint8_t *pre_sc3);

/**********************************************************************************************************************
 * q_buf_init: 
 *     Number of queues to init their initial buffer address
 *     000 - 32K
 *     001 - 24K
 *     010 - 16K
 *     011 - 8K
 *     100 - 4K
 *     101 - 2K
 *     110 - 111 - Reserved
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_dqm_q_buf_init_set(uint8_t q_buf_init);
bdmf_error_t ag_drv_qm_global_cfg_dqm_q_buf_init_get(uint8_t *q_buf_init);

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
 *     valid values = [1..15]
 *     0 - isnt supported
 *     
 *     
 * pd_occupancy_en: 
 *     If set, aggregation of queues with PD occupancy more than encoded on PD_OCCUPANCY_VALUE arent closed even if
 *     the timer is expired.
 *     If not set, then aggregation is closed after queues timer expires
 * pd_occupancy_value: 
 *     if PD_OCCUPANCY_EN == 1 then
 *     Aggregations of queues with more than PD_OCCUPNACY pds value (total_valid counter) are not closed on timeout.
 *     value of 0 is illegal, since the aggregation is counted as 1.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(uint8_t prescaler_granularity, uint8_t aggregation_timeout_value, bdmf_boolean pd_occupancy_en, uint8_t pd_occupancy_value);
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(uint8_t *prescaler_granularity, uint8_t *aggregation_timeout_value, bdmf_boolean *pd_occupancy_en, uint8_t *pd_occupancy_value);

/**********************************************************************************************************************
 * fpm_gbl_cnt: 
 *     FPM global counter
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(uint32_t fpm_gbl_cnt);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(uint32_t *fpm_gbl_cnt);

/**********************************************************************************************************************
 * dummy_profile_0: 
 *     DDR dummy spare room profile 0
 * dummy_profile_1: 
 *     DDR dummy spare room profile 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_dummy_spare_room_profile_id_set(uint8_t dummy_profile_0, uint8_t dummy_profile_1);
bdmf_error_t ag_drv_qm_global_cfg_dummy_spare_room_profile_id_get(uint8_t *dummy_profile_0, uint8_t *dummy_profile_1);

/**********************************************************************************************************************
 * mem_init_en: 
 *     Memory auto init enable
 *     TRIGGERS_HW
 *     
 * mem_sel_init: 
 *     Select which memory to AUTO INIT
 *     
 *     3b000: qm_total_valid_counter
 *     3b001: qm_drop_counter
 *     3b010: qm_dqm_valid_counter
 *     3b011: qm_epon_rpt_cnt_counter
 *     3b111: All memoires
 *     TRIGGERS_HW
 *     
 * mem_size_init: 
 *     What is the size of the memory (according to the Number of Qs)
 *     
 *     3b000: 96
 *     3b001: 128
 *     3b010: 160
 *     3b011: 288
 *     3b100: 448
 *     3b101: 2K
 *     3b110: 8K
 *     3b111: 32K
 *     TRIGGERS_HW
 *     
 * egr_rr_rd_on_init_en: 
 *     Enable read from egress pop RR memories during memory init
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_set(bdmf_boolean mem_init_en, uint8_t mem_sel_init, uint8_t mem_size_init, bdmf_boolean egr_rr_rd_on_init_en);
bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_get(bdmf_boolean *mem_init_en, uint8_t *mem_sel_init, uint8_t *mem_size_init, bdmf_boolean *egr_rr_rd_on_init_en);

/**********************************************************************************************************************
 * mem_init_done: 
 *     Memory auto init done
 *     Bit is asserted when init is done.
 *     Bit is de-asserted when init starts
 * dqm_mem_init_done: 
 *     DQM Memory auto init done
 *     Bit is asserted when DQM init is done.
 *     
 * egr_pop_mem_init_done: 
 *     Egress Pop Memories auto init done
 *     Bit is asserted when init is done.
 *     Bit is de-asserted when init starts
 * pdfifo_mem_init_done: 
 *     PDFIFO Memories auto init done
 *     Bit is asserted when init is done.
 *     Bit is de-asserted when init starts
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_sts_get(bdmf_boolean *mem_init_done, bdmf_boolean *dqm_mem_init_done, bdmf_boolean *egr_pop_mem_init_done, bdmf_boolean *pdfifo_mem_init_done);

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
 *     N/A for CRDP in 68660A0
 *     Number of bytes used for each buffer in pool0 - Up to 16K
 * pool_1_num_of_bytes: 
 *     N/A for CRDP in 68660A0
 *     Number of Bytes used for each buffer in pool1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_set(uint16_t pool_0_num_of_bytes, uint16_t pool_1_num_of_bytes);
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_get(uint16_t *pool_0_num_of_bytes, uint16_t *pool_1_num_of_bytes);

/**********************************************************************************************************************
 * pool_2_num_of_bytes: 
 *     N/A for CRDP in 68660A0
 *     Number of bytes used for each buffer in pool0 - Up to 16K
 * pool_3_num_of_bytes: 
 *     N/A for CRDP in 68660A0
 *     Number of Bytes used for each buffer in pool1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_set(uint16_t pool_2_num_of_bytes, uint16_t pool_3_num_of_bytes);
bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_get(uint16_t *pool_2_num_of_bytes, uint16_t *pool_3_num_of_bytes);

/**********************************************************************************************************************
 * mc_headers_pool_sel: 
 *     N/A for CRDP in 68660A0
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
 *     Set 880d for 1usec @880MHz clk.
 *     *Note that 1d is illegal value.
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
 * agg_closure_suspend_on_fpm_congestion_disable: 
 *     Relevant when AGG_CLOSURE_SUSPEND_ON_BP is set.
 *     When 1, ingress_ddr_fpm_congestion_bp does not suspend aggregation timers closure.
 *     When 0, ingress_ddr_fpm_congestion_bp suspend aggregation timers closure.
 * epsram_buffer_size: 
 *     config EPSRAM buffer size 512B or 256B
 *     0= 512B (default)
 *     1= 256B
 *     
 * agg_closure_suspend_on_stall: 
 *     When set, closure due to aggregation timers will be suspended when the ingress pipeline is stalled.
 * esbpm_prefetch_bp_en: 
 *     When set, bp due to esbpm prefetch will be enable when the thr is crossed.
 * esbpm_empty_bp_en: 
 *     When set, bp due to esbpm empty status will be enable.
 * esbpm_prefetch_bp_thr: 
 *     threshold value for bp due to esbpm prefetch
 * egress_drop_per_queue_cnt_en: 
 *     When set, the per queue drop counter counts also egress drops (per queue).
 *     When 0, dont count egress drop per queue, only global.
 *     default is 1.
 * sec_bn_fifo_usedw_bp_thr: 
 *     threshold value for bp due to cm_sec_bn_fifo_used_words value
 * force_epsram_bufmng_en: 
 *     When set, use the bufmng for force_epsram pkts
 * split_ddr_epsram_en: 
 *     When set, max_agg_bytes < 14k and cr_qm_top_qm_global_cfg_qm_general_ctrl2_copy_to_ddr_on_qtc_dis = 1
 *     
 *     When set, split the queues odd/even to ddr and epsram
 *     
 * copy_to_ddr_on_qtc_dis: 
 *     When set, copy to ddr doesnt consider the qtc counter = 0 condition (current_qtc_value!=19b0)
 * mixed_copy_queue_en: 
 *     Enables single copy queue for DDR and EPSRAM. Allows a single queue to have a random mix of DDR and EPSRAM.
 *     When disabled queue is used for DDR only and once packets are pending in this queue next packets to the same
 *     queue will also be copied to DDR.
 * qm_is_upstream: 
 *     Indicate if the QM instance is Upstream or Downstream.
 *     0=Downstream
 *     1=Upstream
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_qm_general_ctrl2_set(const qm_global_cfg_qm_general_ctrl2 *global_cfg_qm_general_ctrl2);
bdmf_error_t ag_drv_qm_global_cfg_qm_general_ctrl2_get(qm_global_cfg_qm_general_ctrl2 *global_cfg_qm_general_ctrl2);

/**********************************************************************************************************************
 * maxreq: 
 *     max number of on the fly requests
 * enable: 
 *     enable the sm
 * reset: 
 *     reset the fifo
 * bbid: 
 *     bb dest id of the sbpm interface
 * destovrd: 
 *     override the fixed dest id coded in the design with the configuration in this register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_esbpm_ctrl_set(uint16_t esbpm_idx, const qm_esbpm_ctrl *esbpm_ctrl);
bdmf_error_t ag_drv_qm_esbpm_ctrl_get(uint16_t esbpm_idx, qm_esbpm_ctrl *esbpm_ctrl);

/**********************************************************************************************************************
 * bb_id: 
 *     bb id of the sbpm interface
 * bb_id_ovr: 
 *     override the fixed dest id coded in the design with the configuration in this register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_esbpm_bb_id_ctrl_set(uint16_t esbpm_idx, uint16_t bb_id, bdmf_boolean bb_id_ovr);
bdmf_error_t ag_drv_qm_esbpm_bb_id_ctrl_get(uint16_t esbpm_idx, uint16_t *bb_id, bdmf_boolean *bb_id_ovr);

/**********************************************************************************************************************
 * value: 
 *     Global threshold value of Empty
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_esbpm_empty_thr_set(uint16_t value);
bdmf_error_t ag_drv_qm_global_cfg_esbpm_empty_thr_get(uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     Global threshold value of High
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_esbpm_high_thr_set(uint16_t value);
bdmf_error_t ag_drv_qm_global_cfg_esbpm_high_thr_get(uint16_t *value);

/**********************************************************************************************************************
 * value: 
 *     Global threshold value of High
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_channel_congestion_counter_thr_set(uint32_t value);
bdmf_error_t ag_drv_qm_global_cfg_channel_congestion_counter_thr_get(uint32_t *value);

/**********************************************************************************************************************
 * epon_cnt_start_queue: 
 *     EPON counter start queue value.
 *     1st queue of 128 queues block
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_cfg_epon_overhead_ctrl2_set(uint16_t epon_cnt_start_queue);
bdmf_error_t ag_drv_qm_global_cfg_epon_overhead_ctrl2_get(uint16_t *epon_cnt_start_queue);

/**********************************************************************************************************************
 * ddr_headroom: 
 *     DDR headroom space
 * pehs: 
 *     PEHS (Proprietary Embedded Header Size) : size of proprietary tag, includes EH.
 *     A range 8B-64B and w/ quanta of 8B (3 bits)
 *     0=8B default
 *     1=16B
 *     2=24B
 *     ..
 *     7=64B
 *     
 * ddr_tailroom: 
 *     DDR tailroom in bytes
 *     
 * ddr_non_agg: 
 *     DDR non AGG:
 *     Force copy to DDR
 *     Force do not aggregate
 *     Force DDR non AGG egress PD
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_ddr_spare_room_set(uint16_t pair_idx, uint16_t ddr_headroom, uint8_t pehs, uint16_t ddr_tailroom, bdmf_boolean ddr_non_agg);
bdmf_error_t ag_drv_qm_qm_ddr_spare_room_get(uint16_t pair_idx, uint16_t *ddr_headroom, uint8_t *pehs, uint16_t *ddr_tailroom, bdmf_boolean *ddr_non_agg);

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
 * qm_timer_wraparound0: 
 *     QM aggregation timer 0 wraps around. In this case it isnt guaranteed that the aggregation will be closed on
 *     pre-defined timeout expiration. However the aggregation should be closed eventually.
 *     
 * qm_timer_wraparound1: 
 *     QM aggregation timer 1 wraps around. In this case it isnt guaranteed that the aggregation will be closed on
 *     pre-defined timeout expiration. However the aggregation should be closed eventually.
 * qm_timer_wraparound2: 
 *     QM aggregation timer 2 wraps around. In this case it isnt guaranteed that the aggregation will be closed on
 *     pre-defined timeout expiration. However the aggregation should be closed eventually.
 * qm_timer_wraparound3: 
 *     QM aggregation timer 3 wraps around. In this case it isnt guaranteed that the aggregation will be closed on
 *     pre-defined timeout expiration. However the aggregation should be closed eventually.
 * qm_normal_queue_pd_no_credit: 
 *     A PD arrived to the Normal queue without having any credits
 * qm_non_delayed_queue_pd_no_credit: 
 *     N/A for CRDP 68660A0
 *     set as constant 0
 *     
 *     A PD arrived to the NON-delayed queue without having any credits
 * qm_non_valid_queue: 
 *     N/A for CRDP 68660A0
 *     set as constant 0
 *     
 *     old XRDP:
 *     A PD arrived with a non valid queue number (>287)
 * qm_agg_coherent_inconsistency: 
 *     N/A for CRDP 68660A0
 *     set as constant 0
 *     
 *     An aggregation of PDs was done in which the coherent bit of the PD differs between them (The coherent bit of
 *     the first aggregated PD was used)
 * qm_force_copy_on_non_delayed: 
 *     N/A for CRDP 68660A0
 *     set as constant 0
 *     
 *     A PD with force copy bit set was received on the non-delayed queue (in this queue the copy machine is bypassed)
 * qm_fpm_pool_size_nonexistent: 
 *     A PD was marked to be copied, but there does not exist an FPM pool buffer large enough to hold it.
 * qm_target_mem_abs_contradiction: 
 *     N/A for CRDP 68660A0
 *     set as constant 0
 *     
 *     A PD was marked with a target_mem=1 (located in PSRAM) and on the other hand, the absolute address indication
 *     was set.
 * qm_1588_drop: 
 *     For 68660A0 CRDP used as:
 *     Exclusive PD is dropped
 *     qm_exclusive_drop
 *     
 *     Old XRDP:
 *     1588 Packet is dropped when the QM PD occupancy exceeds threshold (64K)
 * qm_1588_multicast_contradiction: 
 *     For 68660A0 CRDP used as:
 *     A PD was marked as force to EPSRAM and multicast together.
 *     multicast_forced_to_epsram_err
 *     
 *     Old XRDP use:
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
 * qm_fpmini_intr: 
 *     FPMINI interrupt
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
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_qm_intr_ctrl_itr_get(uint32_t *ist);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 *     TRIGGERS_HW
 *     
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     TRIGGERS_HW
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 *     TRIGGERS_HW
 *     
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 *     TRIGGERS_HW
 *     
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_set(const qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_get(qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);

/**********************************************************************************************************************
 * queue_num: 
 *     Queue Number
 *     TRIGGERS_HW
 *     
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
 *     TRIGGERS_HW
 *     
 * done: 
 *     Indicates that read/write to DQM is done
 *     TRIGGERS_HW
 *     
 * error: 
 *     Indicates that that an error occured (write on full or read on empty)
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(uint8_t indirect_grp_idx, uint16_t queue_num, uint8_t cmd, bdmf_boolean done, bdmf_boolean error);
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(uint8_t indirect_grp_idx, uint16_t *queue_num, uint8_t *cmd, bdmf_boolean *done, bdmf_boolean *error);

/**********************************************************************************************************************
 * queue_occupancy_thr: 
 *     Queue Occupancy Threshold.
 *     When passing this threhold, packets will be copied to the DDR
 * fadt_en: 
 *     FADT Enable. When FADT is enable, the queue threshold for copying is calculated as below
 *     queue_occupancy_thr = MAX(min_queue_thr , MIN(max_queue_thr ,dynamic_threshold)
 *     
 * psram_thr: 
 *     Indicates which of the two PSRAM threshold crossing indications coming from the SBPM will be used for the copy
 *     decision. when going over the chosen threshold, packets will be copied to the DDR.
 *     0 - Lower threshold
 *     1 - Higher threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_copy_decision_profile_set(uint8_t profile_idx, uint32_t queue_occupancy_thr, bdmf_boolean fadt_en, bdmf_boolean psram_thr);
bdmf_error_t ag_drv_qm_copy_decision_profile_get(uint8_t profile_idx, uint32_t *queue_occupancy_thr, bdmf_boolean *fadt_en, bdmf_boolean *psram_thr);

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
 *     TRIGGERS_HW
 *     
 * pop_pool1: 
 *     Pop FIFO entry
 *     TRIGGERS_HW
 *     
 * pop_pool2: 
 *     Pop FIFO entry
 *     TRIGGERS_HW
 *     
 * pop_pool3: 
 *     Pop FIFO entry
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pop_3_set(bdmf_boolean pop_pool0, bdmf_boolean pop_pool1, bdmf_boolean pop_pool2, bdmf_boolean pop_pool3);
bdmf_error_t ag_drv_qm_pop_3_get(bdmf_boolean *pop_pool0, bdmf_boolean *pop_pool1, bdmf_boolean *pop_pool2, bdmf_boolean *pop_pool3);

/**********************************************************************************************************************
 * pop: 
 *     Pop FIFO entry
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pop_2_set(bdmf_boolean pop);
bdmf_error_t ag_drv_qm_pop_2_get(bdmf_boolean *pop);

/**********************************************************************************************************************
 * pop: 
 *     Pop FIFO entry
 *     TRIGGERS_HW
 *     
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
bdmf_error_t ag_drv_qm_bb0_ing_free_msg_fifo_data_get(uint32_t idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     DATA
 * epsram_or_ddr: 
 *     Indicate if DDR =0 or EPSRAM =1 reservation
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_set(uint32_t idx, uint32_t data, bdmf_boolean epsram_or_ddr);
bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_get(uint32_t idx, uint32_t *data, bdmf_boolean *epsram_or_ddr);

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
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dbg_get(uint32_t *status);

/**********************************************************************************************************************
 * status: 
 *     queue
 *     TRIGGERS_HW
 *     
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
 * queue_vec: 
 *     each bit represents queue.
 *     1 - fc is enabled for this queue (unmasked)
 *     0 - fc is disabled for this queue (masked)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fc_queue_mask_set(uint8_t idx, uint32_t queue_vec);
bdmf_error_t ag_drv_qm_fc_queue_mask_get(uint8_t idx, uint32_t *queue_vec);

/**********************************************************************************************************************
 * select: 
 *     Controls Debug bus select:
 *     
 *     5h1:  qm_dbg_bus = qm_bb_input_dbg_bus;
 *     5h2:  qm_dbg_bus = qm_bb_output_dbg_bus;
 *     5h3:  qm_dbg_bus = qm_cm_dbg_bus;
 *     5h4:  qm_dbg_bus = qm_ddr_write_dbg_bus; - NA for Lotus
 *     5h5:  qm_dbg_bus = qm_counters_dbg_bus;
 *     5h6:  qm_dbg_bus = qm_cpu_if_dbg_bus; - NA for Lotus
 *     5h7:  qm_dbg_bus = qm_dqm_push_dbg_bus; - NA for Lotus
 *     5h8:  qm_dbg_bus = qm_egress_dbg_bus;
 *     5h9:  qm_dbg_bus = qm_fpm_prefetch_dbg_bus;
 *     5ha:  qm_dbg_bus = qm_ingress_dbg_bus;
 *     5hb:  qm_dbg_bus = qm_rmt_fifos_dbg_bus;
 *     5hc:  qm_dbg_bus = {8b0,qm_esbpm_if_debug_0};
 *     5hd:  qm_dbg_bus = {8b0,qm_esbpm_if_debug_1};
 *     5he:  qm_dbg_bus = {8b0,qm_esbpm_if_debug_2};
 *     5hf:  qm_dbg_bus = {8b0,qm_esbpm_if_debug_3};
 *     5h10: qm_dbg_bus = {8b0,qm_esbpm_if_debug_4};
 *     5h11: qm_dbg_bus = dqm_debug;
 *     5h17: qm_dbg_bus = {8b0,dqm_diag_r}; - NA for Lotus
 * enable: 
 *     Enable register controlled debug select
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_debug_sel_set(uint8_t select, bdmf_boolean enable);
bdmf_error_t ag_drv_qm_debug_sel_get(uint8_t *select, bdmf_boolean *enable);

/**********************************************************************************************************************
 * data: 
 *     Data
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_debug_bus_lsb_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 *     CHANGED_BY_HW
 *     
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
bdmf_error_t ag_drv_qm_egress_dqm_ofifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_egress_dqm_ififo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * ovr_en: 
 *     BB rout address decode Override enable
 * dest_id: 
 *     Destination ID
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb_route_ovr_set(uint8_t idx, bdmf_boolean ovr_en, uint16_t dest_id);
bdmf_error_t ag_drv_qm_bb_route_ovr_get(uint8_t idx, bdmf_boolean *ovr_en, uint16_t *dest_id);

/**********************************************************************************************************************
 * ingress_stat: 
 *     Stat
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_ingress_stat_get(uint32_t *ingress_stat);

/**********************************************************************************************************************
 * egress_stat: 
 *     Stat
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_egress_stat_get(uint32_t *egress_stat);

/**********************************************************************************************************************
 * cm_stat: 
 *     Stat
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_stat_get(uint32_t *cm_stat);

/**********************************************************************************************************************
 * fpm_prefetch_stat: 
 *     Stat
 *     CHANGED_BY_HW
 *     
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
 *     0x1 - fpm exclusive threshold (qm_cm_ddr_burst_congestion_status[1:0] == EXCLUSIVE).
 *     0x2 - fpm prefetch occupancy.
 *     0x4 - ESBPM prefetch alloc fifo occupacny. esbpm_bp when
 *     cr_qm_top_qm_global_cfg_qm_general_ctrl2_esbpm_empty_bp_en
 *     0x8 - PD count in copy machine is exceeded.
 *     
 *     Any permutation of the above may occur. especially if the value isnt cleared every time.
 *     
 *     
 *     A write of 0xFFFF_FFFF in order at reset this indication.
 *     TRIGGERS_HW
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
 * total_egress_accum_cnt_pkt: 
 *     CRDP counts only Pkts, use only 1 (default)
 *     
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     Total egress accumulated counters j1211
 * global_egress_drop_cnt_pkt: 
 *     CRDP counts only Pkts, use only 1 (default)
 *     
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     QM Global egress drop counters j1224
 * drop_ing_egr_cnt_pkt: 
 *     CRDP counts only Pkts, use only 1 (default)
 *     
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     Drop counters (ingress and egress) j1226
 * fpm_grp_drop_cnt_pkt: 
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     FPM_GRP_DROP_CNT (UG or BMGR drop)
 * qm_pd_congestion_drop_cnt_pkt: 
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     QM_PD_CONGESTION_DROP_CNT (crossing max pds per queue - total_pd_thr)
 * ddr_pd_congestion_drop_cnt_pkt: 
 *     N/A for CRDP - use only default value 0.
 *     we use only BP for this case.
 *     
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     DDR_PD_CONGESTION_DROP_CNT (copy machine fifo used entries)
 * wred_drop_cnt_pkt: 
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     WRED_DROP_CNT_PKT
 * good_lvl2_pkts_cnt_pkt: 
 *     N/A for CRDP - use only default value 0.
 *     
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     GOOD_LVL2_PKTS_CNT will count ingress agg pd as packets (if reprocessing bit set)
 * good_lvl1_pkts_cnt_pkt: 
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     GOOD_LVL1_PKTS_CNT will count ingress agg pd as packets (if reprocessing bit not set)
 * qm_total_valid_cnt_pkt: 
 *     N/A for CRDP - use only default value 0.
 *     Total valid counts only PDs.
 *     
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     QM_TOTAL_VALID_CNT_PKT will count agg pd as packets
 * dqm_valid_cnt_pkt: 
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     DQM_VALID_CNT_PKT will count agg pd as packets
 * psram_drop_cnt_pkt: 
 *     Mode/Chicken bits for count pkts instead of PDs jira fix:
 *     PSRAM_DROP_CNT (psram occupancy and esbpm congestion states)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_count_pkt_not_pd_mode_bits_set(const qm_count_pkt_not_pd_mode_bits *count_pkt_not_pd_mode_bits);
bdmf_error_t ag_drv_qm_count_pkt_not_pd_mode_bits_get(qm_count_pkt_not_pd_mode_bits *count_pkt_not_pd_mode_bits);

/**********************************************************************************************************************
 * stat: 
 *     Stat
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_ingress_stat2_get(uint32_t *stat);

/**********************************************************************************************************************
 * ovr_en: 
 *     BB rout address decode Override enable
 * dest_id: 
 *     Destination ID
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_bbhtx_bb_route_ovr_set(uint8_t index, bdmf_boolean ovr_en, uint16_t dest_id);
bdmf_error_t ag_drv_qm_cm_bbhtx_bb_route_ovr_get(uint8_t index, bdmf_boolean *ovr_en, uint16_t *dest_id);

/**********************************************************************************************************************
 * bnentry: 
 *     BN
 * valid: 
 *     SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_sbnfifo0_get(uint8_t index, uint16_t *bnentry, bdmf_boolean *valid);

/**********************************************************************************************************************
 * bnentry: 
 *     BN
 * valid: 
 *     SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_sbnfifo1_get(uint8_t index, uint16_t *bnentry, bdmf_boolean *valid);

/**********************************************************************************************************************
 * bnentry0: 
 *     BN
 * bnentry1: 
 *     BN
 * valid: 
 *     SBN is Valid
 * msbvalid: 
 *     ONLY MSB SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_sbnfifoprefetch0_get(uint16_t *bnentry0, uint16_t *bnentry1, bdmf_boolean *valid, bdmf_boolean *msbvalid);

/**********************************************************************************************************************
 * bnentry0: 
 *     BN
 * bnentry1: 
 *     BN
 * valid: 
 *     SBN is Valid
 * msbvalid: 
 *     ONLY MSB SBN is Valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_sbnfifoprefetch1_get(uint16_t *bnentry0, uint16_t *bnentry1, bdmf_boolean *valid, bdmf_boolean *msbvalid);

/**********************************************************************************************************************
 * alloc0used: 
 *     alloc_fifo_0_used_words
 * alloc1used: 
 *     alloc_fifo_1_used_words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_esbpmstat_get(uint8_t *alloc0used, uint8_t *alloc1used);

/**********************************************************************************************************************
 * esbpm_buf_cnt: 
 *     esbpm buffers counter - inc on buffer request, dec on i_sbpm_bn_free_ind_out indication from esbpm.
 * sbpm_thr_stat: 
 *     esbpm thr status 2b:
 *     sbpm_thr_stat[0] = Empty (qm)
 *     sbpm_thr_stat[1] = High (qm)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_esbpmstat2_get(uint16_t *esbpm_buf_cnt, uint8_t *sbpm_thr_stat);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_ddr_burst_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_psr_burst_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_psr_wr_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_sec_bn_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_ing_rd_buf_fifo_status_get(uint8_t index, uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_cbpm_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * egr0_wrr_stat: 
 *     egress wrr stat
 * egr1_wrr_stat: 
 *     egress wrr stat
 * egr2_wrr_stat: 
 *     egress wrr stat
 * egr3_wrr_stat: 
 *     egress wrr stat
 * egr4_wrr_stat: 
 *     egress wrr stat
 * egr5_wrr_stat: 
 *     egress wrr stat
 * egr6_wrr_stat: 
 *     egress wrr stat
 * egr7_wrr_stat: 
 *     egress wrr stat
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_egr_wrr_status_get(qm_cm_egr_wrr_status *cm_egr_wrr_status);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_ddr_egr_fifo_status_get(uint8_t index, uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_psr_egr_fifo_status_get(uint8_t index, uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * addr: 
 *     Route Address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb_routh_ovr_addr_set(uint16_t idx, uint32_t addr);
bdmf_error_t ag_drv_qm_bb_routh_ovr_addr_get(uint16_t idx, uint32_t *addr);

/**********************************************************************************************************************
 * addr: 
 *     Route Address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bbhtx_bb_rough_ovr_addr_set(uint16_t idx, uint32_t addr);
bdmf_error_t ag_drv_qm_bbhtx_bb_rough_ovr_addr_get(uint16_t idx, uint32_t *addr);

/**********************************************************************************************************************
 * ptr: 
 *     DQM BD FLL Memory write pointer
 * ptr_wrap: 
 *     DQM BD FLL Memory write pointer Wrap
 * almst_full: 
 *     DQM BD FLL Almost Full indication status
 * almst_full_stk: 
 *     DQM BD FLL Almost Full indication sticky bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_fll_mem_wr_ptr_get(uint32_t *ptr, bdmf_boolean *ptr_wrap, bdmf_boolean *almst_full, bdmf_boolean *almst_full_stk);

/**********************************************************************************************************************
 * ptr: 
 *     DQM BD FLL Memory read pointer
 * ptr_wrap: 
 *     DQM BD FLL Memory read pointer Wrap
 * almst_full: 
 *     DQM BD FLL Almost Full indication status
 * almst_full_stk: 
 *     DQM BD FLL Almost Full indication sticky bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_fll_mem_rd_ptr_get(uint32_t *ptr, bdmf_boolean *ptr_wrap, bdmf_boolean *almst_full, bdmf_boolean *almst_full_stk);

/**********************************************************************************************************************
 * buf: 
 *     DQM Number of BD FLL Memory available buffers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_fll_buf_avlbl_get(uint16_t *buf);

/**********************************************************************************************************************
 * buf: 
 *     DQM Minimum Number of BD FLL Memory available buffers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_fll_buf_avlbl_min_get(uint16_t *buf);

/**********************************************************************************************************************
 * queue: 
 *     Queue number to count
 * queue_all: 
 *     Count all queues
 * stall_type: 
 *     Stall type to count
 *     000 - BD_next mem stall
 *     001 - PD mem stall
 *     010 - Tail mem stall
 *     011 - FLL mem stall
 *     100 - Head mem stall
 *     101 - HT Pop stall
 *     110 - HT Push stall
 *     
 * stall_all: 
 *     Count all stall types
 * dbg_en: 
 *     Enable Debug counters
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_dbg_cntr_cfg_set(const qm_dqm_dbg_cntr_cfg *dqm_dbg_cntr_cfg);
bdmf_error_t ag_drv_qm_dqm_dbg_cntr_cfg_get(qm_dqm_dbg_cntr_cfg *dqm_dbg_cntr_cfg);

/**********************************************************************************************************************
 * cntr: 
 *     Counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_dbg_push_ack_cntr_get(uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     Counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_dbg_push_done_cntr_get(uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     Counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_dbg_pop_req_cntr_get(uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     Counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_dbg_pop_vld_cntr_get(uint32_t *cntr);

/**********************************************************************************************************************
 * cntr: 
 *     Counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_dbg_stall_cntr_get(uint32_t *cntr);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_bb0_ing_free_msg_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * used_words: 
 *     Used words
 * empty: 
 *     Empty
 * full: 
 *     Full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_dqm_pop_pd_data_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_mc_first_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_mc_rep_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_mc_first_drop_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_mc_body_drop_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * counter: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_global_drop_cnt_get(uint32_t *counter);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_qm_eng_spare_config_set(uint8_t index, uint32_t data);
bdmf_error_t ag_drv_qm_qm_eng_spare_config_get(uint8_t index, uint32_t *data);

/**********************************************************************************************************************
 * fpm_grp_drop: 
 *     Counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_fpm_grp_drop_cnt_get(uint32_t idx, uint32_t *fpm_grp_drop);

/**********************************************************************************************************************
 * data: 
 *     DATA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_cm_residue_data_get(uint16_t idx, uint32_t *data);

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
bdmf_error_t ag_drv_qm_drop_counter_get(uint16_t q_idx, uint32_t *data);

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
bdmf_error_t ag_drv_qm_accumulated_counter_get(uint32_t q_idx, uint32_t *data);

/**********************************************************************************************************************
 * wred_profile: 
 *     Defines to which WRED Profile this queue belongs to.
 * copy_dec_profile: 
 *     Defines to which Copy Decision Profile this queue belongs.
 *     profile 3d7 is copy_to_ddr: always copy to DDR
 * fpm_ug_or_bufmng: 
 *     Defines to which FPM UG or BUFMNG this queue belongs.
 *     Use BUFMNG or UG according to the config bit: BUFMNG_EN_OR_UG_CNTR in QM_GLOBAL_CFG.QM_GENERAL_CTRL2
 * exclusive_priority: 
 *     Defines this queue with exclusive priority.
 * res_profile: 
 *     FPM reservation profile.
 *     Once the QM goes over global FPM reservation threshold.
 *     Queue with more bytes the defined in the profile will be dropped.
 *     Profile 0 means no drop due to FPM reservation for the queues with this profile.
 * timestamp_res_profile: 
 *     Select which profile this queue belongs to. 1 out of 8 resolutions profiles, for 10b out of 32b timestamp
 *     resolution in the PD:
 *     0d = profile0
 *     1d = profile1
 *     2d = profile2
 *     3d = profile3
 *     4d = ... etc.
 * aggregation_profile: 
 *     Select 1 out of 8 aggregation profiles
 * ddr_copy_disable: 
 *     Defines this queue never to copy to DDR.
 * aggregation_disable: 
 *     Defines this queue never to aggregated PDs.
 * spare_room_1: 
 *     Select 1 out of 4 headroom and tailroom pairs. if PDs SOP select is set to 1. This profile is chosen
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_q_context_set(uint16_t q_idx, const qm_q_context *q_context);
bdmf_error_t ag_drv_qm_q_context_get(uint16_t q_idx, qm_q_context *q_context);

/**********************************************************************************************************************
 * wr_ptr: 
 *     PDFIFO WR pointers
 * rd_ptr: 
 *     PDFIFO RD pointers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_pdfifo_ptr_set(uint16_t q_idx, uint8_t wr_ptr, uint8_t rd_ptr);
bdmf_error_t ag_drv_qm_pdfifo_ptr_get(uint16_t q_idx, uint8_t *wr_ptr, uint8_t *rd_ptr);

/**********************************************************************************************************************
 * wr_ptr: 
 *     UF WR pointers
 * rd_ptr: 
 *     UF RD pointers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_qm_update_fifo_ptr_set(uint16_t fifo_idx, uint16_t wr_ptr, uint16_t rd_ptr);
bdmf_error_t ag_drv_qm_update_fifo_ptr_get(uint16_t fifo_idx, uint16_t *wr_ptr, uint16_t *rd_ptr);

#ifdef USE_BDMF_SHELL
enum
{
    cli_qm_ddr_cong_ctrl,
    cli_qm_qm_agg_to_gsu_cfg,
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
    cli_qm_global_cfg_aggregation_ctrl,
    cli_qm_global_cfg_aggregation_ctrl2,
    cli_qm_global_cfg_fpm_coherent_base_addr,
    cli_qm_ddr_sop_offset,
    cli_qm_epon_overhead_ctrl,
    cli_qm_global_cfg_bbhtx_fifo_addr,
    cli_qm_global_cfg_qm_agg_to_timer_cfg,
    cli_qm_global_cfg_dqm_q_buf_init,
    cli_qm_global_cfg_qm_egress_flush_queue,
    cli_qm_global_cfg_qm_aggregation_timer_ctrl,
    cli_qm_global_cfg_qm_fpm_ug_gbl_cnt,
    cli_qm_global_cfg_dummy_spare_room_profile_id,
    cli_qm_global_cfg_mem_auto_init,
    cli_qm_global_cfg_mem_auto_init_sts,
    cli_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens,
    cli_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte,
    cli_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte,
    cli_qm_global_cfg_mc_ctrl,
    cli_qm_global_cfg_aqm_clk_counter_cycle,
    cli_qm_global_cfg_aqm_push_to_empty_thr,
    cli_qm_global_cfg_qm_general_ctrl2,
    cli_qm_esbpm_ctrl,
    cli_qm_esbpm_bb_id_ctrl,
    cli_qm_global_cfg_esbpm_empty_thr,
    cli_qm_global_cfg_esbpm_high_thr,
    cli_qm_global_cfg_channel_congestion_counter_thr,
    cli_qm_global_cfg_epon_overhead_ctrl2,
    cli_qm_qm_ddr_spare_room,
    cli_qm_fpm_pool_thr,
    cli_qm_fpm_ug_cnt,
    cli_qm_intr_ctrl_isr,
    cli_qm_intr_ctrl_ism,
    cli_qm_intr_ctrl_ier,
    cli_qm_intr_ctrl_itr,
    cli_qm_clk_gate_clk_gate_cntrl,
    cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl,
    cli_qm_copy_decision_profile,
    cli_qm_timestamp_res_profile,
    cli_qm_global_egress_drop_counter,
    cli_qm_global_egress_aqm_drop_counter,
    cli_qm_epon_q_byte_cnt,
    cli_qm_epon_q_status,
    cli_qm_rd_data_pool0,
    cli_qm_rd_data_pool1,
    cli_qm_rd_data_pool2,
    cli_qm_rd_data_pool3,
    cli_qm_pop_3,
    cli_qm_pop_2,
    cli_qm_pop_1,
    cli_qm_bb0_egr_msg_out_fifo_data,
    cli_qm_bb0_ing_free_msg_fifo_data,
    cli_qm_fpm_buffer_reservation_data,
    cli_qm_port_cfg,
    cli_qm_fc_ug_mask_ug_en,
    cli_qm_fc_queue_range1_start,
    cli_qm_fc_queue_range2_start,
    cli_qm_dbg,
    cli_qm_ug_occupancy_status,
    cli_qm_queue_range1_occupancy_status,
    cli_qm_queue_range2_occupancy_status,
    cli_qm_fc_queue_mask,
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
    cli_qm_egress_dqm_ofifo_status,
    cli_qm_egress_dqm_ififo_status,
    cli_qm_bb_route_ovr,
    cli_qm_ingress_stat,
    cli_qm_egress_stat,
    cli_qm_cm_stat,
    cli_qm_fpm_prefetch_stat,
    cli_qm_qm_connect_ack_counter,
    cli_qm_qm_ddr_wr_reply_counter,
    cli_qm_qm_ddr_pipe_byte_counter,
    cli_qm_qm_illegal_pd_capture,
    cli_qm_qm_ingress_processed_pd_capture,
    cli_qm_qm_cm_processed_pd_capture,
    cli_qm_fpm_pool_drop_cnt,
    cli_qm_fpm_buffer_res_drop_cnt,
    cli_qm_psram_egress_cong_drp_cnt,
    cli_qm_backpressure,
    cli_qm_aqm_timestamp_curr_counter,
    cli_qm_bb0_egr_msg_out_fifo_status,
    cli_qm_count_pkt_not_pd_mode_bits,
    cli_qm_qm_ingress_stat2,
    cli_qm_cm_bbhtx_bb_route_ovr,
    cli_qm_sbnfifo0,
    cli_qm_sbnfifo1,
    cli_qm_sbnfifoprefetch0,
    cli_qm_sbnfifoprefetch1,
    cli_qm_esbpmstat,
    cli_qm_esbpmstat2,
    cli_qm_cm_ddr_burst_fifo_status,
    cli_qm_cm_psr_burst_fifo_status,
    cli_qm_cm_psr_wr_pd_fifo_status,
    cli_qm_cm_sec_bn_fifo_status,
    cli_qm_cm_ing_rd_buf_fifo_status,
    cli_qm_cm_cbpm_fifo_status,
    cli_qm_cm_egr_wrr_status,
    cli_qm_cm_ddr_egr_fifo_status,
    cli_qm_cm_psr_egr_fifo_status,
    cli_qm_bb_routh_ovr_addr,
    cli_qm_bbhtx_bb_rough_ovr_addr,
    cli_qm_dqm_fll_mem_wr_ptr,
    cli_qm_dqm_fll_mem_rd_ptr,
    cli_qm_dqm_fll_buf_avlbl,
    cli_qm_dqm_fll_buf_avlbl_min,
    cli_qm_dqm_dbg_cntr_cfg,
    cli_qm_dqm_dbg_push_ack_cntr,
    cli_qm_dqm_dbg_push_done_cntr,
    cli_qm_dqm_dbg_pop_req_cntr,
    cli_qm_dqm_dbg_pop_vld_cntr,
    cli_qm_dqm_dbg_stall_cntr,
    cli_qm_bb0_ing_free_msg_fifo_status,
    cli_qm_dqm_pop_pd_data_fifo_status,
    cli_qm_mc_first_cnt,
    cli_qm_mc_rep_cnt,
    cli_qm_mc_first_drop_cnt,
    cli_qm_mc_body_drop_cnt,
    cli_qm_global_drop_cnt,
    cli_qm_qm_eng_spare_config,
    cli_qm_fpm_grp_drop_cnt,
    cli_qm_cm_residue_data,
    cli_qm_total_valid_cnt,
    cli_qm_drop_counter,
    cli_qm_dqm_valid_cnt,
    cli_qm_accumulated_counter,
    cli_qm_q_context,
    cli_qm_pdfifo_ptr,
    cli_qm_update_fifo_ptr,
};

int bcm_qm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_qm_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
