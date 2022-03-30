/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#ifndef _ASM_ARCH_SCG_H
#define _ASM_ARCH_SCG_H

#include <common.h>

#ifdef CONFIG_CLK_DEBUG
#define clk_debug(fmt, args...)	printf(fmt, ##args)
#else
#define clk_debug(fmt, args...)
#endif

#define SCG_CCR_SCS_SHIFT		(24)
#define SCG_CCR_SCS_MASK		((0xFUL) << SCG_CCR_SCS_SHIFT)
#define SCG_CCR_DIVCORE_SHIFT		(16)
#define SCG_CCR_DIVCORE_MASK		((0xFUL) << SCG_CCR_DIVCORE_SHIFT)
#define SCG_CCR_DIVPLAT_SHIFT		(12)
#define SCG_CCR_DIVPLAT_MASK		((0xFUL) << SCG_CCR_DIVPLAT_SHIFT)
#define SCG_CCR_DIVEXT_SHIFT		(8)
#define SCG_CCR_DIVEXT_MASK		((0xFUL) << SCG_CCR_DIVEXT_SHIFT)
#define SCG_CCR_DIVBUS_SHIFT		(4)
#define SCG_CCR_DIVBUS_MASK		((0xFUL) << SCG_CCR_DIVBUS_SHIFT)
#define SCG_CCR_DIVSLOW_SHIFT		(0)
#define SCG_CCR_DIVSLOW_MASK		((0xFUL) << SCG_CCR_DIVSLOW_SHIFT)

/* SCG DDR Clock Control Register */
#define SCG_DDRCCR_DDRCS_SHIFT		(24)
#define SCG_DDRCCR_DDRCS_MASK		((0x1UL) << SCG_DDRCCR_DDRCS_SHIFT)

#define SCG_DDRCCR_DDRDIV_SHIFT		(0)
#define SCG_DDRCCR_DDRDIV_MASK		((0x7UL) << SCG_DDRCCR_DDRDIV_SHIFT)

/* SCG NIC Clock Control Register */
#define SCG_NICCCR_NICCS_SHIFT		(28)
#define SCG_NICCCR_NICCS_MASK		((0x1UL) << SCG_NICCCR_NICCS_SHIFT)

#define SCG_NICCCR_NIC0_DIV_SHIFT       (24)
#define SCG_NICCCR_NIC0_DIV_MASK        ((0xFUL) << SCG_NICCCR_NIC0_DIV_SHIFT)

#define SCG_NICCCR_GPU_DIV_SHIFT        (20)
#define SCG_NICCCR_GPU_DIV_MASK         ((0xFUL) << SCG_NICCCR_GPU_DIV_SHIFT)

#define SCG_NICCCR_NIC1_DIV_SHIFT       (16)
#define SCG_NICCCR_NIC1_DIV_MASK        ((0xFUL) << SCG_NICCCR_NIC1_DIV_SHIFT)

#define SCG_NICCCR_NIC1_DIVEXT_SHIFT    (8)
#define SCG_NICCCR_NIC1_DIVEXT_MASK   ((0xFUL) << SCG_NICCCR_NIC1_DIVEXT_SHIFT)

#define SCG_NICCCR_NIC1_DIVBUS_SHIFT    (4)
#define SCG_NICCCR_NIC1_DIVBUS_MASK   ((0xFUL) << SCG_NICCCR_NIC1_DIVBUS_SHIFT)

/* SCG NIC clock status register */
#define SCG_NICCSR_NICCS_SHIFT          (28)
#define SCG_NICCSR_NICCS_MASK           ((0x1UL) << SCG_NICCSR_NICCS_SHIFT)

#define SCG_NICCSR_NIC0DIV_SHIFT        (24)
#define SCG_NICCSR_NIC0DIV_MASK         ((0xFUL) << SCG_NICCSR_NIC0DIV_SHIFT)
#define SCG_NICCSR_GPUDIV_SHIFT         (20)
#define SCG_NICCSR_GPUDIV_MASK          ((0xFUL) << SCG_NICCSR_GPUDIV_SHIFT)
#define SCG_NICCSR_NIC1DIV_SHIFT        (16)
#define SCG_NICCSR_NIC1DIV_MASK         ((0xFUL) << SCG_NICCSR_NIC1DIV_SHIFT)
#define SCG_NICCSR_NIC1EXTDIV_SHIFT     (8)
#define SCG_NICCSR_NIC1EXTDIV_MASK      ((0xFUL) << SCG_NICCSR_NIC1EXTDIV_SHIFT)
#define SCG_NICCSR_NIC1BUSDIV_SHIFT     (4)
#define SCG_NICCSR_NIC1BUSDIV_MASK      ((0xFUL) << SCG_NICCSR_NIC1BUSDIV_SHIFT)

