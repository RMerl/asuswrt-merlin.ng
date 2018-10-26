/*
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>

*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  63148_common.h                                           */
/*   DATE:    09/05/13                                                 */
/*   PURPOSE: Register definition used by assembly for BCM63148        */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM63148_MAP_COMMON_H
#define __BCM63148_MAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*
#####################################################################
# System PLL Control Register
#####################################################################
*/


/*
#####################################################################
# GPIO Control Registers
#####################################################################
*/
#define GPIO_DATA_HI               (GPIO_BASE + 0x8)
#define GPIO_DATA                  (GPIO_BASE + 0xC)
#define GP_OFFSET                  0x54

/*
#####################################################################
# Perf/Interrupt Registers
#####################################################################
*/
#define PERF_CLKCONTROL                         0x04
#define PERF_CLKCONTROL_FAP0_CLKEN              (1<<7)
#define PERF_CLKCONTROL_FAP1_CLKEN              (1<<8)
#define PERF_TIMER_CONTROL			0x08
#define PERF_TIMER_CONTROL_SOFTRST		(1<<0)
#define PERF_SOFTRESETB                         0x10
#define PERF_SOFTRESETB_SOFT_RST_FAP0_N         (1 << 13)
#define PERF_SOFTRESETB_SOFT_RST_FAP1_N         (1 << 16)

/*
#####################################################################
# Timer Registers
#####################################################################
*/

#define TIMER_CLKRSTCTL                         0x2c
#define TIMER_CLKRSTCTL_FAP1_PLL_CLKEN          (1<<11)
#define TIMER_CLKRSTCTL_FAP2_PLL_CLKEN          (1<<15)

/*
#####################################################################
# Secure Boot Bootrom Registers
#####################################################################
*/

#define BROM_SEC_ACCESS_CNTRL                   0x00

/* #define BROM_SEC_ACCESS_CNTRL_DISABLE_BTRM      0x0c  */
#define BROM_SEC_ACCESS_CNTRL_DISABLE_BTRM      0x00    /* Permenantly disable r/w to bootrom addr space and BROM_SEC_ACCESS_CNTRL register */

#define BROM_SEC_SECBOOTCFG                     0x14
#define BROM_SEC_SECBOOTCFG_JTAG_UNLOCK         (1<<1)


/*
#####################################################################
# OTP Control / Status Registers
#####################################################################
*/

#define JTAG_OTP_GENERAL_CTRL_0                 0x00
#define JTAG_OTP_GENERAL_CTRL_0_START           (1 << 0)
#define JTAG_OTP_GENERAL_CTRL_0_PROG_EN         (1 << 21)
#define JTAG_OTP_GENERAL_CTRL_0_ACCESS_MODE     (2 << 22)

#define JTAG_OTP_GENERAL_CTRL_1                 0x04
#define JTAG_OTP_GENERAL_CTRL_1_CPU_MODE        (1 << 0)

#define JTAG_OTP_GENERAL_CTRL_2                 0x08

#define JTAG_OTP_GENERAL_CTRL_3                 0x0c

#define JTAG_OTP_GENERAL_STATUS_0               0x14

#define JTAG_OTP_GENERAL_STATUS_1               0x18
#define JTAG_OTP_GENERAL_STATUS_1_CMD_DONE      (1 << 1)

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

#define OTP_BRCM_MEK_MIV_ROW                    17
#define OTP_BRCM_MEK_MIV_SHIFT                  7
#define OTP_BRCM_MEK_MIV_MASK                   (7 << OTP_BRCM_MEK_MIV_SHIFT)

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (7 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 24 */
#define OTP_CUST_MFG_MRKTID_ROW                 24
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)

#define OTP_CUST_OP_INUSE_ROW                   24
#define OTP_CUST_OP_INUSE_SHIFT                 16
#define OTP_CUST_OP_INUSE_MASK                  (1 << OTP_CUST_OP_INUSE_SHIFT)

/* row 25 */
#define OTP_CUST_OP_MRKTID_ROW                  25
#define OTP_CUST_OP_MRKTID_SHIFT                0
#define OTP_CUST_OP_MRKTID_MASK                 (0xffff << OTP_CUST_OP_MRKTID_SHIFT)

/* fuse rows (same as get rows) */
#define OTP_CUST_BTRM_BOOT_ENABLE_FUSE_ROW      OTP_CUST_BTRM_BOOT_ENABLE_ROW
#define OTP_CUST_OP_INUSE_FUSE_ROW              OTP_CUST_OP_INUSE_ROW
#define OTP_CUST_OP_MRKTID_FUSE_ROW             OTP_CUST_OP_MRKTID_ROW
#define OTP_CUST_MFG_MRKTID_FUSE_ROW            OTP_CUST_MFG_MRKTID_ROW

/*
#####################################################################
# VDSL Control Register
#####################################################################
*/

#define VDSL_PHY_RESET_BIT                      (1 << 2)
#define VDSL_MIPS_RESET_BIT                     (1 << 1)
#define VDSL_MIPS_POR_RESET_BIT                 (1 << 0)

/*
#####################################################################
# Miscellaneous Registers
#####################################################################
*/
#define MISC_VDSL_CONTROL                       0x0
#define MISC_VDSL_CONTROL_VDSL_MIPS_RESET       (1<<1) 
#define MISC_VDSL_CONTROL_VDSL_MIPS_POR_RESET   (1<<0)

#define MISC_MEMC_CONTROL                       0x10
#define MISC_MEMC_CONTROL_MC_UBUS_ASYNC_MODE    (1<<3)
#define MISC_MEMC_CONTROL_MC_LMB_ASYNC_MODE     (1<<2)

#define MISC_STRAP_BUS                          0x04

#define MISC_STRAP_BUS_SW_RSRVD_0_SHIFT         12
#define MISC_STRAP_BUS_SW_RSRVD_0_MASK          (0x1<<MISC_STRAP_BUS_SW_RSRVD_0_SHIFT)

#define MISC_STRAP_BUS_PMCROM_BOOT_SHIFT        10
#define MISC_STRAP_BUS_PMCROM_BOOT              (1<<MISC_STRAP_BUS_PMCROM_BOOT_SHIFT) /* pmc rom boot enable */

#define MISC_STRAP_BUS_BOOTSEL_SHIFT            4
#define MISC_STRAP_BUS_BOOTSEL_MASK             (0x1f<<MISC_STRAP_BUS_BOOTSEL_SHIFT)

#define MISC_STRAP_BUS_BOOT_SPINOR_MASK         0x18    /* 0b11xxx are all serial spi nor, therefore bottom three bits are don't care*/
#define MISC_STRAP_BUS_BOOT_SPINOR              0x18    /* valid spi nor values: 0b11xxx */


#define MISC_VREG_CONTROL0                      0x1C
#define MISC_VREG_CONTROL0_REG_RESET_B          (1<<31)
#define MISC_VREG_CONTROL0_OVERCUR_SEL_2_SHIFT	18
#define MISC_VREG_CONTROL0_OVERCUR_SEL_2_MASK	0x3
#define MISC_VREG_CONTROL0_OVERCUR_SEL_1_SHIFT	16
#define MISC_VREG_CONTROL0_OVERCUR_SEL_1_MASK	0x3
#define MISC_VREG_CONTROL0_NOVL_2_SHIFT		4
#define MISC_VREG_CONTROL0_NOVL_2_MASK		0xf
#define MISC_VREG_CONTROL0_NOVL_1_SHIFT		0
#define MISC_VREG_CONTROL0_NOVL_1_MASK		0xf
    
#define MISC_VREG_CONTROL1                      0x20
#define MISC_VREG_CONTROL1_VCM2_ADJ_SHIFT       9
#define MISC_VREG_CONTROL1_VCM1_ADJ_SHIFT       0
#define MISC_VREG_CONTROL1_VCM2_ADJ_MASK        0x1FF
#define MISC_VREG_CONTROL1_VCM1_ADJ_MASK        0x1FF

#define MISC_VREG_CONTROL2                      0x24
#define MISC_VREG_CONTROL2_SWITCHCLOCKEN        (1<<7)

#define MISC_IDDQ_CONTROL                       0x4c
#define MISC_IDDQ_CONTROL_IDDQ_CTRL_FAP         (1<<11)
#define MISC_IDDQ_CTRL_VDSL_PHY_BIT             (1 << 9)
#define MISC_IDDQ_CTRL_PCM_BIT                  (1 << 7)

/*
#####################################################################
# SOTP Registers
#####################################################################
*/
#define SOTP_OTP_PROG_CTRL                      0x00

#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT   8
#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN         (1 << SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT)

#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT 9
#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC      (1 << SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT)

#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT 15
#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN      (1 << SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT)

#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT     17
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK      (0xf << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT) /* bits 20:17 */
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN           (0xa << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)
#define SOTP_OTP_PROG_CTRL_REGS_ECC_DIS          (0x5 << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)

/* --- */

#define SOTP_OTP_WDATA_0                        0x04

/* --- */

#define SOTP_OTP_WDATA_1                        0x08
#define SOTP_OTP_WDATA_1_FAIL_SHIFT             7
#define SOTP_OTP_WDATA_1_FAIL_MASK              (0x3 << SOTP_OTP_WDATA_1_FAIL_SHIFT)

/* --- */

#define SOTP_OTP_ADDR                           0x0c
#define SOTP_OTP_ADDR_OTP_ADDR_SHIFT            6

/* --- */

#define SOTP_OTP_CTRL_0                         0x10

#define SOTP_OTP_CTRL_0_OTP_CMD_SHIFT           1
#define SOTP_OTP_CTRL_0_OTP_CMD_MASK            (0x1f << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) /* bits [05:01] */
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ        (0x0 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT)
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_PROG_ENABLE (0x2 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT)
#define SOTP_OTP_CTRL_0_OTP_CMD_PROG            (0xa << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT)

#define SOTP_OTP_CTRL_0_START_SHIFT             0
#define SOTP_OTP_CTRL_0_START                   (0x1 << SOTP_OTP_CTRL_0_START_SHIFT)

/* --- */

#define SOTP_OTP_STATUS_1                       0x1c

