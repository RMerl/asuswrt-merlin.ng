/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
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
char RnrATaskNames[MAX_RNR_THREADS][MAX_THREAD_NAME_SIZE] = {
    [CPU_TX_FAST_THREAD_NUMBER] = "CPU_TX_FAST", 
    [CPU_RX_THREAD_NUMBER] = "CPU_RX",
    [2] = "2",
    [DHD_TX_POST_FAST_A_THREAD_NUMBER] = "DHD_TX_POST_FAST_A", 
    [TIMER_SCHEDULER_MAIN_THREAD_NUMBER] = "TIMER_SCHEDULER_MAIN",
    [POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER] = "POLICER_BUDGET_ALLOCATOR",
    [6] = "6", 
    [7] = "7", 
    [8] = "8", 
    [9] = "9", 
    [10] = "10", 
    [11] = "11", 
    [12] = "12", 
    [13] = "13", 
    [14] = "14", 
    [15] = "15", 
    [DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER] = "DHD_TX_COMPLETE_FAST_A",
    [DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER] = "DHD1_TX_COMPLETE_FAST_A",
    [DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER] = "DHD2_TX_COMPLETE_FAST_A",
    [19] = "19", 
    [20] = "20", 
    [21] = "21", 
    [22] = "22", 
    [23] = "23", 
    [DS_PROCESSING_0_THREAD_NUMBER] = "DS_PROCESSING_0",
    [DS_PROCESSING_1_THREAD_NUMBER] = "DS_PROCESSING_1",
    [DS_PROCESSING_2_THREAD_NUMBER] = "DS_PROCESSING_2",
    [DS_PROCESSING_3_THREAD_NUMBER] = "DS_PROCESSING_3",
    [DOWNSTREAM_MULTICAST_THREAD_NUMBER] = "DOWNSTREAM_MULTICAST",
    [FREE_SKB_INDEX_FAST_THREAD_NUMBER] = "FREE_SKB_INDEX_FAST", 
    [IPSEC_DOWNSTREAM_THREAD_NUMBER] = "IPSEC_DOWNSTREAM", 
    [31] = "31", 
    [CPU_TX_PICO_THREAD_NUMBER] = "CPU_TX_PICO", 
    [GSO_PICO_THREAD_NUMBER] = "GSO_PICO", 
    [TIMER_SCHEDULER_PICO_THREAD_NUMBER] = "TIMER_SCHEDULER_PICO",
    [RATE_SHAPER_BUDGET_ALLOCATOR_THREAD_NUMBER] = "RATE_SHAPER_BUDGET_ALLOCATOR",
    [LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER] = "LOCAL_SWITCHING_LAN_ENQUEUE",
    [DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER] = "DOWNSTREAM_LAN_ENQUEUE",
    [DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER] = "DOWNSTREAM_MULTICAST_LAN_ENQUEUE",
    [CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER] = "CPU_RX_INTERRUPT_COALESCING",
    [FREE_SKB_INDEX_PICO_A_THREAD_NUMBER] = "FREE_SKB_INDEX_PICO_A",
    [WLAN_MCAST_THREAD_NUMBER] = "WLAN_MCAST",
    [LAN_TX_THREAD_NUMBER] = "LAN_TX",
    [43] = "43", 
    [44] = "44", 
    [45] = "45", 
    [46] = "46", 
    [DS_SPDSVC_THREAD_NUMBER] = "DS_SPDSVC"};

                    
