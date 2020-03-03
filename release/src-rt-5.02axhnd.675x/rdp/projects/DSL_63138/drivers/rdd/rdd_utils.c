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
#include "rdd_utils.h"

/******************************************************************************/
/*                                                                            */
/*                            Global Variables                           */
/*                                                                            */
/******************************************************************************/
char rnr_a_task_names[MAX_RNR_THREADS][MAX_THREAD_NAME_SIZE] = {
    [CPU_TX_FAST_THREAD_NUMBER] = "CPU_TX_FAST",
    [CPU_RX_THREAD_NUMBER] = "CPU_RX",
    [2] = "2",
    [3] = "3",
    [TIMER_SCHEDULER_MAIN_THREAD_NUMBER] = "TIMER_SCHEDULER_MAIN",
    [POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER] = "POLICER_BUDGET_ALLOCATOR",
    [6] = "6",
#if defined(DSL_63138) || defined(DSL_63148)
    [WAN_DIRECT_THREAD_NUMBER] = "WAN_DIRECT",
#else
    [7] = "7",
#endif
    [WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER] = "WAN1_FILTERS_AND_CLASSIFICATION",
    [WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER] = "WAN_FILTERS_AND_CLASSIFICATION",
    [ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER] = "ETHWAN2_FILTERS_AND_CLASSIFICATION",
    [DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER] = "DOWNSTREAM_FLOW_CACHE_SLAVE0",
    [DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER] = "DOWNSTREAM_FLOW_CACHE_SLAVE1",
    [DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER] = "DOWNSTREAM_FLOW_CACHE_SLAVE2",
    [DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER] = "DOWNSTREAM_FLOW_CACHE_SLAVE3",
    [15] = "15",
    [DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER] = "DHD_TX_COMPLETE_FAST_A",
    [DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER] = "DHD1_TX_COMPLETE_FAST_A",
    [DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER] = "DHD2_TX_COMPLETE_FAST_A",
    [DHD_TX_POST_FAST_A_THREAD_NUMBER] = "DHD_TX_POST_FAST_A",
    [20] = "20",
    [21] = "21",
    [22] = "22",
    [23] = "23",
    [24] = "24",
#if defined(DSL_63138) || defined(DSL_63148)
    [CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER] = "CPU_DS_FILTERS_AND_CLASSIFICATION",
#else
    [25] = "25",
#endif
    [26] = "26",
    [27] = "27",
    [DOWNSTREAM_MULTICAST_THREAD_NUMBER] = "DOWNSTREAM_MULTICAST",
    [FREE_SKB_INDEX_FAST_THREAD_NUMBER] = "FREE_SKB_INDEX_FAST",
    [IPSEC_DOWNSTREAM_THREAD_NUMBER] = "IPSEC_DOWNSTREAM",
    [31] = "31",
    [CPU_TX_PICO_THREAD_NUMBER] = "CPU_TX_PICO",
    [GSO_PICO_THREAD_NUMBER] = "GSO_PICO",
    [TIMER_SCHEDULER_PICO_A_THREAD_NUMBER] = "TIMER_SCHEDULER_PICO_A",
#if defined(WL4908)
    [DS_RX_BUFFER_COPY_THREAD_NUMBER] = "DS_RX_BUFFER_COPY",
#else
    [35] = "35",
#endif
    [WLAN_MCAST_THREAD_NUMBER] = "WLAN_MCAST",
    [DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER] = "DOWNSTREAM_LAN_ENQUEUE",
    [DS_TIMER_7_THREAD_NUMBER] = "DS_TIMER_7",
    [CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER] = "CPU_RX_INTERRUPT_COALESCING",
    [FREE_SKB_INDEX_PICO_A_THREAD_NUMBER] = "FREE_SKB_INDEX_PICO_A",
    [LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER] = "LOCAL_SWITCHING_LAN_ENQUEUE",
    [ETH_TX_THREAD_NUMBER] = "ETH_TX",
    [43] = "43",
    [SERVICE_QUEUE_ENQUEUE_THREAD_NUMBER] = "SERVICE_QUEUE_ENQUEUE",
    [SERVICE_QUEUE_DEQUEUE_THREAD_NUMBER] = "SERVICE_QUEUE_DEQUEUE",
    [46] = "46",
    [DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER] = "DOWNSTREAM_MULTICAST_LAN_ENQUEUE" };


