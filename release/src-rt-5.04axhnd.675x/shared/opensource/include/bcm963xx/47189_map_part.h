/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

#ifndef __BCM47189_MAP_PART_H
#define __BCM47189_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "bcm_io_map.h"

#define	VENDOR_BROADCOM		0x14e4

#define	BCM43217_CHIP_ID	43217		/* 43217 chip id (OTP chipid) */
#define	BCM43227_CHIP_ID	43227		/* 43227 chipcommon chipid */

#define CHIP_FAMILY_ID_HEX      0x47189

#define ALP_CLOCK_47189         40000000

#ifndef __ASSEMBLER__
enum
{
    CHIPCOMMON_IDX,
    WRAP_IDX,
    SCU_IDX,
    FLASH_IDX,
    GCI_IDX,
    LAST_IDX
};
#endif

/* Physical addresses */
#define CHIPCOMMON_PHYS_BASE		0x18000000
#define CHIPCOMMON_SIZE			0x14000

#define MISC_OFFSET		0
#define JTAG_MASTER_OFFSET      0x30
#define SERIAL_FLASH_OFFSET     0x40
#define GPIO_WATCHDOG_OFFSET    0x58
#define CLOCK_CONTROL_OFFSET    0x80
#define UART_OFFSET             0x300
#define PMUCC_OFFSET		0x600
#define WLAN_MAC_OFFSET         0x1000
#define PCIE_GEN2_OFFSET        0x2000
#define ARM_CA7_OFFSET          0x3000
#define USB_2_0_HOST_OFFSET     0x4000
#define ENET_CORE0_OFFSET       0x5000
#define ENET_CORE1_OFFSET       0xB000
#define IS2_BASE_OFFSET         0x6000
#define DDR_CTRL_OFFSET         0x7000
#define NAND_FLASH_CTRL_OFFSET  0x8000
#define ENET_CORE1_OFFSET       0xB000
#define USB_1_0_HOST_OFFSET     0xD000
#define GCI_OFFSET              0x10000
#define PMU_OFFSET              0x12000


#define MISC_PHYS_BASE			(CHIPCOMMON_PHYS_BASE +  MISC_OFFSET)
#define JTAG_MASTER_PHYS_BASE		(CHIPCOMMON_PHYS_BASE + JTAG_MASTER_OFFSET)
#define SERIAL_FLASH_PHYS_BASE		(CHIPCOMMON_PHYS_BASE + SERIAL_FLASH_OFFSET)
#define GPIO_WATCHDOG_PHYS_BASE		(CHIPCOMMON_PHYS_BASE + GPIO_WATCHDOG_OFFSET)
#define CLOCK_CONTROL_PHYS_BASE		(CHIPCOMMON_PHYS_BASE + CLOCK_CONTROL_OFFSET)
#define UART_PHYS_BASE			(CHIPCOMMON_PHYS_BASE + UART_OFFSET)
#define UART0_PHYS_BASE			UART_PHYS_BASE
#define PCIE_GEN2_PHYS_BASE		(CHIPCOMMON_PHYS_BASE + PCIE_GEN2_OFFSET)
#define USB_EHCI_PHYS_BASE      (CHIPCOMMON_PHYS_BASE + USB_2_0_HOST_OFFSET)
#define USB_OHCI_PHYS_BASE      (CHIPCOMMON_PHYS_BASE + USB_1_0_HOST_OFFSET)
#define PMUCC_PHYS_BASE			(CHIPCOMMON_PHYS_BASE + PMUCC_OFFSET)
/* Wrapper space base */
#define WRAP_PHYS_BASE			0x18100000
#define WRAP_SIZE			0x14000
/* Other cores */
#define GCI_SIZE			0x2000
#define GCI_PHYS_BASE			(CHIPCOMMON_PHYS_BASE + GCI_OFFSET)
#define PMU_PHYS_BASE			(CHIPCOMMON_PHYS_BASE + PMU_OFFSET + 0x600)

#define PCIE_GEN2_OWIN_PHYS_BASE	0x10000000
#define PCIE_GEN2_OWIN_PHYS_SIZE	0x8000000
#define	PCIE_GEN2_CLK_CONTROL		0x0
#define	PCIE_GEN2_PCIE_EXT_CFG_ADDR	0x120
#define	PCIE_GEN2_PCIE_EXT_CFG_DATA	0x124
#define	PCIE_GEN2_PCIE_CFG_ADDR		0x1f8
#define	PCIE_GEN2_PCIE_CFG_DATA		0x1fc
#define	PCIE_GEN2_PCIE_SYS_RC_INTX_EN	0x330
#define	PCIE_GEN2_PCIE_HDR_OFF		0x400
/* 64-bit in-bound mapping windows for func 0..3 */
#define	PCIE_GEN2_IMAP1(f)		(0xc80+((f)<<3))
#define	PCIE_GEN2_IMAP2(f)		(0xcc0+((f)<<3))
/* 64-bit in-bound address range n=0..2 */
#define	PCIE_GEN2_IARR(n)		(0xd00+((n)<<3))
/* 64-bit out-bound address filter n=0..2 */


/* 64-bit out-bound address filter n=0..2 */
#define	PCIE_GEN2_OARR(n)		(0xd20+((n)<<3))
/* 64-bit out-bound mapping windows n=0..2 */
#define	PCIE_GEN2_OMAP(n)		(0xd40+((n)<<3))



#define SI_CORE_SIZE			0x1000
#define SI_MAXCORES			32

#define WLAN_MAC_WRAP_PHYS_BASE              (WRAP_PHYS_BASE + WLAN_MAC_OFFSET)
#define PCIE_GEN2_WRAP_PHYS_BASE             (WRAP_PHYS_BASE + PCIE_GEN2_OFFSET)
#define ARM_CA7_WRAP_PHYS_BASE               (WRAP_PHYS_BASE + ARM_CA7_OFFSET)
#define USB_2_0_HOST_WRAP_PHYS_BASE          (WRAP_PHYS_BASE + USB_2_0_HOST_OFFSET)
#define ENET_CORE0_WRAP_PHYS_BASE            (WRAP_PHYS_BASE + ENET_CORE0_OFFSET)
#define IS2_WRAP_PHYS_BASE                   (WRAP_PHYS_BASE + IS2_BASE_OFFSET)
#define DDR_CTRL_WRAP_PHYS_BASE              (WRAP_PHYS_BASE + DDR_CTRL_OFFSET)
#define NAND_FLASH_CTRL_WRAP_PHYS_BASE       (WRAP_PHYS_BASE + NAND_FLASH_CTRL_OFFSET)
#define ENET_CORE1_WRAP_PHYS_BASE            (WRAP_PHYS_BASE + ENET_CORE1_OFFSET)
#define GCI_WRAP_PHYS_BASE                   (WRAP_PHYS_BASE + GCI_OFFSET)
#define PMU_WRAP_PHYS_BASE                   (WRAP_PHYS_BASE + PMU_OFFSET)




#define AIRC_RESET              1
/* Common core control flags */
#define SICF_BIST_EN            0x8000
#define SICF_PME_EN             0x4000
#define SICF_CORE_BITS          0x3ffc
#define SICF_FGC                0x0002
#define SICF_CLOCK_EN           0x0001

#define SPI_FLASH_PHYS_BASE	0x1c000000
#define SPI_FLASH_SIZE		0x1000000



/* Dummy flash definitions */
#define NAND_REG_BASE           NAND_FLASH_CTRL_BASE
#define NAND_CACHE_BASE         NAND_REG_BASE+0x400 

/* Additional dummy defined */
#define SCU_PHYS_BASE           0x8001e000
#define SCU_SIZE		0x100 //temp


#define MISC_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,MISC_OFFSET)
#define JTAG_MASTER_BASE		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,JTAG_MASTER_OFFSET)
#define SERIAL_FLASH_BASE		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,SERIAL_FLASH_OFFSET)
#define GPIO_WATCHDOG_BASE		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,GPIO_WATCHDOG_OFFSET)
#define CLOCK_CONTROL_BASE		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,CLOCK_CONTROL_OFFSET)
#define UART_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,UART_OFFSET)
#define UART0_BASE			UART_BASE
#define PMUCC_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,PMUCC_OFFSET)

#define WLANMAC_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,WLAN_MAC_OFFSET)
#define PCIEGEN2_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,PCIE_GEN2_OFFSET)
#define ARM_CA7_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,ARM_CA7_OFFSET)
#define USB_2_0_HOST_BASE 		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,USB_2_0_HOST_OFFSET)
#define ENET_CORE0_BASE 		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,ENET_CORE0_OFFSET)
#define ENET_CORE1_BASE 		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,ENET_CORE1_OFFSET)
#define IS2_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,IS2_OFFSET)
#define DDR_CTRL_BASE 			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,DDR_CTRL_OFFSET)
#define NAND_FLASH_CTRL_BASE		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,NAND_FLASH_CTRL_OFFSET)
#define ENET_CORE1_BASE 		BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,ENET_CORE1_OFFSET)
#define PMU_BASE			BCM_IO_MAP(CHIPCOMMON_IDX, CHIPCOMMON_PHYS_BASE,PMU_OFFSET+0x600)

#define SPI_FLASH_BASE      		BCM_IO_MAP(FLASH_IDX, SPI_FLASH_PHYS_BASE,0x00)


