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

/**********************************************************************************/
/*                                                                                */
/* fi_bl_drv_sbpm_init                                                            */
/*                                                                                */
/* Title:                                                                         */
/* SBPM driver - BPM initialize                                                   */
/*                                                                                */
/* Abstruct:                                                                      */
/* This function initialize and sets general configuration of the SBPM block.     */
/* SBPM module initialization is made once in the system lifetime                 */
/*                                                                                */
/* This API performs the following:                                               */
/*  1.    Disable all SBPM source ports.                                          */
/*  2.    Builds the free linked list using the HW accelerator.                   */
/*  3.    Sets SBPM configuration:                                                */
/*      a.    Global threshold.                                                   */
/*      b.    User group configuration (threshold, hysteresis, exclusive          */
/*          threshold/ hysteresis).                                               */
/*      c.    Sets route address of each source port.                             */
/*      d.    Source port to user group mapping.                                  */
/*                                                                                */
/* Input:                                                                         */
/*   xi_base_address - The start address of the BN list                           */
/*   xi_list_size  - The number of buffers in the list                            */
/*   xi_global_configuration - global threshold and hysteresis (struct)           */
/*   xi_user_group_configuration  - user groups threshold and hysteresis (struct) */
/*   xi_replay_address - runner replay address                                    */
/*                                                                                */
/* Output:                                                                        */
/*   DRV_SBPM_ERROR - error code                                                  */
/*       DRV_SBPM_ERROR_NO_ERROR - no error                                       */
/*     DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAXIMUM_VALUE -                       */
/*         the amount of buffers exceeds the maximum value                        */
/**********************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_init(uint16_t xi_base_address, uint16_t xi_list_size, uint16_t xi_replay_address,
    const DRV_SBPM_GLOBAL_CONFIGURATION *xi_global_configuration, const DRV_SBPM_USER_GROUPS_THRESHOLDS *xi_user_group_configuration)
{
    SBPM_BLOCK_REGS_SBPM_SP_EN sbpm_sp_enable;
    SBPM_BLOCK_REGS_INIT_FREE_LIST sbpm_init;
#ifndef _CFE_
    DRV_SBPM_ISR default_interrupts_mask;
#endif
    SBPM_BLOCK_REGS_RADDR_0 route_address0;
    SBPM_BLOCK_REGS_RADDR_1 route_address1;
    SBPM_BLOCK_REGS_RADDR_2 route_address2;
    SBPM_BLOCK_REGS_RADDR_3 route_address3;
    DRV_SBPM_ERROR_HANDLING_PARAMETERS error_handling_params;
    DRV_SBPM_RUNNER_MSG_CTRL_PARAMS runner_msg_ctrl_params;
    uint8_t ug_index;
    DRV_SBPM_ERROR error;

    /*Validate parameters*/
    if (xi_base_address + xi_list_size > CS_DRV_SBPM_MAX_NUMBER_OF_BUFFERS)
        return ( DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAXIMUM_VALUE );

    /* disable all SBPM source ports */
    SBPM_BLOCK_REGS_SBPM_SP_EN_READ(sbpm_sp_enable) ;

    sbpm_sp_enable.rnra_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.rnrb_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.gpon_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth0_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth1_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth2_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth3_sp_en = DRV_SBPM_DISABLE;
    sbpm_sp_enable.eth4_sp_en = DRV_SBPM_DISABLE;

    SBPM_BLOCK_REGS_SBPM_SP_EN_WRITE(sbpm_sp_enable) ;

    /* Write to register init_free_list - Builds the free linked list using the HW accelerator */
    SBPM_BLOCK_REGS_INIT_FREE_LIST_READ(sbpm_init);

    sbpm_init.init_num_bn = xi_list_size;
    sbpm_init.init_head_bn_addr = xi_base_address;
    sbpm_init.bsy = SBPM_BLOCK_REGS_INIT_FREE_LIST_BSY_NO_REQUEST_VALUE;
    sbpm_init.rdy= SBPM_BLOCK_REGS_INIT_FREE_LIST_RDY_DEFAULT_VALUE;
    sbpm_init.init_en = DRV_SBPM_ENABLE;

    SBPM_BLOCK_REGS_INIT_FREE_LIST_WRITE(sbpm_init);

    /* Set BPM global configuration */
    error = fi_bl_drv_sbpm_set_global_threshold(xi_global_configuration->threshold, xi_global_configuration->hysteresis);

    /* Set User Group [0-7] threshold configuration*/
    for (ug_index = 0 ; ug_index < DRV_SBPM_NUMBER_OF_USER_GROUPS; ug_index++)
    {
        error = fi_bl_drv_sbpm_set_user_group_thresholds(ug_index, &xi_user_group_configuration->ug_arr[ug_index]);

        if (error !=  DRV_SBPM_ERROR_NO_ERROR)
            return error ;
    }

    /* Set route address of each source port */
    SBPM_BLOCK_REGS_RADDR_0_READ(route_address0);

    route_address0.eth0_tx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH0_TX_RADDR_ETH0_TX_RADDR_VALUE;
    route_address0.eth0_rx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH0_RX_RADDR_ETH0_RX_RADDR_VALUE;
    route_address0.eth1_tx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH1_TX_RADDR_ETH1_TX_RADDR_VALUE;
    route_address0.eth1_rx_raddr = SBPM_BLOCK_REGS_RADDR_0_ETH1_RX_RADDR_ETH1_RX_RADDR_VALUE;

    SBPM_BLOCK_REGS_RADDR_0_WRITE(route_address0);

    SBPM_BLOCK_REGS_RADDR_1_READ(route_address1);

    route_address1.eth2_tx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH2_TX_RADDR_ETH2_TX_RADDR_VALUE;
    route_address1.eth2_rx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH2_RX_RADDR_ETH2_RX_RADDR_VALUE;
    route_address1.eth3_tx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH3_TX_RADDR_ETH3_TX_RADDR_VALUE;
    route_address1.eth3_rx_raddr = SBPM_BLOCK_REGS_RADDR_1_ETH3_RX_RADDR_ETH3_RX_ADDR_VALUE;

    SBPM_BLOCK_REGS_RADDR_1_WRITE(route_address1);

    SBPM_BLOCK_REGS_RADDR_2_READ(route_address2);

    route_address2.eth4_tx_raddr = SBPM_BLOCK_REGS_RADDR_2_ETH4_TX_RADDR_ETH4_TX_RADDR_VALUE;
    route_address2.eth4_rx_raddr = SBPM_BLOCK_REGS_RADDR_2_ETH4_RX_RADDR_ETH4_RX_ADDR_VALUE;
    route_address2.gpon_tx_raddr = SBPM_BLOCK_REGS_RADDR_2_GPON_TX_RADDR_GPON_TX_RADDR_VALUE;
    route_address2.gpon_rx_raddr = SBPM_BLOCK_REGS_RADDR_2_GPON_RX_RADDR_GPON_RX_RADDR_VALUE;

    SBPM_BLOCK_REGS_RADDR_2_WRITE(route_address2);

    SBPM_BLOCK_REGS_RADDR_3_READ(route_address3);

    route_address3.mipsd_raddr = SBPM_BLOCK_REGS_RADDR_3_MIPSD_RADDR_MIPSD_RADDR_VALUE ;
    route_address3.rnrb_raddr = SBPM_BLOCK_REGS_RADDR_3_RNRB_RADDR_RNRB_RADDR_VALUE ;
    route_address3.rnra_raddr = SBPM_BLOCK_REGS_RADDR_3_RNRA_RADDR_RNRA_RADDR_VALUE;

    SBPM_BLOCK_REGS_RADDR_3_WRITE(route_address3);

    /* Each Source Port is mapped to specified UG */
    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC0, DRV_SBPM_USER_GROUP_0);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC1, DRV_SBPM_USER_GROUP_1);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC2, DRV_SBPM_USER_GROUP_2);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC3, DRV_SBPM_USER_GROUP_3);

    if ( error !=  DRV_SBPM_ERROR_NO_ERROR )
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC4, DRV_SBPM_USER_GROUP_4);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_USB0, DRV_SBPM_USER_GROUP_4);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_USB1, DRV_SBPM_USER_GROUP_4);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_PCI0, DRV_SBPM_USER_GROUP_4);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_PCI1, DRV_SBPM_USER_GROUP_4);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_GPON, DRV_SBPM_USER_GROUP_5);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_A, DRV_SBPM_USER_GROUP_6);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_B, DRV_SBPM_USER_GROUP_6);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_MIPS_C, DRV_SBPM_USER_GROUP_7);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    error = fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_MIPS_D, DRV_SBPM_USER_GROUP_7);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    /*Enable Interrupts in interrupt enablae register*/
#ifndef _CFE_
    default_interrupts_mask.bac_underrun = DRV_SBPM_ENABLE;
    default_interrupts_mask.multicast_counter_overflow = DRV_SBPM_ENABLE;
    default_interrupts_mask.check_last_error = DRV_SBPM_ENABLE;
    default_interrupts_mask.max_search_error = DRV_SBPM_ENABLE;

    error = fi_bl_drv_sbpm_set_interrupt_enable_register(&default_interrupts_mask);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;
