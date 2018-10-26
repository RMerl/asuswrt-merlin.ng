/*
<:copyright-BRCM:2016:proprietary:standard

   Copyright (c) 2016 Broadcom
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
/*   MODULE:  63158_common.h                                           */
/*   DATE:    08/06/15                                                 */
/*   PURPOSE: Register definition used by assembly for BCM63158        */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM63158_MAP_COMMON_H
#define __BCM63158_MAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*
#####################################################################
# Perf Control Registers
#####################################################################
*/

#define PERF_CHIPID                 0x0
#define PERF_CHIPID_CHIP_ID_SHIFT   12
#define PERF_CHIPID_CHIP_ID_MASK    (0xfffff << PERF_CHIPID_CHIP_ID_SHIFT)
#define PERF_CHIPID_REV_ID_SHIFT    0
#define PERF_CHIPID_REV_ID_MASK     0xfff

/*
#####################################################################
# Miscellaneous Registers
#####################################################################
*/

#define MISC_STRAP_BUS                          0x00

#define MISC_STRAP_BUS_BOOTSEL_SHIFT            3     /* shift over to where boot_sel[3] is */
#define MISC_STRAP_BUS_BOOTSEL_MASK             (0x7 << MISC_STRAP_BUS_BOOTSEL_SHIFT)

#define MISC_STRAP_BUS_BOOT_NAND_MASK           0x4     /* 0b0xxxxx are all nand settings, therefore bottom five bits are don't care*/
#define MISC_STRAP_BUS_BOOT_SPINOR              0x7     /* valid spi nor values:  0b111xxx */
#define MISC_STRAP_BUS_BOOT_SPINAND             0x5     /* valid spi nand values: 0b101xxx */
#define MISC_STRAP_BUS_BOOT_EMMCFL              0x6     /* valid emmc values:     0b110xxx */
#define MISC_STRAP_BUS_BOOT_ETH                 0x4     /* valid ethernet values: 0b100xxx */
#define MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT      15
#define MISC_STRAP_BUS_CPU_SLOW_FREQ            (0x1 << MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT)

/*
#####################################################################
# BIU config Registers
#####################################################################
*/
#define TS0_CTRL_CNTCR                          0x1000

/*
* #####################################################################
* # SOTP Registers
* #####################################################################
* */

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
# GIC Registers
#####################################################################
*/

#define GICD_CTLR                                0x0
#define GICD_TYPER                               0x4
#define GICD_IGROUPR0                            0x80
#define GICD_IGROUPR8                            0xa0

#define GICC_CTLR                                0x0
#define GICC_PMR                                 0x4

  /*
#####################################################################
# Memory Control Registers
#####################################################################
*/

#define MEMC_GLB_VERS                              0x00000000 /* MC Global Version Register */
#define MEMC_GLB_GCFG                              0x00000004 /* MC Global Configuration Register */
#define MEMC_GLB_GCFG_GCFG_DRAM_EN                 (1<<31)
#define MEMC_GLB_GCFG_MEM_INIT_DONE                (1<<8)

#define MEMC_GLB_FSBL_STATE                        0x10      /* Firmware state scratchpad */
#define MEMC_GLB_FSBL_DRAM_SIZE_SHIFT              0
#define MEMC_GLB_FSBL_DRAM_SIZE_MASK               (0xf << MEMC_GLB_FSBL_DRAM_SIZE_SHIFT)

#define MEMC_CHN_CFG_DRAM_SIZE_CHK                 0x00000140 /* DRAM wrap over check */
#define MEMC_CHN_CFG_DRAM_SIZE_CHK_LIMIT_MASK      (0xf<<4)
#define MEMC_CHN_CFG_DRAM_SIZE_CHK_LIMIT_SHIFT     4

/***************************************************************************
 *MEMC_SRAM_REMAP - "MC non-secure address remapping to internal SRAM"
 ***************************************************************************/

#define MEMC_SRAM_REMAP_CONTROL                    0x00000020 /* SRAM Remap Control */
#define MEMC_SRAM_REMAP_CONTROL_UPPER              0x00000024 /* SRAM Remap Control */
#define MEMC_SRAM_REMAP_INIT                       0x00000028 /* SRAM Remap Initialization */
#define MEMC_SRAM_REMAP_LOG_INFO_0                 0x0000002c /* SRAM Remap Log Info 0 */
#define MEMC_SRAM_REMAP_LOG_INFO_1                 0x00000030 /* SRAM Remap Log Info 1 */
#define MEMC_SRAM_REMAP_LOG_INFO_2                 0x00000034 /* SRAM Remap Log Info 2 */

/*
#####################################################################
# UART Control Registers
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
# ARM UART Control Registers
#####################################################################
*/
#define UART0DR          0x0
#define UART0RSR         0x4
#define UART0FR          0x18
#define UART0ILPR        0x20
#define UART0IBRD        0x24
#define UART0FBRD        0x28
#define UART0LCR_H       0x2c
#define UART0CR          0x30
#define UART0IFLS        0x34
#define UART0IMSC        0x38
#define UART0IRIS        0x3c
#define UART0IMIS        0x40
#define UART0ICR         0x44
#define UART0DMACR       0x48

