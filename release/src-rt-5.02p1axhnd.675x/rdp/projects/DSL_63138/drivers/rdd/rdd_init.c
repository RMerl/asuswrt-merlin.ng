/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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
#ifdef CONFIG_DHD_RUNNER
#include "dhd_defs.h"
#endif
#include "rdp_mm.h"

#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif
#include "rdd_service_queues.h"


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

#if defined(FIRMWARE_INIT)
uint8_t  *soc_base_address;
uint8_t  *DsConnectionTableBase;
uint8_t  *UsConnectionTableBase;
uint8_t  *cpu_rx_ring_base;
#endif

uint8_t                                               *ContextTableBase;
#if defined(WL4908)
uint8_t                                               *ContextContTableBase;
uint8_t                                               *NatCacheTableBase;
RDD_NAT_CACHE_TABLE_DTS                               *g_nat_cache_table_ptr;
#else
extern RDD_CONNECTION_TABLE_DTS                       *g_ds_connection_table_ptr;
#endif
extern int                                            g_dbg_lvl;
extern RDD_FC_MCAST_CONNECTION2_TABLE_DTS             *g_fc_mcast_connection2_table_ptr;
extern uint8_t                                        *g_runner_ddr_base_addr;
extern uint32_t                                       g_runner_ddr_base_addr_phys;
extern uint8_t                                        *g_runner_extra_ddr_base_addr;
extern uint32_t                                       g_runner_extra_ddr_base_addr_phys;
extern uint32_t                                       g_ddr_headroom_size;
extern uint8_t                                        *g_runner_tables_ptr;
extern uint8_t                                        g_broadcom_switch_mode;
extern BL_LILAC_RDD_BRIDGE_PORT_DTE                   g_broadcom_switch_physical_port;
extern uint32_t                                       g_bridge_flow_cache_mode;
extern BL_LILAC_RDD_VERSION_DTS                       gs_rdd_version;
extern RDD_64_BIT_TABLE_CFG                           g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
extern uint8_t **g_cpu_tx_skb_pointers_reference_array;
extern uint8_t *g_dhd_tx_cpu_usage_reference_array;
extern rdd_phys_addr_t *g_cpu_tx_data_pointers_reference_array;
extern uint32_t g_cpu_tx_abs_packet_limit;
extern rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address;
extern rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address_last_idx;
extern uint16_t *g_free_skb_indexes_fifo_table;
extern RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTE  g_ingress_classification_rule_cfg_table[ 2 ];
extern uint32_t                                       g_rate_controllers_pool_idx;
extern uint32_t                                       g_chip_revision;
extern RDD_WAN_TX_POINTERS_TABLE_DTS                  *wan_tx_pointers_table_ptr;
extern uint8_t                                        g_lookup_port_init_mapping_table[];
rdpa_bpm_buffer_size_t                                g_bpm_buffer_size = LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE;

static BL_LILAC_RDD_ERROR_DTE f_rdd_bpm_initialize ( uint32_t, uint32_t, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_ddr_initialize ( uint32_t, uint32_t, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_psram_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_scheduler_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_free_packet_descriptors_pool_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_global_registers_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_local_registers_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_classification_table_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_eth_tx_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_wan_tx_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_inter_task_queues_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_pm_counters_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_transmit_from_abs_address_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_parallel_processing_initialize ( void );

extern BL_LILAC_RDD_ERROR_DTE rdd_firewall_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_mac_table_initialize ( uint32_t, uint32_t );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_filters_cam_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_layer4_filters_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_vlan_matrix_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_connection_table_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_multicast_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_vid_cam_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_ds_exponent_table_initialize ( void );
extern void f_rdd_full_flow_cache_config ( bdmf_boolean );

#ifdef CONFIG_DHD_RUNNER
extern void rdd_dhd_mode_enable_init(void);
#endif

#if !defined(FIRMWARE_INIT) && defined ( USE_BDMF_SHELL )
extern BL_LILAC_RDD_ERROR_DTE f_rdd_make_shell_commands ( void );
#endif /* USE_BDMF_SHELL */


BL_LILAC_RDD_ERROR_DTE rdd_init ( void )
{
    RUNNER_INST_MAIN   *sram_fast_program_ptr;
    RUNNER_INST_PICO   *sram_pico_program_ptr;
    RUNNER_COMMON      *sram_common_data_ptr;
    RUNNER_PRIVATE     *sram_private_data_ptr;
    RUNNER_CNTXT_MAIN  *sram_fast_context_ptr;
    RUNNER_CNTXT_PICO  *sram_pico_context_ptr;
    RUNNER_PRED_MAIN   *sram_fast_prediction_ptr;
    RUNNER_PRED_PICO   *sram_pico_prediction_ptr;

#ifdef FIRMWARE_INIT
    if ( rdd_sim_alloc_segments() )
    {
        return -1;
    }
#endif

    /* reset SRAM program memory of both Runners */
    sram_fast_program_ptr = ( RUNNER_INST_MAIN * )DEVICE_ADDRESS( RUNNER_INST_MAIN_0_OFFSET );
    rdp_mm_setl ( sram_fast_program_ptr, 0, sizeof ( RUNNER_INST_MAIN ) );

    sram_fast_program_ptr = ( RUNNER_INST_MAIN * )DEVICE_ADDRESS( RUNNER_INST_MAIN_1_OFFSET );
    rdp_mm_setl ( sram_fast_program_ptr, 0, sizeof ( RUNNER_INST_MAIN ) );

    sram_pico_program_ptr = ( RUNNER_INST_PICO * )DEVICE_ADDRESS( RUNNER_INST_PICO_0_OFFSET );
    rdp_mm_setl ( sram_pico_program_ptr, 0, sizeof ( RUNNER_INST_PICO ) );

    sram_pico_program_ptr = ( RUNNER_INST_PICO * )DEVICE_ADDRESS( RUNNER_INST_PICO_1_OFFSET );
    rdp_mm_setl ( sram_fast_program_ptr, 0, sizeof ( RUNNER_INST_PICO ) );

    /* reset SRAM common data memory of both Runners */
    sram_common_data_ptr = ( RUNNER_COMMON * )DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET );
    rdp_mm_setl ( sram_common_data_ptr, 0, sizeof ( RUNNER_COMMON ) );

    sram_common_data_ptr = ( RUNNER_COMMON * )DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET );
    rdp_mm_setl ( sram_common_data_ptr, 0, sizeof ( RUNNER_COMMON ) );

    /* reset SRAM private data memory of both Runners */
    sram_private_data_ptr = ( RUNNER_PRIVATE * )DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET );
    rdp_mm_setl ( sram_private_data_ptr, 0, sizeof ( RUNNER_PRIVATE ) );

    sram_private_data_ptr = ( RUNNER_PRIVATE * )DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET );
    rdp_mm_setl ( sram_private_data_ptr, 0, sizeof ( RUNNER_PRIVATE ) );

    /* reset SRAM context memory of both Runners */
    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_0_OFFSET );
    rdp_mm_setl_context ( sram_fast_context_ptr, 0, sizeof ( RUNNER_CNTXT_MAIN ) );

    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_1_OFFSET );
    rdp_mm_setl_context ( sram_fast_context_ptr, 0, sizeof ( RUNNER_CNTXT_MAIN ) );

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_0_OFFSET );
    rdp_mm_setl_context ( sram_pico_context_ptr, 0, sizeof ( RUNNER_CNTXT_PICO ) );

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_1_OFFSET );
    rdp_mm_setl_context ( sram_pico_context_ptr, 0, sizeof ( RUNNER_CNTXT_PICO ) );

    /* reset SRAM prediction memory of both Runners */
    sram_fast_prediction_ptr = ( RUNNER_PRED_MAIN * )DEVICE_ADDRESS( RUNNER_PRED_MAIN_0_OFFSET );
    rdp_mm_setl ( sram_fast_prediction_ptr, 0, sizeof ( RUNNER_PRED_MAIN ) * 2 );

    sram_fast_prediction_ptr = ( RUNNER_PRED_MAIN * )DEVICE_ADDRESS( RUNNER_PRED_MAIN_1_OFFSET );
    rdp_mm_setl ( sram_fast_prediction_ptr, 0, sizeof ( RUNNER_PRED_MAIN ) * 2 );

    sram_pico_prediction_ptr = ( RUNNER_PRED_PICO * )DEVICE_ADDRESS( RUNNER_PRED_PICO_0_OFFSET );
    rdp_mm_setl ( sram_pico_prediction_ptr, 0, sizeof ( RUNNER_PRED_PICO ) * 2 );

    sram_pico_prediction_ptr = ( RUNNER_PRED_PICO * )DEVICE_ADDRESS( RUNNER_PRED_PICO_1_OFFSET );
    rdp_mm_setl ( sram_pico_prediction_ptr, 0, sizeof ( RUNNER_PRED_PICO ) * 2 );

    return ( BL_LILAC_RDD_OK );
}


void rdd_exit ( void )
{
#ifdef FIRMWARE_INIT
    rdd_sim_free_segments();
#else 
    /* absolute address DDR memories free */
#ifndef RDD_BASIC
    bdmf_free( g_cpu_tx_skb_pointers_reference_array );
    bdmf_free( g_dhd_tx_cpu_usage_reference_array );
#else
    KFREE( g_cpu_tx_skb_pointers_reference_array );
    KFREE( g_dhd_tx_cpu_usage_reference_array );
#endif
    rdp_mm_aligned_free((void *)g_cpu_tx_data_pointers_reference_array, sizeof(rdd_phys_addr_t) * g_cpu_tx_abs_packet_limit);
    rdp_mm_aligned_free((void *)g_free_skb_indexes_fifo_table, sizeof(uint16_t) * g_cpu_tx_abs_packet_limit);
#endif
}


BL_LILAC_RDD_ERROR_DTE rdd_load_microcode ( uint8_t  *xi_runer_A_microcode_ptr,
                                            uint8_t  *xi_runer_B_microcode_ptr,
                                            uint8_t  *xi_runer_C_microcode_ptr,
                                            uint8_t  *xi_runer_D_microcode_ptr )
{
    RUNNER_INST_MAIN  *sram_fast_program_ptr;
    RUNNER_INST_PICO  *sram_pico_program_ptr;


#ifndef RDD_BASIC
    /* load the code segment into the SRAM program memory of fast Runner A */
    sram_fast_program_ptr = ( RUNNER_INST_MAIN * )DEVICE_ADDRESS( RUNNER_INST_MAIN_0_OFFSET );
    MWRITE_BLK_32( sram_fast_program_ptr, xi_runer_A_microcode_ptr, sizeof ( RUNNER_INST_MAIN ) );
#endif

    /* load the code segment into the SRAM program memory of fast Runner B */
    sram_fast_program_ptr = ( RUNNER_INST_MAIN * )DEVICE_ADDRESS( RUNNER_INST_MAIN_1_OFFSET );
    MWRITE_BLK_32( sram_fast_program_ptr, xi_runer_B_microcode_ptr, sizeof ( RUNNER_INST_MAIN ) );

    /* load the code segment into the SRAM program memory of pico Runner A */
    sram_pico_program_ptr = ( RUNNER_INST_PICO * )DEVICE_ADDRESS( RUNNER_INST_PICO_0_OFFSET );
    MWRITE_BLK_32( sram_pico_program_ptr, xi_runer_C_microcode_ptr, sizeof ( RUNNER_INST_PICO ) );

    /* load the code segment into the SRAM program memory of pico Runner B */
    sram_pico_program_ptr = ( RUNNER_INST_PICO * )DEVICE_ADDRESS( RUNNER_INST_PICO_1_OFFSET );
    MWRITE_BLK_32( sram_pico_program_ptr, xi_runer_D_microcode_ptr, sizeof ( RUNNER_INST_PICO ) );

    return ( BL_LILAC_RDD_OK );
}


static void memcpyl_prediction ( void *  __to, void *  __from, unsigned int __n )
{
    uint8_t *src = (uint8_t *)__from;
    uint8_t *dst = (uint8_t *)__to;
    int i;

    for (i = 0; i < (__n / 2); i++, src += 2, dst += 4)
    {
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
        *(volatile unsigned int *)dst = swap4bytes((unsigned int)(*(volatile unsigned short *)src));
#else
        *(volatile unsigned int *)dst = (unsigned int)(*(volatile unsigned short *)src);
#endif
    }
}


