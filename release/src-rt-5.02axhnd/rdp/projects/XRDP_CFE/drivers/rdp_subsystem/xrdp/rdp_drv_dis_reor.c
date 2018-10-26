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

#include "rdp_subsystem_common.h"
#include "rdp_drv_dis_reor.h"
#include "rdp_common.h"
#include "XRDP_AG.h"

#define POLLING_TIME_OUT  1000

uint32_t g_rnr_grp_num;

int drv_dis_reor_queues_init()
{
    uint8_t i;
    dsptchr_cngs_params congs_init = {.frst_lvl = (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1), .scnd_lvl = (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1), 
        .hyst_thrs = ((DIS_REOR_LINKED_LIST_BUFFER_NUM / 4) - 1)};
    dsptchr_glbl_cngs_params glbl_congs_init = {.frst_lvl = (DIS_REOR_CONGESTION_THRESHOLD - NUM_OF_PROCESSING_TASKS), 
        .scnd_lvl = DIS_REOR_CONGESTION_THRESHOLD, .hyst_thrs = 10};
    bdmf_error_t rc = BDMF_ERR_OK;

    /* Ingress/Egress congestion mem */
    for (i = 0; i < DSPTCHR_VIRTUAL_QUEUE_NUM; i++)
    {
        rc = rc ? rc : ag_drv_dsptchr_cngs_params_set(i, &congs_init);
        rc = rc ? rc : ag_drv_dsptchr_congestion_egrs_congstn_set(i, &congs_init);
        rc = rc ? rc : ag_drv_dsptchr_qdes_head_set(i, 0);
        rc = rc ? rc : ag_drv_dsptchr_credit_cnt_set(i, 0);
    }

    rc = rc ? rc : ag_drv_dsptchr_congestion_total_egrs_congstn_set((dsptchr_glbl_cngs_params *)&congs_init);
    return rc ? rc : ag_drv_dsptchr_glbl_cngs_params_set(&glbl_congs_init);
};

/* array of mapping returning tasks to groups init */
int drv_dis_reor_tasks_to_rg_init()
{
    uint8_t i;
    dsptchr_load_balancing_tsk_to_rg_mapping tsk_to_rg = {};
    bdmf_error_t rc = BDMF_ERR_OK;

    for (i = 0; i < (RUNNER_GROUP_MAX_TASKS_NUM / 8); i++)
    {
        rc = rc ? rc : ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set(i, &tsk_to_rg);
    }
    return rc;
}

/* buffers linked list initialization */
int drv_dis_reor_free_linked_list_init()
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint16_t i;
    dsptchr_fll_entry fll_entry;

    for (i = 0; i < (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1); i++)
    {
#ifdef BCM6856
        rc = ag_drv_dsptchr_bdram_next_data_set(i, i+1);
#else
        rc = ag_drv_dsptchr_bdram_data_set(i, i+1);
#endif
        if (rc)
            return rc;
    }

    fll_entry.minbuf = DIS_REOR_LINKED_LIST_BUFFER_NUM; /* linked list minimum value */
    fll_entry.head   = 0;
    fll_entry.tail   = (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1);
    fll_entry.bfin   = DIS_REOR_LINKED_LIST_BUFFER_NUM;
    fll_entry.count  = 0;
    return ag_drv_dsptchr_fll_entry_set(&fll_entry);
}

static int _drv_dis_reor_runner_group_cfg(uint8_t grp_idx, dsptchr_runner_group *rnr_grp)
{
    uint32_t cur_mask;
    uint8_t i,j,idx = 0;
    bdmf_error_t rc;
    dsptchr_load_balancing_tsk_to_rg_mapping tsk_to_rg = {};
  
    rc = ag_drv_dsptchr_mask_msk_tsk_255_0_set(grp_idx, &rnr_grp->tasks_mask);
    rc = rc ? rc : ag_drv_dsptchr_mask_msk_q_set(grp_idx, rnr_grp->queues_mask);
    if (rc)
        return rc;

    for (i = 0; i < (RUNNER_GROUP_MAX_TASKS_NUM / 8); i++)
    {
        rc = ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_get(i, &tsk_to_rg);
        if (rc)
            return rc;
        for (j = 0; j < 8; j++)
        {
            cur_mask = rnr_grp->tasks_mask.task_mask[idx/32];
            if ((cur_mask >> (idx % 32)) & 0x1)
            {
                if      (j == 0) { tsk_to_rg.tsk0 = grp_idx; }
                else if (j == 1) { tsk_to_rg.tsk1 = grp_idx; }
                else if (j == 2) { tsk_to_rg.tsk2 = grp_idx; }
                else if (j == 3) { tsk_to_rg.tsk3 = grp_idx; }
                else if (j == 4) { tsk_to_rg.tsk4 = grp_idx; }
                else if (j == 5) { tsk_to_rg.tsk5 = grp_idx; }
                else if (j == 6) { tsk_to_rg.tsk6 = grp_idx; }
                else if (j == 7) { tsk_to_rg.tsk7 = grp_idx; }
            }
            idx++;
        }
        rc = ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set(i, &tsk_to_rg);
        if (rc)
            return rc;
    }
    return 0;
}

