/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2016-2019 Intel Corporation <www.intel.com>
 *
 */

#ifndef	_CLOCK_MANAGER_S10_
#define	_CLOCK_MANAGER_S10_

#include <asm/arch/clock_manager_soc64.h>
#include <linux/bitops.h>

/* Clock speed accessors */
unsigned long cm_get_sdram_clk_hz(void);
unsigned int cm_get_l4_sp_clk_hz(void);
unsigned int cm_get_mmc_controller_clk_hz(void);
unsigned int cm_get_spi_controller_clk_hz(void);

struct cm_config {
	/* main group */
	u32 main_pll_mpuclk;
	u32 main_pll_nocclk;
	u32 main_pll_cntr2clk;
	u32 main_pll_cntr3clk;
	u32 main_pll_cntr4clk;
	u32 main_pll_cntr5clk;
	u32 main_pll_cntr6clk;
	u32 main_pll_cntr7clk;
	u32 main_pll_cntr8clk;
	u32 main_pll_cntr9clk;
	u32 main_pll_nocdiv;
	u32 main_pll_pllglob;
	u32 main_pll_fdbck;
	u32 main_pll_pllc0;
	u32 main_pll_pllc1;
	u32 spare;

	/* peripheral group */
	u32 per_pll_cntr2clk;
	u32 per_pll_cntr3clk;
	u32 per_pll_cntr4clk;
	u32 per_pll_cntr5clk;
	u32 per_pll_cntr6clk;
	u32 per_pll_cntr7clk;
	u32 per_pll_cntr8clk;
	u32 per_pll_cntr9clk;
	u32 per_pll_emacctl;
	u32 per_pll_gpiodiv;
	u32 per_pll_pllglob;
	u32 per_pll_fdbck;
	u32 per_pll_pllc0;
	u32 per_pll_pllc1;

	/* incoming clock */
	u32 hps_osc_clk_hz;
	u32 fpga_clk_hz;
};

void cm_basic_init(const struct cm_config * const cfg);

/* Control status */
#define CLKMGR_S10_CTRL					0x00
#define CLKMGR_S10_STAT					0x04
#define CLKMGR_S10_INTRCLR				0x14
/* Mainpll group */
#define CLKMGR_S10_MAINPLL_EN				0x30
#define CLKMGR_S10_MAINPLL_BYPASS			0x3c
#define CLKMGR_S10_MAINPLL_MPUCLK			0x48
#define CLKMGR_S10_MAINPLL_NOCCLK			0x4c
#define CLKMGR_S10_MAINPLL_CNTR2CLK			0x50
#define CLKMGR_S10_MAINPLL_CNTR3CLK			0x54
#define CLKMGR_S10_MAINPLL_CNTR4CLK			0x58
#define CLKMGR_S10_MAINPLL_CNTR5CLK			0x5c
#define CLKMGR_S10_MAINPLL_CNTR6CLK			0x60
#define CLKMGR_S10_MAINPLL_CNTR7CLK			0x64
#define CLKMGR_S10_MAINPLL_CNTR8CLK			0x68
#define CLKMGR_S10_MAINPLL_CNTR9CLK			0x6c
#define CLKMGR_S10_MAINPLL_NOCDIV			0x70
#define CLKMGR_S10_MAINPLL_PLLGLOB			0x74
#define CLKMGR_S10_MAINPLL_FDBCK			0x78
#define CLKMGR_S10_MAINPLL_MEMSTAT			0x80
#define CLKMGR_S10_MAINPLL_PLLC0			0x84
#define CLKMGR_S10_MAINPLL_PLLC1			0x88
#define CLKMGR_S10_MAINPLL_VCOCALIB			0x8c
/* Periphpll group */
#define CLKMGR_S10_PERPLL_EN				0xa4
#define CLKMGR_S10_PERPLL_BYPASS			0xb0
#define CLKMGR_S10_PERPLL_CNTR2CLK			0xbc
#define CLKMGR_S10_PERPLL_CNTR3CLK			0xc0
#define CLKMGR_S10_PERPLL_CNTR4CLK			0xc4
#define CLKMGR_S10_PERPLL_CNTR5CLK			0xc8
#define CLKMGR_S10_PERPLL_CNTR6CLK			0xcc
#define CLKMGR_S10_PERPLL_CNTR7CLK			0xd0
#define CLKMGR_S10_PERPLL_CNTR8CLK			0xd4
#define CLKMGR_S10_PERPLL_CNTR9CLK			0xd8
#define CLKMGR_S10_PERPLL_EMACCTL			0xdc
#define CLKMGR_S10_PERPLL_GPIODIV			0xe0
#define CLKMGR_S10_PERPLL_PLLGLOB			0xe4
#define CLKMGR_S10_PERPLL_FDBCK				0xe8
#define CLKMGR_S10_PERPLL_MEMSTAT			0xf0
#define CLKMGR_S10_PERPLL_PLLC0				0xf4
#define CLKMGR_S10_PERPLL_PLLC1				0xf8
#define CLKMGR_S10_PERPLL_VCOCALIB			0xfc