#define SOTP_OTP_STATUS_1_CMD_DONE_SHIFT        1
#define SOTP_OTP_STATUS_1_CMD_DONE              (0x1 << SOTP_OTP_STATUS_1_CMD_DONE_SHIFT)

#define SOTP_OTP_STATUS_1_ECC_COR_SHIFT         16
#define SOTP_OTP_STATUS_1_ECC_COR               (0x1 << SOTP_OTP_STATUS_1_ECC_COR_SHIFT)

#define SOTP_OTP_STATUS_1_ECC_DET_SHIFT         17
#define SOTP_OTP_STATUS_1_ECC_DET               (0x1 << SOTP_OTP_STATUS_1_ECC_DET_SHIFT)

/* --- */

#define SOTP_OTP_RDATA_0                        0x20

/* --- */

#define SOTP_OTP_RDATA_1                        0x24
#define SOTP_OTP_RDATA_1_FAIL_SHIFT             7
#define SOTP_OTP_RDATA_1_FAIL_MASK              (0x3 << SOTP_OTP_RDATA_1_FAIL_SHIFT)

/* --- */

#define SOTP_OTP_REGION_RD_LOCK                 0x3c

/* --- */

#define SOTP_CHIP_CTRL                          0x4c

#define SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES_SHIFT   4
#define SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES (0x1 << SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES_SHIFT)

#define SOTP_CHIP_CTRL_SW_MANU_PROG_SHIFT       5
#define SOTP_CHIP_CTRL_SW_MANU_PROG             (0x1 << SOTP_CHIP_CTRL_SW_MANU_PROG_SHIFT)

#define SOTP_CHIP_CTRL_SW_NON_AB_DEVICE_SHIFT   7
#define SOTP_CHIP_CTRL_SW_NON_AB_DEVICE         (0x1 << SOTP_CHIP_CTRL_SW_NON_AB_DEVICE_SHIFT)

/* --- */

#define SOTP_PERM                               0x70
#define SOTP_PERM_ALLOW_SECURE_ACCESS           0xCC
#define SOTP_PERM_ALLOW_NONSEC_ACCESS           0x33

#define SOTP_SOTP_OUT_0                         0x74
#define SOTP_SOTP_OUT_0_SOTP_OTP_READY_SHIFT    26
#define SOTP_SOTP_OUT_0_SOTP_OTP_READY          (1 << SOTP_SOTP_OUT_0_SOTP_OTP_READY_SHIFT)

/*
#####################################################################
# PKA Registers
#####################################################################
*/
#define PKA_PERM                                0x14

/*
#####################################################################
# PMC registers
#####################################################################
*/

#define PMC_CONTROL_HOST_MBOX_IN               0x1028
#define PMC_CONTROL_HOST_MBOX_OUT              0x102c

#define PMC_DQM_BASE                           (PMC_BASE+0x1800)
#define PMC_DQM_NOT_EMPTY_IRQ_STS              0x18
#define PMC_DQM_QUEUE_RST                      0x1c
#define PMC_DQM_NOT_EMPTY_STS                  0x20

#define PMC_DQM_QUEUE_CNTRL_BASE               (PMC_BASE+0x1a00)
#define PMC_DQM_QUEUE_DATA_BASE                (PMC_BASE+0x1c00)

#define PMC_DQM_QUEUE_DATA_HOST_TO_PMC         0x0
#define PMC_DQM_QUEUE_DATA_PMC_TO_HOST         0x10


/*
#####################################################################
# PMB registers
#####################################################################
*/
#define PMBM0_BASE                            (PROC_MON_BASE+0xc0)
#define PMBM1_BASE                            (PROC_MON_BASE+0xe0)

#define PMB_CNTRL                             0x0
#define PMB_CNTRL_START_SHIFT                 31
#define PMB_CNTRL_START_MASK                  (1<<PMB_CNTRL_START_SHIFT)
#define PMB_CNTRL_TIMEOUT_ERR_SHIFT           30
#define PMB_CNTRL_TIMEOUT_ERR_MASK            (1<<PMB_CNTRL_TIMEOUT_ERR_SHIFT)
#define PMB_CNTRL_SLAVE_ERR_SHIFT             29
#define PMB_CNTRL_SLAVE_ERR_MASK              (1<<PMB_CNTRL_SLAVE_ERR_SHIFT)
#define PMB_CNTRL_BUSY_SHIFT                  28
#define PMB_CNTRL_BUSY_MASK                   (1<<PMB_CNTRL_BUSY_SHIFT)
#define PMB_CNTRL_CMD_SHIFT                   20
#define PMB_CNTRL_CMD_MASK                    (0xf<<PMB_CNTRL_CMD_SHIFT)
#define PMB_CNTRL_ADDR_SHIFT                  0
#define PMB_CNTRL_ADDR_MASK                   (0xfffff<<PMB_CNTRL_ADDR_SHIFT)

#define PMB_WR_DATA                           0x4
#define PMB_TIMEOUT                           0x8
#define PMB_RD_DATA                           0xc

/*
#####################################################################
# Memory Control Registers
#####################################################################
*/
#define MEMC_GLB_VERS                              0x00000000 /* MC Global Version Register */
#define MEMC_GLB_GCFG                              0x00000004 /* MC Global Configuration Register */
#define MEMC_GLB_GCFG_GCFG_DRAM_EN                 (1<<31)
#define MEMC_GLB_GCFG_MEM_INIT_DONE                (1<<8)
#define MEMC_GLB_GCFG_DRAM_SIZE2_SHIFT	           4
#define MEMC_GLB_GCFG_DRAM_SIZE2_MASK	           0xf
#define MEMC_GLB_GCFG_DRAM_SIZE1_SHIFT	           0
#define MEMC_GLB_GCFG_DRAM_SIZE1_MASK	           0xf
#define MEMC_GLB_CFG                               0x00000010 /* "Memc Arbitor Configurations" */
#define MEMC_GLB_QUE_DIS                           0x00000014 /* "Queue Disable" */
#define MEMC_GLB_SP_SEL                            0x00000018 /* "Queue select for Strict Priority / Round Robin Arbiter" */
#define MEMC_GLB_SP_PRI_0                          0x0000001c /* "Priority for Strict Priority Arbiter" */
#define MEMC_GLB_SP_PRI_1                          0x00000020 /* "Priority for Strict Priority Arbiter" */
#define MEMC_GLB_SP_PRI_2                          0x00000024 /* "Priority for Strict Priority Arbiter" */
#define MEMC_GLB_SP_PRI_3                          0x00000028 /* "Priority for Strict Priority Arbiter" */
#define MEMC_GLB_SP_PRI_4                          0x0000002c /* "Priority for Strict Priority Arbiter" */
#define MEMC_GLB_SP_PRI_5                          0x00000030 /* "Priority for Strict Priority Arbiter" */
#define MEMC_GLB_RR_QUANTUM0                       0x00000040 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM1                       0x00000044 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM2                       0x00000048 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM3                       0x0000004c /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM4                       0x00000050 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM5                       0x00000054 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM6                       0x00000058 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM7                       0x0000005c /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM8                       0x00000060 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM9                       0x00000064 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM10                      0x00000068 /* "Queue Quantum / Weight for Round Robin Arbiter" */
#define MEMC_GLB_RR_QUANTUM11                      0x0000006c /* "Queue Quantum / Weight for Round Robin Arbiter" */

/***************************************************************************
 *MEMC_INTR2 - "MC Level 2 Interrupt Controller"
 ***************************************************************************/
#define MEMC_INTR2_CPU_STATUS                      0x00000080 /* CPU interrupt Status Register */
#define MEMC_INTR2_CPU_SET                         0x00000084 /* CPU interrupt Set Register */
#define MEMC_INTR2_CPU_CLEAR                       0x00000088 /* CPU interrupt Clear Register */
#define MEMC_INTR2_CPU_MASK_STATUS                 0x0000008c /* CPU interrupt Mask Status Register */
#define MEMC_INTR2_CPU_MASK_SET                    0x00000090 /* CPU interrupt Mask Set Register */
#define MEMC_INTR2_CPU_MASK_CLEAR                  0x00000094 /* CPU interrupt Mask Clear Register */

/***************************************************************************
 *MEMC_SRAM_REMAP - "MC non-secure address remapping to internal SRAM"
 ***************************************************************************/
#define MEMC_SRAM_REMAP_CONTROL                    0x000000c0 /* SRAM Remap Control */
#define MEMC_SRAM_REMAP_INIT                       0x000000c4 /* SRAM Remap Initialization */
#define MEMC_SRAM_REMAP_LOG_INFO_0                 0x000000c8 /* SRAM Remap Log Info 0 */
#define MEMC_SRAM_REMAP_LOG_INFO_1                 0x000000cc /* SRAM Remap Log Info 1 */

/***************************************************************************
 *MEMC_CHN_CFG - "MC Channel 0 Config Registers"
 ***************************************************************************/
#define MEMC_CHN_CFG_CNFG                          0x00000100 /* CS Interleaving Configuration Register */
#define MEMC_CHN_CFG_CSST                          0x00000104 /* Chip Select Start Address Register */
#define MEMC_CHN_CFG_CSEND                         0x00000108 /* Chip Select End Address Register */
#define MEMC_CHN_CFG_ROW00_0                       0x00000110 /* CS0 Row Address Bit Select Register0_0 */
#define MEMC_CHN_CFG_ROW00_1                       0x00000114 /* CS0 Row Address Bit Select Register0_1 */
#define MEMC_CHN_CFG_ROW01_0                       0x00000118 /* CS0 Row Address Bit Select Register1_0 */
#define MEMC_CHN_CFG_ROW01_1                       0x0000011c /* CS0 Row Address Bit Select Register1_1 */
#define MEMC_CHN_CFG_ROW20_0                       0x00000130 /* CS1 Row Address Bit Select Register0_0 */
#define MEMC_CHN_CFG_ROW20_1                       0x00000134 /* CS1 Row Address Bit Select Register0_1 */
#define MEMC_CHN_CFG_ROW21_0                       0x00000138 /* CS1 Row Address Bit Select Register1_0 */
#define MEMC_CHN_CFG_ROW21_1                       0x0000013c /* CS1 Row Address Bit Select Register1_1 */
#define MEMC_CHN_CFG_COL00_0                       0x00000150 /* CS0 Column Address Bit Select Register0_0 */
#define MEMC_CHN_CFG_COL00_1                       0x00000154 /* CS0 Column Address Bit Select Register0_1 */
#define MEMC_CHN_CFG_COL01_0                       0x00000158 /* CS0 Column Address Bit Select Register1_0 */
#define MEMC_CHN_CFG_COL01_1                       0x0000015c /* CS0 Column Address Bit Select Register1_1 */
#define MEMC_CHN_CFG_COL20_0                       0x00000170 /* CS1 Column Address Bit Select Register0_0 */
#define MEMC_CHN_CFG_COL20_1                       0x00000174 /* CS1 Column Address Bit Select Register0_1 */
#define MEMC_CHN_CFG_COL21_0                       0x00000178 /* CS1 Column Address Bit Select Register1_0 */
#define MEMC_CHN_CFG_COL21_1                       0x0000017c /* CS1 Column Address Bit Select Register1_1 */
#define MEMC_CHN_CFG_BNK10                         0x00000190 /* CS0 Bank Address Bit Select Register */
#define MEMC_CHN_CFG_BNK32                         0x00000194 /* CS1 Bank Address Bit Select Register */

