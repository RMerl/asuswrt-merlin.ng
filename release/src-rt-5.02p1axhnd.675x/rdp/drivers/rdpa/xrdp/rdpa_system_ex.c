/*
 * <:copyright-BRCM:2015:proprietary:standard
 *
 *    Copyright (c) 2015 Broadcom
 *    All Rights Reserved
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdp_drv_rnr.h"
#include "rdd_tcam_ic.h"
#include "rdd_init.h"
#ifdef G9991
#include "rdd_scheduling.h"
#endif
#include "rdpa_system_ex.h"
#include "rdp_drv_cntr.h"
#include "rdd_cpu_rx.h"
#include "rdd_ag_cpu_rx.h"
#include "rdp_drv_bbh_tx.h"
#include "rdp_drv_bbh_rx.h"
#include "rdp_drv_dis_reor.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_qm.h"
#include "rdp_drv_psram.h"
#include "rdp_drv_bac_if.h"
#ifdef CONFIG_MCAST_TASK_LIMIT
#include "rdp_drv_cnpl.h"
#endif
#include "rdpa_platform.h"
#include "rdp_drv_fpm.h"
#include "xrdp_drv_bbh_rx_ag.h"
#include "xrdp_drv_bbh_tx_ag.h"
#include "xrdp_drv_dma_ag.h"
#include "xrdp_drv_psram_ag.h"
#include "xrdp_drv_sbpm_ag.h"
#include "xrdp_drv_natc_ctrs_ag.h"

#ifdef CONFIG_BCM_GPON_TODD
#include <gpon_tod_gpl.h>
#endif

#if defined(DEBUG_PRINTS) && !defined(RDP_SIM)
#include<linux/kthread.h>
#include <linux/bcm_realtime.h>
#include "rdd_debug.h"

static struct task_struct *runner_print_task;
struct task_struct *create_runner_print_task(void);
extern void rdd_debug_prints_handle(void);
#endif

extern int triple_tag_detect_ref_count;
extern int num_wan;
extern int num_lan;
extern struct bdmf_object *system_object;

static uint8_t cpu_reason_to_tc[rdpa_cpu_reason__num_of] = { };
static uint32_t fpm_isr_delay_timer_period;

static bdmf_boolean auto_gate;

#if !defined(BCM6858) && !defined(BCM6846) && !defined(BCM63158) && !defined(BCM6878)
#define EGRESS_TM_COLOR_DROP_TYPE 1
#endif

#define _OFFSET_COMMON_(name) offsetof(rdpa_system_common_stat_t, name)

#define _SET_CLK_GATE_(name, auto_gate_bypass, wait_counter, keep_alive) \
    do \
    { \
        name.bypass_clk_gate = auto_gate_bypass; \
        name.timer_val = wait_counter; \
        name.keep_alive_en = keep_alive; \
    } while (0)

/***********************************************************************/
/* _GET_AND_SET_CLK_GATE_                                               */
/* macro used to get and set clock gate values                          */
/* input params:                                                        */
/*       func_name - function name without _get/_set                    */
/*       name - variable name                                           */
/* auto_gate_bypass, wait_counter, keep_alive - params that will be set */
/*                        in clock_gate register                        */
/************************************************************************/
#define _GET_AND_SET_CLK_GATE_(func_name, name, auto_gate_bypass, wait_counter, keep_alive) \
    do \
    { \
        rc = rc ? rc : func_name##_get(&(name)); \
        _SET_CLK_GATE_(name, auto_gate_bypass, wait_counter, keep_alive); \
        rc = rc ? rc : func_name##_set(&(name)); \
    } while (0)

/* for accumulation of counters read */
static rdpa_system_stat_t accumulative_system_stat;

static rdpa_debug_stat_t accumulative_debug_stat;

int system_attr_clock_gate_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    int wait_counter = 0xC8;
    bdmf_boolean auto_gate_bypass, keep_alive = 0;
    uint8_t i;
    bbh_rx_general_configuration_clk_gate_cntrl bbh_rx_clk_gate_cntrl;
    dsptchr_reorder_cfg_clk_gate_cntrl dsp_clk_gate_cntrl;
    dma_config_clk_gate_cntrl dma_clk_gate_cntrl;
    psram_configurations_clk_gate_cntrl psram_clk_gate_cntrl;
    qm_clk_gate_clk_gate_cntrl qm_clk_gate_cntrl;
#if !defined(DUAL_ISSUE)
    qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl   qm_bbh_tx_qm_bbhtx_clk_gate_cntrl;
    qm_dma_qm_dma_config_clk_gate_cntrl dma_qm_dma_config_clk_gate_cntrl;
#endif
    bdmf_error_t rc = BDMF_ERR_OK;

    auto_gate = *(bdmf_boolean *)val;
    auto_gate_bypass = auto_gate ? 0 : 1;

    for (i = 0; i < BBH_ID_NUM && !rc; i++)
    {
        rc = ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(i, &bbh_rx_clk_gate_cntrl);
        /* keep_alive =  1 - should be hardcoded keep_alive*/
        _SET_CLK_GATE_(bbh_rx_clk_gate_cntrl, auto_gate_bypass, wait_counter, 1); /* bbh_rx_clk_gate_cntrl*/
        rc = rc ? rc :  ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(i, &bbh_rx_clk_gate_cntrl);
    }

    drv_bbh_tx_config_clock_autogate(auto_gate, wait_counter, keep_alive);

    _GET_AND_SET_CLK_GATE_(ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl, dsp_clk_gate_cntrl, auto_gate_bypass, wait_counter, keep_alive);

    _GET_AND_SET_CLK_GATE_(ag_drv_psram_configurations_clk_gate_cntrl, psram_clk_gate_cntrl, auto_gate_bypass, wait_counter, keep_alive);

    _GET_AND_SET_CLK_GATE_(ag_drv_qm_clk_gate_clk_gate_cntrl, qm_clk_gate_cntrl, auto_gate_bypass, wait_counter, keep_alive);
#if !defined(DUAL_ISSUE)
    _GET_AND_SET_CLK_GATE_(ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, qm_bbh_tx_qm_bbhtx_clk_gate_cntrl, 
        auto_gate_bypass, wait_counter, keep_alive);

    _GET_AND_SET_CLK_GATE_(ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl, dma_qm_dma_config_clk_gate_cntrl, 
        auto_gate_bypass, wait_counter, keep_alive);
#endif

    /* Clock-gate enable/disable for BACIF blocks */
    drv_bac_if_config_clock_autogate(auto_gate, wait_counter, keep_alive);

    for (i = 0; i < DMA_NUM; i++)
    {
        rc = rc ? rc :  ag_drv_dma_config_clk_gate_cntrl_get(i, &dma_clk_gate_cntrl);
        _SET_CLK_GATE_(dma_clk_gate_cntrl, auto_gate_bypass, wait_counter, keep_alive);
        rc = rc ? rc :  ag_drv_dma_config_clk_gate_cntrl_set(i, &dma_clk_gate_cntrl);
    }

    rc = rc ? rc : drv_rnr_config_clock_autogate(auto_gate, wait_counter);

    return rc;
}

int system_attr_clock_gate_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
#if defined(CONFIG_BCM_XRDP_AUTOGATE)
     *(bdmf_boolean *)val = auto_gate;
     return 0;
#else
     return BDMF_ERR_NOT_SUPPORTED;
#endif
}

#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
static uint8_t _map_cpu_redirect_mode(rdpa_rx_redirect_cpu_t rdpa_redirect_mode)
{
    uint8_t rdd_mode;
    switch (rdpa_redirect_mode)
    {
    case rdpa_rx_redirect_to_cpu_disabled:
        rdd_mode = CPU_REDIRECT_TYPE_NONE;
        break;
    case rdpa_rx_redirect_to_cpu_all:
        rdd_mode = CPU_REDIRECT_TYPE_ALL;
        break;
    default:
        rdd_mode = CPU_REDIRECT_TYPE_NONE;
        break;
    }
    return rdd_mode;
}
#endif

