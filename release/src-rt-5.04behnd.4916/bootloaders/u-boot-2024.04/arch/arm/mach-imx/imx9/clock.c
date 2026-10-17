// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <command.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/ccm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <div64.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <log.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

static struct anatop_reg *ana_regs = (struct anatop_reg *)ANATOP_BASE_ADDR;

static struct imx_intpll_rate_table imx9_intpll_tbl[] = {
	INT_PLL_RATE(1800000000U, 1, 150, 2), /* 1.8Ghz */
	INT_PLL_RATE(1700000000U, 1, 141, 2), /* 1.7Ghz */
	INT_PLL_RATE(1500000000U, 1, 125, 2), /* 1.5Ghz */
	INT_PLL_RATE(1400000000U, 1, 175, 3), /* 1.4Ghz */
	INT_PLL_RATE(1000000000U, 1, 166, 4), /* 1000Mhz */
	INT_PLL_RATE(900000000U, 1, 150, 4), /* 900Mhz */
};

static struct imx_fracpll_rate_table imx9_fracpll_tbl[] = {
	FRAC_PLL_RATE(1000000000U, 1, 166, 4, 2, 3), /* 1000Mhz */
	FRAC_PLL_RATE(933000000U, 1, 155, 4, 1, 2), /* 933Mhz */
	FRAC_PLL_RATE(700000000U, 1, 145, 5, 5, 6), /* 700Mhz */
	FRAC_PLL_RATE(484000000U, 1, 121, 6, 0, 1),
	FRAC_PLL_RATE(445333333U, 1, 167, 9, 0, 1),
	FRAC_PLL_RATE(466000000U, 1, 155, 8, 1, 3), /* 466Mhz */
	FRAC_PLL_RATE(400000000U, 1, 200, 12, 0, 1), /* 400Mhz */
	FRAC_PLL_RATE(300000000U, 1, 150, 12, 0, 1),
};

/* return in khz */
static u32 decode_pll_vco(struct ana_pll_reg *reg, bool fracpll)
{
	u32 ctrl;
	u32 pll_status;
	u32 div;
	int rdiv, mfi, mfn, mfd;
	int clk = 24000;

	ctrl = readl(&reg->ctrl.reg);
	pll_status = readl(&reg->pll_status);
	div = readl(&reg->div.reg);

	if (!(ctrl & PLL_CTRL_POWERUP))
		return 0;

	if (!(pll_status & PLL_STATUS_PLL_LOCK))
		return 0;

	mfi = (div & GENMASK(24, 16)) >> 16;
	rdiv = (div & GENMASK(15, 13)) >> 13;

	if (rdiv == 0)
		rdiv = 1;

	if (fracpll) {
		mfn = (int)readl(&reg->num.reg);
		mfn >>= 2;
		mfd = (int)(readl(&reg->denom.reg) & GENMASK(29, 0));

		clk = clk * (mfi * mfd + mfn) / mfd / rdiv;
	} else {
		clk = clk * mfi / rdiv;
	}

	return (u32)clk;
}

/* return in khz */
static u32 decode_pll_out(struct ana_pll_reg *reg, bool fracpll)
{
	u32 ctrl = readl(&reg->ctrl.reg);
	u32 div;

	if (ctrl & PLL_CTRL_CLKMUX_BYPASS)
		return 24000;

	if (!(ctrl & PLL_CTRL_CLKMUX_EN))
		return 0;

	div = readl(&reg->div.reg);
	div &= 0xff; /* odiv */

	if (div == 0)
		div = 2;
	else if (div == 1)
		div = 3;

	return decode_pll_vco(reg, fracpll) / div;
}

/* return in khz */
static u32 decode_pll_pfd(struct ana_pll_reg *reg, struct ana_pll_dfs *dfs_reg,
			  bool div2, bool fracpll)
{
	u32 pllvco = decode_pll_vco(reg, fracpll);
	u32 dfs_ctrl = readl(&dfs_reg->dfs_ctrl.reg);
	u32 dfs_div = readl(&dfs_reg->dfs_div.reg);
	u32 mfn, mfi;
	u32 output;

	if (dfs_ctrl & PLL_DFS_CTRL_BYPASS)
		return pllvco;

	if (!(dfs_ctrl & PLL_DFS_CTRL_ENABLE) ||
	    (div2 && !(dfs_ctrl & PLL_DFS_CTRL_CLKOUT_DIV2)) ||
	    (!div2 && !(dfs_ctrl & PLL_DFS_CTRL_CLKOUT)))
		return 0;

	mfn = dfs_div & GENMASK(2, 0);
	mfi = (dfs_div & GENMASK(15, 8)) >> 8;

	if (mfn > 3)
		return 0; /* valid mfn 0-3 */

	if (mfi == 0 || mfi == 1)
		return 0; /* valid mfi 2-255 */

	output = (pllvco * 5) / (mfi * 5 + mfn);

	if (div2)
		return output >> 1;

	return output;
}