#endif
    /* Set Error Handling Parameters */
    error_handling_params.search_deapth = CS_DRV_SBPM_SEARCH_DEPTH;
    error_handling_params.max_search_enable = DRV_SBPM_DISABLE;
    error_handling_params.check_last_enable = DRV_SBPM_ENABLE;
    error_handling_params.freeze_counters = DRV_SBPM_DISABLE;

    error = fi_bl_drv_sbpm_set_error_handling_parameters(&error_handling_params);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    /* Set Runner Message Control parameters */
    error = fi_bl_drv_sbpm_get_runner_msg_ctrl(&runner_msg_ctrl_params);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    runner_msg_ctrl_params.runner_reply_target_address = xi_replay_address >> 3;

    error = fi_bl_drv_sbpm_set_runner_msg_ctrl(&runner_msg_ctrl_params);

    if (error !=  DRV_SBPM_ERROR_NO_ERROR)
        return error;

    return DRV_SBPM_ERROR_NO_ERROR;
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
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*   DRV_SBPM_ERROR_INVALID_SOURCE_PORT - invalid source port                  */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_sp_enable(const DRV_SBPM_SP_ENABLE *xi_sp_enable)
{
    SBPM_BLOCK_REGS_SBPM_SP_EN sbpm_sp_enable;

    SBPM_BLOCK_REGS_SBPM_SP_EN_READ(sbpm_sp_enable);

    sbpm_sp_enable.rnra_sp_en = xi_sp_enable->rnra_sp_enable;
    sbpm_sp_enable.rnrb_sp_en = xi_sp_enable->rnrb_sp_enable;
    sbpm_sp_enable.eth0_sp_en = xi_sp_enable->eth0_sp_enable;
    sbpm_sp_enable.eth1_sp_en = xi_sp_enable->eth1_sp_enable;
    sbpm_sp_enable.eth2_sp_en = xi_sp_enable->eth2_sp_enable;
    sbpm_sp_enable.eth3_sp_en = xi_sp_enable->eth3_sp_enable;
    sbpm_sp_enable.eth4_sp_en = xi_sp_enable->eth4_sp_enable;
    sbpm_sp_enable.gpon_sp_en = xi_sp_enable->gpon_or_eth5_sp_enable;

    SBPM_BLOCK_REGS_SBPM_SP_EN_WRITE(sbpm_sp_enable);

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_sp_enable ) ;

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_global_threshold                                         */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Set SBPM Global threshold                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function sets the global Threshold for Allocated Buffers.              */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*   xi_global_threshold - Global Threshold for Allocated Buffers              */
/*   xi_global_hystersis - how many BNs need to free in order to get out from  */
/*                         global NACK state                                   */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_global_threshold(uint16_t xi_global_threshold, uint16_t xi_global_hysteresis)
{
    SBPM_BLOCK_REGS_SBPM_GL_TRSH global_configuration;

    SBPM_BLOCK_REGS_SBPM_GL_TRSH_READ(global_configuration);

    global_configuration.gl_bah = (xi_global_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
    global_configuration.gl_bat = (xi_global_threshold  & CS_DRV_SBPM_THRESHOLD_MASK);

    SBPM_BLOCK_REGS_SBPM_GL_TRSH_WRITE(global_configuration);

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_global_threshold );

#ifndef _CFE_
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_global_threshold                                         */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get SBPM Global threshold                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the global Threshold for Allocated Buffers.           */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*   xo_global_threshold - Global Threshold for Allocated Buffers              */
/*   xo_global_hystersis - how many BNs need to free in order to get out from  */
/*                         global NACK state                                   */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_global_threshold(uint16_t * const xo_global_threshold, uint16_t * const xo_global_hysteresis)
{
    SBPM_BLOCK_REGS_SBPM_GL_TRSH global_configuration;

    SBPM_BLOCK_REGS_SBPM_GL_TRSH_READ(global_configuration);

    *xo_global_threshold = global_configuration.gl_bat;
    *xo_global_hysteresis = global_configuration.gl_bah;

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_global_threshold );
#endif
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_user_group_thresholds                                    */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Set SBPM User Group threshold configuration                   */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function sets the threshold and hysteresis for a specific user group.  */
/*                                                                             */
/* Input:                                                                      */
/*    xi_user_group - user group                                               */
/*  xi_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_user_group_thresholds(DRV_SBPM_USER_GROUP xi_user_group, const DRV_SBPM_USER_GROUP_CONFIGURATION *xi_configuration)
{
    SBPM_BLOCK_REGS_SBPM_UG0_TRSH user_group_0_configuration;
    SBPM_BLOCK_REGS_SBPM_UG0_EXCL_TRSH user_group_0_excl_configuration;
    SBPM_BLOCK_REGS_SBPM_UG1_TRSH user_group_1_configuration;
    SBPM_BLOCK_REGS_SBPM_UG1_EXCL_TRSH user_group_1_excl_configuration;
    SBPM_BLOCK_REGS_SBPM_UG2_TRSH user_group_2_configuration;
    SBPM_BLOCK_REGS_SBPM_UG2_EXCL_TRSH user_group_2_excl_configuration;
    SBPM_BLOCK_REGS_SBPM_UG3_TRSH user_group_3_configuration;
    SBPM_BLOCK_REGS_SBPM_UG3_EXCL_TRSH user_group_3_excl_configuration;
    SBPM_BLOCK_REGS_SBPM_UG4_TRSH user_group_4_configuration;
    SBPM_BLOCK_REGS_SBPM_UG4_EXCL_TRSH user_group_4_excl_configuration;
    SBPM_BLOCK_REGS_SBPM_UG5_TRSH user_group_5_configuration;
    SBPM_BLOCK_REGS_SBPM_UG5_EXCL_TRSH user_group_5_excl_configuration;
    SBPM_BLOCK_REGS_SBPM_UG6_TRSH user_group_6_configuration;
    SBPM_BLOCK_REGS_SBPM_UG6_EXCL_TRSH user_group_6_excl_configuration;
    SBPM_BLOCK_REGS_SBPM_UG7_TRSH user_group_7_configuration;
    SBPM_BLOCK_REGS_SBPM_UG7_EXCL_TRSH user_group_7_excl_configuration;

    switch (xi_user_group) {
    case DRV_SBPM_USER_GROUP_0:
        SBPM_BLOCK_REGS_SBPM_UG0_TRSH_READ(user_group_0_configuration);
        user_group_0_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_0_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG0_TRSH_WRITE(user_group_0_configuration);

        SBPM_BLOCK_REGS_SBPM_UG0_EXCL_TRSH_READ(user_group_0_excl_configuration);
        user_group_0_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_0_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG0_EXCL_TRSH_WRITE(user_group_0_excl_configuration);
        break;

    case DRV_SBPM_USER_GROUP_1:
        SBPM_BLOCK_REGS_SBPM_UG1_TRSH_READ(user_group_1_configuration);
        user_group_1_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_1_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG1_TRSH_WRITE(user_group_1_configuration);

        SBPM_BLOCK_REGS_SBPM_UG1_EXCL_TRSH_READ(user_group_1_excl_configuration);
        user_group_1_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_1_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG1_EXCL_TRSH_WRITE(user_group_1_excl_configuration);
        break;

    case DRV_SBPM_USER_GROUP_2:
        SBPM_BLOCK_REGS_SBPM_UG2_TRSH_READ(user_group_2_configuration);
        user_group_2_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_2_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG2_TRSH_WRITE(user_group_2_configuration);

        SBPM_BLOCK_REGS_SBPM_UG2_EXCL_TRSH_READ(user_group_2_excl_configuration);
        user_group_2_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_2_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG2_EXCL_TRSH_WRITE(user_group_2_excl_configuration);
        break;

    case DRV_SBPM_USER_GROUP_3:
        SBPM_BLOCK_REGS_SBPM_UG3_TRSH_READ(user_group_3_configuration);
        user_group_3_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_3_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG3_TRSH_WRITE(user_group_3_configuration);

        SBPM_BLOCK_REGS_SBPM_UG3_EXCL_TRSH_READ(user_group_3_excl_configuration);
        user_group_3_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_3_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG3_EXCL_TRSH_WRITE(user_group_3_excl_configuration);
        break;

    case DRV_SBPM_USER_GROUP_4:
        SBPM_BLOCK_REGS_SBPM_UG4_TRSH_READ(user_group_4_configuration);
        user_group_4_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_4_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG4_TRSH_WRITE(user_group_4_configuration);

        SBPM_BLOCK_REGS_SBPM_UG4_EXCL_TRSH_READ(user_group_4_excl_configuration);
        user_group_4_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_4_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG4_EXCL_TRSH_WRITE(user_group_4_excl_configuration);
        break;

    case DRV_SBPM_USER_GROUP_5:
        SBPM_BLOCK_REGS_SBPM_UG5_TRSH_READ(user_group_5_configuration);
        user_group_5_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_5_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG5_TRSH_WRITE(user_group_5_configuration);

        SBPM_BLOCK_REGS_SBPM_UG5_EXCL_TRSH_READ(user_group_5_excl_configuration);
        user_group_5_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_5_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG5_EXCL_TRSH_WRITE(user_group_5_excl_configuration);
        break;

    case DRV_SBPM_USER_GROUP_6:
        SBPM_BLOCK_REGS_SBPM_UG6_TRSH_READ(user_group_6_configuration);
        user_group_6_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_6_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG6_TRSH_WRITE(user_group_6_configuration);

        SBPM_BLOCK_REGS_SBPM_UG6_EXCL_TRSH_READ(user_group_6_excl_configuration);
        user_group_6_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_6_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG6_EXCL_TRSH_WRITE(user_group_6_excl_configuration);
        break;

    case DRV_SBPM_USER_GROUP_7:
        SBPM_BLOCK_REGS_SBPM_UG7_TRSH_READ(user_group_7_configuration);
        user_group_7_configuration.ug_bah = (xi_configuration->hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_7_configuration.ug_bat = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG7_TRSH_WRITE(user_group_7_configuration);

        SBPM_BLOCK_REGS_SBPM_UG7_EXCL_TRSH_READ(user_group_7_excl_configuration);
        user_group_7_excl_configuration.exclh = (xi_configuration->exclusive_hysteresis & CS_DRV_SBPM_THRESHOLD_MASK);
        user_group_7_excl_configuration.exclt = (xi_configuration->threshold & CS_DRV_SBPM_THRESHOLD_MASK);
        SBPM_BLOCK_REGS_SBPM_UG7_EXCL_TRSH_WRITE(user_group_7_excl_configuration);
        break;

    default:
        return DRV_SBPM_ERROR_INVALID_USER_GROUP;
    }

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_user_group_thresholds );
#ifndef _CFE_
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_user_group_thresholds                                    */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get BPM User Group threshold configuration                    */
/*                                                                             */
/* Abstract:                                                                   */
/* This function returns Threshold for Allocated Buffers of UG                 */
/*                                                                             */
/* Input:                                                                      */
/*    xi_user_group - user group                                               */
/*  xo_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range             */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_thresholds (DRV_SBPM_USER_GROUP xi_user_group, DRV_SBPM_USER_GROUP_CONFIGURATION * const xo_configuration)
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
    ug_exclusive_start_address = ( uint32_t ) ( SBPM_BLOCK_REGS_SBPM_UG0_EXCL_TRSH_ADDRESS + CS_DRV_SBPM_UG_ALIGNMENT * ( uint32_t ) xi_user_group );

    READ_32( ug_start_address, ug_configuration);

    xo_configuration->hysteresis = ug_configuration.ug_hysteresis;
    xo_configuration->threshold = ug_configuration.ug_threshold;

    READ_32( ug_exclusive_start_address , ug_exclusive_configuration);

    xo_configuration->exclusive_hysteresis = ug_exclusive_configuration.ug_hysteresis;
    xo_configuration->exclusive_threshold =  ug_exclusive_configuration.ug_threshold;

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_user_group_thresholds );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_user_group_status                                        */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get User Group Status                                         */
/*                                                                             */
/* Abstruct:                                                                   */
/*  This function returns the ACK/NACK state of and in addition two bits of    */
/*  exclusive status for each User-Group                                       */
/*                                                                             */
/* Input:                                                                      */
/*    xi_user_group - user group                                               */
/*    xo_user_group_status - User group status (struct)                        */
/*                                                                             */
/* Output:  error code                                                         */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range             */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_status(DRV_SBPM_USER_GROUP xi_user_group, DRV_SBPM_USER_GROUP_STATUS *const xo_user_group_status)
{
    SBPM_BLOCK_REGS_SBPM_UG_STATUS sbpm_ug_status_register;

    if (!ME_DRV_SBPM_USER_GROUP_IN_RANGE(xi_user_group))
        return DRV_SBPM_ERROR_INVALID_USER_GROUP;

    SBPM_BLOCK_REGS_SBPM_UG_STATUS_READ(sbpm_ug_status_register);

    xo_user_group_status->ug_status = MS_DRV_SBPM_GET_BIT_I(sbpm_ug_status_register.ug_ack_stts, (uint32_t)xi_user_group);
    xo_user_group_status->ug_exclusive_status = MS_DRV_SBPM_GET_BIT_I(sbpm_ug_status_register.ug_excl_stts, (uint32_t)xi_user_group);

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_user_group_status );
#endif
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_user_group_mapping                                       */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Set User Group Mapping                                        */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function maps a User group for a specific Source port                  */
/*                                                                             */
/* Input:                                                                      */
/*    xi_source_port - One of SBPM source ports                                */
/*  xi_user_group  - one of SBPM User group 0-7                                */
/*                                                                             */
/* Output:  error code                                                         */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*     DRV_SBPM_ERROR_INVALID_SOURCE_PORT - invalid source port                */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range             */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_USR xi_source_port, DRV_SBPM_USER_GROUP xi_user_group)
{
    SBPM_BLOCK_REGS_SBPM_UG_MAP_0 sbpm_ug_mapping_r0;
    SBPM_BLOCK_REGS_UG_MAP_REG_1 sbpm_ug_mapping_r1;

    SBPM_BLOCK_REGS_SBPM_UG_MAP_0_READ(sbpm_ug_mapping_r0);
    SBPM_BLOCK_REGS_UG_MAP_REG_1_READ (sbpm_ug_mapping_r1);

    if (!ME_DRV_SBPM_USER_GROUP_IN_RANGE(xi_user_group))
        return DRV_SBPM_ERROR_INVALID_USER_GROUP;

    switch (xi_source_port)
    {
    case DRV_SBPM_SP_MIPS_C:
        sbpm_ug_mapping_r0.cpu = xi_user_group;
        break;
    case DRV_SBPM_SP_EMAC0:
        sbpm_ug_mapping_r0.emac0 = xi_user_group;
        break;
    case DRV_SBPM_SP_EMAC1:
        sbpm_ug_mapping_r0.emac1 = xi_user_group;
        break;
    case DRV_SBPM_SP_EMAC2:
        sbpm_ug_mapping_r0.emac2 = xi_user_group;
        break;
    case DRV_SBPM_SP_EMAC3:
        sbpm_ug_mapping_r0.emac3 = xi_user_group;
        break;
    case DRV_SBPM_SP_EMAC4:
        sbpm_ug_mapping_r1.emac4= xi_user_group;
        break;
    case DRV_SBPM_SP_GPON:
        sbpm_ug_mapping_r0.gpon = xi_user_group;
        break;
    case DRV_SBPM_SP_RNR_A:
        sbpm_ug_mapping_r0.rnra = xi_user_group;
        break;
    case DRV_SBPM_SP_RNR_B:
        sbpm_ug_mapping_r0.rnrb = xi_user_group;
        break;
    case DRV_SBPM_SP_USB0:
        sbpm_ug_mapping_r1.usb0 = xi_user_group;
        break;
    case DRV_SBPM_SP_USB1:
        sbpm_ug_mapping_r1.usb1= xi_user_group;
        break;
    case DRV_SBPM_SP_PCI0:
        sbpm_ug_mapping_r1.pcie0 = xi_user_group;
        break;
    case DRV_SBPM_SP_PCI1:
        sbpm_ug_mapping_r1.pcie1 = xi_user_group;
        break;
    case DRV_SBPM_SP_MIPS_D:
        sbpm_ug_mapping_r1.mipsd = xi_user_group;
        break;
    case DRV_SBPM_SP_SPARE_0:
        sbpm_ug_mapping_r1.spare0 = xi_user_group;
        break;
   case DRV_SBPM_SP_SPARE_1:
        sbpm_ug_mapping_r1.spare1 = xi_user_group;
        break;
    default:
        return ( DRV_SBPM_ERROR_INVALID_SOURCE_PORT );
    }

    SBPM_BLOCK_REGS_SBPM_UG_MAP_0_WRITE(sbpm_ug_mapping_r0);
    SBPM_BLOCK_REGS_UG_MAP_REG_1_WRITE(sbpm_ug_mapping_r1);

   return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_user_group_mapping );