#define WLAN_MAC_WRAP_BASE              BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, WLAN_MAC_OFFSET)
#define PCIE_GEN2_WRAP_BASE             BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, PCIE_GEN2_OFFSET)
#define ARM_CA7_WRAP_BASE               BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, ARM_CA7_OFFSET)
#define USB_2_0_HOST_WRAP_BASE          BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, USB_2_0_HOST_OFFSET)
#define ENET_CORE0_WRAP_BASE            BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, ENET_CORE0_OFFSET)
#define IS2_WRAP_BASE                   BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, IS2_BASE_OFFSET)
#define DDR_CTRL_WRAP_BASE              BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, DDR_CTRL_OFFSET)
#define NAND_FLASH_CTRL_WRAP_BASE       BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, NAND_FLASH_CTRL_OFFSET)
#define ENET_CORE1_WRAP_BASE            BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, ENET_CORE1_OFFSET)
#define PMU_WRAP_BASE                   BCM_IO_MAP(WRAP_IDX, WRAP_PHYS_BASE, PMU_OFFSET)
#define GCI_REG_BASE                    BCM_IO_MAP(GCI_IDX,  GCI_PHYS_BASE, 0)

#ifndef __ASSEMBLER__

#ifdef __BOARD_DRV_ARMV7__
BCM_IO_BLOCKS bcm_io_blocks[] =
{
    {CHIPCOMMON_IDX, CHIPCOMMON_SIZE, CHIPCOMMON_PHYS_BASE},
    {WRAP_IDX, WRAP_SIZE, WRAP_PHYS_BASE},
    {SCU_IDX, SCU_SIZE, SCU_PHYS_BASE},
    {FLASH_IDX, SPI_FLASH_SIZE, SPI_FLASH_PHYS_BASE},
    {GCI_IDX, GCI_SIZE, GCI_PHYS_BASE}
};
unsigned long bcm_io_block_address[LAST_IDX];
#else
extern BCM_IO_BLOCKS bcm_io_blocks[];
extern unsigned long bcm_io_block_address[];
#endif



/*
 * MISC Registers
 */
typedef struct MiscRegs1 {
    uint32 chipid;              /* 0x0 */
#define CID_ID_MASK             0x0000ffff      /* Chip Id mask */
#define CID_REV_MASK            0x000f0000      /* Chip Revision mask */
#define CID_REV_SHIFT           16              /* Chip Revision shift */
#define CID_PKG_MASK            0x00f00000      /* Package Option mask */
#define CID_PKG_SHIFT           20              /* Package Option shift */
#define CID_CC_MASK             0x0f000000      /* CoreCount (corerev >= 4) */
#define CID_CC_SHIFT            24
#define CID_TYPE_MASK           0xf0000000      /* Chip Type */
#define CID_TYPE_SHIFT          28

    uint32 capabilities;
#define CC_CAP_UARTS_MASK       0x00000003      /* Number of UARTs */
#define CC_CAP_MIPSEB           0x00000004      /* MIPS is in big-endian mode */
#define CC_CAP_UCLKSEL          0x00000018      /* UARTs clock select */
#define CC_CAP_UINTCLK          0x00000008      /* UARTs are driven by internal divided clock */
#define CC_CAP_UARTGPIO         0x00000020      /* UARTs own GPIOs 15:12 */
#define CC_CAP_EXTBUS_MASK      0x000000c0      /* External bus mask */
#define CC_CAP_EXTBUS_NONE      0x00000000      /* No ExtBus present */
#define CC_CAP_EXTBUS_FULL      0x00000040      /* ExtBus: PCMCIA, IDE & Prog */
#define CC_CAP_EXTBUS_PROG      0x00000080      /* ExtBus: ProgIf only */
#define CC_CAP_FLASH_MASK       0x00000700      /* Type of flash */
#define CC_CAP_PLL_MASK         0x00038000      /* Type of PLL */
#define CC_CAP_PWR_CTL          0x00040000      /* Power control */
#define CC_CAP_OTPSIZE          0x00380000      /* OTP Size (0 = none) */
#define CC_CAP_OTPSIZE_SHIFT    19              /* OTP Size shift */
#define CC_CAP_OTPSIZE_BASE     5               /* OTP Size base */
#define CC_CAP_JTAGP            0x00400000      /* JTAG Master Present */
#define CC_CAP_ROM              0x00800000      /* Internal boot rom active */
#define CC_CAP_BKPLN64          0x08000000      /* 64-bit backplane */
#define CC_CAP_PMU              0x10000000      /* PMU Present, rev >= 20 */
#define CC_CAP_ECI              0x20000000      /* ECI Present, rev >= 21 */
#define CC_CAP_SROM             0x40000000      /* Srom Present, rev >= 32 */
#define CC_CAP_NFLASH           0x80000000      /* Nand flash present, rev >= 35 */
#define CC_CAP2_SECI            0x00000001      /* SECI Present, rev >= 36 */
#define CC_CAP2_GSIO            0x00000002      /* GSIO (spi/i2c) present, rev >= 37 */

#define CC_CORECONTROL            0x18000008
    uint32 corecontrol;
#define CC_CTRL_UART_CLK_OVR      0x00000001
#define CC_CTRL_SYNC_CLK_OUT_EN   0x00000002
#define CC_CTRL_GPIO_ASYNC_INT_EN 0x00000004
#define CC_CTRL_UART_CLK_EN       0x00000008
#define CC_CTRL_OTP_CLK_EN        0x00000010
#define CC_CTRL_SPROM_CLK_EN      0x00000011
    uint32 bist;

    /* OTP */
    uint32 otpstatus;           /* 0x10 */
    uint32 otpcontrol;
    uint32 otpprog;
    uint32 otplayout;

    /* Interrupt control */
    uint32 intstatus;           /* 0x20 */
    uint32 intmask;

    /* Chip specific regs */
    uint32 chipcontrol;         /* 0x28 */
    uint32 chipstatus;
} MiscRegs1;


#define MISC ((volatile MiscRegs1 * const) MISC_BASE)

/*
 * JTAG Master registers
 */
typedef struct JtagMaster {
    uint32 jtagcmd;
    uint32 jtagir;
    uint32 jtagdr;
    uint32 jtagctrl;
} JTagMaster;

#define JTAG_MASTER ((volatile JTagMaster * const) JTAG_MASTER_BASE)

/*
 * Serial flash interface
 */
typedef struct SerialFlash {
    uint32 flashcontrol;        /* 0x40 */
    uint32 flashaddress;
    uint32 flashdata;
    uint32 otplayoutextension;
} SerialFlash;

#define SERIAL_FLASH ((volatile SerialFlash * const) SERIAL_FLASH_BASE)

/*
 * GPIO & Watchdog
 */
typedef struct GPIOWatchdog {
    /* gpio - cleared only by power-on-reset */
    uint32 gpiopullup;          /* 0x58 */
    uint32 gpiopulldown;
    uint32 gpioin;
    uint32 gpioout;
    uint32 gpioouten;
    uint32 gpiocontrol;
    uint32 gpiointpolarity;
    uint32 gpiointmask;

    /* gpio events */
    uint32 gpioevent;
    uint32 gpioeventintmask;

    /* Watchdog timer */
    uint32 watchdog;            /* 0x80 */

    /* gpio events */
    uint32 gpioeventintpolarity;

    /* GPIO based LED powersave registers */
    uint32 gpiotimerval;
    uint32 gpiotimeroutmask;
} GPIOWatchdog;

#define GPIO_WATCHDOG ((volatile GPIOWatchdog * const) GPIO_WATCHDOG_BASE)
#define GPIO ((volatile GPIOWatchdog * const) GPIO_WATCHDOG_BASE)

#define GPIO_NUM_MAX     32
#define GPIO_IN          0
#define GPIO_OUT         1

/*
 * Clock Control
 */
typedef struct ClockControlRegs {
    /* clock control */
    uint32 watchdog_counter;    /* 0x80 */
    uint32 PAD1[8];
    uint32 clkdiv;              /* corerev >= 3 */
    uint32 PAD2;
    uint32 capabilities_ext;    /* 0xac  */
} ClockControl;

#define CLOCK_CONTROL ((volatile ClockControl * const) CLOCK_CONTROL_BASE)

/*
 * UART peripheral
 */
