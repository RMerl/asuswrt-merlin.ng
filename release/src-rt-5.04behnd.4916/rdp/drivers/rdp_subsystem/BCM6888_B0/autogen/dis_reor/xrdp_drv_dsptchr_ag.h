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


#ifndef _XRDP_DRV_DSPTCHR_AG_H_
#define _XRDP_DRV_DSPTCHR_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint16_t frst_lvl;
    uint16_t scnd_lvl;
    uint8_t hyst_thrs;
} dsptchr_cngs_params;

typedef struct
{
    uint16_t frst_lvl;
    uint16_t scnd_lvl;
    uint8_t hyst_thrs;
} dsptchr_glbl_cngs_params;

typedef struct
{
    uint16_t cmn_pool_lmt;
    uint16_t grnted_pool_lmt;
    uint16_t mcast_pool_lmt;
    uint16_t rnr_pool_lmt;
    uint16_t cmn_pool_size;
    uint16_t grnted_pool_size;
    uint16_t mcast_pool_size;
    uint16_t rnr_pool_size;
    uint16_t processing_pool_size;
} dsptchr_pools_limits;

typedef struct
{
    uint32_t head;
    uint32_t tail;
    uint32_t minbuf;
    uint32_t bfin;
    uint32_t count;
} dsptchr_fll_entry;

typedef struct
{
    uint16_t base_add;
    uint16_t offset_add;
} dsptchr_rnr_dsptch_addr;

typedef struct
{
    bdmf_boolean disp_enable;
    bdmf_boolean auto_init_en;
    uint8_t auto_init_size;
    bdmf_boolean rdy;
    bdmf_boolean reordr_par_mod;
    bdmf_boolean per_q_egrs_congst_en;
    bdmf_boolean dsptch_sm_enh_mod;
    bdmf_boolean ingrs_pipe_dly_en;
    uint8_t ingrs_pipe_dly_cnt;
    bdmf_boolean egrs_drop_only;
    bdmf_boolean crdt_eff_rep;
    bdmf_boolean tsk_free_num_place;
} dsptchr_reorder_cfg_dsptchr_reordr_cfg;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} dsptchr_reorder_cfg_clk_gate_cntrl;

typedef struct
{
    uint8_t glbl_congstn;
    uint8_t glbl_egrs_congstn;
    uint8_t sbpm_congstn;
    uint8_t glbl_congstn_stcky;
    uint8_t glbl_egrs_congstn_stcky;
    uint8_t sbpm_congstn_stcky;
} dsptchr_congestion_congstn_status;

typedef struct
{
    bdmf_boolean rnr_g_sel0;
    bdmf_boolean rnr_g_sel1;
    bdmf_boolean rnr_g_sel2;
    bdmf_boolean rnr_g_sel3;
    bdmf_boolean rnr_g_sel4;
    bdmf_boolean rnr_g_sel5;
    bdmf_boolean rnr_g_sel6;
    bdmf_boolean rnr_g_sel7;
} dsptchr_queue_mapping_pd_dsptch_add_rnr_grp;

typedef struct
{
    uint32_t task_mask[8];
} dsptchr_mask_msk_tsk_255_0;

typedef struct
{
    bdmf_boolean q0;
    bdmf_boolean q1;
    bdmf_boolean q2;
    bdmf_boolean q3;
    bdmf_boolean q4;
    bdmf_boolean q5;
    bdmf_boolean q6;
    bdmf_boolean q7;
    bdmf_boolean q8;
    bdmf_boolean q9;
    bdmf_boolean q10;
    bdmf_boolean q11;
    bdmf_boolean q12;
    bdmf_boolean q13;
    bdmf_boolean q14;
    bdmf_boolean q15;
    bdmf_boolean q16;
    bdmf_boolean q17;
    bdmf_boolean q18;
    bdmf_boolean q19;
    bdmf_boolean q20;
    bdmf_boolean q21;
    bdmf_boolean q22;
    bdmf_boolean q23;
    bdmf_boolean q24;
    bdmf_boolean q25;
    bdmf_boolean q26;
    bdmf_boolean q27;
    bdmf_boolean q28;
    bdmf_boolean q29;
    bdmf_boolean q30;
    bdmf_boolean q31;
} dsptchr_wakeup_control_wkup_req;

typedef struct
{
    bdmf_boolean q0;
    bdmf_boolean q1;
    bdmf_boolean q2;
    bdmf_boolean q3;
    bdmf_boolean q4;
    bdmf_boolean q5;
    bdmf_boolean q6;
    bdmf_boolean q7;
    bdmf_boolean q8;
    bdmf_boolean q9;
    bdmf_boolean q10;
    bdmf_boolean q11;
    bdmf_boolean q12;
    bdmf_boolean q13;
    bdmf_boolean q14;
    bdmf_boolean q15;
    bdmf_boolean q16;
    bdmf_boolean q17;
    bdmf_boolean q18;
    bdmf_boolean q19;
    bdmf_boolean q20;
    bdmf_boolean q21;
    bdmf_boolean q22;
    bdmf_boolean q23;
    bdmf_boolean q24;
    bdmf_boolean q25;
    bdmf_boolean q26;
    bdmf_boolean q27;
    bdmf_boolean q28;
    bdmf_boolean q29;
    bdmf_boolean q30;
    bdmf_boolean q31;
} dsptchr_disptch_scheduling_vld_crdt;

typedef struct
{
    uint8_t tsk0;
    uint8_t tsk1;
    uint8_t tsk2;
    uint8_t tsk3;
    uint8_t tsk4;
    uint8_t tsk5;
    uint8_t tsk6;
    uint8_t tsk7;
} dsptchr_load_balancing_tsk_to_rg_mapping;

typedef struct
{
    bdmf_boolean fll_return_buf;
    bdmf_boolean fll_cnt_drp;
    bdmf_boolean unknwn_msg;
    bdmf_boolean fll_overflow;
    bdmf_boolean fll_neg;
} dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr;

typedef struct
{
    bdmf_boolean qdest0_int;
    bdmf_boolean qdest1_int;
    bdmf_boolean qdest2_int;
    bdmf_boolean qdest3_int;
    bdmf_boolean qdest4_int;
    bdmf_boolean qdest5_int;
    bdmf_boolean qdest6_int;
    bdmf_boolean qdest7_int;
    bdmf_boolean qdest8_int;
    bdmf_boolean qdest9_int;
    bdmf_boolean qdest10_int;
    bdmf_boolean qdest11_int;
    bdmf_boolean qdest12_int;
    bdmf_boolean qdest13_int;
    bdmf_boolean qdest14_int;
    bdmf_boolean qdest15_int;
    bdmf_boolean qdest16_int;
    bdmf_boolean qdest17_int;
    bdmf_boolean qdest18_int;
    bdmf_boolean qdest19_int;
    bdmf_boolean qdest20_int;
    bdmf_boolean qdest21_int;
    bdmf_boolean qdest22_int;
    bdmf_boolean qdest23_int;
    bdmf_boolean qdest24_int;
    bdmf_boolean qdest25_int;
    bdmf_boolean qdest26_int;
    bdmf_boolean qdest27_int;
    bdmf_boolean qdest28_int;
    bdmf_boolean qdest29_int;
    bdmf_boolean qdest30_int;
    bdmf_boolean qdest31_int;
} dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr;

