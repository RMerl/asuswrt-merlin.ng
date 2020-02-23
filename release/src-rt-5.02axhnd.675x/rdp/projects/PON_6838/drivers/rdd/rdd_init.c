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
#include "rdd_legacy_conv.h"
#include "rdd_service_queues.h"
#ifdef CONFIG_DHD_RUNNER
#include "dhd_defs.h"
#endif

/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/
#if defined(FIRMWARE_INIT)
uint8_t  *soc_base_address;
uint32_t  DsConnectionTableBase;
uint32_t  UsConnectionTableBase;
uint32_t  ContextTableBase;
uint32_t  IPTVTableBase;
uint32_t  IPTVContextTableBase;
uint32_t  IPTVSsmContextTableBase;
uint32_t  FirewallRulesMapTable;
uint32_t  FirewallRulesTable;
uint32_t  GenericClassifierTable;
uint32_t  DdrAddressForSyncDmaBase;
uint32_t  CpuRxRingBase;
uint32_t  DhdTxFlowRingMgmtTable;
#endif


#ifdef _CFE_

#define RDP_CPU_RING_MAX_QUEUES             1
#define RDP_WLAN_MAX_QUEUES                 0

#elif defined(__KERNEL__)
#include "rdpa_cpu.h"
#include "bdmf_system.h"
#include "bdmf_shell.h"
#include "bdmf_dev.h"

#define RDP_CPU_RING_MAX_QUEUES             RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES                 RDPA_WLAN_MAX_QUEUES

#ifdef FLUSH_RANGE
#undef FLUSH_RANGE
#endif
#define FLUSH_RANGE(s,l)                    bdmf_dcache_flush(s,l);
#ifdef INV_RANGE
#undef INV_RANGE
#endif
#define INV_RANGE(s,l)                      bdmf_dcache_inv(s,l);

extern const bdmf_attr_enum_table_t rdpa_cpu_reason_enum_table;
#endif


extern uint8_t                                        *g_runner_ddr_base_addr;
extern uint32_t                                       g_runner_ddr_base_addr_phys;
extern uint8_t                                        *g_runner_extra_ddr_base_addr;
extern uint32_t                                       g_runner_extra_ddr_base_addr_phys;
extern uint32_t                                       g_ddr_headroom_size;
extern uint32_t                                       g_runner_tables_ptr;
extern uint8_t                                        g_broadcom_switch_mode;
extern BL_LILAC_RDD_BRIDGE_PORT_DTE                   g_broadcom_switch_physical_port;
extern uint32_t                                       g_bridge_flow_cache_mode;
extern BL_LILAC_RDD_WAN_PHYSICAL_PORT_DTE             g_wan_physical_port;
extern BL_LILAC_RDD_VERSION_DTS                       gs_rdd_version;
extern RDD_64_BIT_TABLE_CFG                           g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
extern uint32_t                                       *g_cpu_tx_skb_pointers_reference_array;
extern uint32_t                                       *g_cpu_tx_data_pointers_reference_array;
extern uint32_t                                       g_cpu_tx_skb_free_indexes_counter;
extern uint16_t                                       *g_free_skb_indexes_fifo_table_physical_address;
extern uint16_t                                       *g_free_skb_indexes_fifo_table_physical_address_last_idx;
extern uint16_t                                       *g_free_skb_indexes_fifo_table;
extern uint16_t                                       *g_rdd_free_skb_indexes_fifo_table;
extern RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTE  g_ingress_classification_rule_cfg_table[ 2 ];
extern uint32_t                                       g_rate_controllers_pool_idx;
extern uint32_t                                       g_chip_revision;
extern RDD_WAN_TX_POINTERS_TABLE_DTS                  *wan_tx_pointers_table_ptr;
#ifdef CONFIG_DHD_RUNNER
extern uint8_t                                        *g_dhd_tx_cpu_usage_reference_array;
#endif
uint32_t                                              g_us_ddr_queue_enable;
rdpa_bpm_buffer_size_t                                g_bpm_buffer_size;
uint32_t                                              g_wan_mapping = 0;

extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_DTE        f_rdd_lock;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_DTE      f_rdd_unlock;
extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_IRQ_DTE    f_rdd_lock_irq;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_IRQ_DTE  f_rdd_unlock_irq;

static BL_LILAC_RDD_ERROR_DTE f_rdd_enable_context_memory ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_bpm_initialize ( uint32_t, uint32_t, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_ddr_initialize ( uint32_t, uint32_t, BL_LILAC_RDD_CHIP_REVISION_DTE );
static BL_LILAC_RDD_ERROR_DTE f_rdd_psram_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_scheduler_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_free_packet_descriptors_pool_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_global_registers_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_local_registers_initialize ( BL_LILAC_RDD_BRIDGE_PORT_DTE        lan_direct_to_cpu_port );
static BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_classification_table_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_port_to_bbh_destination_table_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_eth_tx_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_wan_tx_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_inter_task_queues_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_broadcom_switch_mode_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_pm_counters_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_1588_initialize ( void );
static BL_LILAC_RDD_ERROR_DTE f_rdd_transmit_from_abs_address_initialize ( void );
#ifndef G9991
static BL_LILAC_RDD_ERROR_DTE f_rdd_parallel_processing_initialize ( void );
#if !defined(RDD_BASIC)
static void rdd_actions_proj_init(void);
static void rdd_action_vector_set(uint16_t *action_ptrs_ptr, uint16_t *action_ptrs, uint8_t action_total_num);
#endif
#else
static BL_LILAC_RDD_ERROR_DTE f_rdd_G9991_initialize ( void );
#endif
static void f_rdd_wan_tx_queue_ddr_offload_initialize( bdmf_boolean  xi_enable );

extern BL_LILAC_RDD_ERROR_DTE rdd_firewall_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE rdd_cpu_tx_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE rdd_cpu_rx_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_mac_table_initialize ( uint32_t, uint32_t );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_iptv_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_filters_cam_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_layer4_filters_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_vlan_matrix_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_connection_table_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_multicast_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_vid_cam_initialize ( void );
extern BL_LILAC_RDD_ERROR_DTE f_rdd_tm_service_queues_initialize ( void );
extern void f_rdd_full_flow_cache_config ( bdmf_boolean );
extern BL_LILAC_RDD_ERROR_DTE f_epon_tx_post_scheduling_ddr_queue_initialize ( uint16_t  xi_queue_size );
#ifdef G9991
extern BL_LILAC_RDD_ERROR_DTE f_ds_tx_offload_pd_ddr_queue_initialize ( void );
#endif

#ifdef CONFIG_DHD_RUNNER
extern void rdd_dhd_mode_enable_init(void);
#endif

#ifdef USE_BDMF_SHELL
extern BL_LILAC_RDD_ERROR_DTE f_rdd_make_shell_commands ( void );
#endif

static void f_rdd_dummy_lock ( bdmf_fastlock *xo_int_lock );
static void f_rdd_dummy_unlock ( bdmf_fastlock * xi_int_lock );
static void f_rdd_dummy_lock_irq ( bdmf_fastlock  *xo_int_lock, unsigned long *flags );
static void f_rdd_dummy_unlock_irq ( bdmf_fastlock * xi_int_lock, unsigned long flags );

#ifdef FIRMWARE_INIT

#define SEG_ALLOC_CHK(p, s) \
    do {\
        p = (typeof(p))malloc(s);\
        if (!p) \
        { \
            printf("Failed to allocate " # p " (%u bytes)\n", (unsigned)(s));\
            return -1;\
        }\
    } while (0)

#define SEG_CHK_FREE(p) \
    do {\
        if (p) \
            free((void *)p);\
    } while (0)

/* allocate memory segments for simulation */
static int rdd_sim_alloc_segments ( void )
{
    SEG_ALLOC_CHK( soc_base_address, sizeof( uint8_t ) * SIM_MEM_SIZE );
    MEMSET ( ( uint8_t *)  soc_base_address, 0, sizeof( uint8_t ) * SIM_MEM_SIZE );
    SEG_ALLOC_CHK( DsConnectionTableBase, sizeof( RDD_CONNECTION_TABLE_DTS ) );
    SEG_ALLOC_CHK( UsConnectionTableBase, sizeof( RDD_CONNECTION_TABLE_DTS ) );
    SEG_ALLOC_CHK( ContextTableBase, sizeof( RDD_CONTEXT_TABLE_DTS ) );
    SEG_ALLOC_CHK( IPTVTableBase, sizeof( RDD_IPTV_DDR_LOOKUP_TABLE_DTS ) );
    SEG_ALLOC_CHK( IPTVContextTableBase, sizeof( RDD_IPTV_DDR_CONTEXT_TABLE_DTS ) );
    SEG_ALLOC_CHK( IPTVSsmContextTableBase, sizeof ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS ) );
    SEG_ALLOC_CHK( FirewallRulesMapTable, 65536 * 8 * 2 );
    SEG_ALLOC_CHK( FirewallRulesTable, 256 * 16 );
    SEG_ALLOC_CHK( GenericClassifierTable, 32 * 32 );
    SEG_ALLOC_CHK( DdrAddressForSyncDmaBase, sizeof( uint32_t ) );
    SEG_ALLOC_CHK( CpuRxRingBase, sizeof( RDD_CPU_RX_DESCRIPTOR_DTS )* RDD_RING_DESCRIPTORS_TABLE_SIZE * 10 );
#ifndef G9991
    SEG_ALLOC_CHK( DhdTxFlowRingMgmtTable, sizeof(RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_DTS) * 
            RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_SIZE );
#endif
    return 0;
}

/* allocate memory segments for simulation */
static void rdd_sim_free_segments ( void )
{
    SEG_CHK_FREE( soc_base_address );
    SEG_CHK_FREE( DsConnectionTableBase );
    SEG_CHK_FREE( UsConnectionTableBase );
    SEG_CHK_FREE( ContextTableBase );
    SEG_CHK_FREE( IPTVTableBase );
    SEG_CHK_FREE( IPTVContextTableBase );
    SEG_CHK_FREE( IPTVSsmContextTableBase );
    SEG_CHK_FREE( FirewallRulesMapTable );
    SEG_CHK_FREE( FirewallRulesTable );
    SEG_CHK_FREE( GenericClassifierTable );
    SEG_CHK_FREE(DdrAddressForSyncDmaBase);
#ifndef G9991
    SEG_CHK_FREE( DhdTxFlowRingMgmtTable );
#endif
}


#endif /* #ifdef FIRMWARE_INIT */

/* memory allocation functions for absolute address FW/RDD joined memories */
#if !defined(FIRMWARE_INIT)

    /* making sure that allocation for FW will be non chached and aligned (FOR MIPS ONLY) */
static void *alignedAlloc(uint32_t size, uint32_t *phy_addr_p)
{
#if defined(__KERNEL__) && defined(CONFIG_ARM)
	dma_addr_t phy_addr;
	uint32_t size32 = (uint32_t)(size + sizeof(void*) + 3) & (uint32_t)(~3); // must be multiple of 4B
	/* memory allocation of NONCACHE_MALLOC for ARM is aligned to page size which is aligned to cache */
	uint32_t *mem = (uint32_t *)NONCACHED_MALLOC_ATOMIC(size32, &phy_addr);
	if (unlikely(mem == NULL))
		return NULL;
//	printk("\n\tsize %u, size32 %u, mem %p, &mem[size] %p, phy_addr 0x%08x\n\n", size, size32, mem, &mem[(size32-sizeof(void*))>>2], phy_addr);
	mem[(size32-sizeof(void*))>>2] = phy_addr;
	*phy_addr_p = (uint32_t)phy_addr;
	return (void *)mem;
#else
	uint32_t cacheLine = DMA_CACHE_LINE;
	void *mem = (void*)CACHED_MALLOC_ATOMIC(size + cacheLine + sizeof(void*) -1 );
	void **ptr = (void**)((long)(mem+cacheLine+sizeof(void*)) & ~(cacheLine-1));
	ptr[-1] = mem;
	*phy_addr_p = (uint32_t)VIRT_TO_PHYS(ptr);
	INV_RANGE((uint32_t)ptr, size);
	return (void *)CACHE_TO_NONCACHE(ptr);
#endif
}

static void alignedFree(void *ptr, uint32_t size)
{
#if defined(__KERNEL__) && defined(CONFIG_ARM)
        uint32_t size32 = (uint32_t)(size + sizeof(void*) + 3) & (uint32_t)(~3); // must be multiple of 4B
        NONCACHED_FREE(size, ptr, ((uint32_t *)(ptr))[(size32-sizeof(void*))>>2]);
#else
	CACHED_FREE(((void**)ptr)[-1]);
#endif
}

#endif

static void memsetl_context (void *__to, unsigned int __val, unsigned int __n)
{
    volatile unsigned int *dst = (volatile unsigned int *)__to;
    int          i;

    for (i = 0; i < ( __n / 4 ); i++, dst++)
    {
        if (( i & 0x3 ) == 3)
            continue;

        *dst = __val;
    }
}

#if !defined (FIRMWARE_INIT) && !defined (FSSIM)
static void memcpyl_context (void *__to, void *__from, unsigned int __n)
{
    volatile unsigned int *src = (volatile unsigned int *)__from;
    volatile unsigned int *dst = (volatile unsigned int *)__to;
    int          i;

    for (i = 0; i < ( __n / 4 ); i++, src++, dst++)
    {
        if (( i & 0x3 ) == 3)
            continue;

        *dst = MGET_32(src);
    }
}
#endif

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
    MEMSET ( sram_fast_program_ptr, 0, sizeof ( RUNNER_INST_MAIN ) );

    sram_fast_program_ptr = ( RUNNER_INST_MAIN * )DEVICE_ADDRESS( RUNNER_INST_MAIN_1_OFFSET );
    MEMSET ( sram_fast_program_ptr, 0, sizeof ( RUNNER_INST_MAIN ) );

    sram_pico_program_ptr = ( RUNNER_INST_PICO * )DEVICE_ADDRESS( RUNNER_INST_PICO_0_OFFSET );
    MEMSET ( sram_pico_program_ptr, 0, sizeof ( RUNNER_INST_PICO ) );

    sram_pico_program_ptr = ( RUNNER_INST_PICO * )DEVICE_ADDRESS( RUNNER_INST_PICO_1_OFFSET );
    MEMSET ( sram_fast_program_ptr, 0, sizeof ( RUNNER_INST_PICO ) );

    /* reset SRAM common data memory of both Runners */
    sram_common_data_ptr = ( RUNNER_COMMON * )DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET );
    MEMSET ( sram_common_data_ptr, 0, sizeof ( RUNNER_COMMON ) );

    sram_common_data_ptr = ( RUNNER_COMMON * )DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET );
    MEMSET ( sram_common_data_ptr, 0, sizeof ( RUNNER_COMMON ) );

    /* reset SRAM private data memory of both Runners */
    sram_private_data_ptr = ( RUNNER_PRIVATE * )DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET );
    MEMSET ( sram_private_data_ptr, 0, sizeof ( RUNNER_PRIVATE ) );

    sram_private_data_ptr = ( RUNNER_PRIVATE * )DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET );
    MEMSET ( sram_private_data_ptr, 0, sizeof ( RUNNER_PRIVATE ) );

    /* reset SRAM context memory of both Runners */
    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_0_OFFSET );
    memsetl_context ( sram_fast_context_ptr, 0, sizeof ( RUNNER_CNTXT_MAIN ) );

    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_1_OFFSET );
    memsetl_context ( sram_fast_context_ptr, 0, sizeof ( RUNNER_CNTXT_MAIN ) );

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_0_OFFSET );
    memsetl_context ( sram_pico_context_ptr, 0, sizeof ( RUNNER_CNTXT_PICO ) );

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_1_OFFSET );
    memsetl_context ( sram_pico_context_ptr, 0, sizeof ( RUNNER_CNTXT_PICO ) );

    /* reset SRAM prediction memory of both Runners */
    sram_fast_prediction_ptr = ( RUNNER_PRED_MAIN * )DEVICE_ADDRESS( RUNNER_PRED_MAIN_0_OFFSET );
    MEMSET ( sram_fast_prediction_ptr, 0, sizeof ( RUNNER_PRED_MAIN ) * 2 );

    sram_fast_prediction_ptr = ( RUNNER_PRED_MAIN * )DEVICE_ADDRESS( RUNNER_PRED_MAIN_1_OFFSET );
    MEMSET ( sram_fast_prediction_ptr, 0, sizeof ( RUNNER_PRED_MAIN ) * 2 );

    sram_pico_prediction_ptr = ( RUNNER_PRED_PICO * )DEVICE_ADDRESS( RUNNER_PRED_PICO_0_OFFSET );
    MEMSET ( sram_pico_prediction_ptr, 0, sizeof ( RUNNER_PRED_PICO ) * 2 );

    sram_pico_prediction_ptr = ( RUNNER_PRED_PICO * )DEVICE_ADDRESS( RUNNER_PRED_PICO_1_OFFSET );
    MEMSET ( sram_pico_prediction_ptr, 0, sizeof ( RUNNER_PRED_PICO ) * 2 );

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
#ifdef CONFIG_DHD_RUNNER
    bdmf_free( g_dhd_tx_cpu_usage_reference_array );
