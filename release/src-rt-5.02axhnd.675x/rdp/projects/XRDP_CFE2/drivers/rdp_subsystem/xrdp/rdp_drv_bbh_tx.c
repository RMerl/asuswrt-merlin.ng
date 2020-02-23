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
#include "xrdp_drv_drivers_common_ag.h"
#include "rdp_subsystem_common.h"
#include "rdp_common.h"
#include "rdp_drv_bbh_tx.h"
#include "rdd_data_structures_auto.h"
#ifdef XRDP_BBH_PER_LAN_PORT
#define TX_LAN_QUEUE_PAIRS (1)
#else
#define TX_LAN_QUEUE_PAIRS ((RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_SIZE + 1) / 2)
#endif


bdmf_error_t drv_bbh_tx_common_configurations_q2rnr_set_lan(uint8_t bbh_id, uint8_t q_2_rnr_index, bdmf_boolean q0, bdmf_boolean q1)
{
#if !defined(DUAL_ISSUE)
    return ag_drv_bbh_tx_common_configurations_q2rnr_set(bbh_id, q_2_rnr_index, q0, q1);
#else
    return ag_drv_bbh_tx_unified_configurations_q2rnr_set(bbh_id, q_2_rnr_index, q0, q1);
#endif
}

bdmf_error_t drv_bbh_tx_common_configurations_q2rnr_set_wan(uint8_t bbh_id, uint8_t q_2_rnr_index, bdmf_boolean q0, bdmf_boolean q1)
{
#if !defined(DUAL_ISSUE)
    return ag_drv_bbh_tx_common_configurations_q2rnr_set(bbh_id, q_2_rnr_index, q0, q1);
#else
    return ag_drv_bbh_tx_wan_configurations_q2rnr_set(bbh_id, q_2_rnr_index, q0, q1);
#endif
}

bdmf_error_t drv_bbh_tx_common_configurations_q2rnr_get_wan(uint8_t bbh_id_tx, uint8_t q_2_rnr_index, bdmf_boolean *q0, bdmf_boolean *q1)
{
#if !defined(DUAL_ISSUE)
    return ag_drv_bbh_tx_common_configurations_q2rnr_get(bbh_id_tx, q_2_rnr_index, q0, q1);
#else
    return ag_drv_bbh_tx_wan_configurations_q2rnr_get(bbh_id_tx, q_2_rnr_index, q0, q1);
#endif
}

bdmf_error_t drv_bbh_tx_common_configurations_q2rnr_get_lan(uint8_t bbh_id_tx, uint8_t q_2_rnr_index, bdmf_boolean *q0, bdmf_boolean *q1)
{
#if !defined(DUAL_ISSUE)
    return ag_drv_bbh_tx_common_configurations_q2rnr_get(bbh_id_tx, q_2_rnr_index, q0, q1);
#else
    return ag_drv_bbh_tx_unified_configurations_q2rnr_get(bbh_id_tx, q_2_rnr_index, q0, q1);
#endif
}


int drv_bbh_tx_wan_queue_cfg_set(uint8_t bbh_id, pd_queue_cfg_t *wan_queue_cfg)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t i;

