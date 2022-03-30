// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/clock.h>
#include <asm/arch/mc.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <power/as3722.h>
#include <power/pmic.h>
#include "pinmux-config-nyan-big.h"

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	gpio_config_table(nyan_big_gpio_inits,
			  ARRAY_SIZE(nyan_big_gpio_inits));

	pinmux_config_pingrp_table(nyan_big_pingrps,
				   ARRAY_SIZE(nyan_big_pingrps));

	pinmux_config_drvgrp_table(nyan_big_drvgrps,
				   ARRAY_SIZE(nyan_big_drvgrps));
}

int tegra_board_id(void)
{
	static const int vector[] = {TEGRA_GPIO(Q, 3), TEGRA_GPIO(T, 1),
					TEGRA_GPIO(X, 1), TEGRA_GPIO(X, 4),
					-1};

	gpio_claim_vector(vector, "board_id%d");
	return gpio_get_values_as_int(vector);
}

int tegra_lcd_pmic_init(int board_id)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_as3722), &dev);
	if (ret) {
		debug("%s: Failed to find PMIC\n", __func__);
		return ret;
	}

	if (board_id == 0)
		pmic_reg_write(dev, 0x00, 0x3c);
	else
		pmic_reg_write(dev, 0x00, 0x50);
	pmic_reg_write(dev, 0x12, 0x10);
	pmic_reg_write(dev, 0x0c, 0x07);
	pmic_reg_write(dev, 0x20, 0x10);

	return 0;
}

/* Setup required information for Linux kernel */
static void setup_kernel_info(void)
{
	struct mc_ctlr *mc = (void *)NV_PA_MC_BASE;

	/* The kernel graphics driver needs this region locked down */
	writel(0, &mc->mc_video_protect_bom);
	writel(0, &mc->mc_video_protect_size_mb);
	writel(1, &mc->mc_video_protect_reg_ctrl);
}

/*
 * We need to take ALL audio devices conntected to AHUB (AUDIO, APBIF,
 * I2S, DAM, AMX, ADX, SPDIF, AFC) out of reset and enable the clocks.
 * Otherwise reading AHUB devices will hang when the kernel boots.
 */
static void enable_required_clocks(void)
{
	static enum periph_id ids[] = {
		PERIPH_ID_I2S0,
		PERIPH_ID_I2S1,
		PERIPH_ID_I2S2,
		PERIPH_ID_I2S3,
		PERIPH_ID_I2S4,
		PERIPH_ID_AUDIO,
		PERIPH_ID_APBIF,
		PERIPH_ID_DAM0,
		PERIPH_ID_DAM1,
		PERIPH_ID_DAM2,
		PERIPH_ID_AMX0,
		PERIPH_ID_AMX1,
		PERIPH_ID_ADX0,
		PERIPH_ID_ADX1,
		PERIPH_ID_SPDIF,
		PERIPH_ID_AFC0,
		PERIPH_ID_AFC1,
		PERIPH_ID_AFC2,
		PERIPH_ID_AFC3,
		PERIPH_ID_AFC4,
		PERIPH_ID_AFC5,
		PERIPH_ID_EXTPERIPH1
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(ids); i++)
		clock_enable(ids[i]);
	udelay(2);
	for (i = 0; i < ARRAY_SIZE(ids); i++)
		reset_set_enable(ids[i], 0);
}

int nvidia_board_init(void)
{
	clock_start_periph_pll(PERIPH_ID_EXTPERIPH1, CLOCK_ID_OSC, 12000000);
	clock_start_periph_pll(PERIPH_ID_I2S1, CLOCK_ID_CLK_M, 1500000);

	/* For external MAX98090 audio codec */
	clock_external_output(1);
	setup_kernel_info();
	enable_required_clocks();

	return 0;
}