#endif
#else
    KFREE( g_cpu_tx_skb_pointers_reference_array );
#ifdef CONFIG_DHD_RUNNER
    KFREE( g_dhd_tx_cpu_usage_reference_array );
#endif
#endif
    alignedFree( ( void * )g_cpu_tx_data_pointers_reference_array, sizeof( uint32_t ) *
        LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT );
    alignedFree( ( void * )g_free_skb_indexes_fifo_table, sizeof( uint16_t ) * LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT );
    alignedFree( ( void * )g_rdd_free_skb_indexes_fifo_table, sizeof( uint16_t ) * LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT );
#endif
}


BL_LILAC_RDD_ERROR_DTE rdd_load_microcode ( uint8_t  *xi_runer_A_microcode_ptr,
                                            uint8_t  *xi_runer_B_microcode_ptr,
                                            uint8_t  *xi_runer_C_microcode_ptr,
                                            uint8_t  *xi_runer_D_microcode_ptr )
{
    RUNNER_INST_MAIN  *sram_fast_program_ptr;
    RUNNER_INST_PICO  *sram_pico_program_ptr;

    /* load the code segment into the SRAM program memory of fast Runner A */
    sram_fast_program_ptr = ( RUNNER_INST_MAIN * )DEVICE_ADDRESS( RUNNER_INST_MAIN_0_OFFSET );
    MWRITE_BLK_32( sram_fast_program_ptr, xi_runer_A_microcode_ptr, sizeof ( RUNNER_INST_MAIN ) );

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
    unsigned int  src = (unsigned int )__from;
    unsigned int  dst = (unsigned int )__to;
    int           i;

    for ( i = 0; i < ( __n / 2 ); i++, src+=2, dst+=4 )
    {
        *( volatile unsigned int * )dst = (unsigned int)(*( volatile unsigned short * )src);
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
    runner_global_control_register.main_en = LILAC_RDD_TRUE;
    runner_global_control_register.pico_en = LILAC_RDD_TRUE;
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );

    /* enable Runner B through the global control register */
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register.main_en = LILAC_RDD_TRUE;
    runner_global_control_register.pico_en = LILAC_RDD_TRUE;
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
    /* initialize the base address of the packets in the ddr */
    g_runner_ddr_base_addr = init_params->ddr_pool_ptr;
    g_runner_ddr_base_addr_phys = init_params->ddr_pool_ptr_phys;
    g_runner_extra_ddr_base_addr = init_params->extra_ddr_pool_ptr;
    g_runner_extra_ddr_base_addr_phys = init_params->extra_ddr_pool_ptr_phys;

    g_wan_physical_port = init_params->wan_physical_port;

    g_runner_tables_ptr = ( uint32_t )init_params->ddr_runner_tables_ptr;

    /* enable runner context memories */
    if ( init_params->chip_revision == RDD_CHIP_REVISION_B0 )
    {
        f_rdd_enable_context_memory ();
    }

#ifdef G9991
    f_ds_tx_offload_pd_ddr_queue_initialize ( );
#endif

    g_bpm_buffer_size = init_params->bpm_buffer_size;
    g_us_ddr_queue_enable = init_params->us_ddr_queue_enable;
    f_rdd_wan_tx_queue_ddr_offload_initialize( g_us_ddr_queue_enable );

    g_ddr_headroom_size = init_params->ddr_headroom_size;

    g_broadcom_switch_mode = init_params->broadcom_switch_mode;
    g_broadcom_switch_physical_port = init_params->broadcom_switch_physical_port;

    g_bridge_flow_cache_mode = init_params->bridge_flow_cache_mode;
    g_chip_revision = init_params->chip_revision;

    /* initialize the RDD lock/Unlock mechanism */
    f_rdd_lock = f_rdd_dummy_lock;
    f_rdd_unlock = f_rdd_dummy_unlock;
    f_rdd_lock_irq = f_rdd_dummy_lock_irq;
    f_rdd_unlock_irq = f_rdd_dummy_unlock_irq;

#ifndef RDD_BASIC
    /* DDR reset */
    MEMSET ( ( void * )DsConnectionTableBase, 0, sizeof ( RDD_CONNECTION_TABLE_DTS ) );
    MEMSET ( ( void * )UsConnectionTableBase, 0, sizeof ( RDD_CONNECTION_TABLE_DTS ) );
    MEMSET ( ( void * )ContextTableBase, 0, sizeof ( RDD_CONTEXT_TABLE_DTS ) );
    MEMSET ( ( void * )IPTVTableBase, 0, sizeof( RDD_IPTV_DDR_LOOKUP_TABLE_DTS ) );
    MEMSET ( ( void * )IPTVContextTableBase, 0, sizeof( RDD_IPTV_DDR_CONTEXT_TABLE_DTS ) );
    MEMSET ( ( void * )IPTVSsmContextTableBase, 0, sizeof ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS ) );
#ifdef FIRMWARE_INIT
    MEMSET ( ( void * )CpuRxRingBase, 0, sizeof( RDD_CPU_RX_DESCRIPTOR_DTS ) * RDD_RING_DESCRIPTORS_TABLE_SIZE * 10 );
#endif
#endif

    /* initialize the base address of the BPM base address */
    f_rdd_bpm_initialize ( (uint32_t)g_runner_ddr_base_addr, (uint32_t)g_runner_extra_ddr_base_addr, g_ddr_headroom_size );

    /* initialize runner dma base address */
    f_rdd_ddr_initialize ( (uint32_t)g_runner_ddr_base_addr, g_ddr_headroom_size, g_chip_revision );

    /* initialize runner dma base address */
    f_rdd_psram_initialize ();

    /* initialize scheduler */
    f_rdd_scheduler_initialize ();

    /* create the Runner's free packet descriptors pool */
    f_rdd_free_packet_descriptors_pool_initialize ();

#if !defined(FIRMWARE_INIT)
    /* tune runner */
    TuneRunner();
#endif

    /* initialize the CPU-RX mechanism */
    rdd_cpu_rx_initialize ();

    /* initialize the CPU-TX queue */
    rdd_cpu_tx_initialize ();

#ifndef RDD_BASIC
    /* initialize the MAC HW accelerator and the software CRC functionality */
    f_rdd_mac_table_initialize ( init_params->mac_table_size, init_params->iptv_table_size );

    /* initialize IPTV forwarding free list and the address of IPTV context map table in SRAM */
    f_rdd_iptv_initialize ();
#endif
    f_rdd_ingress_classification_table_initialize ();

    f_rdd_port_to_bbh_destination_table_initialize ();

    /* initialize global registers */
    f_rdd_global_registers_initialize ();

    /* initialize the local registers through the Context memory */
    f_rdd_local_registers_initialize (init_params->lan_direct_to_cpu_port);

    /* initialize ethernet tx queues and ports */
    f_rdd_eth_tx_initialize ();

    /* initialize WAN tx */
    f_rdd_wan_tx_initialize ();

    /* initialize inter task queues */
    f_rdd_inter_task_queues_initialize ();

    /* initialize broadcom switch mode */
    if ( init_params->broadcom_switch_mode )
    {
        f_rdd_broadcom_switch_mode_initialize ();
    }

    /* initialize PM counters */
    f_rdd_pm_counters_initialize ();

#ifndef RDD_BASIC
    /* initialize filters cam */
    f_rdd_ingress_filters_cam_initialize ();

    /* initialize downstream layer4 filters cam */
    f_rdd_layer4_filters_initialize ();

    /* initialize vlan matrix */
    f_rdd_vlan_matrix_initialize ();

    /* initialize connection table */
    f_rdd_connection_table_initialize ();

    /* initialize the multicast vector to number of copies table */
    f_rdd_multicast_initialize ();

    /* initialize vid CAM table */
    f_rdd_vid_cam_initialize ();

    /* initialize firewall structures */
    rdd_firewall_initialize ();

    /* set full flow cache/hybrid mode indication */
    f_rdd_full_flow_cache_config ( g_bridge_flow_cache_mode );

    /* initialize service queues */
    rdd_service_queues_initialize ();
#endif
    /* 1588 mechansism initialize */
    f_rdd_1588_initialize ();
    /* initialize free skb indexes fifo and pointers*/
    f_rdd_transmit_from_abs_address_initialize ();
#ifndef G9991
    /* initialize structures supporting parallel processing */
    f_rdd_parallel_processing_initialize ();
#else
    /* initialize structures supporting G9991 */
    f_rdd_G9991_initialize ();
#endif
    /* initialize structures supporting service queues */
    f_rdd_tm_service_queues_initialize ();

#ifdef USE_BDMF_SHELL
    /* register shell commands */
    f_rdd_make_shell_commands ();
#endif

#ifdef CONFIG_DHD_RUNNER
    rdd_dhd_mode_enable_init();
#endif
    rdd_rate_controller_sustain_budget_limit_config ( 2048 );
#ifndef G9991
    rdd_speed_service_initialize();
#endif

#if !defined(RDD_BASIC)
#ifndef G9991
    rdd_actions_proj_init();
#endif
#endif

#if !defined(RDD_BASIC)
    /* default threshold size value for ack packets */
    rdd_ack_packet_size_threshold_config(ACK_PACKET_SIZE_THRESHOLD);
