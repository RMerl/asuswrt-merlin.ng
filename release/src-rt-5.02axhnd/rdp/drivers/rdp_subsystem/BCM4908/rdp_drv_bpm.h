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
#pragma pack(push,1)
typedef struct
{
    uint32_t token_size:12 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t token_index:17 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t ddr:1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t reserved:1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t token_valid:1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}rdp_fpm_index;
#pragma pack(pop)
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
void init_bpm_virt_base(void);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_init                                                      */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - BPM initialize                                                 */
/*                                                                             */
/* Abstruct:                                                                   */
/* BPM module initialization is made once in the system lifetime               */
/* This API performs the following:                                            */
/*  1.  Write a value to register ram_init.                                    */
/*  2.  Setting Route Addresses to each Source Port                            */
/*  3.  Set BPM global threshold, thresholds for all UG and Exclusive UGs      */
/*      according to input parameters.                                         */
/*  4.  Mapping SP to UGs using registers BPM_UG_MAP_0, BPM_UG_MAP_1.          */
/*This function sets general configuration of the BPM block                    */
/*                                                                             */
/*                                                                             */
/* Input:                                                                      */
/*   global_configuration - global threshold and hysteresis (struct)           */
/*   ug_configuration  - user groups threshold and hysteresis (struct)         */
/*   xi_replay_address - runner replay address                                  */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_init(DRV_BPM_GLOBAL_CONFIGURATION    * xi_global_configuration,
                                         DRV_BPM_USER_GROUPS_THRESHOLDS  * xi_ug_configuration,
                                         uint16_t                             xi_replay_address,
                                         E_DRV_BPM_SPARE_MESSAGE_FORMAT xi_bpm_spare_message_format ) ;

/*******************************************************************************/
/*fi_bl_drv_bpm_sp_enable                                                  */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Source Ports Enable                                            */
/*                                                                             */
/* Abstruct:                                                                   */
/* Source Ports Enable                                                         */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_SP_EN                                                                   */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*    xi_source_port - One of the BPM source port: GPON, EMAC0-4, RNR_A/B      */
/*                                                                             */
/*    xi_enable - enable/ disable                                              */
/*                                                                             */
/* Output:  error code                                                         */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port       */
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
/* Input:                                                                      */
/*                                                                             */
/*   xi_global_threshold - Global Threshold for Allocated Buffers              */
/*   xi_global_hystersis - Global Buffer Allocation Hysteresis threshold       */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
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
/* Input:                                                                      */
/*                                                                             */
/*   xo_global_threshold -  Global Threshold for Allocated Buffers             */
/*   xo_global_hysteresis - Global Buffer Allocation Hysteresis threshold      */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
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
/* Input:                                                                      */
/*  xi_ug - user group                                                         */
/*  xi_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
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
/* Input:                                                                      */
/*  xi_ug - user group                                                         */
/*  xo_configuration - thresholds configuration for the user group (struct)    */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_thresholds (DRV_BPM_USER_GROUP xi_ug,
                                                                          DRV_BPM_USER_GROUP_CONFIGURATION * const  xo_configuration);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_user_group_status                                   */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Get User Group Status                                          */
/*                                                                             */
/* Abstruct:                                                                   */
/*  This function returns the ACK/NACK state of and in addition two bits of    */
/*  exclusive status for each User-Group                                       */
/*                                                                             */
/* Input:                                                                      */
/*  xi_ug - user group                                                         */
/*  xo_user_group_status - User group status (struct)                          */
/*                                                                             */
/* Output:  error code                                                         */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*   DRV_BPM_ERROR_INVALID_USER_GROUP - invalid user group         */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_status ( DRV_BPM_USER_GROUP xi_ug,
                                                                       DRV_BPM_USER_GROUP_STATUS * const xo_user_group_status );


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
/* Input:                                                                      */
/*  xi_source_port - One of BPM source ports                                   */
/*  xi_user_group  - one of BPM User group 0-7                                 */
/*                                                                             */
/* Output:  error code                                                         */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port       */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_user_group_mapping ( DRV_BPM_SP_USR  xi_source_port,
                                                                        DRV_BPM_USER_GROUP xi_user_group );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_user_group_mapping                                  */
