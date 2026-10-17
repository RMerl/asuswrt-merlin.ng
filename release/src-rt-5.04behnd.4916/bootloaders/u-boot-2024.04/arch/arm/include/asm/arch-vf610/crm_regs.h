/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2014 Freescale Semiconductor, Inc.
 */

#ifndef __ARCH_ARM_MACH_VF610_CCM_REGS_H__
#define __ARCH_ARM_MACH_VF610_CCM_REGS_H__

#ifndef __ASSEMBLY__

#include <linux/types.h>

/* Clock Controller Module (CCM) */
struct ccm_reg {
	u32 ccr;
	u32 csr;
	u32 ccsr;
	u32 cacrr;
	u32 cscmr1;
	u32 cscdr1;
	u32 cscdr2;
	u32 cscdr3;
	u32 cscmr2;
	u32 cscdr4;
	u32 ctor;
	u32 clpcr;
	u32 cisr;
	u32 cimr;
	u32 ccosr;
	u32 cgpr;
	u32 ccgr0;
	u32 ccgr1;
	u32 ccgr2;
	u32 ccgr3;
	u32 ccgr4;
	u32 ccgr5;
	u32 ccgr6;
	u32 ccgr7;
	u32 ccgr8;
	u32 ccgr9;
	u32 ccgr10;
	u32 ccgr11;
	u32 cmeor0;
	u32 cmeor1;
	u32 cmeor2;
	u32 cmeor3;
	u32 cmeor4;
	u32 cmeor5;
	u32 cppdsr;
	u32 ccowr;
	u32 ccpgr0;
	u32 ccpgr1;
	u32 ccpgr2;
	u32 ccpgr3;
};

/* Analog components control digital interface (ANADIG) */
struct anadig_reg {
	u32 reserved_0x000[4];
	u32 pll3_ctrl;
	u32 reserved_0x014[3];
	u32 pll7_ctrl;
	u32 reserved_0x024[3];
	u32 pll2_ctrl;
	u32 reserved_0x034[3];
	u32 pll2_ss;
	u32 reserved_0x044[3];
	u32 pll2_num;
	u32 reserved_0x054[3];
	u32 pll2_denom;
	u32 reserved_0x064[3];
	u32 pll4_ctrl;
	u32 reserved_0x074[3];
	u32 pll4_num;
	u32 reserved_0x084[3];
	u32 pll4_denom;
	u32 reserved_0x094[3];
	u32 pll6_ctrl;
	u32 reserved_0x0A4[3];
	u32 pll6_num;
	u32 reserved_0x0B4[3];
	u32 pll6_denom;
	u32 reserved_0x0C4[7];
	u32 pll5_ctrl;
	u32 reserved_0x0E4[3];
	u32 pll3_pfd;
	u32 reserved_0x0F4[3];
	u32 pll2_pfd;
	u32 reserved_0x104[3];
	u32 reg_1p1;
	u32 reserved_0x114[3];
	u32 reg_3p0;
	u32 reserved_0x124[3];
	u32 reg_2p5;
	u32 reserved_0x134[7];
	u32 ana_misc0;
	u32 reserved_0x154[3];
	u32 ana_misc1;
	u32 reserved_0x164[63];
	u32 anadig_digprog;
	u32 reserved_0x264[3];
	u32 pll1_ctrl;
	u32 reserved_0x274[3];
	u32 pll1_ss;
	u32 reserved_0x284[3];
	u32 pll1_num;
	u32 reserved_0x294[3];
	u32 pll1_denom;
	u32 reserved_0x2A4[3];
	u32 pll1_pdf;
	u32 reserved_0x2B4[3];
	u32 pll_lock;
};
#endif

#define CCM_CCR_FIRC_EN				(1 << 16)
#define CCM_CCR_OSCNT_MASK			0xff
#define CCM_CCR_OSCNT(v)			((v) & 0xff)