typedef struct
{
    uint8_t tsk_cnt_rnr_0;
    uint8_t tsk_cnt_rnr_1;
    uint8_t tsk_cnt_rnr_2;
    uint8_t tsk_cnt_rnr_3;
    uint8_t tsk_cnt_rnr_4;
    uint8_t tsk_cnt_rnr_5;
    uint8_t tsk_cnt_rnr_6;
    uint8_t tsk_cnt_rnr_7;
} dsptchr_debug_glbl_tsk_cnt_0_7;

typedef struct
{
    uint8_t tsk_cnt_rnr_8;
    uint8_t tsk_cnt_rnr_9;
    uint8_t tsk_cnt_rnr_10;
    uint8_t tsk_cnt_rnr_11;
    uint8_t tsk_cnt_rnr_12;
    uint8_t tsk_cnt_rnr_13;
    uint8_t tsk_cnt_rnr_14;
    uint8_t tsk_cnt_rnr_15;
} dsptchr_debug_glbl_tsk_cnt_8_15;

typedef struct
{
    uint32_t qdes_bfout[32];
} dsptchr_qdes_bfout;

typedef struct
{
    uint32_t qdes_bfin[32];
} dsptchr_qdes_bufin;

typedef struct
{
    uint32_t qdes_fbdnull[32];
} dsptchr_qdes_fbdnull;

typedef struct
{
    uint32_t qdes_nullbd[32];
} dsptchr_qdes_nullbd;

typedef struct
{
    uint32_t qdes_bufavail[32];
} dsptchr_qdes_bufavail;

typedef struct
{
    uint32_t reorder_ram_data[2];
} dsptchr_pdram_data;


