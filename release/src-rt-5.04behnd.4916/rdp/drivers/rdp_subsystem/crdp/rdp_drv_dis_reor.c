/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
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

#include "rdp_drv_dis_reor.h"
extern dsptchr_config dsptchr_cfg;


extern uint8_t valid_rnr_core[NUM_OF_PROCESSING_RUNNERS + NUM_OF_TM_RUNNERS];

void drv_dispatcher_reorder_viq_init(uint8_t dsptchr_index, dsptchr_config *cfg, dsptchr_cngs_params *ingress_congs_init,
    dsptchr_cngs_params *egress_congs_init, uint8_t bb_id, uint32_t target_address,
    bdmf_boolean dest, uint8_t viq_num, uint8_t guaranteed_limit,
    uint16_t common_max_limit, bdmf_boolean is_bbh_queue, runner_task_group_idx_e rnr_group_idx)
{
    cfg->dsptchr_viq_list[dsptchr_index][viq_num].bb_id = bb_id;
    cfg->dsptchr_viq_list[dsptchr_index][viq_num].bbh_target_address = ((1 << viq_num) << 7) | target_address;
    cfg->dsptchr_viq_list[dsptchr_index][viq_num].queue_dest = dest; /* 0-disp, 1-reor */
    cfg->dsptchr_viq_list[dsptchr_index][viq_num].common_max_limit = common_max_limit;
    cfg->dsptchr_viq_list[dsptchr_index][viq_num].guaranteed_max_limit = guaranteed_limit;
    cfg->rnr_group_idx[dsptchr_index][viq_num] = rnr_group_idx;

    memcpy(&(cfg->dsptchr_viq_list[dsptchr_index][viq_num].ingress_cngs), ingress_congs_init, sizeof(dsptchr_cngs_params));
    memcpy(&(cfg->dsptchr_viq_list[dsptchr_index][viq_num].egress_cngs), egress_congs_init, sizeof(dsptchr_cngs_params));
    cfg->total_viq_guaranteed_buf[dsptchr_index] += guaranteed_limit;
}

