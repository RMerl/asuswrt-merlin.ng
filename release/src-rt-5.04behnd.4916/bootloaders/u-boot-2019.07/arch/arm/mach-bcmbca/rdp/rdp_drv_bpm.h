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
/* This header file defines all datatypes and functions exported for the      */
/* Lilac BPM driver.                                                          */
/*                                                                            */
/******************************************************************************/

#ifndef LILAC_DRV_BPM_H_INCLUDED
#define LILAC_DRV_BPM_H_INCLUDED


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "rdp_bpm.h"

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Error codes returned by BPM driver APIs                                    */
/******************************************************************************/
typedef enum
{
	DRV_BPM_ERROR_NO_ERROR = 0,
    DRV_BPM_ERROR_BPM_BUSY ,
    DRV_BPM_ERROR_NO_FREE_BUFFER,
    DRV_BPM_ERROR_INVALID_SOURCE_PORT,
    DRV_BPM_ERROR_INVALID_USER_GROUP
	
}DRV_BPM_ERROR;


/******************************************************************************/
/* BPM source ports                                                           */
/******************************************************************************/
typedef enum
{
	DRV_BPM_SP_GPON = 0,
	DRV_BPM_SP_EMAC0,
	DRV_BPM_SP_EMAC1,
	DRV_BPM_SP_EMAC2,
	DRV_BPM_SP_EMAC3,
	DRV_BPM_SP_EMAC4,
    DRV_BPM_SP_MIPS_C,
    DRV_BPM_SP_MIPS_D,
    DRV_BPM_SP_PCI0,
    DRV_BPM_SP_PCI1,
    DRV_BPM_SP_USB0,
    DRV_BPM_SP_USB1,
    DRV_BPM_SP_SPARE_0,
    DRV_BPM_SP_SPARE_1,
    DRV_BPM_SP_RNR_A,
    DRV_BPM_SP_RNR_B
	
}DRV_BPM_SP_USR;

/******************************************************************************/
/* BPM Global threshold values                                                */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
	DRV_BPM_GLOBAL_THRESHOLD_2_5K = 0,
	DRV_BPM_GLOBAL_THRESHOLD_5K,
    DRV_BPM_GLOBAL_THRESHOLD_7_5K,
	DRV_BPM_GLOBAL_THRESHOLD_10K,
	DRV_BPM_GLOBAL_THRESHOLD_12_5K,
	DRV_BPM_GLOBAL_THRESHOLD_15K,
    DRV_BPM_GLOBAL_THRESHOLD_17_5K,
    DRV_BPM_GLOBAL_THRESHOLD_20K,
    DRV_BPM_GLOBAL_THRESHOLD_22_5K,
    DRV_BPM_GLOBAL_THRESHOLD_25K,
    DRV_BPM_GLOBAL_THRESHOLD_27_5K,
    DRV_BPM_GLOBAL_THRESHOLD_30K
	
}DRV_BPM_GLOBAL_THRESHOLD;

/******************************************************************************/
/* BPM User Groups index:                                                     */
/******************************************************************************/
typedef enum
{
	DRV_BPM_USER_GROUP_0,
	DRV_BPM_USER_GROUP_1,
    DRV_BPM_USER_GROUP_2,
	DRV_BPM_USER_GROUP_3,
	DRV_BPM_USER_GROUP_4,
    DRV_BPM_USER_GROUP_5,
    DRV_BPM_USER_GROUP_6,
    DRV_BPM_USER_GROUP_7,
    DRV_BPM_NUMBER_OF_USER_GROUPS
	
}DRV_BPM_USER_GROUP;

/******************************************************************************/
/* Boolean                                                                    */
/******************************************************************************/
typedef enum
{
	DRV_BPM_DISABLE = 0,
	DRV_BPM_ENABLE = 1
	
}E_DRV_BPM_ENABLE;

/******************************************************************************/
/* BPM Global Configuration struct                                            */
/******************************************************************************/
typedef struct
{
    /* Global hysteresis */
    uint32_t hysteresis ;

    /* Global threshold */
    DRV_BPM_GLOBAL_THRESHOLD threshold  ;
} 
DRV_BPM_GLOBAL_CONFIGURATION ;


/******************************************************************************/
/* BPM User Group mapping struct                                              */
/******************************************************************************/
typedef struct
{
  	DRV_BPM_USER_GROUP sp_gpon_mapping;
	DRV_BPM_USER_GROUP sp_emac0_mapping;
	DRV_BPM_USER_GROUP sp_emac1_mapping;
	DRV_BPM_USER_GROUP sp_emac2_mapping;
	DRV_BPM_USER_GROUP sp_emac3_mapping;
	DRV_BPM_USER_GROUP sp_emac4_mapping;
    DRV_BPM_USER_GROUP sp_mips_c_mapping;
    DRV_BPM_USER_GROUP sp_mips_d_mapping;
    DRV_BPM_USER_GROUP sp_pci0_mapping;
    DRV_BPM_USER_GROUP sp_pci1_mapping;
    DRV_BPM_USER_GROUP sp_usb0_mapping;
    DRV_BPM_USER_GROUP sp_usb1_mapping;
    DRV_BPM_USER_GROUP sp_rnr_a_mapping;
    DRV_BPM_USER_GROUP sp_rnr_b_mapping;
} 
DRV_BPM_USER_GROUP_MAPPING ;

