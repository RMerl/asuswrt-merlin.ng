/*
 Copyright 2011 Broadcom Corportaion

<:label-BRCM:2012:proprietary:standard

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
/*   MODULE:  63268_common.h                                           */
/*   DATE:    11/09/10                                                 */
/*   PURPOSE: Register definition used by assembly for BCM63268        */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM63268_MAP_COMMON_H
#define __BCM63268_MAP_COMMON_H

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
# OTP Control / Status Registers
#####################################################################
*/

#define OTP_OTP_CONFIG                          0x00
#define OTP_OTP_CONTROL                         0x04
#define OTP_OTP_STATUS                          0x08
#define OTP_OTP_ADDR                            0x0c
#define OTP_OTP_WRITE                           0x10

#define OTP_OTP_SECURE_BOOT_CFG                 0x20
#define OTP_OTP_SECURE_BOOT_CFG_UNLOCK_JTAG     (1 << 5)
#define OTP_OTP_SECURE_BOOT_CFG_BTRM_LOCKDOWN   (1 << 0)

#define OTP_OTP_BRCM_SECURE_0                   0x50
#define OTP_OTP_BRCM_SECURE_0_MEK_MIV_SEL_SHIFT 4
#define OTP_OTP_BRCM_SECURE_0_MEK_MIV_SEL_MASK  (7 << OTP_OTP_BRCM_SECURE_0_MEK_MIV_SEL_SHIFT)
#define OTP_OTP_BRCM_SECURE_0_DBG_ENABLE        (1 << 3)

#define OTP_OTP_CUST_CNTRL_0                    0x80
#define OTP_OTP_CUST_CNTRL_0_SER_DISABLE        (1 << 20)
#define OTP_OTP_CUST_CNTRL_0_OP_INUSE           (1 << 19)
#define OTP_OTP_CUST_CNTRL_0_NAND_DUP_SHIFT     17
#define OTP_OTP_CUST_CNTRL_0_NAND_DUP_MASK      (3 << OTP_OTP_CUST_CNTRL_0_NAND_DUP_SHIFT)
#define OTP_OTP_CUST_CNTRL_0_SEC_BT_ENABLE      (1 << 1)

#define OTP_OTP_CUST_CNTRL_1                    0x84
#define OTP_OTP_CUST_CNTRL_1_OP_MRKT_ID_MASK    0x0000ffff

#define OTP_SEC_BT_ENABLE_OTP_ADDR              0x101   /* 257 */
#define OTP_SEC_BT_OP_INUSE_OTP_ADDR            0x113   /* 275 */
#define OTP_SEC_BT_MID_OTP_ADDR                 0x130   /* 304 */
#define OTP_SEC_BT_OID_OTP_ADDR                 0x120   /* 288 */

#define OTP_SEC_BT_FACTORY_MFG_MRKT_ID_VAL	0x1234
#define OTP_SEC_BT_FACTORY_OP_MRKT_ID_VAL       0x5678

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

