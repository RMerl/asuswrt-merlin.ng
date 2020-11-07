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

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* ddr_byte_congestion_drop_enable: DDR_BYTE_CONGESTION_DROP_ENABLE - This field indicates whethe */
/*                                  r crossing the DDR bytes thresholds (the number of bytes wait */
/*                                  ing to be copied to DDR) will result in dropping packets or i */
/*                                  n applying back pressure to the re-order.0 - apply back press */
/*                                  ure1 - drop packets                                           */
/* ddr_bytes_lower_thr: DDR_BYTES_LOWER_THR - DDR copy bytes Lower Threshold.When working in pack */
/*                      et drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy by */
/*                      tes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.* If  */
/*                      (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), */
/*                       then packets in low/high priority are dropped (only exclusive packets ar */
/*                      e not dropped).* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= ( */
/*                      DDR_BYTES_MID_THR), then packets in low priority are dropped.* If (DDR co */
/*                      py bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped. */
/*                      When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), T */
/*                      hen if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressu */
/*                      re is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES */
/*                      _MID_THR are dont care).                                                  */
/* ddr_bytes_mid_thr: DDR_BYTES_MID_THR - DDR copy bytes Lower Threshold.When working in packet d */
/*                    rop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy bytes co */
/*                    unter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.* If (DDR_BYT */
/*                    ES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then pack */
/*                    ets in low/high priority are dropped (only exclusive packets are not droppe */
/*                    d).* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_ */
/*                    THR), then packets in low priority are dropped.* If (DDR copy bytes counter */
/*                    ) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.When working in ba */
/*                    ckpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy byt */
/*                    es counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-or */
/*                    der (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care). */
/* ddr_bytes_higher_thr: DDR_BYTES_HIGHER_THR - DDR copy bytes Lower Threshold.When working in pa */
/*                       cket drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy */
/*                        bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.* */
/*                        If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_ */
/*                       THR), then packets in low/high priority are dropped (only exclusive pack */
/*                       ets are not dropped).* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counte */
/*                       r) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped.* If */
/*                        (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are */
/*                        dropped.When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_EN */
/*                       ABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then */
/*                        backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR a */
/*                       nd DDR_BYTES_MID_THR are dont care).                                     */
/* ddr_pd_congestion_drop_enable: DDR_PD_CONGESTION_DROP_ENABLE - This field indicates whether cr */
/*                                ossing the DDR Pipe thresholds will result in dropping packets  */
/*                                or in applying back pressure to the re-order.0 - apply back pre */
/*                                ssure1 - drop packets                                           */
/* ddr_pipe_lower_thr: DDR_PIPE_LOWER_THR - DDR copy Pipe Lower Threshold.When working in packet  */
/*                     drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy pipe occu */
/*                     pancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are drop */
/*                     ped (only exclusive packets are not dropped).* If (DDR_PIPE_LOWER_THR) < ( */
/*                     DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low pri */
/*                     ority are dropped.* If (DDR copy pipe occupancy) <=  (DDR_PIPE_LOWER_THR), */
/*                      then no packets are dropped.When working in backpressure mode (DDR_PD_CON */
/*                     GESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_PIPE_HIGH */
/*                     ER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_L */
/*                     OWER_THR is dont care).                                                    */
/* ddr_pipe_higher_thr: DDR_PIPE_HIGHER_THR - DDR copy Pipe Lower Threshold.When working in packe */
/*                      t drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy pipe o */
/*                      ccupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are  */
/*                      dropped (only exclusive packets are not dropped).* If (DDR_PIPE_LOWER_THR */
/*                      ) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in l */
/*                      ow priority are dropped.* If (DDR copy pipe occupancy) <= (DDR_PIPE_LOWER */
/*                      _THR), then no packets are dropped.When working in backpressure mode (DDR */
/*                      _PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_P */
/*                      IPE_HIGHER_THR), then backpressure is applied to re-order (in this case D */
/*                      DR_PIPE_LOWER_THR is dont care).IMPORTANT: recommended maximum value is 0 */
/*                      x7B in order to avoid performance degradation when working with aggregati */
/*                      on timeout enable                                                         */
/**************************************************************************************************/
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


/**************************************************************************************************/
/* lower_thr: FPM_GRP_LOWER_THR - FPM group Lower Threshold.* If (FPM User Group Counter) > (FPM_ */
/*            GRP_HIGHER_THR), all packets in this user group are dropped.* If (FPM_GRP_MID_THR)  */
/*            < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priori */
/*            ty are dropped (only exclusive packets are not dropped).* If (FPM_GRP_LOWER_THR) <  */
/*            (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dro */
/*            pped.* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are drop */
/*            ped.                                                                                */
/* mid_thr: FPM_GRP_MID_THR - FPM group Lower Threshold.* If (FPM User Group Counter) > (FPM_GRP_ */
/*          HIGHER_THR), all packets in this user group are dropped.* If (FPM_GRP_MID_THR) < (FPM */
/*           User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are d */
/*          ropped (only exclusive packets are not dropped).* If (FPM_GRP_LOWER_THR) < (FPM User  */
/*          Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped.* If (F */
/*          PM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.           */
/* higher_thr: FPM_GRP_HIGHER_THR - FPM group Lower Threshold.* If (FPM User Group Counter) > (FP */
/*             M_GRP_HIGHER_THR), all packets in this user group are dropped.* If (FPM_GRP_MID_TH */
/*             R) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high pr */
/*             iority are dropped (only exclusive packets are not dropped).* If (FPM_GRP_LOWER_TH */
/*             R) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority a */
/*             re dropped.* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets a */
/*             re dropped.                                                                        */
/**************************************************************************************************/
typedef struct
{
    uint16_t lower_thr;
    uint16_t mid_thr;
    uint16_t higher_thr;
} qm_fpm_ug_thr;


