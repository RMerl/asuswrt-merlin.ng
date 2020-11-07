/*
<:copyright-BRCM:2012:proprietary:standard 

   Copyright (c) 2012 Broadcom 
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
/*   MODULE:  6878_common.h                                           */
/*   DATE:    09/04/18                                                 */
/*   PURPOSE: Register definition used by assembly for BCM6878        */
/*                                                                     */
/***********************************************************************/

/* BOOTROM inclusions */
#ifndef __BCM6878_MAP_COMMON_H
#define __BCM6878_MAP_COMMON_H

#if defined (_BOOTROM_)
#include "bcm_sbi_header.h"

#else

#define CFG_CHIP_SRAM				0x82600000	
#define CFG_CHIP_SRAM_SIZE			(1024*128)
#endif

#define SPIFLASH_PHYS_BASE          0xffd00000  
#define SPIFLASH_SIZE               0x100000    
#define NANDFLASH_PHYS_BASE         0xffe00000  
#define NANDFLASH_SIZE              0x100000    

/*
#####################################################################
# Memory Control Registers
#####################################################################
*/

#define MEMC_GLB_VERS                              0x00000000 /* MC Global Version Register */
#define MEMC_GLB_GCFG                              0x00000004 /* MC Global Configuration Register */
#define MEMC_GLB_GCFG_GCFG_DRAM_EN                 (1<<31)
#define MEMC_GLB_GCFG_MEM_INIT_DONE                (1<<8)

#define MEMC_GLB_FSBL_STATE            0x10      /* Firmware state scratchpad */
#define MEMC_GLB_FSBL_DRAM_SIZE_SHIFT          0
#define MEMC_GLB_FSBL_DRAM_SIZE_MASK           (0xf << MEMC_GLB_FSBL_DRAM_SIZE_SHIFT)

/* uart init usage in cferom*/
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
#define LCR_8SYM_NOPAR_ONE_STOP_NOFIFO_CFG 0x60/*(BIT8SYM|FIFODIS|NOPAR|ONESTOP)*/
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

/* row 9 */
#define OTP_CPU_CLOCK_FREQ_ROW			9
#define OTP_CPU_CLOCK_FREQ_SHIFT		0
#define OTP_CPU_CLOCK_FREQ_MASK			(0x7 << OTP_CPU_CLOCK_FREQ_SHIFT)

/* row 8 */
#define OTP_CPU_CORE_CFG_ROW			8
#define OTP_CPU_CORE_CFG_SHIFT			28
#define OTP_CPU_CORE_CFG_MASK			(0x1 << OTP_CPU_CORE_CFG_SHIFT) // 0=dual cores, 1=single core

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

/* row 34 */
#define OTP_BRCM_ENFORCE_BINIT_ROW 		17
#define OTP_BRCM_ENFORCE_BINIT_SHIFT		13
#define OTP_BRCM_ENFORCE_BINIT_MASK		(1 << OTP_BRCM_ENFORCE_BINIT_SHIFT)

/* row 26 - 32 hmid public hash*/
#define OTP_BOOT_SW_0                 		26
#define OTP_BOOT_SW_0_SHIFT                     0
#define OTP_BOOT_SW_0_MASK                	(0xffffffff << OTP_BOOT_SW_0_SHIFT)


#define OTP_READ_TIMEOUT_CNTR                   0x100000

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
# GPIO Control Registers
#####################################################################
*/

#define GPIO_DATA                  (GPIO_BASE + 0x20)
#define GP_OFFSET                  0x68

/***************************************************************************
 *MEMC_SRAM_REMAP - "MC non-secure address remapping to internal SRAM"
***************************************************************************/

#define MEMC_SRAM_REMAP_CONTROL                    0x00000020 /* SRAM Remap Control */
#define MEMC_SRAM_REMAP_INIT                       0x00000028 /* SRAM Remap Initialization */
#define MEMC_SRAM_REMAP_LOG_INFO_0                 0x0000002c /* SRAM Remap Log Info 0 */
#define MEMC_SRAM_REMAP_LOG_INFO_1                 0x00000030 /* SRAM Remap Log Info 1 */