#endif

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_enable_context_memory ( void )
{
#if !defined(FIRMWARE_INIT)
    uint32_t  runner_global_control_register;

    /* enable context memories (Oren B0 only) */
    /*  TODO through reggae bitfield after reggae updated */
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register |= 1 << 26;
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );

    RUNNER_REGS_1_CFG_GLOBAL_CTRL_READ ( runner_global_control_register );
    runner_global_control_register |= 1 << 26;
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_WRITE ( runner_global_control_register );
#endif

    return ( BL_LILAC_RDD_OK );
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
static BL_LILAC_RDD_ERROR_DTE f_rdd_bpm_initialize ( uint32_t  xi_runner_ddr_pool_ptr,
                                                     uint32_t  xi_runner_extra_ddr_pool_ptr,
                                                     uint32_t  xi_ddr_headroom_size )
{
    uint32_t  *bpm_ddr_base_ptr;
    uint32_t  *bpm_extra_ddr_base_ptr;
    uint32_t  *bpm_ddr_base_with_rx_offset_ptr;

    bpm_ddr_base_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BPM_DDR_BUFFERS_BASE_ADDRESS );
    bpm_extra_ddr_base_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BPM_EXTRA_DDR_BUFFERS_BASE_ADDRESS );
    bpm_ddr_base_with_rx_offset_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + BPM_DDR_OPTIMIZED_BUFFERS_WITHOUT_HEADROOM_BASE_ADDRESS );

    MWRITE_32( bpm_ddr_base_ptr, xi_runner_ddr_pool_ptr & 0x1FFFFFFF );
    MWRITE_32( bpm_extra_ddr_base_ptr, xi_runner_extra_ddr_pool_ptr & 0x1FFFFFFF );
    MWRITE_32( bpm_ddr_base_with_rx_offset_ptr, ( xi_runner_ddr_pool_ptr + LILAC_RDD_PACKET_DDR_OFFSET ) & 0x1FFFFFFF );

    bpm_ddr_base_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BPM_DDR_BUFFERS_BASE_ADDRESS );
    bpm_extra_ddr_base_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BPM_EXTRA_DDR_BUFFERS_BASE_ADDRESS );

    MWRITE_32( bpm_ddr_base_ptr, xi_runner_ddr_pool_ptr & 0x1FFFFFFF );
    MWRITE_32( bpm_extra_ddr_base_ptr, xi_runner_extra_ddr_pool_ptr & 0x1FFFFFFF );

    f_rdd_ddr_optimized_base_config ( xi_runner_ddr_pool_ptr, xi_ddr_headroom_size );

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
/*   xi_runner_ddr_pool_ptr - Packet DDR buffer base address                  */
/*   xi_ddr_headroom_size - configurable headroom in addition to              */
/*   LILAC_RDD_PACKET_DDR_OFFSET                                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_ddr_initialize ( uint32_t                        xi_runner_ddr_pool_ptr,
                                                     uint32_t                        xi_ddr_headroom_size,
                                                     BL_LILAC_RDD_CHIP_REVISION_DTE  xi_chip_revision )
{
    RUNNER_REGS_CFG_DDR_CFG         runner_ddr_config_register;
    RUNNER_REGS_CFG_DDR_LKUP_MASK0  runner_ddr_lkup_mask0_register;
    RUNNER_REGS_CFG_DDR_LKUP_MASK1  runner_ddr_lkup_mask1_register;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    uint32_t                        *ddr_address_ptr;
    uint32_t                        ddr_address_for_sync;
   
    runner_ddr_config_register.buffer_offset = LILAC_RDD_PACKET_DDR_OFFSET;
    runner_ddr_config_register.rserved1 = 0;
    if ( g_bpm_buffer_size == RDPA_BPM_BUFFER_2_5K )
    {
#ifndef FIRMWARE_INIT
        printk ( "ERROR: unsupported BPM buffer size configuration bpm_buffer_size=%d\n", g_bpm_buffer_size );  
#endif
        return ( BL_LILAC_RDD_OK );
    }
    else
        runner_ddr_config_register.buffer_size = ( g_bpm_buffer_size >> 12 );
    runner_ddr_config_register.dma_base = ( xi_runner_ddr_pool_ptr & 0x1FE00000 ) >> 21;
    runner_ddr_config_register.rserved2 = 0;
#ifdef CONFIG_BCM96848
    runner_ddr_config_register.new_address_calc = 0;
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

    /* write headroom size to runner memory, so that runner will set it upon init */
    f_rdd_ddr_headroom_size_private_config ( xi_ddr_headroom_size );
    ddr_address_for_sync = ( g_runner_tables_ptr + DDR_ADDRESS_FOR_SYNC_DMA_ADDRESS );

    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PACKET_BUFFER_SIZE_ASR_8_ADDRESS );
	MWRITE_8(ddr_address_ptr, LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE >> 8);

	ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PACKET_BUFFER_SIZE_ASR_8_ADDRESS );
	MWRITE_8(ddr_address_ptr, LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE >> 8);

    if ( xi_chip_revision == RDD_CHIP_REVISION_B0 )
    {
        /* Use write reple address */
        ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DDR_ADDRESS_FOR_SYNC_DMA_POINTER_ADDRESS );
        MWRITE_32( ddr_address_ptr, DMA_SYNCHRONIZATION_DUMMY_ADDRESS );

        /* Use write reple address */
        ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DDR_ADDRESS_FOR_SYNC_DMA_POINTER_ADDRESS );
        MWRITE_32( ddr_address_ptr, DMA_SYNCHRONIZATION_DUMMY_ADDRESS );

    }
    else
    {
        /* full sync read from dummy ddr address done in some corner cases in downstream to avoid write reply in preceeding write */
        ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DDR_ADDRESS_FOR_SYNC_DMA_POINTER_ADDRESS );
        MWRITE_32( ddr_address_ptr, ( ddr_address_for_sync & 0x1FFFFFFF ) );

        /* for upstream path sync, use regular DDR address instead of write reply */
        ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DDR_ADDRESS_FOR_SYNC_DMA_POINTER_ADDRESS );
        MWRITE_32( ddr_address_ptr, ( ddr_address_for_sync & 0x1FFFFFFF ) );

        ddr_address_ptr = ( uint32_t * )( DdrAddressForSyncDmaBase );
        MWRITE_32( ddr_address_ptr, 0xffffffff);
    }

    /* save regular DDR address for full read sync */
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DDR_ADDRESS_FOR_FULL_READ_DMA_POINTER_ADDRESS );
    MWRITE_32( ddr_address_ptr, ( ddr_address_for_sync & 0x1FFFFFFF ) );

    /* save regular DDR address for full read sync */
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DDR_ADDRESS_FOR_FULL_READ_DMA_POINTER_ADDRESS );
    MWRITE_32( ddr_address_ptr, ( ddr_address_for_sync & 0x1FFFFFFF ) );

    /* save BPM buffer size */
    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    RDD_BRIDGE_CONFIGURATION_REGISTER_PACKET_BUFFER_SIZE_ASR_8_WRITE( g_bpm_buffer_size >> 8, bridge_cfg_register );
 
    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    RDD_BRIDGE_CONFIGURATION_REGISTER_PACKET_BUFFER_SIZE_ASR_8_WRITE( g_bpm_buffer_size >> 8, bridge_cfg_register );

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
    runner_psram_config_register.buffer_size = RUNNER_REGS_CFG_PSRAM_CFG_BUFFER_SIZE_BUFFER_SIZE_128BYTE_VALUE;
    runner_psram_config_register.dma_base = 0;
    runner_psram_config_register.rserved2 = 0;

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
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0 );

    RUNNER_REGS_0_CFG_PICO_SCH_CFG_WRITE ( runner_scheduler_cfg_register );

    /* pico Runner B - class A */
    runner_scheduler_cfg_register = ( RUNNER_REGS_CFG_PICO_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE << 5 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_A_USE_CLASS_A_VALUE << 4 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_15_8_RR_VALUE << 1 ) |
                                    ( RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0 );

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
/*   Upstream pool is implemented as a stack of 2048 packet descriptors       */
/*   Downstream pool is implemented as a list of 1024 packet descriptors      */
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
#ifndef G9991
    RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_DTS                *ds_free_packet_descriptors_pool_ptr;
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS  *free_packet_descriptors_pool_descriptor_ptr;
#endif
    RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS                *us_free_packet_descriptors_pool_ptr;
    RDD_PACKET_DESCRIPTOR_DTS                              *packet_descriptor_ptr;
    uint32_t                                               next_packet_descriptor_address;
    uint32_t                                               i;

#ifndef G9991
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
            next_packet_descriptor_address = DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ( i + 1 ) * LILAC_RDD_RUNNER_PACKET_DESCRIPTOR_SIZE;
        }

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_WRITE ( next_packet_descriptor_address, packet_descriptor_ptr );
    }

    free_packet_descriptors_pool_descriptor_ptr = ( RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS );

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_WRITE ( DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS, free_packet_descriptors_pool_descriptor_ptr );

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_WRITE ( DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1 ) * LILAC_RDD_RUNNER_PACKET_DESCRIPTOR_SIZE,
                                                                           free_packet_descriptors_pool_descriptor_ptr );

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_WRITE ( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1, free_packet_descriptors_pool_descriptor_ptr );
#endif

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
            next_packet_descriptor_address = US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ( i + 1 ) * LILAC_RDD_RUNNER_PACKET_DESCRIPTOR_SIZE;
        }

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_WRITE ( next_packet_descriptor_address, packet_descriptor_ptr );
    }

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

    global_register[ 2 ] = ( g_broadcom_switch_mode << GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET ) |
                           ( 1 << GLOBAL_CFG_FLOW_CACHE_MODE_BIT_OFFSET ) |
                           ( g_bridge_flow_cache_mode << GLOBAL_CFG_BRIDGE_FLOW_CACHE_MODE_BIT_OFFSET ) |
                           ( g_chip_revision << GLOBAL_CFG_CHIP_REVISION_OFFSET ) |
                           ( 0 << GLOBAL_CFG_EPON_MODE_BIT_OFFSET ) |
                           ( 0 << DS_GLOBAL_CFG_AE_MODE_BIT_OFFSET ); 
#ifndef G9991
    global_register[ 3 ] = ( DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS << 16 ) | DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS;
#endif
#ifdef G9991
    global_register[ 4 ] = G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_ADDRESS;
#else
    global_register[ 4 ] = DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS << 16 | CPU_FLOW_CACHE_INGRESS_QUEUE_ADDRESS;
#endif

    /* R5 low - context_index_cache_write_index */
    global_register[ 5 ] = DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS << 16;

    global_register[ 6 ] = ( DOWNSTREAM_MULTICAST_INGRESS_QUEUE_ADDRESS << 16 ) | DOWNSTREAM_MULTICAST_INGRESS_QUEUE_ADDRESS;

#ifdef CONFIG_DHD_RUNNER
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
#ifndef G9991
#ifdef CONFIG_DHD_RUNNER
    global_register[ 3 ] = (US_DHD_TX_POST_INGRESS_QUEUE_ADDRESS << 16) | US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS;
#else
    global_register[ 3 ] = US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS;
#endif
#endif
    /* R4 - free buffers pool counter */
    global_register[ 4 ] = RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE;

    /* R5 - context_index_cache_write_index */
    global_register[ 5 ] = 0;

    global_register[ 6 ] = ( WAN_INTERWORKING_INGRESS_QUEUE_ADDRESS << 16 ) | WAN_INTERWORKING_INGRESS_QUEUE_ADDRESS;

    global_register[ 7 ] = ( LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_ADDRESS << 16 ) |
                           ( g_broadcom_switch_mode << GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET ) |
                           ( g_chip_revision << GLOBAL_CFG_CHIP_REVISION_OFFSET ) |
                           ( 0 << GLOBAL_CFG_EPON_MODE_BIT_OFFSET );

    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS );

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at task -1 (initialization task) */
    MWRITE_BLK_32( global_register_init_ptr, global_register, sizeof ( global_register ) );


    /********** Pico Runner A **********/

    /* zero all global registers */
    memset ( global_register, 0, sizeof ( global_register ) );

    /* R1 - constant one */
    global_register[ 1 ] = 1;

    global_register[ 2 ] = ( g_chip_revision << GLOBAL_CFG_CHIP_REVISION_OFFSET );

    global_register[ 3 ] = 64 - 1;

    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS );

    /* copy the global regsiters to the data SRAM, the firmware will load it from the SRAM at task -1 (initialization task) */
    MWRITE_BLK_32( global_register_init_ptr, global_register, sizeof ( global_register ) );


    /********** Pico Runner B **********/

    /* zero all global registers */
    memset ( global_register, 0, sizeof ( global_register ) );

    /* R1 - constant one */
    global_register[ 1 ] = 1;

    global_register[ 2 ] = US_CPU_TX_BBH_DESCRIPTORS_ADDRESS << 16;

#ifndef G9991
    global_register[ 3 ] = US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS;
#endif
    /* vlan action cyclic ingress queue pointer */
    global_register[ 5 ] = ( VLAN_ACTION_BRIDGE_INGRESS_QUEUE_ADDRESS << 16 ) | US_ROUTER_INGRESS_QUEUE_ADDRESS;

    global_register[ 6 ] =  UPSTREAM_FLOODING_INGRESS_QUEUE_ADDRESS;

    global_register[ 7 ] = ( LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_ADDRESS << 16 ) |
                           ( g_bridge_flow_cache_mode << GLOBAL_CFG_BRIDGE_FLOW_CACHE_MODE_BIT_OFFSET ) |
                           ( 1 << GLOBAL_CFG_FLOW_CACHE_MODE_BIT_OFFSET ) |
                           ( g_broadcom_switch_mode << GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET ) |
                           ( g_chip_revision << GLOBAL_CFG_CHIP_REVISION_OFFSET );

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
static BL_LILAC_RDD_ERROR_DTE f_rdd_local_registers_initialize ( BL_LILAC_RDD_BRIDGE_PORT_DTE        lan_direct_to_cpu_port )
{
    RUNNER_CNTXT_MAIN  *sram_fast_context_ptr;
    RUNNER_CNTXT_PICO  *sram_pico_context_ptr;
    static uint32_t    local_register[ 32 ][ 32 ];
    uint32_t           rx_descriptors_address, bbh_peripheral_rx;
    bdmf_boolean       lan_direct_to_cpu_en = 1;

    /********** Fast Runner A **********/

    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_0_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_fast_context_ptr, sizeof ( RUNNER_CNTXT_MAIN ) );

    /* CPU TX fast: thread 0 */
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, cpu_tx_wakeup_request) << 16;
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R8  ] = CPU_TX_FAST_QUEUE_ADDRESS;
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R9  ] = ( INGRESS_HANDLER_BUFFER_ADDRESS << 16 ) | DS_CPU_TX_BBH_DESCRIPTORS_ADDRESS;
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R10 ] = ( BBH_PERIPHERAL_IH << 16 ) | ( LILAC_RDD_IH_BUFFER_BBH_ADDRESS + LILAC_RDD_RUNNER_A_IH_BUFFER_BBH_OFFSET );
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;

    /* CPU-RX: thread 1 */
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, cpu_rx_wakeup_request) << 16;
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R8  ] = CPU_RX_FAST_PD_INGRESS_QUEUE_ADDRESS | ( CPU_RX_MIRRORING_PD_INGRESS_QUEUE_ADDRESS << 16 );
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R9  ] = DS_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS + ( DS_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS << 16 );
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R11 ] = DS_CPU_REASON_TO_METER_TABLE_ADDRESS | ( CPU_RX_PD_INGRESS_QUEUE_ADDRESS << 16 );