/**************************************************************************************************/
/* start_queue: START_QUEUE - Indicates the Queue that starts this runner group. Queues belonging */
/*               to the runner group are defined by the following equation:START_QUEUE <= runner_ */
/*              queues <= END_QUEUE                                                               */
/* end_queue: END_QUEUE - Indicates the Queue that ends this runner group.Queues belonging to the */
/*             runner group are defined by the following equation:START_QUEUE <= runner_queues <= */
/*             END_QUEUE                                                                          */
/* pd_fifo_base: BASE_ADDR - PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_AD */
/*               DR*8).                                                                           */
/* pd_fifo_size: SIZE - PD FIFO Size0 - 2 entries1 - 4 entries2 - 8 entries                       */
/* upd_fifo_base: BASE_ADDR - PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_A */
/*                DDR*8).                                                                         */
/* upd_fifo_size: SIZE - PD FIFO Size0 - 8 entries1 - 16 entries2 - 32 entries3 - 64 entries4 - 1 */
/*                28 entries5 - 256 entries                                                       */
/* rnr_bb_id: RNR_BB_ID - Runner BB ID associated with this configuration.                        */
/* rnr_task: RNR_TASK - Runner Task number to be woken up when the update FIFO is written to.     */
/* rnr_enable: RNR_ENABLE - Enable this runner interface                                          */
/**************************************************************************************************/
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


/**************************************************************************************************/
/* min_thr0: MIN_THR - WRED Color Min Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* flw_ctrl_en0: FLW_CTRL_EN - 0 - flow control disable. regular WRED profile1 - flow control ena */
/*               ble. no WRED drop, wake up appropriate runner task when crossed.                 */
/* min_thr1: MIN_THR - WRED Color Min Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* flw_ctrl_en1: FLW_CTRL_EN - 0 - flow control disable. regular WRED profile1 - flow control ena */
/*               ble. no WRED drop, wake up appropriate runner task when crossed.                 */
/* max_thr0: MAX_THR - WRED Color Max Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* max_thr1: MAX_THR - WRED Color Max Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* slope_mantissa0: SLOPE_MANTISSA - WRED Color slope mantissa.                                   */
/* slope_exp0: SLOPE_EXP - WRED Color slope exponent.                                             */
/* slope_mantissa1: SLOPE_MANTISSA - WRED Color slope mantissa.                                   */
/* slope_exp1: SLOPE_EXP - WRED Color slope exponent.                                             */
/**************************************************************************************************/
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


/**************************************************************************************************/
/* fpm_prefetch_enable: FPM_PREFETCH_ENABLE - FPM Prefetch Enable. Setting this bit to 1 will sta */
/*                      rt filling up the FPM pool prefetch FIFOs.Seeting this bit to 0, will sto */
/*                      p FPM prefetches.                                                         */
/* reorder_credit_enable: REORDER_CREDIT_ENABLE - When this bit is set the QM will send credits t */
/*                        o the REORDER block.Disabling this bit will stop sending credits to the */
/*                         reorder.                                                               */
/* dqm_pop_enable: DQM_POP_ENABLE - When this bit is set the QM will pop PDs from the DQM and pla */
/*                 ce them in the runner SRAM                                                     */
/* rmt_fixed_arb_enable: RMT_FIXED_ARB_ENABLE - When this bit is set Fixed arbitration will be do */
/*                       ne in pops from the remote FIFOs (Non delayed highest priority). If this */
/*                        bit is cleared RR arbitration is done                                   */
/* dqm_push_fixed_arb_enable: DQM_PUSH_FIXED_ARB_ENABLE - When this bit is set Fixed arbitration  */
/*                            will be done in DQM pushes (CPU highest priority, then non-delayed  */
/*                            queues and then normal queues. If this bit is cleared RR arbitratio */
/*                            n is done.                                                          */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean fpm_prefetch_enable;
    bdmf_boolean reorder_credit_enable;
    bdmf_boolean dqm_pop_enable;
    bdmf_boolean rmt_fixed_arb_enable;
    bdmf_boolean dqm_push_fixed_arb_enable;
} qm_enable_ctrl;


/**************************************************************************************************/
/* fpm_prefetch0_sw_rst: FPM_PREFETCH0_SW_RST - FPM Prefetch FIFO0 SW reset.                      */
/* fpm_prefetch1_sw_rst: FPM_PREFETCH1_SW_RST - FPM Prefetch FIFO1 SW reset.                      */
/* fpm_prefetch2_sw_rst: FPM_PREFETCH2_SW_RST - FPM Prefetch FIFO2 SW reset.                      */
/* fpm_prefetch3_sw_rst: FPM_PREFETCH3_SW_RST - FPM Prefetch FIFO3 SW reset.                      */
/* normal_rmt_sw_rst: NORMAL_RMT_SW_RST - Normal Remote FIFO SW reset.                            */
/* non_delayed_rmt_sw_rst: NON_DELAYED_RMT_SW_RST - Non-delayed Remote FIFO SW reset.             */
/* pre_cm_fifo_sw_rst: PRE_CM_FIFO_SW_RST - Pre Copy Machine FIFO SW reset.                       */
/* cm_rd_pd_fifo_sw_rst: CM_RD_PD_FIFO_SW_RST - Copy Machine RD PD FIFO SW reset.                 */
/* cm_wr_pd_fifo_sw_rst: CM_WR_PD_FIFO_SW_RST - Pre Copy Machine FIFO SW reset.                   */
/* bb0_output_fifo_sw_rst: BB0_OUTPUT_FIFO_SW_RST - BB0 OUTPUT FIFO SW reset.                     */
/* bb1_output_fifo_sw_rst: BB1_OUTPUT_FIFO_SW_RST - BB1 Output FIFO SW reset.                     */
/* bb1_input_fifo_sw_rst: BB1_INPUT_FIFO_SW_RST - BB1 Input FIFO SW reset.                        */
/* tm_fifo_ptr_sw_rst: TM_FIFO_PTR_SW_RST - TM FIFOs Pointers SW reset.                           */
/* non_delayed_out_fifo_sw_rst: NON_DELAYED_OUT_FIFO_SW_RST - Non delayed output FIFO Pointers SW */
/*                               reset.                                                           */
/**************************************************************************************************/
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
} qm_reset_ctrl;