/* SCG Slow IRC Control Status Register */
#define SCG_SIRC_CSR_SIRCVLD_SHIFT      (24)
#define SCG_SIRC_CSR_SIRCVLD_MASK       ((0x1UL) << SCG_SIRC_CSR_SIRCVLD_SHIFT)

#define SCG_SIRC_CSR_SIRCEN_SHIFT       (0)
#define SCG_SIRC_CSR_SIRCEN_MASK        ((0x1UL) << SCG_SIRC_CSR_SIRCEN_SHIFT)

/* SCG Slow IRC Configuration Register */
#define SCG_SIRCCFG_RANGE_SHIFT         (0)
#define SCG_SIRCCFG_RANGE_MASK          ((0x1UL) << SCG_SIRCCFG_RANGE_SHIFT)
#define SCG_SIRCCFG_RANGE_4M            ((0x0UL) << SCG_SIRCCFG_RANGE_SHIFT)
#define SCG_SIRCCFG_RANGE_16M           ((0x1UL) << SCG_SIRCCFG_RANGE_SHIFT)

/* SCG Slow IRC Divide Register */
#define SCG_SIRCDIV_DIV3_SHIFT          (16)
#define SCG_SIRCDIV_DIV3_MASK           ((0x7UL) << SCG_SIRCDIV_DIV3_SHIFT)

#define SCG_SIRCDIV_DIV2_SHIFT          (8)
#define SCG_SIRCDIV_DIV2_MASK           ((0x7UL) << SCG_SIRCDIV_DIV2_SHIFT)

#define SCG_SIRCDIV_DIV1_SHIFT          (0)
#define SCG_SIRCDIV_DIV1_MASK           ((0x7UL) << SCG_SIRCDIV_DIV1_SHIFT)
/*
 * FIRC/SIRC DIV1 ==> xIRC_PLAT_CLK
 * FIRC/SIRC DIV2 ==> xIRC_BUS_CLK
 * FIRC/SIRC DIV3 ==> xIRC_SLOW_CLK
 */

/* SCG Fast IRC Control Status Register */
#define SCG_FIRC_CSR_FIRCVLD_SHIFT      (24)
#define SCG_FIRC_CSR_FIRCVLD_MASK       ((0x1UL) << SCG_FIRC_CSR_FIRCVLD_SHIFT)

#define SCG_FIRC_CSR_FIRCEN_SHIFT       (0)
#define SCG_FIRC_CSR_FIRCEN_MASK        ((0x1UL) << SCG_FIRC_CSR_FIRCEN_SHIFT)

/* SCG Fast IRC Divide Register */
#define SCG_FIRCDIV_DIV3_SHIFT          (16)
#define SCG_FIRCDIV_DIV3_MASK           ((0x7UL) << SCG_FIRCDIV_DIV3_SHIFT)

#define SCG_FIRCDIV_DIV2_SHIFT          (8)
#define SCG_FIRCDIV_DIV2_MASK           ((0x7UL) << SCG_FIRCDIV_DIV2_SHIFT)

#define SCG_FIRCDIV_DIV1_SHIFT          (0)
#define SCG_FIRCDIV_DIV1_MASK           ((0x7UL) << SCG_FIRCDIV_DIV1_SHIFT)

#define SCG_FIRCCFG_RANGE_SHIFT         (0)
#define SCG_FIRCCFG_RANGE_MASK          ((0x3UL) << SCG_FIRCCFG_RANGE_SHIFT)

#define SCG_FIRCCFG_RANGE_SHIFT         (0)
#define SCG_FIRCCFG_RANGE_48M           ((0x0UL) << SCG_FIRCCFG_RANGE_SHIFT)

/* SCG System OSC Control Status Register */
#define SCG_SOSC_CSR_SOSCVLD_SHIFT      (24)
#define SCG_SOSC_CSR_SOSCVLD_MASK       ((0x1UL) << SCG_SOSC_CSR_SOSCVLD_SHIFT)

