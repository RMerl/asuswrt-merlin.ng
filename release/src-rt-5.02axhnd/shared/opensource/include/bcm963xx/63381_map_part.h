/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
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

#ifndef __BCM63381_MAP_PART_H
#define __BCM63381_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#if !defined(REG_BASE)
#define REG_BASE                    0xb0000000
#endif

#define CHIP_FAMILY_ID_HEX	    0x63381

#define PERF_BASE                   (REG_BASE + 0x00000000)  /* chip control */
#define TIMR_BASE                   (REG_BASE + 0x000000c0)  /* timer registers */
#define GPIO_BASE                   (REG_BASE + 0x00000180)  /* gpio registers */
#define MISC_BASE                   (REG_BASE + 0x00000200)  /* Miscellaneous Registers */
#define UART_BASE                   (REG_BASE + 0x00000280)  /* uart registers */
#define UART1_BASE                  (REG_BASE + 0x000002a0)  /* uart1 registers */
#define LED_BASE                    (REG_BASE + 0x00000300)  /* LED control registers */
#define HSSPIM_BASE                 (REG_BASE + 0x00001000)  /* High-Speed SPI registers */
#define NAND_REG_BASE               (REG_BASE + 0x00002000)  /* nand control register */
#define NAND_CACHE_BASE             (REG_BASE + 0x00002400)  /* nand cache register */
#define NAND_INTR_BASE              (REG_BASE + 0x00000100)  /* nand interrupt register */
#define JTAG_OTP_BASE               (REG_BASE + 0x00004000)  /* OTP control registers */
#define MISC_REG_BASE               (REG_BASE + 0x00005080)  /* Misc control register */
#define PAD_CTL_BASE                (REG_BASE + 0x00005100)  /* Pad control register */
#define USB_CTL_BASE                (REG_BASE + 0x00009000)  /* USB 2.0 device control */
#define USB_DMA_BASE                (REG_BASE + 0x00009800)  /* USB 2.0 device DMA control */
#define MEMC_BASE                   (REG_BASE + 0x0000a000)  /* SDRAM Control */
#define USBH_CFG_BASE               (REG_BASE + 0x0000c200)
#ifdef __KERNEL__
#define USB_EHCI_BASE               0x1000c300  /* USB host registers */
#define USB_OHCI_BASE               0x1000c400  /* USB host registers */
#endif
#define SAR_BASE                    (REG_BASE + 0x00014000)
#define SAR_DMA_BASE                (REG_BASE + 0x00017800)  /* XTM SAR DMA control */
#define PCIE_BASE                   (REG_BASE + 0x00020000)
#define USB30H_CFG_BASE             (REG_BASE + 0x00034200)  /* USB3 host control register */
#ifdef __KERNEL__
#define USB_XHCI_BASE               0x10035000  /* USB3 host registers */
#endif
#define SWITCH_BASE                 (REG_BASE + 0x00080000)  /* Robo Switch control */
#define SWITCH_DMA_BASE             (REG_BASE + 0x00088000)  /* Robo Switch DMA control */
#define PMC_BASE                    (REG_BASE + 0x00200000)  /* PMC register */
#define PROC_MON_BASE               (REG_BASE + 0x00280000)  /* Process Monitor register */
#define DSLPHY_BASE                 (REG_BASE + 0x00400000)  /* VDSL PHY register */
#define DSLPHY_AFE_BASE             (REG_BASE + 0x00457200)  /* VDSL PHY AFE register */
#define DSLLMEM_BASE                (REG_BASE + 0x00500000)  /* VDSL PHY Memoryr */

#ifndef __ASSEMBLER__

/*
** Peripheral Controller
*/

#define IRQ_BITS 64
typedef struct  {
    uint64         IrqMask;
    uint64         ExtIrqMask;
    uint64         reserved[2];
    } IrqControl_t;

typedef struct PerfControl {
     uint32        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define CHIP_VAR_SHIFT   8
#define CHIP_VAR_MASK    (0xf << CHIP_VAR_SHIFT)
#define REV_ID_MASK     0xff

    uint32        diagControl;        /* (04) word 1 */
    uint32        ExtIrqCfg;          /* (08) word 2*/
#define EI_CLEAR_SHFT   0
#define EI_SENSE_SHFT   8
#define EI_INSENS_SHFT  16
#define EI_LEVEL_SHFT   24

    uint32        ExtIrqSts;          /* (0c) word 3 */
#define EI_STATUS_SHFT  0
#define EI_MASK_SHFT    16

    uint64         IrqStatus;         /* (10) word 4 */
    uint64         ExtIrqStatus;      /* (18) word 6 */
    IrqControl_t   IrqControl[4];     /* (20) */
} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)

