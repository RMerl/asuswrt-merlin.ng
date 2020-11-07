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
#undef USE_SOC_BASE_ADDR
#include "rdp_subsystem_common.h"
#include "rdp_drv_bpm.h"


#ifndef FIRMWARE_INIT
uint8_t *g_bpm_virt_base;
#undef DEVICE_ADDRESS
#define DEVICE_ADDRESS(_a) ((volatile uint8_t * const) (g_bpm_virt_base + ((uint32_t)(_a) & 0xFFFFF)))
#else
extern uint8_t *g_bpm_virt_base;
#endif

/******************************************************************************/
/*                                                                            */
/* Default values definitions                                                 */
/*                                                                            */
/******************************************************************************/

#define CS_DRV_BPM_SINGLE_MEM               ( 2560 )
#define CS_DRV_BPM_BN_ALIGMENT              ( 1536 )
#define CS_DRV_BPM_UG_ALIGMENT              ( 4 )
#define CS_DRV_BPM_UG_MASK                  ( 0x3fff )
#define CS_DRV_BPM_GLOBAL_HYSTERSIS_MASK    ( 0x3fff )
#if defined(DSL_63138)
#define CS_DRV_BPM_GLOBAL_THRESHOLD_MASK    ( 0xf )
#define CS_DRV_BPM_FREE_BUFFER_PTR_MASK     ( 0x7fff )
#define CS_DRV_BPM_MCNT_BUFFER_PTR_MASK     ( 0x7fff )
#else
#define CS_DRV_BPM_GLOBAL_THRESHOLD_MASK    ( 0x7 )
#define CS_DRV_BPM_FREE_BUFFER_PTR_MASK     ( 0x3fff )
#define CS_DRV_BPM_MCNT_BUFFER_PTR_MASK     ( 0x3fff )
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

void init_bpm_virt_base()
{
#ifdef __KERNEL__
    g_bpm_virt_base = (uint8_t*)ioremap((phys_addr_t)FPM_BPM_PHYS_ADDR, FPM_BPM_PHYS_SIZE);
    printk("FPM_BPM phy_addr 0x%x,virt_addr 0x%lx\n", FPM_BPM_PHYS_ADDR, (uintptr_t)g_bpm_virt_base);
#endif
}
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
uint8_t f_get_bit_from_data_register ( int32_t index );

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

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_init                                                         */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - BPM initialize                                                */
/*                                                                            */
/* Abstract:                                                                  */
/* BPM module initialization is made once in the system lifetime              */
/* This API performs the following:                                           */
/*  1.  Write a value to register ram_init.                                   */
/*  2.  Setting Route Addresses to each Source Port                           */
/*  3.  Set BPM global threshold, thresholds for all UG and Exclusive UGs     */
/*      according to input parameters.                                        */
/*  4.  Mapping SP to UGs using registers BPM_UG_MAP_0, BPM_UG_MAP_1.         */
/*This function sets general configuration of the BPM block                   */
/*                                                                            */
/*                                                                            */
/* Input:                                                                     */
/*   global_configuration - global threshold and hysteresis (struct)          */
/*   ug_configuration  - user groups threshold and hysteresis (struct)        */
/*   xi_replay_address - runner replay address                                */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_init(DRV_BPM_GLOBAL_CONFIGURATION *xi_global_configuration,
                                 DRV_BPM_USER_GROUPS_THRESHOLDS *xi_ug_configuration,
                                 uint16_t xi_replay_address,
                                 E_DRV_BPM_SPARE_MESSAGE_FORMAT xi_bpm_spare_message_format)
{
    BPM_MODULE_REGS_RAM_INIT bpm_init;
    uint32_t swapped;
    BPM_MODULE_REGS_BPM_RADDR0 raddr0;
    BPM_MODULE_REGS_BPM_RADDR1 raddr1;
    uint32_t zero = 0;
#ifndef _CFE_
    DRV_BPM_ISR default_interrupts_mask;
    DRV_BPM_ERROR error;
#endif

    /* Write to register ram_init value */
    BPM_MODULE_REGS_RAM_INIT_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_init = *((BPM_MODULE_REGS_RAM_INIT *)&swapped);
    bpm_init.bsy = BPM_MODULE_REGS_RAM_INIT_BSY_READY_VALUE;
    bpm_init.rdy = BPM_MODULE_REGS_RAM_INIT_RDY_BUSY_VALUE;
    swapped = swap4bytes(*(uint32_t*)&bpm_init);
    BPM_MODULE_REGS_RAM_INIT_WRITE(swapped);

    /* setting the route addr */
    BPM_MODULE_REGS_BPM_RADDR0_READ(swapped);
    swapped = swap4bytes(swapped);
    raddr0 = *((BPM_MODULE_REGS_BPM_RADDR0 *)&swapped);

    raddr0.emac0_rx_raddr = 0x1f;
    raddr0.gpon_rx_raddr = 0x01;
    raddr0.runa_raddr = 0x02;
    raddr0.runb_raddr = 0x00;

    swapped = swap4bytes(*(uint32_t*)&raddr0);
    BPM_MODULE_REGS_BPM_RADDR0_WRITE(swapped);

    BPM_MODULE_REGS_BPM_RADDR1_READ(swapped);
    swapped = swap4bytes(swapped);
    raddr1 = *((BPM_MODULE_REGS_BPM_RADDR1 *)&swapped);

    raddr1.emac1_rx_raddr = 0x0f;
    raddr1.emac2_rx_raddr = 0x17;
    raddr1.emac3_rx_raddr = 0x7f;
    raddr1.emac4_rx_raddr = 0x11;

    swapped = swap4bytes(*(uint32_t*)&raddr1);
    BPM_MODULE_REGS_BPM_RADDR1_WRITE(swapped);

#ifndef _CFE_
    /* Disable Interrupts */
    default_interrupts_mask.free_interrupt = DRV_BPM_DISABLE;
    default_interrupts_mask.multicast_counter_interrupt = DRV_BPM_DISABLE;

    error = fi_bl_drv_bpm_set_interrupt_enable_register(&default_interrupts_mask);
    if (error != DRV_BPM_ERROR_NO_ERROR)
        return error;
#endif
    /* reset user groups map to prevent FPM consumption*/
    BPM_MODULE_REGS_BPM_UG_MAP_R0_WRITE(zero);
    BPM_MODULE_REGS_BPM_UG_MAP_R1_WRITE(zero);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_init);

