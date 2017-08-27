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
/*   MODULE:  60333_common.h                                           */
/*   DATE:    04/24/13                                                 */
/*   PURPOSE: Define addresses of major hardware components of         */
/*            BCM60333                                                 */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM60333_MAP_COMMON_H
#define __BCM60333_MAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__KERNEL__) || defined (_CFE_) || defined(FAP_4KE)
/* Access to registers from kernelspace */
#define REG_BASE                 0xb2000000
#else
/* Access to registers from userspace, see bcm_mmap.h for api */
#define REG_BASE                 (bcm_mmap_info.mmap_addr)
#define BCM_MMAP_INFO_BASE       0x12000000
#define BCM_MMAP_INFO_SIZE       0x01000000
#endif

#define SDRAM_CTRL_BASE             (REG_BASE + 0x00000000)
#define ENET_CORE0_BASE             (REG_BASE + 0x00400000)
#define ENET_CORE1_BASE             (REG_BASE + 0x00600000)
#define PCIE_BASE                   (REG_BASE + 0x00a00000)
#define PERF_BASE                   (REG_BASE + 0x01e00000)  /* chip control */
#define TIMR_BASE                   (REG_BASE + 0x01e00100)  /* timer registers */
#define GPIO_BASE                   (REG_BASE + 0x01e00200)  /* gpio registers */
#define MISC_BASE                   (REG_BASE + 0x01e00300)  /* Miscellaneous Registers */
#define STRAP_BASE                  (REG_BASE + 0x01e00304)  /* Miscellaneous Registers */
#define BSTI_BASE                   (REG_BASE + 0x01e003f8)  /* Serial Interface Control registers */
#define OTP_BASE                    (REG_BASE + 0x01e00400)
#define UART_BASE                   (REG_BASE + 0x01e00500)  /* uart registers */
#define HSSPIM_BASE                 (REG_BASE + 0x01e01000)  /* High-Speed SPI registers */

#define AVS_MONITOR                 (REG_BASE + 0x01e04000)

#define ETH0_DMA_BASE               (REG_BASE + 0x00410000)
#define ETH1_DMA_BASE               (REG_BASE + 0x00610000)
#define BRIDGE_DMA_BASE             (REG_BASE + 0x01020000)

/*
#####################################################################
# Miscellaneous Registers
#####################################################################
*/
#define STRAP_OVERRIDE_BUS                             (0x0)
#define STRAP_OVERRIDE_BUS_SDRC_MASK_SHIFT              9
#define STRAP_OVERRIDE_BUS_SDRC_MASK                   (0x3 << STRAP_OVERRIDE_BUS_SDRC_MASK_SHIFT)
#define STRAP_OVERRIDE_BUS_SDR_DDR1_SELECT_MASK_SHIFT   12
#define STRAP_OVERRIDE_BUS_SDR_DDR1_SELECT_MASK        (1 << STRAP_OVERRIDE_BUS_SDR_DDR1_SELECT_MASK_SHIFT)

/*
#####################################################################
# Memory Control Registers
#####################################################################
*/
#define SDR_CFG_SDR_CFG      0x00
#define SDR_CFG_PRI_CFG      0x14
#define SDR_CFG_PID_SELECT0  0x18
#define SDR_CFG_PID_SELECT1  0x1C
#define SDR_CFG_AUTO_REFRESH 0x24
#define SDR_CFG_TIMING_PARAM 0x28

#define SDR_PRECHARGE_CMD    0x80
#define SDR_AUTOREFRESH_CMD  0x90
#define SDR_LMR_CMD          0xA0
#define SDR_LMRX_CMD         0xA4

