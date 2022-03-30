/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009, DENX Software Engineering
 * Author: John Rigby <jcrigby@gmail.com
 *
 *   Based on arch-mx31/imx-regs.h
 *	Copyright (C) 2009 Ilya Yanok,
 *		Emcraft Systems <yanok@emcraft.com>
 *   and arch-mx27/imx-regs.h
 *	Copyright (C) 2007 Pengutronix,
 *		Sascha Hauer <s.hauer@pengutronix.de>
 *	Copyright (C) 2009 Ilya Yanok,
 *		Emcraft Systems <yanok@emcraft.com>
 */

#ifndef _IMX_REGS_H
#define _IMX_REGS_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* Clock Control Module (CCM) registers */
struct ccm_regs {
	u32 mpctl;	/* Core PLL Control */
	u32 upctl;	/* USB PLL Control */
	u32 cctl;	/* Clock Control */
	u32 cgr0;	/* Clock Gating Control 0 */
	u32 cgr1;	/* Clock Gating Control 1 */
	u32 cgr2;	/* Clock Gating Control 2 */
	u32 pcdr[4];	/* PER Clock Dividers */
	u32 rcsr;	/* CCM Status */
	u32 crdr;	/* CCM Reset and Debug */
	u32 dcvr0;	/* DPTC Comparator Value 0 */
	u32 dcvr1;	/* DPTC Comparator Value 1 */
	u32 dcvr2;	/* DPTC Comparator Value 2 */
	u32 dcvr3;	/* DPTC Comparator Value 3 */
	u32 ltr0;	/* Load Tracking 0 */
	u32 ltr1;	/* Load Tracking 1 */
	u32 ltr2;	/* Load Tracking 2 */
	u32 ltr3;	/* Load Tracking 3 */
	u32 ltbr0;	/* Load Tracking Buffer 0 */
	u32 ltbr1;	/* Load Tracking Buffer 1 */
	u32 pcmr0;	/* Power Management Control 0 */
	u32 pcmr1;	/* Power Management Control 1 */
	u32 pcmr2;	/* Power Management Control 2 */
	u32 mcr;	/* Miscellaneous Control */
	u32 lpimr0;	/* Low Power Interrupt Mask 0 */
	u32 lpimr1;	/* Low Power Interrupt Mask 1 */
};

/* Enhanced SDRAM Controller (ESDRAMC) registers */
struct esdramc_regs {
	u32 ctl0; 	/* control 0 */
	u32 cfg0; 	/* configuration 0 */
	u32 ctl1; 	/* control 1 */
	u32 cfg1; 	/* configuration 1 */
	u32 misc; 	/* miscellaneous */
	u32 pad[3];
	u32 cdly1;	/* Delay Line 1 configuration debug */
	u32 cdly2;	/* delay line 2 configuration debug */
	u32 cdly3;	/* delay line 3 configuration debug */
	u32 cdly4;	/* delay line 4 configuration debug */
	u32 cdly5;	/* delay line 5 configuration debug */
	u32 cdlyl;	/* delay line cycle length debug */
};

/* General Purpose Timer (GPT) registers */
struct gpt_regs {
	u32 ctrl;   	/* control */
	u32 pre;    	/* prescaler */
	u32 stat;   	/* status */
	u32 intr;   	/* interrupt */
	u32 cmp[3]; 	/* output compare 1-3 */
	u32 capt[2];	/* input capture 1-2 */
	u32 counter;	/* counter */
};

/* Watchdog Timer (WDOG) registers */
struct wdog_regs {
	u16 wcr;	/* Control */
	u16 wsr;	/* Service */
	u16 wrsr;	/* Reset Status */
	u16 wicr;	/* Interrupt Control */
	u16 wmcr;	/* Misc Control */
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
	u32 fuse16_25[0xa];
	u32 mac_addr[6];
};

struct fuse_bank1_regs {
	u32 fuse0_21[0x16];
	u32 usr5;
	u32 fuse23_29[7];
	u32 usr6[2];
};

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
};