int system_attr_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    rdpa_system_cfg_t *cfg = (rdpa_system_cfg_t *)val;
    qm_drop_counters_ctrl drop_counters_ctrl;
    bdmf_boolean ecn_request_cmd;
#ifdef G9991
    bdmf_boolean g9991_single_fragment;
#endif

    if (cfg->force_dscp_to_pbit_us || cfg->force_dscp_to_pbit_ds)
        RDD_BYTE_1_BITS_WRITE_G(1, RDD_FORCE_DSCP_ADDRESS_ARR, 0);
    else
        RDD_BYTE_1_BITS_WRITE_G(0, RDD_FORCE_DSCP_ADDRESS_ARR, 0);

#ifdef CONFIG_INGRESS_PBIT_SUPPORT
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_US_WRITE_G(0, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_DS_WRITE_G(cfg->qos_mapping_mode, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);
    RDD_SYSTEM_CONFIGURATION_ENTRY_INGRESS_PACKET_BASED_MAPPING_DS_OR_US_WRITE_G(cfg->qos_mapping_mode, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
#endif 

#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
    {
        uint8_t rdd_mode = _map_cpu_redirect_mode(cfg->cpu_redirect_mode);
        RDD_CPU_REDIRECT_MODE_ENTRY_MODE_WRITE_G(rdd_mode, RDD_CPU_REDIRECT_MODE_ADDRESS_ARR, 0);
    }
#else
    if (cfg->cpu_redirect_mode != rdpa_rx_redirect_to_cpu_disabled)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "RX Redirect to CPU is not supported\n");
    }
#endif

    ag_drv_qm_drop_counters_ctrl_get(&drop_counters_ctrl);

    /*disable_wrap_around_pkts/bytes corresponds to DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT /_BYTES_SELECT - This bit defines the functionality 
      of the drop packets counter. 0 - Functions as the drop packets counter. 1 - Functions as the max packets occupancy holder.               
    */
    switch (cfg->counter_type)
    {
    case  rdpa_drop_counter_packet:
        drop_counters_ctrl.disable_wrap_around_pkts = 0;  
        drop_counters_ctrl.disable_wrap_around_bytes = 0;
        drop_counters_ctrl.read_clear_pkts = 1;
        drop_counters_ctrl.read_clear_bytes = 1;

#ifdef EGRESS_TM_COLOR_DROP_TYPE
        drop_counters_ctrl.drop_cnt_wred_drops = 0;
#endif
        break;
    case  rdpa_counter_watermark:
        drop_counters_ctrl.disable_wrap_around_pkts = 1;
        drop_counters_ctrl.disable_wrap_around_bytes = 1;
        drop_counters_ctrl.read_clear_pkts = 0;
        drop_counters_ctrl.read_clear_bytes = 0;
#ifdef EGRESS_TM_COLOR_DROP_TYPE
        drop_counters_ctrl.drop_cnt_wred_drops = 0;
#endif
        break;
    case  rdpa_drop_counter_color:
#ifdef EGRESS_TM_COLOR_DROP_TYPE
        drop_counters_ctrl.disable_wrap_around_pkts = 0;
        drop_counters_ctrl.disable_wrap_around_bytes = 0;
        drop_counters_ctrl.read_clear_pkts = 1;
        drop_counters_ctrl.read_clear_bytes = 1;
        drop_counters_ctrl.drop_cnt_wred_drops = 1;
#else
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Drop counter according to color is not supported\n");
#endif
        break;
    default:
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Drop counter according to color is not supported\n");
    }
    drv_qm_drop_counter_clr();
    ag_drv_qm_drop_counters_ctrl_set(&drop_counters_ctrl);

    ecn_request_cmd = (cfg->options & (1 << rdpa_ecn_ipv6_remarking_option)) ? 1 : 0;
    rdd_ecn_remark_enable_cfg(ecn_request_cmd);

#ifdef G9991
    g9991_single_fragment = (cfg->options & (1 << rdpa_g9991_single_fragment_option)) ? 1 : 0;
    rdd_g9991_single_fragment_enable_cfg(g9991_single_fragment);
#endif

    return 0;
}

/* TPID Detect */
static int __tpid_detect_cfg(struct bdmf_object * const mo,
    rdpa_tpid_detect_t tpid_detect,
    drv_parser_qtag_profile_t profile,
    const rdpa_tpid_detect_cfg_t * const tpid_detect_cfg)
{
    uint8_t i;
    system_drv_priv_t *priv = (system_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t ethertype_0[NUM_OF_RNR_QUAD];
    uint16_t ethertype_1[NUM_OF_RNR_QUAD];
    rdpa_tpid_detect_t etype_index;
    bdmf_error_t rc = BDMF_ERR_OK;

    /* Handle user-defined */
    if ((tpid_detect == rdpa_tpid_detect_udef_1) ||
        (tpid_detect == rdpa_tpid_detect_udef_2))
    {
        for (i = 0; !rc && i < NUM_OF_RNR_QUAD; i++)
            rc = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(i, &ethertype_0[i], &ethertype_1[i]);
        if (rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to get user-defined TPIDs Detect values\n");

        if (tpid_detect == rdpa_tpid_detect_udef_1)
        {
            for (i = 0; i < NUM_OF_RNR_QUAD; i++)
                ethertype_0[i] = tpid_detect_cfg->val_udef;
        }
        else
        {
            for (i = 0; i < NUM_OF_RNR_QUAD; i++)
                ethertype_1[i] = tpid_detect_cfg->val_udef;
        }

        for (i = 0; !rc && i < NUM_OF_RNR_QUAD; i++)
            rc = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(i, ethertype_0[i], ethertype_1[i]);

        if (rc)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to set user-defined TPID Detect values\n");

        /* Update data */
        priv->tpids_detect[tpid_detect].val_udef = tpid_detect_cfg->val_udef;
    }

    etype_index = tpid_detect;

    /* Call PARSER API */
    for (i = 0; i < NUM_OF_RNR_QUAD; i++)
    {
        rc = rc ? rc : drv_rnr_quad_parser_configure_outer_qtag(i, profile, tpid_detect_cfg->otag_en, etype_index);
        rc = rc ? rc : drv_rnr_quad_parser_configure_inner_qtag(i, profile, tpid_detect_cfg->itag_en, etype_index);
        rc = rc ? rc : drv_rnr_quad_parser_configure_3rd_qtag(i, profile, tpid_detect_cfg->triple_en, etype_index);
    }

    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Failed to configure TPID Detect: %u\n", tpid_detect);

    /* Handle the SA operations ref counter */
    if (priv->tpids_detect[tpid_detect].triple_en)
    {
        if (!tpid_detect_cfg->triple_en)
            triple_tag_detect_ref_count--;
    }
    else
    {
        if (tpid_detect_cfg->triple_en)
            triple_tag_detect_ref_count++;
    }

    /* Update data */
    priv->tpids_detect[tpid_detect].otag_en = tpid_detect_cfg->otag_en;
    priv->tpids_detect[tpid_detect].itag_en = tpid_detect_cfg->itag_en;
    priv->tpids_detect[tpid_detect].triple_en = tpid_detect_cfg->triple_en;

    return rc;
}

int _tpid_detect_cfg(struct bdmf_object * const mo, rdpa_tpid_detect_t tpid_detect,
    const rdpa_tpid_detect_cfg_t * const tpid_detect_cfg)
{
    return __tpid_detect_cfg(mo, tpid_detect, DRV_PARSER_QTAG_PROFILE_0, tpid_detect_cfg) ||
        __tpid_detect_cfg(mo, tpid_detect, DRV_PARSER_QTAG_PROFILE_1, tpid_detect_cfg) ||
        __tpid_detect_cfg(mo, tpid_detect, DRV_PARSER_QTAG_PROFILE_2, tpid_detect_cfg);
}

int system_post_init_enumerate_emacs(struct bdmf_object *mo)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(mo);

    num_wan = 1;
    num_lan = __bitcount(system->init_cfg.enabled_emac);

    if (system->init_cfg.gbe_wan_emac != rdpa_emac_none)
    {
        if (system->init_cfg.enabled_emac & (1 << system->init_cfg.gbe_wan_emac))
            num_lan--;
        system->init_cfg.enabled_emac |= (1 << system->init_cfg.gbe_wan_emac);
    }
    return 0;
}

int system_pre_init_ex(struct bdmf_object *mo)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(mo);
    system->qm_cfg.number_of_ds_queues = QM_QUEUE_DS_DEFAULT_QUANTITY;
    system->qm_cfg.number_of_us_queues = QM_QUEUE_US_DEFAULT_QUANTITY;
    system->qm_cfg.number_of_service_queues =  QM_QUEUE_SERVICE_Q_DEFAULT_QUANTITY;
    memset(cpu_reason_to_tc, RDPA_CPU_TC_DEFAULT, rdpa_cpu_reason__num_of);
    system->counter_cfg.vlan_stats_enable = 0;
    system->counter_cfg.shared_counters = PROJ_DEFS_NUMBER_OF_SHARED_IC_VLAN_COUNTERS;
    fpm_isr_delay_timer_period = FPM_INTERRUPT_TIMER_DELAY;
    return 0;
}