/*
** Timer
*/
typedef struct Timer {
    uint32        TimerCtl0;
    uint32        TimerCtl1;
    uint32        TimerCtl2;
    uint32        TimerCtl3;
#define TIMERENABLE     0x80000000
#define RSTCNTCLR       0x40000000
    uint32        TimerCnt0;
    uint32        TimerCnt1;
    uint32        TimerCnt2;
    uint32        TimerCnt3;
#define TIMER_COUNT_MASK    0x3FFFFFFF
    uint32        TimerMask;
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
#define TIMER3EN        0x08
    uint32        TimerInts;
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define TIMER3          0x08
#define WATCHDOG        0x10

    uint32        WatchDogDefCount;

    /* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

    uint32        SoftRst;
#define SOFT_RESET              0x00000001      // 0    
    uint32        ResetStatus;
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000    
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)

/*
** UART
*/
typedef struct UartChannel {
    byte          unused0;
    byte          control;
#define BRGEN           0x80    /* Control register bit defs */
#define TXEN            0x40
#define RXEN            0x20
#define LOOPBK          0x10
#define TXPARITYEN      0x08
#define TXPARITYEVEN    0x04
#define RXPARITYEN      0x02
#define RXPARITYEVEN    0x01

    byte          config;
#define XMITBREAK       0x40
#define BITS5SYM        0x00
#define BITS6SYM        0x10
#define BITS7SYM        0x20
#define BITS8SYM        0x30
#define ONESTOP         0x07
#define TWOSTOP         0x0f
    /* 4-LSBS represent STOP bits/char
     * in 1/8 bit-time intervals.  Zero
     * represents 1/8 stop bit interval.
     * Fifteen represents 2 stop bits.
     */
    byte          fifoctl;
#define RSTTXFIFOS      0x80
#define RSTRXFIFOS      0x40
    /* 5-bit TimeoutCnt is in low bits of this register.
     *  This count represents the number of characters
     *  idle times before setting receive Irq when below threshold
     */
    uint32        baudword;
    /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
     */

    byte          txf_levl;       /* Read-only fifo depth */
    byte          rxf_levl;       /* Read-only fifo depth */
    byte          fifocfg;        /* Upper 4-bits are TxThresh, Lower are
                                   *      RxThreshold.  Irq can be asserted
                                   *      when rx fifo> thresh, txfifo<thresh
                                   */
    byte          prog_out;       /* Set value of DTR (Bit0), RTS (Bit1)
                                   *  if these bits are also enabled to GPIO_o
                                   */
#define DTREN   0x01
#define RTSEN   0x02

    byte          unused1;
    byte          DeltaIPEdgeNoSense;     /* Low 4-bits, set corr bit to 1 to
                                           * detect irq on rising AND falling
                                           * edges for corresponding GPIO_i
                                           * if enabled (edge insensitive)
                                           */
    byte          DeltaIPConfig_Mask;     /* Upper 4 bits: 1 for posedge sense
                                           *      0 for negedge sense if
                                           *      not configured for edge
                                           *      insensitive (see above)
                                           * Lower 4 bits: Mask to enable change
                                           *  detection IRQ for corresponding
                                           *  GPIO_i
                                           */
    byte          DeltaIP_SyncIP;         /* Upper 4 bits show which bits
                                           *  have changed (may set IRQ).
                                           *  read automatically clears bit
                                           * Lower 4 bits are actual status
                                           */

    uint16        intMask;                /* Same Bit defs for Mask and status */
    uint16        intStatus;
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

    uint16        unused2;
    uint16        Data;                   /* Write to TX, Read from RX */
                                          /* bits 11:8 are BRK,PAR,FRM errors */

    uint32        unused3;
    uint32        unused4;
} Uart;

#define UART ((volatile Uart * const) UART_BASE)

/*
 * Gpio Controller
 */
typedef struct GpioControl {
        uint32 GPIODir[5];      /* 0x00-0x10 */
        uint32 GPIOio[5];       /* 0x14-0x24 */
        uint32 SpiSlaveCfg;     /* 0x28 */
        uint32 unused;          /* 0x2c */
        uint32 TestControl;             /* 0x30 */
        uint32 TestPortBlockEnMSB;      /* 0x34 */
        uint32 TestPortBlockEnLSB;      /* 0x38 */
        uint32 TestPortBlockDataMSB;    /* 0x3c */
        uint32 TestPortBlockDataLSB;    /* 0x40 */
#define PINMUX_DATA_SHIFT       12
#define PINMUX_0                0
#define PINMUX_1                1
#define PINMUX_2                2
#define PINMUX_3                3
#define PINMUX_4                4
#define PINMUX_5                5
#define PINMUX_6                6
#define PINMUX_MSPI             PINMUX_3
#define PINMUX_MSPI_SS          PINMUX_3
#define PINMUX_PCM              PINMUX_1
#define PINMUX_GPIO             PINMUX_5
        uint32 TestPortCmd;             /* 0x44 */
#define LOAD_MUX_REG_CMD        0x21
        uint32 GPIOBaseMode;            /* 0x48 */
#define GPIO_BASE_USB_LED_OVERRIDE      (1<<11)
#define GPIO_BASE_VDSL_LED_OVERRIDE     (1<<10)
        uint32 DiagReadBack;            /* 0x4c */
        uint32 DiagReadBackHi;          /* 0x50 */
        uint32 GeneralPurpose;          /* 0x54 */
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/* Number to mask conversion macro used for GPIODir and GPIOio */
#define GPIO_NUM_MAX                   64
#define GPIO_NUM_TO_ARRAY_IDX(X)       ((((X) & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? ((((X) & BP_GPIO_NUM_MASK) >> 5) & 0x07) : (0))
#define GPIO_NUM_TO_MASK(X)            ((((X) & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? (1 << (((X) & BP_GPIO_NUM_MASK) & 0x1f)) : (0))
#define GPIO_NUM_TO_ARRAY_SHIFT(X)     (((X) & BP_GPIO_NUM_MASK) & 0x1f)

/*
** Misc Register Set Definitions.
*/

typedef struct Misc {
#define MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK     (0x3)
    uint32  miscPCIECtrl;                       /* 0x00 */
    uint32  miscStrapBus;                       /* 0x04 */
#define MISC_STRAP_BUS_SPI_NAND_DISABLE         (1<<24)
#define MISC_STRAP_BUS_RESET_DELAY_N_SHORT      (1<<23)
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT		18
#define MISC_STRAP_BUS_BOOT_SEL_MASK		(0x1f << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR		(0x18 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_1_24MHZ	(0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_2_54MHZ	(0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_2_81MHZ	(0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_4_81MHZ	(0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_EMMC		(0x1e << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_2K_PAGE	(0x00 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_4K_PAGE	(0x08 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_8K_PAGE	(0x10 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL_ECC_MASK	(0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_DISABLE	(0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_1_BIT	(0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_4_BIT	(0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_8_BIT	(0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_12_BIT	(0x4 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_24_BIT	(0x5 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_40_BIT	(0x6 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_60_BIT	(0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_DDR_N_SDRAM_SELECT       (1<<17)
#define MISC_STRAP_BUS_UBUS_FREQ_SHIFT          15
#define MISC_STRAP_BUS_UBUS_FREQ_MASK           (0x3<<MISC_STRAP_BUS_UBUS_FREQ_SHIFT)
#define MISC_STRAP_BUS_UBUS_FREQ_300MHZ         (0x3<<MISC_STRAP_BUS_UBUS_FREQ_SHIFT)
#define MISC_STRAP_BUS_UBUS_FREQ_200MHZ         (0x1<<MISC_STRAP_BUS_UBUS_FREQ_SHIFT)
#define MISC_STRAP_BUS_UBUS_FREQ_171MHZ         (0x2<<MISC_STRAP_BUS_UBUS_FREQ_SHIFT)
#define MISC_STRAP_BUS_PMC_ROM_BOOT             (1<<14)
#define MISC_STRAP_BUS_PMC_BOOT_AVS             (1<<13)
#define MISC_STRAP_BUS_HS_SPIM_CLK_SLOW_N_FAST  (1<<12)
#define MISC_STRAP_BUS_HS_SPIM_24B_N_32B_ADDR   (1<<11)
#define MISC_STRAP_BUS_BYPASS_XTAL_ENABLE       (1<<10)
#define MISC_STRAP_BUS_LS_SPI_SLAVE_DISABLE     (1<<9)
#define MISC_STRAP_BUS_ROBO_SW_RGMII_N_MII      (1<<8)
#define MISC_STRAP_BUS_MEMC_FREQ_SHIFT          6
#define MISC_STRAP_BUS_MEMC_FREQ_MASK           (0x3<<MISC_STRAP_BUS_MEMC_FREQ_SHIFT)
#define MISC_STRAP_BUS_MEMC_FREQ_400MHZ         (0x3<<MISC_STRAP_BUS_MEMC_FREQ_SHIFT)
#define MISC_STRAP_BUS_MEMC_FREQ_300MHZ         (0x1<<MISC_STRAP_BUS_MEMC_FREQ_SHIFT)
#define MISC_STRAP_BUS_MEMC_FREQ_266MHZ         (0x2<<MISC_STRAP_BUS_MEMC_FREQ_SHIFT)
#define MISC_STRAP_BUS_MEMC_FREQ_200MHZ         (0x0<<MISC_STRAP_BUS_MEMC_FREQ_SHIFT)
#define MISC_STRAP_BUS_XCORE_BIAS_SHIFT         2
#define MISC_STRAP_BUS_XCORE_BIAS_MASK          (0xf<<MISC_STRAP_BUS_XCORE_BIAS_SHIFT)
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT      0
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK       (0x3<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_600MHZ     (0x3<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_800MHZ     (0x1<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_480MHZ     (0x2<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_300MHZ     (0x0<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
    uint32  miscStrapOverride;                  /* 0x08 */
    uint32  miscAdsl_clock_sample;              /* 0x0C */
    uint32  miscRNGCtrl;                        /* 0x10 */
    uint32  miscMbox0_data;                     /* 0x14 */
    uint32  miscMbox1_data;                     /* 0x18 */
    uint32  miscMbox2_data;                     /* 0x1c */
    uint32  miscMbox3_data;                     /* 0x20 */
    uint32  miscMbox_ctrl;                      /* 0x24 */
    uint32  miscMIIPadCtrl;                     /* 0x28 */
    uint32  miscRGMII1PadCtrl;                  /* 0x2c */
    uint32  miscRGMII2PadCtrl;                  /* 0x30 */
    uint32  miscRGMII3PadCtrl;                  /* 0x34 */
    uint32  miscMIIPullCtrl;                    /* 0x38 */
    uint32  miscRGMII1PullCtrl;                 /* 0x3c */
    uint32  miscRGMII2PullCtrl;                 /* 0x40 */
    uint32  miscRGMII3PullCtrl;                 /* 0x44 */
    uint32  miscRbusBridgeCtrl;                 /* 0x48 */
    uint32  miscPerSpareReg[6];                 /* 0x4c - 0x260 */
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)

/*
** High-Speed SPI Controller
*/

#define __mask(end, start)      (((1 << ((end - start) + 1)) - 1) << start)
typedef struct HsSpiControl {

  uint32    hs_spiGlobalCtrl;   // 0x0000
#define HS_SPI_MOSI_IDLE        (1 << 18)
#define HS_SPI_CLK_POLARITY      (1 << 17)
#define HS_SPI_CLK_GATE_SSOFF       (1 << 16)
#define HS_SPI_PLL_CLK_CTRL     (8)
#define HS_SPI_PLL_CLK_CTRL_MASK    __mask(15, HS_SPI_PLL_CLK_CTRL)
#define HS_SPI_SS_POLARITY      (0)
#define HS_SPI_SS_POLARITY_MASK     __mask(7, HS_SPI_SS_POLARITY)

  uint32    hs_spiExtTrigCtrl;  // 0x0004
#define HS_SPI_TRIG_RAW_STATE   (24)
#define HS_SPI_TRIG_RAW_STATE_MASK  __mask(31, HS_SPI_TRIG_RAW_STATE)
#define HS_SPI_TRIG_LATCHED     (16)
#define HS_SPI_TRIG_LATCHED_MASK    __mask(23, HS_SPI_TRIG_LATCHED)
#define HS_SPI_TRIG_SENSE       (8)
#define HS_SPI_TRIG_SENSE_MASK      __mask(15, HS_SPI_TRIG_SENSE)
#define HS_SPI_TRIG_TYPE        (0)
#define HS_SPI_TRIG_TYPE_MASK       __mask(7, HS_SPI_TRIG_TYPE)
#define HS_SPI_TRIG_TYPE_EDGE       (0)
#define HS_SPI_TRIG_TYPE_LEVEL      (1)

  uint32    hs_spiIntStatus;    // 0x0008
#define HS_SPI_IRQ_PING1_USER       (28)
#define HS_SPI_IRQ_PING1_USER_MASK  __mask(31, HS_SPI_IRQ_PING1_USER)
#define HS_SPI_IRQ_PING0_USER       (24)
#define HS_SPI_IRQ_PING0_USER_MASK  __mask(27, HS_SPI_IRQ_PING0_USER)

#define HS_SPI_IRQ_PING1_CTRL_INV   (1 << 12)
#define HS_SPI_IRQ_PING1_POLL_TOUT  (1 << 11)
#define HS_SPI_IRQ_PING1_TX_UNDER   (1 << 10)
#define HS_SPI_IRQ_PING1_RX_OVER    (1 << 9)
#define HS_SPI_IRQ_PING1_CMD_DONE   (1 << 8)

#define HS_SPI_IRQ_PING0_CTRL_INV   (1 << 4)
#define HS_SPI_IRQ_PING0_POLL_TOUT  (1 << 3)
#define HS_SPI_IRQ_PING0_TX_UNDER   (1 << 2)
#define HS_SPI_IRQ_PING0_RX_OVER    (1 << 1)
#define HS_SPI_IRQ_PING0_CMD_DONE   (1 << 0)

  uint32    hs_spiIntStatusMasked;  // 0x000C
#define HS_SPI_IRQSM__PING1_USER    (28)
#define HS_SPI_IRQSM__PING1_USER_MASK   __mask(31, HS_SPI_IRQSM__PING1_USER)
#define HS_SPI_IRQSM__PING0_USER    (24)
#define HS_SPI_IRQSM__PING0_USER_MASK   __mask(27, HS_SPI_IRQSM__PING0_USER)

#define HS_SPI_IRQSM__PING1_CTRL_INV    (1 << 12)
#define HS_SPI_IRQSM__PING1_POLL_TOUT   (1 << 11)
#define HS_SPI_IRQSM__PING1_TX_UNDER    (1 << 10)
#define HS_SPI_IRQSM__PING1_RX_OVER (1 << 9)
#define HS_SPI_IRQSM__PING1_CMD_DONE    (1 << 8)

#define HS_SPI_IRQSM__PING0_CTRL_INV    (1 << 4)
#define HS_SPI_IRQSM__PING0_POLL_TOUT   (1 << 3)
#define HS_SPI_IRQSM__PING0_TX_UNDER    (1 << 2)
#define HS_SPI_IRQSM__PING0_RX_OVER     (1 << 1)
#define HS_SPI_IRQSM__PING0_CMD_DONE    (1 << 0)

  uint32    hs_spiIntMask;      // 0x0010
#define HS_SPI_IRQM_PING1_USER      (28)
#define HS_SPI_IRQM_PING1_USER_MASK __mask(31, HS_SPI_IRQM_PING1_USER)
#define HS_SPI_IRQM_PING0_USER      (24)
#define HS_SPI_IRQM_PING0_USER_MASK __mask(27, HS_SPI_IRQM_PING0_USER)

#define HS_SPI_IRQM_PING1_CTRL_INV  (1 << 12)
#define HS_SPI_IRQM_PING1_POLL_TOUT (1 << 11)
#define HS_SPI_IRQM_PING1_TX_UNDER  (1 << 10)
#define HS_SPI_IRQM_PING1_RX_OVER   (1 << 9)
#define HS_SPI_IRQM_PING1_CMD_DONE  (1 << 8)

#define HS_SPI_IRQM_PING0_CTRL_INV  (1 << 4)
#define HS_SPI_IRQM_PING0_POLL_TOUT (1 << 3)
#define HS_SPI_IRQM_PING0_TX_UNDER  (1 << 2)
#define HS_SPI_IRQM_PING0_RX_OVER   (1 << 1)
#define HS_SPI_IRQM_PING0_CMD_DONE  (1 << 0)

#define HS_SPI_INTR_CLEAR_ALL       (0xFF001F1F)

  uint32    hs_spiFlashCtrl;    // 0x0014
#define HS_SPI_FCTRL_MB_ENABLE      (23)
#define HS_SPI_FCTRL_SS_NUM         (20)
#define HS_SPI_FCTRL_SS_NUM_MASK    __mask(22, HS_SPI_FCTRL_SS_NUM)
#define HS_SPI_FCTRL_PROFILE_NUM    (16)
#define HS_SPI_FCTRL_PROFILE_NUM_MASK   __mask(18, HS_SPI_FCTRL_PROFILE_NUM)
#define HS_SPI_FCTRL_DUMMY_BYTES    (10)
#define HS_SPI_FCTRL_DUMMY_BYTES_MASK   __mask(11, HS_SPI_FCTRL_DUMMY_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES     (8)
#define HS_SPI_FCTRL_ADDR_BYTES_MASK    __mask(9, HS_SPI_FCTRL_ADDR_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES_2   (0)
#define HS_SPI_FCTRL_ADDR_BYTES_3   (1)
#define HS_SPI_FCTRL_ADDR_BYTES_4   (2)
#define HS_SPI_FCTRL_READ_OPCODE    (0)
#define HS_SPI_FCTRL_READ_OPCODE_MASK   __mask(7, HS_SPI_FCTRL_READ_OPCODE)

  uint32    hs_spiFlashAddrBase;    // 0x0018

  char      fill0[0x80 - 0x18];

  uint32    hs_spiPP_0_Cmd;     // 0x0080
#define HS_SPI_PP_SS_NUM        (12)
#define HS_SPI_PP_SS_NUM_MASK       __mask(14, HS_SPI_PP_SS_NUM)
#define HS_SPI_PP_PROFILE_NUM       (8)
#define HS_SPI_PP_PROFILE_NUM_MASK  __mask(10, HS_SPI_PP_PROFILE_NUM)

} HsSpiControl;

typedef struct HsSpiPingPong {

    uint32 command;
#define HS_SPI_SS_NUM (12)
#define ZSI_SPI_DEV_ID 7 // SS_N[7] connected to APM/PCM block for use by MSIF/ZDS interfaces
#define HS_SPI_PROFILE_NUM (8)
#define HS_SPI_TRIGGER_NUM (4)
#define HS_SPI_COMMAND_VALUE (0)
    #define HS_SPI_COMMAND_NOOP (0)
    #define HS_SPI_COMMAND_START_NOW (1)
    #define HS_SPI_COMMAND_START_TRIGGER (2)
    #define HS_SPI_COMMAND_HALT (3)
    #define HS_SPI_COMMAND_FLUSH (4)

    uint32 status;
#define HS_SPI_ERROR_BYTE_OFFSET (16)
#define HS_SPI_WAIT_FOR_TRIGGER (2)
#define HS_SPI_SOURCE_BUSY (1)
#define HS_SPI_SOURCE_GNT (0)

    uint32 fifo_status;
    uint32 control;

} HsSpiPingPong;

typedef struct HsSpiProfile {

    uint32 clk_ctrl;
#define HS_SPI_ACCUM_RST_ON_LOOP (15)
#define HS_SPI_SPI_CLK_2X_SEL (14)
#define HS_SPI_FREQ_CTRL_WORD (0)

    uint32 signal_ctrl;
#define	HS_SPI_ASYNC_INPUT_PATH (1 << 16)
#define	HS_SPI_LAUNCH_RISING    (1 << 13)
#define	HS_SPI_LATCH_RISING     (1 << 12)

    uint32 mode_ctrl;
#define HS_SPI_PREPENDBYTE_CNT (24)
#define HS_SPI_MODE_ONE_WIRE (20)
#define HS_SPI_MULTIDATA_WR_SIZE (18)
#define HS_SPI_MULTIDATA_RD_SIZE (16)
#define HS_SPI_MULTIDATA_WR_STRT (12)
#define HS_SPI_MULTIDATA_RD_STRT (8)
#define HS_SPI_FILLBYTE (0)

    uint32 polling_config;
    uint32 polling_and_mask;
    uint32 polling_compare;
    uint32 polling_timeout;
    uint32 reserved;

} HsSpiProfile;

#define HS_SPI_OP_CODE 13
    #define HS_SPI_OP_SLEEP (0)
    #define HS_SPI_OP_READ_WRITE (1)
    #define HS_SPI_OP_WRITE (2)
    #define HS_SPI_OP_READ (3)
    #define HS_SPI_OP_SETIRQ (4)

#define HS_SPI ((volatile HsSpiControl * const) HSSPIM_BASE)
#define HS_SPI_PINGPONG0 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0x80))
#define HS_SPI_PINGPONG1 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0xc0))
#define HS_SPI_PROFILES ((volatile HsSpiProfile * const) (HSSPIM_BASE+0x100))
#define HS_SPI_FIFO0 ((volatile uint8 * const) (HSSPIM_BASE+0x200))
#define HS_SPI_FIFO1 ((volatile uint8 * const) (HSSPIM_BASE+0x400))


/*
** NAND Controller Registers
*/
typedef struct NandCtrlRegs {
	uint32 NandRevision;	/* 0x00 */
	uint32 NandCmdStart;	/* 0x04 */
#define NCMD_MASK		0x0000001f
#define NCMD_BLOCK_ERASE_MULTI	0x15
#define NCMD_PROGRAM_PAGE_MULTI	0x13
#define NCMD_STS_READ_MULTI	0x12
#define NCMD_PAGE_READ_MULTI	0x11
#define NCMD_LOW_LEVEL_OP	0x10
#define NCMD_PARAM_CHG_COL	0x0f
#define NCMD_PARAM_READ		0x0e
#define NCMD_BLK_LOCK_STS	0x0d
#define NCMD_BLK_UNLOCK		0x0c
#define NCMD_BLK_LOCK_DOWN	0x0b
#define NCMD_BLK_LOCK		0x0a
#define NCMD_FLASH_RESET	0x09
#define NCMD_BLOCK_ERASE	0x08
#define NCMD_DEV_ID_READ	0x07
#define NCMD_COPY_BACK		0x06
#define NCMD_PROGRAM_SPARE	0x05
#define NCMD_PROGRAM_PAGE	0x04
#define NCMD_STS_READ		0x03
#define NCMD_SPARE_READ		0x02
#define NCMD_PAGE_READ		0x01

	uint32 NandCmdExtAddr;	/* 0x08 */
	uint32 NandCmdAddr;	/* 0x0c */
	uint32 NandCmdEndAddr;	/* 0x10 */
	uint32 NandIntfcStatus;	/* 0x14 */
#define NIS_CTLR_READY		(1 << 31)
#define NIS_FLASH_READY		(1 << 30)
#define NIS_CACHE_VALID		(1 << 29)
#define NIS_SPARE_VALID		(1 << 28)
#define NIS_FLASH_STS_MASK	0x000000ff
#define NIS_WRITE_PROTECT	0x00000080
#define NIS_DEV_READY		0x00000040
#define NIS_PGM_ERASE_ERROR	0x00000001


	uint32 NandNandBootConfig;	/* 0x18 */
#define NBC_CS_LOCK		(1 << 31)
#define NBC_AUTO_DEV_ID_CFG	(1 << 30)
#define NBC_WR_PROT_BLK0	(1 << 28)
#define NBC_EBI_CS7_USES_NAND	(1<<15)
#define NBC_EBI_CS6_USES_NAND	(1<<14)
#define NBC_EBI_CS5_USES_NAND	(1<<13)
#define NBC_EBI_CS4_USES_NAND	(1<<12)
#define NBC_EBI_CS3_USES_NAND	(1<<11)
#define NBC_EBI_CS2_USES_NAND	(1<<10)
#define NBC_EBI_CS1_USES_NAND	(1<< 9)
#define NBC_EBI_CS0_USES_NAND	(1<< 8)
#define NBC_EBC_CS7_SEL		(1<< 7)
#define NBC_EBC_CS6_SEL		(1<< 6)
#define NBC_EBC_CS5_SEL		(1<< 5)
#define NBC_EBC_CS4_SEL		(1<< 4)
#define NBC_EBC_CS3_SEL		(1<< 3)
#define NBC_EBC_CS2_SEL		(1<< 2)
#define NBC_EBC_CS1_SEL		(1<< 1)
#define NBC_EBC_CS0_SEL		(1<< 0)

	uint32 NandCsNandXor;		/* 0x1c */
	uint32 NandLlOpNand;            /* 0x20 */
	uint32 NandMplaneBaseExtAddr;	/* 0x24 */
	uint32 NandMplaneBaseAddr;	/* 0x28 */
	uint32 NandReserved1[9];	/* 0x2c-0x4f */
	uint32 NandAccControl;		/* 0x50 */
#define NAC_RD_ECC_EN		(1 << 31)
#define NAC_WR_ECC_EN		(1 << 30)
#define NAC_CE_CARE_EN          (1 << 28)
#define NAC_RD_ERASED_ECC_EN	(1 << 27)
#define NAC_PARTIAL_PAGE_EN	(1 << 26)
#define NAC_WR_PREEMPT_EN	(1 << 25)
#define NAC_PAGE_HIT_EN		(1 << 24)
#define NAC_PREFETCH_EN 	(1 << 23)
#define NAC_CACHE_MODE_EN	(1 << 22)
#define NAC_ECC_LVL_SHIFT	16
#define NAC_ECC_LVL_MASK	0x001f0000
#define NAC_ECC_LVL_DISABLE 0
#define NAC_ECC_LVL_BCH_1   1
#define NAC_ECC_LVL_BCH_2   2
#define NAC_ECC_LVL_BCH_3   3
#define NAC_ECC_LVL_BCH_4   4
#define NAC_ECC_LVL_BCH_5   5
#define NAC_ECC_LVL_BCH_6   6
#define NAC_ECC_LVL_BCH_7   7
#define NAC_ECC_LVL_BCH_8   8
#define NAC_ECC_LVL_BCH_9   9
#define NAC_ECC_LVL_BCH_10  10
#define NAC_ECC_LVL_BCH_11  11
#define NAC_ECC_LVL_BCH_12  12
#define NAC_ECC_LVL_BCH_13  13
#define NAC_ECC_LVL_BCH_14  14
#define NAC_ECC_LVL_HAMMING 15  /* Hamming if spare are size = 16, BCH15 otherwise */
#define NAC_ECC_LVL_BCH15   15    
#define NAC_ECC_LVL_BCH_16  16
#define NAC_ECC_LVL_BCH_17  17
/* BCH18 to 30 for sector size = 1K. To be added when we need it */
#define NAC_SECTOR_SIZE_1K	(1 << 7)
#define NAC_SPARE_SZ_SHIFT	0
#define NAC_SPARE_SZ_MASK	0x0000007f

	uint32 NandConfig;		/* 0x54 */ /* Nand Flash Config */
#define NC_CONFIG_LOCK		(1 << 31)
#define NC_BLK_SIZE_MASK	(0x7 << 28)
#define NC_BLK_SIZE_2048K	(0x6 << 28)
#define NC_BLK_SIZE_1024K	(0x5 << 28)
#define NC_BLK_SIZE_512K	(0x4 << 28)
#define NC_BLK_SIZE_256K	(0x3 << 28)
#define NC_BLK_SIZE_128K	(0x2 << 28)
#define NC_BLK_SIZE_16K		(0x1 << 28)
#define NC_BLK_SIZE_8K		(0x0 << 28)
#define NC_DEV_SIZE_SHIFT	24
#define NC_DEV_SIZE_MASK	(0x0f << NC_DEV_SIZE_SHIFT)
#define NC_DEV_WIDTH_MASK	(1 << 23)
#define NC_DEV_WIDTH_16		(1 << 23)
#define NC_DEV_WIDTH_8		(0 << 23)
#define NC_PG_SIZE_MASK		(0x3 << 20)
#define NC_PG_SIZE_8K		(0x3 << 20)
#define NC_PG_SIZE_4K		(0x2 << 20)
#define NC_PG_SIZE_2K		(0x1 << 20)
#define NC_PG_SIZE_512B		(0x0 << 20)
#define NC_FUL_ADDR_SHIFT	16
#define NC_FUL_ADDR_MASK	(0x7 << NC_FUL_ADDR_SHIFT)
#define NC_BLK_ADDR_SHIFT	8
#define NC_BLK_ADDR_MASK	(0x07 << NC_BLK_ADDR_SHIFT)

	uint32 NandTiming1;	/* 0x58 */ /* Nand Flash Timing Parameters 1 */
#define NT_TREH_MASK        0x000f0000
#define NT_TREH_SHIFT       16
#define NT_TRP_MASK         0x00f00000
#define NT_TRP_SHIFT        20
	uint32 NandTiming2;	/* 0x5c */ /* Nand Flash Timing Parameters 2 */
#define NT_TREAD_MASK       0x0000000f
#define NT_TREAD_SHIFT      0
	/* 0x60 */
	uint32 NandAccControlCs1;	/* Nand Flash Access Control */
	uint32 NandConfigCs1;		/* Nand Flash Config */
	uint32 NandTiming1Cs1;		/* Nand Flash Timing Parameters 1 */
	uint32 NandTiming2Cs1;		/* Nand Flash Timing Parameters 2 */
	/* 0x70 */
	uint32 NandAccControlCs2;	/* Nand Flash Access Control */
	uint32 NandConfigCs2;		/* Nand Flash Config */
	uint32 NandTiming1Cs2;		/* Nand Flash Timing Parameters 1 */
	uint32 NandTiming2Cs2;		/* Nand Flash Timing Parameters 2 */
	/* 0x80 */
	uint32 NandAccControlCs3;	/* Nand Flash Access Control */
	uint32 NandConfigCs3;		/* Nand Flash Config */
	uint32 NandTiming1Cs3;		/* Nand Flash Timing Parameters 1 */
	uint32 NandTiming2Cs3;		/* Nand Flash Timing Parameters 2 */
	/* 0x90 */
	uint32 NandAccControlCs4;	/* Nand Flash Access Control */
	uint32 NandConfigCs4;		/* Nand Flash Config */
	uint32 NandTiming1Cs4;		/* Nand Flash Timing Parameters 1 */
	uint32 NandTiming2Cs4;		/* Nand Flash Timing Parameters 2 */
	/* 0xa0 */
	uint32 NandAccControlCs5;	/* Nand Flash Access Control */
	uint32 NandConfigCs5;		/* Nand Flash Config */
	uint32 NandTiming1Cs5;		/* Nand Flash Timing Parameters 1 */
	uint32 NandTiming2Cs5;		/* Nand Flash Timing Parameters 2 */
	/* 0xb0 */
	uint32 NandAccControlCs6;	/* Nand Flash Access Control */
	uint32 NandConfigCs6;		/* Nand Flash Config */
	uint32 NandTiming1Cs6;		/* Nand Flash Timing Parameters 1 */
	uint32 NandTiming2Cs6;		/* Nand Flash Timing Parameters 2 */

	/* 0xc0 */
	uint32 NandCorrStatThreshold;	/* Correctable Error Reporting Threshold */
	uint32 NandCorrStatThresholdExt;	/* Correctable Error Reporting
						 * Threshold */
	uint32 NandBlkWrProtect;	/* Block Write Protect Enable and Size */
					/*   for EBI_CS0b */
	uint32 NandMplaneOpcode1;

	/* 0xd0 */
	uint32 NandMplaneOpcode2;
	uint32 NandMplaneCtrl;
	uint32 NandReserved2[9];	/* 0xd8-0xfb */
	uint32 NandUncorrErrorCount;	/* 0xfc */

	/* 0x100 */
	uint32 NandCorrErrorCount;
	uint32 NandReadErrorCount;	/* Read Error Count */
	uint32 NandBlockLockStatus;	/* Nand Flash Block Lock Status */
	uint32 NandEccCorrExtAddr;	/* ECC Correctable Error Extended Address*/
	/* 0x110 */
	uint32 NandEccCorrAddr;		/* ECC Correctable Error Address */
	uint32 NandEccUncExtAddr;	/* ECC Uncorrectable Error Extended Addr */
	uint32 NandEccUncAddr;		/* ECC Uncorrectable Error Address */
	uint32 NandFlashReadExtAddr;	/* Flash Read Data Extended Address */
	/* 0x120 */
	uint32 NandFlashReadAddr;	/* Flash Read Data Address */
	uint32 NandProgramPageExtAddr;	/* Page Program Extended Address */
	uint32 NandProgramPageAddr;	/* Page Program Address */
	uint32 NandCopyBackExtAddr;	/* Copy Back Extended Address */
	/* 0x130 */
	uint32 NandCopyBackAddr;	/* Copy Back Address */
	uint32 NandBlockEraseExtAddr;	/* Block Erase Extended Address */
	uint32 NandBlockEraseAddr;	/* Block Erase Address */
	uint32 NandInvReadExtAddr;	/* Flash Invalid Data Extended Address */
	/* 0x140 */
	uint32 NandInvReadAddr;		/* Flash Invalid Data Address */
	uint32 NandInitStatus;
	uint32 NandOnfiStatus;		/* ONFI Status */
	uint32 NandOnfiDebugData;	/* ONFI Debug Data */

	uint32 NandSemaphore;		/* 0x150 */ /* Semaphore */
	uint32 NandReserved3[16];	/* 0x154-0x193 */

	/* 0x194 */
	uint32 NandFlashDeviceId;	/* Nand Flash Device ID */
	uint32 NandFlashDeviceIdExt;	/* Nand Flash Extended Device ID */
	uint32 NandLlRdData;		/* Nand Flash Low Level Read Data */

	uint32 NandReserved4[24];	/* 0x1a0 - 0x1ff */

	/* 0x200 */
	uint32 NandSpareAreaReadOfs0;	/* Nand Flash Spare Area Read Bytes 0-3 */
	uint32 NandSpareAreaReadOfs4;	/* Nand Flash Spare Area Read Bytes 4-7 */
	uint32 NandSpareAreaReadOfs8;	/* Nand Flash Spare Area Read Bytes 8-11 */
	uint32 NandSpareAreaReadOfsC;	/* Nand Flash Spare Area Read Bytes 12-15*/
	/* 0x210 */
	uint32 NandSpareAreaReadOfs10;	/* Nand Flash Spare Area Read Bytes 16-19 */
	uint32 NandSpareAreaReadOfs14;	/* Nand Flash Spare Area Read Bytes 20-23 */
	uint32 NandSpareAreaReadOfs18;	/* Nand Flash Spare Area Read Bytes 24-27 */
	uint32 NandSpareAreaReadOfs1C;	/* Nand Flash Spare Area Read Bytes 28-31*/
	/* 0x220 */
	uint32 NandSpareAreaReadOfs20;	/* Nand Flash Spare Area Read Bytes 32-35 */
	uint32 NandSpareAreaReadOfs24;	/* Nand Flash Spare Area Read Bytes 36-39 */
	uint32 NandSpareAreaReadOfs28;	/* Nand Flash Spare Area Read Bytes 40-43 */
	uint32 NandSpareAreaReadOfs2C;	/* Nand Flash Spare Area Read Bytes 44-47*/
	/* 0x230 */
	uint32 NandSpareAreaReadOfs30;	/* Nand Flash Spare Area Read Bytes 48-51 */
	uint32 NandSpareAreaReadOfs34;	/* Nand Flash Spare Area Read Bytes 52-55 */
	uint32 NandSpareAreaReadOfs38;	/* Nand Flash Spare Area Read Bytes 56-59 */
	uint32 NandSpareAreaReadOfs3C;	/* Nand Flash Spare Area Read Bytes 60-63*/

	uint32 NandReserved5[16];	/* 0x240-0x27f */

	/* 0x280 */
	uint32 NandSpareAreaWriteOfs0;	/* Nand Flash Spare Area Write Bytes 0-3 */
	uint32 NandSpareAreaWriteOfs4;	/* Nand Flash Spare Area Write Bytes 4-7 */
	uint32 NandSpareAreaWriteOfs8;	/* Nand Flash Spare Area Write Bytes 8-11 */
	uint32 NandSpareAreaWriteOfsC;	/* Nand Flash Spare Area Write Bytes 12-15 */
	/* 0x290 */
	uint32 NandSpareAreaWriteOfs10;	/* Nand Flash Spare Area Write Bytes 16-19 */
	uint32 NandSpareAreaWriteOfs14;	/* Nand Flash Spare Area Write Bytes 20-23 */
	uint32 NandSpareAreaWriteOfs18;	/* Nand Flash Spare Area Write Bytes 24-27 */
	uint32 NandSpareAreaWriteOfs1C;	/* Nand Flash Spare Area Write Bytes 28-31 */
	/* 0x2a0 */
	uint32 NandSpareAreaWriteOfs20;	/* Nand Flash Spare Area Write Bytes 32-35 */
	uint32 NandSpareAreaWriteOfs24;	/* Nand Flash Spare Area Write Bytes 36-39 */
	uint32 NandSpareAreaWriteOfs28;	/* Nand Flash Spare Area Write Bytes 40-43 */
	uint32 NandSpareAreaWriteOfs2C;	/* Nand Flash Spare Area Write Bytes 44-47 */
	/* 0x2b0 */
	uint32 NandSpareAreaWriteOfs30;	/* Nand Flash Spare Area Write Bytes 48-51 */
	uint32 NandSpareAreaWriteOfs34;	/* Nand Flash Spare Area Write Bytes 52-55 */
	uint32 NandSpareAreaWriteOfs38;	/* Nand Flash Spare Area Write Bytes 56-59 */
	uint32 NandSpareAreaWriteOfs3C;	/* Nand Flash Spare Area Write Bytes 60-63 */
	/* 0x2c0 */
	uint32 NandDdrTiming;
	uint32 NandDdrNcdlCalibCtl;
	uint32 NandDdrNcdlCalibPeriod;
	uint32 NandDdrNcdlCalibStat;
	/* 0x2d0 */
	uint32 NandDdrNcdlMode;
	uint32 NandDdrNcdlOffset;
	uint32 NandDdrPhyCtl;
	uint32 NandDdrPhyBistCtl;
	/* 0x2e0 */
	uint32 NandDdrPhyBistStat;
	uint32 NandDdrDiagStat0;
	uint32 NandDdrDiagStat1;
	uint32 NandReserved6[69];	/* 0x2ec-0x3ff */

	/* 0x400 */
	uint32 NandFlashCache[128];	/* 0x400-0x5ff */
} NandCtrlRegs;

#define NAND ((volatile NandCtrlRegs * const) NAND_REG_BASE)

/*
** Misc registers control
*/
typedef struct MiscReg {
	uint32 ClkgenCtr3;              /* 0x00 */
	uint32 Reg2P5VLDOCtrl;          /* 0x04 */
	uint32 Reg2P5VLDOCtrlEnable;    /* 0x08 */
	uint32 Reg1P0VLDOCtrl;          /* 0x0c */
	uint32 Reg1P0VLDOCtrlEnable;    /* 0x10 */
	uint32 RoboswCtrl;              /* 0x14 */ 
#define RSW_RGMII_PADS_ENABLE      (1<<18)           
#define RSW_RGMII_PAD_MODEHV_CTRL  (1<<17)
#define RSW_MII_DUMB_FWDG_EN       (1<<15)
#define RSW_HW_FWDG_EN             (1<<12)
 	uint32 EphyPhyAd;               /* 0x18 */
#define EPHY_PHYAD_MASK             0x1f 	 
 	uint32 EphyPwrMgnt;             /* 0x1c */ 
#define EPHY_PWR_DOWN_PHY            (1<<29)
#define EPHY_SUPER_ISOLATE_0 	   (1<<28)
#define EPHY_SUPER_ISOLATE_1	   (1<<27)
#define EPHY_SUPER_ISOLATE_2	   (1<<26)
#define EPHY_SUPER_ISOLATE_3	   (1<<25)
#define EPHY_PWR_DOWN_TX_0	   (1<<24)
#define EPHY_PWR_DOWN_RX_0	   (1<<23)
#define EPHY_PWR_DOWN_SD_0	   (1<<22)
#define EPHY_PWR_DOWN_RD_0	   (1<<21)
#define EPHY_PWR_DOWN_0		   (1<<20)
#define EPHY_PWR_DOWN_TX_1	   (1<<19)
#define EPHY_PWR_DOWN_RX_1	   (1<<18)
#define EPHY_PWR_DOWN_SD_1	   (1<<17)
#define EPHY_PWR_DOWN_RD_1	   (1<<16)
#define EPHY_PWR_DOWN_1		   (1<<15)
#define EPHY_PWR_DOWN_TX_2	   (1<<14)
#define EPHY_PWR_DOWN_RX_2	   (1<<13)
#define EPHY_PWR_DOWN_SD_2	   (1<<12)
#define EPHY_PWR_DOWN_RD_2	   (1<<11)
#define EPHY_PWR_DOWN_2		   (1<<10)
#define EPHY_PWR_DOWN_TX_3	   (1<<9)
#define EPHY_PWR_DOWN_RX_3	   (1<<8)
#define EPHY_PWR_DOWN_SD_3	   (1<<7)
#define EPHY_PWR_DOWN_RD_3	   (1<<6)
#define EPHY_PWR_DOWN_3		   (1<<5)
#define EPHY_PWR_DOWN_ALL_0     (EPHY_PWR_DOWN_0 | EPHY_PWR_DOWN_RD_0 | EPHY_PWR_DOWN_SD_0 | EPHY_PWR_DOWN_RX_0 | EPHY_PWR_DOWN_TX_0)
#define EPHY_PWR_DOWN_SHIFT_FACTOR   5
#define EPHY_PWR_DOWN_DLL  	               (1<<4)
#define EPHY_PWR_DOWN_BIAS                 (1<<3)	
#define EPHY_PWR_MGNT_IDDQ_GLOBAL_PWR      (1<<2)
#define EPHY_PWR_MGNT_AUTO_PWR_DN_EN_LD	   (1<<1)
#define EPHY_PWR_MGNT_MDC_EX_CLK_DIS_LD	   (1<<0)
 	uint32 EphyResetCtrl;           /* 0x20 */ 
#define EPHY_RESET_N               (1<<4)
#define EPHY_RESET_N_PHY_3         (1<<3)
#define EPHY_RESET_N_PHY_2         (1<<2)
#define EPHY_RESET_N_PHY_1         (1<<1)
#define EPHY_RESET_N_PHY_0         (1<<0)
 	uint32 EphyRefClkSel;           /* 0x24 */ 
 	uint32 EphyTestCtrl;            /* 0x28 */
#define EPHY_TEST_DLLBYP_PIN_F     (1<<1)
 	uint32 EphyPhySelect;           /* 0x2c */
 	uint32 Dummy;                   /* 0x30 */
 	uint32 TpDirOverride[2];        /* 0x34 */
 	uint32 SwLedCtrl;               /* 0x3c */
}MiscReg;
#define MISC_REG ((volatile MiscReg * const) MISC_REG_BASE)

/*
** pad control register
*/
typedef struct PadCtlReg {
 	uint32 PadCtrl[26];
#define PADCTRL_25_RGMII_TXD_0_PAD_DRV_SEL_SHIFT  9
#define PADCTRL_25_RGMII_TXD_1_PAD_DRV_SEL_SHIFT  6
#define PADCTRL_25_RGMII_TXD_2_PAD_DRV_SEL_SHIFT  3
#define PADCTRL_25_RGMII_TXD_3_PAD_DRV_SEL_SHIFT  0
#define RGMII_TXD_DRV_SEL_VAL_02mA   0
#define RGMII_TXD_DRV_SEL_VAL_04mA   1
#define RGMII_TXD_DRV_SEL_VAL_06mA   2  
#define RGMII_TXD_DRV_SEL_VAL_08mA   3
#define RGMII_TXD_DRV_SEL_VAL_10mA   4
#define RGMII_TXD_DRV_SEL_VAL_12mA   5
#define RGMII_TXD_DRV_SEL_VAL_14mA   6
#define RGMII_TXD_DRV_SEL_VAL_16mA   7

}PadCtlReg;
#define PAD_CTL_REG ((volatile PadCtlReg * const) PAD_CTL_BASE)

/*
 * Power Management Control
 */
typedef struct PmcCtrlReg {
	/* 0x00 */
	uint32 l1Irq4keMask;
	uint32 l1Irq4keStatus;
	uint32 l1IrqMipsMask;
	uint32 l1IrqMipsStatus;
	/* 0x10 */
	uint32 l2IrqGpMask;
	uint32 l2IrqGpStatus;
	uint32 gpTmr0Ctl;
	uint32 gpTmr0Cnt;
	/* 0x20 */
	uint32 gpTmr1Ctl;
	uint32 gpTmr1Cnt;
	uint32 hostMboxIn;
	uint32 hostMboxOut;
	/* 0x30 */
#define PMC_CTRL_GP_FLASH_BOOT_STALL                  0x00000080
	uint32 gpOut;
	uint32 gpIn;
	uint32 gpInIrqMask;
	uint32 gpInIrqStatus;
	/* 0x40 */
	uint32 dmaCtrl;
	uint32 dmaStatus;
	uint32 dma0_3FifoStatus;
	uint32 unused0[3];	/* 0x4c-0x57 */
	/* 0x58 */
	uint32 l1IrqMips1Mask;
	uint32 diagControl;
	/* 0x60 */
	uint32 diagHigh;
	uint32 diagLow;
	uint32 badAddr;
	uint32 addr1WndwMask;
	/* 0x70 */
	uint32 addr1WndwBaseIn;
	uint32 addr1WndwBaseOut;
	uint32 addr2WndwMask;
	uint32 addr2WndwBaseIn;
	/* 0x80 */
	uint32 addr2WndwBaseOut;
	uint32 scratch;
	uint32 tm;
	uint32 softResets;
	/* 0x90 */
	uint32 eb2ubusTimeout;
	uint32 m4keCoreStatus;
	uint32 gpInIrqSense;
	uint32 ubSlaveTimeout;
	/* 0xa0 */
	uint32 diagEn;
	uint32 devTimeout;
	uint32 ubusErrorOutMask;
	uint32 diagCaptStopMask;
	/* 0xb0 */
	uint32 revId;
	uint32 gpTmr2Ctl;
	uint32 gpTmr2Cnt;
	uint32 legacyMode;
	/* 0xc0 */
	uint32 smisbMonitor;
	uint32 diagCtrl;
	uint32 diagStat;
	uint32 diagMask;
	/* 0xd0 */
	uint32 diagRslt;
	uint32 diagCmp;
	uint32 diagCapt;
	uint32 diagCnt;
	/* 0xe0 */
	uint32 diagEdgeCnt;
	uint32 unused1[4];	/* 0xe4-0xf3 */
	/* 0xf4 */
	uint32 iopPeriphBaseAddr;
	uint32 lfsr;
	uint32 unused2;		/* 0xfc-0xff */
} PmcCtrlReg;

typedef struct PmcOutFifoReg {
	uint32 msgCtrl;		/* 0x00 */
	uint32 msgSts;		/* 0x04 */
	uint32 unused[14];	/* 0x08-0x3f */
	uint32 msgData[16];	/* 0x40-0x7c */
} PmcOutFifoReg;

typedef struct PmcInFifoReg {
	uint32 msgCtrl;		/* 0x00 */
	uint32 msgSts;		/* 0x04 */
	uint32 unused[13];	/* 0x08-0x3b */
	uint32 msgLast;		/* 0x3c */
	uint32 msgData[16];	/* 0x40-0x7c */
} PmcInFifoReg;

typedef struct PmcDmaReg {
	/* 0x00 */
	uint32 src;
	uint32 dest;
	uint32 cmdList;
	uint32 lenCtl;
	/* 0x10 */
	uint32 rsltSrc;
	uint32 rsltDest;
	uint32 rsltHcs;
	uint32 rsltLenStat;
} PmcDmaReg;

typedef struct PmcTokenReg {
	/* 0x00 */
	uint32 bufSize;
	uint32 bufBase;
	uint32 idx2ptrIdx;
	uint32 idx2ptrPtr;
	/* 0x10 */
	uint32 unused[2];
	uint32 bufSize2;
} PmcTokenReg;

typedef struct PmcPerfPowReg {
	/* 0x00 */
	uint32 dcacheHit;
	uint32 dcacheMiss;
	uint32 icacheHit;
	uint32 icacheMiss;
	/* 0x10 */
	uint32 instnComplete;
	uint32 wtbMerge;
	uint32 wtbNoMerge;
	uint32 itlbHit;
	/* 0x20 */
	uint32 itlbMiss;
	uint32 dtlbHit;
	uint32 dtlbMiss;
	uint32 jtlbHit;
	/* 0x30 */
	uint32 jtlbMiss;
	uint32 powerSubZone;
	uint32 powerMemPda;
	uint32 freqScalarCtrl;
	/* 0x40 */
	uint32 freqScalarMask;
} PmcPerfPowReg;

typedef struct PmcDQMReg {
	/* 0x00 */
	uint32 cfg;
	uint32 _4keLowWtmkIrqMask;
	uint32 mipsLowWtmkIrqMask;
	uint32 lowWtmkIrqMask;
	/* 0x10 */
	uint32 _4keNotEmptyIrqMask;
	uint32 mipsNotEmptyIrqMask;
	uint32 notEmptyIrqSts;
	uint32 queueRst;
	/* 0x20 */
	uint32 notEmptySts;
	uint32 nextAvailMask;
	uint32 nextAvailQueue;
	uint32 mips1LowWtmkIrqMask;
	/* 0x30 */
	uint32 mips1NotEmptyIrqMask;
	uint32 autoSrcPidInsert;
} PmcDQMReg;

typedef struct PmcCntReg {
	uint32 cntr[10];
	uint32 unused[6];	/* 0x28-0x3f */
	uint32 cntrIrqMask;
	uint32 cntrIrqSts;
} PmcCntReg;

typedef struct PmcDqmQCtrlReg {
	uint32 size;
	uint32 cfga;
	uint32 cfgb;
	uint32 cfgc;
} PmcDqmQCtrlReg;

typedef struct PmcDqmQDataReg {
	uint32 word[4];
} PmcDqmQDataReg;

typedef struct PmcDqmQMibReg {
	uint32 qNumFull[32];
	uint32 qNumEmpty[32];
	uint32 qNumPushed[32];
} PmcDqmQMibReg;

typedef struct Pmc {
	uint32 baseReserved;		/* 0x0000 */
	uint32 unused0[1023];
	PmcCtrlReg ctrl;		/* 0x1000 */

	PmcOutFifoReg outFifo;		/* 0x1100 */
	uint32 unused1[32];		/* 0x1180-0x11ff */
	PmcInFifoReg inFifo;		/* 0x1200 */
	uint32 unused2[32];		/* 0x1280-0x12ff */

	PmcDmaReg dma[2];		/* 0x1300 */
	uint32 unused3[48];		/* 0x1340-0x13ff */

	PmcTokenReg token;		/* 0x1400 */
	uint32 unused4[121];		/* 0x141c-0x15ff */

	PmcPerfPowReg perfPower;	/* 0x1600 */
	uint32 unused5[47];		/* 0x1644-0x16ff */

	uint32 msgId[32];		/* 0x1700 */
	uint32 unused6[32];		/* 0x1780-0x17ff */

	PmcDQMReg dqm;			/* 0x1800 */
	uint32 unused7[50];		/* 0x1838-0x18ff */

	PmcCntReg hwCounter;		/* 0x1900 */
	uint32 unused8[46];		/* 0x1948-0x19ff */

	PmcDqmQCtrlReg dqmQCtrl[32];	/* 0x1a00 */
	PmcDqmQDataReg dqmQData[32];	/* 0x1c00 */
	uint32 unused9[64];		/* 0x1e00-0x1eff */

	uint32 qStatus[32];		/* 0x1f00 */
	uint32 unused10[32];		/* 0x1f80-0x1fff */

	PmcDqmQMibReg qMib;		/* 0x2000 */
	uint32 unused11[1952];		/* 0x2180-0x3ffff */

	uint32 sharedMem[8192];		/* 0x4000-0xbffc */
} Pmc;

#define PMC ((volatile Pmc * const) PMC_BASE)

/*
 * Process Monitor Module
 */
typedef struct PMRingOscillatorControl {
	uint32 control;
	uint32 en_lo;
	uint32 en_mid;
	uint32 en_hi;
	uint32 idle_lo;
	uint32 idle_mid;
	uint32 idle_hi;
} PMRingOscillatorControl;

#define RCAL_0P25UM_HORZ          0
#define RCAL_0P25UM_VERT          1
#define RCAL_0P5UM_HORZ           2
#define RCAL_0P5UM_VERT           3
#define RCAL_1UM_HORZ             4
#define RCAL_1UM_VERT             5
#define PMMISC_RMON_EXT_REG       ((RCAL_1UM_VERT + 1)/2)
#define PMMISC_RMON_VALID_MASK    (0x1<<16)
typedef struct PMMiscControl {
	uint32 gp_out;
	uint32 clock_select;
	uint32 unused[2];
	uint32 misc[4];
} PMMiscControl;

typedef struct PMSSBMasterControl {
	uint32 control;
	uint32 wr_data;
	uint32 rd_data;
} PMSSBMasterControl;

typedef struct PMEctrControl {
	uint32 control;
	uint32 interval;
	uint32 thresh_lo;
	uint32 thresh_hi;
	uint32 count;
} PMEctrControl;

typedef struct PMBMaster {
	uint32 ctrl;
#define PMC_PMBM_START		(1 << 31)
#define PMC_PMBM_TIMEOUT	(1 << 30)
#define PMC_PMBM_SLAVE_ERR	(1 << 29)
#define PMC_PMBM_BUSY		(1 << 28)
#define PMC_PMBM_Read		(0 << 20)
#define PMC_PMBM_Write		(1 << 20)
	uint32 wr_data;
	uint32 timeout;
	uint32 rd_data;
	uint32 unused[4];
} PMBMaster;

typedef struct PMAPVTMONControl {
	uint32 control;
	uint32 reserved;
	uint32 cfg_lo;
	uint32 cfg_hi;
	uint32 data;
	uint32 vref_data;
	uint32 unused[2];
	uint32 ascan_cfg;
	uint32 warn_temp;
	uint32 reset_temp;
	uint32 temp_value;
	uint32 data1_value;
	uint32 data2_value;
	uint32 data3_value;
} PMAPVTMONControl;

typedef struct PMUBUSCfg {
	uint32 window[8];
	uint32 control;
} PMUBUSCfg;

typedef struct ProcessMonitorRegs {
	uint32 MonitorCtrl;		/* 0x00 */
	uint32 unused0[7];
	PMRingOscillatorControl ROSC;	/* 0x20 */
	uint32 unused1;
	PMMiscControl Misc;		/* 0x40 */
	PMSSBMasterControl SSBMaster;	/* 0x60 */
	uint32 unused2[5];
	PMEctrControl Ectr;		/* 0x80 */
	uint32 unused3[11];
	PMBMaster PMBM[2];		/* 0xc0 */
	PMAPVTMONControl APvtmonCtrl;	/* 0x100 */
	uint32 unused4[9];
	PMUBUSCfg UBUSCfg;		/* 0x160 */
} ProcessMonitorRegs;

#define PROCMON ((volatile ProcessMonitorRegs * const) PROC_MON_BASE)

/*
** LedControl Register Set Definitions.
*/

/*
 * LedControl Register Set Definitions.
 */
typedef struct LedControl {
	uint32 glbCtrl;		/* 0x00 */
	uint32 mask;		/* 0x04 */
	uint32 hWLedEn;		/* 0x08 */
	uint32 serialLedShiftSel; /* 0x0c */
	uint32 flashRateCtrl[4];	/* 0x10-0x1c */
	uint32 brightCtrl[4];	/* 0x20-0x2c */
	uint32 powerLedCfg;	/* 0x30 */
	uint32 pledLut[2][16];	/* 0x34-0x70, 0x74-0xb0 */
	uint32 HwPolarity;	/* 0xb4 */    /*B0 ONLY FROM HERE */
	uint32 SwData;		/* 0xb8 */
	uint32 SwPolarity;	/* 0xbc */
	uint32 ParallelLedPolarity;	/* 0xc0 */
	uint32 SerialLedPolarity;	/* 0xc4 */
	uint32 HwLedStatus;		/* 0xc8 */
} LedControl;


#define LED ((volatile LedControl * const) LED_BASE)
#define LED_NUM_LEDS       32
#define LED_NUM_TO_MASK(X)       (1 << ((X) & (LED_NUM_LEDS-1)))


#define GPIO_NUM_TO_LED_MODE_SHIFT(X) \
    ((((X) & BP_GPIO_NUM_MASK) < 8) ? (32 + (((X) & BP_GPIO_NUM_MASK) << 1)) : \
    ((((X) & BP_GPIO_NUM_MASK) - 8) << 1))


/*
** SDR/DDR Memory Controller Register Set Definitions.
*/
typedef struct UBUSInterface {
	uint32 CFG;			/* 0x00 */
#define UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT   0x0  
#define UBUSIF_CFG_WRITE_REPLY_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT)
#define UBUSIF_CFG_WRITE_BURST_MODE_SHIFT   0x1  
#define UBUSIF_CFG_WRITE_BURST_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_BURST_MODE_SHIFT)
#define UBUSIF_CFG_INBAND_ERR_MASK_SHIFT    0x2
#define UBUSIF_CFG_INBAND_ERR_MASK_MASK     (0x1<< UBUSIF_CFG_INBAND_ERR_MASK_SHIFT)
#define UBUSIF_CFG_OOB_ERR_MASK_SHIFT       0x3
#define UBUSIF_CFG_OOB_ERR_MASK_MASK        (0x1<< UBUSIF_CFG_OOB_ERR_MASK_SHIFT)
	uint32 SRC_QUEUE_CTRL_0;	/* 0x04 */
	uint32 SRC_QUEUE_CTRL_1;	/* 0x08 */
	uint32 SRC_QUEUE_CTRL_2;	/* 0x0c */
	uint32 SRC_QUEUE_CTRL_3;	/* 0x10 */
	uint32 REP_ARB_MODE;		/* 0x14 */
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT 0x0
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_MASK  (0x1<<UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT)
	uint32 SCRATCH;			/* 0x18 */
	uint32 DEBUG_R0;		/* 0x1c */
	uint32 unused[8];		/* 0x20-0x3f */
} UBUSInterface;

typedef struct EDISEngine {
	uint32 REV_ID;			/* 0x00 */
	uint32 CTRL_TRIG;		/* 0x04 */
	uint32 CTRL_MODE;		/* 0x08 */
	uint32 CTRL_SIZE;		/* 0x0c */
	uint32 CTRL_ADDR_START;		/* 0x10 */
	uint32 CTRL_ADDR_START_EXT;	/* 0x14 */
	uint32 CTRL_ADDR_END;		/* 0x18 */
	uint32 CTRL_ADDR_END_EXT;	/* 0x1c */
	uint32 CTRL_WRITE_MASKS;	/* 0x20 */
	uint32 unused0[7];		/* 0x24-0x3f */
	uint32 STAT_MAIN;		/* 0x40 */
	uint32 STAT_WORDS_WRITTEN;	/* 0x44 */
	uint32 STAT_WORDS_READ;		/* 0x48 */
	uint32 STAT_ERROR_COUNT;	/* 0x4c */
	uint32 STAT_ERROR_BITS;		/* 0x50 */
	uint32 STAT_ADDR_LAST;		/* 0x54 */
	uint32 STAT_ADDR_LAST_EXT;	/* 0x58 */
	uint32 unused1[8];		/* 0x5c-0x7b */
	uint32 STAT_DEBUG;		/* 0x7c */
	uint32 STAT_DATA_PORT[8];	/* 0x80-0x9c */
	uint32 GEN_LFSR_STATE[4];	/* 0xa0-0xac */
	uint32 GEN_CLOCK;		/* 0xb0 */
	uint32 GEN_PATTERN;		/* 0xb4 */
	uint32 unused2[2];		/* 0xb8-0xbf */
	uint32 BYTELANE_0_CTRL_LO;	/* 0xc0 */
	uint32 BYTELANE_0_CTRL_HI;	/* 0xc4 */
	uint32 BYTELANE_1_CTRL_LO;	/* 0xc8 */
	uint32 BYTELANE_1_CTRL_HI;	/* 0xcc */
	uint32 BYTELANE_2_CTRL_LO;	/* 0xd0 */
	uint32 BYTELANE_2_CTRL_HI;	/* 0xd4 */
	uint32 BYTELANE_3_CTRL_LO;	/* 0xd8 */
	uint32 BYTELANE_3_CTRL_HI;	/* 0xdc */
	uint32 BYTELANE_0_STAT_LO;	/* 0xe0 */
	uint32 BYTELANE_0_STAT_HI;	/* 0xe4 */
	uint32 BYTELANE_1_STAT_LO;	/* 0xe8 */
	uint32 BYTELANE_1_STAT_HI;	/* 0xec */
	uint32 BYTELANE_2_STAT_LO;	/* 0xf0 */
	uint32 BYTELANE_2_STAT_HI;	/* 0xf4 */
	uint32 BYTELANE_3_STAT_LO;	/* 0xf8 */
	uint32 BYTELANE_3_STAT_HI;	/* 0xfc */
} EDISEngine;

typedef struct SecureRangeCheckers {
	uint32 LOCK;			/* 0x00 */
	uint32 LOG_INFO_0;		/* 0x04 */
	uint32 LOG_INFO_1;		/* 0x08 */
	uint32 CTRL_0;			/* 0x0c */
	uint32 UBUS0_PORT_0;		/* 0x10 */
	uint32 BASE_0;			/* 0x14 */
	uint32 CTRL_1;			/* 0x18 */
	uint32 UBUS0_PORT_1;		/* 0x1c */
	uint32 BASE_1;			/* 0x20 */
	uint32 CTRL_2;			/* 0x24 */
	uint32 UBUS0_PORT_2;		/* 0x28 */
	uint32 BASE_2;			/* 0x2c */
	uint32 CTRL_3;			/* 0x30 */
	uint32 UBUS0_PORT_3;		/* 0x34 */
	uint32 BASE_3;			/* 0x38 */
	uint32 CTRL_4;			/* 0x3c */
	uint32 UBUS0_PORT_4;		/* 0x40 */
	uint32 BASE_4;			/* 0x44 */
	uint32 CTRL_5;			/* 0x48 */
	uint32 UBUS0_PORT_5;		/* 0x4c */
	uint32 BASE_5;			/* 0x50 */
	uint32 CTRL_6;			/* 0x54 */
	uint32 UBUS0_PORT_6;		/* 0x58 */
	uint32 BASE_6;			/* 0x5c */
	uint32 CTRL_7;			/* 0x60 */
	uint32 UBUS0_PORT_7;		/* 0x64 */
	uint32 BASE_7;			/* 0x68 */
	uint32 unused[37];		/* 0x6c-0xff */
} SecureRangeCheckers;


typedef struct SdrCfg {
    uint32  SDR_CFG;                             /* 0x00 */
#define MEMC_SDRAM_SPACE_SHIFT         4
#define MEMC_SDRAM_SPACE_MASK          (0xF<<MEMC_SDRAM_SPACE_SHIFT)
    uint32  SCRATCH;                             /* 0x04 */
    uint32  unused1[7];                          /* 0x08 - 0x23*/
    uint32  AUTO_REFRESH;                        /* 0x24 */
    uint32  TIMING_PARAM;                        /* 0x28 */
    uint32  unused2;                             /* 0x2c */
    uint32  DDR_TIMING_PARAM;                    /* 0x30 */
    uint32  DDR_DRIVE_PARAM;                     /* 0x34 */
    uint32  unused3[2];                          /* 0x38 - 0x3f*/
    uint32  INT_STATUS;                          /* 0x40 */
    uint32  INT_MASK;                            /* 0x44 */
    uint32  SDR_VERSION;                         /* 0x48 */
    uint32  CKTAP_CTRL;                          /* 0x4C */
    uint32  unused4[12];                         /* 0x50 - 0x7f*/
    uint32  DRAM_CMD[32];                        /* 0x80 */
#define SELF_REFRESH_CMD (0x4*4)
} SdrCfg;

typedef struct MEMCControl {
	uint32 GLB_VERS;		/* 0x000 */
	uint32 GLB_GCFG;		/* 0x004 */
#define MEMC_GLB_GCFG_DRAM_EN_SHIFT        31
#define MEMC_GLB_GCFG_DRAM_EN_MASK         (0x1<<MEMC_GLB_GCFG_DRAM_EN_SHIFT)
#define MEMC_GLB_GCFG_MEMINITDONE_SHIFT    8
#define MEMC_GLB_GCFG_MEMINITDONE_MASK     (0x1<<MEMC_GLB_GCFG_MEMINITDONE_SHIFT)
#define MEMC_GLB_GCFG_SIZE1_SHIFT          0
#define MEMC_GLB_GCFG_SIZE1_MASK           (0xf<<MEMC_GLB_GCFG_SIZE1_SHIFT)

	uint32 unused0[2];		/* 0x008-0x00f */  
	uint32 GLB_CFG;			/* 0x010 */
#define MEMC_GLB_CFG_RR_MODE_SHIFT      0x0
#define MEMC_GLB_CFG_RR_MODE_MASK       (0x1<<MEMC_GLB_CFG_RR_MODE_SHIFT)
#define MEMC_GLB_CFG_BURST_MODE_SHIFT   0x1
#define MEMC_GLB_CFG_BURST_MODE_MASK    (0x1<<MEMC_GLB_CFG_BURST_MODE_SHIFT)
	uint32 GLB_QUE_DIS;		/* 0x014 */
	uint32 GLB_SP_SEL;		/* 0x018 */
#define MEMC_GLB_SP_SEL_SELECT_MASK     0xf
	uint32 GLB_SP_PRI_0;		/* 0x01c */
	uint32 GLB_SP_PRI_1;		/* 0x020 */
	uint32 unused1[7];		/* 0x024-0x3f */
	uint32 GLB_RR_QUANTUM[3];	/* 0x040-0x04b */
	uint32 unused2[13];		/* 0x04c-0x07f */

	uint32 INTR2_CPU_STATUS;	/* 0x080 */
	uint32 INTR2_CPU_SET;		/* 0x084 */
	uint32 INTR2_CPU_CLEAR;		/* 0x088 */
	uint32 INTR2_CPU_MASK_STATUS;	/* 0x08c */
	uint32 INTR2_CPU_MASK_SET;	/* 0x090 */
	uint32 INTR2_CPU_MASK_CLEAR;	/* 0x094 */
	uint32 unused3[10];		/* 0x098-0x0bf */

	uint32 SRAM_REMAP_CTRL;		/* 0x0c0 */
	uint32 SRAM_REMAP_INIT;		/* 0x0c4 */
	uint32 SRAM_REMAP_LOG_INFO_0;	/* 0x0c8 */
	uint32 SRAM_REMAP_LOG_INFO_1;	/* 0x0cc */
	uint32 unused4[140];		/* 0x0d0-0x2ff */

        UBUSInterface UBUSIF0;		/* 0x300-0x33f */
	UBUSInterface UBUSIF1;		/* 0x340-0x37f */

	uint32 unused5[96];		/* 0x380-0x4ff */

	EDISEngine EDIS_0;		/* 0x500 */
        uint32 unused6[64];		/* 0x600-0x6ff */

	uint32 STATS_CTRL;		/* 0x700 */
	uint32 STATS_TIMER_CFG;		/* 0x704 */
	uint32 STATS_TIMER_COUNT;	/* 0x708 */
	uint32 STATS_TOTAL_SLICE;	/* 0x70c */
	uint32 STATS_TOTAL_PACKET;	/* 0x710 */
	uint32 STATS_SLICE_REORDER;	/* 0x714 */
	uint32 STATS_IDLE_DDR_CYCLE;	/* 0x718 */
	uint32 STATS_ARB_GRANT;		/* 0x71c */
	uint32 STATS_PROG_0;		/* 0x720 */
	uint32 STATS_PROG_1;		/* 0x724 */
	uint32 STATS_ARB_GRANT_MATCH;	/* 0x728 */
	uint32 STATS_CFG_0;		/* 0x72c */
	uint32 STATS_CFG_1;		/* 0x730 */
	uint32 unused14[19];		/* 0x734-0x77f */

	uint32 CAP_CAPTURE_CFG;		/* 0x780 */
	uint32 CAP_TRIGGER_ADDR;	/* 0x784 */
	uint32 CAP_READ_CTRL;		/* 0x788 */
	uint32 unused15;		/* 0x78c */
	uint32 CAP_CAPTURE_MATCH0;	/* 0x790 */
	uint32 CAP_CAPTURE_MATCH1;	/* 0x794 */
	uint32 CAP_CAPTURE_MATCH2;	/* 0x798 */
	uint32 unused16;		/* 0x79c */
	uint32 CAP_CAPTURE_MASK0;	/* 0x7a0 */
	uint32 CAP_CAPTURE_MASK1;	/* 0x7a4 */
	uint32 CAP_CAPTURE_MASK2;	/* 0x7a8 */
	uint32 unused17;		/* 0x7ac */
	uint32 CAP_TRIGGER_MATCH0;	/* 0x7b0 */
	uint32 CAP_TRIGGER_MATCH1;	/* 0x7b4 */
	uint32 CAP_TRIGGER_MATCH2;	/* 0x7b8 */
	uint32 unused18;		/* 0x7bc */
	uint32 CAP_TRIGGER_MASK0;	/* 0x7c0 */
	uint32 CAP_TRIGGER_MASK1;	/* 0x7c4 */
	uint32 CAP_TRIGGER_MASK2;	/* 0x7c8 */
	uint32 unused19;		/* 0x7cc */
	uint32 CAP_READ_DATA[4];	/* 0x7d0-0x7dc */
	uint32 unused20[8];		/* 0x7e0-0x7ff */

	SecureRangeCheckers SEC_RANGE_CHK;	/* 0x800 */

	uint32 SEC_INTR2_CPU_STATUS;	/* 0x900 */
	uint32 SEC_INTR2_CPU_SET;	/* 0x904 */
	uint32 SEC_INTR2_CPU_CLEAR;	/* 0x908 */
	uint32 SEC_INTR2_CPU_MASK_STATUS;	/* 0x90c */
	uint32 SEC_INTR2_CPU_MASK_SET;	/* 0x910 */
	uint32 SEC_INTR2_CPU_MASK_CLEAR;	/* 0x914 */
	uint32 unused21[10];		/* 0x918-0x93f */

	uint32 SEC_SRAM_REMAP_CTRL;	/* 0x940 */
	uint32 SEC_SRAM_REMAP_INIT;	/* 0x944 */
	uint32 SEC_SRAM_REMAP_LOG_0;	/* 0x948 */
	uint32 SEC_SRAM_REMAP_LOG_1;	/* 0x94c */
	uint32 unused22[428];		/* 0x950-0xfff */

	SdrCfg SDR_CFG;	                /* 0x1000 */
} MEMCControl;
#define MEMC ((volatile MEMCControl * const) MEMC_BASE)


#define IUDMA_MAX_CHANNELS          32

/*
** DMA Channel Configuration (1 .. 32)
*/
typedef struct DmaChannelCfg {
  uint32        cfg;                    /* (00) assorted configuration */
#define         DMA_ENABLE      0x00000001  /* set to enable channel */
#define         DMA_PKT_HALT    0x00000002  /* idle after an EOP flag is detected */
#define         DMA_BURST_HALT  0x00000004  /* idle after finish current memory burst */
  uint32        intStat;                /* (04) interrupts control and status */
  uint32        intMask;                /* (08) interrupts mask */
#define         DMA_BUFF_DONE   0x00000001  /* buffer done */
#define         DMA_DONE        0x00000002  /* packet xfer complete */
#define         DMA_NO_DESC     0x00000004  /* no valid descriptors */
#define         DMA_RX_ERROR    0x00000008  /* rxdma detect client protocol error */
  uint32        maxBurst;               /* (0C) max burst length permitted */
#define         DMA_DESCSIZE_SEL 0x00040000  /* DMA Descriptor Size Selection */
} DmaChannelCfg;

/*
** DMA State RAM (1 .. 16)
*/
typedef struct DmaStateRam {
  uint32        baseDescPtr;            /* (00) descriptor ring start address */
  uint32        state_data;             /* (04) state/bytes done/ring offset */
  uint32        desc_len_status;        /* (08) buffer descriptor status and len */
  uint32        desc_base_bufptr;       /* (0C) buffer descrpitor current processing */
} DmaStateRam;


/*
** DMA Registers
*/
typedef struct DmaRegs {
    uint32 controller_cfg;              /* (00) controller configuration */
#define DMA_MASTER_EN           0x00000001
#define DMA_FLOWC_CH1_EN        0x00000002
#define DMA_FLOWC_CH3_EN        0x00000004

    // Flow control Ch1
    uint32 flowctl_ch1_thresh_lo;           /* 004 */
    uint32 flowctl_ch1_thresh_hi;           /* 008 */
    uint32 flowctl_ch1_alloc;               /* 00c */
#define DMA_BUF_ALLOC_FORCE     0x80000000

    // Flow control Ch3
    uint32 flowctl_ch3_thresh_lo;           /* 010 */
    uint32 flowctl_ch3_thresh_hi;           /* 014 */
    uint32 flowctl_ch3_alloc;               /* 018 */

    // Flow control Ch5
    uint32 flowctl_ch5_thresh_lo;           /* 01C */
    uint32 flowctl_ch5_thresh_hi;           /* 020 */
    uint32 flowctl_ch5_alloc;               /* 024 */

    // Flow control Ch7
    uint32 flowctl_ch7_thresh_lo;           /* 028 */
    uint32 flowctl_ch7_thresh_hi;           /* 02C */
    uint32 flowctl_ch7_alloc;               /* 030 */

    uint32 ctrl_channel_reset;              /* 034 */
    uint32 ctrl_channel_debug;              /* 038 */
    uint32 reserved1;                       /* 03C */
    uint32 ctrl_global_interrupt_status;    /* 040 */
    uint32 ctrl_global_interrupt_mask;      /* 044 */

    // Unused words
    uint8 reserved2[0x200-0x48];

    // Per channel registers/state ram
    DmaChannelCfg chcfg[IUDMA_MAX_CHANNELS];/* (200-3FF) Channel configuration */
    union {
        DmaStateRam     s[IUDMA_MAX_CHANNELS];
        uint32          u32[4 * IUDMA_MAX_CHANNELS];
    } stram;                                /* (400-5FF) state ram */
} DmaRegs;

#define SAR_DMA ((volatile DmaRegs * const) SAR_DMA_BASE)
#define SW_DMA  ((volatile DmaRegs * const) SWITCH_DMA_BASE)

/*
** DMA Buffer
*/
typedef struct DmaDesc {
  union {
    struct {
        uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
        uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
    };
    uint32      word0;
  };

  uint32        address;                /* address of data */
} DmaDesc;

/*
** 16 Byte DMA Buffer
*/
typedef struct {
  union {
    struct {
        uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM        0x8000
#define          DMA_DESC_MULTICAST     0x4000
#define          DMA_DESC_BUFLENGTH     0x0fff
        uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
    };
    uint32      word0;
  };

  uint32        address;                 /* address of data */
  uint32        control;
#define         GEM_ID_MASK             0x001F
  uint32        reserved;
} DmaDesc16;

typedef struct usb_ctrl{
    uint32 setup;
#define USBH_IPP                (1<<5)
#define USBH_IOC                (1<<4)
#define USB_DEVICE_SEL          (1<<11)
    uint32 pll_ctl;
    uint32 fladj_value;
    uint32 bridge_ctl;
#define EHCI_ENDIAN_SWAP        (1<<3)  // A0/A1 only
#define EHCI_DATA_SWAP          (1<<2)  // A0/A1 only
#define OHCI_ENDIAN_SWAP        (1<<1)  // A0/A1 only
#define OHCI_DATA_SWAP          (1<<0)  // A0/A1 only
#define EHCI_SWAP_MODE_SHIFT    (2)     // Starting with B0
#define EHCI_SWAP_MODE_MASK     (0x000c)     // Starting with B0
#define EHCI_SWAP_MODE_BOTH     (1)     // Starting with B0
#define OHCI_SWAP_MODE_SHIFT    (0)     // Starting with B0
#define OHCI_SWAP_MODE_MASK     (0x0003)     // Starting with B0
#define OHCI_SWAP_MODE_BOTH     (1)     // Starting with B0
    uint32 spare1;
    uint32 mdio;
    uint32 mdio2;
    uint32 test_port_control;
    uint32 USBSimControl;
#define USBH_OHCI_MEM_REQ_DIS   (1<<1)
    uint32 usb_testctl;
    uint32 usb_testmon;
    uint32 utmi_ctl_1;
    uint32 spare2;
    uint32 usb_pm;
    uint32 usb_pm_status;
    uint32 spare4;
    uint32 pll_ldo_ctl;
    uint32 pll_ldo_pllbias;
    uint32 pll_afe_bg_cntl;
    uint32 afe_usbio_tst;
    uint32 pll_ndiv_frac;
    uint32 spare5;
} usb_ctrl;

#define USBH_CTRL ((volatile usb_ctrl * const) USBH_CFG_BASE)
#define USBH USBH_CTRL

typedef struct usb30_ctrl{
    uint32 setup;
    uint32 pll_ctl;
    uint32 spare1;
    uint32 bridge_ctl;
    uint32 spare2;
    uint32 mdio;
    uint32 mdio2;
    uint32 test_port_control;
    uint32 usb_simctl;
    uint32 spare3;
    uint32 usb_testmon;
    uint32 spare4;
    uint32 spare5;
    uint32 usb_pm;
    uint32 usb_pm_status;
    uint32 spare6;
    uint32 rsvd[8];
    uint32 usb30_ctl1;
#define PHY3_PLL_SEQ_START      (1<<4)
#define XHC_SOFT_RESETB         (1<<17)
#define USB3_IOC                (1<<28)
#define USB3_IPP                (1<<29)
    uint32 usb30_ctl2;
    uint32 usb30_ctl3;
    uint32 usb30_ctl4;
    uint32 usb30_pctl;
    uint32 usb30_ctl5;
    uint32 spare7;
} usb30_ctrl;

#define USB30H_CTRL ((volatile usb30_ctrl * const) USB30H_CFG_BASE)
/*
** SAR Registers
*/

#define SAR_TX_CTL_REGS (SAR_BASE + 0x00000060) /* SAR Tx Control Registers */
#define SAR_TX_CTL_REGS_SZ  0x00000020
#define SAR_RX_CTL_REGS (SAR_BASE + 0x00000080) /* SAR Rx Control Registers */
#define SAR_RX_CTL_REGS_SZ  0x00000030
#define SAR_RX_VCAM_REGS (SAR_BASE + 0x00000140) /* SAR  Rx ATM VPI_VCI CAM Table Reg Registers */
#define SAR_RX_VCAM_REGS_SZ  0x00000080
#define SAR_SHPR_REGS (SAR_BASE + 0x00000200) /* SAR Atm Shaper Source Shaping Table Registers */
#define SAR_SHPR_REGS_SZ  0x00000070
#define SAR_RX_PBUF_REGS (SAR_BASE + 0x00000300) /* SAR Rx Packet Buffer Control Registers */
#define SAR_RX_PBUF_REGS_SZ  0x00000060
#define SAR_MIB_REGS (SAR_BASE + 0x00000600) /* SAR  Atm MIB Counters Registers */
#define SAR_MIB_REGS_SZ  0x000000C0
#define SAR_RX_PAF_REGS (SAR_BASE + 0x00000800) /* SAR RxPaf Top Registers */
#define SAR_RX_PAF_REGS_SZ  0x00000100
#define SAR_RX_BOND_REGS (SAR_BASE + 0x00000900) /* SAR RxPaf Bonding Registers */
#define SAR_RX_BOND_REGS_SZ  0x000000C0
#define SAR_TMUEXT_REGS (SAR_BASE + 0x00001000) /* SAR Traffic Management Unit Extended Registers */
#define SAR_TMUEXT_REGS_SZ  0x00000600


/* Specific SAR Rx Control Registers - LED Configuration Register */
#define SAR_RX_CTL_LED_CFG (SAR_BASE + 0x000000AC) /* LED Configuration Register */
#define SARRXCTLLEDCFG ((volatile uint32 * const) (SAR_RX_CTL_LED_CFG))

#define SARLEDCFG_TEST                  0x000000100     /* LED Test bit. */
#define SARLEDCFG_BLINK_30MS            0x000000000     /* LED blink speed: 00 = 30 ms */
#define SARLEDCFG_BLINK_50MS            0x000000020     /* LED blink speed: 01 = 50 ms */
#define SARLEDCFG_BLINK_125MS           0x000000040     /* LED blink speed: 10 = 125 ms */
#define SARLEDCFG_BLINK_250MS           0x000000060     /* LED blink speed: 11 = 250 ms */
#define SARLEDCFG_LNK                   0x000000010     /* Link is established - set by software when ADSL link is established. */
#define SARLEDCFG_INT_LED               0x000000008     /* Set to enable using internal LED logic to drive INET_LED, otherwise use Periph LED logic control. */
#define SARLEDCFG_MODE_LINK             0x000000000     /* INET_LED Mode: 00 = Assert on ADSL Link Only (by setting LINK bit) */
#define SARLEDCFG_MODE_TRAFFIC          0x000000002     /* INET_LED Mode: 01 = Assert on Cell Activity */
#define SARLEDCFG_MODE_MELODYLINK       0x000000004     /* INET_LED Mode: 10 = ADSL Melody/Link Mode - blink slowly during training and solid on link */
#define SARLEDCFG_MODE_TRAFFICLINK      0x000000006     /* INET_LED Mode: 11 = Assert on ADSL Link, blink on ATM TX and RX cells traffic */
#define SARLEDCFG_LED_EN                0x000000001     /* ADSL LED Enable */



typedef struct EthSwMIBRegs {
    unsigned int TxOctetsLo;
    unsigned int TxOctetsHi;
    unsigned int TxDropPkts;
    unsigned int TxQoSPkts;
    unsigned int TxBroadcastPkts;
    unsigned int TxMulticastPkts;
    unsigned int TxUnicastPkts;
    unsigned int TxCol;
    unsigned int TxSingleCol;
    unsigned int TxMultipleCol;
    unsigned int TxDeferredTx;
    unsigned int TxLateCol;
    unsigned int TxExcessiveCol;
    unsigned int TxFrameInDisc;
    unsigned int TxPausePkts;
    unsigned int TxQoSOctetsLo;
    unsigned int TxQoSOctetsHi;
    unsigned int RxOctetsLo;
    unsigned int RxOctetsHi;
    unsigned int RxUndersizePkts;
    unsigned int RxPausePkts;
    unsigned int Pkts64Octets;
    unsigned int Pkts65to127Octets;
    unsigned int Pkts128to255Octets;
    unsigned int Pkts256to511Octets;
    unsigned int Pkts512to1023Octets;
    unsigned int Pkts1024to1522Octets;
    unsigned int RxOversizePkts;
    unsigned int RxJabbers;
    unsigned int RxAlignErrs;
    unsigned int RxFCSErrs;
    unsigned int RxGoodOctetsLo;
    unsigned int RxGoodOctetsHi;
    unsigned int RxDropPkts;
    unsigned int RxUnicastPkts;
    unsigned int RxMulticastPkts;
    unsigned int RxBroadcastPkts;
    unsigned int RxSAChanges;
    unsigned int RxFragments;
    unsigned int RxExcessSizeDisc;
    unsigned int RxSymbolError;
    unsigned int RxQoSPkts;
    unsigned int RxQoSOctetsLo;
    unsigned int RxQoSOctetsHi;
    unsigned int Pkts1523to2047;
    unsigned int Pkts2048to4095;
    unsigned int Pkts4096to8191;
    unsigned int Pkts8192to9728;
} EthSwMIBRegs;

#define ETHSWMIBREG ((volatile EthSwMIBRegs * const) (SWITCH_BASE + 0x2000))

/* Enet registers controlling rx iuDMA channel */
typedef struct EthSwQosIngressPortPriRegs{
    unsigned short pri_id_map[9];
} EthSwQosIngressPortPriRegs;

#define ETHSWQOSREG ((volatile EthSwQosIngressPortPriRegs * const) (SWITCH_BASE + 0x3050))

/* SAR registers controlling rx iuDMA channel */
typedef struct SarRxMuxRegs{
    unsigned int vcid0_qid;
    unsigned int vcid1_qid;
} SarRxMuxRegs;

#define SARRXMUXREG ((volatile SarRxMuxRegs * const) (SAR_BASE + 0x0400))


typedef struct EthSwRegs{
    byte port_traffic_ctrl[9]; /* 0x00 - 0x08 */
    byte reserved1[2]; /* 0x09 - 0x0a */
    byte switch_mode; /* 0x0b */
    unsigned short pause_quanta; /*0x0c */
    byte imp_port_state; /*0x0e */
    byte led_refresh; /* 0x0f */
    unsigned short led_function[2]; /* 0x10 */
    unsigned short led_function_map; /* 0x14 */
    unsigned short led_enable_map; /* 0x16 */
    unsigned short led_mode_map0; /* 0x18 */
    unsigned short led_function_map1; /* 0x1a */
    byte reserved2[5]; /* 0x1b - 0x20 */
    byte port_forward_ctrl; /* 0x21 */
    byte reserved3[2]; /* 0x22 - 0x23 */
    unsigned short protected_port_selection; /* 0x24 */
    unsigned short wan_port_select; /* 0x26 */
    unsigned int pause_capability; /* 0x28 */
    byte reserved4[3]; /* 0x2c - 0x2e */
    byte reserved_multicast_control; /* 0x2f */
    byte reserved5; /* 0x30 */
    byte txq_flush_mode_control; /* 0x31 */
    unsigned short ulf_forward_map; /* 0x32 */
    unsigned short mlf_forward_map; /* 0x34 */
    unsigned short mlf_impc_forward_map; /* 0x36 */
    unsigned short pause_pass_through_for_rx; /* 0x38 */
    unsigned short pause_pass_through_for_tx; /* 0x3a */
    unsigned short disable_learning; /* 0x3c */
    byte reserved6[2]; /* 0x3e */
    unsigned short mii_packet_size; /* 0x40 */
    byte reserved7[22]; /* 0x42 - 0x57 */
    byte port_state_override[8]; /* 0x58 - 0x5f */
    byte reserved8[4]; /* 0x60 - 0x63 */
    byte imp_rgmii_ctrl_p4; /* 0x64 */
    byte reserved17[3]; /* 0x65-0x67 */
    byte reserved9[4]; /* 0x68 - 0x6b */
    byte rgmii_timing_delay_p4; /* 0x6c */
    byte reserved16[3]; /* 0x6d-0x6f */
    byte reserved10[9]; /* 0x70 - 0x78 */
    byte software_reset; /* 0x79 */
    byte reserved11[6]; /* 0x7a - 0x7f */
    byte pause_frame_detection; /* 0x80 */
    byte reserved12[7]; /* 0x81 - 0x87 */
    byte fast_aging_ctrl; /* 0x88 */
    byte fast_aging_port; /* 0x89 */
    byte fast_aging_vid; /* 0x8a */
    byte reserved13[29]; /* 0x8b - 0xa7 */
    unsigned int iudma_ctrl; /*0xa8 */
    unsigned int rxfilt_ctrl; /*0xac */
    unsigned int mdio_ctrl; /*0xb0 */
    unsigned int mdio_data; /*0xb4 */
    byte reserved14[16]; /* 0xb6 - 0xc5 */
    unsigned short eee_ctrl_p3; /* c6 */
    unsigned short eee_ctrl_p4; /* c8 */
    unsigned short reserved18[3]; /*ca - ce*/
    unsigned short eee_tw_sys_tx_100; /* d0 */
    unsigned short eee_tw_sys_tx_1000; /* d2 */
    byte reserved15[12]; /* 0xd4 - 0xdf */
    unsigned int sw_mem_test; /*0xe0 */
} EthSwRegs;

#define ETHSWREG ((volatile EthSwRegs * const) SWITCH_BASE)


/*
** PCI-E
*/
#define UBUS2_PCIE
#define PCIE3_CORE
typedef struct PcieRegs{
  uint32 devVenID;
  uint16 command;
  uint16 status;
  uint32 revIdClassCode;
  uint32 headerTypeLatCacheLineSize;
  uint32 bar1;
  uint32 bar2;
  uint32 priSecBusNo;
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_MASK              0x00ff0000
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_SHIFT             16
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_MASK              0x0000ff00
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_SHIFT             8
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_PRI_BUS_NO_MASK              0x000000ff

  uint32 secStatusioBaseLimit;
  uint32 rcMemBaseLimit;
  uint32 rcPrefBaseLimit;
  uint32 rcPrefBaseHi;
  uint32 rcPrefLimitHi;
  uint32 rcIoBaseLimit;
  uint32 capPointer;
  uint32 expRomBase;
  uint32 bridgeCtrl;
  uint32 unused1[2];
  uint32 pm_cap;				/* 0x048 */
  uint32 pm_csr;				/* 0x04c */
  uint32 unused2[23];	
  /* PcieExpressCtrlRegs */
  uint16 pciExpressCap;			/* 0x0ac */		
  uint16 pcieCapabilitiy;
  uint32 deviceCapability;
  uint16 deviceControl;
  uint16 deviceStatus;
  uint32 linkCapability;
#define PCIE_RC_CFG_PCIE_LINK_CAPBILITY_MAX_LINK_SPEED_MASK        0xf
#define PCIE_RC_CFG_PCIE_LINK_CAPBILITY_MAX_LINK_SPEED_2500MBPS    1
#define PCIE_RC_CFG_PCIE_LINK_CAPBILITY_MAX_LINK_SPEED_5000MBPS    2
  uint16 linkControl;
  uint16 linkStatus;
  uint32 slotCapability;
  uint16 slotControl;
  uint16 slotStatus;
  uint16 rootControl;
  uint16 rootCap;
  uint32 rootStatus;
  uint32 deviceCapability2;
  uint16 deviceControl2;
  uint16 deviceStatus2;
  uint32 linkCapability2;
  uint16 linkControl2;
  uint16 linkStatus2;
  uint32 slotCapability2;
  uint16 slotControl2;
  uint16 slotStatus2;
  uint32 unused3[6];		/* 0x0e8 */

  /* PcieErrorRegs */
  uint16 advErrCapId;		/* 0x100 */
  uint16 advErrCapOff;
  uint32 ucErrStatus;
  uint32 ucorrErrMask;
  uint32 ucorrErrSevr;
  uint32 corrErrStatus;
  uint32 corrErrMask;
  uint32 advErrCapControl;
  uint32 headerLog1;
  uint32 headerLog2;
  uint32 headerLog3;
  uint32 headerLog4;
  uint32 rootErrorCommand;
  uint32 rootErrorStatus;
  uint16 rcCorrId;
  uint16 rcFatalNonfatalId;
  uint32 unused4[10];		/* 0x138 */

  /* PcieVcRegs */
  uint16 vcCapId;
  uint16 vcCapOffset;
  uint32 prtVcCapability;
  uint32 portVcCapability2;
  uint16 portVcControl;
  uint16 portVcCtatus;
  uint32 portArbStatus;
  uint32 vcRsrcControl;
  uint32 vcRsrcStatus;
  uint32 unused5[1];		/* 0x17c */
} PcieRegs;

typedef struct PcieRcCfgVendorRegs{
	uint32 vendorCap;
	uint32 specificHeader;
	uint32 specificReg1;
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR1_SHIFT			0	
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR2_SHIFT			2
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BAR3_SHIFT			4
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_WORD_ALIGN			0x0	
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_HWORD_ALIGN		0x1	
#define PCIE_RC_CFG_VENDOR_SPECIFIC_REG1_ENDIAN_MODE_BYTE_ALIGN			0x2
}PcieRcCfgVendorRegs;

typedef struct PcieBlk404Regs{
  uint32 unused;          /* 0x404 */
  uint32 config2;         /* 0x408 */
#define PCIE_IP_BLK404_CONFIG_2_BAR1_SIZE_MASK         0x0000000f
#define PCIE_IP_BLK404_CONFIG_2_BAR1_DISABLE           0
  uint32 config3;         /* 0x40c */
  uint32 pmDataA;         /* 0x410 */
  uint32 pmDataB;         /* 0x414 */
} PcieBlk404Regs;

typedef struct PcieBlk428Regs{
  uint32 vpdIntf;        /* 0x428 */
  uint16 unused_g;       /* 0x42c */
  uint16 vpdAddrFlag;    /* 0x42e */
  uint32 vpdData;        /* 0x430 */
  uint32 idVal1;         /* 0x434 */
  uint32 idVal2;         /* 0x438 */
  uint32 idVal3;         /* 0x43c */
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_MASK                    0xff000000
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_SHIFT                   24
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_MASK                     0x00ffffff
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_SHIFT                    16
#define PCIE_IP_BLK428_ID_VAL3_SUB_CLASS_CODE_SHIFT                 8

  uint32 idVal4;
  uint32 idVal5;
  uint32 unused_h;
  uint32 idVal6;
  uint32 msiData;
  uint32 msiAddr_h;
  uint32 msiAddr_l;
  uint32 unused_1[2];
  uint32 msiMask;
  uint32 msiPend;
  uint32 pmData_c;
  uint32 unused_2[20];
  uint32 msixControl;
  uint32 msixTblOffBir;
  uint32 msixPbaOffBit;
  uint32 unused_k;
  uint32 pcieCapability;
  uint32 deviceCapability;
  uint32 deviceControl;
  uint32 linkCapability;
#define PCIE_IP_BLK428_LINK_CAPBILITY_MAX_LINK_SPEED_MASK          0xf
#define PCIE_IP_BLK428_LINK_CAPBILITY_MAX_LINK_SPEED_2500MBPS      1
#define PCIE_IP_BLK428_LINK_CAPBILITY_MAX_LINK_SPEED_5000MBPS      2
  uint32 bar2Config;
  uint32 pcieDeviceCapability2;
  uint32 pcieLinkCapability2;
  uint32 pcieLinkControl;
  uint32 pcieLinkCapabilityRc;
  uint32 bar3Config;
  uint32 rootCap;
  uint32 devSerNumCapId;
  uint32 lowerSerNum;
  uint32 upperSerNum;
  uint32 advErrCap;
  uint32 pwrBdgtData0;
  uint32 pwrBdgtData1;
  uint32 pwrBdgtData2;
  uint32 pwdBdgtData3;
  uint32 pwrBdgtData4;
  uint32 pwrBdgtData5;
  uint32 pwrBdgtData6;
  uint32 pwrBdgtData7;
  uint32 ext2CapAddr;
  uint32 pwrBdgtData8;
  uint32 pwrBdgtCapability;
  uint32 vsecHdr;
  uint32 rcUserMemLo1;
  uint32 rcUserMemHi1;
  uint32 rcUserMemLo2;
  uint32 rcUserMemHi2;
  uint32 tphCap;
  uint32 resizebarCap;
  uint32 ariCap;
  uint32 initvf;
  uint32 vfOffset;
  uint32 vfBarReg;
  uint32 vfSuppPageSize;
  uint32 vfCap_en;
  uint32 vfMsixTblBirOff;
  uint32 vfMsixPbaOffBit;
  uint32 vfMsixControl;
  uint32 vfBar4Reg;
  uint32 pfInitvf;
  uint32 vfNsp;
  uint32 atsInldQueueDepth;
} PcieBlk428Regs;

typedef struct PcieBlk800Regs{
#define NUM_PCIE_BLK_800_CTRL_REGS  6
  uint32 tlControl[NUM_PCIE_BLK_800_CTRL_REGS];
  /* followed by others not used */

} PcieBlk800Regs;


typedef struct PcieBlk1000Regs{
#define NUM_PCIE_BLK_1000_PDL_CTRL_REGS  16
  uint32 pdlControl[NUM_PCIE_BLK_1000_PDL_CTRL_REGS];
  uint32 dlattnVec;
  uint32 dlAttnMask;
  uint32 dlStatus;        /* 0x1048 */
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_MASK                   0x00002000
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_SHIFT                  13
  uint32 dlTxChecksum;
  uint32 dlForcedUpdateGen1;
  uint32 dlReg_spare0;
  uint32 dlErrStatistic_ctl;
  uint32 dlErrStatistic; 
  uint32 reserved[40];
  uint32 mdioAddr;
  uint32 mdioWrData;
  uint32 mdioRdData;
  uint32 dlAteTlpHdr_0;
  uint32 dlAteTlpHdr_1;
  uint32 dlAteTlpHdr_2;
  uint32 dlAteTlpHdr_3;
  uint32 dlAteTlpCfg;
  uint32 dlAteTlpCtl;
  uint32 reserved1[183];
  uint32 dlRxPFcCl;
  uint32 dlRxCFcCl;
  uint32 dlRxAckNack;
  uint32 dlTxRxSeqnb;
  uint32 dlTxPFcAl;
  uint32 dlTxNpFcAl;
  uint32 regDlSpare;
  uint32 dlRegSpare;
  uint32 dlTxRxSeq;
  uint32 dlRxNpFcCl;
} PcieBlk1000Regs;

typedef struct PcieBlk1800Regs{
#define NUM_PCIE_BLK_1800_PHY_CTRL_REGS         8
  uint32 phyCtrl[NUM_PCIE_BLK_1800_PHY_CTRL_REGS];
#define REG_POWERDOWN_P1PLL_ENA                      (1<<12)
  uint32 phyErrorAttnVec;
  uint32 phyErrorAttnMask;
  uint32 reserved[2];
  uint32 phyCtl8;				/* 0x1830 */
  uint32 reserved1[243];
  uint32 phyReceivedMcpErrors; 	/* 0x1c00 */
  uint32 reserved2[3];
  uint32 phyTransmittedMcpErrors;/* 0x1c10 */
  uint32 reserved3[3];
  uint32 rxFtsLimit;			/* 0x1c20 */
  uint32 reserved4[46];
  uint32 ftsHist;				/* 0x1cd8 */
  uint32 phyGenDebug;
  uint32 phyRecoveryHist;
#define NUM_PCIE_BLK_1800_PHY_LTSSM_HIST_REGS   5
  uint32 phyltssmHist[NUM_PCIE_BLK_1800_PHY_LTSSM_HIST_REGS];
#define NUM_PCIE_BLK_1800_PHY_DBG_REGS          11
  uint32 phyDbg[NUM_PCIE_BLK_1800_PHY_DBG_REGS];
  uint32 phyAteLoopbkInfo;		/* 0x1d30 */
  uint32 reserved5[55];
#define NUM_PCIE_BLK_1800_PHY_DBG_CLK_REGS      4
  uint32 phyDbgClk[NUM_PCIE_BLK_1800_PHY_DBG_CLK_REGS]; /* 0x1e10 */
} PcieBlk1800Regs;

typedef struct PcieMiscRegs{
  uint32 reset_ctrl;                    /* 4000 Reset Control Register */
  uint32 eco_ctrl_core;                 /* 4004 ECO Core Reset Control Register */
  uint32 misc_ctrl;                     /* 4008 MISC Control Register */
#define PCIE_MISC_CTRL_MAX_BURST_SIZE_128B                         (1<<20)
#define PCIE_MISC_CTRL_BURST_ALIGN                                 (1<<19)
#define PCIE_MISC_CTRL_CFG_READ_UR_MODE                            (1<<13)
#define PCIE_MISC_CTRL_PCIE_IN_WR_COMBINE                          (1<<11)
#define PCIE_MISC_CTRL_PCIE_RCB_MPS_MODE                           (1<<10)
#define PCIE_MISC_CTRL_PCIE_RCB_64B_MODE                           (1<<7)

  uint32 cpu_2_pcie_mem_win0_lo;        /* 400c CPU to PCIe Memory Window 0 Low */
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK              0xfff00000
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_SHIFT             20
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_NO_SWAP               0
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_HALF_WORD_SWAP        1
#define PCIE_MISC_CPU_2_PCIE_MEM_ENDIAN_MODE_HALF_BYTE_SWAP        2
  uint32 cpu_2_pcie_mem_win0_hi;        /* 4010 CPU to PCIe Memory Window 0 High */
  uint32 cpu_2_pcie_mem_win1_lo;        /* 4014 CPU to PCIe Memory Window 1 Low */
  uint32 cpu_2_pcie_mem_win1_hi;        /* 4018 CPU to PCIe Memory Window 1 High */
  uint32 cpu_2_pcie_mem_win2_lo;        /* 401c CPU to PCIe Memory Window 2 Low */
  uint32 cpu_2_pcie_mem_win2_hi;        /* 4020 CPU to PCIe Memory Window 2 High */
  uint32 cpu_2_pcie_mem_win3_lo;        /* 4024 CPU to PCIe Memory Window 3 Low */
  uint32 cpu_2_pcie_mem_win3_hi;        /* 4028 CPU to PCIe Memory Window 3 High */
  uint32 rc_bar1_config_lo;             /* 402c RC BAR1 Configuration Low Register */
#define  PCIE_MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK             0xfff00000
#define  PCIE_MISC_RC_BAR_CONFIG_LO_SIZE_256MB                     0xd
#define  PCIE_MISC_RC_BAR_CONFIG_LO_SIZE_MAX                       0x11     /* max is 4GB */
  uint32 rc_bar1_config_hi;             /* 4030 RC BAR1 Configuration High Register */
  uint32 rc_bar2_config_lo;             /* 4034 RC BAR2 Configuration Low Register */
  uint32 rc_bar2_config_hi;             /* 4038 RC BAR2 Configuration High Register */
  uint32 rc_bar3_config_lo;             /* 403c RC BAR3 Configuration Low Register */
  uint32 rc_bar3_config_hi;             /* 4040 RC BAR3 Configuration High Register */
  uint32 msi_bar_config_lo;             /* 4044 Message Signaled Interrupt Base Address Low Register */
  uint32 msi_bar_config_hi;             /* 4048 Message Signaled Interrupt Base Address High Register */
  uint32 msi_data_config;               /* 404c Message Signaled Interrupt Data Configuration Register */
  uint32 rc_bad_address_lo;             /* 4050 RC Bad Address Register Low */
  uint32 rc_bad_address_hi;             /* 4054 RC Bad Address Register High */
  uint32 rc_bad_data;                   /* 4058 RC Bad Data Register */
  uint32 rc_config_retry_timeout;       /* 405c RC Configuration Retry Timeout Register */
  uint32 eoi_ctrl;                      /* 4060 End of Interrupt Control Register */
  uint32 pcie_ctrl;                     /* 4064 PCIe Control */
  uint32 pcie_status;                   /* 4068 PCIe Status */
  uint32 revision;                      /* 406c PCIe Revision */
  uint32 cpu_2_pcie_mem_win0_base_limit;/* 4070 CPU to PCIe Memory Window 0 base/limit */
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK        0xfff00000
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT       20
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_MASK         0x0000fff0
#define PCIE_MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT        4
  uint32 cpu_2_pcie_mem_win1_base_limit;/* 4074 CPU to PCIe Memory Window 1 base/limit */
  uint32 cpu_2_pcie_mem_win2_base_limit;/* 4078 CPU to PCIe Memory Window 2 base/limit */
  uint32 cpu_2_pcie_mem_win3_base_limit;/* 407c CPU to PCIe Memory Window 3 base/limit */
  uint32 cpu_2_pcie_mem_win0_base_hi;
  uint32 cpu_2_pcie_mem_win0_limit_hi;
  uint32 cpu_2_pcie_mem_win1_base_hi;
  uint32 cpu_2_pcie_mem_win1_limit_hi;
  uint32 cpu_2_pcie_mem_win2_base_hi;
  uint32 cpu_2_pcie_mem_win2_limit_hi;
  uint32 cpu_2_pcie_mem_win3_base_hi;
  uint32 cpu_2_pcie_mem_win3_limit_hi;
  uint32 misc_ctrl_1;
  uint32 ubus_ctrl;
  uint32 ubus_timeout;
  uint32 ubus_bar1_config_remap;
#define  PCIE_MISC_UBUS_BAR_CONFIG_OFFSET_MASK                      0xfff00000
#define  PCIE_MISC_UBUS_BAR_CONFIG_ACCESS_EN                        1
  uint32 ubus_bar1_config_remap_hi;
  uint32 ubus_bar2_config_remap;
  uint32 ubus_bar2_config_remap_hi;
  uint32 ubus_bar3_config_remap;
  uint32 ubus_bar3_config_remap_hi;
  uint32 ubus_status;
} PcieMiscRegs;

typedef struct PcieMiscPerstRegs{
  uint32 perst_eco_ctrl_perst;          /* 4100 ECO PCIE Reset Control Register */
  uint32 perst_eco_cce_status;          /* 4104 Config Copy Engine Status */
} PcieMiscPerstRegs;

typedef struct PcieMiscHardRegs{
  uint32 hard_eco_ctrl_hard;            /* 4200 ECO Hard Reset Control Register */
  uint32 hard_pcie_hard_debug;          /* 4204 PCIE Hard Debug Register */
} PcieMiscHardRegs;

typedef struct PcieL2IntrControl{
  uint32 Intr2CpuStatus;
  uint32 Intr2CpuSet;
  uint32 Intr2CpuClear;
  uint32 Intr2CpuMask_status;
  uint32 Intr2CpuMask_set;
  uint32 Intr2CpuMask_clear;
  uint32 Intr2PciStatus;
  uint32 Intr2PciSet;
  uint32 Intr2PciClear;
  uint32 Intr2PciMask_status;
  uint32 Intr2PciMask_set;
  uint32 Intr2PciMask_clear;
} PcieL2IntrControl;

typedef struct PcieDMAregs{
  uint32 TxFirstDesc_l_AddrList0;
  uint32 TxFirstDesc_u_AddrList0;
  uint32 TxFirstDesc_l_AddrList1;
  uint32 TxFirstDesc_u_AddrList1;
  uint32 TxSwDescListCtrlSts;
  uint32 TxWakeCtrl;
  uint32 TxErrorStatus;
  uint32 TxList0CurDesc_l_Addr;
  uint32 TxList0CurDesc_u_Addr;
  uint32 TxList0CurByteCnt;
  uint32 TxList1CurDesc_l_Addr;
  uint32 TxList1CurDesc_u_Addr;
  uint32 TxList1CurByteCnt;
  uint32 RxFirstDesc_l_AddrList0;
  uint32 RxFirstDesc_u_AddrList0;
  uint32 RxFirstDesc_l_AddrList1;
  uint32 RxFirstDesc_u_AddrList1;
  uint32 RxSwDescListCtrlSts;
  uint32 RxWakeCtrl;
  uint32 RxErrorStatus;
  uint32 RxList0CurDesc_l_Addr;
  uint32 RxList0CurDesc_u_Addr;
  uint32 RxList0CurByteCnt;
  uint32 RxList1CurDesc_l_Addr;
  uint32 RxList1CurDesc_u_Addr;
  uint32 RxList1CurByteCnt;
  uint32 Dma_debug_options_reg;
  uint32 ReadChannelErrorStatus;
} PcieDMAregs;

typedef struct PcieUBUSL2IntrControl{
  uint32 UBUSIntr2CPUStatus;
  uint32 UBUSIntr2CPUSet;
  uint32 UBUSIntr2CPUClear;
  uint32 UBUSIntr2CPUMaskStatus;
  uint32 UBUSIntr2CPUMaskSet;
  uint32 UBUSIntr2CPUMaskClear;
  uint32 UBUSIntr2PCIStatus;
  uint32 UBUSIntr2PCISet;
  uint32 UBUSIntr2PCIClear;
  uint32 UBUSIntr2PCIMaskStatus;
  uint32 UBUSIntr2PCIMaskSet;
  uint32 UBUSIntr2PCIMaskClear;
} PcieUBUSL2IntrControl;

typedef struct PcieIPIL2IntrControl{
  uint32 IPIIntr2CPUStatus;
  uint32 IPIIntr2CPUSet;
  uint32 IPIIntr2CPUClear;
  uint32 IPIIntr2CPUMask_status;
  uint32 IPIIntr2CPUMask_set;
  uint32 IPIIntr2CPUMask_clear;
  uint32 IPIIntr2PCIStatus;
  uint32 IPIIntr2PCISet;
  uint32 IPIIntr2PCIClear;
  uint32 IPIIntr2PCIMask_status;
  uint32 IPIIntr2PCIMask_set;
  uint32 IPIIntr2PCIMask_clear;
} PcieIPIL2IntrControl;

typedef struct PciePcieL1Intr1Regs{
  uint32 status;

  uint32 maskStatus;
  uint32 maskSet;
  uint32 maskClear;  
} PciePcieL1Intr1Regs;

typedef struct PcieCpuL1Intr1Regs{
  uint32 status;
#define PCIE_CPU_INTR1_IPI_CPU_INTR                                  (1<<8)
#define PCIE_CPU_INTR1_PCIE_UBUS_CPU_INTR                            (1<<7)
#define PCIE_CPU_INTR1_PCIE_NMI_CPU_INTR                             (1<<6)
#define PCIE_CPU_INTR1_PCIE_INTR_CPU_INTR                            (1<<5)
#define PCIE_CPU_INTR1_PCIE_INTD_CPU_INTR                            (1<<4)
#define PCIE_CPU_INTR1_PCIE_INTC_CPU_INTR                            (1<<3)
#define PCIE_CPU_INTR1_PCIE_INTB_CPU_INTR                            (1<<2)
#define PCIE_CPU_INTR1_PCIE_INTA_CPU_INTR                            (1<<1)
#define PCIE_CPU_INTR1_PCIE_ERR_ATTN_CPU_INTR                        (1<<0)
  uint32 maskStatus;
  uint32 maskSet;
  uint32 maskClear;  
} PcieCpuL1Intr1Regs;

typedef struct PcieExtCfgRegs{
  uint32 index;
#define PCIE_EXT_CFG_BUS_NUM_MASK                                     0x0ff00000
#define PCIE_EXT_CFG_BUS_NUM_SHIFT                                    20
#define PCIE_EXT_CFG_DEV_NUM_MASK                                     0x000f0000
#define PCIE_EXT_CFG_DEV_NUM_SHIFT                                    15
#define PCIE_EXT_CFG_FUNC_NUM_MASK                                    0x00007000
#define PCIE_EXT_CFG_FUNC_NUM_SHIFT                                   12
  uint32 data;
  uint32 scratch;
} PcieExtCfgRegs;

#define PCIEH                         ((volatile uint32 * const) PCIE_BASE)
#define PCIEH_REGS                    ((volatile PcieRegs * const) PCIE_BASE)
#define PCIEH_RC_CFG_VENDOR_REGS      ((volatile PcieRcCfgVendorRegs * const) \
                                        (PCIE_BASE+0x180)) 
#define PCIEH_BLK_404_REGS            ((volatile PcieBlk404Regs * const) \
                                        (PCIE_BASE+0x404))
#define PCIEH_BLK_428_REGS            ((volatile PcieBlk428Regs * const) \
                                        (PCIE_BASE+0x428))
#define PCIEH_BLK_800_REGS            ((volatile PcieBlk800Regs * const) \
                                        (PCIE_BASE+0x800))                                        
#define PCIEH_BLK_1000_REGS           ((volatile PcieBlk1000Regs * const) \
                                        (PCIE_BASE+0x1000))
#define PCIEH_BLK_1800_REGS           ((volatile PcieBlk1800Regs * const) \
                                        (PCIE_BASE+0x1800))
#define PCIEH_MISC_REGS               ((volatile PcieMiscRegs * const)  \
                                        (PCIE_BASE+0x4000))