static u32 decode_pll(enum ccm_clk_src pll)
{
	switch (pll) {
	case ARM_PLL_CLK:
		return decode_pll_out(&ana_regs->arm_pll, false);
	case SYS_PLL_PG:
		return decode_pll_out(&ana_regs->sys_pll, false);
	case SYS_PLL_PFD0:
		return decode_pll_pfd(&ana_regs->sys_pll,
			&ana_regs->sys_pll.dfs[0], false, true);
	case SYS_PLL_PFD0_DIV2:
		return decode_pll_pfd(&ana_regs->sys_pll,
			&ana_regs->sys_pll.dfs[0], true, true);
	case SYS_PLL_PFD1:
		return decode_pll_pfd(&ana_regs->sys_pll,
			&ana_regs->sys_pll.dfs[1], false, true);
	case SYS_PLL_PFD1_DIV2:
		return decode_pll_pfd(&ana_regs->sys_pll,
			&ana_regs->sys_pll.dfs[1], true, true);
	case SYS_PLL_PFD2:
		return decode_pll_pfd(&ana_regs->sys_pll,
			&ana_regs->sys_pll.dfs[2], false, true);
	case SYS_PLL_PFD2_DIV2:
		return decode_pll_pfd(&ana_regs->sys_pll,
			&ana_regs->sys_pll.dfs[2], true, true);
	case AUDIO_PLL_CLK:
		return decode_pll_out(&ana_regs->audio_pll, true);
	case DRAM_PLL_CLK:
		return decode_pll_out(&ana_regs->dram_pll, true);
	case VIDEO_PLL_CLK:
		return decode_pll_out(&ana_regs->video_pll, true);
	default:
		printf("Invalid clock source to decode\n");
		break;
	}

	return 0;
}

int configure_intpll(enum ccm_clk_src pll, u32 freq)
{
	int i;
	struct imx_intpll_rate_table *rate;
	struct ana_pll_reg *reg;
	u32 pll_status;

	for (i = 0; i < ARRAY_SIZE(imx9_intpll_tbl); i++) {
		if (freq == imx9_intpll_tbl[i].rate)
			break;
	}

	if (i == ARRAY_SIZE(imx9_intpll_tbl)) {
		debug("No matched freq table %u\n", freq);
		return -EINVAL;
	}

	rate = &imx9_intpll_tbl[i];

	/* ROM has configured SYS PLL and PFD, no need for it */
	switch (pll) {
	case ARM_PLL_CLK:
		reg = &ana_regs->arm_pll;
		break;
	default:
		return -EPERM;
	}

	/* Clear PLL HW CTRL SEL */
	setbits_le32(&reg->ctrl.reg_clr, PLL_CTRL_HW_CTRL_SEL);

	/* Bypass the PLL to ref */
	writel(PLL_CTRL_CLKMUX_BYPASS, &reg->ctrl.reg_set);

	/* disable pll and output */
	writel(PLL_CTRL_CLKMUX_EN | PLL_CTRL_POWERUP, &reg->ctrl.reg_clr);

	/* Program the ODIV, RDIV, MFI */
	writel((rate->odiv & GENMASK(7, 0)) | ((rate->rdiv << 13) & GENMASK(15, 13)) |
	       ((rate->mfi << 16) & GENMASK(24, 16)), &reg->div.reg);

	/* wait 5us */
	udelay(5);

	/* power up the PLL and wait lock (max wait time 100 us) */
	writel(PLL_CTRL_POWERUP, &reg->ctrl.reg_set);

	udelay(100);

	pll_status = readl(&reg->pll_status);
	if (pll_status & PLL_STATUS_PLL_LOCK) {
		writel(PLL_CTRL_CLKMUX_EN, &reg->ctrl.reg_set);

		/* clear bypass */
		writel(PLL_CTRL_CLKMUX_BYPASS, &reg->ctrl.reg_clr);

	} else {
		debug("Fail to lock PLL %u\n", pll);
		return -EIO;
	}

	return 0;
}

