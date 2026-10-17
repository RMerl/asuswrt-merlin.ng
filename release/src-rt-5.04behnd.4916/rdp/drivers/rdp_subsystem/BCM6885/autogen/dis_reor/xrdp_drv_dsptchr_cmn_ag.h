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


#ifndef _XRDP_DRV_DSPTCHR_CMN_AG_H_
#define _XRDP_DRV_DSPTCHR_CMN_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint8_t rnr_prof_0_1;
    uint8_t rnr_prof_0_2;
    uint8_t rnr_prof_1_1;
    uint8_t rnr_prof_1_2;
    uint8_t rnr_prof_2_1;
    uint8_t rnr_prof_2_2;
    uint8_t rnr_prof_3_1;
    uint8_t rnr_prof_3_2;
} dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof;

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
} dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel;

typedef struct
{
    uint8_t tsk;
    uint8_t rnr;
    bdmf_boolean en_dis;
    bdmf_boolean rnr_all;
    bdmf_boolean vld;
    bdmf_boolean done;
} dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis;

typedef struct
{
    uint8_t chn0_cfg;
    uint8_t chn1_cfg;
    uint8_t chn2_cfg;
    uint8_t chn3_cfg;
    uint8_t chn4_cfg;
    uint8_t chn5_cfg;
    uint8_t chn6_cfg;
    uint8_t chn7_cfg;
    uint8_t chn8_cfg;
    uint8_t chn9_cfg;
    bdmf_boolean chn_crdt_init;
    uint8_t chn_crdt_init_val;
} dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl;


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
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t base_add, uint16_t offset_add);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_get(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t *base_add, uint16_t *offset_add);

