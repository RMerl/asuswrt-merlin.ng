/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#include "rdd.h"
#include "rdd_common.h"
#include "rdd_init.h"
#include "rdd_proj_init.h"
#include "rdd_rx_dispatch.h"
#include "rdd_tm.h"
#include "rdd_cpu.h"

#if defined(FIRMWARE_INIT)
#include "rdd_simulator.h"

extern uint8_t *soc_base_address;
extern uint32_t natc_lkp_table_ptr;
extern uint32_t g_context_table_ptr;
#endif

extern uint32_t g_runner_ddr0_base_addr;
extern uint32_t g_runner_ddr1_base_addr;
extern uint32_t g_runner_nat_cache_key_ptr;
extern uint32_t g_runner_nat_cache_context_ptr;
extern uint32_t g_runner_ddr_phy_iptv_tables_base_ptr;
extern uint32_t g_runner_ddr0_iptv_lookup_ptr;
extern uint32_t g_runner_ddr0_iptv_context_ptr;
extern uint32_t g_runner_ddr0_iptv_ssm_context_ptr;
extern uint32_t g_ddr_packet_headroom_size;
extern uint32_t g_psram_packet_headroom_size;
extern rdd_wan_tx_pointers_table_t *wan_tx_pointers_table;
extern uint32_t g_rate_cntrls_pool_idx;

#ifndef _CFE_
DEFINE_BDMF_FASTLOCK(int_lock);
#endif

extern void _rdd_cam_lookup_init(void);
extern int rdd_sim_alloc_segments(void);
extern void rdd_sim_free_segments(void);
#ifdef USE_BDMF_SHELL
extern int f_rdd_make_shell_commands(void);
#endif /* USE_BDMF_SHELL */

/* table manger need to create macors for entries without tables */
#define GLOBAL_CFG_EPON_MODE_F_OFFSET  0

void rdd_exit(void)
{
#ifdef FIRMWARE_INIT
    rdd_sim_free_segments();
#endif
}

static void rdd_bpm_init(uint32_t ddr_runner_base_address, uint32_t extra_ddr0_runner_base_address, uint32_t extra_ddr1_runner_base_address)
{
    uint32_t *base_address;
 
    base_address = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_PACKET_BUFFERS_BASE_ADDRESS);
    MWRITE_32(base_address, ddr_runner_base_address & 0x1FFFFFFF);

    base_address = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_PACKET_BUFFERS_BASE_ADDRESS);
    MWRITE_32(base_address, ddr_runner_base_address & 0x1FFFFFFF);

    base_address = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_EXTRA_DDR_0_BUFFERS_BASE_ADDRESS);
    MWRITE_32(base_address, extra_ddr0_runner_base_address & 0x1FFFFFFF);

    base_address = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + BPM_EXTRA_DDR_1_BUFFERS_BASE_ADDRESS);
    MWRITE_32(base_address, extra_ddr1_runner_base_address & 0x1FFFFFFF);
}

static void rdd_ddr_init(uint32_t ddr0_runner_base_address, uint32_t ddr1_runner_base_address, uint32_t ddr_packet_headroom_size)
{
    RUNNER_REGS_CFG_DDR_LKUP_MASK0 runner_ddr_lkup_mask0_register;
    RUNNER_REGS_CFG_DDR_LKUP_MASK1 runner_ddr_lkup_mask1_register;
    RUNNER_REGS_CFG_DDR_CFG runner_ddr_cfg_register;

    runner_ddr_cfg_register.new_addr_calc = 1;
    runner_ddr_cfg_register.dma_base = (ddr0_runner_base_address & 0xFFE00000) >> 21;
    runner_ddr_cfg_register.dma2_base = (ddr1_runner_base_address & 0xFFE00000) >> 21;
    runner_ddr_cfg_register.buffer_size = RUNNER_REGS_CFG_DDR_CFG_BUFFER_SIZE_BUFFER_SIZE_512_VALUE;
    runner_ddr_cfg_register.buffer_offset = 0;
    runner_ddr_cfg_register.rserved1 = 0;

    RUNNER_REGS_0_CFG_DDR_CFG_WRITE(runner_ddr_cfg_register);
    RUNNER_REGS_1_CFG_DDR_CFG_WRITE(runner_ddr_cfg_register);


    /* DDR lookup for routed packet - 5 tuples */
    runner_ddr_lkup_mask0_register.global_mask = 0x000001FF;

    RUNNER_REGS_0_CFG_DDR_LKUP_MASK0_WRITE(runner_ddr_lkup_mask0_register);
    RUNNER_REGS_1_CFG_DDR_LKUP_MASK0_WRITE(runner_ddr_lkup_mask0_register);

    /* DDR lookup for IPTV table - destination MAC, destination MAC + VLAN, destination IP */
    runner_ddr_lkup_mask1_register.global_mask = 0x00000000;

    RUNNER_REGS_0_CFG_DDR_LKUP_MASK1_WRITE(runner_ddr_lkup_mask1_register);
    RUNNER_REGS_1_CFG_DDR_LKUP_MASK1_WRITE(runner_ddr_lkup_mask1_register);

    /* write headroom size to runner memory, so that runner will set it upon init */
    rdd_ddr_packet_headroom_size_cfg(ddr_packet_headroom_size);
}