/**************************************************************************************************/
/* read_clear_pkts: DROP_CNT_PKTS_READ_CLEAR_ENABLE - Indicates whether the Drop/max_occupancy pa */
/*                  ckets counter is read clear.                                                  */
/* read_clear_bytes: DROP_CNT_BYTES_READ_CLEAR_ENABLE - Indicates whether the Drop/max_occupancy  */
/*                   bytes counter is read clear.                                                 */
/* disable_wrap_around_pkts: DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT - This bit defines the functional */
/*                           ity of the drop packets counter.0 - Functions as the drop packets co */
/*                           unter1 - Functions as the max packets occupancy holder               */
/* disable_wrap_around_bytes: DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT - This bit defines the function */
/*                            ality of the drop bytes counter.0 - Functions as the drop bytes cou */
/*                            nter1 - Functions as the max bytes occupancy holder                 */
/* free_with_context_last_search: FREE_WITH_CONTEXT_LAST_SEARCH - Indicates The value to put in t */
/*                                he last_search field of the SBPM free with context message      */
/* wred_disable: WRED_DISABLE - Disables WRED influence on drop condition                         */
/* ddr_pd_congestion_disable: DDR_PD_CONGESTION_DISABLE - Disables DDR_PD_CONGESTION influence on */
/*                             drop/bpcondition                                                   */
/* ddr_byte_congestion_disable: DDR_BYTE_CONGESTION_DISABLE - Disables DDR_BYTE_CONGESTION influe */
/*                              nce on drop/bp condition                                          */
/* ddr_occupancy_disable: DDR_OCCUPANCY_DISABLE - Disables DDR_OCCUPANCY influence on drop/bp con */
/*                        dition                                                                  */
/* ddr_fpm_congestion_disable: DDR_FPM_CONGESTION_DISABLE - Disables DDR_FPM_CONGESTION influence */
/*                              on drop/bp condition                                              */
/* fpm_ug_disable: FPM_UG_DISABLE - Disables FPM_UG influence on drop condition                   */
/* queue_occupancy_ddr_copy_decision_disable: QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE - Disable */
/*                                            s QUEUE_OCCUPANCY_DDR_COPY_DECISION influence on co */
/*                                            py condition                                        */
/* psram_occupancy_ddr_copy_decision_disable: PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE - Disable */
/*                                            s PSRAM_OCCUPANCY_DDR_COPY_DECISION influence on co */
/*                                            py condition                                        */
/* dont_send_mc_bit_to_bbh: DONT_SEND_MC_BIT_TO_BBH - When set, the multicast bit of the PD will  */
/*                          not be sent to BBH TX                                                 */
/* close_aggregation_on_timeout_disable: CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE - When set, aggrega */
/*                                       tions are not closed automatically when queue open aggre */
/*                                       gation time expired.                                     */
/* fpm_congestion_buf_release_mechanism_disable: FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE - W */
/*                                               hen cleared, given that there is an FPM congesti */
/*                                               on situation and all prefetch FPM buffers are fu */
/*                                               ll then a min pool size buffer will be freed eac */
/*                                               h 1us. This is done due to the fact that exclusi */
/*                                               ve indication is received only togeter with buff */
/*                                               er allocation reply and if this will not be done */
/*                                                then a deadlock could occur.Setting this bit wi */
/*                                               ll disable this mechanism.                       */
/* fpm_buffer_global_res_enable: FPM_BUFFER_GLOBAL_RES_ENABLE - FPM over subscription mechanism.E */
/*                               ach queue will have one out of 8 reserved byte threshold profile */
/*                               s. Each profile defines 8 bit threshold with 512byte resolution. */
/*                               Once the global FPM counter pass configurable threshold the syst */
/*                               em goes to buffer reservation congestion state. In this state an */
/*                               y PD entering a queue which passes the reserved byte threshold w */
/*                               ill be dropped.                                                  */
/* qm_preserve_pd_with_fpm: QM_PRESERVE_PD_WITH_FPM - Dont drop pd with fpm allocation.           */
/* qm_residue_per_queue: QM_RESIDUE_PER_QUEUE - 6878:1 for 32B/Queue0 debug - no residueOther pro */
/*                       jects:0 for 64B/Queue1 for 128B/Queue                                    */
/* ghost_rpt_update_after_close_agg_en: GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN - Controls the timing */
/*                                       of updating the overhead counters with packets which goe */
/*                                      s through aggregation.0 - updates when the packets enters */
/*                                       QM1 - updates when aggregation is done.                  */
/* fpm_ug_flow_ctrl_disable: FPM_UG_FLOW_CTRL_DISABLE - Disables FPM_UG influence on flow control */
/*                            wake up messages to FW.                                             */
/* ddr_write_multi_slave_en: DDR_WRITE_MULTI_SLAVE_EN - Enables to write packet transaction to mu */
/*                           ltiple slave (unlimited), if disable only one ubus slave allowed.    */
/* ddr_pd_congestion_agg_priority: DDR_PD_CONGESTION_AGG_PRIORITY - global priority bit to aggreg */
/*                                 ated PDs which go through reprocessing.                        */
/* psram_occupancy_drop_disable: PSRAM_OCCUPANCY_DROP_DISABLE - Disables PSRAM_OCCUPANCY_DROP inf */
/*                               luence on drop condition                                         */
/* qm_ddr_write_alignment: QM_DDR_WRITE_ALIGNMENT - 0 According to length1 8-byte aligned         */
/* exclusive_dont_drop: EXCLUSIVE_DONT_DROP - Controls if the exclusive indication in PD marks th */
/*                      e PD as dont drop or as dont drop if the fpm in exclusive state1 - global */
/*                       dont drop0 - FPM exclusive state dont drop                               */
/* exclusive_dont_drop_bp_en: EXCLUSIVE_DONT_DROP_BP_EN - when set 1 backpressure will be applied */
/*                             when the DONT_DROP pd should be dropped.for example, 0 fpm buffers */
/*                             available and the PD should be copied to DDR.                      */
/* gpon_dbr_ceil: GPON_DBR_CEIL - If the bit enable, QM round up to 4 every packet length added g */
/*                host counters.                                                                  */
/* drop_cnt_wred_drops: DROP_CNT_WRED_DROPS - Drop counter counts WRED drops by color per queue.I */
/*                      n order to enable this feature the drop counter should be configured to c */
/*                      ount drops. if the drop counter is configured count max occupancy per que */
/*                      ue, it will override WRED drops count.color 0 - is written in dropped byt */
/*                      es field (word0)color 1 - is written in dropped pkts field (word1)        */
/**************************************************************************************************/
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
    bdmf_boolean exclusive_dont_drop_bp_en;
    bdmf_boolean gpon_dbr_ceil;
    bdmf_boolean drop_cnt_wred_drops;
} qm_drop_counters_ctrl;


