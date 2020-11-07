/*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
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
/*   MODULE:  4908_common.h                                            */
/*   DATE:    04/16/15                                                 */
/*   PURPOSE: Register definition used by assembly for BCM4908         */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM4908_MAP_COMMON_H
#define __BCM4908_MAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*
#####################################################################
# Perf Control Registers
#####################################################################
*/

#define PERF_CHIPID                 0x0
#define PERF_CHIPID_CHIP_ID_SHIFT   16
#define PERF_CHIPID_CHIP_ID_MASK    (0xffff << PERF_CHIPID_CHIP_ID_SHIFT)
#define PERF_CHIPID_REV_ID_SHIFT    0
#define PERF_CHIPID_REV_ID_MASK     0xff

/*
#####################################################################
# BOOT LUT Registers
#####################################################################
*/
#define BOOT_LUT_RST               0x20

/*
#####################################################################
# GIC Registers
#####################################################################
*/

#define GICD_CTLR                  0x0
#define GICD_TYPER                 0x4
#define GICD_IGROUPR0              0x80
#define GICD_IGROUPR8              0xa0

#define GICC_CTLR                  0x0
#define GICC_PMR                   0x4

/*
#####################################################################
# CPU BIU Control Register
#####################################################################
*/
#define CPU_POWER_CONFIG           0x88
#define CPU_RESET_CONFIG           0x8c
#define CPU_CLOCK_CONFIG           0x90
#define CPU_CLUSTER0_CLOCK_CONTROL 0x94
#define CPU_UBUS_CONFIG            0x1d8

/*
#####################################################################
# GPIO Control Registers
#####################################################################
*/

#define GPIO_DATA                  (GPIO_BASE + 0x28)

/*
#####################################################################
# Perf/Interrupt Registers
#####################################################################
*/


/*
#####################################################################
# Timer Registers
#####################################################################
*/


/*
#####################################################################
# SRAM Sec Registers
#####################################################################
*/

#define SRAM_SEC_ACCESS_CNTRL                   0x00
#define SRAM_SEC_SRAM_RDY_STATUS                0x04

/*
#####################################################################
# Secure Boot Bootrom Registers
#####################################################################
*/
#define BROM_GEN_SECBOOTCFG                     0x00
#define BROM_GEN_SECBOOTCFG_JTAG_UNLOCK         (1<<1)
#define BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK      (1<<2)
#define BROM_GEN_SECBOOTCFG_INTF_UNLOCK         (BROM_GEN_SECBOOTCFG_JTAG_UNLOCK | BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK)

#define BROM_SEC1_ACCESS_CNTRL                   0x00
/* #define BROM_SEC_ACCESS_CNTRL_DISABLE_BTRM      0x0c  */
#define BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM      0x00    /* Permenantly disable r/w to bootrom addr space and BROM_SEC_ACCESS_CNTRL register */

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_0       0x04
#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_1       0x08
#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_2       0x0c
#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_3       0x10

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

/* row 10 */
#define OTP_SATA_DISABLE_ROW			10
#define OTP_SATA_DISABLE_SHIFT			24
#define OTP_SATA_DISABLE_MASK			(0x1 << OTP_SATA_DISABLE_SHIFT)

/* row 11 */
#define OTP_PMC_BOOT_ROW			11
#define OTP_PMC_BOOT_SHIFT			25
#define OTP_PMC_BOOT_MASK			(0x1 << OTP_PMC_BOOT_SHIFT)

/* row 12 */
#define OTP_PCM_DISABLE_ROW			12
#define OTP_PCM_DISABLE_SHIFT			6
#define OTP_PCM_DISABLE_MASK			(0x1 << OTP_PCM_DISABLE_SHIFT)

/* row 14 */
#define OTP_CPU_CORE_CFG_ROW			14
#define OTP_CPU_CORE_CFG_SHIFT			14
#define OTP_CPU_CORE_CFG_MASK			(0x3 << OTP_CPU_CORE_CFG_SHIFT)

#define OTP_SGMII_DISABLE_ROW			14
#define OTP_SGMII_DISABLE_SHIFT			17
#define OTP_SGMII_DISABLE_MASK			(0x1 << OTP_SGMII_DISABLE_SHIFT) 

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

#define OTP_READ_TIMEOUT_CNTR                   0x100000

/*
#####################################################################
# Miscellaneous Registers
#####################################################################
*/

#define MISC_STRAP_BUS                          0x00

#define MISC_STRAP_BUS_BOOTSEL_SHIFT            0
#define MISC_STRAP_BUS_BOOTSEL_MASK             (0x38 << MISC_STRAP_BUS_BOOTSEL_SHIFT)