static void rdd_psram_init(uint32_t psram_packet_headroom_size)
{
    RUNNER_REGS_CFG_PSRAM_CFG runner_psram_cfg_register;

    runner_psram_cfg_register.buffer_offset = 0;
    runner_psram_cfg_register.rserved1 = 0;
    runner_psram_cfg_register.buffer_size = RUNNER_REGS_CFG_PSRAM_CFG_BUFFER_SIZE_BUFFER_SIZE_128_VALUE;
    runner_psram_cfg_register.dma_base = 0;

    RUNNER_REGS_1_CFG_PSRAM_CFG_WRITE(runner_psram_cfg_register);

    rdd_psram_packet_headroom_size_cfg(psram_packet_headroom_size);
}

#ifdef EPON
static void rdd_epon_tx_ddr_queue_init(uint16_t queue_size)
{
    RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_DTS *queue_descriptors_table_ptr;
    RDD_EPON_DDR_QUEUE_ADDRESSES_TABLE_DTS *ddr_queue_addresses_table_ptr;
    RDD_DDR_QUEUE_DESCRIPTOR_DTS *queue_descriptor_ptr;
    uint32_t ddr_queue_head_address;
    uint16_t wan_channel_id, queue_address;

    queue_descriptors_table_ptr = RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_PTR();
    ddr_queue_addresses_table_ptr = RDD_EPON_DDR_QUEUE_ADDRESSES_TABLE_PTR();

    for (wan_channel_id = 0; wan_channel_id <= RDD_WAN_CHANNEL_15; wan_channel_id++)
    {
        queue_descriptor_ptr = &(queue_descriptors_table_ptr->entry[wan_channel_id]);

        MEMSET(queue_descriptor_ptr, 0, sizeof(RDD_DDR_QUEUE_DESCRIPTOR_DTS));

        /* initialize the hardcoded parameters of the GPON tx queue descriptor */
        RDD_DDR_QUEUE_DESCRIPTOR_HEAD_PTR_WRITE(0, queue_descriptor_ptr);
        RDD_DDR_QUEUE_DESCRIPTOR_TAIL_PTR_WRITE(0, queue_descriptor_ptr);
        RDD_DDR_QUEUE_DESCRIPTOR_TAIL_ENTRY_WRITE(0, queue_descriptor_ptr);
        RDD_DDR_QUEUE_DESCRIPTOR_HEAD_ENTRY_WRITE(0, queue_descriptor_ptr);
        RDD_DDR_QUEUE_DESCRIPTOR_TAIL_BASE_ENTRY_WRITE(0x8, queue_descriptor_ptr);
        RDD_DDR_QUEUE_DESCRIPTOR_HEAD_BASE_ENTRY_WRITE(0, queue_descriptor_ptr);
        RDD_DDR_QUEUE_DESCRIPTOR_QUEUE_STATE_WRITE(0, queue_descriptor_ptr);

        queue_address = EPON_DDR_CACHE_FIFO_ADDRESS + wan_channel_id * CACHE_FIFO_BYTE_SIZE;
        RDD_DDR_QUEUE_DESCRIPTOR_CACHE_PTR_WRITE(queue_address, queue_descriptor_ptr);

        /* initialize the configured parameters of the GPON queue descriptor */
        RDD_DDR_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(queue_size, queue_descriptor_ptr);

        /* set DDR q base address for each queue */
        ddr_queue_head_address = (EPON_DDR_QUEUES_BASE_ADDRESS + (wan_channel_id * queue_size * CACHE_ENTRY_BYTE_SIZE)) & 0x1FFFFFFF;
        MWRITE_32(&(ddr_queue_addresses_table_ptr->entry[wan_channel_id]), ddr_queue_head_address);
    }
}
#endif

