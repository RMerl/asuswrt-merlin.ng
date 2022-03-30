/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_MX35_H
#define __ASM_ARCH_MX35_H

#define ARCH_MXC

/*
 * IRAM
 */
#define IRAM_BASE_ADDR		0x10000000	/* internal ram */
#define IRAM_SIZE		0x00020000	/* 128 KB */

#define LOW_LEVEL_SRAM_STACK	0x1001E000

/*
 * AIPS 1
 */
#define AIPS1_BASE_ADDR         0x43F00000
#define AIPS1_CTRL_BASE_ADDR    AIPS1_BASE_ADDR
#define MAX_BASE_ADDR           0x43F04000
#define EVTMON_BASE_ADDR        0x43F08000
#define CLKCTL_BASE_ADDR        0x43F0C000
#define I2C1_BASE_ADDR		0x43F80000
#define I2C3_BASE_ADDR          0x43F84000
#define ATA_BASE_ADDR           0x43F8C000
#define UART1_BASE		0x43F90000
#define UART2_BASE		0x43F94000
#define I2C2_BASE_ADDR          0x43F98000
#define CSPI1_BASE_ADDR         0x43FA4000
#define IOMUXC_BASE_ADDR        0x43FAC000

/*
 * SPBA
 */
#define SPBA_BASE_ADDR          0x50000000
#define UART3_BASE		0x5000C000
#define CSPI2_BASE_ADDR         0x50010000
#define ATA_DMA_BASE_ADDR       0x50020000
#define FEC_BASE_ADDR           0x50038000
#define SPBA_CTRL_BASE_ADDR     0x5003C000

/*
 * AIPS 2
 */
#define AIPS2_BASE_ADDR         0x53F00000
#define AIPS2_CTRL_BASE_ADDR    AIPS2_BASE_ADDR
#define CCM_BASE_ADDR           0x53F80000
#define GPT1_BASE_ADDR          0x53F90000
#define EPIT1_BASE_ADDR         0x53F94000
#define EPIT2_BASE_ADDR         0x53F98000
#define GPIO3_BASE_ADDR         0x53FA4000
#define MMC_SDHC1_BASE_ADDR	0x53FB4000
#define MMC_SDHC2_BASE_ADDR	0x53FB8000
#define MMC_SDHC3_BASE_ADDR	0x53FBC000
#define IPU_CTRL_BASE_ADDR	0x53FC0000
#define GPIO1_BASE_ADDR		0x53FCC000
#define GPIO2_BASE_ADDR		0x53FD0000
#define SDMA_BASE_ADDR		0x53FD4000
#define RTC_BASE_ADDR		0x53FD8000
#define WDOG1_BASE_ADDR		0x53FDC000
#define PWM_BASE_ADDR		0x53FE0000
#define RTIC_BASE_ADDR		0x53FEC000
#define IIM_BASE_ADDR		0x53FF0000
#define IMX_USB_BASE		0x53FF4000
#define IMX_USB_PORT_OFFSET	0x400

#define IMX_CCM_BASE		CCM_BASE_ADDR

/*
 * ROMPATCH and AVIC
 */
#define ROMPATCH_BASE_ADDR	0x60000000
#define AVIC_BASE_ADDR		0x68000000

/*
 * NAND, SDRAM, WEIM, M3IF, EMI controllers
 */
#define EXT_MEM_CTRL_BASE	0xB8000000
#define ESDCTL_BASE_ADDR	0xB8001000
#define WEIM_BASE_ADDR		0xB8002000
#define WEIM_CTRL_CS0		WEIM_BASE_ADDR
#define WEIM_CTRL_CS1		(WEIM_BASE_ADDR + 0x10)
#define WEIM_CTRL_CS2		(WEIM_BASE_ADDR + 0x20)
#define WEIM_CTRL_CS3		(WEIM_BASE_ADDR + 0x30)
#define WEIM_CTRL_CS4		(WEIM_BASE_ADDR + 0x40)
#define WEIM_CTRL_CS5		(WEIM_BASE_ADDR + 0x50)
#define M3IF_BASE_ADDR		0xB8003000
#define EMI_BASE_ADDR		0xB8004000

#define NFC_BASE_ADDR		0xBB000000

/*
 * Memory regions and CS
 */
#define IPU_MEM_BASE_ADDR	0x70000000
#define CSD0_BASE_ADDR		0x80000000
#define CSD1_BASE_ADDR		0x90000000
#define CS0_BASE_ADDR		0xA0000000
#define CS1_BASE_ADDR		0xA8000000
#define CS2_BASE_ADDR		0xB0000000
#define CS3_BASE_ADDR		0xB2000000
#define CS4_BASE_ADDR		0xB4000000
#define CS5_BASE_ADDR		0xB6000000

/*
 * IRQ Controller Register Definitions.
 */
#define AVIC_NIMASK		0x04
#define AVIC_INTTYPEH		0x18
#define AVIC_INTTYPEL		0x1C

