/*
 * <:copyright-BRCM:2013:proprietary:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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
#include "rdpa_platform.h"
#include "data_path_init.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#ifndef LEGACY_RDP
#include "rdpa_rdd_map.h"
#else
#include "rdpa_rdd_map_legacy.h"
#endif
#include "rdpa_system.h"
#include "rdpa_cpu.h"
#include "rdp_drv_sbpm.h"
#include "rdd_init.h"
#include "rdp_drv_bbh.h"
#include "rdp_drv_sbpm.h"
#include "rdpa_rdd_inline.h"
#include "rdd_data_structures.h"
#ifndef BDMF_SYSTEM_SIM
#include "clk_rst.h"
#endif

#define usleep_range udelay
#define RDPA_PCI_PORT_MAIN_INTERRUPT_NUM_IN_RDD  1
#define RDPA_PCI_PORT_SUB_INTERRUPT_NUM_IN_RDD   0
#define RDPA_CPU_RX_QUEUES_MAIN_INTERRUPT_NUM_IN_RDD 0

#ifndef BDMF_SYSTEM_SIM
extern int rdp_post_init(void);
extern int runner_reserved_memory_get(uint32_t *tm_base_addr, uint32_t *tm_base_addr_phys,
    uint32_t *mc_base_addr, uint32_t *mc_base_addr_phys, uint32_t *tm_size, uint32_t *mc_size);
#endif

static S_DPI_CFG bcm63138_dpi_cfg;

extern int emac_ports_headroom_hw_cfg(int headroom_size);
extern int emac_ports_mtu_hw_cfg(int mtu_size);

static void f_initialize_runner_parameters(void)
{
    uint32_t i, j;
    const rdpa_system_init_cfg_t *init_cfg = _rdpa_system_init_cfg_get();
    RDD_RATE_LIMIT_PARAMS rdd_budget = {};

    /* General */
    /* mask the CPU Rx queues interrupts in the RDD */
    for (i = 0; i < RDPA_CPU_MAX_QUEUES; i++)
        rdd_interrupt_mask(0, RDPA_CPU_RX_QUEUES_MAIN_INTERRUPT_NUM_IN_RDD);

    /* mask the PCI bridge port interrupt in the RDD */
    rdd_interrupt_mask(RDPA_PCI_PORT_MAIN_INTERRUPT_NUM_IN_RDD, RDPA_PCI_PORT_SUB_INTERRUPT_NUM_IN_RDD);

    /* DSL */
    for (i = RDD_WAN0_CHANNEL_BASE; i < RDD_WAN0_CHANNEL_BASE + RDPA_MAX_XTMCHANNEL; i++)
    {
        /* Configure RDD WAN scheduling */
        rdd_wan_channel_set(i, RDD_WAN_CHANNEL_SCHEDULE_PRIORITY, RDD_US_PEAK_SCHEDULING_MODE_ROUND_ROBIN);

        /* Set the overall US rate limiter to unlimited */
        rdd_wan_channel_rate_limiter_config((RDD_WAN_CHANNEL_ID)i, BL_LILAC_RDD_RATE_LIMITER_DISABLE, BL_LILAC_RDD_RATE_LIMITER_LOW);
    }

    /* Ethernet WAN RDD scheduling and US rate limiter configuration */
    rdd_wan_channel_set(RDD_WAN_CHANNEL_0, RDD_WAN_CHANNEL_SCHEDULE_PRIORITY, RDD_US_PEAK_SCHEDULING_MODE_ROUND_ROBIN);
    rdd_wan_channel_rate_limiter_config(RDD_WAN_CHANNEL_0, BL_LILAC_RDD_RATE_LIMITER_DISABLE, BL_LILAC_RDD_RATE_LIMITER_LOW);
    rdd_us_wan_flow_config(GBE_WAN_FLOW_ID, RDD_WAN_CHANNEL_0, 0, 0, BL_LILAC_RDD_CRC_CALC_ENABLE, 0, 0, 0);

    /* Default configuration of all RDD reasons to CPU_RX_QUEUE_ID_0 */
    for (i = 0; i < rdpa_cpu_reason__num_of; i++)
    {
        rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_us, CPU_REASON_LAN_TABLE_INDEX);
        rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_ds, CPU_REASON_WAN0_TABLE_INDEX);
        rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_ds, CPU_REASON_WAN1_TABLE_INDEX);
    }

    /* Initialize Ethernet priority queues */
    for (i = 0; i < 8; i++)
    {
        /* (-1) TBD : when runner will support EMAC_5 */
        for (j = BL_LILAC_RDD_EMAC_ID_0; j < BL_LILAC_RDD_EMAC_ID_4; j++)
        {
            /* Configure queue size in RDD */
            rdd_eth_tx_queue_config(j, i, 0, rdd_queue_profile_disabled, INVALID_COUNTER_ID);
        }
    }

    /* Disable default policer ( Importent in in flow cache mode ) */
    rdd_policer_config(rdpa_dir_ds, -1, &rdd_budget);

    /* SF2 mapping */
    if (init_cfg->runner_ext_sw_cfg.enabled)
    {
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN0_BRIDGE_PORT, 0);
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN1_BRIDGE_PORT, 1);
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN2_BRIDGE_PORT, 2);
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN3_BRIDGE_PORT, 3);
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN4_BRIDGE_PORT, 4);
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN5_BRIDGE_PORT, 5);
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN6_BRIDGE_PORT, 7);
        rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT, 8); /* IMP-8 to RDD_VIRTUAL mapping */

        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_WAN0_BRIDGE_PORT, BL_LILAC_RDD_WAN0_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_WAN1_BRIDGE_PORT, BL_LILAC_RDD_WAN1_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN0_BRIDGE_PORT, BL_LILAC_RDD_LAN0_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN1_BRIDGE_PORT, BL_LILAC_RDD_LAN1_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN2_BRIDGE_PORT, BL_LILAC_RDD_LAN2_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN3_BRIDGE_PORT, BL_LILAC_RDD_LAN3_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN4_BRIDGE_PORT, BL_LILAC_RDD_LAN4_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN5_BRIDGE_PORT, BL_LILAC_RDD_LAN5_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN6_BRIDGE_PORT, BL_LILAC_RDD_LAN6_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT, BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT); /* IMP-8 to RDD_VIRTUAL mapping */
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_ANY_BRIDGE_PORT, BL_LILAC_RDD_ANY_BRIDGE_PORT);
        rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_PCI_BRIDGE_PORT, BL_LILAC_RDD_PCI_BRIDGE_PORT);
    }
}


