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

#ifdef RDP_SIM
#include "rdd.h"
#include "rdd_simulator.h"
#ifdef CONFIG_DHD_RUNNER
#include "rdpa_dhd_helper_basic.h"
#include "rdd_dhd_helper.h"
#endif

extern RDD_WAN_TX_POINTERS_TABLE_DTS *wan_tx_pointers_table_ptr;

uint8_t *soc_base_address;
uint32_t cpu_rx_ring_base_addr_ptr;
uint32_t sim_iptv_table_ptr;
uint32_t sim_iptv_context_table_ptr;
uint32_t sim_iptv_ssm_context_table_ptr;
#ifdef CONFIG_DHD_RUNNER
uint32_t sim_dhd_tx_flow_ring_mgmt_ptr;
#endif

#if defined(DSL_63138) || defined(DSL_63148)
#if defined(FIRMWARE_INIT)
extern uint8_t  *DsConnectionTableBase;
extern uint8_t  *UsConnectionTableBase;
extern uint8_t  *ContextTableBase;
extern uint8_t  *cpu_rx_ring_base;
#endif
extern uint32_t g_runner_tables_ptr;
#elif defined(WL4908)
#if defined(FIRMWARE_INIT)
extern uint8_t  *NatCacheTableBase;
extern uint8_t  *ContextTableBase;
extern uint8_t  *ContextContTableBase;
extern uint8_t  *cpu_rx_ring_base;
#else
extern uint32_t g_runner_tables_ptr;
#endif
#endif /* WL4908 */

#define SET_FORMAT "set %s %08x %08x\n"
#ifdef CONFIG_DHD_RUNNER
#define DHD_TX_POST_FLOW_RING_MGMT_TABLE_PTR sim_dhd_tx_flow_ring_mgmt_ptr
#endif

void save_table(FILE *table_f, char *table_name, uint32_t *table, uint32_t table_offset, uint32_t table_size,
    int skip_empty)
{
    uint32_t i, val;

    for (i = 0; i < table_size / 4; i++)
    {
        val = MGET_32(table + i);
        if (!val && skip_empty)
            continue;
        fprintf(table_f, SET_FORMAT, table_name, (unsigned int)(table_offset + i * 4), val);
    }
}

#define SAVE_DDR_TABLE(table, offset, size) save_table(ddr_tables, "ddr", table, (uint32_t)(offset), size, 1)

