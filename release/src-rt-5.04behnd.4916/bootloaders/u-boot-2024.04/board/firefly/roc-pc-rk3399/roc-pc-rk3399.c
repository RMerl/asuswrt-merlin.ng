// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <log.h>
#include <spl_gpio.h>
#include <asm/io.h>
#include <power/regulator.h>

#include <asm/arch-rockchip/cru.h>
#include <asm/arch-rockchip/gpio.h>
#include <asm/arch-rockchip/grf_rk3399.h>

#ifndef CONFIG_SPL_BUILD
int board_early_init_f(void)
{
	struct udevice *regulator;
	int ret;

	ret = regulator_get_by_platname("vcc5v0_host", &regulator);
	if (ret) {
		debug("%s vcc5v0_host init fail! ret %d\n", __func__, ret);
		goto out;
	}

	ret = regulator_set_enable(regulator, true);
	if (ret)
		debug("%s vcc5v0-host-en set fail! ret %d\n", __func__, ret);
out:
	return 0;
}

#else

#define PMUGRF_BASE	0xff320000
#define GPIO0_BASE	0xff720000

/**
 * LED setup for roc-rk3399-pc
 *
 * 1. Set the low power leds (only during POR, pwr_key env is 'y')
 *    glow yellow LED, termed as low power
 *    poll for on board power key press
 *    once powe key pressed, turn off yellow
 * 2. Turn on red LED, indicating full power mode
 */
void led_setup(void)
{
	struct rockchip_gpio_regs * const gpio0 = (void *)GPIO0_BASE;
	struct rk3399_pmugrf_regs * const pmugrf = (void *)PMUGRF_BASE;
	bool press_pwr_key = false;

	if (IS_ENABLED(CONFIG_SPL_ENV_SUPPORT)) {
		env_init();
		env_load();
		if (env_get_yesno("pwr_key") == 1)
			press_pwr_key = true;
	}

	if (press_pwr_key && !strcmp(get_reset_cause(), "POR")) {
		spl_gpio_output(gpio0, GPIO(BANK_A, 2), 1);

		spl_gpio_set_pull(&pmugrf->gpio0_p, GPIO(BANK_A, 5),
				  GPIO_PULL_NORMAL);
		while (readl(&gpio0->ext_port) & 0x20)
			;

		spl_gpio_output(gpio0, GPIO(BANK_A, 2), 0);
	}

	spl_gpio_output(gpio0, GPIO(BANK_B, 5), 1);
}

#endif