/* SOTP*/
#define SOTP_PAC_CTRL			0x0
#define	PSRAM_BLKF_PHYS_BASE        0x82d9a800
#define	PSRAM_BLKF_PHYS_BASE_SIZE   0x3c0
#define	PSRAM_BLOCK_FUNC            PSRAM_BLKF_PHYS_BASE
#define PSRAM_BLKF_CFG_CTRL		0x0 
#define PSRAM_BLKF_CFG_SCRM_SEED	0x4
#define PSRAM_BLKF_CFG_SCRM_ADDR	0x8
#define RNG_CTRL_0			0x0
#define RNG_SOFT_RESET			0x4
#define RNG_TOTAL_BIT_COUNT		0xC
#define RNG_TOTAL_BIT_COUNT_TSH		0x10
#define RNG_FIFO_DATA			0x20
#define RNG_FIFO_COUNT			0x24

#ifdef __cplusplus
extern "C" {
#endif


/*
 * #####################################################################
 * # Secure Boot Bootrom Registers
 * #####################################################################
 */
#define BROM_GEN_SECBOOTCFG                     0x00
#define BROM_GEN_SECBOOTCFG_BTRM_MASK		0x1
#define BROM_GEN_SECBOOTCFG_BTRM_LOCKDOWN_SHIFT 0
#define BROM_GEN_SECBOOTCFG_BTRM_LOCKDOWN       (1<<0)
#define BROM_GEN_SECBOOTCFG_JTAG_UNLOCK_SHIFT   1
#define BROM_GEN_SECBOOTCFG_JTAG_UNLOCK         (1<<1)
#define BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK_SHIFT 2
#define BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK      (1<<2)
#define BROM_GEN_SECBOOTCFG_TBUS_UNLOCK         (1<<3)
#define BROM_GEN_SECBOOTCFG_INTF_UNLOCK         (BROM_GEN_SECBOOTCFG_JTAG_UNLOCK | BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK)
#define BROM_GEN_SECBOOTCFG_INTF_UNLOCK_BROM_LOCK         (BROM_GEN_SECBOOTCFG_JTAG_UNLOCK | BROM_GEN_SECBOOTCFG_SPI_SLV_UNLOCK|BROM_GEN_SECBOOTCFG_BTRM_LOCKDOWN)

#define BROM_SEC1_ACCESS_CNTRL                  0x00
#define BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM     0x00

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_0      0x04

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_1      0x08

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_2      0x0c

#define BROM_SEC1_ACCESS_RANGE_CHK_CNTRL_3      0x10

#define BROM_SEC_SECBOOTCFG_JTAG_UNLOCK         BROM_GEN_SECBOOTCFG_JTAG_UNLOCK
#define BROM_SEC_ACCESS_CNTRL                   BROM_SEC1_ACCESS_CNTRL
#define BROM_SEC_ACCESS_CNTRL_DISABLE_BTRM      BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM
#define BROM_SEC_SECBOOTCFG                     BROM_GEN_SECBOOTCFG

#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_SPI    0xffd00000
#define BTRM_EXT_MEM_ADDR_SBI_REGION_BGN_NAND   0xffe00000
#define BROM_SEC_SECBOOTCFG_JTAG_UNLOCK         BROM_GEN_SECBOOTCFG_JTAG_UNLOCK
#define BROM_SEC_ACCESS_CNTRL                   BROM_SEC1_ACCESS_CNTRL
#define BROM_SEC_ACCESS_CNTRL_DISABLE_BTRM      BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM
#define BROM_SEC_SECBOOTCFG                     BROM_GEN_SECBOOTCFG

/*
 * #####################################################################
 * # used by internal bootrom
 * #####################################################################
 */

/* internal memory */
#define BTRM_INT_BOOT_ADDR			CFG_CHIP_BROM 
#define BTRM_INT_BROM_SIZE			CFG_CHIP_BROM_SIZE 
/* needs to be a internal psram */

#define BTRM_INT_SRAM_ADDR			CFG_CHIP_SRAM
#define BTRM_INT_SRAM_SIZE			CFG_CHIP_SRAM_SIZE
/* BTRM Memory Resources  */

#define BTRM_DATA_BSS_RSRVD_SIZE		1024*19	/* BTRM DATA+BSS */
#define BTRM_STACK_RSRVD_SIZE			1024*8	/* STACK */
#define SBI_HDR_RSVD_SZ				(ALIGN((SEC_S_SIGNATURE*4 + BTRM_SBI_AUTH_HDR_MAX_SIZE + BTRM_SBI_UNAUTH_HDR_MAX_SIZE+1024),8))

/* SRAM working area */
/* Locations of shredder code and security credentials */

/* 
   SBI layout and staging area
   BTRM_SBI_AUTH_HDR_SIZE accounts for header and footer size
*/

#define	BTRM_INT_MEM_SBI_START_ADDR		(CFG_CHIP_SRAM)
/* sbi + header address + sig buffer address; needed for sbi processing */
#define	BTRM_INT_MEM_SBI_LOAD_ADDR		(BTRM_INT_MEM_SBI_START_ADDR+SBI_HDR_RSVD_SZ)
#define	BTRM_INT_MEM_SBI_END_ADDR		(BTRM_INT_MEM_SBI_START_ADDR+BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE+256*4+1024)

#define BTRM_INT_DATA_LINK_ADDR			ALIGN(BTRM_INT_MEM_SBI_END_ADDR,8)

#define BTRM_INT_MEM_BEGIN_ADDR                 (CFG_CHIP_SRAM)
#define BTRM_INT_MEM_END_ADDR                   (BTRM_INT_DATA_LINK_ADDR + BTRM_DATA_BSS_RSRVD_SIZE + BTRM_STACK_RSRVD_SIZE)

/*XARGS location to convey params to cfe_launch */
#define BTRM_INT_MEM_SBI_XARGS_ADDR		(ALIGN(BTRM_INT_MEM_END_ADDR,8)+32)
/* Credentials bounds*/
#define BTRM_INT_MEM_CREDENTIALS_ADDR           (CFG_CHIP_SRAM+CFG_CHIP_SRAM_SIZE-0x400)
#define BTRM_INT_MEM_CREDENTIALS_ADDR_END       (CFG_CHIP_SRAM+CFG_CHIP_SRAM_SIZE) 
/* Shredder */
#define BTRM_INT_MEM_SHREDDER_PROG_ADDR         (BTRM_INT_MEM_CREDENTIALS_ADDR-0x400) 
#define BTRM_INT_MEM_SHREDDER_PROG_END          (BTRM_INT_MEM_CREDENTIALS_ADDR) 
/* Aligned to 4k*/
/* 1-to-1 map*/
/* IMPORTANT: ENSURE THAT THE FOLLOWING VIRT/PHYSICAL ADDRESSES MAP TO EACH OTHER! */
#define BTRM_INT_MEM_SBI_LINK_ADDR_PHY        	(CFG_CHIP_SRAM)
#define BTRM_INT_MEM_SBI_LINK_ADDR		(BTRM_INT_MEM_SBI_LINK_ADDR_PHY )

#define BTRM_INT_MEM_AVS_LINK_ADDR               ALIGN((BTRM_INT_MEM_SBI_END_ADDR-SBI_HDR_RSVD_SZ),8)
/*BTRM_INT_MEM_SBI_START_ADDR*/
#define BTRM_INT_MEM_AVS_LINK_ADDR_END           BTRM_INT_MEM_SBI_END_ADDR

#define BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR     (0x300000) /* use DDR */
#define BTRM_INT_MEM_COMP_CFE_RAM_ADDR          (BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR - 0x1000)

#ifdef __cplusplus
}
#endif

#endif