#ifndef _CFE_
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_user_group_mapping                                 */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get User Group Mapping                                        */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the Source port mapping to User groups configuration. */
/*                                                                             */
/* Input:                                                                         */
/*    xi_source_port - One of SBPM source ports                                   */
/*  xo_user_group  - associated User group for this source port                */
/*                                                                             */
/* Output:  error code                                                         */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*     DRV_SBPM_ERROR_INVALID_SOURCE_PORT - invalid source port      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_mapping(DRV_SBPM_SP_USR xi_source_port, DRV_SBPM_USER_GROUP * const xo_user_group)
{
    SBPM_BLOCK_REGS_SBPM_UG_MAP_0 sbpm_ug_mapping_r0;
    SBPM_BLOCK_REGS_UG_MAP_REG_1 sbpm_ug_mapping_r1;

    SBPM_BLOCK_REGS_SBPM_UG_MAP_0_READ(sbpm_ug_mapping_r0);
    SBPM_BLOCK_REGS_UG_MAP_REG_1_READ(sbpm_ug_mapping_r1);

    switch (xi_source_port)
    {
    case DRV_SBPM_SP_MIPS_C:
        * xo_user_group = sbpm_ug_mapping_r0.cpu;
        break;
    case DRV_SBPM_SP_EMAC0:
        * xo_user_group = sbpm_ug_mapping_r0.emac0;
        break;
    case DRV_SBPM_SP_EMAC1:
        * xo_user_group = sbpm_ug_mapping_r0.emac1;
        break;
    case DRV_SBPM_SP_EMAC2:
        * xo_user_group = sbpm_ug_mapping_r0.emac2;
        break;
    case DRV_SBPM_SP_EMAC3:
        * xo_user_group = sbpm_ug_mapping_r0.emac3;
        break;
    case DRV_SBPM_SP_EMAC4:
        * xo_user_group = sbpm_ug_mapping_r1.emac4;
        break;
    case DRV_SBPM_SP_GPON:
        * xo_user_group = sbpm_ug_mapping_r0.gpon;
        break;
    case DRV_SBPM_SP_RNR_A:
        * xo_user_group = sbpm_ug_mapping_r0.rnra;
        break;
    case DRV_SBPM_SP_RNR_B:
        * xo_user_group = sbpm_ug_mapping_r0.rnrb;
        break;
    case DRV_SBPM_SP_USB0:
        * xo_user_group = sbpm_ug_mapping_r1.usb0;
        break;
    case DRV_SBPM_SP_USB1:
        * xo_user_group = sbpm_ug_mapping_r1.usb1;
        break;
    case DRV_SBPM_SP_PCI0:
        * xo_user_group = sbpm_ug_mapping_r1.pcie0;
        break;
    case DRV_SBPM_SP_PCI1:
        * xo_user_group = sbpm_ug_mapping_r1.pcie1;
        break;
    case DRV_SBPM_SP_MIPS_D:
        * xo_user_group = sbpm_ug_mapping_r1.mipsd;
        break;
    case DRV_SBPM_SP_SPARE_0:
        * xo_user_group = sbpm_ug_mapping_r1.spare0;
        break;
    case DRV_SBPM_SP_SPARE_1:
        * xo_user_group = sbpm_ug_mapping_r1.spare1;
        break;
    default:
        return DRV_SBPM_ERROR_INVALID_SOURCE_PORT;
    }

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_user_group_mapping );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_sbpm_get_user_group_counter                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*    SBPM driver - Get User Group Counter                                    */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function returns the number of allocated BN of a specific User Group*/
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    sbpm_ug0_bac-sbpm_ug7_bac                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_user_group - User group 0-7                                                   */
/*   xo_allocated_bn_counter - UG counter for allocated BNs                   */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_SBPM_ERROR - error code                                 */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                           */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range*/
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_counter(DRV_SBPM_USER_GROUP xi_user_group, uint16_t * const xo_allocated_bn_counter)
{
    SBPM_BLOCK_REGS_SBPM_UG0_BAC user_group_counter ;
    uint32_t ug_counter_address;

    if ( ! ME_DRV_SBPM_USER_GROUP_IN_RANGE ( xi_user_group ) )
    {
        return ( DRV_SBPM_ERROR_INVALID_USER_GROUP );
    }

   ug_counter_address = ( uint32_t ) ( SBPM_BLOCK_REGS_SBPM_UG0_BAC_ADDRESS + CS_DRV_SBPM_UG_ALIGNMENT *  ( uint32_t ) xi_user_group );

    READ_32( ug_counter_address, user_group_counter );

    * xo_allocated_bn_counter =  user_group_counter.ug0bac;

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_user_group_counter );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_sbpm_get_global_counter                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*    SBPM driver - Get Global Counter                                        */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function returns the global BN counter                              */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    sbpm_gl_bac                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xo_global_bn_counter - Global counter for allocated BNs                  */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_SBPM_ERROR - error code                                 */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                           */
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_global_counter (  uint16_t * const xo_global_bn_counter )
{
    SBPM_BLOCK_REGS_SBPM_GL_BAC global_bn_counter ;

    SBPM_BLOCK_REGS_SBPM_GL_BAC_READ( global_bn_counter );

    * xo_global_bn_counter =  global_bn_counter.bac;

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_global_counter );
#endif
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_request_buffer                                         */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Request Buffer                                                */
/*                                                                             */
/* Abstruct:                                                                   */
/* cpu requests a free buffer pointer                                          */
/*                                                                             */
/* Registers:                                                                  */
/*    bn_alloc, bn_alloc_rply                                                  */
/*                                                                             */
/* Input:                                                                      */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */
/*     xo_bn - returned 10 bits of SRAM buffer pointer value                   */
/*     xo_status - ACK/NACK state and exclusive status after allocation        */
/*                                                                             */
/* Output:  error code                                                         */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                */
/*     DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation       */
/*     DRV_SBPM_ERROR_NO_FREE_BUFFER -SBPM has no free buffer to allocate*/
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_request_buffer( DRV_SBPM_SP_USR xi_source_port,
                                                                 uint16_t * const xo_bn ,
                                                                 DRV_SBPM_USER_GROUP_STATUS * const xo_status)
{

    SBPM_BLOCK_REGS_BN_ALLOC bn_alloc_reg;
    SBPM_BLOCK_REGS_BN_ALLOC_RPLY bn_alloc_reply_reg;
    uint8_t num_count = 0;

    /*Write source address to bn_alloc register*/
    SBPM_BLOCK_REGS_BN_ALLOC_READ(bn_alloc_reg);
       bn_alloc_reg.sa = xi_source_port;
    SBPM_BLOCK_REGS_BN_ALLOC_WRITE(bn_alloc_reg);

    while( num_count < CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST  )
    {
        SBPM_BLOCK_REGS_BN_ALLOC_RPLY_READ(bn_alloc_reply_reg);
        if(  bn_alloc_reply_reg.busy == 0 )
        {
            break;
        }
        num_count++;
    }

    if( num_count == CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST)
    {
           return ( DRV_SBPM_ERROR_SBPM_BUSY );
    }
    else
    {
        if( bn_alloc_reply_reg.nack ==  1 )
        {
           return ( DRV_SBPM_ERROR_NO_FREE_BUFFER );
        }
        else /* buffer number is valid */
        {
            * xo_bn = bn_alloc_reply_reg.alloc_bn;
            xo_status ->ug_status = bn_alloc_reply_reg.ack;
            xo_status ->ug_exclusive_status = bn_alloc_reply_reg.excl;
           }
    }

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_request_buffer );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_free_buffer_without_context                            */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Free pointer without Context                                  */
/*                                                                             */
/* Abstruct:                                                                   */
/* cpu request to free an occupied pointer without context                     */
/*                                                                             */
/* Input:                                                                       */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */
/*        xi_bn - 10 bits of SRAM buffer pointer value                               */
/*     xo_status - ACK/NACK state and exclusive status after free-buffer       */
/*                                                                             */
/* Output:                                                                     */
/*   error code                                                                */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                */
/*     DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation       */
/*   DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER                    */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_free_buffer_without_context( DRV_SBPM_SP_USR xi_source_port,
                                                                              uint16_t  xi_bn,
                                                                              DRV_SBPM_USER_GROUP_STATUS * const xo_status)
{
    SBPM_BLOCK_REGS_BN_FREE_WITHOUT_CONTXT       sbpm_free_reg ;
    SBPM_BLOCK_REGS_BN_FREE_WITHOUT_CONTXT_RPLY  sbpm_free_reply_reg ;
    uint8_t num_count = 0;

    if ( xi_bn == CS_DRV_SBPM_NULL_VALUE)
    {
        return ( DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER );
    }

    /*Write source address and head bn of the packet to be freed to free_without_context register*/
    SBPM_BLOCK_REGS_BN_FREE_WITHOUT_CONTXT_READ(sbpm_free_reg);

    sbpm_free_reg.head_bn = xi_bn;
    sbpm_free_reg.sa = xi_source_port;

    SBPM_BLOCK_REGS_BN_FREE_WITHOUT_CONTXT_WRITE(sbpm_free_reg);

    /*read from replky register, if busy bit is set - try again.*/
    while( num_count < CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST  )
    {
        SBPM_BLOCK_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_READ(sbpm_free_reply_reg);

        if ( sbpm_free_reply_reg.bsy == 0 ) /*if command was preformed*/
        {
            xo_status ->ug_status = sbpm_free_reply_reg.ack_stat;
            xo_status ->ug_exclusive_status = sbpm_free_reply_reg.excl_stat;
            return DRV_SBPM_ERROR_NO_ERROR;
        }
        num_count++;
    }

    /* operation failed for 10 times - SBPM is busy*/
       return ( DRV_SBPM_ERROR_SBPM_BUSY );

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_free_buffer_without_context );


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_free_buffer_with_context                               */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Free pointer  With Context                                    */
/*                                                                             */
/* Abstruct:                                                                   */
/* cpu request to free an occupied pointer with context                        */
/*                                                                             */
/* Input:                                                                       */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */
/*        xi_head_bn - Head BN of the packet to be freed                            */
/*        xi_last_bn - Last BN of the packet to be freed                           */
/*        xi_size -    Number of BNs of the packet to be freed                    */
/*     xo_status - ACK/NACK state and exclusive status after free-buffer       */
/*                                                                             */
/* Output:                                                                     */
/*   error code                                                                */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                */
/*     DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation       */
/*   DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER                    */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_free_buffer_with_context( DRV_SBPM_SP_USR xi_source_port,
                                                                           uint16_t  xi_head_bn,
                                                                           uint16_t  xi_last_bn,
                                                                           uint16_t  xi_size,
                                                                           DRV_SBPM_USER_GROUP_STATUS * const xo_status)
{
    SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_LOW       sbpm_free_reg_low ;
    SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_HIGH      sbpm_free_reg_high;
    SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_RPLY      sbpm_free_reply_reg ;
    uint8_t num_count = 0;

    if ( xi_head_bn == CS_DRV_SBPM_NULL_VALUE)
    {
        return ( DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER );
    }

    /*Write source address and head bn of the packet to be freed to free_without_context register*/
    SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_HIGH_READ (sbpm_free_reg_high);
    SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_LOW_READ  (sbpm_free_reg_low );

    sbpm_free_reg_high.last_bn = xi_last_bn;
    SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_HIGH_WRITE (sbpm_free_reg_high);

    sbpm_free_reg_low.head_bn = xi_head_bn;
    sbpm_free_reg_low.offset = xi_size;
    sbpm_free_reg_low.sa = xi_source_port;

    SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_LOW_WRITE  (sbpm_free_reg_low );

    /*read from reply register, if busy bit is set - try again.*/
    while( num_count < CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST  )
    {
        SBPM_BLOCK_REGS_BN_FREE_WITH_CONTXT_RPLY_READ(sbpm_free_reply_reg);

        if ( sbpm_free_reply_reg.busy == 0 ) /*if command was preformed*/
        {
            xo_status ->ug_status = sbpm_free_reply_reg.ack_state;
            xo_status ->ug_exclusive_status = sbpm_free_reply_reg.excl_state;
            return DRV_SBPM_ERROR_NO_ERROR;
        }
        num_count++;
    }

    /* operation failed for 10 times - SBPM is busy*/
       return ( DRV_SBPM_ERROR_SBPM_BUSY );

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_free_buffer_with_context );


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_mcnt_update                                            */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Multicast Counter set for pointer                             */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function is responsible to increment Multicast value (3 bits)          */
/* for an occupied buffer pointer.                                             */
/* Remark: Multicast counter value is a delta between 1 and the total number   */
/* to duplicate (i.e.: if we want to duplicate a packet 4 times, counter=3).   */
/*                                                                             */
/* Input:                                                                       */
/*    xi_bn - SBPM pointer for MCNT setting                                       */
/*    xi_mcnt - multicast counter value                                           */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*     DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation       */
/*     DRV_SBPM_ERROR_MULTICAST_VALUE_EXCEEDS_MAXIMUM_VALUE -        */
/*      multicast value can be in range 0-7                                    */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_mcnt_update(uint16_t xi_bn,
                                                             uint8_t  xi_multicast_value )
{
    SBPM_BLOCK_REGS_MCST_INC      sbpm_mcnt_update_reg ;
    SBPM_BLOCK_REGS_MCST_INC_RPLY sbpm_mcnt_update_reply_reg ;
    uint8_t num_count = 0;

    if (  xi_multicast_value > CS_DRV_SBPM_MAX_MULTICAST_VALUE )
    {
        return ( DRV_SBPM_ERROR_MULTICAST_VALUE_EXCEEDS_MAXIMUM_VALUE );
    }

    SBPM_BLOCK_REGS_MCST_INC_READ (sbpm_mcnt_update_reg );
    sbpm_mcnt_update_reg.bn = xi_bn;
    sbpm_mcnt_update_reg.mcst_val = xi_multicast_value;
    SBPM_BLOCK_REGS_MCST_INC_WRITE (sbpm_mcnt_update_reg);

    /*read from reply register, if busy bit is set - try again.*/
    while( num_count < CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST  )
    {
        SBPM_BLOCK_REGS_MCST_INC_RPLY_READ (sbpm_mcnt_update_reply_reg);

        if ( sbpm_mcnt_update_reply_reg.bsy == 0 ) /*if command was preformed*/
        {
            return DRV_SBPM_ERROR_NO_ERROR;
        }
        num_count++;
    }

    /* operation failed for 10 times - SBPM is busy*/
       return ( DRV_SBPM_ERROR_SBPM_BUSY );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_mcnt_update );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_next_bn                                            */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Get next BN                                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function gets a pointer to a buffer in a packet linked list and returns*/