/******************************************************************************/
/* fi_bl_drv_bpm_sp_enable                                                    */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Source Ports Enable                                           */
/*                                                                            */
/* Abstract:                                                                  */
/* Source Ports Enable                                                        */
/*                                                                            */
/* Registers:                                                                 */
/* BPM_SP_EN                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*    xi_source_port - One of the BPM source port: GPON, EMAC0-4, RNR_A/B     */
/*                                                                            */
/*    xi_enable - enable/ disable                                             */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port                  */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_USR xi_source_port,
                                      E_DRV_BPM_ENABLE xi_enable)
{
    BPM_MODULE_REGS_BPM_SP_EN bpm_sp_enable;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_SP_EN_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_sp_enable = *((BPM_MODULE_REGS_BPM_SP_EN *)&swapped);
    bpm_sp_enable.rnra_en = 1;
    bpm_sp_enable.rnrb_en = 1;
    bpm_sp_enable.gpon_en = 1;
    bpm_sp_enable.emac0_en = 1;
    bpm_sp_enable.emac1_en = 1;
    bpm_sp_enable.emac2_en = 1;
    bpm_sp_enable.emac3_en = 1;
    swapped = swap4bytes(*(uint32_t*)&bpm_sp_enable);

    BPM_MODULE_REGS_BPM_SP_EN_WRITE(swapped);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_sp_enable);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_set_global_threshold                                         */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Set BPM Global threshold                                      */
/*                                                                            */
/* Abstract:                                                                  */
/* This function sets the global Threshold for Allocated Buffers.             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_global_threshold - Global Threshold for Allocated Buffers             */
/*   xi_global_hystersis - Global Buffer Allocation Hysteresis threshold      */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_global_threshold(DRV_BPM_GLOBAL_THRESHOLD xi_global_threshold,
                                                 uint32_t xi_global_hysteresis)
{
    uint32_t swapped;
    BPM_MODULE_REGS_BPM_GL_TRSH global_configuration;

    BPM_MODULE_REGS_BPM_GL_TRSH_READ(swapped);
    swapped = swap4bytes(swapped);
    global_configuration = *((BPM_MODULE_REGS_BPM_GL_TRSH *)&swapped);

    global_configuration.gl_bah = xi_global_hysteresis & CS_DRV_BPM_GLOBAL_HYSTERSIS_MASK;
    global_configuration.gl_bat = xi_global_threshold  & CS_DRV_BPM_GLOBAL_THRESHOLD_MASK;

    swapped = swap4bytes(*(uint32_t*)&global_configuration);
    BPM_MODULE_REGS_BPM_GL_TRSH_WRITE(swapped);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_set_global_threshold);

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_global_threshold                                         */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Get BPM Global threshold                                      */
/*                                                                            */
/* Abstract:                                                                  */
/* This function returns the global Threshold for Allocated Buffers.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xo_global_threshold -  Global Threshold for Allocated Buffers            */
/*   xo_global_hysteresis - Global Buffer Allocation Hysteresis threshold     */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_global_threshold(DRV_BPM_GLOBAL_THRESHOLD * const xo_global_threshold,
                                                 uint32_t * const xo_global_hysteresis)
{
    BPM_MODULE_REGS_BPM_GL_TRSH global_configuration;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_GL_TRSH_READ(swapped);
    swapped = swap4bytes(swapped);
    global_configuration = *((BPM_MODULE_REGS_BPM_GL_TRSH *)&swapped);

    *xo_global_threshold = global_configuration.gl_bat;
    *xo_global_hysteresis = global_configuration.gl_bah;

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_global_threshold);
#endif
/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_set_user_group_thresholds                                    */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Set BPM User Group threshold configuration                    */
/*                                                                            */
/* Abstract:                                                                  */
/* Threshold for Allocated Buffers of UG                                      */
/* Ths register also holds UG0 hysteresis value for ACK/NACK transition       */
/* setting                                                                    */
/* This register is affected by soft reset.                                   */
/*                                                                            */
/* Input:                                                                     */
/*   xi_ug - user group                                                       */
/*  xi_configuration - thresholds configuration for the user group (struct)   */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_user_group_thresholds(DRV_BPM_USER_GROUP xi_ug,
                                                      DRV_BPM_USER_GROUP_CONFIGURATION *xi_configuration)
{
    DRV_BPM_UG_THRESHOLD ug_configuration;
    DRV_BPM_UG_THRESHOLD ug_exclusive_configuration;
    uint32_t ug_start_address = BPM_MODULE_REGS_BPM_UG0_TRSH_ADDRESS + CS_DRV_BPM_UG_ALIGMENT * xi_ug;
    uint32_t ug_exclusive_start_address = BPM_MODULE_REGS_BPM_UG0_EXCL_TRSH_ADDRESS + CS_DRV_BPM_UG_ALIGMENT * xi_ug;
    uint32_t swapped;

    READ_32(ug_start_address, swapped);
    swapped = swap4bytes(swapped);
    ug_configuration = *((DRV_BPM_UG_THRESHOLD *)&swapped);

    ug_configuration.ug_hysteresis = xi_configuration->hysteresis & CS_DRV_BPM_UG_MASK;
    ug_configuration.ug_threshold = xi_configuration->threshold & CS_DRV_BPM_UG_MASK;

    swapped = swap4bytes(*(uint32_t*)&ug_configuration);
    WRITE_32(ug_start_address, swapped);

    READ_32(ug_exclusive_start_address, swapped);
    swapped = swap4bytes(swapped);
    ug_exclusive_configuration = *((DRV_BPM_UG_THRESHOLD *)&swapped);

    ug_exclusive_configuration.ug_hysteresis = xi_configuration->exclusive_hysteresis & CS_DRV_BPM_UG_MASK;
    ug_exclusive_configuration.ug_threshold = xi_configuration->exclusive_threshold & CS_DRV_BPM_UG_MASK;

    swapped = swap4bytes(*(uint32_t*)&ug_exclusive_configuration);
    WRITE_32(ug_exclusive_start_address, swapped);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_set_user_group_thresholds);

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_user_group_thresholds                                    */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Get BPM User Group threshold configuration                    */
/*                                                                            */
/* Abstract:                                                                  */
/* This function returns Threshold for Allocated Buffers of UG                */
/*                                                                            */
/* Input:                                                                     */
/*   xi_ug - user group                                                       */
/*   xo_configuration - thresholds configuration for the user group (struct)  */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_thresholds(DRV_BPM_USER_GROUP xi_ug,
                                                      DRV_BPM_USER_GROUP_CONFIGURATION * const xo_configuration)
{

    DRV_BPM_UG_THRESHOLD ug_configuration;
    DRV_BPM_UG_THRESHOLD ug_exclusive_configuration;
    uint32_t ug_start_address = BPM_MODULE_REGS_BPM_UG0_TRSH_ADDRESS + CS_DRV_BPM_UG_ALIGMENT * xi_ug;
    uint32_t ug_exclusive_start_address = BPM_MODULE_REGS_BPM_UG0_EXCL_TRSH_ADDRESS + CS_DRV_BPM_UG_ALIGMENT * xi_ug;
    uint32_t swapped;

    READ_32(ug_start_address, swapped);
    swapped = swap4bytes(swapped);
    ug_configuration = *((DRV_BPM_UG_THRESHOLD *)&swapped);

    xo_configuration->hysteresis = ug_configuration.ug_hysteresis;
    xo_configuration->threshold = ug_configuration.ug_threshold;

    READ_32(ug_exclusive_start_address, swapped);
    swapped = swap4bytes(swapped);
    ug_exclusive_configuration = *((DRV_BPM_UG_THRESHOLD *)&swapped);

    xo_configuration->exclusive_hysteresis = ug_exclusive_configuration.ug_hysteresis;
    xo_configuration->exclusive_threshold = ug_exclusive_configuration.ug_threshold;

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_user_group_thresholds);
#endif

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_user_group_status                                        */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Get User Group Status                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*  This function returns the ACK/NACK state of and in addition two bits of   */
/*  exclusive status for each User-Group                                      */
/*                                                                            */
/* Input:                                                                     */
/*   xi_ug - user group                                                       */
/*   xo_user_group_status - User group status (struct)                        */
/*                                                                            */
/* Output:  error code                                                        */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*   DRV_BPM_ERROR_INVALID_USER_GROUP - invalid user group                    */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_status(DRV_BPM_USER_GROUP xi_ug,
                                                  DRV_BPM_USER_GROUP_STATUS * const xo_user_group_status)
{
    BPM_MODULE_REGS_BPM_UG_STATUS bpm_ug_status_register;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_UG_STATUS_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_ug_status_register = *((BPM_MODULE_REGS_BPM_UG_STATUS *)&swapped);

    switch (xi_ug)
    {
    case DRV_BPM_USER_GROUP_0:
        xo_user_group_status->ug_status = bpm_ug_status_register.ug0_stts;
        xo_user_group_status->ug_exclusive_status = bpm_ug_status_register.ug0_excl_stts;
        break;
    case DRV_BPM_USER_GROUP_1:
        xo_user_group_status->ug_status = bpm_ug_status_register.ug1_stts;
        xo_user_group_status->ug_exclusive_status = bpm_ug_status_register.ug1_excl_stts;
        break;
    case DRV_BPM_USER_GROUP_2:
        xo_user_group_status->ug_status = bpm_ug_status_register.ug2_stts;
        xo_user_group_status->ug_exclusive_status = bpm_ug_status_register.ug2_excl_stts;
        break;
    case DRV_BPM_USER_GROUP_3:
        xo_user_group_status-> ug_status = bpm_ug_status_register.ug3_stts;
        xo_user_group_status-> ug_exclusive_status = bpm_ug_status_register.ug3_excl_stts;
        break;
    case DRV_BPM_USER_GROUP_4:
        xo_user_group_status->ug_status = bpm_ug_status_register.ug4_stts;
        xo_user_group_status->ug_exclusive_status = bpm_ug_status_register.ug4_excl_stts;
        break;
    case DRV_BPM_USER_GROUP_5:
        xo_user_group_status->ug_status = bpm_ug_status_register.ug5_stts;
        xo_user_group_status->ug_exclusive_status = bpm_ug_status_register.ug5_excl_stts;
        break;
    case DRV_BPM_USER_GROUP_6:
        xo_user_group_status->ug_status = bpm_ug_status_register.ug6_stts;
        xo_user_group_status->ug_exclusive_status = bpm_ug_status_register.ug6_excl_stts;
        break;
    case DRV_BPM_USER_GROUP_7:
        xo_user_group_status->ug_status = bpm_ug_status_register.ug7_stts;
        xo_user_group_status->ug_exclusive_status = bpm_ug_status_register.ug7_excl_stts;
        break;
    default:
        return DRV_BPM_ERROR_INVALID_USER_GROUP;
    }
    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_user_group_status);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_set_user_group_mapping                                       */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Set User Group Mapping                                        */
