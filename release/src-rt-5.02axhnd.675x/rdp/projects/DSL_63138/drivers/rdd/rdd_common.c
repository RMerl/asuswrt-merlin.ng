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


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

int                                g_dbg_lvl;
RDD_FC_MCAST_CONNECTION2_TABLE_DTS *g_fc_mcast_connection2_table_ptr;
#if defined(DSL_63138) || defined(DSL_63148)
RDD_CONNECTION_TABLE_DTS    *g_ds_connection_table_ptr;
#endif

uint8_t*  g_runner_ddr_base_addr;
uint32_t  g_runner_ddr_base_addr_phys;
uint8_t*  g_runner_tables_ptr;
uint32_t  g_runner_tables_ptr_phys;
uint8_t*  g_runner_extra_ddr_base_addr;
uint32_t  g_runner_extra_ddr_base_addr_phys;
uint32_t  g_mac_table_size;
uint32_t  g_mac_table_search_depth;
uint32_t  g_iptv_table_size;
uint32_t  g_ddr_headroom_size;
uint8_t   g_broadcom_switch_mode = 0;
uint32_t  g_bridge_flow_cache_mode;
uint32_t  g_iptv_table_search_depth;
uint32_t  g_cpu_tx_queue_write_ptr[ 4 ];
uint32_t  g_cpu_tx_queue_free_counter[ 4 ] = { 0, 0, 0, 0 };
uint32_t  g_cpu_tx_queue_abs_data_ptr_write_ptr[ 4 ];
uint32_t  g_rate_controllers_pool_idx;
uint32_t  g_ih_lookup_mode_occupied[ 2 ];
uint32_t  g_free_context_entries_number;
uint32_t  g_free_context_entries_head;
uint32_t  g_free_context_entries_tail;
uint32_t  *g_free_connection_context_entries;
uint32_t  g_src_mac_anti_spoofing_last_rule_index[ 6 ];
uint32_t  g_acl_layer2_vlan_index_counter[ 6 ][ 32 ];
uint32_t  g_acl_layer2_last_vlan_index[ 6 ];
uint32_t  g_acl_layer2_last_rule_index[ 6 ];
uint32_t  g_acl_layer3_last_rule_index[ 6 ];
uint32_t  g_vlan_mapping_command_to_action[ rdd_max_vlan_command ][ rdd_max_pbits_command ];
uint32_t  g_epon_mode;
uint32_t  g_chip_revision;
uint8_t   g_lookup_port_init_mapping_table[16];
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
bdmf_sysb  *g_pci_tx_skb_reference_pointers_array[ LILAC_RDD_PCI_TX_NUMBER_OF_FIFOS ][ LILAC_RDD_PCI_TX_FIFO_SIZE ];
#endif
RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTE  g_ingress_classification_rule_cfg_table[ 2 ];
BL_LILAC_RDD_WAN_PHYSICAL_PORT_DTE             g_wan_physical_port;
RDD_WAN_TX_POINTERS_TABLE_DTS                  *wan_tx_pointers_table_ptr;
RDD_64_BIT_TABLE_CFG                           g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
RDD_DDR_TABLE_CFG                              g_ddr_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
BL_LILAC_RDD_ACL_LAYER3_FILTER_MODE_DTE        g_acl_layer3_filter_mode[ 6 ];
BL_LILAC_RDD_BRIDGE_PORT_DTE                   g_broadcom_switch_physical_port = 0;
uint32_t                                       g_bbh_peripheral_eth_rx[] =
{
    BBH_PERIPHERAL_ETH0_RX,
	BBH_PERIPHERAL_ETH1_RX,
	BBH_PERIPHERAL_ETH2_RX,
	BBH_PERIPHERAL_ETH3_RX,
	BBH_PERIPHERAL_ETH4_RX
};
uint32_t  g_cpu_tx_skb_free_indexes_release_ptr = 0;
uint32_t  g_cpu_tx_released_skb_counter = 0;
uint32_t  g_cpu_tx_no_free_skb_counter = 0;
uint32_t  g_cpu_tx_queue_full_counter = 0;
uint32_t  g_cpu_tx_sent_abs_packets_counter = 0;
#if !defined(FIRMWARE_INIT)
cpu_tx_skb_free_indexes_cache_t g_cpu_tx_skb_free_indexes_cache;
uint32_t  g_cpu_tx_pending_free_indexes_counter = 0;
#endif
uint32_t  g_cpu_tx_abs_packet_limit = 0;
uint16_t  *g_free_skb_indexes_fifo_table = NULL;
uint8_t **g_cpu_tx_skb_pointers_reference_array = NULL;
uint8_t *g_dhd_tx_cpu_usage_reference_array = NULL;
rdd_phys_addr_t *g_cpu_tx_data_pointers_reference_array = NULL;
rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address = 0;
rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address_last_idx = 0;

#if defined (CONFIG_DHD_RUNNER)
uint32_t  g_cpu_tx_dhd_free_counter = 0;
uint32_t  g_cpu_tx_dhd_threshold = 0;
uint32_t  g_cpu_tx_dhd_over_threshold_counter = 0;
#endif