#define LCR_FIFOEN       0x10
#define LCR_PAREN        0x02
#define LCR_8SYM_NOPAR_ONE_STOP_FIFOEN_CFG 0x70/*(BIT8SYM|FIFOEN|NOPAR|ONESTOP)*/
#define CR_TXE           0x100
#define CR_RXE           0x200
#define CR_EN            0x1

#define FR_TXFE          0x80
#define FR_RXFF          0x40
#define FR_TXFF          0x20
#define FR_RXFE          0x10
#define FR_BUSY          0x04

/*
#####################################################################
# Secure Boot Bootrom Registers
#####################################################################
*/

#define BROM_GEN_SECBOOTCFG                     0x00
#define BROM_GEN_SECBOOTCFG_BTRM_LOCKDOWN       (1<<0)
#define BROM_GEN_SECBOOTCFG_JTAG_UNLOCK         (1<<1)
#define BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK      (1<<2)
#define BROM_GEN_SECBOOTCFG_TBUS_UNLOCK         (1<<3)
#define BROM_GEN_SECBOOTCFG_INTF_UNLOCK         (BROM_GEN_SECBOOTCFG_JTAG_UNLOCK | BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK | BROM_GEN_SECBOOTCFG_TBUS_UNLOCK)

#define BROM_SEC1_ACCESS_CNTRL                  0x00
#define BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM     0x00

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_0      0x04

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_1      0x08

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_2      0x0c

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_3      0x10

/*
#####################################################################
# Crypto block - SKP OTR accessed in assembly when disabling SKP block
#####################################################################
*/

#define SKP_OTR_SKP_OTRC                   0x50
#define SKP_OTR_SKP_OTRC_U_DIS             (3 << 20)

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

#define JTAG_OTP_GENERAL_CTRL_3                 0x10

#define JTAG_OTP_GENERAL_STATUS_0               0x18

#define JTAG_OTP_GENERAL_STATUS_1               0x20
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
#define OTP_PCM_DISABLE_SHIFT			12
#define OTP_PCM_DISABLE_MASK			(0x1 << OTP_PCM_DISABLE_SHIFT)

/* row 14 */
#define OTP_CPU_CLOCK_FREQ_ROW			14
#define OTP_CPU_CLOCK_FREQ_SHIFT		9
#define OTP_CPU_CLOCK_FREQ_MASK			(0x7 << OTP_CPU_CLOCK_FREQ_SHIFT)

#define OTP_CPU_CORE_CFG_ROW			14
#define OTP_CPU_CORE_CFG_SHIFT			14
#define OTP_CPU_CORE_CFG_MASK			(0x3 << OTP_CPU_CORE_CFG_SHIFT)

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (7 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 19 */
#define OTP_CUST_BTRM_UART_DISABLE_ROW          19
#define OTP_CUST_BTRM_UART_DISABLE_SHIFT        29
#define OTP_CUST_BTRM_UART_DISABLE_MASK         (1 << OTP_CUST_BTRM_UART_DISABLE_SHIFT)

/* row 23 */
#define OTP_CUST_MFG_MRKTID_ROW                 23
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)


/* row 26 */
#define OTP_BOOT_SW_ENET_BOOT_DIS_ROW       	26
#define OTP_BOOT_SW_ENET_BOOT_DIS_SHIFT      	0
#define OTP_BOOT_SW_ENET_BOOT_DIS_MASK      	(7 << OTP_BOOT_SW_ENET_BOOT_DIS_SHIFT)


#define OTP_BOOT_SW_ENET_BOOT_FALLBACK_SHIFT    3
#define OTP_BOOT_SW_ENET_BOOT_FALLBACK_MASK     (7 << OTP_BOOT_SW_ENET_BOOT_FALLBACK_SHIFT)


#define OTP_BOOT_SW_ENET_RGMII_SHIFT		6
#define OTP_BOOT_SW_ENET_RGMII_MASK  		(7 << OTP_BOOT_SW_ENET_RGMII_SHIFT)

#define OTP_READ_TIMEOUT_CNTR                   0x100000

/*
#####################################################################
# System Port Registers
#####################################################################
*/

/* RBUF registers */
#define SP_RBUF_CONTROL_REG                           0x0400
#define SP_RBUF_PACKET_READY_THRESH_REG               0x0404

/* UMAC Registers */
#define SP_UMAC_CMD_REG                               0x0808
#define SP_UMAC_RX_MAX_PKT_SIZE                       0x0e08


/* RDMA registers */
#define SP_RDMA_CONTROL_REG                           0x3000
#define SP_RDMA_STATUS_REG                            0x3004
#define SP_RDMA_BSRS_REG                              0x300c
#define SP_RDMA_PRODUCER_INDEX_REG                    0x3018
#define SP_RDMA_CONSUMER_INDEX_REG                    0x301c
#define SP_RDMA_START_ADDR_LOW_REG                    0x3020