#define CLKMGR_STAT					CLKMGR_S10_STAT
#define CLKMGR_INTER					CLKMGR_S10_INTER
#define CLKMGR_PERPLL_EN				CLKMGR_S10_PERPLL_EN


#define CLKMGR_CTRL_SAFEMODE				BIT(0)
#define CLKMGR_BYPASS_MAINPLL_ALL			0x00000007
#define CLKMGR_BYPASS_PERPLL_ALL			0x0000007f

#define CLKMGR_INTER_MAINPLLLOCKED_MASK			0x00000001
#define CLKMGR_INTER_PERPLLLOCKED_MASK			0x00000002
#define CLKMGR_INTER_MAINPLLLOST_MASK			0x00000004
#define CLKMGR_INTER_PERPLLLOST_MASK			0x00000008
#define CLKMGR_STAT_BUSY				BIT(0)
#define CLKMGR_STAT_MAINPLL_LOCKED			BIT(8)
#define CLKMGR_STAT_PERPLL_LOCKED			BIT(9)

#define CLKMGR_PLLGLOB_PD_MASK				0x00000001
#define CLKMGR_PLLGLOB_RST_MASK				0x00000002
#define CLKMGR_PLLGLOB_VCO_PSRC_MASK			0X3
#define CLKMGR_PLLGLOB_VCO_PSRC_OFFSET			16
#define CLKMGR_VCO_PSRC_EOSC1				0
#define CLKMGR_VCO_PSRC_INTOSC				1
#define CLKMGR_VCO_PSRC_F2S				2
#define CLKMGR_PLLGLOB_REFCLKDIV_MASK			0X3f
#define CLKMGR_PLLGLOB_REFCLKDIV_OFFSET			8

#define CLKMGR_CLKSRC_MASK				0x7
#define CLKMGR_CLKSRC_OFFSET				16
#define CLKMGR_CLKSRC_MAIN				0
#define CLKMGR_CLKSRC_PER				1
#define CLKMGR_CLKSRC_OSC1				2
#define CLKMGR_CLKSRC_INTOSC				3
#define CLKMGR_CLKSRC_FPGA				4
#define CLKMGR_CLKCNT_MSK				0x7ff

#define CLKMGR_FDBCK_MDIV_MASK				0xff
#define CLKMGR_FDBCK_MDIV_OFFSET			24

#define CLKMGR_PLLC0_DIV_MASK				0xff
#define CLKMGR_PLLC1_DIV_MASK				0xff
#define CLKMGR_PLLC0_EN_OFFSET				27
#define CLKMGR_PLLC1_EN_OFFSET				24

#define CLKMGR_NOCDIV_L4MAIN_OFFSET			0
#define CLKMGR_NOCDIV_L4MPCLK_OFFSET			8
#define CLKMGR_NOCDIV_L4SPCLK_OFFSET			16
#define CLKMGR_NOCDIV_CSATCLK_OFFSET			24
#define CLKMGR_NOCDIV_CSTRACECLK_OFFSET			26
#define CLKMGR_NOCDIV_CSPDBGCLK_OFFSET			28

#define CLKMGR_NOCDIV_L4SPCLK_MASK			0X3
#define CLKMGR_NOCDIV_DIV1				0
#define CLKMGR_NOCDIV_DIV2				1
#define CLKMGR_NOCDIV_DIV4				2
#define CLKMGR_NOCDIV_DIV8				3
#define CLKMGR_CSPDBGCLK_DIV1				0
#define CLKMGR_CSPDBGCLK_DIV4				1

#define CLKMGR_MSCNT_CONST				200
#define CLKMGR_MDIV_CONST				6
#define CLKMGR_HSCNT_CONST				9

#define CLKMGR_VCOCALIB_MSCNT_MASK			0xff
#define CLKMGR_VCOCALIB_MSCNT_OFFSET			9
#define CLKMGR_VCOCALIB_HSCNT_MASK			0xff

#define CLKMGR_EMACCTL_EMAC0SEL_OFFSET			26
#define CLKMGR_EMACCTL_EMAC1SEL_OFFSET			27
#define CLKMGR_EMACCTL_EMAC2SEL_OFFSET			28

#define CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK		0x00000020

#endif /* _CLOCK_MANAGER_S10_ */