/*                                                                             */
/* Title:                                                                      */
/* BPM driver - Get User Group Mapping                                         */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the Source port mapping to User groups configuration. */
/*                                                                             */
/* Input:                                                                      */
/*  xi_source_port - One of BPM source ports                                   */
/*  xo_user_group  - associated User group for this source port                */
/*                                                                             */
/* Output:  error code                                                         */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port       */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_mapping (  DRV_BPM_SP_USR  xi_source_port,
                                                                         DRV_BPM_USER_GROUP * const xo_user_group );

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
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*   DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation           */
/*   DRV_BPM_ERROR_NO_FREE_BUFFER - BPM has no free buffer to allocate*/
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_USR xi_source_port,
                                                          uint32_t * const xo_bn);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_free_buffer                                             */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Free pointer                                                   */
/*                                                                             */
/* Abstruct:                                                                   */
/* cpu request to free an occupied pointer                                     */
/*                                                                             */
/* Input:                                                                      */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf    */
/*     another port                                                            */
/*  xi_bn - 13 bits of DDR buffer pointer value                                */
/*                                                                             */
/* Output:                                                                     */
/*   error code                                                                */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*   DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation           */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_USR xi_source_port,
                                                           uint32_t  xi_bn);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_mcnt_update                                             */
/*                                                                             */
/* Title:                                                                      */
/* Multi Cast Counter set for pointer                                          */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function is responsible to increment Multicast value (3 bits)          */
/* for an occupied buffer pointer.                                             */
/* Remark: Multicast counter value is a delta between 1 and the total number   */
/* to duplicate (i.e.: if we want to duplicate a packet 4 times, counter=3).   */
/*                                                                             */
/* Input:                                                                      */
/*  xi_bn - BPM pointer for MCNT setting                                       */
/*  xi_mcnt - multicast counter value                                          */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*   DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation       */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_mcnt_update(uint32_t xi_bn,
                                                           uint32_t xi_mcnt );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_runner_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Set Runner message control                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* Enables runner wake-up messages,                                            */
/* select control bit for transition message and task numbers for wake-up      */
/* messages to Runners                                                         */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_RNR_MSG_CTRL,BPM_RNR_RPLY_TA                                            */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*  xi_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
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
/* Enables runner wake-up messages,                                            */
/* select control bit for transition message and task numbers for wake-up      */
/* messages to Runners                                                         */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_RNR_MSG_CTRL,BPM_RNR_RPLY_TA                                            */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*  xo_runner_messsage_control_parameters - struct                             */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_runner_msg_ctrl(DRV_BPM_RUNNER_MSG_CTRL_PARAMS * const xo_runner_messsage_control_parameters);


/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_mips_d_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Set Runner message control                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function sets the target address for Reply message and the             */
/* task numbers for wake-up messages for MIPSD                                 */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_RNR_MSG_CTRL,BPM_MIPSD_RPLY_TA                                          */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*  xi_mips_d_msg_ctrl_params - struct                                         */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_mips_d_msg_ctrl(DRV_BPM_MIPS_D_MSG_CTRL_PARAMS * xi_mips_d_messsage_control_parameters);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_mips_d_msg_ctrl                                     */
/*                                                                             */
/* Title:                                                                      */
/* BPM Driver - Get Runner message control                                     */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function returns the target address for Reply message and the          */
/* task numbers for wake-up messages for MIPSD                                 */
/*                                                                             */
/* Registers:                                                                  */
/* BPM_RNR_MSG_CTRL,BPM_MIPSD_RPLY_TA                                          */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*  xo_mips_d_msg_ctrl_params - struct                                         */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_mips_d_msg_ctrl(DRV_BPM_MIPS_D_MSG_CTRL_PARAMS * const xo_mips_d_messsage_control_parameters);

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_interrupt_status_register                           */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  BPM driver - Get Interrupt Status Register                                 */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function returns the BPM interrupts status register                    */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  BPM_ISR                                                                    */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*  xo_bpm_isr - BPM Interrupt Status register                                 */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_interrupt_status_register ( DRV_BPM_ISR * const xo_bpm_isr );

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_set_interrupt_enable_register                           */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  BPM driver - Set Interrupt Enable Register                                 */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function sets the BPM interrupts enable register                       */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  BPM_IER                                                                    */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*  xi_bpm_ier - BPM Interrupt Enable register                                 */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_interrupt_enable_register ( DRV_BPM_ISR * xi_bpm_ier );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_clear_interrupt_status_register                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BPM driver - Clear Interrupt Status Register                             */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function clear the interrupt status register                        */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    BPM_ISR.                                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                  */
/*      DRV_BPM_ERROR_NO_ERROR - no error                         */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_clear_interrupt_status_register ( void ) ;