/*                                                                            */
/* Abstract:                                                                  */
/* This function maps a User group for a specific Source port                 */
/*                                                                            */
/* Input:                                                                     */
/*   xi_source_port - One of BPM source ports                                 */
/*   xi_user_group  - one of BPM User group 0-7                               */
/*                                                                            */
/* Output:  error code                                                        */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port                  */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_USR xi_source_port,
                                                   DRV_BPM_USER_GROUP xi_user_group)
{
    BPM_MODULE_REGS_BPM_UG_MAP_R0 bpm_ug_mapping_r0;
    BPM_MODULE_REGS_BPM_UG_MAP_R1 bpm_ug_mapping_r1;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_UG_MAP_R0_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_ug_mapping_r0 = *((BPM_MODULE_REGS_BPM_UG_MAP_R0 *)&swapped);

    BPM_MODULE_REGS_BPM_UG_MAP_R1_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_ug_mapping_r1 = *((BPM_MODULE_REGS_BPM_UG_MAP_R1 *)&swapped);

    switch (xi_source_port)
    {
    case DRV_BPM_SP_MIPS_C:
        bpm_ug_mapping_r0.cpu_ = xi_user_group;
        break;
    case DRV_BPM_SP_EMAC0:
        bpm_ug_mapping_r0.emac0 = xi_user_group;
        break;
    case DRV_BPM_SP_EMAC1:
        bpm_ug_mapping_r0.emac1 = xi_user_group;
        break;
    case DRV_BPM_SP_EMAC2:
        bpm_ug_mapping_r0.emac2 = xi_user_group;
        break;
    case DRV_BPM_SP_EMAC3:
        bpm_ug_mapping_r0.emac3 = xi_user_group;
        break;
    case DRV_BPM_SP_EMAC4:
        bpm_ug_mapping_r1.emac4= xi_user_group;
        break;
    case DRV_BPM_SP_GPON:
        bpm_ug_mapping_r0.gpon = xi_user_group;
        break;
    case DRV_BPM_SP_RNR_A:
        bpm_ug_mapping_r0.rnr_a = xi_user_group;
        break;
    case DRV_BPM_SP_RNR_B:
        bpm_ug_mapping_r0.rnr_b = xi_user_group;
        break;
    case DRV_BPM_SP_USB0:
        bpm_ug_mapping_r1.usb0 = xi_user_group;
        break;
    case DRV_BPM_SP_USB1:
        bpm_ug_mapping_r1.usb1= xi_user_group;
        break;
    case DRV_BPM_SP_PCI0:
        bpm_ug_mapping_r1.pcie0 = xi_user_group;
        break;
    case DRV_BPM_SP_PCI1:
        bpm_ug_mapping_r1.pcie1 = xi_user_group;
        break;
    case DRV_BPM_SP_MIPS_D:
        bpm_ug_mapping_r1.mipsd = xi_user_group;
        break;
    case DRV_BPM_SP_SPARE_0:
        bpm_ug_mapping_r1.spare0 = xi_user_group;
        break;
    case DRV_BPM_SP_SPARE_1:
        bpm_ug_mapping_r1.spare1 = xi_user_group;
        break;
    default:
        return DRV_BPM_ERROR_INVALID_SOURCE_PORT;
    }

    swapped = swap4bytes(*(uint32_t*)&bpm_ug_mapping_r0);
    BPM_MODULE_REGS_BPM_UG_MAP_R0_WRITE(swapped);

    swapped = swap4bytes(*(uint32_t*)&bpm_ug_mapping_r1);
    BPM_MODULE_REGS_BPM_UG_MAP_R1_WRITE(swapped);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_set_user_group_mapping);