/* SCG Fast IRC Divide Register */
#define SCG_SOSCDIV_DIV3_SHIFT          (16)
#define SCG_SOSCDIV_DIV3_MASK           ((0x7UL) << SCG_SOSCDIV_DIV3_SHIFT)

#define SCG_SOSCDIV_DIV2_SHIFT          (8)
#define SCG_SOSCDIV_DIV2_MASK           ((0x7UL) << SCG_SOSCDIV_DIV2_SHIFT)

#define SCG_SOSCDIV_DIV1_SHIFT          (0)
#define SCG_SOSCDIV_DIV1_MASK           ((0x7UL) << SCG_SOSCDIV_DIV1_SHIFT)

/* SCG RTC OSC Control Status Register */
#define SCG_ROSC_CSR_ROSCVLD_SHIFT      (24)
#define SCG_ROSC_CSR_ROSCVLD_MASK       ((0x1UL) << SCG_ROSC_CSR_ROSCVLD_SHIFT)

#define SCG_SPLL_CSR_SPLLVLD_SHIFT      (24)
#define SCG_SPLL_CSR_SPLLVLD_MASK       ((0x1UL) << SCG_SPLL_CSR_SPLLVLD_SHIFT)
#define SCG_SPLL_CSR_SPLLEN_SHIFT       (0)
#define SCG_SPLL_CSR_SPLLEN_MASK        ((0x1UL) << SCG_SPLL_CSR_SPLLEN_SHIFT)
#define SCG_APLL_CSR_APLLEN_SHIFT       (0)
#define SCG_APLL_CSR_APLLEN_MASK        (0x1UL)
#define SCG_APLL_CSR_APLLVLD_MASK       (0x01000000)

#define SCG_UPLL_CSR_UPLLVLD_MASK       (0x01000000)


#define SCG_PLL_PFD3_GATE_MASK          (0x80000000)
#define SCG_PLL_PFD2_GATE_MASK          (0x00800000)
#define SCG_PLL_PFD1_GATE_MASK          (0x00008000)
#define SCG_PLL_PFD0_GATE_MASK          (0x00000080)
#define SCG_PLL_PFD3_VALID_MASK         (0x40000000)
#define SCG_PLL_PFD2_VALID_MASK         (0x00400000)
#define SCG_PLL_PFD1_VALID_MASK         (0x00004000)
#define SCG_PLL_PFD0_VALID_MASK         (0x00000040)

#define SCG_PLL_PFD0_FRAC_SHIFT         (0)
#define SCG_PLL_PFD0_FRAC_MASK          ((0x3F) << SCG_PLL_PFD0_FRAC_SHIFT)
#define SCG_PLL_PFD1_FRAC_SHIFT         (8)
#define SCG_PLL_PFD1_FRAC_MASK          ((0x3F) << SCG_PLL_PFD1_FRAC_SHIFT)
#define SCG_PLL_PFD2_FRAC_SHIFT         (16)
#define SCG_PLL_PFD2_FRAC_MASK          ((0x3F) << SCG_PLL_PFD2_FRAC_SHIFT)
#define SCG_PLL_PFD3_FRAC_SHIFT         (24)
#define SCG_PLL_PFD3_FRAC_MASK          ((0x3F) << SCG_PLL_PFD3_FRAC_SHIFT)