#define PCIEH_MISC_PERST_REGS         ((volatile PcieMiscPerstRegs * const)  \
                                        (PCIE_BASE+0x4100))
#define PCIEH_MISC_HARD_REGS         ((volatile PcieMiscHardRegs * const)  \
                                        (PCIE_BASE+0x4200))
#define PCIEH_CPU_INTR1_REGS         ((volatile PcieCpuL1Intr1Regs * const)  \
                                        (PCIE_BASE+0x9400))
#define PCIEH_PCIE_EXT_CFG_REGS      ((volatile PcieExtCfgRegs * const)  \
                                        (PCIE_BASE+0x9000))
#define PCIEH_DEV_OFFSET              0x8000                                                                           

#define PCIEH_RC_CFG_PRIV0            PCIEH_BLK_404_REGS
#define PCIEH_RC_CFG_PRIV1            PCIEH_BLK_428_REGS
#define PCIEH_RC_TL                   PCIEH_BLK_800_REGS
#define PCIEH_RC_DL                   PCIEH_BLK_1000_REGS
#define PCIEH_EXT_CFG_DATA            PCIEH_DEV_OFFSET

#define PCIEH_MEM1_BASE               0x10600000
#define PCIEH_MEM1_SIZE               0x00100000

#define PCIEH_MEM2_BASE               0xa0000000
#define PCIEH_MEM2_SIZE               0x20000000

