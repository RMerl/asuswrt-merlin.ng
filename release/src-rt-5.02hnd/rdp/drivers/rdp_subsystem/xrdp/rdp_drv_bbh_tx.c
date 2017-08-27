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
#include "rdp_common.h"
#include "rdp_drv_bbh_tx.h"
#include "rdd_data_structures_auto.h"

int drv_bbh_tx_wan_queue_cfg_set(uint8_t bbh_id, pd_queue_cfg_t *wan_queue_cfg)
{
    uint8_t i;
    bdmf_error_t rc = BDMF_ERR_OK;

    for (i = 0; i < TX_QEUEU_PAIRS; i++)
    {
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_q2rnr_set(bbh_id, i, wan_queue_cfg->queue_to_rnr[i].q0, wan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdbase_set(bbh_id, i, wan_queue_cfg->pd_fifo_base[i].base0, wan_queue_cfg->pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdsize_set(bbh_id, i, wan_queue_cfg->pd_fifo_size[i].size0, wan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdwkuph_set(bbh_id, i, wan_queue_cfg->pd_wkup_threshold[i].threshold0, wan_queue_cfg->pd_wkup_threshold[i].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(bbh_id, i, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE);
    }
    return rc;
}

static int drv_bbh_tx_wan_queue_cfg_get(uint8_t bbh_id, pd_queue_cfg_t *wan_queue_cfg)
{
    uint8_t i;
    bdmf_error_t rc = BDMF_ERR_OK;

    for (i = 0; i < TX_QEUEU_PAIRS; i++)
    {
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_q2rnr_get(bbh_id, i, &wan_queue_cfg->queue_to_rnr[i].q0, &wan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdbase_get(bbh_id, i, &wan_queue_cfg->pd_fifo_base[i].base0, &wan_queue_cfg->pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdsize_get(bbh_id, i, &wan_queue_cfg->pd_fifo_size[i].size0, &wan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pdwkuph_get(bbh_id, i, &wan_queue_cfg->pd_wkup_threshold[i].threshold0, &wan_queue_cfg->pd_wkup_threshold[i].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(bbh_id, i, &wan_queue_cfg->pd_bytes_threshold[i].threshold0, &wan_queue_cfg->pd_bytes_threshold[i].threshold1);
    }
    return rc;
}

static int drv_bbh_tx_lan_queue_cfg_set(uint8_t bbh_id, pd_queue_cfg_t *lan_queue_cfg)
{
    bdmf_error_t rc;
#ifdef BCM68360
    int i;
#endif

    /* CHECK : how many queues to set for LAN (q2rnr)? */
    rc = ag_drv_bbh_tx_common_configurations_q2rnr_set(bbh_id, 0, lan_queue_cfg->queue_to_rnr[0].q0,
        lan_queue_cfg->queue_to_rnr[0].q1);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_q2rnr_set(bbh_id, 1, lan_queue_cfg->queue_to_rnr[1].q0,
        lan_queue_cfg->queue_to_rnr[1].q1);

    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdbase_set(bbh_id, lan_queue_cfg->pd_fifo_base[0].base0,
        lan_queue_cfg->pd_fifo_base[0].base1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdsize_set(bbh_id, lan_queue_cfg->pd_fifo_size[0].size0,
        lan_queue_cfg->pd_fifo_size[0].size1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdwkuph_set(bbh_id, lan_queue_cfg->pd_wkup_threshold[0].threshold0,
        lan_queue_cfg->pd_wkup_threshold[0].threshold1);

    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pd_byte_th_set(bbh_id,
        lan_queue_cfg->pd_bytes_threshold[0].threshold0, lan_queue_cfg->pd_bytes_threshold[0].threshold1);
#ifdef BCM68360
    for (i = 0; i < LAN_QEUEU_PAIRS; i++)
    {
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_q2rnr_set(bbh_id, i, lan_queue_cfg->queue_to_rnr[i].q0, lan_queue_cfg->queue_to_rnr[i].q1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdbase_set(bbh_id, i, lan_queue_cfg->pd_fifo_base[i].base0, lan_queue_cfg->pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdsize_set(bbh_id, i, lan_queue_cfg->pd_fifo_size[i].size0, lan_queue_cfg->pd_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pdwkuph_set(bbh_id, i, lan_queue_cfg->pd_wkup_threshold[i].threshold0, lan_queue_cfg->pd_wkup_threshold[i].threshold1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(bbh_id, i, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE, DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE);

        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_gtxthresh_set(bbh_id, i, LAN_TX_THRESHOLD);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_febase_set(bbh_id, i, lan_queue_cfg->fe_fifo_base[i].base0, lan_queue_cfg->fe_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_fesize_set(bbh_id, i, lan_queue_cfg->fe_fifo_size[i].size0, lan_queue_cfg->fe_fifo_size[i].size1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_fepdbase_set(bbh_id, i, lan_queue_cfg->fe_pd_fifo_base[i].base0, lan_queue_cfg->fe_pd_fifo_base[i].base1);
        rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_fepdsize_set(bbh_id, i, lan_queue_cfg->fe_pd_fifo_size[i].size0, lan_queue_cfg->fe_pd_fifo_size[i].size1);
    }
#endif
    return rc;
}

static int drv_bbh_tx_lan_queue_cfg_get(uint8_t bbh_id, pd_queue_cfg_t *lan_queue_cfg)
{
    bdmf_error_t rc;

    /* CHECK : how many queues to set for LAN (q2rnr)? */
    rc = ag_drv_bbh_tx_common_configurations_q2rnr_get(bbh_id, 0, &lan_queue_cfg->queue_to_rnr[0].q0,
        &lan_queue_cfg->queue_to_rnr[0].q1);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_q2rnr_get(bbh_id, 1, &lan_queue_cfg->queue_to_rnr[1].q0,
        &lan_queue_cfg->queue_to_rnr[1].q1);

    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdbase_get(bbh_id, &lan_queue_cfg->pd_fifo_base[0].base0,
        &lan_queue_cfg->pd_fifo_base[0].base1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pdsize_get(bbh_id, &lan_queue_cfg->pd_fifo_size[0].size0,
        &lan_queue_cfg->pd_fifo_size[0].size1);
    rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_pd_byte_th_get(bbh_id,
        &lan_queue_cfg->pd_bytes_threshold[0].threshold0, &lan_queue_cfg->pd_bytes_threshold[0].threshold1);

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

    if(IS_WAN_PORT(bbh_id))
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

    if(IS_WAN_PORT(bbh_id))
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

#ifdef USE_BDMF_SHELL
/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
static int drv_bbh_tx_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_tx_config config = {};
    bbh_id_e bbh_id = parm[0].value.unumber;
    bdmf_error_t rc;
    bbh_tx_bbh_dma_cfg dma_cfg = {};
    bbh_tx_bbh_sdma_cfg sdma_cfg = {};
    bbh_tx_bbh_ddr_cfg ddr_cfg = {};
    bdmf_boolean stop_len_err, cmp_width, epon_urgent_flag, consider_full, add_crc;
    uint8_t i, max_req;

    rc = drv_bbh_tx_configuration_get(bbh_id, &config);
    if (!rc)
    {
        bdmf_session_print(session, "BBH TX configurations for port %s:\n\r", bbh_id_enum_table[bbh_id].name);
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

        if(IS_WAN_PORT(bbh_id))
        {
            bdmf_session_print(session, "pd empty threshold:   %d\n\r", config.wan_queue_cfg.pd_empty_threshold);
            bdmf_session_print(session, "pd bytes threshold en:%d\n\r", config.wan_queue_cfg.pd_bytes_threshold_en);

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
            bdmf_session_print(session, "pd fifo size:\n\r");
            bdmf_session_print(session, "=============\n\r");
            for (i = 0; i < TX_QEUEU_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x, ", 2*i, config.wan_queue_cfg.pd_fifo_size[i].size0, 2*i+1, config.wan_queue_cfg.pd_fifo_size[i].size1);
            }
            bdmf_session_print(session, "pd wkup threshold:\n\r");
            bdmf_session_print(session, "==================\n\r");
            for (i = 0; i < TX_QEUEU_PAIRS; i++)
            {
                bdmf_session_print(session, "queue %d: %x, queue %d: %x ", 2*i, config.wan_queue_cfg.pd_wkup_threshold[i].threshold0, 2*i+1, config.wan_queue_cfg.pd_wkup_threshold[i].threshold1);
            }

            bdmf_session_print(session, "pd byte threshold:\n\r");
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
                rc = ag_drv_bbh_tx_wan_configurations_epncfg_get(bbh_id, &stop_len_err, &cmp_width, &consider_full, &add_crc);
                if (!rc)
                {
                    bdmf_session_print(session, "stop length err flag:     %d\n\r", stop_len_err);
                    bdmf_session_print(session, "Compare width flag:       %d\n\r", cmp_width);
                    bdmf_session_print(session, "Consider full flag:       %d\n\r", consider_full);
                    bdmf_session_print(session, "Add CRC:                  %d\n\r", add_crc);
                }
            }
        }
        else
        {
            bdmf_session_print(session, "pd empty threshold:   %d\n\r", config.lan_queue_cfg.pd_empty_threshold);
            bdmf_session_print(session, "pd bytes threshold en:%d\n\r", config.lan_queue_cfg.pd_bytes_threshold_en);

            bdmf_session_print(session, "pd fifo base:\n\r queue 0: %x, queue 1: %x\n\r", config.lan_queue_cfg.pd_fifo_base[0].base0, config.wan_queue_cfg.pd_fifo_base[0].base1);
            bdmf_session_print(session, "pd size base:\n\r queue 0: %x, queue 1: %x\n\r", config.lan_queue_cfg.pd_fifo_size[0].size0, config.wan_queue_cfg.pd_fifo_size[0].size1);
            bdmf_session_print(session, "pd byte threshold:\n\r queue 0: %x, queue 1: %x\n\r", config.lan_queue_cfg.pd_bytes_threshold[0].threshold0, config.wan_queue_cfg.pd_bytes_threshold[0].threshold1);

            /* need to send 4 thresholds whay?*/
           /* bdmf_session_print(session, "pd wkup threshold:\n\r queue 0: %x, queue 1: %x\n\r", config.lan_queue_cfg.pd_wkup_threshold.threshold0, config.wan_queue_cfg.pd_wkup_threshold.threshold1);*/
        }
    }
    rc = ag_drv_bbh_tx_bbh_dma_cfg_get(bbh_id, &dma_cfg);
    if (!rc)
    {
        bdmf_session_print(session, "DMA configurations\n\r");
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

static int drv_bbh_tx_cli_debug_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_id_e bbh_id = parm[0].value.unumber;
    bdmf_error_t rc;
    bbh_tx_debug_counters debug_counters = {};
    
    rc = ag_drv_bbh_tx_debug_counters_get(bbh_id, &debug_counters);
    if (!rc)
    {
        bdmf_session_print(session, "BBH TX debug counters for port %s:\n\r", bbh_id_enum_table[bbh_id].name);
        bdmf_session_print(session, "=====================================\n\r");
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

int drv_bbh_tx_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_id_e bbh_id = parm[0].value.unumber;
    bdmf_error_t rc;
    bbh_tx_debug_counters debug_counters = {};
    
    rc = ag_drv_bbh_tx_debug_counters_get(bbh_id, &debug_counters);
    if (!rc)
    {
        if (debug_counters.pddrop)
            bdmf_session_print(session, "pddrop:        %d\n\r", debug_counters.pddrop);
        if (debug_counters.aggrlenerr)
            bdmf_session_print(session, "aggrlenerr:    %d\n\r", debug_counters.aggrlenerr);
        if (debug_counters.msgdrop)
            bdmf_session_print(session, "msgdrop:       %d\n\r", debug_counters.msgdrop);
        if (debug_counters.stsdrop)
            bdmf_session_print(session, "stsdrop:       %d\n\r", debug_counters.stsdrop);
        if (debug_counters.lenerr) 
            bdmf_session_print(session, "lenerr:        %d\n\r", debug_counters.lenerr);
    }
    return rc;
}

#ifdef USE_BDMF_SHELL
static bdmfmon_handle_t bbh_dir;

void drv_bbh_tx_cli_init(bdmfmon_handle_t driver_dir)
{
    bbh_dir = ag_drv_bbh_tx_cli_init(driver_dir);

    BDMFMON_MAKE_CMD(bbh_dir, "cfg_get", "bbh tx configuration", drv_bbh_tx_cli_config_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0));
    BDMFMON_MAKE_CMD(bbh_dir, "debug_counter_get", "bbh tx counters", drv_bbh_tx_cli_debug_counters_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0));
    BDMFMON_MAKE_CMD(bbh_dir, "sanity_get", "bbh tx counters", drv_bbh_tx_cli_sanity_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0));
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