#ifndef RDP_SIM
static bdmf_timer_t fpm_isr_timer;

static void _fpm_isr_timer_task(bdmf_timer_t *timer, unsigned long priv)
{
    fpm_pool2_intr_msk interrupt_mask = FPM_INTERRUPT_MASK;
    ag_drv_fpm_pool1_intr_msk_set(&interrupt_mask);
}

static int _fpm_isr_wrapper(int irq, void *priv)
{
    fpm_pool2_intr_sts interrupt_status = FPM_INTERRUPT_MASK_OFF;
    fpm_pool2_intr_msk interrupt_mask = FPM_INTERRUPT_MASK_OFF;
    bdmf_error_t rc = BDMF_ERR_OK;

    /* TODO! if we decide to enable 2 FPM pools, then this part of code needs
     * adjustment to also take care of 2nd pool */
    /* fpm interrupts */
    rc = ag_drv_fpm_pool1_intr_sts_get(&interrupt_status);
    rc = rc ? rc : ag_drv_fpm_pool1_intr_msk_get(&interrupt_mask);
    if (!rc)
    {
        bdmf_timer_start(&fpm_isr_timer, bdmf_ms_to_ticks(fpm_isr_delay_timer_period));

        if (interrupt_status.expired_token_recov_sts &&
            interrupt_mask.expired_token_recov_msk)
        {
            interrupt_mask.expired_token_recov_msk = 0;
            pr_info("Expired Token is recovered!\n");
        }

        if (interrupt_status.expired_token_det_sts &&
            interrupt_mask.expired_token_det_msk)
        {
            interrupt_mask.expired_token_det_msk = 0;
            pr_warn("Expired Token is detected!\n");
        }

        if (interrupt_status.illegal_alloc_request_sts &&
            interrupt_mask.illegal_alloc_request_msk)
        {
            interrupt_mask.illegal_alloc_request_msk = 0;
            pr_warn("Pool is fully allocated! Can't allocate anymore!\n");
        }

        if (interrupt_status.illegal_address_access_sts &&
            interrupt_mask.illegal_address_access_msk)
        {
            interrupt_mask.illegal_address_access_msk = 0;
            pr_err("Illegal address access!\n");
        }

#if defined(BCM_DSL_XRDP)
        if (interrupt_status.xon_state_sts && interrupt_mask.xon_msk)
        {
            interrupt_mask.xon_msk = 0;
            pr_info("XON state! Backpressure is released!\n");
        }

        if (interrupt_status.xoff_state_sts && interrupt_mask.xoff_msk)
        {
            interrupt_mask.xoff_msk = 0;
            pr_info("XOFF state! Backpressure is enabled!\n");
        }
#endif

        if (interrupt_status.memory_corrupt_sts &&
            interrupt_mask.memory_corrupt_msk)
        {
            interrupt_mask.memory_corrupt_msk = 0;
            pr_warn("Index memory is corrupted!\n");
        }

        if (interrupt_status.pool_dis_free_multi_sts &&
            interrupt_mask.pool_dis_free_multi_msk)
        {
            interrupt_mask.pool_dis_free_multi_msk = 0;
            pr_err("Free/Mcast update on a disabled pool!\n");
        }

        if (interrupt_status.multi_token_index_out_of_range_sts &&
            interrupt_mask.multi_token_index_out_of_range_msk)
        {
            interrupt_mask.multi_token_index_out_of_range_msk = 0;
            pr_err("Multicast token ID out of range!\n");
        }

        if (interrupt_status.multi_token_no_valid_sts &&
            interrupt_mask.multi_token_no_valid_msk)
        {
            interrupt_mask.multi_token_no_valid_msk = 0;
            pr_err("Invalid multicast token ID!\n");
        }

        if (interrupt_status.free_token_index_out_of_range_sts &&
            interrupt_mask.free_token_index_out_of_range_msk)
        {
            interrupt_mask.free_token_index_out_of_range_msk = 0;
            pr_err("Free token ID out of range!\n");
        }

        if (interrupt_status.free_token_no_valid_sts &&
            interrupt_mask.free_token_no_valid_msk)
        {
            interrupt_mask.free_token_no_valid_msk = 0;
            pr_err("Invalid free token ID!\n");
        }

        if (interrupt_status.pool_full_sts && interrupt_mask.pool_full_msk)
        {
            interrupt_mask.pool_full_msk = 0;
            pr_warn("Pool is fully allocated!\n");
        }

        if (interrupt_status.free_fifo_full_sts &&
            interrupt_mask.free_fifo_full_msk)
        {
            interrupt_mask.free_fifo_full_msk = 0;
            pr_info("Free FIFO is full!\n");
        }

        if (interrupt_status.alloc_fifo_full_sts &&
            interrupt_mask.alloc_fifo_full_msk)
        {
            interrupt_mask.alloc_fifo_full_msk = 0;
            pr_info("Alloc FIFO is full!\n");
        }
    }

    rc = rc ? rc : ag_drv_fpm_pool1_intr_msk_set(&interrupt_mask);
    rc = rc ? rc : ag_drv_fpm_pool1_intr_sts_set(&interrupt_status);
    bdmf_int_enable(irq);
    return BDMF_IRQ_HANDLED;
}
#endif

int system_post_init_ex(struct bdmf_object *mo)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(mo);
    int i;
#ifndef RDP_SIM
    int rc;
#endif

    /* full flow cache mode */
    if (system->init_cfg.ip_class_method == rdpa_method_fc)
        rdd_full_flow_cache_cfg(1);

