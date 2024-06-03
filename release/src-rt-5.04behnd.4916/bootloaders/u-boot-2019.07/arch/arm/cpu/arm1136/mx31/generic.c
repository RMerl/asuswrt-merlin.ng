// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 */

#include <common.h>
#include <div64.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>

static u32 mx31_decode_pll(u32 reg, u32 infreq)
{
	u32 mfi = GET_PLL_MFI(reg);
	s32 mfn = GET_PLL_MFN(reg);
	u32 mfd = GET_PLL_MFD(reg);
	u32 pd =  GET_PLL_PD(reg);

	mfi = mfi <= 5 ? 5 : mfi;
	mfn = mfn >= 512 ? mfn - 1024 : mfn;
	mfd += 1;
	pd += 1;

	return lldiv(2 * (u64)infreq * (mfi * mfd + mfn),
		mfd * pd);
}

static u32 mx31_get_mpl_dpdgck_clk(void)
{
	u32 infreq;

	if ((readl(CCM_CCMR) & CCMR_PRCS_MASK) == CCMR_FPM)
		infreq = MXC_CLK32 * 1024;
	else
		infreq = MXC_HCLK;

	return mx31_decode_pll(readl(CCM_MPCTL), infreq);
}

static u32 mx31_get_mcu_main_clk(void)
{
	/* For now we assume mpl_dpdgck_clk == mcu_main_clk
	 * which should be correct for most boards
	 */
	return mx31_get_mpl_dpdgck_clk();
}

static u32 mx31_get_ipg_clk(void)
{
	u32 freq = mx31_get_mcu_main_clk();
	u32 pdr0 = readl(CCM_PDR0);

	freq /= GET_PDR0_MAX_PODF(pdr0) + 1;
	freq /= GET_PDR0_IPG_PODF(pdr0) + 1;

	return freq;
}

/* hsp is the clock for the ipu */
static u32 mx31_get_hsp_clk(void)
{
	u32 freq = mx31_get_mcu_main_clk();
	u32 pdr0 = readl(CCM_PDR0);

	freq /= GET_PDR0_HSP_PODF(pdr0) + 1;

	return freq;
}

void mx31_dump_clocks(void)
{
	u32 cpufreq = mx31_get_mcu_main_clk();
	printf("mx31 cpu clock: %dMHz\n", cpufreq / 1000000);
	printf("ipg clock     : %dHz\n", mx31_get_ipg_clk());
	printf("hsp clock     : %dHz\n", mx31_get_hsp_clk());
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return mx31_get_mcu_main_clk();
	case MXC_IPG_CLK:
	case MXC_IPG_PERCLK:
	case MXC_CSPI_CLK:
	case MXC_UART_CLK:
	case MXC_ESDHC_CLK:
	case MXC_I2C_CLK:
		return mx31_get_ipg_clk();
	case MXC_IPU_CLK:
		return mx31_get_hsp_clk();
	}
	return -1;
}

u32 imx_get_uartclk(void)
{
	return mxc_get_clock(MXC_UART_CLK);
}

void mx31_gpio_mux(unsigned long mode)
{
	unsigned long reg, shift, tmp;

	reg = IOMUXC_BASE + (mode & 0x1fc);
	shift = (~mode & 0x3) * 8;

	tmp = readl(reg);
	tmp &= ~(0xff << shift);
	tmp |= ((mode >> IOMUX_MODE_POS) & 0xff) << shift;
	writel(tmp, reg);
}

void mx31_set_pad(enum iomux_pins pin, u32 config)
{
	u32 field, l, reg;

	pin &= IOMUX_PADNUM_MASK;
	reg = (IOMUXC_BASE + 0x154) + (pin + 2) / 3 * 4;
	field = (pin + 2) % 3;

	l = readl(reg);
	l &= ~(0x1ff << (field * 10));
	l |= config << (field * 10);
	writel(l, reg);

}

void mx31_set_gpr(enum iomux_gp_func gp, char en)
{
	u32 l;
	struct iomuxc_regs *iomuxc = (struct iomuxc_regs *)IOMUXC_BASE;

	l = readl(&iomuxc->gpr);
	if (en)
		l |= gp;
	else
		l &= ~gp;

	writel(l, &iomuxc->gpr);
}

void mxc_setup_weimcs(int cs, const struct mxc_weimcs *weimcs)
{
	struct mx31_weim *weim = (struct mx31_weim *) WEIM_BASE;
	struct mx31_weim_cscr *cscr = &weim->cscr[cs];

	writel(weimcs->upper, &cscr->upper);
	writel(weimcs->lower, &cscr->lower);
	writel(weimcs->additional, &cscr->additional);
}

struct mx3_cpu_type mx31_cpu_type[] = {
	{ .srev = 0x00, .v = 0x10 },
	{ .srev = 0x10, .v = 0x11 },
	{ .srev = 0x11, .v = 0x11 },
	{ .srev = 0x12, .v = 0x1F },
	{ .srev = 0x13, .v = 0x1F },
	{ .srev = 0x14, .v = 0x12 },
	{ .srev = 0x15, .v = 0x12 },
	{ .srev = 0x28, .v = 0x20 },
	{ .srev = 0x29, .v = 0x20 },
};

u32 get_cpu_rev(void)
{
	u32 i, srev;

	/* read SREV register from IIM module */
	struct iim_regs *iim = (struct iim_regs *)MX31_IIM_BASE_ADDR;
	srev = readl(&iim->iim_srev);

	for (i = 0; i < ARRAY_SIZE(mx31_cpu_type); i++)
		if (srev == mx31_cpu_type[i].srev)
			return mx31_cpu_type[i].v | (MXC_CPU_MX31 << 12);

	return srev | 0x8000;
}

static char *get_reset_cause(void)
{
	/* read RCSR register from CCM module */
	struct clock_control_regs *ccm =
		(struct clock_control_regs *)CCM_BASE;

	u32 cause = readl(&ccm->rcsr) & 0x07;

	switch (cause) {
	case 0x0000:
		return "POR";
	case 0x0001:
		return "RST";
	case 0x0002:
		return "WDOG";
	case 0x0006:
		return "JTAG";
	case 0x0007:
		return "ARM11P power gating";
	default:
		return "unknown reset";
	}
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	u32 srev = get_cpu_rev();

	printf("CPU:   Freescale i.MX31 rev %d.%d%s at %d MHz.\n",
			(srev & 0xF0) >> 4, (srev & 0x0F),
			((srev & 0x8000) ? " unknown" : ""),
			mx31_get_mcu_main_clk() / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());
	return 0;
}
#endif