#ifdef CONFIG_DHD_RUNNER
    /* DHD TX complete Fast_A : thread 16 */
    local_register[ DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_complete_wakeup_request) << 16;
    local_register[ DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R8 ] =  (DHD_RADIO_OFFSET_COMMON_B(0) << 16) | DHD_RADIO_OFFSET_COMMON_A(0);

    /* DHD1 TX complete Fast_A : thread 17 */
    local_register[ DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_complete_wakeup_request) << 16;
    local_register[ DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R8 ] =  (DHD_RADIO_OFFSET_COMMON_B(1) << 16) | DHD_RADIO_OFFSET_COMMON_A(1);

    /* DHD2 TX complete Fast_A : thread 18 */
    local_register[ DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_complete_wakeup_request) << 16;
    local_register[ DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER ][ CS_R8 ] =  (DHD_RADIO_OFFSET_COMMON_B(2) << 16) | DHD_RADIO_OFFSET_COMMON_A(2);

    /* DHD TX post Fast_A (WAN->Dongle): thread 3 */
    local_register[ DHD_TX_POST_FAST_A_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_a, dhd_tx_post) << 16;
    local_register[ DHD_TX_POST_FAST_A_THREAD_NUMBER ][ CS_R9 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD_TX_POST_FAST_A_THREAD_NUMBER ][ CS_R10 ] = DHD_TX_POST_PD_INGRESS_QUEUE_ADDRESS | DS_DHD_TX_POST_INGRESS_QUEUE_ADDRESS << 16;
#endif

    /* Timer scheduler: thread 4 */
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R21 ] = DS_CPU_RX_METER_TABLE_ADDRESS;

    /* DS Policers budget allocator: thread 5 */
    local_register[ POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, policer_budget_allocator_1st_wakeup_request) << 16;

    /* WAN direct: thread 7 */
    local_register[ WAN_DIRECT_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, wan_direct_wakeup_request) << 16;
    local_register[ WAN_DIRECT_THREAD_NUMBER ][ CS_R8  ] = GPON_RX_DIRECT_DESCRIPTORS_ADDRESS;
    local_register[ WAN_DIRECT_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* WAN Filters and Classification : thread 9 */
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, wan_normal_wakeup_request) << 16;
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] = GPON_RX_NORMAL_DESCRIPTORS_ADDRESS << 16;
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R9  ] = LILAC_RDD_CAM_RESULT_SLOT6 | ( LILAC_RDD_CAM_RESULT_SLOT6_IO_ADDRESS << 16 );
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R13 ] = ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT0 << 5 ) | LILAC_RDD_DMA_LOOKUP_4_STEPS | ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT0_IO_ADDRESS << 16 );

    if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_FIBER ) 
    {
        local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] |= BBH_PERIPHERAL_GPON_RX;
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH4 )
    {
        local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] |= BBH_PERIPHERAL_ETH4_RX;
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0 )
    {
        local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] |= BBH_PERIPHERAL_ETH0_RX;
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH1 )
    {
        local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] |= BBH_PERIPHERAL_ETH1_RX;
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH2 )
    {
        local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] |= BBH_PERIPHERAL_ETH2_RX;
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH3 )
    {
        local_register[ WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] |= BBH_PERIPHERAL_ETH3_RX;
    }

#ifdef G9991
    /* Downstream G9991_fragment: thread 10 */
    local_register[ G9991_FRAGMENT_THREAD_NUMBER ][ CS_R8 ] = G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_ADDRESS;
    local_register[ G9991_FRAGMENT_THREAD_NUMBER ][ CS_R9 ] = G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_ADDRESS;
    local_register[ G9991_FRAGMENT_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ G9991_FRAGMENT_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, G9991_fragment_wakeup_request) << 16;
#else
    /* FLOW_CACHE : thread 11,12,13,14 */
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R8  ] = CONNECTION_BUFFER_TABLE_ADDRESS + 0 * RDD_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R9  ] = ( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS << 16 ) | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE0_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R11 ] = SPEED_SERVICE_STREAM_PREFIX_ADDRESS;
        
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R8  ] = CONNECTION_BUFFER_TABLE_ADDRESS + 1 * RDD_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R9  ] = (( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 1 ) << 16 ) | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE1_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R11 ] = SPEED_SERVICE_STREAM_PREFIX_ADDRESS;

    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R8  ] = CONNECTION_BUFFER_TABLE_ADDRESS + 2 * RDD_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R9  ] = (( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 2 ) << 16 ) | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE2_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R11 ] = SPEED_SERVICE_STREAM_PREFIX_ADDRESS;

    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, flow_cache_wakeup_request) << 16;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R8  ] = CONNECTION_BUFFER_TABLE_ADDRESS + 3 * RDD_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R9  ] = (( DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 3 ) << 16 )  | ADDRESS_OF(runner_a, flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE3_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R11 ] = SPEED_SERVICE_STREAM_PREFIX_ADDRESS;

    /* CPU Flow Cache : thread 15 */
    local_register[ DOWNSTREAM_CPU_FLOW_CACHE_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, cpu_flow_cache_wakeup_request) << 16;
    local_register[ DOWNSTREAM_CPU_FLOW_CACHE_THREAD_NUMBER ][ CS_R8  ] = CONNECTION_BUFFER_TABLE_ADDRESS + 4 * RDD_CONNECTION_BUFFER_TABLE_SIZE2 * sizeof(RDD_CONNECTION_ENTRY_DTS);
    local_register[ DOWNSTREAM_CPU_FLOW_CACHE_THREAD_NUMBER ][ CS_R9  ] = CPU_FLOW_CACHE_INGRESS_QUEUE_ADDRESS << 16 | ADDRESS_OF(runner_a, cpu_flow_cache_wakeup_request);
    local_register[ DOWNSTREAM_CPU_FLOW_CACHE_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
#endif
    /* CPU downstream Filters and Classification : thread 25 */
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, wan_cpu_wakeup_request) << 16;
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R8  ] = ( DS_CPU_TX_BBH_DESCRIPTORS_ADDRESS << 16 ) | ( 1 << WAN_FILTERS_AND_CLASSIFICATON_R8_CPU_INDICATION_OFFSET );
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R9  ] = LILAC_RDD_CAM_RESULT_SLOT2 | ( LILAC_RDD_CAM_RESULT_SLOT2_IO_ADDRESS << 16 );
    local_register[ CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* Downstream Multicast: thread 28 */
    local_register[ DOWNSTREAM_MULTICAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, downstream_multicast_wakeup_request) << 16;
    local_register[ DOWNSTREAM_MULTICAST_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#else
    memcpyl_context ( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#endif


    /********** Fast Runner B **********/

    sram_fast_context_ptr = ( RUNNER_CNTXT_MAIN * )DEVICE_ADDRESS( RUNNER_CNTXT_MAIN_1_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_fast_context_ptr, sizeof ( RUNNER_CNTXT_MAIN ) );

    /* CPU-TX: thread 0 */
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, cpu_tx_wakeup_request) << 16;
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R8  ] = CPU_TX_FAST_QUEUE_ADDRESS ;
#ifndef G9991
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R30  ] = 0; /* speed service enable flag */
    local_register[ CPU_TX_FAST_THREAD_NUMBER ][ CS_R31  ] = SPEED_SERVICE_PARAMETERS_TABLE_ADDRESS;
#endif

    /* CPU-RX: thread 1 */
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, cpu_rx_wakeup_request) << 16;
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R9  ] = US_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS + ( US_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS << 16 );
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ CPU_RX_THREAD_NUMBER ][ CS_R11 ] = US_CPU_REASON_TO_METER_TABLE_ADDRESS;

    /* Smart card : thread 2 */
    local_register[ SMART_CARD_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, smart_card_send_and_recieve) << 16;

    /* upstream rate controllers budget allocator: thread 3 */
    local_register[ RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R14 ] = RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS;
    local_register[ RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, rate_control_budget_allocator_1st_wakeup_request) << 16;
    local_register[ RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R18 ] = US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_ADDRESS;

    /* Timer scheduler: thread 4 */
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R20 ] = US_RATE_LIMITER_TABLE_ADDRESS;
    local_register[ TIMER_SCHEDULER_MAIN_THREAD_NUMBER ][ CS_R21 ] = US_CPU_RX_METER_TABLE_ADDRESS;

    /* US Policers budget allocator: thread 5 */
    local_register[ POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, policer_budget_allocator_1st_wakeup_request) << 16;

    /* EPON-TX request: thread 6 */
    local_register[ EPON_TX_REQUEST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, epon_tx_request_wakeup_request) << 16;
#ifndef G9991
    local_register[ EPON_TX_REQUEST_THREAD_NUMBER ][ CS_R31 ] = SPEED_SERVICE_PARAMETERS_TABLE_ADDRESS;
#endif

    /* WAN-TX: thread 7 */
#ifndef G9991
    if ( g_us_ddr_queue_enable )
        local_register[ WAN_TX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, wan_tx_ddr_wakeup_request) << 16;
    else
        local_register[ WAN_TX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, wan_tx_wakeup_request) << 16;
    local_register[ WAN_TX_THREAD_NUMBER ][ CS_R31 ] = SPEED_SERVICE_PARAMETERS_TABLE_ADDRESS;
#else
    local_register[ WAN_TX_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, wan_tx_wakeup_request) << 16;
#endif

#ifndef G9991

#ifdef CONFIG_DHD_RUNNER
    /* DHD TX post Fast_B (LAN->Dongle): thread 8 */
    local_register[ DHD_TX_POST_FAST_B_THREAD_NUMBER ][ CS_R16 ] =  ADDRESS_OF(runner_b, dhd_tx_post) << 16;
    local_register[ DHD_TX_POST_FAST_B_THREAD_NUMBER ][ CS_R9 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD_TX_POST_FAST_B_THREAD_NUMBER ][ CS_R10 ] = US_DHD_TX_POST_INGRESS_QUEUE_ADDRESS << 16;
#endif

    /* SLAVE0: thread 10 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, flow_cache_wakeup_request) << 16;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R9  ] = ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS << 16 ) | ADDRESS_OF(runner_b, flow_cache_wakeup_request);
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE0_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER ][ CS_R11 ] = DUAL_STACK_LITE_TABLE_ADDRESS + ( 0 * sizeof(RDD_DUAL_STACK_LITE_ENTRY_DTS) );
        
    /* SLAVE1: thread 11 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, flow_cache_wakeup_request) << 16;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R9  ] = ( ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 1 ) << 16 ) | ADDRESS_OF(runner_b, flow_cache_wakeup_request);
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE1_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER ][ CS_R11 ] = DUAL_STACK_LITE_TABLE_ADDRESS + ( 1 * sizeof(RDD_DUAL_STACK_LITE_ENTRY_DTS) );
        
    /* SLAVE2: thread 12 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, flow_cache_wakeup_request) << 16;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R9  ] = ( ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 2 ) << 16 ) | ADDRESS_OF(runner_b, flow_cache_wakeup_request);
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE2_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER ][ CS_R11 ] = DUAL_STACK_LITE_TABLE_ADDRESS + ( 2 * sizeof(RDD_DUAL_STACK_LITE_ENTRY_DTS) );
        
    /* SLAVE3: thread 13 */
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, flow_cache_wakeup_request) << 16;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R9  ] = ( ( US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS + 3 ) << 16 ) | ADDRESS_OF(runner_b, flow_cache_wakeup_request);
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R10 ] = ( FLOW_CACHE_SLAVE3_VECTOR_MASK << 16 ) | INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER ][ CS_R11 ] = DUAL_STACK_LITE_TABLE_ADDRESS + ( 3 * sizeof(RDD_DUAL_STACK_LITE_ENTRY_DTS) );
#endif

    /* Upstream VLAN (from bridge): thread 14 */
    local_register[ UPSTREAM_VLAN_BRIDGE_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, upstream_vlan_wakeup_request) << 16;
    local_register[ UPSTREAM_VLAN_BRIDGE_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ UPSTREAM_VLAN_BRIDGE_THREAD_NUMBER ][ CS_R17 ] = VLAN_ACTION_BRIDGE_INGRESS_QUEUE_ADDRESS << 16;

#ifndef G9991
    /* Speed Service (spdsvc) : thread 16 */
    local_register[ SPEED_SERVICE_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, speed_service_timer_init_wakeup_request) << 16;
    local_register[ SPEED_SERVICE_THREAD_NUMBER ][ CS_R30  ] = 1; /* speed service enable flag */
    local_register[ SPEED_SERVICE_THREAD_NUMBER ][ CS_R31 ] = SPEED_SERVICE_PARAMETERS_TABLE_ADDRESS;
#endif

    /* WAN Interworking: thread 22 */
    local_register[ WAN_INTERWORKING_THREAD_NUMBER ][ CS_R16 ] = ( ADDRESS_OF(runner_b, wan_interworking_wakeup_request) << 16 );
    local_register[ WAN_INTERWORKING_THREAD_NUMBER ][ CS_R9 ] = ADDRESS_OF(runner_b, wan_interworking_wakeup_request);
    local_register[ WAN_INTERWORKING_THREAD_NUMBER ][ CS_R10  ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* Local switching Multicast: thread 28 */
    local_register[ LOCAL_SWITCHING_MULTICAST_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_b, local_switching_multicast_wakeup_request) << 16;
    local_register[ LOCAL_SWITCHING_MULTICAST_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* WAN to WAN : thread 31 */
    local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R16 ] = ( ADDRESS_OF(runner_b, wan_to_wan_wakeup_request) << 16 );
    local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R9  ] = GPON_RX_DIRECT_DESCRIPTORS_ADDRESS << 16;
    local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_FIBER ) 
    {
        local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R10 ] |= ( BBH_PERIPHERAL_GPON_RX << 16 );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH4 )
    {
        local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R10 ] |= ( BBH_PERIPHERAL_ETH4_RX << 16 );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0 )
    {
        local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R10  ] = ( BBH_PERIPHERAL_ETH0_RX << 16 );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH1 )
    {
        local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R10  ] = ( BBH_PERIPHERAL_ETH1_RX << 16 );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH2 )
    {
        local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R10  ] = ( BBH_PERIPHERAL_ETH2_RX << 16 );
    }
    else if ( g_wan_physical_port == BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH3 )
    {
        local_register[ WAN_TO_WAN_THREAD_NUMBER ][ CS_R10  ] = ( BBH_PERIPHERAL_ETH3_RX << 16 );
    }

#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#else
    memcpyl_context ( sram_fast_context_ptr, local_register, sizeof ( RUNNER_CNTXT_MAIN ) );