#if defined(CONFIG_RNR_BRIDGE)
    RDD_BYTE_1_BITS_WRITE_G(system->counter_cfg.vlan_stats_enable, RDD_TM_VLAN_STATS_ENABLE_ADDRESS_ARR, 0);
    RDD_BYTE_1_BITS_WRITE_G(system->counter_cfg.vlan_stats_enable, RDD_CPU_TX_VLAN_STATS_ENABLE_ADDRESS_ARR, 0);
    RDD_SYSTEM_CONFIGURATION_ENTRY_VLAN_STATS_ENABLE_WRITE_G(system->counter_cfg.vlan_stats_enable, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
    RDD_TX_EXCEPTION_ENTRY_VLAN_CNTR_EN_WRITE_G(system->counter_cfg.vlan_stats_enable, RDD_TX_EXCEPTION_ADDRESS_ARR, 0);
#endif

    for (i = 0; i < rdpa_cpu_reason__num_of; i++)
        rdd_ag_cpu_rx_cpu_reason_to_tc_set(i, cpu_reason_to_tc[i]);

    if (system->init_cfg.switching_mode == rdpa_vlan_aware_switching)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED,
            "rdpa_vlan_aware_switching is deprecated, "
            "please set vlan learning and swithching mode per configured bridge\n");
    }

#if defined(BCM_DSL_XRDP)
    /* SF2 mapping */
    rdd_egress_port_to_broadcom_switch_port_init(RDD_LAN0_VPORT, 0);
    rdd_egress_port_to_broadcom_switch_port_init(RDD_LAN1_VPORT, 1);
    rdd_egress_port_to_broadcom_switch_port_init(RDD_LAN2_VPORT, 2);
    rdd_egress_port_to_broadcom_switch_port_init(RDD_LAN3_VPORT, 3);
    rdd_egress_port_to_broadcom_switch_port_init(RDD_LAN4_VPORT, 4);
    rdd_egress_port_to_broadcom_switch_port_init(RDD_LAN5_VPORT, 6);

    rdd_broadcom_switch_ports_mapping_table_config(RDD_LAN0_VPORT, 0);
    rdd_broadcom_switch_ports_mapping_table_config(RDD_LAN1_VPORT, 1);
    rdd_broadcom_switch_ports_mapping_table_config(RDD_LAN2_VPORT, 2);
    rdd_broadcom_switch_ports_mapping_table_config(RDD_LAN3_VPORT, 3);
    rdd_broadcom_switch_ports_mapping_table_config(RDD_LAN4_VPORT, 4);
    rdd_broadcom_switch_ports_mapping_table_config(RDD_LAN5_VPORT, 6);

    /* Default VPORT to LOOKUP Port mapping */
    rdd_lookup_ports_mapping_table_init(RDD_WAN0_VPORT, RDD_WAN0_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_WAN1_VPORT, RDD_WAN1_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_WAN2_VPORT, RDD_WAN2_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_LAN0_VPORT, RDD_LAN0_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_LAN1_VPORT, RDD_LAN1_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_LAN2_VPORT, RDD_LAN2_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_LAN3_VPORT, RDD_LAN3_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_LAN4_VPORT, RDD_LAN4_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_LAN5_VPORT, RDD_LAN5_VPORT);
    /* CPU Ports */
    rdd_lookup_ports_mapping_table_init(RDD_CPU0_VPORT, RDD_CPU0_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_CPU1_VPORT, RDD_CPU1_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_CPU2_VPORT, RDD_CPU2_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_CPU3_VPORT, RDD_CPU3_VPORT);
    /* WLAN Ports */
    rdd_lookup_ports_mapping_table_init(RDD_WLAN0_VPORT, RDD_WLAN0_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_WLAN1_VPORT, RDD_WLAN0_VPORT);
    rdd_lookup_ports_mapping_table_init(RDD_WLAN2_VPORT, RDD_WLAN0_VPORT);
    /* Currently the flow-cache blog does not have information about RX-radio
       All flows are configured as ingress_if = rdpa_if_wlan0
       Mapping all RDD_WLAN ports to map to RDD_WLAN0 for now */
    /*rdd_lookup_ports_mapping_table_init(RDD_WLAN1_VPORT, RDD_WLAN1_VPORT);*/
    /*rdd_lookup_ports_mapping_table_init(RDD_WLAN2_VPORT, RDD_WLAN2_VPORT);*/

    rdd_lookup_ports_mapping_table_init(PROJ_DEFS_RDD_VPORT_ANY, PROJ_DEFS_RDD_VPORT_ANY);
#endif

    /* Enable congestion control by default */
    rdd_system_congestion_ctrl_enable(1);

    /* Enable clock autogating by default */
#if defined(CONFIG_BCM_XRDP_AUTOGATE)
    /* Enable clock autogating by default */
    auto_gate = 1;
#else
    auto_gate = 0;
#endif
    system_attr_clock_gate_write(NULL, NULL, 0, &auto_gate, 0);
    memset(&accumulative_system_stat, 0, sizeof(rdpa_system_stat_t));
    memset(&accumulative_debug_stat, 0, sizeof(rdpa_debug_stat_t));

#ifdef CONFIG_INGRESS_PBIT_SUPPORT
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_US_WRITE_G(0, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);
    RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY_DS_WRITE_G(system->cfg.qos_mapping_mode, RDD_INGRESS_PACKET_BASED_MAPPING_ADDRESS_ARR, 0);
    RDD_SYSTEM_CONFIGURATION_ENTRY_INGRESS_PACKET_BASED_MAPPING_DS_OR_US_WRITE_G(system->cfg.qos_mapping_mode, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
#endif 

#ifndef RDP_SIM
    bdmf_timer_init(&fpm_isr_timer, _fpm_isr_timer_task, 0);
    rc = bdmf_int_connect(INTERRUPT_ID_FPM, rdpa_cpu_host, BDMF_IRQF_DISABLED,
                          _fpm_isr_wrapper, "fpm", NULL);
    if (!rc)
        bdmf_int_enable(INTERRUPT_ID_FPM);
    
#if defined(DEBUG_PRINTS)
    runner_print_task = create_runner_print_task();
#endif
#endif

#ifdef CONFIG_MCAST_TASK_LIMIT
    rdd_mcast_min_tasks_limit_cfg(MULTICAST_TASKS_LIMIT_MIN);
    rdd_mcast_max_tasks_limit_cfg(MULTICAST_TASKS_LIMIT_MAX);
#endif

    return 0;
}

#if defined(DEBUG_PRINTS) && !defined(RDP_SIM)
static int runner_print_thread_func(void *thread_data)
{
    /* wake up periodically for every perodicity_ms */
    unsigned long timeout_jiffies = msecs_to_jiffies(rdd_debug_prints_info.perodicity_ms);

    while (!kthread_should_stop()) 
    {
      rdd_debug_prints_handle();
      
      /* TODO: handle many prints */
      if (0) 
      {
        /* if too many prints */
        yield();
      } 
      else 
      {
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(timeout_jiffies);
      }
   }
  return 0;
}


struct task_struct *create_runner_print_task(void)
{
  struct task_struct *tsk;
  /*struct sched_param param;*/


  tsk = kthread_create(runner_print_thread_func, NULL, "runner_print_task");

  if (IS_ERR(tsk)) 
  {
    bdmf_print("runner_print_task creation failed\n");
    return NULL;
  }

  /*param.sched_priority = BCM_RTPRIO_DATA;*/
  /*sched_setscheduler(tsk, SCHED_RR, &param); */
  wake_up_process(tsk);

  bdmf_print("runner_print_task created successfully\n");
  return tsk;
}
#endif

int system_post_init_wan(rdpa_wan_type wan_type, rdpa_emac wan_emac)
{
    int rc;

    if (wan_type == rdpa_wan_gbe)
        rc = system_data_path_init_gbe(wan_emac);
#if defined(BCM_DSL_XRDP)
    else if (wan_type == rdpa_wan_dsl)
        rc = system_data_path_init_dsl();
#endif
    else
        rc = system_data_path_init_fiber(wan_type);
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, system_object, "Failed system data path init fiber rc=%d\n", rc);

    return 0;
}

/*********************************************************************
 * system_read_qm_drop_stat - read QM drop stat and add it to accumulative
 *
 * INPUT PARAMS:
 *     stat - pointer to resulting stat struct
 *
 ********************************************************************/