static int _drv_dis_reor_viq_cfg(uint8_t q_idx, dsptchr_ingress_queue *viq)
{
    uint16_t q_head;
    dsptchr_fll_entry fll_entry = {};
    bdmf_error_t rc;
    dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr isr_struct = {};

    rc = ag_drv_dsptchr_cngs_params_set(q_idx, &viq->ingress_cngs);
    rc = rc ? rc : ag_drv_dsptchr_congestion_egrs_congstn_set(q_idx, &viq->egress_cngs);

    /* Set BBH parameters */
    rc = rc ? rc : ag_drv_dsptchr_queue_mapping_crdt_cfg_set(q_idx, (uint8_t) viq->bb_id, viq->bbh_target_address);
    if (rc)
        return rc;

    /* TODO: add wakeup_thrs once the FFs are added to Reggae */

    /* Q dest handling */
    ag_drv_dsptchr_queue_mapping_q_dest_set(q_idx, viq->queue_dest);

    /* delayed Q handling */
    if (viq->delayed_queue == 1)
        rc = rc ? rc : ag_drv_dsptchr_mask_dly_q_set(q_idx, 1);
    else
        rc = rc ? rc : ag_drv_dsptchr_mask_non_dly_q_set(q_idx, 1);

    rc = rc ? rc : ag_drv_dsptchr_q_limits_params_set(q_idx, viq->common_max_limit, viq->guaranteed_max_limit);
    rc = rc ? rc : ag_drv_dsptchr_ingress_coherency_params_set(q_idx, viq->coherency_en, viq->coherency_ctr);
    if (rc)
        return rc;


    /* set the queue's LL params - at start both points to same entry */
    fll_entry.minbuf = DIS_REOR_LINKED_LIST_BUFFER_NUM;
    fll_entry.head  = 0;
    fll_entry.tail = (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1);
    rc = ag_drv_dsptchr_fll_entry_get(&fll_entry);
    rc = rc ? rc : ag_drv_dsptchr_qdes_head_set(q_idx, fll_entry.head);
    rc = rc ? rc : ag_drv_dsptchr_qdes_tail_set(q_idx, fll_entry.head);
    /* advance the FLL head pointer */
#ifdef BCM6856
    rc = rc ? rc : ag_drv_dsptchr_bdram_next_data_get(fll_entry.head, &q_head);
#else
    rc = rc ? rc : ag_drv_dsptchr_bdram_data_get(fll_entry.head, &q_head);
#endif
    if (rc)
        return rc;

    fll_entry.head = q_head;
    fll_entry.count += 1;
    rc = ag_drv_dsptchr_fll_entry_set(&fll_entry);
    if (rc)
        return rc;

    /* clear isr */
    isr_struct.fll_return_buf = 1;
    isr_struct.fll_cnt_drp = 1;
    isr_struct.unknwn_msg = 1;
    isr_struct.fll_overflow = 1;
    isr_struct.fll_neg = 1;

    return ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set(&isr_struct);
};