char rnr_b_task_names[MAX_RNR_THREADS][MAX_THREAD_NAME_SIZE] = {
    [CPU_TX_FAST_THREAD_NUMBER] = "CPU_TX_FAST",
    [CPU_RX_THREAD_NUMBER] = "CPU_RX",
    [2] = "2",
    [RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER] = "RATE_CONTROLLER_BUDGET_ALLOCATOR",
    [TIMER_SCHEDULER_MAIN_THREAD_NUMBER] = "TIMER_SCHEDULER_MAIN",
    [POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER] = "POLICER_BUDGET_ALLOCATOR",
    [WAN1_TX_THREAD_NUMBER] = "WAN1_TX",
#if defined(DSL_63138) || defined(DSL_63148)
    [WAN_TX_THREAD_NUMBER] = "WAN_TX",
#else
    [7] = "7",
#endif
    [DHD_TX_POST_FAST_B_THREAD_NUMBER] = "DHD_TX_POST_FAST_B",
#if defined(WL4908)
    [US_RX_BUFFER_COPY_THREAD_NUMBER] = "US_RX_BUFFER_COPY",
    [US_RX_BUFFER_COPY1_THREAD_NUMBER] = "US_RX_BUFFER_COPY1",
    [US_RX_BUFFER_COPY2_THREAD_NUMBER] = "US_RX_BUFFER_COPY2",
#else
    [9] = "9",
    [10] = "10",
    [11] = "11",
#endif
    [12] = "12",
    [13] = "13",
    [14] = "14",
    [15] = "15",
    [16] = "16",
    [17] = "17",
    [18] = "18",
    [19] = "19",
    [20] = "20",
    [WAN_ENQUEUE_THREAD_NUMBER] = "WAN_ENQUEUE",
    [22] = "22",
    [23] = "23",
    [24] = "24",
    [25] = "25",
    [26] = "26",
    [US_TIMER_7_THREAD_NUMBER] = "US_TIMER_7",
    [28] = "28",
    [FREE_SKB_INDEX_FAST_THREAD_NUMBER] = "FREE_SKB_INDEX_FAST",
    [30] = "30",
    [31] = "31",
    [UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER] = "UPSTREAM_FLOW_CACHE_SLAVE0",
    [UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER] = "UPSTREAM_FLOW_CACHE_SLAVE1",
    [UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER] = "UPSTREAM_FLOW_CACHE_SLAVE2",
    [UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER] = "UPSTREAM_FLOW_CACHE_SLAVE3",
    [FREE_SKB_INDEX_PICO_B_THREAD_NUMBER] = "FREE_SKB_INDEX_PICO_B",
    [TIMER_SCHEDULER_PICO_B_THREAD_NUMBER] = "TIMER_SCHEDULER_PICO_B",
    [CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER] = "CPU_US_FILTERS_AND_CLASSIFICATION",
#if defined(DSL_63138) || defined(DSL_63148)
    [LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER] = "LAN1_FILTERS_AND_CLASSIFICATION",
#else
    [39] = "39",
#endif
    [DHD_RX_THREAD_NUMBER] = "DHD_RX",
    [DHD1_RX_THREAD_NUMBER] = "DHD1_RX",
    [DHD2_RX_THREAD_NUMBER] = "DHD2_RX",
    [LAN_DISPATCH_THREAD_NUMBER] = "LAN_DISPATCH",
#if defined(WL4908)
    [LAN1_DISPATCH_THREAD_NUMBER] = "LAN1_DISPATCH",
    [LAN2_DISPATCH_THREAD_NUMBER] = "LAN2_DISPATCH",
#else
    [44] = "44",
    [45] = "45",
#endif
    [46] = "46",
    [47] = "47" };

char rdp_fw_trace_events[MAX_RNR_EVENTS][MAX_EVENT_NAME_SIZE] = {
                          "",
                          "THREAD_ENTRY",
                          "THREAD_EXIT",
                          "DMA_RD",
                          "DMA_RD_RET",
                          "DMA_WR",
                          "DMA_WR_RET",
                          "THREAD_EXIT_2",
                          "THREAD_EXIT_3",
                          "THREAD_EXIT_4",
                          "THREAD_EXIT_5",
                          "THREAD_EXIT_6",
                          "THREAD_EXIT_7",
                          "THREAD_EXIT_8",
                          "DMA_RD_2",
                          "DMA_RD_RET_2",
                          };