/**************************************************************************************************/
/* epon_line_rate: EPON_LINE_RATE - EPON Line Rate0 - 1G1 - 10G                                   */
/* epon_crc_add_disable: EPON_CRC_ADD_DISABLE - If this bit is not set then 4-bytes will be added */
/*                        to the ghost reporting accumulated bytes and to the byte overhead calcu */
/*                       lation input                                                             */
/* mac_flow_overwrite_crc_en: MAC_FLOW_OVERWRITE_CRC_EN - Enables to overwrite CRC addition speci */
/*                            fied MAC FLOW in the field below.                                   */
/* mac_flow_overwrite_crc: MAC_FLOW_OVERWRITE_CRC - MAC flow ID to force disable CRC addition     */
/* fec_ipg_length: FEC_IPG_LENGTH - FEC IPG Length                                                */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean epon_line_rate;
    bdmf_boolean epon_crc_add_disable;
    bdmf_boolean mac_flow_overwrite_crc_en;
    uint8_t mac_flow_overwrite_crc;
    uint16_t fec_ipg_length;
} qm_epon_overhead_ctrl;


/**************************************************************************************************/
/* lower_thr: FPM_LOWER_THR - FPM Lower Threshold.When working in packet drop mode (FPM_BP_ENABLE */
/*            =0), Then:* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high pr */
/*            iority are dropped (only exclusive packets are not dropped).* If (FPM_LOWER_THR) <  */
/*            (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped. */
/*            * If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.When work */
/*            ing in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOW */
/*            ER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER_THR is d */
/*            ont care).                                                                          */
/* higher_thr: FPM_HIGHER_THR - FPM Higher Threshold.When working in packet drop mode (FPM_BP_ENA */
/*             BLE=0), Then:* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/hig */
/*             h priority are dropped (only exclusive packets are not dropped).* If (FPM_LOWER_TH */
/*             R) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dr */
/*             opped.* If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.Wh */
/*             en working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) <  */
/*             (FPM_LOWER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER */
/*             _THR is dont care).                                                                */
/**************************************************************************************************/
typedef struct
{
    uint8_t lower_thr;
    uint8_t higher_thr;
} qm_fpm_pool_thr;