char RnrBTaskNames[MAX_RNR_THREADS][MAX_THREAD_NAME_SIZE] = {
    [CPU_TX_FAST_THREAD_NUMBER] = "CPU_TX_FAST", 
    [CPU_RX_THREAD_NUMBER] = "CPU_RX", 
    [2] = "2",
    [RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER] = "RATE_CONTROLLER_BUDGET_ALLOCATOR", 
    [TIMER_SCHEDULER_MAIN_THREAD_NUMBER] = "TIMER_SCHEDULER_MAIN",
    [POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER] = "POLICER_BUDGET_ALLOCATOR",
    [WAN1_TX_THREAD_NUMBER] = "WAN1_TX",
    [7] = "7",
    [DHD_TX_POST_FAST_B_THREAD_NUMBER] = "DHD_TX_POST_FAST_B", 
    [9] = "9", 
    [10] = "10", 
    [11] = "11", 
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
    [US_SPDSVC_THREAD_NUMBER] = "US_SPDSVC",
    [28] = "28", 
    [FREE_SKB_INDEX_FAST_THREAD_NUMBER] = "FREE_SKB_INDEX_FAST",
    [30] = "30",
    [31] = "31",
    [32] = "32",
    [33] = "33",
    [TIMER_SCHEDULER_PICO_THREAD_NUMBER] = "",
    [FREE_SKB_INDEX_PICO_B_THREAD_NUMBER] = "",
    [36] = "36",
    [CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER] = "CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER",
    [DHD_RX_THREAD_NUMBER] = "DHD_RX",
    [DHD1_RX_THREAD_NUMBER] = "DHD1_RX",
    [DHD2_RX_THREAD_NUMBER] = "DHD2_RX",
    [US_PROCESSING_0_THREAD_NUMBER] = "US_PROCESSING_0_THREAD_NUMBER",
    [US_PROCESSING_1_THREAD_NUMBER] = "US_PROCESSING_1_THREAD_NUMBER",
    [US_PROCESSING_2_THREAD_NUMBER] = "US_PROCESSING_2_THREAD_NUMBER",
    [US_PROCESSING_3_THREAD_NUMBER] = "US_PROCESSING_3_THREAD_NUMBER",
    [LAN0_RX_DISPATCH_THREAD_NUMBER] = "LAN0_RX_DISPATCH_THREAD_NUMBER",
    [LAN1_RX_DISPATCH_THREAD_NUMBER] = "LAN1_RX_DISPATCH_THREAD_NUMBER",
    [LAN2_RX_DISPATCH_THREAD_NUMBER] = "LAN2_RX_DISPATCH_THREAD_NUMBER"};

char rdpFwTraceEvents[MAX_RNR_EVENTS][MAX_EVENT_NAME_SIZE] = {
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
int f_rdd_fwtrace_enable_set( uint32_t enable )
{
#ifdef RUNNER_FWTRACE
    volatile uint32_t *main_a_fwtrace_offset_ptr;
    volatile uint32_t *pico_a_fwtrace_offset_ptr;
    volatile uint32_t *main_b_fwtrace_offset_ptr;
    volatile uint32_t *pico_b_fwtrace_offset_ptr;

    main_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS );
    pico_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_PICOA_CURR_OFFSET_ADDRESS );
    main_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS );
    pico_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_PICOB_CURR_OFFSET_ADDRESS );

    if (enable)
    {
        /* Set enable bit in offset registers and clear remaining (used as write pointer) */
        *main_a_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
        *pico_a_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
        *main_b_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
        *pico_b_fwtrace_offset_ptr = RUNNER_FWTRACE_ENABLE_MASK_SWAPPED;
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
    return ( BDMF_ERR_OK );
}

int f_rdd_fwtrace_clear ( void )
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
    
    main_a_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET) + RUNNER_FWTRACE_MAINA_BASE_ADDRESS);
    pico_a_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET) + RUNNER_FWTRACE_PICOA_BASE_ADDRESS);
    main_b_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET) + RUNNER_FWTRACE_MAINB_BASE_ADDRESS);
    pico_b_fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET) + RUNNER_FWTRACE_PICOB_BASE_ADDRESS);

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

    
    main_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS );
    pico_a_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_PICOA_CURR_OFFSET_ADDRESS );
    main_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS );
    pico_b_fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_PICOB_CURR_OFFSET_ADDRESS );

    /* Clear enable bit and remaining portion of write pointer*/
    *main_a_fwtrace_offset_ptr = 0;
    *pico_a_fwtrace_offset_ptr = 0;
    *main_b_fwtrace_offset_ptr = 0;
    *pico_b_fwtrace_offset_ptr = 0;
#endif
    return ( BDMF_ERR_OK );
}

int f_rdd_fwtrace_get ( rdd_runner_index_t runner_id,
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
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET) + RUNNER_FWTRACE_MAINA_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS );
            break;
        case PICO_RUNNER_A:
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET) + RUNNER_FWTRACE_PICOA_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_PICOA_CURR_OFFSET_ADDRESS );
            break;
        case FAST_RUNNER_B:
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET) + RUNNER_FWTRACE_MAINB_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS );
            break;
        case PICO_RUNNER_B:
            fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET) + RUNNER_FWTRACE_PICOB_BASE_ADDRESS);
            fwtrace_offset_ptr = ( volatile uint16_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_PICOB_CURR_OFFSET_ADDRESS );
            break;
        default:
            return (BDMF_ERR_PARM);
    }

    *trace_length = (ntohs(*fwtrace_offset_ptr) & 0x7FFF);
    for (i=0;i<*trace_length;i++)
    {         
        trace_buffer[i] = fwtrace_buf_ptr[i];
    }
#endif
    return ( BDMF_ERR_OK );
}


