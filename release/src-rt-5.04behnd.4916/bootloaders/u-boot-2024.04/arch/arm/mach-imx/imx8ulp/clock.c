// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <command.h>
#include <div64.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/clock.h>
#include <asm/arch/pcc.h>
#include <asm/arch/cgc.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define PLL_USB_EN_USB_CLKS_MASK	(0x01 << 6)
#define PLL_USB_PWR_MASK		(0x01 << 12)
#define PLL_USB_ENABLE_MASK		(0x01 << 13)
#define PLL_USB_BYPASS_MASK		(0x01 << 16)
#define PLL_USB_REG_ENABLE_MASK		(0x01 << 21)
#define PLL_USB_DIV_SEL_MASK		(0x07 << 22)
#define PLL_USB_LOCK_MASK		(0x01 << 31)
#define PCC5_LPDDR4_ADDR 0x2da70108

static void lpuart_set_clk(u32 index, enum cgc_clk clk)
{
	const u32 lpuart_pcc_slots[] = {
		LPUART4_PCC3_SLOT,
		LPUART5_PCC3_SLOT,
		LPUART6_PCC4_SLOT,
		LPUART7_PCC4_SLOT,
	};

	const u32 lpuart_pcc[] = {
		3, 3, 4, 4,
	};

	if (index > 3)
		return;

	pcc_clock_enable(lpuart_pcc[index], lpuart_pcc_slots[index], false);
	pcc_clock_sel(lpuart_pcc[index], lpuart_pcc_slots[index], clk);
	pcc_clock_enable(lpuart_pcc[index], lpuart_pcc_slots[index], true);

	pcc_reset_peripheral(lpuart_pcc[index], lpuart_pcc_slots[index], false);
}

static void init_clk_lpuart(void)
{
	u32 index = 0, i;

	const u32 lpuart_array[] = {
		LPUART4_RBASE,
		LPUART5_RBASE,
		LPUART6_RBASE,
		LPUART7_RBASE,
	};

	for (i = 0; i < 4; i++) {
		if (lpuart_array[i] == LPUART_BASE) {
			index = i;
			break;
		}
	}

	lpuart_set_clk(index, SOSC_DIV2);
}

void init_clk_fspi(int index)
{
	pcc_clock_enable(4, FLEXSPI2_PCC4_SLOT, false);
	pcc_clock_sel(4, FLEXSPI2_PCC4_SLOT, PLL3_PFD2_DIV1);
	pcc_clock_div_config(4, FLEXSPI2_PCC4_SLOT, false, 8);
	pcc_clock_enable(4, FLEXSPI2_PCC4_SLOT, true);
	pcc_reset_peripheral(4, FLEXSPI2_PCC4_SLOT, false);
}

void setclkout_ddr(void)
{
	writel(0x12800000, 0x2DA60020);
	writel(0xa00, 0x298C0000); /* PTD0 */
}

void ddrphy_pll_lock(void)
{
	writel(0x00011542, 0x2E065964);
	writel(0x00011542, 0x2E06586C);

	writel(0x00000B01, 0x2E062000);
	writel(0x00000B01, 0x2E060000);
}

void init_clk_ddr(void)
{
	/* disable the ddr pcc */
	writel(0xc0000000, PCC5_LPDDR4_ADDR);

	/* enable pll4 and ddrclk*/
	cgc2_pll4_init(true);
	cgc2_ddrclk_config(4, 1);

	/* enable ddr pcc */
	writel(0xd0000000, PCC5_LPDDR4_ADDR);

	/* Wait until ddrclk reg lock bit is cleared, so that the div update is finished */
	cgc2_ddrclk_wait_unlock();

	/* for debug */
	/* setclkout_ddr(); */
}