#endif


    /********** Pico Runner A **********/

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_0_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_pico_context_ptr, sizeof ( RUNNER_CNTXT_PICO ) );

    /* CPU-TX: thread 32 */
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, cpu_tx_wakeup_request) << 16;
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R9  ] = CPU_TX_PICO_QUEUE_ADDRESS;

    /* Timer scheduler: thread 33 */
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R14 ] = 0;
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R20 ] = DS_RATE_LIMITER_TABLE_ADDRESS;
    local_register[ TIMER_SCHEDULER_PICO_A_THREAD_NUMBER - 32 ][ CS_R21 ] = RATE_LIMITER_REMAINDER_TABLE_ADDRESS;

#if 0
    /* GSO : thread 34 */
    local_register[ GSO_PICO_THREAD_NUMBER ][ CS_R16 ] = ADDRESS_OF(runner_a, gso_wakeup_request) << 16;
    local_register[ GSO_PICO_THREAD_NUMBER ][ CS_R9 ] = GSO_PICO_QUEUE_ADDRESS;
#endif

#ifndef G9991
    /* WLAN Multicast: thread 35 */
    local_register[ WLAN_MCAST_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, wlan_mcast_wakeup_request) << 16;
    local_register[ WLAN_MCAST_THREAD_NUMBER - 32 ][ CS_R8 ] = ( CPU_RX_FAST_PD_INGRESS_QUEUE_ADDRESS << 16 ) | WLAN_MCAST_INGRESS_QUEUE_ADDRESS;
    local_register[ WLAN_MCAST_THREAD_NUMBER - 32 ][ CS_R9 ] = INGRESS_HANDLER_BUFFER_ADDRESS;

    /* Local switching LAN enqueue: thread 36 */
    local_register[ LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ( ADDRESS_OF(runner_c, lan_enqueue_runner_B_wakeup_request) << 16 ) | ADDRESS_OF(runner_c, lan_enqueue_runner_B_wakeup_request);
    local_register[ LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS;

    /* Downstream LAN enqueue: thread 37 */
    local_register[ DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, lan_enqueue_ds_wakeup_request) << 16;
    local_register[ DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS << 16;

    /* Downstream multicast LAN enqueue: thread 38 */
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ( ADDRESS_OF(runner_c, lan_enqueue_runner_A_wakeup_request) << 16 ) | ADDRESS_OF(runner_c, lan_enqueue_runner_A_wakeup_request);
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS;

    /* Local switching multicast LAN enqueue: thread 39 */
    local_register[ LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ( ADDRESS_OF(runner_c, lan_enqueue_runner_B_wakeup_request) << 16 ) | ADDRESS_OF(runner_c, lan_enqueue_runner_B_wakeup_request);
    local_register[ LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS;

    /* Downstream Service LAN enqueue: thread 40 */
    local_register[ DOWNSTREAM_LAN_SERVICE_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, service_queue_enqueue_wakeup_request) << 16;
    local_register[ DOWNSTREAM_LAN_SERVICE_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_LAN_SERVICE_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS << 16 | DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS;

    /* ETH-TX Inter LAN scheduling: thread 42 */
    local_register[ ETH_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request) << 16 | (ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request) );
    local_register[ ETH_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = BBH_PERIPHERAL_ETH0_TX << 16;
    local_register[ ETH_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH0-TX: thread 43 */
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_wakeup_request);
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ETH_TX_MAC_TABLE_ADDRESS + 1 * sizeof (RDD_ETH_TX_MAC_DESCRIPTOR_DTS) ) << 16;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( ( ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS + 1 * LILAC_RDD_EMAC_NUMBER_OF_QUEUES * sizeof ( RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS ) ) << 16 ) | BBH_PERIPHERAL_ETH0_TX;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH0_TX << 16 ) | BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH1-TX: thread 44 */
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_wakeup_request);
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ETH_TX_MAC_TABLE_ADDRESS + 2 * sizeof (RDD_ETH_TX_MAC_DESCRIPTOR_DTS) ) << 16;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( ( ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS + 2 * LILAC_RDD_EMAC_NUMBER_OF_QUEUES * sizeof ( RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS ) ) << 16) | BBH_PERIPHERAL_ETH1_TX;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH1_TX << 16 ) | BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH2-TX: thread 45 */
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_wakeup_request);
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ETH_TX_MAC_TABLE_ADDRESS + 3 * sizeof (RDD_ETH_TX_MAC_DESCRIPTOR_DTS) ) << 16;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( ( ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS + 3 * LILAC_RDD_EMAC_NUMBER_OF_QUEUES * sizeof ( RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS ) ) << 16) | BBH_PERIPHERAL_ETH2_TX;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH2_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH3-TX: thread 46 */
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_wakeup_request);
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ETH_TX_MAC_TABLE_ADDRESS + 4 * sizeof (RDD_ETH_TX_MAC_DESCRIPTOR_DTS) ) << 16;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( ( ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS + 4 * LILAC_RDD_EMAC_NUMBER_OF_QUEUES * sizeof ( RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS ) ) << 16) | BBH_PERIPHERAL_ETH3_TX;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH3_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH4-TX: thread 47 */
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_wakeup_request);
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R9  ] = ( ETH_TX_MAC_TABLE_ADDRESS + 5 * sizeof (RDD_ETH_TX_MAC_DESCRIPTOR_DTS) ) << 16;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( ( ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS + 5 * LILAC_RDD_EMAC_NUMBER_OF_QUEUES * sizeof ( RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS ) ) << 16) | BBH_PERIPHERAL_ETH4_TX;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH4_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;
#else
    /* Downstream FTTDP enqueue: thread 40 */
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R16 ] = ( ADDRESS_OF(runner_c, lan_enqueue_runner_A_wakeup_request) << 16 ) | ADDRESS_OF(runner_c, lan_enqueue_runner_A_wakeup_request);
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER - 32 ][ CS_R10 ] = DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS;

    /* ETH0-TX: thread 43 */
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request);
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R8  ] = G9991_VIRTUAL_PORT_STATUS_PER_EMAC_ADDRESS << 16 | BBH_PERIPHERAL_ETH0_TX;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R10 ] = BL_LILAC_RDD_EMAC_ID_0;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH0_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH0_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH1-TX: thread 44 */
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request);
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R8  ] = ( G9991_VIRTUAL_PORT_STATUS_PER_EMAC_ADDRESS + 4 ) << 16 | BBH_PERIPHERAL_ETH1_TX;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R10 ] = BL_LILAC_RDD_EMAC_ID_1;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH1_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH1_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH2-TX: thread 45 */
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request);
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R8  ] = ( G9991_VIRTUAL_PORT_STATUS_PER_EMAC_ADDRESS + 8  ) << 16 | BBH_PERIPHERAL_ETH2_TX;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R10 ] = BL_LILAC_RDD_EMAC_ID_2;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH2_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH2_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH3-TX: thread 46 */
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request);
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R8  ] = ( G9991_VIRTUAL_PORT_STATUS_PER_EMAC_ADDRESS + 12 ) << 16 | BBH_PERIPHERAL_ETH3_TX;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R10 ] = BL_LILAC_RDD_EMAC_ID_3;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH3_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH3_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;

    /* ETH4-TX: thread 47 */
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request) << 16 | ADDRESS_OF(runner_c, eth_tx_scheduling_wakeup_request);
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R8  ] = ( G9991_VIRTUAL_PORT_STATUS_PER_EMAC_ADDRESS + 16 ) << 16 | BBH_PERIPHERAL_ETH4_TX;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R10 ] = BL_LILAC_RDD_EMAC_ID_4;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R12 ] = ( BBH_PERIPHERAL_ETH4_TX << 16 )| BBTX_EEE_MODE_CONFIG_MESSAGE;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R13 ] = DS_ONE_BUFFER_ADDRESS;
    local_register[ ETH4_TX_THREAD_NUMBER - 32 ][ CS_R14 ] = DS_NULL_BUFFER_ADDRESS;
#endif

#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
#else
    memcpyl_context ( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
#endif


    /********** Pico Runner B **********/

    sram_pico_context_ptr = ( RUNNER_CNTXT_PICO * )DEVICE_ADDRESS( RUNNER_CNTXT_PICO_1_OFFSET );

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32( local_register, sram_pico_context_ptr, sizeof ( RUNNER_CNTXT_PICO ) );

    /* CPU TX pico: thread 32 */
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, cpu_tx_wakeup_request) << 16;
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R8  ] = ( CPU_TX_EMAC_LOOPBACK_QUEUE_ADDRESS << 16 ) | CPU_TX_PICO_QUEUE_ADDRESS;
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R10 ] = CPU_TX_US_FLOODING_QUEUE_ADDRESS;
    local_register[ CPU_TX_PICO_THREAD_NUMBER - 32 ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;

    /* Lan direct to CPU: thread 33 */
    switch(lan_direct_to_cpu_port)
    {
        case BL_LILAC_RDD_LAN0_BRIDGE_PORT:
            rx_descriptors_address = ETH0_RX_DESCRIPTORS_ADDRESS;
            bbh_peripheral_rx = BBH_PERIPHERAL_ETH0_RX;
            break;
        case BL_LILAC_RDD_LAN1_BRIDGE_PORT:
            rx_descriptors_address = ETH1_RX_DESCRIPTORS_ADDRESS;
            bbh_peripheral_rx = BBH_PERIPHERAL_ETH1_RX;
            break;
        case BL_LILAC_RDD_LAN2_BRIDGE_PORT:
            rx_descriptors_address = ETH2_RX_DESCRIPTORS_ADDRESS;
            bbh_peripheral_rx = BBH_PERIPHERAL_ETH2_RX;
            break;
        case BL_LILAC_RDD_LAN3_BRIDGE_PORT:
            rx_descriptors_address = ETH3_RX_DESCRIPTORS_ADDRESS;
            bbh_peripheral_rx = BBH_PERIPHERAL_ETH3_RX;
            break;
        case BL_LILAC_RDD_LAN4_BRIDGE_PORT:
            rx_descriptors_address = ETH4_RX_DESCRIPTORS_ADDRESS;
            bbh_peripheral_rx = BBH_PERIPHERAL_ETH4_RX;
            break;
        default:
            lan_direct_to_cpu_en = 0;
            break;
    }

    if( lan_direct_to_cpu_en )
    {
        local_register[ LAN_DIRECT_TO_CPU_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_direct_to_cpu_wakeup_request) << 16;
        local_register[ LAN_DIRECT_TO_CPU_THREAD_NUMBER - 32 ][ CS_R8  ] = rx_descriptors_address | ( 1 << LAN_FILTERS_LAN_TYPE_DIRECT_TO_CPU_BIT_OFFSET );;
        local_register[ LAN_DIRECT_TO_CPU_THREAD_NUMBER - 32 ][ CS_R12 ] = lan_direct_to_cpu_port | ( bbh_peripheral_rx << 16 );
    }
    
    /* Timer scheduler: thread 34 */
    local_register[ TIMER_SCHEDULER_PICO_B_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, timer_scheduler_set) << 16;
    local_register[ TIMER_SCHEDULER_PICO_B_THREAD_NUMBER - 32 ][ CS_R20 ] = US_RATE_LIMITER_TABLE_ADDRESS;

    /* CPU-RX interrupt coalescing timer: thread 35 */
    local_register[ CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, cpu_rx_int_coalesce_timer_1st_wakeup_request) << 16;

#ifdef CONFIG_DHD_RUNNER
    /* DHD RX Complete: thread 36 */
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, dhd_rx_complete_wakeup_request) << 16;
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R10 ] = 0; /* RX post packet counter used for doorbell */
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;
    local_register[ DHD_RX_THREAD_NUMBER - 32 ][ CS_R15 ] = DHD_RADIO_OFFSET_COMMON_B(0) << 16; 

    /* DHD1 RX Complete: thread 37 */
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, dhd_rx_complete_wakeup_request) << 16;
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R10 ] = 0; /* RX post packet counter used for doorbell */
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;
    local_register[ DHD1_RX_THREAD_NUMBER - 32 ][ CS_R15 ] = DHD_RADIO_OFFSET_COMMON_B(1) << 16; 

    /* DHD2 RX Complete: thread 38 */
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, dhd_rx_complete_wakeup_request) << 16;
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R9  ] = INGRESS_HANDLER_BUFFER_ADDRESS;
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R10 ] = 0; /* RX post packet counter used for doorbell */
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R11 ] = ( BBH_PERIPHERAL_IH << 16 ) | LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS;
    local_register[ DHD2_RX_THREAD_NUMBER - 32 ][ CS_R15 ] = DHD_RADIO_OFFSET_COMMON_B(2) << 16; 
#endif

#ifndef G9991
    /* Flow cache master: thread 39 */
    local_register[ UPSTREAM_FLOW_CACHE_MASTER_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, flow_cache_master_wakeup_request) << 16;
    local_register[ UPSTREAM_FLOW_CACHE_MASTER_THREAD_NUMBER - 32 ][ CS_R9  ] = ( US_ROUTER_INGRESS_QUEUE_ADDRESS << 16 ) | ADDRESS_OF(runner_d, flow_cache_master_wakeup_request);
    local_register[ UPSTREAM_FLOW_CACHE_MASTER_THREAD_NUMBER - 32 ][ CS_R10 ] = US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS;