int rdd_sim_save_ddr_tables(void)
{
    FILE *ddr_tables;
    uint32_t packet_descriptor_address;
    uint32_t i;
#if defined(CONFIG_WLAN_MCAST) && defined(CONFIG_DHD_RUNNER)
    extern wlan_mcast_dhd_list_table_t wlan_mcast_dhd_list_table_g;
#endif
    uint32_t *ptr;
    extern uint32_t *sim_port_header_buffer_g;
#if defined(WL4908)
    RDD_NATC_CONTEXT_TABLE_DTS *context_table_ptr = ( RDD_NATC_CONTEXT_TABLE_DTS* )ContextTableBase;
    RDD_CONTEXT_CONTINUATION_TABLE_DTS *context_cont_table_ptr = ( RDD_CONTEXT_CONTINUATION_TABLE_DTS* )ContextContTableBase;
#else
    RDD_CONTEXT_TABLE_DTS *context_table_ptr = ( RDD_CONTEXT_TABLE_DTS* )ContextTableBase;
#endif

    ddr_tables = fopen("ddr_tables.dfp", "w+b");
    if (!ddr_tables)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to open ddr_tables.dfp");

#if defined(WL4908)
    SAVE_DDR_TABLE((uint32_t *)NatCacheTableBase, NAT_CACHE_TABLE_ADDRESS, sizeof(RDD_NAT_CACHE_TABLE_DTS));
    SAVE_DDR_TABLE((uint32_t *)ContextTableBase, NATC_CONTEXT_TABLE_ADDRESS, sizeof(RDD_NATC_CONTEXT_TABLE_DTS));
    SAVE_DDR_TABLE((uint32_t *)ContextContTableBase, CONTEXT_CONTINUATION_TABLE_ADDRESS, sizeof(RDD_CONTEXT_CONTINUATION_TABLE_DTS));
#else
    SAVE_DDR_TABLE((uint32_t *)UsConnectionTableBase, g_runner_tables_ptr + US_CONNECTION_TABLE_ADDRESS, sizeof(RDD_CONNECTION_TABLE_DTS));
    SAVE_DDR_TABLE((uint32_t *)DsConnectionTableBase, g_runner_tables_ptr + DS_CONNECTION_TABLE_ADDRESS, sizeof(RDD_CONNECTION_TABLE_DTS));
    SAVE_DDR_TABLE((uint32_t *)ContextTableBase, g_runner_tables_ptr + CONTEXT_TABLE_ADDRESS, sizeof(RDD_CONTEXT_TABLE_DTS));
#endif

    ptr = sim_port_header_buffer_g;
    /* Add multicast port header buffers. */
    if (ptr)
        SAVE_DDR_TABLE((uint32_t *)ptr, 
                   g_runner_ddr_base_addr + SIMULATOR_DDR_PORT_HEADER_BUFFERS_OFFSET,
                   RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE * RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE2);

#if defined(WL4908)
    /* Add first 10 context entries because multicast contextes do not get written out in save_connection_table. */
    SAVE_DDR_TABLE((uint32_t *)context_table_ptr, 
                   g_runner_ddr_base_addr + NATC_CONTEXT_TABLE_ADDRESS,
                   sizeof(RDD_NATC_CONTEXT_TABLE_DTS));

    SAVE_DDR_TABLE((uint32_t *)context_cont_table_ptr,
                   g_runner_ddr_base_addr + CONTEXT_CONTINUATION_TABLE_ADDRESS,
                   sizeof(RDD_CONTEXT_CONTINUATION_TABLE_DTS));
#else
    SAVE_DDR_TABLE((uint32_t *)context_table_ptr, 
                   g_runner_tables_ptr + CONTEXT_TABLE_ADDRESS,
                   sizeof(RDD_CONTEXT_TABLE_DTS));
#endif

    /* Save DDR packet descriptors in rings */
    for ( i = 0, ptr = ( uint32_t * )cpu_rx_ring_base; i < ( RDD_RING_DESCRIPTORS_TABLE_SIZE * 10 ); i++ )
    {
        packet_descriptor_address = SIMULATOR_DDR_RING_OFFSET + sizeof( RDD_CPU_RX_DESCRIPTOR_DTS ) * i;
        SAVE_DDR_TABLE((uint32_t *)ptr, packet_descriptor_address, sizeof(RDD_CPU_RX_DESCRIPTOR_DTS));
        ptr += sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) / sizeof(uint32_t);
    }

#if defined(CONFIG_WLAN_MCAST) && defined(CONFIG_DHD_RUNNER)
    /* Add first 4 wlan multicast dhd lists */
    printf("SIMULATOR_DDR_WLAN_MCAST_DHD_LIST 0x%08x\n", (uint32_t)(g_runner_ddr_base_addr + SIMULATOR_DDR_WLAN_MCAST_DHD_LIST_OFFSET));
    SAVE_DDR_TABLE((uint32_t *)wlan_mcast_dhd_list_table_g.virt_p,
                   g_runner_ddr_base_addr + SIMULATOR_DDR_WLAN_MCAST_DHD_LIST_OFFSET,
                   sizeof(RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DHD_STATION_NUMBER));

#ifdef CONFIG_DHD_RUNNER
    SAVE_DDR_TABLE((uint32_t *)DHD_TX_POST_FLOW_RING_MGMT_TABLE_PTR, DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_ADDRESS, 
        RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_SIZE * sizeof(RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_DTS));
#endif
#endif

    fclose(ddr_tables);
    return 0;
}

