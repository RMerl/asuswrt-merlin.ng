// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-2018 Toradex, Inc.
 */

#include <common.h>
#include <dm.h>
#include <asm/arch-tegra/ap.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <pci_tegra.h>
#include <power/as3722.h>
#include <power/pmic.h>

#include "../common/tdx-common.h"
#include "pinmux-config-apalis-tk1.h"

#define LAN_DEV_OFF_N	TEGRA_GPIO(O, 6)
#define LAN_RESET_N	TEGRA_GPIO(S, 2)
#define LAN_WAKE_N	TEGRA_GPIO(O, 5)
#ifdef CONFIG_APALIS_TK1_PCIE_EVALBOARD_INIT
#define PEX_PERST_N	TEGRA_GPIO(DD, 1) /* Apalis GPIO7 */
#define RESET_MOCI_CTRL	TEGRA_GPIO(U, 4)
#endif /* CONFIG_APALIS_TK1_PCIE_EVALBOARD_INIT */

int arch_misc_init(void)
{
	if (readl(NV_PA_BASE_SRAM + NVBOOTINFOTABLE_BOOTTYPE) ==
	    NVBOOTTYPE_RECOVERY)
		printf("USB recovery mode\n");

	return 0;
}

int checkboard(void)
{
	puts("Model: Toradex Apalis TK1 2GB\n");

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_clear_tristate_input_clamping();

	gpio_config_table(apalis_tk1_gpio_inits,
			  ARRAY_SIZE(apalis_tk1_gpio_inits));

	pinmux_config_pingrp_table(apalis_tk1_pingrps,
				   ARRAY_SIZE(apalis_tk1_pingrps));

	pinmux_config_drvgrp_table(apalis_tk1_drvgrps,
				   ARRAY_SIZE(apalis_tk1_drvgrps));
}

#ifdef CONFIG_PCI_TEGRA
/* TODO: Convert to driver model */
static int as3722_sd_enable(struct udevice *pmic, unsigned int sd)
{
	int err;

	if (sd > 6)
		return -EINVAL;

	err = pmic_clrsetbits(pmic, AS3722_SD_CONTROL, 0, 1 << sd);
	if (err) {
		pr_err("failed to update SD control register: %d", err);
		return err;
	}

	return 0;
}

/* TODO: Convert to driver model */
static int as3722_ldo_enable(struct udevice *pmic, unsigned int ldo)
{
	int err;
	u8 ctrl_reg = AS3722_LDO_CONTROL0;

	if (ldo > 11)
		return -EINVAL;

	if (ldo > 7) {
		ctrl_reg = AS3722_LDO_CONTROL1;
		ldo -= 8;
	}

	err = pmic_clrsetbits(pmic, ctrl_reg, 0, 1 << ldo);
	if (err) {
		pr_err("failed to update LDO control register: %d", err);
		return err;
	}

	return 0;
}

int tegra_pcie_board_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_as3722), &dev);
	if (ret) {
		pr_err("failed to find AS3722 PMIC: %d\n", ret);
		return ret;
	}

	ret = as3722_sd_enable(dev, 4);
	if (ret < 0) {
		pr_err("failed to enable SD4: %d\n", ret);
		return ret;
	}

	ret = as3722_sd_set_voltage(dev, 4, 0x24);
	if (ret < 0) {
		pr_err("failed to set SD4 voltage: %d\n", ret);
		return ret;
	}

	gpio_request(LAN_DEV_OFF_N, "LAN_DEV_OFF_N");
	gpio_request(LAN_RESET_N, "LAN_RESET_N");
	gpio_request(LAN_WAKE_N, "LAN_WAKE_N");

#ifdef CONFIG_APALIS_TK1_PCIE_EVALBOARD_INIT
	gpio_request(PEX_PERST_N, "PEX_PERST_N");
	gpio_request(RESET_MOCI_CTRL, "RESET_MOCI_CTRL");
#endif /* CONFIG_APALIS_TK1_PCIE_EVALBOARD_INIT */

	return 0;
}

void tegra_pcie_board_port_reset(struct tegra_pcie_port *port)
{
	int index = tegra_pcie_port_index_of_port(port);

	if (index == 1) { /* I210 Gigabit Ethernet Controller (On-module) */
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_PMIC,
						  DM_GET_DRIVER(pmic_as3722),
						  &dev);
		if (ret) {
			debug("%s: Failed to find PMIC\n", __func__);
			return;
		}

		/* Reset I210 Gigabit Ethernet Controller */
		gpio_direction_output(LAN_RESET_N, 0);

		/*
		 * Make sure we don't get any back feeding from DEV_OFF_N resp.
		 * LAN_WAKE_N
		 */
		gpio_direction_output(LAN_DEV_OFF_N, 0);
		gpio_direction_output(LAN_WAKE_N, 0);

		/* Make sure LDO9 and LDO10 are initially enabled @ 0V */
		ret = as3722_ldo_enable(dev, 9);
		if (ret < 0) {
			pr_err("failed to enable LDO9: %d\n", ret);
			return;
		}
		ret = as3722_ldo_enable(dev, 10);
		if (ret < 0) {
			pr_err("failed to enable LDO10: %d\n", ret);
			return;
		}
		ret = as3722_ldo_set_voltage(dev, 9, 0x80);
		if (ret < 0) {
			pr_err("failed to set LDO9 voltage: %d\n", ret);
			return;
		}
		ret = as3722_ldo_set_voltage(dev, 10, 0x80);
		if (ret < 0) {
			pr_err("failed to set LDO10 voltage: %d\n", ret);
			return;
		}

		/* Make sure controller gets enabled by disabling DEV_OFF_N */
		gpio_set_value(LAN_DEV_OFF_N, 1);

		/*
		 * Enable LDO9 and LDO10 for +V3.3_ETH on patched prototype
		 * V1.0A and sample V1.0B and newer modules
		 */
		ret = as3722_ldo_set_voltage(dev, 9, 0xff);
		if (ret < 0) {
			pr_err("failed to set LDO9 voltage: %d\n", ret);
			return;
		}
		ret = as3722_ldo_set_voltage(dev, 10, 0xff);
		if (ret < 0) {
			pr_err("failed to set LDO10 voltage: %d\n", ret);
			return;
		}

		/*
		 * Must be asserted for 100 ms after power and clocks are stable
		 */
		mdelay(100);

		gpio_set_value(LAN_RESET_N, 1);
	} else if (index == 0) { /* Apalis PCIe */
#ifdef CONFIG_APALIS_TK1_PCIE_EVALBOARD_INIT
		/*
		 * Reset PLX PEX 8605 PCIe Switch plus PCIe devices on Apalis
		 * Evaluation Board
		 */
		gpio_direction_output(PEX_PERST_N, 0);
		gpio_direction_output(RESET_MOCI_CTRL, 0);

		/*
		 * Must be asserted for 100 ms after power and clocks are stable
		 */
		mdelay(100);

		gpio_set_value(PEX_PERST_N, 1);
		/*
		 * Err_5: PEX_REFCLK_OUTpx/nx Clock Outputs is not Guaranteed
		 * Until 900 us After PEX_PERST# De-assertion
		 */
		mdelay(1);
		gpio_set_value(RESET_MOCI_CTRL, 1);
#endif /* CONFIG_APALIS_TK1_PCIE_EVALBOARD_INIT */
	}
}
#endif /* CONFIG_PCI_TEGRA */

/*
 * Backlight off before OS handover
 */
void board_preboot_os(void)
{
	gpio_request(TEGRA_GPIO(BB, 5), "BL_ON");
	gpio_direction_output(TEGRA_GPIO(BB, 5), 0);
}