#define CCM_CCSR_PLL2_PFD_CLK_SEL_OFFSET	19
#define CCM_CCSR_PLL2_PFD_CLK_SEL_MASK		(0x7 << 19)
#define CCM_CCSR_PLL2_PFD_CLK_SEL(v)		(((v) & 0x7) << 19)

#define CCM_CCSR_PLL1_PFD_CLK_SEL_OFFSET	16
#define CCM_CCSR_PLL1_PFD_CLK_SEL_MASK		(0x7 << 16)
#define CCM_CCSR_PLL1_PFD_CLK_SEL(v)		(((v) & 0x7) << 16)

#define CCM_CCSR_PLL2_PFD4_EN			(1 << 15)
#define CCM_CCSR_PLL2_PFD3_EN			(1 << 14)
#define CCM_CCSR_PLL2_PFD2_EN			(1 << 13)
#define CCM_CCSR_PLL2_PFD1_EN			(1 << 12)
#define CCM_CCSR_PLL1_PFD4_EN			(1 << 11)
#define CCM_CCSR_PLL1_PFD3_EN			(1 << 10)
#define CCM_CCSR_PLL1_PFD2_EN			(1 << 9)
#define CCM_CCSR_PLL1_PFD1_EN			(1 << 8)

#define CCM_CCSR_DDRC_CLK_SEL(v)		((v) << 6)
#define CCM_CCSR_FAST_CLK_SEL(v)		((v) << 5)

#define CCM_CCSR_SYS_CLK_SEL_OFFSET		0
#define CCM_CCSR_SYS_CLK_SEL_MASK		0x7
#define CCM_CCSR_SYS_CLK_SEL(v)			((v) & 0x7)

#define CCM_CACRR_IPG_CLK_DIV_OFFSET		11
#define CCM_CACRR_IPG_CLK_DIV_MASK		(0x3 << 11)
#define CCM_CACRR_IPG_CLK_DIV(v)		(((v) & 0x3) << 11)
#define CCM_CACRR_BUS_CLK_DIV_OFFSET		3
#define CCM_CACRR_BUS_CLK_DIV_MASK		(0x7 << 3)
#define CCM_CACRR_BUS_CLK_DIV(v)		(((v) & 0x7) << 3)
#define CCM_CACRR_ARM_CLK_DIV_OFFSET		0
#define CCM_CACRR_ARM_CLK_DIV_MASK		0x7
#define CCM_CACRR_ARM_CLK_DIV(v)		((v) & 0x7)

#define CCM_CSCMR1_DCU1_CLK_SEL			(1 << 29)
#define CCM_CSCMR1_DCU0_CLK_SEL			(1 << 28)

#define CCM_CSCMR1_QSPI0_CLK_SEL_OFFSET		22
#define CCM_CSCMR1_QSPI0_CLK_SEL_MASK		(0x3 << 22)
#define CCM_CSCMR1_QSPI0_CLK_SEL(v)		(((v) & 0x3) << 22)
#define CCM_CSCMR1_ESDHC1_CLK_SEL_OFFSET	18
#define CCM_CSCMR1_ESDHC1_CLK_SEL_MASK		(0x3 << 18)
#define CCM_CSCMR1_ESDHC1_CLK_SEL(v)		(((v) & 0x3) << 18)
#define CCM_CSCMR1_NFC_CLK_SEL_OFFSET		12
#define CCM_CSCMR1_NFC_CLK_SEL_MASK		(0x3 << 12)
#define CCM_CSCMR1_NFC_CLK_SEL(v)		(((v) & 0x3) << 12)

#define CCM_CSCDR1_RMII_CLK_EN			(1 << 24)

#define CCM_CSCDR2_NFC_EN			(1 << 9)
#define CCM_CSCDR2_NFC_FRAC_DIV_EN		(1 << 13)
#define CCM_CSCDR2_NFC_CLK_INV			(1 << 14)
#define CCM_CSCDR2_NFC_FRAC_DIV_OFFSET		4
#define CCM_CSCDR2_NFC_FRAC_DIV_MASK		(0xf << 4)
#define CCM_CSCDR2_NFC_FRAC_DIV(v)		(((v) & 0xf) << 4)

