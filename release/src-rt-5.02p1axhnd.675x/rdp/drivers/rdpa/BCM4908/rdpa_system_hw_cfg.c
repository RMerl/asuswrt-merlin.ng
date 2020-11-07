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
#include "rdpa_rdd_map.h"
#include "rdpa_system.h"
#include "rdpa_cpu.h"
#include "rdp_drv_sbpm.h"
#include "rdd.h"
#include "rdp_drv_bbh.h"
#include "rdp_drv_sbpm.h"
#include "rdpa_rdd_inline.h"
#ifndef BDMF_SYSTEM_SIM
#include "clk_rst.h"
#ifdef __KERNEL__
#include "fpm.h"
#endif
#endif

#define usleep_range udelay
#define RDPA_PCI_PORT_MAIN_INTERRUPT_NUM_IN_RDD  1
#define RDPA_PCI_PORT_SUB_INTERRUPT_NUM_IN_RDD   0
#define RDPA_CPU_RX_QUEUES_MAIN_INTERRUPT_NUM_IN_RDD 0

#ifndef BDMF_SYSTEM_SIM
extern int runner_reserved_memory_get(uint8_t **bm_base_addr,
                                      uint8_t **bm_base_addr_phys,
                                      unsigned int *bm_size,
                                      uint8_t **fm_base_addr,
                                      uint8_t **fm_base_addr_phys,
                                      unsigned int *fm_size);
#endif

#if defined(__KERNEL__) && !defined(BDMF_SYSTEM_SIM)
extern void fpm_get_hw_info(struct fpm_hw_info *);
#endif

static data_path_init_params bcm4908_dpi_cfg;

extern int emac_ports_headroom_hw_cfg(int headroom_size);
extern int emac_ports_mtu_hw_cfg(int mtu_size);

static void f_initialize_runner_parameters(void)
{
    uint32_t i, j;

    /* General */

    /* mask the CPU Rx queues interrupts in the RDD */
    for (i = 0; i < RDPA_CPU_MAX_QUEUES; i++)
        rdd_interrupt_mask(0, RDPA_CPU_RX_QUEUES_MAIN_INTERRUPT_NUM_IN_RDD);

    /* mask the PCI bridge port interrupt in the RDD */
    rdd_interrupt_mask(RDPA_PCI_PORT_MAIN_INTERRUPT_NUM_IN_RDD, RDPA_PCI_PORT_SUB_INTERRUPT_NUM_IN_RDD);

    /* Ethernet WAN RDD scheduling and US rate limiter configuration */
    rdd_wan_channel_cfg(RDD_WAN_CHANNEL_0, RDD_WAN_CHANNEL_SCHEDULE_PRIORITY, RDD_PEAK_SCHEDULE_MODE_ROUND_ROBIN);
    rdd_wan_channel_rate_limiter_cfg(RDD_WAN_CHANNEL_0, 0/*disabled*/, rdpa_tm_orl_prty_low);
    rdd_us_wan_flow_cfg(GBE_WAN_FLOW_ID, RDD_WAN_CHANNEL_0, 0, 0, 1/*crc calc enable*/, 0, 0, 0);

    /* Default configuration of all RDD reasons to CPU_RX_QUEUE_ID_0 */
    for (i = 0; i < rdpa_cpu_reason__num_of; i++)
    {
        rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_us);
        rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_ds);
    }

    /* Initialize Ethernet priority queues */
    for (i = 0; i < 8; i++)
    {
        /* (-1) TBD : when runner will support EMAC_5 */
        for (j = RDD_LAN0_VPORT; j < RDD_LAN_VPORT_LAST; j++)
        {
            /* Configure queue size in RDD */
            rdd_lan_vport_tx_queue_cfg(j, i, 0, RDD_QUEUE_PROFILE_DISABLED);
        }
    }

    /* SF2 mapping */
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN0_BRIDGE_PORT, 0);
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN1_BRIDGE_PORT, 1);
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN2_BRIDGE_PORT, 2);
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN3_BRIDGE_PORT, 3);
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_LAN4_BRIDGE_PORT, 7);
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT, 4); /* IMP-4 to RDD_VIRTUAL mapping */
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT, 5); /* IMP-5 to RDD_VIRTUAL mapping */
    rdd_broadcom_switch_ports_mapping_table_config(BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT, 8); /* IMP-8 to RDD_VIRTUAL mapping */

    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_WAN1_BRIDGE_PORT, BL_LILAC_RDD_WAN1_BRIDGE_PORT);
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN0_BRIDGE_PORT, BL_LILAC_RDD_LAN0_BRIDGE_PORT);
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN1_BRIDGE_PORT, BL_LILAC_RDD_LAN1_BRIDGE_PORT);
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN2_BRIDGE_PORT, BL_LILAC_RDD_LAN2_BRIDGE_PORT);
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN3_BRIDGE_PORT, BL_LILAC_RDD_LAN3_BRIDGE_PORT);
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_LAN4_BRIDGE_PORT, BL_LILAC_RDD_LAN4_BRIDGE_PORT);
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT, BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT); /* IMP-4,5,8 to RDD_VIRTUAL mapping */
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_ANY_BRIDGE_PORT, BL_LILAC_RDD_ANY_BRIDGE_PORT);
    rdd_lookup_ports_mapping_table_init(BL_LILAC_RDD_PCI_BRIDGE_PORT, BL_LILAC_RDD_PCI_BRIDGE_PORT);
}