static int system_read_qm_drop_stat(rdpa_system_stat_t *stat)
{
    drv_qm_drop_cntr_t qm_drop_stat;
    int rc, i;
    rdpa_system_common_stat_t *accumulative_stat;
    uint32_t drop_packets_norm = 0, drop_packets_excl = 0;

    accumulative_stat = &accumulative_system_stat.common;
    /* 1. read qm statistics from HW */
    rc = drv_qm_drop_stat_get(&qm_drop_stat);
    rc = rc ? rc : ag_drv_qm_drop_counter_get(QM_QUEUE_CPU_RX_COPY_NORMAL * 2, &(drop_packets_norm));
    rc = rc ? rc : ag_drv_qm_drop_counter_get(QM_QUEUE_CPU_RX_COPY_EXCLUSIVE * 2, &(drop_packets_excl));
    
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to read QM drop statistics, rc: %d\n", rc);
        memset(&qm_drop_stat, 0, sizeof(qm_drop_stat)); /* in case of error will return */
    }                                                   /* last known good accumulative stats*/

    /* 2. update system common stats */
    rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
        _OFFSET_COMMON_(qm_wred_drop), qm_drop_stat.qm_wred_drop);

    rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
        _OFFSET_COMMON_(qm_fpm_cong), qm_drop_stat.qm_fpm_cong_drop);

    for (i = 0; i < NUM_OF_FPM_UG; i++)
    {
        rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
            _OFFSET_COMMON_(qm_fpm_grp_drop) + i * sizeof(uint32_t), qm_drop_stat.qm_fpm_grp_drop[i]);
    }

    for (i = 0; i < NUM_OF_FPM_POOLS; i++)
    {
        rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
            _OFFSET_COMMON_(qm_fpm_cong_pool_drop) + i * sizeof(uint32_t), qm_drop_stat.qm_fpm_prefetch_drop[i]);
    }

    rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
        _OFFSET_COMMON_(qm_ddr_pd_cong_drop), qm_drop_stat.qm_ddr_pd_cong_drop);
    rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
        _OFFSET_COMMON_(qm_pd_cong_drop), qm_drop_stat.qm_pd_cong_drop);

    rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
        _OFFSET_COMMON_(qm_psram_egress_cong), qm_drop_stat.qm_psram_egress_cong_drop);
        
    rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
        _OFFSET_COMMON_(cpu_rx_qm_queue_drop_norm), drop_packets_norm);
        
    rdpa_common_update_cntr_results_uint32(&(stat->common), accumulative_stat,
        _OFFSET_COMMON_(cpu_rx_qm_queue_drop_excl), drop_packets_excl);
        
    return rc;
}

/* US drop statistics */
struct bdmf_aggr_type system_us_stat_type =
{
    .name = "system_us_stat", .struct_name = "rdpa_system_us_stat_t",
    .help = "System US Drop Statistics", .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        { .name = "protocol_us_ipv4_drop", .help = "IPV4 protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, protocol_us_ipv4)
        },
        { .name = "protocol_us_ipv6_drop", .help = "IPV6 protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, protocol_us_ipv6)
        },
        { .name = "protocol_us_pppoe_drop", .help = "PPPOE protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, protocol_us_pppoe)
        },
        { .name = "protocol_us_non_ip_drop", .help = "None IP protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, protocol_us_non_ip)
        },
        { .name = "mirror_us_no_sbpm_drop", .help = "RX Mirroring: No SBPM drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, mirror_us_no_sbpm)
        },
        { .name = "mirror_us_no_dispatch_drop", .help = "RX Mirroring: No dispatcher token drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_us_stat_t, mirror_us_no_dispatch_token)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(system_us_stat_type);

/* DS drop statistics */
struct bdmf_aggr_type system_ds_stat_type =
{
    .name = "system_ds_stat", .struct_name = "rdpa_system_ds_stat_t",
    .help = "System DS Drop Statistics", .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        { .name = "protocol_ds_ipv4_drop", .help = "IPV4 protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, protocol_ds_ipv4)
        },
        { .name = "protocol_ds_ipv6_drop", .help = "IPV6 protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, protocol_ds_ipv6)
        },
        { .name = "protocol_ds_pppoe_drop", .help = "PPPOE protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, protocol_ds_pppoe)
        },
        { .name = "protocol_ds_non_ip_drop", .help = "None IP protocol filter drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, protocol_ds_non_ip)
        },
        { .name = "mirror_ds_no_sbpm_drop", .help = "RX Mirroring: No SBPM drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, mirror_ds_no_sbpm)
        },
        { .name = "mirror_ds_no_dispatch_drop", .help = "RX Mirroring: No dispatcher token drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_ds_stat_t, mirror_ds_no_dispatch_token)
        },
        BDMF_ATTR_LAST
    },
};

DECLARE_BDMF_AGGREGATE_TYPE(system_ds_stat_type);

/* Common drop statistics */
struct bdmf_aggr_type system_common_stat_type =
{
    .name = "system_common_stat", .struct_name = "rdpa_system_common_stat_t",
    .help = "System Drop Statistics", .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        { .name = "connection_action_drop", .help = "Connection Action Drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, connection_action)
        },
        { .name = "cpu_rx_ring_congestion_drop", .help = "CPU RX Feed Ring congestion drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_rx_ring_congestion)
        },
        { .name = "cpu_rx_qm_queue_drop_norm", .help = "CPU RX QM normal queue congestion drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_rx_qm_queue_drop_norm)
        },        
        { .name = "cpu_rx_qm_queue_drop_excl", .help = "CPU RX QM exclusive queue congestion drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_rx_qm_queue_drop_excl)
        },  		
        { .name = "cpu_recycle_ring_congestion_drop", .help = "CPU Recycle Ring congestion", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_recycle_ring_congestion)
        },
        { .name = "cpu_tx_copy_no_fpm_drop", .help = "Drop due no FPM when CPU TX copy",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_tx_copy_no_fpm)
        },
        { .name = "cpu_tx_copy_no_sbpm_drop", .help = "Drop due no SBPM when CPU TX copy",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_tx_copy_no_sbpm)
        },
        { .name = "eth_flow_drop_action_drop", .help = "Drop due flow drop action",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, flow_drop_action)
        },
        { .name = "rx_mirror_cpu_mcast_exception_drop", .help = "Drop due RX mirroring or CPU/WLAN Multicast exception",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, rx_mirror_cpu_mcast_exception)
        },
        { .name = "cpu_rx_meter_drop", .help = "Counts total cpu rx meters drops",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_rx_meter_drop)
        },
        { .name = "ingress_resources_congestion_drop", .help = "Drop due resources ingress congestion",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, ingress_resources_congestion)
        },
        { .name = "egress_resources_congestion_drop", .help = "Drop due resources egress congestion",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, egress_resources_congestion)
        },
        { .name = "ingress_isolation_drop", .help = "Drop due resources ingress congestion",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, ingress_isolation_drop)
        },
        { .name = "egress_isolation_drop", .help = "Drop due resources egress congestion",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, egress_isolation_drop)
        },
        { .name = "disabled_tx_flow_drop", .help = "TX flow is not defined",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, disabled_tx_flow)
        },
        { .name = "cpu_rx_tc_to_rxq_map_drop", .help = "CPU RX TC to RXQ map drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_rx_tc_to_rxq_map)
        },
        { .name = "map_vport_to_CPU_ring_fail_drop", .help = "CPU RX vport to CPU object map drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_rx_vport_to_cpu_obj_map)
        },
        { .name = "da_lookup_miss_drop", .help = "Bridge ARL miss on DA MAC lookup drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, da_lookup_miss)
        },
        { .name = "sa_lookup_miss_drop", .help = "Bridge ARL miss on SA MAC lookup drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, sa_lookup_miss)
        },
        { .name = "bridge_fw_eligability_drop", .help = "Ingress and egress ports don't belong to the same bridge",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, bridge_fw_eligability)
        },
        { .name = "da_lookup_match_drop", .help = "DA lookup miss no matched entry drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, da_lookup_match_drop)
        },
        { .name = "sa_lookup_match_drop", .help = "SA lookup miss no matched entry drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, sa_lookup_match_drop)
        },
        { .name = "cpu_tx_disabled_q_drop", .help = "Drop due to disabled queue",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, cpu_tx_disabled_q_drop)
        },
