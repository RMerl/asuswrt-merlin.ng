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
/*   MODULE:  6848_common.h                                            */
/*   DATE:    23/11/14                                                 */
/*   PURPOSE: Register definition used by assembly for BCM6848         */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM6848_MAP_COMMON_H
#define __BCM6848_MAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*
#####################################################################
# GPIO Control Registers
#####################################################################
*/
#define GPIO_DATA                  (GPIO_BASE + 0x28) /* 0-31    */
#define GPIO_DATA_HI0              (GPIO_BASE + 0x2c) /* 32-63   */
#define GPIO_DATA_HI1              (GPIO_BASE + 0x30) /* 64-95   */
#define GPIO_DATA_HI2              (GPIO_BASE + 0x34) /* 96-127  */
#define GPIO_DATA_HI3              (GPIO_BASE + 0x38) /* 128-159 */
#define GPIO_DATA_HI4              (GPIO_BASE + 0x3c) /* 160-191 */
#define GPIO_DATA_HI5              (GPIO_BASE + 0x40) /* 192-223 */
#define GPIO_DATA_HI6              (GPIO_BASE + 0x44) /* 224-255 */
#define GPIO_DATA_HI7              (GPIO_BASE + 0x48) /* 256-287 */
#define GPIO_DATA_HI8              (GPIO_BASE + 0x4c) /* 288-319 */
#define GPIO_PER_TESTCONTROL	   (GPIO_BASE + 0x58)

/*
#####################################################################
# Miscellaneous Registers
#####################################################################
*/
#define MISC_STRAP_BUS                          0x0
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT           0
#define MISC_STRAP_BUS_BOOT_SEL_MASK            (0x38 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR             (0x38 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NAND            (0x28 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND                (0x20 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_PMC_BOOT_SHIFT			9
#define MISC_STRAP_BUS_PMC_BOOT_MASK			(1<<MISC_STRAP_BUS_PMC_BOOT_SHIFT)
#define MISC_STRAP_BUS_PMC_AVS_SHIFT			11
#define MISC_STRAP_BUS_PMC_AVS_MASK				(1<<MISC_STRAP_BUS_PMC_AVS_SHIFT)

#define MISC_STRAP_BUS_OVERRIDE                 0x4
#define MISC_PG_ALT_BV							0x3c

/*
#####################################################################
# Memory Control Registers
#####################################################################
*/

#define MEMC_GLB_GCFG        0x04
#define MEMC_GLB_GCFG_DRAM_EN_SHIFT        31
#define MEMC_GLB_GCFG_DRAM_EN_MASK         (0x1<<MEMC_GLB_GCFG_DRAM_EN_SHIFT)
#define MEMC_GLB_GCFG_MEMINITDONE_SHIFT    8
#define MEMC_GLB_GCFG_MEMINITDONE_MASK     (0x1<<MEMC_GLB_GCFG_MEMINITDONE_SHIFT)    

#define SDR_CFG_SDR_CFG      0x00
#define SDR_CFG_AUTO_REFRESH 0x24
#define SDR_CFG_TIMING_PARAM 0x28
    
#define SDR_PRECHARGE_CMD    0x80
#define SDR_AUTOREFRESH_CMD  0x90
#define SDR_LMR_CMD          0xA0
#define SDR_LMRX_CMD         0xA4

#define MEMC_LMBIF_0_REP_ARB_MODE                          0x484
#define MEMC_LMBIF_0_REP_ARB_MODE_FIFO_MODE_STRICT         1
/*
#####################################################################
# UART Control Registers
#####################################################################
*/
#define UART0CONTROL     0x01
#define UART0CONFIG      0x02
#define UART0RXTIMEOUT   0x03
#define UART0BAUD        0x04
#define UART0FIFOCFG     0x0a
#define UART0INTMASK     0x10
#define UART0INTSTAT     0x12
#define UART0DATA        0x17

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
# PMC registers
#####################################################################
*/

#define PMC_CONTROL_HOST_MBOX_IN               0x1028
#define PMC_CONTROL_HOST_MBOX_OUT              0x102c

#define PMC_DQM_CFG                            0x1800
#define PMC_DQM_NOT_EMPTY_IRQ_STS              0x1818
#define PMC_DQM_QUEUE_RST                      0x181c
#define PMC_DQM_NOT_EMPTY_STS                  0x1820
#define PMC_DQM_QUEUE_CNTRL_BASE               0x1a00
#define PMC_DQM_QUEUE_DATA_BASE                0x1c00