BL_LILAC_RDD_ERROR_DTE rdd_load_prediction ( uint8_t  *xi_runer_A_prediction_ptr,
                                             uint8_t  *xi_runer_B_prediction_ptr,
                                             uint8_t  *xi_runer_C_prediction_ptr,
                                             uint8_t  *xi_runer_D_prediction_ptr )
{
    RUNNER_PRED_MAIN  *sram_fast_prediction_ptr;
    RUNNER_PRED_PICO  *sram_pico_prediction_ptr;

    sram_fast_prediction_ptr = ( RUNNER_PRED_MAIN * )DEVICE_ADDRESS( RUNNER_PRED_MAIN_0_OFFSET );
    memcpyl_prediction ( sram_fast_prediction_ptr, xi_runer_A_prediction_ptr, sizeof ( RUNNER_PRED_MAIN ) );

    sram_fast_prediction_ptr = ( RUNNER_PRED_MAIN * )DEVICE_ADDRESS( RUNNER_PRED_MAIN_1_OFFSET );
    memcpyl_prediction ( sram_fast_prediction_ptr, xi_runer_B_prediction_ptr, sizeof ( RUNNER_PRED_MAIN ) );

    sram_pico_prediction_ptr = ( RUNNER_PRED_PICO * )DEVICE_ADDRESS( RUNNER_PRED_PICO_0_OFFSET );
    memcpyl_prediction ( sram_pico_prediction_ptr, xi_runer_C_prediction_ptr, sizeof ( RUNNER_PRED_PICO ) );

    sram_pico_prediction_ptr = ( RUNNER_PRED_PICO * )DEVICE_ADDRESS( RUNNER_PRED_PICO_1_OFFSET );
    memcpyl_prediction ( sram_pico_prediction_ptr, xi_runer_D_prediction_ptr, sizeof ( RUNNER_PRED_PICO ) );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_runner_enable ( void )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_GLOBAL_CTRL  runner_global_control_register;

    /* enable Runner A through the global control register */
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
#ifndef RDD_BASIC
    runner_global_control_register.main_en = LILAC_RDD_TRUE;
#endif
    runner_global_control_register.pico_en = LILAC_RDD_TRUE;
    runner_global_control_register.main_cntxt_reb_en = LILAC_RDD_TRUE;
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );

    /* enable Runner B through the global control register */
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register.main_en = LILAC_RDD_TRUE;
    runner_global_control_register.pico_en = LILAC_RDD_TRUE;
    runner_global_control_register.main_cntxt_reb_en = LILAC_RDD_TRUE;
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_runner_disable ( void )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_GLOBAL_CTRL  runner_global_control_register;

    /* enable the Runner through the global control register */
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register.main_en = LILAC_RDD_FALSE;
    runner_global_control_register.pico_en = LILAC_RDD_FALSE;
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );


    RUNNER_REGS_1_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register.main_en = LILAC_RDD_FALSE;
    runner_global_control_register.pico_en = LILAC_RDD_FALSE;
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_runner_frequency_set ( uint16_t  xi_runner_frequency )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_GLOBAL_CTRL  runner_global_control_register;

    /* set the frequency of the Runner through the global control register */
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register.micro_sec_val = xi_runner_frequency - 1;
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );

    RUNNER_REGS_1_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register.micro_sec_val = xi_runner_frequency - 1;
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_data_structures_init ( RDD_INIT_PARAMS  *init_params )
{
#if defined(WL4908)
    /* initialize the base address of the packets in the ddr */
    /* in 4908, the packet buffer address will be ddr_bm_ptr.
     * where the multicast packet header address will be ddr_fm_ptr + RDP_DDR_DATA_STRUCTURES_SIZE */
    g_runner_ddr_base_addr = init_params->ddr_bm_ptr;
    g_runner_ddr_base_addr_phys = init_params->ddr_bm_phys;
    g_runner_extra_ddr_base_addr = init_params->ddr_fm_ptr + RDP_DDR_DATA_STRUCTURES_SIZE;
    g_runner_extra_ddr_base_addr_phys = init_params->ddr_fm_phys + RDP_DDR_DATA_STRUCTURES_SIZE;
#if !defined(FIRMWARE_INIT)
    /* In simulation these are setup in rdd_sim_alloc_segments */
    NatCacheTableBase = init_params->runner_nat_cache_key_ptr;
    ContextTableBase = init_params->runner_nat_cache_context_ptr;
    ContextContTableBase = init_params->runner_context_cont_ptr;
#endif
    g_nat_cache_table_ptr = ( RDD_NAT_CACHE_TABLE_DTS * )NatCacheTableBase;
    g_bpm_buffer_size = init_params->token_size;
#else
    /* initialize the base address of the packets in the ddr */
    g_runner_ddr_base_addr = init_params->ddr_pool_ptr;
    g_runner_ddr_base_addr_phys = init_params->ddr_pool_ptr_phys;
    g_runner_extra_ddr_base_addr = init_params->extra_ddr_pool_ptr;
    g_runner_extra_ddr_base_addr_phys = init_params->extra_ddr_pool_ptr_phys;
    g_runner_tables_ptr = init_params->ddr_runner_tables_ptr;
    g_ds_connection_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
#if !defined(FIRMWARE_INIT)
    /* In simulation these are setup in rdd_sim_alloc_segments */
    ContextTableBase = g_runner_tables_ptr + CONTEXT_TABLE_ADDRESS;
#endif
#endif

    g_dbg_lvl = 0;
    g_fc_mcast_connection2_table_ptr = ( RDD_FC_MCAST_CONNECTION2_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + FC_MCAST_CONNECTION2_TABLE_ADDRESS );

    g_ddr_headroom_size = init_params->ddr_headroom_size;

    g_broadcom_switch_mode = init_params->broadcom_switch_mode;
    g_broadcom_switch_physical_port = init_params->broadcom_switch_physical_port;

    g_bridge_flow_cache_mode = init_params->bridge_flow_cache_mode;
    g_chip_revision = init_params->chip_revision;

    /* check abs packet limit legal value*/
    if( ( init_params->cpu_tx_abs_packet_limit <= LILAC_RDD_CPU_TX_SKB_LIMIT_MAX ) &&
        ( init_params->cpu_tx_abs_packet_limit >= LILAC_RDD_CPU_TX_SKB_LIMIT_MIN ) &&
        ( init_params->cpu_tx_abs_packet_limit % LILAC_RDD_CPU_TX_SKB_LIMIT_MIN == 0 ) )
    {
        g_cpu_tx_abs_packet_limit = init_params->cpu_tx_abs_packet_limit;
    }
    else
    {
        g_cpu_tx_abs_packet_limit = LILAC_RDD_CPU_TX_SKB_LIMIT_MIN;
    }

#if !defined(WL4908) && !defined(RDD_BASIC)
    /* DDR reset */
    MEMSET ( ( void * )DsConnectionTableBase, 0, sizeof ( RDD_CONNECTION_TABLE_DTS ) );
    MEMSET ( ( void * )UsConnectionTableBase, 0, sizeof ( RDD_CONNECTION_TABLE_DTS ) );
    MEMSET ( ( void * )ContextTableBase, 0, sizeof ( RDD_CONTEXT_TABLE_DTS ) );
#ifdef FIRMWARE_INIT
    MEMSET ( ( void * )cpu_rx_ring_base, 0, sizeof( RDD_CPU_RX_DESCRIPTOR_DTS ) * RDD_RING_DESCRIPTORS_TABLE_SIZE * 10 );
#endif

    /* SRAM reset */
//    MEMSET ( ( void * )g_fc_mcast_connection2_table_ptr, 0, sizeof ( RDD_FC_MCAST_CONNECTION2_TABLE_DTS ) );
#endif

#if defined(WL4908)
    /* initialize the base address of the BPM base address */
    f_rdd_bpm_initialize (init_params->ddr_bm_phys, init_params->ddr1_bm_phys, init_params->ddr_fm_phys + RDP_DDR_DATA_STRUCTURES_SIZE);

    /* initialize runner dma base address */
    f_rdd_ddr_initialize(init_params->ddr_bm_phys, init_params->ddr1_bm_phys, g_ddr_headroom_size);
#else
    /* initialize the base address of the BPM base address */
    f_rdd_bpm_initialize(init_params->ddr_pool_ptr_phys, 0, init_params->extra_ddr_pool_ptr_phys);

    /* initialize runner dma base address */
    f_rdd_ddr_initialize(init_params->ddr_pool_ptr_phys, 0, g_ddr_headroom_size);
#endif

    /* initialize runner dma base address */
    f_rdd_psram_initialize ();

    /* initialize scheduler */
    f_rdd_scheduler_initialize ();

    /* create the Runner's free packet descriptors pool */
    f_rdd_free_packet_descriptors_pool_initialize ();

    /* initialize the CPU-RX mechanism */
    rdd_cpu_rx_initialize ();

    /* initialize the CPU-TX queue */
    rdd_cpu_tx_initialize ();

    /* initialize global registers */
    f_rdd_global_registers_initialize ();

    /* initialize the local registers through the Context memory */
    f_rdd_local_registers_initialize ();

    /* initialize ethernet tx queues and ports */
    f_rdd_eth_tx_initialize ();

    /* initialize WAN tx */
    f_rdd_wan_tx_initialize ();

    /* initialize inter task queues */
    f_rdd_inter_task_queues_initialize ();

    /* initialize PM counters */
    f_rdd_pm_counters_initialize ();

    /* initialize the CRC software variables */
    init_crc ();

#ifndef RDD_BASIC
    /* initialize the MAC HW accelerator and the software CRC functionality */
    f_rdd_mac_table_initialize ( init_params->mac_table_size, init_params->iptv_table_size );
#endif

    /* initialize ingress classification table */
    f_rdd_ingress_classification_table_initialize ();

    /* set up the ETH0 EEE mode config message*/
    MWRITE_32(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_ETH0_EEE_MODE_CONFIG_MESSAGE_ADDRESS,
              (BBH_PERIPHERAL_ETH0_TX<<16)|BBTX_EEE_MODE_CONFIG_MESSAGE);

#ifndef RDD_BASIC
    /* initialize filters cam */
    f_rdd_ingress_filters_cam_initialize ();

    /* initialize connection table */
    f_rdd_connection_table_initialize ();
#endif

    /* initialize free skb indexes fifo and pointers*/
    f_rdd_transmit_from_abs_address_initialize ();

    /* Part of the bridge initialization. */
    MWRITE_16( (DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR_ADDRESS), DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS );

    /* initialize structures supporting parallel processing */
    f_rdd_parallel_processing_initialize ();

    /* set to not configured */
    rdd_ethwan2_switch_port_config(0xff);

#ifndef RDD_BASIC
    rdd_service_queues_initialize ();
#endif

    /* initialize ds rate limit exponent table */
    f_rdd_ds_exponent_table_initialize ();

#if !defined(FIRMWARE_INIT) && defined ( USE_BDMF_SHELL )
    /* register shell commands */
    f_rdd_make_shell_commands ();
#endif

#ifdef CONFIG_DHD_RUNNER
    rdd_dhd_mode_enable_init();
#endif

#if !defined(RDD_BASIC)
#if defined(DSL_63138)
    if (init_params->lp_mode)
        rdd_cpu_total_pps_rate_limiter_config(400 /*Kpps*/);
#endif

#if defined(CONFIG_BCM_SPDSVC_SUPPORT)
    rdd_spdsvc_initialize();
#endif
#endif

    return ( BL_LILAC_RDD_OK );
}

int rdd_ddr_headroom_get(void)
{
    return (g_ddr_headroom_size + DDR_PACKET_PAYLOAD_OFFSET);
}

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_bpm_initialize                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize BPM                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the operation                        */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_runner_ddr_pool_ptr - Packet DDR buffer base address                  */
/*   xi_extra_ddr_pool_ptr - Packet DDR buffer base address (Multicast)       */
/*   xi_ddr_headroom_size - configurable headroom in addition to              */
/*   LILAC_RDD_PACKET_DDR_OFFSET                                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_bpm_initialize(uint32_t runner_ddr_pool_phys,
                                                   uint32_t runner_ddr1_pool_phys,
                                                   uint32_t runner_extra_ddr_pool_phys)
{
    uint32_t *bpm_ddr_base_ptr;
    uint32_t *bpm_extra_ddr_base_ptr;
#if defined(WL4908)
    uint32_t *bpm_ddr1_base_ptr;
#endif

    bpm_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_base_ptr, runner_ddr_pool_phys);
    bpm_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_base_ptr, runner_ddr_pool_phys);

#if defined(WL4908)
    bpm_ddr1_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_1_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr1_base_ptr, runner_ddr1_pool_phys);
    bpm_ddr1_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_1_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr1_base_ptr, runner_ddr1_pool_phys);
#endif

    bpm_extra_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_EXTRA_DDR_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_extra_ddr_base_ptr, runner_extra_ddr_pool_phys);

    bpm_extra_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_EXTRA_DDR_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_extra_ddr_base_ptr, runner_extra_ddr_pool_phys);

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_ddr_initialize                                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize the runner ddr config register        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the operation                        */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   DDR_config Register                                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_runner_ddr_pool_phys - Packet DDR buffer base address                 */
/*   xi_ddr_headroom_size - configurable headroom in addition to              */
/*   LILAC_RDD_PACKET_DDR_OFFSET                                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_ddr_initialize(uint32_t xi_runner_ddr_pool_phys,
                                                   uint32_t xi_runner_ddr1_pool_phys,
                                                   uint32_t xi_ddr_headroom_size)
{
    RUNNER_REGS_CFG_DDR_CFG         runner_ddr_config_register;
    RUNNER_REGS_CFG_DDR_LKUP_MASK0  runner_ddr_lkup_mask0_register;
    RUNNER_REGS_CFG_DDR_LKUP_MASK1  runner_ddr_lkup_mask1_register;
    uint32_t                        *ddr_address_ptr; /* DSL */

    runner_ddr_config_register.buffer_offset = LILAC_RDD_PACKET_DDR_OFFSET;
    runner_ddr_config_register.rserved1 = 0;
    runner_ddr_config_register.dma_base = (xi_runner_ddr_pool_phys & 0x07E00000) >> 21;
#if defined(WL4908)
    runner_ddr_config_register.dma2_base = (xi_runner_ddr1_pool_phys & 0xFFE00000) >> 21;
    if (g_bpm_buffer_size == 512)
        runner_ddr_config_register.buffer_size = RUNNER_REGS_CFG_DDR_CFG_BUFFER_SIZE_BUFFER_SIZE_512_VALUE;
    else
        runner_ddr_config_register.buffer_size = RUNNER_REGS_CFG_DDR_CFG_BUFFER_SIZE_BUFFER_SIZE_256_VALUE;
