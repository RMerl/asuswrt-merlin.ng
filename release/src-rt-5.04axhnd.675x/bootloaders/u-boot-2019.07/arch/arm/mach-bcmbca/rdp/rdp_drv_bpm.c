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

#include "rdp_subsystem_common.h"
#include "rdp_drv_bpm.h"



/******************************************************************************/
/*                                                                            */
/* Default values definitions                                                 */
/*                                                                            */
/******************************************************************************/

#define CS_DRV_BPM_SINGLE_MEM               ( 2560 )
#define CS_DRV_BPM_BN_ALIGMENT              ( 1536 )
#define CS_DRV_BPM_UG_ALIGMENT              ( 4 )
#if defined(DSL_63138)
#define CS_DRV_BPM_GLOBAL_THRESHOLD_MASK    ( 0xf )
#define CS_DRV_BPM_FREE_BUFFER_PTR_MASK     ( 0x7fff )
#define CS_DRV_BPM_MCNT_BUFFER_PTR_MASK     ( 0x7fff )
#define CS_DRV_BPM_UG_MASK                  ( 0x7fff )
#define CS_DRV_BPM_GLOBAL_HYSTERSIS_MASK    ( 0x7fff )
#else
#define CS_DRV_BPM_GLOBAL_THRESHOLD_MASK    ( 0x7 )
#define CS_DRV_BPM_FREE_BUFFER_PTR_MASK     ( 0x3fff )
#define CS_DRV_BPM_MCNT_BUFFER_PTR_MASK     ( 0x3fff )
#define CS_DRV_BPM_UG_MASK                  ( 0x3fff )
#define CS_DRV_BPM_GLOBAL_HYSTERSIS_MASK    ( 0x3fff )
#endif
#define CS_DRV_BPM_FREE_BUFFER_OWNER_MASK   ( 0x1f )
#define CS_DRV_BPM_MCNT_VALUE_MASK          ( 0x7 )
#define CS_DRV_BPM_WAKEUP_TN_MASK           ( 0x3f )
#define CS_DRV_BPM_MEM_SELECT_MASK          ( 0x7000 )
#define CS_DRV_BPM_ADDRESS_FIELD_MASK       ( 0xfe0 )
#define CS_DRV_BPM_BITS_INDEX_MASK          ( 0x1f )
#define CS_DRV_BPM_NUM_OF_BITS_FOR_BN       ( 3 )
#define CS_DRV_BPM_RNR_TA                   ( 0x1540 )

/* Low */
#define CS_LOW      (  0 )
/* High */
#define CS_HIGH     (  1  )

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/

