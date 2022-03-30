// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <hwconfig.h>
#include <i2c.h>
#include <spi.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <pci.h>
#include <mpc83xx.h>
#include <fsl_esdhc.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_mpc83xx_serdes.h>

#include "mpc8308.h"

#include <gdsys_fpga.h>

#include "../common/ioep-fpga.h"
#include "../common/osd.h"
#include "../common/mclink.h"
#include "../common/phy.h"
#include "../common/fanctrl.h"

#include <pca953x.h>
#include <pca9698.h>

#include <miiphy.h>

#define MAX_MUX_CHANNELS 2

enum {
	MCFPGA_DONE = BIT(0),
	MCFPGA_INIT_N = BIT(1),
	MCFPGA_PROGRAM_N = BIT(2),
	MCFPGA_UPDATE_ENABLE_N = BIT(3),
	MCFPGA_RESET_N = BIT(4),
};

enum {
	GPIO_MDC = 1 << 14,
	GPIO_MDIO = 1 << 15,
};

uint mclink_fpgacount;
struct ihs_fpga *fpga_ptr[] = CONFIG_SYS_FPGA_PTR;

struct {
	u8 bus;
	u8 addr;
} hrcon_fans[] = CONFIG_HRCON_FANS;

int fpga_set_reg(u32 fpga, u16 *reg, off_t regoff, u16 data)
{
	int res;

	switch (fpga) {
	case 0:
		out_le16(reg, data);
		break;
	default:
		res = mclink_send(fpga - 1, regoff, data);
		if (res < 0) {
			printf("mclink_send reg %02lx data %04x returned %d\n",
			       regoff, data, res);
			return res;
		}
		break;
	}

	return 0;
}

int fpga_get_reg(u32 fpga, u16 *reg, off_t regoff, u16 *data)
{
	int res;

	switch (fpga) {
	case 0:
		*data = in_le16(reg);
		break;
	default:
		if (fpga > mclink_fpgacount)
			return -EINVAL;
		res = mclink_receive(fpga - 1, regoff, data);
		if (res < 0) {
			printf("mclink_receive reg %02lx returned %d\n",
			       regoff, res);
			return res;
		}
	}

	return 0;
}

int checkboard(void)
{
	char *s = env_get("serial#");
	bool hw_type_cat = pca9698_get_value(0x20, 20);

	puts("Board: ");

	printf("HRCon %s", hw_type_cat ? "CAT" : "Fiber");

	if (s) {
		puts(", serial# ");
		puts(s);
	}

	puts("\n");

	return 0;
}

