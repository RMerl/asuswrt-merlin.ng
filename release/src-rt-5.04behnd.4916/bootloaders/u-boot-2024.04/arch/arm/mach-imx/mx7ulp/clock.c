// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <clock_legacy.h>
#include <command.h>
#include <div64.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC_IMX
#if CFG_SYS_FSL_ESDHC_ADDR == USDHC0_RBASE
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
#elif CFG_SYS_FSL_ESDHC_ADDR == USDHC1_RBASE
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
#endif
#endif
	return 0;
}

static u32 get_fast_plat_clk(void)
{
	return scg_clk_get_rate(SCG_NIC0_CLK);
}

static u32 get_slow_plat_clk(void)
{
	return scg_clk_get_rate(SCG_NIC1_CLK);
}

static u32 get_ipg_clk(void)
{
	return scg_clk_get_rate(SCG_NIC1_BUS_CLK);
}

u32 get_lpuart_clk(void)
{
	int index = 0;

	const u32 lpuart_array[] = {
		LPUART0_RBASE,
		LPUART1_RBASE,
		LPUART2_RBASE,
		LPUART3_RBASE,
		LPUART4_RBASE,
		LPUART5_RBASE,
		LPUART6_RBASE,
		LPUART7_RBASE,
	};

	const enum pcc_clk lpuart_pcc_clks[] = {
		PER_CLK_LPUART4,
		PER_CLK_LPUART5,
		PER_CLK_LPUART6,
		PER_CLK_LPUART7,
	};

	for (index = 0; index < 8; index++) {
		if (lpuart_array[index] == LPUART_BASE)
			break;
	}

	if (index < 4 || index > 7)
		return 0;

	return pcc_clock_get_rate(lpuart_pcc_clks[index - 4]);
}

#ifdef CONFIG_SYS_I2C_IMX_LPI2C
int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	/* Set parent to FIRC DIV2 clock */
	const enum pcc_clk lpi2c_pcc_clks[] = {
		PER_CLK_LPI2C4,
		PER_CLK_LPI2C5,
		PER_CLK_LPI2C6,
		PER_CLK_LPI2C7,
	};

	if (i2c_num < 4 || i2c_num > 7)
		return -EINVAL;

	if (enable) {
		pcc_clock_enable(lpi2c_pcc_clks[i2c_num - 4], false);
		pcc_clock_sel(lpi2c_pcc_clks[i2c_num - 4], SCG_FIRC_DIV2_CLK);
		pcc_clock_enable(lpi2c_pcc_clks[i2c_num - 4], true);
	} else {
		pcc_clock_enable(lpi2c_pcc_clks[i2c_num - 4], false);
	}
	return 0;
}

u32 imx_get_i2cclk(unsigned i2c_num)
{
	const enum pcc_clk lpi2c_pcc_clks[] = {
		PER_CLK_LPI2C4,
		PER_CLK_LPI2C5,
		PER_CLK_LPI2C6,
		PER_CLK_LPI2C7,
	};

	if (i2c_num < 4 || i2c_num > 7)
		return 0;

	return pcc_clock_get_rate(lpi2c_pcc_clks[i2c_num - 4]);
}
#endif

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return scg_clk_get_rate(SCG_CORE_CLK);
	case MXC_AXI_CLK:
		return get_fast_plat_clk();
	case MXC_AHB_CLK:
		return get_slow_plat_clk();
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_I2C_CLK:
		return pcc_clock_get_rate(PER_CLK_LPI2C4);
	case MXC_UART_CLK:
		return get_lpuart_clk();
	case MXC_ESDHC_CLK:
		return pcc_clock_get_rate(PER_CLK_USDHC0);
	case MXC_ESDHC2_CLK:
		return pcc_clock_get_rate(PER_CLK_USDHC1);
	case MXC_DDR_CLK:
		return scg_clk_get_rate(SCG_DDR_CLK);
	default:
		printf("Unsupported mxc_clock %d\n", clk);
		break;
	}

	return 0;
}

