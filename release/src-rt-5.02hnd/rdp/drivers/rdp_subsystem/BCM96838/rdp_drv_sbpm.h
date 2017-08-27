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
/* This header file defines all datatypes and functions exported for the      */
/* Lilac SBPM driver.                                                          */
/*                                                                            */
/******************************************************************************/

#ifndef LILAC_DRV_SBPM_H_INCLUDED
#define LILAC_DRV_SBPM_H_INCLUDED


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "rdp_sbpm.h"

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Error codes returned by SBPM driver APIs                                    */
/******************************************************************************/
typedef enum
{
	DRV_SBPM_ERROR_NO_ERROR = 0,
    DRV_SBPM_ERROR_SBPM_BUSY ,
    DRV_SBPM_ERROR_NO_FREE_BUFFER,
    DRV_SBPM_ERROR_INVALID_SOURCE_PORT,
    DRV_SBPM_ERROR_INVALID_USER_GROUP,
    DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAXIMUM_VALUE,
    DRV_SBPM_ERROR_TRYING_TO_CONNECT_TO_NULL_POINTER,
    DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER,
    DRV_SBPM_ERROR_MULTICAST_VALUE_EXCEEDS_MAXIMUM_VALUE
	
}DRV_SBPM_ERROR;


/******************************************************************************/
/* BPM source ports                                                           */
/******************************************************************************/
typedef enum
{
	DRV_SBPM_SP_GPON,
	DRV_SBPM_SP_EMAC0,
	DRV_SBPM_SP_EMAC1,
	DRV_SBPM_SP_EMAC2,
	DRV_SBPM_SP_EMAC3,
	DRV_SBPM_SP_EMAC4,
    DRV_SBPM_SP_MIPS_C,
    DRV_SBPM_SP_MIPS_D,
    DRV_SBPM_SP_PCI0,
    DRV_SBPM_SP_PCI1,
    DRV_SBPM_SP_USB0,
    DRV_SBPM_SP_USB1,
    DRV_SBPM_SP_SPARE_0,
    DRV_SBPM_SP_SPARE_1,
    DRV_SBPM_SP_RNR_A,
    DRV_SBPM_SP_RNR_B,
    DRV_SBPM_SP_NUMBER_OF_SOURCE_PORTS
	
}DRV_SBPM_SP_USR;


/******************************************************************************/
/* BPM User Groups index:                                                     */
/******************************************************************************/
typedef enum
{
	DRV_SBPM_USER_GROUP_0,
	DRV_SBPM_USER_GROUP_1,
    DRV_SBPM_USER_GROUP_2,
	DRV_SBPM_USER_GROUP_3,
	DRV_SBPM_USER_GROUP_4,
    DRV_SBPM_USER_GROUP_5,
    DRV_SBPM_USER_GROUP_6,
    DRV_SBPM_USER_GROUP_7,
    DRV_SBPM_NUMBER_OF_USER_GROUPS
	
}DRV_SBPM_USER_GROUP;

#define ME_DRV_SBPM_USER_GROUP_IN_RANGE(v)       ( (v) >= DRV_SBPM_USER_GROUP_0 && (v) < DRV_SBPM_NUMBER_OF_USER_GROUPS )


/******************************************************************************/
/* Boolean                                                                    */
/******************************************************************************/
typedef enum
{
	DRV_SBPM_DISABLE = 0,
	DRV_SBPM_ENABLE = 1
	
}E_DRV_SBPM_ENABLE;

/******************************************************************************/
/* Ack state                                                                   */
/******************************************************************************/
typedef enum
{
	DRV_SBPM_NACK_STATE = 0,
	DRV_SBPM_ACK_STATE = 1
	
}E_DRV_SBPM_ACK_STATE;

/******************************************************************************/
/* Exclusive state                                                            */
/******************************************************************************/
typedef enum
{
	DRV_SBPM_NON_EXCLUSIVE_STATE = 0,
	DRV_SBPM_EXCLUSIVE_STATE = 1
	
}E_DRV_SBPM_EXCLUSIVE_STATE;