/* gets bit #i from a given number */
#define MS_DRV_BPM_GET_BIT_I( number , i )   ( ( ( 1 << i ) & ( number ) ) >> i )

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
/* fi_bl_drv_bpm_init					                                   */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - BPM initialize    	                                           */
/*                                                                             */
/* Abstract:                                                                   */
/* BPM module initialization is made once in the system lifetime               */
/* This API performs the following:                                            */
/*  1.	Write a value to register ram_init.                                    */
/*  2.	Setting Route Addresses to each Source Port                            */
/*  3.	Set BPM global threshold, thresholds for all UG and Exclusive UGs      */
/*      according to input parameters.                                         */   
/*  4.	Mapping SP to UGs using registers BPM_UG_MAP_0, BPM_UG_MAP_1.          */
/*This function sets general configuration of the BPM block                    */
/*	                                                                           */
/*                                                                             */
/* Input: 																	   */
/*   global_configuration - global threshold and hysteresis (struct)           */
/*   ug_configuration  - user groups threshold and hysteresis (struct)         */
/*   xi_replay_address - runner replay address                                  */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_init(DRV_BPM_GLOBAL_CONFIGURATION    * xi_global_configuration, 
                                         DRV_BPM_USER_GROUPS_THRESHOLDS  * xi_ug_configuration,
                                         uint16_t                             xi_replay_address,
                                         E_DRV_BPM_SPARE_MESSAGE_FORMAT xi_bpm_spare_message_format )
{
    BPM_MODULE_REGS_RAM_INIT   bpm_init;     
    DRV_BPM_ERROR     error;
    BPM_MODULE_REGS_BPM_RADDR0 raddr0;
    BPM_MODULE_REGS_BPM_RADDR1 raddr1;
    BPM_MODULE_REGS_BPM_RADDR2 raddr2;
    uint8_t                      ug_index;
    DRV_BPM_RUNNER_MSG_CTRL_PARAMS runner_msg_ctrl_params ;
    BPM_MODULE_REGS_BPM_SPARE bpm_spare_register;

    /* Write to register ram_init value */
    BPM_MODULE_REGS_RAM_INIT_READ(bpm_init);
    bpm_init.bsy = BPM_MODULE_REGS_RAM_INIT_BSY_READY_VALUE;
    bpm_init.rdy = BPM_MODULE_REGS_RAM_INIT_RDY_BUSY_VALUE;
    BPM_MODULE_REGS_RAM_INIT_WRITE(bpm_init);

    /* Set BPM global configuration  */
    error = fi_bl_drv_bpm_set_global_threshold ( xi_global_configuration->threshold , xi_global_configuration->hysteresis );

    /* Set User Group [0-7] threshold configuration*/
    for ( ug_index = 0 ; ug_index < DRV_BPM_NUMBER_OF_USER_GROUPS  ; ug_index++  ) 
    {
        error = fi_bl_drv_bpm_set_user_group_thresholds ( ug_index, &xi_ug_configuration->ug_arr[ug_index] );
        if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
        {
            return ( error );
        }
    }

	/* Write to register bpm_raddr0 value 0x1F010002 :
	 	emac0 Rx route address = 0x1f
		gpon Rx route address = 0x01
		runner B route address = 0x02
		runner A route address = 0 */
    BPM_MODULE_REGS_BPM_RADDR0_READ (raddr0 );
    raddr0.emac0_rx_raddr = BPM_MODULE_REGS_BPM_RADDR0_EMAC0_RX_RADDR_EMAC0_RX_ROUTE_ADDRESS_VALUE ;
    raddr0.gpon_rx_raddr  = BPM_MODULE_REGS_BPM_RADDR0_GPON_RX_RADDR_GPON_RX_ROUTE_ADDRESS_VALUE ;
    raddr0.runa_raddr = BPM_MODULE_REGS_BPM_RADDR0_RUNA_RADDR_RUNNER_A_ROUTE_ADDRESS_VALUE;
    raddr0.runb_raddr = BPM_MODULE_REGS_BPM_RADDR0_RUNB_RADDR_RUNNER_B_ROUTE_ADDRESS_VALUE;
    BPM_MODULE_REGS_BPM_RADDR0_WRITE( raddr0 );

    /* Write to register bpm_raddr1 value 0x1109170f 
		emac4 rx route address = 0x11
		emac3 rx route address = 0x9
		emac2 rx route address = 0x1b
		emac1 rx route address = 0x0f*/	
    BPM_MODULE_REGS_BPM_RADDR1_READ( raddr1 );
    raddr1.emac1_rx_raddr = BPM_MODULE_REGS_BPM_RADDR1_EMAC1_RX_RADDR_EMAC1_ROUTE_ADDRESS_VALUE;
    raddr1.emac2_rx_raddr = BPM_MODULE_REGS_BPM_RADDR1_EMAC2_RX_RADDR_EMAC2_ROUTE_ADDRESS_VALUE;
    raddr1.emac3_rx_raddr = BPM_MODULE_REGS_BPM_RADDR1_EMAC3_RX_RADDR_CONFIGURABLE_EMAC3_RX_ROUTE_ADDRESS_VALUE;
    raddr1.emac4_rx_raddr = BPM_MODULE_REGS_BPM_RADDR1_EMAC4_RX_RADDR_EMAC4_RX_ROUTE_ADDRESS_VALUE ;
    BPM_MODULE_REGS_BPM_RADDR1_WRITE( raddr1 );

    /* Write to register bpm_raddr2 value 0x06 */	
    BPM_MODULE_REGS_BPM_RADDR2_READ ( raddr2 );
    raddr2.mipsd_raddr = BPM_MODULE_REGS_BPM_RADDR2_MIPSD_RADDR_MIPSD_ROUTE_ADDRESS_VALUE;
    BPM_MODULE_REGS_BPM_RADDR2_WRITE( raddr2 );

    /* Each Source Port is mapped to specified UG. */
    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_MIPS_C , DRV_BPM_USER_GROUP_7 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_RNR_A , DRV_BPM_USER_GROUP_6 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_RNR_B , DRV_BPM_USER_GROUP_6 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_GPON , DRV_BPM_USER_GROUP_5 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_EMAC0 , DRV_BPM_USER_GROUP_0 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_EMAC1 , DRV_BPM_USER_GROUP_1 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_EMAC2 , DRV_BPM_USER_GROUP_2 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_EMAC3 , DRV_BPM_USER_GROUP_3 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_EMAC4 , DRV_BPM_USER_GROUP_4 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_USB0 , DRV_BPM_USER_GROUP_4 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_USB1 , DRV_BPM_USER_GROUP_4 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_PCI0 , DRV_BPM_USER_GROUP_4 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_PCI1 , DRV_BPM_USER_GROUP_4 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_MIPS_D , DRV_BPM_USER_GROUP_7 );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_sp_enable ( DRV_BPM_SP_RNR_A , DRV_BPM_ENABLE );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_sp_enable ( DRV_BPM_SP_RNR_B , DRV_BPM_ENABLE );
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    /* Set Runner Message Control parameters */
    error = fi_bl_drv_bpm_get_runner_msg_ctrl ( & runner_msg_ctrl_params ) ;
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    runner_msg_ctrl_params.runner_a_reply_target_address = xi_replay_address >> 3 ;
    runner_msg_ctrl_params.runner_b_reply_target_address = xi_replay_address >> 3 ;

    error = fi_bl_drv_bpm_set_runner_msg_ctrl ( & runner_msg_ctrl_params ) ;
    if ( error !=  DRV_BPM_ERROR_NO_ERROR ) 
    {
        return ( error );
    }

    BPM_MODULE_REGS_BPM_SPARE_READ( bpm_spare_register );
    bpm_spare_register.bn_msg_format = xi_bpm_spare_message_format;
    BPM_MODULE_REGS_BPM_SPARE_WRITE( bpm_spare_register );
    return ( DRV_BPM_ERROR_NO_ERROR );

}
EXPORT_SYMBOL ( fi_bl_drv_bpm_init );