/* L210 */
#define L2CC_BASE_ADDR		0x30000000
#define L2_CACHE_LINE_SIZE		32
#define L2_CACHE_CTL_REG		0x100
#define L2_CACHE_AUX_CTL_REG		0x104
#define L2_CACHE_SYNC_REG		0x730
#define L2_CACHE_INV_LINE_REG		0x770
#define L2_CACHE_INV_WAY_REG		0x77C
#define L2_CACHE_CLEAN_LINE_REG		0x7B0
#define L2_CACHE_CLEAN_INV_LINE_REG	0x7F0
#define L2_CACHE_DBG_CTL_REG		0xF40

#define CLKMODE_AUTO		0
#define CLKMODE_CONSUMER	1

#define PLL_PD(x)		(((x) & 0xf) << 26)
#define PLL_MFD(x)		(((x) & 0x3ff) << 16)
#define PLL_MFI(x)		(((x) & 0xf) << 10)
#define PLL_MFN(x)		(((x) & 0x3ff) << 0)

#define _PLL_BRM(x)	((x) << 31)
#define _PLL_PD(x)	(((x) - 1) << 26)
#define _PLL_MFD(x)	(((x) - 1) << 16)
#define _PLL_MFI(x)	((x) << 10)
#define _PLL_MFN(x)	(x)
#define _PLL_SETTING(brm, pd, mfd, mfi, mfn) \
	(_PLL_BRM(brm) | _PLL_PD(pd) | _PLL_MFD(mfd) | _PLL_MFI(mfi) |\
	 _PLL_MFN(mfn))

#define CCM_MPLL_532_HZ	_PLL_SETTING(1, 1, 12, 11, 1)
#define CCM_MPLL_399_HZ _PLL_SETTING(0, 1, 16, 8, 5)
#define CCM_PPLL_300_HZ _PLL_SETTING(0, 1, 4, 6, 1)

#define CSCR_U(x)	(WEIM_CTRL_CS#x + 0)
#define CSCR_L(x)	(WEIM_CTRL_CS#x + 4)
#define CSCR_A(x)	(WEIM_CTRL_CS#x + 8)

#define IIM_SREV	0x24
#define ROMPATCH_REV	0x40

#define IPU_CONF	IPU_CTRL_BASE_ADDR

#define IPU_CONF_PXL_ENDIAN	(1<<8)
#define IPU_CONF_DU_EN		(1<<7)
#define IPU_CONF_DI_EN		(1<<6)
#define IPU_CONF_ADC_EN		(1<<5)
#define IPU_CONF_SDC_EN		(1<<4)
#define IPU_CONF_PF_EN		(1<<3)
#define IPU_CONF_ROT_EN		(1<<2)
#define IPU_CONF_IC_EN		(1<<1)
#define IPU_CONF_CSI_EN		(1<<0)

/*
 * CSPI register definitions
 */
#define MXC_CSPI
#define MXC_CSPICTRL_EN		(1 << 0)
#define MXC_CSPICTRL_MODE	(1 << 1)
#define MXC_CSPICTRL_XCH	(1 << 2)
#define MXC_CSPICTRL_SMC	(1 << 3)
#define MXC_CSPICTRL_POL	(1 << 4)
#define MXC_CSPICTRL_PHA	(1 << 5)
#define MXC_CSPICTRL_SSCTL	(1 << 6)
#define MXC_CSPICTRL_SSPOL	(1 << 7)
#define MXC_CSPICTRL_CHIPSELECT(x)	(((x) & 0x3) << 12)
#define MXC_CSPICTRL_BITCOUNT(x)	(((x) & 0xfff) << 20)
#define MXC_CSPICTRL_DATARATE(x)	(((x) & 0x7) << 16)
#define MXC_CSPICTRL_TC		(1 << 7)
#define MXC_CSPICTRL_RXOVF	(1 << 6)
#define MXC_CSPICTRL_MAXBITS	0xfff
#define MXC_CSPIPERIOD_32KHZ	(1 << 15)
#define MAX_SPI_BYTES	4

#define MXC_SPI_BASE_ADDRESSES \
	0x43fa4000, \
	0x50010000,

#define GPIO_PORT_NUM		3
#define GPIO_NUM_PIN		32

#define CHIP_REV_1_0		0x10
#define CHIP_REV_2_0		0x20

#define BOARD_REV_1_0		0x0
#define BOARD_REV_2_0		0x1

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* Clock Control Module (CCM) registers */
struct ccm_regs {
	u32 ccmr;	/* Control */
	u32 pdr0;	/* Post divider 0 */
	u32 pdr1;	/* Post divider 1 */
	u32 pdr2;	/* Post divider 2 */
	u32 pdr3;	/* Post divider 3 */
	u32 pdr4;	/* Post divider 4 */
	u32 rcsr;	/* CCM Status */
	u32 mpctl;	/* Core PLL Control */
	u32 ppctl;	/* Peripheral PLL Control */
	u32 acmr;	/* Audio clock mux */
	u32 cosr;	/* Clock out source */
	u32 cgr0;	/* Clock Gating Control 0 */
	u32 cgr1;	/* Clock Gating Control 1 */
	u32 cgr2;	/* Clock Gating Control 2 */
	u32 cgr3;	/* Clock Gating Control 3 */
	u32 reserved;
	u32 dcvr0;	/* DPTC Comparator 0 */
	u32 dcvr1;	/* DPTC Comparator 0 */
	u32 dcvr2;	/* DPTC Comparator 0 */
	u32 dcvr3;	/* DPTC Comparator 0 */
	u32 ltr0;	/* Load Tracking 0 */
	u32 ltr1;	/* Load Tracking 1 */
	u32 ltr2;	/* Load Tracking 2 */
	u32 ltr3;	/* Load Tracking 3 */
	u32 ltbr0;	/* Load Tracking Buffer 0 */
};

