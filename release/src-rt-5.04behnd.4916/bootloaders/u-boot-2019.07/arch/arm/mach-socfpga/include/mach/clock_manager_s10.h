/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#ifndef	_CLOCK_MANAGER_S10_
#define	_CLOCK_MANAGER_S10_

/* Clock speed accessors */
unsigned long cm_get_mpu_clk_hz(void);
unsigned long cm_get_sdram_clk_hz(void);
unsigned int cm_get_l4_sp_clk_hz(void);
unsigned int cm_get_mmc_controller_clk_hz(void);
unsigned int cm_get_qspi_controller_clk_hz(void);
unsigned int cm_get_spi_controller_clk_hz(void);
const unsigned int cm_get_osc_clk_hz(void);
const unsigned int cm_get_f2s_per_ref_clk_hz(void);
const unsigned int cm_get_f2s_sdr_ref_clk_hz(void);
const unsigned int cm_get_intosc_clk_hz(void);
const unsigned int cm_get_fpga_clk_hz(void);

#define CLKMGR_EOSC1_HZ		25000000
#define CLKMGR_INTOSC_HZ	460000000
#define CLKMGR_FPGA_CLK_HZ	50000000

/* Clock configuration accessors */
const struct cm_config * const cm_get_default_config(void);

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

struct socfpga_clock_manager_main_pll {
	u32	en;
	u32	ens;
	u32	enr;
	u32	bypass;
	u32	bypasss;
	u32	bypassr;
	u32	mpuclk;
	u32	nocclk;
	u32	cntr2clk;
	u32	cntr3clk;
	u32	cntr4clk;
	u32	cntr5clk;
	u32	cntr6clk;
	u32	cntr7clk;
	u32	cntr8clk;
	u32	cntr9clk;
	u32	nocdiv;
	u32	pllglob;
	u32	fdbck;
	u32	mem;
	u32	memstat;
	u32	pllc0;
	u32	pllc1;
	u32	vcocalib;
	u32	_pad_0x90_0xA0[5];
};

struct socfpga_clock_manager_per_pll {
	u32	en;
	u32	ens;
	u32	enr;
	u32	bypass;
	u32	bypasss;
	u32	bypassr;
	u32	cntr2clk;
	u32	cntr3clk;
	u32	cntr4clk;
	u32	cntr5clk;
	u32	cntr6clk;
	u32	cntr7clk;
	u32	cntr8clk;
	u32	cntr9clk;
	u32	emacctl;
	u32	gpiodiv;
	u32	pllglob;
	u32	fdbck;
	u32	mem;
	u32	memstat;
	u32	pllc0;
	u32	pllc1;
	u32	vcocalib;
	u32	_pad_0x100_0x124[10];
};

struct socfpga_clock_manager {
	u32	ctrl;
	u32	stat;
	u32	testioctrl;
	u32	intrgen;
	u32	intrmsk;
	u32	intrclr;
	u32	intrsts;
	u32	intrstk;
	u32	intrraw;
	u32	_pad_0x24_0x2c[3];
	struct socfpga_clock_manager_main_pll main_pll;
	struct socfpga_clock_manager_per_pll per_pll;
};

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