int configure_fracpll(enum ccm_clk_src pll, u32 freq)
{
	struct imx_fracpll_rate_table *rate;
	struct ana_pll_reg *reg;
	u32 pll_status;
	int i;

	for (i = 0; i < ARRAY_SIZE(imx9_fracpll_tbl); i++) {
		if (freq == imx9_fracpll_tbl[i].rate)
			break;
	}

	if (i == ARRAY_SIZE(imx9_fracpll_tbl)) {
		debug("No matched freq table %u\n", freq);
		return -EINVAL;
	}

	rate = &imx9_fracpll_tbl[i];

	switch (pll) {
	case SYS_PLL_PG:
		reg = &ana_regs->sys_pll;
		break;
	case DRAM_PLL_CLK:
		reg = &ana_regs->dram_pll;
		break;
	case VIDEO_PLL_CLK:
		reg = &ana_regs->video_pll;
		break;
	default:
		return -EPERM;
	}

	/* Bypass the PLL to ref */
	writel(PLL_CTRL_CLKMUX_BYPASS, &reg->ctrl.reg_set);

	/* disable pll and output */
	writel(PLL_CTRL_CLKMUX_EN | PLL_CTRL_POWERUP, &reg->ctrl.reg_clr);

	/* Program the ODIV, RDIV, MFI */
	writel((rate->odiv & GENMASK(7, 0)) | ((rate->rdiv << 13) & GENMASK(15, 13)) |
	       ((rate->mfi << 16) & GENMASK(24, 16)), &reg->div.reg);

	/* Set SPREAD_SPECRUM enable to 0 */
	writel(PLL_SS_EN, &reg->ss.reg_clr);

	/* Program NUMERATOR and DENOMINATOR */
	writel((rate->mfn << 2), &reg->num.reg);
	writel((rate->mfd & GENMASK(29, 0)), &reg->denom.reg);

	/* wait 5us */
	udelay(5);

	/* power up the PLL and wait lock (max wait time 100 us) */
	writel(PLL_CTRL_POWERUP, &reg->ctrl.reg_set);

	udelay(100);

	pll_status = readl(&reg->pll_status);
	if (pll_status & PLL_STATUS_PLL_LOCK) {
		writel(PLL_CTRL_CLKMUX_EN, &reg->ctrl.reg_set);

		/* check the MFN is updated */
		pll_status = readl(&reg->pll_status);
		if ((pll_status & ~0x3) != (rate->mfn << 2)) {
			debug("MFN update not matched, pll_status 0x%x, mfn 0x%x\n",
			      pll_status, rate->mfn);
			return -EIO;
		}

		/* clear bypass */
		writel(PLL_CTRL_CLKMUX_BYPASS, &reg->ctrl.reg_clr);

	} else {
		debug("Fail to lock PLL %u\n", pll);
		return -EIO;
	}

	return 0;
}

int configure_pll_pfd(enum ccm_clk_src pll_pfg, u32 mfi, u32 mfn, bool div2_en)
{
	struct ana_pll_dfs *dfs;
	struct ana_pll_reg *reg;
	u32 dfs_status;
	u32 index;

	if (mfn > 3)
		return -EINVAL; /* valid mfn 0-3 */

	if (mfi < 2 || mfi > 255)
		return -EINVAL; /* valid mfi 2-255 */

	switch (pll_pfg) {
	case SYS_PLL_PFD0:
		reg = &ana_regs->sys_pll;
		index = 0;
		break;
	case SYS_PLL_PFD1:
		reg = &ana_regs->sys_pll;
		index = 1;
		break;
	case SYS_PLL_PFD2:
		reg = &ana_regs->sys_pll;
		index = 2;
		break;
	default:
		return -EPERM;
	}

	dfs = &reg->dfs[index];

	/* Bypass the DFS to PLL VCO */
	writel(PLL_DFS_CTRL_BYPASS, &dfs->dfs_ctrl.reg_set);

	/* disable DFS and output */
	writel(PLL_DFS_CTRL_ENABLE | PLL_DFS_CTRL_CLKOUT |
		PLL_DFS_CTRL_CLKOUT_DIV2, &dfs->dfs_ctrl.reg_clr);

	writel(((mfi << 8) & GENMASK(15, 8)) | (mfn & GENMASK(2, 0)), &dfs->dfs_div.reg);

	writel(PLL_DFS_CTRL_CLKOUT, &dfs->dfs_ctrl.reg_set);
	if (div2_en)
		writel(PLL_DFS_CTRL_CLKOUT_DIV2, &dfs->dfs_ctrl.reg_set);
	writel(PLL_DFS_CTRL_ENABLE, &dfs->dfs_ctrl.reg_set);

	/*
	 * As HW expert said: after enabling the DFS, clock will start
	 * coming after 6 cycles output clock period.
	 * 5us is much bigger than expected, so it will be safe
	 */
	udelay(5);

	dfs_status = readl(&reg->dfs_status);

	if (!(dfs_status & (1 << index))) {
		debug("DFS lock failed\n");
		return -EIO;
	}

	/* Bypass the DFS to PLL VCO */
	writel(PLL_DFS_CTRL_BYPASS, &dfs->dfs_ctrl.reg_clr);

	return 0;
}