/* 
    main init, get veiq list, and configure all viq's and registers
*/
int dispatcher_reorder_init(int dsptchr_index, int num_viqs)
{ 
    dsptchr_reorder_cfg_dsptchr_reordr_cfg dsptchr_reordr_cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    int i;

    dsptchr_cfg.fll_entry[dsptchr_index].count = num_viqs;
    dsptchr_cfg.fll_entry[dsptchr_index].head = num_viqs;
    dsptchr_cfg.fll_entry[dsptchr_index].tail = (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1);
    dsptchr_cfg.fll_entry[dsptchr_index].bfin = DIS_REOR_LINKED_LIST_BUFFER_NUM - num_viqs;
    dsptchr_cfg.fll_entry[dsptchr_index].minbuf = DIS_REOR_LINKED_LIST_BUFFER_NUM;
    dsptchr_cfg.total_viq_guaranteed_buf[dsptchr_index] = 0;

    dsptchr_cfg.cngs_params.frst_lvl = DIS_REOR_LINKED_LIST_BUFFER_NUM - 1;
    dsptchr_cfg.cngs_params.scnd_lvl = DIS_REOR_LINKED_LIST_BUFFER_NUM - 1;
    dsptchr_cfg.cngs_params.hyst_thrs = 16;

    /* configures the threshold to be higher than the number of buffers that are common and therefore guaranteed.*/
    /* but low numbers, becuase we dont expect that too many packets will be accumulated inside the disp */
    dsptchr_cfg.glbl_congs_init.frst_lvl = DSPTCHR_FLL_1ST_LVL;
    dsptchr_cfg.glbl_congs_init.scnd_lvl = DSPTCHR_FLL_2ND_LVL;
    dsptchr_cfg.glbl_congs_init.hyst_thrs = DSPTCHR_FLL_HYST;

    dsptchr_cfg.disp_num_of_credits = 8;
    dsptchr_cfg.dsptcher_reordr_top_intr_ctrl_0_isr.fll_return_buf = 1;
    dsptchr_cfg.dsptcher_reordr_top_intr_ctrl_0_isr.fll_cnt_drp = 1;
    dsptchr_cfg.dsptcher_reordr_top_intr_ctrl_0_isr.unknwn_msg = 1;
    dsptchr_cfg.dsptcher_reordr_top_intr_ctrl_0_isr.fll_overflow = 1;
    dsptchr_cfg.dsptcher_reordr_top_intr_ctrl_0_isr.fll_neg = 1;
    dsptchr_cfg.dsptcher_reordr_top_intr_ctrl_0_isr.reordr_empty_viq = 1;

    drv_dispatcher_cmn_init(&dsptchr_cfg, (offsetof(PACKET_BUFFER_STRUCT, processing_rx_descriptor)), sizeof(PACKET_BUFFER_STRUCT));

    for (i = 0; i < num_viqs; i++)
    {
        drv_dispatcher_chan_viq_cfg_set(dsptchr_index, i, &dsptchr_cfg, 0);
    }

    dsptchr_cfg.pools_limits[dsptchr_index].grnted_pool_lmt = dsptchr_cfg.total_viq_guaranteed_buf[dsptchr_index];
    dsptchr_cfg.pools_limits[dsptchr_index].grnted_pool_size = dsptchr_cfg.total_viq_guaranteed_buf[dsptchr_index];
    dsptchr_cfg.pools_limits[dsptchr_index].cmn_pool_lmt = DSPTCHR_CONG_PARAM_GLOBAL - dsptchr_cfg.total_viq_guaranteed_buf[dsptchr_index] - num_viqs;
    dsptchr_cfg.pools_limits[dsptchr_index].cmn_pool_size = DSPTCHR_CONG_PARAM_GLOBAL - dsptchr_cfg.total_viq_guaranteed_buf[dsptchr_index] - num_viqs;

    rc = ag_drv_dsptchr_congestion_congestn_thrshld_act_set(dsptchr_index, 0, 511, 0/* TODO check value?*/);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. dispatcher index: %d\n", dsptchr_index);
    
    rc = ag_drv_dsptchr_congestion_total_egrs_congstn_set(0, dsptchr_index, &dsptchr_cfg.cngs_params);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. dispatcher index: %d\n", dsptchr_index);

    rc = ag_drv_dsptchr_glbl_cngs_params_set(dsptchr_index, &dsptchr_cfg.glbl_congs_init);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. dispatcher index: %d\n", dsptchr_index);

    rc = ag_drv_dsptchr_fll_entry_set(dsptchr_index, &dsptchr_cfg.fll_entry[dsptchr_index]);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. dispatcher index: %d\n", dsptchr_index);
    
    rc = ag_drv_dsptchr_wakeup_control_wkup_thrshld_set(dsptchr_index, dsptchr_cfg.disp_num_of_credits);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. dispatcher index: %d\n", dsptchr_index);

    rc = ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set(dsptchr_index, &dsptchr_cfg.dsptcher_reordr_top_intr_ctrl_0_isr);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. dispatcher index: %d\n", dsptchr_index);

    rc = ag_drv_dsptchr_pools_limits_set(dsptchr_index, &dsptchr_cfg.pools_limits[dsptchr_index]);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. dispatcher index: %d\n", dsptchr_index);

    rc = ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_get(dsptchr_index, &dsptchr_reordr_cfg);
    dsptchr_reordr_cfg.chn_mode = 0;
    dsptchr_reordr_cfg.disp_enable = 1;
    dsptchr_reordr_cfg.per_q_egrs_congst_en = 0;
    dsptchr_reordr_cfg.auto_init_en = 1;
    dsptchr_reordr_cfg.dsp_id = 0;
    dsptchr_reordr_cfg.dsp_half = dsptchr_index;
    dsptchr_reordr_cfg.dsp_num = dsptchr_index;

    /* auto_init_size:
        FLL_LEN==2048 -> 0 
        FLL_LEN==256  -> 1 
        FLL_LEN==512  -> 2
        FLL_LEN==1024 -> 3
    */
    dsptchr_reordr_cfg.auto_init_size = 0;

    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(dsptchr_index, &dsptchr_reordr_cfg);
    return rc;
}