int set_ddr_clk(u32 phy_freq_mhz)
{
	debug("%s %u\n", __func__, phy_freq_mhz);

	if (phy_freq_mhz == 48) {
		writel(0x90000000, PCC5_LPDDR4_ADDR); /* disable ddr pcc */
		cgc2_ddrclk_config(2, 0); /* 24Mhz DDR clock */
		writel(0xd0000000, PCC5_LPDDR4_ADDR); /* enable ddr pcc */
	} else if (phy_freq_mhz == 384) {
		writel(0x90000000, PCC5_LPDDR4_ADDR); /* disable ddr pcc */
		cgc2_ddrclk_config(0, 0); /* 192Mhz DDR clock */
		writel(0xd0000000, PCC5_LPDDR4_ADDR); /* enable ddr pcc */
	} else if (phy_freq_mhz == 528) {
		writel(0x90000000, PCC5_LPDDR4_ADDR); /* disable ddr pcc */
		cgc2_ddrclk_config(4, 1); /* 264Mhz DDR clock */
		writel(0xd0000000, PCC5_LPDDR4_ADDR); /* enable ddr pcc */
	} else if (phy_freq_mhz == 264) {
		writel(0x90000000, PCC5_LPDDR4_ADDR); /* disable ddr pcc */
		cgc2_ddrclk_config(4, 3); /* 132Mhz DDR clock */
		writel(0xd0000000, PCC5_LPDDR4_ADDR); /* enable ddr pcc */
	} else if (phy_freq_mhz == 192) {
		writel(0x90000000, PCC5_LPDDR4_ADDR); /* disable ddr pcc */
		cgc2_ddrclk_config(0, 1); /* 96Mhz DDR clock */
		writel(0xd0000000, PCC5_LPDDR4_ADDR); /* enable ddr pcc */
	} else if (phy_freq_mhz == 96) {
		writel(0x90000000, PCC5_LPDDR4_ADDR); /* disable ddr pcc */
		cgc2_ddrclk_config(0, 3); /* 48Mhz DDR clock */
		writel(0xd0000000, PCC5_LPDDR4_ADDR); /* enable ddr pcc */
	} else {
		printf("ddr phy clk %uMhz is not supported\n", phy_freq_mhz);
		return -EINVAL;
	}

	/* Wait until ddrclk reg lock bit is cleared, so that the div update is finished */
	cgc2_ddrclk_wait_unlock();

	return 0;
}

void clock_init_early(void)
{
	cgc1_soscdiv_init();

	init_clk_lpuart();

	/* Enable upower mu1 clk */
	pcc_clock_enable(3, UPOWER_PCC3_SLOT, true);
}

/* This will be invoked after pmic voltage setting */
void clock_init_late(void)
{

	if (IS_ENABLED(CONFIG_IMX8ULP_LD_MODE))
		cgc1_init_core_clk(MHZ(500));
	else if (IS_ENABLED(CONFIG_IMX8ULP_ND_MODE))
		cgc1_init_core_clk(MHZ(750));
	else
		cgc1_init_core_clk(MHZ(960));

	/*
	 * Audio use this frequency in kernel dts,
	 * however nic use pll3 pfd0, we have to
	 * make the freqency same as kernel to make nic
	 * not being disabled
	 */
	cgc1_pll3_init(540672000);

	pcc_clock_enable(4, SDHC0_PCC4_SLOT, false);
	pcc_clock_sel(4, SDHC0_PCC4_SLOT, PLL3_PFD3_DIV1); /* 389M for OD, 194M for LD/ND*/
	pcc_clock_enable(4, SDHC0_PCC4_SLOT, true);
	pcc_reset_peripheral(4, SDHC0_PCC4_SLOT, false);

	pcc_clock_enable(4, SDHC1_PCC4_SLOT, false);
	pcc_clock_sel(4, SDHC1_PCC4_SLOT, PLL3_PFD3_DIV2); /* 194M for OD, 97M for LD/ND */
	pcc_clock_enable(4, SDHC1_PCC4_SLOT, true);
	pcc_reset_peripheral(4, SDHC1_PCC4_SLOT, false);

	pcc_clock_enable(4, SDHC2_PCC4_SLOT, false);
	pcc_clock_sel(4, SDHC2_PCC4_SLOT, PLL3_PFD3_DIV2); /* 194M for OD, 97M for LD/ND*/
	pcc_clock_enable(4, SDHC2_PCC4_SLOT, true);
	pcc_reset_peripheral(4, SDHC2_PCC4_SLOT, false);

	/* enable MU0_MUB clock before access the register of MU0_MUB */
	pcc_clock_enable(3, MU0_B_PCC3_SLOT, true);

	/*
	 * Enable clock division
	 * TODO: may not needed after ROM ready.
	 */
}

