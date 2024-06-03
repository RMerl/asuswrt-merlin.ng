// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
     
*/

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Lilac BPM driver              */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/


#include "packing.h"
#include "rdp_subsystem_common.h"
#include "rdp_drv_sbpm.h"


/******************************************************************************/
/*                                                                            */
/* Default values definitions                                                 */
/*                                                                            */
/******************************************************************************/

#define  CS_DRV_SBPM_MAX_NUMBER_OF_BUFFERS        ( 0x3FF )
#define  CS_DRV_SBPM_THRESHOLD_MASK               ( 0x3FF )
#define  CS_DRV_SBPM_NULL_VALUE                   ( 0x3FF )
#define  CS_DRV_SBPM_UG_ALIGNMENT                 ( 4 )
#define  CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST  ( 10 )
#define  CS_DRV_SBPM_IIR_LOW_PART_MASK            ( 0x1ff )
#define  CS_DRV_SBPM_IIR_HIGH_PART_MASK           ( 0xffe00 )
#define  CS_DRV_SBPM_IIR_SHIFT                    ( 22 )
#define  CS_DRV_SBPM_MAX_MULTICAST_VALUE          ( 7 )
#define  CS_DRV_SBPM_SBPM_EXCL_MEM_GAP            ( SBPM_BLOCK_REGS_SBPM_UG1_EXCL_TRSH_ADDRESS - SBPM_BLOCK_REGS_SBPM_UG0_EXCL_TRSH_ADDRESS )
#define  CS_DRV_SBPM_SEARCH_DEPTH                 ( 0x2b )

/* Low */
#define CS_LOW      ( 0 )
/* High */
#define CS_HIGH     ( 1  )

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/

/* gets bit #i from a given number */
#define MS_DRV_SBPM_GET_BIT_I( number , i )   ( ( ( 1 << i ) & ( number ) ) >> i )

/******************************************************************************/
/*                                                                            */
/* static function declaration                                                */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Init & cleanup module, license                                             */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* API functions implementations                                              */
/*                                                                            */
/******************************************************************************/

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_init                                                   */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - BPM initialize                                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function initialize and sets general configuration of the SBPM block.  */
/* SBPM module initialization is made once in the system lifetime              */
/*                                                                             */
/* This API performs the following:                                            */
/*  1.    Disable all SBPM source ports.                                         */
/*  2.    Builds the free linked list using the HW accelerator.                  */
/*  3.    Sets SBPM configuration:                                               */
/*      a.    Global threshold.                                                  */
/*      b.    User group configuration (threshold, hysteresis, exclusive         */
/*          threshold/ hysteresis).                                            */
/*      c.    Sets route address of each source port.                            */
/*      d.    Source port to user group mapping.                                 */
/*                                                                             */
/* Input:                                                                        */
/*   xi_base_address - The start address of the BN list                        */
/*   xi_list_size  - The number of buffers in the list                         */
/*   xi_global_configuration - global threshold and hysteresis (struct)        */
/*   xi_user_group_configuration  - user groups threshold and hysteresis (struct)      */
/*   xi_replay_address - runner replay address                                  */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*       DRV_SBPM_ERROR_NO_ERROR - no error                            */
/*     DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAXIMUM_VALUE -        */
/*         the amount of buffers exceeds the maximum value                     */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_init ( uint16_t xi_base_address ,
                                            uint16_t xi_list_size,
                                            uint16_t xi_replay_address,
                                            const DRV_SBPM_GLOBAL_CONFIGURATION    * xi_global_configuration,
                                            const DRV_SBPM_USER_GROUPS_THRESHOLDS  * xi_user_group_configuration)