int update_fracpll_mfn(enum ccm_clk_src pll, int mfn)
{
	struct ana_pll_reg *reg;
	bool repoll = false;
	u32 pll_status;
	int count = 20;

	switch (pll) {
	case AUDIO_PLL_CLK:
		reg = &ana_regs->audio_pll;
		break;
	case DRAM_PLL_CLK:
		reg = &ana_regs->dram_pll;
		break;
	case VIDEO_PLL_CLK:
		reg = &ana_regs->video_pll;
		break;
	default:
		printf("Invalid pll %u for update FRAC PLL MFN\n", pll);
		return -EINVAL;
	}

	if (readl(&reg->pll_status) & PLL_STATUS_PLL_LOCK)
		repoll = true;

	mfn <<= 2;
	writel(mfn, &reg->num);

	if (repoll) {
		do {
			pll_status = readl(&reg->pll_status);
			udelay(5);
			count--;
		} while (((pll_status & ~0x3) != (u32)mfn) && count > 0);

		if (count <= 0) {
			printf("update MFN timeout, pll_status 0x%x, mfn 0x%x\n", pll_status, mfn);
			return -EIO;
		}
	}

	return 0;
}

int update_pll_pfd_mfn(enum ccm_clk_src pll_pfd, u32 mfn)
{
	struct ana_pll_dfs *dfs;
	u32 val;
	u32 index;

	switch (pll_pfd) {
	case SYS_PLL_PFD0:
	case SYS_PLL_PFD0_DIV2:
		index = 0;
		break;
	case SYS_PLL_PFD1:
	case SYS_PLL_PFD1_DIV2:
		index = 1;
		break;
	case SYS_PLL_PFD2:
	case SYS_PLL_PFD2_DIV2:
		index = 2;
		break;
	default:
		printf("Invalid pfd %u for update PLL PFD MFN\n", pll_pfd);
		return -EINVAL;
	}

	dfs = &ana_regs->sys_pll.dfs[index];

	val = readl(&dfs->dfs_div.reg);
	val &= ~0x3;
	val |= mfn & 0x3;
	writel(val, &dfs->dfs_div.reg);

	return 0;
}

/* return in khz */
u32 get_clk_src_rate(enum ccm_clk_src source)
{
	u32 ctrl;
	bool clk_on;

	switch (source) {
	case ARM_PLL_CLK:
		ctrl = readl(&ana_regs->arm_pll.ctrl.reg);
	case AUDIO_PLL_CLK:
		ctrl = readl(&ana_regs->audio_pll.ctrl.reg);
		break;
	case DRAM_PLL_CLK:
		ctrl = readl(&ana_regs->dram_pll.ctrl.reg);
		break;
	case VIDEO_PLL_CLK:
		ctrl = readl(&ana_regs->video_pll.ctrl.reg);
		break;
	case SYS_PLL_PFD0:
	case SYS_PLL_PFD0_DIV2:
		ctrl = readl(&ana_regs->sys_pll.dfs[0].dfs_ctrl.reg);
		break;
	case SYS_PLL_PFD1:
	case SYS_PLL_PFD1_DIV2:
		ctrl = readl(&ana_regs->sys_pll.dfs[1].dfs_ctrl.reg);
		break;
	case SYS_PLL_PFD2:
	case SYS_PLL_PFD2_DIV2:
		ctrl = readl(&ana_regs->sys_pll.dfs[2].dfs_ctrl.reg);
		break;
	case OSC_24M_CLK:
		return 24000;
	default:
		printf("Invalid clock source to get rate\n");
		return 0;
	}

	if (ctrl & PLL_CTRL_HW_CTRL_SEL) {
		/* When using HW ctrl, check OSCPLL */
		clk_on = ccm_clk_src_is_clk_on(source);
		if (clk_on)
			return decode_pll(source);
		else
			return 0;
	} else {
		/* controlled by pll registers */
		return decode_pll(source);
	}
}