#define CCM_CSCDR2_ESDHC1_EN			(1 << 29)
#define CCM_CSCDR2_ESDHC1_CLK_DIV_OFFSET	20
#define CCM_CSCDR2_ESDHC1_CLK_DIV_MASK		(0xf << 20)
#define CCM_CSCDR2_ESDHC1_CLK_DIV(v)		(((v) & 0xf) << 20)

#define CCM_CSCDR3_DCU1_EN			(1 << 23)
#define CCM_CSCDR3_DCU1_DIV_MASK		(0x7 << 20)
#define CCM_CSCDR3_DCU1_DIV(v)			(((v) & 0x7) << 20)
#define CCM_CSCDR3_DCU0_EN			(1 << 19)
#define CCM_CSCDR3_DCU0_DIV_MASK		(0x7 << 16)
#define CCM_CSCDR3_DCU0_DIV(v)			(((v) & 0x7) << 16)

#define CCM_CSCDR3_NFC_PRE_DIV_OFFSET		13
#define CCM_CSCDR3_NFC_PRE_DIV_MASK		(0x7 << 13)
#define CCM_CSCDR3_NFC_PRE_DIV(v)		(((v) & 0x7) << 13)
#define CCM_CSCDR3_QSPI0_EN			(1 << 4)
#define CCM_CSCDR3_QSPI0_DIV(v)			((v) << 3)
#define CCM_CSCDR3_QSPI0_X2_DIV(v)		((v) << 2)
#define CCM_CSCDR3_QSPI0_X4_DIV(v)		((v) & 0x3)

#define CCM_CSCMR2_RMII_CLK_SEL_OFFSET		4
#define CCM_CSCMR2_RMII_CLK_SEL_MASK		(0x3 << 4)
#define CCM_CSCMR2_RMII_CLK_SEL(v)		(((v) & 0x3) << 4)

#define CCM_REG_CTRL_MASK			0xffffffff
#define CCM_CCGR0_UART0_CTRL_MASK               (0x3 << 14)
#define CCM_CCGR0_UART1_CTRL_MASK		(0x3 << 16)
#define CCM_CCGR0_UART2_CTRL_MASK		(0x3 << 18)
#define CCM_CCGR0_DSPI0_CTRL_MASK		(0x3 << 24)
#define CCM_CCGR0_DSPI1_CTRL_MASK		(0x3 << 26)
#define CCM_CCGR1_USBC0_CTRL_MASK       (0x3 << 8)
#define CCM_CCGR1_PIT_CTRL_MASK			(0x3 << 14)
#define CCM_CCGR1_TCON0_CTRL_MASK		(0x3 << 26)
#define CCM_CCGR1_WDOGA5_CTRL_MASK		(0x3 << 28)
#define CCM_CCGR2_QSPI0_CTRL_MASK		(0x3 << 8)
#define CCM_CCGR2_IOMUXC_CTRL_MASK		(0x3 << 16)
#define CCM_CCGR2_PORTA_CTRL_MASK		(0x3 << 18)
#define CCM_CCGR2_PORTB_CTRL_MASK		(0x3 << 20)
#define CCM_CCGR2_PORTC_CTRL_MASK		(0x3 << 22)
#define CCM_CCGR2_PORTD_CTRL_MASK		(0x3 << 24)
#define CCM_CCGR2_PORTE_CTRL_MASK		(0x3 << 26)
#define CCM_CCGR3_ANADIG_CTRL_MASK		0x3
#define CCM_CCGR3_SCSC_CTRL_MASK        (0x3 << 4)
#define CCM_CCGR3_DCU0_CTRL_MASK		(0x3 << 16)
#define CCM_CCGR4_WKUP_CTRL_MASK		(0x3 << 20)
#define CCM_CCGR4_CCM_CTRL_MASK			(0x3 << 22)
#define CCM_CCGR4_GPC_CTRL_MASK			(0x3 << 24)
#define CCM_CCGR4_I2C0_CTRL_MASK		(0x3 << 12)
#define CCM_CCGR4_I2C1_CTRL_MASK		(0x3 << 14)
#define CCM_CCGR6_OCOTP_CTRL_MASK		(0x3 << 10)
#define CCM_CCGR6_DSPI2_CTRL_MASK		(0x3 << 24)
#define CCM_CCGR6_DSPI3_CTRL_MASK		(0x3 << 26)
#define CCM_CCGR6_DDRMC_CTRL_MASK		(0x3 << 28)
#define CCM_CCGR7_SDHC1_CTRL_MASK		(0x3 << 4)
#define CCM_CCGR7_USBC1_CTRL_MASK       (0x3 << 8)
#define CCM_CCGR9_FEC0_CTRL_MASK		0x3
#define CCM_CCGR9_FEC1_CTRL_MASK		(0x3 << 2)
#define CCM_CCGR10_NFC_CTRL_MASK		0x3
#define CCM_CCGR10_I2C2_CTRL_MASK		(0x3 << 12)
#define CCM_CCGR10_I2C3_CTRL_MASK		(0x3 << 14)