/***************************************************************************
 *MEMC_CHN_TIM - "MC Channel 0 Config Timing"
 ***************************************************************************/
#define MEMC_CHN_TIM_DCMD                          0x00000200 /* MC DRAM Command Register */
#define MEMC_CHN_TIM_DMODE_0                       0x00000204 /* MC DRAM Mode0,1 Register */
#define MEMC_CHN_TIM_DMODE_2                       0x00000208 /* MC DRAM Mode2,3 Register */
#define MEMC_CHN_TIM_CLKS                          0x0000020c /* DRAM Refresh Control Register */
#define MEMC_CHN_TIM_ODT                           0x00000210 /* DRAM ODT Configuration Register */
#define MEMC_CHN_TIM_TIM1_0                        0x00000214 /* DRAM Timing Register1_0 */
#define MEMC_CHN_TIM_TIM1_1                        0x00000218 /* DRAM Timing Register1_1 */
#define MEMC_CHN_TIM_TIM2                          0x0000021c /* DRAM Timing Register2 */
#define MEMC_CHN_TIM_CTL_CRC                       0x00000220 /* MC CRC Logic Configuration Register */
#define MEMC_CHN_TIM_DOUT_CRC                      0x00000224 /* MC Output CRC Signature Register */
#define MEMC_CHN_TIM_DIN_CRC                       0x00000228 /* MC Input CRC Signature Register */
#define MEMC_CHN_TIM_CRC_CTRL                      0x0000022c /* MC CRC Logic Configuration Register */
#define MEMC_CHN_TIM_PHY_ST                        0x00000230 /* PHY Status Register */
#define MEMC_CHN_TIM_PHY_ST_PHY_PWR_UP             0x1
#define MEMC_CHN_TIM_PHY_ST_HW_RESET               (0x1<<1)
#define MEMC_CHN_TIM_PHY_ST_SW_RESET               (0x1<<2)
#define MEMC_CHN_TIM_PHY_ST_PHY_READY              (0x1<<4)
#define MEMC_CHN_TIM_DRAM_CFG                      0x00000234 /* MC DRAM Config Register */
#define MEMC_CHN_TIM_STAT                          0x00000238 /* MC Status/Error Register */

/***************************************************************************
 *MEMC_UBUSIF_0 - "UBUS Interface 0 Registers"
 ***************************************************************************/
#define MEMC_UBUSIF_0_CFG                          0x00000300 /* "UBUS Interface Configurations" */
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_0             0x00000304 /* "SRC ID/Queue Mapping for SRC ID 7:0" */
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_1             0x00000308 /* "SRC ID/Queue Mapping for SRC ID 15:8" */
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_2             0x0000030c /* "SRC ID/Queue Mapping for SRC ID 23:16" */
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_3             0x00000310 /* "SRC ID/Queue Mapping for SRC ID 31:24" */
#define MEMC_UBUSIF_0_REP_ARB_MODE                 0x00000314 /* "Reply Arbiter Mode" */
#define MEMC_UBUSIF_0_SCRATCH                      0x00000318 /* "Scratch Register" */
#define MEMC_UBUSIF_0_DEBUG_RO                     0x0000031c /* "Debug / Status Register" */

/***************************************************************************
 *MEMC_UBUSIF_1 - "UBUS Interface 1 Registers"
 ***************************************************************************/
#define MEMC_UBUSIF_1_CFG                          0x00000340 /* "UBUS Interface Configurations" */
#define MEMC_UBUSIF_1_SRC_QUEUE_CTRL_0             0x00000344 /* "SRC ID/Queue Mapping for SRC ID 7:0" */
#define MEMC_UBUSIF_1_SRC_QUEUE_CTRL_1             0x00000348 /* "SRC ID/Queue Mapping for SRC ID 15:8" */
#define MEMC_UBUSIF_1_SRC_QUEUE_CTRL_2             0x0000034c /* "SRC ID/Queue Mapping for SRC ID 23:16" */
#define MEMC_UBUSIF_1_SRC_QUEUE_CTRL_3             0x00000350 /* "SRC ID/Queue Mapping for SRC ID 31:24" */
#define MEMC_UBUSIF_1_REP_ARB_MODE                 0x00000354 /* "Reply Arbiter Mode" */
#define MEMC_UBUSIF_1_SCRATCH                      0x00000358 /* "Scratch Register" */
#define MEMC_UBUSIF_1_DEBUG_RO                     0x0000035c /* "Debug / Status Register" */

/***************************************************************************
 *MEMC_AXIRIF_0 - "AXI Read Interface Registers"
 ***************************************************************************/
#define MEMC_AXIRIF_0_CFG                          0x00000380 /* "AXI Interface Configurations" */
#define MEMC_AXIRIF_0_REP_ARB_MODE                 0x00000384 /* "Reply Arbiter Mode" */
#define MEMC_AXIRIF_0_SCRATCH                      0x00000388 /* "Scratch Register" */

/***************************************************************************
 *MEMC_AXIWIF_0 - "AXI Write Interface Registers"
 ***************************************************************************/
#define MEMC_AXIWIF_0_CFG                          0x00000400 /* "AXIW Interface Configurations" */
#define MEMC_AXIWIF_0_REP_ARB_MODE                 0x00000404 /* "Reply Arbiter Mode" */
#define MEMC_AXIWIF_0_SCRATCH                      0x00000408 /* "Scratch Register" */

/***************************************************************************
 *MEMC_EDIS_0 - "EDIS engine 0"
 ***************************************************************************/
#define MEMC_EDIS_0_REV_ID                         0x00000500 /* Enhanced Device Interface Stress (EDIS) revision ID */
#define MEMC_EDIS_0_CTRL_TRIG                      0x00000504 /* Control Triggers for actions */
#define MEMC_EDIS_0_CTRL_MODE                      0x00000508 /* Control Modes for accesses */
#define MEMC_EDIS_0_CTRL_SIZE                      0x0000050c /* Control Sizes for accesses */
#define MEMC_EDIS_0_CTRL_ADDR_START                0x00000510 /* Control Starting Address for accesses */
#define MEMC_EDIS_0_CTRL_ADDR_START_EXT            0x00000514 /* Control Starting Address Extension for accesses */
#define MEMC_EDIS_0_CTRL_ADDR_END                  0x00000518 /* Control Ending Address for accesses */
#define MEMC_EDIS_0_CTRL_ADDR_END_EXT              0x0000051c /* Control Ending Address Extension for accesses */
#define MEMC_EDIS_0_CTRL_WRITE_MASKS               0x00000520 /* Control Byte Write Masks */
#define MEMC_EDIS_0_STAT_MAIN                      0x00000540 /* Main Status of EDIS operation */
#define MEMC_EDIS_0_STAT_WORDS_WRITTEN             0x00000544 /* Status Count of 32-byte JWords Written */
#define MEMC_EDIS_0_STAT_WORDS_READ                0x00000548 /* Status Count of 32-byte JWords Read */
#define MEMC_EDIS_0_STAT_ERROR_COUNT               0x0000054c /* Total Error Count on interface */
#define MEMC_EDIS_0_STAT_ERROR_BITS                0x00000550 /* Per Bit Lane Error Flags */
#define MEMC_EDIS_0_STAT_ADDR_LAST                 0x00000554 /* Last Address Accessed */
#define MEMC_EDIS_0_STAT_ADDR_LAST_EXT             0x00000558 /* Last Address Accessed Extension */
#define MEMC_EDIS_0_STAT_DEBUG                     0x0000057c /* Debug State */
#define MEMC_EDIS_0_STAT_DATA_PORT_0               0x00000580 /* Data Port Word 0 */
#define MEMC_EDIS_0_STAT_DATA_PORT_1               0x00000584 /* Data Port Word 1 */
#define MEMC_EDIS_0_STAT_DATA_PORT_2               0x00000588 /* Data Port Word 2 */
#define MEMC_EDIS_0_STAT_DATA_PORT_3               0x0000058c /* Data Port Word 3 */
#define MEMC_EDIS_0_STAT_DATA_PORT_4               0x00000590 /* Data Port Word 4 */
#define MEMC_EDIS_0_STAT_DATA_PORT_5               0x00000594 /* Data Port Word 5 */
#define MEMC_EDIS_0_STAT_DATA_PORT_6               0x00000598 /* Data Port Word 6 */
#define MEMC_EDIS_0_STAT_DATA_PORT_7               0x0000059c /* Data Port Word 7 */
#define MEMC_EDIS_0_GEN_LFSR_STATE_0               0x000005a0 /* Generator LFSR 0 State */
#define MEMC_EDIS_0_GEN_LFSR_STATE_1               0x000005a4 /* Generator LFSR 1 State */
#define MEMC_EDIS_0_GEN_LFSR_STATE_2               0x000005a8 /* Generator LFSR 2 State */
#define MEMC_EDIS_0_GEN_LFSR_STATE_3               0x000005ac /* Generator LFSR 3 State */
#define MEMC_EDIS_0_GEN_CLOCK                      0x000005b0 /* Generator Clock 0 and 1 State */
#define MEMC_EDIS_0_GEN_PATTERN                    0x000005b4 /* Generator Patterns 0 thru 3 State */
#define MEMC_EDIS_0_BYTELANE_0_CTRL_LO             0x000005c0 /* Byte Lane 0 Control Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_0_CTRL_HI             0x000005c4 /* Byte Lane 0 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_0_BYTELANE_1_CTRL_LO             0x000005c8 /* Byte Lane 0 Control High Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_1_CTRL_HI             0x000005cc /* Byte Lane 1 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_0_BYTELANE_2_CTRL_LO             0x000005d0 /* Byte Lane 2 Control Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_2_CTRL_HI             0x000005d4 /* Byte Lane 2 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_0_BYTELANE_3_CTRL_LO             0x000005d8 /* Byte Lane 3 Control Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_3_CTRL_HI             0x000005dc /* Byte Lane 3 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_0_BYTELANE_0_STAT_LO             0x000005e0 /* Byte Lane 0 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_0_STAT_HI             0x000005e4 /* Byte Lane 0 Status High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_0_BYTELANE_1_STAT_LO             0x000005e8 /* Byte Lane 1 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_1_STAT_HI             0x000005ec /* Byte Lane 1 Status High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_0_BYTELANE_2_STAT_LO             0x000005f0 /* Byte Lane 2 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_2_STAT_HI             0x000005f4 /* Byte Lane 2 Status High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_0_BYTELANE_3_STAT_LO             0x000005f8 /* Byte Lane 3 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_0_BYTELANE_3_STAT_HI             0x000005fc /* Byte Lane 3 Status High Bits, for Bit Lanes 4 thru 7 */