u32 get_arm_core_clk(void)
{
	u32 val;

	ccm_shared_gpr_get(SHARED_GPR_A55_CLK, &val);

	if (val & SHARED_GPR_A55_CLK_SEL_PLL)
		return decode_pll(ARM_PLL_CLK) * 1000;

	return ccm_clk_root_get_rate(ARM_A55_CLK_ROOT);
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_arm_core_clk();
	case MXC_IPG_CLK:
		return ccm_clk_root_get_rate(BUS_WAKEUP_CLK_ROOT);
	case MXC_CSPI_CLK:
		return ccm_clk_root_get_rate(LPSPI1_CLK_ROOT);
	case MXC_ESDHC_CLK:
		return ccm_clk_root_get_rate(USDHC1_CLK_ROOT);
	case MXC_ESDHC2_CLK:
		return ccm_clk_root_get_rate(USDHC2_CLK_ROOT);
	case MXC_ESDHC3_CLK:
		return ccm_clk_root_get_rate(USDHC3_CLK_ROOT);
	case MXC_UART_CLK:
		return ccm_clk_root_get_rate(LPUART1_CLK_ROOT);
	case MXC_FLEXSPI_CLK:
		return ccm_clk_root_get_rate(FLEXSPI1_CLK_ROOT);
	default:
		return -1;
	};

	return -1;
};

int enable_i2c_clk(unsigned char enable, u32 i2c_num)
{
	if (i2c_num > 7)
		return -EINVAL;

	if (enable) {
		/* 24M */
		ccm_lpcg_on(CCGR_I2C1 + i2c_num, false);
		ccm_clk_root_cfg(LPI2C1_CLK_ROOT + i2c_num, OSC_24M_CLK, 1);
		ccm_lpcg_on(CCGR_I2C1 + i2c_num, true);
	} else {
		ccm_lpcg_on(CCGR_I2C1 + i2c_num, false);
	}

	return 0;
}

u32 imx_get_i2cclk(u32 i2c_num)
{
	if (i2c_num > 7)
		return -EINVAL;

	return ccm_clk_root_get_rate(LPI2C1_CLK_ROOT + i2c_num);
}

u32 get_lpuart_clk(void)
{
	return mxc_get_clock(MXC_UART_CLK);
}

void init_uart_clk(u32 index)
{
	switch (index) {
	case LPUART1_CLK_ROOT:
		/* 24M */
		ccm_lpcg_on(CCGR_URT1, false);
		ccm_clk_root_cfg(LPUART1_CLK_ROOT, OSC_24M_CLK, 1);
		ccm_lpcg_on(CCGR_URT1, true);
		break;
	default:
		break;
	}
}

void init_clk_usdhc(u32 index)
{
	u32 div;

	if (IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE))
		div = 3; /* 266.67 Mhz */
	else
		div = 2; /* 400 Mhz */

	switch (index) {
	case 0:
		ccm_lpcg_on(CCGR_USDHC1, 0);
		ccm_clk_root_cfg(USDHC1_CLK_ROOT, SYS_PLL_PFD1, div);
		ccm_lpcg_on(CCGR_USDHC1, 1);
		break;
	case 1:
		ccm_lpcg_on(CCGR_USDHC2, 0);
		ccm_clk_root_cfg(USDHC2_CLK_ROOT, SYS_PLL_PFD1, div);
		ccm_lpcg_on(CCGR_USDHC2, 1);
		break;
	case 2:
		ccm_lpcg_on(CCGR_USDHC3, 0);
		ccm_clk_root_cfg(USDHC3_CLK_ROOT, SYS_PLL_PFD1, div);
		ccm_lpcg_on(CCGR_USDHC3, 1);
		break;
	default:
		return;
	};
}