int drv_dis_reor_cfg(dsptchr_config *cfg)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t i;

    for (i = 0; i < cfg->viq_num; i++)
    {
        rc = rc ? rc : _drv_dis_reor_viq_cfg(i, & cfg->dsptchr_viq_list[i]);
    }
    g_rnr_grp_num = cfg->rnr_grp_num;
    for (i = 0; i < g_rnr_grp_num; i++)
    {
        rc = rc ? rc : _drv_dis_reor_runner_group_cfg(i, & cfg->dsptchr_rnr_group_list[i]);
    }
    for (i = 0; i < GROUPED_EN_SEGMENTS_NUM; i++)
    {
        if (cfg->rnr_disp_en_vector & (1 << i))
            rc = rc ? rc : ag_drv_dsptchr_rnr_dsptch_addr_set(cfg->dsptchr_rnr_cfg[i].rnr_num,
                &cfg->dsptchr_rnr_cfg[i].rnr_addr_cfg);
    }

    rc = rc ? rc : ag_drv_dsptchr_pools_limits_set(&cfg->pools_limits);

    rc = rc ? rc : ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set(cfg->rg_available_tasks[0],
        cfg->rg_available_tasks[1], cfg->rg_available_tasks[2], cfg->rg_available_tasks[3]);
    rc = rc ? rc : ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set(cfg->rg_available_tasks[4],
        cfg->rg_available_tasks[5], cfg->rg_available_tasks[6], cfg->rg_available_tasks[7]);

    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_vq_en_set(cfg->queue_en_vec);
#if defined(RDP_SIM) && !defined(XRDP_EMULATION)
    /* reset value, mark all cores tasks status as free */
    MEMSET((uint8_t *)DEVICE_ADDRESS(RU_BLK(DSPTCHR).addr[0] + RU_REG_OFFSET(DSPTCHR, LOAD_BALANCING_FREE_TASK_0_1)),
        0xFF, sizeof(uint32_t) * 8);
#endif

    return rc;
};
int  drv_dis_reor_viq_queue_mapping_modify(rdd_disp_reor_viq viq, uint16_t new_target_address, uint8_t new_bb_id)
{
    int rc;
    uint8_t old_bb_id;
    uint16_t old_target_address;
    uint32_t queue_en_vec = 0;

    rc = ag_drv_dsptchr_queue_mapping_crdt_cfg_get(viq, &old_bb_id, &old_target_address);
    if (rc) 
        return rc;
    /* if no change , reutrn, it is assumes that RAM address cannot change if thread num didn't change*/
    if ((old_bb_id == new_bb_id) && (new_target_address == old_target_address))  
        return BDMF_ERR_OK;
    /* disable configured queue */
    rc = ag_drv_dsptchr_reorder_cfg_vq_en_get(&queue_en_vec);
    queue_en_vec &= ~(1 << viq);
    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_vq_en_set(queue_en_vec);
    
    /* modify configured queue */
    rc = rc ? rc : ag_drv_dsptchr_queue_mapping_crdt_cfg_set(viq, new_bb_id, new_target_address);

    /* enable configured queue */
    queue_en_vec |= (1 << viq);
    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_vq_en_set(queue_en_vec);

    return rc;
       
}

int drv_dis_reor_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{
     dsptchr_reorder_cfg_clk_gate_cntrl disp_ctrl;

     ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get(&disp_ctrl);
     disp_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
     disp_ctrl.timer_val = timer_val;
     ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set(&disp_ctrl);

     return 0;
}

#ifdef USE_BDMF_SHELL
static inline uint8_t _rdp_drv_dis_reor_queue_num_get(uint8_t *viq_num)
{
    uint32_t viq_en;
    bdmf_error_t rc;

    rc = ag_drv_dsptchr_reorder_cfg_vq_en_get(&viq_en);
    if (rc)
        return rc;
    *viq_num = asserted_bits_count_get(viq_en);

    return 0;
}

int drv_dis_reor_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t q_idx;
    uint8_t viq_num = 0;
    bdmf_error_t rc;
    static uint32_t fll_cfg[] = {cli_dsptchr_fll_entry};
    static uint32_t q_cfg[] = {cli_dsptchr_qdes_head, cli_dsptchr_qdes_tail};

    bdmf_session_print(session, "Free Linked List:\n");
    HAL_CLI_PRINT_LIST(session, dsptchr, fll_cfg);

    rc = _rdp_drv_dis_reor_queue_num_get(&viq_num);
    if (rc)
        return rc;

    bdmf_session_print(session, "Queue Debug Info:\n");
    for (q_idx = 0; q_idx < viq_num; q_idx++)
    {
        bdmf_session_print(session, "queue [%d]:\n", q_idx);
        HAL_CLI_IDX_PRINT_LIST(session, dsptchr, q_cfg, q_idx);
    }
    return 0;
}