/* AHB <-> IP-Bus Interface (AIPS) */
struct aips_regs {
	u32 mpr_0_7;
	u32 mpr_8_15;
};
/* LCD controller registers */
struct lcdc_regs {
	u32 lssar;	/* Screen Start Address */
	u32 lsr;	/* Size */
	u32 lvpwr;	/* Virtual Page Width */
	u32 lcpr;	/* Cursor Position */
	u32 lcwhb;	/* Cursor Width Height and Blink */
	u32 lccmr;	/* Color Cursor Mapping */
	u32 lpcr;	/* Panel Configuration */
	u32 lhcr;	/* Horizontal Configuration */
	u32 lvcr;	/* Vertical Configuration */
	u32 lpor;	/* Panning Offset */
	u32 lscr;	/* Sharp Configuration */
	u32 lpccr;	/* PWM Contrast Control */
	u32 ldcr;	/* DMA Control */
	u32 lrmcr;	/* Refresh Mode Control */
	u32 licr;	/* Interrupt Configuration */
	u32 lier;	/* Interrupt Enable */
	u32 lisr;	/* Interrupt Status */
	u32 res0[3];
	u32 lgwsar;	/* Graphic Window Start Address */
	u32 lgwsr;	/* Graphic Window Size */
	u32 lgwvpwr;	/* Graphic Window Virtual Page Width Regist */
	u32 lgwpor;	/* Graphic Window Panning Offset */
	u32 lgwpr;	/* Graphic Window Position */
	u32 lgwcr;	/* Graphic Window Control */
	u32 lgwdcr;	/* Graphic Window DMA Control */
	u32 res1[5];
	u32 lauscr;	/* AUS Mode Control */
	u32 lausccr;	/* AUS mode Cursor Control */
	u32 res2[31 + 64*7];
	u32 bglut;	/* Background Lookup Table */
	u32 gwlut;	/* Graphic Window Lookup Table */
};

/* Wireless External Interface Module Registers */
struct weim_regs {
	u32 cscr0u;	/* Chip Select 0 Upper Register */
	u32 cscr0l;	/* Chip Select 0 Lower Register */
	u32 cscr0a;	/* Chip Select 0 Addition Register */
	u32 pad0;
	u32 cscr1u;	/* Chip Select 1 Upper Register */
	u32 cscr1l;	/* Chip Select 1 Lower Register */
	u32 cscr1a;	/* Chip Select 1 Addition Register */
	u32 pad1;
	u32 cscr2u;	/* Chip Select 2 Upper Register */
	u32 cscr2l;	/* Chip Select 2 Lower Register */
	u32 cscr2a;	/* Chip Select 2 Addition Register */
	u32 pad2;
	u32 cscr3u;	/* Chip Select 3 Upper Register */
	u32 cscr3l;	/* Chip Select 3 Lower Register */
	u32 cscr3a;	/* Chip Select 3 Addition Register */
	u32 pad3;
	u32 cscr4u;	/* Chip Select 4 Upper Register */
	u32 cscr4l;	/* Chip Select 4 Lower Register */
	u32 cscr4a;	/* Chip Select 4 Addition Register */
	u32 pad4;
	u32 cscr5u;	/* Chip Select 5 Upper Register */
	u32 cscr5l;	/* Chip Select 5 Lower Register */
	u32 cscr5a;	/* Chip Select 5 Addition Register */
	u32 pad5;
	u32 wcr;	/* WEIM Configuration Register */
};

/* Multi-Master Memory Interface */
struct m3if_regs {
	u32 ctl;	/* Control Register */
	u32 wcfg0;	/* Watermark Configuration Register 0 */
	u32 wcfg1;	/* Watermark Configuration Register1 */
	u32 wcfg2;	/* Watermark Configuration Register2 */
	u32 wcfg3;	/* Watermark Configuration Register 3 */
	u32 wcfg4;	/* Watermark Configuration Register 4 */
	u32 wcfg5;	/* Watermark Configuration Register 5 */
	u32 wcfg6;	/* Watermark Configuration Register 6 */
	u32 wcfg7;	/* Watermark Configuration Register 7 */
	u32 wcsr;	/* Watermark Control and Status Register */
	u32 scfg0;	/* Snooping Configuration Register 0 */
	u32 scfg1;	/* Snooping Configuration Register 1 */
	u32 scfg2;	/* Snooping Configuration Register 2 */
	u32 ssr0;	/* Snooping Status Register 0 */
	u32 ssr1;	/* Snooping Status Register 1 */
	u32 res0;
	u32 mlwe0;	/* Master Lock WEIM CS0 Register */
	u32 mlwe1;	/* Master Lock WEIM CS1 Register */
	u32 mlwe2;	/* Master Lock WEIM CS2 Register */
	u32 mlwe3;	/* Master Lock WEIM CS3 Register */
	u32 mlwe4;	/* Master Lock WEIM CS4 Register */
	u32 mlwe5;	/* Master Lock WEIM CS5 Register */
};