int last_stage_init(void)
{
	int slaves;
	uint k;
	uchar mclink_controllers[] = { 0x3c, 0x3d, 0x3e };
	u16 fpga_features;
	bool hw_type_cat = pca9698_get_value(0x20, 20);
	bool ch0_rgmii2_present;

	FPGA_GET_REG(0, fpga_features, &fpga_features);

	/* Turn on Parade DP501 */
	pca9698_direction_output(0x20, 10, 1);
	pca9698_direction_output(0x20, 11, 1);

	ch0_rgmii2_present = !pca9698_get_value(0x20, 30);

	/* wait for FPGA done, then reset FPGA */
	for (k = 0; k < ARRAY_SIZE(mclink_controllers); ++k) {
		uint ctr = 0;

		if (i2c_probe(mclink_controllers[k]))
			continue;

		while (!(pca953x_get_val(mclink_controllers[k])
		       & MCFPGA_DONE)) {
			mdelay(100);
			if (ctr++ > 5) {
				printf("no done for mclink_controller %u\n", k);
				break;
			}
		}

		pca953x_set_dir(mclink_controllers[k], MCFPGA_RESET_N, 0);
		pca953x_set_val(mclink_controllers[k], MCFPGA_RESET_N, 0);
		udelay(10);
		pca953x_set_val(mclink_controllers[k], MCFPGA_RESET_N,
				MCFPGA_RESET_N);
	}

	if (hw_type_cat) {
		uint mux_ch;
		int retval;
		struct mii_dev *mdiodev = mdio_alloc();

		if (!mdiodev)
			return -ENOMEM;
		strncpy(mdiodev->name, bb_miiphy_buses[0].name, MDIO_NAME_LEN);
		mdiodev->read = bb_miiphy_read;
		mdiodev->write = bb_miiphy_write;

		retval = mdio_register(mdiodev);
		if (retval < 0)
			return retval;
		for (mux_ch = 0; mux_ch < MAX_MUX_CHANNELS; ++mux_ch) {
			if ((mux_ch == 1) && !ch0_rgmii2_present)
				continue;

			setup_88e1514(bb_miiphy_buses[0].name, mux_ch);
		}
	}

	/* give slave-PLLs and Parade DP501 some time to be up and running */
	mdelay(500);

	mclink_fpgacount = CONFIG_SYS_MCLINK_MAX;
	slaves = mclink_probe();
	mclink_fpgacount = 0;

	ioep_fpga_print_info(0);
	osd_probe(0);
#ifdef CONFIG_SYS_OSD_DH
	osd_probe(4);
#endif

	if (slaves <= 0)
		return 0;

	mclink_fpgacount = slaves;

	for (k = 1; k <= slaves; ++k) {
		FPGA_GET_REG(k, fpga_features, &fpga_features);

		ioep_fpga_print_info(k);
		osd_probe(k);
#ifdef CONFIG_SYS_OSD_DH
		osd_probe(k + 4);
#endif
		if (hw_type_cat) {
			int retval;
			struct mii_dev *mdiodev = mdio_alloc();

			if (!mdiodev)
				return -ENOMEM;
			strncpy(mdiodev->name, bb_miiphy_buses[k].name,
				MDIO_NAME_LEN);
			mdiodev->read = bb_miiphy_read;
			mdiodev->write = bb_miiphy_write;

			retval = mdio_register(mdiodev);
			if (retval < 0)
				return retval;
			setup_88e1514(bb_miiphy_buses[k].name, 0);
		}
	}

	for (k = 0; k < ARRAY_SIZE(hrcon_fans); ++k) {
		i2c_set_bus_num(hrcon_fans[k].bus);
		init_fan_controller(hrcon_fans[k].addr);
	}

	return 0;
}

/*
 * provide access to fpga gpios and controls (for I2C bitbang)
 * (these may look all too simple but make iocon.h much more readable)
 */
void fpga_gpio_set(uint bus, int pin)
{
	FPGA_SET_REG(bus >= 4 ? (bus - 4) : bus, gpio.set, pin);
}

void fpga_gpio_clear(uint bus, int pin)
{
	FPGA_SET_REG(bus >= 4 ? (bus - 4) : bus, gpio.clear, pin);
}

int fpga_gpio_get(uint bus, int pin)
{
	u16 val;

	FPGA_GET_REG(bus >= 4 ? (bus - 4) : bus, gpio.read, &val);

	return val & pin;
}

void fpga_control_set(uint bus, int pin)
{
	u16 val;

	FPGA_GET_REG(bus >= 4 ? (bus - 4) : bus, control, &val);
	FPGA_SET_REG(bus >= 4 ? (bus - 4) : bus, control, val | pin);
}

void fpga_control_clear(uint bus, int pin)
{
	u16 val;

	FPGA_GET_REG(bus >= 4 ? (bus - 4) : bus, control, &val);
	FPGA_SET_REG(bus >= 4 ? (bus - 4) : bus, control, val & ~pin);
}

void mpc8308_init(void)
{
	pca9698_direction_output(0x20, 4, 1);
}

void mpc8308_set_fpga_reset(uint state)
{
	pca9698_set_value(0x20, 4, state ? 0 : 1);
}