#if IS_ENABLED(CONFIG_SYS_I2C_IMX_LPI2C)
int enable_i2c_clk(unsigned char enable, u32 i2c_num)
{
	/* Set parent to FIRC DIV2 clock */
	const u32 lpi2c_pcc_clks[] = {
		LPI2C4_PCC3_SLOT << 8 | 3,
		LPI2C5_PCC3_SLOT << 8 | 3,
		LPI2C6_PCC4_SLOT << 8 | 4,
		LPI2C7_PCC4_SLOT << 8 | 4,
	};

	if (i2c_num == 0)
		return 0;

	if (i2c_num < 4 || i2c_num > 7)
		return -EINVAL;

	if (enable) {
		pcc_clock_enable(lpi2c_pcc_clks[i2c_num - 4] & 0xff,
				 lpi2c_pcc_clks[i2c_num - 4] >> 8, false);
		pcc_clock_sel(lpi2c_pcc_clks[i2c_num - 4] & 0xff,
			      lpi2c_pcc_clks[i2c_num - 4] >> 8, SOSC_DIV2);
		pcc_clock_enable(lpi2c_pcc_clks[i2c_num - 4] & 0xff,
				 lpi2c_pcc_clks[i2c_num - 4] >> 8, true);
		pcc_reset_peripheral(lpi2c_pcc_clks[i2c_num - 4] & 0xff,
				     lpi2c_pcc_clks[i2c_num - 4] >> 8, false);
	} else {
		pcc_clock_enable(lpi2c_pcc_clks[i2c_num - 4] & 0xff,
				 lpi2c_pcc_clks[i2c_num - 4] >> 8, false);
	}
	return 0;
}

u32 imx_get_i2cclk(u32 i2c_num)
{
	const u32 lpi2c_pcc_clks[] = {
		LPI2C4_PCC3_SLOT << 8 | 3,
		LPI2C5_PCC3_SLOT << 8 | 3,
		LPI2C6_PCC4_SLOT << 8 | 4,
		LPI2C7_PCC4_SLOT << 8 | 4,
	};

	if (i2c_num == 0)
		return 24000000;

	if (i2c_num < 4 || i2c_num > 7)
		return 0;

	return pcc_clock_get_rate(lpi2c_pcc_clks[i2c_num - 4] & 0xff,
				  lpi2c_pcc_clks[i2c_num - 4] >> 8);
}
#endif

#if IS_ENABLED(CONFIG_SYS_I2C_IMX_I3C)
int enable_i3c_clk(unsigned char enable, u32 i3c_num)
{
	if (enable) {
		pcc_clock_enable(3, I3C2_PCC3_SLOT, false);
		pcc_clock_sel(3, I3C2_PCC3_SLOT, SOSC_DIV2);
		pcc_clock_enable(3, I3C2_PCC3_SLOT, true);
		pcc_reset_peripheral(3, I3C2_PCC3_SLOT, false);
	} else {
		pcc_clock_enable(3, I3C2_PCC3_SLOT, false);
	}
	return 0;
}

u32 imx_get_i3cclk(u32 i3c_num)
{
	return pcc_clock_get_rate(3, I3C2_PCC3_SLOT);
}
#endif