/**********************************************************************************************************************
 * rnr_prof_0_1: 
 *     Mapping profile #1 of RNR 4*i
 * rnr_prof_0_2: 
 *     Mapping profile #2 of RNR 4*i
 * rnr_prof_1_1: 
 *     Mapping profile #1 of RNR 4*i+1
 * rnr_prof_1_2: 
 *     Mapping profile #2 of RNR 4*i+1
 * rnr_prof_2_1: 
 *     Mapping profile #1 of RNR 4*i+2
 * rnr_prof_2_2: 
 *     Mapping profile #2 of RNR 4*i+2
 * rnr_prof_3_1: 
 *     Mapping profile #1 of RNR 4*i+3
 * rnr_prof_3_2: 
 *     Mapping profile #2 of RNR 4*i+3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(uint8_t dsptchr_cmn_id, uint32_t idx, const dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof *task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_get(uint8_t dsptchr_cmn_id, uint32_t idx, dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof *task_arb_reord_task_arb_queue_mapping_pd_rnr_prof);

/**********************************************************************************************************************
 * rnr_g_sel0: 
 *     0- Select profile 1
 *     1- Select profile 2
 * rnr_g_sel1: 
 *     0- Select profile 1
 *     1- Select profile 2
 * rnr_g_sel2: 
 *     0- Select profile 1
 *     1- Select profile 2
 * rnr_g_sel3: 
 *     0- Select profile 1
 *     1- Select profile 2
 * rnr_g_sel4: 
 *     0- Select profile 1
 *     1- Select profile 2
 * rnr_g_sel5: 
 *     0- Select profile 1
 *     1- Select profile 2
 * rnr_g_sel6: 
 *     0- Select profile 1
 *     1- Select profile 2
 * rnr_g_sel7: 
 *     0- Select profile 1
 *     1- Select profile 2
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel *task_arb_reord_task_arb_queue_mapping_rg_prof_sel);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel *task_arb_reord_task_arb_queue_mapping_rg_prof_sel);

/**********************************************************************************************************************
 * src_id: 
 *     Source ID - Dispatcher
 * dst_id_ovride: 
 *     Enable dispatcher reorder block
 * ovride_en: 
 *     Enable dispatcher reorder block
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en);

/**********************************************************************************************************************
 * route_override: 
 *     BB Route address override
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override);

/**********************************************************************************************************************
 * src_id: 
 *     Source ID - Dispatcher
 * dst_id_ovride: 
 *     Enable dispatcher reorder block
 * ovride_en: 
 *     Enable dispatcher reorder block
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en);

/**********************************************************************************************************************
 * route_override: 
 *     BB Route address override
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override);

/**********************************************************************************************************************
 * lb_mode: 
 *     RoundRobin = 0
 *     StrictPriority = 1
 *     
 * sp_thrshld: 
 *     Configures the threshold in which the LB mechanism opens activates a new RNR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_set(uint8_t dsptchr_cmn_id, bdmf_boolean lb_mode, uint8_t sp_thrshld);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_get(uint8_t dsptchr_cmn_id, bdmf_boolean *lb_mode, uint8_t *sp_thrshld);

/**********************************************************************************************************************
 * tsk_cnt_rg_0: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_1: 
 *     Counter the amount of available (free) tasks in a RNR Group
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1);

/**********************************************************************************************************************
 * tsk_cnt_rg_0: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_1: 
 *     Counter the amount of available (free) tasks in a RNR Group
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1);

/**********************************************************************************************************************
 * tsk_cnt_rg_0: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_1: 
 *     Counter the amount of available (free) tasks in a RNR Group
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1);

/**********************************************************************************************************************
 * tsk_cnt_rg_0: 
 *     Counter the amount of available (free) tasks in a RNR Group
 * tsk_cnt_rg_1: 
 *     Counter the amount of available (free) tasks in a RNR Group
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7_get(uint8_t dsptchr_cmn_id, uint16_t *tsk_cnt_rg_0, uint16_t *tsk_cnt_rg_1);

/**********************************************************************************************************************
 * rnr0: 
 *     Each bit indicats which task is Free for dispatch
 * rnr1: 
 *     Each bit indicats which task is Free for dispatch
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_set(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t rnr0, uint16_t rnr1);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(uint8_t dsptchr_cmn_id, uint32_t idx, uint16_t *rnr0, uint16_t *rnr1);

/**********************************************************************************************************************
 * tsk: 
 *     Task to enable/disable
 * rnr: 
 *     RNR to enable/disable task
 * en_dis: 
 *     0 - Disable Task
 *     1 - Enable Task
 * rnr_all: 
 *     0 - Enable/Disable specific task of RNR
 *     1 - Enable/Disable all tasks of RNR
 * vld: 
 *     Start Enable/Disable
 * done: 
 *     Enable/Disable complete
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis *task_arb_reord_task_arb_load_balancing_task_en_dis);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis *task_arb_reord_task_arb_load_balancing_task_en_dis);

/**********************************************************************************************************************
 * mask: 
 *     MASK
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(uint8_t dsptchr_cmn_id, uint32_t idx, uint32_t mask);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_get(uint8_t dsptchr_cmn_id, uint32_t idx, uint32_t *mask);

/**********************************************************************************************************************
 * chn0_cfg: 
 *     Channel 0 Config
 * chn1_cfg: 
 *     Channel 1 Config
 * chn2_cfg: 
 *     Channel 2 Config
 * chn3_cfg: 
 *     Channel 3 Config
 * chn4_cfg: 
 *     Channel 4 Config
 * chn5_cfg: 
 *     Channel 5 Config
 * chn6_cfg: 
 *     Channel 6 Config
 * chn7_cfg: 
 *     Channel 7 Config
 * chn8_cfg: 
 *     Channel 8 Config
 * chn9_cfg: 
 *     Channel 9 Config
 * chn_crdt_init: 
 *     Initialize credit towards QM for all 100G Dispatchers
 * chn_crdt_init_val: 
 *     Init credit value towards QM for all 100G Dispatchers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode *task_arb_reord_reord_chn_cfg_chn_mode);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode *task_arb_reord_reord_chn_cfg_chn_mode);

/**********************************************************************************************************************
 * dsp_half: 
 *     To which half the Common Reorder belongs to
 *     0 - Half 0
 *     1 - Half 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_set(uint8_t dsptchr_cmn_id, bdmf_boolean dsp_half);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_get(uint8_t dsptchr_cmn_id, bdmf_boolean *dsp_half);

/**********************************************************************************************************************
 * src_id: 
 *     Source ID - Dispatcher
 * dst_id_ovride: 
 *     Enable dispatcher reorder block
 * ovride_en: 
 *     Enable dispatcher reorder block
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en);

/**********************************************************************************************************************
 * route_override: 
 *     BB Route address override
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override);

/**********************************************************************************************************************
 * src_id: 
 *     Source ID - Dispatcher
 * dst_id_ovride: 
 *     Enable dispatcher reorder block
 * ovride_en: 
 *     Enable dispatcher reorder block
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_set(uint8_t dsptchr_cmn_id, uint16_t src_id, uint16_t dst_id_ovride, bdmf_boolean ovride_en);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_get(uint8_t dsptchr_cmn_id, uint16_t *src_id, uint16_t *dst_id_ovride, bdmf_boolean *ovride_en);

/**********************************************************************************************************************
 * route_override: 
 *     BB Route address override
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_set(uint8_t dsptchr_cmn_id, uint32_t route_override);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt_get(uint8_t dsptchr_cmn_id, uint32_t *route_override);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_byte_wght: 
 *     number of bytes for even channel weight
 *     
 * chn1_byte_wght: 
 *     number of bytes for odd channel weight
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_set(uint8_t dsptchr_cmn_id, uint16_t chn0_byte_wght, uint16_t chn1_byte_wght);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght_get(uint8_t dsptchr_cmn_id, uint16_t *chn0_byte_wght, uint16_t *chn1_byte_wght);

/**********************************************************************************************************************
 * chn0_inc_cntr: 
 *     Number of increments to update PD number
 * chn1_inc_cntr: 
 *     Number of increments to update PD number
 * chn2_inc_cntr: 
 *     Number of increments to update PD number
 * chn3_inc_cntr: 
 *     Number of increments to update PD number
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_set(uint8_t dsptchr_cmn_id, uint8_t chn0_inc_cntr, uint8_t chn1_inc_cntr, uint8_t chn2_inc_cntr, uint8_t chn3_inc_cntr);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr_get(uint8_t dsptchr_cmn_id, uint8_t *chn0_inc_cntr, uint8_t *chn1_inc_cntr, uint8_t *chn2_inc_cntr, uint8_t *chn3_inc_cntr);

/**********************************************************************************************************************
 * chn4_inc_cntr: 
 *     Number of increments to update PD number
 * chn5_inc_cntr: 
 *     Number of increments to update PD number
 * chn6_inc_cntr: 
 *     Number of increments to update PD number
 * chn7_inc_cntr: 
 *     Number of increments to update PD number
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_set(uint8_t dsptchr_cmn_id, uint8_t chn4_inc_cntr, uint8_t chn5_inc_cntr, uint8_t chn6_inc_cntr, uint8_t chn7_inc_cntr);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr_get(uint8_t dsptchr_cmn_id, uint8_t *chn4_inc_cntr, uint8_t *chn5_inc_cntr, uint8_t *chn6_inc_cntr, uint8_t *chn7_inc_cntr);

/**********************************************************************************************************************
 * chn8_inc_cntr: 
 *     Number of increments to update PD number
 * chn9_inc_cntr: 
 *     Number of increments to update PD number
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_set(uint8_t dsptchr_cmn_id, uint8_t chn8_inc_cntr, uint8_t chn9_inc_cntr);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr_get(uint8_t dsptchr_cmn_id, uint8_t *chn8_inc_cntr, uint8_t *chn9_inc_cntr);

/**********************************************************************************************************************
 * sn_cntr_01: 
 *     SN Counter of 01 Dispatchers
 * sn_cntr_23: 
 *     SN Counter of 23 Dispatchers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23);

/**********************************************************************************************************************
 * sn_cntr_01: 
 *     SN Counter of 01 Dispatchers
 * sn_cntr_23: 
 *     SN Counter of 23 Dispatchers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23);

/**********************************************************************************************************************
 * sn_cntr_01: 
 *     SN Counter of 01 Dispatchers
 * sn_cntr_23: 
 *     SN Counter of 23 Dispatchers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23);

/**********************************************************************************************************************
 * sn_cntr_01: 
 *     SN Counter of 01 Dispatchers
 * sn_cntr_23: 
 *     SN Counter of 23 Dispatchers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_set(uint8_t dsptchr_cmn_id, uint16_t sn_cntr_01, uint16_t sn_cntr_23);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts_get(uint8_t dsptchr_cmn_id, uint16_t *sn_cntr_01, uint16_t *sn_cntr_23);

/**********************************************************************************************************************
 * cntr_rst: 
 *     Reset AGG SN Counters.
 *     Write 1 to reset
 *     Bit 0 - Reset Disp 01 Chn 0
 *     Bit 1 - Reset Disp 01 Chn 1
 *     Bit 2 - Reset Disp 23 Chn 0
 *     Bit 3 - Reset Disp 23 Chn 1
 *     Bit 4 - Reset Disp 56 Chn 0
 *     Bit 5 - Reset Disp 56 Chn 1
 *     Bit 6 - Reset Disp 78 Chn 0
 *     Bit 7 - Reset Disp 78 Chn 1
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_set(uint8_t dsptchr_cmn_id, uint8_t cntr_rst);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst_get(uint8_t dsptchr_cmn_id, uint8_t *cntr_rst);

/**********************************************************************************************************************
 * qm_crdt_cntr_sts: 
 *     QM Credit Counter status
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts_get(uint8_t dsptchr_cmn_id, uint8_t *qm_crdt_cntr_sts);

/**********************************************************************************************************************
 * reord_full: 
 *     Full Status of all PD fifos
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts_get(uint8_t dsptchr_cmn_id, uint32_t *reord_full);

/**********************************************************************************************************************
 * reord_empty: 
 *     Empty Status of all PD fifos
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts_get(uint8_t dsptchr_cmn_id, uint32_t *reord_empty);

/**********************************************************************************************************************
 * disp0_chn0_uw: 
 *     Disp0 Chn0 PD Fifo Used Words
 * disp0_chn1_uw: 
 *     Disp0 Chn1 PD Fifo Used Words
 * disp1_chn0_uw: 
 *     Disp1 Chn0 PD Fifo Used Words
 * disp1_chn1_uw: 
 *     Disp1 Chn1 PD Fifo Used Words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01_get(uint8_t dsptchr_cmn_id, uint8_t *disp0_chn0_uw, uint8_t *disp0_chn1_uw, uint8_t *disp1_chn0_uw, uint8_t *disp1_chn1_uw);

/**********************************************************************************************************************
 * disp2_chn0_uw: 
 *     Disp2 Chn0 PD Fifo Used Words
 * disp2_chn1_uw: 
 *     Disp2 Chn1 PD Fifo Used Words
 * disp3_chn0_uw: 
 *     Disp3 Chn0 PD Fifo Used Words
 * disp3_chn1_uw: 
 *     Disp3 Chn1 PD Fifo Used Words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23_get(uint8_t dsptchr_cmn_id, uint8_t *disp2_chn0_uw, uint8_t *disp2_chn1_uw, uint8_t *disp3_chn0_uw, uint8_t *disp3_chn1_uw);

/**********************************************************************************************************************
 * disp5_chn0_uw: 
 *     Disp5 Chn0 PD Fifo Used Words
 * disp5_chn1_uw: 
 *     Disp5 Chn1 PD Fifo Used Words
 * disp6_chn0_uw: 
 *     Disp6 Chn0 PD Fifo Used Words
 * disp6_chn1_uw: 
 *     Disp6 Chn1 PD Fifo Used Words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56_get(uint8_t dsptchr_cmn_id, uint8_t *disp5_chn0_uw, uint8_t *disp5_chn1_uw, uint8_t *disp6_chn0_uw, uint8_t *disp6_chn1_uw);

/**********************************************************************************************************************
 * disp7_chn0_uw: 
 *     Disp7 Chn0 PD Fifo Used Words
 * disp7_chn1_uw: 
 *     Disp7 Chn1 PD Fifo Used Words
 * disp8_chn0_uw: 
 *     Disp8 Chn0 PD Fifo Used Words
 * disp8_chn1_uw: 
 *     Disp8 Chn1 PD Fifo Used Words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78_get(uint8_t dsptchr_cmn_id, uint8_t *disp7_chn0_uw, uint8_t *disp7_chn1_uw, uint8_t *disp8_chn0_uw, uint8_t *disp8_chn1_uw);

/**********************************************************************************************************************
 * disp4_chn0_uw: 
 *     Disp4 Chn0 PD Fifo Used Words
 * disp4_chn1_uw: 
 *     Disp4 Chn1 PD Fifo Used Words
 * disp9_chn0_uw: 
 *     Disp9 Chn0 PD Fifo Used Words
 * disp9_chn1_uw: 
 *     Disp9 Chn1 PD Fifo Used Words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49_get(uint8_t dsptchr_cmn_id, uint8_t *disp4_chn0_uw, uint8_t *disp4_chn1_uw, uint8_t *disp9_chn0_uw, uint8_t *disp9_chn1_uw);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 *     TRIGGERS_HW
 *     
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     Min value for Dispatcher is 0x14
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
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_set(uint8_t dsptchr_cmn_id, const dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl *task_arb_reord_clock_gate_control_clk_gate_cntrl);
bdmf_error_t ag_drv_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl_get(uint8_t dsptchr_cmn_id, dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl *task_arb_reord_clock_gate_control_clk_gate_cntrl);

#ifdef USE_BDMF_SHELL
enum
{
    cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add,
    cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof,
    cli_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel,
    cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg,
    cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_1_cfg_rt,
    cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg,
    cli_dsptchr_cmn_task_arb_reord_task_arb_bb_cfg_bb_2_cfg_rt,
    cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg,
    cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1,
    cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3,
    cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5,
    cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7,
    cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task,
    cli_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_task_en_dis,
    cli_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0,
    cli_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode,
    cli_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg,
    cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg,
    cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_1_cfg_rt,
    cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg,
    cli_dsptchr_cmn_task_arb_reord_reord_bb_cfg_bb_2_cfg_rt,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp0_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp1_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp2_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp3_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp4_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp5_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp6_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp7_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp8_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_dwrr_cfg_disp9_byte_wght,
    cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn03_inc_cntr,
    cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn47_inc_cntr,
    cli_dsptchr_cmn_task_arb_reord_reord_xon_inc_chn89_inc_cntr,
    cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_0_sts,
    cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_0_sts,
    cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_03_1_sts,
    cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_cntr_58_1_sts,
    cli_dsptchr_cmn_task_arb_reord_reord_status_agg_sn_rst,
    cli_dsptchr_cmn_task_arb_reord_reord_status_qm_crdt_cntr_sts,
    cli_dsptchr_cmn_task_arb_reord_reord_status_fifo_full_sts,
    cli_dsptchr_cmn_task_arb_reord_reord_status_fifo_empty_sts,
    cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_01,
    cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_23,
    cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_56,
    cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_78,
    cli_dsptchr_cmn_task_arb_reord_reord_status_pd_fifo_used_words_49,
    cli_dsptchr_cmn_task_arb_reord_clock_gate_control_clk_gate_cntrl,
};

int bcm_dsptchr_cmn_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dsptchr_cmn_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