#define MISC_STRAP_BUS_BOOT_NAND_MASK           0x20    /* 0b0xxxxx are all nand settings, therefore bottom five bits are don't care*/

#define MISC_STRAP_BUS_BOOT_SPINOR              0x38    /* valid spi nor values: 0b111xxx */
#define MISC_STRAP_BUS_BOOT_SPINAND             0x28    /* valid spi nor values: 0b101xxx */
#define MISC_STRAP_BUS_BOOT_EMMCFL              0x30    /* valid spi nor values: 0b110xxx */

#define MISC_STRAP_BUS_BOOTROM_BOOT_N_SHIFT	7
#define MISC_STRAP_BUS_BOOTROM_BOOT_N_MASK	(0x1 << MISC_STRAP_BUS_BOOTROM_BOOT_N_SHIFT)

/*
#####################################################################
# SOTP Registers
#####################################################################
*/

#define SOTP_OTP_PROG_CTRL			0x00

#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT   8
#define SOTP_OTP_PROG_CTRL_OTP_ECC_WREN 	(1 << SOTP_OTP_PROG_CTRL_OTP_ECC_WREN_SHIFT)

#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT 9
#define SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC 	(1 << SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC_SHIFT)

#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT 15
#define SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN	(1 << SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN_SHIFT)

#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT     17
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK      (0xf << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT) /* bits 20:17 */
#define SOTP_OTP_PROG_CTRL_REGS_ECC_EN           (0xa << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)
#define SOTP_OTP_PROG_CTRL_REGS_ECC_DIS          (0x5 << SOTP_OTP_PROG_CTRL_REGS_ECC_EN_SHIFT)

/* --- */

#define SOTP_OTP_WDATA_0			0x04

/* --- */

#define SOTP_OTP_WDATA_1			0x08
#define SOTP_OTP_WDATA_1_FAIL_SHIFT		7
#define SOTP_OTP_WDATA_1_FAIL_MASK		(0x3 << SOTP_OTP_WDATA_1_FAIL_SHIFT)

/* --- */

#define SOTP_OTP_ADDR				0x0c
#define SOTP_OTP_ADDR_OTP_ADDR_SHIFT		6

/* --- */

#define SOTP_OTP_CTRL_0				0x10

