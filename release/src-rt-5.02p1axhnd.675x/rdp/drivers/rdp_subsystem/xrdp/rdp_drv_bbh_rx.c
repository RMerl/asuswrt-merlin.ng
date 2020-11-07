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

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "xrdp_drv_drivers_common_ag.h"
#include "rdp_subsystem_common.h"
#include "rdp_drv_bbh_rx.h"
#include "rdp_common.h"

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/

typedef bdmf_error_t (*drv_bbh_rx_pkt_size_set_cb_t)(bbh_id_e bbh_id, const uint8_t minpkt_size, uint16_t maxpkt_size);
drv_bbh_rx_pkt_size_set_cb_t drv_bbh_rx_pkt_size_set_cbs[] = {
    (drv_bbh_rx_pkt_size_set_cb_t) ag_drv_bbh_rx_pkt_size0_set,
    (drv_bbh_rx_pkt_size_set_cb_t) ag_drv_bbh_rx_pkt_size1_set,
    (drv_bbh_rx_pkt_size_set_cb_t) ag_drv_bbh_rx_pkt_size2_set,
    (drv_bbh_rx_pkt_size_set_cb_t) ag_drv_bbh_rx_pkt_size3_set,
    NULL
};

#define DRV_BBH_RX_PKT_SIZE_SET(sel, bbh_id, minpkt_size, maxpkt_size) \
    drv_bbh_rx_pkt_size_set_cbs[sel](bbh_id, minpkt_size, maxpkt_size)

typedef bdmf_error_t (*drv_bbh_rx_pkt_size_get_cb_t)(bbh_id_e bbh_id, uint8_t *minpkt_size, uint16_t *maxpkt_size);
drv_bbh_rx_pkt_size_get_cb_t drv_bbh_rx_pkt_size_get_cbs[] = {
    (drv_bbh_rx_pkt_size_get_cb_t) ag_drv_bbh_rx_pkt_size0_get,
    (drv_bbh_rx_pkt_size_get_cb_t) ag_drv_bbh_rx_pkt_size1_get,
    (drv_bbh_rx_pkt_size_get_cb_t) ag_drv_bbh_rx_pkt_size2_get,
    (drv_bbh_rx_pkt_size_get_cb_t) ag_drv_bbh_rx_pkt_size3_get,
    NULL
};

#define DRV_BBH_RX_PKT_SIZE_GET(sel, bbh_id, minpkt_size, maxpkt_size) \
    drv_bbh_rx_pkt_size_get_cbs[sel](bbh_id, minpkt_size, maxpkt_size)

#define MIN_PKT_SIZE_MAX_VAL 0x60

#define PER_FLOW_TH_MIN_VAL 0x20

static int drv_bbh_rx_sdma_configuration_set(bbh_id_e bbh_id, bbh_rx_sdma_chunks_cfg_t * sdma_chunks_cfg)
{
    bdmf_error_t rc;
    bbh_rx_sdma_config sdma_config = {};

    sdma_config.database = sdma_chunks_cfg->first_chunk_idx;
    sdma_config.descbase = sdma_chunks_cfg->first_chunk_idx;
    sdma_config.numofcd = sdma_chunks_cfg->sdma_chunks;
    sdma_config.exclth = sdma_chunks_cfg->sdma_chunks;

    rc = ag_drv_bbh_rx_sdma_config_set(bbh_id, &sdma_config);
    return rc ? rc : ag_drv_bbh_rx_sdma_bb_id_set(bbh_id, sdma_chunks_cfg->sdma_bb_id);
}

int drv_bbh_rx_pkt_size_set(uint8_t bbh_id, int sel, uint8_t min_pkt_size, uint16_t max_pkt_size)
{
    if (!min_pkt_size)
        min_pkt_size = MIN_PKT_SIZE_MAX_VAL;
    else if (min_pkt_size > MIN_PKT_SIZE_MAX_VAL)
        return BDMF_ERR_RANGE;

    return DRV_BBH_RX_PKT_SIZE_SET(sel, bbh_id, min_pkt_size, max_pkt_size);
}

int drv_bbh_rx_pkt_size_get(uint8_t bbh_id, int sel, uint8_t *min_pkt_size, uint16_t *max_pkt_size)
{
    return DRV_BBH_RX_PKT_SIZE_GET(sel, bbh_id, min_pkt_size, max_pkt_size);
}