bdmf_error_t drv_dispatcher_cmn_init(dsptchr_config* cfg, uint16_t pd_base_addr, uint16_t pd_offset_addr)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t index;
    dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode task_arb_reord_reord_chn_cfg_chn_mode = {};
    dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel task_arb_reord_task_arb_queue_mapping_rg_prof_sel;

    rc = ag_drv_dsptchr_cmn_task_arb_reord_reord_cfg_reord_cfg_set(0, 0);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);

    /* use profile 0 for processing */
    rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(0, DISPATCHER_GROUP_PROCESSING, pd_base_addr >> 3, pd_offset_addr >> 3);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);

     /* use profile 7 for replication */
    rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(0, DISPATCHER_GROUP_REPLICATION, pd_base_addr >> 3, pd_offset_addr >> 3);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);
    
    /* use profile 5 for direct flows */
    rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_add_set(0, DISPATCHER_GROUP_DIRECT_FLOWS, pd_base_addr >> 3, pd_offset_addr >> 3);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);

    /* map all runners (16 registers) to profile 0 for processing and profile 7 for replication */
    for (index = 0; index < 16; index++)
    {
        dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof mapping_pd_rnr_prof;

        mapping_pd_rnr_prof.rnr_prof_0_1 = DISPATCHER_GROUP_PROCESSING;
        mapping_pd_rnr_prof.rnr_prof_1_1 = DISPATCHER_GROUP_PROCESSING;
        mapping_pd_rnr_prof.rnr_prof_2_1 = DISPATCHER_GROUP_PROCESSING;
        mapping_pd_rnr_prof.rnr_prof_3_1 = DISPATCHER_GROUP_PROCESSING;
        mapping_pd_rnr_prof.rnr_prof_0_2 = DISPATCHER_GROUP_REPLICATION;
        mapping_pd_rnr_prof.rnr_prof_1_2 = DISPATCHER_GROUP_REPLICATION;
        mapping_pd_rnr_prof.rnr_prof_2_2 = DISPATCHER_GROUP_REPLICATION;
        mapping_pd_rnr_prof.rnr_prof_3_2 = DISPATCHER_GROUP_REPLICATION;

        rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_pd_rnr_prof_set(0, index, &mapping_pd_rnr_prof);
        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d ; reg_idx = %d\n", 0, index);
    }

    /* profile select 0/1. profile per runner group */
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel0 = 0;
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel2 = 0;
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel4 = 0;
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel6 = 0;
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel1 = 0;
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel3 = 0;
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel5 = 0;
    task_arb_reord_task_arb_queue_mapping_rg_prof_sel.rnr_g_sel7 = 1;

    ag_drv_dsptchr_cmn_task_arb_reord_task_arb_queue_mapping_rg_prof_sel_set(0, &task_arb_reord_task_arb_queue_mapping_rg_prof_sel);

    /* configure task mask per runner group (each group uses 32 registers ) each register holds configuration of two runners */
    /* in this specific case configure runner group 0 only (first 32 registers) */
    /*
        In order to write to RNR0  you write to register 31 (LSB bits)
        In order to write to RNR1  you write to register 31 (MSB bits)
        In order to write to RNR2  you write to register 30 (LSB bits)
        In order to write to RNR3  you write to register 30 (MSB bits)
        .
        .
        .
        In order to write to RNR22 you write to register 20 (LSB bits)
        In order to write to RNR23 you write to register 20 (LSB bits)
    */

    for (index = 0; index < 12; index++) 
    {
        if (valid_rnr_core[index])
        {
            /* each runner tasks from 8-15 will be used as processing tasks */
            uint32_t processing_task_mask = 0xFF00FF00;

            rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(0, 31 - index, processing_task_mask);
            BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);
        }
    }

    for (index = 0; index < 32; index++)
    {
        if (valid_rnr_core[(63 - index*2)])
        {
            /* each runner tasks from 4-5 will be used as replication tasks (DS) */
            rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(0, DISPATCHER_GROUP_REPLICATION * 32 + index, 0x00300030);
            BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);
        }
    }

    for (index = 0; index < 32; index++)
    {
        if (valid_rnr_core[(63 - index*2)])
        {
            /* each runner task 5 will be used as direct flow processing tasks  (US) */
            rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(0, DISPATCHER_GROUP_DIRECT_FLOWS * 32 + index, 0x00080008); 
            BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);
        }
    }

    if (valid_rnr_core[(63)])
    {
        /* disable processing and direct_flow on US Runner 63. */
        rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(0, DISPATCHER_GROUP_PROCESSING * 32, 0x0000FFC0); 
        rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_mask_msk_tsk_1023_0_set(0, DISPATCHER_GROUP_DIRECT_FLOWS * 32, 0x00000008); 

        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);
    }
    
    /* TODO task arb: set RR priority mode in dispatcher*/
    rc = ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_lb_cfg_set(0, 0, 0); 
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);

    rc = ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_get(0, &task_arb_reord_reord_chn_cfg_chn_mode);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);
    task_arb_reord_reord_chn_cfg_chn_mode.chn_crdt_init = 1;
    rc = ag_drv_dsptchr_cmn_task_arb_reord_reord_chn_cfg_chn_mode_set(0, &task_arb_reord_reord_chn_cfg_chn_mode);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: dispatcher_init failed. cmn dispatcher index: %d\n", 0);