/*
#####################################################################
# UART Control Registers
#####################################################################
*/
#if defined(__MIPSEL)
#define UART0CONTROL     0x02
#define UART0CONFIG      0x01
#define UART0RXTIMEOUT   0x00
#define UART0BAUD        0x04
#define UART0FIFOCFG     0x09
#define UART0INTMASK     0x12
#define UART0INTSTAT     0x10
#define UART0DATA        0x14
#else
#define UART0CONTROL     0x01
#define UART0CONFIG      0x02
#define UART0RXTIMEOUT   0x03
#define UART0BAUD        0x04
#define UART0FIFOCFG     0x0a
#define UART0INTMASK     0x10
#define UART0INTSTAT     0x12
#define UART0DATA        0x17
#endif

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
# GPIO Control Registers
#####################################################################
*/
#define GPIO_DIR                    (GPIO_BASE)
#define GPIO_DATA                   (GPIO_BASE + 0x4)
#define GPIO_FUNC_MODE_CTRL         (GPIO_BASE + 0x8)
#define GPIO_MUX_CTRL_0             (GPIO_BASE + 0x10)
#define GPIO_MUX_CTRL_1             (GPIO_BASE + 0x14)
#define GPIO_MUX_CTRL_2             (GPIO_BASE + 0x18)
#define GPIO_MUX_CTRL_3             (GPIO_BASE + 0x1c)
#define GPIO_MUX_CTRL_4             (GPIO_BASE + 0x20)
#define GPIO_MUX_CTRL_5             (GPIO_BASE + 0x24)
#define GPIO_MUX_CTRL_6             (GPIO_BASE + 0x28)
#define GPIO_MUX_CTRL_7             (GPIO_BASE + 0x2c)

/*
#####################################################################
# BTSI control registers
#####################################################################
*/
#define BSTI_SER_CTRL                                0
#define BSTI_SER_STATUS                              4
#define BSTI_SER_CTRL_START_SHIFT                    28
#define BSTI_SER_CTRL_START_MASK                     (1 << BSTI_SER_CTRL_START_SHIFT)
#define BSTI_SER_CTRL_CMD_SHIFT                      26
#define BSTI_SER_CTRL_CMD_MASK                       (0x3 << BSTI_SER_CTRL_CMD_SHIFT)
#define BSTI_SER_CTRL_ADDR_SHIFT                     16
#define BSTI_SER_CTRL_ADDR_MASK                      (0x3ff << BSTI_SER_CTRL_ADDR_SHIFT)
#define BSTI_SER_CTRL_WR_DATA_SHIFT                  0
#define BSTI_SER_CTRL_WR_DATA_MASK                   (0xffff)
#define BSTI_SER_STATUS_RD_DATA_MASK                 (0xffff)

/*
#####################################################################
# Timer registers
#####################################################################
*/
#define TIMER_TIMRIRQSTAT          (TIMR_BASE + 0x0)  /* Timer Interrupt Status Register */
#define TIMER_TIMERCTL0            (TIMR_BASE + 0x4)  /* Timer Control Word 0 Register */
#define TIMER_TIMERCTL1            (TIMR_BASE + 0x8)  /* Timer Control Word 1 Register */
#define TIMER_TIMERCTL2            (TIMR_BASE + 0xc)  /* Timer Control Word 2 Register */
#define TIMER_TIMERCNT0            (TIMR_BASE + 0x10) /* Timer Count 0 Register */
#define TIMER_TIMERCNT1            (TIMR_BASE + 0x14) /* Timer Count 1 Register */
#define TIMER_TIMERCNT2            (TIMR_BASE + 0x18) /* Timer Count 2 Register */
#define TIMER_WATCHDOG_DEF_COUNT   (TIMR_BASE + 0x1c) /* Watchdog Default Count Value Register */
#define TIMER_WATCHDOG_CTL         (TIMR_BASE + 0x20) /* Watchdog Control Register */
#define TIMER_WATCHDOG_RESET_COUNT (TIMR_BASE + 0x24) /* Watchdog Reset Length Register */
#define TIMER_EN_SW_PLL            (TIMR_BASE + 0x28) /* Software Watchdog Reset Register */

/*
#####################################################################
# BSTI operation codes
#####################################################################
*/
#define BSTI_START_OP                  0x01
#define BSTI_READ_OP                   0x02
#define BSTI_WRITE_OP                  0x01

/*
#####################################################################
# AON serial registers
#####################################################################
*/
#define AON_REGISTERS_LED_EN           0x340
#define AON_REGISTERS_LED0_ON          0x342
#define AON_REGISTERS_LED1_ON          0x344
#define AON_REGISTERS_LED0_OFF         0x346
#define AON_REGISTERS_LED1_OFF         0x348
#define AON_WAKEUP_READ_REG            0x306

#define AON_STBY_WOKEN_STAT_MASK       0xF


#ifdef __cplusplus
}
#endif

#endif