typedef struct UartChannel {
    byte data;                  /* 0x00 */

    byte imr;                   /* 0x01 */
/* interrupt enable register */
#define	IER_ERXRDY              0x1 /* int on rx ready */
#define	IER_ETXRDY              0x2 /* int on tx ready */
#define	IER_ERLS                0x4 /* int on line status change */
#define	IER_EMSC                0x8 /* int on modem status change */
/* interrupt identification register */
#define IIR_IMASK               0xf /* mask */
#define IIR_RXTOUT              0xc /* receive timeout */
#define IIR_RLS                 0x6 /* receive line status */
#define IIR_RXRDY               0x4 /* receive ready */
#define IIR_TXRDY               0x2 /* transmit ready */
#define IIR_NOPEND              0x1 /* nothing */
#define IIR_MLSC                0x0 /* modem status */
#define IIR_FIFO_MASK           0xc0 /* set if FIFOs are enabled */

    byte fcr;                   /* 0x02 */
/* fifo control register */
#define FIFO_ENABLE             0x01 /* enable fifo */
#define FIFO_RCV_RST            0x02 /* reset receive fifo */
#define FIFO_XMT_RST            0x04 /* reset transmit fifo */
#define FIFO_DMA_MODE           0x08 /* enable dma mode */
#define FIFO_TRIGGER_1          0x00 /* trigger at 1 char */
#define FIFO_TRIGGER_4          0x40 /* trigger at 4 chars */
#define FIFO_TRIGGER_8          0x80 /* trigger at 8 chars */
#define FIFO_TRIGGER_14         0xc0 /* trigger at 14 chars */

    byte lcr;                   /* 0x03 */
#define LCR_DLAB                0x80
#define LCR_BC                  0x40
#define LCR_EPS                 0x10
#define LCR_PEN                 0x8
#define LCR_STOP                0x4
#define LCR_DLS_MASK            0x3

    byte mcr;                   /* 0x04 */
/* modem control register */
#define MCR_LOOPBACK            0x10 /* loopback */
#define MCR_IENABLE             0x08 /* output 2 = int enable */
#define MCR_DRS                 0x04
#define MCR_RTS                 0x02 /* enable RTS */
#define MCR_DTR                 0x01 /* enable DTR */

    byte lsr;                   /* 0x05 */
    /* line status register */
#define LSR_RCV_FIFO            0x80 /* error in receive fifo */
#define LSR_TSRE                0x40 /* transmitter empty */
#define LSR_TXRDY               0x20 /* transmitter ready */
#define LSR_BI                  0x10 /* break detected */
#define LSR_FE                  0x08 /* framing error */
#define LSR_PE                  0x04 /* parity error */
#define LSR_OE                  0x02 /* overrun error */
#define LSR_RXRDY               0x01 /* receiver ready */
#define LSR_RCV_MASK            0x1f

    byte msr;                   /* 0x06 */
/* modem status register */
#define MSR_DCD                 0x80 /* DCD active */
#define MSR_RI                  0x40 /* RI  active */
#define MSR_DSR                 0x20 /* DSR active */
#define MSR_CTS                 0x10 /* CTS active */
#define MSR_DDCD                0x08 /* DCD changed */
#define MSR_TERI                0x04 /* RI  changed */
#define MSR_DDSR                0x02 /* DSR changed */
#define MSR_DCTS                0x01 /* CTS changed */
    byte scratch;               /* 0x07 */
} Uart;

#define UART ((volatile Uart * const) UART_BASE)


/**
 * In chipcommon rev 49 the pmu registers have been moved from chipc to the pmu core if the
 * 'AOBPresent' bit of 'CoreCapabilitiesExt' is set. If this field is set, the traditional chipc to
 * [pmu|gci|sreng] register interface is deprecated and removed. These register blocks would instead
 * be assigned their respective chipc-specific address space and connected to the Always On
 * Backplane via the APB interface.
 */

/*
 * PMU registers
 */
typedef struct PmuRegs {
    uint32 pmucontrol;          /* 0x000 */
    uint32 pmucapabilities;
    uint32 pmustatus;
    uint32 res_state;
    uint32 res_pending;
    uint32 pmutimer;
    uint32 min_res_mask;
    uint32 max_res_mask;
    uint32 res_table_sel;
    uint32 res_dep_mask;
    uint32 res_updn_timer;
    uint32 res_timer;
    uint32 clkstretch;
    uint32 pmuwatchdog;
    uint32 gpiosel;             /* 0x038, rev >= 1 */
    uint32 gpioenable;          /* 0x03c, rev >= 1 */
    uint32 res_req_timer_sel;
    uint32 res_req_timer;
    uint32 res_req_mask;
    uint32 PAD2;
    uint32 chipcontrol_addr;    /* 0x050 */
    uint32 chipcontrol_data;    /* 0x054 */
    uint32 regcontrol_addr;
    uint32 regcontrol_data;
    uint32 pllcontrol_addr;
    uint32 pllcontrol_data;
#define PMU_PLL_CTRL_P1DIV_MASK   0x00f00000
#define PMU_PLL_CTRL_P1DIV_SHIFT  20
#define PMU_PLL_CTRL_P2DIV_MASK   0x0f000000
#define PMU_PLL_CTRL_P2DIV_SHIFT  24
#define PMU_PLL_CTRL_M1DIV_MASK   0x000000ff
#define PMU_PLL_CTRL_M1DIV_SHIFT  0
#define PMU_PLL_CTRL_M2DIV_MASK   0x0000ff00
#define PMU_PLL_CTRL_M2DIV_SHIFT  8
#define PMU_PLL_CTRL_M3DIV_MASK   0x00ff0000
#define PMU_PLL_CTRL_M3DIV_SHIFT  16
#define PMU_PLL_CTRL_M4DIV_MASK   0xff000000
#define PMU_PLL_CTRL_M4DIV_SHIFT  24
    uint32 pmustrapopt;         /* 0x068, corerev >= 28 */
    uint32 pmu_xtalfreq;        /* 0x06C, pmurev >= 10 */
    uint32 retention_ctl;       /* 0x070 */
    uint32 ilp_period;          /* 0x074  */
    uint32 PAD3[2];
    uint32 retention_grpidx;    /* 0x080 */
    uint32 retention_grpctl;    /* 0x084 */
    uint32 PAD4[20];
    uint32 pmucontrol_ext;      /* 0x0d8 */
    uint32 slowclkperiod;       /* 0x0dc */
    uint32 PAD5[8];
    uint32 pmuintmask0;         /* 0x000 */
    uint32 pmuintmask1;         /* 0x004 */
    uint32 PAD6[14];
    uint32 pmuintstatus;        /* 0x040 */
    uint32 PAD7[15];
    uint32 pmuintctrl0;         /* 0x080 */
} Pmu;

#define PMU ((volatile Pmu * const) PMU_BASE)

/* PMU registers (corerev >= 20) */
/* Note: all timers driven by ILP clock are updated asynchronously to HT/ALP.
 * The CPU must read them twice, compare, and retry if different.
 */
#define PMUCC ((volatile Pmu * const) PMUCC_BASE)


/*
 * GCI (Global Coex Interface) registers
 */
typedef struct GciRegs {
    uint32 gci_corecaps0;               /* 0x000 */
    uint32 gci_corecaps1;               /* 0x004 */
    uint32 gci_corecaps2;               /* 0x008 */
    uint32 gci_corectrl;                /* 0x00c */
    uint32 gci_corestat;                /* 0x010 */
    uint32 gci_intstat;                 /* 0x014 */
    uint32 gci_intmask;                 /* 0x018 */
    uint32 gci_wakemask;                /* 0x01c */
    uint32 gci_levelintstat;            /* 0x020 */
    uint32 gci_eventintstat;            /* 0x024 */
    uint32 gci_wakelevelintstat;        /* 0x028 */
    uint32 gci_wakeeventintstat;        /* 0x02c */
    uint32 semaphoreintstatus;          /* 0x030 */
    uint32 semaphoreintmask;            /* 0x034 */
    uint32 semaphorerequest;            /* 0x038 */
    uint32 semaphorereserve;            /* 0x03c */
    uint32 gci_indirect_addr;           /* 0x040 */
    uint32 gci_gpioctl;                 /* 0x044 */
    uint32 gci_gpiostatus;              /* 0x048 */
    uint32 gci_gpiomask;                /* 0x04c */
    uint32 eventsummary;                /* 0x050 */
    uint32 gci_miscctl;                 /* 0x054 */
    uint32 gci_gpiointmask;             /* 0x058 */
    uint32 gci_gpiowakemask;            /* 0x05c */
    uint32 gci_input[32];               /* 0x060 */
    uint32 gci_event[32];               /* 0x0e0 */
    uint32 gci_output[4];               /* 0x160 */
    uint32 gci_control_0;               /* 0x170 */
    uint32 gci_control_1;               /* 0x174 */
    uint32 gci_intpolreg;               /* 0x178 */
    uint32 gci_levelintmask;            /* 0x17c */
    uint32 gci_eventintmask;            /* 0x180 */
    uint32 wakelevelintmask;            /* 0x184 */
    uint32 wakeeventintmask;            /* 0x188 */
    uint32 hwmask;                      /* 0x18c */
    uint32 PAD1;
    uint32 gci_inbandeventintmask;      /* 0x194 */
    uint32 PAD2;
    uint32 gci_inbandeventstatus;       /* 0x19c */
    uint32 gci_seciauxtx;               /* 0x1a0 */
    uint32 gci_seciauxrx;               /* 0x1a4 */
    uint32 gci_secitx_datatag;          /* 0x1a8 */
    uint32 gci_secirx_datatag;          /* 0x1ac */
    uint32 gci_secitx_datamask;         /* 0x1b0 */
    uint32 gci_seciusef0tx_reg;         /* 0x1b4 */
    uint32 gci_secif0tx_offset;         /* 0x1b8 */
    uint32 gci_secif0rx_offset;         /* 0x1bc */
    uint32 gci_secif1tx_offset;         /* 0x1c0 */
    uint32 gci_rxfifo_common_ctrl;      /* 0x1c4 */
    uint32 gci_rxfifoctrl;              /* 0x1c8 */
    uint32 PAD3;
    uint32 gci_seciuartescval;          /* 0x1d0 */
    uint32 gic_seciuartautobaudctr;     /* 0x1d4 */
    uint32 gci_secififolevel;           /* 0x1d8 */
    uint32 gci_seciuartdata;            /* 0x1dc */
    uint32 gci_secibauddiv;             /* 0x1e0 */
    uint32 gci_secifcr;                 /* 0x1e4 */
    uint32 gci_secilcr;                 /* 0x1e8 */
    uint32 gci_secimcr;                 /* 0x1ec */
    uint32 gci_secilsr;                 /* 0x1f0 */
    uint32 gci_secimsr;                 /* 0x1f4 */
    uint32 gci_baudadj;                 /* 0x1f8 */
    uint32 gci_inbandintmask;           /* 0x1fc */
    uint32 gci_chipctrl;                /* 0x200 */
    uint32 gci_chipsts;                 /* 0x204 */
    uint32 gci_gpioout;                 /* 0x208 */
    uint32 gci_gpioout_read;            /* 0x20C */
    uint32 gci_mpwaketx;                /* 0x210 */
    uint32 gci_mpwakedetect;            /* 0x214 */
    uint32 gci_seciin_ctrl;             /* 0x218 */
    uint32 gci_seciout_ctrl;            /* 0x21C */
    uint32 gci_seciin_auxfifo_en;       /* 0x220 */
    uint32 gci_seciout_txen_txbr;       /* 0x224 */
    uint32 gci_seciin_rxbrstatus;       /* 0x228 */
    uint32 gci_seciin_rxerrstatus;      /* 0x22C */
    uint32 gci_seciin_fcstatus;         /* 0x230 */
    uint32 gci_seciout_txstatus;        /* 0x234 */
    uint32 gci_seciout_txbrstatus;      /* 0x238 */
    uint32 PAD4[49];
    uint32 gci_chipid;                  /* 0x300 */
    uint32 PAD5[3];
    uint32 otpstatus;                   /* 0x310 */
    uint32 otpcontrol;                  /* 0x314 */
    uint32 otpprog;                     /* 0x318 */
    uint32 otplayout;                   /* 0x31c */
    uint32 otplayoutextension;          /* 0x320 */
    uint32 otpcontrol1;                 /* 0x324 */
} Gci;