void init_clk_usdhc(u32 index)
{
	switch (index) {
	case 0:
		/*Disable the clock before configure it */
		pcc_clock_enable(PER_CLK_USDHC0, false);

		/* 158MHz / 1 = 158MHz */
		pcc_clock_sel(PER_CLK_USDHC0, SCG_NIC1_CLK);
		pcc_clock_div_config(PER_CLK_USDHC0, false, 1);
		pcc_clock_enable(PER_CLK_USDHC0, true);
		break;
	case 1:
		/*Disable the clock before configure it */
		pcc_clock_enable(PER_CLK_USDHC1, false);

		/* 158MHz / 1 = 158MHz */
		pcc_clock_sel(PER_CLK_USDHC1, SCG_NIC1_CLK);
		pcc_clock_div_config(PER_CLK_USDHC1, false, 1);
		pcc_clock_enable(PER_CLK_USDHC1, true);
		break;
	default:
		printf("Invalid index for USDHC %d\n", index);
		break;
	}
}

#ifdef CONFIG_MXC_OCOTP

#define OCOTP_CTRL_PCC1_SLOT		(38)
#define OCOTP_CTRL_HIGH4K_PCC1_SLOT	(39)

void enable_ocotp_clk(unsigned char enable)
{
	u32 val;

	/*
	 * Seems the OCOTP CLOCKs have been enabled at default,
	 * check its inuse flag
	 */

	val = readl(PCC1_RBASE + 4 * OCOTP_CTRL_PCC1_SLOT);
	if (!(val & PCC_INUSE_MASK))
		writel(PCC_CGC_MASK, (PCC1_RBASE + 4 * OCOTP_CTRL_PCC1_SLOT));

	val = readl(PCC1_RBASE + 4 * OCOTP_CTRL_HIGH4K_PCC1_SLOT);
	if (!(val & PCC_INUSE_MASK))
		writel(PCC_CGC_MASK,
		       (PCC1_RBASE + 4 * OCOTP_CTRL_HIGH4K_PCC1_SLOT));
}
#endif

void enable_usboh3_clk(unsigned char enable)
{
	if (enable) {
		pcc_clock_enable(PER_CLK_USB0, false);
		pcc_clock_sel(PER_CLK_USB0, SCG_NIC1_BUS_CLK);
		pcc_clock_enable(PER_CLK_USB0, true);

#ifdef CONFIG_USB_MAX_CONTROLLER_COUNT
		if (CONFIG_USB_MAX_CONTROLLER_COUNT > 1) {
			pcc_clock_enable(PER_CLK_USB1, false);
			pcc_clock_sel(PER_CLK_USB1, SCG_NIC1_BUS_CLK);
			pcc_clock_enable(PER_CLK_USB1, true);
		}
#endif

		pcc_clock_enable(PER_CLK_USB_PHY, true);
		pcc_clock_enable(PER_CLK_USB_PL301, true);
	} else {
		pcc_clock_enable(PER_CLK_USB0, false);
		pcc_clock_enable(PER_CLK_USB1, false);
		pcc_clock_enable(PER_CLK_USB_PHY, false);
		pcc_clock_enable(PER_CLK_USB_PL301, false);
	}
}

static void lpuart_set_clk(uint32_t index, enum scg_clk clk)
{
	const enum pcc_clk lpuart_pcc_clks[] = {
		PER_CLK_LPUART4,
		PER_CLK_LPUART5,
		PER_CLK_LPUART6,
		PER_CLK_LPUART7,
	};

	if (index < 4 || index > 7)
		return;

#ifndef CONFIG_CLK_DEBUG
	pcc_clock_enable(lpuart_pcc_clks[index - 4], false);
#endif
	pcc_clock_sel(lpuart_pcc_clks[index - 4], clk);
	pcc_clock_enable(lpuart_pcc_clks[index - 4], true);
}

static void init_clk_lpuart(void)
{
	u32 index = 0, i;

	const u32 lpuart_array[] = {
		LPUART0_RBASE,
		LPUART1_RBASE,
		LPUART2_RBASE,
		LPUART3_RBASE,
		LPUART4_RBASE,
		LPUART5_RBASE,
		LPUART6_RBASE,
		LPUART7_RBASE,
	};

	for (i = 0; i < 8; i++) {
		if (lpuart_array[i] == LPUART_BASE) {
			index = i;
			break;
		}
	}

	lpuart_set_clk(index, SCG_SOSC_DIV2_CLK);
}