/**************************************************************************************************/
/* qm_dqm_pop_on_empty: QM_DQM_POP_ON_EMPTY - HW tried to pop a PD from the DQM of an empty queue */
/*                      .                                                                         */
/* qm_dqm_push_on_full: QM_DQM_PUSH_ON_FULL - HW tried to pop a PD into the DQM of a full queue.  */
/* qm_cpu_pop_on_empty: QM_CPU_POP_ON_EMPTY - CPU tried to pop a PD from the DQM of an empty queu */
/*                      e.                                                                        */
/* qm_cpu_push_on_full: QM_CPU_PUSH_ON_FULL - CPU tried to push a PD into the DQM of a full queue */
/*                      .                                                                         */
/* qm_normal_queue_pd_no_credit: QM_NORMAL_QUEUE_PD_NO_CREDIT - A PD arrived to the Normal queue  */
/*                               without having any credits                                       */
/* qm_non_delayed_queue_pd_no_credit: QM_NON_DELAYED_QUEUE_PD_NO_CREDIT - A PD arrived to the NON */
/*                                    -delayed queue without having any credits                   */
/* qm_non_valid_queue: QM_NON_VALID_QUEUE - A PD arrived with a non valid queue number (>287)     */
/* qm_agg_coherent_inconsistency: QM_AGG_COHERENT_INCONSISTENCY - An aggregation of PDs was done  */
/*                                in which the coherent bit of the PD differs between them (The c */
/*                                oherent bit of the first aggregated PD was used)                */
/* qm_force_copy_on_non_delayed: QM_FORCE_COPY_ON_NON_DELAYED - A PD with force copy bit set was  */
/*                               received on the non-delayed queue (in this queue the copy machin */
/*                               e is bypassed)                                                   */
/* qm_fpm_pool_size_nonexistent: QM_FPM_POOL_SIZE_NONEXISTENT - A PD was marked to be copied, but */
/*                                there does not exist an FPM pool buffer large enough to hold it */
/*                               .                                                                */
/* qm_target_mem_abs_contradiction: QM_TARGET_MEM_ABS_CONTRADICTION - A PD was marked with a targ */
/*                                  et_mem=1 (located in PSRAM) and on the other hand, the absolu */
/*                                  te address indication was set.                                */
/* qm_1588_drop: QM_1588_DROP - 1588 Packet is dropped when the QM PD occupancy exceeds threshold */
/*                (64K)                                                                           */
/* qm_1588_multicast_contradiction: QM_1588_MULTICAST_CONTRADICTION - A PD was marked as a 1588 a */
/*                                  nd multicast together.                                        */
/* qm_byte_drop_cnt_overrun: QM_BYTE_DROP_CNT_OVERRUN - The byte drop counter of one of the queue */
/*                           s reached its maximum value and a new value was pushed.              */
/* qm_pkt_drop_cnt_overrun: QM_PKT_DROP_CNT_OVERRUN - The Packet drop counter of one of the queue */
/*                          s reached its maximum value and a new value was pushed.               */
/* qm_total_byte_cnt_underrun: QM_TOTAL_BYTE_CNT_UNDERRUN - The Total byte counter was decremente */
/*                             d to a negative value.                                             */
/* qm_total_pkt_cnt_underrun: QM_TOTAL_PKT_CNT_UNDERRUN - The Total PD counter was decremented to */
/*                             a negative value.                                                  */
/* qm_fpm_ug0_underrun: QM_FPM_UG0_UNDERRUN - The UG0 counter was decremented to a negative value */
/*                      .                                                                         */
/* qm_fpm_ug1_underrun: QM_FPM_UG1_UNDERRUN - The UG1 counter was decremented to a negative value */
/*                      .                                                                         */
/* qm_fpm_ug2_underrun: QM_FPM_UG2_UNDERRUN - The UG2 counter was decremented to a negative value */
/*                      .                                                                         */
/* qm_fpm_ug3_underrun: QM_FPM_UG3_UNDERRUN - The UG3 counter was decremented to a negative value */
/*                      .                                                                         */
/* qm_timer_wraparound: QM_TIMER_WRAPAROUND - QM aggregation timers wraps around. In this case it */
/*                       isnt guaranteed that the aggregation will be closed on pre-defined timeo */
/*                      ut expiration. However the aggregation should be closed eventually.       */
/* qm_copy_plen_zero: QM_COPY_PLEN_ZERO - Packet with length = 0 is copied to DDR. FW/SW should t */
/*                    ake care that zero length packets arent copied to DDR.                      */
/**************************************************************************************************/
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
} qm_intr_ctrl_isr;


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
} qm_clk_gate_clk_gate_cntrl;


/**************************************************************************************************/
/* wred_profile: WRED_PROFILE - Defines to which WRED Profile this queue belongs to.              */
/* copy_dec_profile: COPY_DEC_PROFILE - Defines to which Copy Decision Profile this queue belongs */
/*                    to.                                                                         */
/* copy_to_ddr: COPY_TO_DDR - Defines this queue to always copy to DDR.                           */
/* ddr_copy_disable: DDR_COPY_DISABLE - Defines this queue never to copy to DDR.                  */
/* aggregation_disable: AGGREGATION_DISABLE - Defines this queue never to aggregated PDs.         */
/* fpm_ug: FPM_UG - Defines to which FPM UG this queue belongs to.                                */
/* exclusive_priority: EXCLUSIVE_PRIORITY - Defines this queue with exclusive priority.           */
/* q_802_1ae: Q_802_1AE - Defines this queue as 802.1AE for EPON packet overhead calculations.    */
/* sci: SCI - Configures SCI for EPON packet overhead calculations.                               */
/* fec_enable: FEC_ENABLE - FEC enable configuration for EPON packet overhead calculations.       */
/* res_profile: RES_PROFILE - FPM reservation profile.Once the QM goes over global FPM reservatio */
/*              n threshold.Queue with more bytes the defined in the profile will be dropped.Prof */
/*              ile 0 means no drop due to FPM reservation for the queues with this profile.      */
/**************************************************************************************************/
typedef struct
{
    uint8_t wred_profile;
    uint8_t copy_dec_profile;
    bdmf_boolean copy_to_ddr;
    bdmf_boolean ddr_copy_disable;
    bdmf_boolean aggregation_disable;
    uint8_t fpm_ug;
    bdmf_boolean exclusive_priority;
    bdmf_boolean q_802_1ae;
    bdmf_boolean sci;
    bdmf_boolean fec_enable;
    uint8_t res_profile;
} qm_q_context;


/**************************************************************************************************/
/* ug0: UG0 - User group 0 status                                                                 */
/* ug1: UG1 - User group 1 status                                                                 */
/* ug2: UG2 - User group 2 status                                                                 */
/* ug3: UG3 - User group 3 status                                                                 */
/* wred: WRED - OR on all wred flow control queues                                                */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean ug0;
    bdmf_boolean ug1;
    bdmf_boolean ug2;
    bdmf_boolean ug3;
    bdmf_boolean wred;
} qm_flow_ctrl_status;