/* IIM control registers */
struct iim_regs {
	u32 iim_stat;
	u32 iim_statm;
	u32 iim_err;
	u32 iim_emask;
	u32 iim_fctl;
	u32 iim_ua;
	u32 iim_la;
	u32 iim_sdat;
	u32 iim_prev;
	u32 iim_srev;
	u32 iim_prg_p;
	u32 iim_scs0;
	u32 iim_scs1;
	u32 iim_scs2;
	u32 iim_scs3;
	u32 res1[0x1f1];
	struct fuse_bank {
		u32 fuse_regs[0x20];
		u32 fuse_rsvd[0xe0];
	} bank[3];
};

struct fuse_bank0_regs {
	u32 fuse0_7[8];
	u32 uid[8];
	u32 fuse16_31[0x10];
};

struct fuse_bank1_regs {
	u32 fuse0_21[0x16];
	u32 usr;
	u32 fuse23_31[9];
};

/* General Purpose Timer (GPT) registers */
struct gpt_regs {
	u32 ctrl;	/* control */
	u32 pre;	/* prescaler */
	u32 stat;	/* status */
	u32 intr;	/* interrupt */
	u32 cmp[3];	/* output compare 1-3 */
	u32 capt[2];	/* input capture 1-2 */
	u32 counter;	/* counter */
};

/* CSPI registers */
struct cspi_regs {
	u32 rxdata;
	u32 txdata;
	u32 ctrl;
	u32 intr;
	u32 dma;
	u32 stat;
	u32 period;
	u32 test;
};

struct esdc_regs {
	u32	esdctl0;
	u32	esdcfg0;
	u32	esdctl1;
	u32	esdcfg1;
	u32	esdmisc;
	u32	reserved[4];
	u32	esdcdly[5];
	u32	esdcdlyl;
};

#define ESDC_MISC_RST		(1 << 1)
#define ESDC_MISC_MDDR_EN	(1 << 2)
#define ESDC_MISC_MDDR_DL_RST	(1 << 3)
#define ESDC_MISC_DDR_EN	(1 << 8)
#define ESDC_MISC_DDR2_EN	(1 << 9)

/* Multi-Layer AHB Crossbar Switch (MAX) registers */
struct max_regs {
	u32 mpr0;
	u32 pad00[3];
	u32 sgpcr0;
	u32 pad01[59];
	u32 mpr1;
	u32 pad02[3];
	u32 sgpcr1;
	u32 pad03[59];
	u32 mpr2;
	u32 pad04[3];
	u32 sgpcr2;
	u32 pad05[59];
	u32 mpr3;
	u32 pad06[3];
	u32 sgpcr3;
	u32 pad07[59];
	u32 mpr4;
	u32 pad08[3];
	u32 sgpcr4;
	u32 pad09[251];
	u32 mgpcr0;
	u32 pad10[63];
	u32 mgpcr1;
	u32 pad11[63];
	u32 mgpcr2;
	u32 pad12[63];
	u32 mgpcr3;
	u32 pad13[63];
	u32 mgpcr4;
	u32 pad14[63];
	u32 mgpcr5;
};

/* AHB <-> IP-Bus Interface (AIPS) */
struct aips_regs {
	u32 mpr_0_7;
	u32 mpr_8_15;
	u32 pad0[6];
	u32 pacr_0_7;
	u32 pacr_8_15;
	u32 pacr_16_23;
	u32 pacr_24_31;
	u32 pad1[4];
	u32 opacr_0_7;
	u32 opacr_8_15;
	u32 opacr_16_23;
	u32 opacr_24_31;
	u32 opacr_32_39;
};

/*
 * NFMS bit in RCSR register for pagesize of nandflash
 */
#define NFMS_BIT		8
#define NFMS_NF_DWIDTH		14
#define NFMS_NF_PG_SZ		8

#define CCM_RCSR_NF_16BIT_SEL	(1 << 14)

#endif

/*
 * Generic timer support
 */
#ifdef CONFIG_MX35_CLK32
#define	CONFIG_SYS_TIMER_RATE	CONFIG_MX35_CLK32
#else
#define	CONFIG_SYS_TIMER_RATE	32768
#endif

#define CONFIG_SYS_TIMER_COUNTER	(GPT1_BASE_ADDR+36)

#endif /* __ASM_ARCH_MX35_H */