typedef struct
{
    bdmf_ipv6_t ipv6_address;
    uint16_t    ref_count;
} ipv6_host_table_t;

/*missing-braces warning is enabled, so can't use {0} initializer. Relying on BSS zero init rule instead.*/
static ipv6_host_table_t g_ipv6_host_table[RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE];
static uint16_t g_ipv4_host_ref_count_table[RDD_IPV4_HOST_ADDRESS_TABLE_SIZE];


BL_LILAC_RDD_VERSION_DTS  gs_rdd_version =
{
    LILAC_RDD_RELEASE,
    LILAC_RDD_VERSION,
    LILAC_RDD_PATCH,
    LILAC_RDD_REVISION
};

DEFINE_BDMF_FASTLOCK( int_lock );
DEFINE_BDMF_FASTLOCK( int_lock_irq );

#if defined(FIRMWARE_INIT)
int  g_lock_state = LILAC_RDD_FALSE;
void fw_init_lock ( bdmf_fastlock * );
void fw_init_unlock ( bdmf_fastlock * );
void fw_init_lock_irq ( bdmf_fastlock  *xo_int_lock, unsigned long *flags );
void fw_init_unlock_irq ( bdmf_fastlock  *xi_int_lock, unsigned long flags );
#endif

BL_LILAC_RDD_ERROR_DTE rdd_us_padding_config ( BL_LILAC_RDD_UPSTREAM_PADDING_MODE_DTE  xi_control,
                                               BL_LILAC_RDD_UPSTREAM_PADDING_MODE_DTE  xi_cpu_control,
		                                       uint16_t                                xi_size )
{
    RDD_SYSTEM_CONFIGURATION_DTS  *bridge_cfg_register;
    uint16_t                               cpu_size;

    bridge_cfg_register = ( RDD_SYSTEM_CONFIGURATION_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_SYSTEM_CONFIGURATION_ADDRESS );

    if ( xi_control == BL_LILAC_RDD_UPSTREAM_PADDING_DISABLE )
    {
        xi_size = 0;
        cpu_size = 0;
    }
    else
    {
        cpu_size = xi_size;

        if ( xi_cpu_control == BL_LILAC_RDD_UPSTREAM_PADDING_DISABLE )
        {
            cpu_size = 0;
        }
    }

    RDD_SYSTEM_CONFIGURATION_US_PADDING_MAX_SIZE_WRITE ( xi_size, bridge_cfg_register );
    RDD_SYSTEM_CONFIGURATION_US_PADDING_CPU_MAX_SIZE_WRITE ( cpu_size, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_mtu_config ( uint16_t  xi_mtu_size )
{
    RDD_SYSTEM_CONFIGURATION_DTS  *bridge_cfg_register;

    /* configuration will be made only in US direction for the time being*/
    bridge_cfg_register = ( RDD_SYSTEM_CONFIGURATION_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_SYSTEM_CONFIGURATION_ADDRESS );

    if ( xi_mtu_size <= RDD_IPV6_HEADER_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_MTU_INVALID_LENGTH );
    }

    RDD_SYSTEM_CONFIGURATION_MTU_MINUS_40_WRITE ( ( xi_mtu_size - RDD_IPV6_HEADER_SIZE ), bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ddr_headroom_size_config ( uint16_t  xi_ddr_headroom_size )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error = BL_LILAC_RDD_OK;
#if !defined(FIRMWARE_INIT)
    unsigned long           flags;
#endif

    /* updating global var */
    g_ddr_headroom_size = xi_ddr_headroom_size;

    /* updating DDR buffers base address (taken from f_rdd_bpm_initialize() ) */
    f_rdd_ddr_optimized_base_config ( xi_ddr_headroom_size );

    /* updating actual ddr headroom size to runner memory (taken from f_rdd_ddr_initialize) */
    f_rdd_ddr_headroom_size_private_config ( xi_ddr_headroom_size );
    
#if !defined(FIRMWARE_INIT)
    /*sending message to runner to update io memory for dma access*/
    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_DDR_HEADROOM_SIZE_SET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

    if ( rdd_error == BL_LILAC_RDD_OK )
        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_DDR_HEADROOM_SIZE_SET, FAST_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

    if ( rdd_error == BL_LILAC_RDD_OK )
        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_DDR_HEADROOM_SIZE_SET, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );
    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
#endif

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_ddr_optimized_base_config(uint32_t ddr_packet_headroom_size)
{
    uint32_t *bpm_ddr_optimized_base_ptr, *bpm_ddr_base_ptr;
    uint32_t start_addr;

    /* pool#0 */
    bpm_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_BUFFERS_BASE_ADDRESS);
    MREAD_32(bpm_ddr_base_ptr, start_addr);
    start_addr += ddr_packet_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_OPTIMIZED_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);
    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_OPTIMIZED_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);

#if defined(WL4908)
    /* pool#1 */
    bpm_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_1_BUFFERS_BASE_ADDRESS);
    MREAD_32(bpm_ddr_base_ptr, start_addr);
    start_addr += ddr_packet_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET;

    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_1_OPTIMIZED_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);
    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_1_OPTIMIZED_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);