#define SCG_PLL_CFG_POSTDIV2_SHIFT      (28)
#define SCG_PLL_CFG_POSTDIV2_MASK       ((0xFUL) << SCG_PLL_CFG_POSTDIV2_SHIFT)
#define SCG_PLL_CFG_POSTDIV1_SHIFT      (24)
#define SCG_PLL_CFG_POSTDIV1_MASK       ((0xFUL) << SCG_PLL_CFG_POSTDIV1_SHIFT)
#define SCG_PLL_CFG_MULT_SHIFT          (16)
#define SCG1_SPLL_CFG_MULT_MASK         ((0x7FUL) << SCG_PLL_CFG_MULT_SHIFT)
#define SCG_APLL_CFG_MULT_MASK          ((0x7FUL) << SCG_PLL_CFG_MULT_SHIFT)
#define SCG_PLL_CFG_PFDSEL_SHIFT        (14)
#define SCG_PLL_CFG_PFDSEL_MASK         ((0x3UL) << SCG_PLL_CFG_PFDSEL_SHIFT)
#define SCG_PLL_CFG_PREDIV_SHIFT        (8)
#define SCG_PLL_CFG_PREDIV_MASK         ((0x7UL) << SCG_PLL_CFG_PREDIV_SHIFT)
#define SCG_PLL_CFG_BYPASS_SHIFT        (2)
/* 0: SPLL, 1: bypass */
#define SCG_PLL_CFG_BYPASS_MASK         ((0x1UL) << SCG_PLL_CFG_BYPASS_SHIFT)
#define SCG_PLL_CFG_PLLSEL_SHIFT        (1)
/* 0: pll, 1: pfd */
#define SCG_PLL_CFG_PLLSEL_MASK         ((0x1UL) << SCG_PLL_CFG_PLLSEL_SHIFT)
#define SCG_PLL_CFG_CLKSRC_SHIFT        (0)
/* 0: Sys-OSC, 1: FIRC */
#define SCG_PLL_CFG_CLKSRC_MASK         ((0x1UL) << SCG_PLL_CFG_CLKSRC_SHIFT)
#define SCG0_SPLL_CFG_MULT_SHIFT        (17)
/* 0: Multiplier = 20, 1: Multiplier = 22 */
#define SCG0_SPLL_CFG_MULT_MASK         ((0x1UL) << SCG0_SPLL_CFG_MULT_SHIFT)

#define PLL_USB_EN_USB_CLKS_MASK	(0x01 << 6)
#define PLL_USB_PWR_MASK		(0x01 << 12)
#define PLL_USB_ENABLE_MASK		(0x01 << 13)
#define PLL_USB_BYPASS_MASK		(0x01 << 16)
#define PLL_USB_REG_ENABLE_MASK		(0x01 << 21)
#define PLL_USB_DIV_SEL_MASK		(0x07 << 22)
#define PLL_USB_LOCK_MASK		(0x01 << 31)

enum scg_clk {
	SCG_SOSC_CLK,
	SCG_FIRC_CLK,
	SCG_SIRC_CLK,
	SCG_ROSC_CLK,
	SCG_SIRC_DIV1_CLK,
	SCG_SIRC_DIV2_CLK,
	SCG_SIRC_DIV3_CLK,
	SCG_FIRC_DIV1_CLK,
	SCG_FIRC_DIV2_CLK,
	SCG_FIRC_DIV3_CLK,
	SCG_SOSC_DIV1_CLK,
	SCG_SOSC_DIV2_CLK,
	SCG_SOSC_DIV3_CLK,
	SCG_CORE_CLK,
	SCG_BUS_CLK,
	SCG_SPLL_PFD0_CLK,
	SCG_SPLL_PFD1_CLK,
	SCG_SPLL_PFD2_CLK,
	SCG_SPLL_PFD3_CLK,
	SCG_DDR_CLK,
	SCG_NIC0_CLK,
	SCG_GPU_CLK,
	SCG_NIC1_CLK,
	SCG_NIC1_BUS_CLK,
	SCG_NIC1_EXT_CLK,
	SCG_APLL_PFD0_CLK,
	SCG_APLL_PFD1_CLK,
	SCG_APLL_PFD2_CLK,
	SCG_APLL_PFD3_CLK,
	USB_PLL_OUT,
	MIPI_PLL_OUT
};

enum scg_sys_src {
	SCG_SCS_SYS_OSC = 1,
	SCG_SCS_SLOW_IRC,
	SCG_SCS_FAST_IRC,
	SCG_SCS_RTC_OSC,
	SCG_SCS_AUX_PLL,
	SCG_SCS_SYS_PLL,
	SCG_SCS_USBPHY_PLL,
};

/* PLL supported by i.mx7ulp */
enum pll_clocks {
	PLL_M4_SPLL,	/* M4 SPLL */
	PLL_M4_APLL,	/* M4 APLL*/
	PLL_A7_SPLL,	/* A7 SPLL */
	PLL_A7_APLL,	/* A7 APLL */
	PLL_USB,	/* USB PLL*/
	PLL_MIPI,	/* MIPI PLL */
};