#else
    runner_ddr_config_register.buffer_size = RDP_CFG_BUF_SIZE_VALUE;
    runner_ddr_config_register.rserved2 = 0;
#endif

    RUNNER_REGS_0_CFG_DDR_CFG_WRITE ( runner_ddr_config_register );
    RUNNER_REGS_1_CFG_DDR_CFG_WRITE ( runner_ddr_config_register );

    /* DDR lookup for routed packet - 5 tupples */
    runner_ddr_lkup_mask0_register.global_mask = 0x000001FF;

    RUNNER_REGS_0_CFG_DDR_LKUP_MASK0_WRITE ( runner_ddr_lkup_mask0_register );
    RUNNER_REGS_1_CFG_DDR_LKUP_MASK0_WRITE ( runner_ddr_lkup_mask0_register );

    /* DDR lookup for IPTV table - destination MAC, destination MAC + VLAN, destination IP */
    runner_ddr_lkup_mask1_register.global_mask = 0x00000000;

    RUNNER_REGS_0_CFG_DDR_LKUP_MASK1_WRITE ( runner_ddr_lkup_mask1_register );
    RUNNER_REGS_1_CFG_DDR_LKUP_MASK1_WRITE ( runner_ddr_lkup_mask1_register );

    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PACKET_BUFFER_SIZE_ASR_8_ADDRESS );
    MWRITE_8( ddr_address_ptr, g_bpm_buffer_size >> 8 );

    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PACKET_BUFFER_SIZE_ASR_8_ADDRESS );
    MWRITE_8( ddr_address_ptr, g_bpm_buffer_size >> 8 );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_psram_initialize                                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize the runner psram config register      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the operation                        */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   PSRAM_config Register                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_psram_initialize ( void )
{
    RUNNER_REGS_CFG_PSRAM_CFG         runner_psram_config_register;
    RUNNER_REGS_CFG_PSRAM_LKUP_MASK0  runner_psram_lkup_mask0_register;

    runner_psram_config_register.buffer_offset = LILAC_RDD_PACKET_DDR_OFFSET;
    runner_psram_config_register.rserved1 = 0;
#if defined(WL4908)
    runner_psram_config_register.buffer_size = RUNNER_REGS_CFG_PSRAM_CFG_BUFFER_SIZE_BUFFER_SIZE_128_VALUE;
    runner_psram_config_register.rserved1 = 0;
#else
    runner_psram_config_register.buffer_size = RUNNER_REGS_CFG_PSRAM_CFG_BUFFER_SIZE_BUFFER_SIZE_128BYTE_VALUE;
    runner_psram_config_register.rserved2 = 0;
#endif
    runner_psram_config_register.dma_base = 0;

    RUNNER_REGS_0_CFG_PSRAM_CFG_WRITE ( runner_psram_config_register );
    RUNNER_REGS_1_CFG_PSRAM_CFG_WRITE ( runner_psram_config_register );


    /* PSRAM lookup for data collection - 5 tupples & layer 2 */
    runner_psram_lkup_mask0_register.global_mask = 0x0000FFFF;

    RUNNER_REGS_0_CFG_PSRAM_LKUP_MASK0_WRITE ( runner_psram_lkup_mask0_register );
    RUNNER_REGS_1_CFG_PSRAM_LKUP_MASK0_WRITE ( runner_psram_lkup_mask0_register );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_scheduler_initialize                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize the scheduler config register         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the operation                        */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   DDR_config Register                                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_scheduler_initialize ( void )
{
    uint32_t  runner_scheduler_cfg_register;

    /* fast Runner A - class C */
    runner_scheduler_cfg_register = ( RUNNER_REGS_CFG_MAIN_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE << 5 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_A_DONT_USE_CLASS_A_VALUE << 4 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_31_24_RR_VALUE << 3 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_23_16_RR_VALUE << 2 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_15_8_RR_VALUE << 1 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0 );

    RUNNER_REGS_0_CFG_MAIN_SCH_CFG_WRITE ( runner_scheduler_cfg_register );

    /* fast Runner B - class C */
    runner_scheduler_cfg_register = ( RUNNER_REGS_CFG_MAIN_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE << 5 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_A_DONT_USE_CLASS_A_VALUE << 4 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_31_24_RR_VALUE << 3 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_23_16_RR_VALUE << 2 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_15_8_RR_VALUE << 1 ) |
                                    ( RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0 );

    RUNNER_REGS_1_CFG_MAIN_SCH_CFG_WRITE ( runner_scheduler_cfg_register );

    /* pico Runner A - class A */
    runner_scheduler_cfg_register = ( RUNNER_REGS_CFG_PICO_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE << 5 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_A_USE_CLASS_A_VALUE << 4 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_15_8_RR_VALUE << 1 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_RR_VALUE << 0 );

    RUNNER_REGS_0_CFG_PICO_SCH_CFG_WRITE ( runner_scheduler_cfg_register );

    /* pico Runner B - class A */
    runner_scheduler_cfg_register = ( RUNNER_REGS_CFG_PICO_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE << 5 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_A_USE_CLASS_A_VALUE << 4 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_15_8_RR_VALUE << 1 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_RR_VALUE << 0 );

    RUNNER_REGS_1_CFG_PICO_SCH_CFG_WRITE ( runner_scheduler_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_free_packet_descriptors_pool_initialize                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize the list of the free buffers pool     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   Upstream pool is implemented as a stack of 3072 packet descriptors       */
/*   Downstream pool is implemented as a list of 2048 packet descriptors      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_free_packet_descriptors_pool_initialize ( void )
{
    RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_DTS                   *ds_free_packet_descriptors_pool_ptr;
    RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS                   *us_free_packet_descriptors_pool_ptr;
    RDD_PACKET_DESCRIPTOR_DTS                                 *packet_descriptor_ptr;
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS     *free_packet_descriptors_pool_descriptor_ptr;
    RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS  *us_free_packet_descriptors_pool_descriptor_ptr;
    uint32_t                                                  next_packet_descriptor_address;
    uint32_t                                                  i;

    ds_free_packet_descriptors_pool_ptr = ( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS );

    /* create the free packet descriptors pool as a list of packet descriptors */
    for ( i = 0; i < RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE; i++ )
    {
        packet_descriptor_ptr = &( ds_free_packet_descriptors_pool_ptr->entry[ i ].packet_descriptor );

        /* the last packet descriptor should point to NULL, the others points to the next packet descriptor */
        if ( i == ( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1 ) )
        {
            next_packet_descriptor_address = 0;
        }
        else
        {
            next_packet_descriptor_address = DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ( i + 1 ) * sizeof(RDD_PACKET_DESCRIPTOR_DTS);
        }

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_WRITE ( next_packet_descriptor_address, packet_descriptor_ptr );
    }

    free_packet_descriptors_pool_descriptor_ptr = ( RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS );

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_WRITE ( DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS, free_packet_descriptors_pool_descriptor_ptr );

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_WRITE ( DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1 ) * sizeof(RDD_PACKET_DESCRIPTOR_DTS),
                                                                           free_packet_descriptors_pool_descriptor_ptr );

    us_free_packet_descriptors_pool_ptr = ( RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS );

    /* create the free packet descriptors pool as a stack of packet descriptors */
    for ( i = 0; i < RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE; i++ )
    {
        packet_descriptor_ptr = &( us_free_packet_descriptors_pool_ptr->entry[ i ].packet_descriptor );

        /* the last packet descriptor should point to NULL, the others points to the next packet descriptor */
        if ( i == ( RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1 ) )
        {
            next_packet_descriptor_address = 0;
        }
        else
        {
            next_packet_descriptor_address = US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ( i + 1 ) * sizeof(RDD_PACKET_DESCRIPTOR_DTS);
        }

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_WRITE ( next_packet_descriptor_address, packet_descriptor_ptr );
    }

    us_free_packet_descriptors_pool_descriptor_ptr = ( RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS );

    RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_GUARANTEED_THRESHOLD_WRITE ( US_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD, us_free_packet_descriptors_pool_descriptor_ptr );
    RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_GUARANTEED_FREE_COUNT_WRITE (US_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE, us_free_packet_descriptors_pool_descriptor_ptr );
    RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_NON_GUARANTEED_FREE_COUNT_WRITE ( RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE - US_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE,  us_free_packet_descriptors_pool_descriptor_ptr );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_global_registers_initialize                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize the global registers (R1-R7)          */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   Runners global registers (R1-R7)                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_global_registers_initialize ( void )
{
    uint32_t  *global_register_init_ptr;
    uint32_t  global_register[ 8 ];


    /********** Fast Runner A **********/

    /* zero all global registers */
    memset ( global_register, 0, sizeof ( global_register ) );

    /* R1 - constant one */
    global_register[ 1 ] = 1;

    global_register[ 2 ] = ( g_broadcom_switch_mode << DS_GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET ) |
                           ( 1 << DS_GLOBAL_CFG_FLOW_CACHE_MODE_BIT_OFFSET ) |
                           ( g_bridge_flow_cache_mode << DS_GLOBAL_CFG_BRIDGE_FLOW_CACHE_MODE_BIT_OFFSET ) |
                           ( g_chip_revision << DS_GLOBAL_CFG_CHIP_REVISION_OFFSET );

    global_register[ 3 ] = ( DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS << 16 ) | DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS;
    global_register[ 4 ] = DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS << 16 | DS_SQ_ENQUEUE_QUEUE_ADDRESS;
    global_register[ 6 ] = ( DOWNSTREAM_MULTICAST_INGRESS_QUEUE_ADDRESS << 16 ) | DOWNSTREAM_MULTICAST_INGRESS_QUEUE_ADDRESS;

#if defined(CONFIG_DHD_RUNNER)
    global_register[ 7 ] = DS_DHD_TX_POST_INGRESS_QUEUE_ADDRESS << 16;
#endif

    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS );

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at task -1 (initialization task) */
    MWRITE_BLK_32( global_register_init_ptr, global_register, sizeof ( global_register ) );


    /********** Fast Runner B **********/

    /* zero all global registers */
    memset ( global_register, 0, sizeof ( global_register ) );

    /* R1 - constant one */
    global_register[ 1 ] = 1;

    /* R2 - head pointer of the free buffers pool stack */
    global_register[ 2 ] = US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS;

    /* R4 - Not used */

    global_register[ 7 ] = ( g_broadcom_switch_mode << US_GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET ) |
                           ( g_chip_revision << US_GLOBAL_CFG_CHIP_REVISION_OFFSET );

    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS );

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at task -1 (initialization task) */
    MWRITE_BLK_32( global_register_init_ptr, global_register, sizeof ( global_register ) );


    /********** Pico Runner A **********/

    /* zero all global registers */
    memset ( global_register, 0, sizeof ( global_register ) );

    /* R1 - constant one */
    global_register[ 1 ] = 1;

    global_register[ 2 ] = ( g_chip_revision << DS_GLOBAL_CFG_CHIP_REVISION_OFFSET );

#if defined(CONFIG_RUNNER_GSO)
    global_register[ 3 ] = GSO_PICO_QUEUE_ADDRESS;
#endif

#if defined(WL4908_EAP)
    global_register[ 4 ] = ( CAPWAPF_CPU_PROCESSING_TASK_REORDER_FIFO_ADDRESS << 16 ) | CAPWAPF_CPU_PROCESSING_TASK_REORDER_FIFO_ADDRESS;

    /* bitmask of CAPWAP fragmentation tasks being used */
    global_register[ 5 ] = 7; /* three tasks */
#endif

    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS );

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at task -1 (initialization task) */
    MWRITE_BLK_32( global_register_init_ptr, global_register, sizeof ( global_register ) );


    /********** Pico Runner B **********/

    /* zero all global registers */
    memset ( global_register, 0, sizeof ( global_register ) );

    /* R1 - constant one */
    global_register[ 1 ] = 1;

    global_register[ 3 ] = US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS;
    global_register[ 3 ] |= US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS << 16;

    /* R4 - context_index_cache_write_index */
    global_register[ 4 ] = 0;
#if defined(CONFIG_DHD_RUNNER)
    global_register[ 2 ] = US_CPU_TX_BBH_DESCRIPTORS_ADDRESS << 16;
    global_register[ 5 ] = US_DHD_TX_POST_INGRESS_QUEUE_ADDRESS << 16;
#endif
    global_register[ 7 ] = ( g_broadcom_switch_mode << US_GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET ) |
                           ( g_chip_revision << US_GLOBAL_CFG_CHIP_REVISION_OFFSET );

    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS );

    MWRITE_BLK_32( global_register_init_ptr, global_register, sizeof ( global_register ) );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_local_registers_initialize                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize context memeories of 4 Runners        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   initialize the local registers (R8-R31), 32 threads for fast Runners     */
/*   and 16 threads for Pico Runners                                          */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   Runners local registers (R8-R31)                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_local_registers_initialize ( void )
{
    RUNNER_CNTXT_MAIN  *sram_fast_context_ptr;
    RUNNER_CNTXT_PICO  *sram_pico_context_ptr;
    static uint32_t    local_register[ 32 ][ 32 ];

    /********** Fast Runner A **********/

    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_0_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_fast_context_ptr, sizeof ( RUNNER_CNTXT_MAIN ) );

    /* CPU TX fast */
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, cpu_tx_wakeup_request) << 16;
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R8  ] = ( CPU_TX_FAST_QUEUE_ADDRESS << 16 );
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R9  ] = ( INGRESS_HANDLER_BUFFER_ADDRESS << 16 ) | DS_CPU_TX_BBH_DESCRIPTORS_ADDRESS;
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R10 ] = ( BBH_PERIPHERAL_IH << 16 ) | ( LILAC_RDD_IH_BUFFER_BBH_ADDRESS + LILAC_RDD_RUNNER_A_IH_BUFFER_BBH_OFFSET );
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;

    /* CPU-RX */
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, cpu_rx_wakeup_request) << 16;
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R8  ] = CPU_RX_FAST_PD_INGRESS_QUEUE_ADDRESS;
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R9  ] = DS_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS | ( DS_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS << 16 );
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( CPU_RX_SQ_PD_INGRESS_QUEUE_ADDRESS << 16 );
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R11 ] = DS_CPU_REASON_TO_METER_TABLE_ADDRESS | ( CPU_RX_PD_INGRESS_QUEUE_ADDRESS << 16 );