/******************************************************************************/
/* BPM User Group thresholds struct                                           */
/******************************************************************************/
#if defined(DSL_63138)
#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
typedef struct
{
    /* rsv */
    uint32_t rsv2          : 1 ;
    /* UG hysteresis */
    uint32_t ug_hysteresis : 15 ;
    /* reserved */
    uint32_t rsv           : 1  ;
    /*  UG threshold */
    uint32_t ug_threshold  : 15 ;
} 
DRV_BPM_UG_THRESHOLD ;
#else
typedef struct
{
    /*  UG threshold */
    uint32_t ug_threshold  : 15 ;
    /* reserved */
    uint32_t rsv           : 1  ;
    /* UG hysteresis */
    uint32_t ug_hysteresis : 15 ;
    /* rsv */
    uint32_t rsv2          : 1 ;
} 
DRV_BPM_UG_THRESHOLD ;
#endif
#else // DSL_63138
#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
typedef struct
{
    /* rsv */
    uint32_t rsv2          : 2 ;
    /* UG hysteresis */
    uint32_t ug_hysteresis : 14 ;
    /* reserved */
    uint32_t rsv           : 2  ;
    /*  UG threshold */
    uint32_t ug_threshold  : 14 ;
} 
DRV_BPM_UG_THRESHOLD ;
#else
typedef struct
{
    /*  UG threshold */
    uint32_t ug_threshold  : 14 ;
    /* reserved */
    uint32_t rsv           : 2  ;
    /* UG hysteresis */
    uint32_t ug_hysteresis : 14 ;
    /* rsv */
    uint32_t rsv2          : 2 ;
} 
DRV_BPM_UG_THRESHOLD ;
#endif
#endif // DSL_63138

/******************************************************************************/
/* BPM User Group configuration struct                                           */
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
DRV_BPM_USER_GROUP_CONFIGURATION ;

/******************************************************************************/
/* BPM User Group thresholds struct  - contains array of 8 user groups        */
/******************************************************************************/
typedef struct
{
    DRV_BPM_USER_GROUP_CONFIGURATION ug_arr[ DRV_BPM_NUMBER_OF_USER_GROUPS ]   ;
} 
DRV_BPM_USER_GROUPS_THRESHOLDS ;

/*******************************************************************************/
/*  Runner message control parameters - struct                                 */
/*******************************************************************************/
typedef struct
{
    /* Enable/Disable wake-up message after each reply on *Alloc request from Runner */
    E_DRV_BPM_ENABLE reply_wakeup_enable ;

    /* Enable/Disable  wake-up message after each   transition state of any peripheral */
	E_DRV_BPM_ENABLE transition_wakeup_enable;

    /*select for sending transition message: Runner A or B*/
	E_DRV_BPM_ENABLE select_transition_msg;

    /*Task number for Runner A Wake-Up  as result of Reply on Alloc request*/
	uint32_t  runner_a_reply_wakeup_task_number;

    /*Task number for Runner B Wake-Up  as result of Reply on Alloc request*/
	uint32_t  runner_b_reply_wakeup_task_number;

    /*Task number for any slected Runner Wake-Up as result of Transition*/
	uint32_t  runner_transition_wakeup_task_number;

    /*Target address for Runner A*/
    uint16_t  runner_a_reply_target_address; 

    /*Target address for Runner B*/
    uint16_t  runner_b_reply_target_address;
} 
DRV_BPM_RUNNER_MSG_CTRL_PARAMS ;

/*******************************************************************************/
/*  MIPS D message control parameters - struct                                 */
/*******************************************************************************/
typedef struct
{
    /*Enable/Disable wake-up message */
    E_DRV_BPM_ENABLE mips_d_reply_wakeup_enable;

    /*Task number for Runner A Wake-Up as resultof Reply on Alloc request*/
    uint16_t  mips_d_reply_wakeup_task_number ;

    /*Target address for MIPS-D*/
    uint16_t  mips_d_reply_target_address ;
} 
DRV_BPM_MIPS_D_MSG_CTRL_PARAMS ;