#if defined(G9991)
        { .name = "g9991_sof_after_sof_drop", .help = "DPU drop due to SOF after SOF",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, g9991_sof_after_sof)
        },
        { .name = "g9991_mof_eof_without_sof_drop", .help = "DPU drop due to MOF/EOF without SOF",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, g9991_mof_eof_without_sof)
        },
        { .name = "g9991_reassembly_error_drop", .help = "DPU drop due to re-assembly error",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, g9991_reassembly_error)
        },
#endif
        { .name = "ingress_rate_limit_drop", .help = "drop if ingress rate limit exceeded on emac port",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, ingress_rate_limit_drop)
        },
        { .name = "loopback_drop", .help = "drop packets in flow which are not loopbacked ",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, loopback_drop)
        },
        { .name = "undefined_queue_drop", .help = "drop flow control on emac port",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, undefined_queue_drop)
        },
        { .name = "qm_wred_drop", .help = "drops from all queues due to WRED",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, qm_wred_drop)
        },
        { .name = "qm_fpm_cong_pool0_drop", .help = "FPM pool priority thresholds violation, pool size x1",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_system_common_stat_t, qm_fpm_cong_pool_drop),
        },
        { .name = "qm_fpm_cong_pool1_drop", .help = "FPM pool priority thresholds violation, pool size x2",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = (offsetof(rdpa_system_common_stat_t, qm_fpm_cong_pool_drop) + sizeof(uint32_t)),
        },
        { .name = "qm_fpm_cong_pool2_drop", .help = "FPM pool priority thresholds violation, pool size x4",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = (offsetof(rdpa_system_common_stat_t, qm_fpm_cong_pool_drop) + 2 * sizeof(uint32_t)),
        },
        { .name = "qm_fpm_cong_pool3_drop", .help = "FPM pool priority thresholds violation, pool size x8",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = (offsetof(rdpa_system_common_stat_t, qm_fpm_cong_pool_drop) + 3 * sizeof(uint32_t)),
        },
        { .name = "qm_fpm_cong_drop", .help = "Drop due to FPM congestion",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, qm_fpm_cong)
        },
        { .name = "qm_fpm_grp0_drop", .help = "FPM user group priority threshold violation. user group 0",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_system_common_stat_t, qm_fpm_grp_drop)
        },
        { .name = "qm_fpm_grp1_drop", .help = "FPM user group priority threshold violation. user group 1",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = (offsetof(rdpa_system_common_stat_t, qm_fpm_grp_drop) + sizeof(uint32_t))
        },
        { .name = "qm_fpm_grp2_drop", .help = "FPM user group priority threshold violation. user group 2",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = (offsetof(rdpa_system_common_stat_t, qm_fpm_grp_drop) + 2 * sizeof(uint32_t))
        },
        { .name = "qm_fpm_grp3_drop", .help = "FPM user group priority threshold violation. user group 3",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = (offsetof(rdpa_system_common_stat_t, qm_fpm_grp_drop) + 3 * sizeof(uint32_t))
        },
        { .name = "qm_ddr_pd_cong_drop", .help = "Drop due to DDR PD congestion",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, qm_ddr_pd_cong_drop)
        },
        { .name = "qm_pd_cong_drop", .help = "Drop due to PD congestion",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, qm_pd_cong_drop)
        },
        { .name = "qm_psram_egress_cong_drop", .help = "PSSRAM egress congestion drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, qm_psram_egress_cong)
        },
#if !defined(BCM_DSL_XRDP)
        { .name = "tx_mirroring_drop", .help = "TX mirrored packet drop",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, tx_mirroring_drop)
        },
        { .name = "tunnel_no_sbpm_drop", .help = "Drop tunnel US packet due to no SBPM",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_system_common_stat_t, tunnel_no_sbpm_drop)
        },
#endif
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(system_common_stat_type);

typedef enum
{
    system_us_cntr,
    system_ds_cntr,
    system_common_cntr,
    system_debug_cntr,
    system_last_element
} system_cntr_type_enum_t;

typedef struct
{
    uint32_t counter_id;
    uint32_t offset;
    system_cntr_type_enum_t type;
} rdpa_cntrs_and_offsets_t;