void enable_usboh3_clk(unsigned char enable)
{
	if (enable) {
		pcc_clock_enable(4, USB0_PCC4_SLOT, true);
		pcc_clock_enable(4, USBPHY_PCC4_SLOT, true);
		pcc_reset_peripheral(4, USB0_PCC4_SLOT, false);
		pcc_reset_peripheral(4, USBPHY_PCC4_SLOT, false);

#ifdef CONFIG_USB_MAX_CONTROLLER_COUNT
		if (CONFIG_USB_MAX_CONTROLLER_COUNT > 1) {
			pcc_clock_enable(4, USB1_PCC4_SLOT, true);
			pcc_clock_enable(4, USB1PHY_PCC4_SLOT, true);
			pcc_reset_peripheral(4, USB1_PCC4_SLOT, false);
			pcc_reset_peripheral(4, USB1PHY_PCC4_SLOT, false);
		}
#endif

		pcc_clock_enable(4, USB_XBAR_PCC4_SLOT, true);
	} else {
		pcc_clock_enable(4, USB0_PCC4_SLOT, false);
		pcc_clock_enable(4, USB1_PCC4_SLOT, false);
		pcc_clock_enable(4, USBPHY_PCC4_SLOT, false);
		pcc_clock_enable(4, USB1PHY_PCC4_SLOT, false);
		pcc_clock_enable(4, USB_XBAR_PCC4_SLOT, false);
	}
}

int enable_usb_pll(ulong usb_phy_base)
{
	u32 sosc_rate;
	s32 timeout = 1000000;

	struct usbphy_regs *usbphy =
		(struct usbphy_regs *)usb_phy_base;

	sosc_rate = cgc1_sosc_div(SOSC);
	if (!sosc_rate)
		return -EPERM;

	if (!(readl(&usbphy->usb1_pll_480_ctrl) & PLL_USB_LOCK_MASK)) {
		writel(0x1c00000, &usbphy->usb1_pll_480_ctrl_clr);

		switch (sosc_rate) {
		case 24000000:
			writel(0xc00000, &usbphy->usb1_pll_480_ctrl_set);
			break;

		case 30000000:
			writel(0x800000, &usbphy->usb1_pll_480_ctrl_set);
			break;

		case 19200000:
			writel(0x1400000, &usbphy->usb1_pll_480_ctrl_set);
			break;

		default:
			writel(0xc00000, &usbphy->usb1_pll_480_ctrl_set);
			break;
		}

		/* Enable the regulator first */
		writel(PLL_USB_REG_ENABLE_MASK,
		       &usbphy->usb1_pll_480_ctrl_set);

		/* Wait at least 15us */
		udelay(15);

		/* Enable the power */
		writel(PLL_USB_PWR_MASK, &usbphy->usb1_pll_480_ctrl_set);

		/* Wait lock */
		while (timeout--) {
			if (readl(&usbphy->usb1_pll_480_ctrl) &
			    PLL_USB_LOCK_MASK)
				break;
		}

		if (timeout <= 0) {
			/* If timeout, we power down the pll */
			writel(PLL_USB_PWR_MASK,
			       &usbphy->usb1_pll_480_ctrl_clr);
			return -ETIME;
		}
	}

	/* Clear the bypass */
	writel(PLL_USB_BYPASS_MASK, &usbphy->usb1_pll_480_ctrl_clr);

	/* Enable the PLL clock out to USB */
	writel((PLL_USB_EN_USB_CLKS_MASK | PLL_USB_ENABLE_MASK),
	       &usbphy->usb1_pll_480_ctrl_set);

	return 0;
}

void enable_mipi_dsi_clk(unsigned char enable)
{
	if (enable) {
		pcc_clock_enable(5, DSI_PCC5_SLOT, false);
		pcc_reset_peripheral(5, DSI_PCC5_SLOT, true);
		pcc_clock_sel(5, DSI_PCC5_SLOT, PLL4_PFD3_DIV2);
		pcc_clock_div_config(5, DSI_PCC5_SLOT, 0, 6);
		pcc_clock_enable(5, DSI_PCC5_SLOT, true);
		pcc_reset_peripheral(5, DSI_PCC5_SLOT, false);
	} else {
		pcc_clock_enable(5, DSI_PCC5_SLOT, false);
		pcc_reset_peripheral(5, DSI_PCC5_SLOT, true);
	}
}