/***************************************************************************
 *MEMC_EDIS_1 - "EDIS engine 1"
 ***************************************************************************/
#define MEMC_EDIS_1_REV_ID                         0x00000600 /* Enhanced Device Interface Stress (EDIS) revision ID */
#define MEMC_EDIS_1_CTRL_TRIG                      0x00000604 /* Control Triggers for actions */
#define MEMC_EDIS_1_CTRL_MODE                      0x00000608 /* Control Modes for accesses */
#define MEMC_EDIS_1_CTRL_SIZE                      0x0000060c /* Control Sizes for accesses */
#define MEMC_EDIS_1_CTRL_ADDR_START                0x00000610 /* Control Starting Address for accesses */
#define MEMC_EDIS_1_CTRL_ADDR_START_EXT            0x00000614 /* Control Starting Address Extension for accesses */
#define MEMC_EDIS_1_CTRL_ADDR_END                  0x00000618 /* Control Ending Address for accesses */
#define MEMC_EDIS_1_CTRL_ADDR_END_EXT              0x0000061c /* Control Ending Address Extension for accesses */
#define MEMC_EDIS_1_CTRL_WRITE_MASKS               0x00000620 /* Control Byte Write Masks */
#define MEMC_EDIS_1_STAT_MAIN                      0x00000640 /* Main Status of EDIS operation */
#define MEMC_EDIS_1_STAT_WORDS_WRITTEN             0x00000644 /* Status Count of 32-byte JWords Written */
#define MEMC_EDIS_1_STAT_WORDS_READ                0x00000648 /* Status Count of 32-byte JWords Read */
#define MEMC_EDIS_1_STAT_ERROR_COUNT               0x0000064c /* Total Error Count on interface */
#define MEMC_EDIS_1_STAT_ERROR_BITS                0x00000650 /* Per Bit Lane Error Flags */
#define MEMC_EDIS_1_STAT_ADDR_LAST                 0x00000654 /* Last Address Accessed */
#define MEMC_EDIS_1_STAT_ADDR_LAST_EXT             0x00000658 /* Last Address Accessed Extension */
#define MEMC_EDIS_1_STAT_DEBUG                     0x0000067c /* Debug State */
#define MEMC_EDIS_1_STAT_DATA_PORT_0               0x00000680 /* Data Port Word 0 */
#define MEMC_EDIS_1_STAT_DATA_PORT_1               0x00000684 /* Data Port Word 1 */
#define MEMC_EDIS_1_STAT_DATA_PORT_2               0x00000688 /* Data Port Word 2 */
#define MEMC_EDIS_1_STAT_DATA_PORT_3               0x0000068c /* Data Port Word 3 */
#define MEMC_EDIS_1_STAT_DATA_PORT_4               0x00000690 /* Data Port Word 4 */
#define MEMC_EDIS_1_STAT_DATA_PORT_5               0x00000694 /* Data Port Word 5 */
#define MEMC_EDIS_1_STAT_DATA_PORT_6               0x00000698 /* Data Port Word 6 */
#define MEMC_EDIS_1_STAT_DATA_PORT_7               0x0000069c /* Data Port Word 7 */
#define MEMC_EDIS_1_GEN_LFSR_STATE_0               0x000006a0 /* Generator LFSR 0 State */
#define MEMC_EDIS_1_GEN_LFSR_STATE_1               0x000006a4 /* Generator LFSR 1 State */
#define MEMC_EDIS_1_GEN_LFSR_STATE_2               0x000006a8 /* Generator LFSR 2 State */
#define MEMC_EDIS_1_GEN_LFSR_STATE_3               0x000006ac /* Generator LFSR 3 State */
#define MEMC_EDIS_1_GEN_CLOCK                      0x000006b0 /* Generator Clock 0 and 1 State */
#define MEMC_EDIS_1_GEN_PATTERN                    0x000006b4 /* Generator Patterns 0 thru 3 State */
#define MEMC_EDIS_1_BYTELANE_0_CTRL_LO             0x000006c0 /* Byte Lane 0 Control Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_0_CTRL_HI             0x000006c4 /* Byte Lane 0 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_1_BYTELANE_1_CTRL_LO             0x000006c8 /* Byte Lane 0 Control High Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_1_CTRL_HI             0x000006cc /* Byte Lane 1 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_1_BYTELANE_2_CTRL_LO             0x000006d0 /* Byte Lane 2 Control Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_2_CTRL_HI             0x000006d4 /* Byte Lane 2 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_1_BYTELANE_3_CTRL_LO             0x000006d8 /* Byte Lane 3 Control Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_3_CTRL_HI             0x000006dc /* Byte Lane 3 Control High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_1_BYTELANE_0_STAT_LO             0x000006e0 /* Byte Lane 0 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_0_STAT_HI             0x000006e4 /* Byte Lane 0 Status High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_1_BYTELANE_1_STAT_LO             0x000006e8 /* Byte Lane 1 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_1_STAT_HI             0x000006ec /* Byte Lane 1 Status High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_1_BYTELANE_2_STAT_LO             0x000006f0 /* Byte Lane 2 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_2_STAT_HI             0x000006f4 /* Byte Lane 2 Status High Bits, for Bit Lanes 4 thru 7 */
#define MEMC_EDIS_1_BYTELANE_3_STAT_LO             0x000006f8 /* Byte Lane 3 Status Low Bits, for Bit Lanes 0 thru 3 */
#define MEMC_EDIS_1_BYTELANE_3_STAT_HI             0x000006fc /* Byte Lane 3 Status High Bits, for Bit Lanes 4 thru 7 */

/***************************************************************************
 *MEMC_STATS - "MC Statistics Registers"
 ***************************************************************************/
#define MEMC_STATS_CTRL                            0x00000700 /* "Statitiscs Control" */
#define MEMC_STATS_TIMER_CFG                       0x00000704 /* "Statistics Timer Cfg" */
#define MEMC_STATS_TIMER_COUNT                     0x00000708 /* "Statistics Timer snapped Count" */
#define MEMC_STATS_TOTAL_SLICE                     0x0000070c /* "Total number of slices processed" */
#define MEMC_STATS_TOTAL_PACKET                    0x00000710 /* "Total number of packet processed" */
#define MEMC_STATS_SLICE_REORDER                   0x00000714 /* "Total number of slices reordered by channel arb" */
#define MEMC_STATS_IDLE_DDR_CYCLE                  0x00000718 /* "Total number of cycles where these is no data on the DDR_DQ bus" */
#define MEMC_STATS_ARB_GRANT                       0x0000071c /* "Total number of mc_arb grants to programmable virtual queue" */
#define MEMC_STATS_PROG_0                          0x00000720 /* "Programable Counter 0" */
#define MEMC_STATS_PROG_1                          0x00000724 /* "Programable Counter 1" */
#define MEMC_STATS_ARB_GRANT_MATCH                 0x00000728 /* "Arb Grant Counter Filter" */
#define MEMC_STATS_CFG_0                           0x0000072c /* "Programmable Counter 0 Configuration" */
#define MEMC_STATS_CFG_1                           0x00000730 /* "Programmable Counter 1 Configuration" */

/***************************************************************************
 *MEMC_CAP - "MC Diagnostics Capture Registers"
 ***************************************************************************/
#define MEMC_CAP_CAPTURE_CFG                       0x00000780 /* "Diagnostic Capture Config" */
#define MEMC_CAP_TRIGGER_ADDR                      0x00000784 /* "Next Buffer Write address at Trigger" */
#define MEMC_CAP_READ_CTRL                         0x00000788 /* "Buffer Read Control" */
#define MEMC_CAP_CAPTURE_MATCH_0                   0x00000790 /* "Diagnostic Capture Match Value 0" */
#define MEMC_CAP_CAPTURE_MATCH_1                   0x00000794 /* "Diagnostic Capture Match Value 1" */
#define MEMC_CAP_CAPTURE_MATCH_2                   0x00000798 /* "Diagnostic Capture Match Value 2" */
#define MEMC_CAP_CAPTURE_MASK_0                    0x000007a0 /* "Diagnostic Capture Match Mask 0" */
#define MEMC_CAP_CAPTURE_MASK_1                    0x000007a4 /* "Diagnostic Capture Match Mask 1" */
#define MEMC_CAP_CAPTURE_MASK_2                    0x000007a8 /* "Diagnostic Capture Match Mask 2" */
#define MEMC_CAP_TRIGGER_MATCH_0                   0x000007b0 /* "Diagnostic Capture Match Value 0" */
#define MEMC_CAP_TRIGGER_MATCH_1                   0x000007b4 /* "Diagnostic Capture Match Value 1" */
#define MEMC_CAP_TRIGGER_MATCH_2                   0x000007b8 /* "Diagnostic Capture Match Value 2" */
#define MEMC_CAP_TRIGGER_MASK_0                    0x000007c0 /* "Diagnostic Capture Match Mask 0" */
#define MEMC_CAP_TRIGGER_MASK_1                    0x000007c4 /* "Diagnostic Capture Match Mask 1" */
#define MEMC_CAP_TRIGGER_MASK_2                    0x000007c8 /* "Diagnostic Capture Match Mask 2" */
#define MEMC_CAP_READ_DATA_0                       0x000007d0 /* "Buffer Read Data 31:0" */
#define MEMC_CAP_READ_DATA_1                       0x000007d4 /* "Buffer Read Data 63:32" */
#define MEMC_CAP_READ_DATA_2                       0x000007d8 /* "Buffer Read Data 95:64" */
#define MEMC_CAP_READ_DATA_3                       0x000007dc /* "Buffer Read Data 127:96" */