/**********************************************************************************************************************
 * frst_lvl: 
 *     First Level congestion threshold.
 * scnd_lvl: 
 *     Second Level congestion threshold.
 * hyst_thrs: 
 *     Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the
 *     (threshold_level - HYST_TRSH) will the congestion indication be removed
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cngs_params_set(uint8_t viq_idx, const dsptchr_cngs_params *cngs_params);
bdmf_error_t ag_drv_dsptchr_cngs_params_get(uint8_t viq_idx, dsptchr_cngs_params *cngs_params);

/**********************************************************************************************************************
 * frst_lvl: 
 *     First Level congestion threshold.
 * scnd_lvl: 
 *     Second Level congestion threshold.
 * hyst_thrs: 
 *     Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the
 *     (threshold_level - HYST_TRSH) will the congestion indication be removed
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_glbl_cngs_params_set(const dsptchr_glbl_cngs_params *glbl_cngs_params);
bdmf_error_t ag_drv_dsptchr_glbl_cngs_params_get(dsptchr_glbl_cngs_params *glbl_cngs_params);

/**********************************************************************************************************************
 * cmn_cnt: 
 *     Common number of buffers allocated to this Q.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_q_size_params_set(uint8_t q_idx, uint16_t cmn_cnt);
bdmf_error_t ag_drv_dsptchr_q_size_params_get(uint8_t q_idx, uint16_t *cmn_cnt);

/**********************************************************************************************************************
 * credit_cnt: 
 *     Holds the value of the the accumulated credits. this is sent to the BBH/RNR.
 *     BBH disregards the value. RNR uses it to to calculate the amount of available credits.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_credit_cnt_set(uint8_t q_idx, uint16_t credit_cnt);
bdmf_error_t ag_drv_dsptchr_credit_cnt_get(uint8_t q_idx, uint16_t *credit_cnt);

/**********************************************************************************************************************
 * cmn_max: 
 *     Maximum number of buffers allowed to be allocated to the specific VIQ from the common Pool
 * gurntd_max: 
 *     Maximum number of buffers allowed to be allocated to the specific VIQ from the guaranteed Pool
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_q_limits_params_set(uint8_t q_idx, uint16_t cmn_max, uint16_t gurntd_max);
bdmf_error_t ag_drv_dsptchr_q_limits_params_get(uint8_t q_idx, uint16_t *cmn_max, uint16_t *gurntd_max);

/**********************************************************************************************************************
 * chrncy_en: 
 *     Enable coherency counting. In case RNR is allocated to a specific VIQ it will not send coherency messages so
 *     there is no need to take them into consideration during PD dispatch
 * chrncy_cnt: 
 *     Coherency counter value. BBH sends a coherency message per PD. Coherency messages are counted and only if there
 *     is at least 1 coherency message can a PD be forwarded to the RNR for processing.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_ingress_coherency_params_set(uint8_t q_idx, bdmf_boolean chrncy_en, uint16_t chrncy_cnt);
bdmf_error_t ag_drv_dsptchr_ingress_coherency_params_get(uint8_t q_idx, bdmf_boolean *chrncy_en, uint16_t *chrncy_cnt);

/**********************************************************************************************************************
 * cmn_pool_lmt: 
 *     MAX number of buffers allowed in the pool
 * grnted_pool_lmt: 
 *     MAX number of buffers allowed in the pool
 * mcast_pool_lmt: 
 *     MAX number of buffers allowed in the pool
 * rnr_pool_lmt: 
 *     MAX number of buffers allowed in the pool
 * cmn_pool_size: 
 *     Number of buffers currently in the pool
 * grnted_pool_size: 
 *     Number of buffers currently in the pool
 * mcast_pool_size: 
 *     Number of buffers currently in the pool
 * rnr_pool_SIZE: 
 *     Number of buffers currently in the pool
 * processing_pool_size: 
 *     Number of buffers currently in the pool
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_pools_limits_set(const dsptchr_pools_limits *pools_limits);
bdmf_error_t ag_drv_dsptchr_pools_limits_get(dsptchr_pools_limits *pools_limits);

/**********************************************************************************************************************
 * head: 
 *     Pointer to the first BD in the link list of this queue.
 * tail: 
 *     Pointer to the last BD in the linked list of this queue.
 * minbuf: 
 *     Low threshold Interrupt. When number of bytes reach this level, then an interrupt is generated to the Host.
 * bfin: 
 *     32 bit wrap around counter. Counts number of entries that entered this queue since start of queue activity.
 * count: 
 *     32 bit wrap around counter. Counts number of entries that left this queue since start of queue activity.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_fll_entry_set(const dsptchr_fll_entry *fll_entry);
bdmf_error_t ag_drv_dsptchr_fll_entry_get(dsptchr_fll_entry *fll_entry);

/**********************************************************************************************************************
 * base_add: 
 *     Base address within each RNR
 * offset_add: 
 *     OFFSET address, in conjunction with base address for each task there will be a different address to where to
 *     send the PD
 *     
 *     ADD = BASE_ADD + (OFFSET_ADD x TASK)
 *     
 *     PD size is 128bits
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_rnr_dsptch_addr_set(uint8_t rnr_idx, const dsptchr_rnr_dsptch_addr *rnr_dsptch_addr);
bdmf_error_t ag_drv_dsptchr_rnr_dsptch_addr_get(uint8_t rnr_idx, dsptchr_rnr_dsptch_addr *rnr_dsptch_addr);

/**********************************************************************************************************************
 * disp_enable: 
 *     Enable dispatcher reorder block
 * auto_init_en: 
 *     Enable auto init of several block inside the Dispatcher.
 *     Currently includes Prev and Next BD rams
 *     
 *     Once set it will init the BD Ram memories. It will clear when finished
 * auto_init_size: 
 *     Limits configuration of Prev and Next BD rams according to their size
 * rdy: 
 *     Dispatcher reorder block is RDY
 * reordr_par_mod: 
 *     Enables parallel operation of Re-Order scheduler to Re-Order SM.
 *     
 *     Reduces Re-Order cycle from 16 clocks to 7.
 * per_q_egrs_congst_en: 
 *     Enable per Q Egress congestion monitoring
 * dsptch_sm_enh_mod: 
 *     Enables Enhanced performance mode of Dispatcher Load balancing and Dispatcher SM.
 *     
 *     This allows Disptach of PD to RNR instead of every 14 clocks, every 11 clocks.
 * ingrs_pipe_dly_en: 
 *     Enable delay added to the ingress pipe to
 * ingrs_pipe_dly_cnt: 
 *     Ingress delay count.
 *     Adds delay to INGRESS PIPE
 * egrs_drop_only: 
 *     Disables new Ingress drop mech and only allow drop from the re-order
 * crdt_eff_rep: 
 *     Will allow de-assert common_buf_empty when re-order is returning a buffer at the same time msgdec is requesting
 *     a common buffer.
 * tsk_free_num_place: 
 *     Task Free message from RNR need the TASK_NUM value returned. It used to be in target ADD [12:5]. FW required
 *     the option to return it in the DATA field [59:56].
 *     0 - Old mode using target ADD.
 *     1 - New mode bit [59:56] in DATA field
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(const dsptchr_reorder_cfg_dsptchr_reordr_cfg *reorder_cfg_dsptchr_reordr_cfg);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_get(dsptchr_reorder_cfg_dsptchr_reordr_cfg *reorder_cfg_dsptchr_reordr_cfg);

/**********************************************************************************************************************
 * en: 
 *     Enable Virtual Q control - 32 bit vector.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_reorder_cfg_vq_en_set(uint32_t en);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_vq_en_get(uint32_t *en);

/**********************************************************************************************************************
 * src_id: 
 *     Source ID - Dispatcher
 * dst_id_ovride: 
 *     Enable dispatcher reorder block
 * route_ovride: 
 *     Use this route address instead of pre-configured
 * ovride_en: 
 *     Enable dispatcher reorder block
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_reorder_cfg_bb_cfg_set(uint8_t src_id, uint8_t dst_id_ovride, uint16_t route_ovride, bdmf_boolean ovride_en);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_bb_cfg_get(uint8_t *src_id, uint8_t *dst_id_ovride, uint16_t *route_ovride, bdmf_boolean *ovride_en);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     Min value for Dispatcher is 0x14
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
bdmf_error_t ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set(const dsptchr_reorder_cfg_clk_gate_cntrl *reorder_cfg_clk_gate_cntrl);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get(dsptchr_reorder_cfg_clk_gate_cntrl *reorder_cfg_clk_gate_cntrl);

/**********************************************************************************************************************
 * frst_lvl: 
 *     First Level congestion threshold.
 * scnd_lvl: 
 *     Second Level congestion threshold.
 * hyst_thrs: 
 *     Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the
 *     (threshold_level - HYST_TRSH) will the congestion indication be removed
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_congestion_egrs_congstn_set(uint8_t viq_idx, const dsptchr_cngs_params *cngs_params);
bdmf_error_t ag_drv_dsptchr_congestion_egrs_congstn_get(uint8_t viq_idx, dsptchr_cngs_params *cngs_params);

/**********************************************************************************************************************
 * frst_lvl: 
 *     First Level congestion threshold.
 * scnd_lvl: 
 *     Second Level congestion threshold.
 * hyst_thrs: 
 *     Hystersis value in which to stop congestion indication. once reachin a congestion level only after crossing the
 *     (threshold_level - HYST_TRSH) will the congestion indication be removed
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_congestion_total_egrs_congstn_set(uint8_t viq_idx, const dsptchr_cngs_params *cngs_params);
bdmf_error_t ag_drv_dsptchr_congestion_total_egrs_congstn_get(uint8_t viq_idx, dsptchr_cngs_params *cngs_params);

/**********************************************************************************************************************
 * glbl_congstn: 
 *     Global congestion levels (according to FLL buffer availability)
 *     
 * glbl_egrs_congstn: 
 *     Global Egress congestion levels
 * sbpm_congstn: 
 *     SBPM congestion levels according to SPBM messages
 * glbl_congstn_stcky: 
 *     Global congestion levels (according to FLL buffer availability)
 *     Sticky Value
 *     
 * glbl_egrs_congstn_stcky: 
 *     Global Egress congestion levels
 *     Sticky value
 * sbpm_congstn_stcky: 
 *     SBPM congestion levels according to SPBM messages
 *     Sticky value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_congestion_congstn_status_get(dsptchr_congestion_congstn_status *congestion_congstn_status);

/**********************************************************************************************************************
 * congstn_state: 
 *     1 - Passed Threshold
 *     0 - Did not pass threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_congestion_per_q_ingrs_congstn_low_get(uint32_t *congstn_state);

/**********************************************************************************************************************
 * congstn_state: 
 *     1 - Passed Threshold
 *     0 - Did not pass threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_congestion_per_q_ingrs_congstn_high_get(uint32_t *congstn_state);

/**********************************************************************************************************************
 * congstn_state: 
 *     1 - Passed Threshold
 *     0 - Did not pass threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_congestion_per_q_egrs_congstn_low_get(uint32_t *congstn_state);

/**********************************************************************************************************************
 * congstn_state: 
 *     1 - Passed Threshold
 *     0 - Did not pass threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_congestion_per_q_egrs_congstn_high_get(uint32_t *congstn_state);

/**********************************************************************************************************************
 * bb_id: 
 *     BroadBud ID: To which BroadBud agent (RNR/BBH) is the current Q associated with
 * trgt_add: 
 *     Target address within the BB agent where the credit message should be written to.
 *     
 *     In case of RNR:
 *     27:16 - Ram address
 *     31:28 - Task number to wakeup
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_queue_mapping_crdt_cfg_set(uint8_t q_idx, uint8_t bb_id, uint16_t trgt_add);
bdmf_error_t ag_drv_dsptchr_queue_mapping_crdt_cfg_get(uint8_t q_idx, uint8_t *bb_id, uint16_t *trgt_add);

/**********************************************************************************************************************
 * q0: 
 *     0- Dispatcher
 *     1- Reorder
 * q1: 
 *     0- Dispatcher
 *     1- Reorder
 * q2: 
 *     0- Dispatcher
 *     1- Reorder
 * q3: 
 *     0- Dispatcher
 *     1- Reorder
 * q4: 
 *     0- Dispatcher
 *     1- Reorder
 * q5: 
 *     0- Dispatcher
 *     1- Reorder
 * q6: 
 *     0- Dispatcher
 *     1- Reorder
 * q7: 
 *     0- Dispatcher
 *     1- Reorder
 * q8: 
 *     0- Dispatcher
 *     1- Reorder
 * q9: 
 *     0- Dispatcher
 *     1- Reorder
 * q10: 
 *     0- Dispatcher
 *     1- Reorder
 * q11: 
 *     0- Dispatcher
 *     1- Reorder
 * q12: 
 *     0- Dispatcher
 *     1- Reorder
 * q13: 
 *     0- Dispatcher
 *     1- Reorder
 * q14: 
 *     0- Dispatcher
 *     1- Reorder
 * q15: 
 *     0- Dispatcher
 *     1- Reorder
 * q16: 
 *     0- Dispatcher
 *     1- Reorder
 * q17: 
 *     0- Dispatcher
 *     1- Reorder
 * q18: 
 *     0- Dispatcher
 *     1- Reorder
 * q19: 
 *     0- Dispatcher
 *     1- Reorder
 * q20: 
 *     0- Dispatcher
 *     1- Reorder
 *     
 * q21: 
 *     0- Dispatcher
 *     1- Reorder
 * q22: 
 *     0- Dispatcher
 *     1- Reorder
 * q23: 
 *     0- Dispatcher
 *     1- Reorder
 * q24: 
 *     0- Dispatcher
 *     1- Reorder
 * q25: 
 *     0- Dispatcher
 *     1- Reorder
 * q26: 
 *     0- Dispatcher
 *     1- Reorder
 * q27: 
 *     0- Dispatcher
 *     1- Reorder
 * q28: 
 *     0- Dispatcher
 *     1- Reorder
 * q29: 
 *     0- Dispatcher
 *     1- Reorder
 * q30: 
 *     0- Dispatcher
 *     1- Reorder
 * q31: 
 *     0- Dispatcher
 *     1- Reorder
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_queue_mapping_q_dest_set(uint8_t q_idx, bdmf_boolean is_dest_disp);
bdmf_error_t ag_drv_dsptchr_queue_mapping_q_dest_get(uint8_t q_idx, bdmf_boolean *is_dest_disp);

/**********************************************************************************************************************
 * rnr_g_sel0: 
 *     0- Select0
 *     1- Select8
 * rnr_g_sel1: 
 *     0- Select1
 *     1- Select9
 * rnr_g_sel2: 
 *     0- Select2
 *     1- Select10
 * rnr_g_sel3: 
 *     0- Select3
 *     1- Select11
 * rnr_g_sel4: 
 *     0- Select4
 *     1- Select12
 * rnr_g_sel5: 
 *     0- Select5
 *     1- Select13
 * rnr_g_sel6: 
 *     0- Select6
 *     1- Select14
 * rnr_g_sel7: 
 *     0- Select7
 *     1- Select15
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_queue_mapping_pd_dsptch_add_rnr_grp_set(const dsptchr_queue_mapping_pd_dsptch_add_rnr_grp *queue_mapping_pd_dsptch_add_rnr_grp);
bdmf_error_t ag_drv_dsptchr_queue_mapping_pd_dsptch_add_rnr_grp_get(dsptchr_queue_mapping_pd_dsptch_add_rnr_grp *queue_mapping_pd_dsptch_add_rnr_grp);

/**********************************************************************************************************************
 * mask: 
 *     MASK
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_mask_msk_tsk_255_0_set(uint8_t group_idx, const dsptchr_mask_msk_tsk_255_0 *mask_msk_tsk_255_0);
bdmf_error_t ag_drv_dsptchr_mask_msk_tsk_255_0_get(uint8_t group_idx, dsptchr_mask_msk_tsk_255_0 *mask_msk_tsk_255_0);

/**********************************************************************************************************************
 * mask: 
 *     MASK
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_mask_msk_q_set(uint8_t group_idx, uint32_t mask);
bdmf_error_t ag_drv_dsptchr_mask_msk_q_get(uint8_t group_idx, uint32_t *mask);

/**********************************************************************************************************************
 * mask: 
 *     MASK
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_mask_dly_q_set(uint8_t q_idx, bdmf_boolean set_delay);
bdmf_error_t ag_drv_dsptchr_mask_dly_q_get(uint8_t q_idx, bdmf_boolean *set_delay);

/**********************************************************************************************************************
 * mask: 
 *     MASK
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_mask_non_dly_q_set(uint8_t q_idx, bdmf_boolean set_non_delay);
bdmf_error_t ag_drv_dsptchr_mask_non_dly_q_get(uint8_t q_idx, bdmf_boolean *set_non_delay);

/**********************************************************************************************************************
 * dly_crdt: 
 *     The amount of free credits the re-order can utilize to send PDs to the QM
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_set(uint8_t dly_crdt);
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_get(uint8_t *dly_crdt);

/**********************************************************************************************************************
 * non_dly_crdt: 
 *     The amount of free credits the re-order can utilize to send PDs to the QM
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_set(uint8_t non_dly_crdt);
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_get(uint8_t *non_dly_crdt);

/**********************************************************************************************************************
 * total_egrs_size: 
 *     Accumulates all buffers that are marked as egress (after dispatch and before sending to QM)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_egrs_queues_total_q_egrs_size_set(uint16_t total_egrs_size);
bdmf_error_t ag_drv_dsptchr_egrs_queues_total_q_egrs_size_get(uint16_t *total_egrs_size);

/**********************************************************************************************************************
 * q_egrs_size: 
 *     Accumulates all buffers that are marked as egress (after dispatch and before sending to QM)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_egrs_queues_per_q_egrs_size_get(uint16_t q_idx, uint16_t *q_egrs_size);

/**********************************************************************************************************************
 * q0: 
 *     wakeup request pending
 * q1: 
 *     wakeup request pending
 * q2: 
 *     wakeup request pending
 * q3: 
 *     wakeup request pending
 * q4: 
 *     wakeup request pending
 * q5: 
 *     wakeup request pending
 * q6: 
 *     wakeup request pending
 * q7: 
 *     wakeup request pending
 * q8: 
 *     wakeup request pending
 * q9: 
 *     wakeup request pending
 * q10: 
 *     wakeup request pending
 * q11: 
 *     wakeup request pending
 * q12: 
 *     wakeup request pending
 * q13: 
 *     wakeup request pending
 * q14: 
 *     wakeup request pending
 * q15: 
 *     wakeup request pending
 * q16: 
 *     wakeup request pending
 * q17: 
 *     wakeup request pending
 * q18: 
 *     wakeup request pending
 * q19: 
 *     wakeup request pending
 * q20: 
 *     wakeup request pending
 * q21: 
 *     wakeup request pending
 * q22: 
 *     wakeup request pending
 * q23: 
 *     wakeup request pending
 * q24: 
 *     wakeup request pending
 * q25: 
 *     wakeup request pending
 * q26: 
 *     wakeup request pending
 * q27: 
 *     wakeup request pending
 * q28: 
 *     wakeup request pending
 * q29: 
 *     wakeup request pending
 * q30: 
 *     wakeup request pending
 * q31: 
 *     wakeup request pending
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_req_set(const dsptchr_wakeup_control_wkup_req *wakeup_control_wkup_req);
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_req_get(dsptchr_wakeup_control_wkup_req *wakeup_control_wkup_req);

/**********************************************************************************************************************
 * wkup_thrshld: 
 *     Wakeup threshold. Once number of Guaranteed buffer count crosses the threshold and there is a pending wakeup
 *     request, the dispatcher will issue a wakeup message to the appropriate runner according to a predefind address
 *     configuration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_thrshld_set(uint16_t wkup_thrshld);
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_thrshld_get(uint16_t *wkup_thrshld);

/**********************************************************************************************************************
 * q_crdt: 
 *     availabe credits in bytes. Q will not be permitted to dispatch PDs if credit levels are below zero
 * ngtv: 
 *     Bit will be enabled if credit levels are below zero. 2 compliment
 * quntum: 
 *     Quantum size. Should be configured according to Q rate. in Bytes
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_dwrr_info_set(uint8_t dwrr_q_idx, uint32_t q_crdt, bdmf_boolean ngtv, uint16_t quntum);
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_dwrr_info_get(uint8_t dwrr_q_idx, uint32_t *q_crdt, bdmf_boolean *ngtv, uint16_t *quntum);

/**********************************************************************************************************************
 * q0: 
 *     Valid Credits
 * q1: 
 *     Valid Credits.
 * q2: 
 *     Valid Credits
 * q3: 
 *     Valid Credits
 * q4: 
 *     Valid Credits
 * q5: 
 *     Valid Credits
 * q6: 
 *     Valid Credits
 * q7: 
 *     Valid Credits
 * q8: 
 *     Valid Credits
 * q9: 
 *     Valid Credits
 * q10: 
 *     Valid Credits
 * q11: 
 *     Valid Credits
 * q12: 
 *     Valid Credits
 * q13: 
 *     Valid Credits
 * q14: 
 *     Valid Credits
 * q15: 
 *     Valid Credits
 * q16: 
 *     Valid Credits
 * q17: 
 *     Valid Credits
 * q18: 
 *     Valid Credits
 * q19: 
 *     Valid Credits
 * q20: 
 *     Valid Credits
 * q21: 
 *     Valid Credits
 * q22: 
 *     Valid Credits
 * q23: 
 *     Valid Credits
 * q24: 
 *     Valid Credits
 * q25: 
 *     Valid Credits
 * q26: 
 *     Valid Credits
 * q27: 
 *     Valid Credits
 * q28: 
 *     Valid Credits
 * q29: 
 *     Valid Credits
 * q30: 
 *     Valid Credits
 * q31: 
 *     Valid Credits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_vld_crdt_set(const dsptchr_disptch_scheduling_vld_crdt *disptch_scheduling_vld_crdt);
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_vld_crdt_get(dsptchr_disptch_scheduling_vld_crdt *disptch_scheduling_vld_crdt);

/**********************************************************************************************************************
 * lb_mode: 
 *     RoundRobin = 0
 *     StrictPriority = 1
 *     
 * sp_thrshld: 
 *     Configures the threshold in which the LB mechanism opens activates a new RNR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_lb_cfg_set(bdmf_boolean lb_mode, uint8_t sp_thrshld);
bdmf_error_t ag_drv_dsptchr_load_balancing_lb_cfg_get(bdmf_boolean *lb_mode, uint8_t *sp_thrshld);

/**********************************************************************************************************************
 * rnr0: 
 *     Each bit indicats which task is Free for dispatch
 * rnr1: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_0_1_set(uint16_t rnr0, uint16_t rnr1);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_0_1_get(uint16_t *rnr0, uint16_t *rnr1);

/**********************************************************************************************************************
 * rnr2: 
 *     Each bit indicats which task is Free for dispatch
 * rnr3: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_2_3_set(uint16_t rnr2, uint16_t rnr3);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_2_3_get(uint16_t *rnr2, uint16_t *rnr3);

/**********************************************************************************************************************
 * rnr4: 
 *     Each bit indicats which task is Free for dispatch
 * rnr5: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_4_5_set(uint16_t rnr4, uint16_t rnr5);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_4_5_get(uint16_t *rnr4, uint16_t *rnr5);

/**********************************************************************************************************************
 * rnr6: 
 *     Each bit indicats which task is Free for dispatch
 * rnr7: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_6_7_set(uint16_t rnr6, uint16_t rnr7);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_6_7_get(uint16_t *rnr6, uint16_t *rnr7);

/**********************************************************************************************************************
 * rnr8: 
 *     Each bit indicats which task is Free for dispatch
 * rnr9: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_8_9_set(uint16_t rnr8, uint16_t rnr9);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_8_9_get(uint16_t *rnr8, uint16_t *rnr9);

/**********************************************************************************************************************
 * rnr10: 
 *     Each bit indicats which task is Free for dispatch
 * rnr11: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_10_11_set(uint16_t rnr10, uint16_t rnr11);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_10_11_get(uint16_t *rnr10, uint16_t *rnr11);

/**********************************************************************************************************************
 * rnr12: 
 *     Each bit indicats which task is Free for dispatch
 * rnr13: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_12_13_set(uint16_t rnr12, uint16_t rnr13);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_12_13_get(uint16_t *rnr12, uint16_t *rnr13);

/**********************************************************************************************************************
 * rnr14: 
 *     Each bit indicats which task is Free for dispatch
 * rnr15: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_14_15_set(uint16_t rnr14, uint16_t rnr15);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_14_15_get(uint16_t *rnr14, uint16_t *rnr15);

/**********************************************************************************************************************
 * tsk0: 
 *     Can be Task 0/8/16...
 * tsk1: 
 *     Can be Task 1/9/17...
 * tsk2: 
 *     Can be Task 2/10/18...
 * tsk3: 
 *     Can be Task 3/11/19...
 * tsk4: 
 *     Can be Task 4/12/20...
 * tsk5: 
 *     Can be Task 5/13/21...
 * tsk6: 
 *     Can be Task 6/14/22...
 * tsk7: 
 *     Can be Task 7/15/23...
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set(uint8_t task_to_rg_mapping, const dsptchr_load_balancing_tsk_to_rg_mapping *load_balancing_tsk_to_rg_mapping);
bdmf_error_t ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_get(uint8_t task_to_rg_mapping, dsptchr_load_balancing_tsk_to_rg_mapping *load_balancing_tsk_to_rg_mapping);

/**********************************************************************************************************************
 * tsk_cnt_rg_0: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_1: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_2: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_3: 
 *     Counter the amount of available (free) tasks in a RNR Group
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set(uint8_t tsk_cnt_rg_0, uint8_t tsk_cnt_rg_1, uint8_t tsk_cnt_rg_2, uint8_t tsk_cnt_rg_3);
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_get(uint8_t *tsk_cnt_rg_0, uint8_t *tsk_cnt_rg_1, uint8_t *tsk_cnt_rg_2, uint8_t *tsk_cnt_rg_3);

/**********************************************************************************************************************
 * tsk_cnt_rg_4: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_5: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_6: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_7: 
 *     Counter the amount of available (free) tasks in a RNR Group
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set(uint8_t tsk_cnt_rg_4, uint8_t tsk_cnt_rg_5, uint8_t tsk_cnt_rg_6, uint8_t tsk_cnt_rg_7);
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_get(uint8_t *tsk_cnt_rg_4, uint8_t *tsk_cnt_rg_5, uint8_t *tsk_cnt_rg_6, uint8_t *tsk_cnt_rg_7);

/**********************************************************************************************************************
 * fll_return_buf: 
 *     Buffer returned to Fll
 * fll_cnt_drp: 
 *     Drop PD counted
 * unknwn_msg: 
 *     Unknown message entered the dispatcher
 * fll_overflow: 
 *     Number of buffers returned to FLL exceeds the pre-defined allocated buffer amount (due to linked list bug)
 * fll_neg: 
 *     Number of buffers returned to FLL decreased under zero and reached a negative amount (due to linked list bug)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set(const dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr *dsptcher_reordr_top_intr_ctrl_0_isr);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_get(dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr *dsptcher_reordr_top_intr_ctrl_0_isr);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism_get(uint32_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_set(uint32_t iem);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_get(uint32_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_set(uint32_t ist);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_get(uint32_t *ist);

/**********************************************************************************************************************
 * qdest0_int: 
 *     New Entry added to Destination queue 0
 * qdest1_int: 
 *     New Entry added to Destination queue 1
 * qdest2_int: 
 *     New Entry added to Destination queue 2
 * qdest3_int: 
 *     New Entry added to Destination queue 3
 * qdest4_int: 
 *     New Entry added to Destination queue 4
 * qdest5_int: 
 *     New Entry added to Destination queue 5
 * qdest6_int: 
 *     New Entry added to Destination queue 6
 * qdest7_int: 
 *     New Entry added to Destination queue 7
 * qdest8_int: 
 *     New Entry added to Destination queue 8
 * qdest9_int: 
 *     New Entry added to Destination queue 9
 * qdest10_int: 
 *     New Entry added to Destination queue 10
 * qdest11_int: 
 *     New Entry added to Destination queue 11
 * qdest12_int: 
 *     New Entry added to Destination queue 12
 * qdest13_int: 
 *     New Entry added to Destination queue 13
 * qdest14_int: 
 *     New Entry added to Destination queue 14
 * qdest15_int: 
 *     New Entry added to Destination queue 15
 * qdest16_int: 
 *     New Entry added to Destination queue 16
 * qdest17_int: 
 *     New Entry added to Destination queue 17
 * qdest18_int: 
 *     New Entry added to Destination queue 18
 * qdest19_int: 
 *     New Entry added to Destination queue 19
 * qdest20_int: 
 *     New Entry added to Destination queue 20
 * qdest21_int: 
 *     New Entry added to Destination queue 21
 * qdest22_int: 
 *     New Entry added to Destination queue 22
 * qdest23_int: 
 *     New Entry added to Destination queue 23
 * qdest24_int: 
 *     New Entry added to Destination queue 24
 * qdest25_int: 
 *     New Entry added to Destination queue 25
 * qdest26_int: 
 *     New Entry added to Destination queue 26
 * qdest27_int: 
 *     New Entry added to Destination queue 27
 * qdest28_int: 
 *     New Entry added to Destination queue 28
 * qdest29_int: 
 *     New Entry added to Destination queue 29
 * qdest30_int: 
 *     New Entry added to Destination queue 30
 * qdest31_int: 
 *     New Entry added to Destination queue 31
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_set(const dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr *dsptcher_reordr_top_intr_ctrl_1_isr);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_get(dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr *dsptcher_reordr_top_intr_ctrl_1_isr);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism_get(uint32_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_set(uint32_t iem);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_get(uint32_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_set(uint32_t ist);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_get(uint32_t *ist);

/**********************************************************************************************************************
 * en_byp: 
 *     Enable bypass mode
 * bbid_non_dly: 
 *     What BBID to use for NON_DELAY Q when in Bypass mode
 * bbid_dly: 
 *     What BBID to use for DELAY Q when in Bypass mode
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_bypss_cntrl_set(bdmf_boolean en_byp, uint8_t bbid_non_dly, uint8_t bbid_dly);
bdmf_error_t ag_drv_dsptchr_debug_dbg_bypss_cntrl_get(bdmf_boolean *en_byp, uint8_t *bbid_non_dly, uint8_t *bbid_dly);

/**********************************************************************************************************************
 * tsk_cnt_rnr_0: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_1: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_2: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_3: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_4: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_5: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_6: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_7: 
 *     Counter the amount of active tasks
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_set(const dsptchr_debug_glbl_tsk_cnt_0_7 *debug_glbl_tsk_cnt_0_7);
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_get(dsptchr_debug_glbl_tsk_cnt_0_7 *debug_glbl_tsk_cnt_0_7);

/**********************************************************************************************************************
 * tsk_cnt_rnr_8: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_9: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_10: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_11: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_12: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_13: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_14: 
 *     Counter the amount of active tasks
 * tsk_cnt_rnr_15: 
 *     Counter the amount of active tasks
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_set(const dsptchr_debug_glbl_tsk_cnt_8_15 *debug_glbl_tsk_cnt_8_15);
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_get(dsptchr_debug_glbl_tsk_cnt_8_15 *debug_glbl_tsk_cnt_8_15);

/**********************************************************************************************************************
 * dbg_sel: 
 *     Selects with vector to output
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_bus_cntrl_set(uint8_t dbg_sel);
bdmf_error_t ag_drv_dsptchr_debug_dbg_bus_cntrl_get(uint8_t *dbg_sel);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_0_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_1_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_2_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_3_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_4_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_5_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_6_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_7_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_8_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_9_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_10_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_11_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_12_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_13_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_14_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_15_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_16_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_17_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_18_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_19_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_20_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_21_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_22_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_23_get(uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * dbg_mode: 
 *     Selects mode to log
 * en_cntrs: 
 *     Enable statistics
 * clr_cntrs: 
 *     Clears all counters
 * dbg_rnr_sel: 
 *     Selects RNR to log
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_set(uint8_t dbg_mode, bdmf_boolean en_cntrs, bdmf_boolean clr_cntrs, uint8_t dbg_rnr_sel);
bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_get(uint8_t *dbg_mode, bdmf_boolean *en_cntrs, bdmf_boolean *clr_cntrs, uint8_t *dbg_rnr_sel);

/**********************************************************************************************************************
 * dbg_vec_val: 
 *     Debug bus vector value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_cnt_get(uint8_t index, uint32_t *dbg_vec_val);

/**********************************************************************************************************************
 * head: 
 *     Pointer to the first BD in the link list of this queue.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_head_set(uint8_t q_idx, uint32_t head);
bdmf_error_t ag_drv_dsptchr_qdes_head_get(uint8_t q_idx, uint32_t *head);

/**********************************************************************************************************************
 * bfout: 
 *     32 bit wrap around counter. Counts number of packets that left this queue since start of queue activity.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_bfout_set(uint8_t zero, const dsptchr_qdes_bfout *qdes_bfout);
bdmf_error_t ag_drv_dsptchr_qdes_bfout_get(uint8_t zero, dsptchr_qdes_bfout *qdes_bfout);

/**********************************************************************************************************************
 * bufin: 
 *     32 bit wrap around counter. Counts number of packets that entered this queue since start of queue activity.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_bufin_set(uint8_t zero, const dsptchr_qdes_bufin *qdes_bufin);
bdmf_error_t ag_drv_dsptchr_qdes_bufin_get(uint8_t zero, dsptchr_qdes_bufin *qdes_bufin);

/**********************************************************************************************************************
 * tail: 
 *     Pointer to the last BD in the linked list of this queue.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_tail_set(uint8_t q_idx, uint32_t tail);
bdmf_error_t ag_drv_dsptchr_qdes_tail_get(uint8_t q_idx, uint32_t *tail);

/**********************************************************************************************************************
 * fbdnull: 
 *     If this bit is set then the first BD attached to this Q is a null BD. In this case, its Data Pointer field is
 *     not valid, but its Next BD pointer field is valid. When it is set, the NullBD field for this queue is not valid.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_fbdnull_set(uint8_t zero, const dsptchr_qdes_fbdnull *qdes_fbdnull);
bdmf_error_t ag_drv_dsptchr_qdes_fbdnull_get(uint8_t zero, dsptchr_qdes_fbdnull *qdes_fbdnull);

/**********************************************************************************************************************
 * nullbd: 
 *     32 bits index of a Null BD that belongs to this queue. Both the data buffer pointer and the next BD field are
 *     non valid. The pointer defines a memory allocation for a BD that might be used or not.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_nullbd_set(uint8_t zero, const dsptchr_qdes_nullbd *qdes_nullbd);
bdmf_error_t ag_drv_dsptchr_qdes_nullbd_get(uint8_t zero, dsptchr_qdes_nullbd *qdes_nullbd);

/**********************************************************************************************************************
 * bufavail: 
 *     number of entries available in queue.
 *     bufin - bfout
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_bufavail_get(uint8_t zero, dsptchr_qdes_bufavail *qdes_bufavail);

/**********************************************************************************************************************
 * head: 
 *     Q HEAD
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_reg_q_head_set(uint8_t q_head_idx, uint16_t head);
bdmf_error_t ag_drv_dsptchr_qdes_reg_q_head_get(uint8_t q_head_idx, uint16_t *head);

/**********************************************************************************************************************
 * viq_head_vld: 
 *     Q head valid. Each bit indicates for a specific VIQ if the head is valid or not
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_head_vld_set(uint32_t viq_head_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_head_vld_get(uint32_t *viq_head_vld);

/**********************************************************************************************************************
 * chrncy_vld: 
 *     Q Coherency counter is valid. Each bit indicates for a specific VIQ if the there is more than one coherency
 *     message for that Q. meaning the head of the VIQ can be dispatched
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_set(uint32_t chrncy_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_get(uint32_t *chrncy_vld);

/**********************************************************************************************************************
 * viq_head_vld: 
 *     Q head valid. Each bit indicates for a specific VIQ if the head is valid or not
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_reg_veq_head_vld_set(uint32_t viq_head_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_veq_head_vld_get(uint32_t *viq_head_vld);

/**********************************************************************************************************************
 * use_buf_avl: 
 *     Should buf_avail in the QDES affect poping from head of linked list
 * dec_bufout_when_mltcst: 
 *     Should buf_avail in the QDES affect poping from head of linked list
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_set(bdmf_boolean use_buf_avl, bdmf_boolean dec_bufout_when_mltcst);
bdmf_error_t ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_get(bdmf_boolean *use_buf_avl, bdmf_boolean *dec_bufout_when_mltcst);

/**********************************************************************************************************************
 * drpcnt: 
 *     32 bit counter that counts the number of packets arrived when there is no free BD in the FLL.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_flldes_flldrop_set(uint32_t drpcnt);
bdmf_error_t ag_drv_dsptchr_flldes_flldrop_get(uint32_t *drpcnt);

/**********************************************************************************************************************
 * bufavail: 
 *     number of entries available in queue.
 *     bufin - bfout
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_flldes_bufavail_get(uint32_t *bufavail);

/**********************************************************************************************************************
 * freemin: 
 *     minum value of free BD recorded
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_flldes_freemin_get(uint32_t *freemin);

/**********************************************************************************************************************
 * data: 
 *     Data Buffer entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_bdram_next_data_set(uint16_t temp_index, uint16_t data);
bdmf_error_t ag_drv_dsptchr_bdram_next_data_get(uint16_t temp_index, uint16_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data Buffer entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_bdram_prev_data_set(uint16_t temp_index, uint16_t data);
bdmf_error_t ag_drv_dsptchr_bdram_prev_data_get(uint16_t temp_index, uint16_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data Buffer entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_pdram_data_set(uint16_t temp_index, const dsptchr_pdram_data *pdram_data);
bdmf_error_t ag_drv_dsptchr_pdram_data_get(uint16_t temp_index, dsptchr_pdram_data *pdram_data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_dsptchr_cngs_params,
    cli_dsptchr_glbl_cngs_params,
    cli_dsptchr_q_size_params,
    cli_dsptchr_credit_cnt,
    cli_dsptchr_q_limits_params,
    cli_dsptchr_ingress_coherency_params,
    cli_dsptchr_pools_limits,
    cli_dsptchr_fll_entry,
    cli_dsptchr_rnr_dsptch_addr,
    cli_dsptchr_reorder_cfg_dsptchr_reordr_cfg,
    cli_dsptchr_reorder_cfg_vq_en,
    cli_dsptchr_reorder_cfg_bb_cfg,
    cli_dsptchr_reorder_cfg_clk_gate_cntrl,
    cli_dsptchr_congestion_egrs_congstn,
    cli_dsptchr_congestion_total_egrs_congstn,
    cli_dsptchr_congestion_congstn_status,
    cli_dsptchr_congestion_per_q_ingrs_congstn_low,
    cli_dsptchr_congestion_per_q_ingrs_congstn_high,
    cli_dsptchr_congestion_per_q_egrs_congstn_low,
    cli_dsptchr_congestion_per_q_egrs_congstn_high,
    cli_dsptchr_queue_mapping_crdt_cfg,
    cli_dsptchr_queue_mapping_q_dest,
    cli_dsptchr_queue_mapping_pd_dsptch_add_rnr_grp,
    cli_dsptchr_mask_msk_tsk_255_0,
    cli_dsptchr_mask_msk_q,
    cli_dsptchr_mask_dly_q,
    cli_dsptchr_mask_non_dly_q,
    cli_dsptchr_egrs_queues_egrs_dly_qm_crdt,
    cli_dsptchr_egrs_queues_egrs_non_dly_qm_crdt,
    cli_dsptchr_egrs_queues_total_q_egrs_size,
    cli_dsptchr_egrs_queues_per_q_egrs_size,
    cli_dsptchr_wakeup_control_wkup_req,
    cli_dsptchr_wakeup_control_wkup_thrshld,
    cli_dsptchr_disptch_scheduling_dwrr_info,
    cli_dsptchr_disptch_scheduling_vld_crdt,
    cli_dsptchr_load_balancing_lb_cfg,
    cli_dsptchr_load_balancing_free_task_0_1,
    cli_dsptchr_load_balancing_free_task_2_3,
    cli_dsptchr_load_balancing_free_task_4_5,
    cli_dsptchr_load_balancing_free_task_6_7,
    cli_dsptchr_load_balancing_free_task_8_9,
    cli_dsptchr_load_balancing_free_task_10_11,
    cli_dsptchr_load_balancing_free_task_12_13,
    cli_dsptchr_load_balancing_free_task_14_15,
    cli_dsptchr_load_balancing_tsk_to_rg_mapping,
    cli_dsptchr_load_balancing_rg_avlabl_tsk_0_3,
    cli_dsptchr_load_balancing_rg_avlabl_tsk_4_7,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr,
    cli_dsptchr_debug_dbg_bypss_cntrl,
    cli_dsptchr_debug_glbl_tsk_cnt_0_7,
    cli_dsptchr_debug_glbl_tsk_cnt_8_15,
    cli_dsptchr_debug_dbg_bus_cntrl,
    cli_dsptchr_debug_dbg_vec_0,
    cli_dsptchr_debug_dbg_vec_1,
    cli_dsptchr_debug_dbg_vec_2,
    cli_dsptchr_debug_dbg_vec_3,
    cli_dsptchr_debug_dbg_vec_4,
    cli_dsptchr_debug_dbg_vec_5,
    cli_dsptchr_debug_dbg_vec_6,
    cli_dsptchr_debug_dbg_vec_7,
    cli_dsptchr_debug_dbg_vec_8,
    cli_dsptchr_debug_dbg_vec_9,
    cli_dsptchr_debug_dbg_vec_10,
    cli_dsptchr_debug_dbg_vec_11,
    cli_dsptchr_debug_dbg_vec_12,
    cli_dsptchr_debug_dbg_vec_13,
    cli_dsptchr_debug_dbg_vec_14,
    cli_dsptchr_debug_dbg_vec_15,
    cli_dsptchr_debug_dbg_vec_16,
    cli_dsptchr_debug_dbg_vec_17,
    cli_dsptchr_debug_dbg_vec_18,
    cli_dsptchr_debug_dbg_vec_19,
    cli_dsptchr_debug_dbg_vec_20,
    cli_dsptchr_debug_dbg_vec_21,
    cli_dsptchr_debug_dbg_vec_22,
    cli_dsptchr_debug_dbg_vec_23,
    cli_dsptchr_debug_statistics_dbg_sttstcs_ctrl,
    cli_dsptchr_debug_statistics_dbg_cnt,
    cli_dsptchr_qdes_head,
    cli_dsptchr_qdes_bfout,
    cli_dsptchr_qdes_bufin,
    cli_dsptchr_qdes_tail,
    cli_dsptchr_qdes_fbdnull,
    cli_dsptchr_qdes_nullbd,
    cli_dsptchr_qdes_bufavail,
    cli_dsptchr_qdes_reg_q_head,
    cli_dsptchr_qdes_reg_viq_head_vld,
    cli_dsptchr_qdes_reg_viq_chrncy_vld,
    cli_dsptchr_qdes_reg_veq_head_vld,
    cli_dsptchr_qdes_reg_qdes_buf_avl_cntrl,
    cli_dsptchr_flldes_flldrop,
    cli_dsptchr_flldes_bufavail,
    cli_dsptchr_flldes_freemin,
    cli_dsptchr_bdram_next_data,
    cli_dsptchr_bdram_prev_data,
    cli_dsptchr_pdram_data,
};

int bcm_dsptchr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dsptchr_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