/* the next buffer in the list                                                 */
/*                                                                             */
/* Input:                                                                       */
/*    xi_current_bn - current BN pointer                                            */
/*    xo_pointed_bn - Next buffer in the packet list                              */
/*  xo_mcnt_value - Next buffer multicast value                                */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*     DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation       */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_next_bn (uint16_t xi_current_bn,
                                                              uint16_t * const xo_next_bn,
                                                              uint8_t  * const xo_mcnt_value )
{
    SBPM_BLOCK_REGS_GET_NEXT      sbpm_get_next_reg ;
    SBPM_BLOCK_REGS_GET_NEXT_RPLY sbpm_get_next_reply_reg ;
    uint8_t num_count = 0;

    SBPM_BLOCK_REGS_GET_NEXT_READ (sbpm_get_next_reg );
    sbpm_get_next_reg.bn = xi_current_bn;
    sbpm_get_next_reg.rsv = SBPM_BLOCK_REGS_GET_NEXT_RSV_DEFAULT_VALUE;
    SBPM_BLOCK_REGS_GET_NEXT_WRITE (sbpm_get_next_reg);

    /*read from reply register, if busy bit is set - try again.*/
    while( num_count < CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST  )
    {
        SBPM_BLOCK_REGS_GET_NEXT_RPLY_READ (sbpm_get_next_reply_reg);

        if ( sbpm_get_next_reply_reg.busy == 0 ) /*if command was preformed*/
        {
            * xo_next_bn = sbpm_get_next_reply_reg.next_bn;
            * xo_mcnt_value = sbpm_get_next_reply_reg.mcnt_val;

            return DRV_SBPM_ERROR_NO_ERROR;
        }
        num_count++;
    }

    /* operation failed for 10 times - SBPM is busy*/
       return ( DRV_SBPM_ERROR_SBPM_BUSY );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_next_bn );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_connect_bn                                             */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Connect BN                                                    */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function gets a pointer to a buffer in a packet linked list and returns*/