int drv_bbh_rx_configuration_set(uint8_t bbh_id, bbh_rx_config *config)
{
    int i;
    bdmf_error_t rc;

    rc = drv_bbh_rx_sdma_configuration_set(bbh_id, config->sdma_chunks_cfg);
    rc = rc ? rc : ag_drv_bbh_rx_dispatcher_sbpm_bb_id_set(bbh_id, config->disp_bb_id, config->sbpm_bb_id);
    rc = rc ? rc : ag_drv_bbh_rx_dispatcher_virtual_queues_set(bbh_id, config->normal_viq, config->excl_viq);
    rc = rc ? rc : ag_drv_bbh_rx_pattern_recog_set(bbh_id, &config->pattern_recog);
    rc = rc ? rc : ag_drv_bbh_rx_pause_en_set(bbh_id, config->excl_cfg.pause_en);
    rc = rc ? rc : ag_drv_bbh_rx_pfc_en_set(bbh_id, config->excl_cfg.pfc_en);
    rc = rc ? rc : ag_drv_bbh_rx_ctrl_en_set(bbh_id, config->excl_cfg.ctrl_en);
    rc = rc ? rc : ag_drv_bbh_rx_pattern_en_set(bbh_id, config->excl_cfg.pattern_en);
    rc = rc ? rc : ag_drv_bbh_rx_exc_en_set(bbh_id, config->excl_cfg.exc_en);

    for (i = 0; i < 4; i++)
        rc = rc ? rc : drv_bbh_rx_pkt_size_set(bbh_id, i, config->min_pkt_size[i], config->max_pkt_size[i]);

    if (config->sop_offset > DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET)
        return BDMF_ERR_RANGE;
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_sopoffset_set(bbh_id, config->sop_offset);

    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_crcomitdis_set(bbh_id, config->crc_omit_dis);
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_g9991en_set(bbh_id, config->gint_en, config->flow_ctrl_swap);

    if (config->per_flow_th < PER_FLOW_TH_MIN_VAL)
        return BDMF_ERR_RANGE;
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_perflowth_set(bbh_id, config->per_flow_th);

    rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_drops_config_set(bbh_id, &config->flow_ctrl_cfg.drops);
    rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, config->flow_ctrl_cfg.timer);
#if !defined(DUAL_ISSUE)
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_sbpmcfg_set(bbh_id, config->max_otf_sbpm);
#else
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_sbpmcfg_set(bbh_id, config->max_otf_sbpm, 0, 0);
#endif

    /* TODO: wait for update of RXRST and pm_counters */
    return rc;
}

int drv_bbh_rx_configuration_get(uint8_t bbh_id, bbh_rx_config *config)
{
    uint8_t i;
    bdmf_error_t rc;

    rc = ag_drv_bbh_rx_dispatcher_sbpm_bb_id_get(bbh_id, &config->disp_bb_id, &config->sbpm_bb_id);
    rc = rc ? rc : ag_drv_bbh_rx_dispatcher_virtual_queues_get(bbh_id, &config->normal_viq, &config->excl_viq);
    rc = rc ? rc : ag_drv_bbh_rx_ploam_en_get(bbh_id, &config->excl_cfg.ploam_en);
    rc = rc ? rc : ag_drv_bbh_rx_user_priority3_en_get(bbh_id, &config->excl_cfg.user_prio3_en);
    rc = rc ? rc : ag_drv_bbh_rx_pause_en_get(bbh_id, &config->excl_cfg.pause_en);
    rc = rc ? rc : ag_drv_bbh_rx_pfc_en_get(bbh_id, &config->excl_cfg.pfc_en);
    rc = rc ? rc : ag_drv_bbh_rx_ctrl_en_get(bbh_id, &config->excl_cfg.ctrl_en);
    rc = rc ? rc : ag_drv_bbh_rx_pattern_recog_get(bbh_id, &config->pattern_recog);

    for (i = 0; i < 4; i++)
        rc = rc ? rc : DRV_BBH_RX_PKT_SIZE_GET(i, bbh_id, &config->min_pkt_size[i], &config->max_pkt_size[i]);

    /* TODO: SOP should match the relevant configuration in the runner block */
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_sopoffset_get(bbh_id, &config->sop_offset);
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_crcomitdis_get(bbh_id, &config->crc_omit_dis);
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_g9991en_get(bbh_id, &config->gint_en, &config->flow_ctrl_swap);
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_perflowth_get(bbh_id, &config->per_flow_th);

    rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_drops_config_get(bbh_id, &config->flow_ctrl_cfg.drops);
    rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_timer_get(bbh_id, &config->flow_ctrl_cfg.timer);

#if !defined(DUAL_ISSUE)
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_sbpmcfg_get(bbh_id, &config->max_otf_sbpm);
#else
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_sbpmcfg_get(bbh_id, &config->max_otf_sbpm, &config->pridropen, &config->cngsel);
#endif


    return rc;
}

void drv_bbh_rx_get_stat(uint8_t bbh_id, bbh_rx_counters_t *counters)
{
    ag_drv_bbh_rx_pm_counters_get(bbh_id, &counters->pm_counters);
    ag_drv_bbh_rx_error_pm_counters_get(bbh_id, &counters->error_counter);
}

int drv_bbh_rx_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{
     bbh_rx_general_configuration_clk_gate_cntrl bbh_rx_ctrl;
     uint8_t block_id = 0;

     for (block_id = 0; block_id < RU_BLK(BBH_RX).addr_count; block_id++)
     {
         ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(block_id, &bbh_rx_ctrl);
         bbh_rx_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
         bbh_rx_ctrl.keep_alive_en = auto_gate ? 1 : 0;
         bbh_rx_ctrl.timer_val = timer_val;
         ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(block_id, &bbh_rx_ctrl);
     }

     return 0;
}