#endif
    
    /* LAN-0 Filters and Classification : thread 40 */
    local_register[ LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = g_broadcom_switch_mode ? ( ADDRESS_OF(runner_d, lan_broadcom_switch_wakeup_request) << 16 ) : ( ADDRESS_OF(runner_d, lan_normal_wakeup_request) << 16 );
    local_register[ LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = g_broadcom_switch_mode ? LAN0_INGRESS_FIFO_ADDRESS : ETH0_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( LILAC_RDD_HASH_RESULT_SLOT0 << 16 ) | ( LILAC_RDD_HASH_RESULT_SLOT0_IO_ADDRESS << 24 );
    local_register[ LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R12 ] = BL_LILAC_RDD_LAN0_BRIDGE_PORT | ( BBH_PERIPHERAL_ETH0_RX << 16 );
    local_register[ LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = LILAC_RDD_CAM_RESULT_SLOT2 | ( LILAC_RDD_CAM_RESULT_SLOT2_IO_ADDRESS << 16 );
    local_register[ LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R15 ] = ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT0 << 5 ) | LILAC_RDD_DMA_LOOKUP_4_STEPS | ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT0_IO_ADDRESS << 16 );

    /* LAN-1 Filters and Classification : thread 41 */
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = g_broadcom_switch_mode ? ( ADDRESS_OF(runner_d, lan_broadcom_switch_wakeup_request) << 16 ) : ( ADDRESS_OF(runner_d, lan_normal_wakeup_request) << 16 );
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = g_broadcom_switch_mode ? LAN1_INGRESS_FIFO_ADDRESS : ETH1_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( LILAC_RDD_HASH_RESULT_SLOT1 << 16 ) | ( LILAC_RDD_HASH_RESULT_SLOT1_IO_ADDRESS << 24 );
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R12 ] = BL_LILAC_RDD_LAN1_BRIDGE_PORT | ( BBH_PERIPHERAL_ETH1_RX << 16 );
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = LILAC_RDD_CAM_RESULT_SLOT3 | ( LILAC_RDD_CAM_RESULT_SLOT3_IO_ADDRESS << 16 );
    local_register[ LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R15 ] = ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT1 << 5 ) | LILAC_RDD_DMA_LOOKUP_4_STEPS | ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT1_IO_ADDRESS << 16 );

    /* LAN-2 Filters and Classification : thread 42 */
    local_register[ LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = g_broadcom_switch_mode ? ( ADDRESS_OF(runner_d, lan_broadcom_switch_wakeup_request) << 16 ) : ( ADDRESS_OF(runner_d, lan_normal_wakeup_request) << 16 );
    local_register[ LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = g_broadcom_switch_mode ? LAN2_INGRESS_FIFO_ADDRESS : ETH2_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( LILAC_RDD_HASH_RESULT_SLOT2 << 16 ) | ( LILAC_RDD_HASH_RESULT_SLOT2_IO_ADDRESS << 24 );
    local_register[ LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R12 ] = BL_LILAC_RDD_LAN2_BRIDGE_PORT | ( BBH_PERIPHERAL_ETH2_RX << 16 );
    local_register[ LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = LILAC_RDD_CAM_RESULT_SLOT4 | ( LILAC_RDD_CAM_RESULT_SLOT4_IO_ADDRESS << 16 );
    local_register[ LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R15 ] = ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT2 << 5 ) | LILAC_RDD_DMA_LOOKUP_4_STEPS | ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT2_IO_ADDRESS << 16 );

    /* LAN-3 Filters and Classification : thread 43 */
    local_register[ LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = g_broadcom_switch_mode ? ( ADDRESS_OF(runner_d, lan_broadcom_switch_wakeup_request) << 16 ) : ( ADDRESS_OF(runner_d, lan_normal_wakeup_request) << 16 );
    local_register[ LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = g_broadcom_switch_mode ? LAN3_INGRESS_FIFO_ADDRESS : ETH3_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( LILAC_RDD_HASH_RESULT_SLOT3 << 16 ) | ( LILAC_RDD_HASH_RESULT_SLOT3_IO_ADDRESS << 24 );
    local_register[ LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R12 ] = BL_LILAC_RDD_LAN3_BRIDGE_PORT | ( BBH_PERIPHERAL_ETH3_RX << 16 );
    local_register[ LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = LILAC_RDD_CAM_RESULT_SLOT5 | ( LILAC_RDD_CAM_RESULT_SLOT5_IO_ADDRESS << 16 );
    local_register[ LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R15 ] = ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT3 << 5 ) | LILAC_RDD_DMA_LOOKUP_4_STEPS | ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT3_IO_ADDRESS << 16 );

    /* LAN-4 Filters and Classification : thread 44 */
    local_register[ LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = g_broadcom_switch_mode ? ( ADDRESS_OF(runner_d, lan_broadcom_switch_wakeup_request) << 16 ) : ( ADDRESS_OF(runner_d, lan_normal_wakeup_request) << 16 );
    local_register[ LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = g_broadcom_switch_mode ? LAN4_INGRESS_FIFO_ADDRESS : ETH4_RX_DESCRIPTORS_ADDRESS;
    local_register[ LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( LILAC_RDD_HASH_RESULT_SLOT4 << 16 ) | ( LILAC_RDD_HASH_RESULT_SLOT4_IO_ADDRESS << 24 );
    local_register[ LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R12 ] = BL_LILAC_RDD_LAN4_BRIDGE_PORT | ( BBH_PERIPHERAL_ETH4_RX << 16 );
    local_register[ LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = LILAC_RDD_CAM_RESULT_SLOT6 | ( LILAC_RDD_CAM_RESULT_SLOT6_IO_ADDRESS << 16 );
    local_register[ LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R15 ] = ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT4 << 5 ) | LILAC_RDD_DMA_LOOKUP_4_STEPS | ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT4_IO_ADDRESS << 16 );

    /* CPU upstream Filters and Classification : thread 45 */
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, lan_cpu_wakeup_request) << 16;
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R8  ] = US_CPU_TX_BBH_DESCRIPTORS_ADDRESS | ( 1 << LAN_FILTERS_LAN_TYPE_CPU_BIT_OFFSET );
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R10 ] = INGRESS_HANDLER_BUFFER_ADDRESS | ( LILAC_RDD_HASH_RESULT_SLOT5 << 16 ) | ( LILAC_RDD_HASH_RESULT_SLOT5_IO_ADDRESS << 24 );
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R12 ] = BL_LILAC_RDD_PCI_BRIDGE_PORT;
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R13 ] = LILAC_RDD_CAM_RESULT_SLOT7 | ( LILAC_RDD_CAM_RESULT_SLOT7_IO_ADDRESS << 16 );
    local_register[ CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER - 32 ][ CS_R15 ] = ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT5 << 5 ) | LILAC_RDD_DMA_LOOKUP_4_STEPS | ( LILAC_RDD_DMA_LOOKUP_RESULT_SLOT5_IO_ADDRESS << 16 );

    /* LAN Dispatch : thread 46 */
    if ( g_broadcom_switch_physical_port == BL_LILAC_RDD_LAN1_BRIDGE_PORT )
    {
        local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] = ETH1_RX_DESCRIPTORS_ADDRESS;
        local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R10 ] = BBH_PERIPHERAL_ETH1_RX;
    }
    else if ( g_broadcom_switch_physical_port == BL_LILAC_RDD_LAN4_BRIDGE_PORT )
    {
        local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R8  ] = ETH4_RX_DESCRIPTORS_ADDRESS;
        local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R10 ] = BBH_PERIPHERAL_ETH4_RX;
    }

    local_register[ LAN_DISPATCH_THREAD_NUMBER - 32 ][ CS_R11 ] = BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_ADDRESS;

    /* Upstream flooding: thread 47 */
    local_register[ UPSTREAM_FLOODING_THREAD_NUMBER - 32 ][ CS_R16 ] = ADDRESS_OF(runner_d, upstream_flooding_wakeup_request) << 16;
    local_register[ UPSTREAM_FLOODING_THREAD_NUMBER - 32 ][ CS_R8  ] = UPSTREAM_FLOODING_INGRESS_QUEUE_ADDRESS;
    local_register[ UPSTREAM_FLOODING_THREAD_NUMBER - 32 ][ CS_R10  ] = INGRESS_HANDLER_BUFFER_ADDRESS;


#if defined (FIRMWARE_INIT) || defined (FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
#else
    memcpyl_context ( sram_pico_context_ptr, local_register, sizeof ( RUNNER_CNTXT_PICO ) );
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
    uint8_t   *skb_enqueued_indexes_fifo_counters_ptr;

#if !defined(FIRMWARE_INIT)
    uint32_t                        phy_addr;

    /* allocate skb pointer array reference (used only by SW) */
#ifndef RDD_BASIC
    g_cpu_tx_skb_pointers_reference_array = ( uint32_t * )bdmf_alloc( sizeof( uint32_t ) *
        LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT );
#ifdef CONFIG_DHD_RUNNER
    g_dhd_tx_cpu_usage_reference_array = ( uint8_t * )bdmf_alloc( LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT );
#endif
#else
    g_cpu_tx_skb_pointers_reference_array = ( uint32_t * )KMALLOC( sizeof( uint32_t ) *
        LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT, 0 );
#ifdef CONFIG_DHD_RUNNER
    g_dhd_tx_cpu_usage_reference_array = ( uint8_t * )KMALLOC( LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT, 0 );
#endif
#endif

    /* allocate data pointer array pointer (used both by SW & FW) */
	g_cpu_tx_data_pointers_reference_array = ( uint32_t * )alignedAlloc(sizeof( uint32_t ) *
        LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT, &phy_addr );

    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_SKB_DATA_POINTERS_TABLE_ADDRESS );
    MWRITE_32( ddr_address_ptr, ( uint32_t * )VIRT_TO_PHYS( g_cpu_tx_data_pointers_reference_array ) );

    /* allocate Free Indexes table (used both by SW & FW) */
    g_free_skb_indexes_fifo_table = ( uint16_t * )alignedAlloc( sizeof( uint16_t ) * LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT, &phy_addr );
	  
	  /* allocate rdd Free Indexes table (used by SW only) */
	  g_rdd_free_skb_indexes_fifo_table = ( uint16_t * )alignedAlloc( sizeof( uint16_t ) * LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT, &phy_addr );

    g_free_skb_indexes_fifo_table_physical_address = ( uint16_t * )VIRT_TO_PHYS( g_free_skb_indexes_fifo_table );
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_FREE_SKB_INDEXES_FIFO_TABLE_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_free_skb_indexes_fifo_table_physical_address );

    g_free_skb_indexes_fifo_table_physical_address_last_idx = g_free_skb_indexes_fifo_table_physical_address;
    // RDD_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE should be equeal to RDD_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE
    g_free_skb_indexes_fifo_table_physical_address_last_idx += (LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT - RDD_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE);
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_FREE_SKB_INDEXES_FIFO_TABLE_LAST_ENTRY_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_free_skb_indexes_fifo_table_physical_address_last_idx );

    /* Fill free indexes FIFO */
    for ( i = 0; i < LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT ; i++ )
    {        
        /* put invalid released index for debug */
        g_free_skb_indexes_fifo_table[ i ] = LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT + 1;
        g_rdd_free_skb_indexes_fifo_table[ i ] = i;
        g_cpu_tx_skb_pointers_reference_array[ i ] = ( uint32_t )( -1 );
        g_cpu_tx_data_pointers_reference_array[ i ] = ( uint32_t )( -1 );
#ifdef CONFIG_DHD_RUNNER
        g_dhd_tx_cpu_usage_reference_array[ i ] = 0;
#endif
    }
#else
    /* allocate data pointer array pointer (used both by SW & FW) */
    g_cpu_tx_data_pointers_reference_array = (uint32_t *)SIMULATOR_DDR_SKB_DATA_POINTERS_OFFSET;

    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_SKB_DATA_POINTERS_TABLE_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_cpu_tx_data_pointers_reference_array );

    /* allocate Free Indexes table (used both by SW & FW) */
    g_free_skb_indexes_fifo_table = ( uint16_t * )SIMULATOR_DDR_SKB_FREE_INDEXES_OFFSET;

    g_free_skb_indexes_fifo_table_physical_address = g_free_skb_indexes_fifo_table;
    ddr_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DDR_ADDRESS_FOR_FREE_SKB_INDEXES_FIFO_TABLE_ADDRESS );
    MWRITE_32( ddr_address_ptr, g_free_skb_indexes_fifo_table_physical_address );

    g_free_skb_indexes_fifo_table_physical_address_last_idx = g_free_skb_indexes_fifo_table_physical_address;
    g_free_skb_indexes_fifo_table_physical_address_last_idx += ( LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT - RDD_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE );
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

    /* update rdd free indexes counter */
    g_cpu_tx_skb_free_indexes_counter = LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT;

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
        /* Initialize pointers to WAN enqueued indexes FIFO */    
    skb_enqueued_indexes_fifo_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    skb_enqueued_indexes_fifo = GPON_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS;

    for ( i = 0; i < ( RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE ); i++ )
    {
        MWRITE_16( skb_enqueued_indexes_fifo_ptr, skb_enqueued_indexes_fifo );

        skb_enqueued_indexes_fifo_ptr++;
        skb_enqueued_indexes_fifo += 32;
    }

    skb_enqueued_indexes_fifo_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    skb_enqueued_indexes_fifo = GPON_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS;

    for ( i = 0; i < ( RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE ); i++ )
    {
        MWRITE_16( skb_enqueued_indexes_fifo_ptr, skb_enqueued_indexes_fifo );

        skb_enqueued_indexes_fifo_ptr++;
        skb_enqueued_indexes_fifo += 32;
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

    return ( BL_LILAC_RDD_OK );
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_port_to_bbh_destination_table_initialize ( void )
{
    uint8_t*     bbh_destination_table_table_ptr;

    bbh_destination_table_table_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_ADDRESS  - sizeof ( RUNNER_COMMON ) );
    MWRITE_8( bbh_destination_table_table_ptr + BL_LILAC_RDD_WAN_BRIDGE_PORT,  BBH_PERIPHERAL_GPON_RX);
    MWRITE_8( bbh_destination_table_table_ptr + BL_LILAC_RDD_LAN0_BRIDGE_PORT,  BBH_PERIPHERAL_ETH0_RX);
    MWRITE_8( bbh_destination_table_table_ptr + BL_LILAC_RDD_LAN1_BRIDGE_PORT,  BBH_PERIPHERAL_ETH1_RX);
    MWRITE_8( bbh_destination_table_table_ptr + BL_LILAC_RDD_LAN2_BRIDGE_PORT,  BBH_PERIPHERAL_ETH2_RX);
    MWRITE_8( bbh_destination_table_table_ptr + BL_LILAC_RDD_LAN3_BRIDGE_PORT,  BBH_PERIPHERAL_ETH3_RX);
    MWRITE_8( bbh_destination_table_table_ptr + BL_LILAC_RDD_LAN4_BRIDGE_PORT,  BBH_PERIPHERAL_ETH4_RX);
    MWRITE_8( bbh_destination_table_table_ptr + BL_LILAC_RDD_WAN_ROUTER_PORT,  BBH_PERIPHERAL_GPON_RX);

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_classification_table_initialize ( void )
{
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS     *ds_rule_cfg_table_ptr;
	RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS     *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS        *rule_cfg_entry_ptr;
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS  *ds_ingress_classification_counters_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS  *us_ingress_classification_counters_table_ptr;
#endif
    uint8_t                                           *rule_cfg_descriptor_ptr;
    RUNNER_REGS_CFG_LKUP0_CFG                         hash_lkup_0_cfg_register;
#ifdef UNDEF
    RUNNER_REGS_CFG_LKUP0_CAM_CFG                     hash_lkup_0_cam_cfg_register;
#endif
    RUNNER_REGS_CFG_LKUP_GLBL_MASK0_H                 hash_lkup_0_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK0_L                 hash_lkup_0_global_mask_low_register;
    RUNNER_REGS_CFG_LKUP3_CFG                         hash_lkup_3_cfg_register;
    RUNNER_REGS_CFG_LKUP3_CAM_CFG                     hash_lkup_3_cam_cfg_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK3_H                 hash_lkup_3_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK3_L                 hash_lkup_3_global_mask_low_register;
    uint32_t                                          rule_cfg_id;

    for ( rule_cfg_id = 0; rule_cfg_id < 16; rule_cfg_id++ )
    {
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].valid = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].priority = -1;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].rule_type = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].next_group_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].first_rule_cfg_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_ds ].first_gen_filter_rule_cfg_id = 16;

        ds_rule_cfg_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR();

        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ rule_cfg_id ] );

        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( 16, rule_cfg_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( 16, rule_cfg_entry_ptr );

        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].valid = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].priority = -1;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].rule_type = 0;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].next_group_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].first_rule_cfg_id = 16;
        g_ingress_classification_rule_cfg_table[ rdpa_dir_us ].first_gen_filter_rule_cfg_id = 16;

        us_rule_cfg_table_ptr = RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR();

        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ rule_cfg_id ] );

        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( 16, rule_cfg_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( 16, rule_cfg_entry_ptr );
    }

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
    hash_lkup_3_cfg_register.max_hop = BL_LILAC_RDD_MAC_TABLE_MAX_HOP_32;
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
    hash_lkup_0_cfg_register.max_hop = BL_LILAC_RDD_MAC_TABLE_MAX_HOP_64;
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
    hash_lkup_3_cfg_register.max_hop = BL_LILAC_RDD_MAC_TABLE_MAX_HOP_32;
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
    hash_lkup_0_cfg_register.max_hop = BL_LILAC_RDD_MAC_TABLE_MAX_HOP_64;
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