/* TDMA registers */
#define SP_TDMA_DESC_00_WRITE_PORT_LO_REG             0x4000
#define SP_TDMA_DESC_00_WRITE_PORT_HI_REG             0x4004
#define SP_TDMA_DESC_RING_00_HEAD_TAIL_PTR_REG        0x4280
#define SP_TDMA_DESC_RING_00_MAX_HYST_THRESH_REG      0x4288
#define SP_TDMA_DESC_RING_00_PROD_CONS_IDX_REG        0x4290

#define SP_TDMA_CONTROL_REG                           0x4600
#define SP_TDMA_STATUS_REG                            0x4604
#define SP_TDMA_TIER_1_ARB_0_QUEUE_ENABLE_REG         0x4628

/*
#####################################################################
# SF2 Switch Registers
#####################################################################
*/

#define SWITCH_REG_QPHY_CNTRL                        0x1c
#define SWITCH_REG_SPHY_CNTRL                        0x24
#define SPHY_CNTRL                                   (SWITCH_REG_BASE + SWITCH_REG_SPHY_CNTRL)
#define QPHY_CNTRL                                   (SWITCH_REG_BASE + SWITCH_REG_QPHY_CNTRL)
/*
#####################################################################
# GPIO Control Registers
#####################################################################
*/

#define GPIO_DIR0                         (GPIO_BASE + 0x00)
#define GPIO_DATA0                        (GPIO_BASE + 0x20)

#define GPIO_TEST_PORT_BLK_DATA_LSB       (GPIO_BASE + 0x58)
#define GPIO_TEST_PORT_CMD                (GPIO_BASE + 0x5c)

/*
 * #####################################################################
 * # Internal Memory utilization (by internal bootrom)
 * #####################################################################
 * */
/* Periph sram */
#define BTRM_INT_SRAM_STD_32K_SIZE              (32 * 1024)
#define BTRM_INT_SRAM_STD_32K_ADDR              0xfff80000

/* DSL LMEM */
#define BTRM_INT_LMEM_SIZE                      0xa0000         /* 704 KB less 64KB at the end for MMU table */
#define BTRM_INT_LMEM_ADDR                      0x80800000

#define BTRM_INT_MEM_SPU_DMA_ADDR               (BTRM_INT_LMEM_ADDR + 0x98000)
#define BTRM_INT_MEM_SPU_DMA_SIZE               0x8000

#define BTRM_INT_MEM_DOWNLOAD_OFFSET            0x1000
/* VA addres of the ring buffer used by tftp */
#define BTRM_LMEM_RING_BUFFER_ADDR              0x80816000

/* this memory is only available for mmu table when mmu is enabled */
#define BTRM_INT_SRAM_MEMC_64K_SIZE             (64 * 1024)
#define BTRM_INT_SRAM_MEMC_64K_ADDR             0x808a0000

/* Location of Bootrom .data, .bss, stack and heap */
#define BTRM_INT_MEM_UTIL_SIZE                  BTRM_INT_LMEM_SIZE         /* 704K of LMEM less 64KB for mmu table */
#define BTRM_INT_MEM_BEGIN_ADDR                 BTRM_INT_LMEM_ADDR
#define BTRM_INT_MEM_END_ADDR                   (BTRM_INT_MEM_BEGIN_ADDR + BTRM_INT_MEM_UTIL_SIZE)

/* Loctions of shredder code and security credentials */
/* reserve 1KB for IOTP code */
#define BTRM_INT_MEM_SHREDDER_TRANSIT_PROG_ADDR (BTRM_INT_SRAM_STD_32K_ADDR + 0x400)
#define BTRM_INT_MEM_CREDENTIALS_ADDR           (BTRM_INT_SRAM_STD_32K_ADDR + 0x7000)
#define BTRM_INT_MEM_CREDENTIALS_ADDR_END       (BTRM_INT_SRAM_STD_32K_ADDR + BTRM_INT_SRAM_STD_32K_SIZE)

/* Location of CFE ROM link address, Bootrom will authenticate it at this location */
/* IMPORTANT: ENSURE THAT THE FOLLOWING VIRT/PHYSICAL ADDRESSES MAP TO EACH OTHER! */
#define BTRM_INT_MEM_SBI_LINK_ADDR_VIRT         (BTRM_INT_MEM_BEGIN_ADDR + 0x20000)
#define BTRM_INT_MEM_SBI_LINK_ADDR_PHYS         BTRM_INT_MEM_SBI_LINK_ADDR_VIRT

#define BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR     (0x300000) /* use DDR */
#define BTRM_INT_MEM_COMP_CFE_RAM_ADDR          (BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR - 0x1000)  

#define BCM_BTRM_MAJOR    1
#define BCM_BTRM_MINOR    1
#define BCM_BTRM_EXTRA    5

#ifdef __cplusplus
}
#endif

#endif