/**************************************************************************************************/
/* rnr_bb_id: RNR_BB_ID - Runner BB ID                                                            */
/* rnr_task: RNR_TASK - Runner task                                                               */
/* rnr_enable: RNR_ENABLE - Runner enable.if disable, the lossless flow control is disabled.      */
/* sram_wr_en: SRAM_WR_EN - If set, the wake up messages data is written to SRAM_ADDR             */
/* sram_wr_addr: SRAM_WR_ADDR - Sram address to write the vector                                  */
/**************************************************************************************************/
typedef struct
{
    uint8_t rnr_bb_id;
    uint8_t rnr_task;
    bdmf_boolean rnr_enable;
    bdmf_boolean sram_wr_en;
    uint16_t sram_wr_addr;
} qm_flow_ctrl_qm_flow_ctrl_rnr_cfg;

bdmf_error_t ag_drv_qm_ddr_cong_ctrl_set(const qm_ddr_cong_ctrl *ddr_cong_ctrl);
bdmf_error_t ag_drv_qm_ddr_cong_ctrl_get(qm_ddr_cong_ctrl *ddr_cong_ctrl);
bdmf_error_t ag_drv_qm_is_queue_not_empty_get(uint16_t q_idx, bdmf_boolean *data);
bdmf_error_t ag_drv_qm_is_queue_pop_ready_get(uint16_t q_idx, bdmf_boolean *data);
bdmf_error_t ag_drv_qm_is_queue_full_get(uint16_t q_idx, bdmf_boolean *data);
bdmf_error_t ag_drv_qm_fpm_ug_thr_set(uint8_t ug_grp_idx, const qm_fpm_ug_thr *fpm_ug_thr);
bdmf_error_t ag_drv_qm_fpm_ug_thr_get(uint8_t ug_grp_idx, qm_fpm_ug_thr *fpm_ug_thr);
bdmf_error_t ag_drv_qm_rnr_group_cfg_set(uint8_t rnr_idx, const qm_rnr_group_cfg *rnr_group_cfg);
bdmf_error_t ag_drv_qm_rnr_group_cfg_get(uint8_t rnr_idx, qm_rnr_group_cfg *rnr_group_cfg);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_set(uint8_t indirect_grp_idx, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_rd_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3);
bdmf_error_t ag_drv_qm_aggr_context_get(uint16_t idx, uint32_t *context_valid);
bdmf_error_t ag_drv_qm_wred_profile_cfg_set(uint8_t profile_idx, const qm_wred_profile_cfg *wred_profile_cfg);
bdmf_error_t ag_drv_qm_wred_profile_cfg_get(uint8_t profile_idx, qm_wred_profile_cfg *wred_profile_cfg);
bdmf_error_t ag_drv_qm_enable_ctrl_set(const qm_enable_ctrl *enable_ctrl);
bdmf_error_t ag_drv_qm_enable_ctrl_get(qm_enable_ctrl *enable_ctrl);
bdmf_error_t ag_drv_qm_reset_ctrl_set(const qm_reset_ctrl *reset_ctrl);
bdmf_error_t ag_drv_qm_reset_ctrl_get(qm_reset_ctrl *reset_ctrl);
bdmf_error_t ag_drv_qm_drop_counters_ctrl_set(const qm_drop_counters_ctrl *drop_counters_ctrl);
bdmf_error_t ag_drv_qm_drop_counters_ctrl_get(qm_drop_counters_ctrl *drop_counters_ctrl);
bdmf_error_t ag_drv_qm_fpm_ctrl_set(bdmf_boolean fpm_pool_bp_enable, bdmf_boolean fpm_congestion_bp_enable, uint8_t fpm_prefetch_min_pool_size, uint8_t fpm_prefetch_pending_req_limit);
bdmf_error_t ag_drv_qm_fpm_ctrl_get(bdmf_boolean *fpm_pool_bp_enable, bdmf_boolean *fpm_congestion_bp_enable, uint8_t *fpm_prefetch_min_pool_size, uint8_t *fpm_prefetch_pending_req_limit);
bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_set(uint32_t total_pd_thr);
bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_get(uint32_t *total_pd_thr);
bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_set(uint16_t abs_drop_queue, bdmf_boolean abs_drop_queue_en);
bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_get(uint16_t *abs_drop_queue, bdmf_boolean *abs_drop_queue_en);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_set(uint16_t max_agg_bytes, uint8_t max_agg_pkts);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_get(uint16_t *max_agg_bytes, uint8_t *max_agg_pkts);
bdmf_error_t ag_drv_qm_fpm_base_addr_set(uint32_t fpm_base_addr);
bdmf_error_t ag_drv_qm_fpm_base_addr_get(uint32_t *fpm_base_addr);
bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(uint32_t fpm_base_addr);
bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_get(uint32_t *fpm_base_addr);
bdmf_error_t ag_drv_qm_ddr_sop_offset_set(uint16_t ddr_sop_offset0, uint16_t ddr_sop_offset1);
bdmf_error_t ag_drv_qm_ddr_sop_offset_get(uint16_t *ddr_sop_offset0, uint16_t *ddr_sop_offset1);
bdmf_error_t ag_drv_qm_epon_overhead_ctrl_set(const qm_epon_overhead_ctrl *epon_overhead_ctrl);
bdmf_error_t ag_drv_qm_epon_overhead_ctrl_get(qm_epon_overhead_ctrl *epon_overhead_ctrl);
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(uint8_t prescaler_granularity, uint8_t aggregation_timeout_value, bdmf_boolean pd_occupancy_en, uint8_t pd_occupancy_value);
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(uint8_t *prescaler_granularity, uint8_t *aggregation_timeout_value, bdmf_boolean *pd_occupancy_en, uint8_t *pd_occupancy_value);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(uint16_t fpm_gbl_cnt);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(uint16_t *fpm_gbl_cnt);
bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_set(uint16_t queue_num, bdmf_boolean flush_en);
bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_get(uint16_t *queue_num, bdmf_boolean *flush_en);
bdmf_error_t ag_drv_qm_fpm_pool_thr_set(uint8_t pool_idx, const qm_fpm_pool_thr *fpm_pool_thr);
bdmf_error_t ag_drv_qm_fpm_pool_thr_get(uint8_t pool_idx, qm_fpm_pool_thr *fpm_pool_thr);
bdmf_error_t ag_drv_qm_fpm_ug_cnt_set(uint8_t grp_idx, uint16_t fpm_ug_cnt);
bdmf_error_t ag_drv_qm_fpm_ug_cnt_get(uint8_t grp_idx, uint16_t *fpm_ug_cnt);
bdmf_error_t ag_drv_qm_intr_ctrl_isr_set(const qm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_qm_intr_ctrl_isr_get(qm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_qm_intr_ctrl_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_qm_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_qm_intr_ctrl_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_qm_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_qm_intr_ctrl_itr_get(uint32_t *ist);
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_set(const qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_get(qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(uint8_t indirect_grp_idx, uint16_t queue_num, uint8_t cmd, bdmf_boolean done, bdmf_boolean error);
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(uint8_t indirect_grp_idx, uint16_t *queue_num, uint8_t *cmd, bdmf_boolean *done, bdmf_boolean *error);
bdmf_error_t ag_drv_qm_q_context_set(uint16_t q_idx, const qm_q_context *q_context);
bdmf_error_t ag_drv_qm_q_context_get(uint16_t q_idx, qm_q_context *q_context);
bdmf_error_t ag_drv_qm_copy_decision_profile_set(uint8_t profile_idx, uint32_t queue_occupancy_thr, bdmf_boolean psram_thr);
bdmf_error_t ag_drv_qm_copy_decision_profile_get(uint8_t profile_idx, uint32_t *queue_occupancy_thr, bdmf_boolean *psram_thr);
bdmf_error_t ag_drv_qm_total_valid_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_total_valid_cnt_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_dqm_valid_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_dqm_valid_cnt_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_drop_counter_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_epon_q_byte_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_epon_q_byte_cnt_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_epon_q_status_get(uint16_t q_idx, uint32_t *status_bit_vector);
bdmf_error_t ag_drv_qm_rd_data_pool0_get(uint32_t *data);
bdmf_error_t ag_drv_qm_rd_data_pool1_get(uint32_t *data);
bdmf_error_t ag_drv_qm_rd_data_pool2_get(uint32_t *data);
bdmf_error_t ag_drv_qm_rd_data_pool3_get(uint32_t *data);
bdmf_error_t ag_drv_qm_pdfifo_ptr_get(uint16_t q_idx, uint8_t *wr_ptr, uint8_t *rd_ptr);
bdmf_error_t ag_drv_qm_update_fifo_ptr_get(uint16_t fifo_idx, uint16_t *wr_ptr, uint8_t *rd_ptr);
bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_set(uint32_t idx, uint16_t data);
bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_get(uint32_t idx, uint16_t *data);
bdmf_error_t ag_drv_qm_flow_ctrl_ug_ctrl_set(bdmf_boolean flow_ctrl_ug0_en, bdmf_boolean flow_ctrl_ug1_en, bdmf_boolean flow_ctrl_ug2_en, bdmf_boolean flow_ctrl_ug3_en);
bdmf_error_t ag_drv_qm_flow_ctrl_ug_ctrl_get(bdmf_boolean *flow_ctrl_ug0_en, bdmf_boolean *flow_ctrl_ug1_en, bdmf_boolean *flow_ctrl_ug2_en, bdmf_boolean *flow_ctrl_ug3_en);
bdmf_error_t ag_drv_qm_flow_ctrl_status_set(const qm_flow_ctrl_status *flow_ctrl_status);
bdmf_error_t ag_drv_qm_flow_ctrl_status_get(qm_flow_ctrl_status *flow_ctrl_status);
bdmf_error_t ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_set(const qm_flow_ctrl_qm_flow_ctrl_rnr_cfg *flow_ctrl_qm_flow_ctrl_rnr_cfg);
bdmf_error_t ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_get(qm_flow_ctrl_qm_flow_ctrl_rnr_cfg *flow_ctrl_qm_flow_ctrl_rnr_cfg);
bdmf_error_t ag_drv_qm_debug_sel_set(uint8_t select, bdmf_boolean enable);
bdmf_error_t ag_drv_qm_debug_sel_get(uint8_t *select, bdmf_boolean *enable);
bdmf_error_t ag_drv_qm_debug_bus_lsb_get(uint32_t *data);
bdmf_error_t ag_drv_qm_debug_bus_msb_get(uint32_t *data);
bdmf_error_t ag_drv_qm_qm_spare_config_get(uint32_t *data);
bdmf_error_t ag_drv_qm_good_lvl1_pkts_cnt_get(uint32_t *good_lvl1_pkts);
bdmf_error_t ag_drv_qm_good_lvl1_bytes_cnt_get(uint32_t *good_lvl1_bytes);
bdmf_error_t ag_drv_qm_good_lvl2_pkts_cnt_get(uint32_t *good_lvl2_pkts);
bdmf_error_t ag_drv_qm_good_lvl2_bytes_cnt_get(uint32_t *good_lvl2_bytes);
bdmf_error_t ag_drv_qm_copied_pkts_cnt_get(uint32_t *copied_pkts);
bdmf_error_t ag_drv_qm_copied_bytes_cnt_get(uint32_t *copied_bytes);
bdmf_error_t ag_drv_qm_agg_pkts_cnt_get(uint32_t *agg_pkts);
bdmf_error_t ag_drv_qm_agg_bytes_cnt_get(uint32_t *agg_bytes);
bdmf_error_t ag_drv_qm_agg_1_pkts_cnt_get(uint32_t *agg1_pkts);
bdmf_error_t ag_drv_qm_agg_2_pkts_cnt_get(uint32_t *agg2_pkts);
bdmf_error_t ag_drv_qm_agg_3_pkts_cnt_get(uint32_t *agg3_pkts);
bdmf_error_t ag_drv_qm_agg_4_pkts_cnt_get(uint32_t *agg4_pkts);
bdmf_error_t ag_drv_qm_wred_drop_cnt_get(uint32_t *wred_drop);
bdmf_error_t ag_drv_qm_fpm_congestion_drop_cnt_get(uint32_t *fpm_cong);
bdmf_error_t ag_drv_qm_ddr_pd_congestion_drop_cnt_get(uint32_t *ddr_pd_cong_drop);
bdmf_error_t ag_drv_qm_ddr_byte_congestion_drop_cnt_get(uint32_t *ddr_cong_byte_drop);
bdmf_error_t ag_drv_qm_qm_pd_congestion_drop_cnt_get(uint32_t *counter);
bdmf_error_t ag_drv_qm_qm_abs_requeue_cnt_get(uint32_t *abs_requeue);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo0_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo1_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo2_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo3_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_normal_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_non_delayed_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_non_delayed_out_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_pre_cm_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_cm_rd_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_cm_wr_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_cm_common_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb0_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb1_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb1_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_egress_data_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_egress_rr_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb_route_ovr_set(uint8_t idx, bdmf_boolean ovr_en, uint8_t dest_id, uint16_t route_addr);
bdmf_error_t ag_drv_qm_bb_route_ovr_get(uint8_t idx, bdmf_boolean *ovr_en, uint8_t *dest_id, uint16_t *route_addr);
bdmf_error_t ag_drv_qm_ingress_stat_get(uint32_t *ingress_stat);
bdmf_error_t ag_drv_qm_egress_stat_get(uint32_t *egress_stat);
bdmf_error_t ag_drv_qm_cm_stat_get(uint32_t *cm_stat);
bdmf_error_t ag_drv_qm_fpm_prefetch_stat_get(uint32_t *fpm_prefetch_stat);
bdmf_error_t ag_drv_qm_qm_connect_ack_counter_get(uint8_t *connect_ack_counter);
bdmf_error_t ag_drv_qm_qm_ddr_wr_reply_counter_get(uint8_t *ddr_wr_reply_counter);
bdmf_error_t ag_drv_qm_qm_ddr_pipe_byte_counter_get(uint32_t *ddr_pipe);
bdmf_error_t ag_drv_qm_qm_abs_requeue_valid_counter_get(uint16_t *requeue_valid);
bdmf_error_t ag_drv_qm_qm_illegal_pd_capture_get(uint32_t idx, uint32_t *pd);
bdmf_error_t ag_drv_qm_qm_ingress_processed_pd_capture_get(uint32_t idx, uint32_t *pd);
bdmf_error_t ag_drv_qm_qm_cm_processed_pd_capture_get(uint32_t idx, uint32_t *pd);
bdmf_error_t ag_drv_qm_fpm_pool_drop_cnt_get(uint32_t idx, uint32_t *fpm_drop);
bdmf_error_t ag_drv_qm_fpm_grp_drop_cnt_get(uint32_t idx, uint32_t *fpm_grp_drop);
bdmf_error_t ag_drv_qm_fpm_buffer_res_drop_cnt_get(uint32_t *counter);
bdmf_error_t ag_drv_qm_psram_egress_cong_drp_cnt_get(uint32_t *counter);
bdmf_error_t ag_drv_qm_backpressure_get(uint32_t *counter);
bdmf_error_t ag_drv_qm_global_cfg2_bbhtx_fifo_addr_set(uint8_t addr, uint8_t bbhtx_req_otf);
bdmf_error_t ag_drv_qm_global_cfg2_bbhtx_fifo_addr_get(uint8_t *addr, uint8_t *bbhtx_req_otf);
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
    cli_qm_fpm_base_addr,
    cli_qm_global_cfg_fpm_coherent_base_addr,
    cli_qm_ddr_sop_offset,
    cli_qm_epon_overhead_ctrl,
    cli_qm_global_cfg_qm_aggregation_timer_ctrl,
    cli_qm_global_cfg_qm_fpm_ug_gbl_cnt,
    cli_qm_global_cfg_qm_egress_flush_queue,
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
    cli_qm_total_valid_cnt,
    cli_qm_dqm_valid_cnt,
    cli_qm_drop_counter,
    cli_qm_epon_q_byte_cnt,
    cli_qm_epon_q_status,
    cli_qm_rd_data_pool0,
    cli_qm_rd_data_pool1,
    cli_qm_rd_data_pool2,
    cli_qm_rd_data_pool3,
    cli_qm_pdfifo_ptr,
    cli_qm_update_fifo_ptr,
    cli_qm_fpm_buffer_reservation_data,
    cli_qm_flow_ctrl_ug_ctrl,
    cli_qm_flow_ctrl_status,
    cli_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg,
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
    cli_qm_fpm_pool_drop_cnt,
    cli_qm_fpm_grp_drop_cnt,
    cli_qm_fpm_buffer_res_drop_cnt,
    cli_qm_psram_egress_cong_drp_cnt,
    cli_qm_backpressure,
    cli_qm_global_cfg2_bbhtx_fifo_addr,
    cli_qm_cm_residue_data,
};

int bcm_qm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_qm_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