#define MISC_STRAP_BUS                          0x14
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT      21
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK       (0xF<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
#define MISC_STRAP_BUS_DDR2_DDR3_N_SELECT       (1<<6)
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT           11
#define MISC_STRAP_BUS_BOOT_SEL_MASK            (0x1<<MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SERIAL              0x01
#define MISC_STRAP_BUS_BOOT_NAND                0x00

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
# Memory Control Registers
#####################################################################
*/
#define MEMC_CTL_CNFG                           0x000 
#define MEMC_CTL_CSST                           0x004 
#define MEMC_CTL_CSEND                          0x008 
#define MEMC_CTL_ROW00_0                        0x010 
#define MEMC_CTL_ROW00_1                        0x014 
#define MEMC_CTL_ROW01_0                        0x018 
#define MEMC_CTL_ROW01_1                        0x01c 
#define MEMC_CTL_ROW20_0                        0x030 
#define MEMC_CTL_ROW20_1                        0x034 
#define MEMC_CTL_ROW21_0                        0x038 
#define MEMC_CTL_ROW21_1                        0x03c 
#define MEMC_CTL_COL00_0                        0x050 
#define MEMC_CTL_COL00_1                        0x054 
#define MEMC_CTL_COL01_0                        0x058 
#define MEMC_CTL_COL01_1                        0x05c 
#define MEMC_CTL_COL20_0                        0x070 
#define MEMC_CTL_COL20_1                        0x074 
#define MEMC_CTL_COL21_0                        0x078 
#define MEMC_CTL_COL21_1                        0x07c 
#define MEMC_CTL_BNK10                          0x090 
#define MEMC_CTL_BNK32                          0x094 
#define MEMC_CTL_DCMD                           0x100 
#define MEMC_CTL_DMODE_0                        0x104 
#define MEMC_CTL_DMODE_2                        0x108 
#define MEMC_CTL_CLKS                           0x10c 
#define MEMC_CTL_ODT                            0x110 
#define MEMC_CTL_TIM1_0                         0x114 
#define MEMC_CTL_TIM1_1                         0x118 
#define MEMC_CTL_TIM2                           0x11c 
#define MEMC_CTL_CTL_CRC                        0x120 
#define MEMC_CTL_DOUT_CRC                       0x124 
#define MEMC_CTL_DIN_CRC                        0x128 
#define MEMC_CTL_DRAM_CFG                       0x134 
#define MEMC_CTL_STAT                           0x138 

#define PHY_CONTROL_REGS_REVISION               0x200 
#define PHY_CONTROL_REGS_CLK_PM_CTRL            0x204 
#define PHY_CONTROL_REGS_PLL_STATUS             0x210 
#define PHY_CONTROL_REGS_PLL_CONFIG             0x214 
#define PHY_CONTROL_REGS_PLL_PRE_DIVIDER        0x218 
#define PHY_CONTROL_REGS_PLL_DIVIDER            0x21c 
#define PHY_CONTROL_REGS_PLL_CONTROL1           0x220 
#define PHY_CONTROL_REGS_PLL_CONTROL2           0x224 
#define PHY_CONTROL_REGS_PLL_SS_EN              0x228 
#define PHY_CONTROL_REGS_PLL_SS_CFG             0x22c 
#define PHY_CONTROL_REGS_STATIC_VDL_OVERRIDE    0x230 
#define PHY_CONTROL_REGS_DYNAMIC_VDL_OVERRIDE   0x234 
#define PHY_CONTROL_REGS_IDLE_PAD_CONTROL       0x238 
#define PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL        0x23c 
#define PHY_CONTROL_REGS_DRIVE_PAD_CTL          0x240 

#define PHY_BYTE_LANE_0_REVISION                0x300 
#define PHY_BYTE_LANE_0_VDL_CALIBRATE           0x304 
#define PHY_BYTE_LANE_0_VDL_STATUS              0x308 
    #define PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX  31
    #define PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_STRT 8
    #define PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MASK (0x1f << PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_STRT)
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_0          0x310 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_1          0x314 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_2          0x318 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_3          0x31c 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_4          0x320 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_5          0x324 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_6          0x328 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_7          0x32c 
#define PHY_BYTE_LANE_0_READ_CONTROL            0x330 
#define PHY_BYTE_LANE_0_READ_FIFO_STATUS        0x334 
#define PHY_BYTE_LANE_0_READ_FIFO_CLEAR         0x338 
#define PHY_BYTE_LANE_0_IDLE_PAD_CONTROL        0x33c 
#define PHY_BYTE_LANE_0_DRIVE_PAD_CTL           0x340 
#define PHY_BYTE_LANE_0_CLOCK_PAD_DISABLE       0x344 
#define PHY_BYTE_LANE_0_WR_PREAMBLE_MODE        0x348 

#define PHY_BYTE_LANE_1_REVISION                0x400 
#define PHY_BYTE_LANE_1_VDL_CALIBRATE           0x404 
#define PHY_BYTE_LANE_1_VDL_STATUS              0x408 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_0          0x410 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_1          0x414 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_2          0x418 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_3          0x41c 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_4          0x420 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_5          0x424 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_6          0x428 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_7          0x42c 
#define PHY_BYTE_LANE_1_READ_CONTROL            0x430 
#define PHY_BYTE_LANE_1_READ_FIFO_STATUS        0x434 
#define PHY_BYTE_LANE_1_READ_FIFO_CLEAR         0x438 
#define PHY_BYTE_LANE_1_IDLE_PAD_CONTROL        0x43c 
#define PHY_BYTE_LANE_1_DRIVE_PAD_CTL           0x440 
#define PHY_BYTE_LANE_1_CLOCK_PAD_DISABLE       0x444 
#define PHY_BYTE_LANE_1_WR_PREAMBLE_MODE        0x448 

#define PHY_BYTE_LANE_2_REVISION                0x500 
#define PHY_BYTE_LANE_2_VDL_CALIBRATE           0x504 
#define PHY_BYTE_LANE_2_VDL_STATUS              0x508 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_0          0x510 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_1          0x514 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_2          0x518 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_3          0x51c 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_4          0x520 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_5          0x524 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_6          0x528 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_7          0x52c 
#define PHY_BYTE_LANE_2_READ_CONTROL            0x530 
#define PHY_BYTE_LANE_2_READ_FIFO_STATUS        0x534 
#define PHY_BYTE_LANE_2_READ_FIFO_CLEAR         0x538 
#define PHY_BYTE_LANE_2_IDLE_PAD_CONTROL        0x53c 
#define PHY_BYTE_LANE_2_DRIVE_PAD_CTL           0x540 
#define PHY_BYTE_LANE_2_CLOCK_PAD_DISABLE       0x544 
#define PHY_BYTE_LANE_2_WR_PREAMBLE_MODE        0x548 

#define PHY_BYTE_LANE_3_REVISION                0x600 
#define PHY_BYTE_LANE_3_VDL_CALIBRATE           0x604 
#define PHY_BYTE_LANE_3_VDL_STATUS              0x608 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_0          0x610 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_1          0x614 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_2          0x618 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_3          0x61c 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_4          0x620 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_5          0x624 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_6          0x628 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_7          0x62c 
#define PHY_BYTE_LANE_3_READ_CONTROL            0x630 
#define PHY_BYTE_LANE_3_READ_FIFO_STATUS        0x634 
#define PHY_BYTE_LANE_3_READ_FIFO_CLEAR         0x638 
#define PHY_BYTE_LANE_3_IDLE_PAD_CONTROL        0x63c 
#define PHY_BYTE_LANE_3_DRIVE_PAD_CTL           0x640 
#define PHY_BYTE_LANE_3_CLOCK_PAD_DISABLE       0x644 
#define PHY_BYTE_LANE_3_WR_PREAMBLE_MODE        0x648 

#define MEMC_CTL_GCFG                           0x800 
    #define MEMC_CTL_GCFG_MEMINITDONE                   (1 << 8)
#define MEMC_CTL_VERS                           0x804 
#define MEMC_CTL_ARB                            0x80c 
#define MEMC_CTL_PI_GCF                         0x810 
#define MEMC_CTL_PI_UBUS_CTL                    0x814 
#define MEMC_CTL_PI_MIPS_CTL                    0x818 
#define MEMC_CTL_PI_DSL_MIPS_CTL                0x81c 
#define MEMC_CTL_PI_DSL_PHY_CTL                 0x820 
#define MEMC_CTL_PI_UBUS_ST                     0x824 
#define MEMC_CTL_PI_MIPS_ST                     0x828 
#define MEMC_CTL_PI_DSL_MIPS_ST                 0x82c 
#define MEMC_CTL_PI_DSL_PHY_ST                  0x830 
#define MEMC_CTL_PI_UBUS_SMPL                   0x834
    #define MEMC_CTL_PI_UBUS_SMPL_SAMPLING_PERIOD_SHIFT 28
#define MEMC_CTL_TESTMODE                       0x838 
#define MEMC_CTL_TEST_CFG1                      0x83c 
#define MEMC_CTL_TEST_PAT                       0x840 
#define MEMC_CTL_TEST_COUNT                     0x844 
#define MEMC_CTL_TEST_CURR_COUNT                0x848 
#define MEMC_CTL_TEST_ADDR_UPDT                 0x84c 
#define MEMC_CTL_TEST_ADDR                      0x850 
#define MEMC_CTL_TEST_DATA0_0                   0x854 
#define MEMC_CTL_TEST_DATA0_1                   0x858 
#define MEMC_CTL_TEST_DATA0_2                   0x85c 
#define MEMC_CTL_TEST_DATA0_3                   0x860 
#define MEMC_CTL_TEST_DATA1_0                   0x864 
#define MEMC_CTL_TEST_DATA1_1                   0x868 
#define MEMC_CTL_TEST_DATA1_2                   0x86c 
#define MEMC_CTL_TEST_DATA1_3                   0x870 
#define MEMC_CTL_REPLY_DATA0                    0x874 
#define MEMC_CTL_REPLY_DATA1                    0x878 
#define MEMC_CTL_REPLY_DATA2                    0x87c 
#define MEMC_CTL_REPLY_DATA3                    0x880 
#define MEMC_CTL_REPLY_STAT                     0x884 
#define MEMC_CTL_LBIST_CFG                      0x888 
#define MEMC_CTL_LBIST_SEED                     0x88c 
#define MEMC_CTL_PI_MIPS_SMPL                   0x890 
    #define MEMC_CTL_PI_MIPS_SMPL_SAMPLING_PERIOD_SHIFT 28


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
# Internal Memory utilization (by internal bootrom)
#####################################################################
*/

#define BTRM_INT_MEM_UTIL_SIZE                  (440 * 1024)
#define BTRM_INT_MEM_BEGIN_ADDR                 0x90780000
#define BTRM_INT_MEM_END_ADDR                   (BTRM_INT_MEM_BEGIN_ADDR + BTRM_INT_MEM_UTIL_SIZE)
#define BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR     (BTRM_INT_MEM_BEGIN_ADDR + 0x20000)
#define BTRM_INT_MEM_SHREDDER_PROG_ADDR         (BTRM_INT_MEM_BEGIN_ADDR + 0x3e000)
#define BTRM_INT_MEM_TP1_PROG_ADDR              (BTRM_INT_MEM_BEGIN_ADDR + 0x3ef00)
#define BTRM_INT_MEM_CREDENTIALS_ADDR           (BTRM_INT_MEM_BEGIN_ADDR + 0x3f000)
#define BTRM_INT_MEM_COMP_CFE_RAM_ADDR          (BTRM_INT_MEM_BEGIN_ADDR + 0x50000)
#define BTRM_INT_MEM_CFE_ROM_END_ADDR		(BTRM_INT_MEM_BEGIN_ADDR + 0x60000)

#ifdef __cplusplus
}
#endif

#endif
