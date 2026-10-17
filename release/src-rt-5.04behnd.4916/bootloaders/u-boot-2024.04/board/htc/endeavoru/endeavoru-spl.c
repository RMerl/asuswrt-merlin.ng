// SPDX-License-Identifier: GPL-2.0+
/*
 *  T30 HTC Endeavoru SPL stage configuration
 *
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2022
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/tegra_i2c.h>
#include <linux/delay.h>

/*
 * Endeavoru uses TPS80032 PMIC with SMPS1 and SMPS2 in strandard
 * mode with zero offset.
 */

#define TPS80032_DVS_I2C_ADDR			(0x12 << 1)
#define TPS80032_SMPS1_CFG_VOLTAGE_REG		0x56
#define TPS80032_SMPS2_CFG_VOLTAGE_REG		0x5C
#define TPS80032_SMPS1_CFG_VOLTAGE_DATA		(0x2100 | TPS80032_SMPS1_CFG_VOLTAGE_REG)
#define TPS80032_SMPS2_CFG_VOLTAGE_DATA		(0x3000 | TPS80032_SMPS2_CFG_VOLTAGE_REG)

#define TPS80032_CTL1_I2C_ADDR			(0x48 << 1)
#define TPS80032_SMPS1_CFG_STATE_REG		0x54
#define TPS80032_SMPS2_CFG_STATE_REG		0x5A
#define TPS80032_SMPS1_CFG_STATE_DATA		(0x0100 | TPS80032_SMPS1_CFG_STATE_REG)
#define TPS80032_SMPS2_CFG_STATE_DATA		(0x0100 | TPS80032_SMPS2_CFG_STATE_REG)

#define TEGRA_GPIO_PS0				144

void pmic_enable_cpu_vdd(void)
{
	/* Set VDD_CORE to 1.200V. */
	tegra_i2c_ll_write(TPS80032_DVS_I2C_ADDR, TPS80032_SMPS2_CFG_VOLTAGE_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS80032_CTL1_I2C_ADDR, TPS80032_SMPS2_CFG_STATE_DATA);

	udelay(1000);

	/* Bring up VDD_CPU to 1.0125V. */
	tegra_i2c_ll_write(TPS80032_DVS_I2C_ADDR, TPS80032_SMPS1_CFG_VOLTAGE_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS80032_CTL1_I2C_ADDR, TPS80032_SMPS1_CFG_STATE_DATA);
	udelay(10 * 1000);
}

/*
 * Unlike all other supported Tegra devices and most known Tegra devices, the
 * HTC One X has no hardware way to enter APX/RCM mode, which may lead to a
 * dangerous situation when, if BCT is set correctly and the bootloader is
 * faulty, the device will hang in a permanent brick state. Exiting from this
 * state can be done only by disassembling the device and shortening testpad
 * to the ground.
 *
 * To prevent this or to minimize the probability of such an accident, it was
 * proposed to add the RCM rebooting hook as early into SPL as possible since
 * SPL is much more robust and has minimal changes that can break bootflow.
 *
 * gpio_early_init_uart() function was chosen as it is the earliest function
 * exposed for setup by the device. Hook performs a check for volume up
 * button state and triggers RCM if it is pressed.
 */
void gpio_early_init_uart(void)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(TEGRA_GPIO_PS0)];
	u32 value;

	/* Configure pinmux */
	pinmux_set_func(PMUX_PINGRP_KB_ROW8_PS0, PMUX_FUNC_KBC);
	pinmux_set_pullupdown(PMUX_PINGRP_KB_ROW8_PS0, PMUX_PULL_UP);
	pinmux_tristate_disable(PMUX_PINGRP_KB_ROW8_PS0);
	pinmux_set_io(PMUX_PINGRP_KB_ROW8_PS0, PMUX_PIN_INPUT);

	/* Configure GPIO direction as input. */
	value = readl(&bank->gpio_dir_out[GPIO_PORT(TEGRA_GPIO_PS0)]);
	value &= ~(1 << GPIO_BIT(TEGRA_GPIO_PS0));
	writel(value, &bank->gpio_dir_out[GPIO_PORT(TEGRA_GPIO_PS0)]);

	/* Enable the pin as a GPIO */
	value = readl(&bank->gpio_config[GPIO_PORT(TEGRA_GPIO_PS0)]);
	value |= 1 << GPIO_BIT(TEGRA_GPIO_PS0);
	writel(value, &bank->gpio_config[GPIO_PORT(TEGRA_GPIO_PS0)]);

	/* Get GPIO value */
	value = readl(&bank->gpio_in[GPIO_PORT(TEGRA_GPIO_PS0)]);
	value = (value >> GPIO_BIT(TEGRA_GPIO_PS0)) & 1;

	/* Enter RCM if button is pressed */
	if (!value) {
		tegra_pmc_writel(2, PMC_SCRATCH0);
		tegra_pmc_writel(PMC_CNTRL_MAIN_RST, PMC_CNTRL);
	}
}