void enable_usboh3_clk(unsigned char enable)
{
	if (enable) {
		ccm_clk_root_cfg(HSIO_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3);
		ccm_lpcg_on(CCGR_USBC, 1);
	} else {
		ccm_lpcg_on(CCGR_USBC, 0);
	}
}

#ifdef CONFIG_SPL_BUILD
void dram_pll_init(ulong pll_val)
{
	configure_fracpll(DRAM_PLL_CLK, pll_val);
}

void dram_enable_bypass(ulong clk_val)
{
	switch (clk_val) {
	case MHZ(625):
		ccm_clk_root_cfg(DRAM_ALT_CLK_ROOT, SYS_PLL_PFD2, 1);
		break;
	case MHZ(400):
		ccm_clk_root_cfg(DRAM_ALT_CLK_ROOT, SYS_PLL_PFD1, 2);
		break;
	case MHZ(333):
		ccm_clk_root_cfg(DRAM_ALT_CLK_ROOT, SYS_PLL_PFD0, 3);
		break;
	case MHZ(200):
		ccm_clk_root_cfg(DRAM_ALT_CLK_ROOT, SYS_PLL_PFD1, 4);
		break;
	case MHZ(100):
		ccm_clk_root_cfg(DRAM_ALT_CLK_ROOT, SYS_PLL_PFD1, 8);
		break;
	default:
		printf("No matched freq table %lu\n", clk_val);
		return;
	}

	/* Set DRAM APB to 133Mhz */
	ccm_clk_root_cfg(DRAM_APB_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3);
	/* Switch from DRAM  clock root from PLL to CCM */
	ccm_shared_gpr_set(SHARED_GPR_DRAM_CLK, SHARED_GPR_DRAM_CLK_SEL_CCM);
}

void dram_disable_bypass(void)
{
	/* Set DRAM APB to 133Mhz */
	ccm_clk_root_cfg(DRAM_APB_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3);
	/* Switch from DRAM  clock root from CCM to PLL */
	ccm_shared_gpr_set(SHARED_GPR_DRAM_CLK, SHARED_GPR_DRAM_CLK_SEL_PLL);
}

void set_arm_clk(ulong freq)
{
	/* Increase ARM clock to 1.7Ghz */
	ccm_shared_gpr_set(SHARED_GPR_A55_CLK, SHARED_GPR_A55_CLK_SEL_CCM);
	configure_intpll(ARM_PLL_CLK, freq);
	ccm_shared_gpr_set(SHARED_GPR_A55_CLK, SHARED_GPR_A55_CLK_SEL_PLL);
}

void set_arm_core_max_clk(void)
{
	/* Increase ARM clock to max rate according to speed grade */
	u32 speed = get_cpu_speed_grade_hz();

	set_arm_clk(speed);
}

#endif

#if IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE)
struct imx_clk_setting imx_clk_settings[] = {
	/* Set A55 clk to 500M */
	{ARM_A55_CLK_ROOT, SYS_PLL_PFD0, 2},
	/* Set A55 periphal to 200M */
	{ARM_A55_PERIPH_CLK_ROOT, SYS_PLL_PFD1, 4},
	/* Set A55 mtr bus to 133M */
	{ARM_A55_MTR_BUS_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},