/******************************************************************************/
/* BPM User Group status struct                                               */
/******************************************************************************/
typedef struct
{
    /*User group status - ack/nack*/
    E_DRV_BPM_ENABLE ug_status ;

    /*User group exclusive status - non-exclusive/exclusive*/
    E_DRV_BPM_ENABLE ug_exclusive_status ;
} 
DRV_BPM_USER_GROUP_STATUS ;

/******************************************************************************/
/* BPM Interrupts status struct                                               */
/******************************************************************************/
typedef struct
{
    E_DRV_BPM_ENABLE free_interrupt ;
	E_DRV_BPM_ENABLE multicast_counter_interrupt;
} 
DRV_BPM_ISR ;

typedef enum
{
    DRV_SPARE_BN_MESSAGE_FORMAT_14_bit_BN_WIDTH ,
    DRV_SPARE_BN_MESSAGE_FORMAT_15_bit_BN_WIDTH
} 
E_DRV_BPM_SPARE_MESSAGE_FORMAT ;

/*******************************************************************************/
/*                                                                             */
/* Functions prototypes                                                        */
/*                                                                             */
/*******************************************************************************/


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_init					                                   */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - BPM initialize    	                                           */
/*                                                                             */
/* Abstruct:                                                                   */
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
                                         E_DRV_BPM_SPARE_MESSAGE_FORMAT xi_bpm_spare_message_format ) ;

/*******************************************************************************/
/*fi_bl_drv_bpm_sp_enable					                               */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Source Ports Enable	                                           */
/*                                                                             */
/* Abstruct:                                                                   */
/* Source Ports Enable	                                                       */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_SP_EN                                                                   */
/*                                                                             */
/* Input:																	   */
/*                                                                             */
/*    xi_source_port - One of the BPM source port: GPON, EMAC0-4, RNR_A/B      */
/*                                                                             */
/*    xi_enable - enable/ disable                                              */
/*                                                                             */
/* Output:  error code                                                         */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*	 DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port   	   */	
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port       */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_USR xi_source_port, 
                                                         E_DRV_BPM_ENABLE xi_enable );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_global_threshold                                    */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Set BPM Global threshold                                       */
/*                                                                             */
/* Abstruct:                                                                   */
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
                                                                      uint32_t xi_global_hysteresis );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_global_threshold                                    */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Get BPM Global threshold                                       */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the global Threshold for Allocated Buffers.           */
/*                                                                             */
/* Input:																	   */
/*                                                                             */
/*   xo_global_threshold -  Global Threshold for Allocated Buffers             */
/*   xo_global_hysteresis - Global Buffer Allocation Hysteresis threshold      */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_global_threshold (DRV_BPM_GLOBAL_THRESHOLD * const xo_global_threshold,
                                                                     uint32_t * const xo_global_hysteresis );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_user_group_thresholds                               */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Set BPM User Group threshold configuration                     */
/*                                                                             */
/* Abstruct:                                                                   */
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
                                                                          DRV_BPM_USER_GROUP_CONFIGURATION * xi_configuration);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_user_group_thresholds                               */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Get BPM User Group threshold configuration                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns Threshold for Allocated Buffers of UG                 */
/*                                                                             */
/* Input:																	   */
/*	xi_ug - user group 														   */			
/*  xo_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_thresholds (DRV_BPM_USER_GROUP xi_ug, 
                                                                          DRV_BPM_USER_GROUP_CONFIGURATION * const  xo_configuration);


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_user_group_mapping                                  */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Set User Group Mapping                                         */
/*                                                                             */
/* Abstruct:                                                                   */
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
                                                                        DRV_BPM_USER_GROUP xi_user_group );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_req_buffer                                              */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Request Buffer                                                 */
/*                                                                             */
/* Abstruct:                                                                   */
/* cpu requests a free buffer pointer                                          */
/*                                                                             */
/* Input:                                                                      */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */
/*     xo_bn - returned 14 bits of DDR buffer pointer value                    */
/*                                                                             */
/* Output:  error code                                                         */
/*	 DRV_BPM_ERROR_NO_ERROR - no error 					           */	
/*	 DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation		   */	
/*	 DRV_BPM_ERROR_NO_FREE_BUFFER - BPM has no free buffer to allocate*/	
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_USR xi_source_port,
                                                          uint32_t * const xo_bn);


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_runner_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Set Runner message control                                     */
/*                                                                             */
/* Abstruct:                                                                   */
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
DRV_BPM_ERROR fi_bl_drv_bpm_set_runner_msg_ctrl(DRV_BPM_RUNNER_MSG_CTRL_PARAMS  * xi_runner_messsage_control_parameters);


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_runner_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Get Runner message control                                     */
/*                                                                             */
/* Abstruct:                                                                   */
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
DRV_BPM_ERROR fi_bl_drv_bpm_get_runner_msg_ctrl(DRV_BPM_RUNNER_MSG_CTRL_PARAMS * const xo_runner_messsage_control_parameters);

#endif