#ifndef _CFE_
/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_user_group_mapping                                       */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Get User Group Mapping                                        */
/*                                                                            */
/* Abstract:                                                                  */
/* This function returns the Source port mapping to User groups configuration */
/*                                                                            */
/* Input:                                                                     */
/*   xi_source_port - One of BPM source ports                                 */
/*   xo_user_group  - associated User group for this source port              */
/*                                                                            */
/* Output:  error code                                                        */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*   DRV_BPM_ERROR_INVALID_SOURCE_PORT - invalid source port                  */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_mapping(DRV_BPM_SP_USR xi_source_port,
                                                   DRV_BPM_USER_GROUP * const xo_user_group)
{
    BPM_MODULE_REGS_BPM_UG_MAP_R0 bpm_ug_mapping_r0;
    BPM_MODULE_REGS_BPM_UG_MAP_R1 bpm_ug_mapping_r1;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_UG_MAP_R0_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_ug_mapping_r0 = *((BPM_MODULE_REGS_BPM_UG_MAP_R0 *)&swapped);

    BPM_MODULE_REGS_BPM_UG_MAP_R1_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_ug_mapping_r1 = *((BPM_MODULE_REGS_BPM_UG_MAP_R1 *)&swapped);

    switch (xi_source_port)
    {
    case DRV_BPM_SP_MIPS_C:
        *xo_user_group = bpm_ug_mapping_r0.cpu_;
        break;
    case DRV_BPM_SP_EMAC0:
        *xo_user_group = bpm_ug_mapping_r0.emac0;
        break;
    case DRV_BPM_SP_EMAC1:
        *xo_user_group = bpm_ug_mapping_r0.emac1;
        break;
    case DRV_BPM_SP_EMAC2:
        *xo_user_group = bpm_ug_mapping_r0.emac2;
        break;
    case DRV_BPM_SP_EMAC3:
        *xo_user_group = bpm_ug_mapping_r0.emac3;
        break;
    case DRV_BPM_SP_EMAC4:
        *xo_user_group = bpm_ug_mapping_r1.emac4;
        break;
    case DRV_BPM_SP_GPON:
        *xo_user_group = bpm_ug_mapping_r0.gpon;
        break;
    case DRV_BPM_SP_RNR_A:
        *xo_user_group = bpm_ug_mapping_r0.rnr_a;
        break;
    case DRV_BPM_SP_RNR_B:
        *xo_user_group = bpm_ug_mapping_r0.rnr_b;
        break;
    case DRV_BPM_SP_USB0:
        *xo_user_group = bpm_ug_mapping_r1.usb0;
        break;
    case DRV_BPM_SP_USB1:
        *xo_user_group = bpm_ug_mapping_r1.usb1;
        break;
    case DRV_BPM_SP_PCI0:
        *xo_user_group = bpm_ug_mapping_r1.pcie0;
        break;
    case DRV_BPM_SP_PCI1:
        *xo_user_group = bpm_ug_mapping_r1.pcie1;
        break;
    case DRV_BPM_SP_MIPS_D:
        *xo_user_group = bpm_ug_mapping_r1.mipsd;
        break;
    case DRV_BPM_SP_SPARE_0:
        *xo_user_group = bpm_ug_mapping_r1.spare0;
        break;
    case DRV_BPM_SP_SPARE_1:
        *xo_user_group = bpm_ug_mapping_r1.spare1;
        break;
    default:
        return DRV_BPM_ERROR_INVALID_SOURCE_PORT;
    }

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_user_group_mapping);
#endif
/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_req_buffer                                                   */
/*                                                                            */
/* Title:                                                                     */
/* BPM driver - Request Buffer                                                */
/*                                                                            */
/* Abstract:                                                                  */
/* cpu requests a free buffer pointer                                         */
/*                                                                            */
/* Input:                                                                     */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf   */
/*     another port                                                           */
/*     xo_bn - returned 14 bits of DDR buffer pointer value                   */
/*                                                                            */
/* Output:  error code                                                        */
/*     DRV_BPM_ERROR_NO_ERROR - no error                                      */
/*     DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation                */
/*     DRV_BPM_ERROR_NO_FREE_BUFFER - BPM has no free buffer                  */
/*                                              to allocate                   */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_USR xi_source_port,
                                       uint32_t * const xo_bn)
{

    BPM_MODULE_REGS_REQ_PTR req_ptr;
    uint32_t swapped;
    int32_t num_count = 10;

    BPM_MODULE_REGS_REQ_PTR_READ(swapped);
    swapped = swap4bytes(swapped);
    req_ptr = *((BPM_MODULE_REGS_REQ_PTR *)&swapped);
    req_ptr.sp_addr = xi_source_port;
    swapped = swap4bytes(*(uint32_t*)&req_ptr);
    BPM_MODULE_REGS_REQ_PTR_WRITE(req_ptr);

    while (num_count-- > 0)
    {
        BPM_MODULE_REGS_REQ_PTR_READ(req_ptr);
        if (req_ptr.bsy != BPM_MODULE_REGS_REQ_PTR_BSY_BUSY_VALUE)
        {
            break;
        }
    }

    if (num_count == -1)
    {
        return DRV_BPM_ERROR_BPM_BUSY;
    }
    else
    {
        if (req_ptr.nack_status == BPM_MODULE_REGS_REQ_PTR_NACK_STATUS_CPU_IN_NACK_STATE_VALUE )
        {
            return DRV_BPM_ERROR_NO_FREE_BUFFER;
        }
        else /* buffer number is valid */
        {
            *xo_bn = req_ptr.bn;
#if defined(DSL_63138)
            *xo_bn |= req_ptr.bn_bit14 << 14;
#endif
        }
    }

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_req_buffer);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_free_buffer                                                  */
/*                                                                            */
/* Title:                                                                     */
/* BPM Driver - Free pointer                                                  */
/*                                                                            */
/* Abstract:                                                                  */
/* cpu request to free an occupied pointer                                    */
/*                                                                            */
/* Input:                                                                     */
/*     xi_source_port - used by CPU for Buffer Request allocation on behalf   */
/*     another port                                                           */
/*     xi_bn - 14 bits of DDR buffer pointer value                            */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code :                                             */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*   DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation                  */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_USR xi_source_port,
                                        uint32_t xi_bn)
{
    BPM_MODULE_REGS_FREE_PTR bpm_free_ptr;
    uint32_t swapped;
    int32_t num_count = 10;

    BPM_MODULE_REGS_FREE_PTR_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_free_ptr = *((BPM_MODULE_REGS_FREE_PTR *)&swapped);

    bpm_free_ptr.bn = xi_bn & CS_DRV_BPM_FREE_BUFFER_PTR_MASK;
    bpm_free_ptr.own_sa = xi_source_port & CS_DRV_BPM_FREE_BUFFER_OWNER_MASK;

    swapped = swap4bytes(*(uint32_t*)&bpm_free_ptr);
    BPM_MODULE_REGS_FREE_PTR_WRITE(bpm_free_ptr);

    while (num_count-- > 0)
    {
        BPM_MODULE_REGS_FREE_PTR_READ(swapped);
        swapped = swap4bytes(swapped);
        bpm_free_ptr = *((BPM_MODULE_REGS_FREE_PTR *)&swapped);

        if (bpm_free_ptr.bsy != BPM_MODULE_REGS_FREE_PTR_BSY_REQUEST__IN_PROGRESS_VALUE)
        {
            break;
        }
    }

    if (num_count == -1)
    {
        return DRV_BPM_ERROR_BPM_BUSY;
    }

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_free_buffer);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_mcnt_update                                                  */
/*                                                                            */
/* Title:                                                                     */
/* Multi Cast Counter set for pointer                                         */
/*                                                                            */
/* Abstract:                                                                  */
/* This function is responsible to increment Multicast value (3 bits)         */
/* for an occupied buffer pointer.                                            */
/* Remark: Multicast counter value is a delta between 1 and the total number  */
/* to duplicate (i.e.: if we want to duplicate a packet 4 times, counter=3).  */
/*                                                                            */
/* Input:                                                                     */
/*   xi_bn - BPM pointer for MCNT setting                                     */
/*   xi_mcnt - multicast counter value                                        */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*   DRV_BPM_ERROR_BPM_BUSY - BPM busy in previous operation                  */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_mcnt_update(uint32_t xi_bn,
                                        uint32_t xi_mcnt)
{
    BPM_MODULE_REGS_MCNT_PTR bpm_mcnt_ptr;
    uint32_t swapped;

    BPM_MODULE_REGS_MCNT_PTR_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_mcnt_ptr = *((BPM_MODULE_REGS_MCNT_PTR *)&swapped);

    /* prevision request in progress */
    if (bpm_mcnt_ptr.bsy == BPM_MODULE_REGS_MCNT_PTR_BSY_REQUEST_IN_PROGRESS_VALUE)
    {
        return (DRV_BPM_ERROR_BPM_BUSY);
    }

    bpm_mcnt_ptr.bn = xi_bn & CS_DRV_BPM_MCNT_BUFFER_PTR_MASK;
    bpm_mcnt_ptr.mcnt = xi_mcnt & CS_DRV_BPM_MCNT_VALUE_MASK;

    swapped = swap4bytes(*(uint32_t*)&bpm_mcnt_ptr);
    BPM_MODULE_REGS_MCNT_PTR_WRITE(bpm_mcnt_ptr);

    return (DRV_BPM_ERROR_NO_ERROR);
}
EXPORT_SYMBOL(fi_bl_drv_bpm_mcnt_update);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_set_runner_msg_ctrl                                          */
/*                                                                            */
/* Title:                                                                     */
/* BPM Driver - Set Runner message control                                    */
/*                                                                            */
/* Abstract:                                                                  */
/* Enables runner wake-up messages,                                           */
/* select control bit for transition message and task numbers for wake-up     */
/* messages to Runners                                                        */
/*                                                                            */
/* Registers:                                                                 */
/* BPM_RNR_MSG_CTRL,BPM_RNR_RPLY_TA                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*  xi_runner_messsage_control_parameters - struct                            */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/*******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_runner_msg_ctrl(DRV_BPM_RUNNER_MSG_CTRL_PARAMS *xi_runner_msg_ctrl_params)
{
    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL runner_message_control;
    BPM_MODULE_REGS_BPM_RNR_RPLY_TA bpm_rnr_rply_ta_register;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_READ(swapped);
    swapped = swap4bytes(swapped);
    runner_message_control = *((BPM_MODULE_REGS_BPM_RNR_MSG_CTRL *)&swapped);

    runner_message_control.rnr_rply_wkup_en = (xi_runner_msg_ctrl_params->reply_wakeup_enable ==  1) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    runner_message_control.rnr_trans_wkup_en = (xi_runner_msg_ctrl_params->transition_wakeup_enable == 1) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    runner_message_control.rnr_sel_trans_msg = ( xi_runner_msg_ctrl_params->select_transition_msg == 1) ?
        BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_RNR_SEL_TRANS_MSG_TRANSITION_MESSAGE_SELECTED_TO_RUNNER_B_VALUE:
        BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_RNR_SEL_TRANS_MSG_TRANSITION_MESSAGE_SELECTED_TO_RUNNER_A_VALUE;

    runner_message_control.rnr_a_rply_wkup_tn = xi_runner_msg_ctrl_params->runner_a_reply_wakeup_task_number & CS_DRV_BPM_WAKEUP_TN_MASK;
    runner_message_control.rnr_b_rply_wkup_tn = xi_runner_msg_ctrl_params->runner_b_reply_wakeup_task_number & CS_DRV_BPM_WAKEUP_TN_MASK;
    runner_message_control.rnr_trans_wkup_tn = xi_runner_msg_ctrl_params->runner_transition_wakeup_task_number & CS_DRV_BPM_WAKEUP_TN_MASK;
    swapped = swap4bytes(*(uint32_t*)&runner_message_control);

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_WRITE(swapped);

    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_rnr_rply_ta_register = *((BPM_MODULE_REGS_BPM_RNR_RPLY_TA *)&swapped);

    bpm_rnr_rply_ta_register.rnr_a_ta = xi_runner_msg_ctrl_params->runner_a_reply_target_address;
    bpm_rnr_rply_ta_register.rnr_b_ta = xi_runner_msg_ctrl_params->runner_b_reply_target_address;
    swapped = swap4bytes(*(uint32_t*)&bpm_rnr_rply_ta_register);

    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_WRITE(swapped);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_set_runner_msg_ctrl);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_runner_msg_ctrl                                          */