/***************************************************************************
 *MEMC_SEC_RANGE_CHK - "Secure Range Checkers"
 ***************************************************************************/
#define MEMC_SEC_RANGE_CHK_LOCK                    0x00000800 /* Range region lock control */
#define MEMC_SEC_RANGE_CHK_LOG_INFO_0              0x00000804 /* Range Log Info 0 */
#define MEMC_SEC_RANGE_CHK_LOG_INFO_1              0x00000808 /* Range Log Info 1 */
#define MEMC_SEC_RANGE_CHK_CONTROL_0               0x0000080c /* Range control 0 (lowest priority) */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_0            0x00000810 /* Range UBUS0 Port (lowest priority) */
#define MEMC_SEC_RANGE_CHK_BASE_0                  0x00000814 /* Range base 0 (lowest priority) */
#define MEMC_SEC_RANGE_CHK_CONTROL_1               0x00000818 /* Range control 1 */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_1            0x0000081c /* Range UBUS0 Port */
#define MEMC_SEC_RANGE_CHK_BASE_1                  0x00000820 /* Range base 1 */
#define MEMC_SEC_RANGE_CHK_CONTROL_2               0x00000824 /* Range control 2 */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_2            0x00000828 /* Range UBUS0 Port */
#define MEMC_SEC_RANGE_CHK_BASE_2                  0x0000082c /* Range base 2 */
#define MEMC_SEC_RANGE_CHK_CONTROL_3               0x00000830 /* Range control 3 */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_3            0x00000834 /* Range UBUS0 Port */
#define MEMC_SEC_RANGE_CHK_BASE_3                  0x00000838 /* Range base 3 */
#define MEMC_SEC_RANGE_CHK_CONTROL_4               0x0000083c /* Range control 4 */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_4            0x00000840 /* Range UBUS0 Port */
#define MEMC_SEC_RANGE_CHK_BASE_4                  0x00000844 /* Range base 4 */
#define MEMC_SEC_RANGE_CHK_CONTROL_5               0x00000848 /* Range control 5 */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_5            0x0000084c /* Range UBUS0 Port */
#define MEMC_SEC_RANGE_CHK_BASE_5                  0x00000850 /* Range base 5 */
#define MEMC_SEC_RANGE_CHK_CONTROL_6               0x00000854 /* Range control 6 */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_6            0x00000858 /* Range UBUS0 Port */
#define MEMC_SEC_RANGE_CHK_BASE_6                  0x0000085c /* Range base 6 */
#define MEMC_SEC_RANGE_CHK_CONTROL_7               0x00000860 /* Range control 7 (highest priority) */
#define MEMC_SEC_RANGE_CHK_UBUS0_PORT_7            0x00000864 /* Range UBUS0 Port (highest priority) */
#define MEMC_SEC_RANGE_CHK_BASE_7                  0x00000868 /* Range base 7 (highest priority) */

/***************************************************************************
 *MEMC_SEC_INTR2 - "MC Secure Level 2 Interrupt Controller"
 ***************************************************************************/
#define MEMC_SEC_INTR2_CPU_STATUS                  0x00000900 /* CPU interrupt Status Register */
#define MEMC_SEC_INTR2_CPU_SET                     0x00000904 /* CPU interrupt Set Register */
#define MEMC_SEC_INTR2_CPU_CLEAR                   0x00000908 /* CPU interrupt Clear Register */
#define MEMC_SEC_INTR2_CPU_MASK_STATUS             0x0000090c /* CPU interrupt Mask Status Register */
#define MEMC_SEC_INTR2_CPU_MASK_SET                0x00000910 /* CPU interrupt Mask Set Register */
#define MEMC_SEC_INTR2_CPU_MASK_CLEAR              0x00000914 /* CPU interrupt Mask Clear Register */

/***************************************************************************
 *MEMC_SEC_SRAM_REMAP - "MC secure address remapping to internal SRAM"
 ***************************************************************************/
#define MEMC_SEC_SRAM_REMAP_CONTROL                0x00000940 /* SRAM Remap Control */
#define MEMC_SEC_SRAM_REMAP_INIT                   0x00000944 /* SRAM Remap Initialization */
#define MEMC_SEC_SRAM_REMAP_LOG_INFO_0             0x00000948 /* SRAM Remap Log Info 0 */
#define MEMC_SEC_SRAM_REMAP_LOG_INFO_1             0x0000094c /* SRAM Remap Log Info 1 */

/***************************************************************************
 *DDR34_CORE_PHY_CONTROL_REGS - DDR34 Address/Comand control registers
 ***************************************************************************/