int system_data_path_init(void)
{
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();

#ifndef BDMF_SYSTEM_SIM
    struct fpm_hw_info fpm_info;

    if (get_rdp_freq(&bcm4908_dpi_cfg.runner_freq))
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed to get runner freq from clk&rst\n");

#ifdef RUNNER_FWTRACE
    bcm4908_dpi_cfg.runner_freq = TIMER_PERIOD_REG_VALUE;
#endif

    runner_reserved_memory_get(
        &bcm4908_dpi_cfg.rdp_ddr_bm_base, 
        &bcm4908_dpi_cfg.rdp_ddr_bm_phys, 
        &bcm4908_dpi_cfg.runner_ddr_bm_size, 
        &bcm4908_dpi_cfg.rdp_ddr_fm_base, 
        &bcm4908_dpi_cfg.rdp_ddr_fm_phys, 
        &bcm4908_dpi_cfg.runner_ddr_fm_size);

#else
    bcm4908_dpi_cfg.rdp_ddr_bm_base = (uint8_t *)0x00200000;
    bcm4908_dpi_cfg.rdp_ddr_bm_phys = (uint8_t *)0x00200000;
    bcm4908_dpi_cfg.runner_ddr_bm_size = 0x01000000;
    bcm4908_dpi_cfg.rdp_ddr_fm_base = (uint8_t *)0x01200000;
    bcm4908_dpi_cfg.rdp_ddr_fm_phys = (uint8_t *)0x01200000;
    bcm4908_dpi_cfg.runner_ddr_fm_size = 0x00400000;
#endif

    bcm4908_dpi_cfg.mtu_size = system_cfg->mtu_size;
    bcm4908_dpi_cfg.headroom_size = system_cfg->headroom_size;
#if defined(__KERNEL__) && !defined(BDMF_SYSTEM_SIM)
    fpm_get_hw_info(&fpm_info);
    if (fpm_info.chunk_size == 512)
        bcm4908_dpi_cfg.fpm_buff_size_set = DRV_BBH_FPM_BUFF_SIZE_512_1024_2048_4056;
    else
        bcm4908_dpi_cfg.fpm_buff_size_set = DRV_BBH_FPM_BUFF_SIZE_256_512_1024_2048;
#else
    bcm4908_dpi_cfg.fpm_buff_size_set = DRV_BBH_FPM_BUFF_SIZE_256_512_1024_2048;
#endif

#ifndef BDMF_SYSTEM_SIM
    usleep_range(50);

    /* first we call the basic data path initialization */
    if (data_path_init(&bcm4908_dpi_cfg) != 0)
        return -1;
#else
    /* first we call the basic data path initialization */
    if (data_path_init_sim(&bcm4908_dpi_cfg) != 0)
        return -1;
#endif

    rdd_ddr_headroom_size_config(system_cfg->headroom_size);
    f_initialize_runner_parameters();

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
    rc = rc ? rc : rdd_mtu_cfg(mtu_size);

    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set MTU size to RDD, error %d\n", rc);

    return rc;
}

rdpa_bpm_buffer_size_t rdpa_bpm_buffer_size_get(void)
{
    if (bcm4908_dpi_cfg.fpm_buff_size_set == DRV_BBH_FPM_BUFF_SIZE_512_1024_2048_4056)
        return 512;
    else
        return 256;
}
 