int rdd_sim_save_hw_cfg(void)
{
    FILE *sim_cfg;
    RUNNER_REGS_CFG_DDR_CFG ddr_cfg_register;
    RUNNER_REGS_CFG_CAM_CFG cam_cfg_register;
    RUNNER_REGS_CFG_CNTR_CFG counter_cfg_register;
    RUNNER_REGS_CFG_DDR_LKUP_MASK0 runner_ddr_lkup_mask0_register;
    RUNNER_REGS_CFG_DDR_LKUP_MASK1 runner_ddr_lkup_mask1_register;
    RUNNER_REGS_CFG_LKUP2_CFG hash_lkup_2_cfg_register;
    RUNNER_REGS_CFG_LKUP2_CAM_CFG hash_lkup_2_cam_cfg_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK2_H hash_lkup_2_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK2_L hash_lkup_2_global_mask_low_register;

    uint32_t scheduler_config_register;

    sim_cfg = fopen("sim_cfg", "w+b");
    if (!sim_cfg)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to open sim_cfg");

    RUNNER_REGS_0_CFG_DDR_CFG_READ(ddr_cfg_register);

    fprintf(sim_cfg, "ddr_base %08x %08x %d\n", (ddr_cfg_register.dma_base << 21),
        (ddr_cfg_register.dma_base << 21) + 0x1800000, ddr_cfg_register.buffer_offset);

    RUNNER_REGS_0_CFG_DDR_LKUP_MASK0_READ(runner_ddr_lkup_mask0_register);
    fprintf(sim_cfg, "set dma_lkp_global_0 %08x\n", runner_ddr_lkup_mask0_register.global_mask);

    RUNNER_REGS_0_CFG_DDR_LKUP_MASK1_READ(runner_ddr_lkup_mask1_register);
    fprintf(sim_cfg, "set dma_lkp_global_1 %08x\n", runner_ddr_lkup_mask1_register.global_mask);

    RUNNER_REGS_0_CFG_LKUP2_CFG_READ(hash_lkup_2_cfg_register);
    RUNNER_REGS_0_CFG_LKUP2_CAM_CFG_READ(hash_lkup_2_cam_cfg_register);
    fprintf(sim_cfg, "set mac2 A %x %d %d %d", (unsigned int)(hash_lkup_2_cfg_register.base_address << 3),
        (int)32 * (1 << hash_lkup_2_cfg_register.table_size), (int)(1 << hash_lkup_2_cfg_register.max_hop),
        (int)hash_lkup_2_cfg_register.hash_type);

    if (hash_lkup_2_cam_cfg_register.cam_en)
        fprintf(sim_cfg, " %x\n", hash_lkup_2_cam_cfg_register.base_address << 3);
    else
        fprintf(sim_cfg, "\n");

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK2_H_READ(hash_lkup_2_global_mask_high_register);
    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK2_L_READ(hash_lkup_2_global_mask_low_register);
    fprintf(sim_cfg, "set global_mask_2 A %08x %08x\n", (unsigned int)hash_lkup_2_global_mask_high_register.base_address,
        (unsigned int)hash_lkup_2_global_mask_low_register.base_address);

#if defined(WL4908)
    fprintf(sim_cfg, "set natc A %08x %d 16 %d\n",
        NAT_CACHE_TABLE_ADDRESS, RDD_NAT_CACHE_TABLE_SIZE, RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE);

    fprintf(sim_cfg, "set natc_result A %08x %d\n",
        NATC_CONTEXT_TABLE_ADDRESS, sizeof(RDD_NATC_CONTEXT_ENTRY_UNION_DTS));

    fprintf(sim_cfg, "set natc B %08x %d 16 %d\n",
        NAT_CACHE_TABLE_ADDRESS, RDD_NAT_CACHE_TABLE_SIZE, RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE);

    fprintf(sim_cfg, "set natc_result B %08x %d\n",
        NATC_CONTEXT_TABLE_ADDRESS, sizeof(RDD_NATC_CONTEXT_ENTRY_UNION_DTS));
#endif

    RUNNER_REGS_0_CFG_CAM_CFG_READ(cam_cfg_register);
    fprintf(sim_cfg, "set cam_stop_value A %x\n", cam_cfg_register.stop_value);

    RUNNER_REGS_1_CFG_CAM_CFG_READ(cam_cfg_register);
    fprintf(sim_cfg, "set cam_stop_value B %x\n", cam_cfg_register.stop_value);

    /* save scheduller configuration */
    RUNNER_REGS_0_CFG_MAIN_SCH_CFG_READ(scheduler_config_register);

    fprintf(sim_cfg, "set scheduler_cfg A CLASS_%c group_0=%s group_1=%s group_2=%s group_3=%s\n",
        (scheduler_config_register & 0x10) ? 'A' : ((scheduler_config_register & 0x20) ? 'B' : 'C'),
        (scheduler_config_register & 0x01) ? "RR" : "SP",
        (scheduler_config_register & 0x02) ? "RR" : "SP",
        (scheduler_config_register & 0x04) ? "RR" : "SP",
        (scheduler_config_register & 0x08) ? "RR" : "SP");

    RUNNER_REGS_1_CFG_MAIN_SCH_CFG_READ(scheduler_config_register);

    fprintf(sim_cfg, "set scheduler_cfg B CLASS_%c group_0=%s group_1=%s group_2=%s group_3=%s\n",
        (scheduler_config_register & 0x10) ? 'A' : ((scheduler_config_register & 0x20) ? 'B' : 'C'),
        (scheduler_config_register & 0x01) ? "RR" : "SP",
        (scheduler_config_register & 0x02) ? "RR" : "SP",
        (scheduler_config_register & 0x04) ? "RR" : "SP",
        (scheduler_config_register & 0x08) ? "RR" : "SP");

    RUNNER_REGS_0_CFG_PICO_SCH_CFG_READ(scheduler_config_register);

    fprintf(sim_cfg, "set scheduler_cfg C CLASS_%c group_0=%s group_1=%s group_2=SP group_3=SP\n",
        (scheduler_config_register & 0x10) ? 'A' : ((scheduler_config_register & 0x20) ? 'B' : 'C'),
        (scheduler_config_register & 0x01) ? "RR" : "SP",
        (scheduler_config_register & 0x02) ? "RR" : "SP");

    RUNNER_REGS_1_CFG_PICO_SCH_CFG_READ(scheduler_config_register);

    fprintf(sim_cfg, "set scheduler_cfg D CLASS_%c group_0=%s group_1=%s group_2=SP group_3=SP\n",
        (scheduler_config_register & 0x10) ? 'A' : ((scheduler_config_register & 0x20) ? 'B' : 'C'),
        (scheduler_config_register & 0x01) ? "RR" : "SP",
        (scheduler_config_register & 0x02) ? "RR" : "SP");

    RUNNER_REGS_0_CFG_CNTR_CFG_READ(counter_cfg_register);
    fprintf(sim_cfg, "set counter A %x common\n", counter_cfg_register.base_address << 3);

    RUNNER_REGS_1_CFG_CNTR_CFG_READ(counter_cfg_register);
    fprintf(sim_cfg, "set counter B %x common\n", counter_cfg_register.base_address << 3);

    fclose(sim_cfg);
    return 0;
}