/* Pulse width modulation */
struct pwm_regs {
	u32 cr;	/* Control Register */
	u32 sr;	/* Status Register */
	u32 ir;	/* Interrupt Register */
	u32 sar;	/* Sample Register */
	u32 pr;	/* Period Register */
	u32 cnr;	/* Counter Register */
};

/* Enhanced Periodic Interrupt Timer */
struct epit_regs {
	u32 cr;	/* Control register */
	u32 sr;	/* Status register */
	u32 lr;	/* Load register */
	u32 cmpr;	/* Compare register */
	u32 cnr;	/* Counter register */
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

#endif

#define ARCH_MXC

/* AIPS 1 */
#define IMX_AIPS1_BASE		(0x43F00000)
#define IMX_MAX_BASE		(0x43F04000)
#define IMX_CLKCTL_BASE		(0x43F08000)
#define IMX_ETB_SLOT4_BASE	(0x43F0C000)
#define IMX_ETB_SLOT5_BASE	(0x43F10000)
#define IMX_ECT_CTIO_BASE	(0x43F18000)
#define I2C1_BASE_ADDR		(0x43F80000)
#define I2C3_BASE_ADDR		(0x43F84000)
#define IMX_CAN1_BASE		(0x43F88000)
#define IMX_CAN2_BASE		(0x43F8C000)
#define UART1_BASE		(0x43F90000)
#define UART2_BASE		(0x43F94000)
#define I2C2_BASE_ADDR		(0x43F98000)
#define IMX_OWIRE_BASE		(0x43F9C000)
#define IMX_CSPI1_BASE		(0x43FA4000)
#define IMX_KPP_BASE		(0x43FA8000)
#define IMX_IOPADMUX_BASE	(0x43FAC000)
#define IOMUXC_BASE_ADDR	IMX_IOPADMUX_BASE
#define IMX_IOPADCTL_BASE	(0x43FAC22C)
#define IMX_IOPADGRPCTL_BASE	(0x43FAC418)
#define IMX_IOPADINPUTSEL_BASE	(0x43FAC460)
#define IMX_AUDMUX_BASE		(0x43FB0000)
#define IMX_ECT_IP1_BASE	(0x43FB8000)
#define IMX_ECT_IP2_BASE	(0x43FBC000)

/* SPBA */
#define IMX_SPBA_BASE		(0x50000000)
#define IMX_CSPI3_BASE		(0x50004000)
#define UART4_BASE		(0x50008000)
#define UART3_BASE		(0x5000C000)
#define IMX_CSPI2_BASE		(0x50010000)
#define IMX_SSI2_BASE		(0x50014000)
#define IMX_ESAI_BASE		(0x50018000)
#define IMX_ATA_DMA_BASE	(0x50020000)
#define IMX_SIM1_BASE		(0x50024000)
#define IMX_SIM2_BASE		(0x50028000)
#define UART5_BASE		(0x5002C000)
#define IMX_TSC_BASE		(0x50030000)
#define IMX_SSI1_BASE		(0x50034000)
#define IMX_FEC_BASE		(0x50038000)
#define IMX_SPBA_CTRL_BASE	(0x5003C000)

/* AIPS 2 */
#define IMX_AIPS2_BASE		(0x53F00000)
#define IMX_CCM_BASE		(0x53F80000)
#define IMX_GPT4_BASE		(0x53F84000)
#define IMX_GPT3_BASE		(0x53F88000)
#define IMX_GPT2_BASE		(0x53F8C000)
#define IMX_GPT1_BASE		(0x53F90000)
#define IMX_EPIT1_BASE		(0x53F94000)
#define IMX_EPIT2_BASE		(0x53F98000)
#define IMX_GPIO4_BASE		(0x53F9C000)
#define IMX_PWM2_BASE		(0x53FA0000)
#define IMX_GPIO3_BASE		(0x53FA4000)
#define IMX_PWM3_BASE		(0x53FA8000)
#define IMX_SCC_BASE		(0x53FAC000)
#define IMX_SCM_BASE		(0x53FAE000)
#define IMX_SMN_BASE		(0x53FAF000)
#define IMX_RNGD_BASE		(0x53FB0000)
#define IMX_MMC_SDHC1_BASE	(0x53FB4000)
#define IMX_MMC_SDHC2_BASE	(0x53FB8000)
#define IMX_LCDC_BASE		(0x53FBC000)
#define IMX_SLCDC_BASE		(0x53FC0000)
#define IMX_PWM4_BASE		(0x53FC8000)
#define IMX_GPIO1_BASE		(0x53FCC000)
#define IMX_GPIO2_BASE		(0x53FD0000)
#define IMX_SDMA_BASE		(0x53FD4000)
#define IMX_WDT_BASE		(0x53FDC000)
#define WDOG1_BASE_ADDR	IMX_WDT_BASE
#define IMX_PWM1_BASE		(0x53FE0000)
#define IMX_RTIC_BASE		(0x53FEC000)
#define IMX_IIM_BASE		(0x53FF0000)
#define IIM_BASE_ADDR		IMX_IIM_BASE
#define IMX_USB_BASE		(0x53FF4000)
/*
 * This is in contradiction to the imx25 reference manual, which says that
 * port 1's registers start at 0x53FF4200. The correct base address for
 * port 1 is 0x53FF4400. The kernel uses 0x53FF4400 as well.
 */
#define IMX_USB_PORT_OFFSET	0x400
#define IMX_CSI_BASE		(0x53FF8000)
#define IMX_DRYICE_BASE		(0x53FFC000)

#define IMX_ARM926_ROMPATCH	(0x60000000)
#define IMX_ARM926_ASIC		(0x68000000)

/* 128K Internal Static RAM */
#define IMX_RAM_BASE		(0x78000000)
#define IMX_RAM_SIZE		(128 * 1024)

/* SDRAM BANKS */
#define IMX_SDRAM_BANK0_BASE	(0x80000000)
#define IMX_SDRAM_BANK1_BASE	(0x90000000)

#define IMX_WEIM_CS0		(0xA0000000)
#define IMX_WEIM_CS1		(0xA8000000)
#define IMX_WEIM_CS2		(0xB0000000)
#define IMX_WEIM_CS3		(0xB2000000)
#define IMX_WEIM_CS4		(0xB4000000)
#define IMX_ESDRAMC_BASE	(0xB8001000)
#define IMX_WEIM_CTRL_BASE	(0xB8002000)
#define IMX_M3IF_CTRL_BASE	(0xB8003000)
#define IMX_EMI_CTRL_BASE	(0xB8004000)

/* NAND Flash Controller */
#define IMX_NFC_BASE		(0xBB000000)
#define NFC_BASE_ADDR		IMX_NFC_BASE

/* CCM bitfields */
#define CCM_PLL_MFI_SHIFT	10
#define CCM_PLL_MFI_MASK	0xf
#define CCM_PLL_MFN_SHIFT	0
#define CCM_PLL_MFN_MASK	0x3ff
#define CCM_PLL_MFD_SHIFT	16
#define CCM_PLL_MFD_MASK	0x3ff
#define CCM_PLL_PD_SHIFT	26
#define CCM_PLL_PD_MASK		0xf
#define CCM_CCTL_ARM_DIV_SHIFT	30
#define CCM_CCTL_ARM_DIV_MASK	3
#define CCM_CCTL_AHB_DIV_SHIFT	28
#define CCM_CCTL_AHB_DIV_MASK	3
#define CCM_CCTL_ARM_SRC	(1 << 14)
#define CCM_CGR1_GPT1		(1 << 19)
#define CCM_PERCLK_REG(clk)	(clk / 4)
#define CCM_PERCLK_SHIFT(clk)	(8 * (clk % 4))
#define CCM_PERCLK_MASK		0x3f
#define CCM_RCSR_NF_16BIT_SEL	(1 << 14)
#define CCM_RCSR_NF_PS(v)	((v >> 26) & 3)
#define CCM_CRDR_BT_UART_SRC_SHIFT	29
#define CCM_CRDR_BT_UART_SRC_MASK	7

/* ESDRAM Controller register bitfields */
#define ESDCTL_PRCT(x)		(((x) & 0x3f) << 0)
#define ESDCTL_BL		(1 << 7)
#define ESDCTL_FP		(1 << 8)
#define ESDCTL_PWDT(x)		(((x) & 3) << 10)
#define ESDCTL_SREFR(x)		(((x) & 7) << 13)
#define ESDCTL_DSIZ_16_UPPER	(0 << 16)
#define ESDCTL_DSIZ_16_LOWER	(1 << 16)
#define ESDCTL_DSIZ_32		(2 << 16)
#define ESDCTL_COL8		(0 << 20)
#define ESDCTL_COL9		(1 << 20)
#define ESDCTL_COL10		(2 << 20)
#define ESDCTL_ROW11		(0 << 24)
#define ESDCTL_ROW12		(1 << 24)
#define ESDCTL_ROW13		(2 << 24)
#define ESDCTL_ROW14		(3 << 24)
#define ESDCTL_ROW15		(4 << 24)
#define ESDCTL_SP		(1 << 27)
#define ESDCTL_SMODE_NORMAL	(0 << 28)
#define ESDCTL_SMODE_PRECHARGE	(1 << 28)
#define ESDCTL_SMODE_AUTO_REF	(2 << 28)
#define ESDCTL_SMODE_LOAD_MODE	(3 << 28)
#define ESDCTL_SMODE_MAN_REF	(4 << 28)
#define ESDCTL_SDE		(1 << 31)

#define ESDCFG_TRC(x)		(((x) & 0xf) << 0)
#define ESDCFG_TRCD(x)		(((x) & 0x7) << 4)
#define ESDCFG_TCAS(x)		(((x) & 0x3) << 8)
#define ESDCFG_TRRD(x)		(((x) & 0x3) << 10)
#define ESDCFG_TRAS(x)		(((x) & 0x7) << 12)
#define ESDCFG_TWR		(1 << 15)
#define ESDCFG_TMRD(x)		(((x) & 0x3) << 16)
#define ESDCFG_TRP(x)		(((x) & 0x3) << 18)
#define ESDCFG_TWTR		(1 << 20)
#define ESDCFG_TXP(x)		(((x) & 0x3) << 21)

#define ESDMISC_RST		(1 << 1)
#define ESDMISC_MDDREN		(1 << 2)
#define ESDMISC_MDDR_DL_RST	(1 << 3)
#define ESDMISC_MDDR_MDIS	(1 << 4)
#define ESDMISC_LHD		(1 << 5)
#define ESDMISC_MA10_SHARE	(1 << 6)
#define ESDMISC_SDRAM_RDY	(1 << 31)

/* GPT bits */
#define GPT_CTRL_SWR		(1 << 15)	/* Software reset */
#define GPT_CTRL_FRR		(1 << 9)	/* Freerun / restart */
#define GPT_CTRL_CLKSOURCE_32	(4 << 6)	/* Clock source	*/
#define GPT_CTRL_TEN		1		/* Timer enable	*/

/* WDOG enable */
#define WCR_WDE 		0x04
#define WSR_UNLOCK1		0x5555
#define WSR_UNLOCK2		0xAAAA

/* MAX bits */
#define MAX_MGPCR_AULB(x)	(((x) & 0x7) << 0)

/* M3IF bits */
#define M3IF_CTL_MRRP(x)	(((x) & 0xff) << 0)

/* WEIM bits */
/* 13 fields of the upper CS control register */
#define WEIM_CSCR_U(sp, wp, bcd, bcs, psz, pme, sync, dol, \
		cnc, wsc, ew, wws, edc) \
		((sp) << 31 | (wp) << 30 | (bcd) << 28 | (bcs) << 24 | \
		(psz) << 22 | (pme) << 21 | (sync) << 20 | (dol) << 16 | \
		(cnc) << 14 | (wsc) << 8 | (ew) << 7 | (wws) << 4 | (edc) << 0)
