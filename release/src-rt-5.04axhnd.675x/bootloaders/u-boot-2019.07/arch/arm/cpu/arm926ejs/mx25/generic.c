// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Based on mx27/generic.c:
 *  Copyright (c) 2008 Eric Jarrige <eric.jarrige@armadeus.org>
 *  Copyright (c) 2009 Ilya Yanok <yanok@emcraft.com>
 */

#include <common.h>
#include <div64.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch-imx/cpu.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>

DECLARE_GLOBAL_DATA_PTR;
#endif

/*
 *  get the system pll clock in Hz
 *
 *                  mfi + mfn / (mfd +1)
 *  f = 2 * f_ref * --------------------
 *                        pd + 1
 */
static unsigned int imx_decode_pll(unsigned int pll, unsigned int f_ref)
{
	unsigned int mfi = (pll >> CCM_PLL_MFI_SHIFT)
	    & CCM_PLL_MFI_MASK;
	int mfn = (pll >> CCM_PLL_MFN_SHIFT)
	    & CCM_PLL_MFN_MASK;
	unsigned int mfd = (pll >> CCM_PLL_MFD_SHIFT)
	    & CCM_PLL_MFD_MASK;
	unsigned int pd = (pll >> CCM_PLL_PD_SHIFT)
	    & CCM_PLL_PD_MASK;

	mfi = mfi <= 5 ? 5 : mfi;
	mfn = mfn >= 512 ? mfn - 1024 : mfn;
	mfd += 1;
	pd += 1;

	return lldiv(2 * (u64) f_ref * (mfi * mfd + mfn),
		     mfd * pd);
}

static ulong imx_get_mpllclk(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong fref = MXC_HCLK;

	return imx_decode_pll(readl(&ccm->mpctl), fref);
}

static ulong imx_get_upllclk(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong fref = MXC_HCLK;

	return imx_decode_pll(readl(&ccm->upctl), fref);
}

static ulong imx_get_armclk(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong cctl = readl(&ccm->cctl);
	ulong fref = imx_get_mpllclk();
	ulong div;

	if (cctl & CCM_CCTL_ARM_SRC)
		fref = lldiv((u64) fref * 3, 4);

	div = ((cctl >> CCM_CCTL_ARM_DIV_SHIFT)
	       & CCM_CCTL_ARM_DIV_MASK) + 1;

	return fref / div;
}

static ulong imx_get_ahbclk(void)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong cctl = readl(&ccm->cctl);
	ulong fref = imx_get_armclk();
	ulong div;

	div = ((cctl >> CCM_CCTL_AHB_DIV_SHIFT)
	       & CCM_CCTL_AHB_DIV_MASK) + 1;

	return fref / div;
}

static ulong imx_get_ipgclk(void)
{
	return imx_get_ahbclk() / 2;
}

static ulong imx_get_perclk(int clk)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong fref = readl(&ccm->mcr) & (1 << clk) ? imx_get_upllclk() :
						     imx_get_ahbclk();
	ulong div;

	div = readl(&ccm->pcdr[CCM_PERCLK_REG(clk)]);
	div = ((div >> CCM_PERCLK_SHIFT(clk)) & CCM_PERCLK_MASK) + 1;

	return fref / div;
}

int imx_set_perclk(enum mxc_clock clk, bool from_upll, unsigned int freq)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong fref = from_upll ? imx_get_upllclk() : imx_get_ahbclk();
	ulong div = (fref + freq - 1) / freq;

	if (clk > MXC_UART_CLK || !div || --div > CCM_PERCLK_MASK)
		return -EINVAL;

	clrsetbits_le32(&ccm->pcdr[CCM_PERCLK_REG(clk)],
			CCM_PERCLK_MASK << CCM_PERCLK_SHIFT(clk),
			div << CCM_PERCLK_SHIFT(clk));
	if (from_upll)
		setbits_le32(&ccm->mcr, 1 << clk);
	else
		clrbits_le32(&ccm->mcr, 1 << clk);
	return 0;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	if (clk >= MXC_CLK_NUM)
		return -1;
	switch (clk) {
	case MXC_ARM_CLK:
		return imx_get_armclk();
	case MXC_AHB_CLK:
		return imx_get_ahbclk();
	case MXC_IPG_CLK:
	case MXC_CSPI_CLK:
	case MXC_FEC_CLK:
		return imx_get_ipgclk();
	default:
		return imx_get_perclk(clk);
	}
}

u32 get_cpu_rev(void)
{
	u32 srev;
	u32 system_rev = 0x25000;

	/* read SREV register from IIM module */
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	srev = readl(&iim->iim_srev);

	switch (srev) {
	case 0x00:
		system_rev |= CHIP_REV_1_0;
		break;
	case 0x01:
		system_rev |= CHIP_REV_1_1;
		break;
	case 0x02:
		system_rev |= CHIP_REV_1_2;
		break;
	default:
		system_rev |= 0x8000;
		break;
	}

	return system_rev;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	/* read RCSR register from CCM module */
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	u32 cause = readl(&ccm->rcsr) & 0x0f;

	if (cause == 0)
		return "POR";
	else if (cause == 1)
		return "RST";
	else if ((cause & 2) == 2)
		return "WDOG";
	else if ((cause & 4) == 4)
		return "SW RESET";
	else if ((cause & 8) == 8)
		return "JTAG";
	else
		return "unknown reset";

}

int print_cpuinfo(void)
{
	char buf[32];
	u32 cpurev = get_cpu_rev();

	printf("CPU:   Freescale i.MX25 rev%d.%d%s at %s MHz\n",
		(cpurev & 0xF0) >> 4, (cpurev & 0x0F),
		((cpurev & 0x8000) ? " unknown" : ""),
		strmhz(buf, imx_get_armclk()));
	printf("Reset cause: %s\n", get_reset_cause());
	return 0;
}
#endif

#if defined(CONFIG_FEC_MXC)
/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	ulong val;

	val = readl(&ccm->cgr0);
	val |= (1 << 23);
	writel(val, &ccm->cgr0);
	return fecmxc_initialize(bis);
}
#endif

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
#if CONFIG_SYS_FSL_ESDHC_ADDR == IMX_MMC_SDHC2_BASE
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
#else
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);
#endif
#endif
	return 0;
}

#ifdef CONFIG_FSL_ESDHC
/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

#ifdef CONFIG_FEC_MXC
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	int i;
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[0];
	struct fuse_bank0_regs *fuse =
			(struct fuse_bank0_regs *)bank->fuse_regs;

	for (i = 0; i < 6; i++)
		mac[i] = readl(&fuse->mac_addr[i]) & 0xff;
}
#endif /* CONFIG_FEC_MXC */