static rdpa_cntrs_and_offsets_t stat_cntrs_and_offsets[] =
{
    {COUNTER_DROP_CONNECTION_ACTION_DROP_ID, offsetof(rdpa_system_common_stat_t, connection_action), system_common_cntr},
    {COUNTER_CPU_RX_FEED_RING_CONGESTION, offsetof(rdpa_system_common_stat_t, cpu_rx_ring_congestion), system_common_cntr},
    {COUNTER_CPU_RECYCLE_RING_CONGESTION, offsetof(rdpa_system_common_stat_t, cpu_recycle_ring_congestion), system_common_cntr},
    {COUNTER_CPU_TX_COPY_NO_FPM, offsetof(rdpa_system_common_stat_t, cpu_tx_copy_no_fpm), system_common_cntr},
    {COUNTER_CPU_TX_COPY_NO_SBPM, offsetof(rdpa_system_common_stat_t, cpu_tx_copy_no_sbpm), system_common_cntr},
    {COUNTER_ETHERNET_FLOW_DROP_ACTION, offsetof(rdpa_system_common_stat_t, flow_drop_action), system_common_cntr},
    {COUNTER_SBPM_ALLOC_EXCEPTION_DROP, offsetof(rdpa_system_common_stat_t, rx_mirror_cpu_mcast_exception), system_common_cntr},
    {COUNTER_DROP_RESOURCE_CONGESTION_INGRESS, offsetof(rdpa_system_common_stat_t, ingress_resources_congestion), system_common_cntr},
    {COUNTER_DROP_RESOURCE_CONGESTION_EGRESS, offsetof(rdpa_system_common_stat_t, egress_resources_congestion), system_common_cntr},
    {COUNTER_INGRESS_ISOLATION_DROP, offsetof(rdpa_system_common_stat_t, ingress_isolation_drop), system_common_cntr},
    {COUNTER_EGRESS_ISOLATION_DROP, offsetof(rdpa_system_common_stat_t, egress_isolation_drop), system_common_cntr},
    {COUNTER_DISABLED_TX_FLOW_DROP, offsetof(rdpa_system_common_stat_t, disabled_tx_flow), system_common_cntr},
    {COUNTER_CPU_RX_TC_TO_RXQ_MAP_DROP, offsetof(rdpa_system_common_stat_t, cpu_rx_tc_to_rxq_map), system_common_cntr},
    {COUNTER_CPU_RX_METER_DROP, offsetof(rdpa_system_common_stat_t, cpu_rx_meter_drop), system_common_cntr},
    {COUNTER_CPU_RX_VPORT_TO_CPU_OBJ_MAP_DROP, offsetof(rdpa_system_common_stat_t, cpu_rx_vport_to_cpu_obj_map), system_common_cntr},
    {COUNTER_DA_LKP_MISS_DROP, offsetof(rdpa_system_common_stat_t, da_lookup_miss), system_common_cntr},
    {COUNTER_SA_LKP_MISS_DROP, offsetof(rdpa_system_common_stat_t, sa_lookup_miss), system_common_cntr},
    {COUNTER_BRIDGE_FW_ELIGABILITY_DROP, offsetof(rdpa_system_common_stat_t, bridge_fw_eligability), system_common_cntr},
    {COUNTER_DA_LKP_MATCH_DROP, offsetof(rdpa_system_common_stat_t, da_lookup_match_drop), system_common_cntr},
    {COUNTER_SA_LKP_MATCH_DROP, offsetof(rdpa_system_common_stat_t, sa_lookup_match_drop), system_common_cntr},
    {COUNTER_CPU_TX_DISABLED_QUEUE_DROP, offsetof(rdpa_system_common_stat_t, cpu_tx_disabled_q_drop), system_common_cntr},
#if defined(G9991)
    {COUNTER_G9991_SOF_AFTER_SOF, offsetof(rdpa_system_common_stat_t, g9991_sof_after_sof), system_common_cntr},
    {COUNTER_G9991_MOF_EOF_WITHOUT_SOF, offsetof(rdpa_system_common_stat_t, g9991_mof_eof_without_sof), system_common_cntr},
    {COUNTER_G9991_REASSEMBLY_ERROR, offsetof(rdpa_system_common_stat_t, g9991_reassembly_error), system_common_cntr},
#endif
#if !defined(BCM_DSL_XRDP)
    {COUNTER_INGRESS_RATE_LIMIT_DROP, offsetof(rdpa_system_common_stat_t, ingress_rate_limit_drop), system_common_cntr},
    {COUNTER_LOOPBACK_DROP, offsetof(rdpa_system_common_stat_t, loopback_drop), system_common_cntr},
    {COUNTER_UNDEFINED_QUEUE_DROP, offsetof(rdpa_system_common_stat_t, undefined_queue_drop), system_common_cntr},
    {COUNTER_TX_MIRROR_DROP, offsetof(rdpa_system_common_stat_t, tx_mirroring_drop), system_common_cntr},
    {COUNTER_TUNNEL_NO_SBPM_DROP, offsetof(rdpa_system_common_stat_t, tunnel_no_sbpm_drop), system_common_cntr},
#endif
    {COUNTER_DIS_PROTO_US_IPV4, offsetof(rdpa_system_us_stat_t, protocol_us_ipv4), system_us_cntr},
    {COUNTER_DIS_PROTO_US_IPV6, offsetof(rdpa_system_us_stat_t, protocol_us_ipv6), system_us_cntr},
    {COUNTER_DIS_PROTO_US_PPPOE, offsetof(rdpa_system_us_stat_t, protocol_us_pppoe), system_us_cntr},
    {COUNTER_DIS_PROTO_US_NON_IP, offsetof(rdpa_system_us_stat_t, protocol_us_non_ip), system_us_cntr},
    {COUNTER_US_MIRROR_NO_SBPM_DROP, offsetof(rdpa_system_us_stat_t, mirror_us_no_sbpm), system_us_cntr},
    {COUNTER_US_MIRROR_NO_TOKEN_DROP, offsetof(rdpa_system_us_stat_t, mirror_us_no_dispatch_token), system_us_cntr},
    {COUNTER_DIS_PROTO_DS_IPV4, offsetof(rdpa_system_ds_stat_t, protocol_ds_ipv4), system_ds_cntr},
    {COUNTER_DIS_PROTO_DS_IPV6, offsetof(rdpa_system_ds_stat_t, protocol_ds_ipv6), system_ds_cntr},
    {COUNTER_DIS_PROTO_DS_PPPOE, offsetof(rdpa_system_ds_stat_t, protocol_ds_pppoe), system_ds_cntr},
    {COUNTER_DIS_PROTO_DS_NON_IP, offsetof(rdpa_system_ds_stat_t, protocol_ds_non_ip), system_ds_cntr},
    {COUNTER_DS_MIRROR_NO_SBPM_DROP, offsetof(rdpa_system_ds_stat_t, mirror_ds_no_sbpm), system_ds_cntr},
    {COUNTER_DS_MIRROR_NO_TOKEN_DROP, offsetof(rdpa_system_ds_stat_t, mirror_ds_no_dispatch_token), system_ds_cntr},
    {0, 0, system_last_element}
};

static inline int single_cntr_get_and_update_stat(uint32_t counter_id, void *val, uint32_t offset_in_struct, system_cntr_type_enum_t system_type)
{
    uint16_t cntr_val_16bit;
    uint32_t cntr_result;
    rdpa_system_stat_t *stat;
    int rc;

    rc = drv_cntr_various_counter_get(counter_id, &cntr_val_16bit);
    if (rc)
        return rc;

    cntr_result = (uint32_t)cntr_val_16bit;

    if (system_type == system_common_cntr)
    {
        stat = (rdpa_system_stat_t *)val;
        rdpa_common_update_cntr_results_uint32(&(stat->common), &accumulative_system_stat.common, offset_in_struct, cntr_result);
    }
    else if (system_type == system_us_cntr)
    {
        stat = (rdpa_system_stat_t *)val;
        rdpa_common_update_cntr_results_uint32(&(stat->us), &accumulative_system_stat.us, offset_in_struct, cntr_result);
    }
    else if (system_type == system_ds_cntr)
    {
        stat = (rdpa_system_stat_t *)val;
        rdpa_common_update_cntr_results_uint32(&(stat->ds), &accumulative_system_stat.ds, offset_in_struct, cntr_result);
    }
    else if (system_type == system_debug_cntr)
        rdpa_common_update_cntr_results_uint32(val, &accumulative_debug_stat, offset_in_struct, cntr_result);
    else
        return BDMF_ERR_PARM;

    return rc;
}

int system_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    int rc = 0, idx = 0;

    memset(val, 0, sizeof(rdpa_system_stat_t));

    while (!rc && stat_cntrs_and_offsets[idx].type != system_last_element)
    {
        rc = single_cntr_get_and_update_stat(stat_cntrs_and_offsets[idx].counter_id, val,
            stat_cntrs_and_offsets[idx].offset, stat_cntrs_and_offsets[idx].type);
        idx++;
    }

    /* fill QM drop counters in common struct of rdpa_system_common_stat_t type*/
    system_read_qm_drop_stat((rdpa_system_stat_t *)val);
    return rc;
}

int system_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    memset(&accumulative_system_stat , 0, sizeof(rdpa_system_stat_t));

    return BDMF_ERR_OK;
}

int system_attr_debug_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    memset(&accumulative_debug_stat , 0, sizeof(rdpa_debug_stat_t));

    return BDMF_ERR_OK;
}

rdpa_cntrs_and_offsets_t debug_cntrs_and_offsets[] =
{
    {COUNTER_TM_PD_NOT_VALID_ID, offsetof(rdpa_debug_stat_t, tm_pd_not_valid_id), system_debug_cntr},
    {COUNTER_TM_ACTION_NOT_VALID_ID, offsetof(rdpa_debug_stat_t, tm_action_not_valid_id), system_debug_cntr},
    {COUNTER_EPON_TM_PD_NOT_VALID_ID, offsetof(rdpa_debug_stat_t, epon_tm_pd_not_valid_id), system_debug_cntr},
#if defined(G9991)
    {COUNTER_G9991_TM_PD_NOT_VALID_ID, offsetof(rdpa_debug_stat_t, g9991_tm_pd_not_valid_id), system_debug_cntr},
#endif
    {COUNTER_PROCESSING_ACTION_NOT_VALID_ID, offsetof(rdpa_debug_stat_t, processing_action_not_valid_id), system_debug_cntr},
    {COUNTER_SBPM_LIB_DISP_CONG, offsetof(rdpa_debug_stat_t, sbpm_lib_disp_cong), system_debug_cntr},
    {COUNTER_BRIDGE_FLOODING, offsetof(rdpa_debug_stat_t, bridge_flooding), system_debug_cntr},
    {COUNTER_INGRESS_CONGESTION_LAN, offsetof(rdpa_debug_stat_t, ingress_congestion_flow_cntr_lan), system_debug_cntr},
    {0, 0, system_last_element}
};