#if defined(CONFIG_DHD_RUNNER)
    /* DHD TX complete Fast_A */
    local_register[ DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_complete_wakeup_request) << 16;
    local_register[ DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R8 ] =  ( DHD_RADIO_OFFSET_COMMON_B(0) << 16 ) | DHD_RADIO_OFFSET_COMMON_A(0);

    /* DHD1 TX complete Fast_A */
    local_register[ DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_complete_wakeup_request) << 16;
    local_register[ DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R8 ] =  ( DHD_RADIO_OFFSET_COMMON_B(1) << 16 ) | DHD_RADIO_OFFSET_COMMON_A(1);

    /* DHD2 TX complete Fast_A */
    local_register[ DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_complete_wakeup_request) << 16;
    local_register[ DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R8 ] =  ( DHD_RADIO_OFFSET_COMMON_B(2) << 16 ) | DHD_RADIO_OFFSET_COMMON_A(2);

    /* DHD TX post Fast_A (WAN->Dongle) */
    local_register[ DHD_TX_POST_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_post) << 16;
    local_register[ DHD_TX_POST_FAST_A_THREAD_NUMBER ][ CS_R9 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD_TX_POST_FAST_A_THREAD_NUMBER ][ CS_R10 ] = DHD_TX_POST_PD_INGRESS_QUEUE_ADDRESS | ( DS_DHD_TX_POST_INGRESS_QUEUE_ADDRESS << 16 );
#endif

    /* Timer scheduler */
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R19 ] = 0;  /* RX_METER_INDEX */
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R21 ] = DS_CPU_RX_METER_TABLE_ADDRESS;

    /* DS Policers budget allocator */
    local_register[ POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, policer_budget_allocator_1st_wakeup_request) << 16;

#if defined(CONFIG_DSLWAN)
    /* WAN direct */
    local_register[ WAN_DIRECT_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, wan_direct_wakeup_request) << 16;
    local_register[ WAN_DIRECT_THREAD_NUMBER ][ CS_R8  ] = GPON_RX_DIRECT_DESCRIPTORS_ADDRESS;
    local_register[ WAN_DIRECT_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ WAN_DIRECT_THREAD_NUMBER ][ CS_R10 ] |= CPU_REASON_WAN0_TABLE_INDEX << 16;
#endif

    /* WAN Filters and Classification */
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, wan_normal_wakeup_request) << 16;
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] = GPON_RX_NORMAL_DESCRIPTORS_ADDRESS << 16 | BBH_PERIPHERAL_WAN_RX;
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R9  ] = CAM_RESULT_SLOT_1 | ( CAM_RESULT_IO_ADDRESS_1 << 16 );
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | CPU_REASON_WAN0_TABLE_INDEX << 16;
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_0 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_0 << 16 );
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R14 ] = WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER;


    /* WAN1 Filters and Classification */
    local_register[ WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, wan_normal_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] = ( GPON_RX_NORMAL_DESCRIPTORS_ADDRESS << 16 ) | BBH_PERIPHERAL_WAN_RX;
#else
    local_register[ WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] = ( ETH0_RX_DESCRIPTORS_ADDRESS << 16 ) | BBH_PERIPHERAL_ETH0_RX;
#endif
    local_register[ WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R9  ] = CAM_RESULT_SLOT_2 | ( CAM_RESULT_IO_ADDRESS_2 << 16 );
    local_register[ WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( CPU_REASON_WAN1_TABLE_INDEX << 16 );
    local_register[ WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_1 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_1 << 16 );
    local_register[ WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R14 ] = WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER;

    /* ETHWAN2 Filters and Classification */
    // FIXME!!! since this is a different thread from WAN1_FILTER... doesn't it require its own CAM_RESULT and DMA_LOOKUP?
    local_register[ ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, ethwan2_normal_wakeup_request) << 16;
    local_register[ ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] = (ETHWAN2_RX_INGRESS_QUEUE_ADDRESS <<16 ) | ( 1 << WAN_FILTERS_AND_CLASSIFICATON_R8_ETHWAN2_INDICATION_OFFSET ) | BBH_PERIPHERAL_ETH0_RX;
    local_register[ ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R9  ] = CAM_RESULT_SLOT_2 | ( CAM_RESULT_IO_ADDRESS_2 << 16 );
    local_register[ ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( CPU_REASON_WAN1_TABLE_INDEX << 16 );
    local_register[ ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_1 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_1 << 16 );
    local_register[ ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R14 ] = ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER;
    
    /* FLOW_CACHE */
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0200) << 16) | (DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0000);
#else
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_BUFFER_TABLE_ADDRESS + 0 * RDD_DS_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (DS_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0000);
#endif
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R9  ] = ( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS << 16 ) | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE0_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
        
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0220) << 16) | (DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0080);
#else
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_BUFFER_TABLE_ADDRESS + 1 * RDD_DS_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (DS_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0010);
#endif
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R9  ] = (( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 1 ) << 16 ) | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE1_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
        
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0240) << 16) | (DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0100);
#else
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_BUFFER_TABLE_ADDRESS + 2 * RDD_DS_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (DS_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0020);
#endif
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R9  ] = (( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 2 ) << 16 ) | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE2_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;

    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0260) << 16) | (DS_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0180);
#else
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R8  ] = ((DS_CONNECTION_BUFFER_TABLE_ADDRESS + 3 * RDD_DS_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (DS_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0030);
#endif
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R9  ] = (( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 3 ) << 16 )  | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE3_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;

#if defined(CONFIG_DSLWAN)
    /* CPU downstream Filters and Classification */
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, wan_cpu_wakeup_request) << 16;
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] = ( DS_CPU_TX_BBH_DESCRIPTORS_ADDRESS << 16 ) | ( 1 << WAN_FILTERS_AND_CLASSIFICATON_R8_CPU_INDICATION_OFFSET );
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_2 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_2 << 16 );
#endif

    /* Downstream Multicast */
    local_register[ DOWNSTREAM_MULTICAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, downstream_multicast_wakeup_request) << 16;
    local_register[ DOWNSTREAM_MULTICAST_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* Free SKB index */
    local_register[ FREE_SKB_INDEX_FAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, free_skb_index_wakeup_request) << 16;

#if defined(CONFIG_RUNNER_IPSEC)
    /* IPsec downstream processing */
    local_register[ IPSEC_DOWNSTREAM_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, ipsec_ds_wakeup_request) << 16;
    local_register[ IPSEC_DOWNSTREAM_THREAD_NUMBER ][ CS_R8  ] = IPSEC_DS_QUEUE_ADDRESS;
    local_register[ IPSEC_DOWNSTREAM_THREAD_NUMBER ][ CS_R9 ]  = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ IPSEC_DOWNSTREAM_THREAD_NUMBER ][ CS_R10 ] = ( BBH_PERIPHERAL_IH << 16 ) | ( LILAC_RDD_IH_BUFFER_BBH_ADDRESS + LILAC_RDD_RUNNER_A_IH_BUFFER_BBH_OFFSET );
    local_register[ IPSEC_DOWNSTREAM_THREAD_NUMBER ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;
#endif

#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#else
    rdp_mm_cpyl_context ( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#endif


    /********** Fast Runner B **********/

    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_1_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_fast_context_ptr, sizeof ( RUNNER_CNTXT_MAIN ) );

    /* CPU-TX */
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, cpu_tx_wakeup_request) << 16;
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R8  ] = CPU_TX_FAST_QUEUE_ADDRESS;

    /* CPU-RX */
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, cpu_rx_wakeup_request) << 16;
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R9  ] = US_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS + ( US_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS << 16 );
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | (CPU_RX_SQ_PD_INGRESS_QUEUE_ADDRESS << 16);
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R11 ] = US_CPU_REASON_TO_METER_TABLE_ADDRESS;

    /* upstream rate controllers budget allocator */
    local_register[ RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R14 ] = US_RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS;
    local_register[ RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, rate_control_budget_allocator_1st_wakeup_request) << 16;
    local_register[ RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R18 ] = US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_ADDRESS;
    local_register[ RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R31 ] = 0; /* rate_controllers_group */

    /* Timer scheduler */
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R19 ] = 0;  /* RX_METER_INDEX */
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R20 ] = US_RATE_LIMITER_TABLE_ADDRESS;
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R21 ] = US_CPU_RX_METER_TABLE_ADDRESS;

    /* US Policers budget allocator */
    local_register[ POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, policer_budget_allocator_1st_wakeup_request) << 16;

#if defined(WL4908)
    /* RX Buffer Copy */
    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, rx_buffer_copy_wakeup_request) << 16;
    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER ][ CS_R8  ] = ETH0_RX_DESCRIPTORS_ADDRESS;
    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER ][ CS_R19 ] = ( BBH_PERIPHERAL_SBPM + SBPM_REPLY_SET_1 ) | (( SBPM_REPLY_ADDRESS + SBPM_REPLY_GET_NEXT_OFFSET + SBPM_REPLY_SET_1_OFFSET ) << 16 );
    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER ][ CS_R20 ] = LAN_DISPATCH_THREAD_WAKEUP_REQUEST_VALUE;
    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER ][ CS_R21 ] = ( US_0_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS ) | ( US_0_MCAST_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS << 16 );

    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, rx_buffer_copy_wakeup_request) << 16;
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R8  ] = ETH1_RX_DESCRIPTORS_ADDRESS;
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R19 ] = ( BBH_PERIPHERAL_SBPM + SBPM_REPLY_SET_2 ) | (( SBPM_REPLY_ADDRESS + SBPM_REPLY_GET_NEXT_OFFSET + SBPM_REPLY_SET_2_OFFSET ) << 16 );
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R20 ] = LAN1_DISPATCH_THREAD_WAKEUP_REQUEST_VALUE;
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R21 ] = ( US_1_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS ) | ( US_1_MCAST_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS << 16 );

    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, rx_buffer_copy_wakeup_request) << 16;
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R8  ] = ETH2_RX_DESCRIPTORS_ADDRESS;
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R19 ] = ( BBH_PERIPHERAL_SBPM + SBPM_REPLY_SET_3 ) | (( SBPM_REPLY_ADDRESS + SBPM_REPLY_GET_NEXT_OFFSET + SBPM_REPLY_SET_3_OFFSET ) << 16 );
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R20 ] = LAN2_DISPATCH_THREAD_WAKEUP_REQUEST_VALUE;
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R21 ] = ( US_2_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS ) | ( US_2_MCAST_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS << 16 );
#endif
#if defined(WL4908_EAP)
    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER  ][ CS_R8  ] |= CAPWAPR0_RX_DESCRIPTORS_ADDRESS << 16;
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R8  ] |= CAPWAPR1_RX_DESCRIPTORS_ADDRESS << 16;
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R8  ] |= CAPWAPR2_RX_DESCRIPTORS_ADDRESS << 16;

    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER  ][ CS_R22 ] |= (CAPWAPR_FLUSH_TABLE_ADDRESS + (0 * RDD_CAPWAPR_FLUSH_TABLE_SIZE2));
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R22 ] |= (CAPWAPR_FLUSH_TABLE_ADDRESS + (1 * RDD_CAPWAPR_FLUSH_TABLE_SIZE2));
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R22 ] |= (CAPWAPR_FLUSH_TABLE_ADDRESS + (2 * RDD_CAPWAPR_FLUSH_TABLE_SIZE2));

    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER  ][ CS_R23 ]  = 0x1111; /* CAPWAP context age list bit mask */
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R23 ]  = 0x1111; /* CAPWAP context age list bit mask */
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R23 ]  = 0x1111; /* CAPWAP context age list bit mask */

    local_register[ US_RX_BUFFER_COPY_THREAD_NUMBER  ][ CS_R23 ] |= 0x1111 << 16; /* CAPWAP context allocation map bit mask */
    local_register[ US_RX_BUFFER_COPY1_THREAD_NUMBER ][ CS_R23 ] |= 0x1111 << 16; /* CAPWAP context allocation map bit mask */
    local_register[ US_RX_BUFFER_COPY2_THREAD_NUMBER ][ CS_R23 ] |= 0x1111 << 16; /* CAPWAP context allocation map bit mask */
#endif

    /* WAN1-TX */
    local_register[ WAN1_TX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, wan_tx_wakeup_request) << 16;
    local_register[ WAN1_TX_THREAD_NUMBER ][ CS_R8 ] = ( RDD_WAN1_CHANNEL_BASE << 16 ) | ( DATA_POINTER_DUMMY_TARGET_ADDRESS + 4 );
    local_register[ WAN1_TX_THREAD_NUMBER ][ CS_R9 ] = ( ETHWAN_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS << 16 ) | ETHWAN_ABSOLUTE_TX_BBH_COUNTER_ADDRESS;