#define DDR_UBUS_ADDRESS_BASE         0

typedef struct Jtag_Otp {
   uint32 ctrl0;           /* 0x00 */
#define JTAG_OTP_CTRL_ACCESS_MODE       (0x3 << 22)
#define JTAG_OTP_CTRL_PROG_EN           (1 << 21)
#define JTAG_OTP_CTRL_START             (1 << 0)
   uint32 ctrl1;           /* 0x04 */
#define JTAG_OTP_CTRL_CPU_MODE          (1 << 0)
   uint32 ctrl2;           /* 0x08 */
   uint32 ctrl3;           /* 0x0c */
   uint32 ctrl4;           /* 0x10 */
   uint32 status0;         /* 0x14 */
   uint32 status1;         /* 0x18 */
#define JTAG_OTP_STATUS_1_CMD_DONE      (1 << 1)
} Jtag_Otp;

#define JTAG_OTP ((volatile Jtag_Otp * const) JTAG_OTP_BASE)

#define BTRM_OTP_READ_TIMEOUT_CNT               0x10000

/* row 10 */
#define OTP_BITS_320_351_ROW                    10
#define OTP_BRCM_FEATURE_DISABLE_ROW            OTP_BITS_320_351_ROW
#define OTP_BRCM_TP1_DISABLE_SHIFT              25
#define OTP_BRCM_TP1_DISABLE_MASK               (1<<OTP_BRCM_TP1_DISABLE_SHIFT)

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

typedef struct dslphy_afe {
   uint32 unused1[2];        /* 0x200 */
   uint32 RbusAnalogStatus;  /* 0x208 */
   uint32 unused2[2];        /* 0x20c */
   uint32 RbusAfeLdCtrl;     /* 0x214 */
   uint32 unused3[58];       /* 0x21c */
   uint32 AfeReg[2];         /* 0x300 */
   uint32 ClkgenReg[2];      /* 0x308 */
   uint32 RxReg[4];          /* 0x310 */
   uint32 RxadcReg[5];       /* 0x320 */
   uint32 unused4[3];        /* 0x334 */
   uint32 TxReg[2];          /* 0x340 */
   uint32 LdoReg[2];         /* 0x348 */
   uint32 BgBiasReg[12];     /* 0x350 */
} dslphy_afe;
#define DSLPHY_AFE ((volatile dslphy_afe  * const) DSLPHY_AFE_BASE)

#endif

#ifdef __cplusplus
}
#endif

#endif

