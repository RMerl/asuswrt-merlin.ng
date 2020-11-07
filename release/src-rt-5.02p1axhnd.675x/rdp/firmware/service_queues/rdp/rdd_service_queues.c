/*
  Copyright (c) 2014 Broadcom
  All Rights Reserved

  <:label-BRCM:2014:DUAL/GPL:standard
    
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
#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif
#include "rdd_service_queues.h"
#include "rdd_data_structures.h"


#ifndef G9991
/* map wan channel id to service queue scheduler */
#if !defined(OREN)
rdd_wan_channel_id_t g_us_service_queue_map[RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_TABLE_SIZE];
#endif
extern BL_LILAC_RDD_ERROR_DTE f_ds_rate_limiter_config ( RDD_RATE_LIMITER_ID_DTE  xi_rate_limiter_id,
                                                         RDD_RATE_LIMIT_PARAMS    *xi_budget );

static void rdd_ds_service_queues_init ( void )
{
    RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_DTS *service_queue_descriptor_ptr;
    RDD_DDR_QUEUE_DESCRIPTOR_DTS            *service_queue_descriptor_entry_ptr;
    RDD_SERVICE_QUEUES_CFG_ENTRY_DTS        *service_tm_descriptor_ptr;
    uint32_t                                queue_id;

    service_queue_descriptor_ptr = RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_PTR ();
    for ( queue_id = 0; queue_id < RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_SIZE; queue_id++ )
    {
        service_queue_descriptor_entry_ptr = &( service_queue_descriptor_ptr->entry[ queue_id ] );

        RDD_DDR_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE ( 1 << queue_id , service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_PACKET_COUNTER_WRITE ( 0 , service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE ( 0 , service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_HEAD_IDX_WRITE ( 0, service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_TAIL_IDX_WRITE ( 0, service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_RATE_CONTROLLER_ID_WRITE ( RDD_RATE_LIMITER_IDLE, service_queue_descriptor_entry_ptr );
#if defined(OREN)
        RDD_DDR_QUEUE_DESCRIPTOR_HEAD_BASE_ENTRY_WRITE ( 0, service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_TAIL_BASE_ENTRY_WRITE ( 8, service_queue_descriptor_entry_ptr );
#else
        RDD_DDR_QUEUE_DESCRIPTOR_TAIL_BASE_ENTRY_WRITE ( 0, service_queue_descriptor_entry_ptr );
#endif
        RDD_DDR_QUEUE_DESCRIPTOR_PROFILE_ID_WRITE ( 0, service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_PROFILE_EN_WRITE ( 0, service_queue_descriptor_entry_ptr );
        RDD_DDR_QUEUE_DESCRIPTOR_CACHE_PTR_WRITE ( SERVICE_QUEUES_DDR_CACHE_FIFO_ADDRESS + queue_id * (SERVICE_QUEUES_DDR_CACHE_FIFO_BYTE_SIZE / 32), service_queue_descriptor_entry_ptr );
    }

    service_tm_descriptor_ptr = (RDD_SERVICE_QUEUES_CFG_ENTRY_DTS *)(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + SERVICE_QUEUES_CFG_ADDRESS);
    RDD_SERVICE_QUEUES_CFG_ENTRY_SERVICE_QUEUES_STATUS_WRITE ( 0, service_tm_descriptor_ptr );
    RDD_SERVICE_QUEUES_CFG_ENTRY_RATE_LIMITER_STATUS_WRITE ( 0, service_tm_descriptor_ptr );
    RDD_SERVICE_QUEUES_CFG_ENTRY_OVERALL_RATE_LIMITER_MODE_WRITE ( 0, service_tm_descriptor_ptr );
}

static void rdd_us_service_queues_init ( void )
{
#if !defined(OREN)
    RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_TABLE_DTS       *service_queue_scheduler_table;
    RDD_WAN_TX_SERVICE_QUEUES_TABLE_DTS                *service_queues_table;
    RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR_DTS  *service_queue_scheduler;
    RDD_WAN_TX_SERVICE_QUEUE_DESCRIPTOR_DTS            *service_queue;
    int i, j;

    service_queue_scheduler_table = RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_TABLE_PTR ();
    service_queues_table          = RDD_WAN_TX_SERVICE_QUEUES_TABLE_PTR ();

    /* Setup all wan_tx service queue pointers in schedulers */
    for ( i = 0; i < sizeof(g_us_service_queue_map)/sizeof(g_us_service_queue_map[0]); i++ )
    {
        service_queue_scheduler = &service_queue_scheduler_table->entry[ i ];

        g_us_service_queue_map[i] = RDD_WAN_CHANNEL_UNASSIGNED;

        for ( j = 0; j < RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER; j++ )
        {
            int queue_id = j;
            queue_id    += i * RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER;

            service_queue = &service_queues_table->entry[ queue_id ];

            /* allocate queue in scheduler */
            RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR_TX_QUEUE_ADDR_WRITE( WAN_TX_SERVICE_QUEUES_TABLE_ADDRESS + (queue_id * sizeof(*service_queue)), service_queue_scheduler, j );

            /* set queue config */
            RDD_WAN_TX_SERVICE_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE( (uint32_t)(1 << j), service_queue );
        }
    }
#endif
}

void rdd_service_queues_initialize ( void )
{
    rdd_ds_service_queues_init();
    rdd_us_service_queues_init();
}

BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_svcq_scheduler_set ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                             BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                             BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                             bdmf_boolean                         enable )
{
#if !defined(OREN)
    RDD_WAN_TX_SERVICE_QUEUES_TABLE_DTS      *service_queues_table;
    RDD_WAN_TX_SERVICE_QUEUE_DESCRIPTOR_DTS  *service_queue;
    RDD_WAN_TX_POINTERS_ENTRY_DTS        *wan_tx_pointers_entry_ptr;
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS      *wan_tx_queue_descriptor_ptr;
    uint32_t scheduler_id;
    uint16_t scheduler_address;
    uint16_t tx_queue_index;
    int      queue_offset, i;

    wan_tx_pointers_entry_ptr = &( wan_tx_pointers_table_ptr->entry[ xi_wan_channel_id ][ xi_rate_controller_id ][ xi_queue_id ] );

    /* verify that the wan channel queue was configured before */
    if ( wan_tx_pointers_entry_ptr->wan_tx_queue_ptr == 0 )
        return BL_LILAC_RDD_ERROR_GPON_TX_QUEUE_NOT_CONFIGURED;

    wan_tx_queue_descriptor_ptr = ( RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + wan_tx_pointers_entry_ptr->wan_tx_queue_ptr - sizeof ( RUNNER_COMMON ) );

    /* allocate a us_service_queue */
    for ( scheduler_id = 0; scheduler_id < sizeof(g_us_service_queue_map)/sizeof(g_us_service_queue_map[0]); scheduler_id++ )
    {
        if (g_us_service_queue_map[ scheduler_id ] == xi_wan_channel_id ||
            g_us_service_queue_map[ scheduler_id ] == RDD_WAN_CHANNEL_UNASSIGNED)
        {
            g_us_service_queue_map[ scheduler_id ] = xi_wan_channel_id;
            break;
        }
    }

    if ( scheduler_id >= sizeof(g_us_service_queue_map)/sizeof(g_us_service_queue_map[0]) )
        return BL_LILAC_RDD_ERROR_RATE_CONTROLLERS_POOL_OVERFLOW;

    scheduler_address = WAN_TX_SERVICE_QUEUE_SCHEDULER_TABLE_ADDRESS + (scheduler_id * sizeof(RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR_DTS));

    /* configure wan_tx_queue to point to service queue scheduler */
    RDD_WAN_TX_QUEUE_DESCRIPTOR_USE_AS_SCHEDULER_WRITE ( enable, wan_tx_queue_descriptor_ptr );
    RDD_WAN_TX_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE ( scheduler_id, wan_tx_queue_descriptor_ptr );

    /* copy values from tx_queue to service queues */
    RDD_WAN_TX_QUEUE_DESCRIPTOR_INDEX_READ( tx_queue_index, wan_tx_pointers_entry_ptr );

    service_queues_table = RDD_WAN_TX_SERVICE_QUEUES_TABLE_PTR ();
    queue_offset         = scheduler_id * RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER;

    for ( i = 0; i < RDD_WAN_TX_SERVICE_QUEUE_SCHEDULER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER; i++ )
    {
        service_queue = &service_queues_table->entry[ queue_offset + i ];

        RDD_WAN_TX_SERVICE_QUEUE_DESCRIPTOR_INDEX_WRITE ( tx_queue_index, service_queue );
        RDD_WAN_TX_SERVICE_QUEUE_DESCRIPTOR_SVC_QUEUE_SCHED_PTR_WRITE ( scheduler_address, service_queue );
    }

#endif
    return ( BL_LILAC_RDD_OK );
}

void rdd_service_queue_cfg(rdd_service_queue_id_t queue_id, uint16_t pkt_threshold, bdmf_boolean rate_limit)
{
    RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_DTS *service_queue_descriptor_ptr;
    RDD_DDR_QUEUE_DESCRIPTOR_DTS            *service_queue_descriptor_entry_ptr;
    RDD_SERVICE_QUEUES_CFG_ENTRY_DTS        *service_tm_descriptor_ptr;
    uint32_t                                rate_limiter_status;
    uint32_t                                sustain_vector;

    service_queue_descriptor_ptr = RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_PTR ();
    service_queue_descriptor_entry_ptr = &( service_queue_descriptor_ptr->entry[ queue_id ] );
    service_tm_descriptor_ptr = (RDD_SERVICE_QUEUES_CFG_ENTRY_DTS *)(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + SERVICE_QUEUES_CFG_ADDRESS);

    RDD_DDR_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE ( pkt_threshold, service_queue_descriptor_entry_ptr );

    if ( !rate_limit )
    {
        RDD_DDR_QUEUE_DESCRIPTOR_RATE_CONTROLLER_ID_WRITE ( SERVICE_QUEUE_RATE_LIMITER_IDLE, service_queue_descriptor_entry_ptr );

        RDD_SERVICE_QUEUES_CFG_ENTRY_RATE_LIMITER_STATUS_READ ( rate_limiter_status, service_tm_descriptor_ptr );
        rate_limiter_status |= ( 1<<queue_id );
        RDD_SERVICE_QUEUES_CFG_ENTRY_RATE_LIMITER_STATUS_WRITE ( rate_limiter_status, service_tm_descriptor_ptr );

        RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_VECTOR_READ ( sustain_vector, service_tm_descriptor_ptr );
        sustain_vector |= ( 1<<queue_id );
        RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_VECTOR_WRITE ( sustain_vector, service_tm_descriptor_ptr );
    }
    else
    {
        RDD_DDR_QUEUE_DESCRIPTOR_RATE_CONTROLLER_ID_WRITE ( queue_id, service_queue_descriptor_entry_ptr );
    }
}

void rdd_service_queue_addr_cfg (rdd_service_queue_id_t queue_idx, uint32_t ddr_address, uint16_t queue_size)
{
    RDD_SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_DTS  *service_queue_addr_ptr;
    RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS                 *service_queue_addr_entry;

    service_queue_addr_ptr = RDD_SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_PTR();
    service_queue_addr_entry = ( RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS * ) &service_queue_addr_ptr->entry[ queue_idx ];

    /* set DDR q base address for each queue */
    RDD_DDR_QUEUE_ADDRESS_ENTRY_ADDR_WRITE ( ddr_address, service_queue_addr_entry );
    RDD_DDR_QUEUE_ADDRESS_ENTRY_SIZE_WRITE ( queue_size, service_queue_addr_entry );
}


int rdd_service_queue_status_get(rdd_service_queue_id_t queue_id, rdpa_stat_1way_t *stat)
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error = BL_LILAC_RDD_OK;
    uint16_t tmp;

    rdd_error = rdd_4_bytes_counter_get ( SERVICE_QUEUE_PACKET_GROUP, queue_id,
#if defined(OREN)
                                          LILAC_RDD_TRUE,
#endif
                                          &stat->passed.packets );
    if ( rdd_error != BL_LILAC_RDD_OK )
        return ( rdd_error );

    rdd_error = rdd_2_bytes_counter_get ( SERVICE_QUEUE_DROP_PACKET_GROUP, queue_id,
#if defined(OREN)
                                          LILAC_RDD_TRUE,
#endif
                                          &tmp );
    stat->discarded.packets = tmp;
    return ( rdd_error );
}


void rdd_service_queue_overall_rate_limiter_enable(bdmf_boolean enable)
{
    RDD_SERVICE_QUEUES_CFG_ENTRY_DTS  *service_tm_descriptor_ptr;
    service_tm_descriptor_ptr = ( RDD_SERVICE_QUEUES_CFG_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + SERVICE_QUEUES_CFG_ADDRESS );
    RDD_SERVICE_QUEUES_CFG_ENTRY_OVERALL_RATE_LIMITER_MODE_WRITE ( enable, service_tm_descriptor_ptr );
}

void rdd_service_queue_overall_rate_limiter_cfg(rdd_rate_cntrl_params_t *budget)
{
    rdd_service_queue_rate_limiter_cfg(SERVICE_QUEUE_RATE_LIMITER_OVERALL, budget);
}


void rdd_service_queue_rate_limiter_cfg(uint32_t rate_limiter, rdd_rate_cntrl_params_t *budget)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET          runner_timer_target_register;
#endif
    RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_DTS      *rate_limiters_table_ptr;
    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_DTS *rate_limiter_descriptor_ptr;
    RDD_SERVICE_QUEUES_CFG_ENTRY_DTS               *service_tm_descriptor_ptr;
    RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS  *exponent_table_ptr;
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_DTS  *exponent_entry_ptr;
    uint32_t                                rate_limiter_status;
    uint32_t                                exponent_list[ RDD_RATE_CONTROL_EXPONENT_NUM ];
    uint32_t                                exponent_table_index;
    uint32_t                                peak_budget_exponent;
    uint32_t                                peak_limit_exponent;
    uint32_t                                sustain_budget;
    uint32_t                                peak_budget;
    uint32_t                                peak_limit;
    static uint32_t                         api_first_time_call_service_queue_rate_limiter = LILAC_RDD_TRUE;

    rate_limiters_table_ptr = RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_PTR();

    if (rate_limiter == SERVICE_QUEUE_RATE_LIMITER_OVERALL)
        rate_limiter_descriptor_ptr = ( RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + SERVICE_QUEUES_OVERALL_RATE_LIMITER_ADDRESS);
    else
        rate_limiter_descriptor_ptr = &rate_limiters_table_ptr->entry[ rate_limiter ];

    MEMSET ( rate_limiter_descriptor_ptr, 0, sizeof ( RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_DTS ) );

    /* read exponents table */
#if defined(OREN)
    exponent_table_ptr = RDD_RATE_CONTROLLER_EXPONENT_TABLE_PTR();
#else
    exponent_table_ptr = (RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS *)( DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS );
#endif

    for ( exponent_table_index = 0; exponent_table_index < RDD_RATE_CONTROL_EXPONENT_NUM; exponent_table_index++ )
    {
        exponent_entry_ptr = &( exponent_table_ptr->entry[ exponent_table_index ] );
        RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_READ ( exponent_list[ exponent_table_index ], exponent_entry_ptr );
    }

    /* convert sustain rate to allocation units */
    sustain_budget = rdd_budget_to_alloc_unit( budget->sustain_budget, SERVICE_QUEUE_RATE_LIMITER_TIMER_PERIOD, 0 );

    /* convert peak budget to allocation unit and divide to exponent and mantissa */
    peak_budget = rdd_budget_to_alloc_unit( budget->peak_budget.rate, SERVICE_QUEUE_RATE_LIMITER_TIMER_PERIOD, 0 );
    peak_budget_exponent = rdd_get_exponent( peak_budget, 14, RDD_RATE_CONTROL_EXPONENT_NUM, exponent_list );
    peak_budget >>= exponent_list[ peak_budget_exponent ];

    /* convert peak limit to allocation unit and divide to exponent and mantissa */
    peak_limit = rdd_budget_to_alloc_unit( budget->peak_budget.limit, SERVICE_QUEUE_RATE_LIMITER_TIMER_PERIOD, 0 );
    peak_limit_exponent = rdd_get_exponent( peak_limit, 14, RDD_RATE_CONTROL_EXPONENT_NUM, exponent_list );
    peak_limit >>= exponent_list[ peak_limit_exponent ];

    /* write rate control parameters to descriptor */
    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_EXPONENT_WRITE ( peak_budget_exponent, rate_limiter_descriptor_ptr );
    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_WRITE ( peak_budget, rate_limiter_descriptor_ptr );
    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BUDGET_LIMIT_EXPONENT_WRITE ( peak_limit_exponent, rate_limiter_descriptor_ptr );
    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BUDGET_LIMIT_WRITE ( peak_limit, rate_limiter_descriptor_ptr );
    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_SUSTAIN_BUDGET_WRITE ( sustain_budget, rate_limiter_descriptor_ptr );
    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_WEIGHT_WRITE ( budget->peak_weight >> 8, rate_limiter_descriptor_ptr );

    RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BURST_FLAG_WRITE ( LILAC_RDD_ON, rate_limiter_descriptor_ptr );

    service_tm_descriptor_ptr = ( RDD_SERVICE_QUEUES_CFG_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + SERVICE_QUEUES_CFG_ADDRESS );
    if (rate_limiter == SERVICE_QUEUE_RATE_LIMITER_OVERALL)
    {
        RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_RATE_LIMITER_MASK_WRITE ( 0, rate_limiter_descriptor_ptr );

        /* set overall burst rate limiting mode */
        RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_SCHEDULING_MODE_WRITE ( 0, service_tm_descriptor_ptr );
        RDD_SERVICE_QUEUES_CFG_ENTRY_PEAK_SCHEDULING_MODE_WRITE ( 0, service_tm_descriptor_ptr );
    }
    else
    {
        /* initialize the hardcoded parameters of the rate controller descriptor */
        RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_RATE_LIMITER_MASK_WRITE ( ( 1 << rate_limiter ), rate_limiter_descriptor_ptr );

        RDD_SERVICE_QUEUES_CFG_ENTRY_RATE_LIMITER_STATUS_READ ( rate_limiter_status, service_tm_descriptor_ptr );
        rate_limiter_status |= ( 1 << rate_limiter );
        RDD_SERVICE_QUEUES_CFG_ENTRY_RATE_LIMITER_STATUS_WRITE ( rate_limiter_status, service_tm_descriptor_ptr );
    }

    if ( api_first_time_call_service_queue_rate_limiter )
    {
#if !defined(FIRMWARE_INIT)
        RUNNER_REGS_0_CFG_TIMER_TARGET_READ ( runner_timer_target_register );
        runner_timer_target_register.timer_4_6 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_4_6_PICO_CORE_VALUE;
        RUNNER_REGS_0_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );

        rdd_timer_task_config ( rdpa_dir_ds, SERVICE_QUEUE_RATE_LIMITER_TIMER_PERIOD, DOWNSTREAM_SERVICE_QUEUES_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID );
#else
        rdd_timer_task_config ( rdpa_dir_ds, 100, DOWNSTREAM_SERVICE_QUEUES_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID ); 
#endif

        api_first_time_call_service_queue_rate_limiter = LILAC_RDD_FALSE;
    }
}


#else /* G9991 */
void rdd_service_queues_initialize ( void ) { }
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_svcq_scheduler_set ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                             BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                             BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                             bdmf_boolean                         enable )
{
   return BL_LILAC_RDD_OK;
}
void rdd_service_queue_cfg(rdd_service_queue_id_t queue_id, uint16_t pkt_threshold, bdmf_boolean rate_limit) { }
void rdd_service_queue_addr_cfg (rdd_service_queue_id_t queue_idx, uint32_t ddr_address, uint16_t queue_size) { }
int rdd_service_queue_status_get(rdd_service_queue_id_t queue_id, rdpa_stat_1way_t *stat) { return BL_LILAC_RDD_OK; }
void rdd_service_queue_overall_rate_limiter_enable(bdmf_boolean enable) { }
void rdd_service_queue_overall_rate_limiter_cfg(rdd_rate_cntrl_params_t *budget) { }
void rdd_service_queue_rate_limiter_cfg(uint32_t rate_limiter, rdd_rate_cntrl_params_t *budget) { }
#endif /* G9991 */