#if defined(CONFIG_DSLWAN)
    /* WAN-TX */
    local_register[ WAN_TX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, wan_tx_wakeup_request) << 16;
    local_register[ WAN_TX_THREAD_NUMBER ][ CS_R8 ] = ( RDD_WAN0_CHANNEL_BASE << 16 ) | ( DATA_POINTER_DUMMY_TARGET_ADDRESS + 0 );
    local_register[ WAN_TX_THREAD_NUMBER ][ CS_R9 ] = ( GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS << 16 ) | GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS;
#endif

#if defined(CONFIG_DHD_RUNNER)
    /* DHD TX post Fast_B (LAN->Dongle) */
    local_register[ DHD_TX_POST_FAST_B_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_b, dhd_tx_post) << 16;
    local_register[ DHD_TX_POST_FAST_B_THREAD_NUMBER ][ CS_R9 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD_TX_POST_FAST_B_THREAD_NUMBER ][ CS_R10 ] = US_DHD_TX_POST_INGRESS_QUEUE_ADDRESS << 16;
#endif

    /* WAN enqueue (Flow Cache) */
    local_register[ WAN_ENQUEUE_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, wan_interworking_enqueue_wakeup_request) << 16;
    local_register[ WAN_ENQUEUE_THREAD_NUMBER ][ CS_R9 ] = ( WAN_ENQUEUE_INGRESS_QUEUE_ADDRESS << 16 ) | ADDRESS_OF(runner_b, wan_interworking_enqueue_wakeup_request);
    local_register[ WAN_ENQUEUE_THREAD_NUMBER ][ CS_R10  ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* Timer 7 */
    local_register[ US_TIMER_7_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, timer_7_1st_wakeup_request) << 16;

    /* Free SKB index */
    local_register[ FREE_SKB_INDEX_FAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, free_skb_index_wakeup_request) << 16;

#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#else
    rdp_mm_cpyl_context ( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#endif


    /********** Pico Runner A **********/

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_0_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_pico_context_ptr, sizeof ( RUNNER_CNTXT_PICO ) );

    /* CPU-TX */
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, cpu_tx_wakeup_request) << 16;
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R9  ] = CPU_TX_PICO_QUEUE_ADDRESS;
#if defined(WL4908_EAP)
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R9  ] |= CAPWAPF_CONTEXT_BUFFER_ADDRESS << 16;
#endif

#if defined(CONFIG_RUNNER_GSO)
    /* GSO */
    local_register[ GSO_PICO_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, gso_wakeup_request) << 16;
#endif

#if defined(WL4908_EAP)
    /* CAPWAP fragmentation  */
    local_register[ CAPWAPF_CPU_PROCESSING0_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, capwapf_cpu_processing_wakeup_request) << 16;
    local_register[ CAPWAPF_CPU_PROCESSING0_THREAD_NUMBER - 32 ][ CS_R8  ] = (CAPWAPF_PROCESSING_MASK_TASK0 << 24) |
        (CAPWAPF_CPU_PROCESSING0_THREAD_NUMBER << 16) | (CAPWAPF_CONTEXT_BUFFER_ADDRESS + (0 * (CAPWAPF_CONTEXT_BUFFER_BYTE_SIZE / 3)));
    local_register[ CAPWAPF_CPU_PROCESSING0_THREAD_NUMBER - 32 ][ CS_R9 ] = CAPWAPF0_CPU_TX_SCRATCHPAD_ADDRESS;

    local_register[ CAPWAPF_CPU_PROCESSING1_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, capwapf_cpu_processing_wakeup_request) << 16;
    local_register[ CAPWAPF_CPU_PROCESSING1_THREAD_NUMBER - 32 ][ CS_R8  ] = (CAPWAPF_PROCESSING_MASK_TASK1 << 24) |
        (CAPWAPF_CPU_PROCESSING1_THREAD_NUMBER << 16) | (CAPWAPF_CONTEXT_BUFFER_ADDRESS + (1 * (CAPWAPF_CONTEXT_BUFFER_BYTE_SIZE / 3)));
    local_register[ CAPWAPF_CPU_PROCESSING1_THREAD_NUMBER - 32 ][ CS_R9 ] = CAPWAPF1_CPU_TX_SCRATCHPAD_ADDRESS;

    local_register[ CAPWAPF_CPU_PROCESSING2_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, capwapf_cpu_processing_wakeup_request) << 16;
    local_register[ CAPWAPF_CPU_PROCESSING2_THREAD_NUMBER - 32 ][ CS_R8  ] = (CAPWAPF_PROCESSING_MASK_TASK2 << 24) |
        (CAPWAPF_CPU_PROCESSING2_THREAD_NUMBER << 16) | (CAPWAPF_CONTEXT_BUFFER_ADDRESS + (2 * (CAPWAPF_CONTEXT_BUFFER_BYTE_SIZE / 3)));
    local_register[ CAPWAPF_CPU_PROCESSING2_THREAD_NUMBER - 32 ][ CS_R9 ] = CAPWAPF2_CPU_TX_SCRATCHPAD_ADDRESS;
#endif

    /* CPU-RX interrupt coalescing timer */
    local_register[ CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, cpu_rx_int_coalesce_timer_1st_wakeup_request) << 16;

    /* Timer scheduler */
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R19 ] = 0; /* rate limiter index */
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R20 ] = DS_RATE_LIMITER_TABLE_ADDRESS;
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R21 ] = RATE_LIMITER_REMAINDER_TABLE_ADDRESS;

#if defined(WL4908)
    /* RX Buffer Copy */
    local_register[ DS_RX_BUFFER_COPY_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, rx_buffer_copy_wakeup_request) << 16;
    local_register[ DS_RX_BUFFER_COPY_THREAD_NUMBER - 32 ][ CS_R8  ] = GPON_RX_NORMAL_DESCRIPTORS_ADDRESS;
    local_register[ DS_RX_BUFFER_COPY_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( CPU_REASON_WAN1_TABLE_INDEX << 16 );
    local_register[ DS_RX_BUFFER_COPY_THREAD_NUMBER - 32 ][ CS_R19 ] = ( BBH_PERIPHERAL_SBPM + SBPM_REPLY_SET_0 ) | (( SBPM_REPLY_ADDRESS + SBPM_REPLY_GET_NEXT_OFFSET + SBPM_REPLY_SET_0_OFFSET ) << 16 );
    local_register[ DS_RX_BUFFER_COPY_THREAD_NUMBER - 32 ][ CS_R20 ] = WAN1_FILTERS_AND_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE;
    local_register[ DS_RX_BUFFER_COPY_THREAD_NUMBER - 32 ][ CS_R21 ] = ( DS_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS ) | ( DS_MCAST_RX_SBPM_TO_FPM_COPY_FPM_ALLOC_RESULT_TABLE_ADDRESS << 16 );
#endif

    /* Local switching LAN enqueue */
    local_register[ LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ( ADDRESS_OF(runner_c, lan_enqueue_pd_wakeup_request) << 16 ) | ADDRESS_OF(runner_c, lan_enqueue_pd_wakeup_request);
    local_register[ LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS;
    local_register[ LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = DOWNSTREAM_LAN_ENQUEUE_SQ_PD_ADDRESS;

    /* Downstream LAN enqueue */
    local_register[ DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, lan_enqueue_ih_wakeup_request) << 16 | ADDRESS_OF(runner_c, lan_enqueue_ih_wakeup_request);
    local_register[ DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS << 16;
    local_register[ DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10  ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* Downstream multicast LAN enqueue */
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ( ADDRESS_OF(runner_c, multicast_lan_enqueue_wakeup_request) << 16 ) | ADDRESS_OF(runner_c, multicast_lan_enqueue_wakeup_request);
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS;
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* Free SKB index */
    local_register[ FREE_SKB_INDEX_PICO_A_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, free_skb_index_wakeup_request) << 16;
    local_register[ FREE_SKB_INDEX_PICO_A_THREAD_NUMBER - 32 ][ CS_R9  ] = 1; /* lag_port EMAC/BBH 1 */

#if defined(CONFIG_WLAN_MCAST)
    /* WLAN Multicast */
    local_register[ WLAN_MCAST_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, wlan_mcast_wakeup_request) << 16;
    local_register[ WLAN_MCAST_THREAD_NUMBER - 32 ][ CS_R8 ] = ( CPU_RX_FAST_PD_INGRESS_QUEUE_ADDRESS << 16 ) | WLAN_MCAST_INGRESS_QUEUE_ADDRESS;
    local_register[ WLAN_MCAST_THREAD_NUMBER - 32 ][ CS_R9 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
#endif

    /* ETH-TX Inter LAN scheduling: thread 42 */
    local_register[ ETH_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, lan_tx_wakeup_request) << 16 | (ADDRESS_OF(runner_c, lan_tx_wakeup_request) );
    local_register[ ETH_TX_THREAD_NUMBER - 32 ][ CS_R8  ] = 0; /* inter_lan_scheduling_offset */

    /* Timer 7 */
    local_register[ DS_TIMER_7_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, timer_7_1st_wakeup_request) << 16;

    /* Service Queue Enqueue: thread 44 */
    local_register[ SERVICE_QUEUE_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, service_queue_enqueue_wakeup_request) << 16;
    local_register[ SERVICE_QUEUE_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ SERVICE_QUEUE_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = DS_SQ_ENQUEUE_QUEUE_ADDRESS;

    /* Service Queue Dequeue: thread 45 */
    local_register[ SERVICE_QUEUE_DEQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, service_queue_dequeue_wakeup_request) << 16 | ADDRESS_OF(runner_c, service_queue_dequeue_wakeup_request);
    local_register[ SERVICE_QUEUE_DEQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = DOWNSTREAM_LAN_ENQUEUE_SQ_PD_ADDRESS << 16;
#if defined(CONFIG_DHD_RUNNER)
    local_register[ SERVICE_QUEUE_DEQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] |= DHD_TX_POST_PD_INGRESS_QUEUE_ADDRESS;
#endif
    local_register[ SERVICE_QUEUE_DEQUEUE_THREAD_NUMBER - 32 ][ CS_R11 ] = CPU_RX_SQ_PD_INGRESS_QUEUE_ADDRESS;

#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
#else
    rdp_mm_cpyl_context ( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
#endif


    /********** Pico Runner B **********/

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_1_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_pico_context_ptr, sizeof ( RUNNER_CNTXT_PICO ) );

    /* Timer scheduler */
    local_register[ TIMER_SCHEDULER_PICO_B_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_PICO_B_THREAD_NUMBER - 32 ][ CS_R19 ] = 0; /* rate limiter index */
    local_register[ TIMER_SCHEDULER_PICO_B_THREAD_NUMBER - 32 ][ CS_R20 ] = US_RATE_LIMITER_TABLE_ADDRESS;

#if defined(CONFIG_DHD_RUNNER)
    /* DHD RX Complete */
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, dhd_rx_complete_wakeup_request) << 16;
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R10 ] = 0; /* counter */
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R13 ] = 0; /* rx complete read idx */
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R15 ] = ( DHD_RADIO_OFFSET_COMMON_B(0) << 16 ) | ( DHD_RX_POST_FLOW_RING_SIZE - 1 );

    /* DHD1 RX Complete */
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, dhd_rx_complete_wakeup_request) << 16;
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R10 ] = 0; /* counter */
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R13 ] = 0; /* rx complete read idx */
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R15 ] = ( DHD_RADIO_OFFSET_COMMON_B(1) << 16 ) | ( DHD_RX_POST_FLOW_RING_SIZE - 1 );

    /* DHD2 RX Complete */
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, dhd_rx_complete_wakeup_request) << 16;
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R10 ] = 0; /* counter */
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R13 ] = 0; /* rx complete read idx */
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R15 ] = ( DHD_RADIO_OFFSET_COMMON_B(2) << 16 ) | ( DHD_RX_POST_FLOW_RING_SIZE - 1 );
#endif

#if defined(DSL_63138) || defined(DSL_63148)
    /* LAN-1 Filters and Classification - used by CFE boot loader */
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_normal_wakeup_request) << 16;
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = ETH1_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( HASH_RESULT_SLOT_1 << 16 ) | ( HASH_RESULT_IO_ADDRESS_1 << 24 );
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH1_RX << 16 );
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_3 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_3 << 16 );
#endif

    /* Free SKB index */
    local_register[ FREE_SKB_INDEX_PICO_B_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, free_skb_index_wakeup_request) << 16;

    /* LAN Dispatch */
#if defined(WL4908)
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_dispatch_wakeup_request) << 16;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] = ETH0_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_4 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_4 << 16 );
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R14 ] = LAN_DISPATCH_THREAD_NUMBER;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R15 ] = BBH_PERIPHERAL_ETH0_RX;

    local_register[ LAN1_DISPATCH_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_dispatch_wakeup_request) << 16;
    local_register[ LAN1_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] = ETH1_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN1_DISPATCH_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ LAN1_DISPATCH_THREAD_NUMBER - 32 ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_5 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_5 << 16 );
    local_register[ LAN1_DISPATCH_THREAD_NUMBER - 32 ][ CS_R14 ] = LAN1_DISPATCH_THREAD_NUMBER;
    local_register[ LAN1_DISPATCH_THREAD_NUMBER - 32 ][ CS_R15 ] = BBH_PERIPHERAL_ETH1_RX;

    local_register[ LAN2_DISPATCH_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_dispatch_wakeup_request) << 16;
    local_register[ LAN2_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] = ETH2_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN2_DISPATCH_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ LAN2_DISPATCH_THREAD_NUMBER - 32 ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_6 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_6 << 16 );
    local_register[ LAN2_DISPATCH_THREAD_NUMBER - 32 ][ CS_R14 ] = LAN2_DISPATCH_THREAD_NUMBER;
    local_register[ LAN2_DISPATCH_THREAD_NUMBER - 32 ][ CS_R15 ] = BBH_PERIPHERAL_ETH2_RX;