{
    SBPM_BLOCK_REGS_SBPM_SP_EN sbpm_sp_enable;
    SBPM_BLOCK_REGS_INIT_FREE_LIST  sbpm_init;
#ifndef __UBOOT__
    DRV_SBPM_ISR     default_interrupts_mask;
#endif
    DRV_SBPM_ERROR   error;
    SBPM_BLOCK_REGS_RADDR_0   raddr0;
    SBPM_BLOCK_REGS_RADDR_1   raddr1;
    SBPM_BLOCK_REGS_RADDR_2   raddr2;
    SBPM_BLOCK_REGS_RADDR_3   raddr3;
    uint8_t                     ug_index;
    DRV_SBPM_ERROR_HANDLING_PARAMETERS error_handling_params ;
    DRV_SBPM_RUNNER_MSG_CTRL_PARAMS runner_msg_ctrl_params ;
    

    /*Validate parameters*/
    if ( xi_base_address + xi_list_size > CS_DRV_SBPM_MAX_NUMBER_OF_BUFFERS )
    {
        return ( DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAXIMUM_VALUE );
    }


    /*disable all SBPM source ports*/
    SBPM_BLOCK_REGS_SBPM_SP_EN_READ( sbpm_sp_enable ) ;
    sbpm_sp_enable.rnra_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.rnrb_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.gpon_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth0_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth1_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth2_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth3_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth4_sp_en = DRV_SBPM_DISABLE;
    SBPM_BLOCK_REGS_SBPM_SP_EN_WRITE( sbpm_sp_enable ) ;


    /* Write to register init_free_list - Builds the free linked list using the HW accelerator */
    SBPM_BLOCK_REGS_INIT_FREE_LIST_READ(sbpm_init);

    sbpm_init.init_num_bn = xi_list_size;
    sbpm_init.init_head_bn_addr = xi_base_address;
    sbpm_init.bsy = SBPM_BLOCK_REGS_INIT_FREE_LIST_BSY_NO_REQUEST_VALUE;
    sbpm_init.rdy= SBPM_BLOCK_REGS_INIT_FREE_LIST_RDY_DEFAULT_VALUE;
    sbpm_init.init_en = DRV_SBPM_ENABLE;

    SBPM_BLOCK_REGS_INIT_FREE_LIST_WRITE(sbpm_init);

    /* Set BPM global configuration  */
    error = fi_bl_drv_sbpm_set_global_threshold ( xi_global_configuration->threshold , xi_global_configuration->hysteresis );

    /* Set User Group [0-7] threshold configuration*/
    for ( ug_index = 0 ; ug_index < DRV_SBPM_NUMBER_OF_USER_GROUPS  ; ug_index++  )
    {
            error = fi_bl_drv_sbpm_set_user_group_thresholds ( ug_index, &xi_user_group_configuration->ug_arr[ug_index] );
            if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
            {
                return ( error );
            }
    }

    /*     Set route address of each source port*/
    SBPM_BLOCK_REGS_RADDR_0_READ (raddr0 );
    raddr0.eth0_tx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH0_TX_RADDR_ETH0_TX_RADDR_VALUE;
    raddr0.eth0_rx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH0_RX_RADDR_ETH0_RX_RADDR_VALUE;
    raddr0.eth1_tx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH1_TX_RADDR_ETH1_TX_RADDR_VALUE ;
    raddr0.eth1_rx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH1_RX_RADDR_ETH1_RX_RADDR_VALUE ;
    SBPM_BLOCK_REGS_RADDR_0_WRITE( raddr0 );

    SBPM_BLOCK_REGS_RADDR_1_READ (raddr1 );
    raddr1.eth2_tx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH2_TX_RADDR_ETH2_TX_RADDR_VALUE ;
    raddr1.eth2_rx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH2_RX_RADDR_ETH2_RX_RADDR_VALUE ;
    raddr1.eth3_tx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH3_TX_RADDR_ETH3_TX_RADDR_VALUE;
    raddr1.eth3_rx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH3_RX_RADDR_ETH3_RX_ADDR_VALUE;
    SBPM_BLOCK_REGS_RADDR_1_WRITE( raddr1 );

    SBPM_BLOCK_REGS_RADDR_2_READ (raddr2 );
    raddr2.eth4_tx_raddr = SBPM_BLOCK_REGS_RADDR_2_ETH4_TX_RADDR_ETH4_TX_RADDR_VALUE ;
    raddr2.eth4_rx_raddr = SBPM_BLOCK_REGS_RADDR_2_ETH4_RX_RADDR_ETH4_RX_ADDR_VALUE ;
    raddr2.gpon_tx_raddr = SBPM_BLOCK_REGS_RADDR_2_GPON_TX_RADDR_GPON_TX_RADDR_VALUE;
    raddr2.gpon_rx_raddr = SBPM_BLOCK_REGS_RADDR_2_GPON_RX_RADDR_GPON_RX_RADDR_VALUE;
    SBPM_BLOCK_REGS_RADDR_2_WRITE( raddr2 );

    SBPM_BLOCK_REGS_RADDR_3_READ (raddr3 );
    raddr3.mipsd_raddr = SBPM_BLOCK_REGS_RADDR_3_MIPSD_RADDR_MIPSD_RADDR_VALUE ;
    raddr3.rnrb_raddr = SBPM_BLOCK_REGS_RADDR_3_RNRB_RADDR_RNRB_RADDR_VALUE ;
    raddr3.rnra_raddr = SBPM_BLOCK_REGS_RADDR_3_RNRA_RADDR_RNRA_RADDR_VALUE;
    SBPM_BLOCK_REGS_RADDR_3_WRITE( raddr3 );

    /* Each Source Port is mapped to specified UG. */
    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_EMAC0 , DRV_SBPM_USER_GROUP_0 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_EMAC1 , DRV_SBPM_USER_GROUP_1 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_EMAC2 , DRV_SBPM_USER_GROUP_2 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_EMAC3 , DRV_SBPM_USER_GROUP_3 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_EMAC4 , DRV_SBPM_USER_GROUP_4 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_USB0 , DRV_SBPM_USER_GROUP_4 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_USB1 , DRV_SBPM_USER_GROUP_4 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_PCI0 , DRV_SBPM_USER_GROUP_4 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_PCI1 , DRV_SBPM_USER_GROUP_4 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_GPON , DRV_SBPM_USER_GROUP_5 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_RNR_A , DRV_SBPM_USER_GROUP_6 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_RNR_B , DRV_SBPM_USER_GROUP_6 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_MIPS_C , DRV_SBPM_USER_GROUP_7 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_MIPS_D , DRV_SBPM_USER_GROUP_7 );
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    /*Enable Interrupts in interrupt enablae register*/
#ifndef __UBOOT__
    default_interrupts_mask.bac_underrun = DRV_SBPM_ENABLE;
    default_interrupts_mask.multicast_counter_overflow = DRV_SBPM_ENABLE;
    default_interrupts_mask.check_last_error = DRV_SBPM_ENABLE;
    default_interrupts_mask.max_search_error = DRV_SBPM_ENABLE;
    error = fi_bl_drv_sbpm_set_interrupt_enable_register ( &default_interrupts_mask);
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }
#endif
    /* Set Error Handling Parameters */
    error_handling_params.search_deapth = CS_DRV_SBPM_SEARCH_DEPTH ;
    error_handling_params.max_search_enable = DRV_SBPM_DISABLE ;
    error_handling_params.check_last_enable = DRV_SBPM_ENABLE ;
    error_handling_params.freeze_counters = DRV_SBPM_DISABLE ;

    error = fi_bl_drv_sbpm_set_error_handling_parameters ( & error_handling_params ) ;
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    /* Set Runner Message Control parameters */
    error = fi_bl_drv_sbpm_get_runner_msg_ctrl ( & runner_msg_ctrl_params ) ;
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    runner_msg_ctrl_params.runner_reply_target_address = xi_replay_address >> 3 ;

    error = fi_bl_drv_sbpm_set_runner_msg_ctrl ( & runner_msg_ctrl_params ) ;
    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
    {
        return ( error );
    }

    return ( DRV_SBPM_ERROR_NO_ERROR );

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_init );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_sp_enable                                                    */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Source Ports Enable                                            */
/*                                                                             */
/* Abstract:                                                                   */
/* Source Ports Enable                                                         */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_SP_EN                                                                   */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*    xi_sp_enable - source port enable of each port.                          */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                        */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                */
/*   DRV_SBPM_ERROR_INVALID_SOURCE_PORT - invalid source port            */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_sp_enable ( const DRV_SBPM_SP_ENABLE * xi_sp_enable )
{
    SBPM_BLOCK_REGS_SBPM_SP_EN sbpm_sp_enable ;

    SBPM_BLOCK_REGS_SBPM_SP_EN_READ( sbpm_sp_enable ) ;

    sbpm_sp_enable.rnra_sp_en = xi_sp_enable->rnra_sp_enable ;
    sbpm_sp_enable.rnrb_sp_en = xi_sp_enable->rnrb_sp_enable ;
    sbpm_sp_enable.eth0_sp_en = xi_sp_enable->eth0_sp_enable ;
    sbpm_sp_enable.eth1_sp_en = xi_sp_enable->eth1_sp_enable ;
    sbpm_sp_enable.eth2_sp_en = xi_sp_enable->eth2_sp_enable ;
    sbpm_sp_enable.eth3_sp_en = xi_sp_enable->eth3_sp_enable ;
    sbpm_sp_enable.eth4_sp_en = xi_sp_enable->eth4_sp_enable ;
    sbpm_sp_enable.gpon_sp_en = xi_sp_enable->gpon_or_eth5_sp_enable ;

    SBPM_BLOCK_REGS_SBPM_SP_EN_WRITE( sbpm_sp_enable) ;

    return ( DRV_SBPM_ERROR_NO_ERROR ) ;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_sp_enable ) ;

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_global_threshold                                   */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Set SBPM Global threshold                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function sets the global Threshold for Allocated Buffers.              */
/*                                                                             */
/* Input:                                                                       */
/*                                                                             */
/*   xi_global_threshold - Global Threshold for Allocated Buffers              */
/*   xi_global_hystersis - how many BNs need to free in order to get out from  */
/*                         global NACK state                                   */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                            */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_global_threshold ( uint16_t xi_global_threshold,
                                                                        uint16_t xi_global_hysteresis )
{
    SBPM_BLOCK_REGS_SBPM_GL_TRSH global_configuration;

    SBPM_BLOCK_REGS_SBPM_GL_TRSH_READ(global_configuration);

    global_configuration.gl_bah = ( xi_global_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK );
    global_configuration.gl_bat = ( xi_global_threshold  & CS_DRV_SBPM_THRESHOLD_MASK );

    SBPM_BLOCK_REGS_SBPM_GL_TRSH_WRITE(global_configuration);

    return ( DRV_SBPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_global_threshold );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_user_group_thresholds                              */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Set SBPM User Group threshold configuration                   */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function sets the threshold and hysteresis for a specific user group.  */
/*                                                                             */
/* Input:                                                                       */
/*    xi_user_group - user group                                                            */
/*  xi_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                            */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_user_group_thresholds (DRV_SBPM_USER_GROUP xi_user_group,
                                                                            const DRV_SBPM_USER_GROUP_CONFIGURATION * xi_configuration)
{
    DRV_SBPM_UG_THRESHOLD ug_configuration;
    DRV_SBPM_UG_THRESHOLD ug_exclusive_configuration;
    uint32_t ug_start_address;
    uint32_t ug_exclusive_start_address;

    if ( ! ME_DRV_SBPM_USER_GROUP_IN_RANGE ( xi_user_group ) )
    {
        return ( DRV_SBPM_ERROR_INVALID_USER_GROUP );
    }

    ug_start_address = ( uint32_t ) ( SBPM_BLOCK_REGS_SBPM_UG0_TRSH_ADDRESS + CS_DRV_SBPM_UG_ALIGNMENT * ( uint32_t ) xi_user_group );

    if ( xi_user_group == DRV_SBPM_USER_GROUP_0 )
    {
        ug_exclusive_start_address = ( uint32_t )  SBPM_BLOCK_REGS_SBPM_UG0_EXCL_TRSH_ADDRESS;
    }
    else
    {
        ug_exclusive_start_address = ( uint32_t ) ( SBPM_BLOCK_REGS_SBPM_UG0_EXCL_TRSH_ADDRESS + CS_DRV_SBPM_SBPM_EXCL_MEM_GAP + CS_DRV_SBPM_UG_ALIGNMENT * ( uint32_t ) ( xi_user_group - 1 ) );
    }

    READ_32( ug_start_address, ug_configuration);

    ug_configuration.ug_hysteresis = ( uint32_t ) ( xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK );
    ug_configuration.ug_threshold =  ( uint32_t ) ( xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK );

    WRITE_32( ug_start_address, ug_configuration);


    READ_32( ug_exclusive_start_address , ug_exclusive_configuration);

    ug_exclusive_configuration.ug_hysteresis = ( uint32_t ) ( xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK );
    ug_exclusive_configuration.ug_threshold =  ( uint32_t ) ( xi_configuration->exclusive_threshold  & CS_DRV_SBPM_THRESHOLD_MASK );

    WRITE_32( ug_exclusive_start_address , ug_exclusive_configuration);

    return ( DRV_SBPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_user_group_thresholds );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_user_group_mapping                                 */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Set User Group Mapping                                        */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function maps a User group for a specific Source port                  */
/*                                                                             */
/* Input:                                                                         */
/*    xi_source_port - One of SBPM source ports                                   */
/*  xi_user_group  - one of SBPM User group 0-7                                */
/*                                                                             */
/* Output:  error code                                                         */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                            */
/*     DRV_SBPM_ERROR_INVALID_SOURCE_PORT - invalid source port      */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_USR  xi_source_port,
                                                                          DRV_SBPM_USER_GROUP xi_user_group )
{
    SBPM_BLOCK_REGS_SBPM_UG_MAP_0  sbpm_ug_mapping_r0;
    SBPM_BLOCK_REGS_UG_MAP_REG_1  sbpm_ug_mapping_r1;

    SBPM_BLOCK_REGS_SBPM_UG_MAP_0_READ(sbpm_ug_mapping_r0);
    SBPM_BLOCK_REGS_UG_MAP_REG_1_READ (sbpm_ug_mapping_r1);

    if ( ! ME_DRV_SBPM_USER_GROUP_IN_RANGE ( xi_user_group ) )
    {
        return ( DRV_SBPM_ERROR_INVALID_USER_GROUP );
    }

    switch ( xi_source_port )
    {
    case DRV_SBPM_SP_MIPS_C:
        sbpm_ug_mapping_r0.cpu = xi_user_group;
        break ;
    case DRV_SBPM_SP_EMAC0:
        sbpm_ug_mapping_r0.emac0 = xi_user_group;
        break ;
    case DRV_SBPM_SP_EMAC1:
        sbpm_ug_mapping_r0.emac1 = xi_user_group;
        break ;
    case DRV_SBPM_SP_EMAC2:
        sbpm_ug_mapping_r0.emac2 = xi_user_group;
        break ;
    case DRV_SBPM_SP_EMAC3:
        sbpm_ug_mapping_r0.emac3 = xi_user_group;
        break ;
    case DRV_SBPM_SP_EMAC4:
        sbpm_ug_mapping_r1.emac4= xi_user_group;
        break ;
    case DRV_SBPM_SP_GPON:
        sbpm_ug_mapping_r0.gpon = xi_user_group;
        break ;
    case DRV_SBPM_SP_RNR_A:
        sbpm_ug_mapping_r0.rnra = xi_user_group;
        break ;
    case DRV_SBPM_SP_RNR_B:
        sbpm_ug_mapping_r0.rnrb = xi_user_group;
        break ;
    case DRV_SBPM_SP_USB0:
        sbpm_ug_mapping_r1.usb0 = xi_user_group;
        break ;
    case DRV_SBPM_SP_USB1:
        sbpm_ug_mapping_r1.usb1= xi_user_group;
        break ;
    case DRV_SBPM_SP_PCI0:
        sbpm_ug_mapping_r1.pcie0 = xi_user_group;
        break ;
    case DRV_SBPM_SP_PCI1:
        sbpm_ug_mapping_r1.pcie1 = xi_user_group;
        break ;
    case DRV_SBPM_SP_MIPS_D:
        sbpm_ug_mapping_r1.mipsd = xi_user_group;
        break ;
    case DRV_SBPM_SP_SPARE_0:
        sbpm_ug_mapping_r1.spare0 = xi_user_group;
        break ;
   case DRV_SBPM_SP_SPARE_1:
        sbpm_ug_mapping_r1.spare1 = xi_user_group;
        break ;
    default:
        return ( DRV_SBPM_ERROR_INVALID_SOURCE_PORT );
    }

    SBPM_BLOCK_REGS_SBPM_UG_MAP_0_WRITE (sbpm_ug_mapping_r0);
    SBPM_BLOCK_REGS_UG_MAP_REG_1_WRITE (sbpm_ug_mapping_r1);

   return ( DRV_SBPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_user_group_mapping );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_error_handling_parameters                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Set Error Handling Parameters                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function set the parameters and thresholds used for error handling:    */
/*   maximum search threshold in case of free buffer without context, enabling */
/*   max search, and enabling checking last BN.                                */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  SBPM_IER                                                                   */
/*                                                                             */
/* Input:                                                                         */
/*                                                                             */
/*    xi_sbpm_error_handling_parameters - SBPM Error handling parameters (struct)*/
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                            */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_error_handling_parameters ( const DRV_SBPM_ERROR_HANDLING_PARAMETERS * xi_sbpm_error_handling_parameters )
{
    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS error_handling_reg ;

    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS_READ ( error_handling_reg ) ;

    error_handling_reg.search_depth =  xi_sbpm_error_handling_parameters -> search_deapth ;
    error_handling_reg.max_search_en =  xi_sbpm_error_handling_parameters -> max_search_enable ;
    error_handling_reg.chck_last_en =  xi_sbpm_error_handling_parameters -> check_last_enable ;
    error_handling_reg.freeze_in_error =  xi_sbpm_error_handling_parameters -> freeze_counters ;

    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS_WRITE ( error_handling_reg);

    return ( DRV_SBPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_error_handling_parameters );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_runner_msg_ctrl                                    */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Set Runner message control                                    */
/*                                                                             */
/* Abstruct:                                                                   */
/*This function sets the runner control parameters which includes enables for  */
/*wake-up and status messages, select control bit for runner to receive message*/
/*and task number for wake-up messages to Runners.                             */
/*                                                                             */
/* Registers:                                                                  */
/* SBPM_RNR_MSG_CTRL,SBPM_RNR_RPLY_TA                                          */
/*                                                                             */
/* Input:                                                                       */
/*                                                                             */
/*  xi_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                            */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_runner_msg_ctrl(DRV_SBPM_RUNNER_MSG_CTRL_PARAMS  * xi_runner_messsage_control_parameters)
{
    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL runner_message_control;
    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA       runner_rply_ta_register;

    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_READ (runner_message_control);

    runner_message_control.task_num = xi_runner_messsage_control_parameters -> runner_reply_wakeup_task_number;
    runner_message_control.stat_wkup_en = xi_runner_messsage_control_parameters -> status_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    runner_message_control.stat_msg_en = xi_runner_messsage_control_parameters -> status_message_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    runner_message_control.rnr_num = xi_runner_messsage_control_parameters-> select_runner ?
        SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_RNR_NUM_RUNNER_B_VALUE :
        SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_RNR_NUM_RUNNER_A_VALUE ;
    runner_message_control.rnr_stat_msg_base_addr = xi_runner_messsage_control_parameters -> message_base_address;

    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_WRITE (runner_message_control);


    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA_READ (runner_rply_ta_register);

    runner_rply_ta_register.rnr_reply_msg_base_addr = xi_runner_messsage_control_parameters -> runner_reply_target_address ;

    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA_WRITE (runner_rply_ta_register);

    return ( DRV_SBPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_runner_msg_ctrl );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_runner_msg_ctrl                                    */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Get Runner message control                                    */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the runner control parameters which includes enables  */
/* for wake-up and status messages, select control bit for runner to receive   */
/* message and task number for wake-up messages to Runners.                    */
/*                                                                             */
/* Registers:                                                                  */
/* SBPM_RNR_MSG_CTRL,SBPM_RNR_RPLY_TA                                          */
/*                                                                             */
/* Input:                                                                       */
/*                                                                             */
/*  xo_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                            */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_runner_msg_ctrl(DRV_SBPM_RUNNER_MSG_CTRL_PARAMS * const xo_runner_messsage_control_parameters)
{
    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL runner_message_control;
    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA       runner_rply_ta_register;

    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_READ (runner_message_control);

    xo_runner_messsage_control_parameters -> runner_reply_wakeup_task_number = runner_message_control.task_num ;
    xo_runner_messsage_control_parameters -> status_wakeup_enable = runner_message_control.stat_wkup_en ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_runner_messsage_control_parameters -> status_message_enable = runner_message_control.stat_msg_en ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_runner_messsage_control_parameters-> select_runner = runner_message_control.rnr_num  ?
        DRV_SBPM_RUNNER_SELECT_RUNNER_B :
        DRV_SBPM_RUNNER_SELECT_RUNNER_A ;
    xo_runner_messsage_control_parameters -> message_base_address = runner_message_control.rnr_stat_msg_base_addr ;

    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA_READ (runner_rply_ta_register);

    xo_runner_messsage_control_parameters -> runner_reply_target_address = runner_rply_ta_register.rnr_reply_msg_base_addr  ;

    return ( DRV_SBPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_runner_msg_ctrl );