#define GCI ((volatile Gci * const) GCI_BASE)

/*
 * GMAC registers
 */
typedef struct EnetCoreMiscRegs {
    /* device control */
#define DC_TSM                          0x00000002
#define DC_CFCO                         0x00000004
#define DC_RLSS                         0x00000008
#define DC_MROR                         0x00000010
#define DC_FCM_MASK                     0x00000060
#define DC_FCM_SHIFT                    5
#define DC_NAE                          0x00000080
#define DC_TF                           0x00000100
#define DC_RDS_MASK                     0x00030000
#define DC_RDS_SHIFT                    16
#define DC_TDS_MASK                     0x000c0000
#define DC_TDS_SHIFT                    18
    uint32 devcontrol;                  /* 0x000 */
    /* device status */
#define	DS_RBF                          0x00000001
#define	DS_RDF                          0x00000002
#define	DS_RIF                          0x00000004
#define	DS_TBF                          0x00000008
#define	DS_TDF                          0x00000010
#define	DS_TIF                          0x00000020
#define	DS_PO                           0x00000040
#define	DS_MM_MASK                      0x00000300
#define	DS_MM_SHIFT                      8
    uint32 devstatus;                   /* 0x004 */
    uint32 PAD1;
    uint32 biststatus;                  /* 0x00c */
    uint32 PAD2[4];
    /* interrupt status */
#define	I_MRO                           0x00000001
#define	I_MTO                           0x00000002
#define	I_TFD                           0x00000004
#define	I_LS                            0x00000008
#define	I_MDIO                          0x00000010
#define	I_MR                            0x00000020
#define	I_MT                            0x00000040
#define	I_TO                            0x00000080
#define	I_PDEE                          0x00000400
#define	I_PDE                           0x00000800
#define	I_DE                            0x00001000
#define	I_RDU                           0x00002000
#define	I_RFO                           0x00004000
#define	I_XFU                           0x00008000
#define	I_RI                            0x00010000
#define	I_XI0                           0x01000000
#define	I_XI1                           0x02000000
#define	I_XI2                           0x04000000
#define	I_XI3                           0x08000000
#define	I_INTMASK                       0x0f01fcff
#define	I_ERRMASK                       0x0000fc00
#define I_ERRORS                        (I_PDEE | I_PDE | I_DE | I_RDU | I_RFO | I_XFU)
#define DEF_INTMASK                     (I_XI0 | I_XI1 | I_XI2 | I_XI3 | I_RI | I_ERRORS | I_TO)
    uint32 intstatus;                   /* 0x020 */
    uint32 intmask;                     /* 0x024 */
    uint32 gptimer;                     /* 0x028 */
    uint32 PAD3[53];
    /* interrupt receive lazy */
#define	IRL_TO_MASK                     0x00ffffff
#define	IRL_FC_MASK                     0xff000000
#define	IRL_FC_SHIFT                    24
    uint32 intrecvlazy;                 /* 0x100 */
    uint32 flowctlthresh;               /* 0x104 */
    uint32 wrrthresh;                   /* 0x108 */
    uint32 gmac_idle_cnt_thresh;        /* 0x10c */
    uint32 PAD4[28];
    /* phy access */
#define	PA_DATA_MASK                    0x0000ffff
#define	PA_ADDR_MASK                    0x001f0000
#define	PA_ADDR_SHIFT                   16
#define	PA_REG_MASK                     0x1f000000
#define	PA_REG_SHIFT                    24
#define	PA_WRITE                        0x20000000
#define	PA_START                        0x40000000
    uint32 phyaccess;                   /* 0x180 */
    uint32 PAD5;
    /* phy control */
#define	PC_EPA_MASK                     0x0000001f
#define	PC_MCT_MASK                     0x007f0000
#define	PC_MCT_SHIFT                    16
#define	PC_MTE                          0x00800000
    uint32 phycontrol;                  /* 0x188 */
    /* txq control */
#define	TC_DBT_MASK                     0x00000fff
#define	TC_DBT_SHIFT                     0
    uint32 txqctl;                      /* 0x18c */
    /* rxq control */
#define	RC_DBT_MASK                     0x00000fff
#define	RC_DBT_SHIFT                     0
#define	RC_PTE                          0x00001000
#define	RC_MDP_MASK                     0x3f000000
#define	RC_MDP_SHIFT                    24
    uint32 rxqctl;                      /* 0x190 */
    uint32 gpioselect;                  /* 0x194 */
    uint32 gpio_output_en;              /* 0x198 */
    uint32 PAD6[17];
    /* clk control status */
#define CS_FA                           0x00000001
#define CS_FH                           0x00000002
#define CS_FI                           0x00000004
#define CS_AQ                           0x00000008
#define CS_HQ                           0x00000010
#define CS_FC                           0x00000020
#define CS_ER                           0x00000100
#define CS_AA                           0x00010000
#define CS_HA                           0x00020000
#define CS_BA                           0x00040000
#define CS_BH                           0x00080000
#define CS_ES                           0x01000000
    uint32 clk_ctl_st;                  /* 0x1e0 */
/* clk control status */
#define CS_FA                           0x00000001
#define CS_FH                           0x00000002
#define CS_FI                           0x00000004
#define CS_AQ                           0x00000008
#define CS_HQ                           0x00000010
#define CS_FC                           0x00000020
#define CS_ER                           0x00000100
#define CS_AA                           0x00010000
#define CS_HA                           0x00020000
#define CS_BA                           0x00040000
#define CS_BH                           0x00080000
#define CS_ES                           0x01000000
    uint32 hw_war;                      /* 0x1e4 */
    uint32 pwrctl;                      /* 0x1e8 */
} EnetCoreMisc;

/*
 * DMA Channel Configuration
 */