static void rdd_global_registers_init(void)
{
    RDD_DS_MAIN_RUNNER_GLOBAL_REGISTERS_INIT_DTS *ds_main_global_regs_init_buf;
    RDD_US_MAIN_RUNNER_GLOBAL_REGISTERS_INIT_DTS *us_main_global_regs_init_buf;
    RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS *ds_pico_global_regs_init_buf;
    RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS *us_pico_global_regs_init_buf;
    uint32_t global_regs[NUM_OF_GLOBAL_REGS] = {};

    /********** Main Runner A **********/

    MEMSET(global_regs, 0, sizeof(global_regs));

    /* R1 - constant one */
    global_regs[1] = 1;

#ifdef DS_DYNAMIC_DISPATCH
    global_regs[3] = DS_RX_DISPATCH_FIFO_ADDRESS;
#endif
    global_regs[6] = DOWNSTREAM_MULTICAST_INGRESS_QUEUE_ADDRESS;

    ds_main_global_regs_init_buf = RDD_DS_MAIN_RUNNER_GLOBAL_REGISTERS_INIT_PTR();

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at
       task -1 (initialization task) */
    MWRITE_BLK_32(ds_main_global_regs_init_buf, global_regs, sizeof(global_regs));

    /********** Main Runner B **********/

    MEMSET(global_regs, 0, sizeof(global_regs));

    /* R1 - constant one */
    global_regs[1] = 1;

    /* R2 - cpu tx upstream forward & DHD rx complete  */
    global_regs[2] = (US_CPU_TX_BBH_DESCRIPTORS_ADDRESS << 16) /*| US_CPU_TX_BBH_DESCRIPTORS_ADDRESS*/;

    /* R4 - free buffers pool counter */
    global_regs[4] = RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE;

    /* R5 - head pointer of the free buffers pool stack */
    global_regs[5] = US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS;

    global_regs[6] = WAN_ENQUEUE_INGRESS_QUEUE_ADDRESS;

    us_main_global_regs_init_buf = RDD_US_MAIN_RUNNER_GLOBAL_REGISTERS_INIT_PTR();

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at
       task -1 (initialization task) */
    MWRITE_BLK_32(us_main_global_regs_init_buf, global_regs, sizeof(global_regs));

    /********** Pico Runner A **********/

    MEMSET(global_regs, 0, sizeof(global_regs));

    /* R1 - constant one */
    global_regs[1] = 1;

    ds_pico_global_regs_init_buf = RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_PTR();

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at
       task -1 (initialization task) */
    MWRITE_BLK_32(ds_pico_global_regs_init_buf, global_regs, sizeof(global_regs));

    /********** Pico Runner B **********/

    MEMSET(global_regs, 0, sizeof(global_regs));

    /* R1 - constant one */
    global_regs[1] = 1;

#ifdef US_DYNAMIC_DISPATCH
    global_regs[3] = US_RX_DISPATCH_FIFO_ADDRESS;
#endif

    us_pico_global_regs_init_buf = RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_PTR();

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at
       task -1 (initialization task) */
    MWRITE_BLK_32(us_pico_global_regs_init_buf, global_regs, sizeof(global_regs));
}

static void rdd_inter_task_queues_common_init(void)
{
    uint16_t *ingress_queue;

    ingress_queue = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + WAN_ENQUEUE_INGRESS_QUEUE_PTR_ADDRESS);
    MWRITE_16(ingress_queue, WAN_ENQUEUE_INGRESS_QUEUE_ADDRESS);

    ingress_queue = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_CPU_RX_INGRESS_QUEUE_PTR_ADDRESS);
    MWRITE_16(ingress_queue, DS_CPU_RX_INGRESS_QUEUE_ADDRESS);

    ingress_queue = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_CPU_RX_INGRESS_QUEUE_PTR_ADDRESS);
    MWRITE_16(ingress_queue, US_CPU_RX_INGRESS_QUEUE_ADDRESS);

    ingress_queue = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_DHD_TX_POST_INGRESS_QUEUE_PTR_ADDRESS);
    MWRITE_16(ingress_queue, DS_DHD_TX_POST_INGRESS_QUEUE_ADDRESS);

    ingress_queue = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_DHD_TX_POST_INGRESS_QUEUE_PTR_ADDRESS);
    MWRITE_16(ingress_queue, US_DHD_TX_POST_INGRESS_QUEUE_ADDRESS);
}

static void rdd_inter_task_queues_init(void)
{
    rdd_inter_task_queues_common_init();
    rdd_inter_task_queues_proj_init();
}

static void rdd_port_to_bbh_destination_table_init(void)
{
    uint8_t *bbh_destination_table_table_ptr;

    bbh_destination_table_table_ptr = (uint8_t *)RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_PTR();
    MWRITE_8(bbh_destination_table_table_ptr + RDD_WAN0_VPORT,  BBH_PERIPHERAL_WAN_RX);
    MWRITE_8(bbh_destination_table_table_ptr + RDD_LAN0_VPORT,  BBH_PERIPHERAL_ETH0_RX);
    MWRITE_8(bbh_destination_table_table_ptr + RDD_LAN1_VPORT,  BBH_PERIPHERAL_ETH1_RX);
    MWRITE_8(bbh_destination_table_table_ptr + RDD_LAN2_VPORT,  BBH_PERIPHERAL_ETH2_RX);
    MWRITE_8(bbh_destination_table_table_ptr + RDD_LAN3_VPORT,  BBH_PERIPHERAL_ETH3_RX);
    MWRITE_8(bbh_destination_table_table_ptr + RDD_LAN4_VPORT,  BBH_PERIPHERAL_ETH4_RX);
    MWRITE_8(bbh_destination_table_table_ptr + RDD_VPORT_ID_6,  BBH_PERIPHERAL_WAN_RX);
}