int drv_dis_reor_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t viq_num = 0;
    uint32_t q_idx, rg_idx, core_idx;
    uint32_t glbl_qs_cfg[] = {cli_dsptchr_reorder_cfg_dsptchr_reordr_cfg, cli_dsptchr_congestion_glbl_congstn,
        cli_dsptchr_pools_limits, cli_dsptchr_pools_limits, cli_dsptchr_load_balancing_rg_avlabl_tsk_0_3,
        cli_dsptchr_load_balancing_rg_avlabl_tsk_4_7};
    uint32_t wkp_ctrl_cfg[] = {cli_dsptchr_wakeup_control_wkup_req, cli_dsptchr_wakeup_control_wkup_thrshld};
    uint32_t q_cfg[] = {cli_dsptchr_cngs_params, cli_dsptchr_congestion_egrs_congstn, cli_dsptchr_qdes_head,
        cli_dsptchr_queue_mapping_crdt_cfg, cli_dsptchr_queue_mapping_q_dest, cli_dsptchr_mask_dly_q,
        cli_dsptchr_mask_non_dly_q, cli_dsptchr_q_limits_params, cli_dsptchr_ingress_coherency_params};
    uint32_t rg_cfg[] = {cli_dsptchr_mask_msk_tsk_255_0, cli_dsptchr_mask_msk_q};
    /*uint32_t rg_ld_blncr_cfg[] = {cli_dsptchr_load_balancing_tsk_to_rg_mapping};*/
    uint32_t prcs_rnr_cfg[] = {cli_dsptchr_rnr_dsptch_addr};
    bdmf_error_t rc;

    bdmf_session_print(session, "Dispatcher Reorder configurations:\n\r");
    bdmf_session_print(session, "Global configurations:\n");
    HAL_CLI_PRINT_LIST(session, dsptchr, glbl_qs_cfg);
    bdmf_session_print(session, "Wakeup Thresholds:\n");
    HAL_CLI_PRINT_LIST(session, dsptchr, wkp_ctrl_cfg);

    bdmf_session_print(session, "Runner group configurations:\n");
    for (rg_idx = 0; rg_idx < g_rnr_grp_num; rg_idx++)
    {
        bdmf_session_print(session, "RG[%d]:\n", rg_idx);
        HAL_CLI_IDX_PRINT_LIST(session, dsptchr, rg_cfg, rg_idx);
        /*for (core_idx = 0; core_idx < (RUNNER_GROUP_MAX_TASKS_NUM / 8); core_idx++)
        {
            bdmf_session_print(session, "core[%d]:\n", core_idx);
            parm[1].value.unumber = core_idx;
            HAL_CLI_PRINT_LIST(dsptchr, session, parm, 2, rg_ld_blncr_cfg);
        }*/
    }

    bdmf_session_print(session, "runner configrations:\n");
    for (core_idx = 0; core_idx < g_rnr_grp_num; core_idx++)
    {
        bdmf_session_print(session, "Runner[%d]:\n", core_idx);
        HAL_CLI_IDX_PRINT_LIST(session, dsptchr, prcs_rnr_cfg, core_idx);
    }

    bdmf_session_print(session, "Queues configurations:\n");
    bdmf_session_print(session, "Enabled queues:\n");
    parm[0].value.unumber = cli_dsptchr_reorder_cfg_vq_en;
    rc = bcm_dsptchr_cli_get(session, parm, 1);
    rc = rc ? rc : _rdp_drv_dis_reor_queue_num_get(&viq_num);
    if (rc)
        return rc;

    /* Ingress/Egress congestion mem */
    for (q_idx = 0; q_idx < viq_num; q_idx++)
    {
        bdmf_session_print(session, "queue [%d]:\n", q_idx);
        HAL_CLI_IDX_PRINT_LIST(session, dsptchr, q_cfg, q_idx);
    }

    return 0;
}

/* dispatcher reorder isolate flow */
static dsptchr_isolate_restore_t restore_grp[RUNNER_GROUP_MAX_NUM] = {};