/******************************************************************************/
/* Runner selection for receiving status messages                             */
/******************************************************************************/
typedef enum
{
	DRV_SBPM_RUNNER_SELECT_RUNNER_A = 0,
	DRV_SBPM_RUNNER_SELECT_RUNNER_B = 1
	
}DRV_SBPM_RUNNER_SELECT;


/******************************************************************************/
/* SBPM Global Configuration struct                                           */
/******************************************************************************/
typedef struct
{
    /* Global hysteresis */
    uint32_t hysteresis ;

    /* Global threshold */
    uint32_t threshold  ;
} 
DRV_SBPM_GLOBAL_CONFIGURATION ;


/******************************************************************************/
/* BPM User Group mapping struct                                              */
/******************************************************************************/
typedef struct
{
  	DRV_SBPM_USER_GROUP sp_gpon_mapping;
	DRV_SBPM_USER_GROUP sp_emac0_mapping;
	DRV_SBPM_USER_GROUP sp_emac1_mapping;
	DRV_SBPM_USER_GROUP sp_emac2_mapping;
	DRV_SBPM_USER_GROUP sp_emac3_mapping;
	DRV_SBPM_USER_GROUP sp_emac4_mapping;
    DRV_SBPM_USER_GROUP sp_mips_c_mapping;
    DRV_SBPM_USER_GROUP sp_mips_d_mapping;
    DRV_SBPM_USER_GROUP sp_pci0_mapping;
    DRV_SBPM_USER_GROUP sp_pci1_mapping;
    DRV_SBPM_USER_GROUP sp_usb0_mapping;
    DRV_SBPM_USER_GROUP sp_usb1_mapping;
    DRV_SBPM_USER_GROUP sp_rnr_a_mapping;
    DRV_SBPM_USER_GROUP sp_rnr_b_mapping;
} 
DRV_SBPM_USER_GROUP_MAPPING ;

/******************************************************************************/
/* SBPM User Group thresholds struct                                           */
/******************************************************************************/
typedef struct
{
    /* rsv */
    uint32_t rsv2          : 6 ;
    /* UG hysteresis */
    uint32_t ug_hysteresis : 10 ;
    /* reserved */
    uint32_t rsv           : 6  ;
    /*  UG threshold */
    uint32_t ug_threshold  : 10 ;
} 
DRV_SBPM_UG_THRESHOLD ;

/******************************************************************************/
/* SBPM User Group configuration struct                                           */
/******************************************************************************/
typedef struct
{
    /*  threshold */
    uint32_t threshold  ;

    /*  hysteresis */
    uint32_t hysteresis ;

    /*  Exclusive threshold */
    uint32_t exclusive_threshold  ;

    /*  Exclusive hysteresis */
    uint32_t exclusive_hysteresis ;

} 
DRV_SBPM_USER_GROUP_CONFIGURATION ;

/******************************************************************************/
/* SBPM User Group thresholds struct  - contains array of 8 user groups        */
/******************************************************************************/
typedef struct
{
    DRV_SBPM_USER_GROUP_CONFIGURATION ug_arr[ DRV_SBPM_NUMBER_OF_USER_GROUPS ]   ;
} 
DRV_SBPM_USER_GROUPS_THRESHOLDS ;

/******************************************************************************/
/* Source port enable struct                                                  */
/******************************************************************************/
typedef struct
{
    int32_t rnra_sp_enable ;

    int32_t rnrb_sp_enable ;

    int32_t eth0_sp_enable ;

    int32_t eth1_sp_enable ;

    int32_t eth2_sp_enable ;

    int32_t eth3_sp_enable ;

    int32_t eth4_sp_enable ;

    int32_t gpon_or_eth5_sp_enable ;
} 
DRV_SBPM_SP_ENABLE ;

/*******************************************************************************/
/*  Runner message control parameters - struct                                 */
/*******************************************************************************/
typedef struct
{

    /*Task number for runner used in SBPM Status Message Wake-Up */
    uint16_t  runner_reply_wakeup_task_number ;

    /* Enable/Disable wake-up message after each reply on Alloc request from Runner */
    E_DRV_SBPM_ENABLE status_wakeup_enable ;

    /* Enable/Disable  wake-up message after each   transition state of any peripheral */
	E_DRV_SBPM_ENABLE status_message_enable;

    /*select runner for receiving status messages: Runner A or B*/
	DRV_SBPM_RUNNER_SELECT select_runner;

    /*status message base address */
    uint16_t  message_base_address; 

    /*Target address for both Runners*/
    uint16_t  runner_reply_target_address; 

} 
DRV_SBPM_RUNNER_MSG_CTRL_PARAMS ;