#if defined(RDP_SIM)
    /* reset value, mark all cores tasks status as free */
    MEMSET((uint8_t *)DEVICE_ADDRESS(RU_BLK(DSPTCHR_CMN).addr[0] + RU_REG_OFFSET(DSPTCHR_CMN, TASK_ARB_REORD_TASK_ARB_LOAD_BALANCING_FREE_TASK)),
        0xFF, sizeof(uint32_t) * 32);
#endif
    
    return BDMF_ERR_OK;
}

bdmf_error_t drv_dispatcher_chan_viq_cfg_set(int dsptchr_chan_index, uint8_t viq_index, dsptchr_config* dsptchr_cfg, bdmf_boolean reset)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    assert(viq_index < dsptchr_cfg->fll_entry[dsptchr_chan_index].count);

    /* update dispatcher channel with viq_index */
    {
        uint32_t viq_mask;

        rc = ag_drv_dsptchr_reorder_cfg_vq_en_get(dsptchr_chan_index, &viq_mask);
        if (reset)
        {
            viq_mask &= ~(1 << viq_index);
        }
        else
        {
            viq_mask |= (1 << viq_index);
        }

        rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_vq_en_set(dsptchr_chan_index, viq_mask);
    }

    /* update relevant rnr_group_idx with viq_index */
    if (dsptchr_cfg->rnr_group_idx[dsptchr_chan_index][viq_index] != RUNNER_TASK_GROUP_NONE)
    {
        uint16_t viq_mask;

        rc = rc ? rc : ag_drv_dsptchr_mask_msk_q_get(dsptchr_chan_index, dsptchr_cfg->rnr_group_idx[dsptchr_chan_index][viq_index], &viq_mask);
        if (reset)
            viq_mask &= ~(1 << viq_index);
        else
            viq_mask |= (1 << viq_index);

        rc = rc ? rc : ag_drv_dsptchr_mask_msk_q_set(dsptchr_chan_index, dsptchr_cfg->rnr_group_idx[dsptchr_chan_index][viq_index], viq_mask);
    }

    /* remaining per viq config */
    rc = rc ? rc : ag_drv_dsptchr_cngs_params_set(dsptchr_chan_index, viq_index, &(dsptchr_cfg->dsptchr_viq_list[dsptchr_chan_index][viq_index].ingress_cngs));
    rc = rc ? rc: ag_drv_dsptchr_congestion_egrs_congstn_set(dsptchr_chan_index, viq_index, &(dsptchr_cfg->dsptchr_viq_list[dsptchr_chan_index][viq_index].egress_cngs));
    rc = rc ? rc: ag_drv_dsptchr_queue_mapping_crdt_cfg_set(dsptchr_chan_index, viq_index, dsptchr_cfg->dsptchr_viq_list[dsptchr_chan_index][viq_index].bb_id, dsptchr_cfg->dsptchr_viq_list[dsptchr_chan_index][viq_index].bbh_target_address);
    rc = rc ? rc: ag_drv_dsptchr_queue_mapping_q_dest_set(dsptchr_chan_index, viq_index, dsptchr_cfg->dsptchr_viq_list[dsptchr_chan_index][viq_index].queue_dest);
    rc = rc ? rc: ag_drv_dsptchr_q_limits_params_set(dsptchr_chan_index, viq_index, dsptchr_cfg->dsptchr_viq_list[dsptchr_chan_index][viq_index].common_max_limit, 
                                            dsptchr_cfg->dsptchr_viq_list[dsptchr_chan_index][viq_index].guaranteed_max_limit);
    rc = rc ? rc: ag_drv_dsptchr_qdes_head_set(dsptchr_chan_index, viq_index, viq_index);
    rc = rc ? rc: ag_drv_dsptchr_qdes_tail_set(dsptchr_chan_index, viq_index, viq_index);
    rc = rc ? rc: ag_drv_dsptchr_qdes_reg_q_head_set(dsptchr_chan_index, viq_index, viq_index);

    return rc;
}