/* dma registers per channel(xmt or rcv) */
typedef struct {
    uint32  control;
/* Common fields */
#define DMA_EN                                  1
#define DMA_PTY_CHK_DISABLE                     (1 << 11)
#define DMA_SELECT_ACTIVE                       (1 << 13)
#define DMA_ADDR_EXT_MASK                       (3 << 16)
#define DMA_ADDR_EXT_SHIFT                      16
#define DMA_BURST_LEN_MASK                      (7 << 18)
#define DMA_BURST_LEN_SHIFT                     18
#define DMA_PREFETCH_CTL_MASK                   (7 << 21)
#define DMA_PREFETCH_CTL_SHIFT                  21
#define DMA_PREFETCH_THRESH_MASK                (3 << 24)
#define DMA_PREFETCH_THRESH_SHIFT               24
/* Fields for XMT register */
#define DMA_SUSP_EN                             (1 << 1)
#define DMA_LOOPBACK_EN                         (1 << 2)
#define DMA_FLUSH_GMAC                          (1 << 4)
#define DMA_BURST_ALIGN_EN                      (1 << 5)
#define DMA_MULTIPLE_OUTSTANDING_READS_MASK     (7 << 6)
#define DMA_MULTIPLE_OUTSTANDING_READS_SHIFT    6
#define DMA_CHANNEL_SWITCH_EN                   (1 << 9)
/* Fields for RCV register */
#define DMA_OFFSET_MASK                         (0xFE << 1)
#define DMA_OFFSET_SHIFT                        1
#define DMA_FIFO_MODE                           (1 << 8)
#define DMA_SEP_RX_HDR_DESC_EN                  (1 << 9)
#define DMA_OVERFLOW_CONTINUE                   (1 << 10)
#define DMA_WAIT_FOR_COMPLETE                   (1 << 12)
#define DMA_SELECT_ACTIVE                       (1 << 13)
#define DMA_GLOM_EN                             (1 << 14)
    uint32  ptr;      /* last descriptor posted to chip */
    uint32  addrlow;  /* descriptor ring base address low 32-bits (8K aligned) */
    uint32  addrhigh; /* descriptor ring base address bits 63:32 (8K aligned) */
    uint32  status0;  /* current descriptor, xmt state */
#define DMA_CURRENT_DESCR                       0xFFFF
#define DMA_STATE_MASK                          (0xF << 28)
#define DMA_STATE_SHIFT                         28
#define DMA_STATE_ACTIVE                        (1 << DMA_STATE_SHIFT)
#define DMA_STATE_IDLE                          (2 << DMA_STATE_SHIFT)
#define DMA_STATE_STOP                          (3 << DMA_STATE_SHIFT)
#define DMA_STATE_SUSPEND                       (4 << DMA_STATE_SHIFT)

    uint32  status1;  /* active descriptor, xmt error */
#define DMA_ACTIVE_DESCR                        0xFFFF
#define DMA_FLUSH_DONE                          (1 << 26)
#define DMA_DESC_UNDERRUN                       FLUSH_DONE
#define DMA_COMP                                (1 << 27)
#define DMA_PROC_ERROR_MASK                     (0xF << 28)
#define DMA_PROC_ERROR_SHIFT                    28
#define DMA_PROC_PROTO_ERROR                    (1 << DMA_PROC_ERROR_SHIFT)
} DmaChannelCfg;

typedef struct {
  DmaChannelCfg dmaxmt;         /* dma tx */
  uint32 PAD1[2];
  DmaChannelCfg dmarcv;         /* dma rx */
  uint32 PAD2[2];
} DmaRegs;

/* 4 channels (DMA controllers) per Ethernet core */
#define IUDMA_MAX_CHANNELS      4

/*
 * DMA Buffer
 */
typedef struct DmaDesc {
    uint32 ctrl1;
#define DMA_CTRL1_NOTPCIE       (1 << 18)       /* burst size control */
#define DMA_CTRL1_EOT           (1 << 28)       /* End of descriptor table */
#define DMA_CTRL1_IOC           (1 << 29)       /* Interrupt on completion */
#define DMA_CTRL1_EOF           (1 << 30)       /* End of frame */
#define DMA_CTRL1_SOF           (1 << 31)       /* Start of frame */
    uint32 ctrl2;
#define DMA_CTRL2_BC_MASK       0x00007FFF      /* Buffer byte count */
#define DMA_CTRL2_AE            0x00030000      /* Address extension bits */
#define DMA_CTRL2_AE_SHIFT      16
#define DMA_CTRL2_PARITY        0x00040000      /* Parity bit */
    uint32 addrlow;
    uint32 addrhigh;
} DmaDesc;

/*
 * DMA RX status
 */
typedef struct DmaRxStatus {
  union {
    struct {
        uint32        length        :16;        /* in bytes of data in buffer */
        uint32        reserved      : 7;
        uint32        rx_overflow   : 1;        /* fifo overflow */
        uint32        dscr_cntr     : 4;        /* number of descriptor of the frame minus 1 */
        uint32        data_type     : 4;        /* receive data type */
    };
    uint32      word;
  };
}DmaRxStatus;

typedef struct EnetCoreMibRegs {
    uint32 tx_good_octets;              /* 0x300 */
    uint32 tx_good_octets_high;         /* 0x304 */
    uint32 tx_good_pkts;                /* 0x308 */
    uint32 tx_octets;                   /* 0x30c */
    uint32 tx_octets_high;              /* 0x310 */
    uint32 tx_pkts;                     /* 0x314 */
    uint32 tx_broadcast_pkts;           /* 0x318 */
    uint32 tx_multicast_pkts;           /* 0x31c */
    uint32 tx_len_64;                   /* 0x320 */
    uint32 tx_len_65_to_127;            /* 0x324 */
    uint32 tx_len_128_to_255;           /* 0x328 */
    uint32 tx_len_256_to_511;           /* 0x32c */
    uint32 tx_len_512_to_1023;          /* 0x330 */
    uint32 tx_len_1024_to_1522;         /* 0x334 */
    uint32 tx_len_1523_to_2047;         /* 0x338 */
    uint32 tx_len_2048_to_4095;         /* 0x33c */
    uint32 tx_len_4095_to_8191;         /* 0x340 */
    uint32 tx_len_8192_to_max;          /* 0x344 */
    uint32 tx_jabber_pkts;              /* 0x348 */
    uint32 tx_oversize_pkts;            /* 0x34c */
    uint32 tx_fragment_pkts;            /* 0x350 */
    uint32 tx_underruns;                /* 0x354 */
    uint32 tx_total_cols;               /* 0x358 */
    uint32 tx_single_cols;              /* 0x35c */
    uint32 tx_multiple_cols;            /* 0x360 */
    uint32 tx_excessive_cols;           /* 0x364 */
    uint32 tx_late_cols;                /* 0x368 */
    uint32 tx_defered;                  /* 0x36c */
    uint32 tx_carrier_lost;             /* 0x370 */
    uint32 tx_pause_pkts;               /* 0x374 */
    uint32 tx_uni_pkts;                 /* 0x378 */
    uint32 tx_q0_pkts;                  /* 0x37c */
    uint32 tx_q0_octets;                /* 0x380 */
    uint32 tx_q0_octets_high;           /* 0x384 */
    uint32 tx_q1_pkts;                  /* 0x388 */
    uint32 tx_q1_octets;                /* 0x38c */
    uint32 tx_q1_octets_high;           /* 0x390 */
    uint32 tx_q2_pkts;                  /* 0x394 */
    uint32 tx_q2_octets;                /* 0x398 */
    uint32 tx_q2_octets_high;           /* 0x39c */
    uint32 tx_q3_pkts;                  /* 0x3a0 */
    uint32 tx_q3_octets;                /* 0x3a4 */
    uint32 tx_q3_octets_high;           /* 0x3a8 */
    uint32 PAD;
    uint32 rx_good_octets;              /* 0x3b0 */
    uint32 rx_good_octets_high;         /* 0x3b4 */
    uint32 rx_good_pkts;                /* 0x3b8 */
    uint32 rx_octets;                   /* 0x3bc */
    uint32 rx_octets_high;              /* 0x3c0 */
    uint32 rx_pkts;                     /* 0x3c4 */
    uint32 rx_broadcast_pkts;           /* 0x3c8 */
    uint32 rx_multicast_pkts;           /* 0x3cc */
    uint32 rx_len_64;                   /* 0x3d0 */
    uint32 rx_len_65_to_127;            /* 0x3d4 */
    uint32 rx_len_128_to_255;           /* 0x3d8 */
    uint32 rx_len_256_to_511;           /* 0x3dc */
    uint32 rx_len_512_to_1023;          /* 0x3e0 */
    uint32 rx_len_1024_to_1522;         /* 0x3e4 */
    uint32 rx_len_1523_to_2047;         /* 0x3e8 */
    uint32 rx_len_2048_to_4095;         /* 0x3ec */
    uint32 rx_len_4095_to_8191;         /* 0x3f0 */
    uint32 rx_len_8192_to_max;          /* 0x3f4 */
    uint32 rx_jabber_pkts;              /* 0x3f8 */
    uint32 rx_oversize_pkts;            /* 0x3fc */
    uint32 rx_fragment_pkts;            /* 0x400 */
    uint32 rx_missed_pkts;              /* 0x404 */
    uint32 rx_crc_align_errs;           /* 0x408 */
    uint32 rx_undersize;                /* 0x40c */
    uint32 rx_crc_errs;                 /* 0x410 */
    uint32 rx_align_errs;               /* 0x414 */
    uint32 rx_symbol_errs;              /* 0x418 */
    uint32 rx_pause_pkts;               /* 0x41c */
    uint32 rx_nonpause_pkts;            /* 0x420 */
    uint32 rx_sachanges;                /* 0x424 */
    uint32 rx_uni_pkts;                 /* 0x428 */
} EnetCoreMib;