static int drv_dis_reor_isolate_task_cfg(uint8_t rnr_group, uint8_t num_of_tasks)
{
    int rc;
    uint8_t rg_available_tasks[RUNNER_GROUP_MAX_TASK_NUM] = {};

    if (rnr_group >= RUNNER_GROUP_MAX_NUM)
        return BDMF_ERR_PARM;

    rc = ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_get(&rg_available_tasks[0], &rg_available_tasks[1],
        &rg_available_tasks[2], &rg_available_tasks[3]);
    rc = rc ? rc : ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_get(&rg_available_tasks[4], &rg_available_tasks[5],
        &rg_available_tasks[6], &rg_available_tasks[7]);

    rg_available_tasks[rnr_group] = num_of_tasks;

    rc = rc ? rc : ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set(rg_available_tasks[0], rg_available_tasks[1],
        rg_available_tasks[2], rg_available_tasks[3]);
    rc = rc ? rc : ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set(rg_available_tasks[4], rg_available_tasks[5],
        rg_available_tasks[6], rg_available_tasks[7]);

    return rc;
}

int drv_dis_reor_isolate_set(uint8_t core_idx, uint8_t task_idx, uint8_t rnr_grp_idx)
{
    int rc;
    dsptchr_runner_group rnr_group = {};

    if ((core_idx > RNR_LAST) ||
        (task_idx >= RUNNER_MAX_TASKS_NUM) ||
        (rnr_grp_idx >= RUNNER_GROUP_MAX_NUM))
    {
        return BDMF_ERR_PARM;
    }

    /* store the task mask to restore */
    ag_drv_dsptchr_mask_msk_tsk_255_0_get(rnr_grp_idx, &restore_grp[rnr_grp_idx].task_mask);
    ag_drv_dsptchr_mask_msk_q_get(rnr_grp_idx ,&restore_grp[rnr_grp_idx].queue_mask);

    /* configure the rnr group with only one task */
    rnr_group.tasks_mask.task_mask[core_idx/2] |= (task_idx << ((core_idx & 1) << 4));
    rnr_group.queues_mask = restore_grp[rnr_grp_idx].queue_mask;
    rc = _drv_dis_reor_runner_group_cfg(rnr_grp_idx, &rnr_group);
    rc = rc ? rc : drv_dis_reor_isolate_task_cfg(rnr_grp_idx, 1);
    if (!rc)
        restore_grp[rnr_grp_idx].is_isolate = 1;

    return rc;
}

int drv_dis_reor_restore_isolate_set(uint8_t rnr_grp_idx)
{
    int rc;
    uint8_t i;
    uint8_t task_count = 0;
    dsptchr_runner_group rnr_group = {};

    if (rnr_grp_idx >= RUNNER_GROUP_MAX_NUM)
        return BDMF_ERR_PARM;

    if (!restore_grp[rnr_grp_idx].is_isolate)
        return BDMF_ERR_PERM;

    /* restore runner group configurations */
    rnr_group.tasks_mask = restore_grp[rnr_grp_idx].task_mask;
    rnr_group.queues_mask = restore_grp[rnr_grp_idx].queue_mask;
    rc = _drv_dis_reor_runner_group_cfg(rnr_grp_idx, &rnr_group);

    /* restore all viq to rnr group mapping */
    for (i = 0; i < 8; i++)
    {
        task_count += asserted_bits_count_get(rnr_group.tasks_mask.task_mask[i]);
    }

    /* restore the original task count of the runner group */
    rc = rc ? rc : drv_dis_reor_isolate_task_cfg(rnr_grp_idx, task_count);

    if (!rc)
        restore_grp[rnr_grp_idx].is_isolate = 0;

    return rc;
}

static bdmfmon_handle_t dis_reor_dir;

void drv_dis_reor_cli_init(bdmfmon_handle_t driver_dir)
{
    dis_reor_dir = ag_drv_dsptchr_cli_init(driver_dir);
    BDMFMON_MAKE_CMD_NOPARM(dis_reor_dir, "cfg_get", "dispatcher-reorder configuration",
        (bdmfmon_cmd_cb_t)drv_dis_reor_cli_config_get);
}

void drv_dis_reor_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (dis_reor_dir)
    {
        bdmfmon_token_destroy(dis_reor_dir);
        dis_reor_dir = NULL;
    }
}

#endif /* USE_BDMF_SHELL */