bdmf_error_t drv_dispatcher_reorder_available_tasks(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint16_t tsk_cnt_rg[8] = {};
    uint8_t index;
    uint16_t rnr0, rnr1;

    ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_0_1_get(0, &tsk_cnt_rg[0], &tsk_cnt_rg[1]);
    ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_2_3_get(0, &tsk_cnt_rg[2], &tsk_cnt_rg[3]);
    ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_4_5_get(0, &tsk_cnt_rg[4], &tsk_cnt_rg[5]);
    ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_rg_avlabl_tsk_6_7_get(0, &tsk_cnt_rg[6], &tsk_cnt_rg[7]);

    bdmf_session_print(session,  "--------------------------------------------------------------\n\r");
    bdmf_session_print(session,  "runner group |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |\n\r");
    bdmf_session_print(session,  "--------------------------------------------------------------\n\r");
    bdmf_session_print(session,  "avail tasks  ");

    bdmf_session_print(session,  "| %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d |\n\r", tsk_cnt_rg[0], tsk_cnt_rg[1], tsk_cnt_rg[2], tsk_cnt_rg[3],
                       tsk_cnt_rg[4], tsk_cnt_rg[5], tsk_cnt_rg[6], tsk_cnt_rg[7]);

    for (index = 0; index < (NUM_OF_PROCESSING_RUNNERS / 2); index++)
    {
        ag_drv_dsptchr_cmn_task_arb_reord_task_arb_load_balancing_free_task_get(0, index, &rnr0, &rnr1);
        bdmf_session_print(session,  "core %d: %x ; core %d: %x\n\r", index*2, rnr0, index*2+1, rnr1);
    }

    return rc;
}