typedef struct EnetCoreUnimacRegs {
    uint32 unimacversion;               /* 0x800 */
    uint32 hdbkpctl;                    /* 0x804 */
    /* command config */
#define	CC_TE                           0x00000001
#define	CC_RE                           0x00000002
#define	CC_ES_MASK                      0x0000000c
#define	CC_ES_SHIFT                     2
#define	CC_PROM                         0x00000010
#define	CC_PAD_EN                       0x00000020
#define	CC_CF                           0x00000040
#define	CC_PF                           0x00000080
#define	CC_RPI                          0x00000100
#define	CC_TAI                          0x00000200
#define	CC_HD                           0x00000400
#define	CC_HD_SHIFT                     10
#define CC_SR                           0x00002000  /* (corerev >= 4) */
#define	CC_ML                           0x00008000
#define	CC_AE                           0x00400000
#define	CC_CFE                          0x00800000
#define	CC_NLC                          0x01000000
#define	CC_RL                           0x02000000
#define	CC_RED                          0x04000000
#define	CC_PE                           0x08000000
#define	CC_TPI                          0x10000000
#define CC_AT                           0x0         /* (corerev >= 6) */
    uint32 cmdcfg;                      /* 0x808 */
    uint32 macaddrhigh;                 /* 0x80c */
    uint32 macaddrlow;                  /* 0x810 */
    uint32 rxmaxlength;                 /* 0x814 */
    uint32 pausequanta;                 /* 0x818 */
    uint32 PAD9[10];
    uint32 macmode;                     /* 0x844 */
    uint32 outertag;                    /* 0x848 */
    uint32 innertag;                    /* 0x84c */
    uint32 PAD1[3];
    uint32 txipg;                       /* 0x85c */
    uint32 PAD2[180];
    uint32 pausectl;                    /* 0xb30 */
    uint32 txflush;                     /* 0xb34 */
    uint32 rxstatus;                    /* 0xb38 */
    uint32 txstatus;                    /* 0xb3c */
} EnetCoreUnimac;

#define ENET_CORE0_MISC ((volatile EnetCoreMisc * const) ENET_CORE0_BASE)
#define ENET_CORE0_DMA ((volatile DmaRegs * const) (ENET_CORE0_BASE + 0x200))
#define ENET_CORE0_MIB ((volatile EnetCoreMib * const) (ENET_CORE0_BASE + 0x300))
#define ENET_CORE0_UNIMAC ((volatile EnetCoreUnimac * const) (ENET_CORE0_BASE + 0x800))

#define ENET_CORE1_MISC ((volatile EnetCoreMisc * const) ENET_CORE1_BASE)
#define ENET_CORE1_DMA ((volatile DmaRegs * const) (ENET_CORE1_BASE + 0x200))
#define ENET_CORE1_MIB ((volatile EnetCoreMib * const) (ENET_CORE1_BASE + 0x300))
#define ENET_CORE1_UNIMAC ((volatile EnetCoreUnimac * const) (ENET_CORE1_BASE + 0x800))

typedef struct AidmpRegs {
    uint32 oobselina30;                 /* 0x000 */
    uint32 oobselina74;                 /* 0x004 */
    uint32 PAD1[6];
    uint32 oobselinb30;                 /* 0x020 */
    uint32 oobselinb74;                 /* 0x024 */
    uint32 PAD2[6];
    uint32 oobselinc30;                 /* 0x040 */
    uint32 oobselinc74;                 /* 0x044 */
    uint32 PAD3[6];
    uint32 oobselind30;                 /* 0x060 */
    uint32 oobselind74;                 /* 0x064 */
    uint32 PAD4[38];
    uint32 oobselouta30;                /* 0x100 */
    uint32 oobselouta74;                /* 0x104 */
    uint32 PAD5[6];
    uint32 oobseloutb30;                /* 0x120 */
    uint32 oobseloutb74;                /* 0x124 */
    uint32 PAD6[6];
    uint32 oobseloutc30;                /* 0x140 */
    uint32 oobseloutc74;                /* 0x144 */
    uint32 PAD7[6];
    uint32 oobseloutd30;                /* 0x160 */
    uint32 oobseloutd74;                /* 0x164 */
    uint32 PAD8[38];
    uint32 oobsynca;                    /* 0x200 */
    uint32 oobseloutaen;                /* 0x204 */
    uint32 PAD9[6];
    uint32 oobsyncb;                    /* 0x220 */
    uint32 oobseloutben;                /* 0x224 */
    uint32 PAD10[6];
    uint32 oobsyncc;                    /* 0x240 */
    uint32 oobseloutcen;                /* 0x244 */
    uint32 PAD11[6];
    uint32 oobsyncd;                    /* 0x260 */
    uint32 oobseloutden;                /* 0x264 */
    uint32 PAD12[38];
    uint32 oobaextwidth;                /* 0x300 */
    uint32 oobainwidth;                 /* 0x304 */
    uint32 oobaoutwidth;                /* 0x308 */
    uint32 PAD13[5];
    uint32 oobbextwidth;                /* 0x320 */
    uint32 oobbinwidth;                 /* 0x324 */
    uint32 oobboutwidth;                /* 0x328 */
    uint32 PAD14[5];
    uint32 oobcextwidth;                /* 0x340 */
    uint32 oobcinwidth;                 /* 0x344 */
    uint32 oobcoutwidth;                /* 0x348 */
    uint32 PAD15[5];
    uint32 oobdextwidth;                /* 0x360 */
    uint32 oobdinwidth;                 /* 0x364 */
    uint32 oobdoutwidth;                /* 0x368 */
    uint32 PAD16[37];
    uint32 ioctrlset;                   /* 0x400 */
    uint32 ioctrlclear;                 /* 0x404 */
    /* Common core control flags */
#define SICF_BIST_EN                    0x8000
#define SICF_PME_EN                     0x4000
#define SICF_CORE_BITS                  0x3ffc
#define SICF_FGC                        0x0002
#define SICF_CLOCK_EN                   0x0001
#define NAND_APB_LITTLE_ENDIAN          0x01000000
    uint32 ioctrl;                      /* 0x408 */
    uint32 PAD17[61];
    /* Common core status flags */
#define SISF_BIST_DONE                  0x8000
#define SISF_BIST_ERROR                 0x4000
#define SISF_GATED_CLK                  0x2000
#define SISF_DMA64                      0x1000
#define SISF_CORE_BITS                  0x0fff
    uint32 iostatus;                    /* 0x500 */
    uint32 PAD18[127];
    uint32 ioctrlwidth;                 /* 0x700 */
    uint32 iostatuswidth;               /* 0x704 */
    uint32 PAD19[62];
    /* resetctrl */
#define AIRC_RESET  1
    uint32 resetctrl;                   /* 0x800 */
    uint32 resetstatus;                 /* 0x804 */
    uint32 resetreadid;                 /* 0x808 */
    uint32 resetwriteid;                /* 0x80c */
    uint32 PAD20[60];
    uint32 errlogctrl;                  /* 0x900 */
    uint32 errlogdone;                  /* 0x904 */
    uint32 errlogstatus;                /* 0x908 */
    uint32 errlogaddrlo;                /* 0x90c */
    uint32 errlogaddrhi;                /* 0x910 */
    uint32 errlogid;                    /* 0x914 */
    uint32 errloguser;                  /* 0x918 */
    uint32 errlogflags;                 /* 0x91c */
    uint32 PAD21[56];
    uint32 intstatus;                   /* 0xa00 */
    uint32 PAD22[255];
    uint32 config;                      /* 0xe00 */
    uint32 PAD23[63];
    uint32 itcr;                        /* 0xf00 */
    uint32 PAD24[3];
    uint32 itipooba;                    /* 0xf10 */
    uint32 itipoobb;                    /* 0xf14 */
    uint32 itipoobc;                    /* 0xf18 */
    uint32 itipoobd;                    /* 0xf1c */
    uint32 PAD25[4];
    uint32 itipoobaout;                 /* 0xf30 */
    uint32 itipoobbout;                 /* 0xf34 */
    uint32 itipoobcout;                 /* 0xf38 */
    uint32 itipoobdout;                 /* 0xf3c */
    uint32 PAD26[4];
    uint32 itopooba;                    /* 0xf50 */
    uint32 itopoobb;                    /* 0xf54 */
    uint32 itopoobc;                    /* 0xf58 */
    uint32 itopoobd;                    /* 0xf5c */
    uint32 PAD27[4];
    uint32 itopoobain;                  /* 0xf70 */
    uint32 itopoobbin;                  /* 0xf74 */
    uint32 itopoobcin;                  /* 0xf78 */
    uint32 itopoobdin;                  /* 0xf7c */
    uint32 PAD28[4];
    uint32 itopreset;                   /* 0xf90 */
    uint32 PAD29[15];
    uint32 peripherialid4;              /* 0xfd0 */
    uint32 peripherialid5;              /* 0xfd4 */
    uint32 peripherialid6;              /* 0xfd8 */
    uint32 peripherialid7;              /* 0xfdc */
    uint32 peripherialid0;              /* 0xfe0 */
    uint32 peripherialid1;              /* 0xfe4 */
    uint32 peripherialid2;              /* 0xfe8 */
    uint32 peripherialid3;              /* 0xfec */
    uint32 componentid0;                /* 0xff0 */
    uint32 componentid1;                /* 0xff4 */
    uint32 componentid2;                /* 0xff8 */
    uint32 componentid3;                /* 0xffc */
} Aidmp;