static void init_clk_rgpio2p(void)
{
	/*Enable RGPIO2P1 clock */
	pcc_clock_enable(PER_CLK_RGPIO2P1, true);

	/*
	 * Hard code to enable RGPIO2P0 clock since it is not
	 * in clock frame for A7 domain
	 */
	writel(PCC_CGC_MASK, (PCC0_RBASE + 0x3C));
}

/* Configure PLL/PFD freq */
void clock_init(void)
{
	/*
	 * ROM has enabled clocks:
	 * A4 side: SIRC 16Mhz (DIV1-3 off),  FIRC 48Mhz (DIV1-2 on),
	 *          Non-LP-boot:  SOSC, SPLL PFD0 (scs selected)
	 * A7 side:  SPLL PFD0 (scs selected, 413Mhz),
	 *           APLL PFD0 (352Mhz), DDRCLK, all NIC clocks
	 *           A7 Plat0 (NIC0) = 176Mhz, Plat1 (NIC1) = 176Mhz,
	 *           IP BUS (NIC1_BUS) = 58.6Mhz
	 *
	 * In u-boot:
	 * 1. Enable PFD1-3 of APLL for A7 side. Enable FIRC and DIVs.
	 * 2. Enable USB PLL
	 * 3. Init the clocks of peripherals used in u-boot bu
	 *    without set rate interface.The clocks for these
	 *    peripherals are enabled in this intialization.
	 * 4.Other peripherals with set clock rate interface
	 *   does not be set in this function.
	 */

	scg_a7_firc_init();

	scg_a7_soscdiv_init();

	scg_a7_init_core_clk();

	/* APLL PFD1 = 270Mhz, PFD2=345.6Mhz, PFD3=800Mhz */
	scg_enable_pll_pfd(SCG_APLL_PFD1_CLK, 35);
	scg_enable_pll_pfd(SCG_APLL_PFD2_CLK, 28);
	scg_enable_pll_pfd(SCG_APLL_PFD3_CLK, 12);

	init_clk_lpuart();

	init_clk_rgpio2p();

	enable_usboh3_clk(1);
}

#ifdef CONFIG_IMX_HAB
void hab_caam_clock_enable(unsigned char enable)
{
       if (enable)
	       pcc_clock_enable(PER_CLK_CAAM, true);
       else
	       pcc_clock_enable(PER_CLK_CAAM, false);
}
#endif

#ifndef CONFIG_SPL_BUILD
/*
 * Dump some core clockes.
 */
int do_mx7_showclocks(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{

	u32 freq;
	freq = decode_pll(PLL_A7_SPLL);
	printf("PLL_A7_SPLL    %8d MHz\n", freq / 1000000);

	freq = decode_pll(PLL_A7_APLL);
	printf("PLL_A7_APLL    %8d MHz\n", freq / 1000000);

	freq = decode_pll(PLL_USB);
	printf("PLL_USB        %8d MHz\n", freq / 1000000);

	printf("\n");

	printf("CORE       %8d kHz\n", scg_clk_get_rate(SCG_CORE_CLK) / 1000);
	printf("IPG        %8d kHz\n", mxc_get_clock(MXC_IPG_CLK) / 1000);
	printf("UART       %8d kHz\n", mxc_get_clock(MXC_UART_CLK) / 1000);
	printf("AHB        %8d kHz\n", mxc_get_clock(MXC_AHB_CLK) / 1000);
	printf("AXI        %8d kHz\n", mxc_get_clock(MXC_AXI_CLK) / 1000);
	printf("DDR        %8d kHz\n", mxc_get_clock(MXC_DDR_CLK) / 1000);
	printf("USDHC1     %8d kHz\n", mxc_get_clock(MXC_ESDHC_CLK) / 1000);
	printf("USDHC2     %8d kHz\n", mxc_get_clock(MXC_ESDHC2_CLK) / 1000);
	printf("I2C4       %8d kHz\n", mxc_get_clock(MXC_I2C_CLK) / 1000);

	scg_a7_info();

	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx7_showclocks,
	"display clocks",
	""
);
#endif