int system_attr_debug_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    rdpa_debug_stat_t *stat = (rdpa_debug_stat_t *)val;
    int rc = 0, idx = 0;

    memset(stat, 0, sizeof(rdpa_debug_stat_t));

    while (!rc && debug_cntrs_and_offsets[idx].type != system_last_element)
    {
        rc = single_cntr_get_and_update_stat(debug_cntrs_and_offsets[idx].counter_id, stat,
                debug_cntrs_and_offsets[idx].offset, debug_cntrs_and_offsets[idx].type);
        idx++;
    }

    return rc;
}

int system_attr_tod_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    rdpa_system_tod_t *system_tod = (rdpa_system_tod_t *)val;
    rdpa_wan_type wan_type = rdpa_wan_if_to_wan_type(rdpa_wan_type_to_if(rdpa_wan_gpon));
#ifdef CONFIG_BCM_GPON_TODD
    gpon_todd_tstamp_t tstamp;
    uint64_t ts;
#endif

    memset(system_tod, 0, sizeof(rdpa_system_tod_t));

    if (wan_type == rdpa_wan_gpon || wan_type == rdpa_wan_xgpon)
    {
#ifdef CONFIG_BCM_GPON_TODD
        gpon_todd_get_tod(&tstamp, &ts);
        system_tod->sec_ms = tstamp.sec_ms;
        system_tod->sec_ls = tstamp.sec_ls;
        system_tod->nsec = tstamp.nsec;
        system_tod->ts48_nsec = ts;
#endif

        return 0;
    }

    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_cpu_reason_to_tc_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    rdpa_cpu_reason reason = (rdpa_cpu_reason)index;
    uint8_t *tc = (uint8_t *)val;

    *tc = cpu_reason_to_tc[reason];
    return 0;
}

int system_attr_cpu_reason_to_tc_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    rdpa_cpu_reason reason = (rdpa_cpu_reason)index;
    uint8_t tc = *(uint8_t *)val;

    if (tc > 7)
        return BDMF_ERR_PARM;
    cpu_reason_to_tc[reason] = tc;
    rdd_ag_cpu_rx_cpu_reason_to_tc_set(reason, cpu_reason_to_tc[reason]);
    return 0;
}

/* "counter_cfg" attribute "write" callback */
int system_counter_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_counter_cfg_t *counter_cfg = (rdpa_counter_cfg_t *)val;

    if (mo->state == bdmf_state_active)
       return BDMF_ERR_NOT_SUPPORTED;

    /* Save configuration */
    system->counter_cfg.vlan_stats_enable = counter_cfg->vlan_stats_enable;
    system->counter_cfg.shared_counters = PROJ_DEFS_NUMBER_OF_SHARED_IC_VLAN_COUNTERS - ((int)system->counter_cfg.vlan_stats_enable * (PROJ_DEFS_NUMBER_OF_VLANS * 2));
    return 0;
}

int system_attr_fpm_isr_delay_timer_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    *(uint32_t *)val = fpm_isr_delay_timer_period;
    return 0;
}

int system_attr_fpm_isr_delay_timer_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    fpm_isr_delay_timer_period = *(uint32_t *)val;
    return 0;
}

int system_attr_natc_counter_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    rdpa_natc_cntr_t *natc_cntr = (rdpa_natc_cntr_t *)val;
    natc_ctrs_natc_ctrs rdd_natc_ctrs;
    bdmf_error_t rc = BDMF_ERR_OK;
#if !defined(NATC_UNIFIED_COUNTERS)
    rc = ag_drv_natc_ctrs_natc_ctrs_get(index, &rdd_natc_ctrs);
#else
    /* in dual issue it is the same for all indexes */
    rc = ag_drv_natc_ctrs_natc_ctrs_get(&rdd_natc_ctrs);
#endif
    if (rc)
        return rc;

    natc_cntr->cache_hit_count = rdd_natc_ctrs.cache_hit_count;
    natc_cntr->cache_miss_count = rdd_natc_ctrs.cache_miss_count;
    natc_cntr->ddr_request_count = rdd_natc_ctrs.ddr_request_count;
    natc_cntr->ddr_evict_count = rdd_natc_ctrs.ddr_evict_count;
    natc_cntr->ddr_block_count = rdd_natc_ctrs.ddr_block_count;

    return rc;
}

int system_attr_natc_counter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    rdpa_natc_cntr_t *natc_cntr = (rdpa_natc_cntr_t *)val;
    natc_ctrs_natc_ctrs rdd_natc_ctrs;

    rdd_natc_ctrs.cache_hit_count = natc_cntr->cache_hit_count;
    rdd_natc_ctrs.cache_miss_count = natc_cntr->cache_miss_count;
    rdd_natc_ctrs.ddr_request_count = natc_cntr->ddr_request_count;
    rdd_natc_ctrs.ddr_evict_count = natc_cntr->ddr_evict_count;
    rdd_natc_ctrs.ddr_block_count = natc_cntr->ddr_block_count;
#if !defined(NATC_UNIFIED_COUNTERS)
    return ag_drv_natc_ctrs_natc_ctrs_set(index, (const natc_ctrs_natc_ctrs *)&rdd_natc_ctrs);
#else
    return ag_drv_natc_ctrs_natc_ctrs_set((const natc_ctrs_natc_ctrs *)&rdd_natc_ctrs);
#endif
}

int system_attr_ih_cong_threshold_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_ih_cong_threshold_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int system_attr_ing_cong_ctrl_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
#ifdef BCM_DSL_XRDP
    *(bdmf_boolean *)val = rdd_system_congestion_ctrl_get();
    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int system_attr_ing_cong_ctrl_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
#ifdef BCM_DSL_XRDP
    rdd_system_congestion_ctrl_enable(*(bdmf_boolean *)val);
    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

static int2int_map_t tpid_val_map[] =
{
    { 0, 1 }, /* User-defined #1, re-read from system */
    { 0, 2 }, /* User-defined #2, re-read from system */
    { 0x8100, 3 },
    { 0x88a8, 4 },
    { 0x9100, 5 },
    { 0x9200, 6 },
    { -1, -1 },
};

int rdpa_system_tpid_idx_get(uint16_t tpid_val)
{
    system_drv_priv_t *system = (system_drv_priv_t *)bdmf_obj_data(system_object);
    int tpid_idx;

    /* First, update udef_0 and udef_1 from system object */
    tpid_val_map[0].src = system->tpids_detect[rdpa_tpid_detect_udef_1].val_udef;
    tpid_val_map[1].src = system->tpids_detect[rdpa_tpid_detect_udef_2].val_udef;

    tpid_idx = int2int_map(tpid_val_map, tpid_val, -1);

    /* If not found, rdpa_tpid_detect__num_of can be used as un-configured TPID */
    RDD_BTRACE("tpid_val 0x%x, tpid_idx %d\n", tpid_val, tpid_idx);
    return tpid_idx; 
}

uint16_t rdpa_system_tpid_get_by_idx(int tpid_idx)
{
    uint16_t tpid_val;

    tpid_val = (uint16_t)int2int_map_r(tpid_val_map, tpid_idx, -1);

    RDD_BTRACE("tpid_idx %d, tpid_val 0x%x\n", tpid_idx, tpid_val);
    return tpid_val;
}

int _packet_buffer_cfg(const rdpa_packet_buffer_cfg_t *pb_cfg)
{
    rdpa_system_set_global_token_allocation(pb_cfg);
    
    return set_fpm_budget(FPM_RES_NONE, 0, 0);
}