#define PHY_CONTROL_REGS_REVISION                0x00001000 /* Address & Control revision register */
#define PHY_CONTROL_REGS_PLL_STATUS              0x00001004 /* PHY PLL status register */
#define PHY_CONTROL_REGS_PLL_STATUS_LOCK         0x1
#define PHY_CONTROL_REGS_PLL_CONFIG              0x00001008 /* PHY PLL configuration register */
#define PHY_CONTROL_REGS_PLL_CONTROL1            0x0000100c /* PHY PLL control register */
#define PHY_CONTROL_REGS_PLL_CONTROL2            0x00001010 /* PHY PLL control register */
#define PHY_CONTROL_REGS_PLL_CONTROL3            0x00001014 /* PHY PLL control register */
#define PHY_CONTROL_REGS_PLL_DIVIDERS            0x00001018 /* PHY PLL integer divider register */
#define PHY_CONTROL_REGS_PLL_FRAC_DIVIDER        0x0000101c /* PHY PLL fractional divider register */
#define PHY_CONTROL_REGS_PLL_SS_CONTROL          0x00001020 /* PHY PLL spread spectrum control register */
#define PHY_CONTROL_REGS_PLL_SS_LIMIT            0x00001024 /* PHY PLL spread spectrum limit register */
#define PHY_CONTROL_REGS_AUX_CONTROL             0x00001028 /* Aux Control register */
#define PHY_CONTROL_REGS_IDLE_PAD_CONTROL        0x0000102c /* Idle mode pad control register */
#define PHY_CONTROL_REGS_IDLE_PAD_ENABLE0        0x00001030 /* Idle mode pad enable register */
#define PHY_CONTROL_REGS_IDLE_PAD_ENABLE1        0x00001034 /* Idle mode pad enable register */
#define PHY_CONTROL_REGS_DRIVE_PAD_CTL           0x00001038 /* PVT Compensation control register */
#define PHY_CONTROL_REGS_STATIC_PAD_CTL          0x0000103c /* pad rx and tx characteristics control register */
#define PHY_CONTROL_REGS_DRAM_CONFIG             0x00001040 /* DRAM configuration register */
#define PHY_CONTROL_REGS_DRAM_TIMING1            0x00001044 /* DRAM timing register #1 */
#define PHY_CONTROL_REGS_DRAM_TIMING2            0x00001048 /* DRAM timing register #2 */
#define PHY_CONTROL_REGS_DRAM_TIMING3            0x0000104c /* DRAM timing register #3 */
#define PHY_CONTROL_REGS_DRAM_TIMING4            0x00001050 /* DRAM timing register #4 */
#define PHY_CONTROL_REGS_VDL_CALIBRATE           0x00001060 /* PHY VDL calibration control register */
#define PHY_CONTROL_REGS_VDL_CALIB_STATUS1       0x00001064 /* PHY VDL calibration status register #1 */
#define PHY_CONTROL_REGS_VDL_CALIB_STATUS1_IDLE  0x1
#define PHY_CONTROL_REGS_VDL_CALIB_STATUS2       0x00001068 /* PHY VDL calibration status register #2 */
#define PHY_CONTROL_REGS_VDL_MONITOR_CONTROL     0x0000106c /* PHY VDL delay monitoring control register */
#define PHY_CONTROL_REGS_VDL_MONITOR_REF         0x00001070 /* PHY VDL delay monitoring reference register */
#define PHY_CONTROL_REGS_VDL_MONITOR_STATUS      0x00001074 /* PHY VDL delay monitoring status register */
#define PHY_CONTROL_REGS_VDL_MONITOR_OVERRIDE    0x00001078 /* PHY VDL delay monitoring override register */
#define PHY_CONTROL_REGS_VDL_MONITOR_OUT_CONTROL 0x0000107c /* PHY VDL delay monitoring output control register */
#define PHY_CONTROL_REGS_VDL_MONITOR_OUT_STATUS  0x00001080 /* PHY VDL delay monitoring output status register */
#define PHY_CONTROL_REGS_VDL_MONITOR_OUT_STATUS_CLEAR 0x00001084 /* PHY VDL delay monitoring output status clear register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD00        0x00001090 /* DDR interface signal AD[00] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD01        0x00001094 /* DDR interface signal AD[01] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD02        0x00001098 /* DDR interface signal AD[02] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD03        0x0000109c /* DDR interface signal AD[03] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD04        0x000010a0 /* DDR interface signal AD[04] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD05        0x000010a4 /* DDR interface signal AD[05] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD06        0x000010a8 /* DDR interface signal AD[06] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD07        0x000010ac /* DDR interface signal AD[07] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD08        0x000010b0 /* DDR interface signal AD[08] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD09        0x000010b4 /* DDR interface signal AD[09] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD10        0x000010b8 /* DDR interface signal AD[10] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD11        0x000010bc /* DDR interface signal AD[11] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD12        0x000010c0 /* DDR interface signal AD[12] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD13        0x000010c4 /* DDR interface signal AD[13] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD14        0x000010c8 /* DDR interface signal AD[14] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AD15        0x000010cc /* DDR interface signal AD[15] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_BA0         0x000010d0 /* DDR interface signal BA[0] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_BA1         0x000010d4 /* DDR interface signal BA[1] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_BA2         0x000010d8 /* DDR interface signal BA[2] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AUX0        0x000010dc /* DDR interface signal AUX[0] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AUX1        0x000010e0 /* DDR interface signal AUX[1] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_AUX2        0x000010e4 /* DDR interface signal AUX[2] VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_CS0         0x000010e8 /* DDR interface signal CS0 VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_CS1         0x000010ec /* DDR interface signal CS1 VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_PAR         0x000010f0 /* DDR interface signal PAR VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_RAS_N       0x000010f4 /* DDR interface signal RAS_N VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_CAS_N       0x000010f8 /* DDR interface signal CAS_N VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_CKE         0x000010fc /* DDR interface signal CKE0 VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_RST_N       0x00001100 /* DDR interface signal RST_N VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_ODT         0x00001104 /* DDR interface signal ODT0 VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_WE_N        0x00001108 /* DDR interface signal WE_N VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_DDR_CK_P    0x0000110c /* DDR interface signal DDR_CK-P VDL control register */
#define PHY_CONTROL_REGS_VDL_CONTROL_DDR_CK_N    0x00001110 /* DDR interface signal DDR_CK-N VDL control register */
#define PHY_CONTROL_REGS_VDL_CLK_CONTROL         0x00001114 /* DDR interface signal Write Leveling CLK VDL control register */
#define PHY_CONTROL_REGS_VDL_LDE_CONTROL         0x00001118 /* DDR interface signal Write Leveling Capture Enable VDL control register */
#define PHY_CONTROL_REGS_AC_SPARE_REG            0x0000111c /* Address and Control Spare register */
#define PHY_CONTROL_REGS_REFRESH                 0x00001130 /* Refresh engine controller */
#define PHY_CONTROL_REGS_UPDATE_VDL              0x00001134 /* Update VDL control register */
#define PHY_CONTROL_REGS_UPDATE_VDL_SNOOP1       0x00001138 /* Update VDL snoop control register #1 */
#define PHY_CONTROL_REGS_UPDATE_VDL_SNOOP2       0x0000113c /* Update VDL snoop control register #2 */
#define PHY_CONTROL_REGS_COMMAND_REG1            0x00001140 /* DRAM Command Register #1 */
#define PHY_CONTROL_REGS_COMMAND_AUX_REG1        0x00001144 /* DRAM AUX_N Command Register #1 */
#define PHY_CONTROL_REGS_COMMAND_REG2            0x00001148 /* DRAM Command Register #2 */
#define PHY_CONTROL_REGS_COMMAND_AUX_REG2        0x0000114c /* DRAM AUX_N Command Register #2 */
#define PHY_CONTROL_REGS_COMMAND_REG3            0x00001150 /* DRAM Command Register #3 */
#define PHY_CONTROL_REGS_COMMAND_AUX_REG3        0x00001154 /* DRAM AUX_N Command Register #3 */
#define PHY_CONTROL_REGS_COMMAND_REG4            0x00001158 /* DRAM Command Register #4 */
#define PHY_CONTROL_REGS_COMMAND_AUX_REG4        0x0000115c /* DRAM AUX_N Command Register #4 */
#define PHY_CONTROL_REGS_COMMAND_REG_TIMER       0x00001160 /* DRAM Command Timer Register */
#define PHY_CONTROL_REGS_MODE_REG0               0x00001164 /* DDR3/DDR4/GDDR5 Mode Register 0 and LPDDR Mode Register 1 */
#define PHY_CONTROL_REGS_MODE_REG1               0x00001168 /* DDR3/DDR4/GDDR5 Mode Register 1 and LPDDR Mode Register 2 */
#define PHY_CONTROL_REGS_MODE_REG2               0x0000116c /* DDR3/DDR4/GDDR5 Mode Register 2 and LPDDR Mode Register 3 */
#define PHY_CONTROL_REGS_MODE_REG3               0x00001170 /* DDR3/DDR4/GDDR5 Mode Register 3 and LPDDR Mode Register 9 */
#define PHY_CONTROL_REGS_MODE_REG4               0x00001174 /* DDR4/GDDR5 Mode Register 4 and LPDDR Mode Register 10 */
#define PHY_CONTROL_REGS_MODE_REG5               0x00001178 /* DDR4/GDDR5 Mode Register 5 and LPDDR Mode Register 16 */
#define PHY_CONTROL_REGS_MODE_REG6               0x0000117c /* DDR4/GDDR5 Mode Register 6 and LPDDR Mode Register 17 */
#define PHY_CONTROL_REGS_MODE_REG7               0x00001180 /* DDR4/GDDR5 Mode Register 7 and LPDDR Mode Register 41 */
#define PHY_CONTROL_REGS_MODE_REG8               0x00001184 /* GDDR5 Mode Register 8 and LPDDR Mode Register 42 */
#define PHY_CONTROL_REGS_MODE_REG15              0x00001188 /* GDDR5 Mode Register 15 and LPDDR Mode Register 48 */
#define PHY_CONTROL_REGS_MODE_REG63              0x0000118c /* LPDDR Mode Register 63 */
#define PHY_CONTROL_REGS_ALERT_CLEAR             0x00001190 /* DDR4 Alert status clear register */
#define PHY_CONTROL_REGS_ALERT_STATUS            0x00001194 /* DDR4 Alert status register */
#define PHY_CONTROL_REGS_CA_PARITY               0x00001198 /* DDR4 CA parity control register */
#define PHY_CONTROL_REGS_CA_PLAYBACK_CONTROL     0x0000119c /* GDDR5 CA playback control register */
#define PHY_CONTROL_REGS_CA_PLAYBACK_STATUS0     0x000011a0 /* LPDDR3 and GDDR5 CA playback status register0 */
#define PHY_CONTROL_REGS_WRITE_LEVELING_CONTROL  0x000011ac /* Write leveling control register */
#define PHY_CONTROL_REGS_WRITE_LEVELING_STATUS   0x000011b0 /* Write leveling status register */
#define PHY_CONTROL_REGS_READ_ENABLE_CONTROL     0x000011b4 /* Read enable test cycle control register */
#define PHY_CONTROL_REGS_READ_ENABLE_STATUS      0x000011b8 /* Read enable test cycle status register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_LFSR_SEED   0x000011c0 /* Traffic generator seed register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_ADDRESS1    0x000011c4 /* Traffic generator address register #1 */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_ADDRESS2    0x000011c8 /* Traffic generator address register #2 */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_CONTROL     0x000011cc /* Traffic generator control register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_DATA_CONTROL 0x000011d0 /* Traffic generator data control register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_DQ_MASK     0x000011d4 /* Traffic generator DQ mask register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_ECC_DQ_MASK 0x000011d8 /* Traffic generator ECC DQ mask register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_STATUS      0x000011dc /* Traffic generator status register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_DQ_STATUS   0x000011e0 /* Traffic generator DQ status register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_ECC_STATUS  0x000011e4 /* Traffic generator ECC DQ status register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_ERR_CNT_CONTROL 0x000011e8 /* Traffic generator error count control register */
#define PHY_CONTROL_REGS_TRAFFIC_GEN_ERR_CNT_STATUS 0x000011ec /* Traffic generator error count status register */
#define PHY_CONTROL_REGS_VIRTUAL_VTT_CONTROL     0x000011f0 /* Virtual VTT Control and Status register */
#define PHY_CONTROL_REGS_VIRTUAL_VTT_STATUS      0x000011f4 /* Virtual VTT Control and Status register */
#define PHY_CONTROL_REGS_VIRTUAL_VTT_CONNECTIONS 0x000011f8 /* Virtual VTT Connections register */
#define PHY_CONTROL_REGS_VIRTUAL_VTT_OVERRIDE    0x000011fc /* Virtual VTT Override register */
#define PHY_CONTROL_REGS_VREF_DAC_CONTROL        0x00001200 /* VREF DAC Control register */
#define PHY_CONTROL_REGS_PHYBIST_CNTRL           0x00001204 /* PhyBist Control Register */
#define PHY_CONTROL_REGS_PHYBIST_SEED            0x00001208 /* PhyBist Seed Register */
#define PHY_CONTROL_REGS_PHYBIST_CA_MASK         0x0000120c /* PhyBist Command/Address Bus Mask */
#define PHY_CONTROL_REGS_PHYBIST_STATUS          0x00001210 /* PhyBist General Status Register */
#define PHY_CONTROL_REGS_PHYBIST_CTL_STATUS      0x00001214 /* PhyBist Per-Bit Control Pad Status Register */
#define PHY_CONTROL_REGS_PHYBIST_BL0_STATUS      0x00001218 /* PhyBist Byte Lane #0 Status Register */
#define PHY_CONTROL_REGS_PHYBIST_BL1_STATUS      0x0000121c /* PhyBist Byte Lane #1 Status Register */
#define PHY_CONTROL_REGS_STANDBY_CONTROL         0x00001230 /* Standby Control register */
#define PHY_CONTROL_REGS_DEBUG_FREEZE_ENABLE     0x00001234 /* Freeze-on-error enable register */
#define PHY_CONTROL_REGS_DEBUG_MUX_CONTROL       0x00001238 /* Debug Mux Control register */
#define PHY_CONTROL_REGS_DFI_CNTRL               0x0000123c /* DFI Interface Ownership Control register */
#define PHY_CONTROL_REGS_WRITE_ODT_CNTRL         0x00001240 /* Write ODT Control register */
#define PHY_CONTROL_REGS_ABI_PAR_CNTRL           0x00001244 /* ABI and PAR Control register */
#define PHY_CONTROL_REGS_ZQ_CAL                  0x00001248 /* ZQ Calibration Control register */
#define PHY_CONTROL_REGS_RO_PROC_MON_CTL         0x0000124c /* Ring-Osc control register */
#define PHY_CONTROL_REGS_RO_PROC_MON_STATUS      0x00001250 /* Ring-Osc count register */