#define ANADIG_PLL7_CTRL_BYPASS         (1 << 16)
#define ANADIG_PLL7_CTRL_ENABLE         (1 << 13)
#define ANADIG_PLL7_CTRL_POWERDOWN      (1 << 12)
#define ANADIG_PLL7_CTRL_DIV_SELECT     (1 << 1)
#define ANADIG_PLL5_CTRL_BYPASS                 (1 << 16)
#define ANADIG_PLL5_CTRL_ENABLE                 (1 << 13)
#define ANADIG_PLL5_CTRL_POWERDOWN              (1 << 12)
#define ANADIG_PLL5_CTRL_DIV_SELECT		1
#define ANADIG_PLL3_CTRL_BYPASS         (1 << 16)
#define ANADIG_PLL3_CTRL_ENABLE         (1 << 13)
#define ANADIG_PLL3_CTRL_POWERDOWN      (1 << 12)
#define ANADIG_PLL3_CTRL_DIV_SELECT     (1 << 1)
#define ANADIG_PLL2_CTRL_ENABLE			(1 << 13)
#define ANADIG_PLL2_CTRL_POWERDOWN		(1 << 12)
#define ANADIG_PLL2_CTRL_DIV_SELECT		1
#define ANADIG_PLL1_CTRL_ENABLE			(1 << 13)
#define ANADIG_PLL1_CTRL_POWERDOWN		(1 << 12)
#define ANADIG_PLL1_CTRL_DIV_SELECT		1

#define FASE_CLK_FREQ		24000000
#define SLOW_CLK_FREQ		32000
#define PLL1_PFD1_FREQ		500000000
#define PLL1_PFD2_FREQ		452000000
#define PLL1_PFD3_FREQ		396000000
#define PLL1_PFD4_FREQ		528000000
#define PLL1_MAIN_FREQ		528000000
#define PLL2_PFD1_FREQ		500000000
#define PLL2_PFD2_FREQ		396000000
#define PLL2_PFD3_FREQ		339000000
#define PLL2_PFD4_FREQ		413000000
#define PLL2_MAIN_FREQ		528000000
#define PLL3_MAIN_FREQ		480000000
#define PLL3_PFD3_FREQ		298000000
#define PLL5_MAIN_FREQ		500000000

#define ENET_EXTERNAL_CLK	50000000
#define AUDIO_EXTERNAL_CLK	24576000

#endif /*__ARCH_ARM_MACH_VF610_CCM_REGS_H__ */