/*******************************************************************************/
/*  wakeup reply set parameters - struct                                       */
/*******************************************************************************/
typedef struct
{
    /*Task number for Wake-Up as resultof reply */
    uint16_t  reply_wakeup_task_number ;

    /*Enable/Disable wake-up message on alloc reply message */
    E_DRV_SBPM_ENABLE alloc_reply_wakeup_enable;

    /*Enable/Disable wake-up message on bn connect reply message */
    E_DRV_SBPM_ENABLE connect_reply_wakeup_enable;

    /*Enable/Disable wake-up message on get_next reply message */
    E_DRV_SBPM_ENABLE get_next_reply_wakeup_enable;

    /*Enable/Disable wake-up message on multicast update reply message */
    E_DRV_SBPM_ENABLE mcnt_reply_wakeup_enable;
} 
DRV_SBPM_WAKEUP_REPLY_SET_PARAMS ;

/*******************************************************************************/
/*  MIPS D message control parameters - struct                                 */
/*******************************************************************************/
typedef struct
{
    /*Task number for Wake-Up as resultof Reply on Alloc request*/
    DRV_SBPM_WAKEUP_REPLY_SET_PARAMS  mips_d_wakeup_reply_set ;

    /*Target address for MIPS-D*/
    uint16_t  mips_d_reply_target_address ;

} 
DRV_SBPM_MIPS_D_MSG_CTRL_PARAMS ;

/******************************************************************************/
/* SBPM User Group status struct                                               */
/******************************************************************************/
typedef struct
{
    /*User group status - ack/nack*/
    E_DRV_SBPM_ACK_STATE ug_status ;

    /*User group exclusive status - non-exclusive/exclusive*/
    E_DRV_SBPM_EXCLUSIVE_STATE ug_exclusive_status ;
} 
DRV_SBPM_USER_GROUP_STATUS ;

/******************************************************************************/
/* SBPM Interrupts status struct                                               */
/******************************************************************************/
typedef struct
{
    E_DRV_SBPM_ENABLE bac_underrun ;
	E_DRV_SBPM_ENABLE multicast_counter_overflow;
    E_DRV_SBPM_ENABLE check_last_error;
    E_DRV_SBPM_ENABLE max_search_error;
} 
DRV_SBPM_ISR ;

/******************************************************************************/
/* SBPM Interrupt information struct                                           */
/******************************************************************************/
typedef struct
{
    uint16_t cmd_sa ;
	uint16_t cmd_type ;
    uint32_t cmd_data [2] ; 
} 
DRV_SBPM_IIR ;

/******************************************************************************/
/* SBPM Error Handling parameters struct                                      */
/******************************************************************************/
typedef struct
{
    uint16_t search_deapth ;
	E_DRV_SBPM_ENABLE max_search_enable ;
    E_DRV_SBPM_ENABLE check_last_enable  ; 
    E_DRV_SBPM_ENABLE freeze_counters;
} 
DRV_SBPM_ERROR_HANDLING_PARAMETERS ;

