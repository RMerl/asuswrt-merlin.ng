// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2014-2018
 *  Marcel Ziswiler <marcel@ziswiler.com>
 */

#include <common.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/tegra.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <i2c.h>
#include <pci_tegra.h>
#include "../common/tdx-common.h"

#include "pinmux-config-apalis_t30.h"

DECLARE_GLOBAL_DATA_PTR;

#define PMU_I2C_ADDRESS		0x2D
#define MAX_I2C_RETRY		3

#ifdef CONFIG_APALIS_T30_PCIE_EVALBOARD_INIT
#define PEX_PERST_N	TEGRA_GPIO(S, 7) /* Apalis GPIO7 */
#define RESET_MOCI_CTRL	TEGRA_GPIO(I, 4)

static int pci_reset_status;
#endif /* CONFIG_APALIS_T30_PCIE_EVALBOARD_INIT */

int arch_misc_init(void)
{
	if (readl(NV_PA_BASE_SRAM + NVBOOTINFOTABLE_BOOTTYPE) ==
	    NVBOOTTYPE_RECOVERY)
		printf("USB recovery mode\n");

	return 0;
}

int checkboard(void)
{
	printf("Model: Toradex Apalis T30 %dGB\n",
	       (gd->ram_size == 0x40000000) ? 1 : 2);

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
	pinmux_config_pingrp_table(tegra3_pinmux_common,
				   ARRAY_SIZE(tegra3_pinmux_common));

	pinmux_config_pingrp_table(unused_pins_lowpower,
				   ARRAY_SIZE(unused_pins_lowpower));

	/* Initialize any non-default pad configs (APB_MISC_GP regs) */
	pinmux_config_drvgrp_table(apalis_t30_padctrl,
				   ARRAY_SIZE(apalis_t30_padctrl));
}

#ifdef CONFIG_PCI_TEGRA
int tegra_pcie_board_init(void)
{
	struct udevice *dev;
	u8 addr, data[1];
	int err;

	err = i2c_get_chip_for_busnum(0, PMU_I2C_ADDRESS, 1, &dev);
	if (err) {
		debug("%s: Cannot find PMIC I2C chip\n", __func__);
		return err;
	}

	/* TPS659110: VDD2_OP_REG = 1.05V */
	data[0] = 0x27;
	addr = 0x25;

	err = dm_i2c_write(dev, addr, data, 1);
	if (err) {
		debug("failed to set VDD supply\n");
		return err;
	}

	/* TPS659110: VDD2_REG 7.5 mV/us, ACTIVE */
	data[0] = 0x0D;
	addr = 0x24;

	err = dm_i2c_write(dev, addr, data, 1);
	if (err) {
		debug("failed to enable VDD supply\n");
		return err;
	}

	/* TPS659110: LDO6_REG = 1.1V, ACTIVE */
	data[0] = 0x0D;
	addr = 0x35;

	err = dm_i2c_write(dev, addr, data, 1);
	if (err) {
		debug("failed to set AVDD supply\n");
		return err;
	}

#ifdef CONFIG_APALIS_T30_PCIE_EVALBOARD_INIT
	gpio_request(PEX_PERST_N, "PEX_PERST_N");
	gpio_request(RESET_MOCI_CTRL, "RESET_MOCI_CTRL");
#endif /* CONFIG_APALIS_T30_PCIE_EVALBOARD_INIT */

	return 0;
}

void tegra_pcie_board_port_reset(struct tegra_pcie_port *port)
{
	int index = tegra_pcie_port_index_of_port(port);

	if (index == 2) { /* I210 Gigabit Ethernet Controller (On-module) */
		tegra_pcie_port_reset(port);
	}
#ifdef CONFIG_APALIS_T30_PCIE_EVALBOARD_INIT
	/*
	 * Apalis PCIe aka port 1 and Apalis Type Specific 4 Lane PCIe aka port
	 * 0 share the same RESET_MOCI therefore only assert it once for both
	 * ports to avoid losing the previously brought up port again.
	 */
	else if ((index == 1) || (index == 0)) {
		/* only do it once per init cycle */
		if (pci_reset_status % 2 == 0) {
			/*
			 * Reset PLX PEX 8605 PCIe Switch plus PCIe devices on
			 * Apalis Evaluation Board
			 */
			gpio_direction_output(PEX_PERST_N, 0);
			gpio_direction_output(RESET_MOCI_CTRL, 0);

			/*
			 * Must be asserted for 100 ms after power and clocks
			 * are stable
			 */
			mdelay(100);

			gpio_set_value(PEX_PERST_N, 1);
			/*
			 * Err_5: PEX_REFCLK_OUTpx/nx Clock Outputs is not
			 * Guaranteed Until 900 us After PEX_PERST# De-assertion
			 */
			mdelay(1);
			gpio_set_value(RESET_MOCI_CTRL, 1);
		}
		pci_reset_status++;
	}
#endif /* CONFIG_APALIS_T30_PCIE_EVALBOARD_INIT */
}
#endif /* CONFIG_PCI_TEGRA */

/*
 * Backlight off before OS handover
 */
void board_preboot_os(void)
{
	gpio_request(TEGRA_GPIO(V, 2), "BKL1_ON");
	gpio_direction_output(TEGRA_GPIO(V, 2), 0);
}