#define PMC_DQM_QUEUE_DATA_HOST_TO_PMC         (PMC_DQM_QUEUE_DATA_BASE+0x0)
#define PMC_DQM_QUEUE_DATA_PMC_TO_HOST         (PMC_DQM_QUEUE_DATA_BASE+0x10)

/*
#####################################################################
# Secure Boot Bootrom Registers
#####################################################################
*/

#define BROM_SEC_SECBOOTCFG                     0x14
#define BROM_SEC_SECBOOTCFG_BTRM_LOCKDOWN       (1<<0)
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
#define JTAG_OTP_GENERAL_CTRL_4                 0x10

#define JTAG_OTP_GENERAL_STATUS_0               0x14

#define JTAG_OTP_GENERAL_STATUS_1               0x18
#define JTAG_OTP_GENERAL_STATUS_1_CMD_DONE      (1 << 1)
/* ================================================================= */
/* row 10 */
#define OTP_BITS_320_351_ROW                    10
#define OTP_BRCM_FEATURE_DISABLE_ROW            OTP_BITS_320_351_ROW
#define OTP_BRCM_TP1_DISABLE_SHIFT              10
#define OTP_BRCM_TP1_DISABLE_MASK               (1<<OTP_BRCM_TP1_DISABLE_SHIFT)
#define OTP_BRCM_MAX_CLOCK_FREQ_SHIFT			0
#define OTP_BRCM_MAX_CLOCK_FREQ_MASK			(0x7<<OTP_BRCM_MAX_CLOCK_FREQ_SHIFT)
#define OTP_BRCM_MAX_DDR_FREQ_SHIFT				7
#define OTP_BRCM_MAX_DDR_FREQ_MASK				(0x3<<OTP_BRCM_MAX_DDR_FREQ_SHIFT)

#define OTP_BRCM_CHIPID_ROW                     11
#define OTP_BRCM_CHIP_REVID_SHIFT               20
#define OTP_BRCM_CHIP_REVID_MASK                (0xf << OTP_BRCM_CHIP_REVID_SHIFT)

/* row 17 */
#define OTP_BRCM_AUTH_DIS_ROW                   17
#define OTP_BRCM_ALLOW_SEC_BT_SHIFT             4
#define OTP_BRCM_ALLOW_SEC_BT_MASK              (0x1 << OTP_BRCM_ALLOW_SEC_BT_SHIFT)

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
# TOP Control Registers
#####################################################################
*/
#define TOP_CNTRL_1V_LDO_CTRL        (TOP_CNTRL_BASE + 0x30)
#define TOP_CNTRL_1V_LDO_CTRL_EN     (TOP_CNTRL_BASE + 0x34)

/*
#####################################################################
# Internal Memory utilization (by internal bootrom)
#####################################################################
*/

#define BTRM_INT_MEM_UTIL_SIZE                  (256 * 1024)  //0x40000 
#define BTRM_INT_MEM_BEGIN_ADDR                 0x90500000
#define BTRM_INT_MEM_END_ADDR                   (BTRM_INT_MEM_BEGIN_ADDR + BTRM_INT_MEM_UTIL_SIZE)
#define BTRM_INT_MEM_COMP_CFE_RAM_ADDR          (BTRM_INT_MEM_BEGIN_ADDR + 0x00000)
#define BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR     (BTRM_INT_MEM_BEGIN_ADDR + 0x01000)

#define BTRM_INT_MEM_STACK_PTR_ADDR             (BTRM_INT_MEM_BEGIN_ADDR + 0x1e000)
#define BTRM_INT_MEM_SBI_LINK_ADDR              (BTRM_INT_MEM_BEGIN_ADDR + 0x20000)

/* 6848 actually has 384K LMEM, as long as BTRM_INT_MEM_END_ADD is 32KB less than 384KB, we are good here */
#define BTRM_INT_MEM_SECURE_BLOCK               BTRM_INT_MEM_END_ADDR
#define BTRM_INT_MEM_SHREDDER_PROG_ADDR         BTRM_INT_MEM_SECURE_BLOCK 
#define BTRM_INT_MEM_TP1_PROG_ADDR              (BTRM_INT_MEM_SECURE_BLOCK + 0x1f00)
#define BTRM_INT_MEM_CREDENTIALS_ADDR           (BTRM_INT_MEM_SECURE_BLOCK + 0x2000)
#define BTRM_INT_MEM_32K_BLOCK_END_ADDR         (BTRM_INT_MEM_SECURE_BLOCK + 0x8000)


#ifdef __cplusplus
}
#endif

#endif