#if !defined(DUAL_ISSUE)

    for (i = 0; i < TX_QEUEU_PAIRS; i++)
    {
        rc = rc ? rc : drv_bbh_tx_common_configurations_q2rnr_set_wan(bbh_id, i, wan_queue_cfg->queue_to_rnr[i].q0, wan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdbase_set(bbh_id, i, wan_queue_cfg->pd_fifo_base[i].base0, wan_queue_cfg->pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdsize_set(bbh_id, i, wan_queue_cfg->pd_fifo_size[i].size0, wan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdwkuph_set(bbh_id, i, wan_queue_cfg->pd_wkup_threshold[i].threshold0, wan_queue_cfg->pd_wkup_threshold[i].threshold1);
#if defined(BCM63158)
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(bbh_id, i, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE);
#else
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(bbh_id, i, wan_queue_cfg->pd_bytes_threshold[0].threshold0, wan_queue_cfg->pd_bytes_threshold[0].threshold1);
#endif
    }
#else
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_gpr_set(bbh_id , 3);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdsize_set(bbh_id, wan_queue_cfg->pd_fifo_size[0].size0, wan_queue_cfg->pd_fifo_size[0].size1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdwkuph_set(bbh_id, wan_queue_cfg->pd_wkup_threshold[0].threshold0, wan_queue_cfg->pd_wkup_threshold[0].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(bbh_id, wan_queue_cfg->pd_bytes_threshold[0].threshold0, wan_queue_cfg->pd_bytes_threshold[0].threshold1);
        for (i = 0; i < TX_QEUEU_PAIRS; i++)
        {
            rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_q2rnr_set(bbh_id, i, wan_queue_cfg->queue_to_rnr[i].q0, wan_queue_cfg->queue_to_rnr[i].q1);
            rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_qprof_set(bbh_id, i, wan_queue_cfg->queue_profile[i].q0, wan_queue_cfg->queue_profile[i].q1);
            rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_qmq_set(bbh_id, i, wan_queue_cfg->qm_q[i].q0, wan_queue_cfg->qm_q[i].q1);
        }
#endif
    return rc;
}

static int drv_bbh_tx_wan_queue_cfg_get(uint8_t bbh_id, pd_queue_cfg_t *wan_queue_cfg)
{
    uint8_t i;
    bdmf_error_t rc = BDMF_ERR_OK;

#if !defined(DUAL_ISSUE)
    for (i = 0; i < TX_QEUEU_PAIRS; i++)
    {
        rc = rc ? rc : drv_bbh_tx_common_configurations_q2rnr_get_wan(bbh_id, i, &wan_queue_cfg->queue_to_rnr[i].q0, &wan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdbase_get(bbh_id, i, &wan_queue_cfg->pd_fifo_base[i].base0, &wan_queue_cfg->pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdsize_get(bbh_id, i, &wan_queue_cfg->pd_fifo_size[i].size0, &wan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdwkuph_get(bbh_id, i, &wan_queue_cfg->pd_wkup_threshold[i].threshold0, &wan_queue_cfg->pd_wkup_threshold[i].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(bbh_id, i, &wan_queue_cfg->pd_bytes_threshold[i].threshold0, &wan_queue_cfg->pd_bytes_threshold[i].threshold1);
    }
#else
    rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdsize_get(bbh_id, &wan_queue_cfg->pd_fifo_size[0].size0, &wan_queue_cfg->pd_fifo_size[0].size1);
    rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdwkuph_get(bbh_id, &wan_queue_cfg->pd_wkup_threshold[0].threshold0, &wan_queue_cfg->pd_wkup_threshold[0].threshold1);
    rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(bbh_id, &wan_queue_cfg->pd_bytes_threshold[0].threshold0, &wan_queue_cfg->pd_bytes_threshold[0].threshold1);
    for (i = 0; i < TX_QEUEU_PAIRS; i++)
    {
        rc = rc ? rc : drv_bbh_tx_common_configurations_q2rnr_get_wan(bbh_id, i, &wan_queue_cfg->queue_to_rnr[i].q0, &wan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_qprof_get(bbh_id, i, &wan_queue_cfg->queue_profile[i].q0, &wan_queue_cfg->queue_profile[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_qmq_get(bbh_id, i, &wan_queue_cfg->qm_q[i].q0, &wan_queue_cfg->qm_q[i].q1);
    }
#endif
    return rc;
}

static int drv_bbh_tx_lan_queue_cfg_set(uint8_t bbh_id, pd_queue_cfg_t *lan_queue_cfg)
{
    bdmf_error_t rc = 0;
#ifndef XRDP_BBH_PER_LAN_PORT
    int i;
#endif

    /* CHECK : how many queues to set for LAN (q2rnr)? */
#if !defined(DUAL_ISSUE)
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdbase_set(bbh_id, lan_queue_cfg->pd_fifo_base[0].base0,
        lan_queue_cfg->pd_fifo_base[0].base1);
#else
    rc = ag_drv_bbh_tx_lan_configurations_q2rnr_set(bbh_id,  lan_queue_cfg->queue_to_rnr[0].q0, lan_queue_cfg->queue_to_rnr[0].q1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_qprof_set(bbh_id,  lan_queue_cfg->queue_profile[0].q0, lan_queue_cfg->queue_profile[0].q1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_qmq_set(bbh_id,  lan_queue_cfg->qm_q[0].q0, lan_queue_cfg->qm_q[0].q1);
#endif
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdsize_set(bbh_id, lan_queue_cfg->pd_fifo_size[0].size0,
        lan_queue_cfg->pd_fifo_size[0].size1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdwkuph_set(bbh_id, lan_queue_cfg->pd_wkup_threshold[0].threshold0,
        lan_queue_cfg->pd_wkup_threshold[0].threshold1);

    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pd_byte_th_set(bbh_id,
        lan_queue_cfg->pd_bytes_threshold[0].threshold0, lan_queue_cfg->pd_bytes_threshold[0].threshold1);
#ifndef XRDP_BBH_PER_LAN_PORT
    for (i = 0; i < LAN_QUEUE_PAIRS; i++)
    {
        rc = rc ? rc : drv_bbh_tx_common_configurations_q2rnr_set_lan(bbh_id, i, lan_queue_cfg->queue_to_rnr[i].q0, lan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_gtxthresh_set(bbh_id, i, LAN_TX_THRESHOLD);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_febase_set(bbh_id, i, lan_queue_cfg->fe_fifo_base[i].base0, lan_queue_cfg->fe_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_fesize_set(bbh_id, i, lan_queue_cfg->fe_fifo_size[i].size0, lan_queue_cfg->fe_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_fepdbase_set(bbh_id, i, lan_queue_cfg->fe_pd_fifo_base[i].base0, lan_queue_cfg->fe_pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_fepdsize_set(bbh_id, i, lan_queue_cfg->fe_pd_fifo_size[i].size0, lan_queue_cfg->fe_pd_fifo_size[i].size1);

#if !defined(DUAL_ISSUE)
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdbase_set(bbh_id, i, lan_queue_cfg->pd_fifo_base[i].base0, lan_queue_cfg->pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdsize_set(bbh_id, i, lan_queue_cfg->pd_fifo_size[i].size0, lan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdwkuph_set(bbh_id, i, lan_queue_cfg->pd_wkup_threshold[i].threshold0, lan_queue_cfg->pd_wkup_threshold[i].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(bbh_id, i, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE);
#else
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_gpr_set(bbh_id , 3);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_qprof_set(bbh_id, i, lan_queue_cfg->queue_profile[i].q0, lan_queue_cfg->queue_profile[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_qmq_set(bbh_id, i, lan_queue_cfg->qm_q[i].q0, lan_queue_cfg->qm_q[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdsize_set(bbh_id, lan_queue_cfg->pd_fifo_size[0].size0, lan_queue_cfg->pd_fifo_size[0].size1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdwkuph_set(bbh_id, lan_queue_cfg->pd_wkup_threshold[0].threshold0, lan_queue_cfg->pd_wkup_threshold[0].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(bbh_id, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE);

#endif
    }
#else /* XRDP_BBH_PER_LAN_PORT */
    rc = rc ? rc : drv_bbh_tx_common_configurations_q2rnr_set_lan(bbh_id, 0, lan_queue_cfg->queue_to_rnr[0].q0,
            lan_queue_cfg->queue_to_rnr[0].q1);
    rc = rc ? rc : drv_bbh_tx_common_configurations_q2rnr_set_lan(bbh_id, 1, lan_queue_cfg->queue_to_rnr[1].q0,
            lan_queue_cfg->queue_to_rnr[1].q1);
#endif
    return rc;
}

static int drv_bbh_tx_lan_queue_cfg_get(uint8_t bbh_id, pd_queue_cfg_t *lan_queue_cfg)
{
    bdmf_error_t rc;

#ifdef XRDP_BBH_PER_LAN_PORT
    rc = drv_bbh_tx_common_configurations_q2rnr_get_lan(bbh_id, 0, &lan_queue_cfg->queue_to_rnr[0].q0,
        &lan_queue_cfg->queue_to_rnr[0].q1);
    rc = rc ? rc : drv_bbh_tx_common_configurations_q2rnr_get_lan(bbh_id, 1, &lan_queue_cfg->queue_to_rnr[1].q0,
        &lan_queue_cfg->queue_to_rnr[1].q1);

    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdbase_get(bbh_id, &lan_queue_cfg->pd_fifo_base[0].base0,
        &lan_queue_cfg->pd_fifo_base[0].base1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdsize_get(bbh_id, &lan_queue_cfg->pd_fifo_size[0].size0,
        &lan_queue_cfg->pd_fifo_size[0].size1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pd_byte_th_get(bbh_id,
        &lan_queue_cfg->pd_bytes_threshold[0].threshold0, &lan_queue_cfg->pd_bytes_threshold[0].threshold1);
#else
    uint8_t i;
    for (i = 0; i < TX_LAN_QUEUE_PAIRS; i++)
    {
#if !defined(DUAL_ISSUE)
        rc = drv_bbh_tx_common_configurations_q2rnr_get_lan(bbh_id, i, &lan_queue_cfg->queue_to_rnr[i].q0, &lan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdbase_get(bbh_id, i, &lan_queue_cfg->pd_fifo_base[i].base0, &lan_queue_cfg->pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdsize_get(bbh_id, i, &lan_queue_cfg->pd_fifo_size[i].size0, &lan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pd_byte_th_get(bbh_id, i, &lan_queue_cfg->pd_bytes_threshold[i].threshold0, &lan_queue_cfg->pd_bytes_threshold[i].threshold1);
#else
        rc = drv_bbh_tx_common_configurations_q2rnr_get_lan(bbh_id, i, &lan_queue_cfg->queue_to_rnr[i].q0, &lan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdsize_get(bbh_id, &lan_queue_cfg->pd_fifo_size[i].size0, &lan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pd_byte_th_get(bbh_id, &lan_queue_cfg->pd_bytes_threshold[i].threshold0, &lan_queue_cfg->pd_bytes_threshold[i].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_qprof_get(bbh_id, i, &lan_queue_cfg->queue_profile[i].q0, &lan_queue_cfg->queue_profile[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_qmq_get(bbh_id, i, &lan_queue_cfg->qm_q[i].q0, &lan_queue_cfg->qm_q[i].q1);

#endif
    }
#endif
   return rc;
}

int drv_bbh_tx_configuration_set(uint8_t bbh_id, bbh_tx_config *config)
{
    bdmf_error_t rc;

    rc = ag_drv_bbh_tx_mac_type_set(bbh_id, config->mac_type);
    rc = rc ? rc : ag_drv_bbh_tx_cfg_src_id_set(bbh_id, &config->src_id);
    rc = rc ? rc : ag_drv_bbh_tx_rnr_src_id_set(bbh_id, config->rnr_cfg[0].rnr_src_id,  config->rnr_cfg[1].rnr_src_id);
    rc = rc ? rc : ag_drv_bbh_tx_bbh_dma_cfg_set(bbh_id, config->dma_cfg);
    rc = rc ? rc : ag_drv_bbh_tx_bbh_sdma_cfg_set(bbh_id, config->sdma_cfg);
    rc = rc ? rc : ag_drv_bbh_tx_bbh_ddr_cfg_set(bbh_id, config->ddr_cfg);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_ddrtmbasel_set(bbh_id, 0, &config->base_addr_low);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_ddrtmbaseh_set(bbh_id, 0, &config->base_addr_high);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(bbh_id, 0, config->rnr_cfg[0].tcont_addr,
        config->rnr_cfg[0].skb_addr);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(bbh_id, 0, config->rnr_cfg[0].ptr_addr,
        config->rnr_cfg[0].task_number);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(bbh_id, 1, config->rnr_cfg[1].tcont_addr,
        config->rnr_cfg[1].skb_addr);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(bbh_id, 1, config->rnr_cfg[1].ptr_addr,
        config->rnr_cfg[1].task_number);
#ifndef XRDP_BBH_PER_LAN_PORT
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_perqtask_set(bbh_id, &config->per_q_task);
#endif

    if(IS_WAN_TX_PORT(bbh_id))
    {
        rc = rc ? rc : drv_bbh_tx_wan_queue_cfg_set(bbh_id, &config->wan_queue_cfg);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_set(bbh_id,
            config->wan_queue_cfg.pd_bytes_threshold_en);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdempty_set(bbh_id, config->wan_queue_cfg.pd_empty_threshold);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(bbh_id, 0, config->sts_rnr_cfg[0].tcont_addr);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(bbh_id, 0, config->sts_rnr_cfg[0].ptr_addr,
            config->sts_rnr_cfg[0].task_number);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(bbh_id, 1, config->sts_rnr_cfg[1].tcont_addr);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(bbh_id, 1, config->sts_rnr_cfg[1].ptr_addr,
            config->sts_rnr_cfg[1].task_number);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(bbh_id, 0, config->msg_rnr_cfg[0].tcont_addr);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(bbh_id, 0, config->msg_rnr_cfg[0].ptr_addr,
            config->msg_rnr_cfg[0].task_number);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(bbh_id, 1, config->msg_rnr_cfg[1].tcont_addr);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(bbh_id, 1, config->msg_rnr_cfg[1].ptr_addr,
            config->msg_rnr_cfg[1].task_number);
    }
    else
    {
        rc = rc ? rc : drv_bbh_tx_lan_queue_cfg_set(bbh_id, &config->lan_queue_cfg);
        rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_set(bbh_id,
            config->lan_queue_cfg.pd_bytes_threshold_en);
        rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdempty_set(bbh_id, config->lan_queue_cfg.pd_empty_threshold);
    }
    return rc;
}

int drv_bbh_tx_configuration_get(uint8_t bbh_id, bbh_tx_config *config)
{
    bdmf_error_t rc;

    rc = ag_drv_bbh_tx_mac_type_get(bbh_id, (uint8_t *)&config->mac_type);
    rc = rc ? rc : ag_drv_bbh_tx_cfg_src_id_get(bbh_id, &config->src_id);
    rc = rc ? rc : ag_drv_bbh_tx_rnr_src_id_get(bbh_id, &config->rnr_cfg[0].rnr_src_id,
        &config->rnr_cfg[1].rnr_src_id);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_ddrtmbasel_get(bbh_id, 0, &config->base_addr_low);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_ddrtmbaseh_get(bbh_id, 0, &config->base_addr_high);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_1_get(bbh_id, 0, &config->rnr_cfg[0].tcont_addr,
        &config->rnr_cfg[0].skb_addr);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_2_get(bbh_id, 0, &config->rnr_cfg[0].ptr_addr,
        &config->rnr_cfg[0].task_number);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_1_get(bbh_id, 1, &config->rnr_cfg[1].tcont_addr,
        &config->rnr_cfg[1].skb_addr);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_rnrcfg_2_get(bbh_id, 1, &config->rnr_cfg[1].ptr_addr,
        &config->rnr_cfg[1].task_number);

    if(IS_WAN_TX_PORT(bbh_id))
    {
        rc = rc ? rc : drv_bbh_tx_wan_queue_cfg_get(bbh_id, &config->wan_queue_cfg);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_get(bbh_id,
            &config->wan_queue_cfg.pd_bytes_threshold_en);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdempty_get(bbh_id, &config->wan_queue_cfg.pd_empty_threshold);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get(bbh_id, 0, &config->sts_rnr_cfg[0].tcont_addr);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get(bbh_id, 0, &config->sts_rnr_cfg[0].ptr_addr,
            &config->sts_rnr_cfg[0].task_number);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get(bbh_id, 1, &config->sts_rnr_cfg[1].tcont_addr);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get(bbh_id, 1, &config->sts_rnr_cfg[1].ptr_addr,
            &config->sts_rnr_cfg[1].task_number);
    }
    else
    {
        rc = rc ? rc : drv_bbh_tx_lan_queue_cfg_get(bbh_id, &config->lan_queue_cfg);
        rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_get(bbh_id,
            &config->lan_queue_cfg.pd_bytes_threshold_en);
        rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdempty_get(bbh_id, &config->lan_queue_cfg.pd_empty_threshold);
    }
    return rc;
}

int drv_bbh_tx_read_indirect_debug_register(uint8_t bbh_id, bbh_tx_indirect_debug_registers_t debug_sel, uint32_t addr, uint32_t *data)
{
    bdmf_error_t rc;
    bbh_tx_debug_counters_swrden swrden = {};
    bdmf_boolean *swrden_p = (bdmf_boolean *)&swrden;
    swrden_p[debug_sel] = 1;

    /* set read address, vector and read the data */
    rc = ag_drv_bbh_tx_debug_counters_swrdaddr_set(bbh_id, addr);
    rc = rc ? rc :ag_drv_bbh_tx_debug_counters_swrden_set(bbh_id, &swrden);
    rc = rc ? rc :ag_drv_bbh_tx_debug_counters_swrddata_get(bbh_id, data);
    return rc;
}

#ifdef USE_BDMF_SHELL

struct bdmfmon_enum_val bbh_tx_indirect_debug_enum_table[] = {
    {"PDVSEL", PDVSEL},
    {"PDEMPTYSEL", PDEMPTYSEL},
    {"PDFULLSEL", PDFULLSEL},
    {"PDBEMPTYSEL", PDBEMPTYSEL},
    {"PDFFWKPSEL", PDFFWKPSEL},
    {"FBNSEL", FBNSEL},
    {"FBNVSEL", FBNVSEL},
    {"FBNEMPTYSEL", FBNEMPTYSEL},
    {"FBNFULLSEL", FBNFULLSEL},
    {"GETNEXTSEL", GETNEXTSEL},
    {"GETNEXTVSEL", GETNEXTVSEL},
    {"GETNEXTEMPTYSEL", GETNEXTEMPTYSEL},
    {"GETNEXTFULLSEL", GETNEXTFULLSEL},
    {"GPNCNTXTSEL", GPNCNTXTSEL},
    {"BPMSEL", BPMSEL},
    {"BPMFSEL", BPMFSEL},
    {"SBPMSEL", SBPMSEL},
    {"SBPMFSEL", SBPMFSEL},
    {"STSSEL", STSSEL},
    {"STSVSEL", STSVSEL},
    {"STSEMPTYSEL", STSEMPTYSEL},
    {"STSFULLSEL", STSFULLSEL},
    {"STSBEMPTYSEL", STSBEMPTYSEL},
    {"STSFFWKPSEL", STSFFWKPSEL},
    {"MSGSEL", MSGSEL},
    {"MSGVSEL", MSGVSEL},
    {"EPNREQSEL", EPNREQSEL},
    {"DATASEL", DATASEL},
    {"REORDERSEL", REORDERSEL},
    {"TSINFOSEL", TSINFOSEL},
    {"MACTXSEL", MACTXSEL},
    {NULL, 0},
};
/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
static int drv_bbh_tx_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_tx_config config = {};
    queue_to_rnr_t wan_queue_to_rnr[TX_QEUEU_PAIRS] = {};
    pd_fifo_base_t wan_pd_fifo_base[TX_QEUEU_PAIRS] = {};
    pd_fifo_size_t wan_pd_fifo_size[TX_QEUEU_PAIRS];
    pd_wkup_threshold_t wan_pd_wkup_threshold[TX_QEUEU_PAIRS] = {};
    pd_bytes_threshold_t wan_pd_bytes_threshold[TX_QEUEU_PAIRS];
    queue_to_rnr_t lan_queue_to_rnr[TX_LAN_QUEUE_PAIRS] = {};
    pd_fifo_base_t lan_pd_fifo_base[TX_LAN_QUEUE_PAIRS] = {};
    pd_fifo_size_t lan_pd_fifo_size[TX_LAN_QUEUE_PAIRS];
    pd_wkup_threshold_t lan_pd_wkup_threshold[TX_LAN_QUEUE_PAIRS] = {};
    pd_bytes_threshold_t lan_pd_bytes_threshold[TX_LAN_QUEUE_PAIRS];
    bbh_id_e bbh_id = parm[0].value.unumber;
    bdmf_error_t rc;
    bbh_tx_bbh_dma_cfg dma_cfg = {};
    bbh_tx_bbh_sdma_cfg sdma_cfg = {};
    bbh_tx_bbh_ddr_cfg ddr_cfg = {};
    bdmf_boolean epon_urgent_flag;

#if !defined(DUAL_ISSUE)
    bdmf_boolean stop_len_err, cmp_width, consider_full, add_crc;
#else
    bbh_tx_wan_configurations_epncfg wan_configurations_epncfg = {};
#endif

    uint8_t i, max_req;

    config.wan_queue_cfg.queue_to_rnr = wan_queue_to_rnr;
    config.wan_queue_cfg.pd_fifo_base = wan_pd_fifo_base;
    config.wan_queue_cfg.pd_fifo_size = wan_pd_fifo_size;
    config.wan_queue_cfg.pd_wkup_threshold = wan_pd_wkup_threshold;
    config.wan_queue_cfg.pd_bytes_threshold = wan_pd_bytes_threshold;
    config.lan_queue_cfg.queue_to_rnr = lan_queue_to_rnr;
    config.lan_queue_cfg.pd_fifo_base = lan_pd_fifo_base;
    config.lan_queue_cfg.pd_fifo_size = lan_pd_fifo_size;
    config.lan_queue_cfg.pd_wkup_threshold = lan_pd_wkup_threshold;
    config.lan_queue_cfg.pd_bytes_threshold = lan_pd_bytes_threshold;

    rc = drv_bbh_tx_configuration_get(bbh_id, &config);
    if (!rc)
    {
        bdmf_session_print(session, "BBH TX configurations for port %s:\n\r", bbh_id_tx_enum_table[bbh_id].name);
        bdmf_session_print(session, "=====================================\n\r");
        bdmf_session_print(session, "fpm source id:        %d\n\r", config.src_id.fpmsrc);

        bdmf_session_print(session, "runner 0 source id:   %d\n\r", config.rnr_cfg[0].rnr_src_id);
        bdmf_session_print(session, "runner 0 tcont addr:  0x%x\n\r", config.rnr_cfg[0].tcont_addr);
        bdmf_session_print(session, "runner 0 skb addr:    0x%x\n\r", config.rnr_cfg[0].skb_addr);
        bdmf_session_print(session, "runner 0 ptr addr:    0x%x\n\r", config.rnr_cfg[0].ptr_addr);
        bdmf_session_print(session, "runner 0 task num:    %d\n\r", config.rnr_cfg[0].task_number);

        bdmf_session_print(session, "runner 1 source id:   %d\n\r", config.rnr_cfg[1].rnr_src_id);
        bdmf_session_print(session, "runner 1 tcont addr:  0x%x\n\r", config.rnr_cfg[1].tcont_addr);
        bdmf_session_print(session, "runner 1 skb addr:    0x%x\n\r", config.rnr_cfg[1].skb_addr);
        bdmf_session_print(session, "runner 1 ptr addr:    0x%x\n\r", config.rnr_cfg[1].ptr_addr);
        bdmf_session_print(session, "runner 1 task num:    %d\n\r", config.rnr_cfg[1].task_number);

        bdmf_session_print(session, "ddr tm base 0 low:        0x%x\n\r", config.base_addr_low.addr[0]);
        bdmf_session_print(session, "ddr tm base 0 high:       0x%x\n\r", config.base_addr_high.addr[0]);
        bdmf_session_print(session, "ddr tm base 1 low:        0x%x\n\r", config.base_addr_low.addr[1]);
        bdmf_session_print(session, "ddr tm base 1 high:       0x%x\n\r", config.base_addr_high.addr[1]);

        if(IS_WAN_TX_PORT(bbh_id))
        {
            bdmf_session_print(session, "pd empty threshold:   %d\n\r", config.wan_queue_cfg.pd_empty_threshold);
            bdmf_session_print(session, "\npd bytes threshold en:%d\n\r", config.wan_queue_cfg.pd_bytes_threshold_en);

            bdmf_session_print(session, "queue to runner:\n\r");
            bdmf_session_print(session, "================\n\r");
            for (i = 0; i < TX_QEUEU_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %d, queue %d: %d\n\r", 2*i, config.wan_queue_cfg.queue_to_rnr[i].q0, 2*i+1, config.wan_queue_cfg.queue_to_rnr[i].q1);
            }
            bdmf_session_print(session, "pd fifo base:\n\r");
            bdmf_session_print(session, "=============\n\r");
            for (i = 0; i < TX_QEUEU_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.wan_queue_cfg.pd_fifo_base[i].base0, 2*i+1, config.wan_queue_cfg.pd_fifo_base[i].base1);
            }
            bdmf_session_print(session, "\npd fifo size:\n\r");
            bdmf_session_print(session, "=============\n\r");
            for (i = 0; i < TX_QEUEU_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.wan_queue_cfg.pd_fifo_size[i].size0, 2*i+1, config.wan_queue_cfg.pd_fifo_size[i].size1);
            }
            bdmf_session_print(session, "\npd wkup threshold:\n\r");
            bdmf_session_print(session, "==================\n\r");
            for (i = 0; i < TX_QEUEU_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x ", 2*i, config.wan_queue_cfg.pd_wkup_threshold[i].threshold0, 2*i+1, config.wan_queue_cfg.pd_wkup_threshold[i].threshold1);
            }

            bdmf_session_print(session, "\npd byte threshold:\n\r");
            bdmf_session_print(session, "==================\n\r");
            for (i = 0; i < TX_QEUEU_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.wan_queue_cfg.pd_bytes_threshold[i].threshold0, 2*i+1, config.wan_queue_cfg.pd_bytes_threshold[i].threshold1);
            }

            if (config.mac_type == MAC_TYPE_EPON)
            {
                bdmf_session_print(session, "sts runner source id:     %d\n\r", config.src_id.stsrnrsrc);
                bdmf_session_print(session, "sts runner 0 tcont addr:  0x%x\n\r", config.sts_rnr_cfg[0].tcont_addr);
                bdmf_session_print(session, "sts runner 0 ptr addr:    0x%x\n\r", config.sts_rnr_cfg[0].ptr_addr);
                bdmf_session_print(session, "sts runner 0 task num:    %d\n\r", config.sts_rnr_cfg[0].task_number);
                bdmf_session_print(session, "sts runner 1 tcont addr:  0x%x\n\r", config.sts_rnr_cfg[1].tcont_addr);
                bdmf_session_print(session, "sts runner 1 ptr addr:    0x%x\n\r", config.sts_rnr_cfg[1].ptr_addr);
                bdmf_session_print(session, "sts runner 1 task num:    %d\n\r", config.sts_rnr_cfg[1].task_number);
#if !defined(DUAL_ISSUE)
                rc = ag_drv_bbh_tx_wan_configurations_epncfg_get(bbh_id, &stop_len_err, &cmp_width, &consider_full, &add_crc);
                if (!rc)
                {
                    bdmf_session_print(session, "stop length err flag:     %d\n\r", stop_len_err);
                    bdmf_session_print(session, "Compare width flag:       %d\n\r", cmp_width);
                    bdmf_session_print(session, "Consider full flag:       %d\n\r", consider_full);
                    bdmf_session_print(session, "Add CRC:                  %d\n\r", add_crc);
                }
#else
                rc = ag_drv_bbh_tx_wan_configurations_epncfg_get(bbh_id, &wan_configurations_epncfg);
                if (!rc)
                {
                    bdmf_session_print(session, "stop length err flag:     %d\n\r", wan_configurations_epncfg.stplenerr);
                    bdmf_session_print(session, "Compare width flag:       %d\n\r", wan_configurations_epncfg.cmp_width);
                    bdmf_session_print(session, "Consider full flag:       %d\n\r", wan_configurations_epncfg.considerfull);
                    bdmf_session_print(session, "Add CRC:                  %d\n\r", wan_configurations_epncfg.addcrc);
                    bdmf_session_print(session, "req full:                 %d\n\r", wan_configurations_epncfg.req_full);
                }
#endif
             }

        }
        else
        {
            bdmf_session_print(session, "pd empty threshold:   %d\n\r", config.lan_queue_cfg.pd_empty_threshold);
            bdmf_session_print(session, "pd bytes threshold en:%d\n\r", config.lan_queue_cfg.pd_bytes_threshold_en);

            bdmf_session_print(session, "pd fifo base:\n\r");
            bdmf_session_print(session, "=============\n\r");
            for (i = 0; i < TX_LAN_QUEUE_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.lan_queue_cfg.pd_fifo_base[i].base0, 2*i+1, config.lan_queue_cfg.pd_fifo_base[i].base1);
            }
            bdmf_session_print(session, "\npd fifo size:\n\r");
            bdmf_session_print(session, "=============\n\r");
            for (i = 0; i < TX_LAN_QUEUE_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.lan_queue_cfg.pd_fifo_size[i].size0, 2*i+1, config.lan_queue_cfg.pd_fifo_size[i].size1);
            }

            bdmf_session_print(session, "\npd byte threshold:\n\r");
            bdmf_session_print(session, "==================\n\r");
            for (i = 0; i < TX_LAN_QUEUE_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.lan_queue_cfg.pd_bytes_threshold[i].threshold0, 2*i+1, config.lan_queue_cfg.pd_bytes_threshold[i].threshold1);
            }
            bdmf_session_print(session, "\nq2rnr:\n\r");
            bdmf_session_print(session, "==================\n\r");
            for (i = 0; i < TX_LAN_QUEUE_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.lan_queue_cfg.queue_to_rnr[i].q0, 2*i+1, config.lan_queue_cfg.queue_to_rnr[i].q1);
            }
       }
    }
    rc = ag_drv_bbh_tx_bbh_dma_cfg_get(bbh_id, &dma_cfg);
    if (!rc)
    {
        bdmf_session_print(session, "\nDMA configurations\n\r");
        bdmf_session_print(session, "==================\n\r");
        bdmf_session_print(session, "descbase:          %d\n\r", dma_cfg.descbase);
        bdmf_session_print(session, "descsize:          %d\n\r", dma_cfg.descsize);
        bdmf_session_print(session, "source id          %d\n\r", dma_cfg.dmasrc);

        if(!ag_drv_bbh_tx_dma_max_otf_read_request_get(bbh_id, &max_req))
            bdmf_session_print(session, "max read requests: %d\n\r", max_req);
        if(!ag_drv_bbh_tx_dma_epon_urgent_get(bbh_id, &epon_urgent_flag))
            bdmf_session_print(session, "epon urgent:       %d\n\r", epon_urgent_flag);
    }
    rc = ag_drv_bbh_tx_bbh_sdma_cfg_get(bbh_id, &sdma_cfg);
    if (!rc)
    {
        bdmf_session_print(session, "SDMA configurations\n\r");
        bdmf_session_print(session, "==================\n\r");
        bdmf_session_print(session, "descbase:          %d\n\r", sdma_cfg.descbase);
        bdmf_session_print(session, "descsize:          %d\n\r", sdma_cfg.descsize);
        bdmf_session_print(session, "source id:         %d\n\r", sdma_cfg.sdmasrc);

        if(!ag_drv_bbh_tx_sdma_max_otf_read_request_get(bbh_id, &max_req))
            bdmf_session_print(session, "max read requests: %d\n\r", max_req);
        if(!ag_drv_bbh_tx_sdma_epon_urgent_get(bbh_id, &epon_urgent_flag))
            bdmf_session_print(session, "epon urgent:       %d\n\r", epon_urgent_flag);
    }
    rc = ag_drv_bbh_tx_bbh_ddr_cfg_get(bbh_id, &ddr_cfg);
    if (!rc)
    {
        bdmf_session_print(session, "DDR configurations\n\r");
        bdmf_session_print(session, "==================\n\r");
        bdmf_session_print(session, "buffer size:                     %d\n\r", ddr_cfg.bufsize);
        bdmf_session_print(session, "psram po byte res:               %d\n\r", ddr_cfg.byteresul);
        bdmf_session_print(session, "tx packet offset (8 byte res):   %d\n\r", ddr_cfg.ddrtxoffset);
        bdmf_session_print(session, "header number 0 size:            %d\n\r", ddr_cfg.hnsize0);
        bdmf_session_print(session, "header number 1 size:            %d\n\r", ddr_cfg.hnsize1);
    }
    return rc;
}

static int drv_bbh_tx_cli_debug_total_get(bdmf_session_handle session, bbh_id_e bbh_id, bdmf_boolean unified)
{
    bdmf_error_t rc;
    bbh_tx_debug_counters debug_counters = {};

    rc = ag_drv_bbh_tx_debug_counters_get(bbh_id, &debug_counters);
    if (!rc)
    {
        if (unified)
        {
            bdmf_session_print(session, "\nBBH TX debug unified counters for all LAN ports:\n\r");
            bdmf_session_print(session, "================================================\n\r");
        }
        else
        {
            bdmf_session_print(session, "BBH TX debug counters for port %s:\n\r", bbh_id_tx_enum_table[bbh_id].name);
            bdmf_session_print(session, "=====================================\n\r");
        }
        bdmf_session_print(session, "srampd:        %d\n\r", debug_counters.srampd);
        bdmf_session_print(session, "ddrpd:         %d\n\r", debug_counters.ddrpd);
        bdmf_session_print(session, "pddrop:        %d\n\r", debug_counters.pddrop);
        bdmf_session_print(session, "aggrlenerr:    %d\n\r", debug_counters.aggrlenerr);
        bdmf_session_print(session, "ddrpkt:        %d\n\r", debug_counters.ddrpkt);
        bdmf_session_print(session, "srampkt:       %d\n\r", debug_counters.srampkt);
        bdmf_session_print(session, "msgcnt:        %d\n\r", debug_counters.msgcnt);
        bdmf_session_print(session, "msgdrop:       %d\n\r", debug_counters.msgdrop);
        bdmf_session_print(session, "stscnt:        %d\n\r", debug_counters.stscnt);
        bdmf_session_print(session, "stsdrop:       %d\n\r", debug_counters.stsdrop);
        bdmf_session_print(session, "lenerr:        %d\n\r", debug_counters.lenerr);
        bdmf_session_print(session, "flshpkts:      %d\n\r", debug_counters.flshpkts);
    }
    return rc;
}

static int drv_bbh_tx_cli_indirect_register_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_id_e bbh_id = parm[0].value.unumber;
    bbh_tx_indirect_debug_registers_t debug_reg = parm[1].value.unumber;
    uint32_t start_offset = parm[2].value.unumber;
    uint32_t end_offset = parm[3].value.unumber;
    uint32_t data;
    uint32_t i;
    bdmf_error_t rc;
    bdmf_session_print(session, "reading debug indirect register %s (%d-%d)\n\r", bbh_tx_indirect_debug_enum_table[debug_reg].name, start_offset, end_offset);

    for (i = start_offset; i <= end_offset; i++)
    {
        rc = drv_bbh_tx_read_indirect_debug_register(bbh_id, debug_reg, i, &data);
        if (rc)
            return rc;
        bdmf_session_print(session, "(%d) : %d\n\r", i, data);
    }
    return rc;
}

typedef struct {
    uint8_t egress_cnt[TX_QEUEU_PAIRS*2];
    uint8_t ingress_cnt[TX_QEUEU_PAIRS*2];
    uint8_t fifo_size [TX_QEUEU_PAIRS*2];
    uint8_t almostFull[8];
    uint8_t full[8];
    uint8_t empty[8];
} bbh_tx_occupancy_info_t;

static void __drv_bbh_tx_cli_occupancy_dump(bdmf_session_handle session, int q_id, bbh_tx_occupancy_info_t *o)
{
    int almost_full, full, empty;

    almost_full = o->almostFull[q_id/8] & (1 << (q_id % 8)) ? 1 : 0;
    full = o->full[q_id/8] & (1 << (q_id % 8)) ? 1 : 0;
    empty = o->empty[q_id/8] & (1 << (q_id % 8)) ? 1 : 0;

    /* As there can be conflict in values, check all of them */
    if (o->ingress_cnt[q_id] != o->egress_cnt[q_id] || !empty || almost_full || full)
    {
        bdmf_session_print(session, "Unexpected mismatch in occupancy counters:\n\t");
    }

    bdmf_session_print(session, "BBH_TX queue(%03d): ingress=%03d, egress=%03d, "
        "occupancy=%03d, fifo size=%03d, almost_full=%d, full=%d, empty=%d\n",
        q_id, o->ingress_cnt[q_id], o->egress_cnt[q_id],
        o->ingress_cnt[q_id] - o->egress_cnt[q_id], o->fifo_size[q_id], almost_full, full, empty);
}

static int drv_bbh_tx_cli_occupancy_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t number_of_lans;
    uint32_t ghost_ingress;
    uint32_t ghost_egress;
    bbh_id_e bbh_id = parm[0].value.unumber;
    bdmf_error_t rc;
    uint32_t i;
    bbh_tx_config config = {};
    queue_to_rnr_t wan_queue_to_rnr[TX_QEUEU_PAIRS];
    pd_fifo_base_t wan_pd_fifo_base[TX_QEUEU_PAIRS];
    pd_fifo_size_t wan_pd_fifo_size[TX_QEUEU_PAIRS];
    pd_wkup_threshold_t wan_pd_wkup_threshold[TX_QEUEU_PAIRS];
    pd_bytes_threshold_t wan_pd_bytes_threshold[TX_QEUEU_PAIRS];
    queue_to_rnr_t lan_queue_to_rnr[TX_LAN_QUEUE_PAIRS];
    pd_fifo_base_t lan_pd_fifo_base[TX_LAN_QUEUE_PAIRS];
    pd_fifo_size_t lan_pd_fifo_size[TX_LAN_QUEUE_PAIRS];
    pd_wkup_threshold_t lan_pd_wkup_threshold[TX_LAN_QUEUE_PAIRS];
    pd_bytes_threshold_t lan_pd_bytes_threshold[TX_LAN_QUEUE_PAIRS];
    bbh_tx_occupancy_info_t o;

    config.wan_queue_cfg.queue_to_rnr = wan_queue_to_rnr;
    config.wan_queue_cfg.pd_fifo_base = wan_pd_fifo_base;
    config.wan_queue_cfg.pd_fifo_size = wan_pd_fifo_size;
    config.wan_queue_cfg.pd_wkup_threshold = wan_pd_wkup_threshold;
    config.wan_queue_cfg.pd_bytes_threshold = wan_pd_bytes_threshold;
    config.lan_queue_cfg.queue_to_rnr = lan_queue_to_rnr;
    config.lan_queue_cfg.pd_fifo_base = lan_pd_fifo_base;
    config.lan_queue_cfg.pd_fifo_size = lan_pd_fifo_size;
    config.lan_queue_cfg.pd_wkup_threshold = lan_pd_wkup_threshold;
    config.lan_queue_cfg.pd_bytes_threshold = lan_pd_bytes_threshold;

    bdmf_session_print(session, "occupancy_get\n\r");
    rc =  drv_bbh_tx_configuration_get(bbh_id, &config);

    /* read the pd_full_for_wakeup_array_sel, full, empty */
    rc = rc ? rc : drv_bbh_tx_read_indirect_debug_register(bbh_id, PDFFWKPSEL, 0, (uint32_t *)&(o.almostFull[0]));
    rc = rc ? rc : drv_bbh_tx_read_indirect_debug_register(bbh_id, PDFFWKPSEL, 1, (uint32_t *)&(o.almostFull[4]));
    rc = rc ? rc : drv_bbh_tx_read_indirect_debug_register(bbh_id, PDFULLSEL, 0, (uint32_t*)&(o.full[0]));
    rc = rc ? rc : drv_bbh_tx_read_indirect_debug_register(bbh_id, PDFULLSEL, 1, (uint32_t*)&(o.full[4]));
    rc = rc ? rc : drv_bbh_tx_read_indirect_debug_register(bbh_id, PDEMPTYSEL, 0, (uint32_t*)&(o.empty[0]));
    rc = rc ? rc : drv_bbh_tx_read_indirect_debug_register(bbh_id, PDEMPTYSEL, 1, (uint32_t*)&(o.empty[4]));


    if (rc)
    {
        bdmf_session_print(session, "Couldn't get configuration\n\r");
        return rc;
    }

    /* read the egress ingress */
    if(IS_WAN_TX_PORT(bbh_id))
    {
        /* WAN */
        if (config.mac_type == MAC_TYPE_EPON)
        {
#ifdef EPON
            for (i = 0; i < (TX_QEUEU_PAIRS * 2); i++)
            {
                RDD_BYTE_1_BITS_READ_G(o.egress_cnt[i], RDD_BBH_TX_EPON_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i);
                RDD_BYTE_1_BITS_READ_G(o.ingress_cnt[i], RDD_BBH_TX_EPON_INGRESS_COUNTER_TABLE_ADDRESS_ARR, i);
            }
#endif
        }
        else
        {
            for(i = 0; i < (TX_QEUEU_PAIRS * 2); i++)
            {
#if defined(BCM63158)
                if ((i >= RDD_US_CHANNEL_OFFSET_TCONT) && (i <= RDD_US_CHANNEL_OFFSET_TCONT_END))
                {
                    RDD_BYTE_1_BITS_READ_G(o.egress_cnt[i], RDD_US_TM_PON_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i - RDD_US_CHANNEL_OFFSET_TCONT);
                }
                else if ((i >= RDD_US_CHANNEL_OFFSET_DSL) && (i <= RDD_US_CHANNEL_OFFSET_DSL_END))
                {
                    RDD_BYTE_1_BITS_READ_G(o.egress_cnt[i], RDD_US_TM_DSL_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i - RDD_US_CHANNEL_OFFSET_DSL);
                }
                else if ((i >= RDD_US_CHANNEL_OFFSET_AE10) && (i <= RDD_US_CHANNEL_OFFSET_AE10_END))
                {
                    RDD_BYTE_1_BITS_READ_G(o.egress_cnt[i], RDD_US_TM_AE10_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i - RDD_US_CHANNEL_OFFSET_AE10);
                }
                else /* if ((i >= RDD_US_CHANNEL_OFFSET_AE2P5) && (i <= RDD_US_CHANNEL_OFFSET_AE2P5_END)) */
                {
                    RDD_BYTE_1_BITS_READ_G(o.egress_cnt[i], RDD_US_TM_AE2P5_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i - RDD_US_CHANNEL_OFFSET_AE2P5);
                }
#else
                RDD_BYTE_1_BITS_READ_G(o.egress_cnt[i], RDD_US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i);
#endif
                RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_READ_G(o.ingress_cnt[i], RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR,i);
            }
        }
        for(i = 0 ;i < (TX_QEUEU_PAIRS); i++)
        {
            o.fifo_size[i *2]       = config.wan_queue_cfg.pd_fifo_size[i].size0;
            o.fifo_size[(i *2) + 1] = config.wan_queue_cfg.pd_fifo_size[i].size1;
        }
        for(i = 0 ;i < (TX_QEUEU_PAIRS * 2); i++)
        {
            __drv_bbh_tx_cli_occupancy_dump(session, i, &o);
        }
        /* now calculate ghost */
        RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ_G(ghost_egress, RDD_BBH_TX_EGRESS_REPORT_COUNTER_TABLE_ADDRESS_ARR, 0);
        RDD_BYTE_1_BITS_READ_G(ghost_ingress, RDD_BBH_TX_INGRESS_COUNTER_TABLE_ADDRESS_ARR, 0);
        bdmf_session_print(session, "BBH_TX ghost reporting: ingress=%03d, egress=%03d, occupancy=%03d\n\r", ghost_ingress, ghost_egress, ghost_ingress - ghost_egress);
    }
    else
    {
       /* LAN */

#ifdef XRDP_BBH_PER_LAN_PORT
       number_of_lans = 1;
#else
       number_of_lans = RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_SIZE;
#endif

       for (i = 0; i < (number_of_lans); i++)
       {
#ifdef BCM6858
           RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ_G(o.egress_cnt[i], RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i);
#else
           RDD_BYTE_1_BITS_READ_G(o.egress_cnt[i], RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, i);
#endif
           RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_READ_G(o.ingress_cnt[i], RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR,i);
       }
       for (i = 0; i < (TX_LAN_QUEUE_PAIRS); i++)
       {
           o.fifo_size[i *2]       = config.lan_queue_cfg.pd_fifo_size[i].size0;
           o.fifo_size[(i *2) + 1] = config.lan_queue_cfg.pd_fifo_size[i].size1;
       }
       for (i = 0; i < (number_of_lans); i++)
       {
           __drv_bbh_tx_cli_occupancy_dump(session, i, &o);
       }
    }
    return rc;
}

static int drv_bbh_tx_cli_debug_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_id_e bbh_id = parm[0].value.unumber;
    bdmf_error_t rc;

#if defined(BCM63158)
    bbh_tx_debug_counters debug_counters = {};

    if (bbh_id < BBH_ID_PON)
    {
        rc = ag_drv_bbh_tx_debug_counters_unifiedpkt_get(0, bbh_id, &debug_counters.srampkt);
        rc = rc ? rc : ag_drv_bbh_tx_debug_counters_unifiedbyte_get(0, bbh_id, &debug_counters.srampd);
        if (!rc)
        {
            bdmf_session_print(session, "BBH TX debug counters for port %s:\n\r", bbh_id_enum_table[bbh_id].name);
            bdmf_session_print(session, "=====================================\n\r");
            bdmf_session_print(session, "packets:        %d\n\r", debug_counters.srampkt);
            bdmf_session_print(session, "bytes:          %d\n\r", debug_counters.srampd);
            drv_bbh_tx_cli_debug_total_get(session, 0, 1);
        }
        return rc;
    }
    else
        bbh_id -= BBH_ID_PON - BBH_TX_ID_PON;

#elif !defined(BCM6858)
    bbh_tx_debug_counters debug_counters = {};

    if (bbh_id != BBH_ID_PON)
    {
        rc = ag_drv_bbh_tx_debug_counters_unifiedpkt_get(0, bbh_id, &debug_counters.srampkt);
        rc = rc ? rc : ag_drv_bbh_tx_debug_counters_unifiedbyte_get(0, bbh_id, &debug_counters.srampd);
        if (!rc)
        {
            bdmf_session_print(session, "BBH TX debug counters for port %s:\n\r", bbh_id_tx_enum_table[bbh_id].name);
            bdmf_session_print(session, "=====================================\n\r");
            bdmf_session_print(session, "packets:        %d\n\r", debug_counters.srampkt);
            bdmf_session_print(session, "bytes:          %d\n\r", debug_counters.srampd);
            drv_bbh_tx_cli_debug_total_get(session, 0, 1);
        }
        return rc;
    }
    else
        bbh_id = BBH_TX_WAN_ID;
#endif
    rc = drv_bbh_tx_cli_debug_total_get(session, bbh_id, 0);

    return rc;
}

int drv_bbh_tx_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_id_e bbh_id = parm[0].value.unumber;
    bdmf_error_t rc;
    bbh_tx_debug_counters debug_counters = {};
    
    rc = ag_drv_bbh_tx_debug_counters_get(bbh_id, &debug_counters);
    if (!rc)
    {
        if (debug_counters.pddrop)
            bdmf_session_print(session, "\nError:pddrop:        %d\n\r", debug_counters.pddrop);
        if (debug_counters.aggrlenerr)
            bdmf_session_print(session, "\nError:aggrlenerr:    %d\n\r", debug_counters.aggrlenerr);
        if (debug_counters.msgdrop)
            bdmf_session_print(session, "\nError:msgdrop:       %d\n\r", debug_counters.msgdrop);
        if (debug_counters.stsdrop)
            bdmf_session_print(session, "\nError:stsdrop:       %d\n\r", debug_counters.stsdrop);
        if (debug_counters.lenerr) 
            bdmf_session_print(session, "\nError:lenerr:        %d\n\r", debug_counters.lenerr);
    }
    return rc;
}

int drv_bbh_tx_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val, bdmf_boolean keep_alive)
{
     bbh_tx_common_configurations_clk_gate_cntrl bbh_tx_ctrl;
     uint8_t block_id = 0;

     for (block_id = 0; block_id < RU_BLK(BBH_TX).addr_count; block_id++)
     {
         ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get(block_id, &bbh_tx_ctrl);
         bbh_tx_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
         bbh_tx_ctrl.timer_val = timer_val;
         bbh_tx_ctrl.keep_alive_en = keep_alive;
         ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(block_id, &bbh_tx_ctrl);
     }

     return 0;
}

#ifdef USE_BDMF_SHELL
static bdmfmon_handle_t bbh_dir;

void drv_bbh_tx_cli_init(bdmfmon_handle_t driver_dir)
{
    bbh_dir = ag_drv_bbh_tx_cli_init(driver_dir);

    BDMFMON_MAKE_CMD(bbh_dir, "cfg_get", "bbh tx configuration", drv_bbh_tx_cli_config_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_tx_enum_table, 0));
#if defined(BCM63158)
    BDMFMON_MAKE_CMD(bbh_dir, "debug_counter_get", "bbh tx counters", drv_bbh_tx_cli_debug_counters_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0));
#else
    BDMFMON_MAKE_CMD(bbh_dir, "debug_counter_get", "bbh tx counters", drv_bbh_tx_cli_debug_counters_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_tx_enum_table, 0));
#endif
    BDMFMON_MAKE_CMD(bbh_dir, "sanity_get", "bbh tx counters", drv_bbh_tx_cli_sanity_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_tx_enum_table, 0));
    BDMFMON_MAKE_CMD(bbh_dir, "occupancy_get", "bbh tx occupancy", drv_bbh_tx_cli_occupancy_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_tx_enum_table, 0));
    BDMFMON_MAKE_CMD(bbh_dir, "indirect_register_get", "read indirect debug registers", drv_bbh_tx_cli_indirect_register_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_tx_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("reg", "debug register", bbh_tx_indirect_debug_enum_table, 0),
        BDMFMON_MAKE_PARM("start_offset", "start offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("end_offset", "end offset", BDMFMON_PARM_NUMBER, 0));
}

void drv_bbh_tx_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (bbh_dir)
    {
        bdmfmon_token_destroy(bbh_dir);
        bbh_dir = NULL;
    }
}
#endif
/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
/*
    cfg_get
    ag:
        pf_cfg_get (0_31 & group0_1)
        error_pm_counters
        pm_counters
*/
#endif /* USE_BDMF_SHELL */