#else
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_dispatch_wakeup_request) << 16;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R10  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] = ETH1_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R15 ] = BBH_PERIPHERAL_ETH1_RX;
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_4 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_4 << 16 );
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R14 ] = LAN_DISPATCH_THREAD_NUMBER;
#endif
#if defined(WL4908_EAP)
    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ] [ CS_R8  ] |= CAPWAPR0_RX_DESCRIPTORS_ADDRESS << 16;
    local_register[ LAN1_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] |= CAPWAPR1_RX_DESCRIPTORS_ADDRESS << 16;
    local_register[ LAN2_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] |= CAPWAPR2_RX_DESCRIPTORS_ADDRESS << 16;
#endif

#if defined(CONFIG_DHD_RUNNER)
    /* CPU upstream Filters and Classification */
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_cpu_wakeup_request) << 16;
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = US_CPU_TX_BBH_DESCRIPTORS_ADDRESS | ( 1 << LAN_FILTERS_LAN_TYPE_CPU_BIT_OFFSET );
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = ( DMA_LOOKUP_RESULT_SLOT_7 << 5 ) | DMA_LOOKUP_RESULT_FOUR_STEPS | ( DMA_LOOKUP_RESULT_IO_ADDRESS_7 << 16 );
#endif

    /* SLAVE0 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0200) << 16) | (US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0000);
#else
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_BUFFER_TABLE_ADDRESS + 0 * RDD_US_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (US_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0000);
#endif
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER - 32 ][ CS_R9  ] = ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS << 16 );
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER - 32 ][ CS_R10 ] = ( FLOW_CACHE_SLAVE0_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
        
    /* SLAVE1 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0220) << 16) | (US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0080);
#else
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_BUFFER_TABLE_ADDRESS + 1 * RDD_US_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (US_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0010);
#endif
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 1 ) << 16 );
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER - 32 ][ CS_R10 ] = ( FLOW_CACHE_SLAVE1_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
        
    /* SLAVE2 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0240) << 16) | (US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0100);
#else
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_BUFFER_TABLE_ADDRESS + 2 * RDD_US_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (US_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0020);
#endif
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 2 ) << 16 );
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER - 32 ][ CS_R10 ] = ( FLOW_CACHE_SLAVE2_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
        
    /* SLAVE3 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, flow_cache_wakeup_request) << 16;
#if defined(WL4908)
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0260) << 16) | (US_CONNECTION_CONTEXT_BUFFER_ADDRESS + 0x0180);
#else
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER - 32 ][ CS_R8  ] = (uint32_t) ((US_CONNECTION_BUFFER_TABLE_ADDRESS + 3 * RDD_US_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS)) << 16) | (US_L2_UCAST_CONNECTION_BUFFER_ADDRESS + 0x0030);
#endif
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 3 ) << 16 );
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER - 32 ][ CS_R10 ] = ( FLOW_CACHE_SLAVE3_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;

#if defined(DSL_63138) || defined(DSL_63148)
    /* SLAVE 0-3 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER - 32 ][ CS_R9  ] |= (DSL_PTM_BOND_TX_HDR_TABLE_ADDRESS + 0x00);
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER - 32 ][ CS_R9  ] |= (DSL_PTM_BOND_TX_HDR_TABLE_ADDRESS + 0x10);
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER - 32 ][ CS_R9  ] |= (DSL_PTM_BOND_TX_HDR_TABLE_ADDRESS + 0x20);
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER - 32 ][ CS_R9  ] |= (DSL_PTM_BOND_TX_HDR_TABLE_ADDRESS + 0x30);
#endif

#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
#else
    rdp_mm_cpyl_context ( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
#endif

    return ( BL_LILAC_RDD_OK );
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_transmit_from_abs_address_initialize ( void )
{
    uint8_t   *free_indexes_local_fifo_tail_ptr;
    uint16_t  *free_indexes_fifo_tail_ptr;
    uint16_t  skb_enqueued_indexes_fifo;
    uint16_t  *skb_enqueued_indexes_fifo_ptr;
    uint8_t   *absolute_tx_counters_ptr;
    uint16_t  i;
    uint32_t  *ddr_address_ptr;
    uint8_t   skb_enqueued_indexes_fifo_size;
    uint8_t   *skb_enqueued_indexes_fifo_counters_ptr;

#if !defined(FIRMWARE_INIT)
    bdmf_phys_addr_t phy_addr = 0;

    /* allocate skb pointer array reference (used only by SW) */
#ifndef RDD_BASIC
    g_cpu_tx_skb_pointers_reference_array = (uint8_t **)bdmf_alloc(sizeof(uint8_t *) * g_cpu_tx_abs_packet_limit);
    g_dhd_tx_cpu_usage_reference_array = (uint8_t *)bdmf_alloc(g_cpu_tx_abs_packet_limit);
#else
    g_cpu_tx_skb_pointers_reference_array = (uint8_t **)KMALLOC(sizeof(uint8_t *) * g_cpu_tx_abs_packet_limit, 0);
    g_dhd_tx_cpu_usage_reference_array = (uint8_t *)KMALLOC(g_cpu_tx_abs_packet_limit, 0);
#endif

    /* allocate data pointer array pointer (used both by SW & FW) */
    g_cpu_tx_data_pointers_reference_array = (rdd_phys_addr_t *)rdp_mm_aligned_alloc(sizeof(rdd_phys_addr_t) * g_cpu_tx_abs_packet_limit, &phy_addr);

    ddr_address_ptr = (uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_SKB_DATA_POINTERS_TABLE_ADDRESS );
    MWRITE_32(ddr_address_ptr, (uint32_t)phy_addr);

    /* allocate Free Indexes table (used both by SW & FW) */
    g_free_skb_indexes_fifo_table = ( uint16_t * )rdp_mm_aligned_alloc( sizeof( uint16_t ) * g_cpu_tx_abs_packet_limit, &phy_addr );

    g_free_skb_indexes_fifo_table_physical_address = (rdd_phys_addr_t)phy_addr;
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_FREE_SKB_INDEXES_FIFO_TABLE_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_free_skb_indexes_fifo_table_physical_address );

    g_free_skb_indexes_fifo_table_physical_address_last_idx = g_free_skb_indexes_fifo_table_physical_address;
    g_free_skb_indexes_fifo_table_physical_address_last_idx += (g_cpu_tx_abs_packet_limit - 1) * sizeof(uint16_t);
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_FREE_SKB_INDEXES_FIFO_TABLE_LAST_ENTRY_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_free_skb_indexes_fifo_table_physical_address_last_idx );

    /* Fill free indexes FIFO */
    for ( i = 0; i < g_cpu_tx_abs_packet_limit ; i++ )
    {
        g_free_skb_indexes_fifo_table[ i ] = swap2bytes( i );
        g_cpu_tx_data_pointers_reference_array[ i ] = 0;
        g_cpu_tx_skb_pointers_reference_array[ i ] = NULL;
        g_dhd_tx_cpu_usage_reference_array[ i ] = 0;
    }

#if !defined(RDD_BASIC)
    /* Make sure all DDR tables are in sync before Runner starts */
    wmb();
#endif
#else
    /* allocate data pointer array pointer (used both by SW & FW) */
    g_cpu_tx_data_pointers_reference_array = (rdd_phys_addr_t *)SIMULATOR_DDR_SKB_DATA_POINTERS_OFFSET;

    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_SKB_DATA_POINTERS_TABLE_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_cpu_tx_data_pointers_reference_array );

    /* allocate Free Indexes table (used both by SW & FW) */
    g_free_skb_indexes_fifo_table = (uint16_t *)SIMULATOR_DDR_SKB_FREE_INDEXES_OFFSET;

    g_free_skb_indexes_fifo_table_physical_address = (rdd_phys_addr_t)g_free_skb_indexes_fifo_table;
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_FREE_SKB_INDEXES_FIFO_TABLE_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_free_skb_indexes_fifo_table_physical_address );

    g_free_skb_indexes_fifo_table_physical_address_last_idx = g_free_skb_indexes_fifo_table_physical_address;
    g_free_skb_indexes_fifo_table_physical_address_last_idx += (g_cpu_tx_abs_packet_limit - 1) * sizeof(uint16_t);
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_FREE_SKB_INDEXES_FIFO_TABLE_LAST_ENTRY_ADDRESS );
    MWRITE_32( ddr_address_ptr,  g_free_skb_indexes_fifo_table_physical_address_last_idx);