/*                                                                            */
/* Title:                                                                     */
/* BPM Driver - Get Runner message control                                    */
/*                                                                            */
/* Abstract:                                                                  */
/* Enables runner wake-up messages,                                           */
/* select control bit for transition message and task numbers for wake-up     */
/* messages to Runners                                                        */
/*                                                                            */
/* Registers:                                                                 */
/* BPM_RNR_MSG_CTRL,BPM_RNR_RPLY_TA                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*  xo_runner_messsage_control_parameters - struct                            */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_runner_msg_ctrl(DRV_BPM_RUNNER_MSG_CTRL_PARAMS * const xo_runner_msg_ctrl_params)
{
    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL runner_message_control;
    BPM_MODULE_REGS_BPM_RNR_RPLY_TA bpm_rnr_rply_ta_register;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_READ(swapped);
    swapped = swap4bytes(swapped);
    runner_message_control = *((BPM_MODULE_REGS_BPM_RNR_MSG_CTRL *)&swapped);

    xo_runner_msg_ctrl_params->reply_wakeup_enable = (runner_message_control.rnr_rply_wkup_en == 1) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    xo_runner_msg_ctrl_params->transition_wakeup_enable = (runner_message_control.rnr_trans_wkup_en == 1) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    xo_runner_msg_ctrl_params->select_transition_msg = (runner_message_control.rnr_sel_trans_msg == 1) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    xo_runner_msg_ctrl_params->runner_a_reply_wakeup_task_number = runner_message_control.rnr_a_rply_wkup_tn;
    xo_runner_msg_ctrl_params->runner_b_reply_wakeup_task_number = runner_message_control.rnr_b_rply_wkup_tn;
    xo_runner_msg_ctrl_params->runner_transition_wakeup_task_number = runner_message_control.rnr_trans_wkup_tn;

    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_rnr_rply_ta_register = *((BPM_MODULE_REGS_BPM_RNR_RPLY_TA *)&swapped);

    xo_runner_msg_ctrl_params->runner_a_reply_target_address = (uint16_t)bpm_rnr_rply_ta_register.rnr_a_ta;
    xo_runner_msg_ctrl_params->runner_b_reply_target_address = (uint16_t)bpm_rnr_rply_ta_register.rnr_b_ta;

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_runner_msg_ctrl);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_set_mips_d_msg_ctrl                                          */
/*                                                                            */
/* Title:                                                                     */
/* BPM Driver - Set Runner message control                                    */
/*                                                                            */
/* Abstract:                                                                  */
/* This function sets the target address for Reply message and the            */
/* task numbers for wake-up messages for MIPSD                                */
/*                                                                            */
/* Registers:                                                                 */
/* BPM_RNR_MSG_CTRL,BPM_MIPSD_RPLY_TA                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*  xi_mips_d_msg_ctrl_params - struct                                        */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_mips_d_msg_ctrl(DRV_BPM_MIPS_D_MSG_CTRL_PARAMS *xi_mips_d_msg_ctrl_params)
{
    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL runner_message_control;
    BPM_MODULE_REGS_BPM_MIPSD_RPLY_TA mips_d_rply_ta_register;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_READ(swapped);
    swapped = swap4bytes(swapped);
    runner_message_control = *((BPM_MODULE_REGS_BPM_RNR_MSG_CTRL *)&swapped);

    runner_message_control.mipsd_rply_wkup_en = (xi_mips_d_msg_ctrl_params->mips_d_reply_wakeup_enable ==  1) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;

    runner_message_control.mipsd_rply_tn = xi_mips_d_msg_ctrl_params->mips_d_reply_wakeup_task_number;
    swapped = swap4bytes(*(uint32_t*)&runner_message_control);

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_WRITE(swapped);

    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_READ(swapped);
    swapped = swap4bytes(swapped);
    mips_d_rply_ta_register = *((BPM_MODULE_REGS_BPM_MIPSD_RPLY_TA *)&swapped);

    mips_d_rply_ta_register.rply_ta = xi_mips_d_msg_ctrl_params->mips_d_reply_target_address;
    swapped = swap4bytes(*(uint32_t*)&mips_d_rply_ta_register);

    BPM_MODULE_REGS_BPM_MIPSD_RPLY_TA_WRITE(swapped);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_set_mips_d_msg_ctrl );

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_mips_d_msg_ctrl                                          */
/*                                                                            */
/* Title:                                                                     */
/* BPM Driver - Get Runner message control                                    */
/*                                                                            */
/* Abstract:                                                                  */
/* This function returns the target address for Reply message and the         */
/* task numbers for wake-up messages for MIPSD                                */
/*                                                                            */
/* Registers:                                                                 */
/* BPM_RNR_MSG_CTRL,BPM_MIPSD_RPLY_TA                                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*  xo_mips_d_msg_ctrl_params - struct                                        */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_mips_d_msg_ctrl(DRV_BPM_MIPS_D_MSG_CTRL_PARAMS * const xo_mips_d_msg_ctrl_params)
{
    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL runner_message_control;
    BPM_MODULE_REGS_BPM_MIPSD_RPLY_TA mips_d_rply_ta_register;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_READ(swapped);
    swapped = swap4bytes(swapped);
    runner_message_control = *((BPM_MODULE_REGS_BPM_RNR_MSG_CTRL *)&swapped);

    xo_mips_d_msg_ctrl_params->mips_d_reply_wakeup_enable = (runner_message_control.mipsd_rply_wkup_en ==  1) ?
        DRV_BPM_ENABLE : DRV_BPM_DISABLE;
    xo_mips_d_msg_ctrl_params->mips_d_reply_wakeup_task_number = (uint16_t)runner_message_control.mipsd_rply_tn;

    BPM_MODULE_REGS_BPM_RNR_RPLY_TA_READ(swapped);
    swapped = swap4bytes(swapped);
    mips_d_rply_ta_register = *((BPM_MODULE_REGS_BPM_MIPSD_RPLY_TA *)&swapped);

    xo_mips_d_msg_ctrl_params->mips_d_reply_target_address = (uint16_t)mips_d_rply_ta_register.rply_ta;

    return (DRV_BPM_ERROR_NO_ERROR);
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_mips_d_msg_ctrl);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_interrupt_status_register                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*  BPM driver - Get Interrupt Status Register                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* This function returns the BPM interrupts status register                   */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*  BPM_ISR                                                                   */
/*                                                                            */
/* Input:                                                                     */
/*   xo_bpm_isr - BPM Interrupt Status register                               */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_interrupt_status_register(DRV_BPM_ISR * const xo_bpm_isr)
{
    BPM_MODULE_REGS_BPM_ISR bpm_isr;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_RNR_MSG_CTRL_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_isr = *((BPM_MODULE_REGS_BPM_ISR *)&swapped);

    xo_bpm_isr->free_interrupt = bpm_isr.free_isr;
    xo_bpm_isr->multicast_counter_interrupt = bpm_isr.mcnt_isr;

    return (DRV_BPM_ERROR_NO_ERROR);

}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_interrupt_status_register);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_clear_interrupt_status_register                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BPM driver - Clear Interrupt Status Register                             */
/*                                                                            */
/* Abstract:                                                                  */
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
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_clear_interrupt_status_register(void)
{
    BPM_MODULE_REGS_BPM_ISR bpm_isr;
    uint32_t swapped;

    bpm_isr.free_isr = CS_HIGH;
    bpm_isr.mcnt_isr = CS_HIGH;
    bpm_isr.rsv = BPM_MODULE_REGS_BPM_ISR_RSV_RSV_VALUE;

    swapped = swap4bytes(*(uint32_t*)&bpm_isr);
    BPM_MODULE_REGS_BPM_ISR_WRITE(swapped);

    return (DRV_BPM_ERROR_NO_ERROR);
}
EXPORT_SYMBOL(fi_bl_drv_bpm_clear_interrupt_status_register);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_set_interrupt_enable_register                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*  BPM driver - Set Interrupt Enable Register                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* This function sets the BPM interrupts enable register                      */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*  BPM_IER                                                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_bpm_ier - BPM Interrupt Enable register                               */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_set_interrupt_enable_register(DRV_BPM_ISR *xi_bpm_ier)
{
    BPM_MODULE_REGS_BPM_ISR bpm_ier;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_IER_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_ier = *((BPM_MODULE_REGS_BPM_ISR *)&swapped);

    bpm_ier.free_isr = xi_bpm_ier->free_interrupt;
    bpm_ier.mcnt_isr = xi_bpm_ier->multicast_counter_interrupt;

    swapped = swap4bytes(*(uint32_t*)&bpm_ier);
    BPM_MODULE_REGS_BPM_IER_WRITE(swapped);

    return DRV_BPM_ERROR_NO_ERROR;
}
EXPORT_SYMBOL(fi_bl_drv_bpm_set_interrupt_enable_register);