void enable_adc1_clk(bool enable)
{
	if (enable) {
		pcc_clock_enable(1, ADC1_PCC1_SLOT, false);
		pcc_clock_sel(1, ADC1_PCC1_SLOT, CM33_BUSCLK);
		pcc_clock_enable(1, ADC1_PCC1_SLOT, true);
		pcc_reset_peripheral(1, ADC1_PCC1_SLOT, false);
	} else {
		pcc_clock_enable(1, ADC1_PCC1_SLOT, false);
	}
}

void reset_lcdclk(void)
{
	/* Disable clock and reset dcnano*/
	pcc_clock_enable(5, DCNANO_PCC5_SLOT, false);
	pcc_reset_peripheral(5, DCNANO_PCC5_SLOT, true);
}

/* PLL4 PFD0 max frequency */
#define PLL4_PFD0_MAX_RATE 600000 /*khz*/
void mxs_set_lcdclk(u32 base_addr, u32 freq_in_khz)
{
	u8 pcd, best_pcd = 0;
	u32 frac, rate, parent_rate, pfd, div;
	u32 best_pfd = 0, best_frac = 0, best = 0, best_div = 0;
	u32 pll4_rate;

	pcc_clock_enable(5, DCNANO_PCC5_SLOT, false);

	pll4_rate = cgc_clk_get_rate(PLL4);
	pll4_rate = pll4_rate / 1000;  /* Change to khz*/

	debug("PLL4 rate %ukhz\n", pll4_rate);

	for (pfd = 12; pfd <= 35; pfd++) {
		for (div = 1; div <= 64; div++) {
			parent_rate = pll4_rate;
			parent_rate = parent_rate * 18 / pfd;
			if (parent_rate > PLL4_PFD0_MAX_RATE)
				continue;

			parent_rate = parent_rate / div;

			for (pcd = 0; pcd < 8; pcd++) {
				for (frac = 0; frac < 2; frac++) {
					if (pcd == 0 && frac == 1)
						continue;

					rate = parent_rate * (frac + 1) / (pcd + 1);
					if (rate > freq_in_khz)
						continue;

					if (best == 0 || rate > best) {
						best = rate;
						best_pfd = pfd;
						best_frac = frac;
						best_pcd = pcd;
						best_div = div;
					}
				}
			}
		}
	}

	if (best == 0) {
		printf("Can't find parent clock for LCDIF, target freq: %u\n", freq_in_khz);
		return;
	}

	debug("LCD target rate %ukhz, best rate %ukhz, frac %u, pcd %u, best_pfd %u, best_div %u\n",
	      freq_in_khz, best, best_frac, best_pcd, best_pfd, best_div);

	cgc2_pll4_pfd_config(PLL4_PFD0, best_pfd);
	cgc2_pll4_pfddiv_config(PLL4_PFD0_DIV1, best_div - 1);

	pcc_clock_sel(5, DCNANO_PCC5_SLOT, PLL4_PFD0_DIV1);
	pcc_clock_div_config(5, DCNANO_PCC5_SLOT, best_frac, best_pcd + 1);
	pcc_clock_enable(5, DCNANO_PCC5_SLOT, true);
	pcc_reset_peripheral(5, DCNANO_PCC5_SLOT, false);
}

u32 mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ESDHC_CLK:
		return pcc_clock_get_rate(4, SDHC0_PCC4_SLOT);
	case MXC_ESDHC2_CLK:
		return pcc_clock_get_rate(4, SDHC1_PCC4_SLOT);
	case MXC_ESDHC3_CLK:
		return pcc_clock_get_rate(4, SDHC2_PCC4_SLOT);
	case MXC_ARM_CLK:
		return cgc_clk_get_rate(PLL2);
	default:
		return 0;
	}
}

u32 get_lpuart_clk(void)
{
	int index = 0;

	const u32 lpuart_array[] = {
		LPUART4_RBASE,
		LPUART5_RBASE,
		LPUART6_RBASE,
		LPUART7_RBASE,
	};

	const u32 lpuart_pcc_slots[] = {
		LPUART4_PCC3_SLOT,
		LPUART5_PCC3_SLOT,
		LPUART6_PCC4_SLOT,
		LPUART7_PCC4_SLOT,
	};

	const u32 lpuart_pcc[] = {
		3, 3, 4, 4,
	};

	for (index = 0; index < 4; index++) {
		if (lpuart_array[index] == LPUART_BASE)
			break;
	}

	if (index > 3)
		return 0;

	return pcc_clock_get_rate(lpuart_pcc[index], lpuart_pcc_slots[index]);
}