/*******************************************************************************/
/*                                                                             */
/* fi_bl_drv_bpm_get_interrupt_enable_register                           */
/*                                                                             */
/* Title:                                                                      */
/*                                                                             */
/*  BPM driver - Get Interrupt Enable Register                                 */
/*                                                                             */
/* Abstruct:                                                                   */
/*                                                                             */
/* This function returns the BPM interrupts enable register                    */
/*                                                                             */
/* Registers:                                                                  */
/*                                                                             */
/*  BPM_IER                                                                    */
/*                                                                             */
/* Input:                                                                      */
/*                                                                             */
/*  xo_bpm_ier - BPM Interrupt Enable register                                 */
/*                                                                             */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                             */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_interrupt_enable_register ( DRV_BPM_ISR * const xo_bpm_ier );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_generate_interrupt_test_register                      */
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
/*   xi_bpm_itr - BMP Isr struct                                              */
/*                                                                            */
/* Output:                                                                     */
/*   DRV_BPM_ERROR - error code                                   */
/*   DRV_BPM_ERROR_NO_ERROR - no error                             */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_generate_interrupt_test_register ( DRV_BPM_ISR * xi_bpm_itr )  ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_get_user_group_counter                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac_Driver_BPM - Get User Group Counter                                */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function returns the number of allocated BN of a specific User Group*/
/*                                                                            */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*    bpm_ug0_bac-bpm_ug7_bac                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ug - User group 0-7                                                   */
/*   xo_allocated_bn_counter - UG counter for allocated BNs                   */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                  */
/*   DRV_BPM_ERROR_NO_ERROR - no error                            */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_counter (DRV_BPM_USER_GROUP xi_ug,
                                                                       uint16_t * const xo_allocated_bn_counter ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_get_buffer_number_status                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac_Driver_BPM - Get Buffer Number Status                              */
/*                                                                            */
/* Abstruct:                                                                  */
/*                                                                            */
/*   This function returns the status of spesific buffer number - if it is    */
/*   occupied, and if it is occupied to how many ports                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_buffer_number - 14 bits of DDR buffer pointer value                   */
/*   xo_bn_status - Status of buffer number: occupied by 0-7 ports            */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                  */
/*   DRV_BPM_ERROR_NO_ERROR - no error                            */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_buffer_number_status (uint16_t xi_buffer_number,
                                                                         uint8_t * const xo_bn_status );
/** Platform buffer */
typedef struct
{
    void *data;             /**< Buffer pointer */
    uint16_t bpm_bn;        /**< Buffer number */
    uint16_t source;        /**< Buffer source */
    uint16_t offset;        /**< Buffer offset */
    uint16_t length;        /**< Buffer length */
} bdmf_pbuf_t;

/** Initialize platform buffer support
 * \param[in]   size    buffer size
 * \param[in]   offset  min offset
 */
void bdmf_pbuf_init(uint32_t size, uint32_t offset);

static inline void bdmf_pbuf_free(bdmf_pbuf_t *pbuf)
{
    fi_bl_drv_bpm_free_buffer(pbuf->source, pbuf->bpm_bn);
}

/** Allocate pbuf and fill with data
 * The function allocates platform buffer and copies data into it
 * \param[in]   data        data pointer
 * \param[in]   length      data length
 * \param[in]   source      source port
 * \param[out]  pbuf        Platform buffer
 * \return 0 if OK or error < 0
 */
int bdmf_pbuf_alloc(void *data, uint32_t length, uint16_t source, bdmf_pbuf_t *pbuf);

#endif