bdmf_error_t drv_dispatcher_reorder_viq_status(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t dsptchr_id = parm[0].value.number;
    dsptchr_glbl_cngs_params glbl_cngs_params;
    dsptchr_cngs_params cngs_params;
    uint32_t q_idx, bufin, bufout, bufavail, drpcnt, bufmin;
    uint16_t cmn_max, gurntd_max, credit_cnt, q_egrs_size, total_egrs_size;
    dsptchr_pools_limits pools_limits = {};


    bdmf_session_print(session, "---------------------------------------------------------------------------------------------------------\n\r");
    bdmf_session_print(session, "queue | cmmn | grntd | crdt |egress| buf     | buf     | buf  |   cong ingress     |   cong egress      |\n\r");
    bdmf_session_print(session, "      | max  | max   | cntr | cntr | in      | out     |avail | frst | scnd | hyst | frst | scnd | hyst |\n\r");
    bdmf_session_print(session, "---------------------------------------------------------------------------------------------------------\n\r");

    for (q_idx = 0; q_idx < DISPATCHER_CHAN_VIQ_NUM; q_idx++)
    {
        ag_drv_dsptchr_q_limits_params_get(dsptchr_id, q_idx, &cmn_max, &gurntd_max);
        ag_drv_dsptchr_credit_cnt_get(dsptchr_id, q_idx, &credit_cnt);
        ag_drv_dsptchr_cngs_params_get(dsptchr_id, q_idx, &cngs_params);
        ag_drv_dsptchr_egrs_queues_per_q_egrs_size_get(dsptchr_id, q_idx, &q_egrs_size);
        ag_drv_dsptchr_qdes_bufin_get(dsptchr_id, q_idx, &bufin);
        ag_drv_dsptchr_qdes_bfout_get(dsptchr_id, q_idx, &bufout);
        ag_drv_dsptchr_qdes_bufavail_get(dsptchr_id, q_idx, &bufavail);
        bdmf_session_print(session, "[%3d] | %4d | %4d  | %4d | %4d | %7d | %7d | %4d | %4d | %4d | %4d |", 
            q_idx, cmn_max, gurntd_max, credit_cnt, q_egrs_size, bufin, bufout, bufavail, cngs_params.frst_lvl, cngs_params.scnd_lvl, cngs_params.hyst_thrs);
        ag_drv_dsptchr_congestion_egrs_congstn_get(dsptchr_id, q_idx, &cngs_params);
        bdmf_session_print(session, " %4d | %4d | %4d |\n\r", cngs_params.frst_lvl, cngs_params.scnd_lvl, cngs_params.hyst_thrs);
    }
    ag_drv_dsptchr_flldes_flldrop_get(dsptchr_id, &drpcnt);
    ag_drv_dsptchr_flldes_bufavail_get(dsptchr_id, &bufavail);
    ag_drv_dsptchr_flldes_freemin_get(dsptchr_id, &bufmin);
    bdmf_session_print(session, "FLL drop = %u\n", drpcnt);
    bdmf_session_print(session, "FLL buf avail = %u\n", bufavail);
    bdmf_session_print(session, "FLL buf min = %u\n", bufmin);
    ag_drv_dsptchr_pools_limits_get(dsptchr_id, &pools_limits);
    bdmf_session_print(session, "cmn_pool_lmt = %u (0x%x)\n", pools_limits.cmn_pool_lmt, pools_limits.cmn_pool_lmt);
    bdmf_session_print(session, "grnted_pool_lmt = %u (0x%x)\n", pools_limits.grnted_pool_lmt, pools_limits.grnted_pool_lmt);
    bdmf_session_print(session, "cmn_pool_size = %u (0x%x)\n", pools_limits.cmn_pool_size, pools_limits.cmn_pool_size);
    bdmf_session_print(session, "grnted_pool_size = %u (0x%x)\n", pools_limits.grnted_pool_size, pools_limits.grnted_pool_size);
    ag_drv_dsptchr_egrs_queues_total_q_egrs_size_get(dsptchr_id, &total_egrs_size);
    bdmf_session_print(session, "total_egrs_size = %u (0x%x)\n", total_egrs_size, total_egrs_size);
    ag_drv_dsptchr_congestion_total_egrs_congstn_get(0, dsptchr_id, &cngs_params);
    bdmf_session_print(session, "Total egress congstn: first-%4d | second-%4d | hyst-%4d |\n", cngs_params.frst_lvl, cngs_params.scnd_lvl, cngs_params.hyst_thrs);

    ag_drv_dsptchr_glbl_cngs_params_get(dsptchr_id, &glbl_cngs_params);
    bdmf_session_print(session, "Global congstn: first-%4d | second-%4d | hyst-%4d |", glbl_cngs_params.frst_lvl, glbl_cngs_params.scnd_lvl, glbl_cngs_params.hyst_thrs);

    return rc;
}