#endif

    /* update all local tail pointers to 0 */
    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MWRITE_8( free_indexes_local_fifo_tail_ptr, 0 );
    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MWRITE_8( free_indexes_local_fifo_tail_ptr, 0 );
    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MWRITE_8( free_indexes_local_fifo_tail_ptr, 0 );
    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MWRITE_8( free_indexes_local_fifo_tail_ptr, 0 );

    free_indexes_fifo_tail_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + FREE_SKB_INDEXES_DDR_FIFO_TAIL_ADDRESS );
    MWRITE_32( free_indexes_fifo_tail_ptr, g_free_skb_indexes_fifo_table_physical_address );

    /* Initialize pointers to EMAC enqueued indexes FIFO */
    skb_enqueued_indexes_fifo_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_ADDRESS );
    skb_enqueued_indexes_fifo_counters_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_ADDRESS );

    skb_enqueued_indexes_fifo = EMAC_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS;

    for ( i = BL_LILAC_RDD_EMAC_ID_0; i <= BL_LILAC_RDD_EMAC_ID_4; i++ )
    {
        MWRITE_16( skb_enqueued_indexes_fifo_ptr, skb_enqueued_indexes_fifo );
        MWRITE_8(skb_enqueued_indexes_fifo_counters_ptr, 16);

        skb_enqueued_indexes_fifo_ptr++;
        skb_enqueued_indexes_fifo_counters_ptr++;

        skb_enqueued_indexes_fifo += 32;
    }

    skb_enqueued_indexes_fifo_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_ADDRESS );

    skb_enqueued_indexes_fifo = EMAC_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS;

    for ( i = BL_LILAC_RDD_EMAC_ID_0; i <= BL_LILAC_RDD_EMAC_ID_4; i++ )
    {
        MWRITE_16( skb_enqueued_indexes_fifo_ptr, skb_enqueued_indexes_fifo );

        skb_enqueued_indexes_fifo_ptr++;
        skb_enqueued_indexes_fifo += 32;
    }

    skb_enqueued_indexes_fifo_size = 32;

    /* Initialize pointers to WAN enqueued indexes FIFO */    
    skb_enqueued_indexes_fifo_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    skb_enqueued_indexes_fifo = GPON_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS;

    for ( i = 0; i < ( RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE ); i++ )
    {
        MWRITE_16( skb_enqueued_indexes_fifo_ptr, skb_enqueued_indexes_fifo );

        skb_enqueued_indexes_fifo_ptr++;
        skb_enqueued_indexes_fifo += skb_enqueued_indexes_fifo_size;
    }


    skb_enqueued_indexes_fifo_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    skb_enqueued_indexes_fifo = GPON_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS;

    for ( i = 0; i < ( RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE ); i++ )
    {
        MWRITE_16( skb_enqueued_indexes_fifo_ptr, skb_enqueued_indexes_fifo );

        skb_enqueued_indexes_fifo_ptr++;
        skb_enqueued_indexes_fifo += skb_enqueued_indexes_fifo_size;
    }


    /* Initialize to (-1) 6-bit value BBH and FW absolute TX counters */
    absolute_tx_counters_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS );

    for ( i = BL_LILAC_RDD_EMAC_ID_0; i <= BL_LILAC_RDD_EMAC_ID_4; i++ )
    {
        MWRITE_8( absolute_tx_counters_ptr, 0x3F );
        absolute_tx_counters_ptr += 8;
    }

    absolute_tx_counters_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS );

    for ( i = BL_LILAC_RDD_EMAC_ID_0; i <= BL_LILAC_RDD_EMAC_ID_4; i++ )
    {
        MWRITE_8( absolute_tx_counters_ptr, 0x3F );
        absolute_tx_counters_ptr ++;
    }

    absolute_tx_counters_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS );

    for ( i = 0; i < ( RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE ); i++ )
    {
        MWRITE_8( absolute_tx_counters_ptr, 0x3F );
        absolute_tx_counters_ptr ++;
    }

    absolute_tx_counters_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS );

    for ( i = 0; i < ( RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE ); i++ )
    {
        MWRITE_8( absolute_tx_counters_ptr, 0x3F );
        absolute_tx_counters_ptr ++;
    }

    absolute_tx_counters_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETHWAN_ABSOLUTE_TX_BBH_COUNTER_ADDRESS );
    MWRITE_8( absolute_tx_counters_ptr, 0x3F );

    absolute_tx_counters_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETHWAN_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS );
    MWRITE_8( absolute_tx_counters_ptr, 0x3F );

    return ( BL_LILAC_RDD_OK );
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_classification_table_initialize ( void )
{
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS *rule_cfg_entry_ptr;
    uint8_t *rule_cfg_descriptor_ptr;
    RUNNER_REGS_CFG_LKUP0_CFG hash_lkup_0_cfg_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK0_H hash_lkup_0_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK0_L hash_lkup_0_global_mask_low_register;
    RUNNER_REGS_CFG_LKUP3_CFG hash_lkup_3_cfg_register;
    RUNNER_REGS_CFG_LKUP3_CAM_CFG hash_lkup_3_cam_cfg_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK3_H hash_lkup_3_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK3_L hash_lkup_3_global_mask_low_register;
    uint32_t rule_cfg_id;
#ifdef UNDEF
    RUNNER_REGS_CFG_LKUP0_CAM_CFG hash_lkup_0_cam_cfg_register;
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS *ds_ingress_classification_counters_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS *us_ingress_classification_counters_table_ptr;
#endif
#endif

    for (rule_cfg_id = 0; rule_cfg_id < 16; rule_cfg_id++)
    {
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].valid = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].priority = -1;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].rule_type = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].next_group_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id = 16;

        ds_rule_cfg_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR();

        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ rule_cfg_id ] );

        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( 16, rule_cfg_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( 16, rule_cfg_entry_ptr );

        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].valid = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].priority = -1;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].rule_type = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].next_group_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id = 16;

        us_rule_cfg_table_ptr = RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR();

        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ rule_cfg_id ] );

        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( 16, rule_cfg_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( 16, rule_cfg_entry_ptr );
    }

    g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].first_rule_cfg_id = 16;
    g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].first_gen_filter_rule_cfg_id = 16;
    g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].first_rule_cfg_id = 16;
    g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].first_gen_filter_rule_cfg_id = 16;

    rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_DESCRIPTOR_ADDRESS );

    MWRITE_8( rule_cfg_descriptor_ptr, 16 );

    rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_DESCRIPTOR_ADDRESS );

    MWRITE_8( rule_cfg_descriptor_ptr, 16 );

    rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_IP_FLOW_RULE_CFG_DESCRIPTOR_ADDRESS );

    MWRITE_8( rule_cfg_descriptor_ptr, 16 );

    rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_IP_FLOW_RULE_CFG_DESCRIPTOR_ADDRESS );

    MWRITE_8( rule_cfg_descriptor_ptr, 16 );

    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].hash_table_size = RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].hash_table_search_depth = RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_DISABLE;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].context_size = BL_LILAC_RDD_CONTEXT_8_BIT;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].is_extension_cam = LILAC_RDD_TRUE;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].cam_context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].cam_table_size = RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE - 1;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].hash_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                                                                                                             DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE ].cam_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                                                                                                            DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

    hash_lkup_3_cfg_register.base_address = ( DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS >> 3 );
    hash_lkup_3_cfg_register.table_size = BL_LILAC_RDD_MAC_TABLE_SIZE_256;
    hash_lkup_3_cfg_register.max_hop = RDD_INGRESS_CLASSIFICATION_SEARCH_HOP;
    hash_lkup_3_cfg_register.hash_type = LILAC_RDD_MAC_HASH_TYPE_CRC16;
    RUNNER_REGS_0_CFG_LKUP3_CFG_WRITE ( hash_lkup_3_cfg_register );

    hash_lkup_3_cam_cfg_register.cam_en = LILAC_RDD_TRUE;
    hash_lkup_3_cam_cfg_register.base_address = ( DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS >> 3 );
    RUNNER_REGS_0_CFG_LKUP3_CAM_CFG_WRITE ( hash_lkup_3_cam_cfg_register );

    hash_lkup_3_global_mask_high_register.base_address = INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_HIGH;
    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK3_H_WRITE ( hash_lkup_3_global_mask_high_register );

    hash_lkup_3_global_mask_low_register.base_address = INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_LOW;
    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK3_L_WRITE ( hash_lkup_3_global_mask_low_register );

    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].hash_table_size = RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE / 2;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].hash_table_search_depth = RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_DISABLE;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].context_size = BL_LILAC_RDD_CONTEXT_8_BIT;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].is_extension_cam = LILAC_RDD_FALSE;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].cam_context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].cam_table_size = 0;
    g_hash_table_cfg[ BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_LONG_TABLE ].hash_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS );

    hash_lkup_0_cfg_register.base_address = ( DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS >> 3 );
    hash_lkup_0_cfg_register.table_size = BL_LILAC_RDD_MAC_TABLE_SIZE_256;
    hash_lkup_0_cfg_register.max_hop = RDD_INGRESS_CLASSIFICATION_SEARCH_HOP + 1;	/* long table is 120 bit hash value, requires additional hop */
    hash_lkup_0_cfg_register.hash_type = LILAC_RDD_MAC_HASH_TYPE_CRC16;
    RUNNER_REGS_0_CFG_LKUP0_CFG_WRITE ( hash_lkup_0_cfg_register );

    hash_lkup_0_global_mask_high_register.base_address = INGRESS_CLASSIFICATION_LONG_ENTRY_KEY_MASK_HIGH;
    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK0_H_WRITE ( hash_lkup_0_global_mask_high_register );

    hash_lkup_0_global_mask_low_register.base_address = INGRESS_CLASSIFICATION_LONG_ENTRY_KEY_MASK_LOW;
    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK0_L_WRITE ( hash_lkup_0_global_mask_low_register );

    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].hash_table_size = RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].hash_table_search_depth = RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_DISABLE;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].context_size = BL_LILAC_RDD_CONTEXT_8_BIT;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].is_extension_cam = LILAC_RDD_TRUE;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].cam_context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].cam_table_size = RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE - 1;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].hash_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                                                                                                             US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE ].cam_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                                                                                                            US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

    hash_lkup_3_cfg_register.base_address = ( US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS >> 3 );
    hash_lkup_3_cfg_register.table_size = BL_LILAC_RDD_MAC_TABLE_SIZE_256;
    hash_lkup_3_cfg_register.max_hop = RDD_INGRESS_CLASSIFICATION_SEARCH_HOP;
    hash_lkup_3_cfg_register.hash_type = LILAC_RDD_MAC_HASH_TYPE_CRC16;
    RUNNER_REGS_1_CFG_LKUP3_CFG_WRITE ( hash_lkup_3_cfg_register );

    hash_lkup_3_cam_cfg_register.cam_en = LILAC_RDD_TRUE;
    hash_lkup_3_cam_cfg_register.base_address = ( US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS >> 3 );
    RUNNER_REGS_1_CFG_LKUP3_CAM_CFG_WRITE ( hash_lkup_3_cam_cfg_register );

    hash_lkup_3_global_mask_high_register.base_address = INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_HIGH;
    RUNNER_REGS_1_CFG_LKUP_GLBL_MASK3_H_WRITE ( hash_lkup_3_global_mask_high_register );

    hash_lkup_3_global_mask_low_register.base_address = INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_LOW;
    RUNNER_REGS_1_CFG_LKUP_GLBL_MASK3_L_WRITE ( hash_lkup_3_global_mask_low_register );


    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].hash_table_size = RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE / 2;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].hash_table_search_depth = RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_DISABLE;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].context_size = BL_LILAC_RDD_CONTEXT_8_BIT;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].is_extension_cam = LILAC_RDD_FALSE;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].cam_context_table_ptr = NULL;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].cam_table_size = 0;
    g_hash_table_cfg[ BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_LONG_TABLE ].hash_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                                                                                                            US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

    hash_lkup_0_cfg_register.base_address = ( US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS >> 3 );
    hash_lkup_0_cfg_register.table_size = BL_LILAC_RDD_MAC_TABLE_SIZE_256;
    hash_lkup_0_cfg_register.max_hop = RDD_INGRESS_CLASSIFICATION_SEARCH_HOP + 1;	/* long table is 120 bit hash value, requires additional hop */
    hash_lkup_0_cfg_register.hash_type = LILAC_RDD_MAC_HASH_TYPE_CRC16;
    RUNNER_REGS_1_CFG_LKUP0_CFG_WRITE ( hash_lkup_0_cfg_register );
#ifdef UNDEF
    hash_lkup_0_cam_cfg_register.cam_en = LILAC_RDD_TRUE;
    hash_lkup_0_cam_cfg_register.base_address = ( DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS >> 3 );
    RUNNER_REGS_1_CFG_LKUP0_CAM_CFG_WRITE ( hash_lkup_0_cam_cfg_register );
#endif
    hash_lkup_0_global_mask_high_register.base_address = INGRESS_CLASSIFICATION_LONG_ENTRY_KEY_MASK_HIGH;
    RUNNER_REGS_1_CFG_LKUP_GLBL_MASK0_H_WRITE ( hash_lkup_0_global_mask_high_register );

    hash_lkup_0_global_mask_low_register.base_address = INGRESS_CLASSIFICATION_LONG_ENTRY_KEY_MASK_LOW;
    RUNNER_REGS_1_CFG_LKUP_GLBL_MASK0_L_WRITE ( hash_lkup_0_global_mask_low_register );

#ifdef UNDEF
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    /* FIXME!! do we need to initialize SRAM memory? doing memset cause system abort.. */
    ds_ingress_classification_counters_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR();
    us_ingress_classification_counters_table_ptr = RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR();

    MEMSET(ds_ingress_classification_counters_table_ptr, 0, sizeof(RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS));
    MEMSET(us_ingress_classification_counters_table_ptr, 0, sizeof(RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS));