int bl_lilac_rdd_save_sim_config(void)
{
    return rdd_sim_save_hw_cfg();
}

void rdd_sim_save_tx_pointers(uint16_t *buffer)
{
    RDD_WAN_TX_POINTERS_ENTRY_DTS *entry;
    uint32_t wan_channel_id;
    uint32_t rate_controller_id;
    uint32_t queue_id;

#if defined(CONFIG_DSLWAN)
    for (wan_channel_id = 0; wan_channel_id < (RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE); wan_channel_id++)
#else
    for (wan_channel_id = 0; wan_channel_id < RDD_WAN_CHANNELS_0_7_TABLE_SIZE; wan_channel_id++)
#endif
    {
        for (rate_controller_id = 0; rate_controller_id < RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER; rate_controller_id++)
        {
            for (queue_id = 0; queue_id < RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER; queue_id++)
            {
                entry = &(wan_tx_pointers_table_ptr->entry[wan_channel_id][rate_controller_id][queue_id]);

                if (entry->wan_tx_queue_ptr)
                {
                    buffer[wan_channel_id * RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER *
                        RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER + rate_controller_id *
                        RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER + queue_id] =
#ifdef FIRMWARE_LITTLE_ENDIAN
                            swap2bytes((entry->wan_tx_queue_ptr - WAN_TX_QUEUES_TABLE_ADDRESS)
                                / sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS));
#else
                    (entry->wan_tx_queue_ptr - WAN_TX_QUEUES_TABLE_ADDRESS) / sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS);
#endif
                }
                else
                {
                    buffer[wan_channel_id * RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER * RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER +
                        rate_controller_id * RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER + queue_id] = 0xFFFF;
                }
            }
        }
    }
}