void mpc8308_setup_hw(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	/*
	 * set "startup-finished"-gpios
	 */
	setbits_be32(&immr->gpio[0].dir, BIT(31 - 11) | BIT(31 - 12));
	setbits_gpio0_out(BIT(31 - 12));
}

int mpc8308_get_fpga_done(uint fpga)
{
	return pca9698_get_value(0x20, 19);
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_init(bd_t *bd)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;

	/* Enable cache snooping in eSDHC system configuration register */
	out_be32(&sysconf->sdhccr, 0x02000000);

	return fsl_esdhc_mmc_init(bd);
}
#endif

static struct pci_region pcie_regions_0[] = {
	{
		.bus_start = CONFIG_SYS_PCIE1_MEM_BASE,
		.phys_start = CONFIG_SYS_PCIE1_MEM_PHYS,
		.size = CONFIG_SYS_PCIE1_MEM_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CONFIG_SYS_PCIE1_IO_BASE,
		.phys_start = CONFIG_SYS_PCIE1_IO_PHYS,
		.size = CONFIG_SYS_PCIE1_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

void pci_init_board(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;
	law83xx_t *pcie_law = sysconf->pcielaw;
	struct pci_region *pcie_reg[] = { pcie_regions_0 };

	fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_PEX,
			 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);

	/* Deassert the resets in the control register */
	out_be32(&sysconf->pecr1, 0xE0008000);
	udelay(2000);

	/* Configure PCI Express Local Access Windows */
	out_be32(&pcie_law[0].bar, CONFIG_SYS_PCIE1_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[0].ar, LBLAWAR_EN | LBLAWAR_512MB);

	mpc83xx_pcie_init(1, pcie_reg);
}

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	info->portwidth = FLASH_CFI_16BIT;
	info->chipwidth = FLASH_CFI_BY16;
	info->interface = FLASH_CFI_X16;
	return 1;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fsl_fdt_fixup_dr_usb(blob, bd);
	fdt_fixup_esdhc(blob, bd);

	return 0;
}
#endif

/*
 * FPGA MII bitbang implementation
 */

struct fpga_mii {
	uint fpga;
	int mdio;
} fpga_mii[] = {
	{ 0, 1},
	{ 1, 1},
	{ 2, 1},
	{ 3, 1},
};

static int mii_dummy_init(struct bb_miiphy_bus *bus)
{
	return 0;
}

static int mii_mdio_active(struct bb_miiphy_bus *bus)
{
	struct fpga_mii *fpga_mii = bus->priv;

	if (fpga_mii->mdio)
		FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDIO);
	else
		FPGA_SET_REG(fpga_mii->fpga, gpio.clear, GPIO_MDIO);

	return 0;
}

static int mii_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct fpga_mii *fpga_mii = bus->priv;

	FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDIO);

	return 0;
}

static int mii_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct fpga_mii *fpga_mii = bus->priv;

	if (v)
		FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDIO);
	else
		FPGA_SET_REG(fpga_mii->fpga, gpio.clear, GPIO_MDIO);

	fpga_mii->mdio = v;

	return 0;
}

static int mii_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	u16 gpio;
	struct fpga_mii *fpga_mii = bus->priv;

	FPGA_GET_REG(fpga_mii->fpga, gpio.read, &gpio);

	*v = ((gpio & GPIO_MDIO) != 0);

	return 0;
}

static int mii_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct fpga_mii *fpga_mii = bus->priv;

	if (v)
		FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDC);
	else
		FPGA_SET_REG(fpga_mii->fpga, gpio.clear, GPIO_MDC);

	return 0;
}

static int mii_delay(struct bb_miiphy_bus *bus)
{
	udelay(1);

	return 0;
}

struct bb_miiphy_bus bb_miiphy_buses[] = {
	{
		.name = "board0",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[0],
	},
	{
		.name = "board1",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[1],
	},
	{
		.name = "board2",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[2],
	},
	{
		.name = "board3",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[3],
	},
};

int bb_miiphy_buses_num = ARRAY_SIZE(bb_miiphy_buses);