int rdd_data_structures_init(rdd_init_params_t *init_params)
{
#ifdef DS_DYNAMIC_DISPATCH
    uint16_t ds_processing_tasks_arr[] = {
        PROCESSING_0_THREAD_NUMBER, PROCESSING_1_THREAD_NUMBER, PROCESSING_2_THREAD_NUMBER, PROCESSING_3_THREAD_NUMBER
    };
#endif
#ifdef US_DYNAMIC_DISPATCH
    uint16_t us_processing_tasks_arr[] = {
        PROCESSING_0_THREAD_NUMBER, PROCESSING_1_THREAD_NUMBER, PROCESSING_2_THREAD_NUMBER, PROCESSING_3_THREAD_NUMBER
    };
#endif

    g_runner_ddr0_base_addr = (uint32_t)init_params->ddr0_runner_base_ptr;
    g_runner_ddr1_base_addr = (uint32_t)init_params->ddr1_runner_base_ptr;

#ifndef FIRMWARE_INIT
    g_runner_nat_cache_key_ptr = (uint32_t)init_params->runner_nat_cache_key_ptr;
    g_runner_nat_cache_context_ptr = (uint32_t)init_params->runner_nat_cache_context_ptr;
    g_runner_ddr_phy_iptv_tables_base_ptr = (uint32_t)init_params->runner_iptv_tables_base_ptr;
    g_runner_ddr0_iptv_context_ptr = (uint32_t)ioremap(g_runner_ddr_phy_iptv_tables_base_ptr, 0x100000);
    g_runner_ddr0_iptv_lookup_ptr = g_runner_ddr0_iptv_context_ptr + 0x40000;
    g_runner_ddr0_iptv_ssm_context_ptr = g_runner_ddr0_iptv_context_ptr + 0x80000;
#endif
    g_ddr_packet_headroom_size = init_params->ddr_packet_headroom_size;
    g_psram_packet_headroom_size = init_params->psram_packet_headroom_size;

    rdd_bpm_init(g_runner_ddr0_base_addr, (uint32_t)init_params->extra_ddr0_pool_ptr, (uint32_t)init_params->extra_ddr1_pool_ptr);

    rdd_ddr_init(g_runner_ddr0_base_addr, g_runner_ddr1_base_addr, g_ddr_packet_headroom_size);

    rdd_psram_init(g_psram_packet_headroom_size);

    rdd_scheduler_init();

    rdd_pm_counters_init();

    rdd_crc_init();

    _rdd_cam_lookup_init();

    rdd_global_registers_init();
    rdd_local_registers_init();

#ifdef DS_DYNAMIC_DISPATCH
    rdd_ds_dynamic_dispatcher_init(ds_processing_tasks_arr, ARRAY_LENGTH(ds_processing_tasks_arr));
#endif
#ifdef US_DYNAMIC_DISPATCH
    rdd_us_dynamic_dispatcher_init(us_processing_tasks_arr, ARRAY_LENGTH(us_processing_tasks_arr));
#endif
    rdd_inter_task_queues_init();
    rdd_parallel_processing_init();
    rdd_cpu_rx_init();
    rdd_cpu_tx_init();

    rdd_port_to_bbh_destination_table_init();

#ifdef DS_SRAM_TX_QUEUES
    rdd_ds_free_packet_descriptors_pool_init();
#endif
#ifdef US_SRAM_TX_QUEUES
    rdd_us_free_packet_descriptors_pool_init();
#endif

    rdd_eth_tx_init();
    rdd_wan_tx_init();

    rdd_reverse_ffi_table_init();

    rdd_layer2_header_copy_mapping_init();

    rdd_proj_init(init_params);

    rdd_actions_proj_init();

#ifdef USE_BDMF_SHELL
    /* register shell commands */
    f_rdd_make_shell_commands();
#endif

    return 0;
}

void rdd_action_vector_set(uint16_t *action_ptrs_ptr, uint16_t *action_ptrs, uint8_t action_total_num)
{
    MWRITE_BLK_16(action_ptrs, action_ptrs_ptr, action_total_num * sizeof(RDD_BYTES_2_DTS));
}