void bl_lilac_rdd_copy_tx_pointers_table(uint16_t *buffer)
{
    rdd_sim_save_tx_pointers(buffer);
}

/* allocate memory segments for simulation */
int rdd_sim_alloc_segments(void)
{
    SEG_CHK_ALLOC(soc_base_address, sizeof(uint8_t) * SIM_MEM_SIZE);
    MEMSET ( ( uint8_t *)  soc_base_address, 0, sizeof( uint8_t ) * SIM_MEM_SIZE );

#if defined(DSL_63138) || defined(DSL_63148)
    SEG_CHK_ALLOC(DsConnectionTableBase, sizeof( RDD_CONNECTION_TABLE_DTS));
    SEG_CHK_ALLOC(UsConnectionTableBase, sizeof( RDD_CONNECTION_TABLE_DTS));
    SEG_CHK_ALLOC(ContextTableBase, sizeof( RDD_CONTEXT_TABLE_DTS));
    SEG_CHK_ALLOC(cpu_rx_ring_base, sizeof(RDD_RING_DESCRIPTORS_TABLE_DTS));
#ifdef CONFIG_DHD_RUNNER
    SEG_CHK_ALLOC(DHD_TX_POST_FLOW_RING_MGMT_TABLE_PTR, 
            RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_SIZE * sizeof(RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_DTS));
#endif
#elif defined(WL4908)
    SEG_CHK_ALLOC(NatCacheTableBase, sizeof(RDD_NAT_CACHE_TABLE_DTS));
    SEG_CHK_ALLOC(ContextTableBase, sizeof(RDD_NATC_CONTEXT_TABLE_DTS));
    SEG_CHK_ALLOC(ContextContTableBase, sizeof(RDD_CONTEXT_CONTINUATION_TABLE_DTS));
    SEG_CHK_ALLOC(cpu_rx_ring_base, sizeof(RDD_RING_DESCRIPTORS_TABLE_DTS));
#ifdef CONFIG_DHD_RUNNER
    SEG_CHK_ALLOC(DHD_TX_POST_FLOW_RING_MGMT_TABLE_PTR, 
            RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_SIZE * sizeof(RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_DTS));
#endif
#endif
    return 0;
}

/* allocate memory segments for simulation */
void rdd_sim_free_segments(void)
{
#if defined(DSL_63138) || defined(DSL_63148)
    SEG_CHK_FREE(DsConnectionTableBase);
    SEG_CHK_FREE(UsConnectionTableBase);
    SEG_CHK_FREE(ContextTableBase);
    SEG_CHK_FREE(cpu_rx_ring_base);
#elif defined(WL4908)
    SEG_CHK_FREE(cpu_rx_ring_base);
    SEG_CHK_FREE(ContextContTableBase);
    SEG_CHK_FREE(ContextTableBase);
    SEG_CHK_FREE(NatCacheTableBase);
#endif
    SEG_CHK_FREE(soc_base_address);
}

int _segment_file_init(const char *file_name, const char *mode, uint8_t *segment_mem, int segment_size)
{
    FILE *segment_file;

    segment_file = fopen(file_name, mode);

    if (!segment_file)
    {
        printf("Error: Can't open %s\n", file_name);
        return -1;
    }

    fwrite(segment_mem, sizeof(uint8_t), segment_size, segment_file);

    fclose(segment_file);
    return 0;
}

int segment_file_init(const char *file_name, const char *mode, int segment, int segment_size)
{
    return _segment_file_init(file_name, mode, (uint8_t *)DEVICE_ADDRESS(segment), segment_size);
}