	/* ELE to 133M */
	{ELE_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* Bus_wakeup to 133M */
	{BUS_WAKEUP_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* Bus_AON to 133M */
	{BUS_AON_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* M33 to 133M */
	{M33_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* WAKEUP_AXI to 200M  */
	{WAKEUP_AXI_CLK_ROOT, SYS_PLL_PFD1, 4},
	/* SWO TRACE to 133M */
	{SWO_TRACE_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* M33 systetick to 24M */
	{M33_SYSTICK_CLK_ROOT, OSC_24M_CLK, 1},
	/* NIC to 250M */
	{NIC_CLK_ROOT, SYS_PLL_PFD0, 4},
	/* NIC_APB to 133M */
	{NIC_APB_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3}
};
#else
struct imx_clk_setting imx_clk_settings[] = {
	/*
	 * Set A55 clk to 500M. This clock root is normally used as intermediate
	 * clock source for A55 core/DSU when doing ARM PLL reconfig. set it to
	 * 500MHz(LD mode frequency) should be ok.
	 */
	{ARM_A55_CLK_ROOT, SYS_PLL_PFD0, 2},
	/* Set A55 periphal to 333M */
	{ARM_A55_PERIPH_CLK_ROOT, SYS_PLL_PFD0, 3},
	/* Set A55 mtr bus to 133M */
	{ARM_A55_MTR_BUS_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* ELE to 200M */
	{ELE_CLK_ROOT, SYS_PLL_PFD1_DIV2, 2},
	/* Bus_wakeup to 133M */
	{BUS_WAKEUP_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* Bus_AON to 133M */
	{BUS_AON_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* M33 to 200M */
	{M33_CLK_ROOT, SYS_PLL_PFD1_DIV2, 2},
	/*
	 * WAKEUP_AXI to 312.5M, because of FEC only can support to 320M for
	 * generating MII clock at 2.5M
	 */
	{WAKEUP_AXI_CLK_ROOT, SYS_PLL_PFD2, 2},
	/* SWO TRACE to 133M */
	{SWO_TRACE_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3},
	/* M33 systetick to 24M */
	{M33_SYSTICK_CLK_ROOT, OSC_24M_CLK, 1},
	/* NIC to 400M */
	{NIC_CLK_ROOT, SYS_PLL_PFD1, 2},
	/* NIC_APB to 133M */
	{NIC_APB_CLK_ROOT, SYS_PLL_PFD1_DIV2, 3}
};
#endif

int clock_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(imx_clk_settings); i++) {
		ccm_clk_root_cfg(imx_clk_settings[i].clk_root,
				 imx_clk_settings[i].src, imx_clk_settings[i].div);
	}

	if (IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE))
		set_arm_clk(MHZ(900));

	/* allow for non-secure access */
	for (i = 0; i < OSCPLL_END; i++)
		ccm_clk_src_tz_access(i, true, false, false);

	for (i = 0; i < CLK_ROOT_NUM; i++)
		ccm_clk_root_tz_access(i, true, false, false);

	for (i = 0; i < CCGR_NUM; i++)
		ccm_lpcg_tz_access(i, true, false, false);

	for (i = 0; i < SHARED_GPR_NUM; i++)
		ccm_shared_gpr_tz_access(i, true, false, false);

	return 0;
}

int set_clk_eqos(enum enet_freq type)
{
	u32 eqos_post_div;

	switch (type) {
	case ENET_125MHZ:
		eqos_post_div = 2; /* 250M clock */
		break;
	case ENET_50MHZ:
		eqos_post_div = 5; /* 100M clock */
		break;
	case ENET_25MHZ:
		eqos_post_div = 10; /* 50M clock*/
		break;
	default:
		return -EINVAL;
	}

	/* disable the clock first */
	ccm_lpcg_on(CCGR_ENETQOS, false);

	ccm_clk_root_cfg(ENET_CLK_ROOT, SYS_PLL_PFD0_DIV2, eqos_post_div);
	ccm_clk_root_cfg(ENET_TIMER2_CLK_ROOT, SYS_PLL_PFD0_DIV2, 5);

	/* enable clock */
	ccm_lpcg_on(CCGR_ENETQOS, true);

	return 0;
}

u32 imx_get_eqos_csr_clk(void)
{
	return ccm_clk_root_get_rate(WAKEUP_AXI_CLK_ROOT);
}

u32 imx_get_fecclk(void)
{
	return ccm_clk_root_get_rate(WAKEUP_AXI_CLK_ROOT);
}

#if defined(CONFIG_IMX93) && defined(CONFIG_DWC_ETH_QOS)
static int imx93_eqos_interface_init(struct udevice *dev, phy_interface_t interface_type)
{
	struct blk_ctrl_wakeupmix_regs *bctrl =
		(struct blk_ctrl_wakeupmix_regs *)BLK_CTRL_WAKEUPMIX_BASE_ADDR;

	clrbits_le32(&bctrl->eqos_gpr,
		     BCTRL_GPR_ENET_QOS_INTF_MODE_MASK |
		     BCTRL_GPR_ENET_QOS_CLK_GEN_EN);

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
		setbits_le32(&bctrl->eqos_gpr,
			     BCTRL_GPR_ENET_QOS_INTF_SEL_MII |
			     BCTRL_GPR_ENET_QOS_CLK_GEN_EN);
		break;
	case PHY_INTERFACE_MODE_RMII:
		setbits_le32(&bctrl->eqos_gpr,
			     BCTRL_GPR_ENET_QOS_INTF_SEL_RMII |
			     BCTRL_GPR_ENET_QOS_CLK_GEN_EN);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		setbits_le32(&bctrl->eqos_gpr,
			     BCTRL_GPR_ENET_QOS_INTF_SEL_RGMII |
			     BCTRL_GPR_ENET_QOS_CLK_GEN_EN);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#else
static int imx93_eqos_interface_init(struct udevice *dev, phy_interface_t interface_type)
{
	return 0;
}
#endif

int board_interface_eth_init(struct udevice *dev, phy_interface_t interface_type)
{
	if (IS_ENABLED(CONFIG_IMX93) &&
	    IS_ENABLED(CONFIG_DWC_ETH_QOS) &&
	    device_is_compatible(dev, "nxp,imx93-dwmac-eqos"))
		return imx93_eqos_interface_init(dev, interface_type);

	if (IS_ENABLED(CONFIG_IMX93) &&
	    IS_ENABLED(CONFIG_FEC_MXC) &&
	    device_is_compatible(dev, "fsl,imx93-fec"))
		return 0;

	return -EINVAL;
}

int set_clk_enet(enum enet_freq type)
{
	u32 div;

	/* disable the clock first */
	ccm_lpcg_on(CCGR_ENET1, false);

	switch (type) {
	case ENET_125MHZ:
		div = 2; /* 250Mhz */
		break;
	case ENET_50MHZ:
		div = 5; /* 100Mhz */
		break;
	case ENET_25MHZ:
		div = 10; /* 50Mhz */
		break;
	default:
		return -EINVAL;
	}

	ccm_clk_root_cfg(ENET_REF_CLK_ROOT, SYS_PLL_PFD0_DIV2, div);
	ccm_clk_root_cfg(ENET_TIMER1_CLK_ROOT, SYS_PLL_PFD0_DIV2, 5);

#ifdef CONFIG_FEC_MXC_25M_REF_CLK
	ccm_clk_root_cfg(ENET_REF_PHY_CLK_ROOT, SYS_PLL_PFD0_DIV2, 20);
#endif

	/* enable clock */
	ccm_lpcg_on(CCGR_ENET1, true);

	return 0;
}

/*
 * Dump some clockes.
 */
#ifndef CONFIG_SPL_BUILD
int do_showclocks(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	u32 freq;

	freq = decode_pll(ARM_PLL_CLK);
	printf("ARM_PLL    %8d MHz\n", freq / 1000);
	freq = decode_pll(DRAM_PLL_CLK);
	printf("DRAM_PLL    %8d MHz\n", freq / 1000);
	freq = decode_pll(SYS_PLL_PFD0);
	printf("SYS_PLL_PFD0    %8d MHz\n", freq / 1000);
	freq = decode_pll(SYS_PLL_PFD0_DIV2);
	printf("SYS_PLL_PFD0_DIV2    %8d MHz\n", freq / 1000);
	freq = decode_pll(SYS_PLL_PFD1);
	printf("SYS_PLL_PFD1    %8d MHz\n", freq / 1000);
	freq = decode_pll(SYS_PLL_PFD1_DIV2);
	printf("SYS_PLL_PFD1_DIV2    %8d MHz\n", freq / 1000);
	freq = decode_pll(SYS_PLL_PFD2);
	printf("SYS_PLL_PFD2    %8d MHz\n", freq / 1000);
	freq = decode_pll(SYS_PLL_PFD2_DIV2);
	printf("SYS_PLL_PFD2_DIV2    %8d MHz\n", freq / 1000);
	freq = mxc_get_clock(MXC_ARM_CLK);
	printf("ARM CORE    %8d MHz\n", freq / 1000000);
	freq = mxc_get_clock(MXC_IPG_CLK);
	printf("IPG         %8d MHz\n", freq / 1000000);
	freq = mxc_get_clock(MXC_UART_CLK);
	printf("UART3          %8d MHz\n", freq / 1000000);
	freq = mxc_get_clock(MXC_ESDHC_CLK);
	printf("USDHC1         %8d MHz\n", freq / 1000000);
	freq = mxc_get_clock(MXC_FLEXSPI_CLK);
	printf("FLEXSPI           %8d MHz\n", freq / 1000000);

	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_showclocks,
	"display clocks",
	""
);
#endif