/***************************************************************************
 *PHY_BYTE_LANE_0 - DDR34 Byte Lane #0 control registers
 ***************************************************************************/
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQS_P     0x00001400 /* Write channel DQS-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQS_N     0x00001404 /* Write channel DQS-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ0       0x00001408 /* Write channel DQ0 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ1       0x0000140c /* Write channel DQ1 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ2       0x00001410 /* Write channel DQ2 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ3       0x00001414 /* Write channel DQ3 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ4       0x00001418 /* Write channel DQ4 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ5       0x0000141c /* Write channel DQ5 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ6       0x00001420 /* Write channel DQ6 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DQ7       0x00001424 /* Write channel DQ7 VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_DM        0x00001428 /* Write channel DM VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_WR_EDC       0x0000142c /* Write channel EDC VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQSP      0x00001430 /* Read channel DQSP VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQSN      0x00001434 /* Read channel DQSP VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ0P      0x00001438 /* Read channel DQ0-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ0N      0x0000143c /* Read channel DQ0-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ1P      0x00001440 /* Read channel DQ1-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ1N      0x00001444 /* Read channel DQ1-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ2P      0x00001448 /* Read channel DQ2-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ2N      0x0000144c /* Read channel DQ2-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ3P      0x00001450 /* Read channel DQ3-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ3N      0x00001454 /* Read channel DQ3-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ4P      0x00001458 /* Read channel DQ4-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ4N      0x0000145c /* Read channel DQ4-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ5P      0x00001460 /* Read channel DQ5-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ5N      0x00001464 /* Read channel DQ5-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ6P      0x00001468 /* Read channel DQ6-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ6N      0x0000146c /* Read channel DQ6-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ7P      0x00001470 /* Read channel DQ7-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DQ7N      0x00001474 /* Read channel DQ7-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DMP       0x00001478 /* Read channel DM-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_DMN       0x0000147c /* Read channel DM-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EDCP      0x00001480 /* Read channel EDC-P VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EDCN      0x00001484 /* Read channel EDC-N VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS0    0x00001488 /* Read channel CS_N[0] read enable VDL control register */
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS1    0x0000148c /* Read channel CS_N[1] read enable VDL control register */
#define PHY_BYTE_LANE_0_VDL_CLK_CONTROL          0x00001490 /* DDR interface signal Write Leveling CLK VDL control register */
#define PHY_BYTE_LANE_0_VDL_LDE_CONTROL          0x00001494 /* DDR interface signal Write Leveling Capture Enable VDL control register */
#define PHY_BYTE_LANE_0_RD_EN_DLY_CYC            0x000014a0 /* Read enable bit-clock cycle delay control register */
#define PHY_BYTE_LANE_0_WR_CHAN_DLY_CYC          0x000014a4 /* Write leveling bit-clock cycle delay control register */
#define PHY_BYTE_LANE_0_READ_CONTROL             0x000014b0 /* Read channel datapath control register */
#define PHY_BYTE_LANE_0_READ_FIFO_ADDR           0x000014b4 /* Read fifo addresss pointer register */
#define PHY_BYTE_LANE_0_READ_FIFO_DATA           0x000014b8 /* Read fifo data register */
#define PHY_BYTE_LANE_0_READ_FIFO_DM_DBI         0x000014bc /* Read fifo dm/dbi register */
#define PHY_BYTE_LANE_0_READ_FIFO_STATUS         0x000014c0 /* Read fifo status register */
#define PHY_BYTE_LANE_0_READ_FIFO_CLEAR          0x000014c4 /* Read fifo status clear register */
#define PHY_BYTE_LANE_0_IDLE_PAD_CONTROL         0x000014c8 /* Idle mode SSTL pad control register */
#define PHY_BYTE_LANE_0_DRIVE_PAD_CTL            0x000014cc /* SSTL pad drive characteristics control register */
#define PHY_BYTE_LANE_0_RD_EN_DRIVE_PAD_CTL      0x000014d0 /* SSTL read enable pad drive characteristics control register */
#define PHY_BYTE_LANE_0_STATIC_PAD_CTL           0x000014d4 /* pad rx and tx characteristics control register */
#define PHY_BYTE_LANE_0_WR_PREAMBLE_MODE         0x000014d8 /* Write cycle preamble control register */
#define PHY_BYTE_LANE_0_ODT_CONTROL              0x000014e0 /* Read channel ODT control register */
#define PHY_BYTE_LANE_0_EDC_DPD_CONTROL          0x000014f0 /* GDDR5M EDC digital phase detector control register */
#define PHY_BYTE_LANE_0_EDC_DPD_STATUS           0x000014f4 /* GDDR5M EDC digital phase detector status register */
#define PHY_BYTE_LANE_0_EDC_DPD_OUT_CONTROL      0x000014f8 /* GDDR5M EDC digital phase detector output signal control register */
#define PHY_BYTE_LANE_0_EDC_DPD_OUT_STATUS       0x000014fc /* GDDR5M EDC digital phase detector output signal status register */
#define PHY_BYTE_LANE_0_EDC_DPD_OUT_STATUS_CLEAR 0x00001500 /* GDDR5M EDC digital phase detector output signal status clear register */
#define PHY_BYTE_LANE_0_EDC_CRC_CONTROL          0x00001504 /* GDDR5M EDC signal path CRC control register */
#define PHY_BYTE_LANE_0_EDC_CRC_STATUS           0x00001508 /* GDDR5M EDC signal path CRC status register */
#define PHY_BYTE_LANE_0_EDC_CRC_COUNT            0x0000150c /* GDDR5M EDC signal path CRC counter register */
#define PHY_BYTE_LANE_0_EDC_CRC_STATUS_CLEAR     0x00001510 /* GDDR5M EDC signal path CRC counter register */
#define PHY_BYTE_LANE_0_BL_SPARE_REG             0x00001514 /* Byte-Lane Spare register */

/***************************************************************************
 *PHY_BYTE_LANE_1 - DDR34 Byte Lane #1 control registers
 ***************************************************************************/
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQS_P     0x00001600 /* Write channel DQS-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQS_N     0x00001604 /* Write channel DQS-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ0       0x00001608 /* Write channel DQ0 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ1       0x0000160c /* Write channel DQ1 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ2       0x00001610 /* Write channel DQ2 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ3       0x00001614 /* Write channel DQ3 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ4       0x00001618 /* Write channel DQ4 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ5       0x0000161c /* Write channel DQ5 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ6       0x00001620 /* Write channel DQ6 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DQ7       0x00001624 /* Write channel DQ7 VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_DM        0x00001628 /* Write channel DM VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_WR_EDC       0x0000162c /* Write channel EDC VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQSP      0x00001630 /* Read channel DQSP VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQSN      0x00001634 /* Read channel DQSP VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ0P      0x00001638 /* Read channel DQ0-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ0N      0x0000163c /* Read channel DQ0-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ1P      0x00001640 /* Read channel DQ1-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ1N      0x00001644 /* Read channel DQ1-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ2P      0x00001648 /* Read channel DQ2-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ2N      0x0000164c /* Read channel DQ2-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ3P      0x00001650 /* Read channel DQ3-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ3N      0x00001654 /* Read channel DQ3-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ4P      0x00001658 /* Read channel DQ4-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ4N      0x0000165c /* Read channel DQ4-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ5P      0x00001660 /* Read channel DQ5-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ5N      0x00001664 /* Read channel DQ5-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ6P      0x00001668 /* Read channel DQ6-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ6N      0x0000166c /* Read channel DQ6-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ7P      0x00001670 /* Read channel DQ7-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DQ7N      0x00001674 /* Read channel DQ7-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DMP       0x00001678 /* Read channel DM-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_DMN       0x0000167c /* Read channel DM-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EDCP      0x00001680 /* Read channel EDC-P VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EDCN      0x00001684 /* Read channel EDC-N VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS0    0x00001688 /* Read channel CS_N[0] read enable VDL control register */
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS1    0x0000168c /* Read channel CS_N[1] read enable VDL control register */
#define PHY_BYTE_LANE_1_VDL_CLK_CONTROL          0x00001690 /* DDR interface signal Write Leveling CLK VDL control register */
#define PHY_BYTE_LANE_1_VDL_LDE_CONTROL          0x00001694 /* DDR interface signal Write Leveling Capture Enable VDL control register */
#define PHY_BYTE_LANE_1_RD_EN_DLY_CYC            0x000016a0 /* Read enable bit-clock cycle delay control register */
#define PHY_BYTE_LANE_1_WR_CHAN_DLY_CYC          0x000016a4 /* Write leveling bit-clock cycle delay control register */
#define PHY_BYTE_LANE_1_READ_CONTROL             0x000016b0 /* Read channel datapath control register */
#define PHY_BYTE_LANE_1_READ_FIFO_ADDR           0x000016b4 /* Read fifo addresss pointer register */
#define PHY_BYTE_LANE_1_READ_FIFO_DATA           0x000016b8 /* Read fifo data register */
#define PHY_BYTE_LANE_1_READ_FIFO_DM_DBI         0x000016bc /* Read fifo dm/dbi register */
#define PHY_BYTE_LANE_1_READ_FIFO_STATUS         0x000016c0 /* Read fifo status register */
#define PHY_BYTE_LANE_1_READ_FIFO_CLEAR          0x000016c4 /* Read fifo status clear register */
#define PHY_BYTE_LANE_1_IDLE_PAD_CONTROL         0x000016c8 /* Idle mode SSTL pad control register */
#define PHY_BYTE_LANE_1_DRIVE_PAD_CTL            0x000016cc /* SSTL pad drive characteristics control register */
#define PHY_BYTE_LANE_1_RD_EN_DRIVE_PAD_CTL      0x000016d0 /* SSTL read enable pad drive characteristics control register */
#define PHY_BYTE_LANE_1_STATIC_PAD_CTL           0x000016d4 /* pad rx and tx characteristics control register */
#define PHY_BYTE_LANE_1_WR_PREAMBLE_MODE         0x000016d8 /* Write cycle preamble control register */
#define PHY_BYTE_LANE_1_ODT_CONTROL              0x000016e0 /* Read channel ODT control register */
#define PHY_BYTE_LANE_1_EDC_DPD_CONTROL          0x000016f0 /* GDDR5M EDC digital phase detector control register */
#define PHY_BYTE_LANE_1_EDC_DPD_STATUS           0x000016f4 /* GDDR5M EDC digital phase detector status register */
#define PHY_BYTE_LANE_1_EDC_DPD_OUT_CONTROL      0x000016f8 /* GDDR5M EDC digital phase detector output signal control register */
#define PHY_BYTE_LANE_1_EDC_DPD_OUT_STATUS       0x000016fc /* GDDR5M EDC digital phase detector output signal status register */
#define PHY_BYTE_LANE_1_EDC_DPD_OUT_STATUS_CLEAR 0x00001700 /* GDDR5M EDC digital phase detector output signal status clear register */
#define PHY_BYTE_LANE_1_EDC_CRC_CONTROL          0x00001704 /* GDDR5M EDC signal path CRC control register */
#define PHY_BYTE_LANE_1_EDC_CRC_STATUS           0x00001708 /* GDDR5M EDC signal path CRC status register */
#define PHY_BYTE_LANE_1_EDC_CRC_COUNT            0x0000170c /* GDDR5M EDC signal path CRC counter register */
#define PHY_BYTE_LANE_1_EDC_CRC_STATUS_CLEAR     0x00001710 /* GDDR5M EDC signal path CRC counter register */
#define PHY_BYTE_LANE_1_BL_SPARE_REG             0x00001714 /* Byte-Lane Spare register */