/* the next buffer in the list                                                 */
/*                                                                             */
/* Input:                                                                       */
/*    xi_current_bn - current BN pointer                                            */
/*    xi_pointed_bn - The Buffer to connect to                                      */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*     DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation       */
/*   DRV_SBPM_ERROR_TRYING_TO_CONNECT_TO_NULL_POINTER              */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_connect_bn  (uint16_t xi_current_bn,
                                                              uint16_t xi_pointed_bn )
{
    SBPM_BLOCK_REGS_BN_CONNECT      sbpm_bn_connect_reg ;
    SBPM_BLOCK_REGS_BN_CONNECT_RPLY sbpm_bn_connect_reply_reg ;
    uint8_t num_count = 0;

    if ( xi_pointed_bn == CS_DRV_SBPM_NULL_VALUE)
    {
        return ( DRV_SBPM_ERROR_TRYING_TO_CONNECT_TO_NULL_POINTER );
    }

    SBPM_BLOCK_REGS_GET_NEXT_READ (sbpm_bn_connect_reg );
    sbpm_bn_connect_reg.ack_req = xi_current_bn;
    sbpm_bn_connect_reg.pointed_bn = xi_pointed_bn;
    sbpm_bn_connect_reg.rsv = SBPM_BLOCK_REGS_BN_CONNECT_RSV_DEFAULT_VALUE;
    SBPM_BLOCK_REGS_GET_NEXT_WRITE (sbpm_bn_connect_reg);

    /*read from reply register, if busy bit is set - try again.*/
    while( num_count < CS_DRV_SBPM_NUMBER_OF_TRIALS_ON_REQUEST  )
    {
        SBPM_BLOCK_REGS_GET_NEXT_RPLY_READ (sbpm_bn_connect_reply_reg);

        if ( sbpm_bn_connect_reply_reg.busy == 0 ) /*if command was preformed*/
        {
            return DRV_SBPM_ERROR_NO_ERROR;
        }
        num_count++;
    }

    /* operation failed for 10 times - SBPM is busy*/
       return ( DRV_SBPM_ERROR_SBPM_BUSY );
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_connect_bn );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_error_handling_parameters                                */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Set Error Handling Parameters                                */
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
/* Input:                                                                      */
/*                                                                             */
/*    xi_sbpm_error_handling_parameters - SBPM Error handling parameters (struct)*/
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_error_handling_parameters(const DRV_SBPM_ERROR_HANDLING_PARAMETERS * xi_sbpm_error_handling_parameters)
{
    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS error_handling_reg;

    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS_READ(error_handling_reg);

    error_handling_reg.search_depth = xi_sbpm_error_handling_parameters->search_deapth;
    error_handling_reg.max_search_en = xi_sbpm_error_handling_parameters-> max_search_enable;
    error_handling_reg.chck_last_en = xi_sbpm_error_handling_parameters->check_last_enable;
    error_handling_reg.freeze_in_error = xi_sbpm_error_handling_parameters->freeze_counters;

    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS_WRITE(error_handling_reg);

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_error_handling_parameters );
#ifndef _CFE_
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_error_handling_parameters                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Get Error Handling Parameters                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function returns the parameters and thresholds used for error handling:*/
/*   maximum search threshold in case of free buffer without context, enabling */
/*   max search, and enabling checking last BN.                                */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  SBPM_IER                                                                   */
/*                                                                             */
/* Input:                                                                         */
/*                                                                             */
/*    xo_sbpm_error_handling_parameters - SBPM Error handling parameters (struct)*/
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_error_handling_parameters (  DRV_SBPM_ERROR_HANDLING_PARAMETERS * const xo_sbpm_error_handling_parameters )
{
    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS error_handling_reg ;

    SBPM_BLOCK_REGS_ERROR_HANDLING_PARAMS_READ ( error_handling_reg ) ;

    xo_sbpm_error_handling_parameters -> search_deapth =  error_handling_reg.search_depth ;
    xo_sbpm_error_handling_parameters -> max_search_enable = error_handling_reg.max_search_en   ;
    xo_sbpm_error_handling_parameters -> check_last_enable = error_handling_reg.chck_last_en   ;
    xo_sbpm_error_handling_parameters -> freeze_counters  = error_handling_reg.freeze_in_error   ;

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_error_handling_parameters );
#endif

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_runner_msg_ctrl                                          */
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
/* Input:                                                                      */
/*                                                                             */
/*  xi_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_runner_msg_ctrl(DRV_SBPM_RUNNER_MSG_CTRL_PARAMS *xi_runner_messsage_control_parameters)
{
    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL runner_message_control;
    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA runner_rply_ta_register;

    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_READ(runner_message_control);

    runner_message_control.task_num = xi_runner_messsage_control_parameters->runner_reply_wakeup_task_number;
    runner_message_control.stat_wkup_en = xi_runner_messsage_control_parameters->status_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    runner_message_control.stat_msg_en = xi_runner_messsage_control_parameters->status_message_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    runner_message_control.rnr_num = xi_runner_messsage_control_parameters->select_runner ?
        SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_RNR_NUM_RUNNER_B_VALUE:
        SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_RNR_NUM_RUNNER_A_VALUE;
    runner_message_control.rnr_stat_msg_base_addr = xi_runner_messsage_control_parameters->message_base_address;

    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_WRITE(runner_message_control);

    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA_READ(runner_rply_ta_register);

    runner_rply_ta_register.rnr_reply_msg_base_addr = xi_runner_messsage_control_parameters->runner_reply_target_address;

    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA_WRITE(runner_rply_ta_register);

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_runner_msg_ctrl );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_runner_msg_ctrl                                          */
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
/* Input:                                                                      */
/*                                                                             */
/*  xo_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_runner_msg_ctrl(DRV_SBPM_RUNNER_MSG_CTRL_PARAMS *const xo_runner_messsage_control_parameters)
{
    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL runner_message_control;
    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA runner_rply_ta_register;

    SBPM_BLOCK_REGS_SBPM_RNR_STAT_MSG_CTRL_READ(runner_message_control);

    xo_runner_messsage_control_parameters->runner_reply_wakeup_task_number = runner_message_control.task_num ;
    xo_runner_messsage_control_parameters->status_wakeup_enable = runner_message_control.stat_wkup_en ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_runner_messsage_control_parameters->status_message_enable = runner_message_control.stat_msg_en ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_runner_messsage_control_parameters->select_runner = runner_message_control.rnr_num ?
        DRV_SBPM_RUNNER_SELECT_RUNNER_B : DRV_SBPM_RUNNER_SELECT_RUNNER_A;
    xo_runner_messsage_control_parameters->message_base_address = runner_message_control.rnr_stat_msg_base_addr;

    SBPM_BLOCK_REGS_SBPM_RNR_RPLY_TA_READ(runner_rply_ta_register);

    xo_runner_messsage_control_parameters->runner_reply_target_address = runner_rply_ta_register.rnr_reply_msg_base_addr;

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_runner_msg_ctrl );


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_runner_wakeup_reply_set                                  */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Set Runner Wakeup Reply Set                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function sets  the configuration of 1 of the 4 sets which used         */
/* for wakeup reply messages. Each set is configured by giving the task number */
/* used for the wakeup message and enable/disable for each one of the 4 messages*/
/* types that SBPM can give reply on them: Alloc buffer, get next, connect,    */
/* multicast increment.                                                        */
/*                                                                             */
/* Input:                                                                      */
/*  xi_wakeup_reply_set_index - set id (0-3)                                   */
/*  xi_runner_wakeup_reply_set - struct                                        */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_runner_wakeup_reply_set(uint8_t xi_wakeup_reply_set_index, const DRV_SBPM_WAKEUP_REPLY_SET_PARAMS *xi_runner_wakeup_reply_set)
{
    SBPM_BLOCK_REGS_SBPM_RNR_WKUP_RPLY_SET0 runner_wakeup_reply_set;

    uint32_t wakeup_reply_set_address = ( SBPM_BLOCK_REGS_SBPM_RNR_WKUP_RPLY_SET0_ADDRESS + CS_DRV_SBPM_UG_ALIGNMENT * xi_wakeup_reply_set_index );

    READ_32 ( wakeup_reply_set_address , runner_wakeup_reply_set);

    runner_wakeup_reply_set.task_num = xi_runner_wakeup_reply_set -> reply_wakeup_task_number;

    runner_wakeup_reply_set.alloc_wake_up_en  = xi_runner_wakeup_reply_set -> alloc_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    runner_wakeup_reply_set.connect_wake_up_en  = xi_runner_wakeup_reply_set -> connect_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    runner_wakeup_reply_set.get_next_wake_up_en  = xi_runner_wakeup_reply_set -> get_next_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    runner_wakeup_reply_set.mcst_wake_up_en  = xi_runner_wakeup_reply_set ->mcnt_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;

    WRITE_32 ( wakeup_reply_set_address , runner_wakeup_reply_set);

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_runner_wakeup_reply_set );


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_runner_wakeup_reply_set                            */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Get Runner Wakeup Reply Set                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the configuration of 1 of the 4 sets which used       */
/* for wakeup reply messages. Each set is configured by giving the task number */
/* used for the wakeup message and enable/disable for each one of the 4 messages*/
/* types that SBPM can give reply on them: Alloc buffer, get next, connect,    */
/* multicast increment.                                                        */
/*                                                                             */
/* Input:                                                                       */
/*  xi_wakeup_reply_set_index - set id (0-3)                                   */
/*  xo_runner_wakeup_reply_set - struct                                        */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_runner_wakeup_reply_set (  uint8_t xi_wakeup_reply_set_index ,
                                                                                DRV_SBPM_WAKEUP_REPLY_SET_PARAMS   * xo_runner_wakeup_reply_set )
{
    SBPM_BLOCK_REGS_SBPM_RNR_WKUP_RPLY_SET0 runner_wakeup_reply_set;

    uint32_t wakeup_reply_set_address = ( SBPM_BLOCK_REGS_SBPM_RNR_WKUP_RPLY_SET0_ADDRESS + CS_DRV_SBPM_UG_ALIGNMENT * xi_wakeup_reply_set_index );

    READ_32 ( wakeup_reply_set_address , runner_wakeup_reply_set);

    xo_runner_wakeup_reply_set -> reply_wakeup_task_number = runner_wakeup_reply_set.task_num  ;

    xo_runner_wakeup_reply_set -> alloc_reply_wakeup_enable = runner_wakeup_reply_set.alloc_wake_up_en   ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_runner_wakeup_reply_set -> connect_reply_wakeup_enable = runner_wakeup_reply_set.connect_wake_up_en   ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_runner_wakeup_reply_set -> get_next_reply_wakeup_enable = runner_wakeup_reply_set.get_next_wake_up_en   ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_runner_wakeup_reply_set ->mcnt_reply_wakeup_enable = runner_wakeup_reply_set.mcst_wake_up_en   ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_runner_wakeup_reply_set );