/*******************************************************************************/
/* fi_bl_drv_bpm_sp_enable					       */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Source Ports Enable	                                       */
/*                                                                             */
/* Abstract:                                                                   */
/* Source Ports Enable	                                                       */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_SP_EN                                                                   */
/*                                                                             */
/* Input:								       */
/*                                                                             */
/*    xi_source_port - One of the BPM source port: GPON, EMAC0-4, RNR_A/B      */
/*                                                                             */
/*    xi_enable - enable/ disable                                              */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 			       */		
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port       */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_USR xi_source_port, 
                                                         E_DRV_BPM_ENABLE xi_enable )
{
    BPM_MODULE_REGS_BPM_SP_EN bpm_sp_enable;

    BPM_MODULE_REGS_BPM_SP_EN_READ( bpm_sp_enable);

    switch(xi_source_port)
    {
        case DRV_BPM_SP_RNR_A:
            bpm_sp_enable.rnra_en = xi_enable;
            break;
        case DRV_BPM_SP_RNR_B:
            bpm_sp_enable.rnrb_en = xi_enable;
            break;
        case DRV_BPM_SP_GPON:
            bpm_sp_enable.gpon_en = xi_enable;
            break;
        case DRV_BPM_SP_EMAC0:
            bpm_sp_enable.emac0_en = xi_enable;
            break;
        case DRV_BPM_SP_EMAC1:
            bpm_sp_enable.emac1_en = xi_enable;
            break;
        case DRV_BPM_SP_EMAC2:
            bpm_sp_enable.emac2_en = xi_enable;
            break;
        case DRV_BPM_SP_EMAC3:
            bpm_sp_enable.emac3_en = xi_enable;
            break;
        case DRV_BPM_SP_EMAC4:
            bpm_sp_enable.emac4_en = xi_enable;
            break;
        default:
            return ( DRV_BPM_ERROR_INVALID_SOURCE_PORT );
    }

    BPM_MODULE_REGS_BPM_SP_EN_WRITE( bpm_sp_enable);

    return ( DRV_BPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_bpm_sp_enable );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_global_threshold                                    */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Set BPM Global threshold                                       */
/*                                                                             */
/* Abstract:                                                                   */
/* This function sets the global Threshold for Allocated Buffers.              */
/*                                                                             */
/* Input:																	   */
/*                                                                             */
/*   xi_global_threshold - Global Threshold for Allocated Buffers              */
/*   xi_global_hystersis - Global Buffer Allocation Hysteresis threshold       */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_global_threshold ( DRV_BPM_GLOBAL_THRESHOLD xi_global_threshold,
                                                                      uint32_t xi_global_hysteresis )
{
    BPM_MODULE_REGS_BPM_GL_TRSH global_configuration;

    BPM_MODULE_REGS_BPM_GL_TRSH_READ(global_configuration);

    global_configuration.gl_bah = ( xi_global_hysteresis & CS_DRV_BPM_GLOBAL_HYSTERSIS_MASK);
    global_configuration.gl_bat = ( xi_global_threshold  & CS_DRV_BPM_GLOBAL_THRESHOLD_MASK);

    BPM_MODULE_REGS_BPM_GL_TRSH_WRITE(global_configuration);

    return ( DRV_BPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_bpm_set_global_threshold );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_user_group_thresholds                               */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Set BPM User Group threshold configuration                     */
/*                                                                             */
/* Abstract:                                                                   */
/* Threshold for Allocated Buffers of UG                                       */
/* Ths register also holds UG0 hysteresis value for ACK/NACK transition setting*/
/* This register is affected by soft reset.                                    */
/*                                                                             */
/* Input:																	   */
/*	xi_ug - user group 														   */			
/*  xi_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_user_group_thresholds (DRV_BPM_USER_GROUP xi_ug, 
                                                                          DRV_BPM_USER_GROUP_CONFIGURATION * xi_configuration)
{
    DRV_BPM_UG_THRESHOLD ug_configuration;
    DRV_BPM_UG_THRESHOLD ug_exclusive_configuration;
    uint32_t ug_start_address = ( BPM_MODULE_REGS_BPM_UG0_TRSH_ADDRESS + CS_DRV_BPM_UG_ALIGMENT * xi_ug );
    uint32_t ug_exclusive_start_address = ( BPM_MODULE_REGS_BPM_UG0_EXCL_TRSH_ADDRESS + CS_DRV_BPM_UG_ALIGMENT * xi_ug );

    READ_32( ug_start_address, ug_configuration);

    ug_configuration.ug_hysteresis = ( xi_configuration->hysteresis & CS_DRV_BPM_UG_MASK );
    ug_configuration.ug_threshold =  ( xi_configuration->threshold & CS_DRV_BPM_UG_MASK );

    WRITE_32( ug_start_address, ug_configuration);	

    READ_32( ug_exclusive_start_address , ug_exclusive_configuration);

    ug_exclusive_configuration.ug_hysteresis = ( xi_configuration->exclusive_hysteresis & CS_DRV_BPM_UG_MASK );
    ug_exclusive_configuration.ug_threshold =  ( xi_configuration->exclusive_threshold  & CS_DRV_BPM_UG_MASK );

    WRITE_32( ug_exclusive_start_address , ug_exclusive_configuration);

    return ( DRV_BPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_bpm_set_user_group_thresholds );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_user_group_mapping                                  */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Set User Group Mapping                                         */
/*                                                                             */
/* Abstract:                                                                   */
/* This function maps a User group for a specific Source port                  */
/*                                                                             */
/* Input:                                              			               */
/*	xi_source_port - One of BPM source ports		                           */
/*  xi_user_group  - one of BPM User group 0-7                                 */
/*                                                                             */
/* Output:  error code                                                         */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*	 DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port   	   */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_USR  xi_source_port,
                                                                        DRV_BPM_USER_GROUP xi_user_group )
{
    BPM_MODULE_REGS_BPM_UG_MAP_R0  bpm_ug_mapping_r0;
    BPM_MODULE_REGS_BPM_UG_MAP_R1  bpm_ug_mapping_r1;

    BPM_MODULE_REGS_BPM_UG_MAP_R0_READ(bpm_ug_mapping_r0);
    BPM_MODULE_REGS_BPM_UG_MAP_R1_READ(bpm_ug_mapping_r1);

    switch ( xi_source_port )
    {   
        case DRV_BPM_SP_MIPS_C:
            bpm_ug_mapping_r0.cpu_ = xi_user_group;
            break ;
        case DRV_BPM_SP_EMAC0:
            bpm_ug_mapping_r0.emac0 = xi_user_group;
            break ;
        case DRV_BPM_SP_EMAC1:
            bpm_ug_mapping_r0.emac1 = xi_user_group;
            break ;
        case DRV_BPM_SP_EMAC2:
            bpm_ug_mapping_r0.emac2 = xi_user_group;
            break ;
        case DRV_BPM_SP_EMAC3:
            bpm_ug_mapping_r0.emac3 = xi_user_group;
            break ;
        case DRV_BPM_SP_EMAC4:
            bpm_ug_mapping_r1.emac4= xi_user_group;
            break ;
        case DRV_BPM_SP_GPON:
            bpm_ug_mapping_r0.gpon = xi_user_group;
            break ;
        case DRV_BPM_SP_RNR_A:
            bpm_ug_mapping_r0.rnr_a = xi_user_group;
            break ;
        case DRV_BPM_SP_RNR_B:
            bpm_ug_mapping_r0.rnr_b = xi_user_group;
            break ;
        case DRV_BPM_SP_USB0:
            bpm_ug_mapping_r1.usb0 = xi_user_group;
            break ;
        case DRV_BPM_SP_USB1:
            bpm_ug_mapping_r1.usb1= xi_user_group;
            break ;
        case DRV_BPM_SP_PCI0:
            bpm_ug_mapping_r1.pcie0 = xi_user_group;
            break ;
        case DRV_BPM_SP_PCI1:
            bpm_ug_mapping_r1.pcie1 = xi_user_group;
            break ;
        case DRV_BPM_SP_MIPS_D:
            bpm_ug_mapping_r1.mipsd = xi_user_group;
            break ;
        case DRV_BPM_SP_SPARE_0:
            bpm_ug_mapping_r1.spare0 = xi_user_group;
            break ;
        case DRV_BPM_SP_SPARE_1:
            bpm_ug_mapping_r1.spare1 = xi_user_group;
            break ;
        default:
            return ( DRV_BPM_ERROR_INVALID_SOURCE_PORT );
    }

    BPM_MODULE_REGS_BPM_UG_MAP_R0_WRITE (bpm_ug_mapping_r0);
    BPM_MODULE_REGS_BPM_UG_MAP_R1_WRITE (bpm_ug_mapping_r1);

    return ( DRV_BPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_bpm_set_user_group_mapping );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_req_buffer                                              */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Request Buffer                                                 */
/*                                                                             */
/* Abstract:                                                                   */
/* cpu requests a free buffer pointer                                          */
/*                                                                             */
/* Input:                                                                      */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */
/*     xo_bn - returned 14 bits of DDR buffer pointer value                    */
/*                                                                             */
/* Output:  error code                                                         */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*	 DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation	   */	
/*	 DRV_BPM_ERROR_NO_FREE_BUFFER - BPM has no free buffer         */	
/*                                              to allocate                    */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_USR xi_source_port,
                                                          uint32_t * const xo_bn)
{
	
    BPM_MODULE_REGS_REQ_PTR req_ptr;
    int32_t num_count = 10;

    BPM_MODULE_REGS_REQ_PTR_READ(req_ptr);
    req_ptr.sp_addr = xi_source_port;
    BPM_MODULE_REGS_REQ_PTR_WRITE(req_ptr);

    while( num_count-- > 0 )
    { 
        BPM_MODULE_REGS_REQ_PTR_READ(req_ptr);
        if(  req_ptr.bsy != BPM_MODULE_REGS_REQ_PTR_BSY_BUSY_VALUE )
        {
            break;
        }
    }

    if( num_count == -1 )
    {
        return ( DRV_BPM_ERROR_BPM_BUSY );
    }
    else
    {
        if( req_ptr.nack_status == BPM_MODULE_REGS_REQ_PTR_NACK_STATUS_CPU_IN_NACK_STATE_VALUE )
        {
            return ( DRV_BPM_ERROR_NO_FREE_BUFFER );
        } 
        else /* buffer number is valid */
        {
            * xo_bn = req_ptr.bn;
        }
    }

    return ( DRV_BPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_bpm_req_buffer );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_runner_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Set Runner message control                                     */
/*                                                                             */
/* Abstract:                                                                   */
/* Enables runner wake-up messages, 										   */
/* select control bit for transition message and task numbers for wake-up      */
/* messages to Runners														   */ 
/*                                                                             */
/* Registers:                                                                  */
/* BPM_RNR_MSG_CTRL,BPM_RNR_RPLY_TA                                            */
/*                                                                             */
/* Input:																	   */
/*                                                                             */
/*  xi_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_runner_msg_ctrl(DRV_BPM_RUNNER_MSG_CTRL_PARAMS  * xi_runner_msg_ctrl_params)
{
    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL runner_message_control;
    BPM_MODULE_REGS_BPM_RNR_RPLY_TA  bpm_rnr_rply_ta_register;

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_READ( runner_message_control );

    runner_message_control.rnr_rply_wkup_en = ( xi_runner_msg_ctrl_params->reply_wakeup_enable ==  1 ) ? 
        DRV_BPM_ENABLE : DRV_BPM_DISABLE ;
    runner_message_control.rnr_trans_wkup_en = ( xi_runner_msg_ctrl_params->transition_wakeup_enable ==  1 ) ? 
        DRV_BPM_ENABLE : DRV_BPM_DISABLE ;
    runner_message_control.rnr_sel_trans_msg =  ( xi_runner_msg_ctrl_params->select_transition_msg ==  1 ) ? 
        BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_RNR_SEL_TRANS_MSG_TRANSITION_MESSAGE_SELECTED_TO_RUNNER_B_VALUE : 
        BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_RNR_SEL_TRANS_MSG_TRANSITION_MESSAGE_SELECTED_TO_RUNNER_A_VALUE ;
    runner_message_control.rnr_a_rply_wkup_tn = xi_runner_msg_ctrl_params->runner_a_reply_wakeup_task_number & CS_DRV_BPM_WAKEUP_TN_MASK;
    runner_message_control.rnr_b_rply_wkup_tn = xi_runner_msg_ctrl_params->runner_b_reply_wakeup_task_number & CS_DRV_BPM_WAKEUP_TN_MASK;
    runner_message_control.rnr_trans_wkup_tn =  xi_runner_msg_ctrl_params->runner_transition_wakeup_task_number & CS_DRV_BPM_WAKEUP_TN_MASK; 	

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_WRITE( runner_message_control );

    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_READ ( bpm_rnr_rply_ta_register );

    bpm_rnr_rply_ta_register.rnr_a_ta = xi_runner_msg_ctrl_params->runner_a_reply_target_address ;
    bpm_rnr_rply_ta_register.rnr_b_ta = xi_runner_msg_ctrl_params->runner_b_reply_target_address ;

    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_WRITE (bpm_rnr_rply_ta_register);

    return ( DRV_BPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_bpm_set_runner_msg_ctrl );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_runner_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Get Runner message control                                     */
/*                                                                             */
/* Abstract:                                                                   */
/* Enables runner wake-up messages, 										   */
/* select control bit for transition message and task numbers for wake-up      */
/* messages to Runners														   */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_RNR_MSG_CTRL,BPM_RNR_RPLY_TA                                            */
/*                                                                             */
/* Input:																	   */
/*                                                                             */
/*  xo_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_runner_msg_ctrl(DRV_BPM_RUNNER_MSG_CTRL_PARAMS * const xo_runner_msg_ctrl_params)
{
    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL runner_message_control;
    BPM_MODULE_REGS_BPM_RNR_RPLY_TA  bpm_rnr_rply_ta_register;

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_READ( runner_message_control );

    xo_runner_msg_ctrl_params->reply_wakeup_enable = ( runner_message_control.rnr_rply_wkup_en ==  1 ) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    xo_runner_msg_ctrl_params->transition_wakeup_enable =  ( runner_message_control.rnr_trans_wkup_en ==  1 ) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    xo_runner_msg_ctrl_params->select_transition_msg = ( runner_message_control.rnr_sel_trans_msg ==  1 )  ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    xo_runner_msg_ctrl_params->runner_a_reply_wakeup_task_number = runner_message_control.rnr_a_rply_wkup_tn;
    xo_runner_msg_ctrl_params->runner_b_reply_wakeup_task_number = runner_message_control.rnr_b_rply_wkup_tn;	
    xo_runner_msg_ctrl_params->runner_transition_wakeup_task_number = runner_message_control.rnr_trans_wkup_tn;		
		
    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_READ (bpm_rnr_rply_ta_register);

    xo_runner_msg_ctrl_params->runner_a_reply_target_address = ( uint16_t ) bpm_rnr_rply_ta_register.rnr_a_ta;
    xo_runner_msg_ctrl_params->runner_b_reply_target_address = ( uint16_t ) bpm_rnr_rply_ta_register.rnr_b_ta;

    return ( DRV_BPM_ERROR_NO_ERROR );
}
EXPORT_SYMBOL ( fi_bl_drv_bpm_get_runner_msg_ctrl );