/* Offset must be lower than 4K for ARM LDR/STR instructions */
#define PHY_CONTROL_REGS_DRAM_CONFIG_OFF_4K      (PHY_CONTROL_REGS_DRAM_CONFIG - 0x1000)
#define PHY_CONTROL_REGS_DRAM_TIMING1_OFF_4K     (PHY_CONTROL_REGS_DRAM_TIMING1 - 0x1000)
#define PHY_CONTROL_REGS_DRAM_TIMING2_OFF_4K     (PHY_CONTROL_REGS_DRAM_TIMING2 - 0x1000)
#define PHY_CONTROL_REGS_DRAM_TIMING3_OFF_4K     (PHY_CONTROL_REGS_DRAM_TIMING3 - 0x1000)
#define PHY_CONTROL_REGS_DRAM_TIMING4_OFF_4K     (PHY_CONTROL_REGS_DRAM_TIMING4 - 0x1000)
#define PHY_CONTROL_REGS_PLL_CONFIG_OFF_4K       (PHY_CONTROL_REGS_PLL_CONFIG - 0x1000)
#define PHY_CONTROL_REGS_PLL_DIVIDERS_OFF_4K     (PHY_CONTROL_REGS_PLL_DIVIDERS - 0x1000)
#define PHY_CONTROL_REGS_PLL_CONTROL2_OFF_4K     (PHY_CONTROL_REGS_PLL_CONTROL2 - 0x1000)
#define PHY_CONTROL_REGS_VREF_DAC_CONTROL_OFF_4K (PHY_CONTROL_REGS_VREF_DAC_CONTROL - 0x1000)
#define PHY_CONTROL_REGS_VDL_CALIBRATE_OFF_4K    (PHY_CONTROL_REGS_VDL_CALIBRATE - 0x1000)
#define PHY_BYTE_LANE_0_RD_EN_DLY_CYC_OFF_4K     (PHY_BYTE_LANE_0_RD_EN_DLY_CYC - 0x1000)
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS0_OFF_4K (PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS0 - 0x1000)
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS1_OFF_4K (PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS1 - 0x1000)
#define PHY_BYTE_LANE_1_RD_EN_DLY_CYC_OFF_4K     (PHY_BYTE_LANE_1_RD_EN_DLY_CYC - 0x1000)
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS0_OFF_4K (PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS0 - 0x1000)
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS1_OFF_4K (PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS1 - 0x1000)
#define PHY_CONTROL_REGS_DRIVE_PAD_CTL_OFF_4K    (PHY_CONTROL_REGS_DRIVE_PAD_CTL - 0x1000)
#define PHY_BYTE_LANE_0_DRIVE_PAD_CTL_OFF_4K     (PHY_BYTE_LANE_0_DRIVE_PAD_CTL - 0x1000)
#define PHY_BYTE_LANE_1_DRIVE_PAD_CTL_OFF_4K     (PHY_BYTE_LANE_1_DRIVE_PAD_CTL - 0x1000)
#define PHY_BYTE_LANE_0_RD_EN_DRIVE_PAD_CTL_OFF_4K (PHY_BYTE_LANE_0_RD_EN_DRIVE_PAD_CTL - 0x1000)
#define PHY_BYTE_LANE_1_RD_EN_DRIVE_PAD_CTL_OFF_4K (PHY_BYTE_LANE_1_RD_EN_DRIVE_PAD_CTL - 0x1000)
#define PHY_CONTROL_REGS_STATIC_PAD_CTL_OFF_4K   (PHY_CONTROL_REGS_STATIC_PAD_CTL - 0x1000)
#define PHY_BYTE_LANE_0_WR_PREAMBLE_MODE_OFF_4K  (PHY_BYTE_LANE_0_WR_PREAMBLE_MODE - 0x1000)
#define PHY_BYTE_LANE_1_WR_PREAMBLE_MODE_OFF_4K  (PHY_BYTE_LANE_1_WR_PREAMBLE_MODE - 0x1000)
#define PHY_BYTE_LANE_0_IDLE_PAD_CONTROL_OFF_4K  (PHY_BYTE_LANE_0_IDLE_PAD_CONTROL - 0x1000)
#define PHY_BYTE_LANE_1_IDLE_PAD_CONTROL_OFF_4K  (PHY_BYTE_LANE_1_IDLE_PAD_CONTROL - 0x1000)
#define PHY_CONTROL_REGS_DFI_CNTRL_OFF_4K        (PHY_CONTROL_REGS_DFI_CNTRL - 0x1000)
#define PHY_CONTROL_REGS_COMMAND_AUX_REG1_OFF_4K (PHY_CONTROL_REGS_COMMAND_AUX_REG1 - 0x1000)
#define PHY_CONTROL_REGS_COMMAND_REG1_OFF_4K     (PHY_CONTROL_REGS_COMMAND_REG1 - 0x1000)
#define PHY_CONTROL_REGS_PLL_STATUS_OFF_4K       (PHY_CONTROL_REGS_PLL_STATUS - 0x1000)
#define PHY_CONTROL_REGS_VDL_CALIB_STATUS1_OFF_4K (PHY_CONTROL_REGS_VDL_CALIB_STATUS1 - 0x1000)

/*
#####################################################################
# UART0 Control Registers -- Peripheral UART
#####################################################################
*/
#define UART0RXTIMEOUT   0x00
#define UART0CONFIG      0x01
#define UART0CONTROL     0x02
#define UART0BAUD        0x04
#define UART0FIFOCFG     0x09
#define UART0INTSTAT     0x10
#define UART0INTMASK     0x12
#define UART0DATA        0x14

#define BRGEN            0x80   /* Control register bit defs */
#define TXEN             0x40
#define RXEN             0x20
#define LOOPBK           0x10
#define TXPARITYEN       0x08
#define TXPARITYEVEN     0x04
#define RXPARITYEN       0x02
#define RXPARITYEVEN     0x01

#define XMITBREAK        0x40   /* Config register */
#define BITS5SYM         0x00
#define BITS6SYM         0x10
#define BITS7SYM         0x20
#define BITS8SYM         0x30
#define ONESTOP          0x07
#define TWOSTOP          0x0f

#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000

/*
#####################################################################
# UART2 Control Registers -- ARM UART
#####################################################################
*/
#define UART2DATA        0x00
#define UART2STATUS      0x04
#define UART2FLAGS       0x18
#define UART2INTBAUD     0x24
#define UART2FRACBAUD    0x28
#define UART2LINECTRL    0x2C
#define UART2CTRL        0x30

/*
#####################################################################
# GIC reigsters
#####################################################################
*/
#define GICD_CTLR_OFFSET        0x0
#define	GICD_TYPER_OFFSET       0x4
#define GICD_IGROUPR0_OFFSET    0x80
#define GICD_IGROUPR8_OFFSET    0xA0
#define GICD_PRIORITY_OFFSET    0x400
#define GICC_CTLR_OFFSET        0x0
#define GICC_PMR_OFFSET         0x4
#define GICC_BPR_OFFSET         0x8

/*
#####################################################################
# Internal Memory utilization (by internal bootrom)
#####################################################################
*/

#define BTRM_INT_MEM_UTIL_SIZE                  (512 * 1024) /* 0x80000 */
#define BTRM_INT_MEM_BEGIN_ADDR                 0x80700000
#define BTRM_INT_MEM_END_ADDR                   (BTRM_INT_MEM_BEGIN_ADDR + BTRM_INT_MEM_UTIL_SIZE)
#define BTRM_INT_MEM_INCLR_CFE_RAM_ADDR         (BTRM_INT_MEM_BEGIN_ADDR + 0x01000)
#define BTRM_INT_MEM_SHREDDER_PROG_ADDR         (BTRM_INT_MEM_BEGIN_ADDR + 0x3d000)
#define BTRM_INT_MEM_TP1_PROG_ADDR              (BTRM_INT_MEM_BEGIN_ADDR + 0x3ef00)
#define BTRM_INT_MEM_CREDENTIALS_ADDR           (BTRM_INT_MEM_BEGIN_ADDR + 0x3f000)
#define BTRM_INT_MEM_CREDENTIALS_END_ADDR       (BTRM_INT_MEM_BEGIN_ADDR + 0x3f250) /* 512 + 64 + 16 */
#define BTRM_INT_MEM_SBI_LINK_ADDR              (BTRM_INT_MEM_BEGIN_ADDR + 0x40000) /*BTRM auths it here. CFE ROM relocates itself to offset 0x10000 */
#define BTRM_INT_MEM_COMP_CFE_RAM_ADDR          (BTRM_INT_MEM_BEGIN_ADDR + 0x40000)
#define BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR     (BTRM_INT_MEM_BEGIN_ADDR + 0x41000)
#define BTRM_INT_MEM_CFE_ROM_END_ADDR		(BTRM_INT_MEM_BEGIN_ADDR + 0x60000)

#ifdef __cplusplus
}
#endif

#endif