#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    ds_ingress_classification_counters_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR();
    us_ingress_classification_counters_table_ptr = RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR();

    MEMSET ( ds_ingress_classification_counters_table_ptr, 0, sizeof( RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS ) );
    MEMSET ( us_ingress_classification_counters_table_ptr, 0, sizeof( RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS ) );
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_eth_tx_initialize ( void )
{
    RDD_ETH_TX_MAC_TABLE_DTS                               *eth_tx_mac_table_ptr;
    RDD_ETH_TX_MAC_DESCRIPTOR_DTS                          *eth_tx_mac_descriptor_ptr;
#ifndef G9991
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS                        *eth_tx_queue_descriptor_ptr;
    RDD_PACKET_DESCRIPTOR_DTS                              *packet_descriptor_ptr;
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS  *free_packet_descriptors_pool_descriptor_ptr;
#endif
    RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS                   *eth_tx_queues_pointers_table_ptr;
    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS                    *eth_tx_queue_pointers_entry_ptr;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS                  *bridge_cfg_register;
    uint16_t                                               eth_tx_queue_address;
    uint16_t                                               mac_descriptor_address;
    uint32_t                                               tx_queue;
#ifndef G9991
    uint16_t                                               packet_descriptors_counter;
    uint16_t                                               packet_descriptor_address;
    uint32_t                                               emac_id;
    uint32_t                                               queue_number;
    uint16_t                                               next_packet_descriptor_address;
#else
    uint32_t                                               virtual_port;
#endif

    eth_tx_mac_table_ptr = ( RDD_ETH_TX_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_MAC_TABLE_ADDRESS );

    eth_tx_queues_pointers_table_ptr = ( RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS );

#ifndef G9991
    free_packet_descriptors_pool_descriptor_ptr = ( RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS );

    packet_descriptors_counter = 0;

    for ( emac_id = BL_LILAC_RDD_EMAC_ID_START; emac_id <= BL_LILAC_RDD_EMAC_ID_4; emac_id++ )
    {
        eth_tx_mac_descriptor_ptr = &( eth_tx_mac_table_ptr->entry[ emac_id ] );

        if ( emac_id != BL_LILAC_RDD_EMAC_ID_PCI )
        {
            RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE ( ( ETH0_TX_THREAD_NUMBER + ( emac_id - BL_LILAC_RDD_EMAC_ID_0 ) ), eth_tx_mac_descriptor_ptr );
            RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_WRITE ( ETH_TX_MAC_TABLE_ADDRESS + emac_id * sizeof ( RDD_ETH_TX_MAC_DESCRIPTOR_DTS ) + LILAC_RDD_EMAC_EGRESS_COUNTER_OFFSET, eth_tx_mac_descriptor_ptr );
            RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE ( ( LILAC_RDD_GPIO_IO_ADDRESS + ( emac_id - 1 ) ), eth_tx_mac_descriptor_ptr );
        }
        else
        {
            RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE ( PCI_TX_THREAD_NUMBER, eth_tx_mac_descriptor_ptr );
        }
        RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_WRITE ( RDD_RATE_LIMITER_IDLE, eth_tx_mac_descriptor_ptr );

        queue_number = ( ( emac_id == BL_LILAC_RDD_EMAC_ID_PCI )? LILAC_RDD_PCI_TX_NUMBER_OF_FIFOS : LILAC_RDD_EMAC_NUMBER_OF_QUEUES );

        for ( tx_queue = 0; tx_queue < queue_number ; tx_queue++ )
        {
            eth_tx_queue_address = ETH_TX_QUEUES_TABLE_ADDRESS + ( emac_id * LILAC_RDD_EMAC_NUMBER_OF_QUEUES + tx_queue ) * sizeof ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS );

            mac_descriptor_address = ETH_TX_MAC_TABLE_ADDRESS + emac_id * sizeof ( RDD_ETH_TX_MAC_DESCRIPTOR_DTS );

            eth_tx_queue_pointers_entry_ptr = &( eth_tx_queues_pointers_table_ptr->entry[ emac_id * LILAC_RDD_EMAC_NUMBER_OF_QUEUES + tx_queue ] );

            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_ETH_MAC_POINTER_WRITE ( mac_descriptor_address, eth_tx_queue_pointers_entry_ptr );
            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_WRITE ( eth_tx_queue_address , eth_tx_queue_pointers_entry_ptr );

            eth_tx_queue_descriptor_ptr = ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + eth_tx_queue_address );

            RDD_ETH_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE ( 1 << tx_queue , eth_tx_queue_descriptor_ptr );
        }
    }

    for ( tx_queue = 0; tx_queue < 128; tx_queue++ )
    {
        eth_tx_queue_address = ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_ADDRESS + tx_queue * sizeof ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS );
        eth_tx_queue_descriptor_ptr = ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + eth_tx_queue_address );

        RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_READ ( packet_descriptor_address, free_packet_descriptors_pool_descriptor_ptr );

        packet_descriptor_ptr = ( RDD_PACKET_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + packet_descriptor_address );

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_READ ( next_packet_descriptor_address, packet_descriptor_ptr );
        RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_WRITE ( next_packet_descriptor_address, free_packet_descriptors_pool_descriptor_ptr );
        packet_descriptors_counter++;

        RDD_ETH_TX_QUEUE_DESCRIPTOR_HEAD_PTR_WRITE ( packet_descriptor_address, eth_tx_queue_descriptor_ptr );
        RDD_ETH_TX_QUEUE_DESCRIPTOR_TAIL_PTR_WRITE ( packet_descriptor_address, eth_tx_queue_descriptor_ptr );
    }

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_WRITE ( packet_descriptors_counter, free_packet_descriptors_pool_descriptor_ptr );

#else
    for ( virtual_port = BL_LILAC_RDD_EMAC_ID_0; virtual_port < BL_LILAC_RDD_EMAC_ID_COUNT; virtual_port++ )
    {
        eth_tx_mac_descriptor_ptr = &( eth_tx_mac_table_ptr->entry[ virtual_port ] );

        RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE ( ( ETH0_TX_THREAD_NUMBER + ( virtual_port - BL_LILAC_RDD_EMAC_ID_0 ) ), eth_tx_mac_descriptor_ptr );
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_WRITE ( ETH_TX_MAC_TABLE_ADDRESS + virtual_port * sizeof ( RDD_ETH_TX_MAC_DESCRIPTOR_DTS ) + LILAC_RDD_EMAC_EGRESS_COUNTER_OFFSET, eth_tx_mac_descriptor_ptr );

        RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_WRITE ( RDD_RATE_LIMITER_IDLE, eth_tx_mac_descriptor_ptr );

        for ( tx_queue = 0; tx_queue < LILAC_RDD_EMAC_NUMBER_OF_QUEUES; tx_queue++ )
        {
            eth_tx_queue_address = G9991_DDR_QUEUE_DESCRIPTORS_TABLE_ADDRESS + ( virtual_port * LILAC_RDD_EMAC_NUMBER_OF_QUEUES + tx_queue ) * sizeof ( RDD_DDR_QUEUE_DESCRIPTOR_DTS );

            mac_descriptor_address = ETH_TX_MAC_TABLE_ADDRESS + virtual_port * sizeof ( RDD_ETH_TX_MAC_DESCRIPTOR_DTS );

            eth_tx_queue_pointers_entry_ptr = &( eth_tx_queues_pointers_table_ptr->entry[ virtual_port * LILAC_RDD_EMAC_NUMBER_OF_QUEUES + tx_queue ] );

            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_ETH_MAC_POINTER_WRITE ( mac_descriptor_address, eth_tx_queue_pointers_entry_ptr );
            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_WRITE ( eth_tx_queue_address , eth_tx_queue_pointers_entry_ptr );
        }
    }
#endif
    /* Save broadcom switch mode and phisical port */
    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    RDD_BRIDGE_CONFIGURATION_REGISTER_BROADCOM_SWITCH_PORT_WRITE( ( g_broadcom_switch_mode << 3 ) | g_broadcom_switch_physical_port, bridge_cfg_register );

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
    exponent_table_ptr = RDD_RATE_CONTROLLER_EXPONENT_TABLE_PTR();

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 0 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT0, exponent_entry_ptr );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 1 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT1, exponent_entry_ptr );

    exponent_entry_ptr = &( exponent_table_ptr->entry[ 2 ] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_RATE_CONTROL_EXPONENT2, exponent_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}

static void f_rdd_wan_tx_remap_channels ( void )
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS          *wan_channels_0_7_table_ptr;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS      *wan_channel_0_7_descriptor_ptr;
    RDD_WAN_CHANNELS_8_39_TABLE_DTS         *wan_channels_8_39_table_ptr;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS     *wan_channel_8_39_descriptor_ptr;
    uint32_t                                wan_channel_id;
    uint32_t                                bbh_destination;
    
    wan_channels_0_7_table_ptr = ( RDD_WAN_CHANNELS_0_7_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_0_7_TABLE_ADDRESS );

    for ( wan_channel_id = RDD_WAN_CHANNEL_0; wan_channel_id <= RDD_WAN_CHANNEL_7; wan_channel_id++ )
    {
        wan_channel_0_7_descriptor_ptr = &( wan_channels_0_7_table_ptr->entry[ wan_channel_id ] );

        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_READ ( bbh_destination, wan_channel_0_7_descriptor_ptr );
 
        if ( bbh_destination )
        {
            RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE ( ( bbh_destination ^ ( EPON_WAN_CHANNEL_MAPPING  << 7 ) ), wan_channel_0_7_descriptor_ptr );
        }
    }

    wan_channels_8_39_table_ptr = ( RDD_WAN_CHANNELS_8_39_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_8_39_TABLE_ADDRESS );

    for ( wan_channel_id = RDD_WAN_CHANNEL_8; wan_channel_id <= RDD_WAN_CHANNEL_39; wan_channel_id++ )
    {
        wan_channel_8_39_descriptor_ptr = &( wan_channels_8_39_table_ptr->entry[ wan_channel_id - RDD_WAN_CHANNEL_8 ] );

        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BBH_DESTINATION_READ ( bbh_destination, wan_channel_8_39_descriptor_ptr );

        if ( bbh_destination )
        {
            RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BBH_DESTINATION_WRITE ( ( bbh_destination ^ ( EPON_WAN_CHANNEL_MAPPING << 7 ) ), wan_channel_8_39_descriptor_ptr );
        }
    }
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_inter_task_queues_initialize ( void )
{
    uint16_t  *lan_enqueue_ingress_queue_ptr;

    lan_enqueue_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( lan_enqueue_ingress_queue_ptr, LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS );

#ifndef G9991
    lan_enqueue_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( lan_enqueue_ingress_queue_ptr, LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS );
#endif

    lan_enqueue_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( lan_enqueue_ingress_queue_ptr, DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS );


    lan_enqueue_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( lan_enqueue_ingress_queue_ptr, DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS );

#ifdef G9991
    lan_enqueue_ingress_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_PTR_ADDRESS );

    MWRITE_16( lan_enqueue_ingress_queue_ptr, G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_ADDRESS );
