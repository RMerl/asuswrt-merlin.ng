// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 *
 * Copyright 2007-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/fsl_serdes.h>
#include <linux/errno.h>

#include "kmp204x.h"

#define PROM_SEL_L	11
/* control the PROM_SEL_L signal*/
static void toggle_fpga_eeprom_bus(bool cpu_own)
{
	qrio_gpio_direction_output(GPIO_A, PROM_SEL_L, !cpu_own);
}

#define CONF_SEL_L	10
#define FPGA_PROG_L	19
#define FPGA_DONE	18
#define FPGA_INIT_L	17

int trigger_fpga_config(void)
{
	int ret = 0, init_l;
	/* approx 10ms */
	u32 timeout = 10000;

	/* make sure the FPGA_can access the EEPROM */
	toggle_fpga_eeprom_bus(false);

	/* assert CONF_SEL_L to be able to drive FPGA_PROG_L */
	qrio_gpio_direction_output(GPIO_A, CONF_SEL_L, 0);

	/* trigger the config start */
	qrio_gpio_direction_output(GPIO_A, FPGA_PROG_L, 0);

	/* small delay for INIT_L line */
	udelay(10);

	/* wait for FPGA_INIT to be asserted */
	do {
		init_l = qrio_get_gpio(GPIO_A, FPGA_INIT_L);
		if (timeout-- == 0) {
			printf("FPGA_INIT timeout\n");
			ret = -EFAULT;
			break;
		}
		udelay(10);
	} while (init_l);

	/* deassert FPGA_PROG, config should start */
	qrio_set_gpio(GPIO_A, FPGA_PROG_L, 1);

	return ret;
}

/* poll the FPGA_DONE signal and give the EEPROM back to the QorIQ */
static int wait_for_fpga_config(void)
{
	int ret = 0, done;
	/* approx 5 s */
	u32 timeout = 500000;

	printf("PCIe FPGA config:");
	do {
		done = qrio_get_gpio(GPIO_A, FPGA_DONE);
		if (timeout-- == 0) {
			printf(" FPGA_DONE timeout\n");
			ret = -EFAULT;
			goto err_out;
		}
		udelay(10);
	} while (!done);

	printf(" done\n");

err_out:
	/* deactive CONF_SEL and give the CPU conf EEPROM access */
	qrio_set_gpio(GPIO_A, CONF_SEL_L, 1);
	toggle_fpga_eeprom_bus(true);

	return ret;
}

#define PCIE_SW_RST	14
#define PEXHC_RST	13
#define HOOPER_RST	12

void pci_init_board(void)
{
	qrio_prstcfg(PCIE_SW_RST, PRSTCFG_POWUP_UNIT_CORE_RST);
	qrio_prstcfg(PEXHC_RST, PRSTCFG_POWUP_UNIT_CORE_RST);
	qrio_prstcfg(HOOPER_RST, PRSTCFG_POWUP_UNIT_CORE_RST);

	/* wait for the PCIe FPGA to be configured
	 * it has been triggered earlier in board_early_init_r */
	if (wait_for_fpga_config())
		printf("error finishing PCIe FPGA config\n");

	qrio_prst(PCIE_SW_RST, false, false);
	qrio_prst(PEXHC_RST, false, false);
	qrio_prst(HOOPER_RST, false, false);
	/* Hooper is not direcly PCIe capable */
	mdelay(50);

	fsl_pcie_init_board(0);
}

void pci_of_setup(void *blob, bd_t *bd)
{
	FT_FSL_PCI_SETUP;
}