#ifndef CONFIG_SPL_BUILD
/*
 * Dump some core clockes.
 */
int do_mx8ulp_showclocks(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	printf("SDHC0 %8d MHz\n", pcc_clock_get_rate(4, SDHC0_PCC4_SLOT) / 1000000);
	printf("SDHC1 %8d MHz\n", pcc_clock_get_rate(4, SDHC1_PCC4_SLOT) / 1000000);
	printf("SDHC2 %8d MHz\n", pcc_clock_get_rate(4, SDHC2_PCC4_SLOT) / 1000000);

	printf("SOSC %8d MHz\n", cgc_clk_get_rate(SOSC) / 1000000);
	printf("FRO %8d MHz\n", cgc_clk_get_rate(FRO) / 1000000);
	printf("PLL2 %8d MHz\n", cgc_clk_get_rate(PLL2) / 1000000);
	printf("PLL3 %8d MHz\n", cgc_clk_get_rate(PLL3) / 1000000);
	printf("PLL3_VCODIV %8d MHz\n", cgc_clk_get_rate(PLL3_VCODIV) / 1000000);
	printf("PLL3_PFD0 %8d MHz\n", cgc_clk_get_rate(PLL3_PFD0) / 1000000);
	printf("PLL3_PFD1 %8d MHz\n", cgc_clk_get_rate(PLL3_PFD1) / 1000000);
	printf("PLL3_PFD2 %8d MHz\n", cgc_clk_get_rate(PLL3_PFD2) / 1000000);
	printf("PLL3_PFD3 %8d MHz\n", cgc_clk_get_rate(PLL3_PFD3) / 1000000);

	printf("PLL4_PFD0 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD0) / 1000000);
	printf("PLL4_PFD1 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD1) / 1000000);
	printf("PLL4_PFD2 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD2) / 1000000);
	printf("PLL4_PFD3 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD3) / 1000000);

	printf("PLL4_PFD0_DIV1 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD0_DIV1) / 1000000);
	printf("PLL4_PFD0_DIV2 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD0_DIV2) / 1000000);
	printf("PLL4_PFD1_DIV1 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD1_DIV1) / 1000000);
	printf("PLL4_PFD1_DIV2 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD1_DIV2) / 1000000);

	printf("PLL4_PFD2_DIV1 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD2_DIV1) / 1000000);
	printf("PLL4_PFD2_DIV2 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD2_DIV2) / 1000000);
	printf("PLL4_PFD3_DIV1 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD3_DIV1) / 1000000);
	printf("PLL4_PFD3_DIV2 %8d MHz\n", cgc_clk_get_rate(PLL4_PFD3_DIV2) / 1000000);

	printf("LPAV_AXICLK %8d MHz\n", cgc_clk_get_rate(LPAV_AXICLK) / 1000000);
	printf("LPAV_AHBCLK %8d MHz\n", cgc_clk_get_rate(LPAV_AHBCLK) / 1000000);
	printf("LPAV_BUSCLK %8d MHz\n", cgc_clk_get_rate(LPAV_BUSCLK) / 1000000);
	printf("NIC_APCLK %8d MHz\n", cgc_clk_get_rate(NIC_APCLK) / 1000000);

	printf("NIC_PERCLK %8d MHz\n", cgc_clk_get_rate(NIC_PERCLK) / 1000000);
	printf("XBAR_APCLK %8d MHz\n", cgc_clk_get_rate(XBAR_APCLK) / 1000000);
	printf("XBAR_BUSCLK %8d MHz\n", cgc_clk_get_rate(XBAR_BUSCLK) / 1000000);
	printf("AD_SLOWCLK %8d MHz\n", cgc_clk_get_rate(AD_SLOWCLK) / 1000000);
	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx8ulp_showclocks,
	"display clocks",
	""
);
#endif