#define SOTP_OTP_CTRL_0_OTP_CMD_SHIFT		1
#define SOTP_OTP_CTRL_0_OTP_CMD_MASK		(0x1f << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) /* bits [05:01] */
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ	(0x0 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 
#define SOTP_OTP_CTRL_0_OTP_CMD_OTP_PROG_ENABLE	(0x2 << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 
#define SOTP_OTP_CTRL_0_OTP_CMD_PROG		(0xa << SOTP_OTP_CTRL_0_OTP_CMD_SHIFT) 

#define SOTP_OTP_CTRL_0_START_SHIFT		0
#define SOTP_OTP_CTRL_0_START            	(0x1 << SOTP_OTP_CTRL_0_START_SHIFT)   

/* --- */

#define SOTP_OTP_STATUS_1			0x1c

#define SOTP_OTP_STATUS_1_CMD_DONE_SHIFT	1
#define SOTP_OTP_STATUS_1_CMD_DONE	        (0x1 << SOTP_OTP_STATUS_1_CMD_DONE_SHIFT) 

#define SOTP_OTP_STATUS_1_ECC_COR_SHIFT		16
#define SOTP_OTP_STATUS_1_ECC_COR	        (0x1 << SOTP_OTP_STATUS_1_ECC_COR_SHIFT) 

#define SOTP_OTP_STATUS_1_ECC_DET_SHIFT		17
#define SOTP_OTP_STATUS_1_ECC_DET	        (0x1 << SOTP_OTP_STATUS_1_ECC_DET_SHIFT) 

/* --- */

#define SOTP_OTP_RDATA_0			0x20

/* --- */

#define SOTP_OTP_RDATA_1			0x24
#define SOTP_OTP_RDATA_1_FAIL_SHIFT		7
#define SOTP_OTP_RDATA_1_FAIL_MASK		(0x3 << SOTP_OTP_RDATA_1_FAIL_SHIFT)

/* --- */

#define SOTP_OTP_REGION_RD_LOCK			0x3c

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


/*
#####################################################################
# PMB registers
#####################################################################
*/

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

#define MEMC_CHN_TIM_PHY_ST                        0x230
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_POWER_UP        0x1

/***************************************************************************
 *MEMC_SRAM_REMAP - "MC non-secure address remapping to internal SRAM"
 ***************************************************************************/

#define MEMC_SRAM_REMAP_CONTROL                    0x00000020 /* SRAM Remap Control */
#define MEMC_SRAM_REMAP_INIT                       0x00000024 /* SRAM Remap Initialization */
#define MEMC_SRAM_REMAP_LOG_INFO_0                 0x00000028 /* SRAM Remap Log Info 0 */
#define MEMC_SRAM_REMAP_LOG_INFO_1                 0x0000002c /* SRAM Remap Log Info 1 */

/***************************************************************************
 *DDRPHY Registers
 ***************************************************************************/
#define DDRPHY_OFFSET                              0x00002000 /* offset from memc base */
#define DDRPHY_VREF_DAC_CTRL                       0x0000019c
#define DDRPHY_VREF_DAC_CTRL_PDN_SHIFT             12
#define DDRPHY_VREF_DAC_CTRL_PDN_MASK              (0xf << DDRPHY_VREF_DAC_CTRL_PDN_SHIFT)
#define DDRPHY_VREF_DAC_CTRL_DAC1_SHIFT            6
#define DDRPHY_VREF_DAC_CTRL_DAC1_MASK             (0x3f << DDRPHY_VREF_DAC_CTRL_DAC1_SHIFT)
#define DDRPHY_VREF_DAC_CTRL_DAC0_SHIFT            0
#define DDRPHY_VREF_DAC_CTRL_DAC0_MASK             (0x3f << DDRPHY_VREF_DAC_CTRL_DAC0_SHIFT)

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
# Internal Memory utilization (by internal bootrom)
#####################################################################
*/

#define BTRM_INT_SRAM_STD_32K_SIZE              (32 * 1024)
#define BTRM_INT_SRAM_STD_32K_ADDR              0xfff80000

#define BTRM_INT_SRAM_RDP_32K_SIZE              (32 * 1024)
#define BTRM_INT_SRAM_RDP_32K_ADDR              0x82200000

#define BTRM_INT_SRAM_RDP_48K_SIZE              (48 * 1024)
#define BTRM_INT_SRAM_RDP_48K_0_ADDR            0x82210000
#define BTRM_INT_SRAM_RDP_48K_1_ADDR            0x82250000

#define BTRM_INT_SRAM_RDP_128K_SIZE             (128 * 1024)
#define BTRM_INT_SRAM_RDP_128K_ADDR             0x822a0000

/* this memory is only available for mmu table when mmu is enabled */
#define BTRM_INT_SRAM_MEMC_64K_SIZE             (64 * 1024)
#define BTRM_INT_SRAM_MEMC_64K_ADDR             0x7fff0000

/* Location of Bootrom .data, .bss, stack and heap */
#define BTRM_INT_MEM_UTIL_SIZE                  0x40000 	/* 256K of Virtually mapped SRAM */
#define BTRM_INT_MEM_BEGIN_ADDR                 0x90000000
#define BTRM_INT_MEM_END_ADDR                   (BTRM_INT_MEM_BEGIN_ADDR + BTRM_INT_MEM_UTIL_SIZE)

/* Loctions of shredder code and security credentials */
/* reserve 1KB for IOTP code */
#define BTRM_INT_MEM_SHREDDER_TRANSIT_PROG_ADDR (BTRM_INT_SRAM_STD_32K_ADDR + 0x400)
#define BTRM_INT_MEM_MMU_DIS_ADDR               (BTRM_INT_SRAM_STD_32K_ADDR + 0x6000)
#define BTRM_INT_MEM_CREDENTIALS_ADDR           (BTRM_INT_SRAM_STD_32K_ADDR + 0x7000)
#define BTRM_INT_MEM_CREDENTIALS_ADDR_END       (BTRM_INT_SRAM_STD_32K_ADDR + BTRM_INT_SRAM_STD_32K_SIZE)

/* Location of CFE ROM link address, Bootrom will authenticate it at this location */
/* IMPORTANT: ENSURE THAT THE FOLLOWING VIRT/PHYSICAL ADDRESSES MAP TO EACH OTHER! */
#define BTRM_INT_MEM_SBI_LINK_ADDR_VIRT         (BTRM_INT_MEM_BEGIN_ADDR + 0x20000)
#define BTRM_INT_MEM_SBI_LINK_ADDR_PHYS         (BTRM_INT_SRAM_RDP_128K_ADDR)

#define BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR     (0x300000) /* use DDR */
#define BTRM_INT_MEM_COMP_CFE_RAM_ADDR          (BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR - 0x1000)  

#ifdef __cplusplus
}
#endif

#endif