#define WLAN_MAC_WRAP ((volatile Aidmp * const) WLAN_MAC_WRAP_BASE)
#define PCIE_GEN2_WRAP ((volatile Aidmp * const) PCIE_GEN2_WRAP_BASE)
#define ARM_CA7_WRAP ((volatile Aidmp * const) ARM_CA7_WRAP_BASE)
#define USB_2_0_HOST_WRAP ((volatile Aidmp * const) USB_2_0_HOST_WRAP_BASE)
#define ENET_CORE0_WRAP ((volatile Aidmp * const) ENET_CORE0_WRAP_BASE)
#define IS2_WRAP ((volatile Aidmp * const) IS2_WRAP_BASE)
#define DDR_CTRL_WRAP ((volatile Aidmp * const) DDR_CTRL_WRAP_BASE)
#define NAND_FLASH_CTRL_WRAP ((volatile Aidmp * const) NAND_FLASH_CTRL_WRAP_BASE)
#define ENET_CORE1_WRAP ((volatile Aidmp * const) ENET_CORE1_WRAP_BASE)
#define GCI_WRAP ((volatile Aidmp * const) GCI_WRAP_BASE)
#define PMU_WRAP ((volatile Aidmp * const) PMU_WRAP_BASE)


/*
** NAND Controller Registers
*/
typedef struct NandCtrlRegs {
   uint32 NandRevision; /* 0x00 */
   uint32 NandCmdStart; /* 0x04 */
#define NCMD_MASK    0x0000001f
#define NCMD_BLOCK_ERASE_MULTI       0x15000000
#define NCMD_PROGRAM_PAGE_MULTI      0x13000000
#define NCMD_STS_READ_MULTI          0x12000000
#define NCMD_PAGE_READ_MULTI         0x11000000
#define NCMD_LOW_LEVEL_OP            0x10000000
#define NCMD_PARAM_CHG_COL           0x0f000000
#define NCMD_PARAM_READ              0x0e000000
#define NCMD_BLK_LOCK_STS            0x0d000000
#define NCMD_BLK_UNLOCK              0x0c000000
#define NCMD_BLK_LOCK_DOWN           0x0b000000
#define NCMD_BLK_LOCK                0x0a000000
#define NCMD_FLASH_RESET             0x09000000
#define NCMD_BLOCK_ERASE             0x08000000
#define NCMD_DEV_ID_READ             0x07000000
#define NCMD_COPY_BACK               0x06000000
#define NCMD_PROGRAM_SPARE           0x05000000
#define NCMD_PROGRAM_PAGE            0x04000000
#define NCMD_STS_READ                0x03000000
#define NCMD_SPARE_READ              0x02000000
#define NCMD_PAGE_READ               0x01000000
   uint32 NandCmdExtAddr;  /* 0x08 */
   uint32 NandCmdAddr;  /* 0x0c */
   uint32 NandCmdEndAddr;  /* 0x10 */
   uint32 NandIntfcStatus; /* 0x14 */
#define NIS_CTLR_READY     (1 << 31)
#define NIS_FLASH_READY    (1 << 30)
#define NIS_CACHE_VALID    (1 << 29)
#define NIS_SPARE_VALID    (1 << 28)
#define NIS_FLASH_STS_MASK 0xff000000
#define NIS_WRITE_PROTECT  0x80000000
#define NIS_DEV_READY      0x40000000
#define NIS_PGM_ERASE_ERROR   0x01000000


   uint32 NandNandBootConfig; /* 0x18 */
#define NBC_CS_LOCK     (1 << 31)
#define NBC_AUTO_DEV_ID_CFG   (1 << 30)
#define NBC_WR_PROT_BLK0   (1 << 28)
#define NBC_EBI_CS7_USES_NAND (1<<15)
#define NBC_EBI_CS6_USES_NAND (1<<14)
#define NBC_EBI_CS5_USES_NAND (1<<13)
#define NBC_EBI_CS4_USES_NAND (1<<12)
#define NBC_EBI_CS3_USES_NAND (1<<11)
#define NBC_EBI_CS2_USES_NAND (1<<10)
#define NBC_EBI_CS1_USES_NAND (1<< 9)
#define NBC_EBI_CS0_USES_NAND (1<< 8)
#define NBC_EBC_CS7_SEL    (1<< 7)
#define NBC_EBC_CS6_SEL    (1<< 6)
#define NBC_EBC_CS5_SEL    (1<< 5)
#define NBC_EBC_CS4_SEL    (1<< 4)
#define NBC_EBC_CS3_SEL    (1<< 3)
#define NBC_EBC_CS2_SEL    (1<< 2)
#define NBC_EBC_CS1_SEL    (1<< 1)
#define NBC_EBC_CS0_SEL    (1<< 0)

   uint32 NandCsNandXor;      /* 0x1c */
   uint32 NandLlOpNand;            /* 0x20 */
   uint32 NandMplaneBaseExtAddr; /* 0x24 */
   uint32 NandMplaneBaseAddr; /* 0x28 */
   uint32 NandReserved1[9];   /* 0x2c-0x4f */
   uint32 NandAccControl;     /* 0x50 */
#define NAC_RD_ECC_EN      (1 << 31)
#define NAC_WR_ECC_EN      (1 << 30)
#define NAC_CE_CARE_EN          (1 << 28)
#define NAC_RD_ERASED_ECC_EN  (1 << 27)
#define NAC_PARTIAL_PAGE_EN   (1 << 26)
#define NAC_WR_PREEMPT_EN  (1 << 25)
#define NAC_PAGE_HIT_EN    (1 << 24)
#define NAC_PREFETCH_EN    (1 << 23)
#define NAC_CACHE_MODE_EN  (1 << 22)
#define NAC_ECC_LVL_SHIFT  16
#define NAC_ECC_LVL_MASK   0x001f0000
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
#define NAC_SECTOR_SIZE_1K (1 << 7)
#define NAC_SPARE_SZ_SHIFT 0
#define NAC_SPARE_SZ_MASK  0x0000007f

   uint32 NandConfig;      /* 0x54 */ /* Nand Flash Config */
#define NC_CONFIG_LOCK     (1 << 31)
#define NC_BLK_SIZE_MASK   (0x7 << 28)
#define NC_BLK_SIZE_2048K  (0x6 << 28)
#define NC_BLK_SIZE_1024K  (0x5 << 28)
#define NC_BLK_SIZE_512K   (0x4 << 28)
#define NC_BLK_SIZE_256K   (0x3 << 28)
#define NC_BLK_SIZE_128K   (0x2 << 28)
#define NC_BLK_SIZE_16K    (0x1 << 28)
#define NC_BLK_SIZE_8K     (0x0 << 28)
#define NC_DEV_SIZE_SHIFT  24
#define NC_DEV_SIZE_MASK   (0x0f << NC_DEV_SIZE_SHIFT)
#define NC_DEV_WIDTH_MASK  (1 << 23)
#define NC_DEV_WIDTH_16    (1 << 23)
#define NC_DEV_WIDTH_8     (0 << 23)
#define NC_PG_SIZE_MASK    (0x3 << 20)
#define NC_PG_SIZE_8K      (0x3 << 20)
#define NC_PG_SIZE_4K      (0x2 << 20)
#define NC_PG_SIZE_2K      (0x1 << 20)
#define NC_PG_SIZE_512B    (0x0 << 20)
#define NC_FUL_ADDR_SHIFT  16
#define NC_FUL_ADDR_MASK   (0x7 << NC_FUL_ADDR_SHIFT)
#define NC_BLK_ADDR_SHIFT  8
#define NC_BLK_ADDR_MASK   (0x07 << NC_BLK_ADDR_SHIFT)

   uint32 NandTiming1;  /* 0x58 */ /* Nand Flash Timing Parameters 1 */
#define NT_TREH_MASK        0x000f0000
#define NT_TREH_SHIFT       16
#define NT_TRP_MASK         0x00f00000
#define NT_TRP_SHIFT        20
   uint32 NandTiming2;  /* 0x5c */ /* Nand Flash Timing Parameters 2 */
#define NT_TREAD_MASK       0x0000000f
#define NT_TREAD_SHIFT      0
   /* 0x60 */
   uint32 NandAccControlCs1;  /* Nand Flash Access Control */
   uint32 NandConfigCs1;      /* Nand Flash Config */
   uint32 NandTiming1Cs1;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs1;     /* Nand Flash Timing Parameters 2 */
   /* 0x70 */
   uint32 NandAccControlCs2;  /* Nand Flash Access Control */
   uint32 NandConfigCs2;      /* Nand Flash Config */
   uint32 NandTiming1Cs2;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs2;     /* Nand Flash Timing Parameters 2 */
   /* 0x80 */
   uint32 NandAccControlCs3;  /* Nand Flash Access Control */
   uint32 NandConfigCs3;      /* Nand Flash Config */
   uint32 NandTiming1Cs3;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs3;     /* Nand Flash Timing Parameters 2 */
   /* 0x90 */
   uint32 NandAccControlCs4;  /* Nand Flash Access Control */
   uint32 NandConfigCs4;      /* Nand Flash Config */
   uint32 NandTiming1Cs4;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs4;     /* Nand Flash Timing Parameters 2 */
   /* 0xa0 */
   uint32 NandAccControlCs5;  /* Nand Flash Access Control */
   uint32 NandConfigCs5;      /* Nand Flash Config */
   uint32 NandTiming1Cs5;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs5;     /* Nand Flash Timing Parameters 2 */
   /* 0xb0 */
   uint32 NandAccControlCs6;  /* Nand Flash Access Control */
   uint32 NandConfigCs6;      /* Nand Flash Config */
   uint32 NandTiming1Cs6;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs6;     /* Nand Flash Timing Parameters 2 */

   /* 0xc0 */
   uint32 NandCorrStatThreshold; /* Correctable Error Reporting Threshold */
   uint32 NandCorrStatThresholdExt; /* Correctable Error Reporting
                   * Threshold */
   uint32 NandBlkWrProtect;   /* Block Write Protect Enable and Size */
               /*   for EBI_CS0b */
   uint32 NandMplaneOpcode1;

   /* 0xd0 */
   uint32 NandMplaneOpcode2;
   uint32 NandMplaneCtrl;
   uint32 NandReserved2[9];   /* 0xd8-0xfb */
   uint32 NandUncorrErrorCount;  /* 0xfc */

   /* 0x100 */
   uint32 NandCorrErrorCount;
   uint32 NandReadErrorCount; /* Read Error Count */
   uint32 NandBlockLockStatus;   /* Nand Flash Block Lock Status */
   uint32 NandEccCorrExtAddr; /* ECC Correctable Error Extended Address*/
   /* 0x110 */
   uint32 NandEccCorrAddr;    /* ECC Correctable Error Address */
   uint32 NandEccUncExtAddr;  /* ECC Uncorrectable Error Extended Addr */
   uint32 NandEccUncAddr;     /* ECC Uncorrectable Error Address */
   uint32 NandFlashReadExtAddr;  /* Flash Read Data Extended Address */
   /* 0x120 */
   uint32 NandFlashReadAddr;  /* Flash Read Data Address */
   uint32 NandProgramPageExtAddr;   /* Page Program Extended Address */
   uint32 NandProgramPageAddr;   /* Page Program Address */
   uint32 NandCopyBackExtAddr;   /* Copy Back Extended Address */
   /* 0x130 */
   uint32 NandCopyBackAddr;   /* Copy Back Address */
   uint32 NandBlockEraseExtAddr; /* Block Erase Extended Address */
   uint32 NandBlockEraseAddr; /* Block Erase Address */
   uint32 NandInvReadExtAddr; /* Flash Invalid Data Extended Address */
   /* 0x140 */
   uint32 NandInvReadAddr;    /* Flash Invalid Data Address */
   uint32 NandInitStatus;
   uint32 NandOnfiStatus;     /* ONFI Status */
   uint32 NandOnfiDebugData;  /* ONFI Debug Data */

   uint32 NandSemaphore;      /* 0x150 */ /* Semaphore */
   uint32 NandReserved3[16];  /* 0x154-0x193 */

   /* 0x194 */
   uint32 NandFlashDeviceId;  /* Nand Flash Device ID */
   uint32 NandFlashDeviceIdExt;  /* Nand Flash Extended Device ID */
   uint32 NandLlRdData;    /* Nand Flash Low Level Read Data */

   uint32 NandReserved4[24];  /* 0x1a0 - 0x1ff */

   /* 0x200 */
   uint32 NandSpareAreaReadOfs0; /* Nand Flash Spare Area Read Bytes 0-3 */
   uint32 NandSpareAreaReadOfs4; /* Nand Flash Spare Area Read Bytes 4-7 */
   uint32 NandSpareAreaReadOfs8; /* Nand Flash Spare Area Read Bytes 8-11 */
   uint32 NandSpareAreaReadOfsC; /* Nand Flash Spare Area Read Bytes 12-15*/
   /* 0x210 */
   uint32 NandSpareAreaReadOfs10;   /* Nand Flash Spare Area Read Bytes 16-19 */
   uint32 NandSpareAreaReadOfs14;   /* Nand Flash Spare Area Read Bytes 20-23 */
   uint32 NandSpareAreaReadOfs18;   /* Nand Flash Spare Area Read Bytes 24-27 */
   uint32 NandSpareAreaReadOfs1C;   /* Nand Flash Spare Area Read Bytes 28-31*/
   /* 0x220 */
   uint32 NandSpareAreaReadOfs20;   /* Nand Flash Spare Area Read Bytes 32-35 */
   uint32 NandSpareAreaReadOfs24;   /* Nand Flash Spare Area Read Bytes 36-39 */
   uint32 NandSpareAreaReadOfs28;   /* Nand Flash Spare Area Read Bytes 40-43 */
   uint32 NandSpareAreaReadOfs2C;   /* Nand Flash Spare Area Read Bytes 44-47*/
   /* 0x230 */
   uint32 NandSpareAreaReadOfs30;   /* Nand Flash Spare Area Read Bytes 48-51 */
   uint32 NandSpareAreaReadOfs34;   /* Nand Flash Spare Area Read Bytes 52-55 */
   uint32 NandSpareAreaReadOfs38;   /* Nand Flash Spare Area Read Bytes 56-59 */
   uint32 NandSpareAreaReadOfs3C;   /* Nand Flash Spare Area Read Bytes 60-63*/

   uint32 NandReserved5[16];  /* 0x240-0x27f */

   /* 0x280 */
   uint32 NandSpareAreaWriteOfs0;   /* Nand Flash Spare Area Write Bytes 0-3 */
   uint32 NandSpareAreaWriteOfs4;   /* Nand Flash Spare Area Write Bytes 4-7 */
   uint32 NandSpareAreaWriteOfs8;   /* Nand Flash Spare Area Write Bytes 8-11 */
   uint32 NandSpareAreaWriteOfsC;   /* Nand Flash Spare Area Write Bytes 12-15 */
   /* 0x290 */
   uint32 NandSpareAreaWriteOfs10;  /* Nand Flash Spare Area Write Bytes 16-19 */
   uint32 NandSpareAreaWriteOfs14;  /* Nand Flash Spare Area Write Bytes 20-23 */
   uint32 NandSpareAreaWriteOfs18;  /* Nand Flash Spare Area Write Bytes 24-27 */
   uint32 NandSpareAreaWriteOfs1C;  /* Nand Flash Spare Area Write Bytes 28-31 */
   /* 0x2a0 */
   uint32 NandSpareAreaWriteOfs20;  /* Nand Flash Spare Area Write Bytes 32-35 */
   uint32 NandSpareAreaWriteOfs24;  /* Nand Flash Spare Area Write Bytes 36-39 */
   uint32 NandSpareAreaWriteOfs28;  /* Nand Flash Spare Area Write Bytes 40-43 */
   uint32 NandSpareAreaWriteOfs2C;  /* Nand Flash Spare Area Write Bytes 44-47 */
   /* 0x2b0 */
   uint32 NandSpareAreaWriteOfs30;  /* Nand Flash Spare Area Write Bytes 48-51 */
   uint32 NandSpareAreaWriteOfs34;  /* Nand Flash Spare Area Write Bytes 52-55 */
   uint32 NandSpareAreaWriteOfs38;  /* Nand Flash Spare Area Write Bytes 56-59 */
   uint32 NandSpareAreaWriteOfs3C;  /* Nand Flash Spare Area Write Bytes 60-63 */
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
   uint32 NandReserved6[69];  /* 0x2ec-0x3ff */

   /* 0x400 */
   uint32 NandFlashCache[128];   /* 0x400-0x5ff */
} NandCtrlRegs;