typedef struct scg_regs {
	u32 verid;	/* VERSION_ID */
	u32 param;	/*  PARAMETER */
	u32 rsvd11[2];

	u32 csr;	/*  Clock Status Register */
	u32 rccr;	/*  Run Clock Control Register */
	u32 vccr;	/*  VLPR Clock Control Register */
	u32 hccr;	/*  HSRUN Clock Control Register */
	u32 clkoutcnfg;	/*  SCG CLKOUT Configuration Register */
	u32 rsvd12[3];
	u32 ddrccr;	/*  SCG DDR Clock Control Register */
	u32 rsvd13[3];
	u32 nicccr;	/*  NIC Clock Control Register */
	u32 niccsr;	/*  NIC Clock Status Register */
	u32 rsvd10[46];

	u32 sosccsr;	/*  System OSC Control Status Register, offset 0x100 */
	u32 soscdiv;	/*  System OSC Divide Register */
	u32 sosccfg;	/*  System Oscillator Configuration Register */
	u32 sosctest;	/*  System Oscillator Test Register */
	u32 rsvd20[60];

	u32 sirccsr;	/*  Slow IRC Control Status Register, offset 0x200 */
	u32 sircdiv;	/*  Slow IRC Divide Register */
	u32 sirccfg;	/*  Slow IRC Configuration Register */
	u32 sirctrim;	/*  Slow IRC Trim Register */
	u32 loptrim;	/*  Low Power Oscillator Trim Register */
	u32 sirctest;	/*  Slow IRC Test Register */
	u32 rsvd30[58];

	u32 firccsr;	/*  Fast IRC Control Status Register, offset 0x300 */
	u32 fircdiv;
	u32 firccfg;
	u32 firctcfg;	/*  Fast IRC Trim Configuration Register */
	u32 firctriml;	/*  Fast IRC Trim Low Register */
	u32 firctrimh;
	u32 fircstat;	/*  Fast IRC Status Register */
	u32 firctest;	/*  Fast IRC Test Register */
	u32 rsvd40[56];

	u32 rtccsr;	/*  RTC OSC Control Status Register, offset 0x400 */
	u32 rsvd50[63];

	u32 apllcsr; /*  Auxiliary PLL Control Status Register, offset 0x500 */
	u32 aplldiv;	/*  Auxiliary PLL Divider Register */
	u32 apllcfg;	/*  Auxiliary PLL Configuration Register */
	u32 apllpfd;	/*  Auxiliary PLL PFD Register */
	u32 apllnum;	/*  Auxiliary PLL Numerator Register */
	u32 aplldenom;	/*  Auxiliary PLL Denominator Register */
	u32 apllss;	/*  Auxiliary PLL Spread Spectrum Register */
	u32 rsvd60[55];
	u32 apllock_cnfg; /*  Auxiliary PLL LOCK Configuration Register */
	u32 rsvd61[1];

	u32 spllcsr;	/*  System PLL Control Status Register, offset 0x600 */
	u32 splldiv;	/*  System PLL Divide Register */
	u32 spllcfg;	/*  System PLL Configuration Register */
	u32 spllpfd;	/*  System PLL Test Register */
	u32 spllnum;	/*  System PLL Numerator Register */
	u32 splldenom;	/*  System PLL Denominator Register */
	u32 spllss;	/*  System PLL Spread Spectrum Register */
	u32 rsvd70[55];
	u32 spllock_cnfg;	/*  System PLL LOCK Configuration Register */
	u32 rsvd71[1];

	u32 upllcsr;	/*  USB PLL Control Status Register, offset 0x700 */
	u32 uplldiv;	/*  USB PLL Divide Register */
	u32 upllcfg;	/*  USB PLL Configuration Register */
} scg_t, *scg_p;

u32 scg_clk_get_rate(enum scg_clk clk);
int scg_enable_pll_pfd(enum scg_clk clk, u32 frac);
int scg_enable_usb_pll(bool usb_control);
u32 decode_pll(enum pll_clocks pll);

void scg_a7_rccr_init(void);
void scg_a7_spll_init(void);
void scg_a7_ddrclk_init(void);
void scg_a7_apll_init(void);
void scg_a7_firc_init(void);
void scg_a7_nicclk_init(void);
void scg_a7_sys_clk_sel(enum scg_sys_src clk);
void scg_a7_info(void);
void scg_a7_soscdiv_init(void);

#endif