#endif

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_broadcom_switch_mode_initialize ( void )
{
    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_DTS                      *ingress_fifo_ptr_table;
    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_DTS                      *ingress_fifo_ptr_entry;
    RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR_DTS  *broadcom_switch_mapping_table_address_ptr;

    broadcom_switch_mapping_table_address_ptr = ( RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR_ADDRESS - sizeof ( RUNNER_COMMON ) );

    MWRITE_16( broadcom_switch_mapping_table_address_ptr, BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_ADDRESS );

    ingress_fifo_ptr_table = ( RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_ADDRESS );

    ingress_fifo_ptr_entry = &( ingress_fifo_ptr_table->entry[ BL_LILAC_RDD_LAN0_BRIDGE_PORT - 1 ] );

    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_BROADCOM_SWITCH_TASK_WAKEUP_VALUE_WRITE ( LAN0_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE, ingress_fifo_ptr_entry );
    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_INGRESS_FIFO_PTR_WRITE ( LAN0_INGRESS_FIFO_ADDRESS, ingress_fifo_ptr_entry );

    ingress_fifo_ptr_entry = &( ingress_fifo_ptr_table->entry[ BL_LILAC_RDD_LAN1_BRIDGE_PORT - 1 ] );

    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_BROADCOM_SWITCH_TASK_WAKEUP_VALUE_WRITE ( LAN1_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE, ingress_fifo_ptr_entry );
    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_INGRESS_FIFO_PTR_WRITE ( LAN1_INGRESS_FIFO_ADDRESS, ingress_fifo_ptr_entry );

    ingress_fifo_ptr_entry = &( ingress_fifo_ptr_table->entry[ BL_LILAC_RDD_LAN2_BRIDGE_PORT - 1 ] );

    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_BROADCOM_SWITCH_TASK_WAKEUP_VALUE_WRITE ( LAN2_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE, ingress_fifo_ptr_entry );
    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_INGRESS_FIFO_PTR_WRITE ( LAN2_INGRESS_FIFO_ADDRESS, ingress_fifo_ptr_entry );

    ingress_fifo_ptr_entry = &( ingress_fifo_ptr_table->entry[ BL_LILAC_RDD_LAN3_BRIDGE_PORT - 1 ] );

    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_BROADCOM_SWITCH_TASK_WAKEUP_VALUE_WRITE ( LAN3_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE, ingress_fifo_ptr_entry );
    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_INGRESS_FIFO_PTR_WRITE ( LAN3_INGRESS_FIFO_ADDRESS, ingress_fifo_ptr_entry );

    ingress_fifo_ptr_entry = &( ingress_fifo_ptr_table->entry[ BL_LILAC_RDD_LAN4_BRIDGE_PORT - 1 ] );

    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_BROADCOM_SWITCH_TASK_WAKEUP_VALUE_WRITE ( LAN4_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE, ingress_fifo_ptr_entry );
    RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_INGRESS_FIFO_PTR_WRITE ( LAN4_INGRESS_FIFO_ADDRESS, ingress_fifo_ptr_entry );
    
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


static BL_LILAC_RDD_ERROR_DTE f_rdd_1588_initialize ( void )
{
    uint16_t  *ip_sync_1588_queue_ptr;

    ip_sync_1588_queue_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + IP_SYNC_1588_DESCRIPTOR_QUEUE_POINTER_ADDRESS );

    MWRITE_16( ip_sync_1588_queue_ptr, IP_SYNC_1588_DESCRIPTOR_QUEUE_ADDRESS );

    return ( BL_LILAC_RDD_OK );
}

#ifndef G9991
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

#else
BL_LILAC_RDD_ERROR_DTE f_rdd_G9991_initialize ( void )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS       *bridge_cfg_register;
    RDD_ETH_PHYSICAL_PORT_ACK_PENDING_DTS       *eth_ack_pending_array_ptr;
    RDD_G9991_VIRTUAL_PORT_STATUS_PER_EMAC_DTS  *virtual_port_status_per_emac_table_ptr;
    RDD_US_SID_CONTEXT_TABLE_DTS                *sid_context_table_ptr;
    RDD_US_SID_CONTEXT_ENTRY_DTS                *sid_context_ptr;
    uint32_t                                    emac_id;
    uint32_t                                    *virtual_port_rate_limiter_status_ptr;
    uint32_t                                    *dfc_vector_ptr;
    uint32_t                                    sid;
    
    /* Downstream */
    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    RDD_BRIDGE_CONFIGURATION_REGISTER_INTER_LAN_SCHEDULING_MODE_WRITE ( BL_LILAC_RDD_INTER_LAN_SCHEDULING_MODE_ROUND_ROBIN, bridge_cfg_register );

    eth_ack_pending_array_ptr = RDD_ETH_PHYSICAL_PORT_ACK_PENDING_PTR();
    virtual_port_status_per_emac_table_ptr = RDD_G9991_VIRTUAL_PORT_STATUS_PER_EMAC_PTR();

    for ( emac_id = BL_LILAC_RDD_EMAC_ID_0; emac_id <= BL_LILAC_RDD_EMAC_ID_4; emac_id++ )
    {
        MWRITE_8( &( eth_ack_pending_array_ptr->entry[ emac_id ] ), LILAC_RDD_TRUE );
        MWRITE_32( &( virtual_port_status_per_emac_table_ptr->entry[ emac_id ] ), 0 );
    }

    virtual_port_rate_limiter_status_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + G9991_VIRTUAL_PORT_RATE_LIMITER_STATUS_ADDRESS );
    MWRITE_32( virtual_port_rate_limiter_status_ptr, 0xFFFFFFFF );
    
    /* Upstream */
    /* resetting DFC vector - all SID's are x_on*/
    dfc_vector_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + G9991_DFC_VECTOR_ADDRESS );

    MWRITE_32( dfc_vector_ptr, 0xFFFFFFFF );

    sid_context_table_ptr = RDD_US_SID_CONTEXT_TABLE_PTR();

    /* resetting SID context state machines and counters */
    for ( sid = 0; sid < RDD_US_SID_CONTEXT_TABLE_SIZE; sid++ )
    {
        sid_context_ptr = &( sid_context_table_ptr->entry[ sid ] );

        RDD_US_SID_CONTEXT_ENTRY_STATE_MACHINE_WRITE ( US_SID_STATE_MACHINE_WAITING_SOF, sid_context_ptr );
        RDD_US_SID_CONTEXT_ENTRY_FRAGMENT_COUNT_WRITE ( 0 , sid_context_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}
#endif

static void f_rdd_dummy_lock ( bdmf_fastlock  *xo_int_lock )
{
    return;
}


static void f_rdd_dummy_unlock ( bdmf_fastlock  *xi_int_lock )
{
    return;
}

static void f_rdd_dummy_lock_irq ( bdmf_fastlock  *xo_int_lock, unsigned long *flags )
{
    return;
}


static void f_rdd_dummy_unlock_irq ( bdmf_fastlock  *xi_int_lock, unsigned long flags )
{
    return;
}


BL_LILAC_RDD_ERROR_DTE rdd_broadcom_switch_ports_mapping_table_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                        uint8_t                       xi_broadcom_switch_port )
{
    RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS  *broadcom_switch_mapping_table_ptr;

    broadcom_switch_mapping_table_ptr = ( RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_ADDRESS );

    if ( ( xi_bridge_port < BL_LILAC_RDD_LAN0_BRIDGE_PORT ) || ( xi_bridge_port > BL_LILAC_RDD_LAN4_BRIDGE_PORT ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE ( xi_bridge_port, broadcom_switch_mapping_table_ptr + xi_broadcom_switch_port );

    broadcom_switch_mapping_table_ptr = ( RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

    RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE ( xi_broadcom_switch_port, broadcom_switch_mapping_table_ptr + xi_bridge_port );

    return ( BL_LILAC_RDD_OK );
}

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   f_rdd_wan_tx_queue_ddr_offload_initialize                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Runner Initialization - initialize TX queues DDR offload enable          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_enable  - TX queues DDR offload enable                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
void f_rdd_wan_tx_queue_ddr_offload_initialize( bdmf_boolean  xi_enable )
{
    uint8_t     *rdd_wan_tx_queue_ddr_offload_config_ptr;

    g_us_ddr_queue_enable = xi_enable;
    rdd_wan_tx_queue_ddr_offload_config_ptr = (uint8_t *)DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_TX_QUEUES_DDR_OFFLOAD_ENABLE_ADDRESS;
    MWRITE_8( rdd_wan_tx_queue_ddr_offload_config_ptr, (uint8_t)xi_enable);
}

BL_LILAC_RDD_ERROR_DTE rdd_wan_mode_config ( rdd_wan_mode_t  xi_wan_mode)
{
    uint32_t                    *global_register_init_ptr;
    uint32_t                    global_register;
    uint16_t                    skb_enqueued_indexes_fifo, i;
    uint8_t                     skb_enqueued_indexes_fifo_size;
    uint16_t                    *skb_enqueued_indexes_fifo_ptr;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    BL_LILAC_RDD_ERROR_DTE      rdd_error = BL_LILAC_RDD_OK;

    /* Initialize pointers to WAN enqueued indexes FIFO */    
    skb_enqueued_indexes_fifo_size = ( xi_wan_mode == rdd_wan_epon )? 48 : 32;
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

    if ( xi_wan_mode == rdd_wan_epon )
        f_epon_tx_post_scheduling_ddr_queue_initialize ( EPON_QUEUE_MAX_SIZE );

    /* configure EPON mode in fast runner R7 */
    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS + 2 * sizeof ( uint32_t ) );
    
    MREAD_32( global_register_init_ptr, global_register );
    global_register &= ~( ( 1 << GLOBAL_CFG_EPON_MODE_BIT_OFFSET ) | ( 1 << DS_GLOBAL_CFG_AE_MODE_BIT_OFFSET ) );
    global_register |= ( ( xi_wan_mode == rdd_wan_epon ) << GLOBAL_CFG_EPON_MODE_BIT_OFFSET );
    global_register |= ( ( xi_wan_mode == rdd_wan_ae ) << DS_GLOBAL_CFG_AE_MODE_BIT_OFFSET );
    MWRITE_32( global_register_init_ptr, global_register );
#ifndef FIRMWARE_INIT
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTER_SET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 2, 0, 0, BL_LILAC_RDD_NO_WAIT );
    if ( rdd_error != BL_LILAC_RDD_OK )
        return rdd_error;
#endif
    /* configure EPON mode in pico runner R7 */
    global_register_init_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS + 7 * sizeof ( uint32_t ) );
    
    MREAD_32( global_register_init_ptr, global_register );
    global_register &= ~( 1 << GLOBAL_CFG_EPON_MODE_BIT_OFFSET );
    global_register |= ( ( xi_wan_mode == rdd_wan_epon ) << GLOBAL_CFG_EPON_MODE_BIT_OFFSET );
    MWRITE_32( global_register_init_ptr, global_register );
#ifndef FIRMWARE_INIT
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTER_SET, FAST_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, 7, 0, 0, BL_LILAC_RDD_NO_WAIT );
#endif

    if (( xi_wan_mode == rdd_wan_epon ) && ( g_wan_mapping != EPON_WAN_CHANNEL_MAPPING ))
    {
        f_rdd_wan_tx_remap_channels();
    }

    g_wan_mapping = ( xi_wan_mode == rdd_wan_epon )? EPON_WAN_CHANNEL_MAPPING : 0;
    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_CHANNEL_MAPPING_WRITE ( g_wan_mapping, bridge_cfg_register );
 
    return rdd_error;
}
#if !defined(RDD_BASIC)
#ifndef G9991
static void rdd_action_vector_set(uint16_t *action_ptrs_ptr, uint16_t *action_ptrs, uint8_t action_total_num)
{
    MWRITE_BLK_16(action_ptrs, action_ptrs_ptr, action_total_num * sizeof(RDD_TWO_BYTES_DTS));
}

static void rdd_actions_proj_init(void)
{
    uint16_t *actions_arr_ptr;
    uint16_t ds_main_actions_arr[] = {
        [0]  = ADDRESS_OF(runner_a, action_default),
        [1]  = ADDRESS_OF(runner_a, action_policer),
        [2]  = ADDRESS_OF(runner_a, action_ttl_decrement),
        [3]  = ADDRESS_OF(runner_a, action_default),
        [4]  = ADDRESS_OF(runner_a, action_dscp_remarking),
        [5]  = ADDRESS_OF(runner_a, action_nat),
        [6]  = ADDRESS_OF(runner_a, action_gre_remark),
        [7]  = ADDRESS_OF(runner_a, action_outer_pbits_remarking),
        [8]  = ADDRESS_OF(runner_a, action_inner_pbits_remarking),
        [9]  = ADDRESS_OF(runner_a, action_default),
        [10] = ADDRESS_OF(runner_a, action_pppoe),
        [11 ... 15] = ADDRESS_OF(runner_a, action_default),
        [16] = ADDRESS_OF(runner_a, flow_cache_update_header)
    };
    uint16_t us_main_actions_arr[] = {
        [0]  = ADDRESS_OF(runner_b, action_default),
        [1]  = ADDRESS_OF(runner_b, action_us_policer),
        [2]  = ADDRESS_OF(runner_b, action_ttl_decrement),
        [3]  = ADDRESS_OF(runner_b, action_default),
        [4]  = ADDRESS_OF(runner_b, action_dscp_remarking),
        [5]  = ADDRESS_OF(runner_b, action_nat),
        [6]  = ADDRESS_OF(runner_b, action_gre_remark),
        [7]  = ADDRESS_OF(runner_b, action_outer_pbits_remarking),
        [8]  = ADDRESS_OF(runner_b, action_inner_pbits_remarking),
        [9]  = ADDRESS_OF(runner_b, action_dslite),
        [10] = ADDRESS_OF(runner_b, action_gre_tunnel),
        [11] = ADDRESS_OF(runner_b, action_pppoe),
        [12 ... 15] = ADDRESS_OF(runner_b, action_default),
        [16] = ADDRESS_OF(runner_b, flow_cache_update_header)
    };

    /* MAIN A */
    actions_arr_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_FLOW_BASED_ACTION_PTR_TABLE_ADDRESS);
    rdd_action_vector_set(ds_main_actions_arr, actions_arr_ptr, DS_ACTION_ID_TOTAL_NUM);

    /* MAIN B */
    actions_arr_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_FLOW_BASED_ACTION_PTR_TABLE_ADDRESS);
    rdd_action_vector_set(us_main_actions_arr, actions_arr_ptr, US_ACTION_ID_TOTAL_NUM);
}
#endif
#endif