/*******************************************************************************/
/*                                                                             */
/* Functions prototypes                                                        */
/*                                                                             */
/*******************************************************************************/


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_init					                               */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - BPM initialize    	                                           */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function initialize and sets general configuration of the SBPM block.  */
/* SBPM module initialization is made once in the system lifetime              */
/*                                                                             */
/* This API performs the following:                                            */
/*  1.	Disable all SBPM source ports.                                         */
/*  2.	Builds the free linked list using the HW accelerator.                  */
/*  3.	Sets SBPM configuration:                                               */
/*      a.	Global threshold.                                                  */
/*      b.	User group configuration (threshold, hysteresis, exclusive         */
/*          threshold/ hysteresis).                                            */
/*      c.	Sets route address of each source port.                            */
/*      d.	Source port to user group mapping.                                 */
/*                                                                             */
/* Input: 																	   */
/*   xi_base_address - The start address of the BN list                        */
/*   xi_list_size  - The number of buffers in the list                         */
/*   xi_global_configuration - global threshold and hysteresis (struct)        */
/*   xi_user_group_configuration  - user groups threshold and hysteresis (struct)      */
/*   xi_replay_address - runner replay address                                  */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	   DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*     DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAXIMUM_VALUE -        */
/*         the amount of buffers exceeds the maximum value                     */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_init(uint16_t xi_base_address ,
                                          uint16_t xi_list_size,
                                          uint16_t xi_replay_address,
                                          const DRV_SBPM_GLOBAL_CONFIGURATION    * xi_global_configuration,
                                          const DRV_SBPM_USER_GROUPS_THRESHOLDS  * xi_user_group_configuration ) ;


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
DRV_SBPM_ERROR fi_bl_drv_sbpm_sp_enable ( const DRV_SBPM_SP_ENABLE * xi_sp_enable ) ;

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
/* Input:																	   */
/*                                                                             */
/*   xi_global_threshold - Global Threshold for Allocated Buffers              */
/*   xi_global_hystersis - how many BNs need to free in order to get out from  */
/*                         global NACK state                                   */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_global_threshold ( uint16_t xi_global_threshold,
                                                                        uint16_t xi_global_hysteresis );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_global_threshold                                   */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get SBPM Global threshold                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the global Threshold for Allocated Buffers.           */
/*                                                                             */
/* Input:																	   */
/*                                                                             */
/*   xo_global_threshold - Global Threshold for Allocated Buffers              */
/*   xo_global_hystersis - how many BNs need to free in order to get out from  */
/*                         global NACK state                                   */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_global_threshold (uint16_t * const xo_global_threshold,
                                                                       uint16_t * const xo_global_hysteresis );

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
/* Input:																	   */
/*	xi_user_group - user group 														   */			
/*  xi_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_user_group_thresholds (DRV_SBPM_USER_GROUP xi_user_group, 
                                                                            const DRV_SBPM_USER_GROUP_CONFIGURATION * xi_configuration);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_user_group_thresholds                              */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get BPM User Group threshold configuration                    */
/*                                                                             */
/* Abstract:                                                                   */
/* This function returns Threshold for Allocated Buffers of UG                 */
/*                                                                             */
/* Input:																	   */
/*	xi_user_group - user group 														   */			
/*  xo_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_thresholds (DRV_SBPM_USER_GROUP xi_user_group, 
                                                                            DRV_SBPM_USER_GROUP_CONFIGURATION * const  xo_configuration);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_user_group_status                                   */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get User Group Status                                          */
/*                                                                             */
/* Abstruct:                                                                   */
/*  This function returns the ACK/NACK state of and in addition two bits of    */
/*  exclusive status for each User-Group                                       */
/*                                                                             */
/* Input:                                              			               */
/*	xi_user_group - user group 														   */	
/*	xo_user_group_status - User group status (struct)				           */	
/*                                                                             */
/* Output:  error code                                                         */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_status ( DRV_SBPM_USER_GROUP xi_user_group, 
                                                                         DRV_SBPM_USER_GROUP_STATUS * const xo_user_group_status );


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
/* Input:                                              			               */
/*	xi_source_port - One of SBPM source ports		                           */
/*  xi_user_group  - one of SBPM User group 0-7                                */
/*                                                                             */
/* Output:  error code                                                         */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*	 DRV_SBPM_ERROR_INVALID_SOURCE_PORT - invalid source port      */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range */
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_user_group_mapping ( DRV_SBPM_SP_USR  xi_source_port,
                                                                          DRV_SBPM_USER_GROUP xi_user_group );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_user_group_mapping                                  */