#ifdef CONFIG_DHD_RUNNER
void rdd_save_wifi_dongle_config(void)
{
    FILE *dongle_config;

    dongle_config = fopen("wifi_dongle_config", "w+b");

    if (dongle_config == NULL)
    {
        printf("Error: Can't open wifi_dongle_config file\n");
        return;
    }

    fprintf(dongle_config, "rx_post_flow_ring_base_addr %08x\n", DHD_RX_POST_DDR_BUFFER_ADDRESS);
    fprintf(dongle_config, "rx_complete_flow_ring_base_addr %08x\n", DHD_RX_COMPLETE_DDR_BUFFER_ADDRESS);
    fprintf(dongle_config, "tx_post_flow_ring_mgmt_ring_base_addr %08x\n",
        DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_ADDRESS);
    fprintf(dongle_config, "tx_complete_flow_ring_base_addr %08x\n", DHD_TX_COMPLETE_DDR_BUFFER_ADDRESS);
    fprintf(dongle_config, "r2d_wr_arr_base_addr %08x\n", R2D_WR_ARR_DDR_BUFFER_ADDRESS);
    fprintf(dongle_config, "d2r_rd_arr_base_addr %08x\n", D2R_RD_ARR_DDR_BUFFER_ADDRESS);
    fprintf(dongle_config, "r2d_rd_arr_base_addr %08x\n", R2D_RD_ARR_DDR_BUFFER_ADDRESS);
    fprintf(dongle_config, "d2r_wr_arr_base_addr %08x\n", D2R_WR_ARR_DDR_BUFFER_ADDRESS);
    fprintf(dongle_config, "tx_post_flow_ring_max_id %08x\n", RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_SIZE);
    fprintf(dongle_config, "rx_post_flow_ring_size %08x\n", RDD_DHD_RX_POST_DDR_BUFFER_SIZE);
    fprintf(dongle_config, "rx_complete_flow_ring_size %08x\n", RDD_DHD_RX_COMPLETE_DDR_BUFFER_SIZE);
    fprintf(dongle_config, "tx_post_flow_ring_size %08x\n", RDD_DHD_TX_POST_DDR_BUFFER_SIZE2);
    fprintf(dongle_config, "tx_complete_flow_ring_size %08x\n", RDD_DHD_TX_COMPLETE_DDR_BUFFER_SIZE);

    fclose(dongle_config);
}

#define SIM_TX_POST_FLOW_RING_ENTRIES RDD_DHD_TX_POST_DDR_BUFFER_SIZE2
#define SIM_TX_POST_FLOW_RING_SIZE (sizeof(RDD_DHD_TX_POST_DESCRIPTOR_DTS) * SIM_TX_POST_FLOW_RING_ENTRIES)

/* We must use the descriptor defined below for initialization and not the autogenerated one,
   becase otherwise flags and size will be swapped (since they are defined as bitfields inside uint32_t)
   This struct must be always synced with RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_DTS
*/
typedef struct 
{
    uint32_t	flow_ring_base_low;
    uint32_t	flow_ring_base_high;
    uint32_t	size_and_flags;
    uint32_t	reserved;
} dhd_tx_post_mgmt_descriptor_t;

void rdd_sim_dhd_tx_flow_ring_mgmt_init(void)
{
    int i = 0;
    dhd_tx_post_mgmt_descriptor_t *mgmt = (dhd_tx_post_mgmt_descriptor_t *)DHD_TX_POST_FLOW_RING_MGMT_TABLE_PTR;
    uint32_t flow_ring_addr = DHD_TX_POST_DDR_BUFFER_ADDRESS;

    for (; i < RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_SIZE; i++, mgmt++, flow_ring_addr += SIM_TX_POST_FLOW_RING_SIZE)
    {
        memset(mgmt, 0, sizeof(dhd_tx_post_mgmt_descriptor_t));
        mgmt->flow_ring_base_low = swap4bytes(flow_ring_addr);
        mgmt->size_and_flags = SIM_TX_POST_FLOW_RING_ENTRIES; 
    }
}
#endif /* CONFIG_DHD_RUNNER */

#endif /* RDP_SIM */