#endif

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE f_rdd_ddr_headroom_size_private_config ( uint32_t  xi_ddr_headroom_size )
{
    uint16_t  *headroom_size_ptr;

    headroom_size_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BPM_DDR_BUFFER_HEADROOM_SIZE_ADDRESS );
    MWRITE_16( headroom_size_ptr, xi_ddr_headroom_size );

    headroom_size_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BPM_DDR_BUFFER_HEADROOM_SIZE_2_BYTE_RESOLUTION_ADDRESS );
    MWRITE_16( headroom_size_ptr, ( xi_ddr_headroom_size >> 1 ) );

    headroom_size_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BPM_DDR_BUFFER_HEADROOM_SIZE_ADDRESS );
    MWRITE_16( headroom_size_ptr, xi_ddr_headroom_size );

    headroom_size_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BPM_DDR_BUFFER_HEADROOM_SIZE_2_BYTE_RESOLUTION_ADDRESS );
    MWRITE_16( headroom_size_ptr, ( xi_ddr_headroom_size >> 1 ) );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_interrupt_vector_get ( uint32_t  xi_interrupt_number,
                                                  uint8_t   *xo_sub_interrpt_vector )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_INT_CTRL  runner_interrupt_controller_0_register;
    RUNNER_REGS_CFG_INT_CTRL  runner_interrupt_controller_1_register;
    RUNNER_REGS_CFG_INT_MASK  runner_interrupt_mask_register;

    RUNNER_REGS_0_CFG_INT_CTRL_READ ( runner_interrupt_controller_0_register );
    RUNNER_REGS_1_CFG_INT_CTRL_READ ( runner_interrupt_controller_1_register );

    RUNNER_REGS_0_CFG_INT_MASK_READ ( runner_interrupt_mask_register );

    /* clear the interrupt by writing "1" to the corresponding bit in the Runner's interrupt register */
    switch ( xi_interrupt_number )
    {
    case 0:
        *xo_sub_interrpt_vector = ( runner_interrupt_controller_0_register.int0_sts | runner_interrupt_controller_1_register.int0_sts ) & runner_interrupt_mask_register.int0_mask;
        break;
    case 1:
        *xo_sub_interrpt_vector = ( runner_interrupt_controller_0_register.int1_sts | runner_interrupt_controller_1_register.int1_sts ) & runner_interrupt_mask_register.int1_mask;
        break;
    }
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_interrupt_clear ( uint32_t  xi_interrupt_number,
                                             uint32_t  xi_sub_interrpt_number )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_INT_CTRL  runner_interrupt_controller_register;

    RDD_CLEAR_REGISTER ( &runner_interrupt_controller_register );

    /* clear the interrupt by writing "1" to the corresponding bit in the Runner's interrupt register */
    switch ( xi_interrupt_number )
    {
    case 0:
        runner_interrupt_controller_register.int0_sts = ( 1 << xi_sub_interrpt_number );
        break;
    case 1:
        runner_interrupt_controller_register.int1_sts = ( 1 << xi_sub_interrpt_number );
        break;
    case 2:
        runner_interrupt_controller_register.int2_sts = 1;
        break;
    case 3:
        runner_interrupt_controller_register.int3_sts = 1;
        break;
    case 4:
        runner_interrupt_controller_register.int4_sts = 1;
        break;
    case 5:
        runner_interrupt_controller_register.int5_sts = 1;
        break;
    case 6:
        runner_interrupt_controller_register.int6_sts = 1;
        break;
    case 7:
        runner_interrupt_controller_register.int7_sts = 1;
        break;
    case 8:
        runner_interrupt_controller_register.int8_sts = 1;
        break;
    case 9:
        runner_interrupt_controller_register.int9_sts = 1;
        break;
    }

    RUNNER_REGS_0_CFG_INT_CTRL_WRITE ( runner_interrupt_controller_register );
    RUNNER_REGS_1_CFG_INT_CTRL_WRITE ( runner_interrupt_controller_register );
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_interrupt_mask ( uint32_t  xi_interrupt_number,
                                            uint32_t  xi_sub_interrpt_number )
{
#if !defined(FIRMWARE_INIT)
    unsigned long             flags;
    RUNNER_REGS_CFG_INT_MASK  runner_interrupt_mask_register;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RUNNER_REGS_0_CFG_INT_MASK_READ ( runner_interrupt_mask_register );

    /* mask the interrupt by writing "1" to the corresponding bit in the Runner's interrupt register */
    switch ( xi_interrupt_number )
    {
    case 0:
        runner_interrupt_mask_register.int0_mask &= ~( 1 << xi_sub_interrpt_number );
        break;
    case 1:
        runner_interrupt_mask_register.int1_mask &= ~( 1 << xi_sub_interrpt_number );
        break;
    case 2:
        runner_interrupt_mask_register.int2_mask = 0;
        break;
    case 3:
        runner_interrupt_mask_register.int3_mask = 0;
        break;
    case 4:
        runner_interrupt_mask_register.int4_mask = 0;
        break;
    case 5:
        runner_interrupt_mask_register.int5_mask = 0;
        break;
    case 6:
        runner_interrupt_mask_register.int6_mask = 0;
        break;
    case 7:
        runner_interrupt_mask_register.int7_mask = 0;
        break;
    case 8:
        runner_interrupt_mask_register.int8_mask = 0;
        break;
    case 9:
        runner_interrupt_mask_register.int9_mask = 0;
        break;
    default:
        break;
    }

    RUNNER_REGS_0_CFG_INT_MASK_WRITE ( runner_interrupt_mask_register );
    RUNNER_REGS_1_CFG_INT_MASK_WRITE ( runner_interrupt_mask_register );

    bdmf_fastlock_unlock_irq (&int_lock_irq, flags);
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_interrupt_unmask ( uint32_t  xi_interrupt_number,
                                              uint32_t  xi_sub_interrpt_number )
{
#if !defined(FIRMWARE_INIT)
    unsigned long             flags;
    RUNNER_REGS_CFG_INT_MASK  runner_interrupt_mask_register;

    bdmf_fastlock_lock_irq (&int_lock_irq, flags);

    RUNNER_REGS_0_CFG_INT_MASK_READ ( runner_interrupt_mask_register );
    
    /* mask the interrupt by writing "1" to the corresponding bit in the Runner's interrupt register */
    switch ( xi_interrupt_number )
    {
    case 0:
        runner_interrupt_mask_register.int0_mask |= ( 1 << xi_sub_interrpt_number );
        break;
    case 1:
        runner_interrupt_mask_register.int1_mask |= ( 1 << xi_sub_interrpt_number );
        break;
    case 2:
        runner_interrupt_mask_register.int2_mask = 1;
        break;
    case 3:
        runner_interrupt_mask_register.int3_mask = 1;
        break;
    case 4:
        runner_interrupt_mask_register.int5_mask = 1;
        break;
    case 5:
        runner_interrupt_mask_register.int5_mask = 1;
        break;
    case 6:
        runner_interrupt_mask_register.int6_mask = 1;
        break;
    case 7:
        runner_interrupt_mask_register.int7_mask = 1;
        break;
    case 8:
        runner_interrupt_mask_register.int8_mask = 1;
        break;
    case 9:
        runner_interrupt_mask_register.int9_mask = 1;
        break;
    default:
        break;
    }

    RUNNER_REGS_0_CFG_INT_MASK_WRITE ( runner_interrupt_mask_register );
    RUNNER_REGS_1_CFG_INT_MASK_WRITE ( runner_interrupt_mask_register );

    bdmf_fastlock_unlock_irq (&int_lock_irq, flags);
#endif
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_interrupt_mask_get ( uint32_t  xi_interrupt_number,
                                                uint8_t   *xo_sub_interrpt_mask )
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_INT_MASK  runner_interrupt_mask_register;

    switch ( xi_interrupt_number )
    {
    case 0:
        RUNNER_REGS_0_CFG_INT_MASK_READ ( runner_interrupt_mask_register );
        *xo_sub_interrpt_mask = runner_interrupt_mask_register.int0_mask;
        break;
    case 1:
        RUNNER_REGS_1_CFG_INT_MASK_READ ( runner_interrupt_mask_register );
        *xo_sub_interrpt_mask = runner_interrupt_mask_register.int1_mask;
        break;
    }
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_timer_task_config ( rdpa_traffic_dir  xi_direction,
                                               uint16_t          xi_task_period_in_usec,
                                               uint16_t          xi_firmware_routine_address_id )
{
    RDD_TIMER_CONTROL_DESCRIPTOR_DTS       *timer_control_descriptor_ptr;
    RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS    *timer_tasks_table_ptr;
    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_DTS    *timer_tasks_entry_ptr;
    RDD_SYSTEM_CONFIGURATION_DTS  *bridge_cfg_register_ptr;
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;
#endif
    uint16_t                               number_of_active_tasks;
    uint16_t                               task_period_reload;

    if ( ( xi_task_period_in_usec % TIMER_SCHEDULER_TASK_PERIOD ) != 0 )
    {
        return ( BL_LILAC_RDD_ERROR_TIMER_TASK_PERIOD );
    }

    if ( xi_direction == rdpa_dir_ds )
    {

        bridge_cfg_register_ptr = ( RDD_SYSTEM_CONFIGURATION_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_SYSTEM_CONFIGURATION_ADDRESS );

        if ( ( xi_firmware_routine_address_id == CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID ) ||
             ( xi_firmware_routine_address_id == FREE_SKB_INDEX_ALLOCATE_CODE_ID ) ||
             ( xi_firmware_routine_address_id == DOWNSTREAM_DHD_TX_POST_CLOSE_AGGREGATION_CODE_ID ) )  /* FIXME!! Wen.. see if we want to move it to MAIN_A */
        {
            timer_control_descriptor_ptr = ( RDD_TIMER_CONTROL_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_MAIN_TIMER_CONTROL_DESCRIPTOR_ADDRESS );

            timer_tasks_table_ptr = ( RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS );
        }
        else
        {
            timer_control_descriptor_ptr = ( RDD_TIMER_CONTROL_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_TIMER_CONTROL_DESCRIPTOR_ADDRESS );

            timer_tasks_table_ptr = ( RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS );
        }
    }
    else
    {
        bridge_cfg_register_ptr = ( RDD_SYSTEM_CONFIGURATION_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_SYSTEM_CONFIGURATION_ADDRESS );

        if ( ( xi_firmware_routine_address_id == CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID ) || ( xi_firmware_routine_address_id == UPSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID ) ||
             ( xi_firmware_routine_address_id == FREE_SKB_INDEX_ALLOCATE_CODE_ID ) )
        {
            timer_control_descriptor_ptr = ( RDD_TIMER_CONTROL_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_MAIN_TIMER_CONTROL_DESCRIPTOR_ADDRESS );

            timer_tasks_table_ptr = ( RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS );
        }
        else
        {
            timer_control_descriptor_ptr = ( RDD_TIMER_CONTROL_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_TIMER_CONTROL_DESCRIPTOR_ADDRESS );

            timer_tasks_table_ptr = ( RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS );
        }
    }

    /* Want the timer task to have a period of 1000us */
#ifdef RUNNER_FWTRACE
    RDD_SYSTEM_CONFIGURATION_TIMER_SCHEDULER_PERIOD_WRITE ( (TIMER_SCHEDULER_TASK_PERIOD*(1000/TIMER_PERIOD_NS)) - 1, bridge_cfg_register_ptr );
#else
    RDD_SYSTEM_CONFIGURATION_TIMER_SCHEDULER_PERIOD_WRITE ( TIMER_SCHEDULER_TASK_PERIOD - 1, bridge_cfg_register_ptr );
#endif

    RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_READ ( number_of_active_tasks, timer_control_descriptor_ptr );

    if ( number_of_active_tasks == LILAC_RDD_NUMBER_OF_TIMER_TASKS )
    {
        return ( BL_LILAC_RDD_ERROR_TIMER_TASK_TABLE_FULL );
    }

    timer_tasks_entry_ptr = &timer_tasks_table_ptr->entry[ number_of_active_tasks ];

    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_FIRMWARE_PTR_WRITE ( xi_firmware_routine_address_id, timer_tasks_entry_ptr );

    task_period_reload = xi_task_period_in_usec / TIMER_SCHEDULER_TASK_PERIOD;

    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_COUNTER_RELOAD_WRITE ( task_period_reload, timer_tasks_entry_ptr );
    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_PERIOD_WRITE ( 1, timer_tasks_entry_ptr );

    number_of_active_tasks++;

    RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_WRITE ( number_of_active_tasks, timer_control_descriptor_ptr );

#if !defined(FIRMWARE_INIT)
    if ( number_of_active_tasks == 1 )
    {
        if ( xi_firmware_routine_address_id == DOWNSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID ||
             xi_firmware_routine_address_id == DOWNSTREAM_SERVICE_QUEUES_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID )
        {
            runner_cpu_wakeup_register.req_trgt = TIMER_SCHEDULER_PICO_A_THREAD_NUMBER / 32;
            runner_cpu_wakeup_register.thread_num = TIMER_SCHEDULER_PICO_A_THREAD_NUMBER % 32;
        }
        else if ( ( xi_firmware_routine_address_id == UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID ) ||
             ( xi_firmware_routine_address_id == UPSTREAM_QUASI_BUDGET_ALLOCATE_CODE_ID ) )
        {
            runner_cpu_wakeup_register.req_trgt = TIMER_SCHEDULER_PICO_B_THREAD_NUMBER / 32;
            runner_cpu_wakeup_register.thread_num = TIMER_SCHEDULER_PICO_B_THREAD_NUMBER % 32;
        }
        else
        {
            runner_cpu_wakeup_register.req_trgt = TIMER_SCHEDULER_MAIN_THREAD_NUMBER / 32;
            runner_cpu_wakeup_register.thread_num = TIMER_SCHEDULER_MAIN_THREAD_NUMBER % 32;
        }

        if ( xi_direction == rdpa_dir_ds )
        {
            RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
        }
        else
        {
            RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE ( runner_cpu_wakeup_register );
        }
    }
#endif

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_4_bytes_counter_get ( uint32_t  xi_counter_group, 
                                                 uint32_t  xi_counter_num,
                                                 uint32_t  *xo_counter )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                *pm_counters_buffer_ptr;
    unsigned long           flags;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    /* read user counter and reset its value */
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_PM_COUNTER_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
                                            xi_counter_group + xi_counter_num / 16, xi_counter_num % 16, LILAC_RDD_TRUE, BL_LILAC_RDD_WAIT );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    pm_counters_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + PM_COUNTERS_BUFFER_ADDRESS );

    MREAD_32( ( uint32_t * )pm_counters_buffer_ptr, *xo_counter );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_2_bytes_counter_get ( uint32_t  xi_counter_group,
                                                 uint32_t  xi_counter_num,
                                                 uint16_t  *xo_counter )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint16_t                *pm_counters_buffer_ptr;
    unsigned long           flags;

    bdmf_fastlock_lock_irq ( &int_lock_irq, flags );

    /* read user counter and reset its value */
    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_PM_COUNTER_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
                                            xi_counter_group + xi_counter_num / 32, xi_counter_num % 32, LILAC_RDD_FALSE, BL_LILAC_RDD_WAIT );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
        return ( rdd_error );
    }

    pm_counters_buffer_ptr = ( uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + PM_COUNTERS_BUFFER_ADDRESS );

    MREAD_16( ( uint16_t * )pm_counters_buffer_ptr, *xo_counter );

    bdmf_fastlock_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_us_pci_flow_cache_config ( BL_LILAC_RDD_FILTER_MODE_DTE  xi_pci_flow_cache_mode )
{
    RDD_SYSTEM_CONFIGURATION_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_SYSTEM_CONFIGURATION_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_SYSTEM_CONFIGURATION_ADDRESS );

    RDD_SYSTEM_CONFIGURATION_US_PCI_FLOW_CACHE_ENABLE_WRITE( xi_pci_flow_cache_mode, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_debug_mode_config ( bdmf_boolean  xi_enable )
{
    RDD_SYSTEM_CONFIGURATION_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_SYSTEM_CONFIGURATION_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_SYSTEM_CONFIGURATION_ADDRESS );

    RDD_SYSTEM_CONFIGURATION_DEBUG_MODE_WRITE ( xi_enable, bridge_cfg_register );

    bridge_cfg_register = ( RDD_SYSTEM_CONFIGURATION_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_SYSTEM_CONFIGURATION_ADDRESS );

    RDD_SYSTEM_CONFIGURATION_DEBUG_MODE_WRITE ( xi_enable, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
/*                                                                            */
/*               CRC Software Variables (HASH Function)                       */
/*                                                                            */
/******************************************************************************/

static const int32_t  order[2] = { 16, 32 };
static const int32_t  direct[2] = { 1, 1 };
static const int32_t  refin[2] = { 0, 0 };
static const int32_t  refout[2] = { 0, 0 };
static const uint32_t polynom[2] = { 0x1021, 0x04C11DB7 };
static const uint32_t crcinit[2] = { 0xFFFF, 0xFFFFFFFF };
static const uint32_t crcxor[2] = { 0x0, 0xFFFFFFFF };

static uint32_t crcmask[2];
static uint32_t crchighbit[2];
static uint32_t crcinit_direct[2];
static uint32_t crcinit_nondirect[2];
static uint32_t crctab[2][256];

static uint32_t reflect ( uint32_t crc, int32_t bitnum ) {

    /* reflects the lower 'bitnum' bits of 'crc' */
    uint32_t i, j = 1, crcout = 0;

    for ( i = (uint32_t)1 << ( bitnum - 1 ); i; i >>= 1 ) {
        if ( crc & i )
            crcout |= j;

        j <<= 1;
    }

    return ( crcout );
}


uint32_t crcbitbybit ( uint8_t *p, uint32_t byte_len, uint32_t bit_len, uint32_t crc_residue, uint32_t crc_type )
{
    /* bit by bit algorithm with augmented zero bytes.
       does not use lookup table, suited for polynom orders between 1...32. */
    uint32_t  i, j, c, bit;
    uint32_t  crc = crc_residue;

    if ( bit_len != 0 )
    {
        c = *p++;

        if ( refin[crc_type] )
        {
            c = reflect ( c, 8 );
        }

        j = ( 1 << ( bit_len - 1 ) );

        for ( ; j ; j >>= 1 )
        {
            bit = crc & crchighbit[crc_type];
            crc <<= 1;

            if ( c & j )
            {
                bit ^= crchighbit[crc_type];
            }

            if ( bit )
            {
                crc ^= polynom[crc_type];
            }
        }
    }

    for ( i = 0; i < byte_len; i++ ) {
        c = (uint32_t)*p++;

        if ( refin[crc_type] )
            c = reflect (c, 8);

        for ( j = 0x80; j; j >>= 1 ) {
            bit = crc & crchighbit[crc_type];
            crc <<= 1;

            if ( c & j )
                bit ^= crchighbit[crc_type];

            if ( bit )
                crc^= polynom[crc_type];
        }
    }

    if ( refout[crc_type] )
        crc = reflect (crc, order[crc_type]);

    crc ^= crcxor[crc_type];
    crc &= crcmask[crc_type];

    return ( crc );
}


static void generate_crc_table ( void )
{
    /* make CRC lookup table used by table algorithms */
    int32_t   i, j;
    uint32_t  bit, crc;
    uint32_t  crc_type;

    for ( crc_type = RDD_CRC_TYPE_16; crc_type <= RDD_CRC_TYPE_32; crc_type++ )
    {
        for ( i = 0; i < 256; i++ ) {
            crc = ( uint32_t )i;

            if ( refin[crc_type] )
                crc = reflect ( crc, 8 );

            crc <<= order[crc_type] - 8;

            for ( j = 0; j < 8; j++ ) {
                bit = crc & crchighbit[crc_type];
                crc <<= 1;

                if ( bit )
                    crc ^= polynom[crc_type];
            }

            if ( refin[crc_type] )
                crc = reflect ( crc, order[crc_type] );

            crc &= crcmask[crc_type];
            crctab[crc_type][i] = crc;
        }
    }
}

void init_crc ( void )
{
    uint32_t  i;
    uint32_t  bit, crc;
    uint32_t  crc_type;

    for ( crc_type = RDD_CRC_TYPE_16; crc_type <= RDD_CRC_TYPE_32; crc_type++ )
    {
        /* at first, compute constant bit masks for whole CRC and CRC high bit */
        crcmask[crc_type] = ((((uint32_t)1 << (order[crc_type]-1)) - 1) << 1) | 1;
        crchighbit[crc_type] = (uint32_t)1 << (order[crc_type]-1);

        generate_crc_table ();

        /* compute missing initial CRC value */
        if ( !direct[crc_type] ) {
            crcinit_nondirect[crc_type] = crcinit[crc_type];
            crc = crcinit[crc_type];

            for ( i = 0; i < order[crc_type]; i++ ) {
                bit = crc & crchighbit[crc_type];
                crc <<= 1;

                if ( bit )
                    crc ^= polynom[crc_type];
            }

            crc &= crcmask[crc_type];
            crcinit_direct[crc_type] = crc;
        }
        else {
            crcinit_direct[crc_type] = crcinit[crc_type];
            crc = crcinit[crc_type];

            for ( i = 0; i < order[crc_type]; i++ ) {
                bit = crc & 1;

                if ( bit )
                    crc ^= polynom[crc_type];

                crc >>= 1;

                if ( bit )
                    crc |= crchighbit[crc_type];
            }

            crcinit_nondirect[crc_type] = crc;
        }
    }
}


uint32_t get_crc_init_value ( uint32_t crc_type )
{
    return ( crcinit_direct[crc_type] );
}

BL_LILAC_RDD_ERROR_DTE rdd_unknown_sa_mac_cmd_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE          xi_bridge_port,
                                                       BL_LILAC_RDD_UNKNOWN_MAC_COMMAND_DTE  xi_slf_cmd )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_sa_mac_lookup_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  BL_LILAC_RDD_MAC_LOOKUP_DTE   xi_src_mac_lookup_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_1588_mode_config ( BL_LILAC_RDD_1588_MODE_DTE  xi_1588_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_unknown_da_mac_cmd_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE          xi_bridge_port,
                                                       BL_LILAC_RDD_UNKNOWN_MAC_COMMAND_DTE  xi_dlf_cmd )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_da_mac_lookup_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  BL_LILAC_RDD_MAC_LOOKUP_DTE   xi_dst_mac_lookup_mode )
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_ipv4_host_address_table_set(uint32_t xi_table_index, 
                                                       bdmf_ipv4 xi_ipv4_host_addr,
                                                       uint16_t xi_ref_count)
{
    RDD_IPV4_HOST_ADDRESS_TABLE_DTS *host_table = RDD_IPV4_HOST_ADDRESS_TABLE_PTR();

    if (xi_table_index >= RDD_IPV4_HOST_ADDRESS_TABLE_SIZE)
    {
        return BL_LILAC_RDD_ERROR_IPHOST_TABLE_INDEX_INVALID;
    }

    MWRITE_32( host_table->entry + xi_table_index, xi_ipv4_host_addr);
    g_ipv4_host_ref_count_table[xi_table_index] = xi_ref_count;

    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_ipv4_host_address_table_get(uint32_t xi_table_index, 
                                                       bdmf_ipv4 *xo_ipv4_host_addr,
                                                       uint16_t *xo_ref_count)
{
    RDD_IPV4_HOST_ADDRESS_TABLE_DTS *host_table = RDD_IPV4_HOST_ADDRESS_TABLE_PTR();

    if (xi_table_index >= RDD_IPV4_HOST_ADDRESS_TABLE_SIZE)
    {
        return BL_LILAC_RDD_ERROR_IPHOST_TABLE_INDEX_INVALID;
    }

    /*Retrieve host address from RDD table*/
    MREAD_32( host_table->entry + xi_table_index, *xo_ipv4_host_addr);
    /*Retrieve reference count from local table*/
    *xo_ref_count = g_ipv4_host_ref_count_table[xi_table_index];

    return BL_LILAC_RDD_OK;
}
 
BL_LILAC_RDD_ERROR_DTE rdd_ipv6_host_address_table_set(uint32_t xi_table_index, 
                                                       const bdmf_ipv6_t *xi_ipv6_host_addr,
                                                       uint16_t xi_ref_count)
{
    RDD_IPV6_HOST_ADDRESS_CRC_TABLE_DTS *host_table = RDD_IPV6_HOST_ADDRESS_CRC_TABLE_PTR();
    uint32_t ipv6_crc, crc_init_value;

    if (xi_table_index >= RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE)
    {
        return BL_LILAC_RDD_ERROR_IPHOST_TABLE_INDEX_INVALID;
    }

    /*Reduce IPV6 address to a 32-bit value using CRC. This reduced value is what RDP FW will be using for lookup.*/
    crc_init_value = get_crc_init_value ( RDD_CRC_TYPE_32 );
    ipv6_crc = crcbitbybit ( (uint8_t *)xi_ipv6_host_addr->data, 16, 0, crc_init_value, RDD_CRC_TYPE_32 );

    /*Store ipv6 address in a local table so we can return in the get accessor*/
    g_ipv6_host_table[xi_table_index].ipv6_address = *xi_ipv6_host_addr;
    g_ipv6_host_table[xi_table_index].ref_count = xi_ref_count;

    /*Store the CRC in the RDP FW table*/
    MWRITE_32( host_table->entry + xi_table_index, ipv6_crc);

    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_ipv6_host_address_table_get(uint32_t xi_table_index, 
                                                       bdmf_ipv6_t *xo_ipv6_host_addr,
                                                       uint16_t *xo_ref_count)
{
    if (xi_table_index >= RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE)
    {
        return BL_LILAC_RDD_ERROR_IPHOST_TABLE_INDEX_INVALID;
    }

    /*Look up address in local table. The full IP address is not stored in an RDP table, only the CRC is.*/
    *xo_ipv6_host_addr = g_ipv6_host_table[xi_table_index].ipv6_address;
    *xo_ref_count = g_ipv6_host_table[xi_table_index].ref_count;

    return BL_LILAC_RDD_OK;
}


BL_LILAC_RDD_ERROR_DTE rdd_fw_mac_da_filter_table_set(uint32_t xi_mac_table_index, 
                                                      uint8_t *xi_mac_address)
{
    uint8_t * p_runner_table_ds;
    uint8_t * p_runner_table_us;
    uint8_t * p_entry_count_ds;
    uint8_t * p_entry_count_us;
    /* entry_size should be 8 (2 bytes padding plus 6 bytes of mac addr) */
    uint8_t   entry_size = (DS_FW_MAC_ADDRS_BYTE_SIZE / RDD_DS_FW_MAC_ADDRS_SIZE);
    uint8_t   new_mac_is_valid = 0;
    uint8_t   entry_was_valid  = 0;

    if (xi_mac_table_index >= RDD_DS_FW_MAC_ADDRS_SIZE)
    {
        return BL_LILAC_RDD_ERROR_HOST_MAC_TABLE_INDEX_INVALID;
    }

    p_runner_table_ds = (uint8_t *) ((DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FW_MAC_ADDRS_ADDRESS) + (xi_mac_table_index * entry_size));
    p_runner_table_us = (uint8_t *) ((DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FW_MAC_ADDRS_ADDRESS) + (xi_mac_table_index * entry_size));

    p_runner_table_ds[0] = 0;
    p_runner_table_us[0] = 0;
    p_runner_table_ds[1] = 0;
    p_runner_table_us[1] = 0;

    entry_was_valid  |= p_runner_table_ds[2];
    new_mac_is_valid |= xi_mac_address[0];
    p_runner_table_ds[2] = xi_mac_address[0];
    p_runner_table_us[2] = xi_mac_address[0];

    entry_was_valid  |= p_runner_table_ds[3];
    new_mac_is_valid |= xi_mac_address[1];
    p_runner_table_ds[3] = xi_mac_address[1];
    p_runner_table_us[3] = xi_mac_address[1];

    entry_was_valid  |= p_runner_table_ds[4];
    new_mac_is_valid |= xi_mac_address[2];
    p_runner_table_ds[4] = xi_mac_address[2];
    p_runner_table_us[4] = xi_mac_address[2];

    entry_was_valid  |= p_runner_table_ds[5];
    new_mac_is_valid |= xi_mac_address[3];
    p_runner_table_ds[5] = xi_mac_address[3];
    p_runner_table_us[5] = xi_mac_address[3];

    entry_was_valid  |= p_runner_table_ds[6];
    new_mac_is_valid |= xi_mac_address[4];
    p_runner_table_ds[6] = xi_mac_address[4];
    p_runner_table_us[6] = xi_mac_address[4];

    entry_was_valid  |= p_runner_table_ds[7];
    new_mac_is_valid |= xi_mac_address[5];
    p_runner_table_ds[7] = xi_mac_address[5];
    p_runner_table_us[7] = xi_mac_address[5];

    p_entry_count_ds = (uint8_t*) (DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FW_MAC_ADDRS_COUNT_ADDRESS);
    p_entry_count_us = (uint8_t*) (DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FW_MAC_ADDRS_COUNT_ADDRESS);

    // Update the entry count based on changes made
    if (entry_was_valid)
    {
        if (!new_mac_is_valid)
        {
            /* Deleted an entry */
            (*p_entry_count_ds)--;
            (*p_entry_count_us)--;
        }
        /* ELSE: Update... no change in count */
    }
    else
    {
        if (new_mac_is_valid)
        {
            /* Add an entry */
            (*p_entry_count_ds)++;
            (*p_entry_count_us)++;
        }
    }

    return BL_LILAC_RDD_OK;
}