static void f_configure_bpm_sbpm_sp_enable(void)
{
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_RNR_A, DRV_BPM_ENABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_RNR_B, DRV_BPM_ENABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_EMAC0, DRV_BPM_ENABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_EMAC1, DRV_BPM_ENABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_GPON, DRV_BPM_ENABLE);
}

#ifndef BDMF_SYSTEM_SIM
extern unsigned int UtilGetChipIsLP(void);
#endif
 
int system_data_path_init(void)
{
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();

    data_path_shutdown();

#ifndef BDMF_SYSTEM_SIM
    bcm63138_dpi_cfg.runner_lp = UtilGetChipIsLP();

    if (bcm63138_dpi_cfg.runner_lp)
        bdmf_trace("Low Performance Platform\n");

    if (get_rdp_freq(&bcm63138_dpi_cfg.runner_freq))
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed to get runner freq from clk&rst\n");

#ifdef RUNNER_FWTRACE
    bcm63138_dpi_cfg.runner_freq = TIMER_PERIOD_REG_VALUE;
#endif

    runner_reserved_memory_get(
        &bcm63138_dpi_cfg.runner_tm_base_addr, &bcm63138_dpi_cfg.runner_tm_base_addr_phys,
        &bcm63138_dpi_cfg.runner_mc_base_addr, &bcm63138_dpi_cfg.runner_mc_base_addr_phys,
        &bcm63138_dpi_cfg.runner_tm_size, &bcm63138_dpi_cfg.runner_mc_size);
#else
    bcm63138_dpi_cfg.runner_tm_base_addr = 0x00200000;
    bcm63138_dpi_cfg.runner_tm_base_addr_phys = 0x00200000;
    bcm63138_dpi_cfg.runner_tm_size = 24;
    bcm63138_dpi_cfg.runner_mc_base_addr = 0x01600000;
    bcm63138_dpi_cfg.runner_mc_base_addr_phys = 0x01600000;
    bcm63138_dpi_cfg.runner_mc_size = 4;
#endif

    bcm63138_dpi_cfg.mtu_size = system_cfg->mtu_size;
    bcm63138_dpi_cfg.headroom_size = system_cfg->headroom_size;

#ifndef BDMF_SYSTEM_SIM
    usleep_range(50);
#endif

    /* first we call the basic data path initialization */
    data_path_init(&bcm63138_dpi_cfg);
    f_initialize_runner_parameters();
    data_path_go();
    rdd_ddr_headroom_size_config(system_cfg->headroom_size);
    f_configure_bpm_sbpm_sp_enable();
#ifndef BDMF_SYSTEM_SIM
    rdp_post_init();
#endif

    return 0;
}

/* Update EMAC and BBH Frame length */
int headroom_hw_cfg(int headroom_size)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = emac_ports_headroom_hw_cfg(headroom_size);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set headroom size to RDD, error %d\n", rc);

    return rc;
}

int mtu_hw_cfg(int mtu_size)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = emac_ports_mtu_hw_cfg(mtu_size);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set MTU size to RDD, error %d\n", rc);

    return rc;
}

rdpa_bpm_buffer_size_t rdpa_bpm_buffer_size_get(void)
{
    return LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE;
}
 