/******************************************************************************/
/*                                                                            */
/* fi_bl_drv_bpm_get_interrupt_enable_register                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*  BPM driver - Get Interrupt Enable Register                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/* This function returns the BPM interrupts enable register                   */
/*                                                                            */
/* Registers:                                                                 */
/*                                                                            */
/*  BPM_IER                                                                   */
/*                                                                            */
/* Input:                                                                     */
/*   xo_bpm_ier - BPM Interrupt Enable register                               */
/*                                                                            */
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_interrupt_enable_register(DRV_BPM_ISR * const xo_bpm_ier)
{
    BPM_MODULE_REGS_BPM_ISR bpm_ier;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_IER_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_ier = *((BPM_MODULE_REGS_BPM_ISR *)&swapped);

    xo_bpm_ier->free_interrupt = bpm_ier.free_isr;
    xo_bpm_ier->multicast_counter_interrupt = bpm_ier.mcnt_isr;

    return (DRV_BPM_ERROR_NO_ERROR);
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_interrupt_enable_register);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_generate_interrupt_test_register                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac_Driver_BPM - Generate Interrupt test register                      */
/*                                                                            */
/* Abstract:                                                                  */
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
/* Output:                                                                    */
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_generate_interrupt_test_register(DRV_BPM_ISR *xi_bpm_itr)
{
    BPM_MODULE_REGS_BPM_ISR bpm_itr;
    uint32_t swapped;

    BPM_MODULE_REGS_BPM_IER_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_itr = *((BPM_MODULE_REGS_BPM_ISR *)&swapped);

    bpm_itr.free_isr = xi_bpm_itr->free_interrupt;
    bpm_itr.mcnt_isr = xi_bpm_itr->multicast_counter_interrupt;

    swapped = swap4bytes(*(uint32_t*)&bpm_itr);
    BPM_MODULE_REGS_BPM_IER_WRITE(swapped);

    return (DRV_BPM_ERROR_NO_ERROR);
}
EXPORT_SYMBOL(fi_bl_drv_bpm_generate_interrupt_test_register);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_get_user_group_counter                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac_Driver_BPM - Get User Group Counter                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the number of allocated BN of a specific User Group*/
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
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_user_group_counter(DRV_BPM_USER_GROUP xi_ug,
                                                   uint16_t * const xo_allocated_bn_counter)
{
    BPM_MODULE_REGS_BPM_UG0_BAC user_group_counter;
    uint32_t ug_counter_address = BPM_MODULE_REGS_BPM_UG0_BAC_ADDRESS + CS_DRV_BPM_UG_ALIGMENT * xi_ug;

    READ_32(ug_counter_address, user_group_counter);

    *xo_allocated_bn_counter = (uint16_t)user_group_counter.ug0bac;

    return (DRV_BPM_ERROR_NO_ERROR);
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_user_group_counter);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bpm_get_buffer_number_status                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Lilac_Driver_BPM - Get Buffer Number Status                              */
/*                                                                            */
/* Abstract:                                                                  */
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
/*   DRV_BPM_ERROR - error code                                               */
/*   DRV_BPM_ERROR_NO_ERROR - no error                                        */
/*                                                                            */
/******************************************************************************/
DRV_BPM_ERROR fi_bl_drv_bpm_get_buffer_number_status(uint16_t xi_buffer_number,
                                                     uint8_t * const xo_bn_status)
{
    BPM_MODULE_REGS_READ_RAM_ADDR bpm_read_address;
    uint16_t bpm_sys_bn = xi_buffer_number;
    uint8_t bpm_sys_addr_range = 0;
    uint16_t bpm_internal_aligment_value = 0;
    uint16_t bpm_internal_bn = 0;
    uint8_t memory_sel = 0;
    uint8_t address_field = 0;
    uint16_t bits_index = 0;
    uint8_t temp = 0;
    int32_t index;
    uint32_t swapped;

    /* encoding RAM index according to system buffer number */
    while (bpm_sys_bn > CS_DRV_BPM_SINGLE_MEM)
    {
        bpm_sys_bn -= CS_DRV_BPM_SINGLE_MEM;
        bpm_sys_addr_range++;
    }

    /* appropriate internal_aligment_val according to RAM index */
    bpm_internal_aligment_value = (uint16_t)(bpm_sys_addr_range * CS_DRV_BPM_BN_ALIGMENT);

    /* Calculate internal buffer number - 15 bit */
    bpm_internal_bn = xi_buffer_number + bpm_internal_aligment_value;

    memory_sel = (bpm_internal_bn & CS_DRV_BPM_MEM_SELECT_MASK) >> 12;
    address_field = (bpm_internal_bn & CS_DRV_BPM_ADDRESS_FIELD_MASK) >> 5;
    bits_index = (bpm_internal_bn & CS_DRV_BPM_BITS_INDEX_MASK) * CS_DRV_BPM_NUM_OF_BITS_FOR_BN;

    BPM_MODULE_REGS_READ_RAM_ADDR_READ(swapped);
    swapped = swap4bytes(swapped);
    bpm_read_address = *((BPM_MODULE_REGS_READ_RAM_ADDR *)&swapped);

    bpm_read_address.selram = memory_sel;
    bpm_read_address.ramaddr = address_field;

    swapped = swap4bytes(*(uint32_t*)&bpm_read_address);
    BPM_MODULE_REGS_READ_RAM_ADDR_WRITE(swapped);

    for (index = bits_index + 2; index >= bits_index; index--)
    {
        temp = temp << 1;
        temp |= f_get_bit_from_data_register(index);
    }

    *xo_bn_status = temp;

    return (DRV_BPM_ERROR_NO_ERROR);
}
EXPORT_SYMBOL(fi_bl_drv_bpm_get_buffer_number_status );