/******************************************************************************/
/*  RDD Support Functions for FW Trace                         */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE f_rdd_fwtrace_enable_set( uint32_t enable )
{
#ifdef RUNNER_FWTRACE
    volatile uint32_t *main_a_fwtrace_offset_ptr;
    volatile uint32_t *pico_a_fwtrace_offset_ptr;
    volatile uint32_t *main_b_fwtrace_offset_ptr;
    volatile uint32_t *pico_b_fwtrace_offset_ptr;

    main_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS );
    pico_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + (RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS+2) );
    main_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS );
    pico_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + (RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS+2) );

    if (enable)
    {
        /* Set enable bit in offset registers and clear remaining (used as write pointer) */
        *main_a_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED_CLUSTER;
        //*pico_a_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
        *main_b_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED_CLUSTER;
        //*pico_b_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
    }
    else
    {
        /* Clear enable bit and remaining portion of write pointer*/
        *main_a_fwtrace_offset_ptr &= ~RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
        *pico_a_fwtrace_offset_ptr &= ~RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
        *main_b_fwtrace_offset_ptr &= ~RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
        *pico_b_fwtrace_offset_ptr &= ~RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
    }
#endif
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE f_rdd_fwtrace_clear ( void )
{
#ifdef RUNNER_FWTRACE
    volatile uint32_t *main_a_fwtrace_buf_ptr;
    volatile uint32_t *pico_a_fwtrace_buf_ptr;
    volatile uint32_t *main_b_fwtrace_buf_ptr;
    volatile uint32_t *pico_b_fwtrace_buf_ptr;
    volatile uint32_t *main_a_fwtrace_offset_ptr;
    volatile uint32_t *pico_a_fwtrace_offset_ptr;
    volatile uint32_t *main_b_fwtrace_offset_ptr;
    volatile uint32_t *pico_b_fwtrace_offset_ptr;
    unsigned int i;

    main_a_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET) + RUNNER_FWTRACE_MAINA_BASE_ADDRESS);
    pico_a_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET) + RUNNER_FWTRACE_PICOA_BASE_ADDRESS);
    main_b_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET) + RUNNER_FWTRACE_MAINB_BASE_ADDRESS);
    pico_b_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET) + RUNNER_FWTRACE_PICOB_BASE_ADDRESS);

    // Zero out memory
    for (i=0;i<RDD_RUNNER_FWTRACE_MAINA_BASE_SIZE;i++)
    {
        main_a_fwtrace_buf_ptr[i] = 0;
    }

    for (i=0;i<RDD_RUNNER_FWTRACE_PICOA_BASE_SIZE;i++)
    {
        pico_a_fwtrace_buf_ptr[i] = 0;
    }

    for (i=0;i<RDD_RUNNER_FWTRACE_MAINB_BASE_SIZE;i++)
    {
        main_b_fwtrace_buf_ptr[i] = 0;
    }

    for (i=0;i<RDD_RUNNER_FWTRACE_PICOB_BASE_SIZE;i++)
    {
        pico_b_fwtrace_buf_ptr[i] = 0;
    }

    main_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS );
    pico_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + (RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS+2) );
    main_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS );
    pico_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + (RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS+2) );

    /* Clear enable bit and remaining portion of write pointer*/
    *main_a_fwtrace_offset_ptr = 0;
    *pico_a_fwtrace_offset_ptr = 0;
    *main_b_fwtrace_offset_ptr = 0;
    *pico_b_fwtrace_offset_ptr = 0;
#endif
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE f_rdd_fwtrace_get ( LILAC_RDD_RUNNER_INDEX_DTS runner_id,
                                                      uint32_t *trace_length,
                                                      uint32_t *trace_buffer )
{
#ifdef RUNNER_FWTRACE
    volatile uint32_t *fwtrace_buf_ptr;
    volatile uint16_t *fwtrace_offset_ptr;
    uint32_t i;

    switch (runner_id)
    {
        case FAST_RUNNER_A:
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET) + RUNNER_FWTRACE_MAINA_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS );
            break;
        case PICO_RUNNER_A:
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET) + RUNNER_FWTRACE_PICOA_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + (RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS+2) );
            break;
        case FAST_RUNNER_B:
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET) + RUNNER_FWTRACE_MAINB_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS );
            break;
        case PICO_RUNNER_B:
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET) + RUNNER_FWTRACE_PICOB_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + (RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS+2) );
            break;
        default:
            return (BL_LILAC_RDD_ERROR_ILLEGAL_DIRECTION);
    }

    *trace_length = (ntohs(*fwtrace_offset_ptr) & 0x7FFF);
    for (i=0;i<*trace_length;i++)
    {
        trace_buffer[i] = fwtrace_buf_ptr[i];
    }
#endif
    return ( BL_LILAC_RDD_OK );
}