#define NAND ((volatile NandCtrlRegs * const) NAND_REG_BASE)

/*
** NAND Interrupt Controller Registers
*/
typedef struct NandIntrCtrlRegs {
    uint32 NandInterrupt;
#define NINT_STS_MASK           0x00000fff
#define NINT_ECC_ERROR_CORR_SEC 0x00000800
#define NINT_ECC_ERROR_UNC_SEC  0x00000400
#define NINT_CTRL_READY_SEC     0x00000200
#define NINT_INV_ACC_SEC        0x00000100
#define NINT_ECC_ERROR_CORR     0x00000080
#define NINT_ECC_ERROR_UNC      0x00000040
#define NINT_DEV_RBPIN          0x00000020
#define NINT_CTRL_READY         0x00000010
#define NINT_PAGE_PGM           0x00000008
#define NINT_COPY_BACK          0x00000004
#define NINT_BLOCK_ERASE        0x00000002
#define NINT_NP_READ            0x00000001
    uint32 NandInterruptEn;
#define NINT_ENABLE_MASK        0x0000ffff
    uint32 NandBaseAddr0;   /* Default address when booting from NAND flash */
    uint32 NandBaseAddr1;   /* Secondary base address for NAND flash */
} NandIntrCtrlRegs;


#define NAND_CACHE ((volatile uint8 * const) NAND_CACHE_BASE)
#define NAND_INTR ((volatile NandIntrCtrlRegs * const) 0)

typedef struct GciStatusRegs
{
	uint32 pad1[0x10];
	uint32 status_idx_reg_wr;
	uint32 pad2[0x70];
	uint32 status_idx_reg_rd;
}GciStatusRegs;

#define GCI_BASE	((volatile GciStatusRegs*)GCI_REG_BASE)


#define GCI_CHIP_STATUS_REGISTER_INDEX_WR  GCI_WRAP_BASE+GCI_CHIP_STATUS_REGISTER_INDEX_WR_OFFSET
#define GCI_CHIP_STATUS_REGISTER_INDEX_RD  GCI_WRAP_BASE+GCI_CHIP_STATUS_REGISTER_INDEX_RD_OFFSET
#define GCI_CHIP_STATUS_REGISTER_INDEX_7 7
#define BOOT_MODE_MASK			0x3
#define BOOT_MODE_NAND			0x1

#endif

#ifdef __cplusplus
}
#endif

#endif