/*********************************************************************/
/*                   Static Function Implementation                  */
/*********************************************************************/

uint8_t f_get_bit_from_data_register(int32_t index)
{
    BPM_MODULE_REGS_READ_RAM_DATA0 bpm_read_data;
    uint32_t swapped;

    if (index < 32)
    {
        BPM_MODULE_REGS_READ_RAM_DATA0_READ(swapped);
        swapped = swap4bytes(swapped);
        bpm_read_data = *((BPM_MODULE_REGS_READ_RAM_DATA0 *)&swapped);
    }
    else if (index < 64)
    {
        BPM_MODULE_REGS_READ_RAM_DATA1_READ(swapped);
        swapped = swap4bytes(swapped);
        bpm_read_data = *((BPM_MODULE_REGS_READ_RAM_DATA0 *)&swapped);
    }
    else
    {
        BPM_MODULE_REGS_READ_RAM_DATA2_READ(swapped);
        swapped = swap4bytes(swapped);
        bpm_read_data = *((BPM_MODULE_REGS_READ_RAM_DATA0 *)&swapped);
    }

    return (MS_DRV_BPM_GET_BIT_I((uint16_t)bpm_read_data.data, (uint32_t)(index % 32)));
}

#ifndef RDP_SIM
extern int BcmMemReserveGetByName(char *name, void **virt, uint64_t* phys, uint32_t *size);