#ifndef _CFE_
/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_mips_d_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Set MIPS-D message control                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function sets the target address for Reply message and the             */
/* task numbers for wake-up messages for MIPSD                                 */
/*                                                                             */
/* Registers:                                                                  */
/* SBPM_MIPSD_RPLY_TA, SBPM_MIPSD_WKUP_RPLY_SET                                */
/*                                                                             */
/* Input:                                                                       */
/*                                                                             */
/*  xi_mips_d_msg_ctrl_params - struct                                         */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_mips_d_msg_ctrl(DRV_SBPM_MIPS_D_MSG_CTRL_PARAMS * xi_mips_d_messsage_control_parameters)
{
    SBPM_BLOCK_REGS_SBPM_MIPSD_WKUP_RPLY_SET   mips_d_wakeup_reply_set;
    SBPM_BLOCK_REGS_SBPM_MIPSD_RPLY_TA         mips_d_rply_ta_register;


    SBPM_BLOCK_REGS_SBPM_MIPSD_WKUP_RPLY_SET_READ( mips_d_wakeup_reply_set );

    mips_d_wakeup_reply_set.alloc_wake_up_en = xi_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.alloc_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    mips_d_wakeup_reply_set.connect_wake_up_en = xi_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.connect_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    mips_d_wakeup_reply_set.get_next_wake_up_en = xi_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.get_next_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    mips_d_wakeup_reply_set.mcst_wake_up_en = xi_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.mcnt_reply_wakeup_enable ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;

    SBPM_BLOCK_REGS_SBPM_MIPSD_WKUP_RPLY_SET_WRITE( mips_d_wakeup_reply_set );

    SBPM_BLOCK_REGS_SBPM_MIPSD_RPLY_TA_READ (mips_d_rply_ta_register );

    mips_d_rply_ta_register.mipsd_reply_msg_base_addr = xi_mips_d_messsage_control_parameters->mips_d_reply_target_address;

    SBPM_BLOCK_REGS_SBPM_MIPSD_RPLY_TA_WRITE (mips_d_rply_ta_register );

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_mips_d_msg_ctrl );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_mips_d_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* SBPM Driver - Get MIPS-D message control                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the target address for Reply message and the          */
/* task numbers for wake-up messages for MIPSD                                 */
/*                                                                             */
/* Registers:                                                                  */
/* SBPM_MIPSD_RPLY_TA, SBPM_MIPSD_WKUP_RPLY_SET                                */
/*                                                                             */
/* Input:                                                                       */
/*                                                                             */
/*  xo_mips_d_msg_ctrl_params - struct                                         */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_mips_d_msg_ctrl(DRV_SBPM_MIPS_D_MSG_CTRL_PARAMS * const xo_mips_d_messsage_control_parameters)
{

    SBPM_BLOCK_REGS_SBPM_MIPSD_WKUP_RPLY_SET   mips_d_wakeup_reply_set;
    SBPM_BLOCK_REGS_SBPM_MIPSD_RPLY_TA         mips_d_rply_ta_register;


    SBPM_BLOCK_REGS_SBPM_MIPSD_WKUP_RPLY_SET_READ( mips_d_wakeup_reply_set );

    xo_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.alloc_reply_wakeup_enable = mips_d_wakeup_reply_set.alloc_wake_up_en  ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.connect_reply_wakeup_enable = mips_d_wakeup_reply_set.connect_wake_up_en  ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.get_next_reply_wakeup_enable = mips_d_wakeup_reply_set.get_next_wake_up_en  ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;
    xo_mips_d_messsage_control_parameters ->mips_d_wakeup_reply_set.mcnt_reply_wakeup_enable = mips_d_wakeup_reply_set.mcst_wake_up_en  ?
        DRV_SBPM_ENABLE : DRV_SBPM_DISABLE;

    SBPM_BLOCK_REGS_SBPM_MIPSD_RPLY_TA_READ (mips_d_rply_ta_register );

    xo_mips_d_messsage_control_parameters->mips_d_reply_target_address = mips_d_rply_ta_register.mipsd_reply_msg_base_addr  ;

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_mips_d_msg_ctrl );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_interrupt_status_register                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Get Interrupt Status Register                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function returns the BPM interrupts status register                    */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  SBPM_ISR                                                                   */
/*                                                                             */
/* Input:                                                                         */
/*                                                                             */
/*    xo_sbpm_isr - BPM Interrupt Status register                                   */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_interrupt_status_register ( DRV_SBPM_ISR * const xo_sbpm_isr )
{
    SBPM_BLOCK_REGS_SBPM_ISR sbpm_isr ;

    SBPM_BLOCK_REGS_SBPM_ISR_READ ( sbpm_isr ) ;

    xo_sbpm_isr -> bac_underrun = sbpm_isr.bac_underrun ;
    xo_sbpm_isr -> check_last_error = sbpm_isr.check_last_err ;
    xo_sbpm_isr -> max_search_error = sbpm_isr.max_search_err ;
    xo_sbpm_isr -> multicast_counter_overflow = sbpm_isr.mcst_overflow ;

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_interrupt_status_register );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_sbpm_clear_interrupt_status_register                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   SBPM driver - Clear Interrupt Status Register                              */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function clear the interrupt status register                        */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    SBPM_ISR.                                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_SBPM_ERROR - error code                                               */
/*        DRV_SBPM_ERROR_NO_ERROR - no error                           */
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_clear_interrupt_status_register ( void )
{
    SBPM_BLOCK_REGS_SBPM_ISR sbpm_isr ;

    SBPM_BLOCK_REGS_SBPM_ISR_READ ( sbpm_isr ) ;

    sbpm_isr.bac_underrun =  CS_HIGH ;
    sbpm_isr.check_last_err = CS_HIGH ;
    sbpm_isr.max_search_err  = CS_HIGH ;
    sbpm_isr.mcst_overflow  = CS_HIGH ;
    sbpm_isr.rsv = SBPM_BLOCK_REGS_SBPM_IER_RSV_RESRVED_VALUE ;

    SBPM_BLOCK_REGS_SBPM_ISR_WRITE ( sbpm_isr ) ;

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_clear_interrupt_status_register );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_interrupt_enable_register                                */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Set Interrupt Enable Register                                */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function sets the BPM interrupts enable register                       */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  SBPM_IER                                                                   */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*    xi_sbpm_ier - BPM Interrupt Enable register                              */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_interrupt_enable_register(DRV_SBPM_ISR *xi_sbpm_ier)
{
    SBPM_BLOCK_REGS_SBPM_IER sbpm_ier;

    SBPM_BLOCK_REGS_SBPM_IER_READ(sbpm_ier);

    sbpm_ier.bac_underrun_irq_en = xi_sbpm_ier->bac_underrun;
    sbpm_ier.last_err_irq_en = xi_sbpm_ier->check_last_error;
    sbpm_ier.max_srch_err_irq_en  = xi_sbpm_ier->max_search_error;
    sbpm_ier.mcst_overflw_irq_en_  = xi_sbpm_ier->multicast_counter_overflow;

    SBPM_BLOCK_REGS_SBPM_IER_WRITE(sbpm_ier);

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_set_interrupt_enable_register );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_interrupt_enable_register                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Get Interrupt Enable Register                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function returns the BPM interrupts enable register                    */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  BPM_IER                                                                    */
/*                                                                             */
/* Input:                                                                         */
/*                                                                             */
/*    xo_sbpm_ier - BPM Interrupt Enable register                                   */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                               */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                                      */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_interrupt_enable_register ( DRV_SBPM_ISR * const xo_sbpm_ier )
{
    SBPM_BLOCK_REGS_SBPM_IER sbpm_ier ;

    SBPM_BLOCK_REGS_SBPM_IER_READ ( sbpm_ier ) ;

    xo_sbpm_ier -> bac_underrun  = sbpm_ier.bac_underrun_irq_en ;
    xo_sbpm_ier -> check_last_error  = sbpm_ier.last_err_irq_en ;
    xo_sbpm_ier -> max_search_error  = sbpm_ier.max_srch_err_irq_en ;
    xo_sbpm_ier -> multicast_counter_overflow  = sbpm_ier.bac_underrun_irq_en ;

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_interrupt_enable_register );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_sbpm_generate_interrupt_test_register                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac_Driver_BPM - Generate Interrupt test register                      */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function generate interrupt in the interrupt test register          */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*  BPM_ITR                                                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_sbpm_itr - BMP Isr struct                                             */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_SBPM_ERROR - error code                                 */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                           */
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_generate_interrupt_test_register ( DRV_SBPM_ISR * xi_sbpm_itr )
{
    SBPM_BLOCK_REGS_SBPM_ITR sbpm_itr ;

    SBPM_BLOCK_REGS_SBPM_ITR_READ ( sbpm_itr ) ;

    sbpm_itr.bac_underrun_test_irq =  xi_sbpm_itr -> bac_underrun ;
    sbpm_itr.last_err_test_irq = xi_sbpm_itr -> check_last_error ;
    sbpm_itr.max_srch_err_test_irq = xi_sbpm_itr -> max_search_error ;
    sbpm_itr.mcst_overflow_test_irq  = xi_sbpm_itr -> multicast_counter_overflow ;

    SBPM_BLOCK_REGS_SBPM_ITR_WRITE ( sbpm_itr);

    return DRV_SBPM_ERROR_NO_ERROR;

}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_generate_interrupt_test_register );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_sbpm_get_interrupt_information_register                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac_Driver_BPM - Get Interrupt information register                    */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function returns the content of the interrupt information registers.*/
/*    This register is used for debug mode only.                              */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*  SBPM_IIR_LOW, SBPM_IIR_HIGH                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_sbpm_iir - BMP IIr struct                                             */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_SBPM_ERROR - error code                                 */
/*     DRV_SBPM_ERROR_NO_ERROR - no error                           */
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_interrupt_information_register ( DRV_SBPM_IIR * const xo_sbpm_iir )
{
    SBPM_BLOCK_REGS_SBPM_IIR_LOW  sbpm_iir_low ;
    SBPM_BLOCK_REGS_SBPM_IIR_HIGH sbpm_iir_high ;

    SBPM_BLOCK_REGS_SBPM_IIR_HIGH_READ ( sbpm_iir_high ) ;
    SBPM_BLOCK_REGS_SBPM_IIR_LOW_READ ( sbpm_iir_low ) ;

    xo_sbpm_iir->cmd_sa = sbpm_iir_low.cmd_sa;
    xo_sbpm_iir->cmd_type = sbpm_iir_low.cmd_ta;
    xo_sbpm_iir->cmd_data[ 0 ] = sbpm_iir_low.cmd_data_22to0;

    xo_sbpm_iir->cmd_data[ 0 ] |= ( ( ( sbpm_iir_high.cmd_data_23to42 ) & CS_DRV_SBPM_IIR_LOW_PART_MASK ) << CS_DRV_SBPM_IIR_SHIFT );
    xo_sbpm_iir->cmd_data[ 1 ] =  ( ( sbpm_iir_high.cmd_data_23to42 ) & CS_DRV_SBPM_IIR_HIGH_PART_MASK );

    return DRV_SBPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL ( fi_bl_drv_sbpm_get_interrupt_information_register );
#endif