#ifdef USE_BDMF_SHELL
static inline int drv_bbh_rx_pkt_size_cfg_print(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    bdmf_error_t rc = BDMF_ERR_OK;

    parm[1].value.unumber = parm[0].value.unumber;

    bdmf_session_print(session, "\nPacket size configurations:\n");
    for (i = 0; i < 4; i++)
    {
        parm[0].value.unumber = cli_bbh_rx_pkt_size0 + i;
        rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);
    }

    n_parms++;
    bdmf_session_print(session, "Min pkt sel flows:\n");
    parm[0].value.unumber = cli_bbh_rx_min_pkt_sel_flows_0_15;
    for (i = 0; i < 16; i++)
    {
        bdmf_session_print(session, "[%d]:", i);
        parm[2].value.unumber = i;
        rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);
    }
    parm[0].value.unumber = cli_bbh_rx_min_pkt_sel_flows_16_31;
    for (i = 0; i < 16; i++)
    {
        bdmf_session_print(session, "[%d]:", (i + 16));
        parm[2].value.unumber = i;
        rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);
    }

    bdmf_session_print(session, "Max pkt sel flows:\n");
    parm[0].value.unumber = cli_bbh_rx_max_pkt_sel_flows_0_15;
    for (i = 0; i < 16; i++)
    {
        bdmf_session_print(session, "[%d]:", i);
        parm[2].value.unumber = i;
        rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);
    }
    parm[0].value.unumber = cli_bbh_rx_max_pkt_sel_flows_16_31;
    for (i = 0; i < 16; i++)
    {
        bdmf_session_print(session, "[%d]:", i + 16);
        parm[2].value.unumber = i;
        rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);
    }
    n_parms--;

    parm[0].value.unumber = cli_bbh_rx_general_configuration_perflowth;
    rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);

    parm[0].value.unumber = cli_bbh_rx_pkt_sel_group_0;
    rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);

    parm[0].value.unumber = cli_bbh_rx_pkt_sel_group_1;
    rc = rc ? rc : bcm_bbh_rx_cli_get(session, parm, n_parms);

    return rc;
}

int drv_bbh_rx_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bbh_id_e bbh_id = parm[0].value.unumber;

    static uint32_t gen_cfg[] = {cli_bbh_rx_dispatcher_sbpm_bb_id, cli_bbh_rx_general_configuration_crcomitdis,
            cli_bbh_rx_general_configuration_g9991en, cli_bbh_rx_general_configuration_sopoffset,
            cli_bbh_rx_general_configuration_sbpmcfg, cli_bbh_rx_mac_mode};
    static uint32_t disp_reor_cfg[] = {cli_bbh_rx_dispatcher_virtual_queues, cli_bbh_rx_ctrl_en,
            cli_bbh_rx_pause_en, cli_bbh_rx_pfc_en, cli_bbh_rx_ploam_en, cli_bbh_rx_user_priority3_en,
            cli_bbh_rx_pattern_recog};
    static uint32_t sdma_cfg[] = {cli_bbh_rx_sdma_bb_id, cli_bbh_rx_sdma_config};
    static uint32_t flow_ctrl_cfg[] = {cli_bbh_rx_flow_ctrl_timer, cli_bbh_rx_flow_ctrl_drops_config};

    bdmf_session_print(session, "BBH RX configurations for port %s:\n", bbh_id_enum_table[bbh_id].name);
    HAL_CLI_IDX_PRINT_LIST(session, bbh_rx, gen_cfg, bbh_id);

    bdmf_session_print(session, "\nDispatchers exclusive VIQ (Virtual Ingress Queue) configurations:\n");
    HAL_CLI_IDX_PRINT_LIST(session, bbh_rx, disp_reor_cfg, bbh_id);

    bdmf_session_print(session, "\nSDMA Configurations:\n");
    HAL_CLI_IDX_PRINT_LIST(session, bbh_rx, sdma_cfg, bbh_id);

    bdmf_session_print(session, "\nFlow ctrl configurations:\n");
    HAL_CLI_IDX_PRINT_LIST(session, bbh_rx, flow_ctrl_cfg, bbh_id);

    return drv_bbh_rx_pkt_size_cfg_print(session, parm, n_parms);
}

static bdmfmon_handle_t bbh_dir;

void drv_bbh_rx_cli_init(bdmfmon_handle_t driver_dir)
{
    bbh_dir = ag_drv_bbh_rx_cli_init(driver_dir);

    BDMFMON_MAKE_CMD(bbh_dir, "cfg_get", "bbh rx configuration", (bdmfmon_cmd_cb_t)drv_bbh_rx_cli_config_get,
        BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0));
}


void drv_bbh_rx_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (bbh_dir)
    {
        bdmfmon_token_destroy(bbh_dir);
        bbh_dir = NULL;
    }
}

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