static void *tm_ddr_base;
static void *tm_ddr_end;
static uint32_t tm_pbuf_size;
static uint32_t tm_pbuf_offset;
#endif

static inline void *bdmf_get_tm_ddr_base(void)
{
#ifndef RDP_SIM
    uint32_t tm_ddr_size = 0;
    int rc = 0;

    if (tm_ddr_base)
        return tm_ddr_base;

    rc = BcmMemReserveGetByName(BUFFER_MEMORY_BASE_ADDR_STR, &tm_ddr_base, NULL, &tm_ddr_size);
    if (rc == -1)
        printk( "Failed to get valid DDR TM address\n");

    tm_ddr_end = tm_ddr_base + tm_ddr_size - 1;

    return tm_ddr_base;
#else
    return 0;
#endif
}


/** Initialize platform buffer support
 * \param[in]   size    buffer size
 * \param[in]   offset  min offset
 */
void bdmf_pbuf_init(uint32_t size, uint32_t offset)
{
#ifndef RDP_SIM
    /* FIXME!!! need to get the reserved memory from board driver */
    bdmf_get_tm_ddr_base();
    tm_pbuf_size = size;
    tm_pbuf_offset = offset;
#endif
}

/** Allocate pbuf and fill with data
 * The function allocates platform buffer and copies data into it
 * \param[in]   data        data pointer
 * \param[in]   length      data length
 * \param[in]   source      source port as defined by RDD
 * \param[out]  pbuf        Platform buffer
 * \return 0 if OK or error < 0
 */
int bdmf_pbuf_alloc(void *data, uint32_t length, uint16_t source, bdmf_pbuf_t *pbuf)
{
#ifndef RDP_SIM
    uint32_t bn;
    void *buffer_ptr;

    if (!tm_pbuf_size)
        return DRV_BPM_ERROR_NO_FREE_BUFFER;

    if (tm_pbuf_offset + length > tm_pbuf_size)
        return DRV_BPM_ERROR_NO_FREE_BUFFER;

    if (fi_bl_drv_bpm_req_buffer(source, &bn))
        return DRV_BPM_ERROR_NO_FREE_BUFFER;

    /* FIXME!!! need to get the reserved memory from board driver */
    bdmf_get_tm_ddr_base();
    buffer_ptr = bdmf_get_tm_ddr_base() + bn * tm_pbuf_size + tm_pbuf_offset;
    /* ToDo: copy via cache */
    memcpy(buffer_ptr, data, length);
    pbuf->bpm_bn = bn;
    pbuf->length = length;
    pbuf->source = source;
    pbuf->data = buffer_ptr;
    pbuf->offset = tm_pbuf_offset;
#endif

    return 0;
}