#endif
#endif

    return ( BL_LILAC_RDD_OK );
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_eth_tx_initialize ( void )
{
    RDD_ETH_TX_MAC_TABLE_DTS *eth_tx_mac_table;
    RDD_ETH_TX_MAC_DESCRIPTOR_DTS *eth_tx_mac_descriptor;
    RDD_ETH_TX_QUEUES_TABLE_DTS *eth_tx_queues_table;
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS *eth_tx_queue_descriptor;
    RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS *eth_tx_queues_pointers_table;
    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS *eth_tx_queue_pointers_entry;
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *free_packet_descriptors_pool_descriptor;
    RDD_ETH_TX_LOCAL_REGISTERS_DTS *eth_tx_local_registers;
    RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_DTS *eth_tx_local_registers_entry;
    uint16_t eth_tx_queue_address;
    uint16_t mac_descriptor_address;
    uint32_t emac;
    uint32_t tx_queue;

    eth_tx_mac_table = RDD_ETH_TX_MAC_TABLE_PTR();

    eth_tx_queues_table = RDD_ETH_TX_QUEUES_TABLE_PTR();

    eth_tx_queues_pointers_table = RDD_ETH_TX_QUEUES_POINTERS_TABLE_PTR();

    eth_tx_local_registers = RDD_ETH_TX_LOCAL_REGISTERS_PTR();

    for (emac = BL_LILAC_RDD_EMAC_ID_0; emac < BL_LILAC_RDD_EMAC_ID_COUNT; emac++)
    {
        eth_tx_mac_descriptor = &(eth_tx_mac_table->entry[emac]);

        RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE(ETH_TX_THREAD_NUMBER, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_EMAC_MASK_WRITE((1 << emac), eth_tx_mac_descriptor);
#if defined(WL4908)
        if (emac == BL_LILAC_RDD_EMAC_ID_4)
            RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE(RDD_GPIO_IO_ADDRESS + /* SF2 port */ 6, eth_tx_mac_descriptor);
        else
            RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE((RDD_GPIO_IO_ADDRESS + (emac - BL_LILAC_RDD_EMAC_ID_0)), eth_tx_mac_descriptor);

        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_0_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            BL_LILAC_RDD_EMAC_ID_0 * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_1_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            BL_LILAC_RDD_EMAC_ID_1 * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_2_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            BL_LILAC_RDD_EMAC_ID_2 * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_BBH_DESTINATION_0_WRITE(BBH_PERIPHERAL_ETH0_TX, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_BBH_DESTINATION_1_WRITE(BBH_PERIPHERAL_ETH1_TX, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_BBH_DESTINATION_2_WRITE(BBH_PERIPHERAL_ETH2_TX, eth_tx_mac_descriptor);

        if (emac == BL_LILAC_RDD_EMAC_ID_4)
            RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_PORT_WRITE(/* SF2 port */  6, eth_tx_mac_descriptor);
        else
            RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_PORT_WRITE((emac - BL_LILAC_RDD_EMAC_ID_0), eth_tx_mac_descriptor);
#else
        RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE((RDD_GPIO_IO_ADDRESS + (emac - BL_LILAC_RDD_EMAC_ID_0)), eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_1_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            BL_LILAC_RDD_EMAC_ID_1 * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_BBH_DESTINATION_1_WRITE(BBH_PERIPHERAL_ETH1_TX, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_PORT_WRITE((emac - BL_LILAC_RDD_EMAC_ID_0), eth_tx_mac_descriptor);
#endif
        RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_WRITE(RDD_RATE_LIMITER_IDLE, eth_tx_mac_descriptor);

        for (tx_queue = 0; tx_queue < RDD_EMAC_NUMBER_OF_QUEUES; tx_queue++)
        {
            eth_tx_queue_address = ETH_TX_QUEUES_TABLE_ADDRESS +
                ((emac - BL_LILAC_RDD_EMAC_ID_0) * RDD_EMAC_NUMBER_OF_QUEUES + tx_queue) * sizeof(RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS);

            mac_descriptor_address = ETH_TX_MAC_TABLE_ADDRESS + emac * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS);

            eth_tx_queue_pointers_entry =
                &(eth_tx_queues_pointers_table->entry[emac * RDD_EMAC_NUMBER_OF_QUEUES + tx_queue]);

            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_ETH_MAC_POINTER_WRITE(mac_descriptor_address, eth_tx_queue_pointers_entry);
            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_WRITE(eth_tx_queue_address, eth_tx_queue_pointers_entry);

            eth_tx_queue_descriptor = &(eth_tx_queues_table->entry[(emac - BL_LILAC_RDD_EMAC_ID_0) * RDD_EMAC_NUMBER_OF_QUEUES + tx_queue]);

            RDD_ETH_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE(1 << tx_queue , eth_tx_queue_descriptor);
            RDD_ETH_TX_QUEUE_DESCRIPTOR_INDEX_WRITE((emac * RDD_EMAC_NUMBER_OF_QUEUES) + tx_queue, eth_tx_queue_descriptor);
        }
        eth_tx_local_registers_entry = &(eth_tx_local_registers->entry[emac]);

        RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_EMAC_DESCRIPTOR_PTR_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            emac * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS), eth_tx_local_registers_entry);

        RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_ETH_TX_QUEUES_POINTERS_TABLE_PTR_WRITE(ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS +
            emac * RDD_EMAC_NUMBER_OF_QUEUES * sizeof(RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS), eth_tx_local_registers_entry);
    }

    free_packet_descriptors_pool_descriptor =
        (RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) +
        FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS);

    /*Initial values, will be updated by rdd_tm_ds_free_packet_descriptors_pool_size_update.*/
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_GUARANTEED_THRESHOLD_WRITE ( DS_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD, free_packet_descriptors_pool_descriptor );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_GUARANTEED_FREE_COUNT_WRITE (DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE, free_packet_descriptors_pool_descriptor );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_NON_GUARANTEED_FREE_COUNT_WRITE ( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE, free_packet_descriptors_pool_descriptor );

    return ( BL_LILAC_RDD_OK );
}



static BL_LILAC_RDD_ERROR_DTE f_rdd_wan_tx_initialize ( void )
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS          *wan_channels_0_7_table_ptr;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS      *wan_channel_0_7_descriptor_ptr;
    RDD_WAN_CHANNELS_8_39_TABLE_DTS         *wan_channels_8_39_table_ptr;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS     *wan_channel_8_39_descriptor_ptr;
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS   *dummy_rate_controller_descriptor_ptr;
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS         *dummy_wan_tx_queue_descriptor_ptr;
    RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS  *exponent_table_ptr;
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_DTS  *exponent_entry_ptr;
    uint32_t                                wan_channel_id;
    uint32_t                                rate_controller_id;
    uint32_t                                tx_queue_id;

    /* initialize WAN TX pointers table */
    wan_tx_pointers_table_ptr = ( RDD_WAN_TX_POINTERS_TABLE_DTS * )malloc( sizeof( RDD_WAN_TX_POINTERS_TABLE_DTS ) );

    if ( wan_tx_pointers_table_ptr == NULL)
    {
        return ( BL_LILAC_RDD_ERROR_MALLOC_FAILED );
    }

    memset ( wan_tx_pointers_table_ptr, 0, sizeof ( RDD_WAN_TX_POINTERS_TABLE_DTS ) );

    /* reset the dummy segmentation descriptors threshold to zero in order to drop packets */
    dummy_wan_tx_queue_descriptor_ptr = ( RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DUMMY_WAN_TX_QUEUE_DESCRIPTOR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE ( 0, dummy_wan_tx_queue_descriptor_ptr );
    RDD_WAN_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE ( 0, dummy_wan_tx_queue_descriptor_ptr );

    /* all the queues of the dummy rate controller will point to the dummy queue */
    dummy_rate_controller_descriptor_ptr = ( RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DUMMY_RATE_CONTROLLER_DESCRIPTOR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    for ( tx_queue_id = 0; tx_queue_id < RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER; tx_queue_id++ )
    {
        RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_WRITE ( DUMMY_WAN_TX_QUEUE_DESCRIPTOR_ADDRESS, dummy_rate_controller_descriptor_ptr, tx_queue_id );
    }

    /* connect all the tconts to the dummy rate rate controller */
    wan_channels_0_7_table_ptr = ( RDD_WAN_CHANNELS_0_7_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_0_7_TABLE_ADDRESS );

    for ( wan_channel_id = RDD_WAN_CHANNEL_0; wan_channel_id <= RDD_WAN_CHANNEL_7; wan_channel_id++ )
    {
        wan_channel_0_7_descriptor_ptr = &( wan_channels_0_7_table_ptr->entry[ wan_channel_id ] );

        for ( rate_controller_id = BL_LILAC_RDD_RATE_CONTROLLER_0; rate_controller_id <= BL_LILAC_RDD_RATE_CONTROLLER_31; rate_controller_id++ )
        {
            RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE ( DUMMY_RATE_CONTROLLER_DESCRIPTOR_ADDRESS, wan_channel_0_7_descriptor_ptr, rate_controller_id );
        }
    }

    wan_channels_8_39_table_ptr = ( RDD_WAN_CHANNELS_8_39_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_8_39_TABLE_ADDRESS );

    for ( wan_channel_id = RDD_WAN_CHANNEL_8; wan_channel_id <= RDD_WAN_CHANNEL_39; wan_channel_id++ )
    {
        wan_channel_8_39_descriptor_ptr = &( wan_channels_8_39_table_ptr->entry[ wan_channel_id - RDD_WAN_CHANNEL_8 ] );

        for ( rate_controller_id = BL_LILAC_RDD_RATE_CONTROLLER_0; rate_controller_id <= BL_LILAC_RDD_RATE_CONTROLLER_3; rate_controller_id++ )
        {
            RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE ( DUMMY_RATE_CONTROLLER_DESCRIPTOR_ADDRESS, wan_channel_8_39_descriptor_ptr, rate_controller_id );
        }
    }

    g_rate_controllers_pool_idx = 0;

    /* initialize exponents table */
    exponent_table_ptr = ( RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 0 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT0, exponent_entry_ptr );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 1 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT1, exponent_entry_ptr );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 2 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT2, exponent_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_inter_task_queues_initialize ( void )
{
    uint16_t  *wan_enqueue_ingress_queue_ptr;
    uint16_t  *ethwan2_rx_ingress_queue_ptr;
#if defined(CONFIG_DHD_RUNNER)
    uint16_t  *lan_enqueue_ingress_queue_ptr;

    lan_enqueue_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR_ADDRESS );
    MWRITE_16( lan_enqueue_ingress_queue_ptr, LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS );
#endif

    wan_enqueue_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_ENQUEUE_INGRESS_QUEUE_PTR_ADDRESS );
    MWRITE_16( wan_enqueue_ingress_queue_ptr, WAN_ENQUEUE_INGRESS_QUEUE_ADDRESS );

    ethwan2_rx_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETHWAN2_RX_INGRESS_QUEUE_PTR_ADDRESS );
    MWRITE_16( ethwan2_rx_ingress_queue_ptr, ETHWAN2_RX_INGRESS_QUEUE_ADDRESS );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_pm_counters_initialize ( void )
{
    RUNNER_REGS_CFG_CNTR_CFG  runner_counter_cfg_register;

    runner_counter_cfg_register.base_address = ( PM_COUNTERS_ADDRESS >> 3 );

    RUNNER_REGS_0_CFG_CNTR_CFG_WRITE ( runner_counter_cfg_register );
    RUNNER_REGS_1_CFG_CNTR_CFG_WRITE ( runner_counter_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_parallel_processing_initialize ( void )
{
    RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_DTS  *ds_context_index_cache_cam;
    RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_DTS  *us_context_index_cache_cam;
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_DTS                *ds_available_slave_vector_ptr;
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_DTS                *us_available_slave_vector_ptr;
    RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR_DTS               *ds_slave_ih_buffer_ptr;
    RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR_DTS               *us_slave_ih_buffer_ptr;
    uint16_t                                                *ds_context_index_cache_cam_entry;
    uint16_t                                                *us_context_index_cache_cam_entry;
    uint8_t                                                 *context_cache_state_ptr;
    uint8_t                                                 i;

    /* downstream */
    ds_available_slave_vector_ptr = ( RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_SLAVE_VECTOR_ADDRESS );

    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE0_WRITE ( LILAC_RDD_TRUE, ds_available_slave_vector_ptr );
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE1_WRITE ( LILAC_RDD_TRUE, ds_available_slave_vector_ptr );
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE2_WRITE ( LILAC_RDD_TRUE, ds_available_slave_vector_ptr );
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE3_WRITE ( LILAC_RDD_TRUE, ds_available_slave_vector_ptr );

    ds_slave_ih_buffer_ptr = ( RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR_ADDRESS );

    MWRITE_16( ds_slave_ih_buffer_ptr, DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS );

    ds_context_index_cache_cam = RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_PTR();

    for ( i = 0; i < RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_SIZE; i++ )
    {
        ds_context_index_cache_cam_entry = ( uint16_t * ) &ds_context_index_cache_cam->entry[ i ];

        MWRITE_16( ds_context_index_cache_cam_entry, 0xFFFF );
    }

    /* set context cache in enable mode */
    context_cache_state_ptr = ( uint8_t * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_CONTEXT_CACHE_MODE_ADDRESS );

    MWRITE_8( context_cache_state_ptr, 0x0 );

    /* upstream */
    us_available_slave_vector_ptr = ( RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_SLAVE_VECTOR_ADDRESS );

    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE0_WRITE ( LILAC_RDD_TRUE, us_available_slave_vector_ptr );
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE1_WRITE ( LILAC_RDD_TRUE, us_available_slave_vector_ptr );
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE2_WRITE ( LILAC_RDD_TRUE, us_available_slave_vector_ptr );
    RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE3_WRITE ( LILAC_RDD_TRUE, us_available_slave_vector_ptr );

    us_slave_ih_buffer_ptr = ( RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR_ADDRESS );

    MWRITE_16( us_slave_ih_buffer_ptr, US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS );

    us_context_index_cache_cam = RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_PTR();

    for ( i = 0; i < RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_SIZE; i++ )
    {
        us_context_index_cache_cam_entry = ( uint16_t * ) &us_context_index_cache_cam->entry[ i ];

        MWRITE_16( us_context_index_cache_cam_entry, 0xFFFF );
    }

    /* set context cache in enable mode */
    context_cache_state_ptr = ( uint8_t * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_CONTEXT_CACHE_MODE_ADDRESS );

    MWRITE_8( context_cache_state_ptr, 0x0 );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_broadcom_switch_ports_mapping_table_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                        uint8_t                       xi_broadcom_switch_port )
{
    RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS  *broadcom_switch_mapping_table_ptr;

    broadcom_switch_mapping_table_ptr = ( RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_ADDRESS );

    if ( ( xi_bridge_port < BL_LILAC_RDD_LAN0_BRIDGE_PORT ) || ( xi_bridge_port > BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE ( xi_bridge_port, broadcom_switch_mapping_table_ptr + xi_broadcom_switch_port );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_lookup_ports_mapping_table_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                               uint8_t                       xi_lookup_port )
{
    RDD_DS_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_DTS *ds_lookup_port_mapping_table_ptr;
    RDD_US_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_DTS *us_lookup_port_mapping_table_ptr;

    ds_lookup_port_mapping_table_ptr = RDD_DS_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_PTR();
    us_lookup_port_mapping_table_ptr = RDD_US_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_PTR();


    if ( xi_bridge_port > BL_LILAC_RDD_PCI_BRIDGE_PORT )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }


    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE ( xi_lookup_port, ds_lookup_port_mapping_table_ptr + xi_bridge_port );
    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE ( xi_lookup_port, us_lookup_port_mapping_table_ptr + xi_bridge_port );

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_lookup_ports_mapping_table_init ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             uint8_t                       xi_lookup_port )
{
    BL_LILAC_RDD_ERROR_DTE err = BL_LILAC_RDD_OK;

    err = rdd_lookup_ports_mapping_table_config(xi_bridge_port,xi_lookup_port);

    if ( !err )
    {
        g_lookup_port_init_mapping_table[xi_bridge_port] =  xi_lookup_port;
    }

    return ( err );
}

BL_LILAC_RDD_ERROR_DTE rdd_lookup_ports_mapping_table_get ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                            uint8_t                      *xo_lookup_port )
{
    RDD_DS_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_DTS *ds_lookup_port_mapping_table_ptr;
    uint8_t current_lookup_port;

    ds_lookup_port_mapping_table_ptr = RDD_DS_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_PTR();


    if ( xi_bridge_port > BL_LILAC_RDD_PCI_BRIDGE_PORT )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_READ  ( current_lookup_port, ds_lookup_port_mapping_table_ptr + xi_bridge_port );
    *xo_lookup_port =  current_lookup_port;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_lookup_ports_mapping_table_restore ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port )
{
    RDD_DS_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_DTS *ds_lookup_port_mapping_table_ptr;
    RDD_US_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_DTS *us_lookup_port_mapping_table_ptr;
    uint8_t lookup_port;

    ds_lookup_port_mapping_table_ptr = RDD_DS_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_PTR();
    us_lookup_port_mapping_table_ptr = RDD_US_BRIDGE_PORT_TO_LOOKUP_PORT_MAPPING_TABLE_PTR();


    if ( xi_bridge_port > BL_LILAC_RDD_PCI_BRIDGE_PORT )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    lookup_port = g_lookup_port_init_mapping_table[xi_bridge_port];
    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE ( lookup_port, ds_lookup_port_mapping_table_ptr + xi_bridge_port );
    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE ( lookup_port, us_lookup_port_mapping_table_ptr + xi_bridge_port );


    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ethwan2_switch_port_config ( uint8_t xi_switch_port )
{
    uint8_t *ethwan2_switch_port_config_ptr;

    ethwan2_switch_port_config_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETHWAN2_SWITCH_PORT_ADDRESS );
    MWRITE_8( ethwan2_switch_port_config_ptr, xi_switch_port );
    return ( BL_LILAC_RDD_OK );
}