/*                                                                             */
/* Title:                                                                      */
/* SBPM driver - Get User Group Mapping                                         */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the Source port mapping to User groups configuration. */
/*                                                                             */
/* Input:                                              			               */
/*	xi_source_port - One of SBPM source ports		                           */
/*  xo_user_group  - associated User group for this source port                */
/*                                                                             */
/* Output:  error code                                                         */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*	 DRV_SBPM_ERROR_INVALID_SOURCE_PORT - invalid source port      */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_mapping (  DRV_SBPM_SP_USR  xi_source_port,
                                                                           DRV_SBPM_USER_GROUP * const xo_user_group );

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
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					      */
/*   DRV_SBPM_ERROR_INVALID_USER_GROUP -user group id out of range*/
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_user_group_counter ( DRV_SBPM_USER_GROUP xi_user_group, 
                                                                          uint16_t * const xo_allocated_bn_counter ) ;


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
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					      */	
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_global_counter (  uint16_t * const xo_global_bn_counter ) ;

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
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					           */	
/*	 DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation	   */	
/*	 DRV_SBPM_ERROR_NO_FREE_BUFFER -SBPM has no free buffer to allocate*/	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_request_buffer( DRV_SBPM_SP_USR xi_source_port,
                                                                 uint16_t * const xo_bn , 
                                                                 DRV_SBPM_USER_GROUP_STATUS * const xo_status);
                                                                 
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
/* Input:	                                                                   */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */														   
/* 	   xi_bn - 10 bits of SRAM buffer pointer value					           */
/*     xo_status - ACK/NACK state and exclusive status after free-buffer       */
/*                                                                             */
/* Output:                                                                     */
/*   error code                                                                */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					           */	
/*	 DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation	   */	
/*   DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER                    */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_free_buffer_without_context( DRV_SBPM_SP_USR xi_source_port,
                                                                              uint16_t  xi_bn, 
                                                                              DRV_SBPM_USER_GROUP_STATUS * const xo_status);


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
/* Input:	                                                                   */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */														   
/* 	   xi_head_bn - Head BN of the packet to be freed 				           */
/* 	   xi_last_bn - Last BN of the packet to be freed				           */
/* 	   xi_size -    Number of BNs of the packet to be freed                    */
/*     xo_status - ACK/NACK state and exclusive status after free-buffer       */
/*                                                                             */
/* Output:                                                                     */
/*   error code                                                                */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					           */	
/*	 DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation	   */	
/*   DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER                    */	                                                        
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_free_buffer_with_context( DRV_SBPM_SP_USR xi_source_port,
                                                                           uint16_t  xi_head_bn, 
                                                                           uint16_t  xi_last_bn,
                                                                           uint16_t  xi_size,
                                                                           DRV_SBPM_USER_GROUP_STATUS * const xo_status);

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
/* Input:																	   */
/*	xi_bn - SBPM pointer for MCNT setting									   */	
/*	xi_mcnt - multicast counter value										   */	
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*	 DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation	   */
/*	 DRV_SBPM_ERROR_MULTICAST_VALUE_EXCEEDS_MAXIMUM_VALUE -        */
/*      multicast value can be in range 0-7                                    */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_mcnt_update(uint16_t xi_bn, 
                                                             uint8_t  xi_multicast_value );


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
/* Input:																	   */
/*	xi_current_bn - current BN pointer 					            	       */	
/*	xo_pointed_bn - Next buffer in the packet list               			   */	
/*  xo_mcnt_value - Next buffer multicast value                                */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*	 DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation	   */	
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_next_bn (uint16_t xi_current_bn, 
                                                              uint16_t * const xo_next_bn ,
                                                              uint8_t  * const xo_mcnt_value );

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
/* Input:																	   */
/*	xi_current_bn - current BN pointer 					            	       */	
/*	xi_pointed_bn - The Buffer to connect to               		        	   */	
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*	 DRV_SBPM_ERROR_SBPM_BUSY - SBPM busy in previous operation	   */
/*   DRV_SBPM_ERROR_TRYING_TO_CONNECT_TO_NULL_POINTER              */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_connect_bn  (uint16_t xi_current_bn, 
                                                              uint16_t xi_pointed_bn );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_error_handling_parameters                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Set Error Handling Parameters	                               */
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
/* Input:                                              			               */
/*                                                                             */
/*	xi_sbpm_error_handling_parameters - SBPM Error handling parameters (struct)*/	
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_error_handling_parameters ( const DRV_SBPM_ERROR_HANDLING_PARAMETERS * xi_sbpm_error_handling_parameters );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_error_handling_parameters                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Get Error Handling Parameters	                               */
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
/* Input:                                              			               */
/*                                                                             */
/*	xo_sbpm_error_handling_parameters - SBPM Error handling parameters (struct)*/	
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_error_handling_parameters (  DRV_SBPM_ERROR_HANDLING_PARAMETERS * const xo_sbpm_error_handling_parameters );


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
/* Input:																	   */
/*                                                                             */
/*  xi_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_runner_msg_ctrl(DRV_SBPM_RUNNER_MSG_CTRL_PARAMS  * xi_runner_messsage_control_parameters);


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
/* Input:																	   */
/*                                                                             */
/*  xo_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_runner_msg_ctrl(DRV_SBPM_RUNNER_MSG_CTRL_PARAMS * const xo_runner_messsage_control_parameters);


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_runner_wakeup_reply_set                            */
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
/* Input:																	   */
/*  xi_wakeup_reply_set_index - set id (0-3)                                   */
/*  xi_runner_wakeup_reply_set - struct                                        */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_runner_wakeup_reply_set (  uint8_t xi_wakeup_reply_set_index ,
                                                                                const DRV_SBPM_WAKEUP_REPLY_SET_PARAMS  * xi_runner_wakeup_reply_set );

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
/* Input:																	   */
/*  xi_wakeup_reply_set_index - set id (0-3)                                   */
/*  xo_runner_wakeup_reply_set - struct                                        */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_runner_wakeup_reply_set (  uint8_t xi_wakeup_reply_set_index ,
                                                                                DRV_SBPM_WAKEUP_REPLY_SET_PARAMS  * xo_runner_wakeup_reply_set );

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
/* Input:																	   */
/*                                                                             */
/*  xi_mips_d_msg_ctrl_params - struct                                         */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_mips_d_msg_ctrl(DRV_SBPM_MIPS_D_MSG_CTRL_PARAMS * xi_mips_d_messsage_control_parameters);

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
/* Input:																	   */
/*                                                                             */
/*  xo_mips_d_msg_ctrl_params - struct                                         */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_mips_d_msg_ctrl(DRV_SBPM_MIPS_D_MSG_CTRL_PARAMS * const xo_mips_d_messsage_control_parameters);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_interrupt_status_register                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Get Interrupt Status Register	                               */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function returns the BPM interrupts status register                    */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  SBPM_ISR                                                                   */
/*                                                                             */
/* Input:                                              			               */
/*                                                                             */
/*	xo_sbpm_isr - BPM Interrupt Status register				                   */	
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_interrupt_status_register ( DRV_SBPM_ISR * const xo_sbpm_isr );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_sbpm_clear_interrupt_status_register                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   SBPM driver - Clear Interrupt Status Register	                          */
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
/*   DRV_SBPM_ERROR - error code                                  */
/*	    DRV_SBPM_ERROR_NO_ERROR - no error 					      */	
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_clear_interrupt_status_register ( void ) ; 

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_set_interrupt_enable_register                           */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Set Interrupt Enable Register	                               */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function sets the BPM interrupts enable register                       */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  SBPM_IER                                                                   */
/*                                                                             */
/* Input:                                              			               */
/*                                                                             */
/*	xi_sbpm_ier - BPM Interrupt Enable register				                   */	
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_set_interrupt_enable_register ( DRV_SBPM_ISR * xi_sbpm_ier );


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_sbpm_get_interrupt_enable_register                          */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  SBPM driver - Get Interrupt Enable Register	                               */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function returns the BPM interrupts enable register                    */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  BPM_IER                                                                    */
/*                                                                             */
/* Input:                                              			               */
/*                                                                             */
/*	xo_sbpm_ier - BPM Interrupt Enable register				                   */	
/*                                                                             */
/* Output:                                                                     */
/*   DRV_SBPM_ERROR - error code                                  */
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					       */	
/*                                                                             */
/*******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_interrupt_enable_register ( DRV_SBPM_ISR * const xo_sbpm_ier );

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
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					      */	
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_generate_interrupt_test_register ( DRV_SBPM_ISR * xi_sbpm_itr )  ;


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
/*	 DRV_SBPM_ERROR_NO_ERROR - no error 					      */	
/*                                                                            */
/******************************************************************************/
DRV_SBPM_ERROR fi_bl_drv_sbpm_get_interrupt_information_register ( DRV_SBPM_IIR * const xo_sbpm_iir )  ;



#endif