/* 12 fields of the lower CS control register */
#define WEIM_CSCR_L(oea, oen, ebwa, ebwn, \
		csa, ebc, dsz, csn, psr, cre, wrap, csen) \
		((oea) << 28 | (oen) << 24 | (ebwa) << 20 | (ebwn) << 16 |\
		(csa) << 12 | (ebc) << 11 | (dsz) << 8 | (csn) << 4 |\
		(psr) << 3 | (cre) << 2 | (wrap) << 1 | (csen) << 0)
/* 14 fields of the additional CS control register */
#define WEIM_CSCR_A(ebra, ebrn, rwa, rwn, mum, lah, lbn, lba, dww, dct, \
		wwu, age, cnc2, fce) \
		((ebra) << 28 | (ebrn) << 24 | (rwa) << 20 | (rwn) << 16 |\
		(mum) << 15 | (lah) << 13 | (lbn) << 10 | (lba) << 8 |\
		(dww) << 6 | (dct) << 4 | (wwu) << 3 |\
		(age) << 2 | (cnc2) << 1 | (fce) << 0)

/* Names used in GPIO driver */
#define GPIO1_BASE_ADDR		IMX_GPIO1_BASE
#define GPIO2_BASE_ADDR		IMX_GPIO2_BASE
#define GPIO3_BASE_ADDR		IMX_GPIO3_BASE
#define GPIO4_BASE_ADDR		IMX_GPIO4_BASE

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
	IMX_CSPI1_BASE, \
	IMX_CSPI2_BASE, \
	IMX_CSPI3_BASE

#endif				/* _IMX_REGS_H */
